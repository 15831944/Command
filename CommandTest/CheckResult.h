#pragma once
#include "afxcmn.h"
#include "XListCtrl.h"
#include "CommandTestDlg.h"


// CCheckResult 對話方塊

class CCheckResult : public CDialogEx
{
	DECLARE_DYNAMIC(CCheckResult)

public:
	CCheckResult(CWnd* pParent = NULL);   // 標準建構函式
	virtual ~CCheckResult();

// 對話方塊資料
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG12 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支援
    BOOL NoPushEsc;//用來鎖Esc
	DECLARE_MESSAGE_MAP()
public:
    
	CXListCtrl m_ListCheck;
    CWinThread* m_pLoadlist;
    CCommandTestDlg* pCCommandTestDlg;  
    CDialog* m_pPictureViewDlg;
    long volatile lEndthread;
    CString m_ImagePath;
    CString m_ImageName;
public:
    static UINT LoadListThread(LPVOID pParam);
    virtual BOOL OnInitDialog();
    virtual void OnCancel();
    virtual BOOL DestroyWindow();
    afx_msg void OnClose();
    afx_msg void OnDestroy();
    afx_msg void OnNMDblclkList1(NMHDR *pNMHDR, LRESULT *pResult);
    virtual void OnOK();
};
