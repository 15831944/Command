#pragma once
#include "afxcmn.h"
#include "NumEdit.h"

// CLineContinuous ��ܤ��

class CLineContinuous : public CDialogEx
{
	DECLARE_DYNAMIC(CLineContinuous)

public:
	CLineContinuous(CWnd* pParent = NULL);   // �зǫغc�禡
	virtual ~CLineContinuous();

// ��ܤ�����
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG8 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �䴩

	DECLARE_MESSAGE_MAP()
public:
    virtual BOOL OnInitDialog();
    virtual void OnCancel();
    void CreateEdit(NM_LISTVIEW * pEditCtrl, CEdit * m_Editbox1);
    void DistroyEdit(CListCtrl * list, CEdit * distroyedit, int & Item, int & SubItem);
    virtual void OnOK();
    void ListRefresh();
    void LineCoutinuousModify();
    CListCtrl m_ListLCPoint;
    int m_OldListRow, m_OldListColumn;
    CNumEdit m_Edit;
    CWnd* pMain;
    afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
    afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
    afx_msg void OnNMClickList1(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnNMDblclkList1(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnNMRClickList1(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnInsert();
    afx_msg void OnDelete();
    afx_msg void OnMove();
    afx_msg void OnBnClickedBtnlisttoexcel();
    afx_msg void OnBnClickedBtnexceltolist();
};
