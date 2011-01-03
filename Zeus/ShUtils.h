#ifndef SHUTILSH
#define SHUTILSH

#include "ThreadStruct.h"

// shell extension name
#define	SHELLEXNAME				"Zeus"

#ifdef _DEBUG
#define ODS(sz) {if (strlen(sz)>0) {OutputDebugString(SHELLEXNAME); OutputDebugString("=>");} OutputDebugString(sz);}
#else
#define ODS(x)
#endif

#define ResultFromShort(i)  ResultFromScode(MAKE_SCODE(SEVERITY_SUCCESS, 0, (USHORT)(i)))


// pump msgs
extern void ShMsgPump(CWinApp *pWinApp);

// conversion routine
extern int  WideCharToLocal(LPTSTR pLocal, LPWSTR pWide, DWORD dwChars);

STDMETHODIMP GetSelectedFiles(LPCITEMIDLIST pIDFolder,  LPDATAOBJECT &pDataObj, CStringArray &csaPaths);

#endif