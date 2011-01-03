#include "stdafx.h"
#include "resource.h"
#include "priv.h"
#include "ShellExt.h"
#include "Zeus.h"

#include <shlobj.h>
#include <shlguid.h>

#include "ShUtils.h"

/*

	What follows is our implementation of the IClassFactory interface.
	This code is basically unmodified from the SHELLEX example.

*/



/***************************************************************
	Function:
	CShellExtClassFactory::CShellExtClassFactory()

	Purpose:
	constructor

	Param			Type			Use
	-----			----			---

	Returns
	-------

***************************************************************/
CShellExtClassFactory::CShellExtClassFactory()
{
    ODS("CShellExtClassFactory::CShellExtClassFactory()\r\n");

    m_cRef = 0L;

    g_cRefThisDll++;	
}

/***************************************************************
	Function:
	CShellExtClassFactory::~CShellExtClassFactory()	

	Purpose:
	destructor

	Param			Type			Use
	-----			----			---

	Returns
	-------

***************************************************************/																
CShellExtClassFactory::~CShellExtClassFactory()				
{
    g_cRefThisDll--;
}

/***************************************************************
	Function:
	STDMETHODIMP CShellExtClassFactory::QueryInterface(REFIID riid,
                                                   LPVOID FAR *ppv)

	Purpose:
	Return a ptr to an interface

	Param			Type			Use
	-----			----			---
	riid			REFIID			interface to get
	ppv				LPVOID *		ptr to rec our interface

	Returns
	-------
	NOERROR on successz

***************************************************************/
STDMETHODIMP CShellExtClassFactory::QueryInterface(REFIID riid,
                                                   LPVOID FAR *ppv)
{
    ODS("CShellExtClassFactory::QueryInterface()\r\n");

    *ppv = NULL;

    // Any interface on this object is the object pointer

    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IClassFactory))
    {
        *ppv = (LPCLASSFACTORY)this;

        AddRef();

        return NOERROR;
    }

    return E_NOINTERFACE;
}	

/***************************************************************
	Function:
	STDMETHODIMP_(ULONG) CShellExtClassFactory::AddRef()

	Purpose:
	Increase our object's reference count

	Param			Type			Use
	-----			----			---

	Returns
	-------
	# of references

***************************************************************/
STDMETHODIMP_(ULONG) CShellExtClassFactory::AddRef()
{
    return ++m_cRef;
}

/***************************************************************
	Function:
	STDMETHODIMP_(ULONG) CShellExtClassFactory::Release()

	Purpose:
	Decrease our object's reference count

	Param			Type			Use
	-----			----			---

	Returns
	-------
	# of references

***************************************************************/

STDMETHODIMP_(ULONG) CShellExtClassFactory::Release()
{
    if (--m_cRef)
        return m_cRef;

    delete this;

    return 0L;
}

/***************************************************************
	Function:
	STDMETHODIMP CShellExtClassFactory::CreateInstance(LPUNKNOWN pUnkOuter,
                                                      REFIID riid,
                                                      LPVOID *ppvObj)

	Purpose:
	Create an instace of a particular interface

	Param			Type			Use
	-----			----			---
	pUnkOuter		LPUNKNOWN		outer IUnknown
	riid			REFIID			interface ID
	ppvObj			LPVOID *		ptr to rec our interface
	Returns
	-------
	NOERROR on success

***************************************************************/
STDMETHODIMP CShellExtClassFactory::CreateInstance(LPUNKNOWN pUnkOuter,
                                                      REFIID riid,
                                                      LPVOID *ppvObj)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

   ODS("CShellExtClassFactory::CreateInstance()\r\n");

   *ppvObj = NULL;

   // Shell extensions typically don't support aggregation (inheritance)

   if (pUnkOuter)
      return CLASS_E_NOAGGREGATION;

   // Create the main shell extension object.  The shell will then call
   // QueryInterface with IID_IShellExtInit--this is how shell extensions are
   // initialized.

   LPCSHELLEXT pShellExt;
   try 
   {
      pShellExt = new CShellExt();  //Create the CShellExt object
   } catch (CMemoryException *e) {
      e->Delete();
      pShellExt=NULL;
   }


   if (NULL == pShellExt)
      return E_OUTOFMEMORY;

   return pShellExt->QueryInterface(riid, ppvObj);
}

/***************************************************************
	Function:
	STDMETHODIMP CShellExtClassFactory::LockServer(BOOL fLock)

	Purpose:
	Pretend we did something

	Param			Type			Use
	-----			----			---

	Returns
	-------
	NOERROR, always

***************************************************************/
STDMETHODIMP CShellExtClassFactory::LockServer(BOOL fLock)
{
    return NOERROR;
}
