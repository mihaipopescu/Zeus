#include "stdafx.h"
#include "resource.h"
#include "ShUtils.h"
#include "errno.h"
#include <direct.h>
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CRITICAL_SECTION g_critSectionBreak;

////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////
void ShMsgPump(CWinApp *pWinApp)
{
	// if we do MFC stuff in an exported fn, call this first!
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

	DWORD dInitTime = GetTickCount();
	MSG m_msgCur;                   // current message	
	while (::PeekMessage(&m_msgCur, NULL, NULL, NULL, PM_NOREMOVE)  &&
			(GetTickCount() - dInitTime < 100) )
	{
		pWinApp->PumpMessage();
	}
}

////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////
int WideCharToLocal(LPTSTR pLocal, LPWSTR pWide, DWORD dwChars)
{
	*pLocal = 0;

	#ifdef UNICODE
	lstrcpyn(pLocal, pWide, dwChars);
	#else
	WideCharToMultiByte( CP_ACP, 
						 0, 
						 pWide, 
						 -1, 
						 pLocal, 
						 dwChars, 
						 NULL, 
						 NULL);
	#endif

	return lstrlen(pLocal);
}

////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////
STDMETHODIMP GetSelectedFiles(LPCITEMIDLIST pIDFolder,
							  LPDATAOBJECT &pDataObj,
							  CStringArray &csaPaths)
{
	// get these paths into a CStringArray
	csaPaths.RemoveAll();

	// fetch all of the file names we're supposed to operate on
	if (pDataObj) 
	{
		pDataObj->AddRef();

		STGMEDIUM medium;
		FORMATETC fe = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};


		HRESULT hr = pDataObj->GetData (&fe, &medium);
		if (FAILED (hr))
		{
			return E_FAIL;
		}

		// buffer to receive filenames
		char path[MAX_PATH];

		// how many are there?
		UINT fileCount = DragQueryFile((HDROP)medium.hGlobal, 0xFFFFFFFF,
			path, MAX_PATH);

		if (fileCount>0)
		{
			// avoid wasting mem when this thing gets filled in
			csaPaths.SetSize(fileCount);

			// stash the paths in our CStringArray
			for (UINT i=0;i<fileCount;i++) 
			{
				// clear old path
				memset(path, 0, MAX_PATH);
				// fetch new path
				if (DragQueryFile((HDROP)medium.hGlobal, i, path, MAX_PATH)) 
				{
					csaPaths.SetAt(i, path);
				}
			}

			csaPaths.FreeExtra();
		}

		// free our path memory - we have the info in our CStringArray
		ReleaseStgMedium(&medium);
	}

	return NOERROR;
}