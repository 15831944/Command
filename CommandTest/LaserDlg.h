#pragma once


// CLaserDlg ��ܤ��

class CLaserDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CLaserDlg)

public:
	CLaserDlg(CWnd* pParent = NULL);   // �зǫغc�禡
	virtual ~CLaserDlg();

// ��ܤ�����
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG6 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �䴩
	CWnd* pMain;
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnzero();
	afx_msg void OnBnClickedBtnlasertob();
	afx_msg void OnBnClickedBtnb();
	afx_msg void OnBnClickedBtnsetlaser();
    virtual BOOL OnInitDialog();
    afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
    afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
};
