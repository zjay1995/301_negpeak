#ifndef  _JOB_H
#define  _JOB_H

#ifndef  _WCPPLIB_STR_H
#include "wcpplib\str.h"
#endif
class WCPPLIB_CLASS iwstream;
class WCPPLIB_CLASS owstream;

#ifndef  _DATASTRC_H
#include "datastrc.h"
#endif

#ifndef  _METHOD_H
#include "method.h"
#endif
#ifndef  _RUNLIST_H
#include "runlist.h"
#endif

class PeakList;

class Job: public Method, public RunList {
  typedef Method Inherited;
  typedef RunList Inherited2;
 public:
  time_t   start_time,end_time;
  Str comment;
////  long     last_run_pos;              // offset of the last run
  Str FileName;
  unsigned long FileVersion;
  long MethodOffset;
  long RunListOffset;

  Job();
  virtual ~Job();

  friend void destroy(Job *p) { destroy((Inherited *)p); }
  virtual void _cdecl shutDown(void);

  virtual void *readOld(iwstream &is); // Peak Work 4.0 for DOS
  virtual void *read(iwstream &is, unsigned long version);
  virtual void write(owstream &os);

  void writeCalResults(owstream &os, Run *pRun);
  void writeRawData(owstream &os, Run *pRun);
  void writeRunList(owstream &os);
  void writeMethod(owstream &os, Method *pMethod);
  void writeOffsets(owstream &os);

  int readMethod(iwstream &is, unsigned long version, Method *pMethod);

  CalResults *GetCalResults(Run *pRun);
  RunRawData *GetRawData(Run *pRun);
  #ifdef FILTER
  FiltData *GetFiltData(Run *pRun, int num);
  #endif//FILTER
  void SaveRunFile(Run *pRun, PeakList **pPeakList);
  void SaveCalResults(Run *pRun);
  void SaveRawData(Run *pRun);
  void SaveRunList();
  void SaveMethod();
//  void SaveRawDataToFile(Run *pRun, long run_time, long y_point);  // added. P.P. 02/16/12
//  void SaveRawDataToFile(Run *pRun, char det_label, long run_time, long y_point);  // added. P.P. 02/16/12
//  void SaveRawDataToFile(Run *pRun, char det_label, long time_min, long time_s, long time_ms, long y_point);  // added. P.P. 02/16/12
  void SaveRawDataToFile(Run *pRun, char det_label, long time_s, long time_ms, long y_point);  // added. P.P. 02/16/12


  const char *Name() const;
  void MakeName(char far *str, const char far *_ext) const;
  int IsDefault() const;
};

struct RunMode {
  int Acquire;	 		// CGONmode1
  // = 0 Manual mode
  // = 1 Continuous mode
  // = 2 Repeat mode 		// use method_table.repeat_cycle
  // = 3 Remote mode       // comments added. P.P. 06/14/03      
  int SaveRun;	 		// CGONmode2
  // = 0 for manual save
  // = 1 for autosave
  // = 2 for save on alarm
  // for Continuous mode??
  int bTWA;	    		// CGONmode3
  int bSTEL;	    		// CGONmode4

  RunMode() : Acquire(0), SaveRun(1), bTWA(0), bSTEL(0) {}
};

extern RunMode runMode;



struct ShiftMode {
  int Shift;
  // in hours
  int SaveReport;
  // = 0 for manual save (no auto save)
  // = 1 for autosave
  // = 2 for save on alarm
  int PrintReport;
  // = 0 for manual print (no auto print)
  // = 1 for autoprint
  // = 2 for print on alarm
  int bPrintAlarm;
  // = 0
  // = 1 for Print on alarm immideatly

  ShiftMode() : Shift(8), SaveReport(1), PrintReport(0), bPrintAlarm(0) {}
};

extern ShiftMode shiftMode;


const unsigned long job_file_label=0x6A554E48L;
const unsigned long job_file_version_oldest=2;
//const unsigned long job_file_version=14; // 5 - wrong version.
//const unsigned long job_file_version=18; // P.P. 10/11/02
//const unsigned long job_file_version=20; // P.P. 12/19/02 - for 501.
//const unsigned long job_file_version=21; // P.P. 04/06/03 - 501 with check_alarm.
//const unsigned long job_file_version=22; // P.P. 07/29/04 - 501 with x1_time.
//const unsigned long job_file_version=23; // W.K. 09/17/04 - alarms[POINTS()]
//const unsigned long job_file_version=24; // P.P. 09/17/05 - known/all peaks.
//const unsigned long job_file_version=25; // P.P. 02/11/06 - desorb time and temperature.  Not used and removed 02/20/06
//const unsigned long job_file_version=25; // P.P. 02/11/06 - X1-OFF time added. P.P. 06/18/06
//const unsigned long job_file_version=26; // P.P. 07/20/06 - Temp slope and intersect added. P.P. 07/20/06
//const unsigned long job_file_version=28; // P.P. 04/15/07 - multiple analog ranges
//const unsigned long job_file_version=29; // P.P. 11/14/08 - external detector
//const unsigned long job_file_version=30; // P.P. 03/29/09 - invert signal. Here 11/06/09
//const unsigned long job_file_version=31; // P.P. 03/07/10 - desorber preheat time.
//const unsigned long job_file_version=32; // P.P. 06/20/10 - X2-X4 times added. P.P. 06/20/10
//const unsigned long job_file_version=33; // P.P. 12/14/10 - quick_scan time. 32 left for X_2_5 times
//const unsigned long job_file_version=34; // P.P. 02/24/12 - data_to_excel. P.P. 02/24/12
//const unsigned long job_file_version=35; // P.P. 09/07/12 - oven door manual/auto.
//const unsigned long job_file_version=36; // P.P. 11/09/12 - Range_1.
//const unsigned long job_file_version=37; // P.P. 01/24/13 - scaled peaks.
//const unsigned long job_file_version=38; // P.P. 03/12/17 - desorber time added as separate from equil time.
//const unsigned long job_file_version=39; // P.P. 03/29/17 - line heater temperature added.
//const unsigned long job_file_version=40; // P.P. 06/05/17 - next point time added.
//const unsigned long job_file_version=41; // P.P. 12/01/17 - cal_results[] values are double now.
//const unsigned long job_file_version=42; // P.P. 03/26/18 - desorber wait time added.
const unsigned long job_file_version=43; // P.P. 07/16/18 - desorber and fid temperatures added as separate from det_inj temp.
#endif



