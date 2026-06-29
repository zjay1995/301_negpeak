#pragma hdrstop

#ifndef  _WCPPLIB_WSTREAM_H
#include "wcpplib\wstream.h"
#endif

#ifndef  _METHOD_H
#include "method.h"
#endif

#ifndef __STDIO_H
#include <stdio.h>
#endif

MethodTable::MethodTable()
 :inj_det_temp(45),
  oven_temp(45),
  line_temp(0), // added.  P.P. 03/28/17
  desorb_temp(0), // added P.P. 07/15/18 for the case of 3 temp zones
  fid_temp(0),  // added P.P. 07/16/18 - for the case when 4 temp zones -  oven+inj/det+oncentrator+fid
  analysis_time(15*60),
#if MANUAL_INJ()  // cond added. P.P. 05/19/06
  sample_time(5),
  inject_time(5),
#else
  sample_time(1),
  inject_time(1),
#endif // manual_inj
  equilibrate_time(5),    // added. P.P. 04/06/02
//  next_point_time(30),    // added. P.P. 04/09/02
//  purge_time(30),    // added for usage as next point sample ahead. P.P. 12/19/02
  next_point_time(0),    // sddrd. P.P. 04/09/02
  purge_time(0),    // disabled. P.P. 05/25/04. next point sample ahead. P.P. 12/19/02
  calibrate_time(15*60), // added P.P. 12/19/02
  x1_time(0),            // added. P.P. 07/29/04
  x1_off_time(0),            // added. P.P. 06/18/06
  x2_time(0),            // added. P.P. 06/20/10
  x2_off_time(0),            
  x3_time(0),            // added. P.P. 06/20/10
  x3_off_time(0),            
  x4_time(0),            // added. P.P. 06/20/10
  x4_off_time(0),            
  x5_time(0),            // added. P.P. 07/01/10
  x5_off_time(0),            
#if MANUAL_INJ()  // cond added. P.P. 05/19/06
  syringe_on_off(1),
#else
  syringe_on_off(0),
#endif // manual inj
  TWA_on_off(0),
  TWA_cycle(1),
  cal_cycle(0),
  repeat_cycle(1),
  alarms_on_off(0),
  check_alarm(0),  // added. P.P. 04/06/03
  STEL_on_off(0),
  paper_save_on_off(0),
  cont_pump_on_off(0),
  plotter_on_off(0)
//  plotter_on_off(0),
//  desorb_temp(0),    // 2 lines added. P.P. 02/11/06  and removed 02/20/06

  ,RampingEnabled(0) // bool
  ,OvenHoldTime1(0) // time
  ,OvenRampR1(0)
  ,OvenHoldT2(0)     // temp
  ,OvenHoldTime2(0) // time
  ,OvenRampR2(0)
  ,OvenHoldT3(0)     // temp
  ,OvenHoldTime3(0)  // time

  ,points(1) // first bit set
  ,lalarms(1)
  ,halarms(1)

  ,set_point(0)
  ,set_point_1(0)
  ,set_point_2(0)
  ,set_point_3(0)
  ,TempSlope(100.0)    // 2 lines added. P.P. 07/20/06
  ,TempIntercept(0.0) //
  ,TempSetting(0)   // added. P.P. 07/21/06
  ,second_pump_on_off(0) // added. P.P. 01/30/09
//  ,second_pump_on_off(1) //  P.P. 02/01/09
  ,dsb_preheat_time(0) // P.P. 03/07/10
  ,quick_scan_time(0) // P.P. 12/14/10
  ,quick_scan_enabled(0) // added. P.P. 12/14/10
  ,zero_gas_time(0)      // P.P. 03/15/11
  ,zero_gas_cycle(0)     //
  ,oven_door_manual(0)   // P.P. 09/06/12
  ,desorb_time(0)     //  restored P.P. 03/12/17
  ,dsb_wait_time(0) // P.P. 03/26/18
{
}

MethodTable::~MethodTable() {}

#ifndef  _UTL_H
#include "utl.h"
#endif

#ifndef  _DATASTRC_H
#include "datastrc.h"
#endif

void *MethodTable::read(iwstream &is, unsigned long version)
{
  is>>inj_det_temp;
  is>>oven_temp;
  is>>analysis_time;
  is>>sample_time;
  is>>inject_time;
  is>>syringe_on_off;
  is>>TWA_on_off;

  if(version<5) {
    int   _TWA_cycle;    is>>_TWA_cycle;    TWA_cycle   =_TWA_cycle;
    int   _cal_cycle;    is>>_cal_cycle;    cal_cycle   =_cal_cycle;
    int   _repeat_cycle; is>>_repeat_cycle; repeat_cycle=_repeat_cycle;
  }
  else {
    ulong _TWA_cycle;    is>>_TWA_cycle;    TWA_cycle   =_TWA_cycle   /3600;
    ulong _cal_cycle;    is>>_cal_cycle;    cal_cycle   =_cal_cycle   /3600;
    ulong _repeat_cycle; is>>_repeat_cycle; repeat_cycle=_repeat_cycle/3600;
  }

  is>>alarms_on_off;
  is>>STEL_on_off;
  is>>paper_save_on_off;
  is>>cont_pump_on_off;
  is>>plotter_on_off;
  if(version>=4) {
    is>>RampingEnabled; // bool
    is>>OvenHoldTime1; // time
    is>>OvenRampR1;
    is>>OvenHoldT2;     // temp
    is>>OvenHoldTime2; // time
    is>>OvenRampR2;
    is>>OvenHoldT3;     // temp
    is>>OvenHoldTime3;  // time
  }
  if(version>=10) {
    is>>points;
  }
  if(version>=11) {
    is>>lalarms;
    is>>halarms;
  }
  ////////////
//  if(version>=17) {    // added. P.P. 12/19/02
  if(version>=20) {    // changed to read 301 version 19 jobs. P.P. 09/11/04
    is>>purge_time;
  }
  if(version>=20) {    // added. P.P. 12/19/02
    is>>calibrate_time;
    is>>equilibrate_time;
  }
  if(version>=21) {   // added. P.P. 04/06/03
    is>>check_alarm;
  }
  //////////////
  if(version>=20) {
    is>>set_point;
    is>>set_point_1;
    is>>set_point_2;
    is>>set_point_3;
  }

  if(version>=22) {  // added. P.p. 07/29/04
    is>>x1_time;
  }
/* // removed as not needed at this time. P.P. 02/20/06
  if(version>>=25) {   // added. P.P. 02/11/06
    is>>desorb_time;
    is>>desorb_temp;
  }
*/
  if(version>=25) {  // added. P.P. 06/18/06
    is>>x1_off_time;
  }

  if(version>=26) { // added. P.P. 07/20/06
    is>>TempSlope;
    is>>TempIntercept;
    is>>TempSetting;
  }

  if(version>=31) { // added. P.P. 03/07/10
    is>>second_pump_on_off;
    is>>dsb_preheat_time;
  }

  if(version>=32) { // added. P.P. 06/20/10
    is>>x2_time;
    is>>x2_off_time;
    is>>x3_time;
    is>>x3_off_time;
    is>>x4_time;
    is>>x4_off_time;
    is>>x5_time;        // added P.P. 07/01/10
    is>>x5_off_time;
  }

  if(version>=33) { // added. P.P. 12/14/10
    is>>quick_scan_time;
    is>>quick_scan_enabled;
    is>>zero_gas_time;  // added P.P. 03/15/11
    is>>zero_gas_cycle; //
  }

  if(version>=35) { // added. P.P. 09/07/12
    is>>oven_door_manual;
  }

  if(version>=38) {   // added. P.P. 03/12/17
    is>>desorb_time; // added P.P. 03/12/17
//    is>>desorb_temp; // not needed - inj_det temp used for this. P.P. 03/12/17
  }

  if(version>=39) {   //added for heated lines. P.P. 03/28/17
    is>>line_temp;
  }

  if(version>=40) {   //added for next point time, separate from purge_time used popreviously. P.P. 06/05/17
    is>>next_point_time;
  }

  if(version>=42) {     // added. P.P. 03/26/18
    is>>dsb_wait_time;
  }

    if(version>=43) {     // added. P.P. 07/15/18
    is>>desorb_temp;
    is>>fid_temp;
  }

  /////////////
  return this;
}

void MethodTable::write(owstream &os)
{
  os<<inj_det_temp;
  os<<oven_temp;
  os<<analysis_time;
  os<<sample_time;    
  os<<inject_time;
//  os<<calibrate_time;    // 3 lines added. P.P. 12/19/02 
//  os<<purge_time;                                        
//  os<<equilibrate_time;

  os<<syringe_on_off;
  os<<TWA_on_off;
  {
//  os<<TWA_cycle;      
//  os<<cal_cycle;
//  os<<repeat_cycle;
  ulong _TWA_cycle   =TWA_cycle   *3600UL; os<<_TWA_cycle;
  ulong _cal_cycle   =cal_cycle   *3600UL; os<<_cal_cycle;
  ulong _repeat_cycle=repeat_cycle*3600UL; os<<_repeat_cycle;
  }
  os<<alarms_on_off;
  os<<STEL_on_off;
  os<<paper_save_on_off;
  os<<cont_pump_on_off;
  os<<plotter_on_off;

  os<<RampingEnabled; // bool
  os<<OvenHoldTime1; // time
  os<<OvenRampR1;
  os<<OvenHoldT2;     // temp
  os<<OvenHoldTime2; // time
  os<<OvenRampR2;
  os<<OvenHoldT3;     // temp
  os<<OvenHoldTime3;  // time

  os<<points;

  os<<lalarms;
  os<<halarms;
  /////////////////
  os<<purge_time;
  os<<calibrate_time;    // 3 lines added. P.P. 12/19/02
  os<<equilibrate_time;
  os<<check_alarm;       // added. P.P. 04/06/03
  /////////////////
  {
    os<<set_point;
    os<<set_point_1;
    os<<set_point_2;    // 2 lines added. P.P. 02/11/06
    os<<set_point_3;    //
  }
  os<<x1_time;    // added. P.P. 07/30/04
//  os<<desorb_time;   // 2 lines added, but removed 02/20/06 as not needed. P.P. restored - now it is separate from equi time
//  os<<desorb_temp;
  /////////////////
  os<<x1_off_time;  // added. P.P. 06/18/06
  os<<TempSlope;     // added. P.P. 07/20/06
  os<<TempIntercept; //
  os<<TempSetting;   // P.P. 07/21/06
  os<<second_pump_on_off; // 2 lines added P.P. 03/07/10
  os<<dsb_preheat_time;   //
  os<<x2_time;            // X2 - X4 times added. P.P. 06/20/10
  os<<x2_off_time;
  os<<x3_time;
  os<<x3_off_time;
  os<<x4_time;
  os<<x4_off_time;
  os<<x5_time;        // added. P.P. 07/01/10
  os<<x5_off_time;
  os<<quick_scan_time;    // 2 lines added. P.P. 12/14/10
  os<<quick_scan_enabled; //
  os<<zero_gas_time;  // added. P.P. 03/15/11
  os<<zero_gas_cycle; 
  os<<oven_door_manual; // added. P.P. 09/07/12
  os<<desorb_time; // added P.P. 03/12/17
  os<<line_temp; // P.P. 03/28/17
  os<<next_point_time; // P.P. 06/05/17
  os<<dsb_wait_time;   // P.P. 03/26/18
  os<<desorb_temp;   // P.P. 07/15/18
  os<<fid_temp;   // P.P. 07/16/18
}

Method::Method()
 : method_needs_transmitting(TRUE)
{
  for(int n=0; n<NUMDETECTORS; n++) {
    char str[32];
    sprintf(str,"Detector %c",'A'+n);
    det[n].Label.Set(str);
  }
}

Method::~Method(){}

#ifndef __STRING_H
#include <string.h>
#endif

int Old2bool(int b)  // PeakWork 4.0 for DOS had on==0 and off==1 !!!!
{
  return b? 0: 1;
}

void *Method::readOld(iwstream &is)
{
  uchar file_type;
  is>>file_type;

  _meth_tbl m;
  is>>m;

  method_table.inj_det_temp=m.inj_det_temp;
  method_table.oven_temp=m.oven_temp;
  method_table.analysis_time=ConvertTime(m.analysis_time);
  method_table.sample_time=ConvertTime(m.sample_time);
  method_table.inject_time=ConvertTime(m.inject_time);
  method_table.syringe_on_off=Old2bool(m.syringe_on_off);
  method_table.TWA_on_off=Old2bool(m.TWA_on_off);
  method_table.TWA_cycle=m.TWA_cycle; // *3600
  method_table.cal_cycle=m.cal_cycle; // *3600
  method_table.repeat_cycle=m.repeat_cycle; // *3600
  method_table.alarms_on_off=Old2bool(m.alarms_on_off);
  method_table.STEL_on_off=Old2bool(m.STEL_on_off);
  method_table.paper_save_on_off=Old2bool(m.paper_save_on_off);
  method_table.cont_pump_on_off=Old2bool(m.cont_pump_on_off);
  method_table.plotter_on_off=Old2bool(m.plotter_on_off);


  det[0].NandBtime=ConvertTime(m.NandBtime[0]);
  det[1].NandBtime=ConvertTime(m.NandBtime[1]);
  det[0].NandBlen=ConvertTime(m.NandBlen[0]);
  det[1].NandBlen=ConvertTime(m.NandBlen[1]);
  det[0].peak_alg=m.peak_alg[0];
  det[1].peak_alg=m.peak_alg[1];
  det[0].detect_meth=m.detect_meth[0];
  det[1].detect_meth=m.detect_meth[1];
  det[0].gain=m.gain[0];
  det[1].gain=m.gain[1];
  det[0].plot_atten=m.plot_atten[0];
  det[1].plot_atten=m.plot_atten[1];
  det[0].detect_stat=Old2bool(m.detect_stat[0]);
  det[1].detect_stat=Old2bool(m.detect_stat[1]);
  det[0].MinArea=m.threshold[0];
  det[1].MinArea=m.threshold[1];
  det[0].segment_width=m.segment_width[0];
  det[1].segment_width=m.segment_width[1];
  det[0].units.Set(m.units[0]);
  det[1].units.Set(m.units[1]);

  _cpnts    _components[NUMDETECTORS][COMPONENT_ROWS];
  is.read(_components,sizeof(_components));
  is.seek(720,wstream::cur);
  _cal_srnd _standards[NUMDETECTORS][COMPONENT_ROWS];
  is.read(_standards,sizeof(_standards));

  for(int n=0; n<NUMDETECTORS; n++) {
    for(int i=0; i<COMPONENT_ROWS; i++) {
      det[n].components[i].name=_components[n][i].name;
      det[n].components[i].active_yn=_components[n][i].active_yn;
      det[n].components[i].peak_rt=ConvertTime(_components[n][i].peak_rt);
      det[n].components[i].window=ConvertTime(_components[n][i].window);
      det[n].components[i].resp_fact=_components[n][i].resp_fact;
      for(int p=0; p<MAX_POINTS; p++) {
	det[n].components[i].low_alarm[p]=_components[n][i].low_alarm;
	det[n].components[i].high_alarm[p]=_components[n][i].high_alarm;
      }
      det[n].components[i].ref_peak=_standards[n][i].ref_peak;
      for(int j=0; j<STAND_NUM; j++) det[n].components[i].stand[j]=_standards[n][i].stand[j];
    }
  }



  char notes[10][40];
  is.read(notes,sizeof(notes));
  char buff[10*40]; buff[0]='\0';
  for(int i=0; i<10; i++) {		// glue in one array
    strcat(strcat(buff,notes[i]),"\n");
  }
  char *cp1=NULL;			// cut trailing '\n'
  for(char *cp=buff; *cp; cp++) {
    if(*cp=='\n') { if(!cp1) cp1=cp; }
    else { cp1=NULL; }
  }
  if(cp1) *cp1='\0';
  Notes.Set(buff);
  return this;
}

void *Method::read(iwstream &is, unsigned long version)
{
  method_table.read(is,version);
  Notes.read(is);
  for(int n=0; n<NUMDETECTORS; n++) {
    det[n].read(is, version);
  }
  return this;
}

void Method::write(owstream &os)
{
  method_table.write(os);
  Notes.write(os);
  for(int n=0; n<NUMDETECTORS; n++) {
    det[n].write(os);
  }
}


