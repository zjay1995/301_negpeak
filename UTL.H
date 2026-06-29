#ifndef  _UTL_H
#define  _UTL_H


unsigned int ConvertTime( const char *time_str );


#ifndef  _TIME_T
#define  _TIME_T
typedef long time_t;
#endif
time_t ConvertTime( const char *date_str, const char *time_str );


const char *PrintSeconds(long l);


#endif
