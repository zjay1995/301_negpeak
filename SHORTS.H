#ifndef  _SHORTS_H
#define  _SHORTS_H

typedef   signed char  int8;
typedef unsigned char uint8;

typedef   signed int   int16;
typedef unsigned int  uint16;

typedef   signed long  int32;
typedef unsigned long uint32;

typedef unsigned char   uchar;
typedef unsigned short  ushort;
typedef unsigned int    uint;
typedef unsigned long   ulong;

#if !defined _Windows && !defined _WINDOWS
typedef char            BOOL;
#else
typedef int                 BOOL;
#define FALSE               0
#define TRUE                1
#endif




#endif
