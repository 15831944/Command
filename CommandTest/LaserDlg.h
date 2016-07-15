#pragma once


// CLaserDlg 對話方塊

class CLaserDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CLaserDlg)

public:
	CLaserDlg(CWnd* pParent = NULL);   // 標準建構函式
	virtual ~CLaserDlg();

// 對話方塊資料
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG6 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支援
    CWnd* pMain;
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedBtnzero();
    afx_msg void OnBnClickedBtnlasertob();
    afx_msg void OnBnClickedBtnb();
    afx_msg void OnBnClickedBtnsetlaser();
};
