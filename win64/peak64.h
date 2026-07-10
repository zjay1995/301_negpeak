// peak64.h -- 64-bit port of the GC301c WPEAK peak-detection core.
//
// Ported from PEAK.H / CALC.H / RAW_DATA.CPP of the legacy Borland C++ / OWL
// application (WPEAK 2.4.43). This port keeps the original algorithm and
// naming, replacing 16/32-bit Borland types (huge, int32, ulong, ...) with
// <cstdint> so it compiles as standard C++17 for x86-64 Windows and Linux.
//
// Scope of the port: the analytical core only -- DataQueue segment averaging,
// noise/baseline estimation, the positive-peak recognition state machine, the
// tangent-skimming (peak_alg==1) end-of-peak math, and the negative-peak
// detector added to the main algorithm. Hardware acquisition, calibration,
// component matchup and the OWL GUI are not ported.

#ifndef PEAK64_H
#define PEAK64_H

#include <cstdint>
#include <vector>

namespace wpeak64 {

// ---- Peak (from PEAK.H) ----------------------------------------------------
struct Peak {
    int     Num        = -1;   // peak #; stays <=0 for negative peaks (not quantified)
    long    Time       = 0;    // retention time (in data points) of the apex
    double  Area       = 0;    // area (double since 2.4.4x, P.P. 08/25/17)
    long    Height     = 0;    // height above baseline; NEGATIVE for negative peaks
    long    From       = 0;    // retention time of peak start
    long    To         = 0;    // retention time of peak end
    int     multiplier = 1;    // autoscale multiplier (always 1 in this port)
};

// ---- DataQueue (from CALC.H / RAW_DATA.CPP) --------------------------------
// Segment averaging: raw points are accumulated and every segment_width points
// one "scaled sample" is produced. real_segment_width can grow at run time
// (SetMultipleSegment) exactly as in the original.
class DataQueue {
public:
    enum { MAX_Q = 42, MULT_Q = 3 };
    long data[MAX_Q * MULT_Q] = {0};
    int  i;
    int  segment_width;
    int  nMultipleSegment;
    int  real_segment_width;

    explicit DataQueue(int _segment_width);

    void Put(long d);
    void SetMultipleSegment(int _nMultipleSegment) { nMultipleSegment = _nMultipleSegment; }

    long GetScaledSample() const;
    long GetRawSample() const { return data[i]; }
    bool IsSampleFinished() const { return i % segment_width == segment_width - 1; }
};

// ---- Detector settings (subset of DETECTOR.H used by the integrator) -------
struct DetectorSettings {
    int    segment_width = 4;   // points per scaled sample
    long   NandBtime     = 2;   // s: wait before noise/baseline measurement
    long   NandBlen      = 10;  // s: length of noise/baseline measurement
    double MinArea       = 0;   // minimum area to recognize a peak
    long   MinHeight     = 0;   // minimum height to recognize a peak
    int    peak_alg      = 0;   // 0 = baseline projection, 1 = tangent skimming
    int    noise_reduct  = 8;   // NOISE_REDUCT() from INTERNAL.H
};

// ---- Integrator (from CALC.H / CALC.CPP) -----------------------------------
// Peak recognition state machine (state numbers preserved from CALC.H).
enum {
    NOISE               = 0,
    BETWEEN_PEAKS       = 1,
    PEAK_STARTING       = 2,
    PEAK_STILL_STARTING = 4,
    PEAK_RISING         = 5,
    STILL_DESCENDING    = 6,
    END_OF_PEAK         = 7,
    NEW_PEAK_RISING     = 8,
    NEW_PEAK            = 11,
    PEAK_DESCENDING     = 13
};

class Integrator {
public:
    DetectorSettings det;
    int  data_rate;             // points per second
    long analysis_time;         // s

    DataQueue dq;
    long raw_time = 0;          // raw_data.time: retention time in points
    bool acquire_finished = false;

    // state (names preserved from the original Integrator)
    long   temp_noise = 0, temp_base = 0;
    int    NandBctr = 0;
    double peak_area = 0;
    long   start_of_peak = 0, end_of_peak = 0, time_peak_ends = 0;
    long   skim_ending_point = 0, skim_starting_point = 0;
    double peak_half_width = 0;
    long   prev_raw_unit = 0;
    bool   prev_raw_invalid = true;
    long   prev_scaled_sample = 0;
    long   noise = 1;
    long   act_thresh = 0;      // baseline
    long   prev_retention_time = 0;
    long   last_time = -1;
    long   next_skim_starting_point = 0, next_start_of_peak = 0;
    double next_peak_area = 0;
    bool   peak_rising = false;
    int    analyze_segment = NOISE;
    long   scaled_sample = 0, raw_sample = 0;

    // negative-peak detection state (added to the main algorithm, 06/29/26)
    long   neg_start_of_peak = -1;
    long   neg_peak_min      = 0;
    double neg_peak_area     = 0;   // accumulated (negative) area under baseline

    Peak peak;                        // current (positive) peak being built
    std::vector<Peak> peaks;          // detected peaks (replaces PeakList)

    Integrator(const DetectorSettings &settings, int _data_rate, long _analysis_time);

    // Feed one raw data point (RawData::ProcessData + DetectPeaks).
    void ProcessPoint(long value);

private:
    void DetectPeaks();
    void NoiseAndBaseline();
    void BetweenPeaks();
    void PeakStarting();
    void PeakStillStarting();
    void PeakRising();
    void PeakDescending();
    void StillDescending();
    void NewPeakRising();
    void NewPeak();
    void DetectNegativePeak();
    void EndOfPeak();
    void PeakMatchup();               // numbering only (no component table in port)
};

} // namespace wpeak64

#endif // PEAK64_H
