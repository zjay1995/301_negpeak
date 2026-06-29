#pragma hdrstop

#include "internal.h"

#if JSB63x()

#include "myJSB63x.H"

static const char far JSB63xDllname[]="JSB63xDll.dll";

#ifndef __WIN32__

#include "wow.h"

class JSB63XDLL: public WOWDLL
{
 public:
  JSB63XDLL();
};
JSB63XDLL::JSB63XDLL()
 : WOWDLL(JSB63xDllname)
{
  if(!Ok()) {
    #if !DEMO()
    MessageBox(NULL,"JSB63xDll.dll not found", "wPeak", MB_OK);
    #endif
  }
}

static JSB63XDLL JSB63xdll;

int JSB63x_GetNumberOfModules()
{
  //_pMainWindow->print("  JSB63x_GetNumberOfModules()\r\n");
  int ret=0;
  if(JSB63xdll.Ok()) {
    DWORD pFunc=JSB63xdll.GetProcAddress32W("GetNumberOfModules");
    if(pFunc) {
      ret=JSB63xdll.CallProcEx32W( 0|CPEX_DEST_STDCALL, 0x00, pFunc);
    }
  }
  return ret;
}
BOOL JSB63x_GetSerialNumOfModule(int nModule,LPSTR strReturn)
{
  BOOL ret=0;
  //_pMainWindow->print("  GetSerialNumOfModule(%d)\r\n", nModule);
  if(JSB63xdll.Ok()) {
    DWORD pFunc=JSB63xdll.GetProcAddress32W("GetSerialNumOfModule");
    if(pFunc) {
      ret=JSB63xdll.CallProcEx32W( 2|CPEX_DEST_STDCALL, 0x02, pFunc, (DWORD)nModule, (DWORD)strReturn);
    }
  }
  //_pMainWindow->print("JSB63x_GetSerialNumOfModule(%d) results \'%s\'\r\n", nModule, strReturn);
  return ret;
}
int JSB63x_GetLastErrorNumber()
{
  //_pMainWindow->print("  JSB63x_GetLastErrorNumber()\r\n");
  int ret=0;
  if(JSB63xdll.Ok()) {
    DWORD pFunc=JSB63xdll.GetProcAddress32W("GetLastErrorNumber");
    if(pFunc) {
      ret=JSB63xdll.CallProcEx32W( 0|CPEX_DEST_STDCALL, 0x00, pFunc);
    }
  }
  return ret;
}
BOOL JSB63x_GetLastErrorString(LPSTR strReturn)
{
  BOOL ret=0;
  if(JSB63xdll.Ok()) {
    DWORD pFunc=JSB63xdll.GetProcAddress32W("GetLastErrorString");
    if(pFunc) {
      ret=JSB63xdll.CallProcEx32W( 1|CPEX_DEST_STDCALL, 0x01, pFunc, (DWORD)strReturn);
    }
  }
  //_pMainWindow->print("  JSB63x_GetLastErrorString results \'%s\'\r\n", strReturn);
  return ret;
}
BOOL JSB63x_GetDllVersion(LPSTR strReturn)
{
  BOOL ret=0;
  if(JSB63xdll.Ok()) {
    DWORD pFunc=JSB63xdll.GetProcAddress32W("GetDllVersion");
    if(pFunc) {
      ret=JSB63xdll.CallProcEx32W( 1|CPEX_DEST_STDCALL, 0x01, pFunc, (DWORD)strReturn);
    }
  }
  //_pMainWindow->print("  JSB63x_GetDllVersion results \'%s\'\r\n", strReturn);
  return ret;
}
BOOL JSB63x_GetDriverVersion(LPCSTR strSerialNum,LPSTR strReturn)
{
  BOOL ret=0;
  if(JSB63xdll.Ok()) {
    DWORD pFunc=JSB63xdll.GetProcAddress32W("GetDriverVersion");
    if(pFunc) {
      ret=JSB63xdll.CallProcEx32W( 2|CPEX_DEST_STDCALL, 0x01|0x02, pFunc, (DWORD)strSerialNum, (DWORD)strReturn);
    }
  }
  //_pMainWindow->print("  JSB63x_GetDriverVersion(\'%s\') results \'%s\'\r\n", strSerialNum,strReturn);
  return ret;
}
BOOL JSB63x_GetFirmwareVersion(LPCSTR strSerialNum,LPSTR strReturn)
{
  BOOL ret=0;
  if(JSB63xdll.Ok()) {
    DWORD pFunc=JSB63xdll.GetProcAddress32W("GetFirmwareVersion");
    if(pFunc) {
      ret=JSB63xdll.CallProcEx32W( 2|CPEX_DEST_STDCALL, 0x01|0x02, pFunc, (DWORD)strSerialNum, (DWORD)strReturn);
    }
  }
  //_pMainWindow->print("  JSB63x_GetFirmwareVersion(\'%s\') results \'%s\'\r\n", strSerialNum,strReturn);
  return ret;
}
BOOL JSB63x_ReadChannelRaw(LPCSTR strModule,BYTE nChannel,short * nValue)
{
  BOOL ret=0;
  if(JSB63xdll.Ok()) {
//    DWORD pFunc=JSB63xdll.GetProcAddress32W("ReadValueRaw"); // replaced. P.P. 05/30/02
    DWORD pFunc=JSB63xdll.GetProcAddress32W("ReadChannelRaw"); // as in Manual
    if(pFunc) {
      ret=JSB63xdll.CallProcEx32W( 3|CPEX_DEST_STDCALL, 0x01|0x04, pFunc, (DWORD)strModule,(DWORD)nChannel,(DWORD)nValue);
    }
  }
  //_pMainWindow->print("  JSB63x_ReadValueRaw(\'%s\',%d) results \'%d\'\r\n", strModule,nChannel,*nValue));
  return ret;
}
BOOL JSB63x_ReadChannelVolt(LPCSTR strModule,BYTE nChannel,double * nValue)
{
  BOOL ret=0;
  if(JSB63xdll.Ok()) {
//    DWORD pFunc=JSB63xdll.GetProcAddress32W("ReadValueVolt");
    DWORD pFunc=JSB63xdll.GetProcAddress32W("ReadChannelVolt");  // P.P. 05/30/02
    if(pFunc) {
      ret=JSB63xdll.CallProcEx32W( 3|CPEX_DEST_STDCALL, 0x01|0x04, pFunc, (DWORD)strModule,(DWORD)nChannel,(DWORD)nValue);
    }
  }
  //_pMainWindow->print("  JSB63x_ReadValueRaw(\'%s\',%d) results \'%g\'\r\n", strModule,nChannel,*nValue));
  return ret;
}

int JSB63x_ModuleFlashLED(LPCSTR strModule)
{
  //_pMainWindow->print("  JSB63x_ModuleFlashLED(\'%s\')\r\n", strModule);
  BOOL ret=0;
  if(JSB63xdll.Ok()) {
    DWORD pFunc=JSB63xdll.GetProcAddress32W("ModuleFlashLED");
    if(pFunc) {
      ret=JSB63xdll.CallProcEx32W( 1|CPEX_DEST_STDCALL, 0x01, pFunc, (DWORD)strModule);
    }
  }
  return ret;
}

#else // __WIN32__

#define PROC(name) name(hInstance,#name)

JSB63XDLL::JSB63XDLL(const TCHAR *path)
 : DProcModule(path)
 , PROC(GetNumberOfModules)
 , PROC(GetSerialNumOfModule)
 , PROC(GetLastErrorNumber)
 , PROC(GetLastErrorString)
 , PROC(GetDllVersion)
 , PROC(GetDriverVersion)
 , PROC(GetFirmwareVersion)
 , PROC(ReadChannelRaw)
 , PROC(ReadChannelVolt)
 , PROC(ModuleFlashLed)
{
  if(!ReadChannelRaw.IsValid()) MessageBox(0,path,"Error loading", MB_ICONSTOP|MB_OK);
}

JSB63XDLL JSB63xdll(JSB63xDllname);

#endif // __WIN32__


#endif // JSB63x()
