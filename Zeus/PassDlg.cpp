// PassDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Zeus.h"
#include "PassDlg.h"
#include "ShUtils.h"
#include "..\ZipAES\ZipAES.h"

// CPassDlg dialog

IMPLEMENT_DYNAMIC(CPassDlg, CDialog)

CPassDlg::CPassDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPassDlg::IDD, pParent)
	, m_password(_T(""))
	, m_password2(_T(""))
	, m_status(_T(""))
	, m_complevel(0)
{
	m_result = 0;
	m_isProcessing = false;
	HINSTANCE hInst = AfxGetInstanceHandle();
	m_hbErr = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_ERR));
	m_hbOk = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_ALLOK));
}

CPassDlg::~CPassDlg()
{
	DeleteObject(m_hbErr);
	DeleteObject(m_hbOk);
}

void CPassDlg::PostNcDestroy() 
{
	// modeless dialogs delete themselves
	delete this;
}

BOOL CPassDlg::Create(CWnd* pParentWnd) 
{
	m_parent=pParentWnd;
	return CDialog::Create(IDD, pParentWnd);
}

void CPassDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_PASSWORD, m_password);
	DDX_Text(pDX, IDC_PASSWORD2, m_password2);
	DDX_Control(pDX, IDC_PROGRESS, m_progress);
	DDX_Text(pDX, IDC_STATUS, m_status);
	DDX_Control(pDX, IDOK, m_btn);
	DDX_CBIndex(pDX, IDC_CMPLVL, m_complevel);
}


BEGIN_MESSAGE_MAP(CPassDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CPassDlg::OnBnClickedOk)	
	ON_EN_CHANGE(IDC_PASSWORD, &CPassDlg::OnEnChangePassword)
	ON_EN_CHANGE(IDC_PASSWORD2, &CPassDlg::OnEnChangePassword2)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CPassDlg message handlers


void CPassDlg::OnBnClickedOk()
{
	if(m_isProcessing)
	{
		iZeusStop = TRUE;
		OnCancel();
		return;
	}
	UpdateData();
	if(m_password.GetLength()<1)
	{
		AfxMessageBox("Password too small !\nPlease provide a password with at least 1 character.", MB_OK|MB_ICONASTERISK);
		return;
	}
	if(m_password.GetLength()>128)
	{
		AfxMessageBox("Password too big !\nPlease provide a password with at most 128 characters.", MB_OK|MB_ICONASTERISK);
		return;
	}
	if(m_password.Compare(m_password2)!=0)
	{
		AfxMessageBox("Passwords do not match !\nPlease retype your password again for security reasons.", MB_OK|MB_ICONASTERISK);
		return;
	}
	UpdateData(0);
	m_result = IDOK;
	OnOK();
}



BOOL CPassDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CenterWindow();
	m_progress.SetRange(0,100);
	m_btn.SetBitmap(m_hbErr);
	m_complevel = 3;
	UpdateData(0);

	return TRUE;
}


void CPassDlg::OnClose()
{
	m_result = IDCANCEL;
	iZeusStop = TRUE;
	CDialog::OnClose();
}

void CPassDlg::OnEnChangePassword2()
{
	OnEnChangePassword();
}

void CPassDlg::SetProcessingState()
{
	m_isProcessing = true;
	m_btn.SetBitmap(m_hbErr);
	UpdateData(0);
}

void CPassDlg::OnEnChangePassword()
{
	UpdateData();
	int len = m_password.GetLength();
	
	if(len<1)
	{
		m_btn.SetBitmap(m_hbErr);
		m_status = "Password too small! Please provide a password with at least 1 character.";
		UpdateData(0);
		return;
	}
	if(len>128)
	{
		m_btn.SetBitmap(m_hbErr);
		m_status = "Password too big! Please provide a password with at most 128 characters.";
		UpdateData(0);
		return;
	}
	if(m_password.Compare(m_password2)!=0)
	{
		m_btn.SetBitmap(m_hbErr);
		m_status = "Passwords do not match! Please retype your password again for security reasons.";
		UpdateData(0);
		return;
	}

	m_btn.SetBitmap(m_hbOk);
	if(len>=1 && len<8)
		m_status = "An 8 character password is recommanded for a good encryption strength.";
	else
		m_status = "All things seems to be Ok";
	UpdateData(0);
}

void CPassDlg::UpdateStatus(CString status)
{
	UpdateData();

	m_status = status;

	UpdateData(FALSE);
}


void CPassDlg::OnCancel()
{
	m_result = IDCANCEL;

	CDialog::OnCancel();
}
