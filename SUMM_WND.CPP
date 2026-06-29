#include "includes.h"
#pragma hdrstop

#include <window.h>
#include <listbox.h>

#include <math.h>

#include "winlib\dc.h"
#include "winlib\pc.h"
#include "winlib\pen.h"
#include "winlib\selpen.h"
#include "winlib\menu.h"
#include "winutil\thinking.h"
#include "winutil\wutil.h"


#include "id.h"
#include "method.h"
#include "job.h"
#include "calc.h"
#include "country.h"
#include "summ_prn.h"
#include "peak_app.h"


class SummaryListBox : public TListBox {
  typedef TListBox Inherited;
 public:
  SummaryListBox(PTWindowsObject AParent, int AnId, int X, int Y, int W, int H, PTModule AModule = NULL)
    : TListBox(AParent, AnId, X, Y, W, H, AModule ) {}
  SummaryListBox(PTWindowsObject AParent, int ResourceId, PTModule AModule = NULL)
    : TListBox(AParent, ResourceId, AModule ) {}
  virtual void WMKeyDown(RTMessage Msg) = [WM_FIRST + WM_KEYDOWN];
};

void SummaryListBox::WMKeyDown(RTMessage Msg)
{
  switch(Msg.WParam) {
    case VK_RETURN:
      PostMessage(Parent->HWindow, WM_COMMAND, CM_PEAK_EDITITEM, 0L);
    break;
    case VK_INSERT:
      PostMessage(Parent->HWindow, WM_COMMAND, CM_PEAK_ADDITEM, 0L);
    break;
    case VK_DELETE:
      PostMessage(Parent->HWindow, WM_COMMAND, CM_PEAK_DELITEM, 0L);
    break;
  }
  DefWndProc(Msg);
}

#include "internal.h"// USE_TOTAL()
class SummaryWindow : public TWindow {
  typedef TWindow Inherited;
  PTListBox list;
  int HeaderHeight;
  int num;
  virtual LPSTR GetClassName();
  virtual void GetWindowClass(WNDCLASS &AWndClass);
  int TabPositions[7];
  void InitTabPositions(DC &dc);
  double H, A;
 public:
  SummaryWindow(PTWindowsObject AParent,int num,char far *ATitle,PTModule AModule = NULL);
  ~SummaryWindow();
 protected:
  virtual void SetupWindow();
  virtual void ActivationResponse(WORD Activated,  BOOL IsIconified);
  virtual void WMInitMenuPopup(RTMessage) = [WM_FIRST + WM_INITMENUPOPUP];
  virtual void Paint(HDC PaintDC, PAINTSTRUCT _FAR & PaintInfo);
  virtual void DrawItem(DRAWITEMSTRUCT _FAR & DrawInfo);
  virtual void WMMeasureItem(TMessage& Msg) = [WM_FIRST + WM_MEASUREITEM];
  virtual void WMSize(RTMessage Msg) = [WM_FIRST + WM_SIZE];
  virtual void WMSetFocus(RTMessage Msg) = [WM_FIRST + WM_SETFOCUS];
  virtual void WMCommand(RTMessage Msg) = [WM_FIRST + WM_COMMAND];
  virtual void WMIsItYourName(RTMessage Msg) = [WM_FIRST + WM_IS_IT_YOUR_NAME];
  virtual void WMRefresh(RTMessage Msg) = [WM_FIRST + WM_REFRESH];
  virtual void CMAddItem(RTMessage Msg) = [CM_FIRST +  CM_PEAK_ADDITEM];
  virtual void CMDeleteItem(RTMessage Msg) = [CM_FIRST + CM_PEAK_DELITEM];

  virtual void CMPrint(RTMessage) = [CM_FIRST + CM_PRINT];
  virtual void CMEditCopy(RTMessage) = [CM_FIRST + CM_EDITCOPY];
  virtual void CMEditCopyToFile(RTMessage) = [CM_FIRST + CM_EDITCOPYTOFILE];
  virtual void HandleList(RTMessage Msg) = [ID_FIRST + IDD_PEAKLIST];
  void FillList();
  virtual void WMNewPeak(RTMessage Msg) = [WM_FIRST + WM_NEW_PEAK];

  long CreateFile(UINT cf, const char *file);

  void append(const char *s, int Index) {
    //pPeakList->append(s,Index);
    if(Index>=list->GetCount()) {
      list->AddString((LPSTR)s);
    }
    else {
      list->InsertString((LPSTR)s,Index+1);
    }
    list->SetSelIndex(Index+1);
  }
  void remove(int Index) {
    //pPeakList->remove(Index);
    list->DeleteString(Index);
    list->SetSelIndex(Index>1?Index-1:0);
  }

  SummaryWindow(StreamableInit);
  virtual void write (Ropstream os);
  virtual Pvoid read (Ripstream is);
  virtual const Pchar streamableName() const { return name; }
 public:
  static PTStreamable build();
  static char far name[];

//  void DrawComponent(DC &dc, const WRect &r, int Point, const char *name, double concen, int flags, long t
//    , int TWA_runs, float TWA_results, float max_TWA_conc, time_t time
  void DrawComponent(DC &dc, const WRect &r, int Point, const char *name, double concen, int flags, long t
    , int TWA_runs, float TWA_results, float max_TWA_conc, time_t time, float min_TWA_conc // min added. P.P. 12/22/04
  );
};

LPSTR SummaryWindow::GetClassName()
{
  return name;
}

void SummaryWindow::GetWindowClass(WNDCLASS& WndClass)
{
  Inherited::GetWindowClass(WndClass);
  WndClass.hIcon = LoadIcon(GetModule()->hInstance,"LIST");
}


SummaryWindow::SummaryWindow(PTWindowsObject AParent,int _num,char far *ATitle,PTModule AModule)
  : TWindow(AParent,ATitle,AModule),
    list(new SummaryListBox(this, IDD_PEAKLIST, 0, 0, 0, 0, AModule)),
    HeaderHeight(20),
    num(_num)
  , A(0), H(0)
{
  Attr.Style |= WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE;
  if(list) {
    list->Attr.Style&=~(WS_BORDER|LBS_SORT);
    list->Attr.Style|=LBS_NOTIFY|LBS_OWNERDRAWFIXED|/*LBS_HASSTRINGS|*/LBS_NOINTEGRALHEIGHT|WS_VSCROLL|WS_CHILD|WS_VISIBLE|WS_GROUP|WS_TABSTOP;
  }

}

SummaryWindow::~SummaryWindow(void)
{
}

void SummaryWindow::SetupWindow()
{
  Inherited::SetupWindow();
  FillList();
}

SummaryWindow::SummaryWindow(StreamableInit)
 : TWindow(streamableInit), list(0)
{
};

TStreamableClass RegSummaryWindow(SummaryWindow::name, SummaryWindow::build, __DELTA(SummaryWindow));
char far SummaryWindow::name[]="SummaryWindow";
PTStreamable SummaryWindow::build()
{
  return new SummaryWindow(streamableInit);
}

void SummaryWindow::write(Ropstream os)
{
  Inherited::write(os);
  os<<HeaderHeight;
  os<<num;
  PutChildPtr(os,list);
}

Pvoid SummaryWindow::read(Ripstream is)
{
  if(!Inherited::read(is)) return NULL;
  is>>HeaderHeight;
  is>>num;
  GetChildPtr(is,(RPTWindowsObject)list);
  return this;
}


void SummaryWindow::InitTabPositions(DC &dc)
{
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

  WRect rClient; ::GetClientRect(*this,&rClient);
  int delta=rClient.Width()-(TabPositions[6]+dc.GetTextExtent("1").X());
  int wName=dc.GetTextExtent("Name ").X();
  if(delta<wName) delta=wName;
  for(int i=1; i<7; ++i) {
    TabPositions[i]+=delta;
  }
}

void SummaryWindow::FillList()
{
  if(!list) return;
  A=0; H=0;
  ::SendMessage(list->HWindow,WM_SETREDRAW,FALSE,0L);
  list->ClearList();

  Job *pJob=(Job *)SendMessage(Parent->HWindow,WM_GET_JOB,0,0L);
  //Run *pRun=(Run *)SendMessage(Parent->HWindow,WM_GET_RUN,0,0L);
  //PeakList *pPeakList=((PeakList **)SendMessage(Parent->HWindow,WM_GET_PEAKLIST,0,0L))[num];
  {
    Method *pMethod=pJob;
    Detector *pDetector=&pMethod->det[num];
    uint32 points=pMethod->method_table.points? pMethod->method_table.points: 1;
    for(int i=0; i<COMPONENT_ROWS; i++) {
      Component *pComponent=&pDetector->components[i];
      if(!pComponent->active_yn) continue;
      for(int Point=0; Point<POINTS(); Point++) {
	if( !((1L<<Point) & points) ) continue;
	list->AddString((LPSTR)MAKELONG(i,Point));
      }
    }
#if !QUICK_SCAN() // cond added. P.P. 12/14/10
#if USE_TOTAL()
// added. P.P. 09/26/04
    for(int Point=0; Point<POINTS(); Point++) {
      if( !((1L<<Point) & points) ) continue;
      list->AddString((LPSTR)MAKELONG(-1,Point));
    }
// end of added code
//    list->AddString((LPSTR)-1);  // replaced for above. P.P. 09/26/04
#endif // USE_TOTAL()
#endif // !quick_scan
    list->SetSelIndex(0);
  }
  ::SendMessage(list->HWindow,WM_SETREDRAW,TRUE,0L);
  ::InvalidateRect(list->HWindow,NULL,FALSE);
}

void SummaryWindow::WMNewPeak(RTMessage Msg)
{
  if(!list) return;
  if(!Msg.WParam==num) return;

  //#if USE_TOTAL()
  //int i=list->GetCount();
  //list->InsertString((LPSTR)(Peak *)Msg.LParam, i-1);
  //#else // USE_TOTAL()
  list->AddString((LPSTR)(Peak *)Msg.LParam);
  //#endif // USE_TOTAL()
}

void SummaryWindow::WMIsItYourName(RTMessage Msg)
{
  Msg.Result=lstrcmpi(GetClassName(),(LPCSTR)Msg.LParam)==0 && Msg.WParam==num;
}

void SummaryWindow::WMRefresh(RTMessage Msg)
{
  int bGraphOnly=Msg.WParam;
  if(!bGraphOnly) FillList();
}

void SummaryWindow::WMMeasureItem(TMessage& Msg)
{
  MEASUREITEMSTRUCT FAR *lpmisCtl = (MEASUREITEMSTRUCT FAR*) Msg.LParam;
  //lpmisCtl->itemWidth=r.Width()/((nPict+nRow/2)/nRow);
  TEXTMETRIC tm;
  ClientDC(HWindow).GetTextMetrics(&tm);

  lpmisCtl->itemHeight=tm.tmHeight+tm.tmExternalLeading;
  HeaderHeight=lpmisCtl->itemHeight+2;
}

extern char data_rate;           // rate data received in pts/sec.
extern float TWA_total[POINTS()][NUMDETECTORS];  	// TWA for total
extern float max_TWA_total[POINTS()][NUMDETECTORS];	// total max. in TWA
extern float min_TWA_total[POINTS()][NUMDETECTORS];	// total min. in TWA 
extern float total_point[POINTS()][NUMDETECTORS];  //added for total concentrations in points. P.P. 12/26/04
extern int TWA_runs_total[POINTS()][NUMDETECTORS]; // P.P. 12/26/04
extern int flags_point_total[POINTS()][NUMDETECTORS];  //added for total concentrations in points. P.P. 12/26/04

#include <stdio.h> // sprintf("%f")



//void SummaryWindow::DrawComponent(DC &dc, const WRect &r, int Point, const char *component_name, double concen, int flags, long t
//    , int TWA_runs, float TWA_results, float max_TWA_conc, time_t time
void SummaryWindow::DrawComponent(DC &dc, const WRect &r, int Point, const char *component_name, double concen, int flags, long t
    , int TWA_runs, float TWA_results, float max_TWA_conc, time_t time, float min_TWA_conc // min added. P.P. 12/22/04
  )
{
  SelTextAlign oldTextAlign(dc,taLeft|taTop);
  char str[50];
  if(Point>=0) { wsprintf(str,"%d",1+Point); dc.TextOut(r.Left(),r.Top(),str); }
  {
    WRect _rc=r;
    _rc.Left()=r.Left()+TabPositions[0];
    _rc.Right()=r.Left()+TabPositions[1];
    dc.ExtTextOut(_rc.A(), ETO_CLIPPED, _rc, component_name, lstrlen(component_name), (LPINT)NULL);
  }
  Job *pJob=(Job *)SendMessage(Parent->HWindow,WM_GET_JOB,0,0L);
  //Run *pRun=(Run *)SendMessage(Parent->HWindow,WM_GET_RUN,0,0L);
  //PeakList *pPeakList=((PeakList **)SendMessage(Parent->HWindow,WM_GET_PEAKLIST,0,0L))[num];
  /*if(pJob && pRun)*/ {
    if(flags&1) {
      sprintf(str, "%.3f", concen);
      if(flags&2) strcat( str, " H" );
      if(flags&4) strcat( str, " L" );
//      dc.TextOut(r.Left()+TabPositions[1],r.Top(),str); moved down. P.P. 12/26/04
    }
    else{                      // added. P.P. 03/26/03
      sprintf(str, "<0.001");  //
    }                          //
    dc.TextOut(r.Left()+TabPositions[1],r.Top(),str); // moved here. P.P. 12/26/04
  }
  dc.SetTextAlign(taRight|taTop);
/* // ret time removed from the summary. P.P. 12/22/04
  if(t>=0) { // cond added to skip ret time for Total. P.P. 09/26/04
    wsprintf(str,"%ld:%.2ld", t/60, t%60); dc.TextOut(r.Left()+TabPositions[2],r.Top(),str);
  }
  else { // blank for ret time place. P.P. 09/26/04
    wsprintf(str," "); dc.TextOut(r.Left()+TabPositions[2],r.Top(),str);
  }
  wsprintf(str,"%d", TWA_runs); dc.TextOut(r.Left()+TabPositions[3],r.Top(),str);
  sprintf(str,"%.3f",TWA_results); dc.TextOut(r.Left()+TabPositions[4],r.Top(),str);
  sprintf(str,"%.3f",max_TWA_conc); dc.TextOut(r.Left()+TabPositions[5],r.Top(),str);
  lstrcpy(str,PrintTime(time, TRUE)); dc.TextOut(r.Left()+TabPositions[6],r.Top(),str);
*/
// new layout. P.P. 12/22/04
  wsprintf(str,"%d", TWA_runs); dc.TextOut(r.Left()+TabPositions[2],r.Top(),str);
  sprintf(str,"%.3f",min_TWA_conc); dc.TextOut(r.Left()+TabPositions[3],r.Top(),str);
  sprintf(str,"%.3f",TWA_results); dc.TextOut(r.Left()+TabPositions[4],r.Top(),str);
  sprintf(str,"%.3f",max_TWA_conc); dc.TextOut(r.Left()+TabPositions[5],r.Top(),str);
  lstrcpy(str,PrintTime(time, TRUE)); dc.TextOut(r.Left()+TabPositions[6],r.Top(),str);
// end. P.P. 12/22/04
}

void SummaryWindow::DrawItem(DRAWITEMSTRUCT _FAR & DrawInfo)
{
  DC dc(DrawInfo.hDC);
  WRect r(DrawInfo.rcItem);
  Color colorBk;
  Color color;
  if(DrawInfo.itemState&ODS_SELECTED) {		// Reflect selection state
    colorBk=GetSysColor(COLOR_HIGHLIGHT);
    color=GetSysColor(COLOR_HIGHLIGHTTEXT);
  }
  else {
    colorBk=GetSysColor(COLOR_WINDOW);
    color=GetSysColor(COLOR_WINDOWTEXT);
  }
  dc.FillRect(r,LogBrush(colorBk));

  if(int(DrawInfo.itemID)>=0 && /*DrawInfo.itemAction&ODA_DRAWENTIRE*/1) {	// Redraw if neccesary
    SelTextColor oldTextColor(dc,color);
    SelBkMode oldBkMode(dc,bkmTransparent);

    int Point     =HIWORD(DrawInfo.itemData);
    int iComponent=LOWORD(DrawInfo.itemData);
    // if !pPeak - this is a total
    Job *pJob=(Job *)SendMessage(Parent->HWindow,WM_GET_JOB,0,0L);
    //Run *pRun=(Run *)SendMessage(Parent->HWindow,WM_GET_RUN,0,0L);
    //long mul=pow10( pRun->runrawdata.gain[num] );
    Method *pMethod=pJob;
    if(iComponent>=0) {
      Component *pComponent=&pMethod->det[num].components[iComponent];
      const char *component_name=pComponent->name.Get();
      double concen=concen_point[Point][num][iComponent];
      int flags=flags_point[Point][num][iComponent];
      long time=time_point[Point][num][iComponent];
      DrawComponent(dc, r, Point, component_name, concen, flags, time
	, TWA_runs[Point][num][iComponent], TWA_results[Point][num][iComponent], max_TWA_conc[Point][num][iComponent]
//	, start_time[Point][num][iComponent]
	, start_time[Point][num][iComponent], min_TWA_conc[Point][num][iComponent] // min added. P.P. 12/22/04
      );
    }
#if USE_TOTAL()
    else {
/*  // removed and replaced for code below. P.P. 12/26/04
      int total_flags=0;      // restored. 12/23/04. 2 lines moved down. P.P. 09/2/04
      double total_concen=0;  //
      double total_concen_min=0; // added. P.P. 12/23/04
      double total_concen_avg=0; // added. P.P. 12/23/04
      double total_concen_max=0; // added. P.P. 12/23/04
      uint32 points=pMethod->method_table.points? pMethod->method_table.points: 1;
      Component *pComponent=&pMethod->det[num].components[0];
      for(int i=0; i<COMPONENT_ROWS; i++, pComponent++) {
	if(!pComponent->active_yn) continue;

	int iComponent=i;
//rem	for(int Point=0; Point<POINTS(); Point++) {
//          int total_flags=0;       // restored above. 12/23/04. 2 lines moved from above - new for each point. P.P. 09/26/04
//          double total_concen=0;   //
//rem	  if( !((1L<<Point) & points) ) continue;

	  Component *pComponent=&pMethod->det[num].components[iComponent];
	  const char *component_name=pComponent->name.Get();
	  double concen=concen_point[Point][num][iComponent];
	  int flags=flags_point[Point][num][iComponent];
	  //long time=time_point[Point][num][iComponent];
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
/*
	  long t=-1;//pPeak->Time/data_rate;
#if USE_DIFFERENCE()  // added. P.P. 09/26/04
//          DrawComponent(dc, r, -1, "Chlorine by diff", 100.0-total_concen, total_flags&1, t
          DrawComponent(dc, r, Point, "Chlorine by diff", 100.0-total_concen, total_flags&1, t // for given point. P.P. 09/26/04
//	, 0, 0, 0, 0
	, 0, 0, 0, 0, 0  // min conc added. P.P. 12/22/04
	  );
#else
//          DrawComponent(dc, r, -1, "Total", total_concen, total_flags&1, t
          DrawComponent(dc, r, Point, "Total", total_concen, total_flags&1, t  // for given point. P.P. 09/26/04
//	, 0, 0, 0, 0
//	, 0, 0, 0, 0, 0  // min conc added. P.P. 12/22/04
	, TWA_runs[Point][num][iComponent], TWA_results[Point][num][iComponent], max_TWA_conc[Point][num][iComponent]
//	, start_time[Point][num][iComponent]
	, start_time[Point][num][iComponent], min_TWA_conc[Point][num][iComponent] // min added. P.P. 12/22/04
	  );
#endif  // use_difference

// end of moved code.
#endif // USE_TOTAL()
// rem	}
      }

  // this part moved up. P.P. 09/26/04
*/
      long t=-1;//pPeak->Time/data_rate;
#if USE_DIFFERENCE()  // added. P.P. 09/26/04  // should be changed correspondingly! P.P. 12/26/04
      DrawComponent(dc, r, Point, "Chlorine by diff", 100.0-total_concen, total_flags&1, t
//      DrawComponent(dc, r, Point, "Chlorine by diff", 100.0-total_concen, total_flags&1, t // for given point. P.P. 09/26/04
	, 0, 0, 0, 0
      );
#else
/*
      DrawComponent(dc, r, Point, "Total", total_concen, total_flags&1, t
//      DrawComponent(dc, r, Point, "Total", total_concen, total_flags&1, t  // for given point. P.P. 09/26/04
//	, 0, 0, 0, 0
	, TWA_runs[Point][num][0], total_concen_avg, total_concen_max
	, start_time[Point][num][0], total_concen_min // min added. P.P. 12/22/04
      );
*/
      double total_conc= total_point[Point][num];
      int total_flags= flags_point_total[Point][num]&1;
      long time=time_point[Point][num][0];
      int runs_total = TWA_runs_total[Point][num];
      double avg_total = TWA_total[Point][num];
      double max_total = max_TWA_total[Point][num];
      double min_total = min_TWA_total[Point][num];
//      DrawComponent(dc, r, Point, "Total", total_point[Point][num], flags_point_total[Point][num]&1, t
//	, TWA_runs_total[Point][num], TWA_total[Point][num], max_TWA_total[Point][num]
//	, start_time[Point][num][0], min_TWA_total[Point][num] // min added. P.P. 12/22/04
//      );
#if !QUICK_SCAN() // cond added. P.P. 12/14/10
      DrawComponent(dc, r, Point, "Total", total_conc, total_flags, t
	, runs_total, avg_total, max_total
	, time, min_total // min added. P.P. 12/22/04
      );
#endif // !quick_scan
#endif  // use_difference
  // end of removed code, and restored. P.P. 12/23/04
    }
#endif // USE_TOTAL()
  }

  if(DrawInfo.itemState&ODS_FOCUS)		// Reflect focus state
    dc.FrameRect( r, StockBrush(sbiBlack) );
}

void SummaryWindow::WMSetFocus(RTMessage)
{
  SetFocus(list->HWindow);
}

void SummaryWindow::WMSize(RTMessage Msg)
{
  Inherited::WMSize(Msg);
  if( (Msg.WParam != SIZEICONIC) && list) {
    WRect rClient;    ::GetClientRect(*this,&rClient);
    rClient.Top()+=HeaderHeight;
    ::MoveWindow(*list,rClient,TRUE);
  }
  WindowDC dc(HWindow);
  InitTabPositions(dc);
}

#include "frame.h"
void SummaryWindow::ActivationResponse(WORD Activated,  BOOL IsIconified)
{
  FrameWindow *pFrame=(FrameWindow *)Parent;
  pFrame->EnableCommand(CM_PRINT, Activated); // pJob
  Inherited::ActivationResponse(Activated, IsIconified);
}

void SummaryWindow::WMInitMenuPopup(RTMessage Msg)
{
  Menu menu((HMENU)Msg.WParam);
  menu.EnableItem(CM_PEAK_ADDITEM, MF_ENABLED);
  if(list && list->GetCount() && list->GetSelIndex()>=0) {
    menu.EnableItem(CM_PEAK_DELITEM, MF_ENABLED);
  }
  Job *pJob=(Job *)SendMessage(Parent->HWindow,WM_GET_JOB,0,0L);
  //Run *pRun=(Run *)SendMessage(Parent->HWindow,WM_GET_RUN,0,0L);
  //PeakList *pPeakList=((PeakList **)SendMessage(Parent->HWindow,WM_GET_PEAKLIST,0,0L))[num];
  menu.EnableItem(CM_PRINT, pJob? MF_ENABLED: MF_DISABLED);
  menu.EnableItem(CM_EDITCOPY, pJob /*&& pRun && pPeakList*/? MF_ENABLED: MF_DISABLED);
  menu.EnableItem(CM_EDITCOPYTOFILE, pJob /*&& pRun && pPeakList*/? MF_ENABLED: MF_DISABLED);
}


void SummaryWindow::Paint(HDC PaintDC, PAINTSTRUCT _FAR & PaintInfo)
{
  PC pc(PaintDC,&PaintInfo);
  WRect r;    ::GetClientRect(*this,&r);
  r.Bottom()=r.Top()+HeaderHeight;
  r.B()-=WPoint(1,1);
  {
    StockBrush brLightGray(sbiLightGray);
    pc.FillRect(r,brLightGray);
  }
  {
    StockPen penWhite(spiWhite);
    SelPen oldPen(pc,penWhite);
    pc.MoveTo(r.A()); pc.LineTo(r.Left(),r.Bottom());
    pc.MoveTo(r.A()); pc.LineTo(r.Right(),r.Top());
  }
  {
    StockPen penBlack(spiBlack);
    SelPen oldPen(pc,penBlack);
    pc.MoveTo(r.B()); pc.LineTo(r.Right(),r.Top());
    pc.MoveTo(r.B()); pc.LineTo(r.Left(),r.Bottom());
  }
  SelBkMode oldBkMode(pc,bkmTransparent);
  SelTextAlign oldTextAlign(pc,taLeft|taBottom);
  //static const char header[]="\tName\tConc\tHeight\tArea\tTime";
  //pc.TabbedTextOut(r.Left(),r.Bottom(), header, sizeof(header)-1, 5, TabPositions, r.Left());

  pc.TextOut(r.Left(),r.Bottom(),"Pt");
  pc.TextOut(r.Left()+TabPositions[0],r.Bottom(),"Name");
  pc.TextOut(r.Left()+TabPositions[1],r.Bottom(),"Conc");
  pc.SetTextAlign(taRight|taBottom);
/*
  pc.TextOut(r.Left()+TabPositions[2],r.Bottom(),"RTime");
  pc.TextOut(r.Left()+TabPositions[3],r.Bottom(),"run");
  pc.TextOut(r.Left()+TabPositions[4],r.Bottom(),"TWA");
  pc.TextOut(r.Left()+TabPositions[5],r.Bottom(),"Max"); //
  pc.TextOut(r.Left()+TabPositions[6],r.Bottom(),"time");
*/
// new layout. P.P. 12/22/04
  pc.TextOut(r.Left()+TabPositions[2],r.Bottom(),"run");
  pc.TextOut(r.Left()+TabPositions[3],r.Bottom(),"Min");
  pc.TextOut(r.Left()+TabPositions[4],r.Bottom(),"Avg"); // P.P. 12/22/04
  pc.TextOut(r.Left()+TabPositions[5],r.Bottom(),"Max"); //
  pc.TextOut(r.Left()+TabPositions[6],r.Bottom(),"time");
// end of new layout. P.P. 12/22/04
}


void SummaryWindow::CMPrint(RTMessage)
{
  PeakApp *pApp=((PeakApp *)GetApplication());
  Printer *pPrinter=pApp;
  Job *pJob=(Job *)SendMessage(Parent->HWindow,WM_GET_JOB,0,0L);
  if( pPrinter&&pJob ) {
    SummaryPrintout *pPrintout = new SummaryPrintout(pApp->Name, pJob, num);
    if( pPrintout ) {
      pPrinter->Print(this, pPrintout, TRUE);
      delete pPrintout;
    }
  }
}


extern const unsigned CF_RTF;//=RegisterClipboardFormat("Rich Text Format");

#include "wcpplib/fwstream.h"
#include "wcpplib/memwstrm.h"
static const char far rowheader[]=
//                   n         Name       Conc       Time         run        TWA       max TWA          beg
//"\\trowd\\trgaph100\\cellx500\\cellx3236\\cellx4100\\cellx5108\\cellx5842\\cellx6994\\cellx7728\\cellx8639\\intbl \r\n"; // not enough fo 100%
"\\trowd\\trgaph100\\cellx500\\cellx3236\\cellx4100\\cellx5108\\cellx5942\\cellx7094\\cellx7928\\cellx8839\\intbl \r\n";

#define TABS() 0

#if !TABS()
static const char _far manyspaces[]="                                ";
#endif


static void PrintComponentToFile(owstream &os, UINT cf, int Point, const char *component_name, double concen, int flags, long t
//    , int TWA_runs, float TWA_results, float max_TWA_conc, time_t time
    , int TWA_runs, float TWA_results, float max_TWA_conc, time_t time, float min_TWA_conc // min added. P.P. 12/24/04
)
{
  char str[120];
  if(cf==CF_TEXT) {
    if(Point>=0) {
    #if TABS()
    wsprintf(str,"%3d\t%s",1+Point,component_name);
    #else
    wsprintf(str,"%3d %-25s",1+Point,component_name);
    #endif
    }
    else {
    #if TABS()
    wsprintf(str,"   \t%s",component_name);
    #else
    wsprintf(str,"    %-25s",component_name);
    #endif
    }
  }
  else {
    os.write(rowheader,sizeof(rowheader)-1);
    if(Point>=0)
      wsprintf(str,"\\qr %d\\cell ",1+Point);
    else
      wsprintf(str,"\\qr \\cell ");

    os.write(str,strlen(str));
    wsprintf(str,"\\ql %s\\cell\\qr ",component_name);
  }
  os.write(str,strlen(str));
  {
    if(flags&1) {
      sprintf(str, "%.3f", concen);
    }
    else {
      lstrcpy(str,"<0.001");   // changed. P.P. 03/26/03
    }
    if(cf==CF_TEXT) {
      #if TABS()
      os.write("\t",1);
      #else
      if(strlen(str)<10) os.write(manyspaces,10-strlen(str));
      #endif
    }
    else {
      os.write(str,strlen(str));
      wsprintf(str,"\\cell ");
    }
    os.write(str,strlen(str));
  }
  {
    strcpy( str, "  "  );
    if(flags&2) strcpy( str, " H"  );
    if(flags&4) strcpy( str, " L" );
    os.write(str,strlen(str));
  }
/* // Ret time removed from Shift Report. P.P. 12/24/04
  if(cf==CF_TEXT) {
    #if TABS()
    if(t>0) wsprintf(str,"\t%ld:%.2ld", t/60, t%60);
    else lstrcpy(str,"\t     ");
    #else
    if(t>0) wsprintf(str,"%ld:%.2ld", t/60, t%60);
    else str[0]=0;
    if(strlen(str)<10) os.write(manyspaces,10-strlen(str));
    #endif
  }
  else  {
    if(t>0)
      wsprintf(str,"\\qr %ld:%.2ld\\cell ", t/60, t%60);
    else
      wsprintf(str,"\\qr \\cell ");
  }
  os.write(str,strlen(str));
*/
  if(cf==CF_TEXT) {
    #if TABS()
    wsprintf(str,"\t%d",TWA_runs);
    #else
    wsprintf(str,"%13d",TWA_runs);
    #endif
    //if(strlen(str)<13) os.write(manyspaces,13-strlen(str));
  }
  else {
    wsprintf(str,"%d\\cell ",TWA_runs);
  }
  os.write(str,strlen(str));
// min_TWA_conc added to Shift Report. P.P. 12/24/04
  if(cf==CF_TEXT) {
    #if TABS()
    sprintf(str,"\t%.3f",min_TWA_conc);
    #else
    sprintf(str,"%12.3f",min_TWA_conc);
    if(strlen(str)<13) os.write(manyspaces,13-strlen(str));
    #endif
  }
  else {
    sprintf(str,"%.3f\\cell ",min_TWA_conc);
  }
  os.write(str,strlen(str));
// end of added min_TWA_conc. P.P. 12/24/04
  if(cf==CF_TEXT) {
    #if TABS()
    sprintf(str,"\t%.3f",TWA_results);
    #else
    sprintf(str,"%12.3f",TWA_results);
    if(strlen(str)<13) os.write(manyspaces,13-strlen(str));
    #endif
  }
  else {
    sprintf(str,"%.3f\\cell ",TWA_results);
  }
  os.write(str,strlen(str));
  if(cf==CF_TEXT) {
    #if TABS()
    sprintf(str,"\t%.3f",max_TWA_conc);
    #else
    sprintf(str,"%12.3f",max_TWA_conc);
    if(strlen(str)<13) os.write(manyspaces,13-strlen(str));
    #endif
  }
  else {
    sprintf(str,"%.3f\\cell ",max_TWA_conc);
  }
  os.write(str,strlen(str));
  if(cf==CF_TEXT) {
    #if TABS()
    sprintf(str,"\t%s",PrintTime(time, TRUE));
    #else
    sprintf(str,"%s",PrintTime(time, TRUE));
    if(strlen(str)<6+1) os.write(manyspaces,6+1-strlen(str));
    #endif
  }
  else {
    sprintf(str,"%s\\cell",PrintTime(time, TRUE));
  }
  os.write(str,strlen(str));

  if(cf==CF_TEXT) {
    wsprintf(str,"\r\n");
  }
  else  {
    wsprintf(str,"\\row \r\n");
  }
  os.write(str,strlen(str));
}

static void PrintComponentsToFile(owstream &os,UINT cf, Method *pMethod, Detector *pDetector, int num)
{
  if(cf==CF_TEXT) {
    static const char far header[]=
    #if TABS()
//    " Pt\tName   \tConc  \tRTime\trun\tTWA  \tmax  \ttime\r\n";
    " Pt\tName   \tConc  \trun\tMin\tAvg  \tmax  \ttime\r\n";  // replacement. P.P. 12/24/04
    #else
//    " Pt Name                           Conc       RTime          run          TWA          max   time\r\n";
    " Pt Name                           Conc         run          Min          Avg          Max   time\r\n";  // P.P. 12/24/04
    #endif
    os.write(header,sizeof(header)-1);
  }
  else {
    os.write(rowheader,sizeof(rowheader)-1);
    static const char far headerrow[]=
//    "\\qrPt\\cell\\ql Name\\cell\\qr Conc\\cell RTime\\cell run\\cell TWA\\cell max\\cell time\\cell\\row \r\n";
    "\\qrPt\\cell\\ql Name\\cell\\qr Conc\\cell run\\cell Min\\cell Avg\\cell Max\\cell time\\cell\\row \r\n";  // P.P. 12/24/04
    os.write(headerrow,sizeof(headerrow)-1);
  }
  {
    uint32 points=pMethod->method_table.points? pMethod->method_table.points: 1;
    //long mul=pow10( pRun->runrawdata.gain[num] );
/* // rem. P.P. 12/26/04
#if USE_TOTAL()
    int total_flags=0;
    double total_concen=0;
    double total_concen_min=0; // added. P.P. 12/23/04
    double total_concen_avg=0; // added. P.P. 12/23/04
    double total_concen_max=0; // added. P.P. 12/23/04
#endif // USE_TOTAL()
*/
    for(int Point=0; Point<POINTS(); Point++) { // order changed. P.P. 12/26/04
      if( !((1L<<Point) & points) ) continue;
      for(int i=0; i<COMPONENT_ROWS; i++) {
        Component *pComponent=&pDetector->components[i];
        if(!pComponent->active_yn) continue;

        int iComponent=i;
//      for(int Point=0; Point<POINTS(); Point++) { // moved up. P.P. 12/26/04
//	if( !((1L<<Point) & points) ) continue;

	const char *component_name=pComponent->name.Get();
	double concen=concen_point[Point][num][iComponent];
	int flags=flags_point[Point][num][iComponent];
	long time=time_point[Point][num][iComponent];
/* rem. P.P. 12/26/04
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
*/
	PrintComponentToFile(os,cf, Point, component_name, concen, flags, time
	  , TWA_runs[Point][num][iComponent], TWA_results[Point][num][iComponent], max_TWA_conc[Point][num][iComponent]
//	  , start_time[Point][num][iComponent]
	  , start_time[Point][num][iComponent], min_TWA_conc[Point][num][iComponent]
	);
      }
#if USE_TOTAL()
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
// end of added code. P.P. 12/26/04
//        for(int Point=0; Point<POINTS(); Point++) {  // added here. P.P. 12/24/04
//	  if( !((1L<<Point) & points) ) continue;
#if !QUICK_SCAN() // cond added. P.P. 12/14/10
#if USE_DIFFERENCE() // case added. P.P. 09/26/04 // should be changed, as total! P.P. 12/26/04
//          PrintComponentToFile(os,cf, -1, "Chlorine by diff", 100.0-total_concen, total_flags&1, t
//	    , 0,0,0,0);
   // changed but not finished _ what to be shown? P.P. 12/24/04
	  PrintComponentToFile(os,cf, Point, "Chlorine by diff", 100.0-total_concen, total_flags&1, t
	    , 0,0,0,start_time[Point][num][0],0);
#else
//          PrintComponentToFile(os,cf, -1, "Total", total_concen, total_flags&1, t
//	    , 0,0,0,0);
//	  PrintComponentToFile(os,cf, Point, "Total", total_concen, total_flags&1, t
//	    , TWA_runs[Point][num][0],total_concen_avg,total_concen_max,start_time[Point][num][0],total_concen_min);
	  PrintComponentToFile(os,cf, Point, "Total", total_conc, total_flags, t
	    , runs_total,avg_total,max_total, time, min_total);

#endif // use_difference
#endif // !quick_scan
//      } // added. P.P. 12/24/04
      }
    #endif // USE_TOTAL()

    } // for points....
/* moved up. P.P. 12/26/04
#if USE_TOTAL()
    {
      long t=-1;//pPeak->Time/data_rate;
      for(int Point=0; Point<POINTS(); Point++) {  // added here. P.P. 12/24/04
	if( !((1L<<Point) & points) ) continue;
#if USE_DIFFERENCE() // case added. P.P. 09/26/04
//        PrintComponentToFile(os,cf, -1, "Chlorine by diff", 100.0-total_concen, total_flags&1, t
//	  , 0,0,0,0);
   // changed but not finished _ what to be shown? P.P. 12/24/04
	PrintComponentToFile(os,cf, Point, "Chlorine by diff", 100.0-total_concen, total_flags&1, t
	  , 0,0,0,start_time[Point][num][0],0);
#else
//        PrintComponentToFile(os,cf, -1, "Total", total_concen, total_flags&1, t
//	  , 0,0,0,0);
	PrintComponentToFile(os,cf, Point, "Total", total_concen, total_flags&1, t
	  , TWA_runs[Point][num][0],total_concen_avg,total_concen_max,start_time[Point][num][0],total_concen_min);
#endif // use_difference
      } // added. P.P. 12/24/04
    }
    #endif // USE_TOTAL()
*/
  }
}

void writertfstring(owstream &os,const char *str);

long CreateFile(Job *pJob, int num, BOOL bAppend, UINT cf, const char *file)
{
  long len=0;
  ofwstream os(file, bAppend? 0: 1);
  if(os.good()) {
    Thinking think("Writting file: '%s'...", file);
    os.seek(0, wstream::end);
    time_t tim;
    time( &tim );
    Str buff(58);
    wsprintf(buff.GetPtr(), "%s %s",  PrintDate(tim), PrintTime(tim));

    if(cf!=CF_TEXT) {
      static const char _far header[]="{\\rtf1\r\n"
//Write do not like headers	  "{\\header\\pard\\plain\\qc GC501c Gas Chromatograph, \\'a9 1994\\endash 2003, Process Analyzers, LLC\\par}\r\n"
#if GC501() // cond added. P.P. 12/24/04
      "{\\pard\\plain\\qc GC501B Gas Chromatograph, \\'a9 1994\\endash 2005, PID Analyzers, LLC\\par}\r\n"
#else
      "{\\pard\\plain\\qc GC301B Gas Chromatograph, \\'a9 1994\\endash 2005, PID Analyzers, LLC\\par}\r\n"
#endif
      "\\pard\\plain\\fs20{\\qc\\fs24\\sa360 {\\b Shift Report} "
      ;
      // font name : default
      // font size : 20pt/2 (i.e. 10pt)
      // after par 360/20 pt?
      os.write(header,sizeof(header)-1);

      writertfstring(os,buff);

      static const char header2[]="\\par}\r\n";
      os.write(header2,sizeof(header2)-1);
    }
    else {
#if GC501() // cond added. P.P. 12/24/04
      static const char _far header[]="GC501B Gas Chromatograph, (c) 1994-2005, PID Analyzers, LLC\r\n"  // P.P. 06/06/04
#else
//      static const char _far header[]="GC301B Gas Chromatograph, (c) 1994-2005, PID Analyzers, LLC\r\n"  // P.P. 06/06/04
      static const char _far header[]="GC301C Gas Chromatograph, (c) 1994-2010, PID Analyzers, LLC\r\n"  // P.P. 12/14/10
#endif
      "Shift Report ";
      os.write(header,sizeof(header)-1);
      os.write(buff.Get(),buff.Length());
      static const char header2[]="\r\n";
      os.write(header2,sizeof(header2)-1);
    }

    if(pJob /*&& pRun && pPeakList*/) {
      Method *pMethod=pJob;
      PrintComponentsToFile(os,cf, pMethod, &pMethod->det[num],  num);
    }
    if(cf!=CF_TEXT) {
      os.write("}",1);
    }
    len=os.tell();
  }
  return len;
}
long SummaryWindow::CreateFile(UINT cf, const char *file)
{
  long len=0;
  Job *pJob=(Job *)SendMessage(Parent->HWindow,WM_GET_JOB,0,0L);
  //Run *pRun=(Run *)SendMessage(Parent->HWindow,WM_GET_RUN,0,0L);
  //PeakList *pPeakList=((PeakList **)SendMessage(Parent->HWindow,WM_GET_PEAKLIST,0,0L))[num];
  if(pJob /*&& pRun && pPeakList*/) {
    len=::CreateFile(pJob, num, FALSE, cf, file);
  }
  return len;
}

void SummaryWindow::CMEditCopy(RTMessage)
{
  if(OpenClipboard(HWindow)) {
    EmptyClipboard();
    Str tmp_file(255);
    GetTempFileName(GetTempDrive(0), "tmp", 0, tmp_file.GetPtr());
    for(int nformat=0; nformat<2; ++nformat) {
      UINT cf=nformat==0? CF_TEXT: CF_RTF;
      long len=CreateFile(cf, tmp_file);
      HGLOBAL h=GlobalAlloc(GHND|GMEM_SHARE, len+1);
      if(h) {
	char *cp=(char *)GlobalLock(h);
	ifwstream is(tmp_file);
	if(is.good()) {
	  is.read(cp,len);
	}
	cp[len]='\0';
	GlobalUnlock(h);
	SetClipboardData(cf,h);
      }
    }
    AnsiToOem(tmp_file.GetPtr(),tmp_file.GetPtr());
    unlink(tmp_file);
    CloseClipboard();
  }
}

extern void LoadFilterString(HINSTANCE hinst, UINT idResource, LPSTR lpszBuffer, int cbBuffer);
#include "mainwind.h"
void SummaryWindow::CMEditCopyToFile(RTMessage)
{
  //Run *pRun=(Run *)SendMessage(Parent->HWindow,WM_GET_RUN,0,0L);
  //if(!pRun) return;

  HINSTANCE hInstance=GetApplication()->hInstance;
  Str strFilter(128);
  LoadFilterString(hInstance, IDS_RTF_TXT_FILTER, strFilter.GetPtr(), 128);
  Str strTitle(64);
  LoadString(hInstance, IDS_SAVEPEAKLIST, strTitle.GetPtr(), 64);
  Str FileName;

  static int nIndex=0;
  BOOL ret=((MainWindow *)Parent)->OpenSaveDialog(HWindow, strTitle, strFilter, &FileName, TRUE, &nIndex);
  if(ret) {
    UINT cf=nIndex==1? CF_TEXT: CF_RTF;
    CreateFile(cf, FileName);
  }
}


void SummaryWindow::WMCommand(RTMessage Msg)
{
  switch(Msg.LP.Hi) {
    case LBN_SELCHANGE:
    break;
  }
  Inherited::WMCommand(Msg);
}

void SummaryWindow::CMAddItem(RTMessage)
{
  if(!list) return;
}

void SummaryWindow::CMDeleteItem(RTMessage)
{
  if(!list) return;
  int Index=list->GetSelIndex();
  if(Index<0) return;
  MessageBeep(MB_ICONHAND);
  if(IDOK==MessageBox(NULL,"Are you realy want to delete this peak?", "Warning", MB_OKCANCEL|MB_ICONHAND|MB_TASKMODAL)) {
    remove(Index);
  }
}

void SummaryWindow::HandleList(RTMessage Msg)
{
  switch(Msg.LP.Hi) {
    case LBN_DBLCLK:
//      int Index=list->GetSelIndex();
//      GetModule()->MakeWindow(newDIBWindow(Parent,(LPSTR)pPeakList->GetChild(Index)->Get()));
    break;
  }
}




#ifndef  _SUMM_WND_H
#include "summ_wnd.h"
#endif

PTWindowsObject newSummaryWindow(PTWindowsObject AParent, int num, char far *ATitle, PTModule AModule)
{
  return new SummaryWindow(AParent, num, ATitle,AModule);
}



