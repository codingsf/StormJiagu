// AppProtectorDlg.h : header file
//

#if !defined(AFX_APPPROTECTORDLG_H__C9A191F6_D50D_4A94_BFDB_E1235995223F__INCLUDED_)
#define AFX_APPPROTECTORDLG_H__C9A191F6_D50D_4A94_BFDB_E1235995223F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CAppProtectorDlg dialog

class CAppProtectorDlg : public CDialog
{
// Construction
public:
	CAppProtectorDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CAppProtectorDlg)
	enum { IDD = IDD_APPPROTECTOR_DIALOG };
	CString	m_ApkPath;

	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAppProtectorDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;
	char m_ApkName[100];
	//加壳之后存放Apk的路径
	char* m_OutputDir;
	
	//自身所在目录
	char* m_CurrentPath;
	//自身路径
	char* m_SelfPath;
	
	//ApkToolPath
	char* m_ApkToolPath;

	//提取Dex
	void ExtractOneFileFromApk(char*);
	void DecomplileApk();
	void DeleteSmali();
	void CopyPakcerSmali();
	void CopySo();
	void OverWriteApplication();
	void RecompileApk();
	void SignApk();

	// Generated message map functions
	//{{AFX_MSG(CAppProtectorDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnOpen();
	afx_msg void OnClose();
	afx_msg void OnProtect();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_APPPROTECTORDLG_H__C9A191F6_D50D_4A94_BFDB_E1235995223F__INCLUDED_)
