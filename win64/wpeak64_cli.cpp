// wpeak64_cli.cpp -- console front end of the 64-bit WPEAK port.
// Runs the ported GC301c integrator over a synthetic chromatogram and prints
// the detected peaks (positive: numbered; negative: marked NEG, detection
// only). Builds natively on Linux for verification and cross-compiles to a
// 64-bit Windows console executable.

#include "peak64.h"
#include "synth64.h"
#include <cstdio>

using namespace wpeak64;

int main()
{
    SynthConfig cfg;
    DetectorSettings det;
    det.MinHeight = 20;      // suppress noise-sized events
    det.MinArea   = 0;

    Integrator integ(det, cfg.data_rate, cfg.analysis_time);

    std::vector<long> y = MakeChromatogram(cfg, DefaultPeaks());
    for(long v : y)
        integ.ProcessPoint(v);

    std::printf("GC301c Gas Chromatograph -- WPEAK64 (64-bit port of WPEAK 2.4.43 core)\n");
    std::printf("Noise=%ld  Baseline=%ld  (data rate %d pts/s, analysis time %ld s)\n\n",
                integ.noise, integ.act_thresh, cfg.data_rate, cfg.analysis_time);

    std::printf("%-5s %-9s %-10s %-12s %-9s %-9s %s\n",
                "Num", "RT (s)", "Height", "Area", "From (s)", "To (s)", "Type");
    int npos = 0, nneg = 0;
    for(const Peak &p : integ.peaks) {
        bool neg = p.Height < 0;
        if(neg) nneg++; else npos++;
        char num[8];
        if(neg) std::snprintf(num, sizeof num, "-");
        else    std::snprintf(num, sizeof num, "%d", p.Num);
        std::printf("%-5s %-9.1f %-10ld %-12.0f %-9.1f %-9.1f %s\n",
                    num,
                    (double)p.Time / cfg.data_rate,
                    p.Height,
                    p.Area,
                    (double)p.From / cfg.data_rate,
                    (double)p.To   / cfg.data_rate,
                    neg ? "NEGATIVE (detected, not quantified)" : "positive");
    }
    std::printf("\n%d positive peak(s), %d negative peak(s) detected.\n", npos, nneg);

    // exit code doubles as a self-test: expect 3 positive + 1 negative
    return (npos == 3 && nneg == 1) ? 0 : 1;
}
