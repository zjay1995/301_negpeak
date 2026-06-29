#ifndef  _COUNTRY_H
#define  _COUNTRY_H

const char *PrintDate(const time_t &timer, int bPrintYear=1, int bConvertToLocal=1);
const char *PrintTime(const time_t &timer, int bConvertToLocal=1);


#include <time.h>
void ReadDate(const char *str, int *year, int *mon, int *mday);
void ReadTime(const char *str, int *hour, int *min);

#endif
