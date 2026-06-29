#ifndef  _TD_PLUS_H
#define  _TD_PLUS_H


#ifndef  _HELP_DLG_H
#include "help_dlg.h"
#endif


enum ControlInfoType {
  citNULL=0,
  citINT,
  citLONG,
  citDouble,
  citTEMP,
  citTIME,
  citONOFF,
  citOFFON,
  citRadio,
  citComboList,
  citCYCLE,
  citStr
};

struct ControlInfo {
  ControlInfoType type;
  int idControl;
  int offset;
  long info1;
  long info2;
};

class TransferPlusDlg : public HelpDlg {
  typedef HelpDlg Inherited;
 public:
   TransferPlusDlg(PTWindowsObject AParent, int ResourceId, void *data, ControlInfo *cip, PTModule AModule = NULL)
    : HelpDlg(AParent, ResourceId, data, AModule), ciArray(cip) {}
 protected:
  ControlInfo *ciArray;

  virtual void SetupControls(void *data);
  virtual BOOL ValidateControls(void *data);
  virtual void AcceptControls(void *data);

  void SetupControl(ControlInfo *cip, void *data);
  BOOL ValidateControl(ControlInfo *cip, void *data);
  void AcceptControl(ControlInfo *cip, void *data);
};

#endif
