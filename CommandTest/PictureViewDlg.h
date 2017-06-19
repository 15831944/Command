#pragma once
#include "afxwin.h"

// CPictureViewDlg 對話方塊

class CPictureViewDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CPictureViewDlg)

public:
	CPictureViewDlg(CWnd* pParent = NULL);   // 標準建構函式
	virtual ~CPictureViewDlg();

// 對話方塊資料
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG13 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支援

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
    BOOL FileExist(LPCWSTR FilePathName);
};
