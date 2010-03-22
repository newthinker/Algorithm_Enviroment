// PatternMatch.h: interface for the CPatternMatch class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PATTERNMATCH_H__3D74C6F9_F0D1_423E_8C80_C4B20140CD1F__INCLUDED_)
#define AFX_PATTERNMATCH_H__3D74C6F9_F0D1_423E_8C80_C4B20140CD1F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Kernel.h"

class CPatternMatch  
{
public:
	CPatternMatch();
	virtual ~CPatternMatch();

	HRESULT PatternMatch(BYTE* pImage,int nImgRows,int nImgCols,double fImgScanSize,BYTE* pPattern,int nPtnRows,int nPtnCols,double fPtnScanSize,UINT nDataType,double* px,double* py,double* pCoef);

protected:
	BOOL Resample(BYTE* pSrcImage,int nSrcRows,int nSrcCols,UINT nDataType,double lfSrcScanSize,double lfDstScanSize,BYTE** ppDstImage,int* pnDstRows,int* pnDstCols);
	BOOL Match(BYTE* pImage,int nImgRows,int nImgCols,BYTE* pPattern,int nPtnRows,int nPtnCols,UINT nDataType,int nSearchX0,int nSearchY0,int nSearchHeight,int nSearchWidth,double* pX,double* pY,double* pCoef,BOOL bInSubPixel);
	BOOL Pyramid(BYTE* pSrcImage,int nSrcRows,int nSrcCols,UINT nDataType,int nPrdLayer,BYTE** ppDstImage,int* pnDstRows,int* pnDstCols);
};

#endif // !defined(AFX_PATTERNMATCH_H__3D74C6F9_F0D1_423E_8C80_C4B20140CD1F__INCLUDED_)
