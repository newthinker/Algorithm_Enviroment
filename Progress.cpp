// Progress.cpp: implementation of the CProgress class.
//
//////////////////////////////////////////////////////////////////////

#include "Progress.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CProgress::CProgress()
{
	m_pszInfo=NULL;

	Reset();
}

CProgress::~CProgress()
{
	if(m_pszInfo!=NULL)
	{
		delete [] m_pszInfo;
		m_pszInfo=NULL;
	}
}

HRESULT CProgress::EnableCancel(BOOL bCanCancel)
{
	return S_OK;
}

HRESULT CProgress::SetCaption(char* szCaption)
{
	return S_OK;
}

HRESULT CProgress::GetPosition(long * plVal)
{
	return S_OK;
}

HRESULT CProgress::SetPosition(long lPos)
{
	m_nPos=lPos;
	int nPos=(int)((double)(m_nPos-m_nBegin)/(double)(m_nEnd-m_nBegin)*100);
	if(m_pszInfo!=NULL)
	{
		printf("%d%% %s\r",nPos,m_pszInfo);
	}
	else
	{
		printf("%d%% \r",nPos);
	}
	return S_OK;
}

HRESULT CProgress::Reset()
{
	if(m_pszInfo!=NULL)
	{
		delete [] m_pszInfo;
		m_pszInfo=NULL;
	}

	m_nBegin=0;
	m_nEnd=100;
	m_nPos=0;
	m_nStep=1;

	return S_OK;
}

HRESULT CProgress::CheckCancel(BOOL * pbVal)
{
	*pbVal=FALSE;

	return S_OK;
}

HRESULT CProgress::Show(BOOL bShow)
{
	return S_OK;
}

HRESULT CProgress::ClearAllInfo()
{
	return S_OK;
}

HRESULT CProgress::AddCurInfo(char* szInformation)
{
	printf("\n");

	if(m_pszInfo!=NULL)
	{
		delete [] m_pszInfo;
		m_pszInfo=NULL;
	}

	int nLength=strlen(szInformation)+1;
	m_pszInfo=new char[nLength];
	if(m_pszInfo!=NULL)
	{
		memset(m_pszInfo,0,sizeof(char)*nLength);
		strcpy(m_pszInfo,szInformation);
		printf("%s\r",m_pszInfo);
	}
	else
	{
		printf("%s\r",szInformation);
	}

	return S_OK;
}

HRESULT CProgress::StepIt()
{
	m_nPos+=m_nStep;
	int nPos=(int)((double)(m_nPos-m_nBegin)/(double)(m_nEnd-m_nBegin)*100);
	if(m_pszInfo!=NULL)
	{
		printf("%d%% %s\r",nPos,m_pszInfo);
	}
	else
	{
		printf("%d%% \r",nPos);
	}

	return S_OK;
}

HRESULT CProgress::SetStep(long lStep)
{
	m_nStep=lStep;

	return S_OK;
}

HRESULT CProgress::SetRange(long lBegin,long lEnd)
{
	m_nBegin=lBegin;
	m_nEnd=lEnd;

	return S_OK;
}

HRESULT CProgress::GetProgressDlgHandle(long* hWnd)
{
	*hWnd=(long)NULL;

	return S_OK;
}

HRESULT CProgress::SetUILook(UINT uiLook)
{
	return S_OK;
}
