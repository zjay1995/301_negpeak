#ifndef  _DATASTRC_H
#define  _DATASTRC_H

#ifndef  _SHORTS_H
#include "shorts.h"
#endif

#ifndef  _CONSTS_H
#include "consts.h"
#endif


// File structure typedefs
// old DOS version

struct _meth_tbl {
   uint     inj_det_temp;
   uint     oven_temp;
   char     analysis_time[6];
   char     sample_time[5];
   char     inject_time[5];
   char     syringe_on_off;
   char     TWA_on_off;
   int      TWA_cycle;
   int      cal_cycle;
   int      repeat_cycle;
   char     alarms_on_off;
   char     STEL_on_off;
   char     paper_save_on_off;
   char     cont_pump_on_off;
   char     plotter_on_off;

   char     spare1[100];

   char     NandBtime[NUMDETECTORS][6];           /* Detector specific parameters */
   char     NandBlen[NUMDETECTORS][6];
   char     peak_alg[NUMDETECTORS];
   char     detect_meth[NUMDETECTORS];
   char     gain[NUMDETECTORS];
   char     plot_atten[NUMDETECTORS];
   char     detect_stat[NUMDETECTORS];
   long     threshold[NUMDETECTORS];
   int      segment_width[NUMDETECTORS];
   char     units[NUMDETECTORS][11];

   char     spare2[78];
};

struct _cpnts {
   char     name[24];
   char     active_yn;
   char     peak_rt[6];
   char     window[5];
   float    resp_fact;
   float    low_alarm;
   float    high_alarm;

   char     spare1[50];
};

struct _cal_srnd {
   char     ref_peak;
   float    stand[4];
};

struct _raw_struct {
   int      time;
   uint16   raw_units;
   long     atten_sample;
};


struct _method {
   uint8     file_type;                 // 3 for method file
   _meth_tbl method_table;              // method form data
   _cpnts    components[NUMDETECTORS][COMPONENT_ROWS];  // components form (detectors A&B)
   char      spare1[720];               // was calibration results
   _cal_srnd standards[NUMDETECTORS][COMPONENT_ROWS];  // calibration standards (detectors A&B)
   char      notes[10][40];
};


struct _job_header {
   uint8    file_type;                 // 1 for Cal., 2 for Job
   char     start_date[9];
   char     start_time[9];
   char     end_date[9];
   char     end_time[9];
   char     comment[40];
   uint     num_recs;
   long     meth_offset;               // offset of the method table
   long     first_offset;              // offset of the first data record
   long     last_run_pos;              // offset of the last run
   char     spares1[48];               // some spare bytes
};

struct _run {
   ulong    bytes_used;                // # bytes actually used for this run.
   char     start_date[9];
   char     start_time[9];
   char     comment[40];
   long     cal_results[NUMDETECTORS][COMPONENT_ROWS][STAND_NUM];  // indexed by detector, component, and standard.
   uint     autoscale[NUMDETECTORS];   // autoscale values for the run
   char     type_of_run;               /* 2 -> Job, 1 -> Cal */
   char     spares1[13];               // some spare bytes
   // raw data is tacked on here repeatedly.
};


#endif
