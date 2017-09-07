// AppProtector.h : main header file for the APPPROTECTOR application
//

#if !defined(AFX_APPPROTECTOR_H__8C827BF0_640C_4F0D_A85B_FB6F03C0EF40__INCLUDED_)
#define AFX_APPPROTECTOR_H__8C827BF0_640C_4F0D_A85B_FB6F03C0EF40__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CAppProtectorApp:
// See AppProtector.cpp for the implementation of this class
//

class CAppProtectorApp : public CWinApp
{
public:
	CAppProtectorApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAppProtectorApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CAppProtectorApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_APPPROTECTOR_H__8C827BF0_640C_4F0D_A85B_FB6F03C0EF40__INCLUDED_)
