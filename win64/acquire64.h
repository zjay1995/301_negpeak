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
};

class AcquireRun {
public:
    // standard_num: 0 = normal run; 1..STAND_NUM64 = calibration run for that
    // standard (samples through the cal valve; peaks update Component::cal).
    AcquireRun(const Method &m, Hardware &h, int standard_num = 0);

    // Execute the whole sequence. verbose prints phase transitions and zone
    // temperatures. Returns false and fills err on failure.
    bool Run(AcquireResult &out, bool verbose, std::string &err);

    // For a calibration run: fold measured responses into m_out.cal (matched
    // by CheckRT), using the component's standN concentration.
    void UpdateCalibration(Method &m_out, const AcquireResult &res) const;

private:
    const Method &m_;
    Hardware &h_;
    int standard_num_;

    void ServiceTempZones();
    bool ZonesInBand() const;
    double ZoneTempC(const TempZone &z) const;
    void Tick(double dt);            // sleep or advance simulation
    void AllOff();
};

} // namespace wpeak64

#endif // ACQUIRE64_H
