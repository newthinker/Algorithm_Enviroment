// NSDTFDEM.h: interface for the CNSDTFDEM class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NSDTFDEM_H__089AE867_5282_4095_AAD7_60D4B64B193B__INCLUDED_)
#define AFX_NSDTFDEM_H__089AE867_5282_4095_AAD7_60D4B64B193B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "dem.h"

class CNSDTFDEM : public CDEM
{
public:
	CNSDTFDEM();
	virtual ~CNSDTFDEM();

	HRESULT GetCellSize(double* pXCellSize,double* pYCellSize);
	HRESULT GetRange(double* pLBX,double* pLBY,double* pRTX,double* pRTY);
	HRESULT GetTIN(DWORD ppTriangulateio);
	HRESULT Close();
	HRESULT GetSupExts(char* lpszExts,UINT accMode);
	HRESULT GetSupFormats(char* lpszFormats,UINT accMode);
	HRESULT GetIImage(CImage** ppIImage);
	HRESULT GetVertex(double* pX,double* pY,double* pZ,int* pVertexNum);
	HRESULT GetTriangle(int nTriIndex,double* pX,double* pY,double* pZ);
	HRESULT GetTriangleNum(int* pTriangleNum);
	HRESULT ConstrustTIN();
	HRESULT GetAverageAltitude(double* pZ);
	HRESULT GetAltitude(double X,double Y,double* pZ, UINT uResampleMethod);
	HRESULT Create(char* pszPathName,char* pszFormat,UINT accMode,int nRows,int nCols,double LBX,double LBY,double XCellSize,double YCellSize);
	HRESULT Open(char* pszPathName,double lfAltitudeOffset,UINT accMode);
	HRESULT GetMaxAltitude(double* pZ);
	HRESULT GetMinAltitude(double* pZ);
	HRESULT GetType(UINT* pType);
	HRESULT GetRows(int* pRows);
	HRESULT GetCols(int* pCols);
	HRESULT GetStartPos(double* pX0,double* pY0);
	HRESULT GetAltitudeValues(double* pZ);
	HRESULT IsSupported(char* pszPathName,UINT accMode);
};

#endif // !defined(AFX_NSDTFDEM_H__089AE867_5282_4095_AAD7_60D4B64B193B__INCLUDED_)
