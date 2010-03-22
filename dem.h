// DEM.h: interface for the CDEM class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DEM_H__56663792_54B2_4D40_8891_4943826B9BC8__INCLUDED_)
#define AFX_DEM_H__56663792_54B2_4D40_8891_4943826B9BC8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ImageDriver.h"
#include "Image.h"

enum Altitude
{
    INVALID_ALTITUDE = -99999
};

enum DEMType
{
	DT_GRID = 0x0000,
	DT_TIN  = 0x0001
};

class CDEM  
{
public:
	CDEM();
	virtual ~CDEM();

	virtual HRESULT GetCellSize(double* pXCellSize,double* pYCellSize)=0;
	virtual HRESULT GetRange(double* pLBX,double* pLBY,double* pRTX,double* pRTY)=0;
	virtual HRESULT GetTIN(DWORD ppTriangulateio)=0;
	virtual HRESULT Close()=0;
	virtual HRESULT GetSupExts(char* lpszExts,UINT accMode)=0;
	virtual HRESULT GetSupFormats(char* lpszFormats,UINT accMode)=0;
	virtual HRESULT GetIImage(CImage** ppIImage)=0;
	virtual HRESULT GetVertex(double* pX,double* pY,double* pZ,int* pVertexNum)=0;
	virtual HRESULT GetTriangle(int nTriIndex,double* pX,double* pY,double* pZ)=0;
	virtual HRESULT GetTriangleNum(int* pTriangleNum)=0;
	virtual HRESULT ConstrustTIN()=0;
	virtual HRESULT GetAverageAltitude(double* pZ)=0;
	virtual HRESULT GetAltitude(double X,double Y,double* pZ, UINT uResampleMethod)=0;
	virtual HRESULT Create(char* pszPathName,char* pszFormat,UINT accMode,int nRows,int nCols,double LBX,double LBY,double XCellSize,double YCellSize)=0;
	virtual HRESULT Open(char* pszPathName,double lfAltitudeOffset,UINT accMode)=0;
	virtual HRESULT GetMaxAltitude(double* pZ)=0;
	virtual HRESULT GetMinAltitude(double* pZ)=0;
	virtual HRESULT GetType(UINT* pType)=0;
	virtual HRESULT GetRows(int* pRows)=0;
	virtual HRESULT GetCols(int* pCols)=0;
	virtual HRESULT GetStartPos(double* pX0,double* pY0)=0;
	virtual HRESULT GetAltitudeValues(double* pZ)=0;
	virtual HRESULT IsSupported(char* pszPathName,UINT accMode)=0;

protected:
	//DEM基本属性
	double m_X0,m_Y0,m_XCellSize,m_YCellSize,m_Kappa;
	int m_nRows,m_nCols;
	double m_AveAltitude,m_MaxAltitude,m_MinAltitude;
	CImage m_Image;//DEM影像
	char* m_pszDemImage;//DEM影像名称
	double m_lfAltitudeOffset;
	char* m_pszDEMPathName;
};

#endif // !defined(AFX_DEM_H__56663792_54B2_4D40_8891_4943826B9BC8__INCLUDED_)
