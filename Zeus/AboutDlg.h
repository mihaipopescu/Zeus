#pragma once
#include "afxcmn.h"


// CAboutDlg dialog

class CAboutDlg : public CDialog
{
	DECLARE_DYNAMIC(CAboutDlg)

public:
	CAboutDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUT };
	CWnd *		m_parent;

public:
	virtual BOOL Create(CWnd* pParentWnd);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void PostNcDestroy();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOkabout();
public:
	int result;
public:
	CProgressCtrl m_ratioBar;
public:
	afx_msg void OnClose();
protected:
	virtual void OnCancel();
};
