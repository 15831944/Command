#pragma once
#include "afxcmn.h"
#include "NumEdit.h"

// CLaserAdjust ��ܤ��

class CLaserAdjust : public CDialogEx
{
	DECLARE_DYNAMIC(CLaserAdjust)

public:
	CLaserAdjust(CWnd* pParent = NULL);   // �зǫغc�禡
	virtual ~CLaserAdjust();

// ��ܤ�����
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG9 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �䴩

	DECLARE_MESSAGE_MAP()
public:
    virtual BOOL OnInitDialog();
    CListCtrl m_ListLaserAdjust;
    int m_OldListRow, m_OldListColumn;
    CNumEdit m_Edit;
    CWnd* pMain;
    void ListRefresh();
    void LaserAdjustModify();
    afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
    afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
    virtual void OnCancel();
    virtual void OnOK();
    void CreateEdit(NM_LISTVIEW * pEditCtrl, CEdit * m_Editbox1);
    void DistroyEdit(CListCtrl * list, CEdit * distroyedit, int & Item, int & SubItem);
    afx_msg void OnNMRClickList1(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnNMDblclkList1(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnNMClickList1(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnLaserinster();
    afx_msg void OnLaserdelete();
};
