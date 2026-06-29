#pragma hdrstop

#ifndef __MATH_H
#include <math.h>
#endif
#ifndef __STDLIB_H
#include <stdlib.h>
#endif

#ifndef  _OZ_H
#include "oz.h"
#endif

void oz(double far *a,double far *b,int kmin,int kmax,int mm,int *k,double far *dl)
{
  int l1,l,k1,k2,j/*,m*/;
  double p,q,c;
    *dl=*b-*a;
    if(!(*dl)) *dl=1.;
    l1=log10(*dl);
    if(*dl>=1.0) l1=l1+1;
    p=pow10(l1+1);
    c=((*a)+*dl*0.5)/p;
    if(fabs(c)>1.e9) c*=p;
    else c=((long)c)*p;
    q=0.5;
    for(l=l1; l>-30; l--) {
      p=p*0.1;
      while(q>=0.5) {
	*dl=p*q;
	k2=(*b-c)/(*dl); if(*b>=c) k2=k2+1;
	k1=(*a-c)/(*dl); if(*a<c)  k1=k1-1;
	*k=k2-k1;
	if(*k>=kmax) goto l10;
//	if(*k>=kmin) goto l10;
	q=q*0.5;
      }
      q=2.0;
    }
    goto l40;
l10:for(; l<l1; l++) {
      while(q<=2.0) {
	*dl=p*q;
	k2=(*b-c)/(*dl); if(*b>=c) k2=k2+1;
	k1=(*a-c)/(*dl); if(*a<c) k1=k1-1;
	*k=k2-k1;
	if(*k<=kmax) goto l20;
	q=q*2.0;
      }
      q=0.5;
      p=p*10.;
    }
    goto l40;
l20:if(*k<kmin) {
      /*m=max(1,min(9,mm));*/
      j=(q==2.0)? 5: 2;
      /*goto (21,21,21,22,22,22,23,24,25),m;*/
l21:  if(k1>=0) *k=k1/j*j;
      else      *k=(k1-j+1)/j*j;
      if(k2-*k>kmin) goto l22/*(22,22,22,23,24,25),m*/;
      k1=*k;
      goto l25;
l22:  if(k2<0) *k=k2/j*j;
      else     *k=(k2+j-1)/j*j;
      if(*k-k1>kmin) goto l23/*(23,24,25,21,21,21),m*/;
      k2=*k;
      goto l24;
l23:  k2=(k2+k1+kmin)/2;
l24:  k1=k2-kmin;
l25:  k2=k1+kmin;
      *k=k2-k1;
    }
l40:
    *a=k1**dl+c;
    *b=k2**dl+c;
}

