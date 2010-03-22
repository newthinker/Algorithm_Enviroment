// ProjectWGS84.h: interface for the CProjectWGS84 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PROJECTWGS84_H__26372A47_FB12_4415_A525_AD2F80BA5AFB__INCLUDED_)
#define AFX_PROJECTWGS84_H__26372A47_FB12_4415_A525_AD2F80BA5AFB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Projection.h"

class CProjectWGS84 : public CProjection
{
public:
	CProjectWGS84();
	virtual ~CProjectWGS84();

	HRESULT SetFalseNorthing(double lfFalseNorthing);
	HRESULT GetFalseNorthing(double* plfFalseNorthing);
	HRESULT SetFalseEasting(double lfFalseEasting);
	HRESULT GetFalseEasting(double* plfFalseEasting);
	HRESULT GeocentricFromWGS84(double GX84,double GY84,double GZ84,double* pGX,double* pGY,double* pGZ);
	HRESULT GeocentricToWGS84(double GX,double GY,double GZ,double* pGX84,double* pGY84,double* pGZ84);
	HRESULT GetZoneStep(long* pZoneStep);
	HRESULT SetZoneStep(long nZoneStep);
	HRESULT CreateProject(char* pszProjectName);
	HRESULT GetProjectName(char* pszName);
	HRESULT SetZone(LONG nZone);
	HRESULT GetZone(LONG * pZone);
	HRESULT Map2Geodetic(DOUBLE Easting, DOUBLE Northing, DOUBLE * pLatitude, DOUBLE * pLongitude);
	HRESULT Geodetic2Geocentric(DOUBLE Latitude, DOUBLE Longitude, DOUBLE Altitude, DOUBLE * pGX, DOUBLE * pGY, DOUBLE * pGZ);
	HRESULT Geocentric2Geodetic(DOUBLE GX, DOUBLE GY, DOUBLE GZ, DOUBLE * pLatitude, DOUBLE * pLongitude, DOUBLE * pAltitude);
	HRESULT Geodetic2Map(DOUBLE Latitude, DOUBLE Longitude, DOUBLE * pX, DOUBLE * pY);
	HRESULT CalcZone(DOUBLE Latitude, DOUBLE Longitude, LONG * pZone);
	HRESULT SetHemisphere(char szHemisphere);
	HRESULT GetCentralMeridian(double* pLongitude);

private:
	//datum parameters
	double m_a;
	double m_b;
	double m_dx;
	double m_dy;
	double m_dz;
	
	//porjection parameters
	double m_OriginLatitude;
	double m_OriginLongitude;
	double m_FalseEasting;
	double m_FalseNorthing;
	double m_ScaleFactor;
	char m_szHemisphere;

	//zone parameters
	int m_nZone;
	int m_nZoneStep;

	CTMProjection m_TmProjection;
	CGCProjection m_GcProjection;
};

#endif // !defined(AFX_PROJECTWGS84_H__26372A47_FB12_4415_A525_AD2F80BA5AFB__INCLUDED_)
