#pragma once


// CEmgDlg ��ܤ��

class CEmgDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CEmgDlg)

public:
	CEmgDlg(CWnd* pParent = NULL);   // �зǫغc�禡
	virtual ~CEmgDlg();

// ��ܤ�����
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG10 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �䴩

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedOk();
};
