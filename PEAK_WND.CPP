#include "includes.h"
#pragma hdrstop

#ifndef __WINDOW_H
#include <window.h>
#endif
#ifndef __LISTBOX_H
#include <listbox.h>
#endif

#include <math.h>

#ifndef  _WINLIB_DC_H
#include "winlib\dc.h"
#endif
#ifndef  _WINLIB_PC_H
#include "winlib\pc.h"
#endif
#ifndef  _WINLIB_PEN_H
#include "winlib\pen.h"
#endif
#ifndef  _WINLIB_SELPEN_H
#include "winlib\selpen.h"
#endif
#ifndef  _WINLIB_MENU_H
#include "winlib\menu.h"
#endif
#ifndef  _WINUTIL_WUTIL_H
#include "winutil\wutil.h"
#endif


#ifndef  _ID_H
#include "id.h"
#endif
#ifndef  _PEAKLIST_H
#include "peaklist.h"
#endif
#ifndef  _CMPONENT_H
#include "cmponent.h"
#endif

#include "method.h"
#include "job.h"


class PeakListBox : public TListBox {
  typedef TListBox Inherited;
 public:
  PeakListBox(PTWindowsObject AParent, int AnId, int X, int Y, int W, int H, PTModule AModule = NULL)
    : TListBox(AParent, AnId, X, Y, W, H, AModule ) {}
  PeakListBox(PTWindowsObject AParent, int ResourceId, PTModule AModule = NULL)
    : TListBox(AParent, ResourceId, AModule ) {}
  virtual void WMKeyDown(RTMessage Msg) = [WM_FIRST + WM_KEYDOWN];
};

void PeakListBox::WMKeyDown(RTMessage Msg)
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

#include "internal.h" // USE_TOTAL()
class PeakWindow : public TWindow {
  typedef TWindow Inherited;
  PTListBox list;
  int HeaderHeight;
  int num;
  virtual LPSTR GetClassName();
  virtual void GetWindowClass(WNDCLASS &AWndClass);
  int TabPositions[7];
  void InitTabPositions();
  double H, A;
 public:
  PeakWindow(PTWindowsObject AParent,int num,char far *ATitle,PTModule AModule = NULL);
  ~PeakWindow();
 protected:
  virtual void SetupWindow();
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

  PeakWindow(StreamableInit);
  virtual void write (Ropstream os);
  virtual Pvoid read (Ripstream is);
  virtual const Pchar streamableName() const { return name; }
 public:
  static PTStreamable build();
  static char far name[];

//  void DrawPeak(DC &dc, const WRect &r, const char *name, int Num, double concen, int flags, long t, long Height, long Area, double H, double A, long mul);
  void DrawPeak(DC &dc, const WRect &r, const char *name, int Num, double concen, int flags, long t, long Height, double Area, double H, double A, long mul); // area is double now. P.P. 08/25/17
  #if USE_TOTAL()
  int total_flags;
  double total_concen;
  long total_H;
//  long total_A;
  double total_A; // double now. P.P. 08/25/17
  #endif // USE_TOTAL()
};

LPSTR PeakWindow::GetClassName()
{
  return name;
}

void PeakWindow::GetWindowClass(WNDCLASS& WndClass)
{
  Inherited::GetWindowClass(WndClass);
  WndClass.hIcon = LoadIcon(GetModule()->hInstance,"LIST");
}


PeakWindow::PeakWindow(PTWindowsObject AParent,int _num,char far *ATitle,PTModule AModule)
  : TWindow(AParent,ATitle,AModule),
    list(new PeakListBox(this, IDD_PEAKLIST, 0, 0, 0, 0, AModule)),
    HeaderHeight(20),
    num(_num)
  , A(0), H(0)
  #if USE_TOTAL()
  , total_flags(0)
  , total_concen(0)
  , total_H(0), total_A(0)
  #endif // USE_TOTAL()
{
  Attr.Style |= WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE;
  if(list) {
    list->Attr.Style&=~(WS_BORDER|LBS_SORT);
    list->Attr.Style|=LBS_NOTIFY|LBS_OWNERDRAWFIXED|/*LBS_HASSTRINGS|*/LBS_NOINTEGRALHEIGHT|WS_VSCROLL|WS_CHILD|WS_VISIBLE|WS_GROUP|WS_TABSTOP;
  }

}

PeakWindow::~PeakWindow(void)
{
}

void PeakWindow::SetupWindow()
{
  Inherited::SetupWindow();
  FillList();
}

PeakWindow::PeakWindow(StreamableInit)
 : TWindow(streamableInit), list(0)
{
};

TStreamableClass RegPeakWindow(PeakWindow::name, PeakWindow::build, __DELTA(PeakWindow));
char far PeakWindow::name[]="PeakWindow";
PTStreamable PeakWindow::build()
{
  return new PeakWindow(streamableInit);
}

void PeakWindow::write(Ropstream os)
{
  Inherited::write(os);
  os<<HeaderHeight;
  os<<num;
  PutChildPtr(os,list);
}

Pvoid PeakWindow::read(Ripstream is)
{
  if(!Inherited::read(is)) return NULL;
  is>>HeaderHeight;
  is>>num;
  GetChildPtr(is,(RPTWindowsObject)list);
  return this;
}


void PeakWindow::InitTabPositions()
{
  WindowDC dc(HWindow);
  TabPositions[0]=dc.GetTextExtent("99 ").X();
  TabPositions[1]=TabPositions[0]; // variable size defined later
//  TabPositions[2]=TabPositions[1]+dc.GetTextExtent("12345.123  123456789").X();
  TabPositions[2]=TabPositions[1]+dc.GetTextExtent("12345.12356789  123456789").X(); // increased. P.P. 08/01/17
  TabPositions[3]=TabPositions[2]+dc.GetTextExtent("  99.99%").X();
  TabPositions[4]=TabPositions[3]+dc.GetTextExtent("    123456789").X();  // increased by 2 digit places. P.P. 06/30/17
  TabPositions[5]=TabPositions[4]+dc.GetTextExtent("  99.99%").X();
  TabPositions[6]=TabPositions[5]+dc.GetTextExtent("  1234:00").X();
  WRect rClient; ::GetClientRect(*this,&rClient);
  int delta=rClient.Width()-(TabPositions[6]+dc.GetTextExtent("1").X());
  int wName=dc.GetTextExtent("Name ").X();
  if(delta<wName) delta=wName;
  for(int i=1; i<7; ++i) {
    TabPositions[i]+=delta;
  }
}

void PeakWindow::FillList()
{
  if(!list) return;
  PeakList *pPeakList=((PeakList **)SendMessage(Parent->HWindow,WM_GET_PEAKLIST,0,0L))[num];
  A=0; H=0;
  ::SendMessage(list->HWindow,WM_SETREDRAW,FALSE,0L);
  list->ClearList();
#if USE_TOTAL()
  Job *pJob=(Job *)SendMessage(Parent->HWindow,WM_GET_JOB,0,0L);
  Run *pRun=(Run *)SendMessage(Parent->HWindow,WM_GET_RUN,0,0L);
  total_flags=0;
  total_concen=0;
  total_H=0; total_A=0;
#endif // USE_TOTAL()
  if(pPeakList) {
    for(PeakList::Node *pNode=pPeakList->first(); pNode; pNode=pNode->nextNode()) {
//      Peak *pPeak=pNode;      // added. P.P. 02/27/05   // rem temp 12/30/06
#if NOISE_REDUCT() // added. P.P. 02/27/05
//      if(pPeak->pComponent) { // known peak. Cond added. P.P. 02/27/05
#if !USE_TOTAL() // code below changed, for version 24. P.P. 09/18/05
  Job *pJob=(Job *)SendMessage(Parent->HWindow,WM_GET_JOB,0,0L);
#endif
      Method *pMethod=pJob;
      Peak *pPeak=pNode;      // added. P.P. 02/27/05   // moved here 12/30/06
      if(pMethod->det[num].known_peaks) {
        if(pPeak->pComponent) { // known peak only. P.P. 09/17/05
	  if(pNode->Height>0) {  // cond added in all places below to exclude neg peaks/areas from calculations. P.P. 12/31/06
	    H+=pNode->Height;
	  }
	  if(pNode->Area>0) {    // cond added. P.P. 12/31/06
	    A+=pNode->Area;
          }
          list->AddString((LPSTR)(Peak *)pNode);
	}
      }
      else { // all peaks button clicked
	if(pNode->Height>0) {    // cond added. P.P. 12/31/06
	  H+=pNode->Height;
        }
	if(pNode->Area>0) {      // cond added. P.P. 12/31/06
	  A+=pNode->Area;
        }
        list->AddString((LPSTR)(Peak *)pNode);
      }

#else // noise_reduct = 0 - add anyway
      if(pNode->Height>0) {      // cond added. P.P. 12/31/06
	H+=pNode->Height;
      }
      if(pNode->Area>0) {        // cond added. P.P. 12/31/06
	A+=pNode->Area;
      }
      list->AddString((LPSTR)(Peak *)pNode);
#endif // noise_reduct
#if USE_TOTAL()
#if !NOISE_REDUCT() // added. Otherwise, it was defined above.P.P. 09/18/05
      Method *pMethod=pJob;
#endif
//      Peak *pPeak=pNode; // moved up. P.P. 02/27/05  
      double concen;
      int flags=pMethod->det[num].Concentration(pMethod, pRun, pPeak, num, &concen);
      total_flags|=flags;
#if AREA_ALL() // case added. All the peak areas and heights added. P,P, 12/21/11 
      if(flags&1) {
	if(concen>0) {           //
	  total_concen+=concen;
        }
      }
      if(pPeak->Height>0) {    // cond added. P.P. 12/31/06
	total_H+=pPeak->Height;
      }
      if(pPeak->Area>0) {      // cond added. P.P. 12/31/06
	total_A+=pPeak->Area;
      }
#else  // as it was always
      if(flags&1) {
	if(concen>0) {           // cond added. P.P. 12/31/06
	  total_concen+=concen;
        }
	if(pPeak->Height>0) {    // cond added. P.P. 12/31/06
	  total_H+=pPeak->Height;
        }
	if(pPeak->Area>0) {      // cond added. P.P. 12/31/06
	  total_A+=pPeak->Area;
        }
      }
#endif // area_all
#endif // USE_TOTAL()
    }
#if !QUICK_SCAN() // cond added. P.P. 01/13/11  rem 12/19/11
#if USE_TOTAL()
    list->AddString((LPSTR)(Peak *)0);
#endif // USE_TOTAL()
#endif // quick scan  //  rem. P.P. 12/19/11
    list->SetSelIndex(0);
  }
  ::SendMessage(list->HWindow,WM_SETREDRAW,TRUE,0L);
//  ::InvalidateRect(list->HWindow,NULL,FALSE);
  ::InvalidateRect(list->HWindow,NULL,TRUE);   // replacement, to delete previously written lines. P.P. 12/31/06
}

void PeakWindow::WMNewPeak(RTMessage Msg)
{
  if(!list) return;
  if(!Msg.WParam==num) return;

  Peak *pPeak=(Peak *)Msg.LParam;
#if NOISE_REDUCT()  // P.P. 09/18/05
//  if(pPeak->pComponent) { // only known peaks
  Job *pJob=(Job *)SendMessage(Parent->HWindow,WM_GET_JOB,0,0L); // P.P. 09/18/05
  Method *pMethod=pJob;                                          //
  if(pMethod->det[num].known_peaks) {
    if(pPeak->pComponent) { // known peak only. P.P. 09/17/05
      list->AddString((LPSTR)pPeak);
    }
  }
  else {
    list->AddString((LPSTR)pPeak);
  }
#else // noise_reduct=0
  list->AddString((LPSTR)pPeak);
#endif
}

void PeakWindow::WMIsItYourName(RTMessage Msg)
{
  Msg.Result=lstrcmpi(GetClassName(),(LPCSTR)Msg.LParam)==0 && Msg.WParam==num;
}

void PeakWindow::WMRefresh(RTMessage Msg)
{
  int bGraphOnly=Msg.WParam;
  if(!bGraphOnly) FillList();
}

void PeakWindow::WMMeasureItem(TMessage& Msg)
{
  MEASUREITEMSTRUCT FAR *lpmisCtl = (MEASUREITEMSTRUCT FAR*) Msg.LParam;
  //lpmisCtl->itemWidth=r.Width()/((nPict+nRow/2)/nRow);
  TEXTMETRIC tm;
  ClientDC(HWindow).GetTextMetrics(&tm);

  lpmisCtl->itemHeight=tm.tmHeight+tm.tmExternalLeading;
  HeaderHeight=lpmisCtl->itemHeight+2;
}

extern char data_rate;           // rate data received in pts/sec.

#include <stdio.h> // sprintf("%f")

#include "utl.h"
#include "hnu.h" // added for JOB, CAL, CHECKCAL. P.P. 07/26/04


//void PeakWindow::DrawPeak(DC &dc, const WRect &r, const char *component_name, int Num, double concen, int flags, long t, long Height, long Area, double H, double A, long mul)
void PeakWindow::DrawPeak(DC &dc, const WRect &r, const char *component_name, int Num, double concen, int flags, long t, long Height, double Area, double H, double A, long mul) // area is double now. P.P. 08/25/17
{
  SelTextAlign oldTextAlign(dc,taLeft|taTop);
  char str[50];
//  if(Num>0) { wsprintf(str,"%d",Num); dc.TextOut(r.Left(),r.Top(),str); } // replaced. P.P. 12/29/06
  if(Num>0) {  // removing negative peaks from table. P.P. 12/29/06  rem P.P. 12/19/11
    wsprintf(str,"%d",Num);
    dc.TextOut(r.Left(),r.Top(),str); 
  } // moved here to exclude only num for total, leaving the rest. P.P. 12/19/11
    {
      WRect _rc=r;
      _rc.Left()=r.Left()+TabPositions[0];
      _rc.Right()=r.Left()+TabPositions[1];
      dc.ExtTextOut(_rc.A(), ETO_CLIPPED, _rc, component_name, lstrlen(component_name), (LPINT)NULL);
    }
    Job *pJob=(Job *)SendMessage(Parent->HWindow,WM_GET_JOB,0,0L);
    Run *pRun=(Run *)SendMessage(Parent->HWindow,WM_GET_RUN,0,0L);
    Method *pMethod = pJob; // added. P.P. 08/01/17
    if(pJob && pRun) {
      if(flags&1) {
//        sprintf(str, "%.3f", concen);
//        sprintf(str, "%.3f ", concen);
//	strcat(str, pMethod->det[num].units);   // P.P. 07/29/17 increase resolution 08/01/17 units added.
// P.P. 08/12/17, new code, from meth_fun of 08/11/17
        if(pMethod->det[num].units == "%") {
          if(concen <0.001) {                        //<10^-8, then ppb. P.P. 08/04/17
//	    sprintf(str, "%.0f ", concen*10000000.0);   // P.P. 08/04/17 
	    sprintf(str, "%.1f ", concen*10000000.0);   // P.P. 12/25/17
	    strcat(str, "ppb");                         //
          }
          else if(concen <1.0) {                        //<1%, then ppm. P.P. 08/13/17
            if(concen < 0.01) {
	    sprintf(str, "%.1f ", concen*10000.0);   // P.P. 08/13/17
	  }
          else {
	    sprintf(str, "%.0f ", concen*10000.0);   // P.P. 08/04/17
          }
	  strcat(str, "ppm");                         //
        }
        else {
	  sprintf(str, "%.2f ", concen);   // P.P. 08/04/17 2 digit after decimal. 
	  strcat(str, pMethod->det[num].units);   //  08/01/17 units added.
        }
      }
      else if((pMethod->det[num].units == "ppb")||(pMethod->det[num].units == "PPB")) {
        if(concen >=10000000) {                        //>10^7 in ppb, then show in %. P.P. 08/11/17
	  sprintf(str, "%.2f ", concen/100000.0);   //  
	  strcat(str, "%");                         // 
        }
        else if(concen >=10000) {                        //>10^4 in ppb, then show in ppm. P.P. 08/11/17
          if(concen >= 100000) {                        // >=100 ppm - show whole numbers
	    sprintf(str, "%.0f ", concen/1000);   //
	  }
          else {                                        // < 100 ppm - show 1 place after decimal dot. P.P. 08/13/17
	    sprintf(str, "%.1f ", concen/1000);   //  
	  }
	  strcat(str, "ppm");                         //
        }
        else {
//	  sprintf(str, "%.0f ", concen);   // P.P. 08/11/17 whole numbers, in ppb. 
	  sprintf(str, "%.1f ", concen);   // P.P. 12/25/17 1 dec place after dot, in ppb
	  strcat(str, pMethod->det[num].units);   //  ppb units added.
        }
      }
      else if((pMethod->det[num].units == "ppm")||(pMethod->det[num].units == "PPM")) {
        if(concen >=10000) {                        //if >=10^4  ppm, then show in %, 2 digits after decimal. P.P. 08/11/17
	  sprintf(str, "%.2f ", concen/10000.0);   //  
	  strcat(str, "%");                         // 
        }
        else if(concen <10.0) {                        //if <10 ppm, then show in ppb, whole numbers. P.P. 08/11/17
//	  sprintf(str, "%.0f ", concen*1000.0);   //  
	  sprintf(str, "%.1f ", concen*1000.0);   //  // show 1 dec place after dot, in ppb. P.P. 12/25/17
	  strcat(str, "ppb");                         //
        }
        else {
	  if(concen >=100) {
	    sprintf(str, "%.0f ", concen);   // P.P. 08/13/17  if >= 100 ppm, show in whole numbers.
	  }
          else {
	    sprintf(str, "%.1f ", concen);   // P.P. 08/11/17  if < 100 ppm, show with 1 digit after decimal.
	  }
	  strcat(str, pMethod->det[num].units);   //  ppm units added.
        }
      }
      else {
        sprintf(str, "%.6f ", concen);   // P.P. 07/29/17 increase resolution
        strcat(str, pMethod->det[num].units);   // P.P. 07/29/17 increase resolution 08/01/17 units added.
      }
// end of new code 08/11/17
      if(pRun->type_of_run==JOB) { // cond. added. P.P. 07/26/04
        if(flags&2) strcat( str, " H" );
	if(flags&4) strcat( str, " L" );
      }
      dc.TextOut(r.Left()+TabPositions[1],r.Top(),str);
    }
  }
  dc.SetTextAlign(taRight|taTop);
// height and area removed for total. P.P. 01/08/03
//    if(t>0) { // added. P.P. 01/08/03  // cond removed to show total height and area. P.P. 12/19/11
  wsprintf(str,"%ld",Height*mul); dc.TextOut(r.Left()+TabPositions[2],r.Top(),str);
  sprintf(str,"%.2f",H? Height*100./H: 0.); dc.TextOut(r.Left()+TabPositions[3],r.Top(),str);
  sprintf(str,"%.0f",(float) Area*mul); dc.TextOut(r.Left()+TabPositions[4],r.Top(),str);     //  area is double - show as float, and use sprintf instead of wsprintf. P.P. 08/27/17
//    wsprintf(str,"%ld",Area*mul); dc.TextOut(r.Left()+TabPositions[4],r.Top(),str);
  sprintf(str,"%.2f",A? Area*100./A: 0.); dc.TextOut(r.Left()+TabPositions[5],r.Top(),str);
//    }  // end. P.P. 01/08/03
  //if(t>0) { wsprintf(str,"%ld:%.2ld", t/60, t%60); dc.TextOut(r.Left()+TabPositions[6],r.Top(),str); }
  if(t>0) {
    lstrcpy(str,PrintSeconds(t));
    strcat(str,"   ");  // added. P.P. 02/27/05
    dc.TextOut(r.Left()+TabPositions[6],r.Top(),str);

  }
//  } // added - from above. P.P. 12/29/06  rem P.P. 12/19/11
}

void PeakWindow::DrawItem(DRAWITEMSTRUCT _FAR & DrawInfo)
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

    Peak *pPeak=(Peak *)DrawInfo.itemData;
    // if !pPeak - this is a total
    Job *pJob=(Job *)SendMessage(Parent->HWindow,WM_GET_JOB,0,0L);
    Run *pRun=(Run *)SendMessage(Parent->HWindow,WM_GET_RUN,0,0L);
//    long mul=pow10( pRun->runrawdata.gain[num] );
    long mul = 1;  // in the new variant, the data is already corrected for the multiplier from raw data! P.P. 06/30/17
    if(!mul) mul=1; // gain==-1? old file?
//    if(pPeak) {
    if((pPeak)&&((pPeak->Num)>0)) { // exclude negative peaks. P.P. 12/29/06
      Method *pMethod=pJob;
      const char *component_name=pPeak->pComponent? pPeak->pComponent->name.Get(): "unknown";
      double concen;
      int flags=pMethod->det[num].Concentration(pMethod, pRun, pPeak, num, &concen);
      long t=pPeak->Time/data_rate;
      DrawPeak(dc, r, component_name, pPeak->Num, concen, flags, t, pPeak->Height, pPeak->Area, H, A, mul);
    }
    #if USE_TOTAL()
    else {
      long t=-1;//pPeak->Time/data_rate;
#if USE_DIFFERENCE() // added. P.P. 08/01/04
      DrawPeak(dc, r, "Chlorine by diff", -1, 100.0-total_concen, total_flags&1, t, 0, 0, 0, 0, mul); // P.P. 08/01/04
#else
#if !QUICK_SCAN() // cond added. P.P. 01/13/11
      DrawPeak(dc, r, "Total", -1, total_concen, total_flags&1, t, total_H, total_A, H, A, mul);
#endif // quick scan
#endif // use_difference
    }
    #endif // USE_TOTAL()
  }

  if(DrawInfo.itemState&ODS_FOCUS)		// Reflect focus state
    dc.FrameRect( r, StockBrush(sbiBlack) );
}

void PeakWindow::WMSetFocus(RTMessage)
{
  SetFocus(list->HWindow);
}

void PeakWindow::WMSize(RTMessage Msg)
{
  Inherited::WMSize(Msg);
  if( (Msg.WParam != SIZEICONIC) && list) {
    WRect rClient;    ::GetClientRect(*this,&rClient);
    rClient.Top()+=HeaderHeight;
    ::MoveWindow(*list,rClient,TRUE);
  }
  InitTabPositions();
}

void PeakWindow::WMInitMenuPopup(RTMessage Msg)
{
  Menu menu((HMENU)Msg.WParam);
  menu.EnableItem(CM_PEAK_ADDITEM, MF_ENABLED);
  if(list && list->GetCount() && list->GetSelIndex()>=0) {
    menu.EnableItem(CM_PEAK_DELITEM, MF_ENABLED);
  }
  Job *pJob=(Job *)SendMessage(Parent->HWindow,WM_GET_JOB,0,0L);
  Run *pRun=(Run *)SendMessage(Parent->HWindow,WM_GET_RUN,0,0L);
  PeakList *pPeakList=((PeakList **)SendMessage(Parent->HWindow,WM_GET_PEAKLIST,0,0L))[num];
  //menu.EnableItem(CM_PRINT, MF_ENABLED);
  menu.EnableItem(CM_EDITCOPY, pJob && pRun && pPeakList? MF_ENABLED: MF_DISABLED);
  menu.EnableItem(CM_EDITCOPYTOFILE, pJob && pRun && pPeakList? MF_ENABLED: MF_DISABLED);
}


void PeakWindow::Paint(HDC PaintDC, PAINTSTRUCT _FAR & PaintInfo)
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

  pc.TextOut(r.Left()+TabPositions[0],r.Bottom(),"Name");
  pc.TextOut(r.Left()+TabPositions[1],r.Bottom(),"Conc");
  pc.SetTextAlign(taRight|taBottom);
  pc.TextOut(r.Left()+TabPositions[2],r.Bottom(),"Height");
  pc.TextOut(r.Left()+TabPositions[3],r.Bottom(),"%");
  pc.TextOut(r.Left()+TabPositions[4],r.Bottom(),"Area");
  pc.TextOut(r.Left()+TabPositions[5],r.Bottom(),"%");
  pc.TextOut(r.Left()+TabPositions[6],r.Bottom(),"Time");
}


void PeakWindow::CMPrint(RTMessage)
{
 ///
}


extern const unsigned CF_RTF=RegisterClipboardFormat("Rich Text Format");

#include "wcpplib/fwstream.h"
#include "wcpplib/memwstrm.h"
static const char far rowheader[]=
//                   n         Name       Conc       Height     %          Area       %          Time
//"\\trowd\\trgaph100\\cellx500\\cellx3236\\cellx4100\\cellx5108\\cellx5842\\cellx6994\\cellx7728\\cellx8639\\intbl \r\n"; // not enough fo 100%
//"\\trowd\\trgaph100\\cellx500\\cellx3236\\cellx4100\\cellx5108\\cellx5942\\cellx7094\\cellx7928\\cellx8839\\intbl \r\n";
"\\trowd\\trgaph100\\cellx500\\cellx3236\\cellx4500\\cellx5508\\cellx6342\\cellx7494\\cellx8328\\cellx9239\\intbl \r\n"; // P.P. 08/16/17

#define TABS() 1

#if !TABS()
static const char manyspaces[]="                                ";
#endif


//static void PrintPeakToFile(owstream &os, UINT cf, Run *pRun, const char *component_name, int Num, double concen, int flags, long t, long Height, long Area, double H, double A, long mul)
static void PrintPeakToFile(owstream &os, UINT cf, Run *pRun, const char *component_name, int Num, double concen, int flags, long t, long Height, double Area, double H, double A, long mul)  // area is double now. P.P. 08/25/17
{
  char str[120];
  if(cf==CF_TEXT) {
    if(Num>0) {
    #if TABS()
    wsprintf(str,"%3d\t%s",Num,component_name);
    #else
    wsprintf(str,"%3d %-25s",Num,component_name);
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
    if(Num>0)
      wsprintf(str,"\\qr %d\\cell ",Num);
    else
      wsprintf(str,"\\qr \\cell ");

    os.write(str,strlen(str));
    wsprintf(str,"\\ql %s\\cell\\qr ",component_name);
  }
  os.write(str,strlen(str));
  {
    if(flags&1) {
//      sprintf(str, "%.3f", concen); // added from old file. P.P. 07/28/04
      sprintf(str, "%.6f", concen); //  higher resolution. P.P. 08/16/17
      if(pRun->type_of_run==JOB) { // cond. added. P.P. 07/26/04
	if(flags&2) strcat( str, " H" );
	if(flags&4) strcat( str, " L" );
      }
    }
    else {
      str[0]=0;
    }
    if(cf==CF_TEXT) {
      #if TABS()
      os.write("\t",1);
      #else
      if(strlen(str)<10) os.write(manyspaces,10-strlen(str));
      #endif
      os.write(str,strlen(str));
    }
    else {
      os.write(str,strlen(str));
      wsprintf(str,"\\cell ");
      os.write(str,strlen(str));
    }
  }
  if(cf==CF_TEXT) {
    #if TABS()
    wsprintf(str,"\t%ld",Height*mul);
    #else
    wsprintf(str,"%13ld",Height*mul);
    #endif

    //if(strlen(str)<13) os.write(manyspaces,13-strlen(str));
  }
  else {
    wsprintf(str,"%ld\\cell ",Height*mul);
  }
  os.write(str,strlen(str));
  if(cf==CF_TEXT) {
    #if TABS()
    sprintf(str,"\t%.2f",H? Height*100./H: 0.);
    #else
    sprintf(str,"%6.2f",H? Height*100./H: 0.);
    #endif
    //if(strlen(str)<6) os.write(manyspaces,6-strlen(str));
  }
  else {
    sprintf(str,"%.2f\\cell ",double(H? Height*100./H: 0.));
  }
  os.write(str,strlen(str));
  if(cf==CF_TEXT) {
    #if TABS()
//    wsprintf(str,"\t%ld",Area*mul);
    sprintf(str,"\t%.0f",(float) Area*mul); // area is double, show as float and with sprintf. P.P. 08/27/17
    #else
//    wsprintf(str,"%13ld",Area*mul);
    sprintf(str,"%13.0f",(float) Area*mul); // P.P. 08/27/17
    #endif
    //if(strlen(str)<13) os.write(manyspaces,13-strlen(str));
  }
  else {
//    wsprintf(str,"%ld\\cell ",Area*mul);  
    sprintf(str,"%.0f\\cell ",(float) Area*mul);  // P.P. 08/27/17
  }
  os.write(str,strlen(str));
  if(cf==CF_TEXT) {
    #if TABS()
    sprintf(str,"\t%.2f",A? Area*100./A: 0.);
    #else
    sprintf(str,"%6.2f",A? Area*100./A: 0.);
    #endif
    //if(strlen(str)<6) os.write(manyspaces,6-strlen(str));
  }
  else {
    sprintf(str,"%.2f\\cell",A? Area*100./A: 0.);
  }
  os.write(str,strlen(str));
  if(cf==CF_TEXT) {
    #if TABS()
    if(t>0) wsprintf(str,"\t%ld:%.2ld\r\n", t/60, t%60);
    else str[0]=0;
    #else
    if(t>0) wsprintf(str,"%ld:%.2ld\r\n", t/60, t%60);
    else str[0]=0;
    if(strlen(str)<10) os.write(manyspaces,10-strlen(str));
    #endif
  }
  else  {
    if(t>0)
      wsprintf(str,"\\qr %ld:%.2ld\\cell\\row \r\n", t/60, t%60);
    else
      wsprintf(str,"\\qr \\cell\\row \r\n");
  }
  os.write(str,strlen(str));
}

void PrintPeakToFile(owstream &os,UINT cf, PeakList *pPeakList, Method *pMethod, Detector *pDetector, Run *pRun, int num)
{
  if(cf==CF_TEXT) {
    static const char far header[]=
    #if TABS()
    "\tName\tConc\tHeight\t%\tArea  \t%\tTime\r\n";
    #else
    "    Name                           Conc       Height     %         Area     %    Time\r\n";
    #endif
    os.write(header,sizeof(header)-1);
  }
  else {
    os.write(rowheader,sizeof(rowheader)-1);
    static const char far headerrow[]=
    "\\qr\\cell\\ql Name\\cell\\qr Conc\\cell Height\\cell %\\cell Area\\cell %\\cell Time\\cell\\row \r\n";
    os.write(headerrow,sizeof(headerrow)-1);
  }
  if(pPeakList) {
    double A=0;
    double H=0;
    {
      for(PeakList::Node *pNode=pPeakList->first(); pNode; pNode=pNode->nextNode()) {
	if(pNode->Height>0) {  // cond added. P.P. 12/31/06
	  H+=pNode->Height;
	}
	if(pNode->Area>0) {   // cond added. P.P. 12/31/06
	  A+=pNode->Area;
        }
      }
    }
    #if USE_TOTAL()
    int total_flags=0;
    double total_concen=0;
    long total_H=0;
    long total_A=0;
    #endif // USE_TOTAL()
//    long mul=pow10( pRun->runrawdata.gain[num] );
    long mul=1; // P.P. 07/8/16/17
    if(!mul) mul=1; // gain==-1? old file?
    for(PeakList::Node *pNode=pPeakList->first(); pNode; pNode=pNode->nextNode()) {
      Peak *pPeak=pNode;
      const char *component_name=pPeak->pComponent? pPeak->pComponent->name.Get(): "unknown";
      double concen;
      int flags=pDetector?pDetector->Concentration(pMethod, pRun, pPeak, num, &concen):0;
      #if USE_TOTAL()
      total_flags|=flags;
      if(flags&1) {
	if(concen>0) {           // cond added. P.P. 12/31/06
	  total_concen+=concen;
        }
	if(pPeak->Height>0) {    // cond added. P.P. 12/31/06
	  total_H+=pPeak->Height;
	}
	if(pPeak->Area>0) {      // cond added. P.P. 12/31/06
	  total_A+=pPeak->Area;
        }
      }
      #endif // USE_TOTAL()
      long t=pPeak->Time/data_rate;
      PrintPeakToFile(os,cf, pRun, component_name, pPeak->Num, concen, flags, t, pPeak->Height, pPeak->Area, H, A, mul);
    }
    #if USE_TOTAL()
    {
      long t=-1;//pPeak->Time/data_rate;
//      PrintPeakToFile(os,cf, "Total", -1, total_concen, total_flags&1, t, total_H, total_A, H, A, mul);
#if !QUICK_SCAN() // cond added. P.P. 01/13/11
#if USE_DIFFERENCE()
      PrintPeakToFile(os,cf, pRun, "Chlorine by diff", -1, 100.0-total_concen, total_flags&1, t, 0, 0, 0, 0, mul); // P.P. 08/01/04
#else 
      PrintPeakToFile(os,cf, pRun, "Total", -1, total_concen, total_flags&1, t, total_H, total_A, H, A, mul); // P.P. 08/01/04
#endif // use difference
#endif // quick scan
    }
    #endif // USE_TOTAL()
  }
}

long PeakWindow::CreateFile(UINT cf, const char *file)
{
  long len=0;
  Job *pJob=(Job *)SendMessage(Parent->HWindow,WM_GET_JOB,0,0L);
  Run *pRun=(Run *)SendMessage(Parent->HWindow,WM_GET_RUN,0,0L);
  PeakList *pPeakList=((PeakList **)SendMessage(Parent->HWindow,WM_GET_PEAKLIST,0,0L))[num];
  if(pRun && pJob && pPeakList) {
    {
      ofwstream os(file);
      if(os.good()) {
	if(cf!=CF_TEXT) {
	  static const char header[]="{\\rtf1\r\n\\pard\\plain\\fs20\r\n";
	  // font name : default
	  // font size : 20pt/2 (i.e. 10pt)
	  os.write(header,sizeof(header)-1);
	}

	if(pJob && pRun && pPeakList) {
	  Method *pMethod=pJob;
	  PrintPeakToFile(os,cf,pPeakList, pMethod, &pMethod->det[num], pRun, num);
	}
	if(cf!=CF_TEXT) {
	  os.write("}",1);
	}
	len=os.tell();
      }
    }
  }
  return len;
}


void PeakWindow::CMEditCopy(RTMessage)
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
void PeakWindow::CMEditCopyToFile(RTMessage)
{
  Run *pRun=(Run *)SendMessage(Parent->HWindow,WM_GET_RUN,0,0L);
  if(!pRun) return;

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


void PeakWindow::WMCommand(RTMessage Msg)
{
  switch(Msg.LP.Hi) {
    case LBN_SELCHANGE:
    break;
  }
  Inherited::WMCommand(Msg);
}

void PeakWindow::CMAddItem(RTMessage)
{
  if(!list) return;
}

void PeakWindow::CMDeleteItem(RTMessage)
{
  if(!list) return;
  int Index=list->GetSelIndex();
  if(Index<0) return;
  MessageBeep(MB_ICONHAND);
  if(IDOK==MessageBox(NULL,"Are you realy want to delete this peak?", "Warning", MB_OKCANCEL|MB_ICONHAND|MB_TASKMODAL)) {
    remove(Index);
  }
}

void PeakWindow::HandleList(RTMessage Msg)
{
  switch(Msg.LP.Hi) {
    case LBN_DBLCLK:
//      int Index=list->GetSelIndex();
//      GetModule()->MakeWindow(newDIBWindow(Parent,(LPSTR)pPeakList->GetChild(Index)->Get()));
    break;
  }
}




#ifndef  _PEAK_WND_H
#include "peak_wnd.h"
#endif

PTWindowsObject newPeakWindow(PTWindowsObject AParent, int num, char far *ATitle, PTModule AModule)
{
  return new PeakWindow(AParent, num, ATitle,AModule);
}





