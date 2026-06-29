#include "includes.h"
#pragma hdrstop

#ifndef __OWL_H
#include <owl.h>
#endif

#ifndef  _ID_H
#include "id.h"
#endif
#ifndef  _MAIN_WND_H
#include "main_wnd.h"	// newMainWindow()
#endif
#ifndef  _PEAK_APP_H
#include "peak_app.h"
#endif


#include "wcpplib/3dui.h"



static const char Ctl3dDllName[] = "CTL3DV2.DLL";

PeakApp::PeakApp(LPSTR AName, HINSTANCE hInstance,
		HINSTANCE hPrevInstance, LPSTR lpCmd,
		int nCmdShow)
  : TApplication(AName, hInstance, hPrevInstance, lpCmd, nCmdShow)
  , Ctl3dModule(NULL), Ctl3dOn(FALSE)
{
  Pref.SetIniFile(hInstance);
}

PeakApp::~PeakApp()
{
  if (Ctl3dModule) {
    BOOL (FAR PASCAL *Ctl3dUnregister)(HANDLE);
    (FARPROC &)Ctl3dUnregister = GetProcAddress(Ctl3dModule,"Ctl3dUnregister");
    if (Ctl3dUnregister)
      Ctl3dUnregister(*this);
    FreeLibrary(Ctl3dModule);
  }
}


BOOL PeakApp::Ctl3dAvailable() const
{
  if(Ctl3dModule) return TRUE;

  //
  // If we are running Win4.0 or greater, then ctl3d is unnecessary
  //
  if(__Has3dUI())
    return FALSE; //not needed!

  HINSTANCE hInst;
  {
    UINT oldErrorMode = SetErrorMode(0);
    SetErrorMode(oldErrorMode | SEM_FAILCRITICALERRORS|SEM_NOOPENFILEERRORBOX);
    hInst=LoadLibrary(Ctl3dDllName);
    SetErrorMode(oldErrorMode);
  }
  if(hInst<(HINSTANCE)(32)) return FALSE;
  FreeLibrary(hInst);
  return TRUE;
}

void PeakApp::EnableCtl3d(BOOL enable)
{
  //
  // If we are running Win4.0 or greater, then ctl3d is unnecessary
  //
  if(__Has3dUI())
    return;

  //
  // Load the Ctl3d DLL if needed & register our instance
  //
  if (enable) {
    if (!Ctl3dModule) {
      Ctl3dModule = LoadLibrary(Ctl3dDllName);
      if(Ctl3dModule<(HINSTANCE)(32)) Ctl3dModule=0;
      BOOL (FAR PASCAL *ctl3dRegister)(HANDLE);
      (FARPROC &)ctl3dRegister = GetProcAddress(Ctl3dModule,"Ctl3dRegister");
      if (ctl3dRegister)
	ctl3dRegister(*this);
      else {
	FreeLibrary(Ctl3dModule);
	Ctl3dModule = NULL;
	return;   // could not register--don't change Ctl3dOn flag
      }
    }
  }
  Ctl3dOn = enable;
}
void PeakApp::EnableCtl3dAutosubclass(BOOL enable)
{
  if (Ctl3dEnabled()) {
    if (enable) {
      BOOL (FAR PASCAL *ctl3dRegister)(HANDLE);
      (FARPROC &)ctl3dRegister = GetProcAddress(Ctl3dModule,"Ctl3dRegister");
      if (ctl3dRegister) {
	ctl3dRegister(*this);

	BOOL (FAR PASCAL *ctl3dAutoSubclass)(HANDLE);
	(FARPROC &)ctl3dAutoSubclass = GetProcAddress(Ctl3dModule,"Ctl3dAutoSubclass");
	if (ctl3dAutoSubclass)
	  ctl3dAutoSubclass(*this);
      }
    }
    else {
      BOOL (FAR PASCAL *Ctl3dUnregister)(HANDLE);
      (FARPROC &)Ctl3dUnregister = GetProcAddress(Ctl3dModule,"Ctl3dUnregister");
      if (Ctl3dUnregister)
	Ctl3dUnregister(*this);
    }
  }
}



void PeakApp::InitMainWindow()
{
  MainWindow = newMainWindow(NULL,ID_MAIN_STATUS,"wPeak.hlp",Pref,this);
  HAccTable = LoadAccelerators(hInstance,MAKEINTRESOURCE(ID_MAIN_STATUS));
}

void PeakApp::IdleAction()
{
  if(MainWindow)
    if(MainWindow->HWindow)
      SendMessage(MainWindow->HWindow,WM_IDLE,0,0L);
}

void PeakApp::Error(int ErrorCode)
{
  Inherited::Error(ErrorCode);
}

#ifndef __COMMDLG_H
#include <commdlg.h>
#endif

#ifndef __DLGS_H
#include <dlgs.h>
#endif
UINT CALLBACK _export fnDefPrintHook(HWND hwnd, UINT message, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
  switch (message) {
    case WM_INITDIALOG:
      ShowWindow(GetDlgItem(hwnd,grp1),SW_HIDE);
      ShowWindow(GetDlgItem(hwnd,rad1),SW_HIDE);
      ShowWindow(GetDlgItem(hwnd,rad2),SW_HIDE);
      ShowWindow(GetDlgItem(hwnd,rad3),SW_HIDE);
      ShowWindow(GetDlgItem(hwnd,stc2),SW_HIDE);
      ShowWindow(GetDlgItem(hwnd,edt1),SW_HIDE);
      ShowWindow(GetDlgItem(hwnd,stc3),SW_HIDE);
      ShowWindow(GetDlgItem(hwnd,edt2),SW_HIDE);
    return FALSE; //TRUE;
    default:
    break;
  }
  return FALSE;
}

BOOL PeakApp::ExecPrintDialog(PTWindowsObject parent)
{
  if(!Data) return FALSE;
  DWORD Flags=Data->Flags;
  Data->Flags|=PD_NOPAGENUMS|PD_NOSELECTION;
  Data->hInstance=hInstance;
  int b=0;
  if(!Data->lpfnPrintHook) {
    Data->Flags|=PD_ENABLEPRINTHOOK;
    (FARPROC)Data->lpfnPrintHook=MakeProcInstance((FARPROC)fnDefPrintHook,Data->hInstance);
    b=1;
  }
  BOOL ret=Printer::ExecPrintDialog(parent);
  if(b) { FreeProcInstance((FARPROC)Data->lpfnPrintHook); Data->lpfnPrintHook=0; }
  Data->Flags=Flags;
  return ret;
}
