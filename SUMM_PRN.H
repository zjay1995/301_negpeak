#ifndef  _SUMM_PRN_H
#define  _SUMM_PRN_H

#ifndef  _WINUTIL_WPRINT_H
#include "winutil\wprint.h"
#endif

class Job;

class SummaryPrintout : public Printout
{
  typedef Printout Inherited;
 protected:
  Job *pJob; int num;
  int hLine;
  int LinesPerPage;
  int NumLines;
  int TabPositions[7];
  void InitTabPositions(DC &dc);
 public:
  SummaryPrintout(LPCSTR ATitle, Job *_pJob, int _num): Printout(ATitle), pJob(_pJob), num(_num) {}
  virtual void SetPrintParams(DC *_pdc, WPoint pageSize);
  virtual BOOL HasPage(int pageNumber);
  virtual void PrintPage(int page, WRect& rect, unsigned flags);

//  void PrintComponent(DC &dc, const WRect &r, int Point, const char *component_name, double concen, int flags, long t
//    , int TWA_runs, float TWA_results, float max_TWA_conc, time_t time
  void PrintComponent(DC &dc, const WRect &r, int Point, const char *component_name, double concen, int flags, long t
    , int TWA_runs, float TWA_results, float max_TWA_conc, time_t time, float min_TWA_conc // min added. P.P. 12/22/04
  );
};


#endif
