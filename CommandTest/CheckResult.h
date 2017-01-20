#pragma once


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

	DECLARE_MESSAGE_MAP()
};
