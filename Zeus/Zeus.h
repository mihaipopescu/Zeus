// Zeus.h : main header file for the Zeus DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CZeusApp
// See Zeus.cpp for the implementation of this class
//

class CZeusApp : public CWinApp
{
public:
	CZeusApp();

// Overrides
public:
	virtual BOOL InitInstance();
	virtual BOOL ExitInstance();

	DECLARE_MESSAGE_MAP()
};

extern UINT      g_cRefThisDll;

// {FAC46F68-5FC1-4d4e-B03E-E2DC3C1DA351}
DEFINE_GUID(CLSID_ZeusShellExtID, 
0xfac46f68, 0x5fc1, 0x4d4e, 0xb0, 0x3e, 0xe2, 0xdc, 0x3c, 0x1d, 0xa3, 0x51);