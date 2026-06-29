#pragma hdrstop

#include "internal.h"

//#if JSB218()
#if JSB218()||JSB215() // changed. P.P. 10/30/05

#include "myRelDLL.h"

static const char far relaydllname[]="relayDLL.dll";

#ifndef __WIN32__

#include "wow.h"

class RelayDll : public WOWDLL {
 public:
  RelayDll();
};

RelayDll::RelayDll()
 : WOWDLL(relaydllname)
{
  if(!Ok()) {
    #if !DEMO()
    MessageBox(NULL,"relayDLL.DLL not found", "wPeak", MB_OK);
    #endif
  }
}

static RelayDll relaydll;

int relaydll_ModuleRelayOn(LPCSTR strModule,int nRelayNum)
{
  //_pMainWindow->print("  ModuleRelayOn(\'%s\', %d)\r\n", strModule, nRelayNum);
  int ret=0;
  if(relaydll.Ok()) {
    DWORD pFunc=relaydll.GetProcAddress32W("ModuleRelayOn");
    if(pFunc) {
      ret=relaydll.CallProcEx32W( 2|CPEX_DEST_STDCALL, 0x01, pFunc, (DWORD)strModule, (DWORD)nRelayNum);
    }
  }
  return ret;
}
int relaydll_ModuleRelayOff(LPCSTR strModule,int nRelayNum)
{
  //_pMainWindow->print("  ModuleRelayOff(\'%s\', %d)\r\n", strModule, nRelayNum);
  int ret=0;
  if(relaydll.Ok()) {
    DWORD pFunc=relaydll.GetProcAddress32W("ModuleRelayOff");
    if(pFunc) {
      ret=relaydll.CallProcEx32W( 2|CPEX_DEST_STDCALL, 0x01, pFunc, (DWORD)strModule, (DWORD)nRelayNum);
    }
  }
  return ret;
}
int relaydll_ModuleRelayMask(LPCSTR strModule,int nRelayMask)
{
  //_pMainWindow->print("  ModuleRelayMask(\'%s\', %d)\r\n", strModule, nRelayMask);
  int ret=0;
  if(relaydll.Ok()) {
    DWORD pFunc=relaydll.GetProcAddress32W("ModuleRelayMask");
    if(pFunc) {
      ret=relaydll.CallProcEx32W( 2|CPEX_DEST_STDCALL, 0x01, pFunc, (DWORD)strModule, (DWORD)nRelayMask);
    }
  }
  return ret;
}
int relaydll_ModuleAllRelayOn(LPCSTR strModule)
{
  //_pMainWindow->print("  ModuleAllRelayOn(\'%s\')\r\n", strModule);
  int ret=0;
  if(relaydll.Ok()) {
    DWORD pFunc=relaydll.GetProcAddress32W("ModuleAllRelayOn");
    if(pFunc) {
      ret=relaydll.CallProcEx32W( 1|CPEX_DEST_STDCALL, 0x01, pFunc, (DWORD)strModule);
    }
  }
  return ret;
}
int relaydll_ModuleAllRelayOff(LPCSTR strModule)
{
  //_pMainWindow->print("  ModuleAllRelayOff(\'%s\')\r\n", strModule);
  int ret=0;
  if(relaydll.Ok()) {
    DWORD pFunc=relaydll.GetProcAddress32W("ModuleAllRelayOff");
    if(pFunc) {
      ret=relaydll.CallProcEx32W( 1|CPEX_DEST_STDCALL, 0x01, pFunc, (DWORD)strModule);
    }
  }
  return ret;
}
int relaydll_ModuleRelayStates(LPCSTR strModule)
{
  //_pMainWindow->print("  ModuleRelayStates(\'%s\')\r\n", strModule);
  int ret=0;
  if(relaydll.Ok()) {
    DWORD pFunc=relaydll.GetProcAddress32W("ModuleRelayStates");
    if(pFunc) {
      ret=relaydll.CallProcEx32W( 1|CPEX_DEST_STDCALL, 0x01, pFunc, (DWORD)strModule);
    }
  }
  return ret;
}
BOOL relaydll_IsModuleRelayOn(LPCSTR strModule,int nRelayNum)
{
  //_pMainWindow->print("  IsModuleRelayOn(\'%s\', %d)\r\n", strModule, nRelayNum);
  BOOL ret=0;
  if(relaydll.Ok()) {
    DWORD pFunc=relaydll.GetProcAddress32W("IsModuleRelayOn");
    if(pFunc) {
      ret=relaydll.CallProcEx32W( 2|CPEX_DEST_STDCALL, 0x01, pFunc, (DWORD)strModule, (DWORD)nRelayNum);
    }
  }
  return ret;
}
int relaydll_GetNumberOfModules()
{
  //_pMainWindow->print("  GetNumberOfModules()\r\n");
  int ret=0;
  if(relaydll.Ok()) {
    DWORD pFunc=relaydll.GetProcAddress32W("GetNumberOfModules");
    if(pFunc) {
      ret=relaydll.CallProcEx32W( 0|CPEX_DEST_STDCALL, 0x00, pFunc);
    }
  }
  return ret;
}
void relaydll_GetSerialNumOfModule(int nModule,LPSTR strReturn)
{
  //_pMainWindow->print("  GetSerialNumOfModule(%d)\r\n", nModule);
  if(relaydll.Ok()) {
    DWORD pFunc=relaydll.GetProcAddress32W("GetSerialNumOfModule");
    if(pFunc) {
      relaydll.CallProcEx32W( 2|CPEX_DEST_STDCALL, 0x02, pFunc, (DWORD)nModule, (DWORD)strReturn);
    }
  }
  ////_pMainWindow->print("GetSerialNumOfModule(%d) results \'%s\'\r\n", nModule, strReturn);
}

//*****************************************************************************
int relaydll_ModuleFlashLED(LPCSTR strModule)
{
  //_pMainWindow->print("  ModuleFlashLED(\'%s\')\r\n", strModule);
  BOOL ret=0;
  if(relaydll.Ok()) {
    DWORD pFunc=relaydll.GetProcAddress32W("ModuleFlashLED");
    if(pFunc) {
      ret=relaydll.CallProcEx32W( 1|CPEX_DEST_STDCALL, 0x01, pFunc, (DWORD)strModule);
    }
  }
  return ret;
}
int relaydll_GetNumRelays(LPCSTR strModule)
{
  //_pMainWindow->print("  GetNumRelays(\'%s\')\r\n", strModule);
  BOOL ret=0;
  if(relaydll.Ok()) {
    DWORD pFunc=relaydll.GetProcAddress32W("GetNumRelays");
    if(pFunc) {
      ret=relaydll.CallProcEx32W( 1|CPEX_DEST_STDCALL, 0x01, pFunc, (DWORD)strModule);
    }
  }
  return ret;
}

#else // __WIN32__

#define PROC(name) name(hInstance,#name)

RelayDLL::RelayDLL(const TCHAR *path)
 : DProcModule(path)
, PROC(ModuleRelayOn)
, PROC(ModuleRelayOff)
, PROC(ModuleRelayMask)
, PROC(ModuleAllRelayOn)
, PROC(ModuleAllRelayOff)
, PROC(ModuleRelayStates)
, PROC(IsModuleRelayOn)
, PROC(GetNumberOfModules)
, PROC(GetSerialNumOfModule)
, PROC(ModuleFlashLED)
, PROC(GetNumRelays)
{
  if(!ModuleRelayOn.IsValid()) MessageBox(0,path,"Error loading", MB_ICONSTOP|MB_OK);
}

RelayDLL RelayDLL(relaydllname);

#endif // __WIN32__
#endif // JSB218
