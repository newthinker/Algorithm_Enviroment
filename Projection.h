// Projection.h: interface for the CProjection class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PROJECTION_H__B48F03B7_0804_4227_B3FF_A5E513A2630E__INCLUDED_)
#define AFX_PROJECTION_H__B48F03B7_0804_4227_B3FF_A5E513A2630E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Reference.h"
#include "TMProjection.h"
#include "GCProjection.h"

class CProjection  
{
public:
	CProjection();
	virtual ~CProjection();

	virtual HRESULT SetFalseNorthing(double lfFalseNorthing)=0;
	virtual HRESULT GetFalseNorthing(double* plfFalseNorthing)=0;
	virtual HRESULT SetFalseEasting(double lfFalseEasting)=0;
	virtual HRESULT GetFalseEasting(double* plfFalseEasting)=0;
	virtual HRESULT GeocentricFromWGS84(double GX84,double GY84,double GZ84,double* pGX,double* pGY,double* pGZ)=0;
	virtual HRESULT GeocentricToWGS84(double GX,double GY,double GZ,double* pGX84,double* pGY84,double* pGZ84)=0;
	virtual HRESULT GetZoneStep(long* pZoneStep)=0;
	virtual HRESULT SetZoneStep(long nZoneStep)=0;
	virtual HRESULT CreateProject(char* pszProjectName)=0;
	virtual HRESULT GetProjectName(char* pszName)=0;
	virtual HRESULT SetZone(LONG nZone)=0;
	virtual HRESULT GetZone(LONG * pZone)=0;
	virtual HRESULT Map2Geodetic(DOUBLE Easting, DOUBLE Northing, DOUBLE * pLatitude, DOUBLE * pLongitude)=0;
	virtual HRESULT Geodetic2Geocentric(DOUBLE Latitude, DOUBLE Longitude, DOUBLE Altitude, DOUBLE * pGX, DOUBLE * pGY, DOUBLE * pGZ)=0;
	virtual HRESULT Geocentric2Geodetic(DOUBLE GX, DOUBLE GY, DOUBLE GZ, DOUBLE * pLatitude, DOUBLE * pLongitude, DOUBLE * pAltitude)=0;
	virtual HRESULT Geodetic2Map(DOUBLE Latitude, DOUBLE Longitude, DOUBLE * pX, DOUBLE * pY)=0;
	virtual HRESULT CalcZone(DOUBLE Latitude, DOUBLE Longitude, LONG * pZone)=0;
	virtual HRESULT SetHemisphere(char szHemisphere)=0;
	virtual HRESULT GetCentralMeridian(double* pLongitude)=0;
};

#endif // !defined(AFX_PROJECTION_H__B48F03B7_0804_4227_B3FF_A5E513A2630E__INCLUDED_)
