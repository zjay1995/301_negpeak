#ifndef  _DRAW_H
#define  _DRAW_H


#ifndef  _WCPPLIB_STR_H
#include "wcpplib\str.h"
#endif
class AxisInfo {
 public:
  Str Title;
  int bAuto;
  double mi, ma;
  int k;
  int bScale; // P.P. 01/22/13

  AxisInfo();
  void write (Ropstream os);
  Pvoid read (Ripstream is);
};

class PictInfo {
 public:
  LOGFONT lfUser;
  COLORREF TextColor;
  AxisInfo ai[2];
  COLORREF Color[2];

  PictInfo();
  void write (Ropstream os);
  Pvoid read (Ripstream is);
};


#include "internal.h" // FLAME()

#include "consts.h"

class Run;
class Job;
class RunRawData;
class PictInfo;
#ifdef FILTER
class FiltData;
#endif//FILTER
void Draw(Job *pJob, Run *pRun, PictInfo **pPictInfo,const DC &dc, const DC &ic, const WRect &rc, const RunRawData &runrawdata,
  #ifdef FILTER
  FiltData *pFiltData,
  #endif//FILTER
  int num, PeakList **pPeakList, HWND *hwnd
  );





class RunPictInfo : public PictInfo
{
};
/*
class RunPictInfo {
 public:
  LOGFONT lfUser;
  COLORREF TextColor;
  AxisInfo ai[2];
  COLORREF Color[2];

  RunPictInfo();
  void write (Ropstream os);
  Pvoid read (Ripstream is);
};
*/

class RunInfo: public List::Node
{
 public:
  RunInfo();

  Str det_name;
  Str peak_name;
  double conc;
  long height;
  long area;
  Str rt; //??
  Str alarm; //??

  Str date; //??
  Str time; //??
  double datetime; // date&time in local
  Str jobname;
  Str comment;
  /*double twa1;
  double twa2;*/
};

class Method;
void Draw1(Method *pMethod, RunPictInfo *pPictInfo,const DC &dc, const DC &ic, const WRect &rc, const List &list, HWND *hwnd);


#endif
