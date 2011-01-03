// ProtectDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Zeus.h"
#include "ProtectDlg.h"
#include "..\ZipAES\ZeusProtection.h"

// CProtectDlg dialog

IMPLEMENT_DYNAMIC(CProtectDlg, CDialog)

CProtectDlg::CProtectDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CProtectDlg::IDD, pParent)
	, m_outfilename(_T(""))
	, m_compresionLevel(0)
	, m_password(_T(""))
	, m_password2(_T(""))
	, m_encryptionStrength(0)
	, m_crtFile(_T("Current file"))
	, m_provall(_T("Overall progress"))
	, m_removeOrg(FALSE)
	, m_bProtect(true)
{
	m_result = 0;
	
	m_exfop = EXFILE_CREATEBACKUP;
}

CProtectDlg::~CProtectDlg()
{
}

void CProtectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_OUTFILENAME, m_outfilename);
	DDX_CBIndex(pDX, IDC_COMPLEVEL, m_compresionLevel);
	DDX_Text(pDX, IDC_PASSWORD1, m_password);
	DDX_Text(pDX, IDC_PASSWORD2, m_password2);
	DDX_CBIndex(pDX, IDC_ENCRYPTPOW, m_encryptionStrength);
	DDX_Control(pDX, IDC_PROGRESS, m_progressCrt);
	DDX_Control(pDX, IDC_PROGRESS2, m_progressAll);
	DDX_Text(pDX, IDC_CRTFILE, m_crtFile);
	DDX_Text(pDX, IDC_PROVALL, m_provall);
	DDX_Check(pDX, IDC_REMOR, m_removeOrg);
}


BEGIN_MESSAGE_MAP(CProtectDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CProtectDlg::OnBnClickedOk)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDCANCEL, &CProtectDlg::OnBnClickedCancel)
	ON_EN_CHANGE(IDC_PASSWORD2, &CProtectDlg::OnEnChangePassword2)
	ON_BN_CLICKED(IDC_EXF_BACKUP, &CProtectDlg::OnBnClickedExfBackup)
	ON_BN_CLICKED(IDC_EXF_APPEND, &CProtectDlg::OnBnClickedExfAppend)
	ON_BN_CLICKED(IDC_EXF_OVERWRITE, &CProtectDlg::OnBnClickedExfOverwrite)
	ON_EN_CHANGE(IDC_PASSWORD1, &CProtectDlg::OnEnChangePassword1)
END_MESSAGE_MAP()


// CProtectDlg message handlers



void CProtectDlg::OnBnClickedOk()
{
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

	if(m_bProtect)
	{
		if(m_password.Compare(m_password2)!=0)
		{
			AfxMessageBox("Passwords do not match !\nPlease retype your password again for security reasons.", MB_OK|MB_ICONASTERISK);
			return;
		}
	}

	m_result = IDOK;
	GetDlgItem(IDOK)->EnableWindow(FALSE);
	OnOK();
}

BOOL CProtectDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CenterWindow();

	GetDlgItem(IDOK)->EnableWindow(FALSE);
	CheckDlgButton(IDC_EXF_BACKUP, BST_CHECKED);
	GetDlgItem(IDC_COMPLEVEL)->SendMessage(CB_SETCURSEL, 3, 0);

	return TRUE;
}

void CProtectDlg::SetState(bool protect, CString fname)
{
	m_bProtect = protect;

	if(protect) 
	{
		fname+=".zeus";
		SetDlgItemText(IDC_OUTFILENAME, (LPCSTR)(fname));
	}
	else
	{
		SetWindowText("Zeus UnProtection Wizzard");
		SetDlgItemText(IDC_OUTFILENAME, (LPCSTR)(fname));
		GetDlgItem(IDC_OUTFILENAME)->EnableWindow(FALSE);
		GetDlgItem(IDC_COMPLEVEL)->EnableWindow(FALSE);
		GetDlgItem(IDC_ENCRYPTPOW)->EnableWindow(FALSE);
		GetDlgItem(IDC_PASSWORD2)->EnableWindow(FALSE);
		GetDlgItem(IDC_PASSWORD1)->SetFocus();
	}
}

void CProtectDlg::PostNcDestroy()
{
	delete this;
}

void CProtectDlg::OnClose()
{
	m_result = IDCANCEL;
	CDialog::OnClose();
}

void CProtectDlg::OnCancel()
{
	m_result = IDCANCEL;
	CDialog::OnCancel();
}
void CProtectDlg::OnBnClickedCancel()
{
	m_result = IDCANCEL;
	OnCancel();
}

void CProtectDlg::OnEnChangePassword2()
{
	UpdateData();
	if(m_outfilename.GetLength()>0 && m_password.GetLength()>0 && m_password.Compare(m_password2)==0)
		GetDlgItem(IDOK)->EnableWindow(TRUE);
	else
		GetDlgItem(IDOK)->EnableWindow(FALSE);
}

void CProtectDlg::OnBnClickedExfBackup()
{
	m_exfop = EXFILE_CREATEBACKUP;
}


void CProtectDlg::OnBnClickedExfAppend()
{
	m_exfop = EXFILE_APPEND;
}

void CProtectDlg::OnBnClickedExfOverwrite()
{
	m_exfop = EXFILE_OVERWRITE;
}

void CProtectDlg::OnEnChangePassword1()
{
	UpdateData();
	if(m_bProtect)
	{
		if(m_outfilename.GetLength()>0 && m_password2.GetLength()>0 && m_password.Compare(m_password2)==0)
			GetDlgItem(IDOK)->EnableWindow(TRUE);
		else
			GetDlgItem(IDOK)->EnableWindow(FALSE);
	} 
	else
	{
		if(m_password.GetLength()>0)
			GetDlgItem(IDOK)->EnableWindow(TRUE);
		else
			GetDlgItem(IDOK)->EnableWindow(FALSE);
	}
}
