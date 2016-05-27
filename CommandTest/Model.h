#pragma once
#include "afxcmn.h"
#include <vector>

// CModel ��ܤ��

class CModel : public CDialogEx
{
	DECLARE_DYNAMIC(CModel)

public:
	CModel(CWnd* pParent = NULL);   // �зǫغc�禡
	virtual ~CModel();

// ��ܤ�����
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG3 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �䴩

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
};
