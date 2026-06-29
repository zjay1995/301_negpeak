#ifndef  _RUN_H
#define  _RUN_H

#ifndef  _WCPPLIB_WSTREAM_H
#include "wcpplib\wstream.h"
#endif

#ifndef  _WCPPLIB_STR_H
#include "wcpplib\str.h"
#endif

#ifndef  _SHORTS_H
#include "shorts.h"
#endif
#ifndef  _CONSTS_H
#include "consts.h"
#endif


#ifndef  _TIME_T
#define  _TIME_T
typedef long time_t;
#endif

#ifdef FILTER
class FiltData {
 public:
  int gain;
  int32 huge *p;
  long size;
  FiltData() : gain(-1), p(0), size(0) {}
  ~FiltData() { delete [] p; }
};
#endif//FILTER

class CalResults {
 public:
  CalResults() { Clear(); }
  void Clear();

//  long cal_results[NUMDETECTORS][COMPONENT_ROWS][STAND_NUM];  // indexed by detector, component, and standard.
  double cal_results[NUMDETECTORS][COMPONENT_ROWS][STAND_NUM];  // P.P. 11/18/17 to store area as double
};

class RunRawData {
 public:
  uint8 nRawDataFormat;    // 0 - old data format. 1 - 16 bit data with 16bit flags
  uint16 huge *pRawData16; // raw data if loaded
  int32 huge *pRawData32;
  long RawDataSize;     // current data size
  int16 gain[NUMDETECTORS];  // RawData format 1: starting gain

  RunRawData();
};

class Run {
 public:
  time_t   start_time;
  Str      comment;
//   uint16   autoscale[NUMDETECTORS];   // autoscale values for the run
  int16    type_of_run;               /* 2 -> Job, 1 -> Cal */

  uint16   pointnumber; // see points flag in method_table

  long CalResultsOffset; // ofset in file
  CalResults *pCalResults; // cal_results if loaded - use Run::GetCalResults(Run *) to access it
  long RawDataOffset;   // ofset in file
  long MaxRawDataSize;     // allocated data size
  RunRawData runrawdata;
  #ifdef FILTER
  FiltData filtData[NUMDETECTORS]; // filtered data if exist
  void FreeFiltData();
  #endif//FILTER
  void AllocRawData16(long _RawDataSize);
  void AllocRawData32(long _RawDataSize);
  void FreeRawData();
  void AllocCalResults();
  void FreeCalResults();

  Run();
  ~Run();

  void *readOld(iwstream &is); // Peak Work 4.0 for DOS
  void *read(iwstream &is, unsigned long version);
  void write(owstream &os);

  int bDelete; // Do not save! Delete it from RunList! Do not save in JOB!
  int bSkipTWA; // added to exclude run from TWA calculations. P.P. 08/27/05
  int quick_scan_run;  // added. P.P. 01/05/11
};




class DataIterator {
 public:
  DataIterator() {}
  virtual ~DataIterator() {}
  virtual int32 Get() { return 0; }
  virtual void GetNext() {}
  virtual int Ok() { return 0; }
  virtual int Gain() { return 1; }
};

class RawDataIterator : public DataIterator {
 public:
  int gain;
  uint32 num_mask;
  uint16 huge *pRawData16Cur;
  int32 huge *pRawData32Cur;
  uint32 detectors;
  long l;
  const RunRawData &runrawdata;
  int32 value;
  RawDataIterator(const RunRawData &_runrawdata, int num);
  virtual void GetNext();
  virtual int32 Get() { return value; }
  virtual int Ok() { return l<runrawdata.RawDataSize; }
  virtual int Gain() { return gain; }
};

#ifdef FILTER
class FiltDataIterator: public DataIterator {
 public:
  FiltData *pFiltData;
  int32 huge *p;
  long l;
  FiltDataIterator(FiltData *_pFiltData, int num);
  virtual void GetNext();
  virtual int32 Get() { return *p; }
  virtual int Ok() { return l<pFiltData->size; }
  virtual int Gain() { return pFiltData->gain; }
};
#endif//FILTER


#endif
