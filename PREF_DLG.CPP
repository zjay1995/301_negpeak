#include "includes.h"

#ifndef  _WUTIL_H
#include "winutil\wutil.h"
#endif

#ifndef  _ID_H
#include "id.h"
#endif

#ifndef  _PREF_H
#include "pref.h"
#endif

#ifndef  _TD_PLUS_H
#include "td_plus.h"
#endif

#include "internal.h" // INTERNAL()

//#define FIELDOFFSET(type, field)    ((int)(&((type NEAR*)1)->field)-1)

static ControlInfo far PreferencesControlInfo[]={
  { citONOFF,	IDD_ENVIROMENT,	FIELDOFFSET(Preferences,bAutoSaveEnvironment),	},
  { citONOFF,	IDD_DESKTOP,	FIELDOFFSET(Preferences,bAutoSaveDesktop),	},
  { citONOFF,	IDD_WINDOW,	FIELDOFFSET(Preferences,bAutoSaveWindow),	},
  { citONOFF,	IDD_METHOD,	FIELDOFFSET(Preferences,bAutoSaveMethod),	},
  #if INTERNAL()
  { citComboList,IDD_COMPORT,	FIELDOFFSET(Preferences,iCom), (LPARAM)"COM1:\0COM2:\0COM3:\0COM4:\0None\0"},
  #else
  { citComboList,IDD_COMPORT,	FIELDOFFSET(Preferences,iCom), (LPARAM)"COM1:\0COM2:\0COM3:\0COM4:\0"},
  #endif
  #if MODBUS()
  { citComboList,IDD_MODBUS,	FIELDOFFSET(Preferences,iModBusCom), (LPARAM)"None\0COM1:\0COM2:\0COM3:\0COM4:\0"},
  #endif

  { citRadio,	IDD_CM,		FIELDOFFSET(Preferences,bInches), 2	},
  { citONOFF,	IDD_STARTUPABOUT,FIELDOFFSET(Preferences,bStartupAbout),	},
  { citStr,	IDD_STARTUPFILE,FIELDOFFSET(Preferences,sStartupFile),	},
  { citONOFF,	IDD_STARTUPWINDOW,FIELDOFFSET(Preferences,bStartupWindow),	},
//  { citStr,	IDD_GC311VERSION,FIELDOFFSET(Preferences,GC311version), },
  { citONOFF,	IDD_3DDIALOGS,FIELDOFFSET(Preferences,bUse3dControls),	},
  { citNULL }	// End of array marker
};

class PreferencesDialog : public TransferPlusDlg {
  typedef TransferPlusDlg Inherited;
 public:
  PreferencesDialog(PTWindowsObject AParent, Preferences *pPreferences, PTModule AModule = NULL)
   : TransferPlusDlg(AParent, ID_PREFERENCESDIALOG, pPreferences, PreferencesControlInfo, AModule)
  {
  }
  virtual void SetupWindow()
  {
    Inherited::SetupWindow();
    Centralize(this);
  }
  virtual void HandleBrowse(RTMessage) = [ID_FIRST+IDBROWSE];
  virtual void SetupControls(void *data);
};

#ifndef  _PEAK_APP_H
#include "peak_app.h"
#endif
void PreferencesDialog::SetupControls(void *data)
{
  Inherited::SetupControls(data);
  EnableWindow(GetDlgItem(HWindow,IDD_3DDIALOGS),((PeakApp *)GetApplication())->Ctl3dAvailable());

  #if !MODBUS()
  DestroyWindow(GetDlgItem(HWindow,IDD_MODBUS));
  DestroyWindow(GetDlgItem(HWindow,IDD_MODBUS_LABEL)); // added. P.P. 08/05/05
  #endif
  DestroyWindow(GetDlgItem(HWindow,IDD_CM)); // added. P.P. 08/03/18
  DestroyWindow(GetDlgItem(HWindow,IDD_INCH)); // added. P.P. 08/03/18
}

#ifndef  _MAINWIND_H
#include "mainwind.h"
#endif

void PreferencesDialog::HandleBrowse(RTMessage)
{
  Str FileName;
  if(((MainWindow *)(GetApplicationObject()->MainWindow))->
    GetJobFileName(HWindow, &FileName, FALSE)) {
    SetDlgItemText(HWindow,IDD_STARTUPFILE,FileName);
  }
}



#ifndef  _PREF_DLG_H
#include "pref_dlg.h"
#endif

PTWindowsObject newPreferencesDialog(PTWindowsObject AParent, Preferences *pPreferences, PTModule AModule)
{
  return new PreferencesDialog(AParent, pPreferences, AModule);
}
