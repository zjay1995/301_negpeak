#ifndef  _COMM_H
#define  _COMM_H

#ifndef  _SHORTS_H
#include "shorts.h"
#endif

#define MAX_MSG         260            // Maximum Message
#define MAX_RETRY        3             // Maximum Retries

#define GCON_ID         0              // ID of G-CON
#define GC_ID           1              // ID of the 311 GC

#define WPEAK_ID        2              // ID of wPeak
#define WPEAKC_ID       3              // ID of wPeakComm

#define STX             2
#define ETX             3

#define ACK_MSG         'A'
#define NAK_MSG         'N'

#define TIMEOUT_ACK     2000L         // ACK      Timeout = 2 Seconds
#define TIMEOUT_RESP    5000L	      // Response Timeout = 5 Seconds
/*
#define TIMEOUT_ACK     1000L         // ACK      Timeout = 1 Second
#define TIMEOUT_RESP    2000L	      // Response Timeout = 2 Seconds
*/

#ifndef  _WCOMM_H
#include "wcomm.h"
#endif


class Comm : public WComm {

  uint8 msg_tx_cnt;              // message transmit byte count
  uint8 msg_tx_buf[MAX_MSG];     // message transmit buffer
  uint8 msg_retry;               // message retry count


  uint8 msg_rx_id;               // message receive ID
  uint8 msg_rx_cnt;              // message receive byte count
  char  msg_rx_cksm;             // message receive checksum acc
  uint8 msg_rx_state;	         // message receive state
  uint8 msg_rx_buf[MAX_MSG];     // message receive buffer
  uint8 *msg_rx_p;               // ptr to rx message buffer

  BOOL bWaitingAck;
  DWORD dwTimeOutEnd;
  DWORD bRetry;

  void put_msg(const uint8 *msg, uint8 count);
  long get_msg(uint8 *msg, uint8 *count);

  uint8 send_id; // usually GC_ID
  uint8 recv_id; // usually GCON_ID

 public:
  Comm(LPCSTR lpszDevControl, UINT cbInQueue, UINT cbOutQueue, uint8 send_id, uint8 recv_id);
  ~Comm();

  void send_msg(const uint8 *msg, uint8 count);
  long recv_msg(uint8 *msg, uint8 *count, int *bAck);
};

// Error codes additional to Windows Comm Error Codes CE_xxxx

#define _NAK_ERR 	0x00010000L	// NAK received
#define _ID_ERR  	0x00020000L	// Wrong ID received
#define _CKSM_ERR	0x00040000L     // Receive Checksum Error
#define _ETX_ERR	0x00080000L     // No ETX terminator received
#define _TO_ERR		0x00100000L     // Message Timeout Error
#define _PROTO_ERR	0x00200000L	// Protocol Error (unknown message)


inline DWORD GetTime()
{
  return GetTickCount();  // accuracy == 55ms
  //return timeGetTime(); // in <mmsystem.h> ( more acurate! == 1ms )
}

#endif
