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

CDownInfo DownInfo;	// 线程同步变量
INICFG    stLocal;	// 本地配置文件结构
INICFG    stRemote;	// 远程配置文件结构

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

const CString CLiveUpdateDlg::STEPS[] = { _T("检查(&C)"), _T("更新(&U)"), _T("返回(&F)") };
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
	
	// 目录初始化
	m_sAbsDir = WND::GetAbsPath(TRUE);
	m_sTmpDir = m_sAbsDir + _T("LiveUpdate\\");
	m_sBakDir.Empty();
	
	// 配置文件名，放在目录初始化之后
	m_sLocalIniFile  = m_sAbsDir + _T("LiveUpdate.ini");
	m_sRemoteIniFile = m_sTmpDir + _T("LiveUpdate.ini");

	// 自我删除批处理名
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

	// 线程变量
	DownInfo.hWnd = GetSafeHwnd();
	DownInfo.SetProgMode(&m_prog, GetDlgItem(IDC_STATIC_STATUS));
	
	// 列表框
	m_List.SetExtendedStyle( m_List.GetExtendedStyle()
		| LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_CHECKBOXES );
	// 标题
	m_List.InsertColumn(0,"名称",LVCFMT_LEFT,100);
	m_List.InsertColumn(1,"本地版本",LVCFMT_LEFT,80);
	m_List.InsertColumn(2,"最新版本",LVCFMT_LEFT,90);
	m_List.InsertColumn(3,"状态",LVCFMT_LEFT,130);

	m_List.SetReadOnlyColumn(-1);
	m_List.SetReadOnlyColumn(3, FALSE);
	
	// 进度条
	m_prog.SetRange(0, 100);
	m_prog.SetPos(0);
	PlaceProgress(-1);

	// 搜索组件
	if( !GetProductList(m_sLocalIniFile, &stLocal, FALSE) )
	{
#ifdef _MY3D_
		m_strStatus =
			CString(_T("本地配置文件不存在，将默认从下面的地址下载更新配置：\n"))
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
 *	访问主页
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
 *	设定步序
 */
void CLiveUpdateDlg::Step(int iStep)
{
	ASSERT( iStep >= 0 && iStep < COUNT_STEPS);
	
	m_btnOk.SetWindowText(STEPS[iStep]);

	m_iStep = iStep;
	switch(iStep)
	{
	case 0:
		m_strStatus = _T("\n点击 [检查] 查看可用的更新...");
		break;
	case 1:
		m_strStatus = _T("\n点击 [更新] 开始下载...");
		break;
	case 2:
		m_strStatus = _T("\n完成！");
		if( stRemote.sRun.IsEmpty() || !WND::IsExistFile(m_sAbsDir+stRemote.sRun) )
			m_btnOk.SetWindowText(_T("完成"));
		m_btnOk.EnableWindow(TRUE);
		break;
	}
}

/*
 *	分析下载地址
 *  必须以 http:// 或 ftp:// 开头
 *  调试版本中可以 file:// 开头，便从本地源调试
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
 *	分析远程配置文件
 */
int CLiveUpdateDlg::ParseNewVersion()
{	
	if(stLocal.iInst != stRemote.iInst)
		return -1;

	CString strSummary(_T("摘要:\r\n\r\n"));
	int iCount = 0;
	for(int i = 0; i < stRemote.iInst; ++i)
	{
		if( !ParseUrl( REMOTE_URL(i) ) )
		{
			// 下载地址不可用
			m_List.SetItemText( i, 3, _T("错误地址:") + REMOTE_URL(i) );
		}
		else if( LOCAL_VER(i).IsEmpty() || REMOTE_VER(i) != LOCAL_VER(i) )
		{
			m_List.SetItemText(i, 3, _T("发现更新！"));
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
 *	检查更新
 */
void CLiveUpdateDlg::RunStep1()
{
	// 临时目录
	if( !WND::IsExistFile(m_sTmpDir) )
		CreateDirectory(m_sTmpDir, NULL);
	if( !WND::IsExistFile(m_sTmpDir) )
	{
		AfxMessageBox(_T("创建临时目录失败！"));
		return;
	}

	// 检查下载配置文件 URL 是否合法
	if( !ParseUrl(stLocal.sIniUrl) )
	{
		Step(COUNT_STEPS - 1);
		m_strStatus = stLocal.sIniUrl + _T("\n服务地址不可用...");
		UpdateData(FALSE);
		return;
	}
	
	m_strStatus = _T("检查新版本，正在连接...\n") + stLocal.sIniUrl;
	UpdateData(FALSE);
	
	DownInfo.SetFile(0, stLocal.sIniUrl, m_sRemoteIniFile);
	GetDlgItem(IDCANCEL)->SetWindowText(_T("取消"));
	pThread = AfxBeginThread(CDownInfo::GetFileFromHttp, &DownInfo);
	while ( pThread && DownInfo.bBusy )
		WAIT_HERE;

	// 下载新配置文件
 	if( CDownInfo::ERR_OK == DownInfo.nRet )
	{
		// 读取远程配置
		if( !GetProductList(m_sRemoteIniFile, &stRemote, TRUE) )
		{
			Step(COUNT_STEPS-1);
			m_strStatus = _T("远程配置错误!可能下载出错或网络故障，请稍候再试..."
				"\n可检查临时目录 LiveUpdate 下的 LiveUpdate.ini 文件!");
			UpdateData(FALSE);
			return;
		}

		// 检查配置版本
		if( !WND::IsExistFile(m_sLocalIniFile) || stLocal.sIniVer.IsEmpty()
			|| stLocal.sIniVer != stRemote.sIniVer
			|| stLocal.iInst != stRemote.iInst )
		{
			CString str = _T("更新配置文件不匹配！\n可能本地配置文件错误或不存在，或远程配置版本变更\n")
				_T("\n是否复位本地配置？");
			if(IDNO == AfxMessageBox(str, MB_YESNO))
				return;

			// 初始化本地配置
			CopyFile(m_sLocalIniFile, m_sLocalIniFile + _T(".bak"), FALSE);
			CopyFile(m_sRemoteIniFile, m_sLocalIniFile, FALSE);
			if( !ResetIniFile(m_sLocalIniFile) )
			{
				m_strStatus = _T("打开配置文件失败！");
				UpdateData(FALSE);
				return;
			}

			GetProductList(m_sLocalIniFile, &stLocal, FALSE);
			GetProductList(m_sRemoteIniFile, &stRemote, TRUE);
		}
		
		// 分析新配置文件
		m_iDiscovers = ParseNewVersion();
		if( 0 > m_iDiscovers )
		{
			Step(COUNT_STEPS - 1);
			m_strStatus = _T("配置文件内容不匹配!\n请访问网页手动更新...");			
			UpdateData(FALSE);
			return;
		}
		else if( 0 == m_iDiscovers )
		{
			Step(1);
			m_strStatus = _T("已是最新版本！但仍可勾选组件强制更新...\n")
				+ m_strStatus;

			m_btnOk.EnableWindow( m_List.GetCheckCount() > 0 );
		}
		else
		{
			Step(1);
			m_strStatus = _T("发现可用更新！\n") + m_strStatus;
			m_btnOk.EnableWindow(TRUE);
		}
	}
	else // 下载失败
	{
		Step(COUNT_STEPS - 1);
		m_strStatus.Format(_T("\n下载远程配置文件失败...(ERR%d): "), DownInfo.nRet);
		m_strStatus += DownInfo.GetLastError();
		UpdateData(FALSE);
		return;
	}
	UpdateData(FALSE);
}

/*
 *	下载更新
 */
void CLiveUpdateDlg::RunStep2()
{
	// 结束进程
	if( !CloseRunning() )
		return;

	// 创建备份目录
	if( ! CreateBackupDirectory() )
	{
		if( IDNO == AfxMessageBox(_T("创建备份目录失败，继续更新吗？\n")
			_T("建议先手动备份目录..."), MB_YESNO) )
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
	
	// 更新文件
	for(int i = 0; i < m_List.GetItemCount(); ++i )
	{
		if( !m_List.GetCheck(i) )
			continue;
		
		// 网址、下载文件名
		strUrl = REMOTE_URL(i);
		strDownFile = strUrl.Mid( (strUrl.ReverseFind(_T('/')) + 1) );
		if( 0 == strDownFile.Right(4).CompareNoCase(_T(".txt")) )
			strDownFile = strDownFile.Left( strDownFile.GetLength()-4 );

		PlaceProgress(i, 3);	// 显示进度条
		m_strStatus = _T("连接 : ") + strUrl;
		UpdateData(FALSE);

		// 开启下载线程
		DownInfo.SetFile(i+1, strUrl, m_sTmpDir + strDownFile);
		pThread = AfxBeginThread(CDownInfo::GetFileFromHttp, &DownInfo);
		// 等待下载线程结束
		while ( pThread && DownInfo.bBusy )
			WAIT_HERE;

		// 下载结果
		m_List.SetItemText(i, 3, DownInfo.GetLastError());
		if( CDownInfo::ERR_OK != DownInfo.nRet )
		{
			if( CDownInfo::ERR_ABORT != DownInfo.nRet )
			{
				Step(COUNT_STEPS - 1);
				m_strStatus = _T("下载出错！\n") + strUrl;
				UpdateData(FALSE);
				bSucc = FALSE;
			}
			else	// 用户中止下载
				return;
		}

		////////////////////////////////////////////////////////////////////////
		if( bSucc )
		{
			// 处理 BAK 变量，备份文件
			if( !m_sBakDir.IsEmpty() && WND::IsExistFile(m_sBakDir) )
			{
				if( ! REMOTE_BAK(i).IsEmpty() )
				{
					ParseFileName( arrBakDel, REMOTE_BAK(i) );
					BakOrDelFiles( arrBakDel, TRUE );
				}
			}

			// 检查下载文件扩展名
			str = strDownFile.Mid( strDownFile.ReverseFind(_T('.'))+1);
			str.MakeLower();

			// 解压 7z, zip
			if( _T("7z") == str || _T("zip") == str )
			{
				if( !b7z && !WND::IsExistFile(m_sAbsDir+_T("7za.exe")) )
					b7z = ReleaseFile(IDR_7ZA_EXE, m_sAbsDir+_T("7za.exe"));
				else
					b7z = TRUE;

				if( b7z )
				{
					// 调用 7-Zip 命令行，从临时目录解压到当前目录
					// 命令行: 7za.exe x -y -o"workDir" Archive -p"password"
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
						m_List.SetItemText( i, 3, _T("解压失败！") );
						bSucc = FALSE;
					}
				}
				else
				{
					m_List.SetItemText(i, 3, _T("解压失败！"));
					bSucc = FALSE;
				}
			}
			// 运行
			else if( _T("exe") == str || _T("bat") == str )
			{
				ShowWindow(SW_MINIMIZE);
				WND::WaitProcess(m_sTmpDir + strDownFile, TRUE, INFINITE);
				ShowWindow(SW_SHOWNORMAL);
			}
			// 覆盖可执行文件
			else if( _T("ex_") == str )
			{
				CString strRename = strDownFile;
				strRename.SetAt( strRename.GetLength()-1, _T('e') );

				if( !CopyFile( m_sTmpDir + strDownFile, m_sAbsDir + strRename, FALSE ) )
				{
					m_List.SetItemText(i, 3, _T("覆盖exe文件失败！"));
					bSucc = FALSE;
				}
			}
			// 覆盖其它文件
			else
			{
				if( !CopyFile(m_sTmpDir + strDownFile, m_sAbsDir + strDownFile, FALSE) )
				{
					m_List.SetItemText(i, 3, _T("复制文件失败！"));
					bSucc = FALSE;
				}
			}

			// 处理 DEL 变量，删除文件
			if( ! REMOTE_DEL(i).IsEmpty() )
			{
				ParseFileName( arrBakDel, REMOTE_DEL(i) );
				BakOrDelFiles( arrBakDel, FALSE);
			}
		}
		PlaceProgress(-1);		// 隐藏进度条

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
		// 改变服务器地址
		if( stRemote.iDepend < 0 )
		{
			fini.SetVarStr(_T("WELCOME"), _T("URL"), stRemote.sIniUrl, 0);
		}
		// 查看日志
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
 *	重置配置文件
 *  将程配置文件中的版本号全置 0
 *  调用此方法后需用远程配置文件替换本地配置文件
 */
BOOL CLiveUpdateDlg::ResetIniFile(LPCTSTR szIniFile, LPCSTR szVer/*=_T("0")*/)
{
	// 复位 [INSTALL] 段所有组件版号
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
		if( IDYES == MessageBox(_T("正确下载更新，确定要停止下载吗?"),
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
			// 删除临时目录
			m_sTmpDir.TrimRight(_T("\\"));
			if( WND::IsExistFile(m_sTmpDir) )
				WND::DeleteFileEx(m_sTmpDir);
		}

		if(m_bCheckDelBak)
		{
			if( m_sBakDir.IsEmpty() )
				m_sBakDir = m_sAbsDir + _T("Backup");

			// 删除备份目录
			m_sBakDir.TrimRight(_T('\\'));
			if( WND::IsExistFile(m_sBakDir) )
				WND::DeleteFileEx(m_sBakDir);
		}

// 		if( WND::IsExistFile(m_sAbsDir + _T("7za.exe")) )
// 			WND::DeleteFileEx( m_sAbsDir + _T("7za.exe") );

		// 自我更新，存在 LiveUpdate.upd 文件时释出 bat 文件
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
		CString(_T(" 正在运行！\n先关闭程序才能进行更新！\n\n"))
		+ _T("选择 [确定] 将强制关闭程序...\n")
		+ _T("选择 [取消] 停止更新，手动关闭后再更新...");

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
	GetDlgItem(IDCANCEL)->SetWindowText(_T("关闭"));
	return 0;
}

void CLiveUpdateDlg::PlaceProgress(int row, int col/* = -1*/)
{
	CRect rcProg, rcList, rc;
	m_prog.GetWindowRect(&rcProg);
	ScreenToClient(&rcProg);
	m_List.GetWindowRect(&rcList);
	ScreenToClient(&rcList);

	// 复位
	if( 0 == row && -1 == col )
	{
		m_prog.MoveWindow(&rcProg);
		m_prog.ShowWindow(SW_SHOW);
	}
	// 隐藏
	else if( -1 == row && -1 == col )
	{
		m_prog.SetPos(0);
		m_prog.ShowWindow(SW_HIDE);
	}
	// 移到列表子项
	else if( row >= 0 && col >= 0)
	{
		ASSERT(row < m_List.GetItemCount() && col < 4);
		m_List.GetSubItemRect(row, col, LVIR_LABEL, rc);
		
		rc.OffsetRect(rcList.left+4, rcList.top+3);
		rc.InflateRect(0, 0, -4, -3);
		m_prog.MoveWindow(&rc, TRUE);
		m_prog.SetPos(0);
		m_prog.ShowWindow(SW_SHOW);
		GetDlgItem(IDCANCEL)->SetWindowText(_T("取消"));
	}
}

BOOL CLiveUpdateDlg::GetProductList(LPCTSTR szIniFile, PINICFG pIniCfg, BOOL bIsRemote)
{
	// 读文件
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
		// 缺省值
#ifdef _MY3D_
		if( pIniCfg->sIniUrl.IsEmpty() )
			pIniCfg->sIniUrl = _T("https://github.com/SeanTo/LiveUpdate");
		if( pIniCfg->sHome.IsEmpty() )
			pIniCfg->sHome = _T("https://github.com/SeanTo/LiveUpdate");
#endif // _MY3D_
		if( pIniCfg->sTip.IsEmpty() )
			pIniCfg->sTip = _T("欢迎使用 LiveUpdate 通用升级程序！");
		
		// 提示语句
		m_stcWelcome.SetWindowText(pIniCfg->sTip);
		
		// 组件列表
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
 *  读取配置文件到 IniCfg 结构	
 */
BOOL CLiveUpdateDlg::ReadVersion(const CString &strIniFile, PINICFG pCfg)
{
	ResetIniCfg(pCfg);

	// 打开配置文件
	CIniFile fini;
	if( !fini.Create(strIniFile) )
		return FALSE;
	
	// 段名、变量名、值
	CString strSect, strVar, str;
	
	// [WELCOME] : 配置文件地址、主页地址、提示语、版本、更新组序号
	strSect = _T("WELCOME");
	fini.GetVarStr(strSect, _T("URL"),	pCfg->sIniUrl);
	fini.GetVarStr(strSect, _T("HOME"), pCfg->sHome);
	fini.GetVarStr(strSect, _T("TIP"),	pCfg->sTip);
	fini.GetVarStr(strSect, _T("VER"),	pCfg->sIniVer);
	fini.GetVarInt(strSect, _T("DEPEND"), pCfg->iDepend);
	
	// [PROCESS] : 终止进程、更新日志、日志版本
	strSect = _T("PROCESS");
	fini.GetVarStr(strSect, _T("RUN"), pCfg->sRun);
	fini.GetVarStr(strSect, _T("KILL"), pCfg->sKill);
	fini.GetVarStr(strSect, _T("READ"), pCfg->sLog);
	fini.GetVarStr(strSect, _T("WHATNEWS"), pCfg->sLogVer);
	
	// [INSTALL] : 组件列表、解码密码、超时
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
			// 名称
			str.Empty();
			strVar.Format(_T("NAME%02d"), i);
			fini.GetVarStr(strSect, strVar, str);
			pCfg->arrInst.Add(str);
			
			// 版本
			str.Empty();
			strVar.Format(_T("VER%02d"), i);
			fini.GetVarStr(strSect, strVar, str);
			pCfg->arrInst.Add(str);
			
			// 下载地址
			str.Empty();
			strVar.Format(_T("URL%02d"), i);
			fini.GetVarStr(strSect, strVar, str);
			pCfg->arrInst.Add(str);
			
			// 备份文件
			str.Empty();
			strVar.Format(_T("BAK%02d"), i);
			fini.GetVarStr(strSect, strVar, str);
			pCfg->arrInst.Add(str);
			
			// 删除文件
			str.Empty();
			strVar.Format(_T("DEL%02d"), i);
			fini.GetVarStr(strSect, strVar, str);
			pCfg->arrInst.Add(str);

			// 摘要
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
 *	INICFG 外部构造函数
 */
void CLiveUpdateDlg::ResetIniCfg(PINICFG pIniCfg)
{
#ifdef _7Z_PASSWORD_
	pIniCfg->sPassword.Empty();
#endif // _7Z_PASSWORD_
	// 复位
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
 *	响应列表前的检查框状态改变
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
 *	分解由字符 ch 隔开的文件名串
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
 *	备份/删除 arr 中给出的所有文件
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
 *	创建备份目录
 *  目录指定优先级: 远程配置>本地配置>默认Backup
 */
BOOL CLiveUpdateDlg::CreateBackupDirectory()
{
	CString str;
	m_sBakDir.Empty();

	if( !stRemote.sBakDir.IsEmpty() )
	{
		// 根据远程配置创建备份目录
		str = m_sAbsDir + stRemote.sBakDir;
		if( !WND::IsExistFile(str) )
			CreateDirectory(str, FALSE);
		if( WND::IsExistFile(str) )
			m_sBakDir = str;
	}
	
	if( m_sBakDir.IsEmpty() )
	{
		// 根据本地配置创建备份目录
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
		// 默认备份目录
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
	//定位自定义资源
	HRSRC hRsrc = FindResource(NULL, MAKEINTRESOURCE(dwID), _T("FILE"));
	if (NULL == hRsrc)
		return FALSE;
	
	//资源大小
	DWORD dwSize = SizeofResource(NULL, hRsrc);
	if (0 == dwSize)
		return FALSE;
	
	//加载资源
	HGLOBAL hGlobal = LoadResource(NULL, hRsrc);
	if (NULL == hGlobal)
		return FALSE;
	
	//锁定资源
	LPVOID pBuffer = LockResource(hGlobal);
	if (NULL == pBuffer)
		return FALSE;
	
	//将资源写入文件
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
