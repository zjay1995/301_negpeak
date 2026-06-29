#ifndef  _PEAK_H
#define  _PEAK_H

class Component;

class Peak
{
 public:
  short Num;	                  // peak #, some peaks match mult. components
  Component *pComponent;	  // pointer to the component
  long Time;	                  // retention time for this peak
//  long Area;                      // area for this peak
  double Area;                      // to overcome problem with too large values of the area. P.P. 08/25/17
  long Height;                    // height for this peak

  long From;	                  // retention time for begin of this peak
  long To;	                  // retention time for end of this peak

  double _Height;
  double _HalfWidth;
  double _Time;

  Peak();
  Peak(const Peak &peak);
  virtual ~Peak();
  int multiplier; // P.P. 12/27/12
};

#endif
