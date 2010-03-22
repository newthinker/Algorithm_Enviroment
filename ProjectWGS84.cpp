// ProjectWGS84.cpp: implementation of the CProjectWGS84 class.
//
//////////////////////////////////////////////////////////////////////

#include "ProjectWGS84.h"
#include <string.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CProjectWGS84::CProjectWGS84()
{
	m_a=6378137.0;
	m_b=6356752.3142;
	m_dx=0;
	m_dy=0;
	m_dz=0;

	//��׼���ʼ��Ϊ������1��
	m_OriginLatitude=0;
	m_OriginLongitude=183;
	m_FalseEasting=500000;
	m_FalseNorthing=0;
	m_ScaleFactor=0.9996;
	m_szHemisphere='N';

	//zone parameters
	m_nZone=1;
	m_nZoneStep=6;
}

CProjectWGS84::~CProjectWGS84()
{

}

HRESULT CProjectWGS84::SetZoneStep(long nZoneStep)
{
	m_nZoneStep=nZoneStep;

	m_OriginLongitude=(m_nZoneStep*m_nZone+180)%360-m_nZoneStep/2.0;

	m_TmProjection.SetProjectionPara(m_OriginLongitude*PI/180.0,m_OriginLatitude*PI/180.0,m_FalseEasting,m_FalseNorthing,m_ScaleFactor);

	return S_OK;
}

HRESULT CProjectWGS84::GetZoneStep(long *pZoneStep)
{
	*pZoneStep=m_nZoneStep;

	return S_OK;
}

HRESULT CProjectWGS84::CreateProject(char* pszProjectName)
{
	m_TmProjection.SetDatumPara(m_a,m_b);
	m_GcProjection.SetDatumPara(m_a,m_b);

	return S_OK;
}

HRESULT CProjectWGS84::GetProjectName(char* pszName)
{
	if(pszName!=NULL)
	{
		strcpy(pszName,"WGS84 UTM");

		return S_OK;
	}

	return S_FALSE;
}

HRESULT CProjectWGS84::SetZone(LONG nZone)
{
	m_nZone=nZone;
	
	m_OriginLongitude=(m_nZoneStep*m_nZone+180)%360-m_nZoneStep/2.0;

	m_TmProjection.SetProjectionPara(m_OriginLongitude*PI/180.0,m_OriginLatitude*PI/180.0,m_FalseEasting,m_FalseNorthing,m_ScaleFactor);
	/*
	if (m_nZone >= 31)
	{
		m_OriginLongitude = ((6 * m_nZone - 183) * PI / 180.0 );//+ 0.00000005);
	}
	else
	{
		m_OriginLongitude = ((6 * m_nZone + 177) * PI / 180.0 );//+ 0.00000005);
	}*/

	return S_OK;
}

HRESULT CProjectWGS84::GetZone(LONG * pZone)
{
	*pZone=m_nZone;

	return S_OK;
}

HRESULT CProjectWGS84::Map2Geodetic(DOUBLE Easting, DOUBLE Northing, DOUBLE * pLatitude, DOUBLE * pLongitude)
{	
	return m_TmProjection.TM2Geodetic(Easting,Northing,pLatitude,pLongitude);
}

HRESULT CProjectWGS84::Geodetic2Geocentric(DOUBLE Latitude, DOUBLE Longitude, DOUBLE Altitude, DOUBLE * pGX, DOUBLE * pGY, DOUBLE * pGZ)
{
	return m_GcProjection.Geodetic2Geocentric(Latitude,Longitude,Altitude,pGX,pGY,pGZ);
}

HRESULT CProjectWGS84::Geocentric2Geodetic(DOUBLE GX, DOUBLE GY, DOUBLE GZ, DOUBLE * pLatitude, DOUBLE * pLongitude, DOUBLE * pAltitude)
{
	return m_GcProjection.Geocentric2Geodetic(GX,GY,GZ,pLatitude,pLongitude,pAltitude);
}

HRESULT CProjectWGS84::Geodetic2Map(DOUBLE Latitude, DOUBLE Longitude, DOUBLE * pX, DOUBLE * pY)
{
	return m_TmProjection.Geodetic2TM(Latitude,Longitude,pX,pY);
}

HRESULT CProjectWGS84::CalcZone(DOUBLE Latitude, DOUBLE Longitude, LONG * pZone)
{
	long Lat_Degrees;
	long Long_Degrees;
	long temp_zone;
	long Error_Code = UTM_NO_ERROR;

	if ((Latitude < MIN_LAT) || (Latitude > MAX_LAT))
	{ /* Latitude out of range */
		Error_Code |= UTM_LAT_ERROR;
	}
	if ((Longitude < -PI) || (Longitude > (2*PI)))
	{ /* Longitude out of range */
		Error_Code |= UTM_LON_ERROR;
	}
	if (!Error_Code)
	{ /* no errors */
		if (Longitude < 0)
			Longitude += (2*PI);
		Lat_Degrees = (long)(Latitude * 180.0 / PI);
		Long_Degrees = (long)(Longitude * 180.0 / PI);
		
		if (Longitude < PI)
			temp_zone = (long)(31 + ((Longitude * 180.0 / PI) / 6.0));
		else
			temp_zone = (long)(((Longitude * 180.0 / PI) / 6.0) - 29);
		if (temp_zone > 60)
			temp_zone = 1;
		/* UTM special cases */
		if ((Lat_Degrees > 55) && (Lat_Degrees < 64) && (Long_Degrees > -1)
			&& (Long_Degrees < 3))
			temp_zone = 31;
		if ((Lat_Degrees > 55) && (Lat_Degrees < 64) && (Long_Degrees > 2)
			&& (Long_Degrees < 12))
			temp_zone = 32;
		if ((Lat_Degrees > 71) && (Long_Degrees > -1) && (Long_Degrees < 9))
			temp_zone = 31;
		if ((Lat_Degrees > 71) && (Long_Degrees > 8) && (Long_Degrees < 21))
			temp_zone = 33;
		if ((Lat_Degrees > 71) && (Long_Degrees > 20) && (Long_Degrees < 33))
			temp_zone = 35;
		if ((Lat_Degrees > 71) && (Long_Degrees > 32) && (Long_Degrees < 42))
			temp_zone = 37;

		*pZone=temp_zone;

		return S_OK;
	}

	return S_FALSE;
}


HRESULT CProjectWGS84::GeocentricToWGS84(double GX, double GY, double GZ, double *pGX84, double *pGY84, double *pGZ84)
{
	*pGX84=GX;
	*pGY84=GY;
	*pGZ84=GZ;

	return S_OK;
}

HRESULT CProjectWGS84::GeocentricFromWGS84(double GX84, double GY84, double GZ84, double *pGX, double *pGY, double *pGZ)
{
	*pGX=GX84;
	*pGY=GY84;
	*pGZ=GZ84;

	return S_OK;
}

HRESULT CProjectWGS84::SetHemisphere(char szHemisphere)
{
	m_szHemisphere=szHemisphere;
	if(m_szHemisphere=='n'||m_szHemisphere=='N')
	{
		m_FalseNorthing=0;
	}
	else if(m_szHemisphere=='s'||m_szHemisphere=='S')
	{
		m_FalseNorthing=10000000;
	}
	else
	{
		return S_FALSE;
	}

	m_TmProjection.SetProjectionPara(m_OriginLongitude*PI/180.0,m_OriginLatitude*PI/180.0,m_FalseEasting,m_FalseNorthing,m_ScaleFactor);

	return S_OK;
}

HRESULT CProjectWGS84::GetCentralMeridian(double *pLongitude)
{
	*pLongitude=m_OriginLongitude;

	return S_OK;
}

HRESULT CProjectWGS84::GetFalseEasting(double *plfFalseEasting)
{
	*plfFalseEasting=m_FalseEasting;

	return S_OK;
}

HRESULT CProjectWGS84::SetFalseEasting(double lfFalseEasting)
{
	m_FalseEasting=lfFalseEasting;
	m_TmProjection.SetProjectionPara(m_OriginLongitude*PI/180.0,m_OriginLatitude*PI/180.0,m_FalseEasting,m_FalseNorthing,m_ScaleFactor);

	return S_OK;
}

HRESULT CProjectWGS84::GetFalseNorthing(double *plfFalseNorthing)
{
	*plfFalseNorthing=m_FalseNorthing;

	return S_OK;
}

HRESULT CProjectWGS84::SetFalseNorthing(double lfFalseNorthing)
{
	m_FalseNorthing=lfFalseNorthing;
	m_TmProjection.SetProjectionPara(m_OriginLongitude*PI/180.0,m_OriginLatitude*PI/180.0,m_FalseEasting,m_FalseNorthing,m_ScaleFactor);

	return S_OK;
}
