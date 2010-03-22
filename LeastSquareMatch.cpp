// LeastSquareMatch.cpp: implementation of the CLeastSquareMatch class.
//
//////////////////////////////////////////////////////////////////////

#include "LeastSquareMatch.h"

#define PYRAMIDLAYER 1
#define MATCHWINDOW 13

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLeastSquareMatch::CLeastSquareMatch()
{

}

CLeastSquareMatch::~CLeastSquareMatch()
{

}

void CLeastSquareMatch::pnrml(double *aa, int n, double bb, double *a, double *b, double p)
{
	register int  i,j;
	double *a0 = a;
				                          	
	for (i=0; i<n; i++){
 	     for (j=0; j<n-i; j++){
	        *a += ((*aa)*(*(aa+j))*p);
                a++; 
	     }
	     *b += ((*aa)*bb*p);
	     b++; aa++;
	}

	a = a0;
};

void CLeastSquareMatch::dnrml (double* aa,int n,double bb,double* a,double* b)
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
}

void CLeastSquareMatch::dsolve (double* a,double* b,double* x,int n,int wide)
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

void CLeastSquareMatch::dldltban1 (double* a,double* d,double* l,int n,int wide)
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

void CLeastSquareMatch::dldltban2 (double* l,double* d,double* b,double* x,int n,int wide)
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

HRESULT CLeastSquareMatch::SinglePtMatch(BYTE *pG, 
										  int nGRows, int nGCols, 
										  CImage *pG2Image, 
										  double x0, double y0, 
										  double* px2,double* py2,
										  double* pCoef)
{
	int nBandNum,nBPB,nBPP;
	pG2Image->GetBandNum(&nBandNum);
	pG2Image->GetBPB(&nBPB);
	nBPP=nBPB*nBandNum;

	if(nBPB!=1)
	{
		return S_FALSE;
	}

	BYTE* pG2=new BYTE[nGRows*nGCols*nBPB];
	memset(pG2,0,sizeof(BYTE)*nGRows*nGCols*nBPB);
	BYTE* pPixel=new BYTE[nBPP];

	for(int pydLayer=PYRAMIDLAYER;pydLayer>=1;pydLayer--)
	{
		int nIterate=0;
		double a0[3]={0,1*pydLayer,0};
		double b0[3]={0,0,1*pydLayer};
		double a[3],b[3];
		double lfCorreCoef0=0,lfCorreCoef1=0;
		double h0=0,h1=1;
		memcpy(a,a0,sizeof(double)*3);
		memcpy(b,b0,sizeof(double)*3);

		BYTE *pG1=NULL; 
		int nG1Rows,nG1Cols;
		Pyramid(pG,nGRows,nGCols,
				Pixel_Byte,pydLayer, 
				&pG1,&nG1Rows,&nG1Cols);

		do
		{
			//��ͶӰ�����������
			int i=0;
			for(i=-nG1Rows/2;i<=nG1Rows/2;i++)
			{
				int ii=i+nG1Rows/2;

				BYTE* pG2Index=pG2+ii*nG1Cols;
				int j=0;
				for(j=-nG1Cols/2;j<=nG1Cols/2;j++)
				{
					int jj=j+nG1Cols/2;

					double x2=a[0]+a[1]*j+a[2]*i;
					double y2=b[0]+b[1]*j+b[2]*i;
					memset(pPixel,0,nBPP);
					pG2Image->GetPixelF((float)(x2+x0),(float)(y2+y0),pPixel,1);

					double tempGray=0;
					for(int m=0;m<nBandNum;m++)
					{
						tempGray+=pPixel[m];
					}
					tempGray=h0+h1*tempGray/nBandNum;
					if(tempGray>=1e-5&&tempGray<=255)
					{
						pG2Index[jj]=(BYTE)(tempGray);
					}
					else if(tempGray<=1e-5)
					{
						pG2Index[jj]=0;
					}
					else
					{
						pG2Index[jj]=255;
					}
				}
			}

			//�������ϵ��
			double si=0.0,sp=0.0,sii=0.0,spp=0.0,sip=0.0;
			int nCount=0;
			for(int m=0;m<nG1Rows;m++)
			{
				BYTE* pG1Index=pG1+m*nG1Cols;
				BYTE* pG2Index=pG2+m*nG1Cols;
				for(int n=0;n<nG1Cols;n++)
				{
					BYTE gi=pG2Index[n];
					BYTE gp=pG1Index[n];
					si=si+gi;
					sp=sp+gp;
					sii=sii+gi*gi;
					spp=spp+gp*gp;
					sip=sip+gi*gp;
					
					nCount++;
				}
			}
			double N=(double)nCount;
			lfCorreCoef1=(sip-si*sp/N)/(sqrt((sii-si*si/N)*(spp-sp*sp/N))+1e-6);
			if(lfCorreCoef1<lfCorreCoef0||nIterate>20)
			{
				break;
			}
			//���汾�ε����
			memcpy(a0,a,sizeof(double)*3);
			memcpy(b0,b,sizeof(double)*3);
			lfCorreCoef0=lfCorreCoef1;

			//���㼸�λ�����������������
			double na[8], nab[8], naa[64], nb;
			double dcoef[8];
			memset(naa,0,sizeof(double)*64);
			memset(nab,0,sizeof(double)*8);
			for(i=-nG1Rows/2+1;i<=nG1Rows/2-1;i++)
			{
				int ii=i+nG1Rows/2;

				for(int j=-nG1Cols/2+1;j<=nG1Cols/2-1;j++)
				{
					int jj=j+nG1Cols/2;

					double dgx=0.5*(pG2[ii*nG1Cols+jj+1]-pG2[ii*nG1Cols+jj-1]);
					double dgy=0.5*(pG2[(ii+1)*nG1Cols+jj]-pG2[(ii-1)*nG1Cols+jj]);

					na[0]=1;
					na[1]=pG2[ii*nG1Cols+jj];
					na[2]=dgx;
					na[3]=j*dgx;
					na[4]=i*dgx;
					na[5]=dgy;
					na[6]=j*dgy;
					na[7]=i*dgy;

					nb=pG1[ii*nG1Cols+jj]-pG2[ii*nG1Cols+jj];

					dnrml(na,8,nb,naa,nab);
				}
			}
			dsolve(naa,nab,dcoef,8,8);

			//���㼸�λ������������
			a[0]=a0[0]+dcoef[2]+a0[0]*dcoef[3]+b0[0]*dcoef[4];
			a[1]=a0[1]+a0[1]*dcoef[3]+b0[1]*dcoef[4];
			a[2]=a0[2]+a0[2]*dcoef[3]+b0[2]*dcoef[4];

			b[0]=b0[0]+dcoef[5]+a0[0]*dcoef[6]+b0[0]*dcoef[7];
			b[1]=b0[1]+a0[1]*dcoef[6]+b0[1]*dcoef[7];
			b[2]=b0[2]+a0[2]*dcoef[6]+b0[2]*dcoef[7];

			h0=h0+dcoef[0]+h0*dcoef[1];
			h1=h1+h1*dcoef[1];

			nIterate++;
		}
		while(TRUE);

		//�������ƥ���λ
		double numx=0,denx=0,numy=0,deny=0;
		for(int i=-nG1Rows/2+1;i<=nG1Rows/2-1;i++)
		{
			int ii=i+nG1Rows/2;

			for(int j=-nG1Cols/2+1;j<=nG1Cols/2-1;j++)
			{
				int jj=j+nG1Cols/2;

				double dgx=0.5*(pG1[ii*nG1Cols+jj+1]-pG1[ii*nG1Cols+jj-1]);
				double dgy=0.5*(pG1[(ii+1)*nG1Cols+jj]-pG1[(ii-1)*nG1Cols+jj]);
				
				double dgx2=dgx*dgx;
				double dgy2=dgy*dgy;

				numx=numx+j*dgx2;
				denx=denx+dgx2;

				numy=numy+i*dgy2;
				deny=deny+dgy2;
			}
		}

		delete [] pG1; pG1=NULL;

		double xt=numx/denx;
		double yt=numy/deny;

		*px2=a0[0]+a0[1]*xt+a0[2]*yt+x0;
		*py2=b0[0]+b0[1]*xt+b0[2]*yt+y0;
		*pCoef=lfCorreCoef0;
		if(*pCoef>0.75)
		{
			x0=*px2;
			y0=*py2;
		}
	}

	delete [] pG2; pG2=NULL;
	delete pPixel; pPixel=NULL;

	return *pCoef>0.6?S_OK:S_FALSE;
}

BOOL CLeastSquareMatch::Pyramid(BYTE *pSrcImage, 
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

	int xOff=0;
	int yOff=0;
	if(nCols>MATCHWINDOW)
	{
		xOff=(nCols-MATCHWINDOW)/2;
		nCols=MATCHWINDOW;
	}
	if(nRows>MATCHWINDOW)
	{
		yOff=(nRows-MATCHWINDOW)/2;
		nRows=MATCHWINDOW;
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
						if((i+yOff)*nPrdLayer+m>=nSrcRows)
						{
							continue;
						}

						BYTE* pSrcImageIndex=pSrcImage+((i+yOff)*nPrdLayer+m)*nSrcCols;
						for(int n=0;n<nPrdLayer;n++)
						{
							if((j+xOff)*nPrdLayer+n>=nSrcCols)
							{
								continue;
							}

							gray=gray+pSrcImageIndex[(j+xOff)*nPrdLayer+n];

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
						if((i+yOff)*nPrdLayer+m>=nSrcRows)
						{
							continue;
						}

						unsigned short* pSrcImageIndex=((unsigned short*)pSrcImage)+((i+yOff)*nPrdLayer+m)*nSrcCols;
						for(int n=0;n<nPrdLayer;n++)
						{
							if((j+xOff)*nPrdLayer+n>=nSrcCols)
							{
								continue;
							}

							gray=gray+pSrcImageIndex[(j+xOff)*nPrdLayer+n];

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
						if((i+yOff)*nPrdLayer+m>=nSrcRows)
						{
							continue;
						}

						short* pSrcImageIndex=((short*)pSrcImage)+((i+yOff)*nPrdLayer+m)*nSrcCols;
						for(int n=0;n<nPrdLayer;n++)
						{
							if((j+xOff)*nPrdLayer+n>=nSrcCols)
							{
								continue;
							}

							gray=gray+pSrcImageIndex[(j+xOff)*nPrdLayer+n];

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
						if((i+yOff)*nPrdLayer+m>=nSrcRows)
						{
							continue;
						}

						unsigned int* pSrcImageIndex=((unsigned int*)pSrcImage)+((i+yOff)*nPrdLayer+m)*nSrcCols;
						for(int n=0;n<nPrdLayer;n++)
						{
							if((j+xOff)*nPrdLayer+n>=nSrcCols)
							{
								continue;
							}

							gray=gray+pSrcImageIndex[(j+xOff)*nPrdLayer+n];

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
						if((i+yOff)*nPrdLayer+m>=nSrcRows)
						{
							continue;
						}

						int* pSrcImageIndex=((int*)pSrcImage)+((i+yOff)*nPrdLayer+m)*nSrcCols;
						for(int n=0;n<nPrdLayer;n++)
						{
							if((j+xOff)*nPrdLayer+n>=nSrcCols)
							{
								continue;
							}

							gray=gray+pSrcImageIndex[(j+xOff)*nPrdLayer+n];

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
						if((i+yOff)*nPrdLayer+m>=nSrcRows)
						{
							continue;
						}

						float* pSrcImageIndex=((float*)pSrcImage)+((i+yOff)*nPrdLayer+m)*nSrcCols;
						for(int n=0;n<nPrdLayer;n++)
						{
							if((j+xOff)*nPrdLayer+n>=nSrcCols)
							{
								continue;
							}

							gray=gray+pSrcImageIndex[(j+xOff)*nPrdLayer+n];

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
						if((i+yOff)*nPrdLayer+m>=nSrcRows)
						{
							continue;
						}

						double* pSrcImageIndex=((double*)pSrcImage)+((i+yOff)*nPrdLayer+m)*nSrcCols;
						for(int n=0;n<nPrdLayer;n++)
						{
							if((j+xOff)*nPrdLayer+n>=nSrcCols)
							{
								continue;
							}

							gray=gray+pSrcImageIndex[(j+xOff)*nPrdLayer+n];

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