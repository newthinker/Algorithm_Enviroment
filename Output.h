// Output.h: interface for the COutput class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_OUTPUT_H__A51591A6_5F4B_4D24_A3E1_C6FFDAA87D66__INCLUDED_)
#define AFX_OUTPUT_H__A51591A6_5F4B_4D24_A3E1_C6FFDAA87D66__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define _OUTPUT_SILENCE_

class COutput  
{
public:
	static void OutputFileDoesNotExist(char* pszFile);
	static void OutputPixelTypeError();
	static void OutputImageDriverCreatedFailed();
	static void OutputMemoryError();
	static void OutputFileOpenFailed(char* pszFile);
	static void Output(char* szMessage);
	
	COutput();
	virtual ~COutput();

};

#endif // !defined(AFX_OUTPUT_H__A51591A6_5F4B_4D24_A3E1_C6FFDAA87D66__INCLUDED_)
