#ifndef _MY45XDLL_H
#define _MY45XDLL_H



#ifndef __WIN32__

//#include "45xdll.h"


short MyGetNumberOfModules(void);
void MyGetSerialNumber(LONG nModuleIndex,LPSTR strReturn);
short MyGetUnitType(LPCSTR strSerialNumber);

/*
__declspec(dllexport) short LIBCALL FlashModuleLed(LPCSTR strSerialNumber)

__declspec(dllexport) BOOL LIBCALL IsOutputOn(LPCTSTR strSerialNumber, short nOutputNumber)

__declspec(dllexport) short LIBCALL ReadOutputPort(LPCTSTR strSerialNumber, short nPortNumber)

__declspec(dllexport) long LIBCALL ReadOutputBank(LPCTSTR strSerialNumber, short nBankNumber)

__declspec(dllexport) short LIBCALL SetAllOutputs(LPCTSTR strSerialNumber, BOOL bOn)
*/
short MySetOutput(LPCSTR strSerialNumber, short nOutputNumber, BOOL bOn);
short MySetOutputPort(LPCSTR strSerialNumber, short nPortNumber, short nValue);

/*
__declspec(dllexport) short LIBCALL SetOutputBank(LPCTSTR strSerialNumber, short nBankNumber, long nValue)
*/
BOOL MyIsInputOn(LPCSTR strSerialNumber, short nInputNumber);
/*
__declspec(dllexport) short LIBCALL SetDebounceValue(LPCTSTR strSerialNumber, short nValue)

__declspec(dllexport) long LIBCALL ReadInputBank(LPCTSTR strSerialNumber, short nBankNumber)

__declspec(dllexport) short LIBCALL CloseModule()
*/
short MyReadInputPort(LPCSTR strSerialNumber, short nPortNumber);
/// end



#else // __WIN32__


//#include "45xdll.h"


// use dynamic linking of DLL due to DLL function names conflict
#include "winutil/dllproc.h"

class _45DLL : public DProcModule
{
public:
  _45DLL(const TCHAR *path);

public:
  DModuleProc0<short> GetNumberOfModules;
  DModuleProcV2<LONG, LPSTR> GetSerialNumber;
  DModuleProc1<short, LPCSTR> GetUnitType;

  DModuleProc3<short, LPCSTR, short, BOOL> SetOutput;
  DModuleProc3<short, LPCSTR, short, short> SetOutputPort;

  DModuleProc2<BOOL, LPCSTR, short> IsInputOn;

  DModuleProc2<short, LPCSTR, short> ReadInputPort;
};

extern _45DLL _45dll;



#define MyGetNumberOfModules            _45dll.GetNumberOfModules
#define MyGetSerialNumber               _45dll.GetSerialNumber
#define MyGetUnitType                   _45dll.GetUnitType

/*
__declspec(dllexport) short LIBCALL FlashModuleLed(LPCSTR strSerialNumber)

__declspec(dllexport) BOOL LIBCALL IsOutputOn(LPCTSTR strSerialNumber, short nOutputNumber)

__declspec(dllexport) short LIBCALL ReadOutputPort(LPCTSTR strSerialNumber, short nPortNumber)

__declspec(dllexport) long LIBCALL ReadOutputBank(LPCTSTR strSerialNumber, short nBankNumber)

__declspec(dllexport) short LIBCALL SetAllOutputs(LPCTSTR strSerialNumber, BOOL bOn)
*/
#define MySetOutput                     _45dll.SetOutput
#define MySetOutputPort                 _45dll.SetOutputPort

/*
__declspec(dllexport) short LIBCALL SetOutputBank(LPCTSTR strSerialNumber, short nBankNumber, long nValue)
*/
#define MyIsInputOn                     _45dll.IsInputOn
/*
__declspec(dllexport) short LIBCALL SetDebounceValue(LPCTSTR strSerialNumber, short nValue)

__declspec(dllexport) long LIBCALL ReadInputBank(LPCTSTR strSerialNumber, short nBankNumber)

__declspec(dllexport) short LIBCALL CloseModule()
*/
#define MyReadInputPort                 _45dll.ReadInputPort
/// end


#endif  // __WIN32__




#endif // _MY45XDLL_H