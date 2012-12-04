// Histogram.h: interface for the CHistogram class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HISTOGRAM_H__16E9785D_545F_419D_A0E3_E409EA401578__INCLUDED_)
#define AFX_HISTOGRAM_H__16E9785D_545F_419D_A0E3_E409EA401578__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ImageDriver.h"
class CImage;

class CHistogram  
{
	typedef enum _enhancetype
	{
		ET_DEFAULT,
		ET_NONE,
		ET_LINER
	}EnhanceType;
public:
	BYTE* GetLUT();
	unsigned int* GetHistogram();
	BYTE LUT(BYTE* pValue,UINT DataType);
	BYTE LUT(double value);
	BYTE LUT(float value);
	BYTE LUT(int value);
	BYTE LUT(unsigned int value);
	BYTE LUT(short value);
	BYTE LUT(unsigned short value);
	BYTE LUT(BYTE value);
	BYTE* GetImageLUT();
	double GetMinGray();
	double GetMaxGray();
	void StatHistogram(CImage* pImage,int nBandIndex);
	CHistogram();
	virtual ~CHistogram();

protected:
	BOOL CalculateLUT(UINT datatype);

private:
	double m_BandMax;//��ǰ���εĻҶ����ֵ
	double m_BandMin;//��ǰ���εĻҶ���Сֵ
	unsigned int m_Histogram[256];
	unsigned int m_Histogram16[65536];//��ǰ���εĻҶ�ֱ��ͼ[0,65535]
	BYTE m_LUT[65536];//��ɫ���ұ�
	EnhanceType m_EnhanceType;

};

#endif // !defined(AFX_HISTOGRAM_H__16E9785D_545F_419D_A0E3_E409EA401578__INCLUDED_)
