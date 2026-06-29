#ifndef  _DEBUG_H
#define  _DEBUG_H

void assertfail( char *__msg,char *__cond,char *__file,int __line);


#ifdef ASSERT
#undef ASSERT
#endif

#ifdef TRACE1
#undef TRACE
#undef TRACE1
#undef TRACE2
#undef TRACE3
#undef TRACE4
#undef TRACE5
#undef TRACE6
#endif

#define _NDEBUG

#ifdef _NDEBUG
#  define ASSERT(p) ((void)0)
#  define TRACE(f)                    ((void)0)
#  define TRACE1(f,v)                 ((void)0)
#  define TRACE2(f,v1,v2)             ((void)0)
#  define TRACE3(f,v1,v2,v3)          ((void)0)
#  define TRACE4(f,v1,v2,v3,v4)       ((void)0)
#  define TRACE5(f,v1,v2,v3,v4,v5)    ((void)0)
#  define TRACE6(f,v1,v2,v3,v4,v5,v6) ((void)0)
#else
#  ifdef _Windows
#    define _ENDL
#  else
#    define _ENDL "\n"
#  endif
#  define ASSERT(p) ((p) ? (void)0 : (void) assertfail( \
		    "Assertion failed: %s, file %s, line %d" _ENDL, \
		    #p, __FILE__, __LINE__ ) )

/*static */void MyMemMessage( const char *fmt, ... );
#  define TRACE(f)                    do { static const char _far msg[]=f; MyMemMessage( msg ); } while(0) // MyMemMessage( msg );
#  define TRACE1(f,v)                 do { static const char _far msg[]=f; MyMemMessage( msg, v ); } while(0) // MyMemMessage( f,v );
#  define TRACE2(f,v1,v2)             MyMemMessage( f,v1,v2 )
#  define TRACE3(f,v1,v2,v3)          MyMemMessage( f,v1,v2,v3 )
#  define TRACE4(f,v1,v2,v3,v4)       MyMemMessage( f,v1,v2,v3,v4 )
#  define TRACE5(f,v1,v2,v3,v4,v5)    MyMemMessage( f,v1,v2,v3,v4,v5 )
#  define TRACE6(f,v1,v2,v3,v4,v5,v6) MyMemMessage( f,v1,v2,v3,v4,v5,v6 )
#endif

#endif


