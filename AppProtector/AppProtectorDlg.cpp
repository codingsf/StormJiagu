// AppProtectorDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AppProtector.h"
#include "AppProtectorDlg.h"
#include "Markup.h"
#include <direct.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "htziptools.h"
/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAppProtectorDlg dialog

CAppProtectorDlg::CAppProtectorDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAppProtectorDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAppProtectorDlg)
	m_ApkPath = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDI_MAINFRAME);
}

void CAppProtectorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAppProtectorDlg)
	DDX_Text(pDX, IDC_EDIT1, m_ApkPath);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAppProtectorDlg, CDialog)
	//{{AFX_MSG_MAP(CAppProtectorDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, OnOpen)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_Protect, OnProtect)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAppProtectorDlg message handlers

BOOL CAppProtectorDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	m_hIcon = AfxGetApp()->LoadIcon(IDI_MAINFRAME);
	m_SelfPath=(char*)malloc(MAX_PATH);
	m_CurrentPath=(char*)malloc(MAX_PATH);
	m_ApkToolPath=(char*)malloc(MAX_PATH);
	m_OutputDir=(char*)malloc(MAX_PATH);
	
	memset(m_SelfPath,0,MAX_PATH);
	memset(m_CurrentPath,0,MAX_PATH);
	memset(m_ApkToolPath,0,MAX_PATH);
	memset(m_OutputDir,0,MAX_PATH);
	
	GetModuleFileName(NULL,m_SelfPath,MAX_PATH);
	OutputDebugString(m_SelfPath);
	char* p=strrchr(m_SelfPath,'\\');
	
	memcpy(m_CurrentPath,m_SelfPath,(int)(p-m_SelfPath));
	OutputDebugString(m_CurrentPath);
	
	sprintf(m_ApkToolPath,"%s\\libs\\shaka.bat",m_CurrentPath);
	OutputDebugString(m_ApkToolPath);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CAppProtectorDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CAppProtectorDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CAppProtectorDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CAppProtectorDlg::OnOpen() 
{
	// TODO: Add your control notification handler code here
	const char pszFilter[] = _T("App File (*.apk)|*.apk|All Files (*.*)|*.*||");
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		pszFilter, this);
	char szInfo[MAX_PATH]={0};
	char szCurrentPath[MAX_PATH]={0};

	dlg.m_ofn.lpstrInitialDir = "c:\\WINDOWS\\";  //设置对话框默认呈现的路径


	if(dlg.DoModal() == IDOK)
	{
		m_ApkPath = dlg.GetPathName();
		UpdateData(false);
		//要加壳的App路径
		char* path=m_ApkPath.GetBuffer(m_ApkPath.GetLength());
		char* p=strrchr(path,'\\');
		//获取Apk文件名
		memcpy(m_ApkName,p+1,path+m_ApkPath.GetLength()-p);
		
		sprintf(szInfo,"选择的文件:%s",(char*)m_ApkName);
		OutputDebugString(szInfo);
		// 0x2e 表示 .
		p=strrchr(m_ApkName,0x2e);
		char szApkNameNotPrefix[100]={0};
		memcpy(szApkNameNotPrefix,m_ApkName,p-m_ApkName);
		sprintf(m_OutputDir,"%s\\Outputs\\%s",m_CurrentPath,szApkNameNotPrefix);

// 		sprintf(szInfo,"输出目录:%s",m_OutputDir);
// 		OutputDebugString(szInfo);

	}
	
}

void CAppProtectorDlg::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	free(m_SelfPath);
	free(m_CurrentPath);
	free(m_ApkToolPath);
	free(m_OutputDir);
//	AfxMessageBox("你确定要退出吗");
	CDialog::OnClose();
}


void CAppProtectorDlg::ExtractOneFileFromApk(char* oneFile)
{
	char* szApkPath=m_ApkPath.GetBuffer(m_ApkPath.GetLength());
	char szDexOutputPath[MAX_PATH]={0};
	
	sprintf(szDexOutputPath,"%s\\assets",m_OutputDir);
	
	ht_zipExtractOneFile(szApkPath,szDexOutputPath,oneFile,"encrypt.data");
//	ht_zipExtract(szApkPath,szDexOutputPath);
}

void CAppProtectorDlg::DecomplileApk()
{
	char szCommandLine[256] = { 0 };
	char szInfo[256]={0};
	char szApktoolDir[MAX_PATH]={0};	

	sprintf(szCommandLine, "\"%s\" d \"%s\" -o \"%s\" -f", m_ApkToolPath, m_ApkPath.GetBuffer(m_ApkPath.GetLength()), m_OutputDir);
	sprintf(szInfo,"commandLine:%s\n", szCommandLine);
	OutputDebugString(szInfo);
	
	char* p = strrchr(m_ApkToolPath, '\\');
	memcpy(szApktoolDir, m_ApkToolPath, (int)(p - (char*)m_ApkToolPath));
	sprintf(szInfo,"ApktoolDir:%s",szApktoolDir);
	
	SetCurrentDirectory(m_CurrentPath);

	STARTUPINFO si;	
	PROCESS_INFORMATION pi;	
	ZeroMemory(&pi, sizeof(pi));	
	ZeroMemory(&si, sizeof(si));	
	si.cb = sizeof(si);
	
	int status = CreateProcess(NULL,
								szCommandLine,
								NULL,
								NULL,
								TRUE,
								CREATE_NEW_CONSOLE,
								NULL,
								NULL,
								&si,
								&pi
								);
	if (!status)
	{
		OutputDebugString("\n[+]反编译失败");
		int error=GetLastError();
		sprintf(szInfo,"CreateProcess Error:%d\n",error);
		OutputDebugString(szInfo);
		return ;
	}
	
	HANDLE hChildProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pi.dwProcessId);	
	DWORD dwWait = WaitForSingleObject(hChildProcess, INFINITE);
	if (dwWait == WAIT_OBJECT_0)
	{
		OutputDebugString("[+]Apktool反编译完成");
	}
	
}

void CAppProtectorDlg::DeleteSmali()
{
	//删除要加壳apk的smali文件
	char szApkSmali[MAX_PATH] = { 0 };
	sprintf(szApkSmali, "%s\\smali", m_OutputDir);
	OutputDebugString(szApkSmali);
	SHFILEOPSTRUCT FileOp = { 0 };
	FileOp.fFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMATION; //允许放回回收站 不出现确认对话框
	FileOp.pFrom = szApkSmali;
	FileOp.pTo = NULL; //一定要是NULL
	FileOp.wFunc = FO_DELETE; //删除操作
	SHFileOperation(&FileOp);
}

void CAppProtectorDlg::CopyPakcerSmali()
{
	char szInfo[256]={0};
	char szCommandLine[256] = { 0 };

	//添加Java层的壳启动代码
	sprintf(szCommandLine,"xcopy \"%s\\libs\\PackerStartup\" \"%s\" /S",m_CurrentPath,m_OutputDir);
	sprintf(szInfo,"复制命令:%s",szCommandLine);
	OutputDebugString(szInfo);
	WinExec(szCommandLine,SW_HIDE);

}

void CAppProtectorDlg::OverWriteApplication()
{

	CMarkup xml;
	char szXmlPath[256] = { 0 };
	sprintf(szXmlPath, "%s\\AndroidManifest.xml", m_OutputDir);
	BOOL isLoad=xml.Load(szXmlPath);
	if (!isLoad)
	{
		OutputDebugString("读取AndroidManifest.xml失败");
		return ;
	}	
	xml.ResetMainPos();
	xml.FindElem();  
	//寻找application标签
	while (xml.FindChildElem("application"))
	{
		xml.SetChildAttrib("android:name", "com.storm.simpleandroidpacker.StubApplication");		
		xml.Save(szXmlPath);
	}
}

void CAppProtectorDlg::RecompileApk()
{
	char szCommandLine[300] = { 0 };
	char szInfo[256]={0};
	char szApktoolDir[MAX_PATH]={0};	
	char szNewApkPath[MAX_PATH]={0};

	sprintf(szNewApkPath,"%s\\Outputs\\new_%s",m_CurrentPath,m_ApkName);
	
	sprintf(szCommandLine, "\"%s\" b \"%s\" -o \"%s\" -f", m_ApkToolPath, m_OutputDir, szNewApkPath);
	sprintf(szInfo,"回编译命令:%s\n", szCommandLine);
	OutputDebugString(szInfo);
	
	
	char* p = strrchr(m_ApkToolPath, '\\');
	memcpy(szApktoolDir, m_ApkToolPath, (int)(p - (char*)m_ApkToolPath));
	sprintf(szInfo,"ApktoolDir:%s",szApktoolDir);
	
	SetCurrentDirectory(m_CurrentPath);


	STARTUPINFO si;	
	PROCESS_INFORMATION pi;	
	ZeroMemory(&pi, sizeof(pi));	
	ZeroMemory(&si, sizeof(si));	
	si.cb = sizeof(si);
	
	int status = CreateProcess(NULL,
								szCommandLine,
								NULL,
								NULL,
								TRUE,
								CREATE_NEW_CONSOLE,
								NULL,
								szApktoolDir,
								&si,
								&pi
								);
	if (!status)
	{
		OutputDebugString("\n[+]回编译失败");
		int error=GetLastError();
		sprintf(szInfo,"CreateProcess Error:%d\n",error);
		OutputDebugString(szInfo);
		return ;
	}
	
	HANDLE hChildProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pi.dwProcessId);	
	DWORD dwWait = WaitForSingleObject(hChildProcess, INFINITE);
	if (dwWait == WAIT_OBJECT_0)
	{
		OutputDebugString("[+]Apktool反编译完成");
	}
}

void CAppProtectorDlg::CopySo()
{
	char soPath[MAX_PATH]={0};
	char soNewPath[MAX_PATH]={0};
	char szDir[MAX_PATH]={0};

	sprintf(soPath,"%s\\libs\\libdexload.so",m_CurrentPath);
	sprintf(soNewPath,"%s\\lib\\armeabi\\libdexload.so",m_OutputDir);
	
	memcpy(szDir,m_OutputDir,strlen(m_OutputDir));
	DWORD fileAttr = GetFileAttributes(strcat(szDir,"\\lib"));
	if(fileAttr==-1  || !(fileAttr & FILE_ATTRIBUTE_DIRECTORY))
	{
		CreateDirectory(szDir,NULL);
		fileAttr=GetFileAttributes(strcat(szDir,"\\armeabi"));
		if(fileAttr==-1  || !(fileAttr & FILE_ATTRIBUTE_DIRECTORY))
			CreateDirectory(szDir,NULL);
	}
	CopyFile(soPath,soNewPath,false);

	sprintf(soPath,"%s\\libs\\libshella.so",m_CurrentPath);
	sprintf(soNewPath,"%s\\lib\\armeabi\\libshella.so",m_OutputDir);
	CopyFile(soPath,soNewPath,false);
	
} 

void CAppProtectorDlg::SignApk()
{
	char szCommandLine[300] = { 0 };
	char szInfo[256]={0};
	char szApktoolDir[MAX_PATH]={0};	

//	"D:\tools\AndroidKiller_v1.3.1\bin\apktool\signapk.bat" 
//	"D:\tools\AndroidKiller_v1.3.1\projects\alicloud_release\Bin\project.apk" 
//	"D:\tools\AndroidKiller_v1.3.1\projects\alicloud_release\Bin\project.apk.sign"
	char signApkPath[MAX_PATH]={0};
	
	sprintf(szCommandLine, "\"%s\\libs\\signapk.bat\"  \"%s\\Outputs\\new_%s\"  \"%s\\Outputs\\sign_new_%s\" ", m_CurrentPath, m_CurrentPath,m_ApkName,m_CurrentPath,m_ApkName);
	sprintf(szInfo,"签名命令:%s\n", szCommandLine);
	OutputDebugString(szInfo);
	
	
	char* p = strrchr(m_ApkToolPath, '\\');
	memcpy(szApktoolDir, m_ApkToolPath, (int)(p - (char*)m_ApkToolPath));
	sprintf(szInfo,"ApktoolDir:%s",szApktoolDir);
	
	SetCurrentDirectory(m_CurrentPath);
	
	
	STARTUPINFO si;	
	PROCESS_INFORMATION pi;	
	ZeroMemory(&pi, sizeof(pi));	
	ZeroMemory(&si, sizeof(si));	
	si.cb = sizeof(si);
	
	int status = CreateProcess(NULL,
								szCommandLine,
								NULL,
								NULL,
								TRUE,
								CREATE_NEW_CONSOLE,
								NULL,
								szApktoolDir,
								&si,
								&pi
								);
	if (!status)
	{
		OutputDebugString("签名失败");
		int error=GetLastError();
		sprintf(szInfo,"CreateProcess Error:%d\n",error);
		OutputDebugString(szInfo);
		return ;
	}
	
	HANDLE hChildProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pi.dwProcessId);	
	DWORD dwWait = WaitForSingleObject(hChildProcess, INFINITE);
	if (dwWait == WAIT_OBJECT_0)
	{
		OutputDebugString("完成签名");
	}
}

void CAppProtectorDlg::OnProtect() 
{
	// TODO: Add your control notification handler code here
	
	char szCurrentDir[MAX_PATH]={0};
	char szApktoolDir[MAX_PATH]={0};	

	char szInfo[256]={0};
	char szCommandLine[256] = { 0 };

	//在VC6.0 下 [2548] 当前环境目录:C:\Documents and Settings\storm\桌面\样本
	GetCurrentDirectory(MAX_PATH,szCurrentDir);
	sprintf(szInfo,"当前环境目录:%s\n",szCurrentDir);
	OutputDebugString(szInfo);
	
	DecomplileApk();

	ExtractOneFileFromApk("classes.dex");

	DeleteSmali();
		
	CopyPakcerSmali();

	CopySo();

	OverWriteApplication();

	RecompileApk();

	SignApk();

	AfxMessageBox("加固成功");
	


}
