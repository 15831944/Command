#pragma once
#include "afxcmn.h"
#include <vector>

// CModel 對話方塊

class CModel : public CDialogEx
{
	DECLARE_DYNAMIC(CModel)

public:
	CModel(CWnd* pParent = NULL);   // 標準建構函式
	virtual ~CModel();

// 對話方塊資料
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG3 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支援

	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_ModelList;
	int ModelCount;
	std::vector<CString> AllModelName;
public:
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	BOOL ListAllFileInDirectory(LPTSTR szPath, LPTSTR szName);
	afx_msg void OnBnClickedButton1();
	afx_msg void OnNMRClickList1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDeletefile();
};
