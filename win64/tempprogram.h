// tempprogram.h -- GC oven temperature programming for WPEAK64.
//
// Ports the legacy TCTRL.CPP TempControl state machine (T_INIT -> T_HOLD1 ->
// T_RAMP1 -> T_HOLD2 -> T_RAMP2 -> T_HOLD3): an optional "hold, ramp, hold,
// ramp, hold" temperature profile layered on top of a zone's flat setpoint,
// so a run can start the oven cool (better resolution for early, volatile
// peaks) and ramp it up during the run (elutes late, high-boiling
// components faster) -- standard GC practice, and a real capability gap
// against a single fixed setpoint.
//
// Pure calculation, no hardware -- unit-testable without a real thermal
// system (see tempprogram_test.cpp). AcquireRun (acquire64.cpp) is the only
// caller: it feeds elapsed seconds since injection so ServiceTempZones/
// ZonesInBand track the moving target instead of TempZone::setpoint_c.

#ifndef TEMPPROGRAM_H
#define TEMPPROGRAM_H

#include <cmath>

namespace wpeak64 {

struct TempProgram {
    double initial_temp_c   = 0;   // T_HOLD1 setpoint
    long   initial_hold_s   = 0;   // T_HOLD1 duration
    double ramp1_rate_c_min = 0;   // deg C / min, either sign; 0 = no ramp1
    double temp2_c          = 0;   // T_HOLD2 setpoint (ramp1 target)
    long   hold2_s          = 0;   // T_HOLD2 duration
    double ramp2_rate_c_min = 0;   // deg C / min, either sign; 0 = no ramp2
    double temp3_c          = 0;   // T_HOLD3 (final) setpoint, held indefinitely

    bool Enabled() const { return initial_hold_s > 0 || ramp1_rate_c_min != 0; }
};

// Current target temperature for a zone running this program.
// elapsed_s < 0 means "before injection, program hasn't started" -- returns
// the initial hold setpoint, so the oven equilibrates there first (legacy
// T_INIT/T_HOLD1 semantics). elapsed_s >= 0 walks HOLD1 -> RAMP1 -> HOLD2
// -> RAMP2 -> HOLD3, holding at temp3_c indefinitely once reached.
// Returns 0 (caller falls back to the zone's static setpoint) when the
// program isn't enabled.
inline double ProgramTarget(const TempProgram &p, double elapsed_s)
{
    if(!p.Enabled()) return 0;
    if(elapsed_s < 0) return p.initial_temp_c;

    // "last" cascades forward through unconfigured stages (temp2_c/temp3_c
    // left at 0, or a ramp rate left at 0) so an isothermal or single-ramp
    // program just holds at the last real setpoint instead of falling
    // through to 0 -- a two-segment program needs only initial_temp_c/
    // initial_hold_s/ramp1_rate_c_min/temp2_c, not every field filled in.
    double last = p.initial_temp_c;
    double t = elapsed_s;
    if(t < (double)p.initial_hold_s) return last;
    t -= p.initial_hold_s;

    double temp2 = p.temp2_c > 0 ? p.temp2_c : last;
    double ramp1_s = p.ramp1_rate_c_min != 0
        ? std::fabs(temp2 - last) / std::fabs(p.ramp1_rate_c_min) * 60.0 : 0.0;
    if(t < ramp1_s) {
        double dir = temp2 >= last ? 1.0 : -1.0;
        return last + dir * std::fabs(p.ramp1_rate_c_min) / 60.0 * t;
    }
    t -= ramp1_s;
    last = temp2;

    if(t < (double)p.hold2_s) return last;
    t -= p.hold2_s;

    double temp3 = p.temp3_c > 0 ? p.temp3_c : last;
    double ramp2_s = p.ramp2_rate_c_min != 0
        ? std::fabs(temp3 - last) / std::fabs(p.ramp2_rate_c_min) * 60.0 : 0.0;
    if(t < ramp2_s) {
        double dir = temp3 >= last ? 1.0 : -1.0;
        return last + dir * std::fabs(p.ramp2_rate_c_min) / 60.0 * t;
    }
    return temp3;
}

} // namespace wpeak64

#endif // TEMPPROGRAM_H
