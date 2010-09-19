// LeastSquareMatch.h: interface for the CLeastSquareMatch class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LEASTSQUAREMATCH_H__14F2E98E_05D1_47C9_91C6_CB6F2E81EED8__INCLUDED_)
#define AFX_LEASTSQUAREMATCH_H__14F2E98E_05D1_47C9_91C6_CB6F2E81EED8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Kernel.h"

class CLeastSquareMatch  
{
public:
	CLeastSquareMatch();
	virtual ~CLeastSquareMatch();

	HRESULT SinglePtMatch(BYTE* pG,int nGRows,int nGCols,CImage* pG2Image,double x0,double y0,double* px2,double* py2,double* pCoef);

protected:
	void dnrml (double* aa,int n,double bb,double* a,double* b);
	void pnrml(double *aa, int n, double bb, double *a, double *b, double p);
	void dsolve (double* a,double* b,double* x,int n,int wide);
	void dldltban1 (double* a,double* d,double* l,int n,int wide);
	void dldltban2 (double* l,double* d,double* b,double* x,int n,int wide);
	BOOL Pyramid(BYTE* pSrcImage,int nSrcRows,int nSrcCols,UINT nDataType,int nPrdLayer,BYTE** ppDstImage,int* pnDstRows,int* pnDstCols);
};

#endif // !defined(AFX_LEASTSQUAREMATCH_H__14F2E98E_05D1_47C9_91C6_CB6F2E81EED8__INCLUDED_)
