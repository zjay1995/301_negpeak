#pragma hdrstop

#ifndef  _WCPPLIB_WSTREAM_H
#include "wcpplib\wstream.h"
#endif

#ifndef  _DETECTOR_H
#include "detector.h"
#endif

#include "calc.h" // TWA_xxx
#include "conio.h" // P.P. 02/27/05

Detector::Detector()
 :
  NandBtime(1), NandBlen(1),
  peak_alg(0),
  detect_meth(0),
//  gain(0),
  gain(2),    // changed. P.P. 04/22/06
  gain_1(2),    //  P.P. 11/09/12
  auto_gain_on_off(0),
  plot_atten(0),
  detect_stat(1),
  MinArea(0),
  MinHeight(0),
//  segment_width(1),
  segment_width(10), // P.P. 02/27/05, to avoid long peak fit time.
  units("PPM"),
  uv_type(0),  // added. P.P. 10/11/02
//  AnalogRange(100)    //added. P.P. 03/19/02
//  , lamp_save(1)
   lamp_save(0)  // changed. P.P. 08/26/05  07/27/07
  , known_peaks(0) // P.P. 09/17/05
  , ext_det(0)     // P.P. 11/14/08
  , invert(0)     // P.P. 03/25/09, here 11/06/09
  , data_to_excel(0)  // P.P. 02/24/12
  {
// added 07/27/07
  for(int i=0; i<COMPONENT_ROWS; i++) {
    AnalogRange[i] = 100UL;
  }
// end 07/27/07
}

Detector::~Detector() {}


void *Detector::read(iwstream &is, unsigned long version)
{
  if(version>=14) Label.read(is);
  else Label="?"+Label;

  is>>NandBtime>>NandBlen;
  is>>peak_alg;
  is>>detect_meth;
  is>>gain;
  if(version>=13) is>>auto_gain_on_off; else auto_gain_on_off=0;
  is>>plot_atten;
  is>>detect_stat;
  is>>MinArea;
  is>>MinHeight;
  is>>segment_width;
  units.read(is);
  if(version>8) {
    is>>lamp_save;
  }
  else {
//    lamp_save=1;
    lamp_save=0;    // changed. P.P. 04/22/06
  }
  if(version>=18) { // we add uv_type to the dedector dlg. P.P. 10/11/02
    is>>uv_type;
  }
  else {
    uv_type = 0;
  }
 // changed 07/27/07
  if(version>=19) {  // added. P.P. 06/14/03
//    is>>AnalogRange; // and aded here 09/06/04
    is>>AnalogRange[0]; // changed. 07/27/07
  }

  if(version>=24) { // added. P.P. 09/17/05
    is>>known_peaks;
  }
  else {
    known_peaks = 0;
  }
// added 07/27/07
  if(version>=28) {  // added. P.P. 04/15/07
//    for(int i = 0; i<COMPONENT_ROWS; i++) { // P.P. 04/22/07
    for(int i = 1; i<COMPONENT_ROWS; i++) { // P.P. 07/27/07
      is>>AnalogRange[i];
    }
  }
  else {
  }
// end 07/27/07
  if(version>=29) {  // added. P.P. 11/14/08
    is>>ext_det;
  }
  if(version>=30) {  // added. P.P. 03/25/09, here 11/06/09
    is>>invert;
  }
  if(version>=34) {  // added. P.P. 02/24/12
    is>>data_to_excel;
  }
  if(version>=36) {
    is>>gain_1;
    auto_gain_on_off=0; // added, to exclude and disable this option. P.P. 10/17/14
  }
  for(int i=0; i<COMPONENT_ROWS; i++) {
    components[i].read(is, version);
  }
  return this;
}

void Detector::write(owstream &os)
{
  Label.write(os);

  os<<NandBtime<<NandBlen;
  os<<peak_alg;
  os<<detect_meth;
  os<<gain;
  os<<auto_gain_on_off;
  os<<plot_atten;
  os<<detect_stat;
  os<<MinArea;
  os<<MinHeight;
  os<<segment_width;
  units.write(os);
  os<<lamp_save;
  os<<uv_type;  // added. P.P. 10/11/02
//  os<<AnalogRange; //Added. P.P. 06/14/03 added here 09/06/04
  os<<AnalogRange[0]; // 07/27/07
  os<<known_peaks; // P.P. 09/17/05
// added 07/27/07
  for(int i = 1; i<COMPONENT_ROWS; i++) { // P.P. 07/27/07
    os<<AnalogRange[i];
  }
// end 07/27/07
  os<<ext_det; // P.P. 11/14/08
  os<<invert;  // P.P. 03/25/09, here 11/06/09
  os<<data_to_excel; // P.P. 02/24/12
  os<<gain_1; // P.P. 11/09/12
//  for(int i=0; i<COMPONENT_ROWS; i++) {
  for(i=0; i<COMPONENT_ROWS; i++) {   // 07/27/07
    components[i].write(os);
  }
}


int Detector::FindRefCompIndex()
{
  for(int i=0; i<COMPONENT_ROWS; i++) {
    if(components/*standards*/[i].ref_peak) {
      return i;
    }
  }
  return -1; // default to no ref. component
}

Component *Detector::FindRefComp()
{
  Component *pComponent=components/*standards*/;
  for(int i=0; i<COMPONENT_ROWS; i++, pComponent++) {
    if(pComponent->ref_peak) {
      return pComponent;
    }
  }
  return 0;
}

#ifndef __MATH_H
#include <math.h> // pow10()
#endif

#include "peak.h"
#include "method.h"
#include "run.h"
#include "internal.h" // POINTS()

static int GetAlarmFlags(double concen, Component *pComponent, Method *pMethod, Run *pRun)
{
  int flags=0;

  if(pMethod->method_table.alarms_on_off) {
    int pointnumber=pRun->pointnumber;
#if POINTS()>1
    ulong m=1L<<pointnumber;
#endif
    if((pComponent->high_alarm[pointnumber]>0.0) && (concen>=pComponent->high_alarm[pointnumber])) {
#if POINTS()>1
      if(pMethod->method_table.halarms&m)
#endif
	flags|=2; // High Alarm occured
    }
//	else if((pComponent->low_alarm>0.0) && (concen<=pComponent->low_alarm)) {
    //To change Low Alarm from range to level. P.P. 10/16/01
//      }
    else if((pComponent->low_alarm[pointnumber]>0.0) && (concen>=pComponent->low_alarm[pointnumber])) {
#if POINTS()>1
      if(pMethod->method_table.lalarms&m)
#endif
	flags|=4; // Low Alarm occured
    }
  }
  return flags;
}

int Detector::Concentration(Method *pMethod, Run *pRun, Peak *pPeak, int nDetector, double *pConcentration)
{
  int flags=0; // Initially is Unknown
  if(pConcentration) *pConcentration=0;

  Component *pComponent=pPeak->pComponent;
  if(pComponent) {                // if peak is "known",...
    double area_or_height;
    if(detect_meth == 0) { // if using height,...
      area_or_height = pPeak->Height;
    }
    else {
      area_or_height = pPeak->Area;
    }
    area_or_height*=(uint32)pow10( pRun->runrawdata.gain[nDetector] );

    BOOL b=FALSE;
    {
      for(int j1=STAND_NUM-1; j1>0; j1--) {
	if(pComponent->lstart[j1]) {
	  b=TRUE;
	}
      }
    }


    if(b) { // multipoint calibration
      // find interval
      for(int j1=STAND_NUM; j1>0; j1--) {
	if(pComponent->lstart[j1]) {
	  if(pComponent->lstart[j1]<=area_or_height)
	    break;
	}
      }

      double concen = pComponent->lvalue[j1] + pComponent->lslope[j1] * (area_or_height-pComponent->lstart[j1]);

      flags|=1; // found
      flags|=GetAlarmFlags(concen, pComponent, pMethod, pRun);
      if(pConcentration) *pConcentration=concen;
    }
    else { // single point calibration
      // calc. the concentration of this component.
      double slope = pComponent->slope;
      Component *pRefComponent=FindRefComp();
      double ref_slope = pRefComponent? pRefComponent->slope: 0.0;
      double rf;                        // temp. response factor
      if(slope > 0.0) {         // if slope is valid,...
        rf = 1.0;		 // .the response factor isn't used.
      }
      else if(ref_slope > 0.0) { // if ref. slope is valid,...
        rf = pComponent->resp_fact; // .use calculated response factor.
        slope = ref_slope;
      }
      else {
        rf = 0.0;
      }

      if( (rf > 0.0) &&               // if concentration info. valid,...
	  (slope > 0.0) /*&&
	  (acquire_type != CAL)*/ ) {
        double concen = rf * slope * area_or_height;
	if(/*row < peak_idx[det_idx]*/1) {  // if the row has peak info,...
  	  flags|=1; // found
          flags|=GetAlarmFlags(concen, pComponent, pMethod, pRun);

          if(pConcentration) *pConcentration=concen;
        }
      }
    }
  }
  return flags;
}


float TWA_results[POINTS()][NUMDETECTORS][COMPONENT_ROWS];  	// TWA results
float max_TWA_conc[POINTS()][NUMDETECTORS][COMPONENT_ROWS];	// max. conc. in TWA
float min_TWA_conc[POINTS()][NUMDETECTORS][COMPONENT_ROWS];	// min. conc. in TWA. added. P.P. 12/22/04
uint32 TWA_start_time;      			   	// time TWA cycle began
int TWA_runs[POINTS()][NUMDETECTORS][COMPONENT_ROWS]; // P.P. 01/11/03	// # of runs in a TWA with active comps
DWORD TWA_cycle_start_time; // added P.P. 11/24/01
float concen_point[POINTS()][NUMDETECTORS][COMPONENT_ROWS] = {{{0}}};  //added for current concentrations in points. P.P. 12/24/02
int flags_point[POINTS()][NUMDETECTORS][COMPONENT_ROWS];  //added for current concentrations in points
long time_point[POINTS()][NUMDETECTORS][COMPONENT_ROWS];  //added for current concentrations in points
time_t start_time[POINTS()][NUMDETECTORS][COMPONENT_ROWS];  //added for current concentrations in points
#if USE_TOTAL()  // parameters added. P.P. 12/26/04
float TWA_total[POINTS()][NUMDETECTORS];  	// TWA for total
float max_TWA_total[POINTS()][NUMDETECTORS];	// total max. in TWA
float min_TWA_total[POINTS()][NUMDETECTORS];	// total min. in TWA
float total_point[POINTS()][NUMDETECTORS] = {{{0}}};  //added for total concentrations in points. P.P. 12/26/04
int TWA_runs_total[POINTS()][NUMDETECTORS] = {{{0}}}; // P.P. 12/26/04
int flags_point_total[POINTS()][NUMDETECTORS];  //added for total concentrations in points. P.P. 12/26/04
#endif

#if ETHUS() // P.P. 12/16/18
float concen0_point[POINTS()] = {0};  //added for current conc of 1st comp for detA in points. P.P. 12/16/18
#endif
#if TOTAL_SUMMARY()
TotalList total_list;
void InitTotalList()
{
  total_list.clear();
}
#include <time.h>
void AddConcentration(int pointnumber, int n, float conc, time_t start_time)
{
  TotalListNode *pNode=(TotalListNode *)total_list.last();
  if(pNode==0 || pNode->tim[pointnumber][n]) { // node already used
    pNode=new TotalListNode;
    memset(pNode->conc,0,sizeof(pNode->conc));
    memset(pNode->tim,0,sizeof(pNode->tim));
    total_list.append(pNode);
  }
  pNode->tim[pointnumber][n]=start_time;//time(&pNode->tim[pointnumber][n]);
  pNode->conc[pointnumber][n]=conc;
}
#endif // TOTAL_SUMMARY()

#if !ETHUS() // regular GC
uint16 GetModBusRegister(uint16 n)
{
  int i=n/2;
  int j=n%2;

  static const float *arrays[]={
    &TWA_results[0][0][0],
    &max_TWA_conc[0][0][0],
    &min_TWA_conc[0][0][0],
    &concen_point[0][0][0],
  };
  static const arraysnum=sizeof(arrays)/sizeof(arrays[0]);

  int a=i/(POINTS()*NUMDETECTORS*COMPONENT_ROWS); // the number of array
  int b=i%(POINTS()*NUMDETECTORS*COMPONENT_ROWS); // the number of element in array

  if(a<arraysnum) {
    const float *array=arrays[a];
    uint32 reg=*(uint32 *)&array[b];// get float as 32bit int (two 16bit integers)

    if(j==0) {
      return (uint16)(reg>>16); // high 16 bits
    }
    else {
      return (uint16)reg; // low 16 bits
    }
  }

#if USE_TOTAL()  // parameters added. P.P. 12/26/04
  {
    i-=arraysnum*(POINTS()*NUMDETECTORS*COMPONENT_ROWS); // shift i to the beginning
    static const float *arrays[]={
      &TWA_total[0][0],
      &max_TWA_total[0][0],
      &min_TWA_total[0][0],
      &total_point[0][0],
    };
    static const arraysnum=sizeof(arrays)/sizeof(arrays[0]);

    int a=i/(POINTS()*NUMDETECTORS); // the number of array
    int b=i%(POINTS()*NUMDETECTORS); // the number of element in array
    if(a<arraysnum) {
      const float *array=arrays[a];
      uint32 reg=*(uint32 *)&array[b];// get float as 32bit int (two 16bit integers)

      if(j==0) {
        return (uint16)(reg>>16); // high 16 bits
      }
      else {
        return (uint16)reg; // low 16 bits
      }
    }
  }
#endif

  return 0;
}

#else // ETHUS - only 1st comp in det A comp list. P.P. 12/16/18
uint16 GetModBusRegister(uint16 n)
{
  int i=n/2;
  int j=n%2;

  const float *array = &concen0_point[0];

  int a=i/POINTS(); // the number of array
  int b=i%POINTS(); // the number of element in array

  if(a<1) { // to exclude requests for registers out of range. P.P. 12/16/18
    const float *array = &concen0_point[0];
    uint32 reg=*(uint32 *)&array[b];// get float as 32bit int (two 16bit integers)

    if(j==0) {
      return (uint16)(reg>>16); // high 16 bits
    }
    else {
      return (uint16)reg; // low 16 bits
    }
  }
  else if(a==1) {   // added for sending the acive points info as a 32-bit uint32. P.P. 12/16/18
    Method *pMethod;
    uint32 reg = (uint32) pMethod->method_table.points;
    if(j==0) {
      return (uint16)(reg>>16); // high 16 bits
    }
    else {
      return (uint16)reg; // low 16 bits
    }
  }

  return 0;
}
#endif // !ETHUS

 // No changes in STEL at this moment. P.P.
float STEL_results[STEL_RUNS][NUMDETECTORS][COMPONENT_ROWS];	// STEL results
float max_STEL_conc[NUMDETECTORS][COMPONENT_ROWS];	// max. conc. in a STEL */
int   STEL_q_idx;					// current index into the STEL Qs
uint32 STEL_start_time;					// time STEL cycle began
int   STEL_runs;					// # of runs in a STEL

#include "comm.h"// GetTime()
void InitTWA()
{
  for(int p=0; p<POINTS(); p++) {
//    TWA_runs[p]=0;		   // replaced. P.P. 01/11/03	
    for(int n=0; n<NUMDETECTORS; n++) {
      for(int i=0; i<COMPONENT_ROWS; i++) {
	TWA_results[p][n][i]=0;
	max_TWA_conc[p][n][i]=0;
	min_TWA_conc[p][n][i]=0;  // added. P.P. 12/22/04
	concen_point[p][n][i] = 0;  //at start of shift. P.P. 12/24/02
	flags_point[p][n][i] = 0;  //at start of shift.
	time_point[p][n][i] = 0;  //at start of shift.
	start_time[p][n][i] = 0;  //at start of shift.
	TWA_runs[p][n][i] = 0; // P.P. 01/11/03
      }
#if USE_TOTAL() // added. P.P. 12/26/04
      TWA_total[p][n]=0;      //
      max_TWA_total[p][n]=0;  //
      min_TWA_total[p][n]=0;  // added. P.P. 12/26/04
      total_point[p][n] = 0;  //at start of shift. P.P. 12/26/04
      TWA_runs_total[p][n] = 0; // P.P. 12/26/04
      flags_point_total[p][n] = 0;  //at start of shift. P.P. 12/26/04
#endif
    }
#if ETHUS() // P.P. 12/16/18
    concen0_point[p] = 0;  //added for current conc of 1st comp for detA in points. P.P. 12/16/18
#endif
  }
  TWA_cycle_start_time = GetTime(); // store last init time

#if TOTAL_SUMMARY()
  InitTotalList();
#endif
}

void InitSTEL()
{
  for(int n=0; n<NUMDETECTORS; n++) {
    for(int i=0; i<COMPONENT_ROWS; i++) {
      for(int r=0; r<STEL_RUNS; r++) {
	STEL_results[r][n][i]=0;
      }
      max_STEL_conc[n][i]=0;
    }
  }
  STEL_q_idx = 0;
}



#ifndef  _METHOD_H
#include "method.h"
#endif
#ifndef  _PEAKLIST_H
#include "peaklist.h"
#endif
void TWA_calc(Method *pMethod, Run *pRun, PeakList **pPeakList, int pointnumber)
{
//  float concentration[NUMDETECTORS][COMPONENT_ROWS];  // added for current concentration. P.P. 12/10/01
//  float concen_point[POINTS()][NUMDETECTORS][COMPONENT_ROWS];  //added for current concentrations in points. P.P. 12/24/02
  float concen_total[NUMDETECTORS];  // added for total concentration. P.P. 11/09/02
  void Write_conc_to_DAC(Method *pMethod, int port_DAC, float concen_DAC); // P.P. 10/09/02

  for(int n=0; n<NUMDETECTORS; n++) {
    if(!pPeakList[n]) continue;
    concen_total[n] = 0;
    Detector *pDetector=&pMethod->det[n];
   if(pMethod->det[n].detect_stat) { // to skip non-active detector. P.P. 11/10/02
// added for start total as 0. P.P. 12/26/04
#if USE_TOTAL() // added. P.P. 12/26/04
     total_point[pointnumber][n] = 0;  //start adding components. P.P. 12/26/04
     flags_point_total[pointnumber][n] = 0;  //start adding flags. P.P. 12/26/04
#endif // use total
// end of added init. P.P. 12/26/04
#if QUICK_SCAN() // added code for quick scan compile. P.P. 01/05/11
    int scan_first = 0; // added. P.P. 01/05/11
    int scan_last = COMPONENT_ROWS;
    if(!pRun->quick_scan_run) {
      scan_first = 1;
    }
    else {
      scan_last = 1;
    }
    for(int i = scan_first; i<scan_last; i++) {
#else
    for(int i=0; i<COMPONENT_ROWS; i++) {
#endif // quick scan
      Component *pComponent=&pDetector->components[i];
      double concen=0;
      int flags=0;
      long time=0;
      for(PeakList::Node *ppln=pPeakList[n]->first(); ppln; ppln=ppln->nextNode()) {
	if(ppln->pComponent==pComponent) {
          double c=0;
	  int f=pDetector->Concentration(pMethod, pRun, ppln, n, &c);
//	  if(!f) { // concentration is not valid
	  if((!f)||(c<0)) { // concentration is not valid or negative. P.P. 10/01/05
	    c=0;
	  }
	  flags|=f;
          concen+=c;
#if USE_TOTAL()
	  flags_point_total[pointnumber][n]|=flags; // added. P.P. 12/26/04
#endif
	  extern char data_rate;           // rate data received in pts/sec.
	  time=ppln->Time/data_rate;
	}
      }
      // if peak not found - concen is ZERO
// 2 lines replaced. P.P. 01/11/03
//      TWA_results[pointnumber][n][i] = (TWA_results[pointnumber][n][i]*TWA_runs[pointnumber] +
//	  concen)/(TWA_runs[pointnumber] + 1);    // add concen, find average.
      if(!pRun->bSkipTWA) { // exclude run with FID error from TWA calculations. P.P. 08/27/05
	TWA_results[pointnumber][n][i] = (TWA_results[pointnumber][n][i]*TWA_runs[pointnumber][n][i] +
	  concen)/(TWA_runs[pointnumber][n][i] + 1);    // add concen, find average.
      }
#if USE_TOTAL() // added. P.P. 12/26/04
      total_point[pointnumber][n]+=concen; // adding all components' concentrations. P.P. 12/26/04
#endif
      if(!pRun->bSkipTWA) { // exclude run with FID error from TWA calculations. P.P. 08/27/05
	TWA_runs[pointnumber][n][i]++; // added. P.P. 01/11/03
        // store the maximum concentration used in this TWA.
        if(max_TWA_conc[pointnumber][n][i]<concen) max_TWA_conc[pointnumber][n][i]=concen;
        if(TWA_runs[pointnumber][n][i]<2) { // if the 1st run. P.P. 12/23/04
	  min_TWA_conc[pointnumber][n][i] = concen; // to start with. P.P. 12/23/04
        }
        else {
	  if(min_TWA_conc[pointnumber][n][i]>concen) {
	    min_TWA_conc[pointnumber][n][i] = concen;
	  }
        }
      }
      //concentration[n][i]=concen; // Store for write to DAC later
      float concen_DAC = concen; // added. P.P. 10/06/02
      concen_total[n] += concen; // Store for write to DAC later
      concen_point[pointnumber][n][i] = concen; // P.P. 12/24/02. Added to refresh concen in all points.
#if ETHUS() // P.P. 12/16/18
      if(!(n||i)) {  // only for det A and component 0. P.P. 12/16/18
        concen0_point[pointnumber] = concen;  //added for current conc of 1st comp for detA in points. P.P. 12/16/18
      }
#endif
      flags_point[pointnumber][n][i] = flags;
      time_point[pointnumber][n][i] = time;
      start_time[pointnumber][n][i] = pRun->start_time;
// Copied from wp102602 and modified to write >5 an outputs. P.P. 11/09/02
////Added to write concen of up to 5 components to DDA analog outputs. P.P. 10/04/02
#if ANALOG_OUTPUT() // case added. P.P. 04/16/06. Without DDA board - use other boards.
#if USB_3112()||USB_3106()||USB_3103()||USB_3101() // P.P. 05/20/07
#if (POINTS()==1)||ANALOG_MIX() // All points -> to the same outputs.
#if USB_3112()||USB_3103() // P.P. 04/16/06
	if(i<8)   // only 8 outputs available in USB_3112 and 3103 boards 
#elif USB_3106() // 3106
	if(i<16)   // only 16 outputs available in USB_3106 board 
#elif USB_3101() // 3101
	if(i<4)   // only 4 outputs available in USB_3101 board
#else
        if(0) 
#endif
        {
	  int Port_DAC = i; // write to the same output
	  Write_conc_to_DAC(pMethod, Port_DAC, concen);
	}
#else // multipoint and without analog mix. Only 1 comp to out. 
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
#elif USB_3106() 
//#if ANALOG_DELAYED() // cond. added. P.P. 09/16/06  and removed - write zero if not found! 10/03/06
	  if(Port_DAC<16) {  // only 16 outputs available in USB_3106 board board
	    Write_conc_to_DAC(pMethod, Port_DAC, concen);  // P.P. 10/09/02
          }
#else // no ao board  - do nothing. P.P. 08/26/18

#endif // one of usb_31xx boards
        }
#endif  // point==1 or analog_mix
#endif // usb3** AO boards 
#endif // analog output. Added here to have total without analog output. P.P. 12/26/04
      /*
      STEL_results[STEL_q_idx][n][i] = concen;  // store STEL concentration
      // store the maximum concentration used in this STEL.
	  TWA_runs[pointnumber] += 1;
		      if(max_STEL_conc[n][i]<concen) max_STEL_conc[n][i]=concen;
      */
    }
#if TOTAL_SUMMARY()
    AddConcentration(pointnumber, n, total_point[pointnumber][n], pRun->start_time);
#endif
#if USE_TOTAL() // code added. P.P. 12/26/04
      if(!pRun->bSkipTWA) { // exclude run with FID error from TWA calculations. P.P. 08/27/05
        TWA_total[pointnumber][n] = (TWA_total[pointnumber][n]*TWA_runs_total[pointnumber][n] +
	  total_point[pointnumber][n])/(TWA_runs_total[pointnumber][n] + 1);    // add concen, find average.
        // store the maximum concentration used in this TWA.
        if(max_TWA_total[pointnumber][n]<total_point[pointnumber][n]) max_TWA_total[pointnumber][n]=total_point[pointnumber][n];
        if(TWA_runs_total[pointnumber][n]<2) { // if the 1st run. P.P. 12/26/04
	  min_TWA_total[pointnumber][n] = total_point[pointnumber][n]; // to start with. P.P. 12/26/04
        }
        else {
	  if(min_TWA_total[pointnumber][n]>total_point[pointnumber][n]) {
	    min_TWA_total[pointnumber][n] = total_point[pointnumber][n];
	  }
        }
        TWA_runs_total[pointnumber][n]++; // P.P. 12/26/04
      }
#endif
   }
  }
#if ANALOG_OUTPUT() // case added, use USB boards. P.P. 09/10/06
//added. P.P. 09/10/06
#if USE_TOTAL()
  float concen_DAC_total = 0; // initialize. P.P. 
#if USE_DIFFERENCE()               //
  float concen_DAC_difference = 0; // 
#endif // use_difference
  if(pMethod->det[0].detect_stat) {  // if det A ON - ignore det B for total. Or we should add separate analog
                          // outputs for det B. P.P. 11/10/02
    concen_DAC_total = concen_total[0];  // only det A. P.P. 11/09/02
    Detector *pDetector=&pMethod->det[0];  // added to exclude latest (det B) component_rows. P.P. 11/10/02
    Component *pComponent=pDetector->components;
  }
  else if((!pMethod->det[0].detect_stat)&&(pMethod->det[1].detect_stat)) { // det A is OFF, Det B ON. P.P. 11/10/02
     concen_DAC_total = concen_total[1];  // only det B. P.P. 11/09/02
     Detector *pDetector=&pMethod->det[1];  //
     Component *pComponent=pDetector->components;
  }
  else  {  // both detectors are OFF - never happens. P.P. 11/10/02
  }
#endif // total
#if (POINTS()==1)||ANALOG_MIX() // Added here. P.P. 12/05/04. All points -> to the same outputs.
#if USE_TOTAL() // 
#if USB_3112()||USB_3103()  // 3103 added. 05/20/07
#if POINTS()<8
  int Port_DAC_total = 7; // write to the last output. P.P.
  Write_conc_to_DAC(pMethod, Port_DAC_total, concen_DAC_total);  // P.P. 09/10/06
#endif // points
#elif USB_3106()
#if POINTS()<16
  int Port_DAC_total = 15; // write to the last output.  P.P.
  Write_conc_to_DAC(pMethod, Port_DAC_total, concen_DAC_total);  // P.P. 09/10/06
#endif // points
#elif USB_3101() // added. P.P. 05/20/07
#if POINTS()<4
  int Port_DAC_total = 3; // write to the last output.  P.P.
  Write_conc_to_DAC(pMethod, Port_DAC_total, concen_DAC_total);  // P.P. 09/10/06
#endif // points
#endif // usb 3112 or 3106 or 3103 or 3101
#endif // use_total
#endif // points==1 or analog mix. P.P. 12/05/04
// end. P.P. 09/10/06

#endif //ANALOG_OUTPUT(), without DDA. P.P.
//  TWA_runs[pointnumber]++;  // removed - done above. P.P. 01/11/03
}
// Modified as in wp102602. P.P. 11/09/02
//#include "wow.h"   // added. P.P. 07/25/03  
#if CBW() // cond. added. P.P. 08/29/03
#include "MyCBW.h" // added. P.P. 07/25/03
#endif // cbw()
void Write_conc_to_DAC(Method *pMethod, int port_DAC, float concen_DAC)
{
  Detector *pDetector =&pMethod->det[0];
 // Added to write concen to DAC. P.P. 12/14/01
#if USB_3112()||USB_3106()||USB_3103()||USB_3101() // P.P. 09/10/06, 05/20/07
  const unsigned int Max_DAC_Value = 65535;  //for 16-bit DAC, unsigned. P.P. 04/15/06
#else
  const unsigned int Max_DAC_Value = 4095;  //for 12-bit DAC, unsigned. P.P.
#endif // usb_3112
//  const float Max_DAC_Volts = 2048.0; // for 12-bit DAC. P.P.
//  const double Volts_Full_Scale = 5.0;
//  const double Max_concen = 100.00;  // Changed. P.P. 03/19/02
  double DAC_Value = 0;
  double Max_concen = 0;
  double conc_DAC= (double) concen_DAC; // P.P. 04/21/07
//  const double Max_concen = pDetector->AnalogRange;  // Should be defined in mainwind.h. P.P. - do this later.
// added. 07/27/07
  int num_of_outputs = 0;
#if USB_3101()
  num_of_outputs = 4;
#elif USB_3103()
  num_of_outputs = 8;
#elif USB_3106()
  num_of_outputs = 16;  
#elif USB_3112()
  num_of_outputs = 8;
#else // no board - no outputs. P.P. 08/26/18
  num_of_outputs = 0;
#endif
#if (POINTS()==1)||ANALOG_MIX() // Cond. added P.P. 04/28/07. All points -> to the same outputs.
//  if(port_DAC<num_of_outputs) {
  if((port_DAC<num_of_outputs)&&(port_DAC<10)) { // changed - only 10 exist in det dialog. P.P. 12/27/07
    Max_concen = (double) pDetector->AnalogRange[port_DAC];  // P.P. 04/23/07
  }
  else {
    Max_concen = (double) pDetector->AnalogRange[0];
  }
#else // multipoint, no mixing - only 1st comp.
  if((port_DAC<num_of_outputs)&&(port_DAC<10)) { // changed - 10 points can be set in det dialog. P.P. 12/27/07
    Max_concen = (double) pDetector->AnalogRange[port_DAC];  // P.P. 04/23/07
  }
  else {
    Max_concen = (double) pDetector->AnalogRange[0];  // P.P. 04/28/07
  }
#endif
// end. 07/27/07
//  if(concen_DAC > (double) Max_concen)
//    concen_DAC = (double) Max_concen;
  if(conc_DAC > Max_concen) conc_DAC = Max_concen;  // 07/27/07
  uint data_value = 0; // moved here. 07/27/07
  if(Max_concen) { // cond added. P.P. 04/16/07
//  if(concen_DAC <0) concen_DAC = 0; // added. P.P. 08/28/03 Rem 08/29/03
#if USB_3106() // make 4-20 mA out of 0-20 mA range. P.P. 09/10/06
    DAC_Value = ((1000.00*conc_DAC)/Max_concen)*Max_DAC_Value*0.80;
    data_value = (uint) ((DAC_Value/1000.00)+Max_DAC_Value/5);
    if(data_value>Max_DAC_Value) data_value=Max_DAC_Value;            // 20 mA
    else if(data_value<(Max_DAC_Value/5)) data_value=Max_DAC_Value/5; // 4 mA
#else
    DAC_Value = ((1000.00*conc_DAC)/Max_concen)*Max_DAC_Value;
    data_value = (uint) (DAC_Value/1000.00);
#endif // usb_3106,
  }
  if(data_value > Max_DAC_Value) data_value = Max_DAC_Value; // added P.P. 12/12/02
  // for +-5V DAC - zero iz in the middle. P.P. 12/14/01
//  data_value = Max_DAC_Value + data_value;
//  int DAC_channel = 2*(pointnumber + 1);
  //int BASE1 = 0x310;
//  if(extern int pointnumber <= 4) // we have only 5 analog outputs available in DDA06/Jr. P.P.
#if CBW()// with cbw only. 07/27/07
#if USB_3112() // case added. P.P. 04/15/06 - no DDA board
  if(port_DAC<8) {
    My_cbAOut(CBWDEVICE_3112, port_DAC, UNI10VOLTS, (USHORT) data_value); // 
  }
#elif USB_3106() // case added. P.P. 09/10/06
  if(port_DAC<16) {
    My_cbAOut(CBWDEVICE_3106, port_DAC, UNI10VOLTS, (USHORT) data_value); // 
  }
#elif USB_3103() // case added. P.P. 05/20/07
  if(port_DAC<8) {
    My_cbAOut(CBWDEVICE_3103, port_DAC, UNI10VOLTS, (USHORT) data_value); // 
  }
#elif USB_3101() // case added. P.P. 05/20/07
  if(port_DAC<4) {
    My_cbAOut(CBWDEVICE_3101, port_DAC, UNI10VOLTS, (USHORT) data_value); // 
  }
#endif // usb AO boards
#endif // cbw
// End of added. P.P.
}
// Initialization of DACs. P.P. 21/22/01
void InitDAC() // the function changed to initialize DAC of DDA board. P.P. 03/26/03
{
/// added to initialize analog outputs to zero. P.P. 03/26/03
}



