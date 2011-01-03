#ifndef SHELLEXTDEFSH
#define SHELLEXTDEFSH

#include "..\aes\zipcrypt.h"
#include "..\ZipAES\ZeusProtection.h"

// this class factory object creates context menu handlers for Windows 95 shell
class CShellExtClassFactory : public IClassFactory
{
protected:
	ULONG        m_cRef;


public:
	CShellExtClassFactory();
	~CShellExtClassFactory();

	//IUnknown members
	STDMETHODIMP			QueryInterface(REFIID, LPVOID FAR *);
	STDMETHODIMP_(ULONG)	AddRef();
	STDMETHODIMP_(ULONG)	Release();

	//IClassFactory members
	STDMETHODIMP		CreateInstance(LPUNKNOWN, REFIID, LPVOID FAR *);
	STDMETHODIMP		LockServer(BOOL);

};
typedef CShellExtClassFactory *LPCSHELLEXTCLASSFACTORY;

// this is the actual OLE Shell context menu handler
class CShellExt : public IContextMenu, 
                         IShellExtInit,
						 IShellIconOverlayIdentifier
{
public:
  

protected:
	ULONG				m_cRef;
	LPDATAOBJECT		m_pDataObj;

   CBitmap				m_menuBmp;
   CBitmap				m_aboutBmp;
   
   AES_ExtraField		m_aef;
   CString				m_filename;
   CStringArray			m_csaPaths;
   bool					m_isProtected;
   ZeusProtection		m_zp;

	//////////////////////////////////////////////////////////////////

protected:

public:
	// generic shell ext functions
	CShellExt();
	~CShellExt();

	//IUnknown members
	STDMETHODIMP			QueryInterface(REFIID, LPVOID FAR *);
	STDMETHODIMP_(ULONG)	AddRef();
	STDMETHODIMP_(ULONG)	Release();

   //IShellExtInit methods
	STDMETHODIMP		    Initialize(LPCITEMIDLIST pIDFolder, 
                                          LPDATAOBJECT pDataObj, 
                                          HKEY hKeyID);

	//IShell members

   //////////////////////////////////////////////////////////////
   //
   // These are the only functions you really need to modify. They
   // are all in ShellCtxMenu.cpp
   
   //
   // called to get menu item strings for Explorer
   //
	STDMETHODIMP			QueryContextMenu(HMENU hMenu,
                                          UINT indexMenu, 
                                          UINT idCmdFirst,
                                          UINT idCmdLast, 
                                          UINT uFlags);
   //
   // called after a menu item has been selected
   //
	STDMETHODIMP			InvokeCommand(LPCMINVOKECOMMANDINFO lpcmi);

   //
   // fetch various strings for Explorer
   //
	STDMETHODIMP			GetCommandString(UINT idCmd, 
                                          UINT uFlags, 
                                          UINT FAR *reserved, 
                                          LPSTR pszName, 
                                          UINT cchMax);

   //
   //
   //////////////////////////////////////////////////////////////
  
	STDMETHODIMP			GetOverlayInfo(LPWSTR pwszIconFile, 
											int cchMax, 
											int *pIndex, 
											DWORD *pdwFlags);
	//
	//
	// Specifies the priority of an icon overlay.
	// This method is called only during initialization. It assigns a priority value 
	// (ranging from 0=Highest to 100=Lowest priority) to the handler's icon overlay.
	// Priority helps resolve the conflict when multiple handlers are installed.
	//
	STDMETHODIMP			GetPriority(int *pPriority); 

	// "Before painting an object's icon, the Shell passes the object's name to
	//  each icon overlay handler's IShellIconOverlayIdentifier::IsMemberOf
	//  method. If a handler wants to have its icon overlay displayed,
	//  it returns S_OK. The Shell then calls the handler's
	//  IShellIconOverlayIdentifier::GetOverlayInfo method to determine which icon
	//  to display." 

	STDMETHODIMP			IsMemberOf(LPCWSTR pwszPath, 
										DWORD dwAttrib);

};
typedef CShellExt *LPCSHELLEXT;


#endif