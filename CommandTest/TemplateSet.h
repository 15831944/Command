#pragma once
#include "afxwin.h"
#include <vector>

// CTemplateSet ��ܤ��

class CTemplateSet : public CDialogEx
{
	DECLARE_DYNAMIC(CTemplateSet)

public:
	CTemplateSet(CWnd* pParent = NULL);   // �зǫغc�禡
	virtual ~CTemplateSet();

// ��ܤ�����
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG11 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �䴩

	DECLARE_MESSAGE_MAP()
public:
    virtual BOOL OnInitDialog();
    CListBox m_ListOK;
    CListBox m_ListNG;
    std::vector<CString> AllModelName;
    afx_msg void OnBnClickedOk();
    BOOL ListAllFileInDirectory(LPTSTR szPath, LPTSTR szName);
    CWnd *pMain;
};
