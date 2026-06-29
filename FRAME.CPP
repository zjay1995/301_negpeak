#include "includes.h"
#pragma hdrstop

#ifndef __OWL_H
#include <owl.h>
#endif

#ifndef  _WCPPLIB_MESSAGE_H
#include "wcpplib\message.h"	// message( const char far *, .... )
#endif

#ifndef  _WINLIB_MENU_H
#include "winlib\menu.h"
#endif

#ifndef  _WINUTIL_TOOLBAR_H
#include "winutil\toolbar.h"
#endif
#ifndef  _WINUTIL_STATLINE_H
#include "winutil\statline.h"
#endif
#ifndef  _WINUTIL_THINKING_H
#include "winutil\thinking.h"
#endif
#ifndef  _WINUTIL_APP_H
#include "winutil\app.h"	// MakeHostDirName()
#endif
#ifndef  _WINUTIL_WUTIL_H
#include "winutil\wutil.h"
#endif

#ifndef  _ID_H
#include "id.h"
#endif
#ifndef  _FRAME_H
#include "frame.h"
#endif

FrameWindow:: FrameWindow(LPSTR ATitle, int MenuId, LPSTR HelpFile, PTModule AModule)
 : TMDIFrame(ATitle, MenuId, AModule),
  sl(new StatusLine(this, AModule)),
  tb(new ToolBar(this, AModule))
{
  lstrcpy(szHelpFileName,HelpFile);
  Attr.Style|=WS_CLIPCHILDREN;
  Attr.Style&=~WS_VISIBLE; // Wait ShowWindow() with nCmdShow from WinMain()
}

FrameWindow::~FrameWindow()
{
  destroy(sl); sl=0;
  destroy(tb); tb=0;
}

void FrameWindow::InitClientWindow()
{
  Inherited::InitClientWindow();
  ClientWnd->Attr.Style|=WS_CLIPSIBLINGS;
}


void FrameWindow::SetupWindow()
{
  Inherited::SetupWindow();
  Thinking::Register(sl);
  MakeHostDirName(szHelpFileName);
}

void FrameWindow::WMDestroy(RTMessage Msg)
{
  WinHelp(HWindow,szHelpFileName,HELP_QUIT,0L);
  Thinking::Unregister();
  Inherited::WMDestroy(Msg);
}

void FrameWindow::WMClose(RTMessage Msg)
{
  Inherited::WMClose(Msg);
}

void FrameWindow::WMLButtonDown(RTMessage Msg)
{
  if(tb) tb->WMLButtonDown(Msg);
}

void FrameWindow::WMLButtonUp(RTMessage Msg)
{
  if(tb) tb->WMLButtonUp(Msg);
}

void FrameWindow::WMMouseMove(RTMessage Msg)
{
  if(tb) tb->WMMouseMove(Msg);
}


#ifndef  _WINLIB_PC_H
#include "winlib\pc.h"
#endif
void FrameWindow::Paint(HDC PaintDC, PAINTSTRUCT _FAR & PaintInfo)
{
  PC pc(PaintDC,&PaintInfo);
  if(sl) sl->Paint(pc);
  if(tb) tb->Paint(pc);
}

void FrameWindow::WMSize(RTMessage Msg)
{
  //Inherited::WMSize(Msg);
  if( Msg.WParam != SIZEICONIC ) {
    WRect rClient;    ::GetClientRect(HWindow,(LPRECT)&rClient);
    WRect r;
    if(sl) {
      r=rClient; r.Top()=r.Bottom()-20; sl->Size(r);
      rClient.Bottom()-=20;
    }
    if(tb) {
      r=rClient; r.Bottom()=r.Top()+25; tb->Size(r);
      rClient.Top()+=25;
    }
    MoveWindow(*ClientWnd,  rClient, TRUE);
    ::InvalidateRect(HWindow,NULL,FALSE);
    //::UpdateWindow(HWindow);
  }
}

void FrameWindow::WMInitMenuPopup(RTMessage Msg)
{
  Menu menu((HMENU)Msg.WParam);
  PTWindowsObject pActive=ActiveChild();
  if(pActive) {
    pActive->DispatchAMessage(WM_FIRST + Msg.Message, Msg, &TWindowsObject::DefWndProc);
    menu.EnableItem(CM_TILECHILDRENHORIZ, MF_ENABLED/*: MF_DISABLED*/);
  }
}

void FrameWindow::WMMenuSelect(RTMessage Msg)
{
  if(!sl) return;
  static BOOL bFirst=TRUE;
  WORD wIDItem = Msg.WParam;      // item identifier or menu handle
  WORD fwMenu = Msg.LP.Lo;        // menu flags
  HMENU hmenu = (HMENU)Msg.LP.Hi; // handle of the menu
  int CurrentID;
  HMENU CurrentPopup;
  if(fwMenu == 0x0FFFF) {
    CurrentPopup = 0;
    CurrentID = 0;
    bFirst = TRUE;
    sl->Restore();
    return;
  }
  else if(fwMenu & MF_POPUP) {
    if(fwMenu & MF_SYSMENU) {
      CurrentPopup = hmenu;
    }
    else {
      CurrentPopup = (HMENU)wIDItem;
    }
    CurrentID = 0;
  }
  else if(fwMenu & MF_SEPARATOR) {
    CurrentID = 0;
    CurrentPopup = hmenu;
  }
  else
    CurrentID = wIDItem;

  {
    int StrID=0;
    HWND hwndFrame=GetApplication()->MainWindow->HWindow;
    HWND CurrentWindow=Msg.Receiver;
    if(CurrentID) {
      StrID=CurrentID;
      if(CurrentWindow!=hwndFrame) switch(CurrentID) {
	case SC_RESTORE:      StrID = IDS_MDIRESTORE; break;
	case SC_MOVE:         StrID = IDS_MDIMOVE;    break;
	case SC_SIZE:         StrID = IDS_MDISIZE;    break;
	case SC_MINIMIZE:     StrID = IDS_MDIMINIMIZE; break;
	case SC_MAXIMIZE:     StrID = IDS_MDIMAXIMIZE; break;
	case SC_CLOSE:        StrID = IDS_MDICLOSE;   break;
	case SC_NEXTWINDOW:   StrID = IDS_MDINEXT;    break;
	case SC_PREVWINDOW:   StrID = IDS_MDIPREV;    break;
      }
    }
    else if(CurrentPopup) {
      if(((int)CurrentPopup)<0) StrID = 0;//CurrentPopup;
      else {
	UINT id=GetMenuItemID(CurrentPopup, 0);
	if(id==SystemFirst) {
	  StrID = IDS_SYSTEM;
	  if(CurrentWindow!=hwndFrame)
	    StrID = IDS_MDISYSTEM;
	}
	else {
	  StrID=GetMainMenuString(id);
	}
      }
    }
    char Str[128];
    if(!LoadString((HINSTANCE)GetWindowWord(HWindow,GWW_HINSTANCE), StrID, Str, sizeof(Str)-1)) {
      *Str=0;
    }
    if(bFirst) sl->Put(Str);
    else sl->Replace(Str);
    bFirst = FALSE;
  }
}

const unsigned long dsk_file_label=0x77554E48L;
const unsigned long dsk_file_version=8;


/*
#include "wow.h"
static WOWDLL kernel32("kernel32");
*/
void FrameWindow::SaveDesktop(const char *DskFile)
{
  //return; // CODEGUARD
  Thinking think("Saving Desktop File %s ...",DskFile);

  // Cannot write .WND file to dir>8 chars long
  Str _DskFile;
  /*
  lstrcpy(_DskFile.Alloc(256),DskFile);

  static DWORD dwGetShortPathName = kernel32.GetProcAddress32W("GetShortPathNameA");
  if(dwGetShortPathName) {
    kernel32.CallProcEx32W(3|CPEX_DEST_STDCALL,0x1|0x2,dwGetShortPathName,_DskFile.GetPtr(),_DskFile.GetPtr(),255);
  }

  AnsiToOem(_DskFile.GetPtr(),_DskFile.GetPtr());
  */
  AnsiToOem(DskFile,_DskFile.Alloc(256));
  unlink(_DskFile);
  ofpstream os(_DskFile);

  os<<dsk_file_label;
  os<<dsk_file_version;
  PutChildren(os);
  SaveDesktopInfo(os);
  os.close();

  if(!os.good()) {
    unlink(_DskFile);
    message("Unable to write desktop file %s.",DskFile);
  }
}

static void DoCreateChild(Pvoid P, Pvoid p)
{
  if ( ((PTWindowsObject)P)->IsFlagSet(WB_AUTOCREATE) ) {
    LPCSTR pClassName=((PTWindowsObject)P)->nameOf();
    if(0==strcmp("TDialog",pClassName)) {
      delete ((PTWindowsObject)P);
      return;
    }
    ((PTWindow)P)->Attr.X-=((LPPOINT)p)->x;
    ((PTWindow)P)->Attr.Y-=((LPPOINT)p)->y;
    ((PTWindowsObject)P)->Create();
  }
}

/* Read windows positions and contents from the "desk top" file. */
int FrameWindow::RestoreDesktop(const char *DskFile)
{
  //return 0; // CODEGUARD
  Thinking think("Loading Desktop File %s ...",DskFile);
  LPSTR ErrorMsg = NULL;

  Str _DskFile;
  AnsiToOem(DskFile,_DskFile.Alloc(256));
  ifpstream is(_DskFile);

  if( is.good() ) {
    unsigned long label;   is>>label;   if(label  !=dsk_file_label)   return -1;
    unsigned long version; is>>version; if(version!=dsk_file_version) return -1;
    if ( CloseChildren() ) {
      GetChildren(is);
      if ( !is.good() )
	ErrorMsg = "Error Reading Desktop File %s.";
      if ( GetApplication()->LowMemory() ) {
	CloseChildren();
	ErrorMsg = "Not enough memory to open Desktop File %s.";
      }
      else {
	POINT ptShift;
	ptShift.x=ClientWnd->Attr.X;
	ptShift.y=ClientWnd->Attr.Y;
	ForEach(DoCreateChild, (LPPOINT)&ptShift);
	RestoreDesktopInfo(is);
	return 0; // OK
      }
    }
  }
  if(ErrorMsg) message(ErrorMsg,DskFile);
  return -1;
}

struct rec {
  UINT   message;
  WPARAM wParam;
  LPARAM lParam;
};

static BOOL HasClassName(void *P, void *data)
{
  rec *p=(rec *)data;
  if(((PTWindowsObject)P)->IsFlagSet(WB_MDICHILD))
    return (BOOL)SendMessage(((PTWindowsObject)P)->HWindow,p->message,p->wParam,p->lParam);
  else
    return FALSE;
}

PTWindowsObject FrameWindow::FindChild(LPCSTR ClassName, WPARAM wParam)
{
  rec r;
  r.message=WM_IS_IT_YOUR_NAME;
  r.wParam=wParam;
  r.lParam=(LPARAM)ClassName;
  return FirstThat(HasClassName,(Pvoid)&r);
}

static BOOL IsActive(void *P, void *)
{
  PTWindowsObject p=(PTWindowsObject)P;
  if(p->IsFlagSet(WB_MDICHILD)) {
    if(IsIconic(p->HWindow)) return FALSE;
    HWND hwndFocus = GetFocus();
    if(!hwndFocus) return TRUE;
    if(hwndFocus==p->HWindow) return TRUE;
    if((HWND)GetWindowWord(hwndFocus,GWW_HWNDPARENT)==p->HWindow) return TRUE;
  }
  return FALSE;
}

PTWindowsObject FrameWindow::ActiveChild()
{
  return FirstThat(IsActive,0);
}

struct CNP {
  LPCSTR ClassName;
  TActionFunc Action;
  Pvoid PParamList;
};

static void ActClassName(void *P, void *PParamList)
{
  CNP *cnp = (CNP *)PParamList;
  if(HasClassName(P,(void *)cnp->ClassName)) {
    cnp->Action(P,cnp->PParamList);
  }
}

void FrameWindow::ForChild(LPCSTR ClassName, TActionFunc Action, Pvoid PParamList)
{
  CNP cnp = { ClassName, Action, PParamList };
  ForEach(ActClassName,&cnp);
}


void FrameWindow::EnableCommand(int id, int bEnable)
{
  Menu menu(GetMenu(HWindow));
  menu.EnableItem(id, bEnable? MF_ENABLED: MF_GRAYED);
  if(tb) tb->EnableCommand(id,bEnable);
}

void FrameWindow::CMTileChildrenHoriz(RTMessage )
{
  TMDIClient *pMDIClient=GetClient();
  if(pMDIClient) {
    SendMessage(pMDIClient->HWindow, WM_MDITILE, MDITILE_HORIZONTAL, 0);
  }
}
