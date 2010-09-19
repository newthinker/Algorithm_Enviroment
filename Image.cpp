// Image.cpp: implementation of the CImage class.
//
//////////////////////////////////////////////////////////////////////

#include "Image.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CImage::CImage()
{
	m_nResampleMethord=1;
	m_LBX=0.0;
	m_LBY=0.0;
	m_CellSize=1.0;
	m_nRows=0;
	m_nCols=0;
	m_nBandNum=0;
	m_bFlip=TRUE;
	m_nSensorType=-1;
	//��ʼ��������
	m_pCache=NULL;
	m_CacheSize=0;
	m_nCacheRow=-1;
	m_nCacheRow0=-1;
	m_nCacheRow1=-1;
	m_pGdalImage=NULL;
	m_fpRaws=NULL;

	m_pszPathName=NULL;
	m_pHistogram=NULL;
}

CImage::~CImage()
{
	Close();
}

#define BiLinear_GDAL(Type) void Type##_BiLinear_GDAL(BYTE *buf11,BYTE *buf21, \
									   BYTE *buf22,BYTE *buf12, \
									   double Dx,double Dy,     \
									   BYTE *pOut)				\
{																\
	*((Type *)pOut) = (Type)((1-Dx) * (1-Dy) * (*((Type*)buf11))			\
			 + Dx     * (1-Dy) * (*((Type*)buf21))				\
			 + Dx     * Dy     * (*((Type*)buf22))				\
			 + (1-Dx) * Dy     * (*((Type*)buf12)));			\
}

BiLinear_GDAL(BYTE)
BiLinear_GDAL(USHORT)
BiLinear_GDAL(short)
BiLinear_GDAL(UINT)
BiLinear_GDAL(int)
BiLinear_GDAL(float)
BiLinear_GDAL(double)

#define FuncName_GDAL(Type) Type##_BiLinear_GDAL

typedef void (* BiLinearFunc_GDAL)(BYTE *buf11,BYTE *buf21, BYTE *buf22,BYTE *buf12,double Dx,double Dy, BYTE *pOut);

static BiLinearFunc_GDAL FuncTable_GDAL[9] = {FuncName_GDAL(BYTE),
											FuncName_GDAL(USHORT),
											FuncName_GDAL(short),
											FuncName_GDAL(UINT),
											FuncName_GDAL(int),
											FuncName_GDAL(float),
											FuncName_GDAL(double)
											};

HRESULT CImage::Open(char* pszPathName,UINT uMode)
{
	Close();

	m_pszPathName=new char[strlen(pszPathName)+1];
	strcpy(m_pszPathName,pszPathName);

	if(strcmp(m_pszPathName+strlen(m_pszPathName)-3,"txt")==0)
	{
		char szPath[512];
		memset(szPath,0,512*sizeof(char));

		int nPos=-1;
		int i=strlen(m_pszPathName)-1;
		for(;i>=0;i--)
		{
			if(m_pszPathName[i]=='\\'||m_pszPathName[i]=='/')
			{
				nPos=i;
				break;
			}
		}
		for(i=0;i<=nPos;i++)
		{
			szPath[i]=m_pszPathName[i];
		}

		FILE* fp=fopen(m_pszPathName,"rt");
		if(fp==NULL)
		{
			return S_FALSE;
		}
		BOOL bGeoCoded=FALSE;
		char szBuffer[1024];
		char szTag[100],szEqualMark[10],szValue[512];
		while(!feof(fp))
		{
			memset(szBuffer, 0 ,sizeof(char)*1024);
			memset(szTag, 0 ,sizeof(char)*100);
			memset(szEqualMark, 0 ,sizeof(char)*10);
			memset(szValue, 0 ,sizeof(char)*512);
			
			fgets(szBuffer,1023,fp);
		
			sscanf(szBuffer,"%s%s%s",szTag,szEqualMark,szValue);
			int nLength=strlen(szValue);
			if(nLength>0)
			{
				if(szValue[nLength-1]==';')
				{
					szValue[nLength-1]='\0';
				}
			}
			// Parse the satellite ID from the GC report file. [Wei.Zuo,4/15/2009]
			if(strcmp(szTag,"satelliteID")==0)
			{
				if(strcmp(szValue,"\"HJ1A\"")==0)
					m_nSatelliteID = 1;
				else if(strcmp(szValue,"\"HJ1B\"")==0)
					m_nSatelliteID = 2;
				else 
					m_nSatelliteID = 0;
			}
			else if(strcmp(szTag,"sensorID")==0)
			{
				if(strcmp(szValue,"\"CCD\"")==0)
				{
					m_nSensorType=SENSOR_CCD;
					m_nBPB=1;
					m_datatype=ConvertDataType2GDALDataType(Pixel_Byte);
				}
				else if(strcmp(szValue,"\"HSI\"")==0)
				{
					m_nSensorType=SENSOR_HSI;
					m_nBPB=2;
					m_datatype=ConvertDataType2GDALDataType(Pixel_Int16);
				}
				else if(strcmp(szValue,"\"IRS\"")==0)
				{
					m_nSensorType=SENSOR_IRS;
					m_nBPB=2;
					m_datatype=ConvertDataType2GDALDataType(Pixel_Int16);
				}
				// Add sensor CCD1 and CCD2. [Wei.Zuo,4/15/2009]
				else if(strcmp(szValue,"\"CCD1\"")==0)
				{
					m_nSensorType=SENSOR_CCD1;
					m_nBPB=1;
					m_datatype=ConvertDataType2GDALDataType(Pixel_Byte);
				}
				else if(strcmp(szValue,"\"CCD2\"")==0)
				{
					m_nSensorType=SENSOR_CCD2;
					m_nBPB=1;
					m_datatype=ConvertDataType2GDALDataType(Pixel_Byte);
				}
				// Later will add sensor SAR. [Wei.Zuo,4/15/2009]
			}
			else if(strcmp(szTag,"ImgNum")==0)
			{
				m_nImgNum=atoi(szValue);
				m_nBandNum=m_nImgNum;
			}
			else if(strcmp(szTag,"Bands")==0)
			{
				m_nBandNum=atoi(szValue);
			}
			else if(strcmp(szTag,"processedImageWidth")==0)
			{
				m_nCols=atoi(szValue);
			}
			else if(strcmp(szTag,"processedImageHeight")==0)
			{
				m_nRows=atoi(szValue);
			}
			//������Ϣ
			else if(strcmp(szTag,"pixelSpacing")==0)
			{
				m_CellSize=atof(szValue);

				bGeoCoded=TRUE;
			}
			else if(strcmp(szTag,"productLowerLeftX")==0)
			{
				m_LBX=atof(szValue);
			}
			else if(strcmp(szTag,"productLowerLeftY")==0)
			{
				m_LBY=atof(szValue);
			}
			else if(strcmp(szTag,"productLowerRightX")==0)
			{
				
			}
			else if(strcmp(szTag,"productLowerRightY")==0)
			{
				
			}
			else if(strcmp(szTag,"productUpperRightX")==0)
			{
				m_RTX=atof(szValue);
			}
			else if(strcmp(szTag,"productUpperRightY")==0)
			{
				m_RTY=atof(szValue);
			}
			else if(strcmp(szTag,"productUpperLeftX")==0)
			{
				
			}
			else if(strcmp(szTag,"productUpperLeftY")==0)
			{
				
			}
		}
		if(m_nBandNum<=0||m_nImgNum<=0)
		{
			fclose(fp);

			return S_FALSE;
		}

		if(bGeoCoded==FALSE)
		{
			m_LBX=0;
			m_LBY=0;
			m_RTX=m_nCols;
			m_RTY=m_nRows;
			m_CellSize=1.0;
		}

		m_fpRaws=new FILE*[m_nImgNum];
		memset(m_fpRaws,0,sizeof(FILE*)*m_nImgNum);

		for(i=0;i<m_nImgNum;i++)
		{
			char szStdTag[100];
			sprintf(szStdTag,"outputdata%d",i);

			fseek(fp,0,SEEK_SET);
			while(!feof(fp))
			{
				fgets(szBuffer,1023,fp);
			
				sscanf(szBuffer,"%s%s%s",szTag,szEqualMark,szValue);
				if(strcmp(szTag,szStdTag)==0)
				{
					char szName[512];
					memset(szName,0,512*sizeof(char));

					int nPos=-1;
					int k=strlen(szValue)-1;
					for(;k>=0;k--)
					{
						if(szValue[k]=='\\'||szValue[k]=='/')
						{
							nPos=k;
							break;
						}
					}
					for(k=nPos+1;k<((int)(strlen(szValue)));k++)
					{
						if(szValue[k]=='\"')
						{
							break;
						}
						szName[k-nPos-1]=szValue[k];
					}
					
					char szRawPathName[512];
					strcpy(szRawPathName,szPath);
					strcat(szRawPathName,szName);

					if(m_nSensorType==SENSOR_IRS&&i==3&&m_nRows==4500&&m_nCols==4500)
					{
						char szRawPathNameTemp[512];
						sprintf(szRawPathNameTemp,"%s_resample.raw",szRawPathName);
						
						FILE* fpSrc=fopen(szRawPathName,"rb");
						FILE* fpDst=fopen(szRawPathNameTemp,"wb");
						if(fpSrc==NULL||fpDst==NULL)
						{
							Close();
							fclose(fpSrc);
							fclose(fpDst);
							return S_FALSE;
						}
						BYTE* pSrcBuffer=new BYTE[m_nCols/2*m_nBPB];
						BYTE* pDstBuffer=new BYTE[m_nCols*m_nBPB];
						int m=0,n=0;
						for(m=0;m<m_nRows/2;m++)
						{
							fread(pSrcBuffer,m_nBPB,m_nCols/2,fpSrc);
							for(n=0;n<m_nCols/2;n++)
							{
								memcpy(pDstBuffer+(n*2+0)*m_nBPB,pSrcBuffer+n*m_nBPB,m_nBPB);
								memcpy(pDstBuffer+(n*2+1)*m_nBPB,pSrcBuffer+n*m_nBPB,m_nBPB);
							}
							fwrite(pDstBuffer,m_nBPB,m_nCols,fpDst);
							fwrite(pDstBuffer,m_nBPB,m_nCols,fpDst);
						}
						fclose(fpSrc);
						fclose(fpDst);

						delete [] pSrcBuffer;
						delete [] pDstBuffer;

						strcpy(szRawPathName,szRawPathNameTemp);
					}

					m_fpRaws[i]=fopen(szRawPathName,"rb");
					if(m_fpRaws[i]==NULL)
					{
						Close();
						fclose(fp);
						return S_FALSE;
					}
				}
			}
		}

		fclose(fp);
	}
	else
	{
		GDALAllRegister();
		m_pGdalImage=(GDALDataset*)GDALOpen(m_pszPathName,GA_ReadOnly);

		if(m_pGdalImage==NULL)
		{
			return S_FALSE;
		}
		//��ȡӰ��ߴ���Ϣ
		m_nCols=GDALGetRasterXSize(m_pGdalImage);
		m_nRows=GDALGetRasterYSize(m_pGdalImage);
		m_nBandNum=GDALGetRasterCount(m_pGdalImage);
		if(m_nBandNum<=0)
		{
			return S_FALSE;
		}
		m_datatype=GDALGetRasterDataType(GDALGetRasterBand(m_pGdalImage,1));
		m_nBPB=GDALGetDataTypeSize(m_datatype)/8;
		for(int i=2;i<=m_nBandNum;i++)
		{
			if(GDALGetRasterDataType(GDALGetRasterBand(m_pGdalImage,i))!=m_datatype)
			{
				GDALClose(m_pGdalImage);
				COutput::OutputFileOpenFailed(m_pszPathName);

				return S_FALSE;
			}
		}
		//��ȡ������Ϣ
		double grdTransform[6];
		memset(grdTransform,0,sizeof(double)*6);
		CPLErr error=m_pGdalImage->GetGeoTransform(grdTransform);
		if(error==CE_None)
		{
			double X0,Y0,X2,Y2;
			X0=grdTransform[0];
			Y0=grdTransform[3];
			// 差一个像素 [ZuoW,2010/6/29]
			X2=grdTransform[0]+(m_nCols-1)*grdTransform[1]+(m_nRows-1)*grdTransform[2];
			Y2=grdTransform[3]+(m_nCols-1)*grdTransform[4]+(m_nRows-1)*grdTransform[5];

			m_LBX=__min(X0,X2);
			m_LBY=__min(Y0,Y2);
			m_RTX=__max(X0,X2);
			m_RTY=__max(Y0,Y2);
			m_CellSize=fabs(grdTransform[1]);
		}
		else
		{
			m_LBX=0;
			m_LBY=0;
			m_RTX=m_nCols;
			m_RTY=m_nRows;
			m_CellSize=1.0;
		}
	}

	//��ʼ��������
	m_nCacheRow=m_CacheSize/(m_nCols*m_nBandNum*m_nBPB);
	if(m_nCacheRow<=0)
	{
		if(m_pCache)
		{
			delete [] m_pCache;
		}
		m_pCache=NULL;
		BOOL bMemEnough=FALSE;
		int nRow=24000;
		while(bMemEnough==FALSE&&nRow>1)
		{
			try
			{
				m_nCacheRow=nRow;
				m_CacheSize=m_nCols*m_nBandNum*m_nBPB*m_nCacheRow;
				m_pCache=new BYTE[m_CacheSize];
				bMemEnough=TRUE;
			}
			catch(...)
			{
				nRow=nRow/2;
				if(nRow<=0)
				{
					nRow=1;
				}
			}
		}
		if(m_pCache==NULL)
		{
			try
			{
				m_pCache=new BYTE[m_nCols*m_nBandNum*m_nBPB*1];
				m_CacheSize=m_nCols*m_nBandNum*m_nBPB*1;
				m_nCacheRow=1;
			}
			catch(...)
			{
				COutput::OutputMemoryError();
				Close();
				return S_FALSE;
			}
		}
	}

	if((uMode&modeAqlut)==modeAqlut)
	{
		//here pHistogram mast write as follows
		CHistogram* pHistogram=new CHistogram[m_nBandNum];
		for(int i=0;i<m_nBandNum;i++)
		{
			pHistogram[i].StatHistogram(this,i);
		}
		m_pHistogram=pHistogram;
		pHistogram=NULL;
	}
	
	return S_OK;
}

HRESULT CImage::CreateImg(char* pszFilePath, UINT uMode, 
						  int Cols, int Rows, 
						  UINT nDataType, 
						  int nBandNum, 
						  UINT BandType,
						  double xStart,double yStart,double cellSize)
{
	if(m_pszPathName!=NULL)
	{
		delete [] m_pszPathName;
		m_pszPathName=NULL;
	}
	m_pszPathName=new char[strlen(pszFilePath)+1];
	strcpy(m_pszPathName,pszFilePath);

	GDALAllRegister(); 

	
	char szDriver[]="GTiff";

    GDALDriver* pDriver=(GDALDriver*)GDALGetDriverByName(szDriver);
	if(pDriver==NULL)
	{
		COutput::OutputImageDriverCreatedFailed();
		return S_FALSE;
	}

	m_nRows=Rows;
	m_nCols=Cols;
	m_nBandNum=nBandNum;
	m_datatype=ConvertDataType2GDALDataType(nDataType);
	m_nBPB=GDALGetDataTypeSize(m_datatype)/8;

	// ����һ������ݼ�(����ΪTIFFӰ��)
	// Ӱ��ĳ�/��Ϊԭ4��һ��
	// ����INTERLEAVE=PIXELָ��Ӱ����RGBRGBRGB��ʽ���
	// (Ĭ����RRRGGGBBB��ʽ)
	//char *papszParmList[] = { "INTERLEAVE=PIXEL", NULL };
	m_pGdalImage=pDriver->Create(m_pszPathName,
								m_nCols,m_nRows,
								m_nBandNum,
								m_datatype,
								NULL);
	if(m_pGdalImage==NULL)
	{
		COutput::OutputFileOpenFailed(m_pszPathName);

		return S_FALSE;
	}
	
	double grdTransform[6];
	if(m_bFlip)
	{
		grdTransform[0]=xStart;					grdTransform[1]=cellSize;	grdTransform[2]=0.0;
		grdTransform[3]=yStart+Rows*cellSize;	grdTransform[4]=0.0;		grdTransform[5]=0.0-cellSize;
	}
	else
	{
		grdTransform[0]=xStart;	grdTransform[1]=cellSize;	grdTransform[2]=0.0;
		grdTransform[3]=yStart;	grdTransform[4]=0.0;		grdTransform[5]=cellSize;
	}
	
	m_pGdalImage->SetGeoTransform(grdTransform);

	double X0,Y0,X2,Y2;
	X0=grdTransform[0];
	Y0=grdTransform[3];
	X2=grdTransform[0]+Cols*grdTransform[1]+Rows*grdTransform[2];
	Y2=grdTransform[3]+Cols*grdTransform[4]+Rows*grdTransform[5];

	m_LBX=__min(X0,X2);
	m_LBY=__min(Y0,Y2);
	m_RTX=__max(X0,X2);
	m_RTY=__max(Y0,Y2);
	m_CellSize=fabs(grdTransform[1]);
	
	return S_OK;
}

HRESULT CImage::Close()
{
	if(m_fpRaws!=NULL)
	{
		for(int i=0;i<m_nImgNum;i++)
		{
			if(m_fpRaws[i]!=NULL)
			{
				fclose(m_fpRaws[i]);
			}
		}
		delete [] m_fpRaws;
		m_fpRaws=NULL;
	}
	if(m_pGdalImage)
	{
		GDALClose(m_pGdalImage);
		m_pGdalImage=NULL;
	}
	//��ջ�����
	if(m_pCache)
	{
		delete [] m_pCache;
		m_pCache=NULL;
	}
	m_pCache=NULL;
	m_CacheSize=0;
	m_nCacheRow=-1;
	m_nCacheRow0=-1;
	m_nCacheRow1=-1;

	if(m_pszPathName!=NULL)
	{
		delete [] m_pszPathName;
		m_pszPathName=NULL;
	}

	if(m_pHistogram!=NULL)
	{
		delete [] m_pHistogram;
		m_pHistogram=NULL;
	}

	return S_OK;
}

HRESULT CImage::IsGeoCoded()
{
	if(m_pGdalImage==NULL&&m_fpRaws==NULL)
	{
		return S_FALSE;
	}

	if( fabs(m_LBX-0.0)<1e-6 &&
		fabs(m_LBY-1.0)<1e-6)
	{
		return S_FALSE;
	}
	else
	{
		return S_OK;
	}

	return S_FALSE;
}

HRESULT CImage::GetBandFormat(UINT* pBandFormat)
{
	if(m_pGdalImage!=NULL)
	{
		*pBandFormat=BIP;

		return S_OK;
	}
	else if(m_fpRaws!=NULL)
	{
		*pBandFormat=BSQ;

		return S_OK;
	}

	return S_FALSE;
}

HRESULT CImage::SetBandForamt(UINT nBandFormat)
{

	return S_FALSE;
}

HRESULT CImage::GetBandNum(int* pBandNum)
{
	if(m_pGdalImage!=NULL||m_fpRaws!=NULL)
	{
		*pBandNum=m_nBandNum;
		return S_OK;
	}
	
	return S_FALSE;
}

HRESULT CImage::SetBandNum(int nBandNum)
{

	return S_FALSE;
}

HRESULT CImage::GetRows(int* pRows)
{
	if(m_pGdalImage!=NULL||m_fpRaws!=NULL)
	{
		*pRows=m_nRows;

		return S_OK;
	}

	return S_FALSE;
}

HRESULT CImage::SetRows(int nRows)
{

	return S_FALSE;
}

HRESULT CImage::GetCols(int* pCols)
{
	if(m_pGdalImage!=NULL||m_fpRaws!=NULL)
	{
		*pCols=m_nCols;

		return S_OK;
	}

	return S_FALSE;
}

HRESULT CImage::SetCols(int nCols)
{

	return S_FALSE;
}

HRESULT CImage::GetDataType(UINT* pDataType)
{
	if(m_pGdalImage!=NULL||m_fpRaws!=NULL)
	{
		if(m_pHistogram==NULL)
		{
			*pDataType=ConvertGDALDataType2DataType(m_datatype);
		}
		else
		{
			*pDataType=Pixel_Byte;
		}
		
		return S_OK;
	}

	return S_FALSE;
}

HRESULT CImage::SetDataType(UINT nDataType)
{

	return S_FALSE;
}

HRESULT CImage::GetByteOrder(UINT* pByteOrder)
{

	return S_FALSE;
}

HRESULT CImage::SetByteOrder(UINT nByteOrder)
{

	return S_FALSE;
}

HRESULT CImage::GetScale(float* pScale)
{

	return S_FALSE;
}

HRESULT CImage::SetScale(float Scale)
{

	return S_FALSE;
}

HRESULT CImage::GetGrdInfo(double* xStart,double* yStart,double* cellSize)
{
	if(m_pGdalImage!=NULL||m_fpRaws!=NULL)
	{
		*xStart=m_LBX;
		*yStart=m_LBY;
		*cellSize=m_CellSize;

		return S_OK;
	}

	return S_FALSE;
}

HRESULT CImage::SetGrdInfo(double xStart,double yStart,double cellSize)
{
	if(m_pGdalImage)
	{
		double grdTransform[6];
		grdTransform[0]=xStart;						grdTransform[1]=cellSize;	grdTransform[2]=0.0;
		grdTransform[3]=yStart+m_nRows*cellSize;	grdTransform[4]=0.0;		grdTransform[5]=0.0-cellSize;
	
		m_pGdalImage->SetGeoTransform(grdTransform);

		return S_OK;
	}

	return S_FALSE;
}

HRESULT CImage::HaveColorTable()
{
	if(m_pGdalImage)
	{
		GDALRasterBand  *pBand=NULL;
		for(int i=0;i<m_nBandNum;i++)
		{
			pBand=m_pGdalImage->GetRasterBand(i+1);
			if(pBand)
			{
				if(pBand->GetColorTable()!=NULL)
				{
					return S_OK;
				}
			}
		}
	}

	return S_FALSE;
}

HRESULT CImage::GetEntryNum(int* pEntryNum)
{
	if(m_pGdalImage)
	{
		GDALRasterBand  *pBand=NULL;
		for(int i=0;i<m_nBandNum;i++)
		{
			pBand=m_pGdalImage->GetRasterBand(i+1);
			if(pBand)
			{
				if(pBand->GetColorTable()!=NULL)
				{
					*pEntryNum=pBand->GetColorTable()->GetColorEntryCount();
					return S_OK;
				}
			}
		}
	}

	return S_OK;
}

HRESULT CImage::GetColorTable(BYTE* pColorTable)
{
	if(m_pGdalImage)
	{
		GDALRasterBand  *pBand=NULL;
		for(int i=0;i<m_nBandNum;i++)
		{
			pBand=m_pGdalImage->GetRasterBand(i+1);
			if(pBand)
			{
				GDALColorTable* pGDALColorTable=pBand->GetColorTable();
				if(pGDALColorTable!=NULL)
				{
					int nEntryNum=pGDALColorTable->GetColorEntryCount();
					GDALColorEntry Entry;
					for(int i=0;i<nEntryNum;i++)
					{
						pGDALColorTable->GetColorEntryAsRGB(i,&Entry);
						pColorTable[i*4+0]=(BYTE)Entry.c3;
						pColorTable[i*4+1]=(BYTE)Entry.c2;
						pColorTable[i*4+2]=(BYTE)Entry.c1;
						pColorTable[i*4+3]=(BYTE)Entry.c4;
					}

					return S_OK;
				}
			}
		}
	}

	return S_FALSE;
}

HRESULT CImage::SetColorTable(BYTE* pColorTable,int nEntryNum)
{

	return S_FALSE;
}

HRESULT CImage::GetResampleMethod(int* pResampleMethod)
{
	*pResampleMethod=m_nResampleMethord;

	return S_OK;
}

HRESULT CImage::SetResampleMethod(int ResampleMethod)
{
	m_nResampleMethord=ResampleMethod;

	return S_OK;
}

HRESULT CImage::GetScanSize(float* pScanSize)
{
	*pScanSize=0.025f;

	return S_FALSE;
}

HRESULT CImage::SetScanSize(float ScanSize)
{
	return S_FALSE;
}

HRESULT CImage::GetBPB(int* pBPB)
{
	if(m_pGdalImage!=NULL||m_fpRaws!=NULL)
	{
		if(m_pHistogram==NULL)
		{
			*pBPB=m_nBPB;
		}
		else
		{
			*pBPB=1;
		}

		return S_OK;
	}

	return S_FALSE;
}

HRESULT CImage::SetBPB(int nBPB)
{

	return S_FALSE;
}

HRESULT CImage::GetBPP(int* pBPP)
{
	if(m_pGdalImage!=NULL||m_fpRaws!=NULL)
	{
		if(m_pHistogram==NULL)
		{
			*pBPP=m_nBPB*m_nBandNum;
		}
		else
		{
			*pBPP=m_nBandNum;
		}

		return S_OK;
	}

	return S_FALSE;
}

HRESULT CImage::SetBPP(int nBPP)
{

	return S_FALSE;
}

HRESULT CImage::GetPathName(char** ppszPathName)
{
	if(m_pGdalImage!=NULL||m_fpRaws!=NULL)
	{
		*ppszPathName=new char[strlen(m_pszPathName)+1];
		strcpy(*ppszPathName,m_pszPathName);

		return S_OK;
	}

	return S_FALSE;
}

HRESULT CImage::GetPixel(int nRows, int nCols, BYTE* pPixel)
{
	if(m_pGdalImage==NULL&&m_fpRaws==NULL)
	{
		return S_FALSE;
	}

	if(m_bFlip)
	{
		nRows=m_nRows-nRows-1;
	}

	if(nRows<0||nRows>=m_nRows||nCols<0||nCols>=m_nCols)
	{
		memset(pPixel,0,m_nBPB*m_nBandNum);
		return S_OK;
	}

	/*if(m_fpRaws!=NULL)
	{
		if(m_nBandNum==m_nImgNum)
		{
			unsigned int nOffset=(nRows*m_nCols+nCols)*m_nBPB;
			for(int i=0;i<m_nBandNum;i++)
			{
				fseek(m_fpRaws[i],nOffset,SEEK_SET);
				fread(pPixel+i*m_nBPB,m_nBPB,1,m_fpRaws[i]);
			}
		}
		else
		{
			for(int i=0;i<m_nBandNum;i++)
			{
				unsigned int nOffset=(nRows*m_nCols+nCols)*m_nBPB+i*m_nRows*m_nCols*m_nBPB;
				fseek(m_fpRaws[0],nOffset,SEEK_SET);
				fread(pPixel+i*m_nBPB,m_nBPB,1,m_fpRaws[0]);
			}
		}

		return S_OK;
	}
	else if(m_pGdalImage!=NULL)*/
	{
		if(nRows<m_nCacheRow0||nRows>=m_nCacheRow1)
		{
			ResetCache(nRows,nCols);
		}

		UINT datatype=ConvertGDALDataType2DataType(m_datatype);
		int nBPB;
		GetBPB(&nBPB);

		if(m_pHistogram==NULL)
		{
			for(int i=0;i<m_nBandNum;i++)
			{
				memcpy(pPixel+i*m_nBPB,
						m_pCache+i*m_nCacheBandSize+(nRows-m_nCacheRow0)*m_nCacheScanSize+nCols*m_nBPB,
						m_nBPB);
			}
		}
		else
		{
			for(int i=0;i<m_nBandNum;i++)
			{
				pPixel[i]=m_pHistogram[i].LUT(m_pCache+i*m_nCacheBandSize+(nRows-m_nCacheRow0)*m_nCacheScanSize+nCols*m_nBPB,datatype);
			}
		}
		
	}
	
	return S_OK;
}

HRESULT CImage::SetPixel(int nRows, int nCols, BYTE* pPixel)
{
	if(m_pGdalImage==NULL)
	{
		return S_FALSE;
	}

	if(m_bFlip)
	{
		nRows=m_nRows-nRows-1;
	}

	if(nRows<0||nRows>=m_nRows||nCols<0||nCols>=m_nCols)
	{
		return S_OK;
	}

	for(int i=0;i<m_nBandNum;i++)
	{
		int band=i+1;

		GDALDataType datatype=m_datatype;
		int nBPB=m_nBPB;
		
		BYTE* pGray=new BYTE[nBPB];
		memcpy(pGray,pPixel+i*nBPB,nBPB);
		((GDALRasterBand*)GDALGetRasterBand(m_pGdalImage,band))->
		RasterIO(GF_Write,
				 nCols,nRows,
				 1,1,
				 pGray,
				 1,1,
				 datatype,
				 0,0);

		delete [] pGray; pGray=NULL;
	}

	return S_OK;
}

HRESULT CImage::GetGray(int nRows, int nCols, int nBand, BYTE* gray)
{
	if(m_pGdalImage==NULL&&m_fpRaws==NULL)
	{
		return S_FALSE;
	}

	if(m_bFlip)
	{
		nRows=m_nRows-nRows-1;
	}

	if(nRows<0||nRows>=m_nRows||nCols<0||nCols>=m_nCols)
	{
		memset(gray,0,m_nBPB);
		return S_OK;
	}
	
	if(nRows<m_nCacheRow0||nRows>=m_nCacheRow1)
	{
		ResetCache(nRows,nCols);
	}

	UINT datatype=ConvertGDALDataType2DataType(m_datatype);
	int nBPB;
	GetBPB(&nBPB);

	if(m_pHistogram==NULL)
	{
		memcpy(gray,
				m_pCache+nBand*m_nCacheBandSize+(nRows-m_nCacheRow0)*m_nCacheScanSize+nCols*m_nBPB,
				m_nBPB);
	}
	else
	{
		*gray=m_pHistogram[nBand].LUT(m_pCache+nBand*m_nCacheBandSize+(nRows-m_nCacheRow0)*m_nCacheScanSize+nCols*m_nBPB,datatype);
	}

	return S_OK;
}

HRESULT CImage::SetGray(int nRows, int nCols, int nBand, BYTE* gray)
{
	if(m_pGdalImage==NULL)
	{
		return S_FALSE;
	}

	if(m_bFlip)
	{
		nRows=m_nRows-nRows-1;
	}

	if(nRows<0||nRows>=m_nRows||nCols<0||nCols>=m_nCols)
	{
		return S_OK;
	}

	int band=nBand+1;
	GDALDataType datatype=m_datatype;

	((GDALRasterBand*)GDALGetRasterBand(m_pGdalImage,band))->
	RasterIO(GF_Write,
			 nCols,nRows,
			 1,1,
			 gray,
			 1,1,
			 datatype,
			 0,0);

	return S_OK;
}

HRESULT CImage::GetPixelF(float x, float y, BYTE* pPixel, UINT nResampleMethod)
{
	if(m_pGdalImage==NULL&&m_fpRaws==NULL)
	{
		return S_FALSE;
	}

	if(nResampleMethod == 0)
	{
		int nCol, nRow;
	
		nCol = int(x + 0.5);
		nRow = int(y + 0.5);

		if(nCol >= 0 && nCol < m_nCols && nRow >= 0 && nRow < m_nRows)
		{
			GetPixel(nRow,nCol , pPixel);
		}
		else
		{
			memset(pPixel,0,m_nBPB*m_nBandNum);
		}
	}
	else
	{
		int nCol1, nCol2, nRow1, nRow2;
		float Dx, Dy;
	
		BYTE pBuf11[512], pBuf12[512], pBuf21[512], pBuf22[512];

		nCol1 = int(x);
		nRow1 = int(y);
		nCol2 = nCol1 + 1;
		nRow2 = nRow1 + 1;
		//be sure the second pixel in image
		nCol2=__min(nCol2,m_nCols-1);
		nRow2=__min(nRow2,m_nRows-1);

		Dx = x - nCol1;
		Dy = y - nRow1;

		UINT nDataType;
		GetDataType(&nDataType);
		int nBPB;
		GetBPB(&nBPB);

		if(nRow1 >= 0 && nRow1 < m_nRows && nCol1 >= 0 && nCol1 < m_nCols)
		{
			GetPixel(nRow1,nCol1 , pBuf11);
			GetPixel(nRow1,nCol2 , pBuf21);
			GetPixel(nRow2,nCol1 , pBuf12);
			GetPixel(nRow2,nCol2 , pBuf22);

			for(int i = 0;i<m_nBandNum;i++)
			{
				(* FuncTable_GDAL[nDataType])(pBuf11 + nBPB *i,
											 pBuf21 + nBPB *i,
											 pBuf22 + nBPB *i,
											 pBuf12 + nBPB *i,
											 Dx,Dy,
											 pPixel  + nBPB *i);
			}
		}
		else
		{
			memset(pPixel,0,nBPB*m_nBandNum);
		}
	}

	return S_OK;
}

HRESULT CImage::GetGrayF(float x, float y, int nBand, BYTE* pGray, int nResampleMethod)
{
	if(m_pGdalImage==NULL&&m_fpRaws==NULL)
	{
		return S_FALSE;
	}

	if(nResampleMethod == 0)
	{
		int nCol, nRow;
	
		nCol = int(x + 0.5);
		nRow = int(y + 0.5);

		if(nCol >= 0 && nCol < m_nCols && nRow >= 0 && nRow < m_nRows)
		{
			GetGray(nRow,nCol,nBand,pGray);
		}
		else
		{
			memset(pGray,0,m_nBPB);
		}
	}
	else
	{
		int nCol1, nCol2, nRow1, nRow2;
		float Dx, Dy;
	
		static BYTE pBuf11[512], pBuf12[512], pBuf21[512], pBuf22[512];	//, pBufDest[512];

		nCol1 = int(x);
		nRow1 = int(y);
		nCol2 = nCol1 + 1;
		nRow2 = nRow1 + 1;
		//be sure the second pixel in image
		nCol2=__min(nCol2,m_nCols-1);
		nRow2=__min(nRow2,m_nRows-1);

		Dx = x - nCol1;
		Dy = y - nRow1;

		UINT nDataType;
		GetDataType(&nDataType);
		int nBPB;
		GetBPB(&nBPB);

		if(nRow1 >= 0 && nRow1 < m_nRows && nCol1 >= 0 && nCol1 < m_nCols)
		{
			GetGray(nRow1,nCol1,nBand,pBuf11);
			GetGray(nRow1,nCol2,nBand,pBuf21);
			GetGray(nRow2,nCol1,nBand,pBuf12);
			GetGray(nRow2,nCol2,nBand,pBuf22);

			(* FuncTable_GDAL[nDataType])(pBuf11,
										 pBuf21,
										 pBuf22,
										 pBuf12,
										 Dx,Dy,
										 pGray);
		}
		else
		{
			memset(pGray,0,nBPB);
		}
	}

	return S_OK;
}

HRESULT CImage::ReadImgBand(float fSrcLeft, float fSrcTop, float fSrcRight, float fSrcBottom, 
							   BYTE* pBuf, int nBufWidth, int nBufHeight, int nBandNum, 
							   int nDestLeft, int nDestTop, int nDestRight, int nDestBottom, 
							   int nSrcSkip, int nDestSkip)
{
	if(m_bFlip)
	{
		float tempTop=m_nRows-fSrcBottom;
		float tempBottom=m_nRows-fSrcTop;
		fSrcTop=tempTop;
		fSrcBottom=tempBottom;
	}

	if(fSrcBottom-fSrcTop<m_nCacheRow)
	{
		float xScale=(fSrcRight-fSrcLeft)/(nDestRight-nDestLeft);
		float yScale=(fSrcBottom-fSrcTop)/(nDestBottom-nDestTop);
		
		for(int i=nDestTop;i<nDestBottom;i++)
		{
			int nImageRow=(int)((i-nDestTop)*yScale+fSrcTop+0.5);
			if(nImageRow<0||nImageRow>=m_nRows)
			{
				continue;
			}
			if(nImageRow<m_nCacheRow0||nImageRow>=m_nCacheRow1)
			{
				
				ResetCache(nImageRow,0);
			}

			int nRowIndex=i;
			if(m_bFlip)
			{
				nRowIndex=nBufHeight-nRowIndex-1;
			}
			BYTE* pBufIndex=pBuf+nRowIndex*nBufWidth*m_nBPB*nBandNum+nDestSkip*m_nBPB;
			BYTE* pCacheIndex=m_pCache+nSrcSkip*m_nCacheBandSize+(nImageRow-m_nCacheRow0)*m_nCacheScanSize;
			for(int j=nDestLeft;j<nDestRight;j++)
			{
				int nImageCol=(int)((j-nDestLeft)*xScale+fSrcLeft+0.5);
				if(nImageCol<0||nImageCol>=m_nCols)
				{
					continue;
				}
				memcpy(pBufIndex+j*nBandNum*m_nBPB,pCacheIndex+nImageCol*m_nBPB,m_nBPB);
			}
		}
	}
	else
	{
		int band=nSrcSkip+1;
		GDALDataType datatype=m_datatype;
		int nBPB=m_nBPB;

		int offset=nDestSkip*nBPB;
		
		int nLeft=(int)(fSrcLeft+0.5);
		int nTop=(int)(fSrcTop+0.5);
		int nRight=(int)(fSrcRight+0.5);
		int nBottom=(int)(fSrcBottom+0.5);

		int nXOffset=0;
		if(nLeft<0)
		{
			nXOffset=0-nLeft;
			nLeft=0;
		}
		else if(nLeft>=m_nCols)
		{
			return S_OK;
		}
		if(nRight>m_nCols)
		{
			nRight=m_nCols;
		}
		else if(nRight<0)
		{
			return S_OK;
		}
		int nYOffset=0;
		if(nTop<0)
		{
			nYOffset=0-nTop;
			nTop=0;
		}
		else if(nTop>=m_nRows)
		{
			return S_OK;
		}
		if(nBottom>m_nRows)
		{
			nBottom=m_nRows;
		}
		else if(nBottom<0)
		{
			return S_OK;
		}

		int nWidth=nRight-nLeft;
		int nHeight=nBottom-nTop;
		float xscale=(float)nBufWidth/(fSrcRight-fSrcLeft);
		float yscale=(float)nBufHeight/(fSrcBottom-fSrcTop);
		int nTempBufWidth=(int)(nWidth*xscale+0.5);
		int nTempBufHeight=(int)(nHeight*yscale+0.5);
		nXOffset=(int)(nXOffset*xscale+0.5);
		nYOffset=(int)(nYOffset*yscale+0.5);

		BYTE* pTempBuf=new BYTE[nTempBufWidth*nTempBufHeight*nBPB];
		memset(pTempBuf,255,nTempBufWidth*nTempBufHeight*nBPB);

		if(m_pGdalImage!=NULL)
		{
			CPLErr error=((GDALRasterBand*)GDALGetRasterBand(m_pGdalImage,band))->
			RasterIO(GF_Read,
					 nLeft,nTop,
					 nWidth,nHeight,
					 pTempBuf,
					 nTempBufWidth,nTempBufHeight,
					 datatype,
					 0,0);
		}
		else if(m_fpRaws!=NULL)
		{	
			BYTE* pTmp=new BYTE[nWidth*m_nBPB];
			for(int i=0;i<nTempBufHeight;i++)
			{
				unsigned long nOffset=((int)(i/yscale+0.5)+nTop)*m_nCols*m_nBPB;
				BYTE* pTempBufIndex=pTempBuf+i*nTempBufWidth*m_nBPB;

				if(m_nBandNum==m_nImgNum)//seprate
				{
					fseek(m_fpRaws[nSrcSkip],nOffset+((int)(nLeft/xscale+0.5))*m_nBPB,SEEK_SET);
					fread(pTmp,nWidth*m_nBPB,1,m_fpRaws[nSrcSkip]);
				}
				else//bsq
				{
					fseek(m_fpRaws[0],nOffset+((int)(nLeft/xscale+0.5))*m_nBPB+nSrcSkip*m_nRows*m_nCols*m_nBPB,SEEK_SET);
					fread(pTmp,nWidth*m_nBPB,1,m_fpRaws[0]);
				}

				int j=0;
				for(j=0;j<nTempBufWidth;j++)
				{
					//pTempBufIndex[j]=pTmp[(int)(j/xscale+0.5)];
					memcpy(pTempBufIndex+j*m_nBPB,pTmp+(int)(j/xscale+0.5)*m_nBPB,m_nBPB);
				}
			}
			delete [] pTmp;
			pTmp=NULL;
		}
		
		offset=offset+nXOffset*nBPB*nBandNum;
		for(int i=0;i<nTempBufHeight;i++)
		{
			int nRowIndex=i+nYOffset;
			if(m_bFlip)
			{
				nRowIndex=nBufHeight-nRowIndex-1;
			}
			if(nRowIndex>=nBufHeight||nRowIndex<0)
			{
				continue;
			}
			BYTE* pBufIndex=pBuf+nRowIndex*nBufWidth*nBPB*nBandNum;
			BYTE* pTempBufIndex=pTempBuf+i*nTempBufWidth*nBPB;
			for(int j=0;j<nTempBufWidth;j++)
			{
				if(j+nXOffset>=nBufWidth)
				{
					break;
				}
				memcpy(pBufIndex+offset,pTempBufIndex,nBPB);
				pBufIndex=pBufIndex+nBPB*nBandNum;
				pTempBufIndex=pTempBufIndex+nBPB;
			}
		}
		delete [] pTempBuf; pTempBuf=NULL;
	}
	
	return S_OK;
}

HRESULT CImage::ReadImg(float fSrcLeft, float fSrcTop, float fSrcRight, float fSrcBottom, 
									 BYTE* pBuf, int nBufWidth, int nBufHeight, int nBandNum, 
									 int nDestLeft, int nDestTop, int nDestRight, int nDestBottom, 
									 int nSrcSkip, int nDestSkip)
{
	if(m_pGdalImage==NULL&&m_fpRaws==NULL)
	{
		return S_FALSE;
	}
	
	if(m_pHistogram==NULL)
	{
		if(nSrcSkip==-1)
		{
			for(int band=0;band<nBandNum;band++)
			{
				ReadImgBand(fSrcLeft,fSrcTop,fSrcRight,fSrcBottom,pBuf,nBufWidth,nBufHeight,nBandNum,nDestLeft,nDestTop,nDestRight,nDestBottom,band,nDestSkip+band);
			}
		}
		else
		{
			ReadImgBand(fSrcLeft,fSrcTop,fSrcRight,fSrcBottom,pBuf,nBufWidth,nBufHeight,nBandNum,nDestLeft,nDestTop,nDestRight,nDestBottom,nSrcSkip,nDestSkip);
		}
	}
	else
	{
		int nBPB=m_nBPB;
		int nImgBandNum=m_nBandNum;
		UINT datatype=ConvertGDALDataType2DataType(m_datatype);

		BYTE* pTmpBuffer=new BYTE[nBufWidth*nBufHeight*nBPB];
		
		if(nSrcSkip==-1)
		{
			for(int band=0;band<nImgBandNum;band++)
			{
				ReadImgBand(fSrcLeft,fSrcTop,fSrcRight,fSrcBottom,
							pTmpBuffer,
							nBufWidth,nBufHeight,1,
							nDestLeft,nDestTop,nDestRight,nDestBottom,
							band,
							0);

				for(int i=nDestTop;i<nDestBottom;i++)
				{
					BYTE* pBufIndex=pBuf+i*nBufWidth*nBandNum;
					BYTE* pTmpBufIndex=pTmpBuffer+i*nBufWidth*nBPB;

					for(int j=nDestLeft;j<nDestRight;j++)
					{
						pBufIndex[j*nBandNum+nDestSkip+band]=m_pHistogram[band].LUT(&pTmpBufIndex[j*nBPB],datatype);
					}
				}
			}
		}
		else
		{
			ReadImgBand(fSrcLeft,fSrcTop,fSrcRight,fSrcBottom,
							pTmpBuffer,
							nBufWidth,nBufHeight,1,
							nDestLeft,nDestTop,nDestRight,nDestBottom,
							nSrcSkip,
							0);
			for(int i=nDestTop;i<nDestBottom;i++)
			{
				BYTE* pBufIndex=pBuf+i*nBufWidth*nBandNum;
				BYTE* pTmpBufIndex=pTmpBuffer+i*nBufWidth*nBPB;

				for(int j=nDestLeft;j<nDestRight;j++)
				{
					pBufIndex[j*nBandNum+nDestSkip]=m_pHistogram[nSrcSkip].LUT(&pTmpBufIndex[j*nBPB],datatype);
				}
			}
		}
		
		delete [] pTmpBuffer;
		pTmpBuffer=NULL;
	}

	return S_OK;
}

HRESULT CImage::WriteImgBand(float fSrcLeft, float fSrcTop, float fSrcRight, float fSrcBottom, 
							 BYTE* pBuf, 
							 int nBufWid, int nBufHeight, 
							 int nBandNum, 
							 int nDestLeft, int nDestTop, int nDestRight, int nDestBottom, 
							 int nSrcSkip, int nDestSkip)
{
	int band=nSrcSkip+1;
	GDALDataType datatype=m_datatype;
	int nBPB=m_nBPB;
	int nBPP=nBPB*nBandNum;

	BYTE* pBandBuf=new BYTE[nBufWid*nBufHeight*nBPB];
	if(m_bFlip)
	{
		float tempTop=m_nRows-fSrcBottom;
		float tempBottom=m_nRows-fSrcTop;
		fSrcTop=tempTop;
		fSrcBottom=tempBottom;

		for(int i=0;i<nBufHeight;i++)
		{
			BYTE* pBandBufIndex=pBandBuf+(nBufHeight-i-1)*nBufWid*nBPB;
			BYTE* pBufIndex=pBuf+i*nBufWid*nBPP+nDestSkip*nBPB;
			for(int j=0;j<nBufWid;j++)
			{
				memcpy(pBandBufIndex+j*nBPB,pBufIndex+j*nBPP,nBPB);
			}
		}
	}
	else
	{
		for(int i=0;i<nBufWid*nBufHeight;i++)
		{
			memcpy(pBandBuf+i*nBPB,pBuf+i*nBPP+nDestSkip*nBPB,nBPB);
		}
	}

	int nLeft=(int)(fSrcLeft+0.5);
	int nTop=(int)(fSrcTop+0.5);
	int nRight=(int)(fSrcRight+0.5);
	int nBottom=(int)(fSrcBottom+0.5);

	int nWidth=nRight-nLeft;
	int nHeight=nBottom-nTop;
	
	CPLErr error=((GDALRasterBand*)GDALGetRasterBand(m_pGdalImage,band))->
	RasterIO(GF_Write,
			 nLeft,nTop,
			 nWidth,nHeight,
			 pBandBuf,
			 nWidth,nHeight,
			 datatype,
			 0,0);

	delete [] pBandBuf; pBandBuf=NULL;

	return S_OK;
}

HRESULT CImage::WriteImg(float fSrcLeft, float fSrcTop, float fSrcRight, float fSrcBottom, 
						 BYTE* pBuf, int nBufWid, int nBufHeight, int nBandNum, 
						 int nDestLeft, int nDestTop, int nDestRight, int nDestBottom, 
						 int nSrcSkip, int nDestSkip)
{
	if(m_pGdalImage==NULL)
	{
		return S_FALSE;
	}

	if(nSrcSkip==-1)
	{
		for(int band=0;band<nBandNum;band++)
		{
			WriteImgBand(fSrcLeft,fSrcTop,fSrcRight,fSrcBottom,pBuf,nBufWid,nBufHeight,nBandNum,nDestLeft,nDestTop,nDestRight,nDestBottom,band,nDestSkip+band);
		}
	}
	else
	{
		WriteImgBand(fSrcLeft,fSrcTop,fSrcRight,fSrcBottom,pBuf,nBufWid,nBufHeight,nBandNum,nDestLeft,nDestTop,nDestRight,nDestBottom,nSrcSkip,nDestSkip);
	}
	return S_OK;
}

HRESULT CImage::SetProgressInterface(void* pIUnknown)
{

	return S_FALSE;
}

HRESULT CImage::GetSupExts(char** ppszExts,UINT flags)
{	

	return S_FALSE;
}

HRESULT CImage::Image2World(float x,float y,double* pX,double* pY)
{
	if(m_pGdalImage!=NULL||m_fpRaws!=NULL)
	{
		*pX=m_LBX+x*m_CellSize;
		*pY=m_LBY+y*m_CellSize;

		return S_OK;
	}

	return S_FALSE;
}

HRESULT CImage::World2Image(double X,double Y,float* px,float* py)
{
	if(m_pGdalImage!=NULL||m_fpRaws!=NULL)
	{
		*px=(float)((X-m_LBX)/m_CellSize);
		*py=(float)((Y-m_LBY)/m_CellSize);

		return S_OK;
	}

	return S_FALSE;
}

HRESULT CImage::GetDefaultBand(int* nBandIndex)
{
	if(nBandIndex!=NULL)
	{
		if(m_nBandNum>=3)
		{
			nBandIndex[0]=2;
			nBandIndex[1]=1;
			nBandIndex[2]=0;
		}
		else
		{
			nBandIndex[0]=0;
			nBandIndex[1]=0;
			nBandIndex[2]=0;
		}
		
		return S_OK;
	}

	return S_FALSE;
}

GDALDataType CImage::ConvertDataType2GDALDataType(UINT datatype)
{
	switch(datatype)
	{
	case Pixel_Byte:
		return GDT_Byte;
	case Pixel_Int16:
		return GDT_UInt16;
	case Pixel_SInt16:
		return GDT_Int16;
	case Pixel_Int32:
		return GDT_UInt32;
	case Pixel_SInt32:
		return GDT_Int32;
	case Pixel_Float:
		return GDT_Float32;
	case Pixel_Double:
		return GDT_Float64;
	default:
		COutput::OutputPixelTypeError();
		return GDT_Unknown;
	}
}

UINT CImage::ConvertGDALDataType2DataType(GDALDataType gdalDataType)
{
	switch(gdalDataType)
	{
	case GDT_Byte:
		return Pixel_Byte;
	case GDT_UInt16:
		return Pixel_Int16;
	case GDT_Int16:
		return Pixel_SInt16;
	case GDT_UInt32:
		return Pixel_Int32;
	case GDT_Int32:
		return Pixel_SInt32;
	case GDT_Float32:
		return Pixel_Float;
	case GDT_Float64:
		return Pixel_Double;
	default:
		COutput::OutputPixelTypeError();
		return Pixel_Byte;
	}
}

BOOL CImage::ResetCache(int nRow,int nCol)
{
	//��ݵ�ǰλ�����û�����Χ
	m_nCacheRow0=nRow-m_nCacheRow/2;
	if(m_nCacheRow0<0)
	{
		m_nCacheRow0=0;
	}
	m_nCacheRow1=m_nCacheRow0+m_nCacheRow;
	m_nCacheScanSize=m_nCols*m_nBPB;
	m_nCacheBandSize=(m_nCacheRow1-m_nCacheRow0)*m_nCacheScanSize;

	if(m_nCacheRow1>m_nRows)
	{
		m_nCacheRow1=m_nRows;
	}
	if(m_pGdalImage==NULL&&m_fpRaws!=NULL)
	{
		if(m_nBandNum==m_nImgNum)
		{
			for(int i=0;i<m_nBandNum;i++)
			{
				if(m_fpRaws[i]==NULL)
				{
					return FALSE;
				}
				unsigned long nOffset=m_nCacheRow0*m_nCols*m_nBPB;
				fseek(m_fpRaws[i],nOffset,SEEK_SET);
				fread(m_pCache+i*m_nCacheBandSize,m_nCacheBandSize,1,m_fpRaws[i]);
			}
		}
		else
		{
			for(int i=0;i<m_nBandNum;i++)
			{
				if(m_fpRaws[0]==NULL)
				{
					return FALSE;
				}
				unsigned long nOffset=m_nCacheRow0*m_nCols*m_nBPB+i*m_nRows*m_nCols*m_nBPB;
				fseek(m_fpRaws[0],nOffset,SEEK_SET);
				fread(m_pCache+i*m_nCacheBandSize,m_nCacheBandSize,1,m_fpRaws[0]);
			}
		}

		return TRUE;
	}
	else if(m_pGdalImage!=NULL)
	{
		//��ȡͼ�񵽻�����
		CPLErr error=CE_None;
		for(int i=0;i<m_nBandNum;i++)
		{
			int band=i+1;
			error=((GDALRasterBand*)GDALGetRasterBand(m_pGdalImage,band))->
			RasterIO(GF_Read,
					 0,m_nCacheRow0,
					 m_nCols,m_nCacheRow1-m_nCacheRow0,
					 m_pCache+i*m_nCacheBandSize,
					 m_nCols,m_nCacheRow1-m_nCacheRow0,
					 m_datatype,
					 0,0);
		}

		return error==CE_None?TRUE:FALSE;
	}

	return FALSE;
}

HRESULT CImage::GetDPI(float* pxDPI,float* pyDPI)
{
	*pxDPI=254;
	*pyDPI=254;

	return S_FALSE;
}

HRESULT CImage::SetDPI(float xDPI,float yDPI)
{

	return S_FALSE;
}

//add by  dy 20081027 for get sensortype begin
HRESULT CImage::GetSensorType(int * pSensorType)
{

	*pSensorType=m_nSensorType;
	return S_OK;
}

HRESULT CImage::SetSensorType(int nSensorType)
{
	m_nSensorType=nSensorType;
	return S_OK;
}

//Get and set satellite ID. [Wei.Zuo,4/15/2009]
HRESULT CImage::GetSatelliteID(int *pSatelliteID)
{
	*pSatelliteID = m_nSatelliteID;
	return S_OK;
}

HRESULT CImage::SetSatelliteID(int nSatelliteID)
{
	m_nSatelliteID = nSatelliteID;
	return S_OK;
}

