/*******************************************************************
  SeaIo32.H

    (C) Copyright 1996 Sealevel Systems, Inc.

  Include file for SeaIo API.

  History:

    1996-09-24  VSG     Created file.
    2001-09-17  JMA     Obsoleted numerous API calls; introduced
						their replacements and added a few.

 *******************************************************************/
#ifndef SEAIO32_H_215243525
#define SEAIO32_H_215243525
// SEAIO32.DLL function prototypes

#ifndef APIENTRY
#define APIENTRY
#endif
typedef BYTE  BOOLEAN;
#ifndef _WIN32
typedef void FAR *LPOVERLAPPED; // <winbase.h>
#endif
typedef void FAR *pADAPTER_INFO;
typedef void FAR *pADAPTER_STATE;

typedef struct __SEAIO { int dummy; } *HANDLE32;


#ifdef __cplusplus
    extern "C" {
#endif

enum Direction {
    Input,
    Output,
    NotExist
};

enum AccessMode {
    Relative,
    Absolute
};

typedef enum {
	DIO = 0,
	PIO_8255 = 1,
	PIO_Non8255 = 2,
} SeaIo_CardType;

// HANDLE is defined in Windows.h as follows:
//
//   typedef void *HANDLE;
//
// VB (and other) programmers have successfully used type int for handle 
// values.



// These functions comprise the current SeaIO API.

// Obtain the file handle for a device and open it for I/O
// Returns SEAIO_ERROR_NONE on success; error code on failure.
unsigned long APIENTRY SeaIo_OpenDevice(unsigned short wPortNum, 
					HANDLE32 *pHDevice);

// Close the file handle for a SeaIO device
// Returns SEAIO_ERROR_NONE on success; error code on failure.
unsigned long APIENTRY SeaIo_CloseDevice(HANDLE32 hDevice);

// Set the state of all available output ports on a card in a single 
// operation.
// This function is specially nuanced for USB devices; refer to the 
// documentation for more information.
// Returns SEAIO_ERROR_NONE on success; error code on failure.
unsigned long APIENTRY SeaIo_WriteAllOutputs(HANDLE32 hDevice,
                                           const unsigned char *pBuffer, 
                                           unsigned long *Length);

// Get the state of all available input ports on a card in a single
// operation.
// This function is specially nuanced for USB devices; refer to the 
// documentation for more information.
// Returns SEAIO_ERROR_NONE on success; error code on failure.
unsigned long APIENTRY SeaIo_ReadAllInputs(HANDLE32 hDevice,
                                          unsigned char *pBuffer, 
                                          unsigned long *Length);


// Write a single byte to a single port on a device.
// Returns SEAIO_ERROR_NONE on success; error code on failure.
unsigned long APIENTRY SeaIo_WriteByte(HANDLE32 hDevice,
                                       unsigned long Port, 
                                       const unsigned char Data, 
                                       enum AccessMode Mode);

// Read a single byte from a single port on a device
// Returns SEAIO_ERROR_NONE on success; error code on failure.
unsigned long APIENTRY SeaIo_ReadByte(HANDLE32 hDevice,
                                      unsigned long Port, 
									  unsigned char *Byte,
                                      enum AccessMode Mode);

// Set the state of a single bit / relay on a device
// Returns SEAIO_ERROR_NONE on success; error code on failure.
unsigned long APIENTRY SeaIo_WriteBit(HANDLE32 hDevice,
                                      unsigned long BitNumber, 
                                      const BOOLEAN State, 
                                      enum AccessMode Mode);

// Read the state of a single bit / relay on a device
// Returns SEAIO_ERROR_NONE on success; error code on failure.
unsigned long APIENTRY SeaIo_ReadBit(HANDLE32 hDevice,
                               unsigned long BitNumber,
							   BOOLEAN *BitState,
                               enum AccessMode Mode);


// Block until the state of any input bit changes, then return all data from 
// all input ports.
// Returns STATUS_PENDING on success; error code on failure.
// Call GetOverlappedResult() to initiate blocking and wait for data.
unsigned long APIENTRY SeaIo_NotifyInputChange(HANDLE32 hDevice,
                                               unsigned char *pBuffer, 
                                               unsigned long Length, 
                                               unsigned long *pRetLength, 
                                               LPOVERLAPPED Overlapped);

// Get information about a device's model number, resource allocation, number 
// of available inputs / outputs, etc.
// Returns SEAIO_ERROR_NONE on success, error code on failure.
unsigned long APIENTRY SeaIo_GetAdapterInfo(HANDLE32 hDevice,
                                            pADAPTER_INFO pAdapter_Info);

// Get information about a device's sampling rate, mode control words, 
// interrupt sampling mode, etc.  
// This function also causes the driver to re-read its configuration from the 
// registry, resetting any volatile configuration which may have previously 
// been set but not saved.
// Returns SEAIO_ERROR_NONE on success; error code on failure.
unsigned long APIENTRY SeaIo_GetAdapterState(HANDLE32 hDevice,
					      pADAPTER_STATE pAdapterState);

// Set a device's sampling rate, mode control words, interrupt sampling mode, 
// etc.  
// This sets a volatile configuration; calling SeaIo_GetAdapterState will 
// reset the adapter to the configuration recorded in the registry.  To set up
// a non-volatile configuration, call SeaIo_SetAdapterState followed by 
// SeaIo_SaveAdapterState.
// NOTE: this is the only way to configure the 4030 adapter for "nybble 
// mode" -- refer to the documentation for more details.
// Returns SEAIO_ERROR_NONE on success; error code on failure.
unsigned long APIENTRY SeaIo_SetAdapterState(HANDLE32 hDevice,
                                             pADAPTER_STATE pAdapterState);

// Save a volatile adapter state to the registry, making it permanent
// Returns SEAIO_ERROR_NONE on success; error code on failure.
unsigned long APIENTRY SeaIo_SaveAdapterState(HANDLE32 hDevice);

// Set the direction of I/O on a single port for programmable I/O devices.
// Returns SEAIO_ERROR_NONE on success; error code on failure.
unsigned long APIENTRY SeaIo_SetDirection(HANDLE32 hDevice,
                                          unsigned char PortNum, 
                                          const enum Direction direction);

// Get direction of I/O on programmable devices.
// Returns SEAIO_ERROR_NONE on success; error code on failure.
unsigned long APIENTRY SeaIo_GetDirection(HANDLE32 hDevice,
                                           unsigned char PortNum,
										   enum Direction *direction);

// Get the serial string stored in EEPROM for a USB device
// Returns SEAIO_ERROR_NONE on success; error code on failure.
unsigned long APIENTRY SeaIo_GetSerialString(HANDLE32 hDevice,
                                             char Buffer[9] );

// Set the serial string stored in EEPROM for a USB device
// Returns SEAIO_ERROR_NONE on success; error code on failure.
unsigned long APIENTRY SeaIo_SetSerialString(HANDLE32 hDevice,
                                             char Buffer[9] );

// Initiate sampling operation for devices which support interrupts.
// Returns SEAIO_ERROR_NONE on success; error code on failure.
unsigned long APIENTRY SeaIo_StartInterruptSampling(HANDLE32 hDevice);

// Cease sampling operation for devices which support interrupts.
// Returns SEAIO_ERROR_NONE on success; error code on failure.
unsigned long APIENTRY SeaIo_StopInterruptSampling(HANDLE32 hDevice);

// Clear any data which may have been in the interrupt-sampled buffer.
// Returns SEAIO_ERROR_NONE on success; error code on failure.
unsigned long APIENTRY SeaIo_FlushBuffer(HANDLE32 hDevice);

// Find out what type of card we're using
// Returns the appropriate SeaIo_CardType 
// DIO = Digital / relay I/O;
// PIO_8255 = cards which use or mimic 8255-style control words, (i.e. 4030, 8205)
// PIO_Non8255 = cards which use linear control words (i.e. 8001, 8010)
SeaIo_CardType APIENTRY SeaIo_GetCardType(HANDLE32 hDevice);


/*****************************************************************************/
// The following calls are no longer supported.  
// They remain here for backward compatibility.

// OBSOLETE
unsigned long APIENTRY SeaIo_WriteReg(HANDLE32 hDevice,
                                      unsigned char bRegNumber, 
                                      unsigned char bMask);

// OBSOLETE
unsigned long APIENTRY SeaIo_ReadReg(HANDLE32 hDevice,
                                     unsigned char bRegNumber, 
                                     unsigned char *pBuffer, 
                                     unsigned long dwLength, 
                                     unsigned long *pDwRetLength);

// OBSOLETE
unsigned long APIENTRY SeaIo_RelayClose(HANDLE32 hDevice,
                                        unsigned char uRelayNum);

// OBSOLETE
unsigned long APIENTRY SeaIo_RelayOpen(HANDLE32 hDevice,
                                       unsigned char uRelayNum);

// OBSOLETE
unsigned long APIENTRY SeaIo_PutData(HANDLE32 hDevice,
                                     unsigned char *pBuffer);

// OBSOLETE
unsigned long APIENTRY SeaIo_GetData(HANDLE32 hDevice,
                                     unsigned char *pBuffer, 
                                     unsigned long dwLength, 
                                     unsigned long *pDwRetLength);

// OBSOLETE
unsigned long APIENTRY SeaIo_Cancel(HANDLE32 hDevice);

// UNSUPPORTED -- this call WILL damage your device if used improperly.
unsigned long APIENTRY SeaIo_GetSerialString16(HANDLE32 hDevice,
                                               char *Buffer );

// UNSUPPORTED -- this call WILL damage your device if used improperly.
unsigned long APIENTRY SeaIo_SetSerialString16(HANDLE32 hDevice,
                                               char *Buffer );
/*****************************************************************************/

#ifdef __cplusplus
    }
#endif

#endif // #ifndef SEAIO32_H_215243525