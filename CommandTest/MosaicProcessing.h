#pragma once
#include "CommandTestDlg.h"
#include "afxcmn.h"

// CMosaicProcessing 對話方塊

class CMosaicProcessing : public CDialogEx
{
	DECLARE_DYNAMIC(CMosaicProcessing)

public:
	CMosaicProcessing(CWnd* pParent = NULL);   // 標準建構函式
	virtual ~CMosaicProcessing();

// 對話方塊資料
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG14 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支援

	DECLARE_MESSAGE_MAP()
public:
    CProgressCtrl m_Progress;
    CCommandTestDlg* pCCommandTestDlg;
    short num_stepadd;
public:
	virtual BOOL OnInitDialog();  
    afx_msg void OnTimer(UINT_PTR nIDEvent);

    virtual void OnCancel();
    virtual void OnOK();
};
