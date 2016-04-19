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
    g_bIsDispend = TRUE;
    g_iNumberGluePort = 1;
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
#ifdef MOVE
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
/*
*�u�q�}�l�ʧ@
*��J(�u�q�}�l�B�u�q�I���]�w�B�[�t�סB�u�t�סB�t�ιw�]�Ѽ�)
*/
void CAction::DecideLineStartMove(LONG lX, LONG lY, LONG lZ, LONG lStartDelayTime, LONG lStartDistance,
    LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
    /*�u�q�}�l(x�y�СAy�y�СAz�y�СA�u�q�_�l�I�A)
    LONG lX, LONG lY, LONG lZ
    */
    /*�u�q�I���]�w(���ʫe����A�]�m�Z���A)
    LONG lStartDelayTime ,LONG lStartDistance
    */
    //1.���ʫe�I�����b�@���u�q�_�l�I�B�O�����}���ɪ��C �����ɥi����w�Y�b�y��y�ʤ��e�u�u�q�o�Ͳ��ʡC
    //2. �I�����}�ҫe�A ���F���}���u�u�q�_�l�I�����ʶZ���C �ӶZ�������F���ѤF�������_�t�ɶ��A�D�n�ΨӮ����L�q�y��b�u�q�_�l�B���n�E�C
#ifdef MOVE
    LONG lNowX = 0, lNowY = 0;
    LONG lXClose = 0, lYClose = 0, lLineClose = 0;
    int iBuf = 0;
    lNowX = MO_ReadLogicPosition(0);
    lNowY = MO_ReadLogicPosition(1);
    lLineClose = lStartDistance;
    LineGetToPoint(lXClose, lYClose, lNowX, lNowY, lX, lY, lLineClose);
    iBuf = lStartDelayTime ^ lStartDistance;//�����β��ʫe�����]�m�Z�� �p�G��̳��ۦP���G��0(���̳����ȮɥH"���ʫe����"�u��)
    AttachPointMove(lX, lY, 0, lWorkVelociy, lAcceleration, lInitVelociy);//�ϥΪ��ݰ���
    AttachPointMove(0, 0, lZ, lWorkVelociy, lAcceleration, lInitVelociy);//�ϥΪ��ݰ���
    PreventMoveError();//����b�d�X��
    if (iBuf == 0)
    {
        if (g_bIsDispend == 1)
        {
            MO_GummingSet(g_iNumberGluePort,0, GummingTimeOutThread);//�(���d)
        }
        MO_Timer(0, 0, lStartDelayTime * 1000);
        MO_Timer(1, 0, lStartDelayTime * 1000);//�u�q�I���]�w---(1)���ʫe����(�b�u�q�}�l�I�W)
        Sleep(1);//����X���A�קK�p�ɾ��쪽��0
        while (MO_Timer(3, 0, 0))
        {
            Sleep(1);
        }
    }
    else
    {
        if (lStartDistance == 0)
        {
            if (g_bIsDispend == 1)
            {
                MO_GummingSet(g_iNumberGluePort,0, GummingTimeOutThread);//�(���d)
            }
            MO_Timer(0, 0, lStartDelayTime * 1000);
            MO_Timer(1, 0, lStartDelayTime * 1000);//�u�q�I���]�w---(1)���ʫe����(�b�u�q�}�l�I�W)
            Sleep(1);//����X���A�קK�p�ɾ��쪽��0
            while (MO_Timer(3, 0, 0))
            {
                Sleep(1);
            }
        }
        else
        {
            if (!g_bIsStop)
            {
                MO_Do3DLineMove(lXClose, lYClose, 0, lWorkVelociy, lAcceleration, lInitVelociy); //�u�q�I���]�w---(2)�]�m�Z��
                PreventMoveError();//����b�d�X��
                if (g_bIsDispend == 1)
                {
                    MO_GummingSet(g_iNumberGluePort,0, GummingTimeOutThread);//�(���d)
                }
            }
        }
    }
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
    if (!g_bIsStop)
    {
        AttachPointMove(lX, lY, lZ, lWorkVelociy, lAcceleration, lInitVelociy);//�ϥΪ��ݰ���
        PreventMoveError();//����b�d�X��
    }
    MO_Timer(0, 0, lMidDelayTime * 1000);
    MO_Timer(1, 0, lMidDelayTime * 1000);//�u�q�I���]�w---(4)�`�I�ɶ�
    Sleep(1);//����X���A�קK�p�ɾ��쪽��0
    while (MO_Timer(3, 0, 0))
    {
        Sleep(1);
    }
#endif
}
/*
*�u�q�����ʧ@
*��J(�u�q�����I�B�u�q�I���]�w�B�[�t�סB�u�t�סBZ�b�u�@���׳]�w�B�I�������]�w�B��^�]�w�B�t�ΰѼ�)
*/
void CAction::DecideLineEndMove(LONG lX, LONG lY, LONG lZ, LONG lCloseOffDelayTime, LONG lCloseDistance,
    LONG lCloseONDelayTime, LONG lZBack, LONG lHighVelocity, LONG lDistance, LONG lHigh,
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
    LONG lZBack
    */
    /*�I�������]�w(���t��)
    LONG lHighVelocity
    */
#ifdef MOVE
    LONG lNowX = 0, lNowY = 0, lNowZ = 0;
    LONG lLineClose = 0, lXClose = 0, lYClose = 0;
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
            Sleep(1);
        }
    }
    else
    {
        if (lCloseDistance == 0) //�u�q�I���]�w---(5)�����Z��
        {
            AttachPointMove(lX, lY, lZ, lWorkVelociy, lAcceleration, lInitVelociy);//�ϥΪ��ݰ���
            PreventMoveError();//����b�d�X��
            MO_Timer(0, 0, lCloseONDelayTime * 1000);
            MO_Timer(1, 0, lCloseONDelayTime * 1000);//�u�q�I���]�w---(6)��������  
            Sleep(1);//����X���A�קK�p�ɾ��쪽��0
            while (MO_Timer(3, 0, 0))
            {
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
    GelatinizeBack(iType, lX, lY, lZ, lNowX, lNowY, lDistance, lHigh, lZBack, lLowVelocity, lHighVelocity, lAcceleration, lInitVelociy);//��^�]�w
    MO_Timer(0, 0, lCloseOffDelayTime * 1000);
    MO_Timer(1, 0, lCloseOffDelayTime * 1000);//�u�q�I���]�w---(3)���d�ɶ� 
    Sleep(1);//����X���A�קK�p�ɾ��쪽��0
    while (MO_Timer(3, 0, 0))
    {
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
    LONG lCircleX = 0, lCircleY = 0, lRev = 0;
    LONG lNowX = 0, lNowY = 0;
    CString csX = 0, csY = 0;
    CString csBuff = 0, csNowPonit = 0, csLineCircle = 0, csLineCircleEnd = 0;
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
    lRev = CStringToLong(csBuff, 2);//���o���(X�AY�ARev�A)
    if (!g_bIsStop)
    {
        MO_Do2DArcMove(0, 0, lCircleX - lNowX, lCircleY - lNowY, lInitVelociy, lWorkVelociy, lRev);
        PreventMoveError();//����b�d�X��
    }
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
    LONG lCircleX = 0, lCircleY = 0, lRev = 0;
    LONG lNowX = 0, lNowY = 0;
    CString csX = 0, csY = 0;
    CString csBuff = 0, csNowPonit = 0, csLineCircle = 0, csLineCircleEnd = 0;
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
    lRev = CStringToLong(csBuff, 2);//���o���(X�AY�ARev�A)
    if (!g_bIsStop)
    {
        MO_Do2DArcMove(lX2 - lNowX, lY2 - lNowY, lCircleX - lNowX, lCircleY - lNowY, lInitVelociy, lWorkVelociy, lRev);
        PreventMoveError();//����b�d�X��
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
    AttachPointMove(lX, lY, lZ, lWorkVelociy, lAcceleration, lInitVelociy);//�ϥΪ��ݰ���
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
    AttachPointMove(lX, lY, lZ, lWorkVelociy, lAcceleration, lInitVelociy);//�ϥΪ��ݰ���
    PreventMoveError();//����b�d�X��
    MO_Timer(0, 0, lWaitTime * 1000);
    MO_Timer(1, 0, lWaitTime * 1000);//���ݮɶ�(us��ms)
    Sleep(1);//����X���A�קK�p�ɾ��쪽��0
    while (MO_Timer(3, 0, 0))
    {
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
    AttachPointMove(lX, lY, lZ, lWorkVelociy, lAcceleration, lInitVelociy);//�ϥΪ��ݰ���
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
        MO_MoveToHome(lSpeed1, lSpeed2, 7, 0);
        PreventMoveError();//����b�d�X��
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
/***************************************************************************************************************�ۦ�B�Ψ��*/
/*
*����---�����I�ʧ@
*��J(�I�B�X�ʳt�סB�[�t�סB��t��)
*/
void CAction::AttachPointMove(LONG lX, LONG lY, LONG lZ, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
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
    if (!g_bIsStop)
    {
        MO_Do3DLineMove(lX - lNowX, lY - lNowY, lZ - lNowZ, lWorkVelociy, lAcceleration, lInitVelociy);
        PreventMoveError();//����b�d�X��
    }
#endif 
}
/*�X��(�t�X������ϥ�)
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
            Sleep(1);
        }
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
/*���u�Z���ഫ���y���I*/
void CAction::LineGetToPoint(LONG &lXClose, LONG &lYClose, LONG lX0, LONG lY0, LONG lX1, LONG lY1, LONG &lLineClose)
{
    LONG lM, lB; //(X0,Y0)  �_�l�I;(X1,Y1) �����I; Y0=M*X0+B �ײv����;L�w������
    if ((lX1 - lX0) == 0)
    {
        lM = 0;
    }
    else
    {
        lM = (lY1 - lY0) / (lX1 - lX0);
    }
    lB = lY0 - (lM*lX0);
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
                lYClose = (lM*lX1) + lB;
            }
            else
            {
                lXClose = lX1 + lLineClose;
                lYClose = (lM*lX1) + lB;
            }
        }
        else if (abs(la) == abs(lb))
        {
            if (la > 0)
            {
                lXClose = lX1 - lLineClose;
                lYClose = (lM*lX1) + lB;
            }
            else
            {
                lXClose = lX1 + lLineClose;
                lYClose = (lM*lX1) + lB;
            }
        }
        else if (abs(la) < abs(lb))
        {
            if (lb > 0)
            {
                lYClose = lY1 - lLineClose;
                lXClose = (lYClose - lB) / lM;
            }
            else
            {
                lYClose = lY1 + lLineClose;
                lXClose = (lYClose - lB) / lM;
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
/*�r��������
�ϥΪ������@�w�n�O"�A"
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





