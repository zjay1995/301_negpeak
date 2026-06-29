#pragma hdrstop

#ifndef  _PEAK_H
#include "peak.h"
#endif

#include "debug.h"

Peak::Peak()
 : Num(-1),
   pComponent(0),
   Time(0),
   Area(0),
   Height(0),
   From(0),
   To(0)
   , _Time(0)
   , _Height(0)
   , _HalfWidth(0)
   , multiplier(1)  // added. P.P. 01/11/13
{
//  TRACE1("Peak=%p",this);
}

Peak::Peak(const Peak &peak)
 : Num(peak.Num),
   pComponent(peak.pComponent),
   Time(peak.Time),
   Area(peak.Area),
   Height(peak.Height),
   From(peak.From),
   To(peak.To)
   , _Time(peak._Time)
   , _Height(peak._Height)
   , _HalfWidth(peak._HalfWidth)
   , multiplier(peak.multiplier) // added. P.P. 01/11/13
{
//  TRACE2("Peak(%p)=%p",&peak,this);
}


Peak::~Peak()
{
//  TRACE1("~Peak=%p",this);
}


