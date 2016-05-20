// MoveButton.cpp : 實作檔
//

#include "stdafx.h"
#include "MoveButton.h"
#include "Camera.h"
#include "CommandTest.h"
#include "CommandTestDlg.h"


// CMoveButton

IMPLEMENT_DYNAMIC(CMoveButton, CButton)

CMoveButton::CMoveButton()
{
    MoveX = 0;
    MoveY = 0;
    MoveZ = 0;
}
CMoveButton::~CMoveButton()
{
}

BEGIN_MESSAGE_MAP(CMoveButton, CButton)
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONUP()
    ON_WM_TIMER()
END_MESSAGE_MAP()

// CMoveButton 訊息處理常式
/*按鈕按下*/
void CMoveButton::OnLButtonDown(UINT nFlags, CPoint point)
{
    SetTimer(1, 1, NULL);  
    CButton::OnLButtonDown(nFlags, point);
}
/*按鈕放開*/
void CMoveButton::OnLButtonUp(UINT nFlags, CPoint point)
{
#ifdef MOVE
    MO_DecSTOP();
#endif // MOVE
    KillTimer(1);
    CButton::OnLButtonUp(nFlags, point);
}
/*計數執行次數*/
void CMoveButton::OnTimer(UINT_PTR nIDEvent)
{
    CWnd* pMain = AfxGetApp()->m_pMainWnd;
#ifdef MOVE
    switch (((CCamera*)((CCommandTestDlg*)pMain)->m_pCameraDlg)->RaiChoose)
    {
    case 1:
        MO_Do3DLineMove(MoveX, MoveY, MoveZ, 30000, 100000, 5000);
        break;
    case 2:                                      
        MO_Do3DLineMove(MoveX, MoveY, MoveZ, 15000, 80000, 5000);
        break;
    case 3:
        MO_Do3DLineMove(MoveX, MoveY, MoveZ, 5000, 60000, 1000);
        break;
    default:
        //MessageBox(_T("程式出現錯誤!"));
        break;
    }
#endif // MOVE
    CButton::OnTimer(nIDEvent);
}
