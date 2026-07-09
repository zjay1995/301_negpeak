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
            else if(section != "detector") {
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
            else { err = path + ":" + std::to_string(lineno) + ": unknown component key '" + key + "'"; return false; }
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
                if(c.response != 0) {
                    double amount = m.detect_meth == 0 ? (double)p.Height : p.Area;
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

bool WriteReportCsv(const std::string &path, const std::vector<ReportRow> &rows,
                    const Method &m, long noise, long baseline, std::string &err)
{
    std::ofstream f(path);
    if(!f) { err = "cannot write report file: " + path; return false; }

    f << "# GC301c WPEAK64 peak report\n";
    f << "# noise=" << noise << " baseline=" << baseline
      << " detect_meth=" << (m.detect_meth == 0 ? "height" : "area") << "\n";
    f << "num,component,rt_s,height,area,from_s,to_s,concentration,type\n";
    char buf[256];
    for(const ReportRow &r : rows) {
        const Peak &p = r.peak;
        bool neg = p.Height < 0;
        std::string conc = "";
        if(r.calibrated) {
            std::snprintf(buf, sizeof buf, "%g", r.concentration);
            conc = buf;
        }
        std::snprintf(buf, sizeof buf, "%s,%s,%.1f,%ld,%.0f,%.1f,%.1f,%s,%s\n",
                      neg ? "-" : std::to_string(p.Num).c_str(),
                      r.name.c_str(),
                      (double)p.Time / m.data_rate,
                      p.Height,
                      neg ? 0.0 : p.Area,
                      (double)p.From / m.data_rate,
                      (double)p.To   / m.data_rate,
                      conc.c_str(),
                      neg ? "negative" : "positive");
        f << buf;
    }
    return (bool)f;
}

} // namespace wpeak64
