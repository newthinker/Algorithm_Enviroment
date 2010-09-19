// PatternMatch.cpp: implementation of the CPatternMatch class.
//
//////////////////////////////////////////////////////////////////////

#include "PatternMatch.h"
#include <math.h>

#define PYRAMIDLAYER 2
#define SUCCEEDEDCOEF 0.75		// match coff threshold(0.75)
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPatternMatch::CPatternMatch()
{

}

CPatternMatch::~CPatternMatch()
{

}

HRESULT CPatternMatch::PatternMatch(BYTE *pImage, int nImgRows, int nImgCols, double fImgScanSize,
								 BYTE *pPattern, int nPtnRows, int nPtnCols, double fPtnScanSize,
								 UINT nDataType,
								 double *px, double *py, double *pCoef)
{
	*px=nImgCols/2;
	*py=nImgRows/2;
	//�ز�����ģ��
	BYTE* pNewPattern=NULL;
	int nNewPtnRows,nNewPtnCols;
	Resample(pPattern,nPtnRows,nPtnCols,nDataType,fPtnScanSize,fImgScanSize,&pNewPattern,&nNewPtnRows,&nNewPtnCols);

	//�������
	int nX0,nY0,nHeight,nWidth;
	//ƥ����
	double X,Y,Coef;
	BOOL bMatched=FALSE;
	int nImgPydRowsPre=0,nImgPydColsPre=0;
	
	//
	int nLevel = 0;
	for(int pl=PYRAMIDLAYER;pl>=1;pl--)
	{
		nLevel = (int)pow(2,pl-1);
		BYTE* pImgPyd=NULL;
		int nImgPydRows=0,nImgPydCols=0;
		Pyramid(pImage,
				nImgRows,nImgCols,
				nDataType,
				nLevel,
				&pImgPyd,&nImgPydRows,&nImgPydCols);
		
		BYTE* pPtnPyd=NULL;
		int nPtnPydRows=0,nPtnPydCols=0;
		Pyramid(pNewPattern,
				nNewPtnRows,nNewPtnCols,
				nDataType,
				nLevel,
				&pPtnPyd,&nPtnPydRows,&nPtnPydCols);

		if(bMatched==FALSE)
		{
			nX0=nPtnPydCols/2;
			nY0=nPtnPydRows/2;
			nHeight=nImgPydRows-nPtnPydRows/2;
			nWidth=nImgPydCols-nPtnPydCols/2;
		}
		else
		{
			nX0=(int)(X*(double)nImgPydCols/(double)nImgPydColsPre+0.5)-10;
			nY0=(int)(Y*(double)nImgPydRows/(double)nImgPydRowsPre+0.5)-10;
			nHeight=21;
			nWidth=21;
		}

		bMatched=FALSE;
		if(Match(pImgPyd,nImgPydRows,nImgPydCols,
			pPtnPyd,nPtnPydRows,nPtnPydCols,
			nDataType,
			nX0,nY0,nHeight,nWidth,
			&X,&Y,&Coef,
			pl==1?TRUE:FALSE)==TRUE)
		{
			bMatched=TRUE;

			nImgPydRowsPre=nImgPydRows;
			nImgPydColsPre=nImgPydCols;
		}

		delete [] pImgPyd; pImgPyd=NULL;
		delete [] pPtnPyd; pPtnPyd=NULL;

		//��������ϲ�������ϲ���ƥ�䵽ͬ��㣬�򷵻�֮����ֹ��������²�ƥ���˷�ʱ��
		if(bMatched==FALSE)
		{
			break;
		}
	}

	delete [] pNewPattern; pNewPattern=NULL;

	*px=X;
	*py=Y;
	*pCoef=Coef;

	return bMatched==TRUE?S_OK:S_FALSE;
}

BOOL CPatternMatch::Pyramid(BYTE *pSrcImage, 
							int nSrcRows, int nSrcCols, 
							UINT nDataType,
							int nPrdLayer, 
							BYTE **ppDstImage, int *pnDstRows, int *pnDstCols)
{
	int nRows=(int)(nSrcRows/nPrdLayer);
	int nCols=(int)(nSrcCols/nPrdLayer);
	if(nRows%2==0)
	{
		nRows+=1;
	}
	if(nCols%2==0)
	{
		nCols+=1;
	}
	*pnDstRows=nRows;
	*pnDstCols=nCols;
	BYTE* pBuffer=NULL;
	switch(nDataType)
	{
	case Pixel_Byte:
		{
			pBuffer=new BYTE[nRows*nCols];
			for(int i=0;i<nRows;i++)
			{
				BYTE* pBufferIndex=pBuffer+i*nCols;
				for(int j=0;j<nCols;j++)
				{
					double gray=0.0;
					
					int nCount=0;
					for(int m=0;m<nPrdLayer;m++)
					{
						if(i*nPrdLayer+m>=nSrcRows)
						{
							continue;
						}

						BYTE* pSrcImageIndex=pSrcImage+(i*nPrdLayer+m)*nSrcCols;
						for(int n=0;n<nPrdLayer;n++)
						{
							if(j*nPrdLayer+n>=nSrcCols)
							{
								continue;
							}

							gray=gray+pSrcImageIndex[j*nPrdLayer+n];

							nCount++;
						}
					}

					pBufferIndex[j]=(BYTE)(gray/nCount);
				}
			}
		}
		break;
	case Pixel_Int16:
		{
			pBuffer=(BYTE*)new unsigned short[nRows*nCols];
			for(int i=0;i<nRows;i++)
			{
				unsigned short* pBufferIndex=((unsigned short*)pBuffer)+i*nCols;
				for(int j=0;j<nCols;j++)
				{
					double gray=0.0;
					
					int nCount=0;
					for(int m=0;m<nPrdLayer;m++)
					{
						if(i*nPrdLayer+m>=nSrcRows)
						{
							continue;
						}

						unsigned short* pSrcImageIndex=((unsigned short*)pSrcImage)+(i*nPrdLayer+m)*nSrcCols;
						for(int n=0;n<nPrdLayer;n++)
						{
							if(j*nPrdLayer+n>=nSrcCols)
							{
								continue;
							}

							gray=gray+pSrcImageIndex[j*nPrdLayer+n];

							nCount++;
						}
					}

					pBufferIndex[j]=(unsigned short)(gray/nCount);
				}
			}
		}
		break;
	case Pixel_SInt16:
		{
			pBuffer=(BYTE*)new short[nRows*nCols];
			for(int i=0;i<nRows;i++)
			{
				short* pBufferIndex=((short*)pBuffer)+i*nCols;
				for(int j=0;j<nCols;j++)
				{
					double gray=0.0;
					
					int nCount=0;
					for(int m=0;m<nPrdLayer;m++)
					{
						if(i*nPrdLayer+m>=nSrcRows)
						{
							continue;
						}

						short* pSrcImageIndex=((short*)pSrcImage)+(i*nPrdLayer+m)*nSrcCols;
						for(int n=0;n<nPrdLayer;n++)
						{
							if(j*nPrdLayer+n>=nSrcCols)
							{
								continue;
							}

							gray=gray+pSrcImageIndex[j*nPrdLayer+n];

							nCount++;
						}
					}

					pBufferIndex[j]=(short)(gray/nCount);
				}
			}
		}
		break;
	case Pixel_Int32:
		{
			pBuffer=(BYTE*)new unsigned int[nRows*nCols];
			for(int i=0;i<nRows;i++)
			{
				unsigned int* pBufferIndex=((unsigned int*)pBuffer)+i*nCols;
				for(int j=0;j<nCols;j++)
				{
					double gray=0.0;
					
					int nCount=0;
					for(int m=0;m<nPrdLayer;m++)
					{
						if(i*nPrdLayer+m>=nSrcRows)
						{
							continue;
						}

						unsigned int* pSrcImageIndex=((unsigned int*)pSrcImage)+(i*nPrdLayer+m)*nSrcCols;
						for(int n=0;n<nPrdLayer;n++)
						{
							if(j*nPrdLayer+n>=nSrcCols)
							{
								continue;
							}

							gray=gray+pSrcImageIndex[j*nPrdLayer+n];

							nCount++;
						}
					}

					pBufferIndex[j]=(unsigned int)(gray/nCount);
				}
			}
		}
		break;
	case Pixel_SInt32:
		{
			pBuffer=(BYTE*)new int[nRows*nCols];
			for(int i=0;i<nRows;i++)
			{
				int* pBufferIndex=((int*)pBuffer)+i*nCols;
				for(int j=0;j<nCols;j++)
				{
					double gray=0.0;
					
					int nCount=0;
					for(int m=0;m<nPrdLayer;m++)
					{
						if(i*nPrdLayer+m>=nSrcRows)
						{
							continue;
						}

						int* pSrcImageIndex=((int*)pSrcImage)+(i*nPrdLayer+m)*nSrcCols;
						for(int n=0;n<nPrdLayer;n++)
						{
							if(j*nPrdLayer+n>=nSrcCols)
							{
								continue;
							}

							gray=gray+pSrcImageIndex[j*nPrdLayer+n];

							nCount++;
						}
					}

					pBufferIndex[j]=(int)(gray/nCount);
				}
			}
		}
		break;
	case Pixel_Float:
		{
			pBuffer=(BYTE*)new float[nRows*nCols];
			for(int i=0;i<nRows;i++)
			{
				float* pBufferIndex=((float*)pBuffer)+i*nCols;
				for(int j=0;j<nCols;j++)
				{
					double gray=0.0;
					
					int nCount=0;
					for(int m=0;m<nPrdLayer;m++)
					{
						if(i*nPrdLayer+m>=nSrcRows)
						{
							continue;
						}

						float* pSrcImageIndex=((float*)pSrcImage)+(i*nPrdLayer+m)*nSrcCols;
						for(int n=0;n<nPrdLayer;n++)
						{
							if(j*nPrdLayer+n>=nSrcCols)
							{
								continue;
							}

							gray=gray+pSrcImageIndex[j*nPrdLayer+n];

							nCount++;
						}
					}

					pBufferIndex[j]=(float)(gray/nCount);
				}
			}
		}
		break;
	case Pixel_Double:
		{
			pBuffer=(BYTE*)new double[nRows*nCols];
			for(int i=0;i<nRows;i++)
			{
				double* pBufferIndex=((double*)pBuffer)+i*nCols;
				for(int j=0;j<nCols;j++)
				{
					double gray=0.0;
					
					int nCount=0;
					for(int m=0;m<nPrdLayer;m++)
					{
						if(i*nPrdLayer+m>=nSrcRows)
						{
							continue;
						}

						double* pSrcImageIndex=((double*)pSrcImage)+(i*nPrdLayer+m)*nSrcCols;
						for(int n=0;n<nPrdLayer;n++)
						{
							if(j*nPrdLayer+n>=nSrcCols)
							{
								continue;
							}

							gray=gray+pSrcImageIndex[j*nPrdLayer+n];

							nCount++;
						}
					}

					pBufferIndex[j]=gray/nCount;
				}
			}
		}
		break;
	default:
		return FALSE;
	}
	
	*ppDstImage=pBuffer;

	return TRUE;
}

BOOL CPatternMatch::Match(BYTE* pImage,int nImgRows,int nImgCols,
						  BYTE* pPattern,int nPtnRows,int nPtnCols,
						  UINT nDataType,
						  int nSearchX0,int nSearchY0,
						  int nSearchHeight,int nSearchWidth,
						  double* pX,double* pY,double* pCoef,
						  BOOL bInSubPixel)
{
	if(pImage==NULL||pPattern==NULL||nImgRows<nPtnRows||nImgCols<nPtnCols)
	{
		return FALSE;
	}
	//�����������,����ҿ�
	int nRow0=nSearchY0;
	nRow0=__max(nRow0,0);
	int nCol0=nSearchX0;
	nCol0=__max(nCol0,0);
	int nRow1=nRow0+nSearchHeight;
	nRow1=__min(nRow1,nImgRows-1);
	int nCol1=nCol0+nSearchWidth;
	nCol1=__min(nCol1,nImgCols-1);

	//��[nRow0,nRow1)[nCol0,nCol1)֮��Ѱ����������
	double maxCoef=-99999.9;
	double minCoef=99999.9;
	int X,Y;

	for(int i=nRow0;i<nRow1;i++)
	{
		for(int j=nCol0;j<nCol1;j++)
		{
			double si=0.0,sp=0.0,sii=0.0,spp=0.0,sip=0.0;
			int nCount=0;
			for(int m=-nPtnRows/2;m<=nPtnRows/2;m++)
			{
				if(m+i<0||m+i>=nImgRows)
				{
					continue;
				}
				switch(nDataType)
				{
				case Pixel_Byte:
					{
						BYTE* pImgIndex=pImage+(m+i)*nImgCols;
						BYTE* pPtnIndex=pPattern+(m+nPtnRows/2)*nPtnCols;
						for(int n=-nPtnCols/2;n<=nPtnCols/2;n++)
						{
							if(n+j<0||n+j>=nImgCols)
							{
								continue;
							}
							BYTE gi=pImgIndex[n+j];
							BYTE gp=pPtnIndex[n+nPtnCols/2];
							si=si+gi;
							sp=sp+gp;
							sii=sii+gi*gi;
							spp=spp+gp*gp;
							sip=sip+gi*gp;
							
							nCount++;
						}
					}
					break;
				case Pixel_Int16:
					{
						unsigned short* pImgIndex=((unsigned short*)pImage)+(m+i)*nImgCols;
						unsigned short* pPtnIndex=((unsigned short*)pPattern)+(m+nPtnRows/2)*nPtnCols;
						for(int n=-nPtnCols/2;n<=nPtnCols/2;n++)
						{
							if(n+j<0||n+j>=nImgCols)
							{
								continue;
							}
							double gi=pImgIndex[n+j];
							double gp=pPtnIndex[n+nPtnCols/2];
							si=si+gi;
							sp=sp+gp;
							sii=sii+gi*gi;
							spp=spp+gp*gp;
							sip=sip+gi*gp;
							
							nCount++;
						}
					}
					break;
				case Pixel_SInt16:
					{
						short* pImgIndex=((short*)pImage)+(m+i)*nImgCols;
						short* pPtnIndex=((short*)pPattern)+(m+nPtnRows/2)*nPtnCols;
						for(int n=-nPtnCols/2;n<=nPtnCols/2;n++)
						{
							if(n+j<0||n+j>=nImgCols)
							{
								continue;
							}
							double gi=pImgIndex[n+j];
							double gp=pPtnIndex[n+nPtnCols/2];
							si=si+gi;
							sp=sp+gp;
							sii=sii+gi*gi;
							spp=spp+gp*gp;
							sip=sip+gi*gp;
							
							nCount++;
						}
					}
					break;
				case Pixel_Int32:
					{
						unsigned int* pImgIndex=((unsigned int*)pImage)+(m+i)*nImgCols;
						unsigned int* pPtnIndex=((unsigned int*)pPattern)+(m+nPtnRows/2)*nPtnCols;
						for(int n=-nPtnCols/2;n<=nPtnCols/2;n++)
						{
							if(n+j<0||n+j>=nImgCols)
							{
								continue;
							}
							double gi=pImgIndex[n+j];
							double gp=pPtnIndex[n+nPtnCols/2];
							si=si+gi;
							sp=sp+gp;
							sii=sii+gi*gi;
							spp=spp+gp*gp;
							sip=sip+gi*gp;
							
							nCount++;
						}
					}
					break;
				case Pixel_SInt32:
					{
						int* pImgIndex=((int*)pImage)+(m+i)*nImgCols;
						int* pPtnIndex=((int*)pPattern)+(m+nPtnRows/2)*nPtnCols;
						for(int n=-nPtnCols/2;n<=nPtnCols/2;n++)
						{
							if(n+j<0||n+j>=nImgCols)
							{
								continue;
							}
							double gi=pImgIndex[n+j];
							double gp=pPtnIndex[n+nPtnCols/2];
							si=si+gi;
							sp=sp+gp;
							sii=sii+gi*gi;
							spp=spp+gp*gp;
							sip=sip+gi*gp;
							
							nCount++;
						}
					}
					break;
				case Pixel_Float:
					{
						float* pImgIndex=((float*)pImage)+(m+i)*nImgCols;
						float* pPtnIndex=((float*)pPattern)+(m+nPtnRows/2)*nPtnCols;
						for(int n=-nPtnCols/2;n<=nPtnCols/2;n++)
						{
							if(n+j<0||n+j>=nImgCols)
							{
								continue;
							}
							double gi=pImgIndex[n+j];
							double gp=pPtnIndex[n+nPtnCols/2];
							si=si+gi;
							sp=sp+gp;
							sii=sii+gi*gi;
							spp=spp+gp*gp;
							sip=sip+gi*gp;
							
							nCount++;
						}
					}
					break;
				case Pixel_Double:
					{
						double* pImgIndex=((double*)pImage)+(m+i)*nImgCols;
						double* pPtnIndex=((double*)pPattern)+(m+nPtnRows/2)*nPtnCols;
						for(int n=-nPtnCols/2;n<=nPtnCols/2;n++)
						{
							if(n+j<0||n+j>=nImgCols)
							{
								continue;
							}
							double gi=pImgIndex[n+j];
							double gp=pPtnIndex[n+nPtnCols/2];
							si=si+gi;
							sp=sp+gp;
							sii=sii+gi*gi;
							spp=spp+gp*gp;
							sip=sip+gi*gp;
							
							nCount++;
						}
					}
					break;
				}
			}

			double N=(double)nCount;
			double a=sip-si*sp/N;
			double b=(sii-si*si/N)*(spp-sp*sp/N);
			if(b>1e-5)
			{
				double Coef=a/(sqrt(b)+1e-6);
				if(Coef>maxCoef)
				{
					maxCoef=Coef;
					X=j;
					Y=i;
				}
			}
		}
	}

	*pX=X;
	*pY=Y;
	*pCoef=maxCoef;

	if(maxCoef<SUCCEEDEDCOEF)
	{
		return FALSE;
	}

	//���������
	if(bInSubPixel)
	{
		double CoefMatrix[3][3];
		memset(CoefMatrix,0,sizeof(double)*9);
		
		nSearchX0=X-1;
		nSearchY0=Y-1;
		nSearchHeight=3;
		nSearchWidth=3;

		int nRow0=nSearchY0;
		nRow0=__max(nRow0,0);
		int nCol0=nSearchX0;
		nCol0=__max(nCol0,0);
		int nRow1=nRow0+nSearchHeight;
		int nCol1=nCol0+nSearchWidth;

		for(int i=nRow0;i<nRow1;i++)
		{
			for(int j=nCol0;j<nCol1;j++)
			{
				double si=0.0,sp=0.0,sii=0.0,spp=0.0,sip=0.0;
				int nCount=0;
				for(int m=-nPtnRows/2;m<=nPtnRows/2;m++)
				{
					if(m+i<0||m+i>=nImgRows)
					{
						continue;
					}
					switch(nDataType)
					{
					case Pixel_Byte:
						{
							BYTE* pImgIndex=pImage+(m+i)*nImgCols;
							BYTE* pPtnIndex=pPattern+(m+nPtnRows/2)*nPtnCols;
							for(int n=-nPtnCols/2;n<=nPtnCols/2;n++)
							{
								if(n+j<0||n+j>=nImgCols)
								{
									continue;
								}
								BYTE gi=pImgIndex[n+j];
								BYTE gp=pPtnIndex[n+nPtnCols/2];
								si=si+gi;
								sp=sp+gp;
								sii=sii+gi*gi;
								spp=spp+gp*gp;
								sip=sip+gi*gp;
								
								nCount++;
							}
						}
						break;
					case Pixel_Int16:
						{
							unsigned short* pImgIndex=((unsigned short*)pImage)+(m+i)*nImgCols;
							unsigned short* pPtnIndex=((unsigned short*)pPattern)+(m+nPtnRows/2)*nPtnCols;
							for(int n=-nPtnCols/2;n<=nPtnCols/2;n++)
							{
								if(n+j<0||n+j>=nImgCols)
								{
									continue;
								}
								double gi=pImgIndex[n+j];
								double gp=pPtnIndex[n+nPtnCols/2];
								si=si+gi;
								sp=sp+gp;
								sii=sii+gi*gi;
								spp=spp+gp*gp;
								sip=sip+gi*gp;
								
								nCount++;
							}
						}
						break;
					case Pixel_SInt16:
						{
							short* pImgIndex=((short*)pImage)+(m+i)*nImgCols;
							short* pPtnIndex=((short*)pPattern)+(m+nPtnRows/2)*nPtnCols;
							for(int n=-nPtnCols/2;n<=nPtnCols/2;n++)
							{
								if(n+j<0||n+j>=nImgCols)
								{
									continue;
								}
								double gi=pImgIndex[n+j];
								double gp=pPtnIndex[n+nPtnCols/2];
								si=si+gi;
								sp=sp+gp;
								sii=sii+gi*gi;
								spp=spp+gp*gp;
								sip=sip+gi*gp;
								
								nCount++;
							}
						}
						break;
					case Pixel_Int32:
						{
							unsigned int* pImgIndex=((unsigned int*)pImage)+(m+i)*nImgCols;
							unsigned int* pPtnIndex=((unsigned int*)pPattern)+(m+nPtnRows/2)*nPtnCols;
							for(int n=-nPtnCols/2;n<=nPtnCols/2;n++)
							{
								if(n+j<0||n+j>=nImgCols)
								{
									continue;
								}
								double gi=pImgIndex[n+j];
								double gp=pPtnIndex[n+nPtnCols/2];
								si=si+gi;
								sp=sp+gp;
								sii=sii+gi*gi;
								spp=spp+gp*gp;
								sip=sip+gi*gp;
								
								nCount++;
							}
						}
						break;
					case Pixel_SInt32:
						{
							int* pImgIndex=((int*)pImage)+(m+i)*nImgCols;
							int* pPtnIndex=((int*)pPattern)+(m+nPtnRows/2)*nPtnCols;
							for(int n=-nPtnCols/2;n<=nPtnCols/2;n++)
							{
								if(n+j<0||n+j>=nImgCols)
								{
									continue;
								}
								double gi=pImgIndex[n+j];
								double gp=pPtnIndex[n+nPtnCols/2];
								si=si+gi;
								sp=sp+gp;
								sii=sii+gi*gi;
								spp=spp+gp*gp;
								sip=sip+gi*gp;
								
								nCount++;
							}
						}
						break;
					case Pixel_Float:
						{
							float* pImgIndex=((float*)pImage)+(m+i)*nImgCols;
							float* pPtnIndex=((float*)pPattern)+(m+nPtnRows/2)*nPtnCols;
							for(int n=-nPtnCols/2;n<=nPtnCols/2;n++)
							{
								if(n+j<0||n+j>=nImgCols)
								{
									continue;
								}
								double gi=pImgIndex[n+j];
								double gp=pPtnIndex[n+nPtnCols/2];
								si=si+gi;
								sp=sp+gp;
								sii=sii+gi*gi;
								spp=spp+gp*gp;
								sip=sip+gi*gp;
								
								nCount++;
							}
						}
						break;
					case Pixel_Double:
						{
							double* pImgIndex=((double*)pImage)+(m+i)*nImgCols;
							double* pPtnIndex=((double*)pPattern)+(m+nPtnRows/2)*nPtnCols;
							for(int n=-nPtnCols/2;n<=nPtnCols/2;n++)
							{
								if(n+j<0||n+j>=nImgCols)
								{
									continue;
								}
								double gi=pImgIndex[n+j];
								double gp=pPtnIndex[n+nPtnCols/2];
								si=si+gi;
								sp=sp+gp;
								sii=sii+gi*gi;
								spp=spp+gp*gp;
								sip=sip+gi*gp;
								
								nCount++;
							}
						}
						break;
					}
				}
				double N=(double)nCount;
				double a=sip-si*sp/N;
				double b=(sii-si*si/N)*(spp-sp*sp/N);
				if(b>1e-5)
				{
					double Coef=a/sqrt(b);
					CoefMatrix[i-nRow0][j-nCol0]=Coef;
				}
				else
				{
					CoefMatrix[i-nRow0][j-nCol0]=0.0;
				}
			}
		}

		double xcoef0=/*CoefMatrix[0][0]+*/CoefMatrix[1][0]/*+CoefMatrix[2][0]*/;
		double xcoef1=/*CoefMatrix[0][1]+*/CoefMatrix[1][1]/*+CoefMatrix[2][1]*/;
		double xcoef2=/*CoefMatrix[0][2]+*/CoefMatrix[1][2]/*+CoefMatrix[2][2]*/;

		double ycoef0=/*CoefMatrix[0][0]+*/CoefMatrix[0][1]/*+CoefMatrix[0][2]*/;
		double ycoef1=/*CoefMatrix[1][0]+*/CoefMatrix[1][1]/*+CoefMatrix[1][2]*/;
		double ycoef2=/*CoefMatrix[2][0]+*/CoefMatrix[2][1]/*+CoefMatrix[2][2]*/;

		double xoffset=(xcoef0-xcoef2)/(xcoef0+xcoef2-2.0*xcoef1)/2.0;
		double yoffset=(ycoef0-ycoef2)/(ycoef0+ycoef2-2.0*ycoef1)/2.0;
		if(1)
		{
			*pX += xoffset;
			*pY += yoffset;
			double CoefX = - (xcoef0-xcoef2)*(xcoef0-xcoef2)
					/(8*(xcoef0+xcoef2-2*xcoef1));
			double CoefY = - (ycoef0-ycoef2)*(ycoef0-ycoef2)
					/(8*(ycoef0+ycoef2-2*ycoef1));
			*pCoef += CoefX+CoefY;
			if(*pCoef>1.00)
			{
				*pCoef=1.00;
			}
		}
	}

	return TRUE;
}

BOOL CPatternMatch::Resample(BYTE* pSrcImage,
							 int nSrcRows,int nSrcCols,
							 UINT nDataType,
							 double lfSrcScanSize,double lfDstScanSize,
							 BYTE** ppDstImage,
							 int* pnDstRows,int* pnDstCols)
{
	int nRows=(int)(nSrcRows/(lfDstScanSize/lfSrcScanSize));//�˴�����0.5�������
	int nCols=(int)(nSrcCols/(lfDstScanSize/lfSrcScanSize));
	if(nRows%2==0)
	{
		nRows+=1;
	}
	if(nCols%2==0)
	{
		nCols+=1;
	}
	*pnDstRows=nRows;
	*pnDstCols=nCols;

	double fXScale=(double)nSrcCols/(double)nCols;
	double fYScale=(double)nSrcRows/(double)nRows;

	BYTE* pBuffer=NULL;
	switch(nDataType)
	{
	case Pixel_Byte:
		{
			pBuffer=new BYTE[nRows*nCols];
			for(int i=0;i<nRows;i++)
			{
				int nRow0=(int)(i*fYScale);
				int nRow2=__min(nRow0+1,nSrcRows-1);
				double dy=i*fYScale-nRow0;

				for(int j=0;j<nCols;j++)
				{
					int nCol0=(int)(j*fXScale);
					int nCol2=__min(nCol0+1,nSrcCols-1);

					double dx=j*fXScale-nCol0;
					BYTE G0,G1,G2,G3;
					G0=pSrcImage[nRow0*nSrcCols+nCol0];
					G1=pSrcImage[nRow0*nSrcCols+nCol2];
					G2=pSrcImage[nRow2*nSrcCols+nCol2];
					G3=pSrcImage[nRow2*nSrcCols+nCol0];

					double G=(1.0-dx) *  (1.0-dy)  *  G0+
								dx    *  (1.0-dy)  *  G1+
		   						dx    *  dy        *  G2+
							 (1.0-dx) *  dy        *  G3;
				
					pBuffer[i*nCols+j]=(BYTE)G;
				}
			}
		}
		break;
	case Pixel_Int16:
		{
			pBuffer=(BYTE*)new unsigned short[nRows*nCols];
			for(int i=0;i<nRows;i++)
			{
				int nRow0=(int)(i*fYScale);
				int nRow2=__min(nRow0+1,nSrcRows-1);
				double dy=i*fYScale-nRow0;

				for(int j=0;j<nCols;j++)
				{
					int nCol0=(int)(j*fXScale);
					int nCol2=__min(nCol0+1,nSrcCols-1);

					double dx=j*fXScale-nCol0;
					double G0,G1,G2,G3;
					G0=((unsigned short*)pSrcImage)[nRow0*nSrcCols+nCol0];
					G1=((unsigned short*)pSrcImage)[nRow0*nSrcCols+nCol2];
					G2=((unsigned short*)pSrcImage)[nRow2*nSrcCols+nCol2];
					G3=((unsigned short*)pSrcImage)[nRow2*nSrcCols+nCol0];

					double G=(1.0-dx) *  (1.0-dy)  *  G0+
								dx    *  (1.0-dy)  *  G1+
		   						dx    *  dy        *  G2+
							 (1.0-dx) *  dy        *  G3;
				
					((unsigned short*)pBuffer)[i*nCols+j]=(BYTE)G;
				}
			}
		}
		break;
	case Pixel_SInt16:
		{
			pBuffer=(BYTE*)new short[nRows*nCols];
			for(int i=0;i<nRows;i++)
			{
				int nRow0=(int)(i*fYScale);
				int nRow2=__min(nRow0+1,nSrcRows-1);
				double dy=i*fYScale-nRow0;

				for(int j=0;j<nCols;j++)
				{
					int nCol0=(int)(j*fXScale);
					int nCol2=__min(nCol0+1,nSrcCols-1);

					double dx=j*fXScale-nCol0;
					double G0,G1,G2,G3;
					G0=((short*)pSrcImage)[nRow0*nSrcCols+nCol0];
					G1=((short*)pSrcImage)[nRow0*nSrcCols+nCol2];
					G2=((short*)pSrcImage)[nRow2*nSrcCols+nCol2];
					G3=((short*)pSrcImage)[nRow2*nSrcCols+nCol0];

					double G=(1.0-dx) *  (1.0-dy)  *  G0+
								dx    *  (1.0-dy)  *  G1+
		   						dx    *  dy        *  G2+
							 (1.0-dx) *  dy        *  G3;
				
					((short*)pBuffer)[i*nCols+j]=(BYTE)G;
				}
			}
		}
		break;
	case Pixel_Int32:
		{
			pBuffer=(BYTE*)new unsigned int[nRows*nCols];
			for(int i=0;i<nRows;i++)
			{
				int nRow0=(int)(i*fYScale);
				int nRow2=__min(nRow0+1,nSrcRows-1);
				double dy=i*fYScale-nRow0;

				for(int j=0;j<nCols;j++)
				{
					int nCol0=(int)(j*fXScale);
					int nCol2=__min(nCol0+1,nSrcCols-1);

					double dx=j*fXScale-nCol0;
					double G0,G1,G2,G3;
					G0=((unsigned int*)pSrcImage)[nRow0*nSrcCols+nCol0];
					G1=((unsigned int*)pSrcImage)[nRow0*nSrcCols+nCol2];
					G2=((unsigned int*)pSrcImage)[nRow2*nSrcCols+nCol2];
					G3=((unsigned int*)pSrcImage)[nRow2*nSrcCols+nCol0];

					double G=(1.0-dx) *  (1.0-dy)  *  G0+
								dx    *  (1.0-dy)  *  G1+
		   						dx    *  dy        *  G2+
							 (1.0-dx) *  dy        *  G3;
				
					((unsigned int*)pBuffer)[i*nCols+j]=(BYTE)G;
				}
			}
		}
		break;
	case Pixel_SInt32:
		{
			pBuffer=(BYTE*)new int[nRows*nCols];
			for(int i=0;i<nRows;i++)
			{
				int nRow0=(int)(i*fYScale);
				int nRow2=__min(nRow0+1,nSrcRows-1);
				double dy=i*fYScale-nRow0;

				for(int j=0;j<nCols;j++)
				{
					int nCol0=(int)(j*fXScale);
					int nCol2=__min(nCol0+1,nSrcCols-1);

					double dx=j*fXScale-nCol0;
					double G0,G1,G2,G3;
					G0=((int*)pSrcImage)[nRow0*nSrcCols+nCol0];
					G1=((int*)pSrcImage)[nRow0*nSrcCols+nCol2];
					G2=((int*)pSrcImage)[nRow2*nSrcCols+nCol2];
					G3=((int*)pSrcImage)[nRow2*nSrcCols+nCol0];

					double G=(1.0-dx) *  (1.0-dy)  *  G0+
								dx    *  (1.0-dy)  *  G1+
		   						dx    *  dy        *  G2+
							 (1.0-dx) *  dy        *  G3;
				
					((int*)pBuffer)[i*nCols+j]=(BYTE)G;
				}
			}
		}
		break;
	case Pixel_Float:
		{
			pBuffer=(BYTE*)new float[nRows*nCols];
			for(int i=0;i<nRows;i++)
			{
				int nRow0=(int)(i*fYScale);
				int nRow2=__min(nRow0+1,nSrcRows-1);
				double dy=i*fYScale-nRow0;

				for(int j=0;j<nCols;j++)
				{
					int nCol0=(int)(j*fXScale);
					int nCol2=__min(nCol0+1,nSrcCols-1);

					double dx=j*fXScale-nCol0;
					double G0,G1,G2,G3;
					G0=((float*)pSrcImage)[nRow0*nSrcCols+nCol0];
					G1=((float*)pSrcImage)[nRow0*nSrcCols+nCol2];
					G2=((float*)pSrcImage)[nRow2*nSrcCols+nCol2];
					G3=((float*)pSrcImage)[nRow2*nSrcCols+nCol0];

					double G=(1.0-dx) *  (1.0-dy)  *  G0+
								dx    *  (1.0-dy)  *  G1+
		   						dx    *  dy        *  G2+
							 (1.0-dx) *  dy        *  G3;
				
					((float*)pBuffer)[i*nCols+j]=(BYTE)G;
				}
			}
		}
		break;
	case Pixel_Double:
		{
			pBuffer=(BYTE*)new double[nRows*nCols];
			for(int i=0;i<nRows;i++)
			{
				int nRow0=(int)(i*fYScale);
				int nRow2=__min(nRow0+1,nSrcRows-1);
				double dy=i*fYScale-nRow0;

				for(int j=0;j<nCols;j++)
				{
					int nCol0=(int)(j*fXScale);
					int nCol2=__min(nCol0+1,nSrcCols-1);

					double dx=j*fXScale-nCol0;
					double G0,G1,G2,G3;
					G0=((double*)pSrcImage)[nRow0*nSrcCols+nCol0];
					G1=((double*)pSrcImage)[nRow0*nSrcCols+nCol2];
					G2=((double*)pSrcImage)[nRow2*nSrcCols+nCol2];
					G3=((double*)pSrcImage)[nRow2*nSrcCols+nCol0];

					double G=(1.0-dx) *  (1.0-dy)  *  G0+
								dx    *  (1.0-dy)  *  G1+
		   						dx    *  dy        *  G2+
							 (1.0-dx) *  dy        *  G3;
				
					((double*)pBuffer)[i*nCols+j]=(BYTE)G;
				}
			}
		}
		break;
	default:
		return FALSE;
	}
	
	*ppDstImage=pBuffer;

	return TRUE;
}
