#include "includes.h"
#pragma hdrstop

#ifndef __LISTBOX_H
#include <listbox.h>
#endif

#ifndef  _ID_H
#include "id.h"
#endif

#ifndef  _HELP_DLG_H
#include "help_dlg.h"
#endif
#ifndef  _WINUTIL_STATLINE_H
#include "winutil\statline.h"
#endif

#ifndef  _WINUTIL_WUTIL_H
#include "winutil\wutil.h"
#endif

extern UINT commdlg_help;
extern UINT commdlg_id;

HelpDlg::HelpDlg(PTWindowsObject AParent, int ResourceId, void *data, PTModule AModule)
 : TransferDlg(AParent, ResourceId, data, AModule)
 , sl(new StatusLine(this, AModule))
{
  commdlg_id=ResourceId;
}

HelpDlg::~HelpDlg()
{
  commdlg_id=0;
  destroy(sl);
}

void HelpDlg::SetupWindow()
{
  Inherited::SetupWindow();
  WRect r;    ::GetWindowRect(HWindow,&r);
  r.Bottom()+=20;
  MoveWindow(*this, r, FALSE);
  Centralize(this);
}

void HelpDlg::WMPaint(RTMessage Msg)
{
  if(sl) {
    /*PaintContext*/ClientDC pc(HWindow);
    sl->Paint(pc);
  }
  DefWndProc(Msg);
}

void HelpDlg::WMSize(RTMessage Msg)
{
  //Inherited::WMSize(Msg);
  if( sl && (Msg.WParam != SIZEICONIC) ) {
    WRect rClient;    ::GetClientRect(HWindow,&rClient);
    WRect r;
    r=rClient; r.Top()=r.Bottom()-20; /*MoveWindow(sl,  r,TRUE);*/sl->Size(r);
    //r=rClient; r.Bottom()=r.Bottom()-20; MoveWindow(*ClientWnd,  r,TRUE);
    UpdateWindow(HWindow);
  }
}


void HelpDlg::WMSetCursor(RTMessage Msg)
{
  if(sl) {
    POINT pt; GetCursorPos(&pt);   //=*(POINT *)&Msg.LParam;
    HWND hwnd=WindowFromPoint(pt); //ChildWindowFromPoint(HWindow, pt);
    if(hwnd && hwnd!=HWindow) {
      int id=GetDlgCtrlID(hwnd);
      char str[80];
      if(LoadString(GetModule()->hInstance,id,str,sizeof(str))) {
	sl->Replace(str);
      }
    }
  }
  DefWndProc(Msg);
}

void HelpDlg::WMCommand(RTMessage Msg)
{
  if(sl) {
    int id=GetDlgCtrlID((HWND)Msg.LP.Lo);
    char str[80];
    if(LoadString(GetModule()->hInstance,id,str,sizeof(str))) {
      sl->Replace(str);
    }
  }
  Inherited::WMCommand(Msg);
}

void HelpDlg::DMGetDefID(RTMessage Msg)
{
  if(sl) {
    int id=GetDlgCtrlID(::GetFocus());
    char str[80];
    if(LoadString(GetModule()->hInstance,id,str,sizeof(str))) {
      sl->Replace(str);
    }
  }
  DefWndProc(Msg);
}


void HelpDlg::Help(RTMessage)
{
  Help();
}
void HelpDlg::WMKeyUp(RTMessage Msg)
{
  UINT key=Msg.WParam;
  if(key==VK_F1) Help();
  DefWndProc(Msg);
}

#ifndef  _WINUTIL_APP_H
#include "winutil\app.h"
#endif


void HelpDlg::Help()
{
  char str[255];
  lstrcpy(str,"wPeak.hlp");
  MakeHostDirName(str);
  WinHelp(GetApplication()->MainWindow->HWindow,str,HELP_CONTEXT, LOWORD(Attr.Name));
}




