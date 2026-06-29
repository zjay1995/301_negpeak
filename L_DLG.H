#ifndef  _L_DLG_H
#define  _L_DLG_H

#ifndef  _HELP_DLG_H
#include "help_dlg.h"	// HelpDlg
#endif

#ifndef  _METHOD_H
#include "method.h"	// NUMDETECTORS
#endif

#ifndef  _ID_H
#include "id.h"
#endif

class ListDlg : public HelpDlg {
  typedef HelpDlg Inherited;
 public:
   ListDlg(PTWindowsObject AParent, int ResourceId, void *data, PTModule AModule = NULL);
 protected:
  TListBox *list[NUMDETECTORS];
  virtual void WMCommand(RTMessage Msg) = [WM_FIRST + WM_COMMAND];
  virtual void Edit(int n);
  virtual void WMDeleteItem(RTMessage Msg) = [WM_FIRST+WM_DELETEITEM];
  virtual void DeleteItem(DELETEITEMSTRUCT _FAR & DeleteInfo);
  virtual void EditButton(RTMessage Msg) = [ID_FIRST + ID_EDIT_BUTTON];
};

#endif
