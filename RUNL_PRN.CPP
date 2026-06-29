#include "includes.h"
#pragma hdrstop

#ifndef  _WINLIB_DC_H
#include "winlib\dc.h"
#endif

#ifndef  _RUNLIST_H
#include "runlist.h"
#endif
#ifndef  _RUNL_PRN_H
#include "runl_prn.h"
#endif

#ifndef  _COUNTRY_H
#include "country.h" // PrintDate()
#endif

void RunListPrintout::SetPrintParams(DC *_pdc, WPoint pageSize)
{
  Inherited::SetPrintParams(_pdc,pageSize);
  hLine=pdc->GetTextExtent("H").Y();
  if(!hLine || (LinesPerPage=(PageSize.Y()-hLine)/hLine)<1) LinesPerPage=1;
  NumLines=pRunList? pRunList->countChildren(): 0;
}

BOOL RunListPrintout::HasPage(int pageNumber)
{
  return (pageNumber-1)*LinesPerPage<NumLines;
}

void GetPage(DC &ic,WRect *rect,const WPoint &size);
void RunListPrintout::PrintPage(int page, WRect& /*_rect*/, unsigned flags)
{
  WRect rect;
  GetPage(*pdc,&rect,PageSize);
  pdc->IntersectClipRect(rect);
  int dw=pdc->GetTextExtent("  ").X();
  int w1=pdc->GetTextExtent("234").X();
  int w2=pdc->GetTextExtent("22-12-1993").X();
  int w3=pdc->GetTextExtent("22-22pm").X();

  int y=rect.Top();
  SelTextAlign oldTextAlign(*pdc,taLeft|taTop);
  int bFirst=1;
  for(int i=(page-1)*LinesPerPage; i<NumLines && i< page*LinesPerPage; i++) {
    RunList::Node *prln=pRunList? pRunList->GetChild(i): NULL;
    if(prln) {
    {
      SelTextAlign oldTextAlign(*pdc,taRight|taTop);
      char str[12]; wsprintf(str, "%d",1+i);
      pdc->TextOut(rect.left+w1,y,bFirst? "": str);
      }
      pdc->TextOut(rect.left+w1+dw,y,bFirst? "Date": PrintDate(prln->start_time));
      pdc->TextOut(rect.left+w1+dw+w2+dw,y,bFirst? "Time": PrintTime(prln->start_time));
      pdc->TextOut(rect.left+w1+dw+w2+dw+w3+dw,y,bFirst? "Comment": prln->comment.Get());
      y+=hLine;
      if(bFirst) {
	i--;
	bFirst=0;
      }
    }
  }
}

