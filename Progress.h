// Progress.h: interface for the CProgress class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PROGRESS_H__56B312F0_1E58_4FED_ADFE_8A7CC4D4106D__INCLUDED_)
#define AFX_PROGRESS_H__56B312F0_1E58_4FED_ADFE_8A7CC4D4106D__INCLUDED_

#include "Common.h"

class CProgress  
{
public:
	CProgress();
	virtual ~CProgress();

	HRESULT EnableCancel(BOOL bCanCancel);
	HRESULT SetCaption(char* szCaption);
	HRESULT GetPosition(long * plVal);
	HRESULT SetPosition(long lPos);
	HRESULT Reset();
	HRESULT CheckCancel(BOOL * pbVal);
	HRESULT Show(BOOL bShow);
	HRESULT ClearAllInfo();
	HRESULT AddCurInfo(char* szInformation);
	HRESULT StepIt();
	HRESULT SetStep(long lStep);
	HRESULT SetRange(long lBegin,long lEnd);
	HRESULT GetProgressDlgHandle(long* hWnd);
	HRESULT SetUILook(UINT uiLook);

private:
	char* m_pszInfo;//task information
	long m_nBegin,m_nEnd,m_nPos,m_nStep;
};

#endif // !defined(AFX_PROGRESS_H__56B312F0_1E58_4FED_ADFE_8A7CC4D4106D__INCLUDED_)
