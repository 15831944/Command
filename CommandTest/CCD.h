#pragma once


// CCCD ��ܤ��
static CWinThread *pTouchMoveThread = NULL;
class CCCD : public CDialogEx
{
	DECLARE_DYNAMIC(CCCD)

public:
	CCCD(CWnd* pParent = NULL);   // �зǫغc�禡
	virtual ~CCCD();

// ��ܤ�����
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CCD };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �䴩

	DECLARE_MESSAGE_MAP()
public:
	double m_MeasPosX, m_MeasPosY;
	double m_TouchReturnXPos, m_TouchReturnYPos;
	BOOL m_TouchMoveThreadStop;
	BOOL m_TouchMoveSwitch;
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	static UINT TouchMove(LPVOID pParam);
};
