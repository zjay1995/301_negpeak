// gainrange_test.cpp -- unit test for the ADS1115 auto-ranging decision
// logic in gainrange.h. Pure, portable, no I2C/hardware needed.

#include "gainrange.h"
#include <cstdio>
#include <cstdlib>
#include <vector>

using namespace wpeak64;

static int g_fail = 0;
#define CHECK(cond, msg) do { \
    if(!(cond)) { std::printf("FAIL: %s (%s:%d)\n", msg, __FILE__, __LINE__); g_fail++; } \
    else std::printf("ok: %s\n", msg); \
} while(0)

int main()
{
    // 1) normal-range reading: no range change, output == input (same range as ref)
    {
        int bits = PgaBitsForMv(2048);
        double ref_fsr = FsrForBits(bits);
        std::vector<long> reads = { 15000 };
        size_t i = 0;
        long out = AutoRangedRead(bits, ref_fsr, [&](int) { return reads[i++]; });
        CHECK(out == 15000, "mid-scale reading passes through unchanged");
        CHECK(bits == PgaBitsForMv(2048), "mid-scale reading keeps the same range");
        CHECK(i == 1, "mid-scale reading does one physical read");
    }

    // 2) saturated reading at the reference range: steps to a coarser range
    //    and re-reads once, returning a non-clipped, correctly normalized value
    {
        int bits = PgaBitsForMv(2048);            // 2.048V FSR
        double ref_fsr = FsrForBits(bits);
        // first read (at 2048mV) saturates; second read (at 4096mV, coarser)
        // returns a code that represents the same physical voltage
        std::vector<long> reads = { 32000, 16000 };
        size_t i = 0;
        long out = AutoRangedRead(bits, ref_fsr, [&](int) { return reads[i++]; });
        CHECK(i == 2, "saturated reading triggers exactly one re-read");
        CHECK(bits == PgaBitsForMv(4096), "saturated reading steps to the next coarser range");
        // 16000 counts at 4096mV FSR == 32000 counts at 2048mV FSR (same voltage);
        // normalized back to the 2048mV reference it should read ~32000
        CHECK(out > 31900 && out < 32100, "saturated reading normalizes back to the reference scale");
    }

    // 3) saturated at the coarsest range already: no further range to step to,
    //    returns the (still saturated) reading as-is rather than crashing
    {
        int bits = PgaBitsForMv(6144);            // already coarsest
        double ref_fsr = FsrForBits(bits);
        std::vector<long> reads = { 32700 };
        size_t i = 0;
        long out = AutoRangedRead(bits, ref_fsr, [&](int) { return reads[i++]; });
        CHECK(i == 1, "saturated at the coarsest range does not attempt a retry");
        CHECK(bits == PgaBitsForMv(6144), "coarsest range stays selected when no coarser one exists");
        CHECK(out == 32700, "coarsest-range saturation is returned unmodified (no headroom left)");
    }

    // 4) weak reading: value is returned as-is this call, but the range steps
    //    finer for the NEXT call only (not retried immediately)
    {
        int bits = PgaBitsForMv(2048);
        double ref_fsr = FsrForBits(bits);
        std::vector<long> reads = { 1500 };
        size_t i = 0;
        long out = AutoRangedRead(bits, ref_fsr, [&](int) { return reads[i++]; });
        CHECK(i == 1, "weak reading does not retry within the same call");
        CHECK(out == 1500, "weak reading's own value is unchanged (normalized to itself)");
        CHECK(bits == PgaBitsForMv(1024), "weak reading steps to a finer range for next time");
    }

    // 5) weak at the finest range already: stays put, no out-of-bounds step
    {
        int bits = PgaBitsForMv(256);             // already finest
        double ref_fsr = FsrForBits(bits);
        std::vector<long> reads = { 500 };
        size_t i = 0;
        AutoRangedRead(bits, ref_fsr, [&](int) { return reads[i++]; });
        CHECK(bits == PgaBitsForMv(256), "finest range stays selected when no finer one exists");
    }

    // 6) a full run: signal ramps up across several calls, forcing successive
    //    coarser ranges, then a later weak segment brings it back down --
    //    check the range converges and stays stable in the deadband
    {
        int bits = PgaBitsForMv(256);   // start over-sensitive on purpose
        double ref_fsr = FsrForBits(PgaBitsForMv(2048));
        // simulate a fixed real-world voltage that would read ~32000 at 256mV;
        // provide the correctly-scaled raw code for whatever range is asked
        double true_volts = 32000.0 * 0.256 / 32768.0;
        auto physical_read = [&](int rbits) {
            double fsr = FsrForBits(rbits);
            long code = (long)(true_volts / fsr * 32768.0);
            if(code > 32767) code = 32767;
            if(code < -32768) code = -32768;
            return code;
        };
        int steps = 0;
        long out = 0;
        for(int k = 0; k < 10; k++) {
            out = AutoRangedRead(bits, ref_fsr, physical_read);
            steps++;
            if(std::labs(physical_read(bits)) < kSatThresh &&
               std::labs(physical_read(bits)) >= kWeakThresh)
                break;   // converged into the deadband
        }
        CHECK(steps < 10, "range converges to a stable non-clipping setting within a few reads");
        double expect = true_volts / ref_fsr * 32768.0;
        CHECK(out > expect - 50 && out < expect + 50,
             "converged reading normalizes to the correct reference-scale value");
    }

    if(g_fail) { std::printf("\n%d check(s) FAILED\n", g_fail); return 1; }
    std::printf("\nall checks passed\n");
    return 0;
}
