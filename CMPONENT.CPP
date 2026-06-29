#pragma hdrstop

#ifndef  _WCPPLIB_WSTREAM_H
#include "wcpplib\wstream.h"
#endif

#ifndef  _CMPONENT_H
#include "cmponent.h"
#endif

Component::Component()
 ://name("No name"),
  active_yn(0),
  peak_rt(0),
  window(1),

  resp_fact(1),

  ref_peak(0),
  slope(0)
{
  for(int p=0; p<MAX_POINTS; p++) {
    low_alarm[p]=0;
    high_alarm[p]=0;
  }
  for(int j=0; j<STAND_NUM; j++) stand[j]=0;

  { for(int j=0; j<STAND_NUM+1; j++) lstart[j]=lslope[j]=lvalue[j]=0.; }
}

Component::~Component()
{
}

Component &Component::operator =(const Component &src)
{
  name=src.name;
  active_yn=src.active_yn;
  peak_rt=src.peak_rt;
  window=src.window;

  resp_fact=src.resp_fact;
  memcpy(low_alarm,src.low_alarm,sizeof(low_alarm));
  memcpy(high_alarm,src.high_alarm,sizeof(high_alarm));

  ref_peak=src.ref_peak;

  for(int j=0; j<STAND_NUM; j++) stand[j]=src.stand[j];


  slope=src.slope;

  for(int j1=0; j1<=STAND_NUM; j1++) {
    lstart[j1]=src.lstart[j1];
    lslope[j1]=src.lslope[j1];
    lvalue[j1]=src.lvalue[j1];
  }

  return *this;
}

void *Component::read(iwstream &is, unsigned long version)
{
  name.read(is);
  is>>active_yn;
  is>>peak_rt;
  is>>window;

  is>>resp_fact;
  if(version<23) {
    is>>low_alarm[0];
    is>>high_alarm[0];
    for(int p=1; p<MAX_POINTS; p++) {
      low_alarm[p]=low_alarm[0];
      high_alarm[p]=high_alarm[0];
    }
  }
  else {
    is.read(low_alarm,sizeof(low_alarm));
    is.read(high_alarm,sizeof(high_alarm));
  }


  is>>ref_peak;
  for(int j=0; j<STAND_NUM; j++) is>>stand[j];
  return this;
}

void Component::write(owstream &os)
{
  name.write(os); // 2 byte - counter + counter bytes data
  os<<active_yn; // 2 bt
  os<<peak_rt;   // 4
  os<<window;    // 4

  os<<resp_fact;     // double
  os.write(low_alarm,sizeof(low_alarm));//os<<low_alarm;     // double
  os.write(high_alarm, sizeof(high_alarm));//os<<high_alarm;    // double

  os<<ref_peak;      // 2 bt
  for(int j=0; j<STAND_NUM; j++) os<<stand[j]; // double
}
