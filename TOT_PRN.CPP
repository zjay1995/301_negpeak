#pragma hdrstop

#include <time.h>
#include <stdio.h>

#include "winlib\dc.h"

#include "internal.h" // POINTS()
#if TOTAL_SUMMARY()
#include "tot_prn.h"
#include "country.h" // PrintDate()
#include "method.h"
#include "run.h"
#include "job.h"
#include "calc.h"
#include "debug.h"


void GetPage(DC &ic,WRect *rect,const WPoint &size);

void TotalPrintout::SetPrintParams(DC *_pdc, WPoint pageSize)
{
TRACE("TotalPrintout::SetPrintParams");
  Inherited::SetPrintParams(_pdc,pageSize);
  hLine=pdc->GetTextExtent("H").Y();
TRACE1("TotalPrintout::SetPrintParams hline=%d", hLine);

  WRect rect;
  GetPage(*pdc,&rect,PageSize);

TRACE4("TotalPrintout::SetPrintParams page=%d,%d,%d,%d", rect.left,rect.top,rect.right,rect.bottom);
  if(!hLine || (LinesPerPage=(rect.Height()-hLine)/hLine -3)<1) LinesPerPage=1;
  NumLines=0;

  Method *pMethod=pJob;
  Detector *pDetector=&pMethod->det[num];
  uint32 points=pMethod->method_table.points? pMethod->method_table.points: 1;
  for(TotalListNode *pNode=(TotalListNode *)total_list.first(); pNode; pNode=(TotalListNode *)pNode->nextNode()) {
    NumLines++;
  }
  NumLines+=3;
TRACE1("TotalPrintout::SetPrintParams numlines=%d", NumLines);
}

BOOL TotalPrintout::HasPage(int pageNumber)
{
TRACE("TotalPrintout::HasPage");
  return (pageNumber-1)*LinesPerPage<NumLines;
}

void TotalPrintout::PrintPage(int page, WRect& /*_rect*/, unsigned flags)
{
TRACE1("TotalPrintout::PrintPage %d", page);
  WRect rect;
  GetPage(*pdc,&rect,PageSize);
  pdc->IntersectClipRect(rect);

  Method *pMethod=pJob;
  Detector *pDetector=&pMethod->det[num];
  uint32 points=pMethod->method_table.points? pMethod->method_table.points: 1;

  const char *str;
  int y=rect.Top();
  int wLine=rect.Width();
  if(POINTS()) wLine/=POINTS();
  {
    DC &dc=*pdc;
    WRect r(rect);
    r.top=y;
    r.bottom=y+=hLine;
#if GC501() // cond added. P.P. 12/24/04
    str="GC501B Gas Chromatograph, \xa9 1994\x96""2005, PID Analyzers, LLC";
#else
    str="GC301B Gas Chromatograph, \xa9 1994\x96""2005, PID Analyzers, LLC";  // P.P. 12/22/04
#endif
    dc.TextOut(r.Left(),r.Top(),str);

    time_t tim;
    time( &tim );
    /*if(total_list.first()) { //??
      TotalListNode *pNode=(TotalListNode *)total_list.first();
      tim=pNode->t;
    }*/
    Str buff(20+58);
    wsprintf(buff.GetPtr(), "Analisys Result Report %s, Detector '%s'", PrintDate(tim), pDetector->Label.Get());
    r.top=y;
    r.bottom=y+=hLine;
    str=buff;
    dc.TextOut(r.Left(),r.Top(),str);
  }

TRACE("TotalPrintout::PrintPage after header");
  {
    for(int Point=0; Point<POINTS(); Point++) {
      if( !((1L<<Point) & points) ) continue;

      DC &dc=*pdc;
      WRect r(rect);
      r.top=y;
      r.bottom=r.top+hLine;
      r.left=rect.left+Point*wLine;
      r.right=r.left+wLine;
      {
        char str[50];
        sprintf(str,"Point #%d",Point+1);

        SelTextAlign oldTextAlign(dc,taRight|taTop);
        dc.TextOut(r.Left()+r.Width(),r.Top(),str);
      }

      r.top+=hLine; r.bottom=r.top+hLine;
      __PrintComponent(dc, r, "Time", "Total");

      r.top+=hLine; r.bottom=r.top+hLine;
      __PrintComponent(dc, r, "", pDetector->units);
    }
    y+=hLine*3;
TRACE("TotalPrintout::PrintPage after columns header");
  }


  float min_conc[POINTS()], max_conc[POINTS()];
  double ave_conc[POINTS()]; int ave_n[POINTS()]={0};

  int iLine=0;
  int iFrom=(page-1)*LinesPerPage;
  int iTo=page*LinesPerPage;
  for(TotalListNode *pNode=(TotalListNode *)total_list.first(); pNode; pNode=(TotalListNode *)pNode->nextNode(), iLine++) {
    if(iLine<iFrom) continue;
    if(iLine>=iTo) break;
    for(int Point=0; Point<POINTS(); Point++) {
      if( !((1L<<Point) & points) ) continue;
TRACE1("TotalPrintout::PrintPage point=%d", Point);

      if(!pNode->tim[Point][num]) continue; // not measured

      float conc=pNode->conc[Point][num];
      if(ave_n[Point]==0) {
        min_conc[Point]=max_conc[Point]=ave_conc[Point]=conc;
      }
      else {
        if(min_conc[Point]>conc) min_conc[Point]=conc;
        if(max_conc[Point]<conc) max_conc[Point]=conc;
        ave_conc[Point]+=conc;
      }
      ave_n[Point]++;

      WRect r(rect);
      r.top=y;
      r.bottom=r.top+hLine;
      r.left=rect.left+Point*wLine;
      r.right=r.left+wLine;
      PrintComponent(*pdc, r, pNode->tim[Point][num], conc);
    }
    y+=hLine;
  }

  if(!pNode) { // last page
    for(int Point=0; Point<POINTS(); Point++) {
      if( !((1L<<Point) & points) ) continue;

      if(!ave_n[Point]) continue; // not measured
      ave_conc[Point]/=ave_n[Point];

      WRect r(rect);
      r.top=y;
      r.bottom=r.top+hLine;
      r.left=rect.left+Point*wLine;
      r.right=r.left+wLine;

      _PrintComponent(*pdc, r, "Max", max_conc[Point]);  iLine++;
      r.top+=hLine; r.bottom=r.top+hLine;
      _PrintComponent(*pdc, r, "Min", min_conc[Point]);  iLine++;
      r.top+=hLine; r.bottom=r.top+hLine;
      _PrintComponent(*pdc, r, /*"Average"*/"Avr", ave_conc[Point]);  iLine++;
    }
  }

TRACE("TotalPrintout::PrintPage end");
}


void TotalPrintout::__PrintComponent(DC &dc, const WRect &r, const char *t, const char *conc)
{
  SelTextAlign oldTextAlign(dc,taRight|taTop);
  dc.TextOut(r.Left()+r.Width()/2,r.Top(),t);
  dc.TextOut(r.Left()+r.Width(),r.Top(),conc);
}

void TotalPrintout::_PrintComponent(DC &dc, const WRect &r, const char *t, float conc)
{
  char str[50];
  sprintf(str,"%.3f",conc);
  __PrintComponent(dc, r, t, str);
}

void TotalPrintout::PrintComponent(DC &dc, const WRect &r, time_t t, float conc)
{
TRACE("TotalPrintout::PrintComponent");
  SelTextAlign oldTextAlign(dc,taRight|taTop);
  _PrintComponent(dc, r, PrintTime(t, TRUE), conc);
}


#endif // TOTAL_SUMMARY()
