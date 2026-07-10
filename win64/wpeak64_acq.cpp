// wpeak64_acq.cpp -- acquisition & instrument-control front end for WPEAK64.
//
// Usage:
//   wpeak64_acq -m method.ini run        [-p N] [-C cal.ini] [-j jobdir] [-o report.csv] [-D trace.csv]
//   wpeak64_acq -m method.ini cal -s N   [-C cal.ini] [-j jobdir]
//   wpeak64_acq -m method.ini continuous [-C cal.ini] [-j jobdir] [-n maxruns]
//   wpeak64_acq -m method.ini monitor    [-t seconds]
//   wpeak64_acq history -j jobdir
//
//   run         one full sequence (equilibrate/sample/inject/analyze/purge)
//   cal         same sequence through the calibration valve; responses of
//               matched components update standard N (1..8) in the cal file
//   continuous  scheduled operation (legacy Continuous/Repeat modes): cycles
//               all sample points, waits [timing] repeat_interval between run
//               starts, auto-runs a calibration every auto_cal_every runs
//               (auto_cal_standard), persists everything to the job dir,
//               drives the alarm relays. -n limits the number of runs
//               (0 = until interrupted).
//   monitor     print ADC channels, zone temperatures and output states
//   history     print the run list of a job directory
//   twa         TWA / STEL exposure report over a job directory's stored
//               runs (legacy TWA_results / STEL_results): per point and
//               component the time-weighted average (mean over runs),
//               min/max, and STEL = highest mean over any window of up to
//               15 consecutive runs (legacy STEL_RUNS)
//
//   -m  method file   -C calibration file   -j job directory (run persistence)
//   -p  sample point number (multipoint manifold; default 1)
//   -o  report CSV    -D chromatogram CSV
//   --sim / --sim-scale X : simulated instrument
//
// Hardware: ADS1115 ADC via Linux i2c-dev, control lines via Linux gpiochip.
// backend=sim runs everywhere (including Windows) with a modeled instrument.

#include "method64.h"
#include "acquire64.h"
#include "runstore64.h"
#include <cstdio>
#include <cstring>
#include <csignal>
#include <string>
#include <fstream>
#include <thread>
#include <chrono>

using namespace wpeak64;

static volatile std::sig_atomic_t g_stop = 0;
static void OnSigInt(int) { g_stop = 1; }

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

static void PrintRows(const Method &m, const AcquireResult &res, int point)
{
    std::printf("\nPoint %d   Noise=%ld  Baseline=%ld%s\n", point,
                res.noise, res.baseline,
                res.alarm_state ? "   *** ALARM ***" : "");
    std::printf("%-5s %-14s %-8s %-9s %-11s %-12s %-6s %s\n",
                "Num", "Component", "RT (s)", "Height", "Area", "Concentr.",
                "Alarm", "Type");
    for(const ReportRow &r : res.rows) {
        const Peak &p = r.peak;
        bool neg = p.Height < 0;
        char num[8], conc[24];
        if(neg) std::snprintf(num, sizeof num, "-");
        else    std::snprintf(num, sizeof num, "%d", p.Num);
        if(r.calibrated) std::snprintf(conc, sizeof conc, "%g", r.concentration);
        else             std::snprintf(conc, sizeof conc, "%s", neg ? "-" : "n/cal");
        const char *alarm = r.alarm == ALARM_NONE ? "" :
                            r.alarm == ALARM_HIGH ? "HIGH" :
                            r.alarm == ALARM_LOW  ? "LOW"  : "H+L";
        std::printf("%-5s %-14s %-8.1f %-9ld %-11.0f %-12s %-6s %s\n",
                    num, r.name.c_str(), (double)p.Time / m.data_rate,
                    p.Height, neg ? 0.0 : p.Area, conc, alarm,
                    neg ? "NEGATIVE (not quantified)" : "positive");
    }
}

// abort mid-run on Ctrl+C
class StopCheck : public AcquireProgress {
public:
    bool Aborted() override { return g_stop != 0; }
};

// One acquisition (run or cal) with printing, persistence, cal update.
static int DoOneRun(Method &m, Hardware &h, bool is_cal, int standard,
                    int point, const std::string &cal_path,
                    const std::string &jobdir,
                    const std::string &report_path,
                    const std::string &trace_path)
{
    std::string err;
    StopCheck stop;
    AcquireRun run(m, h, is_cal ? standard : 0, point);
    AcquireResult res;
    if(!run.Run(res, true, err, &stop)) {
        std::fprintf(stderr, "error: %s\n", err.c_str());
        return 1;
    }
    PrintRows(m, res, point);

    int rc = 0;
    if(is_cal) {
        run.UpdateCalibration(m, res);
        std::string out_cal = cal_path.empty() ? "cal.ini" : cal_path;
        if(!SaveCalibration(out_cal, m, err)) {
            std::fprintf(stderr, "error: %s\n", err.c_str());
            rc = 1;
        }
        else
            std::printf("Calibration standard %d stored in %s\n",
                        standard, out_cal.c_str());
    }
    if(!jobdir.empty()) {
        RunRecord rec;
        if(!SaveRun(jobdir, m, res, is_cal ? "cal" : "run", point,
                    is_cal ? standard : 0, res.alarm_state, rec, err)) {
            std::fprintf(stderr, "error: %s\n", err.c_str());
            rc = 1;
        }
        else
            std::printf("Saved as %s/%s\n", jobdir.c_str(), rec.dir.c_str());
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
    return rc;
}

int main(int argc, char **argv)
{
    std::string method_path, cal_path, report_path, trace_path, jobdir, mode;
    int standard_num = 0, monitor_s = 10, point = 1, max_runs = 0;
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
        else if(!std::strcmp(argv[i], "-j"))  jobdir      = need("-j");
        else if(!std::strcmp(argv[i], "-s"))  standard_num = std::atoi(need("-s"));
        else if(!std::strcmp(argv[i], "-p"))  point       = std::atoi(need("-p"));
        else if(!std::strcmp(argv[i], "-n"))  max_runs    = std::atoi(need("-n"));
        else if(!std::strcmp(argv[i], "-t"))  monitor_s   = std::atoi(need("-t"));
        else if(!std::strcmp(argv[i], "--sim"))       force_sim = true;
        else if(!std::strcmp(argv[i], "--sim-scale")) sim_scale = std::atof(need("--sim-scale"));
        else if(argv[i][0] != '-' && mode.empty())    mode = argv[i];
        else {
            std::fprintf(stderr, "unknown argument: %s\n", argv[i]);
            return 2;
        }
    }

    if(mode == "twa") {
        if(jobdir.empty()) { std::fprintf(stderr, "twa requires -j jobdir\n"); return 2; }
        std::vector<RunRecord> hist;
        std::string err;
        if(!LoadRunList(jobdir, hist, err)) {
            std::fprintf(stderr, "error: %s\n", err.c_str());
            return 2;
        }
        // per (point, component): concentrations in run order
        struct Series { int point; std::string comp; std::vector<double> concs; };
        std::vector<Series> series;
        for(const RunRecord &r : hist) {
            if(r.type != "run") continue;            // cal runs excluded
            std::vector<std::pair<std::string,double>> concs;
            if(!LoadRunConcentrations(jobdir, r, concs, err)) {
                std::fprintf(stderr, "warning: %s\n", err.c_str());
                continue;
            }
            for(auto &pc : concs) {
                Series *s = nullptr;
                for(auto &e : series)
                    if(e.point == r.point && e.comp == pc.first) s = &e;
                if(!s) { series.push_back({ r.point, pc.first, {} }); s = &series.back(); }
                s->concs.push_back(pc.second);
            }
        }
        const int STEL_RUNS64 = 15;                  // legacy STEL_RUNS
        std::printf("TWA / STEL exposure report -- %s\n", jobdir.c_str());
        std::printf("%-6s %-14s %-6s %-10s %-10s %-10s %-10s\n",
                    "Point", "Component", "Runs", "TWA", "Min", "Max", "STEL");
        for(const Series &s : series) {
            double sum = 0, mn = s.concs[0], mx = s.concs[0];
            for(double c : s.concs) { sum += c; if(c < mn) mn = c; if(c > mx) mx = c; }
            double twa = sum / s.concs.size();
            int w = (int)s.concs.size() < STEL_RUNS64 ? (int)s.concs.size() : STEL_RUNS64;
            double stel = 0;
            for(size_t i = 0; i + w <= s.concs.size(); i++) {
                double ws = 0;
                for(int k = 0; k < w; k++) ws += s.concs[i + k];
                ws /= w;
                if(ws > stel) stel = ws;
            }
            std::printf("%-6d %-14s %-6zu %-10.4g %-10.4g %-10.4g %-10.4g\n",
                        s.point, s.comp.c_str(), s.concs.size(), twa, mn, mx, stel);
        }
        if(series.empty())
            std::printf("(no calibrated run data in %s)\n", jobdir.c_str());
        return 0;
    }

    if(mode == "history") {
        if(jobdir.empty()) { std::fprintf(stderr, "history requires -j jobdir\n"); return 2; }
        std::vector<RunRecord> hist;
        std::string err;
        if(!LoadRunList(jobdir, hist, err)) {
            std::fprintf(stderr, "error: %s\n", err.c_str());
            return 2;
        }
        std::printf("%-6s %-20s %-5s %-6s %-4s %-8s %-9s %-6s %s\n",
                    "Num", "Time", "Type", "Point", "Std", "Noise", "Baseline",
                    "Peaks", "Alarm");
        for(const RunRecord &r : hist)
            std::printf("%-6d %-20s %-5s %-6d %-4d %-8ld %-9ld %-6d %s\n",
                        r.run_num, r.timestamp.c_str(), r.type.c_str(), r.point,
                        r.standard, r.noise, r.baseline, r.npeaks,
                        r.alarm.c_str());
        std::printf("%zu run(s) in %s\n", hist.size(), jobdir.c_str());
        return 0;
    }

    bool valid_mode = mode == "run" || mode == "cal" ||
                      mode == "continuous" || mode == "monitor";
    if(method_path.empty() || !valid_mode) {
        std::fprintf(stderr,
            "usage: %s -m method.ini run|cal|continuous|monitor [options]\n"
            "       %s history|twa -j jobdir\n"
            "  run         [-p point] [-C cal.ini] [-j jobdir] [-o report.csv] [-D trace.csv]\n"
            "  cal         -s N [-C cal.ini] [-j jobdir]    (N = standard 1..%d)\n"
            "  continuous  [-C cal.ini] [-j jobdir] [-n maxruns]\n"
            "  monitor     [-t seconds]\n"
            "  --sim / --sim-scale X  simulation backend\n",
            argv[0], argv[0], STAND_NUM64);
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
        // calibration file is optional on the first cal run
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
    std::signal(SIGINT, OnSigInt);

    int rc = 0;
    if(mode == "monitor") {
        rc = Monitor(m, h, monitor_s);
    }
    else if(mode == "run") {
        rc = DoOneRun(m, h, false, 0, point, cal_path, jobdir,
                      report_path, trace_path);
    }
    else if(mode == "cal") {
        if(standard_num < 1 || standard_num > STAND_NUM64) {
            std::fprintf(stderr, "error: cal requires -s 1..%d\n", STAND_NUM64);
            CloseHardware(h);
            return 2;
        }
        rc = DoOneRun(m, h, true, standard_num, point, cal_path, jobdir,
                      report_path, trace_path);
    }
    else {   // continuous: cycle points, pause repeat_interval, auto-cal
        int points = m.hw.point_valves.empty() ? 1 : (int)m.hw.point_valves.size();
        int runs_done = 0, runs_since_cal = 0;
        std::printf("Continuous mode: %d point(s), repeat_interval=%lds, "
                    "auto-cal every %d run(s)%s\n",
                    points, m.timing.repeat_interval, m.timing.auto_cal_every,
                    max_runs ? "" : ", Ctrl+C to stop");
        while(!g_stop && (max_runs == 0 || runs_done < max_runs)) {
            // scheduled auto-calibration
            if(m.timing.auto_cal_every > 0 &&
               runs_since_cal >= m.timing.auto_cal_every) {
                std::printf("\n--- auto-calibration (standard %d) ---\n",
                            m.timing.auto_cal_standard);
                rc |= DoOneRun(m, h, true, m.timing.auto_cal_standard, 1,
                               cal_path, jobdir, "", "");
                runs_since_cal = 0;
                if(g_stop) break;
            }
            for(int p = 1; p <= points && !g_stop; p++) {
                std::printf("\n--- run %d, point %d ---\n", runs_done + 1, p);
                rc |= DoOneRun(m, h, false, 0, p, cal_path, jobdir, "", "");
                runs_done++;
                runs_since_cal++;
                if(max_runs && runs_done >= max_runs) break;
            }
            if(g_stop || (max_runs && runs_done >= max_runs)) break;
            if(m.timing.repeat_interval > 0) {   // pause between cycles
                if(h.sim) h.sim->AdvanceSeconds((double)m.timing.repeat_interval);
                else
                    for(long s = 0; s < m.timing.repeat_interval && !g_stop; s++)
                        std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }
        std::printf("\nContinuous mode finished: %d run(s)%s\n", runs_done,
                    g_stop ? " (interrupted)" : "");
    }

    CloseHardware(h);
    return rc;
}
