// GCProjection.h: interface for the CGCProjection class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GCPROJECTION_H__4E918A32_4499_4E6C_9135_933B964C00C8__INCLUDED_)
#define AFX_GCPROJECTION_H__4E918A32_4499_4E6C_9135_933B964C00C8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Reference.h"

class CGCProjection  
{
	typedef struct _tagEllipsoid
	{
		double a,b;
		double e,e2;
	}Ellipsoid;

public:
	CGCProjection();
	virtual ~CGCProjection();

	HRESULT Geocentric2Geodetic(double X, double Y, double Z, double *pLat, double *pLon, double *pAltitude);
	HRESULT Geodetic2Geocentric(double latitude, double longitude, double altitude, double *pX, double *pY, double *pZ);
	HRESULT SetDatumPara(double a, double b);

protected:
	BOOL TimDEqual(double value1,double value2,double tolerance);
private:
	Ellipsoid m_Ellipsoid;
};

#endif // !defined(AFX_GCPROJECTION_H__4E918A32_4499_4E6C_9135_933B964C00C8__INCLUDED_)
