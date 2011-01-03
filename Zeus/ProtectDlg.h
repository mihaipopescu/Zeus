#pragma once
#include "afxcmn.h"


// CProtectDlg dialog

class CProtectDlg : public CDialog
{
	DECLARE_DYNAMIC(CProtectDlg)

public:
	CProtectDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CProtectDlg();

// Dialog Data
	enum { IDD = IDD_WIZZARD };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void PostNcDestroy();
	virtual void OnCancel();

	DECLARE_MESSAGE_MAP()
public:
	CString m_outfilename;
public:
	int m_compresionLevel;
public:
	CString m_password;
public:
	CString m_password2;
public:
	int m_encryptionStrength;
public:
	int m_result;
public:
	afx_msg void OnBnClickedOk();
public:
	afx_msg void OnClose();
public:
	afx_msg void OnBnClickedCancel();
public:
	CProgressCtrl m_progressCrt;
public:
	CProgressCtrl m_progressAll;
public:
	CString m_crtFile;
public:
	CString m_provall;
public:
	afx_msg void OnEnChangePassword2();
public:
	int m_exfop;
public:
	afx_msg void OnBnClickedExfBackup();
public:
	afx_msg void OnBnClickedExfAppend();
public:
	afx_msg void OnBnClickedExfOverwrite();
public:
	BOOL m_removeOrg;
public:
	afx_msg void OnEnChangePassword1();
public:
	void SetState(bool protect, CString fname);
private:
	bool m_bProtect;
};
