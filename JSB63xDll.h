//*****************************************************************************
//*****************************************************************************
// function call prototypes for JSB63x dll
//
//      The library file outputDll.lib must be linked with your apllication code
//


//*****************************************************************************
//*****************************************************************************
//      return codes for function calls
//
// the dll uses stdcall convention
#define LIBCALL	__stdcall

//*****************************************************************************
//*****************************************************************************
//  Library Functions
//
__declspec(dllimport) int LIBCALL GetNumberOfModules();
__declspec(dllimport) BOOL LIBCALL GetSerialNumOfModule(int nModule,LPSTR strReturn);
__declspec(dllexport) int LIBCALL GetLastErrorNumber();
__declspec(dllexport) BOOL LIBCALL GetLastErrorString(LPSTR strReturn);
__declspec(dllexport) BOOL LIBCALL GetDllVersion(LPSTR strReturn);
__declspec(dllexport) BOOL LIBCALL GetDriverVersion(LPCSTR strSerialNum,LPSTR strReturn);
__declspec(dllexport) BOOL LIBCALL GetFirmwareVersion(LPCSTR strSerialNum,LPSTR strReturn);
__declspec(dllimport) BOOL LIBCALL ReadChannelRaw(LPCSTR strModule,char nChannel,short * nValue);
__declspec(dllimport) BOOL LIBCALL ReadChannelVolt(LPCSTR strModule,char nChannel,double * nValue);
__declspec(dllimport) BOOL LIBCALL ModuleFlashLed(LPCSTR strModule);

//*****************************************************************************
//*****************************************************************************
//*****************************************************************************
//*****************************************************************************
// end of file outputdll.h
//














