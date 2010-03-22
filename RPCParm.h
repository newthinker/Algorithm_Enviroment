// RPCParm.h: interface for the CRPCParm class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RPCPARM_H__0E4D7538_0156_4E88_95E1_CE29D17C42C5__INCLUDED_)
#define AFX_RPCPARM_H__0E4D7538_0156_4E88_95E1_CE29D17C42C5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Kernel.h"

class CRPCParm  
{
public:
	CRPCParm();
	virtual ~CRPCParm();

	HRESULT GetLONG_OFF(double* pLONG_OFF);
	HRESULT GetLAT_OFF(double* pLAT_OFF);
	HRESULT SetDEMProjection(CProjection* pProjection);
	HRESULT Image2Ground3(double IX,double IY,CDEM* pDEM,double* pX,double* pY,double* pZ);
	HRESULT Image2Ground2(double IX, double IY, CDEM* pDEM,double *pLATD,double *pLATM,double *pLATS,double *pLONGD,double *pLONGM,double *pLONGS,double *pHEIGHT);
	HRESULT Image2Ground(double IX,double IY,CDEM* pDEM,double* pLAT,double* pLONG,double* pHEIGHT);
	HRESULT CalculateImageAdjustMatrix(double* pGCPX,double* pGCPY,double* pGCPZ,double* pGCPIX,double* pGCPIY,int nGCPNum);
	HRESULT CalculateGroundAdjustMatrix(CDEM* pDEM,double *pGCPX,double *pGCPY,double *pGCPZ,double *pGCPIX,double *pGCPIY,int nGCPNum);
	HRESULT Ground2Image3(double X,double Y,double Z,double* px,double* py);
	HRESULT Ground2Image2(double LATD,double LATM,double LATS,double LONGD,double LONGM,double LONGS,double HEIGHT,double* px,double* py);
	HRESULT Ground2Image(double latitude, double longitude, double height, double *px, double *py);
	HRESULT Initialize(char* pszRPC);

protected:
	void Covert2DEMProjection(double X84,double Y84,double Z84,double* pX,double* pY,double* pZ);
	void CalculateAntiRPC();
	void Affine1(double *pSrcX, double *pSrcY, double *pDstX, double *pDstY, int nNum, double rotMatrix[2][3]);
	BOOL CalcPolyTransPara(double *pSrcX,double *pSrcY,double *pDstX,double *pDstY,int nNum,double rotMatrix[][3],double* pError);
	int dnrml (double* aa,int n,double bb,double* a,double* b);
	void dsolve (double* a,double* b,double* x,int n,int wide);
	void dldltban1 (double* a,double* d,double* l,int n,int wide);
	void dldltban2 (double* l,double* d,double* b,double* x,int n,int wide);

private:
	//RPC参数
	double LINE_OFF,SAMP_OFF,LINE_SCALE,SAMP_SCALE;
	double LAT_OFF,LONG_OFF,HEIGHT_OFF,LAT_SCALE,LONG_SCALE,HEIGHT_SCALE;
	double* LINE_NUM_COEFF,*LINE_DEN_COEFF;
	double* SAMP_NUM_COEFF,*SAMP_DEN_COEFF;
	//RPC逆变换参数
	double* ANTI_LINE_NUM_COEFF,*ANTI_LINE_DEN_COEFF;
	double* ANTI_SAMP_NUM_COEFF,*ANTI_SAMP_DEN_COEFF;
	//像方改正系数
	double m_rotIMatrix[2][3];
	//物方改正系数
	double m_rotGMatrix[2][3];
	//投影转换组件
	//ITMProjection* m_pTMProjection;
	CProjection* m_pWGS84Projection;
	//DEM投影信息，缺省为WGS84
	CProjection* m_pDEMProjection;
};

#endif // !defined(AFX_RPCPARM_H__0E4D7538_0156_4E88_95E1_CE29D17C42C5__INCLUDED_)
