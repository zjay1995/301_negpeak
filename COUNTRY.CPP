#pragma hdrstop

#ifndef __WINDOWS_H
#include <windows.h>
#endif

#ifndef __TIME_H
#include <time.h>
#endif
#ifndef __STRING_H
#include <string.h>
#endif

#ifndef  _COUNTRY_H
#include "country.h"
#endif

static char *SWeekday[7] = {
  "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
  };

static char *LWeekday[7] = {
  "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"
  };

static char *SMonth[12] = {
  "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
  };

static char *LMonth[12] = {
  "January", "February", "March",     "April",   "May",      "June",
  "July",    "August",   "September", "October", "November", "December"
  };

const char *PrintDate(const time_t &timer, int bPrintYear, int bConvertToLocal)
{
  struct tm *adjtime = bConvertToLocal? localtime( &timer ): gmtime( &timer );
  char fmt[20];
  GetProfileString("Intl","sShortDate","m/d/yy",fmt,sizeof(fmt));
  static char str[20];
  char *pSrc=fmt;
  char *pDst=str;
  int m,d,y;
  m=d=y=0;

  if(!bPrintYear) {
    char *cp=strpbrk(fmt,"yY");
    if(cp) {
      if(cp>fmt) {
	char *p=cp-1; // skip delim
	while(*cp=='y' || *cp=='Y') cp++;
	while(*cp) *(p++)=*(cp++);
	*p='\0';
      }
      else {
	char *p=cp;
	while(*cp=='y' || *cp=='Y') cp++;
	if(*cp) cp++; // skip delim
	while(*cp) *(p++)=*(cp++);
	*p='\0';
      }
    }
  }

  for( ; ; pSrc++) {
    switch(*pSrc) {
      case 'd': case 'D':  d++; break;
      case 'm': case 'M':  m++; break;
      case 'y': case 'Y':  y++; break;
      default:
	if(d==1) {
	  if(adjtime->tm_mday/10)
	  *(pDst++)=adjtime->tm_mday/10+'0';
	  *(pDst++)=adjtime->tm_mday%10+'0';
	}
	else if(d==2) {
	  *(pDst++)=adjtime->tm_mday/10+'0';
	  *(pDst++)=adjtime->tm_mday%10+'0';
	}
	else if(d==3) {
	  pDst=stpcpy(pDst,SWeekday[adjtime->tm_wday]);
	}
	else if(d>=4) {
	  pDst=stpcpy(pDst,LWeekday[adjtime->tm_wday]);
	}
	else if(m==1) {
	  if((adjtime->tm_mon+1)/10)
	  *(pDst++)=(adjtime->tm_mon+1)/10+'0';
	  *(pDst++)=(adjtime->tm_mon+1)%10+'0';
	}
	else if(m==2) {
	  *(pDst++)=(adjtime->tm_mon+1)/10+'0';
	  *(pDst++)=(adjtime->tm_mon+1)%10+'0';
	}
	else if(m==3) {
	  pDst=stpcpy(pDst,SMonth[adjtime->tm_mon]);
	}
	else if(m>=4) {
	  pDst=stpcpy(pDst,LMonth[adjtime->tm_mon]);
	}
	else if(y>=2) {
	  *(pDst++)=adjtime->tm_year/10%10+'0';
	  *(pDst++)=adjtime->tm_year%10+'0';
	}
	else if(y>=4) {
	  int tm_year=1900+adjtime->tm_year;
	  *(pDst++)=tm_year/1000+'0';
	  *(pDst++)=tm_year/100%10+'0';
	  *(pDst++)=tm_year/10%10+'0';
	  *(pDst++)=tm_year%10+'0';
	}
	*(pDst++)=*pSrc;
	m=d=y=0;
	if(!*pSrc) goto end;
      break;
    }
  }
 end:
  *pDst='\0';
  return str;
}

const char *PrintTime(const time_t &timer, int bConvertToLocal)
{
  static char str[40];
  struct tm *adjtime = bConvertToLocal? localtime( &timer ): gmtime( &timer );
  int iTime=GetProfileInt("Intl","iTime",0);
  char sTime[10]; GetProfileString("Intl","sTime",":",sTime,sizeof(sTime));
  char s1159[10]; GetProfileString("Intl","s1159","",s1159,sizeof(s1159));
  char s2359[10]; GetProfileString("Intl","s2359","",s2359,sizeof(s2359));
  int iTLZero=GetProfileInt("Intl","iTLZero",0);

  wsprintf(str,iTLZero?"%.2d%s%.2d%s": "%d%s%.2d%s",
    (iTime==0 && adjtime->tm_hour>12)? adjtime->tm_hour-12: adjtime->tm_hour,
    sTime,adjtime->tm_min,(adjtime->tm_hour<12 && iTime==0)? s1159: s2359
  );
  return str;
}

#include <stdlib.h>

void ReadDate(const char *str, int *year, int *mon, int *mday)
{
  *year=0;
  *mon=0;
  *mday=0;
  char fmt[20];
  GetProfileString("Intl","sShortDate","m/d/yy",fmt,sizeof(fmt));

  { // Read american files in Moscow
    //if(strchr(str,'.')) strcpy(fmt,"d.m.yy");
    if(strchr(str,'/')) strcpy(fmt,"m/d/yy");
  }
  strlwr(fmt);
  // mdy - America
  // dmy - Europe
  // ymd - Japan

  enum {
    WaitValue,
    Value,
  } status(WaitValue);
  char value[200], *vp;
  int value_num=0;
  for(const char *cp=str; ; cp++) {
    switch(status) {
      case WaitValue:
	switch(*cp) {
	  case ' ': break;
	  default: vp=value; *(vp++)=*cp; status=Value; break;
	}
      break;
      case Value:
	switch(*cp) {
	  case '\0':
	  case ',':
	  case '.':
	  case '-':
	  case ':':
	  case '/':
	    *vp=0;
	    status=WaitValue;
	    //////////
	    switch(value_num) {
	      case 0:
		if(fmt[0]=='m') *mon=atoi(value);
		else if(fmt[0]=='d') *mday=atoi(value);
		else *year=atoi(value);
	      break;
	      case 1:
		if(fmt[0]=='d') *mon=atoi(value);
		else *mday=atoi(value);
	      break;
	      case 2:
		if(fmt[0]=='y') *mday=atoi(value);
		else *year=atoi(value);
	      break;
	    }
	    value_num++;
	  break;
	  default: *(vp++)=*cp; break;
	}
      break;
    }
    if(!*cp) break;
  }

  if(*year<80) *year+=2000;
  else if(*year<100) *year+=1900;
}
void ReadTime(const char *str, int *hour, int *min)
{
  *hour=0;
  *min=0;
  enum {
    WaitValue,
    Value,
  } status(WaitValue);
  char value[200], *vp;
  int value_num=0;
  for(const char *cp=str; ; cp++) {
    switch(status) {
      case WaitValue:
	switch(*cp) {
	  case ' ': break;
	  default: vp=value; *(vp++)=*cp; status=Value; break;
	}
      break;
      case Value:
	int bAdd=0;
	switch(*cp) {
	  case 'A':
	  case 'a':
	  case 'P':
	  case 'p':
	    bAdd=1;
	  case '\0':
	  case ',':
	  case '.':
	  case ':':
	    *vp=0;
	    status=WaitValue;
	    //////////
	    switch(value_num) {
	      case 0: *hour=atoi(value); break;
	      case 1: *min=atoi(value); break;
	      case 2: if((value[0]=='P') || (value[0]=='p')) {
			if(*hour<12) *hour+=12;
		      }
		      else {
			if(*hour>=12) *hour-=12;
		      }
	      break;
	    }
	    value_num++;
	    if(!bAdd) break;

	    vp=value; // Fall throught
	    status=Value;
	  default: *(vp++)=*cp; break;
	}
      break;
    }
    if(!*cp) break;
  }
}
