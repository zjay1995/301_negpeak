#ifndef  _WOW_H
#define  _WOW_H


//#include "wownt16.h"
#ifndef CPEX_DEST_STDCALL
  #define CPEX_DEST_STDCALL   0x00000000L
  #define CPEX_DEST_CDECL     0x80000000L
#endif

#pragma hdrstop

class WOW {
 public:
  WOW();
  ~WOW();
  BOOL Ok() const;

  HINSTANCE hKernel;
  typedef DWORD (WINAPI *PFNLOADLIBRARYEX32W)(LPCSTR lpszLibFile, DWORD hFile, DWORD dwFlags);
  PFNLOADLIBRARYEX32W LoadLibraryEx32W;
  typedef DWORD (WINAPI *PFNGETPROCADDRESS32W)(DWORD hModule, LPCSTR lpszProc);
  PFNGETPROCADDRESS32W GetProcAddress32W;
  typedef DWORD (WINAPI *PFNFREELIBRARY32W)(DWORD hLibModule);
  PFNFREELIBRARY32W FreeLibrary32W;
  typedef DWORD (FAR *PFNCALLPROCEX32W)(
    DWORD nParams,          // number of parameters passed
    DWORD fAddressConvert,  // bit mask
    DWORD lpProcAddress,    // the DLL function to be called
    //DWORD param1            // parameter for DLL function
    ...                     // up to 32 function parameters
  ); // cdecl!!!
  PFNCALLPROCEX32W CallProcEx32W;
};

class WOWDLL : public WOW
{
 public:
  WOWDLL(const char *path);
  ~WOWDLL();

  DWORD GetProcAddress32W(LPCSTR lpszProc) { return WOW::GetProcAddress32W(hDLL, lpszProc); }

  BOOL Ok() const;

  DWORD hDLL;
};


#endif

