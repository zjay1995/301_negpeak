#pragma hdrstop

#ifndef  _IND_H
#include "ind.h"
#endif

#ifndef __MEM_H
#include <mem.h>
#endif


Indexes::Indexes() : p(0), n(0) {}
Indexes::Indexes(const Indexes &ind) : p(0), n(0) { *this=ind; }
Indexes::~Indexes()
{
  Alloc(0);
}
void Indexes::operator=(const Indexes &ind)
{
  Alloc(ind.n); memcpy(p,ind.p,n*sizeof(*p));
}
void Indexes::Alloc(int _n)
{
  delete []p; p=0==_n?NULL:new int[_n];
  n=p? _n: 0;
}


int Indexes::contain(int _i) const
{
  for(int i=0; i<n; i++) {
    if(p[i]==_i) return 1;
  }
  return 0;
}

