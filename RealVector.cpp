// RealVector.cpp: implementation of the CRealVector class.
//
//////////////////////////////////////////////////////////////////////

#include "RealVector.h"
#include <math.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRealVector::CRealVector()
{
	x=0;
	y=0;
	z=0;
}

CRealVector::~CRealVector()
{

}

CRealVector CRealVector::Add(CRealVector v2)
{
	CRealVector v;
	
	v.x=x+v2.x;
	v.y=y+v2.y;
	v.z=z+v2.z;

	return v;
}

CRealVector CRealVector::Sub(CRealVector v2)
{
	CRealVector v;

	v.x=x-v2.x;
	v.y=y-v2.y;
	v.z=z-v2.z;

	return v;
}

CRealVector CRealVector::operator=(CRealVector other)
{
	x=other.x;
	y=other.y;
	z=other.z;

	return *this;
}

CRealVector CRealVector::CrossProduct(CRealVector v2)
{
	CRealVector v;
	
	v.x=y*v2.z-z*v2.y;
	v.y=z*v2.x-x*v2.z;
	v.z=x*v2.y-y*v2.x;

	return v;
}

double CRealVector::DotProduct(CRealVector v2)
{
	return 0;
}

void CRealVector::Normalize()
{
	double mod=sqrt(x*x+y*y+z*z);
	x/=mod;
	y/=mod;
	z/=mod;
}