#pragma once


// CBlock 對話方塊

class CBlock : public CDialogEx
{
	DECLARE_DYNAMIC(CBlock)

public:
	CBlock(CWnd* pParent = NULL);   // 標準建構函式
	virtual ~CBlock();

// 對話方塊資料
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG5 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支援
	DECLARE_MESSAGE_MAP()
    CButton* Button;
    int TotalBlock;
public:
    virtual BOOL OnInitDialog();
    virtual void OnCancel();
    virtual void OnOK();
    afx_msg void OnBlockClick(UINT uId);
};
