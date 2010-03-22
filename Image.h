// Image.h: interface for the CImage class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IMAGE_H__6552CE30_B9DC_4D91_A2C4_64746BB9AE41__INCLUDED_)
#define AFX_IMAGE_H__6552CE30_B9DC_4D91_A2C4_64746BB9AE41__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "gdal_priv.h"
#include "ImageDriver.h"
#include "Histogram.h"

typedef enum _tagOpenFlags 
{
	modeRead =				0x0000,
	modeWrite =				0x0001,
	modeReadWrite =		    0x0002,
	modePyramidCreate =		0x0100,
	modePyramidClean =		0x0200,
	modeAqlut =				0x0400,
	modePyramidRead =		0x0800,
	shareCompat =			0x0000,
	shareExclusive =		0x0010,
	shareDenyWrite =		0x0020,
	shareDenyRead =			0x0030,
	shareDenyNone =			0x0040,
	modeNoInherit =			0x0080,
	modeCreate =			0x1000,
	modeNoTruncate =		0x2000,
}OpenFlags;

typedef enum _tagBandFormat
{
	BIP,
	BIL,
	BSQ
} BandFormat;

typedef enum _tagByteOrder
{
	Intel,
	IEEE
}ByteOrder;

typedef enum _tagDataType
{
	Pixel_Byte,
	Pixel_Int16,
	Pixel_SInt16,
	Pixel_Int32,
	Pixel_SInt32,
	Pixel_Float,
	Pixel_Double, 
	Pixel_Int64,
	Pixel_SInt64
}DataType;

typedef enum _tagSensorType
{
	SENSOR_CCD=0,
	SENSOR_HSI=1,
	SENSOR_IRS=2,
	SENSOR_CCD1=3,	// Add sensor type CCD1,CCD2 and SAR. [Wei.Zuo,4/15/2009]
	SENSOR_CCD2=4,
	SENSOR_SAR=5
}SENSORTYPE;

class CImage  
{
public:
	CImage();
	virtual ~CImage();

public:
	HRESULT Open(char* pszPathName,UINT uMode);
	HRESULT CreateImg(char* pszFilePath, UINT uMode, int Cols, int Rows, UINT DataType, int nBandNum, UINT BandType,double xStart,double yStart,double cellSize);
	HRESULT Close();
	HRESULT IsGeoCoded();
	HRESULT GetBandFormat(UINT* pBandFormat);
	HRESULT SetBandForamt(UINT BandFormat);
	HRESULT GetBandNum(int* pBandNum);
	HRESULT SetBandNum(int nBandNum);
	HRESULT GetRows(int* pRows);
	HRESULT SetRows(int nRows);
	HRESULT GetCols(int* pCols);
	HRESULT SetCols(int nCols);
	HRESULT GetDataType(UINT* pDataType);
	HRESULT SetDataType(UINT DataType);
	HRESULT GetByteOrder(UINT* pByteOrder);
	HRESULT SetByteOrder(UINT ByteOrder);
	HRESULT GetScale(float* pScale);
	HRESULT SetScale(float Scale);
	HRESULT GetGrdInfo(double* xStart,double* yStart,double* cellSize);
	HRESULT SetGrdInfo(double xStart,double yStart,double cellSize);
	HRESULT HaveColorTable();
	HRESULT GetEntryNum(int* pEntryNum);
	HRESULT GetColorTable(BYTE* pColorTable);
	HRESULT SetColorTable(BYTE* pColorTable,int nEntryNum);
	HRESULT GetResampleMethod(int* pResampleMethod);
	HRESULT SetResampleMethod(int ResampleMethod);
	HRESULT GetScanSize(float* pScanSize);
	HRESULT SetScanSize(float ScanSize);
	HRESULT GetBPB(int* pBPB);
	HRESULT SetBPB(int nBPB);
	HRESULT GetBPP(int* pBPP);
	HRESULT SetBPP(int nBPP);
	HRESULT GetPathName(char** ppszPathName);
	HRESULT GetPixel(int nRows, int nCols, BYTE* pPixel);
	HRESULT SetPixel(int nRows, int nCols, BYTE* pPixel);
	HRESULT GetGray(int nRows, int nCols, int nBand, BYTE* gray);
	HRESULT SetGray(int nRows, int nCols, int nBand, BYTE* gray);
	HRESULT GetPixelF(float x, float y, BYTE* pPixel, UINT nResampleMethod);
	HRESULT GetGrayF(float x, float y, int nBand, BYTE* pGray, int nResampleMethod);
	HRESULT ReadImg(float fSrcLeft, float fSrcTop, float fSrcRight, float fSrcBottom, BYTE* pBuf, int nBufWid, int nBufHeight, int nBandNum, int nDestLeft, int nDestTop, int nDestRight, int nDestBottom, int nSrcSkip, int nDestSkip);
	HRESULT WriteImg(float fSrcLeft, float fSrcTop, float fSrcRight, float fSrcBottom, BYTE* pBuf, int nBufWid, int nBufHeight, int nBandNum, int nDestLeft, int nDestTop, int nDestRight, int nDestBottom, int nSrcSkip, int nDestSkip);
	HRESULT SetProgressInterface(void* pIUnknown);
	HRESULT GetSupExts(char** ppszExts,UINT flags);
	HRESULT Image2World(float x,float y,double* pX,double* pY);
	HRESULT World2Image(double X,double Y,float* px,float* py);
	HRESULT GetDefaultBand(int* nBandIndex);
	HRESULT GetDPI(float* pxDPI,float* pyDPI);
	HRESULT SetDPI(float xDPI,float yDPI);
	//add by  dy 20081027 for get sensortype begin
	HRESULT GetSensorType(int * pSensorType);
	HRESULT SetSensorType(int nSensorType);
	//Get satellite ID. [Wei.Zuo,4/15/2009]
	HRESULT GetSatelliteID(int *pSatelliteID);
	HRESULT SetSatelliteID(int nSatelliteID);
protected:
	BOOL ResetCache(int nRow,int nCol);
	HRESULT ReadImgBand(float fSrcLeft, float fSrcTop, float fSrcRight, float fSrcBottom, BYTE* pBuf, int nBufWidth, int nBufHeight, int nBandNum, int nDestLeft, int nDestTop, int nDestRight, int nDestBottom, int nSrcSkip, int nDestSkip);
	HRESULT WriteImgBand(float fSrcLeft, float fSrcTop, float fSrcRight, float fSrcBottom, BYTE* pBuf, int nBufWid, int nBufHeight, int nBandNum, int nDestLeft, int nDestTop, int nDestRight, int nDestBottom, int nSrcSkip, int nDestSkip);
	GDALDataType ConvertDataType2GDALDataType(UINT datatype);
	UINT ConvertGDALDataType2DataType(GDALDataType gdalDataType);
protected:
	BOOL m_bFlip;
	double m_LBX,m_LBY,m_RTX,m_RTY;
	double m_CellSize;
	UINT m_nResampleMethord;
	char* m_pszPathName;
	int m_nRows,m_nCols,m_nBandNum,m_nBPB;
	int m_nImgNum;
	GDALDataType m_datatype;
	int m_nSensorType;
	int m_nSatelliteID;		// Added for CCD bands correcting. [Wei.Zuo,4/15/2009]
protected:
	BYTE* m_pCache;//BSQ������
	int m_nCacheBandSize;//�������в��εĴ�С
	int m_nCacheScanSize;//�������е������еĴ�С
	int m_CacheSize;//�������С
	int m_nCacheRow;//�������ܹ������Ӱ�������
	int m_nCacheRow0;//��������ʼ�У���m_nCacheRow0
	int m_nCacheRow1;//��������ֹ�У�����m_nCacheRow1

	GDALDataset* m_pGdalImage;
	FILE** m_fpRaws;

	CHistogram* m_pHistogram;
};

#endif // !defined(AFX_IMAGE_H__6552CE30_B9DC_4D91_A2C4_64746BB9AE41__INCLUDED_)
