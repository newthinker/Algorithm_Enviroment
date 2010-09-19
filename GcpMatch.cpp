// GcpMatch.cpp: implementation of the CGcpMatch class.
//
//////////////////////////////////////////////////////////////////////

#include "GcpMatch.h"
#include "PatternMatch.h"
#include "LeastSquareMatch.h"
#include "FArray.hpp"

#include <stdio.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGcpMatch::CGcpMatch()
{

}

CGcpMatch::~CGcpMatch()
{

}

HRESULT CGcpMatch::Match(char* pszWarpFile,
						  char* pszGcpFiles, 
						  double lfMatchWindow, 
						  double *pGX,double *pGY,
						  double *pIX, double *pIY, 
						  int *pGcpNum,
						  BOOL* bSucceeded)
{
	double lfTempMatchWindow=lfMatchWindow;

	//����Ӱ�������
	CImage gcpImage;
	CImage warpImage;

	HRESULT hRes=warpImage.Open(pszWarpFile,modeRead|modeAqlut);
	if(hRes==S_FALSE)
	{
		COutput::OutputFileOpenFailed(pszWarpFile);

		return S_FALSE;
	}
	int nRows,nCols;
	warpImage.GetRows(&nRows);
	warpImage.GetCols(&nCols);
	int nBandNum;
	warpImage.GetBandNum(&nBandNum);
	double LBX,LBY,RTX,RTY,GSD;
	// Get the image's info: LeftBottomX, LeftBottomY and GridSpacingDistance. [Zuo.Wei,4/9/2009]
	warpImage.GetGrdInfo(&LBX,&LBY,&GSD);	
	RTX=LBX+GSD*nCols;
	RTY=LBY+GSD*nRows;
	
	int nBPB;
	warpImage.GetBPB(&nBPB);
	UINT DataType;
	warpImage.GetDataType(&DataType);

	//����ƥ�����
	CPatternMatch PatternMatch;
	CLeastSquareMatch LeastSquareMatch;

	//��������ļ�
	char* pszTemp=pszGcpFiles;
	while(*pszTemp!='\0')
	{
		if(*pszTemp=='*')
		{
			*pszTemp=' ';
		}
		pszTemp++;
	}
	CFArray<char*> strGcpFiles;
	char* pszTemp0=pszGcpFiles;
	while(*pszTemp0!='\0')
	{
		char* pszTemp1=pszTemp0;
	
		while(*pszTemp1!='\0')
		{
			if(*pszTemp1==';')
			{
				*pszTemp1='\0';
				pszTemp1++;
				break;
			}

			pszTemp1++;
		}
		
		if(pszTemp1==pszTemp0)
		{
			break;
		}

		if(access(pszTemp0,0)!=-1)
		{
			strGcpFiles.Add(pszTemp0);
		}
		else
		{
			COutput::OutputFileDoesNotExist(pszTemp0);
		}

		pszTemp0=pszTemp1;
	}

	CProgress progress;
	progress.AddCurInfo("[AGM]");
	progress.AddCurInfo("Prepare data ...");
	progress.SetRange(0,strGcpFiles.GetSize());
	progress.SetStep(1);
	progress.SetPosition(0);

	int i=0;
	for(i=0;i<strGcpFiles.GetSize();i++)
	{
		bSucceeded[i]=FALSE;

		progress.StepIt();

		HRESULT hRes=gcpImage.Open(strGcpFiles[i],modeRead);
		if(hRes==S_FALSE)
		{
			COutput::OutputFileOpenFailed(strGcpFiles[i]);

			continue;
		}

		int nGcpRows,nGcpCols;
		gcpImage.GetRows(&nGcpRows);
		gcpImage.GetCols(&nGcpCols);
		int nGcpBandNum;
		gcpImage.GetBandNum(&nGcpBandNum);
		double gcpLBX,gcpLBY,gcpGSD;
		gcpImage.GetGrdInfo(&gcpLBX,&gcpLBY,&gcpGSD);

		pGX[i]=gcpLBX+((int)(nGcpRows/2)+1)*gcpGSD;
		pGY[i]=gcpLBY+((int)(nGcpCols/2)+1)*gcpGSD;

		pIX[i]=(pGX[i]-LBX)/GSD;
		pIY[i]=(pGY[i]-LBY)/GSD;

		gcpImage.Close();
	}
	
	progress.AddCurInfo("GCP Matching ...");
	progress.SetRange(0,strGcpFiles.GetSize());
	progress.SetStep(1);
	progress.SetPosition(0);
	//��ʼ��ƥ��
	HRESULT hRet=S_FALSE;
	*pGcpNum=0;
	for(i=0;i<strGcpFiles.GetSize();i++)
	{
		if(pGX[i]<LBX||pGX[i]>RTX||pGY[i]<LBY||pGY[i]>RTY)
		{
			continue;
		}

		HRESULT hRes=gcpImage.Open(strGcpFiles[i],modeRead);
		if(hRes==S_FALSE)
		{
			COutput::OutputFileOpenFailed(strGcpFiles[i]);

			continue;
		}

		int nGcpRows,nGcpCols;
		gcpImage.GetRows(&nGcpRows);
		gcpImage.GetCols(&nGcpCols);
		int nGcpBandNum;
		gcpImage.GetBandNum(&nGcpBandNum);
		double gcpLBX,gcpLBY,gcpGSD;
		gcpImage.GetGrdInfo(&gcpLBX,&gcpLBY,&gcpGSD);
		int nGcpBPB;
		gcpImage.GetBPB(&nGcpBPB);
		UINT gcpDataType;
		gcpImage.GetDataType(&gcpDataType);

		BYTE* pGcpBuffer=new BYTE[nGcpRows*nGcpCols*nGcpBandNum*nGcpBPB];
		gcpImage.ReadImg(0.0f,0.0f,(float)nGcpCols,(float)nGcpRows,
							pGcpBuffer,nGcpCols,nGcpRows,nGcpBandNum,
							0,0,nGcpCols,nGcpRows,
							-1,0);
		Gray(pGcpBuffer,nGcpRows,nGcpCols,nGcpBandNum,gcpDataType);

		gcpImage.Close();

		int nHalfMatchWindowCols=(int)(gcpGSD*(nGcpCols+lfMatchWindow*2)/GSD+0.5)/2;
		int nHalfMatchWindowRows=(int)(gcpGSD*(nGcpRows+lfMatchWindow*2)/GSD+0.5)/2;
		int nMatchWindowCols=nHalfMatchWindowCols*2+1;
		int nMatchWindowRows=nHalfMatchWindowRows*2+1;
		
		BYTE* pImageBuffer=new BYTE[nMatchWindowCols*nMatchWindowRows*nBandNum*nBPB];
	
		float x0=(float)pIX[i]-nHalfMatchWindowCols;
		float y0=(float)pIY[i]-nHalfMatchWindowRows;
	
		warpImage.ReadImg(x0,y0,x0+nMatchWindowCols,y0+nMatchWindowRows,
							pImageBuffer,nMatchWindowCols,nMatchWindowRows,nBandNum,
							0,0,nMatchWindowCols,nMatchWindowRows,
							-1,0);
		Gray(pImageBuffer,nMatchWindowRows,nMatchWindowCols,nBandNum,DataType);
//		Gray(pImageBuffer,nMatchWindowRows,nMatchWindowCols,nBandNum,90);
		double X,Y,Coef;
		hRes=PatternMatch.PatternMatch(pImageBuffer,nMatchWindowRows,nMatchWindowCols,GSD,
											pGcpBuffer,nGcpRows,nGcpCols,gcpGSD,
											Pixel_Byte,
											&X,&Y,&Coef);
		if(hRes==S_OK)
		{
			progress.StepIt();

			double xOffset=x0+X-pIX[i];
			double yOffset=y0+Y-pIY[i];

			pIX[i]=x0+X;
			pIY[i]=y0+Y;
			bSucceeded[i]=TRUE;
			
			printf("%s  Success!\n", strGcpFiles[i]);
			
			*pGcpNum+=1;

			for(int j=0;j<strGcpFiles.GetSize();j++)
			{
				if(bSucceeded[j]==FALSE)
				{
					pIX[j]+=xOffset;
					pIY[j]+=yOffset;
				}
			}
			
			break;
		}

		delete [] pGcpBuffer; pGcpBuffer=NULL;
		delete [] pImageBuffer; pImageBuffer=NULL;
	}
	//���ǰ4��ƥ��
//	if(*pGcpNum<=0)
//	{
//		goto EXIT; 
//	}
	//lfMatchWindow=lfTempMatchWindow*2/3;
	for(i=0;i<strGcpFiles.GetSize();i++)
	{
		if(bSucceeded[i]==TRUE)
		{
			continue;
		}

		if(pGX[i]<LBX||pGX[i]>RTX||pGY[i]<LBY||pGY[i]>RTY)
		{
			continue;
		}

		HRESULT hRes=gcpImage.Open(strGcpFiles[i],modeRead);
		if(hRes==S_FALSE)
		{
			COutput::OutputFileOpenFailed(strGcpFiles[i]);

			continue;
		}

		int nGcpRows,nGcpCols;
		gcpImage.GetRows(&nGcpRows);
		gcpImage.GetCols(&nGcpCols);
		int nGcpBandNum;
		gcpImage.GetBandNum(&nGcpBandNum);
		double gcpLBX,gcpLBY,gcpGSD;
		gcpImage.GetGrdInfo(&gcpLBX,&gcpLBY,&gcpGSD);
		int nGcpBPB;
		gcpImage.GetBPB(&nGcpBPB);
		UINT gcpDataType;
		gcpImage.GetDataType(&gcpDataType);

		BYTE* pGcpBuffer=new BYTE[nGcpRows*nGcpCols*nGcpBandNum*nGcpBPB];
		gcpImage.ReadImg(0.0f,0.0f,(float)nGcpCols,(float)nGcpRows,
							pGcpBuffer,nGcpCols,nGcpRows,nGcpBandNum,
							0,0,nGcpCols,nGcpRows,-1,0);
		Gray(pGcpBuffer,nGcpRows,nGcpCols,nGcpBandNum,gcpDataType);

		gcpImage.Close();

		int nHalfMatchWindowCols=(int)(gcpGSD*(nGcpCols+lfMatchWindow*2)/GSD+0.5)/2;
		int nHalfMatchWindowRows=(int)(gcpGSD*(nGcpRows+lfMatchWindow*2)/GSD+0.5)/2;
		int nMatchWindowCols=nHalfMatchWindowCols*2+1;
		int nMatchWindowRows=nHalfMatchWindowRows*2+1;
		
		BYTE* pImageBuffer=new BYTE[nMatchWindowCols*nMatchWindowRows*nBandNum*nBPB];
	
		float x0=(float)pIX[i]-nHalfMatchWindowCols;
		float y0=(float)pIY[i]-nHalfMatchWindowRows;
	
		warpImage.ReadImg(x0,y0,x0+nMatchWindowCols,y0+nMatchWindowRows,
							pImageBuffer,nMatchWindowCols,nMatchWindowRows,nBandNum,
							0,0,nMatchWindowCols,nMatchWindowRows,
							-1,0);
		Gray(pImageBuffer,nMatchWindowRows,nMatchWindowCols,nBandNum,DataType);
//		Gray(pImageBuffer,nMatchWindowRows,nMatchWindowCols,nBandNum,90);
		printf("%s\n", strGcpFiles[i]);
		
		double X,Y,Coef;
		hRes=PatternMatch.PatternMatch(pImageBuffer,nMatchWindowRows,nMatchWindowCols,GSD,
											pGcpBuffer,nGcpRows,nGcpCols,gcpGSD,
											Pixel_Byte,
											&X,&Y,&Coef);
		if(hRes==S_OK)
		{
			progress.StepIt();

			pIX[i]=x0+X;
			pIY[i]=y0+Y;
			bSucceeded[i]=TRUE;
			
	        printf("%s  Success!\n", strGcpFiles[i]);
	
			*pGcpNum+=1;
			
			// if(*pGcpNum==4)
			// {
				// break;
			// }
		}

		delete [] pGcpBuffer; pGcpBuffer=NULL;
		delete [] pImageBuffer; pImageBuffer=NULL;
	}
	//���ǰ4�㹹��һ�ζ���ʽ�任����
	/*if(*pGcpNum>=4)
	{
		double xCoef[10];
		double a0[10], ab0[10], aa0[100], b0;
		memset(aa0,0,sizeof(double)*9);
		memset(ab0,0,sizeof(double)*3);

		double yCoef[10];
		double a1[10], ab1[10], aa1[100], b1;
		memset(aa1,0,sizeof(double)*9);
		memset(ab1,0,sizeof(double)*3);

		for(i=0;i<strGcpFiles.GetSize();i++)	
		{
			if(bSucceeded[i]==TRUE)
			{
				*(a0+0) = 1;
				*(a0+1) = pGX[i];
				*(a0+2) = pGY[i];

				b0 = pIX[i];
				dnrml(a0,3,b0,aa0,ab0);

				*(a1+0) = 1;
				*(a1+1) = pGX[i];
				*(a1+2) = pGY[i];

				b1 = pIY[i];
				dnrml(a1,3,b1,aa1,ab1);
			}
		}
		dsolve(aa0, ab0, xCoef,3,3);
		dsolve(aa1, ab1, yCoef,3,3);
		for(i=0;i<strGcpFiles.GetSize();i++)
		{
			if(bSucceeded[i]==TRUE)
			{
				continue;
			}
			pIX[i]=xCoef[0]+xCoef[1]*pGX[i]+xCoef[2]*pGY[i];
			pIY[i]=yCoef[0]+yCoef[1]*pGX[i]+yCoef[2]*pGY[i];
		}
	}*/

	//lfMatchWindow=lfTempMatchWindow*2/3;
	for(i=0;i<strGcpFiles.GetSize();i++)
	{
		if(bSucceeded[i]==TRUE)
		{
			continue;
		}

		progress.StepIt();

		if(pGX[i]<LBX||pGX[i]>RTX||pGY[i]<LBY||pGY[i]>RTY)
		{
			continue;
		}

		HRESULT hRes=gcpImage.Open(strGcpFiles[i],modeRead);
		if(hRes==S_FALSE)
		{
			COutput::OutputFileOpenFailed(strGcpFiles[i]);

			continue;
		}

		int nGcpRows,nGcpCols;
		gcpImage.GetRows(&nGcpRows);
		gcpImage.GetCols(&nGcpCols);
		int nGcpBandNum;
		gcpImage.GetBandNum(&nGcpBandNum);
		double gcpLBX,gcpLBY,gcpGSD;
		gcpImage.GetGrdInfo(&gcpLBX,&gcpLBY,&gcpGSD);
		int nGcpBPB;
		gcpImage.GetBPB(&nGcpBPB);
		UINT gcpDataType;
		gcpImage.GetDataType(&gcpDataType);
		
		const int nGcpTileSize=17;
		int gcpx0=nGcpCols/2-nGcpTileSize/2;
		int gcpy0=nGcpRows/2-nGcpTileSize/2;
		int gcpx2=gcpx0+nGcpTileSize;
		int gcpy2=gcpy0+nGcpTileSize;
		
		nGcpCols=nGcpTileSize;
		nGcpRows=nGcpTileSize;
		BYTE* pGcpBuffer=new BYTE[nGcpRows*nGcpCols*nGcpBandNum*nGcpBPB];
		gcpImage.ReadImg((float)gcpx0,(float)gcpy0,(float)gcpx2,(float)gcpy2,
							pGcpBuffer,nGcpCols,nGcpRows,nGcpBandNum,
							0,0,nGcpCols,nGcpRows,-1,0);
		Gray(pGcpBuffer,nGcpRows,nGcpCols,nGcpBandNum,gcpDataType);

		gcpImage.Close();

		int nHalfMatchWindowCols=(int)(gcpGSD*(nGcpCols+lfMatchWindow*2)/GSD+0.5)/2;
		int nHalfMatchWindowRows=(int)(gcpGSD*(nGcpRows+lfMatchWindow*2)/GSD+0.5)/2;
		int nMatchWindowCols=nHalfMatchWindowCols*2+1;
		int nMatchWindowRows=nHalfMatchWindowRows*2+1;
		
		BYTE* pImageBuffer=new BYTE[nMatchWindowCols*nMatchWindowRows*nBandNum*nBPB];
	
		float x0=(float)pIX[i]-nHalfMatchWindowCols;
		float y0=(float)pIY[i]-nHalfMatchWindowRows;
	
		warpImage.ReadImg(x0,y0,x0+nMatchWindowCols,y0+nMatchWindowRows,
							pImageBuffer,nMatchWindowCols,nMatchWindowRows,nBandNum,
							0,0,nMatchWindowCols,nMatchWindowRows,
							-1,0);
		Gray(pImageBuffer,nMatchWindowRows,nMatchWindowCols,nBandNum,DataType);
//		Gray(pImageBuffer,nMatchWindowRows,nMatchWindowCols,nBandNum,90);
		double X,Y,Coef;
		hRes=PatternMatch.PatternMatch(pImageBuffer,nMatchWindowRows,nMatchWindowCols,GSD,
											pGcpBuffer,nGcpRows,nGcpCols,gcpGSD,
											Pixel_Byte,
											&X,&Y,&Coef);
		if(hRes==S_OK)
		{
			pIX[i]=x0+X;
			pIY[i]=y0+Y;
			bSucceeded[i]=TRUE;
			
			printf("%s  Success!\n", strGcpFiles[i]);

			*pGcpNum+=1;
		}

		delete [] pGcpBuffer; pGcpBuffer=NULL;
		delete [] pImageBuffer; pImageBuffer=NULL;
	} 
	
	
	//��С���˷�ƥ��
	/*if(*pGcpNum<6)
	{
		goto EXIT;
	}

	memset(aa0,0,sizeof(double)*36);
	memset(ab0,0,sizeof(double)*6);
	memset(aa1,0,sizeof(double)*36);
	memset(ab1,0,sizeof(double)*6);

	for(i=0;i<strGcpFiles.GetSize();i++)	
	{
		if(bSucceeded[i]==TRUE)
		{
			*(a0+0) = 1;
			*(a0+1) = pGX[i];
			*(a0+2) = pGY[i];
			*(a0+3) = pGX[i] * pGX[i];
			*(a0+4) = pGY[i] * pGY[i];
			*(a0+5) = pGX[i] * pGY[i];

			b0 = pIX[i];
			dnrml(a0,6,b0,aa0,ab0);

			*(a1+0) = 1;
			*(a1+1) = pGX[i];
			*(a1+2) = pGY[i];
			*(a1+3) = pGX[i] * pGX[i];
			*(a1+4) = pGY[i] * pGY[i];
			*(a1+5) = pGX[i] * pGY[i];

			b1 = pIY[i];
			dnrml(a1,6,b1,aa1,ab1);
		}
	}
	dsolve(aa0, ab0, xCoef,6,6);
	dsolve(aa1, ab1, yCoef,6,6);

	progress.SetRange(0,strGcpFiles.GetSize());
	progress.SetStep(1);
	progress.SetPosition(0);

	for(i=0;i<strGcpFiles.GetSize();i++)
	{
		progress.StepIt();

		if(bSucceeded[i]==TRUE)
		{
			//continue;
		}

		if(pGX[i]<LBX||pGX[i]>RTX||pGY[i]<LBY||pGY[i]>RTY)
		{
			continue;
		}

		HRESULT hRes=gcpImage.Open(strGcpFiles[i],modeRead);
		if(hRes==S_FALSE)
		{
			COutput::OutputFileOpenFailed(strGcpFiles[i]);

			continue;
		}
		
		int nGcpRows,nGcpCols;
		gcpImage.GetRows(&nGcpRows);
		gcpImage.GetCols(&nGcpCols);
		int nGcpBandNum;
		gcpImage.GetBandNum(&nGcpBandNum);
		double gcpLBX,gcpLBY,gcpGSD;
		gcpImage.GetGrdInfo(&gcpLBX,&gcpLBY,&gcpGSD);
		int nGcpBPB;
		gcpImage.GetBPB(&nGcpBPB);
		UINT gcpDataType;
		gcpImage.GetDataType(&gcpDataType);

		BYTE* pGcpBuffer=new BYTE[nGcpRows*nGcpCols*nGcpBandNum*nGcpBPB];
		gcpImage.ReadImg(0.0f,0.0f,(float)nGcpCols,(float)nGcpRows,
							pGcpBuffer,nGcpCols,nGcpRows,nGcpBandNum,
							0,0,nGcpCols,nGcpRows,-1,0);
		Gray(pGcpBuffer,nGcpCols,nGcpRows,nGcpBandNum,gcpDataType);

		int nPtRow,nPtCol;
		Moravec(pGcpBuffer,nGcpRows,nGcpCols,&nPtRow,&nPtCol);
		double GX=gcpLBX+gcpGSD*nPtCol;
		double GY=gcpLBY+gcpGSD*nPtRow;
		double IX0=xCoef[0]+xCoef[1]*GX+xCoef[2]*GY+xCoef[3]*GX*GX+xCoef[4]*GY*GY+xCoef[5]*GX*GY;
		double IY0=yCoef[0]+yCoef[1]*GX+yCoef[2]*GY+yCoef[3]*GX*GX+yCoef[4]*GY*GY+yCoef[5]*GX*GY;

		int nHalfMatchWindow=(int)(__min(__min(nPtRow,nGcpRows-nPtRow-1),__min(nPtCol,nGcpCols-nPtCol-1))*gcpGSD/GSD);
		int nMatchWindow=nHalfMatchWindow*2+1;
		
		float x0=(float)(nPtCol-nHalfMatchWindow*GSD/gcpGSD);
		float y0=float(nPtRow-nHalfMatchWindow*GSD/gcpGSD);
		memset(pGcpBuffer,0,sizeof(BYTE)*nMatchWindow*nMatchWindow*nGcpBandNum*nGcpBPB);
		gcpImage.ReadImg(x0,y0,(float)(x0+nMatchWindow*GSD/gcpGSD),(float)(y0+nMatchWindow*GSD/gcpGSD),
							pGcpBuffer,nMatchWindow,nMatchWindow,nGcpBandNum,
							0,0,nMatchWindow,nMatchWindow,-1,0);
		Gray(pGcpBuffer,nMatchWindow,nMatchWindow,nGcpBandNum,gcpDataType);

		gcpImage.Close();

		double IX,IY,coef;

		hRes=LeastSquareMatch.SinglePtMatch(pGcpBuffer,
											nMatchWindow,nMatchWindow,
											&warpImage,
											IX0,IY0,
											&IX,&IY,&coef);
		if(hRes==S_OK)
		{
			if(bSucceeded[i]==FALSE)
			{
				*pGcpNum+=1;
			}

			pGX[i]=GX;
			pGY[i]=GY;
			pIX[i]=IX;
			pIY[i]=IY;
			
			bSucceeded[i]=TRUE;
		}
		else if(bSucceeded[i]==FALSE)
		{
			pGX[i]=GX;
			pGY[i]=GY;
			pIX[i]=IX;
			pIY[i]=IY;
		}

		delete [] pGcpBuffer; pGcpBuffer=NULL;
	}*/

EXIT:

	warpImage.Close();

	return *pGcpNum>0?S_OK:S_FALSE;
}

void CGcpMatch::Gray(BYTE *pBuffer, int nRows, int nCols, int nBandNum, UINT datatype)
{
	for(int i=0;i<nRows;i++)
	{
		BYTE* pGrayIndex=pBuffer+i*nCols;
		BYTE* pRowIndex=pBuffer+i*nCols*nBandNum;
		for(int j=0;j<nCols;j++)
		{
			BYTE* pPxlIndex=pRowIndex+j*nBandNum;
			int sum=0;
			for(int k=0;k<nBandNum;k++)
			{
				sum+=pPxlIndex[k];
			}

			pGrayIndex[j]=sum/nBandNum;
		}
	}
}

/**************************************************************************************
 * Select one band in multi_band image. [ZuoW,2010/2/4]
 **************************************************************************************/
void CGcpMatch::Gray(BYTE *pBuffer, int nRows, int nCols, int nBandNum, int nSelBandNum)
{
	for(int i=0;i<nRows;i++)
	{
		BYTE* pGrayIndex=pBuffer+i*nCols;
		BYTE* pRowIndex=pBuffer+i*nCols*nBandNum;
		for(int j=0;j<nCols;j++)
		{
			BYTE* pPxlIndex=pRowIndex+j*nBandNum;

			pGrayIndex[j] = pPxlIndex[nSelBandNum-1];
		}
	}
}

int CGcpMatch::dnrml (double* aa,int n,double bb,double* a,double* b)
{
	register int  i,j;
				                          	
	for (i=0; i<n; i++) {
 	     for (j=0; j<n-i; j++) {
	        *a += *aa * *(aa+j);
                a++; 
	     }
	     *b += *aa * bb;
	     b++; aa++;
	}
	return 0;
}

void CGcpMatch::dsolve (double* a,double* b,double* x,int n,int wide)
{
	int      m;
	double   *d,*l;

	m= n*(n+1)/2;
	d=(double *) malloc(n * sizeof(double));
	l=(double *) malloc((m-n) * sizeof(double));

	memset(d,0,sizeof(double)*n);
	memset(l,0,sizeof(double)*(m-n));

	dldltban1 (a,d,l,n,wide);

	dldltban2 (l,d,b,x,n,wide);
	free(d); free(l);
}

void CGcpMatch::dldltban1 (double* a,double* d,double* l,int n,int wide)
{
	int i,j,k,kk,km,m;
	double *ao,*aa,*co,*c;

	m = wide*(2*n+1-wide)/2;
	c =(double *)calloc ((m-wide),sizeof(double));

	ao=a; co=c; a +=wide;
	for (i=0; i<m-wide; i++) *c++ = *a++;
	c=co; a=ao;

	for (k=1; k<n; k++) {
	   if (k<n-wide+2) kk=wide-1;
	   else kk--;
	
	   *d = *a++; aa=a;  a += kk;

	   if (k<n-wide+1) km=wide;
	   else km=n-k+1;

	   for (i=1; i<kk+1; i++) {
              *l = *aa++ / *d;
	      for (j=0; j<kk-i+1; j++) *(a+j) -= *l * *(aa+j-1);
              l++;

              if (k+i>n-wide+1) km--;
              a += km;
           }

	   a=aa; d++;
           if (k==n-1)  *d = *a;
        }

	a=ao;  a +=wide;
	for (i=0; i<m-wide; i++) *a++ = *c++;
	c=co; free(c);
}

void CGcpMatch::dldltban2 (double* l,double* d,double* b,double* x,int n,int wide)
{
	int i,j,kk,m;
		double *bo, *lo, *xx;
		double *bb,*bbo;

		bb =(double*)calloc(n,sizeof(double));
		bbo=bb;

		bo=b; lo=l;

		for (i=0; i<n; i++)
		{
			*(bb++) = *(b++);
		}
		b=bo;  bb=bbo;
		m = wide*(2*n+1-wide)/2;

		for (i=1; i<n; i++)
		{
		   if (i<n-wide+2) kk=wide;
		   else kk--;

		   b=bo+i;
		   for (j=1; j<kk; j++) 
		   {
			   *b -= *(b-j) * *l;
			   b++;
			   l++;
		   }
		} 		
		
	        kk=0;
		b=bo+n-1;  l=lo+m-n-1;
		x += n-1;  xx=x;  d += n-1;
		
		*x-- = (*b--) / (*d--);
	        
		for (i=1; i<n; i++)  
		{
		   if (i<wide) kk++;
		   else {  kk=wide-1;  xx--; }

		   *x = *b-- / *d--;
		   for (j=1; j<kk+1; j++) *x -= *(l--) * *(xx-j+1);
		   x--;
		}

		b=bo;
		for (i=0; i<n; i++) *b++ = *bb++;
		bb=bbo; free(bb);
}

double* gpIV=NULL;
int IVcompare( const void *arg1, const void *arg2 )
{
	int index1=*(int*)arg1;
	int index2=*(int*)arg2;
	
	double sub=gpIV[index1]-gpIV[index2];
	if(sub>1e-5)
	{
		return -1;
	}
	else if(sub<1e-5)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

BOOL CGcpMatch::Moravec(BYTE *pBuffer, int nRows, int nCols, int *pPtRow, int *pPtCol)
{
	*pPtRow=nRows/2;
	*pPtCol=nCols/2;

	int ivRows=nRows-18;
	int ivCols=nCols-18;
	double* pIV=new double[ivRows*ivCols];
	memset(pIV,0,sizeof(double)*ivRows*ivCols);
	int* inxIV=new int[ivRows*ivCols];
	memset(inxIV,0,sizeof(int)*ivRows*ivCols);
	int* srtIV=new int[ivRows*ivCols];
	memset(srtIV,0,sizeof(int)*ivRows*ivCols);

	for(int i=0;i<ivRows;i++)
	{
		double* pIVIndex=pIV+i*ivCols;
		int* inxIVIndex=inxIV+i*ivCols;
		int* srtIVIndex=srtIV+i*ivCols;

		int m=i+9;
		for(int j=0;j<ivCols;j++)
		{
			int n=j+9;

			double iv0=0,iv1=0,iv2=0,iv3=0;

			//ˮƽ����
			int ii,jj;
			BYTE* pBufferIndex=pBuffer+m*nCols;
			for(ii=n-2;ii<n+2;ii++)
			{
				iv0=iv0+(pBufferIndex[ii]-pBufferIndex[ii+1])*(pBufferIndex[ii]-pBufferIndex[ii+1]);
			}
			//��ֱ����
			for(ii=m-2;ii<m+2;ii++)
			{
				BYTE* pBufferIndex0=pBuffer+ii*nCols;
				BYTE* pBufferIndex1=pBuffer+(ii+1)*nCols;
				iv1=iv1+(pBufferIndex0[n]-pBufferIndex1[n])*(pBufferIndex0[n]-pBufferIndex1[n]);
			}
			//�������Ϸ���
			for(ii=m-2,jj=n-2;ii<m+2&&jj<n+2;ii++,jj++)
			{
				BYTE* pBufferIndex0=pBuffer+ii*nCols;
				BYTE* pBufferIndex1=pBuffer+(ii+1)*nCols;
				iv2=iv2+(pBufferIndex0[jj]-pBufferIndex1[jj+1])*(pBufferIndex0[jj]-pBufferIndex1[jj+1]);
			}
			//�������·���
			for(ii=m+2,jj=n-2;ii>m-2&&jj<n+2;ii--,jj++)
			{
				BYTE* pBufferIndex0=pBuffer+ii*nCols;
				BYTE* pBufferIndex1=pBuffer+(ii-1)*nCols;
				iv3=iv3+(pBufferIndex0[jj]-pBufferIndex1[jj+1])*(pBufferIndex0[jj]-pBufferIndex1[jj+1]);
			}

			pIVIndex[j]=__min(__min(iv0,iv1),__min(iv2,iv3));
			inxIVIndex[j]=m*nCols+n;
			srtIVIndex[j]=i*ivCols+j;
		}
	}

	gpIV=pIV;
	qsort(srtIV,ivRows*ivCols,sizeof(int),IVcompare);

	*pPtRow=inxIV[srtIV[0]]/nCols;
	*pPtCol=inxIV[srtIV[0]]%nCols;

	delete [] pIV; pIV=NULL;
	delete [] inxIV; inxIV=NULL;
	delete [] srtIV; srtIV=NULL;
	
	return TRUE;
}
