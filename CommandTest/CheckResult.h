#pragma once
#include "afxcmn.h"
#include "XListCtrl.h"
#include "CommandTestDlg.h"

// CCheckResult ��ܤ��

class CCheckResult : public CDialogEx
{
	DECLARE_DYNAMIC(CCheckResult)

public:
	CCheckResult(CWnd* pParent = NULL);   // �зǫغc�禡
	virtual ~CCheckResult();

// ��ܤ�����
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG12 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �䴩

	DECLARE_MESSAGE_MAP()
public:
	CXListCtrl m_ListCheck;
    CWinThread* m_pLoadlist;
    CCommandTestDlg* pCCommandTestDlg;
    long volatile lEndthread;
    static UINT LoadListThread(LPVOID pParam);
    virtual BOOL OnInitDialog();
    afx_msg void OnClose();
    virtual void OnCancel();
    afx_msg void OnDestroy();
    virtual BOOL DestroyWindow();
    afx_msg void OnBnClickedButton1();
};
