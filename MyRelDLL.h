#ifndef  _MYRELDLL_H
#define  _MYRELDLL_H

#ifndef __WIN32__

#ifndef _INC_WINDOWS
#include <windows.h>
#endif
// JSB218()
  // from RelayDLL.H
#define JSB_NO_ERROR                      1         // function completed ok
#define JSB_ERR_MODULE_NOT_FOUND        -10         // could not find the module
#define JSB_ERR_RELAY_NUM_NOT_VALID     -20         // relay number not valid
#define JSB_ERR_COMM_FAIL               -30         // communication error with USB Driver

int relaydll_ModuleRelayOn(LPCSTR strModule,int nRelayNum);
int relaydll_ModuleRelayOff(LPCSTR strModule,int nRelayNum);
int relaydll_ModuleRelayMask(LPCSTR strModule,int nRelayMask);
int relaydll_ModuleAllRelayOn(LPCSTR strModule);
int relaydll_ModuleAllRelayOff(LPCSTR strModule);
int relaydll_ModuleRelayStates(LPCSTR strModule);
BOOL relaydll_IsModuleRelayOn(LPCSTR strModule,int nRelayNum);
int relaydll_GetNumberOfModules();
void relaydll_GetSerialNumOfModule(int nModule,LPSTR strReturn);

//*****************************************************************************
int relaydll_ModuleFlashLED(LPCSTR strModule);
int relaydll_GetNumRelays(LPCSTR strModule);

#else // __WIN32__

//#include "RelayDLL.H"

// use dynamic linking of DLL due to DLL function names conflict
#include "winutil/dllproc.h"

class RelayDLL : public DProcModule
{
public:
  RelayDLL(const TCHAR *path);

public:
  DModuleProc2<int, LPCSTR,int> ModuleRelayOn;
  DModuleProc2<int, LPCSTR,int> ModuleRelayOff;
  DModuleProc2<int, LPCSTR,int> ModuleRelayMask;
  DModuleProc1<int, LPCSTR> ModuleAllRelayOn;
  DModuleProc1<int, LPCSTR> ModuleAllRelayOff;
  DModuleProc1<int, LPCSTR> ModuleRelayStates;
  DModuleProc2<BOOL, LPCSTR, int> IsModuleRelayOn;
  DModuleProc0<int> GetNumberOfModules;
  DModuleProcV2<int, LPSTR> GetSerialNumOfModule;
  DModuleProc1<int, LPCSTR> ModuleFlashLED;
  DModuleProc1<int, LPCSTR> GetNumRelays;
};

extern RelayDLL RelayDll;


#define relaydll_ModuleRelayOn		RelayDll.ModuleRelayOn
#define relaydll_ModuleRelayOff		RelayDll.ModuleRelayOff
#define relaydll_ModuleRelayMask	RelayDll.ModuleRelayMask
#define relaydll_ModuleAllRelayOn	RelayDll.ModuleAllRelayOn
#define relaydll_ModuleAllRelayOff	RelayDll.ModuleAllRelayOff
#define relaydll_ModuleRelayStates	RelayDll.ModuleRelayStates
#define relaydll_IsModuleRelayOn	RelayDll.IsModuleRelayOn
#define relaydll_GetNumberOfModules	RelayDll.GetNumberOfModules
#define relaydll_GetSerialNumOfModule	RelayDll.GetSerialNumOfModule

//*****************************************************************************
#define relaydll_ModuleFlashLED		RelayDll.ModuleFlashLED
#define relaydll_GetNumRelays		RelayDll.GetNumRelays

#endif // __WIN32__

#endif // _MYRELDLL_H
