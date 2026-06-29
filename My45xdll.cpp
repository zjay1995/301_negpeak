#pragma hdrstop

#include "mainwind.h"

//#if INTERNAL() && ((!DDA())&&(!DAQ2000())) //hence, USB digital board
#if INTERNAL() && !(DDA()||DAQ2000()||PMD_12LS()||PMD_12FS()||PMD_16FS()) //hence, JSB450 USB digital board

#include "my45xdll.h"

static const char far _45dllname[]="45xDLL.dll";

#ifndef __WIN32__

#include "wow.h"

class _45DLL : public WOWDLL {
 public:
  _45DLL();
};

_45DLL::_45DLL()
 : WOWDLL(_45dllname)
{
  if(!Ok()) {
    #if !DEMO()
    MessageBox(NULL,"45xDLL.dll not found", "wPeak", MB_OK);
    #endif
  }
}

static _45DLL _45dll;

short MyGetNumberOfModules(void)
{
  short ret=0;
  if(_45dll.Ok()) {
    DWORD pGetNumberOfModules=_45dll.GetProcAddress32W("GetNumberOfModules");
    if(pGetNumberOfModules) {
      ret=(short)_45dll.CallProcEx32W( 0|CPEX_DEST_STDCALL, 0x00, pGetNumberOfModules );
    }
  }
  return ret;
}
void MyGetSerialNumber(LONG nModuleIndex,LPSTR strReturn)
{
  *strReturn='\0';
  if(_45dll.Ok()) {
    DWORD pGetSerialNumber=_45dll.GetProcAddress32W("GetSerialNumber");
    if(pGetSerialNumber) {
      _45dll.CallProcEx32W( 2|CPEX_DEST_STDCALL, 0x02, pGetSerialNumber, (DWORD)nModuleIndex, (DWORD)strReturn );
    }
  }
  //_pMainWindow->print("GetSerialNumber %ld results \'%s\'\r\n", nModuleIndex, strReturn);
}
short MyGetUnitType(LPCSTR strSerialNumber)
{
  short ret=0;
  if(_45dll.Ok()) {
    DWORD pGetUnitType=_45dll.GetProcAddress32W("GetUnitType");
    if(pGetUnitType) {
      ret=_45dll.CallProcEx32W( 1|CPEX_DEST_STDCALL, 0x01, pGetUnitType, (DWORD)strSerialNumber );
    }
  }
  return ret;
}

/*
__declspec(dllexport) short LIBCALL FlashModuleLed(LPCSTR strSerialNumber)

__declspec(dllexport) BOOL LIBCALL IsOutputOn(LPCTSTR strSerialNumber, short nOutputNumber)

__declspec(dllexport) short LIBCALL ReadOutputPort(LPCTSTR strSerialNumber, short nPortNumber)

__declspec(dllexport) long LIBCALL ReadOutputBank(LPCTSTR strSerialNumber, short nBankNumber)

__declspec(dllexport) short LIBCALL SetAllOutputs(LPCTSTR strSerialNumber, BOOL bOn)
*/
short MySetOutput(LPCSTR strSerialNumber, short nOutputNumber, BOOL bOn)
{
  //_pMainWindow->print("  SetOutput(\'%s\' %d, %d)\r\n", strSerialNumber, nOutputNumber, bOn);
  short ret=0;
  if(_45dll.Ok()) {
    DWORD pSetOutput=_45dll.GetProcAddress32W("SetOutput");
    if(pSetOutput) {
      ret=_45dll.CallProcEx32W( 3|CPEX_DEST_STDCALL, 0x01, pSetOutput, (DWORD)strSerialNumber, (DWORD)nOutputNumber, (DWORD)bOn);
    }
  }
  return ret;
}
short MySetOutputPort(LPCSTR strSerialNumber, short nPortNumber, short nValue)
{
  //_pMainWindow->print("  SetOutputPort(\'%s\' %d, %d)\r\n", strSerialNumber, nPortNumber, nValue);
  short ret=0;
  if(_45dll.Ok()) {
    DWORD pSetOutputPort=_45dll.GetProcAddress32W("SetOutputPort");
    if(pSetOutputPort) {
      ret=_45dll.CallProcEx32W( 3|CPEX_DEST_STDCALL, 0x01, pSetOutputPort, (DWORD)strSerialNumber, (DWORD)nPortNumber, (DWORD)nValue);
    }
  }
  return ret;
}

/*

__declspec(dllexport) short LIBCALL SetOutputBank(LPCTSTR strSerialNumber, short nBankNumber, long nValue)
*/
BOOL MyIsInputOn(LPCSTR strSerialNumber, short nInputNumber)
{
  //_pMainWindow->print("  IsInputOn(\'%s\', %d)\r\n", strSerialNumber, nInputNumber);
  BOOL ret=0;
  if(_45dll.Ok()) {
    DWORD pIsInputOn=_45dll.GetProcAddress32W("IsInputOn");
    if(pIsInputOn) {
      ret=_45dll.CallProcEx32W( 2|CPEX_DEST_STDCALL, 0x01, pIsInputOn, (DWORD)strSerialNumber, (DWORD)nInputNumber);
    }
  }
  return ret;
}
/*
__declspec(dllexport) short LIBCALL SetDebounceValue(LPCTSTR strSerialNumber, short nValue)

__declspec(dllexport) long LIBCALL ReadInputBank(LPCTSTR strSerialNumber, short nBankNumber)

__declspec(dllexport) short LIBCALL CloseModule()
*/
short MyReadInputPort(LPCSTR strSerialNumber, short nPortNumber)
{
  //_pMainWindow->print("  ReadInputPort(\'%s\', %d)\r\n", strSerialNumber, nPortNumber);
  short ret=0;
  if(_45dll.Ok()) {
    DWORD pReadInputPort=_45dll.GetProcAddress32W("ReadInputPort");
    if(pReadInputPort) {
      ret=_45dll.CallProcEx32W( 2|CPEX_DEST_STDCALL, 0x01, pReadInputPort, (DWORD)strSerialNumber, (DWORD)nPortNumber);
    }
  }
  return ret;
}

#else // __WIN32__

#define PROC(name) name(hInstance,#name)

_45DLL::_45DLL(const TCHAR *path)
 : DProcModule(path)
, PROC(GetNumberOfModules)
, PROC(GetSerialNumber)
, PROC(GetUnitType)

, PROC(SetOutput)
, PROC(SetOutputPort)

, PROC(IsInputOn)

, PROC(ReadInputPort)

{
  if(!GetNumberOfModules.IsValid()) MessageBox(0,path,"Error loading", MB_ICONSTOP|MB_OK);
}

_45DLL _45dll(_45dllname);



#endif // __WIN32__


#endif
