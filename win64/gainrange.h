// gainrange.h -- ADS1115 PGA auto-ranging: pure decision logic, no I/O.
//
// The legacy GC301c avoided detector clipping with autorange bits baked into
// each raw sample (pow10_array / cur_autoscale in RAW_DATA.CPP): whichever
// x1/x10/x100/x1000 gain was active, downstream code multiplied the sample
// back to a single canonical unit before it ever reached the integrator.
//
// This header is the ADS1115 equivalent, factored out from ads1115.cpp so it
// has no Linux/I2C dependency and can be exercised by a plain unit test:
// AutoRangedRead() decides, from one physical read, whether the ADC's PGA
// (programmable gain amplifier / full-scale range) needs to step to a less
// sensitive range this call (on saturation, with an immediate re-read so the
// caller never sees a clipped sample) or a more sensitive one on the next
// call (on a weak signal). Every returned count is normalized back to the
// method's configured reference range, so a mid-run range change is
// invisible to the Integrator -- exactly like the legacy cur_autoscale
// normalization.

#ifndef GAINRANGE_H
#define GAINRANGE_H

#include <cstdlib>
#include <cmath>

namespace wpeak64 {

struct PgaChoice { int mv; int bits; double fsr_v; };

// full-scale ranges, descending sensitivity (index 0 = least sensitive /
// widest range, last = most sensitive / narrowest range)
inline const PgaChoice *PgaTable(int &count)
{
    static const PgaChoice table[] = {
        { 6144, 0, 6.144 }, { 4096, 1, 4.096 }, { 2048, 2, 2.048 },
        { 1024, 3, 1.024 }, {  512, 4, 0.512 }, {  256, 5, 0.256 },
    };
    count = (int)(sizeof table / sizeof table[0]);
    return table;
}

inline int PgaIndexForBits(int bits)
{
    int n; const PgaChoice *t = PgaTable(n);
    for(int i = 0; i < n; i++) if(t[i].bits == bits) return i;
    return -1;
}
inline double FsrForBits(int bits)
{
    int n; const PgaChoice *t = PgaTable(n);
    for(int i = 0; i < n; i++) if(t[i].bits == bits) return t[i].fsr_v;
    return 2.048;
}
inline int PgaBitsForMv(int mv, double *fsr_v_out = nullptr)
{
    int n; const PgaChoice *t = PgaTable(n);
    for(int i = 0; i < n; i++)
        if(t[i].mv == mv) { if(fsr_v_out) *fsr_v_out = t[i].fsr_v; return t[i].bits; }
    return -1;
}

// ADS1115 codes are 16-bit signed; a code within kSatThresh of full scale is
// treated as saturated (91.5%), one within kWeakThresh of zero as
// under-ranged (9.2%) -- the wide deadband between them avoids range chatter
// on a signal that sits near either boundary.
constexpr long kSatThresh  = 30000;
constexpr long kWeakThresh = 3000;
constexpr long kFullScale  = 32767;

// Reads via read_fn(bits) -> raw code (one physical ADC conversion at the
// given PGA setting). `bits` is both the range to start from and, on return,
// the range the NEXT call should start from. Returns the raw code normalized
// to ref_fsr_v (the method's configured reference range), so the caller
// always sees counts on one consistent scale regardless of which physical
// range was actually used for this sample.
template<class ReadFn>
long AutoRangedRead(int &bits, double ref_fsr_v, ReadFn read_fn)
{
    long raw = read_fn(bits);

    // table index 0 = 6144mV (coarsest/least sensitive) .. last = 256mV
    // (finest/most sensitive); saturation needs a LOWER index (bigger full
    // scale), a weak signal needs a HIGHER index (smaller full scale, more
    // resolution).
    if(std::labs(raw) >= kSatThresh) {          // saturated: step coarser, re-read now
        int i = PgaIndexForBits(bits);
        if(i > 0) {
            int n; const PgaChoice *t = PgaTable(n);
            bits = t[i - 1].bits;
            raw = read_fn(bits);
        }
    }

    double used_fsr = FsrForBits(bits);
    long out = (long)std::lround((double)raw * used_fsr / ref_fsr_v);

    if(std::labs(raw) < kWeakThresh) {          // weak: step finer for next time only
        int i = PgaIndexForBits(bits);
        int n; const PgaChoice *t = PgaTable(n);
        if(i >= 0 && i + 1 < n)
            bits = t[i + 1].bits;
    }
    return out;
}

} // namespace wpeak64

#endif // GAINRANGE_H
