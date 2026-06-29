#pragma hdrstop

#ifdef __TURBOC__
#ifndef __OWL_H
#include <owl.h>	// GetApplicationObject()->hInstance
#endif
#else
#include <windows.h>
#endif

#include "wcomm.h"

int WComm::Read(void FAR* lpvBuf, int cbRead) const 
{ 
  #ifdef __WIN32__
  COMSTAT status;
  {
    DWORD commerror;
    if(!::ClearCommError(*this, &commerror, &status))
      return -1;//GetLastError();
  }   
  if(status.cbInQue==0) return 0;
  if((unsigned)cbRead>status.cbInQue) cbRead=status.cbInQue;

  DWORD numberOfBytesRead;
  if(::ReadFile(*this, lpvBuf, cbRead, &numberOfBytesRead, NULL)) {
    return numberOfBytesRead;
  }
  else {
    return -1;//numberOfBytesRead        
  }
  #else
  return ::ReadComm(*this, lpvBuf, cbRead); 
  #endif
}

int WComm::Write(const void FAR* lpvBuf, int cbWrite) const 
{ 
  #ifdef __WIN32__
  /*
  DWORD dwMask=0;
  GetCommMask(*this,&dwMask);
  if(!(dwMask&(EV_RXCHAR|EV_BREAK))) 
    return 0;
  */
  DWORD numberOfBytesWritten;
  if(::WriteFile(*this, lpvBuf, cbWrite, &numberOfBytesWritten, NULL)) {
    return numberOfBytesWritten;
  }
  else {
    return -1;//numberOfBytesWritten
  }
  #else
  return ::WriteComm(*this, lpvBuf, cbWrite); 
  #endif
}


int WComm::ShowError(int err, LPCSTR lpszFuncName) const
{
  MessageBeep(MB_ICONHAND);
  char errorMessage[80];
  #ifdef __TURBOC__
  if(!LoadString(GetApplicationObject()->hInstance,err,errorMessage,sizeof(errorMessage))) 
  #else
  if(0==FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_MAX_WIDTH_MASK, NULL, err, 0, errorMessage, sizeof(errorMessage), NULL)) 
  #endif
  {
    wsprintf(errorMessage,"Error #%d",err);
  }
  char str[120];
  wsprintf(str,"%s\nin function '%s'\non device '%s'",errorMessage,lpszFuncName, comm_name.Get());
  MessageBox(NULL,str,"Communication Error", MB_OK|MB_ICONHAND|MB_TASKMODAL);
  return err;
}

WComm::WComm(LPCSTR lpszDevControl, UINT cbInQueue, UINT cbOutQueue)
  #ifndef __WIN32__
  : h(OpenComm(lpszDevControl, cbInQueue, cbOutQueue))
  #else
  : h(CreateFile(lpszDevControl, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0,0))
  #endif
  , comm_name(lpszDevControl)
{
  #ifndef __WIN32__
  if(h<0) {
    ShowError(h, "OpenComm");
    h=-1;
  }
  #else
  if(h==INVALID_HANDLE_VALUE) {
    ShowError(GetLastError(), "OpenComm");
  }
  else {
    SetupComm(*this,cbInQueue,cbOutQueue);
  }
  #endif
}

WComm::~WComm()
{
  #ifndef __WIN32__
  if(h<0) return;
  int err=CloseComm(*this);
  if(err<0) ShowError(err, "CloseComm");
  #else
  if(h==INVALID_HANDLE_VALUE) return;
  if(!CloseHandle(*this)) {
     ShowError(GetLastError(), "CloseComm");
  }
  #endif
}

int WComm::SetParam(LPCSTR lpszDef) const // "COM1:9600,n,8,1"
{
  DCB dcb;
  #ifndef __WIN32__
  int err;
  err = ::BuildCommDCB(lpszDef, &dcb);
  if(err<0) return ShowError(err, "BuildCommDCB");
  err = ::SetCommState(&dcb);
  if(err<0) return ShowError(err, "SetCommState");
  return err;
  #else
  ZeroMemory(&dcb,sizeof(dcb));
  dcb.DCBlength=sizeof(dcb);           
  if(!BuildCommDCB(lpszDef, &dcb))
    return ShowError(GetLastError(), "BuildCommDCB");
  if(!::SetCommState(*this,&dcb)) 
    return ShowError(GetLastError(), "SetCommState");
  return 0; 
  #endif
}





