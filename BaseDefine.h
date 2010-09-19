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

// 大小字节序判断 [ZuoW,2010/3/6]
const int endian = 1;
#define is_bigendian()  ((*(char*)&endian)==0)
#define is_littlendian() ((*(char*)&endian)==1)
#define LITTLENDIAN		1

#define	DEBUG		// debug标示 [ZuoW,2010/5/11]
#define		IMAGE_LD	// GCP像素坐标是左下角 [ZuoW,2010/7/4]
//#define APFLOAT		// 使用APFloat库进行精确计算 [ZuoW,2010/6/2]

//add by dy 20080925 for DE_ABERRATION define begin
/*
#define MAX_ABERRATION_RANGE 6000
#define DE_ABERRATION_COEF0 5.6843e-14
#define DE_ABERRATION_COEF1 0.99913f
#define DE_ABERRATION_COEF2 0.0f
#define DE_ABERRATION_COEF3 8.7929e-10
#define ABERRATION_CORRECT_VALUES(value) value-MAX_ABERRATION_RANGE
#define DE_ABERRATION(x) (DE_ABERRATION_COEF3*ABERRATION_CORRECT_VALUES(x)*ABERRATION_CORRECT_VALUES(x)*ABERRATION_CORRECT_VALUES(x) \
							+ DE_ABERRATION_COEF2*ABERRATION_CORRECT_VALUES(x)*ABERRATION_CORRECT_VALUES(x) \
							+ DE_ABERRATION_COEF1*ABERRATION_CORRECT_VALUES(x) \
							+ DE_ABERRATION_COEF0 \
							+ MAX_ABERRATION_RANGE)
#define DE_CONVERSE_ABERRATION_COEF0 4.5475e-013 
#define DE_CONVERSE_ABERRATION_COEF1 1.0001f 
#define DE_CONVERSE_ABERRATION_COEF2 0.0f
#define DE_CONVERSE_ABERRATION_COEF3 -7.8821e-010
#define DE_CONVERSE_ABERRATION(x) (DE_CONVERSE_ABERRATION_COEF3*ABERRATION_CORRECT_VALUES(x)*ABERRATION_CORRECT_VALUES(x)*ABERRATION_CORRECT_VALUES(x) \
									+ DE_CONVERSE_ABERRATION_COEF2*ABERRATION_CORRECT_VALUES(x)*ABERRATION_CORRECT_VALUES(x) \
									+ DE_CONVERSE_ABERRATION_COEF1*ABERRATION_CORRECT_VALUES(x) \
									+ DE_CONVERSE_ABERRATION_COEF0 \
									+ MAX_ABERRATION_RANGE)*/

//add by dy 20080925 for DE_ABERRATION define end
#endif
