#include "includes.h"
#pragma hdrstop

#ifndef  _SGFILTER_H
#include "sgfilter.h"
#endif
#ifdef FILTER

inline int32 sqr(int32 x) { return x*x;  }

int storcoeff(int32 tmpcoef[],int32 *tmpnorm,int fnum,int flag,int wtflg);

  // indata & odata cannot overlap
  // fnum >=5 && fnum <= 25, must be odd number
  // flag = 0,1,2  --  smooth, 1st, and 2nd coef.
  // wtflg - flag to calculate the weight function (squared)
int sgfilter(int32 huge *indata,int32 huge *odata,int ndata,int fnum,int flag,int wtflg)
{
  if(!(fnum % 2))
     fnum += 1;                 // must be odd number
  int halfw = fnum / 2;             // half the filter number - 1

  /*  All the coefficients are stored in a two dimensional array.
	      cindx is the offset to the set of coefficients specified.
	      flag is the number of the derivative desired.
	      fnum is the number of points to use in the filter.
	      storcoef() stores and retrieves the coefficients.
      */

  int32  tmpcoef[25], tmpnorm;
  if(!storcoeff(tmpcoef,&tmpnorm,fnum,flag,wtflg))
      return(0);

  int i=0;
  // Put unfiltered data into out buffer
  for(; i<halfw; i++)
    odata[i] = indata[i];

  if(tmpnorm) {                    // Check for divide by zero */
    int32 half_tmpnorm=tmpnorm/2;
     // Filter data, first and last halfw points are not filtered
    for(int j=0; i<ndata-halfw; i++, j++) {
      int32 sum = 0;
      for(int k=0; k<fnum; k++)
	sum += indata[j+k] * tmpcoef[k]; // j+k== i-halfw+k;
      if(sum<0) sum=0;
      odata[i] = (sum + half_tmpnorm)/tmpnorm;
    }
  }

  // Put unfiltered data into out buffer
  for(; i<ndata; i++)
    odata[i] = indata[i];

//  for(i=0; i<ndata; i++)
//    if(odata[i]<0)
//      odata[i]=0;

  return 1;
}

static int storcoeff(int32 tmpcoef[],int32 *tmpnorm,int fnum,int flag,int wtflg)
{
  // coefficient array
  static int32 coef[33][13] = {
       {  -3,     12,     17},                    // smoothing
       {  -2,     3,      6,      7},
       {  -21,    14,     39,     54,     59},
       {  -36,    9,      44,     69,     84,     89},
       {  -11,    0,      9,      16,     21,     24,   25},
       {  -78,    -13,    42,     87,     122,    147,   162,   167},
       {  -21,    -6,     7,      18,     27,     34,     39,    42,   43},
       {  -136,   -51,    24,     89,     144,    189,   224,   249,   264,  269},
       {  -171,   -76,    9,      84,     149,    204,   249,   284,   309,   324,   329},
       {  -42,    -21,    -2,     15,     30,     43,     54,    63,    70,    75,     78,     79},
       {  -253,   -138,   -33,    62,     147,    222,   287,   322,   387,   422,    447,    462,    467},
       {  1,      -8,     0},             // 1st derivative
       {  22,     -67,    -58,    0},
       {  86,     -142,   -193,   -126,   0},
       {  300,    -294,   -532,   -503,   -296,   0},
       {  1133,   -660,   -1578,  -1796,  -1489,  -832,      0},
       {  12922,  -4121,  -14150, -18334, -17842, -13843, -7506,     0},
       {  748,    -98,    -643,   -930,   -1002,  -902,   -673,    -358,      0},
       {  6936,   68,     -4648,  -7481,  -8700,  -8574,   -8179,  -5363,  -2816,      0},
       {  84075,  10032,  -43284, -78176, -96947, -101900,-95338, -79564, -56881,  -29592,     0},
       {  3938,   815,    -1518,  -3140,  -4130,  -4567,   -4530,  -4098,  -3350,   -2365,  -1222,     0},
       {  30866,  8602,   -8525,  -20982, -29236, -33754, -35003, -33450, -29562,  -23806,  -16649, -8558,    0},
       {  2,      -1,     -2},            // 2nd derivative
       {  5,      0,      -3,     -4},
       {  28,     7,      -8,     -17,    -20},
       {  15,     6,      -1,     -6,     -9,     -10},
       {  22,     11,     2,      -5,     -10,    -13,   -14},
       {  91,     52,     19,     -8,     -29,    -48,   -53,   -56},
       {  40,     25,     12,     1,      -8,     -15,   -20,   -23,   -24},
       {  51,     34,     19,     6,      -5,     -14,   -21,   -26,   -29,   -30},
       {  190,    133,    82,     37,     -2,     -35,   -62,   -83,   -98,   -107,   -110},
       {  77,     56,     37,     20,      5,      -8,   -19,   -28,   -35,    -40,    -43,    -44},
       {  92,     69,     48,     29,     12,      -3,   -16,   -27,   -36,    -43,    -48,    -51,    -52}
  };

   // normalization constant
  static int32 norm[33] = { // [3][11] - [flag][fnum-5];
				       // Smoothing
       35,  21,  231,  429,  143,   1105,   323,   2261,    3059,     805,    5135,
				       // 1st derivative
       12, 252, 1188, 5148, 24024, 334152, 23256, 260658, 3634092, 197340, 1776060,
				       // 2nd derivative
       7,   42,  462,  429,  1001,   6188,  3876,   6783,   33649,  17710,   26910};

  int sign;
  if(flag == 1)   // 1st derivative coef. symetric except for sign
    sign = -1;
  else
    sign = 1;
  int halfw = fnum / 2;          // half the filter number - 1
    /*  All the coefficients are stored in a two dimensional array.
       cindx is the offset to the set of coefficients specified.
       flag is the number of the derivative desired.
       fnum is the number of points to use in the filter.
       storcoef() stores and retrieves the coefficients.
    */
  // position of smooth, 1st, and 2nd coef.
  int cindx = (fnum - 5) / 2 + (flag * 11);   // table size dependent
  if(fnum < 5 || fnum > 25) {              /* input error */
    // printf("\nError in storcoeff()\n");
    return 0;
  }
  if(flag < 0 || flag > 2)
    return 0;  // input error

  // if(cindx < 0 || cindx >= 33)
  // return 0;  // input error

  // establish coefficients
  if(wtflg) {                      // calculate the weight function (squared)
    for(int k=0;k<halfw+1;k++)    // first half of coefficients
      tmpcoef[k] = sqr(coef[cindx][k]); //coef[cindx][k] * coef[cindx][k];
    for(       ;k<fnum   ;k++)    // second half of coefficients
      tmpcoef[k] = sqr(coef[cindx][fnum-k-1]); //coef[cindx][fnum-k-1] * coef[cindx][fnum-k-1];
    *tmpnorm = sqr(norm[cindx]); //*(norm+cindx) * *(norm+cindx);
  }
  else {                      // calculate the normal function
    for(int k=0;k<halfw+1;k++)          // first half of coefficients
      tmpcoef[k] = coef[cindx][k];
    for(       ;k<fnum   ;k++)       // second half of coefficients
      tmpcoef[k] = sign * coef[cindx][fnum-k-1];
    *tmpnorm = norm[cindx];// *(norm+cindx);
  }

  return 1;
}

#endif//FILTER
