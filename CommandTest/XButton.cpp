// XButton.cpp : ��@��
//

#include "stdafx.h"
#include "CommandTest.h"
#include "XButton.h"


// CXButton

IMPLEMENT_DYNAMIC(CXButton, CButton)

CXButton::CXButton()
{

}

CXButton::~CXButton()
{
}


BEGIN_MESSAGE_MAP(CXButton, CButton)
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONUP()
    ON_WM_TIMER()
    ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

// CXButton �T���B�z�`��

void CXButton::OnTimer(UINT_PTR nIDEvent)
{
    KillTimer(1);
    ::PostMessage(hwnd,ControlIDMsg,0,0);
    CButton::OnTimer(nIDEvent);
}
void CXButton::OnLButtonDown(UINT nFlags, CPoint point)
{
    SetTimer(1, 1000, 0);
    CButton::OnLButtonDown(nFlags, point);
}
void CXButton::OnLButtonUp(UINT nFlags, CPoint point)
{
    KillTimer(1);
    CButton::OnLButtonUp(nFlags, point);
}
void CXButton::OnMouseMove(UINT nFlags, CPoint point)
{
    CRect Rect;
    GetWindowRect(Rect);
    //�P�_�ƹ��O�_�����s�ϰ줺
    if (point.x > Rect.Width() || point.x<0 || point.y > Rect.Height() || point.y < 0)//�ϰ�~
    {
        if (nFlags == 1)//�P�_���s�O�_�Q���U
        {
            KillTimer(1);
        }
    }
    else//�ϰ줺
    {
    }
    CButton::OnMouseMove(nFlags, point);
}
