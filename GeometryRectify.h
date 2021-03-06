// GeometryRectify.h: interface for the CGeometryRectify class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GEOMETRYRECTIFY_H__0FDAA1E4_14AA_4906_8442_8BAF46952A5B__INCLUDED_)
#define AFX_GEOMETRYRECTIFY_H__0FDAA1E4_14AA_4906_8442_8BAF46952A5B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Kernel.h"
#define CONST_NUM		3		// 3 points or 6. [ZuoW, 11/9/2009]

typedef enum _POLYTRANSTYPE
{
	PT_NONE=0,
	PT_LINEAR=1,
	PT_SQUARE=2,
	PT_CUBE=3,
	PT_NONLINEAR_XY=4,
	PT_CUBE_Z=5
}PolytransType;

typedef struct tagRGBQUAD 
{
  BYTE    rgbBlue; 
  BYTE    rgbGreen; 
  BYTE    rgbRed; 
  BYTE    rgbReserved; 
} RGBQUAD;

class CGeometryRectify  
{
public:
	HRESULT OrthoRectifyHJCCD(char* pszAuxFile,char* pszEphFile,char* pszAttFile,char* pszCamFile,double* pIX,double* pIY,
										   double* PGX,double* pGY,double* pGZ,int nGcpNum,
										   char* pszDEMFile,char* pszL1Image,char* pszL4Image,double lfGSD,
										   double* pL4ImgRgnX,double* pL4ImgRgnY,double * pMatchBandParam=NULL, UINT nPolytransType=1);
	
	HRESULT OrthoRectifyHJCCD2(double* pIX, double* pIY, double* PGX,double* pGY,double* pGZ,int nGcpNum,
			                               char* pszDEMFile,char* pszL1Image,char* pszL4Image,double lfGSD,
			                               double* pL4ImgRgnX,double* pL4ImgRgnY,double * pMatchBandParam=NULL, UINT nPolytransType=1);
	
	CGeometryRectify();
	virtual ~CGeometryRectify();
	
	//add by dy  20081027 for add inparam pMatchBandParam for MatchBand begin
	HRESULT ImagePolyRectify(char* pszSrcImage,char* pszDstImage,double* pSrcX,double* pSrcY,double* pDstX,double* pDstY,int  nPtNum,UINT nPolytransType,UINT nResampleMethod,double outputLBX,double outputLBY,double outputRTX,double outputRTY,double outputGSD,double* pDstImgRgnX,double* pDstImgRgnY,double * pMatchBandParam=NULL);
	
	//add by dy  20081027 for add inparam pMatchBandParam for MatchBand begin
	HRESULT ImagePolyZRectify(CDEM* pDem, char* pszSrcImage,char* pszDstImage,double* pSrcX,double* pSrcY,double* pDstX,double* pDstY, double* pDstZ, int  nPtNum,UINT nPolytransType,UINT nResampleMethod,double outputLBX,double outputLBY,double outputRTX,double outputRTY,double outputGSD,double* pDstImgRgnX,double* pDstImgRgnY,double * pMatchBandParam=NULL);
		
	//add by dy  20081027 for add inparam  pMatchBandParam for MatchBand end
	HRESULT ImageTinRectify(char* pszSrcImage,char* pszDstImage,double* pSrcX,double* pSrcY,double* pDstX,double* pDstY,int  nPtNum,UINT nPolyType, UINT nResampleMethod,double outputLBX,double outputLBY,double outputRTX,double outputRTY,double outputGSD);
	HRESULT OrthoRectifyRPC(char* pszSrcImage,char* pszSrcRPC,char* pszSrcDEM,double* pGCPX,double* pGCPY,double* pGCPZ,double* pGCPIX,double* pGCPIY,int nGCPNum,char* pszDstImage,double outputLBX,double outputLBY,double outputRTX,double outputRTY,double OutputGSD,UINT nResampleMethod,CProjection* pProjection);
	
	HRESULT InitPolyTransMatrix(double* pSrcX,double* pSrcY,double* pDstX,double* pDstY,int nPtNum,double* pError,UINT nPolytransType);
	HRESULT PolyTrans(double SrcX,double SrcY,double* pDstX,double* pDstY);
	HRESULT PolyTransZ(double SrcX,double SrcY,double SrcZ, double* pDstX,double* pDstY);
	
	HRESULT ProjectImageRPC(char* pszImage,char* pszRPC,CDEM* pDEM,double* pGCPX,double* pGCPY,double* pGCPZ,double* pGCPIX,double* pGCPIY,int nGCPNum,double* pLBX,double* pLBY,double* pRTX,double* pRTY,CProjection* pProjection);
private:
	void CovertFromWGS84(CProjection* pProjectionWGS84,CProjection *pProjection,double X84, double Y84, double Z84,double *pX, double *pY, double *pZ);
	void Convert2WGS84(CProjection* pProjection,CProjection* pProjectionWGS84,double X,double Y,double Z,double* pWGS84X,double* pWGS84Y,double* pWGS84Z);
	void TriangleFillinFst(CImage* pSrcImage,CImage* pDstImage,double* pSrcTriangleX,double* pSrcTriangleY,double* pDstTriangleX,double* pDstTriangleY,UINT nResampleMethod);
	void RectangleFillin(CImage* pSrcImage,CImage* pDstImage,double* pSrcRectangleX,double* pSrcRectangleY,double* pDstRectangleX,double* pDstRectangleY,UINT nResampleMethod);
	BOOL CalcPolyTransPara(double* pSrcX,double* pSrcY,double* pDstX,double* pDstY,int nPtNum,UINT nPolytransType,double* pError);
	BOOL CalcPolyZTransPara(double* pSrcX,double* pSrcY, double *pSrcZ, double* pDstX, double* pDstY,int nPtNum,UINT nPolytransType,double* pError);
	
	int dnrml (double* aa,int n,double bb,double* a,double* b);
	void dsolve (double* a,double* b,double* x,int n,int wide);
	void dldltban1 (double* a,double* d,double* l,int n,int wide);
	void dldltban2 (double* l,double* d,double* b,double* x,int n,int wide);
	void de_aberration(double *x,int ifconverse);   //0 de_aberration 1 de_converse_aberration
private:
	PolytransType m_transType;
	double m_pCoeX[15], m_pCoeY[15];
	double m_pAntiCoeX[10], m_pAntiCoeY[10];
	//for HJ ortho
	double m_rotIMatrix[2][3];
	double m_rotGMatrix[2][3];
};

#endif // !defined(AFX_GEOMETRYRECTIFY_H__0FDAA1E4_14AA_4906_8442_8BAF46952A5B__INCLUDED_)



