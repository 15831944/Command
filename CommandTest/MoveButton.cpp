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
    MoveW = 0;
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
    /*備註:再校正時的機械座標偏移量不會歸0*/
    CWnd* pMain = AfxGetApp()->m_pMainWnd;
    if (MoveX == 6)//X+
    {
        if (((CCommandTestDlg*)pMain)->a.m_Action.m_MachineOffSet.x == TCXYOffsetInit && ((CCommandTestDlg*)pMain)->a.m_Action.m_MachineOffSet.y == TCXYOffsetInit || ((CCamera*)((CCommandTestDlg*)pMain)->m_pCameraDlg)->TCOffstAdjust)//尚未執行非同軸校正
            MoveXYZW((405000 - ((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition(UseMachineMode).x), 0, 0, 0);//往+行程距為285000//總行程距410000
        else
            MoveXYZW((PosXWorkRange - ((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition(UseNeedleMode).x), 0, 0, 0);
    }
    else if (MoveX == 4)//X-
    {
        if (((CCommandTestDlg*)pMain)->a.m_Action.m_MachineOffSet.x == TCXYOffsetInit && ((CCommandTestDlg*)pMain)->a.m_Action.m_MachineOffSet.y == TCXYOffsetInit || ((CCamera*)((CCommandTestDlg*)pMain)->m_pCameraDlg)->TCOffstAdjust)//尚未執行非同軸校正
            MoveXYZW(-(((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition(UseMachineMode).x) + 5000, 0, 0, 0);//往-行程距為200000 
        else
            MoveXYZW((NegXWorkRange - ((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition(UseNeedleMode).x), 0, 0, 0);
    }
    else if (MoveY == 8)//Y+
    {
        if (((CCommandTestDlg*)pMain)->a.m_Action.m_MachineOffSet.x == TCXYOffsetInit && ((CCommandTestDlg*)pMain)->a.m_Action.m_MachineOffSet.y == TCXYOffsetInit || ((CCamera*)((CCommandTestDlg*)pMain)->m_pCameraDlg)->TCOffstAdjust)//尚未執行非同軸校正
            MoveXYZW(0, -(((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition(UseMachineMode).y) + 5000, 0, 0);//往-行程距為200000
        else
            MoveXYZW(0, (NegYWorkRange - ((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition(UseNeedleMode).y), 0, 0);
    }
    else if (MoveY == 2)//Y-
    {
        if (((CCommandTestDlg*)pMain)->a.m_Action.m_MachineOffSet.x == TCXYOffsetInit && ((CCommandTestDlg*)pMain)->a.m_Action.m_MachineOffSet.y == TCXYOffsetInit || ((CCamera*)((CCommandTestDlg*)pMain)->m_pCameraDlg)->TCOffstAdjust)//尚未執行非同軸校正
            MoveXYZW(0, (405000 - ((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition(UseMachineMode).y), 0, 0);//往+行程距為285000//總行程距410000
        else
            MoveXYZW(0, (PosYWorkRange - ((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition(UseNeedleMode).y), 0, 0);
    }
    else if (MoveZ == 7)//Z+
    {
        if (((CCommandTestDlg*)pMain)->a.m_Action.m_MachineOffSet.x == TCXYOffsetInit && ((CCommandTestDlg*)pMain)->a.m_Action.m_MachineOffSet.y == TCXYOffsetInit || ((CCamera*)((CCommandTestDlg*)pMain)->m_pCameraDlg)->TCOffstAdjust)//尚未執行非同軸校正
            MoveXYZW(0, 0, -(((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition(UseMachineMode).z) + 5000, 0);//往-行程距為10000
        else
            MoveXYZW(0, 0, -(((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition(UseNeedleMode).z), 0);
    }
    else if (MoveZ == 1)//Z-
    {
        if (((CCommandTestDlg*)pMain)->a.m_Action.m_MachineOffSet.x == TCXYOffsetInit && ((CCommandTestDlg*)pMain)->a.m_Action.m_MachineOffSet.y == TCXYOffsetInit || ((CCamera*)((CCommandTestDlg*)pMain)->m_pCameraDlg)->TCOffstAdjust)//尚未執行非同軸校正
            MoveXYZW(0, 0, (110000 - ((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition(UseMachineMode).z), 0);//往+行程距為110000
        else
            MoveXYZW(0, 0, (((CCommandTestDlg*)pMain)->a.m_Action.m_TablelZ - ((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition(UseNeedleMode).z), 0);
    }
    else
    {
        if (!((CCamera*)((CCommandTestDlg*)pMain)->m_pCameraDlg)->TCOffstAdjust && !((CCamera*)((CCommandTestDlg*)pMain)->m_pCameraDlg)->TipToCCDAdjust && !((CCamera*)((CCommandTestDlg*)pMain)->m_pCameraDlg)->PixToPlusAdjust)
        {
            if (MoveW == 9)//W+
            {
                if (((CCommandTestDlg*)pMain)->a.m_Action.m_MachineOffSet.x == TCXYOffsetInit && ((CCommandTestDlg*)pMain)->a.m_Action.m_MachineOffSet.y == TCXYOffsetInit || ((CCamera*)((CCommandTestDlg*)pMain)->m_pCameraDlg)->TCOffstAdjust)//尚未執行非同軸校正
                    MoveXYZW(0, 0, 0, -(((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition(UseMachineMode).w) - 360);//往-行程距為360度
                else
                    MoveXYZW(0, 0, 0, -(((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition(UseNeedleMode).w) + NegWWorkRange);
            }
            else if (MoveW == 3)//W-
            {
                if (((CCommandTestDlg*)pMain)->a.m_Action.m_MachineOffSet.x == TCXYOffsetInit && ((CCommandTestDlg*)pMain)->a.m_Action.m_MachineOffSet.y == TCXYOffsetInit || ((CCamera*)((CCommandTestDlg*)pMain)->m_pCameraDlg)->TCOffstAdjust)//尚未執行非同軸校正
                    MoveXYZW(0, 0, 0, (360 - ((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition(UseMachineMode).w));//往+行程距為360度
                else
                    MoveXYZW(0, 0, 0, (PosWWorkRange - ((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition(UseNeedleMode).w));//最大動到360度
            }
        }
    }
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
/*XYZW按鈕移動*/
void CMoveButton::MoveXYZW(int MoveX, int MoveY, int MoveZ, double MoveW) {
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
                        if (!MO_ReadIsDriving(15))
                            ((CCommandTestDlg*)pMain)->a.m_Action.MCO_JogMove(MoveX, MoveY, MoveZ, JOGHSpeedW, JOGHSpeedA, JOGHSpeedI, MoveW, ((CCamera*)((CCommandTestDlg*)pMain)->m_pCameraDlg)->WJOGMode);
                        //MO_Do3DLineMove(MoveX, MoveY, MoveZ, 80000, 1200000, 6000);
                        break;
                    case 2:
                        if (!MO_ReadIsDriving(15))
                            ((CCommandTestDlg*)pMain)->a.m_Action.MCO_JogMove(MoveX, MoveY, MoveZ, JOGMSpeedW, JOGMSpeedA, JOGMSpeedI, MoveW, ((CCamera*)((CCommandTestDlg*)pMain)->m_pCameraDlg)->WJOGMode);
                        //MO_Do3DLineMove(MoveX, MoveY, MoveZ, 50000, 800000, 5000);
                        break;
                    case 3:
                        if (!MO_ReadIsDriving(15))
                            ((CCommandTestDlg*)pMain)->a.m_Action.MCO_JogMove(MoveX, MoveY, MoveZ, JOGLSpeedW, JOGLSpeedA, JOGLSpeedI, MoveW, ((CCamera*)((CCommandTestDlg*)pMain)->m_pCameraDlg)->WJOGMode);
                        //MO_Do3DLineMove(MoveX, MoveY, MoveZ, 5000, 50000, 1000);
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
