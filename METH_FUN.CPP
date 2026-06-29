#include "includes.h"
#pragma hdrstop

#ifndef __STRING_H
#include <string.h>
#endif
#ifndef __COMMDLG_H
#include <commdlg.h>
#endif

#ifndef  _WCPPLIB_MESSAGE_H
#include "wcpplib\message.h"
#endif

#ifndef  _ID_H
#include "id.h"
#endif

#ifndef  _OUTPWIND_H
#include "outpwind.h"
#endif
#ifndef  _PEAKLIST_H
#include "peaklist.h"
#endif
#ifndef  _CALC_H
#include "calc.h"
#endif
#ifndef  _COMM_H
#include "comm.h"
#endif
#ifndef  _ACQUIRE_H
#include "acquire.h"
#endif
extern UINT commdlg_help;
extern UINT commdlg_id;


#include "debug.h"



void LoadFilterString(HINSTANCE hinst, UINT idResource, LPSTR lpszBuffer, int cbBuffer)
{
  int cbString=LoadString(hinst, idResource, lpszBuffer, cbBuffer);
  char chReplace = lpszBuffer[cbString-1];      // retrieve wildcard
  for(LPSTR cp=lpszBuffer; *cp; cp++) {
    if(*cp == chReplace) *cp='\0';
  }
}
#ifndef  _MAINWIND_H
#include "mainwind.h"
#endif

BOOL MainWindow::GetJobFileName(HWND hwnd, Str *FileName, BOOL fSave)
{
  HINSTANCE hInstance=GetApplication()->hInstance;
  Str strFilter(128);
  LoadFilterString(hInstance, fSave? IDS_JOB_FILTER: IDS_METHOD_JOB_FILTER, strFilter.GetPtr(), 128);
  Str strTitle(64);
  LoadString(hInstance, fSave? IDS_SAVE_JOB_AS: IDS_LOAD_JOB_FROM, strTitle.GetPtr(), 64);
  static Str staticFileName;
  if(!fSave) staticFileName="";
  else { const char *cp=strrchr(staticFileName,'\\'); if(cp) staticFileName.Set(cp+1); }
  BOOL retValue=OpenSaveDialog(hwnd, strTitle, strFilter, &staticFileName, fSave);
  if(retValue) {
    *FileName=staticFileName;
  }
  return retValue;
}

#include "wcpplib\3dui.h"
#ifndef OFN_LONGNAMES
#define OFN_LONGNAMES                0x00200000
#endif

BOOL MainWindow::OpenSaveDialog(HWND hwnd, const char *title, const char *filter, Str *FileName, BOOL fSave, int *nIndex)
{
  HINSTANCE hInstance=GetApplicationObject()->hInstance;
  BOOL retValue;
  /*static */char szFile[256];
  static char _far szFileTitle[256];
  {
    strcpy(szFile,*FileName);

    OPENFILENAME ofn;
    memset(&ofn, 0, sizeof(OPENFILENAME));      // Set all structure members to zero.
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hInstance = hInstance;
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = filter;
    ofn.nFilterIndex = 1+(nIndex? *nIndex: 0);
    ofn.lpstrFile= szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFileTitle = szFileTitle;
    ofn.nMaxFileTitle = sizeof(szFileTitle);
    ofn.lpstrInitialDir = NULL; //szDirName;
    ofn.Flags = OFN_SHOWHELP | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
    ofn.Flags |= fSave? OFN_OVERWRITEPROMPT|OFN_NOREADONLYRETURN: OFN_FILEMUSTEXIST;
    ofn.lpstrTitle = title;
    ofn.lpstrDefExt = 0;

    /*
    if(__Has3dUI()) {
      ofn.Flags |= OFN_LONGNAMES; // Cannot write .WND file to dir>8 chars long
    }
    */

    /*
    if(fSave) {
      ofn.Flags |= OFN_ENABLETEMPLATE;
      ofn.lpTemplateName=MAKEINTRESOURCE(ID_OPENDLG);
    }
    */

    commdlg_id=fSave? ID_SAVEDIALOG : ID_OPENDIALOG;
    retValue=fSave? GetSaveFileName(&ofn): GetOpenFileName(&ofn);
    commdlg_id=0;
    if(retValue) {
      if(nIndex) {
	*nIndex=ofn.nFilterIndex-1;
      }
      if(ofn.nFileExtension && !szFile[ofn.nFileExtension]) {
        const char *cp=filter;
        cp+=lstrlen(cp)+1;
        for(int i=0; i<(ofn.nFilterIndex-1); i++) {
          cp+=lstrlen(cp)+1;
          cp+=lstrlen(cp)+1;
        }
        cp=strchr(cp,'.');
        if(cp) {
          lstrcat(szFile,cp);
        }
      }
    }
  }
  if(!retValue) {
    DWORD dwError=CommDlgExtendedError();
    if(dwError) {
      message("CommDlg error %lu",dwError);
    }
    return FALSE;
  }

  if(FileName) *FileName=szFile;

  return TRUE;
}

#ifndef  _METHOD_H
#include "method.h"
#endif
#ifndef  _JOB_H
#include "job.h"
#endif
#ifndef  _RUN_H
#include "run.h"
#endif

void MainWindow::NewJob(void)
{
  //TRACE("NewJob");
  DeleteJob();
  ASSERT(pJob==0);
  pJob=new Job;
  //ChangePeakName(0);
  ReflectJob();
  //TRACE1("end of NewJob=%p",pJob);
  InitTWA();
#if !DEMO() // added. P.P. 10/09/04
#endif // !demo
}

#ifndef _NDEBUG
void PrintHeap();
#endif


void MainWindow::DeleteJob(void)
{
  //TRACE1("DeleteJob=%p", pJob);
  if(Pref.bAutoSaveMethod) {
    if(pJob && !pJob->FileName.IsEmpty()) {
      pJob->SaveMethod();
    }
  }
  if(Pref.bAutoSaveDesktop) {
    const char *DskFile=GetDesktopName();
    if(DskFile) SaveDesktop(DskFile);
  }
  // DeleteIntegration(); in DeleteRun()
  DeleteRun();
  CloseChildren();
  destroy(pJob); pJob=0;
  ChangePeakName(0);
  //ReflectJob();
  #ifndef _NDEBUG
  PrintHeap();
  #endif
}

void MainWindow::ReflectJob(void)
{
  int bJob=pJob!=0;
  EnableCommand(CM_FILESAVE,   bJob);
  EnableCommand(CM_FILESAVEAS, bJob);
  EnableCommand(CM_METHODEDIT,       bJob);
  EnableCommand(CM_METHODCOMPONENTS, bJob);
  EnableCommand(CM_METHODSTANDARDS,  bJob);
  EnableCommand(CM_DETECTORAEDIT,    bJob);
  EnableCommand(CM_DETECTORBEDIT,    bJob);
  EnableCommand(CM_METHODDOWNLOAD,   bJob);
  EnableCommand(CM_METHODSAVE,       bJob);

#if !DEMO() // added. P.P. 10/09/04
  EnableCommand(CM_ACQUIRE,    bJob);
  EnableCommand(CM_CALIBRATE,  bJob);
  EnableCommand(CM_CHECKCALIBRATE,  bJob);
#else
  EnableCommand(CM_ACQUIRE,    FALSE);
  EnableCommand(CM_CALIBRATE,  FALSE);
  EnableCommand(CM_CHECKCALIBRATE,  FALSE);
#endif // !demo
  EnableCommand(CM_SELECT_RUN, bJob);
  //EnableCommand(CM_PRINT, bJob);
//#if GC321() // P.P. 07/10/06
#if !(TCD()&&FID()) // cond added - no lamp present. P.P. 02/18/11
  EnableCommand(CM_STARTLAMP, TRUE);  // 4 lines added. P.P. 06/22/06
  EnableCommand(CM_STOPLAMP, TRUE);   //
#endif // tcd and fid
  EnableCommand(CM_AUTOZERO, !bAutoZero); // added P.P. 08/28/09
  EnableCommand(CM_AUTOZ_OF, bAutoZero); // added P.P. 08/28/09
  EnableCommand(CM_AUTOZ_B, !bAutoZeroB); // added P.P. 07/14/17
  EnableCommand(CM_AUTOZ_B_OFF, bAutoZeroB); // added P.P. 07/14/17
//#endif // gc321
//  EnableCommand(CM_STARTLAMPB, TRUE);  // 
//  EnableCommand(CM_STOPLAMPB, TRUE);   //
}

void MainWindow::ReflectRun(void)
{
  int bRun=pJob!=0 && pRun!=0;
  EnableCommand(CM_INTEGRATE,   bRun);
  EnableCommand(CM_EDIT_RUN,    bRun);
  #ifdef FILTER
  EnableCommand(CM_APPLYFILTER, bRun);
  #endif//FILTER
  EnableCommand(CM_SAVECALRES,  bRun);
  ChangePeakName(pJob? pJob->FileName.Get(): 0);

#if INTERNAL()
#if TCD() // added. P.P. 07/10/04
#if !DEMO() // added. P.P. 10/09/04
  EnableCommand(CM_STARTTCDDETECTOR, TRUE);
  EnableCommand(CM_STOPTCDDETECTOR, TRUE);
#else
  EnableCommand(CM_STARTTCDDETECTOR, FALSE);
  EnableCommand(CM_STOPTCDDETECTOR, FALSE);
#endif // !demo
#else // no tcd
  DeleteMenu(GetMenu(HWindow),CM_STARTTCDDETECTOR, MF_BYCOMMAND); // added. P.P. 07/10/04
  DeleteMenu(GetMenu(HWindow),CM_STOPTCDDETECTOR, MF_BYCOMMAND);  //
#endif // TCD
// code added for flame. P.P. 06/28/05
#if FID() // added. P.P. 06/28/05
#if !DEMO() 
  EnableCommand(CM_STARTFLAMEDETECTOR, TRUE);
  EnableCommand(CM_STOPFLAMEDETECTOR, TRUE);
#else
  EnableCommand(CM_STARTFLAMEDETECTOR, FALSE);
  EnableCommand(CM_STOPFLAMEDETECTOR, FALSE);
#endif // !demo
#else
  DeleteMenu(GetMenu(HWindow),CM_STARTFLAMEDETECTOR, MF_BYCOMMAND); // added. P.P. 07/10/04
  DeleteMenu(GetMenu(HWindow),CM_STOPFLAMEDETECTOR, MF_BYCOMMAND);  //
#endif // FID()
// end of added code. P.P. 06/28/05
/*  // replaced for the code below, for the case with flow control. P.P. 12/08/18
//  EnableCommand(CM_EDIT_FLOWMODE, FALSE); // added to disable. P.P. 05/25/04
  DeleteMenu(GetMenu(HWindow),CM_EDIT_FLOWMODE, MF_BYCOMMAND);  // removed. P.P. 08/20/05
*/
//#if FLOW()   // new conditions for the case of flow. P.P. 12/08/18
#if FLOW()&&FLOW_RATE()   // new conditions for the case of flow. P.P. 12/09/18
#if !DEMO()
  EnableCommand(CM_EDIT_FLOWMODE, TRUE); // 
#else
  EnableCommand(CM_EDIT_FLOWMODE, FALSE); // disable if demo. P.P. 12/08/18
#endif // !demo
#else
  DeleteMenu(GetMenu(HWindow),CM_EDIT_FLOWMODE, MF_BYCOMMAND);  // 
#endif // flow

//#if !GC321()                      // P.P. 06/13/02 - remove these items if GC321()
#if !(GC321()||GC326())             // P.P. 12/08/18
//  DeleteMenu(GetMenu(HWindow),CM_OPENDOOR, MF_BYCOMMAND);    // P.P. 07/01/06  //rem
//  DeleteMenu(GetMenu(HWindow),CM_CLOSEDOOR, MF_BYCOMMAND);    // P.P. 07/01/06 // rem
//#if GC501()                             // cond added. P.P. 07/11/04
#if GC501()&&OLIN()      // cond changed. P.P. 08/20/05
  DeleteMenu(GetMenu(HWindow),CM_STOPPUMP, MF_BYCOMMAND); // rem from 501. P.P. 07/11/04
#elif GC501()
//  EnableCommand(CM_STOPPUMP, TRUE);    // temp. P.P. 08/20/05
  EnableCommand(CM_STOPPUMP, FALSE);    //  P.P. 08/23/05
#else                                   //
#if !DEMO() // added. P.P. 10/09/04
  EnableCommand(CM_STOPPUMP, TRUE);
#else
  EnableCommand(CM_STOPPUMP, FALSE);
#endif // !demo
#endif // gc501                         //
#if !DEMO() // added. P.P. 10/09/04
#if GC312() // added. P.P. 03/26/06
  DeleteMenu(GetMenu(HWindow),CM_STOPALARM, MF_BYCOMMAND);
#else
  EnableCommand(CM_STOPALARM, TRUE);
#endif // gc312
#else
  EnableCommand(CM_STOPALARM, FALSE);
#endif // !demo
#else // for GC321 or GC326
  DeleteMenu(GetMenu(HWindow),CM_STOPPUMP, MF_BYCOMMAND);
  DeleteMenu(GetMenu(HWindow),CM_STOPALARM, MF_BYCOMMAND);
#if !FLOW() // condition added. P.P. 12/08/18
  DeleteMenu(GetMenu(HWindow),CM_EDIT_FLOWMODE, MF_BYCOMMAND); // added. P.P. 07/16/04
#endif
  DeleteMenu(GetMenu(HWindow),CM_STARTFLAMEDETECTOR, MF_BYCOMMAND); // added. P.P. 08/20/05
  DeleteMenu(GetMenu(HWindow),CM_STOPFLAMEDETECTOR, MF_BYCOMMAND);  // 
#endif // GC321
#if !GC501() // added to leave these items only in 501. P.P. 07/11/04
  DeleteMenu(GetMenu(HWindow),CM_CHECKCALIBRATE, MF_BYCOMMAND); // rem from 301 and 321. P.P. 07/16/04
#else                                      // added. P.P. 10/09/04
  EnableCommand(CM_CHECKCALIBRATE, FALSE); //
#endif // gc501
#else  // !INTERNAL
  // code removed. P.P. 09/01/18
#endif // internal

#if !SHIFT_REPORT()
  DeleteMenu(GetMenu(HWindow),CM_WINDOW_SUMMARY_A, MF_BYCOMMAND);
  DeleteMenu(GetMenu(HWindow),CM_WINDOW_SUMMARY_B, MF_BYCOMMAND);
#endif // !shift_report

//#if 0 // removed to disable for today's variant. P.P. 05/25/04 restored P.P. 12/07/18
  EnableCommand(CM_EDIT_FLOWMODE, (!bRun) && (runMode.Acquire==0)); // Manual
//#endif
}

extern char data_rate;           // rate data received in pts/sec.
void MainWindow::NewRun(void)
{
  //TRACE("NewRun");
  if(pJob) {
    Method *pMethod=pJob;
    if(!pRun) { // if no current run
      pRun=pJob->last(); // get last run to copy cal_results
    }
    if(pRun) { // Copy cal_results from previous run
      if(pJob->GetCalResults(pRun)) // ensure to load
        memcpy(pTempCalResults->cal_results,pRun->pCalResults->cal_results,sizeof(pTempCalResults->cal_results));
      else pTempCalResults->Clear();
      // strore in the temporary variable to arange new/delete sequence
    }
    else pTempCalResults->Clear();

    DeleteRun();

    RunList::Node *prln=new RunList::Node;

    prln->AllocCalResults();
    prln->AllocRawData32(long(pMethod->method_table.analysis_time) * data_rate * NUMDETECTORS);
    memcpy(prln->pCalResults->cal_results,pTempCalResults->cal_results,sizeof(prln->pCalResults->cal_results));

    pJob->append(prln);
    pRun=prln;
    ReflectRun();
  }
  //TRACE1("end of NewRun=%p",pRun);
}

void MainWindow::NewRun(int iRun)
{
  //TRACE1("NewRun #%d",iRun);
  if(pJob) {
    DeleteRun();
    ASSERT(pRun==0);
    pRun=pJob->GetChild(iRun);
    if(pRun) if(pJob->GetCalResults(pRun)) // enshure to load
    ReflectRun();
    /*
    Run *pRunNew=pJob->GetChild(iRun);
    //#ifdef FILTER
    if(pRun==pRunNew) {
      #ifdef FILTER
      if(pRun) pRun->FreeFiltData();
      #endif
      pRun=0;
    }
    //#endif//FILTER
    DeleteRun();
    pRun=pRunNew;
    */
  }
  //ReflectRun();
  //TRACE1("end of NewRun=%p",pRun);
}




void MainWindow::SaveRun(void) // Save unsaved data
{
  if(pJob && pRun && pRun->bDelete==0) {
    Method *pMethod=pJob;
    if(pRun->runrawdata.pRawData32 && pRun->RawDataOffset<0) { // not saved
      int bSave=0;
      //if(pRun->RawDataSize==0) pRun->RawDataSize=100; // debug
      if(pRun->runrawdata.RawDataSize!=0) // if smthng to save
      switch(runMode.SaveRun) {
        case 2: // Save on alarm
        {
          for(int n=0; n<NUMDETECTORS; n++) {
            if(!pPeakList[n]) continue;
            int flags=0;
            for(PeakList::Node *ppln=pPeakList[n]->first(); ppln; ppln=ppln->nextNode()) {
              flags|=pMethod->det[n].Concentration(pMethod, pRun, ppln, n, 0);
            }
            if(flags&(2|4)) { //High or Low alarm occured
              bSave=1;
            }
          }
        }
	break;
        case 1:  // Autosave
          bSave=1;
        break;
        default: // Manual
          bSave=IDYES==MessageBox(HWindow,"Save last acquisition?",GetApplication()->Name,MB_YESNO|MB_ICONQUESTION|MB_TASKMODAL);
      }
      if(bSave) {
        if(pJob->FileName.IsEmpty())
          message("Use Save file As to create file"); // never be! MUST ask about it before run began!!
        else {
          pJob->SaveRawData(pRun);
          //pJob->SaveCalResults(pRun); // is in SaveRawData
          pJob->SaveRunFile(pRun,pPeakList);
        }
      }
      else {
        pRun->bDelete=1;
        pJob->SaveRunFile(pRun,pPeakList); // ??
      }
    }
  }
}


void MainWindow::DeleteRun(void) // Unload from memory
{
  //TRACE1("DeleteRun=%p",pRun);
  DeleteIntegration();
  if(pRun) {
    SaveRun(); // Save unsaved data
    __DeleteRun();
  }
  DeletePeakList();
  Refresh();
  ReflectRun();
  //TRACE("end of DeleteRun");
}
void MainWindow::__DeleteRun(void) // Unload from memory
{
  if(pRun) {
    if(pRun->CalResultsOffset==-1 && !pRun->bDelete)
      __asm int 3 // DEBUG
    else
      pRun->FreeCalResults();
    pRun->FreeRawData();
    #ifdef FILTER
    pRun->FreeFiltData();
    #endif//FILTER
    if(pRun->bDelete) destroy((RunList::Node *)pRun); //!!!!! Detele From RunList!!
    pRun=0;
  }
}


void MainWindow::DeletePeakList(void)
{
  //TRACE("DeletePeakList");
  DeleteIntegration();
  for(int n=0; n<NUMDETECTORS; n++) {
    //TRACE2("DeletePeakList[%d]=%p",n,pPeakList[n]);
    destroy( pPeakList[n] ); pPeakList[n]=0;
  }
  ReflectPeakList();
  Refresh();
}

void MainWindow::ReflectPeakList()
{
  if(pRun) for(int n=0; n<NUMDETECTORS; n++) {
    if(pPeakList[n]) {
      EnableCommand(CM_SAVEINTEGRATION,   TRUE);
      return;
    }
  }
  EnableCommand(CM_SAVEINTEGRATION,   FALSE);
}

void MainWindow::DeleteIntegration()
{
  //TRACE1("DeleteIntegration=%p",pIntegration);
  Integration *p=pIntegration; pIntegration=NULL;
  delete p;
  ReflectIntegration();
  SaveRun(); // Save unsaved data
}

void MainWindow::ReflectIntegration(void)
{
  int bIntegration=pIntegration!=0;
#if !DEMO() // cond added. P.P. 10/09/04
  EnableCommand(CM_CHECKCALIBRATE,   !bIntegration);
  EnableCommand(CM_CALIBRATE,   !bIntegration);
  EnableCommand(CM_ACQUIRE,     !bIntegration);
  EnableCommand(CM_INTEGRATE,   !bIntegration);
#if !GC501() // cond added. P.P. 08/23/05
  EnableCommand(CM_STOPPUMP,   !bIntegration);
  #endif // !gc501
  EnableCommand(CM_STOPINTEGRATION,   bIntegration);
#else // case added. P.P. 10/09/04
  EnableCommand(CM_CHECKCALIBRATE, FALSE);
  EnableCommand(CM_CALIBRATE, FALSE);
  EnableCommand(CM_ACQUIRE, FALSE);
#endif // !demo
#if INTERNAL() // cond added. P.P. 09/25/04
//#if !GC321()  // P.P. 06/13/02 - removed, no alarm in GC321
#if !(GC321()||GC326())  // P.P. 06/13/02 - removed, no alarm in GC321 or 326
#if !(TCD()&&FID()) // tcd and fid together, no lamp present. P.P. 02/18/11
  EnableCommand(CM_STARTLAMP, (!bIntegration)); // added. P.P. 07/22/06
  EnableCommand(CM_STOPLAMP, (!bIntegration)); // added. P.P. 07/22/06
#endif // tcd and fid
#if !GC312() // added. P.P. 03/26/06
  EnableCommand(CM_STOPALARM, !bIntegration);  // added to disable StopAlarm until run stops. P.P. 01/25/02
#endif // gc312
#if FLOW()                                        // added P.P. 12/31/03
//#if 0  // disabled in today's 501 variant. P.P. 06/04/04    restored 12/07/18
  EnableCommand(CM_EDIT_FLOWMODE, !bIntegration); // to setup
//#endif
#endif                                            // flow
#else // for gc321/322/326
  EnableCommand(CM_OPENDOOR, (!bIntegration)); // added. P.P. 07/02/06
  EnableCommand(CM_CLOSEDOOR, (!bIntegration)); // added. P.P. 07/02/06
  EnableCommand(CM_STARTLAMP, (!bIntegration)); // added. P.P. 07/02/06
  EnableCommand(CM_STOPLAMP, (!bIntegration)); // added. P.P. 07/02/06
#if FLOW()                                        // added P.P. 12/07/18
  EnableCommand(CM_EDIT_FLOWMODE, !bIntegration); // to setup flow
#endif // flow                                    
#endif // !gc321
#endif // internal
}

#ifndef  _WCPPLIB_MESSAGE_H
#include "wcpplib\message.h"
#endif
#ifndef  _WINUTIL_THINKING_H
#include "winutil\thinking.h"
#endif
#ifndef  _WCPPLIB_FWSTREAM_H
#include "wcpplib\fwstream.h"
#endif


void MainWindow::CMFileNew(RTMessage)
{
  NewJob();
}

int MainWindow::FileOpen(const char *meth_name)
{
  ifwstream is(meth_name);
  if(!is.good()) {
    message("Cannot open file:\n%s", meth_name);
    return -1;
  }
  else {
    Thinking think("Loading file: '%s'...", meth_name);
    int ret=LoadJob(is);
    switch(ret) {
      case 0: // OK
      {
        ChangePeakName(meth_name);
        const char *DskFile=GetDesktopName();
        if(!DskFile || RestoreDesktop(DskFile)) {
          CloseChildren();
        }
      }
      break;
      case -2:  // Wrong label
        message("Invalid file format:\n%s", meth_name);
        NewJob();
      break;
      case -3:  // Unsupported old version
        message("Unsupported file format version:\n%s", meth_name);
        NewJob();
      break;
      case -4:  // Unsupported new version
        message("File format require newer version of program:\n%s", meth_name);
        NewJob();
      break;
      default:
        message("Error loading file:\n%s", meth_name);
        NewJob();
      break;
    }
    return ret;
  }
}

void MainWindow::CMFileOpen(RTMessage)
{
  Str meth_name;
  if(GetJobFileName(HWindow, &meth_name, FALSE)) {
    FileOpen(meth_name);
  }
}

void MainWindow::CMFileSave(RTMessage Msg)
{
  if(!pJob) return;
  if(pJob->FileName.IsEmpty()) CMFileSaveAs(Msg);
  else if(pRun && pRun->runrawdata.pRawData32 && pRun->RawDataOffset<0) { // not saved
    //DeleteRun(); // Unload it from meory
    pJob->SaveRawData(pRun);
    pJob->SaveRunFile(pRun,pPeakList); //??
//    pJob->SaveRawDataToFile(pRun); // added. P.P. 02/17/12
  }
//  pJob->SaveRawDataToFile(pRun); // added. P.P. 02/17/12
}

#ifndef  _RUNL_DLG_H
#include "runl_dlg.h"
#endif

void MainWindow::CMFileSaveAs(RTMessage)
{
  if(!pJob) return;
  Str meth_name;
  //Force Windows to lock file to exclude self overwriting
  HFILE hfile=pJob && !pJob->FileName.IsEmpty()?
    _lopen(pJob->FileName, OF_READ|OF_SHARE_EXCLUSIVE): 
    HFILE_ERROR;
  BOOL ret=GetJobFileName(HWindow, &meth_name, TRUE);
  if(hfile!=HFILE_ERROR) _lclose(hfile);
  if(ret) {
    ofwstream os(meth_name);
    if(!os.good()) {
      message("Cannot open file:\n%s", meth_name.Get());
    }
    else {
      if(pJob->first()) { // not empty?
        RunListDialogInfo info;
        info.bMul=1;
        info.pJob=pJob;
        int i=GetModule()->ExecDialog(newRunListDialog(this,ID_RUNLISTDIALOG_MULT,&info));
        if(i!=IDOK) return;
        DeleteRun();
        /*if(info.ind.p)*/ {
          int i=0;
          for(RunList::Node *prln=pJob->first(), *prlnNext; prln; prln=prlnNext, i++) {
            prlnNext=prln->nextNode();
            if(!info.ind.contain(i))
              destroy(prln);
          }
        }
      }
      Thinking think("Saving file: '%s'...", meth_name.Get());
      if(!SaveJob(os)) {
        ChangePeakName(meth_name);
        if(Pref.bAutoSaveDesktop) {
          const char *DskFile=GetDesktopName();
          if(DskFile) SaveDesktop(DskFile);
        }
      }
      else {
        message("Error saving file:\n%s", meth_name.Get());
      }
    }
  }
}

#ifndef  _HNU_H
#include "hnu.h"
#endif

int MainWindow::LoadJob(iwstream &is)
{
  uchar file_type;
  is>>file_type;
  is.seek(-(wstreampos)sizeof(file_type),wstream::cur);
  if(file_type==JOB) {
    NewJob();
    if(pJob) {
      pJob->FileVersion=0;
      pJob->readOld(is);
    }
  }
  else if(file_type==METHOD) {
    NewJob();
    if(pJob) {
      pJob->FileVersion=0;
      pJob->Method::readOld(is);
    }
  }
  else {
    unsigned long label;
    unsigned long version;
    is>>label;
    is>>version;
    if(!is.good()) return -1;
    if(label==job_file_label) {
      if(version==5) return -3;
      if(version<job_file_version_oldest) return -3;
      if(version>job_file_version) return -4;
      NewJob();
      if(pJob) {
        pJob->FileVersion=version;
        pJob->read(is,version);
      }
    }
    else {
      return -2;// ERROR - WRONG FORMAT
    }
  }
  if(!is.good() || !pJob ) {
    return -1;
  }
  return 0;
}


int MainWindow::SaveJob(owstream &os)
{
  if(pJob) {
    os<<job_file_label;
    os<<job_file_version;
    // pJob->FileVersion must corresponde to FileName
    pJob->write(os);
    pJob->FileVersion=job_file_version;
    if(!os.good()) {
      return -1;
    }
  }
  return 0;
}

void MainWindow::CMFileDelete(RTMessage)
{
}

#ifndef  _METH_DLG_H
#include "meth_dlg.h"
#endif

void MainWindow::CMMethodEdit(RTMessage)
{
  if(pJob) {
    Method *pMethod=pJob;
    int i=GetModule()->ExecDialog(newMethodDlg(this,ID_METHODDIALOG,pMethod,Pref));
    if(i==IDOK) {
      pMethod->method_needs_transmitting=TRUE;
      Refresh();
    }
  }
}

#ifndef  _CMPN_DLG_H
#include "cmpn_dlg.h"
#endif

void MainWindow::CMMethodComponents(RTMessage)
{
  if(pJob) {
    Method *pMethod=pJob;
    int i=GetModule()->ExecDialog(newComponentsDlg(this,ID_COMPONENTSDIALOG,pMethod));
    if(i==IDOK) {
      pMethod->method_needs_transmitting=TRUE;
      Refresh();
    }
  }
}

#ifndef  _STND_DLG_H
#include "stnd_dlg.h"
#endif

void MainWindow::CMMethodStandards(RTMessage)
{
  if(pJob) {
    Method *pMethod=pJob;
    int i=GetModule()->ExecDialog(newStandardsDlg(this,ID_STANDARDSDIALOG,pMethod,pPeakList));
    if(i==IDOK) {
      pMethod->method_needs_transmitting=TRUE;
      Refresh();
    }
  }
}

#ifndef  _DET_DLG_H
#include "det_dlg.h"
#endif

void MainWindow::CMDetectorAEdit(RTMessage)
{
  if(pJob) {
    Method *pMethod=pJob;
    int i=GetModule()->ExecDialog(newDetectorDlg(this,ID_DETECTORDIALOG,"Detector A",&pMethod->det[0], 0));
    if(i==IDOK) {
      pMethod->method_needs_transmitting=TRUE;
      Refresh();
    }
  }
}

void MainWindow::CMDetectorBEdit(RTMessage)
{
  if(pJob) {
    Method *pMethod=pJob;
    int i=GetModule()->ExecDialog(newDetectorDlg(this,ID_DETECTORDIALOG,"Detector B",&pMethod->det[1], 1));
    if(i==IDOK) {
      pMethod->method_needs_transmitting=TRUE;
      Refresh();
    }
  }
}

void MainWindow::CMMethodSave(RTMessage)
{
  if(pJob) {
    pJob->SaveMethod();
  }
}


void MainWindow::CMStopPump(RTMessage)
{
#if INTERNAL() // separated. P.P. 09/25/04
//  #if (INTERNAL()&&(!GC321()))  // P/P/ 06/13/02
//#if (!GC321())  // P/P/ 06/13/02
#if !(GC321()||GC326())  // P/P/ 12/09/18
  Acquire::SamplePump(FALSE);
#endif
#endif // internal
}
void MainWindow::CMStopAlarm(RTMessage)
{
#if INTERNAL() // separated. P.P. 09/25/04
//  #if (INTERNAL()&&(!GC321()))  // P/P/ 06/13/02
//#if (!GC321())  // P/P/ 06/13/02
#if !(GC321()||GC326())  // P/P/ 12/09/18
  Acquire::Alarm(FALSE);
#if POINTS()>=4 //cond. added also. P.P. 09/05/04
  Acquire::Diagnostics_Alarm(5, FALSE); // added. P.P. 08/08/04  // leftover, need to remove. P.P. 09/0`/`8
  Acquire::Diagnostics_Alarm(6, FALSE); // added. P.P. 09/05/04 // stop cal error alarm added.
#endif // points>=4
#endif // !gc321
#endif // internal // P.P. 09/25/04
}

void MainWindow::CMStartTCD(RTMessage)
{
#if INTERNAL()
  Acquire::TCDDetector(TRUE);
#endif //INTERNAL()
}

void MainWindow::CMStopTCD(RTMessage)
{
#if INTERNAL()
  Acquire::TCDDetector(FALSE);
#endif //INTERNAL()
}

// 2 functions added. P.P. 06/28/05
void MainWindow::CMStartFlame(RTMessage)
{
#if INTERNAL()
  Acquire::FlameDetector(TRUE);
  Acquire::HydrogenValve(TRUE);
#endif //INTERNAL()
}

void MainWindow::CMStopFlame(RTMessage)
{
#if INTERNAL()
  Acquire::HydrogenValve(FALSE);
  Acquire::FlameDetector(FALSE);
#endif //INTERNAL()
}
// end of added functions. P.P. 06/28/05

// functions added to turn lamps ON/OFF. P.P. 06/21/06
void MainWindow::CMStartLamp(RTMessage)
{
#if INTERNAL()
  Acquire::Lamp(TRUE);
  bLampOn = 1;
//  MainWindow *pMainWindow;
//  pMainWindow->sl->SetStatus("Lamp turned ON");  // message added. P.P. 07/02/06
#endif //INTERNAL()
}

void MainWindow::CMStopLamp(RTMessage)
{
#if INTERNAL()
  Acquire::Lamp(FALSE);
  bLampOn = 0;
//  MainWindow *pMainWindow;
//  pMainWindow->sl->Replace("Lamp turned OFF");  // message added. P.P. 07/02/06
#endif //INTERNAL()
}

void MainWindow::CMStartLampB(RTMessage)
{
#if INTERNAL()
  Acquire::LampB(TRUE);
#endif //INTERNAL()
}

void MainWindow::CMStopLampB(RTMessage)
{
#if INTERNAL()
  Acquire::LampB(FALSE);
#endif //INTERNAL()
}

void MainWindow::CMEnableOven(RTMessage)  // Two functions added. P.P. 06/30/06
{
#if INTERNAL()
  Acquire::EnableOven(TRUE);
//  MainWindow *pMainWindow;
//  pMainWindow->sl->Replace("Oven enabled");  // message added. P.P. 07/02/06
#endif //INTERNAL()
}
void MainWindow::CMDisableOven(RTMessage)
{
#if INTERNAL()
  Acquire::EnableOven(FALSE);
//  MainWindow *pMainWindow;
//  pMainWindow->sl->Replace("Oven disabled");  // message added. P.P. 07/02/06
#endif //INTERNAL()
}

void MainWindow::CMOpenDoor(RTMessage)  // Two functions added. P.P. 07/01/06
{
#if INTERNAL()
//#if GC321()
#if GC321()||GC326()  // P/P/ 12/09/18
  Acquire::OvenDoor(TRUE);
  Acquire::DoorMotor(TRUE); // added. P.P. 10/21/11
  bDoorOn = 1;
  bFan = 1; // added to relate fan to oven door. P.P. 12/12/18
  ReflectOvenDoor();
#endif
#endif //INTERNAL()
}
void MainWindow::CMCloseDoor(RTMessage)
{
#if INTERNAL()
//#if GC321()
#if GC321()||GC326()  // P/P/ 12/09/18
  Acquire::OvenDoor(FALSE);
  Acquire::DoorMotor(TRUE); // added. P.P. 10/21/11
  bDoorOn = 0;
  bFan = 0; // added to relate fan to oven door. P.P. 12/12/18
  ReflectOvenDoor();
#endif
#endif //INTERNAL()
}

//#if GC321() // cond added. P.P. 07/07/06
#if GC321()||GC326()  // P/P/ 12/09/18
void MainWindow::ReflectOvenDoor(void)
{
  EnableCommand(CM_OPENDOOR, !bDoorOn);
  EnableCommand(CM_CLOSEDOOR, bDoorOn);
//  MainWindow *pMainWindow;
  if(bDoorOn) {                                   // messages added. P.P. 07/02/06
//    pMainWindow->sl->Replace("Oven door is open" ); // P.P. 08/20/05
  }
  else {
//    pMainWindow->sl->Replace("Oven door is closed" ); // P.P. 08/20/05
  }
}
#endif
// end of added functions. P.P. 06/21/06
// aded to simulate the oven ON/OFF LED. P.P. 05/02/13  here 10/24/14
void MainWindow::ReflectHeat(BOOL bHeat)
{
  EnableCommand(IDD_HEAT_ON, bHeat);
}

void MainWindow::ReflectHeatInj(BOOL bHeatInj) // new function. P.P. 12/12/18
{
  EnableCommand(IDD_HEAT_INJ_ON, bHeatInj);
}

// new indicators added. P.P. 03/14/17
void MainWindow::ReflectSample(BOOL bSample)
{
  EnableCommand(IDD_SAMPLE, bSample);
}

void MainWindow::ReflectEquilibrate(bEquilibrate)
{
  EnableCommand(IDD_EQUILIBRATE, bEquilibrate);
}

void MainWindow::ReflectInject(BOOL bInject)
{
  EnableCommand(IDD_INJECT, bInject);
}

#if CONCENTRATOR()
void MainWindow::ReflectDesorb(BOOL bDesorb)
{
  EnableCommand(IDD_DESORB, bDesorb);
}

void MainWindow::ReflectPurge(BOOL bPurge)
{
  EnableCommand(IDD_PURGE, bPurge);
}

void MainWindow::ReflectInj_V4(BOOL bInj_V4)
{
  EnableCommand(IDD_INJ_V4, bInj_V4);
}
#endif

void MainWindow::ReflectFan(bFan)
{
  EnableCommand(IDD_FAN, bFan);
}

#if HEATED_LINE() // new function added. P.P. 03/28/17
void MainWindow::ReflectLineHeat(BOOL bLineHeat)
{
  EnableCommand(IDD_LINE_HEAT, bLineHeat);
}
#endif

#if FLOW_RATE()
void MainWindow::ReflectFlow(BOOL bFlowLow)
{
  EnableCommand(IDD_FLOW_HIGH, !bFlowLow);
  EnableCommand(IDD_FLOW_LOW, bFlowLow);
}
#endif

// autozero function added. P.P. 08/28/09
void MainWindow::CMAutoZero(RTMessage)
{
#if INTERNAL()
    Acquire::AutoZero(TRUE);
    bAutoZero = 1;
    EnableCommand(CM_AUTOZERO, !bAutoZero);
    EnableCommand(CM_AUTOZ_OF, bAutoZero);
    return;
#endif // internal
}

void MainWindow::CMAutoZ_Of(RTMessage)
{
#if INTERNAL()
//  if(bAutoZero) { // rem - send dig signal anyway. P.P. 12/06/09
    Acquire::AutoZero(FALSE);
//    Acquire::AutoZeroB(FALSE); // added P.P. 07/14/17
    bAutoZero = 0;
    EnableCommand(CM_AUTOZERO, !bAutoZero);
    EnableCommand(CM_AUTOZ_OF, bAutoZero);
    return;
//  }
//  else {
//    Acquire::AutoZero(TRUE);
//    bAutoZero=0;
//    return;
//  }
#endif // internal
}

// two new functions added. P.P. 07/14/17
void MainWindow::CMAutoZ_B(RTMessage)
{
#if INTERNAL()
    Acquire::AutoZeroB(TRUE);  // added. P.P. 07/14/17
    bAutoZeroB = 1;
    EnableCommand(CM_AUTOZ_B, !bAutoZeroB);
    EnableCommand(CM_AUTOZ_B_OFF, bAutoZeroB);
    return;
#endif // internal
}
void MainWindow::CMAutoZ_B_OFF(RTMessage)
{
#if INTERNAL()
    Acquire::AutoZeroB(FALSE); // added P.P. 07/14/17
    bAutoZeroB = 0;
    EnableCommand(CM_AUTOZ_B, !bAutoZeroB);
    EnableCommand(CM_AUTOZ_B_OFF, bAutoZeroB);
    return;
#endif // internal
}

#include "calc.h"
#include "peaklist.h"
#include "runl_dlg.h"

void MainWindow::CMIntegrate(RTMessage )
{
  if(pJob && pRun) {
    Method *pMethod=pJob;
    DeleteIntegration();
    DeletePeakList();
    for(int n=0; n<NUMDETECTORS; n++) {
      if(!pMethod->det[n].detect_stat) continue;
      ASSERT(pPeakList[n]==0);
      pPeakList[n] = new PeakList(this,n);
      //TRACE2("alloc PeakList[%d]=%p",n,pPeakList[n]);
    }
    ReflectPeakList();
    ASSERT(pIntegration==0);
    int nStandard=0;
    if(pRun->type_of_run == CAL) {
      int i=GetModule()->ExecDialog(newStandardListDialog(this,ID_STANDARDLIST, &nStandard));
      if(IDOK!=i) return;
    }
    pIntegration=new RunIntegration(pMethod,pPeakList,pRun,nStandard);
    ReflectIntegration();
  }
}

void MainWindow::CMMethodDownload(RTMessage)
{
  if(pJob) pJob->method_needs_transmitting=TRUE; // really need
  AcquireFunc(METHOD,FALSE);
}

DWORD cal_cycle_start_time=0;

void MainWindow::CMAcquire(RTMessage)
{
  cal_cycle_start_time=GetTime();
  if(pJob) pJob->method_needs_transmitting=TRUE; // ??

  InitTWA(); //??
  AcquireFunc(JOB,FALSE);
}

void MainWindow::CMAcquire1(RTMessage)
{
  // Internal in continuous and Repeat Mode: no method transmitting!
  AcquireFunc(JOB,TRUE);
}

void MainWindow::CMCalibrate(RTMessage)
{
  cal_cycle_start_time=GetTime();
  if(pJob) pJob->method_needs_transmitting=TRUE; // ??
  AcquireFunc(CAL,FALSE);
}

void MainWindow::CMCalibrate1(RTMessage)
{
  cal_cycle_start_time=GetTime();
  AcquireFunc(CAL,TRUE);
}

void MainWindow::CMCheckCalibrate(RTMessage)
{
//??  cal_cycle_start_time=GetTime();
  if(pJob) pJob->method_needs_transmitting=TRUE; // ??
  AcquireFunc(CHECKCAL,FALSE);
}

void MainWindow::CMCheckCalibrate1(RTMessage)
{
//??  cal_cycle_start_time=GetTime();
  // Internal in continuous and Repeat Mode with auto cal: no method transmitting!
  AcquireFunc(CHECKCAL,TRUE);
}

Comm *MainWindow::GetComm()
{
#if _NOCOMM()
  return 0;
#else
  if(!pComm) {
    if(0<=Pref.iCom && Pref.iCom<4) {
      char str[40];
      wsprintf(str,"COM%d",Pref.iCom+1);
      TRACE1("new Comm=%s",str);
      pComm=new Comm(str, 1024, 1024,
#if !INTERNAL()
      GC_ID, GCON_ID  
#else
      WPEAK_ID, WPEAKC_ID
#endif
      );
      wsprintf(str,"COM%d:9600,n,8,1",Pref.iCom+1);
      pComm->SetParam(str);
      pComm->EnableNotification(HWindow, 1, -1);
    }
#if !INTERNAL()
    else
      MessageBox(HWindow,"COM port not set. Use Options|Preferences dialog to choose COM port",GetApplication()->Name,MB_OK|MB_ICONSTOP|MB_TASKMODAL);
#endif
  }
  TRACE1("GetComm=%p",pComm);
  return pComm;
#endif  // _ocomm
}

void MainWindow::ClearComm()
{
#if !_NOCOMM()
  TRACE1("ClearComm=%p",pComm);
  delete pComm; pComm=0;
#endif
}

#ifndef _NDEBUG
#include <time.h>
#include <toolhelp.h>
//#include "\objedit\libs\lib\mymem.cpp"
static class A {
public:
 A();
 HGLOBAL h;
} o;
A::A()
: h(0)
{
  HGLOBAL hg=GlobalAlloc(GHND,100);
  {
    GLOBALENTRY ge;
    ge.dwSize = sizeof(ge);
    for(int i = GlobalFirst(&ge, GLOBAL_ALL); i; i=GlobalNext(&ge, GLOBAL_ALL)) {
      if(hg==ge.hBlock) {
        h=ge.hOwner;
        break;
      }
    }
  }
  GlobalFree(hg);
}

void PrintHeap()
{
  DWORD used=0;
  DWORD used2=0;
  DWORD used3=0;
  {
    GLOBALENTRY ge;
    ge.dwSize = sizeof(ge);
    for(int i = GlobalFirst(&ge, GLOBAL_ALL); i; i=GlobalNext(&ge, GLOBAL_ALL)) {
      if(ge.wType != GT_FREE) {
        used+=ge.dwBlockSize;
        if(ge.hOwner==o.h) {
          used2+=ge.dwBlockSize;
          //if(ge.wType == GT_UNKNOWN)
          //  used3+=ge.dwBlockSize;
          used3+=1;
        }
      }
      //if (ge.wType == GT_UNKNOWN) {
      //}
    }
  }
  int GDIPercentage,UserPercentage;
  {
    SYSHEAPINFO shi;
    shi.dwSize = sizeof(shi);
    int ret = SystemHeapInfo(&shi);
    if (ret) {
      GDIPercentage = shi.wGDIFreePercent;
      UserPercentage = shi.wUserFreePercent;
    } else {
      GDIPercentage = -1;
      UserPercentage = -1;
    }
  }
  TRACE5("GDI=%d%% USER=%d%% used=%ld used2=%ld used3=%ld",GDIPercentage,UserPercentage,used,used2,used3);
  /*
  PrintStatistics(0);
  TRACE4("GDI=%d%% USER=%d%% free=%ld used=%ld",GDIPercentage,UserPercentage,freesize,usedsize);
  */
}
#endif // _NDEBUG

#if OPTICS()
class PointNode : public List::Node
{
 public:
  float x, y;
};
#endif


#include <time.h>
#include <values.h>
void MainWindow::AcquireFunc(int acquire_type, BOOL bAuto)
{
#ifndef _NDEBUG
  {
    time_t t=time(NULL);
    TRACE1("%s",ctime(&t));
  }
  PrintHeap();
#endif
  TRACE1("AcquireFunc(%d)",acquire_type);
  if(pJob) {
    if(pJob->FileName.IsEmpty()) {
      message("Use Save file As to create file");
      return;
    }
    if(pJob->FileVersion!=job_file_version) {
      message("Cannot save in old format file:\n%s", pJob->FileName.Get());
      return;
    }
    Method *pMethod=pJob;
    //DeleteIntegration(); is in DeleteRun();
    //DeletePeakList(); PeakList is USED in DeleteRun()!!
    NewRun();

    Str new_name;
    if(1 /*bAuto*//* && bSomethingMore*/) { //??
      struct tm *t=localtime(&pJob->start_time);
      int tm_mday=t->tm_mday;
      int tm_mon=t->tm_mon;
      int tm_year=t->tm_year;
      time_t cur_time=time(NULL);
      t=localtime(&cur_time);
      if(tm_mday!=t->tm_mday || tm_mon!=t->tm_mon || tm_year!=t->tm_year) {
        new_name=pJob->FileName;
        const char *cp=strrchr(new_name,'\\');
        if(cp) {
          *((char *)(cp+1))=0; // cut name
          // DDMMYY -  for better file name sorting
          char buff[15];
          // The file name in format mm/dd/yy or dd/mm/yy - dependd on country settings
          int dateformat=0; // 0 - MMDDYY(USA); 1 - DDMMYY(Europe); 2 - YYMMDD(Japan)
          {
            char fmt[20];
            GetProfileString("Intl","sShortDate","m/d/yy",fmt,sizeof(fmt));

            if(fmt[0]=='d'||fmt[0]=='D') dateformat=1;
            else if(fmt[0]=='y'||fmt[0]=='Y') dateformat=2;
          }

          switch(dateformat) {
            case 1: // DDMMYY
              wsprintf(buff,"%02d%02d%02d.job",t->tm_mday,1+t->tm_mon,t->tm_year%100);
            break;
            case 2: // YYMMDD
              wsprintf(buff,"%02d%02d%02d.job",t->tm_year%100,1+t->tm_mon,t->tm_mday);
            break;
            default: // MMDDYY
              wsprintf(buff,"%02d%02d%02d.job",1+t->tm_mon,t->tm_mday,t->tm_year%100);
          }

          new_name+=buff; // add new
        }
      }
    }
    if(!new_name.IsEmpty()) {
      // last run can contain valid calibration!
      time(&pJob->start_time);

      RunList::Node *pLastNode =(RunList::Node *)pRun;
      pJob->remove(pLastNode); // clear all runs but last
      pRun=0;

      for(RunList::Node *prln=pJob->first(), *prlnNext; prln; prln=prlnNext) {
        prlnNext=prln->nextNode();
        destroy(prln);
      }
      struct find_t ffblk;
      unsigned err=_dos_findfirst(new_name,FA_RDONLY|FA_HIDDEN|FA_SYSTEM|FA_LABEL|FA_DIREC|FA_ARCH, &ffblk);
      if(!err) { // file already exists!
        if(0==FileOpen(new_name)) {
           // all is OK!
        }
        else {
          //??
        }
      }
      else {
        ofwstream os(new_name);
        if(!os.good()) {
          message("Cannot open file:\n%s", new_name.Get());
          return;
        }
        else {
          Thinking think("Saving file: '%s'...", new_name.Get());
          if(!SaveJob(os)) {
            ChangePeakName(new_name);
            if(Pref.bAutoSaveDesktop) {
              const char *DskFile=GetDesktopName();
              if(DskFile) SaveDesktop(DskFile);
            }
          }
        }
      }
      DeleteRun();

      pJob->append(pLastNode);
      pRun=pLastNode;
      pJob->GetCalResults(pRun);
      // RelectRun later
    }


    for(int n=0; n<NUMDETECTORS; n++) {
      if(!pMethod->det[n].detect_stat) continue;
      ASSERT(pPeakList[n]==0);
      pPeakList[n] = new PeakList(this,n);
      //TRACE2("alloc PeakList[%d]=%p",n,pPeakList[n]);
    }
    ReflectPeakList();
    Comm *pComm=GetComm();
#if !INTERNAL() // #if !_NOCOMM()
    if(!pComm) return;
#endif
    int nStandard=0;
    if(acquire_type == CAL && !bAuto) {
      int i=GetModule()->ExecDialog(newStandardListDialog(this,ID_STANDARDLIST, &nStandard));
      if(IDOK!=i) return;
    }

    if(!bAuto) cur_point=0;

#if OPTICS()
    HINSTANCE hInstance=GetApplication()->hInstance;
    //Str strFilter(128);
    //LoadFilterString(hInstance, IDS_RTF_FILTER, strFilter.GetPtr(), 128);
    //char strFilter[]="samples\0*.sample\0";
    static char far strFilter[]="Sample files(*.sam)\0*.sam\0"
                     "All files(*.*)\0*.*\0";
    //Str strTitle(64);
    //LoadString(hInstance, IDS_SAVEREPORT, strTitle.GetPtr(), 64);
    static char far strTitle[]="Select Sample file to load";
    Str FileName;
    BOOL ret=OpenSaveDialog(HWindow, strTitle, strFilter, &FileName, FALSE);
    if(ret) {
      FILE *f=fopen(FileName,"rb");
      if(f) {
        char buff[100];
        char *s;
        for(;;) {
          s=fgets(buff,100,f);
          if(!s) break;
          if(buff[0]=='>') break;
          pRun->comment+=buff;
        }
        if(s) {
          List list;
          int n;
          do {
            float x, y;
            n=fscanf(f,"%g %g",&x,&y);
            if(n==2) {
              PointNode *p=new PointNode;
              p->x=x;
              p->y=y;
              list.append(p);
            }
          } while(n==2);
          {
            /*float ymax=-MAXFLOAT;
            float ymin=-MINFLOAT;
            {
              for(PointNode *p=(PointNode *)list.first(); p; p=(PointNode *)p->nextNode()) {
                if(p->y>ymax) ymax=p->y;
                if(p->y<ymin) ymin=p->y;
              }
            }*/
            {
              double ay=10000;//ymax-ymin? 30000/(ymax-ymin): 100;
              for(PointNode *p=(PointNode *)list.first(); p; p=(PointNode *)p->nextNode()) {
                p->y=ay*p->y;
                if(p->y<0) p->y=0;
              }
            }
          }
          {
            long n=list.countChildren();


            pRun->FreeRawData();
            pRun->AllocRawData32(n * NUMDETECTORS); // ZLP

            pRun->runrawdata.nRawDataFormat=2;
            pRun->runrawdata.RawDataSize=0;
            pRun->runrawdata.gain[0]=pRun->runrawdata.gain[1]=0;

            extern double _xmin_;
            extern double _dx_;
            double _xmax_=0;
            _xmin_=MAXDOUBLE;

            long l=0;
            for(PointNode *p=(PointNode *)list.first(); p; p=(PointNode *)p->nextNode()) {
              if((l%32)==0) pRun->runrawdata.pRawData32[pRun->runrawdata.RawDataSize++]=0; // Zero bits for Detector A
              pRun->runrawdata.pRawData32[pRun->runrawdata.RawDataSize++]=p->y;
              l++;
              if(_xmax_<p->x) _xmax_=p->x;
              if(_xmin_>p->x) _xmin_=p->x;
            }

            if(l) _dx_=(_xmax_-_xmin_)/l;
            data_rate=1;
          }
        }
        fclose(f);
      }
    }
    ReflectRun(); // pRun->type_of_run
    ReflectIntegration();
    Refresh();
    PostMessage(HWindow, WM_COMMAND,CM_INTEGRATE,0L);
    return;
#endif // optics


    Acquire *pAcquire=new Acquire(pComm, acquire_type, 0, nStandard, pMethod, pPeakList, pRun, pJob, cur_point, this);
    pAcquire->Start();
    ASSERT(pIntegration==0);
    pIntegration=pAcquire;
    ReflectRun(); // pRun->type_of_run
    ReflectIntegration();
  }
  //TRACE1("end of AcquireFunc(%d)",type);
}

void MainWindow::CMStopIntegration(RTMessage)
{
  DeleteIntegration();
}

#ifndef  _WCPPLIB_FWSTREAM_H
#include "wcpplib\fwstream.h"
#endif
static void PeaksWrite(owstream &os, PeakList *pPeakList)
{
  char buff[15];
  if(pPeakList) {
    short n=pPeakList->countChildren(); // Number of Peaks
    wsprintf(buff,"%d\r\n",n); os.write(buff,strlen(buff));
    for(PeakList::Node *pNode=pPeakList->first(); pNode; pNode=pNode->nextNode()) {
      Peak *pPeak=(Peak *)pNode;
      long t=pPeak->Time/data_rate; // Sec
      wsprintf(buff,"%ld ",t); os.write(buff,strlen(buff));
//      wsprintf(buff,"%ld ",pPeak->Area); os.write(buff,strlen(buff)); // in ???*sec (?)
      sprintf(buff,"%.0f ",(float) pPeak->Area); os.write(buff,strlen(buff)); // area as a float, nd using printf. P.P. 08/27/17
      wsprintf(buff,"%ld\r\n",pPeak->Height); os.write(buff,strlen(buff)); // in ???
    }
  }
}

static void PeaksWrite(const char *filename, Run *pRun,PeakList **pPeakList)
{
  char buff[15];
  ofwstream os(filename);
  wsprintf(buff,"%d\r\n",pRun->type_of_run); os.write(buff,strlen(buff)); // 2 -> Job, 1 -> Cal
  int16 numdet=NUMDETECTORS; // always = 2
  wsprintf(buff,"%d\r\n",numdet); os.write(buff,strlen(buff));
  for(int n=0; n<NUMDETECTORS; n++) {
    int16 flag=pPeakList[n]? 1: 0;
    wsprintf(buff,"%d\r\n",flag); os.write(buff,strlen(buff));
    if(pPeakList[n]) {
      PeaksWrite(os,pPeakList[n]);
    }
  }
}

void MainWindow::CMSaveIntegration(RTMessage)
{
  if(!pRun) return;
  HINSTANCE hInstance=GetApplication()->hInstance;
  Str strFilter(128);
  LoadFilterString(hInstance, IDS_PEAKS_FILTER, strFilter.GetPtr(), 128);
  Str strTitle(64);
  LoadString(hInstance, IDS_SAVE_PEAKS_AS, strTitle.GetPtr(), 64);
  Str FileName;
  if(OpenSaveDialog(HWindow, strTitle, strFilter, &FileName, TRUE)) {
    PeaksWrite(FileName,pRun,pPeakList);
  }
}


static void Update(Pvoid Child, Pvoid)
{
  UpdateWindow(((PTWindowsObject)Child)->HWindow);
}

void MainWindow::ProcessIntegration()
{
  if(pIntegration) {
    if(pIntegration->Continue()) {
      DeleteIntegration();
      Refresh();
      { // ??? ZLP
        { // for continuous modes if CM_ACQUIRE1 posted
          ForEach(Update,(void *)0); // update all pictures
          // and wait a second
          for(DWORD dw=GetTickCount(), dwEnd=GetTickCount()+2000; dw<dwEnd; dw=GetTickCount()) {
          }
        }
      }
    }
  }
}

void MainWindow::WMCommNotify(RTMessage )
{
  ProcessIntegration();
}


void MainWindow::WMTimer(RTMessage )
{
  if(pIntegration) {
    ProcessIntegration();
  }
#if INTERNAL()
  else if(wtimer) {
    EvTimer(this);
  }
#endif

#if MODBUS()
  if(modBus && *modBus!=-1) {
    modBus->OnTimer();
  }
#endif
}

void MainWindow::WMIdle(RTMessage)
{
  if(pIntegration) {
    MSG msg;
    do {
      ProcessIntegration();
    } while(!PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE) && pIntegration);
  }
  else {
    WaitMessage();
  }
}

void MainWindow::WMGetJob(RTMessage Msg)
{
  Msg.Result=(LPARAM)pJob;
}

void MainWindow::WMGetRun(RTMessage Msg)
{
  Msg.Result=(LPARAM)pRun;
}

void MainWindow::WMGetPeakList(RTMessage Msg)
{
  Msg.Result=(LPARAM)pPeakList;
}

void MainWindow::WMGetPictInfo(RTMessage Msg)
{
  Msg.Result=(LPARAM)pPictInfo;
}

void MainWindow::WMGetRunPictInfo(RTMessage Msg)
{
  Msg.Result=(LPARAM)pRunPictInfo;
}

OutputWindow *MainWindow::GetOutputWindow()
{
#if 1//def _NDEBUG
  return 0;
#else
  PTWindowsObject pOutputWindow=FindOutputWindow();
  if(!pOutputWindow) {
    MakeOutputWindow();
    pOutputWindow=FindOutputWindow();
  }
  return (OutputWindow *)pOutputWindow;
#endif
}

void MainWindow::WMGetOutput(RTMessage Msg)
{
  Msg.Result=(LPARAM)GetOutputWindow();
}

void MainWindow::print(UINT fmtID, ... )
{
  OutputWindow *pOutputWindow=GetOutputWindow();
  if(pOutputWindow) pOutputWindow->vprint(fmtID,...);
}
void MainWindow::print(LPCSTR fmt, ... )
{
  OutputWindow *pOutputWindow=GetOutputWindow();
  if(pOutputWindow) pOutputWindow->vprint(fmt,...);
}

static void CommandRefresh(Pvoid Child, Pvoid ParamList)
{
  SendMessage(((PTWindowsObject)Child)->HWindow,WM_REFRESH,LOWORD(ParamList),0L);
}
void MainWindow::Refresh(int bGraphOnly)
{
  ForEach(CommandRefresh,(void *)bGraphOnly);
}

void MainWindow::CMWindowSummaryA(RTMessage)
{
  PTWindowsObject p=FindSummaryWindow(0);
  if(!p) MakeSummaryWindow(0);
  else if(!IsIconic(p->HWindow))
    PostMessage(ClientWnd->HWindow, WM_MDIACTIVATE, (WPARAM)p->HWindow, 0L);
    // DeleteSummaryWindow(0);
  else ShowWindow(p->HWindow, SW_RESTORE);
}

void MainWindow::CMWindowSummaryB(RTMessage)
{
  PTWindowsObject p=FindSummaryWindow(1);
  if(!p) MakeSummaryWindow(1);
  else if(!IsIconic(p->HWindow))
    PostMessage(ClientWnd->HWindow, WM_MDIACTIVATE, (WPARAM)p->HWindow, 0L);
    // DeleteSummaryWindow(1);
  else ShowWindow(p->HWindow, SW_RESTORE);
}

void MainWindow::CMWindowPeakA(RTMessage)
{
  PTWindowsObject p=FindPeakWindow(0);
  if(!p) MakePeakWindow(0);
  else if(!IsIconic(p->HWindow))
    PostMessage(ClientWnd->HWindow, WM_MDIACTIVATE, (WPARAM)p->HWindow, 0L);
    // DeletePeakWindow(0);
  else ShowWindow(p->HWindow, SW_RESTORE);
}
void MainWindow::CMWindowPeakB(RTMessage)
{
  PTWindowsObject p=FindPeakWindow(1);
  if(!p) MakePeakWindow(1);
  else if(!IsIconic(p->HWindow))
    PostMessage(ClientWnd->HWindow, WM_MDIACTIVATE, (WPARAM)p->HWindow, 0L);
    // DeletePeakWindow(1);
  else ShowWindow(p->HWindow, SW_RESTORE);
}

void MainWindow::CMWindowGraphAll(RTMessage)
{
  PTWindowsObject p=FindGraphWindow(NUMDETECTORS);
  if(!p) MakeGraphWindow(NUMDETECTORS);
  else if(!IsIconic(p->HWindow))
    PostMessage(ClientWnd->HWindow, WM_MDIACTIVATE, (WPARAM)p->HWindow, 0L);
    // DeleteGraphWindow(NUMDETECTORS);
  else ShowWindow(p->HWindow, SW_RESTORE);
}

void MainWindow::CMWindowGraphA(RTMessage)
{
  PTWindowsObject p=FindGraphWindow(0);
  if(!p) MakeGraphWindow(0);
  else if(!IsIconic(p->HWindow))
    PostMessage(ClientWnd->HWindow, WM_MDIACTIVATE, (WPARAM)p->HWindow, 0L);
    // DeleteGraphWindow(0);
  else ShowWindow(p->HWindow, SW_RESTORE);
}

void MainWindow::CMWindowGraphB(RTMessage)
{
  PTWindowsObject p=FindGraphWindow(1);
  if(!p) MakeGraphWindow(1);
  else if(!IsIconic(p->HWindow))
    PostMessage(ClientWnd->HWindow, WM_MDIACTIVATE, (WPARAM)p->HWindow, 0L);
    // DeleteGraphWindow(1);
  else ShowWindow(p->HWindow, SW_RESTORE);
}

void MainWindow::CMWindowRunGraph(RTMessage)
{
  PTWindowsObject p=FindRunGraphWindow();
  if(!p) MakeRunGraphWindow();
  else if(!IsIconic(p->HWindow))
    PostMessage(ClientWnd->HWindow, WM_MDIACTIVATE, (WPARAM)p->HWindow, 0L);
    // DeleteGraphWindow(1);
  else ShowWindow(p->HWindow, SW_RESTORE);
}

void MainWindow::CMWindowOutput(RTMessage)
{
  PTWindowsObject p=FindOutputWindow();
  if(!p) MakeOutputWindow();
  else if(!IsIconic(p->HWindow))
    PostMessage(ClientWnd->HWindow, WM_MDIACTIVATE, (WPARAM)p->HWindow, 0L);
    // DeleteOutputWindow();
  else ShowWindow(p->HWindow, SW_RESTORE);
}


#ifndef  _SUMM_WND_H
#include "summ_wnd.h"
#endif

void MainWindow::MakeSummaryWindow(int i)
{
  GetModule()->MakeWindow(newSummaryWindow(this, i, i? "Summary B": "Summary A"));
}

void MainWindow::DeleteSummaryWindow(int i)
{
  PTWindowsObject p=FindSummaryWindow(i);
  if(p) p->ShutDownWindow();
}

#ifndef  _PEAK_WND_H
#include "peak_wnd.h"
#endif

void MainWindow::MakePeakWindow(int i)
{
  GetModule()->MakeWindow(newPeakWindow(this, i, i? "Table B": "Table A"));
}

void MainWindow::DeletePeakWindow(int i)
{
  PTWindowsObject p=FindPeakWindow(i);
  if(p) p->ShutDownWindow();
}

#ifndef  _GRPH_WND_H
#include "grph_wnd.h"
#endif

void MainWindow::MakeGraphWindow(int i)
{
  GetModule()->MakeWindow(newGraphWindow(this, i, i==NUMDETECTORS? "Both Graph": i? "Graph B": "Graph A"));
}

void MainWindow::DeleteGraphWindow(int i)
{
  PTWindowsObject p=FindGraphWindow(i);
  if(p) p->ShutDownWindow();
}

void MainWindow::MakeRunGraphWindow()
{
  GetModule()->MakeWindow(newRunGraphWindow(this, "Run Graph"));
}

void MainWindow::DeleteRunGraphWindow()
{
  PTWindowsObject p=FindRunGraphWindow();
  if(p) p->ShutDownWindow();
}

#ifndef  _OUTP_WND_H
#include "outp_wnd.h"
#endif

void MainWindow::MakeOutputWindow(void)
{
  PTWindowsObject p=
  GetModule()->MakeWindow(newOutputWindow(this));
  //if(p) ShowWindow(p->HWindow,SW_MINIMIZE);
}

void MainWindow::DeleteOutputWindow(void)
{
  PTWindowsObject p=FindOutputWindow();
  if(p) p->ShutDownWindow();
}

void MainWindow::CMSelectRun(RTMessage)
{
  if(pJob) {
    RunListDialogInfo info;
    info.bMul=0;
    info.pJob=pJob;
    int i=GetModule()->ExecDialog(newRunListDialog(this,ID_RUNLISTDIALOG,&info));
    if(i==IDOK && info.ind.p) {
      NewRun(*info.ind.p);
      Refresh();
    }
  }
}

void MainWindow::CMSaveCalRes(RTMessage)
{
  if(pJob && pRun) {
    if(pJob->FileName.IsEmpty()) {
      message("Use Save file As to create file");
      return;
    }
    if(pJob->FileVersion!=job_file_version) {
      message("Cannot save in old format file:\n%s", pJob->FileName.Get());
      return;
    }
    RunListDialogInfo info;
    info.bMul=1;
    info.pJob=pJob;
    int i=GetModule()->ExecDialog(newRunListDialog(this,ID_RUNLISTDIALOG_MULT,&info));
    if(i==IDOK && info.ind.p) {
      CalResults *pCalResults=pJob->GetCalResults(pRun);
      for(int i=0; i<info.ind.n; i++) {
        RunList::Node *prln=pJob->GetChild(info.ind.p[i]);
        if(prln) {
          if(!prln->pCalResults) //??
            prln->AllocCalResults();
          memcpy(prln->pCalResults->cal_results,pCalResults->cal_results,sizeof(prln->pCalResults->cal_results));
          pJob->SaveCalResults(prln);
        }
      }
      //pJob->SaveRunList(); //??
      Refresh();
    }
  }
}


#ifndef  _TD_PLUS_H
#include "td_plus.h"
#endif

class RunDialog : public TransferPlusDlg {
  typedef TransferPlusDlg Inherited;
 public:
   static ControlInfo far ci[];
   RunDialog(PTWindowsObject AParent, int ResourceId, Run *data, PTModule AModule = NULL)
    : TransferPlusDlg(AParent, ResourceId, data, ci, AModule) {}
   virtual void SetupControls(void *data);
};

ControlInfo far RunDialog::ci[]={
// {IDD_RUN_DATE
  { citStr, IDD_RUN_NOTES,      FIELDOFFSET(Run,comment),       },
  { citNULL }   // End of array marker
};


#ifndef  _COUNTRY_H
#include "country.h"
#endif

void RunDialog::SetupControls(void *data)
{
  Inherited::SetupControls(data);
  Run *pRun=(Run *)data;
  char str[80];
  wsprintf(str,"%s  %s",PrintDate(pRun->start_time),PrintTime(pRun->start_time));
  SetDlgItemText(HWindow,IDD_RUN_DATE,str);
}


void MainWindow::CMEditRun(RTMessage)
{
  if(pRun) {
    if(IDOK==GetModule()->ExecDialog( new RunDialog(this,ID_RUNDIALOG, pRun) )) {
      //::InvalidateRect(HWindow,NULL,FALSE);
    }
  }
}

RunMode runMode;
ShiftMode shiftMode;

void WriteRunMode(ApplicationProfile &profile)
{
   profile.WriteInt("RunMode","Acquire",runMode.Acquire);
   profile.WriteInt("RunMode","SaveRun",runMode.SaveRun);

   profile.WriteInt("ShiftMode","Acquire",shiftMode.Shift);
   profile.WriteInt("ShiftMode","SaveReport",shiftMode.SaveReport);
   profile.WriteInt("ShiftMode","PrintReport",shiftMode.PrintReport);
   profile.WriteInt("ShiftMode","bPrintAlarm",shiftMode.bPrintAlarm);
}
void ReadRunMode(ApplicationProfile &profile)
{
   runMode.Acquire=profile.GetInt("RunMode","Acquire",runMode.Acquire);
   runMode.SaveRun=profile.GetInt("RunMode","SaveRun",runMode.SaveRun);

   shiftMode.Shift=profile.GetInt("ShiftMode","Acquire",shiftMode.Shift);
   shiftMode.SaveReport=profile.GetInt("ShiftMode","SaveReport",shiftMode.SaveReport);
   shiftMode.PrintReport=profile.GetInt("ShiftMode","PrintReport",shiftMode.PrintReport);
   shiftMode.bPrintAlarm=profile.GetInt("ShiftMode","bPrintAlarm",shiftMode.bPrintAlarm);
}

class RunModeDialog : public TransferPlusDlg {
  typedef TransferPlusDlg Inherited;
 public:
  static ControlInfo far ci[];
  RunModeDialog(PTWindowsObject AParent, int ResourceId, RunMode *data, PTModule AModule = NULL)
   : TransferPlusDlg(AParent, ResourceId, data, ci, AModule) {}
  virtual void SetupControls(void *data);
 protected:
  virtual void HandleAcquireMode (RTMessage Msg)=[ID_FIRST+IDD_ACQUIRE_MODE  ];
  virtual void HandleAcquireMode1(RTMessage Msg)=[ID_FIRST+IDD_ACQUIRE_MODE_1];
  virtual void HandleAcquireMode2(RTMessage Msg)=[ID_FIRST+IDD_ACQUIRE_MODE_2];
  virtual void HandleAcquireMode3(RTMessage Msg)=[ID_FIRST+IDD_ACQUIRE_MODE_3];

  void ReflectAcquireMode();
};

ControlInfo far RunModeDialog::ci[]={
  { citRadio,   IDD_ACQUIRE_MODE,  FIELDOFFSET(RunMode,Acquire),4       },
  { citRadio,   IDD_SAVERUN_MODE,  FIELDOFFSET(RunMode,SaveRun),3       },
  { citONOFF,   IDD_TWA_MODE,      FIELDOFFSET(RunMode,bTWA),           },
  { citONOFF,   IDD_STEL_MODE,     FIELDOFFSET(RunMode,bSTEL),          },
  { citNULL }   // End of array marker
};

void RunModeDialog::SetupControls(void *data)
{
  Inherited::SetupControls(data);
  CheckRadioButton(HWindow, IDD_ACQUIRE_MODE, IDD_ACQUIRE_MODE_3, IDD_ACQUIRE_MODE+runMode.Acquire); // added. P.P. 09/05/04
  ReflectAcquireMode();
}

void RunModeDialog::ReflectAcquireMode()
{
  BOOL bManual=IsDlgButtonChecked(HWindow,IDD_ACQUIRE_MODE)||IsDlgButtonChecked(HWindow,IDD_ACQUIRE_MODE_3);
  if(!bManual && !IsDlgButtonChecked(HWindow,IDD_SAVERUN_MODE_2))
    CheckRadioButton(HWindow, IDD_SAVERUN_MODE, IDD_SAVERUN_MODE_2, IDD_SAVERUN_MODE_1);
  EnableWindow(GetDlgItem(HWindow, IDD_SAVERUN_MODE),bManual);

//  EnableWindow(GetDlgItem(HWindow, IDD_ACQUIRE_MODE_3),FALSE); // rem. P.P. 06/04/04
}

void RunModeDialog::HandleAcquireMode (RTMessage /*Msg*/)
{
  ReflectAcquireMode();
}

void RunModeDialog::HandleAcquireMode1(RTMessage /*Msg*/)
{
  ReflectAcquireMode();
}

void RunModeDialog::HandleAcquireMode2(RTMessage /*Msg*/)
{
  ReflectAcquireMode();
}

void RunModeDialog::HandleAcquireMode3(RTMessage /*Msg*/)
{
  ReflectAcquireMode();
}

void MainWindow::CMEditRunMode(RTMessage)
{
  {
    if(IDOK==GetModule()->ExecDialog( new RunModeDialog(this,ID_RUNMODEDIALOG, &runMode) )) {
      //::InvalidateRect(HWindow,NULL,FALSE);
      WriteRunMode(Pref);
      ReflectRun();
    }
  }
}

class ShiftModeDialog : public TransferPlusDlg {
  typedef TransferPlusDlg Inherited;
 public:
  static ControlInfo far ci[];
  ShiftModeDialog(PTWindowsObject AParent, int ResourceId, ShiftMode *data, PTModule AModule = NULL)
   : TransferPlusDlg(AParent, ResourceId, data, ci, AModule) {}
  virtual void SetupControls(void *data);
 protected:
};

ControlInfo far ShiftModeDialog::ci[]={
//  { citINT,   IDD_SHIFTTIME,  FIELDOFFSET(ShiftMode,Shift),0,24       },
  { citRadio,   IDD_SAVESHIFT_MODE,  FIELDOFFSET(ShiftMode,SaveReport),3        },
  { citRadio,   IDD_PRINTSHIFT_MODE,      FIELDOFFSET(ShiftMode,PrintReport),3          },
  { citONOFF,   IDD_PRINTSHIFTONALARM,     FIELDOFFSET(ShiftMode,bPrintAlarm),          },
  { citNULL }   // End of array marker
};

void ShiftModeDialog::SetupControls(void *data)
{
  Inherited::SetupControls(data);
}

void MainWindow::CMEditShiftMode(RTMessage)
{
  {
    if(IDOK==GetModule()->ExecDialog( new ShiftModeDialog(this,ID_SHIFTMODEDIALOG, &shiftMode) )) {
      //::InvalidateRect(HWindow,NULL,FALSE);
      WriteRunMode(Pref);
    }
  }
}

/// P.P. 12/30/03. Copied from Run Mode part of this file and modified
/// to get a new feature - Flow Setup: dialog with the  flow reading
/// and swithing valves to measure flows in different locations. P.P. 12/30/03

#if FLOW() // cond added. P.P. 07/05/04
extern double Get_Flow();
#endif

class FlowModeDialog : public TransferPlusDlg {
  typedef TransferPlusDlg Inherited;
 public:
  static ControlInfo far FlowInfo[];
  FlowModeDialog(PTWindowsObject AParent, int ResourceId, MethodTable *data, PTModule AModule = NULL)
   : TransferPlusDlg(AParent, ResourceId, data, FlowInfo, AModule) {}
  virtual void SetupControls(void *data);
  virtual void AcceptControls(void *data); // added. P.P. 01/05/04
 protected:
  virtual void WMDestroy(RTMessage Msg)=[WM_FIRST+WM_DESTROY];
  virtual void EvTimer(RTMessage Msg)=[WM_FIRST+WM_TIMER];

  virtual void HandleFlowMode (RTMessage Msg)=[ID_FIRST+IDD_FLOW_MODE  ];
  virtual void HandleFlowMode1(RTMessage Msg)=[ID_FIRST+IDD_FLOW_MODE_1];
  virtual void HandleFlowMode2(RTMessage Msg)=[ID_FIRST+IDD_FLOW_MODE_2];  // return back to 4 flow modes. P.P. 01/09/04
  virtual void HandleFlowMode3(RTMessage Msg)=[ID_FIRST+IDD_FLOW_MODE_3];
//  virtual void HandleFlowInjMode(RTMessage Msg)=[ID_FIRST+IDD_FLOWINJ_MODE];
//  virtual void HandleFlowInjMode1(RTMessage Msg)=[ID_FIRST+IDD_FLOWINJ_MODE_1];
  virtual void HandleLockSetpoint(RTMessage Msg)=[ID_FIRST+IDD_LOCK_SETPOINT];

  int GetFlowMode();

  void ReflectFlowMode();
// rem. P.P. 01/09/04  void ReflectFlowInjMode();
  void ReflectLockSetpoint();
};

ControlInfo far FlowModeDialog::FlowInfo[]={
//  { citRadio,   IDD_FLOW_MODE,  FIELDOFFSET(FlowMode,flow_mode),4     },
//  { citDouble,        IDD_FLOW_VALUE, FIELDOFFSET(FlowMode,flow_value), 0, 100},
  { citDouble,  IDD_SETPOINT, FIELDOFFSET(MethodTable, set_point), 0, 100},
  { citDouble,  IDD_SETPOINT_1, FIELDOFFSET(MethodTable, set_point_1), 0, 100},
  { citDouble,  IDD_SETPOINT_2, FIELDOFFSET(MethodTable, set_point_2), 0, 100},
  { citDouble,  IDD_SETPOINT_3, FIELDOFFSET(MethodTable, set_point_3), 0, 100},
//  { citONOFF,   IDD_LOCK_SETPOINT, FIELDOFFSET(FlowMode, lock_setpoint)},
  { citNULL }   // End of array marker
};

void FlowModeDialog::SetupControls(void *data)
{
  Inherited::SetupControls(data);
  CheckRadioButton(HWindow, IDD_FLOW_MODE, IDD_FLOW_MODE_3, IDD_FLOW_MODE);
  CheckDlgButton(HWindow, IDD_LOCK_SETPOINT, 1);

  ReflectLockSetpoint();
  ReflectFlowMode();
// ReflectFlowInjMode();

  SetTimer(HWindow,1234,500,NULL); // 500 msec
}

void FlowModeDialog::WMDestroy(RTMessage Msg)
{
  KillTimer(HWindow,1234);
  Inherited::WMDestroy(Msg);
}

// added. P.P. 01/05/04
void FlowModeDialog::AcceptControls(void *data)
{
  Inherited::AcceptControls(data);
  ReflectFlowMode();
}

#include "winutil\dlg_item.h"

void FlowModeDialog::EvTimer(RTMessage Msg)
{
#if FLOW() // cond added. P.P. 07/05/04
  double flow=Get_Flow();
  SetDlgItemDouble(HWindow, IDD_FLOW_VALUE, flow, "%f");
#endif
}

int FlowModeDialog::GetFlowMode()
{
  if(IsDlgButtonChecked(HWindow,IDD_FLOW_MODE  )) return 0;
  if(IsDlgButtonChecked(HWindow,IDD_FLOW_MODE_1)) return 1;
  if(IsDlgButtonChecked(HWindow,IDD_FLOW_MODE_2)) return 2;
  if(IsDlgButtonChecked(HWindow,IDD_FLOW_MODE_3)) return 3;
  return 0;
}

void FlowModeDialog::ReflectFlowMode()
{
#if INTERNAL()
  int flow_mode=GetFlowMode();
  Acquire::SetFlowMode(flow_mode);
#endif //INTERNAL()
}

void FlowModeDialog::ReflectLockSetpoint()
{
  BOOL bLock = IsDlgButtonChecked(HWindow, IDD_LOCK_SETPOINT);
  EnableWindow(GetDlgItem(HWindow, IDD_SETPOINT), !bLock);
  EnableWindow(GetDlgItem(HWindow, IDD_SETPOINT_1), !bLock);
  EnableWindow(GetDlgItem(HWindow, IDD_SETPOINT_2), !bLock);
  EnableWindow(GetDlgItem(HWindow, IDD_SETPOINT_3), !bLock);
}

void FlowModeDialog::HandleFlowMode (RTMessage /*Msg*/)
{
  ReflectFlowMode();
}

void FlowModeDialog::HandleFlowMode1(RTMessage /*Msg*/)
{
  ReflectFlowMode();
}

void FlowModeDialog::HandleFlowMode2(RTMessage /*Msg*/)
{
  ReflectFlowMode();
}

void FlowModeDialog::HandleFlowMode3(RTMessage /*Msg*/)
{
  ReflectFlowMode();
}

void FlowModeDialog::HandleLockSetpoint(RTMessage)
{
  ReflectLockSetpoint();
}

void MainWindow::CMEditFlowMode(RTMessage)
{
  if(pJob) {
    Method *pMethod=pJob;
    if(IDOK==GetModule()->ExecDialog( new FlowModeDialog(this,ID_FLOWMODEDIALOG, &pMethod->method_table) )) {
      //::InvalidateRect(HWindow,NULL,FALSE);
#if FLOW()
      //if(!IsDlgButtonChecked(HWindow,IDD_FLOW_MODE)) {
      //        CheckRadioButton(HWindow, IDD_FLOW_MODE, IDD_FLOW_MODE_3, IDD_FLOW_MODE);
      //}
      Acquire::Inject(FALSE); // turn Inject OFF
      Acquire::Makeup(TRUE);  // and Makeup ON on exit from setup. P.P. 01/09/04
//      WriteFlowMode(Pref);  // not needed! remove later. P.P. 01/02/04
//      flowMode.flow_mode = 0; // added. P.P. 01/09/04
#endif
    }
  }
}

/// end of inserted code for the flow setup. P.P. 12/30/03

#ifdef FILTER
class Filter {
 public:
  int num;
  int flag;
  int fnum5;
  Filter() : num(0), flag(0), fnum5(0) {}
};

class FilterDialog : public TransferPlusDlg {
  typedef TransferPlusDlg Inherited;
 public:
  static ControlInfo far ci[];
  FilterDialog(PTWindowsObject AParent, int ResourceId, Filter *data, PTModule AModule = NULL)
   : TransferPlusDlg(AParent, ResourceId, data, ci, AModule) {}
  virtual void SetupControls(void *data);
};

ControlInfo FilterDialog::ci[]={
  { citComboList, IDD_FILTER_DETECTOR,  FIELDOFFSET(Filter,num),(LPARAM)" Detector A \0 Detector B \0"  },
  { citComboList, IDD_FILTER_FLAG,      FIELDOFFSET(Filter,flag),(LPARAM)" Smooth \0 1st derivative \0 2nd derivative \0"       },
  { citComboList, IDD_FILTER_NUM,       FIELDOFFSET(Filter,fnum5),(LPARAM)" 5 \0 7 \0 9 \0 11 \0 13 \0 15 \0 17 \0 19 \0 21 \0 23 \0 25 \0"     },
  { citNULL }   // End of array marker
};

void FilterDialog::SetupControls(void *data)
{
  Inherited::SetupControls(data);
}

#ifndef  _SGFILTER_H
#include "sgfilter.h"
#endif

void MainWindow::CMApplyFilter(RTMessage)
{
  if(pRun) {
    static Filter filter;
    if(IDOK==GetModule()->ExecDialog(new FilterDialog(this,ID_FILTDLG, &filter) )) {
      Thinking think("Filtering...");
      int num=filter.num;
      int flag=filter.flag;
      int fnum=5+filter.fnum5;
      int wtflg=0;
      if(pJob->GetFiltData(pRun,num)) {
        FiltData *pFiltData=&pRun->filtData[num];
        int32 huge *p=new int32 huge[pFiltData->size];
        if(!p) {
          message("Not enough memory");
          return;
        }
        sgfilter(pFiltData->p, p,pFiltData->size,fnum,flag,wtflg);
        delete [] pFiltData->p; pFiltData->p=p;
      }
      else if(pJob->GetRawData(pRun)) {
        long size=0;
        {
          for(RawDataIterator it(pRun->pRawData, pRun->RawDataSize, num); it.Ok(); it.GetNext()) {
            size++;
          }
        }
        FiltData *pFiltData=&pRun->filtData[num];
        pFiltData->size=size;
        pFiltData->p=new int32 huge[pFiltData->size];
        if(!pFiltData->p) {
          message("Not enough memory");
          return;
        }
        {
          int32 huge *pCur=pFiltData->p;
          for(RawDataIterator it(pRun->pRawData, pRun->RawDataSize, num); it.Ok(); it.GetNext()) {
            *pCur=it.Get();
            pCur++;
          }
          pFiltData->gain=it.Gain();
        }
        if(pRun->RawDataOffset>=0) { // saved
          pRun->FreeRawData();
        }

        int32 huge *p=new int32 huge[pFiltData->size];
        if(!p) {
          message("Not enough memory");
          return;
        }
        {
          sgfilter(pFiltData->p, p,pFiltData->size,fnum,flag,wtflg);
          delete [] pFiltData->p; pFiltData->p=p;
        }
      }
      Refresh();
    }
  }
}
#endif//FILTER

#ifndef __COMMDLG_H
#include <commdlg.h>
#endif

#ifndef  _REPORT_H
#include "report.h"
#endif

static Report report;

UINT CALLBACK _export fnPrintHook(HWND hwnd, UINT message, WPARAM wParam, LPARAM /*lParam*/)
{
  switch (message) {
    case WM_INITDIALOG:
    {
      CheckDlgButton(hwnd, IDD_PRINT_METHOD, report.bMethod ? 1: 0);
      CheckDlgButton(hwnd, IDD_PRINT_DETECTORS, report.bDetectors ? 1: 0);
      CheckDlgButton(hwnd, IDD_PRINT_COMPONENTS, report.bComponents ? 1: 0);
      CheckDlgButton(hwnd, IDD_PRINT_STANDARDS, report.bStandards ? 1: 0);
      CheckDlgButton(hwnd, IDD_PRINT_RUN_LIST, report.bRunList ? 1: 0);
      CheckDlgButton(hwnd, IDD_PRINT_GRAPH, report.bGraph ? 1: 0);
      CheckDlgButton(hwnd, IDD_PRINT_TABLE, report.bTable ? 1: 0);
      CheckDlgButton(hwnd, IDD_PRINT_STL, report.bSTL ? 1: 0);
      CheckDlgButton(hwnd, IDD_PRINT_TWA, report.bTWA ? 1: 0);
      for(int n=0; n<NUMDETECTORS; n++) {
        char str[50];
        Method *pMethod=0; //??
        wsprintf(str,"Detector %c",'A'+n);
        if(pMethod) wsprintf(strchr(str,0)," '%s'",pMethod->det[n].Label.Get());
        SendDlgItemMessage(hwnd, IDD_PRINT_DETECTOR, CB_ADDSTRING, 0, (LPARAM)(LPCSTR)str);
      }
      SendDlgItemMessage(hwnd, IDD_PRINT_DETECTOR, CB_SETCURSEL, report.num, 0);  
    }
    return FALSE; //TRUE;
    case WM_COMMAND:
      switch(wParam) {
        case IDOK:
          report.bMethod=IsDlgButtonChecked(hwnd, IDD_PRINT_METHOD)==1;
          report.bDetectors=IsDlgButtonChecked(hwnd, IDD_PRINT_DETECTORS)==1;
          report.bComponents=IsDlgButtonChecked(hwnd, IDD_PRINT_COMPONENTS)==1;
          report.bStandards=IsDlgButtonChecked(hwnd, IDD_PRINT_STANDARDS)==1;
          report.bRunList=IsDlgButtonChecked(hwnd, IDD_PRINT_RUN_LIST)==1;
          report.bGraph=IsDlgButtonChecked(hwnd, IDD_PRINT_GRAPH)==1;
          report.bTable=IsDlgButtonChecked(hwnd, IDD_PRINT_TABLE)==1;
          report.bSTL=IsDlgButtonChecked(hwnd, IDD_PRINT_STL)==1;
          report.bTWA=IsDlgButtonChecked(hwnd, IDD_PRINT_TWA)==1;
          report.num=SendDlgItemMessage(hwnd, IDD_PRINT_DETECTOR, CB_GETCURSEL, 0, 0);
        break;
      }
    break;
    default:
    break;
  }
  return FALSE;
}

#ifndef  _JOB_H
#include "job.h"
#endif
#ifndef  _RUNL_PRN_H
#include "runl_prn.h"
#endif
#ifndef  _WINLIB_FONT_H
#include "winlib\font.h"
#endif
#ifndef  _WINLIB_COLOR_H
#include "winlib\color.h"
#endif

class ReportPrintout : public Printout
{
  typedef Printout Inherited;
 protected:
  Method *pMethod;
  Job *pJob;
  Run *pRun;
  PeakList **pPeakList;
  PictInfo **pPictInfo;
  int numPages;
  int pageGraph;
  int pageMethod;
  int pageTable;
  List *listTableStr;
  int TabPositions[5];
 public:
  ReportPrintout(LPCSTR ATitle, Job *_pJob, Run *_pRun, PeakList **_pPeakList, PictInfo **_pPictInfo)
   : Printout(ATitle), pJob(_pJob), pRun(_pRun), pPeakList(_pPeakList), pPictInfo(_pPictInfo)
   , pMethod(_pJob), numPages(1), pageGraph(0), pageMethod(0), pageTable(0)
   , listTableStr(0)
   , hOldFont(0)
   {
   }
  ~ReportPrintout() { destroy(listTableStr); }
  virtual void SetPrintParams(DC *_pdc, WPoint pageSize);
  virtual BOOL HasPage(int page);
  virtual void PrintPage(int page, WRect& rect, unsigned flags);
  virtual void EndPrinting();

  HGDIOBJ hOldFont;
};

BOOL ReportPrintout::HasPage(int page)
{
  return page <= numPages;
}

#ifndef __TIME_H
#include <time.h>
#endif
#ifndef  _COUNTRY_H
#include "country.h"
#endif
#ifndef  _WINLIB_DC_H
#include "winlib\dc.h"
#endif
#ifndef  _DRAW_H
#include "draw.h"
#endif
#ifndef  _PEAKLIST_H
#include "peaklist.h"
#endif
extern char data_rate;           // rate data received in pts/sec.
void PrintMethod(int page,int LinesPerPage,Method *pMethod,DC *pdc,WRect &rect);
void GetPage(DC &ic,WRect *rect,const WPoint &size);

class StrNode: public List::Node, public Str
{
 public:
  StrNode(const char *str=0);
};

StrNode::StrNode(const char *str)
 : Str(str)
{
}

static void PrintPeakListFillTabs(DC &dc, int TabPositions[], int W)
{
// new code, from peak_wnd, modified. P.P. 08/04/17
  TabPositions[0]=dc.GetTextExtent("99 ").X();
  TabPositions[1]=TabPositions[0]; // variable size defined later
  TabPositions[2]=TabPositions[1]+dc.GetTextExtent("12345.12356789  123456789").X(); // increased. P.P. 08/01/17
  TabPositions[3]=TabPositions[2]+dc.GetTextExtent("    123456789").X();  // increased by 2 digit places. P.P. 06/30/17
  TabPositions[4]=TabPositions[3]+dc.GetTextExtent("    123456789").X();
  int delta=W-(TabPositions[4]+dc.GetTextExtent("1234:00").X());
  int wName=dc.GetTextExtent("Name ").X();
  if(delta<wName) delta=wName;
  for(int i=1; i<5; ++i) {
    TabPositions[i]+=delta;
  }
// end of new code. P.P. 08/04/17
}


//static void PrintPeak(DC &dc, int TabPositions[], List *listTableStr, const char *component_name, int Num, double concen, int flags, long t, long Height, long Area, long mul)
//static void PrintPeak(DC &dc, int TabPositions[], List *listTableStr, const char *component_name, int Num, double concen, int flags, long t, long Height, long Area, long mul, Method *pMethod, int nDetector) // P.P. 08/04/17
static void PrintPeak(DC &dc, int TabPositions[], List *listTableStr, const char *component_name, int Num, double concen, int flags, long t, long Height, double Area, long mul, Method *pMethod, int nDetector) // area is double now. P.P. 08/25/17
{
  StrNode *pStrNode=new StrNode;
  char str[140];
  if(Num>0) wsprintf(str,"%d\t",Num); else wsprintf(str,"  \t"); (*pStrNode)+=str;
  {
    (*pStrNode)+=component_name;
    if(TabPositions[0]+dc.GetTextExtent(component_name).X()>TabPositions[1]) { // Add new line for long names
      listTableStr->append(pStrNode);
      pStrNode=new StrNode;
      (*pStrNode)+="\t";
    }
  }
  (*pStrNode)+="\t";
// new code, as in peak_wnd. P.P. 08/04/17
  if(flags&1) {
    if(pMethod->det[nDetector].units == "%") {
      if(concen <0.001) {                        //<10^-8, then ppb. P.P. 08/04/17
//	sprintf(str, "%.0f ", concen*10000000.0);   // P.P. 08/04/17 
	sprintf(str, "%.1f ", concen*10000000.0);   // P.P. 12/25/17
	strcat(str, "ppb");                         //
      }
      else if(concen <1.0) {                        //<10^-8, then ppb. P.P. 08/04/17
	if(concen < 0.01) {
	  sprintf(str, "%.1f ", concen*10000.0);   // P.P. 08/13/17
	}
        else {
	  sprintf(str, "%.0f ", concen*10000.0);   // P.P. 08/04/17
        }
	strcat(str, "ppm");                         // 
      }
      else {
	sprintf(str, "%.2f ", concen);   // P.P. 08/04/17 2 digit after decimal. 
	strcat(str, pMethod->det[nDetector].units);   //  08/01/17 units added.
      }
    }
    else if((pMethod->det[nDetector].units == "ppb")||(pMethod->det[nDetector].units == "PPB")) {
      if(concen >=10000000) {                        //>10^7 in ppb, then show in %. P.P. 08/11/17
	sprintf(str, "%.2f ", concen/100000.0);   //  
	strcat(str, "%");                         // 
      }
      else if(concen >=10000) {                        //>10^4 in ppb, then show in ppm. P.P. 08/11/17
	if(concen >= 100000) {                        // >=100 ppm - show whole numbers
//	  sprintf(str, "%.0f ", concen/1000);   //
	  sprintf(str, "%.1f ", concen/1000);   //  P.P. 12/25/17
	}
        else {                                        // < 100 ppm - show 1 place after decimal dot. P.P. 08/13/17
	  sprintf(str, "%.1f ", concen/1000);   //  
	}
      }
      else {
	sprintf(str, "%.0f ", concen);   // P.P. 08/11/17 2 digit after decimal. 
	strcat(str, pMethod->det[nDetector].units);   //  ppb units added.
      }
    }
    else if((pMethod->det[nDetector].units == "ppm")||(pMethod->det[nDetector].units == "PPM")) {
      if(concen >=10000) {                        //if >=10^4  ppm, then show in %, 2 digits after decimal. P.P. 08/11/17
	sprintf(str, "%.2f ", concen/10000.0);   //  
	strcat(str, "%");                         // 
      }
      else if(concen <10.0) {                        //if <10 ppm, then show in ppb, whole numbers. P.P. 08/11/17
//	sprintf(str, "%.0f ", concen*1000.0);   //  
	sprintf(str, "%.1f ", concen*1000.0);   //  P.P. 12/25/17
	strcat(str, "ppb");                         //
      }
      else {
	if(concen >=100) {
	  sprintf(str, "%.0f ", concen);   // P.P. 08/13/17  if >= 100 ppm, show in whole numbers.
	}
        else {
	  sprintf(str, "%.1f ", concen);   // P.P. 08/11/17  if < 100 ppm, show with 1 digit after decimal.
	}
	strcat(str, pMethod->det[nDetector].units);   //  ppm units added.
      }
    }
    else {
      sprintf(str, "%.6f ", concen);   // P.P. 07/29/17 increase resolution
      strcat(str, pMethod->det[nDetector].units);   // P.P. 07/29/17 increase resolution 08/01/17 units added.
    }
    if(flags&2) strcat( str, " H" );
    if(flags&4) strcat( str, " L" );
    (*pStrNode)+=str;
  }
// end of new code. P.P. 08/04/17
   // done as in 501_test. Needs to be tested. P.P. 01/10/03
  if(t>0) {  // added to eliminate Height and Area from Total. P.P. 01/10/03
    wsprintf(str,"\t%ld",Height*mul); (*pStrNode)+=str;
//    wsprintf(str,"\t%ld",Area  *mul); (*pStrNode)+=str;
    sprintf(str,"\t%.0f", (float) Area *mul); (*pStrNode)+=str;   // area as a float, and using printf. P.P. 08/27/17
    wsprintf(str,"\t%ld:%.2ld", t/60, t%60); (*pStrNode)+=str;
  } // end. P.P. 01/10/03

  listTableStr->append(pStrNode);
}

#include <math.h>
#include "internal.h" // USE_TOTAL()

static List *PrintPeakList(PeakList *pPeakList, Method *pMethod, Detector *pDetector, int num, Run *pRun, int nDetector, DC &dc, int TabPositions[])
{
  List *listTableStr = new List;
  listTableStr->append(new StrNode(" \tName\tConc\tHeight\tArea\tTime"));
//  long mul=pow10( pRun->runrawdata.gain[num] );
  long mul=1;  // for 312 with scaled peaks, new format for data. P.P. 08/04/17
  if(!mul) mul=1; // gain==-1? old file?
#if USE_TOTAL()
  int total_flags=0;
  double total_concen=0;
  long total_H=0;
  long total_A=0;
#endif // USE_TOTAL()
  for(PeakList::Node *pNode=pPeakList->first(); pNode; pNode=pNode->nextNode()) {
    Peak *pPeak=(Peak *)pNode;

    const char *component_name=pPeak->pComponent? pPeak->pComponent->name.Get(): "unknown";
    double concen;
    int flags=pDetector->Concentration(pMethod, pRun, pPeak, nDetector, &concen);
#if USE_TOTAL()
    total_flags|=flags;
    if(flags&1) {
      total_concen+=concen;
      total_H+=pPeak->Height;
      total_A+=pPeak->Area;
    }
#endif // USE_TOTAL()
    long t=pPeak->Time/data_rate;
//    PrintPeak(dc, TabPositions, listTableStr, component_name, pPeak->Num, concen, flags, t, pPeak->Height, pPeak->Area, mul);
    PrintPeak(dc, TabPositions, listTableStr, component_name, pPeak->Num, concen, flags, t, pPeak->Height, pPeak->Area, mul, pMethod, nDetector);  // P.P. 08/04/17
  }
#if USE_TOTAL()
  {
    long t=-1;//pPeak->Time/data_rate;
//    PrintPeak(dc, TabPositions, listTableStr, "Total", -1, total_concen, total_flags&1, t, total_H, total_A, mul);
    PrintPeak(dc, TabPositions, listTableStr, "Total", -1, total_concen, total_flags&1, t, total_H, total_A, mul, pMethod, nDetector);  // P.P. 08/11/17
  }
#endif // USE_TOTAL()
  return listTableStr;
}

void ReportPrintout::SetPrintParams(DC *_pdc, WPoint pageSize)
{
  Inherited::SetPrintParams(_pdc, pageSize);
  WRect pageClient;
  GetPage(*pdc,&pageClient,PageSize);
  int logy=pdc->GetCapability(LOGPIXELSY);
  //int logx=pdc->GetCapability(LOGPIXELSX);

  LOGFONT lf;
  lf.lfHeight=-MulDiv(10,logy,72); // 10 pt ?
  lf.lfWidth=0;
  lf.lfEscapement=0;
  lf.lfOrientation=0;
  lf.lfWeight=FW_NORMAL;
  lf.lfItalic=0;
  lf.lfUnderline=0;
  lf.lfStrikeOut=0;
  lf.lfCharSet=DEFAULT_CHARSET;
  lf.lfOutPrecision=OUT_DEFAULT_PRECIS;
  lf.lfClipPrecision=CLIP_DEFAULT_PRECIS;
  lf.lfQuality=PROOF_QUALITY;
  lf.lfPitchAndFamily=VARIABLE_PITCH|FF_SWISS;
  lf.lfFaceName[0]=0;

  HFONT hfont=CreateFontIndirect(&lf);
  hOldFont=SelectObject(*pdc,hfont);

    PrintPeakListFillTabs(*pdc, TabPositions, pageClient.Width());

  if(report.bTable && pPeakList[report.num]) {
    destroy(listTableStr);
    listTableStr=PrintPeakList(pPeakList[report.num], pMethod, &pMethod->det[report.num], report.num, pRun, report.num, *pdc, TabPositions);
  }

  int page;
  for(page=1; ;++page) {
    WRect rc=pageClient;
    int topHeader=0.5*logy; // 0.5"
    rc.Top()+=topHeader;
    BOOL bEmpty=TRUE;

    if(!pageGraph && report.bGraph && pRun) {
      rc.Top()-=topHeader/2; // trick
      int graphHeight=5*logy; // 5"
      rc.Top()=rc.Top()+graphHeight;
      if(bEmpty || rc.Top()<rc.Bottom()) {
        pageGraph=page;
        bEmpty=FALSE;
      }
      else continue; // goto next page
    }
    if(!pageMethod && report.bMethod && pMethod) {
      int methodHeight=2*logy; // 2"
      if(pMethod->method_table.RampingEnabled) methodHeight+=logy;
      rc.Top()=rc.Top()+methodHeight;
      if(bEmpty || rc.Top()<rc.Bottom()) {
        pageMethod=page;
        bEmpty=FALSE;
      }
      else continue; // goto next page
    }
    if(!pageTable && report.bTable && pPeakList[report.num]) {
      int hLine=pdc->GetTextExtent("H").Y();
      if(listTableStr) {
        for( List::Iterator p(*listTableStr); p; ++p) {
          rc.Top()+=hLine;
        }
      }
      if(bEmpty || rc.Top()<rc.Bottom()) {
        pageTable=page;
        bEmpty=FALSE;
      }
      else continue; // goto next page
    }
    if(bEmpty) break;
  }
  numPages=page-1;
}


void ReportPrintout::PrintPage(int page, WRect& /*rect*/, unsigned /*flags*/)
{
  pdc->Save();
  SelTextAlign oldTextAlign(*pdc,taLeft|taTop);
  //pdc->IntersectClipRect(rect);
  int logy=pdc->GetCapability(LOGPIXELSY);
  //int logx=pdc->GetCapability(LOGPIXELSX);

  WRect rc;
  GetPage(*pdc,&rc,PageSize);
  pdc->IntersectClipRect(rc);

  char title[240];
  int nRun=0;
  {
    for(RunList::Node *prln=pJob->first(); prln; prln=prln->nextNode()) {
      nRun++;
      if(pRun==prln) break;
    }
  }
  time_t tim;
  time( &tim );
  wsprintf(title, "'%s' ", pJob->FileName.Get());
  if(pRun) wsprintf(strchr(title,0), "Run #%d ", nRun);
//  if(report.bDetectors) wsprintf(strchr(title,0), "'%s' ", pMethod->det[report.num].Label.Get());
  if(report.bDetectors) {  // detector and units added. P.P. 08/18/17
    wsprintf(strchr(title,0), "; '%s' ", pMethod->det[report.num].Label.Get());
    wsprintf(strchr(title,0), "; Units: %s; ", pMethod->det[report.num].units);
  }
  wsprintf(strchr(title,0), "%s %s",  PrintDate(tim), PrintTime(tim));
  TextOut(*pdc, rc.Left(), rc.Top(), title, lstrlen(title));
  if(pRun) TextOut(*pdc, rc.Left(), rc.Top()+pdc->GetTextExtent("H").Y(), pRun->comment, lstrlen(pRun->comment));
  int topHeader=0.5*logy; // 0.5"
  rc.Top()+=topHeader;
  if(page==pageGraph && report.bGraph && pRun) {
    rc.Top()-=topHeader/2; // trick
    WRect rcGraph=rc;
    int graphHeight=5*logy; // 5"
    rcGraph.Bottom()=rcGraph.Top()+graphHeight;
    if(
      #ifdef FILTER
      pJob->GetFiltData(pRun,report.num) ||
      #endif//FILTER
      pJob->GetRawData(pRun)) {
      ::Draw(pJob, pRun, pPictInfo,*pdc, *pdc, rcGraph, pRun->runrawdata,
        #ifdef FILTER
        pRun->filtData,
        #endif//FILTER
        report.num, pPeakList, 0);
    }
    rc.Top()=rcGraph.Bottom();
  }
  if(page==pageMethod && report.bMethod && pMethod) {
    WRect rcMethod=rc;
    int methodHeight=2*logy; // 2"
    if(pMethod->method_table.RampingEnabled) methodHeight+=logy;
    rcMethod.Bottom()=rcMethod.Top()+methodHeight;
    PrintMethod(1,32000,pMethod,pdc,rcMethod);
    rc.Top()=rcMethod.Bottom();
  }

  if(page>=pageTable && report.bTable && pPeakList[report.num]) {
    int x=rc.Left();
    int y=rc.Top();
    //Results Section
    int hLine=pdc->GetTextExtent("H").Y();
    if(listTableStr) {
      for( List::Iterator p(*listTableStr); p; ++p) {
        StrNode *pStrNode=(StrNode *)(List::Node *)p;
        pdc->TabbedTextOut(x, y, pStrNode->Get(), pStrNode->Length(), 5, TabPositions, x);
        y+=hLine;
      }
    }
    rc.Top()=y;
  }
/// report.bMethod;
// report.bDetectors;
// report.bComponents;
// report.bStandards;
//\ report.bRunList;
/// report.bGraph;
/// report.bTable;
//\ report.bSTL;
//\ report.bTWA;
  //Comments Section

  pdc->Restore();
}

void ReportPrintout::EndPrinting()
{
  if(pdc) {
    DeleteObject(SelectObject(*pdc,hOldFont));
    hOldFont=0;
  }

  Inherited::EndPrinting();
}

#ifndef  _WINLIB_SELFONT_H
#include "winlib\selfont.h"
#endif
#include "peak_app.h"

void MainWindow::CMPrintReport(RTMessage)
{
  if(!pJob) return;
  PeakApp *pApp=((PeakApp *)GetApplication());
  Printer *pPrinter=pApp;
  if( pPrinter ) {
    PRINTDLG *Data=&pPrinter->GetSetup();
    DWORD Flags=Data->Flags;
    Data->Flags|=PD_ENABLEPRINTTEMPLATE;
    Data->lpPrintTemplateName=MAKEINTRESOURCE(ID_PRINTDLG);
    Data->Flags|=PD_ENABLEPRINTHOOK;
    (FARPROC)Data->lpfnPrintHook=MakeProcInstance((FARPROC)fnPrintHook,Data->hInstance);

    ReportPrintout *pPrintout = new ReportPrintout(pApp->Name, pJob, pRun, pPeakList, pPictInfo);
    if( pPrintout ) {
      pPrinter->Print(this, pPrintout, TRUE);
      delete pPrintout;
    }

    if(Data->lpfnPrintHook) {
      FreeProcInstance((FARPROC)Data->lpfnPrintHook); Data->lpfnPrintHook=0;
    }
    Data->Flags=Flags;
  }
}

////////////////

extern const unsigned CF_RTF; // peak_wnd.cpp
extern void PrintPeakToFile(owstream &os,UINT cf, PeakList *pPeakList, Method *pMethod, Detector *pDetector, Run *pRun, int num);

void writebra(owstream &os)
{
  os.write("{",1);
}

void writeket(owstream &os)
{
  os.write("}",1);
}

void writecw(owstream &os,const char *cw)
{
  os.write("\\",1);
  os.write(cw,strlen(cw));
}

void writecwandspace(owstream &os,const char *cw)
{
  os.write("\\",1);
  os.write(cw,strlen(cw));
  os.write(" ",1);
}

void writecw1(owstream &os,const char *cw) // optional
{
  os.write("\\*\\",3);
  os.write(cw,strlen(cw));
}

void writecw1andspace(owstream &os,const char *cw) // optional
{
  os.write("\\*\\",3);
  os.write(cw,strlen(cw));
  os.write(" ",1);
}

void writeln(owstream &os)
{
  os.write("\r\n",2);
}

void writertfstring(owstream &os,const char *str)
{
  int len=strlen(str);
  int k=0;
  for(const char *cp=str; len>0 && *cp; ++cp, --len) {
    if((*(uint8 *)cp>128 || *(uint8 *)cp<32) || *(uint8 *)cp==9) { // not printable or tab
      switch(*(uint8 *)cp) {
        case 9:   writecwandspace(os,"tab");    break;
        case 145: writecwandspace(os,"lquote");    break;
        case 146: writecwandspace(os,"rquote");    break;
        case 147: writecwandspace(os,"ldblquote"); break;
        case 148: writecwandspace(os,"rdblquote"); break;
        case 149: writecwandspace(os,"bullet");    break;
        case 150: writecwandspace(os,"endash");    break;
        case 151: writecwandspace(os,"emdash");    break;
        default:
          if((unsigned char)(*cp)<32) ;
          else
          {
            static char str[]="\\\'xx";
            if((str[2]=char('0'+(*(uint8 *)cp)/16))>'9') str[2]+=char('a'-'9'-1);
            if((str[3]=char('0'+(*(uint8 *)cp)%16))>'9') str[3]+=char('a'-'9'-1);
            os.write(str,4);
          }
        break;
      }
      k+=4;
      if(k>70) { writeln(os); k=0; }
    }
    else {
      switch(*cp) {
        case '{':
          os.write("\\{",2); k+=2;
        break;
        case '}':        
          os.write("\\}",2); k+=2;
        break;
        case '\\':
          os.write("\\\\",2); k+=2;
        break;
        default:
          os.write(cp,1); k++;
        break;
      }
      if(*cp==' ' && k>70 || k>90) { writeln(os); k=0; }
    }
  }
}

#include "wcpplib\wrect.h"
#include "winlib\metafile.h"
#include "peak_app.h"

extern MetafileDC *CreateMetafile(DC *infoDC,Job *pJob,Run *pRun,PeakList **pPeakList,PictInfo **pPictInfo, int num, WRect *rect); // GRPH_WND.CPP
class ortfwstreambuf : public wstreambuf {
 protected:
  owstream &rtf;
  long l;
 public:
  ortfwstreambuf(owstream &_rtf) : rtf(_rtf), l(0) {}
  virtual unsigned maxlen() { return 65534U; }
  virtual unsigned    write(const void *p, unsigned len);
  virtual wstreampos seek(wstreampos offset, wstream::seek_dir fromwhere);
};

unsigned ortfwstreambuf::write(const void *p, unsigned len)
{
  unsigned n=0;
  for(; len--; n++) {
    if(l++%40==0) writeln(rtf);
    char str[2];
    uint8 c=*(((uint8 *)p)++);
    static const char itoa2_16[16]="0123456789ABCDEF";
    str[0]=itoa2_16[(c>>4)];
    str[1]=itoa2_16[c&'\x0f'];
    rtf.write(str,2);
  }
  return n;
}

wstreampos ortfwstreambuf::seek(wstreampos /*offset*/, wstream::seek_dir /*fromwhere*/)
{
  return -1;
}

#include "time.h"
extern void StrPrintMethod(Method *pMethod, List *list); // meth_dlg.h
void MainWindow::CMSaveReport(RTMessage)
{
  if(!pJob) return;
  HINSTANCE hInstance=GetApplication()->hInstance;
  Str strFilter(128);
  LoadFilterString(hInstance, IDS_RTF_FILTER, strFilter.GetPtr(), 128);
  Str strTitle(64);
  LoadString(hInstance, IDS_SAVEREPORT, strTitle.GetPtr(), 64);
  Str FileName;
  BOOL ret=OpenSaveDialog(HWindow, strTitle, strFilter, &FileName, TRUE);
  if(ret) {
    ofwstream os(FileName);
    if(!os.good()) {
      message("Cannot open file:\n%s", FileName.Get());
    }
    else {
      Thinking think("Writting file: '%s'...", FileName.Get());
      static const char _far deffont[]=
      "\\pard\\plain\\fs20\\lang1033"; // fonts size 20/2=10pt English(USA)

      writebra(os); writecw(os,"rtf1"); writecw(os,"ansi");
      writeln(os); writebra(os); writecw(os,"info");
      writeln(os); writebra(os); writecwandspace(os,"comment");  writertfstring(os,"Written by wPeak"); writeket(os);
      writeln(os); writebra(os); writecwandspace(os,"title");  writertfstring(os,pJob->FileName); writeket(os);
      if(!pJob->comment.IsEmpty()) {
        writeln(os); writebra(os); writecwandspace(os,"doccomm"); writertfstring(os,pJob->comment); writeket(os);
      }
      //"{\subject Topic}\r\n"
      //"{\author Author}\r\n"
      writeln(os);
      time_t tim;
      time( &tim );
      struct tm *adjtime = localtime( &tim );
      {
        char *buf=new char[128];
        wsprintf(buf,"{\\creatim\\yr%d\\mo%d\\dy%d\\hr%d\\min%d}",
          1900+adjtime->tm_year,
          1+adjtime->tm_mon,
          adjtime->tm_mday,
          1+adjtime->tm_hour,
          adjtime->tm_min
        );
        os.write(buf,strlen(buf));
        delete []buf;
      }
      writeln(os); writeket(os); //info

      writeln(os); writebra(os); writecw(os,"header");
      writeln(os); os.write(deffont,sizeof(deffont)-1);

      writeln(os); writebra(os); writecw(os,"field");
      writebra(os); writecw1andspace(os,"fldinst"); writertfstring(os,"TITLE"); writeket(os);
      writebra(os); writecwandspace(os,"fldrslt"); writertfstring(os,pJob->FileName.Get()); writeket(os);
      writeket(os); //field

      writeln(os);
      {
        int nRun=0;
        {
          for(RunList::Node *prln=pJob->first(); prln; prln=prln->nextNode()) {
            nRun++;
            if(pRun==prln) break;
          }
        }
        if(pRun) {
          Str buff;
          if(pRun->type_of_run==CAL) {
            wsprintf(buff.Alloc(48), " Run #%d, calibration", nRun);
          }
          else {
            #if POINTS()>1
            wsprintf(buff.Alloc(48), " Run #%d, pt#%d", nRun, 1+pRun->pointnumber);
            #else
            wsprintf(buff.Alloc(28), " Run #%d", nRun);
            #endif
          }
          writertfstring(os,buff);
        }
        writeln(os);
      }
      writebra(os); writecw(os,"field");
      writebra(os); writecw1andspace(os,"fldinst"); writertfstring(os,"CREATEDATE \\* MERGEFORMAT"); writeket(os);
      writebra(os); writecwandspace(os,"fldrslt");
      {
        Str buff(58);
        wsprintf(buff.GetPtr(), "%s %s",  PrintDate(tim), PrintTime(tim));
        writertfstring(os,buff);
      }
      writeket(os); //fldrslt
      writeket(os); //field

      if(pRun && !pRun->comment.IsEmpty()) {
        writeln(os);
        writecwandspace(os,"par");
        writertfstring(os,pRun->comment);
      }

      writeln(os);
      writeket(os); //header

      Method *pMethod=pJob;

      BOOL bFirst=TRUE;
      for(int n=0; n<NUMDETECTORS; n++) {
        //if(!pPeakList[n]) continue;
        if(!pMethod->det[n].detect_stat) continue;
        writeln(os); writecwandspace(os,"pard"); os.write(deffont,sizeof(deffont)-1);
        Detector *pDetector=&pMethod->det[n];

        if(bFirst) bFirst=FALSE;
        else {
          writeln(os);
          writecwandspace(os,"page");
        }

        writeln(os);
        writertfstring(os,"Detector: ");
        writertfstring(os,pMethod->det[n].Label);
	writertfstring(os,";  Units: ");               // added, to show units selected in the Detector dialog. P.P. 08/18/17
        writertfstring(os,pMethod->det[n].units);   //
	writeln(os);
        writecwandspace(os,"par");

        if(pRun) { // Graph
          WRect rc;
          MetafileDC *dc=0;
          {
            DC *infoDC=0;
            PeakApp *pApp=((PeakApp *)GetApplication());
            Printer *pPrinter=pApp;
            if( pPrinter ) infoDC=pPrinter->GetDC(this,0);
            if(!infoDC) infoDC=new WindowDC(HWindow);
            dc=CreateMetafile(infoDC,pJob,pRun,pPeakList,pPictInfo,n, &rc);
            delete infoDC;
          }

          if(dc) {
            writeln(os);
            Str buff(128);
            wsprintf(buff.GetPtr(),
              "{\\pict\\wmetafile8\\picw%d\\pich%d\\picwgoal%d\\pichgoal%d ",
              rc.Width(),rc.Height(),MulDiv(rc.Width(),1440,2540),MulDiv(rc.Height(),1440,2540)
            );
            os.write(buff,strlen(buff));

            ortfwstreambuf buf(os);
            owstream os1(&buf);
            //MetaFileBitsTemp(*p).Write(os);

            MetaFile mf(*dc);
            mf.Write(os1/*,rc,mmAnisotropic*/);

            writeln(os);
            writeket(os); // pict
          }
          delete dc;
        }

        if(pMethod) { // Method Table
          List *list = new List;
          writeln(os); writecw(os,"pard"); writecw(os,"tx2880"); writecwandspace(os,"tx5760");
          StrPrintMethod(pMethod, list);
          for( List::Iterator p(*list); p; ++p) {
            StrNode *pStrNode=(StrNode *)(List::Node *)p;
            writeln(os);
            writecwandspace(os,"par");
            writertfstring(os,*pStrNode);
          }
          destroy(list);
        }


        writeln(os); writebra(os);
        writecw(os,"par");
        writecw(os,"pard");
        writeln(os);
        PrintPeakToFile(os,CF_RTF, pPeakList[n], pMethod, pDetector, pRun, n); // peak_wnd.cpp
        writeln(os); writeket(os); // restore ParProp
      }

      writeln(os); writeket(os); writeln(os);//rtf
    }
  }
}

//////////////////////////////////

#if 1

static void WinMessageFunc( va_list argptr, const char *fmt )
{
#ifdef _Windows
  //MessageBeep(MB_ICONSTOP);
#ifdef BUILD_MYMEM_DLL
  static
#endif
  char mess[256];
  vsprintf(mess,fmt,argptr);
  //MessageBox(NULL,mess,"MyMem Error",MB_OK|MB_ICONSTOP|/*MB_TASKMODAL*/MB_SYSTEMMODAL);
  {
    extern HINSTANCE _hInstance;
    static char filename[80];
    GetModuleFileName(_hInstance, filename, sizeof(filename));
    char *cp=strrchr(filename,'.');
    if(cp) {
      strcpy(cp,".log");
      HFILE h;
      static BOOL bFirst=TRUE;
      if(bFirst) { h=_lcreat(filename,0); bFirst=FALSE; }
      else { h=_lopen(filename,WRITE); _llseek(h,0,2); }
      _lwrite(h,mess,lstrlen(mess));
      _lwrite(h,"\r\n",2);
      _lclose(h);
    }
    else {
      MessageBox(NULL,mess,"MyMem Error",MB_OK|MB_ICONSTOP|/*MB_TASKMODAL*/MB_SYSTEMMODAL);
    }
  }

#ifdef __DLL__
  HFILE h=_lopen("mymem.err",WRITE);
  _llseek(h,0,2);
  _lwrite(h,mess,lstrlen(mess)+1);
  _lwrite(h,"\r\n",2);
  _lclose(h);
#endif  // __DLL__
#endif  // Windows
}

// Special function to set breakpoints
/*static */void MyMemMessage( const char *fmt, ... )
{
  va_list ap;
  va_start(ap, fmt);
  /*vLibMessage*/WinMessageFunc( ap, fmt );
  va_end(ap);
#ifndef __WIN32__
  asm int 3;
#endif
}

void assertfail(char *__msg,char *__cond,char *__file,int __line)
{
  MyMemMessage( __msg, __cond, __file, __line );
}

#endif  // 1





