// wpeak64_acq.cpp -- acquisition & instrument-control front end for WPEAK64.
//
// Usage:
//   wpeak64_acq -m method.ini run     [-C cal.ini] [-o report.csv] [-D trace.csv]
//   wpeak64_acq -m method.ini cal -s N [-C cal.ini]
//   wpeak64_acq -m method.ini monitor [-t seconds]
//
//   run      full sequence (equilibrate/sample/inject/analyze/purge),
//            report identified peaks with concentrations
//   cal      same sequence sampling through the calibration valve; measured
//            responses of matched components update standard N (1..8) in the
//            calibration file
//   monitor  print ADC channels, zone temperatures and output states
//
//   -m  method file (detector, components+standards, hardware, tempzones, timing)
//   -C  calibration file to read/update (default: cal.ini next to nothing)
//   -o  write the peak report CSV
//   -D  write the acquired chromatogram CSV (reusable with wpeak64_cli -d)
//   --sim         force the simulation backend regardless of the method
//   --sim-scale X scale the simulated detector peaks (default 1.0)
//
// Hardware: ADS1115 ADC via Linux i2c-dev, control lines via Linux gpiochip.
// backend=sim runs everywhere (including Windows) with a modeled instrument.

#include "method64.h"
#include "acquire64.h"
#include <cstdio>
#include <cstring>
#include <string>
#include <fstream>
#include <thread>
#include <chrono>

using namespace wpeak64;

static int Monitor(const Method &m, Hardware &h, int seconds)
{
    for(int s = 0; s < seconds; s++) {
        long det = h.adc->ReadCounts(m.hw.adc_channel);
        std::printf("t=%3ds  det[ch%d]=%6ld (%.4f V)", s, m.hw.adc_channel,
                    det, h.adc->CountsToVolts(det));
        for(const TempZone &z : m.zones) {
            long c = h.adc->ReadCounts(z.adc_channel);
            double t = z.scale * h.adc->CountsToVolts(c) + z.offset;
            std::printf("  %s=%.1fC(heat=%d)", z.name.c_str(), t,
                        z.heater_line >= 0 ? (int)h.out->Get(z.heater_line) : 0);
        }
        std::printf("\n");
        if(h.sim) h.sim->AdvanceSeconds(1.0);
        else      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return 0;
}

int main(int argc, char **argv)
{
    std::string method_path, cal_path, report_path, trace_path, mode;
    int standard_num = 0, monitor_s = 10;
    bool force_sim = false;
    double sim_scale = 1.0;

    for(int i = 1; i < argc; i++) {
        auto need = [&](const char *opt) -> const char * {
            if(i + 1 >= argc) { std::fprintf(stderr, "missing argument for %s\n", opt); std::exit(2); }
            return argv[++i];
        };
        if     (!std::strcmp(argv[i], "-m"))  method_path = need("-m");
        else if(!std::strcmp(argv[i], "-C"))  cal_path    = need("-C");
        else if(!std::strcmp(argv[i], "-o"))  report_path = need("-o");
        else if(!std::strcmp(argv[i], "-D"))  trace_path  = need("-D");
        else if(!std::strcmp(argv[i], "-s"))  standard_num = std::atoi(need("-s"));
        else if(!std::strcmp(argv[i], "-t"))  monitor_s   = std::atoi(need("-t"));
        else if(!std::strcmp(argv[i], "--sim"))       force_sim = true;
        else if(!std::strcmp(argv[i], "--sim-scale")) sim_scale = std::atof(need("--sim-scale"));
        else if(argv[i][0] != '-' && mode.empty())    mode = argv[i];
        else {
            std::fprintf(stderr, "unknown argument: %s\n", argv[i]);
            return 2;
        }
    }
    if(method_path.empty() || (mode != "run" && mode != "cal" && mode != "monitor")) {
        std::fprintf(stderr,
            "usage: %s -m method.ini run|cal|monitor [options]\n"
            "  run      [-C cal.ini] [-o report.csv] [-D trace.csv]\n"
            "  cal      -s N [-C cal.ini]     (N = standard number 1..%d)\n"
            "  monitor  [-t seconds]\n"
            "  --sim / --sim-scale X  simulation backend\n",
            argv[0], STAND_NUM64);
        return 2;
    }

    Method m;
    std::string err;
    if(!LoadMethod(method_path, m, err)) {
        std::fprintf(stderr, "error: %s\n", err.c_str());
        return 2;
    }
    if(force_sim) m.hw.backend = "sim";
    if(!cal_path.empty()) {
        // calibration file is optional on first cal run
        std::string cal_err;
        if(!LoadCalibration(cal_path, m, cal_err)) {
            std::ifstream probe(cal_path);
            if(probe) { std::fprintf(stderr, "error: %s\n", cal_err.c_str()); return 2; }
        }
    }

    Hardware h;
    if(!OpenHardware(m.hw, m.zones, sim_scale, h, err)) {
        std::fprintf(stderr, "error: %s\n", err.c_str());
        return 2;
    }

    int rc = 0;
    if(mode == "monitor") {
        rc = Monitor(m, h, monitor_s);
    }
    else {
        if(mode == "cal" && (standard_num < 1 || standard_num > STAND_NUM64)) {
            std::fprintf(stderr, "error: cal requires -s 1..%d\n", STAND_NUM64);
            CloseHardware(h);
            return 2;
        }
        AcquireRun run(m, h, mode == "cal" ? standard_num : 0);
        AcquireResult res;
        if(!run.Run(res, true, err)) {
            std::fprintf(stderr, "error: %s\n", err.c_str());
            CloseHardware(h);
            return 1;
        }

        std::printf("\nNoise=%ld  Baseline=%ld\n", res.noise, res.baseline);
        std::printf("%-5s %-14s %-8s %-9s %-11s %-12s %s\n",
                    "Num", "Component", "RT (s)", "Height", "Area", "Concentr.", "Type");
        for(const ReportRow &r : res.rows) {
            const Peak &p = r.peak;
            bool neg = p.Height < 0;
            char num[8], conc[24];
            if(neg) std::snprintf(num, sizeof num, "-");
            else    std::snprintf(num, sizeof num, "%d", p.Num);
            if(r.calibrated) std::snprintf(conc, sizeof conc, "%g", r.concentration);
            else             std::snprintf(conc, sizeof conc, "%s", neg ? "-" : "n/cal");
            std::printf("%-5s %-14s %-8.1f %-9ld %-11.0f %-12s %s\n",
                        num, r.name.c_str(), (double)p.Time / m.data_rate,
                        p.Height, neg ? 0.0 : p.Area, conc,
                        neg ? "NEGATIVE (not quantified)" : "positive");
        }

        if(mode == "cal") {
            run.UpdateCalibration(m, res);
            std::string out_cal = cal_path.empty() ? "cal.ini" : cal_path;
            if(!SaveCalibration(out_cal, m, err)) {
                std::fprintf(stderr, "error: %s\n", err.c_str());
                rc = 1;
            }
            else
                std::printf("\nCalibration standard %d stored in %s\n",
                            standard_num, out_cal.c_str());
        }
        if(!report_path.empty() &&
           !WriteReportCsv(report_path, res.rows, m, res.noise, res.baseline, err)) {
            std::fprintf(stderr, "error: %s\n", err.c_str());
            rc = 1;
        }
        if(!trace_path.empty()) {
            std::ofstream tf(trace_path);
            tf << "time_s,counts\n";
            for(size_t i = 0; i < res.trace.size(); i++)
                tf << (double)i / m.data_rate << "," << res.trace[i] << "\n";
        }
    }

    CloseHardware(h);
    return rc;
}
