#pragma once
#include "afxcmn.h"


// CDefault 對話方塊

class CDefault : public CDialogEx
{
	DECLARE_DYNAMIC(CDefault)

public:
	CDefault(CWnd* pParent = NULL);   // 標準建構函式
	virtual ~CDefault();

// 對話方塊資料
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG1 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支援

	DECLARE_MESSAGE_MAP()
public:
    CDialog* pQuestion;
	CListCtrl m_ListDefault;
	CEdit m_Edit;
	int m_OldListRow, m_OldListColumn;
	virtual BOOL OnInitDialog();
	void ParamShow();
	void ParamModify();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnNMDblclkList1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickList1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMRClickList1(NMHDR *pNMHDR, LRESULT *pResult);
	void CreateEdit(NM_LISTVIEW * pEditCtrl, CEdit * m_Edit);
	void DistroyEdit(CListCtrl * list, CEdit * distroyedit, int & Item, int & SubItem);
};
