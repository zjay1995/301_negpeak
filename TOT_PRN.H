#ifndef  _TOT_PRN_H
#define  _TOT_PRN_H

#ifndef  _WINUTIL_WPRINT_H
#include "winutil\wprint.h"
#endif

#ifndef  _INTERNAL_H
#include "internal.h"
#endif


class Job;

class TotalPrintout : public Printout
{
  typedef Printout Inherited;
 protected:
  Job *pJob; int num;
  int hLine;
  int LinesPerPage;
  int NumLines;
 public:
  TotalPrintout(LPCSTR ATitle, Job *_pJob, int _num): Printout(ATitle), pJob(_pJob), num(_num) {}
  virtual void SetPrintParams(DC *_pdc, WPoint pageSize);
  virtual BOOL HasPage(int pageNumber);
  virtual void PrintPage(int page, WRect& rect, unsigned flags);

  void PrintComponent(DC &dc, const WRect &r, time_t t, float conc);
  void _PrintComponent(DC &dc, const WRect &r, const char *t, float conc);
  void __PrintComponent(DC &dc, const WRect &r, const char *t, const char *conc);
};


#endif
