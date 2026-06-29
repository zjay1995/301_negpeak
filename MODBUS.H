#ifndef  _MODBUS_H
#define  _MODBUS_H

#ifndef  _SHORTS_H
#include "shorts.h"
#endif

#define MAX_MSG         260            // Maximum Message
//#define MAX_RETRY        3             // Maximum Retries

//#define TIMEOUT_ACK     2000L         // ACK      Timeout = 2 Seconds
//#define TIMEOUT_RESP    5000L	      // Response Timeout = 5 Seconds

#ifndef  _WCOMM_H
#include "wcomm.h"
#endif


// ModBus Slave
class ModBusSlave : public WComm
{
  uint8 msg_tx_cnt;              // message transmit byte count
  uint8 msg_tx_buf[MAX_MSG];     // message transmit buffer


  uint8 msg_rx_cnt;              // message receive byte count
  uint8 msg_rx_state;	         // message receive state
  uint8 msg_rx_buf[MAX_MSG];     // message receive buffer
  uint8 *msg_rx_p;               // ptr to rx message buffer

  DWORD dwTimeOutEnd;

  void put_msg(uint8 address, uint8 func, uint8 *msg, uint8 count);
  long get_msg(uint8 *address, uint8 *func, uint8 *msg, uint8 *count);

 public:
  ModBusSlave(LPCSTR lpszDevControl, UINT cbInQueue, UINT cbOutQueue);
  ~ModBusSlave();

  void send_msg(uint8 address, uint8 func, uint8 *msg, uint8 count);
  long recv_msg(uint8 *address, uint8 *func, uint8 *msg, uint8 *count);
};

// Error codes additional to Windows Comm Error Codes CE_xxxx

#define _NAK_ERR 	0x00010000L	// NAK received
#define _ID_ERR  	0x00020000L	// Wrong ID received
#define _CKSM_ERR	0x00040000L     // Receive Checksum Error
#define _ETX_ERR	0x00080000L     // No ETX terminator received
#define _TO_ERR		0x00100000L     // Message Timeout Error
#define _PROTO_ERR	0x00200000L	// Protocol Error (unknown message)
/*

inline DWORD GetTime()
{
  return GetTickCount();  // accuracy == 55ms
  //return timeGetTime(); // in <mmsystem.h> ( more acurate! == 1ms )
}
*/



//// WPeak implementaton


class ModBusServer : public ModBusSlave {
 public:
  ModBusServer(LPCSTR lpszDevControl);

  void OnTimer();
 protected:
  void Process();
};







#endif
