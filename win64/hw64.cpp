// hw64.cpp -- simulation backend and hardware factory. See hw64.h.

#include "hw64.h"
#include "ads1115.h"
#include "gpio64.h"
#include <cmath>

namespace wpeak64 {

// ---- SimHardware -------------------------------------------------------------
// Detector: ~0.10 V baseline; after an injection, gaussian peaks (the same
// pattern as the synthetic demo, incl. one negative peak) scaled by
// sim_scale. Thermal zones: first-order lag toward 25 C (heater off) or
// 250 C (heater on), tau 20 s -- enough physics to exercise the hysteresis
// controller and the equilibration phase.

static const double kBaselineV   = 0.10;
static const double kAmbientC    = 25.0;
static const double kHeaterMaxC  = 250.0;
static const double kThermalTau  = 20.0;

SimHardware::SimHardware(const HardwareConfig &hw, const std::vector<TempZone> &zones,
                         double sim_scale)
    : hw_(hw), zones_(zones), sim_scale_(sim_scale)
{
    zone_temp_c_.assign(zones_.size(), kAmbientC);
}

void SimHardware::AdvanceSeconds(double dt)
{
    t_ += dt;
    for(size_t z = 0; z < zones_.size(); z++) {
        bool heat = zones_[z].heater_line >= 0 && Get(zones_[z].heater_line);
        double target = heat ? kHeaterMaxC : kAmbientC;
        zone_temp_c_[z] += (target - zone_temp_c_[z]) * (dt / kThermalTau);
    }
}

double SimHardware::DetectorVolts()
{
    double v = kBaselineV;
    if(inject_t_ >= 0) {
        // peak pattern (s after injection, height V, sigma s) -- mirrors
        // synth64.h DefaultPeaks scaled to the ADS1115 2.048 V range
        static const struct { double rt, h, sig; } defs[] = {
            { 40.0, +0.400, 3.0 },
            { 60.0, +0.250, 4.0 },
            { 85.0, -0.090, 3.0 },     // negative peak (baseline is 0.10 V)
            {105.0, +0.150, 3.5 },
        };
        double ta = t_ - inject_t_;
        for(const auto &d : defs) {
            double dt = (ta - d.rt) / d.sig;
            v += sim_scale_ * point_factor_ * d.h * std::exp(-0.5 * dt * dt);
        }
    }
    rng_ = rng_ * 1103515245u + 12345u;
    v += 0.0002 * (((rng_ >> 16) & 0x7FFF) / 32767.0 - 0.5);
    return v;
}

long SimHardware::VoltsToCounts(double v) const
{
    double fsr = hw_.pga_mv / 1000.0;
    double c = v / fsr * 32768.0;
    if(c > 32767) c = 32767;
    if(c < -32768) c = -32768;
    return (long)std::lround(c);
}

long SimHardware::ReadCounts(int channel)
{
    if(channel == hw_.adc_channel)
        return VoltsToCounts(DetectorVolts());
    for(size_t z = 0; z < zones_.size(); z++) {
        if(zones_[z].adc_channel == channel) {
            // invert the zone's linear conversion: volts = (temp-offset)/scale
            double v = (zone_temp_c_[z] - zones_[z].offset) / zones_[z].scale;
            return VoltsToCounts(v);
        }
    }
    return VoltsToCounts(0);
}

double SimHardware::CountsToVolts(long counts) const
{
    return (double)counts * (hw_.pga_mv / 1000.0) / 32768.0;
}

void SimHardware::Set(int line, bool on)
{
    if(line >= 0 && line < 64) lines_[line] = on;
}

bool SimHardware::Get(int line) const
{
    return line >= 0 && line < 64 ? lines_[line] : false;
}

// ---- factory -----------------------------------------------------------------
bool OpenHardware(const HardwareConfig &hw, const std::vector<TempZone> &zones,
                  double sim_scale, Hardware &h, std::string &err)
{
    h = Hardware();
    if(hw.backend == "sim") {
        h.sim = new SimHardware(hw, zones, sim_scale);
        h.adc = h.sim;
        h.out = h.sim;
        return true;
    }
    if(hw.backend == "ads1115") {
#ifdef __linux__
        Ads1115 *adc = Ads1115::Open(hw, err);
        if(!adc) return false;

        std::vector<int> lines = {
            hw.sample_valve, hw.inject_valve, hw.cal_valve, hw.purge_valve,
            hw.pump, hw.lamp, hw.fan,
            hw.alarm_high_line, hw.alarm_low_line,
        };
        for(int pv : hw.point_valves)
            lines.push_back(pv);
        for(const TempZone &z : zones)
            lines.push_back(z.heater_line);
        GpioOut *out = GpioOut::Open(hw.gpio_chip, lines, err);
        if(!out) { delete adc; return false; }

        h.adc = adc;
        h.out = out;
        return true;
#else
        err = "backend=ads1115 requires Linux (i2c-dev/gpiochip); "
              "use backend=sim on Windows";
        return false;
#endif
    }
    err = "unknown hardware backend '" + hw.backend + "' (sim|ads1115)";
    return false;
}

void CloseHardware(Hardware &h)
{
    if(h.sim) {                 // sim object serves as both interfaces
        delete h.sim;
    }
    else {
        delete h.adc;
        delete h.out;
    }
    h = Hardware();
}

} // namespace wpeak64
