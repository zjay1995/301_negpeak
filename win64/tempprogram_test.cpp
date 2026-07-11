// tempprogram_test.cpp -- unit test for the oven temperature program
// calculation (tempprogram.h), no hardware/acquisition involved.

#include "tempprogram.h"
#include <cstdio>
#include <cmath>

using namespace wpeak64;

static int g_fail = 0;

static void Check(bool cond, const char *what)
{
    std::printf("%s: %s\n", cond ? "ok" : "FAIL", what);
    if(!cond) g_fail++;
}

static bool Near(double a, double b, double eps = 1e-6) { return std::fabs(a - b) < eps; }

int main()
{
    TempProgram disabled;
    Check(!disabled.Enabled(), "default-constructed program is disabled");
    Check(ProgramTarget(disabled, 0) == 0, "disabled program returns 0 (caller falls back to setpoint_c)");

    // 40C hold 60s -> ramp to 100C at 10C/min -> hold 30s -> ramp to 250C at
    // 20C/min -> final hold at 250C.
    TempProgram p;
    p.initial_temp_c = 40; p.initial_hold_s = 60;
    p.ramp1_rate_c_min = 10; p.temp2_c = 100; p.hold2_s = 30;
    p.ramp2_rate_c_min = 20; p.temp3_c = 250;
    Check(p.Enabled(), "configured program is enabled");

    Check(ProgramTarget(p, -1) == 40, "before injection: target is the initial hold temperature");
    Check(ProgramTarget(p, 0) == 40, "t=0: still in the initial hold");
    Check(ProgramTarget(p, 60) == 40, "t=hold1 end: still 40C (ramp1 hasn't started)");

    // ramp1: 40C -> 100C at 10C/min = 360s
    double ramp1_s = (100.0 - 40.0) / 10.0 * 60.0;
    Check(Near(ProgramTarget(p, 60), 40.0), "ramp1 start temperature is 40C");
    Check(Near(ProgramTarget(p, 60 + ramp1_s), 100.0), "ramp1 end temperature is 100C");
    Check(Near(ProgramTarget(p, 60 + ramp1_s / 2), 70.0), "ramp1 midpoint is 70C (halfway 40->100)");

    double hold2_start = 60 + ramp1_s;
    Check(Near(ProgramTarget(p, hold2_start), 100.0), "hold2 start is 100C");
    Check(Near(ProgramTarget(p, hold2_start + 30), 100.0), "hold2 end is still 100C");

    double ramp2_s = (250.0 - 100.0) / 20.0 * 60.0;   // 450s
    double ramp2_start = hold2_start + 30;
    Check(Near(ProgramTarget(p, ramp2_start), 100.0), "ramp2 start temperature is 100C");
    Check(Near(ProgramTarget(p, ramp2_start + ramp2_s), 250.0), "ramp2 end temperature is 250C");
    Check(Near(ProgramTarget(p, ramp2_start + ramp2_s / 2), 175.0), "ramp2 midpoint is 175C (halfway 100->250)");
    Check(Near(ProgramTarget(p, ramp2_start + ramp2_s + 999999), 250.0),
         "final hold stays at 250C indefinitely");

    // cooling ramp (temp2 < initial_temp_c) is handled too, not just heating
    TempProgram cool;
    cool.initial_temp_c = 200; cool.initial_hold_s = 10;
    cool.ramp1_rate_c_min = 5; cool.temp2_c = 100;   // cools from 200 to 100
    Check(Near(ProgramTarget(cool, 10), 200.0), "cooling ramp starts at the initial (higher) temp");
    double cool_ramp_s = (200.0 - 100.0) / 5.0 * 60.0;
    Check(Near(ProgramTarget(cool, 10 + cool_ramp_s), 100.0), "cooling ramp reaches the lower target");
    Check(Near(ProgramTarget(cool, 10 + cool_ramp_s / 2), 150.0), "cooling ramp midpoint is between the two temps");

    // no ramp1 configured (rate 0): jumps straight to hold2 after hold1
    TempProgram noramp;
    noramp.initial_temp_c = 50; noramp.initial_hold_s = 20;
    noramp.temp2_c = 80; noramp.hold2_s = 10;   // ramp1_rate_c_min left at 0
    Check(noramp.Enabled(), "program with only a hold time is still enabled");
    Check(Near(ProgramTarget(noramp, 20), 80.0), "rate1=0 jumps straight to hold2's temperature");

    // isothermal: only an initial hold configured, nothing past it --
    // should hold at initial_temp_c forever, not fall through to 0.
    TempProgram iso;
    iso.initial_temp_c = 60; iso.initial_hold_s = 30;
    Check(iso.Enabled(), "hold-only program is enabled");
    Check(Near(ProgramTarget(iso, 30), 60.0), "isothermal program holds at its temperature right after the initial hold");
    Check(Near(ProgramTarget(iso, 999999), 60.0), "isothermal program holds forever, not just at the boundary");

    // cooling ramp with no third stage configured: should hold at the
    // ramp1 target (100C) instead of falling through to 0 -- this is
    // exactly the bug the cascading "last" fix above addresses.
    Check(Near(ProgramTarget(cool, 10 + cool_ramp_s + 999999), 100.0),
         "cooling program with no hold3 holds at the ramp1 target, not 0");

    std::printf("\n%s\n", g_fail ? "SOME CHECKS FAILED" : "all checks passed");
    return g_fail ? 1 : 0;
}
