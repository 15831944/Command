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
    MoveW = 0;
}
CMoveButton::~CMoveButton()
{
}

BEGIN_MESSAGE_MAP(CMoveButton, CButton)
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONUP()
END_MESSAGE_MAP()

// CMoveButton �T���B�z�`��
/*���s���U*/
void CMoveButton::OnLButtonDown(UINT nFlags, CPoint point)
{
#ifdef MOVE
    /*�Ƶ�:�A�ե��ɪ�����y�а����q���|�k0*/
    CWnd* pMain = AfxGetApp()->m_pMainWnd;
    if (MoveX == 6)//X+
    {
        if (((CCommandTestDlg*)pMain)->a.m_Action.m_MachineOffSet.x == TCXYOffsetInit && ((CCommandTestDlg*)pMain)->a.m_Action.m_MachineOffSet.y == TCXYOffsetInit || ((CCamera*)((CCommandTestDlg*)pMain)->m_pCameraDlg)->TCOffstAdjust)//�|������D�P�b�ե�
            MoveXYZW((405000 - ((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition(UseMachineMode).x), 0, 0, 0);//��+��{�Z��285000//�`��{�Z410000
        else
            MoveXYZW((PosXWorkRange - ((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition(UseNeedleMode).x), 0, 0, 0);
    }
    else if (MoveX == 4)//X-
    {
        if (((CCommandTestDlg*)pMain)->a.m_Action.m_MachineOffSet.x == TCXYOffsetInit && ((CCommandTestDlg*)pMain)->a.m_Action.m_MachineOffSet.y == TCXYOffsetInit || ((CCamera*)((CCommandTestDlg*)pMain)->m_pCameraDlg)->TCOffstAdjust)//�|������D�P�b�ե�
            MoveXYZW(-(((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition(UseMachineMode).x) + 5000, 0, 0, 0);//��-��{�Z��200000 
        else
            MoveXYZW((NegXWorkRange - ((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition(UseNeedleMode).x), 0, 0, 0);
    }
    else if (MoveY == 8)//Y+
    {
        if (((CCommandTestDlg*)pMain)->a.m_Action.m_MachineOffSet.x == TCXYOffsetInit && ((CCommandTestDlg*)pMain)->a.m_Action.m_MachineOffSet.y == TCXYOffsetInit || ((CCamera*)((CCommandTestDlg*)pMain)->m_pCameraDlg)->TCOffstAdjust)//�|������D�P�b�ե�
            MoveXYZW(0, -(((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition(UseMachineMode).y) + 5000, 0, 0);//��-��{�Z��200000
        else
            MoveXYZW(0, (NegYWorkRange - ((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition(UseNeedleMode).y), 0, 0);
    }
    else if (MoveY == 2)//Y-
    {
        if (((CCommandTestDlg*)pMain)->a.m_Action.m_MachineOffSet.x == TCXYOffsetInit && ((CCommandTestDlg*)pMain)->a.m_Action.m_MachineOffSet.y == TCXYOffsetInit || ((CCamera*)((CCommandTestDlg*)pMain)->m_pCameraDlg)->TCOffstAdjust)//�|������D�P�b�ե�
            MoveXYZW(0, (405000 - ((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition(UseMachineMode).y), 0, 0);//��+��{�Z��285000//�`��{�Z410000
        else
            MoveXYZW(0, (PosYWorkRange - ((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition(UseNeedleMode).y), 0, 0);
    }
    else if (MoveZ == 7)//Z+
    {
        if (((CCommandTestDlg*)pMain)->a.m_Action.m_MachineOffSet.x == TCXYOffsetInit && ((CCommandTestDlg*)pMain)->a.m_Action.m_MachineOffSet.y == TCXYOffsetInit || ((CCamera*)((CCommandTestDlg*)pMain)->m_pCameraDlg)->TCOffstAdjust)//�|������D�P�b�ե�
            MoveXYZW(0, 0, -(((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition(UseMachineMode).z) + 5000, 0);//��-��{�Z��10000
        else
            MoveXYZW(0, 0, -(((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition(UseNeedleMode).z), 0);
    }
    else if (MoveZ == 1)//Z-
    {
        if (((CCommandTestDlg*)pMain)->a.m_Action.m_MachineOffSet.x == TCXYOffsetInit && ((CCommandTestDlg*)pMain)->a.m_Action.m_MachineOffSet.y == TCXYOffsetInit || ((CCamera*)((CCommandTestDlg*)pMain)->m_pCameraDlg)->TCOffstAdjust)//�|������D�P�b�ե�
            MoveXYZW(0, 0, (110000 - ((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition(UseMachineMode).z), 0);//��+��{�Z��110000
        else
            MoveXYZW(0, 0, (((CCommandTestDlg*)pMain)->a.m_Action.m_TablelZ - ((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition(UseNeedleMode).z), 0);
    }
    else
    {
        if (!((CCamera*)((CCommandTestDlg*)pMain)->m_pCameraDlg)->TCOffstAdjust && !((CCamera*)((CCommandTestDlg*)pMain)->m_pCameraDlg)->TipToCCDAdjust && !((CCamera*)((CCommandTestDlg*)pMain)->m_pCameraDlg)->PixToPlusAdjust)
        {
            if (MoveW == 9)//W+
            {
                if (((CCommandTestDlg*)pMain)->a.m_Action.m_MachineOffSet.x == TCXYOffsetInit && ((CCommandTestDlg*)pMain)->a.m_Action.m_MachineOffSet.y == TCXYOffsetInit || ((CCamera*)((CCommandTestDlg*)pMain)->m_pCameraDlg)->TCOffstAdjust)//�|������D�P�b�ե�
                {
                    if (((CCamera*)((CCommandTestDlg*)pMain)->m_pCameraDlg)->WJOGMode)
                    {
                        MoveXYZW(0, 0, 0, -360);//�����m-360
                    }
                    else
                    {
                        MoveXYZW(0, 0, 0, -(((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition(UseMachineMode).w) - 360);//�۹啕-��{�Z��360��
                    }
                }           
                else
                {
                    if (((CCamera*)((CCommandTestDlg*)pMain)->m_pCameraDlg)->WJOGMode)
                    {
                        MoveXYZW(0, 0, 0, NegWWorkRange);//�����m360
                    }
                    else
                    {
                        MoveXYZW(0, 0, 0, -(((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition(UseNeedleMode).w) + NegWWorkRange);
                    }
                }              
            }
            else if (MoveW == 3)//W-
            {
                if (((CCommandTestDlg*)pMain)->a.m_Action.m_MachineOffSet.x == TCXYOffsetInit && ((CCommandTestDlg*)pMain)->a.m_Action.m_MachineOffSet.y == TCXYOffsetInit || ((CCamera*)((CCommandTestDlg*)pMain)->m_pCameraDlg)->TCOffstAdjust)//�|������D�P�b�ե�
                {
                    if (((CCamera*)((CCommandTestDlg*)pMain)->m_pCameraDlg)->WJOGMode)
                    {
                        MoveXYZW(0, 0, 0, 360);//�����m360
                    }
                    else
                    {
                        MoveXYZW(0, 0, 0, (360 - ((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition(UseMachineMode).w));//��+��{�Z��360��
                    }
                }              
                else
                {
                    if (((CCamera*)((CCommandTestDlg*)pMain)->m_pCameraDlg)->WJOGMode)
                    {
                        MoveXYZW(0, 0, 0, PosWWorkRange);//�����m360
                    }
                    else
                    {
                        MoveXYZW(0, 0, 0, (PosWWorkRange - ((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition(UseNeedleMode).w));//�̤j�ʨ�360��
                    }
                }                   
            }
        }
    }
#endif
    CButton::OnLButtonDown(nFlags, point);
}
/*���s��}*/
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
/*XYZW���s����*/
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
                        {
                            if (((CCamera*)((CCommandTestDlg*)pMain)->m_pCameraDlg)->WJOGMode && MoveW != 0)
                            {
                                ((CCommandTestDlg*)pMain)->a.m_Action.MCO_JogMove(MoveX, MoveY, MoveZ, JOGHSpeedW * 2, JOGHSpeedA * 6, JOGHSpeedI, MoveW, ((CCamera*)((CCommandTestDlg*)pMain)->m_pCameraDlg)->WJOGMode);
                            }
                            else
                            {
                                ((CCommandTestDlg*)pMain)->a.m_Action.MCO_JogMove(MoveX, MoveY, MoveZ, JOGHSpeedW, JOGHSpeedA, JOGHSpeedI, MoveW, ((CCamera*)((CCommandTestDlg*)pMain)->m_pCameraDlg)->WJOGMode);
                            }
                        }                          
                        //MO_Do3DLineMove(MoveX, MoveY, MoveZ, 80000, 1200000, 6000);
                        break;
                    case 2:
                        if (!MO_ReadIsDriving(15))
                        {
                            if (((CCamera*)((CCommandTestDlg*)pMain)->m_pCameraDlg)->WJOGMode && MoveW != 0)
                            {
                                ((CCommandTestDlg*)pMain)->a.m_Action.MCO_JogMove(MoveX, MoveY, MoveZ, JOGMSpeedW * 2, JOGMSpeedA * 6, JOGMSpeedI, MoveW, ((CCamera*)((CCommandTestDlg*)pMain)->m_pCameraDlg)->WJOGMode);
                            }
                            else
                            {
                                ((CCommandTestDlg*)pMain)->a.m_Action.MCO_JogMove(MoveX, MoveY, MoveZ, JOGMSpeedW, JOGMSpeedA, JOGMSpeedI, MoveW, ((CCamera*)((CCommandTestDlg*)pMain)->m_pCameraDlg)->WJOGMode);
                            }
                        }                                                
                        //MO_Do3DLineMove(MoveX, MoveY, MoveZ, 50000, 800000, 5000);
                        break;
                    case 3:
                        if (!MO_ReadIsDriving(15))
                        {
                            if (((CCamera*)((CCommandTestDlg*)pMain)->m_pCameraDlg)->WJOGMode && MoveW != 0)
                            {
                                ((CCommandTestDlg*)pMain)->a.m_Action.MCO_JogMove(MoveX, MoveY, MoveZ, JOGLSpeedW * 2, JOGLSpeedA * 6, JOGLSpeedI, MoveW, ((CCamera*)((CCommandTestDlg*)pMain)->m_pCameraDlg)->WJOGMode);
                            }
                            else
                            {
                                ((CCommandTestDlg*)pMain)->a.m_Action.MCO_JogMove(MoveX, MoveY, MoveZ, JOGLSpeedW, JOGLSpeedA, JOGLSpeedI, MoveW, ((CCamera*)((CCommandTestDlg*)pMain)->m_pCameraDlg)->WJOGMode);
                            }
                        }                            
                        //MO_Do3DLineMove(MoveX, MoveY, MoveZ, 5000, 50000, 1000);
                        break;
                    default:
                        //MessageBox(_T("�{���X�{���~!"));
                        break;
                    }
#ifdef PRINTF
                    //_cwprintf("�i�ӤF:%d", ((CCommandTestDlg*)pMain)->a.RunStatusRead.RunStatus);
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
