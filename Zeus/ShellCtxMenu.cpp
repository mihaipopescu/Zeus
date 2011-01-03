#include "stdafx.h"
#include "resource.h"
#include "priv.h"
#include "ShellExt.h"
#include "Zeus.h"

#include <shlobj.h>
#include <shlguid.h>
#include "io.h"

// utilities
#include "ShUtils.h"
#include "FileProcess.h"
#include "ProtectDlg.h"
#include "AboutDlg.h"
#include "..\ZipAES\ZeusProtection.h"             

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//
//  FUNCTION: CShellExt::Initialize(LPCITEMIDLIST, LPDATAOBJECT, HKEY)
//
//  PURPOSE: Called by the shell when initializing a context menu or property
//           sheet extension.
//
//  PARAMETERS:
//    pIDFolder - Specifies the parent folder
//    pDataObj  - Spefifies the set of items selected in that folder.
//    hRegKey   - Specifies the type of the focused item in the selection.
//
//  RETURN VALUE:
//
//    NOERROR in all cases.
//
//  COMMENTS:   Note that at the time this function is called, we don't know 
//              (or care) what type of shell extension is being initialized.  
//              It could be a context menu or a property sheet.
//

STDMETHODIMP CShellExt::Initialize(LPCITEMIDLIST pIDFolder,
								   LPDATAOBJECT pDataObj,
								   HKEY hRegKey)
{

	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

	ODS("CShellExtFld::Initialize()\r\n");

	// this can be called more than once
	if (m_pDataObj)
		m_pDataObj->Release();

	
   // duplicate the object pointer and registry handle
   if (pDataObj)
   {
	   m_pDataObj = pDataObj;
	   pDataObj->AddRef();
   } 
   else
   {
	   return E_FAIL;
   }
   
   
   HRESULT hret = GetSelectedFiles(pIDFolder, m_pDataObj, m_csaPaths);
   
   if(FAILED(hret))
	   return hret;

   if(m_csaPaths.GetSize()==1)
   {
	   m_zp.OpenArchive((LPCSTR)m_csaPaths.GetAt(0));
	   m_isProtected = m_zp.IsProtected();
	   m_zp.CloseArchive();
   }
   else
	   m_isProtected = false;

   return NOERROR;
}


/*----------------------------------------------------------------

FUNCTION: CShellExt::InvokeCommand(LPCMINVOKECOMMANDINFO)

PURPOSE: Called by the shell after the user has selected on of the
menu items that was added in QueryContextMenu(). This is 
the function where you get to do your real work!!!

PARAMETERS:
lpcmi - Pointer to an CMINVOKECOMMANDINFO structure

RETURN VALUE:


COMMENTS:

----------------------------------------------------------------*/

STDMETHODIMP CShellExt::InvokeCommand(LPCMINVOKECOMMANDINFO lpcmi)
{
	// look at all the MFC stuff in here! call this to allow us to 
	// not blow up when we try to use it.
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

	HINSTANCE hInst = AfxGetInstanceHandle();

	ODS("CShellExt::InvokeCommand()\r\n");

	HWND hParentWnd = lpcmi->hwnd;

	HRESULT hr = NOERROR;

	//If HIWORD(lpcmi->lpVerb) then we have been called programmatically
	//and lpVerb is a command that should be invoked.  Otherwise, the shell
	//has called us, and LOWORD(lpcmi->lpVerb) is the menu ID the user has
	//selected.  Actually, it's (menu ID - idCmdFirst) from QueryContextMenu().
	UINT idCmd = 0;
	CWnd *pParentWnd = NULL;
	CWinApp	*pWinApp = AfxGetApp(); 
	HWND hDlg;

	if (!HIWORD(lpcmi->lpVerb)) 
	{
		idCmd = LOWORD(lpcmi->lpVerb);

		// process it
		switch (idCmd) 
		{
		case 0: // operation 1					
				if (hParentWnd!=NULL)
					pParentWnd = CWnd::FromHandle(hParentWnd);

				// disable explorer
				::PostMessage(hParentWnd, WM_ENABLE, (WPARAM)FALSE, (LPARAM)0);

				
				CProtectDlg *pProtectDlg;
				

				try
				{
					pProtectDlg = new CProtectDlg;
				}
				catch (CMemoryException *e) 
				{
					e->ReportError();
					e->Delete();
					pProtectDlg=NULL;
					return E_FAIL;
				}

				// show it
				pProtectDlg->Create(IDD_WIZZARD, pParentWnd);

				hDlg = pProtectDlg->operator HWND();

				if(!m_isProtected && m_csaPaths.GetSize()>1)
				{
					CString fna = m_csaPaths.GetAt(0).Left(m_csaPaths.GetAt(0).ReverseFind('\\'));
					fna = fna.Mid(fna.ReverseFind('\\')+1);
					if(fna.GetLength()==2)
						pProtectDlg->SetState(true, "output");
					else
						pProtectDlg->SetState(true, fna);
				}
				else
				{
					pProtectDlg->SetState(!m_isProtected, m_csaPaths.GetAt(0).Mid(m_csaPaths.GetAt(0).ReverseFind('\\')+1));
				}

				if (::IsWindow(pProtectDlg->m_hWnd)) 
				{
					pProtectDlg->ShowWindow(SW_SHOW);
				}

				//pump dialog messages
				while(!pProtectDlg->m_result)
				{
					ShMsgPump(pWinApp);
				}
				
				//if user pressed ok button
				if(pProtectDlg->m_result == IDOK)
				{

					ThreadInfo is;

					is.bDone = FALSE;
					
					is.csOutFile = pProtectDlg->m_outfilename;
					
					is.csaFiles.Append(m_csaPaths);

					is.bProtect = !m_isProtected;
					is.iExOp = pProtectDlg->m_exfop;
					is.bRemoveDone = pProtectDlg->m_removeOrg;
					is.csPasswrod = pProtectDlg->m_password;
					is.iLevel = pProtectDlg->m_compresionLevel==0?0:pProtectDlg->m_compresionLevel*2-1;

					switch(pProtectDlg->m_encryptionStrength)
					{
					case 0:
						is.iPower = AES_128;
						break;
					case 1:
						is.iPower = AES_256;
						break;
					case 2:
						is.iPower = AES_RND;
						break;
					}

					is.iOverallDone = 0;

					//////////////////////////////////////////////////////
					// start the worker thread
					CWinThread *pThread = AfxBeginThread(FileProcessThreadFunc, 
						(LPVOID)&is,
						THREAD_PRIORITY_NORMAL,
						0,
						CREATE_SUSPENDED);

					//////////////////////////////////////////////////////
					// did we create OK?
					if (pThread ==NULL) 
					{
						ODS("Thread creation falied\n");
						break;
					}

					//////////////////////////////////////////////////////
					// start doing some real work

					// this will start the thread for real. 
					if (pThread->ResumeThread()==-1) 
					{
						ODS("Resume thread failure\n");
						break;
					}

					BOOL bCancel = FALSE;
					BOOL bDone = FALSE;
					int iCurPercentDone = 0, iAllDone=-1;
					int iLastPercentDone = 0, iAllLast=-1;
					char peDone[4];

					//////////////////////////////////////////////////////
					// now, we just sit back and wait
					// for the thread to signal finished

					pProtectDlg->ShowWindow(SW_SHOW);

					
					EnableWindow(GetDlgItem(hDlg, IDC_PASSWORD1), FALSE);
					EnableWindow(GetDlgItem(hDlg, IDC_PASSWORD2), FALSE);					

					EnableWindow(GetDlgItem(hDlg, IDC_OUTFILENAME), FALSE);
					EnableWindow(GetDlgItem(hDlg, IDC_COMPLEVEL), FALSE);
					EnableWindow(GetDlgItem(hDlg, IDC_ENCRYPTPOW), FALSE);

					EnableWindow(GetDlgItem(hDlg, IDC_EXF_APPEND), FALSE);
					EnableWindow(GetDlgItem(hDlg, IDC_EXF_BACKUP), FALSE);
					EnableWindow(GetDlgItem(hDlg, IDC_EXF_OVERWRITE), FALSE);

					EnableWindow(GetDlgItem(hDlg, IDC_REMOR), FALSE);
					while (!bDone) 
					{
						// test for cancel signal from the progress dialod
						bCancel = pProtectDlg->m_result!=IDOK?TRUE:FALSE;

						if (bCancel) 
						{
							ODS("bCancel (main)\n");
							is.zp.ZeusHalt = true;
							while(!is.bDone){ShMsgPump(pWinApp);};
							break;
						}

						// test for thread finished
						bDone = is.bDone;

						// get progress
						iCurPercentDone = is.zp.ZeusProgress;

						// update the progress meter
						if (iCurPercentDone!=iLastPercentDone)
						{
							pProtectDlg->m_crtFile = is.csCrtFile;
							pProtectDlg->m_progressCrt.SetPos(iCurPercentDone);
							pProtectDlg->UpdateData(0);
							iLastPercentDone = iCurPercentDone;
						}

						// overall progress
						iAllDone = is.iOverallDone;
						
						if(iAllDone!=iAllLast)
						{
							itoa(iAllDone, peDone, 10);
							pProtectDlg->m_provall = CString(peDone) + "% done";
							pProtectDlg->m_progressAll.SetPos(iAllDone);
							pProtectDlg->UpdateData(0);
							iAllLast = iAllDone;
						}

						// let the dialog breathe
						ShMsgPump(pWinApp);
					}

					pProtectDlg->ShowWindow(SW_HIDE);

					// handle any errors that may have popped up
					int iErr = is.iErr;

					if(iErr<0)
					{
						char ErrorStr[500];
						is.zp.GetLastError(ErrorStr);
						MessageBox(hDlg, ErrorStr, "ZEUS Error", MB_OK | MB_ICONSTOP);
					} // err
					else
						if(m_isProtected && pProtectDlg->m_removeOrg)
						{
							CString fn = m_csaPaths.GetAt(0);
							remove((LPCSTR)fn);
						}
					
					
				}//do zeus ?
				
				
				pProtectDlg->DestroyWindow();

				// enable explorer
				::PostMessage(hParentWnd, WM_ENABLE, (WPARAM)TRUE, (LPARAM)0);

			break;
		case 2: // operation 2
			{		
				if (hParentWnd!=NULL)
					pParentWnd = CWnd::FromHandle(hParentWnd);

				// disable explorer
				::PostMessage(hParentWnd, WM_ENABLE, (WPARAM)FALSE, (LPARAM)0);


				CAboutDlg *pAboutDlg;

				try
				{
					pAboutDlg = new CAboutDlg;
				}
				catch (CMemoryException *e) 
				{
					e->ReportError();
					e->Delete();
					pAboutDlg=NULL;
					return E_FAIL;
				}

				// show it
				pAboutDlg->Create(pParentWnd);
				if (::IsWindow(pAboutDlg->m_hWnd)) 
				{
					pAboutDlg->ShowWindow(SW_SHOW);
				}

				if(m_isProtected)
				{
					CString str = m_csaPaths.GetAt(0);
					m_zp.OpenArchive((LPCSTR)str);
					int ratio = m_zp.GetArchiveRatio();
					pAboutDlg->m_ratioBar.SetPos(ratio);
					m_zp.CloseArchive();
				}
				
				while(!pAboutDlg->result)
				{
					ShMsgPump(pWinApp);
				}

				pAboutDlg->ShowWindow(SW_HIDE);
				pAboutDlg->DestroyWindow();

				::PostMessage(hParentWnd, WM_ENABLE, (WPARAM)TRUE, (LPARAM)0);
			}
			break;
		}	// switch on command
	}

	return hr;
}


////////////////////////////////////////////////////////////////////////
//
//  FUNCTION: CShellExt::GetCommandString(...)
//
//  PURPOSE: Retrieve various text strinsg associated with the context menu
//
//	Param			Type			Use
//	-----			----			---
//	idCmd			UINT			ID of the command
//	uFlags			UINT			which type of info are we requesting
//	reserved		UINT *			must be NULL
//	pszName			LPSTR			output buffer
//	cchMax			UINT			max chars to copy to pszName
//
////////////////////////////////////////////////////////////////////////

STDMETHODIMP CShellExt::GetCommandString(UINT idCmd,
										 UINT uFlags,
										 UINT FAR *reserved,
										 LPSTR pszName,
										 UINT cchMax)
{
	ODS("CShellExt::GetCommandString()\r\n");

	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

	HINSTANCE hInst = AfxGetInstanceHandle();

	switch (uFlags) 
   {
	case GCS_HELPTEXT:		// fetch help text for display at the bottom of the 
							// explorer window
		switch (idCmd)
		{
			case 0:
				strncpy(pszName, m_isProtected?"Unprotect the file using Zeus":"Protect the file using Zeus", cchMax);
				break;
			case 2:
				strncpy(pszName, "About...", cchMax);
				break;
         default:
				strncpy(pszName, SHELLEXNAME, cchMax);
				break;
		}
		break;

	case GCS_VALIDATE:
		break;

	case GCS_VERB:			// language-independent command name for the menu item 
		switch (idCmd)
		{
			case 0:
            strncpy(pszName, m_isProtected?"Unprotect":"Protect", cchMax);
				break;
			case 2:
            strncpy(pszName, "About", cchMax);
				break;
         default:
            strncpy(pszName, SHELLEXNAME, cchMax);
				break;
		}

		break;
	}
    return NOERROR;
}

///////////////////////////////////////////////////////////////////////////
//
//  FUNCTION: CShellExt::QueryContextMenu(HMENU, UINT, UINT, UINT, UINT)
//
//  PURPOSE: Called by the shell just before the context menu is displayed.
//           This is where you add your specific menu items.
//
//  PARAMETERS:
//    hMenu      - Handle to the context menu
//    indexMenu  - Index of where to begin inserting menu items
//    idCmdFirst - Lowest value for new menu ID's
//    idCmtLast  - Highest value for new menu ID's
//    uFlags     - Specifies the context of the menu event
//
//  RETURN VALUE:
//		number of menus inserted
//
//  COMMENTS:
//
///////////////////////////////////////////////////////////////////////////

STDMETHODIMP CShellExt::QueryContextMenu(HMENU hMenu,
										 UINT indexMenu,
										 UINT idCmdFirst,
										 UINT idCmdLast,
										 UINT uFlags)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
   
   BOOL bAppendItems=TRUE;

   if ((uFlags & 0x000F) == CMF_NORMAL)  //Check == here, since CMF_NORMAL=0
   {
      ODS("CMF_NORMAL...\r\n");
   }
   else
   if (uFlags & CMF_VERBSONLY)
   {
      ODS("CMF_VERBSONLY...\r\n");
   }
   else
   if (uFlags & CMF_EXPLORE)
   {
      ODS("CMF_EXPLORE...\r\n");
   }
   else
   if (uFlags & CMF_DEFAULTONLY)
   {
      ODS("CMF_DEFAULTONLY...\r\n");
      bAppendItems = FALSE;
   }
   else
   {
	  ODS("CMF_UNKNOWN...\r\n");
      bAppendItems = FALSE;
   }

   if (bAppendItems)
   {
      HMENU hParentMenu;
	  UINT idCmd = idCmdFirst;

      HBITMAP hbmp1 = (HBITMAP)m_menuBmp.GetSafeHandle();
	  HBITMAP hbmp2 = (HBITMAP)m_aboutBmp.GetSafeHandle();

	  if (!m_csaPaths.IsEmpty())
	  {
		  // add popup
		  hParentMenu = ::CreateMenu();

		  if(hParentMenu)
		  {
			  InsertMenu(hMenu, indexMenu++, MF_POPUP|MF_BYPOSITION, (UINT)hParentMenu, SHELLEXNAME);
			  SetMenuItemBitmaps(hMenu, indexMenu-1, MF_BITMAP | MF_BYPOSITION, hbmp1, hbmp1);

			  InsertMenu(hParentMenu, indexMenu++, MF_STRING|MF_BYPOSITION, idCmd++, m_isProtected?"Unprotect":"Protect");
			  
			  InsertMenu(hParentMenu, indexMenu++, MF_SEPARATOR|MF_BYPOSITION, 0, NULL); idCmd++;

			  InsertMenu(hParentMenu, indexMenu++, MF_STRING|MF_BYPOSITION, idCmd++, "About");
			  SetMenuItemBitmaps(hParentMenu, indexMenu-2, MF_BITMAP | MF_BYPOSITION, hbmp2, hbmp2);

		  }		
	  }
	  else
	  {
		  InsertMenu(hMenu, indexMenu, MF_STRING|MF_BYPOSITION|MF_GRAYED|MF_DISABLED, idCmd++, SHELLEXNAME);			   
		  SetMenuItemBitmaps(hMenu, indexMenu, MF_BITMAP | MF_BYPOSITION, hbmp1, hbmp1);
	  }


	  return ResultFromShort(idCmd-idCmdFirst);
   }

   return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, 1);
}

