//*****************************************************************************
//*****************************************************************************
// function call prototypes for JSB-45x I/O module dll
//
//      The library file 45xDll.lib must be linked with your apllication code
//


//*****************************************************************************
//*****************************************************************************
//      return codes for function calls
//
#define JSB_NO_ERROR                      1         // function completed ok
#define JSB_ERR_MODULE_NOT_FOUND        -10         // could not find the module
#define JSB_ERR_NUM_NOT_VALID     		  -20         // output number not valid
#define JSB_ERR_COMM_FAIL               -30         // communication error with USB Driver


//*****************************************************************************
//*****************************************************************************
// calls to the optodll must always use the stdcall convention.
// 
#define LIBCALL	__stdcall


//*****************************************************************************
//*****************************************************************************
//  Library Functions
//


__declspec(dllexport) short LIBCALL GetNumberOfModules() 

__declspec(dllexport) void LIBCALL GetSerialNumber(int nModuleIndex,LPSTR strReturn) 

__declspec(dllexport) short LIBCALL GetUnitType(LPCSTR strSerialNumber) 

__declspec(dllexport) short LIBCALL FlashModuleLed(LPCSTR strSerialNumber) 

__declspec(dllexport) BOOL LIBCALL IsOutputOn(LPCTSTR strSerialNumber, short nOutputNumber) 

__declspec(dllexport) short LIBCALL ReadOutputPort(LPCTSTR strSerialNumber, short nPortNumber) 

__declspec(dllexport) long LIBCALL ReadOutputBank(LPCTSTR strSerialNumber, short nBankNumber) 

__declspec(dllexport) short LIBCALL SetAllOutputs(LPCTSTR strSerialNumber, BOOL bOn) 

__declspec(dllexport) short LIBCALL SetOutput(LPCTSTR strSerialNumber, short nOutputNumber, BOOL bOn) 

__declspec(dllexport) short LIBCALL SetOutputPort(LPCTSTR strSerialNumber, short nPortNumber, short nValue) 

__declspec(dllexport) short LIBCALL SetOutputBank(LPCTSTR strSerialNumber, short nBankNumber, long nValue) 

__declspec(dllexport) BOOL LIBCALL IsInputOn(LPCTSTR strSerialNumber, short nInputNumber) 

__declspec(dllexport) short LIBCALL SetDebounceValue(LPCTSTR strSerialNumber, short nValue) 

__declspec(dllexport) long LIBCALL ReadInputBank(LPCTSTR strSerialNumber, short nBankNumber) 

__declspec(dllexport) short LIBCALL CloseModule()

__declspec(dllexport) short LIBCALL ReadInputPort(LPCTSTR strSerialNumber, short nPortNumber) 



//*****************************************************************************
//*****************************************************************************
//*****************************************************************************
//*****************************************************************************
// end of file 45xdll.h
//

