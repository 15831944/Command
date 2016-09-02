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
END_MESSAGE_MAP()

// CMoveButton 訊息處理常式
/*按鈕按下*/
void CMoveButton::OnLButtonDown(UINT nFlags, CPoint point)
{
#ifdef MOVE
    if (MoveX == 350000)
        MoveXYZ((350000 - MO_ReadLogicPosition(0)), 0, 0);
    else if (MoveX == -350000)
        MoveXYZ(-(MO_ReadLogicPosition(0)), 0, 0);
    else if (MoveY == 340000)
        MoveXYZ(0, (340000 - MO_ReadLogicPosition(1)), 0);
    else if (MoveY == -340000)
        MoveXYZ(0, -(MO_ReadLogicPosition(1)), 0);
    else if (MoveZ == 85000)
        MoveXYZ(0, 0, (85000 - MO_ReadLogicPosition(2)));
    else if(MoveZ == -85000)
        MoveXYZ(0, 0, -(MO_ReadLogicPosition(2)));
#endif
    CButton::OnLButtonDown(nFlags, point);
}
/*按鈕放開*/
void CMoveButton::OnLButtonUp(UINT nFlags, CPoint point)
{
#ifdef MOVE
    CWnd* pMain = AfxGetApp()->m_pMainWnd;
    if (((CCommandTestDlg*)pMain)->a.RunStatusRead.RunLoopStatus == 0)
    {
        if (((CCommandTestDlg*)pMain)->a.RunStatusRead.RunStatus == 0 || ((CCommandTestDlg*)pMain)->a.RunStatusRead.RunStatus == 2)
        {
            if (((CCommandTestDlg*)pMain)->a.RunStatusRead.GoHomeStatus == TRUE)
            {
                if (!((CCommandTestDlg*)pMain)->a.RunStatusRead.StepCommandStatus)
                {
                    MO_DecSTOP();
                }
            }
        }
    }
#endif // MOVE
    CButton::OnLButtonUp(nFlags, point);
}
/*XYZ移動*/
void CMoveButton::MoveXYZ(int MoveX, int MoveY, int MoveZ) {
#ifdef MOVE
    CWnd* pMain = AfxGetApp()->m_pMainWnd;
    if (((CCommandTestDlg*)pMain)->a.RunStatusRead.RunLoopStatus == 0)
    {
        if (((CCommandTestDlg*)pMain)->a.RunStatusRead.RunStatus == 0 || ((CCommandTestDlg*)pMain)->a.RunStatusRead.RunStatus == 2)
        {
            if (((CCommandTestDlg*)pMain)->a.RunStatusRead.GoHomeStatus == TRUE)
            {
                if (!((CCommandTestDlg*)pMain)->a.RunStatusRead.StepCommandStatus)
                {
                    switch (((CCamera*)((CCommandTestDlg*)pMain)->m_pCameraDlg)->RaiChoose)
                    {
                    case 1:
                        if (!MO_ReadIsDriving(7))
                            MO_Do3DLineMove(MoveX, MoveY, MoveZ, 80000, 1200000, 6000);
                        break;
                    case 2:
                        if (!MO_ReadIsDriving(7))
                            MO_Do3DLineMove(MoveX, MoveY, MoveZ, 50000, 800000, 5000);
                        break;
                    case 3:
                        if (!MO_ReadIsDriving(7))
                            MO_Do3DLineMove(MoveX, MoveY, MoveZ, 5000, 50000, 1000);
                        break;
                    default:
                        //MessageBox(_T("程式出現錯誤!"));
                        break;
                    }
#ifdef PRINTF
                    //_cwprintf("進來了:%d", ((CCommandTestDlg*)pMain)->a.RunStatusRead.RunStatus);
#endif
                }
            }
        }
    }
#ifdef PRINTF
    //_cprintf("%d", ((CCommandTestDlg*)pMain)->a.RunStatusRead.RunStatus);
#endif
#endif // MOVE
}
