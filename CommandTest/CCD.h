#pragma once


// CCCD 對話方塊
static CWinThread *pTouchMoveThread = NULL;
class CCCD : public CDialogEx
{
	DECLARE_DYNAMIC(CCCD)

public:
	CCCD(CWnd* pParent = NULL);   // 標準建構函式
	virtual ~CCCD();

// 對話方塊資料
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CCD };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支援

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
