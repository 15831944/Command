#pragma once
#include "afxcmn.h"


// CPositionModify 對話方塊

class CPositionModify : public CDialogEx
{
	DECLARE_DYNAMIC(CPositionModify)

public:
	CPositionModify(CWnd* pParent = NULL);   // 標準建構函式
	virtual ~CPositionModify();

// 對話方塊資料
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG7 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支援

	DECLARE_MESSAGE_MAP()
public:
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    virtual void OnCancel();
    CListCtrl m_Listposition;
    int m_OldListRow, m_OldListColumn;
    CEdit m_Edit;
    CWnd* pMain;
    void CreateEdit(NM_LISTVIEW * pEditCtrl, CEdit * m_Edit);
    void DistroyEdit(CListCtrl * list, CEdit * distroyedit, int & Item, int & SubItem);
    void PositionModifyMO();
    void ListRefresh();
    afx_msg void OnNMRClickModifylist(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnNMDblclkModifylist(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnNMClickModifylist(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnInsert3();
    afx_msg void OnDelete3();
    afx_msg void OnBnClickedBtnnew();
    afx_msg void OnBnClickedBtnclear();
    afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
    afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
};
