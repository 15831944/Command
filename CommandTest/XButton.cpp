// XButton.cpp : 實作檔
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

// CXButton 訊息處理常式

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
    //判斷滑鼠是否載按鈕區域內
    if (point.x > Rect.Width() || point.x<0 || point.y > Rect.Height() || point.y < 0)//區域外
    {
        if (nFlags == 1)//判斷按鈕是否被按下
        {
            KillTimer(1);
        }
    }
    else//區域內
    {
    }
    CButton::OnMouseMove(nFlags, point);
}
