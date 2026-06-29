#pragma hdrstop

#include "internal.h" 

#if DAQ2000()

#include "MyDAQX.h"

#ifndef __WIN32__

#include "wow.h"
class DAQXDLL : public WOWDLL {
 public:
  DAQXDLL(const char *path);
  ~DAQXDLL();


  BOOL OK() { return bOK; }
  BOOL bOK;

  DWORD daqOpen;
  DWORD daqClose;
  DWORD daqOnline;

  DWORD daqProcessError;
  DWORD daqGetLastError;
  DWORD daqDefaultErrorHandler;
  DWORD daqFormatError;


  DWORD daqGetDeviceCount;
  DWORD daqGetDeviceList;
  DWORD daqGetDeviceProperties;


  DWORD daqAdcExpSetBank;
  DWORD daqAdcRd;
  DWORD daqAdcRdScan;
  DWORD daqAdcRdN;
  DWORD daqAdcRdScanN;


  DWORD daqDacSetOutputMode;

  DWORD daqDacWt;
  DWORD daqDacWtMany;



  DWORD daqIOReadBit;
  DWORD daqIORead;
  DWORD daqIOWriteBit;
  DWORD daqIOWrite;
  DWORD daqIOGet8255Conf;
};


DAQXDLL::DAQXDLL(const char *path)
 : WOWDLL(path)
{
  bOK = WOWDLL::Ok()
    && (daqOpen=GetProcAddress32W("daqOpen"))!=NULL
    && (daqClose=GetProcAddress32W("daqClose"))!=NULL
    && (daqOnline=GetProcAddress32W("daqOnline"))!=NULL
    && (daqProcessError=GetProcAddress32W("daqProcessError"))!=NULL
    && (daqGetLastError=GetProcAddress32W("daqGetLastError"))!=NULL
    && (daqDefaultErrorHandler=GetProcAddress32W("daqDefaultErrorHandler"))!=NULL
    && (daqFormatError=GetProcAddress32W("daqFormatError"))!=NULL
    && (daqGetDeviceCount=GetProcAddress32W("daqGetDeviceCount"))!=NULL
    && (daqGetDeviceList=GetProcAddress32W("daqGetDeviceList"))!=NULL
    && (daqGetDeviceProperties=GetProcAddress32W("daqGetDeviceProperties"))!=NULL
    && (daqAdcExpSetBank=GetProcAddress32W("daqAdcExpSetBank"))!=NULL
    && (daqAdcRd=GetProcAddress32W("daqAdcRd"))!=NULL
    && (daqAdcRdScan=GetProcAddress32W("daqAdcRdScan"))!=NULL
    && (daqAdcRdN=GetProcAddress32W("daqAdcRdN"))!=NULL
    && (daqAdcRdScanN=GetProcAddress32W("daqAdcRdScanN"))!=NULL
    && (daqDacSetOutputMode=GetProcAddress32W("daqDacSetOutputMode"))!=NULL
    && (daqDacWt=GetProcAddress32W("daqDacWt"))!=NULL
    && (daqDacWtMany=GetProcAddress32W("daqDacWtMany"))!=NULL
    && (daqIOReadBit=GetProcAddress32W("daqIOReadBit"))!=NULL
    && (daqIORead=GetProcAddress32W("daqIORead"))!=NULL
    && (daqIOWriteBit=GetProcAddress32W("daqIOWriteBit"))!=NULL
    && (daqIOWrite=GetProcAddress32W("daqIOWrite"))!=NULL
    && (daqIOGet8255Conf=GetProcAddress32W("daqIOGet8255Conf"))!=NULL
    ;
  if(!bOK) {
    #if !DEMO()
    MessageBox(NULL,"DAQX.dll not found", "wPeak", MB_OK);
    #endif
  }
}

DAQXDLL::~DAQXDLL()
{
}

static DAQXDLL DAQX_Dll("DAQX.dll");



DaqHandleT My_daqOpen(LPCSTR daqName)
{
  DaqHandleT ret=0;
  if(DAQX_Dll.bOK) {
    ret=DAQX_Dll.CallProcEx32W( 1|CPEX_DEST_STDCALL, 0x01, DAQX_Dll.daqOpen, (DWORD)daqName);
  }
  return ret;
}
DaqError My_daqClose(DaqHandleT handle)
{
  DaqError ret=(DaqError)-1;
  if(DAQX_Dll.bOK) {
    ret=(DaqError)DAQX_Dll.CallProcEx32W( 1|CPEX_DEST_STDCALL, 0x00, DAQX_Dll.daqClose, (DWORD)handle);
  }
  return ret;
}
DaqError My_daqOnline(DaqHandleT handle, PBOOL *online)
{
  DaqError ret=(DaqError)-1;
  if(DAQX_Dll.bOK) {
    ret=(DaqError)DAQX_Dll.CallProcEx32W( 2|CPEX_DEST_STDCALL, 0x02, DAQX_Dll.daqOnline, (DWORD)handle, (DWORD)online);
  }
  return ret;
}


DaqError My_daqProcessError(DaqHandleT handle, DaqError errCode)
{
  DaqError ret=(DaqError)-1;
  if(DAQX_Dll.bOK) {
    ret=(DaqError)DAQX_Dll.CallProcEx32W( 2|CPEX_DEST_STDCALL, 0x00, DAQX_Dll.daqProcessError, (DWORD)handle, (DWORD)errCode);
  }
  return ret;
}
DaqError My_daqGetLastError(DaqHandleT handle, DaqError *errCode)
{
  DaqError ret=(DaqError)-1;
  if(DAQX_Dll.bOK) {
    ret=(DaqError)DAQX_Dll.CallProcEx32W( 2|CPEX_DEST_STDCALL, 0x02, DAQX_Dll.daqGetLastError, (DWORD)handle, (DWORD)errCode);
  }
  return ret;
}
VOID My_daqDefaultErrorHandler(DaqHandleT handle, DaqError errCode)
{
  if(DAQX_Dll.bOK) {
    DAQX_Dll.CallProcEx32W( 2|CPEX_DEST_STDCALL, 0x00, DAQX_Dll.daqDefaultErrorHandler, (DWORD)handle, (DWORD)errCode);
  }
}
DaqError My_daqFormatError(DaqError errorNum, PCHAR msg)
{
  DaqError ret=(DaqError)-1;
  if(DAQX_Dll.bOK) {
    ret=(DaqError)DAQX_Dll.CallProcEx32W( 2|CPEX_DEST_STDCALL, 0x02, DAQX_Dll.daqFormatError, (DWORD)errorNum, (DWORD)msg);
  }
  return ret;
}


DaqError My_daqGetDeviceCount(DWORD *deviceCount)
{
  *deviceCount=0;
  DaqError ret=(DaqError)-1;
  if(DAQX_Dll.bOK) {
    ret=(DaqError)DAQX_Dll.CallProcEx32W( 1|CPEX_DEST_STDCALL, 0x01, DAQX_Dll.daqGetDeviceCount, (DWORD)deviceCount);
  }
  return ret;
}
DaqError My_daqGetDeviceList(DaqDeviceListT *deviceList, DWORD *deviceCount)
{
  DaqError ret=(DaqError)-1;
  if(DAQX_Dll.bOK) {
    ret=(DaqError)DAQX_Dll.CallProcEx32W( 2|CPEX_DEST_STDCALL, 0x01|0x02, DAQX_Dll.daqGetDeviceList, (DWORD)deviceList, (DWORD)deviceCount);
  }
  return ret;
}
DaqError My_daqGetDeviceProperties(LPSTR daqName, DaqDevicePropsT *deviceProps)
{
  DaqError ret=(DaqError)-1;
  if(DAQX_Dll.bOK) {
    ret=(DaqError)DAQX_Dll.CallProcEx32W( 2|CPEX_DEST_STDCALL, 0x01|0x02, DAQX_Dll.daqGetDeviceProperties, (DWORD)daqName, (DWORD)deviceProps);
  }
  return ret;
}




DaqError My_daqAdcExpSetBank(DaqHandleT handle, DWORD chan, DaqAdcExpType bankType)
{
  DaqError ret=(DaqError)-1;
  if(DAQX_Dll.bOK) {
    ret=(DaqError)DAQX_Dll.CallProcEx32W( 3|CPEX_DEST_STDCALL, 0x00, DAQX_Dll.daqAdcExpSetBank, (DWORD)handle, (DWORD)chan, (DWORD)bankType);
  }
  return ret;
}
DaqError My_daqAdcRd(DaqHandleT handle, DWORD chan, LPWORD sample, DaqAdcGain gain, DWORD flags)
{
  DaqError ret=(DaqError)-1;
  if(DAQX_Dll.bOK) {
    ret=(DaqError)DAQX_Dll.CallProcEx32W( 5|CPEX_DEST_STDCALL, 0x04, DAQX_Dll.daqAdcRd, (DWORD)handle, (DWORD)chan, (DWORD)sample, (DWORD)gain, (DWORD)flags);
  }
  return ret;
}
DaqError My_daqAdcRdScan(DaqHandleT handle, DWORD startChan, DWORD endChan,
			   WORD * buf, DaqAdcGain gain, DWORD flags)
{
  DaqError ret=(DaqError)-1;
  if(DAQX_Dll.bOK) {
    ret=(DaqError)DAQX_Dll.CallProcEx32W( 6|CPEX_DEST_STDCALL, 0x08, DAQX_Dll.daqAdcRdScan, (DWORD)handle, (DWORD)startChan, (DWORD)endChan, (DWORD)buf, (DWORD)gain, (DWORD)flags);
  }
  return ret;
}
DaqError My_daqAdcRdN(DaqHandleT handle, DWORD chan, LPWORD buf, DWORD scanCount,
                           DaqAdcTriggerSource triggerSource, BOOL rising, WORD level, FLOAT freq,
                           DaqAdcGain gain, DWORD flags)
{
  DaqError ret=(DaqError)-1;
  if(DAQX_Dll.bOK) {
    ret=(DaqError)DAQX_Dll.CallProcEx32W( 10|CPEX_DEST_STDCALL, 0x04, DAQX_Dll.daqAdcRdN, (DWORD)handle, (DWORD)chan, (DWORD)buf, (DWORD)scanCount, (DWORD)triggerSource, (DWORD)rising, (DWORD)level, *(DWORD *)&freq, (DWORD)gain, (DWORD)flags);
  }
  return ret;
}
DaqError My_daqAdcRdScanN(DaqHandleT handle, DWORD startChan, DWORD endChan, LPWORD buf,
                           DWORD scanCount, DaqAdcTriggerSource triggerSource, BOOL rising, WORD level,
                           FLOAT freq, DaqAdcGain gain, DWORD flags)
{
  DaqError ret=(DaqError)-1;
  if(DAQX_Dll.bOK) {
    ret=(DaqError)DAQX_Dll.CallProcEx32W( 11|CPEX_DEST_STDCALL, 0x08, DAQX_Dll.daqAdcRdScanN, (DWORD)handle, (DWORD)startChan, (DWORD)endChan, (DWORD)buf,
                           (DWORD)scanCount, (DWORD)triggerSource, (DWORD)rising, (DWORD)level,
			   (DWORD *)&freq, (DWORD)gain, (DWORD)flags);
  }
  return ret;
}





DaqError My_daqDacSetOutputMode(DaqHandleT handle, DaqDacDeviceType deviceType, DWORD chan, DaqDacOutputMode outputMode)
{
  DaqError ret=(DaqError)-1;
  if(DAQX_Dll.bOK) {
    ret=(DaqError)DAQX_Dll.CallProcEx32W( 4|CPEX_DEST_STDCALL, 0x00, DAQX_Dll.daqDacSetOutputMode, (DWORD)handle, (DWORD)deviceType, (DWORD)chan, (DWORD)outputMode);
  }
  return ret;
}
DaqError My_daqDacWt(DaqHandleT handle, DaqDacDeviceType deviceType, DWORD chan, WORD dataVal)
{
  DaqError ret=(DaqError)-1;
  if(DAQX_Dll.bOK) {
    ret=(DaqError)DAQX_Dll.CallProcEx32W( 4|CPEX_DEST_STDCALL, 0x00, DAQX_Dll.daqDacWt, (DWORD)handle, (DWORD)deviceType, (DWORD)chan, (DWORD)/*????*/dataVal);
  }
  return ret;
}

DaqError My_daqDacWtMany(DaqHandleT handle,
			   DaqDacDeviceType *deviceTypes, LPDWORD chans,
			   LPWORD dataVals, DWORD count)
{
  DaqError ret=(DaqError)-1;
  if(DAQX_Dll.bOK) {
    ret=(DaqError)DAQX_Dll.CallProcEx32W( 5|CPEX_DEST_STDCALL, 0x04|0x08, DAQX_Dll.daqDacWtMany, (DWORD)handle, (DWORD)deviceTypes, (DWORD)chans, (DWORD)dataVals, (DWORD)count);
  }
  return ret;
}





DaqError My_daqIOReadBit(DaqHandleT handle, DaqIODeviceType devType, DaqIODevicePort devPort,
                           DWORD whichDevice, DaqIOExpansionPort whichExpPort, DWORD bitNum, PBOOL bitValue)
{
  DaqError ret=(DaqError)-1;
  if(DAQX_Dll.bOK) {
    ret=(DaqError)DAQX_Dll.CallProcEx32W( 7|CPEX_DEST_STDCALL, 0x40, DAQX_Dll.daqIOReadBit, (DWORD)handle, (DWORD)devType, (DWORD)devPort,
                           (DWORD)whichDevice, (DWORD)whichExpPort, (DWORD)bitNum, (DWORD)bitValue);
  }
  return ret;
}
DaqError My_daqIORead(DaqHandleT handle, DaqIODeviceType devType, DaqIODevicePort devPort,
			   DWORD whichDevice, DaqIOExpansionPort whichExpPort, LPDWORD value)
{
  DaqError ret=(DaqError)-1;
  if(DAQX_Dll.bOK) {
    ret=(DaqError)DAQX_Dll.CallProcEx32W( 6|CPEX_DEST_STDCALL, 0x20, DAQX_Dll.daqIORead, (DWORD)handle, (DWORD)devType, (DWORD)devPort,
                           (DWORD)whichDevice, (DWORD)whichExpPort, (DWORD)value);
  }
  return ret;
}
DaqError My_daqIOWriteBit(DaqHandleT handle, DaqIODeviceType devType, DaqIODevicePort devPort,
                           DWORD whichDevice, DaqIOExpansionPort whichExpPort, DWORD bitNum, BOOL bitValue)
{
  DaqError ret=(DaqError)-1;
  if(DAQX_Dll.bOK) {
    ret=(DaqError)DAQX_Dll.CallProcEx32W( 7|CPEX_DEST_STDCALL, 0x00, DAQX_Dll.daqIOWriteBit, (DWORD)handle, (DWORD)devType, (DWORD)devPort,
                           (DWORD)whichDevice, (DWORD)whichExpPort, (DWORD)bitNum, (DWORD)bitValue);
  }
  return ret;
}
DaqError My_daqIOWrite(DaqHandleT handle, DaqIODeviceType devType, DaqIODevicePort devPort,
                           DWORD whichDevice, DaqIOExpansionPort whichExpPort, DWORD value)
{
  DaqError ret=(DaqError)-1;
  if(DAQX_Dll.bOK) {
    ret=(DaqError)DAQX_Dll.CallProcEx32W( 6|CPEX_DEST_STDCALL, 0x00, DAQX_Dll.daqIOWrite, (DWORD)handle, (DWORD)devType, (DWORD)devPort,
                           (DWORD)whichDevice, (DWORD)whichExpPort, (DWORD)value);
  }
  return ret;
}
DaqError My_daqIOGet8255Conf(DaqHandleT handle, BOOL portA, BOOL portB, BOOL portCHigh,
			   BOOL portCLow, LPDWORD config)
{
  DaqError ret=(DaqError)-1;
  if(DAQX_Dll.bOK) {
    ret=(DaqError)DAQX_Dll.CallProcEx32W( 6|CPEX_DEST_STDCALL, 0x20, DAQX_Dll.daqIOGet8255Conf, (DWORD)handle, (DWORD)portA, (DWORD)portB, (DWORD)portCHigh,
                           (DWORD)portCLow, (DWORD)config);
  }
  return ret;
}



/*
DaqError My_daqRegWrite(DaqHandleT handle, DWORD reg, DWORD value)
{
  DaqError ret=(DaqError)-1;
  if(DAQX_Dll.Ok()) {
    DWORD pFunc=DAQX_Dll.GetProcAddress32W("daqRegWrite");
    if(pFunc) {
      ret=(DaqError)DAQX_Dll.CallProcEx32W( 3|CPEX_DEST_STDCALL, 0x00, pFunc, (DWORD)handle, (DWORD)reg, (DWORD)value);
    }
  }
  return ret;
}
DaqError My_daqRegRead(DaqHandleT handle, DWORD reg, LPDWORD value)
{
  DaqError ret=(DaqError)-1;
  if(DAQX_Dll.Ok()) {
    DWORD pFunc=DAQX_Dll.GetProcAddress32W("daqRegRead");
    if(pFunc) {
      ret=(DaqError)DAQX_Dll.CallProcEx32W( 3|CPEX_DEST_STDCALL, 0x04, pFunc, (DWORD)handle, (DWORD)reg, (DWORD)value);
    }
  }
  return ret;
}

*/

#endif // __WIN32__




void GetDaqDeviceName(char *name, int buflen)
{
  *name=0;
  DWORD deviceCount;
  DaqError err=My_daqGetDeviceCount(&deviceCount);
  if(deviceCount) {
    DaqDeviceListT *deviceList=new DaqDeviceListT[deviceCount];
    err=My_daqGetDeviceList(deviceList, &deviceCount);
    for(unsigned deviceIndex=0; deviceIndex<deviceCount; deviceIndex++) {
      //DaqDevicePropsT deviceProps;
      //err=My_daqGetDeviceProperties(deviceList[deviceIndex].daqName, &deviceProps);
      // Just get first card
      //if(devProps.deviceType!=DaqBoard2000) continue;

      lstrcpyn(name,deviceList[deviceIndex].daqName,buflen);
      break;
    }
  }
}


#endif