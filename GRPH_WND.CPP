#include "includes.h"
#pragma hdrstop

#ifndef __OWL_H
#include <owl.h>
#endif
#ifndef __COMMDLG_H
#include <commdlg.h>
#endif

#ifndef  _WINLIB_MENU_H
#include "winlib\menu.h"
#endif
#ifndef  _WINLIB_FONT_H
#include "winlib\font.h"
#endif

#ifndef  _GRPHWIND_H
#include "grphwind.h"
#endif

#ifndef  _JOB_H
#include "job.h"
#endif
#ifndef  _RUN_H
#include "run.h"
#endif
#ifndef  _ID_H
#include "id.h"
#endif
#ifndef  _PEAK_APP_H
#include "peak_app.h"
#endif

#ifndef  _DRAW_H
#include "draw.h"
#endif


#ifndef  _WINUTIL_THINKING_H
#include "winutil\thinking.h"
#endif

#ifndef  _TD_PLUS_H
#include "td_plus.h"
#endif
#ifndef  _WINUTIL_WUTIL_H
#include "winutil\wutil.h"
#endif

LPSTR GraphWindow::GetClassName()
{
  return name;
}

void GraphWindow::GetWindowClass(WNDCLASS& WndClass)
{
  Inherited::GetWindowClass(WndClass);
  WndClass.hIcon = LoadIcon(GetModule()->hInstance,MAKEINTRESOURCE(ID_WPEAK_ICON));
}


GraphWindow::GraphWindow(PTWindowsObject AParent, int _num, LPSTR ATitle, PTModule AModule)
 :TWindow(AParent, ATitle, AModule),
 num(_num)
{
}

GraphWindow::~GraphWindow()
{
}


void GraphWindow::SetupWindow()
{
  Inherited::SetupWindow();
}

GraphWindow::GraphWindow(StreamableInit)
 : TWindow(streamableInit)
{
};

TStreamableClass RegGraphWindow(GraphWindow::name, GraphWindow::build, __DELTA(GraphWindow));
char far GraphWindow::name[]="GraphWindow";
PTStreamable GraphWindow::build()
{
  return new GraphWindow(streamableInit);
}

void GraphWindow::write(Ropstream os)
{
  Inherited::write(os);
  os<<num;
}

Pvoid GraphWindow::read(Ripstream is)
{
  if(!Inherited::read(is)) return NULL;
  is>>num;
  return this;
}

void GraphWindow::WMDestroy(RTMessage Msg)
{
  Inherited::WMDestroy(Msg);
}

#ifndef  _PEAK_APP_H
#include "peak_app.h"
#endif
#ifndef  _WINLIB_DC_H
#include "winlib\dc.h"
#endif

class GraphPrintout : public Printout
{
  typedef Printout Inherited;
 protected:
  GraphWindow *pGraphWindow;
  Job *pJob;
  Run *pRun;
  PeakList **pPeakList;
  PictInfo **pPictInfo;
  int num;
 public:
  GraphPrintout(LPSTR ATitle, Job *_pJob, Run *_pRun, PeakList **_pPeakList, PictInfo **_pPictInfo, int _num)
   : Printout(ATitle), pJob(_pJob), pRun(_pRun), pPeakList(_pPeakList), pPictInfo(_pPictInfo), num(_num) {}
  virtual void PrintPage(int page, WRect& rect, unsigned flags);
};
void GetPage(DC &ic,WRect *rect,const WPoint &size);
void GraphPrintout::PrintPage(int page, WRect& /*rect*/, unsigned flags)
{
  if(pJob) {
    if(pRun) {
      if(
	#ifdef FILTER
	pJob->GetFiltData(pRun,num) ||
	#endif//FILTER
	pJob->GetRawData(pRun)) {
	WRect rc;
	GetPage(*pdc,&rc,PageSize);
	pdc->IntersectClipRect(rc);
	::Draw(pJob, pRun, pPictInfo,*pdc, *pdc, rc, pRun->runrawdata,
	  #ifdef FILTER
	  pRun->filtData,
	  #endif//FILTER
	  num, pPeakList, 0);
      }
    }
  }
}


void GraphWindow::CMPrint(RTMessage)
{
  PeakApp *pApp=((PeakApp *)GetApplication());
  Printer *pPrinter=pApp;
  if( pPrinter ) {
    Job *pJob=(Job *)SendMessage(Parent->HWindow,WM_GET_JOB,0,0L);
    Run *pRun=(Run *)SendMessage(Parent->HWindow,WM_GET_RUN,0,0L);
    PeakList **pPeakList=(PeakList **)SendMessage(Parent->HWindow,WM_GET_PEAKLIST,0,0L);
    PictInfo **pPictInfo=(PictInfo **)SendMessage(Parent->HWindow,WM_GET_PICTINFO,0,0L);
    GraphPrintout *pPrintout = new GraphPrintout(pApp->Name, pJob, pRun, pPeakList, pPictInfo, num);
    if( pPrintout ) {
      pPrinter->Print(this, pPrintout, TRUE);
      delete pPrintout;
    }
  }
}
#ifndef  _WINLIB_METAFILE_H
#include "winlib\metafile.h"
#endif

MetafileDC *CreateMetafile(DC *infoDC,Job *pJob,Run *pRun,PeakList **pPeakList,PictInfo **pPictInfo, int num, WRect *rect)
{
  int xSize=6; //6"
  int ySize=5; //5"
  WRect rc(0,0,2540*xSize,2540*ySize);

  MetafileDC *dc=new MetafileDC;
  dc->SetWindow(rc); // Viewport must be set before playing

  static char const far Comment[]="This picture is produced by WPeak program.";
  dc->Escape(MFCOMMENT, lstrlen(Comment), Comment);

  if(
    #ifdef FILTER
    pJob->GetFiltData(pRun,num) ||
    #endif//FILTER
    pJob->GetRawData(pRun)) {
    Thinking think;
    if(infoDC) {
      DC &ic=*infoDC;
      SelMapMode oldMapMode(ic,mmAnisotropic);
      ic.SetWindow(rc);
      int resX=ic.GetCapability(LOGPIXELSX);
      int resY=ic.GetCapability(LOGPIXELSY);
      ic.SetViewport(WRect(0,0,resX*xSize,resY*ySize));
      ::Draw(pJob, pRun, pPictInfo, *dc, ic, rc, pRun->runrawdata,
	#ifdef FILTER
	pRun->filtData,
	#endif//FILTER
	num, pPeakList, 0);
    }
  }
  *rect=rc;
  return dc;
}


MetafileDC *GraphWindow::CreateMetafile(WRect *rc)
{
  MetafileDC *dc=0;
  Job *pJob=(Job *)SendMessage(Parent->HWindow,WM_GET_JOB,0,0L);
  Run *pRun=(Run *)SendMessage(Parent->HWindow,WM_GET_RUN,0,0L);
  PeakList **pPeakList=(PeakList **)SendMessage(Parent->HWindow,WM_GET_PEAKLIST,0,0L);
  PictInfo **pPictInfo=(PictInfo **)SendMessage(Parent->HWindow,WM_GET_PICTINFO,0,0L);

  if(pJob && pRun) {
    DC *infoDC=0;
    PeakApp *pApp=((PeakApp *)GetApplication());
    Printer *pPrinter=pApp;
    if( pPrinter ) infoDC=pPrinter->GetDC(this,0);
    if(!infoDC) infoDC=new WindowDC(HWindow);

    dc=::CreateMetafile(infoDC,pJob,pRun,pPeakList,pPictInfo,num, rc);

    delete infoDC;
  }
  return dc;
}

void GraphWindow::CMEditCopy(RTMessage)
{
  WRect rc;
  MetafileDC *dc=CreateMetafile(&rc);

  if(dc && OpenClipboard(HWindow)) {
    EmptyClipboard();
    MetaFile mf(*dc);
    mf.SetClipboardData(rc,mmAnisotropic);

    CloseClipboard();
  }
  delete dc;
}

extern void LoadFilterString(HINSTANCE hinst, UINT idResource, LPSTR lpszBuffer, int cbBuffer);


#include "mainwind.h"
#include "wcpplib\fwstream.h"
void GraphWindow::CMEditCopyToFile(RTMessage)
{
  Run *pRun=(Run *)SendMessage(Parent->HWindow,WM_GET_RUN,0,0L);
  if(!pRun) return;

  HINSTANCE hInstance=GetApplication()->hInstance;
  Str strFilter(128);
  LoadFilterString(hInstance, IDS_WMF_FILTER, strFilter.GetPtr(), 128);
  Str strTitle(64);
  LoadString(hInstance, IDS_SAVEPICTURE, strTitle.GetPtr(), 64);
  Str FileName;

  BOOL ret=((MainWindow *)Parent)->OpenSaveDialog(HWindow, strTitle, strFilter, &FileName, TRUE);
  if(ret) {
    ofwstream os(FileName);

    WRect rc;
    MetafileDC *dc=CreateMetafile(&rc);

    if(dc && os.good()) {
      MetaFile mf(*dc);
      mf.Write(os,rc,mmAnisotropic);
    }
    delete dc;
  }
}


void GraphWindow::CMChangeFont(RTMessage)
{
  DC *dc=((PeakApp *)GetApplication())->GetDC(); // Printer DC
  PictInfo *pPictInfo=((PictInfo **)SendMessage(Parent->HWindow,WM_GET_PICTINFO,0,0L))[num];

  static far CHOOSEFONT cf;
  LOGFONT lf=pPictInfo->lfUser;
  memset(&cf, 0, sizeof(CHOOSEFONT));
  cf.lStructSize = sizeof(CHOOSEFONT);
  cf.hInstance = GetApplicationObject()->hInstance;
  cf.hwndOwner = HWindow;
  cf.hDC       = dc? (HDC)*dc: (HDC)0;
  cf.lpLogFont = &lf;
  cf.Flags = CF_SCREENFONTS | CF_EFFECTS | CF_INITTOLOGFONTSTRUCT | CF_PRINTERFONTS;
  cf.rgbColors = 0; //RGB(0, 255, 255); // light blue
  cf.rgbColors = pPictInfo->TextColor;

  //cf.Flags |= CF_ENABLEHOOK;
  //(FARPROC)cf.lpfnHook=MakeProcInstance((FARPROC)FontHook,GetApplicationObject()->hInstance);

  int resY=72;//dc.GetCapability(LOGPIXELSY);
  int resScreenY=ScreenDC().GetCapability(LOGPIXELSY);
  cf.lpLogFont->lfHeight=MulDiv(cf.lpLogFont->lfHeight,resScreenY,resY);
  if(ChooseFont(&cf)) InvalidateRect(HWindow, NULL, TRUE);
  cf.lpLogFont->lfHeight=MulDiv(cf.lpLogFont->lfHeight,resY,resScreenY);
  //if(cf.lpfnHook) FreeProcInstance((FARPROC)cf.lpfnHook);

  pPictInfo->lfUser=lf;
  pPictInfo->TextColor=Color(cf.rgbColors);
}
/*
struct AxisDialogInfo {
  long xmin,xmax;
  int kx;
  int xAuto;
  long ymin,ymax;
  int ky;
  int yAuto;
};
*/
static ControlInfo far AxisControlInfo[]={
  { citTIME,  IDD_XMIN,  FIELDOFFSET(AxisDialogInfo,xmin),  0, 0x7FFFFFFFL},
  { citTIME,  IDD_XMAX,  FIELDOFFSET(AxisDialogInfo,xmax),  0, 0x7FFFFFFFL},
  { citINT,   IDD_KX,    FIELDOFFSET(AxisDialogInfo,kx),  1, 50},
  { citONOFF, IDD_XAUTO, FIELDOFFSET(AxisDialogInfo,xAuto)},
  { citLONG,  IDD_YMIN,  FIELDOFFSET(AxisDialogInfo,ymin),  -0x7FFFFFFFL, 0x7FFFFFFFL}, // bipolar signal
  { citLONG,  IDD_YMAX,  FIELDOFFSET(AxisDialogInfo,ymax),  -0x7FFFFFFFL, 0x7FFFFFFFL},
  { citINT,   IDD_KY,    FIELDOFFSET(AxisDialogInfo,ky),  1, 50},
  { citONOFF, IDD_YAUTO, FIELDOFFSET(AxisDialogInfo,yAuto)},
  { citONOFF, IDD_SCALE, FIELDOFFSET(AxisDialogInfo,bScale)},  // added. P.P. 01/22/13
  { citNULL }	// End of array marker
};
class AxisDialog : public TransferPlusDlg {
  typedef TransferPlusDlg Inherited;
 public:
  AxisDialog(PTWindowsObject AParent, int ResourceId, char *Name, AxisDialogInfo *info, PTModule AModule = NULL)
   : TransferPlusDlg(AParent, ResourceId, info, AxisControlInfo, AModule)
  {
    SetCaption(Name);
  }
 protected:
  virtual void SetupWindow()
  {
    Inherited::SetupWindow();
    Centralize(this);
  }
};


void GraphWindow::CMAxisSetup(RTMessage)
{
  PictInfo *pPictInfo=((PictInfo **)SendMessage(Parent->HWindow,WM_GET_PICTINFO,0,0L))[num];
  char name[30];
  wsprintf(name,"Axes for Graph %c",'A'+num);
  AxisDialogInfo info;
  info.xmin=pPictInfo->ai[0].mi;
  info.xmax=pPictInfo->ai[0].ma;
  info.kx=pPictInfo->ai[0].k;
  info.xAuto=pPictInfo->ai[0].bAuto;
  info.ymin=pPictInfo->ai[1].mi;
  info.ymax=pPictInfo->ai[1].ma;
  info.ky=pPictInfo->ai[1].k;
  info.yAuto=pPictInfo->ai[1].bAuto;
  info.bScale=pPictInfo->ai[1].bScale; // P.P. 01/22/13
  int i=GetModule()->ExecDialog(new AxisDialog(this,ID_AXISDIALOG,name,&info));
  if(i==IDOK) {
    pPictInfo->ai[0].mi=info.xmin;
    pPictInfo->ai[0].ma=info.xmax;
    pPictInfo->ai[0].k=info.kx;
    pPictInfo->ai[0].bAuto=info.xAuto;
    pPictInfo->ai[1].mi=info.ymin;
    pPictInfo->ai[1].ma=info.ymax;
    pPictInfo->ai[1].k=info.ky;
    pPictInfo->ai[1].bAuto=info.yAuto;
    pPictInfo->ai[1].bScale=info.bScale;  // P.P. 01/22/13
    InvalidateRect(HWindow,NULL,TRUE);
  }
}

void GraphWindow::CMZoomIn(RTMessage)  // new function. P.P. 11/12/09
{
  PictInfo *pPictInfo=((PictInfo **)SendMessage(Parent->HWindow,WM_GET_PICTINFO,0,0L))[num];


  char name[30];
  wsprintf(name,"Zoom in Graph %c",'A'+num);
  AxisDialogInfo info;
  info.ymin=pPictInfo->ai[1].mi;
  info.ymax=pPictInfo->ai[1].ma;
  info.ky=pPictInfo->ai[1].k;
  info.yAuto=0;
    pPictInfo->ai[1].mi=info.ymin;
  if((info.ymin+(info.ymax - info.ymin)/2)>info.ymin) {
    info.ymax = info.ymin+(info.ymax - info.ymin)/2;
//  if((info.ymin+(info.ymax - info.ymin)/10)>info.ymin) {    // zoom increment = X10. P.P. 11/02/12  and back. 11/08/12
//    info.ymax = info.ymin+(info.ymax - info.ymin)/10;       //
  }
  pPictInfo->ai[1].ma=info.ymax;
  pPictInfo->ai[1].k=info.ky;
  pPictInfo->ai[1].bAuto=info.yAuto;
  InvalidateRect(HWindow,NULL,TRUE);

}

void GraphWindow::CMZoomOut(RTMessage)  // new function. P.P. 11/12/09
{
  PictInfo *pPictInfo=((PictInfo **)SendMessage(Parent->HWindow,WM_GET_PICTINFO,0,0L))[num];
  char name[30];
  wsprintf(name,"Zoom out Graph %c",'A'+num);
  AxisDialogInfo info;
  info.ymin=pPictInfo->ai[1].mi;
  info.ymax=pPictInfo->ai[1].ma;
  info.ky=pPictInfo->ai[1].k;
  info.yAuto=0;
  if((info.ymin + (info.ymax-info.ymin)*2) < 0x3FFFFFFFL) {
    info.ymax = info.ymin + (info.ymax-info.ymin)*2;
//  if((info.ymin + (info.ymax-info.ymin)*10) < 0x3FFFFFFFL) { // zoom decrement = /10. P.P. 11/02/12  and back. 11/08/12
//    info.ymax = info.ymin + (info.ymax-info.ymin)*10;
  }
  pPictInfo->ai[1].mi=info.ymin;
  pPictInfo->ai[1].ma=info.ymax;
  pPictInfo->ai[1].k=info.ky;
  pPictInfo->ai[1].bAuto=info.yAuto;
  InvalidateRect(HWindow,NULL,TRUE);
}

void GraphWindow::CMScaleIn(RTMessage)  // new function. P.P. 01/22/13
{
  PictInfo *pPictInfo=((PictInfo **)SendMessage(Parent->HWindow,WM_GET_PICTINFO,0,0L))[num];


  char name[30];
  wsprintf(name,"Scale peaks in Graph %c",'A'+num);
  AxisDialogInfo info;
  info.bScale=pPictInfo->ai[1].bScale;
  info.yAuto=1;
  info.bScale = 1; // P.P. 01/22/13
  pPictInfo->ai[1].bAuto=info.yAuto;
  pPictInfo->ai[1].bScale=info.bScale;
  InvalidateRect(HWindow,NULL,TRUE);

}

void GraphWindow::CMScaleOut(RTMessage)  // new function. P.P. 01/22/13
{
  PictInfo *pPictInfo=((PictInfo **)SendMessage(Parent->HWindow,WM_GET_PICTINFO,0,0L))[num];


  char name[30];
  wsprintf(name,"No scaling peaks in Graph %c",'A'+num);
  AxisDialogInfo info;
  info.bScale=pPictInfo->ai[1].bScale;
  info.yAuto=1;
  info.bScale = 0; // P.P. 01/22/13
  pPictInfo->ai[1].bAuto=info.yAuto;
  pPictInfo->ai[1].bScale=info.bScale;
  InvalidateRect(HWindow,NULL,TRUE);

}

#include <combobox.h>

class ColorComboBox : public TComboBox
{
  typedef TComboBox Inherited;
 public:
  ColorComboBox(TWindowsObject *AParent, int ResourceId, TModule *AModule = NULL);

  void FillList();
  BOOL GetSelected(COLORREF *color) const;
  void SetSelected(COLORREF color);

protected:
  virtual void SetupWindow();
  virtual void DrawItem(DRAWITEMSTRUCT far&);
  virtual void WMDrawItem(RTMessage Msg) = [WM_FIRST + WM_DRAWITEM];
private:
};
#define COLOR_NUMBER 16
static struct {
  const char * id;
}
_far color[] = {
    { "Black" },      { "Blue" },
    { "Green" },      { "Cyan" },
    { "Red" },        { "Magenta" },
    { "Brown" },      { "Dark Gray" },
    { "Light Gray" }, { "Light Blue" },
    { "Light Green" },{ "Light Cyan" },
    { "Light Red" },  { "Light Magenta" },
    { "Yellow" },     { "White" }
};
COLORREF DefColorPalette[COLOR_NUMBER]={
  RGB(0, 0, 0),
  RGB(0, 0, 0x80),
  RGB(0, 0x80, 0),
  RGB(0, 0x80, 0x80),
  RGB(0x80, 0, 0),
  RGB(0x80, 0, 0x80),
  RGB(0x80, 0x80, 0),
  RGB(0x80, 0x80, 0x80),
  RGB(0xC0, 0xC0, 0xC0),
  RGB(0, 0, 0xFF),
  RGB(0, 0xFF, 0),
  RGB(0, 0xFF, 0xFF),
  RGB(0xFF, 0, 0),
  RGB(0xFF, 0, 0xFF),
  RGB(0xFF, 0xFF, 0),
  RGB(0xFF, 0xFF, 0xFF)
};

ColorComboBox::ColorComboBox(TWindowsObject *AParent, int ResourceId, TModule *AModule)
  : TComboBox(AParent, ResourceId, 0, AModule)
{}

void ColorComboBox::SetupWindow()
{
  Inherited::SetupWindow();
}

void ColorComboBox::SetSelected(COLORREF col)
{
  int iSel = 0;
  for( int i = 0; i < COLOR_NUMBER; i++ ) {
    if(DefColorPalette[i] == col) {
      iSel = i;
      break;
    }
  }
  SetSelIndex(iSel);
}

void ColorComboBox::FillList()
{
  ClearList();
  for( int i = 0; i < COLOR_NUMBER; i++ ) {
    AddString("");
  }
}


BOOL ColorComboBox::GetSelected(COLORREF *color) const
{
  int iSel=GetSelIndex();
  if(iSel>=0) {
    *color=DefColorPalette[iSel];
    return FALSE;
  }
  *color=0;
  return FALSE; //Default
}

#include "winutil/wutil.h"
#include "winlib/brush.h"

void ColorComboBox::WMDrawItem(RTMessage Msg)
{
  DrawItem(*((LPDRAWITEMSTRUCT)(Msg.LParam)));
}

void ColorComboBox::DrawItem(DRAWITEMSTRUCT far &DrawInfo)
{
  Inherited::DrawItem(DrawInfo);

  DC gv(DrawInfo.hDC);
  if(!gv.IsValid()) return;
  if( DrawInfo.itemAction & (ODA_DRAWENTIRE|ODA_SELECT ) ) {
    WRect rc((WRect &)DrawInfo.rcItem);
    LogBrush br(GetSysColor(DrawInfo.itemState&ODS_SELECTED? COLOR_HIGHLIGHT: COLOR_WINDOW));
    gv.FillRect( rc, br );
    BkMode tm=gv.SetBkMode(bkmTransparent);
    Color clrOld;
    if(DrawInfo.itemState&ODS_SELECTED) clrOld = gv.SetTextColor( GetSysColor(COLOR_HIGHLIGHTTEXT) );
    int ColorId = (int)DrawInfo.itemID;
    if( ColorId >= 0 && ColorId < COLOR_NUMBER ) {
      {
	WRect r = rc;
        r.right = r.left + 2*r.Height();
        rc.left = r.right;
	r.Inflate( -2, -2 );
	StockBrush br(sbiBlack);
	gv.FrameRect( r, br );
	r.Inflate( -1, -1 );
	gv.FillRect( r, LogBrush(Color(DefColorPalette[ ColorId ])) );
      }
      {
	gv.ExtTextOut( rc.left+2, rc.top+1, ETO_CLIPPED, rc, color[ ColorId ].id, lstrlen(color[ ColorId ].id), 0);
      }
      if(DrawInfo.itemState&ODS_SELECTED) gv.SetTextColor( clrOld );
    }
    else /*if (DrawInfo.itemID == 0)*/ {
      //gv.TextOutClip( rc.left+2, rc.top+1, rc, ""/*"Default"*/ );
    }
    if(DrawInfo.itemState&ODS_SELECTED) gv.SetTextColor( clrOld );
    gv.SetBkMode( tm );
    if( DrawInfo.itemState & ODS_FOCUS ) DrawFocusRect(gv,&DrawInfo.rcItem);
  }
}


struct CurveDialogInfo {
  int num;
  COLORREF col[2];
};

static ControlInfo far CurveControlInfo[]={
  //{ citTIME,  IDD_XMIN,  FIELDOFFSET(AxisDialogInfo,xmin),  0, 0x7FFFFFFFL},
  //{ citTIME,  IDD_XMAX,  FIELDOFFSET(AxisDialogInfo,xmax),  0, 0x7FFFFFFFL},
  { citNULL }	// End of array marker
};
class CurveDialog : public TransferPlusDlg {
  typedef TransferPlusDlg Inherited;
 public:
  CurveDialog(PTWindowsObject AParent, int ResourceId, char *Name, CurveDialogInfo *info, PTModule AModule = NULL)
   : TransferPlusDlg(AParent, ResourceId, info, CurveControlInfo, AModule)
   , col1(this, IDD_COLORA)
   , col2(this, IDD_COLORB)
  {
    SetCaption(Name);
  }
 protected:
  virtual void SetupWindow()
  {
    Inherited::SetupWindow();
    Centralize(this);
  }
  virtual void SetupControls(void *data);
  virtual void AcceptControls(void *data);
  ColorComboBox col1;
  ColorComboBox col2;
};
void CurveDialog::SetupControls(void *data)
{
  Inherited::SetupControls(data);
  CurveDialogInfo *p=(CurveDialogInfo *)data;
  col1.FillList();  col1.SetSelected(p->col[0]);
  col2.FillList();  col2.SetSelected(p->col[1]);
  if(p->num==1) {
    col2.Show(SW_HIDE);
    ::ShowWindow(::GetDlgItem(HWindow,IDD_COLORALABEL),SW_HIDE);
    ::ShowWindow(::GetDlgItem(HWindow,IDD_COLORBLABEL),SW_HIDE);
  }
}
void CurveDialog::AcceptControls(void *data)
{
  Inherited::AcceptControls(data);
  CurveDialogInfo *p=(CurveDialogInfo *)data;
  col1.GetSelected(&p->col[0]);
  col2.GetSelected(&p->col[1]);
}

void GraphWindow::CMCurveSetup(RTMessage)
{
  PictInfo *pPictInfo=((PictInfo **)SendMessage(Parent->HWindow,WM_GET_PICTINFO,0,0L))[num];
  char name[30];
  wsprintf(name,"Color for Graph %c",'A'+num);
  CurveDialogInfo info;
  info.num=num>=2? 2: 1;
  { for(int i=0; i<2; i++) info.col[i]=pPictInfo->Color[i]; }
  int i=GetModule()->ExecDialog(new CurveDialog(this,ID_CURVEDIALOG,name,&info));
  if(i==IDOK) {
    { for(int i=0; i<2; i++) pPictInfo->Color[i]=info.col[num>=2? i: 0]; }
    InvalidateRect(HWindow,NULL,TRUE);
  }
}

#ifndef  _FRAME_H
#include "frame.h"
#endif

void GraphWindow::ActivationResponse(WORD Activated,  BOOL IsIconified)
{
  FrameWindow *pFrame=(FrameWindow *)Parent;
  pFrame->EnableCommand(CM_PRINT, Activated);
  pFrame->EnableCommand(CM_CHANGEFONT, Activated);
  pFrame->EnableCommand(CM_AXIS_SETUP, Activated);
  pFrame->EnableCommand(CM_CURVE_SETUP, Activated);
  pFrame->EnableCommand(CM_ZOOM_IN, Activated); // P.P. 11/11/09
  pFrame->EnableCommand(CM_ZOOM_OUT, Activated); // P.P. 11/11/09
//  pFrame->EnableCommand(CM_EXPORT, Activated);
  Inherited::ActivationResponse(Activated, IsIconified);
}

void GraphWindow::WMInitMenuPopup(RTMessage Msg)
{
  Menu menu((HMENU)Msg.WParam);
  Job *pJob=(Job *)SendMessage(Parent->HWindow,WM_GET_JOB,0,0L);
  Run *pRun=(Run *)SendMessage(Parent->HWindow,WM_GET_RUN,0,0L);
  //menu.EnableItem(CM_EXPORT, MF_ENABLED);
  menu.EnableItem(CM_EDITCOPY, pJob && pRun? MF_ENABLED: MF_DISABLED);
  menu.EnableItem(CM_EDITCOPYTOFILE, pJob && pRun? MF_ENABLED: MF_DISABLED);
}

#ifndef  _WINLIB_PC_H
#include "winlib\pc.h"
#endif


void GraphWindow::Paint(HDC PaintDC, PAINTSTRUCT _FAR & PaintInfo)
{
  PC pc(PaintDC,&PaintInfo);
  Job *pJob=(Job *)SendMessage(Parent->HWindow,WM_GET_JOB,0,0L);
  if(pJob) {
    Run *pRun=(Run *)SendMessage(Parent->HWindow,WM_GET_RUN,0,0L);
    if(pRun) {
      if(
	#ifdef FILTER
	pJob->GetFiltData(pRun,num) ||
	#endif//FILTER
	pJob->GetRawData(pRun)) {
	Thinking think;
	WRect rc;
	GetClientRect(HWindow,&rc);
	PeakList **pPeakList=(PeakList **)SendMessage(Parent->HWindow,WM_GET_PEAKLIST,0,0L);
	PictInfo **pPictInfo=(PictInfo **)SendMessage(Parent->HWindow,WM_GET_PICTINFO,0,0L);
	::Draw(pJob, pRun, pPictInfo, pc, pc, rc, pRun->runrawdata,
	  #ifdef FILTER
	  pRun->filtData,
	  #endif//FILTER
	  num, pPeakList, &HWindow);
      }
    }
  }
}


/*
void GraphWindow::Draw(const DC &dc, const DC &ic, const WRect &rc, uint16 huge *pRawData,
   #ifdef FILTER
   FiltData *pFiltData,
   #endif//FILTER
   long size, PictInfo *pPictInfo, PeakList *pPeakList)
{
  ::Draw(pPictInfo,dc, ic, rc, pRawData,
    #ifdef FILTER
    pFiltData,
    #endif//FILTER
    size, num, pPeakList);
}
*/

void GraphWindow::WMIsItYourName(RTMessage Msg)
{
  Msg.Result=lstrcmpi(name,(LPSTR)Msg.LParam)==0 && Msg.WParam==num;
}

void GraphWindow::WMRefresh(RTMessage Msg)
{
  int bGraphOnly=Msg.WParam;
  if(!bGraphOnly) InvalidateRect(HWindow,NULL,TRUE);
  else {
    InvalidateRect(HWindow,NULL,FALSE); //???
  }
}



/* --------------------------------------------------------------- */

#ifndef  _GRPH_WND_H
#include "grph_wnd.h"
#endif

PTWindowsObject newGraphWindow(PTWindowsObject AParent, int num, LPSTR ATitle, PTModule AModule)
{
  return new GraphWindow(AParent, num, ATitle, AModule);
}



/////////////////////////////////////////////////////////////////////



LPSTR RunGraphWindow::GetClassName()
{
  return name;
}

void RunGraphWindow::GetWindowClass(WNDCLASS& WndClass)
{
  Inherited::GetWindowClass(WndClass);
  WndClass.hIcon = LoadIcon(GetModule()->hInstance,MAKEINTRESOURCE(ID_WPEAK_ICON));
}


RunGraphWindow::RunGraphWindow(PTWindowsObject AParent, LPSTR ATitle, PTModule AModule)
 :TWindow(AParent, ATitle, AModule)
{
}

RunGraphWindow::~RunGraphWindow()
{
}


void RunGraphWindow::SetupWindow()
{
  Inherited::SetupWindow();
}

RunGraphWindow::RunGraphWindow(StreamableInit)
 : TWindow(streamableInit)
{
};

TStreamableClass RegRunGraphWindow(RunGraphWindow::name, RunGraphWindow::build, __DELTA(RunGraphWindow));
char far RunGraphWindow::name[]="RunGraphWindow";
PTStreamable RunGraphWindow::build()
{
  return new RunGraphWindow(streamableInit);
}

void RunGraphWindow::write(Ropstream os)
{
  Inherited::write(os);
}

Pvoid RunGraphWindow::read(Ripstream is)
{
  if(!Inherited::read(is)) return NULL;
  return this;
}

void RunGraphWindow::WMDestroy(RTMessage Msg)
{
  Inherited::WMDestroy(Msg);
}

#ifndef  _PEAK_APP_H
#include "peak_app.h"
#endif
#ifndef  _WINLIB_DC_H
#include "winlib\dc.h"
#endif

class RunGraphPrintout : public Printout
{
  typedef Printout Inherited;
 protected:
  RunGraphWindow *pRunGraphWindow;
  Job *pJob;
  Run *pRun;
  PeakList **pPeakList;
  PictInfo **pPictInfo;
  int num;
 public:
  RunGraphPrintout(LPSTR ATitle, Job *_pJob, Run *_pRun, PeakList **_pPeakList, PictInfo **_pPictInfo, int _num)
   : Printout(ATitle), pJob(_pJob), pRun(_pRun), pPeakList(_pPeakList), pPictInfo(_pPictInfo), num(_num) {}
  virtual void PrintPage(int page, WRect& rect, unsigned flags);
};
void GetPage(DC &ic,WRect *rect,const WPoint &size);
void RunGraphPrintout::PrintPage(int page, WRect& /*rect*/, unsigned flags)
{
  if(pJob) {
    if(pRun) {
      if(
	#ifdef FILTER
	pJob->GetFiltData(pRun,num) ||
	#endif//FILTER
	pJob->GetRawData(pRun)) {
	WRect rc;
	GetPage(*pdc,&rc,PageSize);
	pdc->IntersectClipRect(rc);
	::Draw(pJob, pRun, pPictInfo,*pdc, *pdc, rc, pRun->runrawdata,
	  #ifdef FILTER
	  pRun->filtData,
	  #endif//FILTER
	  num, pPeakList, 0);
      }
    }
  }
}


void RunGraphWindow::CMPrint(RTMessage)
{
  /*
  PeakApp *pApp=((PeakApp *)GetApplication());
  Printer *pPrinter=pApp;
  if( pPrinter ) {
    Job *pJob=(Job *)SendMessage(Parent->HWindow,WM_GET_JOB,0,0L);
    Run *pRun=(Run *)SendMessage(Parent->HWindow,WM_GET_RUN,0,0L);
    PeakList **pPeakList=(PeakList **)SendMessage(Parent->HWindow,WM_GET_PEAKLIST,0,0L);
    PictInfo **pPictInfo=(PictInfo **)SendMessage(Parent->HWindow,WM_GET_PICTINFO,0,0L);
    RunGraphPrintout *pPrintout = new RunGraphPrintout(pApp->Name, pJob, pRun, pPeakList, pPictInfo, num);
    if( pPrintout ) {
      pPrinter->Print(this, pPrintout, TRUE);
      delete pPrintout;
    }
  }
  */
}
#ifndef  _WINLIB_METAFILE_H
#include "winlib\metafile.h"
#endif

MetafileDC *CreateMetafile2(DC *infoDC,Job *pJob,Run *pRun,RunPictInfo *pRunPictInfo, WRect *rect)
{
  int xSize=6; //6"
  int ySize=5; //5"
  WRect rc(0,0,2540*xSize,2540*ySize);

  MetafileDC *dc=new MetafileDC;
  dc->SetWindow(rc); // Viewport must be set before playing

  static char far const Comment[]="This picture is produced by WPeak program.";
  dc->Escape(MFCOMMENT, lstrlen(Comment), Comment);
  /*
  if(
    #ifdef FILTER
    pJob->GetFiltData(pRun,num) ||
    #endif//FILTER
    pJob->GetRawData(pRun)) {
    Thinking think;
    if(infoDC) {
      DC &ic=*infoDC;
      SelMapMode oldMapMode(ic,mmAnisotropic);
      ic.SetWindow(rc);
      int resX=ic.GetCapability(LOGPIXELSX);
      int resY=ic.GetCapability(LOGPIXELSY);
      ic.SetViewport(WRect(0,0,resX*xSize,resY*ySize));
      //void Draw1(Method *pMethod, RunPictInfo *pPictInfo,const DC &dc, const DC &ic, const WRect &rc, const List &list, HWND *hwnd);
      ::Draw1(pMethod, pRunPictInfo, *dc, ic, rc, pRun->runrawdata,
	#ifdef FILTER
	pRun->filtData,
	#endif//FILTER
	num, pPeakList, 0);
    }
  }
  */
  *rect=rc;
  return dc;
}


MetafileDC *RunGraphWindow::CreateMetafile(WRect *rc)
{
  MetafileDC *dc=0;
  Job *pJob=(Job *)SendMessage(Parent->HWindow,WM_GET_JOB,0,0L);
  Run *pRun=(Run *)SendMessage(Parent->HWindow,WM_GET_RUN,0,0L);
  PeakList **pPeakList=(PeakList **)SendMessage(Parent->HWindow,WM_GET_PEAKLIST,0,0L);
  RunPictInfo *pRunPictInfo=(RunPictInfo *)SendMessage(Parent->HWindow,WM_GET_RUNPICTINFO,0,0L);

  if(pJob && pRun) {
    DC *infoDC=0;
    PeakApp *pApp=((PeakApp *)GetApplication());
    Printer *pPrinter=pApp;
    if( pPrinter ) infoDC=pPrinter->GetDC(this,0);
    if(!infoDC) infoDC=new WindowDC(HWindow);

//    dc=::CreateMetafile2(infoDC,pJob,pRun,pPeakList, rc);

    delete infoDC;
  }
  return dc;
}

void RunGraphWindow::CMEditCopy(RTMessage)
{
  WRect rc;
  MetafileDC *dc=CreateMetafile(&rc);

  if(dc && OpenClipboard(HWindow)) {
    EmptyClipboard();
    MetaFile mf(*dc);
    mf.SetClipboardData(rc,mmAnisotropic);

    CloseClipboard();
  }
  delete dc;
}

extern void LoadFilterString(HINSTANCE hinst, UINT idResource, LPSTR lpszBuffer, int cbBuffer);


#include "mainwind.h"
#include "wcpplib\fwstream.h"
void RunGraphWindow::CMEditCopyToFile(RTMessage)
{
  Run *pRun=(Run *)SendMessage(Parent->HWindow,WM_GET_RUN,0,0L);
  if(!pRun) return;

  HINSTANCE hInstance=GetApplication()->hInstance;
  Str strFilter(128);
  LoadFilterString(hInstance, IDS_WMF_FILTER, strFilter.GetPtr(), 128);
  Str strTitle(64);
  LoadString(hInstance, IDS_SAVEPICTURE, strTitle.GetPtr(), 64);
  Str FileName;

  BOOL ret=((MainWindow *)Parent)->OpenSaveDialog(HWindow, strTitle, strFilter, &FileName, TRUE);
  if(ret) {
    ofwstream os(FileName);

    WRect rc;
    MetafileDC *dc=CreateMetafile(&rc);

    if(dc && os.good()) {
      MetaFile mf(*dc);
      mf.Write(os,rc,mmAnisotropic);
    }
    delete dc;
  }
}


void RunGraphWindow::CMChangeFont(RTMessage)
{
  DC *dc=((PeakApp *)GetApplication())->GetDC(); // Printer DC
  RunPictInfo *pRunPictInfo=(RunPictInfo *)SendMessage(Parent->HWindow,WM_GET_RUNPICTINFO,0,0L);

  static far CHOOSEFONT cf;
  LOGFONT lf=pRunPictInfo->lfUser;
  memset(&cf, 0, sizeof(CHOOSEFONT));
  cf.lStructSize = sizeof(CHOOSEFONT);
  cf.hInstance = GetApplicationObject()->hInstance;
  cf.hwndOwner = HWindow;
  cf.hDC       = dc? (HDC)*dc: (HDC)0;
  cf.lpLogFont = &lf;
  cf.Flags = CF_SCREENFONTS | CF_EFFECTS | CF_INITTOLOGFONTSTRUCT | CF_PRINTERFONTS;
  cf.rgbColors = 0; //RGB(0, 255, 255); // light blue
  cf.rgbColors = pRunPictInfo->TextColor;

  //cf.Flags |= CF_ENABLEHOOK;
  //(FARPROC)cf.lpfnHook=MakeProcInstance((FARPROC)FontHook,GetApplicationObject()->hInstance);

  int resY=72;//dc.GetCapability(LOGPIXELSY);
  int resScreenY=ScreenDC().GetCapability(LOGPIXELSY);
  cf.lpLogFont->lfHeight=MulDiv(cf.lpLogFont->lfHeight,resScreenY,resY);
  if(ChooseFont(&cf)) InvalidateRect(HWindow, NULL, TRUE);
  cf.lpLogFont->lfHeight=MulDiv(cf.lpLogFont->lfHeight,resY,resScreenY);
  //if(cf.lpfnHook) FreeProcInstance((FARPROC)cf.lpfnHook);

  pRunPictInfo->lfUser=lf;
  pRunPictInfo->TextColor=Color(cf.rgbColors);
}

struct AxisDialog2Info {
  long xmin,xmax;
  int kx;
  int xAuto;
  long ymin,ymax;
  int ky;
  int yAuto;
};

static ControlInfo far AxisControlInfo2[]={
//  { citTIME,  IDD_XMIN,  FIELDOFFSET(AxisDialog2Info,xmin),  0, 0x7FFFFFFFL},
//  { citTIME,  IDD_XMAX,  FIELDOFFSET(AxisDialog2Info,xmax),  0, 0x7FFFFFFFL},
  { citINT,   IDD_KX,    FIELDOFFSET(AxisDialog2Info,kx),  1, 50},
  { citONOFF, IDD_XAUTO, FIELDOFFSET(AxisDialog2Info,xAuto)},
  { citLONG,  IDD_YMIN,  FIELDOFFSET(AxisDialog2Info,ymin),  0, 0x7FFFFFFFL},
  { citLONG,  IDD_YMAX,  FIELDOFFSET(AxisDialog2Info,ymax),  0, 0x7FFFFFFFL},
  { citINT,   IDD_KY,    FIELDOFFSET(AxisDialog2Info,ky),  1, 50},
  { citONOFF, IDD_YAUTO, FIELDOFFSET(AxisDialog2Info,yAuto)},
  { citNULL }	// End of array marker
};
class AxisDialog2 : public TransferPlusDlg {
  typedef TransferPlusDlg Inherited;
 public:
  AxisDialog2(PTWindowsObject AParent, int ResourceId, char *Name, AxisDialog2Info *info, PTModule AModule = NULL)
   : TransferPlusDlg(AParent, ResourceId, info, AxisControlInfo2, AModule)
  {
    SetCaption(Name);
  }
 protected:
  virtual void SetupWindow()
  {
    Inherited::SetupWindow();
    Centralize(this);
  }
  void SetupControls(void *data);
  virtual BOOL ValidateControls(void *data);
  void AcceptControls(void *data);
};

#include "country.h"
#include "winutil\dlg_item.h"

void AxisDialog2::SetupControls(void *data)
{
  AxisDialog2Info *info=(AxisDialog2Info *)data;

  Str str;

  str=PrintDate(info->xmin, 1, 1);
  str+="; ";
  str+=PrintTime(info->xmin, 1);;
  SetDlgItemText(HWindow, IDD_XMIN, str);

  str=PrintDate(info->xmax, 1, 1);
  str+="; ";
  str+=PrintTime(info->xmax, 1);
  SetDlgItemText(HWindow, IDD_XMAX, str);

  Inherited::SetupControls(data);
}
BOOL AxisDialog2::ValidateControls(void *data)
{
  //int i=GetDlgItemInt(HWindow, idControl, &fTranslated, TRUE); if(!fTranslated) goto error_label;
  return Inherited::ValidateControls(data);
}
void AxisDialog2::AcceptControls(void *data)
{
  AxisDialog2Info *info=(AxisDialog2Info *)data;

  Str str;

  str=GetDlgItemText(HWindow, IDD_XMIN);
  const char *cp=strchr(str,';');
  if(cp) {
    *((char *)cp)='\0';
    struct tm t;
    ReadDate(str, &t.tm_year, &t.tm_mon, &t.tm_mday);
    ReadTime(cp+1, &t.tm_hour, &t.tm_min);
    t.tm_sec=0;      /* Seconds */
    //t.tm_min;      /* Minutes */
    //t.tm_hour;     /* Hour (0--23) */
    //t.tm_mday;     /* Day of month (1--31) */
    t.tm_mon-=1;      /* Month (0--11) */
    t.tm_year-=1900;     /* Year (calendar year minus 1900) */
    //t.tm_wday;     /* Weekday (0--6; Sunday = 0) */
    //t.tm_yday;     /* Day of year (0--365) */
    //t.tm_isdst;
    info->xmin=mktime(&t);
  }

  str=GetDlgItemText(HWindow, IDD_XMAX);
  cp=strchr(str,';');
  if(cp) {
    *((char *)cp)='\0';
    struct tm t;
    ReadDate(str, &t.tm_year, &t.tm_mon, &t.tm_mday);
    ReadTime(cp+1, &t.tm_hour, &t.tm_min);
    t.tm_sec=0;      /* Seconds */
    //t.tm_min;      /* Minutes */
    //t.tm_hour;     /* Hour (0--23) */
    //t.tm_mday;     /* Day of month (1--31) */
    t.tm_mon-=1;      /* Month (0--11) */
    t.tm_year-=1900;     /* Year (calendar year minus 1900) */
    //t.tm_wday;     /* Weekday (0--6; Sunday = 0) */
    //t.tm_yday;     /* Day of year (0--365) */
    //t.tm_isdst;
    info->xmax=mktime(&t);
  }
  Inherited::AcceptControls(data);
}


void RunGraphWindow::CMAxisSetup(RTMessage)
{
  RunPictInfo *pRunPictInfo=(RunPictInfo *)SendMessage(Parent->HWindow,WM_GET_RUNPICTINFO,0,0L);
  AxisDialog2Info info;
  info.xmin=pRunPictInfo->ai[0].mi;
  info.xmax=pRunPictInfo->ai[0].ma;
  info.kx=pRunPictInfo->ai[0].k;
  info.xAuto=pRunPictInfo->ai[0].bAuto;
  info.ymin=pRunPictInfo->ai[1].mi;
  info.ymax=pRunPictInfo->ai[1].ma;
  info.ky=pRunPictInfo->ai[1].k;
  info.yAuto=pRunPictInfo->ai[1].bAuto;
  int i=GetModule()->ExecDialog(new AxisDialog2(this,ID_AXISDIALOG,"Axes for Run Graph",&info));
  if(i==IDOK) {
    pRunPictInfo->ai[0].mi=info.xmin;
    pRunPictInfo->ai[0].ma=info.xmax;
    pRunPictInfo->ai[0].k=info.kx;
    pRunPictInfo->ai[0].bAuto=info.xAuto;
    pRunPictInfo->ai[1].mi=info.ymin;
    pRunPictInfo->ai[1].ma=info.ymax;
    pRunPictInfo->ai[1].k=info.ky;
    pRunPictInfo->ai[1].bAuto=info.yAuto;
    InvalidateRect(HWindow,NULL,TRUE);
  }
}





void RunGraphWindow::CMCurveSetup(RTMessage)
{
  RunPictInfo *pRunPictInfo=(RunPictInfo *)SendMessage(Parent->HWindow,WM_GET_RUNPICTINFO,0,0L);
  CurveDialogInfo info;
  info.num=0;
  { for(int i=0; i<2; i++) info.col[i]=pRunPictInfo->Color[i]; }
  int i=GetModule()->ExecDialog(new CurveDialog(this,ID_CURVEDIALOG,"Color for Run Graph",&info));
  if(i==IDOK) {
    { for(int i=0; i<2; i++) pRunPictInfo->Color[i]=info.col[0]; }
    InvalidateRect(HWindow,NULL,TRUE);
  }
}

#ifndef  _FRAME_H
#include "frame.h"
#endif

void RunGraphWindow::ActivationResponse(WORD Activated,  BOOL IsIconified)
{
  FrameWindow *pFrame=(FrameWindow *)Parent;
  pFrame->EnableCommand(CM_PRINT, Activated);
  pFrame->EnableCommand(CM_CHANGEFONT, Activated);
  pFrame->EnableCommand(CM_AXIS_SETUP, Activated);
  pFrame->EnableCommand(CM_CURVE_SETUP, Activated);
//  pFrame->EnableCommand(CM_EXPORT, Activated);
  Inherited::ActivationResponse(Activated, IsIconified);
}

void RunGraphWindow::WMInitMenuPopup(RTMessage Msg)
{
  Menu menu((HMENU)Msg.WParam);
  Job *pJob=(Job *)SendMessage(Parent->HWindow,WM_GET_JOB,0,0L);
  Run *pRun=(Run *)SendMessage(Parent->HWindow,WM_GET_RUN,0,0L);
  //menu.EnableItem(CM_EXPORT, MF_ENABLED);
  menu.EnableItem(CM_EDITCOPY, pJob && pRun? MF_ENABLED: MF_DISABLED);
  menu.EnableItem(CM_EDITCOPYTOFILE, pJob && pRun? MF_ENABLED: MF_DISABLED);
}

#include "winlib\pc.h"
#include "wcpplib\fwstream.h"

#include "country.h"


void RunGraphWindow::Paint(HDC PaintDC, PAINTSTRUCT _FAR & PaintInfo)
{
  Thinking think;
  PC pc(PaintDC,&PaintInfo);
  Job *pJob=(Job *)SendMessage(Parent->HWindow,WM_GET_JOB,0,0L);
  Method *pMethod=pJob;
  if(pJob) {
    static char RunFile[80];
    pJob->MakeName(RunFile, "run");
    ifwstream is(RunFile);
    if(is.good()) {
      List list;
      for(BOOL bEnd=FALSE; !bEnd; ) {
	char line[300];
	{
	  for(char *cp=line; cp<line+299; cp++) {
	    int n=is.read(cp,1);
	    if(n<0) {
	      break;
	    }
	    if(n==0) {
	      bEnd=TRUE;
	      break;
	    }
	    if(*cp=='\n') { *cp='\0'; break; }
	  }
	  if(!is.good())
	    break;
	  *cp='\0';
	}

	RunInfo *p=new RunInfo;
	list.append(p);

	//"Detector A", "NMHC", 6.302, 213, 72240, "0:31", "Low", "1/8/02", "3:27PM", "010802", "Calibration, A FAILED"
	//"Detector A", "NMHC", 3.463, 13921, 2543830, "0:16", "none", "1/10/02", "10:16AM", "010802", "amb air through charcoal trap", 3.463, 3.463
	{
	  enum {
	    WaitValue,
	    Value,
	    QuotValue,
	  } status(WaitValue);
	  char value[200], *vp;
	  int value_num=0;
	  for(const char *cp=line; ; cp++) {
	    switch(status) {
	      case WaitValue:
		switch(*cp) {
		  case ' ': break;
		  case '\"': vp=value; status=QuotValue; break;
		  default: vp=value; *(vp++)=*cp; status=Value; break;
		}
	      break;
	      case Value:
		switch(*cp) {
		  case '\0':
		  case ',':
		    *vp=0;
		    status=WaitValue;
		    //////////
		    switch(value_num) {
		      case 0: p->det_name=value; break;
		      case 1: p->peak_name=value; break;
		      case 2: p->conc=atof(value); break;
		      case 3: p->height=atol(value); break;
		      case 4: p->area=atol(value); break;
		      case 5: p->rt=value; break;
		      case 6: p->alarm=value; break;
		      case 7: p->date=value; break;
		      case 8: p->time=value; break;
		      case 9: p->jobname=value; break;
		      case 10: p->comment=value; break;
		    }
		    value_num++;
		  break;
		  default: *(vp++)=*cp; break;
		}
	      break;
	      case QuotValue:
		switch(*cp) {
		  case '\"': status=Value; break;
		  default: *(vp++)=*cp; break;
		}
	      break;
	    }
	    if(!*cp) break;
	  }
	}
	{
	  struct tm t;
	  memset(&t,0,sizeof t);

	  ReadDate(p->date, &t.tm_year, &t.tm_mon, &t.tm_mday);
	  t.tm_mon-=1;
	  t.tm_year-=1900;
	  ReadTime(p->time, &t.tm_hour, &t.tm_min);


	  p->datetime=(unsigned long)mktime(&t);
	}
	if((p->det_name=="No detectors On")  ||
	   (0==strncmp(p->comment,"Calibration",11)) ||
	   (p->conc<0)
	   ) {
	  p->shutDown();
	  delete p;
	}
      }
      WRect rc;
      GetClientRect(HWindow,&rc);
      RunPictInfo *pRunPictInfo=(RunPictInfo *)SendMessage(Parent->HWindow,WM_GET_RUNPICTINFO,0,0L);

      Draw1(pMethod, pRunPictInfo, pc, pc, rc, list, &HWindow);

      list.shutDown();
    }
  }
}


/*
void RunGraphWindow::Draw(const DC &dc, const DC &ic, const WRect &rc, uint16 huge *pRawData,
   #ifdef FILTER
   FiltData *pFiltData,
   #endif//FILTER
   long size, PictInfo *pPictInfo, PeakList *pPeakList)
{
  ::Draw(pPictInfo,dc, ic, rc, pRawData,
    #ifdef FILTER
    pFiltData,
    #endif//FILTER
    size, num, pPeakList);
}
*/

void RunGraphWindow::WMIsItYourName(RTMessage Msg)
{
  Msg.Result=lstrcmpi(name,(LPSTR)Msg.LParam)==0;
}

void RunGraphWindow::WMRefresh(RTMessage Msg)
{
  int bGraphOnly=Msg.WParam;
  if(!bGraphOnly) InvalidateRect(HWindow,NULL,TRUE);
  else {
    InvalidateRect(HWindow,NULL,FALSE); //???
  }
}



/* --------------------------------------------------------------- */

#ifndef  _GRPH_WND_H
#include "grph_wnd.h"
#endif

PTWindowsObject newRunGraphWindow(PTWindowsObject AParent, LPSTR ATitle, PTModule AModule)
{
  return new RunGraphWindow(AParent, ATitle, AModule);
}

