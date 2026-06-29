#ifndef  _MYJSB63X_H
#define  _MYJSB63X_H


#ifndef  __WIN32__
  // from JSB63xDll.h

int JSB63x_GetNumberOfModules();
BOOL JSB63x_GetSerialNumOfModule(int nModule,LPSTR strReturn);
int JSB63x_GetLastErrorNumber();
BOOL JSB63x_GetLastErrorString(LPSTR strReturn);
BOOL JSB63x_GetDllVersion(LPSTR strReturn);
BOOL JSB63x_GetDriverVersion(LPCSTR strSerialNum,LPSTR strReturn);
BOOL JSB63x_GetFirmwareVersion(LPCSTR strSerialNum,LPSTR strReturn);
BOOL JSB63x_ReadChannelRaw(LPCSTR strModule,BYTE nChannel,short * nValue);
BOOL JSB63x_ReadChannelVolt(LPCSTR strModule,BYTE nChannel,double * nValue);
int JSB63x_ModuleFlashLED(LPCSTR strModule);

#else  //__WIN32__

//#include "JSB63xDll.h"

// use dynamic linking of DLL due to DLL function names conflict
#include "winutil/dllproc.h"

class JSB63XDLL : public DProcModule
{
public:
  JSB63XDLL(const TCHAR *path);

public:
  DModuleProc0<int> GetNumberOfModules;
  DModuleProc2<BOOL, int, LPSTR> GetSerialNumOfModule;
  DModuleProc0<int> GetLastErrorNumber;
  DModuleProc1<BOOL, LPSTR> GetLastErrorString;
  DModuleProc1<BOOL, LPSTR> GetDllVersion;
  DModuleProc2<BOOL, LPCSTR, LPSTR> GetDriverVersion;
  DModuleProc2<BOOL, LPCSTR, LPSTR> GetFirmwareVersion;
  DModuleProc3<BOOL, LPCSTR, char, short *> ReadChannelRaw;
  DModuleProc3<BOOL, LPCSTR, char, double *> ReadChannelVolt;
  DModuleProc1<BOOL, LPCSTR> ModuleFlashLed;
};

extern JSB63XDLL JSB63XDll;

#define JSB63x_GetNumberOfModules               JSB63XDll.GetNumberOfModules
#define JSB63x_GetSerialNumOfModule             JSB63XDll.GetSerialNumOfModule
#define JSB63x_GetLastErrorNumber               JSB63XDll.GetLastErrorNumber
#define JSB63x_GetLastErrorString               JSB63XDll.GetLastErrorString
#define JSB63x_GetDllVersion                    JSB63XDll.GetDllVersion
#define JSB63x_GetDriverVersion                 JSB63XDll.GetDriverVersion
#define JSB63x_GetFirmwareVersion               JSB63XDll.GetFirmwareVersion
#define JSB63x_ReadChannelRaw                   JSB63XDll.ReadChannelRaw
#define JSB63x_ReadChannelVolt                  JSB63XDll.ReadChannelVolt
#define JSB63x_ModuleFlashLED                   JSB63XDll.ModuleFlashLED


#endif //__WIN32__


#endif // _MYJSB63X_H 
