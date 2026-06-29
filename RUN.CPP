#pragma hdrstop

#ifndef  _WCPPLIB_WSTREAM_H
#include "wcpplib\wstream.h"
#endif

#ifndef  _RUN_H
#include "run.h"
#endif


#ifndef __TIME_H
#include <time.h>
#endif

#ifndef __STRING_H
#include <string.h>
#endif


#include "debug.h"


RunRawData::RunRawData()
 : pRawData16(0), pRawData32(0)
 , RawDataSize(0), nRawDataFormat(0)
{
  for(int n=0; n<NUMDETECTORS; n++) {
    gain[n]=-1;
  }
}



void CalResults::Clear()
{
  memset(cal_results,0,sizeof(cal_results));
}

Run::Run()
 : RawDataOffset(-1), pCalResults(0), CalResultsOffset(-1),
  type_of_run(0)

 , bDelete(0)
 , bSkipTWA(0) // added to exclude run from TWA calculations. P.P. 08/27/05. Pasted here 10/01/05
 , pointnumber(0)
// , quick_scan_run(1) // added to include/exclude runs from TWA. P.P. 01/05/11
 , quick_scan_run(0) // changed to zero. P.P. 12/28/12
// , zero_gas_run(0) // added. P.P. 03/18/11   rem temp P.P. 06/08/11
{
  time(&start_time);
//  autoscale[0]=1;
//  autoscale[1]=1;
}

Run::~Run()
{
//  TRACE1("~Run=%p",this);
  FreeCalResults();
  FreeRawData();
  #ifdef FILTER
  FreeFiltData();
  #endif//FILTER
  //TRACE1("end of ~Run=%p",this);
}

#ifdef __TURBOC__
#define USE_GLOBALALLOC() 1
#else
#define USE_GLOBALALLOC() 0
#endif

#if defined USE_GLOBALALLOC
#include <windows.h>
#endif

void Run::AllocCalResults()
{
  ASSERT(0==pCalResults);
  #if USE_GLOBALALLOC()
  pCalResults=(CalResults *)GlobalLock(GlobalAlloc(GMEM_MOVEABLE,sizeof(CalResults)));
  #else
  pCalResults=new CalResults;
  #endif
//  TRACE1("alloc pCalResults=%p",pCalResults);
}

void Run::FreeCalResults()
{
//  TRACE1("free pCalResults=%p",pCalResults);
  #if USE_GLOBALALLOC()
  if(pCalResults) GlobalFree(((HGLOBAL)LOWORD(GlobalHandle(SELECTOROF(pCalResults)))));
  #else
  delete pCalResults;
  #endif
  pCalResults=0;
}


void Run::AllocRawData16(long _RawDataSize)
{
  ASSERT(0==runrawdata.pRawData16);
  ASSERT(0==runrawdata.pRawData32);

  long q=_RawDataSize%16;
  long p=_RawDataSize/16;
  _RawDataSize=(p+(q?1:0))*(16+1); // alloc 1 uint16 for detector flags for next 16 data uint16


  MaxRawDataSize=_RawDataSize;
  runrawdata.RawDataSize=0;
  #if USE_GLOBALALLOC()
  runrawdata.pRawData16 = (uint16 huge *)GlobalLock(GlobalAlloc(GHND/*GMEM_MOVEABLE*/,MaxRawDataSize*sizeof(uint16)));
  #else
  runrawdata.pRawData16 = new uint16 huge [MaxRawDataSize];
  #endif
  TRACE3("alloc pRawData=%p,%ld*%d",runrawdata.pRawData16,MaxRawDataSize,sizeof(uint16));
}
void Run::AllocRawData32(long _RawDataSize)
{
  ASSERT(0==runrawdata.pRawData16);
  ASSERT(0==runrawdata.pRawData32);

  long q=_RawDataSize%32;
  long p=_RawDataSize/32;
  _RawDataSize=(p+(q?1:0))*(32+1); // alloc 1 uint32 for detector flags for next 32 data uint32


  MaxRawDataSize=_RawDataSize;
  runrawdata.RawDataSize=0;
  #if USE_GLOBALALLOC()
  runrawdata.pRawData32 = (int32 huge *)GlobalLock(GlobalAlloc(GHND/*GMEM_MOVEABLE*/,MaxRawDataSize*sizeof(uint32)));
  #else
  runrawdata.pRawData32 = new int32 huge [MaxRawDataSize];
  #endif
  TRACE3("alloc pRawData=%p,%ld*%d",runrawdata.pRawData32,MaxRawDataSize,sizeof(uint16));
}

void Run::FreeRawData()
{
//  TRACE1("free pRawData=%p",pRawData);
  #if USE_GLOBALALLOC()
  if(runrawdata.pRawData16) GlobalFree(((HGLOBAL)LOWORD(GlobalHandle(SELECTOROF(runrawdata.pRawData16)))));
  if(runrawdata.pRawData32) GlobalFree(((HGLOBAL)LOWORD(GlobalHandle(SELECTOROF(runrawdata.pRawData32)))));
  #else
  delete [] runrawdata.pRawData16;
  delete [] runrawdata.pRawData32;
  #endif
  runrawdata.pRawData16=0;
  runrawdata.pRawData32=0;
}
#ifdef FILTER
void Run::FreeFiltData()
{
  for(int n=0; n<NUMDETECTORS; n++) {
    delete [] filtData[n].p;  filtData[n].p=0;
  }
}
#endif//FILTER
#ifndef  _UTL_H
#include "utl.h"
#endif
#ifndef  _DATASTRC_H
#include "datastrc.h"
#endif

#ifndef FIELDOFFSET
#define FIELDOFFSET(type, field)    ((int)(&((type near*)1)->field)-1)
#endif

void *Run::readOld(iwstream &is)
{
  _run r;
  CalResultsOffset=is.tell()+FIELDOFFSET(_run,cal_results[0][0]);
  is>>r;
  runrawdata.nRawDataFormat=0;
  for(int n=0; n<NUMDETECTORS; n++) {
    runrawdata.gain[n]=-1;
  }
  runrawdata.RawDataSize=(r.bytes_used-sizeof(_run))/sizeof(uint16);
  r.start_date[8]=0;
  r.start_time[8]=0;
  start_time=ConvertTime( r.start_date, r.start_time );
  r.comment[39]=0; comment=r.comment;
  ///pCalResults=new CalResults;
  ///memcpy(pCalResults->cal_results,r.cal_results,sizeof(pCalResults->cal_results));
  ///see below!! CalResultsOffset=is.tell();
  is.seek(sizeof(r.cal_results),wstream::cur);
  //memcpy(autoscale,r.autoscale,sizeof(autoscale));
  type_of_run=r.type_of_run;
  RawDataOffset=is.tell();
  return this;
}
void *Run::read(iwstream &is, unsigned long version)
{
  is>>RawDataOffset;
  is>>runrawdata.RawDataSize;
  is>>start_time;
  comment.read(is);
  ///pCalResults = new CalResults;
  ///is.read(pCalResults->cal_results,sizeof(pCalResults->cal_results));
  if(version<6) {
    CalResultsOffset=is.tell();
    is.seek(sizeof(pCalResults->cal_results),wstream::cur);
  }
  else {
    is>>CalResultsOffset;
  }
  if(version<7) {
    runrawdata.nRawDataFormat=0;
    for(int n=0; n<NUMDETECTORS; n++) {
      runrawdata.gain[n]=-1;
    }
  }
  else {
    is>>runrawdata.nRawDataFormat;
    if(version>7) {
      is.read(runrawdata.gain,sizeof(runrawdata.gain));
    }
  }
  if(version<3) {
    uint16 autoscale[2/*NUMDETECTORS*/];
    is.read(autoscale,sizeof(autoscale));
  }
  is>>type_of_run;

  if(version>=10) {
    is>>pointnumber;
  }
  return this;
}
#include <assert.h>  // DEBUG
void Run::write(owstream &os)
{
  os<<RawDataOffset;
  os<<runrawdata.RawDataSize;
  os<<start_time;
  comment.write(os);

  ASSERT(CalResultsOffset!=-1);
  os<<CalResultsOffset; // version 6

  os<<runrawdata.nRawDataFormat; // version 7
  os.write(runrawdata.gain,sizeof(runrawdata.gain)); // version 8
  /*
  CalResultsOffset=os.tell();
  os.write(pCalResults->cal_results,sizeof(pCalResults->cal_results));
  */
//  os.write(autoscale,sizeof(autoscale));
  os<<type_of_run;

  os<<pointnumber;
}






//extern long pow10_array[3];
extern long pow10_array[4]; // returned to this, no 10000 range anymore. P.P. 11/02/12
//extern long pow10_array[5]; // range 10000. P.P. 05/13/11
RawDataIterator::RawDataIterator(const RunRawData &_runrawdata, int num)
 : pRawData16Cur(_runrawdata.pRawData16)
 , pRawData32Cur(_runrawdata.pRawData32)
 , runrawdata(_runrawdata)
 , l(0), num_mask(num*(_runrawdata.nRawDataFormat==2?0x80000000UL:0x8000))
 , gain(_runrawdata.gain[num])
{
  GetNext();
}
void RawDataIterator::GetNext()
{
  beg:
  if(l>=runrawdata.RawDataSize) {
    return; // may be some error here
  }

  if(runrawdata.nRawDataFormat==2) {
    int k=(int)(l%33);
    if(k==0) {
      detectors=*pRawData32Cur;
      pRawData32Cur++;
      l++;
      goto beg;
    }
    k--;

    uint32 raw=*pRawData32Cur;
    pRawData32Cur++;
    l++;

    uint32 m=detectors<<k; // bit in detector flags word
    if((m&0x80000000UL) == num_mask) { // detector OK?
      value=raw;
      return;
    }
    else goto beg;
  }
  if(runrawdata.nRawDataFormat==1) {
    int k=(int)(l%17);
    if(k==0) {
      detectors=*pRawData16Cur;
      pRawData16Cur++;
      l++;
      goto beg;
    }
    k--;

    uint16 raw=*pRawData16Cur;
    pRawData16Cur++;
    l++;

    uint16 m=detectors<<k; // bit in detector flags word
    if((m&0x8000U) == num_mask) { // detector OK?
      value=raw;
      return;
    }
    else goto beg;
  }

  // old format
  uint16 raw=*pRawData16Cur;
  pRawData16Cur++;
  l++;
  if((raw&0x8000)==num_mask) {
    int32 cur_autoscale;
    if(gain==-1) {
      gain = (raw & 0x7000) >> 12;
      cur_autoscale = 1;
    }
    else {
      int power = ((raw & 0x7000) >> 12) - gain;     // old, 12 bit detector? This should not be used, check! P.P. 06/30/17
//      if(power<0 || power>2) cur_autoscale=0;
      if(power<0 || power>3) cur_autoscale=0;   // P.P. 12/11/09. New range 1000.  restored 06/30/17
//      if(power<0 || power>4) cur_autoscale=0;   // P.P. 05/13/11. New range 10000.
      else cur_autoscale = pow10_array[power];
    }
    value=int32(raw & 0x0fff) * cur_autoscale;    // old, 12 bit detector? This should not be used, check! P.P. 06/30/17
  }
  else goto beg;
}

#ifdef FILTER
FiltDataIterator::FiltDataIterator(FiltData *_pFiltData, int num)
 : pFiltData(_pFiltData+num)
, l(0)
{ p=pFiltData->p; }
void FiltDataIterator::GetNext()
{
  p++; l++;
}
#endif//FILTER


