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
	CString sPassword;	// ��ѹ����
#endif // _7Z_PASSWORD_

	CString sIniUrl;	// ���������ļ���ַ
	CString sHome;		// ��ҳ
	CString sTip;		// ��ʾ���
	CString sIniVer;	// �����ļ��汾��

	CString sLog;		// ������־
	CString sLogVer;	// ��־�汾

	CString sRun, sKill;// ��������
	CString sBakDir;	// ����Ŀ¼


	CStringArray arrInst;	// ��װ�б�
	int			 iInst;		// ��װ�����
	int			 iDepend;	// ����������
	DWORD		 dwTime;	// ��װ��ʱʱ��

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
	
	CWinThread	*pThread;	// �߳�ָ��
	int			m_iStep;	// �����
	
	CString m_sAbsDir;		// ��ǰĿ¼
	CString m_sBakDir;		// ����Ŀ¼
	CString m_sTmpDir;		// ��ʱĿ¼
	CString m_strBat;		// ����ɾ������������
	
	CString m_sLocalIniFile;	// ���������ļ���
	CString m_sRemoteIniFile;	// Զ�������ļ���(���ص�����)

private:
	int  m_iDiscovers;
	BOOL ReleaseFile(DWORD dwID, LPCTSTR file);
	BOOL CreateBackupDirectory();

	void BakOrDelFiles(const CStringArray &arr, BOOL bIsBak=TRUE);
	int  ParseFileName(CStringArray &arr, const CString &strDel, const TCHAR &ch=_T(';'));
	void ResetIniCfg(PINICFG pIniCfg);
	static const CString STEPS[];	// ������
// 	static const CString ZIP_FMT[];	// ֧��ѹ����ʽ
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LiveUpdateDLG_H__CC76064E_053D_4E9C_A7B3_BB4C4842E1C9__INCLUDED_)
