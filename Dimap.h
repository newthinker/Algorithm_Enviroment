// Dimap.h: interface for the CDimap class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DIMAP_H__A7A24405_EE52_4F53_95FE_FDD167FB6B99__INCLUDED_)
#define AFX_DIMAP_H__A7A24405_EE52_4F53_95FE_FDD167FB6B99__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Kernel.h"
#include "RealVector.h"
#include "FArray.hpp"

class CDimap  
{
	typedef enum
	{
		HJ1A,
		HJ1B
	}SATELLITEID;
	typedef enum
	{
		HJ1A_CCD1	=	0,
		HJ1A_CCD2	=	1,
		HJ1A_HSI	=	2,
		HJ1B_CCD1	=	3,
		HJ1B_CCD2	=	4,
		HJ1B_IRS	=	5,
		HJ1A_CCD	=	6,
		HJ1B_CCD	=	7
	}SENSORTYPE;
	typedef struct _tagCCDCamera
	{
		double lfFocus;
		double* sx;
		double* sy;
		double lfScanSize;
		double lfRoll;
		double lfPitch;
		double lfYaw;
		int nCCDLength;

		_tagCCDCamera()
		{
			nCCDLength=12000;
			sx=NULL;
			sy=NULL;
		};
		~_tagCCDCamera()
		{
			if(sx!=NULL)
			{
				delete [] sx;
				sx=NULL;
			}
			if(sy!=NULL)
			{
				delete [] sy;
				sy=NULL;
			}
		};
	}CCDCamera;

	typedef struct _tagScanCamera
	{
		double lfFocus;
		double sx;
		double* sy;
		double lfScanSize;
		double lfRoll;
		double lfPitch;
		double lfYaw;
		int nScanLength;
		double lfPixelFOV;
		double lfPixelTime;
		int nScanFrame;

		_tagScanCamera()
		{
			nScanLength=4500;
			nScanFrame=20;
			sx=0;
			sy=NULL;
		};
		~_tagScanCamera()
		{
			if(sy!=NULL)
			{
				delete [] sy;
				sy=NULL;
			}
		};
	}ScanCamera;

	typedef struct _tagEphemeris
	{
		CRealVector P;//位置
		CRealVector V;//速度
		double t;
	}Ephemeris;

	typedef struct _tagAttitude
	{
		double YAW;
		double PITCH;
		double ROLL;
		double t;
	}Attitude;

public:
	HRESULT Image2Map(int nRow, int nCol, double fHeight, double *pGX, double *pGY);
	CDimap();
	virtual ~CDimap();

	HRESULT Image2Geodetic(int nRow,int nCol,double fHeight,double* pLat,double* pLon,double* pAltitude);
	HRESULT Initialize(char* pszAuxFile,char* pszEphFile,char* pszAttFile,char* pszCamFile);
	HRESULT ContrustRPC(char* pszRPCFile);
	HRESULT Geodetic2Map(double Latitude, double Longitude, double * pX, double * pY);		// Lat/Lon --> Geodetic coordinate. [WeiZ,4/21/2009]
protected:
	void MultipleMatrix(double * m1 , double *m2 , double *  multiple, int nDimension);
	void InitializeCamera(char* pszCamFile);
	void CreateRotMatrixPOK(double phi, double omega, double kappa, double rotMatrix[3][3]);
	void CreateRotMatrixPOK2(double phi, double omega, double kappa, double rotMatrix[3][3]);
	Attitude AttitudeDate(double fDate);
	Ephemeris EphemerisDate(double fDate);
	double LineDate(int nLine);
	double PixelDate(int nRow,int nCol);
	double ParseTime(char* pszTime);
	void dnrml (double* aa,int n,double bb,double* a,double* b);
	void pnrml(double *aa, int n, double bb, double *a, double *b, double p);
	void dsolve (double* a,double* b,double* x,int n,int wide);
	void dldltban1 (double* a,double* d,double* l,int n,int wide);
	void dldltban2 (double* l,double* d,double* b,double* x,int n,int wide);

private:
	int m_nRows,m_nCols;//景行列数
	double m_fT0;//景中心成像时间
	double m_fLSP;//Line Sampling Period
	CFArray<Ephemeris> m_Ephemeris;//星历参数
	CFArray<Attitude> m_Attitude;//姿态参数
	CFArray<double> m_ScanFrameTime;//IRS帧扫描开始时间
	double m_fDatasetFrameLat[4];
	double m_fDatasetFrameLon[4];
	double m_fDatasetCenterLat;
	double m_fDatasetCenterLon;
	CProjectWGS84 m_Projection;
	CCDCamera m_CCDCamera[5];
	ScanCamera m_ScanCamera;
	SATELLITEID m_nSatelliteID;
	SENSORTYPE m_nSensorType;
};

#endif // !defined(AFX_DIMAP_H__A7A24405_EE52_4F53_95FE_FDD167FB6B99__INCLUDED_)
