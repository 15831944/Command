#pragma once


// CBlock ��ܤ��

class CBlock : public CDialogEx
{
	DECLARE_DYNAMIC(CBlock)

public:
	CBlock(CWnd* pParent = NULL);   // �зǫغc�禡
	virtual ~CBlock();

// ��ܤ�����
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG5 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �䴩
	DECLARE_MESSAGE_MAP()
    CButton* Button;
    int TotalBlock;
public:
    virtual BOOL OnInitDialog();
    virtual void OnCancel();
    virtual void OnOK();
    afx_msg void OnBlockClick(UINT uId);
};
