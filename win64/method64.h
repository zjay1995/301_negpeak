// method64.h -- Method / component table / data file support for WPEAK64.
// Ports the parts of METHOD.H / CMPONENT.H / CALC.CPP (CheckRT, PeakMatchup,
// Concentration) that identify peaks and produce a report. The legacy binary
// method format is replaced by a plain INI-style text file; chromatograms are
// read from CSV.

#ifndef METHOD64_H
#define METHOD64_H

#include "peak64.h"
#include "hw64.h"
#include <string>
#include <vector>

namespace wpeak64 {

enum { STAND_NUM64 = 8 };   // max calibration standards per component

// One calibration point: known standard concentration vs measured response
// (height or area per detect_meth). Mirrors one column of the legacy
// CalRes[COMPONENT_ROWS][STAND_NUM] table.
struct CalPoint {
    double conc = 0;
    double resp = 0;
    bool   valid = false;
};

// Component (from CMPONENT.H): a named compound expected at a retention time.
struct Component {
    std::string name;
    double peak_rt   = 0;   // expected retention time, s
    double window    = 0;   // RT match window, +/- s
    double response  = 0;   // fixed response factor fallback when no
                            // calibration table exists (0 = "not calibrated")
    bool   active_yn = true;
    double stand[STAND_NUM64] = {0};      // standard concentrations (std1..std8)
    CalPoint cal[STAND_NUM64];            // measured calibration table
    // concentration alarm limits (legacy H/L alarms; 0 = disabled)
    double alarm_high = 0;
    double alarm_low  = 0;
    // analog concentration output (legacy Write_conc_to_DAC / AnalogRange):
    // dac_channel indexes HardwareConfig::dac_i2c_addrs; dac_range is the
    // concentration that maps to full-scale DAC output. -1 / 0 = no output.
    int    dac_channel = -1;
    double dac_range   = 0;
};

// Method: detector settings + run parameters + component table + hardware.
// "Detector A" is the fields below at the top level, unchanged from the
// original single-detector port. Detector B (legacy NUMDETECTORS=2 -- a
// second, fully independent detector with its own settings, component
// table and ADC channel, sampled in the same run) is optional and purely
// additive: see det_b_enabled and AsDetectorB() below.
struct Method {
    DetectorSettings det;
    int    data_rate     = 10;    // points per second
    long   analysis_time = 120;   // s
    int    detect_meth   = 0;     // 0 = height, 1 = area (Detector::detect_meth)
    bool   known_peaks   = false; // report matched (known) peaks only
    std::vector<Component> components;

    HardwareConfig        hw;     // [hardware]
    std::vector<TempZone> zones;  // [tempzone] sections
    TimingConfig          timing; // [timing]

    // ---- Detector B (optional second channel, [detector_b]/[component_b]) --
    bool   det_b_enabled     = false;
    int    det_b_adc_channel = -1;    // ADC channel for detector B's signal
    DetectorSettings det_b;
    int    det_b_detect_meth = 0;
    bool   det_b_known_peaks = false;
    std::vector<Component> components_b;

    // A Method-shaped view of detector B's settings, for reuse with
    // BuildReport/EvaluateAlarms/ConcentrationFromCal/WriteReportCsv (which
    // all read det/components/detect_meth/known_peaks from a Method) without
    // duplicating that logic for a second detector.
    Method AsDetectorB() const
    {
        Method m = *this;
        m.det          = det_b;
        m.components   = components_b;
        m.detect_meth  = det_b_detect_meth;
        m.known_peaks  = det_b_known_peaks;
        return m;
    }
};

// A reported peak: the detected Peak plus identification results.
enum AlarmState { ALARM_NONE = 0, ALARM_HIGH = 1, ALARM_LOW = 2 };

struct ReportRow {
    Peak        peak;
    int         component  = -1;     // index into Method::components, -1 = unknown
    std::string name;                // component name or "unknown"
    double      concentration = 0;   // 0 when not calibrated / unknown / negative
    bool        calibrated = false;
    int         alarm = ALARM_NONE;  // legacy H/L concentration alarms
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

// Write a Method back to the INI format LoadMethod reads (all sections:
// detector, components, hardware, tempzones, timing). Calibration data is
// NOT written here -- it lives in the calibration file (SaveCalibration).
bool SaveMethod(const std::string &path, const Method &m, std::string &err);

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
// but never identified/quantified. Concentration comes from the component's
// multipoint calibration table when present (legacy CalcConcVars /
// Detector::Concentration piecewise-linear interpolation), otherwise from
// the fixed response factor.
std::vector<ReportRow> BuildReport(const std::vector<Peak> &peaks, const Method &m);

// Multipoint concentration (piecewise-linear, legacy semantics):
// points sorted by response; below the first standard and above the last
// one, the line through the origin and that standard is used. Returns false
// when the component has no valid calibration points.
bool ConcentrationFromCal(const Component &c, double response, double &conc);

// ---- Calibration file -------------------------------------------------------
// INI with one [calibration] section per component:
//   [calibration]
//   component=Benzene
//   std1=1.0,394        ; concentration,measured response
//   std3=5.0,1980
// Loads into / saves from Component::cal of a loaded Method (matched by
// component name; unknown names are an error on load, skipped on save).
bool LoadCalibration(const std::string &path, Method &m, std::string &err);
bool SaveCalibration(const std::string &path, const Method &m, std::string &err);

// ---- Alarms -------------------------------------------------------------------
// Evaluate the H/L concentration alarms (legacy GetAlarmFlags): a calibrated,
// identified peak trips HIGH when conc >= alarm_high (if set) and LOW when
// conc <= alarm_low (if set). Fills ReportRow::alarm; returns the OR of all
// alarm states so the caller can drive the alarm relays.
int EvaluateAlarms(std::vector<ReportRow> &rows, const Method &m);

// Write the report as CSV. Returns false and fills err on I/O failure.
bool WriteReportCsv(const std::string &path, const std::vector<ReportRow> &rows,
                    const Method &m, long noise, long baseline, std::string &err);

} // namespace wpeak64

#endif // METHOD64_H
