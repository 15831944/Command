#pragma once
#include "CommandTestDlg.h"
#include "afxcmn.h"

// CMosaicProcessing ��ܤ��

class CMosaicProcessing : public CDialogEx
{
	DECLARE_DYNAMIC(CMosaicProcessing)

public:
	CMosaicProcessing(CWnd* pParent = NULL);   // �зǫغc�禡
	virtual ~CMosaicProcessing();

// ��ܤ�����
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG14 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �䴩

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
