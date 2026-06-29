#pragma hdrstop


#ifndef  _WINUTIL_APP_H
#include "winutil\app.h"
#endif
/* ---------------------------- */

int message(LPCSTR fmt, ...)
{
  static int reenter=0;
  if(reenter) return -1;
  reenter=1;

  char str[200];			/* Buffer to build sting into	*/
  int cnt;				/* Result of VFPRINTF for return */

  cnt = wvsprintf( str, fmt, ... );	/* prints string to buffer	*/

  ::MessageBox(NULL,str,"Message",MB_OK|MB_ICONINFORMATION|MB_TASKMODAL);

  reenter=0;

  return cnt;				/* Return the conversion count	*/
}
