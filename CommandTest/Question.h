#pragma once


// CQuestion 對話方塊

class CQuestion : public CDialogEx
{
	DECLARE_DYNAMIC(CQuestion)

public:
	CQuestion(CWnd* pParent = NULL);   // 標準建構函式
	virtual ~CQuestion();

// 對話方塊資料
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG4 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支援

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedButton1();
    afx_msg void OnBnClickedButton2();
    afx_msg void OnBnClickedCancel();
    afx_msg void OnBnClickedButton3();
};
