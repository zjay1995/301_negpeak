#ifndef  _HELP_DLG_H
#define  _HELP_DLG_H

#ifndef  _WINUTIL_TRANSDLG_H
#include "winutil\transdlg.h"	// TransferDlg
#endif

#define IDHELP			998

class StatusLine;

class HelpDlg : public TransferDlg {
  typedef TransferDlg Inherited;
  StatusLine *sl;
 public:
  HelpDlg(PTWindowsObject AParent, int ResourceId, void *data, PTModule AModule = NULL);
  virtual ~HelpDlg();
 protected:
  virtual void SetupWindow();
  virtual void WMSize(RTMessage Msg) = [WM_FIRST + WM_SIZE];
  virtual void WMPaint(RTMessage Msg) = [WM_FIRST + WM_PAINT];
  virtual void WMSetCursor(RTMessage Msg) = [WM_FIRST + WM_SETCURSOR];
  virtual void WMCommand(RTMessage Msg) = [WM_FIRST + WM_COMMAND];
  virtual void DMGetDefID(RTMessage Msg) = [WM_FIRST + DM_GETDEFID];
  virtual void WMKeyUp(RTMessage Msg) = [WM_FIRST + WM_KEYUP];
  virtual void Help(RTMessage Msg) = [ID_FIRST + IDHELP];

  void Help();
};

#endif
