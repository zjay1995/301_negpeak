#include "includes.h"
#pragma hdrstop

#ifndef  _PREF_H
#include "pref.h"
#endif

#ifndef  _WINUTIL_APP_H
#include "winutil\app.h"
#endif

Preferences::Preferences()
:
  bAutoSaveEnvironment(1),
  bAutoSaveDesktop    (1),
  bAutoSaveWindow     (1),
  bAutoSaveMethod     (0),
//  iCom                (1),
//  iCom                (0),  // replaced - com1 default. P.P. 03/17/05
  iCom                (4),  // replaced - none default. P.P. 10/30/09
  bInches             (0),
  bStartupAbout       (1),
  sStartupFile        ((const char *)0),
  bStartupWindow      (1)
  , bUse3dControls(1)
  ,leftMargin(2540)
  ,topMargin(2540/2)
  ,rightMargin(2540/4)
  ,bottomMargin(2540/2)
// added. P.P. 08/24/04
  ,V0_flow(1.232)
  ,V30_flow(1.892)
  ,A0_flow_S(15.116)
  ,exp_flow_S(0.699)
// end of added. P.P. 08/24/04
//  ,GC311version(0) // unknown
//  ,GC311version("2.0")
  ,iModBusCom(0)
{
  bInches=GetProfileInt("intl","iMeasure",bInches)? 1: 0;
}
// added. P.P. 08/24/04
double V0_value = 1.232;
double V30_value = 1.892;
double A0_value_S = 15.116;
double exp_value_S = 0.699;
// end of added. P.P. 08/24/04



void Preferences::Read(void)
{
  bAutoSaveEnvironment=ApplicationProfile::GetInt("AutoSave", "Environment", bAutoSaveEnvironment);
  bAutoSaveDesktop    =ApplicationProfile::GetInt("AutoSave", "Desktop",     bAutoSaveDesktop);
  bAutoSaveWindow     =ApplicationProfile::GetInt("AutoSave", "Window",      bAutoSaveWindow);
  bAutoSaveMethod     =ApplicationProfile::GetInt("AutoSave", "Method",      bAutoSaveMethod);
  iCom                =ApplicationProfile::GetInt("Preferences", "iCom",     iCom);
  iModBusCom   =ApplicationProfile::GetInt("Preferences", "iModBusCom",     iModBusCom);
  bInches             =ApplicationProfile::GetInt("Preferences", "bInches",  bInches);
//  GC311version              =ApplicationProfile::GetInt("Preferences", "GC311version", GC311version);
//  GC311version              =ApplicationProfile::GetString("Preferences", "GC311version", GC311version);
  bStartupAbout       =ApplicationProfile::GetInt("Startup",   "About",      bStartupAbout);
  sStartupFile        =ApplicationProfile::GetString("Startup",   "File",       sStartupFile);
  bStartupWindow      =ApplicationProfile::GetInt("Startup",   "Window",     bStartupWindow);
  bUse3dControls      =ApplicationProfile::GetInt("Preferences",   "bUse3dControls",     bUse3dControls);
  leftMargin  =ApplicationProfile::GetDouble("Page", "leftMargin",   leftMargin);
  topMargin   =ApplicationProfile::GetDouble("Page", "topMargin",    topMargin);
  rightMargin =ApplicationProfile::GetDouble("Page", "rightMargin",  rightMargin);
  bottomMargin=ApplicationProfile::GetDouble("Page", "bottomMargin", bottomMargin);
// added. P.P. 08/24/04
  V0_value =ApplicationProfile::GetDouble("Column Flow", "V0_flow", V0_flow);
  V30_value =ApplicationProfile::GetDouble("Column Flow", "V30_flow", V30_flow);
  A0_value_S =ApplicationProfile::GetDouble("Sample Flow", "A0_flow_S", A0_flow_S);
  exp_value_S =ApplicationProfile::GetDouble("Sample Flow", "exp_flow_S", exp_flow_S);
// end of added. P.P. 08/24/04
}

void Preferences::Write(void)
{
  ApplicationProfile::WriteInt("AutoSave", "Environment", bAutoSaveEnvironment);
  ApplicationProfile::WriteInt("AutoSave", "Desktop",     bAutoSaveDesktop);
  ApplicationProfile::WriteInt("AutoSave", "Window",      bAutoSaveWindow);
  ApplicationProfile::WriteInt("AutoSave", "Method",      bAutoSaveMethod);  // added. P.P. 03/15/05
  ApplicationProfile::WriteInt("Preferences", "iCom",     iCom);
  ApplicationProfile::WriteInt("Preferences", "iModBusCom",     iModBusCom);
  ApplicationProfile::WriteInt("Preferences", "bInches",  bInches);
  //ApplicationProfile::WriteInt("Preferences", "GC311version", GC311version);
  //ApplicationProfile::WriteString("Preferences", "GC311version", GC311version);
  ApplicationProfile::WriteInt("Startup",   "About",      bStartupAbout);
  ApplicationProfile::WriteString("Startup",   "File",       sStartupFile);
  ApplicationProfile::WriteInt("Startup",   "Window",     bStartupWindow);
  ApplicationProfile::WriteInt("Preferences", "bUse3dControls",     bUse3dControls);
  ApplicationProfile::WriteDouble("Page", "leftMargin",   leftMargin);
  ApplicationProfile::WriteDouble("Page", "topMargin",    topMargin);
  ApplicationProfile::WriteDouble("Page", "rightMargin",  rightMargin);
  ApplicationProfile::WriteDouble("Page", "bottomMargin", bottomMargin);
// added to write coefficient's default values for flow sensors. P.P. 08/24/04
  ApplicationProfile::WriteDouble("Column Flow", "V0_flow", V0_value);
  ApplicationProfile::WriteDouble("Column Flow", "V30_flow", V30_value);
  ApplicationProfile::WriteDouble("Sample Flow", "A0_flow_S", A0_value_S);
  ApplicationProfile::WriteDouble("Sample Flow", "exp_flow_S", exp_value_S);
// end of added. P.P. 08/24/04
}

/* ------------------------------------------------------------------- */


