/*
 * SrtmDem.h
 *
 *  Created on: Mar 29, 2010
 *      Author: zuow
 */

#ifndef SRTMDEM_H_
#define SRTMDEM_H_

#include "dem.h"

class CSrtmDem : public CDEM
{
public:
	CSrtmDem();
	virtual ~CSrtmDem();

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

public:
	int m_nZone;	// UTM投影带号
	double m_dLBX, m_dLBY, m_dRTX, m_dRTY;		// 左下角/右上角X/Y坐标
};

#endif /* SRTMDEM_H_ */
