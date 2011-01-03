#include "stdafx.h"
#include "priv.h"
#include "ShellExt.h"
#include "ShUtils.h"
#include "..\ZipAES\ZeusProtection.h"


//
// This method is first called during initialization, it returns 
// the fully qualified path of the file containing the icon overlay image, 
// and its zero-based index within the file. The icon can be contained in any 
// of the standard file types, including .exe, .dll, and .ico.
//
STDMETHODIMP CShellExt::GetOverlayInfo(LPWSTR pwszIconFile, int cchMax, int *pIndex, DWORD *pdwFlags)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

	ODS("In GetOverlay Info\r\n");

	HINSTANCE hInst = AfxGetInstanceHandle();
	
	char buff[MAX_PATH];

	// Get's our dll path
	GetModuleFileName(hInst, (LPCH)buff, cchMax);
	
	mbstowcs(pwszIconFile, buff, cchMax);

	//Use icon in the resource
	*pIndex=0; 

	*pdwFlags = ISIOI_ICONFILE | ISIOI_ICONINDEX;

	return S_OK;
}	

// Our icon overlay priority
STDMETHODIMP CShellExt::GetPriority(int *pPriority)
{
	ODS("In GetPriority\r\n");

	//highest priority
	*pPriority = 0;

	return S_OK;
}


//
// Specifies whether an icon overlay should be added to a Shell object's icon.
// Returns S_OK to allow adding the overlay icon or S_FALSE to keep object's icon intact.
//
STDMETHODIMP CShellExt::IsMemberOf(LPCWSTR pwszPath, DWORD dwAttrib)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
	
	ODS("In IsMemberOf\r\n");

	if (pwszPath == NULL)
		return S_FALSE;

	// the shell sometimes asks overlays for invalid paths, e.g. for network
	// printers (in that case the path is "0", at least for me here).
	if (wcslen(pwszPath)<2)
		return S_FALSE;

	char buff[MAX_PATH];
	wcstombs(buff, pwszPath, MAX_PATH);

	m_zp.OpenArchive(buff, NULL, 0);
	HRESULT ret = m_zp.IsProtected()?S_OK:S_FALSE;
	m_zp.CloseArchive();

	return ret;
}