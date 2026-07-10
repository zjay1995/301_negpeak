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
            else if(section == "tempzone") {
                m.zones.push_back(TempZone());
                zone = &m.zones.back();
            }
            else if(section != "detector" && section != "hardware" && section != "timing") {
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
            if     (key == "segment_width") m.det.segment_width = std::atoi(val.c_str());
            else if(key == "nandb_time")    m.det.NandBtime     = std::atol(val.c_str());
            else if(key == "nandb_len")     m.det.NandBlen      = std::atol(val.c_str());
            else if(key == "min_height")    m.det.MinHeight     = std::atol(val.c_str());
            else if(key == "min_area")      m.det.MinArea       = std::atof(val.c_str());
            else if(key == "peak_alg")      m.det.peak_alg      = std::atoi(val.c_str());
            else if(key == "noise_reduct")  m.det.noise_reduct  = std::atoi(val.c_str());
            else if(key == "detect_meth")   m.detect_meth       = std::atoi(val.c_str());
            else if(key == "known_peaks")   m.known_peaks       = std::atoi(val.c_str()) != 0;
            else if(key == "data_rate")     m.data_rate         = std::atoi(val.c_str());
            else if(key == "analysis_time") m.analysis_time     = std::atol(val.c_str());
            else { err = path + ":" + std::to_string(lineno) + ": unknown detector key '" + key + "'"; return false; }
        }
        else if(section == "component" && cur) {
            if     (key == "name")     cur->name      = val;
            else if(key == "rt")       cur->peak_rt   = std::atof(val.c_str());
            else if(key == "window")   cur->window    = std::atof(val.c_str());
            else if(key == "response") cur->response  = std::atof(val.c_str());
            else if(key == "active")   cur->active_yn = std::atoi(val.c_str()) != 0;
            else if(key == "alarm_high") cur->alarm_high = std::atof(val.c_str());
            else if(key == "alarm_low")  cur->alarm_low  = std::atof(val.c_str());
            else if(key.size() == 4 && key.compare(0, 3, "std") == 0 &&
                    key[3] >= '1' && key[3] <= '0' + STAND_NUM64) {
                cur->stand[key[3] - '1'] = std::atof(val.c_str());   // std1..std8
            }
            else { err = path + ":" + std::to_string(lineno) + ": unknown component key '" + key + "'"; return false; }
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
    for(const Component &c : m.components) {
        bool any = false;
        for(int i = 0; i < STAND_NUM64; i++)
            if(c.cal[i].valid) any = true;
        if(!any) continue;
        f << "\n[calibration]\ncomponent=" << c.name << "\n";
        char buf[96];
        for(int i = 0; i < STAND_NUM64; i++) {
            if(!c.cal[i].valid) continue;
            std::snprintf(buf, sizeof buf, "std%d=%g,%g\n",
                          i + 1, c.cal[i].conc, c.cal[i].resp);
            f << buf;
        }
    }
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
        std::snprintf(buf, sizeof buf, "%s,%s,%.1f,%ld,%.0f,%.1f,%.1f,%s,%s,%s\n",
                      neg ? "-" : std::to_string(p.Num).c_str(),
                      r.name.c_str(),
                      (double)p.Time / m.data_rate,
                      p.Height,
                      neg ? 0.0 : p.Area,
                      (double)p.From / m.data_rate,
                      (double)p.To   / m.data_rate,
                      conc.c_str(),
                      neg ? "negative" : "positive",
                      alarm);
        f << buf;
    }
    return (bool)f;
}

} // namespace wpeak64
