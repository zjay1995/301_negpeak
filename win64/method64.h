// method64.h -- Method / component table / data file support for WPEAK64.
// Ports the parts of METHOD.H / CMPONENT.H / CALC.CPP (CheckRT, PeakMatchup,
// Concentration) that identify peaks and produce a report. The legacy binary
// method format is replaced by a plain INI-style text file; chromatograms are
// read from CSV.

#ifndef METHOD64_H
#define METHOD64_H

#include "peak64.h"
#include <string>
#include <vector>

namespace wpeak64 {

// Component (from CMPONENT.H): a named compound expected at a retention time.
struct Component {
    std::string name;
    double peak_rt   = 0;   // expected retention time, s
    double window    = 0;   // RT match window, +/- s
    double response  = 0;   // response factor: concentration = response * height|area
                            // (0 = report "not calibrated")
    bool   active_yn = true;
};

// Method: detector settings + run parameters + component table.
struct Method {
    DetectorSettings det;
    int    data_rate     = 10;    // points per second
    long   analysis_time = 120;   // s
    int    detect_meth   = 0;     // 0 = height, 1 = area (Detector::detect_meth)
    bool   known_peaks   = false; // report matched (known) peaks only
    std::vector<Component> components;
};

// A reported peak: the detected Peak plus identification results.
struct ReportRow {
    Peak        peak;
    int         component  = -1;     // index into Method::components, -1 = unknown
    std::string name;                // component name or "unknown"
    double      concentration = 0;   // 0 when not calibrated / unknown / negative
    bool        calibrated = false;
};

// ---- Method file (INI) ------------------------------------------------------
// Example:
//   [detector]
//   segment_width=4
//   nandb_time=2
//   nandb_len=10
//   min_height=20
//   min_area=0
//   peak_alg=0
//   detect_meth=0
//   known_peaks=0
//   data_rate=10
//   analysis_time=120
//   [component]
//   name=Benzene
//   rt=40
//   window=5
//   response=0.001
//   [component]
//   ...
// Returns false and fills err on parse failure.
bool LoadMethod(const std::string &path, Method &m, std::string &err);

// ---- Chromatogram CSV -------------------------------------------------------
// Accepts one sample per line: either "value" or "time,value" (time ignored --
// the method's data_rate defines timing, as in the original raw data files).
// '#' or ';' comment lines and a non-numeric header line are skipped.
bool LoadChromatogram(const std::string &path, std::vector<long> &y, std::string &err);

// ---- Identification / report ------------------------------------------------
// CheckRT (CALC.CPP): peak matches component when
//   |peak.Time/data_rate - component.peak_rt| <= component.window.
bool CheckRT(const Peak &peak, const Component &c, int data_rate);

// PeakMatchup semantics: first active matching component wins; unknown peaks
// are kept unless method.known_peaks is set. Negative peaks are always kept
// but never identified/quantified. Concentration = response * height or area
// per detect_meth, when a response factor is configured.
std::vector<ReportRow> BuildReport(const std::vector<Peak> &peaks, const Method &m);

// Write the report as CSV. Returns false and fills err on I/O failure.
bool WriteReportCsv(const std::string &path, const std::vector<ReportRow> &rows,
                    const Method &m, long noise, long baseline, std::string &err);

} // namespace wpeak64

#endif // METHOD64_H
