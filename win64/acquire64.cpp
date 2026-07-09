// acquire64.cpp -- acquisition run sequencer. See acquire64.h.

#include "acquire64.h"
#include <cstdio>
#include <cmath>
#include <thread>
#include <chrono>

namespace wpeak64 {

AcquireRun::AcquireRun(const Method &m, Hardware &h, int standard_num)
    : m_(m), h_(h), standard_num_(standard_num)
{
}

double AcquireRun::ZoneTempC(const TempZone &z) const
{
    long counts = h_.adc->ReadCounts(z.adc_channel);
    double volts = h_.adc->CountsToVolts(counts);
    return z.scale * volts + z.offset;
}

void AcquireRun::ServiceTempZones()
{
    for(const TempZone &z : m_.zones) {
        if(z.setpoint_c <= 0 || z.heater_line < 0) continue;
        double t = ZoneTempC(z);
        // bang-bang with hysteresis, as the legacy oven relay control
        if(t < z.setpoint_c - z.hysteresis_c)
            h_.out->Set(z.heater_line, true);
        else if(t > z.setpoint_c + z.hysteresis_c)
            h_.out->Set(z.heater_line, false);
    }
}

bool AcquireRun::ZonesInBand() const
{
    for(const TempZone &z : m_.zones) {
        if(z.setpoint_c <= 0) continue;
        double t = ZoneTempC(z);
        if(std::fabs(t - z.setpoint_c) > z.hysteresis_c)
            return false;
    }
    return true;
}

void AcquireRun::Tick(double dt)
{
    if(h_.sim)
        h_.sim->AdvanceSeconds(dt);
    else
        std::this_thread::sleep_for(
            std::chrono::microseconds((long long)(dt * 1e6)));
}

void AcquireRun::AllOff()
{
    const HardwareConfig &hw = m_.hw;
    int lines[] = { hw.sample_valve, hw.inject_valve, hw.cal_valve,
                    hw.purge_valve, hw.pump, hw.lamp, hw.fan };
    for(int l : lines)
        if(l >= 0) h_.out->Set(l, false);
    for(const TempZone &z : m_.zones)
        if(z.heater_line >= 0) h_.out->Set(z.heater_line, false);
}

bool AcquireRun::Run(AcquireResult &out, bool verbose, std::string &err)
{
    const HardwareConfig &hw = m_.hw;
    out = AcquireResult();
    const double tick = 0.1;                         // control loop period, s

    auto phase = [&](const char *name) {
        if(verbose) {
            std::printf("[%s]", name);
            for(const TempZone &z : m_.zones)
                if(z.setpoint_c > 0)
                    std::printf("  %s=%.1fC", z.name.c_str(), ZoneTempC(z));
            std::printf("\n");
        }
    };

    // --- EQUILIBRATE ---
    phase("EQUILIBRATE");
    if(hw.fan >= 0) h_.out->Set(hw.fan, true);
    {
        double waited = 0;
        const double max_wait = 600;                 // give up after 10 min
        while(waited < (double)m_.timing.equil_time ||
              (!ZonesInBand() && waited < max_wait)) {
            ServiceTempZones();
            Tick(tick);
            waited += tick;
        }
        if(!ZonesInBand()) {
            AllOff();
            err = "equilibration timeout: temperature zone(s) not at setpoint";
            return false;
        }
    }

    // --- SAMPLE --- (cal valve replaces sample valve for a calibration run)
    phase(standard_num_ > 0 ? "SAMPLE (calibration standard)" : "SAMPLE");
    int intake = standard_num_ > 0 && hw.cal_valve >= 0 ? hw.cal_valve
                                                        : hw.sample_valve;
    if(intake >= 0)   h_.out->Set(intake, true);
    if(hw.pump >= 0)  h_.out->Set(hw.pump, true);
    for(double t = 0; t < (double)m_.timing.sample_time; t += tick) {
        ServiceTempZones();
        Tick(tick);
    }
    if(hw.pump >= 0)  h_.out->Set(hw.pump, false);
    if(intake >= 0)   h_.out->Set(intake, false);

    // --- INJECT ---
    phase("INJECT");
    if(hw.lamp >= 0)         h_.out->Set(hw.lamp, true);
    if(hw.inject_valve >= 0) h_.out->Set(hw.inject_valve, true);
    if(h_.sim) h_.sim->MarkInjection();
    for(double t = 0; t < (double)m_.timing.inject_time; t += tick) {
        ServiceTempZones();
        Tick(tick);
    }
    if(hw.inject_valve >= 0) h_.out->Set(hw.inject_valve, false);

    // --- ANALYZE --- (retention clock starts at injection)
    phase("ANALYZE");
    Integrator integ(m_.det, m_.data_rate, m_.analysis_time);
    {
        const double dt = 1.0 / m_.data_rate;
        long total = m_.analysis_time * m_.data_rate;
        // note: injection happened inject_time ago; the legacy retention
        // clock also starts when acquisition starts.
        for(long i = 0; i < total; i++) {
            long counts = h_.adc->ReadCounts(hw.adc_channel);
            out.trace.push_back(counts);
            integ.ProcessPoint(counts);
            if(i % (m_.data_rate * 5) == 0)          // service heaters ~5 s
                ServiceTempZones();
            Tick(dt);
        }
    }
    if(hw.lamp >= 0) h_.out->Set(hw.lamp, false);

    // --- PURGE ---
    phase("PURGE");
    if(hw.purge_valve >= 0) h_.out->Set(hw.purge_valve, true);
    if(hw.pump >= 0)        h_.out->Set(hw.pump, true);
    for(double t = 0; t < (double)m_.timing.purge_time; t += tick) {
        ServiceTempZones();
        Tick(tick);
    }
    AllOff();

    out.noise    = integ.noise;
    out.baseline = integ.act_thresh;
    out.peaks    = integ.peaks;
    out.rows     = BuildReport(integ.peaks, m_);
    phase("DONE");
    return true;
}

void AcquireRun::UpdateCalibration(Method &m_out, const AcquireResult &res) const
{
    if(standard_num_ < 1 || standard_num_ > STAND_NUM64) return;
    int idx = standard_num_ - 1;

    for(Component &c : m_out.components) {
        if(!c.active_yn) continue;
        if(c.stand[idx] <= 0) continue;              // no std conc configured
        for(const Peak &p : res.peaks) {
            if(p.Height < 0) continue;               // negative peaks never calibrate
            if(CheckRT(p, c, m_out.data_rate)) {
                double resp = m_out.detect_meth == 0 ? (double)p.Height : p.Area;
                c.cal[idx] = { c.stand[idx], resp, true };
                break;                               // first match wins
            }
        }
    }
}

} // namespace wpeak64
