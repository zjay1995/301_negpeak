#pragma hdrstop

#include "internal.h"

#if SEAIO()

#include "MySeaIO.h"

#ifndef __WIN32__

#include "wow.h"

class SEAIODLL : public WOWDLL {
 public:
  SEAIODLL();
  ~SEAIODLL();


  BOOL OK() { return bOK; }
  BOOL bOK;

  DWORD SeaIo_OpenDevice;
  DWORD SeaIo_CloseDevice;
  DWORD SeaIo_WriteBit;
  DWORD SeaIo_ReadBit;
  DWORD SeaIo_WriteByte; // added. P.P. 07/27/04

// OBSOLETE
  DWORD SeaIo_RelayClose;
  DWORD SeaIo_RelayOpen;
};

SEAIODLL::SEAIODLL()
 : WOWDLL("SEAIO32.dll")
{
  bOK = WOWDLL::Ok()

    && (SeaIo_OpenDevice=GetProcAddress32W("SeaIo_OpenDevice"))!=NULL
    && (SeaIo_CloseDevice=GetProcAddress32W("SeaIo_CloseDevice"))!=NULL
    && (SeaIo_WriteBit=GetProcAddress32W("SeaIo_WriteBit"))!=NULL
    && (SeaIo_ReadBit=GetProcAddress32W("SeaIo_ReadBit"))!=NULL
    && (SeaIo_WriteByte=GetProcAddress32W("SeaIo_WriteByte"))!=NULL  // added. P.P. 07/27/04
    ;
  if(!bOK) {
    #if !DEMO()
    MessageBox(NULL,"SEAIO32.dll not found", "wPeak", MB_OK);
    #endif
  }
}

SEAIODLL::~SEAIODLL()
{
}


SEAIODLL SEAIO_Dll;



unsigned long My_SeaIo_OpenDevice(unsigned short wPortNum, HANDLE32 FAR *pHDevice)

{
  *pHDevice=0;
  unsigned long ret=(unsigned long )-1;
  if(SEAIO_Dll.bOK) {
    ret=SEAIO_Dll.CallProcEx32W( 2|CPEX_DEST_STDCALL, 0x02, SEAIO_Dll.SeaIo_OpenDevice, (DWORD)wPortNum, (DWORD)pHDevice);
  }
  return ret;
}
unsigned long My_SeaIo_CloseDevice(HANDLE32 hDevice)
{
  unsigned long ret=(unsigned long )-1;
  if(SEAIO_Dll.bOK) {
    ret=SEAIO_Dll.CallProcEx32W( 1|CPEX_DEST_STDCALL, 0x00, SEAIO_Dll.SeaIo_CloseDevice, (DWORD)hDevice);
  }
  return ret;
}

// Write a single byte to a single port on a device.
// Returns SEAIO_ERROR_NONE on success; error code on failure.
unsigned long My_SeaIo_WriteBit(HANDLE32 hDevice,
				      unsigned long BitNumber,
				      const BOOLEAN State,
				      enum AccessMode Mode)
{
  unsigned long ret=(unsigned long )-1;
  if(SEAIO_Dll.bOK) {
    ret=SEAIO_Dll.CallProcEx32W( 4|CPEX_DEST_STDCALL, 0x00, SEAIO_Dll.SeaIo_WriteBit, (DWORD)hDevice, (DWORD)BitNumber, (DWORD)State, (DWORD)Mode);
  }
  return ret;
}

// Read the state of a single bit / relay on a device
// Returns SEAIO_ERROR_NONE on success; error code on failure.
unsigned long My_SeaIo_ReadBit(HANDLE32 hDevice,
			       unsigned long BitNumber,
			       BOOLEAN FAR *BitState,
			       enum AccessMode Mode)
{
  unsigned long ret=(unsigned long )-1;
  if(SEAIO_Dll.bOK) {
    ret=SEAIO_Dll.CallProcEx32W( 4|CPEX_DEST_STDCALL, 0x04, SEAIO_Dll.SeaIo_ReadBit, (DWORD)hDevice, (DWORD)BitNumber, (DWORD)BitState, (DWORD)Mode);
  }
  return ret;
}

// added. P.P. 07/27/04
// Write a single byte to a single port on a device.
// Returns SEAIO_ERROR_NONE on success; error code on failure.
unsigned long My_SeaIo_WriteByte(HANDLE32 hDevice,
				       unsigned long Port,
                                       const unsigned char Data, 
                                       enum AccessMode Mode)
{
  unsigned long ret=(unsigned long )-1;
  if(SEAIO_Dll.bOK) {
    ret=SEAIO_Dll.CallProcEx32W( 4|CPEX_DEST_STDCALL, 0x00, SEAIO_Dll.SeaIo_WriteByte, (DWORD)hDevice, (DWORD)Port, (DWORD)Data, (DWORD)Mode);
  }
  return ret;
}
// end of added. P.P. 07/27/04

// OBSOLETE
unsigned long My_SeaIo_RelayClose(HANDLE32 hDevice, unsigned char uRelayNum)
{
  unsigned long ret=(unsigned long )-1;
  if(SEAIO_Dll.bOK) {
    ret=SEAIO_Dll.CallProcEx32W( 2|CPEX_DEST_STDCALL, 0x04, SEAIO_Dll.SeaIo_RelayClose, (DWORD)hDevice, (DWORD)uRelayNum);
  }
  return ret;
}

// OBSOLETE
unsigned long My_SeaIo_RelayOpen(HANDLE32 hDevice, unsigned char uRelayNum)
{
  unsigned long ret=(unsigned long )-1;
  if(SEAIO_Dll.bOK) {
    ret=SEAIO_Dll.CallProcEx32W( 2|CPEX_DEST_STDCALL, 0x04, SEAIO_Dll.SeaIo_RelayOpen, (DWORD)hDevice, (DWORD)uRelayNum);
  }
  return ret;
}


#else // __WIN32__

#endif





BOOL GetSeaIoDevice(unsigned short *wPortNum)
{
  for(*wPortNum=0; *wPortNum<8; (*wPortNum)++) {
    HANDLE32 HDevice;
    unsigned long errCode=My_SeaIo_OpenDevice(*wPortNum, &HDevice);
    if(!errCode) {
      // something found
      My_SeaIo_CloseDevice(HDevice); // close it
      return TRUE;
    }
  }
  return FALSE;
}



enum SEAIO_ERROR_CODES {
  SEAIO_ERROR_NONE,   /* No Error */
  SEAIO_ERROR_PARAM,       /* Invalid parameter passed to driver*/
  SEAIO_ERROR_NOT_SUPPORTED,     /* Feature not supported for this driver */
  SEAIO_ERROR_PORT_NOT_OPEN,     /* Invalid port handle in DLL structures */
  SEAIO_ERROR_INVALID_NAME,      /* OS does not recognize device name */
  SEAIO_ERROR_SHARING_VIOLATION, /* Someone else has the device open */
  SEAIO_ERROR_DRIVER_TYPE,
  SEAIO_ERROR_BUFFER,
  SEAIO_ERROR_INCOMPLETE,
  SEAIO_ERROR_GENERAL,
  SEAIO_ERROR_IO_PENDING,
  SEAIO_ERROR_NO_TXBUFFER,
  SEAIO_ERROR_INVALID_DEVICE_REQUEST,   /* Not a valid operation for the target device */
  SEAIO_ERROR_INVALID_HANDLE,
  SEAIO_ERROR_CANCELED,
  SEAIO_ERROR_LAST,
  SEAIO_ERROR_NO_MEMORY,
  SEAIO_ERROR_BUSY,
  SEAIO_ERROR_INTERRUPT_BUFFER_OVERFLOW = 128 /* will be or'ed with other error codes on read */
};
void GetSeaIoErrorString(unsigned long error, char *errorstring)
{
  const char *string=0;
  switch(error) {
    case SEAIO_ERROR_NONE: string="No Error"; break;
    case SEAIO_ERROR_PARAM: string="Invalid parameter passed to driver"; break;
    case SEAIO_ERROR_NOT_SUPPORTED: string="Feature not supported for this driver"; break;
    case SEAIO_ERROR_PORT_NOT_OPEN: string="Invalid port handle in DLL structures"; break;
    case SEAIO_ERROR_INVALID_NAME: string="OS does not recognize device name"; break;
    case SEAIO_ERROR_SHARING_VIOLATION: string="Someone else has the device open"; break;
    case SEAIO_ERROR_DRIVER_TYPE: string="SEAIO_ERROR_DRIVER_TYPE"; break;
    case SEAIO_ERROR_BUFFER: string="SEAIO_ERROR_BUFFER"; break;
    case SEAIO_ERROR_INCOMPLETE: string="SEAIO_ERROR_INCOMPLETE"; break;
    case SEAIO_ERROR_GENERAL: string="SEAIO_ERROR_GENERAL"; break;
    case SEAIO_ERROR_IO_PENDING: string="SEAIO_ERROR_IO_PENDING"; break;
    case SEAIO_ERROR_NO_TXBUFFER: string="SEAIO_ERROR_NO_TXBUFFER"; break;
    case SEAIO_ERROR_INVALID_DEVICE_REQUEST: string="Not a valid operation for the target device"; break;
    case SEAIO_ERROR_INVALID_HANDLE: string="SEAIO_ERROR_INVALID_HANDLE"; break;
    case SEAIO_ERROR_CANCELED: string="SEAIO_ERROR_CANCELED"; break;
    case SEAIO_ERROR_LAST: string="SEAIO_ERROR_LAST"; break;
    case SEAIO_ERROR_NO_MEMORY: string="SEAIO_ERROR_NO_MEMORY"; break;
    case SEAIO_ERROR_BUSY: string="SEAIO_ERROR_BUSY"; break;
    //case SEAIO_ERROR_INTERRUPT_BUFFER_OVERFLOW = 128 /* will be or'ed with other error codes on read */
  }
  if(string)
    lstrcpy(errorstring,string);
  else
    wsprintf(errorstring,"Error %ld", error);
}



#endif
