// DEM.cpp: implementation of the CDEM class.
//
//////////////////////////////////////////////////////////////////////

#include "dem.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDEM::CDEM()
{
	m_X0=m_Y0=m_XCellSize=m_YCellSize=m_Kappa=0;
	m_nRows=m_nCols=0;
	m_AveAltitude=0;
	m_MaxAltitude=0;
	m_MinAltitude=0;
	m_lfAltitudeOffset=0;

	m_pszDemImage=NULL;
	m_pszDEMPathName=NULL;
}

CDEM::~CDEM()
{
	if(m_pszDemImage!=NULL)
	{
		delete [] m_pszDemImage;
		m_pszDemImage=NULL;
	}
	if(m_pszDEMPathName!=NULL)
	{
		delete [] m_pszDEMPathName;
		m_pszDEMPathName=NULL;
	}
}

