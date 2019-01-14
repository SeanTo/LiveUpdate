// LiveUpdate.h : main header file for the LiveUpdate application
//

#if !defined(AFX_LiveUpdate_H__4369A48F_F1EA_43E3_85C0_B230EEEBDAFD__INCLUDED_)
#define AFX_LiveUpdate_H__4369A48F_F1EA_43E3_85C0_B230EEEBDAFD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CLiveUpdateApp:
// See LiveUpdate.cpp for the implementation of this class
//

class CLiveUpdateApp : public CWinApp
{
public:
	CLiveUpdateApp();
	
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLiveUpdateApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CLiveUpdateApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	HANDLE m_hMutex;
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LiveUpdate_H__4369A48F_F1EA_43E3_85C0_B230EEEBDAFD__INCLUDED_)