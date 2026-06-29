#include "includes.h"
#pragma hdrstop

#ifndef  _WINUTIL_WUTIL_H
#include "winutil\wutil.h"
#endif

#ifndef  _TD_PLUS_H
#include "td_plus.h"
#endif
#ifndef  _OPTS_DLG_H
#include "opts_dlg.h"
#endif

#ifndef  _ID_H
#include "id.h"
#endif

static ControlInfo far ci[]=
{
  { citONOFF,	IDD_ENVIROMENT, FIELDOFFSET(OptionsSaveStruct,bSaveEnvironment),},
  { citONOFF,	IDD_DESKTOP,	FIELDOFFSET(OptionsSaveStruct,bSaveDesktop),	},
  { citONOFF,	IDD_WINDOW,	FIELDOFFSET(OptionsSaveStruct,bSaveWindow),	},
  { citNULL }	// End of array marker
};

class OptionsSaveDialog : public TransferPlusDlg{
  typedef TransferPlusDlg Inherited;
 public:
  OptionsSaveDialog(PTWindowsObject AParent, OptionsSaveStruct *data, PTModule AModule = NULL)
   : TransferPlusDlg(AParent, ID_SAVEOPTIONSDIALOG, data, ci, AModule)
  {
  }
  virtual void SetupWindow()
  {
    Inherited::SetupWindow();
    Centralize(this);
  }
};


/* ----------------------------------------------------------------------- */

#ifndef  _OPTS_DLG_H
#include "opts_dlg.h"
#endif

PTWindowsObject newOptionsSaveDialog(PTWindowsObject AParent, OptionsSaveStruct *data, PTModule AModule)
{
  return new OptionsSaveDialog(AParent, data, AModule);
}
