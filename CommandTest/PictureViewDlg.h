#pragma once
#include "afxwin.h"

// CPictureViewDlg ��ܤ��

class CPictureViewDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CPictureViewDlg)

public:
	CPictureViewDlg(CWnd* pParent = NULL);   // �зǫغc�禡
	virtual ~CPictureViewDlg();

// ��ܤ�����
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG13 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �䴩

	DECLARE_MESSAGE_MAP()
public:
    CString FileName;
    CString FilePath;
    BOOL ShowResult;
public:
    virtual BOOL OnInitDialog();
    virtual void OnCancel();
    virtual void OnOK();
    virtual BOOL DestroyWindow();
};
