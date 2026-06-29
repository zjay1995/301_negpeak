#pragma hdrstop


#include <math.h>
#include "method.h"


#include "debug.h"

extern char data_rate;           // rate data received in pts/sec.


#include "calc.h"


DataQueue::DataQueue(int _segment_width)
 : i(-1)
 , segment_width(_segment_width)
 , nMultipleSegment(1)
 , real_segment_width(_segment_width)
{
}


int DataQueue::IsSampleFinished()
{
  return i%segment_width==segment_width-1;
}

void DataQueue::Put(long d)
{
  i++;
  if(i==real_segment_width) {
    int new_real_segment_width=segment_width*nMultipleSegment;

    if(new_real_segment_width>real_segment_width) { // real segment changed
      // duplicate previous data (i.e. nMultipleSegment==3 after 1)
      for(int idx=real_segment_width+segment_width; idx<new_real_segment_width; idx++) {
	data[idx]=data[idx-segment_width];
      }
      // do not change i! Continue fill the array
    }
    else {
      i=0; // begin fill the array from the beginning
    }
    real_segment_width=new_real_segment_width;
  }
  data[i]=d;
}
  // replaced for code below. P.P. 12/26/06  restored 03/03/07
long DataQueue::GetScaledSample()
{
  long scaled_sample=0l;
//  uint32 scaled_sample=0l;  // P.P. 11/13/14
  for(int idx=0; idx<real_segment_width; idx++) {
    scaled_sample += data[idx];
//    scaled_sample += data[idx]&0x8000FFFF;   // remove range. P.P. 11/07/14
  }
  scaled_sample /= real_segment_width;
  return scaled_sample;
}

/*
//static long prev_4_scaled_sample = 0l;
//static long prev_3_scaled_sample = 0l;
static long prev_2_scaled_sample = 0l;
static long prev_1_scaled_sample = 0l;
long DataQueue::GetScaledSample() // variant with sliding averaging. P.P. 12/26/06
{
//static long prev_4_scaled_sample = 0l;
//static long prev_3_scaled_sample = 0l;
//static long prev_2_scaled_sample = 0l;
//static long prev_1_scaled_sample = 0l;
  long scaled_0_sample=0l;
  long scaled_sample =0l;
  for(int idx=0; idx<real_segment_width; idx++) {
    scaled_0_sample += data[idx];
  }
  scaled_0_sample /= real_segment_width;
//  scaled_sample=(prev_2_scaled_sample+prev_1_scaled_sample+scaled_0_sample)/3;
  scaled_sample=(prev_2_scaled_sample+prev_1_scaled_sample*2+scaled_0_sample)/4;
//  scaled_sample=(prev_4_scaled_sample+2*prev_3_scaled_sample+3*prev_2_scaled_sample+2*prev_1_scaled_sample+scaled_0_sample)/9;
//  prev_4_scaled_sample=prev_3_scaled_sample;
//  prev_3_scaled_sample=prev_2_scaled_sample;
  prev_2_scaled_sample=prev_1_scaled_sample;
  prev_1_scaled_sample=scaled_0_sample;
  return scaled_sample;
}
*/

long DataQueue::GetRawSample()
{
  return data[i];
}


//RawData::RawData(int segment_width, uint _analysis_time)
RawData::RawData(int segment_width, ulong _analysis_time)  // P.P. 09/06/09
  : gain(-1), dq(segment_width),
  acquire_finished(0),
  analysis_time(_analysis_time)
  , nErrorMesages(3)
{
  raw_data.time = 0;         // initialize retention time
  //TRACE( "RawData=" );
}

RawData::~RawData()
{
  //TRACE( "~RawData" );
}

#ifndef  _WCPPLIB_MESSAGE_H
#include "wcpplib\message.h"
#endif
//long pow10_array[3]={1,10,100};
long pow10_array[4]={1,10,100,1000}; // new range 1000. P.P. 12/11/09   back. P.P. 06/20/11
//long pow10_array[5]={1,10,100,1000,10000}; //range 10000. P.P. 02/15/11. here 04/01/11
#if 0
void RawData::ProcessRawData(uint16 data)
{
  long cur_autoscale;
  if((raw_data.time/data_rate) < analysis_time) {
    if(gain == -1) {     // if gain factor needs retrieving,...
      gain = (data & 0x7000) >> 12;
      cur_autoscale = 1;
    }
    else {
      int power = ((data & 0x7000) >> 12) - gain;
      if(power<0 || power >2) {  // if power out of range,...
	     // .the data is bogus somehow.
	//message( "Autorange of 10 to the %d, EOR: %X", (int)power, -1/*cur_end_of_run*/ );
	if(nErrorMesages) {
	  nErrorMesages--;
	  message( "Bogus data\nAutorange of 10 to the %d%s", power,
	    nErrorMesages? "": "\nAnother messages will be skiped" );
	}
	TRACE1( "Autorange of 10 to the %d", power );
	cur_autoscale = 0;
      }
      else {
	//replace for speed!  cur_autoscale = (long)pow10( power );
	cur_autoscale = pow10_array[power];
      }
    }

    // rentention_time measured in data_rate points/second.
    raw_data.time++;
    if((raw_data.time/data_rate) >= analysis_time) {
      acquire_finished = TRUE;
      raw_data.time = analysis_time * data_rate;
    }

    dq.Put((long)(data & 0x0fff) * cur_autoscale);
  }
}
#endif
#ifndef  _JOB_H     // added. P.P. 01/24/13
#include "job.h"
#endif

void RawData::ProcessData(uint32 data)
//void RawData::ProcessData(int32 data)     // test. P.P. 11/07/14
{
  if((raw_data.time/data_rate) < analysis_time) {
    if(job_file_version >=37) {     // added P.P. 01/24/13
      int32 power = 0; // P.P. 11/09/14
      if(gain == -1) {     // if gain factor needs retrieving,...
// added to retrive tge power. P.P. 11/09/14
//	power = (int32) ((data & 0x30000000) >> 28); // P.P. 11/07/14
	power = (uint32) ((data & 0x30000000) >> 28); // P.P. 11/13/14
	if(power<0 || power >3) {  // if power out of range,...
	     // .the data is bogus somehow.
	//message( "Autorange of 10 to the %d, EOR: %X", (int)power, -1/*cur_end_of_run*/ );
	  if(nErrorMesages) {
	    nErrorMesages--;
	    message( "Bogus data\nAutorange of 10 to the %d%s", power,
	      nErrorMesages? "": "\nAnother messages will be skiped" );
	  }
	  TRACE1( "Autorange of 10 to the %d", power );
//	  cur_autoscale = 0;
	  cur_autoscale = 1;
        }
	else {
          gain = (int) power; // P.P. 11/09/14
	//replace for speed!  cur_autoscale = (long)pow10( power );
	  cur_autoscale = pow10_array[power];
        }
// end
//        gain = (data & 0x70000000) >> 28;
//	gain = (data & 0x30000000UL) >> 28;   // P.P. 11/07/14
//	gain = (int) ((data & 0x30000000UL) >> 28);   // P.P. 11/09/14
//	cur_autoscale = 1;
//      cur_autoscale = pow10_array[gain]; // we need it right from start. P.P. 12/28/12
      }
      else {
//      int32 power = ((data & 0x70000000) >> 28) - gain;
//        int32 power = ((data & 0x70000000) >> 28); // testing. P.P. 12/26/12
//	power = (int32) ((data & 0x30000000) >> 28); // P.P. 11/09/14
	power = (uint32) ((data & 0x30000000) >> 28); // P.P. 11/13/14
	if(power<0 || power >3) {  // if power out of range,...
	     // .the data is bogus somehow.
	//message( "Autorange of 10 to the %d, EOR: %X", (int)power, -1/*cur_end_of_run*/ );
	  if(nErrorMesages) {
	    nErrorMesages--;
	    message( "Bogus data\nAutorange of 10 to the %d%s", power,
	      nErrorMesages? "": "\nAnother messages will be skiped" );
	  }
	  TRACE1( "Autorange of 10 to the %d", power );
//	  cur_autoscale = 0;
	  cur_autoscale = 1;
        }
	else {
	//replace for speed!  cur_autoscale = (long)pow10( power );
	  cur_autoscale = pow10_array[power];
	}
      }
    }
    else {  // as it was always. P.P. 01/24/13
      cur_autoscale = 1;
    }
/*
  if((raw_data.time/data_rate) < analysis_time) {
    if(gain == -1) {     // if gain factor needs retrieving,...
      //??  asm int 3 // Debug Trap
    }
*/
    // rentention_time measured in data_rate points/second.
    raw_data.time++;
    uint cur_time=(raw_data.time/data_rate);
    if( cur_time >= analysis_time) {
      acquire_finished = TRUE;
      raw_data.time = analysis_time * data_rate;
    }
    // rem to test constant segment width. P.P. 06/30/17 and restored.
    #if !OPTICS()
    // Increase segment after some time
    if(cur_time > analysis_time/3)
      dq.SetMultipleSegment(2);
    else if(cur_time > analysis_time/2)
      dq.SetMultipleSegment(3);
    #endif

//    dq.Put(data);
    int16 data_16 = 0;
    if(job_file_version >=37) {     // added P.P. 01/24/13
//      dq.Put((long)(data & 0x8000FFFF) * cur_autoscale);
//      dq.Put((long)(data & 0x80007FFF) * cur_autoscale);  // P.P. 11/07/14
//      int32 data_16 = data & 0x80007FFF;
      int32 data_32 = 0;  // P.P. 12/17/14
      uint16 data_u16 = (uint16) (((uint32) data)&0x0000FFFFUL); // P.P. 11/13/14
//      int16 data_16 = (uint16) (data & 0x0000FFFF);
//      data_16 = (int16) data_u16; // rem P.P. 12/12/14
      if (data_u16<32768) {
	 data_16 = data_u16;  // P.P. 12/12/14
      }
      else {
	 data_16 = - (~data_u16+1);
      }
      data_32 = data_16;               // P.P. 12/17/14
      data_32 *= cur_autoscale;        //
//      dq.Put((long) (((int16) data_16) * cur_autoscale));  // P.P. 11/09/14
//      dq.Put((long) (data_16 * cur_autoscale));  // P.P. 11/09/14
      dq.Put(data_32);  // P.P. 12/17/14
    }
    else {   // P.P. 01/24/13
    dq.Put(data);
    }
  }
}



