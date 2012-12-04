/*
 * SrtmDem.cpp
 *
 *  Created on: Mar 29, 2010
 *      Author: zuow
 */

#include <stdio.h>
#include "SrtmDem.h"
#include "ProjectWGS84.h"

CSrtmDem::CSrtmDem()
{

}

CSrtmDem::~CSrtmDem()
{
	Close();
}

HRESULT CSrtmDem::Close()
{
	m_Image.Close();

	if(m_pszDemImage!=NULL&&strlen(m_pszDemImage)>0)
	{
		remove(m_pszDemImage);
	}

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

	return S_OK;
}

HRESULT CSrtmDem::Open(char* pszPathName,double lfAltitudeOffset,UINT accMode)
{
	m_Image.Close();
	if(m_pszDEMPathName!=NULL&&strlen(m_pszDemImage)>0)
	{
		remove(m_pszDemImage);
	}

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

	m_pszDEMPathName=new char[strlen(pszPathName)+1];
	strcpy(m_pszDEMPathName,pszPathName);

	m_lfAltitudeOffset=lfAltitudeOffset;

	if(IsSupported(m_pszDEMPathName,accMode))
	{
		return S_FALSE;
	}

	if(access(m_pszDEMPathName,0)==-1)
	{
		COutput::OutputFileDoesNotExist(m_pszDEMPathName);

		return S_FALSE;
	}
	// 打开DEM文件
	FILE* fp=fopen(m_pszDEMPathName,"rt");
	if(fp==NULL)
	{
		COutput::OutputFileOpenFailed(m_pszDEMPathName);

		return S_FALSE;
	}

	char UNIT[20];
	int	nHZoom=1;
	fscanf(fp,"%*s%s", UNIT);
	fscanf(fp,"%lf%lf%lf%lf%d%d%d",
			&m_dLBX, &m_dLBY,
			&m_dRTX, &m_dRTY,
			&m_nRows, &m_nCols,
			&nHZoom);

	m_AveAltitude=0;
	m_MaxAltitude=-99999.9;
	m_MinAltitude=99999.9;
	double* pAltitude=new double[m_nRows*m_nCols];
	for(int i=m_nRows-1;i>=0;i--)
	{
		int nCount=0;
		double* pAltitudeIndex=pAltitude+m_nCols*i;
		for(int j=0;j<m_nCols;j++)
		{
			fscanf(fp,"%lf",pAltitudeIndex+j);

			if(fabs(pAltitudeIndex[j]+99999.0)<1e-5)
			{
				pAltitudeIndex[j]=DBL_MAX;
			}
			else
			{
				pAltitudeIndex[j]/=nHZoom;
				pAltitudeIndex[j]+=m_lfAltitudeOffset;

				if(pAltitudeIndex[j]>m_MaxAltitude)		// 获取最大高程
				{
					m_MaxAltitude=pAltitudeIndex[j];
				}
				if(pAltitudeIndex[j]<m_MinAltitude)		// 获取最小高程
				{
					m_MinAltitude=pAltitudeIndex[j];
				}

				nCount++;
				m_AveAltitude += (pAltitudeIndex[j]-m_AveAltitude)/nCount;				// 计算均值
			}
		}
	}
	fclose(fp);

	// 将DEM数据体部分单独生成一个文件
	m_pszDemImage=new char[strlen(m_pszDEMPathName)+128];
	strcpy(m_pszDemImage,m_pszDEMPathName);
	strcat(m_pszDemImage,"~~.tif");

	m_XCellSize = (m_dRTX-m_dLBX)/m_nCols;
	if(m_Image.CreateImg(m_pszDemImage,modeCreate|modeWrite,
							m_nCols,m_nRows,Pixel_Double,
							1,BSQ,
							m_dLBX, m_dLBY, m_XCellSize)!=S_OK)
	{
		COutput::OutputFileOpenFailed(m_pszDemImage);

		delete [] pAltitude;
		pAltitude=NULL;

		return S_FALSE;
	}

	m_Image.WriteImg(0,(float)0,(float)m_nCols,(float)m_nRows,
						(BYTE*)pAltitude,m_nCols,m_nRows,
						1,
						0,0,m_nCols,m_nRows,
						-1,0);

	delete [] pAltitude;
	pAltitude=NULL;

	m_Image.Close();
	m_Image.Open(m_pszDemImage,modeRead);

	return S_OK;
}

HRESULT CSrtmDem::Create(char* pszPathName,char* pszFormat,UINT accMode,int nRows,int nCols,double LBX,double LBY,double XCellSize,double YCellSize)
{
	return S_OK;
}

HRESULT CSrtmDem::GetSupExts(char* lpszExts,UINT accMode)
{
	return S_OK;
}

HRESULT CSrtmDem::GetSupFormats(char* lpszFormats,UINT accMode)
{

	return S_OK;
}

HRESULT CSrtmDem::GetIImage(CImage** ppIImage)
{
	if(ppIImage==NULL)
	{
		return S_FALSE;
	}

	*ppIImage=&m_Image;

	return S_OK;
}

HRESULT CSrtmDem::GetVertex(double* pX,double* pY,double* pZ,int* pVertexNum)
{
	return S_OK;
}

HRESULT CSrtmDem::GetTriangle(int nTriIndex,double* pX,double* pY,double* pZ)
{

	return S_FALSE;
}

HRESULT CSrtmDem::GetTriangleNum(int* pTriangleNum)
{
	return S_FALSE;
}

HRESULT CSrtmDem::ConstrustTIN()
{

	return S_FALSE;
}

HRESULT CSrtmDem::GetAverageAltitude(double* pZ)
{
	if(pZ==NULL)
	{
		return S_FALSE;
	}

	*pZ=m_AveAltitude;

	return S_OK;
}

HRESULT CSrtmDem::GetAltitude(double X,double Y,double* pZ, UINT uResampleMethod)
{
	*pZ=INVALID_ALTITUDE;

	float x,y;
	double dLat, dLong;
	// 将投影坐标转化成地理坐标
	CProjectWGS84 project;
	project.CreateProject("WGS84");
	project.SetZoneStep(6);
	project.SetZone(m_nZone);
	project.SetFalseEasting(500000);
	project.SetFalseNorthing(0);
	project.Map2Geodetic(X, Y, &dLat, &dLong);
	dLat = dLat * 180.0 / PI;
	dLong = dLong * 180.0 / PI;

	m_Image.World2Image(dLong, dLat, &x, &y);		// 转换成影像上的行列号
	int nCol0=int(x);
	int nRow0=int(y);
	int nCol2=nCol0+1;
	int nRow2=nRow0+1;
	if((nCol0<0||nCol0>=m_nCols||nRow0<0||nRow0>=m_nRows)&&
		(nCol2<0||nCol2>=m_nCols||nRow2<0||nRow2>=m_nRows))
	{
		*pZ=INVALID_ALTITUDE;//m_AveAltitude

		return S_FALSE;
	}

	double dx=x-nCol0;
	double dy=y-nRow0;

	double Z0,Z1,Z2,Z3;
	m_Image.GetGray(nRow0,nCol0,0,(BYTE*)&Z0);
	m_Image.GetGray(nRow0,nCol2,0,(BYTE*)&Z1);
	m_Image.GetGray(nRow2,nCol2,0,(BYTE*)&Z2);
	m_Image.GetGray(nRow2,nCol0,0,(BYTE*)&Z3);

	if(Z0!=DBL_MAX&&Z1!=DBL_MAX&&Z2!=DBL_MAX&&Z3!=DBL_MAX)		// 通过插值计算高程
	{
		*pZ=(1.0-dx) *  (1.0-dy)  *  Z0+
			   dx    *  (1.0-dy)  *  Z1+
		   	   dx    *  dy        *  Z2+
		    (1.0-dx) *  dy        *  Z3;

		return S_OK;
	}
	else
	{
		return S_FALSE;
	}
}

HRESULT CSrtmDem::GetTIN(DWORD ppTriangulateio)
{
	return S_FALSE;
}

HRESULT CSrtmDem::GetRange(double *pLBX, double *pLBY, double *pRTX, double *pRTY)
{
	*pLBX = m_dLBX;		*pLBY = m_dLBY;
	*pRTX = m_dRTX;		*pRTY = m_dRTY;

	return S_OK;
}

HRESULT CSrtmDem::GetCellSize(double *pXCellSize, double *pYCellSize)
{
	double LBX,LBY;
	double CellSize;
	m_Image.GetGrdInfo(&LBX,&LBY,&CellSize);
	*pXCellSize=CellSize;
	*pYCellSize=CellSize;

	return S_OK;;
}

HRESULT CSrtmDem::GetMaxAltitude(double *pZ)
{
	if(pZ==NULL)
	{
		return S_FALSE;
	}

	*pZ=m_MaxAltitude;

	return S_OK;
}

HRESULT CSrtmDem::GetMinAltitude(double *pZ)
{
	if(pZ==NULL)
	{
		return S_FALSE;
	}

	*pZ=m_MinAltitude;

	return S_OK;
}

HRESULT CSrtmDem::GetType(UINT* pType)
{
	*pType=DT_GRID;

	return S_OK;
}

HRESULT CSrtmDem::GetRows(int* pRows)
{
	*pRows=m_nRows;

	return S_OK;
}

HRESULT CSrtmDem::GetCols(int* pCols)
{
	*pCols=m_nCols;

	return S_OK;
}

HRESULT CSrtmDem::GetStartPos(double* pX0,double* pY0)
{
	*pX0 = m_dLBX;		*pY0 = m_dLBY;

	return S_OK;
}

HRESULT CSrtmDem::GetAltitudeValues(double* pZ)
{
	m_Image.ReadImg(0.0f,0.0f,(float)m_nCols,(float)m_nRows,
					(BYTE*)pZ,m_nCols,m_nRows,
					1,
					0,0,m_nCols,m_nRows,
					0,0);

	for(int i=0;i<m_nRows*m_nCols;i++)
	{
		if(pZ[i]==DBL_MAX)
		{
			pZ[i]=INVALID_ALTITUDE;
		}
	}

	return S_OK;
}

HRESULT CSrtmDem::IsSupported(char* pszPathName,UINT accMode)
{
	// 查看是否为SRTM grid 数据
	if(accMode&modeWrite||accMode&modeCreate)
	{
		return S_FALSE;
	}

	FILE* fp=fopen(pszPathName,"rt");
	if(fp==NULL)
	{
		return S_FALSE;
	}

	char cSrtmHeader[20];
	fscanf(fp,"%s",cSrtmHeader);
	fclose(fp);

	if(strcmp(cSrtmHeader,"SRTM-DEM")==0)
	{
		return S_OK;
	}

	return S_FALSE;
}
