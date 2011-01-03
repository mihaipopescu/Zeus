#include "stdafx.h"
#include "resource.h"
#include "priv.h"
#include "ShellExt.h"
#include "Zeus.h"

#include <shlobj.h>
#include <shlguid.h>

#include "ShUtils.h"


/*
	This code is a modified version of SHELLEX (source: MSDN)
*/
// *********************** CShellExt *************************

/***************************************************************
	Function:
	CShellExt::CShellExt()

	Purpose:
	contructor

	Param			Type			Use
	-----			----			---

	Returns
	-------

***************************************************************/
CShellExt::CShellExt()
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

   ODS("CShellExt::CShellExt()\r\n");

   m_cRef = 0L;
   m_pDataObj = NULL;

   g_cRefThisDll++;   

   m_menuBmp.LoadBitmap(IDB_MENU_BMP);
   m_aboutBmp.LoadBitmap(IDB_ABOUT_BMP);
}

/***************************************************************
	Function:
	CShellExt::~CShellExt()

	Purpose:
	destructor

	Param			Type			Use
	-----			----			---

	Returns
	-------

***************************************************************/
CShellExt::~CShellExt()
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

   if (m_pDataObj)
      m_pDataObj->Release();

   g_cRefThisDll--;

   if (m_menuBmp.GetSafeHandle())
      m_menuBmp.DeleteObject();

   if (m_aboutBmp.GetSafeHandle())
      m_aboutBmp.DeleteObject();
}

/***************************************************************
	Function:
	STDMETHODIMP CShellExt::QueryInterface(REFIID riid, LPVOID FAR *ppv)

	Purpose:
	See if we support a given interface

	Param			Type			Use
	-----			----			---
	riid			REFIID			interface ID
	ppv				LPVOID *		ptr to rec our interface

	Returns
	-------
	NOERROR on success

***************************************************************/
STDMETHODIMP CShellExt::QueryInterface(REFIID riid, LPVOID FAR *ppv)
{
   *ppv = NULL;

   if (IsEqualIID(riid, IID_IShellExtInit) || IsEqualIID(riid, IID_IUnknown))
   {
      ODS("CShellExt::QueryInterface()==>IID_IShellExtInit\r\n");

      *ppv = (LPSHELLEXTINIT)this;
   }
   else if (IsEqualIID(riid, IID_IContextMenu))
   {
      ODS("CShellExt::QueryInterface()==>IID_IContextMenu\r\n");

      *ppv = (LPCONTEXTMENU)this;
   }
   else if (IsEqualIID(riid, IID_IShellIconOverlayIdentifier))
   {
      ODS("CShellExt::QueryInterface()==>IID_IShellIconIdentifier\r\n");

	  *ppv = (IShellIconOverlayIdentifier*)this;
   }

   if (*ppv)
   {
      AddRef();
      return NOERROR;
   }

   ODS("CShellExt::QueryInterface()==>Unknown Interface!\r\n");

   return E_NOINTERFACE;
}

/***************************************************************
	Function:
	STDMETHODIMP_(ULONG) CShellExt::AddRef()

	Purpose:
	increase our reference count

	Param			Type			Use
	-----			----			---

	Returns
	-------
	# of references

***************************************************************/
STDMETHODIMP_(ULONG) CShellExt::AddRef()
{
   ODS("CShellExt::AddRef()\r\n");

   return ++m_cRef;
}

/***************************************************************
	Function:
	STDMETHODIMP_(ULONG) CShellExt::Release()

	Purpose:
	decrease our reference count, clean up

	Param			Type			Use
	-----			----			---

	Returns
	-------
	# of references

***************************************************************/
STDMETHODIMP_(ULONG) CShellExt::Release()
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

   ODS("CShellExt::Release()\r\n");

   if (--m_cRef)
      return m_cRef;

   delete this;

   return 0L;
}

