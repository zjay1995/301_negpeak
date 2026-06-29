#ifndef  _RUNL_PRN_H
#define  _RUNL_PRN_H

#ifndef  _WINUTIL_WPRINT_H
#include "winutil\wprint.h"
#endif

class RunList;

class RunListPrintout : public Printout
{
  typedef Printout Inherited;
 protected:
  RunList *pRunList;
  int hLine;
  int LinesPerPage;
  int NumLines;
 public:
  RunListPrintout(LPCSTR ATitle, RunList *_pRunList): Printout(ATitle), pRunList(_pRunList) {}
  virtual void SetPrintParams(DC *_pdc, WPoint pageSize);
  virtual BOOL HasPage(int pageNumber);
  virtual void PrintPage(int page, WRect& rect, unsigned flags);
};


#endif
