#ifndef  _OUTPWIND_H
#define  _OUTPWIND_H

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
class OutputWindow : public TBWindow {
  typedef TBWindow Inherited;
  PTEdit Editor;
  LOGFONT lfUser;
  COLORREF TextColor;
  virtual LPSTR GetClassName();
  virtual void GetWindowClass(WNDCLASS &AWndClass);
 public:
  OutputWindow(PTWindowsObject AParent, PTModule AModule = NULL);
  ~OutputWindow();
  void vprint(UINT fmtID, va_list arglist);
  void vprint(LPCSTR fmt, va_list arglist);
  void print(UINT fmtID, ... );
  void print(LPCSTR fmt, ... );
 protected:
  virtual void SetupWindow();
  virtual void ActivationResponse(WORD Activated,  BOOL IsIconified);
  virtual void WMInitMenuPopup(RTMessage) = [WM_FIRST + WM_INITMENUPOPUP];
  virtual void WMSize(RTMessage Msg) = [WM_FIRST + WM_SIZE];
  virtual void WMSetFocus(RTMessage Msg) = [WM_FIRST + WM_SETFOCUS];
  virtual void WMIsItYourName(RTMessage Msg) = [WM_FIRST + WM_IS_IT_YOUR_NAME];


  virtual void CMPrint(RTMessage) = [CM_FIRST + CM_PRINT];
  virtual void CMExport(RTMessage) = [CM_FIRST + CM_EXPORT];
  virtual void CMChangeFont(RTMessage) = [CM_FIRST + CM_CHANGEFONT];


  OutputWindow(StreamableInit);
  virtual void write (Ropstream os);
  virtual Pvoid read (Ripstream is);
  virtual const Pchar streamableName() const { return name; }
 public:
  static PTStreamable build();
  static char far name[];
};

#endif
