// synth64.h -- synthetic GC301c chromatogram used by the 64-bit demo apps.
// Produces a signal in detector counts: flat baseline + deterministic noise,
// three positive component peaks and one negative peak (e.g. a composition
// dip / injection artifact) so both detector paths can be demonstrated.

#ifndef SYNTH64_H
#define SYNTH64_H

#include <cmath>
#include <vector>

namespace wpeak64 {

struct SynthConfig {
    int    data_rate     = 10;    // points per second
    long   analysis_time = 120;   // s
    long   baseline      = 1000;  // detector counts
    double noise_amp     = 2.0;   // counts, peak-to-peak-ish
};

struct SynthPeakDef {
    double rt_s;     // retention time, s
    double height;   // counts (negative for a negative peak)
    double sigma_s;  // gaussian width, s
};

inline std::vector<long> MakeChromatogram(const SynthConfig &cfg,
                                          const std::vector<SynthPeakDef> &defs)
{
    const long n = cfg.analysis_time * cfg.data_rate;
    std::vector<long> y((size_t)n);
    unsigned rng = 12345;                    // deterministic LCG noise
    for(long i = 0; i < n; i++) {
        double t = (double)i / cfg.data_rate;
        double v = (double)cfg.baseline;
        for(const auto &p : defs) {
            double dt = (t - p.rt_s) / p.sigma_s;
            v += p.height * std::exp(-0.5 * dt * dt);
        }
        rng = rng * 1103515245u + 12345u;
        v += cfg.noise_amp * (((rng >> 16) & 0x7FFF) / 32767.0 - 0.5);
        y[(size_t)i] = (long)std::lround(v);
    }
    return y;
}

inline std::vector<SynthPeakDef> DefaultPeaks()
{
    return {
        { 40.0, +400.0, 3.0 },   // component 1
        { 60.0, +250.0, 4.0 },   // component 2
        { 85.0, -300.0, 3.0 },   // negative peak
        {105.0, +150.0, 3.5 },   // component 3
    };
}

} // namespace wpeak64

#endif // SYNTH64_H
