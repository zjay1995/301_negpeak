#ifndef  _GRPHWIND_H
#define  _GRPHWIND_H

#ifndef  _ID_H
#include "id.h"
#endif

#ifndef  _WINLIB_DC_H
#include "winlib\dc.h"
#endif
#ifndef  _WCPPLIB_STR_H
#include "wcpplib\str.h"
#endif

#ifndef  _SHORTS_H
#include "shorts.h"
#endif

class MetafileDC;

//class PictInfo;
#ifdef FILTER
class FiltData;
#endif//FILTER
class PeakList;
class GraphWindow : public TWindow {
  typedef TWindow Inherited;
 protected:
  int num;
 public:
  GraphWindow(PTWindowsObject Parent, int num, LPSTR ATitle, PTModule AModule = NULL);

 protected:
  virtual ~GraphWindow();
  virtual LPSTR GetClassName();
  virtual void GetWindowClass(WNDCLASS& WndClass);

  virtual void SetupWindow();
  virtual void WMDestroy(RTMessage) = [WM_FIRST + WM_DESTROY];

  virtual void Paint(HDC PaintDC, PAINTSTRUCT _FAR & PaintInfo);
 public:
/*
  void Draw(const DC &dc, const DC &ic, const WRect &rc, uint16 huge *pRawData,
    #ifdef FILTER
    FiltData *pFiltData,
    #endif//FILTER
    long size, PictInfo *pPictInfo, PeakList *pPeakList);
*/
 protected:

  virtual void ActivationResponse(WORD Activated,  BOOL IsIconified);
  virtual void WMInitMenuPopup(RTMessage) = [WM_FIRST + WM_INITMENUPOPUP];
  virtual void WMIsItYourName(RTMessage Msg) = [WM_FIRST + WM_IS_IT_YOUR_NAME];
  virtual void WMRefresh(RTMessage Msg) = [WM_FIRST + WM_REFRESH];

  virtual void CMEditCopy(RTMessage) = [CM_FIRST + CM_EDITCOPY];
  virtual void CMEditCopyToFile(RTMessage) = [CM_FIRST + CM_EDITCOPYTOFILE];
  virtual void CMPrint(RTMessage) = [CM_FIRST + CM_PRINT];
  virtual void CMChangeFont(RTMessage) = [CM_FIRST + CM_CHANGEFONT];
  virtual void CMAxisSetup(RTMessage) = [CM_FIRST + CM_AXIS_SETUP];
  virtual void CMCurveSetup(RTMessage) = [CM_FIRST + CM_CURVE_SETUP];
  virtual void CMZoomIn(RTMessage) = [CM_FIRST + CM_ZOOM_IN];   // P.P. 11/11/09
  virtual void CMZoomOut(RTMessage) = [CM_FIRST + CM_ZOOM_OUT];   // P.P. 11/11/09
  virtual void CMScaleIn(RTMessage) = [CM_FIRST + CM_SCALEIN];     // P.P. 01/22/13
  virtual void CMScaleOut(RTMessage) = [CM_FIRST + CM_SCALEOUT];   //

  MetafileDC *CreateMetafile(WRect *rc);
  GraphWindow(StreamableInit);
  virtual void write (Ropstream os);
  virtual Pvoid read (Ripstream is);
  virtual const Pchar streamableName() const { return name; }
 public:
  static PTStreamable build();
  static char far name[];

};


class RunGraphWindow : public TWindow {
  typedef TWindow Inherited;
 public:
  RunGraphWindow(PTWindowsObject Parent, LPSTR ATitle, PTModule AModule = NULL);

 protected:
  virtual ~RunGraphWindow();
  virtual LPSTR GetClassName();
  virtual void GetWindowClass(WNDCLASS& WndClass);

  virtual void SetupWindow();
  virtual void WMDestroy(RTMessage) = [WM_FIRST + WM_DESTROY];

  virtual void Paint(HDC PaintDC, PAINTSTRUCT _FAR & PaintInfo);
 public:
 protected:

  virtual void ActivationResponse(WORD Activated,  BOOL IsIconified);
  virtual void WMInitMenuPopup(RTMessage) = [WM_FIRST + WM_INITMENUPOPUP];
  virtual void WMIsItYourName(RTMessage Msg) = [WM_FIRST + WM_IS_IT_YOUR_NAME];
  virtual void WMRefresh(RTMessage Msg) = [WM_FIRST + WM_REFRESH];

  virtual void CMEditCopy(RTMessage) = [CM_FIRST + CM_EDITCOPY];
  virtual void CMEditCopyToFile(RTMessage) = [CM_FIRST + CM_EDITCOPYTOFILE];
  virtual void CMPrint(RTMessage) = [CM_FIRST + CM_PRINT];
  virtual void CMChangeFont(RTMessage) = [CM_FIRST + CM_CHANGEFONT];
  virtual void CMAxisSetup(RTMessage) = [CM_FIRST + CM_AXIS_SETUP];
  virtual void CMCurveSetup(RTMessage) = [CM_FIRST + CM_CURVE_SETUP];
//  virtual void CMZoomIn(RTMessage) = [CM_FIRST + CM_ZOOM_IN];   // P.P. 11/11/09, then  rem for run graph
//  virtual void CMZoomOut(RTMessage) = [CM_FIRST + CM_ZOOM_OUT];   // P.P. 11/11/09


  MetafileDC *CreateMetafile(WRect *rc);
  RunGraphWindow(StreamableInit);
  virtual void write (Ropstream os);
  virtual Pvoid read (Ripstream is);
  virtual const Pchar streamableName() const { return name; }
 public:
  static PTStreamable build();
  static char far name[];

};
struct AxisDialogInfo {  // moved here from grph_wnd.cpp. P.P. 11/02/12
  long xmin,xmax;
  int kx;
  int xAuto;
  long ymin,ymax;
  int ky;
  int yAuto;
  int bScale; // added. P.P. 01/22/13
};

#endif
