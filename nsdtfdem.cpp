// NSDTFDEM.cpp: implementation of the CNSDTFDEM class.
//
//////////////////////////////////////////////////////////////////////

#include "nsdtfdem.h"
#include <stdio.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNSDTFDEM::CNSDTFDEM()
{
	
}

CNSDTFDEM::~CNSDTFDEM()
{
	Close();
}

HRESULT CNSDTFDEM::Open(char* pszPathName,double lfAltitudeOffset,UINT accMode)
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
	//��ȡ���
	FILE* fp=fopen(m_pszDEMPathName,"rt");
	if(fp==NULL)
	{
		COutput::OutputFileOpenFailed(m_pszDEMPathName);

		return S_FALSE;
	}

	char NSDTFHEADER[20];
	char VERSION[20];
	char UNIT[20];
	char COMPRESS[20];
	int	nHZoom=1;
	fscanf(fp,"%s%s%s%lf%s",NSDTFHEADER,VERSION,UNIT,&m_Kappa,COMPRESS);
	fscanf(fp,"%lf%lf%lf%lf%d%d%d",
			&m_X0,
			&m_Y0,
			&m_XCellSize,
			&m_YCellSize,
			&m_nRows,
			&m_nCols,
			&nHZoom);
	
	m_AveAltitude=0;
	m_MaxAltitude=-99999.9;
	m_MinAltitude=99999.9;
	double* pAltitude=new double[m_nRows*m_nCols];
	if(fabs(m_Kappa)<1e-5)
	{
		for(int i=m_nRows-1;i>=0;i--)
		{
			double temp=0;
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

					if(pAltitudeIndex[j]>m_MaxAltitude)
					{
						m_MaxAltitude=pAltitudeIndex[j];
					}
					if(pAltitudeIndex[j]<m_MinAltitude)
					{
						m_MinAltitude=pAltitudeIndex[j];
					}

					temp+=pAltitudeIndex[j];
					nCount++;
				}
			}
			if(nCount>0)
			{
				m_AveAltitude+=(temp/nCount);
			}
		}
	}
	else
	{
		for(int i=0;i<m_nRows;i++)
		{
			double temp=0;
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
					
					if(pAltitudeIndex[j]>m_MaxAltitude)
					{
						m_MaxAltitude=pAltitudeIndex[j];
					}
					if(pAltitudeIndex[j]<m_MinAltitude)
					{
						m_MinAltitude=pAltitudeIndex[j];
					}

					temp+=pAltitudeIndex[j];
					nCount++;
				}
			}
			if(nCount>0)
			{
				m_AveAltitude+=(temp/nCount);
			}
		}
	}
	m_AveAltitude/=m_nRows;
	fclose(fp);
	
	//����DEMդ��Ӱ��
	m_pszDemImage=new char[strlen(m_pszDEMPathName)+128];
	strcpy(m_pszDemImage,m_pszDEMPathName);
	strcat(m_pszDemImage,"~~.tif");
	
	if(fabs(m_Kappa)<1e-5)
	{
		if(m_Image.CreateImg(m_pszDemImage,modeCreate|modeWrite,
								m_nCols,m_nRows,Pixel_Double,
								1,BSQ,
								m_X0,m_Y0-m_nRows*m_YCellSize,m_XCellSize)!=S_OK)
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
	}
	else
	{
		double cosk=cos(m_Kappa);
		double sink=sin(m_Kappa);
		double dX=m_nCols*m_XCellSize;
		double dY=-m_nRows*m_YCellSize;

		//	0	1
		//	3	2
		double X0=m_X0;
		double Y0=m_Y0;

		double X1=m_X0+dX*cosk-0*sink;
		double Y1=m_Y0+dX*sink+0*cosk;

		double X2=m_X0+dX*cosk-dY*sink;
		double Y2=m_Y0+dX*sink+dY*cosk;

		double X3=m_X0+0*cosk-dY*sink;
		double Y3=m_Y0+0*sink+dY*cosk;
		
		double LBX=__min(__min(X0,X1),__min(X2,X3));
		double LBY=__min(__min(Y0,Y1),__min(Y2,Y3));
		double RTX=__max(__max(X0,X1),__max(X2,X3));
		double RTY=__max(__max(Y0,Y1),__max(Y2,Y3));

		int nRows=(int)((RTY-LBY)/m_YCellSize+0.5);
		int nCols=(int)((RTX-LBX)/m_XCellSize+0.5);

		if(m_Image.CreateImg(m_pszDemImage,modeCreate|modeWrite,
								nCols,nRows,Pixel_Double,
								1,BSQ,
								LBX,LBY,m_XCellSize)!=S_OK)
		{
			COutput::OutputFileOpenFailed(m_pszDemImage);

			delete [] pAltitude;
			pAltitude=NULL;

			return S_FALSE;
		}
		for(int i=0;i<nRows;i++)
		{
			for(int j=0;j<nCols;j++)
			{
				double X=LBX+j*m_XCellSize;
				double Y=LBY+i*m_YCellSize;
				double dX=X-m_X0;
				double dY=Y-m_Y0;
				double x=(dX*cosk+dY*sink)/m_XCellSize;
				double y=-(-dX*sink+dY*cosk)/m_YCellSize;

				int nCol0=int(x);
				int nRow0=int(y);
				int nCol2=nCol0+1;
				int nRow2=nRow0+1;
				if((nCol0<0||nCol0>=m_nCols||nRow0<0||nRow0>=m_nRows)&&
					(nCol2<0||nCol2>=m_nCols||nRow2<0||nRow2>=m_nRows))
				{
					double Z=DBL_MAX;
					m_Image.SetPixel(i,j,(BYTE*)&Z);
					continue;
				}

				nCol0=__max(nCol0,0);
				nRow0=__max(nRow0,0);
				nCol0=__min(nCol0,m_nCols-1);
				nRow0=__min(nRow0,m_nRows-1);
				nCol2=__max(nCol2,0);
				nRow2=__max(nRow2,0);
				nCol2=__min(nCol2,m_nCols-1);
				nRow2=__min(nRow2,m_nRows-1);

				double dx=x-nCol0;
				double dy=y-nRow0;

				double Z0=pAltitude[nRow0*m_nCols+nCol0];
				double Z1=pAltitude[nRow0*m_nCols+nCol2];
				double Z2=pAltitude[nRow2*m_nCols+nCol2];
				double Z3=pAltitude[nRow2*m_nCols+nCol0];

				double Z=DBL_MAX;
				if(Z0!=DBL_MAX&&Z1!=DBL_MAX&&Z2!=DBL_MAX&&Z3!=DBL_MAX)
				{
					Z=(1.0-dx) *  (1.0-dy)  *  Z0+
						 dx    *  (1.0-dy)  *  Z1+
						 dx    *  dy        *  Z2+
					  (1.0-dx) *  dy        *  Z3;

					Z+=m_lfAltitudeOffset;
				}
				
				m_Image.SetPixel(i,j,(BYTE*)&Z);
			}
		}
	}

	delete [] pAltitude;
	pAltitude=NULL;

	m_Image.Close();
	m_Image.Open(m_pszDemImage,modeRead);

	return S_OK;
}

HRESULT CNSDTFDEM::Create(char* pszPathName,char* pszFormat,UINT accMode,int nRows,int nCols,double LBX,double LBY,double XCellSize,double YCellSize)
{
	return S_OK;
}

HRESULT CNSDTFDEM::Close()
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

HRESULT CNSDTFDEM::GetSupExts(char* lpszExts,UINT accMode)
{
	return S_OK;
}

HRESULT CNSDTFDEM::GetSupFormats(char* lpszFormats,UINT accMode)
{
	
	return S_OK;
}

HRESULT CNSDTFDEM::GetIImage(CImage** ppIImage)
{
	if(ppIImage==NULL)
	{
		return S_FALSE;
	}

	*ppIImage=&m_Image;

	return S_OK;
}

HRESULT CNSDTFDEM::GetVertex(double* pX,double* pY,double* pZ,int* pVertexNum)
{
	if(pX==NULL||pY==NULL||pZ==NULL)
	{
		if(pVertexNum!=NULL)
		{
			*pVertexNum=m_nRows*m_nCols;
		}

		return S_FALSE;
	}

	if(pVertexNum==NULL)
	{
		return S_FALSE;
	}
	if(*pVertexNum<m_nRows*m_nCols)
	{
		return S_FALSE;
	}

	//��ȡDEM���
	double X0,Y0,CellSize;
	m_Image.GetGrdInfo(&X0,&Y0,&CellSize);
	//��ȡDEM�����
	for(int i=0;i<m_nRows;i++)
	{
		double* pXIndex=pX+i*m_nCols;
		double* pYIndex=pY+i*m_nCols;
		
		double X=X0;
		double Y=Y0+i*CellSize;

		for(int j=0;j<m_nCols;j++)
		{
			pXIndex[j]=X;
			pYIndex[j]=Y;

			X+=CellSize;
		}
	}
	m_Image.ReadImg(0.0f,0.0f,(float)m_nCols,(float)m_nRows,
					(BYTE*)pZ,m_nCols,m_nRows,
					1,
					0,0,m_nCols,m_nRows,
					0,0);

	return S_OK;
}

HRESULT CNSDTFDEM::GetTriangle(int nTriIndex,double* pX,double* pY,double* pZ)
{

	return S_FALSE;
}

HRESULT CNSDTFDEM::GetTriangleNum(int* pTriangleNum)
{

	return S_FALSE;
}

HRESULT CNSDTFDEM::ConstrustTIN()
{

	return S_FALSE;
}

HRESULT CNSDTFDEM::GetAverageAltitude(double* pZ)
{
	if(pZ==NULL)
	{
		return S_FALSE;
	}

	*pZ=m_AveAltitude;

	return S_OK;
}

HRESULT CNSDTFDEM::GetAltitude(double X,double Y,double* pZ, UINT uResampleMethod)
{
	*pZ=INVALID_ALTITUDE;

	float x,y;
	m_Image.World2Image(X,Y,&x,&y);
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

	if(Z0!=DBL_MAX&&Z1!=DBL_MAX&&Z2!=DBL_MAX&&Z3!=DBL_MAX)
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

HRESULT CNSDTFDEM::GetTIN(DWORD ppTriangulateio)
{
	return S_FALSE;
}

HRESULT CNSDTFDEM::GetRange(double *pLBX, double *pLBY, double *pRTX, double *pRTY)
{
	double CellSize;
	m_Image.GetGrdInfo(pLBX,pLBY,&CellSize);
	int nRows,nCols;
	m_Image.GetRows(&nRows);
	m_Image.GetCols(&nCols);
	*pRTX=*pLBX+nCols*CellSize;
	*pRTY=*pLBY+nRows*CellSize;

	return S_OK;
}

HRESULT CNSDTFDEM::GetCellSize(double *pXCellSize, double *pYCellSize)
{
	double LBX,LBY;
	double CellSize;
	m_Image.GetGrdInfo(&LBX,&LBY,&CellSize);
	*pXCellSize=CellSize;
	*pYCellSize=CellSize;

	return S_OK;;
}

HRESULT CNSDTFDEM::GetMaxAltitude(double *pZ)
{
	if(pZ==NULL)
	{
		return S_FALSE;
	}

	*pZ=m_MaxAltitude;

	return S_OK;
}

HRESULT CNSDTFDEM::GetMinAltitude(double *pZ)
{
	if(pZ==NULL)
	{
		return S_FALSE;
	}

	*pZ=m_MinAltitude;

	return S_OK;
}

HRESULT CNSDTFDEM::GetType(UINT* pType)
{
	*pType=DT_GRID;

	return S_OK;
}

HRESULT CNSDTFDEM::GetRows(int* pRows)
{
	*pRows=m_nRows;

	return S_OK;
}

HRESULT CNSDTFDEM::GetCols(int* pCols)
{
	*pCols=m_nCols;

	return S_OK;
}

HRESULT CNSDTFDEM::GetStartPos(double* pX0,double* pY0)
{
	double temp;
	m_Image.GetGrdInfo(pX0,pY0,&temp);

	return S_OK;
}

HRESULT CNSDTFDEM::GetAltitudeValues(double* pZ)
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

HRESULT CNSDTFDEM::IsSupported(char* pszPathName,UINT accMode)
{
	//��ȡ���
	if(accMode&modeWrite||accMode&modeCreate)
	{
		return S_FALSE;
	}

	FILE* fp=fopen(pszPathName,"rt");
	if(fp==NULL)
	{
		return S_FALSE;
	}

	char NSDTFHEADER[20];
	fscanf(fp,"%s",NSDTFHEADER);
	fclose(fp);

	if(strcmp(NSDTFHEADER,"NSDTF-DEM")==0)
	{
		return S_OK;
	}

	return S_FALSE;
}
