// wpeak64_cli.cpp -- console front end of the 64-bit WPEAK port.
//
// Usage:
//   wpeak64_cli                              synthetic demo run + self-test
//   wpeak64_cli -d run.csv [-m method.ini] [-o report.csv] [-H report.html]
//
//   -d  chromatogram CSV ("value" or "time,value" per line)
//   -m  method file: detector settings + component table (see method64.h)
//   -o  write the peak report to a CSV file
//   -H  write a printable HTML report (table + SVG chromatogram)
//
// Peaks are identified against the method's component table (CheckRT retention
// -time windows), concentrations computed from response factors, and negative
// peaks reported as detected but not quantified -- as in the GC301c build.

#include "peak64.h"
#include "method64.h"
#include "synth64.h"
#include <cstdio>
#include <cstring>
#include <string>

using namespace wpeak64;

int main(int argc, char **argv)
{
    std::string data_path, method_path, report_path, html_path;
    for(int i = 1; i < argc; i++) {
        auto need = [&](const char *opt) -> const char * {
            if(i + 1 >= argc) { std::fprintf(stderr, "missing argument for %s\n", opt); std::exit(2); }
            return argv[++i];
        };
        if     (!std::strcmp(argv[i], "-d")) data_path   = need("-d");
        else if(!std::strcmp(argv[i], "-m")) method_path = need("-m");
        else if(!std::strcmp(argv[i], "-o")) report_path = need("-o");
        else if(!std::strcmp(argv[i], "-H")) html_path   = need("-H");
        else {
            std::fprintf(stderr,
                "usage: %s [-d run.csv] [-m method.ini] [-o report.csv] [-H report.html]\n", argv[0]);
            return 2;
        }
    }

    Method m;                       // defaults match the synthetic demo
    m.det.MinHeight = 20;
    std::string err;

    if(!method_path.empty() && !LoadMethod(method_path, m, err)) {
        std::fprintf(stderr, "error: %s\n", err.c_str());
        return 2;
    }

    bool synthetic = data_path.empty();
    std::vector<long> y;
    if(synthetic) {
        SynthConfig cfg;
        cfg.data_rate = m.data_rate;
        cfg.analysis_time = m.analysis_time;
        y = MakeChromatogram(cfg, DefaultPeaks());
    }
    else if(!LoadChromatogram(data_path, y, err)) {
        std::fprintf(stderr, "error: %s\n", err.c_str());
        return 2;
    }

    Integrator integ(m.det, m.data_rate, m.analysis_time);
    for(long v : y)
        integ.ProcessPoint(v);

    std::vector<ReportRow> rows = BuildReport(integ.peaks, m);

    std::printf("GC301c Gas Chromatograph -- WPEAK64 (64-bit port of WPEAK 2.4.43 core)\n");
    std::printf("Data: %s   Method: %s\n",
                synthetic ? "(synthetic demo)" : data_path.c_str(),
                method_path.empty() ? "(defaults)" : method_path.c_str());
    std::printf("Noise=%ld  Baseline=%ld  (data rate %d pts/s, analysis time %ld s, %s method)\n\n",
                integ.noise, integ.act_thresh, m.data_rate, m.analysis_time,
                m.detect_meth == 0 ? "height" : "area");

    std::printf("%-5s %-14s %-8s %-9s %-11s %-8s %-8s %-12s %s\n",
                "Num", "Component", "RT (s)", "Height", "Area",
                "From(s)", "To(s)", "Concentr.", "Type");
    int npos = 0, nneg = 0;
    for(const ReportRow &r : rows) {
        const Peak &p = r.peak;
        bool neg = p.Height < 0;
        if(neg) nneg++; else npos++;
        char num[8], conc[24];
        if(neg) std::snprintf(num, sizeof num, "-");
        else    std::snprintf(num, sizeof num, "%d", p.Num);
        if(r.calibrated) std::snprintf(conc, sizeof conc, "%g", r.concentration);
        else             std::snprintf(conc, sizeof conc, "%s", neg ? "-" : "n/cal");
        std::printf("%-5s %-14s %-8.1f %-9ld %-11.0f %-8.1f %-8.1f %-12s %s\n",
                    num, r.name.c_str(),
                    (double)p.Time / m.data_rate,
                    p.Height,
                    p.Area,
                    (double)p.From / m.data_rate,
                    (double)p.To   / m.data_rate,
                    conc,
                    neg ? "NEGATIVE (not quantified)" : "positive");
    }
    std::printf("\n%d positive peak(s), %d negative peak(s) reported.\n", npos, nneg);

    if(!report_path.empty()) {
        if(!WriteReportCsv(report_path, rows, m, integ.noise, integ.act_thresh, err)) {
            std::fprintf(stderr, "error: %s\n", err.c_str());
            return 2;
        }
        std::printf("Report written to %s\n", report_path.c_str());
    }
    if(!html_path.empty()) {
        if(!WriteReportHtml(html_path, y, rows, m, integ.noise, integ.act_thresh, err)) {
            std::fprintf(stderr, "error: %s\n", err.c_str());
            return 2;
        }
        std::printf("HTML report written to %s\n", html_path.c_str());
    }

    // with no arguments the exit code doubles as the synthetic self-test:
    // expect 3 positive + 1 negative peak
    if(synthetic && method_path.empty())
        return (npos == 3 && nneg == 1) ? 0 : 1;
    return 0;
}
