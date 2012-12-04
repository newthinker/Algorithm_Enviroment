// RealVector.h: interface for the CRealVector class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_REALVECTOR_H__2B2681CE_80C2_4E74_AF5D_6481A4597518__INCLUDED_)
#define AFX_REALVECTOR_H__2B2681CE_80C2_4E74_AF5D_6481A4597518__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CRealVector  
{
public:
	void Normalize();
	CRealVector Add(CRealVector v2);
	CRealVector Sub(CRealVector v2);
	CRealVector operator=(CRealVector other);
	CRealVector CrossProduct(CRealVector v2);
	double DotProduct(CRealVector v2);
	CRealVector();
	virtual ~CRealVector();
public:
	double x,y,z;
};

#endif // !defined(AFX_REALVECTOR_H__2B2681CE_80C2_4E74_AF5D_6481A4597518__INCLUDED_)
