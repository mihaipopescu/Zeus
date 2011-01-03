#pragma once
#include "afxcmn.h"
#include "afxwin.h"


// CPassDlg dialog

class CPassDlg : public CDialog
{
	DECLARE_DYNAMIC(CPassDlg)

public:
	CPassDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CPassDlg();

// Dialog Data
	enum { IDD = IDD_PWD };
	CWnd *		m_parent;

public:
	virtual BOOL Create(CWnd* pParentWnd);
	
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void PostNcDestroy();


	DECLARE_MESSAGE_MAP()
public:
	int m_result;
public:
	CString m_password;
public:
	afx_msg void OnEnChangePassword();
public:
	afx_msg void OnBnClickedOk();
public:
	CProgressCtrl m_progress;
public:
	afx_msg void OnClose();
protected:
	HBITMAP m_hbOk, m_hbErr;
	CString m_password2;
	bool m_isProcessing;
public:
	afx_msg void OnEnChangePassword2();
public:
	CString m_status;
public:
	CButton m_btn;
public:
	void SetProcessingState();
	void UpdateStatus(CString status);
protected:
	virtual void OnCancel();
public:
	int m_complevel;
};
