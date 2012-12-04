#ifndef _BASEDEFINE_
#define _BASEDEFINE_

#ifndef BYTE
#define BYTE unsigned char
#endif

#ifndef HRESULT
#define HRESULT unsigned int
#endif

#ifndef UINT
#define UINT unsigned int
#endif

#ifndef DWORD
#define DWORD unsigned long
#endif

#ifndef BOOL
#define BOOL unsigned int
#endif

#ifndef USHORT
#define USHORT unsigned short
#endif

#ifndef LONG
#define LONG long
#endif

#ifndef DOUBLE
#define DOUBLE double
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef S_OK
#define S_OK 0
#endif

#ifndef S_FALSE
#define S_FALSE 1
#endif

#ifndef NULL
#define NULL 0
#endif

#ifndef DBL_MAX
#define DBL_MAX 1.7976931348623158e+308
#endif

#ifndef __min
#define __min(a,b)  (((a) < (b)) ? (a) : (b))
#endif

#ifndef __max
#define __max(a,b)  (((a) > (b)) ? (a) : (b))
#endif

#define DEBUG

//#define ONE_BAND

#endif
