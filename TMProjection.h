// TMProjection.h: interface for the CTMProjection class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TMPROJECTION_H__0C9AB82F_1294_4FB5_BE70_1E4194E2CE78__INCLUDED_)
#define AFX_TMPROJECTION_H__0C9AB82F_1294_4FB5_BE70_1E4194E2CE78__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Reference.h"

class CTMProjection
{
public:
	CTMProjection();
	virtual ~CTMProjection();

	HRESULT TM2Geodetic(double Easting,double Northing,double *Latitude,double *Longitude);
	HRESULT SetDatumPara(double a, double b);
	HRESULT Geodetic2TM(double Latitude,double Longitude,double *Easting,double *Northing);
	HRESULT SetProjectionPara(double Origin_Lon, double Origin_Lat, double False_Easting, double False_Northing, double ScaleFactor);

protected:
	inline double SPHTMD(double Latitude,int index=0)
	{
		return ((double) (m_lfTM_ap * Latitude
			- m_lfTM_bp * sin(2.e0 * Latitude) + m_lfTM_cp * sin(4.e0 * Latitude) 
			- m_lfTM_dp * sin(6.e0 * Latitude) + m_lfTM_ep * sin(8.e0 * Latitude) ) );
	};

	inline double SPHSN(double Latitude,int index=0)
	{
		return ((double) (m_lfTM_a / sqrt( 1.e0 - m_lfTM_es * pow(sin(Latitude), 2))));
	};

	inline double SPHSR(double Latitude,int index=0)
	{
		return ((double) (m_lfTM_a * (1.e0 - m_lfTM_es) / pow(DENOM(Latitude), 3)));
	}

	inline double DENOM(double Latitude,int index=0)
	{
		return ((double) (sqrt(1.e0 - m_lfTM_es * pow(sin(Latitude),2))));
	}
private:
	// Datum Parameters
	double	m_lfTM_a;
	double	m_lfTM_b;

	double	m_lfTM_es;
	double	m_lfTM_ebs;
	double	m_lfTM_ap;
	double	m_lfTM_bp;
	double	m_lfTM_cp;
	double	m_lfTM_dp;
	double	m_lfTM_ep;

	// Projection Zone Parameters
	double	m_tmOrigin_Lat;
	double	m_tmOrigin_Long;
	double	m_tmFalse_Northing;
	double	m_tmFalse_Easting; 
	double	m_tmScale_Factor;

	double	m_tmDelta_Easting;
	double	m_tmDelta_Northing;

};

#endif // !defined(AFX_TMPROJECTION_H__0C9AB82F_1294_4FB5_BE70_1E4194E2CE78__INCLUDED_)
