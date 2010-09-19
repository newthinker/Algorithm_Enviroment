// Dimap.cpp: implementation of the CDimap class.
//
//////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <gsl/gsl_linalg.h>		// gsl数学库
#include <gsl/gsl_blas.h>
#include "apfloat.h"
#include "apcplx.h"

#include "Dimap.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDimap::CDimap()
{
	m_Projection.CreateProject("WGS84");

	m_nRows=0;
	m_nCols=0;
	m_fT0=0;
	m_fLSP=0;
}

CDimap::~CDimap()
{

}

void CDimap::CreateRotMatrixPOK(double phi, double omega, double kappa, double rotMatrix[3][3])
{
#ifdef APFLOAT
	apfloat fPhi = phi;
	apfloat fOmega = omega;
	apfloat fKappa = kappa;
	apfloat tmp;

	tmp = cos(fPhi)*cos(fKappa) - sin(fPhi)*sin(fOmega)*sin(fKappa);
	tmp.prec(64);
	rotMatrix[0][0] = ap2double(tmp.ap);

	tmp = -cos(fPhi)*sin(fKappa)-sin(fPhi)*sin(fOmega)*cos(fKappa);
	rotMatrix[0][1] = ap2double(tmp.ap);

	tmp = -sin(fPhi)*cos(fOmega);
	rotMatrix[0][2] = ap2double(tmp.ap);

	tmp = cos(fOmega)*sin(fKappa);
	rotMatrix[1][0] = ap2double(tmp.ap);

	tmp = cos(fOmega)*cos(fKappa);
	rotMatrix[1][1] = ap2double(tmp.ap);

	tmp = -sin(fOmega);
	rotMatrix[1][2] = ap2double(tmp.ap);

	tmp = sin(fPhi)*cos(fKappa)+cos(fPhi)*sin(fOmega)*sin(fKappa);
	rotMatrix[2][0] = ap2double(tmp.ap);

	tmp = -sin(fPhi)*sin(fKappa)+cos(fPhi)*sin(fOmega)*cos(fKappa);
	rotMatrix[2][1] = ap2double(tmp.ap);

	tmp = cos(fPhi)*cos(fOmega);
	rotMatrix[2][2] = ap2double(tmp.ap);

#else
	rotMatrix[0][0]=cos(phi)*cos(kappa)-sin(phi)*sin(omega)*sin(kappa);//a1
	rotMatrix[0][1]=-cos(phi)*sin(kappa)-sin(phi)*sin(omega)*cos(kappa);//a2
	rotMatrix[0][2]=-sin(phi)*cos(omega);//a3

	rotMatrix[1][0]=cos(omega)*sin(kappa);//b1
	rotMatrix[1][1]=cos(omega)*cos(kappa);//b2
	rotMatrix[1][2]=-sin(omega);//b3

	rotMatrix[2][0]=sin(phi)*cos(kappa)+cos(phi)*sin(omega)*sin(kappa);//c1
	rotMatrix[2][1]=-sin(phi)*sin(kappa)+cos(phi)*sin(omega)*cos(kappa);//c2
	rotMatrix[2][2]=cos(phi)*cos(omega);//c3
#endif
}

void CDimap::CreateRotMatrixPOK2(double phi, double omega, double kappa, double rotMatrix[3][3])
{
	rotMatrix[0][0]=cos(phi)*cos(kappa);// cos锟秸★拷cos锟绞★拷
	rotMatrix[0][1]=-cos(phi)*sin(kappa);// -cos锟秸★拷sin锟绞★拷 
	rotMatrix[0][2]=-sin(phi);// -sin锟秸★拷

	rotMatrix[1][0]=cos(omega)*sin(kappa)-sin(omega)*sin(phi)*cos(kappa);// cos锟截★拷sin锟绞★拷 锟紺sin锟截★拷 sin锟秸★拷cos锟绞★拷 
	rotMatrix[1][1]=cos(omega)*cos(kappa)+sin(omega)*sin(phi)*sin(kappa);//cos锟截★拷cos锟绞★拷+ sin锟截★拷 sin锟秸★拷sin锟绞★拷 
	rotMatrix[1][2]=-sin(omega)*cos(phi);// -sin锟截★拷 cos锟秸★拷

	rotMatrix[2][0]=sin(omega)*sin(kappa)+cos(omega)*sin(phi)*cos(kappa);// sin锟截★拷sin锟绞★拷+ cos锟截★拷sin锟秸★拷cos锟绞★拷
	rotMatrix[2][1]=sin(omega)*cos(kappa)-cos(omega)*sin(phi)*sin(kappa);// sin锟截★拷cos锟绞★拷- cos锟截★拷sin锟秸★拷sin锟绞★拷
	rotMatrix[2][2]=cos(phi)*cos(omega);// cos锟秸★拷cos锟斤拷
}
double CDimap::ParseTime(char* pszTime)
{
/*	CString strYear=strTime.Left(4);
	CString strMonth=strTime.Mid(5,2);
	CString strDay=strTime.Mid(8,2);
	CString strHour=strTime.Mid(11,2);
	CString strMinute=strTime.Mid(14,2);
	CString strSecond=strTime.Right(strTime.GetLength()-strTime.ReverseFind(':')-1);

	int nYear=atoi(strYear.GetBuffer(0));
	int nMonth=atoi(strMonth.GetBuffer(0));
	int nDay=atoi(strDay.GetBuffer(0));
	int nHour=atoi(strHour.GetBuffer(0));
	int nMinute=atoi(strMinute.GetBuffer(0));
	double fSecond=atof(strSecond.GetBuffer(0));

	CTime tc(1970+nYear-m_nYear0,nMonth,nDay,nHour,nMinute,0);
	time_t time=tc.GetTime();
	
	return time+fSecond;*/
	
	return 0;
}

double CDimap::LineDate(int nLine)
{
	if(m_nSensorType==HJ1B_IRS)
	{
		return m_ScanFrameTime[nLine/m_ScanCamera.nScanFrame];
	}
	else
	{
		return m_fT0+m_fLSP*nLine;
	}
}

double CDimap::PixelDate(int nRow,int nCol)
{
	double lfScanTime0=LineDate(nRow);

	return lfScanTime0+nCol*m_ScanCamera.lfPixelTime;
}

CDimap::Ephemeris CDimap::EphemerisDate(double fDate)
{
	double PX=0;	double PY=0;	double PZ=0;
	double VX=0;	double VY=0;	double VZ=0;

	int i=0;
	for(i=0;i<m_Ephemeris.GetSize()-1&&i<=4;i++)
	{
		if(fDate>=m_Ephemeris[i].t&&fDate<m_Ephemeris[i+1].t)
		{
			double fCoef=(fDate-m_Ephemeris[i].t)/(m_Ephemeris[i+1].t-m_Ephemeris[i].t);

			PX=m_Ephemeris[i].P.x+(m_Ephemeris[i+1].P.x-m_Ephemeris[i].P.x)*fCoef;
			PY=m_Ephemeris[i].P.y+(m_Ephemeris[i+1].P.y-m_Ephemeris[i].P.y)*fCoef;
			PZ=m_Ephemeris[i].P.z+(m_Ephemeris[i+1].P.z-m_Ephemeris[i].P.z)*fCoef;

			VX=m_Ephemeris[i].V.x+(m_Ephemeris[i+1].V.x-m_Ephemeris[i].V.x)*fCoef;
			VY=m_Ephemeris[i].V.y+(m_Ephemeris[i+1].V.y-m_Ephemeris[i].V.y)*fCoef;
			VZ=m_Ephemeris[i].V.z+(m_Ephemeris[i+1].V.z-m_Ephemeris[i].V.z)*fCoef;

			Ephemeris ep;
			ep.P.x=PX; ep.P.y=PY; ep.P.z=PZ;
			ep.V.x=VX; ep.V.y=VY; ep.V.z=VZ;
			ep.t=fDate;

			return ep;
		}
	}
	for(i=m_Ephemeris.GetSize()-1;i>=m_Ephemeris.GetSize()-4;i--)
	{
		if(fDate>=m_Ephemeris[i-1].t&&fDate<m_Ephemeris[i].t)
		{
			double fCoef=(fDate-m_Ephemeris[i-1].t)/(m_Ephemeris[i].t-m_Ephemeris[i-1].t);

			PX=m_Ephemeris[i-1].P.x+(m_Ephemeris[i].P.x-m_Ephemeris[i-1].P.x)*fCoef;
			PY=m_Ephemeris[i-1].P.y+(m_Ephemeris[i].P.y-m_Ephemeris[i-1].P.y)*fCoef;
			PZ=m_Ephemeris[i-1].P.z+(m_Ephemeris[i].P.z-m_Ephemeris[i-1].P.z)*fCoef;

			VX=m_Ephemeris[i-1].V.x+(m_Ephemeris[i].V.x-m_Ephemeris[i-1].V.x)*fCoef;
			VY=m_Ephemeris[i-1].V.y+(m_Ephemeris[i].V.y-m_Ephemeris[i-1].V.y)*fCoef;
			VZ=m_Ephemeris[i-1].V.z+(m_Ephemeris[i].V.z-m_Ephemeris[i-1].V.z)*fCoef;

			Ephemeris ep;
			ep.P.x=PX; ep.P.y=PY; ep.P.z=PZ;
			ep.V.x=VX; ep.V.y=VY; ep.V.z=VZ;
			ep.t=fDate;

			return ep;
		}
	}

	for(i=0;i<m_Ephemeris.GetSize();i++)
	{
		double fCoef=1.0;
		for(int j=0;j<m_Ephemeris.GetSize();j++)
		{
			if(i==j)
			{
				continue;
			}
			fCoef=fCoef*(fDate-m_Ephemeris[j].t)/(m_Ephemeris[i].t-m_Ephemeris[j].t);
		}

		PX=PX+fCoef*m_Ephemeris[i].P.x;
		PY=PY+fCoef*m_Ephemeris[i].P.y;
		PZ=PZ+fCoef*m_Ephemeris[i].P.z;

		VX=VX+fCoef*m_Ephemeris[i].V.x;
		VY=VY+fCoef*m_Ephemeris[i].V.y;
		VZ=VZ+fCoef*m_Ephemeris[i].V.z;
	}

	Ephemeris ep;
	ep.P.x=PX; ep.P.y=PY; ep.P.z=PZ;
	ep.V.x=VX; ep.V.y=VY; ep.V.z=VZ;
	ep.t=fDate;

	return ep;
}

CDimap::Attitude CDimap::AttitudeDate(double fDate)
{
	int n0=-1,n1=-1;
	for(int i=0;i<m_Attitude.GetSize()-1;i++)
	{
		if(fDate>=m_Attitude[i].t&&fDate<=m_Attitude[i+1].t)
		{
			n0=i;
			n1=i+1;
			break;
		}
	}

	Attitude at;
	if(n0!=-1&&n1!=-1)
	{
		at.t=fDate;
		double tCoef=(at.t-m_Attitude[n0].t)/(m_Attitude[n1].t-m_Attitude[n0].t);

		at.PITCH=m_Attitude[n0].PITCH+(m_Attitude[n1].PITCH-m_Attitude[n0].PITCH)*tCoef;
		at.ROLL=m_Attitude[n0].ROLL+(m_Attitude[n1].ROLL-m_Attitude[n0].ROLL)*tCoef;
		at.YAW=m_Attitude[n0].YAW+(m_Attitude[n1].YAW-m_Attitude[n0].YAW)*tCoef;
	}

	return at;
}

void CDimap::pnrml(double *aa, int n, double bb, double *a, double *b, double p)
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

void CDimap::dnrml (double* aa,int n,double bb,double* a,double* b)
{
	register int  i,j;
				                          	
	for (i=0; i<n; i++) 
	{
 	     for (j=0; j<n-i; j++) 
		 {
	        *a += *aa * *(aa+j);
            a++; 
	     }
	     *b += *aa * bb;
	     b++; aa++;
	}
}

void CDimap::dsolve (double* a,double* b,double* x,int n,int wide)
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

void CDimap::dldltban1 (double* a,double* d,double* l,int n,int wide)
{
	int i,j,k,kk,km,m;
	double *ao,*aa,*co,*c;

	m = wide*(2*n+1-wide)/2;
	c =(double *)calloc ((m-wide),sizeof(double));

	ao=a; co=c; a +=wide;
	for (i=0; i<m-wide; i++) *c++ = *a++;
	c=co; a=ao;

	for (k=1; k<n; k++) 
	{
	   if (k<n-wide+2) kk=wide-1;
	   else kk--;
	
	   *d = *a++; aa=a;  a += kk;

	   if (k<n-wide+1) km=wide;
	   else km=n-k+1;

	   for (i=1; i<kk+1; i++) 
	   {
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

void CDimap::dldltban2 (double* l,double* d,double* b,double* x,int n,int wide)
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

HRESULT CDimap::Image2Geodetic(int nRow, int nCol, double fHeight, double *pLat, double *pLon, double *pAltitude)
{
	switch(m_nSensorType)
	{
	case HJ1A_CCD1:
	case HJ1A_CCD2:
	case HJ1A_HSI:
	case HJ1B_CCD1:
	case HJ1B_CCD2:
	case HJ1A_CCD:
	case HJ1B_CCD:
		{
			double X,Y,Z;
			//intersect attitude and emphemeris
			double fDate=LineDate(nRow);
			Ephemeris ep=EphemerisDate(fDate);
			Attitude at=AttitudeDate(fDate);

			double Xs,Ys,Zs;
			m_Projection.Geodetic2Map(ep.P.x,ep.P.y,&Xs,&Ys);
			Zs=ep.P.z;

			int nCameraIndex=0;
			if(m_nSensorType==HJ1A_CCD1||m_nSensorType==HJ1A_CCD2||
			   m_nSensorType==HJ1B_CCD1||m_nSensorType==HJ1B_CCD2||
			   m_nSensorType==HJ1A_HSI)
			{
				nCameraIndex=m_nSensorType;
			}
			else
			{
				int nHalfCols=m_nCols/2;
				if(m_nSatelliteID==HJ1A&&m_nSensorType==HJ1A_CCD)
				{
					if(nCol<nHalfCols)					
					{
						nCameraIndex=HJ1A_CCD1;
					}
					else
					{
						
						nCameraIndex=HJ1A_CCD2;
						nCol=m_CCDCamera[nCameraIndex].nCCDLength+nCol-m_nCols;
					}
				}
				else if(m_nSatelliteID==HJ1B&&m_nSensorType==HJ1B_CCD)
				{
					if(nCol<nHalfCols)
					{
						nCameraIndex=HJ1B_CCD1;
					}
					else
					{
						
						nCameraIndex=HJ1B_CCD2;
						nCol=m_CCDCamera[nCameraIndex].nCCDLength+nCol-m_nCols;
					}
				}
			}
			
			double lfFocusLength=m_CCDCamera[nCameraIndex].lfFocus;
			double sx=m_CCDCamera[nCameraIndex].sx[nCol];
			double sy=m_CCDCamera[nCameraIndex].sy[nCol];

			double Phi=at.ROLL;
			double Omega=at.PITCH;
			double Kappa=at.YAW;
			double rotMatrixPlatform[3][3];
			CreateRotMatrixPOK(Phi,Omega,Kappa,rotMatrixPlatform);
			double rotMatrixSensor[3][3];
			CreateRotMatrixPOK(m_CCDCamera[nCameraIndex].lfRoll,m_CCDCamera[nCameraIndex].lfPitch,m_CCDCamera[nCameraIndex].lfYaw,rotMatrixSensor);
			double rotMatrix[3][3];
			MultipleMatrix(&rotMatrixSensor[0][0],&rotMatrixPlatform[0][0],&rotMatrix[0][0],3);

			Z=fHeight;
			double lamed=(Z-Zs)/(rotMatrix[0][2]*sx+rotMatrix[1][2]*sy-rotMatrix[2][2]*lfFocusLength);
			X=lamed*(rotMatrix[0][0]*sx+rotMatrix[1][0]*sy-rotMatrix[2][0]*lfFocusLength)+Xs;
			Y=lamed*(rotMatrix[0][1]*sx+rotMatrix[1][1]*sy-rotMatrix[2][1]*lfFocusLength)+Ys;

			*pAltitude=Z;
			return m_Projection.Map2Geodetic(X,Y,pLat,pLon);
		}
		break;
	case HJ1B_IRS:
		{
			double X,Y,Z;
			//intersect attitude and emphemeris
			double fDate=PixelDate(nRow,nCol);
			Ephemeris ep=EphemerisDate(fDate);
			Attitude at=AttitudeDate(fDate);

			double Xs,Ys,Zs;
			m_Projection.Geodetic2Map(ep.P.x,ep.P.y,&Xs,&Ys);
			Zs=ep.P.z;

			double lfFocusLength=m_ScanCamera.lfFocus;
			double sx=m_ScanCamera.sx;
			double sy=m_ScanCamera.sy[nRow%m_ScanCamera.nScanFrame];

			double Phi=at.ROLL;
			double Omega=at.PITCH;
//			double Kappa=at.YAW;
			double rotMatrixPlatform[3][3];
			CreateRotMatrixPOK(Phi,Omega,15*PI/180,rotMatrixPlatform);
			double rotMatrixSensor[3][3];
			CreateRotMatrixPOK(m_ScanCamera.lfPixelFOV*(2499-nCol),0.0,0.0,rotMatrixSensor);
			double rotMatrix[3][3];
			MultipleMatrix(&rotMatrixSensor[0][0],&rotMatrixPlatform[0][0],&rotMatrix[0][0],3);

			Z=fHeight;
			double lamed=(Z-Zs)/(rotMatrix[0][2]*sx+rotMatrix[1][2]*sy-rotMatrix[2][2]*lfFocusLength);
			X=lamed*(rotMatrix[0][0]*sx+rotMatrix[1][0]*sy-rotMatrix[2][0]*lfFocusLength)+Xs;
			Y=lamed*(rotMatrix[0][1]*sx+rotMatrix[1][1]*sy-rotMatrix[2][1]*lfFocusLength)+Ys;

			*pAltitude=Z;
			return m_Projection.Map2Geodetic(X,Y,pLat,pLon);
		}
	}

	return S_FALSE;
}

void CDimap::InitializeCamera(char* pszCamFile)
{
	FILE* fpAux=fopen(pszCamFile,"rt");
	if(fpAux==NULL)
		return;
	char szBuffer[512];
	char szTag[256];
	char szMean[10];
	char szValue[256];
//	double lfT0,lfTn;
	while(!feof(fpAux))
	{
		fgets(szBuffer,511,fpAux);
		sscanf(szBuffer,"%s%s%s",szTag,szMean,szValue);
		//-----------------------A-CCD1-------------------------
		if(strcmp(szTag,"HJ1ACCD1CameraFocus")==0)
		{
			m_CCDCamera[0].lfFocus=atof(szValue)*1000.0f;
			printf("HJ1ACCD1CameraFocus is %lf\n",m_CCDCamera[0].lfFocus);
		}
		else if(strcmp(szTag,"HJ1ACCD1CameraScanSize")==0)
		{
			m_CCDCamera[0].lfScanSize=atof(szValue)*1000.0f;
			printf("HJ1ACCD1CameraScanSize is %lf\n",m_CCDCamera[0].lfScanSize);
		}
		else if(strcmp(szTag,"HJ1ACCD1CameraCCDLength")==0)
		{
			m_CCDCamera[0].nCCDLength=atoi(szValue);
			printf("HJ1ACCD1CameraCCDLength is %d\n",m_CCDCamera[0].nCCDLength);
		}
		else if(strcmp(szTag,"HJ1ACCD1CameraRoll")==0)
		{
			m_CCDCamera[0].lfRoll=atof(szValue);
			printf("HJ1ACCD1CameraRoll is %lf\n",m_CCDCamera[0].lfRoll);
		}
		else if(strcmp(szTag,"HJ1ACCD1CameraPitch")==0)
		{
			m_CCDCamera[0].lfPitch=atof(szValue);
			printf("HJ1ACCD1CameraPitch is %lf\n",m_CCDCamera[0].lfPitch);
		}
		else if(strcmp(szTag,"HJ1ACCD1CameraYaw")==0)
		{
			m_CCDCamera[0].lfYaw=atof(szValue);
			printf("HJ1ACCD1CameraYaw is %lf\n",m_CCDCamera[0].lfYaw);
		}
		//-----------------------A-CCD2-------------------------
		else if(strcmp(szTag,"HJ1ACCD2CameraFocus")==0)
		{
			m_CCDCamera[1].lfFocus=atof(szValue)*1000.0f;
			printf("HJ1ACCD2CameraFocus is %lf\n",m_CCDCamera[1].lfFocus);
		}
		else if(strcmp(szTag,"HJ1ACCD2CameraScanSize")==0)
		{
			m_CCDCamera[1].lfScanSize=atof(szValue)*1000.0f;
			printf("HJ1ACCD2CameraScanSize is %lf\n",m_CCDCamera[1].lfScanSize);
		}
		else if(strcmp(szTag,"HJ1ACCD2CameraCCDLength")==0)
		{
			m_CCDCamera[1].nCCDLength=atoi(szValue);
			printf("HJ1ACCD2CameraCCDLength is %d\n",m_CCDCamera[1].nCCDLength);
		}
		else if(strcmp(szTag,"HJ1ACCD2CameraRoll")==0)
		{
			m_CCDCamera[1].lfRoll=atof(szValue);
			printf("HJ1ACCD2CameraRoll is %lf\n",m_CCDCamera[1].lfRoll);
		}
		else if(strcmp(szTag,"HJ1ACCD2CameraPitch")==0)
		{
			m_CCDCamera[1].lfPitch=atof(szValue);
			printf("HJ1ACCD2CameraPitch is %lf\n",m_CCDCamera[1].lfPitch);
		}
		else if(strcmp(szTag,"HJ1ACCD2CameraYaw")==0)
		{
			m_CCDCamera[1].lfYaw=atof(szValue);
			printf("HJ1ACCD2CameraYaw is %lf\n",m_CCDCamera[1].lfYaw);
		}
		//---------------------A-HSI-------------------------
		else if(strcmp(szTag,"HJ1AHSICameraFocus")==0)
		{
			m_CCDCamera[2].lfFocus=atof(szValue)*1000.0f;
			printf("HJ1AHSICameraFocus is %lf\n",m_CCDCamera[2].lfFocus);
		}
		else if(strcmp(szTag,"HJ1AHSICameraScanSize")==0)
		{
			m_CCDCamera[2].lfScanSize=atof(szValue)*1000.0f;
			printf("HJ1AHSICameraScanSize is %lf\n",m_CCDCamera[2].lfScanSize);
		}
		else if(strcmp(szTag,"HJ1AHSICameraCCDLength")==0)
		{
			m_CCDCamera[2].nCCDLength=atoi(szValue);
			printf("HJ1AHSICameraCCDLength is %d\n",m_CCDCamera[2].nCCDLength);
		}
		else if(strcmp(szTag,"HJ1AHSICameraRoll")==0)
		{
			m_CCDCamera[2].lfRoll=atof(szValue);
			printf("HJ1AHSICameraRoll is %lf\n",m_CCDCamera[2].lfRoll);
		}
		else if(strcmp(szTag,"HJ1AHSICameraPitch")==0)
		{
			m_CCDCamera[2].lfPitch=atof(szValue);
			printf("HJ1AHSICameraPitch is %lf\n",m_CCDCamera[2].lfPitch);
		}
		else if(strcmp(szTag,"HJ1AHSICameraYaw")==0)
		{
			m_CCDCamera[2].lfYaw=atof(szValue);
			printf("HJ1AHSICameraYaw is %lf\n",m_CCDCamera[2].lfYaw);
		}
		//---------------------HJ1B-CCD1--------------------
		else if(strcmp(szTag,"HJ1BCCD1CameraFocus")==0)
		{
			m_CCDCamera[3].lfFocus=atof(szValue)*1000.0f;
			printf("HJ1BCCD1CameraFocus is %lf\n",m_CCDCamera[3].lfFocus);
		}
		else if(strcmp(szTag,"HJ1BCCD1CameraScanSize")==0)
		{
			m_CCDCamera[3].lfScanSize=atof(szValue)*1000.0f;
			printf("HJ1BCCD1CameraScanSize is %lf\n",m_CCDCamera[3].lfScanSize);
		}
		else if(strcmp(szTag,"HJ1BCCD1CameraCCDLength")==0)
		{
			m_CCDCamera[3].nCCDLength=atoi(szValue);
			printf("HJ1BCCD1CameraCCDLength is %d\n",m_CCDCamera[3].nCCDLength);
		}
		else if(strcmp(szTag,"HJ1BCCD1CameraRoll")==0)
		{
			m_CCDCamera[3].lfRoll=atof(szValue);
			printf("HJ1BCCD1CameraRoll is %lf\n",m_CCDCamera[3].lfRoll);
		}
		else if(strcmp(szTag,"HJ1BCCD1CameraPitch")==0)
		{
			m_CCDCamera[3].lfPitch=atof(szValue);
			printf("HJ1BCCD1CameraPitch is %lf\n",m_CCDCamera[3].lfPitch);
		}
		else if(strcmp(szTag,"HJ1BCCD1CameraYaw")==0)
		{
			m_CCDCamera[3].lfYaw=atof(szValue);
			printf("HJ1BCCD1CameraYaw is %lf\n",m_CCDCamera[3].lfYaw);
		}
		//----------------HJ1B-CCD2-------------------------
		else if(strcmp(szTag,"HJ1BCCD2CameraFocus")==0)
		{
			m_CCDCamera[4].lfFocus=atof(szValue)*1000.0f;
			printf("HJ1BCCD2CameraFocus is %lf\n",m_CCDCamera[4].lfFocus);
		}
		else if(strcmp(szTag,"HJ1BCCD2CameraScanSize")==0)
		{
			m_CCDCamera[4].lfScanSize=atof(szValue)*1000.0f;
			printf("HJ1BCCD2CameraScanSize is %lf\n",m_CCDCamera[4].lfScanSize);
		}
		else if(strcmp(szTag,"HJ1BCCD2CameraCCDLength")==0)
		{
			m_CCDCamera[4].nCCDLength=atoi(szValue);
			printf("HJ1BCCD2CameraScanSize is %d\n",m_CCDCamera[4].nCCDLength);
		}
		else if(strcmp(szTag,"HJ1BCCD2CameraRoll")==0)
		{
			m_CCDCamera[4].lfRoll=atof(szValue);
			printf("HJ1BCCD2CameraRoll is %lf\n",m_CCDCamera[4].lfRoll);
		}
		else if(strcmp(szTag,"HJ1BCCD2CameraPitch")==0)
		{
			m_CCDCamera[4].lfPitch=atof(szValue);
			printf("HJ1BCCD2CameraPitch is %lf\n",m_CCDCamera[4].lfPitch);
		}
		else if(strcmp(szTag,"HJ1BCCD2CameraYaw")==0)
		{
			m_CCDCamera[4].lfYaw=atof(szValue);
			printf("HJ1BCCD2CameraYaw is %lf\n",m_CCDCamera[4].lfYaw);
		}
		//--------------------HJ1B-IRS----------------------
		else if(strcmp(szTag,"HJ1BIRSCameraFocus")==0)
		{
			m_ScanCamera.lfFocus=atof(szValue)*1000.0f;
			printf("HJ1BIRSCameraFocus is %lf\n",m_ScanCamera.lfFocus);
		}
		else if(strcmp(szTag,"HJ1BIRSCameraScanSize")==0)
		{
			m_ScanCamera.lfScanSize=atof(szValue)*1000.0f;
			printf("HJ1BIRSCameraScanSize is %lf\n",m_ScanCamera.lfScanSize);
		}
		else if(strcmp(szTag,"HJ1BIRSCameraCCDLength")==0)
		{
			m_ScanCamera.nScanLength=atoi(szValue);
			printf("HJ1BIRSCameraCCDLength is %d\n",m_ScanCamera.nScanLength);
		}
		else if(strcmp(szTag,"HJ1BIRSCameraScanFrame")==0)
		{
			m_ScanCamera.nScanFrame=atoi(szValue);
			printf("HJ1BIRSCameraScanFrame is %d\n",m_ScanCamera.nScanFrame);
		}
		else if(strcmp(szTag,"HJ1BIRSCameraPixelFOV")==0)
		{
			m_ScanCamera.lfPixelFOV=atof(szValue);
			printf("HJ1BIRSCameraPixelFOV is %.10lf\n",m_ScanCamera.lfPixelFOV);
		}
		else if(strcmp(szTag,"HJ1BIRSCameraPixelTime")==0)
		{
			m_ScanCamera.lfPixelTime=atof(szValue);
			printf("HJ1BIRSCameraPixelTime is %.10lf\n",m_ScanCamera.lfPixelTime);
		}
		else if(strcmp(szTag,"HJ1BIRSCameraRoll")==0)
		{
			m_ScanCamera.lfRoll=atof(szValue);
			printf("HJ1BIRSCameraRoll is %lf\n",m_ScanCamera.lfRoll);
		}
		else if(strcmp(szTag,"HJ1BIRSCameraPitch")==0)
		{
			m_ScanCamera.lfPitch=atof(szValue);
			printf("HJ1BIRSCameraPitch is %lf\n",m_ScanCamera.lfPitch);
		}
		else if(strcmp(szTag,"HJ1BIRSCameraYaw")==0)
		{
			m_ScanCamera.lfYaw=atof(szValue);
			printf("HJ1BIRSCameraYaw is %lf\n",m_ScanCamera.lfYaw);
		}
		else
		{
			continue;
		}
	}
	//===============================A-CCD1=====================================//
	int nHalfCols=m_CCDCamera[0].nCCDLength/2;
	m_CCDCamera[0].sx=new double[m_CCDCamera[0].nCCDLength];
	m_CCDCamera[0].sy=new double[m_CCDCamera[0].nCCDLength];
	int i=0;
	for(i=0;i<m_CCDCamera[0].nCCDLength;i++)
	{
		m_CCDCamera[0].sx[i]=(i-nHalfCols)*m_CCDCamera[0].lfScanSize;
		m_CCDCamera[0].sy[i]=0;
	}
	//===============================A-CCD2=====================================//
	nHalfCols=m_CCDCamera[1].nCCDLength/2;
	m_CCDCamera[1].sx=new double[m_CCDCamera[1].nCCDLength];
	m_CCDCamera[1].sy=new double[m_CCDCamera[1].nCCDLength];
	for(i=0;i<m_CCDCamera[1].nCCDLength;i++)
	{
		m_CCDCamera[1].sx[i]=(i-nHalfCols)*m_CCDCamera[1].lfScanSize;
		m_CCDCamera[1].sy[i]=0;
	}
	//===============================A-HSI=====================================//
	nHalfCols=m_CCDCamera[2].nCCDLength/2;
	m_CCDCamera[2].sx=new double[m_CCDCamera[2].nCCDLength];
	m_CCDCamera[2].sy=new double[m_CCDCamera[2].nCCDLength];
	for(i=0;i<m_CCDCamera[2].nCCDLength;i++)
	{
		m_CCDCamera[2].sx[i]=(i-nHalfCols)*m_CCDCamera[2].lfScanSize;
		m_CCDCamera[2].sy[i]=0;

	}
	//===============================B-CCD1=====================================//

	nHalfCols=m_CCDCamera[3].nCCDLength/2;
	m_CCDCamera[3].sx=new double[m_CCDCamera[3].nCCDLength];
	m_CCDCamera[3].sy=new double[m_CCDCamera[3].nCCDLength];
	for(i=0;i<m_CCDCamera[3].nCCDLength;i++)
	{
		m_CCDCamera[3].sx[i]=(i-nHalfCols)*m_CCDCamera[3].lfScanSize;
		m_CCDCamera[3].sy[i]=0;
	}
	//===============================B-CCD2=====================================//
		
	nHalfCols=m_CCDCamera[4].nCCDLength/2;
	m_CCDCamera[4].sx=new double[m_CCDCamera[4].nCCDLength];
	m_CCDCamera[4].sy=new double[m_CCDCamera[4].nCCDLength];
	for(i=0;i<m_CCDCamera[4].nCCDLength;i++)
	{
		m_CCDCamera[4].sx[i]=(i-nHalfCols)*m_CCDCamera[4].lfScanSize;
		m_CCDCamera[4].sy[i]=0;
	}

	//===============================B-IRS=====================================//
	int nHalfFrame=m_ScanCamera.nScanFrame/2;
	m_ScanCamera.sx=0;
	m_ScanCamera.sy=new double[m_ScanCamera.nScanFrame];
	for(i=0;i<m_ScanCamera.nScanFrame;i++)
	{
		m_ScanCamera.sy[i]=(i-nHalfFrame)*m_ScanCamera.lfScanSize;
	}

}

HRESULT CDimap::Initialize(char* pszAuxFile,
						   char* pszEphFile,
						   char* pszAttFile,
						   char* pszCamFile)
{
	FILE* fpAux=fopen(pszAuxFile,"rt");
	if(fpAux==NULL)
	{
		COutput::OutputFileOpenFailed(pszAuxFile);
		
		return S_FALSE;
	}

	FILE* fpEph=fopen(pszEphFile,"rt");
	if(fpEph==NULL)
	{
		COutput::OutputFileOpenFailed(pszEphFile);

		fclose(fpAux);

		return S_FALSE;
	}

	FILE* fpAtt=fopen(pszAttFile,"rt");
	if(fpAtt==NULL)
	{
		COutput::OutputFileOpenFailed(pszAttFile);

		fclose(fpAux);
		fclose(fpEph);

		return S_FALSE;
	}

	char szBuffer[512];
	char szTag[256];
	char szMean[10];
	char szValue[256];

	double lfT0,lfTn;
	while(!feof(fpAux))
	{
		fgets(szBuffer,511,fpAux);
		
		sscanf(szBuffer,"%s%s%s",szTag,szMean,szValue);
		if(strcmp(szTag,"satelliteID")==0)
		{
			if(strcmp(szValue,"\"HJ1A\"")==0)
			{
				m_nSatelliteID=HJ1A;
			}
			else if(strcmp(szValue,"\"HJ1B\"")==0)
			{
				m_nSatelliteID=HJ1B;
			}
		}
		else if(strcmp(szTag,"cameraID")==0)
		{
			if(strcmp(szValue,"\"CCD1\"")==0)
			{
				if(m_nSatelliteID==HJ1A)
				{
					m_nSensorType=HJ1A_CCD1;
				}
				else if(m_nSatelliteID==HJ1B)
				{
					m_nSensorType=HJ1B_CCD1;
				}
			}
			else if(strcmp(szValue,"\"CCD2\"")==0)
			{
				if(m_nSatelliteID==HJ1A)
				{
					m_nSensorType=HJ1A_CCD2;
				}
				else if(m_nSatelliteID==HJ1B)
				{
					m_nSensorType=HJ1B_CCD2;
				}
			}
			else if(strcmp(szValue,"\"CCD\"")==0)
			{
				if(m_nSatelliteID==HJ1A)
				{
					m_nSensorType=HJ1A_CCD;
				}
				else if(m_nSatelliteID==HJ1B)
				{
					m_nSensorType=HJ1B_CCD;
				}
			}
			else if(strcmp(szValue,"\"HSI\"")==0)
			{
				m_nSensorType=HJ1A_HSI;
			}
			else if(strcmp(szValue,"\"IRS\"")==0)
			{
				m_nSensorType=HJ1B_IRS;
			}
		}
		else if(strcmp(szTag,"firstLineTimeCode")==0)
		{
			lfT0=atof(szValue);
		}
		else if(strcmp(szTag,"lastLineTimeCode")==0)
		{
			lfTn=atof(szValue);
		}
		else if(strcmp(szTag,"imageWidth")==0)
		{
			m_nCols=atoi(szValue);
		}
		else if(strcmp(szTag,"imageHeight")==0)
		{
			m_nRows=atoi(szValue);
		}
	}
	m_fT0=lfT0;
	m_fLSP=(lfTn-lfT0)/(m_nRows-1);

	if(m_nSensorType==HJ1B_IRS)
	{
		int nFrame=m_nRows;
		m_nRows=m_nRows*m_ScanCamera.nScanFrame;

		fseek(fpAux,0,SEEK_SET);

		int i=0;
		for(i=0;i<nFrame;i++)
		{
			while(!feof(fpAux))
			{
				fgets(szBuffer,511,fpAux);
				
				if(szBuffer[0]=='{')
				{
					break;
				}
			}

			while(!feof(fpAux))
			{
				fgets(szBuffer,511,fpAux);
				if(szBuffer[0]=='}')
				{
					break;
				}
				
				sscanf(szBuffer,"%s%s%s",szTag,szMean,szValue);
				if(strcmp(szTag,"GpsTimeCode")==0)
				{
					m_ScanFrameTime.Add(atof(szValue));
				}
			}
		}
		if(m_ScanFrameTime.GetSize()!=nFrame)
		{
			COutput::Output("aux file format error");
			
			fclose(fpAux);
			fclose(fpEph);
			fclose(fpAtt);

			return S_FALSE;
		}
	}

	int nEphNum=0;
	while(!feof(fpEph))
	{
		fgets(szBuffer,511,fpEph);
		sscanf(szBuffer,"%s%s%s",szTag,szMean,szValue);
		if(strcmp(szTag,"groupNumber")==0)
		{
			nEphNum=atoi(szValue);
			break;
		}
	}
	int i=0;
	for(i=0;i<nEphNum;i++)
	{
		while(!feof(fpEph))
		{
			fgets(szBuffer,511,fpEph);
			
			if(szBuffer[0]=='{')
			{
				break;
			}
		}

		Ephemeris ep;
		double GX,GY,GZ;
		
		while(!feof(fpEph))
		{
			fgets(szBuffer,511,fpEph);
			if(szBuffer[0]=='}')
			{
				break;
			}
			
			sscanf(szBuffer,"%s%s%s",szTag,szMean,szValue);
			if(strcmp(szTag,"timeCode")==0)
			{
				ep.t=atof(szValue);
			}
			else if(strcmp(szTag,"PX")==0)
			{
				GX=atof(szValue);
			}
			else if(strcmp(szTag,"PY")==0)
			{
				GY=atof(szValue);
			}
			else if(strcmp(szTag,"PZ")==0)
			{
				GZ=atof(szValue);
			}
			else if(strcmp(szTag,"VX")==0)
			{
				ep.V.x=atof(szValue);
			}
			else if(strcmp(szTag,"VY")==0)
			{
				ep.V.y=atof(szValue);
			}
			else if(strcmp(szTag,"VZ")==0)
			{
				ep.V.z=atof(szValue);
			}
		}

		m_Projection.Geocentric2Geodetic(GX,GY,GZ,&ep.P.x,&ep.P.y,&ep.P.z);
		m_Ephemeris.Add(ep);
	}

	int nAttNum=0;
	while(!feof(fpAtt))
	{
		fgets(szBuffer,511,fpAtt);
		sscanf(szBuffer,"%s%s%s",szTag,szMean,szValue);
		if(strcmp(szTag,"groupNumber")==0)
		{
			nAttNum=atoi(szValue);
			break;
		}
	}
	for(i=0;i<nAttNum;i++)
	{
		while(!feof(fpAtt))
		{
			fgets(szBuffer,511,fpAtt);
			
			if(szBuffer[0]=='{')
			{
				break;
			}
		}

		Attitude at;
		
		while(!feof(fpAtt))
		{
			fgets(szBuffer,511,fpAtt);
			if(szBuffer[0]=='}')
			{
				break;
			}
			
			sscanf(szBuffer,"%s%s%s",szTag,szMean,szValue);
			if(strcmp(szTag,"timeCode")==0)
			{
				at.t=atof(szValue);
			}
			else if(strcmp(szTag,"roll")==0)
			{
				at.ROLL=atof(szValue);
				at.ROLL=at.ROLL*PI/180.0;
			}
			else if(strcmp(szTag,"pitch")==0)
			{
				at.PITCH=atof(szValue);
				at.PITCH=at.PITCH*PI/180.0;
			}
			else if(strcmp(szTag,"yaw")==0)
			{
				at.YAW=atof(szValue);
				at.YAW=at.YAW*PI/180.0;
			}
		}

		m_Attitude.Add(at);
	}

	InitializeCamera(pszCamFile);
	
	fclose(fpAux);
	fclose(fpEph);
	fclose(fpAtt);

	double lfTc=LineDate(m_nRows/2);
	Ephemeris epc=EphemerisDate(lfTc);
	double lfLatitude=epc.P.x;
	double lfLongitude=epc.P.y;
	long Zone;
	m_Projection.CalcZone(lfLatitude,lfLongitude,&Zone);

	m_Projection.SetZoneStep(6);
	m_Projection.SetZone(Zone);
	m_Projection.SetHemisphere(lfLatitude>0?'N':'S');

	double lfTempAltitude;
	Image2Geodetic(0,0,0,&m_fDatasetFrameLat[0],&m_fDatasetFrameLon[0],&lfTempAltitude);
	Image2Geodetic(0,m_nCols-1,0,&m_fDatasetFrameLat[1],&m_fDatasetFrameLon[1],&lfTempAltitude);
	Image2Geodetic(m_nRows-1,m_nCols-1,0,&m_fDatasetFrameLat[2],&m_fDatasetFrameLon[2],&lfTempAltitude);
	Image2Geodetic(m_nRows-1,0,0,&m_fDatasetFrameLat[3],&m_fDatasetFrameLon[3],&lfTempAltitude);
	Image2Geodetic(m_nRows/2,m_nCols/2,0,&m_fDatasetCenterLat,&m_fDatasetCenterLon,&lfTempAltitude);
	for(i=0;i<4;i++)
	{
		m_fDatasetFrameLat[i]=m_fDatasetFrameLat[i]*180.0/PI;
		m_fDatasetFrameLon[i]=m_fDatasetFrameLon[i]*180.0/PI;
	}

	return S_OK;
}

HRESULT CDimap::ContrustRPC(char* pszRPCFile)
{
	double fAltitudeMin=-5000;
	double fAltitudeMax=5000;

	//锟斤拷锟斤拷细窦负锟侥ｏ拷凸锟斤拷锟斤拷锟狡革拷锟斤拷
	const int nInterval=__min(m_nRows/25,m_nCols/25);
	const int nLayerNum=10;
	
	int nGCPNum=(m_nRows/nInterval)*(m_nCols/nInterval)*nLayerNum;
	double* pLat=new double[nGCPNum];
	double* pLon=new double[nGCPNum];
	double* pHeight=new double[nGCPNum];
	double* pSamp=new double[nGCPNum];
	double* pLine=new double[nGCPNum];

	int nIndex=0;
	for(int nLayer=0;nLayer<nLayerNum;nLayer++)
	{
		double fAltitude=fAltitudeMin+(fAltitudeMax-fAltitudeMin)/nLayerNum*nLayer;
		/*FILE* fp=NULL;
		if(nLayer==5)
		{
			fp=fopen("F:\\HJ-AB-Data\\一锟斤拷影锟斤拷\\test.gcp","wt");
			fprintf(fp,";\n");
			fprintf(fp,";\n");
			fprintf(fp,";\n");
			fprintf(fp,"%d\n",(m_nRows/nInterval)*(m_nCols/nInterval));
		}*/
		for(int i=0;i<m_nRows;i+=nInterval)
		{
			for(int j=0;j<m_nCols;j+=nInterval)
			{
				pSamp[nIndex]=j+1;
				pLine[nIndex]=i+1;
				if(pSamp[nIndex]>m_nRows||pLine[nIndex]>m_nCols)
				{
					continue;
				}

				if(Image2Geodetic((int)pLine[nIndex]-1,(int)pSamp[nIndex]-1,fAltitude,&pLat[nIndex],&pLon[nIndex],&pHeight[nIndex])==S_OK)
				{
					/*if(fp!=NULL)
					{
						double X,Y;
						m_Projection.Geodetic2Map(pLat[nIndex],pLon[nIndex],&X,&Y);
						fprintf(fp,"%d   %lf  %lf  %lf  %lf  %lf\n",nIndex,X,Y,fAltitude,pSamp[nIndex],m_nRows-pLine[nIndex]);
					}*/

					pLat[nIndex]=pLat[nIndex]*180.0/PI;
					pLon[nIndex]=pLon[nIndex]*180.0/PI;

					nIndex++;
				}
			}
		}
		/*if(fp!=NULL)
		{
			fclose(fp);
			fp=NULL;
		}
		return S_OK;*/
	}
	nGCPNum=nIndex;
	//锟斤拷锟斤拷一锟斤拷
	double LAT_OFF=0,LONG_OFF=0,HEIGHT_OFF=0,LINE_OFF=0,SAMP_OFF=0;
	double LAT_SCALE=0,LONG_SCALE=0,HEIGHT_SCALE=0,LINE_SCALE=0,SAMP_SCALE=0;
	double maxLat=pLat[0],minLat=pLat[0];
	double maxLon=pLon[0],minLon=pLon[0];
	double maxHeight=pHeight[0],minHeight=pHeight[0];
	double maxLine=pLine[0],minLine=pLine[0];
	double maxSamp=pSamp[0],minSamp=pSamp[0];
	int i=0;
	for(i=0;i<nGCPNum;i++)
	{
		LAT_OFF+=pLat[i];
		LONG_OFF+=pLon[i];
		HEIGHT_OFF+=pHeight[i];
		LINE_OFF+=pLine[i];
		SAMP_OFF+=pSamp[i];

		if(maxLat<pLat[i])
		{
			maxLat=pLat[i];
		}
		if(minLat>pLat[i])
		{
			minLat=pLat[i];
		}

		if(maxLon<pLon[i])
		{
			maxLon=pLon[i];
		}
		if(minLon>pLon[i])
		{
			minLon=pLon[i];
		}

		if(maxHeight<pHeight[i])
		{
			maxHeight=pHeight[i];
		}
		if(minHeight>pHeight[i])
		{
			minHeight=pHeight[i];
		}

		if(maxLine<pLine[i])
		{
			maxLine=pLine[i];
		}
		if(minLine>pLine[i])
		{
			minLine=pLine[i];
		}

		if(maxSamp<pSamp[i])
		{
			maxSamp=pSamp[i];
		}
		if(minSamp>pSamp[i])
		{
			minSamp=pSamp[i];
		}
	}
	LAT_OFF/=nGCPNum;
	LONG_OFF/=nGCPNum;
	HEIGHT_OFF/=nGCPNum;
	LINE_OFF/=nGCPNum;
	SAMP_OFF/=nGCPNum;
	LAT_SCALE=__max(fabs(maxLat-LAT_OFF),fabs(minLat-LAT_OFF));
	LONG_SCALE=__max(fabs(maxLon-LONG_OFF),fabs(minLon-LONG_OFF));
	HEIGHT_SCALE=__max(fabs(maxHeight-HEIGHT_OFF),fabs(minHeight-HEIGHT_OFF));
	LINE_SCALE=__max(fabs(maxLine-LINE_OFF),fabs(minLine-LINE_OFF));
	SAMP_SCALE=__max(fabs(maxSamp-SAMP_OFF),fabs(minSamp-SAMP_OFF));

	for(i=0;i<nGCPNum;i++)
	{
		pLat[i]=(pLat[i]-LAT_OFF)/LAT_SCALE;
		pLon[i]=(pLon[i]-LONG_OFF)/LONG_SCALE;
		pHeight[i]=(pHeight[i]-HEIGHT_OFF)/HEIGHT_SCALE;
		pLine[i]=(pLine[i]-LINE_OFF)/LINE_SCALE;
		pSamp[i]=(pSamp[i]-SAMP_OFF)/SAMP_SCALE;
	}

	//锟斤拷锟斤拷RPC锟斤拷锟斤拷
	int nTimes=0;
	int n;
	double fLCoef[39],fSCoef[39];
	memset(fLCoef,0,39*sizeof(double));
	memset(fSCoef,0,39*sizeof(double));
	do
	{
		double fLCoef0[39],fSCoef0[39];
		double a[39], ab[39], aa[1521], b;	
		double* P=pLat;
		double* L=pLon;
		double* H=pHeight;
		double* X=pSamp;
		double* Y=pLine;
		//锟叫凤拷锟斤拷
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
			double p=1.0;
			if(nTimes>0)
			{
				double LINE_DEN_COEFF[20];
				LINE_DEN_COEFF[0]=1.0;
				int j=0;
				for(j=1;j<20;j++)
				{
					LINE_DEN_COEFF[j]=fLCoef[j+19];
				}
				p=0;
				for(j=0;j<20;j++)
				{
					p+=(LINE_DEN_COEFF[j]*a[j]);
				}
			}
			pnrml(a,39,b,aa,ab,p*p);
		}
		for(i=0,n=0;i<39;i++)
		{
			aa[n]+=1e-6;
			n+=(39-i);
		}
		dsolve(aa,ab, fLCoef0, 39, 39);
		//锟叫凤拷锟斤拷
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
			double p=1.0;
			if(nTimes>0)
			{
				double SAMPLE_DEN_COEFF[20];
				SAMPLE_DEN_COEFF[0]=1.0;
				int j=0;
				for(j=1;j<20;j++)
				{
					SAMPLE_DEN_COEFF[j]=fSCoef[j+19];
				}
				p=0;
				for(j=0;j<20;j++)
				{
					p+=(SAMPLE_DEN_COEFF[j]*a[j]);
				}
			}
			pnrml(a,39,b,aa,ab,p*p);
		}
		for(i=0,n=0;i<39;i++)
		{
			aa[n]+=1e-6;
			n+=(39-i);
		}
		dsolve(aa,ab, fSCoef0, 39, 39);

		//锟叫断碉拷锟斤拷锟斤拷
		double fMaxLError=fabs(fLCoef[0]-fLCoef0[0]);
		for(i=1;i<20;i++)
		{
			double fError=fabs(fLCoef[i]-fLCoef0[i]);
			if(fError>fMaxLError)
			{
				fMaxLError=fError;
			}
		}

		double fMaxSError=fabs(fSCoef[0]-fSCoef0[0]);
		for(i=1;i<20;i++)
		{
			double fError=fabs(fSCoef[i]-fSCoef0[i]);
			if(fError>fMaxSError)
			{
				fMaxSError=fError;
			}
		}

		memcpy(fLCoef,fLCoef0,39*sizeof(double));
		memcpy(fSCoef,fSCoef0,39*sizeof(double));

		if(fMaxLError<1e-5&&fMaxSError<1e-5)
		{
			break;
		}

		nTimes++;
	}while(nTimes<50);

	delete [] pLat; pLat=NULL;
	delete [] pLon; pLon=NULL;
	delete [] pHeight; pHeight=NULL;
	delete [] pSamp; pSamp=NULL;
	delete [] pLine; pLine=NULL;

	//锟斤拷录RPC锟斤拷锟斤拷
	FILE* fp=fopen(pszRPCFile,"wt");
	if(fp==NULL)
	{
		COutput::OutputFileOpenFailed(pszRPCFile);
		
		return S_FALSE;
	}

	fprintf(fp,"LINE_OFF: %lf pixels\n",LINE_OFF);
	fprintf(fp,"SAMP_OFF: %lf pixels\n",SAMP_OFF);
	fprintf(fp,"LAT_OFF: %lf degrees\n",LAT_OFF);
	fprintf(fp,"LONG_OFF: %lf degrees\n",LONG_OFF);
	fprintf(fp,"HEIGHT_OFF: %lf meters\n",HEIGHT_OFF);
	fprintf(fp,"LINE_SCALE: %lf pixels\n",LINE_SCALE);
	fprintf(fp,"SAMP_SCALE: %lf pixels\n",SAMP_SCALE);
	fprintf(fp,"LAT_SCALE: %lf degrees\n",LAT_SCALE);
	fprintf(fp,"LONG_SCALE: %lf degrees\n",LONG_SCALE);
	fprintf(fp,"HEIGHT_SCALE: %lf meters\n",HEIGHT_SCALE);
	
	for(i=0;i<20;i++)
	{
		fprintf(fp,"LINE_NUM_COEFF_%d: %e\n",i+1,fLCoef[i]);
	}
	fprintf(fp,"LINE_DEN_COEFF_1: %e\n",1.0);
	for(i=20;i<39;i++)
	{
		fprintf(fp,"LINE_DEN_COEFF_%d: %e\n",i-20+2,fLCoef[i]);
	}

	for(i=0;i<20;i++)
	{
		fprintf(fp,"SAMP_NUM_COEFF_%d: %e\n",i+1,fSCoef[i]);
	}
	fprintf(fp,"SAMP_DEN_COEFF_1: %e\n",1.0);
	for(i=20;i<39;i++)
	{
		fprintf(fp,"SAMP_DEN_COEFF_%d: %e\n",i-20+2,fSCoef[i]);
	}

	fclose(fp);
	fp=NULL;

	return S_OK;
}

void CDimap::MultipleMatrix(double * m1 , double *m2 , double *  multiple, int nDimension)
{
	double    tmp[9];
	int      i, j, k;
    memset(tmp,0,sizeof(double)*9);

	for(i=0; i<nDimension; i++)
	{
		for(j=0; j<nDimension; j++)
		{
			for(k=0; k<nDimension; k++)
			{
				tmp[3*i+j] =tmp[3*i+j]+ (m1[3*i+k] * m2[3*k+j]);
			}
		}
	}

	for(i=0; i<nDimension; i++)
	{
		for(j=0; j<nDimension; j++)
		{
			multiple[3*i+j] = tmp[3*i+j];
		}
	}
}

HRESULT CDimap::Image2Map(int nRow, int nCol, double fHeight, double *pGX, double *pGY)
{
	double lfLatitude,lfLongitude,lfAltitude;
	Image2Geodetic(nRow,nCol,fHeight,&lfLatitude,&lfLongitude,&lfAltitude);
	m_Projection.Geodetic2Map(lfLatitude,lfLongitude,pGX,pGY);

	return S_OK;
}

HRESULT CDimap::Geodetic2Map(double Latitude, double Longitude, double * pGX, double * pGY)
{
	m_Projection.Geodetic2Map(Latitude, Longitude, pGX, pGY);
}

/*
* 通过迭代计算外方位元素 [ZuoW,2010/4/28]
*/
HRESULT CDimap::InteriavteOrientation(double* pIX, double* pIY, double* pGX, double* pGY, 
	double* pGZ, int nGcpNum, double dOrientation[12])
{
	// 源影像参数
	int nSrcCol = dOrientation[3];
	int nSrcRow = dOrientation[4];
	double dCellSize = dOrientation[2];
	double dFocus = (m_CCDCamera[m_nSensorType].lfFocus)*1e-3;		// CCD焦距 	0.141
	double dCCDLength = (m_CCDCamera[m_nSensorType].lfScanSize)*1e2;	// CCD像元大小	0.65
	int time = 1;	// 迭代次数
	
	// 初始外方位元素
	double dOriX = dOrientation[0]+((nSrcCol+1)/2)*dCellSize;
	double dOriY = dOrientation[1]+((nSrcRow+1)/2)*dCellSize;
	double dOriZ = (dOrientation[2]*dFocus/dCCDLength)*100000;
	double Phi = 0.0;
	double Omega = 0.0;
	double Kappa = 0.0;
	
	// 将控制点坐标转换成框标坐标系
	int i, j, k;
	for(i=0; i<nGcpNum; i++)
	{
		pIX[i] = ((pIX[i]-(nSrcCol+1)/2)*dCCDLength)/100000;
		pIY[i] = ((pIY[i]-(nSrcRow+1)/2)*dCCDLength)/100000;
	}
	
	double rotMatrix[3][3];
	double a[2][6];
	double dOffset[6];
	double *dArr = new double[2*nGcpNum*6];
	double *dL = new double[2*nGcpNum];
	double dInv[6*6];
	double* dTransArr = new double[6*2*nGcpNum];
	
	while (time)
	{
		// 初始化 [ZuoW,2010/5/6]
		memset(rotMatrix, 0, 3*3*sizeof(double));
		memset(a, 0, 2*6*sizeof(double));
		memset(dOffset, 0, 6*sizeof(double));
		memset(dArr, 0, 2*nGcpNum*6*sizeof(double));
		memset(dL, 0, 2*nGcpNum*sizeof(double));
		memset(dInv, 0, 6*6*sizeof(double));
		memset(dTransArr, 0, 6*2*nGcpNum*sizeof(double));

		CreateRotMatrixPOK(Phi, Omega, Kappa, rotMatrix);
		
		for(i=0; i<nGcpNum; i++)
		{
			//计算X/Y坐标
			double dPSimilarX = -dFocus*(rotMatrix[0][0]*(pGX[i]-dOriX) + 
				rotMatrix[1][0]*(pGY[i]-dOriY) + rotMatrix[3][0]*(pGZ[i]-dOriZ));
			dPSimilarX /= (rotMatrix[0][2]*(pGX[i]-dOriX)+rotMatrix[1][2]*(pGY[i]-dOriY) +
				rotMatrix[2][2]*(pGZ[i]-dOriZ)); 
			double dPSimilarY = -dFocus*(rotMatrix[0][1]*(pGX[i]-dOriX) + 
				rotMatrix[1][1]*(pGY[i]-dOriY) + rotMatrix[3][1]*(pGZ[i]-dOriZ));
			dPSimilarY /= (rotMatrix[0][2]*(pGX[i]-dOriX)+rotMatrix[1][2]*(pGY[i]-dOriY) +
				rotMatrix[2][2]*(pGZ[i]-dOriZ));
			// 计算系数	
			double dTemp = (rotMatrix[0][2]*(pGX[i]-dOriX) + rotMatrix[1][2]*(pGY[i]-dOriY) + rotMatrix[2][2]*(pGZ[i]-dOriZ));
			a[0][0] = (rotMatrix[0][0]*dFocus + rotMatrix[0][2]*pIX[i]);
			a[0][0] /= dTemp;
			a[0][1] = (rotMatrix[1][0]*dFocus + rotMatrix[1][2]*pIX[i]);
			a[0][1] /= dTemp;
			a[0][2] = (rotMatrix[2][0]*dFocus + rotMatrix[2][2]*pIX[i]);
			a[0][2] /= dTemp;
			
			a[1][0] = (rotMatrix[0][1]*dFocus + rotMatrix[0][2]*pIY[i]);
			a[1][0] /= dTemp;
			a[1][1] = (rotMatrix[1][1]*dFocus + rotMatrix[1][2]*pIY[i]);
			a[1][1] /= dTemp;
			a[1][2] = (rotMatrix[2][1]*dFocus + rotMatrix[2][2]*pIY[i]);
			a[1][2] /= dTemp;
			
			dTemp = (pIX[i]*cos(Kappa)-pIY[i]*sin(Kappa))/dFocus;
			a[0][3] = pIY[i]*sin(Omega);
			a[0][3] -= (pIX[i]*dTemp + dFocus*cos(Kappa))*cos(Omega);
			
			dTemp = pIX[i]*sin(Kappa) + pIY[i]*cos(Kappa);
			a[0][4] = -dFocus*sin(Kappa) - pIX[i]*dTemp/dFocus;
			
			a[0][5] = pIY[i];
			
			dTemp = pIY[i]*(pIX[i]*cos(Kappa)-pIY[i]*sin(Kappa))/dFocus;
			a[1][3] = -pIX[i]*sin(Omega) - (dTemp - dFocus*sin(Kappa))*cos(Omega);
			
			dTemp = pIY[i]*(pIX[i]*sin(Kappa) + pIY[i]*cos(Kappa))/dFocus;
			a[1][4] = -dFocus*cos(Kappa) - dTemp;
			
			a[1][5] = -pIX[i];
			
			// 形成矩阵
			for(j=0; j<6; j++)
			{
				dArr[2*i*6+j] = a[0][j];
				dArr[(2*i+1)*6+j] = a[1][j];
			}

			dL[2*i] = pIX[i] - dPSimilarX;
			dL[2*i+1]   = pIY[i] - dPSimilarY;
		}
		/////////////////////////////////////////////////////////////////////////////////
		gsl_matrix_view matArr = gsl_matrix_view_array((double*)dArr, 2*nGcpNum, 6);

		// 矩阵 A 的转置
		gsl_matrix_view matTransArr = gsl_matrix_view_array(dTransArr, 6, 2*nGcpNum);
		gsl_matrix_transpose_memcpy(&matTransArr.matrix, &matArr.matrix);

		// 矩阵 A'*A
		gsl_matrix *matMulti1 = gsl_matrix_alloc(6, 6);
		gsl_blas_dgemm(CblasNoTrans,CblasNoTrans, 1.0, 
			&matTransArr.matrix, &matArr.matrix, 0.0, matMulti1);

		// 计算 A'*A 结果的行列式
		int signum;
		double det = gsl_linalg_LU_det(matMulti1, signum);
		if(det==0)
		{
			printf("迭代不能收敛，计算退出!\n");
			break;
		}
		
		// 计算矩阵L的转置
		gsl_matrix_view matL = gsl_matrix_view_array(dL, 1, 2*nGcpNum);
		gsl_matrix *matTransL = gsl_matrix_alloc(2*nGcpNum, 1);
		gsl_matrix_transpose_memcpy(matTransL, &matL.matrix);
		
		// 计算 A' * L'
		gsl_matrix *matMulti2 = gsl_matrix_alloc(6, 1);
		gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1.0,
					&matTransArr.matrix, matTransL, 0.0, matMulti2);
		
		// 计算矩阵 (A'*A) 的逆
		gsl_matrix_view matInv = gsl_matrix_view_array(dInv, 6, 6);
		gsl_permutation * p = gsl_permutation_alloc (6);
		gsl_linalg_LU_decomp (matMulti1, p, &k);
		gsl_linalg_LU_invert (matMulti1, p, &matInv.matrix);
		
		// 计算 Inv(A'*A)*(A'*L')
		gsl_matrix *matOffset = gsl_matrix_alloc(6, 1);
		gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1.0,
				&matInv.matrix, matMulti2, 0.0, matOffset);

		for(i=0; i<6; i++)
		{
			dOffset[i] = gsl_matrix_get(matOffset, i, 0);
		}
		////////////////////////////////////////////////////////////////////////////////
		// 外方位元素
		dOriX += dOffset[0];
		dOriY += dOffset[1];
		dOriZ += dOffset[2];
		Phi += dOffset[3];
		Omega += dOffset[4];
		Kappa += dOffset[5];
		
		////////////////////////////////////////////////////////////////////////////////
/*		FILE* pf = fopen("/home/zuow/Documents/matA.txt", "w+");
		for(i=0; i<2*nGcpNum; i++)
		{
			for(j=0; j<6; j++)
			{
				fprintf(pf, " %8.6e ", gsl_matrix_get(&matArr.matrix, i, j));
			}
			fprintf(pf, "\n");
		}
		fclose(pf);

		pf = fopen("/home/zuow/Documents/matL.txt", "w+");
		for(i=0; i<2*nGcpNum; i++)
		{
			fprintf(pf, " %8.6e \n", dL[i]);
		}
		fclose(pf);

		pf = fopen("/home/zuow/Documents/matInv.txt", "w+");
		for(i=0; i<6; i++)
		{
			for(j=0; j<6; j++)
			{
				fprintf(pf, " %8.6e ", gsl_matrix_get(&matInv.matrix, i, j));
			}
			fprintf(pf, "\n");
		}
		fclose(pf);

		pf = fopen("/home/zuow/Documents/matAL.txt", "w+");
		for(i=0; i<6; i++)
		{
			for(j=0; j<1; j++)
			{
				fprintf(pf, " %8.6e ", gsl_matrix_get(matMulti2, i, j));
			}
			fprintf(pf, "\n");
		}
		fclose(pf);

		pf = fopen("/home/zuow/Documents/matInvAA.txt", "w+");
		for(i=0; i<6; i++)
		{
			for(j=0; j<6; j++)
			{
				fprintf(pf, " %8.6e ", gsl_matrix_get(matMulti1, i, j));
			}
			fprintf(pf, "\n");
		}
		fclose(pf);

		pf = fopen("/home/zuow/Documents/Orientation.txt", "wt");
		fprintf(pf, "The %dth Interiavter: \n", time);
		fprintf(pf, "Offsets: %f, %f, %f, %f, %f, %f\n", dOffset[0], dOffset[1], dOffset[2], dOffset[3], dOffset[4], dOffset[5]);
		fprintf(pf, "Orientations: %f, %f, %f, %f, %f, %f \n", dOriX, dOriY, dOriZ, Phi, Omega, Kappa);
		fclose(pf);
*/
		////////////////////////////////////////////////////////////////////////////////

		// 退出迭代条件(判断最大误差)
		double dLocError = abs(dOffset[0]);
		double dDegError = abs(dOffset[5]);
		for(i=1; i<3; i++)
		{
			if(abs(dOffset[i])>dLocError)
				dLocError = abs(dOffset[i]);
			if(abs(dOffset[6-i-1])>dDegError)
				dDegError = abs(dOffset[6-i-1]);
		}
		
		if(dLocError<1e-5 && dDegError<PI/(1.8*(10^4)*60))
		{
			printf("%dth time: %f %f %f %f %f %f\n",
					time, dOffset[0], dOffset[1], dOffset[2], dOffset[3], dOffset[4], dOffset[5]);
			printf("%f, %f, %f, %f, %f, %f\n",
					dOriX, dOriY, dOriZ, Phi, Omega, Kappa);
			break;
		}
		
		time++;

		// 清理内存 [ZuoW,2010/5/1]
		gsl_matrix_free(matMulti1);
		gsl_matrix_free(matMulti2);
		gsl_matrix_free(matOffset);
		gsl_permutation_free (p);
	}
	
	// 返回外方位元素
	dOrientation[0] = dOriX;	dOrientation[1] = dOriY;	dOrientation[2] = dOriZ;
	////////////////////////////////////////////////////////////////////////////////
//	dOrientation[0] = 611115.707719;	dOrientation[1] = 3989468.321996;	dOrientation[2] = 652090.946769;
//	Phi = 6.283072;	Omega = 3.141724;	Kappa = -9.424739;
	////////////////////////////////////////////////////////////////////////////////
	memset(rotMatrix, 0, 3*3*sizeof(double));
	CreateRotMatrixPOK(Phi, Omega, Kappa, rotMatrix);
	for(i=1; i<4; i++)
		for(j=0; j<3; j++)
			dOrientation[i*3+j] = rotMatrix[i-1][j];

	// clear [ZuoW,2010/5/6]
	if(dArr!=NULL)
	{
		delete [] dArr;		dArr = NULL;
	}
	if(dL!=NULL)
	{
		delete [] dL; 		dL = NULL;
	}
	if(dTransArr!=NULL)
	{
		delete [] dTransArr;	dTransArr = NULL;
	}

	return S_OK;
}

/*
* 获取相机参数 [ZuoW,2010/5/4]
*/
HRESULT CDimap::GetCamCoef(double dCamCoef[6])
{
	if(m_nSensorType<0 || m_nSensorType>7)
	{
		printf("Invalid sensor type!\n");
		return S_FALSE;
	}
	
	dCamCoef[0] = m_CCDCamera[m_nSensorType].lfFocus;		dCamCoef[1] = m_CCDCamera[m_nSensorType].lfScanSize;		
	dCamCoef[2] = m_CCDCamera[m_nSensorType].nCCDLength;	dCamCoef[3] = m_CCDCamera[m_nSensorType].lfRoll;
	dCamCoef[4] = m_CCDCamera[m_nSensorType].lfPitch;		dCamCoef[5] = m_CCDCamera[m_nSensorType].lfYaw;
	
	return S_OK;
}


