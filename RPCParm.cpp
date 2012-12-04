// RPCParm.cpp: implementation of the CRPCParm class.
//
//////////////////////////////////////////////////////////////////////

#include "RPCParm.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRPCParm::CRPCParm()
{
	m_pDEMProjection=NULL;

	ANTI_LINE_NUM_COEFF=new double[20];
	ANTI_LINE_DEN_COEFF=new double[20];
	ANTI_SAMP_NUM_COEFF=new double[20];
	ANTI_SAMP_DEN_COEFF=new double[20];
	LINE_NUM_COEFF=new double[20];
	LINE_DEN_COEFF=new double[20];
	SAMP_NUM_COEFF=new double[20];
	SAMP_DEN_COEFF=new double[20];

	m_rotIMatrix[0][0]=0.0; m_rotIMatrix[0][1]=1.0; m_rotIMatrix[0][2]=0.0;
	m_rotIMatrix[1][0]=0.0; m_rotIMatrix[1][1]=0.0; m_rotIMatrix[1][2]=1.0;

	m_rotGMatrix[0][0]=0.0; m_rotGMatrix[0][1]=1.0; m_rotGMatrix[0][2]=0.0;
	m_rotGMatrix[1][0]=0.0; m_rotGMatrix[1][1]=0.0; m_rotGMatrix[1][2]=1.0;

	/*m_pTMProjection=NULL;
	const GUID CLSID_TMProjection={0xf8c5ddc5,0xaf43,0x4e94,{0x95,0xa2,0x98,0xe4,0xf3,0x5f,0x60,0xfd}};
	const GUID IID_ITMProjection={0x7b4c946a,0x24d8,0x4b0e,{0x9a,0xd9,0xd8,0x5d,0x06,0x48,0xf9,0x64}};
	CoCreateInstance(CLSID_TMProjection,NULL,CLSCTX_ALL,IID_ITMProjection,(void**)&m_pTMProjection);*/
	m_pWGS84Projection=new CProjectWGS84;
	m_pWGS84Projection->CreateProject("WGS84");
}

CRPCParm::~CRPCParm()
{
	m_pDEMProjection=NULL;

	/*if(m_pTMProjection)
	{
		m_pTMProjection->Release();
		m_pTMProjection=NULL;
	}*/
	if(m_pWGS84Projection)
	{
		delete m_pWGS84Projection;
		m_pWGS84Projection=NULL;
	}
	delete [] ANTI_LINE_NUM_COEFF;
	delete [] ANTI_LINE_DEN_COEFF;
	delete [] ANTI_SAMP_NUM_COEFF;
	delete [] ANTI_SAMP_DEN_COEFF;
	delete [] LINE_NUM_COEFF;
	delete [] LINE_DEN_COEFF;
	delete [] SAMP_NUM_COEFF;
	delete [] SAMP_DEN_COEFF;
}

HRESULT CRPCParm::Initialize(char* pszRPC)
{
	//��ʼ�������﷽�������
	m_rotIMatrix[0][0]=0.0; m_rotIMatrix[0][1]=1.0; m_rotIMatrix[0][2]=0.0;
	m_rotIMatrix[1][0]=0.0; m_rotIMatrix[1][1]=0.0; m_rotIMatrix[1][2]=1.0;

	m_rotGMatrix[0][0]=0.0; m_rotGMatrix[0][1]=1.0; m_rotGMatrix[0][2]=0.0;
	m_rotGMatrix[1][0]=0.0; m_rotGMatrix[1][1]=0.0; m_rotGMatrix[1][2]=1.0;
	
	/*if(m_pTMProjection==NULL)
	{
		AfxMessageBox(IDS_PROJECTION_COMPONENT_ERROR);
		
		return S_FALSE;
	}*/

	FILE* fp=fopen(pszRPC,"rt");
	if(fp==NULL)
	{
		COutput::OutputFileOpenFailed(pszRPC);

		return S_FALSE;
	}

	HRESULT hRet=S_FALSE;
	char szTemp[50];
	int i=0;
	if(fscanf(fp,"%s%lf%s",szTemp,&LINE_OFF,szTemp)!=3)
	{
		goto EXIT;
	}
	if(fscanf(fp,"%s%lf%s",szTemp,&SAMP_OFF,szTemp)!=3)
	{
		goto EXIT;
	}
	if(fscanf(fp,"%s%lf%s",szTemp,&LAT_OFF,szTemp)!=3)
	{
		goto EXIT;
	}
	if(fscanf(fp,"%s%lf%s",szTemp,&LONG_OFF,szTemp)!=3)
	{
		goto EXIT;
	}
	if(fscanf(fp,"%s%lf%s",szTemp,&HEIGHT_OFF,szTemp)!=3)
	{
		goto EXIT;
	}
	if(fscanf(fp,"%s%lf%s",szTemp,&LINE_SCALE,szTemp)!=3)
	{
		goto EXIT;
	}
	if(fscanf(fp,"%s%lf%s",szTemp,&SAMP_SCALE,szTemp)!=3)
	{
		goto EXIT;
	}
	if(fscanf(fp,"%s%lf%s",szTemp,&LAT_SCALE,szTemp)!=3)
	{
		goto EXIT;
	}
	if(fscanf(fp,"%s%lf%s",szTemp,&LONG_SCALE,szTemp)!=3)
	{
		goto EXIT;
	}
	if(fscanf(fp,"%s%lf%s",szTemp,&HEIGHT_SCALE,szTemp)!=3)
	{
		goto EXIT;
	}
	
	for(i=0;i<20;i++)
	{
		if(fscanf(fp,"%s%lf",szTemp,&LINE_NUM_COEFF[i])!=2)
		{
			goto EXIT;
		}
	}
	for(i=0;i<20;i++)
	{
		if(fscanf(fp,"%s%lf",szTemp,&LINE_DEN_COEFF[i])!=2)
		{
			goto EXIT;
		}
	}
	for(i=0;i<20;i++)
	{
		if(fscanf(fp,"%s%lf",szTemp,&SAMP_NUM_COEFF[i])!=2)
		{
			goto EXIT;
		}
	}
	for(i=0;i<20;i++)
	{
		if(fscanf(fp,"%s%lf",szTemp,&SAMP_DEN_COEFF[i])!=2)
		{
			goto EXIT;
		}
	}

	/*if(m_pTMProjection)
	{
		int Zone=(int)(LONG_OFF+180)/6+1;

		char szProjName[100];
		sprintf(szProjName,"UTM %c %d",LAT_OFF>0?'N':'S',Zone);
		m_pTMProjection->TM_Set_Projection_Name((BYTE*)szProjName);
	}*/
	if(m_pWGS84Projection)
	{
		long Zone;
		m_pWGS84Projection->CalcZone(LAT_OFF*PI/180.0,LONG_OFF*PI/180.0,&Zone);

		m_pWGS84Projection->SetZoneStep(6);
		m_pWGS84Projection->SetZone(Zone);
		char szHemisphere=LAT_OFF>0?'N':'S';
		m_pWGS84Projection->SetHemisphere(szHemisphere);
	}

	//����xyZ��XY��ת����ϵ
	CalculateAntiRPC();
	hRet=S_OK;
EXIT:
	if(hRet==S_FALSE)
	{
		COutput::OutputFileOpenFailed(pszRPC);
	}
	fclose(fp);

	return hRet;
}

HRESULT CRPCParm::Ground2Image(double latitude, double longitude, double height, double *px, double *py)
{
	double P=(latitude-LAT_OFF)/LAT_SCALE;
	double L=(longitude-LONG_OFF)/LONG_SCALE;
	double H=(height-HEIGHT_OFF)/HEIGHT_SCALE;

	double NX=(SAMP_NUM_COEFF[0]+SAMP_NUM_COEFF[1]*L+SAMP_NUM_COEFF[2]*P+SAMP_NUM_COEFF[3]*H+SAMP_NUM_COEFF[4]*L*P+SAMP_NUM_COEFF[5]*L*H+SAMP_NUM_COEFF[6]*P*H+SAMP_NUM_COEFF[7]*L*L+SAMP_NUM_COEFF[8]*P*P+SAMP_NUM_COEFF[9]*H*H+SAMP_NUM_COEFF[10]*P*L*H+SAMP_NUM_COEFF[11]*L*L*L+SAMP_NUM_COEFF[12]*L*P*P+SAMP_NUM_COEFF[13]*L*H*H+SAMP_NUM_COEFF[14]*L*L*P+SAMP_NUM_COEFF[15]*P*P*P+SAMP_NUM_COEFF[16]*P*H*H+SAMP_NUM_COEFF[17]*L*L*H+SAMP_NUM_COEFF[18]*P*P*H+SAMP_NUM_COEFF[19]*H*H*H)/
			  (SAMP_DEN_COEFF[0]+SAMP_DEN_COEFF[1]*L+SAMP_DEN_COEFF[2]*P+SAMP_DEN_COEFF[3]*H+SAMP_DEN_COEFF[4]*L*P+SAMP_DEN_COEFF[5]*L*H+SAMP_DEN_COEFF[6]*P*H+SAMP_DEN_COEFF[7]*L*L+SAMP_DEN_COEFF[8]*P*P+SAMP_DEN_COEFF[9]*H*H+SAMP_DEN_COEFF[10]*P*L*H+SAMP_DEN_COEFF[11]*L*L*L+SAMP_DEN_COEFF[12]*L*P*P+SAMP_DEN_COEFF[13]*L*H*H+SAMP_DEN_COEFF[14]*L*L*P+SAMP_DEN_COEFF[15]*P*P*P+SAMP_DEN_COEFF[16]*P*H*H+SAMP_DEN_COEFF[17]*L*L*H+SAMP_DEN_COEFF[18]*P*P*H+SAMP_DEN_COEFF[19]*H*H*H);
	double NY=(LINE_NUM_COEFF[0]+LINE_NUM_COEFF[1]*L+LINE_NUM_COEFF[2]*P+LINE_NUM_COEFF[3]*H+LINE_NUM_COEFF[4]*L*P+LINE_NUM_COEFF[5]*L*H+LINE_NUM_COEFF[6]*P*H+LINE_NUM_COEFF[7]*L*L+LINE_NUM_COEFF[8]*P*P+LINE_NUM_COEFF[9]*H*H+LINE_NUM_COEFF[10]*P*L*H+LINE_NUM_COEFF[11]*L*L*L+LINE_NUM_COEFF[12]*L*P*P+LINE_NUM_COEFF[13]*L*H*H+LINE_NUM_COEFF[14]*L*L*P+LINE_NUM_COEFF[15]*P*P*P+LINE_NUM_COEFF[16]*P*H*H+LINE_NUM_COEFF[17]*L*L*H+LINE_NUM_COEFF[18]*P*P*H+LINE_NUM_COEFF[19]*H*H*H)/
			  (LINE_DEN_COEFF[0]+LINE_DEN_COEFF[1]*L+LINE_DEN_COEFF[2]*P+LINE_DEN_COEFF[3]*H+LINE_DEN_COEFF[4]*L*P+LINE_DEN_COEFF[5]*L*H+LINE_DEN_COEFF[6]*P*H+LINE_DEN_COEFF[7]*L*L+LINE_DEN_COEFF[8]*P*P+LINE_DEN_COEFF[9]*H*H+LINE_DEN_COEFF[10]*P*L*H+LINE_DEN_COEFF[11]*L*L*L+LINE_DEN_COEFF[12]*L*P*P+LINE_DEN_COEFF[13]*L*H*H+LINE_DEN_COEFF[14]*L*L*P+LINE_DEN_COEFF[15]*P*P*P+LINE_DEN_COEFF[16]*P*H*H+LINE_DEN_COEFF[17]*L*L*H+LINE_DEN_COEFF[18]*P*P*H+LINE_DEN_COEFF[19]*H*H*H);

	double X=NX*SAMP_SCALE+SAMP_OFF;
	double Y=NY*LINE_SCALE+LINE_OFF;
	//RPC�����е�һ�����ص�λ��Ϊ(1,1)
	X-=1;
	Y-=1;
	//�񷽸���
	*px=m_rotIMatrix[0][0]+m_rotIMatrix[0][1]*X+m_rotIMatrix[0][2]*Y;
	*py=m_rotIMatrix[1][0]+m_rotIMatrix[1][1]*X+m_rotIMatrix[1][2]*Y;
	/**px=X;
	*py=Y;*/

	return S_OK;
}

HRESULT CRPCParm::Ground2Image2(double LATD, double LATM, double LATS, double LONGD, double LONGM, double LONGS, double HEIGHT, double *px, double *py)
{
	double latitude=LATD+LATM/60.0+LATS/3600.0;
	double longitude=LONGD+LONGM/60.0+LONGS/3600.0;

	return Ground2Image(latitude,longitude,HEIGHT,px,py);
}

HRESULT CRPCParm::Ground2Image3(double X, double Y, double Z, double *px, double *py)
{
	/*if(m_pTMProjection==NULL)
	{
		return S_FALSE;
	}*/
	if(m_pWGS84Projection==NULL)
	{
		return S_FALSE;
	}

	double latitude,longitude,height;
	height=Z;
	//m_pTMProjection->TM_Convert_To_Geodetic(X,Y,&LAT,&LONG);
	m_pWGS84Projection->Map2Geodetic(X,Y,&latitude,&longitude);
	latitude=latitude/PI*180.0;
	longitude=longitude/PI*180.0;

	return Ground2Image(latitude,longitude,height,px,py);
}

HRESULT CRPCParm::CalculateImageAdjustMatrix(double *pGCPX, double *pGCPY, double *pGCPZ, double *pGCPIX, double *pGCPIY, int nGCPNum)
{
	if(nGCPNum<=0)
	{
		return S_FALSE;
	}
	//ȡ�������񷽸���ϵ��
	m_rotIMatrix[0][0]=0.0; m_rotIMatrix[0][1]=1.0; m_rotIMatrix[0][2]=0.0;
	m_rotIMatrix[1][0]=0.0; m_rotIMatrix[1][1]=0.0; m_rotIMatrix[1][2]=1.0;

	double* pSrcIX=new double[nGCPNum];
	double* pSrcIY=new double[nGCPNum];
	
	for(int i=0;i<nGCPNum;i++)
	{
		Ground2Image3(pGCPX[i],pGCPY[i],pGCPZ[i],&pSrcIX[i],&pSrcIY[i]);
	}

	if(nGCPNum==1)
	{
		m_rotIMatrix[0][0]=pGCPIX[0]-pSrcIX[0];
		m_rotIMatrix[0][1]=1.0;
		m_rotIMatrix[0][2]=0.0;
		m_rotIMatrix[1][0]=pGCPIY[0]-pSrcIY[0];
		m_rotIMatrix[1][1]=0.0;
		m_rotIMatrix[1][2]=1.0;
	}
	else if(nGCPNum==2)
	{
		Affine1(pSrcIX,pSrcIY,pGCPIX,pGCPIY,nGCPNum,m_rotIMatrix);
	}
	else
	{
		double Error=0.0;
		CalcPolyTransPara(pSrcIX,pSrcIY,pGCPIX,pGCPIY,nGCPNum,m_rotIMatrix,&Error);
	}

	delete [] pSrcIX; pSrcIX=NULL;
	delete [] pSrcIY; pSrcIY=NULL;

	return S_OK;
}

HRESULT CRPCParm::CalculateGroundAdjustMatrix(CDEM* pDEM, double *pGCPX, double *pGCPY, double *pGCPZ, double *pGCPIX, double *pGCPIY, int nGCPNum)
{
	if(nGCPNum<=0)
	{
		return S_FALSE;
	}
	//ȡ�������﷽����ϵ��
	m_rotGMatrix[0][0]=0.0; m_rotGMatrix[0][1]=1.0; m_rotGMatrix[0][2]=0.0;
	m_rotGMatrix[1][0]=0.0; m_rotGMatrix[1][1]=0.0; m_rotGMatrix[1][2]=1.0;

	double* pSrcGX=new double[nGCPNum];
	double* pSrcGY=new double[nGCPNum];
	double* pSrcGZ=new double[nGCPNum];
	
	for(int i=0;i<nGCPNum;i++)
	{
		Image2Ground3(pGCPIX[i],pGCPIY[i],pDEM,&pSrcGX[i],&pSrcGY[i],&pSrcGZ[i]);
	}

	if(nGCPNum==1)
	{
		m_rotGMatrix[0][0]=pGCPX[0]-pSrcGX[0];
		m_rotGMatrix[0][1]=1.0;
		m_rotGMatrix[0][2]=0.0;
		m_rotGMatrix[1][0]=pGCPY[0]-pSrcGY[0];
		m_rotGMatrix[1][1]=0.0;
		m_rotGMatrix[1][2]=1.0;
	}
	else if(nGCPNum==2)
	{
		Affine1(pSrcGX,pSrcGY,pGCPX,pGCPY,nGCPNum,m_rotGMatrix);
	}
	else
	{
		double Error=0.0;
		CalcPolyTransPara(pSrcGX,pSrcGY,pGCPX,pGCPY,nGCPNum,m_rotGMatrix,&Error);
	}

	delete [] pSrcGX; pSrcGX=NULL;
	delete [] pSrcGY; pSrcGY=NULL;
	delete [] pSrcGZ; pSrcGZ=NULL;

	return S_OK;
}

void CRPCParm::Affine1(double *pSrcX, double *pSrcY, 
				   double *pDstX, double *pDstY, 
				   int nNum, 
				   double rotMatrix[][3])
{
	double gxt,gyt;
	double gxm,gym,b1,b2,b3;
	double *xm,*ym,*xt,*yt;
	int i;

	xm=(double *)calloc(nNum,sizeof(double));
	ym=(double *)calloc(nNum,sizeof(double));
	xt=(double *)calloc(nNum,sizeof(double));
	yt=(double *)calloc(nNum,sizeof(double));

	for(i=0;i<nNum;i++) 
	{
		*(xm+i)=*(pDstX+i);
		*(ym+i)=*(pDstY+i);
		*(xt+i)=*(pSrcX+i);
		*(yt+i)=*(pSrcY+i);
	}

	gxt=gyt=(double)0;
	for(i=0;i<nNum;i++) 
	{
		gxt+=*(xt+i);
		gyt+=*(yt+i);
	}
	gxt/=nNum;        gyt/=nNum;

	for(i=0;i<nNum;i++) 
	{
		*(xt+i)-=gxt;
		*(yt+i)-=gyt;
	}

	gxm=(double)0; gym=(double)0;
	for(i=0;i<nNum;i++) 
	{
		gxm+=*(xm+i);
		gym+=*(ym+i);
	}
	gxm/=nNum; gym/=nNum;

	for(i=0;i<nNum;i++) 
	{
		*(xm+i)-=gxm;
		*(ym+i)-=gym;
	}
	 
	b1=b2=b3=(double)0.0;
	for(i=0;i<nNum;i++) 
	{
		b1+=*(xt+i)* *(xt+i)+ *(yt+i)* *(yt+i);
		b2+=*(xm+i)* *(xt+i)+ *(ym+i)* *(yt+i);
		b3+=*(xt+i)* *(ym+i)- *(xm+i)* *(yt+i);
	}
	
	b2=b2/b1;
	b3=b3/b1;
	
	rotMatrix[0][0]=gxm-b2*gxt+b3*gyt;
	rotMatrix[0][1]=b2;
	rotMatrix[0][2]=-b3;
	rotMatrix[1][0]=gym-b3*gxt-b2*gyt;
	rotMatrix[1][1]=b3;
	rotMatrix[1][2]=b2;
	
	free(xm);	free(ym);
	free(xt);	free(yt);
}

BOOL CRPCParm::CalcPolyTransPara(double *pSrcX, double *pSrcY, 
							double *pDstX, double *pDstY, 
							int nNum, 
							double rotMatrix[][3],
							double* pError)
{
	double a[3], ab[3], aa[9], b;

	memset(aa,0,sizeof(double)*9);
	memset(ab,0,sizeof(double)*3);
	int i=0;
	for(i=0; i<nNum; i++)	
	{
		*(a+0) = 1;
		*(a+1) = pSrcX[i];
		*(a+2) = pSrcY[i];

		b = pDstX[i];
		dnrml(a,3,b,aa,ab);
	}
	dsolve(aa, ab, rotMatrix[0],3,3);

	memset(aa,0,sizeof(double)*9);
	memset(ab,0,sizeof(double)*3);
	for(i=0;i<nNum;i++)	
	{
		*(a+0) = 1;
		*(a+1) = pSrcX[i];
		*(a+2) = pSrcY[i];

		b = pDstY[i];
		dnrml(a,3,b,aa,ab);
	}
	dsolve(aa,ab, rotMatrix[1],3,3);

	*pError=0.0;
	double tempX,tempY,ErrorX,ErrorY,RMS;
	double SumErrorX=0.0,SumErrorY=0.0;
	for(i=0;i<nNum;i++)
	{
		tempX=rotMatrix[0][0]+rotMatrix[0][1]*pSrcX[i]+rotMatrix[0][2]*pSrcY[i];
		tempY=rotMatrix[1][0]+rotMatrix[1][1]*pSrcX[i]+rotMatrix[1][2]*pSrcY[i];
		
		ErrorX=tempX-pDstX[i];
		ErrorY=tempY-pDstY[i];
		
		tempX=ErrorX*ErrorX;
		tempY=ErrorY*ErrorY;

		RMS=sqrt(tempX+tempY);

		SumErrorX=SumErrorX+tempX;
		SumErrorY=SumErrorY+tempY;
	}
	*pError=sqrt(SumErrorX/nNum+SumErrorY/nNum);

	return TRUE;
}

int CRPCParm::dnrml (double* aa,int n,double bb,double* a,double* b)
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

void CRPCParm::dsolve (double* a,double* b,double* x,int n,int wide)
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

void CRPCParm::dldltban1 (double* a,double* d,double* l,int n,int wide)
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

void CRPCParm::dldltban2 (double* l,double* d,double* b,double* x,int n,int wide)
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

HRESULT CRPCParm::Image2Ground(double IX, double IY, 
						CDEM* pDEM, 
						double *pLAT, double *pLONG, double *pHEIGHT)
{
	/*if(m_pTMProjection==NULL)
	{
		return S_FALSE;
	}*/
	if(m_pWGS84Projection==NULL)
	{
		return S_FALSE;
	}

	//RPC�����е�һ�����ص�λ��Ϊ(1,1)
	IX+=1;
	IY+=1;

	double lat,lon,Height;
	double H0=0.0;
	pDEM->GetAverageAltitude(&H0);
	int nCount=0;
	double P,L,H;
	double NX,NY;
	do
	{
		Height=H0;
		P=(IY-LINE_OFF)/LINE_SCALE;
		L=(IX-SAMP_OFF)/SAMP_SCALE;
		H=(Height-HEIGHT_OFF)/HEIGHT_SCALE;

		NX=(ANTI_SAMP_NUM_COEFF[0]+ANTI_SAMP_NUM_COEFF[1]*L+ANTI_SAMP_NUM_COEFF[2]*P+ANTI_SAMP_NUM_COEFF[3]*H+ANTI_SAMP_NUM_COEFF[4]*L*P+ANTI_SAMP_NUM_COEFF[5]*L*H+ANTI_SAMP_NUM_COEFF[6]*P*H+ANTI_SAMP_NUM_COEFF[7]*L*L+ANTI_SAMP_NUM_COEFF[8]*P*P+ANTI_SAMP_NUM_COEFF[9]*H*H+ANTI_SAMP_NUM_COEFF[10]*P*L*H+ANTI_SAMP_NUM_COEFF[11]*L*L*L+ANTI_SAMP_NUM_COEFF[12]*L*P*P+ANTI_SAMP_NUM_COEFF[13]*L*H*H+ANTI_SAMP_NUM_COEFF[14]*L*L*P+ANTI_SAMP_NUM_COEFF[15]*P*P*P+ANTI_SAMP_NUM_COEFF[16]*P*H*H+ANTI_SAMP_NUM_COEFF[17]*L*L*H+ANTI_SAMP_NUM_COEFF[18]*P*P*H+ANTI_SAMP_NUM_COEFF[19]*H*H*H)/
				  (ANTI_SAMP_DEN_COEFF[0]+ANTI_SAMP_DEN_COEFF[1]*L+ANTI_SAMP_DEN_COEFF[2]*P+ANTI_SAMP_DEN_COEFF[3]*H+ANTI_SAMP_DEN_COEFF[4]*L*P+ANTI_SAMP_DEN_COEFF[5]*L*H+ANTI_SAMP_DEN_COEFF[6]*P*H+ANTI_SAMP_DEN_COEFF[7]*L*L+ANTI_SAMP_DEN_COEFF[8]*P*P+ANTI_SAMP_DEN_COEFF[9]*H*H+ANTI_SAMP_DEN_COEFF[10]*P*L*H+ANTI_SAMP_DEN_COEFF[11]*L*L*L+ANTI_SAMP_DEN_COEFF[12]*L*P*P+ANTI_SAMP_DEN_COEFF[13]*L*H*H+ANTI_SAMP_DEN_COEFF[14]*L*L*P+ANTI_SAMP_DEN_COEFF[15]*P*P*P+ANTI_SAMP_DEN_COEFF[16]*P*H*H+ANTI_SAMP_DEN_COEFF[17]*L*L*H+ANTI_SAMP_DEN_COEFF[18]*P*P*H+ANTI_SAMP_DEN_COEFF[19]*H*H*H);
		NY=(ANTI_LINE_NUM_COEFF[0]+ANTI_LINE_NUM_COEFF[1]*L+ANTI_LINE_NUM_COEFF[2]*P+ANTI_LINE_NUM_COEFF[3]*H+ANTI_LINE_NUM_COEFF[4]*L*P+ANTI_LINE_NUM_COEFF[5]*L*H+ANTI_LINE_NUM_COEFF[6]*P*H+ANTI_LINE_NUM_COEFF[7]*L*L+ANTI_LINE_NUM_COEFF[8]*P*P+ANTI_LINE_NUM_COEFF[9]*H*H+ANTI_LINE_NUM_COEFF[10]*P*L*H+ANTI_LINE_NUM_COEFF[11]*L*L*L+ANTI_LINE_NUM_COEFF[12]*L*P*P+ANTI_LINE_NUM_COEFF[13]*L*H*H+ANTI_LINE_NUM_COEFF[14]*L*L*P+ANTI_LINE_NUM_COEFF[15]*P*P*P+ANTI_LINE_NUM_COEFF[16]*P*H*H+ANTI_LINE_NUM_COEFF[17]*L*L*H+ANTI_LINE_NUM_COEFF[18]*P*P*H+ANTI_LINE_NUM_COEFF[19]*H*H*H)/
				  (ANTI_LINE_DEN_COEFF[0]+ANTI_LINE_DEN_COEFF[1]*L+ANTI_LINE_DEN_COEFF[2]*P+ANTI_LINE_DEN_COEFF[3]*H+ANTI_LINE_DEN_COEFF[4]*L*P+ANTI_LINE_DEN_COEFF[5]*L*H+ANTI_LINE_DEN_COEFF[6]*P*H+ANTI_LINE_DEN_COEFF[7]*L*L+ANTI_LINE_DEN_COEFF[8]*P*P+ANTI_LINE_DEN_COEFF[9]*H*H+ANTI_LINE_DEN_COEFF[10]*P*L*H+ANTI_LINE_DEN_COEFF[11]*L*L*L+ANTI_LINE_DEN_COEFF[12]*L*P*P+ANTI_LINE_DEN_COEFF[13]*L*H*H+ANTI_LINE_DEN_COEFF[14]*L*L*P+ANTI_LINE_DEN_COEFF[15]*P*P*P+ANTI_LINE_DEN_COEFF[16]*P*H*H+ANTI_LINE_DEN_COEFF[17]*L*L*H+ANTI_LINE_DEN_COEFF[18]*P*P*H+ANTI_LINE_DEN_COEFF[19]*H*H*H);

		lon=NX*LONG_SCALE+LONG_OFF;
		lat=NY*LAT_SCALE+LAT_OFF;
		double X,Y;
		//m_pTMProjection->TM_Convert_From_Geodetic(lat*PI/180.0,lon*PI/180.0,&X,&Y);
		m_pWGS84Projection->Geodetic2Map(lat*PI/180.0,lon*PI/180.0,&X,&Y);
		//�﷽����
		double GX=m_rotGMatrix[0][0]+m_rotGMatrix[0][1]*X+m_rotGMatrix[0][2]*Y;
		double GY=m_rotGMatrix[1][0]+m_rotGMatrix[1][1]*X+m_rotGMatrix[1][2]*Y;
		if(m_pDEMProjection)
		{
			double tempX,tempY,tempZ;
			Covert2DEMProjection(GX,GY,0,&tempX,&tempY,&tempZ);

			pDEM->GetAltitude(tempX,tempY,&H0,1);
		}
		else
		{
			pDEM->GetAltitude(GX,GY,&H0,1);
		}
		if(fabs(H0-INVALID_ALTITUDE)<1e-5)
		{
			break;
		}

	}
	while(nCount<10&&fabs(H0-Height)>1.0);
	/*double NX=(IX-SAMP_OFF)/SAMP_SCALE;
	double NY=(IY-LINE_OFF)/LINE_SCALE;

	double P,L,H;
	double H0=0.0;
	pDEM->GetAverageAltitude(&H0);

	int nCount=0;
	do
	{
		H=H0;

		double NH=(H-HEIGHT_OFF)/HEIGHT_SCALE;

		double a0=SAMP_NUM_COEFF[0],a1=SAMP_NUM_COEFF[1],a2=SAMP_NUM_COEFF[2],a3=SAMP_NUM_COEFF[3];
		double b0=SAMP_DEN_COEFF[0],b1=SAMP_DEN_COEFF[1],b2=SAMP_DEN_COEFF[2],b3=SAMP_DEN_COEFF[3];
		double c0=LINE_NUM_COEFF[0],c1=LINE_NUM_COEFF[1],c2=LINE_NUM_COEFF[2],c3=LINE_NUM_COEFF[3];
		double d0=LINE_DEN_COEFF[0],d1=LINE_DEN_COEFF[1],d2=LINE_DEN_COEFF[2],d3=LINE_DEN_COEFF[3];

		double A0=a2-NX*b2,A1=a3-NX*b3,A2=a0-NX*b0,A3=a1-NX*b1;
		double B0=c2-NY*d2,B1=c3-NY*d3,B2=c0-NY*d0,B3=c1-NY*d1;

		double NP=(NH*A3*B1+A3*B2-NH*A1*B3-A2*B3)/(A0*B3-A3*B0);
		double NL=-(NP*(a2-NX*b2)+NH*(a3-NX*b3)+a0-NX*b0)/(a1-NX*b1);

		P=NP*LAT_SCALE+LAT_OFF;
		L=NL*LONG_SCALE+LONG_OFF;

		double X,Y;
		m_pTMProjection->TM_Convert_From_Geodetic(P*PI/180.0,L*PI/180.0,&X,&Y);
		//�﷽����
		double GX=m_rotGMatrix[0][0]+m_rotGMatrix[0][1]*X+m_rotGMatrix[0][2]*Y;
		double GY=m_rotGMatrix[1][0]+m_rotGMatrix[1][1]*X+m_rotGMatrix[1][2]*Y;
		hRes=pDEM->GetAltitude(GX,GY,&H0,1);
		if(fabs(H0-INVALID_ALTITUDE)<1e-5)
		{
			break;
		}
		
		nCount++;
	}
	while(nCount<10&&fabs(H0-H)>1.0);
	*/
	//�﷽����
	double X,Y;
	//m_pTMProjection->TM_Convert_From_Geodetic(lat*PI/180.0,lon*PI/180.0,&X,&Y);
	m_pWGS84Projection->Geodetic2Map(lat*PI/180.0,lon*PI/180.0,&X,&Y);
	double GX=m_rotGMatrix[0][0]+m_rotGMatrix[0][1]*X+m_rotGMatrix[0][2]*Y;
	double GY=m_rotGMatrix[1][0]+m_rotGMatrix[1][1]*X+m_rotGMatrix[1][2]*Y;
	//m_pTMProjection->TM_Convert_To_Geodetic(GX,GY,&P,&L);
	m_pWGS84Projection->Map2Geodetic(GX,GY,&P,&L);
	P=P*180.0/PI;
	L=L*180.0/PI;

	*pLAT=P;
	*pLONG=L;
	*pHEIGHT=Height;

	return S_OK;
}

HRESULT CRPCParm::Image2Ground3(double IX, double IY, 
						 CDEM *pDEM, 
						 double *pX, double *pY, double *pZ)
{
	/*if(m_pTMProjection==NULL)
	{
		return S_FALSE;
	}*/
	if(m_pWGS84Projection==NULL)
	{
		return S_FALSE;
	}

	double latitude,longitude,height;
	Image2Ground(IX,IY,pDEM,&latitude,&longitude,&height);
	//m_pTMProjection->TM_Convert_From_Geodetic(LAT*PI/180.0,LONG*PI/180.0,pX,pY);
	m_pWGS84Projection->Geodetic2Map(latitude*PI/180.0,longitude*PI/180.0,pX,pY);
	*pZ=height;

	return S_OK;
}

HRESULT CRPCParm::Image2Ground2(double IX, double IY, 
						CDEM *pDEM, 
						double *pLATD, double *pLATM, double *pLATS, 
						double *pLONGD, double *pLONGM, double *pLONGS, 
						double *pHEIGHT)
{
	double latitude,longitude,height;
	Image2Ground(IX,IY,pDEM,&latitude,&longitude,&height);

	*pLATD=(int)latitude;
	latitude=(latitude-*pLATD)*60.0;
	*pLATM=(int)(latitude);
	latitude=(latitude-*pLATM)*60.0;
	*pLATS=latitude;

	*pLONGD=(int)longitude;
	longitude=(longitude-*pLONGD)*60.0;
	*pLONGM=(int)(longitude);
	longitude=(longitude-*pLONGM)*60.0;
	*pLONGS=longitude;

	*pHEIGHT=height;

	return S_OK;
}

void CRPCParm::CalculateAntiRPC()
{
	//���RPCģ�͹�����Ƹ���
	const double fInterval=__min(LAT_SCALE*2/25,LONG_SCALE*2/25);
	const int nLayerNum=10;
	int nRows=(int)(LAT_SCALE*2/fInterval+0.5);
	int nCols=(int)(LONG_SCALE*2/fInterval+0.5);
	const double fAltitudeMin=HEIGHT_OFF-HEIGHT_SCALE;
	const double fAltitudeMax=HEIGHT_OFF+HEIGHT_SCALE;

	int nGCPNum=nRows*nCols*nLayerNum;
	double* pLat=new double[nGCPNum];
	double* pLon=new double[nGCPNum];
	double* pHeight=new double[nGCPNum];
	double* pSamp=new double[nGCPNum];
	double* pLine=new double[nGCPNum];

	int nIndex=0;
	double LAT0=LAT_OFF-LAT_SCALE;
	double LONG0=LONG_OFF-LONG_SCALE;
	for(int nLayer=0;nLayer<nLayerNum;nLayer++)
	{
		double fAltitude=fAltitudeMin+(fAltitudeMax-fAltitudeMin)/nLayerNum*nLayer;
		for(int i=0;i<nRows;i++)
		{
			for(int j=0;j<nCols;j++)
			{
				pLat[nIndex]=LAT0+i*fInterval;
				pLon[nIndex]=LONG0+j*fInterval;
				pHeight[nIndex]=fAltitude;

				Ground2Image(pLat[nIndex],pLon[nIndex],fAltitude,&pSamp[nIndex],&pLine[nIndex]);

				nIndex++;
			}
		}
	}
	//����һ��
	int i=0;
	for(i=0;i<nGCPNum;i++)
	{
		pLat[i]=(pLat[i]-LAT_OFF)/LAT_SCALE;
		pLon[i]=(pLon[i]-LONG_OFF)/LONG_SCALE;
		pHeight[i]=(pHeight[i]-HEIGHT_OFF)/HEIGHT_SCALE;
		pLine[i]=(pLine[i]-LINE_OFF)/LINE_SCALE;
		pSamp[i]=(pSamp[i]-SAMP_OFF)/SAMP_SCALE;
	}
	//������RPC����
	double fLCoef[39],fSCoef[39];
	double a[39], ab[39], aa[1521], b;	
	double* P=pLine;
	double* L=pSamp;
	double* H=pHeight;
	double* X=pLon;
	double* Y=pLat;
	//�з���
	memset(aa,0,sizeof(double)*1521);
	memset(ab,0,sizeof(double)*39);
	for(i=0; i<nGCPNum; i++)	
	{
		*(a+ 0) = 1;
		*(a+ 1) = L[i];
		*(a+ 2) = P[i];
		*(a+ 3) = H[i];
		*(a+ 4) = L[i]*P[i];
		*(a+ 5) = L[i]*H[i];
		*(a+ 6) = P[i]*H[i];
		*(a+ 7) = L[i]*L[i];
		*(a+ 8) = P[i]*P[i];
		*(a+ 9) = H[i]*H[i];
		*(a+10) = P[i]*L[i]*H[i];
		*(a+11) = L[i]*L[i]*L[i];
		*(a+12) = L[i]*P[i]*P[i];
		*(a+13) = L[i]*H[i]*H[i];
		*(a+14) = L[i]*L[i]*P[i];
		*(a+15) = P[i]*P[i]*P[i];
		*(a+16) = P[i]*H[i]*H[i];
		*(a+17) = L[i]*L[i]*H[i];
		*(a+18) = P[i]*P[i]*H[i];
		*(a+19) = H[i]*H[i]*H[i];
		
		*(a+20) = -L[i]*Y[i];
		*(a+21) = -P[i]*Y[i];
		*(a+22) = -H[i]*Y[i];
		*(a+23) = -L[i]*P[i]*Y[i];
		*(a+24) = -L[i]*H[i]*Y[i];
		*(a+25) = -P[i]*H[i]*Y[i];
		*(a+26) = -L[i]*L[i]*Y[i];
		*(a+27) = -P[i]*P[i]*Y[i];
		*(a+28) = -H[i]*H[i]*Y[i];
		*(a+29) = -P[i]*L[i]*H[i]*Y[i];
		*(a+30) = -L[i]*L[i]*L[i]*Y[i];
		*(a+31) = -L[i]*P[i]*P[i]*Y[i];
		*(a+32) = -L[i]*H[i]*H[i]*Y[i];
		*(a+33) = -L[i]*L[i]*P[i]*Y[i];
		*(a+34) = -P[i]*P[i]*P[i]*Y[i];
		*(a+35) = -P[i]*H[i]*H[i]*Y[i];
		*(a+36) = -L[i]*L[i]*H[i]*Y[i];
		*(a+37) = -P[i]*P[i]*H[i]*Y[i];
		*(a+38) = -H[i]*H[i]*H[i]*Y[i];

		b = Y[i];
		dnrml(a,39,b,aa,ab);
	}
	dsolve(aa,ab, fLCoef, 39, 39);
	//�з���
	memset(aa,0,sizeof(double)*1521);
	memset(ab,0,sizeof(double)*39);
	for(i=0; i<nGCPNum; i++)	
	{
		*(a+ 0) = 1;
		*(a+ 1) = L[i];
		*(a+ 2) = P[i];
		*(a+ 3) = H[i];
		*(a+ 4) = L[i]*P[i];
		*(a+ 5) = L[i]*H[i];
		*(a+ 6) = P[i]*H[i];
		*(a+ 7) = L[i]*L[i];
		*(a+ 8) = P[i]*P[i];
		*(a+ 9) = H[i]*H[i];
		*(a+10) = P[i]*L[i]*H[i];
		*(a+11) = L[i]*L[i]*L[i];
		*(a+12) = L[i]*P[i]*P[i];
		*(a+13) = L[i]*H[i]*H[i];
		*(a+14) = L[i]*L[i]*P[i];
		*(a+15) = P[i]*P[i]*P[i];
		*(a+16) = P[i]*H[i]*H[i];
		*(a+17) = L[i]*L[i]*H[i];
		*(a+18) = P[i]*P[i]*H[i];
		*(a+19) = H[i]*H[i]*H[i];
		
		*(a+20) = -L[i]*X[i];
		*(a+21) = -P[i]*X[i];
		*(a+22) = -H[i]*X[i];
		*(a+23) = -L[i]*P[i]*X[i];
		*(a+24) = -L[i]*H[i]*X[i];
		*(a+25) = -P[i]*H[i]*X[i];
		*(a+26) = -L[i]*L[i]*X[i];
		*(a+27) = -P[i]*P[i]*X[i];
		*(a+28) = -H[i]*H[i]*X[i];
		*(a+29) = -P[i]*L[i]*H[i]*X[i];
		*(a+30) = -L[i]*L[i]*L[i]*X[i];
		*(a+31) = -L[i]*P[i]*P[i]*X[i];
		*(a+32) = -L[i]*H[i]*H[i]*X[i];
		*(a+33) = -L[i]*L[i]*P[i]*X[i];
		*(a+34) = -P[i]*P[i]*P[i]*X[i];
		*(a+35) = -P[i]*H[i]*H[i]*X[i];
		*(a+36) = -L[i]*L[i]*H[i]*X[i];
		*(a+37) = -P[i]*P[i]*H[i]*X[i];
		*(a+38) = -H[i]*H[i]*H[i]*X[i];

		b = X[i];
		dnrml(a,39,b,aa,ab);
	}
	dsolve(aa,ab, fSCoef, 39, 39);

	L=NULL; P=NULL; H=NULL; X=NULL; Y=NULL;
	delete [] pLat; pLat=NULL;
	delete [] pLon; pLon=NULL;
	delete [] pHeight; pHeight=NULL;
	delete [] pSamp; pSamp=NULL;
	delete [] pLine; pLine=NULL;
	//��¼��RPC����
	for(i=0;i<20;i++)
	{
		ANTI_LINE_NUM_COEFF[i]=fLCoef[i];
	}
	ANTI_LINE_DEN_COEFF[0]=1.0;
	for(i=20;i<39;i++)
	{
		ANTI_LINE_DEN_COEFF[i-20+1]=fLCoef[i];
	}

	for(i=0;i<20;i++)
	{
		ANTI_SAMP_NUM_COEFF[i]=fSCoef[i];
	}
	ANTI_SAMP_DEN_COEFF[0]=1.0;
	for(i=20;i<39;i++)
	{
		ANTI_SAMP_DEN_COEFF[i-20+1]=fSCoef[i];
	}
}

HRESULT CRPCParm::SetDEMProjection(CProjection *pProjection)
{
	m_pDEMProjection=pProjection;

	return S_OK;
}

void CRPCParm::Covert2DEMProjection(double X84, double Y84, double Z84, double *pX, double *pY, double *pZ)
{
	if(m_pDEMProjection==NULL)
	{
		*pX=X84;
		*pY=Y84;
		*pZ=Z84;
	}

	double lfLatitude,lfLongitude;
	m_pWGS84Projection->Map2Geodetic(X84,Y84,&lfLatitude,&lfLongitude);
	double GX84,GY84,GZ84;
	m_pWGS84Projection->Geodetic2Geocentric(lfLatitude,lfLongitude,Z84,&GX84,&GY84,&GZ84);

	double GX,GY,GZ;
	m_pDEMProjection->GeocentricFromWGS84(GX84,GY84,GZ84,&GX,&GY,&GZ);
	m_pDEMProjection->Geocentric2Geodetic(GX,GY,GZ,&lfLatitude,&lfLongitude,pZ);
	m_pDEMProjection->Geodetic2Map(lfLatitude,lfLongitude,pX,pY);
}

HRESULT CRPCParm::GetLAT_OFF(double *pLAT_OFF)
{
	*pLAT_OFF=LAT_OFF;

	return S_OK;
}

HRESULT CRPCParm::GetLONG_OFF(double *pLONG_OFF)
{
	*pLONG_OFF=LONG_OFF;

	return S_OK;
}