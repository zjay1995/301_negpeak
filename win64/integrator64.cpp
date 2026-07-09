// integrator64.cpp -- 64-bit port of the WPEAK peak-detection state machine.
// Faithful port of Integrator/DataQueue from CALC.CPP and RAW_DATA.CPP; see
// peak64.h for the scope of the port. Original comments are kept where they
// explain the algorithm.

#include "peak64.h"
#include <cstdlib>
#include <cmath>

namespace wpeak64 {

// ---- DataQueue (RAW_DATA.CPP) ----------------------------------------------
DataQueue::DataQueue(int _segment_width)
    : i(-1)
    , segment_width(_segment_width)
    , nMultipleSegment(1)
    , real_segment_width(_segment_width)
{
}

void DataQueue::Put(long d)
{
    i++;
    if(i == real_segment_width) {
        int new_real_segment_width = segment_width * nMultipleSegment;

        if(new_real_segment_width > real_segment_width) { // real segment changed
            // duplicate previous data (i.e. nMultipleSegment==3 after 1)
            for(int idx = real_segment_width + segment_width; idx < new_real_segment_width; idx++) {
                data[idx] = data[idx - segment_width];
            }
            // do not change i! Continue fill the array
        }
        else {
            i = 0; // begin fill the array from the beginning
        }
        real_segment_width = new_real_segment_width;
    }
    data[i] = d;
}

long DataQueue::GetScaledSample() const
{
    long scaled = 0;
    for(int idx = 0; idx < real_segment_width; idx++)
        scaled += data[idx];
    scaled /= real_segment_width;
    return scaled;
}

// ---- Integrator ------------------------------------------------------------
Integrator::Integrator(const DetectorSettings &settings, int _data_rate, long _analysis_time)
    : det(settings)
    , data_rate(_data_rate)
    , analysis_time(_analysis_time)
    , dq(settings.segment_width)
{
    peak.Num = 0; // no peaks yet
}

// RawData::ProcessData (RAW_DATA.CPP), reduced: synthetic/ported data carries
// no autorange bits, so cur_autoscale is always 1.
void Integrator::ProcessPoint(long value)
{
    if(raw_time / data_rate >= analysis_time) {
        acquire_finished = true;
        return;
    }
    raw_time++;
    long cur_time = raw_time / data_rate;
    if(cur_time >= analysis_time) {
        acquire_finished = true;
        raw_time = analysis_time * data_rate;
    }

    // Increase segment after some time (as in RAW_DATA.CPP; the ==3 branch is
    // unreachable there too because /3 is tested before /2)
    if(cur_time > analysis_time / 3)
        dq.SetMultipleSegment(2);
    else if(cur_time > analysis_time / 2)
        dq.SetMultipleSegment(3);

    dq.Put(value);
    DetectPeaks();
}

// Integrator::DetectPeaks (CALC.CPP), minus the Excel/DAC/quick-scan hooks.
void Integrator::DetectPeaks()
{
    if(last_time == raw_time && analyze_segment != END_OF_PEAK)
        return;

    if(dq.IsSampleFinished()) {
        scaled_sample = dq.GetScaledSample();
    }
    else if(analyze_segment != NOISE && analyze_segment != END_OF_PEAK)
        return;

    raw_sample = dq.GetRawSample();
    last_time = raw_time;

    switch(analyze_segment) {
        case NOISE:               NoiseAndBaseline();  break;
        case BETWEEN_PEAKS:       BetweenPeaks();      break;
        case PEAK_STARTING:       PeakStarting();      break;
        case PEAK_STILL_STARTING: PeakStillStarting(); break;
        case PEAK_RISING:         PeakRising();        break;
        case STILL_DESCENDING:    StillDescending();   break;
        case NEW_PEAK_RISING:     NewPeakRising();     break;
        case NEW_PEAK:            NewPeak();           break;
        case PEAK_DESCENDING:     PeakDescending();    break;
        case END_OF_PEAK:         EndOfPeak();         break;
    }

    // Detection-only negative-peak finder running alongside the positive
    // state machine (added to the main algorithm, 06/29/26).
    if(dq.IsSampleFinished())
        DetectNegativePeak();

    prev_raw_unit = raw_sample;
    prev_scaled_sample = scaled_sample;
    prev_retention_time = raw_time;
    prev_raw_invalid = false;
}

void Integrator::NoiseAndBaseline()
{
    long NandBTime = det.NandBtime * data_rate;
    if(raw_time < NandBTime) return;

    long NandBLen = det.NandBlen * data_rate;
    if(raw_time < NandBTime + NandBLen) {
        temp_base += raw_sample;                     // total for average raw unit
        long TempPnt = prev_raw_invalid ? 0 : raw_sample - prev_raw_unit;
        temp_noise += std::labs(TempPnt);            // total the diff. in raw units
        NandBctr++;
    }
    else {                                           // if done w/ this state,...
        if(NandBctr) {
            noise = (long)((double)temp_noise / (double)NandBctr * 2.0);
            if(det.noise_reduct > 1)
                noise = noise / det.noise_reduct;    // NOISE_REDUCT()
            if(noise == 0)
                noise = 1;
            act_thresh = (long)(temp_base / (long)NandBctr); // baseline
        }
        else {
            noise = 1;
            act_thresh = 0;
        }
        analyze_segment = BETWEEN_PEAKS;             // next state
    }
}

void Integrator::BetweenPeaks()
{
    long height = scaled_sample - act_thresh;
    if((height > noise || det.peak_alg == 1) &&
       ((scaled_sample - prev_scaled_sample) > noise)) {
        // Initialize start of peak by Trap. rule.
        peak_area = (double)height * dq.segment_width;
        skim_starting_point = raw_sample;
        peak.From = start_of_peak = raw_time;
        analyze_segment = PEAK_STARTING;             // next state
    }
}

void Integrator::PeakStarting()
{
    long height = scaled_sample - act_thresh;
    if((height > noise || det.peak_alg == 1) &&
       ((scaled_sample - prev_scaled_sample) > noise)) {
        peak_area += (double)height * dq.segment_width;
        analyze_segment = PEAK_STILL_STARTING;       // next state
    }
    else
        analyze_segment = BETWEEN_PEAKS;             // next state
}

void Integrator::PeakStillStarting()
{
    long height = scaled_sample - act_thresh;
    if((height > noise || det.peak_alg == 1) &&
       ((scaled_sample - prev_scaled_sample) > noise)) {
        peak_area += (double)height * dq.segment_width;
        analyze_segment = PEAK_RISING;               // next state
    }
    else
        analyze_segment = BETWEEN_PEAKS;             // next state
}

void Integrator::PeakRising()
{
    long height = scaled_sample - act_thresh;
    peak_area += (double)height * dq.segment_width;

    if((prev_scaled_sample - scaled_sample) > noise) { // if now descending,...
        // Set temporary Peak Height, Total Height and actual Retention
        // Time and establish Peak Width and Cutoff time.
        peak.Time = raw_time;
        peak_half_width = raw_time - start_of_peak;
        end_of_peak = raw_time + (long)peak_half_width + (long)(peak_half_width / 4);
        peak.Height = height;                        // temp. Peak Height
        analyze_segment = PEAK_DESCENDING;           // next state
    }
}

void Integrator::PeakDescending()
{
    long height = scaled_sample - act_thresh;
    peak_area += (double)height * dq.segment_width;

    if((prev_scaled_sample - scaled_sample) > noise) { // if still descending,...
        prev_retention_time = raw_time;
        analyze_segment = STILL_DESCENDING;          // next state
    }
    else {
        analyze_segment = PEAK_RISING;               // was just a glitch?
    }
}

void Integrator::StillDescending()
{
    long height = scaled_sample - act_thresh;

    // Has another Peak started?
    if((height > noise || det.peak_alg == 1) &&
       ((scaled_sample - prev_scaled_sample) > noise)) { // if new peak ascending,...
        next_peak_area = (double)height * dq.segment_width;
        next_skim_starting_point = raw_sample;
        next_start_of_peak = raw_time;
        peak_rising = false;
        analyze_segment = NEW_PEAK_RISING;           // next state
    }
    else {                                           // else peak still descending
        peak_area += (double)height * dq.segment_width;
    }

    // Has the signal sunk below the Baseline? Or is it cutoff time?
    if(((height <= 0) && !(det.peak_alg == 1)) ||
       (raw_time > end_of_peak)) {
        next_skim_starting_point = raw_sample;
        next_start_of_peak = raw_time;
        EndOfPeak();
    }
}

void Integrator::NewPeakRising()
{
    long height = scaled_sample - act_thresh;
    next_peak_area += (double)height * dq.segment_width;
    if((scaled_sample - prev_scaled_sample) > noise) { // if still rising,...
        analyze_segment = NEW_PEAK;
    }
    else { // Else not a new peak: merge into the previous one.
        peak_area += next_peak_area;
        analyze_segment = STILL_DESCENDING;
    }
}

void Integrator::NewPeak()
{
    long height = scaled_sample - act_thresh;
    next_peak_area += (double)height * dq.segment_width;
    if((scaled_sample - prev_scaled_sample) > noise) { // if new peak still rising,...
        peak_rising = true;                          // process the previous peak
        EndOfPeak();
    }
    else {
        peak_area += next_peak_area;
        analyze_segment = STILL_DESCENDING;
    }
}

// Detection-only negative-peak finder for the main algorithm; same logic as
// the DetectNegativePeak() added to CALC.CPP. Appends a peak with a NEGATIVE
// Height and a non-positive Num so it is shown but never quantified.
void Integrator::DetectNegativePeak()
{
    if(det.peak_alg == 2)                    // curve-fit path finds its own
        return;
    if(analyze_segment != BETWEEN_PEAKS) {   // only look when not inside a positive peak
        neg_start_of_peak = -1;              // abandon any partial negative peak
        return;
    }

    long depth = act_thresh - scaled_sample; // how far below baseline (>0 = dipping)

    if(depth > noise) {                      // below baseline by > noise
        if(neg_start_of_peak < 0) {          // start of a new negative peak
            neg_start_of_peak = raw_time;
            neg_peak_min = 0;
        }
        long h = scaled_sample - act_thresh; // negative height
        if(h < neg_peak_min)
            neg_peak_min = h;                // keep the most negative point
    }
    else if(neg_start_of_peak >= 0) {        // negative peak just ended
        if(std::labs(neg_peak_min) >= det.MinHeight) { // ignore noise-sized dips
            Peak neg_peak;
            neg_peak.Height = neg_peak_min;  // negative -> excluded from quant.
            neg_peak.From   = neg_start_of_peak;
            neg_peak.To     = raw_time;
            neg_peak.Time   = neg_start_of_peak + (raw_time - neg_start_of_peak) / 2;
            // Num stays -1: not numbered/quantified, only drawn.
            peaks.push_back(neg_peak);
        }
        neg_start_of_peak = -1;
    }
}

void Integrator::EndOfPeak()
{
    if(peak_rising)                          // if new peak has already started,...
        analyze_segment = PEAK_RISING;
    else
        analyze_segment = BETWEEN_PEAKS;

    double cur_peak_area;
    if(det.peak_alg != 1) {                  // baseline projection
        cur_peak_area = peak_area;
        peak.To = next_start_of_peak;
    }
    else {                                   // Tangent Skimming
        skim_ending_point = next_skim_starting_point;
        peak.To = time_peak_ends = next_start_of_peak;
        cur_peak_area = peak_area;

        // Calculate Actual Peak Height.
        double delta_y = (double)(skim_ending_point - skim_starting_point);
        double delta_x = (double)(time_peak_ends - start_of_peak);
        double slope   = delta_x != 0 ? delta_y / delta_x : 0;
        delta_x = peak_half_width;
        delta_y = delta_x * slope;
        delta_y += (skim_starting_point - act_thresh);
        peak.Height -= (long)delta_y;

        // Peak Area after Tangent Skim: subtract trapezoid under the skim line.
        double base = (double)(time_peak_ends - start_of_peak);
        long trapez_height = ((skim_ending_point - act_thresh) +
                              (skim_starting_point - act_thresh)) / 2;
        cur_peak_area -= base * trapez_height;
    }
    peak.Area = cur_peak_area;

    // if peak meets min. area & height requirements, recognize the peak.
    if(peak.Area >= det.MinArea && peak.Height >= det.MinHeight) {
        PeakMatchup();
    }

    // Clear out variables
    peak.To = time_peak_ends = 0;
    end_of_peak = 0;
    peak_rising = false;
    peak_area = next_peak_area;
    peak.From = start_of_peak = next_start_of_peak;
    skim_starting_point = next_skim_starting_point;
    prev_retention_time = 0;
}

// PeakMatchup: the original matches components and computes concentrations;
// the port only numbers and stores the peak.
void Integrator::PeakMatchup()
{
    peak.Num++;                              // inc. the peak counter
    peaks.push_back(peak);
}

} // namespace wpeak64
