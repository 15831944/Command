#pragma once


// CMoveButton

class CMoveButton : public CButton
{
	DECLARE_DYNAMIC(CMoveButton)

public:
	CMoveButton();
    int MoveX;
    int MoveY;
    int MoveZ;
    DOUBLE MoveW;
    virtual ~CMoveButton();

protected:
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    void MoveXYZ(int MoveX, int MoveY, int MoveZ);
};


