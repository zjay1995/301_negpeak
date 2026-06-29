#ifndef  _CMPONENT_H
#define  _CMPONENT_H

#ifndef  _WCPPLIB_STR_H
#include "wcpplib\str.h"
#endif
class WCPPLIB_CLASS iwstream;
class WCPPLIB_CLASS owstream;

#ifndef  _CONSTS_H
#include "consts.h"
#endif

#ifndef  _INTERNAL_H
#include "internal.h" // MAX_POINTS
#endif

class Component {
 public:
  Str name;
  int active_yn;
  long peak_rt;
  long window;

  double resp_fact;
  double low_alarm[MAX_POINTS];
  double high_alarm[MAX_POINTS];

  int ref_peak;
  double stand[STAND_NUM];



  double slope; // relative to calibration. Not saved in file! Used by Integrator

  double lstart[STAND_NUM+1]; // Not saved in file! Used by Integrator
  double lslope[STAND_NUM+1]; // Not saved in file! Used by Integrator
  double lvalue[STAND_NUM+1]; // Not saved in file! Used by Integrator

  Component();
  ~Component();

  Component &operator =(const Component &);

  void *read(iwstream &is, unsigned long version);
  void write(owstream &os);
};

#endif
