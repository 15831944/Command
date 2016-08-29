#pragma once


// CEmgDlg 對話方塊

class CEmgDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CEmgDlg)

public:
	CEmgDlg(CWnd* pParent = NULL);   // 標準建構函式
	virtual ~CEmgDlg();

// 對話方塊資料
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG10 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支援

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedOk();
};
