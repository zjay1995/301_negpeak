#include "includes.h"
#pragma hdrstop

#include "wow.h"
WOW::WOW()
 : hKernel(LoadLibrary("KERNEL"))
 , LoadLibraryEx32W((PFNLOADLIBRARYEX32W)::GetProcAddress(hKernel,"LoadLibraryEx32W"))
 , GetProcAddress32W((PFNGETPROCADDRESS32W)::GetProcAddress(hKernel,"GetProcAddress32W"))
 , FreeLibrary32W((PFNFREELIBRARY32W)::GetProcAddress(hKernel,"FreeLibrary32W"))
 , CallProcEx32W((PFNCALLPROCEX32W)::GetProcAddress(hKernel,"_CALLPROCEX32W"))
{
}
WOW::~WOW()
{
  FreeLibrary(hKernel);
}
BOOL WOW::Ok() const
{
  return LoadLibraryEx32W && GetProcAddress32W && FreeLibrary32W && CallProcEx32W;
}

////////////////////
WOWDLL::WOWDLL(const char *path)
 : hDLL(LoadLibraryEx32W? LoadLibraryEx32W(path, 0, 0): 0)
{
}
WOWDLL::~WOWDLL()
{
  if(FreeLibrary32W) FreeLibrary32W(hDLL);
}
BOOL WOWDLL::Ok() const
{
  return WOW::Ok() && hDLL;
}
