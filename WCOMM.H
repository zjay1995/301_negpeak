#ifndef  _WCOMM_H
#define  _WCOMM_H

#ifdef __TURBOC__
  #ifndef __WINDOWS_H
  #include <windows.h>
  #endif
#endif

#include "wcpplib\str.h"

class WComm {
 protected:
  Str comm_name; // Debug
  /*static*/ int ShowError(int err, LPCSTR lpszFuncName) const;
  #ifdef __WIN32__
  HANDLE h;
  #else
  int h;
  #endif
 public:
  #ifdef __WIN32__
  operator HANDLE() const { return h; }
  #else
  operator int() const { return h; }
  #endif

  WComm(LPCSTR lpszDevControl, UINT cbInQueue, UINT cbOutQueue);
  ~WComm();

  int SetParam(LPCSTR lpszDef) const;

  int Read(void FAR* lpvBuf, int cbRead) const;
  int Write(const void FAR* lpvBuf, int cbWrite) const;

  int ReadChar(void FAR* lpvBuf) const { return Read(lpvBuf, 1); }
  int WriteChar(int c)           const { return Write(&c, 1); }

  int TransmitChar(char chTransmit) const { return ::TransmitCommChar(*this, chTransmit); }
  #ifndef __WIN32__
  int UngetChar(char chUnget) const { return ::UngetCommChar(*this, chUnget); }
  #endif

  int SetBreak()   const { return ::SetCommBreak(*this); }
  int ClearBreak() const { return ::ClearCommBreak(*this); }

  int Flush(int fnQueue) const 
    { 
      #ifndef __WIN32__
      return ::FlushComm(*this,fnQueue); 
      #else
      return ::PurgeComm(*this,fnQueue? /*PURGE_RXABORT|*/PURGE_RXCLEAR: /*PURGE_TXABORT|*/PURGE_TXCLEAR);
      #endif
    }
  #ifndef __WIN32__
  UINT FAR* SetEventMask(UINT fuEvtMask) const { return ::SetCommEventMask(*this, fuEvtMask); }
  UINT GetEventMask(int fnEvtClear) const { return ::GetCommEventMask(*this, fnEvtClear); }
  #endif

  BOOL EnableNotification(HWND hwnd, int cbWriteNotify, int cbOutQueue) const
    { 
      #ifndef __WIN32__
      return ::EnableCommNotification(*this, hwnd, cbWriteNotify, cbOutQueue); 
      #else
      return TRUE; 
      #endif
    }

  int GetError(COMSTAT FAR* lpStat=NULL) const 
    { 
      #ifndef __WIN32__
      return ::GetCommError(*this, lpStat); 
      #else
      DWORD dwErrors=0;
      BOOL ret=::ClearCommError(*this, &dwErrors, lpStat);
      if(!ret) return GetLastError();
      return dwErrors;
      #endif      
    }
};

#endif

