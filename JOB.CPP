#pragma hdrstop

#ifndef  _WCPPLIB_WSTREAM_H
#include "wcpplib\wstream.h"
#endif

#ifndef  _JOB_H
#include "job.h"
#endif

#ifndef __TIME_H
#include <time.h>
#endif

#include "debug.h"

Job::Job()
 : FileVersion(0), RunListOffset(-1), MethodOffset(-1)
{
  time(&start_time); time(&end_time);
}
Job::~Job()
{
  //TRACE1("~Job=%p",this);
}

void cdecl Job::shutDown(void)
{
  Inherited2::shutDown();
  Inherited::shutDown();
}


#ifndef  _UTL_H
#include "utl.h"
#endif

#ifndef  _WCPPLIB_MESSAGE_H
#include "wcpplib\message.h"
#endif

void *Job::readOld(iwstream &is)
{
  _job_header h;
  is>>h;

  start_time=ConvertTime(h.start_date,h.start_time);
    end_time=ConvertTime(h.  end_date,h.  end_time);

  comment.Set(h.comment);
//  last_run_pos=h.last_run_pos;

  is.seek(h.meth_offset,wstream::beg);
  Inherited::readOld(is);

  for(int i=0; i<h.num_recs; i++) {
    RunList::Node *prln=new RunList::Node;
    if(!prln) return 0;
    if(!is.good()) return 0;
    prln->readOld(is);
    if(!is.good()) {
      message("Error reading Run #%d.\nFile truncated.",i+1);
      destroy(prln);
      is.clear();
      return this;
    }
    append(prln);
    is.seek(prln->runrawdata.RawDataSize*sizeof(uint16),wstream::cur); // skip data
  }
  return this;
}

const long job_start_offset=8;

const unsigned long run_label=0x72554E48L;
const unsigned long stop_label=0x73554E48L;
const unsigned long method_label=0x6d554E48L;

void *Job::read(iwstream &is, unsigned long version)
{
  is>>MethodOffset>>RunListOffset;
  is.seek(RunListOffset);
  for(;;) {
    ulong label;
    is>>label;
    if(label==stop_label) break;
    if(label!=run_label) return NULL; // error
    RunList::Node *prln=new RunList::Node;
    if(!prln) return 0;
    if(!is.good()) return 0;
    prln->read(is,version);
    append(prln);
  }
  int ret=readMethod(is, version, this);
  if(ret) {
    return NULL;
  }
  //is.seek(MethodOffset);
  //Inherited::read(is, version);
  //if(!is.good()) return NULL;
  comment.read(is);
  is>>start_time>>end_time;
//  is>>last_run_pos;
  return this;
}
static const unsigned long CalResults_label=0x63554E48L;
static const unsigned long RawData_label=0x64554E48L;

void Job::writeCalResults(owstream &os, Run *pRun)
{
  CalResults *pCalResults=GetCalResults(pRun);
  if(pCalResults) {
    pRun->CalResultsOffset=os.tell();
    os.write(&CalResults_label,sizeof(CalResults_label));
    os.hwrite(pCalResults,sizeof(pCalResults->cal_results));
  }
}

void Job::writeRawData(owstream &os, Run *pRun)
{
  RunRawData *runrawdata=GetRawData(pRun);
  if(runrawdata && runrawdata->pRawData32) {
    pRun->RawDataOffset=os.tell();
    os.write(&RawData_label,sizeof(RawData_label));
    os.hwrite(runrawdata->pRawData32,pRun->runrawdata.RawDataSize*sizeof(uint32));
  }
}

void Job::writeRunList(owstream &os)
{
  //TRACE("Job::writeRunList");
  RunListOffset=os.tell();
  for(RunList::Node *prln=first(); prln; prln=prln->nextNode()) {
    os<<run_label;
    prln->write(os);
  }
  os<<stop_label;
  //TRACE("end of Job::writeRunList");
}
void Job::writeMethod(owstream &os, Method *pMethod)
{
  MethodOffset=os.tell();
  os.write(&method_label,sizeof(method_label));
  pMethod->Method::write(os);
  comment.write(os);
  os<<start_time<<end_time;
}
void Job::writeOffsets(owstream &os)
{
  wstreampos offset=os.tell();
  os.seek(job_start_offset);
  os<<MethodOffset<<RunListOffset;
  os.seek(offset);
}

/*
  HNUj
  version

  MethodOffset
  RunListOffset
  ...RawData...
  ...CalResults...
  ...RawData...
  ...CalResults..
  ...RawData...
  ...CalResults..
  RunList
  Method
*/


void Job::write(owstream &os) // full
{
  long l=os.tell();
  l+=32;
  os<<MethodOffset<<RunListOffset;
  for(RunList::Node *prln=first(); prln; prln=prln->nextNode()) {
    writeRawData(os, prln);
    writeCalResults(os, prln);
    prln->FreeCalResults(); // unload from memory
    prln->FreeRawData();    // unload from memory
  }
  writeRunList(os);
  writeMethod(os,this);
  writeOffsets(os);
}
#ifndef  _WCPPLIB_FWSTREAM_H
#include "wcpplib\fwstream.h"
#endif
#ifdef FILTER
FiltData *Job::GetFiltData(Run *pRun, int num)
{
  FiltData *p=&pRun->filtData[num];
  if(p->p) return p;
  return 0;
}
#endif//FILTER


CalResults *Job::GetCalResults(Run *pRun)
{
  if(!pRun->pCalResults && pRun->CalResultsOffset>=0) {
    ifwstream is(FileName);
    is.seek(pRun->CalResultsOffset);
    if(is.good()) {
      ASSERT(pRun->pCalResults==0);
      if(FileVersion>=6) {
	unsigned long _CalResults_label;
	is.read(&_CalResults_label,sizeof(_CalResults_label));
	ASSERT(CalResults_label==_CalResults_label);
	if(CalResults_label!=_CalResults_label)
	  return 0;
      }
      pRun->AllocCalResults();
      if(pRun->pCalResults)
	is.hread(pRun->pCalResults->cal_results,sizeof(pRun->pCalResults->cal_results));
    }
  }
  return pRun->pCalResults;
}

RunRawData *Job::GetRawData(Run *pRun)
{
  if(!(pRun->runrawdata.pRawData16 || pRun->runrawdata.pRawData32) && pRun->RawDataOffset>=0) {
    ifwstream is(FileName);
    is.seek(pRun->RawDataOffset);
    if(is.good()) {
      if(FileVersion>=6) {
	unsigned long _RawData_label;
	is.read(&_RawData_label,sizeof(_RawData_label));
	ASSERT(RawData_label==_RawData_label);
	if(RawData_label!=_RawData_label)
	  return 0;
      }
      long RawDataSize=pRun->runrawdata.RawDataSize; // pRun->RawDataSize will be zeroed at alloc

      if(FileVersion>=12) {
	pRun->AllocRawData32(RawDataSize);
	if(pRun->runrawdata.pRawData32) {
	  is.hread(pRun->runrawdata.pRawData32,RawDataSize*sizeof(uint32));
	  pRun->runrawdata.RawDataSize=RawDataSize;
	}
      }
      else {
	pRun->AllocRawData16(RawDataSize);
	if(pRun->runrawdata.pRawData16) {
	  is.hread(pRun->runrawdata.pRawData16,RawDataSize*sizeof(uint16));
	  pRun->runrawdata.RawDataSize=RawDataSize;
	}
      }
    }
  }
  ASSERT(pRun->runrawdata.pRawData16 || pRun->runrawdata.pRawData32);
  return &pRun->runrawdata;
}

void Job::SaveCalResults(Run *pRun)
{
  if(pRun->pCalResults) {
    if(FileName.IsEmpty()) {
      message("Use Save file As to create file");
      return;
    }
    if(FileVersion!=job_file_version) {
      message("Cannot save in old format file:\n%s", FileName.Get());
      return;
    }
    ofwstream os(FileName,FALSE); // open existing
    os.seek(pRun->CalResultsOffset);
    writeCalResults(os, pRun);
    if(!os.good()) message("Error saving Cal Results to file:\n%s", FileName.Get());
  }
}

int Job::readMethod(iwstream &is, unsigned long version, Method *pMethod)
{
  is.seek(MethodOffset);
  if(FileVersion>=6) {
    unsigned long _method_label;
    is.read(&_method_label,sizeof(_method_label));
    if(method_label!=_method_label) return -1;
  }
  pMethod->Method::read(is,version);
  if(!is.good()) {
    return -1;
  }
  return 0;
}

void Job::SaveRawData(Run *pRun) // & CalResults & RunList & Method
{
  TRACE("Job::SaveRawData");
  if(pRun->runrawdata.pRawData32) {
    if(FileName.IsEmpty()) {
      message("Use Save file As to create file");
      return;
    }
    if(FileVersion!=job_file_version) {
      message("Cannot save in old format file:\n%s", FileName.Get());
      return;
    }
    Method *pMethodOld=new Method;
    {
      ifwstream is(FileName);
      int ret=readMethod(is, FileVersion, pMethodOld);
      if(ret) {
	message("Error reading file:\n%s", FileName.Get());
	return;
      }
    }
    ofwstream os(FileName,FALSE); // open existing
    os.seek(/*MethodOffset*/RunListOffset);

    writeRawData(os, pRun);
    writeCalResults(os, pRun);

    writeRunList(os);
    writeMethod(os,pMethodOld);
    writeOffsets(os);
    destroy(pMethodOld);
    if(!os.good()) message("Error saving Raw Data to file:\n%s", FileName.Get());
  }
  TRACE("end of Job::SaveRawData");
}

void Job::SaveRunList()
{
  TRACE("Job::SaveRunList");
  if(1) {
    if(FileName.IsEmpty()) {
      message("Use Save file As to create file");
      return;
    }
    if(FileVersion!=job_file_version) {
      message("Cannot save Run List in old format file:\n%s", FileName.Get());
      return;
    }
    Method *pMethodOld=new Method;
    {
      ifwstream is(FileName);
      int ret=readMethod(is, FileVersion, pMethodOld);
      if(ret) {
	message("Error reading file:\n%s", FileName.Get());
	return;
      }
    }
    ofwstream os(FileName,FALSE); // open existing
    os.seek(/*MethodOffset*/RunListOffset);
    writeRunList(os);
    writeMethod(os,pMethodOld);
    writeOffsets(os);
    destroy(pMethodOld);
    if(!os.good()) message("Error saving Run List to file:\n%s", FileName.Get());
  }
  TRACE("end of Job::SaveRunList");
}

void Job::SaveMethod()
{
  if(1) {
    if(FileName.IsEmpty()) {
      message("Use Save file As to create file");
      return;
    }
    if(FileVersion!=job_file_version) {
      message("Cannot save Method in old format file:\n%s", FileName.Get());
      return;
    }
    ofwstream os(FileName,FALSE); // open existing
//    os.seek(/*Meth}odOffset*/RunListOffset);
//    writeRunList(os);
    os.seek(MethodOffset);
    writeMethod(os,this);
    writeOffsets(os);
    if(!os.good()) message("Error saving Method to file:\n%s", FileName.Get());
  }
}


#ifndef __DIR_H
#include <dir.h>
#endif

#ifdef _Windows
  #ifndef __WINDOWS_H
  #include <windows.h>	// 	AnsiLowerBuff
  #endif
#endif

const char *Job::Name() const
{
  char drive[MAXDRIVE];
  char dir[MAXDIR];
  static char name[MAXFILE];
  char ext[MAXEXT];
  fnsplit(FileName.Get(), drive, dir, name, ext);
  #ifdef _Windows
    AnsiLowerBuff(name,lstrlen(name));
  #else
    strlwr(name);
  #endif
  return name;
}

void Job::MakeName(char far *str, const char far *_ext) const
{
  char drive[MAXDRIVE];
  char dir[MAXDIR];
  char name[MAXFILE];
  char ext[MAXEXT];
  fnsplit(FileName.Get(), drive, dir, name, ext);
  fnmerge(str, drive, dir, name, _ext);
}

int Job::IsDefault() const
{
  return FileName.IsEmpty();
}

#include <stdio.h>
#include "country.h"
#include "peaklist.h"
extern char data_rate;           // rate data received in pts/sec.


static void PrintRunTimeAndComment(owstream &os, Run *pRun, const char *file)
{
  Str str(100);
  wsprintf(str.GetPtr(),"\"%s\", \"%s\", \"%s\", \"",PrintDate(pRun->start_time), PrintTime(pRun->start_time), file);
  os.write(str,str.Length());
  os.write(pRun->comment.Get(),pRun->comment.Length());
  wsprintf(str.GetPtr(),"\"");
  os.write(str,str.Length());
}

#include "calc.h" // TWAxxx
//static void PrintTWAResults(owstream &os, int n, Detector *pDetector, Peak *pPeak, int pointnumber)
static void PrintTWAResults(owstream &os, int n, Detector *pDetector, Peak *pPeak, int pointnumber, long t) // t added to include TWA_total. P.P. 12/31/04
{
  if(t>0) { // not a total or no peaks - write for each component. P.P. 01/02/05
    for(int i=0; i<COMPONENT_ROWS; i++) {
      Component *pComponent=&pDetector->components[i];
  // P.P. 12/18/01. To print TWA anyway, if() condition was removed.
      if(pPeak->pComponent==pComponent) { // i component found  P.P. removed condition- restored 01/11/03
//    if(pComponent) { // if component found  P.P. 
        Str str(100);
//      sprintf(str.GetPtr(),", %.3f, %.3f", TWA_results[pointnumber][n][i], max_TWA_conc[pointnumber][n][i]);
// code changed to include TWA_total. P.P. 12/31/04
	sprintf(str.GetPtr(),", %.3f, %.3f, %.3f", min_TWA_conc[pointnumber][n][i], TWA_results[pointnumber][n][i], max_TWA_conc[pointnumber][n][i]); // P.P. 12/31/04
        os.write(str,str.Length());
        return;
      }
    }
  }
#if USE_TOTAL() // added. P.P. 01/02/05
//  else if(t<0) { // total. P.P. 01/02/05
  else { // total shown if no peaks. P.P. restored here 01/07/05
    Str str(100);
    sprintf(str.GetPtr(),", %.3f, %.3f, %.3f", min_TWA_total[pointnumber][n], TWA_total[pointnumber][n], max_TWA_total[pointnumber][n]); // P.P. 12/31/04
    os.write(str,str.Length());
  }
#endif
}

#include "hnu.h" //CAL

// P.P. 01/11/03. pPeak added to the parameters to be able to address PrintTWAResults.
//static void WritePeakToRunFile(owstream &os, int num, Detector *pDetector, Run *pRun, Method *pMethod, const char *file, const char *component_name, double concen, int flags, long t, long Height, long Area, long mul)
//static void WritePeakToRunFile(owstream &os, int num, Detector *pDetector, Run *pRun, Method *pMethod, const char *file, const char *component_name, double concen, int flags, long t, long Height, long Area, long mul, Peak *pPeak)
static void WritePeakToRunFile(owstream &os, int num, Detector *pDetector, Run *pRun, Method *pMethod, const char *file, const char *component_name, double concen, int flags, long t, long Height, double Area, long mul, Peak *pPeak) // P.P. 03/23/18
{
  char str[120];
//  Peak *pPeak; // added. P.P. 01/11/03 and removed.
  {
    wsprintf(str,"\"%s\", \"%s\", ",pDetector->Label.Get(),component_name);
    os.write(str,strlen(str));
  }
// time stamp is added here. P.P. 03/23/18
  {
//    wsprintf(str.GetPtr(),"\"%s\", ", PrintTime(pRun->start_time));
    wsprintf(str,"%s, ", PrintTime(pRun->start_time));
//    wsprintf(str,"\"%s\", ", PrintTime(pRun->start_time));
    os.write(str,strlen(str));
  }

  const char *alarm="none";
  {
    if((flags&1)==0) {
#if USE_TOTAL()  // case added. no peaks - concen total = 0. P.P. restored here 01/07/05
      concen=0;
#else
      concen=-1;
#endif
    }
    if(pRun->type_of_run==JOB) {  // cond added. P.P. 07/26/04
      if(flags&2) alarm="High";
      if(flags&4) alarm="Low";
      if(t<=0)    alarm ="";  // added. P.P. 01/10/03
    }
    else { // CAL or CHECKCAL  // added. P.P. 07/26/04
      alarm = "";             //
    }                          //

    {
//      sprintf(str, "%.3f, ", concen);
      sprintf(str, "%.6f, ", concen);  // P.P. 07/30/17
      os.write(str,strlen(str));
    }
  }

  {
//    if(t>0) {  // condition added to remove Height and Area from writing Total. P.P. 01/08/03 
    if(1) {  //  Height and Area from now in Total as well. P.P. 12/20/11
//      wsprintf(str,"%ld, %ld, ",Height*mul,Area*mul);
      sprintf(str,"%ld, %.0f",Height*mul,(float) Area*mul);  //  area is double - show as float, and use sprintf instead of wsprintf. P.P. 03/23/18
    }
    else {
      wsprintf(str,"\"\", \"\", "); // changed. P.P. 01/10/03
    }
    // end of added. P.P. 01/08/03
    os.write(str,strlen(str));
      
  }
  {
    if(t>0) wsprintf(str,"\"%ld:%.2ld\", ", t/60, t%60);
    else wsprintf(str,"\"\", ");
    os.write(str,strlen(str));
  }

  {
    wsprintf(str,"\"%s\", ",alarm);
    os.write(str,strlen(str));
  }
  PrintRunTimeAndComment(os, pRun, file);

//  if(pRun->type_of_run!=CAL) {
  if((pRun->type_of_run!=CAL)&&(pRun->type_of_run!=CHECKCAL)) { // replacement. P.P. 12/31/04
    if(pMethod->method_table.TWA_on_off) {
//	      if(pPeak)   // The condition removed to print TWA anyway. P.P. 12/18/01
//      PrintTWAResults(os, num, pDetector, 0/*pPeak*/, pRun->pointnumber);
      if(t>0) { // condition added to avoid printing TWA for Total and no peaks. P.P. 01/11/03
//	PrintTWAResults(os, num, pDetector, pPeak, pRun->pointnumber); // restored pPeak. P.P. 01/11/03
	PrintTWAResults(os, num, pDetector, pPeak, pRun->pointnumber, t); // P.P. 12/31/04
      }
      else if(t<0) { // case added for total. P.P. 12/31/04. if t=0 - no peaks, do not print
//      else { //  if t=0 - no peaks, but print twa anyway. test later. P.P. 01/02/05
	PrintTWAResults(os, num, pDetector, 0/*pPeak*/, pRun->pointnumber, t); // P.P. 12/31/04
      }
      else { //  if t=0 - no peaks, but print twa anyway. test later. P.P. 01/02/05  3 lines restored here. P.P. 01/07/05
	PrintTWAResults(os, num, pDetector, 0/*pPeak*/, pRun->pointnumber, t); // P.P. 12/31/04
      }
    }
  }
  os.write("\r\n",2);
}

#include <math.h>
static void WritePeaksToRunFile(owstream &os, Run *pRun, Method *pMethod, PeakList **pPeakList, const char *file)
{
  if(pRun->type_of_run!=CAL) {
//    for(int n=0; n<NUMDETECTORS; n++) { // removed - detectors scanned in TWA_calc itself. P.P. 01/14/03
// rem P.P.n 01/18/03      TWA_calc(pMethod, pRun, pPeakList, pRun->pointnumber);
//    }
  }

  // write a record for each component found in the run
  int bEmpty=1;
  for(int num=0; num<NUMDETECTORS; num++) {
    Detector *pDetector=&pMethod->det[num];
    if(pDetector) {
      if(pPeakList[num]) {
//	long mul=pow10( pRun->runrawdata.gain[num] );
	long mul=1; // all was taken care of in calculations already. P.P. 03/23/18
	if(!mul) mul=1; // gain==-1? old file?
	#if USE_TOTAL()
	int total_flags=0;
	double total_concen=0;
	long total_H=0;
	long total_A=0;
	#endif // USE_TOTAL()
	for(PeakList::Node *pNode=pPeakList[num]->first(); pNode; pNode=pNode->nextNode()) {
	  Peak *pPeak=pNode;
	  const char *component_name=pPeak->pComponent? pPeak->pComponent->name.Get(): "unknown";
	  double concen;
	  int flags=pDetector->Concentration(pMethod, pRun, pPeak, num, &concen);

	  #if USE_TOTAL()
	  total_flags|=flags;
#if AREA_ALL() // case added. P.P. 12/21/11
	  if(flags&1) {
	    total_concen+=concen;
          }
	  total_H+=pPeak->Height;
	  total_A+=pPeak->Area;
#else // as it was always
	  if(flags&1) {
	    total_concen+=concen;
	    total_H+=pPeak->Height;
	    total_A+=pPeak->Area;
	  }
#endif // area_all
	  #endif // USE_TOTAL()

	  long t=pPeak->Time/data_rate;
//	  WritePeakToRunFile(os, num, pDetector, pRun, pMethod, file, component_name, concen, flags, t, pPeak->Height, pPeak->Area, mul);
	  WritePeakToRunFile(os, num, pDetector, pRun, pMethod, file, component_name, concen, flags, t, pPeak->Height, pPeak->Area, mul, pPeak); // P.P. 01/11/03

	  bEmpty=0; // Something written

	  if(!pNode) break;
	}
	if(!pPeakList[num]->first()) { // Print "no peaks"
	  long t=0;
//	  WritePeakToRunFile(os, num, pDetector, pRun, pMethod, file, "no peaks", 0, 0, t, 0, 0, mul);
#if QUICK_SCAN() // case added. P.P. 01/13/11
	  WritePeakToRunFile(os, num, pDetector, pRun, pMethod, file, "no peaks", 0, 0, t, 0, 0, mul, 0); // P.P. 01/13/11
#else // as it was
#if USE_TOTAL() // case added: no peaks - total=0,
	  WritePeakToRunFile(os, num, pDetector, pRun, pMethod, file, "Total", 0, 0, t, 0, 0, mul, 0); // P.P. 01/04/05 restored here 01/07/05
#else
	  WritePeakToRunFile(os, num, pDetector, pRun, pMethod, file, "no peaks", 0, 0, t, 0, 0, mul, 0); // P.P. 01/11/03
#endif
#endif // quick scan
	  bEmpty=0; // Something written
	}
	else {
#if !QUICK_SCAN() // cond added. P.P. 01/13/11
#if USE_TOTAL()
	  long t=-1;//pPeak->Time/data_rate;
#if USE_DIFFERENCE()  // added. P.P. 08/01/04
	  WritePeakToRunFile(os, num, pDetector, pRun, pMethod, file, "Clorine by diff", 100.0-total_concen, total_flags&1, t, 0, 0, mul, 0); // P.P. 08/01/04
#else
//	  WritePeakToRunFile(os, num, pDetector, pRun, pMethod, file, "Total", total_concen, total_flags&1, t, total_H, total_A, mul);
	  WritePeakToRunFile(os, num, pDetector, pRun, pMethod, file, "Total", total_concen, total_flags&1, t, total_H, total_A, mul, 0); // P.P. 01/11/03
#endif  // use_difference
	  bEmpty=0; // Something written
#endif //USE_TOTAL()
#endif // !quick scan
	}
      }
    }
  }
  if(os.good() && bEmpty) { // Diagnostic
    //message("Nothing to write to file:\n%s", RunFile.Get()); // DEBUG
    int bNoDetectors=1;
    int bNoDetectorsOn=1;
    int bNoPeakLists=1;
    for(int n=0; n<NUMDETECTORS; n++) {
      Detector *pDetector=&pMethod->det[n];
      if(pDetector) {
	bNoDetectors=0;
	if(!pDetector->detect_stat) {
	  bNoDetectorsOn=0;
	}
      }
      if(pPeakList[n]) {
	bNoPeakLists=0;
      }
    }
    char str[120];
    const char *err;
    const char *err2="";
    if(bNoDetectors) {
      // can't never be
      err="No detectors";
    }
    else if(bNoDetectorsOn) {
      err="No detectors On";
    }
    else if(bNoPeakLists) {
      err="No peak lists";
      err2="integration not start?";
    }
    else {
      // can't never be
      err="Nothing to print";
    }
    wsprintf(str,"\"%s\", \"%s\", -1.0, 0, 0, \"0\", \"none\", ",err, err2);
    os.write(str,strlen(str));
    PrintRunTimeAndComment(os, pRun, file);
    os.write("\r\n",2);
  }
}

#include "mainwind.h"
#if INTERNAL() && !_NOCOMM()
#include "wcpplib/fwstream.h"
#include "winutil/statline.h"
#include "comm.h"
#endif  // internal

void Job::SaveRunFile(Run *pRun, PeakList **pPeakList)
{
  Method *pMethod=this;
  if(FileName.IsEmpty()) {
    message("Use Save file As to create file");
    return;
  }
  else {
    Str file;
    {
      const char *_file=strrchr(FileName,'\\');
      if(_file) _file++;
      else _file=FileName;
      file=_file;
    }
    {
      char *ext=(char *)strrchr(file.Get(),'.');
      if(ext) *ext=0;
    }
    Str RunFile(256);
    MakeName(RunFile.GetPtr(), "run");
    owstream *pos = new ofwstream(RunFile,FALSE); // open existing
    if(!pos->good()) { delete pos; pos=new ofwstream(RunFile,TRUE); } // open new
    if(!pos->good()) {
      message("Error opening file:\n%s", RunFile.Get());
    }
    else {
      owstream &os=*pos;
      os.seek(0,wstream::end); // go to EOF
      if(!os.good()) {
	message("Error going to the end of file:\n%s", RunFile.Get());
      }
      else {
	// TWA_calc moved here from WritePeaks... P.P. 01/18/03
//	if(pRun->type_of_run!=CAL) {
//go to Acquire::Continue	  TWA_calc(pMethod, pRun, pPeakList, pRun->pointnumber);
//	}
	WritePeaksToRunFile(os, pRun, pMethod, pPeakList, file/*RunFile*/); // short name of file!!
	if(!os.good()) message("Error writting file:\n%s", RunFile.Get());
      }
    }
    delete pos;
#if INTERNAL() && !_NOCOMM()
    MainWindow *pMainWindow=(MainWindow *)GetApplicationObject()->MainWindow;
    Comm *pComm=pMainWindow->GetComm();
    if(pComm) {
      Str tmp_file(255);
      GetTempFileName(GetTempDrive(0), "tmp", 0, tmp_file.GetPtr());
      {
        ofwstream os(tmp_file);
	WritePeaksToRunFile(os, pRun, pMethod, pPeakList, file);
      }
      ifwstream is(tmp_file);
      wstreampos len=is.seek(0,wstream::end);
      is.seek(0,wstream::beg);
      if(is.good()) {
        if(pMainWindow->sl) pMainWindow->sl->Put("Send Peak List To COM Port");
        while(len) {
          uint8 l=len>250? 250: (uint8)len;
          uint8 buff[250];
          is.read(buff,l);
          pComm->send_msg(buff, l);
          len-=l;
          long err;
          for(;;) {
            uint8 mess_buf[260];
            uint8 nRead;
            int bAck=0;
            err=pComm->recv_msg(mess_buf,&nRead,&bAck);
            if(bAck) {
              if(pMainWindow->sl) pMainWindow->sl->SetStatus("Active");
              break;
            }
            else if(err) {  // else if message caused an error, repeat last message.
              break;
            }
            // nothing - wait ACK
          }
          if(err)
            break;
        }
        if(pMainWindow->sl) pMainWindow->sl->Restore();
      }
      AnsiToOem(tmp_file.GetPtr(),tmp_file.GetPtr());
      unlink(tmp_file);
    }
#endif  // internal
  }
}

//static void PrintDataToFile(owstream &os, Run *pRun, const char *file, char det_label, long run_time, long y_point)
//static void PrintDataToFile(owstream &os, Run *pRun, const char *file, char det_label, long time_min, long time_s, long time_ms, long y_point)
//static void PrintDataToFile(owstream &os, Run *pRun, const char *file, char det_label, long time_s, long time_ms, long y_point)
static void PrintDataToFile(owstream &os, Run *pRun, const char *file, char det_label, long time_s, long time_ms, long y_point, int nRun)
{
  Str str(200);
//  wsprintf(str.GetPtr(),"%s, %s, %s, %i, %li, %li,\r\n",PrintDate(pRun->start_time), PrintTime(pRun->start_time), file, 1+pRun->pointnumber, run_time, y_point);
// det label added, and time is formatted as time now. P.P. 02/12/19
//  wsprintf(str.GetPtr(),"%s, %s, %s, %c, %i, %li, %li,\r\n",PrintDate(pRun->start_time), PrintTime(pRun->start_time), file, det_label, 1+pRun->pointnumber, run_time, y_point);
//  wsprintf(str.GetPtr(),"%s, %s, %s, %c, %i, %li:%.2li.%.2li, %li,\r\n",PrintDate(pRun->start_time), PrintTime(pRun->start_time), file, det_label, 1+pRun->pointnumber, time_min, time_s, time_ms, y_point);
//  wsprintf(str.GetPtr(),"%s, %s, %s, %c, %i, %li.%.2li, %li,\r\n",PrintDate(pRun->start_time), PrintTime(pRun->start_time), file, det_label, 1+pRun->pointnumber, time_s, time_ms, y_point);
  wsprintf(str.GetPtr(),"%s, %s, %s, %c, %i, %i, %li.%.2li, %li,\r\n",PrintDate(pRun->start_time), PrintTime(pRun->start_time), file, det_label, 1+pRun->pointnumber, nRun, time_s, time_ms, y_point);
  os.write(str,str.Length());
//  "%ld:%.2ld\", ", t/60, t%60
}

static void PrintHeaderToFile(owstream &os, Run *pRun, const char *file)
{
  Str str(200);
//  wsprintf(str.GetPtr(),"%s, %s, %s, %i, %li, %li,\r\n",PrintDate(pRun->start_time), PrintTime(pRun->start_time), file, 1+pRun->pointnumber, run_time, y_point);
//  wsprintf(str.GetPtr(),"Date, Time, File, Detector, Point, run_time, Data,\r\n");
  wsprintf(str.GetPtr(),"Date, Time, File, Detector, Point, Run #, run_time, Data,\r\n"); // P.P. 02/22/12
  os.write(str,str.Length());
}

///// New function to write raw data to Excel or text file. P,P, 02/16/12
//void Job::SaveRawDataToFile(Run *pRun, char det_label, long run_time, long y_point)
//void Job::SaveRawDataToFile(Run *pRun, char det_label, long time_min, long time_s, long time_ms, long y_point)
void Job::SaveRawDataToFile(Run *pRun, char det_label, long time_s, long time_ms, long y_point)
{
  Method *pMethod=this;
  if(FileName.IsEmpty()) {
    message("Use Save file As to create file");
    return;
  }
  else {
    Str file;
    Str str(200);
    int nRun = 0; // added. P.P. 02/22/12
    {
      const char *_file=strrchr(FileName,'\\');
      if(_file) _file++;
      else _file=FileName;
      file=_file;
    }
    {
      char *ext=(char *)strrchr(file.Get(),'.');
      if(ext) *ext=0;
    }
    Str DataFile(256);
    MakeName(DataFile.GetPtr(), "csv");
    owstream *pos = new ofwstream(DataFile,FALSE); // open existing
    if(!pos->good()) {
      delete pos;
      pos=new ofwstream(DataFile,TRUE);  // open new
//      } // open new
      if(!pos->good()) {
        message("Error opening file:\n%s", DataFile.Get());
      }
      else {
        owstream &os=*pos;
        os.seek(0,wstream::end); // go to EOF
        if(!os.good()) {
	message("Error going to the end of file:\n%s", DataFile.Get());
        }
        else {
//	  PrintDataToFile(os, pRun, file, run_time, y_point);
	  PrintHeaderToFile(os, pRun, file);
	  if(!os.good()) message("Error writting file:\n%s", DataFile.Get());
        }
      }
    }
    else {
      if(!pos->good()) {
        message("Error opening file:\n%s", DataFile.Get());
      }
      else {
        owstream &os=*pos;
        os.seek(0,wstream::end); // go to EOF
        if(!os.good()) {
	message("Error going to the end of file:\n%s", DataFile.Get());
        }
        else {

//          Job *pJob;  // code added to get run#. P.P. 02/22/12
	  if(pRun) {
            nRun=0;
//            for(RunList::Node *prln=pJob->first(); prln; prln=prln->nextNode()) {
	    for(RunList::Node *prln=this->first(); prln; prln=prln->nextNode()) {
	      nRun++;
              if(pRun==prln) break;
            }
          }
//	  PrintDataToFile(os, pRun, file, det_label, run_time, y_point);
//	  PrintDataToFile(os, pRun, file, det_label, time_s, time_ms, y_point);  // P.P. 02/19/12
 	  PrintDataToFile(os, pRun, file, det_label, time_s, time_ms, y_point, nRun);  // P.P. 02/22/12
	  if(!os.good()) message("Error writting file:\n%s", DataFile.Get());
        }
      }
    }
    delete pos;
  }
}
///// End of new function for writing to Excel or text file. P,P, 02/16/12



