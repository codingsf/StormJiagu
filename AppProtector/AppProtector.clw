; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CAppProtectorDlg
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "AppProtector.h"

ClassCount=3
Class1=CAppProtectorApp
Class2=CAppProtectorDlg
Class3=CAboutDlg

ResourceCount=3
Resource1=IDD_ABOUTBOX
Resource2=IDR_MAINFRAME
Resource3=IDD_APPPROTECTOR_DIALOG

[CLS:CAppProtectorApp]
Type=0
HeaderFile=AppProtector.h
ImplementationFile=AppProtector.cpp
Filter=N

[CLS:CAppProtectorDlg]
Type=0
HeaderFile=AppProtectorDlg.h
ImplementationFile=AppProtectorDlg.cpp
Filter=D
BaseClass=CDialog
VirtualFilter=dWC
LastObject=IDC_Protect

[CLS:CAboutDlg]
Type=0
HeaderFile=AppProtectorDlg.h
ImplementationFile=AppProtectorDlg.cpp
Filter=D

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889

[DLG:IDD_APPPROTECTOR_DIALOG]
Type=1
Class=CAppProtectorDlg
ControlCount=5
Control1=IDOK,button,1342242817
Control2=IDC_Protect,button,1342242816
Control3=IDC_EDIT1,edit,1350633600
Control4=IDC_STATIC,button,1342177287
Control5=IDC_STATIC,static,1342308352

