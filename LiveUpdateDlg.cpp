// LiveUpdateDlg.cpp : implementation file
//

#include "stdafx.h"
#include "LiveUpdate.h"
#include "LiveUpdateDlg.h"

#include "..\Share\wnd.h"
#include "..\Share\EMFC.h"
#include "..\Share\IniFile.h"
#include "..\Share\DownFile.h"

//////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define for if(1) for
#define COUNT_STEPS 3

#define WAIT_HERE \
{	\
	MSG  msg;\
	if(::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))	\
	{	\
	::TranslateMessage(&msg);\
	::DispatchMessage(&msg);	\
	}	\
}	\

//////////////////////////////////////////////////////////////////////////

CDownInfo DownInfo;	// �߳�ͬ������
INICFG    stLocal;	// ���������ļ��ṹ
INICFG    stRemote;	// Զ�������ļ��ṹ

#define _MY3D_
#define INST_VAR 6

#define NAME(i)	(i) * INST_VAR
#define VER(i)	(i) * INST_VAR + 1
#define URL(i)	(i) * INST_VAR + 2
#define BAK(i)	(i) * INST_VAR + 3
#define DEL(i)	(i) * INST_VAR + 4
#define SMR(i)	(i) * INST_VAR + 5

#define LOCAL_NAME(i)	stLocal.arrInst[NAME(i)]
#define LOCAL_VER(i)	stLocal.arrInst[VER(i)]
#define LOCAL_URL(i)	stLocal.arrInst[URL(i)]
#define LOCAL_BAK(i)	stLocal.arrInst[BAK(i)]
#define LOCAL_DEL(i)	stLocal.arrInst[DEL(i)]

#define REMOTE_NAME(i)	stRemote.arrInst[NAME(i)]
#define REMOTE_VER(i)	stRemote.arrInst[VER(i)]
#define REMOTE_URL(i)	stRemote.arrInst[URL(i)]
#define REMOTE_BAK(i)	stRemote.arrInst[BAK(i)]
#define REMOTE_DEL(i)	stRemote.arrInst[DEL(i)]
#define REMOTE_SMR(i)	stRemote.arrInst[SMR(i)]

/////////////////////////////////////////////////////////////////////////////
// CLiveUpdateDlg dialog

const CString CLiveUpdateDlg::STEPS[] = { _T("���(&C)"), _T("����(&U)"), _T("����(&F)") };
// const CString CLiveUpdateDlg::ZIP_FMT[] = {
// 	_T(".7z"), _T("zip"), _T("cab")
// };

CLiveUpdateDlg::CLiveUpdateDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLiveUpdateDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLiveUpdateDlg)
	m_strStatus = _T("");
	m_bCheckDelBak = FALSE;
	m_bCheckDelTemp = TRUE;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);	

	//////////////////////////////////////////////////////////////////////////
	pThread = NULL;
	m_iStep = 0;
	
	// Ŀ¼��ʼ��
	m_sAbsDir = WND::GetAbsPath(TRUE);
	m_sTmpDir = m_sAbsDir + _T("LiveUpdate\\");
	m_sBakDir.Empty();
	
	// �����ļ���������Ŀ¼��ʼ��֮��
	m_sLocalIniFile  = m_sAbsDir + _T("LiveUpdate.ini");
	m_sRemoteIniFile = m_sTmpDir + _T("LiveUpdate.ini");

	// ����ɾ����������
	m_strBat = m_sAbsDir + _T("SelfUpdate.bat");
	if( WND::IsExistFile(m_strBat) )
		DeleteFile(m_strBat);

	m_iDiscovers = 0;
}

void CLiveUpdateDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLiveUpdateDlg)
	DDX_Control(pDX, IDC_EDIT_SUMMARY, m_editSummary);
	DDX_Control(pDX, IDC_STATIC_WELCOME, m_stcWelcome);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDC_LIST1, m_List);
	DDX_Control(pDX, IDC_PROGRESS1, m_prog);
	DDX_Text(pDX, IDC_STATIC_STATUS, m_strStatus);
	DDX_Check(pDX, IDC_CHECK_DEL_BAK, m_bCheckDelBak);
	DDX_Check(pDX, IDC_CHECK_DEL_TEMP, m_bCheckDelTemp);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CLiveUpdateDlg, CDialog)
	//{{AFX_MSG_MAP(CLiveUpdateDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_HOMEPAGE, OnHomepage)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST1, OnItemchangedList1)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST1, OnDblclkList1)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_DOWN_END, OnDownEnd)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLiveUpdateDlg message handlers

BOOL CLiveUpdateDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	//////////////////////////////////////////////////////////////////////////
	::SetWindowPos(GetSafeHwnd(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE); 

	// �̱߳���
	DownInfo.hWnd = GetSafeHwnd();
	DownInfo.SetProgMode(&m_prog, GetDlgItem(IDC_STATIC_STATUS));
	
	// �б��
	m_List.SetExtendedStyle( m_List.GetExtendedStyle()
		| LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_CHECKBOXES );
	// ����
	m_List.InsertColumn(0,"����",LVCFMT_LEFT,100);
	m_List.InsertColumn(1,"���ذ汾",LVCFMT_LEFT,80);
	m_List.InsertColumn(2,"���°汾",LVCFMT_LEFT,90);
	m_List.InsertColumn(3,"״̬",LVCFMT_LEFT,130);

	m_List.SetReadOnlyColumn(-1);
	m_List.SetReadOnlyColumn(3, FALSE);
	
	// ������
	m_prog.SetRange(0, 100);
	m_prog.SetPos(0);
	PlaceProgress(-1);

	// �������
	if( !GetProductList(m_sLocalIniFile, &stLocal, FALSE) )
	{
#ifdef _MY3D_
		m_strStatus =
			CString(_T("���������ļ������ڣ���Ĭ�ϴ�����ĵ�ַ���ظ������ã�\n"))
			+ stLocal.sIniUrl;
#endif // _MY3D_
	}
	else
		Step(0);

	UpdateData(FALSE);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CLiveUpdateDlg::OnPaint() 
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
HCURSOR CLiveUpdateDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

/*
 *	������ҳ
 */
void CLiveUpdateDlg::OnHomepage() 
{
	// TODO: Add your control notification handler code here	
	if( ParseUrl(stRemote.sHome) )
		WND::WinExec(stRemote.sHome);
	else if(ParseUrl(stLocal.sHome))
		WND::WinExec(stLocal.sHome);
	else
		WND::WinExec( _T("https://seanto.github.io") );
}

void CLiveUpdateDlg::OnOK()
{
	m_btnOk.EnableWindow(FALSE);

	switch(Step())
	{
	case 0:
		RunStep1();
		break;
	case 1:
		RunStep2();
		break;
	case 2:
		RunStep3();
	    break;
	default:
	    break;
	}
}

/*
 *	�趨����
 */
void CLiveUpdateDlg::Step(int iStep)
{
	ASSERT( iStep >= 0 && iStep < COUNT_STEPS);
	
	m_btnOk.SetWindowText(STEPS[iStep]);

	m_iStep = iStep;
	switch(iStep)
	{
	case 0:
		m_strStatus = _T("\n��� [���] �鿴���õĸ���...");
		break;
	case 1:
		m_strStatus = _T("\n��� [����] ��ʼ����...");
		break;
	case 2:
		m_strStatus = _T("\n��ɣ�");
		if( stRemote.sRun.IsEmpty() || !WND::IsExistFile(m_sAbsDir+stRemote.sRun) )
			m_btnOk.SetWindowText(_T("���"));
		m_btnOk.EnableWindow(TRUE);
		break;
	}
}

/*
 *	�������ص�ַ
 *  ������ http:// �� ftp:// ��ͷ
 *  ���԰汾�п��� file:// ��ͷ����ӱ���Դ����
 */
BOOL CLiveUpdateDlg::ParseUrl(const CString &strUrl)
{
	if( -1 != strUrl.Find(_T('\\')))
		return FALSE;

	if( 0 != strUrl.Left(7).CompareNoCase(_T("http://")) )
		return FALSE;

	return TRUE;
}

/*
 *	����Զ�������ļ�
 */
int CLiveUpdateDlg::ParseNewVersion()
{	
	if(stLocal.iInst != stRemote.iInst)
		return -1;

	CString strSummary(_T("ժҪ:\r\n\r\n"));
	int iCount = 0;
	for(int i = 0; i < stRemote.iInst; ++i)
	{
		if( !ParseUrl( REMOTE_URL(i) ) )
		{
			// ���ص�ַ������
			m_List.SetItemText( i, 3, _T("�����ַ:") + REMOTE_URL(i) );
		}
		else if( LOCAL_VER(i).IsEmpty() || REMOTE_VER(i) != LOCAL_VER(i) )
		{
			m_List.SetItemText(i, 3, _T("���ָ��£�"));
			m_List.SetCheck(i, TRUE);
			++iCount;

			strSummary += _T("[") + REMOTE_NAME(i) + _T("]\r\n")
				+ REMOTE_SMR(i) + _T("\r\n\r\n");
		}
	}

	if( iCount > 0 )
	{
		strSummary.Replace(_T(" "), _T("\r\n"));
		CRect rc;
		GetDlgItem(IDC_STATIC_LEFT)->GetClientRect(&rc);
		GetDlgItem(IDC_STATIC_LEFT)->ShowWindow(SW_HIDE);
		
		rc.left += 5;
		rc.top += 5;
		m_editSummary.MoveWindow(&rc);
		m_editSummary.SetWindowText(strSummary);
		m_editSummary.ShowWindow(SW_SHOW);
	}

	return iCount;
}

/*
 *	������
 */
void CLiveUpdateDlg::RunStep1()
{
	// ��ʱĿ¼
	if( !WND::IsExistFile(m_sTmpDir) )
		CreateDirectory(m_sTmpDir, NULL);
	if( !WND::IsExistFile(m_sTmpDir) )
	{
		AfxMessageBox(_T("������ʱĿ¼ʧ�ܣ�"));
		return;
	}

	// ������������ļ� URL �Ƿ�Ϸ�
	if( !ParseUrl(stLocal.sIniUrl) )
	{
		Step(COUNT_STEPS - 1);
		m_strStatus = stLocal.sIniUrl + _T("\n�����ַ������...");
		UpdateData(FALSE);
		return;
	}
	
	m_strStatus = _T("����°汾����������...\n") + stLocal.sIniUrl;
	UpdateData(FALSE);
	
	DownInfo.SetFile(0, stLocal.sIniUrl, m_sRemoteIniFile);
	GetDlgItem(IDCANCEL)->SetWindowText(_T("ȡ��"));
	pThread = AfxBeginThread(CDownInfo::GetFileFromHttp, &DownInfo);
	while ( pThread && DownInfo.bBusy )
		WAIT_HERE;

	// �����������ļ�
 	if( CDownInfo::ERR_OK == DownInfo.nRet )
	{
		// ��ȡԶ������
		if( !GetProductList(m_sRemoteIniFile, &stRemote, TRUE) )
		{
			Step(COUNT_STEPS-1);
			m_strStatus = _T("Զ�����ô���!�������س����������ϣ����Ժ�����..."
				"\n�ɼ����ʱĿ¼ LiveUpdate �µ� LiveUpdate.ini �ļ�!");
			UpdateData(FALSE);
			return;
		}

		// ������ð汾
		if( !WND::IsExistFile(m_sLocalIniFile) || stLocal.sIniVer.IsEmpty()
			|| stLocal.sIniVer != stRemote.sIniVer
			|| stLocal.iInst != stRemote.iInst )
		{
			CString str = _T("���������ļ���ƥ�䣡\n���ܱ��������ļ�����򲻴��ڣ���Զ�����ð汾���\n")
				_T("\n�Ƿ�λ�������ã�");
			if(IDNO == AfxMessageBox(str, MB_YESNO))
				return;

			// ��ʼ����������
			CopyFile(m_sLocalIniFile, m_sLocalIniFile + _T(".bak"), FALSE);
			CopyFile(m_sRemoteIniFile, m_sLocalIniFile, FALSE);
			if( !ResetIniFile(m_sLocalIniFile) )
			{
				m_strStatus = _T("�������ļ�ʧ�ܣ�");
				UpdateData(FALSE);
				return;
			}

			GetProductList(m_sLocalIniFile, &stLocal, FALSE);
			GetProductList(m_sRemoteIniFile, &stRemote, TRUE);
		}
		
		// �����������ļ�
		m_iDiscovers = ParseNewVersion();
		if( 0 > m_iDiscovers )
		{
			Step(COUNT_STEPS - 1);
			m_strStatus = _T("�����ļ����ݲ�ƥ��!\n�������ҳ�ֶ�����...");			
			UpdateData(FALSE);
			return;
		}
		else if( 0 == m_iDiscovers )
		{
			Step(1);
			m_strStatus = _T("�������°汾�����Կɹ�ѡ���ǿ�Ƹ���...\n")
				+ m_strStatus;

			m_btnOk.EnableWindow( m_List.GetCheckCount() > 0 );
		}
		else
		{
			Step(1);
			m_strStatus = _T("���ֿ��ø��£�\n") + m_strStatus;
			m_btnOk.EnableWindow(TRUE);
		}
	}
	else // ����ʧ��
	{
		Step(COUNT_STEPS - 1);
		m_strStatus.Format(_T("\n����Զ�������ļ�ʧ��...(ERR%d): "), DownInfo.nRet);
		m_strStatus += DownInfo.GetLastError();
		UpdateData(FALSE);
		return;
	}
	UpdateData(FALSE);
}

/*
 *	���ظ���
 */
void CLiveUpdateDlg::RunStep2()
{
	// ��������
	if( !CloseRunning() )
		return;

	// ��������Ŀ¼
	if( ! CreateBackupDirectory() )
	{
		if( IDNO == AfxMessageBox(_T("��������Ŀ¼ʧ�ܣ�����������\n")
			_T("�������ֶ�����Ŀ¼..."), MB_YESNO) )
		{
			return;
		}
	}
		
	BOOL b7z = FALSE;
	BOOL bSucc = TRUE;
	int  iSucc = 0;
	CString strUrl, strDownFile, strPara, str;
	CStringArray arrBakDel;

	CIniFile fini;
	fini.Create(m_sLocalIniFile);
	
	// �����ļ�
	for(int i = 0; i < m_List.GetItemCount(); ++i )
	{
		if( !m_List.GetCheck(i) )
			continue;
		
		// ��ַ�������ļ���
		strUrl = REMOTE_URL(i);
		strDownFile = strUrl.Mid( (strUrl.ReverseFind(_T('/')) + 1) );
		if( 0 == strDownFile.Right(4).CompareNoCase(_T(".txt")) )
			strDownFile = strDownFile.Left( strDownFile.GetLength()-4 );

		PlaceProgress(i, 3);	// ��ʾ������
		m_strStatus = _T("���� : ") + strUrl;
		UpdateData(FALSE);

		// ���������߳�
		DownInfo.SetFile(i+1, strUrl, m_sTmpDir + strDownFile);
		pThread = AfxBeginThread(CDownInfo::GetFileFromHttp, &DownInfo);
		// �ȴ������߳̽���
		while ( pThread && DownInfo.bBusy )
			WAIT_HERE;

		// ���ؽ��
		m_List.SetItemText(i, 3, DownInfo.GetLastError());
		if( CDownInfo::ERR_OK != DownInfo.nRet )
		{
			if( CDownInfo::ERR_ABORT != DownInfo.nRet )
			{
				Step(COUNT_STEPS - 1);
				m_strStatus = _T("���س���\n") + strUrl;
				UpdateData(FALSE);
				bSucc = FALSE;
			}
			else	// �û���ֹ����
				return;
		}

		////////////////////////////////////////////////////////////////////////
		if( bSucc )
		{
			// ���� BAK �����������ļ�
			if( !m_sBakDir.IsEmpty() && WND::IsExistFile(m_sBakDir) )
			{
				if( ! REMOTE_BAK(i).IsEmpty() )
				{
					ParseFileName( arrBakDel, REMOTE_BAK(i) );
					BakOrDelFiles( arrBakDel, TRUE );
				}
			}

			// ��������ļ���չ��
			str = strDownFile.Mid( strDownFile.ReverseFind(_T('.'))+1);
			str.MakeLower();

			// ��ѹ 7z, zip
			if( _T("7z") == str || _T("zip") == str )
			{
				if( !b7z && !WND::IsExistFile(m_sAbsDir+_T("7za.exe")) )
					b7z = ReleaseFile(IDR_7ZA_EXE, m_sAbsDir+_T("7za.exe"));
				else
					b7z = TRUE;

				if( b7z )
				{
					// ���� 7-Zip �����У�����ʱĿ¼��ѹ����ǰĿ¼
					// ������: 7za.exe x -y -o"workDir" Archive -p"password"
					strPara = m_sAbsDir + _T("7za.exe x -y -o\"")
						+ m_sAbsDir + _T("\" \"")
						+ m_sTmpDir + strDownFile + _T("\"");
					
					#ifdef _7Z_PASSWORD_
					if( !stRemote.sPassword.IsEmpty() )
						strPara += _T(" -p\"") + stRemote.sPassword + _T("\"");
					#endif // _7Z_PASSWORD_

					DWORD dwRet = WND::WaitProcess(strPara, FALSE, stRemote.dwTime);
					if( 0 != dwRet )
					{
						m_List.SetItemText( i, 3, _T("��ѹʧ�ܣ�") );
						bSucc = FALSE;
					}
				}
				else
				{
					m_List.SetItemText(i, 3, _T("��ѹʧ�ܣ�"));
					bSucc = FALSE;
				}
			}
			// ����
			else if( _T("exe") == str || _T("bat") == str )
			{
				ShowWindow(SW_MINIMIZE);
				WND::WaitProcess(m_sTmpDir + strDownFile, TRUE, INFINITE);
				ShowWindow(SW_SHOWNORMAL);
			}
			// ���ǿ�ִ���ļ�
			else if( _T("ex_") == str )
			{
				CString strRename = strDownFile;
				strRename.SetAt( strRename.GetLength()-1, _T('e') );

				if( !CopyFile( m_sTmpDir + strDownFile, m_sAbsDir + strRename, FALSE ) )
				{
					m_List.SetItemText(i, 3, _T("����exe�ļ�ʧ�ܣ�"));
					bSucc = FALSE;
				}
			}
			// ���������ļ�
			else
			{
				if( !CopyFile(m_sTmpDir + strDownFile, m_sAbsDir + strDownFile, FALSE) )
				{
					m_List.SetItemText(i, 3, _T("�����ļ�ʧ�ܣ�"));
					bSucc = FALSE;
				}
			}

			// ���� DEL ������ɾ���ļ�
			if( ! REMOTE_DEL(i).IsEmpty() )
			{
				ParseFileName( arrBakDel, REMOTE_DEL(i) );
				BakOrDelFiles( arrBakDel, FALSE);
			}
		}
		PlaceProgress(-1);		// ���ؽ�����

		if( bSucc )
		{
			str.Format(_T("VER%02d"), i);
			fini.SetVarStr(_T("INSTALL"), str, REMOTE_VER(i), 0);
			++iSucc;

			if( stRemote.iDepend == i && stRemote.sIniUrl != stLocal.sIniUrl )
				fini.SetVarStr(_T("WELCOME"), _T("URL"), stRemote.sIniUrl, 0);
		}
		else
		{
			m_bCheckDelTemp = FALSE;
			m_bCheckDelBak = FALSE;
			UpdateData(FALSE);
		}
	}

	if( iSucc > 0  && iSucc >= m_iDiscovers )
	{
		// �ı��������ַ
		if( stRemote.iDepend < 0 )
		{
			fini.SetVarStr(_T("WELCOME"), _T("URL"), stRemote.sIniUrl, 0);
		}
		// �鿴��־
		if( stLocal.sLog.IsEmpty() || stRemote.sLogVer != stLocal.sLogVer )
		{
			fini.SetVarStr(_T("PROCESS"), _T("WHATNEWS"), stRemote.sLogVer, 0);
			WND::WinExec( m_sAbsDir + stRemote.sLog );
		}
	}

	Step(COUNT_STEPS - 1);
}

void CLiveUpdateDlg::RunStep3()
{
	if( !stRemote.sRun.IsEmpty() && WND::IsExistFile(m_sAbsDir+stRemote.sRun) )
		WND::WinExec( m_sAbsDir + stRemote.sRun );
	
	OnCancel();
}

/*
 *	���������ļ�
 *  ���������ļ��еİ汾��ȫ�� 0
 *  ���ô˷���������Զ�������ļ��滻���������ļ�
 */
BOOL CLiveUpdateDlg::ResetIniFile(LPCTSTR szIniFile, LPCSTR szVer/*=_T("0")*/)
{
	// ��λ [INSTALL] ������������
	CIniFile fini;
	if( fini.Create(szIniFile) )
	{
		int iCount = 0;
		fini.GetVarInt(_T("INSTALL"), _T("FILES"), iCount);
		if(iCount <= 0)
			return FALSE;

		if(iCount >= 100)
			iCount = 100;

		CString strVar;	
		for(int i = 0; i < iCount; ++i)
		{
			strVar.Format(_T("VER%02d"), i);
			fini.SetVarStr(_T("INSTALL"), strVar, szVer, 0);

			strVar.Format(_T("URL%02d"), i);
			fini.SetVarStr(_T("INSTALL"), strVar, _T(""), 1);
			
			strVar.Format(_T("BAK%02d"), i);
			fini.SetVarStr(_T("INSTALL"), strVar, _T(""), 1);
			
			strVar.Format(_T("DEL%02d"), i);
			fini.SetVarStr(_T("INSTALL"), strVar, _T(""), 1);
			
			strVar.Format(_T("SMRY%02d"), i);
			fini.SetVarStr(_T("INSTALL"), strVar, _T(""), 1);
		}

		fini.SetVarStr(_T("PROCESS"), _T("WHATNEWS"), szVer, 0);

		return TRUE;
	}

	return FALSE;
}

BOOL CLiveUpdateDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if(WM_KEYDOWN == pMsg->message)
	{
		if(VK_RETURN == pMsg->wParam || VK_ESCAPE == pMsg->wParam)
		{   
			return TRUE;
		}
	}	
	
	return CDialog::PreTranslateMessage(pMsg);
}

void CLiveUpdateDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	if( DownInfo.bBusy )
	{
		if( IDYES == MessageBox(_T("��ȷ���ظ��£�ȷ��Ҫֹͣ������?"),
			NULL, MB_YESNO | MB_ICONWARNING) )
		{
			DownInfo.bCancel = TRUE;
		}
	}
	else
	{
		UpdateData(TRUE);

		if(m_bCheckDelTemp)
		{
			// ɾ����ʱĿ¼
			m_sTmpDir.TrimRight(_T("\\"));
			if( WND::IsExistFile(m_sTmpDir) )
				WND::DeleteFileEx(m_sTmpDir);
		}

		if(m_bCheckDelBak)
		{
			if( m_sBakDir.IsEmpty() )
				m_sBakDir = m_sAbsDir + _T("Backup");

			// ɾ������Ŀ¼
			m_sBakDir.TrimRight(_T('\\'));
			if( WND::IsExistFile(m_sBakDir) )
				WND::DeleteFileEx(m_sBakDir);
		}

// 		if( WND::IsExistFile(m_sAbsDir + _T("7za.exe")) )
// 			WND::DeleteFileEx( m_sAbsDir + _T("7za.exe") );

		// ���Ҹ��£����� LiveUpdate.upd �ļ�ʱ�ͳ� bat �ļ�
		if( WND::IsExistFile( m_sAbsDir + _T("LiveUpdate.upd")) )
		{
			if( ReleaseFile(IDR_SELF_BAT, m_strBat) )
			{
				if( WND::IsExistFile(m_strBat) )
				{
					m_sAbsDir.TrimRight(_T('\\'));
					WND::WinExec(m_strBat, NULL, m_sAbsDir, SW_HIDE);
				}
			}
		}

		CDialog::OnCancel();
	}
}

BOOL CLiveUpdateDlg::CloseRunning()
{
	CString str =
		CString(_T(" �������У�\n�ȹرճ�����ܽ��и��£�\n\n"))
		+ _T("ѡ�� [ȷ��] ��ǿ�ƹرճ���...\n")
		+ _T("ѡ�� [ȡ��] ֹͣ���£��ֶ��رպ��ٸ���...");

	CStringArray arr;
	ParseFileName( arr, stRemote.sKill );
	if( !stRemote.sRun.IsEmpty() )
		arr.Add(stRemote.sRun);

	for(int i = 0, id = -1; i < arr.GetSize() ; ++i)
	{
		id = WND::FindExePID( arr[i] );
		if( -1 != id )
		{
			if( IDOK == AfxMessageBox(arr[i] + str, MB_OKCANCEL|MB_ICONWARNING) )
			{
				HANDLE ProcessHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, id);
				if(ProcessHandle)
					TerminateProcess(ProcessHandle,0);
			}
			else
				return FALSE;
		}
	}
	return TRUE;
}

LRESULT CLiveUpdateDlg::OnDownEnd(WPARAM wParam, LPARAM lParam)
{
	GetDlgItem(IDCANCEL)->SetWindowText(_T("�ر�"));
	return 0;
}

void CLiveUpdateDlg::PlaceProgress(int row, int col/* = -1*/)
{
	CRect rcProg, rcList, rc;
	m_prog.GetWindowRect(&rcProg);
	ScreenToClient(&rcProg);
	m_List.GetWindowRect(&rcList);
	ScreenToClient(&rcList);

	// ��λ
	if( 0 == row && -1 == col )
	{
		m_prog.MoveWindow(&rcProg);
		m_prog.ShowWindow(SW_SHOW);
	}
	// ����
	else if( -1 == row && -1 == col )
	{
		m_prog.SetPos(0);
		m_prog.ShowWindow(SW_HIDE);
	}
	// �Ƶ��б�����
	else if( row >= 0 && col >= 0)
	{
		ASSERT(row < m_List.GetItemCount() && col < 4);
		m_List.GetSubItemRect(row, col, LVIR_LABEL, rc);
		
		rc.OffsetRect(rcList.left+4, rcList.top+3);
		rc.InflateRect(0, 0, -4, -3);
		m_prog.MoveWindow(&rc, TRUE);
		m_prog.SetPos(0);
		m_prog.ShowWindow(SW_SHOW);
		GetDlgItem(IDCANCEL)->SetWindowText(_T("ȡ��"));
	}
}

BOOL CLiveUpdateDlg::GetProductList(LPCTSTR szIniFile, PINICFG pIniCfg, BOOL bIsRemote)
{
	// ���ļ�
	BOOL bRet = TRUE;
	if( WND::IsExistFile(szIniFile) )
		bRet = ReadVersion(szIniFile, pIniCfg);
	else
	{
		ResetIniCfg(pIniCfg);
		bRet = FALSE;
	}

	if( bIsRemote )
	{
		if( bRet && pIniCfg->iInst > 0 && !pIniCfg->sIniVer.IsEmpty() )
		{
			const int iList = m_List.GetItemCount();
			for(int i = 0; i < pIniCfg->iInst; i++)
			{
				if(i >= iList)
					m_List.InsertItem(i, pIniCfg->arrInst[NAME(i)]);
				m_List.SetItemText(i, 2, pIniCfg->arrInst[VER(i)]);
			}
		}
		else
			bRet = FALSE;
	}
	else
	{
		// ȱʡֵ
#ifdef _MY3D_
		if( pIniCfg->sIniUrl.IsEmpty() )
			pIniCfg->sIniUrl = _T("https://github.com/SeanTo/LiveUpdate");
		if( pIniCfg->sHome.IsEmpty() )
			pIniCfg->sHome = _T("https://github.com/SeanTo/LiveUpdate");
#endif // _MY3D_
		if( pIniCfg->sTip.IsEmpty() )
			pIniCfg->sTip = _T("��ӭʹ�� LiveUpdate ͨ����������");
		
		// ��ʾ���
		m_stcWelcome.SetWindowText(pIniCfg->sTip);
		
		// ����б�
		m_List.DeleteAllItems();
		for(int i = 0; i < pIniCfg->iInst ; i++)
		{
			m_List.InsertItem(i, pIniCfg->arrInst[NAME(i)]);
			m_List.SetItemText(i, 1, pIniCfg->arrInst[VER(i)]);
		}
	}

	return bRet;
}

/*
 *  ��ȡ�����ļ��� IniCfg �ṹ	
 */
BOOL CLiveUpdateDlg::ReadVersion(const CString &strIniFile, PINICFG pCfg)
{
	ResetIniCfg(pCfg);

	// �������ļ�
	CIniFile fini;
	if( !fini.Create(strIniFile) )
		return FALSE;
	
	// ��������������ֵ
	CString strSect, strVar, str;
	
	// [WELCOME] : �����ļ���ַ����ҳ��ַ����ʾ��汾�����������
	strSect = _T("WELCOME");
	fini.GetVarStr(strSect, _T("URL"),	pCfg->sIniUrl);
	fini.GetVarStr(strSect, _T("HOME"), pCfg->sHome);
	fini.GetVarStr(strSect, _T("TIP"),	pCfg->sTip);
	fini.GetVarStr(strSect, _T("VER"),	pCfg->sIniVer);
	fini.GetVarInt(strSect, _T("DEPEND"), pCfg->iDepend);
	
	// [PROCESS] : ��ֹ���̡�������־����־�汾
	strSect = _T("PROCESS");
	fini.GetVarStr(strSect, _T("RUN"), pCfg->sRun);
	fini.GetVarStr(strSect, _T("KILL"), pCfg->sKill);
	fini.GetVarStr(strSect, _T("READ"), pCfg->sLog);
	fini.GetVarStr(strSect, _T("WHATNEWS"), pCfg->sLogVer);
	
	// [INSTALL] : ����б��������롢��ʱ
	strSect = _T("INSTALL");
	fini.GetVarInt(strSect, _T("FILES"), pCfg->iInst);
	if( pCfg->iInst > 100 )
		pCfg->iInst = 100;
	
	if(pCfg->iInst > 0)
	{
#ifdef _7Z_PASSWORD_
		fini.GetVarStr(strSect, _T("PASSWORD"), pCfg->sPassword);
#endif // _7Z_PASSWORD_

		for(int i = 0; i < pCfg->iInst ; i++)
		{
			// ����
			str.Empty();
			strVar.Format(_T("NAME%02d"), i);
			fini.GetVarStr(strSect, strVar, str);
			pCfg->arrInst.Add(str);
			
			// �汾
			str.Empty();
			strVar.Format(_T("VER%02d"), i);
			fini.GetVarStr(strSect, strVar, str);
			pCfg->arrInst.Add(str);
			
			// ���ص�ַ
			str.Empty();
			strVar.Format(_T("URL%02d"), i);
			fini.GetVarStr(strSect, strVar, str);
			pCfg->arrInst.Add(str);
			
			// �����ļ�
			str.Empty();
			strVar.Format(_T("BAK%02d"), i);
			fini.GetVarStr(strSect, strVar, str);
			pCfg->arrInst.Add(str);
			
			// ɾ���ļ�
			str.Empty();
			strVar.Format(_T("DEL%02d"), i);
			fini.GetVarStr(strSect, strVar, str);
			pCfg->arrInst.Add(str);

			// ժҪ
			str.Empty();
			strVar.Format(_T("SMRY%02d"), i);
			fini.GetVarStr(strSect, strVar, str);
			pCfg->arrInst.Add(str);
		}

		int iTime = 0;
		fini.GetVarInt(strSect, _T("FILES"), iTime);
		if( iTime > 0 )
			pCfg->dwTime = (DWORD)iTime * 1000;
	}
	return TRUE;
}

/*
 *	INICFG �ⲿ���캯��
 */
void CLiveUpdateDlg::ResetIniCfg(PINICFG pIniCfg)
{
#ifdef _7Z_PASSWORD_
	pIniCfg->sPassword.Empty();
#endif // _7Z_PASSWORD_
	// ��λ
	pIniCfg->sIniUrl.Empty();
	pIniCfg->sHome.Empty();
	pIniCfg->sTip.Empty();
	pIniCfg->sIniVer.Empty();
	pIniCfg->sLog.Empty();
	pIniCfg->sLogVer.Empty();
	pIniCfg->sRun.Empty();
	pIniCfg->sKill.Empty();
	pIniCfg->sBakDir.Empty();
	pIniCfg->arrInst.RemoveAll();
	pIniCfg->iInst = 0;
	pIniCfg->iDepend = -1;
	pIniCfg->dwTime = INFINITE;
}

/*
 *	��Ӧ�б�ǰ�ļ���״̬�ı�
 */
void CLiveUpdateDlg::OnItemchangedList1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here
	if( 1 == m_iStep )
		m_btnOk.EnableWindow(m_List.GetCheckCount() > 0);
	
	*pResult = 0;
}

/*
 *	�ֽ����ַ� ch �������ļ�����
 */
int CLiveUpdateDlg::ParseFileName(CStringArray &arr,
								   const CString &strDel,
								   const TCHAR &ch/*=_T(';')*/)
{
	CString sDel(strDel);
	EMFC::Trim(sDel, _T("/<>\n"));
	sDel.TrimLeft(ch);
	sDel.TrimRight(ch);

	if( sDel.IsEmpty() )
		return 0;

	CString str;
	int iSt = 0, iEnd = -1;
	
	arr.RemoveAll();
	while( TRUE )
	{
		iEnd = sDel.Find(ch, iSt);
		if( -1 == iEnd )
		{
			arr.Add( sDel.Mid(iSt) );
			break;
		}
		else if( iEnd > iSt )
			arr.Add( sDel.Mid(iSt , iEnd - iSt) );
		iSt = iEnd + 1;
	}

	return arr.GetSize();
}

/*
 *	����/ɾ�� arr �и����������ļ�
 */
void CLiveUpdateDlg::BakOrDelFiles(const CStringArray &arr, BOOL bIsBak/* =TRUE */)
{
	int iFiles = arr.GetSize();
	if( iFiles <= 0 )
		return;
	
	CString str;
	for(int i = 0; i < iFiles ; ++i)
	{
		str = m_sBakDir + arr[i] + (bIsBak ? _T(".bak") : _T(".del"));
		WND::CopyFileEx(m_sAbsDir + arr[i], str);
		
		if( !bIsBak )
			WND::DeleteFileEx(m_sAbsDir + arr[i]);
	}
}

/*
 *	��������Ŀ¼
 *  Ŀ¼ָ�����ȼ�: Զ������>��������>Ĭ��Backup
 */
BOOL CLiveUpdateDlg::CreateBackupDirectory()
{
	CString str;
	m_sBakDir.Empty();

	if( !stRemote.sBakDir.IsEmpty() )
	{
		// ����Զ�����ô�������Ŀ¼
		str = m_sAbsDir + stRemote.sBakDir;
		if( !WND::IsExistFile(str) )
			CreateDirectory(str, FALSE);
		if( WND::IsExistFile(str) )
			m_sBakDir = str;
	}
	
	if( m_sBakDir.IsEmpty() )
	{
		// ���ݱ������ô�������Ŀ¼
		if( !stLocal.sBakDir.IsEmpty() )
		{
			str = m_sAbsDir + stLocal.sBakDir;
			if( !WND::IsExistFile(str) )
				CreateDirectory(str, FALSE);
			if( WND::IsExistFile(str) )
				m_sBakDir = str;
		}
	}
	
	if( m_sBakDir.IsEmpty() )
	{
		// Ĭ�ϱ���Ŀ¼
		str = m_sAbsDir + _T("Backup");
		if( !WND::IsExistFile(str) )
			CreateDirectory(str, FALSE);
		if( WND::IsExistFile(str) )
			m_sBakDir = str;
	}

	if( m_sBakDir.IsEmpty() )
		return FALSE;

	m_sBakDir += _T("\\");
	return TRUE;
}

BOOL CLiveUpdateDlg::ReleaseFile(DWORD dwID, LPCTSTR szFile)
{
	//��λ�Զ�����Դ
	HRSRC hRsrc = FindResource(NULL, MAKEINTRESOURCE(dwID), _T("FILE"));
	if (NULL == hRsrc)
		return FALSE;
	
	//��Դ��С
	DWORD dwSize = SizeofResource(NULL, hRsrc);
	if (0 == dwSize)
		return FALSE;
	
	//������Դ
	HGLOBAL hGlobal = LoadResource(NULL, hRsrc);
	if (NULL == hGlobal)
		return FALSE;
	
	//������Դ
	LPVOID pBuffer = LockResource(hGlobal);
	if (NULL == pBuffer)
		return FALSE;
	
	//����Դд���ļ�
	FILE *fbat = _tfopen(szFile, _T("wb"));
	if( NULL == fbat )
		return FALSE;
	
	DWORD dwLen = fwrite(pBuffer, 1, dwSize, fbat);
	fclose(fbat);

	return TRUE;
}

void CLiveUpdateDlg::OnDblclkList1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	if( 1 != Step() )
		return;

	int iSel = m_List.GetCurSel();
	if( iSel >= 0 && iSel < m_List.GetItemCount() && m_List.GetCheck(iSel) )
	{
		if( !REMOTE_SMR(iSel).IsEmpty() )
			MsgBox(REMOTE_SMR(iSel));
	}
	*pResult = 0;
}
