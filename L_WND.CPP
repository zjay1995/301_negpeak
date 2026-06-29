#include "includes.h"
#pragma hdrstop

#error not used

#ifndef __WINDOW_H
#include <window.h>
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
#ifndef  _WINLIB_MENU_H
#include "winlib\menu.h"
#endif
#ifndef  _WINUTIL_WUTIL_H
#include "winutil\wutil.h"
#endif

#ifndef  _ID_H
#include "id.h"
#endif
#ifndef  _L_WND_H
#include "l_wnd.h"
#endif

class ListBox : public TListBox {
  typedef TListBox Inherited;
 public:
  ListBox(PTWindowsObject AParent, int AnId, int X, int Y, int W, int H, PTModule AModule = NULL)
    : TListBox(AParent, AnId, X, Y, W, H, AModule ) {}
  ListBox(PTWindowsObject AParent, int ResourceId, PTModule AModule = NULL)
    : TListBox(AParent, ResourceId, AModule ) {}
  virtual void WMChar(RTMessage Msg) = [WM_FIRST + WM_CHAR]
  {
    switch(Msg.WParam) {
      case VK_RETURN:
	PostMessage(Parent->HWindow, WM_COMMAND, CM_RUNLIST_EDITITEM, 0L);
      break;
      case VK_INSERT:
	PostMessage(Parent->HWindow, WM_COMMAND, CM_RUNLIST_ADDITEM, 0L);
      break;
      case VK_DELETE:
	PostMessage(Parent->HWindow, WM_COMMAND, CM_RUNLIST_DELITEM, 0L);
      break;
    }
    DefWndProc(Msg);
  }
};



ListWindow::ListWindow(PTWindowsObject AParent,char far *ATitle,PTModule AModule)
  : TWindow(AParent,ATitle,AModule),
    list(new ListBox(this, IDD_PEAKLIST, 0, 0, 0, 0, AModule))
{
  Attr.Style |= WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE;
  if(list) {
    list->Attr.Style&=~(WS_BORDER|LBS_SORT);
    list->Attr.Style|=LBS_NOTIFY|LBS_OWNERDRAWFIXED|/*LBS_HASSTRINGS|*/LBS_NOINTEGRALHEIGHT|WS_VSCROLL|WS_CHILD|WS_VISIBLE|WS_GROUP|WS_TABSTOP;
  }
  SetHeaderHeight();
}

ListWindow::~ListWindow(void)
{
}

void ListWindow::SetupWindow()
{
  Inherited::SetupWindow();
  FillList();
}

ListWindow::ListWindow(StreamableInit)
 : TWindow(streamableInit), list(0)
{
  SetHeaderHeight();
};


void ListWindow::write(Ropstream os)
{
  Inherited::write(os);
  PutChildPtr(os,list);
}

Pvoid ListWindow::read(Ripstream is)
{
  if(!Inherited::read(is)) return NULL;
  GetChildPtr(is,(RPTWindowsObject)list);
  return this;
}

void ListWindow::SetHeaderHeight()
{
  TEXTMETRIC tm;
  ScreenDC().GetTextMetrics(&tm);
  HeaderHeight=tm.tmHeight+tm.tmExternalLeading;
}

void ListWindow::FillList()
{
}

void ListWindow::WMIsItYourName(RTMessage Msg)
{
  Msg.Result=lstrcmpi(GetClassName(),(LPCSTR)Msg.LParam)==0;
}

void ListWindow::WMRefresh(RTMessage Msg)
{
  FillList();
}

void ListWindow::WMMeasureItem(TMessage& Msg)
{
  MEASUREITEMSTRUCT FAR *lpmisCtl = (MEASUREITEMSTRUCT FAR*) Msg.LParam;
  lpmisCtl->itemHeight=HeaderHeight-2;
}


void ListWindow::WMSetFocus(RTMessage)
{
  SetFocus(list->HWindow);
}

void ListWindow::WMSize(RTMessage Msg)
{
  Inherited::WMSize(Msg);
  if( (Msg.WParam != SIZEICONIC) && list) {
    WRect rClient;    ::GetClientRect(*this,&rClient);
    rClient.Top()+=HeaderHeight;
    ::MoveWindow(*list,rClient,TRUE);
  }
}

void ListWindow::WMInitMenuPopup(RTMessage Msg)
{
  Menu menu((HMENU)Msg.WParam);
  menu.EnableItem(CM_PEAK_ADDITEM, MF_ENABLED);
  if(list && list->GetCount() && list->GetSelIndex()>=0) {
    menu.EnableItem(CM_PEAK_DELITEM, MF_ENABLED);
  }
}

void ListWindow::Paint(HDC PaintDC, PAINTSTRUCT _FAR & PaintInfo)
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
    LogPen penDarkGray(psSolid, 0, DarkGray);
    SelPen oldPen(pc,penDarkGray);
    pc.MoveTo(r.B()); pc.LineTo(r.Right(),r.Top());
    pc.MoveTo(r.B()); pc.LineTo(r.Left(),r.Bottom());
  }
  SelBkMode oldBkMode(pc,bkmTransparent);
  SelTextAlign oldTextAlign(pc,taLeft|taBottom);

  DRAWITEMSTRUCT DrawInfo;
  DrawInfo.CtlType=ODT_LISTBOX;
  DrawInfo.CtlID=IDD_PEAKLIST;
  DrawInfo.itemID=0x7FFF;
  DrawInfo.itemAction=ODA_DRAWENTIRE;
  DrawInfo.itemState=0;
  DrawInfo.hwndItem=GetDlgItem(HWindow,DrawInfo.CtlID);
  DrawInfo.hDC=pc;
  DrawInfo.rcItem=r;
  DrawInfo.itemData=0;

  DrawItem(DrawInfo);
}

void ListWindow::WMCommand(RTMessage Msg)
{
  switch(Msg.LP.Hi) {
    case LBN_SELCHANGE:
    break;
  }
  Inherited::WMCommand(Msg);
}

void ListWindow::HandleList(RTMessage Msg)
{
  switch(Msg.LP.Hi) {
    case LBN_DBLCLK:
      int Index=list->GetSelIndex();
//      GetModule()->MakeWindow(newDIBWindow(Parent,(LPSTR)pList->GetChild(Index)->Get()));
    break;
  }
}




