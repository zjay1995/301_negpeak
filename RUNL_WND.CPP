#include "includes.h"
#pragma hdrstop

#error not used

#ifndef __BWINDOW_H
#include <bwindow.h>
#endif
#ifndef __LISTBOX_H
#include <listbox.h>
#endif

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
#ifndef  _WINLIB_DC_H
#include "winlib\dc.h"
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
#ifndef  _RUNLIST_H
#include "runlist.h"
#endif
#ifndef  _JOB_H
#include "job.h"
#endif

#ifndef  _COUNTRY_H
#include "country.h"
#endif

#ifndef  _PEAK_APP_H
#include "peak_app.h"
#endif


class RunListBox : public TListBox {
  typedef TListBox Inherited;
 public:
  RunListBox(PTWindowsObject AParent, int AnId, int X, int Y, int W, int H, PTModule AModule = NULL)
    : TListBox(AParent, AnId, X, Y, W, H, AModule ) {}
  RunListBox(PTWindowsObject AParent, int ResourceId, PTModule AModule = NULL)
    : TListBox(AParent, ResourceId, AModule ) {}
  virtual void WMChar(RTMessage Msg) = [WM_FIRST + WM_CHAR];

  RunListBox(StreamableInit);
  virtual const Pchar streamableName() const { return name; }
 public:
  static PTStreamable build();
  static char far name[];
};

RunListBox::RunListBox(StreamableInit)
 : TListBox(streamableInit)
{
};

TStreamableClass RegRunListBox(RunListBox::name, RunListBox::build, __DELTA(RunListBox));
char far RunListBox::name[]="RunListBox";
PTStreamable RunListBox::build()
{
  return new RunListBox(streamableInit);
}


void RunListBox::WMChar(RTMessage Msg)
{
  switch(Msg.WParam) {
    case VK_RETURN:
      PostMessage(Parent->HWindow, WM_COMMAND, CM_RUNLIST_EDITITEM, 0L);
    break;
    case VK_INSERT:
      PostMessage(Parent->HWindow, WM_COMMAND, CM_RUNLIST_ADDITEM, 0L);
    break;
    case VK_DELETE:
//    PostMessage(Parent->HWindow, WM_COMMAND, CM_RUNLIST_DELITEM, 0L);
    break;
  }
  DefWndProc(Msg);
}

class RunListWindow : public TBWindow {
  typedef TBWindow Inherited;
  PTListBox list;
  int HeaderHeight;
  int xZ,x0,x1,x2,x3,x4;
  RunList *pRunList;
  virtual LPSTR GetClassName();
  virtual void GetWindowClass(WNDCLASS &AWndClass);
 public:
  RunListWindow(PTWindowsObject AParent,PTModule AModule = NULL);
  ~RunListWindow();
 protected:
  void Fill();
  virtual void SetupWindow();
  virtual void ActivationResponse(WORD Activated, BOOL IsIconified);
  virtual void WMInitMenuPopup(RTMessage) = [WM_FIRST + WM_INITMENUPOPUP];
  virtual void Paint(HDC PaintDC, PAINTSTRUCT _FAR & PaintInfo);
  virtual void DrawItem(DRAWITEMSTRUCT _FAR & DrawInfo);
  virtual void WMMeasureItem(TMessage& Msg) = [WM_FIRST + WM_MEASUREITEM];
  virtual void WMSize(RTMessage Msg) = [WM_FIRST + WM_SIZE];
  virtual void WMSetFocus(RTMessage Msg) = [WM_FIRST + WM_SETFOCUS];
  virtual void WMCommand(RTMessage Msg) = [WM_FIRST + WM_COMMAND];
  virtual void WMIsItYourName(RTMessage Msg) = [WM_FIRST + WM_IS_IT_YOUR_NAME];

  virtual void CMEditItem(RTMessage Msg) = [CM_FIRST +  CM_RUNLIST_EDITITEM];
//  virtual void CMDeleteItem(RTMessage Msg) = [CM_FIRST + CM_RUNLIST_DELITEM];

  virtual void CMIntegrate(RTMessage Msg) = [CM_FIRST + CM_INTEGRATE];
  virtual void CMPrint(RTMessage Msg) = [CM_FIRST + CM_PRINT];

  virtual void HandleList(RTMessage Msg) = [ID_FIRST + IDD_RUNLIST];

  virtual void WMNewRun(RTMessage Msg) = [WM_FIRST + WM_NEW_RUN];
  virtual void WMRefresh(RTMessage Msg) = [WM_FIRST + WM_REFRESH];

  void append(const char *s, int Index)
  {
    //pRunList->append(s,Index);
    if(Index>=list->GetCount()) {
      list->AddString((LPSTR)s);
    }
    else {
      list->InsertString((LPSTR)s,Index+1);
    }
    list->SetSelIndex(Index+1);
  }
  void remove(int Index)
  {
    //pRunList->remove(Index);
    list->DeleteString(Index);
    list->SetSelIndex(Index>1?Index-1:0);
  }


  RunListWindow(StreamableInit);
  virtual void write (Ropstream os);
  virtual Pvoid read (Ripstream is);
  virtual const Pchar streamableName() const { return name; }
 public:
  static PTStreamable build();
  static char far name[];
};

LPSTR RunListWindow::GetClassName()
{
  return name;
}

void RunListWindow::GetWindowClass(WNDCLASS& WndClass)
{
  Inherited::GetWindowClass(WndClass);
  WndClass.hIcon = LoadIcon(GetModule()->hInstance,"LIST");
}


RunListWindow::RunListWindow(PTWindowsObject AParent,PTModule AModule)
  : TBWindow(AParent,"RunList",AModule),
    list(new RunListBox(this, IDD_RUNLIST, 0, 0, 0, 0, AModule)),
    HeaderHeight(20), xZ(0),x0(0),x1(0),x2(0),x3(0),x4(0)
{
  Attr.Style |= WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE;
  if(list) {
    list->Attr.Style&=~(WS_BORDER|LBS_SORT);
    list->Attr.Style|=LBS_NOTIFY|LBS_OWNERDRAWFIXED|/*LBS_HASSTRINGS|*/LBS_NOINTEGRALHEIGHT|WS_VSCROLL|WS_CHILD|WS_VISIBLE|WS_GROUP|WS_TABSTOP;
  }
}

RunListWindow::~RunListWindow(void)
{
}

void RunListWindow::WMRefresh(RTMessage)
{
  if(list) list->InvalidateRect(HWindow, 0, 0);
}


void RunListWindow::WMNewRun(RTMessage)
{
  Fill();
}

void RunListWindow::Fill()
{
  Job *pJob=(Job *)SendMessage(Parent->HWindow,WM_GET_JOB,0,0L);
  if(pJob && list) {
    pRunList=pJob;
    list->SendMessage(WM_SETREDRAW,FALSE,0L);
    list->ClearList();
    for(RunList::Node *prln=pRunList->first(); prln; prln=prln->nextNode()) {
      list->AddString((LPSTR)prln);
    }
    list->SetSelIndex(0);
    list->SendMessage(WM_SETREDRAW,TRUE,0L);
    list->InvalidateRect(NULL,FALSE);
  }
}

void RunListWindow::SetupWindow()
{
  Inherited::SetupWindow();
  Fill();
  TEXTMETRIC tm;
  dc.GetTextMetrics(&tm);
  int dx=dc.GetTextExtent(" ").X();
  xZ=0;//dx/2;
  x0=xZ+dc.GetTextExtent("*").X()+dx+tm.tmAveCharWidth*2;
  x1=x0+dx;
  x2=x1+tm.tmAveCharWidth*8+dx;
//  x3=x2+tm.tmAveCharWidth*4+dx;
  x3=x2+tm.tmAveCharWidth*7+dx;   // P.P. 11/01/13
  x4=x3+dc.GetTextExtent("cal").X()+dx;
}

RunListWindow::RunListWindow(StreamableInit)
 : TBWindow(streamableInit), list(0), xZ(0), x0(0),x1(0),x2(0),x3(0),x4(0)
{
};

TStreamableClass RegRunListWindow(RunListWindow::name, RunListWindow::build, __DELTA(RunListWindow));
char far RunListWindow::name[]="RunListWindow";
PTStreamable RunListWindow::build()
{
  return new RunListWindow(streamableInit);
}

void RunListWindow::write(Ropstream os)
{
  Inherited::write(os);
  os<<HeaderHeight;
  PutChildPtr(os,list);
}

Pvoid RunListWindow::read(Ripstream is)
{
  if(!Inherited::read(is)) return NULL;
  is>>HeaderHeight;
  GetChildPtr(is,(RPTWindowsObject)list);
  return this;
}

void RunListWindow::WMIsItYourName(RTMessage Msg)
{
  Msg.Result=lstrcmpi(GetClassName(),(LPCSTR)Msg.LParam)==0;
}

void RunListWindow::WMMeasureItem(TMessage& Msg)
{
  MEASUREITEMSTRUCT FAR *lpmisCtl = (MEASUREITEMSTRUCT FAR*) Msg.LParam;
  //lpmisCtl->itemWidth=r.Width()/((nPict+nRow/2)/nRow);
  TEXTMETRIC tm;
  ClientDC(HWindow).GetTextMetrics(&tm);

  lpmisCtl->itemHeight=tm.tmHeight+tm.tmExternalLeading;
  HeaderHeight=lpmisCtl->itemHeight+2;
}

#ifndef  _HNU_H
#include "hnu.h"
#endif
void RunListWindow::DrawItem(DRAWITEMSTRUCT _FAR & DrawInfo)
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

  r.Inflate( -1, -1 );

  if(int(DrawInfo.itemID)>=0 && /*DrawInfo.itemAction&ODA_DRAWENTIRE*/1) {	// Redraw if neccesary
    //RunList::Node *prln=(RunList::Node *)DrawInfo.itemData;
    RunList::Node *prln=pRunList->GetChild(DrawInfo.itemID);
    if(prln) {
      SelTextColor oldTextColor(dc,color);
      SelBkMode oldBkMode(dc,bkmTransparent);
      SelTextAlign oldTextAlign(dc,taLeft|taBottom);
      Run *pRun=(Run *)SendMessage(Parent->HWindow,WM_GET_RUN,0,0L);
      dc.TextOut(r.Left()+xZ,r.Bottom(), pRun==prln? "*": "");
      {
      SelTextAlign oldTextAlign(dc,taRight|taBottom);
      char str[12]; wsprintf(str, "%d",1+DrawInfo.itemID);
      dc.TextOut(r.Left()+x0,r.Bottom(),str);
      }

      dc.TextOut(r.Left()+x1,r.Bottom(),PrintDate(prln->start_time));
      dc.TextOut(r.Left()+x2,r.Bottom(),PrintTime(prln->start_time));
      dc.TextOut(r.Left()+x3,r.Bottom(),prln->type_of_run==CAL? "c": "");
      Str comment=prln->comment;
      char *cp=strchr(comment,'\r'); if(cp) *cp=0;
      dc.TextOut(r.Left()+x4,r.Bottom(),comment);
    }
  }

  r.Inflate( +1, +1 );

  if(DrawInfo.itemState&ODS_FOCUS)		// Reflect focus state
    dc.FrameRect( r, StockBrush(sbiBlack) );
}

void RunListWindow::WMSetFocus(RTMessage)
{
  SetFocus(list->HWindow);
}

void RunListWindow::WMSize(RTMessage Msg)
{
  Inherited::WMSize(Msg);
  if( (Msg.WParam != SIZEICONIC) && list) {
    WRect rClient;    ::GetClientRect(*this,&rClient);
    rClient.Top()+=HeaderHeight;
    ::MoveWindow(*list,rClient,TRUE);
  }
}

#ifndef  _FRAME_H
#include "frame.h"
#endif

void RunListWindow::ActivationResponse(WORD Activated, BOOL IsIconified)
{
  FrameWindow *pFrame=(FrameWindow *)Parent;
  int b= Activated&& list && list->GetCount() && list->GetSelIndex()>=0;
  pFrame->EnableCommand(CM_INTEGRATE, b);
  pFrame->EnableCommand(CM_RUNLIST_EDITITEM, b);
  pFrame->EnableCommand(CM_RUNLIST_DELITEM, b);
  pFrame->EnableCommand(CM_RUNLIST_ADDITEM, Activated);
  pFrame->EnableCommand(CM_PRINT, Activated);
  Inherited::ActivationResponse(Activated, IsIconified);
}

void RunListWindow::WMInitMenuPopup(RTMessage Msg)
{
//  Menu menu((HMENU)Msg.WParam);
//  if(list && list->GetCount() && list->GetSelIndex()>=0) {
//    menu.EnableItem(CM_INTEGRATE, MF_ENABLED);
//    menu.EnableItem(CM_RUNLIST_EDITITEM, MF_ENABLED);
//    menu.EnableItem(CM_RUNLIST_DELITEM, MF_ENABLED);

    //menu.EnableItem(CM_EDITCOPY, MF_ENABLED);
    //menu.EnableItem(CM_EDITCOPYOUTPUT, MF_ENABLED);
    //menu.EnableItem(CM_PRINT, MF_ENABLED);
    //menu.EnableItem(CM_EXPORT, MF_ENABLED);
//  }
}



void RunListWindow::Paint(HDC PaintDC, PAINTSTRUCT _FAR & PaintInfo)
{
  PC pc(PaintDC,&PaintInfo);
  WRect r;    ::GetClientRect(*this,&r);
  r.Bottom()=r.Top()+HeaderHeight;
  r.B()-=WPoint(1,1);
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

  pc.TextOut(r.Left()+x1,r.Bottom(),"Date");
  pc.TextOut(r.Left()+x2,r.Bottom(),"Time");
  pc.TextOut(r.Left()+x4,r.Bottom(),"Comment");
}

void RunListWindow::WMCommand(RTMessage Msg)
{
  switch(Msg.LP.Hi) {
  }
  Inherited::WMCommand(Msg);
}


#ifndef  _TD_PLUS_H
#include "td_plus.h"
#endif
class RunDialog : public TransferPlusDlg {
  typedef TransferPlusDlg Inherited;
 public:
   RunDialog(PTWindowsObject AParent, int ResourceId, Run *data, ControlInfo *cip, PTModule AModule = NULL)
    : TransferPlusDlg(AParent, ResourceId, data, cip, AModule) {}
   virtual void SetupControls(void *data);
};

void RunDialog::SetupControls(void *data)
{
  Inherited::SetupControls(data);
  Run *pRun=(Run *)data;
  char str[80];
  wsprintf(str,"%s  %s",PrintDate(pRun->start_time),PrintTime(pRun->start_time));
  SetDlgItemText(HWindow,IDD_RUN_DATE,str);
}


static ControlInfo far ci[]={
// {IDD_RUN_DATE
  { citStr, IDD_RUN_NOTES,	FIELDOFFSET(Run,comment),	},
  { citNULL }	// End of array marker
};

void RunListWindow::CMEditItem(RTMessage)
{
  if(!list) return;
  if(!pRunList) return;
  int Index=list->GetSelIndex();
  RunList::Node *prln=pRunList->GetChild(Index);
  if(!prln) return;
  if(IDOK==GetModule()->ExecDialog( new RunDialog(this,ID_RUNDIALOG, prln, ci) )) {
    ::InvalidateRect(list->HWindow,NULL,FALSE);
  }
}

/*
void RunListWindow::CMDeleteItem(RTMessage)
{
  if(!list) return;
  int Index=list->GetSelIndex();
  if(Index<0) return;
  MessageBeep(MB_ICONHAND);
  if(IDOK==MessageBox(NULL,"Are you realy want to delete this run?", "Warning", MB_OKCANCEL|MB_ICONHAND)) {
    MessageBox(NULL,"Not work yet", "", MB_OK);
    //remove(Index);
  }
}
*/

void RunListWindow::CMIntegrate(RTMessage)
{
  int Index=list->GetSelIndex();
  if(Index<0) return;
  PostMessage(Parent->HWindow,WM_INTEGRATE_RUN,Index,0L);
}

void RunListWindow::HandleList(RTMessage Msg)
{
  switch(Msg.LP.Hi) {
    case LBN_SELCHANGE:
    break;
    case LBN_DBLCLK:
      PostMessage(HWindow,WM_COMMAND,CM_RUNLIST_EDITITEM,0L);
    break;
  }
}

#ifndef  _RUNL_PRN_H
#include "runl_prn.h"
#endif

void RunListWindow::CMPrint(RTMessage Msg)
{
  PeakApp *pApp=((PeakApp *)GetApplication());
  Printer *pPrinter=pApp;
  if( pPrinter ) {
    RunListPrintout *pPrintout = new RunListPrintout(pApp->Name, pRunList);
    if( pPrintout ) {
      pPrinter->Print(this, pPrintout, TRUE);
      delete pPrintout;
    }
  }
}


#ifndef  _RUNL_WND_H
#include "runl_wnd.h"
#endif

PTWindowsObject newRunListWindow(PTWindowsObject AParent, PTModule AModule)
{
  return new RunListWindow(AParent, AModule);
}

