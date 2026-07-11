// acquire64.h -- acquisition run sequencer for WPEAK64.
//
// Ports the run flow of ACQUIRE.CPP to the ADS1115/GPIO hardware layer:
//
//   EQUILIBRATE  heaters enabled, wait until every temperature zone is
//                within its hysteresis band (at least equil_time seconds)
//   SAMPLE       pump + sample valve (or cal valve for a calibration run)
//                energized for sample_time
//   INJECT       injection valve energized for inject_time, lamp on,
//                retention clock starts
//   ANALYZE      detector sampled at data_rate, each point fed to the
//                Integrator (positive + negative peak detection live)
//   PURGE        purge valve for purge_time, everything else off
//
// Temperature zones run bang-bang control with hysteresis on every tick --
// the same on/off heater control the legacy oven used. In simulation the
// loop advances SimHardware's clock instead of sleeping, so a full run
// executes instantly; on real hardware it paces with the wall clock.

#ifndef ACQUIRE64_H
#define ACQUIRE64_H

#include "method64.h"
#include "hw64.h"

namespace wpeak64 {

struct AcquireResult {
    std::vector<long>      trace;    // acquired chromatogram (ADC counts)
    std::vector<ReportRow> rows;     // identified peaks
    long noise = 0, baseline = 0;
    std::vector<Peak> peaks;         // raw detected peaks
    int alarm_state = 0;             // OR of ReportRow alarms (AlarmState)

    // ---- Detector B (only populated when Method::det_b_enabled) -------------
    bool has_b = false;
    std::vector<long>      trace_b;
    std::vector<ReportRow> rows_b;
    long noise_b = 0, baseline_b = 0;
    std::vector<Peak> peaks_b;
    int alarm_state_b = 0;
};

// Observer for live displays (GUI acquisition window): called from the
// acquisition loop on phase changes and on every analyzed data point.
class AcquireProgress {
public:
    virtual ~AcquireProgress() {}
    virtual void OnPhase(const char *phase) { (void)phase; }
    virtual void OnPoint(long counts) { (void)counts; }
    virtual void OnPointB(long counts) { (void)counts; }
    virtual void OnZone(const char *name, double temp_c) { (void)name; (void)temp_c; }
    // Fired every ANALYZE tick with the detector's peak list as it stands so
    // far (Integrator::peaks -- a peak lands in it the instant it finishes,
    // so callers get identification results as they happen instead of only
    // at the end of the run) and the current baseline, so a live display can
    // draw/identify peaks while the trace is still growing.
    virtual void OnLivePeaks(const std::vector<Peak> &peaks, long baseline) {
        (void)peaks; (void)baseline;
    }
    virtual void OnLivePeaksB(const std::vector<Peak> &peaks, long baseline) {
        (void)peaks; (void)baseline;
    }
    // return true to abort the run between phases/samples
    virtual bool Aborted() { return false; }
};

class AcquireRun {
public:
    // standard_num: 0 = normal run; 1..STAND_NUM64 = calibration run for that
    // standard (samples through the cal valve; peaks update Component::cal).
    // point: 1-based sample point; with a point-valve manifold configured the
    // matching point valve is energized during SAMPLE instead of sample_valve.
    AcquireRun(const Method &m, Hardware &h, int standard_num = 0, int point = 1);

    // Execute the whole sequence. verbose prints phase transitions and zone
    // temperatures. Returns false and fills err on failure. After analysis
    // the H/L alarms are evaluated and the alarm relay lines updated (alarms
    // hold their state until the next run clears them).
    bool Run(AcquireResult &out, bool verbose, std::string &err,
             AcquireProgress *progress = nullptr);

    // For a calibration run: fold measured responses into m_out.cal (matched
    // by CheckRT), using the component's standN concentration.
    void UpdateCalibration(Method &m_out, const AcquireResult &res) const;

private:
    const Method &m_;
    Hardware &h_;
    int standard_num_;
    int point_;

    void ServiceTempZones();
    bool ZonesInBand() const;
    double ZoneTempC(const TempZone &z) const;
    void Tick(double dt);            // sleep or advance simulation
    void AllOff();
    void WriteDacOutputs(const Method &mm, const std::vector<ReportRow> &rows) const;
};

} // namespace wpeak64

#endif // ACQUIRE64_H
