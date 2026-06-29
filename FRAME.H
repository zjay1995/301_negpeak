#ifndef  _FRAME_H
#define  _FRAME_H

#ifndef __OWL_H
#include <owl.h>
#endif
#include "helpids.h"
class StatusLine;
class ToolBar;

class FrameWindow : public TMDIFrame {
  typedef TMDIFrame Inherited;
 public:
  StatusLine *sl;
  ToolBar *tb;
  char szHelpFileName[128];

  FrameWindow(LPSTR ATitle, int MenuId, LPSTR HelpFile, PTModule AModule = NULL);
  virtual ~FrameWindow();
  virtual void SetupWindow();
  virtual void WMDestroy(RTMessage) = [WM_FIRST + WM_DESTROY];

  virtual void InitClientWindow();

  virtual void Paint(HDC PaintDC, PAINTSTRUCT _FAR & PaintInfo);
  virtual void WMSize(RTMessage) = [WM_FIRST + WM_SIZE];
  virtual void WMClose(RTMessage) = [WM_FIRST + WM_CLOSE];

  virtual void WMLButtonDown(RTMessage) = [WM_FIRST + WM_LBUTTONDOWN];
  virtual void WMLButtonUp(RTMessage) = [WM_FIRST + WM_LBUTTONUP];
  virtual void WMMouseMove(RTMessage) = [WM_FIRST + WM_MOUSEMOVE];

  virtual void WMInitMenuPopup(RTMessage) = [WM_FIRST + WM_INITMENUPOPUP];
  virtual void WMMenuSelect(RTMessage) = [WM_FIRST + WM_MENUSELECT];
  virtual void CMTileChildrenHoriz(RTMessage Msg) = [CM_FIRST + CM_TILECHILDRENHORIZ];

  virtual int  GetMainMenuString(UINT id) = 0;

  void SaveDesktop(const char *DskFile);
  int RestoreDesktop(const char *DskFile);
  virtual void SaveDesktopInfo(Ropstream os) {}
  virtual void RestoreDesktopInfo(Ripstream is) {}

  PTWindowsObject FindChild(LPCSTR ClassName, WPARAM wParam=0);
  PTWindowsObject ActiveChild();
  void ForChild(LPCSTR ClassName, TActionFunc Action, Pvoid PParamList=0);

  void EnableCommand(int id, int bEnable);
};


#endif
