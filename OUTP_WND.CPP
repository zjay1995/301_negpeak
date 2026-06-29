#include "includes.h"
#pragma hdrstop

#ifndef __BWINDOW_H
#include <bwindow.h>
#endif
#ifndef __EDIT_H
#include <edit.h>
#endif
#ifndef __STDIO_H
#include <stdio.h>
#endif
#ifndef __STDARG_H
#include <stdarg.h>
#endif

#ifndef  _WCPPLIB_STR_H
#include "wcpplib\str.h"
#endif
#ifndef  _WINLIB_DC_H
#include "winlib\dc.h"
#endif
#ifndef  _WINLIB_MENU_H
#include "winlib\menu.h"
#endif
#ifndef  _WINLIB_FONT_H
#include "winlib\font.h"
#endif
#ifndef  _WINUTIL_WUTIL_H
#include "winutil\wutil.h"
#endif

#ifndef  _ID_H
#include "id.h"
#endif
#ifndef  _OUTPWIND_H
#include "outpwind.h"
#endif
#ifndef  _PEAK_APP_H
#include "peak_app.h"
#endif


LPSTR OutputWindow::GetClassName()
{
  return name;
}

void OutputWindow::GetWindowClass(WNDCLASS& WndClass)
{
  Inherited::GetWindowClass(WndClass);
  WndClass.hIcon = LoadIcon(GetModule()->hInstance,"EDITTEXT");
}


OutputWindow::OutputWindow(PTWindowsObject AParent,PTModule AModule)
  : TBWindow(AParent,"Output",AModule),
    Editor(new TEdit(this, ID_OUTPUT_EDIT, NULL, 0, 0, 0, 0, 0, TRUE, AModule))
{
  Attr.Style |= WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE;
  if(Editor) {
    Editor->Attr.Style&=~(WS_BORDER);
    Editor->Attr.Style|=ES_NOHIDESEL|WS_CHILD|WS_VISIBLE;
  }
  StockFont(sfiANSIVar).Get(&lfUser);
  lfUser.lfHeight=16;
}

OutputWindow::~OutputWindow(void)
{
}

void OutputWindow::SetupWindow()
{
  Inherited::SetupWindow();
}

OutputWindow::OutputWindow(StreamableInit)
 : TBWindow(streamableInit), Editor(0)
{
};


void OutputWindow::vprint(UINT fmtID, va_list arglist)
{
  Str fmt(1024);
  if(LoadString(GetModule()->hInstance, fmtID, fmt.GetPtr(), 1024)) {
    vprint(fmt,arglist);
  }
}
void OutputWindow::vprint(LPCSTR fmt, va_list arglist)
{
  if(Editor) {
    Str buff(1024);
    LPSTR buf=buff.GetPtr();
    if(buf) {
      vsprintf(buf,fmt,arglist);
      int l=Editor->GetTextLen();
      if(l>30000) {
	//Editor->SetSelection(0,-1); // All
	Editor->SetSelection(0,300);
	Editor->Insert("");
	l=Editor->GetTextLen();
      }
      Editor->SetSelection(l,l);
      Editor->Insert(buf);
    }
  }
}

void OutputWindow::print(UINT fmtID, ... )
{
  vprint(fmtID,...);
}
void OutputWindow::print(LPCSTR fmt, ... )
{
  vprint(fmt,...);
}



TStreamableClass RegOutputWindow(OutputWindow::name, OutputWindow::build, __DELTA(OutputWindow));
char far OutputWindow::name[]="OutputWindow";
PTStreamable OutputWindow::build()
{
  return new OutputWindow(streamableInit);
}

void OutputWindow::write(Ropstream os)
{
  Inherited::write(os);
  os<<TextColor;
  os.writeBytes(&lfUser,sizeof(lfUser));
  PutChildPtr(os,Editor);
}

Pvoid OutputWindow::read(Ripstream is)
{
  if(!Inherited::read(is)) return NULL;
  is>>TextColor;
  is.readBytes(&lfUser,sizeof(lfUser));
  GetChildPtr(is,(RPTWindowsObject)Editor);
  return this;
}

void OutputWindow::WMIsItYourName(RTMessage Msg)
{
  Msg.Result=lstrcmpi(GetClassName(),(LPCSTR)Msg.LParam)==0;
}

void OutputWindow::WMSize(RTMessage Msg)
{
  Inherited::WMSize(Msg);
  if( (Msg.WParam != SIZEICONIC) && Editor) {
    WRect rClient;    ::GetClientRect(*this,&rClient);
    ::MoveWindow(*Editor,rClient,TRUE);
  }
}

void OutputWindow::WMSetFocus(RTMessage)
{
  SetFocus(Editor->HWindow);
}


class OutputPrintout : public Printout
{
  typedef Printout Inherited;
 protected:
  PTEdit Editor;
  int hLine;
  int LinesPerPage;
  int NumLines;
 public:
  OutputPrintout(LPCSTR ATitle, PTEdit _Editor) : Printout(ATitle), Editor(_Editor) {}
  virtual void SetPrintParams(DC *_pdc, WPoint pageSize)
  {
    Inherited::SetPrintParams(_pdc,pageSize);
    hLine=pdc->GetTextExtent("H").Y();
    if(!hLine || (LinesPerPage=PageSize.Y()/hLine)==0) LinesPerPage=1;
    NumLines = Editor->GetNumLines();
    /*
    WPoint pt;
    pdc->Escape(GETPHYSPAGESIZE, NULL, NULL, (LPVOID)&pt);
    pdc->Escape(GETPRINTINGOFFSET, NULL, NULL, (LPVOID)&pt);
    */
  }

  virtual BOOL HasPage(int pageNumber)
  {
    return (pageNumber-1)*LinesPerPage<NumLines;
  }
  virtual void GetDialogInfo(UINT *minPage, UINT *maxPage, UINT *selFromPage, UINT *selToPage);
  virtual void PrintPage(int page, WRect& rect, unsigned flags);
};

void OutputPrintout::GetDialogInfo(UINT *minPage, UINT *maxPage, UINT *selFromPage, UINT *selToPage)
{
  Inherited::GetDialogInfo(minPage, maxPage, selFromPage, selToPage);
  int StartPos, EndPos;
  Editor->GetSelection(StartPos, EndPos);
  if((unsigned)StartPos<(unsigned)EndPos) {
    *selFromPage=1;
    *selToPage=0x7FFF;
  }
}

void GetPage(DC &ic,WRect *rect,const WPoint &size);
void OutputPrintout::PrintPage(int page, WRect&/*_rect*/, unsigned /*flags*/)
{
  WRect rect;
  GetPage(*pdc,&rect,PageSize);
  pdc->IntersectClipRect(rect);
  char buff[256];
  int l=0;
  for(int i=(page-1)*LinesPerPage; i<NumLines && i< page*LinesPerPage; i++) {
    Editor->GetLine(buff, 256, i);
    pdc->TextOut(rect.Left(),rect.Top()+hLine*l,buff,lstrlen(buff));
    l++;
  }
}


void OutputWindow::CMPrint(RTMessage)
{
  if(!Editor) return;
  PeakApp *pApp=((PeakApp *)GetApplication());
  Printer *pPrinter=pApp;
  if( pPrinter ) {
    OutputPrintout *pPrintout = new OutputPrintout(pApp->Name, Editor);
    if( pPrintout ) {
      pPrinter->Print(this, pPrintout, TRUE);
      delete pPrintout;
    }
  }
}

void OutputWindow::CMExport(RTMessage)
{
}

#ifndef __COMMDLG_H
#include <commdlg.h>
#endif

void OutputWindow::CMChangeFont(RTMessage)
{
  DC *dc=((PeakApp *)GetApplication())->GetDC(); // Printer DC

  CHOOSEFONT cf;
  LOGFONT lf=lfUser;
  memset(&cf, 0, sizeof(CHOOSEFONT));
  cf.lStructSize = sizeof(CHOOSEFONT);
  cf.hInstance = GetApplicationObject()->hInstance;
  cf.hwndOwner = HWindow;
  cf.hDC       = dc? (HDC)*dc: (HDC)0;
  cf.lpLogFont = &lf;
  cf.Flags = CF_SCREENFONTS | CF_EFFECTS | CF_INITTOLOGFONTSTRUCT | CF_PRINTERFONTS;
  cf.rgbColors = TextColor;

  //cf.Flags |= CF_ENABLEHOOK;
  //(FARPROC)cf.lpfnHook=MakeProcInstance((FARPROC)FontHook,GetApplicationObject()->hInstance);

  int resY=72; // 1 point == 1"/72
  int resScreenY=ScreenDC().GetCapability(LOGPIXELSY);
  cf.lpLogFont->lfHeight=MulDiv(cf.lpLogFont->lfHeight,resScreenY,resY);
  if(ChooseFont(&cf)) {
    InvalidateRect(HWindow,NULL,TRUE);
  }
  cf.lpLogFont->lfHeight=MulDiv(cf.lpLogFont->lfHeight,resY,resScreenY);
  //if(cf.lpfnHook) FreeProcInstance((FARPROC)cf.lpfnHook);

  lfUser=lf;
  TextColor=Color(cf.rgbColors);
}

#ifndef  _FRAME_H
#include "frame.h"
#endif

void OutputWindow::ActivationResponse(WORD Activated,  BOOL IsIconified)
{
  FrameWindow *pFrameWindow=(FrameWindow *)Parent;
  pFrameWindow->EnableCommand(CM_PRINT, Activated);
//  pFrameWindow->EnableCommand(CM_EXPORT, Activated);
//  pFrameWindow->EnableCommand(CM_CHANGEFONT, Activated);
  Inherited::ActivationResponse(Activated, IsIconified);
}


void OutputWindow::WMInitMenuPopup(RTMessage Msg)
{
  if(!Editor) return;
  Menu menu((HMENU)Msg.WParam);
  int StartPos,EndPos;
  Editor->GetSelection(StartPos,EndPos);
  BOOL bSel=StartPos<EndPos;
  BOOL bPaste=IsClipboardFormatAvailable(CF_TEXT);
  BOOL bCanUndo=Editor->CanUndo();
  BOOL bIsClear=Editor->GetNumLines()==0;
  menu.EnableItem(CM_EDITUNDO, bCanUndo? MF_ENABLED: MF_GRAYED);
  menu.EnableItem(CM_EDITCUT, bSel? MF_ENABLED: MF_GRAYED);
  menu.EnableItem(CM_EDITCOPY, bSel? MF_ENABLED: MF_GRAYED);
  menu.EnableItem(CM_EDITPASTE, bPaste? MF_ENABLED: MF_GRAYED);
  menu.EnableItem(CM_EDITDELETE, bSel? MF_ENABLED: MF_GRAYED);
  menu.EnableItem(CM_EDITCLEAR, bIsClear? MF_GRAYED: MF_ENABLED);

  menu.EnableItem(CM_EDITCOPYTOFILE, FALSE);
}

/*

Editor->Search(int StartPos, LPSTR AText, BOOL CaseSensitive);

*/




#ifndef  _OUTP_WND_H
#include "outp_wnd.h"
#endif


PTWindowsObject newOutputWindow(PTWindowsObject AParent, PTModule AModule)
{
  return new OutputWindow(AParent,AModule);
}

