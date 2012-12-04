// GcpMatch.h: interface for the CGcpMatch class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GCPMATCH_H__24250497_F0B9_4C03_B9D4_9E7E38FDDEFB__INCLUDED_)
#define AFX_GCPMATCH_H__24250497_F0B9_4C03_B9D4_9E7E38FDDEFB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Kernel.h"

class CGcpMatch  
{
public:
	CGcpMatch();
	virtual ~CGcpMatch();

	HRESULT Match(char* pszWarpFile,char* pszGcpFiles,double lfMatchWindow,double* pGX,double* pGY,double* px,double* py,int* pGcpNum,BOOL* bSucceeded);
protected:
	BOOL Moravec(BYTE* pBuffer,int nRows,int nCols,int* pPtRow,int* pPtCol);
	void Gray(BYTE* &pBuffer,int nRows,int nCols,int nBandNum,UINT datatype);
	void Gray(BYTE* &pBuffer, int nRows, int nCols, int nBandNum, int nSelBandNum);
	void Gray(BYTE* pMulBuffer, BYTE* &pBuffer, int nRows, int nCols, int nBandNum, int nSelBandNum, UINT datatype);
	int dnrml (double* aa,int n,double bb,double* a,double* b);
	void dsolve (double* a,double* b,double* x,int n,int wide);
	void dldltban1 (double* a,double* d,double* l,int n,int wide);
	void dldltban2 (double* l,double* d,double* b,double* x,int n,int wide);
};

#endif // !defined(AFX_GCPMATCH_H__24250497_F0B9_4C03_B9D4_9E7E38FDDEFB__INCLUDED_)
