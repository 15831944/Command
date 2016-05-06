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
    pAction = this;
    g_bIsPause = FALSE;//�Ȱ�
    g_bIsStop = FALSE;//����
    g_bIsDispend = TRUE;//�I�����}
    g_iNumberGluePort = 1;//�ϥΤ@���I����
}
CAction::~CAction()
{
}
/***************************************************************************************************************�B��API*/
//���ݮɶ�(��������ե�)
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
/*
*���I�I���ʧ@(�h��-�����]�m+�[�t��+�I���I�t��)
*��J(���I�I���B���I�I���]�w�BZ�b�u�@���׳]�w�B�I�������]�w�B�[�t�סB�]�I���I�^�t�סB�t�ιw�]�Ѽ�)
*/
void CAction::DecidePointGlue(LONG lX, LONG lY, LONG lZ, LONG lDoTime, LONG lDelayStopTime,
    LONG lZBackDistance,BOOL bZDisType, LONG lZdistance, LONG lHighVelocity, LONG lLowVelocity, LONG lWorkVelociy,
    LONG lAcceleration, LONG lInitVelociy)
{
    /*���I�I��
    LONG lX, LONG lY, LONG lZ
    */
    /*���I�I���]�w
    LONG lDDoTime,LONG lDDelayStopTime
    */
    /*Z�b�u�@���׳]�w-Z�b�^�ɰ���(�۹�)�̰��I
    LONG lDZBackDistance
    BOOL bZDisType(0�����m/1�۹��m)
    */
    /*�I�������]�w(Z�b�۹�Z���A���t�סA�C�t�סA)
    lZdistance
    lHighVelocity
    lLowVelocity
    */
    /*�t�ιw�]�Ѽ�(�X�ʳt�סA�[�t�סA��t��)
    LONG lWorkVelociy,LONG lAcceleration, LONG lInitVelociy
    */
#ifdef MOVE
    LONG lNowX = 0, lNowY = 0, lNowZ = 0;
    lNowX = MO_ReadLogicPosition(0);
    lNowY = MO_ReadLogicPosition(1);
    lNowZ = MO_ReadLogicPosition(2);
    if (!bZDisType)//�����m
    {
        lZBackDistance = lZBackDistance - lZ;
    }
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
/*
*�u�q�}�l�ʧ@
*��J(�u�q�}�l�B�u�q�I���]�w�B�[�t�סB�u�t�סB�t�ιw�]�Ѽ�)
*/
void CAction::DecideLineStartMove(LONG lX, LONG lY, LONG lZ, LONG lStartDelayTime,
    LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
    /*�u�q�}�l(x�y�СAy�y�СAz�y�СA�u�q�_�l�I�A)
    LONG lX, LONG lY, LONG lZ
    */
    /*�u�q�I���]�w(1.���ʫe����A)
    LONG lStartDelayTime
    */
    //1.���ʫe�I�����b�@���u�q�_�l�I�B�O�����}���ɪ��C �����ɥi����w�Y�b�y��y�ʤ��e�u�u�q�o�Ͳ��ʡC
    //2. �I�����}�ҫe�A ���F���}���u�u�q�_�l�I�����ʶZ���C �ӶZ�������F���ѤF�������_�t�ɶ��A�D�n�ΨӮ����L�q�y��b�u�q�_�l�B���n�E�C
    /*�t�ΰѼ�(�X�ʳt�סA�[�t�סA��t��)
    LONG lWorkVelociy,LONG lAcceleration, LONG lInitVelociy
    */
#ifdef MOVE
    if (!g_bIsStop)
    {
        AttachPointMove(lX, lY, 0, lWorkVelociy, lAcceleration, lInitVelociy, 0);//�ϥΪ��ݰ���  x,y�b����
        PreventMoveError();//����b�d�X��
    }
    AttachPointMove(0, 0, lZ, lWorkVelociy, lAcceleration, lInitVelociy, 0);//�ϥΪ��ݰ���(�b) z�b���U��
    PreventMoveError();//����b�d�X��
    if (g_bIsDispend == 1)
    {
        MO_GummingSet(g_iNumberGluePort, 0, GummingTimeOutThread);//�(���d)
    }
    MO_Timer(0, 0, lStartDelayTime * 1000);
    MO_Timer(1, 0, lStartDelayTime * 1000);//�u�q�I���]�w---(1)���ʫe����(�b�u�q�}�l�I�W)
    Sleep(1);//����X���A�קK�p�ɾ��쪽��0
    while (MO_Timer(3, 0, 0))
    {
        if (g_bIsStop)
        {
            break;
        }
        Sleep(1);
    }
    PauseStopGlue();//�Ȱ��ɰ����(g_bIsPause=1)
#endif
}
/*
*�u�q���I�ʧ@
*��J(�u�q���I�B�u�q�I���]�w�B�[�t�סB�u�t�סB�t�ιw�]�Ѽ�)
*/
void CAction::DecideLineMidMove(LONG lX, LONG lY, LONG lZ, LONG lMidDelayTime, LONG lWorkVelociy,
    LONG lAcceleration, LONG lInitVelociy)
{
    /*�u�q���I(x�y�СAy�y�СAz�y�СA�u�q�_�l�I�A)
    LONG lX, LONG lY, LONG lZ
    */
    /*�u�q�I���]�w(�`�I�ɶ�)
    LONG lMidDelayTime
    */
#ifdef MOVE
    AttachPointMove(0, 0, lZ, lWorkVelociy, lAcceleration, lInitVelociy, 0);//�ϥΪ��ݰ���(�b) z�b���U��
    PreventMoveError();//����b�d�X��
    PauseDoGlue();//�Ȱ���_���~��X��(g_bIsPause=0) �X��
    if (!g_bIsStop)
    {
        AttachPointMove(lX, lY, lZ, lWorkVelociy, lAcceleration, lInitVelociy,0);//�ϥΪ��ݰ���
        PreventMoveError();//����b�d�X��
    }
    MO_Timer(0, 0, lMidDelayTime * 1000);
    MO_Timer(1, 0, lMidDelayTime * 1000);//�u�q�I���]�w---(4)�`�I�ɶ�
    Sleep(1);//����X���A�קK�p�ɾ��쪽��0
    while (MO_Timer(3, 0, 0))
    {
        if (g_bIsStop)
        {
            break;
        }
        Sleep(1);
    }
    PauseStopGlue();//�Ȱ��ɰ����(g_bIsPause=1)
#endif
}
/*
*�u�q�����ʧ@
*��J(�u�q�����I�B�u�q�I���]�w�B�[�t�סB�u�t�סBZ�b�u�@���׳]�w�B�I�������]�w�B��^�]�w�B�t�ΰѼ�)
*/
void CAction::DecideLineEndMove(LONG lX, LONG lY, LONG lZ, LONG lCloseOffDelayTime, LONG lCloseDistance,
    LONG lCloseONDelayTime, LONG lZBackDistance, BOOL bZDisType, LONG lHighVelocity, LONG lDistance, LONG lHigh,
    LONG lLowVelocity, int iType, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
    /*�u�q�����I(x�y�СAy�y�СAz�y�СA�u�q�_�l�I�A)
    LONG lX, LONG lY, LONG lZ
    */
    /*�u�q�I���]�w(���d�ɶ��A�����Z���A��������)
    LONG lCloseOffDelayTime ,LONG lCloseDistance ,LONG lCloseONDelayTime
    */
    //3.�I����������A���F�����O�b�w�Y���ܤU�@�I�e�ܱo���ŦӦb�u�q�I�������I�B���ͪ����ɡC
    //5.������L�q�y��b�u�q�����I�B�o�Ͱ�n�A�I�����b�Z���u�q�����I�e�h���B�����C
    //6.�I�����b�u�q�����I�B�����O���}�Ҫ��ɪ��C
    /*��^�]�w(��^���סAz��^���סA��^�t�סA����)
    LONG lDistance ,LONG lHigh ,LONG lLowVelocity,int iType
    */
    /*Z�b�u�@���׳]�w(Z�b�^�ɬ۹�Z��)
    LONG lZBackDistance
    BOOL bZDisType
    */
    /*�I�������]�w(���t��)
    LONG lHighVelocity
    */
#ifdef MOVE
    if (!bZDisType)//�����m
    {
        lZBackDistance = lZBackDistance - lZ;
    }
    LONG lNowX = 0, lNowY = 0, lNowZ = 0;
    LONG lLineClose = 0, lXClose = 0, lYClose = 0;
    PauseDoGlue();//�Ȱ���_���~��X��(g_bIsPause=0)
    lNowX = MO_ReadLogicPosition(0);
    lNowY = MO_ReadLogicPosition(1);
    lNowZ = MO_ReadLogicPosition(2);
    if (lHighVelocity == 0)
    {
        lHighVelocity = lWorkVelociy;
    }
    if (lLowVelocity == 0)
    {
        lLowVelocity = lWorkVelociy;
    }
    if (lX == lNowX && lY == lNowY)//�w�g�b�����I�W���������Z��
    {
        MO_Timer(0, 0, lCloseONDelayTime * 1000);
        MO_Timer(1, 0, lCloseONDelayTime * 1000);//�u�q�I���]�w---(6)�������� 
        Sleep(1);//����X���A�קK�p�ɾ��쪽��0
        while (MO_Timer(3, 0, 0))
        {
            if (g_bIsStop)
            {
                break;
            }
            Sleep(1);
        }
    }
    else
    {
        if (lCloseDistance == 0) //�u�q�I���]�w---(5)�����Z��
        {
            AttachPointMove(lX, lY, lZ, lWorkVelociy, lAcceleration, lInitVelociy,0);//�ϥΪ��ݰ���
            PreventMoveError();//����b�d�X��
            MO_Timer(0, 0, lCloseONDelayTime * 1000);
            MO_Timer(1, 0, lCloseONDelayTime * 1000);//�u�q�I���]�w---(6)��������  
            Sleep(1);//����X���A�קK�p�ɾ��쪽��0
            while (MO_Timer(3, 0, 0))
            {
                if (g_bIsStop == 1)
                {
                    break;
                }
                Sleep(1);
            }
        }
        else
        {
            lLineClose = lCloseDistance;
            LineGetToPoint(lXClose, lYClose, lX, lY, lNowX, lNowY, lLineClose);
            if (!g_bIsStop)
            {
                MO_Do3DLineMove(lXClose, lYClose, 0, lWorkVelociy, lAcceleration, lInitVelociy); //�u�q�I���]�w---(5)�����Z��
                PreventMoveError();//����b�d�X��
            }
        }
    }
    MO_StopGumming();//����X��
    GelatinizeBack(iType, lX, lY, lZ, lNowX, lNowY, lDistance, lHigh, lZBackDistance , lLowVelocity, lHighVelocity, lAcceleration, lInitVelociy);//��^�]�w
    MO_Timer(0, 0, lCloseOffDelayTime * 1000);
    MO_Timer(1, 0, lCloseOffDelayTime * 1000);//�u�q�I���]�w---(3)���d�ɶ� 
    Sleep(1);//����X���A�קK�p�ɾ��쪽��0
    while (MO_Timer(3, 0, 0))
    {
        if (g_bIsStop == 1)
        {
            break;
        }
        Sleep(1);
    }
#endif
}
/*
*(�{�b��m�b�����I�W)�u�q�����ʧ@--�h��
*��J(�u�q�����I�B�u�q�I���]�w�B�[�t�סB�u�t�סBZ�b�u�@���׳]�w�B�I�������]�w�B��^�]�w�B�t�ΰѼ�)
*����Z�b�^�ɫ��A1
*/
void CAction::DecideLineEndMove(LONG lCloseOffDelayTime,
    LONG lCloseONDelayTime, LONG lZBackDistance, BOOL bZDisType, LONG lHighVelocity, LONG lDistance, LONG lHigh,
    LONG lLowVelocity, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
    /*�u�q�I���]�w(���d�ɶ��A�����Z���A��������)
    LONG lCloseOffDelayTime ,LONG lCloseDistance ,LONG lCloseONDelayTime
    */
    //3.�I����������A���F�����O�b�w�Y���ܤU�@�I�e�ܱo���ŦӦb�u�q�I�������I�B���ͪ����ɡC
    //5.������L�q�y��b�u�q�����I�B�o�Ͱ�n�A�I�����b�Z���u�q�����I�e�h���B�����C
    //6.�I�����b�u�q�����I�B�����O���}�Ҫ��ɪ��C
    /*��^�]�w(��^���סAz��^���סA��^�t�סA����)
    LONG lDistance ,LONG lHigh ,LONG lLowVelocity
    *(iType ����1�ϥΨ�q�t�^��)
    */
    /*Z�b�u�@���׳]�w-Z�b�^�ɰ���(�۹�)�̰��I
    LONG lZBackDistance ,BOOL bZDisType(0�����m/1�۹��m)
    */
    /*�I�������]�w(���t��)
    LONG lHighVelocity
    */
#ifdef MOVE
    LONG lNowX = 0, lNowY = 0, lNowZ = 0;
    LONG lLineClose = 0, lXClose = 0, lYClose = 0;
    PauseDoGlue();//�Ȱ���_���~��X��(g_bIsPause=0)
    lNowX = MO_ReadLogicPosition(0);
    lNowY = MO_ReadLogicPosition(1);
    lNowZ = MO_ReadLogicPosition(2);
    if (!bZDisType)//�����m
    {
        lZBackDistance = lZBackDistance - lNowZ;
    }
    if (lHighVelocity == 0)
    {
        lHighVelocity = lWorkVelociy;
    }
    if (lLowVelocity == 0)
    {
        lLowVelocity = lWorkVelociy;
    }
    MO_Timer(0, 0, lCloseONDelayTime * 1000);
    MO_Timer(1, 0, lCloseONDelayTime * 1000);//�u�q�I���]�w---(6)�������� 
    Sleep(1);//����X���A�קK�p�ɾ��쪽��0
    while (MO_Timer(3, 0, 0))
    {
        if (g_bIsStop == 1)
        {
            break;
        }
        Sleep(1);
    }
    MO_StopGumming();//����X��
    GelatinizeBack(1, lNowX, lNowY, lNowZ, lNowX, lNowY, lDistance, lHigh, lZBackDistance , lLowVelocity, lHighVelocity, lAcceleration, lInitVelociy);//��^�]�w
    MO_Timer(0, 0, lCloseOffDelayTime * 1000);
    MO_Timer(1, 0, lCloseOffDelayTime * 1000);//�u�q�I���]�w---(3)���d�ɶ� 
    Sleep(1);//����X���A�קK�p�ɾ��쪽��0
    while (MO_Timer(3, 0, 0))
    {
        if (g_bIsStop == 1)
        {
            break;
        }
        Sleep(1);
    }
#endif
}
/*
*�u�q�}�l���u�_�����I�ʧ@
*��J(�u�q�}�lX,Y,Z,�u�q���IX,Y,Z,���ʫe����,�}���e�q�_�I���ʶZ��(�]�m�Z��),��u�q���I�����d�ɶ�(�`�I�ɶ�ms),�X�ʳt��,�[�t��,��t��)
*/
void CAction::DecideLineSToP(LONG lX, LONG lY, LONG lZ, LONG lX2, LONG lY2, LONG lZ2, LONG lStartDelayTime, LONG lStartDistance, LONG lMidDelayTime, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
    /*�u�q�}�l(x�y�СAy�y�СAz�y�СA�u�q�_�l�I�A)
    LONG lX, LONG lY, LONG lZ
    */
    /*�u�q���I(x�y�СAy�y�СAz�y�СA�u�q�_�l�I�A)
    LONG lX2, LONG lY2, LONG lZ2
    */
    /*�u�q�I���]�w(1.���ʫe����A2.�]�m�Z���A)
    LONG lStartDelayTime ,LONG lStartDistance
    */
    /*�u�q�I���]�w(4�`�I�ɶ�)
    LONG lMidDelayTime
    */
    //1.���ʫe�I�����b�@���u�q�_�l�I�B�O�����}���ɪ��C �����ɥi����w�Y�b�y��y�ʤ��e�u�u�q�o�Ͳ��ʡC
    //2. �I�����}�ҫe�A ���F���}���u�u�q�_�l�I�����ʶZ���C �ӶZ�������F���ѤF�������_�t�ɶ��A�D�n�ΨӮ����L�q�y��b�u�q�_�l�B���n�E�C
    //4.�Ȱw��u�q�����I���R�O�Ӳ��ͪ����ɡC
    /*�t�ΰѼ�(�X�ʳt�סA�[�t�סA��t��)
    LONG lWorkVelociy,LONG lAcceleration, LONG lInitVelociy
    */
#ifdef MOVE
    LONG lNowX = 0, lNowY = 0;
    LONG lXClose = 0, lYClose = 0, lLineClose = 0;
    LONG lBuffX = 0, lBuffY = 0;
    DOUBLE dTime = 0;
    int iBuf = 0;
    dTime = (sqrt((DOUBLE)lInitVelociy*(DOUBLE)lInitVelociy + ((DOUBLE)lAcceleration*(DOUBLE)lStartDistance / 2)) - (DOUBLE)lInitVelociy) / (DOUBLE)lAcceleration;
    lNowX = MO_ReadLogicPosition(0);
    lNowY = MO_ReadLogicPosition(1);
    lLineClose = lStartDistance;
    LineGetToPoint(lXClose, lYClose, lX2, lY2, lX, lY, lLineClose);
    lBuffX = (-(lXClose - lX)) + lX;
    lBuffY = (-(lYClose - lY)) + lY;
    iBuf = lStartDelayTime ^ lStartDistance;//�����β��ʫe�����]�m�Z�� �p�G��̳��ۦP���G��0(���̳����ȮɥH"���ʫe����"�u��)
    if (iBuf == 0 || lStartDistance == 0)
    {
        AttachPointMove(lX, lY, 0, lWorkVelociy, lAcceleration, lInitVelociy, 0);//�ϥΪ��ݰ���(���ʨ�_�l�I)
        PreventMoveError();//����b�d�X��
        AttachPointMove(0, 0, lZ, lWorkVelociy, lAcceleration, lInitVelociy, 0);//�ϥΪ��ݰ��� (z�b���U��)
        PreventMoveError();//����b�d�X��
        if (g_bIsDispend == 1)
        {
            MO_GummingSet(g_iNumberGluePort, 0, GummingTimeOutThread);//�(���d)
        }
        MO_Timer(0, 0, lStartDelayTime * 1000);
        MO_Timer(1, 0, lStartDelayTime * 1000);//�u�q�I���]�w---(1)���ʫe����(�b�u�q�}�l�I�W)
        Sleep(1);//����X���A�קK�p�ɾ��쪽��0
        while (MO_Timer(3, 0, 0))
        {
            if (g_bIsStop)
            {
                break;
            }
            Sleep(1);
        }
        AttachPointMove(lX2, lY2, 0, lWorkVelociy, lAcceleration, lInitVelociy, 0);//�ϥΪ��ݰ���(���ʨ��I2)
        PreventMoveError();//����b�d�X��
    }
    else
    {
        AttachPointMove(lBuffX, lBuffY, 0, lWorkVelociy, lAcceleration, lInitVelociy, 0);//�ϥΪ��ݰ���
        PreventMoveError();//����b�d�X��
        AttachPointMove(0, 0, lZ, lWorkVelociy, lAcceleration, lInitVelociy, 0);//�ϥΪ��ݰ��� (z�b���U��)
        PreventMoveError();//����b�d�X��
        MO_TimerSetIntter(dTime * 1000000, LPInterrupt);//�p�ɨ���ܰ����
        AttachPointMove(lX2, lY2, 0, lWorkVelociy, lAcceleration, lInitVelociy, 0);//�ϥΪ��ݰ���(���ʨ��I2�ϥΰ�������_) 
        PreventMoveError();//����b�d�X��
    }
    MO_Timer(0, 0, lMidDelayTime * 1000);
    MO_Timer(1, 0, lMidDelayTime * 1000);//�u�q�I���]�w---(4)�`�I�ɶ�
    Sleep(1);//����X���A�קK�p�ɾ��쪽��0
    while (MO_Timer(3, 0, 0))
    {
        if (g_bIsStop == 1)
        {
            break;
        }
        Sleep(1);
    }
    PauseStopGlue();//�Ȱ��ɰ����(g_bIsPause=1)
#endif
}
/*
*�u�q�}�l���u�q�����ʧ@
*��J(�u�q�}�lX,Y,Z,�����IX,Y,Z,���ʫe����,�}���e�q�_�I���ʶZ��(�]�m�Z��),������b�����I���d�ɶ�(���d�ɶ�),�Z�������I�h���Z������(�����Z��),�����᪺����ɶ�(��������),Z�b�^�ɶZ��,Z�b�^�ɫ��A,�I�������]�w(���t��),��^����,z��^����,��^�t��(�C�t),��^����,�X�ʳt��,�[�t��,��t��)
*/
void CAction::DecideLineSToE(LONG lX, LONG lY, LONG lZ, LONG lX2, LONG lY2, LONG lZ2, LONG lStartDelayTime, LONG lStartDistance, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime, LONG lZBackDistance,BOOL bZDisType, LONG lHighVelocity, LONG lDistance, LONG lHigh, LONG lLowVelocity, int iType, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
    /*�u�q�}�l(x�y�СAy�y�СAz�y�СA�u�q�_�l�I�A)
    LONG lX, LONG lY, LONG lZ
    */
    /*�u�q�����I(x�y�СAy�y�СAz�y�СA�u�q�_�l�I�A)
    LONG lX2, LONG lY2, LONG lZ2
    /*��^�]�w(��^���סAz��^���סA��^�t�סA�����A���t��)
    LONG lDistance ,LONG lHigh ,LONG lLowVelocity,int iType,lHighVelocity
    */
    /*�u�q�I���]�w(1.���ʫe����A2.�]�m�Z���A3���d�ɶ��A4�����Z���A5��������)
    LONG lStartDelayTime ,LONG lStartDistance ,LONG lCloseOffDelayTime ,LONG lCloseDistance ,LONG lCloseONDelayTime
    */
    //1.���ʫe�I�����b�@���u�q�_�l�I�B�O�����}���ɪ��C �����ɥi����w�Y�b�y��y�ʤ��e�u�u�q�o�Ͳ��ʡC
    //2. �I�����}�ҫe�A ���F���}���u�u�q�_�l�I�����ʶZ���C �ӶZ�������F���ѤF�������_�t�ɶ��A�D�n�ΨӮ����L�q�y��b�u�q�_�l�B���n�E�C
    //3.�I����������A���F�����O�b�w�Y���ܤU�@�I�e�ܱo���ŦӦb�u�q�I�������I�B���ͪ����ɡC
    //5.������L�q�y��b�u�q�����I�B�o�Ͱ�n�A�I�����b�Z���u�q�����I�e�h���B�����C
    //6.�I�����b�u�q�����I�B�����O���}�Ҫ��ɪ��C
    /*�t�ΰѼ�(�X�ʳt�סA�[�t�סA��t��)
    LONG lWorkVelociy,LONG lAcceleration, LONG lInitVelociy
    */
#ifdef MOVE
    LONG lNowX = 0, lNowY = 0 ,lNowZ = 0;
    LONG lXClose = 0, lYClose = 0, lLineClose = 0;
    LONG lBuffX = 0, lBuffY = 0;
    DOUBLE dTime = 0;
    int iBuf = 0;
    dTime = (sqrt((DOUBLE)lInitVelociy*(DOUBLE)lInitVelociy + ((DOUBLE)lAcceleration*(DOUBLE)lStartDistance / 2)) - (DOUBLE)lInitVelociy) / (DOUBLE)lAcceleration;
    lNowX = MO_ReadLogicPosition(0);
    lNowY = MO_ReadLogicPosition(1);
    lNowZ = MO_ReadLogicPosition(2);
    lLineClose = lStartDistance;
    LineGetToPoint(lXClose, lYClose, lX2, lY2, lX, lY, lLineClose);
    lBuffX = (-(lXClose - lX)) + lX;
    lBuffY = (-(lYClose - lY)) + lY;
    if (!bZDisType)//�����m
    {
        lZBackDistance = lZBackDistance - lNowZ;
    }
    if (lHighVelocity == 0)
    {
        lHighVelocity = lWorkVelociy;
    }
    if (lLowVelocity == 0)
    {
        lLowVelocity = lWorkVelociy;
    }
    iBuf = lStartDelayTime ^ lStartDistance;//�����β��ʫe�����]�m�Z�� �p�G��̳��ۦP���G��0(���̳����ȮɥH"���ʫe����"�u��)
    if (iBuf == 0 || lStartDistance == 0)
    {
        AttachPointMove(lX, lY, 0, lWorkVelociy, lAcceleration, lInitVelociy, 0);//�ϥΪ��ݰ���(���ʨ�_�l�I)
        PreventMoveError();//����b�d�X��
        AttachPointMove(0, 0, lZ, lWorkVelociy, lAcceleration, lInitVelociy, 0);//�ϥΪ��ݰ��� (z�b���U��)
        PreventMoveError();//����b�d�X��
        if (g_bIsDispend == 1)
        {
            MO_GummingSet(g_iNumberGluePort, 0, GummingTimeOutThread);//�(���d)
        }
        MO_Timer(0, 0, lStartDelayTime * 1000);
        MO_Timer(1, 0, lStartDelayTime * 1000);//�u�q�I���]�w---(1)���ʫe����(�b�u�q�}�l�I�W)
        Sleep(1);//����X���A�קK�p�ɾ��쪽��0
        while (MO_Timer(3, 0, 0))
        {
            if (g_bIsStop)
            {
                break;
            }
            Sleep(1);
        }
        if (lCloseDistance == 0)//�u�q�I���]�w---(5)�����Z��
        {
            AttachPointMove(lX2, lY2, 0, lWorkVelociy, lAcceleration, lInitVelociy, 0);//�ϥΪ��ݰ���(���ʨ��I2)
            PreventMoveError();//����b�d�X��
        }
        else
        {
            lLineClose = lCloseDistance;
            LineGetToPoint(lXClose, lYClose, lX, lY, lX2, lX2, lLineClose);
            if (!g_bIsStop)
            {
                MO_Do3DLineMove(lXClose, lYClose, 0, lWorkVelociy, lAcceleration, lInitVelociy); //�u�q�I���]�w---(5)�����Z��
                PreventMoveError();//����b�d�X��
            }
        }
    }
    else
    {
        AttachPointMove(lBuffX, lBuffY, 0, lWorkVelociy, lAcceleration, lInitVelociy, 0);//�ϥΪ��ݰ���
        PreventMoveError();//����b�d�X��
        AttachPointMove(0, 0, lZ, lWorkVelociy, lAcceleration, lInitVelociy, 0);//�ϥΪ��ݰ��� (z�b���U��)
        PreventMoveError();//����b�d�X��
        if (lCloseDistance == 0)//�u�q�I���]�w---(5)�����Z��
        {
            MO_TimerSetIntter(dTime * 1000000, LPInterrupt);//�p�ɨ���ܰ����
            AttachPointMove(lX2, lY2, 0, lWorkVelociy, lAcceleration, lInitVelociy, 0);//�ϥΪ��ݰ���(���ʨ��I2�ϥΰ�������_) 
            PreventMoveError();//����b�d�X��
        }
        else
        {
            lLineClose = lCloseDistance;
            LineGetToPoint(lXClose, lYClose, lX, lY, lX2, lX2, lLineClose);
            if (!g_bIsStop)
            {
                MO_Do3DLineMove(lXClose, lYClose, 0, lWorkVelociy, lAcceleration, lInitVelociy); //�u�q�I���]�w---(5)�����Z��
                PreventMoveError();//����b�d�X��
            }
        }

    }
    MO_Timer(0, 0, lCloseONDelayTime * 1000);
    MO_Timer(1, 0, lCloseONDelayTime * 1000);//�u�q�I���]�w---(6)��������
    Sleep(1);//����X���A�קK�p�ɾ��쪽��0
    while (MO_Timer(3, 0, 0))
    {
        if (g_bIsStop == 1)
        {
            break;
        }
        Sleep(1);
    }
    MO_StopGumming();//����X��
    GelatinizeBack(iType, lX2, lY2, lZ2, lNowX, lNowY, lDistance, lHigh, lZBackDistance, lLowVelocity, lHighVelocity, lAcceleration, lInitVelociy);//��^�]�w
    MO_Timer(0, 0, lCloseOffDelayTime * 1000);
    MO_Timer(1, 0, lCloseOffDelayTime * 1000);//�u�q�I���]�w---(3)���d�ɶ� 
    Sleep(1);//����X���A�קK�p�ɾ��쪽��0
    while (MO_Timer(3, 0, 0))
    {
        if (g_bIsStop == 1)
        {
            break;
        }
        Sleep(1);
    }
#endif
}
/*
*��ΰʧ@
*��J(��ΡB��ε����I�B�u�t�B�t�ΰѼ�)
*/
void CAction::DecideCircle(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lWorkVelociy, LONG lInitVelociy)
{
    /*���(x�y�СAy�y�СA)
    LONG lX1, LONG lY1
    */
    /*��ε����I(x�y�СAy�y�СA)
    LONG lX2, LONG lY2
    */
    /*�t�ΰѼ�(�X�ʳt�סA�[�t�סA��t��)
    LONG lWorkVelociy, LONG lInitVelociy
    */
#ifdef MOVE
    LONG lSpeed = 0, lInit = 0, lSSpeed = 0;
    LONG lCircleX = 0, lCircleY = 0;
    LONG lNowX = 0, lNowY = 0;
    BOOL bRev = 0;
    CString csX = 0, csY = 0;
    CString csBuff = 0, csNowPonit = 0, csLineCircle = 0, csLineCircleEnd = 0;
    PauseDoGlue();//�Ȱ���_���~��X��(g_bIsPause=0)
    lNowX = MO_ReadLogicPosition(0);
    lNowY = MO_ReadLogicPosition(1);
    csX.Format(L"%ld", lNowX);
    csY.Format(L"%ld", lNowY);
    csNowPonit = csX + _T("�A") + csY + _T("�A");
    csX.Format(L"%ld", lX1);
    csY.Format(L"%ld", lY1);
    csLineCircle = csX + _T("�A") + csY + _T("�A");
    csX.Format(L"%ld", lX2);
    csY.Format(L"%ld", lY2);
    csLineCircleEnd = csX + _T("�A") + csY + _T("�A");
    csBuff = TRoundCCalculation(csNowPonit, csLineCircleEnd, csLineCircle);
    lCircleX = CStringToLong(csBuff, 0);
    lCircleY = CStringToLong(csBuff, 1);
    bRev = CStringToLong(csBuff, 2);//���o���(X�AY�ARev�A)
    if (!g_bIsStop)
    {
        MO_Do2DArcMove(0, 0, lCircleX - lNowX, lCircleY - lNowY, lInitVelociy, lWorkVelociy, bRev);
        PreventMoveError();//����b�d�X��
    }
    PauseStopGlue();//�Ȱ��ɰ����(g_bIsPause=1)
#endif
}
/*
*�꩷�ʧ@
*��J(�꩷�B�꩷�����I�B�u�t�B�t�ΰѼ�)
*/
void CAction::DecideArc(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lWorkVelociy, LONG lInitVelociy)
{
    /*�꩷(x�y�СAy�y�СA)
    LONG lX1, LONG lY1
    */
    /*�꩷�����I(x�y�СAy�y�СA)
    LONG lX2, LONG lY2
    */
    /*�t�ΰѼ�(�X�ʳt�סA�[�t�סA��t��)
    LONG lWorkVelociy, LONG lInitVelociy
    */
#ifdef MOVE
    LONG lSpeed = 0, lInit = 0, lSSpeed = 0;
    LONG lCircleX = 0, lCircleY = 0;
    LONG lNowX = 0, lNowY = 0;
    BOOL bRev = 0;
    CString csX = 0, csY = 0;
    CString csBuff = 0, csNowPonit = 0, csLineCircle = 0, csLineCircleEnd = 0;
    PauseDoGlue();//�Ȱ���_���~��X��(g_bIsPause=0)
    lNowX = MO_ReadLogicPosition(0);
    lNowY = MO_ReadLogicPosition(1);
    csX.Format(L"%ld", lNowX);
    csY.Format(L"%ld", lNowY);
    csNowPonit = csX + _T("�A") + csY + _T("�A");
    csX.Format(L"%ld", lX1);
    csY.Format(L"%ld", lY1);
    csLineCircle = csX + _T("�A") + csY + _T("�A");
    csX.Format(L"%ld", lX2);
    csY.Format(L"%ld", lY2);
    csLineCircleEnd = csX + _T("�A") + csY + _T("�A");
    csBuff = TRoundCCalculation(csNowPonit, csLineCircleEnd, csLineCircle);
    lCircleX = CStringToLong(csBuff, 0);
    lCircleY = CStringToLong(csBuff, 1);
    bRev = CStringToLong(csBuff, 2);//���o���(X�AY�ARev�A)
    if (!g_bIsStop)
    {
        MO_Do2DArcMove(lX2 - lNowX, lY2 - lNowY, lCircleX - lNowX, lCircleY - lNowY, lInitVelociy, lWorkVelociy, bRev);
        PreventMoveError();//����b�d�X��
    }
    PauseStopGlue();//�Ȱ��ɰ����(g_bIsPause=1)
#endif
}
/*
*��쵲���I
*(��J�Ѽ�:���1�A���2�A�u�q�����I�A�u�q�I���]�w�A��^�]�w�AZ�b�u�@���׳]�w�A�I�������]�w�A�t�ΰѼ�)
*���e:��J�������I�|�P�_�O�_�P�{�b��m�ۦP�A���X�۹������ʧ@
*/
void CAction::DecideCircleToEnd(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lX3, LONG lY3, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime, LONG lZBackDistance, BOOL bZDisType, LONG lHighVelocity, LONG lDistance, LONG lHigh, LONG lLowVelocity, int iType, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
    /*���(x�y�СAy�y�СA)
    LONG lX1, LONG lY1
    */
    /*��ε����I(x�y�СAy�y�СA)�u�q�����I(x�y�СAy�y�СAz�y�СA�u�q�_�l�I�A)
    LONG lX2, LONG lY2
    */
    /*�u�q�����I(x�y�СAy�y�СAz�y�СA�u�q�_�l�I�A)
    LONG lX3, LONG lY3
    */
    /*�u�q�I���]�w(���d�ɶ��A�����Z���A��������)
    LONG lCloseOffDelayTime ,LONG lCloseDistance ,LONG lCloseONDelayTime
    //3.�I����������A���F�����O�b�w�Y���ܤU�@�I�e�ܱo���ŦӦb�u�q�I�������I�B���ͪ����ɡC
    //5.������L�q�y��b�u�q�����I�B�o�Ͱ�n�A�I�����b�Z���u�q�����I�e�h���B�����C
    //6.�I�����b�u�q�����I�B�����O���}�Ҫ��ɪ��C
    */
    /*��^�]�w(��^���סAz��^���סA��^�t�סA����)
    LONG lDistance ,LONG lHigh ,LONG lLowVelocity,int iType
    */
    /*Z�b�u�@���׳]�w-Z�b�^�ɰ���(�۹�)�̰��I
    LONG lZBackDistance ,BOOL bZDisType(0�����m/1�۹��m)
    */
    /*�I�������]�w(���t��)
    LONG lHighVelocity
    */
    /*�t�ΰѼ�(�X�ʳt�סA�[�t�סA��t��)
    LONG lWorkVelociy,LONG lAcceleration, LONG lInitVelociy
    */
#ifdef MOVE
    LONG lCircleX = 0, lCircleY = 0;
    LONG lNowX = 0, lNowY = 0, lNowZ = 0;
    BOOL bRev = 0;
    CString csX = 0, csY = 0;
    CString csBuff = 0, csNowPonit = 0, csLineCircle = 0, csLineCircleEnd = 0;
    LONG lLineClose = 0, lXClose = 0, lYClose = 0;
    LONG lR = 0;//�_���I�P�b�|
    LONG lEndX = 0, lEndY = 0;
    lNowX = MO_ReadLogicPosition(0);
    lNowY = MO_ReadLogicPosition(1);
    lNowZ = MO_ReadLogicPosition(2);
    csX.Format(L"%ld", lNowX);
    csY.Format(L"%ld", lNowY);
    csNowPonit = csX + _T("�A") + csY + _T("�A");
    csX.Format(L"%ld", lX1);
    csY.Format(L"%ld", lY1);
    csLineCircle = csX + _T("�A") + csY + _T("�A");
    csX.Format(L"%ld", lX2);
    csY.Format(L"%ld", lY2);
    csLineCircleEnd = csX + _T("�A") + csY + _T("�A");
    csBuff = TRoundCCalculation(csNowPonit, csLineCircleEnd, csLineCircle);
    lCircleX = CStringToLong(csBuff, 0);
    lCircleY = CStringToLong(csBuff, 1);
    bRev = CStringToLong(csBuff, 2);//���o���(X�AY�ARev�A)
    lR = sqrt(pow(lNowX - lCircleX, 2) + pow(lNowY - lCircleY, 2));//�b�|
    if (lHighVelocity == 0)
    {
        lHighVelocity = lWorkVelociy;
    }
    if (lLowVelocity == 0)
    {
        lLowVelocity = lWorkVelociy;
    }
    if (!bZDisType)//�����m
    {
        lZBackDistance = lZBackDistance - lNowZ;
    }
    PauseDoGlue();//�Ȱ���_���~��X��(g_bIsPause=0)
    if (lNowX == lX3 && lNowY == lY3)//��ܵ����I�b�_�l�I�W
    {
        if (lCloseDistance == 0) //�u�q�I���]�w---(5)�����Z��
        {
            if (!g_bIsStop)
            {
                MO_Do2DArcMove(0, 0, lCircleX - lNowX, lCircleY - lNowY, lInitVelociy, lWorkVelociy, bRev);//��
                PreventMoveError();//����b�d�X��
            }
            MO_Timer(0, 0, lCloseONDelayTime * 1000);
            MO_Timer(1, 0, lCloseONDelayTime * 1000);//�u�q�I���]�w---(6)��������
            Sleep(1);//����X���A�קK�p�ɾ��쪽��0
            while (MO_Timer(3, 0, 0))
            {
                if (g_bIsStop == 1)
                {
                    break;
                }
                Sleep(1);
            }
        }
        else
        {
            ArcGetToPoint(lEndX, lEndY, lCloseDistance, lX3, lY3, lCircleX, lCircleY, lR, !bRev);//��X�_���I
            if (!g_bIsStop)
            {
                MO_Do2DArcMove(lEndX - lNowX, lEndY - lNowY, lCircleX - lNowX, lCircleY - lNowY, lInitVelociy, lWorkVelociy, bRev);//���_��
                PreventMoveError();//����b�d�X��
            }
            MO_StopGumming();//�_��
            if (!g_bIsStop)
            {
                MO_Do2DArcMove(lX3 - lEndX, lY3 - lEndY, lCircleX - lEndX, lCircleY - lEndY, lInitVelociy, lWorkVelociy, bRev);//���_��
                PreventMoveError();//����b�d�X��
            }
        }
        MO_StopGumming();//����X��
                         //��^�]�w���Ѧ��I(�����I�Olx3,ly3,�X�o�Ѧ��I�ϥζ�2)
        GelatinizeBack(iType, lX3, lY3, lNowZ, lX2, lY2, lDistance, lHigh, lZBackDistance, lLowVelocity, lHighVelocity, lAcceleration, lInitVelociy);//��^�]�w
        MO_Timer(0, 0, lCloseOffDelayTime * 1000);
        MO_Timer(1, 0, lCloseOffDelayTime * 1000);//�u�q�I���]�w---(3)���d�ɶ� 
        Sleep(1);//����X���A�קK�p�ɾ��쪽��0
        while (MO_Timer(3, 0, 0))
        {
            if (g_bIsStop == 1)
            {
                break;
            }
        }
    }
    else//��ܵ����I���b��W
    {
        if (!g_bIsStop)
        {
            MO_Do2DArcMove(0, 0, lCircleX - lNowX, lCircleY - lNowY, lInitVelociy, lWorkVelociy, bRev);//��
            PreventMoveError();//����b�d�X��
        }
        DecideLineEndMove(lX3, lY3, lNowZ, lCloseOffDelayTime, lCloseDistance, lCloseONDelayTime, lZBackDistance, bZDisType, lHighVelocity, lDistance, lHigh, lLowVelocity, iType, lWorkVelociy, lAcceleration, lInitVelociy);
        PreventMoveError();//����b�d�X��
    }
#endif
}
/*
*�꩷�쵲���I
*(��J�Ѽ�:�꩷�A�u�q�����I�A�u�q�I���]�w�A��^�]�w�AZ�b�u�@���׳]�w�A�I�������]�w�A�t�ΰѼ�)
*���e:��J���꩷�O�s�������I�A���X�۹������ʧ@
*/
void CAction::DecideArcleToEnd(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime, LONG lZBackDistance, BOOL bZDisType, LONG lHighVelocity, LONG lDistance, LONG lHigh, LONG lLowVelocity, int iType, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
    /*�꩷(x�y�СAy�y�СA)
    LONG lX1, LONG lY1
    */
    /*�u�q�����I(x�y�СAy�y�СAz�y�СA�u�q�_�l�I�A)
    LONG lX2, LONG lY2
    */
    /*�u�q�I���]�w(���d�ɶ��A�����Z���A��������)
    LONG lCloseOffDelayTime ,LONG lCloseDistance ,LONG lCloseONDelayTime
    //3.�I����������A���F�����O�b�w�Y���ܤU�@�I�e�ܱo���ŦӦb�u�q�I�������I�B���ͪ����ɡC
    //5.������L�q�y��b�u�q�����I�B�o�Ͱ�n�A�I�����b�Z���u�q�����I�e�h���B�����C
    //6.�I�����b�u�q�����I�B�����O���}�Ҫ��ɪ��C
    */
    /*��^�]�w(��^���סAz��^���סA��^�t�סA����)
    LONG lDistance ,LONG lHigh ,LONG lLowVelocity,int iType
    */
    /*Z�b�u�@���׳]�w-Z�b�^�ɰ���(�۹�)�̰��I
    LONG lZBackDistance ,BOOL bZDisType(0�����m/1�۹��m)
    */
    /*�I�������]�w(���t��)
    LONG lHighVelocity
    */
    /*�t�ΰѼ�(�X�ʳt�סA�[�t�סA��t��)
    LONG lWorkVelociy,LONG lAcceleration, LONG lInitVelociy
    */
#ifdef MOVE
    LONG lCircleX = 0, lCircleY = 0;
    LONG lNowX = 0, lNowY = 0, lNowZ = 0;
    BOOL bRev = 0;
    CString csX = 0, csY = 0;
    CString csBuff = 0, csNowPonit = 0, csLineCircle = 0, csLineCircleEnd = 0;
    LONG lLineClose = 0, lXClose = 0, lYClose = 0;
    LONG lR = 0;//�_���I�P�b�|
    LONG lEndX = 0, lEndY = 0, lCrev = 0;
    DOUBLE dAngl = 0;
    lNowX = MO_ReadLogicPosition(0);
    lNowY = MO_ReadLogicPosition(1);
    lNowZ = MO_ReadLogicPosition(2);
    csX.Format(L"%ld", lNowX);
    csY.Format(L"%ld", lNowY);
    csNowPonit = csX + _T("�A") + csY + _T("�A");
    csX.Format(L"%ld", lX1);
    csY.Format(L"%ld", lY1);
    csLineCircle = csX + _T("�A") + csY + _T("�A");
    csX.Format(L"%ld", lX2);
    csY.Format(L"%ld", lY2);
    csLineCircleEnd = csX + _T("�A") + csY + _T("�A");
    csBuff = TRoundCCalculation(csNowPonit, csLineCircleEnd, csLineCircle);
    lCircleX = CStringToLong(csBuff, 0);
    lCircleY = CStringToLong(csBuff, 1);
    bRev = CStringToLong(csBuff, 2);//���o���(X�AY�ARev�A)
    lR = sqrt(pow(lNowX - lCircleX, 2) + pow(lNowY - lCircleY, 2));//�b�|
    dAngl = AngleCount(lCircleX, lCircleY, lX1, lY1, lX2, lY2, bRev);//����
    lCrev = 2 * lR * M_PI * dAngl / 360;
    if (lHighVelocity == 0)
    {
        lHighVelocity = lWorkVelociy;
    }
    if (lLowVelocity == 0)
    {
        lLowVelocity = lWorkVelociy;
    }
    if (!bZDisType)//�����m
    {
        lZBackDistance = lZBackDistance - lNowZ;
    }

    PauseDoGlue();//�Ȱ���_���~��X��(g_bIsPause=0)
    if (lCloseDistance == 0) //�u�q�I���]�w---(5)�����Z��
    {
        if (!g_bIsStop)
        {
            MO_Do2DArcMove(lX2 - lNowX, lY2 - lNowY, lCircleX - lNowX, lCircleY - lNowY, lInitVelociy, lWorkVelociy, bRev);//��
            PreventMoveError();//����b�d�X��
        }
        MO_Timer(0, 0, lCloseONDelayTime * 1000);
        MO_Timer(1, 0, lCloseONDelayTime * 1000);//�u�q�I���]�w---(6)��������
        Sleep(1);//����X���A�קK�p�ɾ��쪽��0
        while (MO_Timer(3, 0, 0))
        {
            if (g_bIsStop == 1)
            {
                break;
            }
            Sleep(1);
        }
    }
    else
    {
        ArcGetToPoint(lEndX, lEndY, lCloseDistance, lX2, lY2, lCircleX, lCircleY, lR, !bRev);//��X�_���I

        if (lCrev > lCloseDistance)
        {
            if (!g_bIsStop)
            {
                MO_Do2DArcMove(lEndX - lNowX, lEndY - lNowY, lCircleX - lNowX, lCircleY - lNowY, lInitVelociy, lWorkVelociy, bRev);//���_��
                PreventMoveError();//����b�d�X��
            }
            MO_StopGumming();//�_��
            if (!g_bIsStop)
            {
                MO_Do2DArcMove(lX2 - lEndX, lY2 - lEndY, lCircleX - lEndX, lCircleY - lEndY, lInitVelociy, lWorkVelociy, bRev);//���_��
                PreventMoveError();//����b�d�X��
            }
        }
        else
        {
            AfxMessageBox(L"�_���Z���L�j");
        }
    }
    MO_StopGumming();//����X��
                     //��^�]�w���Ѧ��I(�����I�Olx2,ly2,�X�o�Ѧ��I�ϥζ꩷���I)
    GelatinizeBack(iType, lX2, lY2, lNowZ, lX1, lY1, lDistance, lHigh, lZBackDistance, lLowVelocity, lHighVelocity, lAcceleration, lInitVelociy);//��^�]�w
    MO_Timer(0, 0, lCloseOffDelayTime * 1000);
    MO_Timer(1, 0, lCloseOffDelayTime * 1000);//�u�q�I���]�w---(3)���d�ɶ� 
    Sleep(1);//����X���A�קK�p�ɾ��쪽��0
    while (MO_Timer(3, 0, 0))
    {
        if (g_bIsStop == 1)
        {
            break;
        }
    }
#endif
}
/*
*�����I�ʧ@
*��J(�����I�B�t�ΰѼ�)
*/
void CAction::DecideVirtualPoint(LONG lX, LONG lY, LONG lZ, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
    /*�����I(x�y�СAy�y�СAz�y�СA)
    LONG lX, LONG lY, LONG lZ
    */
    /*�t�ΰѼ�(�X�ʳt�סA�[�t�סA��t��)
    LONG lWorkVelociy,LONG lAcceleration, LONG lInitVelociy
    */
#ifdef MOVE
    AttachPointMove(lX, lY, lZ, lWorkVelociy, lAcceleration, lInitVelociy ,0);//�ϥΪ��ݰ���
    PreventMoveError();//����b�d�X��
#endif
}
/*
*�����I�ʧ@
*��J(�����I�B�t�ΰѼ�)
*/
void CAction::DecideWaitPoint(LONG lX, LONG lY, LONG lZ, LONG lWaitTime, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
    /*�����I(x�y�СAy�y�СAz�y�СA���ݮɶ��A)
    LONG lX, LONG lY, LONG lZ ,LONG lWaitTime
    */
    /*�t�ΰѼ�(�X�ʳt�סA�[�t�סA��t��)
    LONG lWorkVelociy,LONG lAcceleration, LONG lInitVelociy
    */
#ifdef MOVE
    AttachPointMove(lX, lY, lZ, lWorkVelociy, lAcceleration, lInitVelociy ,0);//�ϥΪ��ݰ���
    PreventMoveError();//����b�d�X��
    MO_Timer(0, 0, lWaitTime * 1000);
    MO_Timer(1, 0, lWaitTime * 1000);//���ݮɶ�(us��ms)
    Sleep(1);//����X���A�קK�p�ɾ��쪽��0
    while (MO_Timer(3, 0, 0))
    {
        if (g_bIsStop == 1)
        {
            break;
        }
        Sleep(1);
    }
#endif
}
/*
*���n�I�ʧ@
*��J(���n�I�B�t�ΰѼ�)
*/
void CAction::DecideParkPoint(LONG lX, LONG lY, LONG lZ, LONG lTimeGlue, LONG lWaitTime, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
    /*���n�I(x�y�СAy�y�СAz�y�СA�ƽ��ɶ��A�����ᵥ�ݮɶ��A)
    LONG lX, LONG lY, LONG lZ,LONG lTimeGlue,LONG lWaitTime
    */
    /*�t�ΰѼ�(�X�ʳt�סA�[�t�סA��t��)
    LONG lWorkVelociy,LONG lAcceleration, LONG lInitVelociy
    */
#ifdef MOVE
    AttachPointMove(lX, lY, lZ, lWorkVelociy, lAcceleration, lInitVelociy, 0);//�ϥΪ��ݰ���
    PreventMoveError();//����b�d�X��
    if (lTimeGlue == 0)
    {
        Sleep(1);
    }
    else
    {
        DoGlue(lTimeGlue, lWaitTime, GummingTimeOutThread);//����ƽ�
    }
#endif
}
/*
*���I���k�ʧ@
*��J(LONG�t��1�ALONG�t��2�ALONG���w�b(0~7)�ALONG�����q)
*/
void CAction::DecideInitializationMachine(LONG lSpeed1, LONG lSpeed2, LONG lAxis, LONG lMove)
{
#ifdef MOVE
    MO_SetHardLim(7, 1);
    if (!g_bIsStop)
    {
        MO_MoveToHome(lSpeed1, lSpeed2, lAxis, lMove);
        PreventMoveError();//����b�d�X��
    }
#endif
}
/*
*��R�ʧ@
*��J(�u�q�}�l�A�u�q�����AZ�b�u�@���׳]�w�A��R�R�O�A�t�ΰѼ�)
*/
void CAction::DecideFill(LONG lX1, LONG lY1, LONG lZ1, LONG lX2, LONG lY2, LONG lZ2, LONG lZBackDistance, BOOL bZDisType, int iType, LONG lWidth, LONG lWidth2, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
    /*�u�q�}�l(x�y�СAy�y�СAz�y�СA�u�q�_�l�I�A)
    LONG lX1, LONG lY1, LONG lZ1
    */
    /*�u�q����(x�y�СAy�y�СAz�y�СA�u�q�����I�A)
    LONG lX2, LONG lY2, LONG lZ2
    */
    /*Z�b�u�@���׳]�w-Z�b�^�ɰ���(�۹�)�̰��I
    LONG lZBackDistance ,BOOL bZDisType(0�����m/1�۹��m)
    */
    /*��R�R�O(��R�Φ�(1~7)�A�e��(mm)�A��ݼe��(mm)�A)
    int iType, LONG lWidth, LONG lWidth2
    */
    /*�t�ΰѼ�(�X�ʳt�סA�[�t�סA��t��)
    LONG lWorkVelociy,LONG lAcceleration, LONG lInitVelociy
    */
#ifdef MOVE
    LONG lNowX = 0, lNowY = 0, lNowZ = 0;
    PauseDoGlue();//�Ȱ���_���~��X��(g_bIsPause=0)

    lNowX = MO_ReadLogicPosition(0);
    lNowY = MO_ReadLogicPosition(1);
    lNowZ = MO_ReadLogicPosition(2);

    if (!g_bIsStop)
    {
        MO_Do3DLineMove(lX1 - lNowX, lY1 - lNowY, lZ1 - lNowZ, lWorkVelociy, lAcceleration, lInitVelociy);//�^��_�l�I!
        PreventMoveError();
    }
    if (!bZDisType)//�����m
    {
        lZBackDistance = lZBackDistance - lZ1;
    }
    if (lZ1 == lZ2)
    {
        switch (iType)
        {
        case 0:
        {
            break;
        }
        case 1:
        {
            AttachFillType1(lX1, lY1, lX2, lY2, lZ1, lZBackDistance, lWidth, lWorkVelociy, lAcceleration, lInitVelociy);
            break;
        }
        case 2:
        {
            AttachFillType2(lX1, lY1, lX2, lY2, lZ1, lZBackDistance, lWidth, lWorkVelociy, lAcceleration, lInitVelociy);
            break;
        }
        case 3:
        {
            AttachFillType3(lX1, lY1, lX2, lY2, lZ1, lZBackDistance, lWidth, lWorkVelociy, lAcceleration, lInitVelociy);
            break;
        }
        case 4:
        {
            AttachFillType4(lX1, lY1, lX2, lY2, lZ1, lZBackDistance, lWidth, lWidth2, lWorkVelociy, lAcceleration, lInitVelociy);
            break;
        }
        case 5:
        {
            AttachFillType5(lX1, lY1, lX2, lY2, lZ1, lZBackDistance, lWidth, lWidth2, lWorkVelociy, lAcceleration, lInitVelociy);
            break;
        }
        case 6:
        {
            AttachFillType6(lX1, lY1, lX2, lY2, lZ1, lZBackDistance, lWidth, lWorkVelociy, lAcceleration, lInitVelociy);
            break;
        }
        case 7:
        {
            AttachFillType7(lX1, lY1, lX2, lY2, lZ1, lZBackDistance, lWidth, lWorkVelociy, lAcceleration, lInitVelociy);
            break;
        }
        default:
            break;
        }
    }
    else
    {
        AfxMessageBox(L"Z�b���פ��P�A�Эץ�");
    }
#endif
}
/*
*��X
*�g�J���w��X��A�åB�}�ҩ�����(�@16�ӿ�X)
*iPort ��ܰ�(�i�H��J0~15)
*bChoose �}������
*return BOOL(1�}�� 0����)
*/
BOOL CAction::DecideOutPutSign(int iPort, BOOL bChoose)
{
#ifdef MOVE
    MO_SetPIOOutput(iPort, bChoose);
#endif
    return bChoose;
}
/*
*��J
*Ū�����w��J��A�åB�}�ҩ�����(�@12�ӿ�J)
*iPort ��ܰ�(�i�H��J0~11)
*bChoose �}������
*return BOOL(1�}�� 0����)
*/
BOOL CAction::DecideInPutSign(int iPort, BOOL bChoose)
{
#ifdef MOVE
    if (MO_ReadPIOInput(iPort) == bChoose)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
#endif 
#ifndef MOVE
    return bChoose;
#endif
}
/*
*��m����
*��J�@��(X,Y,Z)�y�СA�|�^�Ǧ����{�b��m���۹�y��
*return CStirng(X,Y,Z)
*/
CString CAction::NowOffSet(LONG lX, LONG lY, LONG lZ)
{
#ifdef MOVE
    LONG lNowX = 0, lNowY = 0, lNowZ = 0;
    CString csBuff = 0;
    lNowX = MO_ReadLogicPosition(0);
    lNowY = MO_ReadLogicPosition(1);
    lNowZ = MO_ReadLogicPosition(1);
    csBuff.Format(_T("%ld,%ld,%ld"), (lNowX - lX), (lNowY - lY), (lNowZ - lZ));
    return csBuff;
#endif
#ifndef MOVE
    return NULL;
#endif
}
/*
*�^�ǥثe������u��m
*return CString(X,Y,Z)
*/
CString CAction::NowLocation()
{
    CString Location;
#ifdef MOVE
    Location.Format(_T("%d,%d,%d"), MO_ReadLogicPosition(0), MO_ReadLogicPosition(1), MO_ReadLogicPosition(2));
#endif
    return Location;
}

/***************************************************************************************************************�ۦ�B�Ψ��*/
/*
*����---�����I�ʧ@
*��J(�I�B�X�ʳt�סB�[�t�סB��t�סB�O�_�ϥΤ��_)
*/
void CAction::AttachPointMove(LONG lX, LONG lY, LONG lZ, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy,BOOL bIntt)
{
#ifdef MOVE
    LONG lNowX = 0, lNowY = 0, lNowZ = 0;
    lNowX = MO_ReadLogicPosition(0);
    lNowY = MO_ReadLogicPosition(1);
    lNowZ = MO_ReadLogicPosition(2);
    if (lX == 0)
    {
        lX = lNowX;
    }
    if (lY == 0)
    {
        lY = lNowY;
    }
    if (lZ == 0)
    {
        lZ = lNowZ;
    }
    if (bIntt == 1)
    {
        MO_Do3DLineMoveInterr(lX - lNowX, lY - lNowY, lZ - lNowZ, lWorkVelociy, lAcceleration, lInitVelociy, LPInterrupt);//�����������_
        PreventMoveError();//����b�d�X��
    }
    else
    {
        if (!g_bIsStop)
        {
            MO_Do3DLineMove(lX - lNowX, lY - lNowY, lZ - lNowZ, lWorkVelociy, lAcceleration, lInitVelociy);
            PreventMoveError();//����b�d�X��
        }
    }
#endif 
}
/*
*�X��(�t�X������ϥ�)
*��J(�I�����}��or�I��������)��Ķ�᪺LONG��
*/
void CAction::DoGlue(LONG lTime, LONG lDelayTime, LPTHREAD_START_ROUTINE GummingTimeOutThread)
{
#ifdef MOVE
    if (!g_bIsStop)
    {
        if (g_bIsDispend == 1)
        {
            MO_GummingSet(g_iNumberGluePort, lTime * 1000, GummingTimeOutThread);
        }
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
            if (g_bIsStop == 1)
            {
                MO_Timer(2, 0, 0);//����p�ɾ�
                break;
            }
            Sleep(1);
        }
    }
#endif
}      
/*
*�����
*���_�ɰ��槹���I���C
*/
DWORD CAction::GummingTimeOutThread(LPVOID)
{
#ifdef MOVE
    MO_FinishGumming();
#endif
    return 0;
}
/*              ***********************************************************************************
*�����
*���_�ɰ����C
*/
DWORD CAction::LPInterrupt(LPVOID)
{
#ifdef MOVE
    if (((CAction*)pAction)->g_bIsDispend == 1)
    {
        MO_GummingSet(((CAction*)pAction)->g_iNumberGluePort, 0, GummingTimeOutThread);//�(���d)
    }
#endif
    return 0;
}
/*
*����b�d�X��
*Ū���U�b�X�ʪ��A�A��ʧ@�ɰ���C
*/
void CAction::PreventMoveError()
{
#ifdef MOVE
    while (MO_ReadIsDriving(7) && !g_bIsStop)
    {
        Sleep(1);
    }
#endif
}
/*
*����X���X��
*Ū���X�����A�A��ʧ@�ɰ���C
*/
void CAction::PreventGlueError()
{
#ifdef MOVE
    while (MO_ReadGumming() && !g_bIsStop)
    {
        Sleep(1);
    }
#endif
}
/*
*�Ȱ��^�_���~��
*Ū���Ȱ��ѼơA��ѼƬ�0�ɥX���A�B�I�����n���}�C
*/
void CAction::PauseDoGlue()
{
#ifdef MOVE
    if (!g_bIsPause) //�Ȱ���_���~��X��(g_bIsPause=0)
    {
        if ((g_bIsPause == 0) && (g_bIsDispend == 1))
        {
            MO_GummingSet(g_iNumberGluePort, 0, GummingTimeOutThread);//�(���d)
        }
    }
#endif
}
/*
*�Ȱ��ɰ����
*Ū���Ȱ��ѼơA��ѼƬ�1���_���A���I�������ѼƬ�0�]�_���C
*/
void CAction::PauseStopGlue()
{
#ifdef MOVE
    if ((g_bIsPause == 1) || (g_bIsDispend == 0)) //�Ȱ��ɰ����(g_bIsPause=1)
    {
        MO_StopGumming();//����X��
    }
#endif
}
/*
*��^�]�m
*iType ��^���A,lXarEnd X�b�����I�y��(����),lYarEnd Y�b�����I�y��(����),lZarEnd Z�b�����I�y��(����),
*lXarUp X�b�W�@�I�y��(����),lYarUp Y�b�W�@�I�y��(����),lLineStop �^�{�Z��(�۹�),lStopZar�^�{�Z��(�۹�),lBackZar�^�ɶZ��(�۹�),
*lLowSpeed�C�^�{�t��,lHighSpeed���^�{�t��,lAcceleration�[�t��,lInitSpeed��t��
*/
void CAction::GelatinizeBack(int iType, LONG lXarEnd, LONG lYarEnd, LONG lZarEnd,
    LONG lXarUp, LONG lYarUp, LONG lLineStop, LONG lStopZar, LONG lBackZar,
    LONG lLowSpeed, LONG lHighSpeed, LONG lAcceleration, LONG lInitSpeed)
{
#ifdef MOVE
    LONG lBackXar, lBackYar, lR; //lBackXar x�b���ʮy��(����), lBackYar Y�b���ʮy��(����)
    lR = sqrt(pow(lXarEnd - lXarUp, 2) + pow(lYarEnd - lYarUp, 2));
    if (lLineStop > lR)
    {
        lBackXar = lXarUp;
        lBackYar = lYarUp;
    }
    else
    {
        LONG lXClose, lYClose, lLineClose;
        lLineClose = lLineStop;
        LineGetToPoint(lXClose, lYClose, lXarEnd, lYarEnd, lXarUp, lYarUp, lLineClose);
        lBackXar = lXClose;
        lBackYar = lYClose;
    }
    switch (iType) //0~5
    {
    case 0:
    {
        break;
    }
    case 1:
    {
        if (lStopZar > lZarEnd)
        {
            lStopZar = lZarEnd;
        }
        if (lStopZar == 0)
        {
            if (!g_bIsStop)
            {
                MO_Do3DLineMove(0, 0, (lZarEnd - lBackZar) - lZarEnd, lHighSpeed, lAcceleration, lInitSpeed);
                PreventMoveError();//����b�d�X��
            }
        }
        else
        {
            if (!g_bIsStop)
            {
                MO_Do3DLineMove(0, 0, (lZarEnd - lStopZar) - lZarEnd, lLowSpeed, lAcceleration, lInitSpeed);
            }
            if (!g_bIsStop)
            {
                MO_Do3DLineMove(0, 0, (lZarEnd - lBackZar) - (lZarEnd - lStopZar), lHighSpeed, lAcceleration, lInitSpeed);
                PreventMoveError();//����b�d�X��
            }
        }
        break;
    }
    case 2:
    {

        if (lStopZar == 0)
        {
            if (!g_bIsStop)
            {
                MO_Do3DLineMove(lBackXar - lXarEnd, lBackYar - lYarEnd, 0, lLowSpeed, lAcceleration, lInitSpeed);
            }
            if (!g_bIsStop)
            {
                MO_Do3DLineMove(0, 0, lBackZar - lZarEnd, lLowSpeed, lAcceleration, lInitSpeed);
                PreventMoveError();//����b�d�X��
            }


        }
        else
        {
            if (!g_bIsStop)
            {
                MO_Do3DLineMove(lBackXar - lXarEnd, lBackYar - lYarEnd, (lZarEnd - lStopZar) - lZarEnd, lLowSpeed, lAcceleration, lInitSpeed);
            }
            if (!g_bIsStop)
            {
                MO_Do3DLineMove(0, 0, lBackZar - (lZarEnd - lStopZar), lHighSpeed, lAcceleration, lInitSpeed);
                PreventMoveError();//����b�d�X��
            }
        }
        break;
    }
    case 3:
    {
        if (lStopZar == 0)
        {
            if (!g_bIsStop)
            {
                MO_Do3DLineMove(0, 0, 0, lLowSpeed, lAcceleration, lInitSpeed);
                PreventMoveError();//����b�d�X��
            }
        }
        else
        {
            if (!g_bIsStop)
            {
                MO_Do3DLineMove(0, 0, (lZarEnd - lStopZar) - lZarEnd, lLowSpeed, lAcceleration, lInitSpeed);
                PreventMoveError();//����b�d�X��
            }
        }
        if (!g_bIsStop)
        {
            MO_Do3DLineMove(lBackXar - lXarEnd, lBackYar - lYarEnd, 0, lHighSpeed, lAcceleration, lInitSpeed);
        }
        if (!g_bIsStop)
        {
            MO_Do3DLineMove(0, 0, lBackZar - (lZarEnd - lStopZar), lHighSpeed, lAcceleration, lInitSpeed);
            PreventMoveError();//����b�d�X��
        }
        break;
    }
    case 4:
    {
        lBackXar = (lXarEnd - lBackXar) + lXarEnd;
        lBackYar = (lYarEnd - lBackYar) + lYarEnd;
        if (lStopZar == 0)
        {
            if (!g_bIsStop)
            {
                MO_Do3DLineMove(lBackXar - lXarEnd, lBackYar - lYarEnd, 0, lLowSpeed, lAcceleration, lInitSpeed);
            }
            if (!g_bIsStop)
            {
                MO_Do3DLineMove(0, 0, lBackZar - lZarEnd, lLowSpeed, lAcceleration, lInitSpeed);
                PreventMoveError();//����b�d�X��
            }
        }
        else
        {
            if (!g_bIsStop)
            {
                MO_Do3DLineMove(lBackXar - lXarEnd, lBackYar - lYarEnd, (lZarEnd - lStopZar) - lZarEnd, lLowSpeed, lAcceleration, lInitSpeed);
            }
            if (!g_bIsStop)
            {
                MO_Do3DLineMove(0, 0, lBackZar - (lZarEnd - lStopZar), lHighSpeed, lAcceleration, lInitSpeed);
                PreventMoveError();//����b�d�X��
            }
        }
        break;
    }
    case 5:
    {
        lBackXar = (lXarEnd - lBackXar) + lXarEnd;
        lBackYar = (lYarEnd - lBackYar) + lYarEnd;
        if (lStopZar == 0)
        {
            if (!g_bIsStop)
            {
                MO_Do3DLineMove(0, 0, 0, lLowSpeed, lAcceleration, lInitSpeed);
                PreventMoveError();//����b�d�X��
            }
        }
        else
        {
            if (!g_bIsStop)
            {
                MO_Do3DLineMove(0, 0, (lZarEnd - lStopZar) - lZarEnd, lLowSpeed, lAcceleration, lInitSpeed);
                PreventMoveError();//����b�d�X��
            }
        }
        if (!g_bIsStop)
        {
            MO_Do3DLineMove(lBackXar - lXarEnd, lBackYar - lYarEnd, 0, lHighSpeed, lAcceleration, lInitSpeed);
        }
        if (!g_bIsStop)
        {
            MO_Do3DLineMove(0, 0, lBackZar - (lZarEnd - lStopZar), lHighSpeed, lAcceleration, lInitSpeed);
            PreventMoveError();//����b�d�X��
        }
        break;
    }
    default:
    {
        break;
    }
    }
#endif
}
/*
*���u�Z���ഫ���y���I
*/
void CAction::LineGetToPoint(LONG &lXClose, LONG &lYClose, LONG lX0, LONG lY0, LONG lX1, LONG lY1, LONG &lLineClose)
{
    DOUBLE dM, dB; //(X0,Y0)�_�l�I; (X1,Y1)�����I; Y0=M*X0+B �ײv����;L�w������
    if ((lX1 - lX0) == 0)
    {
        dM = 0;
    }
    else
    {
        dM = ((float)lY1 - (float)lY0) / ((float)lX1 - (float)lX0);
    }
    dB = lY0 - (dM*lX0);
    LONG la = lX1 - lX0, lb = lY1 - lY0;
    if (la == 0)
    {
        if (lY0 > lY1)
        {
            lYClose = lY1 + lLineClose;
            lXClose = lX1;
        }
        else
        {
            lYClose = lY1 - lLineClose;
            lXClose = lX1;
        }
    }
    else if (lb == 0)
    {
        if (lX0 > lX1)
        {
            lXClose = lX1 + lLineClose;
            lYClose = lY1;
        }
        else
        {
            lXClose = lX1 - lLineClose;
            lYClose = lY1;
        }
    }
    else
    {
        if (abs(la) > abs(lb))
        {
            if (la > 0)
            {
                lXClose = lX1 - lLineClose;
                lYClose = (dM*lX1) + dB;
            }
            else
            {
                lXClose = lX1 + lLineClose;
                lYClose = (dM*lX1) + dB;
            }
        }
        else if (abs(la) == abs(lb))
        {
            if (la > 0)
            {
                lXClose = lX1 - ((float)lLineClose / sqrt(2));
                lYClose = lY1 - ((float)lLineClose / sqrt(2));
            }
            else
            {
                lXClose = lX1 + ((float)lLineClose / sqrt(2));
                lYClose = lY1 + ((float)lLineClose / sqrt(2));
            }
        }
        else if (abs(la) < abs(lb))
        {
            if (lb > 0)
            {
                lYClose = lY1 - lLineClose;
                lXClose = (lYClose - dB) / dM;
            }
            else
            {
                lYClose = lY1 + lLineClose;
                lXClose = (lYClose - dB) / dM;
            }
        }
    }
}
/*
*�T�I�p����
*return CString(x�b��ߡAy�b��ߡA0�f��/1����A)
*/
CString CAction::TRoundCCalculation(CString Origin, CString End, CString Between)
{
    if (Origin == End || End == Between || Origin == Between)
    {
        AfxMessageBox(_T("What the fucking you type in my system."));
        return _T("F");
    }
    CString Ans;
    double x1 = double(CStringToLong(Origin, 0)); double y1 = double(CStringToLong(Origin, 1));
    double x2 = double(CStringToLong(End, 0)); double y2 = double(CStringToLong(End, 1));
    double x3 = double(CStringToLong(Between, 0)); double y3 = double(CStringToLong(Between, 1));

    double tan13 = (y3 - y1) / (x3 - x1);
    if ((tan13) >= 200) tan13 = 200;
    else if ((tan13) <= -200) tan13 = -200;
    double tan12 = (y2 - y1) / (x2 - x1);
    if ((tan12) >= 200) tan12 = 200;
    else if ((tan12) <= -200) tan13 = -200;

    double a = 2 * (x1 - x2);
    double b = 2 * (y1 - y2);
    double c = y1*y1 + x1*x1 - x2*x2 - y2*y2;
    double d = 2 * (x1 - x3);
    double e = 2 * (y1 - y3);
    double f = y1*y1 + x1*x1 - x3*x3 - y3*y3;
    int h = round((e*c - b*f) / (a*e - b*d));
    int k = round((a*f - d*c) / (a*e - b*d));
    Ans.Format(_T("%d�A%d�A"), h, k);
    if ((x2 - x1) > 0)
    {
        if ((y3 - tan12*x3) > (y1 - tan12*x1))
        {
            Ans = Ans + _T("0") + _T("�A");
        }
        else if ((y3 - tan12*x3) < (y1 - tan12*x1))
        {
            Ans = Ans + _T("1") + _T("�A");
        }
        else
        {
            Ans = _T("F");
            AfxMessageBox(_T("�T�I�@�u"));
        }
    }
    else if ((x2 - x1) < 0)
    {
        if ((y3 - tan12*x3) > (y1 - tan12*x1))
        {
            Ans = Ans + _T("1") + _T("�A");
        }
        else if ((y3 - tan12*x3) < (y1 - tan12*x1))
        {
            Ans = Ans + _T("0") + _T("�A");
        }
        else
        {
            Ans = _T("F");
            AfxMessageBox(_T("�T�I�@�u"));
        }
    }
    else
    {
        if ((x3 > x1))
        {
            Ans = Ans + _T("1") + _T("�A");
        }
        else if ((x3 < x1))
        {
            Ans = Ans + _T("0") + _T("�A");
        }
        else
        {
            Ans = _T("F");
            AfxMessageBox(_T("�T�I�@�u"));
        }
    }
    return Ans;
}
/*
*�r��������
*�ϥΪ������@�w�n�O"�A"
*/
LONG CAction::CStringToLong(CString csData, int iChoose)
{
    LONG lOne[256] = { 0 };//�ഫ����
    int ione[256] = { 0 };//�аO��m
    ione[0] = csData.Find(L"�A");
    for (int i = 0; i<csData.GetLength(); i++)
    {
        ione[i + 1] = csData.Find(L"�A", ione[i] + 1);
    }
    lOne[0] = _ttol(csData.Left(ione[0]));
    for (int j = 0; j<iChoose; j++)
    {
        lOne[j + 1] = _ttol(csData.Mid(ione[j] + 1, ione[j + 1] - ione[j] - 1));
    }
    return lOne[iChoose];
}
/*
*�T�I���o����
*(�V�q����)
*(���x,y�y��x1,y1,�y��x2,y2,���f��)
*/
DOUBLE CAction::AngleCount(DOUBLE LocatX, DOUBLE LocatY, DOUBLE LocatX1, DOUBLE LocatY1, DOUBLE LocatX2, DOUBLE LocatY2, BOOL bRev)
{
    //offsetx=offset x,offsety=offset y,sum=�K,
    DOUBLE offsetx, offsety;
    DOUBLE Xa, Ya;
    DOUBLE theta_a;
    DOUBLE offset_x, offset_y;
    DOUBLE X_a, Y_a;
    DOUBLE theta_b;

    offsetx = LocatX1 - LocatX;
    offsety = LocatY1 - LocatY;

    Xa = offsetx / pow(((pow(offsetx, 2)) + (pow(offsety, 2))), 0.5);
    Ya = offsety / pow(((pow(offsetx, 2)) + (pow(offsety, 2))), 0.5);

    theta_a = (atan(fabs((Ya / Xa)))) * 180 / M_PI;

    if (offsetx<0 && offsety >= 0)
        theta_a = 180 - theta_a;
    else if (offsetx<0 && offsety<0)
        theta_a = 180 + theta_a;
    else if (offsetx >= 0 && offsety<0)
        theta_a = 360 - theta_a;

    offset_x = LocatX2 - LocatX;
    offset_y = LocatY2 - LocatY;

    X_a = offset_x / pow(((pow(offset_x, 2)) + (pow(offset_y, 2))), 0.5);
    Y_a = offset_y / pow(((pow(offset_x, 2)) + (pow(offset_y, 2))), 0.5);

    theta_b = (atan(fabs((Y_a / X_a)))) * 180 / M_PI;

    if (offset_x<0 && offset_y >= 0)
        theta_b = 180 - theta_b;
    else if (offset_x<0 && offset_y<0)
        theta_b = 180 + theta_b;
    else if (offset_x >= 0 && offset_y<0)
        theta_b = 360 - theta_b;

    if (theta_a<theta_b)
        theta_a = theta_b - theta_a;
    else
        theta_a = 360 - (theta_a - theta_b);

    if (bRev == 1)
    {
        theta_a = 360 - theta_a;
    }

    return theta_a;
}
/*�꩷���Z�����I����
*�Ѷ꩷���A���X�I�y��
*bDir 0�f��/1����
*/
void CAction::ArcGetToPoint(LONG &lArcX, LONG &lArcY, LONG lDistance, LONG lX, LONG lY, LONG lCenX, LONG lCenY, LONG lRadius, BOOL bDir)
{
    DOUBLE dAngl = 0;
    dAngl = (DOUBLE)lDistance / (DOUBLE)lRadius;
    if (bDir == 0)
    {
        lArcX = (((DOUBLE)lX - (DOUBLE)lCenX)*cos(dAngl)) + (((DOUBLE)lY - (DOUBLE)lCenY)*sin(dAngl)) + lCenX;
        lArcY = (((DOUBLE)lX - (DOUBLE)lCenX)*sin(dAngl)) + (((DOUBLE)lY - (DOUBLE)lCenY)*cos(dAngl)) + lCenY;
    }
    else
    {
        lArcX = (((DOUBLE)lX - (DOUBLE)lCenX)*cos(dAngl)) - (((DOUBLE)lY - (DOUBLE)lCenY)*sin(dAngl)) + lCenX;
        lArcY = (((DOUBLE)lX - (DOUBLE)lCenX)*sin(dAngl)) + (((DOUBLE)lY - (DOUBLE)lCenY)*cos(dAngl)) + lCenY;
    }
}


/*����---��R�κA(���A1�x��s���|��R)
*��J(�_�l�Ix1,y1,�����Ix2,y2,�e��,�X�ʳt��,�[�t��,��t��)
*/
void CAction::AttachFillType1(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lZ, LONG lZBackDistance, LONG lWidth, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
#ifdef MOVE
    LONG lBuffX = 0, lBuffY = 0, lBuffMove = 0;
    LONG lPointX1 = 0, lPointY1 = 0, lPointX2 = 0, lPointY2 = 0;
    LONG lLineClose = 0, lXClose = 0, lYClose = 0;
    LONG lLingX1 = 0, lLingY1 = 0;
    LONG lNowX = 0, lNowY = 0;
    BOOL bNum1 = 0;
    int iType1 = 0, iCnt1 = 0, iData1 = 0;
    if ((lX1 == lX2) || (lY1 == lY2))//�_�l�I�P�����I���@���u
    {
        if ((lX2>lX1) == 1 || (lY2>lY1))//iType1 == 1 || iType1 == 3
        {
            bNum1 = 1;
        }
        else  //((lX2<lX1) || (lY2<lY1))iType1 == 2 || iType1 == 4
        {
            bNum1 = 0;
        }
        if ((lX2 > lX1) || (lX2 < lX1))//iType1 == 1 || iType1 == 2
        {
            lPointX1 = lX1 + (lX2 - lX1) / 2;
            lPointY1 = lY1 + (abs(lX2 - lX1) / 2);
            lPointX2 = lPointX1;
            lPointY2 = lY1 - (abs(lX2 - lX1) / 2);
        }
        else //((lY2 > lY1) || (lY2 < lY1))iType1 == 3 || iType1 == 4
        {
            lPointY1 = lY1 + ((lY2 - lY1) / 2);
            lPointX1 = lX1 + (abs(lY2 - lY1) / 2);
            lPointX2 = lX1 - (abs(lY2 - lY1) / 2);
            lPointY2 = lPointY1;
        }
        if (lPointY2 < 0 || lPointX2 < 0) //�P�_�x�Υ|���I�O�_�W�L�t����
        {
            AfxMessageBox(L"�W�X�����d��");
            g_bIsStop = 1;//����ʧ@
        }
        lBuffMove = abs(lPointX1 - lX1)*sqrt(2);//���ʶq
        iCnt1 = abs(lBuffMove) / (lWidth * 1000);  //���榸��
        iData1 = abs(lBuffMove) % (lWidth * 1000); //���v���e��
        lBuffX = abs(lPointX1 - lX1);
        lBuffY = abs(lPointY1 - lY1);//X���ʶq
        lLineClose = (lWidth * 1000);
        LineGetToPoint(lXClose, lYClose, lPointX1, lPointY1, lX1, lY1, lLineClose);
        lLingX1 = abs(lXClose - lX1);
        lLingY1 = abs(lYClose - lY1);//Y���ʶq

        PauseDoGlue();//�Ȱ���_���~��X��(g_bIsPause=0)�X��
        for (int i = 1; i <= iCnt1; i++)
        {
            if ((lX2 > lX1) || (lX2 < lX1))//iType1 == 1 || iType1 == 2
            {
                if (i % 2 == bNum1)// (bNum1=1)X2>X1 �hX���k����/(bNum1=0) X2<X1 �hX��������   1+2-
                {
                    if (!g_bIsStop)
                    {
                        MO_Do3DLineMove(lBuffX, lBuffY, 0, lWorkVelociy, lAcceleration, lInitVelociy);//X���k�U����(+)
                        PreventMoveError();//����b�d�X��
                    }
                }
                else
                {
                    if (!g_bIsStop)
                    {
                        MO_Do3DLineMove((~lBuffX), (~lBuffY), 0, lWorkVelociy, lAcceleration, lInitVelociy);//X�����W����(-)
                        PreventMoveError();//����b�d�X��
                    }
                }
            }
            else//((lY2 > lY1) || (lY2 < lY1))iType1 == 3 || iType1 == 4
            {
                if (i % 2 == bNum1)// (bNum1=1)X2>X1 �hX���k����/(bNum1=0) X2<X1 �hX��������   3+4-
                {
                    if (!g_bIsStop)
                    {
                        MO_Do3DLineMove((~lBuffX), lBuffY, 0, lWorkVelociy, lAcceleration, lInitVelociy);//x�����U����(x-,y+)
                        PreventMoveError();//����b�d�X��
                    }
                }
                else
                {
                    if (!g_bIsStop)
                    {
                        MO_Do3DLineMove(lBuffX, (~lBuffY), 0, lWorkVelociy, lAcceleration, lInitVelociy);//x���k�W����(x+,y-)
                        PreventMoveError();//����b�d�X��
                    }
                }
            }
            if (lX2>lX1)//iType1 = 1;
            {
                if (!g_bIsStop)
                {
                    MO_Do3DLineMove(lLingX1, (~lLingY1), 0, lWorkVelociy, lAcceleration, lInitVelociy);//Y���k�W����(x+,y-)
                    PreventMoveError();//����b�d�X��
                }
            }
            else if (lX2<lX1)//iType1 = 2;
            {
                if (!g_bIsStop)
                {
                    MO_Do3DLineMove((~lLingX1), lLingY1, 0, lWorkVelociy, lAcceleration, lInitVelociy);//Y���U����(x-,y+)
                    PreventMoveError();//����b�d�X��
                }
            }
            else if (lY2>lY1)//iType1 = 3;
            {
                if (!g_bIsStop)
                {
                    MO_Do3DLineMove(lLingX1, lLingY1, 0, lWorkVelociy, lAcceleration, lInitVelociy);//Y���k�U����(+)
                    PreventMoveError();//����b�d�X��
                }
            }
            else if (lY2<lY1)//iType1 = 4;
            {
                if (!g_bIsStop)
                {
                    MO_Do3DLineMove((~lLingX1), (~lLingY1), 0, lWorkVelociy, lAcceleration, lInitVelociy);//Y���W����(-)
                    PreventMoveError();//����b�d�X��
                }
            }
            if (i == iCnt1) //�̫�@���A�P�_�O�_�ݭn���v
            {
                if ((lX2 > lX1) || (lX2 < lX1))//iType1 == 1 || iType1 == 2
                {
                    if (i % 2 != bNum1)// (bNum1=1)X2>X1 �hX���k����/(bNum1=0) X2<X1 �hX��������   1+2-
                    {
                        if (!g_bIsStop)
                        {
                            MO_Do3DLineMove(lBuffX, lBuffY, 0, lWorkVelociy, lAcceleration, lInitVelociy);//X���k�U����(+)
                            PreventMoveError();//����b�d�X��
                        }
                    }
                    else
                    {
                        if (!g_bIsStop)
                        {
                            MO_Do3DLineMove((~lBuffX), (~lBuffY), 0, lWorkVelociy, lAcceleration, lInitVelociy);//X�����W����(-)
                            PreventMoveError();//����b�d�X��
                        }
                    }
                }
                else//((lY2 > lY1) || (lY2 < lY1))iType1 == 3 || iType1 == 4
                {
                    if (i % 2 != bNum1)// (bNum1=1)X2>X1 �hX���k����/(bNum1=0) X2<X1 �hX��������   3+4-
                    {
                        if (!g_bIsStop)
                        {
                            MO_Do3DLineMove((~lBuffX), lBuffY, 0, lWorkVelociy, lAcceleration, lInitVelociy);//x�����U����(x-,y+)
                            PreventMoveError();//����b�d�X��
                        }
                    }
                    else
                    {
                        if (!g_bIsStop)
                        {
                            MO_Do3DLineMove(lBuffX, (~lBuffY), 0, lWorkVelociy, lAcceleration, lInitVelociy);//x���k�W����(x+,y-)
                            PreventMoveError();//����b�d�X��
                        }
                    }
                }
                if (iData1 != 0 && i % 2 == 1)
                {
                    lNowX = MO_ReadLogicPosition(0);
                    lNowY = MO_ReadLogicPosition(1);
                    if ((lX2 > lX1) || (lY2 < lY1))//iType1 = 1,4;
                    {
                        if (!g_bIsStop)
                        {
                            MO_Do3DLineMove(lPointX2 - lNowX, lPointY2 - lNowY, 0, lWorkVelociy, lAcceleration, lInitVelociy);//���ʨ�(px2,py2)
                            PreventMoveError();//����b�d�X��
                        }
                    }
                    else if ((lX2<lX1) || (lY2>lY1))//iType1 = 2,3;
                    {
                        if (!g_bIsStop)
                        {
                            MO_Do3DLineMove(lPointX1 - lNowX, lPointY1 - lNowY, 0, lWorkVelociy, lAcceleration, lInitVelociy);//���ʨ�(px1,py1)
                            PreventMoveError();//����b�d�X��
                        }
                    }
                }
                lNowX = MO_ReadLogicPosition(0);
                lNowY = MO_ReadLogicPosition(1);
                if (!g_bIsStop)
                {
                    MO_Do3DLineMove(lX2 - lNowX, lY2 - lNowY, 0, lWorkVelociy, lAcceleration, lInitVelociy);//���ʨ쵲���I
                    PreventMoveError();//����b�d�X��
                }

            }
        }

    }
    else //�_�l�I�P�����I���׽u
    {
        lBuffX = abs(lX2 - lX1);//X���ʶq
        lBuffY = abs(lY2 - lY1);//Y���ʶq
        iCnt1 = abs(lBuffY) / (lWidth * 1000);  //���榸��
        iData1 = abs(lBuffY) % (lWidth * 1000); //���v���e��
        if (lX2>lX1 && lY2>lY1)
        {
            iType1 = 1;
        }
        else if (lX2>lX1 && lY2<lY1)
        {
            iType1 = 2;
        }
        else if (lX2<lX1 &&lY2>lY1)
        {
            iType1 = 3;
        }
        else if (lX2<lX1 &&lY2<lY1)
        {
            iType1 = 4;
        }

        if (iType1 == 1 || iType1 == 2)
        {
            bNum1 = 1;
        }
        else if (iType1 == 3 || iType1 == 4)
        {
            bNum1 = 0;
        }

        for (int i = 1; i <= iCnt1; i++)
        {
            if (i % 2 == bNum1)// (bNum1=1)X2>X1 �hX���k����/(bNum1=0) X2<X1 �hX��������
            {
                if (!g_bIsStop)
                {
                    MO_Do3DLineMove(lBuffX, 0, 0, lWorkVelociy, lAcceleration, lInitVelociy);//X���k����(+)
                    PreventMoveError();//����b�d�X��
                }
            }
            else
            {
                if (!g_bIsStop)
                {
                    MO_Do3DLineMove((~lBuffX), 0, 0, lWorkVelociy, lAcceleration, lInitVelociy);//X��������(-)
                    PreventMoveError();//����b�d�X��
                }
            }
            if (iType1 == 1 || iType1 == 3)
            {
                if (!g_bIsStop)
                {
                    MO_Do3DLineMove(0, (lWidth * 1000), 0, lWorkVelociy, lAcceleration, lInitVelociy);//Y���U����(+)
                    PreventMoveError();//����b�d�X��
                }
            }
            else
            {
                if (!g_bIsStop)
                {
                    MO_Do3DLineMove(0, (~lWidth * 1000), 0, lWorkVelociy, lAcceleration, lInitVelociy);//Y���W����(-)
                    PreventMoveError();//����b�d�X��
                }
            }
            if (i == iCnt1) //�̫�@���A�P�_�O�_�ݭn���v
            {
                if (i % 2 != bNum1)// (bNum1=1)X2>X1 �hX���k����/(bNum1=0) X2<X1 �hX��������
                {
                    if (!g_bIsStop)
                    {
                        MO_Do3DLineMove(lBuffX, 0, 0, lWorkVelociy, lAcceleration, lInitVelociy);//X���k����(+)
                        PreventMoveError();//����b�d�X��
                    }
                }
                else
                {
                    if (!g_bIsStop)
                    {
                        MO_Do3DLineMove((~lBuffX), 0, 0, lWorkVelociy, lAcceleration, lInitVelociy);//X��������(-)
                        PreventMoveError();//����b�d�X��
                    }
                }
                if (iData1 != 0)
                {
                    if (iType1 == 1 || iType1 == 3)
                    {
                        if (!g_bIsStop)
                        {
                            MO_Do3DLineMove(0, iData1, 0, lWorkVelociy, lAcceleration, lInitVelociy);//Y���U����(+)
                            PreventMoveError();//����b�d�X��
                        }
                    }
                    else
                    {
                        if (!g_bIsStop)
                        {
                            MO_Do3DLineMove(0, (~iData1), 0, lWorkVelociy, lAcceleration, lInitVelociy);//Y���W����(-)
                            PreventMoveError();//����b�d�X��
                        }
                    }
                    if (i % 2 == bNum1)
                    {
                        if (!g_bIsStop)
                        {
                            MO_Do3DLineMove(lBuffX, 0, 0, lWorkVelociy, lAcceleration, lInitVelociy);//X���k����(+)
                            PreventMoveError();//����b�d�X��
                        }
                    }
                    else
                    {
                        if (!g_bIsStop)
                        {
                            MO_Do3DLineMove((~lBuffX), 0, 0, lWorkVelociy, lAcceleration, lInitVelociy);//X��������(-)
                            PreventMoveError();//����b�d�X��
                        }
                    }
                }
            }
        }
    }
    PauseStopGlue();//�Ȱ��ɰ����(g_bIsPause=1)
    MO_StopGumming();//����X��
    if (!g_bIsStop)
    {
        MO_Do3DLineMove(0, 0, (lZ - lZBackDistance) - lZ, lWorkVelociy, lAcceleration, lInitVelociy);//Z�b��^
        PreventMoveError();//����b�d�X��
    }
#endif
}
/*����---��R�κA(���A2������۶�R.�ѥ~�Ӥ�)
*��J(�_�l�Ix1,y1,���x2,y2,�e��,�X�ʳt��,�[�t��,��t��)
*/
void CAction::AttachFillType2(LONG lX1, LONG lY1, LONG lCenX, LONG lCenY, LONG lZ, LONG lZBackDistance, LONG lWidth, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
#ifdef MOVE
    LONG lRadius = 0, lX2 = 0, lY2 = 0, lCenX2 = 0, lCenY2 = 0;
    DOUBLE dAng0 = 0, dAng1 = 0, dAng2 = 0;
    BOOL bRev = 1;//0�f��/1����
    LONG lLineClose = 0, lXClose = 0, lYClose = 0;
    LONG lNowX = 0, lNowY = 0;
    int  iCnt1 = 0, iData1 = 0, lBuffX = 0, lBuffY = 0;
    lRadius = sqrt(pow(lCenX - lX1, 2) + pow(lCenY - lY1, 2));//�b�|
    dAng1 = ((DOUBLE)lX1 - (DOUBLE)lCenX) / (DOUBLE)lRadius;
    dAng2 = ((DOUBLE)lY1 - (DOUBLE)lCenY) / (DOUBLE)lRadius;
    dAng1 = acos(dAng1);//����
    dAng2 = asin(dAng2);
    iCnt1 = abs(lRadius) / (lWidth * 1000);  //���榸��
    iData1 = abs(lRadius) % (lWidth * 1000); //���v���e��
    lX2 = lRadius*cos(dAng1 + M_PI) + lCenX;
    lY2 = lRadius*sin(dAng2 + M_PI) + lCenY;
    PauseDoGlue();//�Ȱ���_���~��X��(g_bIsPause=0)�X��
    if (!g_bIsStop)
    {
        MO_Do2DArcMove(lX2 - lX1, lY2 - lY1, lCenX - lX1, lCenY - lY1, lInitVelociy, lWorkVelociy, bRev);//��l�b��
        PreventMoveError();
    }
    for (int i = 1; i <= iCnt1; i++)
    {
        lLineClose = (lWidth * 1000);
        LineGetToPoint(lXClose, lYClose, lCenX, lCenY, lX1, lY1, lLineClose);

        lCenX2 = (lX2 + lXClose) / 2;
        lCenY2 = (lY2 + lYClose) / 2;
        lNowX = MO_ReadLogicPosition(0);
        lNowY = MO_ReadLogicPosition(1);
        if (!g_bIsStop)
        {
            MO_Do2DArcMove(lX1 - lNowX, lY1 - lNowY, lCenX2 - lNowX, lCenY2 - lNowY, lInitVelociy, lWorkVelociy, bRev);//�U�b��
            PreventMoveError();
        }
        lX1 = lXClose;
        lY1 = lYClose;
        Sleep(1);
        LineGetToPoint(lXClose, lYClose, lCenX, lCenY, lX2, lY2, lLineClose);
        lNowX = MO_ReadLogicPosition(0);
        lNowY = MO_ReadLogicPosition(1);
        if (!g_bIsStop)
        {
            MO_Do2DArcMove(lXClose - lNowX, lYClose - lNowY, lCenX - lNowX, lCenY - lNowY, lInitVelociy, lWorkVelociy, bRev);//�P�ߥb��
            PreventMoveError();
        }
        lX2 = lXClose;
        lY2 = lYClose;

        if ((i == iCnt1) && (iData1 != 0))
        {
            lNowX = MO_ReadLogicPosition(0);
            lNowY = MO_ReadLogicPosition(1);
            if (!g_bIsStop)
            {
                MO_Do3DLineMove(lCenX - lNowX, lCenY - lNowY, 0, lWorkVelociy, lAcceleration, lInitVelociy);
                PreventMoveError();
            }
        }
    }
    PauseStopGlue();//�Ȱ��ɰ����(g_bIsPause=1)
    MO_StopGumming();//����X��
    if (!g_bIsStop)
    {
        MO_Do3DLineMove(0, 0, (lZ - lZBackDistance) - lZ, lWorkVelociy, lAcceleration, lInitVelociy);//Z�b��^
        PreventMoveError();//����b�d�X��
    }
#endif
}
/*����---��R�κA(���A3�x�ζ�R.�ѥ~�Ӥ�)
*��J(�_�l�Ix1,y1,�����Ix2,y2,�e��,�X�ʳt��,�[�t��,��t��)
*/
void CAction::AttachFillType3(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lZ, LONG lZBackDistance, LONG lWidth, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
#ifdef MOVE
#pragma region ****�Ƥ諬���Y4�I�\��****
    LONG lNowX = 0, lNowY = 0;
    CPoint cPt1 = 0, cPt2 = 0, cPt3 = 0, cPt4 = 0, cPtCen = 0;
    DOUBLE dRadius = 0, dDistance = 0, dWidth = 0, dAngCenCos = 0, dAngCenSin = 0, dAngCos = 0, dAngSin = 0;
    DOUBLE dAngCenCos2 = 0, dAngCenSin2 = 0, dAngCos2 = 0, dAngSin2 = 0;
    std::vector<CPoint>::iterator ptIter;//���N��
    std::vector<CPoint> m_ptVec;
    m_ptVec.clear();
    cPt1.x = lX1;
    cPt1.y = lY1;
    cPt3.x = lX2;
    cPt3.y = lX2;
    dWidth = lWidth * 1000;
    cPtCen.x = cPt1.x + (cPt3.x - cPt1.x) / 2;
    cPtCen.y = cPt1.y + (cPt3.y - cPt1.y) / 2;
    dRadius = sqrt(pow(cPt1.x - cPtCen.x, 2) + pow(cPt1.y - cPtCen.y, 2));
    dAngCenCos = acos(double(cPt1.x - cPtCen.x) / dRadius);
    dAngCenSin = asin(double(cPt1.y - cPtCen.y) / dRadius);
    dAngCenCos2 = M_PI * 2 - dAngCenCos;
    dAngCenSin2 = M_PI - dAngCenSin;
    if (abs(dAngCenCos - dAngCenSin) > 0.01)
    {
        if (abs(dAngCenCos - dAngCenSin2) < 0.01)
        {
            dAngCenSin = dAngCenSin2;
        }
        else if (abs(dAngCenCos2 - dAngCenSin) < 0.01)
        {
            dAngCenCos = dAngCenCos2;
        }
        else
        {
            dAngCenSin = dAngCenSin2;
            dAngCenCos = dAngCenCos2;
        }
    }
    else
    {
        dAngCenSin = dAngCenCos;
    }
    cPt2.x = dRadius*cos(dAngCenCos + M_PI_2) + cPtCen.x;
    cPt2.y = dRadius*sin(dAngCenSin + M_PI_2) + cPtCen.y;
    cPt4.x = dRadius*cos(dAngCenCos + M_PI_2 * 3) + cPtCen.x;
    cPt4.y = dRadius*sin(dAngCenSin + M_PI_2 * 3) + cPtCen.y;
    dDistance = sqrt(pow((cPt1.x - cPt4.x), 2) + pow((cPt1.y - cPt4.y), 2));
    m_ptVec.push_back(cPt1);
    m_ptVec.push_back(cPt2);
    m_ptVec.push_back(cPt3);
    m_ptVec.push_back(cPt4);
    dAngCos = acos((cPt1.x - cPt4.x) / dDistance);
    dAngSin = asin((cPt1.y - cPt4.y) / dDistance);
    if (dAngCos < 0)
    {
        dAngCos += M_PI * 2;
    }
    if (dAngSin < 0)
    {
        dAngSin += M_PI * 2;
    }
    dAngCos2 = M_PI * 2 - dAngCos;
    dAngSin2 = M_PI - dAngSin;
    if (abs(dAngCos - dAngSin) > 0.01)
    {
        if (abs(dAngCos - dAngSin2) < 0.01)
        {
            dAngSin = dAngSin2;
        }
        else if (abs(dAngCos2 - dAngSin) < 0.01)
        {
            dAngCos = dAngCos2;
        }
        else
        {
            dAngCos = dAngCos2;
            dAngSin = dAngSin2;
        }
    }
    else
    {
        dAngSin = dAngCos;
    }
    while (1)
    {
        dRadius = dRadius - dWidth*sqrt(2);
        dDistance = dDistance - dWidth;
        if (dDistance < dWidth)
        {
            break;
        }
        cPt1.x = dDistance*cos(dAngCos) + cPt4.x;
        cPt1.y = dDistance*sin(dAngSin) + cPt4.y;
        m_ptVec.push_back(cPt1);
        cPt2.x = dRadius*cos(dAngCenCos + M_PI_2) + cPtCen.x;
        cPt2.y = dRadius*sin(dAngCenSin + M_PI_2) + cPtCen.y;
        m_ptVec.push_back(cPt2);
        dDistance = dDistance - dWidth;
        if (dDistance < dWidth)
        {
            break;
        }
        cPt3.x = dRadius*cos(dAngCenCos + M_PI) + cPtCen.x;
        cPt3.y = dRadius*sin(dAngCenSin + M_PI) + cPtCen.y;
        m_ptVec.push_back(cPt3);
        cPt4.x = dRadius*cos(dAngCenCos + M_PI_2 * 3) + cPtCen.x;
        cPt4.y = dRadius*sin(dAngCenSin + M_PI_2 * 3) + cPtCen.y;
        m_ptVec.push_back(cPt4);
    }
#pragma endregion
    PauseDoGlue();//�Ȱ���_���~��X��(g_bIsPause=0)�X��
    for (ptIter = m_ptVec.begin(); ptIter != m_ptVec.end(); ptIter++)
    {
        lNowX = MO_ReadLogicPosition(0);
        lNowY = MO_ReadLogicPosition(1);
        if (!g_bIsStop)
        {
            MO_Do3DLineMove((*ptIter).x - lNowX, (*ptIter).y - lNowY, 0, lWorkVelociy, lAcceleration, lInitVelociy);//����
            PreventMoveError();
        }
    }
    PauseStopGlue();//�Ȱ��ɰ����(g_bIsPause=1)
    MO_StopGumming();//����X��
    if (!g_bIsStop)
    {
        MO_Do3DLineMove(0, 0, (lZ - lZBackDistance) - lZ, lWorkVelociy, lAcceleration, lInitVelociy);//Z�b��^
        PreventMoveError();//����b�d�X��
    }
#endif
}
/*����---��R�κA(���A4�x����)
*��J(�_�l�Ix1,y1,�����Ix2,y2,�e��,��ݼe��,�X�ʳt��,�[�t��,��t��)
*/
void CAction::AttachFillType4(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lZ, LONG lZBackDistance, LONG lWidth, LONG lWidth2, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
#ifdef MOVE
#pragma region ****�Ƥ諬���Y4�I�\��****
    LONG lNowX = 0, lNowY = 0;
    CPoint cPt1 = 0, cPt2 = 0, cPt3 = 0, cPt4 = 0, cPtCen = 0;
    DOUBLE dRadius = 0, dDistance = 0, dWidth = 0, dWidth2 = 0, dAngCenCos = 0, dAngCenSin = 0, dAngCos = 0, dAngSin = 0;
    DOUBLE dAngCenCos2 = 0, dAngCenSin2 = 0, dAngCos2 = 0, dAngSin2 = 0;
    int iBuff = 0;//�P�_��ݼe�ץ�
    std::vector<CPoint>::iterator ptIter;//���N��
    std::vector<CPoint> m_ptVec;
    m_ptVec.clear();
    cPt1.x = lX1;
    cPt1.y = lY1;
    cPt3.x = lX2;
    cPt3.y = lX2;
    dWidth = lWidth * 1000;
    dWidth2 = lWidth2 * 1000;
    cPtCen.x = cPt1.x + (cPt3.x - cPt1.x) / 2;
    cPtCen.y = cPt1.y + (cPt3.y - cPt1.y) / 2;
    dRadius = sqrt(pow(cPt1.x - cPtCen.x, 2) + pow(cPt1.y - cPtCen.y, 2));
    dAngCenCos = acos(DOUBLE(cPt1.x - cPtCen.x) / dRadius);
    dAngCenSin = asin(DOUBLE(cPt1.y - cPtCen.y) / dRadius);
    dAngCenCos2 = M_PI * 2 - dAngCenCos;
    dAngCenSin2 = M_PI - dAngCenSin;
    if (abs(dAngCenCos - dAngCenSin) > 0.01)
    {
        if (abs(dAngCenCos - dAngCenSin2) < 0.01)
        {
            dAngCenSin = dAngCenSin2;
        }
        else if (abs(dAngCenCos2 - dAngCenSin) < 0.01)
        {
            dAngCenCos = dAngCenCos2;
        }
        else
        {
            dAngCenSin = dAngCenSin2;
            dAngCenCos = dAngCenCos2;
        }
    }
    else
    {
        dAngCenSin = dAngCenCos;
    }
    cPt2.x = dRadius*cos(dAngCenCos + M_PI_2) + cPtCen.x;
    cPt2.y = dRadius*sin(dAngCenSin + M_PI_2) + cPtCen.y;
    cPt4.x = dRadius*cos(dAngCenCos + M_PI_2 * 3) + cPtCen.x;
    cPt4.y = dRadius*sin(dAngCenSin + M_PI_2 * 3) + cPtCen.y;
    dDistance = sqrt(pow((cPt1.x - cPt4.x), 2) + pow((cPt1.y - cPt4.y), 2));
    m_ptVec.push_back(cPt1);
    m_ptVec.push_back(cPt2);
    m_ptVec.push_back(cPt3);
    m_ptVec.push_back(cPt4);

    dAngCos = acos((cPt1.x - cPt4.x) / dDistance);
    dAngSin = asin((cPt1.y - cPt4.y) / dDistance);
    if (dAngCos < 0)
    {
        dAngCos += M_PI * 2;
    }
    if (dAngSin < 0)
    {
        dAngSin += M_PI * 2;
    }
    dAngCos2 = M_PI * 2 - dAngCos;
    dAngSin2 = M_PI - dAngSin;
    if (abs(dAngCos - dAngSin) > 0.01)
    {
        if (abs(dAngCos - dAngSin2) < 0.01)
        {
            dAngSin = dAngSin2;
        }
        else if (abs(dAngCos2 - dAngSin) < 0.01)
        {
            dAngCos = dAngCos2;
        }
        else
        {
            dAngCos = dAngCos2;
            dAngSin = dAngSin2;
        }
    }
    else
    {
        dAngSin = dAngCos;
    }
    iBuff = 1;
    while (1)
    {
        if ((iBuff == 1) && (dRadius - (2 * dWidth2) < 0))
        {
            AfxMessageBox(L"��ݼe�׹L�j");
            m_ptVec.push_back(cPt1);
            break;
        }
        dRadius = dRadius - dWidth*sqrt(2);
        dDistance = dDistance - dWidth;
        if (iBuff*dWidth >  dWidth2) //dWidth2��ݼe��
        {
            dDistance += dWidth;
            cPt1.x = dDistance*cos(dAngCos) + cPt4.x;
            cPt1.y = dDistance*sin(dAngSin) + cPt4.y;
            m_ptVec.push_back(cPt1);
            break;
        }
        if (dDistance < dWidth)
        {
            break;
        }
        cPt1.x = dDistance*cos(dAngCos) + cPt4.x;
        cPt1.y = dDistance*sin(dAngSin) + cPt4.y;
        m_ptVec.push_back(cPt1);
        cPt2.x = dRadius*cos(dAngCenCos + M_PI_2) + cPtCen.x;
        cPt2.y = dRadius*sin(dAngCenSin + M_PI_2) + cPtCen.y;
        m_ptVec.push_back(cPt2);
        dDistance = dDistance - dWidth;
        if (dDistance < dWidth)
        {
            break;
        }
        cPt3.x = dRadius*cos(dAngCenCos + M_PI) + cPtCen.x;
        cPt3.y = dRadius*sin(dAngCenSin + M_PI) + cPtCen.y;
        m_ptVec.push_back(cPt3);
        cPt4.x = dRadius*cos(dAngCenCos + M_PI_2 * 3) + cPtCen.x;
        cPt4.y = dRadius*sin(dAngCenSin + M_PI_2 * 3) + cPtCen.y;
        m_ptVec.push_back(cPt4);
        iBuff++;
    }
#pragma endregion	
    PauseDoGlue();//�Ȱ���_���~��X��(g_bIsPause=0)�X��
    for (ptIter = m_ptVec.begin(); ptIter != m_ptVec.end(); ptIter++)
    {
        lNowX = MO_ReadLogicPosition(0);
        lNowY = MO_ReadLogicPosition(1);
        if (!g_bIsStop)
        {
            MO_Do3DLineMove((*ptIter).x - lNowX, (*ptIter).y - lNowY, 0, lWorkVelociy, lAcceleration, lInitVelociy);//����
            PreventMoveError();
        }
    }
    PauseStopGlue();//�Ȱ��ɰ����(g_bIsPause=1)
    MO_StopGumming();//����X��
    if (!g_bIsStop)
    {
        MO_Do3DLineMove(0, 0, (lZ - lZBackDistance) - lZ, lWorkVelociy, lAcceleration, lInitVelociy);//Z�b��^
        PreventMoveError();//����b�d�X��
    }
#endif
}
/*����---��R�κA(���A5����)
*��J(�_�l�Ix1,y1,�����Ix2,y2,�e��,��ݼe��,�X�ʳt��,�[�t��,��t��)
*/
void CAction::AttachFillType5(LONG lX1, LONG lY1, LONG lCenX, LONG lCenY, LONG lZ, LONG lZBackDistance, LONG lWidth, LONG lWidth2, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
#ifdef MOVE
#pragma region ****�ꫬ���ۥ\��****
    DOUBLE dRadius = 0, dWidth = 0, dWidth2 = 0, dAng0 = 0, dAng1 = 0, dAng2 = 0;
    BOOL bRev = 1;//0�f��/1����
    LONG lLineClose = 0, lXClose = 0, lYClose = 0, lDistance = 0;
    LONG lNowX = 0, lNowY = 0;
    CPoint cPt1 = 0, cPt2 = 0, cPt3 = 0, cPt4 = 0, cPtCen1 = 0, cPtCen2 = 0;
    int iData = 0, iBuff = 0;//�P�_�O�_���l�ơAbuff�Ω�p�ƨ�ݼe��
    std::vector<CPoint>m_ptVec;
    std::vector<CPoint>::iterator ptIter;//���N��
    m_ptVec.clear();
    cPt1.x = lX1;
    cPt1.y = lY1;
    cPtCen1.x = lCenX;
    cPtCen1.y = lCenY;//�W�b����
    dRadius = sqrt(pow(cPtCen1.x - cPt1.x, 2) + pow(cPtCen1.y - cPt1.y, 2));//�b�|
    lDistance = dRadius;
    dWidth = lWidth * 1000;
    dWidth2 = lWidth2 * 1000;
    dAng1 = acos((cPt1.x - cPtCen1.x) / dRadius);
    dAng2 = asin((cPt1.y - cPtCen1.y) / dRadius);
    cPt2.x = dRadius*cos(dAng1 + M_PI) + cPtCen1.x;
    cPt2.y = dRadius*sin(dAng2 + M_PI) + cPtCen1.y;
    m_ptVec.push_back(cPt1);
    m_ptVec.push_back(cPt2);
    lDistance = lDistance - dWidth;
    LineGetToPoint(lXClose, lYClose, cPt1.x, cPt1.y, cPtCen1.x, cPtCen1.y, lDistance);
    cPt3.x = lXClose;
    cPt3.y = lYClose;
    cPtCen2.x = (cPt3.x + cPt2.x) / 2;
    cPtCen2.y = (cPt3.y + cPt2.y) / 2;//�U�b����
    lDistance = lDistance + dWidth;
    iData = (int)dRadius % (int)dWidth;
    iBuff = 1;
    while (1)
    {
        if ((iBuff == 1) && (lDistance - (2 * dWidth2) < 0))
        {
            AfxMessageBox(L"��ݼe�׹L�j");
            break;
        }
        dRadius = dRadius - dWidth;
        lDistance = lDistance - dWidth;
        if (iBuff * dWidth > dWidth2)
        {
            lDistance += dWidth;
            LineGetToPoint(lXClose, lYClose, cPt1.x, cPt1.y, cPtCen1.x, cPtCen1.y, lDistance);
            cPt3.x = lXClose;
            cPt3.y = lYClose;
            m_ptVec.push_back(cPt3);
            break;
        }
        LineGetToPoint(lXClose, lYClose, cPt1.x, cPt1.y, cPtCen1.x, cPtCen1.y, lDistance);
        cPt3.x = lXClose;
        cPt3.y = lYClose;
        m_ptVec.push_back(cPt3);
        cPt4.x = dRadius*cos(dAng1 + M_PI) + cPtCen1.x;
        cPt4.y = lDistance*sin(dAng2 + M_PI) + cPtCen1.y;
        m_ptVec.push_back(cPt4);
        iBuff++;
    }
#pragma endregion
    PauseDoGlue();//�Ȱ���_���~��X��(g_bIsPause=0)�X��
    for (UINT i = 1; i < m_ptVec.size(); i++)
    {
        lNowX = MO_ReadLogicPosition(0);
        lNowY = MO_ReadLogicPosition(1);
        if (i == m_ptVec.size() - 1)
        {
            if (i % 2 == 0)
            {
                if (!g_bIsStop)
                {
                    MO_Do2DArcMove(m_ptVec.at(i).x - lNowX, m_ptVec.at(i).y - lNowY, cPtCen1.x - lNowX, cPtCen1.y - lNowY, lInitVelociy, lWorkVelociy, bRev);//�W�b��
                    PreventMoveError();
                }
            }
            else
            {
                if (!g_bIsStop)
                {
                    MO_Do2DArcMove(m_ptVec.at(i).x - lNowX, m_ptVec.at(i).y - lNowY, cPtCen2.x - lNowX, cPtCen2.y - lNowY, lInitVelociy, lWorkVelociy, bRev);//�U�b��
                    PreventMoveError();
                }
            }
        }
        else
        {
            if (i % 2 != 0)
            {
                if (!g_bIsStop)
                {
                    MO_Do2DArcMove(m_ptVec.at(i).x - lNowX, m_ptVec.at(i).y - lNowY, cPtCen1.x - lNowX, cPtCen1.y - lNowY, lInitVelociy, lWorkVelociy, bRev);//�W�b��
                    PreventMoveError();
                }
            }
            else
            {
                if (!g_bIsStop)
                {
                    MO_Do2DArcMove(m_ptVec.at(i).x - lNowX, m_ptVec.at(i).y - lNowY, cPtCen2.x - lNowX, cPtCen2.y - lNowY, lInitVelociy, lWorkVelociy, bRev);//�U�b��
                    PreventMoveError();
                }
            }
        }
    }
    PauseStopGlue();//�Ȱ��ɰ����(g_bIsPause=1)
    MO_StopGumming();//����X��
    if (!g_bIsStop)
    {
        MO_Do3DLineMove(0, 0, (lZ - lZBackDistance) - lZ, lWorkVelociy, lAcceleration, lInitVelociy);//Z�b��^
        PreventMoveError();//����b�d�X��
    }
#endif
}
/*����---��R�κA(���A6�x�ζ�R.�Ѥ��ӥ~)
*��J(�_�l�Ix1,y1,�����Ix2,y2,�e��,�X�ʳt��,�[�t��,��t��)
*/
void CAction::AttachFillType6(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lZ, LONG lZBackDistance, LONG lWidth, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
#ifdef MOVE
#pragma region ****�Ƥ諬���Y4�I�\��****
    LONG lNowX = 0, lNowY = 0;
    CPoint cPt1 = 0, cPt2 = 0, cPt3 = 0, cPt4 = 0, cPtCen = 0;
    DOUBLE dRadius = 0, dDistance = 0, dWidth = 0, dWidth2 = 0, dAngCenCos = 0, dAngCenSin = 0, dAngCos = 0, dAngSin = 0;
    DOUBLE dAngCenCos2 = 0, dAngCenSin2 = 0, dAngCos2 = 0, dAngSin2 = 0;
    std::vector<CPoint>::reverse_iterator rptIter;//�ϦV���N��
    std::vector<CPoint> m_ptVec;
    m_ptVec.clear();
    cPt1.x = lX1;
    cPt1.y = lY1;
    cPt3.x = lX2;
    cPt3.y = lX2;
    dWidth = lWidth * 1000;
    cPtCen.x = cPt1.x + (cPt3.x - cPt1.x) / 2;
    cPtCen.y = cPt1.y + (cPt3.y - cPt1.y) / 2;
    dRadius = sqrt(pow(cPt1.x - cPtCen.x, 2) + pow(cPt1.y - cPtCen.y, 2));
    dAngCenCos = acos(DOUBLE(cPt1.x - cPtCen.x) / dRadius);
    dAngCenSin = asin(DOUBLE(cPt1.y - cPtCen.y) / dRadius);
    dAngCenCos2 = M_PI * 2 - dAngCenCos;
    dAngCenSin2 = M_PI - dAngCenSin;
    if (abs(dAngCenCos - dAngCenSin) > 0.01)
    {
        if (abs(dAngCenCos - dAngCenSin2) < 0.01)
        {
            dAngCenSin = dAngCenSin2;
        }
        else if (abs(dAngCenCos2 - dAngCenSin) < 0.01)
        {
            dAngCenCos = dAngCenCos2;
        }
        else
        {
            dAngCenSin = dAngCenSin2;
            dAngCenCos = dAngCenCos2;
        }
    }
    else
    {
        dAngCenSin = dAngCenCos;
    }
    cPt2.x = dRadius*cos(dAngCenCos + M_PI_2) + cPtCen.x;
    cPt2.y = dRadius*sin(dAngCenSin + M_PI_2) + cPtCen.y;
    cPt4.x = dRadius*cos(dAngCenCos + M_PI_2 * 3) + cPtCen.x;
    cPt4.y = dRadius*sin(dAngCenSin + M_PI_2 * 3) + cPtCen.y;
    dDistance = sqrt(pow((cPt1.x - cPt4.x), 2) + pow((cPt1.y - cPt4.y), 2));
    m_ptVec.push_back(cPt1);
    m_ptVec.push_back(cPt2);
    m_ptVec.push_back(cPt3);
    m_ptVec.push_back(cPt4);

    dAngCos = acos((cPt1.x - cPt4.x) / dDistance);
    dAngSin = asin((cPt1.y - cPt4.y) / dDistance);
    if (dAngCos < 0)
    {
        dAngCos += M_PI * 2;
    }
    if (dAngSin < 0)
    {
        dAngSin += M_PI * 2;
    }
    dAngCos2 = M_PI * 2 - dAngCos;
    dAngSin2 = M_PI - dAngSin;
    if (abs(dAngCos - dAngSin) > 0.01)
    {
        if (abs(dAngCos - dAngSin2) < 0.01)
        {
            dAngSin = dAngSin2;
        }
        else if (abs(dAngCos2 - dAngSin) < 0.01)
        {
            dAngCos = dAngCos2;
        }
        else
        {
            dAngCos = dAngCos2;
            dAngSin = dAngSin2;
        }
    }
    else
    {
        dAngSin = dAngCos;
    }
    while (1)
    {
        dRadius = dRadius - dWidth*sqrt(2);
        dDistance = dDistance - dWidth;
        if (dDistance < dWidth)
        {
            break;
        }
        cPt1.x = dDistance*cos(dAngCos) + cPt4.x;
        cPt1.y = dDistance*sin(dAngSin) + cPt4.y;
        m_ptVec.push_back(cPt1);
        cPt2.x = dRadius*cos(dAngCenCos + M_PI_2) + cPtCen.x;
        cPt2.y = dRadius*sin(dAngCenSin + M_PI_2) + cPtCen.y;
        m_ptVec.push_back(cPt2);
        dDistance = dDistance - dWidth;
        if (dDistance < dWidth)
        {
            break;
        }
        cPt3.x = dRadius*cos(dAngCenCos + M_PI) + cPtCen.x;
        cPt3.y = dRadius*sin(dAngCenSin + M_PI) + cPtCen.y;
        m_ptVec.push_back(cPt3);
        cPt4.x = dRadius*cos(dAngCenCos + M_PI_2 * 3) + cPtCen.x;
        cPt4.y = dRadius*sin(dAngCenSin + M_PI_2 * 3) + cPtCen.y;
        m_ptVec.push_back(cPt4);
    }
#pragma endregion	
    for (rptIter = m_ptVec.rbegin(); rptIter != m_ptVec.rend(); rptIter++)
    {
        lNowX = MO_ReadLogicPosition(0);
        lNowY = MO_ReadLogicPosition(1);
        if (!g_bIsStop)
        {
            MO_Do3DLineMove((*rptIter).x - lNowX, (*rptIter).y - lNowY, 0, lWorkVelociy, lAcceleration, lInitVelociy);//����
            PreventMoveError();
        }
        if (rptIter == m_ptVec.rbegin())
        {
            PauseDoGlue();//�Ȱ���_���~��X��(g_bIsPause=0) �X��
        }
    }
    PauseStopGlue();//�Ȱ��ɰ����(g_bIsPause=1)
    MO_StopGumming();//����X��
    if (!g_bIsStop)
    {
        MO_Do3DLineMove(0, 0, (lZ - lZBackDistance) - lZ, lWorkVelociy, lAcceleration, lInitVelociy);//Z�b��^
        PreventMoveError();//����b�d�X��
    }
#endif
}
/*����---��R�κA(���A7������۶�R.�Ѥ��ӥ~)
*��J(�_�l�Ix1,y1,�����Ix2,y2,�e��,�X�ʳt��,�[�t��,��t��)
*/
void CAction::AttachFillType7(LONG lX1, LONG lY1, LONG lCenX, LONG lCenY, LONG lZ, LONG lZBackDistance, LONG lWidth, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
#ifdef MOVE
#pragma region ****�ꫬ���ۥ\��****
    DOUBLE dRadius = 0, dWidth = 0, dAng0 = 0, dAng1 = 0, dAng2 = 0;
    BOOL bRev = 0;//0�f��/1����
    LONG lLineClose = 0, lXClose = 0, lYClose = 0, lDistance = 0;
    LONG lNowX = 0, lNowY = 0;
    CPoint cPt1 = 0, cPt2 = 0, cPt3 = 0, cPt4 = 0, cPtCen1 = 0, cPtCen2 = 0;
    int iData = 0, iOdd = 0;//�P�_�_��(�_���W�b��/�����U�b��)
    CString csbuff = 0;
    std::vector<CPoint>m_ptVec;
    std::vector<CPoint>::reverse_iterator rptIter;//�ϦV���N��
    m_ptVec.clear();
    cPt1.x = lX1;
    cPt1.y = lY1;
    cPtCen1.x = lCenX;
    cPtCen1.y = lCenY;//�W�b����
    dRadius = sqrt(pow(cPtCen1.x - cPt1.x, 2) + pow(cPtCen1.y - cPt1.y, 2));//�b�|
    lDistance = dRadius;
    dWidth = lWidth * 1000;
    dAng1 = acos((cPt1.x - cPtCen1.x) / dRadius);
    dAng2 = asin((cPt1.y - cPtCen1.y) / dRadius);
    cPt2.x = dRadius*cos(dAng1 + M_PI) + cPtCen1.x;
    cPt2.y = dRadius*sin(dAng2 + M_PI) + cPtCen1.y;
    m_ptVec.push_back(cPt1);
    m_ptVec.push_back(cPt2);
    lDistance = lDistance - dWidth;
    LineGetToPoint(lXClose, lYClose, cPt1.x, cPt1.y, cPtCen1.x, cPtCen1.y, lDistance);
    cPt3.x = lXClose;
    cPt3.y = lYClose;
    cPtCen2.x = (cPt3.x + cPt2.x) / 2;
    cPtCen2.y = (cPt3.y + cPt2.y) / 2;//�U�b����
    iData = (int)dRadius % (int)dWidth;
    lDistance = lDistance + dWidth;
    while (1)
    {
        lDistance = lDistance - dWidth;
        if (lDistance < dWidth)
        {
            break;
        }
        LineGetToPoint(lXClose, lYClose, cPt1.x, cPt1.y, cPtCen1.x, cPtCen1.y, lDistance);
        cPt3.x = lXClose;
        cPt3.y = lYClose;
        m_ptVec.push_back(cPt3);
        cPt4.x = lDistance*cos(dAng1 + M_PI) + cPtCen1.x;
        cPt4.y = lDistance*sin(dAng2 + M_PI) + cPtCen1.y;
        m_ptVec.push_back(cPt4);
    }
#pragma endregion
    lNowX = MO_ReadLogicPosition(0);
    lNowY = MO_ReadLogicPosition(1);
    if (!g_bIsStop)
    {
        MO_Do3DLineMove(lCenX - lNowX, lCenY - lNowY, 0, lWorkVelociy, lAcceleration, lInitVelociy);//���u���ʦܶ��
        PreventMoveError();
    }
    for (rptIter = m_ptVec.rbegin(); rptIter != m_ptVec.rend(); rptIter++)
    {
        lNowX = MO_ReadLogicPosition(0);
        lNowY = MO_ReadLogicPosition(1);
        if ((iData != 0) && (iOdd == 0))
        {
            if (!g_bIsStop)
            {
                MO_Do3DLineMove((*rptIter).x - lNowX, (*rptIter).y - lNowY, 0, lWorkVelociy, lAcceleration, lInitVelociy);//���u����
                PreventMoveError();
            }
            PauseDoGlue();//�Ȱ���_���~��X��(g_bIsPause=0) �X��
        }
        else if ((iData == 0) && (iOdd == 0))
        {
            if (!g_bIsStop)
            {
                MO_Do2DArcMove((*rptIter).x - lNowX, (*rptIter).y - lNowY, cPtCen2.x - lNowX, cPtCen2.y - lNowY, lInitVelociy, lWorkVelociy, bRev);//��l�b��
                PreventMoveError();
            }
            PauseDoGlue();//�Ȱ���_���~��X��(g_bIsPause=0) �X��
        }
        else
        {
            if (iOdd % 2 == 0)
            {
                if (!g_bIsStop)
                {
                    MO_Do2DArcMove((*rptIter).x - lNowX, (*rptIter).y - lNowY, cPtCen2.x - lNowX, cPtCen2.y - lNowY, lInitVelociy, lWorkVelociy, bRev);//��l�b��
                    PreventMoveError();
                }
            }
            else
            {
                if (!g_bIsStop)
                {
                    MO_Do2DArcMove((*rptIter).x - lNowX, (*rptIter).y - lNowY, lCenX - lNowX, lCenY - lNowY, lInitVelociy, lWorkVelociy, bRev);//��l�b��
                    PreventMoveError();
                }
            }
        }
        iOdd++;
    }
    PauseStopGlue();//�Ȱ��ɰ����(g_bIsPause=1)
    MO_StopGumming();//����X��
    if (!g_bIsStop)
    {
        MO_Do3DLineMove(0, 0, (lZ - lZBackDistance) - lZ, lWorkVelociy, lAcceleration, lInitVelociy);//Z�b��^
        PreventMoveError();//����b�d�X��
    }
#endif
}





