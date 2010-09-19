// Output.cpp: implementation of the COutput class.
//
//////////////////////////////////////////////////////////////////////

#include "Output.h"
#include "stdio.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

COutput::COutput()
{
	
}

COutput::~COutput()
{

}

void COutput::Output(char *szMessage)
{
#ifdef _OUTPUT_SILENCE_
	return;
#endif
	printf("%s\n",szMessage);
}

void COutput::OutputFileOpenFailed(char *pszFile)
{
#ifdef _OUTPUT_SILENCE_
	return;
#endif
	char szBuffer[256];
	sprintf(szBuffer,"%s open failed",pszFile);
	printf("%s\n",szBuffer);
}

void COutput::OutputMemoryError()
{
#ifdef _OUTPUT_SILENCE_
	return;
#endif
	printf("Memory Error\n");
}

void COutput::OutputImageDriverCreatedFailed()
{
#ifdef _OUTPUT_SILENCE_
	return;
#endif
	printf("Image driver created failed\n");
}

void COutput::OutputPixelTypeError()
{
#ifdef _OUTPUT_SILENCE_
	return;
#endif
	printf("Can not support the pixel datatype\n");
}

void COutput::OutputFileDoesNotExist(char *pszFile)
{
#ifdef _OUTPUT_SILENCE_
	return;
#endif
	char szBuffer[256];
	sprintf(szBuffer,"%s does not exist",pszFile);
	printf("%s\n",szBuffer);
}
