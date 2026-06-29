#ifndef  _PREF_H
#define  _PREF_H

#ifndef  _WCPPLIB_STR_H
#include "wcpplib\str.h"
#endif

#ifndef  _WINUTIL_APP_H
#include "winutil\app.h"
#endif

class Preferences : public ApplicationProfile
{
 public:
  int bAutoSaveEnvironment;
  int bAutoSaveDesktop;
  int bAutoSaveWindow;
  int bAutoSaveMethod;

  int iCom;             // 0 - COM1:, 1 - COM2:
  int bInches;
  int bStartupAbout;
  Str sStartupFile;
  int bStartupWindow;
  int bUse3dControls;
  double leftMargin;	// in HiMetrics ( 1/1000 cm  or   1/2540 inch)
  double topMargin;
  double rightMargin;
  double bottomMargin;
// added for flow sensors parameters. P.P. 08/24/04
double V0_flow;
double V30_flow;
double A0_flow_S;
double exp_flow_S;
// end of added. P.P. 08/24/04
//  int GC311version; // high byte - version number, lowbyte - subversion number
//  Str GC311version;


  int iModBusCom; // 0 - nothing, 1- COM1:

  Preferences();

  void Read(void);
  void Write(void);
};


#endif
