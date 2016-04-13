/*
*�ɮצW��:Action
*���e²�z:�B�ʩR�OAPI�A�ԲӰѼƽЬd��excel
*�Iauthor �@�̦W��:R
*�Idata ��s���:2016/04/11	*/
#include "stdafx.h"
#include "Action.h"
/***********************************************************
**                                                        **
**          �B�ʼҲ�-�B�ʫ��O�R�O (�����ʧ@�P�_)             **
**                                                        **
************************************************************/
CAction::CAction()
{     
    g_bIsStop = FALSE;
}
CAction::~CAction()
{
}
/***************************************************************************************************************�B��API*/
//���ݮɶ�
void CAction::WaitTime(HANDLE wakeEvent, int Time)
{
    if (!g_bIsStop)
    {
        DWORD rc = WaitForSingleObject(wakeEvent, Time);
        switch (rc)
        {
        case WAIT_OBJECT_0:
            // wakeEvent signaled
            break;
        case WAIT_TIMEOUT:
            // 10-second timer passed 
            break;
        case WAIT_ABANDONED:
            // main thread ended 
            break;
        }
    }
    //for (int i = 0; i < 10; i++)
    //{
    //    if (!((COrder*)pParam)->StopFlag)
    //    {
    //        DWORD rc = WaitForSingleObject(wakeEvent, 1000);
    //        switch (rc)
    //        {
    //        case WAIT_OBJECT_0:
    //            // wakeEvent signaled
    //            ((COrder*)pParam)->Time = 100;
    //            break;
    //        case WAIT_TIMEOUT:
    //            // 10-second timer passed 
    //            ((COrder*)pParam)->Time++;
    //            break;
    //        case WAIT_ABANDONED:
    //            // main thread ended 
    //            return 0;
    //        }
    //    }
    //}
}
/*���I�I���ʧ@(�h��-�����]�m+�[�t��+�I���I�t��)
*��J(���I�I���B���I�I���]�w�BZ�b�u�@���׳]�w�B�I�������]�w�B�[�t�סB�]�I���I�^�t�סB�t�ιw�]�Ѽ�)*/
void CAction::DecidePointGlue(LONG lX, LONG lY, LONG lZ, LONG lDoTime, LONG lDelayStopTime,
    LONG lZBackDistance, LONG lZdistance, LONG lHighVelocity, LONG lLowVelocity, LONG lWorkVelociy,
    LONG lAcceleration, LONG lInitVelociy)
{
#ifdef MOVE
    /*���I�I��
    LONG lX, LONG lY, LONG lZ
    */
    /*���I�I���]�w
    LONG lDDoTime,LONG lDDelayStopTime
    */
    /*Z�b�u�@���׳]�w-Z�b�^�ɰ���(�۹�)�̰��I
    LONG lDZBackDistance
    */
    /*�I�������]�w(Z�b�۹�Z���A���t�סA�C�t�סA)
    lZdistance
    lHighVelocity
    lLowVelocity
    */
    /*�t�ιw�]�Ѽ�(�X�ʳt�סA�[�t�סA��t��)
    LONG lWorkVelociy,LONG lAcceleration, LONG lInitVelociy
    */
    LONG lNowX = 0, lNowY = 0, lNowZ = 0;
    lNowX = MO_ReadLogicPosition(0);
    lNowY = MO_ReadLogicPosition(1);
    lNowZ = MO_ReadLogicPosition(2);
    if (!g_bIsStop)
    {
        MO_Do3DLineMove(lX - lNowX, lY - lNowY, 0, lWorkVelociy, lAcceleration, lInitVelociy);//���ʨ��I���I
        PreventMoveError();//����b�d�X��
    }
    if (!g_bIsStop)
    {
        MO_Do3DLineMove(0, 0, lZ - lNowZ, lWorkVelociy, lAcceleration, lInitVelociy);//���ʨ��I���I
        PreventMoveError();//����b�d�X��
    }
    if (!g_bIsStop)
    {
        if (lDoTime == 0)
        {
            Sleep(1);
        }
        else
        {
            DoGlue(lDoTime, lDelayStopTime, GummingTimeOutThread);//�����I��
        }
    }
    if (lZBackDistance == 0)
    {
        Sleep(1);
    }
    else
    {
        if (lZdistance == 0)
        {
            if (!g_bIsStop)
            {
                MO_Do3DLineMove(0, 0, (lZ - lZBackDistance) - lZ, lWorkVelociy, lAcceleration, lInitVelociy);//�I�������]�m
                PreventMoveError();//����b�d�X��
            }
        }
        else
        {
            if (lZdistance >  lZBackDistance)
            {
                lZdistance = lZBackDistance;
                MO_Do3DLineMove(0, 0, (lZ - lZBackDistance) - lZ, lHighVelocity, lAcceleration, lInitVelociy);//�I�������]�m
                PreventMoveError();//����b�d�X��
            }
            else
            {
                if (!g_bIsStop)
                {
                    MO_Do3DLineMove(0, 0, (lZ - lZdistance) - lZ, lLowVelocity, lAcceleration, lInitVelociy);//�I�������]�m
                    PreventMoveError();//����b�d�X��
                }
                if (!g_bIsStop)
                {
                    MO_Do3DLineMove(0, 0, (lZ - lZBackDistance) - (lZ - lZdistance), lHighVelocity, lAcceleration, lInitVelociy);//�I�������]�m
                    PreventMoveError();//����b�d�X��
                }
            }

        }
}
#endif
}
/***************************************************************************************************************�ۦ�B�Ψ��*/
/*�X��(�t�X������ϥ�)
*��J(�I�����}��or�I��������)��Ķ�᪺LONG��
*/
void CAction::DoGlue(LONG lTime, LONG lDelayTime, LPTHREAD_START_ROUTINE GummingTimeOutThread)
{
#ifdef MOVE
    if (!g_bIsStop)
    {
        MO_GummingSet((lTime), GummingTimeOutThread);
        PreventGlueError();//����X���X��
    }
    if (!g_bIsStop)
    {
        // lTime / 1000 =(us��ms)   
        MO_Timer(0, 0, lDelayTime * 1000);
        MO_Timer(1, 0, 0);//(ms)
        Sleep(1);//����X���A�קK�p�ɾ��쪽��0
        while (MO_Timer(3, 0, 0))
        {
            Sleep(1);
        }
    }
#endif
}
/*����b�d�X��
*Ū���U�b�X�ʪ��A�A��ʧ@�ɰ���C*/
void CAction::PreventMoveError()
{
#ifdef MOVE
    while (MO_ReadIsDriving(7) && !g_bIsStop)
    {
        Sleep(1);
    }
#endif
}
/*����X���X��
*Ū���X�����A�A��ʧ@�ɰ���C*/
void CAction::PreventGlueError()
{
#ifdef MOVE
    while (MO_ReadGumming() && !g_bIsStop)
    {
        Sleep(1);
    }
#endif
}
/*�����
*���_�ɰ��槹���I���C*/
DWORD CAction::GummingTimeOutThread(LPVOID)
{
#ifdef MOVE
    MO_FinishGumming();
#endif
    return 0;
}




