#pragma once


// CCheckResult ��ܤ��

class CCheckResult : public CDialogEx
{
	DECLARE_DYNAMIC(CCheckResult)

public:
	CCheckResult(CWnd* pParent = NULL);   // �зǫغc�禡
	virtual ~CCheckResult();

// ��ܤ�����
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG12 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �䴩

	DECLARE_MESSAGE_MAP()
};
