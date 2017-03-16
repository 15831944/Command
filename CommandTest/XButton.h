#pragma once


// CXButton

class CXButton : public CButton
{
	DECLARE_DYNAMIC(CXButton)

public:
	CXButton();
	virtual ~CXButton();
    LONG ControlIDMsg;
    HWND hwnd;

protected:
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
};


