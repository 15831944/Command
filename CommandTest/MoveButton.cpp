// MoveButton.cpp : ��@��
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

// CMoveButton �T���B�z�`��
/*���s���U*/
void CMoveButton::OnLButtonDown(UINT nFlags, CPoint point)
{
    SetTimer(1, 1, NULL);  
    CButton::OnLButtonDown(nFlags, point);
}
/*���s��}*/
void CMoveButton::OnLButtonUp(UINT nFlags, CPoint point)
{
#ifdef MOVE
    MO_DecSTOP();
#endif // MOVE
    KillTimer(1);
    CButton::OnLButtonUp(nFlags, point);
}
/*�p�ư��榸��*/
void CMoveButton::OnTimer(UINT_PTR nIDEvent)
{
    CWnd* pMain = AfxGetApp()->m_pMainWnd;
#ifdef MOVE
    switch (((CCamera*)((CCommandTestDlg*)pMain)->m_pCameraDlg)->RaiChoose)
    {
    case 1:
        if(!MO_ReadIsDriving(7))
            MO_Do3DLineMove(MoveX, MoveY, MoveZ, 30000, 120000, 5000);
        break;
    case 2:
        if (!MO_ReadIsDriving(7))
            MO_Do3DLineMove(MoveX, MoveY, MoveZ, 15000, 80000, 5000);
        break;
    case 3:
        if (!MO_ReadIsDriving(7))
            MO_Do3DLineMove(MoveX, MoveY, MoveZ, 5000, 60000, 1000);
        break;
    default:
        //MessageBox(_T("�{���X�{���~!"));
        break;
    }
#endif // MOVE
    CButton::OnTimer(nIDEvent);
}
