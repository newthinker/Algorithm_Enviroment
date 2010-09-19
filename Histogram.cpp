// Histogram.cpp: implementation of the CHistogram class.
//
//////////////////////////////////////////////////////////////////////

#include "Histogram.h"
#include "math.h"
#include "float.h"
#include "stdlib.h"
#include "stdio.h"
#include "Image.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CHistogram::CHistogram()
{
	memset(m_Histogram,0,sizeof(unsigned int)*256);
	memset(m_Histogram16,0,sizeof(unsigned int)*65536);
	memset(m_LUT,0,sizeof(BYTE)*65536);
	m_EnhanceType=ET_DEFAULT;
}

CHistogram::~CHistogram()
{

}

void CHistogram::StatHistogram(CImage *pImage, int nBandIndex)
{
	int i=0;
	for(i=0;i<65536;i++)
	{
		m_Histogram16[i]=0;
	}
	
	m_BandMax=-100000;
	m_BandMin=100000;
	//ͳ�Ƶ�ǰ���εĻҶ�ֱ��ͼ�ͻҶ���ֵ
	int nRow,nCol;
	pImage->GetRows(&nRow);
	pImage->GetCols(&nCol);
	int nBPB;
	pImage->GetBPB(&nBPB);
	UINT datatype;
	pImage->GetDataType(&datatype);

	int nBufferRow=nRow;
	int nBufferCol=nCol;
	while(nBufferRow>1024)
	{
		nBufferRow/=2;
	}
	while(nBufferCol>1024)
	{
		nBufferCol/=2;
	}

	BYTE* pBuffer=new BYTE[nBufferRow*nBufferCol*nBPB];

	memset(pBuffer,0,nBufferRow*nBufferCol*nBPB);
	pImage->ReadImg(0,0,(float)nCol,(float)nRow,
					  pBuffer,nBufferCol,nBufferRow,
					  1,
					  0,0,nBufferCol,nBufferRow,
					  nBandIndex,0);

	m_BandMax=-100000;
	m_BandMin=100000;
	switch(datatype)
	{
	case Pixel_Byte:
		{
			for(int i=0;i<nBufferRow;i++)
			{
				BYTE* pBufferIndex=pBuffer+i*nBufferCol*nBPB;
				for(int j=0;j<nBufferCol;j++)
				{
					if(pBufferIndex[j]>m_BandMax)
					{
						m_BandMax=pBufferIndex[j];
					}
					else if(pBufferIndex[j]<m_BandMin)
					{
						m_BandMin=pBufferIndex[j];
					}
					m_Histogram16[pBufferIndex[j]]++;
				}
			}
			if(fabs(m_BandMax-m_BandMin)<1e-6)
			{
				m_BandMax=255;
				m_BandMin=0;
			}
		}
		break;
	case Pixel_Int16:
	case Pixel_Int32:
		{
			for(int i=0;i<nBufferRow;i++)
			{
				unsigned short* pBufferIndex=(unsigned short*)(pBuffer+i*nBufferCol*nBPB);
				for(int j=0;j<nBufferCol;j++)
				{
					if(pBufferIndex[j]>m_BandMax)
					{
						m_BandMax=pBufferIndex[j];
					}
					else if(pBufferIndex[j]<m_BandMin)
					{
						m_BandMin=pBufferIndex[j];
					}
					m_Histogram16[pBufferIndex[j]]++;
				}
			}
			if(fabs(m_BandMax-m_BandMin)<1e-6)
			{
				m_BandMax=65535;
				m_BandMin=0;
			}
		}
		break;
	case Pixel_SInt16:
	case Pixel_SInt32:
		{
			for(int i=0;i<nBufferRow;i++)
			{
				short* pBufferIndex=(short*)(pBuffer+i*nBufferCol*nBPB);
				for(int j=0;j<nBufferCol;j++)
				{
					if(pBufferIndex[j]>m_BandMax)
					{
						m_BandMax=pBufferIndex[j];
					}
					else if(pBufferIndex[j]<m_BandMin)
					{
						m_BandMin=pBufferIndex[j];
					}
					m_Histogram16[pBufferIndex[j]+32768]++;
				}
			}
			if(fabs(m_BandMax-m_BandMin)<1e-6)
			{
				m_BandMax=32767;
				m_BandMin=-32768;
			}
		}
		break;
	case Pixel_Float:
		{
			for(int i=0;i<nBufferRow;i++)
			{
				float* pBufferIndex=(float*)(pBuffer+i*nBufferCol*nBPB);
				for(int j=0;j<nBufferCol;j++)
				{
					if(pBufferIndex[j]!=FLT_MAX)
					{
						if(pBufferIndex[j]>m_BandMax)
						{
							m_BandMax=pBufferIndex[j];
						}
						else if(pBufferIndex[j]<m_BandMin)
						{
							m_BandMin=pBufferIndex[j];
						}
					}
				}
			}
			if(fabs(m_BandMax-m_BandMin)<1e-6)
			{
				//m_BandMax=65535;
				m_BandMin=0;
			}
		}
		break;
	case Pixel_Double:
		{
			for(int i=0;i<nBufferRow;i++)
			{
				double* pBufferIndex=(double*)(pBuffer+i*nBufferCol*nBPB);
				for(int j=0;j<nBufferCol;j++)
				{
					if(pBufferIndex[j]!=DBL_MAX)
					{
						if(pBufferIndex[j]>m_BandMax)
						{
							m_BandMax=pBufferIndex[j];
						}
						else if(pBufferIndex[j]<m_BandMin)
						{
							m_BandMin=pBufferIndex[j];
						}
					}
				}
			}
			if(fabs(m_BandMax-m_BandMin)<1e-6)
			{
				//m_BandMax=65535;
				m_BandMin=0;
			}
		}
		break;
	}

	//����FLOAT��DOUBLEͼ���16λ�Ҷ�ֱ��ͼ
	if(datatype==Pixel_Float)
	{
		for(i=0;i<nBufferRow;i++)
		{
			float* pBufferIndex=(float*)(pBuffer+i*nBufferCol*nBPB);
			for(int j=0;j<nBufferCol;j++)
			{
				double temp=(pBufferIndex[j]-m_BandMin)/(m_BandMax-m_BandMin)*65535;
				if(temp>65535)
				{
					temp=65535;
				}
				else if(temp<0)
				{
					temp=0;
				}
				m_Histogram16[(int)temp]++;
			}
		}
	}
	else if(datatype==Pixel_Double)
	{
		for(i=0;i<nBufferRow;i++)
		{
			double* pBufferIndex=(double*)(pBuffer+i*nBufferCol*nBPB);
			for(int j=0;j<nBufferCol;j++)
			{
				double temp=(pBufferIndex[j]-m_BandMin)/(m_BandMax-m_BandMin)*65535;
				if(temp>65535)
				{
					temp=65535;
				}
				else if(temp<0)
				{
					temp=0;
				}
				m_Histogram16[(int)temp]++;
			}
		}
	}

	delete [] pBuffer; pBuffer=NULL;

	//���16λ�Ҷ�ֱ��ͼ����ε������Сֵ����ֹ���ڸ����������Ӱ��������ʾ
	double SamplesOfPixel=0;
	for(i=0;i<65536;i++)
	{
		SamplesOfPixel+=m_Histogram16[i];
	}

	double noiseLimit=0.002;
	switch(datatype)
	{
	case Pixel_Int16:
		{
			double noise=0;
			for(i=10;i<65536;i++)
			{
				noise+=m_Histogram16[i];
				if(noise/SamplesOfPixel>noiseLimit)
				{
					m_BandMin=i;
					break;
				}
			}
			noise=0;
			for(i=65525;i>=0;i--)
			{
				noise+=m_Histogram16[i];
				if(noise/SamplesOfPixel>noiseLimit)
				{
					m_BandMax=i;
					break;
				}
			}
			if(m_BandMax<=m_BandMin)
			{
				m_BandMax=65535;
				m_BandMin=0;
			}
		}
		break;
	case Pixel_SInt16:
		{
			double noise=0;
			for(i=10;i<65536;i++)
			{
				noise+=m_Histogram16[i];
				if(noise/SamplesOfPixel>noiseLimit)
				{
					m_BandMin=i-32768;
					break;
				}
			}
			noise=0;
			for(i=65525;i>=0;i--)
			{
				noise+=m_Histogram16[i];
				if(noise/SamplesOfPixel>noiseLimit)
				{
					m_BandMax=i-32768;
					break;
				}
			}
			if(m_BandMax<=m_BandMin)
			{
				m_BandMax=32767;
				m_BandMin=-32768;
			}
		}
		break;
	case Pixel_Float:
		{
			int min=0,max=65525;
			double noise=0;
			for(i=10;i<65536;i++)
			{
				noise+=m_Histogram16[i];
				if(noise/SamplesOfPixel>noiseLimit)
				{
					min=i;
					break;
				}
			}
			noise=0;
			for(i=65525;i>=0;i--)
			{
				noise+=m_Histogram16[i];
				if(noise/SamplesOfPixel>noiseLimit)
				{
					max=i;
					break;
				}
			}
			if(m_BandMax<=m_BandMin)
			{
				max=65535;
				min=0;
			}
			double tempMin=m_BandMin+(m_BandMax-m_BandMin)/65535*min;
			double tempMax=m_BandMax-(m_BandMax-m_BandMin)/65535*(65535-max);
			m_BandMin=tempMin;
			m_BandMax=tempMax;
		}
		break;
	case Pixel_Double:
		{
			int min=0,max=65525;
			double noise=0;
			for(i=10;i<65536;i++)
			{
				noise+=m_Histogram16[i];
				if(noise/SamplesOfPixel>noiseLimit)
				{
					min=i;
					break;
				}
			}
			noise=0;
			for(i=65525;i>=0;i--)
			{
				noise+=m_Histogram16[i];
				if(noise/SamplesOfPixel>noiseLimit)
				{
					max=i;
					break;
				}
			}
			if(m_BandMax<=m_BandMin)
			{
				max=65535;
				min=0;
			}
			double tempMin=m_BandMin+(m_BandMax-m_BandMin)/65535*min;
			double tempMax=m_BandMax-(m_BandMax-m_BandMin)/65535*(65535-max);
			m_BandMin=tempMin;
			m_BandMax=tempMax;
		}
		break;
	}

	CalculateLUT(datatype);

	return ;
}

double CHistogram::GetMaxGray()
{
	return m_BandMax;
}

double CHistogram::GetMinGray()
{
	return m_BandMin;
}

BYTE* CHistogram::GetImageLUT()
{
	return m_LUT;
}

BOOL CHistogram::CalculateLUT(UINT datatype)
{
	BOOL bRet=FALSE;

	//Ĭ����ǿ��ʽ
	if(m_EnhanceType==ET_DEFAULT)
	{
		if(datatype==Pixel_Byte)
		{
			m_EnhanceType=ET_NONE;
		}
		else
		{
			m_EnhanceType=ET_LINER;
		}
	}
	
	int i=0;
	for(i=0;i<65536;i++)
	{
		m_LUT[i]=0;
	}
	switch(datatype)
	{
	case Pixel_Byte:
		{
			switch(m_EnhanceType)
			{
			case ET_NONE:
				{
					for(int i=0;i<256;i++)
					{
						m_LUT[i]=i;
					}
				}
				break;
			case ET_LINER:
				{
					for(int i=0;i<256;i++)
					{
						double temp=(i-m_BandMin)/(m_BandMax-m_BandMin)*255+0.5;
						if(temp>255)
						{
							m_LUT[i]=255;
						}
						else if(temp<0)
						{
							m_LUT[i]=0;
						}
						else
						{
							m_LUT[i]=(BYTE)temp;
						}
					}
				}
				break;
			}
			bRet=TRUE;
		}
		break;
	case Pixel_Int16:
	case Pixel_Int32:
		{
			switch(m_EnhanceType)
			{
			case ET_NONE:
				bRet=FALSE;
				break;
			case ET_LINER:
				{
					for(int i=0;i<65536;i++)
					{
						double temp=(i-m_BandMin)/(m_BandMax-m_BandMin)*255+0.5;
						if(temp>255)
						{
							m_LUT[i]=255;
						}
						else if(temp<0)
						{
							m_LUT[i]=0;
						}
						else
						{
							m_LUT[i]=(BYTE)temp;
						}
					}
				}
				bRet=TRUE;
				break;
			}
		}
		break;
	case Pixel_SInt16:
	case Pixel_SInt32:
		{
			switch(m_EnhanceType)
			{
			case ET_NONE:
				bRet=FALSE;
				break;
			case ET_LINER:
				{
					for(int i=-32768;i<=32767;i++)
					{
						double temp=(i-m_BandMin)/(m_BandMax-m_BandMin)*255+0.5;
						if(temp>255)
						{
							m_LUT[i+32768]=255;
						}
						else if(temp<0)
						{
							m_LUT[i+32768]=0;
						}
						else
						{
							m_LUT[i+32768]=(BYTE)temp;
						}
					}
				}
				bRet=TRUE;
				break;
			}	
		}
		break;
	case Pixel_Float:
		{
			switch(m_EnhanceType)
			{
			case ET_NONE:
				bRet=FALSE;
				break;
			case ET_LINER:
				{
					for(int i=0;i<65536;i++)
					{
						double temp=((double)(i-0))/(double)65535*255+0.5;
						if(temp>255)
						{
							m_LUT[i]=255;
						}
						else if(temp<0)
						{
							m_LUT[i]=0;
						}
						else
						{
							m_LUT[i]=(BYTE)temp;
						}
					}
				}
				bRet=TRUE;
				break;
			}
		}
		break;
	case Pixel_Double:
		{
			switch(m_EnhanceType)
			{
			case ET_NONE:
				bRet=FALSE;
				break;
			case ET_LINER:
				{
					for(int i=0;i<65536;i++)
					{
						double temp=((double)(i-0))/(double)65535*255+0.5;
						if(temp>255)
						{
							m_LUT[i]=255;
						}
						else if(temp<0)
						{
							m_LUT[i]=0;
						}
						else
						{
							m_LUT[i]=(BYTE)temp;
						}
					}
				}
				bRet=TRUE;
				break;
			}
		}
		break;
	default:
		{
			bRet=FALSE;
		}
	}

	for(i=0;i<65536;i++)
	{
		m_Histogram[m_LUT[i]]+=m_Histogram16[i];
	}

	return bRet;
}

/*switch(nBaseDataType)
		{
		case Pixel_Byte:
			{
				for(int j=0;j<nDstCol;j++)
				{
					pDstRowBuffer[j]=BaseLUT[pSrcRowBuffer[j]];
				}
			}
			break;
		case Pixel_Int16:
			{
				for(int j=0;j<nDstCol;j++)
				{
					pDstRowBuffer[j]=BaseLUT[((unsigned short*)pSrcRowBuffer)[j]];
				}
			}
			break;
		case Pixel_SInt16:
			{
				for(int j=0;j<nDstCol;j++)
				{
					pDstRowBuffer[j]=BaseLUT[((short*)pSrcRowBuffer)[j]+32768];
				}
			}
			break;
		case Pixel_Int32:
			{
				for(int j=0;j<nDstCol;j++)
				{
					pDstRowBuffer[j]=BaseLUT[((unsigned int*)pSrcRowBuffer)[j]];
				}
			}
			break;
		case Pixel_SInt32:
			{
				for(int j=0;j<nDstCol;j++)
				{
					pDstRowBuffer[j]=BaseLUT[((int*)pSrcRowBuffer)[j]+32768];
				}
			}
			break;
		case Pixel_Float:
			{
				for(int j=0;j<nDstCol;j++)
				{
					
				}
			}
			break;
		default:
			AfxMessageBox(IDS_PIXCEL_TYPE_ERROR);
			break;
		}

	}*/

BYTE CHistogram::LUT(BYTE value)
{
	return m_LUT[value];
}

BYTE CHistogram::LUT(unsigned short value)
{
	return m_LUT[value];
}

BYTE CHistogram::LUT(short value)
{
	return m_LUT[value+32768];
}

BYTE CHistogram::LUT(unsigned int value)
{
	return m_LUT[value];
}

BYTE CHistogram::LUT(int value)
{
	return m_LUT[value+32768];
}

BYTE CHistogram::LUT(float value)
{
	double temp=(value-m_BandMin)/(m_BandMax-m_BandMin)*65535;
	int nIndex=(int)temp;
	if(nIndex>65535)
	{
		nIndex=65535;
	}
	else if(nIndex<0)
	{
		nIndex=0;
	}

	return m_LUT[nIndex];
}

BYTE CHistogram::LUT(double value)
{
	double temp=(value-m_BandMin)/(m_BandMax-m_BandMin)*65535;
	int nIndex=(int)temp;
	if(nIndex>65535)
	{
		nIndex=65535;
	}
	else if(nIndex<0)
	{
		nIndex=0;
	}

	return m_LUT[nIndex];
}

BYTE CHistogram::LUT(BYTE *pValue, UINT DataType)
{
	switch(DataType)
	{
	case Pixel_Byte:
		return LUT(*(BYTE*)pValue);;
	case Pixel_Int16:
		return LUT(*(unsigned short*)pValue);
	case Pixel_SInt16:
		return LUT(*(short*)pValue);
	case Pixel_Int32:
		return LUT(*(unsigned int*)pValue);
	case Pixel_SInt32:
		return LUT(*(int*)pValue);
	case Pixel_Float:
		return LUT(*(float*)pValue);
	case Pixel_Double:
		return LUT(*(double*)pValue);
	default:
		return 0;
	}
	return 0;
}

unsigned int* CHistogram::GetHistogram()
{
	return m_Histogram;
}

BYTE* CHistogram::GetLUT()
{
	return m_LUT;
}
