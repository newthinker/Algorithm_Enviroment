// GeometryRectify.cpp: implementation of the CGeometryRectify class.
//
//////////////////////////////////////////////////////////////////////

#include <vector>
#include "apfloat.h"
using namespace std;
#include "GeometryRectify.h"
#include "Triangle.h"
#include "FArray.hpp"
#include "RPCParm.h"
#include "Dimap.h"
#include "BaseDefine.h"
#define TILE_SIZE 32

#define BiLinear(Type) void Type##_BiLinear(BYTE *buf11,BYTE *buf21, \
									   BYTE *buf22,BYTE *buf12, \
									   double Dx,double Dy,     \
									   BYTE *pOut)				\
{																\
	*((Type *)pOut) = (Type)((1-Dx) * (1-Dy) * (*((Type*)buf11))			\
			 + Dx     * (1-Dy) * (*((Type*)buf21))				\
			 + Dx     * Dy     * (*((Type*)buf22))				\
			 + (1-Dx) * Dy     * (*((Type*)buf12)));			\
}

BiLinear(BYTE)
BiLinear(USHORT)
BiLinear(short)
BiLinear(UINT)
BiLinear(int)
BiLinear(float)
BiLinear(double)
//BiLinear(__int64)

#define FuncName(Type) Type##_BiLinear

typedef void (* BiLinearFunc)(BYTE *buf11, BYTE *buf21, BYTE *buf22,
		BYTE *buf12, double Dx, double Dy, BYTE *pOut);

static BiLinearFunc FuncTable[9] = { FuncName(BYTE), FuncName(USHORT), FuncName(short), FuncName(UINT), FuncName(int), FuncName(float), FuncName(double) //,
		//FuncName(__int64),
		//FuncName(__int64)//the singed int64 is same as int64
		};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGeometryRectify::CGeometryRectify() {
	m_rotIMatrix[0][0]=0.0;
	m_rotIMatrix[0][1]=1.0;
	m_rotIMatrix[0][2]=0.0;
	m_rotIMatrix[1][0]=0.0;
	m_rotIMatrix[1][1]=0.0;
	m_rotIMatrix[1][2]=1.0;

	m_rotGMatrix[0][0]=0.0;
	m_rotGMatrix[0][1]=1.0;
	m_rotGMatrix[0][2]=0.0;
	m_rotGMatrix[1][0]=0.0;
	m_rotGMatrix[1][1]=0.0;
	m_rotGMatrix[1][2]=1.0;
}

CGeometryRectify::~CGeometryRectify() {

}

HRESULT CGeometryRectify::InitPolyTransMatrix(double *pSrcX, double *pSrcY,
		double *pDstX, double *pDstY,
		int nPtNum,
		double* pError,
		UINT nPolytransType)
{
	BOOL bRet=CalcPolyTransPara(pSrcX,pSrcY,pDstX,pDstY,nPtNum,nPolytransType,pError);

	return bRet?S_OK:S_FALSE;
}

HRESULT CGeometryRectify::PolyTrans(double SrcX, double SrcY, double *pDstX, double *pDstY)
{
	switch(m_transType)
	{
		case PT_LINEAR:
		*pDstX = m_pCoeX[0] + m_pCoeX[1] * SrcX + m_pCoeX[2] * SrcY;
		*pDstY = m_pCoeY[0] + m_pCoeY[1] * SrcX + m_pCoeY[2] * SrcY;
		break;

		case PT_NONLINEAR_XY:
		*pDstX = m_pCoeX[0] + m_pCoeX[1] * SrcX + m_pCoeX[2] * SrcY + m_pCoeX[3] * SrcX * SrcY;
		*pDstY = m_pCoeY[0] + m_pCoeY[1] * SrcX + m_pCoeY[2] * SrcY + m_pCoeY[3] * SrcX * SrcY;
		break;

		case PT_SQUARE:
		*pDstX = m_pCoeX[0] + m_pCoeX[1] * SrcX + m_pCoeX[2] * SrcY
		+ m_pCoeX[3] * SrcX * SrcX + m_pCoeX[4] * SrcY * SrcY + m_pCoeX[5] * SrcX * SrcY;

		*pDstY = m_pCoeY[0] + m_pCoeY[1] * SrcX + m_pCoeY[2] * SrcY
		+ m_pCoeY[3] * SrcX * SrcX + m_pCoeY[4] * SrcY * SrcY + m_pCoeY[5] * SrcX * SrcY;
		break;

		case PT_CUBE:
		case PT_CUBE_Z:
		*pDstX = m_pCoeX[0] + m_pCoeX[1] * SrcX + m_pCoeX[2] * SrcY
		+ m_pCoeX[3] * SrcX * SrcX + m_pCoeX[4] * SrcY * SrcY + m_pCoeX[5] * SrcX * SrcY
		+ m_pCoeX[6] * SrcX * SrcX * SrcX + m_pCoeX[7] * SrcY * SrcY * SrcY
		+ m_pCoeX[8] * SrcX * SrcX * SrcY + m_pCoeX[9] * SrcX * SrcY * SrcY;

		*pDstY = m_pCoeY[0] + m_pCoeY[1] * SrcX + m_pCoeY[2] * SrcY
		+ m_pCoeY[3] * SrcX * SrcX + m_pCoeY[4] * SrcY * SrcY + m_pCoeY[5] * SrcX * SrcY
		+ m_pCoeY[6] * SrcX * SrcX * SrcX + m_pCoeY[7] * SrcY * SrcY * SrcY
		+ m_pCoeY[8] * SrcX * SrcX * SrcY + m_pCoeY[9] * SrcX * SrcY * SrcY;				
		break;
	}

	return S_OK;
}

HRESULT CGeometryRectify::PolyTransZ(double SrcX,double SrcY,double SrcZ, double* pDstX,double* pDstY)
{
	switch(m_transType)
		{
			case PT_LINEAR:
			case PT_NONLINEAR_XY:
			case PT_SQUARE:
			case PT_CUBE:
			case PT_CUBE_Z:
			*pDstX = m_pCoeX[0] + m_pCoeX[1] * SrcX + m_pCoeX[2] * SrcY + m_pCoeX[3] * SrcZ
			+ m_pCoeX[4] * SrcX * SrcX + m_pCoeX[5] * SrcY * SrcY + m_pCoeX[6] * SrcZ * SrcZ
			+ m_pCoeX[7] * SrcX * SrcY + m_pCoeX[8] * SrcX * SrcZ + m_pCoeX[9] * SrcY * SrcZ
			+ m_pCoeX[10] * SrcX * SrcX * SrcX + m_pCoeX[11] * SrcY * SrcY *SrcY + m_pCoeX[12] * SrcX * SrcX * SrcY
			+ m_pCoeX[13] * SrcX * SrcY * SrcY + m_pCoeX[14] * SrcX * SrcY * SrcZ;

			*pDstY =  m_pCoeY[0] + m_pCoeY[1] * SrcX + m_pCoeY[2] * SrcY + m_pCoeY[3] * SrcZ
			+ m_pCoeY[4] * SrcX * SrcX + m_pCoeY[5] * SrcY * SrcY + m_pCoeY[6] * SrcZ * SrcZ
			+ m_pCoeY[7] * SrcX * SrcY + m_pCoeY[8] * SrcX * SrcZ + m_pCoeY[9] * SrcY * SrcZ
			+ m_pCoeY[10] * SrcX * SrcX * SrcX + m_pCoeY[11] * SrcY * SrcY *SrcY + m_pCoeY[12] * SrcX * SrcX * SrcY
			+ m_pCoeY[13] * SrcX * SrcY * SrcY + m_pCoeY[14] * SrcX * SrcY * SrcZ;
					
			break;
		}

		return S_OK;
}

HRESULT CGeometryRectify::ImagePolyRectify(char* pszSrcImage,
		char* pszDstImage,
		double *pSrcX, double *pSrcY,
		double *pDstX, double *pDstY,
		int nPtNum,
		UINT nPolytransType,
		UINT nResampleMethod,
		double outputLBX, double outputLBY,
		double outputRTX, double outputRTY,
		double outputGSD,
		double* pDstImgRgnX,double* pDstImgRgnY,
		//add by dy  20081027 for add pMatchBandParam for MatchBand begin
		double * pMatchBandParam)
{
	printf("inter so2 \n");
	//������������
	CProgress progress;
	//progress.AddCurInfo("[FGC]");

	//��ԭʼӰ��
	CImage srcImage;
	printf("inter so3 \n");
	if(srcImage.Open(pszSrcImage,modeRead)!=S_OK)
	{
		COutput::OutputFileOpenFailed(pszSrcImage);

		return S_FALSE;
	}
	//add by dy 20080925 for DE_ABERRATION ,it is Transfer Init ROWS begin
	
	double *pTempSrcX=pSrcX;
	 for(int i=0;i<nPtNum;i++)
	 {

	 //de_aberration(pTempSrcX,0);
	 pTempSrcX++;
	 }
	 //return S_OK;
	 pTempSrcX=NULL;

	//add by dy 20080925 for DE_ABERRATION ,it is Transfer Init ROWS end
	int nSrcRow,nSrcCol,nBandNum,nBPB;
	UINT datatype;
	srcImage.GetRows(&nSrcRow);
	srcImage.GetCols(&nSrcCol);
	srcImage.GetBandNum(&nBandNum);
	srcImage.GetDataType(&datatype);
	srcImage.GetBPB(&nBPB);
	int nBPP=nBPB*nBandNum;
	//ȷ�ϸ�����Ӱ��Χ��׼ȷ��-
	if(fabs(outputRTY-outputLBY)<1e-6||fabs(outputRTX-outputLBX)<1e-6)
	{
		double X0,Y0,X1,Y1,X2,Y2,X3,Y3;
		double Error;
		double dLX,dRX;
		dLX=0;
		dRX=nSrcCol;
		//printf("LX is %lf \n",dLX);
		//printf("RX is %lf \n",dRX);
		//de_aberration(&dLX,0);
		//de_aberration(&dRX,0);
		printf("LX is %lf \n",dLX);
		//	printf("RX is %lf \n",dRX);
		InitPolyTransMatrix(pSrcX,pSrcY,pDstX,pDstY,nPtNum,&Error,nPolytransType);
		printf("Error is %lf \n",Error);
		PolyTrans(dLX,0,&X0,&Y0);
		PolyTrans(dLX,nSrcRow,&X1,&Y1);
		PolyTrans(dRX,nSrcRow,&X2,&Y2);
		PolyTrans(dRX,0,&X3,&Y3);
		outputLBX=__min(__min(X0,X1),__min(X2,X3));
		outputLBY=__min(__min(Y0,Y1),__min(Y2,Y3));
		outputRTX=__max(__max( X0,X1),__max(X2,X3));
		outputRTY=__max(__max(Y0,Y1),__max(Y2,Y3));

		pDstImgRgnX[0]=X0;
		pDstImgRgnY[0]=Y0;

		pDstImgRgnX[1]=X1;
		pDstImgRgnY[1]=Y1;

		pDstImgRgnX[2]=X2;
		pDstImgRgnY[2]=Y2;

		pDstImgRgnX[3]=X3;
		pDstImgRgnY[3]=Y3;
	}
	//����Ŀ��Ӱ���������
	int nDstRow=abs(int((outputRTY-outputLBY)/outputGSD+0.5));
	int nDstCol=abs(int((outputRTX-outputLBX)/outputGSD+0.5));
	//����Ŀ��Ӱ��
	CImage dstImage;

	if(dstImage.CreateImg(pszDstImage,modeCreate|modeWrite,
					nDstCol,nDstRow,datatype,
					nBandNum,BSQ,
					outputLBX,outputLBY,outputGSD)!=S_OK)
	{
		COutput::OutputFileOpenFailed(pszDstImage);

		return S_FALSE;
	}
	//������ɫ����Ϣ
	HRESULT hColorTable=srcImage.HaveColorTable();
	if(hColorTable==S_OK)
	{
		int nEntryNum=0;
		srcImage.GetEntryNum(&nEntryNum);
		RGBQUAD* pColorTable=new RGBQUAD[nEntryNum];
		srcImage.GetColorTable((BYTE*)pColorTable);
		dstImage.SetColorTable((BYTE*)pColorTable,nEntryNum);
		delete [] pColorTable;
		pColorTable=NULL;
	}

	//�����Ƶ������ת��Ϊ���Ƶ���Ŀ��Ӱ���ϵ�Ӱ�����
	double* pDstImgX=new double[nPtNum];
	double* pDstImgY=new double[nPtNum];
	int i=0;
	for(i=0;i<nPtNum;i++)
	{
		pDstImgX[i]=(pDstX[i]-outputLBX)/outputGSD;
		pDstImgY[i]=(pDstY[i]-outputLBY)/outputGSD;
	}

	//����Ŀ��Ӱ��ԴӰ���ת������
	double Error;
	InitPolyTransMatrix(pDstImgX,pDstImgY,pSrcX,pSrcY,nPtNum,&Error,nPolytransType);

	delete [] pDstImgX; pDstImgX=NULL;
	delete [] pDstImgY; pDstImgY=NULL;

	char szTemp[256];
	sprintf(szTemp,"Processing %s ...",pszSrcImage);
	progress.AddCurInfo(szTemp);
	progress.SetRange(0,nDstRow);
	progress.SetPosition(0);
	progress.SetStep(1);

	int nSensorType;
	srcImage.GetSensorType(&nSensorType);	
	BYTE* pDstBuffer=new BYTE[nDstCol*nBPP];
	for(i=nDstRow-1; i>=0; i--)
	{
		for(int j=0;j<nDstCol;j++)
		{
			//��Ŀ��Ӱ�����ת��ΪԴӰ�����
			double x,y;
			PolyTrans(j,i,&x,&y);

			// Bands circling correcting for CCD.
			// But no need while correct from Level2. [ZuoW, 11/11/2009]
			if(nSensorType==SENSOR_CCD1||nSensorType==SENSOR_CCD2)
			{
//				x=x+pMatchBandParam[1];
//				y=y+x*pMatchBandParam[2]+pMatchBandParam[3];
			}

			if( (x<0) || (x>=nSrcCol) || (y<0) || (y>=nSrcRow))
			{
				memset(pDstBuffer+j*nBPP,0,nBPP);
			}
			else
			{
				srcImage.GetPixelF((float)x,(float)y,pDstBuffer+j*nBPP,nResampleMethod);
			}
		}

		dstImage.WriteImg(0.0f,(float)i,(float)nDstCol,(float)(i+1),
				pDstBuffer,nDstCol,1,nBandNum,
				0,0,nDstCol,1,
				-1, 0);

		progress.StepIt();
	}
	delete [] pDstBuffer; pDstBuffer=NULL;

	// Clear memory.
	srcImage.Close();
	dstImage.Close();

	return S_OK;
}

HRESULT CGeometryRectify::ImagePolyZRectify(CDEM* pDEM, char* pszSrcImage,
               char* pszDstImage,
			   double* pSrcX,double* pSrcY,
			   double* pDstX,double* pDstY, double* pDstZ, 
			   int  nPtNum,
			   UINT nPolytransType,
			   UINT nResampleMethod,
			   double outputLBX,double outputLBY,
			   double outputRTX,double outputRTY,
			   double outputGSD,
			   double* pDstImgRgnX,
			   double* pDstImgRgnY,
			   double * pMatchBandParam)
{
	printf("inter so2 \n");
	
	//������������
	CProgress progress;

	//��ԭʼӰ��
	CImage srcImage;
	printf("inter so3 \n");
	if(srcImage.Open(pszSrcImage,modeRead)!=S_OK)
	{
		COutput::OutputFileOpenFailed(pszSrcImage);

		return S_FALSE;
	}

	double *pTempSrcX=pSrcX;
	for(int i=0;i<nPtNum;i++)
	{

	//de_aberration(pTempSrcX,0);
	pTempSrcX++;
	}
	//return S_OK;
	pTempSrcX=NULL;

	//add by dy 20080925 for DE_ABERRATION ,it is Transfer Init ROWS end
	int nSrcRow,nSrcCol,nBandNum,nBPB;
	UINT datatype;
	srcImage.GetRows(&nSrcRow);
	srcImage.GetCols(&nSrcCol);
	srcImage.GetBandNum(&nBandNum);
	srcImage.GetDataType(&datatype);
	srcImage.GetBPB(&nBPB);
	int nBPP=nBPB*nBandNum;
	//ȷ�ϸ�����Ӱ��Χ��׼ȷ��-
	if(fabs(outputRTY-outputLBY)<1e-6||fabs(outputRTX-outputLBX)<1e-6)
	{
		double X0,Y0,X1,Y1,X2,Y2,X3,Y3;
		double Error;
		double dLX,dRX;
		dLX=0;
		dRX=nSrcCol;
		//printf("LX is %lf \n",dLX);
		//printf("RX is %lf \n",dRX);
		//de_aberration(&dLX,0);
		//de_aberration(&dRX,0);
		printf("LX is %lf \n",dLX);
		//	printf("RX is %lf \n",dRX);
		InitPolyTransMatrix(pSrcX,pSrcY,pDstX,pDstY,nPtNum,&Error,nPolytransType);
		printf("Error is %lf \n",Error);
		PolyTrans(dLX,0,&X0,&Y0);
		PolyTrans(dLX,nSrcRow,&X1,&Y1);
		PolyTrans(dRX,nSrcRow,&X2,&Y2);
		PolyTrans(dRX,0,&X3,&Y3);
		outputLBX=__min(__min(X0,X1),__min(X2,X3));
		outputLBY=__min(__min(Y0,Y1),__min(Y2,Y3));
		outputRTX=__max(__max( X0,X1),__max(X2,X3));
		outputRTY=__max(__max(Y0,Y1),__max(Y2,Y3));

		pDstImgRgnX[0]=X0;
		pDstImgRgnY[0]=Y0;

		pDstImgRgnX[1]=X1;
		pDstImgRgnY[1]=Y1;

		pDstImgRgnX[2]=X2;
		pDstImgRgnY[2]=Y2;

		pDstImgRgnX[3]=X3;
		pDstImgRgnY[3]=Y3;
	}
	//����Ŀ��Ӱ���������
	int nDstRow=abs(int((outputRTY-outputLBY)/outputGSD+0.5));
	int nDstCol=abs(int((outputRTX-outputLBX)/outputGSD+0.5));
	//����Ŀ��Ӱ��
	CImage dstImage;

	if(dstImage.CreateImg(pszDstImage,modeCreate|modeWrite,
					nDstCol,nDstRow,datatype,
					nBandNum,BSQ,
					outputLBX,outputLBY,outputGSD)!=S_OK)
	{
		COutput::OutputFileOpenFailed(pszDstImage);

		return S_FALSE;
	}
	//������ɫ����Ϣ
	HRESULT hColorTable=srcImage.HaveColorTable();
	if(hColorTable==S_OK)
	{
		int nEntryNum=0;
		srcImage.GetEntryNum(&nEntryNum);
		RGBQUAD* pColorTable=new RGBQUAD[nEntryNum];
		srcImage.GetColorTable((BYTE*)pColorTable);
		dstImage.SetColorTable((BYTE*)pColorTable,nEntryNum);
		delete [] pColorTable;
		pColorTable=NULL;
	}

	//�����Ƶ������ת��Ϊ���Ƶ���Ŀ��Ӱ���ϵ�Ӱ�����
	int i=0;
	
	//����Ŀ��Ӱ��ԴӰ���ת������
	double Error;
	CalcPolyZTransPara(pDstX,pDstY,pDstZ,pSrcX,pSrcY,nPtNum,nPolytransType,&Error);
/*	// Output the coef.
	FILE* fp = fopen("/dps/workdir/CCD2/LEVEL3/376/TinCoef.txt", "a+");
	if(fp!=NULL)
	{
		for(int num=0; num<nPtNum; num++)
		{
			fprintf(fp, "The %dth point:%lf, %lf, %lf, %lf, %lf\n", num+1,
					pSrcX[num], pSrcY[num], pDstX[num], pDstY[num], pDstZ[num]);
		}
		fprintf(fp, "The coef. are:\n");
		for(int num=0; num<15; num++)
		{
			fprintf(fp, "%8.6e, %8.6e\n", m_pCoeX[num], m_pCoeY[num]);
		}
		fprintf(fp, "\n");
		fprintf(fp, "\n");
		fprintf(fp, "\n");
		fclose(fp);
	}
*/
// Print the coef. [ZuoW, 2010/5/20]
	printf("X coef.:%f, %f, %f\n", m_pCoeX[0], m_pCoeX[1], m_pCoeX[2]);
	printf("Y coef.:%f, %f, %f\n", m_pCoeY[0], m_pCoeY[1], m_pCoeY[2]);

	char szTemp[256];
	sprintf(szTemp,"Processing %s ...",pszSrcImage);
	progress.AddCurInfo(szTemp);
	progress.SetRange(0,nDstRow);
	progress.SetPosition(0);
	progress.SetStep(1);

	int nSensorType;
	srcImage.GetSensorType(&nSensorType);	
	BYTE* pDstBuffer=new BYTE[nDstCol*nBPP];
	for(i=0; i<nDstRow; i++)
	{
		for(int j=0;j<nDstCol;j++)
		{
			//��Ŀ��Ӱ�����ת��ΪԴӰ�����
			double x,y;
			double X,Y,Z;
			
			X = outputLBX + outputGSD * j;
			Y = outputLBY + outputGSD * i;
			
			pDEM->GetAltitude(X, Y, &Z, 0);
			if(fabs(Z-INVALID_ALTITUDE)<1e-5)	
				pDEM->GetAverageAltitude(&Z);	
			
			PolyTransZ(X,Y,Z,&x,&y);

			// Bands circling correcting for CCD. [ZuoW,2009/4/17]
			if(nSensorType==SENSOR_CCD1||nSensorType==SENSOR_CCD2)
			{
				x=x+pMatchBandParam[1];
				y=y+x*pMatchBandParam[2]+pMatchBandParam[3];
			}

			if( (x<0) || (x>=nSrcCol) || (y<0) || (y>=nSrcRow))
			{
				memset(pDstBuffer+j*nBPP,0,nBPP);
			}
			else
			{
				srcImage.GetPixelF((float)x,(float)y,pDstBuffer+j*nBPP,nResampleMethod);
			}
		}

		dstImage.WriteImg(0.0f,(float)i,(float)nDstCol,(float)(i+1),
				pDstBuffer,nDstCol,1,nBandNum,
				0,0,nDstCol,1,
				-1, 0);

		progress.StepIt();
	}
	delete [] pDstBuffer; pDstBuffer=NULL;

	// Clear memory.
	srcImage.Close();
	dstImage.Close();

	return S_OK;
}

HRESULT CGeometryRectify::ImageTinRectify(char* pszSrcImage,
		char* pszDstImage,
		double *pSrcX, double *pSrcY,
		double *pDstX, double *pDstY,
		int nPtNum,
		UINT nPolytransType,					// ����ʽģ�ͽ��� [WeiZ,6/8/2009]
		UINT nResampleMethod,
		double outputLBX, double outputLBY,
		double outputRTX, double outputRTY,
		double outputGSD)
{
	HRESULT hRet=S_FALSE;
	// 进度条
	CProgress progress;

	CImage srcImage;
	if(srcImage.Open(pszSrcImage,modeRead)!=S_OK)
	{
		COutput::OutputFileOpenFailed(pszSrcImage);

		return S_FALSE;
	}

	int nSrcRow,nSrcCol;
	srcImage.GetRows(&nSrcRow);
	srcImage.GetCols(&nSrcCol);
	int nBandNum;
	srcImage.GetBandNum(&nBandNum);
	UINT datatype;
	srcImage.GetDataType(&datatype);
	int nBPB;
	srcImage.GetBPB(&nBPB);

	if(fabs(outputRTY-outputLBY)<1e-6||fabs(outputRTX-outputLBX)<1e-6)
	{
		double X0,Y0,X1,Y1,X2,Y2,X3,Y3;
		double Error;
		double inLBX=0.0f;
		double inLBY=0.0f;
		double inRTX=nSrcCol-1;
		double inRTY=nSrcRow-1;
		//de_aberration(&inLBX,0);
		//de_aberration(&inLBY,0);
		//de_aberration(&inRTX,0);
		//de_aberration(&inRTY,0);
		printf("inLBX is %lf,inLBY is %lf,inRTX is %lf ,inRTY is %lf\n",inLBX,inLBY,inRTX,inRTY);
		InitPolyTransMatrix(pSrcX,pSrcY,pDstX,pDstY,nPtNum,&Error,nPolytransType);
		PolyTrans(inLBX,inLBY,&X0,&Y0);
		PolyTrans(inLBX,inRTY,&X1,&Y1);
		PolyTrans(inRTX,inRTY,&X2,&Y2);
		PolyTrans(inRTX,inLBY,&X3,&Y3);

		outputLBX=__min(__min(X0,X1),__min(X2,X3));
		outputLBY=__min(__min(Y0,Y1),__min(Y2,Y3));
		outputRTX=__max(__max(X0,X1),__max(X2,X3));
		outputRTY=__max(__max(Y0,Y1),__max(Y2,Y3));
	}
	// 目标影像行列号
	int nDstRow=abs(int((outputRTY-outputLBY)/outputGSD+0.5));
	int nDstCol=abs(int((outputRTX-outputLBX)/outputGSD+0.5));

	CImage dstImage;
	if(dstImage.CreateImg(pszDstImage,modeCreate|modeWrite,
					nDstCol,nDstRow,datatype,
					nBandNum,BSQ,
					outputLBX,outputLBY,outputGSD)!=S_OK)
	{
		COutput::OutputFileOpenFailed(pszDstImage);

		return S_FALSE;
	}

	HRESULT hColorTable=srcImage.HaveColorTable();
	if(hColorTable==S_OK)
	{
		int nEntryNum=0;
		srcImage.GetEntryNum(&nEntryNum);
		RGBQUAD* pColorTable=new RGBQUAD[nEntryNum];
		srcImage.GetColorTable((BYTE*)pColorTable);
		dstImage.SetColorTable((BYTE*)pColorTable,nEntryNum);
		delete [] pColorTable;
		pColorTable=NULL;
	}

	double* pSrcImgX=new double[nPtNum+4];
	double* pSrcImgY=new double[nPtNum+4];
	double* pDstImgX=new double[nPtNum+4];
	double* pDstImgY=new double[nPtNum+4];
	int i=0;
	for(i=0;i<nPtNum;i++)
	{
		pSrcImgX[i]=pSrcX[i];
		pSrcImgY[i]=pSrcY[i];
		pDstImgX[i]=(pDstX[i]-outputLBX)/outputGSD;
		pDstImgY[i]=(pDstY[i]-outputLBY)/outputGSD;
	}

	double Error;
	InitPolyTransMatrix(pDstImgX,pDstImgY,pSrcImgX,pSrcImgY,nPtNum,&Error,nPolytransType);
	pDstImgX[nPtNum+0]=0; pDstImgY[nPtNum+0]=0;
	pDstImgX[nPtNum+1]=0; pDstImgY[nPtNum+1]=nDstRow-1;
	pDstImgX[nPtNum+2]=nDstCol-1; pDstImgY[nPtNum+2]=nDstRow-1;
	pDstImgX[nPtNum+3]=nDstCol-1; pDstImgY[nPtNum+3]=0;
	for(i=0;i<4;i++)
	{
		PolyTrans(pDstImgX[nPtNum+i],pDstImgY[nPtNum+i],&pSrcImgX[nPtNum+i],&pSrcImgY[nPtNum+i]);
	}
	nPtNum=nPtNum+4;

	// 构建TIN
	triangulateio in,mid,vorout;
	memset(&in,0,sizeof(triangulateio));
	memset(&mid,0,sizeof(triangulateio));
	memset(&vorout,0,sizeof(triangulateio));

	in.numOfPoints=nPtNum;
	in.numOfPointAttrs=1;
	in.numOfTriangles=0;
	in.numOfCorners=0;
	in.numOfHoles=0;
	in.numOfRegions=0;
	in.numOfSegments=0;

	in.pointList=new REAL[in.numOfPoints*2];
	in.pointAttrList=new REAL[in.numOfPoints];
	in.pointMarkList=new int[in.numOfPoints];

	// Output the points.
//	FILE *fp = fopen("/home/zuowei/workspace/test/gm.txt", "w");
//	if(fp==NULL)
//		printf("Bad file!\n");

//	fprintf(fp, "%d\n", nPtNum);
	for(i=0;i<nPtNum;i++)
	{
		in.pointList[i*2+0]=pDstImgX[i];
		in.pointList[i*2+1]=pDstImgY[i];
		in.pointAttrList[i]=0.0;
		in.pointMarkList[i]=0;

//		fprintf(fp, "%d     %f     %f\n", i+1, pDstImgX[i], pDstImgY[i]);
	}
//	fclose(fp);

	triangulate("pczAevnQP", &in, &mid, &vorout);

	char szTemp[256];
	sprintf(szTemp,"Processing %s ...",pszSrcImage);
	progress.AddCurInfo(szTemp);
	progress.SetRange(0,mid.numOfTriangles);
	progress.SetPosition(0);
	progress.SetStep(1);

	double xSrcTriangle[CONST_NUM],ySrcTriangle[CONST_NUM];		// 3 Points --> 6 Points
	double xDstTriangle[CONST_NUM],yDstTriangle[CONST_NUM];
	for(i=0; i<CONST_NUM; i++)		// Initialize
	{
		xSrcTriangle[i] = ySrcTriangle[i] = xDstTriangle[i] = yDstTriangle[i] = -99999.0;
	}
	for(i=0;i<mid.numOfTriangles;i++)
	{
		progress.StepIt();

		int nTriPtIndex=0;
		for(nTriPtIndex=0;nTriPtIndex<3;nTriPtIndex++)
		{
			xDstTriangle[nTriPtIndex]=mid.pointList[mid.triList[i*3+nTriPtIndex]*2+0];
			yDstTriangle[nTriPtIndex]=mid.pointList[mid.triList[i*3+nTriPtIndex]*2+1];
			if(mid.triList[i*3+nTriPtIndex]>=nPtNum)
			{
				break;
			}
			xSrcTriangle[nTriPtIndex]=pSrcImgX[mid.triList[i*3+nTriPtIndex]];
			ySrcTriangle[nTriPtIndex]=pSrcImgY[mid.triList[i*3+nTriPtIndex]];

//			printf("Corner %d's coordinateion:%f, %f, %f, %f\n", nTriPtIndex,
//					xDstTriangle[nTriPtIndex], yDstTriangle[nTriPtIndex],
//					xSrcTriangle[nTriPtIndex],ySrcTriangle[nTriPtIndex]);
		}

		// Get the neighbor triangle's corner
//        printf("Num of corners:%d\n",mid.numOfCorners);
		printf("Currnet triangle ID:%d\n", i+1);
		printf("Current Corners: %d, %d, %d\n", mid.triList[i*3], mid.triList[i*3+1],
				mid.triList[i*3+2]);
		int j = 0;
		for(j=0; j<3; j++)
		{
			int iNghTriID = mid.neighborList[i*3+j];
			printf("Neighbor triangle ID:%4d\n", iNghTriID);
			if(iNghTriID!=-1)	// Get the valid neighbor triangle ID
			{
				printf("Neighbor Corners:%d, %d, %d\n", mid.triList[iNghTriID*3], mid.triList[iNghTriID*3+1],
						mid.triList[iNghTriID*3+2]);
				int k=0;
				for(k=0; k<3; k++)
				{
					int iCorNum = mid.triList[iNghTriID*3+k];
					if(iCorNum!=mid.triList[i*3] && iCorNum!=mid.triList[i*3+1] &&
							iCorNum!=mid.triList[i*3+2])
					{
//						xDstTriangle[3+k] = mid.pointList[mid.triList[iNghTriID*3+k]*2+0];
//						yDstTriangle[3+k] = mid.pointList[mid.triList[iNghTriID*3+k]*2+1];

//						xSrcTriangle[3+k] = pSrcImgX[mid.triList[iNghTriID*3+k]];
//						ySrcTriangle[3+k] = pSrcImgY[mid.triList[iNghTriID*3+k]];
					}
				}
			}
		}
		if(nTriPtIndex<3)
		{
			continue;
		}

		TriangleFillinFst(&srcImage,&dstImage,xSrcTriangle,ySrcTriangle,xDstTriangle,yDstTriangle,nResampleMethod);
	}
	hRet=S_OK;

	//���������ڴ�
	delete [] in.pointList;
	delete [] in.pointAttrList;
	delete [] in.pointMarkList;
	delete [] in.triList;
	delete [] in.triAreaList;
	delete [] in.triAttrList;
	delete [] in.segmentList;
	delete [] in.holeList;
	delete [] in.regionList;
	delete [] in.segMarkList;

	free(mid.pointAttrList);mid.pointAttrList=NULL;
	free(mid.pointMarkList);mid.pointMarkList=NULL;
	free(mid.triList); mid.triList=NULL;
	free(mid.neighborList); mid.neighborList=NULL;
	free(mid.segmentList); mid.segmentList=NULL;
	free(mid.segMarkList); mid.segMarkList=NULL;
	free(mid.edgeList); mid.edgeList=NULL;
	free(mid.edgeMarkList); mid.edgeMarkList=NULL;

	free(vorout.pointList);
	free(vorout.pointAttrList);
	free(vorout.pointMarkList);
	free(vorout.edgeList);
	free(vorout.normList);

	delete [] pSrcImgX; pSrcImgX=NULL;
	delete [] pSrcImgY; pSrcImgY=NULL;
	delete [] pDstImgX; pDstImgX=NULL;
	delete [] pDstImgY; pDstImgY=NULL;

	srcImage.Close();
	dstImage.Close();

	return hRet;
}

/*
 * 改进后的TIN纠正，增加了根据多项式阶数自动搜索GCP功能，并融合了带高程的多项式纠正算法。 [ZuoW,2010/7/7]
 */
HRESULT CGeometryRectify::ImageTinRectify(char* pszSrcImage,
		char* pszDstImage, char* szDEMFile,
		double *pSrcX, double *pSrcY,
		double *pDstX, double *pDstY, double *pDstZ,
		int nPtNum,
		UINT nPolytransType,
		UINT nResampleMethod,
		double outputLBX, double outputLBY,
		double outputRTX, double outputRTY,
		double outputGSD)
{
	HRESULT hRet=S_FALSE;
	// 进度条
	CProgress progress;

	// 打开源影像
	CImage srcImage;
	if(srcImage.Open(pszSrcImage,modeRead)!=S_OK)
	{
		COutput::OutputFileOpenFailed(pszSrcImage);

		return S_FALSE;
	}

	int nSrcRow,nSrcCol;
	srcImage.GetRows(&nSrcRow);
	srcImage.GetCols(&nSrcCol);
	int nBandNum;
	srcImage.GetBandNum(&nBandNum);
	UINT datatype;
	srcImage.GetDataType(&datatype);
	int nBPB;
	srcImage.GetBPB(&nBPB);
//	int nBPP=nBPB*nBandNum;
	// 计算目标影像范围
	if(fabs(outputRTY-outputLBY)<1e-6||fabs(outputRTX-outputLBX)<1e-6)
	{
		double X0,Y0,X1,Y1,X2,Y2,X3,Y3;
		double Error;
		double inLBX=0.0f;
		double inLBY=0.0f;
		double inRTX=nSrcCol-1;
		double inRTY=nSrcRow-1;
		//de_aberration(&inLBX,0);
		//de_aberration(&inLBY,0);
		//de_aberration(&inRTX,0);
		//de_aberration(&inRTY,0);
		printf("inLBX is %lf,inLBY is %lf,inRTX is %lf ,inRTY is %lf\n",inLBX,inLBY,inRTX,inRTY);
		InitPolyTransMatrix(pSrcX,pSrcY,pDstX,pDstY,nPtNum,&Error,nPolytransType);
		PolyTrans(inLBX,inLBY,&X0,&Y0);
		PolyTrans(inLBX,inRTY,&X1,&Y1);
		PolyTrans(inRTX,inRTY,&X2,&Y2);
		PolyTrans(inRTX,inLBY,&X3,&Y3);

		outputLBX=__min(__min(X0,X1),__min(X2,X3));
		outputLBY=__min(__min(Y0,Y1),__min(Y2,Y3));
		outputRTX=__max(__max(X0,X1),__max(X2,X3));
		outputRTY=__max(__max(Y0,Y1),__max(Y2,Y3));
	}
	// 计算目标影像行/列号
	int nDstRow=abs(int((outputRTY-outputLBY)/outputGSD+0.5));
	int nDstCol=abs(int((outputRTX-outputLBX)/outputGSD+0.5));
	// 生成目标影像
	CImage dstImage;
	if(dstImage.CreateImg(pszDstImage,modeCreate|modeWrite,
					nDstCol,nDstRow,datatype,
					nBandNum,BSQ,
					outputLBX,outputLBY,outputGSD)!=S_OK)
	{
		COutput::OutputFileOpenFailed(pszDstImage);

		return S_FALSE;
	}
	// 如果源影像有颜色表就赋值给目标影像
	HRESULT hColorTable=srcImage.HaveColorTable();
	if(hColorTable==S_OK)
	{
		int nEntryNum=0;
		srcImage.GetEntryNum(&nEntryNum);
		RGBQUAD* pColorTable=new RGBQUAD[nEntryNum];
		srcImage.GetColorTable((BYTE*)pColorTable);
		dstImage.SetColorTable((BYTE*)pColorTable,nEntryNum);
		delete [] pColorTable;
		pColorTable=NULL;
	}

	/// 打开DEM文件 [ZuoW,2010/5/18]
	CDEM* pDEM = new CNSDTFDEM;
	hRet = pDEM->Open(szDEMFile, 0, modeRead);
	if(hRet!=S_OK)
	{
		printf("Open DEM file failed and return!\n");
		return S_FALSE;
	}

	// 将四个角点加入控制点列表中
	double* pSrcImgX=new double[nPtNum+4];
	double* pSrcImgY=new double[nPtNum+4];
	double* pDstImgX=new double[nPtNum+4];
	double* pDstImgY=new double[nPtNum+4];
	double* pDstImgZ=new double[nPtNum+4];
	int i=0;
	for(i=0;i<nPtNum;i++)
	{
		pSrcImgX[i]=pSrcX[i];
		pSrcImgY[i]=pSrcY[i];
		pDstImgX[i]=(pDstX[i]-outputLBX)/outputGSD;
		pDstImgY[i]=(pDstY[i]-outputLBY)/outputGSD;
		pDstImgZ[i]=pDstZ[i];
	}
	// 计算四个角点的坐标
	pSrcImgX[nPtNum+0] = 0;	pSrcImgY[nPtNum+0] = 0;
	pSrcImgX[nPtNum+1] = 0;	pSrcImgY[nPtNum+1] = nSrcRow-1;
	pSrcImgX[nPtNum+2] = nSrcCol-1;	pSrcImgY[nPtNum+2] = nSrcRow-1;
	pSrcImgX[nPtNum+3] = nSrcCol-1; pSrcImgY[nPtNum+3] = 0;
	pDstImgX[nPtNum+0]=0; pDstImgY[nPtNum+0]=0;
	pDstImgX[nPtNum+1]=0; pDstImgY[nPtNum+1]=nDstRow-1;
	pDstImgX[nPtNum+2]=nDstCol-1; pDstImgY[nPtNum+2]=nDstRow-1;
	pDstImgX[nPtNum+3]=nDstCol-1; pDstImgY[nPtNum+3]=0;
	for(i=0;i<4;i++)
	{
		double Z = 0.;
		pDEM->GetAltitude(outputLBX+pDstImgX[nPtNum+i]*outputGSD, outputLBY+pDstImgY[nPtNum+i]*outputGSD, &Z, 0);
		pDstImgZ[nPtNum+i] = Z;
	}
	nPtNum=nPtNum+4;

	// 构造TIN
	triangulateio in,mid,vorout;
	memset(&in,0,sizeof(triangulateio));
	memset(&mid,0,sizeof(triangulateio));
	memset(&vorout,0,sizeof(triangulateio));

	in.numOfPoints=nPtNum;
	in.numOfPointAttrs=1;
	in.numOfTriangles=0;
	in.numOfCorners=0;
	in.numOfHoles=0;
	in.numOfRegions=0;
	in.numOfSegments=0;

	in.pointList=new REAL[in.numOfPoints*2];
	in.pointAttrList=new REAL[in.numOfPoints];
	in.pointMarkList=new int[in.numOfPoints];

/*	/// 输出TIN顶点
	FILE *fp = fopen("/home/zuowei/workspace/test/gm.txt", "w");
	if(fp==NULL)
		printf("Bad file!\n");
	fprintf(fp, "%d\n", nPtNum);
*/	
	for(i=0;i<nPtNum;i++)
	{
		in.pointList[i*2+0]=pDstImgX[i];
		in.pointList[i*2+1]=pDstImgY[i];
		in.pointAttrList[i]=0.0;
		in.pointMarkList[i]=0;

//		fprintf(fp, "%d     %f     %f\n", i+1, pDstImgX[i], pDstImgY[i]);
	}
//	fclose(fp);

	triangulate("pczAevnQP", &in, &mid, &vorout);

	char szTemp[256];
	sprintf(szTemp,"Processing %s ...",pszSrcImage);
	progress.AddCurInfo(szTemp);
	progress.SetRange(0,mid.numOfTriangles);
	progress.SetPosition(0);
	progress.SetStep(1);

	for(i=0;i<mid.numOfTriangles;i++)
	{
		progress.StepIt();

		// 采用通用接口方式
		double *xSrcTriangle = NULL;	double *ySrcTriangle = NULL;
		double *xDstTriangle = NULL;	double *yDstTriangle = NULL;	double *zDstTriangle = NULL;
		int ptNum = 0;
		ObtainTriangleCorner(mid, i, nPolytransType, pSrcImgX, pSrcImgY, pDstImgZ,
				xSrcTriangle, ySrcTriangle, xDstTriangle, yDstTriangle, zDstTriangle, ptNum);
		if(ptNum<=0)
		{
			printf("Invalid GCP count and return!\n");
			return S_FALSE;
		}
		
		TriangleFillinFst(pDEM, &srcImage,&dstImage, ptNum, xSrcTriangle,ySrcTriangle,xDstTriangle,yDstTriangle,zDstTriangle,nPolytransType,nResampleMethod);
		
		// 内存回收
		if (xSrcTriangle!=NULL)
		{
			delete [] xSrcTriangle;		xSrcTriangle = NULL;
		}
		if (ySrcTriangle!=NULL)
		{
			delete [] ySrcTriangle;		ySrcTriangle = NULL;
		}
		if (xDstTriangle!=NULL)
		{
			delete [] xDstTriangle;		xDstTriangle = NULL;
		}
		if (yDstTriangle!=NULL)
		{
			delete [] yDstTriangle;		yDstTriangle = NULL;
		}		
	}
	hRet=S_OK;

	// 清理内存
	delete [] in.pointList;
	delete [] in.pointAttrList;
	delete [] in.pointMarkList;
	delete [] in.triList;
	delete [] in.triAreaList;
	delete [] in.triAttrList;
	delete [] in.segmentList;
	delete [] in.holeList;
	delete [] in.regionList;
	delete [] in.segMarkList;

	free(mid.pointAttrList);mid.pointAttrList=NULL;
	free(mid.pointMarkList);mid.pointMarkList=NULL;
	free(mid.triList); mid.triList=NULL;
	free(mid.neighborList); mid.neighborList=NULL;
	free(mid.segmentList); mid.segmentList=NULL;
	free(mid.segMarkList); mid.segMarkList=NULL;
	free(mid.edgeList); mid.edgeList=NULL;
	free(mid.edgeMarkList); mid.edgeMarkList=NULL;

	free(vorout.pointList);
	free(vorout.pointAttrList);
	free(vorout.pointMarkList);
	free(vorout.edgeList);
	free(vorout.normList);

	delete [] pSrcImgX; pSrcImgX=NULL;
	delete [] pSrcImgY; pSrcImgY=NULL;
	delete [] pDstImgX; pDstImgX=NULL;
	delete [] pDstImgY; pDstImgY=NULL;
	delete [] pDstImgZ;	pDstImgZ=NULL;

	// 关闭DEM和源/目标影像
	if(pDEM!=NULL)
	{
		pDEM->Close();
		delete pDEM;
		pDEM = NULL;
	}
	srcImage.Close();
	dstImage.Close();

	return hRet;
}

/*
 * 根据多项式阶数初始化控制点。 [ZuoW,2010/3/8]
 */
void CGeometryRectify::ObtainTriangleCorner(triangulateio mid, int nTriID, int nPolytransType, double* pSrcImgX, double* pSrcImgY, double* pDstImgZ,
		double* &dxSrcTriangle, double* &dySrcTriangle, double* &dxDstTriangle, double* &dyDstTriangle, double* &dzDstTriangle, int &ptNum)
{
	bool bTriExist = false;
	bool bPtExist = false;
	vector<int> triangleID;	// 保存triangleID的栈
	vector<int> cornerID;	// 保存cornerID的容器
	/*
	 * 找出指定ID三角形周边的三角形并保存所有三角形的顶点坐标。 [ZuoW,2010/3/9]
	 */
	if(nTriID<0)		// 检查triangle ID.
	{
		printf("Invalid triangle ID and return!\n");
		return;
	}
	triangleID.push_back(nTriID);	// 将当前triangle ID存入容器中 [ZuoW,2010/3/24]
	// 将第一个三角形三个顶点存入容器中 [ZuoW,2010/3/24]
	cornerID.push_back(mid.triList[nTriID*3+0]);
	cornerID.push_back(mid.triList[nTriID*3+1]);
	cornerID.push_back(mid.triList[nTriID*3+2]);
	int ptCurNum = 3;	// 当前的GCP个数 [ZuoW,2010/3/24]
	int ptTotalNum = (nPolytransType+1)*(nPolytransType+2)/2;	// 需要的GCP个数 [ZuoW,2010/3/24]

	vector<int>::iterator triBegin = triangleID.begin();
	vector<int>::iterator triEnd = triangleID.end();
	vector<int>::iterator ptBegin;
	vector<int>::iterator ptEnd;
	while(ptCurNum<ptTotalNum)
	{
		while(triBegin!=triEnd)
		{
			int curTriID = *triBegin;
/*			// 将当前triangle 顶点存入容器
			for (int m=0; m<3; m++)
			{
				int iPtID = mid.triList[curTriID*3+m];
				ptBegin = cornerID.begin();
				ptEnd = cornerID.end();
				bPtExist = false;

				while(ptBegin<=ptEnd)
				{
					if(iPtID==*ptBegin)
					{
						bPtExist = true;
						break;
					}
					ptBegin++;
				}
				if(!bPtExist)
					cornerID.push_back(iPtID);
			}
*/			// 获取其相邻三角形ID和顶点ID
			for (int i=0; i<3; i++)
			{
				int iNghTriID = mid.neighborList[curTriID*3+i];
				// 检查ID是否存在
				if(iNghTriID==-1)
					continue;
				vector<int>::iterator itTemp = triangleID.begin();
				bTriExist = false;
				for(int j=0; j<(int)(triangleID.size()); j++, itTemp++)
				{
					if(iNghTriID == *itTemp)
					{
						bTriExist = true;
						break;
					}
				}
				if (!bTriExist)	// 如果不存在就保存此邻接三角形和其顶点 [ZuoW,2010/3/24]
				{
					triangleID.push_back(iNghTriID);
					// 将临近三角形的顶点存入容器中
					for (int k=0; k<3; k++)
					{
						int iPtID = mid.triList[iNghTriID*3+k];
						ptBegin = cornerID.begin();
						ptEnd = cornerID.end();
						bPtExist = false;
						while(ptBegin<=ptEnd)
						{
							if(iPtID==*ptBegin)
							{
								bPtExist = true;
								break;
							}
							ptBegin++;
						}
						if(!bPtExist)
							cornerID.push_back(iPtID);
					}
				}
			}

			triBegin++;
		}

		ptCurNum = cornerID.size();
		triBegin = triangleID.begin();
		triEnd   = triangleID.end();
	}
/*	while(level<nPolytransType)
	{
		while(itBegin<=itEnd)
		{
			int curTriID = *itBegin;
			// 将当前三角形的顶点存入容器中
			for (int i=0; i<3; i++)
			{
				int iPtID = mid.triList[curTriID*3+i];
				ptBegin = cornerID.begin();
				ptEnd = cornerID.end();
				// 将第一个顶点存入容器中 [ZuoW,2010/3/24]
				if(ptBegin==ptEnd)
				{
					cornerID.push_back(iPtID);
					continue;
				}
				bExist = false;
				while(ptBegin<=ptEnd)
				{
					if(iPtID==*ptBegin)
					{
						bExist = true;
						break;
					}
					ptBegin++;
				}
				if(!bExist)
					cornerID.push_back(iPtID);
			}

			// 获取其相邻三角形ID
			for (int i=0; i<3; i++)
			{
				int iNghTriID = mid.neighborList[curTriID*3+i];
				// 检查ID是否存在
				if(iNghTriID==-1)
					continue;
				// 判断是否已存在于容器中
				vector<int>::iterator itTemp = itBegin;
				bExist = false;
				for(int j=0; j<triangleID.size(); j++, itTemp++)
				{
					if(iNghTriID == *itTemp)
					{
						bExist = true;
						break;
					}
				}
				if (!bExist)
				{
					triangleID.push_back(iNghTriID);
					// 将临近三角形的顶点存入容器中
					int curTriID = iNghTriID;
					// 将三角形的顶点存入容器中
					for (int i=0; i<3; i++)
					{
						int iPtID = mid.triList[curTriID*3+i];
						ptBegin = cornerID.begin();
						ptEnd = cornerID.end();
						bExist = false;
						while(ptBegin<=ptEnd)
						{
							if(iPtID==*ptBegin)
							{
								bExist = true;
								break;
							}
							ptBegin++;
						}
						if(!bExist)
							cornerID.push_back(iPtID);
					}
				}
			}

			itBegin++;
		}
		itEnd = triangleID.end();	//  指向新的终点

		level++;
	}
*/
	/*
	 * 取出所有顶点坐标。 ［ZuoW,2010/3/9]
	 */
	ptNum = cornerID.size();
	if (ptNum<3)
	{
		printf("Triangle corners is not enough and return!\n");
		return;
	}
	dxSrcTriangle = new double[ptNum];		dySrcTriangle = new double[ptNum];
	dxDstTriangle = new double[ptNum];		dyDstTriangle = new double[ptNum];	dzDstTriangle = new double[ptNum];
	for(int num=0; num<ptNum; num++)
	{
		dxSrcTriangle[num] = dySrcTriangle[num] = dxDstTriangle[num] = dyDstTriangle[num] = dzDstTriangle[num] = -99999.0;
	}

	ptBegin = cornerID.begin();
	ptEnd = cornerID.end();
	for(int num=0;num<ptNum;num++)
	{
		int pointID = cornerID.at(num);
		dxDstTriangle[num] = mid.pointList[pointID*2+0];
		dyDstTriangle[num] = mid.pointList[pointID*2+1];
		dzDstTriangle[num] = pDstImgZ[pointID];
		dxSrcTriangle[num] = pSrcImgX[pointID];
		dySrcTriangle[num] = pSrcImgY[pointID];
	}
}

BOOL CGeometryRectify::CalcPolyTransPara(double *pSrcX, double *pSrcY,
		double *pDstX, double *pDstY,
		int nPtNum,
		UINT nPolytransType,
		double *pError)
{
	m_transType = (PolytransType)nPolytransType;
	if(m_transType == PT_LINEAR)
	{
		int j;
		double a[3], ab[3], aa[9], b;
		memset(aa,0,sizeof(double)*9);
		memset(ab,0,sizeof(double)*3);

		for(j=0; j<nPtNum; j++)
		{
			*(a+0) = 1;
			*(a+1) = pSrcX[j];
			*(a+2) = pSrcY[j];

			b = pDstX[j];
			dnrml(a,3,b,aa,ab);
		}

		dsolve(aa, ab, m_pCoeX,3,3);

		memset(aa,0,sizeof(double)*9);
		memset(ab,0,sizeof(double)*3);

		for(j=0;j<nPtNum;j++)
		{
			*(a+0) = 1;
			*(a+1) = pSrcX[j];
			*(a+2) = pSrcY[j];

			b = pDstY[j];
			dnrml(a,3,b,aa,ab);
		}

		dsolve(aa,ab, m_pCoeY,3,3);
	}
	else if(m_transType == PT_NONLINEAR_XY)
	{
		int j;
		double a[4], ab[4], aa[16], b;

		memset(aa,0,sizeof(double)*16);
		memset(ab,0,sizeof(double)*4);

		for(j=0; j<nPtNum; j++)
		{
			*(a+0) = 1;
			*(a+1) = pSrcX[j];
			*(a+2) = pSrcY[j];
			*(a+3) = pSrcX[j] * pSrcY[j];

			b = pDstX[j];
			dnrml(a,4,b,aa,ab);
		}

		dsolve(aa,ab, m_pCoeX,4,4);

		memset(aa,0,sizeof(double)*16);
		memset(ab,0,sizeof(double)*4);

		for(j=0;j<nPtNum;j++)
		{
			*(a+0) = 1;
			*(a+1) = pSrcX[j];
			*(a+2) = pSrcY[j];
			*(a+3) = pSrcX[j] * pSrcY[j];

			b = pDstY[j];
			dnrml(a,4,b,aa,ab);
		}

		dsolve(aa,ab,m_pCoeY,4,4);
	}
	else if(m_transType == PT_SQUARE)
	{
		int j;
		double a[6], ab[6], aa[36], b;

		memset(aa,0,sizeof(double)*36);
		memset(ab,0,sizeof(double)*6);

		for(j=0; j<nPtNum; j++)
		{
			*(a+0) = 1;
			*(a+1) = pSrcX[j];
			*(a+2) = pSrcY[j];
			*(a+3) = pSrcX[j] * pSrcX[j];
			*(a+4) = pSrcY[j] * pSrcY[j];
			*(a+5) = pSrcX[j] * pSrcY[j];

			b = pDstX[j];
			dnrml(a,6,b,aa,ab);
		}

		dsolve(aa,ab, m_pCoeX,6,6);

		memset(aa,0,sizeof(double)*36);
		memset(ab,0,sizeof(double)*6);

		for(j=0;j<nPtNum;j++)
		{
			*(a+0) = 1;
			*(a+1) = pSrcX[j];
			*(a+2) = pSrcY[j];
			*(a+3) = pSrcX[j] * pSrcX[j];
			*(a+4) = pSrcY[j] * pSrcY[j];
			*(a+5) = pSrcX[j] * pSrcY[j];

			b = pDstY[j];
			dnrml(a,6,b,aa,ab);
		}

		dsolve(aa,ab, m_pCoeY,6,6);
	}
	else if((m_transType == PT_CUBE) || (m_transType == PT_CUBE_Z))
	{
		int j;
		double a[10], ab[10], aa[100], b;

		memset(aa,0,sizeof(double)*100);
		memset(ab,0,sizeof(double)*10);

		for(j=0; j<nPtNum; j++)
		{
			*(a+0) = 1;
			*(a+1) = pSrcX[j];
			*(a+2) = pSrcY[j];
			*(a+3) = pSrcX[j] * pSrcX[j];
			*(a+4) = pSrcY[j] * pSrcY[j];
			*(a+5) = pSrcX[j] * pSrcY[j];
			*(a+6) = pSrcX[j] * pSrcX[j] * pSrcX[j];
			*(a+7) = pSrcY[j] * pSrcY[j] * pSrcY[j];
			*(a+8) = pSrcX[j] * pSrcX[j] * pSrcY[j];
			*(a+9) = pSrcX[j] * pSrcY[j] * pSrcY[j];

			b = pDstX[j];
			dnrml(a,10,b,aa,ab);
		}

		dsolve(aa,ab, m_pCoeX,10,10);

		memset(aa,0,sizeof(double)*100);
		memset(ab,0,sizeof(double)*10);

		for(j=0;j<nPtNum;j++)
		{
			*(a+0) = 1;
			*(a+1) = pSrcX[j];
			*(a+2) = pSrcY[j];
			*(a+3) = pSrcX[j] * pSrcX[j];
			*(a+4) = pSrcY[j] * pSrcY[j];
			*(a+5) = pSrcX[j] * pSrcY[j];
			*(a+6) = pSrcX[j] * pSrcX[j] * pSrcX[j];
			*(a+7) = pSrcY[j] * pSrcY[j] * pSrcY[j];
			*(a+8) = pSrcX[j] * pSrcX[j] * pSrcY[j];
			*(a+9) = pSrcX[j] * pSrcY[j] * pSrcY[j];

			b = pDstY[j];
			dnrml(a,10,b,aa,ab);
		}

		dsolve(aa,ab,m_pCoeY,10,10);
	}

	*pError=0.0;
	double tempX,tempY,ErrorX,ErrorY,RMS;
	double SumErrorX=0.0,SumErrorY=0.0;
	for(int i=0;i<nPtNum;i++)
	{
		PolyTrans(pSrcX[i],pSrcY[i],&tempX,&tempY);
		ErrorX=tempX-pDstX[i];
		ErrorY=tempY-pDstY[i];

		tempX=ErrorX*ErrorX;
		tempY=ErrorY*ErrorY;

		RMS=sqrt(tempX+tempY);

		SumErrorX=SumErrorX+tempX;
		SumErrorY=SumErrorY+tempY;
	}
	*pError=sqrt(SumErrorX/nPtNum+SumErrorY/nPtNum);

	return TRUE;
}

BOOL CGeometryRectify::CalcPolyZTransPara(double *pSrcX, double *pSrcY, double *pSrcZ,
		double *pDstX, double *pDstY,
		int nPtNum,
		UINT nPolytransType,
		double *pError)
{
	m_transType = (PolytransType)nPolytransType;
	if(m_transType == PT_CUBE_Z)
	{
		int j;
		double a[15], ab[15], aa[225], b;

		memset(aa,0,sizeof(double)*225);
		memset(ab,0,sizeof(double)*10);
		
		for(j=0; j<nPtNum; j++)
		{
			*(a+0) = 1;
			*(a+1) = pSrcX[j];
			*(a+2) = pSrcY[j];
			*(a+3) = pSrcZ[j];
			*(a+4) = pSrcX[j] * pSrcX[j];
			*(a+5) = pSrcY[j] * pSrcY[j];
			*(a+6) = pSrcZ[j] * pSrcZ[j];
			*(a+7) = pSrcX[j] * pSrcY[j];
			*(a+8) = pSrcX[j] * pSrcZ[j];
			*(a+9) = pSrcY[j] * pSrcZ[j];
			*(a+10) = pSrcX[j] * pSrcX[j] * pSrcX[j];
			*(a+11) = pSrcY[j] * pSrcY[j] * pSrcY[j] ;
			*(a+12) = pSrcX[j] * pSrcX[j] * pSrcY[j];
			*(a+13) = pSrcX[j] * pSrcY[j] * pSrcY[j];
			*(a+14) = pSrcX[j] * pSrcY[j] * pSrcZ[j];
			
			b = pDstX[j];
			dnrml(a,15,b,aa,ab);
		}

		dsolve(aa,ab, m_pCoeX,15,15);

		memset(aa,0,sizeof(double)*225);
		memset(ab,0,sizeof(double)*15);

		for(j=0;j<nPtNum;j++)
		{
			*(a+0) = 1;
			*(a+1) = pSrcX[j];
			*(a+2) = pSrcY[j];
			*(a+3) = pSrcZ[j];
			*(a+4) = pSrcX[j] * pSrcX[j];
			*(a+5) = pSrcY[j] * pSrcY[j];
			*(a+6) = pSrcZ[j] * pSrcZ[j];
			*(a+7) = pSrcX[j] * pSrcY[j];
			*(a+8) = pSrcX[j] * pSrcZ[j];
			*(a+9) = pSrcY[j] * pSrcZ[j];
			*(a+10) = pSrcX[j] * pSrcX[j] * pSrcX[j];
			*(a+11) = pSrcY[j] * pSrcY[j] * pSrcY[j] ;
			*(a+12) = pSrcX[j] * pSrcX[j] * pSrcY[j];
			*(a+13) = pSrcX[j] * pSrcY[j] * pSrcY[j];
			*(a+14) = pSrcX[j] * pSrcY[j] * pSrcZ[j];

			b = pDstY[j];
			dnrml(a,15,b,aa,ab);
		}

		dsolve(aa,ab,m_pCoeY,15,15);
	}

	*pError=0.0;
	double tempX,tempY,ErrorX,ErrorY,RMS;
	double SumErrorX=0.0,SumErrorY=0.0;
	for(int i=0;i<nPtNum;i++)
	{
		PolyTransZ(pSrcX[i],pSrcY[i],pSrcZ[i], &tempX,&tempY);
		ErrorX=tempX-pDstX[i];
		ErrorY=tempY-pDstY[i];

		tempX=ErrorX*ErrorX;
		tempY=ErrorY*ErrorY;

		RMS=sqrt(tempX+tempY);

		SumErrorX=SumErrorX+tempX;
		SumErrorY=SumErrorY+tempY;
	}
	*pError=sqrt(SumErrorX/nPtNum+SumErrorY/nPtNum);

	return TRUE;
}

int CGeometryRectify::dnrml(double* aa, int n, double bb, double* a, double* b) {
	register int i, j;

	for (i=0; i<n; i++) {
		for (j=0; j<n-i; j++) {
			*a += *aa * *(aa+j);
			a++;
		}
		*b += *aa * bb;
		b++;
		aa++;
	}
	return 0;
}

void CGeometryRectify::dsolve(double* a, double* b, double* x, int n, int wide) 
{
	int m;
	double *d, *l;

	m= n*(n+1)/2;
	d=(double *) malloc(n * sizeof(double));
	l=(double *) malloc((m-n) * sizeof(double));

	memset(d, 0, sizeof(double)*n);
	memset(l, 0, sizeof(double)*(m-n));

	dldltban1(a, d, l, n, wide);

	dldltban2(l, d, b, x, n, wide);
	free(d);
	free(l);
}

void CGeometryRectify::dldltban1(double* a, double* d, double* l, int n, int wide) 
{
	int i, j, k, kk, km, m;
	double *ao, *aa, *co, *c;

	m = wide*(2*n+1-wide)/2;
	c =(double *)calloc((m-wide), sizeof(double));

	ao=a;
	co=c;
	a +=wide;
	for (i=0; i<m-wide; i++)
		*c++ = *a++;
	c=co;
	a=ao;

	for (k=1; k<n; k++) 
	{
		if (k<n-wide+2)
			kk=wide-1;
		else
			kk--;

		*d = *a++;
		aa=a;
		a += kk;

		if (k<n-wide+1)
			km=wide;
		else
			km=n-k+1;

		for (i=1; i<kk+1; i++) 
		{
			*l = *aa++ / *d;
			for (j=0; j<kk-i+1; j++)
				*(a+j) -= *l * *(aa+j-1);
			l++;

			if (k+i>n-wide+1)
				km--;
			a += km;
		}

		a=aa;
		d++;
		if (k==n-1)
			*d = *a;
	}

	a=ao;
	a +=wide;
	for (i=0; i<m-wide; i++)
		*a++ = *c++;
	c=co;
	free(c);
}

void CGeometryRectify::dldltban2(double* l, double* d, double* b, double* x,
		int n, int wide) 
{
	int i, j, kk, m;
	double *bo, *lo, *xx;
	double *bb, *bbo;

	bb =(double*)calloc(n, sizeof(double));
	bbo=bb;

	bo=b;
	lo=l;

	for (i=0; i<n; i++) {
		*(bb++) = *(b++);
	}
	b=bo;
	bb=bbo;
	m = wide*(2*n+1-wide)/2;

	for (i=1; i<n; i++) {
		if (i<n-wide+2)
			kk=wide;
		else
			kk--;

		b=bo+i;
		for (j=1; j<kk; j++) {
			*b -= *(b-j) * *l;
			b++;
			l++;
		}
	}

	kk=0;
	b=bo+n-1;
	l=lo+m-n-1;
	x += n-1;
	xx=x;
	d += n-1;

	*x-- = (*b--) / (*d--);

	for (i=1; i<n; i++) {
		if (i<wide)
			kk++;
		else {
			kk=wide-1;
			xx--;
		}

		*x = *b-- / *d--;
		for (j=1; j<kk+1; j++)
			*x -= *(l--) * *(xx-j+1);
		x--;
	}

	b=bo;
	for (i=0; i<n; i++)
		*b++ = *bb++;
	bb=bbo;
	free(bb);
}

CFArray<int> *comprasterx=NULL, *comprastery=NULL;
int TriangleCompare(const void *arg1, const void *arg2) {
	int index1=*(int*)arg1, index2=*(int*)arg2;
	CFArray<int>& x=*(CFArray<int>*)comprasterx;
	CFArray<int>& y=*(CFArray<int>*)comprastery;

	if (y[index1]!=y[index2]) {
		return y[index1]-y[index2];
	} else {
		return x[index1]-x[index2];
	}

	return 0;
}

BOOL VectorRaster(int x0,int y0,int x1,int y1,CFArray<int>& rasterx,CFArray<int>& rastery)
{
	if(y0>y1)
	{
		int tempx=x0;
		int tempy=y0;
		x0=x1;
		y0=y1;
		x1=tempx;
		y1=tempy;
	}
	else if(y0==y1)
	{
		if(x0>x1)
		{
			int tempx=x0;
			int tempy=y0;
			x0=x1;
			y0=y1;
			x1=tempx;
			y1=tempy;
		}
	}

	int dx=x1-x0;
	int dy=y1-y0;
	int nIx=dx>0?dx:(0-dx);
	int nIy=dy>0?dy:(0-dy);
	int nPtNum=0;
	if(nIx==0&&nIy==0)
	{
		nPtNum=1;
		rasterx.Add(x0);
		rastery.Add(y0);
		return TRUE;
	}

	nPtNum=nIx>nIy?nIx:nIy;

	int xFlag=-nIy,yFlag=-nIx;
	int xStep=2*nIx;
	int yStep=2*nIy;

	int tempx=x0,tempy=y0;
	rasterx.Add(tempx);
	rastery.Add(tempy);

	for(int j=0;j<nPtNum;j++)
	{
		xFlag+=xStep;
		yFlag+=yStep;
		BOOL bPtOnLine=FALSE;

		if(xFlag>=0)
		{
			bPtOnLine=TRUE;
			xFlag-=yStep;
			if(dx>0)
			{
				tempx++;
			}
			else
			{
				tempx--;
			}
		}

		if(yFlag>=0)
		{
			bPtOnLine=TRUE;
			yFlag-=xStep;

			if(dy>0)
			{
				tempy++;
			}
			else
			{
				tempy--;
			}
		}

		if(bPtOnLine&&rastery[rastery.GetSize()-1]!=tempy)
		{
			rasterx.Add(tempx);
			rastery.Add(tempy);
		}
	}
	if(y0==y1)
	{
		rasterx.Add(x1);
		rastery.Add(y1);
	}

	return TRUE;
}

BOOL VectorRaster(int x0,int y0,double z0,int x1,int y1,double z1,CFArray<int>& rasterx,CFArray<int>& rastery,CFArray<double>& rasterz)
{
	if(y0>y1)
	{
		int tempx=x0;
		int tempy=y0;
		double tempz=z0;
		x0=x1;
		y0=y1;
		z0=z1;
		x1=tempx;
		y1=tempy;
		z1=tempz;
	}
	else if(y0==y1)
	{
		if(x0>x1)
		{
			int tempx=x0;
			int tempy=y0;
			double tempz=z0;
			x0=x1;
			y0=y1;
			z0=z1;
			x1=tempx;
			y1=tempy;
			z1=tempz;
		}
	}

	int dx=x1-x0;
	int dy=y1-y0;
	double dz=0.0;
	if(y1!=y0)
	{
		dz=(z1-z0)/fabs(y1-y0);
	}
	int nIx=dx>0?dx:(0-dx);
	int nIy=dy>0?dy:(0-dy);
	int nPtNum=0;
	if(nIx==0&&nIy==0)
	{
		nPtNum=1;
		rasterx.Add(x0);
		rastery.Add(y0);
		rasterz.Add(z0);
		return TRUE;
	}

	nPtNum=nIx>nIy?nIx:nIy;

	int xFlag=-nIy,yFlag=-nIx;
	int xStep=2*nIx;
	int yStep=2*nIy;

	int tempx=x0,tempy=y0;
	double tempz=z0;
	rasterx.Add(tempx);
	rastery.Add(tempy);
	rasterz.Add(tempz);
	tempz=tempz+dz;

	for(int j=0;j<nPtNum;j++)
	{
		xFlag+=xStep;
		yFlag+=yStep;
		BOOL bPtOnLine=FALSE;

		if(xFlag>=0)
		{
			bPtOnLine=TRUE;
			xFlag-=yStep;
			if(dx>0)
			{
				tempx++;
			}
			else
			{
				tempx--;
			}
		}

		if(yFlag>=0)
		{
			bPtOnLine=TRUE;
			yFlag-=xStep;

			if(dy>0)
			{
				tempy++;
			}
			else
			{
				tempy--;
			}
		}

		if(bPtOnLine&&rastery[rastery.GetSize()-1]!=tempy)
		{
			rasterx.Add(tempx);
			rastery.Add(tempy);
			rasterz.Add(tempz);
			tempz=tempz+dz;
		}
	}
	if(y0==y1)
	{
		rasterx.Add(x1);
		rastery.Add(y1);
		rasterz.Add(z1);
	}

	return TRUE;
}

void CGeometryRectify::TriangleFillinFst(CImage* pSrcImage, CImage* pDstImage,
		double *pSrcTriangleX, double *pSrcTriangleY, double *pDstTriangleX,
		double *pDstTriangleY, UINT nResampleMethod)
{
	if (pSrcImage==NULL||pDstImage==NULL)
	{
		return;
	}

	// 三角形顶点坐标
	int x[3], y[3];
	int i=0;
	for (i=0; i<3; i++)
	{
		x[i]=(int)(pDstTriangleX[i]+0.5);
		y[i]=(int)(pDstTriangleY[i]+0.5);
	}

	// 保存三角形左右起止像素坐标
	CFArray<int> rasterx, rastery;
	for (i=0; i<3; i++)
	{
		VectorRaster(x[i], y[i], x[(i+1)%3], y[(i+1)%3], rasterx, rastery);
	}

	int nPtNum=rasterx.GetSize();
	for (i=0; i<3; i++)
	{
		if ((y[(i-1+3)%3]-y[i])*(y[(i+1)%3]-y[i])<=0)
		{
			for (int j=0; j<nPtNum; j++)
			{
				if (rasterx[j]==x[i]&&rastery[j]==y[i])
				{
					rasterx.RemoveAt(j);
					rastery.RemoveAt(j);

					nPtNum=rasterx.GetSize();
					break;
				}
			}
		}
	}

	nPtNum=rasterx.GetSize();
	int* indextemp=new int[nPtNum];

	for (i=0; i<nPtNum; i++)
	{
		indextemp[i]=i;
	}
	comprasterx=&rasterx;
	comprastery=&rastery;
	qsort((void*)indextemp, nPtNum, sizeof(int), TriangleCompare);
	CFArray<int> index;
	for (i=0; i<nPtNum; i++)
	{
		index.Add(indextemp[i]);
	}
	delete [] indextemp;
	indextemp=NULL;

	nPtNum=index.GetSize();
	CFArray<int> rasterlx;
	CFArray<int> rasterly;
	CFArray<int> rasterrx;
	CFArray<int> rasterry;
	for (i=0; i<nPtNum/2; i++)
	{
		int x0=rasterx[index[i*2+0]];
		int y0=rastery[index[i*2+0]];
		int x1=rasterx[index[i*2+1]];
		int y1=rastery[index[i*2+1]];
		if (x0<x1)
		{
			rasterlx.Add(x0);
			rasterly.Add(y0);
			rasterrx.Add(x1);
			rasterry.Add(y1);
		}
		else
		{
			rasterlx.Add(x1);
			rasterly.Add(y1);
			rasterrx.Add(x0);
			rasterry.Add(y0);
		}
	}

	// 计算转换系数
	double Error=0;
	CalcPolyTransPara(pDstTriangleX, pDstTriangleY, pSrcTriangleX,
			pSrcTriangleY, CONST_NUM, PT_LINEAR, &Error);	// 3 -> 6

	// Print the coef. [ZuoW, 11/12/2009]
	printf("X coef.:%f, %f, %f\n", m_pCoeX[0], m_pCoeX[1], m_pCoeX[2]);
	printf("Y coef.:%f, %f, %f\n", m_pCoeY[0], m_pCoeY[1], m_pCoeY[2]);

	int nBPP;
	pSrcImage->GetBPP(&nBPP);
	BYTE* pPixel=new BYTE[nBPP];
	double xsrc, ysrc, xdst, ydst;
	for (i=0; i<nPtNum/2; i++)
	{
		ydst=rasterly[i];
		for (int j=rasterlx[i]; j<=rasterrx[i]; j++)
		{
			xdst=j;
			PolyTrans(xdst, ydst, &xsrc, &ysrc);
			memset(pPixel, 0, nBPP);
			// 反推法，通过在源影像上重采样实现。 [ZuoW, 2009/11/17]
			pSrcImage->GetPixelF((float)xsrc, (float)ysrc, pPixel,
					nResampleMethod);
			pDstImage->SetPixel(ydst, xdst, pPixel);
		}
	}

	delete [] pPixel;
	return;
}

void CGeometryRectify::TriangleFillinFst(CDEM* pDEM, CImage* pSrcImage, CImage* pDstImage,
		int nPointNum, double *pSrcTriangleX, double *pSrcTriangleY, double *pDstTriangleX,
		double *pDstTriangleY, double* pDstTriangleZ, UINT nPolytransType, UINT nResampleMethod)		// 添加了多项式阶数 [ZuoW,2010/5/18]
{
	int ptNum = nPointNum;		// 传入的GCP个数。 [ZuoW,2010/3/10]
	m_transType = (PolytransType)nPolytransType;

	if (pSrcImage==NULL||pDstImage==NULL) 
	{
		return;
	}

	//������Ӱ�����ת��Ϊ����Ӱ�����
	int x[3], y[3];
	int i=0;
	for (i=0; i<3; i++)
	{
		x[i]=(int)(pDstTriangleX[i]+0.5);
		y[i]=(int)(pDstTriangleY[i]+0.5);
	}

	//�����ɾ��
	CFArray<int> rasterx, rastery;
	for (i=0; i<3; i++)
	{
		VectorRaster(x[i], y[i], x[(i+1)%3], y[(i+1)%3], rasterx, rastery);
	}
	//��ɾ������εı߽���
	int nPtNum=rasterx.GetSize();
	for (i=0; i<3; i++)
	{
		if ((y[(i-1+3)%3]-y[i])*(y[(i+1)%3]-y[i])<=0)
		{
			for (int j=0; j<nPtNum; j++) 
			{
				if (rasterx[j]==x[i]&&rastery[j]==y[i]) 
				{
					rasterx.RemoveAt(j);
					rastery.RemoveAt(j);

					nPtNum=rasterx.GetSize();
					break;
				}
			}
		}
	}

	nPtNum=rasterx.GetSize();
	int* indextemp=new int[nPtNum];

	for (i=0; i<nPtNum; i++) 
	{
		indextemp[i]=i;
	}
	comprasterx=&rasterx;
	comprastery=&rastery;
	qsort((void*)indextemp, nPtNum, sizeof(int), TriangleCompare);
	CFArray<int> index;
	for (i=0; i<nPtNum; i++) 
	{
		index.Add(indextemp[i]);
	}
	delete [] indextemp;
	indextemp=NULL;

	nPtNum=index.GetSize();
	CFArray<int> rasterlx;
	CFArray<int> rasterly;
	CFArray<int> rasterrx;
	CFArray<int> rasterry;
	for (i=0; i<nPtNum/2; i++) 
	{
		int x0=rasterx[index[i*2+0]];
		int y0=rastery[index[i*2+0]];
		int x1=rasterx[index[i*2+1]];
		int y1=rastery[index[i*2+1]];
		if (x0<x1) 
		{
			rasterlx.Add(x0);
			rasterly.Add(y0);
			rasterrx.Add(x1);
			rasterry.Add(y1);
		}
		else 
		{
			rasterlx.Add(x1);
			rasterly.Add(y1);
			rasterrx.Add(x0);
			rasterry.Add(y0);
		}
	}

	// 计算多项式转换系数
	/// 如果根据GCP个数来确定多项式阶数，那么结果影像中畸变不统一，还是使用统一的阶数。 [ZuoW,2010/3/24]
	double Error=0;
	if(ptNum<3)		// 安全性检查 [ZuoW,2010/3/24]
	{
		printf("Invalid GCP count and return!\n");
		return;
	}
	else if(ptNum<(m_transType+1)*(m_transType+2)/2)
	{
		printf("Not enough GCP and reset the transfer type!\n");
		m_transType = PT_LINEAR;
	}
	
	double dstLBX, dstLBY, dstCell;
	if(m_transType == PT_CUBE_Z)	/// 加入高程计算多项式系数(可以考虑将这几个变换类型进行集成) [ZuoW,2010/5/18]
	{
		// 获取目标影像坐标信息 [ZuoW,2010/5/19]
		pDstImage->GetGrdInfo(&dstLBX, &dstLBY, &dstCell);

		// 获取高程 [ZuoW,2010/5/18]
		for(i=0; i<ptNum; i++)
		{
			// 首先将行列号转换为地理坐标 [ZuoW,2010/5/19]
			pDstTriangleX[i] = dstLBX + pDstTriangleX[i]*dstCell;
			pDstTriangleY[i] = dstLBY + pDstTriangleY[i]*dstCell;

			if(fabs(pDstTriangleZ[i]-INVALID_ALTITUDE)<1e-5)
				pDEM->GetAverageAltitude(&(pDstTriangleZ[i]));

			printf("The %dth GCP's coordination:%lf, %lf, %lf, %lf, %lf\n", i+1,
					pDstTriangleX[i], pDstTriangleY[i], pDstTriangleZ[i], pSrcTriangleX[i], pSrcTriangleY[i]);
		}


		CalcPolyZTransPara(pDstTriangleX, pDstTriangleY, pDstTriangleZ,
				pSrcTriangleX, pSrcTriangleY, ptNum, nPolytransType, &Error);

	}
	else
		CalcPolyTransPara(pDstTriangleX, pDstTriangleY, pSrcTriangleX,
			pSrcTriangleY, ptNum, m_transType, &Error);

	// Print the coef. [ZuoW, 2009/12/11]
/*	FILE* fp = fopen("/dps/workdir/CCD2/LEVEL3/376/Coef.txt", "a+");
	if(fp!=NULL)
	{
		for(int num=0; num<ptNum; num++)
		{
			fprintf(fp, "%dth point:%lf, %lf, %lf, %lf\n", num+1,
					pSrcTriangleX[num], pSrcTriangleY[num], pDstTriangleX[num], pDstTriangleY[num]);
		}
		fprintf(fp, "The coef. are:\n");
		for(int num=0; num<15; num++)
		{
			fprintf(fp, "%8.5e, %8.5e\n", m_pCoeX[num], m_pCoeY[num]);
		}
		fprintf(fp, "\n");
		fprintf(fp, "\n");
		fprintf(fp, "\n");
		fclose(fp);
	}
*/
	printf("X coef.:%f, %f, %f\n", m_pCoeX[0], m_pCoeX[1], m_pCoeX[2]);
	printf("Y coef.:%f, %f, %f\n", m_pCoeY[0], m_pCoeY[1], m_pCoeY[2]);

	//��������������
	int nBPP;
	pSrcImage->GetBPP(&nBPP);
	BYTE* pPixel=new BYTE[nBPP];
	double xsrc, ysrc, xdst, ydst;
	for (i=0; i<nPtNum/2; i++) 
	{
		ydst=rasterly[i];
		for (int j=rasterlx[i]; j<=rasterrx[i]; j++) 
		{
			xdst = j;
			if(m_transType == PT_CUBE_Z)
			{
				double X, Y, Z;
				X = dstLBX + dstCell*xdst;
				Y = dstLBY + dstCell*ydst;

				pDEM->GetAltitude(X, Y, &Z, 0);
				if(fabs(Z-INVALID_ALTITUDE)<1e-5)
					pDEM->GetAverageAltitude(&Z);

				PolyTransZ(X, Y, Z, &xsrc, &ysrc);
			}
			else
			{
				PolyTrans(xdst, ydst, &xsrc, &ysrc);
			}
			memset(pPixel, 0, nBPP);
			// NearestNeighbour in the triangle and Bilinear on the borderline. [ZuoW, 2009/11/17]
			pSrcImage->GetPixelF((float)xsrc, (float)ysrc, pPixel,
					nResampleMethod);
			pDstImage->SetPixel(ydst, xdst, pPixel);
		}
	}

	delete [] pPixel;	pPixel = NULL;
	return;
}

void CGeometryRectify::RectangleFillin(CImage *pSrcImage, CImage *pDstImage,
		double *pSrcRectangleX, double *pSrcRectangleY, double *pDstRectangleX,
		double *pDstRectangleY, UINT nResampleMethod) {
	if (pSrcImage==NULL||pDstImage==NULL) {
		return;
	}
	int nSrcRows, nSrcCols;
	pSrcImage->GetRows(&nSrcRows);
	pSrcImage->GetCols(&nSrcCols);
	int nBandNum;
	pSrcImage->GetBandNum(&nBandNum);
	int nBPP, nBPB;
	pSrcImage->GetBPP(&nBPP);
	pSrcImage->GetBPB(&nBPB);
	int nDstRows, nDstCols;
	pDstImage->GetRows(&nDstRows);
	pDstImage->GetCols(&nDstCols);
	UINT nDataType;
	pSrcImage->GetDataType(&nDataType);

	//������Ӱ�����ת��Ϊ����Ӱ�����
	int x[4], y[4];
	int i=0;
	for (i=0; i<4; i++) {
		x[i]=(int)(pDstRectangleX[i]+0.5);
		y[i]=(int)(pDstRectangleY[i]+0.5);
	}
	//����Ŀ����ε�Դ���εı任����ʽ
	if (pSrcRectangleX!=NULL||pSrcRectangleY!=NULL) {
		double Error=0;
		CalcPolyTransPara(pDstRectangleX, pDstRectangleY, pSrcRectangleX,
				pSrcRectangleY, 4, PT_NONLINEAR_XY, &Error);
	}

	//BYTE* pPixel=new BYTE[nBPP];
	int nTileRows, nTileCols;
	int nTileRow0=__max(0,y[0]);
	int nTileCol0=__max(0,x[0]);
	int nTileRow1=__min(nDstRows,y[2]);
	int nTileCol1=__min(nDstCols,x[2]);
	nTileRows=nTileRow1-nTileRow0;
	nTileCols=nTileCol1-nTileCol0;
	BYTE* pBuffer=new BYTE[nTileRows*nTileCols*nBPP];
	double xsrc, ysrc, xdst, ydst;
	//������������
	double xmin, ymin, xmax, ymax;
	PolyTrans(nTileCol0, nTileRow0, &xsrc, &ysrc);
	xmin=xsrc;
	ymin=ysrc;
	xmax=xsrc;
	ymax=ysrc;
	PolyTrans(nTileCol1, nTileRow0, &xsrc, &ysrc);
	if (xsrc<xmin) {
		xmin=xsrc;
	}
	if (ysrc<ymin) {
		ymin=ysrc;
	}
	if (xsrc>xmax) {
		xmax=xsrc;
	}
	if (ysrc>ymax) {
		ymax=ysrc;
	}
	PolyTrans(nTileCol1, nTileRow1, &xsrc, &ysrc);
	if (xsrc<xmin) {
		xmin=xsrc;
	}
	if (ysrc<ymin) {
		ymin=ysrc;
	}
	if (xsrc>xmax) {
		xmax=xsrc;
	}
	if (ysrc>ymax) {
		ymax=ysrc;
	}
	PolyTrans(nTileCol0, nTileRow1, &xsrc, &ysrc);
	if (xsrc<xmin) {
		xmin=xsrc;
	}
	if (ysrc<ymin) {
		ymin=ysrc;
	}
	if (xsrc>xmax) {
		xmax=xsrc;
	}
	if (ysrc>ymax) {
		ymax=ysrc;
	}
	int ix0=(int)xmin-10;
	int iy0=(int)ymin-10;
	int ix1=(int)xmax+10;
	int iy1=(int)ymax+10;

	int srcRow=iy1-iy0;
	int srcCol=ix1-ix0;
	BYTE* pSrcBuffer=new BYTE[srcRow*srcCol*nBPP];
	memset(pSrcBuffer, 0, srcRow*srcCol*nBPP);
	pSrcImage->ReadImg((float)ix0, (float)iy0, (float)ix1, (float)iy1,
			pSrcBuffer, srcCol, srcRow, nBandNum, 0, 0, srcCol, srcRow, -1, 0);

	for (i=nTileRow0; i<nTileRow1; i++) {
		int nRowIndex=i-nTileRow0;

		BYTE* pBufferIndex=pBuffer+nRowIndex*nTileCols*nBPP;
		ydst=i;
		for (int j=nTileCol0; j<nTileCol1; j++) {
			int nColIndex=j-nTileCol0;

			xdst=j;
			PolyTrans(xdst, ydst, &xsrc, &ysrc);
			if ( (xsrc<0) || (xsrc>=nSrcCols) || (ysrc<0) || (ysrc>=nSrcRows)) {
				memset(&pBufferIndex[nColIndex*nBPP], 0, nBPP);
			} else {
				int x1=(int)xsrc;
				int y1=(int)ysrc;
				int x2=x1+1;
				int y2=y1+1;
				if (x2>=nSrcCols) {
					x2=x1;
				}
				if (y2>=nSrcRows) {
					y2=y1;
				}
				double dx=xsrc-x1;
				double dy=ysrc-y1;

				BYTE* pPixel=pBufferIndex+nColIndex*nBPP;
				BYTE* pBuf11=pSrcBuffer+(y1-iy0)*srcCol*nBPP+(x1-ix0)*nBPP;
				BYTE* pBuf21=pBuf11+(x2-x1)*nBPP;
				BYTE* pBuf12=pBuf11+(y2-y1)*srcCol*nBPP;
				BYTE* pBuf22=pBuf12+(x2-x1)*nBPP;

				switch (nResampleMethod) {
				case 1: {
					for (int band=0; band<nBandNum; band++) {
						(*FuncTable[nDataType])(pBuf11 + nBPB *band, pBuf21
								+ nBPB *band, pBuf22 + nBPB *band, pBuf12
								+ nBPB *band, dx, dy, pPixel + nBPB *band);
					}
				}
					break;
				case 0: {
					memcpy(pPixel, pSrcBuffer+((int)(ysrc+0.5)-iy0)*srcCol*nBPP
							+((int)(xsrc+0.5)-ix0)*nBPP, nBPP);
				}
					break;
				}
			}
		}
	}

	pDstImage->WriteImg((float)nTileCol0, (float)nTileRow0, (float)nTileCol1,
			(float)nTileRow1, pBuffer, nTileCols, nTileRows, nBandNum, 0, 0,
			nTileCols, nTileRows, -1, 0);

	delete [] pSrcBuffer;
	pSrcBuffer=NULL;

	//delete [] pPixel;
	delete [] pBuffer;

	/*double xSrcTriangle[3],ySrcTriangle[3],xDstTriangle[3],yDstTriangle[3];
	 //��}������ν������
	 xSrcTriangle[0]=pSrcRectangleX[0]; ySrcTriangle[0]=pSrcRectangleY[0];
	 xSrcTriangle[1]=pSrcRectangleX[1]; ySrcTriangle[1]=pSrcRectangleY[1];
	 xSrcTriangle[2]=pSrcRectangleX[2]; ySrcTriangle[2]=pSrcRectangleY[2];
	 xDstTriangle[0]=pDstRectangleX[0]; yDstTriangle[0]=pDstRectangleY[0];
	 xDstTriangle[1]=pDstRectangleX[1]; yDstTriangle[1]=pDstRectangleY[1];
	 xDstTriangle[2]=pDstRectangleX[2]; yDstTriangle[2]=pDstRectangleY[2];
	 TriangleFillinFst(pSrcImage,pDstImage,xSrcTriangle,ySrcTriangle,xDstTriangle,yDstTriangle,nResampleMethod);

	 xSrcTriangle[0]=pSrcRectangleX[0]; ySrcTriangle[0]=pSrcRectangleY[0];
	 xSrcTriangle[1]=pSrcRectangleX[2]; ySrcTriangle[1]=pSrcRectangleY[2];
	 xSrcTriangle[2]=pSrcRectangleX[3]; ySrcTriangle[2]=pSrcRectangleY[3];
	 xDstTriangle[0]=pDstRectangleX[0]; yDstTriangle[0]=pDstRectangleY[0];
	 xDstTriangle[1]=pDstRectangleX[2]; yDstTriangle[1]=pDstRectangleY[2];
	 xDstTriangle[2]=pDstRectangleX[3]; yDstTriangle[2]=pDstRectangleY[3];
	 TriangleFillinFst(pSrcImage,pDstImage,xSrcTriangle,ySrcTriangle,xDstTriangle,yDstTriangle,nResampleMethod);

	 return ;*/
}

HRESULT CGeometryRectify::OrthoRectifyRPC(char* pszSrcImage,
		char* pszSrcRPC,
		char* pszSrcDEM,
		double* pGCPX,double* pGCPY,double* pGCPZ,
		double* pGCPIX,double* pGCPIY,
		int nGCPNum,
		char* pszDstImage,
		double outputLBX,double outputLBY,
		double outputRTX,double outputRTY,
		double OutputGSD,
		UINT nResampleMethod,
		CProjection* pProjection)
{
	//��ʼ��RPC����
	CRPCParm RPC;
	HRESULT hRes=RPC.Initialize(pszSrcRPC);
	if(hRes==S_FALSE)
	{
		return S_FALSE;
	}
	RPC.SetDEMProjection(pProjection);

	CDEM* pDEM=new CNSDTFDEM;
	hRes=pDEM->Open(pszSrcDEM,0,modeRead);
	if(hRes==S_FALSE)
	{
		COutput::OutputFileOpenFailed(pszSrcDEM);
		return S_FALSE;
	}

	//�����Ƶ�ͶӰ��WGS84���ϵ
	double* pGCPGX=new double[nGCPNum];
	double* pGCPGY=new double[nGCPNum];
	double* pGCPGZ=new double[nGCPNum];
	memcpy(pGCPGX,pGCPX,sizeof(double)*nGCPNum);
	memcpy(pGCPGY,pGCPY,sizeof(double)*nGCPNum);
	memcpy(pGCPGZ,pGCPZ,sizeof(double)*nGCPNum);

	CProjection* pWGS84Projection=new CProjectWGS84;
	if(pProjection)
	{
		pWGS84Projection->CreateProject("WGS84");

		double lfLatitude,lfLongitude;
		RPC.GetLAT_OFF(&lfLatitude);
		RPC.GetLONG_OFF(&lfLongitude);
		//pProjection->GetCentralMeridian(&lfLongitude);
		long Zone;
		pWGS84Projection->CalcZone(lfLatitude*PI/180.0,lfLongitude*PI/180.0,&Zone);

		pWGS84Projection->SetZoneStep(6);
		pWGS84Projection->SetZone(Zone);
		pWGS84Projection->SetHemisphere(lfLatitude>0?'N':'S');

		for(int i=0;i<nGCPNum;i++)
		{
			Convert2WGS84(pProjection,pWGS84Projection,pGCPX[i],pGCPY[i],pGCPZ[i],&pGCPGX[i],&pGCPGY[i],&pGCPGZ[i]);
		}
	}

	//��ȡԭʼӰ�����
	CImage srcImage;
	if(srcImage.Open(pszSrcImage,modeRead)!=S_OK)
	{
		COutput::OutputFileOpenFailed(pszSrcImage);

		pDEM->Close();
		delete pDEM;
		pDEM=NULL;

		return S_FALSE;
	}

	int nSrcRows,nSrcCols;
	srcImage.GetRows(&nSrcRows);
	srcImage.GetCols(&nSrcCols);
	int nBandNum;
	srcImage.GetBandNum(&nBandNum);
	UINT datatype;
	srcImage.GetDataType(&datatype);
	int nBPB;
	srcImage.GetBPB(&nBPB);
	int nBPP=nBPB*nBandNum;
	//��Ӱ�����ת��ΪRPC�е�Ӱ�����ϵ
	int i=0;
	for(i=0;i<nGCPNum;i++)
	{
		pGCPIY[i]=nSrcRows-1-pGCPIY[i];
	}
	//�����񷽸���ϵ�����
	if(nGCPNum>0)
	{
		RPC.CalculateImageAdjustMatrix(pGCPGX,pGCPGY,pGCPGZ,pGCPIX,pGCPIY,nGCPNum);
	}
	//�����﷽����ϵ�����
	if(nGCPNum>0)
	{
		RPC.CalculateGroundAdjustMatrix(pDEM,pGCPGX,pGCPGY,pGCPGZ,pGCPIX,pGCPIY,nGCPNum);
	}
	//�ָ�����ͨӰ�����ϵ
	for(i=0;i<nGCPNum;i++)
	{
		pGCPIY[i]=nSrcRows-1-pGCPIY[i];
	}

	//����Ŀ�귶Χ
	double LBX,LBY,RTX,RTY;
	if(outputRTX-outputLBX<1e-5||outputRTY-outputLBY<1e-5)
	{
		//��ȡDEM��Χ
		double demLBX,demLBY,demRTX,demRTY;
		pDEM->GetRange(&demLBX,&demLBY,&demRTX,&demRTY);

		LBX=demLBX;
		LBY=demLBY;
		RTX=demRTX;
		RTY=demRTY;
		//����Ŀ��Ӱ�����
		double imgLBX,imgLBY,imgRTX,imgRTY;
		ProjectImageRPC(pszSrcImage,
				pszSrcRPC,
				pDEM,
				pGCPX,pGCPY,pGCPZ,
				pGCPIX,pGCPIY,
				nGCPNum,
				&imgLBX,&imgLBY,
				&imgRTX,&imgRTY,
				pProjection);
		//����DEM��Ӱ����ص�����
		LBX=__max(LBX,imgLBX);
		LBY=__max(LBY,imgLBY);
		RTX=__min(RTX,imgRTX);
		RTY=__min(RTY,imgRTY);
	}
	else
	{
		LBX=outputLBX;
		LBY=outputLBY;
		RTX=outputRTX;
		RTY=outputRTY;
	}

	int nDstRows=int((RTY-LBY)/OutputGSD+0.5);
	int nDstCols=int((RTX-LBX)/OutputGSD+0.5);
	//����Ŀ��Ӱ��
	CImage dstImage;

	if(dstImage.CreateImg(pszDstImage,modeCreate|modeWrite,
					nDstCols,nDstRows,datatype,
					nBandNum,BSQ,
					LBX,LBY,OutputGSD)!=S_OK)
	{
		COutput::OutputFileOpenFailed(pszDstImage);

		srcImage.Close();

		pDEM->Close();
		delete pDEM;
		pDEM=NULL;

		return S_FALSE;
	}
	//������ɫ����Ϣ
	HRESULT hColorTable=srcImage.HaveColorTable();
	if(hColorTable==S_OK)
	{
		int nEntryNum=0;
		srcImage.GetEntryNum(&nEntryNum);
		RGBQUAD* pColorTable=new RGBQUAD[nEntryNum];
		srcImage.GetColorTable((BYTE*)pColorTable);
		dstImage.SetColorTable((BYTE*)pColorTable,nEntryNum);
		delete [] pColorTable;
		pColorTable=NULL;
	}

	//������������
	CProgress progress;
	progress.AddCurInfo("�������");

	HRESULT hRet=S_FALSE;
	UINT nDEMType=0;
	pDEM->GetType(&nDEMType);
	switch(nDEMType)
	{
		case DT_GRID:
		{
			//��ȡDEM���
			int nDEMRows,nDEMCols;
			pDEM->GetRows(&nDEMRows);
			pDEM->GetCols(&nDEMCols);
			double X0,Y0;
			pDEM->GetStartPos(&X0,&Y0);
			double XCellSize,YCellSize;
			pDEM->GetCellSize(&XCellSize,&YCellSize);
			int nDemRow0=(int)((LBY-Y0)/YCellSize);
			int nDemCol0=(int)((LBX-X0)/XCellSize);
			int nDemRow2=__min((int)((RTY-Y0)/YCellSize+1.5),nDEMRows);
			int nDemCol2=__min((int)((RTX-X0)/XCellSize+1.5),nDEMCols);
			int nDemRatio=(int)(OutputGSD*5.0/XCellSize+1);
			while((nDemRow2-nDemRow0<2*nDemRatio||nDemCol2-nDemCol0<2*nDemRatio)&&nDemRatio>2)
			{
				nDemRatio/=2;
			}
			//			double* pZ=new double[nDEMRows*nDEMCols];
			//			pDEM->GetAltitudeValues(pZ);
			//��������Ӱ��
			progress.SetRange(0,(nDemRow2-nDemRow0)/nDemRatio*(nDemCol2-nDemCol0)/nDemRatio+1);
			progress.SetPosition(0);

			double xSrcRectangle[4],ySrcRectangle[4];
			double xDstRectangle[4],yDstRectangle[4];
			for(int i=nDemRow0;i<nDemRow2-1;i+=nDemRatio)
			{
				for(int j=nDemCol0;j<nDemCol2-1;j+=nDemRatio)
				{
					progress.StepIt();

					double X,Y,Z;
					//left bottom
					X=(j+0*nDemRatio)*XCellSize+X0;
					Y=(i+0*nDemRatio)*YCellSize+Y0;
					//Z=0;
					pDEM->GetAltitude(X,Y,&Z,0);
					if(fabs(Z-INVALID_ALTITUDE)<1e-5)
					{
						continue;
					}
					xDstRectangle[0]=(X-LBX)/OutputGSD;
					yDstRectangle[0]=(Y-LBY)/OutputGSD;
					if(pProjection)
					{
						double X84,Y84,Z84;
						Convert2WGS84(pProjection,pWGS84Projection,X,Y,Z,&X84,&Y84,&Z84);
						X=X84;
						Y=Y84;
						Z=Z84;
					}
					RPC.Ground2Image3(X,Y,Z,&xSrcRectangle[0],&ySrcRectangle[0]);
					ySrcRectangle[0]=nSrcRows-1-ySrcRectangle[0];
					//right bottom
					X=(j+1*nDemRatio)*XCellSize+X0;
					Y=(i+0*nDemRatio)*YCellSize+Y0;
					//Z=0;
					pDEM->GetAltitude(X,Y,&Z,0);
					if(fabs(Z-INVALID_ALTITUDE)<1e-5)
					{
						continue;
					}
					xDstRectangle[1]=(X-LBX)/OutputGSD;
					yDstRectangle[1]=(Y-LBY)/OutputGSD;
					if(pProjection)
					{
						double X84,Y84,Z84;
						Convert2WGS84(pProjection,pWGS84Projection,X,Y,Z,&X84,&Y84,&Z84);
						X=X84;
						Y=Y84;
						Z=Z84;
					}
					RPC.Ground2Image3(X,Y,Z,&xSrcRectangle[1],&ySrcRectangle[1]);
					ySrcRectangle[1]=nSrcRows-1-ySrcRectangle[1];
					//right top
					X=(j+1*nDemRatio)*XCellSize+X0;
					Y=(i+1*nDemRatio)*YCellSize+Y0;
					//Z=0;
					pDEM->GetAltitude(X,Y,&Z,0);
					if(fabs(Z-INVALID_ALTITUDE)<1e-5)
					{
						continue;
					}
					xDstRectangle[2]=(X-LBX)/OutputGSD;
					yDstRectangle[2]=(Y-LBY)/OutputGSD;
					if(pProjection)
					{
						double X84,Y84,Z84;
						Convert2WGS84(pProjection,pWGS84Projection,X,Y,Z,&X84,&Y84,&Z84);
						X=X84;
						Y=Y84;
						Z=Z84;
					}
					RPC.Ground2Image3(X,Y,Z,&xSrcRectangle[2],&ySrcRectangle[2]);
					ySrcRectangle[2]=nSrcRows-1-ySrcRectangle[2];
					//left top
					X=(j+0*nDemRatio)*XCellSize+X0;
					Y=(i+1*nDemRatio)*YCellSize+Y0;
					//Z=0;
					pDEM->GetAltitude(X,Y,&Z,0);
					if(fabs(Z-INVALID_ALTITUDE)<1e-5)
					{
						continue;
					}
					xDstRectangle[3]=(X-LBX)/OutputGSD;
					yDstRectangle[3]=(Y-LBY)/OutputGSD;
					if(pProjection)
					{
						double X84,Y84,Z84;
						Convert2WGS84(pProjection,pWGS84Projection,X,Y,Z,&X84,&Y84,&Z84);
						X=X84;
						Y=Y84;
						Z=Z84;
					}
					RPC.Ground2Image3(X,Y,Z,&xSrcRectangle[3],&ySrcRectangle[3]);
					ySrcRectangle[3]=nSrcRows-1-ySrcRectangle[3];

					if(xDstRectangle[2]<0||yDstRectangle[2]<0||
							xDstRectangle[0]>nDstCols||yDstRectangle[0]>nDstRows)
					{
						continue;
					}

					RectangleFillin(&srcImage,&dstImage,xSrcRectangle,ySrcRectangle,xDstRectangle,yDstRectangle,nResampleMethod);
				}
			}
		}
		break;
		case DT_TIN:
		{
			pDEM->ConstrustTIN();
			//������������
			triangulateio* pMid=NULL;
			pDEM->GetTIN((DWORD)&pMid);

			double* orthoIX=new double[pMid->numOfPoints];
			double* orthoIY=new double[pMid->numOfPoints];
			double* srcIX=new double[pMid->numOfPoints];
			double* srcIY=new double[pMid->numOfPoints];
			int i=0;
			for(i=0;i<pMid->numOfPoints;i++)
			{
				double X=pMid->pointList[i*2+0];
				double Y=pMid->pointList[i*2+1];
				double Z=pMid->pointAttrList[i];
				if(fabs(Z-INVALID_ALTITUDE)<1e-5)
				{
					continue;
				}

				orthoIX[i]=(X-LBX)/OutputGSD;
				orthoIY[i]=(Y-LBY)/OutputGSD;

				if(pProjection)
				{
					double X84,Y84,Z84;
					Convert2WGS84(pProjection,pWGS84Projection,X,Y,Z,&X84,&Y84,&Z84);
					X=X84;
					Y=Y84;
					Z=Z84;
				}
				RPC.Ground2Image3(X,Y,Z,&srcIX[i],&srcIY[i]);
				srcIY[i]=nSrcRows-1-srcIY[i];
			}

			//������ξ���Ӱ��
			int nNumOfTriangles=pMid->numOfTriangles;
			progress.SetRange(0,nNumOfTriangles);
			progress.SetPosition(0);

			double xSrcTriangle[3],ySrcTriangle[3];
			double xDstTriangle[3],yDstTriangle[3];
			for(i=0;i<nNumOfTriangles;i++)
			{
				progress.StepIt();

				BOOL bValidTri=TRUE;
				for(int m=0;m<3;m++)
				{
					int nPtIndex=pMid->triList[i*3+m];

					if(fabs(pMid->pointAttrList[nPtIndex]-INVALID_ALTITUDE)<1e-5)
					{
						bValidTri=FALSE;
						break;
					}

					xDstTriangle[m]=orthoIX[nPtIndex];
					yDstTriangle[m]=orthoIY[nPtIndex];

					xSrcTriangle[m]=srcIX[nPtIndex];
					ySrcTriangle[m]=srcIY[nPtIndex];
				}
				//�ж�������Ƿ�����Ŀ��Ӱ����
				if(bValidTri)
				{
					BOOL bTemp=FALSE;
					for(int m=0;m<3;m++)
					{
						if(xSrcTriangle[m]>=0&&xSrcTriangle[m]<nSrcCols&&
								ySrcTriangle[m]>=0&&ySrcTriangle[m]<nSrcRows)
						{
							bTemp=TRUE;
							break;
						}
					}
					bValidTri=bTemp;
				}
				if(bValidTri)
				{
//					TriangleFillinFst(&srcImage,&dstImage,xSrcTriangle,ySrcTriangle,xDstTriangle,yDstTriangle,nResampleMethod);
				}
			}

			delete [] orthoIX; orthoIX=NULL;
			delete [] orthoIY; orthoIY=NULL;
			delete [] srcIX; srcIX=NULL;
			delete [] srcIY; srcIY=NULL;
		}
		break;
	}
	hRet=S_OK;

	srcImage.Close();
	dstImage.Close();

	pDEM->Close();
	delete pDEM;
	pDEM=NULL;

	if(pWGS84Projection!=NULL)
	{
		delete pWGS84Projection;
		pWGS84Projection=NULL;
	}
	if(pGCPGX)
	{
		delete [] pGCPGX;
		pGCPGX=NULL;
	}
	if(pGCPGY)
	{
		delete [] pGCPGY;
		pGCPGY=NULL;
	}
	if(pGCPGY)
	{
		delete [] pGCPGY;
		pGCPGY=NULL;
	}

	return S_OK;
}

void CGeometryRectify::Convert2WGS84(CProjection *pProjection,
		CProjection* pProjectionWGS84, double X, double Y, double Z,
		double *pWGS84X, double *pWGS84Y, double *pWGS84Z) {
	if (pProjection!=NULL&&pProjectionWGS84!=NULL) {
		double lfLatitude, lfLongitude;
		pProjection->Map2Geodetic(X, Y, &lfLatitude, &lfLongitude);
		double GX, GY, GZ;
		pProjection->Geodetic2Geocentric(lfLatitude, lfLongitude, Z, &GX, &GY,
				&GZ);
		double GX84, GY84, GZ84;
		pProjection->GeocentricToWGS84(GX, GY, GZ, &GX84, &GY84, &GZ84);

		pProjectionWGS84->Geocentric2Geodetic(GX84, GY84, GZ84, &lfLatitude,
				&lfLongitude, pWGS84Z);
		pProjectionWGS84->Geodetic2Map(lfLatitude, lfLongitude, pWGS84X,
				pWGS84Y);
	} else {
		*pWGS84X=X;
		*pWGS84Y=Y;
		*pWGS84Z=Z;
	}
}

void CGeometryRectify::CovertFromWGS84(CProjection* pProjectionWGS84,
		CProjection *pProjection, double X84, double Y84, double Z84,
		double *pX, double *pY, double *pZ) {
	if (pProjectionWGS84!=NULL&&pProjection!=NULL) {
		double lfLatitude, lfLongitude;
		pProjectionWGS84->Map2Geodetic(X84, Y84, &lfLatitude, &lfLongitude);
		double GX84, GY84, GZ84;
		pProjectionWGS84->Geodetic2Geocentric(lfLatitude, lfLongitude, Z84,
				&GX84, &GY84, &GZ84);

		double GX, GY, GZ;
		pProjection->GeocentricFromWGS84(GX84, GY84, GZ84, &GX, &GY, &GZ);
		pProjection->Geocentric2Geodetic(GX, GY, GZ, &lfLatitude, &lfLongitude,
				pZ);
		pProjection->Geodetic2Map(lfLatitude, lfLongitude, pX, pY);
	} else {
		*pX=X84;
		*pY=Y84;
		*pZ=Z84;
	}
}

HRESULT CGeometryRectify::ProjectImageRPC(char* pszImage,
		char* pszRPC,
		CDEM *pDEM,
		double *pGCPX, double *pGCPY, double *pGCPZ,
		double *pGCPIX, double *pGCPIY,
		int nGCPNum,
		double *pLBX, double *pLBY,
		double *pRTX, double *pRTY,
		CProjection* pProjection)
{
	//��ʼ��RPC����
	CRPCParm RPC;
	HRESULT hRes=RPC.Initialize(pszRPC);
	RPC.SetDEMProjection(pProjection);
	if(hRes==S_FALSE)
	{
		return S_FALSE;
	}
	//�����Ƶ�ͶӰ��WGS84���ϵ
	double* pGCPGX=new double[nGCPNum];
	double* pGCPGY=new double[nGCPNum];
	double* pGCPGZ=new double[nGCPNum];
	memcpy(pGCPGX,pGCPX,sizeof(double)*nGCPNum);
	memcpy(pGCPGY,pGCPY,sizeof(double)*nGCPNum);
	memcpy(pGCPGZ,pGCPZ,sizeof(double)*nGCPNum);

	CProjection* pWGS84Projection=new CProjectWGS84;
	if(pProjection)
	{
		pWGS84Projection->CreateProject("WGS84");

		double lfLatitude,lfLongitude;
		RPC.GetLAT_OFF(&lfLatitude);
		RPC.GetLONG_OFF(&lfLongitude);
		//pProjection->GetCentralMeridian(&lfLongitude);
		long Zone;
		pWGS84Projection->CalcZone(lfLatitude*PI/180.0,lfLongitude*PI/180.0,&Zone);

		pWGS84Projection->SetZoneStep(6);
		pWGS84Projection->SetZone(Zone);
		pWGS84Projection->SetHemisphere(lfLatitude>0?'N':'S');

		for(int i=0;i<nGCPNum;i++)
		{
			Convert2WGS84(pProjection,pWGS84Projection,pGCPX[i],pGCPY[i],pGCPZ[i],&pGCPGX[i],&pGCPGY[i],&pGCPGZ[i]);
		}
	}

	//��ȡԭʼӰ�����
	CImage Image;

	if(Image.Open(pszImage,modeRead)!=S_OK)
	{
		COutput::OutputFileOpenFailed(pszImage);

		return S_FALSE;
	}

	int nRows,nCols;
	Image.GetRows(&nRows);
	Image.GetCols(&nCols);

	Image.Close();

	//��Ӱ�����ת��ΪRPC�е�Ӱ�����ϵ
	int i=0;
	for(i=0;i<nGCPNum;i++)
	{
		pGCPIY[i]=nRows-1-pGCPIY[i];
	}
	//�����񷽸���ϵ�����
	if(nGCPNum>0)
	{
		RPC.CalculateImageAdjustMatrix(pGCPGX,pGCPGY,pGCPGZ,pGCPIX,pGCPIY,nGCPNum);
	}
	//�����﷽����ϵ�����
	if(nGCPNum>0)
	{
		RPC.CalculateGroundAdjustMatrix(pDEM,pGCPGX,pGCPGY,pGCPGZ,pGCPIX,pGCPIY,nGCPNum);
	}
	//�ָ�����ͨӰ�����ϵ
	for(i=0;i<nGCPNum;i++)
	{
		pGCPIY[i]=nRows-1-pGCPIY[i];
	}

	double X0,Y0,Z0,X1,Y1,Z1,X2,Y2,Z2,X3,Y3,Z3;
	double X84,Y84,Z84;
	RPC.Image2Ground3(0,0,pDEM,&X84,&Y84,&Z84);
	CovertFromWGS84(pWGS84Projection,pProjection,X84,Y84,Z84,&X0,&Y0,&Z0);
	RPC.Image2Ground3(0,nRows,pDEM,&X84,&Y84,&Z84);
	CovertFromWGS84(pWGS84Projection,pProjection,X84,Y84,Z84,&X1,&Y1,&Z1);
	RPC.Image2Ground3(nCols,nRows,pDEM,&X84,&Y84,&Z84);
	CovertFromWGS84(pWGS84Projection,pProjection,X84,Y84,Z84,&X2,&Y2,&Z2);
	RPC.Image2Ground3(nCols,0,pDEM,&X84,&Y84,&Z84);
	CovertFromWGS84(pWGS84Projection,pProjection,X84,Y84,Z84,&X3,&Y3,&Z3);

	*pLBX=__min(__min(X0,X1),__min(X2,X3));
	*pLBY=__min(__min(Y0,Y1),__min(Y2,Y3));
	*pRTX=__max(__max(X0,X1),__max(X2,X3));
	*pRTY=__max(__max(Y0,Y1),__max(Y2,Y3));

	if(pWGS84Projection!=NULL)
	{
		delete pWGS84Projection;
		pWGS84Projection=NULL;
	}
	if(pGCPGX)
	{
		delete [] pGCPGX;
		pGCPGX=NULL;
	}
	if(pGCPGY)
	{
		delete [] pGCPGY;
		pGCPGY=NULL;
	}
	if(pGCPGY)
	{
		delete [] pGCPGY;
		pGCPGY=NULL;
	}

	return S_OK;
}

HRESULT CGeometryRectify::OrthoRectifyHJCCD2(double* pIX, double* pIY, 
                           double* pGX,double* pGY,double* pGZ,int nGcpNum,
						   char* pszDEMFile,char* pszL1Image,char* pszL4Image,double lfGSD,
						   double* pL4ImgRgnX,double* pL4ImgRgnY,double * pMatchBandParam, UINT nPolytransType)
{
	if(nGcpNum<=15)
	{
		printf("GCP number less than 15!");
		return S_FALSE;
	}
		
	printf("Reading %s ...\n",pszDEMFile);
	CDEM* pDEM=new CNSDTFDEM;
	HRESULT hRes=pDEM->Open(pszDEMFile,0,modeRead);
	if(hRes==S_FALSE)
	{
		COutput::OutputFileOpenFailed(pszDEMFile);
		return S_FALSE;
	}
	double lfAverageAltitude=0;
	pDEM->GetAverageAltitude(&lfAverageAltitude);		// Get the all grids' average  altitude. [ZuoW,2009/4/21]

	CImage image;
	hRes=image.Open(pszL1Image,modeRead);
	if(hRes==S_FALSE)
	{
		COutput::OutputFileOpenFailed(pszL1Image);

		pDEM->Close();
		delete pDEM;
		pDEM=NULL;
		return S_FALSE;
	}

	int nRows,nCols,nSensorType,nSatelliteID;
	image.GetRows(&nRows);
	image.GetCols(&nCols);
	image.GetSensorType(&nSensorType);
	image.GetSatelliteID(&nSatelliteID);
	printf("nRows is %d,nCols is %d\n",nRows,nCols);
	image.Close();

	hRes=ImagePolyZRectify(pDEM, pszL1Image,pszL4Image,
	 pIX, pIY,
	 pGX,pGY,pGZ,
	 nGcpNum,
	 //PT_SQUARE,
	 nPolytransType,
	 //PT_LINEAR,
	 0,			// 1 --> 0
	 0,0,0,0,
	 lfGSD,
	 pL4ImgRgnX,pL4ImgRgnY,
	 pMatchBandParam);
	
	pDEM->Close();
	delete pDEM;
	pDEM=NULL;

	return hRes;
}

HRESULT CGeometryRectify::OrthoRectifyHJCCD(char* pszAuxFile,
		char* pszEphFile,
		char* pszAttFile,
		char* pszCamFile,
		double* pIX,double* pIY,
		double* pGX,double* pGY,double* pGZ,
		int nGcpNum,
		char* pszDEMFile,
		char* pszL1Image,
		char* pszL4Image,
		double lfGSD,
		double* pL4ImgRgnX,double* pL4ImgRgnY,
		//add by dy  20081029 for pMatchBandParam for MatchBand begin
		double * pMatchBandParam,
		UINT nPolytransType)
{
	printf("Reading %s ...\n",pszDEMFile);
	CDEM* pDEM=new CNSDTFDEM;
	HRESULT hRes=pDEM->Open(pszDEMFile,0,modeRead);
	if(hRes==S_FALSE)
	{
		COutput::OutputFileOpenFailed(pszDEMFile);
		return S_FALSE;
	}
	double lfAverageAltitude=0;
	pDEM->GetAverageAltitude(&lfAverageAltitude);		// Get the all grids' average  altitude. [WeiZ,4/21/2009]

	CDimap dimap;
	hRes=dimap.Initialize(pszAuxFile,pszEphFile,pszAttFile,pszCamFile);
	if(hRes==S_FALSE)
	{
		pDEM->Close();
		delete pDEM;
		pDEM=NULL;

		return S_FALSE;
	}

	CImage image;
	hRes=image.Open(pszL1Image,modeRead);
	if(hRes==S_FALSE)
	{
		COutput::OutputFileOpenFailed(pszL1Image);

		pDEM->Close();
		delete pDEM;
		pDEM=NULL;

		return S_FALSE;
	}

	int nRows,nCols,nSensorType,nSatelliteID;
	image.GetRows(&nRows);
	image.GetCols(&nCols);
	image.GetSensorType(&nSensorType);
	image.GetSatelliteID(&nSatelliteID);
	printf("nRows is %d,nCols is %d\n",nRows,nCols);
	image.Close();

	//generate image or ground GCP adjust rotate matrix
	m_rotIMatrix[0][0]=0.0; m_rotIMatrix[0][1]=1.0; m_rotIMatrix[0][2]=0.0;
	m_rotIMatrix[1][0]=0.0; m_rotIMatrix[1][1]=0.0; m_rotIMatrix[1][2]=1.0;

	m_rotGMatrix[0][0]=0.0; m_rotGMatrix[0][1]=1.0; m_rotGMatrix[0][2]=0.0;
	m_rotGMatrix[1][0]=0.0; m_rotGMatrix[1][1]=0.0; m_rotGMatrix[1][2]=1.0;
	
	if(nGcpNum>0&&pIX!=NULL&&pIY!=NULL&&pGX!=NULL&&pGY!=NULL&&pGZ!=NULL)
	{
		double* dimGX=new double[nGcpNum];
		double* dimGY=new double[nGcpNum];
		int i=0;
		printf("nGCPnum is %d\n",nGcpNum);
		for(i=0;i<nGcpNum;i++)
		{
			if(pGZ[i]<-99995)	{	pGZ[i]=0;	}
			dimap.Image2Map(nRows-pIY[i]-1,pIX[i],pGZ[i],&dimGX[i],&dimGY[i]);
		}

		if(nGcpNum<=3)
		{
			m_rotGMatrix[0][0]=0;
			for(i=0;i<nGcpNum;i++)
			{
				m_rotGMatrix[0][0]+=(dimGX[i]-pGX[i]);
				m_rotGMatrix[1][0]+=(dimGY[i]-pGY[i]);
			}
			m_rotGMatrix[0][0]/=nGcpNum;
			m_rotGMatrix[1][0]/=nGcpNum;
		}
		else
		{
			double Error;
			CalcPolyTransPara(dimGX,dimGY,pGX,pGY,nGcpNum,nPolytransType,&Error);		// PT_CUBE --> nPolytransType, [WeiZ,20090510]
			printf("Error1 is %lf\n",Error);
			memcpy(&m_rotGMatrix[0][0],m_pCoeX,sizeof(double)*3);
			memcpy(&m_rotGMatrix[1][0],m_pCoeY,sizeof(double)*3);
		}

		delete [] dimGX;
		delete [] dimGY;
	}

	int nInterval=__min(nRows/25,nCols/25);
	int nPsdGCPNum=(nRows+nInterval-1)/nInterval*(nCols+nInterval-1)/nInterval;
	double* pPsdGX=new double[nPsdGCPNum];
	double* pPsdGY=new double[nPsdGCPNum];
	double* pPsdGZ=new double[nPsdGCPNum];
	double* pPsdIX=new double[nPsdGCPNum];
	double* pPsdIY=new double[nPsdGCPNum];

	int nIndex=0;
	for(int i=0;i<nRows;i+=nInterval)
	{
		for(int j=0;j<nCols;j+=nInterval)
		{
			pPsdIX[nIndex]=j;
			pPsdIY[nIndex]=nRows-i-1;
			
			int nTime=0;
			double Z0=lfAverageAltitude;
			while(TRUE)
			{
				double tempX,tempY;
				double Z1=Z0;
				dimap.Image2Map(i,j,Z1,&tempX,&tempY);
//				dimap.Image2Map(i,pPsdIX[nIndex],Z1,&tempX,&tempY);
				pPsdGX[nIndex]=m_rotGMatrix[0][0]+m_rotGMatrix[0][1]*tempX+m_rotGMatrix[0][2]*tempY;
				pPsdGY[nIndex]=m_rotGMatrix[1][0]+m_rotGMatrix[1][1]*tempX+m_rotGMatrix[1][2]*tempY;
				//PolyTrans(tempX,tempY,&pPsdGX[nIndex],&pPsdGY[nIndex]);
				pDEM->GetAltitude(pPsdGX[nIndex],pPsdGY[nIndex],&Z1,0);
				if(fabs(Z1-INVALID_ALTITUDE)<1e-5||fabs(Z0-Z1)<0.1||nTime>=20)	{	break;	}
				
				Z0=Z1;
				nTime++;
			}
			pPsdGZ[nIndex]=Z0;
			nIndex++;
		}
	}
	nPsdGCPNum=nIndex;

	hRes=ImagePolyRectify(pszL1Image,pszL4Image,
	 pPsdIX,pPsdIY,
	 pPsdGX,pPsdGY,
	 nPsdGCPNum,
	 //PT_SQUARE,
	 nPolytransType,		// PT_CUBE --> nPolytransType [WeiZ,5/10/2009]
	 //PT_LINEAR,
	 1,
	 0,0,0,0,
	 lfGSD,
	 pL4ImgRgnX,pL4ImgRgnY,
	 pMatchBandParam);

	 /*hRes= ImageTinRectify(pszL1Image,
			pszL4Image,
			pPsdIX,pPsdIY,
			pPsdGX,pPsdGY,
			nPsdGCPNum,
			0,
			0.0, 0.0,
			0.0, 0.0,
			lfGSD);*/
	/*FILE* fp=fopen("G:\\HJ-AB-Data\\control.txt","wt");
	 fprintf(fp,"%d\n",nPsdGCPNum);
	 for(i=0;i<nPsdGCPNum;i++)
	 {
	 fprintf(fp,"%d %lf %lf %lf\n",i,pPsdGX[i],pPsdGY[i],pPsdGZ[i]);
	 }
	 fclose(fp);*/

	delete [] pPsdGX;
	delete [] pPsdGY;
	delete [] pPsdGZ;
	delete [] pPsdIX;
	delete [] pPsdIY;

	pDEM->Close();
	delete pDEM;
	pDEM=NULL;

	return hRes;
}

void CGeometryRectify::de_aberration(double *x, int ifconverse) {
	//printf("source is %15.6f\n", *x);
	//return;
	//printf("source is %15.6f\n", *x);
	(*x)=(*x)-6000.0f;
	if (ifconverse==0) 	{	(*x)=(8.7929e-10)*(*x)*(*x)*(*x)+0.99913f*(*x)+(5.6843e-14)+6000.0f;	} 
	else 				{	(*x)=(-7.8821e-010)*(*x)*(*x)*(*x)+1.0001f*(*x)+(4.5475e-013)+6000.0f;	}
	//printf("target is %15.6f\n", *x);
	//printf("target is %15.6f\n", *x);
}

/*
* 结合多项式和共线方程进行正射校正 [ZuoW,2010/4/27]
*/
HRESULT CGeometryRectify::reOrthoRectifyHJCCD(double* pIX, double* pIY, double* pGX, double* pGY, double* pGZ, int nGcpNum,
											char* pszAuxFile, char* pszEphFile, char* pszAttFile, char* pszCamFile,
											char* pszDEMFile, char* pszSrcImage, char* pszDstImage,
											double* pMatchBandParam, UINT nPolytransType, UINT nResampleMethod)
{
	int nDstRow, nDstCol;	// 目标影像的行/列号
	double dOrientation[12] = {0};		// 外方位元素(X, Y, Z, Phi, Omega, Kappa)
	HRESULT hRes;

	//////////////////////////////////////////////////
	// 解析源影像并计算目标影像范围 [ZuoW,2010/4/27]
	//////////////////////////////////////////////////
	// 打开源影像
	CImage srcImage;
	if(srcImage.Open(pszSrcImage,modeRead)!=S_OK)
	{
		COutput::OutputFileOpenFailed(pszSrcImage);

		return S_FALSE;
	}

	int nSrcRow,nSrcCol;		// 源影像行/列号
	srcImage.GetRows(&nSrcRow);
	srcImage.GetCols(&nSrcCol);
	int nBandNum;
	srcImage.GetBandNum(&nBandNum);
	UINT datatype;
	srcImage.GetDataType(&datatype);
	int nBPB;
	srcImage.GetBPB(&nBPB);
	if(nSrcCol*nSrcRow*nBPB<=0)		// 影像参数检查
	{
		printf("Invalid image and return!\n");
		return S_FALSE;
	}
	double dLBX, dLBY, dCell;
	srcImage.GetGrdInfo(&dLBX, &dLBY, &dCell);	
	// 计算目标影像范围
	double outputLBX, outputLBY, outputRTX, outputRTY, outputGSD;
	outputLBX = outputLBY = outputRTX = outputRTY = 0.0;
	outputGSD = dCell;
	if(fabs(outputRTY-outputLBY)<1e-6||fabs(outputRTX-outputLBX)<1e-6)
	{
		double X0,Y0,X1,Y1,X2,Y2,X3,Y3;
		double Error;
		double inLBX=0.0f;
		double inLBY=0.0f;
		double inRTX=nSrcCol-1;
		double inRTY=nSrcRow-1;
		printf("inLBX is %lf,inLBY is %lf,inRTX is %lf ,inRTY is %lf\n",inLBX,inLBY,inRTX,inRTY);
		InitPolyTransMatrix(pIX,pIY,pGX,pGY,nGcpNum,&Error,nPolytransType);
		PolyTrans(inLBX,inLBY,&X0,&Y0);
		PolyTrans(inLBX,inRTY,&X1,&Y1);
		PolyTrans(inRTX,inRTY,&X2,&Y2);
		PolyTrans(inRTX,inLBY,&X3,&Y3);

		outputLBX=__min(__min(X0,X1),__min(X2,X3));
		outputLBY=__min(__min(Y0,Y1),__min(Y2,Y3));
		outputRTX=__max(__max(X0,X1),__max(X2,X3));
		outputRTY=__max(__max(Y0,Y1),__max(Y2,Y3));
	}
	// 目标影像行/列号
	nDstRow=abs(int((outputRTY-outputLBY)/outputGSD+0.5));
	nDstCol=abs(int((outputRTX-outputLBX)/outputGSD+0.5));

	//////////////////////////////////////////////////
	// Test [ZuoW,2010/6/3]
//	nDstRow = 13736;	nDstCol = 16018;
//	outputLBX = 370857.9787;					outputRTY = 4195600.0573;
//	outputRTX = outputLBX+(nDstCol-1)*outputGSD;	outputLBY = outputRTY-(nDstRow-1)*outputGSD;
	//////////////////////////////////////////////////

	// 如果控制点像素坐标是左下角就转换为左上角坐标 [ZuoW,2010/7/4]
#ifdef IMAGE_LD
	for(int i=0; i<nGcpNum; i++)
	{
		pIY[i] = nSrcRow - pIY[i];
	}
#endif

	//////////////////////////////////////////////////////////////////////////
	// 初始化传感器参数并进行计算外方位元素 [ZuoW,2010/4/28]
	//////////////////////////////////////////////////////////////////////////
	// 初始化CDimap 
	CDimap dimap;
	hRes=dimap.Initialize(pszAuxFile,pszEphFile,pszAttFile,pszCamFile);
	if(hRes==S_FALSE)
	{
		srcImage.Close();

		return S_FALSE;
	}
	// 初始化外方位元素
	dOrientation[0] = dLBX;		// + ((nSrcCol+1)/2)*dCell;
	dOrientation[1] = dLBY;		// + ((nSrcRow+1)/2)*dCell;
	dOrientation[2] = dCell;
	dOrientation[3] = nSrcCol;	// 
	dOrientation[4] = nSrcRow;	//
	// 迭代计算外方位元素
	dimap.InteriavteOrientation(pIX, pIY, pGX, pGY, pGZ, nGcpNum, dOrientation);
	
	/////////////////////////////////////////////////////////////////////////////
	// 打开DEM影像数据
	/////////////////////////////////////////////////////////////////////////////
#ifdef DEBUG
/*	CImage* pDEM = new CImage();
	if(pDEM->Open(pszDEMFile, modeRead)!=S_OK)
	{
		COutput::OutputFileOpenFailed(pszSrcImage);
		return S_FALSE;
	}
	int demRow, demCol, demBPB;
	double demLBX, demLBY, demCell;
	pDEM->GetRows(&demRow);
	pDEM->GetCols(&demCol);
	pDEM->GetBPB(&demBPB);
	pDEM->GetGrdInfo(&demLBX, &demLBY, &demCell);
	BYTE* demValue = new BYTE[demBPB];
#else
*/
	CDEM* pDEM=new CNSDTFDEM;
	hRes=pDEM->Open(pszDEMFile,0,modeRead);
	if(hRes==S_FALSE)
	{
		COutput::OutputFileOpenFailed(pszDEMFile);
		return S_FALSE;
	}
#endif
	/////////////////////////////////////////////////////////////////////////////
	// 纠正影像
	/////////////////////////////////////////////////////////////////////////////
	CImage dstImage;
	if(dstImage.CreateImg(pszDstImage, modeCreate|modeWrite,
					nDstCol, nDstRow, datatype, nBandNum, BSQ,
					outputLBX, outputLBY, outputGSD)!=S_OK)
	{
		COutput::OutputFileOpenFailed(pszDstImage);
		return S_FALSE;
	}
	
	double dCamCoef[6] = {0};
	hRes = dimap.GetCamCoef(dCamCoef);
	if(hRes!=S_OK)
	{
		printf("Get camera coef. failed!\n");
		return S_FALSE;
	}
	double dFocus = dCamCoef[0];
	double dCCDLength = dCamCoef[1];

	int nBPP;
	srcImage.GetBPP(&nBPP);
	BYTE* pPixel = new BYTE[nBPP];
	double srcx, srcy;
	double  dstx, dsty, dstz;
	// 左上角坐标系
	int i,j;
	double tmpX,tmpY;
	for(i=0; i<nDstRow; i++)
	{
		for(j=0; j<nDstCol; j++)
		{
			double  dTemp1, dTemp2, dTemp3;
			dTemp1 = dTemp2 = dTemp3 = 0.0;
			dstx = outputLBX+j*outputGSD;
			dsty = outputLBY+(nDstRow-i-1)*outputGSD;
#ifdef DEBUG
/*			float dstIX = (dstx-demLBX)/demCell;
			float dstIY = (dsty-demLBY)/demCell;
			memset(demValue, 0, demBPB);
			pDEM->GetPixelF(dstIX, dstIY, demValue, 0);
			dstz = *((short*)(demValue));		/// DEM影像宽
#else
*/
			pDEM->GetAltitude(dstx, dsty, &dstz, 0);
			if(dstz<0)		// 海平面以下的都赋值为0(防止出现非法DEM值导致获取原始影像行列号错误) [ZuoW,2010/7/12]
				dstz = 0;
#endif

#ifdef APFLOAT

			CalDstCoordiantion(dstx, dsty, dstz, dOrientation, dFocus, dCCDLength, srcx, srcy);

#else
			dTemp1 = (dstx-dOrientation[0]);
			dTemp2 = (dsty-dOrientation[1]);
			dTemp3 = (dstz-dOrientation[2]);
			srcx = (dOrientation[3]*dTemp1+dOrientation[6]*dTemp2+dOrientation[9]*dTemp3);
			srcx /= (dOrientation[5]*dTemp1+dOrientation[8]*dTemp2+dOrientation[11]*dTemp3);
			srcx *= (-dFocus/dCCDLength);
			srcy = (dOrientation[4]*dTemp1+dOrientation[7]*dTemp2+dOrientation[10]*dTemp3);
			srcy /= (dOrientation[5]*dTemp1+dOrientation[8]*dTemp2+dOrientation[11]*dTemp3);
			srcy *= (-dFocus/dCCDLength);
#endif
			// 从框标坐标系转换到左上角坐标系
			srcx += (nDstCol+1)/2;
			srcy += (nDstRow+1)/2;
			
			if(i>0&&j>0)
			{
				int a = abs((int)(srcx)-(int)(tmpX));
//				int b = abs(srcy-tmpY);
				if(a<1)
					printf("%d,%d, %f, %f, %f, %f\n",i, j, (double)srcx, (double)srcy, (double)tmpX, (double)tmpY);
			}
			tmpX = srcx;
			tmpY = srcy;

			memset(pPixel, 0, nBPP);	// 保存像素灰度
			srcImage.GetPixelF((float)srcx, (float)(nSrcRow-srcy), pPixel, nResampleMethod);
			dstImage.SetPixel(nDstRow-i-1, j, pPixel);
		}
	}	

	// 内存清理
#ifdef DEBUG
//	delete [] demValue;
//	demValue = NULL;
#endif

	pDEM->Close();
	delete pDEM;
	pDEM = NULL;

	srcImage.Close();
	dstImage.Close();

	if (pPixel!=NULL)
	{
		delete [] pPixel;
		pPixel = NULL;
	}
	
	return S_OK;
}
										  
/*
* 将原始控制点(L2)投影到L3级影像上(左下角坐标系) [ZuoW,2010/5/4]
*/
void CGeometryRectify::ProjectGCPL3(double* &pIX, double* &pIY, double* pGX, double* pGY, double* pGZ, 
									int nGcpNum, CImage* pSrcImage)
{
	double dLBX, dLBY, dGSD;
	int nSrcCol, nSrcRow;
	if(pSrcImage==NULL)
	{
		printf("Invalid image!\n");
		return;
	}
	
	HRESULT hRes = pSrcImage->GetGrdInfo(&dLBX, &dLBY, &dGSD);
	if(hRes!=S_OK)
	{
		printf("Invalid image!\n");
		return;
	}
	pSrcImage->GetCols(&nSrcCol);
	pSrcImage->GetRows(&nSrcRow);
	
	for(int i=0; i<nGcpNum; i++)
	{	
		pIX[i] = (pGX[i]-dLBX)/dGSD+1;
		pIY[i] = (pGY[i]-dLBY)/dGSD+1;
		if(pIX[i]<1)	pIX[i] = 1;
		else if(pIX[i]>nSrcCol)		pIX[i] = nSrcCol;
		
		if(pIY[i]<1)	pIY[i] = 1;
		else if(pIY[i]>nSrcCol) pIY[i] = nSrcRow;
	}
	
	return;
}

void CGeometryRectify::CalDstCoordiantion(double dstx, double dsty, double dstz, double dOrientation[12],
		double dFocus, double dCCDLength, double& srcx, double& srcy)
{
	apfloat tmpx = dstx;
	apfloat tmpy = dsty;
	apfloat tmpz = dstz;
	apfloat tmp1 = dOrientation[0];
	apfloat tmp2 = dOrientation[1];
	apfloat tmp3 = dOrientation[2];

	tmpx = tmpx - tmp1;
	tmpy = tmpy - tmp2;
	tmpz = tmpz - tmp3;

	tmp1 = dOrientation[3];
	tmp2 = dOrientation[6];
	tmp3 = dOrientation[9];

	apfloat x = tmp1*tmpx + tmp2*tmpy + tmp3*tmpz;
	tmp1 = dOrientation[5];
	tmp2 = dOrientation[8];
	tmp3 = dOrientation[11];
	x /= (tmp1*tmpx + tmp2*tmpx + tmp3*tmpz);
	tmp1 = dFocus;
	tmp2 = dCCDLength;
	x *= (-tmp1/tmp2);

	tmp1 = dOrientation[4];
	tmp2 = dOrientation[7];
	tmp3 = dOrientation[10];
	apfloat y = tmp1*tmpx+tmp2*tmpy+tmp3*tmpz;
	tmp1 = dOrientation[5];
	tmp2 = dOrientation[8];
	tmp3 = dOrientation[11];
	y /= (tmp1*tmpx + tmp2*tmpx + tmp3*tmpz);
	tmp1 = dFocus;
	tmp2 = dCCDLength;
	y *= (-tmp1/tmp2);

//	cout << x << endl;
//	cout << y << endl;

	srcx = ap2double(x.ap);
	srcy = ap2double(y.ap);

//	cout << srcx << endl;
//	cout << srcy << endl;

	return;
}




										   
