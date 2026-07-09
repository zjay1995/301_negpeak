/*
          Pull data from Raw Data Buffer,then SEGMENT the raw data.
          SEGMENT is the average of a specified number of sampling data points
          that will give a data rate of 1,5,10 or 20 points/sec.
          SAMPLE_RATE is the user specified sampling rate.
          DATA_RATE is the actual sampling rate used by the Processor.
          1 pt./sec.   = summation of 20 points for 1 sec. div by 20
          5 pt.s/sec.  = summation of 5 segments of 4 pt.s each div by 4
          10 pt.s/sec. = summation of 2 segments of 10 pt.s each div by 10
          20 pt.s/sec. = each individual point as a segment.
          The data acquisition rate is always 20 pt.s/sec . It is in this
          module that the SEGMENT is created for the desired data rate.
*/

#pragma hdrstop

#ifndef __STDLIB_H
#include <stdlib.h>
#endif
#ifndef __MATH_H
#include <math.h>
#endif
#ifndef __STRING_H
#include <string.h>
#endif

#ifndef  _CALC_H
#include "calc.h"
#endif
#ifndef  _METHOD_H
#include "method.h"
#endif
#ifndef  _RUN_H
#include "run.h"
#endif

char data_rate = 20;      // rate data received in pts/sec.
float concen_total = 0; // added. P.P. 09/23/03

#ifndef  _PEAKLIST_H
#include "peaklist.h"
#endif

int mult_num[NUMDETECTORS][COMPONENT_ROWS][4] = {0}; // added. P.P. 01/10/13
static void near CalcConcVars(Detector *pDetector, CalRes &cal_results/*///???, float comp_slope[]*/)
{
int rci = pDetector->FindRefCompIndex(); // index to reference component
Component *pRefComponent=pDetector->FindRefComp(); // Reference component
if(pRefComponent && rci !=-1) {   // if valid ref. comp. for this detector,...
for(int i=0; i<COMPONENT_ROWS; i++) {
Component *pComponent=&pDetector->components[i];
// if the component is active, calc. slope and rf for it.
if(pComponent->active_yn) {
double slope = 0.0;
double rf = 0.0;
int slope_div = 0;
int rf_div = 0;

// for each standard in the component...
for(int stand_idx=0; stand_idx!=STAND_NUM; stand_idx++) {
double ref_conc = pRefComponent->stand[stand_idx];
double stand_conc = pComponent->stand[stand_idx];
double ref_results = cal_results[rci][stand_idx];
double stand_results = cal_results[i][stand_idx];
// Determine slope (height or area / concentration)...
// .for this component.
if(stand_results > 0.0 && stand_conc > 0.0) {
slope_div++;
slope += stand_conc / stand_results;
// Determine Response Factor for this component.
// NOTE: The response factor calculated here will...
// .not be used other than for display because the
// .slope is always valid when the RF can be calculated.
// ALSO NOTE: The response factor calculated here is NOT...
// .saved to the Method part of the Job file, so the...
// .response factors entered as part of the Method...
// .are not destroyed and therefore will still be...
// .used when a response factor can not be calculated...
// .for a component.
if(ref_conc > 0.0) {
rf_div++;
rf += ref_results / ref_conc *
        stand_conc / stand_results;
}
}
}
if(slope_div != 0)
slope /= slope_div;
if(rf_div != 0) {
rf /= rf_div;
pComponent->resp_fact = rf;
}

pComponent->slope = slope;

{ // multipoint calibration
{ for(int j=0; j<STAND_NUM+1; j++) pComponent->lstart[j]=pComponent->lslope[j]=pComponent->lvalue[j]=0.; }
// sort
for(int stand_idx=0; stand_idx!=STAND_NUM; stand_idx++) {
double stand_conc = pComponent->stand[stand_idx];
double stand_results = cal_results[i][stand_idx];

if(stand_results > 0.0 && stand_conc > 0.0) {
for(int j=0; j<STAND_NUM; j++) {
if(pComponent->lstart[j]>stand_results) {
for(int j1=STAND_NUM-1; j1>j; j1--) { // shift
pComponent->lstart[j1]=pComponent->lstart[j1-1];
pComponent->lslope[j1]=pComponent->lslope[j1-1];
pComponent->lvalue[j1]=pComponent->lvalue[j1-1];
}
// insert
pComponent->lstart[j]=stand_results;
pComponent->lslope[j]=stand_conc;
pComponent->lvalue[j]=stand_conc;
break;
}
if(pComponent->lstart[j]==stand_results) {
// average?
break;
}
if(pComponent->lstart[j]==0.) {
pComponent->lstart[j]=stand_results;
pComponent->lslope[j]=stand_conc;
pComponent->lvalue[j]=stand_conc;
break;
}
}
}
}
BOOL bLast=TRUE;
for(int j1=STAND_NUM-1; j1>=0; j1--) {
if(pComponent->lstart[j1]) {
if(bLast) {
bLast=FALSE;
pComponent->lslope[j1+1]= // after last point use last point and ZERO point
pComponent->lvalue[j1]/
pComponent->lstart[j1];
pComponent->lstart[j1+1]=pComponent->lstart[j1];
pComponent->lvalue[j1+1]=pComponent->lvalue[j1];
}
if(j1==0) {
pComponent->lslope[j1]=pComponent->lvalue[j1]/pComponent->lstart[j1];
pComponent->lstart[j1]=0.;
pComponent->lvalue[j1]=0;
}
else {
pComponent->lslope[j1]=
(pComponent->lvalue[j1]-pComponent->lvalue[j1-1])/
(pComponent->lstart[j1]-pComponent->lstart[j1-1]);
pComponent->lstart[j1]=pComponent->lstart[j1-1];
pComponent->lvalue[j1]=pComponent->lvalue[j1-1];
}
}
else {
pComponent->lstart[j1]=pComponent->lslope[j1]=pComponent->lvalue[j1]=0.;
}
}
}

}
else {
pComponent->slope = 0.0;
{ for(int j=0; j<STAND_NUM+1; j++) pComponent->lstart[j]=pComponent->lslope[j]=0.; }
}
}
}
}


#ifndef  _HNU_H
#include "hnu.h"
#endif

Integrator::Integrator(Method *_pMethod, Detector *_pDetector, Run *_pRun,
                       CalRes *_cal_results, PeakList *_pPeakList, int _acquire_type, int _nDetector, int _nStandard)
        :cal_results(*_cal_results), acquire_type(_acquire_type), cal_passed(0),
         nStandard(_nStandard),
         pMethod(_pMethod),
         pDetector(_pDetector),
         pRun(_pRun),
         nDetector(_nDetector),
         RawData(_pDetector->segment_width, _pMethod->method_table.analysis_time),
         pPeakList(_pPeakList),

         NandBctr(0),
         temp_noise(0L),
         temp_base(0L),
         prev_raw_unit(0),
         prev_raw_invalid(TRUE),
         prev_scaled_sample(0l),
         last_time(-1),
         end_of_peak(0l),
         start_of_peak(0l),
         peak_area(0.0),
         analyze_segment(NOISE),   // peak recognition state machine
        //??  alarm_occurred(FALSE), // set when a conc. detected above alarm level
         scaled_sample(0l),
         raw_sample(0),
         ext_det_done(0), // added for external detector only. P.P. 11/22/08
         quick_run(0), // added. P.P. 01/05/11

         time_peak_ends       (0l),
         peak_rising          (FALSE),
         next_peak_area       (0.0),
         next_start_of_peak   (0l),
         skim_starting_point  (0),
         next_skim_starting_point (0),
         prev_retention_time  (0l)
{
//  ext_det_done = 0; // rem P.P. 05/13/11
    neg_start_of_peak = -1;  // no negative peak in progress. 06/29/26
    neg_peak_min = 0;        // 06/29/26
    peak.Num=0;              // no peaks yet
    concen_total = 0; // added. P.P. 09/23/03
    CalcConcVars(pDetector, cal_results/*///???, comp_slope*/);
    peak.multiplier = 1; // added. P.P. 12/26/12

    if(_acquire_type==CAL||_acquire_type==CHECKCAL) {
//    if(analysis_time>pMethod->method_table.calibrate_time)
//    if(analysis_time>=pMethod->method_table.calibrate_time)  // changed P.P. 01/21/11, then removed
#if !GC321() // cond added. P.P. 12/19/11
        analysis_time = pMethod->method_table.calibrate_time;  // stop at cal time.
#endif
    }
#if QUICK_SCAN() // cond added. P.P. 01/17/11

    else {
    if(pRun->quick_scan_run>0) {  // case added. P.P. 01/06/11
      quick_run = 1;
      analysis_time = pMethod->method_table.quick_scan_time;
    }
    else {   // added. P.P. 01/17/11
      quick_run = 0;
      analysis_time = pMethod->method_table.analysis_time;
    }
  }
  if(_acquire_type==CAL||_acquire_type==CHECKCAL) {  // added. P.P. 01/21/11
    quick_run = 0;
  }

#endif
}

Integrator::~Integrator()
{
}

#include "job.h" // added. P.P. 02/18/12
int Integrator::DetectPeaks( void )
{
    int flags=0;
    Job *pJob=(Job *)pMethod; // added. P.P. 02/18/12
    char det_label = 'A';
//  long time_min = 0;
    long time_s = 0;
    long time_ms = 0;

//  float run_time = 0;
    // If we've already processed this piece of raw data, return now.
    if(last_time == raw_data.time && analyze_segment != END_OF_PEAK)
        return 0;

    // All calculations require the average sample.
    // The Noise&Baseline function uses the raw data.
    if(dq.IsSampleFinished()) { // if sample finished,...
        scaled_sample = dq.GetScaledSample();
//    run_time = (float) (((float) raw_data.time)/data_rate);
        if(nDetector) det_label = 'B';
        else det_label = 'A';
//    pJob->SaveRawDataToFile(pRun, raw_data.time, scaled_sample); // added to save data to Excel file. P.P. 02/18/12
//    pJob->SaveRawDataToFile(pRun, det_label, raw_data.time, scaled_sample); // det label added, save data to Excel file. P.P. 02/19/12
        time_ms = ((1000*raw_data.time)/data_rate)%1000;
        time_s = (((1000*raw_data.time)/data_rate)/1000);
//    time_min = raw_data.time/(60*data_rate);
//    pJob->SaveRawDataToFile(pRun, det_label, time_min, time_s, time_ms, scaled_sample); // det label added, save data to Excel file. P.P. 02/19/12
        if(pDetector->data_to_excel) { // cond added. P.P. 02/24/12
            pJob->SaveRawDataToFile(pRun, det_label, time_s, time_ms, scaled_sample); // det label added, save data to Excel file. P.P. 02/19/12
        }
    }
    else if(analyze_segment != NOISE && analyze_segment != END_OF_PEAK)
        return 0;

    // store the raw sample
    raw_sample = dq.GetRawSample();  // in fact, it is not the raw data but the one data point from data[i]. P.P. 12/27/12

    last_time = raw_data.time;  // save current ret. time

    // Peak recognition state machine
    switch(analyze_segment) {
        case NOISE:                      // noise and baseline calculations
            NoiseAndBaseline();
            break;

        case BETWEEN_PEAKS:              // waiting for a peak
            BetweenPeaks();
            break;

        case PEAK_STARTING:              // Peak starts
            PeakStarting();
            break;

        case PEAK_STILL_STARTING:        // peak still starts
            PeakStillStarting();
            break;

        case PEAK_RISING:                // peak is rising
            PeakRising();
            break;

        case STILL_DESCENDING:           // Peak is descending
            flags=StillDescending();
            break;

        case NEW_PEAK_RISING:            // Apparent new peak starting
            NewPeakRising();
            break;

            // Another Peak has started or the signal has sunk below the
            // Baseline. Store the acquired data and bump peak.Num.
        case NEW_PEAK:
            NewPeak();
            break;

        case PEAK_DESCENDING:            // peak descending
            PeakDescending();
            break;

        case END_OF_PEAK:                // force a peak to end
            flags=EndOfPeak( );
            break;
    }

    // Detection-only negative-peak finder for the production state machine
    // (peak_alg 0/1). Runs alongside the positive state machine. 06/29/26
    if(dq.IsSampleFinished())
        DetectNegativePeak();

    prev_raw_unit = raw_sample;
    prev_scaled_sample = scaled_sample;
    prev_retention_time = raw_data.time;
    prev_raw_invalid = FALSE;  // because we now have valid prev_raw_unit
//  ext_det_done = 0; // added. P.P. 01/21/11

    return flags;
}                                      // END OF DetectPeaks

#include "internal.h"
#include "mainwind.h"
//#if QUICK_SCAN() // added P.P. 01/02/11 to use quick_scan_active
//#ifndef _ACQUIRE_H     //
////#include "acquire.h"   //
//#endif //acquire
//#endif // quick scan
void Integrator::NoiseAndBaseline( void ) //Calculate noise and set baseline
{
    // if we haven't reached N & B Time, get out now.
    long NandBTime = pDetector->NandBtime * data_rate;
    long retention_time = raw_data.time;
    if(retention_time < NandBTime) return;

    long NandBLen = pDetector->NandBlen * data_rate;
#if !OPTICS()
    if( retention_time < (NandBTime + NandBLen) ) {
    temp_base += raw_sample;  // total for average raw unit
                              // difference in raw units
    long TempPnt = prev_raw_invalid? 0: raw_sample - prev_raw_unit;
    temp_noise += labs(TempPnt);   // total the diff. in raw units
    NandBctr++;
  }
  else
#endif
    {                     // if done w/ this state,...
//    #if OPTICS()  // removed. P.P. 02/24/04
        if(NandBctr) {
            noise = (long)( (double)temp_noise / (double)(NandBctr) * 2.0 );
//#if TRACE_GC()    // rem pemp. P.P. 12/11/04
//      noise = noise/64; // added. P.P. 02/24/04
#if NOISE_REDUCT()>1 // cond replaced. noise>1 - divide by the factor. P.P. 12/11/04
            noise = noise/NOISE_REDUCT(); // P.P. 12/11/04
#endif // noise>1
            if(noise == 0)
                noise = 1L;
            // calculate baseline
#if QUICK_SCAN() // case added. P.P. 01/16/11
            if(quick_run) { // count from zero. P.P. 01/16/11
	act_thresh = 0L;
      }
      else {
	act_thresh = (long)(temp_base / (long)NandBctr);
      }
#else // as it always been
            if(pDetector->ext_det) { // case added. P.P. 11/16/08
                act_thresh = 0L;
            }
            else {
                act_thresh = (long)(temp_base / (long)NandBctr);
            }
#endif // quick scan
            pPeakList->_baseline=pPeakList->baseline=act_thresh;
            pPeakList->_baselineslope=0;
        }
        else
//    #endif  // removed. P.P. 02/24/04
        {
            noise = 1L;
            act_thresh = 0L;
        }
        {
            ((MainWindow *)GetApplicationObject()->MainWindow)->print("Noise=%ld\r\n",noise);
            ((MainWindow *)GetApplicationObject()->MainWindow)->print("Baseline=%ld\r\n",act_thresh);
        }
        analyze_segment = BETWEEN_PEAKS;  // next state
    }
}

void Integrator::BetweenPeaks( void )
{
    long height = scaled_sample - act_thresh;
    long half_noise = noise/2;  //added to detect sharp peaks. P.P. 02/24/04
    if(half_noise==0)           //
        half_noise = 1L;            //
#if QUICK_SCAN() // case added. P.P. 01/02/11
    //#if 0 // testing det B as ext, det A as regular. P.P. 01/21/11
  if(!quick_run) {  // P.P. 01/05/11
//  if(!pRun->quick_scan_run) {  // case added. P.P. 01/19/11
    if(ext_det_done) { // ext det time is over - go to regular peak analysis
//    if(1) { // regular analysis from regular detector. P.P 01/17/11
      if((height > noise || pDetector->peak_alg==1) &&
         ((scaled_sample - prev_scaled_sample) > noise) ) { // restored for full noise. P.P. 11/05/10
//        ((scaled_sample - prev_scaled_sample) > half_noise) ) {  // replaced. P.P. 02/24/04
      // Initialize start of peak by Trap. rule.
        peak_area = height * dq.segment_width;
        skim_starting_point = raw_sample;
        peak.From=
        start_of_peak = raw_data.time;

        analyze_segment = PEAK_STARTING;    // next state
      }
    }

    // code added to skip component 0 which is quick scan concentration. P.P. 01/02/11
    else { // ext det not =1 yet
      if(pDetector->components[0].active_yn) {
// added. P.P. 01/16/11
	if(labs((raw_data.time+dq.segment_width) - (pDetector->components[0].peak_rt)*data_rate)<=(pDetector->components[0].window)*data_rate) {  // changed. P.P. 11/21/08
	  peak_area += height * dq.segment_width;
	}
// end
	else if(raw_data.time/data_rate > (pDetector->components[0].peak_rt + pDetector->components[0].window)) {
	  ext_det_done = 1;
	  analyze_segment = END_OF_PEAK;    // all done. Added here P.P. 01/16/11
//added. P.P. 01/16/11
	  peak_area-=height*dq.segment_width;  // 11/23/08
	  peak.Height = peak_area/(2*data_rate*(pDetector->components[0].window));
          peak.Area = peak_area; // 11/23/08
	  peak.Time=(pDetector->components[0].peak_rt)*data_rate;
// end
	}
	else { // nothing to do
        }
      }
      else {
	ext_det_done = 1; // 01/02/11
      }
    }
// end of added code. 01/02/11

  }
  else { // quick scan run - averaging in a window
    if(!ext_det_done) { // still use the same variable, even without checking ext det
      if(pDetector->components[0].active_yn) {
	if(labs((raw_data.time+dq.segment_width) - (pDetector->components[0].peak_rt)*data_rate)<=(pDetector->components[0].window)*data_rate) {  // changed. P.P. 11/21/08
	  peak_area += height * dq.segment_width;
	}
	else if(raw_data.time/data_rate > (pDetector->components[0].peak_rt + pDetector->components[0].window)) {
	  peak_area-=height*dq.segment_width;  // 11/23/08
	  peak.Height = peak_area/(2*data_rate*(pDetector->components[0].window));
          peak.Area = peak_area; // 11/23/08
	  peak.Time=(pDetector->components[0].peak_rt)*data_rate;
	  analyze_segment = END_OF_PEAK;    // all done
	  ext_det_done = 1; // 11/22/08
	}
	else {  // nothing to do
        }
      }
 // rem P.P. 01/17/11
      else {
	ext_det_done = 1; // 01/02/11
      }

    }
//  }
    else { // when it is done - other components are processes as usual peaks. P.P. 12/31/10
 // rem, no other comp in quick scan. P.P. 01/09/11
      if((height > noise || pDetector->peak_alg==1) &&
          ((scaled_sample - prev_scaled_sample) > noise) ) { // restored for full noise. P.P. 11/05/10
//        ((scaled_sample - prev_scaled_sample) > half_noise) ) {  // replaced. P.P. 02/24/04
        // Initialize start of peak by Trap. rule.
        peak_area = height * dq.segment_width;
        skim_starting_point = raw_sample;
        peak.From=
        start_of_peak = raw_data.time;

        analyze_segment = PEAK_STARTING;    // next state
      }

    }
  }
#else // without the quick scan compile - all stays as it was
    if(!pDetector->ext_det) { // as it always had been
        if( /*(height > 0) &&*/
                (height > noise || pDetector->peak_alg==1) &&
                ((scaled_sample - prev_scaled_sample) > noise) ) { // restored for full noise. P.P. 11/05/10
//      ((scaled_sample - prev_scaled_sample) > half_noise) ) {  // replaced. P.P. 02/24/04
            // Initialize start of peak by Trap. rule.
            peak_area = height * dq.segment_width;
#if 0    /* UNUSED CODE  -  BDF  -  08/28/92   11:36 */
            peak_area = height / 2;
#endif
            skim_starting_point = raw_sample;
            peak.From=
            start_of_peak = raw_data.time;

            analyze_segment = PEAK_STARTING;    // next state
        }
        }
        else { // ext det - averaging in a window
        if(!ext_det_done) {
        if(pDetector->components[0].active_yn) {
        if(labs((raw_data.time+dq.segment_width) - (pDetector->components[0].peak_rt)*data_rate)<=(pDetector->components[0].window)*data_rate) {  // changed. P.P. 11/21/08
        peak_area += height * dq.segment_width;
        }
        else if(raw_data.time/data_rate > (pDetector->components[0].peak_rt + pDetector->components[0].window)) {
        peak_area-=height*dq.segment_width;  // 11/23/08
        peak.Height = peak_area/(2*data_rate*(pDetector->components[0].window));
        peak.Area = peak_area; // 11/23/08
        peak.Time=(pDetector->components[0].peak_rt)*data_rate;
        analyze_segment = END_OF_PEAK;    // all done replaced for the line below. P.P. 01/21/11
        ext_det_done = 1; // 11/22/08
        }
        }
        }
        }
#endif // quick scan. P.P. 01/02/11
        }


        void Integrator::PeakStarting( void )
        {
            long height = scaled_sample - act_thresh;
            long half_noise = noise/2;  //added to detect sharp peaks. P.P. 02/24/04
            if(half_noise==0)           //
                half_noise = 1L;            //

            if( /*(height > 0) &&*/
                    (height > noise || pDetector->peak_alg==1) &&
                    ((scaled_sample - prev_scaled_sample) > noise) ) { // hmmm??? // restored for full noise. P.P. 11/05/10
//      ((scaled_sample - prev_scaled_sample) > half_noise) ) { // replaced. P.P. 02/24/04
                peak_area += height * dq.segment_width;
                analyze_segment = PEAK_STILL_STARTING;      // next state
            }
            else
                analyze_segment = BETWEEN_PEAKS;    // next state
        }


        void Integrator::PeakStillStarting( void )
        {
            long height = scaled_sample - act_thresh;
            long half_noise = noise/2;  //added to detect sharp peaks. P.P. 02/24/04
            if(half_noise==0)           //
                half_noise = 1L;            //

            if( /*(height > 0) &&*/
                    (height > noise || pDetector->peak_alg==1) &&
                    ((scaled_sample - prev_scaled_sample) > noise) ) { // hmmm??? // restored for full noise. P.P. 11/05/10
//      ((scaled_sample - prev_scaled_sample) > half_noise) ) { // replaced. P.P. 02/24/04
                peak_area += height * dq.segment_width;
                analyze_segment = PEAK_RISING;      // next state
            }
            else
                analyze_segment = BETWEEN_PEAKS;    // next state
        }


        void Integrator::PeakRising( void )
        {
            long height = scaled_sample - act_thresh;

            peak_area += height * dq.segment_width;
            //Trying to lower rate of decsending. P.P. 11/13/01
//  long half_noise = noise/4; // Does not work. Returned to 1/2. P.P. 11/18/01
            long half_noise = noise/2;
            if(half_noise==0)
                half_noise = 1L;
//  if((prev_scaled_sample - scaled_sample) > half_noise) { // if now descending,...
            //   End. P.P. 11/13/01
            if((prev_scaled_sample - scaled_sample) > noise) { // if now descending,... // restored for full noise. P.P. 11/05/10
                //   Set temporary Peak Height, Total Height and actual Retention
                //   Time and establish Peak Width and Cutoff time.
                peak.Time = raw_data.time;

                peak_half_width = raw_data.time - start_of_peak;
                end_of_peak = raw_data.time +
                              peak_half_width +
                              (peak_half_width / 4);
                //   Store temp. Peak Height
                peak.Height = /*labs*/(height);
//    peak.multiplier = (int) cur_autoscale; // added. P.P. 12/27/12  and removed. 01/03/13
                int range_index = 0;                             // code added to store the multiplier. P.P. 01/11/13
                if(cur_autoscale == 1) range_index = 0;
                else if(cur_autoscale == 10) range_index = 1;
                else if(cur_autoscale == 100) range_index = 2;
                else if(cur_autoscale == 1000) range_index = 3;
                else range_index = 2;
                mult_num[nDetector][peak.Num][range_index] = cur_autoscale;
//    mult_num[nDetector][peak.Num][range_index] = range_index;
                peak.multiplier = mult_num[nDetector][peak.Num][range_index];
                analyze_segment = PEAK_DESCENDING;  // next state
            }
        }

        void Integrator::PeakDescending( void )
        {
            long height = scaled_sample - act_thresh;

            peak_area += height * dq.segment_width;
            //Trying to lower rate of decsending. P.P. 11/13/01
//  long half_noise = noise/4; // Does not work. Returned to 1/2. P.P. 11/18/01
            long half_noise = noise/2;
            if(half_noise==0)
                half_noise = 1L;
//  if((prev_scaled_sample - scaled_sample) > half_noise) { // if now descending,...
            // End. P.P. 11/13/01
            if((prev_scaled_sample - scaled_sample) > noise) {  // if still descending,... // restored for full noise. P.P. 11/05/10
                prev_retention_time = raw_data.time;        // to enter segment six.
                analyze_segment = STILL_DESCENDING;         // next state
            }
            else {
                analyze_segment = PEAK_RISING;  // was just a glitch?
            }
        }


        int Integrator::StillDescending( void )
        {
            long height = scaled_sample - act_thresh;

            // Has another Peak started?
            if( /*(height > 0) &&*/
                    (height > noise || pDetector->peak_alg==1) &&
                    ((scaled_sample - prev_scaled_sample) > noise) ) {// if new peak ascending,...
                // Initialize start of peak by Trap. rule.
                next_peak_area = height * dq.segment_width;
#if 0    /* UNUSED CODE  -  BDF  -  08/28/92   11:47 */
                next_peak_area = height / 2;
#endif
                next_skim_starting_point = raw_sample;
                next_start_of_peak = raw_data.time;
                peak_rising = FALSE;    // Initialize flag

                analyze_segment = NEW_PEAK_RISING;  // next state
            }
            else {                         // else peak still descending
                peak_area += height * dq.segment_width;
                //W.K.analyze_segment = STILL_DESCENDING;
            }

            // Has the signal sunk below the Baseline?
            // Or is it cutoff time?
//  if( (height <= 0) ||    // exclude case of tangent skimming, let signal be allowed to go below baseline. P.P. 07/03/17
            if( ((height <= 0)&& (!(pDetector->peak_alg==1))) ||
                #if /*0*/ 1
                (raw_data.time > end_of_peak ) ) {
#else
                // added code to extend peak end until signal does not drop > than noise. P.P. 11/12/10 from us082010
   ((prev_scaled_sample - scaled_sample) <noise)) {

// end of new code. P.P. 06/09/10  11/12/10
#endif
                next_skim_starting_point = raw_sample;
                next_start_of_peak = raw_data.time;

                /*analyze_segment = END_OF_PEAK; //W.K.*/ return EndOfPeak();
            }
            return 0;
        }


        void Integrator::NewPeakRising( void )
        {
            long height = scaled_sample - act_thresh;

            next_peak_area += height * dq.segment_width;
            if( (scaled_sample - prev_scaled_sample) > noise ) { // if still rising,...
                // .continue w/ new peak.
                analyze_segment = NEW_PEAK;
            }
            else { // Else not a new peak and therefore must set...
                // .previous values to the current point.
                peak_area += next_peak_area;
                analyze_segment = STILL_DESCENDING;
            }
        }


        void Integrator::NewPeak( void )
        {
            long height = scaled_sample - act_thresh;

            next_peak_area += height * dq.segment_width;

            if( (scaled_sample - prev_scaled_sample) > noise ) { // if new peak still rising,...
                // .process the previous peak.
                peak_rising = TRUE;
                /*analyze_segment = END_OF_PEAK; //W.K.*/ EndOfPeak();
            }
            else {  // Else not a new peak and therefore must set...
                // .previous values to the current point.
                peak_area += next_peak_area;
                analyze_segment = STILL_DESCENDING;
            }
        }

        // Detection-only negative-peak finder for the production state machine
        // (peak_alg 0/1). Mirrors the negative-peak logic of the peak_alg==2
        // curve-fit path (see Data::Set in this file): it watches for the signal
        // dipping below the baseline by more than the noise level, tracks the
        // most negative point, and on recovery appends a peak with a NEGATIVE
        // Height. Negative Height (and the default Num) keep the peak excluded
        // from area / height / concentration calculations and from the numbered
        // report table (see the Height>0 / Num>0 guards in PEAK_WND.CPP), so the
        // peak is shown/marked but not quantified. 06/29/26
        void Integrator::DetectNegativePeak( void )
        {
            if(pDetector->ext_det)                // regular detectors only. 06/29/26
                return;
            if(pDetector->peak_alg==2)            // peak_alg==2 finds neg. peaks in Data::Set. 06/29/26
                return;
            if(analyze_segment != BETWEEN_PEAKS) {// only look when not inside a positive peak
                neg_start_of_peak = -1;           // abandon any partial negative peak
                return;
            }

            long depth = act_thresh - scaled_sample; // how far below baseline (>0 = dipping)

            if(depth > noise) {                   // signal is below baseline by > noise
                if(neg_start_of_peak < 0) {       // start of a new negative peak
                    neg_start_of_peak = raw_data.time;
                    neg_peak_min = 0;
                }
                long h = scaled_sample - act_thresh; // negative height
                if(h < neg_peak_min)
                    neg_peak_min = h;             // keep the most negative point
            }
            else if(neg_start_of_peak >= 0) {     // negative peak just ended
                // ignore noise-sized dips, same MinHeight gate used for positive peaks
                if(labs(neg_peak_min) >= (long)(pDetector->MinHeight)) {
                    Peak neg_peak;               // separate from member 'peak'
                    neg_peak.Height  = neg_peak_min;   // negative -> excluded from quant.
                    neg_peak._Height = neg_peak_min;
                    neg_peak.From    = neg_start_of_peak;
                    neg_peak.To      = raw_data.time;
                    neg_peak.Time    =
                    neg_peak._Time   = neg_start_of_peak + (raw_data.time - neg_start_of_peak)/2;
                    neg_peak._HalfWidth = (raw_data.time - neg_start_of_peak)/2;
                    neg_peak.multiplier = (int) cur_autoscale;
                    // Num left at its default so the peak is not numbered/quantified,
                    // only drawn. PeakMatchup() is deliberately not called. 06/29/26
                    pPeakList->append(neg_peak);
                }
                neg_start_of_peak = -1;
            }
        }


        int Integrator::EndOfPeak()
        {
//  if(!pDetector->ext_det) { // regular case. Cond added 11/23/08
            if(peak_rising == TRUE)   // if new peak has already started,...
                analyze_segment = PEAK_RISING;
            else
                analyze_segment = BETWEEN_PEAKS;

            if(!pDetector->ext_det) { // regular case. Cond added 11/23/08

                double cur_peak_area;
                if(pDetector->peak_alg != 1) {  // if baseline projection,...
#if 0    /* UNUSED CODE  -  BDF  -  08/28/92   11:37 */
                    long height = scaled_sample - act_thresh;
      long scratch = height / 2;
      cur_peak_area = peak_area + scratch;
#endif   /* UNUSED CODE */
                    cur_peak_area = peak_area;
                    peak.To=next_start_of_peak; // W.K. for painting only??
                }
                else {                    // else Tangent Skimming
                    skim_ending_point = next_skim_starting_point;
                    peak.To=
                    time_peak_ends = next_start_of_peak;
                    cur_peak_area = peak_area;
#if 0    /* UNUSED CODE  -  BDF  -  08/31/92   15:43 */
                    cur_peak_area = peak_area +  // hmmm???
                   ((prev_scaled_sample-act_thresh)/2);
#endif   /* UNUSED CODE */

                    // Calculate Actual Peak Height.

                    double delta_y = (skim_ending_point - skim_starting_point);
                    double delta_x = (time_peak_ends - start_of_peak);// W.K. / dq.segment_width;
                    double slope = delta_y / delta_x;
                    delta_x = peak_half_width;// W.K. / dq.segment_width;
                    //W.K.double y_intercept;
                    //W.K.if(slope >= 0.0) {               // slope up from the left
                    //W.K.y_intercept = 0.0;
                    delta_y = delta_x * slope;
                    delta_y += (skim_starting_point - act_thresh);
                    //W.K.}
                    //W.K.else {                            // slope down from the left
                    //W.K.  y_intercept = fabs(delta_y);
                    //W.K.  delta_y = (delta_x * slope) + y_intercept;
                    //W.K.  delta_y += (skim_ending_point - act_thresh);
                    //W.K.}
                    peak.Height -= (long)delta_y;

                    // Now Calculate actual Peak Area after Tangent Skim is applied
                    // This is essentially the AREA less the areas of a triangle
                    // and a rectangle.

                    double base = (time_peak_ends - start_of_peak);//W.K. / dq.segment_width;

                    long trapez_height = ((skim_ending_point-act_thresh)+(skim_starting_point-act_thresh))/2;

                    double area_removed = base * trapez_height;
                    cur_peak_area -= area_removed;
                }
                //W.K. if(cur_peak_area < 0)
                //W.K.   cur_peak_area = 0.0;

//    peak.Area = (long)cur_peak_area;
                peak.Area = cur_peak_area;  // now all are double. P.P. 08/25/17
            } // !ext_det

            int flags=0;
            // if peak meets min. area & height requirements,...
            // .recognize the peak.
//  if( peak.Area >= pDetector->MinArea && peak.Height >= pDetector->MinHeight ) {
//  if( peak.Area >= (long)(pDetector->MinArea) && peak.Height >= (long)(pDetector->MinHeight) ) {  // replacement. P.P. 10/03/04
            if( peak.Area >= (double)(pDetector->MinArea) && peak.Height >= (long)(pDetector->MinHeight) ) {  // area is double now. P.P. 08/25/17
                flags=PeakMatchup();            // Match peaks with components.
            }

            // Clear out variables

            peak.To= // next peak
            time_peak_ends       = 0l;
            end_of_peak          = 0l;
            peak_rising          = FALSE;
            peak_area            = next_peak_area;
            peak.From= // next peak
            start_of_peak        = next_start_of_peak;
            skim_starting_point  = next_skim_starting_point;
            prev_retention_time  = 0l;
            return flags;
        }


#ifndef  _HNU_H
#include "hnu.h"
#endif
//// added. P.P. 09/05/03
//#if DAQ2000()   // rem P.P. 08/26/18
#ifndef _ACQUIRE_H     // and added
#include "acquire.h"   //
#endif //acquire
//#endif // daq2000
//// end of added. 09/05/03

#if ANALOG_OUTPUT()                          // added. P.P. 09/07/03
        //static float conc_DAC[POINTS()][NUMDETECTORS][COMPONENT_ROWS]={{{0}}}; //
#endif                                       //
/*
// added. 07/27/07 and removed
#ifndef _MYCBW_H    // added. P.P. 04/27/07
#include "mycbw.h"
#endif
int My_cbAOut(int BoardNum, int Chan, int Gain, USHORT DataValue); // added. P.P. 04/27/07
// end. 07/27/07
*/
        int Integrator::PeakMatchup( void )
        {
            peak.Num++;             // inc. the peak counter for this detector
            return PeakMatchup( &peak, pDetector->peak_alg==2);
        }

//#if DDA() // P.P. 10/06/02. Cond removed 04/15/06 - used without dda as well.
        void Write_conc_to_DAC(Method *pMethod, int port_DAC, float concen_DAC);
//#endif

//static void WriteConcentrationToDAC(int f, double concen, Peak *pPeak, Run *pRun, Method *pMethod, Detector *pDetector, int nDetector)
        static void WriteConcentrationToDAC(int f, double concen, Peak *pPeak, Run *pRun, Method *pMethod, Detector *pDetector, int nDetector, int i) // i added. P.P. 04/10/05
        {
            if(f) { // concen is defined
                int pointnumber = pRun->pointnumber; // moved here. P.P. 09/13/03
#if ANALOG_OUTPUT()&&(!ANALOG_DELAYED()) // P.P. 05/25/04
                if(pDetector->detect_stat) { // detector is ON. If both of them ON - will
      float concen_DAC = concen;  // added. P.P. 10/06/02
#if USB_3112()||USB_3106()||USB_3103()||USB_3101() // changed. P.P. 05/20/07
#if CBW() // cond added. P.P. 05/20/07
#if (POINTS()==1)||ANALOG_MIX() // All points -> to the same outputs.
// add code here later when needed
#if USB_3112()||USB_3103() // P.P. 04/16/06
      if(i<8)   // only 8 outputs available in USB_3112 and 3103 boards
#elif USB_3106() // 3106
      if(i<16)   // only 16 outputs available in USB_3106 board
#elif USB_3101() // 3101
      if(i<4)   // only 4 outputs available in USB_3101 board
#endif
      {
      int Port_DAC = i; // write to the same output
      Write_conc_to_DAC(pMethod, Port_DAC, concen);
      }

#else // multipoint and no mixing
      if(i==0) { // only 1st component
        if(pRun->type_of_run ==JOB) {   // cond added. P.P. 10/03/06
          if(pDetector->detect_stat) {  //
#if 0 // kill the regular variant. P.P. 12/12/08
#if USB_3112()||USB_3103() // P.P. 04/16/06
            if(pointnumber<8) {
#elif USB_3101() // 3101
            if(pointnumber<4) {
#elif USB_3106()// 3106
            if(pointnumber<16) {
#else // no DA board - do not write anything. P.P. 08/26/18
            if(0) {
#endif // one of usb31xx boards
	      Write_conc_to_DAC(pMethod, pointnumber, concen);  // P.P. 04/15/06
            }
// variant with 2 detectors, comp #0 for each det, 4 points. P.P. 12/12/08
#else // not 0 - hence, working code
	    int analog_out_chan = 0;
#if USB_3112()||USB_3103() // P.P. 12/12/08
            if(pointnumber<4) {
	      analog_out_chan = pointnumber + 4*nDetector;
#elif USB_3101() // 3101
            if(pointnumber<2) {
	      analog_out_chan = pointnumber + 2*nDetector;
#elif USB_3106() // 3106
            if(pointnumber<8) {
	      analog_out_chan = pointnumber + 8*nDetector;
#else // no DA board - do nothing.
            if(0) {
#endif // one of usb31xx boards
	      Write_conc_to_DAC(pMethod, analog_out_chan, concen);  // P.P. 12/12/08
            }
#endif // 0. 12/12/08
// end of this variant. 12/12/08
	  } // det is on
        }  // run type == job
      } // i = 0, only 1st component
#endif // points==1
#endif // cbw. 05/20/07
#endif // usb_31xx. P.P. 05/20/07
    } // det == on
// new code added. P.P. 07/25/04
#elif ANALOG_OUTPUT() //delayed(), but peaks are sent during cal/checkcal run. P.P. 07/25/04
                //  if(acquire_type !=JOB) { // cond added - only cal or checkcal data. P.P. 07/25/04
    if(pRun->type_of_run !=JOB) { // changed P.P. 04/10/05
      if(pDetector->detect_stat) { // detector is ON. If both of them ON - will
        float concen_DAC = concen;  // added. P.P. 10/06/02
#if USB_3112()||USB_3106()||USB_3103()||USB_3101() // P.P. 05/20/07
#if (POINTS()==1)||ANALOG_MIX() // All points -> to the same outputs.
// add code here later when needed
        int Port_DAC = i; //
#if USB_3112()||USB_3103() // P.P. 04/16/06, 05/20/07
        if(Port_DAC<8) {  // only 8 outputs available in USB_3112 and usb_3103 boards
	  Write_conc_to_DAC(pMethod, Port_DAC, concen);  // P.P. 10/09/02
        }
#elif USB_3101() // 3101
        if(Port_DAC<4) {  // only 4 outputs available in USB_3101 board
	  Write_conc_to_DAC(pMethod, Port_DAC, concen);  // P.P. 10/09/02
        }
#else // 3106
        if(Port_DAC<16) {  // only 16 outputs available in USB_3106 board board
	  Write_conc_to_DAC(pMethod, Port_DAC, concen);  // P.P. 10/09/02
        }
#endif // one of usb_31xx boards
// end
#else // multipoint and no mixing
/*  // removed - do not write to an out during cal if delayed. P.P. 10/03/06
    if(i==0) { // only 1st component
      Write_conc_to_DAC(pMethod, pointnumber, concen);  //
    }
*/
        int Port_DAC = pointnumber; //
        if(!i) { // output the 1st component only.
#if USB_3112()||USB_3103() // P.P. 04/16/06, 05/20/07
	  if(Port_DAC<8) {  // only 8 outputs available in USB_3112 and usb_3103 boards
	    Write_conc_to_DAC(pMethod, Port_DAC, concen);  // P.P. 10/09/02
	  }
#elif USB_3101() // 3101
	  if(Port_DAC<4) {  // only 4 outputs available in USB_3101 board
	    Write_conc_to_DAC(pMethod, Port_DAC, concen);  // P.P. 10/09/02
	  }
#else // 3106
	  if(Port_DAC<16) {  // only 16 outputs available in USB_3106 board board
	  Write_conc_to_DAC(pMethod, Port_DAC, concen);  // P.P. 10/09/02
          }
#endif // one of usb_31xx boards
        }
#endif // points==1
#endif // usb-3112/3103/3106.
      } // closing det is on
    } // added to close cond !JOB. P.P. 07/25/04
// end of new code. P.P. 07/25/04
#endif // ANALOG_OUTPUT. P.P. 09/05/03 and !delayed
//// end of added. P.P. 10/03/02
            } // if(f)

// end of transferred code. P.P.
        }

        int Integrator::PeakMatchup( Peak *pPeak, int bPreliminary )
        {
            //const int peak=0;

            int flags=0;
            // Search for components that match the current peak for current detector.
//      peak.multiplier = (int) cur_autoscale; // added. P.P. 12/27/12  and removed. 01/03/13
            int match_cnt = 0;
            int ref_comp_idx=pDetector->FindRefCompIndex();
            for(int i=0; i!=COMPONENT_ROWS; i++) {
                pPeak->pComponent=&pDetector->components[i];
                if(pPeak->pComponent->active_yn) { // If component ON,...
                    // if peak matches component, store component which matched,
                    // .and if doing a cal. acquire, store cal. result
                    if(Integrator::CheckRT(pPeak, pPeak->pComponent) == TRUE) { // Component has matched.
                        match_cnt++;
                        pPeakList->append(*pPeak);        // store peak; ( make a copy of peak structure )

                        double concen;
                        int f=pDetector->Concentration(pMethod, pRun, pPeak, nDetector, &concen); // Check Alarms
                        flags|=f;
                        if(!bPreliminary) {
//          WriteConcentrationToDAC(f, concen, pPeak, pRun, pMethod, pDetector, nDetector);
                            WriteConcentrationToDAC(f, concen, pPeak, pRun, pMethod, pDetector, nDetector, i);
                        }

                        if(acquire_type == CAL) { // if cal. results need storing,...
                            if(pDetector->detect_meth == 0) {  // if using height detection method,...
                                cal_results[i][nStandard] =
//                    pPeak->Height * (long)pow10( pRun->runrawdata.gain[nDetector]  );
                                        ((double) pPeak->Height) * (double) pow10( pRun->runrawdata.gain[nDetector]  );  // P.P. 11/18/17 store height as double
                            }
                            else {                   // else using area detect. meth.
                                cal_results[i][nStandard] =
                                        pPeak->Area * (long)pow10( pRun->runrawdata.gain[nDetector] );
                            }
                            // If this is the reference peak and it is in the valid...
                            // .range, validate the cal. for this detector.
                            if(i == ref_comp_idx) {
//          if( peak.Height >= 400 )
                                if( pPeak->Height >= 1000 ) // P.P. 11/23/02
#if 0    /* UNUSED CODE  -  BDF  -  08/18/92   14:21 */
                                    (40000L / (long)pow10( pRun->gain[nDetector] )) )
#endif
                                {
                                    cal_passed = TRUE;
                                }
                            }
                        }
                        break; // Component found
                    }
                }
            }
            pPeak->multiplier = (int) cur_autoscale; // added. P.P. 12/27/12  and moved here 01/03/13
            if(!match_cnt) {
                pPeak->pComponent = NULL;  // "unknown" peak
//#if (!TRACE_GC()) //cond. added not to show unknown peaks' numbers. P.P. 02/24/04
#if (NOISE_REDUCT()>0) // P.P. 09/17/05
                if(bPreliminary) {
      pPeakList->append(*pPeak);
    }
    else {
      if(!pDetector->known_peaks) {
        pPeakList->append(*pPeak);
      }
    }
#else // show all peaks anyway. 2 lines below removed. P.P. 09/17/05
//#if (NOISE_REDUCT()<1) // cond replaced. noise_reduct!=0 - only known peaks shown in Table. P.P. 12/11/04
//    else
                pPeakList->append(*pPeak);    // store peak  Removed. P.P. 12/14/02 - not shown in table.
#endif // noise_reduct >0
            }                             // and restored. P.P. 12/17/02
            return flags;                 // and removed again. P.P. 04/10/03 -> restored 04/11/03
        }




        int Integrator::CheckRT( const Peak *pPeak, Component *pComponent )
        {
            ulong ltemp = labs( pPeak->Time / data_rate - pComponent->peak_rt );
            return ltemp <= pComponent->window;  // Peak matched within Component Data Table
        }



#include "job.h"


        Integration::Integration(Method *_pMethod, PeakList **_pPeakList, Run *_pRun, int _acquire_type, int _nStandard)
                : pMethod(_pMethod), pPeakList(_pPeakList)
                ,acquire_type(_acquire_type), nStandard(_nStandard)
                , pRun(_pRun)
        {
            Job *pJob=(Job *)pMethod;
            CalResults *pCalResults=pJob->GetCalResults(pRun);
            pRun->type_of_run=acquire_type;
            for(int n=0; n<NUMDETECTORS; n++) {
                pIntegrator[n]=NULL;
                Detector *_pDetector=&_pMethod->det[n];
                if(!_pDetector->detect_stat) continue;
                pIntegrator[n]=new Integrator(_pMethod, _pDetector, _pRun, &pCalResults->cal_results[n], _pPeakList[n], _acquire_type, n, _nStandard);
            }

        }
/*
#ifndef  _WCPPLIB_FWSTREAM_H
#include "wcpplib\fwstream.h"
#endif
static void PeaksWrite(owstream &os, PeakList *pPeakList)
{
  if(pPeakList) {
    short n=pPeakList->countChildren(); // Number of Peaks
    os.write(&n,sizeof(n)); // 2 byte   Number of Peaks
    for(PeakList::Node *pNode=pPeakList->first(); pNode; pNode=pNode->nextNode()) {
      Peak *pPeak=(Peak *)pNode;
      long t=pPeak->Time/data_rate;
      os.write(&t,sizeof(t)); // 4 byte in Sec
      os.write(&pPeak->Area,sizeof(pPeak->Area)); // 4 byte in ???*sec (?)
      os.write(&pPeak->Height,sizeof(pPeak->Height));// 4 byte in ???
    }
  }
}

static void PeaksWrite(Integrator **pIntegrator,Run *pRun,PeakList **pPeakList)
{
//  GetWindowsDirectory();
  ofwstream os("c:\\~~peaks~~.tmp");
  os.write(&pRun->type_of_run,sizeof(pRun->type_of_run)); // int 2 byte   2 -> Job, 1 -> Cal
  int16 numdet=NUMDETECTORS; // always = 2
  os.write(&numdet,sizeof(numdet)); // 2 bytes : always 2
  for(int n=0; n<NUMDETECTORS; n++) {
    int16 flag=pIntegrator[n]? 1: 0;
    os.write(&flag,sizeof(flag)); // 2 bytes : 1 - this detector present
    if(pIntegrator[n]) {
      PeaksWrite(os,pPeakList[n]);
    }
  }
}
*/

        Integration::~Integration()
        {
//  PeaksWrite(pIntegrator,pRun,pPeakList);
            for(int n=0; n<NUMDETECTORS; n++) {
                delete pIntegrator[n]; pIntegrator[n]=NULL;
            }
        }



#include "wcpplib\message.h"

        RunIntegration::RunIntegration(Method *_pMethod, PeakList **_pPeakList, Run *_pRun, int _nStandard)
                : Integration(_pMethod, _pPeakList, _pRun, _pRun->type_of_run,  _nStandard), //??
                  pRun(_pRun), iCur(0)
        {
            Job *pJob=(Job *)pMethod;
            RunRawData *runrawdata=pJob->GetRawData(pRun);
            if(!runrawdata) {
                message("Cannot load raw data");
                //return -1;
            }
            for(int n=0; n<NUMDETECTORS; n++) {
                bRun[n]=pIntegrator[n]!=0;
                it[n]=new RawDataIterator(*runrawdata, n);
            }
        }

/*
double f()
{
  double result=0;
  RawData data;
  {
    data
    for(PeakList::Node *p=pPeakList->GetFirst(); p; p=p->GetNext()) {
    }
  }
  return result;
}
*/

//inline double sqr(double x) { return x*x; }
#include "wmath/wmath.h"

        long g_count;

        double form(long x, double Time, double HalfWidth)
        {
            g_count++;

            if(!HalfWidth) return 0;
            double y=(x-Time)/HalfWidth;

            // My Gauss<>Lorenz
            if(y<0)
                goto gaussian;

//  return 0.5*exp(-sqr(y)) + 0.5/(1.+y*y);

            // Lorenzian
            if(fabs(y)>100)
                return 0;

            return 1./(1.+y*y);


            // Laplace or Double exponential
            y=fabs(y);

            if(y>9.61)
                return 0;

            return exp(-y);

            logistic:
            // logistic
            if(fabs(y)>6)
                return 0;

            y=exp(-y);
            return 4.*y/(sqr(1.+y));

            gaussian:
            // gaussian
            if(fabs(y)>3.1/*sqrt(9.61)*/)
                return 0;
            return exp(-sqr(y));
        }
#include <math.h>
#ifndef M_2_SQRTPI
#define M_2_SQRTPI  1.12837916709551257390
#endif

        struct PeakParam {
            double Time;
            double Height;
            double HalfWidth;
            Peak *pPeak;
        };

        struct Params
        {
            double baseline;
            double baselineslope;

            PeakParam *peaks;

            int size;
            Params(int _size);
            Params(const Params& params);
            void operator =(const Params& params);
            ~Params();
            void Set(const PeakList &list);
            void Get(PeakList *list);
            int Contain(Peak *pPeak);
            void Sort();
        };


        Params::Params(int _size)
                : baseline(0)
                , baselineslope(0)
                , peaks(new PeakParam[_size])
                , size(_size)
        {
        }
        Params::Params(const Params& params)
                : baseline(params.baseline)
                , baselineslope(params.baselineslope)
                , peaks(new PeakParam[params.size])
                , size(params.size)
        {
            memcpy(peaks,params.peaks,size*sizeof(peaks[0]));
        }
        void Params::operator =(const Params& params)
        {
            delete [] peaks;

            baseline=params.baseline;
            baselineslope=params.baselineslope;
            size=params.size;
            peaks=new PeakParam[params.size];

            memcpy(peaks,params.peaks,size*sizeof(peaks[0]));
        }
        Params::~Params()
        {
            delete [] peaks;
        }
        void Params::Set(const PeakList &list)
        {
            baseline=list.baseline;
            baselineslope=0;//list.baseline;
            int m=0;
            for(PeakList::Node *pNode=list.first(); pNode; pNode=pNode->nextNode(), m++) {
                peaks[m].Time=pNode->_Time;
                peaks[m].HalfWidth=pNode->_HalfWidth;
                peaks[m].Height=pNode->_Height;
                peaks[m].pPeak=pNode;
            }
            Sort();
        }
        void Params::Sort()
        {
            // sort  by height!
            /**/
            for(int _size=size; _size>2; _size--) {
                int bSorted=1;
                for(int m=1; m<_size; m++) {
                    if(fabs(peaks[m].Height)>=fabs(peaks[m-1].Height)) { // swap
                        PeakParam peak=peaks[m-1];
                        peaks[m-1]=peaks[m];
                        peaks[m]=peak;
                        bSorted=0;
                    }
                }
                if(bSorted) break;
            }
            /**/
        }

        void Params::Get(PeakList *list)
        {
            list->_baseline=baseline;
            list->_baselineslope=baselineslope;
//??  list->baseline=baseline;
            for(int m=0; m<size; m++) {
                Peak *pPeak=peaks[m].pPeak;
                pPeak->_Time=peaks[m].Time;
                pPeak->_HalfWidth=peaks[m].HalfWidth;
                pPeak->_Height=peaks[m].Height;
                pPeak->Time=peaks[m].Time;
                pPeak->Height=peaks[m].Height;
                //pPeak->Area=peaks[m].Height*peaks[m].HalfWidth; //??

                static double norm;
                static int bNormComputed=0;
                if(!bNormComputed) {
                    bNormComputed=1;
                    norm=0;
                    for(int x=-8000; x<8000; x++) {
                        norm+=form(x,0,1000);
                    }
                    norm/=1000;
                }
                pPeak->Area=norm * peaks[m].Height*peaks[m].HalfWidth;
                pPeak->From=peaks[m].Time-2*peaks[m].HalfWidth;
                pPeak->To=peaks[m].Time+2*peaks[m].HalfWidth;
            }
        }
        int Params::Contain(Peak *pPeak)
        {
            for(int m=0; m<size; m++) {
                if(pPeak==peaks[m].pPeak) {
                    return 1;
                }
            }
            return 0;
        }


        struct DataPoint {
            long x;
            int32 y;
            int32 y1;
        };


        struct Data
        {
            double xFrom;
            DataPoint huge *data;
            long size;

            int32 ymin;
            int32 ymax;
            int32 xmin;
            int32 xmax;
            Data();
            ~Data();
            void Set(Method *pMethod, RunRawData *pRunRawData, PeakList *pPeakList, int n);
            double Func(const Params &params);

            void Make0(const Params &params);
            double Func0( const Params &params);
            void Make1(int m1, const Params &params);
            double Func1(int m1, const Params &params);
        };

        Data::Data()
                : xFrom(0)
                , data(NULL)
                , size(0)
                , ymin(0)
                , ymax(0)
                , xmin(0)
                , xmax(0)
        {
        }
        Data::~Data()
        {
            // free arrays
            delete [] data;
        }

//static int NearPeak(int x);
        void Data::Set(Method *pMethod, RunRawData *pRunRawData, PeakList *pPeakList, int n)
        {
            // free arrays
            delete [] data;

            ymin=0x7FFFFFFFL;
            ymax=-0x7FFFFFFFL;
            size=0;
            static const int s=1;
            { // count size
                RawDataIterator it(*pRunRawData, n);
                {
                    int32 yprev=0x7FFFFFFFL;
                    RawData rawdata(pMethod->det[n].segment_width, pMethod->method_table.analysis_time);
                    for(; it.Ok(); it.GetNext()) {
                        int32 y=it.Get();
                        if(s==1) { // raw  or sampled data
                            rawdata.ProcessData(y);
                            if(rawdata.acquire_finished)
                                break;
                            if(!rawdata.dq.IsSampleFinished()) { // if sample finished,...
                                continue;
                            }
                            y=rawdata.dq.GetScaledSample();
                        }
                        if(labs(yprev-y)>labs(y)/1000) {
                            yprev=y;
                            size++;
                        }
                    }
                }
            }
            // alloc array
            data=new huge DataPoint[size];
            { // fill arrays
                RawDataIterator it(*pRunRawData, n);
                {
                    int32 yprev=0x7FFFFFFFL;
                    long l=0;
                    long x=0;//_xmin_;
                    xmin=x;
                    RawData rawdata(pMethod->det[n].segment_width, pMethod->method_table.analysis_time);
                    for(; it.Ok(); it.GetNext(), x++) {
                        int32 y=it.Get();
                        if(s==1) { // raw  or sampled data
                            rawdata.ProcessData(y);
                            if(rawdata.acquire_finished)
                                break;
                            if(!rawdata.dq.IsSampleFinished()) { // if sample finished,...
                                continue;
                            }
                            y=rawdata.dq.GetScaledSample();
                        }

                        xmax=x;
                        if(labs(yprev-y)>labs(y)/1000) {
                            yprev=y;
                            if(y<ymin) ymin=y;
                            else if(y>ymax) ymax=y;
                            data[l].x=x;
                            data[l].y=y;
                            l++;
                        }
                    }
                }
            }
            { // detect negative peaks
                int32 dy=ymax-ymin;
                if(dy/15) {
                    const DataPoint huge *p=data;
                    long peak_start=-1;
                    Peak peak;
                    for(long l=0; l<size; l++, p++) {
                        int32 y=p->y;
                        if(y<pPeakList->baseline-dy/8) {
                            if(peak_start<0) {
                                long x=p->x;
                                peak_start=x;
                                peak.Height=0;
                            }
                            if(peak.Height>p->y-pPeakList->baseline)
                                peak.Height=p->y-pPeakList->baseline;
                        }
                        else if(peak_start>=0) {
                            peak.Num; //??
                            peak._Height=peak.Height;
                            peak._Time=peak.Time=peak_start+(p->x-peak_start)/2;
                            peak._HalfWidth=(p->x-peak_start)/2;
                            peak.From=peak_start;
                            peak.To=p->x;
                            pPeakList->append(peak);

                            peak_start=-1;
                        }
                    }
                }
            }
        }

        Params _pMin(0);
        double _fmin;

#include "debug.h"
#include "mainwind.h"
        double Data::Func(const Params &params)
        {
            long double F=0;
            {
                const DataPoint huge *p=data;
                for(long l=0; l<size; l++, p++) {
                    double x=p->x;
                    long double a=params.baseline+params.baselineslope*x;
                    for(int m=0; m<params.size; m++) {
                        a+=params.peaks[m].Height*form(x,params.peaks[m].Time,params.peaks[m].HalfWidth);
                    }
                    a-=p->y;
                    F+=a*a;
                }
            }
            if(F<_fmin) {
                _pMin=params;
                _fmin=F;
                for(int m=0; m<params.size; m++) {
                    //TRACE2("%12.4e %12.4e", params.Height[m],params.HalfWidth[m]);
                }
                //TRACE1("%12.4e", (double)F);
            }
            return F;
        }
        void Data::Make0(const Params &params)
        {
            {
                DataPoint huge *p=data;
                for(long l=0; l<size; l++, p++) {
                    double x=p->x;
                    long double a=0;
                    for(int m=0; m<params.size; m++) {
                        a+=params.peaks[m].Height*form(x,params.peaks[m].Time,params.peaks[m].HalfWidth);
                    }
                    p->y1=a;
                }
            }
        }
        double Data::Func0(const Params &params)
        {
            long double F=0;
            {
                const DataPoint huge *p=data;
                for(long l=0; l<size; l++, p++) {
                    double x=p->x;
                    double a=params.baseline+params.baselineslope*x + p->y1;
                    a-=p->y;
                    F+=a*a;
                }
            }
            if(F<_fmin) {
                _pMin=params;
                _fmin=F;
                for(int m=0; m<params.size; m++) {
                    //TRACE2("%12.4e %12.4e", params.Height[m],params.HalfWidth[m]);
                }
                //TRACE1("%12.4e", (double)F);
            }
            return F;
        }
        void Data::Make1(int m1, const Params &params)
        {
            {
                DataPoint huge *p=data;
                for(long l=0; l<size; l++, p++) {
                    double x=p->x;
                    long double a=params.baseline+params.baselineslope*x;
                    for(int m=0; m<params.size; m++) {
                        if(m==m1) continue;
                        a+=params.peaks[m].Height*form(x,params.peaks[m].Time,params.peaks[m].HalfWidth);
                    }
                    p->y1=a;
                }
            }
        }
        double Data::Func1(int m1, const Params &params)
        {
            long double F=0;
            {
                const DataPoint huge *p=data;
                for(long l=0; l<size; l++, p++) {
                    double x=p->x;
                    long double a=p->y1;
                    {
                        a+=params.peaks[m1].Height*form(x,params.peaks[m1].Time,params.peaks[m1].HalfWidth);
                    }
                    a-=p->y;
                    F+=a*a;
                }
            }
            if(F<_fmin) {
                _pMin=params;
                _fmin=F;
                for(int m=0; m<params.size; m++) {
                    //TRACE2("%12.4e %12.4e", params.Height[m],params.HalfWidth[m]);
                }
                //TRACE1("%12.4e", (double)F);
            }
            return F;
        }

        struct _param {
            Data data;

            Method *pMethod;
            RunRawData *pRunRawData;
            PeakList *pPeakList;
            int n;
            PeakList::Node *pNode;
        } _p;

        Params _p1(0);

        int _m1;
        static double _f1(double x)
        {
            _p1.peaks[_m1].HalfWidth=x;
            //return _p.data.Func(_p1);
            return _p.data.Func1(_m1,_p1);
        }
        static double _f2(double x)
        {
            _p1.peaks[_m1].Height=x;
            //return _p.data.Func(_p1);
            return _p.data.Func1(_m1,_p1);
        }
        static double _f3(double x)
        {
            _p1.peaks[_m1].Time=x;
            //return _p.data.Func(_p1);
            return _p.data.Func1(_m1,_p1);
        }

#include "wmath/brent.h"
#define ZEPS 1.0e-10

        double _brent(double ax,double bx,double cx,double (*f)(double),double tol,double *xmin)
        {
            double x1=cx;
            double f1=f(x1);

            double dx=2*tol*fabs(x1)+ZEPS;

            double x2=x1+dx;
            double f2=f(x2);
            double x3=x1;
            double f3=f1;
            if(f2<f1) {
                x3=x1;
                f3=f1;
                x1=x2;
                f1=f2;
                dx=dx*3;
                for(;;) {
                    if(x1+dx>=bx) {
                        x2=bx;
                        break;
                    }
                    x3=x1-dx;
                    f3=f(x3);
                    if(f3>=f1) {
                        break;
                    }
                    x3=x1;
                    f3=f1;
                    x1=x2;
                    f1=f2;
                    dx=dx*3;
                }
            }
            else {
                for(;;) {
                    x3=x1-dx;
                    f3=f(x3);
                    if(f3>=f1) {
                        break;
                    }
                    x2=x1;
                    f2=f1;
                    x1=x3;
                    f1=f3;
                    dx=dx*3;
                    if(x1-dx<=ax) {
                        x3=ax;
                        break;
                    }
                }
            }

            return brent(x3,x1,x2,f,tol,xmin);
        }


        Params _p0(0);

        static double _f(double x)
        {
            _p1.baseline=x;
            //return _p.data.Func(_p1);
            return _p.data.Func0(_p1);
        }
        static double _f0(double x)
        {
            _p1.baselineslope=x;
            //return _p.data.Func(_p1);
            return _p.data.Func0(_p1);
        }

        void NewDetectPeaks(Job *pJob, Method *pMethod, Run *pRun, Integrator *pIntegrator, PeakList *pPeakList, int n)
        {
            /*
            static double aa;
            aa=0;
            for(int x=0; x<8000; x++) {
              aa+=form(x,0,1000);
            }
            aa*=M_2_SQRTPI;
            */
            if(pJob) {
                RunRawData *pRunRawData=pJob->GetRawData(pRun);
                if(pRunRawData) {
                    if(pPeakList->first()) { // cond added. P.P. 10/07/05
                        for(PeakList::Node *pNode=pPeakList->first(); pNode; pNode=pNode->nextNode()) {
                            //if(pNode==pPeakList->first()) continue;
                            pNode->_Time=pNode->Time;
                            pNode->_Height=pNode->Height;

                            pNode->_HalfWidth=(pNode->Height /* *pMethod->det[n].segment_width*/)?
                                              pNode->Area/(pNode->Height /* *pMethod->det[n].segment_width*/): 1;
                            pNode->_HalfWidth/=2;

                            pNode->_HalfWidth*=M_2_SQRTPI;//aa = 2/sqrt(3.1415926);
                            //pNode->Time;                  // retention time for this peak
                            //long Area;                      // area for this peak
                            //long Height;                    // height for this peak
                            //long From;                    // retention time for begin of this peak
                            //long To;                      // retention time for end of this peak
                        }
                        pPeakList->_baseline=pPeakList->baseline;
                        pPeakList->_baselineslope=0;

                        /////////
                        _p.data.Set(pMethod, pRunRawData, pPeakList, n);
                        _p.pMethod=pMethod;
                        _p.pRunRawData=pRunRawData;
                        _p.pPeakList=pPeakList;
                        _p.n=n;

                        Params params(_p.pPeakList->countChildren());
                        params.Set(*_p.pPeakList);
                        _p0=params;

                        _pMin=params;
                        _fmin=MAXDOUBLE;

                        _p.data.Func(params);

                        // slope is zero yet

                        double ymin=pPeakList->_baseline-(_p.data.ymax-_p.data.ymin)/5;
                        if(ymin<_p.data.ymin) ymin=_p.data.ymin;
                        double ymax=pPeakList->_baseline+(_p.data.ymax-_p.data.ymin)/5;
                        if(ymin>_p.data.ymax) ymax=_p.data.ymax;
                        //_brent(ymin,pPeakList->_baseline,ymax,_f,1e-3,&pPeakList->_baseline);
                        //_f(pPeakList->_baseline);
                        g_count=0;
                        {
                            double f=0;
                            double fprev=MAXDOUBLE/2;
                            {
                                _p1=_p0;
                                for(int i=0; i<10; i++) {
                                    _p1.Sort();

                                    if(i>=1) {
                                        double Amin=fabs(_p1.peaks[0].Height)*fabs(_p1.peaks[0].HalfWidth)*1e-5;
                                        double Hmin=fabs(_p1.peaks[0].Height)*3e-3;
                                        for(int m=1; m<_p1.size; m++) {
                                            if(fabs(_p1.peaks[m].Height)<Hmin || fabs(_p1.peaks[m].Height)*fabs(_p1.peaks[m].HalfWidth)<Amin) {
                                                if(m>4) {
                                                    _p1.peaks[m].pPeak->_Height=0;

                                                    if(_p1.size!=(m+1))
                                                        memcpy(_p1.peaks+m,_p1.peaks+(m+1),(_p1.size-(m+1))*sizeof(_p1.peaks[0]));
                                                    _p1.size--;
                                                    m--;
                                                }
                                            }
                                        }
                                        if(i>2)
                                            if(_p1.size>10) {
                                                _p1.size=10;
                                                _fmin=MAXDOUBLE; //???
                                            }
                                    }

                                    _p.data.Make0(_p1);

                                    f=brent(ymin,pPeakList->_baseline,ymax,_f,1e-4,&pPeakList->_baseline);
                                    //f=_f(_p1.Height[m]);

                                    long dx=_p.data.xmax-_p.data.xmin;
                                    long dy=_p.data.ymax-_p.data.ymin;
                                    if(dx) {
                                        f=brent(-dy*0.75/dx,pPeakList->_baselineslope,+dy*0.75/dx,_f0,1e-4,&pPeakList->_baselineslope);
                                    }

                                    double tol=1.e-4;
                                    if(i==0) tol=1e-2;
                                    else if(i==1) tol=6e-3;
                                    else if(i==2) tol=9e-4;
                                    else if(i==3) tol=4e-4;


                                    int s=_p1.size;
                                    s=i<2? s-2*s/3: i<3? s-s/2: i<4? s-s/3: s;


                                    for(int m=0; m<s; m++) {
                                        _m1=m;

                                        _p.data.Make1(_m1, _p1);

                                        f=brent(_p1.peaks[m].pPeak->_HalfWidth/3,_p1.peaks[m].HalfWidth,_p1.peaks[m].pPeak->_HalfWidth*3,_f1,tol,&_p1.peaks[m].HalfWidth);

                                        f=brent(_p1.peaks[m].Height/7,_p1.peaks[m].Height,_p1.peaks[m].Height*3,_f2,tol,&_p1.peaks[m].Height);

                                        //f=brent(_p1.peaks[m].Time-_p1.peaks[m].HalfWidth/4,_p1.peaks[m].Time,_p1.peaks[m].Time+_p1.peaks[m].HalfWidth/4,_f3,tol,&_p1.peaks[m].Time);
                                        f=brent(_p1.peaks[m].pPeak->Time-_p1.peaks[m].HalfWidth/3,_p1.peaks[m].Time,_p1.peaks[m].pPeak->Time+_p1.peaks[m].HalfWidth/3,_f3,tol,&_p1.peaks[m].Time);

                                    }
                                    if(i>4 && fabs(f-fprev)<=1e-5*f)
                                        break;
                                    fprev=f;
                                }
                            }
                        }


                        _pMin.Get(_p.pPeakList);


                        PeakList list(_p.pPeakList->Window, _p.pPeakList->id);
                        {
                            for(PeakList::Node *pNode=_p.pPeakList->first(); pNode; pNode=pNode->nextNode()) {
                                if(_pMin.Contain(pNode)) {
                                    list.append(*pNode, 1);
                                }
                            }
                        }
                        _p.pPeakList->clear();
                        PTWindowsObject pPeakWindow=list.Window->FindPeakWindow(list.id);
                        if(pPeakWindow)
                            ::SendMessage(pPeakWindow->HWindow,WM_REFRESH, 0, 0L);

                        if(pRun->type_of_run == CAL) { // if cal. results need storing,...
                            memset(pIntegrator->cal_results,0,sizeof(pIntegrator->cal_results));
                        }
                        {
                            for(PeakList::Node *pNode=list.first(); pNode; pNode=pNode->nextNode()) {
                                pIntegrator->PeakMatchup( pNode, 0 );
                            }
                        }
                    } // added. P.P. 10/07/05
                }
            }
        }

        void NewDetectPeaks(Job *pJob, Method *pMethod, Run *pRun, Integrator *pIntegrator[NUMDETECTORS], PeakList **pPeakList)
        {
/*
  static double aa;
  aa=0;
  for(int x=0; x<8000; x++) {
    aa+=form(x,0,1000);
  }
  aa*=M_2_SQRTPI;
*/
            for(int n=0; n<NUMDETECTORS; n++) {
                Detector *pDetector=&pMethod->det[n];
                if(pDetector->detect_stat && pDetector->peak_alg==2) {
                    if(pPeakList[n]) {
                        NewDetectPeaks(pJob, pMethod, pRun, pIntegrator[n], pPeakList[n], n);
                    }
                }
            }
        }

        RunIntegration::~RunIntegration()
        {
/**/
            Job *pJob=(Job *)pMethod;
            NewDetectPeaks(pJob, pMethod, pRun, pIntegrator, pPeakList);
/**/

            for(int n=0; n<NUMDETECTORS; n++) {
                delete it[n];
            }
        }

#ifndef  _JOB_H
#include "job.h"
#endif
#ifndef  _WCPPLIB_MESSAGE_H
#include "wcpplib\message.h"
#endif

        int RunIntegration::Continue()
        {
            Job *pJob=(Job *)pMethod;
#ifdef FILTER
            {
    FiltData *pFiltData;
    for(int n=0; n<NUMDETECTORS; n++) {
      pFiltData=pJob->GetFiltData(pRun,n);
      if(pFiltData) {
        if(bRun[n]) {
          Integrator *p=pIntegrator[n];
          if(p) {                               // detector may be off
            p->gain=pFiltData->gain;
            for(int iCur=0; iCur<pFiltData->size; iCur++) {
              p->ProcessData(pFiltData->p[iCur]);
              p->DetectPeaks();
            }
            bRun[n]=0;
          }
        }
      }
    }
  }
#endif//FILTER
            int bStop=1;
            for(int n=0; n<NUMDETECTORS; n++) {
                if(bRun[n]) bStop=0;
            }
            if(bStop) return 1;

            RunRawData *runrawdata=pJob->GetRawData(pRun);
            if(!runrawdata) {
                message("Cannot load raw data");
                return -1;
            }
#if 1
            for(int num=0; num<NUMDETECTORS; num++) {
                if(!bRun[num]) continue;
                Integrator *p=pIntegrator[num];
                if(!p) continue; // detector may be off
                for(int k=0; k<50 && it[num]->Ok(); k++,iCur++,it[num]->GetNext()) { // Get next 50 points
                    if(p) {
                        /*if(pRun->runrawdata.nRawDataFormat==0) {
                          p->ProcessRawData(it[num]->Get()); //??
                        }
                        else*/
                        p->ProcessData(it[num]->Get());
                        p->DetectPeaks();
                    }
                }
                if(!it[num]->Ok()) {
                    bRun[num]=0;
                }
            }
#else
            uint32 detectors;
  int kmax=runrawdata.nRawDataFormat==2? 33*3: 17*3;
  for(int k=0; k<kmax; k++,iCur++) { // Get next 51 points
    if(!(iCur<runrawdata.RawDataSize)) return 1;        // The end
    if(runrawdata.nRawDataFormat==2) {
      uint32 data=runrawdata.pRawData32[iCur];
      int n=k%33;
      if(n==0) {
        detectors=data;
        continue;
      }
      n--;
      int det_idx = (detectors&(0x80000000UL>>n))? 1: 0;     // get the detector ID
      if(bRun[det_idx]) {
        Integrator *p=pIntegrator[det_idx];
        if(p) {                         // detector may be off
          p->ProcessData(data);
          p->DetectPeaks();
        }
      }
      continue;
    }
    if(runrawdata.nRawDataFormat==1) {
      uint16 data=runrawdata.pRawData16[iCur];
      int n=k%17;
      if(n==0) {
        detectors=data;
        continue;
      }
      n--;
      int det_idx = (detectors&(0x8000U>>n))? 1: 0;     // get the detector ID
      if(bRun[det_idx]) {
        Integrator *p=pIntegrator[det_idx];
        if(p) {                         // detector may be off
          p->ProcessData(data);
          p->DetectPeaks();
        }
      }
      continue;
    }
    uint16 data=runrawdata.pRawData16[iCur];
    int det_idx = (data >> 15)? 1: 0;     // get the detector ID
    if(bRun[det_idx]) {
      Integrator *p=pIntegrator[det_idx];
      if(p) {                           // detector may be off
        p->ProcessRawData(data);
        p->DetectPeaks();
      }
    }
  }
#endif
            return 0;
        }