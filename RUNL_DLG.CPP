#include "includes.h"
#pragma hdrstop

#ifndef  _HELP_DLG_H
#include "help_dlg.h"
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

#ifndef  _RUNLBOX_H
#include "runlbox.h"
#endif

#ifndef  _RUNL_DLG_H
#include "runl_dlg.h"
#endif

class RunListDialog : public HelpDlg {
  typedef HelpDlg Inherited;
  PTListBox list;
  int xZ,x0,x1,x2,x3,x4;
  RunList *pRunList;
 public:
  RunListDialog(PTWindowsObject AParent, int ResourceId, RunListDialogInfo *pInfo, PTModule AModule = NULL);
  ~RunListDialog();
 protected:
  void Fill(RunListDialogInfo *pInfo);
  virtual void DrawItem(DRAWITEMSTRUCT _FAR & DrawInfo);
  virtual void WMMeasureItem(TMessage& Msg) = [WM_FIRST + WM_MEASUREITEM];

  virtual void HandleList(RTMessage Msg) = [ID_FIRST + IDD_RUNLIST];
  virtual void CMPrint(RTMessage Msg) = [ID_FIRST + IDPRINT];
  virtual void HandleSelectAll(RTMessage Msg) = [ID_FIRST + IDD_SELECT_ALL];
  virtual void HandleUnselectAll(RTMessage Msg) = [ID_FIRST + IDD_UNSELECT_ALL];

  /*
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
  */
  virtual void SetupControls(void *data);
  virtual BOOL ValidateControls(void *data);
  virtual void AcceptControls(void *data);
};


RunListDialog::RunListDialog(PTWindowsObject AParent,int ResourceId, RunListDialogInfo *pInfo,PTModule AModule)
  : HelpDlg(AParent,ResourceId,pInfo,AModule),
    list(new RunListBox(this, IDD_RUNLIST, AModule)),
    xZ(0),x0(0),x1(0),x2(0),x3(0),x4(0)
{
}

RunListDialog::~RunListDialog(void)
{
}


void RunListDialog::Fill(RunListDialogInfo *pInfo)
{
  Job *pJob=(Job *)pInfo->pJob;
  if(pJob && list) {
    pRunList=pJob;
    ::SendMessage(list->HWindow,WM_SETREDRAW,FALSE,0L);
    list->ClearList();
    Run *pRun=(Run *)SendMessage(Parent->HWindow,WM_GET_RUN,0,0L);
    int lCurrent=0;
    for(RunList::Node *prln=pRunList->first(); prln; prln=prln->nextNode()) {
      int i=list->AddString((LPSTR)prln);
      if(pRun==prln) {
        lCurrent=i;
      }
    }
    if(pInfo->bMul) {
      list->SetSelIndexes(0, -1, 0);
      if(pInfo->ind.p) list->SetSelIndexes(pInfo->ind.p, pInfo->ind.n, 1);
    }
    else {
      if(pInfo->ind.p) list->SetSelIndex(*pInfo->ind.p);
    }

    ::SendMessage(list->HWindow,LB_SETCARETINDEX,lCurrent,MAKELPARAM(TRUE, 0));

    ::SendMessage(list->HWindow,WM_SETREDRAW,TRUE,0L);
    ::InvalidateRect(list->HWindow,NULL,FALSE);
  }
}

void RunListDialog::SetupControls(void *data)
{
//  Inherited::SetupControls(data);
  RunListDialogInfo *pInfo=(RunListDialogInfo *)(RunListDialogInfo *)data;

  Fill(pInfo);

  TEXTMETRIC tm;
  ClientDC dc(HWindow);
  dc.GetTextMetrics(&tm);
  int dx=dc.GetTextExtent(" ").X();
  xZ=0;//dx/2;
  x0=xZ+tm.tmAveCharWidth*3+dx;
  x1=x0+dx;
  x2=x1+tm.tmAveCharWidth*8+dx;
//  x3=x2+tm.tmAveCharWidth*4+dx;
  x3=x2+tm.tmAveCharWidth*7+dx;    // P.P. 11/01/13
  x4=x3+dc.GetTextExtent("cal").X()+dx;
}
BOOL RunListDialog::ValidateControls(void *data)
{
  RunListDialogInfo *pInfo=(RunListDialogInfo *)data;
  return Inherited::ValidateControls(data) && (list->GetSelCount() || pInfo->bMul);
}
void RunListDialog::AcceptControls(void *data)
{
  RunListDialogInfo *pInfo=(RunListDialogInfo *)data;
  pInfo->ind.Alloc(list->GetSelCount());
  if(pInfo->bMul) {
    list->GetSelIndexes(pInfo->ind.p, pInfo->ind.n);
  }
  else {
    if(pInfo->ind.p) *pInfo->ind.p=list->GetSelIndex();
  }
}

void RunListDialog::WMMeasureItem(TMessage& Msg)
{
  MEASUREITEMSTRUCT FAR *lpmisCtl = (MEASUREITEMSTRUCT FAR*) Msg.LParam;
  if(lpmisCtl->CtlID==IDD_RUNLIST_LABEL) {
    return;
  }
  //lpmisCtl->itemWidth=r.Width()/((nPict+nRow/2)/nRow);
  TEXTMETRIC tm;
  ClientDC(HWindow).GetTextMetrics(&tm);

  lpmisCtl->itemHeight=tm.tmHeight+tm.tmExternalLeading;
}

#ifndef  _HNU_H
#include "hnu.h"
#endif
#ifndef  _WINLIB_BITMAP_H
#include "winlib\bitmap.h"
#endif

void RunListDialog::DrawItem(DRAWITEMSTRUCT _FAR & DrawInfo)
{
  DC dc(DrawInfo.hDC);
  WRect r(DrawInfo.rcItem);
  if(DrawInfo.CtlID==IDD_RUNLIST_LABEL) {
    {
      HBRUSH hBrush=(HBRUSH)SendMessage(HWindow,WM_CTLCOLOR,(WPARAM)DrawInfo.hDC,MAKELPARAM(DrawInfo.hwndItem,
	/*l? CTLCOLOR_LISTBOX:*/ CTLCOLOR_STATIC));
      if(hBrush) dc.FillRect(r,Brush(hBrush));
    }
    SelBkMode oldBkMode(dc,bkmTransparent);
    SelTextAlign oldTextAlign(dc,taLeft|taTop);
    dc.TextOut(r.Left()+x1,r.Top(),"Date");
    dc.TextOut(r.Left()+x2,r.Top(),"Time");
    dc.TextOut(r.Left()+x4,r.Top(),"Comment");
    return;
  }
  Color bkColor;
  Color color;
  if(DrawInfo.itemState&ODS_SELECTED) {		// Reflect selection state
    bkColor=GetSysColor(COLOR_HIGHLIGHT);
    color=GetSysColor(COLOR_HIGHLIGHTTEXT);
  }
  else {
    bkColor=GetSysColor(COLOR_WINDOW);
    color=GetSysColor(COLOR_WINDOWTEXT);
  }
  dc.FillRect(r,LogBrush(bkColor));

  r.Inflate( -1, -1 );

  if(int(DrawInfo.itemID)>=0 && /*DrawInfo.itemAction&ODA_DRAWENTIRE*/1) {	// Redraw if neccesary
    //RunList::Node *prln=(RunList::Node *)DrawInfo.itemData;
    RunList::Node *prln=pRunList->GetChild(DrawInfo.itemID);
    if(prln) {
      SelTextColor oldTextColor(dc,color);
      SelBkMode oldBkMode(dc,bkmTransparent);
      SelTextAlign oldTextAlign(dc,taLeft|taBottom);
      Run *pRun=(Run *)SendMessage(Parent->HWindow,WM_GET_RUN,0,0L);
      //dc.TextOut(r.Left()+xZ,r.Bottom(), pRun==prln? "*": "");
      if(pRun==prln) {
	SelBkColor oldBkColor(dc,bkColor);
	StockBitmap bmpCheck(OBM_CHECK);
	WPoint pt=bmpCheck.Size( dc );
	bmpCheck.BitBlt(dc,r.Left()+xZ,r.Top(), pt.x, pt.y, 0, 0, SRCCOPY);
      }
      {
      SelTextAlign oldTextAlign(dc,taRight|taBottom);
      char str[12]; wsprintf(str, "%d",1+DrawInfo.itemID);
      dc.TextOut(r.Left()+x0,r.Bottom(),str);
      }

      dc.TextOut(r.Left()+x1,r.Bottom(),PrintDate(prln->start_time));
      dc.TextOut(r.Left()+x2,r.Bottom(),PrintTime(prln->start_time));
      dc.TextOut(r.Left()+x3,r.Bottom(),prln->type_of_run==CAL? "cal": ""); // restored. P.P. 11/01/13
//      dc.TextOut(r.Left()+x3,r.Bottom(),prln->type_of_run==CAL? "": ""); // removed extra cal. P.P. 04/25/03
      Str comment=prln->comment;
      const char *cp=strchr(comment,'\r'); if(cp) *(char *)cp=0;
      dc.TextOut(r.Left()+x4,r.Bottom(),comment);
    }
  }

  r.Inflate( +1, +1 );

  if(DrawInfo.itemState&ODS_FOCUS)		// Reflect focus state
    dc.FrameRect( r, StockBrush(sbiBlack) );
}



void RunListDialog::HandleList(RTMessage Msg)
{
  switch(Msg.LP.Hi) {
    case LBN_SELCHANGE:
    break;
    case LBN_DBLCLK:
    {
      /*RunListDialogInfo *pInfo=(RunListDialogInfo *)TransferBuffer;
      if(!pInfo->bMul)*/ {
	CloseWindow(IDOK);
      }
    }
    break;
  }
}

void RunListDialog::HandleSelectAll(RTMessage)
{
  if(list) list->SetSelIndexes(0, -1, 1);
}
void RunListDialog::HandleUnselectAll(RTMessage)
{
  if(list) list->SetSelIndexes(0, -1, 0);
}

#ifndef  _RUNL_PRN_H
#include "runl_prn.h"
#endif

void RunListDialog::CMPrint(RTMessage )
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


PTWindowsObject newRunListDialog(PTWindowsObject AParent, int ResourceId, RunListDialogInfo *pInfo, PTModule AModule)
{
  return new RunListDialog(AParent, ResourceId, pInfo, AModule);
}




class StandardListDialog : public HelpDlg {
  typedef HelpDlg Inherited;
  PTListBox list;
  //int xZ,x0,x1,x2,x3,x4;
 public:
  StandardListDialog(PTWindowsObject AParent, int ResourceId, int *nStandard, PTModule AModule = NULL);
  ~StandardListDialog();
 protected:
  void Fill();
  //virtual void DrawItem(DRAWITEMSTRUCT _FAR & DrawInfo);
  //virtual void WMMeasureItem(TMessage& Msg) = [WM_FIRST + WM_MEASUREITEM];

  virtual void HandleList(RTMessage Msg) = [ID_FIRST + IDD_STANDARDLIST];

  virtual void SetupControls(void *data);
  virtual BOOL ValidateControls(void *data);
  virtual void AcceptControls(void *data);
};


StandardListDialog::StandardListDialog(PTWindowsObject AParent,int ResourceId, int *nStandard,PTModule AModule)
  : HelpDlg(AParent,ResourceId,nStandard,AModule),
    list(new TListBox(this, IDD_STANDARDLIST, AModule))
    //,xZ(0),x0(0),x1(0),x2(0),x3(0),x4(0)
{
}

StandardListDialog::~StandardListDialog(void)
{
}


void StandardListDialog::Fill()
{
  ::SendMessage(list->HWindow,WM_SETREDRAW,FALSE,0L);
  for(int i=0; i<STAND_NUM; i++) {
    char buff[50];
    wsprintf(buff,"Standard %d", i);
    list->AddString(buff);
  }
  ::SendMessage(list->HWindow,WM_SETREDRAW,TRUE,0L);
  ::InvalidateRect(list->HWindow,NULL,FALSE);
}

void StandardListDialog::SetupControls(void *data)
{
//  Inherited::SetupControls(data);
  const int &nStandard=*(const int *)data;

  Fill();
  list->SetSelIndex(nStandard);
  /*
  TEXTMETRIC tm;
  ClientDC dc(HWindow);
  dc.GetTextMetrics(&tm);
  int dx=dc.GetTextExtent(" ").X();
  xZ=0;//dx/2;
  x0=xZ+tm.tmAveCharWidth*3+dx;
  x1=x0+dx;
  x2=x1+tm.tmAveCharWidth*8+dx;
  x3=x2+tm.tmAveCharWidth*4+dx;
  x4=x3+dc.GetTextExtent("cal").X()+dx;
  */
}
BOOL StandardListDialog::ValidateControls(void *data)
{
  return Inherited::ValidateControls(data) && (list->GetSelIndex()>=0);
}
void StandardListDialog::AcceptControls(void *data)
{
  int *nStandard=(int *)data;
  *nStandard=list->GetSelIndex();
}

#if 0
void StandardListDialog::WMMeasureItem(TMessage& Msg)
{
  MEASUREITEMSTRUCT FAR *lpmisCtl = (MEASUREITEMSTRUCT FAR*) Msg.LParam;
  if(lpmisCtl->CtlID==IDD_RUNLIST_LABEL) {
    return;
  }
  //lpmisCtl->itemWidth=r.Width()/((nPict+nRow/2)/nRow);
  TEXTMETRIC tm;
  ClientDC(HWindow).GetTextMetrics(&tm);

  lpmisCtl->itemHeight=tm.tmHeight+tm.tmExternalLeading;
}

void StandardListDialog::DrawItem(DRAWITEMSTRUCT _FAR & DrawInfo)
{
  DC dc(DrawInfo.hDC);
  WRect r(DrawInfo.rcItem);
  if(DrawInfo.CtlID==IDD_RUNLIST_LABEL) {
    {
      HBRUSH hBrush=(HBRUSH)SendMessage(HWindow,WM_CTLCOLOR,(WPARAM)DrawInfo.hDC,MAKELPARAM(DrawInfo.hwndItem,
	/*l? CTLCOLOR_LISTBOX:*/ CTLCOLOR_STATIC));
      if(hBrush) dc.FillRect(r,Brush(hBrush));
    }
    SelBkMode oldBkMode(dc,bkmTransparent);
    SelTextAlign oldTextAlign(dc,taLeft|taTop);
    dc.TextOut(r.Left()+x1,r.Top(),"Date");
    dc.TextOut(r.Left()+x2,r.Top(),"Time");
    dc.TextOut(r.Left()+x4,r.Top(),"Comment");
    return;
  }
  Color bkColor;
  Color color;
  if(DrawInfo.itemState&ODS_SELECTED) {		// Reflect selection state
    bkColor=GetSysColor(COLOR_HIGHLIGHT);
    color=GetSysColor(COLOR_HIGHLIGHTTEXT);
  }
  else {
    bkColor=GetSysColor(COLOR_WINDOW);
    color=GetSysColor(COLOR_WINDOWTEXT);
  }
  dc.FillRect(r,LogBrush(bkColor));

  r.Inflate( -1, -1 );

  if(int(DrawInfo.itemID)>=0 && /*DrawInfo.itemAction&ODA_DRAWENTIRE*/1) {	// Redraw if neccesary
    //StandardList::Node *prln=(StandardList::Node *)DrawInfo.itemData;
    StandardList::Node *prln=pStandardList->GetChild(DrawInfo.itemID);
    if(prln) {
      SelTextColor oldTextColor(dc,color);
      SelBkMode oldBkMode(dc,bkmTransparent);
      SelTextAlign oldTextAlign(dc,taLeft|taBottom);
      Run *pRun=(Run *)SendMessage(Parent->HWindow,WM_GET_RUN,0,0L);
      //dc.TextOut(r.Left()+xZ,r.Bottom(), pRun==prln? "*": "");
      if(pRun==prln) {
	SelBkColor oldBkColor(dc,bkColor);
	StockBitmap bmpCheck(OBM_CHECK);
	WPoint pt=bmpCheck.Size( dc );
	bmpCheck.BitBlt(dc,r.Left()+xZ,r.Top(), pt.x, pt.y, 0, 0, SRCCOPY);
      }
      {
      SelTextAlign oldTextAlign(dc,taRight|taBottom);
      char str[12]; wsprintf(str, "%d",1+DrawInfo.itemID);
      dc.TextOut(r.Left()+x0,r.Bottom(),str);
      }

      dc.TextOut(r.Left()+x1,r.Bottom(),PrintDate(prln->start_time));
      dc.TextOut(r.Left()+x2,r.Bottom(),PrintTime(prln->start_time));
      dc.TextOut(r.Left()+x3,r.Bottom(),prln->type_of_run==CAL? "cal": ""); // space added. P.P. 05/31/13 and rem 11/01/13
      Str comment=prln->comment;
      const char *cp=strchr(comment,'\r'); if(cp) *(char *)cp=0;
      dc.TextOut(r.Left()+x4,r.Bottom(),comment);
    }
  }

  r.Inflate( +1, +1 );

  if(DrawInfo.itemState&ODS_FOCUS)		// Reflect focus state
    dc.FrameRect( r, StockBrush(sbiBlack) );
}

#endif

void StandardListDialog::HandleList(RTMessage Msg)
{
  switch(Msg.LP.Hi) {
    case LBN_SELCHANGE:
    break;
    case LBN_DBLCLK:
    {
      CloseWindow(IDOK);
    }
    break;
  }
}



PTWindowsObject newStandardListDialog(PTWindowsObject AParent, int ResourceId, int *nStandard, PTModule AModule)
{
  return new StandardListDialog(AParent, ResourceId, nStandard, AModule);
}

