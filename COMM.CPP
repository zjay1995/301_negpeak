#pragma hdrstop

#ifdef __TURBOC__
  #ifndef __MEM_H
  #include <mem.h>
  #endif
#else
#endif

#ifndef __WINDOWS_H
#include <windows.h>
#endif
#ifndef  _COMM_H
#include "comm.h"
#endif

#ifndef  _WCPPLIB_STR_H
#include "wcpplib\str.h"
#endif

#include "debug.h"

Comm::Comm(LPCSTR lpszDevControl, UINT cbInQueue, UINT cbOutQueue, uint8 _send_id, uint8 _recv_id)
   : WComm(lpszDevControl, cbInQueue, cbOutQueue),
  msg_tx_cnt(0),
 // msg_tx_buf[MAX_MSG];
  msg_retry(0),

  msg_rx_id(0),
  msg_rx_cnt(0),
  msg_rx_cksm(0),
  msg_rx_state(0),
 // msg_rx_buf[MAX_MSG];
  msg_rx_p(msg_rx_buf),

  bWaitingAck(0),
  dwTimeOutEnd(0),
  bRetry(0)
  , send_id(_send_id)
  , recv_id(_recv_id)
{
  memset(msg_tx_buf,0,sizeof(msg_tx_buf));
  memset(msg_rx_buf,0,sizeof(msg_rx_buf));
}

Comm::~Comm()
{
}

void Comm::put_msg(const uint8 *msg, uint8 count)
{
//  msg[count]=0; OutputDebugString("Put "+Str((char *)msg)+"\r\n");
  #ifndef _NDEBUG
  char buff[200];
  int n=count<200? count: 200-1;
  strncpy(buff,msg,n);
  buff[n]='\0';
  TRACE1("put_msg %s",buff);
  #endif

  WriteChar(STX);			// move STX into buffer
  WriteChar(count+1);			// move length into buffer
  WriteChar(send_id/*GC_ID*/);			// write the 311 GC ID
  char cksm = count+1+send_id/*GC_ID*/;		// init checksum

  while(count--) {
    WriteChar(*msg);			// move byte into buffer
    cksm += *msg++;			// update checksum
  }

  WriteChar(-cksm);		        // move -checksum into buffer
  WriteChar(ETX);                     	// move ETX into buffer

  // For Multidrop, the receiver needs to know how many characters to
  // ignore before turning off the transmitter.
  /*
  if(pcb.flag & PCB_MULTI_DROP)
    pcb.md_cnt = count+4;              // set count for receiver
  else
    pcb.md_cnt = 0;                    // no Multidrop

  enbxmt(&pcb);                          // enable transmitter
  */
  ClearBreak();
  return;
}


void Comm::send_msg(const uint8 *msg, uint8 count)
{
  msg_tx_cnt = count;                   // save byte count
  memcpy(msg_tx_buf,msg,msg_tx_cnt);    // save message (for retries)
  msg_retry = MAX_RETRY;                // init message retry count

  Flush(1);				// flush rcv buffer
  bWaitingAck=1;      			// expecting ACK as response
  dwTimeOutEnd=GetTime()+TIMEOUT_ACK;
  bRetry=0;
  put_msg(msg_tx_buf,msg_tx_cnt);	// issue message to comm
}






// Returns = 0 If no error
// else error code
long Comm::get_msg(uint8 *msg, uint8 *count)
{
//  TRACE("get_msg");
  *count=0;
  long err = 0;                       // return code (assume empty)
  for(;;) {
    uint8 b;
    int nRead=ReadChar(&b); // get byte (if any)
    err=GetError();
    if(err!=0 || nRead!=1) break; // while smth to read
    TRACE1("%c",b);
    switch(msg_rx_state) {
      case 0 :               // waiting for <STX>
	if(b == STX)
	  msg_rx_state++;      // advance to state #1
      break;
      case 1 :               // waiting for <CNT>
	msg_rx_cksm = b;        // init checksum accumulator
	msg_rx_cnt = b;         // save size (include cksm)
	msg_rx_state++;         // advance to state #2
      break;
      case 2 :               // waiting for <ID>
	msg_rx_id = b;          // save ID
	msg_rx_cksm += b;       // update checksum accumulator
	msg_rx_p = msg_rx_buf;  // save starting adr of buffer
	msg_rx_state++;         // advance to state #3
      break;
      case 3 :              // waiting for message body
	*msg_rx_p++ = b;        // save byte in array
	msg_rx_cksm += b;       // update checksum accumulator
	if(--msg_rx_cnt == 0)   // if count now 0,
	  msg_rx_state++;       // advance to state #4
      break;
      case 4 :              // waiting for <ETX>
	if(b == ETX) {          // ETX terminator found?
	  if(msg_rx_cksm == 0) {       // if checksum OK,...
	    if(msg_rx_id == recv_id/*GCON_ID*/) { // if ID OK,...
	      // Message received OKay.
	      // Return byte count to caller
	      *count = (int)(msg_rx_p - msg_rx_buf); // return byte count
	      (*count)--;                    	// knock off the checksum byte
	      memcpy(msg,msg_rx_buf,*count); // copy msg into local buf
//	      msg[*count]=0; OutputDebugString("Get "+Str((char *)msg)+"\r\n");
	      err = 0;
	    }
	    else {
	      err = _ID_ERR; //*** ERR: Bad ID ****
	    }
	  }
	  else {
	    err = _CKSM_ERR; //*** ERR: Bad checksum ****
	  }
	}
	else {
	  err = _ETX_ERR;    //*** ERR: no ETX *****
	}
	msg_rx_state = 0;       	// return to waiting for <STX>
	goto end;
      break;
    }
  }
 end:
  if(err & _TO_ERR) { 		// Message Timeout Error?
    msg_rx_state = 0;                	// return to waiting for <STX>
  }

  return err;                        	// return code
}

// Returns = 0 If no error
// else error code
long Comm::recv_msg(uint8 *msg, uint8 *count, int *bAck)
{
  if(bRetry && dwTimeOutEnd<=GetTime()) {
    // A previous error has caused us to retry, and the timeout period
    // .has expired. Re-issue message now.
    bRetry=0;
    bWaitingAck=1;     			// expecting ACK as response
    dwTimeOutEnd=GetTime()+TIMEOUT_ACK;
    put_msg(msg_tx_buf,msg_tx_cnt);	// re-issue message
    *count = 0;
    return 0;
  }

  long err = get_msg(msg,count);        // go check for message rcvd
  TRACE2("get_msg=%d, %ld", *count, err);

  if(err==0 && *count==0) {		// nor responce nor error
    if(dwTimeOutEnd && dwTimeOutEnd<=GetTime()) {
					// timeout period has expired
      err=_TO_ERR;			// GC311 not responds
      msg_rx_state = 0;                	// return to waiting for <STX>
    }
  }
  else if(err==0) {
    dwTimeOutEnd = 0;		// no more timing
  }

  if(err==0 && *count==1) {	        // if OKay message,
    if(msg[0] == NAK_MSG) { 		// NAK received?
      err = _NAK_ERR;
    }
  }

  if(err==0 && *count>0) {		// if OKay message
    if(bWaitingAck) {			// expecting ACK?
      bWaitingAck=0;			// expecting response
      dwTimeOutEnd=GetTime()+TIMEOUT_RESP;     	// set timeout for response
      if(*count==1 && msg[0] == ACK_MSG) { // ACK received?
	//   ACK expected and ACK received. Prepare for real response message
	*count = 0;
	err = 0;                   // don't tell caller about the ACK.
	*bAck = 1;		   // tell caller about the ACK.
      }
      else {                         // unexpected message
	err = _PROTO_ERR;      //*** ERR: protocol error *****
      }
    }
    else  {                            // expecting Response
      bWaitingAck=0;
      dwTimeOutEnd=0;
      put_msg((uint8*)"A",1);       	// issue "ACK"
    }
  }
  if(err) {                        // if bad message,...
    // If the error is a timeout or a NAK received, retry the command.
    if((err == _TO_ERR) || (err == _NAK_ERR)) {
      //   Resend message if retry count not exhausted, but only after
      //   the timeout period.
      msg_retry--;
      if(msg_retry) {
	bRetry=1;        // set flag, wait for timeout
	*count = 0;
	err = 0;                      // don't tell caller yet
      }
    }
    else {
      //  If the error is anything else, send a NAK to the GC, telling it to
      //  retransmit the same message.
      Flush(0);				// flush xmt buffer
      Flush(1);		   		// flush rcv buffer
      msg_rx_state = 0;                	// return to receive state waiting for <STX>
      bWaitingAck=0;
      dwTimeOutEnd=GetTime()+TIMEOUT_RESP;
      put_msg((uint8*)"N",1);		// issue "NAK"
      *count = 0;
      err = 0;                      	// don't tell caller yet
    }
  }
  return err;
}




