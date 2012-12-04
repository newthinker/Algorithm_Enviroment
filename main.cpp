/*
 * main.cpp
 *
 *  Created on: Mar 23, 2010
 *      Author: zuow
 */

#include "Image.h"
#include "GeometryRectify.h"
#include "GcpMatch.h"
#include "ProjectWGS84.h"
#include "Dimap.h"
#include "BaseDefine.h"
#include <sstream>

#define DEBUG_OC
//#define DEBUG_FGC
//#define GEODETIC_COORDINATE
//#define IMAGE_LD

#define PATH_DEMILE		"/DPS/DEM/"
using namespace std;

// 加载GCP数据
int LoadGMData(char* szFilename, double *&pSrcX, double *&pSrcY, double *&pSrcZ,
		double *&pDstX, double *&pDstY, int& iPointCount, int nZone, int nSrcRows)
{
#ifdef	GEODETIC_COORDINATE
	int iZone = nZone;
	CProjectWGS84 project;
	project.CreateProject("WGS84");
	project.SetZone(iZone);
#endif

	FILE* pInputFile = fopen(szFilename, "r");
	printf("Load GM pairs from %s.",szFilename);
	if(pInputFile != NULL)
	{
		char buffer[512];
		memset(buffer, 0, 512*sizeof(char));

		fgets(buffer, 512, pInputFile);
		iPointCount = atoi(buffer);

		pSrcX = new double[iPointCount];	pSrcY = new double[iPointCount];	pSrcZ = new double[iPointCount];
		pDstX = new double[iPointCount];	pDstY = new double[iPointCount];
		double X, Y;
		for(int i = 0; i < iPointCount; i++)
		{
			fgets(buffer, 512, pInputFile);
			sscanf( buffer, "%*s%lf%lf%lf%lf%lf", &(pSrcX[i]), &(pSrcY[i]), &(pSrcZ[i]),
					&(pDstX[i]), &(pDstY[i]) );

/*
#ifdef	IMAGE_LD		// 左下角坐标系
			pDstY[i] = nSrcRows-pDstY[i];
#endif
*/
			printf("Load %d point is %lf,%lf,%lf,%lf \n",i+1,pSrcX[i], pSrcY[i], pDstX[i], pDstY[i]);
			// 对GCP数据进行转换
#ifdef	GEODETIC_COORDINATE
			project.Geodetic2Map(pSrcY[i] * PI/180.0, pSrcX[i] * PI/180.0, &X, &Y);
			printf("Transfer source is %lf,%lf,target is %lf,%lf \n",pSrcX[i],pSrcY[i],X,Y);
			pSrcX[i]=X;		pSrcY[i]=Y;
#endif
			printf("GCP projection coordinate: %lf  %lf  %lf  %lf  %lf \n", pSrcX[i], pSrcY[i], pSrcZ[i],
					pDstX[i], pDstY[i]);
		}

		fclose(pInputFile);
	}

	return 0;
}

// 进行GCP匹配 [ZuoW,2010/5/13]
int GCPMatch(char* szGCPDir, char* srcImage, char* szReGMFile, double* &pMatchGX, double* &pMatchGY, double* &pMatchGZ, double* &pMatchIX, double* &pMatchIY, int* pGCPNum)
{
	int i;
	HRESULT hRes;
	int pointNum = *pGCPNum;
	char szGcpFiles[25600];		memset(szGcpFiles, 0, 25600);
	for(i=0; i<pointNum; i++)
	{
		char szTemp[32];	memset(szTemp, 0, 32);
		sprintf(szTemp, "%s%d.tif;", szGCPDir, i);
		strcat(szGcpFiles, szTemp);
	}

	double* pGX = new double[pointNum];
	double* pGY = new double[pointNum];
	double* pIX = new double[pointNum];
	double* pIY = new double[pointNum];
	unsigned int* pSuccess = new unsigned int[pointNum];
	double dMaskWindow = 200;
	CGcpMatch GcpMatch;
	hRes = GcpMatch.Match(srcImage, szGcpFiles, dMaskWindow, pGX, pGY, pIX, pIY, &pointNum, pSuccess);
	if(hRes==S_FALSE || pointNum<6)
	{
		delete [] pGX;			pGX = NULL;
		delete [] pGY;			pGY = NULL;
		delete [] pIX;			pIX = NULL;
		delete [] pIY;			pIY = NULL;
		delete [] pSuccess;		pSuccess = NULL;
		return -1;
	}

	pMatchGX = new double[pointNum];	memset(pMatchGX, 0, pointNum*sizeof(double));
	pMatchGY = new double[pointNum];	memset(pMatchGY, 0, pointNum*sizeof(double));
	pMatchGZ = new double[pointNum];	memset(pMatchGZ, 0, pointNum*sizeof(double));
	pMatchIX = new double[pointNum];	memset(pMatchIX, 0, pointNum*sizeof(double));
	pMatchIY = new double[pointNum];	memset(pMatchIY, 0, pointNum*sizeof(double));
	int cpNum = 0;
	for(i=0; i<pointNum; i++)
	{
		if(pSuccess[i])
		{
			pMatchGX[cpNum] = pGX[i];
			pMatchGY[cpNum] = pGY[i];
			pMatchIX[cpNum] = pIX[i];
			pMatchIY[cpNum] = pIY[i];
			cpNum++;
		}
	}
	delete [] pGX;		pGX = NULL;
	delete [] pGY;		pGY = NULL;
	delete [] pIX; 		pIX = NULL;
	delete [] pIY;		pIY = NULL;
	delete [] pSuccess;	pSuccess = NULL;

	CGeometryRectify CalMatchErr;
	double dMatchErr=0.0f;
	int nMaxErrPoint;
	double dGCPMed = 4;
	CalMatchErr.InitPolyTransMatrix(pMatchGX, pMatchGY, pMatchIX, pMatchIY,
			cpNum, &dMatchErr, dGCPMed);
	double dPointErr,dMatchIX,dMatchIY;
	while(dMatchErr>=dGCPMed)
	{
		CalMatchErr.PolyTrans(pMatchGX[0],pMatchGY[0],&dMatchIX,&dMatchIY);
		dMatchErr=sqrt((dMatchIX-pMatchIX[0])*(dMatchIX-pMatchIX[0])+(dMatchIY-pMatchIY[0])*(dMatchIY-pMatchIY[0]));
		nMaxErrPoint=0;

		for(int j=0;j<cpNum;j++)
		{
			CalMatchErr.PolyTrans(pMatchGX[j],pMatchGY[j],&dMatchIX,&dMatchIY);
			dPointErr=sqrt((dMatchIX-pMatchIX[j])*(dMatchIX-pMatchIX[j])+(dMatchIY-pMatchIY[j])*(dMatchIY-pMatchIY[j]));
			if(dMatchErr<dPointErr)
			{
				nMaxErrPoint=j;
				dMatchErr=dPointErr;
			}
		}
		cpNum--;
		for(int j=nMaxErrPoint;j<cpNum;j++)
		{
			pMatchGX[j]=pMatchGX[j+1];
			pMatchGY[j]=pMatchGY[j+1];
			pMatchIX[j]=pMatchIX[j+1];
			pMatchIY[j]=pMatchIY[j+1];
		}
		if(cpNum<6)
		{
			delete [] pMatchGX;		pMatchGX = NULL;
			delete [] pMatchGY;		pMatchGY = NULL;
			delete [] pMatchGZ;		pMatchGZ = NULL;
			delete [] pMatchIX;		pMatchIX = NULL;
			delete [] pMatchIY;		pMatchIY = NULL;
			return -1;
		}
		CalMatchErr.InitPolyTransMatrix( pMatchGX,pMatchGY,pMatchIX,pMatchIY,cpNum, &dMatchErr, 3);
	}

	// 获取高程
	CProjectWGS84 project;
	project.CreateProject("WGS84");
	project.SetZoneStep(6);
	long nZone = 50;
	project.SetZone(nZone);
	project.SetFalseEasting(500000);
	project.SetFalseNorthing(0);

	float px=0.0, py=0.0;
	double latitude,longitude;
	char sdemFilename[256];
	CImage image;
	int iBPB = 1;
	for(i = 0; i < cpNum; i++)
	{
		project.Map2Geodetic(pMatchGX[i], pMatchGY[i], &latitude, &longitude);

		latitude = latitude * 180.0 / PI;
		longitude = longitude * 180.0 / PI;
		strcpy(sdemFilename,"");
		sprintf(sdemFilename,"%sZ_%d_%d.TIF",PATH_DEMILE,(int)((longitude+180+4.99999999)/5),
				(int)((60-latitude+4.99999999)/5));
		if(access(sdemFilename,00)==-1)
		{
			pMatchGZ[i]=-99999;
		}
		else
		{
			image.Open(sdemFilename,modeRead);
			image.World2Image(latitude, longitude, &px, &py);
			image.GetBPB(&iBPB);
			double xStart,yStart,CellSize;
			image.GetGrdInfo(&xStart,&yStart,&CellSize);

			BYTE *pZValues = new BYTE[iBPB];

			image.GetGrayF(px, py, 0, pZValues, 0);
			pMatchGZ[i] = *((short *)pZValues);
			if(pMatchGZ[i]==-32768)
				pMatchGZ[i]=-99999;
			image.Close();

			// Clear
			if(pZValues!=NULL)
			{
				delete []pZValues;
				pZValues = NULL;
			}
		}

		// 转换成经纬度
		pMatchGY[i] = longitude;
		pMatchGX[i] = latitude;
	}

	// 输出GCP到文件[ZuoW,2010/5/14]
	FILE *fp = fopen(szReGMFile, "wt");
	if(fp!=NULL)
	{
		fprintf(fp, "%d\n", cpNum);
		for(i=0;i<cpNum;i++)
		{
			fprintf(fp,"%04d\t%25.6lf%25.6lf%25.6lf%25.6lf%25.6lf\n",
					i+1,pMatchGY[i], pMatchGX[i], pMatchGZ[i], pMatchIX[i],pMatchIY[i]);
			fflush(fp);
		}
		fclose(fp);
	}

	*pGCPNum = cpNum;

	return 0;
}

int Transfer(char* szCheckPoint, char* szSrcImg, char* szOutputFile)
{
	// 打开源影像
	int nSrcRow, nSrcCol;
	double dLBX, dLBY, dCell;
	double *pIX, *pIY, *pSrcX, *pSrcY;
	CImage srcImage;
	if(srcImage.Open(szSrcImg, modeRead)!=S_OK)
	{
		printf("Open source image failed!\n");
		return S_FALSE;
	}
	srcImage.GetRows(&nSrcRow);
	srcImage.GetCols(&nSrcCol);
	srcImage.GetGrdInfo(&dLBX, &dLBY, &dCell);
	srcImage.Close();

	FILE* pInputFile = fopen(szCheckPoint, "r");
	FILE* pOutputFile = fopen(szOutputFile, "w+");
	if(pInputFile!=NULL && pOutputFile!=NULL)
	{
		char buffer[512];
		memset(buffer, 0, 512*sizeof(char));

		fgets(buffer, 512, pInputFile);
		int iPointCount = atoi(buffer);

		pIX = new double[iPointCount];		pIY = new double[iPointCount];
		pSrcX = new double[iPointCount];	pSrcY = new double[iPointCount];
		for(int i = 0; i < iPointCount; i++)
		{
			double dX = 0.;	double dY = 0.;
			fgets(buffer, 512, pInputFile);
			sscanf( buffer, "%*s%lf%lf%lf%lf", &(pIX[i]), &(pIY[i]), &(pSrcX[i]), &(pSrcY[i]));

			printf("Load %d point is %lf,%lf,%lf,%lf \n",i+1,pIX[i], pIY[i], pSrcX[i], pSrcY[i]);
			// 获取纠正影像坐标
			dX = dLBX + pIX[i]*dCell;
#ifdef IMAGE_LD
			dY = dLBY + (nSrcRow-pIY[i])*dCell;
#else
			dY = dLBY + pIY[i]*dCell;
#endif
			printf("Check GCP coordinate: %lf  %lf  %lf  %lf  \n", dX, dY, pSrcX[i], pSrcY[i]);

			double offX = (pSrcX[i]-dX)/dCell;		double offY = (pSrcY[i]-dY)/dCell;
			fprintf(pOutputFile, "%d, %d, %d, %16.3f, %16.3f, %16.3f, %16.3f, %5.3f, %5.3f \n",
					i+1, (int)(pIX[i]), (int)(pIY[i]), dX, dY, pSrcX[i], pSrcY[i], offX, offY);
		}

		fclose(pInputFile);
		fclose(pOutputFile);
	}

	return 0;
}

#ifdef DEBUG
int main()
{
	HRESULT hRes;
	string inputpath = "/dps/workdir/CCD2/LEVEL3/374/";
	string inputfile = "/dps/workdir/CCD2/LEVEL3/374/GC_report.txt";
	int nMaskWindow = 30;
	int nGcpNum = 630;
	
	std::string strGCPFiles;
	std::stringstream ioStream;
	for( int num = 0;num<nGcpNum; num++)
	{
		ioStream<<inputpath<<"TestData/"<<num<<".tif;";
	}
	strGCPFiles = ioStream.str();
	
	double* pGX = new double[nGcpNum];
	double* pGY = new double[nGcpNum];
	double* pIX = new double[nGcpNum];
	double* pIY = new double[nGcpNum];
	BOOL* pbSucceeded = new BOOL[nGcpNum];
	memset(pbSucceeded, 0, sizeof(BOOL) * nGcpNum);
	
	CGcpMatch GcpMatch;
	hRes = GcpMatch.Match((char*)(inputfile.c_str()), (char*)(strGCPFiles.c_str()), nMaskWindow,
			pGX, pGY, pIX, pIY, &nGcpNum, pbSucceeded);
	
/*	
	// GCP data
	double* pSrcX=NULL;		double* pSrcY=NULL;		double* pSrcZ=NULL;
	double* pDstX=NULL;		double* pDstY=NULL;
	int iPointCount = 0;
	int ret = 0;

	char szWorkDir[128];	memset(szWorkDir, 0, 128);
	char szGMFile[256];		memset(szGMFile, 0, 256);
	char szDEMFile[256];	memset(szDEMFile, 0, 256);
	char szInputFile[256];	memset(szInputFile, 0, 256);
	char szOutputFile[256];	memset(szOutputFile, 0, 256);
	char szGCPDir[256];		memset(szGCPDir, 0, 256);
	char szReGMFile[256];	memset(szReGMFile, 0, 256);
	strcpy(szWorkDir, "/dps/workdir/CCD2/LEVEL3/376/");
	strcpy(szGMFile, szWorkDir);	strcat(szGMFile, "GCP_Pair.txt");
	strcpy(szDEMFile, szWorkDir);	strcat(szDEMFile, "TestOC.dem");
	strcpy(szGCPDir, szWorkDir);	strcat(szGCPDir, "TestData/");
	strcpy(szReGMFile, szWorkDir);	strcat(szReGMFile, "ReGM_gcp_pair.txt");
#ifdef	DEBUG_FGC
	strcpy(szInputFile, szWorkDir);	strcat(szInputFile, "GC_report_4.txt");
	strcpy(szOutputFile, szWorkDir);	strcat(szOutputFile, "FGC_img_data_4.tif");
	char szTemp[256];
	strcpy(szTemp, szWorkDir);		strcat(szTemp, "FGC_temp.tif");

//	strcpy(szInputFile, "/dps/workdir/CCD2/LEVEL3/376/transfer/376_check_0517_auto.txt");
//	strcpy(szOutputFile, "/dps/workdir/CCD2/LEVEL3/376/transfer/376level4_Result1.tif");
//	strcpy(szTemp, "/dps/workdir/CCD2/LEVEL3/376/transfer/376_check.txt");
//	Transfer(szInputFile, szOutputFile, szTemp);

#endif
#ifdef	DEBUG_OC
	char strApAuxdata[128];		char strApEphData[128];		char strOpAttData[128];
	strcpy(szInputFile, szWorkDir);		strcat(szInputFile, "OCTemp_4.tif");
	strcpy(szOutputFile, szWorkDir);	strcat(szOutputFile, "OCOutputFile_4.tif");
	strcpy(strApAuxdata, szWorkDir);	strcat(strApAuxdata, "AP_aux_data.txt");
	strcpy(strApEphData, szWorkDir);	strcat(strApEphData, "AP_eph_data.txt");
	strcpy(strOpAttData, szWorkDir);	strcat(strOpAttData, "OP_att_data.txt");
#define CAMERA_FILENAME "/DPS/Config/OPP/OC/Camera.ini"

	double* pMatchBandParam;
#endif
	// 打开源影像
	int nSrcRow, nSrcCol, nZone=50;
	double dLBX, dLBY, dRTX, dRTY, dCell;
	// 打开源影像
	CImage srcImage;
	if(srcImage.Open(szInputFile, modeRead)!=S_OK)
	{
		printf("Open source image failed!\n");
		return S_FALSE;
	}
	srcImage.GetRows(&nSrcRow);
	srcImage.GetCols(&nSrcCol);
	srcImage.GetGrdInfo(&dLBX, &dLBY, &dCell);

	// 解析输入配置文件并读取输入数据
	ret = LoadGMData(szGMFile, pSrcX, pSrcY, pSrcZ, pDstX, pDstY, iPointCount, nZone, nSrcRow);
	if(ret!=0)
	{
		printf("Load GCP data failed!\n");
		return -1;
	}

	// 进行影像校正
	CGeometryRectify GeometryRectify;

#ifdef DEBUG_FGC
	unsigned int iFGCMode = 5;
	unsigned int iResample = 1;

	dLBX = dLBY = dRTX = dRTY = 0.0;
	dCell = 30.0;
//	hRes = GeometryRectify.ImageTinRectify(szInputFile, szTemp,
//			pDstX, pDstY, pSrcX, pSrcY, iPointCount,
//			(UINT)iFGCMode, (UINT)iResample,
//			dLBX, dLBY, dRTX, dRTY, dCell);


	hRes = GeometryRectify.ImageTinRectify(szInputFile, szOutputFile, szDEMFile,
			pDstX, pDstY, pSrcX, pSrcY, pSrcZ, iPointCount,
			iFGCMode, iResample,
			dLBX, dLBY, dRTX, dRTY, dCell);

	delete [] pSrcX;	pSrcX = NULL;
	delete [] pSrcY;	pSrcY = NULL;
	delete [] pDstX;	pDstX = NULL;
	delete [] pDstY;	pDstY = NULL;
#endif

#ifdef DEBUG_OC


	hRes = GeometryRectify.reOrthoRectifyHJCCD(pDstX, pDstY, pSrcX, pSrcY, pSrcZ, iPointCount,
									strApAuxdata, strApEphData, strOpAttData, CAMERA_FILENAME,
									szDEMFile, szInputFile, szOutputFile,
									pMatchBandParam, (UINT)2, (UINT)1);

#endif

	if(hRes!=S_OK)
	{
		printf("Image rectify failed!\n");
		return -1;
	}
	else
	{
		printf("Output params:LB/RT X/Y and cellsize: %f, %f, %f, %f, %f\n",
				dLBX, dLBY, dRTX, dRTY, dCell);
	}
*/
	 
	
	return 0;
}
#endif


