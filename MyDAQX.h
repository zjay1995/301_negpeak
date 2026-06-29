

#include "DaqX.h"


#ifndef __WIN32__


DaqHandleT My_daqOpen(LPCSTR daqName);
DaqError My_daqClose(DaqHandleT handle);
DaqError My_daqOnline(DaqHandleT handle, PBOOL *online);

DaqError My_daqProcessError(DaqHandleT handle, DaqError errCode);
DaqError My_daqGetLastError(DaqHandleT handle, DaqError *errCode);
VOID My_daqDefaultErrorHandler(DaqHandleT handle, DaqError errCode);
DaqError My_daqFormatError(DaqError errorNum, PCHAR msg);


DaqError My_daqGetDeviceCount(DWORD *deviceCount);
DaqError My_daqGetDeviceList(DaqDeviceListT *deviceList, DWORD *deviceCount);
DaqError My_daqGetDeviceProperties(LPSTR daqName, DaqDevicePropsT *deviceProps);


DaqError My_daqAdcExpSetBank(DaqHandleT handle, DWORD chan, DaqAdcExpType bankType);
DaqError My_daqAdcRd(DaqHandleT handle, DWORD chan, LPWORD sample, DaqAdcGain gain, DWORD flags);
DaqError My_daqAdcRdScan(DaqHandleT handle, DWORD startChan, DWORD endChan,
			   WORD * buf, DaqAdcGain gain, DWORD flags);
DaqError My_daqAdcRdN(DaqHandleT handle, DWORD chan, LPWORD buf, DWORD scanCount,
                           DaqAdcTriggerSource triggerSource, BOOL rising, WORD level, FLOAT freq,
                           DaqAdcGain gain, DWORD flags);
DaqError My_daqAdcRdScanN(DaqHandleT handle, DWORD startChan, DWORD endChan, LPWORD buf,
                           DWORD scanCount, DaqAdcTriggerSource triggerSource, BOOL rising, WORD level,
                           FLOAT freq, DaqAdcGain gain, DWORD flags);


DaqError My_daqDacSetOutputMode(DaqHandleT handle, DaqDacDeviceType deviceType, DWORD chan, DaqDacOutputMode outputMode);

DaqError My_daqDacWt(DaqHandleT handle, DaqDacDeviceType deviceType, DWORD chan, WORD dataVal);

DaqError My_daqDacWtMany(DaqHandleT handle,
			   DaqDacDeviceType *deviceTypes, LPDWORD chans,
			   LPWORD dataVals, DWORD count);


DaqError My_daqIOReadBit(DaqHandleT handle, DaqIODeviceType devType, DaqIODevicePort devPort,
                           DWORD whichDevice, DaqIOExpansionPort whichExpPort, DWORD bitNum, PBOOL bitValue);
DaqError My_daqIORead(DaqHandleT handle, DaqIODeviceType devType, DaqIODevicePort devPort,
			   DWORD whichDevice, DaqIOExpansionPort whichExpPort, LPDWORD value);
DaqError My_daqIOWriteBit(DaqHandleT handle, DaqIODeviceType devType, DaqIODevicePort devPort,
                           DWORD whichDevice, DaqIOExpansionPort whichExpPort, DWORD bitNum, BOOL bitValue);
DaqError My_daqIOWrite(DaqHandleT handle, DaqIODeviceType devType, DaqIODevicePort devPort,
                           DWORD whichDevice, DaqIOExpansionPort whichExpPort, DWORD value);
DaqError My_daqIOGet8255Conf(DaqHandleT handle, BOOL portA, BOOL portB, BOOL portCHigh,
			   BOOL portCLow, LPDWORD config);


//DaqError My_daqRegWrite(DaqHandleT handle, DWORD reg, DWORD value);
//DaqError My_daqRegRead(DaqHandleT handle, DWORD reg, LPDWORD value);


#else // __WIN32__


//#define My_daqOpen                          daqOpen
DaqHandleT My_daqOpen(LPCSTR daqName) { return daqOpen((LPSTR)daqName); }// LPSTR=>LPCSTR
#define My_daqClose                         daqClose
#define My_daqOnline                        daqOnline

#define My_daqProcessError                  daqProcessError
#define My_daqGetLastError                  daqGetLastError
#define My_daqDefaultErrorHandler           daqDefaultErrorHandler
#define My_daqFormatError                   daqFormatError


#define My_daqGetDeviceCount                daqGetDeviceCount
#define My_daqGetDeviceList                 daqGetDeviceList
#define My_daqGetDeviceProperties           daqGetDeviceProperties


#define My_daqAdcExpSetBank                 daqAdcExpSetBank
#define My_daqAdcRd                         daqAdcRd
#define My_daqAdcRdScan                     daqAdcRdScan
#define My_daqAdcRdN                        daqAdcRdN

#define My_daqAdcRdScanN                    daqAdcRdScanN


#define My_daqDacSetOutputMode              daqDacSetOutputMode

#define My_daqDacWt                         daqDacWt

#define My_daqDacWtMany                     daqDacWtMany


#define My_daqIOReadBit                     daqIOReadBit
#define My_daqIORead                        daqIORead
#define My_daqIOWriteBit                    daqIOWriteBit
#define My_daqIOWrite                       daqIOWrite
#define My_daqIOGet8255Conf                 daqIOGet8255Conf


//#define My_daqRegWrite                    daqRegWrite
//#define My_daqRegRead                     daqRegRead

#pragma comment(lib, "daqx.lib")

#endif // __WIN32_


void GetDaqDeviceName(char *name, int buflen);
