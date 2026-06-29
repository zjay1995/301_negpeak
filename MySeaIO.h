
#ifndef __WINDOWS_H
#include <windows.h>
#endif

#include "SeaIO32.h"

#ifndef __WIN32__
unsigned long My_SeaIo_OpenDevice(unsigned short wPortNum,
					HANDLE32 FAR *pHDevice);
unsigned long My_SeaIo_CloseDevice(HANDLE32 hDevice);
// Write a single byte to a single port on a device.
// Returns SEAIO_ERROR_NONE on success; error code on failure.
unsigned long My_SeaIo_WriteBit(HANDLE32 hDevice,
				      unsigned long BitNumber,
				      const BOOLEAN State,
				      enum AccessMode Mode);

// Read the state of a single bit / relay on a device
// Returns SEAIO_ERROR_NONE on success; error code on failure.
unsigned long My_SeaIo_ReadBit(HANDLE32 hDevice,
			       unsigned long BitNumber,
			       BOOLEAN FAR *BitState,
			       enum AccessMode Mode);

// added. P.P. 07/27/04
// Write a single byte to a single port on a device.
// Returns SEAIO_ERROR_NONE on success; error code on failure.
unsigned long My_SeaIo_WriteByte(HANDLE32 hDevice,
				       unsigned long Port,
                                       const unsigned char Data, 
                                       enum AccessMode Mode);

// OBSOLETE
unsigned long My_SeaIo_RelayClose(HANDLE32 hDevice, unsigned char uRelayNum);

// OBSOLETE
unsigned long My_SeaIo_RelayOpen(HANDLE32 hDevice, unsigned char uRelayNum);


#else // __WIN32__

#define My_SeaIo_OpenDevice     SeaIo_OpenDevice
#define My_SeaIo_CloseDevice    SeaIo_CloseDevice
#define My_SeaIo_WriteBit       SeaIo_WriteBit
#define My_SeaIo_ReadBit        SeaIo_ReadBit
#define My_SeaIo_WriteByte      SeaIo_WriteByte
#define My_SeaIo_RelayClose     SeaIo_RelayClose
#define My_SeaIo_RelayOpen      SeaIo_RelayOpen

#pragma comment(lib, "SeaIo32.lib")

#endif // __WIN32__

BOOL GetSeaIoDevice(unsigned short *wPortNum);
void GetSeaIoErrorString(unsigned long error, char *errorstring);

