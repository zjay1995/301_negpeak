#include "includes.h"
#pragma hdrstop

#ifndef __OWL_H
#include <owl.h>
#endif

#ifndef  _WINLIB_MENU_H
#include "winlib\menu.h"
#endif

#ifndef  _WINUTIL_THINKING_H
#include "winutil\thinking.h"
#endif
#ifndef  _WINUTIL_APP_H
#include "winutil\app.h"
#endif


#ifndef  _ID_H
#include "id.h"
#endif
#ifndef  _METHOD_H
#include "method.h"
#endif
#ifndef  _JOB_H
#include "job.h"
#endif
#ifndef  _ABOUT_H
#include "about.h"
#endif

#ifndef  _PEAK_APP_H
#include "peak_app.h"
#endif

#ifndef  _MAINWIND_H
#include "mainwind.h"
#endif
#ifndef  _OUTPWIND_H
#include "outpwind.h"
#endif
#ifndef  _WINUTIL_TOOLBAR_H
#include "winutil\toolbar.h"
#endif
#ifndef __COMMDLG_H
#include <commdlg.h>
#endif
#ifndef  _DRAW_H
#include "draw.h"
#endif

UINT commdlg_help=RegisterWindowMessage(HELPMSGSTRING);
UINT commdlg_id=0;



LPSTR MainWindow::GetClassName()
{
  return "MainWindow";
}

void MainWindow::GetWindowClass(WNDCLASS& WndClass)
{
  Inherited::GetWindowClass(WndClass);
  WndClass.hIcon = LoadIcon(GetModule()->hInstance,MAKEINTRESOURCE(ID_WPEAK_ICON));
}



MainWindow::MainWindow(LPSTR ATitle, int MenuId, LPSTR HelpFile, Preferences &_Pref, PTModule AModule)
 : FrameWindow(ATitle, MenuId, HelpFile, AModule),
 pJob(0), pRun(0),
 pIntegration(0),
 bF1Pressed(FALSE)
 , Pref(_Pref)
#if !_NOCOMM()
 , pComm(0)
// , pComm(4)   // P.P. 11/06/09, for "none"
#endif
 , pTempCalResults(new CalResults)
 , cur_point(0)
#if MODBUS()
 , modBus(0)
#endif
// added. P.P. 07/01/06
 , bOvenOn(0)
 , bLampOn(0)
 , bAutoZero(0) // added here 12/06/09
 , bAutoZeroB(0) // added 07/14/17
 , bFan(0)     // added P.P. 02/23/10
 , bHeat(0)    // added. P.P. 05/02/13  here 10/24/14
//#if GC321()
#if GC321()||GC326() // P.P. 12/08/18
 , bDoorOn(0)
#endif
// end. P.P. 07/01/06
 , bSample(0)    // added. P.P. 03/14/17
 , bEquilibrate(0)    // added. P.P. 03/14/17
 , bInject(0)    // added. P.P. 03/14/17
 , bDesorb(0)    // added. P.P. 03/14/17
 , bPurge(0)    // added. P.P. 03/16/17
 , bInj_V4(0)    // added. P.P. 03/18/17
 , bLineHeat(0)  // added. P.P. 03/28/17
 , bFlowLow(0)   // P.P. 05/13/18
 , bHeatInj(0)    // added. P.P. 12/12/18  
{
  pRunPictInfo=new RunPictInfo;
  {
    for(int n=0; n<NUMDETECTORS+1; n++) {
      pPictInfo[n]=new PictInfo;
    }
  }
  {
    for(int n=0; n<NUMDETECTORS; n++) {
      pPeakList[n]=NULL;
    }
  }
  ChildMenuPos = -1;
  if(Pref.bStartupWindow) {
    Attr.X=Pref.ApplicationProfile::GetInt("Window","X",Attr.X);
    Attr.Y=Pref.ApplicationProfile::GetInt("Window","Y",Attr.Y);
    Attr.W=Pref.ApplicationProfile::GetInt("Window","W",Attr.W);
    Attr.H=Pref.ApplicationProfile::GetInt("Window","H",Attr.H);
    int z=Pref.ApplicationProfile::GetInt("Window","IsZoomed",0);
    if(z) GetApplication()->nCmdShow=SW_SHOWMAXIMIZED;
  }
  if(tb) {
    tb->append(CM_FILENEW);
    tb->append(CM_FILEOPEN);
    tb->append(CM_FILESAVE);
    tb->append(CM_PRINT);
    tb->append(CM_EXIT);
    tb->append(0); // separator
    tb->append(CM_ACQUIRE);
    tb->append(CM_INTEGRATE);
    tb->append(CM_STOPINTEGRATION);
    tb->append(CM_SELECT_RUN);
    tb->append(0); // separator // 5 lines added. P.P. 07/01/06
//#if GC321() // no such icons and menu items in regular GC. P.P. 07/10/06   Restored. P.P. 07/22/06
#if !(TCD()&&FID()) // cond added - no lamp at all. P.P. 02/18/11
    tb->append(CM_STARTLAMP);   //
    tb->append(CM_STOPLAMP);    //
#endif // tcd + fid
 // removed 03/23/08 temp, for today's GC322  restored 10/24/08
//#if GC321()  // moved up. P.P. 07/10/06
#if GC321()||GC326() // P.P. 12/08/18
    tb->append(CM_OPENDOOR); //  P.P. 07/01/06
    tb->append(CM_CLOSEDOOR); //  P.P. 07/01/06
#endif // gc321
#if CONCENTRATOR()     // two buttons added. P.P. 02/23/10
//    tb->append(CM_FAN);
//    tb->append(CM_FAN_OFF); 
#endif // concentrator
    tb->append(CM_AUTOZERO); // added P.P. 12/04/09
    tb->append(CM_AUTOZ_OF); // added P.P. 12/04/09
    tb->append(CM_AUTOZ_B); // added P.P. 07/14/17
    tb->append(CM_AUTOZ_B_OFF); // added P.P. 07/14/17

    tb->append(0); // separator
    tb->append(CM_HELPCONTENTS);
    tb->append(0);                  //
    tb->append(0);                  //  added. P.P. 11/11/09
    tb->append(CM_ZOOM_IN);         //
    tb->append(CM_ZOOM_OUT);        //
    tb->append(CM_SCALEIN);           //  P.P. 01/22/13
    tb->append(CM_SCALEOUT);        //
    tb->append(0); // separator     // added 2, may be move even further. P.P. 05/02/13 here 10/24/14
    tb->append(0); // separator     //
    tb->append(IDD_HEAT_ON);        // added. P.P. 04/30/13
    tb->append(0); // separator     //
    tb->append(IDD_HEAT_INJ_ON);        // added. P.P. 12/12/18
    tb->append(0); // separator     //
    tb->append(IDD_SAMPLE);        // added. P.P. 03/14/17
    tb->append(0); // separator     //
    tb->append(IDD_EQUILIBRATE);        // added. P.P. 03/14/17
    tb->append(0); // separator     //
    tb->append(IDD_INJECT);        // added. P.P. 03/14/17
    tb->append(0); // separator     //
#if CONCENTRATOR()     // desorb indicator added. P.P. 03/14/17
    tb->append(IDD_DESORB);        // added. P.P. 03/14/17
    tb->append(0); // separator     //
    tb->append(IDD_PURGE);        // added. P.P. 03/18/17
    tb->append(0); // separator     //
#endif // concentrator
    tb->append(IDD_FAN);        // added. P.P. 03/18/17
    tb->append(0); // separator     //
#if CONCENTRATOR()
    tb->append(IDD_INJ_V4);        // added. P.P. 03/18/17
#endif
#if HEATED_LINE()
    tb->append(0); // separator     //
    tb->append(IDD_LINE_HEAT);        // added. P.P. 03/28/17
#endif
#if FLOW_RATE()
    tb->append(0); // separator     //
    tb->append(IDD_FLOW_HIGH);        // added. P.P. 05/13/18
    tb->append(0); // separator     //
    tb->append(IDD_FLOW_LOW);        // added. P.P. 05/13/18
    EnableCommand(IDD_FLOW_HIGH, !bFlowLow);  // added. P.P. 05/02/13
    EnableCommand(IDD_FLOW_LOW, bFlowLow);  // added. P.P. 05/02/13
#endif

    EnableCommand(IDD_HEAT_ON, bHeat);  // added. P.P. 05/02/13

  }


 #if MODBUS()
 StartModBus();
 #endif

}

#include "acquire.h"      // P.P. 05/30/04
MainWindow::~MainWindow()
{
  DeletePeakList();
  for(int n=0; n<NUMDETECTORS+1; n++) {
    delete pPictInfo[n];
  }
  delete pRunPictInfo;
  ClearComm();
  delete pTempCalResults;
#if !DEMO() // added. P.P. 10/09/04
#if INTERNAL()
  StartInternal(FALSE); // added. P.P. 05/30/04
  Acquire::Lamp(FALSE); // added. P.P. 03/31/07
  bLampOn = 0;          //
  Acquire::Fan(FALSE);  // P.P. 02/23/10
  bFan = 0;
#endif // INTERNAL()
#endif // !demo

#if MODBUS()
  delete modBus;
#endif
}

//#include "acquire.h"  moved up. P.P. 05/30/04

void MainWindow::SetupWindow()
{
  Inherited::SetupWindow();
/*
  if(!InitInternal()) {     // cond added to exit if the USB_1608FS board # is wrong. P.P. 01/01/07
    ::PostMessage(HWindow,WM_COMMAND,CM_AUTOEXEC,0L);
  }
  else {
    ::PostMessage(HWindow,WM_COMMAND,CM_EXIT,0L);
    return;
  }
*/
  EnableCommand(CM_FILENEW, TRUE);
  EnableCommand(CM_FILEOPEN, TRUE);
  EnableCommand(CM_PRINTERSETUP, TRUE);
  EnableCommand(CM_HELPCONTENTS, TRUE);
  EnableCommand(CM_EXIT, TRUE);
  EnableCommand(CM_EDIT_RUNMODE, TRUE);
  EnableCommand(CM_ZOOM_IN, TRUE);   // P.P. 11/11/09
  EnableCommand(CM_ZOOM_OUT, TRUE);   // P.P. 11/11/09
  EnableCommand(CM_AUTOZERO, TRUE); // added P.P. 08/28/09 here 12/04/09
  EnableCommand(CM_AUTOZ_OF, TRUE); // added P.P. 08/28/09
  EnableCommand(CM_SCALEIN, TRUE);   // P.P. 01/22/13
  EnableCommand(CM_SCALEOUT, TRUE);   // 
//#if GC321() // cond added. P.P. 07/10/06   Restored. 07/22/06
//#if !(STAND_ALONE()&&TCD())
#if (!(STAND_ALONE()&&TCD()))||FID()&&TCD() // no lamp present if both are not PID. P.P. 02/18/11
  EnableCommand(CM_STARTLAMP, TRUE); // 2 lines added. P.P. 06/24/06
  EnableCommand(CM_STOPLAMP, TRUE);
#endif // ! alone+tcd
// lamp B menu item deleted because of TCD is Det B in GC312 with 2 det. 02/20/09
#if GC312()&&TCD() // then det B is TCD. P.P. 02/20/09
  DeleteMenu(GetMenu(HWindow),CM_STARTLAMPB, MF_BYCOMMAND);  // 4 lines added. P.P. 06/22/06
  DeleteMenu(GetMenu(HWindow),CM_STOPLAMPB, MF_BYCOMMAND);   // and removed. Only 1 detector. P.P. 06/24/06
#if FID() // TCD + FID case - no lamp at all. P.P. 02/18/11
  DeleteMenu(GetMenu(HWindow),CM_STARTLAMP, MF_BYCOMMAND);
  DeleteMenu(GetMenu(HWindow),CM_STOPLAMP, MF_BYCOMMAND);   
  DeleteMenu(GetMenu(HWindow),CM_STARTFLAMEDETECTOR, MF_BYCOMMAND);  // 2 lines added. P.P. 02/18/11
  DeleteMenu(GetMenu(HWindow),CM_STOPFLAMEDETECTOR, MF_BYCOMMAND);
#endif // fid
#elif STAND_ALONE()&&TCD() // case added. P.P. 06/26/09
  DeleteMenu(GetMenu(HWindow),CM_STARTLAMP, MF_BYCOMMAND);  
  DeleteMenu(GetMenu(HWindow),CM_STOPLAMP, MF_BYCOMMAND);   
  DeleteMenu(GetMenu(HWindow),CM_STARTLAMPB, MF_BYCOMMAND);  
  DeleteMenu(GetMenu(HWindow),CM_STOPLAMPB, MF_BYCOMMAND);   
#else
  EnableCommand(CM_STARTLAMPB, TRUE);  // restored - again 2 detectors. 03/23/08
  EnableCommand(CM_STOPLAMPB, TRUE);
#endif // gc312 and TCD
#if INTERNAL()
  DeleteMenu(GetMenu(HWindow),CM_METHODDOWNLOAD, MF_BYCOMMAND); // added. Not needed if Internal.P.P. 06/26/09
#endif
//#if GC321() // added. Only for GC322/321 at this time. P.P. 06/30/06 moved down. 07/07/06
#if STAND_ALONE()  // case added. 06/26/09
  DeleteMenu(GetMenu(HWindow),CM_ENABLEOVEN, MF_BYCOMMAND);
  DeleteMenu(GetMenu(HWindow),CM_DISABLEOVEN, MF_BYCOMMAND);
#else
  EnableCommand(CM_ENABLEOVEN, TRUE);
  EnableCommand(CM_DISABLEOVEN, TRUE);
#endif // stand_alone
//  EnableCommand(CM_OPENDOOR, TRUE);    // P.P. 07/01/06
//  EnableCommand(CM_CLOSEDOOR, TRUE);   //
//#if GC321() // added. Only for GC322/321 at this time. P.P. 06/30/06  moved up. P.P. 07/10/06
#if GC321()||GC326() // P.P. 12/07/18 // added. Only for GC322/321 at this time. P.P. 06/30/06  moved up. P.P. 07/10/06
//#if 0 // temp, for today's gc322. 03/23/08
//#if 1 // Restored P.P. 10/24/08 removed 02/20/09 for GC312. GC321 restored
  EnableCommand(CM_OPENDOOR, !bDoorOn);    // P.P. 07/01/06
  EnableCommand(CM_CLOSEDOOR, bDoorOn);   //
#else
  DeleteMenu(GetMenu(HWindow),CM_OPENDOOR, MF_BYCOMMAND);      //    P.P. 07/01/06
  DeleteMenu(GetMenu(HWindow),CM_CLOSEDOOR, MF_BYCOMMAND);     //
#endif
#if !CONCENTRATOR()
//  DeleteMenu(GetMenu(HWindow),CM_FAN, MF_BYCOMMAND);      //    P.P. 02/23/10
//  DeleteMenu(GetMenu(HWindow),CM_FAN_OFF, MF_BYCOMMAND);     //
#endif // !concentrator
//#if GC501()                                // added. P.P. 07/11/04
#if GC501()||SHIFT_REPORT()                  // changed. P.P. 08/18/04
  EnableCommand(CM_EDIT_SHIFTMODE, TRUE);
#else                                      //
  EnableCommand(CM_EDIT_SHIFTMODE, FALSE); //
#endif                                     //
  NewJob();

#if INTERNAL()
  wtimer=new WTimer(HWindow, 1234, 1000, NULL); // 1 sec

#if !DEMO() // added. P.P. 10/09/05
//  InitInternal(); // I think all initailization we must do on begin acquire W.K.
// Autoexec moved here. P.P. 01/01/07
//  int check = InitInternal();  // rem P.P. 03/29/18 to test
//  if(check==1) {     // cond added to exit if the USB_1608FS board # is wrong. P.P. 03/15/07
  if(1) {     // cond ignored. P.P. 03/15/17, program runs OK for any board
    ::PostMessage(HWindow,WM_COMMAND,CM_AUTOEXEC,0L);
  }
  else {
    ::PostMessage(HWindow,WM_COMMAND,CM_EXIT,0L);
    return;
  }
// end


  StartInternal(TRUE); // added. P.P. 05/30/04  // rem P.P. 03/29/18 to test   restored P.P. 12/09/18
#else // demo
  EnableCommand(CM_ACQUIRE, FALSE);
  EnableCommand(CM_ACQUIRE1, FALSE);
  EnableCommand(CM_CALIBRATE, FALSE);
  EnableCommand(CM_CALIBRATE1, FALSE);
  EnableCommand(CM_STOPPUMP, FALSE);
  EnableCommand(CM_STOPALARM, FALSE);
  EnableCommand(CM_STARTTCDDETECTOR, FALSE);
  EnableCommand(CM_STOPTCDDETECTOR, FALSE);
  EnableCommand(CM_CHECKCALIBRATE, FALSE);
  EnableCommand(CM_CHECKCALIBRATE1, FALSE);
  EnableCommand(CM_STARTFLAMEDETECTOR, FALSE);
  EnableCommand(CM_STOPFLAMEDETECTOR, FALSE);
//#if GC321() // cond added. P.P. 07/10/06
#if GC321()||GC326() // P.P. 12/08/18
  EnableCommand(CM_STARTLAMP, FALSE);  // 2 lines added. P.P. 06/21/06
  EnableCommand(CM_STOPLAMP, FALSE);   //
  EnableCommand(CM_OPENDOOR, FALSE);    // added here P.P. 12/08/18
  EnableCommand(CM_CLOSEDOOR, FALSE);   //
#endif // gc321
#endif // !demo
#endif // internal

}

BOOL MainWindow::CanClose()
{
  if(Pref.bAutoSaveEnvironment) Pref.Write();
  if(Pref.bAutoSaveDesktop) {
    const char *DskFile=GetDesktopName();
    if(DskFile) SaveDesktop(DskFile);
  }
  if(Pref.bAutoSaveWindow) SaveWindow();
  DeleteJob();
  ClearComm();
  return Inherited::CanClose();
}

void MainWindow::WMDestroy(RTMessage Msg) // When Windows shutdown WM_DESTROY not called?
{
  DeleteJob();
  ClearComm();
#if INTERNAL()
  delete wtimer;
#endif
  Inherited::WMDestroy(Msg);
}

void MainWindow::SaveWindow()
{
  Pref.ApplicationProfile::WriteInt("Window","X",Attr.X);
  Pref.ApplicationProfile::WriteInt("Window","Y",Attr.Y);
  Pref.ApplicationProfile::WriteInt("Window","W",Attr.W);
  Pref.ApplicationProfile::WriteInt("Window","H",Attr.H);
  Pref.ApplicationProfile::WriteInt("Window","IsZoomed",IsZoomed(HWindow));
}

void MainWindow::SaveDesktopInfo(Ropstream os)
{
  for(int n=0; n<NUMDETECTORS+1; n++) {
    pPictInfo[n]->write(os);
  }
  pRunPictInfo->write(os);
}

void MainWindow::RestoreDesktopInfo(Ripstream is)
{
  for(int n=0; n<NUMDETECTORS+1; n++) {
    pPictInfo[n]->read(is);
  }
  pRunPictInfo->read(is);
}

void ReadRunMode(ApplicationProfile &profile);
void ReadFlowMode(ApplicationProfile &profile); // added. P.P. 12/31/03
void MainWindow::CMAutoexec(RTMessage)
{
  ReadRunMode(Pref);
#if FLOW()              // added. P.P. 12/31/03
//  ReadFlowMode(Pref);   // removed. P.P. 01/09/04 restored 12/07/18 rem temp - gives error
#endif                  //

  if(Pref.bStartupAbout) {
    ::PostMessage(HWindow,WM_COMMAND,CM_ABOUT,0L);
  }
  if(!Pref.sStartupFile.IsEmpty()) {
    FileOpen(Pref.sStartupFile);
  }
  // P.P. 10/27/01. Added to automatically run Continuous on Startup.
  //::PostMessage(HWindow,WM_COMMAND,CM_ACQUIRE,0L);
   // In GC321, only manual mode - next line removed. P.P. 04/20/02
#if !DEMO() // added. P.P. 10/09/04
//#if !GC321()   // P.P. 06/13/02
//#if !(GC321()||GC312())   // gc312 added for not running run on start. P.P. 06/05/09
#if !(GC321()||GC312()||GC326())   // gc325 added for not running run on start. P.P. 12/07/18
#if !STAND_ALONE() // added. P.P. 03/19/04
#if NO_CAL_START() // added. P.P. 10/12/03
  ::PostMessage(HWindow,WM_COMMAND,CM_ACQUIRE,0L); //
#else // regular case - goes to cal
  ::PostMessage(HWindow,WM_COMMAND,CM_CALIBRATE1,0L); // W.K. Calibrate in auto mode
#endif // no_cal_start()
#endif // !stand_alone
#endif // !GC321
#endif // !demo. P.P. 10/09/04
}

    
void MainWindow::ActivationResponse(WORD Activated,  BOOL IsIconified)
{
  EnableCommand(CM_SAVEREPORT, pJob && Activated);
  EnableCommand(CM_PRINTREPORT, pJob && Activated);
  EnableCommand(CM_PAGE_SETUP,Activated);
  Inherited::ActivationResponse(Activated, IsIconified);
}   
    
void MainWindow::WMInitMenuPopup(RTMessage Msg)
{
  {
    Menu menu((HMENU)Msg.WParam);

    menu.EnableItem(CM_EDITCUT, MF_GRAYED);
    menu.EnableItem(CM_EDITCOPY, MF_GRAYED);
    menu.EnableItem(CM_EDITPASTE, MF_GRAYED);
    menu.EnableItem(CM_EDITDELETE, MF_GRAYED);
    menu.EnableItem(CM_EDITCLEAR, MF_GRAYED);
    menu.EnableItem(CM_EDITSELECTALL, MF_GRAYED);
    menu.EnableItem(CM_EDITCOPYOUTPUT, MF_GRAYED);

    Method *pMethod=pJob;
    for(int n=0; n<NUMDETECTORS; n++) {
      char str[50];
      wsprintf(str,"Detector %c...",'A'+n);
      if(pMethod) wsprintf(strchr(str,0),"\t'%s'",pMethod->det[n].Label.Get());
      menu.Modify(CM_DETECTORAEDIT+n, MF_BYCOMMAND, CM_DETECTORAEDIT+n, str);
      menu.CheckItem(CM_DETECTORAEDIT+n, pMethod && pMethod->det[n].detect_stat? MF_CHECKED: MF_UNCHECKED);
      wsprintf(str,"Table %c",'A'+n);
      if(pMethod) wsprintf(strchr(str,0),"\t'%s'",pMethod->det[n].Label.Get());
      menu.Modify(CM_WINDOW_PEAK_A+n, MF_BYCOMMAND, CM_WINDOW_PEAK_A+n, str);
      wsprintf(str,"Graph %c",'A'+n);
      if(pMethod) wsprintf(strchr(str,0),"\t'%s'",pMethod->det[n].Label.Get());
      menu.Modify(CM_WINDOW_GRAPH_A+n, MF_BYCOMMAND, CM_WINDOW_GRAPH_A+n, str);
    }

    PTWindowsObject pSummaryWindowA=FindSummaryWindow(0);
    PTWindowsObject pSummaryWindowB=FindSummaryWindow(1);
    menu.CheckItem(CM_WINDOW_SUMMARY_A, pSummaryWindowA? MF_CHECKED: MF_UNCHECKED);
    menu.CheckItem(CM_WINDOW_SUMMARY_B, pSummaryWindowB? MF_CHECKED: MF_UNCHECKED);

    PTWindowsObject pPeakWindowA=FindPeakWindow(0);
    PTWindowsObject pPeakWindowB=FindPeakWindow(1);
    menu.CheckItem(CM_WINDOW_PEAK_A, pPeakWindowA? MF_CHECKED: MF_UNCHECKED);
    menu.CheckItem(CM_WINDOW_PEAK_B, pPeakWindowB? MF_CHECKED: MF_UNCHECKED);

    PTWindowsObject pGraphWindowAll=FindGraphWindow(NUMDETECTORS);
    PTWindowsObject pGraphWindowA=FindGraphWindow(0);
    PTWindowsObject pGraphWindowB=FindGraphWindow(1);
    menu.CheckItem(CM_WINDOW_GRAPH_ALL, pGraphWindowAll? MF_CHECKED: MF_UNCHECKED);
    menu.CheckItem(CM_WINDOW_GRAPH_A, pGraphWindowA? MF_CHECKED: MF_UNCHECKED);
    menu.CheckItem(CM_WINDOW_GRAPH_B, pGraphWindowB? MF_CHECKED: MF_UNCHECKED);

    PTWindowsObject pOutputWindow=FindOutputWindow();
    menu.CheckItem(CM_WINDOW_OUTPUT, pOutputWindow? MF_CHECKED: MF_UNCHECKED);

  }

  Inherited::WMInitMenuPopup(Msg);
}

int MainWindow::GetMainMenuString(UINT id)
{
  switch(id) {
    case FileFirst:    return IDS_FILE;
    case EditFirst:    return IDS_EDIT;
    case ViewFirst:    return IDS_VIEW;
    case MethodFirst:  return IDS_METHOD;
    case RunFirst:     return IDS_RUN;
    case OptionsFirst: return IDS_OPTIONS;
    case WindowFirst:  return IDS_WINDOW;
    case HelpFirst:    return IDS_HELP;
  }
  return 0;
}


void MainWindow::CMPrinterSetup(RTMessage)
{
  Printer *pPrinter=((PeakApp *)GetApplication());
  pPrinter->Setup(this);
}


#ifndef  _TD_PLUS_H
#include "td_plus.h"
#endif
class PageSetupDialog : public TransferPlusDlg {
  typedef TransferPlusDlg Inherited;
 public:
   static ControlInfo far ci[];
   PageSetupDialog(PTWindowsObject AParent, Preferences *data, PTModule AModule = NULL)
    : TransferPlusDlg(AParent, ID_PAGESETUPDIALOG, data, ci, AModule) {}
  virtual void SetupControls(void *data);

};

void PageSetupDialog::SetupControls(void *data)
{
  Inherited::SetupControls(data);
  Preferences *Pref=(Preferences *)data;
  char str[12];
  strcpy(str,Pref->bInches? "in": "cm");
  for(int n=0; n<4; n++) {
    SendDlgItemMessage(HWindow, IDD_TOP_MARGIN_UNITS+n*2, WM_SETTEXT, 0, (LPARAM)(LPCSTR)str);
  }
}


ControlInfo far PageSetupDialog::ci[]={
  { citDouble, IDD_LEFT_MARGIN, FIELDOFFSET(Preferences,leftMargin), },
  { citDouble, IDD_TOP_MARGIN, FIELDOFFSET(Preferences,topMargin), },
  { citDouble, IDD_RIGHT_MARGIN, FIELDOFFSET(Preferences,rightMargin), },
  { citDouble, IDD_BOTTOM_MARGIN, FIELDOFFSET(Preferences,bottomMargin), },
  { citNULL }   // End of array marker
};

#include "winlib\dc.h"

void GetPage(DC &ic,WRect *rect,const WPoint &size)
{
  MainWindow *win=(MainWindow *)GetApplicationObject()->MainWindow;
  int logpixelsy=ic.GetCapability(LOGPIXELSY);
  int logpixelsx=ic.GetCapability(LOGPIXELSX);
  rect->Left()=win->Pref.leftMargin/2540*logpixelsx;
  rect->Top()=win->Pref.topMargin/2540*logpixelsy;
  rect->Right()=size.x-win->Pref.rightMargin/2540*logpixelsx;
  rect->Bottom()=size.y-win->Pref.bottomMargin/2540*logpixelsy;
}

void MainWindow::CMPageSetup(RTMessage)
{
  double factor=Pref.bInches? 2540: 1000;
  Pref.leftMargin/=factor;
  Pref.topMargin/=factor;
  Pref.rightMargin/=factor;
  Pref.bottomMargin/=factor;
  GetModule()->ExecDialog(new PageSetupDialog(this,&Pref));
  Pref.leftMargin*=factor;
  Pref.topMargin*=factor;
  Pref.rightMargin*=factor;
  Pref.bottomMargin*=factor;
}

#include "hnu.h" // CAL

void MainWindow::ChangePeakName(const char *name)
{
  char str[200];
  if(pJob) {
    pJob->FileName.Set(name);
    if(pRun) {
      int nRun=0;
      for(RunList::Node *prln=pJob->first(); prln; prln=prln->nextNode()) {
        nRun++;
        if(pRun==prln) break;
      }
      if(pRun->type_of_run==CAL) {
	wsprintf(str,APP_NAME": %s (Run #%d, calibration)",pJob->FileName.Get(),nRun);
      }
      else if(pRun->type_of_run==CHECKCAL) {  // case added. P.P. 07/26/04
	wsprintf(str,APP_NAME": %s (Run #%d, check calibration)",pJob->FileName.Get(),nRun);
      }
      else {
#if POINTS()>1
#if QUICK_SCAN() // case added. P.P. 01/13/11
        if(pRun->quick_scan_run) {
	  wsprintf(str,APP_NAME": %s (Quick scan, Run #%d, pt#%d)",pJob->FileName.Get(),nRun, 1+pRun->pointnumber);
	}
	else { // regular run
	  wsprintf(str,APP_NAME": %s (Run #%d, pt#%d)",pJob->FileName.Get(),nRun, 1+pRun->pointnumber);
	}
#else
	wsprintf(str,APP_NAME": %s (Run #%d, pt#%d)",pJob->FileName.Get(),nRun, 1+pRun->pointnumber);
#endif // quick scan
#else
	wsprintf(str,APP_NAME": %s (Run #%d)",pJob->FileName.Get(),nRun);
#endif
      }
    }
    else if(!pJob->FileName.IsEmpty())
      wsprintf(str,APP_NAME": %s",pJob->FileName.Get());
    else
      wsprintf(str,APP_NAME);
  }
  else {
    wsprintf(str,APP_NAME);
  }
  SetCaption(str);
}

/* ------------------------------------------------------------------- */


#ifndef  _PREF_DLG_H
#include "pref_dlg.h"
#endif

void MainWindow::CMOptionsPreferences(RTMessage)
{
#if MODBUS()
  int iModBusComOld=Pref.iModBusCom; // Store old value
#endif

  GetModule()->ExecDialog(newPreferencesDialog(this,&Pref));

#if MODBUS()
  if(iModBusComOld!=Pref.iModBusCom) // if port changed
    StartModBus(); // restart
#endif


  PeakApp *app=(PeakApp *)GetApplication();
  if(app->Ctl3dAvailable()) {
    if(Pref.bUse3dControls && !app->Ctl3dEnabled()) {
      ((PeakApp *)GetApplication())->EnableCtl3d(TRUE);
      ((PeakApp *)GetApplication())->EnableCtl3dAutosubclass(TRUE);
    }
    else if(!Pref.bUse3dControls && app->Ctl3dEnabled()) {
      ((PeakApp *)GetApplication())->EnableCtl3dAutosubclass(FALSE);
      ((PeakApp *)GetApplication())->EnableCtl3d(FALSE);
    }
  }
}

/* ------------------------------------------------------------------- */

const char far *MainWindow::GetDesktopName()
{
  static char DskFile[80];
  if(pJob && !pJob->FileName.IsEmpty()) {
    pJob->MakeName(DskFile,"wnd");
    return DskFile;
  }
  return NULL;
}

#ifndef  _OPTS_DLG_H
#include "opts_dlg.h"
#endif

void MainWindow::CMOptionsSave(RTMessage)
{
  OptionsSaveStruct data={1,1,1};
  if(IDCANCEL!=GetModule()->ExecDialog(newOptionsSaveDialog(this,&data))) {
    if(data.bSaveEnvironment) Pref.Write();
    if(data.bSaveDesktop && pJob) {
      const char *DskFile=GetDesktopName();
      if(DskFile) SaveDesktop(DskFile);
    }
    if(data.bSaveWindow) SaveWindow();
    //if(TransferData[2] && pMethod) pMethod->Save();
  }
}

void MainWindow::WMEnterIdle( RTMessage Msg )
{
 if( (GetKeyState( VK_F1 ) & 0x8000) != 0) {
   if(Msg.WParam == MSGF_MENU ) {
     // if the high bit is set, then the key is pressed
     bF1Pressed = TRUE;
     PostMessage( HWindow, WM_KEYDOWN, VK_RETURN, 0L );
   }
   else if(Msg.WParam == MSGF_DIALOGBOX) {
     //HWND hwndDlg = (HWND)Msg.LP.Lo;
     if(commdlg_id) PostMessage( HWindow, commdlg_help, 0, 0L );
   }
 }
}

void MainWindow::WMCommand(RTMessage Msg)
{
  if(bF1Pressed) { // F1 pressed in menu
    bF1Pressed=FALSE;
    WinHelp( HWindow, szHelpFileName, HELP_CONTEXT, Msg.WParam ); // rem temp. P.P. 08/12/05
  }
  else {
    Inherited::WMCommand(Msg);
  }
}

void MainWindow::DefWndProc(RTMessage Msg)
{
  if(Msg.Message==commdlg_help) { // Common Dialog notification about HELP button pressing
        // commdlg_id must be set before Common Dialog calling
//    WinHelp( HWindow, szHelpFileName, HELP_CONTEXT, commdlg_id); // rem temp. P.P. 08/12/05
  }
  else {
    Inherited::DefWndProc(Msg);
  }
}

#ifndef  _HELPIDS_H
#include "helpids.h"
#endif
void MainWindow::CMHelpContents(RTMessage)
{
  WinHelp(HWindow,szHelpFileName,HELP_CONTENTS,0L); // rem temp. P.P. 08/12/05
}
void MainWindow::CMHelpOnHelp(RTMessage)
{
  WinHelp(HWindow,szHelpFileName,HELP_HELPONHELP,0L); // rem temp. P.P. 08/12/05
}

void MainWindow::CMAbout(RTMessage)
{
  //Thinking think("Loading About dialog ...");
  GetModule()->ExecDialog/*MakeWindow*/( newAboutDialog(this,ID_ABOUTDIALOG) );
}


#if MODBUS()
#include "debug.h"
void MainWindow::StartModBus()
{
  StopModBus();

  if(Pref.iModBusCom>0) {
    char str[40];
    wsprintf(str,"COM%d",Pref.iModBusCom);
    TRACE1("new ModBusSlave=%s",str);
    modBus=new ModBusServer(str);
    wsprintf(str,"COM%d:9600,n,8,1",Pref.iModBusCom);
    modBus->SetParam(str);
  }
}
void MainWindow::StopModBus()
{
  delete modBus;
  modBus=NULL;
}
#endif


/* ------------------------------------------------------------------ */
#ifndef  _MAIN_WND_H
#include "main_wnd.h"   // newMainWindow()
#endif

PTWindowsObject newMainWindow(char far *ATitle, int MenuId, LPSTR HelpFile, Preferences &Pref, PTModule AModule)
{
  return new MainWindow(ATitle,MenuId,HelpFile,Pref,AModule);
}




