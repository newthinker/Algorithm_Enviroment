// TMProjection.cpp: implementation of the CTMProjection class.
//
//////////////////////////////////////////////////////////////////////

#include "TMProjection.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTMProjection::CTMProjection()
{

}

CTMProjection::~CTMProjection()
{

}

HRESULT CTMProjection::SetProjectionPara(double Origin_Lon, 
										  double Origin_Lat, 
										  double False_Easting, 
										  double False_Northing, 
										  double ScaleFactor)
{
	double dummy_northing;

	m_tmOrigin_Lat = 0;
	m_tmOrigin_Long = 0;
	m_tmFalse_Northing = 0;
	m_tmFalse_Easting = 0; 
	m_tmScale_Factor = 1;

	Geodetic2TM(MAX_LAT,
				MAX_DELTA_LONG,
				&m_tmDelta_Easting,
				&m_tmDelta_Northing);
	Geodetic2TM(0,
				MAX_DELTA_LONG,
				&m_tmDelta_Easting,
				&dummy_northing);
	if (Origin_Lon > PI)
		Origin_Lon -= (2*PI);
	
	m_tmOrigin_Lat = Origin_Lat;
	m_tmOrigin_Long = Origin_Lon;
	m_tmFalse_Northing = False_Northing;
	m_tmFalse_Easting = False_Easting; 
	m_tmScale_Factor = ScaleFactor;

	return S_OK;
}

HRESULT CTMProjection::SetDatumPara(double a, double b)
{
	double a2,b2;
	double tn,tn2,tn3,tn4,tn5;

	if( a<=0 || b<=0 || a<b )return FALSE;

	m_lfTM_a = a;
	m_lfTM_b = b;	

	/* Eccentricity Squared */
	a2 = a * a;
	b2 = b * b;
	m_lfTM_es = (a2 - b2) / a2;
	/* Second Eccentricity Squared */
	m_lfTM_ebs = (a2 - b2) / b2;
	
	/*True meridional constants  */
	tn = (a - b) / (a + b);
	tn2 = tn * tn;
	tn3 = tn2 * tn;
	tn4 = tn3 * tn;
	tn5 = tn4 * tn;
	
	m_lfTM_ap = a * (1.e0 - tn + 5.e0 * (tn2 - tn3)/4.e0
		+ 81.e0 * (tn4 - tn5)/64.e0 );
	m_lfTM_bp = 3.e0 * a * (tn - tn2 + 7.e0 * (tn3 - tn4)
		/8.e0 + 55.e0 * tn5/64.e0 )/2.e0;
	m_lfTM_cp = 15.e0 * a * (tn2 - tn3 + 3.e0 * (tn4 - tn5 )/4.e0) /16.0;
	m_lfTM_dp = 35.e0 * a * (tn3 - tn4 + 11.e0 * tn5 / 16.e0) / 48.e0;
	m_lfTM_ep = 315.e0 * a * (tn4 - tn5) / 512.e0;

	return S_OK;
}

/*
 * The function Convert_Geodetic_To_Transverse_Mercator converts geodetic
 * (latitude and longitude) coordinates to Transverse Mercator projection
 * (easting and northing) coordinates, according to the current ellipsoid
 * and Transverse Mercator projection coordinates.  If any errors occur, the
 * error code(s) are returned by the function, otherwise TRANMERC_NO_ERROR is
 * returned.
 *
 *    Latitude      : Latitude in radians                         (input)
 *    Longitude     : Longitude in radians                        (input)
 *    Easting       : Easting/X in meters                         (output)
 *    Northing      : Northing/Y in meters                        (output)
 */
HRESULT CTMProjection::Geodetic2TM(double Latitude, double Longitude, double *Easting, double *Northing)
{
	/* BEGIN Convert_Geodetic_To_Transverse_Mercator */
	
	/*
	* The function Convert_Geodetic_To_Transverse_Mercator converts geodetic
	* (latitude and longitude) coordinates to Transverse Mercator projection
	* (easting and northing) coordinates, according to the current ellipsoid
	* and Transverse Mercator projection coordinates.  If any errors occur, the
	* error code(s) are returned by the function, otherwise TRANMERC_NO_ERROR is
	* returned.
	*
	*    Latitude      : Latitude in radians                         (input)
	*    Longitude     : Longitude in radians                        (input)
	*    Easting       : Easting/X in meters                         (output)
	*    Northing      : Northing/Y in meters                        (output)
	*	index		  : index of datum,0 for primary,other for second (input)
	*/
	
	double c;       /* Cosine of latitude                          */
	double c2;
	double c3;
	double c5;
	double c7;
	double dlam;    /* Delta longitude - Difference in Longitude       */
	double eta;     /* constant - m_lfTM_ebs *c *c                   */
	double eta2;
	double eta3;
	double eta4;
	double s;       /* Sine of latitude                        */
	double sn;      /* Radius of curvature in the prime vertical       */
	double t;       /* Tangent of latitude                             */
	double tan2;
	double tan3;
	double tan4;
	double tan5;
	double tan6;
	double t1;      /* Term in coordinate conversion formula - GP to Y */
	double t2;      /* Term in coordinate conversion formula - GP to Y */
	double t3;      /* Term in coordinate conversion formula - GP to Y */
	double t4;      /* Term in coordinate conversion formula - GP to Y */
	double t5;      /* Term in coordinate conversion formula - GP to Y */
	double t6;      /* Term in coordinate conversion formula - GP to Y */
	double t7;      /* Term in coordinate conversion formula - GP to Y */
	double t8;      /* Term in coordinate conversion formula - GP to Y */
	double t9;      /* Term in coordinate conversion formula - GP to Y */
	double tmd;     /* True Meridional distance                        */
	double tmdo;    /* True Meridional distance for latitude of origin */
	long    Error_Code = TRANMERC_NO_ERROR;
	double temp_Origin;
	double temp_Long;
	
	if ((Latitude < -MAX_LAT) || (Latitude > MAX_LAT))
	{  /* Latitude out of range */
		Error_Code|= TRANMERC_LAT_ERROR;
	}
	if (Longitude > PI)
		Longitude -= (2 * PI);
	if ((Longitude < (m_tmOrigin_Long - MAX_DELTA_LONG))
		|| (Longitude > (m_tmOrigin_Long + MAX_DELTA_LONG)))
	{
		if (Longitude < 0)
			temp_Long = Longitude + 2 * PI;
		else
			temp_Long = Longitude;
		if (m_tmOrigin_Long < 0)
			temp_Origin = m_tmOrigin_Long + 2 * PI;
		else
			temp_Origin = m_tmOrigin_Long;
		if ((temp_Long < (temp_Origin - MAX_DELTA_LONG))
			|| (temp_Long > (temp_Origin + MAX_DELTA_LONG)))
			Error_Code|= TRANMERC_LON_ERROR;
	}
	if (!Error_Code)
	{ /* no errors */
		
	  /* 
	  *  Delta Longitude
		*/
		dlam = Longitude - m_tmOrigin_Long;
		
		if (fabs(dlam) > (9.0 * PI / 180))
		{ /* Distortion will result if Longitude is more than 9 degrees from the Central Meridian */
			Error_Code |= TRANMERC_LON_WARNING;
		}
		
		if (dlam > PI)
			dlam -= (2 * PI);
		if (dlam < -PI)
			dlam += (2 * PI);
		if (fabs(dlam) < 2.e-10)
			dlam = 0.0;
		
		s = sin(Latitude);
		c = cos(Latitude);
		c2 = c * c;
		c3 = c2 * c;
		c5 = c3 * c2;
		c7 = c5 * c2;
		t = tan (Latitude);
		tan2 = t * t;
		tan3 = tan2 * t;
		tan4 = tan3 * t;
		tan5 = tan4 * t;
		tan6 = tan5 * t;
		eta = m_lfTM_ebs * c2;
		eta2 = eta * eta;
		eta3 = eta2 * eta;
		eta4 = eta3 * eta;
		
		/* radius of curvature in prime vertical */
		sn = SPHSN(Latitude);
		
		/* True Meridonal Distances */
		tmd = SPHTMD(Latitude);
		
		/*  Origin  */
		tmdo = SPHTMD (m_tmOrigin_Lat);
		
		/* northing */
		t1 = (tmd - tmdo) * m_tmScale_Factor;
		t2 = sn * s * c * m_tmScale_Factor/ 2.e0;
		t3 = sn * s * c3 * m_tmScale_Factor * (5.e0 - tan2 + 9.e0 * eta 
			+ 4.e0 * eta2) /24.e0; 
		
		t4 = sn * s * c5 * m_tmScale_Factor * (61.e0 - 58.e0 * tan2
			+ tan4 + 270.e0 * eta - 330.e0 * tan2 * eta + 445.e0 * eta2
			+ 324.e0 * eta3 -680.e0 * tan2 * eta2 + 88.e0 * eta4 
			-600.e0 * tan2 * eta3 - 192.e0 * tan2 * eta4) / 720.e0;
		
		t5 = sn * s * c7 * m_tmScale_Factor * (1385.e0 - 3111.e0 * 
			tan2 + 543.e0 * tan4 - tan6) / 40320.e0;
		
		*Northing = m_tmFalse_Northing + t1 + pow(dlam,2.e0) * t2
			+ pow(dlam,4.e0) * t3 + pow(dlam,6.e0) * t4
			+ pow(dlam,8.e0) * t5; 
		
		/* Easting */
		t6 = sn * c * m_tmScale_Factor;
		t7 = sn * c3 * m_tmScale_Factor * (1.e0 - tan2 + eta ) /6.e0;
		t8 = sn * c5 * m_tmScale_Factor * (5.e0 - 18.e0 * tan2 + tan4
			+ 14.e0 * eta - 58.e0 * tan2 * eta + 13.e0 * eta2 + 4.e0 * eta3 
			- 64.e0 * tan2 * eta2 - 24.e0 * tan2 * eta3 )/ 120.e0;
		t9 = sn * c7 * m_tmScale_Factor * ( 61.e0 - 479.e0 * tan2
			+ 179.e0 * tan4 - tan6 ) /5040.e0;
		
		*Easting = m_tmFalse_Easting + dlam * t6 + pow(dlam,3.e0) * t7 
			+ pow(dlam,5.e0) * t8 + pow(dlam,7.e0) * t9;
	}
	
	return Error_Code==UTM_NO_ERROR?S_OK:S_FALSE;
}

HRESULT CTMProjection::TM2Geodetic(double Easting, double Northing, double *Latitude, double *Longitude)
{
	/* BEGIN Convert_Transverse_Mercator_To_Geodetic */
	
	/*
	* The function Convert_Transverse_Mercator_To_Geodetic converts Transverse
	* Mercator projection (easting and northing) coordinates to geodetic
	* (latitude and longitude) coordinates, according to the current ellipsoid
	* and Transverse Mercator projection parameters.  If any errors occur, the
	* error code(s) are returned by the function, otherwise TRANMERC_NO_ERROR is
	* returned.
	*
	*    Easting       : Easting/X in meters                         (input)
	*    Northing      : Northing/Y in meters                        (input)
	*    Latitude      : Latitude in radians                         (output)
	*    Longitude     : Longitude in radians                        (output)
	*/
	
	double c;       /* Cosine of latitude                          */
	double de;      /* Delta easting - Difference in Easting (Easting-Fe)    */
	double dlam;    /* Delta longitude - Difference in Longitude       */
	double eta;     /* constant - m_lfTM_ebs *c *c                   */
	double eta2;
	double eta3;
	double eta4;
	double ftphi;   /* Footpoint latitude                              */
	int    i;       /* Loop iterator                   */
	double s;       /* Sine of latitude                        */
	double sn;      /* Radius of curvature in the prime vertical       */
	double sr;      /* Radius of curvature in the meridian             */
	double t;       /* Tangent of latitude                             */
	double tan2;
	double tan4;
	double t10;     /* Term in coordinate conversion formula - GP to Y */
	double t11;     /* Term in coordinate conversion formula - GP to Y */
	double t12;     /* Term in coordinate conversion formula - GP to Y */
	double t13;     /* Term in coordinate conversion formula - GP to Y */
	double t14;     /* Term in coordinate conversion formula - GP to Y */
	double t15;     /* Term in coordinate conversion formula - GP to Y */
	double t16;     /* Term in coordinate conversion formula - GP to Y */
	double t17;     /* Term in coordinate conversion formula - GP to Y */
	double tmd;     /* True Meridional distance                        */
	double tmdo;    /* True Meridional distance for latitude of origin */
	long Error_Code = TRANMERC_NO_ERROR;
	
	if ((Easting < (m_tmFalse_Easting - m_tmDelta_Easting))
		||(Easting > (m_tmFalse_Easting + m_tmDelta_Easting)))
	{ /* Easting out of range  */
		Error_Code |= TRANMERC_EASTING_ERROR;
	}
	if ((Northing < (m_tmFalse_Northing - m_tmDelta_Northing))
		|| (Northing > (m_tmFalse_Northing + m_tmDelta_Northing)))
	{ /* Northing out of range */
		Error_Code |= TRANMERC_NORTHING_ERROR;
	}
	
	if (!Error_Code)
	{
		/* True Meridional Distances for latitude of origin */
		tmdo = SPHTMD(m_tmOrigin_Lat);
		
		/*  Origin  */
		tmd = tmdo +  (Northing - m_tmFalse_Northing) / m_tmScale_Factor; 
		
		/* First Estimate */
		sr = SPHSR(0.e0);
		ftphi = tmd/sr;
		
		for (i = 0; i < 5 ; i++)
		{
			t10 = SPHTMD (ftphi);
			sr = SPHSR(ftphi);
			ftphi = ftphi + (tmd - t10) / sr;
		}
		
		/* Radius of Curvature in the meridian */
		sr = SPHSR(ftphi);
		
		/* Radius of Curvature in the meridian */
		sn = SPHSN(ftphi);
		
		/* Sine Cosine terms */
		s = sin(ftphi);
		c = cos(ftphi);
		
		/* Tangent Value  */
		t = tan(ftphi);
		tan2 = t * t;
		tan4 = tan2 * tan2;
		eta = m_lfTM_ebs * pow(c,2);
		eta2 = eta * eta;
		eta3 = eta2 * eta;
		eta4 = eta3 * eta;
		de = Easting - m_tmFalse_Easting;
		if (fabs(de) < 0.0001)
			de = 0.0;
		
		/* Latitude */
		t10 = t / (2.e0 * sr * sn * pow(m_tmScale_Factor, 2));
		t11 = t * (5.e0  + 3.e0 * tan2 + eta - 4.e0 * pow(eta,2)
			- 9.e0 * tan2 * eta) / (24.e0 * sr * pow(sn,3) 
			* pow(m_tmScale_Factor,4));
		t12 = t * (61.e0 + 90.e0 * tan2 + 46.e0 * eta + 45.E0 * tan4
			- 252.e0 * tan2 * eta  - 3.e0 * eta2 + 100.e0 
			* eta3 - 66.e0 * tan2 * eta2 - 90.e0 * tan4
			* eta + 88.e0 * eta4 + 225.e0 * tan4 * eta2
			+ 84.e0 * tan2* eta3 - 192.e0 * tan2 * eta4)
			/ ( 720.e0 * sr * pow(sn,5) * pow(m_tmScale_Factor, 6) );
		t13 = t * ( 1385.e0 + 3633.e0 * tan2 + 4095.e0 * tan4 + 1575.e0 
			* pow(t,6))/ (40320.e0 * sr * pow(sn,7) * pow(m_tmScale_Factor,8));
		*Latitude = ftphi - pow(de,2) * t10 + pow(de,4) * t11 - pow(de,6) * t12 
			+ pow(de,8) * t13;
		
		t14 = 1.e0 / (sn * c * m_tmScale_Factor);
		
		t15 = (1.e0 + 2.e0 * tan2 + eta) / (6.e0 * pow(sn,3) * c * 
			pow(m_tmScale_Factor,3));
		
		t16 = (5.e0 + 6.e0 * eta + 28.e0 * tan2 - 3.e0 * eta2
			+ 8.e0 * tan2 * eta + 24.e0 * tan4 - 4.e0 
			* eta3 + 4.e0 * tan2 * eta2 + 24.e0 
			* tan2 * eta3) / (120.e0 * pow(sn,5) * c  
			* pow(m_tmScale_Factor,5));
		
		t17 = (61.e0 +  662.e0 * tan2 + 1320.e0 * tan4 + 720.e0 
			* pow(t,6)) / (5040.e0 * pow(sn,7) * c 
			* pow(m_tmScale_Factor,7));
		
		/* Difference in Longitude */
		dlam = de * t14 - pow(de,3) * t15 + pow(de,5) * t16 - pow(de,7) * t17;
		
		/* Longitude */
		(*Longitude) = m_tmOrigin_Long + dlam;
		while (*Latitude > (90.0 * PI / 180.0))
		{
			*Latitude = PI - *Latitude;
			*Longitude += PI;
			if (*Longitude > PI)
				*Longitude -= (2 * PI);
		}
		
		while (*Latitude < (-90.0 * PI / 180.0))
		{
			*Latitude = - (*Latitude + PI);
			*Longitude += PI;
			if (*Longitude > PI)
				*Longitude -= (2 * PI);
		}
		if (*Longitude > (2*PI))
			*Longitude -= (2 * PI);
		if (*Longitude < -PI)
			*Longitude += (2 * PI);
		
		if (fabs(dlam) > (9.0 * PI / 180))
		{ /* Distortion will result if Longitude is more than 9 degrees from the Central Meridian */
			Error_Code |= TRANMERC_LON_WARNING;
		}
	}
	
	return Error_Code==UTM_NO_ERROR?S_OK:S_FALSE;
}
