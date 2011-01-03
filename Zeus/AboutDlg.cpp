// AboutDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Zeus.h"
#include "AboutDlg.h"


// AboutDlg dialog

IMPLEMENT_DYNAMIC(CAboutDlg, CDialog)

CAboutDlg::CAboutDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAboutDlg::IDD, pParent)
{
	result = 0;
}

CAboutDlg::~CAboutDlg()
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RATIO, m_ratioBar);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	ON_BN_CLICKED(ID_OKABOUT, &CAboutDlg::OnBnClickedOkabout)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// AboutDlg message handlers

void CAboutDlg::OnBnClickedOkabout()
{
	result = 1;
	OnOK();
}


BOOL CAboutDlg::Create(CWnd* pParentWnd) 
{
	m_parent=pParentWnd;
	return CDialog::Create(IDD, pParentWnd);
}

void CAboutDlg::PostNcDestroy() 
{
	// modeless dialogs delete themselves
	delete this;
}

BOOL CAboutDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CenterWindow();

	return TRUE;
}

void CAboutDlg::OnClose()
{
	result = 1;

	CDialog::OnClose();
}

void CAboutDlg::OnCancel()
{
	result = 1;

	CDialog::OnCancel();
}