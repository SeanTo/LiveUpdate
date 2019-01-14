// LiveUpdate.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "LiveUpdate.h"
#include "LiveUpdateDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLiveUpdateApp

BEGIN_MESSAGE_MAP(CLiveUpdateApp, CWinApp)
	//{{AFX_MSG_MAP(CLiveUpdateApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLiveUpdateApp construction

CLiveUpdateApp::CLiveUpdateApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CLiveUpdateApp object

CLiveUpdateApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CLiveUpdateApp initialization

BOOL CLiveUpdateApp::InitInstance()
{
	AfxEnableControlContainer();
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	// 多实例检查
	CString	strClassName = _T("MyLiveUpdate");
	m_hMutex = OpenMutex (MUTEX_ALL_ACCESS, FALSE, strClassName);
	if (NULL == m_hMutex)
	{
		m_hMutex = CreateMutex(NULL, TRUE, strClassName) ;
	}
	else if(ERROR_ALREADY_EXISTS == ::GetLastError())
	{
		// 上次异常退出，终止异常实例
		AfxMessageBox(_T("上次程序异常退出！"));
	}
	else
		return FALSE;

	CLiveUpdateDlg dlg;
	m_pMainWnd = &dlg;
	int nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
	}
	else if (nResponse == IDCANCEL)
	{
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

int CLiveUpdateApp::ExitInstance() 
{
	// TODO: Add your specialized code here and/or call the base class
	if(NULL != m_hMutex)
	{
		ReleaseMutex(m_hMutex);
	}
	
	return CWinApp::ExitInstance();
}
