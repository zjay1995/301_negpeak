#pragma hdrstop

#ifndef __STRING_H
#include <string.h>
#endif
#ifndef __STDLIB_H
#include <stdlib.h>
#endif
#ifndef  _UTL_H
#include "utl.h"
#endif
#ifndef  _WCPPLIB_STR_H
#include "wcpplib\str.h"
#endif

unsigned int ConvertTime( const char *time_str )
{
  static const char delim[]=":., ";
  unsigned int retValue = 0;
  Str buff=time_str;
  if(!buff.IsEmpty()) {
    for(char *cp=strtok(buff.GetPtr(),delim); cp; cp=strtok(NULL,delim)) {
      retValue=retValue*60+atoi(cp);
    }
  }
  return retValue;
}

#ifndef __TIME_H
#include <time.h>
#endif
time_t ConvertTime( const char *date_str, const char *time_str )
{
  static const char delim[]="\\/-.";
  struct tm t;
  memset(&t,0,sizeof(t));
  Str buff=date_str;
  if(!buff.IsEmpty()) {
    int i=0;
    for(char *cp=strtok(buff.GetPtr(),delim); cp; cp=strtok(NULL,delim),i++) {
      switch(i) {
	case 0: t.tm_mon=atoi(cp); break;
	case 1: t.tm_mday=atoi(cp); break;
	case 2: t.tm_year=atoi(cp); break;
      }
    }
  }
  t.tm_sec=ConvertTime( time_str );
  return mktime(&t);
}

#ifndef __WINDOWS_H
#include <windows.h>
#endif

#include "mainwind.h"
#include <stdio.h>
const char *PrintSeconds(long l)
{
  #if OPTICS()
  extern double _dx_;
  extern double _xmin_;
  static char str[45];
  sprintf(str,"%8.1f",_xmin_+l*_dx_);
  #else
  static char str[25];
  char sTime[10]; GetProfileString("Intl","sTime",":",sTime,sizeof(sTime));
  if(l/3600==0) wsprintf(str, "%ld%s%.2d",l/60,sTime,(int)(l%60));
  else wsprintf(str, "%ld%s%.2d%s%.2d",l/3600,sTime,(int)(l/60%60),sTime,(int)(l%60));
  #endif
  return str;
}
