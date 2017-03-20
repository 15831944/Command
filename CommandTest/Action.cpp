/*
*�ɮצW��:Action(W�b��)
*���e²�z:�B�ʩR�OAPI�A�ԲӰѼƽЬd��excel
*�Iauthor �@�̦W��:R
*�Idata ��s���:2017/3/17
*@��s���e:nova�b�d�ϥΦb�|�b�I�����W*/
#include "stdafx.h"
#include "Action.h"
#define LA_SCANEND -99999
BOOL CAction::m_ZtimeOutGlueSet = FALSE;
BOOL CAction::m_YtimeOutGlueSet = FALSE;
#define _USE_MATH_DEFINES
#include <math.h>
//**************���_�ϥζ���*********************
//�����n���i��s��(�u���s���@��)     MO_InterruptCase(0, NULL, mo.MoInterrupt,mo.pAction);//���_�s��
/***********************************************************
**                                                        **
**          �B�ʼҲ�-�B�ʫ��O�R�O (�����ʧ@�P�_)             **
**                                                        **
************************************************************/
CAction::CAction()
{
    // �ݫŧi��l��
    pAction = this;
    m_hComm = NULL;
    m_bIsPause = FALSE;//�Ȱ�
    m_bIsStop = FALSE;//����
    m_bIsGetLAend = FALSE;//���y�����_
    m_bIsDispend = TRUE;//�I�����}
    m_interruptLock = FALSE;//���_��
    m_OffSetLaserX = 0;//�p�g�����q��l��X
    m_OffSetLaserY = 0;//�p�g�����q��l��Y
    m_OffSetLaserZ = 0;//�p�g�����q��l��Z
    m_HeightLaserZero = 0;//�p�g��Z�b�k�s�����ᰪ��(���y����)
    m_LaserCnt = 0;//�p�g�u�q�p�ƾ�(�ϥΦ���)
    m_OffSetScan = 500;//�p�g�첾���v
    m_LaserAverage = FALSE; //�p�g����(1�ϥ�/0���ϥ�)
    m_LaserAveBuffZ = 0; // �p�g�Υ����Ȧs��(�����mz��)
    m_TablelZ = 100068;//�u�@���x����(Z�b�`�u�@����)
    m_MachineCirMid.x = 0;//����P�߶�y��x
    m_MachineCirMid.y = 0;//����P�߶�y��y
    m_MachineOffSet.x = -99999;//����P�߶갾���qx(�w�]����l�Ƭ�-99999)
    m_MachineOffSet.y = -99999;//����P�߶갾���qy(�w�]����l�Ƭ�-99999)
    m_Wangle = 0;//����ե�����w
    m_MachineCirRad = 0;//����P�߶�b�|
    WangBuff = 0;//w�b����buff
    cpCirMidBuff[0] = { 0 };
    cpCirMidBuff[1] = { 0 };
    m_ThreadFlag = 0;//������X��(��ܰʧ@��)
    m_IsCutError = FALSE;//���ȿ��~
    m_HomingOffset_INIT.x = 200000;   //���I�����qX(����y��)
    m_HomingOffset_INIT.y = 200000;   //���I�����qY(����y��)
    m_HomingOffset_INIT.z = 10000;    //���I�����qZ(����y��)
    m_HomingOffset_INIT.w = 0;      //���I�����qW(����y��)
	//����y�Ь����x������y�СA�ɱר��׬�90��
    m_HomingPoint.x = 92000;   //���I�_�k�IX(����y��)
    m_HomingPoint.y = 96000;   //���I�_�k�IY(����y��)
    m_HomingPoint.z = 10000;    //���I�_�k�IZ(����y��)
    m_HomingPoint.w = 0;        //���I�_�k�IW(����y��)
	m_HomeSpeed_DEF = 15000;	//���I�_�k�w�]�t��(Z,W�b)
	m_WSpeed = 6.0;				//W�b�t�׽վ��ܼ�(1~10)
	m_IsHomingOK = 0;           //���I�_�k�Ѽ�
#ifdef MOVE
    W_m_ptVec.clear();//W��ƪ�Ϥ�
    LA_m_ptVec2D.clear();
    LA_m_ptVec.clear();
#endif
}
CAction::~CAction()
{
}
CString CAction::ShowVersion()
{
    CString csbuf, csa, csb, csc, csd;
#ifdef MOVE
    MO_DllVersion(csa);
    MO_DllDate(csb);
#ifdef LA
    LAS_DllVersion(csc);
    LAS_DllDate(csd);
#endif
    csbuf.Format(_T("�B�ʪ���=%s�B�B�ʮɶ�=%s\n�p�g����=%s�B�p�g�ɶ�=%s\n"), csa, csb, csc, csd);
#endif
    return csbuf;
}

//���ݮɶ�
void CAction::WaitTime(HANDLE wakeEvent, int Time)
{
    if(!m_bIsStop)
    {
        DWORD rc = WaitForSingleObject(wakeEvent, Time);
        switch(rc)
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
}
/*************�B��API***************************************************************************************************/
/*
*���I�I���ʧ@(�h��-�����]�m+�[�t��+�I���I�t��)
*��J(���I�I���B���I�I���]�w�BZ�b�u�@���׳]�w�B�I�������]�w�B�[�t�סB�]�I���I�^�t�סB�t�ιw�]�Ѽ�)
*/
void CAction::DecidePointGlue(LONG lX, LONG lY, LONG lZ,DOUBLE dAng, LONG lDoTime, LONG lDelayStopTime,
                              LONG lZBackDistance, BOOL bZDisType, LONG lZdistance, LONG lHighVelocity, LONG lLowVelocity, LONG lWorkVelociy,
                              LONG lAcceleration, LONG lInitVelociy)
{
    /*���I�I��
    LONG lX, LONG lY, LONG lZ
    */
    /*W�b����
    DOUBLE dAng
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
    //�P�_W�b�O�_�ե�
    if(m_MachineOffSet.x == -99999 && m_MachineOffSet.y == -99999)
    {
        return;
    }
    //�P�_�ϥΰw�Y�y���٬O����y��
    if(USEMODE_W == 1) //�ϥξ���y��
    {
        CPoint cpbuf = W_GetNeedlePoint(lX, lY, dAng, 1);
        lX = cpbuf.x;
        lY = cpbuf.y;
    }
    /*******�H�U�{�����|���ܨϥΰw�Y�y�а���B��************************/
    //�y�{: ��J�w�Y�y��>�B��w�Y�y��>�ഫ������y�п�X
    if(lHighVelocity == 0)
    {
        lHighVelocity = lWorkVelociy;
    }
    if(lLowVelocity == 0)
    {
        lLowVelocity = lWorkVelociy;
    }
    if(bZDisType) //��ϥά۹��m���ɭ�
    {
        LONG lbuf = m_TablelZ - lZBackDistance;//�q���x�̧C���W�۹�Z��
        lZBackDistance = lbuf;
        bZDisType = 0;//�אּ�����m
    }
    if(!bZDisType)  //�����m
    {
        if(lZBackDistance > lZ)
        {
            lZBackDistance = lZ;
        }
        lZBackDistance = abs(lZBackDistance - lZ);
    }
    if(lZBackDistance>lZ)
    {
        lZBackDistance = lZ;
    }
    //W_Rotation(dAng, lWorkVelociy, lAcceleration, lInitVelociy);//w�b����
    if(!m_bIsStop)
    {
        MO_Do4DLineMove(0, 0, 0, dAng-MO_ReadLogicPositionW(), (LONG)round(lWorkVelociy/ m_WSpeed), (LONG)(lAcceleration/ m_WSpeed), lInitVelociy);//����W
        PreventMoveError();//����b�d�X��
    }
    CPoint cpRobot=W_GetMachinePoint(lX,lY,dAng,1) ;//�����I������y��
    if(!m_bIsStop)
    {
        MO_Do3DLineMove(cpRobot.x - MO_ReadLogicPosition(0), cpRobot.y - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration, lInitVelociy);//���ʨ��I���I
        PreventMoveError();//����b�d�X��
    }
    if(!m_bIsStop)
    {
        MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//���ʨ��I���I
        PreventMoveError();//����b�d�X��
    }
    if(!m_bIsStop)
    {
        if(lDoTime == 0)
        {
            Sleep(1);
        }
        else
        {
            DoGlue(lDoTime, lDelayStopTime);//�����I��
        }
    }
    if(lZdistance == 0)
    {
        if(!m_bIsStop)
        {
            MO_Do3DLineMove(0, 0, (lZ - lZBackDistance) - lZ, lWorkVelociy, lAcceleration, lInitVelociy);//�I�������]�m
            PreventMoveError();//����b�d�X��
        }
    }
    else
    {
        if(lZBackDistance == 0)
        {
            lZdistance = lZBackDistance;
        }
        if(lZdistance>lZBackDistance && lZBackDistance != 0)
        {
            lZdistance = lZBackDistance;
            MO_Do3DLineMove(0, 0, (lZ - lZBackDistance) - lZ, lHighVelocity, lAcceleration, lInitVelociy);//�I�������]�m
            PreventMoveError();//����b�d�X��
        }
        else
        {
            if(!m_bIsStop)
            {
                MO_Do3DLineMove(0, 0, (lZ - lZdistance) - lZ, lLowVelocity, lAcceleration, lInitVelociy);//�I�������]�m
                PreventMoveError();//����b�d�X��
            }
            if(!m_bIsStop)
            {
                MO_Do3DLineMove(0, 0, (lZ - lZBackDistance) - (lZ - lZdistance), lHighVelocity, lAcceleration, lInitVelociy);//�I�������]�m
                PreventMoveError();//����b�d�X��
            }
        }

    }
#endif
}
/*
*�u�q�}�l�ʧ@
*��J(�u�q�}�l�B�u�q�I���]�w�B�[�t�סB�u�t�סB�t�ιw�]�Ѽ�)
*/
void CAction::DecideLineStartMove(LONG lX, LONG lY, LONG lZ, DOUBLE dAng, LONG lStartDelayTime,LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy, BOOL bIsNeedleP)
{
    /*�u�q�}�l(x�y�СAy�y�СAz�y�СA�u�q�_�l�I�A)
    LONG lX, LONG lY, LONG lZ
    */
    /*W�b����
    DOUBLE dAng
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
    //�P�_W�b�O�_�ե�
    if(m_MachineOffSet.x == -99999 && m_MachineOffSet.y == -99999)
    {
        return;
    }
    //�P�_�ϥΰw�Y�y���٬O����y��
    if(USEMODE_W == 1 && bIsNeedleP == 0) //�ϥξ���y��
    {
        CPoint cpbuf = W_GetNeedlePoint(lX, lY, dAng, 1);
        lX = cpbuf.x;
        lY = cpbuf.y;
    }
    /*******�H�U�{�����|���ܨϥΰw�Y�y�а���B��************************/
    //�y�{: ��J�w�Y�y��>�B��w�Y�y��>�ഫ������y�п�X
    //W_Rotation(dAng, lWorkVelociy, lAcceleration, lInitVelociy);//w�b����
    if(!m_bIsStop)
    {
        MO_Do4DLineMove(0,0, 0, dAng - MO_ReadLogicPositionW(), (LONG)round(lWorkVelociy / m_WSpeed), (LONG)(lAcceleration / m_WSpeed), lInitVelociy);//���ʨ��I���I
        PreventMoveError();//����b�d�X��
    }
    CPoint cpRobot = W_GetMachinePoint(lX, lY, dAng, 1);//�����I������y��
    if(!m_bIsStop)
    {
        MO_Do3DLineMove(cpRobot.x - MO_ReadLogicPosition(0), cpRobot.y - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration, lInitVelociy);//���ʨ��I���I
        PreventMoveError();//����b�d�X��
    }
    if(!m_bIsStop)
    {
        MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//x,y,z�b����
        PreventMoveError();//����b�d�X��
    }
    if(!m_bIsStop && m_bIsDispend == 1)
    {
        MO_GummingSet();//�(���d)
    }
    MO_Timer(0, 0, lStartDelayTime * 1000);
    MO_Timer(1, 0, lStartDelayTime * 1000);//�u�q�I���]�w---(1)���ʫe����(�b�u�q�}�l�I�W)
    while(MO_Timer(3, 0, 0))
    {
        if(m_bIsStop)
        {
            break;
        }
        Sleep(1);
    }
    PauseStopGlue();//�Ȱ��ɰ����(m_bIsPause=1)
#endif
}
/*
*�u�q���I�ʧ@
*��J(�u�q���I�B�u�q�I���]�w�B�[�t�סB�u�t�סB�t�ιw�]�Ѽ�)
*/
void CAction::DecideLineMidMove(LONG lX, LONG lY, LONG lZ, DOUBLE dAng, LONG lMidDelayTime, LONG lWorkVelociy,LONG lAcceleration, LONG lInitVelociy, BOOL bIsNeedleP)
{
    /*�u�q���I(x�y�СAy�y�СAz�y�СA�u�q�_�l�I�A)
    LONG lX, LONG lY, LONG lZ
    */
    /*W�b����
    DOUBLE dAng
    */
    /*�u�q�I���]�w(�`�I�ɶ�)
    LONG lMidDelayTime
    */
#ifdef MOVE
    //�P�_W�b�O�_�ե�
    if(m_MachineOffSet.x == -99999 && m_MachineOffSet.y == -99999)
    {
        return;
    }
    //�P�_�ϥΰw�Y�y���٬O����y��
    if(USEMODE_W == 1 && bIsNeedleP == 0) //�ϥξ���y��
    {
        CPoint cpbuf = W_GetNeedlePoint(lX, lY, dAng, 1);
        lX = cpbuf.x;
        lY = cpbuf.y;
    }
    /*******�H�U�{�����|���ܨϥΰw�Y�y�а���B��************************/
    //�y�{: ��J�w�Y�y��>�B��w�Y�y��>�ഫ������y�п�X
    CPoint cpNeedlePoint = W_GetNeedlePoint();//�N��U��m�ഫ���w�Y�y��
    W_UpdateNeedleMotor_Needle(cpNeedlePoint.x, cpNeedlePoint.y, MO_ReadLogicPosition(2), MO_ReadLogicPositionW(), lX, lY, lZ, dAng);//���I����(�ϥιw�]�C1�ר��@���I)
    PauseDoGlue();//�Ȱ���_���~��X��(m_bIsPause=0) �X��
    if(!m_bIsStop)
    {
        W_Line4DtoDo(lWorkVelociy, lAcceleration, lInitVelociy);//�|�b�s�򴡸�
        PreventMoveError();//����b�d�X��
    }
	CPoint cpMachinePoint = W_GetMachinePoint(lX, lY, dAng, 1);
	lX = cpMachinePoint.x;
	lY = cpMachinePoint.y;
	//_cwprintf(_T("%s \n"), _T("���v�ץ�"));
	//if (!m_bIsStop)
	//{
	//	MO_Do4DLineMove(lX - MO_ReadLogicPosition(0), lY - MO_ReadLogicPosition(1), lZ - MO_ReadLogicPosition(2),dAng- MO_ReadLogicPositionW(), lInitVelociy, lWorkVelociy, lInitVelociy);
	//	PreventMoveError();
	//}
    MO_Timer(0, 0, lMidDelayTime * 1000);
    MO_Timer(1, 0, lMidDelayTime * 1000);//�u�q�I���]�w---(4)�`�I�ɶ�
    Sleep(1);//����X���A�קK�p�ɾ��쪽��0
    while(MO_Timer(3, 0, 0))
    {
        if(m_bIsStop)
        {
            break;
        }
        Sleep(1);
    }
    PauseStopGlue();//�Ȱ��ɰ����(m_bIsPause=1)
#endif
}
/*
*�u�q�����ʧ@
*��J(�u�q�����I�B�u�q�I���]�w�B�[�t�סB�u�t�סBZ�b�u�@���׳]�w�B�I�������]�w�B��^�]�w�B�t�ΰѼ�)
*/
void CAction::DecideLineEndMove(LONG lX, LONG lY, LONG lZ, DOUBLE dAng, LONG lCloseOffDelayTime, LONG lCloseDistance,
                                LONG lCloseONDelayTime, LONG lZBackDistance, BOOL bZDisType, LONG lHighVelocity, LONG lDistance, LONG lHigh,
                                LONG lLowVelocity, int iType, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
    /*�u�q�����I(x�y�СAy�y�СAz�y�СA�u�q�_�l�I�A)
    LONG lX, LONG lY, LONG lZ
    */
    /*W�b����
    DOUBLE dAng
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
    //�P�_W�b�O�_�ե�
    if(m_MachineOffSet.x == -99999 && m_MachineOffSet.y == -99999)
    {
        return;
    }
    //�P�_�ϥΰw�Y�y���٬O����y��
    if(USEMODE_W == 1) //�ϥξ���y��
    {
        CPoint cpbuf = W_GetNeedlePoint(lX, lY, dAng, 1);
        lX = cpbuf.x;
        lY = cpbuf.y;
    }
    /*******�H�U�{�����|���ܨϥΰw�Y�y�а���B��************************/
    //�y�{: ��J�w�Y�y��>�B��w�Y�y��>�ഫ������y�п�X
    if(bZDisType) //��ϥά۹��m���ɭ�
    {
        LONG lbuf = m_TablelZ - lZBackDistance;//�q���x�̧C���W�۹�Z��
        lZBackDistance = lbuf;
        bZDisType = 0;//�אּ�����m
    }
    if(!bZDisType)  //�����m
    {
        if(lZBackDistance > lZ)
        {
            lZBackDistance = lZ;
        }
        lZBackDistance = abs(lZBackDistance - lZ);
    }
    LONG lLineClose = 0, lXClose = 0, lYClose = 0, lZClose = 0, LineLength = 0;
    PauseDoGlue();//�Ȱ���_���~��X��(m_bIsPause=0)
    CPoint cpNeedlePoint = W_GetNeedlePoint();//�N��U��m�ഫ���w�Y�y��
    LineLength = LONG(sqrt(DOUBLE(pow(lX - cpNeedlePoint.x, 2) + pow(lY - cpNeedlePoint.y, 2)+ pow(lZ - MO_ReadLogicPosition(2), 2))));
    if(lCloseDistance != 0 && lCloseDistance > LineLength)
    {
        lCloseDistance = LineLength;
    }
    if(lHighVelocity == 0)
    {
        lHighVelocity = lWorkVelociy;
    }
    if(lLowVelocity == 0)
    {
        lLowVelocity = lWorkVelociy;
    }
    if(lX == cpNeedlePoint.x && lY == cpNeedlePoint.y) //�w�g�b�����I�W���������Z��
    {
        MO_Timer(0, 0, lCloseONDelayTime * 1000);
        MO_Timer(1, 0, lCloseONDelayTime * 1000);//�u�q�I���]�w---(6)��������
        Sleep(1);//����X���A�קK�p�ɾ��쪽��0
        while(MO_Timer(3, 0, 0))
        {
            if(m_bIsStop)
            {
                break;
            }
            Sleep(1);
        }
        MO_StopGumming();//����X��
        return;
    }
    else
    {
        if(lCloseDistance == 0)  //�u�q�I���]�w---(5)�����Z��
        {
            W_UpdateNeedleMotor_Needle(cpNeedlePoint.x, cpNeedlePoint.y, MO_ReadLogicPosition(2), MO_ReadLogicPositionW(), lX, lY, lZ, dAng);//���I����(�ϥιw�]�C1�ר��@���I)
            if(!m_bIsStop)
            {
                W_Line4DtoDo(lWorkVelociy, lAcceleration, lInitVelociy);//�|�b�s�򴡸�
                PreventMoveError();//����b�d�X��
            }
            MO_Timer(0, 0, lCloseONDelayTime * 1000);
            MO_Timer(1, 0, lCloseONDelayTime * 1000);//�u�q�I���]�w---(6)��������
            Sleep(1);//����X���A�קK�p�ɾ��쪽��0
            while(MO_Timer(3, 0, 0))
            {
                if(m_bIsStop == 1)
                {
                    break;
                }
                Sleep(1);
            }
            MO_StopGumming();//����X��
        }
        else
        {
            W_UpdateNeedleMotor_Needle(cpNeedlePoint.x, cpNeedlePoint.y, MO_ReadLogicPosition(2), MO_ReadLogicPositionW(), lX, lY, lZ, dAng);//���I����(�ϥιw�]�C1�ר��@���I)
            //�ϥ�(5)�����Z��(lCloseDistance)
            LONG sumPath = lCloseDistance;
            LONG finishTime = 0;
            DOUBLE avgTime = 0;
            LONG accLength = CalPreglue(lWorkVelociy, lAcceleration, lInitVelociy);
            avgTime = ((DOUBLE)sumPath - (DOUBLE)accLength) / (DOUBLE)lWorkVelociy;
            finishTime = (LONG)round(avgTime * 1000000) + CalPreglueTime(lWorkVelociy, lAcceleration, lInitVelociy);
            LONG closeDistTime = CalPreglue(lCloseDistance, lWorkVelociy, 0, lInitVelociy);
            /*======�p�ɾ���Ĳ�o���_�����_���A�ϥ�z���_����================*/
            if(!m_bIsStop)
            {
                CAction::m_ZtimeOutGlueSet = FALSE;
                MO_TimerSetIntter(finishTime - closeDistTime, 1);
            }
            if(!m_bIsStop)
            {
                W_Line4DtoDo(lWorkVelociy, lAcceleration, lInitVelociy);//�|�b�s�򴡸�
                PreventMoveError();//����b�d�X��
            }
        }
    }
    CPoint cpBuf = W_GetNeedlePoint(lX, lY, dAng, 1);
    GelatinizeBack(iType, cpBuf.x, cpBuf.y, lZ, cpNeedlePoint.x, cpNeedlePoint.y, lDistance, lHigh, lZBackDistance, lLowVelocity, lHighVelocity, lAcceleration, lInitVelociy);//��^�]�w
    MO_Timer(0, 0, lCloseOffDelayTime * 1000);
    MO_Timer(1, 0, lCloseOffDelayTime * 1000);//�u�q�I���]�w---(3)���d�ɶ�
    Sleep(1);//����X���A�קK�p�ɾ��쪽��0
    while(MO_Timer(3, 0, 0))
    {
        if(m_bIsStop == 1)
        {
            break;
        }
        Sleep(1);
    }
#endif
}

/*
*(�{�b��m�b�����I�W)�u�q�����ʧ@--�h��
*��J(�u�q�����I�B�u�q�I���]�w�B�[�t�סB�u�t�סBZ�b�u�@���׳]�w�B�I�������]�w�B��^�]�w�B�t�ΰѼơB�@�뵲���I�ϥ�)
*����Z�b�^�ɫ��A1
*/
void CAction::DecideLineEndMove(LONG lCloseOffDelayTime,
                                LONG lCloseONDelayTime, LONG lZBackDistance, BOOL bZDisType, LONG lHighVelocity, LONG lDistance, LONG lHigh,
                                LONG lLowVelocity, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy, BOOL bGeneral)
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
    /*
    BOOL bGeneral �Ω󤣶�X�����u�q�ϥ�(1�ϥ�/0���ϥ�)
    */
#ifdef MOVE
    //�P�_W�b�O�_�ե�
    if(m_MachineOffSet.x == -99999 && m_MachineOffSet.y == -99999)
    {
        return;
    }
    LONG lLineClose = 0, lXClose = 0, lYClose = 0;
    if(!bGeneral)
    {
        PauseDoGlue();//�Ȱ���_���~��X��(m_bIsPause=0)
    }
    if(bZDisType) //��ϥά۹��m���ɭ�
    {
        LONG lbuf = m_TablelZ - lZBackDistance;//�q���x�̧C���W�۹�Z��
        lZBackDistance = lbuf;
        bZDisType = 0;//�אּ�����m
    }
    if(!bZDisType)  //�����m
    {
        if(lZBackDistance > MO_ReadLogicPosition(2))
        {
            lZBackDistance = MO_ReadLogicPosition(2);
        }
        lZBackDistance = abs(lZBackDistance - MO_ReadLogicPosition(2));
    }
    if(lHighVelocity == 0)
    {
        lHighVelocity = lWorkVelociy;
    }
    if(lLowVelocity == 0)
    {
        lLowVelocity = lWorkVelociy;
    }
    MO_Timer(0, 0, lCloseONDelayTime * 1000);
    MO_Timer(1, 0, lCloseONDelayTime * 1000);//�u�q�I���]�w---(6)��������
    Sleep(1);//����X���A�קK�p�ɾ��쪽��0
    while(MO_Timer(3, 0, 0))
    {
        if(m_bIsStop == 1)
        {
            break;
        }
        Sleep(1);
    }
    if(!bGeneral || MO_ReadGumming())
    {
        MO_StopGumming();//����X��
    }
    GelatinizeBack(1, MO_ReadLogicPosition(0), MO_ReadLogicPosition(1), MO_ReadLogicPosition(2), MO_ReadLogicPosition(0), MO_ReadLogicPosition(1), lDistance, lHigh, lZBackDistance, lLowVelocity, lHighVelocity, lAcceleration, lInitVelociy);//��^�]�w
    MO_Timer(0, 0, lCloseOffDelayTime * 1000);
    MO_Timer(1, 0, lCloseOffDelayTime * 1000);//�u�q�I���]�w---(3)���d�ɶ�
    Sleep(1);//����X���A�קK�p�ɾ��쪽��0
    while(MO_Timer(3, 0, 0))
    {
        if(m_bIsStop == 1)
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
void CAction::DecideLineSToP(LONG lX, LONG lY, LONG lZ, DOUBLE dAng, LONG lX2, LONG lY2, LONG lZ2, DOUBLE dAng2,
                             LONG lStartDelayTime, LONG lStartDistance, LONG lMidDelayTime,
                             LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
    /*�u�q�}�l(x�y�СAy�y�СAz�y�СA�u�q�_�l�I�AW�b����)
    LONG lX, LONG lY, LONG lZ, DOUBLE dAng
    */
    /*�u�q���I(x�y�СAy�y�СAz�y�СA�u�q�_�l�I�AW�b����)
    LONG lX2, LONG lY2, LONG lZ2, DOUBLE dAng
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
    //�P�_W�b�O�_�ե�
    if(m_MachineOffSet.x == -99999 && m_MachineOffSet.y == -99999)
    {
        return;
    }
    //�P�_�ϥΰw�Y�y���٬O����y��
    if(USEMODE_W == 1) //�ϥξ���y��
    {
        CPoint cpbuf = W_GetNeedlePoint(lX, lY, dAng, 1);
        lX = cpbuf.x;
        lY = cpbuf.y;
        cpbuf = W_GetNeedlePoint(lX2, lY2, dAng2, 1);
        lX2 = cpbuf.x;
        lY2 = cpbuf.y;
    }
    /*******�H�U�{�����|���ܨϥΰw�Y�y�а���B��************************/
    //�y�{: ��J�w�Y�y��>�B��w�Y�y��>�ഫ������y�п�X
    LONG lXClose = 0, lYClose = 0, lZClose = 0, lLineClose = 0;
    LONG lBuffX = 0, lBuffY = 0;
    LONG lTime = 0;
    lLineClose = lStartDistance;
    LineGetToPoint(lXClose, lYClose, lZClose, lX2, lY2, lX, lY, lZ2, lZ, lLineClose);
    lBuffX = (-(lXClose - lX)) + lX;
    lBuffY = (-(lYClose - lY)) + lY;

    if((lStartDelayTime > 0 && lStartDistance > 0) || lStartDistance == 0) //(���̳����ȮɥH"���ʫe����"�u��)  ����__���ʫe����
    {
        DecideLineStartMove(lX, lY, lZ, dAng, lStartDelayTime, lWorkVelociy, lAcceleration, lInitVelociy,1);//���ʨ�_�l�I
        DecideLineMidMove(lX2, lY2, lZ2, dAng2, lMidDelayTime, lWorkVelociy, lAcceleration, lInitVelociy,1);//���ʦܤ����I
    }
    else  //����__�]�m�Z��
    {

        DecideLineStartMove(lBuffX, lBuffY, lZClose, dAng, 0, lWorkVelociy, lAcceleration, lInitVelociy,1);//���ʨ�_�l�I
        CPoint cpNeedlePoint = W_GetNeedlePoint();//�N��U��m�ഫ���w�Y�y��
        W_UpdateNeedleMotor_Needle(cpNeedlePoint.x, cpNeedlePoint.y, MO_ReadLogicPosition(2), MO_ReadLogicPositionW(), lX2, lY2, lZ2, dAng);//���I����(�ϥιw�]�C1�ר��@���I)
        lTime = CalPreglue(lStartDistance, lWorkVelociy, lAcceleration, lInitVelociy);
        /*======�p�ɾ���Ĳ�o���_����X���A�ϥ�y���_����================*/
        if(!m_bIsStop)
        {
            CAction::m_YtimeOutGlueSet = TRUE;
            MO_TimerSetIntter(lTime, 0);//�p�ɨ���ܰ����
        }
        if(!m_bIsStop)
        {
            W_Line4DtoDo(lWorkVelociy, lAcceleration, lInitVelociy);//�|�b�s�򴡸�
            PreventMoveError();//����b�d�X��
        }
        MO_Timer(0, 0, lMidDelayTime * 1000);
        MO_Timer(1, 0, lMidDelayTime * 1000);//�u�q�I���]�w---(4)�`�I�ɶ�
        Sleep(1);//����X���A�קK�p�ɾ��쪽��0
        while(MO_Timer(3, 0, 0))
        {
            if(m_bIsStop)
            {
                break;
            }
            Sleep(1);
        }
        PauseStopGlue();//�Ȱ��ɰ����(m_bIsPause=1)
    }

#endif
}
/*
*�u�q�}�l���u�q�����ʧ@
*��J(�u�q�}�lX,Y,Z,�����IX,Y,Z,���ʫe����,�}���e�q�_�I���ʶZ��(�]�m�Z��),������b�����I���d�ɶ�(���d�ɶ�),�Z�������I�h���Z������(�����Z��),�����᪺����ɶ�(��������),Z�b�^�ɶZ��,Z�b�^�ɫ��A,�I�������]�w(���t��),��^����,z��^����,��^�t��(�C�t),��^����,�X�ʳt��,�[�t��,��t��)
*/
void CAction::DecideLineSToE(LONG lX, LONG lY, LONG lZ, DOUBLE dAng, LONG lX2, LONG lY2, LONG lZ2, DOUBLE dAng2,
                             LONG lStartDelayTime, LONG lStartDistance, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime,
                             LONG lZBackDistance, BOOL bZDisType, LONG lHighVelocity, LONG lDistance, LONG lHigh,
                             LONG lLowVelocity, int iType, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
    /*�u�q�}�l(x�y�СAy�y�СAz�y�СAW�b����)
    LONG lX, LONG lY, LONG lZ, DOUBLE dAng
    */
    /*�u�q�����I(x�y�СAy�y�СAz�y�СAW�b����)
    LONG lX2, LONG lY2, LONG lZ2, DOUBLE dAng
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
    //�P�_W�b�O�_�ե�
    if(m_MachineOffSet.x == -99999 && m_MachineOffSet.y == -99999)
    {
        return;
    }
    //�P�_�ϥΰw�Y�y���٬O����y��
    if(USEMODE_W == 1) //�ϥξ���y��
    {
        CPoint cpbuf = W_GetNeedlePoint(lX, lY, dAng, 1);
        lX = cpbuf.x;
        lY = cpbuf.y;
        cpbuf = W_GetNeedlePoint(lX2, lY2, dAng2, 1);
        lX2 = cpbuf.x;
        lY2 = cpbuf.y;
    }
    /*******�H�U�{�����|���ܨϥΰw�Y�y�а���B��************************/
    //�y�{: ��J�w�Y�y��>�B��w�Y�y��>�ഫ������y�п�X
    //LONG lNowZ = 0;
    LONG lXClose = 0, lYClose = 0, lZClose = 0, lLineClose = 0;
    LONG lBuffX = 0, lBuffY = 0, LineLength = 0;
    LONG lTime = 0;
    lLineClose = lStartDistance;
    LineGetToPoint(lXClose, lYClose, lZClose, lX2, lY2, lX, lY, lZ2, lZ, lLineClose);
    lBuffX = (-(lXClose - lX)) + lX;
    lBuffY = (-(lYClose - lY)) + lY;
    LineLength = LONG(sqrt(DOUBLE(pow(lX2 - lX, 2) + pow(lY2 - lY, 2))));
    if(lCloseDistance != 0 && lCloseDistance > LineLength)
    {
        lCloseDistance = LineLength;
    }
    if(bZDisType) //��ϥά۹��m���ɭ�
    {
        LONG lbuf = m_TablelZ - lZBackDistance;//�q���x�̧C���W�۹�Z��
        lZBackDistance = lbuf;
        bZDisType = 0;//�אּ�����m
    }
    if(!bZDisType) //�����m
    {
        if(lZBackDistance > lZ)
        {
            lZBackDistance = lZ;
        }
        lZBackDistance = abs(lZBackDistance - lZ);
    }
    if(lHighVelocity == 0)
    {
        lHighVelocity = lWorkVelociy;
    }
    if(lLowVelocity == 0)
    {
        lLowVelocity = lWorkVelociy;
    }
    if((lStartDelayTime > 0 && lStartDistance > 0) || lStartDistance == 0) //(���̳����ȮɥH"���ʫe����"�u��)  ����__���ʫe����
    {
        DecideLineStartMove(lX, lY, lZ, dAng, lStartDelayTime, lWorkVelociy, lAcceleration, lInitVelociy,1);//���ʨ�_�l�I
        CPoint cpNeedlePoint = W_GetNeedlePoint();//�N��U��m�ഫ���w�Y�y��
        W_UpdateNeedleMotor_Needle(cpNeedlePoint.x, cpNeedlePoint.y, MO_ReadLogicPosition(2), MO_ReadLogicPositionW(), lX2, lY2, lZ2, dAng2);//���I����(�ϥιw�]�C1�ר��@���I)
        if(lCloseDistance != 0) //�u�q�I���]�w---(5)�����Z��
        {
            LONG lAddDis = CalPreglue(lWorkVelociy, lAcceleration, lInitVelociy);//�[�t�׶Z��
            LONG lAddTime = CalPreglue(lWorkVelociy, lAcceleration, lInitVelociy);//�[�t�׮ɶ�
            DOUBLE dSumPath = sqrt(pow(lX2 - lX, 2) + pow(lY2 - lY, 2) + pow(lZ2 - lZ, 2));//�u�q�`��
            LONG lCloseTime =  1000000 * (LONG)round(((dSumPath - lCloseDistance - lAddDis) / (DOUBLE)lWorkVelociy));//�����ɶ�
            /*======�p�ɾ���Ĳ�o���_�����_���A�ϥ�z���_����================*/
            if(!m_bIsStop)
            {
                CAction::m_ZtimeOutGlueSet = FALSE;
              //  _cwprintf(_T("End�����Z�����ɶ�=%lf \n"), (lCloseTime+ lAddTime) / 1000000.0);
                MO_TimerSetIntter(lCloseTime + lAddTime, 1);//�p�ɨ���ܰ����
            }
        }
        if(!m_bIsStop)
        {
            W_Line4DtoDo(lWorkVelociy, lAcceleration, lInitVelociy);//�|�b�s�򴡸�__���ʦܵ����I
            PreventMoveError();//����b�d�X��
        }
    }
    else
    {
        DecideLineStartMove(lBuffX, lBuffY, lZClose, dAng, lStartDelayTime, lWorkVelociy, lAcceleration, lInitVelociy,1);//���ʨ�_�l�I
        CPoint cpNeedlePoint = W_GetNeedlePoint();//�N��U��m�ഫ���w�Y�y��
        W_UpdateNeedleMotor_Needle(cpNeedlePoint.x, cpNeedlePoint.y, MO_ReadLogicPosition(2), MO_ReadLogicPositionW(), lX2, lY2, lZ2, dAng2);//���I����(�ϥιw�]�C1�ר��@���I)
        lTime = CalPreglue(lStartDistance, lWorkVelociy, lAcceleration, lInitVelociy);//�p�ⴣ�e�Z���X���ɶ�
        /*======�p�ɾ���Ĳ�o���_����X���A�ϥ�y���_����================*/
        if(!m_bIsStop)
        {
            CAction::m_YtimeOutGlueSet = TRUE;
            MO_TimerSetIntter(lTime, 0);//�p�ɨ���ܰ����
        }
        if(lCloseDistance != 0) //�u�q�I���]�w---(5)�����Z��
        {
            LONG lAddDis = CalPreglue(lWorkVelociy, lAcceleration, lInitVelociy);//�[�t�׶Z��
            LONG lAddTime = CalPreglue(lWorkVelociy, lAcceleration, lInitVelociy);//�[�t�׮ɶ�
            DOUBLE dSumPath = sqrt(pow(lX2 - lBuffX, 2) + pow(lY2 - lBuffY, 2) + pow(lZ2 - lZClose, 2));//�u�q�`��
            LONG lCloseTime = 1000000 * (LONG)round(((dSumPath - lCloseDistance - lAddDis) / (DOUBLE)lWorkVelociy));//�����ɶ�
            /*======�p�ɾ���Ĳ�o���_�����_���A�ϥ�z���_����================*/
            if(!m_bIsStop)
            {
                CAction::m_ZtimeOutGlueSet = FALSE;
             //   _cwprintf(_T("End�����Z�����ɶ�=%lf \n"), (lCloseTime + lAddTime) / 1000000.0);
                MO_TimerSetIntter(lCloseTime + lAddTime, 1);//�p�ɨ���ܰ����
            }
        }
        if(!m_bIsStop)
        {
            W_Line4DtoDo(lWorkVelociy, lAcceleration, lInitVelociy);//�|�b�s�򴡸�
            PreventMoveError();//����b�d�X��
        }
    }
    MO_StopGumming();//����X��
    GelatinizeBack(iType, lX2, lY2, lZ2, lX, lY, lDistance, lHigh, lZBackDistance, lLowVelocity, lHighVelocity, lAcceleration, lInitVelociy);//��^�]�w
    MO_Timer(0, 0, lCloseOffDelayTime * 1000);
    MO_Timer(1, 0, lCloseOffDelayTime * 1000);//�u�q�I���]�w---(3)���d�ɶ�
    Sleep(1);//����X���A�קK�p�ɾ��쪽��0
    while(MO_Timer(3, 0, 0))
    {
        if(m_bIsStop == 1)
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
void CAction::DecideCircle(LONG lX1, LONG lY1, LONG lZ1, DOUBLE dAng1, LONG lX2, LONG lY2, LONG lZ2, DOUBLE dAng2, LONG lWorkVelociy, LONG lInitVelociy)
{
    /*���(x�y�СAy�y�СAW�b����)
    LONG lX1, LONG lY1,LONG lZ1, DOUBLE dAng1
    */
    /*��ε����I(x�y�СAy�y�СAz�y�СAW�b����)
    LONG lX2, LONG lY2,LONG lZ2, DOUBLE dAng2
    */
    /*�t�ΰѼ�(�X�ʳt�סA�[�t�סA��t��)
    LONG lWorkVelociy, LONG lInitVelociy
    */
#ifdef MOVE
	//�P�_W�b�O�_�ե�
	if (m_MachineOffSet.x == -99999 && m_MachineOffSet.y == -99999)
	{
		return;
	}
	//�P�_�ϥΰw�Y�y���٬O����y��
	if (USEMODE_W == 1) //�ϥξ���y��
	{
		CPoint cpbuf = W_GetNeedlePoint(lX1, lY1, dAng1, 1);
		lX1 = cpbuf.x;
		lY1 = cpbuf.y;
		cpbuf = W_GetNeedlePoint(lX2, lY2, dAng2, 1);
		lX2 = cpbuf.x;
		lY2 = cpbuf.y;
	}
	/*******�H�U�{�����|���ܨϥΰw�Y�y�а���B��************************/
	//�y�{: ��J�w�Y�y��>�B��w�Y�y��>�ഫ������y�п�X
	CCircleFormula Circle;
	AxeSpace p1, p2, p3;

	p1 = { W_GetNeedlePoint().x, W_GetNeedlePoint().y,MO_ReadLogicPosition(2),MO_ReadLogicPositionW() };
	p2 = { lX1,lY1,lZ1,dAng1 };
	p3 = { lX2,lY2,lZ2,dAng2 };
	std::vector<AxeSpace> VecBuf;
	LONG cutRad = 1000;//�H1mm�A�]�N�O1000um�����������I
	if (lZ1 == lZ2&& lZ1 == MO_ReadLogicPosition(2))
	{
		Circle.CircleCutPath_2D_unit(p1, p2, p3, TRUE, cutRad, VecBuf);//2D����
	}
	else
	{
		Circle.m_cuttingLength = cutRad;//���꩷����
		Circle.CircleCutPoint(p1, p2, p3, VecBuf); //3D����
	}
	DOUBLE SumAng = dAng2 - MO_ReadLogicPositionW();//W�j�ਤ�׶q(�t+/-��V��)
	size_t num = VecBuf.size();
	DOUBLE num_Ang = 360.0 / (DOUBLE)num;//�̶꩷�����I�Ƥ���360��
	num_Ang = floor(num_Ang / 0.036)*0.036;//�̰��F 0.036 [deg/pulse] �ഫ���X�z�����פ��q
	DOUBLE rest_Ang = (360.0 - num_Ang*(DOUBLE)num)*((SumAng>0) ? 1 : -1);//�Ѿl������
	DATA_4MOVE *pDataM = new DATA_4MOVE[num];
	DATA_4MOVE *pDataShift = pDataM;
	CPoint cpMpbuf(0, 0), cpMpLast(0, 0);
	int Fin_cnt = (int)floor(fabs(rest_Ang) / 0.072);//�̫�Ӽ�
	DOUBLE absAngle = MO_ReadLogicPositionW();

	for (size_t i = 0; i<VecBuf.size(); i++)
	{
		//Get ���﨤��
		if (Fin_cnt != 0)
		{
			//Get ���﨤��
			absAngle = absAngle + ((SumAng > 0) ? num_Ang + 0.072 : -num_Ang - 0.072);
			pDataShift->AngleW = ((SumAng > 0) ? num_Ang + 0.072 : -num_Ang - 0.072);
			Fin_cnt--;
		}
		else
		{
			absAngle = absAngle + ((SumAng > 0) ? num_Ang : -num_Ang);
			pDataShift->AngleW = ((SumAng > 0) ? num_Ang : -num_Ang);
		}
		//Get ���񵴹�y��
		cpMpbuf = W_GetMachinePoint(VecBuf.at(i).x, VecBuf.at(i).y, absAngle, 1);

		//�i�浴��/�۹�y���ഫ
		if (i == 0)
		{
			pDataShift->EndPX = cpMpbuf.x - MO_ReadLogicPosition(0);
			pDataShift->EndPY = cpMpbuf.y - MO_ReadLogicPosition(1);
			pDataShift->EndPZ = VecBuf.at(i).z - MO_ReadLogicPosition(2);
			pDataShift->Distance = (LONG)sqrt(pow(pDataShift->EndPX, 2) + pow(pDataShift->EndPY, 2) + pow(pDataShift->EndPZ, 2));
			//_cwprintf(_T("%d,%d,%.3f\n"), pDataShift->EndPX, pDataShift->EndPY, pDataShift->AngleW);
			//TRACE(_T(",%d,%d,%.3f\n"), pDataShift->EndPX, pDataShift->EndPY, pDataShift->AngleW);
		}
		else
		{
			pDataShift->EndPX = cpMpbuf.x - cpMpLast.x;
			pDataShift->EndPY = cpMpbuf.y - cpMpLast.y;
			pDataShift->EndPZ = VecBuf.at(i).z - VecBuf.at(i - 1).z;
			pDataShift->Distance = (LONG)sqrt(pow(pDataShift->EndPX, 2) + pow(pDataShift->EndPY, 2) + pow(pDataShift->EndPZ, 2));
			//_cwprintf(_T("%d,%d,%.3f\n"), pDataShift->EndPX, pDataShift->EndPY, pDataShift->AngleW);
			// TRACE(_T(",%d,%d,%.3f\n"), pDataShift->EndPX, pDataShift->EndPY, pDataShift->AngleW);
		}
		cpMpLast = cpMpbuf;
		pDataShift++;
	}
	PauseDoGlue();//�Ȱ���_���~��X��(m_bIsPause=0)
	if (!m_bIsStop)
	{
		MO_DO4Curve(pDataM, num, lWorkVelociy);
		PreventMoveError();//�����X�ʿ��~
	}
	pDataShift = NULL;
	delete[] pDataM;
	PauseStopGlue();//�Ȱ��ɰ����(m_bIsPause=1)
#endif
}
/*
*�꩷�ʧ@
*��J(�꩷�B�꩷�����I�B�u�t�B�t�ΰѼ�)
*/
void CAction::DecideArc(LONG lX1, LONG lY1, LONG lZ1, DOUBLE dAng1, LONG lX2, LONG lY2, LONG lZ2, DOUBLE dAng2, LONG lWorkVelociy, LONG lInitVelociy)
{
    /*�꩷(x�y�СAy�y�СA)
    LONG lX1, LONG lY1,LONG lZ1, DOUBLE dAng1
    */
    /*�꩷�����I(x�y�СAy�y�СA)
    LONG lX2, LONG lY2,LONG lZ2, DOUBLE dAng2
    */
    /*�t�ΰѼ�(�X�ʳt�סA�[�t�סA��t��)
    LONG lWorkVelociy, LONG lInitVelociy
    */
#ifdef MOVE
	//�P�_W�b�O�_�ե�
	if (m_MachineOffSet.x == -99999 && m_MachineOffSet.y == -99999)
	{
		return;
	}
	//�P�_�ϥΰw�Y�y���٬O����y��
	if (USEMODE_W == 1) //�ϥξ���y��
	{
		CPoint cpbuf = W_GetNeedlePoint(lX1, lY1, dAng1, 1);
		lX1 = cpbuf.x;
		lY1 = cpbuf.y;
		cpbuf = W_GetNeedlePoint(lX2, lY2, dAng2, 1);
		lX2 = cpbuf.x;
		lY2 = cpbuf.y;
	}
	/*******�H�U�{�����|���ܨϥΰw�Y�y�а���B��************************/
	//�y�{: ��J�w�Y�y��>�B��w�Y�y��>�ഫ������y�п�X
	CCircleFormula Circle;
	AxeSpace p1, p2, p3;
	p1 = { W_GetNeedlePoint().x, W_GetNeedlePoint().y,MO_ReadLogicPosition(2),MO_ReadLogicPositionW() };
	p2 = { lX1,lY1,lZ1,dAng1 };
	p3 = { lX2,lY2,lZ2,dAng2 };
	std::vector<AxeSpace> VecBuf;
	LONG cutRad = 1000;//�H1mm�A�]�N�O1000um�����������I
	if (lZ1 == lZ2&&lZ1 == MO_ReadLogicPosition(2))
	{
		Circle.CircleCutPath_2D_unit(p1, p2, p3, false, cutRad, VecBuf);//2D���꩷
	}
	else
	{
		Circle.m_cuttingLength = cutRad;//���꩷����
		Circle.ArcCutPoint(p1, p2, p3, VecBuf);//3D���꩷
	}
	DOUBLE SumAng = dAng2 - MO_ReadLogicPositionW();//W�j�ਤ�׶q(�t+/-��V��)
	size_t num = VecBuf.size();
	DOUBLE num_Ang = fabs(SumAng) / (DOUBLE)num;//�̶꩷�����I�Ƥ��ΰj�ਤ�׶q(���t+/-��V��)
	num_Ang = floor(num_Ang / 0.036)*0.036;//�̰��F 0.036 [deg/pulse] �ഫ���X�z�����פ��q(���t+/-��V��)
	DOUBLE rest_Ang = (fabs(SumAng) - num_Ang*(DOUBLE)num)*((SumAng>0) ? 1 : -1);//�Ѿl������(�t+/-��V��)
	DATA_4MOVE *pDataM = new DATA_4MOVE[num];
	DATA_4MOVE *pDataShift = pDataM;
	CPoint cpMpbuf(0, 0), cpMpLast(0, 0);
	int Fin_cnt = (int)floor(fabs(rest_Ang) / 0.072);//�̫�Ӽ�
	DOUBLE absAngle = MO_ReadLogicPositionW();

	for (size_t i = 0; i<VecBuf.size(); i++)
	{
		if (Fin_cnt != 0)
		{
			//Get ���﨤��
			absAngle = absAngle + ((SumAng > 0) ? num_Ang + 0.072 : -num_Ang - 0.072);
			pDataShift->AngleW = ((SumAng > 0) ? num_Ang + 0.072 : -num_Ang - 0.072);
			Fin_cnt--;
		}
		else
		{
			absAngle = absAngle + ((SumAng > 0) ? num_Ang : -num_Ang);
			pDataShift->AngleW = ((SumAng > 0) ? num_Ang : -num_Ang);
		}
		//Get ���񵴹�y��
		cpMpbuf = W_GetMachinePoint(VecBuf.at(i).x, VecBuf.at(i).y, absAngle, 1);

		//�i�浴��/�۹�y���ഫ
		if (i == 0)
		{
			pDataShift->EndPX = cpMpbuf.x - MO_ReadLogicPosition(0);
			pDataShift->EndPY = cpMpbuf.y - MO_ReadLogicPosition(1);
			pDataShift->EndPZ = VecBuf.at(i).z - MO_ReadLogicPosition(2);
			pDataShift->Distance = (LONG)sqrt(pow(pDataShift->EndPX, 2) + pow(pDataShift->EndPY, 2) + pow(pDataShift->EndPZ, 2));
			//_cwprintf(_T("%d,%d,%.3f\n"), pDataShift->EndPX, pDataShift->EndPY, pDataShift->AngleW);
			//TRACE(_T(",%d,%d,%.3f\n"), pDataShift->EndPX, pDataShift->EndPY, pDataShift->AngleW);
		}
		else
		{
			pDataShift->EndPX = cpMpbuf.x - cpMpLast.x;
			pDataShift->EndPY = cpMpbuf.y - cpMpLast.y;
			pDataShift->EndPZ = VecBuf.at(i).z - VecBuf.at(i - 1).z;
			pDataShift->Distance = (LONG)sqrt(pow(pDataShift->EndPX, 2) + pow(pDataShift->EndPY, 2) + pow(pDataShift->EndPZ, 2));
			if ((i == VecBuf.size() - 2) || (i == VecBuf.size() - 1))
			{
				_cwprintf(_T("lost_%d,%d,%.3f\n"), pDataShift->EndPX, pDataShift->EndPY, pDataShift->AngleW);
			}
			//_cwprintf(_T("%d,%d,%.3f\n"), pDataShift->EndPX, pDataShift->EndPY, pDataShift->AngleW);
			//TRACE(_T(",%d,%d,%.3f\n"), pDataShift->EndPX, pDataShift->EndPY, pDataShift->AngleW);
		}
		cpMpLast = cpMpbuf;
		pDataShift++;
	}
	PauseDoGlue();//�Ȱ���_���~��X��(m_bIsPause=0)
	if (!m_bIsStop)
	{
		MO_DO4Curve(pDataM, num, lWorkVelociy);
		PreventMoveError();//�����X�ʿ��~
	}
	pDataShift = NULL;
	delete[] pDataM;
	PauseStopGlue();//�Ȱ��ɰ����(m_bIsPause=1)
#endif
}
/*�u�q�}�l��ꤤ���I�ʧ@
*(��J�Ѽ�:�u�q�}�l,��ήy��,��ήy��,���ʫe����,�}���e�q�_�I���ʶZ��(�]�m�Z��),��u�q���I�����d�ɶ�(�`�I�ɶ�ms),�X�ʳt��,�[�t��,��t��)
*/
void CAction::DecideLineSToCirP(LONG lX1, LONG lY1, LONG lZ1, DOUBLE dAng1, LONG lX2, LONG lY2, LONG lZ2, DOUBLE dAng2, LONG lX3, LONG lY3, LONG lZ3, DOUBLE dAng3, LONG lStartDelayTime, LONG lStartDistance, LONG lMidDelayTime, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
    /*�u�q�}�l(x�y�СAy�y�СAz�y�СA�u�q�_�l�I�AW�b����)
    LONG lX, LONG lY, LONG lZ, DOUBLE dAng
    */
    /*����I(x�y�СAy�y�СAz�y�СAW�b����)
    LONG lX2, LONG lY2, LONG lZ2, DOUBLE dAng2
    */
    /*����I(x�y�СAy�y�СAz�y�СAW�b����)
    LONG lX3, LONG lY3, LONG lZ3, DOUBLE dAng3
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
    //�P�_W�b�O�_�ե�
    if(m_MachineOffSet.x == -99999 && m_MachineOffSet.y == -99999)
    {
        return;
    }
    //�P�_�ϥΰw�Y�y���٬O����y��
    if(USEMODE_W == 1) //�ϥξ���y��
    {
        CPoint cpbuf = W_GetNeedlePoint(lX1, lY1, dAng1, 1);
        lX1 = cpbuf.x;
        lY1 = cpbuf.y;
        cpbuf = W_GetNeedlePoint(lX2, lY2, dAng2, 1);
        lX2 = cpbuf.x;
        lY2 = cpbuf.y;
        cpbuf = W_GetNeedlePoint(lX3, lY3, dAng3, 1);
        lX3 = cpbuf.x;
        lY3 = cpbuf.y;
    }
    /*******�H�U�{�����|���ܨϥΰw�Y�y�а���B��************************/
    //�y�{: ��J�w�Y�y��>�B��w�Y�y��>�ഫ������y�п�X
    //DOUBLE dSumPath = 0, dRad = 0, angle = 0;//�u�q�`���סB�b�|�B�_���ɶ��B����(����)
    //BOOL bRev = 0;//���f��
    //CCircleFormula Circle;
    //AxeSpace p1, p2, p3;
    //p1 = { W_GetNeedlePoint().x, W_GetNeedlePoint().y,MO_ReadLogicPosition(2),MO_ReadLogicPositionW() };
    //p2 = { lX1,lY1,lZ1,dAng1 };
    //p3 = { lX2,lY2,lZ2,dAng2 };
    //DPoint pCen, dp1, dp2, dp3;
    //dp1 = { W_GetNeedlePoint().x, W_GetNeedlePoint().y,MO_ReadLogicPosition(2),MO_ReadLogicPositionW() };
    //dp2 = { lX1,lY1,lZ1,dAng1 };
    //dp3 = { lX2,lY2,lZ2,dAng2 };
    //std::vector<AxeSpace> VecBuf;
    //LONG cutRad = 1000;//�H1mm�A�]�N�O1000um�����������I
    //if (lZ1 == lZ2)
    //{
    //    //�D�G����߻P�b�|
    //    pCen = Circle.ArcCentCalculation_2D(dp1, dp2, dp3, dRad, angle, bRev);
    //    Circle.CircleCutPath_2D_unit(p1, p2, p3, TRUE, cutRad, VecBuf);//2D����
    //}
    //else
    //{
    //    //�D�T����߻P�b�|
    //    Circle.ArcCentCalculation_2D(dp1, dp2, dp3, dRad, angle, bRev);//�ϥ�2d�D���׻P���f��
    //    pCen = Circle.SpaceCircleCenterCalculation(dp1, dp2, dp3);
    //    dRad = sqrt(pow(pCen.x - p2.x, 2) + pow(pCen.y - p2.y, 2) + pow(pCen.z - p2.z, 2));
    //    Circle.CircleCutPoint(p1, p2, p3, VecBuf); //3d����
    //}
    //DOUBLE SumAng = dAng2 - MO_ReadLogicPositionW();
    //size_t num = VecBuf.size();
    //DOUBLE num_Ang = 360 / (DOUBLE)num;
    //DATA_4MOVE *pDataM = new DATA_4MOVE[num];
    //DATA_4MOVE *pDataShift = pDataM;
    //for (size_t i = 0; i<VecBuf.size() - 1; i++)
    //{
    //    if (i == 0)
    //    {
    //        pDataShift->EndPX = W_GetMachinePoint(VecBuf.at(i).x, VecBuf.at(i).y, MO_ReadLogicPositionW(), 1).x - MO_ReadLogicPosition(0);
    //        pDataShift->EndPY = W_GetMachinePoint(VecBuf.at(i).x, VecBuf.at(i).y, MO_ReadLogicPositionW(), 1).y - MO_ReadLogicPosition(1);
    //        pDataShift->EndPZ = VecBuf.at(i).z - MO_ReadLogicPosition(2);
    //        pDataShift->AngleW = MO_ReadLogicPositionW();
    //        pDataShift->Distance = pDataShift->Distance = 0;
    //        //_cwprintf(_T("%d,%d,%.3f\n"), pDataShift->EndPX, pDataShift->EndPY, pDataShift->AngleW);
    //        //TRACE(_T(",%d,%d,%.3f\n"), pDataShift->EndPX, pDataShift->EndPY, pDataShift->AngleW);
    //    }
    //    else
    //    {

    //        pDataShift->EndPX = W_GetMachinePoint(VecBuf.at(i).x, VecBuf.at(i).y, MO_ReadLogicPositionW() + (num_Ang * i), 1).x - VecBuf.at(i - 1).x;
    //        pDataShift->EndPY = W_GetMachinePoint(VecBuf.at(i).x, VecBuf.at(i).y, MO_ReadLogicPositionW() + (num_Ang * i), 1).y - VecBuf.at(i - 1).y;
    //        pDataShift->EndPZ = VecBuf.at(i).z - VecBuf.at(i - 1).z;
    //        pDataShift->AngleW = MO_ReadLogicPositionW() + (num_Ang * i);
    //        pDataShift->Distance = pDataShift->Distance = 0;
    //        //_cwprintf(_T("%d,%d,%.3f\n"), pDataShift->EndPX, pDataShift->EndPY, pDataShift->AngleW);
    //        // TRACE(_T(",%d,%d,%.3f\n"), pDataShift->EndPX, pDataShift->EndPY, pDataShift->AngleW);

    //    }
    //    pDataShift++;
    //}
    //LONG lStartX = 0, lStartY = 0, lTime = 0;
    //ArcGetToPoint(lStartX, lStartY, lStartDistance, lX1, lY1, pCen.x, pCen.y, LONG(dRad), bRev);//��X�_�l�I
    //DecideLineStartMove(lStartX, lStartY, MO_ReadLogicPosition(2), dAng1, 0, lWorkVelociy, lAcceleration, lInitVelociy);
    //CPoint cpNeedlePoint = W_GetNeedlePoint();//�N��U��m�ഫ���w�Y�y��
    //W_UpdateNeedleMotor_Needle(cpNeedlePoint.x, cpNeedlePoint.y, MO_ReadLogicPosition(2), MO_ReadLogicPositionW(), lStartX, lStartY, lZ1, dAng1);//���I����(�ϥιw�]�C1�ר��@���I)
    //if (!m_bIsStop)
    //{
    //    W_Line4DtoDo(lWorkVelociy, lAcceleration, lInitVelociy);//�|�b�s�򴡸�
    //    PreventMoveError();//����b�d�X��
    //}

    //lTime = CalPreglue(lStartDistance, lWorkVelociy, lAcceleration, lInitVelociy);//�p�ⴣ�e�Z���X���ɶ�
    ///*======�p�ɾ���Ĳ�o���_����X���A�ϥ�y���_����================*/
    //if (!m_bIsStop)
    //{
    //    CAction::m_YtimeOutGlueSet = TRUE;
    //    MO_TimerSetIntter(lTime, 0);//�p�ɨ���ܰ����
    //}

    //LONG lXClose = 0, lYClose = 0, lZClose = 0, lLineClose = 0;
    //LONG lBuffX = 0, lBuffY = 0;
    //LONG lTime = 0;
    //lLineClose = lStartDistance;
    //LineGetToPoint(lXClose, lYClose, lZClose, lX2, lY2, lX, lY, lZ2, lZ, lLineClose);
    //lBuffX = (-(lXClose - lX)) + lX;
    //lBuffY = (-(lYClose - lY)) + lY;
    //if ((lStartDelayTime > 0 && lStartDistance > 0) || lStartDistance == 0)//(���̳����ȮɥH"���ʫe����"�u��)  ����__���ʫe����
    //{
    //    DecideLineStartMove(lX, lY, lZ, dAng, lStartDelayTime, lWorkVelociy, lAcceleration, lInitVelociy);//���ʨ�_�l�I
    //    DecideLineMidMove(lX2, lY2, lZ2, dAng2, lMidDelayTime, lWorkVelociy, lAcceleration, lInitVelociy);//���ʦܤ����I
    //}
    //else  //����__�]�m�Z��
    //{

    //    DecideLineStartMove(lBuffX, lBuffY, lZClose, dAng, 0, lWorkVelociy, lAcceleration, lInitVelociy);//���ʨ�_�l�I
    //    CPoint cpNeedlePoint = W_GetNeedlePoint();//�N��U��m�ഫ���w�Y�y��
    //    W_UpdateNeedleMotor_Needle(cpNeedlePoint.x, cpNeedlePoint.y, MO_ReadLogicPosition(2), MO_ReadLogicPositionW(), lX2, lY2, lZ2, dAng);//���I����(�ϥιw�]�C1�ר��@���I)
    //    lTime = CalPreglue(lStartDistance, lWorkVelociy, lAcceleration, lInitVelociy);
    //    /*======�p�ɾ���Ĳ�o���_����X���A�ϥ�y���_����================*/
    //    if (!m_bIsStop)
    //    {
    //        CAction::m_YtimeOutGlueSet = TRUE;
    //        MO_TimerSetIntter(lTime, 0);//�p�ɨ���ܰ����
    //    }
    //    if (!m_bIsStop)
    //    {
    //        W_Line4DtoDo(lWorkVelociy, lAcceleration, lInitVelociy);//�|�b�s�򴡸�
    //        PreventMoveError();//����b�d�X��
    //    }
    //    MO_Timer(0, 0, lMidDelayTime * 1000);
    //    MO_Timer(1, 0, lMidDelayTime * 1000);//�u�q�I���]�w---(4)�`�I�ɶ�
    //    Sleep(1);//����X���A�קK�p�ɾ��쪽��0
    //    while (MO_Timer(3, 0, 0))
    //    {
    //        if (m_bIsStop)
    //        {
    //            break;
    //        }
    //        Sleep(1);
    //    }
    //    PauseStopGlue();//�Ȱ��ɰ����(m_bIsPause=1)
    //}

#endif
}

/*
*�ꤤ���I�쵲���I
*(��J�Ѽ�:���1�A���2�A�u�q�����I�A�u�q�I���]�w�A��^�]�w�AZ�b�u�@���׳]�w�A�I�������]�w�A�t�ΰѼ�)
*���e:��J�������I�|�P�_�O�_�P�{�b��m�ۦP�A���X�۹������ʧ@
*/
void CAction::DecideCirclePToEnd(LONG lX1, LONG lY1, LONG lZ1, DOUBLE dAng1, LONG lX2, LONG lY2, LONG lZ2, DOUBLE dAng2, LONG lX3, LONG lY3, LONG lZ3, DOUBLE dAng3, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime, LONG lZBackDistance, BOOL bZDisType, LONG lHighVelocity, LONG lDistance, LONG lHigh, LONG lLowVelocity, int iType, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
    /*���(x�y�СAy�y�СAz�y�СAW�b����)
    LONG lX1, LONG lY1, LONG lZ1, DOUBLE dAng1
    */
    /*��ε����I(x�y�СAy�y�СAz�y�СAW�b����)
    LONG lX2, LONG lY2, LONG lZ2, DOUBLE dAng2
    */
    /*�u�q�����I(x�y�СAy�y�СAz�y�СAW�b����)
    LONG lX3, LONG lY3, LONG lZ3, DOUBLE dAng3
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
	//�P�_W�b�O�_�ե�
	if (m_MachineOffSet.x == -99999 && m_MachineOffSet.y == -99999)
	{
		return;
	}
	//�P�_�ϥΰw�Y�y���٬O����y��
	if (USEMODE_W == 1) //�ϥξ���y��
	{
		CPoint cpbuf = W_GetNeedlePoint(lX1, lY1, dAng1, 1);
		lX1 = cpbuf.x;
		lY1 = cpbuf.y;
		cpbuf = W_GetNeedlePoint(lX2, lY2, dAng2, 1);
		lX2 = cpbuf.x;
		lY2 = cpbuf.y;
		cpbuf = W_GetNeedlePoint(lX3, lY3, dAng3, 1);
		lX3 = cpbuf.x;
		lY3 = cpbuf.y;
	}
	/*******�H�U�{�����|���ܨϥΰw�Y�y�а���B��************************/
	//�y�{: ��J�w�Y�y��>�B��w�Y�y��>�ഫ������y�п�X
	DOUBLE dSumPath = 0, dRad = 0, dCloseTime = 0, angle = 0;;//�u�q�`���סB�b�|�B�_���ɶ��B����
	if (lHighVelocity == 0)
	{
		lHighVelocity = lWorkVelociy;
	}
	if (lLowVelocity == 0)
	{
		lLowVelocity = lWorkVelociy;
	}
	if (bZDisType) //��ϥά۹��m���ɭ�
	{
		LONG lbuf = m_TablelZ - lZBackDistance;//�q���x�̧C���W�۹�Z��
		lZBackDistance = lbuf;
		bZDisType = 0;//�אּ�����m
	}
	if (!bZDisType) //�����m
	{
		if (lZBackDistance > MO_ReadLogicPosition(2))
		{
			lZBackDistance = MO_ReadLogicPosition(2);
		}
		lZBackDistance = abs(lZBackDistance - MO_ReadLogicPosition(2));
	}
	CCircleFormula Circle;
	AxeSpace p1, p2, p3;
	p1 = { W_GetNeedlePoint().x, W_GetNeedlePoint().y,MO_ReadLogicPosition(2),MO_ReadLogicPositionW() };
	p2 = { lX1,lY1,lZ1,dAng1 };
	p3 = { lX2,lY2,lZ2,dAng2 };
	DPoint pCen, dp1, dp2, dp3;
	dp1 = { (DOUBLE)W_GetNeedlePoint().x, (DOUBLE)W_GetNeedlePoint().y,(DOUBLE)MO_ReadLogicPosition(2),MO_ReadLogicPositionW() };
	dp2 = { (DOUBLE)(DOUBLE)lX1,(DOUBLE)lY1,(DOUBLE)lZ1,dAng1 };
	dp3 = { (DOUBLE)(DOUBLE)lX2,(DOUBLE)lY2,(DOUBLE)lZ2,dAng2 };
	std::vector<AxeSpace> VecBuf;
	LONG cutRad = 1000;//�H1mm�A�]�N�O1000um�����������I
	if (lZ1 == lZ2&& lZ1 == MO_ReadLogicPosition(2))
	{
		//�D�G����ߡB�b�|�B����
		pCen = Circle.ArcCentCalculation_2D(dp1, dp2, dp3, dRad, angle);
		Circle.CircleCutPath_2D_unit(p1, p2, p3, TRUE, cutRad, VecBuf);//2D����
	}
	else
	{
		//�D�T����ߡB�b�|�B����
		Circle.m_cuttingLength = cutRad;//���꩷����
		pCen = Circle.SpaceCircleCenterCalculation(dp1, dp2, dp3, dRad, angle);
		angle = 360.0;
		Circle.CircleCutPoint(p1, p2, p3, VecBuf); //3D����
	}
	DOUBLE SumAng = dAng2 - MO_ReadLogicPositionW();//W�j�ਤ�׶q(�t+/-��V��)
	size_t num = VecBuf.size();
	DOUBLE num_Ang = 360.0 / (DOUBLE)num;//�̶꩷�����I�Ƥ���360��
	num_Ang = floor(num_Ang / 0.036)*0.036;//�̰��F 0.036 [deg/pulse] �ഫ���X�z�����פ��q
	DOUBLE rest_Ang = (360.0 - num_Ang*(DOUBLE)num)*((SumAng>0) ? 1 : -1);//�Ѿl������
	DATA_4MOVE *pDataM = new DATA_4MOVE[num];
	DATA_4MOVE *pDataShift = pDataM;
	CPoint cpMpbuf(0, 0), cpMpLast(0, 0);
	int Fin_cnt = (int)floor(fabs(rest_Ang) / 0.072);//�̫�Ӽ�
	DOUBLE absAngle = MO_ReadLogicPositionW();

	for (size_t i = 0; i<VecBuf.size(); i++)
	{
		//Get ���﨤��
		if (Fin_cnt != 0)
		{
			//Get ���﨤��
			absAngle = absAngle + ((SumAng > 0) ? num_Ang + 0.072 : -num_Ang - 0.072);
			pDataShift->AngleW = ((SumAng > 0) ? num_Ang + 0.072 : -num_Ang - 0.072);
			Fin_cnt--;
		}
		else
		{
			absAngle = absAngle + ((SumAng > 0) ? num_Ang : -num_Ang);
			pDataShift->AngleW = ((SumAng > 0) ? num_Ang : -num_Ang);
		}
		//Get ���񵴹�y��
		cpMpbuf = W_GetMachinePoint(VecBuf.at(i).x, VecBuf.at(i).y, absAngle, 1);

		//�i�浴��/�۹�y���ഫ
		if (i == 0)
		{
			pDataShift->EndPX = cpMpbuf.x - MO_ReadLogicPosition(0);
			pDataShift->EndPY = cpMpbuf.y - MO_ReadLogicPosition(1);
			pDataShift->EndPZ = VecBuf.at(i).z - MO_ReadLogicPosition(2);
			pDataShift->Distance = (LONG)sqrt(pow(pDataShift->EndPX, 2) + pow(pDataShift->EndPY, 2) + pow(pDataShift->EndPZ, 2));
			//_cwprintf(_T("%d,%d,%.3f\n"), pDataShift->EndPX, pDataShift->EndPY, pDataShift->AngleW);
			//TRACE(_T(",%d,%d,%.3f\n"), pDataShift->EndPX, pDataShift->EndPY, pDataShift->AngleW);
		}
		else
		{
			pDataShift->EndPX = cpMpbuf.x - cpMpLast.x;
			pDataShift->EndPY = cpMpbuf.y - cpMpLast.y;
			pDataShift->EndPZ = VecBuf.at(i).z - VecBuf.at(i - 1).z;
			pDataShift->Distance = (LONG)sqrt(pow(pDataShift->EndPX, 2) + pow(pDataShift->EndPY, 2) + pow(pDataShift->EndPZ, 2));
			//_cwprintf(_T("%d,%d,%.3f\n"), pDataShift->EndPX, pDataShift->EndPY, pDataShift->AngleW);
			// TRACE(_T(",%d,%d,%.3f\n"), pDataShift->EndPX, pDataShift->EndPY, pDataShift->AngleW);
		}
		cpMpLast = cpMpbuf;
		pDataShift++;
	}
	PauseDoGlue();//�Ȱ���_���~��X��(m_bIsPause=0)
	if ((W_GetNeedlePoint().x >= lX3 - 5) && (W_GetNeedlePoint().x <= lX3 + 5) && (W_GetNeedlePoint().y >= lY3 - 5) && (W_GetNeedlePoint().y <= lY3 + 5))  //��ܵ����I�b�_�l�I�W
	{
		if (lCloseDistance != 0)  //�u�q�I���]�w---(5)�����Z��
		{
			dSumPath = M_PI * dRad*2.0;//���ʪ��`����
			if (lCloseDistance != 0 && lCloseDistance > (LONG)round(dSumPath))
			{
				lCloseDistance = (LONG)round(dSumPath);
			}
			dCloseTime = 1000000 * (((LONG)round(dSumPath) - lCloseDistance) / (DOUBLE)lWorkVelociy);
			/*======�p�ɾ���Ĳ�o���_�����_���A�ϥ�z���_����================*/
			if (!m_bIsStop)
			{
				CAction::m_ZtimeOutGlueSet = FALSE;
				_cwprintf(_T("End�����Z�����ɶ�=%l.3f \n"), DOUBLE(dCloseTime / 1000000.0));
				MO_TimerSetIntter((LONG)round(dCloseTime), 1);//�p�ɨ���ܰ����
			}
		}
		if (!m_bIsStop)
		{
			MO_DO4Curve(pDataM, num, lWorkVelociy);//����|�b����
			PreventMoveError();//�����X�ʿ��~
		}
		pDataShift = NULL;
		delete[] pDataM;
		if (lCloseDistance == 0) //��S�������Z���ɦ���������N�|����
		{
			MO_Timer(0, 0, lCloseONDelayTime * 1000);
			MO_Timer(1, 0, lCloseONDelayTime * 1000);//�u�q�I���]�w---(6)��������
			Sleep(1);//����X���A�קK�p�ɾ��쪽��0
			while (MO_Timer(3, 0, 0))
			{
				if (m_bIsStop == 1)
				{
					break;
				}
				Sleep(1);
			}
		}
		MO_StopGumming();//����X��
						 //��^�]�w���Ѧ��I(�����I�Olx3,ly3,�X�o�Ѧ��I�ϥζ�2)
		GelatinizeBack(iType, lX3, lY3, MO_ReadLogicPosition(2), lX2, lY2, lDistance, lHigh, lZBackDistance, lLowVelocity, lHighVelocity, lAcceleration, lInitVelociy);//��^�]�w
	}
	else//��ܵ����I���b��W
	{
		if (!m_bIsStop)
		{
			MO_DO4Curve(pDataM, num, lWorkVelociy);//����|�b����
			PreventMoveError();//�����X�ʿ��~
		}
		pDataShift = NULL;
		delete[] pDataM;
		CPoint cpNeedlePoint = W_GetNeedlePoint();//�N��U��m�ഫ���w�Y�y��
		W_UpdateNeedleMotor_Needle(cpNeedlePoint.x, cpNeedlePoint.y, MO_ReadLogicPosition(2), MO_ReadLogicPositionW(), lX3, lY3, lZ3, dAng3);//���I����(�ϥιw�]�C1�ר��@���I)
		if (lCloseDistance != 0)  //�u�q�I���]�w---(5)�����Z��
		{
			dSumPath = sqrt(pow(lX3 - W_GetNeedlePoint().x, 2) + pow(lY3 - W_GetNeedlePoint().y, 2) + pow(lZ3 - MO_ReadLogicPosition(2), 2));;//���ʪ��`����
			if (lCloseDistance != 0 && lCloseDistance > (LONG)round(dSumPath))
			{
				lCloseDistance = (LONG)round(dSumPath);
			}
			dCloseTime = 1000000 * (((LONG)round(dSumPath) - lCloseDistance) / (DOUBLE)lWorkVelociy);
			/*======�p�ɾ���Ĳ�o���_�����_���A�ϥ�z���_����================*/
			if (!m_bIsStop)
			{
				CAction::m_ZtimeOutGlueSet = FALSE;
				_cwprintf(_T("End�����Z�����ɶ�=%l.3f \n"), DOUBLE(dCloseTime / 1000000.0));
				MO_TimerSetIntter((LONG)round(dCloseTime), 1);//�p�ɨ���ܰ����
			}
		}
		if (!m_bIsStop)
		{
			W_Line4DtoDo(lWorkVelociy, lAcceleration, lInitVelociy);//�|�b�s�򴡸�
			PreventMoveError();//����b�d�X��
		}
		if (lCloseDistance == 0) //��S�������Z���ɦ���������N�|����
		{
			MO_Timer(0, 0, lCloseONDelayTime * 1000);
			MO_Timer(1, 0, lCloseONDelayTime * 1000);//�u�q�I���]�w---(6)��������
			Sleep(1);//����X���A�קK�p�ɾ��쪽��0
			while (MO_Timer(3, 0, 0))
			{
				if (m_bIsStop == 1)
				{
					break;
				}
				Sleep(1);
			}
		}
		MO_StopGumming();//����X��
						 //��^�]�w���Ѧ��I(�����I�Olx3,ly3,�X�o�Ѧ��I�ϥζ�2)
		GelatinizeBack(iType, lX3, lY3, MO_ReadLogicPosition(2), W_GetNeedlePoint().x, W_GetNeedlePoint().y, lDistance, lHigh, lZBackDistance, lLowVelocity, lHighVelocity, lAcceleration, lInitVelociy);//��^�]�w
	}
	MO_Timer(0, 0, lCloseOffDelayTime * 1000);
	MO_Timer(1, 0, lCloseOffDelayTime * 1000);//�u�q�I���]�w---(3)���d�ɶ�
	Sleep(1);//����X���A�קK�p�ɾ��쪽��0
	while (MO_Timer(3, 0, 0))
	{
		if (m_bIsStop == 1)
		{
			break;
		}
	}
#endif
}
/*
*�꩷�����I�쵲��
*(��J�Ѽ�:�꩷�A�u�q�����I�A�u�q�I���]�w�A��^�]�w�AZ�b�u�@���׳]�w�A�I�������]�w�A�t�ΰѼ�)
*���e:��J���꩷�O�s�������I�A���X�۹������ʧ@
*/
void CAction::DecideArclePToEnd(LONG lX1, LONG lY1, LONG lZ1, DOUBLE dAng1, LONG lX2, LONG lY2, LONG lZ2, DOUBLE dAng2, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime, LONG lZBackDistance, BOOL bZDisType, LONG lHighVelocity, LONG lDistance, LONG lHigh, LONG lLowVelocity, int iType, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
    /*�꩷(x�y�СAy�y�СAz�y�СAW�b����)
    LONG lX1, LONG lY1, LONG lZ1, DOUBLE dAng1
    */
    /*�u�q�����I(x�y�СAy�y�СAz�y�СAW�b����)
    LONG lX2, LONG lY2, LONG lZ2, DOUBLE dAng2
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
	//�P�_W�b�O�_�ե�
	if (m_MachineOffSet.x == -99999 && m_MachineOffSet.y == -99999)
	{
		return;
	}
	//�P�_�ϥΰw�Y�y���٬O����y��
	if (USEMODE_W == 1) //�ϥξ���y��
	{
		CPoint cpbuf = W_GetNeedlePoint(lX1, lY1, dAng1, 1);
		lX1 = cpbuf.x;
		lY1 = cpbuf.y;
		cpbuf = W_GetNeedlePoint(lX2, lY2, dAng2, 1);
		lX2 = cpbuf.x;
		lY2 = cpbuf.y;
	}
	/*******�H�U�{�����|���ܨϥΰw�Y�y�а���B��************************/
	//�y�{: ��J�w�Y�y��>�B��w�Y�y��>�ഫ������y�п�X
	DOUBLE dSumPath = 0, dRad = 0, dCloseTime = 0, angle = 0;//�u�q�`���סB�b�|�B�_���ɶ��B����(����)
	if (lHighVelocity == 0)
	{
		lHighVelocity = lWorkVelociy;
	}
	if (lLowVelocity == 0)
	{
		lLowVelocity = lWorkVelociy;
	}
	if (bZDisType) //��ϥά۹��m���ɭ�
	{
		LONG lbuf = m_TablelZ - lZBackDistance;//�q���x�̧C���W�۹�Z��
		lZBackDistance = lbuf;
		bZDisType = 0;//�אּ�����m
	}
	if (!bZDisType) //�����m
	{
		if (lZBackDistance > MO_ReadLogicPosition(2))
		{
			lZBackDistance = MO_ReadLogicPosition(2);
		}
		lZBackDistance = abs(lZBackDistance - MO_ReadLogicPosition(2));
	}
	CCircleFormula Circle;
	AxeSpace p1, p2, p3;
	p1 = { W_GetNeedlePoint().x, W_GetNeedlePoint().y,MO_ReadLogicPosition(2),MO_ReadLogicPositionW() };
	p2 = { lX1,lY1,lZ1,dAng1 };
	p3 = { lX2,lY2,lZ2,dAng2 };
	DPoint pCen, dp1, dp2, dp3;
	dp1 = { (DOUBLE)W_GetNeedlePoint().x,(DOUBLE)W_GetNeedlePoint().y,(DOUBLE)MO_ReadLogicPosition(2),MO_ReadLogicPositionW() };
	dp2 = { (DOUBLE)lX1,(DOUBLE)lY1,(DOUBLE)lZ1,dAng1 };
	dp3 = { (DOUBLE)lX2,(DOUBLE)lY2,(DOUBLE)lZ2,dAng2 };
	std::vector<AxeSpace> VecBuf;
	LONG cutRad = 1000;//�H1mm�A�]�N�O1000um�����������I
	if (lZ1 == lZ2&&lZ1 == MO_ReadLogicPosition(2))
	{
		//�D�G����߻P�b�|
		pCen = Circle.ArcCentCalculation_2D(dp1, dp2, dp3, dRad, angle);
		Circle.CircleCutPath_2D_unit(p1, p2, p3, FALSE, cutRad, VecBuf);//2D����
	}
	else
	{
		//�D�T����߻P�b�|
		pCen = Circle.SpaceCircleCenterCalculation(dp1, dp2, dp3, dRad, angle);
		Circle.m_cuttingLength = cutRad;//���꩷����
		Circle.ArcCutPoint(p1, p2, p3, VecBuf); //3D����; VecBuf�s�w�Y����y��
	}
	DOUBLE SumAng = dAng2 - MO_ReadLogicPositionW();//W�j�ਤ�׶q(�t+/-��V��)
	size_t num = VecBuf.size();
	DOUBLE num_Ang = fabs(SumAng) / (DOUBLE)num;//�̶꩷�����I�Ƥ��ΰj�ਤ�׶q
	num_Ang = floor(num_Ang / 0.036)*0.036;//�̰��F 0.036 [deg/pulse] �ഫ���X�z�����פ��q
	DOUBLE rest_Ang = (fabs(SumAng) - num_Ang*(DOUBLE)num)*((SumAng>0) ? 1 : -1);//�Ѿl������
	DATA_4MOVE *pDataM = new DATA_4MOVE[num];
	DATA_4MOVE *pDataShift = pDataM;
	CPoint cpMpbuf(0, 0), cpMpLast(0, 0);
	int Fin_cnt = (int)floor(fabs(rest_Ang) / 0.072);//�̫�Ӽ�
	DOUBLE absAngle = MO_ReadLogicPositionW();

	//���񵴹�y��=>����۹�y��
	for (size_t i = 0; i<VecBuf.size(); i++)
	{
		if (Fin_cnt != 0)
		{
			//Get ���﨤��
			absAngle = absAngle + ((SumAng > 0) ? num_Ang + 0.072 : -num_Ang - 0.072);
			pDataShift->AngleW = ((SumAng > 0) ? num_Ang + 0.072 : -num_Ang - 0.072);
			Fin_cnt--;
		}
		else
		{
			absAngle = absAngle + ((SumAng > 0) ? num_Ang : -num_Ang);
			pDataShift->AngleW = ((SumAng > 0) ? num_Ang : -num_Ang);
		}
		//Get ���񵴹�y��
		cpMpbuf = W_GetMachinePoint(VecBuf.at(i).x, VecBuf.at(i).y, absAngle, 1);

		//�i�浴��/�۹�y���ഫ
		if (i == 0)
		{
			pDataShift->EndPX = cpMpbuf.x - MO_ReadLogicPosition(0);
			pDataShift->EndPY = cpMpbuf.y - MO_ReadLogicPosition(1);
			pDataShift->EndPZ = VecBuf.at(i).z - MO_ReadLogicPosition(2);
			pDataShift->Distance = (LONG)sqrt(pow(pDataShift->EndPX, 2) + pow(pDataShift->EndPY, 2) + pow(pDataShift->EndPZ, 2));
			//_cwprintf(_T("%d,%d,%.3f\n"), pDataShift->EndPX, pDataShift->EndPY, pDataShift->AngleW);
			//TRACE(_T(",%d,%d,%.3f\n"), pDataShift->EndPX, pDataShift->EndPY, pDataShift->AngleW);
		}
		else
		{
			pDataShift->EndPX = cpMpbuf.x - cpMpLast.x;
			pDataShift->EndPY = cpMpbuf.y - cpMpLast.y;
			pDataShift->EndPZ = VecBuf.at(i).z - VecBuf.at(i - 1).z;
			pDataShift->Distance = (LONG)sqrt(pow(pDataShift->EndPX, 2) + pow(pDataShift->EndPY, 2) + pow(pDataShift->EndPZ, 2));
			if ((i == VecBuf.size() - 2) || (i == VecBuf.size() - 1))
			{
				_cwprintf(_T("lost_%d,%d,%.3f\n"), pDataShift->EndPX, pDataShift->EndPY, pDataShift->AngleW);
			}
			//_cwprintf(_T("%d,%d,%.3f\n"), pDataShift->EndPX, pDataShift->EndPY, pDataShift->AngleW);
			// TRACE(_T(",%d,%d,%.3f\n"), pDataShift->EndPX, pDataShift->EndPY, pDataShift->AngleW);
		}
		//TRACE(_T(",%d,%d,%d,%.3f\n"), pDataShift->EndPX, pDataShift->EndPY, pDataShift->EndPZ, pDataShift->AngleW);
		//TRACE(_T(",%d,%d,%d,%.3f\n"), VecBuf.at(i).x, VecBuf.at(i).y, VecBuf.at(i).z, VecBuf.at(i).w);
		//_cwprintf(_T(",%d,%d,%d,%.3f\n"), pDataShift->EndPX, pDataShift->EndPY, pDataShift->EndPZ, pDataShift->AngleW);
		cpMpLast = cpMpbuf;
		pDataShift++;
	}
	PauseDoGlue();//�Ȱ���_���~��X��(m_bIsPause=0)
	if (lCloseDistance != 0)  //�u�q�I���]�w---(5)�����Z��
	{
		dSumPath = M_PI * dRad * (angle / 180.0);//���ʪ��`����
		if (lCloseDistance != 0 && lCloseDistance > (LONG)round(dSumPath))
		{
			lCloseDistance = (LONG)round(dSumPath);
		}
		dCloseTime = 1000000 * (((LONG)round(dSumPath) - lCloseDistance) / (DOUBLE)lWorkVelociy);
		/*======�p�ɾ���Ĳ�o���_�����_���A�ϥ�z���_����================*/
		if (!m_bIsStop)
		{
			CAction::m_ZtimeOutGlueSet = FALSE;
			_cwprintf(_T("End�����Z�����ɶ�=%l.3f \n"), DOUBLE(dCloseTime / 1000000.0));
			MO_TimerSetIntter((LONG)round(dCloseTime), 1);//�p�ɨ���ܰ����
		}
	}
	if (!m_bIsStop)
	{
		MO_DO4Curve(pDataM, num, lWorkVelociy);//����|�b����
		PreventMoveError();//�����X�ʿ��~
	}
	pDataShift = NULL;
	delete[] pDataM;
	if (lCloseDistance == 0) //��S�������Z���ɦ���������N�|����
	{
		MO_Timer(0, 0, lCloseONDelayTime * 1000);
		MO_Timer(1, 0, lCloseONDelayTime * 1000);//�u�q�I���]�w---(6)��������
		Sleep(1);//����X���A�קK�p�ɾ��쪽��0
		while (MO_Timer(3, 0, 0))
		{
			if (m_bIsStop == 1)
			{
				break;
			}
			Sleep(1);
		}
	}
	MO_StopGumming();//����X��
					 //��^�]�w���Ѧ��I(�����I�Olx3,ly3,�X�o�Ѧ��I�ϥζ�2)
	GelatinizeBack(iType, lX2, lY2, MO_ReadLogicPosition(2), lX1, lY1, lDistance, lHigh, lZBackDistance, lLowVelocity, lHighVelocity, lAcceleration, lInitVelociy);//��^�]�w
	MO_Timer(0, 0, lCloseOffDelayTime * 1000);
	MO_Timer(1, 0, lCloseOffDelayTime * 1000);//�u�q�I���]�w---(3)���d�ɶ�
	Sleep(1);//����X���A�קK�p�ɾ��쪽��0
	while (MO_Timer(3, 0, 0))
	{
		if (m_bIsStop == 1)
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
void CAction::DecideVirtualPoint(LONG lX, LONG lY, LONG lZ, DOUBLE dAng, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy, BOOL bIsNeedleP)
{
    /*�����I(x�y�СAy�y�СAz�y�СAw����)
    LONG lX, LONG lY, LONG lZ, DOUBLE dAng
    */
    /*�t�ΰѼ�(�X�ʳt�סA�[�t�סA��t��)
    LONG lWorkVelociy,LONG lAcceleration, LONG lInitVelociy
    */
#ifdef MOVE
    //�P�_W�b�O�_�ե�
    if(m_MachineOffSet.x == -99999 && m_MachineOffSet.y == -99999)
    {
        return;
    }
    //�P�_�ϥΰw�Y�y���٬O����y��
    if(USEMODE_W == 1 && bIsNeedleP == 0) //�ϥξ���y��
    {
        CPoint cpbuf = W_GetNeedlePoint(lX, lY, dAng, 1);
        lX = cpbuf.x;
        lY = cpbuf.y;
    }
    /*******�H�U�{�����|���ܨϥΰw�Y�y�а���B��************************/
    //�y�{: ��J�w�Y�y��>�B��w�Y�y��>�ഫ������y�п�X
    CPoint cpNeedlePoint = W_GetNeedlePoint();//�N��U��m�ഫ���w�Y�y��
    W_UpdateNeedleMotor_Needle(cpNeedlePoint.x, cpNeedlePoint.y, MO_ReadLogicPosition(2), MO_ReadLogicPositionW(), lX, lY, lZ, dAng);//���I����(�ϥιw�]�C1�ר��@���I)
    PauseDoGlue();//�Ȱ���_���~��X��(m_bIsPause=0) �X��
    if(!m_bIsStop)
    {
        W_Line4DtoDo(lWorkVelociy, lAcceleration, lInitVelociy);//�|�b�s�򴡸�
        PreventMoveError();//����b�d�X��
    }
#endif
}
/*
*�����I�ʧ@
*��J(�����I�B�t�ΰѼ�)
*/
void CAction::DecideWaitPoint(LONG lX, LONG lY, LONG lZ, DOUBLE dAng, LONG lWaitTime, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
    /*�����I(x�y�СAy�y�СAz�y�СAw���סA���ݮɶ��A)
    LONG lX, LONG lY, LONG lZ , DOUBLE dAng,LONG lWaitTime
    */
    /*�t�ΰѼ�(�X�ʳt�סA�[�t�סA��t��)
    LONG lWorkVelociy,LONG lAcceleration, LONG lInitVelociy
    */
#ifdef MOVE
    //�P�_W�b�O�_�ե�
    if(m_MachineOffSet.x == -99999 && m_MachineOffSet.y == -99999)
    {
        return;
    }
    //�P�_�ϥΰw�Y�y���٬O����y��
    if(USEMODE_W == 1) //�ϥξ���y��
    {
        CPoint cpbuf = W_GetNeedlePoint(lX, lY, dAng, 1);
        lX = cpbuf.x;
        lY = cpbuf.y;
    }
    /*******�H�U�{�����|���ܨϥΰw�Y�y�а���B��************************/
    //�y�{: ��J�w�Y�y��>�B��w�Y�y��>�ഫ������y�п�X
    DecideVirtualPoint(lX, lY, lZ, dAng, lWorkVelociy, lAcceleration, lInitVelociy, 1);//�����I����
    MO_Timer(0, 0, lWaitTime * 1000);
    MO_Timer(1, 0, lWaitTime * 1000);//���ݮɶ�(us��ms)
    Sleep(1);//����X���A�קK�p�ɾ��쪽��0
    while(MO_Timer(3, 0, 0))
    {
        if(m_bIsStop == 1)
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
void CAction::DecideParkPoint(LONG lX, LONG lY, LONG lZ, DOUBLE dAng, LONG lTimeGlue, LONG lWaitTime, LONG lStayTime, LONG lZBackDistance, BOOL bZDisType, LONG lZdistance, LONG lHighVelocity, LONG lLowVelocity, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
    /*���n�I(x�y�СAy�y�СAz�y�СAw���סA�ƽ��ɶ��A�����ᵥ�ݮɶ��A)
    LONG lX, LONG lY, LONG lZ, dAng, LONG lTimeGlue,LONG lWaitTime
    */
    /*Z�b�u�@���׳]�w-Z�b�^�ɰ���(�۹�)�̰��I
    LONG lZBackDistance ,BOOL bZDisType(0�����m/1�۹��m)
    */
    /*�I�������]�w(Z�b�۹�Z���A���t�סA�C�t�סA)
    LONG lZdistance,LONG lHighVelocity,LONG lLowVelocity
    */
    /*�t�ΰѼ�(�X�ʳt�סA�[�t�סA��t��)
    LONG lWorkVelociy,LONG lAcceleration, LONG lInitVelociy
    */
#ifdef MOVE
    //�P�_W�b�O�_�ե�
    if(m_MachineOffSet.x == -99999 && m_MachineOffSet.y == -99999)
    {
        return;
    }
    //�P�_�ϥΰw�Y�y���٬O����y��
    if(USEMODE_W == 1) //�ϥξ���y��
    {
        CPoint cpbuf = W_GetNeedlePoint(lX, lY, dAng, 1);
        lX = cpbuf.x;
        lY = cpbuf.y;
    }
    /*******�H�U�{�����|���ܨϥΰw�Y�y�а���B��************************/
    //�y�{: ��J�w�Y�y��>�B��w�Y�y��>�ഫ������y�п�X
    //W_Rotation(dAng, lWorkVelociy, lAcceleration, lInitVelociy);//w�b����
    if(!m_bIsStop)
    {
        MO_Do4DLineMove(0, 0, 0, dAng - MO_ReadLogicPositionW(), (LONG)round(lWorkVelociy / m_WSpeed), (LONG)(lAcceleration / m_WSpeed), lInitVelociy);//����W
        PreventMoveError();//����b�d�X��
    }
    CPoint cpMpbuf(0, 0);
    cpMpbuf = W_GetMachinePoint(lX, lY, dAng, 1);
    lX = cpMpbuf.x;//�w�Y�y���ഫ������y��
    lY = cpMpbuf.y;//�w�Y�y���ഫ������y��

    if(lHighVelocity == 0)
    {
        lHighVelocity = lWorkVelociy;
    }
    if(lLowVelocity == 0)
    {
        lLowVelocity = lWorkVelociy;
    }
    if(bZDisType) //��ϥά۹��m���ɭ�
    {
        LONG lbuf = m_TablelZ - lZBackDistance;//�q���x�̧C���W�۹�Z��
        lZBackDistance = lbuf;
        bZDisType = 0;//�אּ�����m
    }
    if(!bZDisType)  //�����m
    {
        if(lZBackDistance > lZ)
        {
            lZBackDistance = lZ;
        }
        lZBackDistance = abs(lZBackDistance - lZ);
    }
    if(!m_bIsStop)
    {
        MO_Do3DLineMove(lX - MO_ReadLogicPosition(0), lY - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration, lInitVelociy);//x,y�b����
        PreventMoveError();//����b�d�X��
    }
    if(!m_bIsStop)
    {
        MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//z�b����
        PreventMoveError();//����b�d�X��
    }
    MO_Timer(0, 0, lWaitTime * 1000);
    MO_Timer(1, 0, 0);//(ms)
    Sleep(1);//����X���A�קK�p�ɾ��쪽��0
    while(MO_Timer(3, 0, 0))
    {
        if(m_bIsStop == 1)
        {
            MO_Timer(2, 0, 0);//����p�ɾ�
            break;
        }
        Sleep(1);
    }
    if(lTimeGlue == 0)
    {
        Sleep(1);
    }
    else
    {
        DoGlue(lTimeGlue, lStayTime);//����ƽ�
    }
    Sleep(10);//����{���X��
    //�ƽ����᪺�^�ɰʧ@
    if(!MO_ReadGumming())
    {
        if(lZdistance == 0)
        {
            if(!m_bIsStop)
            {
                MO_Do3DLineMove(0, 0, (lZ - lZBackDistance) - lZ, lWorkVelociy, lAcceleration,
                                lInitVelociy);//�I�������]�m
                PreventMoveError();//����b�d�X��
            }
        }
        else
        {
            if(lZdistance>lZBackDistance && lZBackDistance != 0)
            {
                lZdistance = lZBackDistance;
                MO_Do3DLineMove(0, 0, (lZ - lZBackDistance) - lZ, lHighVelocity, lAcceleration,
                                lInitVelociy);//�I�������]�m
                PreventMoveError();//����b�d�X��
            }
            else
            {
                if(!m_bIsStop)
                {
                    MO_Do3DLineMove(0, 0, (lZ - lZdistance) - lZ, lLowVelocity, lAcceleration,
                                    lInitVelociy);//�I�������]�m
                    PreventMoveError();//����b�d�X��
                }
                if(!m_bIsStop)
                {
                    MO_Do3DLineMove(0, 0, (lZ - lZBackDistance) - (lZ - lZdistance), lHighVelocity,
                                    lAcceleration, lInitVelociy);//�I�������]�m
                    PreventMoveError();//����b�d�X��
                }
            }

        }
    }
#endif
}

/*==���I���k�ʧ@
*@��J(LONG�t��1�ALONG�t��2�ALONG���w�b(0~15)�ALONG�����q)
*@�ϥ� m_HomeSpeed_INIT ���t�׶i��Z,W
*@��ĳ�t��:���I�_�k�t��1:30000 �t��2:2000 , m_HomeSpeed_INIT =2000
*/
void CAction::DecideInitializationMachine(LONG lSpeed1, LONG lSpeed2, LONG lAxis, LONG lMoveX, LONG lMoveY, LONG lMoveZ, DOUBLE dMoveW)
{
    /*�t�ΰѼ�(���I�_�k�t��1�A���I�_�k�t��2�A���w�_�k�b�A�����q)
    LONG lSpeed1,LONG lSpeed2, LONG lAxis, LONG lMove
    */
#ifdef MOVE
    //���ʼҦ�:�_�k�̧�:Z >> W >> X�BY
    MO_FinishGumming();
    MO_SetSoftLim(lAxis, 0);    //TODO::�n�鷥����
    MO_SetHardLim(lAxis, 1);
    if(lAxis > 3 && lAxis != 8 && lAxis != 9 && lAxis != 10 && lAxis != 11)
    {
        MO_MoveToHome(m_HomeSpeed_DEF, lSpeed2, 4, 0, 0, lMoveZ);//*******************�_�k�t�ר̷� m_HomeSpeed_INIT �����X�ʳt��
        PreventMoveError();//����b�d�X��
        MO_SetSoftLim(4, 1);
        lAxis = lAxis - 4;
    }
    if(lAxis >= 10 || lAxis == 8 || lAxis == 9)
    {
        DecideGoHomeW(m_HomeSpeed_DEF, 500);//*******************�_�k�t�ר̷� m_HomeSpeed_INIT �����X�ʳt��,�G���q�t�׫�ĳ�Ȭ�500
        lAxis = lAxis - 8;
    }

    if(!m_bIsStop)
    {
        MO_MoveToHome(lSpeed1, lSpeed2, lAxis, lMoveX,lMoveY,0);
        PreventMoveError();//����b�d�X��
    }
    MO_SetSoftLim(lAxis, 1);
#endif
}
/*==W�b���I���k�ʧ@
*w�b�����0~360��
*/
void CAction::DecideGoHomeW(LONG lSpeed1, LONG lSpeed2)
{
    /*�t�ΰѼ�(���I�_�k�t��1�A���I�_�k�t��2)
    LONG lSpeed1,LONG lSpeed2
    */
#ifdef MOVE
    MO_FinishGumming();
    MO_SetSoftLim(8, 0);    //TODO::�n�鷥���n�O�o��
    MO_SetHardLim(8, 0);//�w�鷥����
    BOOL HMbuf[5] = { 0 };//�����T��
    //BOOL Errbuf[4] = { 0 };//�����}���G��
    //MO_ReadRunHardLim(HMbuf[0], Errbuf[0], HMbuf[1], Errbuf[1], HMbuf[2], Errbuf[2], HMbuf[3], Errbuf[3]);
    MO_ReadSoftLimError(0, HMbuf[0], HMbuf[1], HMbuf[2], HMbuf[3]);//+
    MO_ReadSoftLimError(1, HMbuf[0], HMbuf[1], HMbuf[2], HMbuf[4]);//-

    //���I�_�k�ɱ��઺��V
    if(MO_ReadLogicPositionW() < 0.0)
    {
        //�U�C�ʧ@���f�ɰw��((�t�f
        //AfxMessageBox(L"�{�b���פp��0");
        if (MO_ReadLogicPositionW() <= -340.0 || HMbuf[4] == 1)
        {
            //AfxMessageBox(L"���פp��-340");
            MO_AlarmCClean();//�Ѱ�
        }
        if (!m_bIsStop)
        {
            MO_Do4DLineMove(0, 0, 0, abs(MO_ReadLogicPositionW()) + 10, lSpeed1, LONG(lSpeed1*m_WSpeed), lSpeed2);
            PreventMoveError();
        }
    }
    else
    {
        //�U�C�ʧ@�����ɰw��((����
       // AfxMessageBox(L"�{�b���פj�󵥩�0");
        if (MO_ReadLogicPositionW() >= 340.0 || HMbuf[3] == 1)
        {
            //AfxMessageBox(L"���פj��340");
            MO_AlarmCClean();//�Ѱ�
            MO_Do4DLineMove(0, 0, 0, -90, lSpeed1, LONG(lSpeed1*m_WSpeed), lSpeed2);
            PreventMoveError();
        }
    }
    MO_SetHardLim(8, 1);//�w�鷥������
    if(!m_bIsStop)
    {
		BOOL type = 0;//w�b�����V--------------�f������V�^�k(�ϥ�-�����T��  +�������a)
        MO_MoveToHomeW(lSpeed1, lSpeed2, 8, 0, 0, 0, 0, type);
        PreventMoveError();//����b�d�X��
    }
    MO_SetHardLim(8, 0);//W�b�w�鷥������
    MO_SetSoftLim(8, 1);//�n�鷥���}
#endif
}
/*
*bZDisType  TRUE:�۹�y��/FALSE:����y��
*/
void CAction::DecideVirtualHome(LONG lX, LONG lY, LONG lZ, LONG lZBackDistance, BOOL bZDisType, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
#ifdef MOVE
    //�P�_W�b�O�_�ե�
    if(m_MachineOffSet.x == -99999 && m_MachineOffSet.y == -99999)
    {
        return;
    }
    //�P�_�ϥΰw�Y�y���٬O����y��
    if(USEMODE_W == 1) //�ϥξ���y��
    {
        CPoint cpbuf = W_GetNeedlePoint(lX, lY, 0, 1);
        lX = cpbuf.x;
        lY = cpbuf.y;
    }
    /*******�H�U�{�����|���ܨϥΰw�Y�y�а���B��************************/
    //�y�{: ��J�w�Y�y��>�B��w�Y�y��>�ഫ������y�п�X
    CPoint cpMpbuf(0, 0);
    DecideGoHomeW(lWorkVelociy, 2000);//w�b�_�k
    cpMpbuf = W_GetMachinePoint(lX, lY, 0, 1);
    lX = cpMpbuf.x;//�w�Y�y���ഫ������y��
    lY = cpMpbuf.y;//�w�Y�y���ഫ������y��
    if(lZBackDistance <= 0)
    {
        return;
    }
    if(bZDisType) //��ϥά۹��m���ɭ�
    {
        LONG lbuf = m_TablelZ - lZBackDistance;//�q���x�̧C���W�۹�Z��
        lZBackDistance = lbuf;
        bZDisType = 0;//�אּ�����m
    }
    //����y��
    if(!m_bIsStop)
    {
        if(lZBackDistance < MO_ReadLogicPosition(2))
        {
            //z go back
            MO_Do3DLineMove(0, 0, lZBackDistance - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);
            PreventMoveError();//����b�d�X��
        }
    }
    if(!m_bIsStop)
    {
        DecideGoHomeW(lWorkVelociy, lInitVelociy);
    }
    if(!m_bIsStop)
    {
        //xyz move
        MO_Do3DLineMove(lX - MO_ReadLogicPosition(0), lY - MO_ReadLogicPosition(1), lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);
        PreventMoveError();//����b�d�X��
    }
#endif
}
/*
�w�Y�M��˸m
*/
void CAction::DispenClear(LONG lX, LONG lY, LONG lZ, int ClreaPort, LONG lZBackDistance, BOOL bZDisType, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{

    /*�Ѽƻ���
    �w�Y�M��˸m��m
    LONG lX,LONG lY,LONG lZ
    �˸m�ϥΪ���XIO�}��(��J0~7)
    int ClreaPort
    Z�b�u�@���׳]�w-Z�b�^�ɰ���(�۹�)�̰��I
    LONG lZBackDistance ,BOOL bZDisType(0�����m/1�۹��m)
    �t�ΰѼ�(�X�ʳt�סA�[�t�סA��t��)
    LONG lWorkVelociy,LONG lAcceleration, LONG lInitVelociy
    */
#ifdef MOVE
    //�P�_W�b�O�_�ե�
    if(m_MachineOffSet.x == -99999 && m_MachineOffSet.y == -99999)
    {
        return;
    }
    //�P�_�ϥΰw�Y�y���٬O����y��
    if(USEMODE_W == 1) //�ϥξ���y��
    {
        CPoint cpbuf = W_GetNeedlePoint(lX, lY, 0, 1);
        lX = cpbuf.x;
        lY = cpbuf.y;
    }
    /*******�H�U�{�����|���ܨϥΰw�Y�y�а���B��************************/
    //�y�{: ��J�w�Y�y��>�B��w�Y�y��>�ഫ������y�п�X
    CPoint cpMpbuf(0, 0);
    DecideGoHomeW(lWorkVelociy, 2000);//w�b�_�k
    cpMpbuf = W_GetMachinePoint(lX, lY, 0, 1);
    lX = cpMpbuf.x;//�w�Y�y���ഫ������y��
    lY = cpMpbuf.y;//�w�Y�y���ഫ������y��
    if(bZDisType) //��ϥά۹��m���ɭ�
    {
        LONG lbuf = m_TablelZ - lZBackDistance;//�q���x�̧C���W�۹�Z��
        lZBackDistance = lbuf;
        bZDisType = 0;//�אּ�����m
    }
    if(!bZDisType)  //�����m
    {
        if(lZBackDistance > lZ)
        {
            lZBackDistance = lZ;
        }
        lZBackDistance = abs(lZBackDistance - lZ);
    }
    if(lZBackDistance > lZ)
    {
        lZBackDistance = lZ;
    }
    //����X,Y,Z�ܰw�Y�M��˸m��m
    if(!m_bIsStop)
    {
        MO_Do3DLineMove(lX - MO_ReadLogicPosition(0), lY - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration, lInitVelociy);//x,y�b����
        PreventMoveError();//����b�d�X��
    }
    if(!m_bIsStop)
    {
        MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//z�b����
        PreventMoveError();//����b�d�X��
    }
    //�Ұʰw�Y�M��io�˸m
    DecideOutPutSign(ClreaPort, 1);
    //������I�I��
    DoGlue(1000, 1000);//�X���@����@��
    Sleep(10);
    //�����w�Y�M��io�˸m
    DecideOutPutSign(ClreaPort, 0);
    //z�b�^�ɨ�̰��I
    if(!m_bIsStop)
    {
        MO_Do3DLineMove(0, 0, lZBackDistance - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//z�b����
        PreventMoveError();//����b�d�X��
    }
#endif
}
/*
*��R�ʧ@
*��J(�u�q�}�l�A�u�q�����AZ�b�u�@���׳]�w�A��R�R�O�A�t�ΰѼ�)
*/
void CAction::DecideFill(LONG lX1, LONG lY1, LONG lZ1, LONG lX2, LONG lY2, LONG lZ2,
                         LONG lZBackDistance, BOOL bZDisType, int iType, LONG lWidth, LONG lWidth2,
                         LONG lStartDelayTime, LONG lStartDistance, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime,
                         LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
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
    /*�u�q�I���]�w(1.���ʫe����A2.�]�m�Z���A3���d�ɶ��A5�����Z���A6��������)
    LONG lStartDelayTime ,LONG lStartDistance ,LONG lCloseOffDelayTime ,LONG lCloseDistance ,LONG lCloseONDelayTime
    //1.���ʫe�I�����b�@���u�q�_�l�I�B�O�����}���ɪ��C �����ɥi����w�Y�b�y��y�ʤ��e�u�u�q�o�Ͳ��ʡC
    //2. �I�����}�ҫe�A ���F���}���u�u�q�_�l�I�����ʶZ���C �ӶZ�������F���ѤF�������_�t�ɶ��A�D�n�ΨӮ����L�q�y��b�u�q�_�l�B���n�E�C
    //3.�I����������A���F�����O�b�w�Y���ܤU�@�I�e�ܱo���ŦӦb�u�q�I�������I�B���ͪ����ɡC
    //5.������L�q�y��b�u�q�����I�B�o�Ͱ�n�A�I�����b�Z���u�q�����I�e�h���B�����C
    //6.�I�����b�u�q�����I�B�����O���}�Ҫ��ɪ��C
    */
#ifdef MOVE
    //�P�_W�b�O�_�ե�
    if(m_MachineOffSet.x == -99999 && m_MachineOffSet.y == -99999)
    {
        return;
    }
    //�P�_�ϥΰw�Y�y���٬O����y��
    if(USEMODE_W == 1) //�ϥξ���y��
    {
        CPoint cpbuf = W_GetNeedlePoint(lX1, lY1, MO_ReadLogicPositionW(), 1);
        lX1 = cpbuf.x;
        lY1 = cpbuf.y;
        cpbuf = W_GetNeedlePoint(lX2, lY2, MO_ReadLogicPositionW(), 1);
        lX2 = cpbuf.x;
        lY2 = cpbuf.y;
    }
    /*******�H�U�{�����|���ܨϥΰw�Y�y�а���B��************************/
    //�y�{: ��J�w�Y�y��>�B��w�Y�y��>�ഫ������y�п�X
    CPoint cpMpbuf(0, 0);
    cpMpbuf = W_GetMachinePoint(lX1, lY1, MO_ReadLogicPositionW(), 1);
    lX1 = cpMpbuf.x;
    lY1 = cpMpbuf.y;
    cpMpbuf = W_GetMachinePoint(lX2, lY2, MO_ReadLogicPositionW(), 1);
    lX2 = cpMpbuf.x;
    lY2 = cpMpbuf.y;
    if(lStartDelayTime > 0 && lStartDistance > 0)
    {
        lStartDistance = 0;
    }
    if(lCloseDistance > 0 && lCloseONDelayTime > 0)
    {
        lCloseONDelayTime = 0;
    }
    if(bZDisType) //��ϥά۹��m���ɭ�
    {
        LONG lbuf = m_TablelZ - lZBackDistance;//�q���x�̧C���W�۹�Z��
        lZBackDistance = lbuf;
        bZDisType = 0;//�אּ�����m
    }
    if(!bZDisType)  //�����m
    {
        if(lZBackDistance > lZ1)
        {
            lZBackDistance = lZ1;
        }
        lZBackDistance = abs(lZBackDistance - lZ1);
    }
    if(lZBackDistance > lZ1)
    {
        lZBackDistance = lZ1;
    }
    if(lZ1 == lZ2)
    {
        switch(iType)
        {
            case 0:
            {
                break;
            }
            case 1:
            {
                AttachFillType1(lX1, lY1, lX2, lY2, lZ1, lZBackDistance, lWidth, lStartDelayTime, lStartDistance, lCloseOffDelayTime,
                                lCloseDistance, lCloseONDelayTime, lWorkVelociy, lAcceleration, lInitVelociy);
                break;
            }
            case 2:
            {
                AttachFillType2(lX1, lY1, lX2, lY2, lZ1, lZBackDistance, lWidth, lStartDelayTime, lStartDistance, lCloseOffDelayTime,
                                lCloseDistance, lCloseONDelayTime, lWorkVelociy, lAcceleration, lInitVelociy);
                break;
            }
            case 3:
            {
                AttachFillType3(lX1, lY1, lX2, lY2, lZ1, lZBackDistance, lWidth, lStartDelayTime, lStartDistance, lCloseOffDelayTime,
                                lCloseDistance, lCloseONDelayTime, lWorkVelociy, lAcceleration, lInitVelociy);
                break;
            }
            case 4:
            {
                AttachFillType4(lX1, lY1, lX2, lY2, lZ1, lZBackDistance, lWidth, lWidth2, lStartDelayTime, lStartDistance,
                                lCloseOffDelayTime, lCloseDistance, lCloseONDelayTime, lWorkVelociy, lAcceleration, lInitVelociy);
                break;
            }
            case 5:
            {
                AttachFillType5(lX1, lY1, lX2, lY2, lZ1, lZBackDistance, lWidth, lWidth2, lStartDelayTime, lStartDistance,
                                lCloseOffDelayTime, lCloseDistance, lCloseONDelayTime, lWorkVelociy, lAcceleration, lInitVelociy);
                break;
            }
            case 6:
            {
                AttachFillType6(lX1, lY1, lX2, lY2, lZ1, lZBackDistance, lWidth, lStartDelayTime, lStartDistance, lCloseOffDelayTime,
                                lCloseDistance, lCloseONDelayTime, lWorkVelociy, lAcceleration, lInitVelociy);
                break;
            }
            case 7:
            {
                AttachFillType7(lX1, lY1, lX2, lY2, lZ1, lZBackDistance, lWidth, lStartDelayTime, lStartDistance, lCloseOffDelayTime,
                                lCloseDistance, lCloseONDelayTime, lWorkVelociy, lAcceleration, lInitVelociy);
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
    if(MO_ReadPIOInput(iPort) == bChoose)
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
*��J�@��(X,Y,Z,W)�y�СA�|�^�Ǧ����{�b��m���۹�y��
*return CStirng(X,Y,Z,W)
*/
CString CAction::NowOffSet(LONG lX, LONG lY, LONG lZ,DOUBLE dAng)
{
#ifdef MOVE
    //�P�_�ϥΰw�Y�y���٬O����y��
    if(USEMODE_W == 1) //�ϥξ���y��
    {
        CPoint cpbuf = W_GetNeedlePoint(lX, lY, dAng, 1);
        lX = cpbuf.x;
        lY = cpbuf.y;
    }
    /*******�H�U�{�����|���ܨϥΰw�Y�y�а���B��************************/
    //�y�{: ��J�w�Y�y��>�B��w�Y�y��>�ഫ������y�п�X
    CPoint cpMpbuf(0, 0);
    cpMpbuf = W_GetMachinePoint(lX, lY, dAng, 1);
    lX = cpMpbuf.x;
    lY = cpMpbuf.y;

    LONG lNowX = 0, lNowY = 0, lNowZ = 0;
    CString csBuff = 0;
    lNowX = MO_ReadLogicPosition(0);
    lNowY = MO_ReadLogicPosition(1);
    lNowZ = MO_ReadLogicPosition(2);
    csBuff.Format(_T("%ld,%ld,%ld,%.3f"), (lNowX - lX), (lNowY - lY), (lNowZ - lZ),(MO_ReadLogicPositionW()-dAng));
    return csBuff;
#endif
#ifndef MOVE
    return NULL;
#endif
}
/*
*�^�ǥثe���u��m
*return CString(X,Y,Z,w)
*/
CString CAction::NowLocation()
{
    CString Location;
#ifdef MOVE
    if(USEMODE_W == 1) //�ϥξ���y��
    {
        Location.Format(_T("%d,%d,%d,%.3f"), MO_ReadLogicPosition(0), MO_ReadLogicPosition(1), MO_ReadLogicPosition(2), MO_ReadLogicPositionW());
    }
    else
    {
        Location.Format(_T("%d,%d,%d,%.3f"), W_GetNeedlePoint().x, W_GetNeedlePoint().y, MO_ReadLogicPosition(2), MO_ReadLogicPositionW());
    }
#endif
    return Location;
}

/*���槹���^(0,0,0,0)��m*/
void CAction::BackGOZero(LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
#ifdef MOVE
    //�P�_W�b�O�_�ե�
    if(m_MachineOffSet.x == -99999 && m_MachineOffSet.y == -99999)
    {
        return;
    }
    AxeSpace axe = m_HomingPoint;
    if(USEMODE_W == 1) //�ϥξ���y��
    {
        axe = { 0 };
    }
    if(!m_bIsStop)
    {
        MO_Do3DLineMove(0, 0, axe.z - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);
        PreventMoveError();
    }
    if(!m_bIsStop)
    {
        DecideGoHomeW(lWorkVelociy, lInitVelociy);//w�b�_�k
    }
    if(!m_bIsStop)
    {
        MO_Do3DLineMove(axe.x - MO_ReadLogicPosition(0), axe.y - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration, lInitVelociy);
        PreventMoveError();
    }
#endif
}
/*
*CCD���ʰʧ@
*��J(�ؼ��I�B�t�ΰѼ�)
*/
void CAction::DoCCDMove(LONG lX, LONG lY, LONG lZ, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
    /*�ؼ��I(x�y�СAy�y�СAz�y�СA)
    LONG lX, LONG lY, LONG lZ
    */
    /*�t�ΰѼ�(�X�ʳt�סA�[�t�סA��t��)
    LONG lWorkVelociy,LONG lAcceleration, LONG lInitVelociy
    */
#ifdef MOVE
    //�P�_W�b�O�_�ե�
    if(m_MachineOffSet.x == -99999 && m_MachineOffSet.y == -99999)
    {
        return;
    }
    //�P�_�ϥΰw�Y�y���٬O����y��
    if(USEMODE_W == 1) //�ϥξ���y��
    {
        CPoint cpbuf = W_GetNeedlePoint(lX, lY, MO_ReadLogicPositionW(), 1);
        lX = cpbuf.x;
        lY = cpbuf.y;
    }
    /*******�H�U�{�����|���ܨϥΰw�Y�y�а���B��************************/
    //�y�{: ��J�w�Y�y��>�B��w�Y�y��>�ഫ������y�п�X
    CPoint cpMpbuf(0, 0);
    cpMpbuf = W_GetMachinePoint(lX, lY, MO_ReadLogicPositionW(), 1);
    lX = cpMpbuf.x;
    lY = cpMpbuf.y;
    if(!m_bIsStop)
    {
        MO_Do3DLineMove(lX - MO_ReadLogicPosition(0), lY - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration, lInitVelociy);//x,y�b����
        PreventMoveError();//����b�d�X��
    }
    if(!m_bIsStop)
    {
        MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//z�b����
        PreventMoveError();//����b�d�X��
    }
#endif
}

//----------------------------------------------------
//�H���ϥ�API--�ϥΫe�Шϥέ��I�_�k�����q
//----------------------------------------------------
//�n��t����(x,y,z�w�]��-10)
void CAction::HMNegLim(LONG lX, LONG lY, LONG lZ, DOUBLE dW)
{
#ifdef MOVE
    MO_SetSoftLim(15, 1);
    MO_SetCompSoft(1, -lX, -lY, -lZ,-dW);
#endif
}
//�n�饿����(x,y,z)
void CAction::HMPosLim(LONG lX, LONG lY, LONG lZ,DOUBLE dW)
{
#ifdef MOVE
    MO_SetSoftLim(15, 1);
    MO_SetCompSoft(0, lX, lY, lZ, dW);
#endif
}
/*�H���Ψ��-���ʩR�O(Z�b��͡�W�b�����X,Y���ʡ�Z�b�U��)
*@��J�Ѽ�:x,y,z,w��m�B�X�ʳt�סB�[�t�סB��t��
*/
void CAction::HMGoPosition(LONG lX, LONG lY, LONG lZ, DOUBLE dW, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
    /*��m(x�y�СAy�y�СAz�y��)
    LONG lX, LONG lY, LONG lZ
    */
    /*W�b����
    DOUBLE dAng
    */
    /*�t�ΰѼ�(�X�ʳt�סA�[�t�סA��t��)
    LONG lWorkVelociy,LONG lAcceleration, LONG lInitVelociy
    */
#ifdef MOVE
    //�P�_W�b�O�_�ե�
    if(m_MachineOffSet.x == -99999 && m_MachineOffSet.y == -99999)
    {
        return;
    }
    //�P�_�ϥΰw�Y�y���٬O����y��
    if(USEMODE_W == 1) //�ϥξ���y��
    {
        CPoint cpbuf = W_GetNeedlePoint(lX, lY, dW, 1);
        lX = cpbuf.x;
        lY = cpbuf.y;
    }
    /*******�H�U�{�����|���ܨϥΰw�Y�y�а���B��************************/
    //�y�{: ��J�w�Y�y��>�B��w�Y�y��>�ഫ������y�п�X
    if(!m_bIsStop)
    {
        MO_Do3DLineMove(0, 0, 0 - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//z�b��ɦ̰ܳ��I
        PreventMoveError();//����b�d�X��
    }
    if(!m_bIsStop)
    {
        MO_Do4DLineMove(0, 0, 0, dW - MO_ReadLogicPositionW(), (LONG)round(lWorkVelociy / m_WSpeed), (LONG)(lAcceleration / m_WSpeed), lInitVelociy);//w�b����
        PreventMoveError();//����b�d�X��
    }
    CPoint cpRobot = W_GetMachinePoint(lX, lY, dW, 1);//�R�O�I������y��
    if(!m_bIsStop)
    {
        MO_Do3DLineMove(cpRobot.x - MO_ReadLogicPosition(0), cpRobot.y - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration, lInitVelociy);//x,y���ʨ�R�O�I
        PreventMoveError();//����b�d�X��
    }
    if(!m_bIsStop)
    {
        MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//z�b����
        PreventMoveError();//����b�d�X��
    }
#endif
}
/***********************************************************
**                                                        **
**          �B�ʼҲ�-�p�g3D���N���| (�����ʧ@�P�_)           **
**                                                        **
************************************************************/
/*==================�p�g���s=====================================
//�\�໡��:�w�Y���ʦ�B�I�AZ�b�ݭ���̧C(�n�������)�A���U���s�C
//�|�NB�I��T�x�s��DATA_3ZERO_B���C
*/
void CAction::LA_Butt_GoBPoint()
{
#ifdef LA
    LAS_GoBPoint(DATA_3ZERO_B);
#endif
}
/*==�\�໡��:�p�g���ʦ�B�I�A���U���s�C
//�|���oX,Y�������q�x�s��m_OffSetLaserX,m_OffSetLaserY�ܼƤ�*/
void CAction::LA_Butt_GoLAtoBPoint()
{
#ifdef LA
    CPoint cpbuf(0, 0);
    W_UpdateNeedleMotorOffset(cpbuf, MO_ReadLogicPositionW());//�{�b���ק@�����
    DATA_3ZERO_LA = LAS_GoLAtoBPoint(DATA_3ZERO_B);
    m_OffSetLaserX = DATA_3ZERO_LA.EndPX - cpbuf.x;
    m_OffSetLaserY = DATA_3ZERO_LA.EndPY - cpbuf.y;//�P��߰�offset
    //TODO::�p�gOFFSET�ק�b�o
    //m_OffSetLaserX = 48436;
    //m_OffSetLaserY = 0;

#endif
}

/*==�p�g��l��*/
void CAction::LA_SetInit()
{
#ifdef LA
    #pragma region Omron Laser

    if(m_hComm != NULL)
    {
        AfxMessageBox(L"Already Opened!");
        return;
    }

    m_hComm = CreateFile(ComportNo, GENERIC_READ | GENERIC_WRITE, 0, NULL,
                         OPEN_EXISTING, false ? FILE_FLAG_OVERLAPPED : 0, NULL);
    if(m_hComm == INVALID_HANDLE_VALUE)
    {
        // MessageBox(L"Open failed");
        return;
    }
    //LAS_SetInit(&m_hComm);
    LAS_SetInit();
    #pragma endregion //Omron
    ////LAS_SetInit();
    ////TODO::�p�gOFFSET�ק�b�o
    ////*************�ثe�g���ʧ@***************************************
    //m_OffSetLaserX = 40324;
    //m_OffSetLaserY = -647;
    //m_HeightLaserZero = 31079;//�p�g��Z�b�k�s�I����
    //m_OffSetLaserZ = 43274;//B�I��Z���ר�p�g�k�sZ���ת��첾��(+)
#endif
}

/*
*���I���y
*/
BOOL CAction::LA_Dot3D(LONG lX, LONG lY, LONG &lZ, LONG lWorkVelociy,LONG lAcceleration, LONG lInitVelociy)
{
#ifdef LA
    //�P�_W�b�O�_�ե�
    if(m_MachineOffSet.x == -99999 && m_MachineOffSet.y == -99999)
    {
        return FALSE;
    }
    //�P�_�ϥΰw�Y�y���٬O����y��
    if(USEMODE_W == 1) //�ϥξ���y��
    {
        CPoint cpbuf = W_GetNeedlePoint(lX, lY, MO_ReadLogicPositionW(), 1);
        lX = cpbuf.x;
        lY = cpbuf.y;
    }
    /*******�H�U�{�����|���ܨϥΰw�Y�y�а���B��************************/
    //�y�{: ��J�w�Y�y��>�B��w�Y�y��>�ഫ������y�п�X
    CPoint cpMpbuf(0, 0);
    cpMpbuf = W_GetMachinePoint(lX, lY, MO_ReadLogicPositionW(), 1);
    lX = cpMpbuf.x;
    lY = cpMpbuf.y;
    LONG lCalcData1;
    if(!m_bIsStop)
    {
        MO_Do3DLineMove(0, 0, m_HeightLaserZero - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);
        PreventMoveError();//����b�d�X�ʿ��~
    }
    if(!m_bIsStop)
    {
        MO_Do3DLineMove(lX - m_OffSetLaserX - MO_ReadLogicPosition(0), lY - m_OffSetLaserY - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration, lInitVelociy);
        PreventMoveError();//����b�d�X�ʿ��~
    }
    if(LAS_GetLaserData(lCalcData1))
    {
        if(lCalcData1 == LAS_LONGMIN)
        {
            lZ = MO_ReadLogicPosition(2);
            return FALSE;//�������(ffffff)
        }
        else
        {
            lZ = MO_ReadLogicPosition(2) - lCalcData1 + m_OffSetLaserZ;//30000���P���d��
            return TRUE;
        }
    }
    else
    {
        lZ = MO_ReadLogicPosition(2);
        return FALSE;//�������(ffffff)
    }
#endif
#ifndef LA
    return FALSE;
#endif
}

/*
*�p�g�W�s
*�p�g�ϥΫe�����i���k�s�P�����Ӫ��]�w�C
*lZ���Ȭ����x��p�g�Z����65mm�C
*/
BOOL CAction::LA_SetZero()
{
#ifdef LA
#ifdef MOVE
    if(LAS_SetZero()) //�����k�s�I
    {
        if(!m_bIsStop)
        {
            MO_Do3DLineMove(0, 0, -LAS_MEASURE_RANGE, 20000, 100000, 1000);//�W��3cm�A�����p�g�k�s
            PreventMoveError();//����b�d�X��
        }
        Sleep(1000);
        if(LAS_SetZero())
        {
            m_HeightLaserZero = MO_ReadLogicPosition(2);//�p�g��Z�b�k�s�I����
            m_OffSetLaserZ = DATA_3ZERO_B.EndPZ - m_HeightLaserZero;//B�I��Z���ר�p�g�k�sZ���ת��첾��(+)
            return TRUE;//�p�g�k�s����
        }
        else
        {
            AfxMessageBox(L"�k�s���פ����T_�۰ʭץ�&�ЦA������");
            return FALSE;
        }
    }
    else
    {
        AfxMessageBox(L"�k�s���~");
        return FALSE;//z�b���פ����T�p�g�L�k���o��
    }
#endif
#endif
#ifndef LA
    return FALSE;
#endif
#ifndef MOVE
    return FALSE;
#endif
}

/*��b�s�򴡸�*/
//�H(lx3,ly3)�������I
void CAction::LA_Do2DVetor(LONG lX3, LONG lY3, LONG lX2, LONG lY2, LONG lX1, LONG lY1)
{
#ifdef MOVE
    //�P�_W�b�O�_�ե�
    if(m_MachineOffSet.x == -99999 && m_MachineOffSet.y == -99999)
    {
        return;
    }
    CPoint cpbuf[2] = { 0 };
    //�P�_�ϥΰw�Y�y���٬O����y��
    if(USEMODE_W == 1) //�ϥξ���y��
    {
        cpbuf[0] = W_GetNeedlePoint(lX1, lY1, MO_ReadLogicPositionW(), 1);
        cpbuf[1] = W_GetNeedlePoint(lX2, lY2, MO_ReadLogicPositionW(), 1);
        CPoint cpBuff = W_GetNeedlePoint(lX3, lY3, MO_ReadLogicPositionW(), 1);
        lX3 = cpBuff.x;
        lY3 = cpBuff.y;
    }
    /*******�H�U�{�����|���ܨϥΰw�Y�y�а���B��************************/
    //�y�{: ��J�w�Y�y��>�B��w�Y�y��>�ഫ������y�п�X
    BOOL bType = 0;
    DATA_2MOVE DATA_2D;
    CPoint cpMpbuf(0, 0);
    if(lX1 == 0 && lY1 == 0 && lX2 == 0 && lY2 == 0)
    {
        //���u
        bType = 0;
        DATA_2D.CirCentP.x = 0;
        DATA_2D.CirCentP.y = 0;
        DATA_2D.CirRev = 0;
    }
    else
    {
        //�꩷
        bType = 1;
        CString csbuff;
        CString csP1, csP2, csP3;
        LONG machine[2] = { 0 };
        csP1.Format(L"%d�A%d�A", cpbuf[0].x, cpbuf[0].y);
        csP2.Format(L"%d�A%d�A", cpbuf[1].x, cpbuf[1].y);
        csP3.Format(L"%d�A%d�A", lX3, lY3);
        csbuff = TRoundCCalculation(csP1, csP3, csP2);
        machine[0] = CStringToLong(csbuff, 0);
        machine[1] = CStringToLong(csbuff, 1);
        cpMpbuf = W_GetMachinePoint(machine[0], machine[1], MO_ReadLogicPositionW(), 1);
        DATA_2D.CirCentP.x = cpMpbuf.x;
        DATA_2D.CirCentP.y = cpMpbuf.y;
        DATA_2D.CirRev = CStringToLong(csbuff, 2);//���o���(X�AY�ARev�A)
    }
    cpMpbuf = W_GetMachinePoint(lX3, lY3, MO_ReadLogicPositionW(), 1);
    DATA_2D.EndP.x = cpMpbuf.x;
    DATA_2D.EndP.y = cpMpbuf.y;
    DATA_2D.Type = bType;
    DATA_2D.Speed = 0;
    LA_m_ptVec2D.push_back(DATA_2D);
#endif
}
/*��b�u�q�I*/
void CAction::LA_Do2dDataLine(LONG EndPX, LONG EndPY, BOOL bIsNeedleP)
{
#ifdef MOVE
    //�P�_W�b�O�_�ե�
    if(m_MachineOffSet.x == -99999 && m_MachineOffSet.y == -99999)
    {
        return;
    }
    //�P�_�ϥΰw�Y�y���٬O����y��
    if(USEMODE_W == 1 && bIsNeedleP == 0) //�ϥξ���y��
    {
        CPoint cpbuf = W_GetNeedlePoint(EndPX, EndPY, MO_ReadLogicPositionW(), 1);
        EndPX = cpbuf.x;
        EndPY = cpbuf.y;
    }
    /*******�H�U�{�����|���ܨϥΰw�Y�y�а���B��************************/
    //�y�{: ��J�w�Y�y��>�B��w�Y�y��>�ഫ������y�п�X
    DATA_2MOVE DATA_2D;
    CPoint cpMpbuf(0, 0);
    cpMpbuf = W_GetMachinePoint(EndPX, EndPY, MO_ReadLogicPositionW(), 1);
    DATA_2D.EndP.x = cpMpbuf.x;
    DATA_2D.EndP.y = cpMpbuf.y;
    DATA_2D.Type = 0;//���u
    DATA_2D.Speed = 0;
    DATA_2D.CirCentP.x = 0;
    DATA_2D.CirCentP.y = 0;
    DATA_2D.CirRev = 0;
    LA_m_ptVec2D.push_back(DATA_2D);
#endif
}
/*��b�u�q�꩷*/
void CAction::LA_Do2dDataArc(LONG EndPX, LONG EndPY, LONG ArcX, LONG ArcY)
{
#ifdef MOVE
    //�P�_W�b�O�_�ե�
    if(m_MachineOffSet.x == -99999 && m_MachineOffSet.y == -99999)
    {
        return;
    }
    if(LA_m_ptVec2D.empty())
    {
        return;
    }
    if(USEMODE_W == 1) //�ϥξ���y��
    {
        CPoint cpbuf = W_GetNeedlePoint(EndPX, EndPY, MO_ReadLogicPositionW(), 1);
        EndPX = cpbuf.x;
        EndPY = cpbuf.y;
        cpbuf = W_GetNeedlePoint(ArcX, ArcY, MO_ReadLogicPositionW(), 1);
        ArcX = cpbuf.x;
        ArcY = cpbuf.y;
    }
    /*******�H�U�{�����|���ܨϥΰw�Y�y�а���B��************************/
    //�y�{: ��J�w�Y�y��>�B��w�Y�y��>�ഫ������y�п�X
    DATA_2MOVE DATA_2D;
    CString csbuff;
    CString csP1, csP2, csP3;
    LONG machine[2] = { 0 };
    CPoint cpMpbuf(0, 0);
    csP1.Format(L"%d�A%d�A", LA_m_ptVec2D.back().EndP.x, LA_m_ptVec2D.back().EndP.y);
    csP2.Format(L"%d�A%d�A", ArcX, ArcY);
    csP3.Format(L"%d�A%d�A", EndPX, EndPY);
    csbuff = TRoundCCalculation(csP1, csP3, csP2);
    DATA_2D.Type = 1;//��
    cpMpbuf = W_GetMachinePoint(EndPX, EndPY, MO_ReadLogicPositionW(), 1);
    DATA_2D.EndP.x = cpMpbuf.x;
    DATA_2D.EndP.y = cpMpbuf.y;
    machine[0] = CStringToLong(csbuff, 0);
    machine[1] = CStringToLong(csbuff, 1);
    cpMpbuf = W_GetMachinePoint(machine[0], machine[1], MO_ReadLogicPositionW(), 1);
    DATA_2D.CirCentP.x = cpMpbuf.x;
    DATA_2D.CirCentP.y = cpMpbuf.y;
    DATA_2D.CirRev = CStringToLong(csbuff, 2);//���o���(X�AY�ARev�A)
    LA_m_ptVec2D.push_back(DATA_2D);
#endif
}
/*��b�u�q��*/
void CAction::LA_Do2dDataCircle(LONG EndPX, LONG EndPY, LONG CirP1X, LONG CirP1Y, LONG CirP2X, LONG CirP2Y)
{
#ifdef MOVE
    //�P�_W�b�O�_�ե�
    if(m_MachineOffSet.x == -99999 && m_MachineOffSet.y == -99999)
    {
        return;
    }
    if(LA_m_ptVec2D.empty())
    {
        return;
    }
    if(USEMODE_W == 1) //�ϥξ���y��
    {
        CPoint cpbuf = W_GetNeedlePoint(EndPX, EndPY, MO_ReadLogicPositionW(), 1);
        EndPX = cpbuf.x;
        EndPY = cpbuf.y;
        cpbuf = W_GetNeedlePoint(CirP1X, CirP1Y, MO_ReadLogicPositionW(), 1);
        CirP1X = cpbuf.x;
        CirP1Y = cpbuf.y;
        cpbuf = W_GetNeedlePoint(CirP2X, CirP2Y, MO_ReadLogicPositionW(), 1);
        CirP2X = cpbuf.x;
        CirP2Y = cpbuf.y;
    }
    /*******�H�U�{�����|���ܨϥΰw�Y�y�а���B��************************/
    //�y�{: ��J�w�Y�y��>�B��w�Y�y��>�ഫ������y�п�X
    DATA_2MOVE DATA_2D;
    CString csbuff;
    CString csP1, csP2, csP3;
    LONG machine[2] = { 0 };
    CPoint cpMpbuf(0, 0);
    csP1.Format(L"%d�A%d�A", EndPX, EndPY);
    csP2.Format(L"%d�A%d�A", CirP1X, CirP1Y);
    csP3.Format(L"%d�A%d�A", CirP2X, CirP2Y);
    csbuff = TRoundCCalculation(csP1, csP3, csP2);
    DATA_2D.Type = 1;//��
    cpMpbuf = W_GetMachinePoint(EndPX, EndPY, MO_ReadLogicPositionW(), 1);
    DATA_2D.EndP.x = cpMpbuf.x;
    DATA_2D.EndP.y = cpMpbuf.y;
    machine[0] = CStringToLong(csbuff, 0);
    machine[1] = CStringToLong(csbuff, 1);
    cpMpbuf = W_GetMachinePoint(machine[0], machine[1], MO_ReadLogicPositionW(), 1);
    DATA_2D.CirCentP.x = cpMpbuf.x;
    DATA_2D.CirCentP.y = cpMpbuf.y;
    DATA_2D.CirRev = CStringToLong(csbuff, 2);//���o���(X�AY�ARev�A)
    LA_m_ptVec2D.push_back(DATA_2D);
#endif
}
/***********************************************************
**                                                        **
**          �B�ʼҲ�-�p�g3D���N���| (�����ʧ@�P�_)           **
**                                                        **
************************************************************/
#ifdef MOVE
/*����y����۹�y��3�b�s�򴡸ɨϥ�*/
void CAction::LA_AbsToOppo3Move(std::vector<DATA_3MOVE> &str)
{
    std::vector<DATA_3MOVE> vecBuf;
    DATA_3MOVE mData;
    LONG lNowX = MO_ReadLogicPosition(0), lNowY = MO_ReadLogicPosition(1), lNowZ = MO_ReadLogicPosition(2);
    vecBuf.clear();
    mData.EndPX = str.at(0).EndPX - lNowX;
    mData.EndPY = str.at(0).EndPY - lNowY;
    mData.EndPZ = str.at(0).EndPZ - lNowZ;
    mData.Distance = LONG(sqrt(pow(str.at(0).EndPX, 2) + pow(str.at(0).EndPY, 2)));
    vecBuf.push_back(mData);
    for(UINT i = 1; i < str.size(); i++)
    {
        if((str.at(i).EndPX == LA_SCANEND) && (str.at(i).EndPY == LA_SCANEND) && (str.at(i).EndPZ == LA_SCANEND))
        {
            mData.EndPX = str.at(i).EndPX;
            mData.EndPY = str.at(i).EndPY;
            mData.EndPZ = str.at(i).EndPZ;
            //vecBuf.push_back(mData);
            //�p�G-99999���O�̫�@�����
            if(i + 1 < str.size())
            {
                mData.EndPX = str.at(i + 1).EndPX - str.at(i - 1).EndPX;
                mData.EndPY = str.at(i + 1).EndPY - str.at(i - 1).EndPY;
                mData.EndPZ = str.at(i + 1).EndPZ - str.at(i - 1).EndPZ;
                vecBuf.push_back(mData);
            }
            i++;
        }
        else
        {
            mData.Speed = str.at(i).Speed;
            mData.EndPX = str.at(i).EndPX - str.at(i - 1).EndPX;
            mData.EndPY = str.at(i).EndPY - str.at(i - 1).EndPY;
            mData.EndPZ = str.at(i).EndPZ - str.at(i - 1).EndPZ;
            mData.Distance = LONG(sqrt(pow(mData.EndPX, 2) + pow(mData.EndPY, 2)));
            vecBuf.push_back(mData);
        }

    }
    str.clear();
    str.insert(str.end(), vecBuf.begin(), vecBuf.end());
    for(UINT i = 0; i < str.size() - 1; i++)
    {
        DATA_3Do[i] = { 0 };
        DATA_3Do[i] = str[i + 1];
    }
}
#endif
#ifdef MOVE
/*����y����۹�y��2�b�s�򴡸ɨϥ�*/
void CAction::LA_AbsToOppo2Move(std::vector<DATA_2MOVE> &str)
{
    std::vector<DATA_2MOVE> vecBuf;
    DATA_2MOVE mData;
    vecBuf.clear();
    mData.Type = str.at(0).Type;
    mData.EndP.x = str.at(0).EndP.x - MO_ReadLogicPosition(0);
    mData.EndP.y = str.at(0).EndP.y - MO_ReadLogicPosition(1);
    mData.CirCentP.x = str.at(0).CirCentP.x - MO_ReadLogicPosition(0);
    mData.CirCentP.y = str.at(0).CirCentP.y - MO_ReadLogicPosition(1);
    mData.CirRev = str.at(0).CirRev;
    mData.Distance = (LONG)round(sqrt(pow(str.at(0).EndP.x, 2) + pow(str.at(0).EndP.y, 2)));
    mData.Speed = 0;
    vecBuf.push_back(mData);
    for(UINT i = 1; i < str.size(); i++)
    {
        mData.Speed = str.at(i).Speed;
        mData.Type = str.at(i).Type;
        mData.EndP = str.at(i).EndP - str.at(i - 1).EndP;
        mData.Distance = (LONG)round(sqrt(pow(mData.EndP.x, 2) + pow(mData.EndP.y, 2)));
        if(str.at(i).Type)
        {
            mData.CirCentP = str.at(i).CirCentP - str.at(i - 1).EndP;
            mData.CirRev = str.at(i).CirRev;
        }
        else
        {
            mData.CirCentP = 0;
            mData.CirRev = 0;
        }
        vecBuf.push_back(mData);
    }
    str.clear();
    str.insert(str.end(), vecBuf.begin(), vecBuf.end());
}
#endif
//����
//RefX����I/OffSetX�����q(��쪺�����q)/Andgle���઺����/CameraToTipOffsetX (ccd��w�Y�������q)
void CAction::LA_CorrectLocation(LONG &PointX, LONG &PointY, LONG RefX, LONG RefY, DOUBLE OffSetX, DOUBLE OffSetY, DOUBLE Andgle, DOUBLE CameraToTipOffsetX, DOUBLE CameraToTipOffsetY, BOOL Mode, LONG lSubOffsetX, LONG lSubOffsetY)
{
    DOUBLE X, Y;
    DOUBLE PX, PY, RX, RY;
    PointX += lSubOffsetX;
    PointY += lSubOffsetY;
    RX = DOUBLE(RefX);
    RY = DOUBLE(RefY);
    //��v���оɡC�N�y���٭쬰��v���Ҧ�
    PX = DOUBLE(PointX) - CameraToTipOffsetX;
    PY = DOUBLE(PointY) - CameraToTipOffsetY;

    //�D���I������x�}�B�Ѧҭ��I�B�첾
    //�H��v�������ߡA���y�Эץ��C
    X = cos(Andgle* M_PI / 180)*(PX - RX) - sin(Andgle* M_PI / 180)*(PY - RY) + (RX + OffSetX);
    Y = sin(Andgle* M_PI / 180)*(PX - RX) + cos(Andgle* M_PI / 180)*(PY - RY) + (RY + OffSetY);
    //�̫��ܧ��I���w�Y�Ҧ�
    X = X + CameraToTipOffsetX;
    Y = Y + CameraToTipOffsetY;
    if(Mode)
    {
        //Camera ��v���Ҧ�
        //��v���M�I���w�Y�������Z���A�|�ˤ��J
        PointX = LONG(X - CameraToTipOffsetX + 0.5);
        PointY = LONG(Y - CameraToTipOffsetY + 0.5);
    }
    else
    {
        //Tip �I���w�Y�Ҧ��A�|�ˤ��J
        PointX = LONG(X + 0.5);
        PointY = LONG(Y + 0.5);
    }
}
/*
*�s��u�q����
*lStartVe, lStartAcc, lStartInitVe  ���ʨ챽�y�ҩl�I���X�ʳt�סB�[�t�סB��t��
*lWorkVelociy, lAcceleration, lInitVelociy  ���y���X�ʳt�סB�[�t�סB��t��
*/
void CAction::LA_Line2D(LONG lStartVe, LONG lStartAcc, LONG lStartInitVe, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
#ifdef LA
#ifdef MOVE
    //�P�_W�b�O�_�ե�
    if(m_MachineOffSet.x == -99999 && m_MachineOffSet.y == -99999)
    {
        return;
    }
    if(m_LaserAverage == FALSE)
    {
        m_LaserCnt++;
    }
    LA_AbsToOppo2Move(LA_m_ptVec2D);
    if(!m_bIsStop)
    {
        MO_Do3DLineMove(0, 0, m_HeightLaserZero - MO_ReadLogicPosition(2), lStartVe, lStartAcc, lStartInitVe);
        PreventMoveError();//�_�l�I�ǳƲ���
    }
    if(!m_bIsStop)
    {
        MO_Do3DLineMove(LA_m_ptVec2D.at(0).EndP.x - m_OffSetLaserX,
                        LA_m_ptVec2D.at(0).EndP.y - m_OffSetLaserY, 0, lStartVe, lStartAcc, lStartInitVe);
        PreventMoveError();//�_�l�I�ǳƲ���
    }

    /*����Ĳ�o�ɡA�������y*/
    if(m_bIsStop)
    {
        return;
    }

    MO_InterruptCase(1, 2);
    MO_InterruptCase(1, 3);
    MO_Timer(0, 100000);//�]�w�p�ɾ�(0.1sĲ�o�@��)
    /*���J�}�l�I�@��*/
    LONG lCalcData1;

    if(LAS_GetLaserData(lCalcData1))
    {
        if(lCalcData1 == LAS_LONGMIN)
        {
            g_LaserErrCnt++;
        }
        else
        {
            DATA_3Do[0].EndPX = MO_ReadLogicPosition(0) + m_OffSetLaserX;
            DATA_3Do[0].EndPY = MO_ReadLogicPosition(1) + m_OffSetLaserY;
            DATA_3Do[0].EndPZ = MO_ReadLogicPosition(2) - lCalcData1 + m_OffSetLaserZ;//30000���P���d��
            if(m_LaserAverage == FALSE)
            {
                LA_m_ptVec.push_back(DATA_3Do[0]);
            }
            if(m_LaserCnt == 1 && m_LaserAverage == FALSE)
            {
                LA_m_iVecSP.push_back(m_LaserCnt);//main
            }
        }
    }
    for(UINT i = 1; i < LA_m_ptVec2D.size(); i++)
    {
        DATA_2Do[i - 1] = LA_m_ptVec2D.at(i);
    }
    MO_DO2Curve(DATA_2Do, LA_m_ptVec2D.size() - 1, lWorkVelociy);
    PreventMoveError();
    Sleep(200);
    LA_m_ptVec2D.clear();
#endif
#endif
}
/*
�s��u�q�ʧ@--(�T�b�s�򴡸�)
*/
void CAction::LA_Line3DtoDo(int iData, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy, BOOL bDoAll)
{
#ifdef MOVE
    //�P�_W�b�O�_�ե�
    if(m_MachineOffSet.x == -99999 && m_MachineOffSet.y == -99999)
    {
        return;
    }
    /*����Ĳ�o�ɡA�������y*/
    if(m_bIsStop)
    {
        return;
    }

    if(iData <= 0)
    {
        return;
    }

    while(m_bIsGetLAend == FALSE);
    std::vector<DATA_3MOVE>::iterator LA_ptIter;//���N��
    std::vector<DATA_3MOVE>LA_Buff;//�u�q�ȧP�_
    LONG lNowZHigh = 0;
    LA_Buff.clear();
    if(bDoAll)
    {
        LA_Buff.assign(LA_m_ptVec.begin(), LA_m_ptVec.begin() + LA_m_iVecSP.at(1) - 1);
        lNowZHigh = m_HeightLaserZero - MO_ReadLogicPosition(2);//�۹��m
    }
    else
    {
        if((UINT)iData > LA_m_iVecSP.size())
        {
            return;
        }
        else if(iData == 1)
        {
            LA_Buff.assign(LA_m_ptVec.begin(), LA_m_ptVec.begin() + LA_m_iVecSP.at(1) - 1);
            lNowZHigh = m_HeightLaserZero - MO_ReadLogicPosition(2);//�۹��m
        }
        else
        {
            LA_Buff.assign(LA_m_ptVec.begin() + LA_m_iVecSP.at(iData - 1), LA_m_ptVec.begin() + LA_m_iVecSP.at(iData) - 1);
            lNowZHigh = 0;//�۹��m
        }
    }
    if(!m_bIsStop)
    {
        MO_Do3DLineMove(0, 0, lNowZHigh, lWorkVelociy, lAcceleration, lInitVelociy);//���y���צ^�h
        PreventMoveError();//�_�l�I�ǳƲ���
    }
    LA_AbsToOppo3Move(LA_Buff);
    if(!m_bIsStop)
    {
        LA_ptIter = LA_Buff.begin();
        MO_Do3DLineMove(LA_ptIter->EndPX, LA_ptIter->EndPY, 0, lWorkVelociy, lAcceleration, lInitVelociy);//���ʰ_�l�I
        PreventMoveError();//�����X�ʿ��~
    }
    if(!m_bIsStop)
    {
        LA_ptIter = LA_Buff.begin();
        MO_Do3DLineMove(0, 0, LA_ptIter->EndPZ, lWorkVelociy, lAcceleration, lInitVelociy);//z�b�U����_�l�I����
        PreventMoveError();//�����X�ʿ��~
    }
    if(!m_bIsStop)
    {
        if(!m_bIsStop && m_bIsDispend == 1)
        {
            MO_GummingSet();//�(���d)
        }
        MO_DO3Curve(DATA_3Do, LA_Buff.size() - 1, lWorkVelociy);//�s�򴡸ɶ}�l
        PreventMoveError();//�����X�ʿ��~
    }
#endif
}
/*
�p�g�M�����O
*/
void CAction::LA_Clear()
{
#ifdef MOVE
    LA_m_ptVec.clear();//�p�g�s����I�x�svector
    LA_m_ptVec2D.clear();//��b�s�򴡸�vector
    LA_m_iVecSP.clear();//�D�n�p�gvector(SP:Scan End)
    m_LaserCnt = 0; //�p�g�u�q�p�ƾ�(���y��)
#endif
}
/*
*�p�g��������
*�y��(lStrX, lStrY)���y��(lEndX, lEndY)
*&lZ    ���y���槹��g�JZ���������׭�
*lStartVe, lStartAcc, lStartInitVe  ���ʨ챽�y�ҩl�I���X�ʳt�סB�[�t�סB��t��
*lWorkVelociy, lAcceleration, lInitVelociy  ���y���X�ʳt�סB�[�t�סB��t��
*/
void CAction::LA_AverageZ(LONG lStrX, LONG lStrY, LONG lEndX, LONG lEndY, LONG &lZ, LONG lStartVe, LONG lStartAcc, LONG lStartInitVe, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
#ifdef LA
#ifdef MOVE
    //�P�_W�b�O�_�ե�
    if(m_MachineOffSet.x == -99999 && m_MachineOffSet.y == -99999)
    {
        return;
    }
    //�P�_�ϥΰw�Y�y���٬O����y��
    if(USEMODE_W == 1) //�ϥξ���y��
    {
        CPoint cpbuf = W_GetNeedlePoint(lStrX, lStrY, MO_ReadLogicPositionW(), 1);
        lStrX = cpbuf.x;
        lStrY = cpbuf.y;
        cpbuf = W_GetNeedlePoint(lEndX, lEndY, MO_ReadLogicPositionW(), 1);
        lEndX = cpbuf.x;
        lEndY = cpbuf.y;
    }
    /*******�H�U�{�����|���ܨϥΰw�Y�y�а���B��************************/
    //�y�{: ��J�w�Y�y��>�B��w�Y�y��>�ഫ������y�п�X
    m_LaserAverage = TRUE;
    m_LaserAveBuffZ = 0;
    LA_Do2dDataLine(lStrX, lStrY,1);
    LA_Do2dDataLine(lEndX, lEndY,1);
    LA_Line2D(lStartVe, lStartAcc, lStartInitVe, lWorkVelociy, lAcceleration, lInitVelociy);
    Sleep(200);
    lZ = m_LaserAveBuffZ;
    m_LaserAverage = FALSE;
#endif
#endif
}
//�ץ��[����s��u�q
void CAction::LA_CorrectVectorToDo(LONG  lWorkVelociy, LONG lAcceleration, LONG lInitVelociy, LONG RefX, LONG RefY, DOUBLE OffSetX, DOUBLE OffSetY, DOUBLE Andgle, DOUBLE CameraToTipOffsetX, DOUBLE CameraToTipOffsetY, BOOL Mode, LONG lSubOffsetX, LONG lSubOffsetY)
{
#ifdef MOVE
    //�P�_W�b�O�_�ե�
    if(m_MachineOffSet.x == -99999 && m_MachineOffSet.y == -99999)
    {
        return;
    }
    std::vector<DATA_3MOVE>::iterator LA_ptIter;//���N��
    std::vector<DATA_3MOVE>LA_Buff;//�u�q�ȧP�_
    LONG lNowZHigh = 0;
    m_LaserCnt = 1;
    //while (m_bIsGetLAend == FALSE);
    LA_Buff.clear();

    LA_Buff.assign(LA_m_ptVec.begin(), LA_m_ptVec.begin() + LA_m_iVecSP.at(1) - 1);
    lNowZHigh = m_HeightLaserZero - MO_ReadLogicPosition(2);//�۹��m
    for(UINT i = 0; i < LA_Buff.size(); i++)
    {
        if(!(LA_Buff.at(i).EndPX == LA_SCANEND && LA_m_ptVec.at(i).EndPY == LA_SCANEND))
        {
            LA_CorrectLocation(LA_Buff.at(i).EndPX, LA_Buff.at(i).EndPY, RefX, RefY, OffSetX, OffSetY, Andgle, CameraToTipOffsetX, CameraToTipOffsetY, Mode, lSubOffsetX, lSubOffsetY);
        }
    }
    //�N�Ȧb��^LA_m_ptVec
    if(!m_bIsStop)
    {
        MO_Do3DLineMove(0, 0, lNowZHigh, lWorkVelociy, lAcceleration, lInitVelociy);//���y���צ^�h
        PreventMoveError();//�_�l�I�ǳƲ���
    }

    LA_AbsToOppo3Move(LA_Buff);

    if(!m_bIsStop)
    {
        LA_ptIter = LA_Buff.begin();
        MO_Do3DLineMove(LA_ptIter->EndPX, LA_ptIter->EndPY, 0, lWorkVelociy, lAcceleration, lInitVelociy);//���ʰ_�l�I
        PreventMoveError();//�����X�ʿ��~
    }
    if(!m_bIsStop)
    {
        LA_ptIter = LA_Buff.begin();
        MO_Do3DLineMove(0, 0, LA_ptIter->EndPZ, lWorkVelociy, lAcceleration, lInitVelociy);//z�b�U����_�l�I����
        PreventMoveError();//�����X�ʿ��~
    }
    if(!m_bIsStop)
    {
        if(!m_bIsStop && m_bIsDispend == 1)
        {
            MO_GummingSet();//�(���d)
        }
        MO_DO3Curve(DATA_3Do, LA_Buff.size() - 1, lWorkVelociy);//�s�򴡸ɶ}�l
        PreventMoveError();//�����X�ʿ��~
    }
#endif
}
//��R��ܮ����̫�@�I�y��(EndX,EndY)
void CAction::Fill_EndPoint(LONG &lEndX, LONG &lEndY, LONG lX1, LONG lY1, LONG lZ1, LONG lX2, LONG lY2, LONG lZ2, int iType, LONG lWidth, LONG lWidth2)
{
    /*�u�q�}�l(x�y�СAy�y�СAz�y�СA�u�q�_�l�I�A)
    LONG lX1, LONG lY1, LONG lZ1
    */
    /*�u�q����(x�y�СAy�y�СAz�y�СA�u�q�����I�A)
    LONG lX2, LONG lY2, LONG lZ2
    */
    /*��R�R�O(��R�Φ�(1~7)�A�e��(mm)�A��ݼe��(mm)�A)
    int iType, LONG lWidth, LONG lWidth2
    */
    //�P�_W�b�O�_�ե�
#ifdef MOVE
    if(m_MachineOffSet.x == -99999 && m_MachineOffSet.y == -99999)
    {
        return;
    }
    //�P�_�ϥΰw�Y�y���٬O����y��
    if(USEMODE_W == 1) //�ϥξ���y��
    {
        CPoint cpbuf = W_GetNeedlePoint(lX1, lY1, MO_ReadLogicPositionW(), 1);
        lX1 = cpbuf.x;
        lY1 = cpbuf.y;
        cpbuf = W_GetNeedlePoint(lX2, lY2, MO_ReadLogicPositionW(), 1);
        lX2 = cpbuf.x;
        lY2 = cpbuf.y;
    }
    /*******�H�U�{�����|���ܨϥΰw�Y�y�а���B��************************/
    //�y�{: ��J�w�Y�y��>�B��w�Y�y��>�ഫ������y�п�X-----�a�}��m�̷ӿ�J�y�Ц^�ǹ������y��
    CPoint cpMpbuf(0, 0);
    cpMpbuf = W_GetMachinePoint(lX1, lY1, MO_ReadLogicPositionW(), 1);
    lX1 = cpMpbuf.x;
    lY1 = cpMpbuf.y;
    cpMpbuf = W_GetMachinePoint(lX2, lY2, MO_ReadLogicPositionW(), 1);
    lX2 = cpMpbuf.x;
    lY2 = cpMpbuf.y;
    LONG lBufX = 0, lBufY = 0;
    if(lZ1 == lZ2)
    {
        switch(iType)
        {
            case 0:
            {
                break;
            }
            case 1:
            {
                lEndX = lX2;
                lEndY = lY2;
                break;
            }
            case 2:
            {
                lEndX = lX2;
                lEndY = lY2;
                break;
            }
            case 3:
            {
                AttachFillType3_End(lBufX, lBufY, lX1, lY1, lX2, lY2, lWidth, lWidth2);
                lEndX = lBufX;
                lEndY = lBufY;
                break;
            }
            case 4:
            {
                AttachFillType4_End(lBufX, lBufY, lX1, lY1, lX2, lY2, lWidth, lWidth2);
                lEndX = lBufX;
                lEndY = lBufY;
                break;
            }
            case 5:
            {
                AttachFillType5_End(lBufX, lBufY, lX1, lY1, lX2, lY2, lWidth, lWidth2);
                lEndX = lBufX;
                lEndY = lBufY;
                break;
            }
            case 6:
            {
                lEndX = lX1;
                lEndY = lY1;
                break;
            }
            case 7:
            {
                lEndX = lX1;
                lEndY = lY1;
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
    //�̷өҿ�Ϊ����A(�w�Y�y�ЩάO����y�Ц^�Ǽƭ�!!!)
    if(USEMODE_W == 0) //�ϥΰw�Y�y��
    {
        CPoint cpbuf = W_GetNeedlePoint(lEndX, lEndY, MO_ReadLogicPositionW(), 1);
        lEndX = cpbuf.x;
        lEndY = cpbuf.y;
    }
#endif // MOVE
}

/***********************************************************
**                                                        **
**          �B�ʼҲ�-�����                                **
**                                                        **
************************************************************/
//3d���N���|�����
DWORD CAction::MoInterrupt(LPVOID param)
{
#ifdef MOVE
#ifdef PRINTF
  //  _cwprintf(L"%s\n", L"Interrupt");
#endif
    LONG RR1X, RR1Y, RR1Z, RR1U;
    MO_ReadEvent(&RR1X, &RR1Y, &RR1Z, &RR1U);
    if(RR1X & 0x0010) //�쥻��RR1X&0x0020 �X�ʶ}�l���_
    {
        g_LaserErrCnt = 0;//�p�ƪ�l��
        g_LaserNuCnt = 1;//�p�ƪ�l��
        //MO_Timer(0, 100000);//�]�w�p�ɾ�(0.1sĲ�o�@��)
        MO_Timer(1, 100000);//�Ұʭp�ɾ�
        ((CAction *)param)->m_getHeightFlag = TRUE;//�p�g�����X�СG���\����
        ((CAction *)param)->m_bIsGetLAend = FALSE;//���y�|������
    }
    if((RR1X & 0x0080) || (g_LaserErrCnt == 10)) //�쥻����((RR1X&0x0040)|| (g_LaserErrCnt == 10)) �X�ʵ������_
    {
#ifdef LA
#ifdef PRINTF
     //   _cwprintf(L"%s\n", L"�X�ʵ������_!");
#endif
        ((CAction *)param)->m_getHeightFlag = FALSE;
        //((CAction *)param)->m_bIsGetLAend = TRUE;
        if((g_LaserErrCnt >= 10))
        {
            AfxMessageBox(L"�p�g�I�Эץ�");
            return 0;
        }
        /*���J�����I�@��*/
        LONG lCalcData1;
        if(LAS_GetLaserData(lCalcData1))
        {
            if(lCalcData1 == LAS_LONGMIN)
            {
                g_LaserErrCnt++;
            }
            else//�p�g���쪺�Ȭ�(0~+30)
            {
                ((CAction *)param)->DATA_3Do[g_LaserNuCnt].EndPX = MO_ReadLogicPosition(0) + ((CAction *)param)->m_OffSetLaserX;
                ((CAction *)param)->DATA_3Do[g_LaserNuCnt].EndPY = MO_ReadLogicPosition(1) + ((CAction *)param)->m_OffSetLaserY;
                ((CAction *)param)->DATA_3Do[g_LaserNuCnt].EndPZ = MO_ReadLogicPosition(2) - lCalcData1 + ((CAction *)param)->m_OffSetLaserZ;//30000���P���d��
                if(((CAction *)param)->m_LaserAverage == TRUE)
                {
                    ((CAction *)param)->m_LaserAveBuffZ += ((CAction *)param)->DATA_3Do[g_LaserNuCnt].EndPZ;
                }
                else
                {
                    ((CAction *)param)->LA_m_ptVec.push_back(((CAction *)param)->DATA_3Do[g_LaserNuCnt]);
                }
                g_LaserNuCnt++;
            }
        }

        ((CAction *)param)->DATA_3Do[g_LaserNuCnt].EndPX = LA_SCANEND;//-99999���u�q����
        ((CAction *)param)->DATA_3Do[g_LaserNuCnt].EndPY = LA_SCANEND;//-99999���u�q����
        ((CAction *)param)->DATA_3Do[g_LaserNuCnt].EndPZ = LA_SCANEND;//-99999���u�q����

        if(((CAction *)param)->m_LaserAverage == TRUE)
        {
            ((CAction *)param)->m_LaserAveBuffZ = LONG(round(((DOUBLE)((CAction *)param)->m_LaserAveBuffZ) / (g_LaserNuCnt - 1)));
        }
        else
        {
            ((CAction *)param)->LA_m_ptVec.push_back(((CAction *)param)->DATA_3Do[g_LaserNuCnt]);
            ((CAction *)param)->LA_m_iVecSP.push_back(((CAction *)param)->LA_m_ptVec.size());
        }
#endif
        MO_Timer(2, 1000000);//�����p�ɾ�
        g_LaserErrCnt = 0;//�������~�p�ƾ��k�s
        g_LaserNuCnt = 1;//�p�ƪ�l��
        MO_InterruptCase(0, 2);//�������_
        MO_InterruptCase(0, 3);//�������_
        ((CAction *)param)->m_bIsGetLAend = TRUE;// ���y����
    }
    if(RR1Y & 0x0200)  //Y�p�ɾ����_(�X���P�_��)
    {
#ifdef PRINTF
    //    _cwprintf(L"y\n");
#endif
        if(!((CAction *)param)->m_bIsStop)
        {
            (CAction::m_YtimeOutGlueSet) ? MO_GummingSet() : MO_FinishGumming();
            CAction::m_YtimeOutGlueSet = FALSE;
        }
    }
    if(RR1Z & 0x0200)  //Z�p�ɾ����_
    {
#ifdef PRINTF
  //      _cwprintf(L"z\n");
#endif
        if(!((CAction *)param)->m_bIsStop)
        {
            (CAction::m_ZtimeOutGlueSet) ? MO_GummingSet() : MO_FinishGumming();
            CAction::m_ZtimeOutGlueSet = FALSE;
        }
    }
    if(RR1U & 0x0200 && ((CAction *)param)->m_getHeightFlag)  //U�p�ɾ����_
    {
#ifdef LA
        LONG lCalcData1;
        if(LAS_GetLaserData(lCalcData1))
        {
            if(lCalcData1 == LAS_LONGMIN)
            {
                g_LaserErrCnt++;
            }
            else
            {
                ((CAction *)param)->DATA_3Do[g_LaserNuCnt].EndPX = MO_ReadLogicPosition(0) + ((CAction *)param)->m_OffSetLaserX - ((CAction *)param)->m_OffSetScan;
                ((CAction *)param)->DATA_3Do[g_LaserNuCnt].EndPY = MO_ReadLogicPosition(1) + ((CAction *)param)->m_OffSetLaserY - ((CAction *)param)->m_OffSetScan;
                ((CAction *)param)->DATA_3Do[g_LaserNuCnt].EndPZ = MO_ReadLogicPosition(2) - lCalcData1 + ((CAction *)param)->m_OffSetLaserZ;//30000���P���d��
                if(((CAction *)param)->m_LaserAverage == TRUE)
                {
                    ((CAction *)param)->m_LaserAveBuffZ += ((CAction *)param)->DATA_3Do[g_LaserNuCnt].EndPZ;
                }
                else
                {
                    ((CAction *)param)->LA_m_ptVec.push_back(((CAction *)param)->DATA_3Do[g_LaserNuCnt]);
#ifdef PRINTF
     //               _cwprintf(L"%s\n", L"Get Point");
#endif
                }
                g_LaserNuCnt++;
            }
        }

#endif
    }
#endif
    return 0;
}
//�ʧ@�����
UINT CAction::MoMoveThread(LPVOID param)
{
#ifdef MOVE
    //_cwprintf(L"%s\n", L"JogMove : W_Rotation");
    //m_ThreadFlag = 1 ------ W_Rotation
    //WangBuff         ------ dW
    //cpCirMidBuff[0].x------ lWorkVelociy;
    //cpCirMidBuff[0].y------ lAcceleration;
    //cpCirMidBuff[1].x------ lInitVelociy;
    if(((CAction *)param)->m_ThreadFlag == 1) //w�b����
    {
    //_cwprintf(L"%s\n", L"W_Rotation");
        ((CAction *)param)->W_Rotation(((CAction *)param)->WangBuff, ((CAction *)param)->cpCirMidBuff[0].x, ((CAction *)param)->cpCirMidBuff[0].y, ((CAction *)param)->cpCirMidBuff[1].x);
    }
    else if(((CAction *)param)->m_ThreadFlag == 2)
    {
    //_cwprintf(L"%s\n", L"GoHoming");
        //m_ThreadFlag = 2;------W_NeedleGoHoming
        //cpCirMidBuff[0].x------ Speed1;
        //cpCirMidBuff[0].y------ Speed2;
        //cpCirMidBuff[1].x------ bStep;
		((CAction *)param)->m_IsHomingOK = 1;//���I�_�k���椤
        if(!((CAction *)param)->cpCirMidBuff[1].x)
        {
            ((CAction *)param)->DecideInitializationMachine(((CAction *)param)->cpCirMidBuff[0].x, ((CAction *)param)->cpCirMidBuff[0].x, 15,0, 0, 0, 0);
			if (!((CAction *)param)->m_bIsStop)
			{
				MO_Do3DLineMove(((CAction *)param)->m_HomingOffset_INIT.x, ((CAction *)param)->m_HomingOffset_INIT.y, ((CAction *)param)->m_HomingOffset_INIT.z,
					((CAction *)param)->cpCirMidBuff[0].x, (((CAction *)param)->cpCirMidBuff[0].x) * 3, ((CAction *)param)->cpCirMidBuff[0].y);
				((CAction *)param)->PreventMoveError();
			}
        }
        else
        {
            //�P�_W�b�O�_�ե�
            if(((CAction *)param)->m_MachineOffSet.x == -99999 && ((CAction *)param)->m_MachineOffSet.y == -99999)
            {
                //((CAction *)param)->DecideInitializationMachine(((CAction *)param)->cpCirMidBuff[0].x, ((CAction *)param)->cpCirMidBuff[0].x, 15,
                //  ((CAction *)param)->m_HomingOffset_INIT.x, ((CAction *)param)->m_HomingOffset_INIT.y, ((CAction *)param)->m_HomingOffset_INIT.z, ((CAction *)param)->m_HomingOffset_INIT.w);
                return 0;
            }
            ((CAction *)param)->DecideInitializationMachine(((CAction *)param)->cpCirMidBuff[0].x, ((CAction *)param)->cpCirMidBuff[0].y, 15,
                    (((CAction *)param)->m_HomingPoint.x ), (((CAction *)param)->m_HomingPoint.y), ((CAction *)param)->m_HomingPoint.z, ((CAction *)param)->m_HomingPoint.w);

			if(!((CAction *)param)->m_bIsStop)
            {
               MO_Do3DLineMove(-(((CAction *)param)->m_MachineOffSet.x), -(((CAction *)param)->m_MachineOffSet.y), 0,
                                ((CAction *)param)->cpCirMidBuff[0].x, (((CAction *)param)->cpCirMidBuff[0].x) * 3, ((CAction *)param)->cpCirMidBuff[0].y);
                ((CAction *)param)->PreventMoveError();
            }
        }
    }
    else
    {
        Sleep(1);
    }
    //�M��buff���A
	((CAction *)param)->m_IsHomingOK = 0;//���I�_�k��l��
    ((CAction *)param)->m_ThreadFlag = 0;//�_�k�X�Ъ��A
    ((CAction *)param)->cpCirMidBuff[0].x = 0;
    ((CAction *)param)->cpCirMidBuff[0].y = 0;
    ((CAction *)param)->cpCirMidBuff[1].x = 0;
    ((CAction *)param)->cpCirMidBuff[1].y = 0;
    ((CAction *)param)->WangBuff = 0;
#endif
    return 0;
}

/***********************************************************
**                                                        **
**          �B�ʼҲ�-�ۦ�B�Ψ��                           **
**                                                        **
************************************************************/

/*
*�X��(�t�X������ϥ�)
*��J(�I�����}��or�I��������)��Ķ�᪺LONG��
*/
void CAction::DoGlue(LONG lTime, LONG lDelayTime, LPTHREAD_START_ROUTINE MoInterrupt)
{
#ifdef MOVE
    if(!m_bIsStop)
    {
        if(!m_bIsStop && m_bIsDispend == 1)
        {
            MO_GummingSet(lTime * 1000);
        }
        PreventGlueError();//����X���X��
    }
    if(!m_bIsStop)
    {
        // lTime / 1000 =(us��ms)
        MO_Timer(0, 0, lDelayTime * 1000);
        MO_Timer(1, 0, 0);//(ms)
        Sleep(1);//����X���A�קK�p�ɾ��쪽��0
        while(MO_Timer(3, 0, 0))
        {
            if(m_bIsStop == 1)
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
*����b�d�X��
*Ū���U�b�X�ʪ��A�A��ʧ@�ɰ���C
*/
void CAction::PreventMoveError()
{
#ifdef MOVE
    while(MO_ReadIsDriving(15) && !m_bIsStop)
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
    while(MO_ReadGumming() && !m_bIsStop)
    {
        Sleep(1);
    }
    MO_StopGumming();//����X��
#endif
}
/*
*�Ȱ��^�_���~��
*Ū���Ȱ��ѼơA��ѼƬ�0�ɥX���A�B�I�����n���}�C
*/
void CAction::PauseDoGlue()
{
#ifdef MOVE
    if(!m_bIsPause)   //�Ȱ���_���~��X��(m_bIsPause=0)
    {
        if((m_bIsPause == 0) && (m_bIsDispend == 1))
        {
            MO_GummingSet();//�(���d)
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
    if((m_bIsPause == 1) || (m_bIsDispend == 0))  //�Ȱ��ɰ����(m_bIsPause=1)
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
    /*�u�q�����I(x�y�СAy�y�СAz�y�СA)
    LONG lXarEnd, LONG lYarEnd, LONG lZarEnd
    */
    /*�u�q�}�l(x�y�СAy�y�СA)
    LONG lXarUp, LONG lYarUp
    /*
    /*��^�]�w(��^���סAz��^���סA��^�t�סA�����A���t��)
    LONG lLineStop ,LONG lStopZar ,LONG lLowSpeed,int iType,lHighSpeed
    */
    /*Z�b�u�@���׳]�w-Z�b�^�ɰ���(�۹�)�̰��I
    LONG lBackZar (�ϥά۹� �ҥH���A����,1)
    */
    /*�t�ΰѼ�(�[�t�סA��t��)
    LONG lAcceleration, LONG lInitSpeed
    */
#ifdef MOVE
    LONG lBackXar, lBackYar, lR; //lBackXar x�b���ʮy��(����), lBackYar Y�b���ʮy��(����)
    LONG lNowX = 0, lNowY = 0, lNowZ = 0;
    lNowX = MO_ReadLogicPosition(0);
    lNowY = MO_ReadLogicPosition(1);
    lR = LONG(sqrt(pow((DOUBLE)(lXarEnd - lXarUp), 2) + pow((DOUBLE)(lYarEnd - lYarUp), 2)));
    if(lLineStop>lR)
    {
        lBackXar = lXarUp;
        lBackYar = lYarUp;
    }
    else if(lXarEnd == 0 && lYarEnd == 0 && lXarUp == 0 && lYarUp == 0)
    {
        lBackXar = lNowX;
        lBackYar = lNowY;
    }
    else
    {
        LONG lXClose, lYClose, lLineClose;
        lLineClose = lLineStop;
        LineGetToPoint(lXClose, lYClose, lXarUp, lYarUp, lXarEnd, lYarEnd, lLineClose);
        lBackXar = lXClose;
        lBackYar = lYClose;//��^���׮y��
    }
    if(lStopZar>lZarEnd)
    {
        lStopZar = lZarEnd;
    }
    if(lStopZar>lBackZar)
    {
        lStopZar = lBackZar;
    }
    if(lBackZar>lZarEnd)
    {
        lBackZar = lZarEnd;
    }
    switch(iType)  //0~5
    {
        case 0://�L�ʧ@
        {
            break;
        }
        case 1://z�b�W�ɨ�q�t
        {
            if(lStopZar == 0)
            {
                if(!m_bIsStop)
                {
                    MO_Do3DLineMove(0, 0, (lZarEnd - lBackZar) - lZarEnd, lHighSpeed, lAcceleration, lInitSpeed);//�S����^���ץH���t��^�̰��I
                    PreventMoveError();//����b�d�X��
                }
            }
            else
            {
                if(!m_bIsStop)
                {
                    MO_Do3DLineMove(0, 0, (lZarEnd - lStopZar) - lZarEnd, lLowSpeed, lAcceleration, lInitSpeed);//�C�t
                    PreventMoveError();//����b�d�X��
                }
                if(!m_bIsStop)
                {
                    MO_Do3DLineMove(0, 0, (lZarEnd - lBackZar) - (lZarEnd - lStopZar), lHighSpeed, lAcceleration, lInitSpeed);//���t
                    PreventMoveError();//����b�d�X��
                }
            }
            break;
        }
        case 2: //�V��T�b����(�C�t)�A�Az�b�W��(���t)
        {
            if(lStopZar == 0)
            {
                if(!m_bIsStop)
                {
                    MO_Do3DLineMove(lBackXar - lNowX, lBackYar - lNowY, 0, lLowSpeed, lAcceleration, lInitSpeed);//�C�t
                    PreventMoveError();//����b�d�X��

                }
                if(!m_bIsStop)
                {
                    MO_Do3DLineMove(0, 0, (lZarEnd - lBackZar) - lZarEnd, lHighSpeed, lAcceleration, lInitSpeed);//���t
                    PreventMoveError();//����b�d�X��
                }
            }
            else
            {
                if(!m_bIsStop)
                {
                    MO_Do3DLineMove(lBackXar - lNowX, lBackYar - lNowY, (lZarEnd - lStopZar) - lZarEnd, lLowSpeed, lAcceleration, lInitSpeed);//�C�t
                    PreventMoveError();//����b�d�X��
                }
                if(!m_bIsStop)
                {
                    MO_Do3DLineMove(0, 0, (lZarEnd - lBackZar) - (lZarEnd - lStopZar), lHighSpeed, lAcceleration, lInitSpeed);//���t
                    PreventMoveError();//����b�d�X��
                }
            }

            break;
        }
        case 3://�����V��Az�b���W��(�C�t)�Axy��b���ɦV�Ჾ��(���t)�A�Az�b�W��(���t)
        {
            if(lStopZar != 0)
            {
                if(!m_bIsStop)
                {
                    MO_Do3DLineMove(0, 0, (lZarEnd - lStopZar) - lZarEnd, lLowSpeed, lAcceleration, lInitSpeed);//�C�tz�b�W��
                    PreventMoveError();//����b�d�X��
                }
            }
            if(!m_bIsStop)
            {
                MO_Do3DLineMove(lBackXar - lNowX, lBackYar - lNowY, 0, lHighSpeed, lAcceleration, lInitSpeed);//�V�Ჾ�ʰ��t
                PreventMoveError();//����b�d�X��
            }
            if(!m_bIsStop)
            {
                MO_Do3DLineMove(0, 0, (lZarEnd - lBackZar) - (lZarEnd - lStopZar), lHighSpeed, lAcceleration, lInitSpeed);//�V�Ჾ�ʰ��t
                PreventMoveError();//����b�d�X��
            }
            break;
        }
        case 4://�V�e�T�b����(�C�t)�A�Az�b�W��(���t)
        {
            lBackXar = (lXarEnd - lBackXar) + lXarEnd;
            lBackYar = (lYarEnd - lBackYar) + lYarEnd;
            if(lStopZar == 0)
            {
                if(!m_bIsStop)
                {
                    MO_Do3DLineMove(lBackXar - lNowX, lBackYar - lNowY, 0, lLowSpeed, lAcceleration, lInitSpeed);//�C�t
                    PreventMoveError();//����b�d�X��
                }
                if(!m_bIsStop)
                {
                    MO_Do3DLineMove(0, 0, (lZarEnd - lBackZar) - lZarEnd, lLowSpeed, lAcceleration, lInitSpeed);//���t
                    PreventMoveError();//����b�d�X��
                }
            }
            else
            {
                if(!m_bIsStop)
                {
                    MO_Do3DLineMove(lBackXar - lNowX, lBackYar - lNowY, (lZarEnd - lStopZar) - lZarEnd, lLowSpeed, lAcceleration, lInitSpeed);//�C�t
                    PreventMoveError();//����b�d�X��
                }
                if(!m_bIsStop)
                {
                    MO_Do3DLineMove(0, 0, (lZarEnd - lBackZar) - (lZarEnd - lStopZar), lHighSpeed, lAcceleration, lInitSpeed);//���t
                    PreventMoveError();//����b�d�X��
                }
            }
            break;
        }
        case 5: //�����V�e�Az�b���W��(�C�t)�Axy��b���ɦV�e����(���t)�A�Az�b�W��(���t)
        {
            lBackXar = (lXarEnd - lBackXar) + lXarEnd;
            lBackYar = (lYarEnd - lBackYar) + lYarEnd;

            if(lStopZar != 0)
            {
                if(!m_bIsStop)
                {
                    MO_Do3DLineMove(0, 0, (lZarEnd - lStopZar) - lZarEnd, lLowSpeed, lAcceleration, lInitSpeed);//z�b�C�t�W��
                    PreventMoveError();//����b�d�X��
                }
            }
            if(!m_bIsStop)
            {
                MO_Do3DLineMove(lBackXar - lNowX, lBackYar - lNowY, 0, lHighSpeed, lAcceleration, lInitSpeed);//��b�V�e����(���t)
                PreventMoveError();//����b�d�X��
            }
            if(!m_bIsStop)
            {
                MO_Do3DLineMove(0, 0, (lZarEnd - lBackZar) - (lZarEnd - lStopZar), lHighSpeed, lAcceleration, lInitSpeed);//���tz�b�W��
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
    DOUBLE dX = lX0 - lX1;
    DOUBLE dY = lY0 - lY1;
    DOUBLE D = sqrt(pow(dX, 2) + pow(dY, 2));
    DOUBLE dXc = dX*(DOUBLE)lLineClose / D;
    DOUBLE dYc = dY*(DOUBLE)lLineClose / D;
    lXClose = lX1 + (LONG)round(dXc);
    lYClose = lY1 + (LONG)round(dYc);
}
/*
*���u�Z���ഫ���y���I--�h��
*/
void CAction::LineGetToPoint(LONG &lXClose, LONG &lYClose, LONG &lZClose, LONG lX0, LONG lY0, LONG lX1, LONG lY1, LONG lZ0, LONG lZ1, LONG &lLineClose)
{
    LONG lLength = 0;
    lLength = LONG(sqrt(pow(lX0 - lX1, 2) + pow(lY0 - lY1, 2) + pow(lZ0 - lZ1, 2)));
    if(lZ0 == lZ1)
    {
        lZClose = lZ0;
    }
    else
    {
        lZClose = (lLineClose * (lZ1 - lZ0) / lLength) + lZ0;
    }
    LineGetToPoint(lXClose, lYClose, lX0, lY0, lX1, lY1, lLineClose);
}

/*���u�q�ϥΡA�]�m�Z���b�_�l�I�X��*/
LONG CAction::CalPreglue(LONG lStartDistance, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
    DOUBLE dStartTime = 0;
    LONG lDistance = 0;
    DOUBLE S = 0, t = 0;
    CString csS;
    if(lAcceleration == 0)
    {
        t = (DOUBLE)lStartDistance / (DOUBLE)lWorkVelociy;
        return (LONG)round(t * 1000000);
    }
    t = (DOUBLE)(lWorkVelociy - lInitVelociy) / (DOUBLE)lAcceleration;
    S = (DOUBLE)lInitVelociy*t + (DOUBLE)lAcceleration*pow(t, 2) / 2.0;
    if(lStartDistance >= (LONG)round(S))
    {
        lDistance = lStartDistance - (LONG)round(S);
        dStartTime = (DOUBLE)lDistance / (DOUBLE)lWorkVelociy;
        dStartTime += t;
        return (LONG)round(dStartTime * 1000000);
    }
    else
    {
        csS.Format(L"�Z�����A��,�̤j�Z��%.3f um\n",S);
        AfxMessageBox(csS);
        return (LONG)round(t * 1000000);
    }
}

/*==���u�q�ϥΡA�]�m�Z���b�_�l�I�X��*/
//�p��쵥�t���,�һݭn��"�Z��"
LONG CAction::CalPreglue(LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
    DOUBLE S = 0, t = 0;
    if(lAcceleration == 0)
    {
        return 0;
    }
    t = (DOUBLE)(lWorkVelociy - lInitVelociy) / (DOUBLE)lAcceleration;
    S = (DOUBLE)lInitVelociy*t + (DOUBLE)lAcceleration*pow(t, 2) / 2.0;
    return (LONG)round(S);
}

/*==���u�q�ϥΡA�]�m�Z���b�_�l�I�X��*/
//�p��쵥�t���,�һݭn��"�ɶ�"
LONG CAction::CalPreglueTime(LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
    DOUBLE S = 0, t = 0;
    if(lAcceleration == 0)
    {
        return 0;
    }
    t = (DOUBLE)(lWorkVelociy - lInitVelociy) / (DOUBLE)lAcceleration;
    S = (DOUBLE)lInitVelociy*t + (DOUBLE)lAcceleration*pow(t, 2) / 2.0;
    return (LONG)round(t * 1000000);
}
/*
*�T�I�p����
*return CString(x�b��ߡAy�b��ߡA0�f��/1����A)
*/
CString CAction::TRoundCCalculation(CString Origin, CString End, CString Between)
{
    if(Origin == End || End == Between || Origin == Between)
    {
        AfxMessageBox(_T("What the fucking you type in my system."));
        return _T("F");
    }
    CString Ans;
    DOUBLE x1 = DOUBLE(CStringToLong(Origin, 0));
    DOUBLE y1 = DOUBLE(CStringToLong(Origin, 1));
    DOUBLE x2 = DOUBLE(CStringToLong(End, 0));
    DOUBLE y2 = DOUBLE(CStringToLong(End, 1));
    DOUBLE x3 = DOUBLE(CStringToLong(Between, 0));
    DOUBLE y3 = DOUBLE(CStringToLong(Between, 1));

    DOUBLE tan13 = (y3 - y1) / (x3 - x1);
    if((tan13) >= 200)
    {
        tan13 = 200;
    }
    else if((tan13) <= -200)
    {
        tan13 = -200;
    }
    DOUBLE tan12 = (y2 - y1) / (x2 - x1);
    if((tan12) >= 200)
    {
        tan12 = 200;
    }
    else if((tan12) <= -200)
    {
        tan13 = -200;
    }

    DOUBLE a = 2 * (x1 - x2);
    DOUBLE b = 2 * (y1 - y2);
    DOUBLE c = y1*y1 + x1*x1 - x2*x2 - y2*y2;
    DOUBLE d = 2 * (x1 - x3);
    DOUBLE e = 2 * (y1 - y3);
    DOUBLE f = y1*y1 + x1*x1 - x3*x3 - y3*y3;
    int h = int(round((e*c - b*f) / (a*e - b*d)));
    int k = int(round((a*f - d*c) / (a*e - b*d)));
    Ans.Format(_T("%d�A%d�A"), h, k);
    if((x2 - x1) > 0)
    {
        if((y3 - tan12*x3) > (y1 - tan12*x1))
        {
            Ans = Ans + _T("0") + _T("�A");
        }
        else if((y3 - tan12*x3) < (y1 - tan12*x1))
        {
            Ans = Ans + _T("1") + _T("�A");
        }
        else
        {
            Ans = _T("F");
            AfxMessageBox(_T("�T�I�@�u"));
        }
    }
    else if((x2 - x1) < 0)
    {
        if((y3 - tan12*x3) > (y1 - tan12*x1))
        {
            Ans = Ans + _T("1") + _T("�A");
        }
        else if((y3 - tan12*x3) < (y1 - tan12*x1))
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
        if((x3 > x1))
        {
            Ans = Ans + _T("1") + _T("�A");
        }
        else if((x3 < x1))
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
    for(int i = 0; i<csData.GetLength(); i++)
    {
        ione[i + 1] = csData.Find(L"�A", ione[i] + 1);
    }
    lOne[0] = _ttol(csData.Left(ione[0]));
    for(int j = 0; j<iChoose; j++)
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

    if(offsetx<0 && offsety >= 0)
    {
        theta_a = 180 - theta_a;
    }
    else if(offsetx<0 && offsety<0)
    {
        theta_a = 180 + theta_a;
    }
    else if(offsetx >= 0 && offsety<0)
    {
        theta_a = 360 - theta_a;
    }

    offset_x = LocatX2 - LocatX;
    offset_y = LocatY2 - LocatY;

    X_a = offset_x / pow(((pow(offset_x, 2)) + (pow(offset_y, 2))), 0.5);
    Y_a = offset_y / pow(((pow(offset_x, 2)) + (pow(offset_y, 2))), 0.5);

    theta_b = (atan(fabs((Y_a / X_a)))) * 180 / M_PI;

    if(offset_x<0 && offset_y >= 0)
    {
        theta_b = 180 - theta_b;
    }
    else if(offset_x<0 && offset_y<0)
    {
        theta_b = 180 + theta_b;
    }
    else if(offset_x >= 0 && offset_y<0)
    {
        theta_b = 360 - theta_b;
    }

    if(theta_a<theta_b)
    {
        theta_a = theta_b - theta_a;
    }
    else
    {
        theta_a = 360 - (theta_a - theta_b);
    }

    if(bRev == 1)
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
    if(bDir == 0)
    {
        lArcX = LONG((((DOUBLE)lX - (DOUBLE)lCenX)*cos(dAngl)) - (((DOUBLE)lY - (DOUBLE)lCenY)*sin(dAngl)) + lCenX);
        lArcY = LONG((((DOUBLE)lX - (DOUBLE)lCenX)*sin(dAngl)) + (((DOUBLE)lY - (DOUBLE)lCenY)*cos(dAngl)) + lCenY);
    }
    else
    {
        lArcX = LONG((((DOUBLE)lX - (DOUBLE)lCenX)*cos(-dAngl)) - (((DOUBLE)lY - (DOUBLE)lCenY)*sin(-dAngl)) + lCenX);
        lArcY = LONG((((DOUBLE)lX - (DOUBLE)lCenX)*sin(-dAngl)) + (((DOUBLE)lY - (DOUBLE)lCenY)*cos(-dAngl)) + lCenY);
    }
}
/*����---��R�κA(���A1�x��s���|��R)
*��J(�_�l�Ix1,y1,�����Ix2,y2,�e��,�X�ʳt��,�[�t��,��t��)
*/
void CAction::AttachFillType1(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lZ, LONG lZBackDistance, LONG lWidth,
                              LONG lStartDelayTime, LONG lStartDistance, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime,
                              LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
#ifdef MOVE
    /*�_�l/�����]�w
    *1.lStartDelayTime       ���ʫe����
    *2.lStartDistance        �}���e�q�_�I���ʶZ��(�]�m�Z��)
    *3.lCloseOffDelayTime    ������b�����I���d�ɶ�(���d�ɶ�)
    *5.lCloseDistance        �Z�������I�h���Z������(�����Z��)
    *6.lCloseONDelayTime     �����᪺����ɶ�(��������)
    */
    CPoint pt1(lX1, lY1);
    CPoint pt2(lX2, lY2);
    CPoint ptNow = pt1;
    if(pt1 == pt2 || lWidth == 0)
    {
        return;
    }

    std::vector<DATA_2MOVE> vecData2do;
    vecData2do.clear();
    LONG width = lWidth * 1000;

    //�ϥ�(1)���ʫe����(lStartDelayTime)
    if(lStartDelayTime>0)
    {
        if(!m_bIsStop)
        {
            MO_Do3DLineMove(lX1 - MO_ReadLogicPosition(0), lY1 - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration,
                            lInitVelociy);//�^��_�l�I!
            PreventMoveError();
        }
        if(!m_bIsStop)
        {
            MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//�^��_�l�I!
            PreventMoveError();
        }
        PauseDoGlue();//�Ȱ��^�_�᭫�s�(Ū���Ȱ��ѼơA��ѼƬ�0�ɥX���A�B�I�����n���}�C)
        if(!m_bIsStop && m_bIsDispend == 1)
        {
            MO_GummingSet();//�(���d)
        }
        MO_Timer(0, 0, lStartDelayTime * 1000);
        MO_Timer(1, 0, lStartDelayTime * 1000);//�u�q�I���]�w---(1)���ʫe����(�b�u�q�}�l�I�W)

        Sleep(1);//����X���A�קK�p�ɾ��쪽��0
        while(MO_Timer(3, 0, 0))
        {
            if(m_bIsStop)
            {
                break;
            }
            Sleep(1);
        }
        /*���J�Ĥ@�I*/
        MCO_Do2dDataLine(pt1.x, pt1.y, vecData2do);
    }
    //�ϥ�(2)�p��X���Z���A�]�w�X���I
    else if(lStartDistance>0)
    {
        LONG glueDist = lStartDistance;

        if(pt1.x == pt2.x) //���I����
        {
            /*�Z����xy���q*/
            glueDist = LONG(glueDist*M_SQRT1_2);
            glueDist = LONG((pt1.y<pt2.y) ? -glueDist : glueDist);
            MCO_Do2dDataLine(pt1.x + glueDist, pt1.y + glueDist, vecData2do);
        }
        else if(pt1.y == pt2.y) //���I����
        {
            /*�Z����xy���q*/
            glueDist = LONG(glueDist*M_SQRT1_2);
            glueDist = (pt1.x<pt2.x) ? glueDist : -glueDist;
            MCO_Do2dDataLine(pt1.x + glueDist, pt1.y - glueDist, vecData2do);
        }
        else
        {
            glueDist = (pt1.x<pt2.x) ? -glueDist : glueDist;
            MCO_Do2dDataLine(pt1.x + glueDist, pt1.y, vecData2do);
        }
    }
    //(1)(2)�Ҥ��ϥ�
    else
    {
        /*���J�Ĥ@�I*/
        MCO_Do2dDataLine(pt1.x, pt1.y, vecData2do);
    }

    //���I����
    if(pt1.x == pt2.x)
    {
        /*��R�ϰ�(�x��)���*/
        LONG lHeightOfRect = (LONG)abs(round(DOUBLE(pt2.y - pt1.y)*M_SQRT1_2));
        /*���j�e��xy�첾�q�A��Rxy�첾�q�p��AoddEven���f�V����*/
        LONG shiftWidth = LONG(width*M_SQRT1_2);
        LONG shift = (LONG)abs(round((pt1.y - pt2.y) / 2.0));
        BOOL oddEven = TRUE;
        if(pt2.y<pt1.y)
        {
            oddEven = FALSE;
        }
        while(lHeightOfRect >= 0)
        {
            ptNow.x = (oddEven) ? (ptNow.x + shift) : (ptNow.x - shift);
            ptNow.y = (oddEven) ? (ptNow.y + shift) : (ptNow.y - shift);
            MCO_Do2dDataLine(ptNow.x, ptNow.y, vecData2do);

            lHeightOfRect -= width;
            if(lHeightOfRect >= 0)
            {
                ptNow.x = (pt2.y>pt1.y) ? (ptNow.x - shiftWidth) : (ptNow.x + shiftWidth);
                ptNow.y = (pt2.y>pt1.y) ? (ptNow.y + shiftWidth) : (ptNow.y - shiftWidth);
                MCO_Do2dDataLine(ptNow.x, ptNow.y, vecData2do);
            }
            else
            {
                if(oddEven)
                {
                    ptNow = pt2;
                    MCO_Do2dDataLine(ptNow.x, ptNow.y, vecData2do);
                }
                else
                {
                    ptNow.x = (pt2.y>pt1.y) ? pt1.x - shift : pt1.x + shift;
                    ptNow.y = LONG((pt1.y + pt2.y) / 2.0);
                    MCO_Do2dDataLine(ptNow.x, ptNow.y, vecData2do);
                    ptNow = pt2;
                    MCO_Do2dDataLine(ptNow.x, ptNow.y, vecData2do);
                }
            }
            oddEven = !oddEven;
        }
    }
    //���I����
    else if(pt1.y == pt2.y)
    {
        LONG lHeightOfRect = (LONG)abs(round(DOUBLE(pt2.x - pt1.x)*M_SQRT1_2));
        LONG shiftWidth = LONG(width*M_SQRT1_2);
        LONG shift = (LONG)abs(round((pt1.x - pt2.x) / 2.0));
        BOOL oddEven = TRUE;
        if(pt2.x<pt1.x)
        {
            oddEven = FALSE;
        }
        while(lHeightOfRect >= 0)
        {
            ptNow.x = (oddEven) ? (ptNow.x + shift) : (ptNow.x - shift);
            ptNow.y = (oddEven) ? (ptNow.y - shift) : (ptNow.y + shift);
            MCO_Do2dDataLine(ptNow.x, ptNow.y, vecData2do);

            lHeightOfRect -= width;
            if(lHeightOfRect >= 0)
            {
                ptNow.x = (pt2.x>pt1.x) ? (ptNow.x + shiftWidth) : (ptNow.x - shiftWidth);
                ptNow.y = (pt2.x>pt1.x) ? (ptNow.y + shiftWidth) : (ptNow.y - shiftWidth);
                MCO_Do2dDataLine(ptNow.x, ptNow.y, vecData2do);
            }
            else
            {
                if(oddEven)
                {
                    ptNow.x = LONG((pt1.x + pt2.x) / 2.0);
                    ptNow.y = (pt2.x>pt1.x) ? pt1.x + shift : pt1.x - shift;
                    MCO_Do2dDataLine(ptNow.x, ptNow.y, vecData2do);
                    ptNow = pt2;
                    MCO_Do2dDataLine(ptNow.x, ptNow.y, vecData2do);
                }
                else
                {
                    ptNow = pt2;
                    MCO_Do2dDataLine(ptNow.x, ptNow.y, vecData2do);
                }
            }
            oddEven = !oddEven;
        }
    }
    else
    {
        LONG lHeightOfRect = abs(pt2.y - pt1.y);
        LONG shiftX = pt2.x - pt1.x;
        CPoint ptNow = pt1;
        BOOL oddEven = TRUE;
        while(lHeightOfRect >= 0)
        {
            /*x�����U�@�I*/
            ptNow.x = (oddEven) ? (ptNow.x + shiftX) : (ptNow.x - shiftX);
            MCO_Do2dDataLine(ptNow.x, ptNow.y, vecData2do);
            if(ptNow == pt2)
            {
                break;
            }

            /*�P�_���j�e�׬O�_�٦��Ѿl�A��s���j�e�ת��y��*/
            lHeightOfRect -= width;
            if(lHeightOfRect >= 0)
            {
                /*y�e�ץ����U�@�I*/
                ptNow.y = (pt2.y>pt1.y) ? (ptNow.y + width) : (ptNow.y - width);
                MCO_Do2dDataLine(ptNow.x, ptNow.y, vecData2do);
            }
            else
            {
                /*���j�e�פ������B�z*/
                if(oddEven)
                {
                    /*�����쵲���I*/
                    ptNow = pt2;
                    MCO_Do2dDataLine(ptNow.x, ptNow.y, vecData2do);
                }
                else
                {
                    /*�ɨ��e�צܵ����I*/
                    ptNow.y = pt2.y;
                    MCO_Do2dDataLine(ptNow.x, ptNow.y, vecData2do);
                    ptNow = pt2;
                    MCO_Do2dDataLine(ptNow.x, ptNow.y, vecData2do);
                }
            }
            oddEven = !oddEven;
        }
    }

    //�I�}�C�ഫ
    LA_AbsToOppo2Move(vecData2do);
    for(UINT i = 1; i < vecData2do.size(); i++)
    {
        DATA_2Do[i - 1] = vecData2do.at(i);
    }
#ifdef MOVE
    if(!m_bIsStop)
    {
        MO_Do3DLineMove(vecData2do.at(0).EndP.x, vecData2do.at(0).EndP.y, 0, lWorkVelociy, lAcceleration,
                        lInitVelociy);//�^��_�l�I!
        PreventMoveError();
    }
    if(!m_bIsStop)
    {
        MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//�^��_�l�I!
        PreventMoveError();
    }
#endif
    //�ϥ�(2)�]�m�Z��
    if(!(lStartDelayTime>0) && lStartDistance>0)
    {
        //timeUpGlue ���us
        LONG timeUpGlue = CalPreglue(lStartDistance, lWorkVelociy, lAcceleration, lInitVelociy);
        if(!m_bIsStop)
        {
            CAction::m_YtimeOutGlueSet = TRUE;
#ifdef MOVE
            MO_TimerSetIntter(timeUpGlue, 0);//�ϥ�Y timer���_�X��
#endif
        }
    }

    //�ϥ�(5)�����Z��(lCloseDistance)
    if(lCloseDistance>0)
    {
        LONG sumPath = 0;
        LONG finishTime = 0;
        DOUBLE avgTime = 0;
        LONG accLength = CalPreglue(lWorkVelociy, lAcceleration, lInitVelociy);
        for(UINT i = 1; i<vecData2do.size(); i++)
        {
            sumPath += vecData2do.at(i).Distance;
        }
        avgTime = ((DOUBLE)sumPath - (DOUBLE)accLength) / (DOUBLE)lWorkVelociy;
        finishTime = (LONG)round(avgTime * 1000000) + CalPreglueTime(lWorkVelociy, lAcceleration, lInitVelociy);
        LONG closeDistTime = CalPreglue(lCloseDistance, lWorkVelociy, 0, lInitVelociy);
        if(!m_bIsStop)
        {
            CAction::m_ZtimeOutGlueSet = FALSE;
            MO_TimerSetIntter(finishTime - closeDistTime, 1);
        }
    }

    if(lStartDelayTime == 0 && lStartDistance == 0)
    {
        if(!m_bIsStop && m_bIsDispend == 1)
        {
            MO_GummingSet();
        }
    }

    MO_DO2Curve(DATA_2Do, vecData2do.size() - 1, lWorkVelociy);
    PreventMoveError();//�����X�ʿ��~
    Sleep(200);
    vecData2do.clear();



    //�ϥ�(3)���d�ɶ�(lCloseOffDelayTime)
    if(lCloseOffDelayTime>0)
    {
        MO_Timer(0, 0, lCloseOffDelayTime * 1000);
        MO_Timer(1, 0, lCloseOffDelayTime * 1000);//�u�q�I���]�w---(3)���d�ɶ�
        Sleep(1);//����X���A�קK�p�ɾ��쪽��0
        while(MO_Timer(3, 0, 0))
        {
            if(m_bIsStop == 1)
            {
                break;
            }
            Sleep(1);
        }
    }
    PauseStopGlue();//�Ȱ��ɰ����(m_bIsPause=1)
    MO_StopGumming();//����X��

    //�ϥ�(6)��������(lCloseONDelayTime)
    if(!(lCloseDistance>0) && lCloseONDelayTime>0)
    {
        MO_Timer(0, 0, lCloseONDelayTime * 1000);
        MO_Timer(1, 0, lCloseONDelayTime * 1000);//�u�q�I���]�w---(6)��������
        Sleep(1);//����X���A�קK�p�ɾ��쪽��0
        while(MO_Timer(3, 0, 0))
        {
            if(m_bIsStop == 1)
            {
                break;
            }
            Sleep(1);
        }
    }
    if(!m_bIsStop)
    {
        //*************************��R�^��z�b�t�ץثe���X�ʳt�ת��⭿******************
        MO_Do3DLineMove(0, 0, (lZ - lZBackDistance) - lZ, lWorkVelociy * 2, lAcceleration, lInitVelociy);//Z�b��^
        PreventMoveError();//����b�d�X��
    }
#endif
}
/*����---��R�κA(���A2������۶�R.�ѥ~�Ӥ�)
*��J(�_�l�Ix1,y1,���x2,y2,�e��,�X�ʳt��,�[�t��,��t��)
*/
/*����---��R�κA(���A2������۶�R.�ѥ~�Ӥ�)
*��J(�_�l�Ix1,y1,���x2,y2,�e��,�X�ʳt��,�[�t��,��t��)
*/
void CAction::AttachFillType2(LONG lX1, LONG lY1, LONG lCenX, LONG lCenY,
                              LONG lZ, LONG lZBackDistance, LONG lWidth,
                              LONG lStartDelayTime, LONG lStartDistance, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime,
                              LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
    /*�u�q�I���]�w(1.���ʫe����A2.�]�m�Z���A3���d�ɶ��A5�����Z���A6��������)
    LONG lStartDelayTime ,LONG lStartDistance ,LONG lCloseOffDelayTime ,LONG lCloseDistance ,LONG lCloseONDelayTime
    //1.���ʫe�I�����b�@���u�q�_�l�I�B�O�����}���ɪ��C �����ɥi����w�Y�b�y��y�ʤ��e�u�u�q�o�Ͳ��ʡC
    //2. �I�����}�ҫe�A ���F���}���u�u�q�_�l�I�����ʶZ���C �ӶZ�������F���ѤF�������_�t�ɶ��A�D�n�ΨӮ����L�q�y��b�u�q�_�l�B���n�E�C
    //3.�I����������A���F�����O�b�w�Y���ܤU�@�I�e�ܱo���ŦӦb�u�q�I�������I�B���ͪ����ɡC
    //5.������L�q�y��b�u�q�����I�B�o�Ͱ�n�A�I�����b�Z���u�q�����I�e�h���B�����C
    //6.�I�����b�u�q�����I�B�����O���}�Ҫ��ɪ��C
    */
    #pragma region ****�ꫬ���ۥ\��****
#ifdef MOVE
    DOUBLE dRadius = 0, dWidth = 0, dAng0 = 0, dAng1 = 0, dAng2 = 0;
    BOOL bRev = 1;//0�f��/1����
    LONG lLineClose = 0, lXClose = 0, lYClose = 0, lDistance = 0;
    LONG lNowX = 0, lNowY = 0, lTime = 0;
    CPoint cPt1 = 0, cPt2 = 0, cPt3 = 0, cPt4 = 0, cPtCen1 = 0, cPtCen2 = 0, cPt0 = 0;
    int iData = 0, iOdd = 0;//�P�_�_��(�_���W�b��/�����U�b��)
    CString csbuff = 0;
    DOUBLE dSumPath = 0, dCloseTime = 0;//�`���|��.�����ɶ�
    int icnt = 0;//�p�ƾ�
    std::vector<CPoint>m_ptVec;
    std::vector<CPoint>::iterator ptIter;//�ϦV���N��
    m_ptVec.clear();
    cPt1.x = lX1;
    cPt1.y = lY1;
    cPtCen1.x = lCenX;
    cPtCen1.y = lCenY;//�W�b����
    dRadius = sqrt(pow(cPtCen1.x - cPt1.x, 2) + pow(cPtCen1.y - cPt1.y, 2));//�b�|
    if(LONG(dRadius) == 0)
    {
        return;
    }
    if(lStartDistance>0) //�ϥ�--(2)�]�m�Z��(lStartDistance)
    {
        LONG lStartX = 0, lStartY = 0;
        ArcGetToPoint(lStartX, lStartY, lStartDistance, lX1, lY1, lCenX, lCenY, LONG(dRadius), bRev);//��X�_�l�I
        cPt0.x = lStartX;
        cPt0.y = lStartY;
        //      m_ptVec.push_back(cPt0);
        if(!m_bIsStop)
        {
            MO_Do3DLineMove(lStartX - MO_ReadLogicPosition(0), lStartY - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration,
                            lInitVelociy);//�^��_�l�I!
            PreventMoveError();
        }
        if(!m_bIsStop)
        {
            MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//�^��_�l�I!
            PreventMoveError();
        }
        lTime = LONG(1000000 * (DOUBLE)lStartDistance / (DOUBLE)lWorkVelociy);
        //_cwprintf(_T("�p�ɾ��]�m�Z�����ɶ�=%lf \n"), DOUBLE(lTime / 1000000.0));
        /*======�p�ɾ���Ĳ�o���_����X���A�ϥ�y���_����================*/
        if(!m_bIsStop)
        {
            CAction::m_YtimeOutGlueSet = TRUE;
            MO_TimerSetIntter(lTime, 0);//�p�ɨ���ܰ����
        }
    }
    else
    {

        if(!m_bIsStop)
        {
            MO_Do3DLineMove(lX1 - MO_ReadLogicPosition(0), lY1 - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration,
                            lInitVelociy);//�^��_�l�I!
            PreventMoveError();
        }
        if(!m_bIsStop)
        {
            MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//�^��_�l�I!
            PreventMoveError();
        }
        PauseDoGlue();//�Ȱ��^�_�᭫�s�(Ū���Ȱ��ѼơA��ѼƬ�0�ɥX���A�B�I�����n���}�C)
        if(!m_bIsStop && m_bIsDispend == 1)
        {
            MO_GummingSet();//�(���d)
        }
        if(lStartDelayTime > 0) //�ϥ�(1)���ʫe����(lStartDelayTime)
        {
            MO_Timer(0, 0, lStartDelayTime * 1000);
            MO_Timer(1, 0, lStartDelayTime * 1000);//�u�q�I���]�w---(1)���ʫe����(�b�u�q�}�l�I�W)
            Sleep(1);//����X���A�קK�p�ɾ��쪽��0
            while(MO_Timer(3, 0, 0))
            {
                if(m_bIsStop)
                {
                    break;
                }
                Sleep(1);
            }
        }
    }
    lDistance = LONG(dRadius);
    dWidth = lWidth * 1000;
    dAng1 = acos((cPt1.x - cPtCen1.x) / dRadius);
    dAng2 = asin((cPt1.y - cPtCen1.y) / dRadius);
    /*cPt2.x = LONG(dRadius*cos(dAng1 + M_PI) + cPtCen1.x);
    cPt2.y = LONG(dRadius*sin(dAng2 + M_PI) + cPtCen1.y);*/
    cPt2.x = (LONG)((cPt1.x - cPtCen1.x)*cos(M_PI) - (cPt1.y - cPtCen1.y)*sin(M_PI) + cPtCen1.x);
    cPt2.y = (LONG)((cPt1.x - cPtCen1.x)*sin(M_PI) + (cPt1.y - cPtCen1.y)*cos(M_PI) + cPtCen1.y);
    dSumPath = M_PI*dRadius;//���ʪ��`����
    m_ptVec.push_back(cPt1);
    m_ptVec.push_back(cPt2);
    lDistance = LONG(lDistance - dWidth);
    LineGetToPoint(lXClose, lYClose, cPt1.x, cPt1.y, cPtCen1.x, cPtCen1.y,
                   lDistance);
    cPt3.x = lXClose;
    cPt3.y = lYClose;
    cPtCen2.x = LONG((cPt3.x + cPt2.x) / 2.0);
    cPtCen2.y = LONG((cPt3.y + cPt2.y) / 2.0);//�U�b����
    iData = (int)dRadius % (int)dWidth;
    lDistance = LONG(lDistance + dWidth);
    while(1)
    {
        lDistance = LONG(lDistance - dWidth);
        if(lDistance < dWidth)
        {
            dSumPath -= M_PI / 2 * (sqrt(pow(cPt3.x - cPt4.x, 2) + pow(cPt3.y - cPt4.y, 2)));
            dSumPath += sqrt(pow(cPt3.x - cPt4.x, 2) + pow(cPt3.y - cPt4.y, 2));
            break;
        }
        LineGetToPoint(lXClose, lYClose, cPt1.x, cPt1.y, cPtCen1.x, cPtCen1.y,
                       lDistance);
        cPt3.x = lXClose;
        cPt3.y = lYClose;
        if(icnt == 0)
        {
            dSumPath += M_PI / 2 * (sqrt(pow(cPt2.x - cPt3.x, 2) + pow(cPt2.y - cPt3.y, 2)));//���ʪ��`����
        }
        else
        {
            dSumPath += M_PI / 2 * (sqrt(pow(cPt4.x - cPt3.x, 2) + pow(cPt4.y - cPt3.y, 2)));//���ʪ��`����
        }
        m_ptVec.push_back(cPt3);
        cPt4.x = (LONG)((cPt3.x - cPtCen1.x)*cos(M_PI) - (cPt3.y - cPtCen1.y)*sin(M_PI) + cPtCen1.x);
        cPt4.y = (LONG)((cPt3.x - cPtCen1.x)*sin(M_PI) + (cPt3.y - cPtCen1.y)*cos(M_PI) + cPtCen1.y);
        dSumPath += M_PI / 2 * (sqrt(pow(cPt3.x - cPt4.x, 2) + pow(cPt3.y - cPt4.y, 2)));//���ʪ��`����
        m_ptVec.push_back(cPt4);
        icnt++;
    }
    if(lCloseDistance > 0) //�ϥ� --(5)�����Z��
    {
        dCloseTime = 1000000 * ((dSumPath - lCloseDistance) / (DOUBLE)lWorkVelociy);
        /*======�p�ɾ���Ĳ�o���_�����_���A�ϥ�z���_����================*/
        if(!m_bIsStop)
        {
            CAction::m_ZtimeOutGlueSet = FALSE;
            if(lStartDistance > 0)
            {
                //_cwprintf(_T("End�����Z�����ɶ�=%lf \n"), DOUBLE(lTime) + DOUBLE(dCloseTime / 1000000.0));
                MO_TimerSetIntter(lTime + LONG(dCloseTime), 1);//�p�ɨ���ܰ����
            }
            else
            {
                //_cwprintf(_T("End�����Z�����ɶ�=%lf \n"), DOUBLE(dCloseTime / 1000000.0));
                MO_TimerSetIntter(LONG(dCloseTime), 1);//�p�ɨ���ܰ����
            }
        }
    }
    #pragma endregion

    std::vector<DATA_2MOVE> DATA_2DO;
    UINT cnt = 0;
    DATA_2DO.clear();
    icnt = 0;
    for(ptIter = m_ptVec.begin(); ptIter != m_ptVec.end(); ptIter++)
    {
        if(lStartDistance < 0 && cnt == 0)
        {
            cnt++;
        }
        else if((iData != 0) && (cnt == m_ptVec.size() - 1))
        {
            MCO_Do2dDataLine((*ptIter).x, (*ptIter).y, DATA_2DO);
        }
        else if((iData == 0) && (cnt == m_ptVec.size() - 1))
        {
            MCO_Do2dDataCir((*ptIter).x, (*ptIter).y, cPtCen2.x, cPtCen2.y, bRev, DATA_2DO);
        }
        else
        {
            if(cnt % 2 == 0)
            {
                MCO_Do2dDataCir((*ptIter).x, (*ptIter).y, cPtCen2.x, cPtCen2.y, bRev, DATA_2DO);
            }
            else
            {
                MCO_Do2dDataCir((*ptIter).x, (*ptIter).y, lCenX, lCenY, bRev, DATA_2DO);
            }
        }
        cnt++;
    }
    LA_AbsToOppo2Move(DATA_2DO);
    if(lStartDistance > 0)
    {
        for(UINT i = 0; i < DATA_2DO.size(); i++)
        {
            DATA_2Do[i] = DATA_2DO.at(i);
        }
        MO_DO2Curve(DATA_2Do, DATA_2DO.size(), lWorkVelociy);
    }
    else
    {
        for(UINT i = 1; i < DATA_2DO.size(); i++)
        {
            DATA_2Do[i - 1] = DATA_2DO.at(i);
        }
        MO_DO2Curve(DATA_2Do, DATA_2DO.size() - 1, lWorkVelociy);
    }
    PreventMoveError();//�����X�ʿ��~
    Sleep(200);
    DATA_2DO.clear();

    MO_Timer(0, 0, lCloseONDelayTime * 1000);
    MO_Timer(1, 0, lCloseONDelayTime * 1000);//�u�q�I���]�w---(6)��������

    Sleep(1);//����X���A�קK�p�ɾ��쪽��0
    while(MO_Timer(3, 0, 0))
    {
        if(m_bIsStop == 1)
        {
            break;
        }
        Sleep(1);
    }
    MO_StopGumming();//����X��
    if(!m_bIsStop)
    {
        //*************************��R�^��z�b�t�ץثe���X�ʳt�ת��⭿******************
        MO_Do3DLineMove(0, 0, (lZ - lZBackDistance) - lZ, lWorkVelociy * 2, lAcceleration,
                        lInitVelociy);//Z�b��^
        PreventMoveError();//����b�d�X��
    }
    MO_Timer(0, 0, lCloseOffDelayTime * 1000);
    MO_Timer(1, 0, lCloseOffDelayTime * 1000);//�u�q�I���]�w---(3)���d�ɶ�
    Sleep(1);//����X���A�קK�p�ɾ��쪽��0
    while(MO_Timer(3, 0, 0))
    {
        if(m_bIsStop == 1)
        {
            break;
        }
        Sleep(1);
    }
#endif // MOVE
}
/*����---��R�κA(���A3�x�ζ�R.�ѥ~�Ӥ�)
*��J(�_�l�Ix1,y1,�����Ix2,y2,�e��,�X�ʳt��,�[�t��,��t��)
*/
void CAction::AttachFillType3(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lZ, LONG lZBackDistance, LONG lWidth,
                              LONG lStartDelayTime, LONG lStartDistance, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime,
                              LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
#ifdef MOVE
    #pragma region ****�Ƥ諬���Y4�I�\��****
    LONG lNowX = 0, lNowY = 0;
    CPoint cPt1 = 0, cPt2 = 0, cPt3 = 0, cPt4 = 0, cPtCen = 0;
    DOUBLE dRadius = 0, dDistance = 0, dWidth = 0, dAngCenCos = 0, dAngCenSin = 0, dAngCos = 0,
           dAngSin = 0;
    DOUBLE dAngCenCos2 = 0, dAngCenSin2 = 0, dAngCos2 = 0, dAngSin2 = 0;
    DOUBLE dSumPath = 0, dCloseTime = 0;///�`�Z��,�����ɶ�
    LONG timeUpGlue = 0;//�]�m�Z�������ʮɶ�
    int icnt = 0;//�p�ƾ�
    LONG S1 = 0, S2 = 0;//���ʲĤ@�G�q���Z��
    std::vector<CPoint>::iterator ptIter;//���N��
    std::vector<CPoint> m_ptVec;
    m_ptVec.clear();
    cPt1.x = lX1;
    cPt1.y = lY1;
    cPt3.x = lX2;
    cPt3.y = lY2;
    dWidth = lWidth * 1000;
    cPtCen.x = LONG(cPt1.x + (cPt3.x - cPt1.x) / 2.0);
    cPtCen.y = LONG(cPt1.y + (cPt3.y - cPt1.y) / 2.0);
    dRadius = sqrt(pow(cPt1.x - cPtCen.x, 2) + pow(cPt1.y - cPtCen.y, 2));
    if(dRadius == 0)
    {
        return;
    }
    dAngCenCos = acos(DOUBLE(cPt1.x - cPtCen.x) / dRadius);
    dAngCenSin = asin(DOUBLE(cPt1.y - cPtCen.y) / dRadius);
    dAngCenCos2 = M_PI * 2 - dAngCenCos;
    dAngCenSin2 = M_PI - dAngCenSin;
    if(abs(dAngCenCos - dAngCenSin) > 0.01)
    {
        if(abs(dAngCenCos - dAngCenSin2) < 0.01)
        {
            dAngCenSin = dAngCenSin2;
        }
        else if(abs(dAngCenCos2 - dAngCenSin) < 0.01)
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
    cPt2.x = LONG(dRadius*cos(dAngCenCos + M_PI_2) + cPtCen.x);
    cPt2.y = LONG(dRadius*sin(dAngCenSin + M_PI_2) + cPtCen.y);
    cPt4.x = LONG(dRadius*cos(dAngCenCos + M_PI_2 * 3) + cPtCen.x);
    cPt4.y = LONG(dRadius*sin(dAngCenSin + M_PI_2 * 3) + cPtCen.y);
    dDistance = sqrt(pow((cPt1.x - cPt4.x), 2) + pow((cPt1.y - cPt4.y), 2));

    #pragma region ****�u�q��]�m****
    //�ϥ�(1)���ʫe����(lStartDelayTime)
    if(lStartDelayTime>0)
    {
        if(!m_bIsStop)
        {
            MO_Do3DLineMove(lX1 - MO_ReadLogicPosition(0), lY1 - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration,
                            lInitVelociy);//�^��_�l�I!
            PreventMoveError();
        }
        if(!m_bIsStop)
        {
            MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//�^��_�l�I!
            PreventMoveError();
        }
        PauseDoGlue();//�Ȱ��^�_�᭫�s�(Ū���Ȱ��ѼơA��ѼƬ�0�ɥX���A�B�I�����n���}�C)
        if(!m_bIsStop && m_bIsDispend == 1)
        {
            MO_GummingSet();//�(���d)
        }
        MO_Timer(0, 0, lStartDelayTime * 1000);
        MO_Timer(1, 0, lStartDelayTime * 1000);//�u�q�I���]�w---(1)���ʫe����(�b�u�q�}�l�I�W)

        Sleep(1);//����X���A�קK�p�ɾ��쪽��0

        //�p��s1�Ps2
        S1 = LONG(sqrt(pow(cPt2.x - cPt1.x, 2) + (cPt2.y - cPt1.y, 2)));
        S2 = LONG(sqrt(pow(cPt3.x - cPt2.x, 2) + (cPt3.y - cPt2.y, 2)));
        while(MO_Timer(3, 0, 0))
        {
            if(m_bIsStop)
            {
                break;
            }
            Sleep(1);
        }
        /*���J�Ĥ@�I*/
        m_ptVec.push_back(cPt1);
    }
    //�ϥ�(2)�p��X���Z���A�]�w�X���I
    else if(lStartDistance>0)
    {
        CPoint ptSetDist(0, 0);
        LONG lXClose = 0, lYClose = 0;
        LineGetToPoint(lXClose, lYClose, cPt2.x, cPt2.y, lX1, lY1, lStartDistance);
        ptSetDist.x = (-(lXClose - lX1)) + lX1;
        ptSetDist.y = (-(lYClose - lY1)) + lY1;
        //���J�]�m�Z�����y�Ь��Ĥ@�I
        m_ptVec.push_back(ptSetDist);

        //�p��s1�Ps2
        S1 = LONG(sqrt(pow(cPt2.x - ptSetDist.x, 2) + (cPt2.y - ptSetDist.y, 2)));
        S2 = LONG(sqrt(pow(cPt3.x - cPt2.x, 2) + (cPt3.y - cPt2.y, 2)));

    }
    //(1)(2)�Ҥ��ϥ�
    else
    {
        m_ptVec.push_back(cPt1);

        //�p��s1�Ps2
        S1 = LONG(sqrt(pow(cPt2.x - cPt1.x, 2) + (cPt2.y - cPt1.y, 2)));
        S2 = LONG(sqrt(pow(cPt3.x - cPt2.x, 2) + (cPt3.y - cPt2.y, 2)));
    }

    #pragma endregion

    m_ptVec.push_back(cPt2);
    m_ptVec.push_back(cPt3);
    m_ptVec.push_back(cPt4);
    /*�`�Z���p��dSumPath*/
    dSumPath = sqrt(pow(cPt1.x - cPt2.x, 2) + pow(cPt1.y - cPt2.y, 2));
    dSumPath += sqrt(pow(cPt2.x - cPt3.x, 2) + pow(cPt2.y - cPt3.y, 2));
    dSumPath += sqrt(pow(cPt3.x - cPt4.x, 2) + pow(cPt3.y - cPt4.y, 2));//���ʪ��`����


    dAngCos = acos((cPt1.x - cPt4.x) / dDistance);
    dAngSin = asin((cPt1.y - cPt4.y) / dDistance);
    if(dAngCos < 0)
    {
        dAngCos += M_PI * 2;
    }
    if(dAngSin < 0)
    {
        dAngSin += M_PI * 2;
    }
    dAngCos2 = M_PI * 2 - dAngCos;
    dAngSin2 = M_PI - dAngSin;
    if(abs(dAngCos - dAngSin) > 0.01)
    {
        if(abs(dAngCos - dAngSin2) < 0.01)
        {
            dAngSin = dAngSin2;
        }
        else if(abs(dAngCos2 - dAngSin) < 0.01)
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
    icnt = 1;
    while(1)
    {
        dRadius = dRadius - dWidth*sqrt(2);
        dDistance = dDistance - dWidth;
        if(dDistance < dWidth)
        {
            if(icnt == 1)
            {
                dSumPath += sqrt(pow(cPt4.x - cPt1.x, 2) + pow(cPt4.y - cPt1.y, 2));//���ʪ��`����
            }
            break;
        }
        cPt1.x = LONG(dDistance*cos(dAngCos) + cPt4.x);
        cPt1.y = LONG(dDistance*sin(dAngSin) + cPt4.y);
        m_ptVec.push_back(cPt1);
        dSumPath += sqrt(pow(cPt4.x - cPt1.x, 2) + pow(cPt4.y - cPt1.y, 2));//���ʪ��`����
        cPt2.x = LONG(dRadius*cos(dAngCenCos + M_PI_2) + cPtCen.x);
        cPt2.y = LONG(dRadius*sin(dAngCenSin + M_PI_2) + cPtCen.y);
        m_ptVec.push_back(cPt2);
        dSumPath += sqrt(pow(cPt1.x - cPt2.x, 2) + pow(cPt1.y - cPt2.y, 2));//���ʪ��`����
        dDistance = dDistance - dWidth;
        if(dDistance < dWidth)
        {
            break;
        }
        cPt3.x = LONG(dRadius*cos(dAngCenCos + M_PI) + cPtCen.x);
        cPt3.y = LONG(dRadius*sin(dAngCenSin + M_PI) + cPtCen.y);
        m_ptVec.push_back(cPt3);
        dSumPath += sqrt(pow(cPt2.x - cPt3.x, 2) + pow(cPt2.y - cPt3.y, 2));//���ʪ��`����
        cPt4.x = LONG(dRadius*cos(dAngCenCos + M_PI_2 * 3.0) + cPtCen.x);
        cPt4.y = LONG(dRadius*sin(dAngCenSin + M_PI_2 * 3.0) + cPtCen.y);
        m_ptVec.push_back(cPt4);
        dSumPath += sqrt(pow(cPt3.x - cPt4.x, 2) + pow(cPt3.y - cPt4.y, 2));//���ʪ��`����
        icnt++;
    }
    #pragma endregion
    std::vector<DATA_2MOVE>DATA_2DO;
    DATA_2DO.clear();
    for(ptIter = m_ptVec.begin(); ptIter != m_ptVec.end(); ptIter++)
    {
        MCO_Do2dDataLine((*ptIter).x, (*ptIter).y, DATA_2DO);
    }
    //�I�}�C�ഫ
    LA_AbsToOppo2Move(DATA_2DO);
    for(UINT i = 1; i < DATA_2DO.size(); i++)
    {
        DATA_2Do[i - 1] = DATA_2DO.at(i);
    }
    if(!m_bIsStop)
    {
        MO_Do3DLineMove(DATA_2DO.at(0).EndP.x, DATA_2DO.at(0).EndP.y, 0, lWorkVelociy, lAcceleration,
                        lInitVelociy);//�^��_�l�I!
        PreventMoveError();
    }
    if(!m_bIsStop)
    {
        MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//�^��_�l�I!
        PreventMoveError();
    }
    //�ϥ�(2)�]�m�Z��
    if(lStartDistance>0)
    {
        //timeUpGlue ���us
        timeUpGlue = CalPreglue(lStartDistance, lWorkVelociy, lAcceleration, lInitVelociy);
        if(!m_bIsStop)
        {
            CAction::m_YtimeOutGlueSet = TRUE;
            MO_TimerSetIntter(timeUpGlue, 0);//�ϥ�y timer���_ �X��
        }
    }
    //�ϥ�(5)�����Z��(lCloseDistance)
    DOUBLE T1 = 0, T2 = 0;
    LONG V1 = 0, V2 = 0, AllS = 0, AllT = 0, FinallV = 0;
    if(lCloseDistance>0)
    {
        //�Ĥ@�϶��s�򴡸�
        T1 = (-lInitVelociy / 1000 + (sqrt(pow(lInitVelociy / 1000, 2) + 2 * lAcceleration / 1000 * S1 / 1000))) / (DOUBLE)(lAcceleration / 1000.0);
        V1 = LONG(lInitVelociy + lAcceleration*T1);
        if(lWorkVelociy > V1)
        {
            AllT = LONG(T1 * 1000000);
            //�ĤG�϶��s�򴡸�
            T2 = (-V1 / 1000 + (sqrt(pow(V1 / 1000, 2) + 2 * lAcceleration / 1000 * S1 / 1000))) / (DOUBLE)(lAcceleration / 1000.0);
            V2 = LONG(lAcceleration* LONG(T1) + V1);
            if(lWorkVelociy > V2)
            {
                AllT = LONG(AllT + (T2 * 1000000));
                AllS = S1 + S2;
                FinallV = V2;
            }
            else
            {
                AllS = S1;
                FinallV = lWorkVelociy;
            }
        }
        else
        {
            FinallV = lWorkVelociy;
            AllS = 0;
        }
        if(lStartDistance > 0)
        {
            dCloseTime = 1000000 * ((dSumPath - lCloseDistance) / (DOUBLE)lWorkVelociy);
            /*======�p�ɾ���Ĳ�o���_�����_���A�ϥ�z���_����================*/
            if(!m_bIsStop)
            {
                CAction::m_ZtimeOutGlueSet = FALSE;
                //_cwprintf(_T("End�����Z�����ɶ�=%lf \n"), DOUBLE(timeUpGlue) + DOUBLE(dCloseTime / 1000000.0));
                MO_TimerSetIntter(timeUpGlue + LONG(dCloseTime), 1);//�p�ɨ���ܰ����
            }
        }
        else
        {
            LONG lCloseTime = 0;
            lCloseTime = LONG(CalPreglue(LONG(dSumPath - AllS - lCloseDistance), FinallV, lAcceleration, lInitVelociy));
            lCloseTime = lCloseTime - AllT;
            /*======�p�ɾ���Ĳ�o���_�����_���A�ϥ�z���_����================*/
            if(!m_bIsStop)
            {
                CAction::m_ZtimeOutGlueSet = FALSE;
                //_cwprintf(_T("End�����Z�����ɶ�=%d \n"), lCloseTime);
                MO_TimerSetIntter(lCloseTime, 1);//�p�ɨ���ܰ����
            }
        }
        /*LONG sumPath = 0;
        LONG finishTime = 0;
        DOUBLE avgTime = 0;
        LONG accLength = CalPreglue(lWorkVelociy, lAcceleration, lInitVelociy);
        for(UINT i = 1; i<DATA_2DO.size(); i++)
        {
        sumPath += DATA_2DO.at(i).Distance;
        }
        avgTime = ((DOUBLE)sumPath-(DOUBLE)accLength)/(DOUBLE)lWorkVelociy;
        finishTime = (LONG)round(avgTime*1000000)+CalPreglueTime(lWorkVelociy, lAcceleration, lInitVelociy);
        LONG closeDistTime = CalPreglue(lCloseDistance, lWorkVelociy, 0, lInitVelociy);
        CAction::m_ZtimeOutGlueSet = FALSE;
        MO_TimerSetIntter(finishTime-closeDistTime, 1);*/
    }

    if(lStartDelayTime == 0 && lStartDistance == 0)
    {
        PauseDoGlue();//�Ȱ���_���~��X��(m_bIsPause=0)�X��
    }

    MO_DO2Curve(DATA_2Do, DATA_2DO.size() - 1, lWorkVelociy);
    PreventMoveError();//�����X�ʿ��~
    Sleep(200);
    DATA_2DO.clear();

    //�ϥ�(3)���d�ɶ�(lCloseOffDelayTime)
    if(lCloseOffDelayTime>0)
    {
        MO_Timer(0, 0, lCloseOffDelayTime * 1000);
        MO_Timer(1, 0, lCloseOffDelayTime * 1000);//�u�q�I���]�w---(3)���d�ɶ�
        Sleep(1);//����X���A�קK�p�ɾ��쪽��0
        while(MO_Timer(3, 0, 0))
        {
            if(m_bIsStop == 1)
            {
                break;
            }
            Sleep(1);
        }
    }

    PauseStopGlue();//�Ȱ��ɰ����(m_bIsPause=1)
    MO_StopGumming();//����X��

    //�ϥ�(6)��������(lCloseONDelayTime)
    if(!(lCloseDistance>0) && lCloseONDelayTime>0)
    {
        MO_Timer(0, 0, lCloseONDelayTime * 1000);
        MO_Timer(1, 0, lCloseONDelayTime * 1000);//�u�q�I���]�w---(6)��������
        Sleep(1);//����X���A�קK�p�ɾ��쪽��0
        while(MO_Timer(3, 0, 0))
        {
            if(m_bIsStop == 1)
            {
                break;
            }
            Sleep(1);
        }
    }

    if(!m_bIsStop)
    {
        //*************************��R�^��z�b�t�ץثe���X�ʳt�ת��⭿******************
        MO_Do3DLineMove(0, 0, (lZ - lZBackDistance) - lZ, lWorkVelociy * 2, lAcceleration,
                        lInitVelociy);//Z�b��^
        PreventMoveError();//����b�d�X��
    }
#endif
}
/*����---��R�κA(���A4�x����)
*��J(�_�l�Ix1,y1,�����Ix2,y2,�e��,��ݼe��,�X�ʳt��,�[�t��,��t��)
*/
void CAction::AttachFillType4(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lZ, LONG lZBackDistance, LONG lWidth,
                              LONG lWidth2,
                              LONG lStartDelayTime, LONG lStartDistance, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime,
                              LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
#ifdef MOVE
    #pragma region ****�Ƥ諬���Y4�I�\��****
    LONG lNowX = 0, lNowY = 0;
    CPoint cPt1 = 0, cPt2 = 0, cPt3 = 0, cPt4 = 0, cPtCen = 0;
    DOUBLE dRadius = 0, dDistance = 0, dWidth = 0, dWidth2 = 0, dAngCenCos = 0,
           dAngCenSin = 0, dAngCos = 0, dAngSin = 0;
    DOUBLE dAngCenCos2 = 0, dAngCenSin2 = 0, dAngCos2 = 0, dAngSin2 = 0;
    int iBuff = 0;//�P�_��ݼe�ץ�
    std::vector<CPoint>::iterator ptIter;//���N��
    std::vector<CPoint> m_ptVec;
    m_ptVec.clear();
    cPt1.x = lX1;
    cPt1.y = lY1;
    cPt3.x = lX2;
    cPt3.y = lY2;
    dWidth = lWidth * 1000;
    dWidth2 = lWidth2 * 1000;
    cPtCen.x = LONG(cPt1.x + (cPt3.x - cPt1.x) / 2.0);
    cPtCen.y = LONG(cPt1.y + (cPt3.y - cPt1.y) / 2.0);
    dRadius = sqrt(pow(cPt1.x - cPtCen.x, 2) + pow(cPt1.y - cPtCen.y, 2));
    if(dRadius == 0)
    {
        return;
    }
    dAngCenCos = acos(DOUBLE(cPt1.x - cPtCen.x) / dRadius);
    dAngCenSin = asin(DOUBLE(cPt1.y - cPtCen.y) / dRadius);
    dAngCenCos2 = M_PI * 2 - dAngCenCos;
    dAngCenSin2 = M_PI - dAngCenSin;
    if(abs(dAngCenCos - dAngCenSin) > 0.01)
    {
        if(abs(dAngCenCos - dAngCenSin2) < 0.01)
        {
            dAngCenSin = dAngCenSin2;
        }
        else if(abs(dAngCenCos2 - dAngCenSin) < 0.01)
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
    cPt2.x = LONG(dRadius*cos(dAngCenCos + M_PI_2) + cPtCen.x);
    cPt2.y = LONG(dRadius*sin(dAngCenSin + M_PI_2) + cPtCen.y);
    cPt4.x = LONG(dRadius*cos(dAngCenCos + M_PI_2 * 3) + cPtCen.x);
    cPt4.y = LONG(dRadius*sin(dAngCenSin + M_PI_2 * 3) + cPtCen.y);
    dDistance = sqrt(pow((cPt1.x - cPt4.x), 2) + pow((cPt1.y - cPt4.y), 2));


    #pragma region ****�u�q��]�m****
    //�ϥ�(1)���ʫe����(lStartDelayTime)
    if(lStartDelayTime>0)
    {
        if(!m_bIsStop)
        {
            MO_Do3DLineMove(lX1 - MO_ReadLogicPosition(0), lY1 - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration,
                            lInitVelociy);//�^��_�l�I!
            PreventMoveError();
        }
        if(!m_bIsStop)
        {
            MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//�^��_�l�I!
            PreventMoveError();
        }
        PauseDoGlue();//�Ȱ��^�_�᭫�s�(Ū���Ȱ��ѼơA��ѼƬ�0�ɥX���A�B�I�����n���}�C)
        if(!m_bIsStop && m_bIsDispend == 1)
        {
            MO_GummingSet();//�(���d)
        }
        MO_Timer(0, 0, lStartDelayTime * 1000);
        MO_Timer(1, 0, lStartDelayTime * 1000);//�u�q�I���]�w---(1)���ʫe����(�b�u�q�}�l�I�W)

        Sleep(1);//����X���A�קK�p�ɾ��쪽��0
        while(MO_Timer(3, 0, 0))
        {
            if(m_bIsStop)
            {
                break;
            }
            Sleep(1);
        }
        /*���J�Ĥ@�I*/
        m_ptVec.push_back(cPt1);
    }
    //�ϥ�(2)�p��X���Z���A�]�w�X���I
    else if(lStartDistance>0)
    {
        CPoint ptSetDist(0, 0);
        LONG glueDist = lStartDistance;
        DOUBLE dLength = sqrt(pow(cPt1.x - cPt2.x, 2) + pow(cPt1.y - cPt2.y, 2));
        if(cPt1.x == cPt2.x)
        {
            ptSetDist = cPt1;
            glueDist = LONG(glueDist*M_SQRT1_2);
            ptSetDist.y = (cPt1.y<cPt2.y) ? cPt1.y - glueDist : cPt1.y + glueDist;
        }
        else if(cPt1.y == cPt2.y)
        {
            ptSetDist = cPt1;
            glueDist = LONG(glueDist*M_SQRT1_2);
            ptSetDist.x = (cPt1.x<cPt2.x) ? cPt1.x - glueDist : cPt1.x + glueDist;
        }
        else
        {
            CPoint ptDist(0, 0);
            ptDist.x = (LONG)round(glueDist*abs(cPt1.x - cPt2.x) / dLength);
            ptDist.y = (LONG)round(glueDist*abs(cPt1.y - cPt2.y) / dLength);

            ptSetDist.x = (cPt1.x<cPt2.x) ? cPt1.x - ptDist.x : cPt1.x + ptDist.x;
            ptSetDist.y = (cPt1.y<cPt2.y) ? cPt1.y - ptDist.y : cPt1.y + ptDist.y;
        }
        //���J�]�m�Z�����y�Ь��Ĥ@�I
        m_ptVec.push_back(ptSetDist);
    }
    //(1)(2)�Ҥ��ϥ�
    else
    {
        m_ptVec.push_back(cPt1);
    }
    #pragma endregion //�����u�q��]�m

    m_ptVec.push_back(cPt2);
    m_ptVec.push_back(cPt3);
    m_ptVec.push_back(cPt4);

    dAngCos = acos((cPt1.x - cPt4.x) / dDistance);
    dAngSin = asin((cPt1.y - cPt4.y) / dDistance);
    if(dAngCos < 0)
    {
        dAngCos += M_PI * 2;
    }
    if(dAngSin < 0)
    {
        dAngSin += M_PI * 2;
    }
    dAngCos2 = M_PI * 2 - dAngCos;
    dAngSin2 = M_PI - dAngSin;
    if(abs(dAngCos - dAngSin) > 0.01)
    {
        if(abs(dAngCos - dAngSin2) < 0.01)
        {
            dAngSin = dAngSin2;
        }
        else if(abs(dAngCos2 - dAngSin) < 0.01)
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
    while(1)
    {
        if((iBuff == 1) && (dRadius - (2 * dWidth2) < 0))
        {
#ifdef PRINTF
      //      _cwprintf(_T("��ݼe�׹L�j \n"));
#endif
            m_ptVec.push_back(cPt1);
            break;
        }
        dRadius = dRadius - dWidth*sqrt(2);
        dDistance = dDistance - dWidth;
        if(iBuff*dWidth >  dWidth2)    //dWidth2��ݼe��
        {
            dDistance += dWidth;
            cPt1.x = LONG(dDistance*cos(dAngCos) + cPt4.x);
            cPt1.y = LONG(dDistance*sin(dAngSin) + cPt4.y);
            m_ptVec.push_back(cPt1);
            break;
        }
        if(dDistance < dWidth)
        {
            break;
        }
        cPt1.x = LONG(dDistance*cos(dAngCos) + cPt4.x);
        cPt1.y = LONG(dDistance*sin(dAngSin) + cPt4.y);
        m_ptVec.push_back(cPt1);
        cPt2.x = LONG(dRadius*cos(dAngCenCos + M_PI_2) + cPtCen.x);
        cPt2.y = LONG(dRadius*sin(dAngCenSin + M_PI_2) + cPtCen.y);
        m_ptVec.push_back(cPt2);
        dDistance = dDistance - dWidth;
        if(dDistance < dWidth)
        {
            break;
        }
        cPt3.x = LONG(dRadius*cos(dAngCenCos + M_PI) + cPtCen.x);
        cPt3.y = LONG(dRadius*sin(dAngCenSin + M_PI) + cPtCen.y);
        m_ptVec.push_back(cPt3);
        cPt4.x = LONG(dRadius*cos(dAngCenCos + M_PI_2 * 3) + cPtCen.x);
        cPt4.y = LONG(dRadius*sin(dAngCenSin + M_PI_2 * 3) + cPtCen.y);
        m_ptVec.push_back(cPt4);
        iBuff++;
    }
    #pragma endregion

    std::vector<DATA_2MOVE> DATA_2DO;
    DATA_2DO.clear();
    for(ptIter = m_ptVec.begin(); ptIter != m_ptVec.end(); ptIter++)
    {
        MCO_Do2dDataLine((*ptIter).x, (*ptIter).y, DATA_2DO);
    }
    //�I�}�C�ഫ
    LA_AbsToOppo2Move(DATA_2DO);
    for(UINT i = 1; i < DATA_2DO.size(); i++)
    {
        DATA_2Do[i - 1] = DATA_2DO.at(i);
    }
    if(!m_bIsStop)
    {
        MO_Do3DLineMove(DATA_2DO.at(0).EndP.x, DATA_2DO.at(0).EndP.y, 0, lWorkVelociy, lAcceleration,
                        lInitVelociy);//�^��_�l�I!
        PreventMoveError();
    }
    if(!m_bIsStop)
    {
        MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//�^��_�l�I!
        PreventMoveError();
    }
    //�ϥ�(2)�]�m�Z��
    if(lStartDistance>0)
    {
        //timeUpGlue ���us
        LONG timeUpGlue = CalPreglue(lStartDistance, lWorkVelociy, lAcceleration, lInitVelociy);
        if(!m_bIsStop)
        {
            m_YtimeOutGlueSet = TRUE;
            MO_TimerSetIntter(timeUpGlue, 0);//�ϥ�z timer���_ �X��
        }
    }
    //�ϥ�(5)�����Z��(lCloseDistance)
    if(lCloseDistance>0)
    {
        LONG sumPath = 0;
        LONG finishTime = 0;
        DOUBLE avgTime = 0;
        LONG accLength = CalPreglue(lWorkVelociy, lAcceleration, lInitVelociy);
        for(UINT i = 1; i<DATA_2DO.size(); i++)
        {
            sumPath += DATA_2DO.at(i).Distance;
        }
        avgTime = ((DOUBLE)sumPath - (DOUBLE)accLength) / (DOUBLE)lWorkVelociy;
        finishTime = (LONG)round(avgTime * 1000000) + CalPreglueTime(lWorkVelociy, lAcceleration, lInitVelociy);
        LONG closeDistTime = CalPreglue(lCloseDistance, lWorkVelociy, 0, lInitVelociy);
        if(!m_bIsStop)
        {
            CAction::m_ZtimeOutGlueSet = FALSE;
            MO_TimerSetIntter(finishTime - closeDistTime, 1);
        }
    }

    if(lStartDelayTime == 0 && lStartDistance == 0)
    {
        PauseDoGlue();//�Ȱ���_���~��X��(m_bIsPause=0)�X��
    }
    MO_DO2Curve(DATA_2Do, DATA_2DO.size() - 1, lWorkVelociy);
    PreventMoveError();//�����X�ʿ��~
    Sleep(200);
    DATA_2DO.clear();

    //�ϥ�(3)���d�ɶ�(lCloseOffDelayTime)
    if(lCloseOffDelayTime>0)
    {
        MO_Timer(0, 0, lCloseOffDelayTime * 1000);
        MO_Timer(1, 0, lCloseOffDelayTime * 1000);//�u�q�I���]�w---(3)���d�ɶ�
        Sleep(1);//����X���A�קK�p�ɾ��쪽��0
        while(MO_Timer(3, 0, 0))
        {
            if(m_bIsStop == 1)
            {
                break;
            }
            Sleep(1);
        }
    }

    PauseStopGlue();//�Ȱ��ɰ����(m_bIsPause=1)
    MO_StopGumming();//����X��
    //�ϥ�(6)��������(lCloseONDelayTime)
    if(!(lCloseDistance>0) && lCloseONDelayTime>0)
    {
        MO_Timer(0, 0, lCloseONDelayTime * 1000);
        MO_Timer(1, 0, lCloseONDelayTime * 1000);//�u�q�I���]�w---(6)��������
        Sleep(1);//����X���A�קK�p�ɾ��쪽��0
        while(MO_Timer(3, 0, 0))
        {
            if(m_bIsStop == 1)
            {
                break;
            }
            Sleep(1);
        }
    }
    if(!m_bIsStop)
    {
        //*************************��R�^��z�b�t�ץثe���X�ʳt�ת��⭿******************
        MO_Do3DLineMove(0, 0, (lZ - lZBackDistance) - lZ, lWorkVelociy * 2, lAcceleration,
                        lInitVelociy);//Z�b��^
        PreventMoveError();//����b�d�X��
    }
#endif
}
/*����---��R�κA(���A5����)
*��J(�_�l�Ix1,y1,�����Ix2,y2,�e��,��ݼe��,�X�ʳt��,�[�t��,��t��)
*/
void CAction::AttachFillType5(LONG lX1, LONG lY1, LONG lCenX, LONG lCenY,
                              LONG lZ, LONG lZBackDistance, LONG lWidth, LONG lWidth2,
                              LONG lStartDelayTime, LONG lStartDistance, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime,
                              LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
    /*�u�q�I���]�w(1.���ʫe����A2.�]�m�Z���A3���d�ɶ��A5�����Z���A6��������)
    LONG lStartDelayTime ,LONG lStartDistance ,LONG lCloseOffDelayTime ,LONG lCloseDistance ,LONG lCloseONDelayTime
    //1.���ʫe�I�����b�@���u�q�_�l�I�B�O�����}���ɪ��C �����ɥi����w�Y�b�y��y�ʤ��e�u�u�q�o�Ͳ��ʡC
    //2. �I�����}�ҫe�A ���F���}���u�u�q�_�l�I�����ʶZ���C �ӶZ�������F���ѤF�������_�t�ɶ��A�D�n�ΨӮ����L�q�y��b�u�q�_�l�B���n�E�C
    //3.�I����������A���F�����O�b�w�Y���ܤU�@�I�e�ܱo���ŦӦb�u�q�I�������I�B���ͪ����ɡC
    //5.������L�q�y��b�u�q�����I�B�o�Ͱ�n�A�I�����b�Z���u�q�����I�e�h���B�����C
    //6.�I�����b�u�q�����I�B�����O���}�Ҫ��ɪ��C
    */
#ifdef MOVE
    #pragma region ****�ꫬ���ۥ\��****
    DOUBLE dRadius = 0, dWidth = 0, dWidth2 = 0, dAng0 = 0, dAng1 = 0, dAng2 = 0;
    BOOL bRev = 1;//0�f��/1����
    LONG lLineClose = 0, lXClose = 0, lYClose = 0, lDistance = 0;
    LONG lNowX = 0, lNowY = 0, lTime = 0;
    CPoint cPt1 = 0, cPt2 = 0, cPt3 = 0, cPt4 = 0, cPtCen1 = 0, cPtCen2 = 0;
    DOUBLE dSumPath = 0, dCloseTime = 0;//�`���|��.�����ɶ�
    int iData = 0, iBuff = 0;//�P�_�O�_���l�ơAbuff�Ω�p�ƨ�ݼe��
    std::vector<CPoint>m_ptVec;
    std::vector<CPoint>::iterator ptIter;//���N��
    m_ptVec.clear();
    cPt1.x = lX1;
    cPt1.y = lY1;
    cPtCen1.x = lCenX;
    cPtCen1.y = lCenY;//�W�b����
    dRadius = sqrt(pow(cPtCen1.x - cPt1.x, 2) + pow(cPtCen1.y - cPt1.y, 2));//�b�|
    if(LONG(dRadius) == 0)
    {
        return;
    }
    if(LONG(dRadius) == 0)
    {
        return;
    }
    if(lStartDistance>0) //�ϥ�--(2)�]�m�Z��(lStartDistance)
    {
        LONG lStartX = 0, lStartY = 0;
        ArcGetToPoint(lStartX, lStartY, lStartDistance, lX1, lY1, lCenX, lCenY, LONG(dRadius), bRev);//��X�_�l�I
        if(!m_bIsStop)
        {
            MO_Do3DLineMove(lStartX - MO_ReadLogicPosition(0), lStartY - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration,
                            lInitVelociy);//�^��_�l�I!
            PreventMoveError();
        }
        if(!m_bIsStop)
        {
            MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//�^��_�l�I!
            PreventMoveError();
        }
        lTime = LONG(1000000 * (DOUBLE)lStartDistance / (DOUBLE)lWorkVelociy);
#ifdef PRINTF
        //_cwprintf(_T("�p�ɾ��]�m�Z�����ɶ�=%lf \n"), DOUBLE(lTime / 1000000.0));
#endif
        /*======�p�ɾ���Ĳ�o���_����X���A�ϥ�y���_����================*/
        if(!m_bIsStop)
        {
            CAction::m_YtimeOutGlueSet = TRUE;
            MO_TimerSetIntter(lTime, 0);//�p�ɨ���ܰ����
        }
    }
    else
    {

        if(!m_bIsStop)
        {
            MO_Do3DLineMove(lX1 - MO_ReadLogicPosition(0), lY1 - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration,
                            lInitVelociy);//�^��_�l�I!
            PreventMoveError();
        }
        if(!m_bIsStop)
        {
            MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//�^��_�l�I!
            PreventMoveError();
        }
        PauseDoGlue();//�Ȱ��^�_�᭫�s�(Ū���Ȱ��ѼơA��ѼƬ�0�ɥX���A�B�I�����n���}�C)
        if(!m_bIsStop && m_bIsDispend == 1)
        {
            MO_GummingSet();//�(���d)
        }
        if(lStartDelayTime > 0) //�ϥ�(1)���ʫe����(lStartDelayTime)
        {
            MO_Timer(0, 0, lStartDelayTime * 1000);
            MO_Timer(1, 0, lStartDelayTime * 1000);//�u�q�I���]�w---(1)���ʫe����(�b�u�q�}�l�I�W)
            Sleep(1);//����X���A�קK�p�ɾ��쪽��0
            while(MO_Timer(3, 0, 0))
            {
                if(m_bIsStop)
                {
                    break;
                }
                Sleep(1);
            }
        }
    }
    lDistance = LONG(dRadius);
    dWidth = lWidth * 1000;
    dWidth2 = lWidth2 * 1000;
    dAng1 = acos((cPt1.x - cPtCen1.x) / dRadius);
    dAng2 = asin((cPt1.y - cPtCen1.y) / dRadius);
    //cPt2.x = LONG(dRadius*cos(dAng1 + M_PI) + cPtCen1.x);
    //cPt2.y = LONG(dRadius*sin(dAng2 + M_PI) + cPtCen1.y);
    cPt2.x = (LONG)((cPt1.x - cPtCen1.x)*cos(M_PI) - (cPt1.y - cPtCen1.y)*sin(M_PI) + cPtCen1.x);
    cPt2.y = (LONG)((cPt1.x - cPtCen1.x)*sin(M_PI) + (cPt1.y - cPtCen1.y)*cos(M_PI) + cPtCen1.y);
    dSumPath = M_PI*dRadius;//���ʪ��`����
    m_ptVec.push_back(cPt1);
    m_ptVec.push_back(cPt2);
    lDistance = LONG(lDistance - dWidth);
    LineGetToPoint(lXClose, lYClose, cPt1.x, cPt1.y, cPtCen1.x, cPtCen1.y,
                   lDistance);
    cPt3.x = lXClose;
    cPt3.y = lYClose;
    cPtCen2.x = LONG((cPt3.x + cPt2.x) / 2.0);
    cPtCen2.y = LONG((cPt3.y + cPt2.y) / 2.0);//�U�b����
    lDistance = LONG(lDistance + dWidth);
    iData = (int)dRadius % (int)dWidth;
    iBuff = 1;
    while(1)
    {
        if((iBuff == 1) && (lDistance - (2 * dWidth2) < 0)) //��ܨϥγ����������
        {
#ifdef PRINTF
            //_cwprintf(_T("��ݼe�׹L�j \n"));
#endif
            dSumPath += M_PI*dRadius;
            iBuff = 1;
            m_ptVec.push_back(cPt1);//������
            break;
        }
        else if((iBuff == 1) && dWidth2 == 0) //��ܨϥγ����������
        {
#ifdef PRINTF
            //_cwprintf(_T("����� \n"));
#endif
            dSumPath += M_PI*dRadius;
            iBuff = 1;
            m_ptVec.push_back(cPt1);//������
            break;
        }
        dRadius = dRadius - dWidth;
        lDistance = LONG(lDistance - dWidth);
        if(iBuff * dWidth > dWidth2)
        {
            lDistance += LONG(dWidth);
            LineGetToPoint(lXClose, lYClose, cPt1.x, cPt1.y, cPtCen1.x, cPtCen1.y,
                           lDistance);
            cPt3.x = lXClose;
            cPt3.y = lYClose;
            m_ptVec.push_back(cPt3);
            if(iBuff == 1)
            {
                dSumPath += M_PI / 2 * (sqrt(pow(cPt2.x - cPt3.x, 2) + pow(cPt2.y - cPt3.y, 2)));//���ʪ��`����
            }
            else
            {
                dSumPath += M_PI / 2 * (sqrt(pow(cPt3.x - cPt4.x, 2) + pow(cPt3.y - cPt4.y, 2)));//���ʪ��`����
            }
            break;
        }
        LineGetToPoint(lXClose, lYClose, cPt1.x, cPt1.y, cPtCen1.x, cPtCen1.y,
                       lDistance);
        cPt3.x = lXClose;
        cPt3.y = lYClose;
        if(iBuff == 1)
        {
            dSumPath += M_PI / 2 * (sqrt(pow(cPt2.x - cPt3.x, 2) + pow(cPt2.y - cPt3.y, 2)));//���ʪ��`����
        }
        else
        {
            dSumPath += M_PI / 2 * (sqrt(pow(cPt4.x - cPt3.x, 2) + pow(cPt4.y - cPt3.y, 2)));//���ʪ��`����
        }
        m_ptVec.push_back(cPt3);
        cPt4.x = LONG(dRadius*cos(dAng1 + M_PI) + cPtCen1.x);
        cPt4.y = LONG(lDistance*sin(dAng2 + M_PI) + cPtCen1.y);
        dSumPath += M_PI / 2 * (sqrt(pow(cPt3.x - cPt4.x, 2) + pow(cPt3.y - cPt4.y, 2)));//���ʪ��`����
        m_ptVec.push_back(cPt4);
        iBuff++;
    }
    if(lCloseDistance > 0) //�ϥ� --(5)�����Z��
    {
        dCloseTime = 1000000 * ((dSumPath - lCloseDistance) / (DOUBLE)lWorkVelociy);
        /*======�p�ɾ���Ĳ�o���_�����_���A�ϥ�z���_����================*/
        if(!m_bIsStop)
        {
            CAction::m_ZtimeOutGlueSet = FALSE;
            if(lStartDistance > 0)
            {
#ifdef PRINTF
               // _cwprintf(_T("End�����Z�����ɶ�=%lf \n"), DOUBLE(lTime) + DOUBLE(dCloseTime / 1000000.0));
#endif
                MO_TimerSetIntter(lTime + LONG(dCloseTime), 1);//�p�ɨ���ܰ����
            }
            else
            {
#ifdef PRINTF
             //   _cwprintf(_T("End�����Z�����ɶ�=%lf \n"), DOUBLE(dCloseTime / 1000000.0));
#endif
                MO_TimerSetIntter(LONG(dCloseTime), 1);//�p�ɨ���ܰ����
            }
        }
    }
    #pragma endregion
    std::vector<DATA_2MOVE> DATA_2DO;
    DATA_2DO.clear();
    if(iBuff == 1) //��ܨ�ݼe�׬��s  ���e�X�@�ӳ����
    {
        for(UINT i = 1; i < m_ptVec.size(); i++)
        {
            {
                MCO_Do2dDataCir(m_ptVec.at(i).x, m_ptVec.at(i).y, cPtCen1.x, cPtCen1.y, bRev, DATA_2DO);//�W�b��
            }
        }
    }
    else
    {
        for(UINT i = 1; i < m_ptVec.size(); i++)
        {
            if(i == m_ptVec.size() - 1)
            {
                if(i % 2 == 0)
                {
                    MCO_Do2dDataCir(m_ptVec.at(i).x, m_ptVec.at(i).y, cPtCen1.x, cPtCen1.y, bRev, DATA_2DO);//�W�b��
                }
                else
                {
                    MCO_Do2dDataCir(m_ptVec.at(i).x, m_ptVec.at(i).y, cPtCen2.x, cPtCen2.y, bRev, DATA_2DO);//�U�b��
                }
            }
            else
            {
                if(i % 2 != 0)
                {
                    MCO_Do2dDataCir(m_ptVec.at(i).x, m_ptVec.at(i).y, cPtCen1.x, cPtCen1.y, bRev, DATA_2DO);//�W�b��
                }
                else
                {
                    MCO_Do2dDataCir(m_ptVec.at(i).x, m_ptVec.at(i).y, cPtCen2.x, cPtCen2.y, bRev, DATA_2DO);//�U�b��
                }
            }
        }
    }
    LA_AbsToOppo2Move(DATA_2DO);
    for(UINT i = 0; i < DATA_2DO.size(); i++)
    {
        DATA_2Do[i] = DATA_2DO.at(i);
    }
    MO_DO2Curve(DATA_2Do, DATA_2DO.size(), lWorkVelociy);
    PreventMoveError();//�����X�ʿ��~
    Sleep(200);
    DATA_2DO.clear();
    MO_Timer(0, 0, lCloseONDelayTime * 1000);
    MO_Timer(1, 0, lCloseONDelayTime * 1000);//�u�q�I���]�w---(6)��������
    Sleep(1);//����X���A�קK�p�ɾ��쪽��0
    while(MO_Timer(3, 0, 0))
    {
        if(m_bIsStop == 1)
        {
            break;
        }
        Sleep(1);
    }
    MO_StopGumming();//����X��
    if(!m_bIsStop)
    {
        //*************************��R�^��z�b�t�ץثe���X�ʳt�ת��⭿******************
        MO_Do3DLineMove(0, 0, (lZ - lZBackDistance) - lZ, lWorkVelociy * 2, lAcceleration,
                        lInitVelociy);//Z�b��^
        PreventMoveError();//����b�d�X��
    }
    MO_Timer(0, 0, lCloseOffDelayTime * 1000);
    MO_Timer(1, 0, lCloseOffDelayTime * 1000);//�u�q�I���]�w---(3)���d�ɶ�
    Sleep(1);//����X���A�קK�p�ɾ��쪽��0
    while(MO_Timer(3, 0, 0))
    {
        if(m_bIsStop == 1)
        {
            break;
        }
        Sleep(1);
    }
#endif // MOVE
}
/*����---��R�κA(���A6�x�ζ�R.�Ѥ��ӥ~)
*��J(�_�l�Ix1,y1,�����Ix2,y2,�e��,�X�ʳt��,�[�t��,��t��)
*/
void CAction::AttachFillType6(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lZ, LONG lZBackDistance, LONG lWidth,
                              LONG lStartDelayTime, LONG lStartDistance, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime,
                              LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
#ifdef MOVE
    #pragma region ****�Ƥ諬���Y4�I�\��****
    LONG lNowX = 0, lNowY = 0, lNowZ = 0;
    CPoint cPt1 = 0, cPt2 = 0, cPt3 = 0, cPt4 = 0, cPtCen = 0;
    DOUBLE dRadius = 0, dDistance = 0, dWidth = 0, dWidth2 = 0, dAngCenCos = 0,
           dAngCenSin = 0, dAngCos = 0, dAngSin = 0;
    DOUBLE dAngCenCos2 = 0, dAngCenSin2 = 0, dAngCos2 = 0, dAngSin2 = 0;
    std::vector<CPoint>::reverse_iterator rptIter;//�ϦV���N��
    std::vector<CPoint> m_ptVec;
    m_ptVec.clear();
    cPt1.x = lX1;
    cPt1.y = lY1;
    cPt3.x = lX2;
    cPt3.y = lY2;
    dWidth = lWidth * 1000;
    cPtCen.x = LONG(cPt1.x + (cPt3.x - cPt1.x) / 2.0);
    cPtCen.y = LONG(cPt1.y + (cPt3.y - cPt1.y) / 2.0);
    dRadius = sqrt(pow(cPt1.x - cPtCen.x, 2) + pow(cPt1.y - cPtCen.y, 2));
    if(dRadius == 0)
    {
        return;
    }
    dAngCenCos = acos(DOUBLE(cPt1.x - cPtCen.x) / dRadius);
    dAngCenSin = asin(DOUBLE(cPt1.y - cPtCen.y) / dRadius);
    dAngCenCos2 = M_PI * 2 - dAngCenCos;
    dAngCenSin2 = M_PI - dAngCenSin;
    if(abs(dAngCenCos - dAngCenSin) > 0.01)
    {
        if(abs(dAngCenCos - dAngCenSin2) < 0.01)
        {
            dAngCenSin = dAngCenSin2;
        }
        else if(abs(dAngCenCos2 - dAngCenSin) < 0.01)
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
    cPt2.x = LONG(dRadius*cos(dAngCenCos + M_PI_2) + cPtCen.x);
    cPt2.y = LONG(dRadius*sin(dAngCenSin + M_PI_2) + cPtCen.y);
    cPt4.x = LONG(dRadius*cos(dAngCenCos + M_PI_2 * 3) + cPtCen.x);
    cPt4.y = LONG(dRadius*sin(dAngCenSin + M_PI_2 * 3) + cPtCen.y);
    dDistance = sqrt(pow((cPt1.x - cPt4.x), 2) + pow((cPt1.y - cPt4.y), 2));
    m_ptVec.push_back(cPt1);
    m_ptVec.push_back(cPt2);
    m_ptVec.push_back(cPt3);
    m_ptVec.push_back(cPt4);

    dAngCos = acos((cPt1.x - cPt4.x) / dDistance);
    dAngSin = asin((cPt1.y - cPt4.y) / dDistance);
    if(dAngCos < 0)
    {
        dAngCos += M_PI * 2;
    }
    if(dAngSin < 0)
    {
        dAngSin += M_PI * 2;
    }
    dAngCos2 = M_PI * 2 - dAngCos;
    dAngSin2 = M_PI - dAngSin;
    if(abs(dAngCos - dAngSin) > 0.01)
    {
        if(abs(dAngCos - dAngSin2) < 0.01)
        {
            dAngSin = dAngSin2;
        }
        else if(abs(dAngCos2 - dAngSin) < 0.01)
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
    while(1)
    {
        dRadius = dRadius - dWidth*sqrt(2);
        dDistance = dDistance - dWidth;
        if(dDistance < dWidth)
        {
            break;
        }
        cPt1.x = LONG(dDistance*cos(dAngCos) + cPt4.x);
        cPt1.y = LONG(dDistance*sin(dAngSin) + cPt4.y);
        m_ptVec.push_back(cPt1);
        cPt2.x = LONG(dRadius*cos(dAngCenCos + M_PI_2) + cPtCen.x);
        cPt2.y = LONG(dRadius*sin(dAngCenSin + M_PI_2) + cPtCen.y);
        m_ptVec.push_back(cPt2);
        dDistance = dDistance - dWidth;
        if(dDistance < dWidth)
        {
            break;
        }
        cPt3.x = LONG(dRadius*cos(dAngCenCos + M_PI) + cPtCen.x);
        cPt3.y = LONG(dRadius*sin(dAngCenSin + M_PI) + cPtCen.y);
        m_ptVec.push_back(cPt3);
        cPt4.x = LONG(dRadius*cos(dAngCenCos + M_PI_2 * 3) + cPtCen.x);
        cPt4.y = LONG(dRadius*sin(dAngCenSin + M_PI_2 * 3) + cPtCen.y);
        m_ptVec.push_back(cPt4);
    }
    #pragma endregion

    #pragma region ****�u�q��]�m****
    //�ϥ�(1)���ʫe����(lStartDelayTime)
    if(lStartDelayTime>0)
    {
        if(!m_bIsStop)
        {
            rptIter = m_ptVec.rbegin();
            MO_Do3DLineMove(rptIter->x - MO_ReadLogicPosition(0), rptIter->y - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration,
                            lInitVelociy);//�^��_�l�I!
            PreventMoveError();
        }
        if(!m_bIsStop)
        {
            MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//�^��_�l�I!
            PreventMoveError();
        }
        PauseDoGlue();//�Ȱ��^�_�᭫�s�(Ū���Ȱ��ѼơA��ѼƬ�0�ɥX���A�B�I�����n���}�C)
        if(!m_bIsStop && m_bIsDispend == 1)
        {
            MO_GummingSet();//�(���d)
        }
        MO_Timer(0, 0, lStartDelayTime * 1000);
        MO_Timer(1, 0, lStartDelayTime * 1000);//�u�q�I���]�w---(1)���ʫe����(�b�u�q�}�l�I�W)

        Sleep(1);//����X���A�קK�p�ɾ��쪽��0
        while(MO_Timer(3, 0, 0))
        {
            if(m_bIsStop)
            {
                break;
            }
            Sleep(1);
        }
    }
    //�ϥ�(2)�p��X���Z���A�]�w�X���I
    else if(lStartDistance>0)
    {
        rptIter = m_ptVec.rbegin();
        CPoint rPt1 = *rptIter;
        CPoint rPt2 = *(rptIter + 1);
        CPoint ptSetDist(0, 0);
        LONG glueDist = lStartDistance;
        DOUBLE dLength = sqrt(pow(rPt1.x - rPt2.x, 2) + pow(rPt1.y - rPt2.y, 2));
        if(rPt1.x == rPt2.x)
        {
            ptSetDist = rPt1;
            glueDist = LONG(glueDist*M_SQRT1_2);
            ptSetDist.y = (rPt1.y<rPt2.y) ? rPt1.y - glueDist : rPt1.y + glueDist;
        }
        else if(rPt1.y == rPt2.y)
        {
            ptSetDist = rPt1;
            glueDist = LONG(glueDist*M_SQRT1_2);
            ptSetDist.x = (rPt1.x<rPt2.x) ? rPt1.x - glueDist : rPt1.x + glueDist;
        }
        else
        {
            CPoint ptDist(0, 0);
            ptDist.x = (LONG)round(glueDist*abs(rPt1.x - rPt2.x) / dLength);
            ptDist.y = (LONG)round(glueDist*abs(rPt1.y - rPt2.y) / dLength);

            ptSetDist.x = (rPt1.x<rPt2.x) ? rPt1.x - ptDist.x : rPt1.x + ptDist.x;
            ptSetDist.y = (rPt1.y<rPt2.y) ? rPt1.y - ptDist.y : rPt1.y + ptDist.y;
        }
        //�h���̫�@�I�A���J�]�m�Z�����y�Ь��̫�@�I(�ϦV�Y���Ĥ@�I)
        m_ptVec.pop_back();
        m_ptVec.push_back(ptSetDist);
    }
    else
    {
    }

    std::vector<DATA_2MOVE> DATA_2DO;
    DATA_2DO.clear();
    for(rptIter = m_ptVec.rbegin(); rptIter != m_ptVec.rend(); rptIter++)
    {
        MCO_Do2dDataLine((*rptIter).x, (*rptIter).y, DATA_2DO);
    }

    LA_AbsToOppo2Move(DATA_2DO);
    for(UINT i = 1; i < DATA_2DO.size(); i++)
    {
        DATA_2Do[i - 1] = DATA_2DO.at(i);
    }
    if(!m_bIsStop)
    {
        MO_Do3DLineMove(DATA_2DO.at(0).EndP.x, DATA_2DO.at(0).EndP.y, 0, lWorkVelociy,
                        lAcceleration, lInitVelociy);//����
        PreventMoveError();
    }
    if(!m_bIsStop)
    {
        MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//Z�b���U
        PreventMoveError();//����b�d�X��
    }
    //�ϥ�(2)�]�m�Z��
    if(lStartDistance>0)
    {
        //timeUpGlue ���us
        LONG timeUpGlue = CalPreglue(lStartDistance, lWorkVelociy, lAcceleration, lInitVelociy);
        if(!m_bIsStop)
        {
            m_YtimeOutGlueSet = TRUE;
            MO_TimerSetIntter(timeUpGlue, 0);//�ϥ�Y timer���_ �X��
        }
    }
    //�ϥ�(5)�����Z��(lCloseDistance)
    if(lCloseDistance>0)
    {
        LONG sumPath = 0;
        LONG finishTime = 0;
        DOUBLE avgTime = 0;
        LONG accLength = CalPreglue(lWorkVelociy, lAcceleration, lInitVelociy);
        for(UINT i = 1; i<DATA_2DO.size(); i++)
        {
            sumPath += DATA_2DO.at(i).Distance;
        }
        avgTime = ((DOUBLE)sumPath - (DOUBLE)accLength) / (DOUBLE)lWorkVelociy;
        finishTime = (LONG)round(avgTime * 1000000) + CalPreglueTime(lWorkVelociy, lAcceleration, lInitVelociy);
        LONG closeDistTime = CalPreglue(lCloseDistance, lWorkVelociy, 0, lInitVelociy);
        if(!m_bIsStop)
        {
            CAction::m_ZtimeOutGlueSet = FALSE;
            MO_TimerSetIntter(finishTime - closeDistTime, 1);
        }
    }

    if(lStartDelayTime == 0 && lStartDistance == 0)
    {
        PauseDoGlue();//�Ȱ���_���~��X��(m_bIsPause=0)�X��
    }
    MO_DO2Curve(DATA_2Do, DATA_2DO.size() - 1, lWorkVelociy);
    PreventMoveError();//�����X�ʿ��~
    Sleep(200);
    DATA_2DO.clear();
    //�ϥ�(3)���d�ɶ�(lCloseOffDelayTime)
    if(lCloseOffDelayTime>0)
    {
        MO_Timer(0, 0, lCloseOffDelayTime * 1000);
        MO_Timer(1, 0, lCloseOffDelayTime * 1000);//�u�q�I���]�w---(3)���d�ɶ�
        Sleep(1);//����X���A�קK�p�ɾ��쪽��0
        while(MO_Timer(3, 0, 0))
        {
            if(m_bIsStop == 1)
            {
                break;
            }
            Sleep(1);
        }
    }
    PauseStopGlue();//�Ȱ��ɰ����(m_bIsPause=1)
    MO_StopGumming();//����X��
    //�ϥ�(6)��������(lCloseONDelayTime)
    if(!(lCloseDistance>0) && lCloseONDelayTime>0)
    {
        MO_Timer(0, 0, lCloseONDelayTime * 1000);
        MO_Timer(1, 0, lCloseONDelayTime * 1000);//�u�q�I���]�w---(6)��������
        Sleep(1);//����X���A�קK�p�ɾ��쪽��0
        while(MO_Timer(3, 0, 0))
        {
            if(m_bIsStop == 1)
            {
                break;
            }
            Sleep(1);
        }
    }
    if(!m_bIsStop)
    {
        //*************************��R�^��z�b�t�ץثe���X�ʳt�ת��⭿******************
        MO_Do3DLineMove(0, 0, (lZ - lZBackDistance) - lZ, lWorkVelociy * 2, lAcceleration,
                        lInitVelociy);//Z�b��^
        PreventMoveError();//����b�d�X��
    }
#endif
}
/*����---��R�κA(���A7������۶�R.�Ѥ��ӥ~)
*��J(�_�l�Ix1,y1,�����Ix2,y2,�e��,�X�ʳt��,�[�t��,��t��)
*/
void CAction::AttachFillType7(LONG lX1, LONG lY1, LONG lCenX, LONG lCenY,
                              LONG lZ, LONG lZBackDistance, LONG lWidth,
                              LONG lStartDelayTime, LONG lStartDistance, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime,
                              LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
    /*�u�q�I���]�w(1.���ʫe����A2.�]�m�Z���A3���d�ɶ��A5�����Z���A6��������)
    LONG lStartDelayTime ,LONG lStartDistance ,LONG lCloseOffDelayTime ,LONG lCloseDistance ,LONG lCloseONDelayTime
    //1.���ʫe�I�����b�@���u�q�_�l�I�B�O�����}���ɪ��C �����ɥi����w�Y�b�y��y�ʤ��e�u�u�q�o�Ͳ��ʡC
    //2. �I�����}�ҫe�A ���F���}���u�u�q�_�l�I�����ʶZ���C �ӶZ�������F���ѤF�������_�t�ɶ��A�D�n�ΨӮ����L�q�y��b�u�q�_�l�B���n�E�C
    //3.�I����������A���F�����O�b�w�Y���ܤU�@�I�e�ܱo���ŦӦb�u�q�I�������I�B���ͪ����ɡC
    //5.������L�q�y��b�u�q�����I�B�o�Ͱ�n�A�I�����b�Z���u�q�����I�e�h���B�����C
    //6.�I�����b�u�q�����I�B�����O���}�Ҫ��ɪ��C
    */
#ifdef MOVE
    #pragma region ****�ꫬ���ۥ\��****
    DOUBLE dRadius = 0, dWidth = 0, dAng0 = 0, dAng1 = 0, dAng2 = 0;
    BOOL bRev = 0;//0�f��/1����
    DOUBLE dSumPath = 0, dCloseTime = 0;//�`���|��.�����ɶ�
    LONG lLineClose = 0, lXClose = 0, lYClose = 0, lDistance = 0;
    LONG lNowX = 0, lNowY = 0, lNowZ = 0, lTime = 0;
    CPoint cPt1 = 0, cPt2 = 0, cPt3 = 0, cPt4 = 0, cPtCen1 = 0, cPtCen2 = 0;
    int iData = 0, iOdd = 0, icnt = 0;//�P�_�_��(�_���W�b��/�����U�b��)
    CString csbuff = 0;
    std::vector<CPoint>m_ptVec;
    std::vector<CPoint>::reverse_iterator rptIter;//�ϦV���N��
    m_ptVec.clear();
    cPt1.x = lX1;
    cPt1.y = lY1;
    cPtCen1.x = lCenX;
    cPtCen1.y = lCenY;//�W�b����
    dRadius = sqrt(pow(cPtCen1.x - cPt1.x, 2) + pow(cPtCen1.y - cPt1.y, 2));//�b�|
    if(LONG(dRadius) == 0)
    {
        return;
    }
    lDistance = LONG(dRadius);
    dWidth = lWidth * 1000;
    dAng1 = acos((cPt1.x - cPtCen1.x) / dRadius);
    dAng2 = asin((cPt1.y - cPtCen1.y) / dRadius);
    //cPt2.x = LONG(dRadius*cos(dAng1 + M_PI) + cPtCen1.x);
    //cPt2.y = LONG(dRadius*sin(dAng2 + M_PI) + cPtCen1.y);
    cPt2.x = (LONG)((cPt1.x - cPtCen1.x)*cos(M_PI) - (cPt1.y - cPtCen1.y)*sin(M_PI) + cPtCen1.x);
    cPt2.y = (LONG)((cPt1.x - cPtCen1.x)*sin(M_PI) + (cPt1.y - cPtCen1.y)*cos(M_PI) + cPtCen1.y);
    dSumPath = M_PI*dRadius;//���ʪ��`����
    m_ptVec.push_back(cPt1);
    m_ptVec.push_back(cPt2);
    lDistance = LONG(lDistance - dWidth);
    LineGetToPoint(lXClose, lYClose, cPt1.x, cPt1.y, cPtCen1.x, cPtCen1.y,
                   lDistance);
    cPt3.x = lXClose;
    cPt3.y = lYClose;
    cPtCen2.x = LONG((cPt3.x + cPt2.x) / 2.0);
    cPtCen2.y = LONG((cPt3.y + cPt2.y) / 2.0);//�U�b����
    iData = (int)dRadius % (int)dWidth;
    lDistance = LONG(lDistance + dWidth);
    while(1)
    {
        lDistance = LONG(lDistance - dWidth);
        if(lDistance < dWidth)
        {
            dSumPath -= M_PI / 2 * (sqrt(pow(cPt3.x - cPt4.x, 2) + pow(cPt3.y - cPt4.y, 2)));
            dSumPath += sqrt(pow(cPt3.x - cPt4.x, 2) + pow(cPt3.y - cPt4.y, 2));
            break;
        }
        LineGetToPoint(lXClose, lYClose, cPt1.x, cPt1.y, cPtCen1.x, cPtCen1.y,
                       lDistance);
        cPt3.x = lXClose;
        cPt3.y = lYClose;
        if(icnt == 0)
        {
            dSumPath += M_PI / 2 * (sqrt(pow(cPt2.x - cPt3.x, 2) + pow(cPt2.y - cPt3.y, 2)));//���ʪ��`����
        }
        else
        {
            dSumPath += M_PI / 2 * (sqrt(pow(cPt4.x - cPt3.x, 2) + pow(cPt4.y - cPt3.y, 2)));//���ʪ��`����
        }
        m_ptVec.push_back(cPt3);
        cPt4.x = (LONG)((cPt3.x - cPtCen1.x)*cos(M_PI) - (cPt3.y - cPtCen1.y)*sin(M_PI) + cPtCen1.x);
        cPt4.y = (LONG)((cPt3.x - cPtCen1.x)*sin(M_PI) + (cPt3.y - cPtCen1.y)*cos(M_PI) + cPtCen1.y);
        dSumPath += M_PI / 2 * (sqrt(pow(cPt3.x - cPt4.x, 2) + pow(cPt3.y - cPt4.y, 2)));//���ʪ��`����
        m_ptVec.push_back(cPt4);
        icnt++;
    }
    #pragma endregion
    if(lStartDistance>0) //�ϥ�--(2)�]�m�Z��(lStartDistance)
    {
        LONG lStartX = 0, lStartY = 0;
        LineGetToPoint(lStartX, lStartY, lX1, lY1, lCenX, lCenY, lStartDistance);//��X�_�l�I
        if(!m_bIsStop)
        {
            MO_Do3DLineMove(lStartX - MO_ReadLogicPosition(0), lStartY - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration,
                            lInitVelociy);//�^��_�l�I!
            PreventMoveError();
        }
        if(!m_bIsStop)
        {
            MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//�^��_�l�I!
            PreventMoveError();
        }
        lTime = CalPreglue(lStartDistance, lWorkVelociy, lAcceleration, lInitVelociy);
#ifdef PRINTF
        //_cwprintf(_T("�p�ɾ��]�m�Z�����ɶ�=%lf \n"), DOUBLE(lTime / 1000000.0));
#endif
        /*======�p�ɾ���Ĳ�o���_����X���A�ϥ�y���_����================*/
        if(!m_bIsStop)
        {
            CAction::m_YtimeOutGlueSet = TRUE;
            MO_TimerSetIntter(lTime, 0);//�p�ɨ���ܰ����
        }
    }
    else
    {
        //����ɡA���ʨ줤���I�b�U��
        if(!m_bIsStop)
        {
            MO_Do3DLineMove(lCenX - MO_ReadLogicPosition(0), lCenY - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration,
                            lInitVelociy);//���u���ʦܶ��
            PreventMoveError();
        }
        if(!m_bIsStop)
        {
            MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration,
                            lInitVelociy);//Z�b���U
            PreventMoveError();//����b�d�X��
        }
        PauseDoGlue();//�Ȱ��^�_�᭫�s�(Ū���Ȱ��ѼơA��ѼƬ�0�ɥX���A�B�I�����n���}�C)
        if(!m_bIsStop && m_bIsDispend == 1)
        {
            MO_GummingSet();//�(���d)
        }
        if(lStartDelayTime > 0) //�ϥ�(1)���ʫe����(lStartDelayTime)
        {
            MO_Timer(0, 0, lStartDelayTime * 1000);
            MO_Timer(1, 0, lStartDelayTime * 1000);//�u�q�I���]�w---(1)���ʫe����(�b�u�q�}�l�I�W)
            Sleep(1);//����X���A�קK�p�ɾ��쪽��0
            while(MO_Timer(3, 0, 0))
            {
                if(m_bIsStop)
                {
                    break;
                }
                Sleep(1);
            }
        }
    }
    if(lCloseDistance > 0) //�ϥ� --(5)�����Z��
    {
        dCloseTime = 1000000 * ((dSumPath - lCloseDistance) / (DOUBLE)lWorkVelociy);
        /*======�p�ɾ���Ĳ�o���_�����_���A�ϥ�z���_����================*/
        if(!m_bIsStop)
        {
            CAction::m_ZtimeOutGlueSet = FALSE;
            if(lStartDistance > 0)
            {
#ifdef PRINTF
           //     _cwprintf(_T("End�����Z�����ɶ�=%lf \n"), DOUBLE(lTime) + DOUBLE(dCloseTime / 1000000.0));
#endif
                MO_TimerSetIntter(lTime + LONG(dCloseTime), 1);//�p�ɨ���ܰ����
            }
            else
            {
#ifdef PRINTF
            //    _cwprintf(_T("End�����Z�����ɶ�=%lf \n"), DOUBLE(dCloseTime / 1000000.0));
#endif
                MO_TimerSetIntter(LONG(dCloseTime), 1);//�p�ɨ���ܰ����
            }
        }
    }
    std::vector<DATA_2MOVE> DATA_2DO;
    DATA_2DO.clear();
    for(rptIter = m_ptVec.rbegin(); rptIter != m_ptVec.rend(); rptIter++)
    {
        if((iData != 0) && (iOdd == 0))
        {
            MCO_Do2dDataLine((*rptIter).x, (*rptIter).y, DATA_2DO);
        }
        else if((iData == 0) && (iOdd == 0))
        {
            MCO_Do2dDataCir((*rptIter).x, (*rptIter).y, cPtCen2.x, cPtCen2.y, bRev, DATA_2DO);
        }
        else
        {
            if(iOdd % 2 == 0)
            {
                MCO_Do2dDataCir((*rptIter).x, (*rptIter).y, cPtCen2.x, cPtCen2.y, bRev, DATA_2DO);
            }
            else
            {
                MCO_Do2dDataCir((*rptIter).x, (*rptIter).y, lCenX, lCenY, bRev, DATA_2DO);
            }
        }
        iOdd++;
    }
    LA_AbsToOppo2Move(DATA_2DO);
    for(UINT i = 0; i < DATA_2DO.size(); i++)
    {
        DATA_2Do[i] = DATA_2DO.at(i);
    }
    MO_DO2Curve(DATA_2Do, DATA_2DO.size(), lWorkVelociy);
    PreventMoveError();//�����X�ʿ��~
    Sleep(200);
    DATA_2DO.clear();

    MO_Timer(0, 0, lCloseONDelayTime * 1000);
    MO_Timer(1, 0, lCloseONDelayTime * 1000);//�u�q�I���]�w---(6)��������
    Sleep(1);//����X���A�קK�p�ɾ��쪽��0
    while(MO_Timer(3, 0, 0))
    {
        if(m_bIsStop == 1)
        {
            break;
        }
        Sleep(1);
    }
    MO_StopGumming();//����X��
    if(!m_bIsStop)
    {
        //*************************��R�^��z�b�t�ץثe���X�ʳt�ת��⭿******************
        MO_Do3DLineMove(0, 0, (lZ - lZBackDistance) - lZ, lWorkVelociy * 2, lAcceleration,
                        lInitVelociy);//Z�b��^
        PreventMoveError();//����b�d�X��
    }
    MO_Timer(0, 0, lCloseOffDelayTime * 1000);
    MO_Timer(1, 0, lCloseOffDelayTime * 1000);//�u�q�I���]�w---(3)���d�ɶ�
    Sleep(1);//����X���A�קK�p�ɾ��쪽��0
    while(MO_Timer(3, 0, 0))
    {
        if(m_bIsStop == 1)
        {
            break;
        }
        Sleep(1);
    }
#endif // MOVE
}
/*����---��R�κA(���A3�x�ζ�R.�ѥ~�Ӥ�)
*��J(�_�l�Ix1,y1,�����Ix2,y2,�e��,�X�ʳt��,�[�t��,��t��)
*/
void CAction::AttachFillType3_End(LONG &EndX, LONG &EndY, LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lWidth, LONG lWidth2)
{
#ifdef MOVE
    #pragma region ****�Ƥ諬���Y4�I�\��****
    LONG lNowX = 0, lNowY = 0;
    CPoint cPt1 = 0, cPt2 = 0, cPt3 = 0, cPt4 = 0, cPtCen = 0;
    DOUBLE dRadius = 0, dDistance = 0, dWidth = 0, dAngCenCos = 0, dAngCenSin = 0, dAngCos = 0,
           dAngSin = 0;
    DOUBLE dAngCenCos2 = 0, dAngCenSin2 = 0, dAngCos2 = 0, dAngSin2 = 0;
    std::vector<CPoint>::iterator ptIter;//���N��
    std::vector<CPoint> m_ptVec;
    m_ptVec.clear();
    cPt1.x = lX1;
    cPt1.y = lY1;
    cPt3.x = lX2;
    cPt3.y = lY2;
    dWidth = lWidth * 1000;
    cPtCen.x = cPt1.x + (cPt3.x - cPt1.x) / 2;
    cPtCen.y = cPt1.y + (cPt3.y - cPt1.y) / 2;
    dRadius = sqrt(pow(cPt1.x - cPtCen.x, 2) + pow(cPt1.y - cPtCen.y, 2));
    if(dRadius == 0)
    {
        return;
    }
    dAngCenCos = acos(DOUBLE(cPt1.x - cPtCen.x) / dRadius);
    dAngCenSin = asin(DOUBLE(cPt1.y - cPtCen.y) / dRadius);
    dAngCenCos2 = M_PI * 2 - dAngCenCos;
    dAngCenSin2 = M_PI - dAngCenSin;
    if(abs(dAngCenCos - dAngCenSin) > 0.01)
    {
        if(abs(dAngCenCos - dAngCenSin2) < 0.01)
        {
            dAngCenSin = dAngCenSin2;
        }
        else if(abs(dAngCenCos2 - dAngCenSin) < 0.01)
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
    cPt2.x = LONG(dRadius*cos(dAngCenCos + M_PI_2) + cPtCen.x);
    cPt2.y = LONG(dRadius*sin(dAngCenSin + M_PI_2) + cPtCen.y);
    cPt4.x = LONG(dRadius*cos(dAngCenCos + M_PI_2 * 3) + cPtCen.x);
    cPt4.y = LONG(dRadius*sin(dAngCenSin + M_PI_2 * 3) + cPtCen.y);
    dDistance = sqrt(pow((cPt1.x - cPt4.x), 2) + pow((cPt1.y - cPt4.y), 2));
    m_ptVec.push_back(cPt1);
    m_ptVec.push_back(cPt2);
    m_ptVec.push_back(cPt3);
    m_ptVec.push_back(cPt4);
    dAngCos = acos((cPt1.x - cPt4.x) / dDistance);
    dAngSin = asin((cPt1.y - cPt4.y) / dDistance);
    if(dAngCos < 0)
    {
        dAngCos += M_PI * 2;
    }
    if(dAngSin < 0)
    {
        dAngSin += M_PI * 2;
    }
    dAngCos2 = M_PI * 2 - dAngCos;
    dAngSin2 = M_PI - dAngSin;
    if(abs(dAngCos - dAngSin) > 0.01)
    {
        if(abs(dAngCos - dAngSin2) < 0.01)
        {
            dAngSin = dAngSin2;
        }
        else if(abs(dAngCos2 - dAngSin) < 0.01)
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
    while(1)
    {
        dRadius = dRadius - dWidth*sqrt(2);
        dDistance = dDistance - dWidth;
        if(dDistance < dWidth)
        {
            break;
        }
        cPt1.x = LONG(dDistance*cos(dAngCos) + cPt4.x);
        cPt1.y = LONG(dDistance*sin(dAngSin) + cPt4.y);
        m_ptVec.push_back(cPt1);
        cPt2.x = LONG(dRadius*cos(dAngCenCos + M_PI_2) + cPtCen.x);
        cPt2.y = LONG(dRadius*sin(dAngCenSin + M_PI_2) + cPtCen.y);
        m_ptVec.push_back(cPt2);
        dDistance = dDistance - dWidth;
        if(dDistance < dWidth)
        {
            break;
        }
        cPt3.x = LONG(dRadius*cos(dAngCenCos + M_PI) + cPtCen.x);
        cPt3.y = LONG(dRadius*sin(dAngCenSin + M_PI) + cPtCen.y);
        m_ptVec.push_back(cPt3);
        cPt4.x = LONG(dRadius*cos(dAngCenCos + M_PI_2 * 3) + cPtCen.x);
        cPt4.y = LONG(dRadius*sin(dAngCenSin + M_PI_2 * 3) + cPtCen.y);
        m_ptVec.push_back(cPt4);
    }
    #pragma endregion
    std::vector<DATA_2MOVE>DATA_2DO;
    DATA_2DO.clear();
    for(ptIter = m_ptVec.begin(); ptIter != m_ptVec.end(); ptIter++)
    {
        MCO_Do2dDataLine((*ptIter).x, (*ptIter).y, DATA_2DO);
    }
    EndX = DATA_2DO.back().EndP.x;
    EndY = DATA_2DO.back().EndP.y;
    Sleep(1);
    DATA_2DO.clear();
#endif
}
/*����---��R�κA(���A4�x����)
*��J(�_�l�Ix1,y1,�����Ix2,y2,�e��,��ݼe��,�X�ʳt��,�[�t��,��t��)
*/
void CAction::AttachFillType4_End(LONG &EndX, LONG &EndY, LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lWidth, LONG lWidth2)
{
#ifdef MOVE
    #pragma region ****�Ƥ諬���Y4�I�\��****
    LONG lNowX = 0, lNowY = 0;
    CPoint cPt1 = 0, cPt2 = 0, cPt3 = 0, cPt4 = 0, cPtCen = 0;
    DOUBLE dRadius = 0, dDistance = 0, dWidth = 0, dWidth2 = 0, dAngCenCos = 0,
           dAngCenSin = 0, dAngCos = 0, dAngSin = 0;
    DOUBLE dAngCenCos2 = 0, dAngCenSin2 = 0, dAngCos2 = 0, dAngSin2 = 0;
    int iBuff = 0;//�P�_��ݼe�ץ�
    std::vector<CPoint>::iterator ptIter;//���N��
    std::vector<CPoint> m_ptVec;
    m_ptVec.clear();
    cPt1.x = lX1;
    cPt1.y = lY1;
    cPt3.x = lX2;
    cPt3.y = lY2;
    dWidth = lWidth * 1000;
    dWidth2 = lWidth2 * 1000;
    cPtCen.x = cPt1.x + (cPt3.x - cPt1.x) / 2;
    cPtCen.y = cPt1.y + (cPt3.y - cPt1.y) / 2;
    dRadius = sqrt(pow(cPt1.x - cPtCen.x, 2) + pow(cPt1.y - cPtCen.y, 2));
    if(dRadius == 0)
    {
        return;
    }
    dAngCenCos = acos(DOUBLE(cPt1.x - cPtCen.x) / dRadius);
    dAngCenSin = asin(DOUBLE(cPt1.y - cPtCen.y) / dRadius);
    dAngCenCos2 = M_PI * 2 - dAngCenCos;
    dAngCenSin2 = M_PI - dAngCenSin;
    if(abs(dAngCenCos - dAngCenSin) > 0.01)
    {
        if(abs(dAngCenCos - dAngCenSin2) < 0.01)
        {
            dAngCenSin = dAngCenSin2;
        }
        else if(abs(dAngCenCos2 - dAngCenSin) < 0.01)
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
    cPt2.x = LONG(dRadius*cos(dAngCenCos + M_PI_2) + cPtCen.x);
    cPt2.y = LONG(dRadius*sin(dAngCenSin + M_PI_2) + cPtCen.y);
    cPt4.x = LONG(dRadius*cos(dAngCenCos + M_PI_2 * 3) + cPtCen.x);
    cPt4.y = LONG(dRadius*sin(dAngCenSin + M_PI_2 * 3) + cPtCen.y);
    dDistance = sqrt(pow((cPt1.x - cPt4.x), 2) + pow((cPt1.y - cPt4.y), 2));
    m_ptVec.push_back(cPt1);
    m_ptVec.push_back(cPt2);
    m_ptVec.push_back(cPt3);
    m_ptVec.push_back(cPt4);

    dAngCos = acos((cPt1.x - cPt4.x) / dDistance);
    dAngSin = asin((cPt1.y - cPt4.y) / dDistance);
    if(dAngCos < 0)
    {
        dAngCos += M_PI * 2;
    }
    if(dAngSin < 0)
    {
        dAngSin += M_PI * 2;
    }
    dAngCos2 = M_PI * 2 - dAngCos;
    dAngSin2 = M_PI - dAngSin;
    if(abs(dAngCos - dAngSin) > 0.01)
    {
        if(abs(dAngCos - dAngSin2) < 0.01)
        {
            dAngSin = dAngSin2;
        }
        else if(abs(dAngCos2 - dAngSin) < 0.01)
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
    while(1)
    {
        if((iBuff == 1) && (dRadius - (2 * dWidth2) < 0))
        {
#ifdef PRINTF
         //   _cwprintf(_T("��ݼe�׹L�j \n"));
#endif
            m_ptVec.push_back(cPt1);
            break;
        }
        dRadius = dRadius - dWidth*sqrt(2);
        dDistance = dDistance - dWidth;
        if(iBuff*dWidth >  dWidth2)   //dWidth2��ݼe��
        {
            dDistance += dWidth;
            cPt1.x = LONG(dDistance*cos(dAngCos) + cPt4.x);
            cPt1.y = LONG(dDistance*sin(dAngSin) + cPt4.y);
            m_ptVec.push_back(cPt1);
            break;
        }
        if(dDistance < dWidth)
        {
            break;
        }
        cPt1.x = LONG(dDistance*cos(dAngCos) + cPt4.x);
        cPt1.y = LONG(dDistance*sin(dAngSin) + cPt4.y);
        m_ptVec.push_back(cPt1);
        cPt2.x = LONG(dRadius*cos(dAngCenCos + M_PI_2) + cPtCen.x);
        cPt2.y = LONG(dRadius*sin(dAngCenSin + M_PI_2) + cPtCen.y);
        m_ptVec.push_back(cPt2);
        dDistance = dDistance - dWidth;
        if(dDistance < dWidth)
        {
            break;
        }
        cPt3.x = LONG(dRadius*cos(dAngCenCos + M_PI) + cPtCen.x);
        cPt3.y = LONG(dRadius*sin(dAngCenSin + M_PI) + cPtCen.y);
        m_ptVec.push_back(cPt3);
        cPt4.x = LONG(dRadius*cos(dAngCenCos + M_PI_2 * 3) + cPtCen.x);
        cPt4.y = LONG(dRadius*sin(dAngCenSin + M_PI_2 * 3) + cPtCen.y);
        m_ptVec.push_back(cPt4);
        iBuff++;
    }
    #pragma endregion
    std::vector<DATA_2MOVE> DATA_2DO;
    DATA_2DO.clear();
    for(ptIter = m_ptVec.begin(); ptIter != m_ptVec.end(); ptIter++)
    {
        MCO_Do2dDataLine((*ptIter).x, (*ptIter).y, DATA_2DO);
    }
    EndX = DATA_2DO.back().EndP.x;
    EndY = DATA_2DO.back().EndP.y;
    Sleep(1);
    DATA_2DO.clear();
#endif
}
/*����---��R�κA(���A5����)
*��J(�_�l�Ix1,y1,�����Ix2,y2,�e��,��ݼe��,�X�ʳt��,�[�t��,��t��)
*/
void CAction::AttachFillType5_End(LONG &EndX, LONG &EndY, LONG lX1, LONG lY1, LONG lCenX, LONG lCenY, LONG lWidth, LONG lWidth2)
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
    if(dRadius == 0)
    {
        return;
    }
    lDistance = LONG(dRadius);
    dWidth = lWidth * 1000;
    dWidth2 = lWidth2 * 1000;
    dAng1 = acos((cPt1.x - cPtCen1.x) / dRadius);
    dAng2 = asin((cPt1.y - cPtCen1.y) / dRadius);
    cPt2.x = LONG(dRadius*cos(dAng1 + M_PI) + cPtCen1.x);
    cPt2.y = LONG(dRadius*sin(dAng2 + M_PI) + cPtCen1.y);
    m_ptVec.push_back(cPt1);
    m_ptVec.push_back(cPt2);
    lDistance = LONG(lDistance - dWidth);
    LineGetToPoint(lXClose, lYClose, cPt1.x, cPt1.y, cPtCen1.x, cPtCen1.y,
                   lDistance);
    cPt3.x = lXClose;
    cPt3.y = lYClose;
    cPtCen2.x = (cPt3.x + cPt2.x) / 2;
    cPtCen2.y = (cPt3.y + cPt2.y) / 2;//�U�b����
    lDistance = LONG(lDistance + dWidth);
    iData = (int)dRadius % (int)dWidth;
    iBuff = 1;
    while(1)
    {
        if((iBuff == 1) && (lDistance - (2 * dWidth2) < 0))
        {
#ifdef PRINTF
        //    _cwprintf(_T("��ݼe�׹L�j \n"));
#endif
            break;
        }
        dRadius = dRadius - dWidth;
        lDistance = LONG(lDistance - dWidth);
        if(iBuff * dWidth > dWidth2)
        {
            lDistance += LONG(dWidth);
            LineGetToPoint(lXClose, lYClose, cPt1.x, cPt1.y, cPtCen1.x, cPtCen1.y,
                           lDistance);
            cPt3.x = lXClose;
            cPt3.y = lYClose;
            m_ptVec.push_back(cPt3);
            break;
        }
        LineGetToPoint(lXClose, lYClose, cPt1.x, cPt1.y, cPtCen1.x, cPtCen1.y,
                       lDistance);
        cPt3.x = lXClose;
        cPt3.y = lYClose;
        m_ptVec.push_back(cPt3);
        cPt4.x = LONG(dRadius*cos(dAng1 + M_PI) + cPtCen1.x);
        cPt4.y = LONG(lDistance*sin(dAng2 + M_PI) + cPtCen1.y);
        m_ptVec.push_back(cPt4);
        iBuff++;
    }
    #pragma endregion
    std::vector<DATA_2MOVE> DATA_2DO;
    DATA_2DO.clear();
    for(UINT i = 1; i < m_ptVec.size(); i++)
    {
        if(i == m_ptVec.size() - 1)
        {
            if(i % 2 == 0)
            {
                MCO_Do2dDataCir(m_ptVec.at(i).x, m_ptVec.at(i).y, cPtCen1.x, cPtCen1.y, bRev, DATA_2DO);//�W�b��
            }
            else
            {
                MCO_Do2dDataCir(m_ptVec.at(i).x, m_ptVec.at(i).y, cPtCen2.x, cPtCen2.y, bRev, DATA_2DO);//�U�b��
            }
        }
        else
        {
            if(i % 2 != 0)
            {
                MCO_Do2dDataCir(m_ptVec.at(i).x, m_ptVec.at(i).y, cPtCen1.x, cPtCen1.y, bRev, DATA_2DO);//�W�b��
            }
            else
            {
                MCO_Do2dDataCir(m_ptVec.at(i).x, m_ptVec.at(i).y, cPtCen2.x, cPtCen2.y, bRev, DATA_2DO);//�U�b��
            }
        }
    }
    EndX = DATA_2DO.back().EndP.x;
    EndY = DATA_2DO.back().EndP.y;
    Sleep(1);
    DATA_2DO.clear();
#endif
}
/***********************************************************
**                                                        **
**          �B�ʼҲ�-�s��t��.                             **
**                                                        **
************************************************************/

#ifdef MOVE
//��R�Ψ�b�s��t��(����--���u)
void CAction::MCO_Do2dDataLine(LONG EndPX, LONG EndPY, std::vector<DATA_2MOVE> &str)
{
    DATA_2MOVE DATA_2D;
    DATA_2D.EndP.x = EndPX;
    DATA_2D.EndP.y = EndPY;
    DATA_2D.Type = 0;//���u
    DATA_2D.Speed = 0;
    DATA_2D.CirCentP.x = 0;
    DATA_2D.CirCentP.y = 0;
    DATA_2D.CirRev = 0;
    str.push_back(DATA_2D);
}
#endif
//��R�Ψ�b�s��t��(����--��)
#ifdef MOVE
void CAction::MCO_Do2dDataCir(LONG EndPX, LONG EndPY, LONG CenX, LONG CenY, BOOL bRev, std::vector<DATA_2MOVE> &str)
{
    DATA_2MOVE DATA_2D;
    DATA_2D.Type = 1;//��
    DATA_2D.EndP.x = EndPX;
    DATA_2D.EndP.y = EndPY;
    DATA_2D.CirCentP.x = CenX;
    DATA_2D.CirCentP.y = CenY;
    DATA_2D.CirRev = bRev;
    str.push_back(DATA_2D);
}
#endif
/***********************************************************
**                                                        **
**          �B�ʼҲ�-w�b (�����ʧ@�ѪR)                     **
**                                                        **
************************************************************/
/*w�boffset��s(offset,w�b���׵���)
*@�ت�:������m�ץ����w�Y��m!(�w�Y�y�� = ����y�� + offset)
*/
void CAction::W_UpdateNeedleMotorOffset(CPoint &offset, DOUBLE degree)
{
#ifdef MOVE
    CPoint tmpOffset(0, 0);
    tmpOffset = m_MachineOffSet;//����l��offset
    //DOUBLE degreeNow = MO_ReadLogicPositionW(); ��۹��
    offset.x = (LONG)round(tmpOffset.x*cos(degree *M_PI / 180.0) - tmpOffset.y*sin(degree *M_PI / 180.0));
    offset.y = (LONG)round(tmpOffset.x*sin(degree *M_PI / 180.0) + tmpOffset.y*cos(degree *M_PI / 180.0));
#endif
}

/*w�b�s���s�ƭ�
*@�ت�:�ǤJ����y��(����y�з|�ഫ���w�Y�y�жi����I �̫��ন����y�п�X)
*/
void CAction::W_UpdateNeedleMotor_Robot(LONG lX, LONG lY, LONG lZ, DOUBLE dAngle0, LONG lX2, LONG lY2, LONG lZ2, DOUBLE dAngle1, DOUBLE DisAngle)
{
#ifdef MOVE
    //�ϥ�X,Y,Z�T�b���q���p��
    CPoint cpoffsetbuf(0, 0);//offsetbuf
    W_UpdateNeedleMotorOffset(cpoffsetbuf, dAngle0);//OFFSET�P�����ഫ
    CPoint NeedlePointStart = { lX + cpoffsetbuf.x,lY + cpoffsetbuf.y };//����y���ഫ���w�Y�y��
    W_UpdateNeedleMotorOffset(cpoffsetbuf, dAngle1);//OFFSET�P�����ഫ
    CPoint NeedlePointEnd = { lX2 + cpoffsetbuf.x,lY2 + cpoffsetbuf.y };//����y���ഫ���w�Y�y��
    //  LONG Distance = (LONG)sqrt(pow(NeedlePointStart.x - NeedlePointEnd.x, 2) + pow(NeedlePointStart.y - NeedlePointEnd.y, 2));//�u�q�`����
	int idx = (int)((dAngle1 - dAngle0) / (DisAngle * 1.008));//�T�w���ר��X���I
	LONG idx_xVec = 0, idx_yVec = 0, idx_zVec = 0;
	if (fmod((dAngle1 - dAngle0), (DisAngle * 1.008)) != 0.0)
	{
		idx_xVec = (LONG)(DOUBLE(NeedlePointEnd.x - NeedlePointStart.x) / ((DOUBLE)abs(idx)+1.0));
		idx_yVec = (LONG)(DOUBLE(NeedlePointEnd.x - NeedlePointStart.x) / ((DOUBLE)abs(idx) + 1.0));
		idx_zVec = (LONG)fabs((DOUBLE)(NeedlePointEnd.y - NeedlePointStart.y) / ((DOUBLE)abs(idx) + 1.0));//�C���I���h��
	}
	else
	{
		idx_xVec = (LONG)(DOUBLE(NeedlePointEnd.x - NeedlePointStart.x) / (DOUBLE)abs(idx));
		idx_yVec = (LONG)(DOUBLE(NeedlePointEnd.x - NeedlePointStart.x) / (DOUBLE)abs(idx));
		idx_zVec = (LONG)fabs((DOUBLE)(NeedlePointEnd.y - NeedlePointStart.y) / (DOUBLE)abs(idx));//�C���I���h��
	}
    if(idx > 767)
    {
        AfxMessageBox(L"���Ȩ��׹L�p�Э��s�]�w");
        m_IsCutError = TRUE;
        return;
    }
    W_m_ptVec.clear();//�Ĥ@���βM��
    DATA_4Do[0].EndPX = lX;
    DATA_4Do[0].EndPY = lY;
    DATA_4Do[0].EndPZ = lZ;
    DATA_4Do[0].AngleW = dAngle0;
    W_m_ptVec.push_back(DATA_4Do[0]);//��X����y��
    DATA_4MOVE Data4buf = { 0 };//�ǭȥ�
    DATA_4Do[0].EndPX = NeedlePointStart.x;
    DATA_4Do[0].EndPY = NeedlePointStart.y;
	DOUBLE absAngle = 0;
    for(int i = 1; i < abs(idx); i++)
    {
		absAngle = dAngle0 + DisAngle * i* 1.008* ((idx>0) ? 1 : -1);
        W_UpdateNeedleMotorOffset(cpoffsetbuf, dAngle0 + absAngle);//OFFSET�P�����ഫ
        //LineGetToPoint(DATA_4Do[i].EndPX, DATA_4Do[i].EndPY, NeedlePointEnd.x, NeedlePointEnd.y, DATA_4Do[i - 1].EndPX, DATA_4Do[i - 1].EndPY, idx_dis);
        DATA_4Do[i].EndPX = NeedlePointStart.x + idx_xVec * i;
        DATA_4Do[i].EndPY = NeedlePointStart.y + idx_yVec * i;
        Data4buf.EndPX = DATA_4Do[i].EndPX - cpoffsetbuf.x;//�w�Y�y��-offset  �|�o�����y��!!!!!
        Data4buf.EndPY = DATA_4Do[i].EndPY - cpoffsetbuf.y;//�w�Y�y��-offset  �|�o�����y��!!!!!
        if(lZ == lZ2)
        {
            Data4buf.EndPZ = lZ;
        }
        else if(lZ > lZ2)
        {
            Data4buf.EndPZ = lZ - idx_zVec * i;
        }
        else
        {
            Data4buf.EndPZ = lZ + idx_zVec * i;
        }
        Data4buf.AngleW = absAngle;//w���﨤��
        W_m_ptVec.push_back(Data4buf);  //��X����y��
    }
    DATA_4Do[0].EndPX = lX2;
    DATA_4Do[0].EndPY = lY2;
    DATA_4Do[0].EndPZ = lZ2;
    DATA_4Do[0].AngleW = dAngle1;
    W_m_ptVec.push_back(DATA_4Do[0]);//��X����y��
    m_IsCutError = FALSE;
#endif
}
/*w�b�s���s�ƭ�
*@�ت�:�ǤJ�w�Y�y��(�w�Y�y�жi����I �̫��ন����y�п�X)
*/
void CAction::W_UpdateNeedleMotor_Needle(LONG lX, LONG lY, LONG lZ, DOUBLE dAngle0, LONG lX2, LONG lY2, LONG lZ2, DOUBLE dAngle1, DOUBLE DisAngle)
{
#ifdef MOVE
   //�ϥ�X,Y,Z�T�b���q���p��
    //LONG Distance = (LONG)sqrt(pow(lX - lX2, 2) + pow(lY - lY2, 2)+ pow(lZ - lZ2, 2));//�u�q�`����
	int idx = (int)((dAngle1 - dAngle0) / (DisAngle * 1.008));//�T�w���ר��X���I
	//DOUBLE dFin = dAngle1 - (DisAngle * 1.008)*idx;
	LONG idx_xVec = 0 , idx_yVec =0, idx_zVec =0;
	if (fmod((dAngle1 - dAngle0),(DisAngle * 1.008)) != 0.0)
	{
		idx_xVec = (LONG)(DOUBLE(lX2 - lX) / ((DOUBLE)abs(idx) + 1.0));
		idx_yVec = (LONG)(DOUBLE(lY2 - lY) / ((DOUBLE)abs(idx) + 1.0));
		idx_zVec = (LONG)abs((DOUBLE(lZ2 - lZ) / ((DOUBLE)abs(idx) + 1.0)));//�C���I���h��
	}
	else
	{
		idx_xVec = (LONG)(DOUBLE(lX2 - lX) / (DOUBLE)abs(idx));
		idx_yVec = (LONG)(DOUBLE(lY2 - lY) / (DOUBLE)abs(idx));
		idx_zVec = (LONG)abs((DOUBLE(lZ2 - lZ) / (DOUBLE)abs(idx)));//�C���I���h��
	}
    DATA_4MOVE Data4buf = { 0 };//�ǭȥ�
    if(abs(idx) > 767)
    {
        AfxMessageBox(L"���Ȩ��׹L�p�Э��s�]�w");
        m_IsCutError = TRUE;
        return;
    }
    W_m_ptVec.clear();//�Ĥ@���βM��
    CPoint cpoffsetbuf(0,0);//offsetbuf
    W_UpdateNeedleMotorOffset(cpoffsetbuf, dAngle0);//OFFSET�P�����ഫ
    DATA_4Do[0].EndPX = lX - cpoffsetbuf.x;
    DATA_4Do[0].EndPY = lY - cpoffsetbuf.y;
    DATA_4Do[0].EndPZ = lZ;
    DATA_4Do[0].AngleW = dAngle0;
    W_m_ptVec.push_back(DATA_4Do[0]);//��X����y��
    DATA_4Do[0].EndPX = lX;
    DATA_4Do[0].EndPY = lY;
	DOUBLE absAngle=0;
    for(int i = 1; i < abs(idx); i++)
    {
		absAngle = dAngle0 + DisAngle * i* 1.008* ((idx>0) ? 1 : -1);
        W_UpdateNeedleMotorOffset(cpoffsetbuf, absAngle);//OFFSET�P�����ഫ
        DATA_4Do[i].EndPX = lX + idx_xVec * i;
        DATA_4Do[i].EndPY = lY + idx_yVec * i;
        Data4buf.EndPX = DATA_4Do[i].EndPX - cpoffsetbuf.x;//�w�Y�y��-offset  �|�o�����y��!!!!!
        Data4buf.EndPY = DATA_4Do[i].EndPY - cpoffsetbuf.y;//�w�Y�y��-offset  �|�o�����y��!!!!!
        if(lZ == lZ2)
        {
            Data4buf.EndPZ = lZ;
        }
        else if(lZ > lZ2)
        {
            Data4buf.EndPZ = lZ - idx_zVec * i;
        }
        else
        {
            Data4buf.EndPZ = lZ + idx_zVec * i;
        }
        Data4buf.AngleW = absAngle;//w���﨤��
        W_m_ptVec.push_back(Data4buf);  //��X����y��
    }
    W_UpdateNeedleMotorOffset(cpoffsetbuf, dAngle1);//OFFSET�P�����ഫ
    DATA_4Do[0].EndPX = lX2 - cpoffsetbuf.x;
    DATA_4Do[0].EndPY = lY2 - cpoffsetbuf.y;
    DATA_4Do[0].EndPZ = lZ2;
    DATA_4Do[0].AngleW = dAngle1;
    W_m_ptVec.push_back(DATA_4Do[0]);//��X����y��
    m_IsCutError = FALSE;
#endif
}
/*w�b����(�w�Y�y�ЩT�w�P�@���I)
*@�ت�:��1�װ�����
*/
void CAction::W_Rotation(DOUBLE dAngle, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy, DOUBLE DisAngle)
{
#ifdef MOVE
	if (MO_ReadLogicPositionW() == dAngle || dAngle >360 || dAngle <-360 || MO_ReadLogicPositionW() >361 || MO_ReadLogicPositionW() <-361)
	{
		return;
	}
    CPoint cpNeedlePoint[2] = { 0 };//�w�Y�y��(0�ϥβ{�b��m/1�ϥέn���ʪ��ؼЦ�m)
    cpNeedlePoint[0] = W_GetNeedlePoint();//�N��U��m�ഫ���w�Y�y��
    //cpNeedlePoint[1] = W_GetNeedlePoint(MO_ReadLogicPosition(0), MO_ReadLogicPosition(1), dAngle, 1);//�NW�b�����m�ഫ���w�Y�y��
   // W_UpdateNeedleMotorOffset(cpNeedlePoint[1], dAngle);//���쨤��offset
    //W_UpdateNeedleMotor_Robot(MO_ReadLogicPosition(0), MO_ReadLogicPosition(1), MO_ReadLogicPosition(2), MO_ReadLogicPositionW(), cpNeedlePoint[0].x - cpNeedlePoint[1].x, cpNeedlePoint[0].y - cpNeedlePoint[1].y, MO_ReadLogicPosition(2), dAngle, DisAngle);//��1����
    //W_UpdateNeedleMotor_Needle(cpNeedlePoint[0].x, cpNeedlePoint[0].y, MO_ReadLogicPosition(2), MO_ReadLogicPositionW(), cpNeedlePoint[1].x, cpNeedlePoint[1].y, MO_ReadLogicPosition(2), dAngle, DisAngle);//��1����
    CPoint offset(0, 0);
    DATA_4MOVE data4M = { 0 };
	DOUBLE idx = (dAngle - MO_ReadLogicPositionW()) / (DisAngle * 1.008);
	DOUBLE absAngle = 0;
    W_m_ptVec.clear();
    for(int i = 0; i <=(LONG)fabs(idx); i++)
    {
		absAngle = MO_ReadLogicPositionW() + DisAngle*i* 1.008 * ((idx > 0) ? 1 : -1);
        W_UpdateNeedleMotorOffset(offset, absAngle);//�̨��ק�soffset
        //�w�Y�y����^����y��(����y��)
        data4M.EndPX = cpNeedlePoint[0].x - offset.x;
        data4M.EndPY = cpNeedlePoint[0].y - offset.y;
        data4M.EndPZ = MO_ReadLogicPosition(2);
        data4M.AngleW = absAngle;
        W_m_ptVec.push_back(data4M);
    }
	//�̫�@��
	W_UpdateNeedleMotorOffset(offset, dAngle);//�̨��ק�soffset
	data4M.EndPX = cpNeedlePoint[0].x - offset.x;
	data4M.EndPY = cpNeedlePoint[0].y - offset.y;
	data4M.EndPZ = MO_ReadLogicPosition(2);
	data4M.AngleW = dAngle;
	W_m_ptVec.push_back(data4M);

    if(!m_bIsStop)
    {
        W_Line4DtoMove(lWorkVelociy, lAcceleration, lInitVelociy);
        PreventMoveError();//����b�d�X��
    }
#endif
}
/*�o��w�Y�y��(��ؼҦ� 0.����{�b�w�Y��m 1.��J����y�Ю���w�Y��m)
*@�Ҧ�0:�o��"��U"�w�Y��m
*@�Ҧ�1:��J����y�бo��w�Y��m
*/
CPoint CAction::W_GetNeedlePoint(LONG lRobotX, LONG lRobotY, DOUBLE dRobotW, BOOL bMode)
{
#ifdef MOVE
    CPoint NeedleOffset(0,0),Needlebuf(0, 0);
    if(!bMode) //�Ҧ�0: ��U������y���ন�w�Y�y�п�X
    {
        W_UpdateNeedleMotorOffset(NeedleOffset, MO_ReadLogicPositionW());
        Needlebuf.x = MO_ReadLogicPosition(0) + NeedleOffset.x;
        Needlebuf.y = MO_ReadLogicPosition(1) + NeedleOffset.y;
    }
    else//�Ҧ�1: �]�w������y���ন�w�Y�y�п�X
    {
        W_UpdateNeedleMotorOffset(NeedleOffset, dRobotW);
        Needlebuf.x = lRobotX + NeedleOffset.x;
        Needlebuf.y = lRobotY + NeedleOffset.y;
    }
    return Needlebuf;
#endif
    return 0;
}
/*�o�����y��(��ؼҦ� 0.����{�b����y�� 1.��J�w�Y��m�������y��)
*@�Ҧ�0:�o��"��U"����y��
*@�Ҧ�1:��J�w�Y�y�бo�����y��
*/
CPoint CAction::W_GetMachinePoint(LONG lNeedleX, LONG lNeedleY, DOUBLE dNeedleW, BOOL bMode)
{
    CPoint NeedleOffset(0, 0),Machinebuf(0, 0);
    if(!bMode) //�Ҧ�0: ��U���w�Y�y���ন����y�п�X
    {
#ifdef MOVE
        Machinebuf.x = MO_ReadLogicPosition(0);
        Machinebuf.y = MO_ReadLogicPosition(1);
#endif
    }
    else//�Ҧ�1: �]�w���w�Y�y���ন����y�п�X
    {
        W_UpdateNeedleMotorOffset(NeedleOffset, dNeedleW);
        Machinebuf.x = lNeedleX - NeedleOffset.x;
        Machinebuf.y = lNeedleY - NeedleOffset.y;
    }
    return Machinebuf;
}

#ifdef MOVE
/*����y����۹�y��4�b�s�򴡸ɨϥ�*/
void CAction::W_AbsToOppo4Move(std::vector<DATA_4MOVE> &str)
{
    //����y��
    DATA_4MOVE mData;
    std::vector<DATA_4MOVE> vecBuf;
    vecBuf.clear();
    mData.EndPX = str.at(0).EndPX - MO_ReadLogicPosition(0);
    mData.EndPY = str.at(0).EndPY - MO_ReadLogicPosition(1);
    mData.EndPZ = str.at(0).EndPZ - MO_ReadLogicPosition(2);
    mData.AngleW = str.at(0).AngleW - MO_ReadLogicPositionW();
    mData.Distance = LONG(sqrt(pow(str.at(0).EndPX, 2) + pow(str.at(0).EndPY, 2)));
    vecBuf.push_back(mData);
    for(UINT i = 1; i < str.size(); i++)
    {

        mData.Speed = str.at(i).Speed;
        mData.EndPX = str.at(i).EndPX - str.at(i - 1).EndPX;
        mData.EndPY = str.at(i).EndPY - str.at(i - 1).EndPY;
        mData.EndPZ = str.at(i).EndPZ - str.at(i - 1).EndPZ;
        mData.AngleW = str.at(i).AngleW - str.at(i - 1).AngleW;
        mData.Distance = LONG(sqrt(pow(mData.EndPX, 2) + pow(mData.EndPY, 2)));
        vecBuf.push_back(mData);
        //TRACE(_T(",%d,%d,%.3f\n"), mData.EndPX, mData.EndPY, mData.AngleW);
    }
    str.clear();
    str.insert(str.end(), vecBuf.begin(), vecBuf.end());
    for(UINT i = 0; i < str.size() - 1; i++)
    {
        DATA_4Do[i] = { 0 };
        DATA_4Do[i] = str[i + 1];
    }

}
#endif
/*�s��u�q�ʧ@--(�|�b�s�򴡸�)
*@�ϥΫe�ݥ��g�JVector�ƭ�(W_m_ptVec)
*/
void CAction::W_Line4DtoDo(LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
#ifdef MOVE
    //�P�_W�b�O�_�ե��P���ȿ��~
    if(m_MachineOffSet.x == -99999 && m_MachineOffSet.y == -99999 || m_IsCutError == TRUE)
    {
        return;
    }
    std::vector<DATA_4MOVE>W_Buff;//�u�q�ȧP�_
    W_Buff.assign(W_m_ptVec.begin(), W_m_ptVec.end());
    W_AbsToOppo4Move(W_Buff);
    CPoint cpMachinePoint = W_GetMachinePoint();//�N��U��m�ഫ������y��
    if(!m_bIsStop)
    {
        MO_Do4DLineMove(W_m_ptVec.at(0).EndPX - cpMachinePoint.x, W_m_ptVec.at(0).EndPY- cpMachinePoint.y,0, W_m_ptVec.at(0).AngleW - MO_ReadLogicPositionW(), lWorkVelociy, lAcceleration, lInitVelociy);//w����(x,y,w�P��)
        PreventMoveError();//����b�d�X��
    }

    if(!m_bIsStop)
    {
        MO_Do3DLineMove(0, 0, W_m_ptVec.at(0).EndPZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//z�b����
        PreventMoveError();//����b�d�X��
    }
    if(!m_bIsStop)
    {
        if(!m_bIsStop && m_bIsDispend == 1)
        {
            MO_GummingSet();//�(���d)
        }
        MO_DO4Curve(DATA_4Do, W_Buff.size() - 1, lWorkVelociy);//�s�򴡸ɶ}�l
        PreventMoveError();//�����X�ʿ��~
    }
    memset(&DATA_4Do, 0, sizeof(DATA_4MOVE) * (sizeof(DATA_4Do) / sizeof(DATA_4Do[0])));//�M�Ű}�C
    MO_GummingSet();//�(���d)
#endif
}
/*W�b�|�s�򴡸ɳ�²���*/
/*�s��u�q�ʧ@--(�|�b�s�򴡸�)
*@�ϥΫe�ݥ��g�JVector�ƭ�(W_m_ptVec)
*/
void CAction::W_Line4DtoMove(LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
#ifdef MOVE
    //�P�_W�b�O�_�ե��P���ȿ��~
    if(m_MachineOffSet.x == -99999 && m_MachineOffSet.y == -99999 || m_IsCutError == TRUE)
    {
        return;
    }
    std::vector<DATA_4MOVE>W_Buff;//�u�q�ȧP�_
    W_Buff.assign(W_m_ptVec.begin(), W_m_ptVec.end());
    W_AbsToOppo4Move(W_Buff);
    MO_DO4Curve(DATA_4Do, W_Buff.size() - 1, lWorkVelociy);//�s�򴡸ɶ}�l
#endif
}
/*W�b�ե��ʧ@-�B�J1
*@�ت�:���o�P�b�߶��߰��лPOffset
*@�Ѽ�:�B�J1 bStep:0  �B�J2  bStep:1
*/
void CAction::W_Correction(BOOL bStep, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy,LONG lMoveZ)
{
#ifdef MOVE
    if(!bStep)  //�B�J1
    {
        m_Wangle = MO_ReadLogicPositionW();
        cpCirMidBuff[0].x = MO_ReadLogicPosition(0) ;
        cpCirMidBuff[0].y = MO_ReadLogicPosition(1) ;
		m_TablelZ = MO_ReadLogicPosition(2);//���oz�b�`����
        MO_Do3DLineMove(0, 0, lMoveZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//z�b��ɨ�10000��m
        PreventMoveError();
        MO_Do4DLineMove(0, 0, 0, 180,(LONG)round(lWorkVelociy / m_WSpeed), (LONG)(lAcceleration / m_WSpeed), lInitVelociy);//W�b����180��
        PreventMoveError();

    }
    else  //�B�J2
    {
        cpCirMidBuff[1].x = MO_ReadLogicPosition(0) ;
        cpCirMidBuff[1].y = MO_ReadLogicPosition(1) ;
        m_MachineCirMid.x = (LONG)round((cpCirMidBuff[0].x + cpCirMidBuff[1].x) / 2.0);//���o���(X)
        m_MachineCirMid.y = (LONG)round((cpCirMidBuff[0].y + cpCirMidBuff[1].y) / 2.0);//���o���(Y)
        m_MachineOffSet.x =  MO_ReadLogicPosition(0)- m_MachineCirMid.x;//���ooffsetx
        m_MachineOffSet.y =  MO_ReadLogicPosition(1)- m_MachineCirMid.y;//���ooffsety
		//�_���ե�(RESOLUTION)
		if (abs(m_MachineOffSet.x) % 2 == 1)
		{
			m_MachineOffSet.x = m_MachineOffSet.x + 1;
		}
		if (abs(m_MachineOffSet.y) % 2 == 1)
		{
			m_MachineOffSet.y = m_MachineOffSet.y + 1;
		}
        m_MachineCirRad = (LONG)sqrt(pow(m_MachineCirMid.x - MO_ReadLogicPosition(0), 2) + pow(m_MachineCirMid.y - MO_ReadLogicPosition(1), 2));//�b�|
    }
#endif
}

/*Ū���{�b��m����
*@��J:0:�w�Y�y�� / 1:����y��
*@
*/
AxeSpace CAction::MCO_ReadPosition(BOOL NedMah)
{
    AxeSpace SpaceBuf = {0};
#ifdef MOVE
    if(NedMah == 0) //�o��w�Y��m
    {
		//�P�_W�b�O�_�ե�
		if (m_MachineOffSet.x == -99999 && m_MachineOffSet.y == -99999)
		{
			return SpaceBuf = {-99999,-99999,-99999 ,-99999 };
		}
        SpaceBuf.x = W_GetNeedlePoint().x;
        SpaceBuf.y = W_GetNeedlePoint().y;
        SpaceBuf.z = MO_ReadLogicPosition(2);
        SpaceBuf.w = MO_ReadLogicPositionW();
    }
    else//�o������޿��m
    {
        SpaceBuf.x = MO_ReadLogicPosition(0);
        SpaceBuf.y = MO_ReadLogicPosition(1);
        SpaceBuf.z = MO_ReadLogicPosition(2);
        SpaceBuf.w = MO_ReadLogicPositionW();
    }
#endif // MOVE
    return SpaceBuf;

}
/*�|�b���ɲ���
*@�ت�:�|�b���ɲ���W�b�A�i�����۹�P����(0:����/1:�۹�)
*/
void CAction::MCO_Do4DLineMove(DOUBLE dAng, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy, BOOL bIsType)
{
	/*�|�b��m����(w����)
	 DOUBLE dAng
	*/
	/*�t�ΰѼ�(�X�ʳt�סA�[�t�סA��t��)
	LONG lWorkVelociy,LONG lAcceleration, LONG lInitVelociy
	*/
	/*�۹ﵴ��P�_
	BOOL bIsType
	*/
#ifdef MOVE
	//�P�_W�b�O�_�ե�
	if (m_MachineOffSet.x == -99999 && m_MachineOffSet.y == -99999)
	{
		return;
	}
	//�P�_�ϥΰw�Y�y���٬O����y��
	if (bIsType == 1) //�ϥά۹�y��
	{
		dAng = dAng + MO_ReadLogicPositionW();
	}
	/*******�H�U�{�����|���ܨϥε���y�а���B��************************/
	if (!m_bIsStop)
	{
		MO_Do4DLineMove(0,0,0,dAng- MO_ReadLogicPositionW(), lWorkVelociy, lAcceleration, lInitVelociy);//�|�b�s�򴡸�
		PreventMoveError();//����b�d�X��
	}
#endif
}
/*�w�Y�Ҧ����I�_�k
*@��J�Ѽ�:(�_�k�t��1�B�_�k�t��2�B�B�J0.1)
*@�B�J0:��l�ƭ��I�_�k���ʨ�T�w�����q��m/
*@�B�J1:���`���I�_�k���w�Y�y�Ь�0
*/
void CAction::W_NeedleGoHoming(LONG Speed1, LONG Speed2,BOOL bStep)
{
#ifdef MOVE
    m_ThreadFlag = 2;//W_NeedleGoHoming
	m_IsHomingOK = 0;//���I�_�k��l��
    cpCirMidBuff[0].x = Speed1;
    cpCirMidBuff[0].y = Speed2;
    cpCirMidBuff[1].x = bStep;
    AfxBeginThread(MoMoveThread, (LPVOID)this);
    /*************�H�U�O�d*******/
    //if (!bStep)
    //{
    //  DecideInitializationMachine(Speed1, Speed2, 15, m_HomingOffset_INIT.x, m_HomingOffset_INIT.y, m_HomingOffset_INIT.z, m_HomingOffset_INIT.w);
    //}
    //else
    //{
    //  //�P�_W�b�O�_�ե�
    //  if (m_MachineOffSet.x == -99999 && m_MachineOffSet.y == -99999)
    //  {
    //      DecideInitializationMachine(Speed1, Speed2, 15, m_HomingOffset_INIT.x, m_HomingOffset_INIT.y, m_HomingOffset_INIT.z, m_HomingOffset_INIT.w);
    //      return;
    //  }
    //  DecideInitializationMachine(Speed1, Speed2, 15, m_HomingPoint.x + m_MachineOffSet.x, m_HomingPoint.y + m_MachineOffSet.y, m_HomingPoint.z, m_HomingPoint.w);
    //  if (!m_bIsStop)
    //  {
    //      MO_Do3DLineMove(-m_MachineOffSet.x, -m_MachineOffSet.y, 0, Speed1, Speed1 * 3, Speed2);
    //      PreventMoveError();
    //  }
    //}
#endif // MOVE
}

/*JOG��ʲ��ʼҦ�
*@��J�۹�q�y�жi�沾��(X,Y,Z,W�۹�q����/WType:0��b,1�P�b����/������0�ϥδ�t����)
*@
*/
void CAction::MCO_JogMove(LONG lX, LONG lY, LONG lZ, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy, DOUBLE dW, BOOL WType)
{
#ifdef MOVE
    int sw=0;//���
    if(lX == 0 && lY == 0 && lZ == 0 && dW == 0)
    {
        sw = 0;//����
    }
    else if(dW == 0)
    {
        sw = 1;//x,y,z��b����
    }
    else
    {
        sw = 2;//w�b����
    }
    switch(sw)
    {
        case 0:
        {
            MO_DecSTOP();//��t����
            break;
        }
        case 1:
        {
            MO_Do3DLineMove(lX, lY, lZ, lWorkVelociy, lAcceleration, lInitVelociy);//x,y,z�b�۹�q����
            break;
        }
        case 2:
        {
            if(WType == 0) //W��b����
            {
                MO_Do4DLineMove(0, 0, 0, dW, (LONG)round(lWorkVelociy / m_WSpeed), (LONG)(lAcceleration / m_WSpeed), lInitVelociy);//w�b��b����
            }
            else //W���� �w�w�Y�����߱���
            {
                //dW, lWorkVelociy, lAcceleration, lInitVelociy
                //W_Rotation(dW, lWorkVelociy, lAcceleration, lInitVelociy);
                m_ThreadFlag = 1;//W_Rotation
                WangBuff = dW;
                cpCirMidBuff[0].x = lWorkVelociy;
                cpCirMidBuff[0].y = lAcceleration;
                cpCirMidBuff[1].x = lInitVelociy;
                AfxBeginThread(MoMoveThread, (LPVOID)this);
            }
            break;
        }
        default:
            break;
    }
#endif
}

/*���׶꩷����*/
void CAction::AnglCir(LONG lCenX, LONG lCenY, LONG lR, DOUBLE dStartAngl,DOUBLE dEndAngl, LONG lWorkVelociy, LONG lInitVelociy)
{
#ifdef MOVE
    //lR.�ꪽ�|�]mm�^
    //dStartAngl.�۹���Ҧb�B��ߪ��_�I���ס]���G�ס^�C �w�]0�׬۷�����3 : 00��m�C �q�{ = 0�]�ס^�Ƚd��G0 �V 360
    //dEndAngl.��󨤫װ_�I�Ȥ���B�I���ާ@�N�b��������ס]���G�ס^�C �q�{ = 0�]�ס^�n�Q���f�ɰw��V�I���A�i��J�@�ӭt��
    CPoint cptS, cptE;
    DOUBLE dRadStart = 0, dRadEnd = 0;
    LONG  lNowX = 0, lNowY = 0;
    BOOL bRev = 0;//0�f��/1����
    lNowX = MO_ReadLogicPosition(0);
    lNowY = MO_ReadLogicPosition(1);
    dRadStart = dStartAngl * 180 / M_PI;
    dRadEnd = dEndAngl * 180 / M_PI;
    cptS.x = LONG(lR * cos(dRadStart) + lCenX);
    cptS.y = LONG(lR * sin(dRadStart) + lCenY);
    cptE.x = LONG(lR * cos(dRadEnd) + lCenX);
    cptE.y = LONG(lR* sin(dRadEnd) + lCenY);
    if(dEndAngl > 0)
    {
        bRev = 1;//����
    }
    if(!m_bIsStop)
    {
        MO_Do2DArcMove(cptS.x - lNowX, cptE.y - lNowY, lCenX - lNowX, lCenY - lNowY,
                       lInitVelociy, lWorkVelociy, bRev);//���ײ��ʶ�
        PreventMoveError();//����b�d�X��
    }
#endif
}
/*�T�b�꩷����*/
void CAction::Do3AxisCirle(LONG x1, LONG y1, LONG z1, LONG x2, LONG y2, LONG z2, LONG x3, LONG y3, LONG z3, LONG speed)
{
#ifdef MOVE
    CCircleFormula A;
    AxeSpace p1, p2, p3;
    p1 = { x1,y1,z1,0 };
    p2 = { x2,y2,z2,0 };
    p3 = { x3,y3,z3,0 };
    std::vector<AxeSpace> M;
    A.CircleCutPoint(p1, p2, p3, M);
    //A.ArcCutPoint(p1, p2, p3, M);


    size_t num = M.size();
    DATA_3MOVE *pDataM = new DATA_3MOVE[num];
    DATA_3MOVE *pDataShift = pDataM;
    for(size_t i = 0; i<M.size(); i++)
    {
        if(i == 0)
        {
            pDataShift->EndPX = M.at(i).x - MO_ReadLogicPosition(0);
            pDataShift->EndPY = M.at(i).y - MO_ReadLogicPosition(1);
            pDataShift->EndPZ = M.at(i).z - MO_ReadLogicPosition(2);
            pDataShift->Speed = pDataShift->Distance = 0;    //�ǩǪ�!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        }
        else
        {
            pDataShift->EndPX = M.at(i).x - M.at(i - 1).x;
            pDataShift->EndPY = M.at(i).y - M.at(i - 1).y;
            pDataShift->EndPZ = M.at(i).z - M.at(i - 1).z;
            pDataShift->Speed = pDataShift->Distance = 0;    //�ǩǪ�!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        }
        pDataShift++;
    }
    //_cwprintf(L"data count:%d\n", num);

    MO_Do3DLineMove(pDataM->EndPX, pDataM->EndPY, 0, speed, 100000, 2000);
    PreventMoveError();

    MO_Do3DLineMove(0, 0, pDataM->EndPZ, speed, 100000, 2000);
    PreventMoveError();
    pDataShift = pDataM + 1;
    //_cwprintf(L"�@ %d ���I\n", num);

    /*for(int i = 0; i<num; i++)
    {
    _cwprintf(L"%d, %d, %d\n", pDataM[i].EndPX, pDataM[i].EndPY, pDataM[i].EndPZ);
    }*/

    MO_DO3Curve(pDataShift, num - 1, speed);

    PreventMoveError();//�����X�ʿ��~
    pDataShift = NULL;
    delete[] pDataM;

    //LONG errorR2 = MO_ReadReg(2, 0);
    //_cwprintf(L"R2:%x\n", errorR2);

    /*LA_m_ptVec.clear();
    for(size_t i = 0; i<M.size(); i++)
    {
    DATA_3MOVE data3m;
    data3m.EndPX = M.at(i).x;
    data3m.EndPY = M.at(i).y;
    data3m.EndPZ = M.at(i).z;
    data3m.Speed = data3m.Distance = 0;
    LA_m_ptVec.push_back(data3m);
    }
    LA_m_iVecSP.push_back(1);
    LA_m_iVecSP.push_back(LA_m_ptVec.size());
    m_bIsGetLAend = TRUE;
    LA_Line3DtoDo(1, 200000, 500000, 3000);*/
#endif
}
/*======================================���ϥγs�򴡸ɪ���R���A================================================================================================================*/
/*����---��R�κA(���A2������۶�R.�ѥ~�Ӥ�)
*��J(�_�l�Ix1,y1,���x2,y2,�e��,�X�ʳt��,�[�t��,��t��)
*/
void CAction::AttachFillType2_1(LONG lX1, LONG lY1, LONG lCenX, LONG lCenY, LONG lZ, LONG lZBackDistance, LONG lWidth, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
#ifdef MOVE
    #pragma region ****�ꫬ���ۥ\��****
    DOUBLE dRadius = 0, dWidth = 0, dAng0 = 0, dAng1 = 0, dAng2 = 0;
    BOOL bRev = 1;//0�f��/1����
    LONG lLineClose = 0, lXClose = 0, lYClose = 0, lDistance = 0;
    LONG lNowX = 0, lNowY = 0;
    CPoint cPt1 = 0, cPt2 = 0, cPt3 = 0, cPt4 = 0, cPtCen1 = 0, cPtCen2 = 0;
    int iData = 0, iOdd = 0;//�P�_�_��(�_���W�b��/�����U�b��)
    CString csbuff = 0;
    std::vector<CPoint>m_ptVec;
    std::vector<CPoint>::iterator ptIter;//�ϦV���N��
    m_ptVec.clear();
    cPt1.x = lX1;
    cPt1.y = lY1;
    cPtCen1.x = lCenX;
    cPtCen1.y = lCenY;//�W�b����
    dRadius = sqrt(pow(cPtCen1.x - cPt1.x, 2) + pow(cPtCen1.y - cPt1.y, 2));//�b�|
    if(dRadius == 0)
    {
        return;
    }
    lDistance = LONG(dRadius);
    dWidth = lWidth * 1000;
    dAng1 = acos((cPt1.x - cPtCen1.x) / dRadius);
    dAng2 = asin((cPt1.y - cPtCen1.y) / dRadius);
    cPt2.x = LONG(dRadius*cos(dAng1 + M_PI) + cPtCen1.x);
    cPt2.y = LONG(dRadius*sin(dAng2 + M_PI) + cPtCen1.y);
    m_ptVec.push_back(cPt1);
    m_ptVec.push_back(cPt2);
    lDistance = LONG(lDistance - dWidth);
    LineGetToPoint(lXClose, lYClose, cPt1.x, cPt1.y, cPtCen1.x, cPtCen1.y,
                   lDistance);
    cPt3.x = lXClose;
    cPt3.y = lYClose;
    cPtCen2.x = (cPt3.x + cPt2.x) / 2;
    cPtCen2.y = (cPt3.y + cPt2.y) / 2;//�U�b����
    iData = (int)dRadius % (int)dWidth;
    lDistance = LONG(lDistance + dWidth);
    while(1)
    {
        lDistance = LONG(lDistance - dWidth);
        if(lDistance < dWidth)
        {
            break;
        }
        LineGetToPoint(lXClose, lYClose, cPt1.x, cPt1.y, cPtCen1.x, cPtCen1.y,
                       lDistance);
        cPt3.x = lXClose;
        cPt3.y = lYClose;
        m_ptVec.push_back(cPt3);
        cPt4.x = LONG(lDistance*cos(dAng1 + M_PI) + cPtCen1.x);
        cPt4.y = LONG(lDistance*sin(dAng2 + M_PI) + cPtCen1.y);
        m_ptVec.push_back(cPt4);
    }
    #pragma endregion
    std::vector<DATA_2MOVE> DATA_2DO;
    UINT cnt = 0;
    DATA_2DO.clear();
    for(ptIter = m_ptVec.begin(); ptIter != m_ptVec.end(); ptIter++)
    {
        if((iData != 0) && (cnt == m_ptVec.size() - 1))
        {
            MCO_Do2dDataLine((*ptIter).x, (*ptIter).y, DATA_2DO);
        }
        else if((iData == 0) && (cnt == m_ptVec.size() - 1))
        {
            MCO_Do2dDataCir((*ptIter).x, (*ptIter).y, cPtCen2.x, cPtCen2.y, bRev, DATA_2DO);
        }
        else
        {
            if(cnt % 2 == 0)
            {
                MCO_Do2dDataCir((*ptIter).x, (*ptIter).y, cPtCen2.x, cPtCen2.y, bRev, DATA_2DO);
            }
            else
            {
                MCO_Do2dDataCir((*ptIter).x, (*ptIter).y, lCenX, lCenY, bRev, DATA_2DO);
            }
        }
        cnt++;
    }
    LA_AbsToOppo2Move(DATA_2DO);

    if((iData != 0))
    {
        if(!m_bIsStop)
        {
            MO_Do3DLineMove(DATA_2DO.at(0).EndP.x, DATA_2DO.at(0).EndP.y, 0, lWorkVelociy, lAcceleration, lInitVelociy);//���u����
            PreventMoveError();
        }
        PauseDoGlue();//�Ȱ���_���~��X��(m_bIsPause=0) �X��
    }
    else
    {
        if(!m_bIsStop)
        {
            MO_Do2DArcMove(DATA_2DO.at(0).EndP.x, DATA_2DO.at(0).EndP.y, DATA_2DO.at(0).CirCentP.x, DATA_2DO.at(0).CirCentP.y,
                           lInitVelociy, lWorkVelociy, DATA_2DO.at(0).CirRev);//��l�b��
            PreventMoveError();
        }
        PauseDoGlue();//�Ȱ���_���~��X��(m_bIsPause=0) �X��
    }

    for(UINT i = 1; i < DATA_2DO.size(); i++)
    {
        DATA_2Do[i - 1] = DATA_2DO.at(i);
    }
    MO_DO2Curve(DATA_2Do, DATA_2DO.size() - 1, lWorkVelociy);
    PreventMoveError();//�����X�ʿ��~
    Sleep(200);
    DATA_2DO.clear();

    PauseStopGlue();//�Ȱ��ɰ����(m_bIsPause=1)
    MO_StopGumming();//����X��
    if(!m_bIsStop)
    {
        MO_Do3DLineMove(0, 0, (lZ - lZBackDistance) - lZ, lWorkVelociy, lAcceleration,
                        lInitVelociy);//Z�b��^
        PreventMoveError();//����b�d�X��
    }
#endif
}
/*����---��R�κA(���A3�x�ζ�R.�ѥ~�Ӥ�)
*��J(�_�l�Ix1,y1,�����Ix2,y2,�e��,�X�ʳt��,�[�t��,��t��)
*/
void CAction::AttachFillType3_1(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lZ, LONG lZBackDistance, LONG lWidth, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
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
    cPt3.y = lY2;
    dWidth = lWidth * 1000;
    cPtCen.x = cPt1.x + (cPt3.x - cPt1.x) / 2;
    cPtCen.y = cPt1.y + (cPt3.y - cPt1.y) / 2;
    dRadius = sqrt(pow(cPt1.x - cPtCen.x, 2) + pow(cPt1.y - cPtCen.y, 2));
    if(dRadius == 0)
    {
        return;
    }
    dAngCenCos = acos(DOUBLE(cPt1.x - cPtCen.x) / dRadius);
    dAngCenSin = asin(DOUBLE(cPt1.y - cPtCen.y) / dRadius);
    dAngCenCos2 = M_PI * 2 - dAngCenCos;
    dAngCenSin2 = M_PI - dAngCenSin;
    if(abs(dAngCenCos - dAngCenSin) > 0.01)
    {
        if(abs(dAngCenCos - dAngCenSin2) < 0.01)
        {
            dAngCenSin = dAngCenSin2;
        }
        else if(abs(dAngCenCos2 - dAngCenSin) < 0.01)
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
    cPt2.x = LONG(dRadius*cos(dAngCenCos + M_PI_2) + cPtCen.x);
    cPt2.y = LONG(dRadius*sin(dAngCenSin + M_PI_2) + cPtCen.y);
    cPt4.x = LONG(dRadius*cos(dAngCenCos + M_PI_2 * 3) + cPtCen.x);
    cPt4.y = LONG(dRadius*sin(dAngCenSin + M_PI_2 * 3) + cPtCen.y);
    dDistance = sqrt(pow((cPt1.x - cPt4.x), 2) + pow((cPt1.y - cPt4.y), 2));
    m_ptVec.push_back(cPt1);
    m_ptVec.push_back(cPt2);
    m_ptVec.push_back(cPt3);
    m_ptVec.push_back(cPt4);
    dAngCos = acos((cPt1.x - cPt4.x) / dDistance);
    dAngSin = asin((cPt1.y - cPt4.y) / dDistance);
    if(dAngCos < 0)
    {
        dAngCos += M_PI * 2;
    }
    if(dAngSin < 0)
    {
        dAngSin += M_PI * 2;
    }
    dAngCos2 = M_PI * 2 - dAngCos;
    dAngSin2 = M_PI - dAngSin;
    if(abs(dAngCos - dAngSin) > 0.01)
    {
        if(abs(dAngCos - dAngSin2) < 0.01)
        {
            dAngSin = dAngSin2;
        }
        else if(abs(dAngCos2 - dAngSin) < 0.01)
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
    while(1)
    {
        dRadius = dRadius - dWidth*sqrt(2);
        dDistance = dDistance - dWidth;
        if(dDistance < dWidth)
        {
            break;
        }
        cPt1.x = LONG(dDistance*cos(dAngCos) + cPt4.x);
        cPt1.y = LONG(dDistance*sin(dAngSin) + cPt4.y);
        m_ptVec.push_back(cPt1);
        cPt2.x = LONG(dRadius*cos(dAngCenCos + M_PI_2) + cPtCen.x);
        cPt2.y = LONG(dRadius*sin(dAngCenSin + M_PI_2) + cPtCen.y);
        m_ptVec.push_back(cPt2);
        dDistance = dDistance - dWidth;
        if(dDistance < dWidth)
        {
            break;
        }
        cPt3.x = LONG(dRadius*cos(dAngCenCos + M_PI) + cPtCen.x);
        cPt3.y = LONG(dRadius*sin(dAngCenSin + M_PI) + cPtCen.y);
        m_ptVec.push_back(cPt3);
        cPt4.x = LONG(dRadius*cos(dAngCenCos + M_PI_2 * 3) + cPtCen.x);
        cPt4.y = LONG(dRadius*sin(dAngCenSin + M_PI_2 * 3) + cPtCen.y);
        m_ptVec.push_back(cPt4);
    }
    #pragma endregion
    PauseDoGlue();//�Ȱ���_���~��X��(m_bIsPause=0)�X��
    for(ptIter = m_ptVec.begin(); ptIter != m_ptVec.end(); ptIter++)
    {
        lNowX = MO_ReadLogicPosition(0);
        lNowY = MO_ReadLogicPosition(1);
        if(!m_bIsStop)
        {
            MO_Do3DLineMove((*ptIter).x - lNowX, (*ptIter).y - lNowY, 0, lWorkVelociy, lAcceleration, lInitVelociy);//����
            PreventMoveError();
        }
    }
    PauseStopGlue();//�Ȱ��ɰ����(m_bIsPause=1)
    MO_StopGumming();//����X��
    if(!m_bIsStop)
    {
        MO_Do3DLineMove(0, 0, (lZ - lZBackDistance) - lZ, lWorkVelociy, lAcceleration, lInitVelociy);//Z�b��^
        PreventMoveError();//����b�d�X��
    }
#endif
}
/*����---��R�κA(���A4�x����)
*��J(�_�l�Ix1,y1,�����Ix2,y2,�e��,��ݼe��,�X�ʳt��,�[�t��,��t��)
*/
void CAction::AttachFillType4_1(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lZ, LONG lZBackDistance, LONG lWidth, LONG lWidth2, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
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
    cPt3.y = lY2;
    dWidth = lWidth * 1000;
    dWidth2 = lWidth2 * 1000;
    cPtCen.x = cPt1.x + (cPt3.x - cPt1.x) / 2;
    cPtCen.y = cPt1.y + (cPt3.y - cPt1.y) / 2;
    dRadius = sqrt(pow(cPt1.x - cPtCen.x, 2) + pow(cPt1.y - cPtCen.y, 2));
    if(dRadius == 0)
    {
        return;
    }
    dAngCenCos = acos(DOUBLE(cPt1.x - cPtCen.x) / dRadius);
    dAngCenSin = asin(DOUBLE(cPt1.y - cPtCen.y) / dRadius);
    dAngCenCos2 = M_PI * 2 - dAngCenCos;
    dAngCenSin2 = M_PI - dAngCenSin;
    if(abs(dAngCenCos - dAngCenSin) > 0.01)
    {
        if(abs(dAngCenCos - dAngCenSin2) < 0.01)
        {
            dAngCenSin = dAngCenSin2;
        }
        else if(abs(dAngCenCos2 - dAngCenSin) < 0.01)
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
    cPt2.x = LONG(dRadius*cos(dAngCenCos + M_PI_2) + cPtCen.x);
    cPt2.y = LONG(dRadius*sin(dAngCenSin + M_PI_2) + cPtCen.y);
    cPt4.x = LONG(dRadius*cos(dAngCenCos + M_PI_2 * 3) + cPtCen.x);
    cPt4.y = LONG(dRadius*sin(dAngCenSin + M_PI_2 * 3) + cPtCen.y);
    dDistance = sqrt(pow((cPt1.x - cPt4.x), 2) + pow((cPt1.y - cPt4.y), 2));
    m_ptVec.push_back(cPt1);
    m_ptVec.push_back(cPt2);
    m_ptVec.push_back(cPt3);
    m_ptVec.push_back(cPt4);

    dAngCos = acos((cPt1.x - cPt4.x) / dDistance);
    dAngSin = asin((cPt1.y - cPt4.y) / dDistance);
    if(dAngCos < 0)
    {
        dAngCos += M_PI * 2;
    }
    if(dAngSin < 0)
    {
        dAngSin += M_PI * 2;
    }
    dAngCos2 = M_PI * 2 - dAngCos;
    dAngSin2 = M_PI - dAngSin;
    if(abs(dAngCos - dAngSin) > 0.01)
    {
        if(abs(dAngCos - dAngSin2) < 0.01)
        {
            dAngSin = dAngSin2;
        }
        else if(abs(dAngCos2 - dAngSin) < 0.01)
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
    while(1)
    {
        if((iBuff == 1) && (dRadius - (2 * dWidth2) < 0))
        {
#ifdef PRINTF
        //    _cwprintf(_T("��ݼe�׹L�j \n"));
#endif
            m_ptVec.push_back(cPt1);
            break;
        }
        dRadius = dRadius - dWidth*sqrt(2);
        dDistance = dDistance - dWidth;
        if(iBuff*dWidth >  dWidth2)  //dWidth2��ݼe��
        {
            dDistance += dWidth;
            cPt1.x = LONG(dDistance*cos(dAngCos) + cPt4.x);
            cPt1.y = LONG(dDistance*sin(dAngSin) + cPt4.y);
            m_ptVec.push_back(cPt1);
            break;
        }
        if(dDistance < dWidth)
        {
            break;
        }
        cPt1.x = LONG(dDistance*cos(dAngCos) + cPt4.x);
        cPt1.y = LONG(dDistance*sin(dAngSin) + cPt4.y);
        m_ptVec.push_back(cPt1);
        cPt2.x = LONG(dRadius*cos(dAngCenCos + M_PI_2) + cPtCen.x);
        cPt2.y = LONG(dRadius*sin(dAngCenSin + M_PI_2) + cPtCen.y);
        m_ptVec.push_back(cPt2);
        dDistance = dDistance - dWidth;
        if(dDistance < dWidth)
        {
            break;
        }
        cPt3.x = LONG(dRadius*cos(dAngCenCos + M_PI) + cPtCen.x);
        cPt3.y = LONG(dRadius*sin(dAngCenSin + M_PI) + cPtCen.y);
        m_ptVec.push_back(cPt3);
        cPt4.x = LONG(dRadius*cos(dAngCenCos + M_PI_2 * 3) + cPtCen.x);
        cPt4.y = LONG(dRadius*sin(dAngCenSin + M_PI_2 * 3) + cPtCen.y);
        m_ptVec.push_back(cPt4);
        iBuff++;
    }
    #pragma endregion
    PauseDoGlue();//�Ȱ���_���~��X��(m_bIsPause=0)�X��
    for(ptIter = m_ptVec.begin(); ptIter != m_ptVec.end(); ptIter++)
    {
        lNowX = MO_ReadLogicPosition(0);
        lNowY = MO_ReadLogicPosition(1);
        if(!m_bIsStop)
        {
            MO_Do3DLineMove((*ptIter).x - lNowX, (*ptIter).y - lNowY, 0, lWorkVelociy, lAcceleration, lInitVelociy);//����
            PreventMoveError();
        }
    }
    PauseStopGlue();//�Ȱ��ɰ����(m_bIsPause=1)
    MO_StopGumming();//����X��
    if(!m_bIsStop)
    {
        MO_Do3DLineMove(0, 0, (lZ - lZBackDistance) - lZ, lWorkVelociy, lAcceleration, lInitVelociy);//Z�b��^
        PreventMoveError();//����b�d�X��
    }
#endif
}
/*����---��R�κA(���A5����)
*��J(�_�l�Ix1,y1,�����Ix2,y2,�e��,��ݼe��,�X�ʳt��,�[�t��,��t��)
*/
void CAction::AttachFillType5_1(LONG lX1, LONG lY1, LONG lCenX, LONG lCenY, LONG lZ, LONG lZBackDistance, LONG lWidth, LONG lWidth2, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
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
    if(dRadius == 0)
    {
        return;
    }
    lDistance = LONG(dRadius);
    dWidth = lWidth * 1000;
    dWidth2 = lWidth2 * 1000;
    dAng1 = acos((cPt1.x - cPtCen1.x) / dRadius);
    dAng2 = asin((cPt1.y - cPtCen1.y) / dRadius);
    cPt2.x = LONG(dRadius*cos(dAng1 + M_PI) + cPtCen1.x);
    cPt2.y = LONG(dRadius*sin(dAng2 + M_PI) + cPtCen1.y);
    m_ptVec.push_back(cPt1);
    m_ptVec.push_back(cPt2);
    lDistance = LONG(lDistance - dWidth);
    LineGetToPoint(lXClose, lYClose, cPt1.x, cPt1.y, cPtCen1.x, cPtCen1.y,
                   lDistance);
    cPt3.x = lXClose;
    cPt3.y = lYClose;
    cPtCen2.x = (cPt3.x + cPt2.x) / 2;
    cPtCen2.y = (cPt3.y + cPt2.y) / 2;//�U�b����
    lDistance = LONG(lDistance + dWidth);
    iData = (int)dRadius % (int)dWidth;
    iBuff = 1;
    while(1)
    {
        if((iBuff == 1) && (lDistance - (2 * dWidth2) < 0))
        {
#ifdef PRINTF
      //      _cwprintf(_T("��ݼe�׹L�j \n"));
#endif
            break;
        }
        dRadius = dRadius - dWidth;
        lDistance = LONG(lDistance - dWidth);
        if(iBuff * dWidth > dWidth2)
        {
            lDistance += LONG(dWidth);
            LineGetToPoint(lXClose, lYClose, cPt1.x, cPt1.y, cPtCen1.x, cPtCen1.y,
                           lDistance);
            cPt3.x = lXClose;
            cPt3.y = lYClose;
            m_ptVec.push_back(cPt3);
            break;
        }
        LineGetToPoint(lXClose, lYClose, cPt1.x, cPt1.y, cPtCen1.x, cPtCen1.y,
                       lDistance);
        cPt3.x = lXClose;
        cPt3.y = lYClose;
        m_ptVec.push_back(cPt3);
        cPt4.x = LONG(dRadius*cos(dAng1 + M_PI) + cPtCen1.x);
        cPt4.y = LONG(lDistance*sin(dAng2 + M_PI) + cPtCen1.y);
        m_ptVec.push_back(cPt4);
        iBuff++;
    }
    #pragma endregion
    std::vector<DATA_2MOVE> DATA_2DO;
    DATA_2DO.clear();
    for(UINT i = 1; i < m_ptVec.size(); i++)
    {
        if(i == m_ptVec.size() - 1)
        {
            if(i % 2 == 0)
            {
                MCO_Do2dDataCir(m_ptVec.at(i).x, m_ptVec.at(i).y, cPtCen1.x, cPtCen1.y, bRev, DATA_2DO);//�W�b��
            }
            else
            {
                MCO_Do2dDataCir(m_ptVec.at(i).x, m_ptVec.at(i).y, cPtCen2.x, cPtCen2.y, bRev, DATA_2DO);//�U�b��
            }
        }
        else
        {
            if(i % 2 != 0)
            {
                MCO_Do2dDataCir(m_ptVec.at(i).x, m_ptVec.at(i).y, cPtCen1.x, cPtCen1.y, bRev, DATA_2DO);//�W�b��
            }
            else
            {
                MCO_Do2dDataCir(m_ptVec.at(i).x, m_ptVec.at(i).y, cPtCen2.x, cPtCen2.y, bRev, DATA_2DO);//�U�b��
            }
        }
    }
    LA_AbsToOppo2Move(DATA_2DO);
    PauseDoGlue();//�Ȱ���_���~��X��(m_bIsPause=0) �X��
    for(UINT i = 0; i < DATA_2DO.size(); i++)
    {
        DATA_2Do[i] = DATA_2DO.at(i);
    }
    MO_DO2Curve(DATA_2Do, DATA_2DO.size(), lWorkVelociy);
    PreventMoveError();//�����X�ʿ��~
    Sleep(200);
    DATA_2DO.clear();

    PauseStopGlue();//�Ȱ��ɰ����(m_bIsPause=1)
    MO_StopGumming();//����X��
    if(!m_bIsStop)
    {
        MO_Do3DLineMove(0, 0, (lZ - lZBackDistance) - lZ, lWorkVelociy, lAcceleration,
                        lInitVelociy);//Z�b��^
        PreventMoveError();//����b�d�X��
    }
#endif
}
/*����---��R�κA(���A6�x�ζ�R.�Ѥ��ӥ~)
*��J(�_�l�Ix1,y1,�����Ix2,y2,�e��,�X�ʳt��,�[�t��,��t��)
*/
void CAction::AttachFillType6_1(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lZ, LONG lZBackDistance, LONG lWidth, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
#ifdef MOVE
    #pragma region ****�Ƥ諬���Y4�I�\��****
    LONG lNowX = 0, lNowY = 0, lNowZ = 0;
    CPoint cPt1 = 0, cPt2 = 0, cPt3 = 0, cPt4 = 0, cPtCen = 0;
    DOUBLE dRadius = 0, dDistance = 0, dWidth = 0, dWidth2 = 0, dAngCenCos = 0, dAngCenSin = 0, dAngCos = 0, dAngSin = 0;
    DOUBLE dAngCenCos2 = 0, dAngCenSin2 = 0, dAngCos2 = 0, dAngSin2 = 0;
    std::vector<CPoint>::reverse_iterator rptIter;//�ϦV���N��
    std::vector<CPoint> m_ptVec;
    m_ptVec.clear();
    cPt1.x = lX1;
    cPt1.y = lY1;
    cPt3.x = lX2;
    cPt3.y = lY2;
    dWidth = lWidth * 1000;
    cPtCen.x = cPt1.x + (cPt3.x - cPt1.x) / 2;
    cPtCen.y = cPt1.y + (cPt3.y - cPt1.y) / 2;
    dRadius = sqrt(pow(cPt1.x - cPtCen.x, 2) + pow(cPt1.y - cPtCen.y, 2));
    if(dRadius == 0)
    {
        return;
    }
    dAngCenCos = acos(DOUBLE(cPt1.x - cPtCen.x) / dRadius);
    dAngCenSin = asin(DOUBLE(cPt1.y - cPtCen.y) / dRadius);
    dAngCenCos2 = M_PI * 2 - dAngCenCos;
    dAngCenSin2 = M_PI - dAngCenSin;
    if(abs(dAngCenCos - dAngCenSin) > 0.01)
    {
        if(abs(dAngCenCos - dAngCenSin2) < 0.01)
        {
            dAngCenSin = dAngCenSin2;
        }
        else if(abs(dAngCenCos2 - dAngCenSin) < 0.01)
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
    cPt2.x = LONG(dRadius*cos(dAngCenCos + M_PI_2) + cPtCen.x);
    cPt2.y = LONG(dRadius*sin(dAngCenSin + M_PI_2) + cPtCen.y);
    cPt4.x = LONG(dRadius*cos(dAngCenCos + M_PI_2 * 3) + cPtCen.x);
    cPt4.y = LONG(dRadius*sin(dAngCenSin + M_PI_2 * 3) + cPtCen.y);
    dDistance = sqrt(pow((cPt1.x - cPt4.x), 2) + pow((cPt1.y - cPt4.y), 2));
    m_ptVec.push_back(cPt1);
    m_ptVec.push_back(cPt2);
    m_ptVec.push_back(cPt3);
    m_ptVec.push_back(cPt4);

    dAngCos = acos((cPt1.x - cPt4.x) / dDistance);
    dAngSin = asin((cPt1.y - cPt4.y) / dDistance);
    if(dAngCos < 0)
    {
        dAngCos += M_PI * 2;
    }
    if(dAngSin < 0)
    {
        dAngSin += M_PI * 2;
    }
    dAngCos2 = M_PI * 2 - dAngCos;
    dAngSin2 = M_PI - dAngSin;
    if(abs(dAngCos - dAngSin) > 0.01)
    {
        if(abs(dAngCos - dAngSin2) < 0.01)
        {
            dAngSin = dAngSin2;
        }
        else if(abs(dAngCos2 - dAngSin) < 0.01)
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
    while(1)
    {
        dRadius = dRadius - dWidth*sqrt(2);
        dDistance = dDistance - dWidth;
        if(dDistance < dWidth)
        {
            break;
        }
        cPt1.x = LONG(dDistance*cos(dAngCos) + cPt4.x);
        cPt1.y = LONG(dDistance*sin(dAngSin) + cPt4.y);
        m_ptVec.push_back(cPt1);
        cPt2.x = LONG(dRadius*cos(dAngCenCos + M_PI_2) + cPtCen.x);
        cPt2.y = LONG(dRadius*sin(dAngCenSin + M_PI_2) + cPtCen.y);
        m_ptVec.push_back(cPt2);
        dDistance = dDistance - dWidth;
        if(dDistance < dWidth)
        {
            break;
        }
        cPt3.x = LONG(dRadius*cos(dAngCenCos + M_PI) + cPtCen.x);
        cPt3.y = LONG(dRadius*sin(dAngCenSin + M_PI) + cPtCen.y);
        m_ptVec.push_back(cPt3);
        cPt4.x = LONG(dRadius*cos(dAngCenCos + M_PI_2 * 3) + cPtCen.x);
        cPt4.y = LONG(dRadius*sin(dAngCenSin + M_PI_2 * 3) + cPtCen.y);
        m_ptVec.push_back(cPt4);
    }
    #pragma endregion
    //����ɡA���ʨ줤���I�b�U��
    if(!m_bIsStop)
    {
        MO_Do3DLineMove(0, 0, (lZ - lZBackDistance) - lZ, lWorkVelociy, lAcceleration, lInitVelociy);//Z�b��^
        PreventMoveError();//����b�d�X��
    }
    for(rptIter = m_ptVec.rbegin(); rptIter != m_ptVec.rend(); rptIter++)
    {
        lNowX = MO_ReadLogicPosition(0);
        lNowY = MO_ReadLogicPosition(1);
        if(!m_bIsStop)
        {
            MO_Do3DLineMove((*rptIter).x - lNowX, (*rptIter).y - lNowY, 0, lWorkVelociy, lAcceleration, lInitVelociy);//����
            PreventMoveError();
        }
        if(rptIter == m_ptVec.rbegin())
        {
            if(!m_bIsStop)
            {
                lNowZ = MO_ReadLogicPosition(2);
                MO_Do3DLineMove(0, 0, lZ - lNowZ, lWorkVelociy, lAcceleration, lInitVelociy);//Z�b���U
                PreventMoveError();//����b�d�X��
            }
            PauseDoGlue();//�Ȱ���_���~��X��(m_bIsPause=0) �X��
        }
    }
    PauseStopGlue();//�Ȱ��ɰ����(m_bIsPause=1)
    MO_StopGumming();//����X��
    if(!m_bIsStop)
    {
        MO_Do3DLineMove(0, 0, (lZ - lZBackDistance) - lZ, lWorkVelociy, lAcceleration, lInitVelociy);//Z�b��^
        PreventMoveError();//����b�d�X��
    }
#endif
}
/*����---��R�κA(���A7������۶�R.�Ѥ��ӥ~)
*��J(�_�l�Ix1,y1,�����Ix2,y2,�e��,�X�ʳt��,�[�t��,��t��)
*/
void CAction::AttachFillType7_1(LONG lX1, LONG lY1, LONG lCenX, LONG lCenY, LONG lZ, LONG lZBackDistance, LONG lWidth, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
#ifdef MOVE
    #pragma region ****�ꫬ���ۥ\��****
    DOUBLE dRadius = 0, dWidth = 0, dAng0 = 0, dAng1 = 0, dAng2 = 0;
    BOOL bRev = 0;//0�f��/1����
    LONG lLineClose = 0, lXClose = 0, lYClose = 0, lDistance = 0;
    LONG lNowX = 0, lNowY = 0, lNowZ = 0;
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
    if(dRadius == 0)
    {
        return;
    }
    lDistance = LONG(dRadius);
    dWidth = lWidth * 1000;
    dAng1 = acos((cPt1.x - cPtCen1.x) / dRadius);
    dAng2 = asin((cPt1.y - cPtCen1.y) / dRadius);
    cPt2.x = LONG(dRadius*cos(dAng1 + M_PI) + cPtCen1.x);
    cPt2.y = LONG(dRadius*sin(dAng2 + M_PI) + cPtCen1.y);
    m_ptVec.push_back(cPt1);
    m_ptVec.push_back(cPt2);
    lDistance = LONG(lDistance - dWidth);
    LineGetToPoint(lXClose, lYClose, cPt1.x, cPt1.y, cPtCen1.x, cPtCen1.y,
                   lDistance);
    cPt3.x = lXClose;
    cPt3.y = lYClose;
    cPtCen2.x = (cPt3.x + cPt2.x) / 2;
    cPtCen2.y = (cPt3.y + cPt2.y) / 2;//�U�b����
    iData = (int)dRadius % (int)dWidth;
    lDistance = LONG(lDistance + dWidth);
    while(1)
    {
        lDistance = LONG(lDistance - dWidth);
        if(lDistance < dWidth)
        {
            break;
        }
        LineGetToPoint(lXClose, lYClose, cPt1.x, cPt1.y, cPtCen1.x, cPtCen1.y,
                       lDistance);
        cPt3.x = lXClose;
        cPt3.y = lYClose;
        m_ptVec.push_back(cPt3);
        cPt4.x = LONG(lDistance*cos(dAng1 + M_PI) + cPtCen1.x);
        cPt4.y = LONG(lDistance*sin(dAng2 + M_PI) + cPtCen1.y);
        m_ptVec.push_back(cPt4);
    }
    #pragma endregion
    //����ɡA���ʨ줤���I�b�U��
    if(!m_bIsStop)
    {
        MO_Do3DLineMove(lCenX - MO_ReadLogicPosition(0), lCenY - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration,
                        lInitVelociy);//���u���ʦܶ��
        PreventMoveError();
    }
    if(!m_bIsStop)
    {
        MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration,
                        lInitVelociy);//Z�b���U
        PreventMoveError();//����b�d�X��
    }

    std::vector<DATA_2MOVE> DATA_2DO;
    DATA_2DO.clear();
    for(rptIter = m_ptVec.rbegin(); rptIter != m_ptVec.rend(); rptIter++)
    {
        if((iData != 0) && (iOdd == 0))
        {
            MCO_Do2dDataLine((*rptIter).x, (*rptIter).y, DATA_2DO);
        }
        else if((iData == 0) && (iOdd == 0))
        {
            MCO_Do2dDataCir((*rptIter).x, (*rptIter).y, cPtCen2.x, cPtCen2.y, bRev, DATA_2DO);
        }
        else
        {
            if(iOdd % 2 == 0)
            {
                MCO_Do2dDataCir((*rptIter).x, (*rptIter).y, cPtCen2.x, cPtCen2.y, bRev, DATA_2DO);
            }
            else
            {
                MCO_Do2dDataCir((*rptIter).x, (*rptIter).y, lCenX, lCenY, bRev, DATA_2DO);
            }
        }
        iOdd++;
    }
    LA_AbsToOppo2Move(DATA_2DO);
    PauseDoGlue();//�Ȱ���_���~��X��(m_bIsPause=0) �X��
    for(UINT i = 0; i < DATA_2DO.size(); i++)
    {
        DATA_2Do[i] = DATA_2DO.at(i);
    }
    MO_DO2Curve(DATA_2Do, DATA_2DO.size(), lWorkVelociy);
    PreventMoveError();//�����X�ʿ��~
    Sleep(200);
    DATA_2DO.clear();

    PauseStopGlue();//�Ȱ��ɰ����(m_bIsPause=1)
    MO_StopGumming();//����X��
    if(!m_bIsStop)
    {
        MO_Do3DLineMove(0, 0, (lZ - lZBackDistance) - lZ, lWorkVelociy, lAcceleration,
                        lInitVelociy);//Z�b��^
        PreventMoveError();//����b�d�X��
    }
#endif
}