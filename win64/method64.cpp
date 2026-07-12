// method64.cpp -- Method file parsing, CSV chromatogram loading, component
// matching and report generation for WPEAK64. See method64.h.

#include "method64.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <ctime>

namespace wpeak64 {

static std::string Trim(const std::string &s)
{
    size_t a = s.find_first_not_of(" \t\r\n");
    size_t b = s.find_last_not_of(" \t\r\n");
    return a == std::string::npos ? std::string() : s.substr(a, b - a + 1);
}

static std::string Lower(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c){ return (char)std::tolower(c); });
    return s;
}

// Shared key parsing for [detector]/[detector_b] and [component]/[component_b]
// -- detector A and B use an identical key set, differing only in which
// struct fields they land in (Method's top-level fields for A, the det_b_*
// fields for B). Returns false if the key isn't recognized.
static bool ParseDetectorSettingKey(DetectorSettings &d, const std::string &key, const std::string &val)
{
    if     (key == "segment_width") d.segment_width = std::atoi(val.c_str());
    else if(key == "nandb_time")    d.NandBtime     = std::atol(val.c_str());
    else if(key == "nandb_len")     d.NandBlen      = std::atol(val.c_str());
    else if(key == "min_height")    d.MinHeight     = std::atol(val.c_str());
    else if(key == "min_area")      d.MinArea       = std::atof(val.c_str());
    else if(key == "peak_alg")      d.peak_alg      = std::atoi(val.c_str());
    else if(key == "noise_reduct")  d.noise_reduct  = std::atoi(val.c_str());
    else return false;
    return true;
}

static bool ParseComponentKey(Component &c, const std::string &key, const std::string &val)
{
    if     (key == "name")     c.name      = val;
    else if(key == "rt")       c.peak_rt   = std::atof(val.c_str());
    else if(key == "window")   c.window    = std::atof(val.c_str());
    else if(key == "response") c.response  = std::atof(val.c_str());
    else if(key == "active")   c.active_yn = std::atoi(val.c_str()) != 0;
    else if(key == "alarm_high") c.alarm_high = std::atof(val.c_str());
    else if(key == "alarm_low")  c.alarm_low  = std::atof(val.c_str());
    else if(key == "dac_channel") c.dac_channel = std::atoi(val.c_str());
    else if(key == "dac_range")   c.dac_range   = std::atof(val.c_str());
    else if(key.size() == 4 && key.compare(0, 3, "std") == 0 &&
            key[3] >= '1' && key[3] <= '0' + STAND_NUM64) {
        c.stand[key[3] - '1'] = std::atof(val.c_str());   // std1..std8
    }
    else return false;
    return true;
}

bool LoadMethod(const std::string &path, Method &m, std::string &err)
{
    std::ifstream f(path);
    if(!f) { err = "cannot open method file: " + path; return false; }

    m = Method();
    std::string section;
    Component *cur = nullptr;
    TempZone  *zone = nullptr;
    std::string line;
    int lineno = 0;

    while(std::getline(f, line)) {
        lineno++;
        std::string t = Trim(line);
        if(t.empty() || t[0] == '#' || t[0] == ';') continue;

        if(t.front() == '[' && t.back() == ']') {
            section = Lower(Trim(t.substr(1, t.size() - 2)));
            if(section == "component") {
                m.components.push_back(Component());
                cur = &m.components.back();
            }
            else if(section == "component_b") {
                m.components_b.push_back(Component());
                cur = &m.components_b.back();
            }
            else if(section == "tempzone") {
                m.zones.push_back(TempZone());
                zone = &m.zones.back();
            }
            else if(section != "detector" && section != "detector_b" &&
                    section != "hardware" && section != "timing") {
                err = path + ":" + std::to_string(lineno) + ": unknown section [" + section + "]";
                return false;
            }
            continue;
        }

        size_t eq = t.find('=');
        if(eq == std::string::npos) {
            err = path + ":" + std::to_string(lineno) + ": expected key=value";
            return false;
        }
        std::string key = Lower(Trim(t.substr(0, eq)));
        std::string val = Trim(t.substr(eq + 1));

        if(section == "detector") {
            if(ParseDetectorSettingKey(m.det, key, val)) {}
            else if(key == "detect_meth")   m.detect_meth       = std::atoi(val.c_str());
            else if(key == "known_peaks")   m.known_peaks       = std::atoi(val.c_str()) != 0;
            else if(key == "data_rate")     m.data_rate         = std::atoi(val.c_str());
            else if(key == "analysis_time") m.analysis_time     = std::atol(val.c_str());
            else { err = path + ":" + std::to_string(lineno) + ": unknown detector key '" + key + "'"; return false; }
        }
        // Detector B: a second, independent detector (legacy NUMDETECTORS=2)
        // sampled in the same run on its own ADC channel. data_rate/
        // analysis_time are shared (one run, both detectors); everything
        // else -- segment width, noise window, MinHeight/Area, peak
        // algorithm, detect method, known-peaks -- is separate.
        else if(section == "detector_b") {
            if(ParseDetectorSettingKey(m.det_b, key, val)) {}
            else if(key == "enabled")       m.det_b_enabled     = std::atoi(val.c_str()) != 0;
            else if(key == "adc_channel")   m.det_b_adc_channel = std::atoi(val.c_str());
            else if(key == "detect_meth")   m.det_b_detect_meth = std::atoi(val.c_str());
            else if(key == "known_peaks")   m.det_b_known_peaks = std::atoi(val.c_str()) != 0;
            else { err = path + ":" + std::to_string(lineno) + ": unknown detector_b key '" + key + "'"; return false; }
        }
        else if((section == "component" || section == "component_b") && cur) {
            if(!ParseComponentKey(*cur, key, val)) {
                err = path + ":" + std::to_string(lineno) + ": unknown component key '" + key + "'";
                return false;
            }
        }
        else if(section == "hardware") {
            HardwareConfig &h = m.hw;
            if     (key == "backend")      h.backend      = Lower(val);
            else if(key == "i2c_dev")      h.i2c_dev      = val;
            else if(key == "i2c_addr")     h.i2c_addr     = (int)std::strtol(val.c_str(), nullptr, 0);
            else if(key == "adc_channel")  h.adc_channel  = std::atoi(val.c_str());
            else if(key == "pga_mv")       h.pga_mv       = std::atoi(val.c_str());
            else if(key == "sps")          h.sps          = std::atoi(val.c_str());
            else if(key == "gpio_chip")    h.gpio_chip    = val;
            else if(key == "sample_valve") h.sample_valve = std::atoi(val.c_str());
            else if(key == "inject_valve") h.inject_valve = std::atoi(val.c_str());
            else if(key == "cal_valve")    h.cal_valve    = std::atoi(val.c_str());
            else if(key == "purge_valve")  h.purge_valve  = std::atoi(val.c_str());
            else if(key == "pump")         h.pump         = std::atoi(val.c_str());
            else if(key == "lamp")         h.lamp         = std::atoi(val.c_str());
            else if(key == "fan")          h.fan          = std::atoi(val.c_str());
            else if(key == "autozero")     h.autozero     = std::atoi(val.c_str());
            else if(key == "alarm_high_line") h.alarm_high_line = std::atoi(val.c_str());
            else if(key == "alarm_low_line")  h.alarm_low_line  = std::atoi(val.c_str());
            else if(key == "point_valves") {   // comma-separated GPIO line list
                h.point_valves.clear();
                std::stringstream ss(val);
                std::string tok;
                while(std::getline(ss, tok, ','))
                    if(!Trim(tok).empty())
                        h.point_valves.push_back(std::atoi(Trim(tok).c_str()));
            }
            else if(key == "dac_i2c_addrs") {  // comma-separated I2C address list
                h.dac_i2c_addrs.clear();
                std::stringstream ss(val);
                std::string tok;
                while(std::getline(ss, tok, ','))
                    if(!Trim(tok).empty())
                        h.dac_i2c_addrs.push_back((int)std::strtol(Trim(tok).c_str(), nullptr, 0));
            }
            else if(key == "dac_vref") h.dac_vref = std::atof(val.c_str());
            else { err = path + ":" + std::to_string(lineno) + ": unknown hardware key '" + key + "'"; return false; }
        }
        else if(section == "tempzone" && zone) {
            if     (key == "name")        zone->name         = val;
            else if(key == "adc_channel") zone->adc_channel  = std::atoi(val.c_str());
            else if(key == "heater_line") zone->heater_line  = std::atoi(val.c_str());
            else if(key == "setpoint")    zone->setpoint_c   = std::atof(val.c_str());
            else if(key == "hysteresis")  zone->hysteresis_c = std::atof(val.c_str());
            else if(key == "scale")       zone->scale        = std::atof(val.c_str());
            else if(key == "offset")      zone->offset       = std::atof(val.c_str());
            // optional oven temperature program (legacy TCTRL.CPP -- see
            // tempprogram.h); a run's initial equilibration targets
            // prog_initial_temp instead of setpoint when any of these are set.
            else if(key == "prog_initial_temp") zone->program.initial_temp_c   = std::atof(val.c_str());
            else if(key == "prog_initial_hold") zone->program.initial_hold_s   = std::atol(val.c_str());
            else if(key == "prog_ramp1_rate")   zone->program.ramp1_rate_c_min = std::atof(val.c_str());
            else if(key == "prog_temp2")        zone->program.temp2_c          = std::atof(val.c_str());
            else if(key == "prog_hold2")        zone->program.hold2_s          = std::atol(val.c_str());
            else if(key == "prog_ramp2_rate")   zone->program.ramp2_rate_c_min = std::atof(val.c_str());
            else if(key == "prog_temp3")        zone->program.temp3_c          = std::atof(val.c_str());
            else { err = path + ":" + std::to_string(lineno) + ": unknown tempzone key '" + key + "'"; return false; }
        }
        else if(section == "timing") {
            if     (key == "equil_time")  m.timing.equil_time  = std::atol(val.c_str());
            else if(key == "autozero_time") m.timing.autozero_time = std::atol(val.c_str());
            else if(key == "sample_time") m.timing.sample_time = std::atol(val.c_str());
            else if(key == "inject_time") m.timing.inject_time = std::atol(val.c_str());
            else if(key == "purge_time")  m.timing.purge_time  = std::atol(val.c_str());
            else if(key == "repeat_interval")   m.timing.repeat_interval   = std::atol(val.c_str());
            else if(key == "auto_cal_every")    m.timing.auto_cal_every    = std::atoi(val.c_str());
            else if(key == "auto_cal_standard") m.timing.auto_cal_standard = std::atoi(val.c_str());
            else { err = path + ":" + std::to_string(lineno) + ": unknown timing key '" + key + "'"; return false; }
        }
        else {
            err = path + ":" + std::to_string(lineno) + ": key outside of a section";
            return false;
        }
    }

    if(m.data_rate <= 0 || m.analysis_time <= 0 || m.det.segment_width <= 0) {
        err = path + ": data_rate, analysis_time and segment_width must be positive";
        return false;
    }
    return true;
}

bool SaveMethod(const std::string &path, const Method &m, std::string &err)
{
    std::ofstream f(path);
    if(!f) { err = "cannot write method file: " + path; return false; }
    char b[128];

    f << "# GC301c WPEAK64 method\n\n[detector]\n";
    f << "segment_width=" << m.det.segment_width << "\n";
    f << "nandb_time="    << m.det.NandBtime     << "\n";
    f << "nandb_len="     << m.det.NandBlen      << "\n";
    f << "min_height="    << m.det.MinHeight     << "\n";
    std::snprintf(b, sizeof b, "min_area=%g\n", m.det.MinArea); f << b;
    f << "peak_alg="      << m.det.peak_alg      << "\n";
    f << "noise_reduct="  << m.det.noise_reduct  << "\n";
    f << "detect_meth="   << m.detect_meth       << "\n";
    f << "known_peaks="   << (m.known_peaks ? 1 : 0) << "\n";
    f << "data_rate="     << m.data_rate         << "\n";
    f << "analysis_time=" << m.analysis_time     << "\n";

    auto write_component = [&](const Component &c, const char *section) {
        f << "\n[" << section << "]\nname=" << c.name << "\n";
        std::snprintf(b, sizeof b, "rt=%g\nwindow=%g\nresponse=%g\n",
                      c.peak_rt, c.window, c.response);
        f << b << "active=" << (c.active_yn ? 1 : 0) << "\n";
        if(c.alarm_high > 0) { std::snprintf(b, sizeof b, "alarm_high=%g\n", c.alarm_high); f << b; }
        if(c.alarm_low  > 0) { std::snprintf(b, sizeof b, "alarm_low=%g\n",  c.alarm_low);  f << b; }
        if(c.dac_channel >= 0 && c.dac_range > 0) {
            std::snprintf(b, sizeof b, "dac_channel=%d\ndac_range=%g\n", c.dac_channel, c.dac_range);
            f << b;
        }
        for(int s = 0; s < STAND_NUM64; s++)
            if(c.stand[s] > 0) {
                std::snprintf(b, sizeof b, "std%d=%g\n", s + 1, c.stand[s]);
                f << b;
            }
    };
    for(const Component &c : m.components) write_component(c, "component");

    if(m.det_b_enabled) {
        f << "\n[detector_b]\nenabled=1\n";
        f << "adc_channel=" << m.det_b_adc_channel << "\n";
        f << "segment_width=" << m.det_b.segment_width << "\n";
        f << "nandb_time="    << m.det_b.NandBtime     << "\n";
        f << "nandb_len="     << m.det_b.NandBlen      << "\n";
        f << "min_height="    << m.det_b.MinHeight     << "\n";
        std::snprintf(b, sizeof b, "min_area=%g\n", m.det_b.MinArea); f << b;
        f << "peak_alg="      << m.det_b.peak_alg      << "\n";
        f << "noise_reduct="  << m.det_b.noise_reduct  << "\n";
        f << "detect_meth="   << m.det_b_detect_meth   << "\n";
        f << "known_peaks="   << (m.det_b_known_peaks ? 1 : 0) << "\n";

        for(const Component &c : m.components_b) write_component(c, "component_b");
    }

    const HardwareConfig &h = m.hw;
    f << "\n[hardware]\nbackend=" << h.backend << "\n";
    f << "i2c_dev=" << h.i2c_dev << "\n";
    std::snprintf(b, sizeof b, "i2c_addr=0x%02X\n", h.i2c_addr); f << b;
    f << "adc_channel=" << h.adc_channel << "\npga_mv=" << h.pga_mv
      << "\nsps=" << h.sps << "\ngpio_chip=" << h.gpio_chip << "\n";
    auto line_kv = [&](const char *k, int v) { if(v >= 0) f << k << "=" << v << "\n"; };
    line_kv("sample_valve", h.sample_valve);
    line_kv("inject_valve", h.inject_valve);
    line_kv("cal_valve",    h.cal_valve);
    line_kv("purge_valve",  h.purge_valve);
    line_kv("pump",         h.pump);
    line_kv("lamp",         h.lamp);
    line_kv("fan",          h.fan);
    line_kv("autozero",     h.autozero);
    line_kv("alarm_high_line", h.alarm_high_line);
    line_kv("alarm_low_line",  h.alarm_low_line);
    if(!h.point_valves.empty()) {
        f << "point_valves=";
        for(size_t i = 0; i < h.point_valves.size(); i++)
            f << (i ? "," : "") << h.point_valves[i];
        f << "\n";
    }
    if(!h.dac_i2c_addrs.empty()) {
        f << "dac_i2c_addrs=";
        for(size_t i = 0; i < h.dac_i2c_addrs.size(); i++) {
            std::snprintf(b, sizeof b, "0x%02X", h.dac_i2c_addrs[i]);
            f << (i ? "," : "") << b;
        }
        std::snprintf(b, sizeof b, "\ndac_vref=%g\n", h.dac_vref);
        f << b;
    }

    for(const TempZone &z : m.zones) {
        f << "\n[tempzone]\nname=" << z.name << "\n";
        f << "adc_channel=" << z.adc_channel << "\nheater_line=" << z.heater_line << "\n";
        std::snprintf(b, sizeof b, "setpoint=%g\nhysteresis=%g\nscale=%g\noffset=%g\n",
                      z.setpoint_c, z.hysteresis_c, z.scale, z.offset);
        f << b;
        if(z.program.Enabled()) {
            char pb[256];
            std::snprintf(pb, sizeof pb,
                "prog_initial_temp=%g\nprog_initial_hold=%ld\nprog_ramp1_rate=%g\n"
                "prog_temp2=%g\nprog_hold2=%ld\nprog_ramp2_rate=%g\nprog_temp3=%g\n",
                z.program.initial_temp_c, z.program.initial_hold_s, z.program.ramp1_rate_c_min,
                z.program.temp2_c, z.program.hold2_s, z.program.ramp2_rate_c_min, z.program.temp3_c);
            f << pb;
        }
    }

    const TimingConfig &t = m.timing;
    f << "\n[timing]\nequil_time=" << t.equil_time << "\n";
    if(t.autozero_time > 0) f << "autozero_time=" << t.autozero_time << "\n";
    f << "sample_time=" << t.sample_time << "\ninject_time=" << t.inject_time
      << "\npurge_time=" << t.purge_time << "\n";
    if(t.repeat_interval > 0) f << "repeat_interval=" << t.repeat_interval << "\n";
    if(t.auto_cal_every > 0)
        f << "auto_cal_every=" << t.auto_cal_every
          << "\nauto_cal_standard=" << t.auto_cal_standard << "\n";
    return (bool)f;
}

bool LoadChromatogram(const std::string &path, std::vector<long> &y, std::string &err)
{
    std::ifstream f(path);
    if(!f) { err = "cannot open data file: " + path; return false; }

    y.clear();
    std::string line;
    int lineno = 0;
    while(std::getline(f, line)) {
        lineno++;
        std::string t = Trim(line);
        if(t.empty() || t[0] == '#' || t[0] == ';') continue;

        // "value" or "time,value" -- take the last comma-separated field
        size_t comma = t.find_last_of(',');
        std::string field = Trim(comma == std::string::npos ? t : t.substr(comma + 1));

        char *end = nullptr;
        double v = std::strtod(field.c_str(), &end);
        if(end == field.c_str() || (end && Trim(end).size())) {
            if(y.empty() && lineno == 1) continue;   // tolerate one header line
            err = path + ":" + std::to_string(lineno) + ": not a number: '" + field + "'";
            return false;
        }
        y.push_back((long)std::lround(v));
    }
    if(y.empty()) { err = path + ": no data points"; return false; }
    return true;
}

bool CheckRT(const Peak &peak, const Component &c, int data_rate)
{
    double rt_s = (double)peak.Time / data_rate;
    return std::fabs(rt_s - c.peak_rt) <= c.window;   // CALC.CPP CheckRT
}

// Legacy multipoint calibration (CalcConcVars builds the piecewise table,
// Detector::Concentration interpolates on it): calibration points sorted by
// response; below the first standard the segment is the line from the origin
// to that standard, between standards it is linear interpolation, and above
// the last standard the line through the origin and the last standard is
// used (lslope[last+1] = lvalue/lstart in CALC.CPP).
bool ConcentrationFromCal(const Component &c, double response, double &conc)
{
    // collect valid points, sorted ascending by response
    std::vector<CalPoint> pts;
    for(int i = 0; i < STAND_NUM64; i++)
        if(c.cal[i].valid && c.cal[i].resp > 0 && c.cal[i].conc > 0)
            pts.push_back(c.cal[i]);
    if(pts.empty()) return false;
    std::sort(pts.begin(), pts.end(),
              [](const CalPoint &a, const CalPoint &b){ return a.resp < b.resp; });

    if(response <= 0) { conc = 0; return true; }

    const CalPoint &last = pts.back();
    if(response >= last.resp) {                    // extrapolate through origin
        conc = last.conc / last.resp * response;
        return true;
    }
    double r0 = 0, c0 = 0;                         // segment start (origin first)
    for(const CalPoint &p : pts) {
        if(response <= p.resp) {
            conc = c0 + (p.conc - c0) / (p.resp - r0) * (response - r0);
            return true;
        }
        r0 = p.resp; c0 = p.conc;
    }
    conc = last.conc / last.resp * response;       // unreachable, kept for safety
    return true;
}

std::vector<ReportRow> BuildReport(const std::vector<Peak> &peaks, const Method &m)
{
    std::vector<ReportRow> rows;
    for(const Peak &p : peaks) {
        ReportRow row;
        row.peak = p;

        if(p.Height < 0) {                    // negative peak: never identified/quantified
            row.name = "NEGATIVE";
            rows.push_back(row);
            continue;
        }

        for(size_t i = 0; i < m.components.size(); i++) {   // first match wins (PeakMatchup)
            const Component &c = m.components[i];
            if(!c.active_yn) continue;
            if(CheckRT(p, c, m.data_rate)) {
                row.component = (int)i;
                row.name = c.name;
                double amount = m.detect_meth == 0 ? (double)p.Height : p.Area;
                double conc;
                if(ConcentrationFromCal(c, amount, conc)) {   // multipoint cal
                    row.concentration = conc;
                    row.calibrated = true;
                }
                else if(c.response != 0) {                    // fixed factor
                    row.concentration = c.response * amount;
                    row.calibrated = true;
                }
                break;
            }
        }
        if(row.component < 0) {
            if(m.known_peaks) continue;       // known peaks only: drop unknowns
            row.name = "unknown";
        }
        rows.push_back(row);
    }
    return rows;
}

int EvaluateAlarms(std::vector<ReportRow> &rows, const Method &m)
{
    int all = ALARM_NONE;
    for(ReportRow &r : rows) {
        r.alarm = ALARM_NONE;
        if(r.component < 0 || !r.calibrated || r.peak.Height < 0)
            continue;
        const Component &c = m.components[(size_t)r.component];
        if(c.alarm_high > 0 && r.concentration >= c.alarm_high)
            r.alarm |= ALARM_HIGH;
        if(c.alarm_low > 0 && r.concentration <= c.alarm_low)
            r.alarm |= ALARM_LOW;
        all |= r.alarm;
    }
    return all;
}

bool LoadCalibration(const std::string &path, Method &m, std::string &err)
{
    std::ifstream f(path);
    if(!f) { err = "cannot open calibration file: " + path; return false; }

    Component *cur = nullptr;
    std::string line, section;
    int lineno = 0;
    while(std::getline(f, line)) {
        lineno++;
        std::string t = Trim(line);
        if(t.empty() || t[0] == '#' || t[0] == ';') continue;
        if(t.front() == '[' && t.back() == ']') {
            section = Lower(Trim(t.substr(1, t.size() - 2)));
            cur = nullptr;
            if(section != "calibration") {
                err = path + ":" + std::to_string(lineno) + ": expected [calibration]";
                return false;
            }
            continue;
        }
        size_t eq = t.find('=');
        if(eq == std::string::npos || section != "calibration") {
            err = path + ":" + std::to_string(lineno) + ": expected key=value in [calibration]";
            return false;
        }
        std::string key = Lower(Trim(t.substr(0, eq)));
        std::string val = Trim(t.substr(eq + 1));

        if(key == "component") {
            cur = nullptr;
            for(Component &c : m.components)
                if(Lower(c.name) == Lower(val)) { cur = &c; break; }
            if(!cur)
                for(Component &c : m.components_b)
                    if(Lower(c.name) == Lower(val)) { cur = &c; break; }
            if(!cur) {
                err = path + ":" + std::to_string(lineno) +
                      ": component '" + val + "' not in method";
                return false;
            }
        }
        else if(key.size() == 4 && key.compare(0, 3, "std") == 0 &&
                key[3] >= '1' && key[3] <= '0' + STAND_NUM64) {
            if(!cur) {
                err = path + ":" + std::to_string(lineno) + ": stdN before component=";
                return false;
            }
            int idx = key[3] - '1';
            double conc = 0, resp = 0;
            if(std::sscanf(val.c_str(), "%lf , %lf", &conc, &resp) != 2) {
                err = path + ":" + std::to_string(lineno) + ": expected stdN=conc,response";
                return false;
            }
            cur->cal[idx] = { conc, resp, true };
        }
        else {
            err = path + ":" + std::to_string(lineno) + ": unknown calibration key '" + key + "'";
            return false;
        }
    }
    return true;
}

bool SaveCalibration(const std::string &path, const Method &m, std::string &err)
{
    std::ofstream f(path);
    if(!f) { err = "cannot write calibration file: " + path; return false; }
    f << "# GC301c WPEAK64 calibration table (stdN = concentration,response)\n";
    auto write_cal = [&](const Component &c) {
        bool any = false;
        for(int i = 0; i < STAND_NUM64; i++)
            if(c.cal[i].valid) any = true;
        if(!any) return;
        f << "\n[calibration]\ncomponent=" << c.name << "\n";
        char buf[96];
        for(int i = 0; i < STAND_NUM64; i++) {
            if(!c.cal[i].valid) continue;
            std::snprintf(buf, sizeof buf, "std%d=%g,%g\n",
                          i + 1, c.cal[i].conc, c.cal[i].resp);
            f << buf;
        }
    };
    for(const Component &c : m.components)   write_cal(c);
    for(const Component &c : m.components_b) write_cal(c);
    return (bool)f;
}

bool WriteReportCsv(const std::string &path, const std::vector<ReportRow> &rows,
                    const Method &m, long noise, long baseline, std::string &err)
{
    std::ofstream f(path);
    if(!f) { err = "cannot write report file: " + path; return false; }

    f << "# GC301c WPEAK64 peak report\n";
    f << "# noise=" << noise << " baseline=" << baseline
      << " detect_meth=" << (m.detect_meth == 0 ? "height" : "area") << "\n";
    f << "num,component,rt_s,height,area,from_s,to_s,concentration,type,alarm\n";
    char buf[256];
    for(const ReportRow &r : rows) {
        const Peak &p = r.peak;
        bool neg = p.Height < 0;
        std::string conc = "";
        if(r.calibrated) {
            std::snprintf(buf, sizeof buf, "%g", r.concentration);
            conc = buf;
        }
        const char *alarm = r.alarm == ALARM_NONE ? "" :
                            r.alarm == ALARM_HIGH ? "HIGH" :
                            r.alarm == ALARM_LOW  ? "LOW"  : "HIGH+LOW";
        std::snprintf(buf, sizeof buf, "%d,%s,%.1f,%ld,%.0f,%.1f,%.1f,%s,%s,%s\n",
                      p.Num,   // one sequence, negative peaks numbered too
                      r.name.c_str(),
                      (double)p.Time / m.data_rate,
                      p.Height,
                      p.Area,
                      (double)p.From / m.data_rate,
                      (double)p.To   / m.data_rate,
                      conc.c_str(),
                      neg ? "negative" : "positive",
                      alarm);
        f << buf;
    }
    return (bool)f;
}

static std::string HtmlEscape(const std::string &s)
{
    std::string out;
    out.reserve(s.size());
    for(char c : s) {
        switch(c) {
            case '&': out += "&amp;"; break;
            case '<': out += "&lt;";  break;
            case '>': out += "&gt;";  break;
            case '"': out += "&quot;"; break;
            default:  out += c;
        }
    }
    return out;
}

bool WriteReportHtml(const std::string &path, const std::vector<long> &trace,
                     const std::vector<ReportRow> &rows, const Method &m,
                     long noise, long baseline, std::string &err)
{
    std::ofstream f(path);
    if(!f) { err = "cannot write report file: " + path; return false; }

    char tbuf[64];
    std::time_t t = std::time(nullptr);
    std::strftime(tbuf, sizeof tbuf, "%Y-%m-%d %H:%M:%S", std::localtime(&t));

    f << "<!doctype html>\n<html><head><meta charset=\"utf-8\">\n"
         "<title>GC301c WPEAK64 Peak Report</title>\n"
         "<style>\n"
         "body{font-family:'Segoe UI',Arial,sans-serif;color:#222;margin:24px;}\n"
         "h1{font-size:18px;margin:0 0 4px 0;}\n"
         ".meta{color:#666;font-size:12px;margin-bottom:16px;}\n"
         "table{border-collapse:collapse;width:100%;font-size:13px;margin-bottom:8px;}\n"
         "th,td{border:1px solid #ccc;padding:4px 8px;text-align:right;}\n"
         "th{background:#26292f;color:#eee;text-align:center;}\n"
         "td:first-child,td:nth-child(2){text-align:left;}\n"
         "tr.neg{color:#c05a00;} tr.alarm{color:#c62828;font-weight:bold;}\n"
         "svg{border:1px solid #ccc;margin-top:16px;background:#fff;max-width:100%;}\n"
         ".axis{stroke:#999;stroke-width:1;} .grid{stroke:#eee;stroke-width:1;}\n"
         ".trace{stroke:#0577b1;stroke-width:1.3;fill:none;}\n"
         ".base{stroke:#2e8b57;stroke-width:1;stroke-dasharray:4 3;}\n"
         ".pk{stroke:#c62828;stroke-width:1.5;} .pkneg{stroke:#c05a00;stroke-width:1.5;}\n"
         ".lbl{font:11px sans-serif;fill:#333;} .axlbl{font:10px sans-serif;fill:#666;}\n"
         "@media print{body{margin:0;} svg{border:none;}}\n"
         "</style></head><body>\n"
      << "<h1>GC301c Gas Chromatograph &mdash; WPEAK64 Peak Report</h1>\n"
      << "<div class=\"meta\">Generated " << tbuf
      << " &middot; Noise=" << noise << " Baseline=" << baseline
      << " &middot; " << (m.detect_meth == 0 ? "height" : "area") << " method</div>\n";

    f << "<table><thead><tr><th>Num</th><th>Component</th><th>RT (s)</th>"
         "<th>Height</th><th>Area</th><th>From (s)</th><th>To (s)</th>"
         "<th>Concentration</th><th>Type</th><th>Alarm</th></tr></thead><tbody>\n";
    char buf[256];
    for(const ReportRow &r : rows) {
        const Peak &p = r.peak;
        bool neg = p.Height < 0;
        std::string conc = "-";
        if(r.calibrated) { std::snprintf(buf, sizeof buf, "%g", r.concentration); conc = buf; }
        const char *alarm = r.alarm == ALARM_NONE ? "" :
                            r.alarm == ALARM_HIGH ? "HIGH" :
                            r.alarm == ALARM_LOW  ? "LOW"  : "HIGH+LOW";
        std::string cls = std::string(neg ? "neg " : "") + (r.alarm ? "alarm" : "");
        f << "<tr class=\"" << cls << "\"><td>" << p.Num
          << "</td><td>" << HtmlEscape(r.name) << "</td><td>";
        std::snprintf(buf, sizeof buf, "%.1f", (double)p.Time / m.data_rate); f << buf;
        f << "</td><td>" << p.Height << "</td><td>";
        std::snprintf(buf, sizeof buf, "%.0f", p.Area); f << buf;
        f << "</td><td>";
        std::snprintf(buf, sizeof buf, "%.1f", (double)p.From / m.data_rate); f << buf;
        f << "</td><td>";
        std::snprintf(buf, sizeof buf, "%.1f", (double)p.To / m.data_rate); f << buf;
        f << "</td><td>" << conc << "</td><td>" << (neg ? "negative" : "positive")
          << "</td><td>" << alarm << "</td></tr>\n";
    }
    f << "</tbody></table>\n";

    if(trace.size() >= 2) {
        const int W = 900, H = 320, ML = 55, MR = 15, MT = 15, MB = 35;
        long ymin = trace[0], ymax = trace[0];
        for(long v : trace) { if(v < ymin) ymin = v; if(v > ymax) ymax = v; }
        if(baseline) { if(baseline < ymin) ymin = baseline; if(baseline > ymax) ymax = baseline; }
        long yspan = ymax - ymin; if(yspan < 1) yspan = 1;
        ymin -= yspan / 10; ymax += yspan / 10; yspan = ymax - ymin;
        const long n = (long)trace.size();
        auto X = [&](double i) { return ML + (W - ML - MR) * i / (n - 1); };
        auto Y = [&](double v) { return H - MB - (H - MT - MB) * (v - ymin) / yspan; };

        f << "<svg viewBox=\"0 0 " << W << " " << H << "\" width=\"" << W << "\" height=\"" << H << "\">\n";
        // gridlines + y ticks
        for(int t2 = 0; t2 <= 5; t2++) {
            double v = ymin + (double)yspan * t2 / 5;
            double yy = Y(v);
            std::snprintf(buf, sizeof buf,
                "<line class=\"grid\" x1=\"%d\" y1=\"%.1f\" x2=\"%d\" y2=\"%.1f\"/>"
                "<text class=\"axlbl\" x=\"%d\" y=\"%.1f\" text-anchor=\"end\">%ld</text>\n",
                ML, yy, W - MR, yy, ML - 4, yy + 3, (long)v);
            f << buf;
        }
        // x ticks
        long total_s = n / (m.data_rate > 0 ? m.data_rate : 1);
        long step = total_s > 0 ? (total_s + 5) / 6 : 1; if(step < 1) step = 1;
        for(long s = 0; s <= total_s; s += step) {
            double x = X((double)s * m.data_rate);
            std::snprintf(buf, sizeof buf,
                "<text class=\"axlbl\" x=\"%.1f\" y=\"%d\" text-anchor=\"middle\">%lds</text>\n",
                x, H - MB + 14, s);
            f << buf;
        }
        if(baseline) {
            std::snprintf(buf, sizeof buf, "<line class=\"base\" x1=\"%d\" y1=\"%.1f\" x2=\"%d\" y2=\"%.1f\"/>\n",
                          ML, Y((double)baseline), W - MR, Y((double)baseline));
            f << buf;
        }
        // trace polyline
        f << "<polyline class=\"trace\" points=\"";
        for(long i = 0; i < n; i++) {
            std::snprintf(buf, sizeof buf, "%.1f,%.1f ", X((double)i), Y((double)trace[(size_t)i]));
            f << buf;
        }
        f << "\"/>\n";
        // peak markers/labels
        for(const ReportRow &r : rows) {
            const Peak &p = r.peak;
            bool neg = p.Height < 0;
            double xm = X((double)p.Time), yb = Y((double)baseline);
            double yap = Y((double)(baseline + p.Height));
            std::snprintf(buf, sizeof buf,
                "<line class=\"%s\" x1=\"%.1f\" y1=\"%.1f\" x2=\"%.1f\" y2=\"%.1f\"/>\n",
                neg ? "pkneg" : "pk", xm, yb, xm, yap);
            f << buf;
            std::string lbl = neg ? "NEG " + std::to_string(p.Num)
                                   : (r.component >= 0 ? r.name : std::to_string(p.Num));
            std::snprintf(buf, sizeof buf,
                "<text class=\"lbl\" x=\"%.1f\" y=\"%.1f\" text-anchor=\"middle\">%s</text>\n",
                xm, yap - 4, HtmlEscape(lbl).c_str());
            f << buf;
        }
        f << "</svg>\n";
    }

    f << "</body></html>\n";
    return (bool)f;
}

} // namespace wpeak64
