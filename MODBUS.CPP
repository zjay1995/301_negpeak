#include "includes.h"
#pragma hdrstop

#ifdef __TURBOC__
  #ifndef __MEM_H
  #include <mem.h>
  #endif
#else
#endif

#include <windows.h>


#include "modbus.h"

#include "wcpplib\str.h"

#include <stdio.h> // sscanf
#include "debug.h"
#include "comm.h" // GetTime()

ModBusSlave::ModBusSlave(LPCSTR lpszDevControl, UINT cbInQueue, UINT cbOutQueue)
   : WComm(lpszDevControl, cbInQueue, cbOutQueue),
  msg_tx_cnt(0),
 // msg_tx_buf[MAX_MSG];

  msg_rx_cnt(0),
  msg_rx_state(0),
 // msg_rx_buf[MAX_MSG];
  msg_rx_p(msg_rx_buf)
 , dwTimeOutEnd(0)
{
  memset(msg_tx_buf,0,sizeof(msg_tx_buf));
  memset(msg_rx_buf,0,sizeof(msg_rx_buf));
}

ModBusSlave::~ModBusSlave()
{
}

void ModBusSlave::put_msg(uint8 address, uint8 func, uint8 *msg, uint8 count)
{
//  msg[count]=0; OutputDebugString("Put "+Str((char *)msg)+"\r\n");
  TRACE("ModBusSlave::put_msg");

  WriteChar(':');			// move Start into buffer
  TRACE(":");

  char buff[30];

  wsprintf(buff,"%02X",(uint)address);
  Write(buff,2);                        // move address into buffer
  TRACE1("%s",buff);

  wsprintf(buff,"%02X",(uint)func);
  Write(buff,2);                        // move function num into buffer
  TRACE1("%s",buff);

  wsprintf(buff,"%02X",(uint)count);
  Write(buff,2);                        // move length into buffer
  TRACE1("%s",buff);


  uint8 cksm = address+func+count;		// init checksum

  while(count--) {
    wsprintf(buff,"%02X",(uint)*msg);
    Write(buff,2);			// move byte into buffer
    TRACE1("%s",buff);
    cksm += *(msg++);			// update checksum
  }

  wsprintf(buff,"%02X",(uint8)((uint)-(int8)cksm));
  Write(buff,2);		        // move -checksum into buffer
  TRACE1("%s",buff);
  Write("\r\n",2);			// move End into buffer

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


void ModBusSlave::send_msg(uint8 address, uint8 func, uint8 *msg, uint8 count)
{
  msg_tx_cnt = count;                   // save byte count
  memcpy(msg_tx_buf,msg,msg_tx_cnt);    // save message (for retries)

  dwTimeOutEnd=GetTime()+1000;//
  Flush(1);				// flush rcv buffer
  put_msg(address, func, msg_tx_buf,msg_tx_cnt);	// issue message to comm
}


static uint8 hexchar(uint8 c)
{
  if(c>'a') return c-('a'-10);
  if(c>'A') return c-('A'-10);
  return c-'0';
}

// Returns = 0 If no error
// else error code
long ModBusSlave::get_msg(uint8 *address, uint8 *func, uint8 *msg, uint8 *count)
{
//  TRACE("ModBusSlave::get_msg");
  *count=0;
  long err = 0;                       // return code (assume empty)
  for(;;) {
    uint8 b;
    int nRead=ReadChar(&b); // get byte (if any)
    /*
    static const uint8 msg1[]="\x3a\x30\x31\x30\x34\x30\x30\x30\x30\x30\x30\x30\x34\x46\x37\x0d\x0a";
    static const uint8 *cp=msg1;
    b=*(cp++);
    int nRead=1;
    */
    err=GetError();
    if(err!=0 || nRead!=1) break; // while smth to read
    dwTimeOutEnd=GetTime()+1200;//?? 1000;//
    TRACE1("ModBusSlave:: got char %c",b);
    switch(msg_rx_state) {
      case 0 :               // waiting for <STX>
	if(b == ':') {
	  msg_rx_state++;      // advance to state #1
	  msg_rx_p=msg_rx_buf;
	}
      break;
      case 1 :               // waiting for <data>
	if(b=='\r') {
	  msg_rx_state++;         // advance to the next state
	}
	else {
	  *(msg_rx_p++) = b;        // save byte in array
	  //msg_rx_state++;         // advance to the next state
	}
      break;
      case 2 :               // waiting for <data>
	if(b=='\n') {           // ETX terminator found?
	  int cnt = (int)(msg_rx_p - msg_rx_buf); // count of recieved bytes
	  if(cnt%2==0 && cnt>2*3/*address + func + chksum*/) {
	    cnt/=2;
	    uint8 cksm=0;

            // decode and calc chksum
	    for(int i=0; i<cnt; i++) {
	      const uint8 *buff=&msg_rx_buf[i*2];
              uint8 tmp=hexchar(buff[0])*16+hexchar(buff[1]);
              msg_rx_buf[i]=tmp;
	      cksm+=tmp;
	    }

	    if(cksm == 0) {       // if checksum OK,...
	      // Message received OKay.
	      // Return byte count to caller
	      *count = cnt;
	      (*count)--;                    	// knock off the checksum byte

	      *address=msg_rx_buf[0]; (*count)--;
	      *func=msg_rx_buf[1]; (*count)--;

	      memcpy(msg,msg_rx_buf+2,*count); // copy msg into local buf
//	      msg[*count]=0; OutputDebugString("Get "+Str((char *)msg)+"\r\n");
	      err = 0;
	    }
	    else {
	      err = _CKSM_ERR; //*** ERR: Bad checksum ****
	    }
	  }
	  else {                         // unexpected message
	    err = _PROTO_ERR;      //*** ERR: protocol error *****
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
long ModBusSlave::recv_msg(uint8 *address, uint8 *func, uint8 *msg, uint8 *count)
{
  long err = get_msg(address,func,msg,count);        // go check for message rcvd
  TRACE2("ModBusSlave::get_msg=%d, %ld", *count, err);

  if(err==0 && *count==0) {		// nor responce nor error
    if(dwTimeOutEnd && dwTimeOutEnd<=GetTime()) {
					// timeout period has expired
    //??   err=_TO_ERR;			// GC311 not responds
    //??   msg_rx_state = 0;                	// return to waiting for <STX>
    }
  }
  else if(err==0) {
    dwTimeOutEnd = 0;		// no more timing
  }

  //if(err==0 && *count==1) {	        // if OKay message,
    //if(msg[0] == NAK_MSG) { 		// NAK received?
    //  err = _NAK_ERR;
    //}
  //}

  if(err==0 && *count>0) {		// if OKay message
    /*if(bWaitingAck) {			// expecting ACK?
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
    else*/ {                            // expecting Response
      dwTimeOutEnd=0;
    }

  }
  if(err) {                        // if bad message,...
    {
      //  If the error is anything else, send a NAK to the GC, telling it to
      //  retransmit the same message.
      Flush(0);				// flush xmt buffer
      Flush(1);		   		// flush rcv buffer
      msg_rx_state = 0;                	// return to receive state waiting for <STX>
      //dwTimeOutEnd=GetTime()+TIMEOUT_RESP;


      //put_msg((uint8*)"N",1);		// issue "NAK"
      put_msg(*address, (*func)|0x80, (uint8*)"N",1);		// issue "NAK"


      *count = 0;
      err = 0;                      	// don't tell caller yet
    }
  }
  return err;
}


///
extern uint16 GetModBusRegister(uint16 i);


ModBusServer::ModBusServer(LPCSTR lpszDevControl)
 : ModBusSlave(lpszDevControl,1024*10,1024*10)
{
}

void ModBusServer::OnTimer()
{
  COMSTAT stat;
  for(;;) {
    int err=GetError(&stat);
    if(err) break;
    if(!stat.cbInQue) break;

    // Process if something to read
    Process();
  }
}

void ModBusServer::Process()
{
  uint8 address;
  uint8 func;
  uint8 msg[300];
  uint8 count;
  long err=recv_msg(&address, &func, msg, &count);
  if(err) return;

  if(count) {
    //?? if(address!=??) return; //??
    switch(func) {
      case 0x03: // Read Holding Registers
      case 0x04: // Read Input Registers
      {
	uint16 start   =(msg[0]<<8)+msg[1]; // hi and lo bytes
	uint16 quantity=(msg[2]<<8)+msg[3]; // hi and lo bytes

	if(quantity<1 || quantity>0x7d) {
          send_msg(address, func|0x80, (uint8*)"\x03", 1); // Wrong quantity of registers
	}
	/*??else if(start>1000?) {
          send_msg(address, func|0x80, "\x02", 1); // Wrong start register
	}*/
        else { // OK
          count=0;
          for(uint16 i=0; i<quantity; i++) {
            uint16 r=GetModBusRegister(start+i);
            msg[i*2]=r>>8; // hi byte
            msg[i*2+1]=r; // lo byte
            count+=2;
          }
	  send_msg(address, func, msg, count);
	}
      }
      break;
      default:
      {
	send_msg(address, func|0x80, (uint8*)"\x01", 1); // Not supported
      }
      break;
    }
  }
}


