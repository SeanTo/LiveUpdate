// LiveUpdateDlg.h : header file
//

#if !defined(AFX_LiveUpdateDLG_H__CC76064E_053D_4E9C_A7B3_BB4C4842E1C9__INCLUDED_)
#define AFX_LiveUpdateDLG_H__CC76064E_053D_4E9C_A7B3_BB4C4842E1C9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <afxinet.h>

#include "..\Share\ListCtrlEx.h"

//////////////////////////////////////////////////////////////////////////

// #define _7Z_PASSWORD_
typedef struct
{
#ifdef _7Z_PASSWORD_
	CString sPassword;	// 解压密码
#endif // _7Z_PASSWORD_

	CString sIniUrl;	// 下载配置文件地址
	CString sHome;		// 主页
	CString sTip;		// 提示语句
	CString sIniVer;	// 配置文件版本号

	CString sLog;		// 更新日志
	CString sLogVer;	// 日志版本

	CString sRun, sKill;// 结束进程
	CString sBakDir;	// 备份目录


	CStringArray arrInst;	// 安装列表
	int			 iInst;		// 安装组件数
	int			 iDepend;	// 更新组件序号
	DWORD		 dwTime;	// 安装超时时间

} INICFG, *PINICFG;

/////////////////////////////////////////////////////////////////////////////
// CLiveUpdateDlg dialog

class CLiveUpdateDlg : public CDialog
{
// Construction
public:
	CLiveUpdateDlg(CWnd* pParent = NULL);	// standard constructor
							 
// Dialog Data
	//{{AFX_DATA(CLiveUpdateDlg)
	enum { IDD = IDD_LiveUpdate_DIALOG };
	CEdit	m_editSummary;
	CStatic	m_stcWelcome;
	CButton	m_btnOk;
	CListCtrlEx m_List;
	CProgressCtrl	m_prog;
	CString	m_strStatus;
	BOOL	m_bCheckDelBak;
	BOOL	m_bCheckDelTemp;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLiveUpdateDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CLiveUpdateDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	virtual void OnOK();
	afx_msg void OnHomepage();
	virtual void OnCancel();
	afx_msg void OnItemchangedList1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclkList1(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	afx_msg LRESULT OnDownEnd(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
		
	HICON m_hIcon;
	
private:		
	BOOL ResetIniFile(LPCTSTR szIniFile, LPCSTR szVer=_T("0"));
	BOOL ReadVersion(const CString &strIniFile, PINICFG pCfg);
	BOOL GetProductList(LPCTSTR szIniFile, PINICFG pIniCfg, BOOL bIsRemote);
	BOOL ParseUrl(const CString &strUrl);
	int  ParseNewVersion();

	void PlaceProgress(int row, int col = -1);
	int  Step() const { return m_iStep; };
	void Step(int iStep);
	void RunStep1();
	void RunStep2();
	void RunStep3();
	
	BOOL CloseRunning();
	
	CWinThread	*pThread;	// 线程指针
	int			m_iStep;	// 步序号
	
	CString m_sAbsDir;		// 当前目录
	CString m_sBakDir;		// 备份目录
	CString m_sTmpDir;		// 临时目录
	CString m_strBat;		// 自我删除的批处理名
	
	CString m_sLocalIniFile;	// 本地配置文件名
	CString m_sRemoteIniFile;	// 远程配置文件名(下载到本地)

private:
	int  m_iDiscovers;
	BOOL ReleaseFile(DWORD dwID, LPCTSTR file);
	BOOL CreateBackupDirectory();

	void BakOrDelFiles(const CStringArray &arr, BOOL bIsBak=TRUE);
	int  ParseFileName(CStringArray &arr, const CString &strDel, const TCHAR &ch=_T(';'));
	void ResetIniCfg(PINICFG pIniCfg);
	static const CString STEPS[];	// 步序名
// 	static const CString ZIP_FMT[];	// 支持压缩格式
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LiveUpdateDLG_H__CC76064E_053D_4E9C_A7B3_BB4C4842E1C9__INCLUDED_)
