#pragma hdrstop

#include <time.h>
#include <stdio.h>

#include "winlib\dc.h"

#include "internal.h" // POINTS()
#include "summ_prn.h"
#include "country.h" // PrintDate()
#include "method.h"
#include "run.h"
#include "job.h"
#include "calc.h"
#include "debug.h"

void GetPage(DC &ic,WRect *rect,const WPoint &size);

void SummaryPrintout::SetPrintParams(DC *_pdc, WPoint pageSize)
{
TRACE("SummaryPrintout::SetPrintParams");
  Inherited::SetPrintParams(_pdc,pageSize);
  hLine=pdc->GetTextExtent("H").Y();
TRACE1("SummaryPrintout::SetPrintParams hline=%d", hLine);

  WRect rect;
  GetPage(*pdc,&rect,PageSize);

TRACE4("SummaryPrintout::SetPrintParams page=%d,%d,%d,%d", rect.left,rect.top,rect.right,rect.bottom);
  if(!hLine || (LinesPerPage=(rect.Height()-hLine)/hLine -3)<1) LinesPerPage=1;
  NumLines=0;//ZLP pRunList? pRunList->countChildren(): 0;

  Method *pMethod=pJob;
  Detector *pDetector=&pMethod->det[num];
  uint32 points=pMethod->method_table.points? pMethod->method_table.points: 1;
  for(int i=0; i<COMPONENT_ROWS; i++) {
    Component *pComponent=&pDetector->components[i];
    if(!pComponent->active_yn) continue;

    for(int Point=0; Point<POINTS(); Point++) {
      if( !((1L<<Point) & points) ) continue;
      NumLines++;
    }
  }
#if USE_TOTAL()
  NumLines++;
#endif
TRACE1("SummaryPrintout::SetPrintParams numlines=%d", NumLines);
}

BOOL SummaryPrintout::HasPage(int pageNumber)
{
TRACE("SummaryPrintout::HasPage");
  return (pageNumber-1)*LinesPerPage<NumLines;
}

void SummaryPrintout::PrintPage(int page, WRect& /*_rect*/, unsigned flags)
{
TRACE1("SummaryPrintout::PrintPage %d", page);
  WRect rect;
  GetPage(*pdc,&rect,PageSize);
  pdc->IntersectClipRect(rect);
  InitTabPositions(*pdc);

  const char *str;
  int y=rect.Top();
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
    Str buff(20+58);
    wsprintf(buff.GetPtr(), "Shift Report %s %s",  PrintDate(tim), PrintTime(tim));
    r.top=y;
    r.bottom=y+=hLine;
    str=buff;
    dc.TextOut(r.Left(),r.Top(),str);
  }

TRACE("SummaryPrintout::PrintPage after header");
  {
  DC &dc=*pdc;
  WRect r(rect);
  r.top=y;
  r.bottom=y+=hLine;
  SelTextAlign oldTextAlign(dc,taLeft|taTop);
  str="Pt";  dc.TextOut(r.Left(),r.Top(),str);
  {
    WRect _rc=r;
    _rc.Left()=r.Left()+TabPositions[0];
    _rc.Right()=r.Left()+TabPositions[1];
    str="Name"; dc.ExtTextOut(_rc.A(), ETO_CLIPPED, _rc, str, lstrlen(str), (LPINT)NULL);
  }
  str="Conc";  dc.TextOut(r.Left()+TabPositions[1],r.Top(),str);
  dc.SetTextAlign(taRight|taTop);
/*
  str="RTime"; dc.TextOut(r.Left()+TabPositions[2],r.Top(),str);
  str="run"; dc.TextOut(r.Left()+TabPositions[3],r.Top(),str);
  str="TWA"; dc.TextOut(r.Left()+TabPositions[4],r.Top(),str);
  str="max"; dc.TextOut(r.Left()+TabPositions[5],r.Top(),str);
  str="time"; dc.TextOut(r.Left()+TabPositions[6],r.Top(),str);
*/
// new layout for summary. Can be done on #if cond later. P.P. 12/22/04
  str=" run"; dc.TextOut(r.Left()+TabPositions[2],r.Top(),str);
  str="Min"; dc.TextOut(r.Left()+TabPositions[3],r.Top(),str);
  str="Avg"; dc.TextOut(r.Left()+TabPositions[4],r.Top(),str);  // P.P. 12/22/04
  str="Max"; dc.TextOut(r.Left()+TabPositions[5],r.Top(),str);
  str="time"; dc.TextOut(r.Left()+TabPositions[6],r.Top(),str);
// end of new layout. P.P. 12/22/04
  }

TRACE("SummaryPrintout::PrintPage after columns header");


  SelTextAlign oldTextAlign(*pdc,taLeft|taTop);
  Method *pMethod=pJob;
  Detector *pDetector=&pMethod->det[num];
  uint32 points=pMethod->method_table.points? pMethod->method_table.points: 1;

  int iLine=0;
  int iFrom=(page-1)*LinesPerPage;
  int iTo=page*LinesPerPage;
// order changed: points go first. P.P. 12/24/04
/* rem. P.P. 12/26/04
#if USE_TOTAL()
  int total_flags=0;
  double total_concen=0;
  double total_concen_min=0; // added. P.P. 12/23/04
  double total_concen_avg=0; // added. P.P. 12/23/04
  double total_concen_max=0; // added. P.P. 12/23/04
#endif // USE_TOTAL()
*/
  for(int Point=0; Point<POINTS(); Point++) {
    if( !((1L<<Point) & points) ) continue;
/* // rem. P.P. 12/26/04
// added to print total. P.P. 12/24/04
#if USE_TOTAL()
  total_flags=0;
  total_concen=0;
  total_concen_min=0; // added. P.P. 12/23/04
  total_concen_avg=0; // added. P.P. 12/23/04
  total_concen_max=0; // added. P.P. 12/23/04
#endif // USE_TOTAL()
// end. P.P. 12/24/04
*/
TRACE1("SummaryPrintout::PrintPage point=%d", Point);
    for(int i=0; i<COMPONENT_ROWS; i++, iLine++) {
      Component *pComponent=&pDetector->components[i];
      if(!pComponent->active_yn) continue;

      int iComponent=i;
TRACE1("SummaryPrintout::PrintPage iComponent=%d", iComponent);
/*  // moved up. P.P. 12/24/04
    for(int Point=0; Point<POINTS(); Point++) {
      if( !((1L<<Point) & points) ) continue;
*/
      if(iLine<iFrom) continue;
      if(iLine>=iTo) break;

      const char *component_name=pComponent->name.Get();
      double concen=concen_point[Point][num][iComponent];
      int flags=flags_point[Point][num][iComponent];
      long time=time_point[Point][num][iComponent];
/* rem. P.P. 12/26/04
// added. P.P. 12/24/04
#if USE_TOTAL()
      total_flags|=flags;
      if(flags&1) {
	total_concen+=concen;
      }
      concen=min_TWA_conc[Point][num][iComponent];
      total_concen_min+=concen; // added. P.P. 12/23/04 moved here from below to write total for each point individually. P.P. 09/26/04
      concen=max_TWA_conc[Point][num][iComponent];
      total_concen_max+=concen; // added. P.P. 12/23/04 moved here from below to write total for each point individually. P.P. 09/26/04
      concen=TWA_results[Point][num][iComponent];
      total_concen_avg+=concen; // added. P.P. 12/23/04 moved here from below to write total for each point individually. P.P. 09/26/04
#endif // USE_TOTAL()
// end. P.P. 12/24/04
*/
      WRect r(rect);
      r.top=y;
      r.bottom=y+=hLine;
//      PrintComponent(*pdc, r, Point, component_name, concen, total_flags, time
      PrintComponent(*pdc, r, Point, component_name, concen, flags, time
	, TWA_runs[Point][num][iComponent], TWA_results[Point][num][iComponent], max_TWA_conc[Point][num][iComponent]
//	, start_time[Point][num][iComponent]);
	, start_time[Point][num][iComponent], min_TWA_conc[Point][num][iComponent]); // min added. P.P. 12/22/04
//      iLine++; // moved up. P.P. 09/04/05
    }
// added to print Total. P.P. 12/24/04
#if USE_TOTAL()
TRACE("SummaryPrintout::PrintPage total");
    {
      long t=-1;//pPeak->Time/data_rate;
// added. P.P. 12/26/04
      double total_conc= total_point[Point][num];
      int total_flags= flags_point_total[Point][num]&1;
      long time=time_point[Point][num][0];
      int runs_total = TWA_runs_total[Point][num];
      double avg_total = TWA_total[Point][num];
      double max_total = max_TWA_total[Point][num];
      double min_total = min_TWA_total[Point][num];
// end. P.P. 12/26/04
      WRect r(rect);
      r.top=y;
      r.bottom=y+=hLine;
//      PrintComponent(*pdc, r, Point, "Total", total_concen, flags, /*time*/t
//      PrintComponent(*pdc, r, Point, "Total", total_concen, total_flags, /*time*/t
//	, TWA_runs[Point][num][0], total_concen_avg, total_concen_max
//	, start_time[Point][num][iComponent]);
//	, start_time[Point][num][0], total_concen_min); // min added. P.P. 12/22/04
// replacement. P.P. 12/26/04
      PrintComponent(*pdc, r, Point, "Total", total_conc, total_flags, t
	, runs_total, avg_total, max_total
	, time, min_total); // min added. P.P. 12/22/04
// end
      iLine++;
    }
#endif
// end of added code. P.P. 12/24/04
  }
TRACE("SummaryPrintout::PrintPage end");
}

void SummaryPrintout::InitTabPositions(DC &dc)
{
TRACE("SummaryPrintout::InitTabPositions");
  TabPositions[0]=dc.GetTextExtent("99 ").X();
  TabPositions[1]=TabPositions[0]; // variable size defined later
/*
  TabPositions[2]=TabPositions[1]+dc.GetTextExtent("12345.123  123456789").X();
  TabPositions[3]=TabPositions[2]+dc.GetTextExtent("  99.99%").X();
  TabPositions[4]=TabPositions[3]+dc.GetTextExtent("  123456789").X();
  TabPositions[5]=TabPositions[4]+dc.GetTextExtent("  99.99%").X();
  TabPositions[6]=TabPositions[5]+dc.GetTextExtent("  1234:00").X();
*/
// new tab positions. P.P. 12/22/04
  TabPositions[2]=TabPositions[1]+dc.GetTextExtent("  12345.123 ").X(); // for concen
  TabPositions[3]=TabPositions[2]+dc.GetTextExtent("  123456").X();     // for run #
  TabPositions[4]=TabPositions[3]+dc.GetTextExtent("  12345.123").X();  // for Min
  TabPositions[5]=TabPositions[4]+dc.GetTextExtent("  12345.123").X();     // fot Avg
  TabPositions[6]=TabPositions[5]+dc.GetTextExtent("  12345.123  ").X();    // for Max
// end of new tab positions. P.P. 12/22/04

  WRect rClient; GetPage(dc,&rClient,PageSize);

  int delta=rClient.Width()-(TabPositions[6]+dc.GetTextExtent("1").X());
  int wName=dc.GetTextExtent("Name ").X();
  if(delta<wName) delta=wName;
  for(int i=1; i<7; ++i) {
    TabPositions[i]+=delta;
  }
}

//void SummaryPrintout::PrintComponent(DC &dc, const WRect &r, int Point, const char *component_name, double concen, int flags, long t
//    , int TWA_runs, float TWA_results, float max_TWA_conc, time_t time
void SummaryPrintout::PrintComponent(DC &dc, const WRect &r, int Point, const char *component_name, double concen, int flags, long t
    , int TWA_runs, float TWA_results, float max_TWA_conc, time_t time, float min_TWA_conc // min added. P.P. 12/22/04
  )
{
TRACE("SummaryPrintout::PrintComponent");
  //WRect rect;

  SelTextAlign oldTextAlign(dc,taLeft|taTop);
  char str[50];
  if(Point>=0) { wsprintf(str,"%d",1+Point); dc.TextOut(r.Left(),r.Top(),str); }
  {
    WRect _rc=r;
    _rc.Left()=r.Left()+TabPositions[0];
    _rc.Right()=r.Left()+TabPositions[1];
    dc.ExtTextOut(_rc.A(), ETO_CLIPPED, _rc, component_name, lstrlen(component_name), (LPINT)NULL);
  }
  {
    if(flags&1) {
      sprintf(str, "%.3f", concen);
      if(flags&2) strcat( str, " H" );
      if(flags&4) strcat( str, " L" );
    }
    else{                      // added. P.P. 03/26/03
      sprintf(str, "<0.001");  //
    }                          //
    dc.TextOut(r.Left()+TabPositions[1],r.Top(),str);
  }
  dc.SetTextAlign(taRight|taTop);
/*
  wsprintf(str,"%ld:%.2ld", t/60, t%60); dc.TextOut(r.Left()+TabPositions[2],r.Top(),str);
  wsprintf(str,"%d", TWA_runs); dc.TextOut(r.Left()+TabPositions[3],r.Top(),str);
  sprintf(str,"%.3f",TWA_results); dc.TextOut(r.Left()+TabPositions[4],r.Top(),str);
  sprintf(str,"%.3f",max_TWA_conc); dc.TextOut(r.Left()+TabPositions[5],r.Top(),str);
  lstrcpy(str,PrintTime(time, TRUE)); dc.TextOut(r.Left()+TabPositions[6],r.Top(),str);
*/
// new layout. P.P. 12/22/04
//  wsprintf(str,"%ld:%.2ld", t/60, t%60); dc.TextOut(r.Left()+TabPositions[2],r.Top(),str);
  wsprintf(str,"%d", TWA_runs); dc.TextOut(r.Left()+TabPositions[2],r.Top(),str);
  sprintf(str,"%.3f",min_TWA_conc); dc.TextOut(r.Left()+TabPositions[3],r.Top(),str);
  sprintf(str,"%.3f",TWA_results); dc.TextOut(r.Left()+TabPositions[4],r.Top(),str);
  sprintf(str,"%.3f",max_TWA_conc); dc.TextOut(r.Left()+TabPositions[5],r.Top(),str);
  lstrcpy(str,PrintTime(time, TRUE)); dc.TextOut(r.Left()+TabPositions[6],r.Top(),str);
// end of new layout. P.P. 12/22/04
}

