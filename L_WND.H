#ifndef  _L_WND_H
#define  _L_WND_H

#ifndef __OWL_H
#include <owl.h>
#endif

_CLASSDEF(ListBox);


class ListWindow : public TWindow {
  typedef TWindow Inherited;
  PTListBox list;
  int HeaderHeight;
 public:
  ListWindow(PTWindowsObject AParent,char far *ATitle,PTModule AModule = NULL);
  ~ListWindow();
 protected:
  virtual void SetupWindow();
  virtual void WMInitMenuPopup(RTMessage) = [WM_FIRST + WM_INITMENUPOPUP];
  virtual void Paint(HDC PaintDC, PAINTSTRUCT _FAR & PaintInfo);
  virtual void WMMeasureItem(TMessage& Msg) = [WM_FIRST + WM_MEASUREITEM];
  virtual void WMSize(RTMessage Msg) = [WM_FIRST + WM_SIZE];
  virtual void WMSetFocus(RTMessage Msg) = [WM_FIRST + WM_SETFOCUS];
  virtual void WMCommand(RTMessage Msg) = [WM_FIRST + WM_COMMAND];
  virtual void WMIsItYourName(RTMessage Msg) = [WM_FIRST + WM_IS_IT_YOUR_NAME];
  virtual void WMRefresh(RTMessage Msg) = [WM_FIRST + WM_REFRESH];

  virtual void HandleList(RTMessage Msg) = [ID_FIRST + IDD_PEAKLIST];
  virtual void FillList();

  void SetHeaderHeight();

  ListWindow(StreamableInit);
  virtual void write (Ropstream os);
  virtual Pvoid read (Ripstream is);
};



#endif
