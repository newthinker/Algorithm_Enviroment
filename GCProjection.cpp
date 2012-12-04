// GCProjection.cpp: implementation of the CGCProjection class.
//
//////////////////////////////////////////////////////////////////////

#include "GCProjection.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGCProjection::CGCProjection()
{
	SetDatumPara(6378137.0,6356752.3142);
}

CGCProjection::~CGCProjection()
{

}

#define TIM_LATITUDE_COMPUTING_PRECISION 1.e-11 /* computing tolerance */
#define TIM_MAX_ITERATION_NUMBER         100    /* max iteration number */

BOOL CGCProjection::TimDEqual(double value1,double value2,double tolerance)
{
	if(fabs(value1-value2)<tolerance)
	{
		return TRUE;
	}
	return FALSE;
}

HRESULT CGCProjection::SetDatumPara(double a, double b)
{
	m_Ellipsoid.a=a;
	m_Ellipsoid.b=b;
	m_Ellipsoid.e=sqrt(m_Ellipsoid.a*m_Ellipsoid.a-m_Ellipsoid.b*m_Ellipsoid.b)/m_Ellipsoid.a;
	m_Ellipsoid.e2=m_Ellipsoid.e*m_Ellipsoid.e;

	return S_OK;
}

HRESULT CGCProjection::Geodetic2Geocentric(double latitude, double longitude, double altitude, double *pX, double *pY, double *pZ)
{
	double normal;//large normal to ellipsoid

	//Compute large normal to ellipsoid
	normal = m_Ellipsoid.a                                /
      sqrt(1.0 - m_Ellipsoid.e2 * pow(sin(latitude),2.0));
	//Compute cartesian values
	*pX = (normal + altitude) * cos(latitude) * cos(longitude);
	*pY = (normal + altitude) * cos(latitude) * sin(longitude);
	*pZ = (normal * (1.0 - m_Ellipsoid.e2) + altitude) * 
           sin(latitude);

	return S_OK;
}

HRESULT CGCProjection::Geocentric2Geodetic(double X, double Y, double Z, double *pLat, double *pLon, double *pAltitude)
{
	double XY_norme;           //norme of geocentric vector on XY
	double XYZ_norme;          //norme of geocentric vector
	double XY_radius;          //projection of radius on XY plane
	double latitude;           //current latitude in iterative comp
	double new_latitude;       //new latitude in iterative computing
	BOOL done;               //"computing iteration is done" flag
	double iteration_count;    //current iteration count

	//Compute longitude
	if (X == 0)
	{
		*pLon = PI / 2;
	}
	else
	{
		*pLon = atan2 (Y,X);
	}

	//Compute latitude
	XY_norme  = sqrt(X*X + Y*Y);
	XYZ_norme = sqrt(X*X + Y*Y + Z*Z);
	XY_radius = XY_norme * (1.0 - (m_Ellipsoid.a *
                                  m_Ellipsoid.e2) /
                                  XYZ_norme);
   if (TimDEqual(XY_radius,(double)0.0,
       (double)TIM_LATITUDE_COMPUTING_PRECISION))
      latitude = PI /2;
   else
		latitude = atan2 (Z,XY_radius);

   iteration_count = 0;
   done            = FALSE;

   while ((!done) && (iteration_count < TIM_MAX_ITERATION_NUMBER))
	{
		new_latitude = atan ((Z / XY_norme) * (1.0 /
                      (1.0 - (m_Ellipsoid.a *
                              m_Ellipsoid.e2 *
                              cos(latitude)) /
                      (XY_norme *
                       sqrt(1.0 - m_Ellipsoid.e2 *
                            pow(sin(latitude),2.0))))));
		if (TimDEqual(new_latitude,latitude,
          (double)TIM_LATITUDE_COMPUTING_PRECISION))
		{
			done = TRUE;
		}
		else
		{
			latitude = new_latitude;
		}

		iteration_count += 1;
	}

	*pLat = latitude;
	*pAltitude = sqrt(X*X + Y*Y) / cos(latitude) -
                 m_Ellipsoid.a                 /
                 sqrt(1.0 - m_Ellipsoid.e2 *
                            pow(sin(latitude),2.0));

	return S_OK;
}
