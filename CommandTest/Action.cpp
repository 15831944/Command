/*
*�ɮצW��:Action.cpp(3D��)
*���e²�z:�B�ʩR�OAPI�A�ԲӰѼƽЬd��excel
*�Iauthor �@�̦W��:R
*�Idata ��s���:2016/09/26
*@��s���e�T�b��b�s��t��&�p�gAPI*/
#include "stdafx.h"
#include "Action.h"
#define LA_SCANEND -99999
BOOL CAction::g_ZtimeOutGlueSet = FALSE;
BOOL CAction::g_YtimeOutGlueSet = FALSE;
/***********************************************************
**                                                        **
**          �B�ʼҲ�-�B�ʫ��O�R�O (�����ʧ@�P�_)             **
**                                                        **
************************************************************/
CAction::CAction()
{
	// �ݫŧi��l��
	pAction = this;
	g_bIsPause = FALSE;//�Ȱ�
	g_bIsStop = FALSE;//����
	g_bIsGetLAend = FALSE;//���y�����_
	g_bIsDispend = TRUE;//�I�����}
	g_iNumberGluePort = 1;//�ϥΤ@���I����
	g_interruptLock = FALSE;//���_��
	g_OffSetLaserX = 0;//�p�g�����q��l��X
	g_OffSetLaserY = 0;//�p�g�����q��l��Y
	g_OffSetLaserZ = 0;//�p�g�����q��l��Z
	g_HeightLaserZero = 0;//�p�g��Z�b�k�s�����ᰪ��(���y����)   
	g_LaserCnt = 0;//�p�g�u�q�p�ƾ�(�ϥΦ���)
	g_OffSetScan = 500;//�p�g�첾���v
	g_LaserAverage = FALSE; //�p�g����(1�ϥ�/0���ϥ�)
	g_LaserAveBuffZ = 0; // �p�g�Υ����Ȧs��(�����mz��)
    m_hComm = NULL;
#ifdef MOVE
	LA_m_ptVec2D.clear();
	LA_m_ptVec.clear();
#endif
}
CAction::~CAction()
{
}
/******************************************************�B��API**********************************************************/
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
	LONG lZBackDistance, BOOL bZDisType, LONG lZdistance, LONG lHighVelocity, LONG lLowVelocity, LONG lWorkVelociy,
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
	if (lHighVelocity == 0)
	{
		lHighVelocity = lWorkVelociy;
	}
	if (lLowVelocity == 0)
	{
		lLowVelocity = lWorkVelociy;
	}
	if (!bZDisType) //�����m
	{
		if (lZBackDistance > lZ)
		{
			lZBackDistance = lZ;
		}
		lZBackDistance = abs(lZBackDistance - lZ);
	}
	if (lZBackDistance>lZ)
	{
		lZBackDistance = lZ;
	}
	if (!g_bIsStop)
	{
		MO_Do3DLineMove(lX - lNowX, lY - lNowY, 0, lWorkVelociy, lAcceleration, lInitVelociy);//���ʨ��I���I
		PreventMoveError();//����b�d�X��
	}
	if (!g_bIsStop)
	{
		MO_Do3DLineMove(0, 0, lZ - lNowZ, lWorkVelociy, lAcceleration,lInitVelociy);//���ʨ��I���I
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
			DoGlue(lDoTime, lDelayStopTime);//�����I��
		}
	}
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
        if (lZBackDistance == 0)
        {
            lZdistance = lZBackDistance;
        }
		if (lZdistance>lZBackDistance && lZBackDistance != 0)
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
		MO_Do3DLineMove(lX - MO_ReadLogicPosition(0), lY - MO_ReadLogicPosition(1),0, lWorkVelociy, lAcceleration, lInitVelociy);//x,y,z�b����
		PreventMoveError();//����b�d�X��
	}
	if (!g_bIsStop)
	{
		MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//x,y,z�b����
		PreventMoveError();//����b�d�X��
	}
	if (!g_bIsStop && g_bIsDispend == 1)
	{
		MO_GummingSet(g_iNumberGluePort, 0);//�(���d)
	}
	MO_Timer(0, 0, lStartDelayTime * 1000);
	MO_Timer(1, 0, lStartDelayTime * 1000);//�u�q�I���]�w---(1)���ʫe����(�b�u�q�}�l�I�W)
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
	PauseDoGlue();//�Ȱ���_���~��X��(g_bIsPause=0) �X��
	if (!g_bIsStop)
	{
		MO_Do3DLineMove(lX - MO_ReadLogicPosition(0), lY - MO_ReadLogicPosition(1), lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);// x,y�b����
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
	if(!bZDisType) //�����m
	{
		if (lZBackDistance > lZ)
		{
			lZBackDistance = lZ;
		}
		lZBackDistance = abs(lZBackDistance - lZ);
	}
	LONG lNowX = 0, lNowY = 0, lNowZ = 0, LineLength=0;
	LONG lLineClose = 0, lXClose = 0, lYClose = 0;
	PauseDoGlue();//�Ȱ���_���~��X��(g_bIsPause=0)
	lNowX = MO_ReadLogicPosition(0);
	lNowY = MO_ReadLogicPosition(1);
	lNowZ = MO_ReadLogicPosition(2);
	LineLength = LONG(sqrt(DOUBLE(pow(lX - lNowX, 2) + pow(lY - lNowY, 2))));
	if (lCloseDistance != 0 && lCloseDistance > LineLength)
	{
		lCloseDistance = LineLength;
	}
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
		MO_StopGumming();//����X��
		return;
	}
	else
	{
		if (lCloseDistance == 0) //�u�q�I���]�w---(5)�����Z��
		{
			if (!g_bIsStop)
			{
				MO_Do3DLineMove(lX - MO_ReadLogicPosition(0), lY - MO_ReadLogicPosition(1), lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);// z�b���U��
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
			MO_StopGumming();//����X��
		}
		else
		{
			lLineClose = lCloseDistance;
			LineGetToPoint(lXClose, lYClose, lNowX, lNowY, lX, lY, lLineClose);
			if (!g_bIsStop)
			{
				MO_Do3DLineMove(lXClose - MO_ReadLogicPosition(0), lYClose - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration, lInitVelociy);//�u�q�I���]�w---(5)�����Z��
				PreventMoveError();//����b�d�X��
			}
			MO_StopGumming();//����X��
			if (!g_bIsStop)
			{
				MO_Do3DLineMove(lX - MO_ReadLogicPosition(0), lY - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration, lInitVelociy);// z�b���U��
				PreventMoveError();//����b�d�X��
			}
		}
	}

	GelatinizeBack(iType, lX, lY, lZ, lNowX, lNowY, lDistance, lHigh, lZBackDistance, lLowVelocity, lHighVelocity, lAcceleration, lInitVelociy);//��^�]�w
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
*��J(�u�q�����I�B�u�q�I���]�w�B�[�t�סB�u�t�סBZ�b�u�@���׳]�w�B�I�������]�w�B��^�]�w�B�t�ΰѼơB�@�뵲���I�ϥ�)
*����Z�b�^�ɫ��A1
*/
void CAction::DecideLineEndMove(LONG lCloseOffDelayTime,
	LONG lCloseONDelayTime, LONG lZBackDistance, BOOL bZDisType, LONG lHighVelocity, LONG lDistance, LONG lHigh,
	LONG lLowVelocity, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy,BOOL bGeneral)
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
	LONG lNowX = 0, lNowY = 0, lNowZ = 0;
	LONG lLineClose = 0, lXClose = 0, lYClose = 0;
	if (!bGeneral)
	{
		PauseDoGlue();//�Ȱ���_���~��X��(g_bIsPause=0)
	}
	lNowX = MO_ReadLogicPosition(0);
	lNowY = MO_ReadLogicPosition(1);
	lNowZ = MO_ReadLogicPosition(2);
	if (!bZDisType) //�����m
	{
		if (lZBackDistance > lNowZ)
		{
			lZBackDistance = lNowZ;
		}
		lZBackDistance = abs(lZBackDistance - lNowZ);
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
	if (!bGeneral || MO_ReadGumming())
	{
		MO_StopGumming();//����X��
	}
	GelatinizeBack(1, lNowX, lNowY, lNowZ, lNowX, lNowY, lDistance, lHigh, lZBackDistance, lLowVelocity, lHighVelocity, lAcceleration, lInitVelociy);//��^�]�w
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
void CAction::DecideLineSToP(LONG lX, LONG lY, LONG lZ, LONG lX2, LONG lY2, LONG lZ2,
	LONG lStartDelayTime, LONG lStartDistance, LONG lMidDelayTime, 
	LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
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
	LONG lXClose = 0, lYClose = 0, lZClose = 0, lLineClose = 0;
	LONG lBuffX = 0, lBuffY = 0;
	LONG lTime = 0;
	BOOL bSame = 0;
	lNowX = MO_ReadLogicPosition(0);
	lNowY = MO_ReadLogicPosition(1);
	lLineClose = lStartDistance;
	LineGetToPoint(lXClose, lYClose, lZClose, lX2, lY2, lX, lY, lZ2, lZ, lLineClose);
	lBuffX = (-(lXClose - lX)) + lX;
	lBuffY = (-(lYClose - lY)) + lY;
	if (lStartDelayTime > 0 && lStartDistance > 0) //(���̳����ȮɥH"���ʫe����"�u��)
	{
		bSame = 1;
	}
	else
	{
		bSame = 0;
	}
	if (bSame == 1 || lStartDistance == 0)
	{
		if (!g_bIsStop)
		{
			MO_Do3DLineMove(lX - MO_ReadLogicPosition(0), lY - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration, lInitVelociy);//x,y,z�b����
			PreventMoveError();//����b�d�X��
		}
		if (!g_bIsStop)
		{
			MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//x,y,z�b����
			PreventMoveError();//����b�d�X��
		}
        if (!g_bIsStop)
        {
		if (!g_bIsStop && g_bIsDispend == 1)
		{
			MO_GummingSet(g_iNumberGluePort, 0);//�(���d)
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
    }
		if (!g_bIsStop)
		{
			MO_Do3DLineMove(lX2 - MO_ReadLogicPosition(0), lY2 - MO_ReadLogicPosition(1), lZ2 - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//x,y,z�b(���ʨ��I2)
			PreventMoveError();//����b�d�X��
		}
	}
	else
	{
		if (!g_bIsStop)
		{
			MO_Do3DLineMove(lBuffX - MO_ReadLogicPosition(0), lBuffY - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration, lInitVelociy);//x,y�b
			PreventMoveError();//����b�d�X��
		}
		if (!g_bIsStop)
		{
			MO_Do3DLineMove(0, 0, lZClose - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//x,y�b
			PreventMoveError();//����b�d�X��
		}
		if (!g_bIsStop)
		{
			MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//z�b���U��
			PreventMoveError();//����b�d�X��
		}
		lTime = CalPreglue(lStartDistance, lWorkVelociy, lAcceleration, lInitVelociy);
		/*======�p�ɾ���Ĳ�o���_����X���A�ϥ�y���_����================*/
		if (!g_bIsStop)
		{
			CAction::g_YtimeOutGlueSet = TRUE;
			MO_TimerSetIntter(lTime, 0);//�p�ɨ���ܰ����
		}
		if (!g_bIsStop)
		{
			MO_Do3DLineMove(lX2 - MO_ReadLogicPosition(0), lY2 - MO_ReadLogicPosition(1), lZ2 - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//x,y�b(���ʨ��I2)
			PreventMoveError();//����b�d�X��
		}
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
void CAction::DecideLineSToE(LONG lX, LONG lY, LONG lZ, LONG lX2, LONG lY2, LONG lZ2,
	LONG lStartDelayTime, LONG lStartDistance, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime, 
	LONG lZBackDistance, BOOL bZDisType, LONG lHighVelocity, LONG lDistance, LONG lHigh, 
	LONG lLowVelocity, int iType, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
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
	LONG lNowZ = 0;
	LONG lXClose = 0, lYClose = 0, lZClose = 0, lLineClose = 0;
	LONG lBuffX = 0, lBuffY = 0, LineLength=0;
	LONG lTime = 0;
	BOOL bSame = 0;
	lNowZ = MO_ReadLogicPosition(2);
	lLineClose = lStartDistance;
	LineGetToPoint(lXClose, lYClose, lZClose, lX2, lY2, lX, lY, lZ2, lZ, lLineClose);
	lBuffX = (-(lXClose - lX)) + lX;
	lBuffY = (-(lYClose - lY)) + lY;
	LineLength = LONG(sqrt(DOUBLE(pow(lX2 - lX, 2) + pow(lY2 - lY, 2))));
	if (lCloseDistance != 0 && lCloseDistance > LineLength)
	{
		lCloseDistance = LineLength;
	}
	if (!bZDisType)//�����m
	{
		if (lZBackDistance > lZ)
		{
			lZBackDistance = lZ;
		}
		lZBackDistance = abs(lZBackDistance - lZ);
	}
	if (lHighVelocity == 0)
	{
		lHighVelocity = lWorkVelociy;
	}
	if (lLowVelocity == 0)
	{
		lLowVelocity = lWorkVelociy;
	}
	if (lStartDelayTime > 0 && lStartDistance > 0) //(���̳����ȮɥH"���ʫe����"�u��)
	{
		bSame = 1;
	}
	else
	{
		bSame = 0;
	}
	if (bSame == 1 || lStartDistance == 0)
	{
		if (!g_bIsStop)
		{
			MO_Do3DLineMove(lX - MO_ReadLogicPosition(0), lY - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration, lInitVelociy);//x,y�b����
			PreventMoveError();//����b�d�X��
		}
		if (!g_bIsStop)
		{
			MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//x,y�b����
			PreventMoveError();//����b�d�X��
		}
		if (!g_bIsStop && g_bIsDispend == 1)
		{
			MO_GummingSet(g_iNumberGluePort, 0);//�(���d)
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
			if (!g_bIsStop)
			{
				MO_Do3DLineMove(lX2 - MO_ReadLogicPosition(0), lY2 - MO_ReadLogicPosition(1), lZ2 - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//x,y�b����(2)
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
			lLineClose = lCloseDistance;
			LineGetToPoint(lXClose, lYClose, lZClose, lX, lY, lX2, lY2, lZ, lZ2, lLineClose);
			if (!g_bIsStop)
			{
				MO_Do3DLineMove(lXClose - MO_ReadLogicPosition(0), lYClose - MO_ReadLogicPosition(1), lZClose - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//�u�q�I���]�w---(5)�����Z��
				PreventMoveError();//����b�d�X��
			}
			MO_StopGumming();//����X��
			if (!g_bIsStop)
			{
				MO_Do3DLineMove(lX2 - MO_ReadLogicPosition(0), lY2 - MO_ReadLogicPosition(1), lZ2- MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//x,y,Z�b����
				PreventMoveError();//����b�d�X��
			}
		}
	}
	else
	{
		if (!g_bIsStop)
		{
			MO_Do3DLineMove(lBuffX - MO_ReadLogicPosition(0), lBuffY - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration, lInitVelociy);//x,y,Z�b����
			PreventMoveError();//����b�d�X��
		}
		if (!g_bIsStop)
		{
			MO_Do3DLineMove(0, 0, lZClose - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//x,y,Z�b����
			PreventMoveError();//����b�d�X��
		}
		lTime = CalPreglue(lStartDistance, lWorkVelociy, lAcceleration, lInitVelociy);//�p�ⴣ�e�Z���X���ɶ�
		/*======�p�ɾ���Ĳ�o���_����X���A�ϥ�y���_����================*/
		if (!g_bIsStop)
		{
			CAction::g_YtimeOutGlueSet = TRUE;
			MO_TimerSetIntter(lTime, 0);//�p�ɨ���ܰ����
		}
		if (lCloseDistance == 0)//�u�q�I���]�w---(5)�����Z��
		{
			if (!g_bIsStop)
			{
				MO_Do3DLineMove(lX2 - MO_ReadLogicPosition(0), lY2 - MO_ReadLogicPosition(1), lZ2 - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//x,y,Z�b����
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
			lLineClose = lCloseDistance;
			LineGetToPoint(lXClose, lYClose, lZClose, lX, lY, lX2, lY2, lZ, lZ2, lLineClose);
			if (!g_bIsStop)
			{
				MO_Do3DLineMove(lXClose - MO_ReadLogicPosition(0), lYClose - MO_ReadLogicPosition(1), lZClose - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);// �u�q�I���]�w-- - (5)�����Z��
				PreventMoveError();//����b�d�X��
			}
			MO_StopGumming();//����X��
			if (!g_bIsStop)
			{
				MO_Do3DLineMove(lX2 - MO_ReadLogicPosition(0), lY2 - MO_ReadLogicPosition(1), lZ2 - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//x,y�b����
				PreventMoveError();//����b�d�X��
			}
		}

	}
	MO_StopGumming();//����X��
	GelatinizeBack(iType, lX2, lY2, lZ2, lX, lY, lDistance, lHigh, lZBackDistance, lLowVelocity, lHighVelocity, lAcceleration, lInitVelociy);//��^�]�w
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
	PauseDoGlue();//�Ȱ���_���~��X��(g_bIsPause=0)
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
	PauseDoGlue();//�Ȱ���_���~��X��(g_bIsPause=0)
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
	LONG lR = 0, LineLength = 0;//�_���I�P�b�|
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
	lR = LONG(sqrt(pow(lNowX - lCircleX, 2) + pow(lNowY - lCircleY, 2)));//�b�|
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
		if (lZBackDistance > lNowZ)
		{
			lZBackDistance = lNowZ;
		}
		lZBackDistance = abs(lZBackDistance - lNowZ);
	}
	PauseDoGlue();//�Ȱ���_���~��X��(g_bIsPause=0)
	if ((lNowX >= lX3 - 5) && (lNowX <= lX3 + 5) && (lNowY >= lY3 - 5) && (lNowY <= lY3 + 5)) //��ܵ����I�b�_�l�I�W
	{
		LineLength = LONG((2 * M_PI*lR));
		if (lCloseDistance != 0 && lCloseDistance > LineLength)
		{
			lCloseDistance = LineLength;
		}
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
			ArcGetToPoint(lEndX, lEndY, lCloseDistance, lX3, lY3, lCircleX, lCircleY, lR, bRev);//��X�_���I
			lCloseDistance = LONG((2 * M_PI*lR)) - lCloseDistance;
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
		LineLength = LONG(sqrt(DOUBLE(pow(lX3 - lX1, 2) + pow(lY3 - lY1, 2))));
		if (lCloseDistance != 0 && lCloseDistance > LineLength)
		{
			lCloseDistance = LineLength;
		}
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
	lR = LONG(sqrt(pow(lNowX - lCircleX, 2) + pow(lNowY - lCircleY, 2)));//�b�|
	dAngl = AngleCount(lCircleX, lCircleY, lNowX, lNowY, lX2, lY2, bRev);//����
	lCrev = LONG(2 * lR * M_PI * dAngl / 360.0);
	if (lCloseDistance != 0 && lCloseDistance > lCrev)
	{
		lCloseDistance = lCrev;
	}
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
		if (lZBackDistance > lNowZ)
		{
			lZBackDistance = lNowZ;
		}
		lZBackDistance = abs(lZBackDistance - lNowZ);
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
		ArcGetToPoint(lEndX, lEndY, lCloseDistance, lX2, lY2, lCircleX, lCircleY, lR, bRev);//��X�_���I
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
	                MO_StopGumming();//����X��
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
	if (!g_bIsStop)
	{
		MO_Do3DLineMove(lX - MO_ReadLogicPosition(0), lY - MO_ReadLogicPosition(1), lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//x,y�b����
		PreventMoveError();//����b�d�X��
	}
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
	if (!g_bIsStop)
	{
		MO_Do3DLineMove(lX - MO_ReadLogicPosition(0), lY - MO_ReadLogicPosition(1), lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//x,y�b����
		PreventMoveError();//����b�d�X��
	}
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
void CAction::DecideParkPoint(LONG lX, LONG lY, LONG lZ, LONG lTimeGlue, LONG lWaitTime, LONG lStayTime, LONG lZBackDistance, BOOL bZDisType, LONG lZdistance, LONG lHighVelocity, LONG lLowVelocity, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
	/*���n�I(x�y�СAy�y�СAz�y�СA�ƽ��ɶ��A�����ᵥ�ݮɶ��A)
	LONG lX, LONG lY, LONG lZ,LONG lTimeGlue,LONG lWaitTime
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
	if (lHighVelocity == 0)
	{
		lHighVelocity = lWorkVelociy;
	}
	if (lLowVelocity == 0)
	{
		lLowVelocity = lWorkVelociy;
	}
	if (!bZDisType) //�����m
	{
		if (lZBackDistance > lZ)
		{
			lZBackDistance = lZ;
		}
		lZBackDistance = abs(lZBackDistance - lZ);
	}

	if (!g_bIsStop)
	{
		MO_Do3DLineMove(lX - MO_ReadLogicPosition(0), lY - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration, lInitVelociy);//x,y�b����
		PreventMoveError();//����b�d�X��
	}
	if (!g_bIsStop)
	{
		MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//z�b����
		PreventMoveError();//����b�d�X��
	}
	MO_Timer(0, 0, lWaitTime * 1000);
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
	if (lTimeGlue == 0)
	{
		Sleep(1);
	}
	else
	{
		DoGlue(lTimeGlue, lStayTime);//����ƽ�
	}
	Sleep(10);//����{���X��
			  //�ƽ����᪺�^�ɰʧ@
	if (!MO_ReadGumming())
	{
		if (lZdistance == 0)
		{
			if (!g_bIsStop)
			{
				MO_Do3DLineMove(0, 0, (lZ - lZBackDistance) - lZ, lWorkVelociy, lAcceleration,
					lInitVelociy);//�I�������]�m
				PreventMoveError();//����b�d�X��
			}
		}
		else
		{
			if (lZdistance>lZBackDistance && lZBackDistance != 0)
			{
				lZdistance = lZBackDistance;
				MO_Do3DLineMove(0, 0, (lZ - lZBackDistance) - lZ, lHighVelocity, lAcceleration,
					lInitVelociy);//�I�������]�m
				PreventMoveError();//����b�d�X��
			}
			else
			{
				if (!g_bIsStop)
				{
					MO_Do3DLineMove(0, 0, (lZ - lZdistance) - lZ, lLowVelocity, lAcceleration,
						lInitVelociy);//�I�������]�m
					PreventMoveError();//����b�d�X��
				}
				if (!g_bIsStop)
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
/*
*���I���k�ʧ@
*��J(LONG�t��1�ALONG�t��2�ALONG���w�b(0~7)�ALONG�����q)
*/
void CAction::DecideInitializationMachine(LONG lSpeed1, LONG lSpeed2, LONG lAxis, LONG lMoveX, LONG lMoveY, LONG lMoveZ)
{
#ifdef MOVE
	MO_SetHardLim(lAxis, 1);
	//TODO::�n�鷥���n�O�o��
	MO_SetSoftLim(lAxis, 0);
	MO_FinishGumming();
	if (!g_bIsStop)
	{
		MO_MoveToHome(lSpeed1, lSpeed2, lAxis, lMoveX, lMoveY, lMoveZ);
		PreventMoveError();//����b�d�X��
	}
	MO_SetSoftLim(lAxis, 1);
#endif
}
/*
*��R�ʧ@
*��J(�u�q�}�l�A�u�q�����AZ�b�u�@���׳]�w�A��R�R�O�A�t�ΰѼ�)
*/
void CAction::DecideFill(LONG lX1, LONG lY1, LONG lZ1, LONG lX2, LONG lY2, LONG lZ2,
	LONG lZBackDistance, BOOL bZDisType, int iType,LONG lWidth, LONG lWidth2,
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
	LONG lNowX = 0, lNowY = 0, lNowZ = 0;
	lNowX = MO_ReadLogicPosition(0);
	lNowY = MO_ReadLogicPosition(1);
	lNowZ = MO_ReadLogicPosition(2);
	if (lStartDelayTime > 0 && lStartDistance > 0)
	{
		lStartDistance = 0;
	}
	if (lCloseDistance > 0 && lCloseONDelayTime > 0)
	{
		lCloseONDelayTime = 0;
	}


	if (!bZDisType) //�����m
	{
		if (lZBackDistance > lZ1)
		{
			lZBackDistance = lZ1;
		}
		lZBackDistance = abs(lZBackDistance - lZ1);
	}
	if (lZBackDistance > lZ1)
	{
		lZBackDistance = lZ1;
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
	lNowZ = MO_ReadLogicPosition(2);
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
	if (!g_bIsStop)
	{
		MO_Do3DLineMove(lX - MO_ReadLogicPosition(0), lY - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration, lInitVelociy);//x,y�b����
		PreventMoveError();//����b�d�X��
	}
	if (!g_bIsStop)
	{
		MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//z�b����
		PreventMoveError();//����b�d�X��
	}
#endif
}
/*���槹���^(0,0,0)��m*/
void CAction::BackGOZero(LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
#ifdef MOVE
	if (!g_bIsStop)
	{
		MO_Do3DLineMove(0, 0, 0 - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);
		PreventMoveError();
	}
	if (!g_bIsStop)
	{
		MO_Do3DLineMove(0 - MO_ReadLogicPosition(0), 0 - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration, lInitVelociy);
		PreventMoveError();
	}
#endif
}
//----------------------------------------------------
//�H���ϥ�API--�ϥΫe�Шϥέ��I�_�k�����q
//----------------------------------------------------
//�n��t����(x,y,z�w�]��-10)
void CAction::HMNegLim(LONG lX, LONG lY, LONG lZ)
{
#ifdef MOVE
	MO_SetSoftLim(7, 1);
	MO_SetCompSoft(1, -lX, -lY, -lZ);
#endif
}
//�n�饿����(x,y,z)
void CAction::HMPosLim(LONG lX, LONG lY, LONG lZ)
{
#ifdef MOVE
	MO_SetSoftLim(7, 1);
	MO_SetCompSoft(0, lX, lY, lZ);
#endif
}
/***********************************************************
**                                                        **
**          �B�ʼҲ�-�p�g3D���N���| (�����ʧ@�P�_)           **
**                                                        **
************************************************************/
/*
*�p�gB�I�]�w
*/
void CAction::LA_Butt_GoBPoint()
{
#ifdef LA
	LAS_GoBPoint(DATA_3ZERO_B);
#endif
}
/*
*�p�g��B�I
*/
void CAction::LA_Butt_GoLAtoBPoint()
{
#ifdef LA
	DATA_3ZERO_LA = LAS_GoLAtoBPoint(DATA_3ZERO_B);
	g_OffSetLaserX = DATA_3ZERO_LA.EndPX;
	g_OffSetLaserY = DATA_3ZERO_LA.EndPY;
	//TODO::�p�gOFFSET�ק�b�o
	//g_OffSetLaserX = 48436; 
	//g_OffSetLaserY = 0;

#endif
}
/*
*�p�g��l��
*/
void CAction::LA_SetInit()
{
#ifdef LA
#pragma region Omron Laser

    if (m_hComm != NULL)
    {
        AfxMessageBox(L"Already Opened!");
        return;
    }

    m_hComm = CreateFile(ComportNo, GENERIC_READ | GENERIC_WRITE, 0, NULL,
        OPEN_EXISTING, false ? FILE_FLAG_OVERLAPPED : 0, NULL);
    if (m_hComm == INVALID_HANDLE_VALUE)
    {
        // MessageBox(L"Open failed");
        return;
    }
    LAS_SetInit(&m_hComm);

#pragma endregion //Omron
	////LAS_SetInit();
	////TODO::�p�gOFFSET�ק�b�o
	////*************�ثe�g���ʧ@***************************************
	//g_OffSetLaserX = 40324;
	//g_OffSetLaserY = -647;
	//g_HeightLaserZero = 31079;//�p�g��Z�b�k�s�I����
	//g_OffSetLaserZ = 43274;//B�I��Z���ר�p�g�k�sZ���ת��첾��(+)
#endif
}
/*
*���I���y
*/
BOOL CAction::LA_Dot3D(LONG lX, LONG lY, LONG &lZ, LONG lWorkVelociy,
	LONG lAcceleration, LONG lInitVelociy)
{
#ifdef LA
	LONG lCalcData1;
	if (!g_bIsStop)
	{
		MO_Do3DLineMove(0, 0, g_HeightLaserZero - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);
		PreventMoveError();//����b�d�X�ʿ��~
	}
	if (!g_bIsStop)
	{
		MO_Do3DLineMove(lX - g_OffSetLaserX - MO_ReadLogicPosition(0), lY - g_OffSetLaserY - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration, lInitVelociy);
		PreventMoveError();//����b�d�X�ʿ��~
	}
	if (LAS_GetLaserData(lCalcData1))
	{
		if (lCalcData1 == LAS_LONGMIN)
		{
			lZ = MO_ReadLogicPosition(2);
			return FALSE;//�������(ffffff)
		}
		else
		{
			lZ = MO_ReadLogicPosition(2) - lCalcData1 + g_OffSetLaserZ;//30000���P���d��
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
	if (LAS_SetZero())//�����k�s�I
	{
		if (!g_bIsStop)
		{
			MO_Do3DLineMove(0, 0, -LAS_MEASURE_RANGE, 20000, 100000, 1000);//�W��3cm�A�����p�g�k�s
			PreventMoveError();//����b�d�X��
		}
		if (LAS_SetZero())
		{
			g_HeightLaserZero = MO_ReadLogicPosition(2);//�p�g��Z�b�k�s�I����
			g_OffSetLaserZ = DATA_3ZERO_B.EndPZ - g_HeightLaserZero;//B�I��Z���ר�p�g�k�sZ���ת��첾��(+)
			return TRUE;//�p�g�k�s����
		}
		else
		{
			AfxMessageBox(L"�k�s���פ����T");
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
	BOOL bType = 0;
	DATA_2MOVE DATA_2D;
	if (lX1 == 0 && lY1 == 0 && lX2 == 0 && lY2 == 0)
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
		csP1.Format(L"%d�A%d�A", lX1, lY1);
		csP2.Format(L"%d�A%d�A", lX2, lY2);
		csP3.Format(L"%d�A%d�A", lX3, lY3);
		csbuff = TRoundCCalculation(csP1, csP3, csP2);
		DATA_2D.CirCentP.x = CStringToLong(csbuff, 0);
		DATA_2D.CirCentP.y = CStringToLong(csbuff, 1);
		DATA_2D.CirRev = CStringToLong(csbuff, 2);//���o���(X�AY�ARev�A)
	}
	DATA_2D.EndP.x = lX3;
	DATA_2D.EndP.y = lY3;
	DATA_2D.Type = bType;
	DATA_2D.Speed = 0;
	LA_m_ptVec2D.push_back(DATA_2D);
#endif
}
/*��b�u�q�I*/
void CAction::LA_Do2dDataLine(LONG EndPX, LONG EndPY)
{
#ifdef MOVE
	DATA_2MOVE DATA_2D;
	DATA_2D.EndP.x = EndPX;
	DATA_2D.EndP.y = EndPY;
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
	if (LA_m_ptVec2D.empty())
	{
		return;
	}
	DATA_2MOVE DATA_2D;
	CString csbuff;
	CString csP1, csP2, csP3;

	csP1.Format(L"%d�A%d�A", LA_m_ptVec2D.back().EndP.x, LA_m_ptVec2D.back().EndP.y);
	csP2.Format(L"%d�A%d�A", ArcX, ArcY);
	csP3.Format(L"%d�A%d�A", EndPX, EndPY);
	csbuff = TRoundCCalculation(csP1, csP3, csP2);
	DATA_2D.Type = 1;//��
	DATA_2D.EndP.x = EndPX;
	DATA_2D.EndP.y = EndPY;
	DATA_2D.CirCentP.x = CStringToLong(csbuff, 0);
	DATA_2D.CirCentP.y = CStringToLong(csbuff, 1);
	DATA_2D.CirRev = CStringToLong(csbuff, 2);//���o���(X�AY�ARev�A)
	LA_m_ptVec2D.push_back(DATA_2D);
#endif
}
/*��b�u�q��*/
void CAction::LA_Do2dDataCircle(LONG EndPX, LONG EndPY, LONG CirP1X, LONG CirP1Y, LONG CirP2X, LONG CirP2Y)
{
#ifdef MOVE
	if (LA_m_ptVec2D.empty())
	{
		return;
	}
	DATA_2MOVE DATA_2D;
	CString csbuff;
	CString csP1, csP2, csP3;

	csP1.Format(L"%d�A%d�A", EndPX, EndPY);
	csP2.Format(L"%d�A%d�A", CirP1X, CirP1Y);
	csP3.Format(L"%d�A%d�A", CirP2X, CirP2Y);
	csbuff = TRoundCCalculation(csP1, csP3, csP2);
	DATA_2D.Type = 1;//��
	DATA_2D.EndP.x = EndPX;
	DATA_2D.EndP.y = EndPY;
	DATA_2D.CirCentP.x = CStringToLong(csbuff, 0);
	DATA_2D.CirCentP.y = CStringToLong(csbuff, 1);
	DATA_2D.CirRev = CStringToLong(csbuff, 2);//���o���(X�AY�ARev�A)
	LA_m_ptVec2D.push_back(DATA_2D);
#endif
}
//�s��u�q����--
void CAction::LA_Line2D(LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
#ifdef LA
#ifdef MOVE
	if (g_LaserAverage == FALSE)
	{
		g_LaserCnt++;
	}
	LA_AbsToOppo2Move(LA_m_ptVec2D);
	if (!g_bIsStop)
	{
		MO_Do3DLineMove(0, 0, g_HeightLaserZero - MO_ReadLogicPosition(2), lWorkVelociy,
			lAcceleration, lInitVelociy);
		PreventMoveError();//�_�l�I�ǳƲ���
	}
	if (!g_bIsStop)
	{
		MO_Do3DLineMove(LA_m_ptVec2D.at(0).EndP.x - g_OffSetLaserX,
			LA_m_ptVec2D.at(0).EndP.y- g_OffSetLaserY , 0, lWorkVelociy,
			lAcceleration, lInitVelociy);
		PreventMoveError();//�_�l�I�ǳƲ���
	}

    /*����Ĳ�o�ɡA�������y*/
    if (g_bIsStop)
    {
        return;
    }

	MO_InterruptCase(1, 2);
	MO_InterruptCase(1, 3);
    MO_Timer(0, 100000);//�]�w�p�ɾ�(0.1sĲ�o�@��)
	/*���J�}�l�I�@��*/
	LONG lCalcData1;

	if (LAS_GetLaserData(lCalcData1))
	{
		if (lCalcData1 == LAS_LONGMIN)
		{
			g_LaserErrCnt++;
		}
		else
		{
			DATA_3Do[0].EndPX = MO_ReadLogicPosition(0) + g_OffSetLaserX;
			DATA_3Do[0].EndPY = MO_ReadLogicPosition(1) + g_OffSetLaserY;
			DATA_3Do[0].EndPZ = MO_ReadLogicPosition(2) - lCalcData1 + g_OffSetLaserZ;//30000���P���d��
			if (g_LaserAverage == FALSE)
			{
				LA_m_ptVec.push_back(DATA_3Do[0]);
			}
			if (g_LaserCnt == 1 && g_LaserAverage == FALSE)
			{
				LA_m_iVecSP.push_back(g_LaserCnt);//main
			}
		}
	}
	for (UINT i = 1; i < LA_m_ptVec2D.size(); i++)
	{
		DATA_2Do[i-1] = LA_m_ptVec2D.at(i);
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
    /*����Ĳ�o�ɡA�������y*/
    if (g_bIsStop)
    {
        return;
    }

	if (iData <= 0)
	{
		return;
	}

	while (g_bIsGetLAend == FALSE);
	std::vector<DATA_3MOVE>::iterator LA_ptIter;//���N��
	std::vector<DATA_3MOVE>LA_Buff;//�u�q�ȧP�_
	LONG lNowZHigh = 0;
	LA_Buff.clear();
	if (bDoAll)
	{
		LA_Buff.assign(LA_m_ptVec.begin(), LA_m_ptVec.begin() + LA_m_iVecSP.at(1) - 1);
		lNowZHigh = g_HeightLaserZero - MO_ReadLogicPosition(2);//�۹��m
	}
	else
	{
		if ((UINT)iData > LA_m_iVecSP.size())
		{
			return;
		}
		else if (iData == 1)
		{
			LA_Buff.assign(LA_m_ptVec.begin(), LA_m_ptVec.begin() + LA_m_iVecSP.at(1) - 1);
			lNowZHigh = g_HeightLaserZero - MO_ReadLogicPosition(2);//�۹��m
		}
		else
		{
			LA_Buff.assign(LA_m_ptVec.begin() + LA_m_iVecSP.at(iData - 1), LA_m_ptVec.begin() + LA_m_iVecSP.at(iData) - 1);
			lNowZHigh = 0;//�۹��m
		}
	}
	if (!g_bIsStop)
	{
		MO_Do3DLineMove(0, 0, lNowZHigh, lWorkVelociy, lAcceleration,lInitVelociy);//���y���צ^�h
		PreventMoveError();//�_�l�I�ǳƲ���
	}
	LA_AbsToOppo3Move(LA_Buff);
	if (!g_bIsStop)
	{
		LA_ptIter = LA_Buff.begin();
		MO_Do3DLineMove(LA_ptIter->EndPX, LA_ptIter->EndPY, 0, lWorkVelociy,lAcceleration, lInitVelociy);//���ʰ_�l�I
		PreventMoveError();//�����X�ʿ��~
	}
	if (!g_bIsStop)
	{
		LA_ptIter = LA_Buff.begin();
		MO_Do3DLineMove(0, 0, LA_ptIter->EndPZ, lWorkVelociy, lAcceleration,lInitVelociy);//z�b�U����_�l�I����
		PreventMoveError();//�����X�ʿ��~
	}
	if (!g_bIsStop)
	{
		if (!g_bIsStop && g_bIsDispend == 1)
		{
			MO_GummingSet(1, 0);//�(���d)
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
	g_LaserCnt = 0; //�p�g�u�q�p�ƾ�(���y��)
#endif
}

//�p�g��������
void CAction::LA_AverageZ(LONG lStrX, LONG lStrY, LONG lEndX, LONG lEndY, LONG &lZ)
{
#ifdef LA
#ifdef MOVE
	g_LaserAverage = TRUE;
    g_LaserAveBuffZ = 0;
	LA_Do2dDataLine(lStrX, lStrY);
	LA_Do2dDataLine(lEndX, lEndY);
	LA_Line2D(10000, 10000, 2000);
	Sleep(200);
	lZ = g_LaserAveBuffZ;
    g_LaserAverage = FALSE;
#endif
#endif
}
//�ץ��[����s��u�q
void CAction::LA_CorrectVectorToDo(LONG  lWorkVelociy, LONG lAcceleration, LONG lInitVelociy, LONG RefX, LONG RefY, DOUBLE OffSetX, DOUBLE OffSetY, DOUBLE Andgle, DOUBLE CameraToTipOffsetX, DOUBLE CameraToTipOffsetY, BOOL Mode, LONG lSubOffsetX, LONG lSubOffsetY)
{
#ifdef MOVE
	std::vector<DATA_3MOVE>::iterator LA_ptIter;//���N��
	std::vector<DATA_3MOVE>LA_Buff;//�u�q�ȧP�_
	LONG lNowZHigh = 0;
	g_LaserCnt = 1;
	//while (g_bIsGetLAend == FALSE);
	LA_Buff.clear();

	LA_Buff.assign(LA_m_ptVec.begin(), LA_m_ptVec.begin() + LA_m_iVecSP.at(1) - 1);
	lNowZHigh = g_HeightLaserZero - MO_ReadLogicPosition(2);//�۹��m
	for (UINT i = 0; i < LA_Buff.size(); i++)
	{
		if (!(LA_Buff.at(i).EndPX == LA_SCANEND && LA_m_ptVec.at(i).EndPY == LA_SCANEND))
		{
			LA_CorrectLocation(LA_Buff.at(i).EndPX, LA_Buff.at(i).EndPY, RefX, RefY, OffSetX, OffSetY, Andgle, CameraToTipOffsetX, CameraToTipOffsetY, Mode, lSubOffsetX, lSubOffsetY);
		}
	}
	//�N�Ȧb��^LA_m_ptVec
	if (!g_bIsStop)
	{
		MO_Do3DLineMove(0, 0, lNowZHigh, lWorkVelociy, lAcceleration, lInitVelociy);//���y���צ^�h
		PreventMoveError();//�_�l�I�ǳƲ���
	}

	LA_AbsToOppo3Move(LA_Buff);

	if (!g_bIsStop)
	{
		LA_ptIter = LA_Buff.begin();
		MO_Do3DLineMove(LA_ptIter->EndPX, LA_ptIter->EndPY, 0, lWorkVelociy, lAcceleration, lInitVelociy);//���ʰ_�l�I
		PreventMoveError();//�����X�ʿ��~
	}
	if (!g_bIsStop)
	{
		LA_ptIter = LA_Buff.begin();
		MO_Do3DLineMove(0, 0, LA_ptIter->EndPZ, lWorkVelociy, lAcceleration, lInitVelociy);//z�b�U����_�l�I����
		PreventMoveError();//�����X�ʿ��~
	}
	if (!g_bIsStop)
	{
		if (!g_bIsStop && g_bIsDispend == 1)
		{
			MO_GummingSet(1, 0);//�(���d)
		}
		MO_DO3Curve(DATA_3Do, LA_Buff.size() - 1, lWorkVelociy);//�s�򴡸ɶ}�l
		PreventMoveError();//�����X�ʿ��~
	}
#endif
}
//��R��ܮ����̫�@�I�y��(EndX,EndY)
void CAction::Fill_EndPoint(LONG & lEndX, LONG & lEndY, LONG lX1, LONG lY1, LONG lZ1, LONG lX2, LONG lY2, LONG lZ2, int iType, LONG lWidth, LONG lWidth2)
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
	LONG lBufX = 0, lBufY = 0;
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
}

/*********************************************************�����************************************************************/
//3d���N���|�����
DWORD CAction::MoInterrupt(LPVOID param)
{
#ifdef MOVE
#ifdef PRINTF
	_cwprintf(L"%s\n", L"Interrupt");
#endif
	LONG RR1X, RR1Y, RR1Z, RR1U;
	MO_ReadEvent(&RR1X, &RR1Y, &RR1Z, &RR1U);
	if (RR1X & 0x0010)//�쥻��RR1X&0x0020 �X�ʶ}�l���_
	{
		g_LaserErrCnt = 0;//�p�ƪ�l��
		g_LaserNuCnt = 1;//�p�ƪ�l��
		//MO_Timer(0, 100000);//�]�w�p�ɾ�(0.1sĲ�o�@��)
		MO_Timer(1, 100000);//�Ұʭp�ɾ�
        ((CAction *)param)->g_getHeightFlag = TRUE;//�p�g�����X�СG���\����
		((CAction *)param)->g_bIsGetLAend = FALSE;//���y�|������
	}
    if ((RR1X & 0x0080) || (g_LaserErrCnt == 10))//�쥻����((RR1X&0x0040)|| (g_LaserErrCnt == 10)) �X�ʵ������_
    {
#ifdef LA
#ifdef PRINTF
        _cwprintf(L"%s\n", L"�X�ʵ������_!");
#endif
        ((CAction *)param)->g_getHeightFlag = FALSE;
        //((CAction *)param)->g_bIsGetLAend = TRUE;
        if ((g_LaserErrCnt >= 10))
        {
            AfxMessageBox(L"�p�g�I�Эץ�");
            return 0;
        }
        /*���J�����I�@��*/
        LONG lCalcData1;
        if (LAS_GetLaserData(lCalcData1))
        {
            if (lCalcData1 == LAS_LONGMIN)
            {
                g_LaserErrCnt++;
            }
            else//�p�g���쪺�Ȭ�(0~+30)
            {
                ((CAction *)param)->DATA_3Do[g_LaserNuCnt].EndPX = MO_ReadLogicPosition(0) + ((CAction *)param)->g_OffSetLaserX;
                ((CAction *)param)->DATA_3Do[g_LaserNuCnt].EndPY = MO_ReadLogicPosition(1) + ((CAction *)param)->g_OffSetLaserY;
                ((CAction *)param)->DATA_3Do[g_LaserNuCnt].EndPZ = MO_ReadLogicPosition(2) - lCalcData1 + ((CAction *)param)->g_OffSetLaserZ;//30000���P���d��
                if (((CAction *)param)->g_LaserAverage == TRUE)
                {
                    ((CAction *)param)->g_LaserAveBuffZ += ((CAction *)param)->DATA_3Do[g_LaserNuCnt].EndPZ;
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

        if (((CAction *)param)->g_LaserAverage == TRUE)
        {
            ((CAction *)param)->g_LaserAveBuffZ = LONG(round(((DOUBLE)((CAction *)param)->g_LaserAveBuffZ) / (g_LaserNuCnt-1)));
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
        ((CAction *)param)->g_bIsGetLAend = TRUE;// ���y����
    }
	if (RR1Y & 0x0200) //Y�p�ɾ����_(�X���P�_��)
	{
#ifdef PRINTF
		_cwprintf(L"y\n");
#endif
        if (!((CAction*)param)->g_bIsStop)
        {
            (CAction::g_YtimeOutGlueSet) ? MO_GummingSet() : MO_FinishGumming();
            CAction::g_YtimeOutGlueSet = FALSE;
        }			
	}
    if (RR1Z & 0x0200) //Z�p�ɾ����_
    {
#ifdef PRINTF
        _cwprintf(L"z\n");
#endif
        if (!((CAction*)param)->g_bIsStop)
        {
            (CAction::g_ZtimeOutGlueSet) ? MO_GummingSet() : MO_FinishGumming();
            CAction::g_ZtimeOutGlueSet = FALSE;
        } 
	}
	if (RR1U & 0x0200 && ((CAction *)param)->g_getHeightFlag) //U�p�ɾ����_
	{
#ifdef LA
		LONG lCalcData1;
		if (LAS_GetLaserData(lCalcData1))
		{
			if (lCalcData1 == LAS_LONGMIN)
			{
				g_LaserErrCnt++;
			}
			else
			{
				((CAction *)param)->DATA_3Do[g_LaserNuCnt].EndPX = MO_ReadLogicPosition(0) + ((CAction *)param)->g_OffSetLaserX - ((CAction *)param)->g_OffSetScan;
				((CAction *)param)->DATA_3Do[g_LaserNuCnt].EndPY = MO_ReadLogicPosition(1) + ((CAction *)param)->g_OffSetLaserY - ((CAction *)param)->g_OffSetScan;
				((CAction *)param)->DATA_3Do[g_LaserNuCnt].EndPZ = MO_ReadLogicPosition(2) - lCalcData1 + ((CAction *)param)->g_OffSetLaserZ;//30000���P���d��
				if (((CAction *)param)->g_LaserAverage == TRUE)
				{
					((CAction *)param)->g_LaserAveBuffZ += ((CAction *)param)->DATA_3Do[g_LaserNuCnt].EndPZ;
				}
				else
				{
					((CAction *)param)->LA_m_ptVec.push_back(((CAction *)param)->DATA_3Do[g_LaserNuCnt]);
#ifdef PRINTF
                    _cwprintf(L"%s\n", L"Get Point");
#endif
				}
				g_LaserNuCnt++;
			}
		}

#endif
	}
	return 0;
#endif
#ifndef MOVE
	return 0;
#endif
}
/******************************************************�ۦ�B�Ψ��**********************************************************/
/*
*����---�����I�ʧ@
*��J(�I�B�X�ʳt�סB�[�t�סB��t�סB�O�_�ϥΤ��_)
*/
void CAction::AttachPointMove(LONG lX, LONG lY, LONG lZ, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy, BOOL bIntt)
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
		MO_Do3DLineMoveInterr(lX - lNowX, lY - lNowY, lZ - lNowZ, lWorkVelociy,
			lAcceleration, lInitVelociy, MoInterrupt);//�����������_
		PreventMoveError();//����b�d�X��
	}
	else
	{
		if (!g_bIsStop)
		{
			MO_Do3DLineMove(lX - lNowX, lY - lNowY, lZ - lNowZ, lWorkVelociy, lAcceleration,
				lInitVelociy);
			PreventMoveError();//����b�d�X��
		}
	}
#endif 
}
/*
*�X��(�t�X������ϥ�)
*��J(�I�����}��or�I��������)��Ķ�᪺LONG��
*/
void CAction::DoGlue(LONG lTime, LONG lDelayTime, LPTHREAD_START_ROUTINE MoInterrupt)
{
#ifdef MOVE
	if (!g_bIsStop)
	{
		if (!g_bIsStop && g_bIsDispend == 1)
		{
			MO_GummingSet(g_iNumberGluePort, lTime * 1000);
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
	if (!g_bIsPause)  //�Ȱ���_���~��X��(g_bIsPause=0)
	{
		if ((g_bIsPause == 0) && (g_bIsDispend == 1))
		{
			MO_GummingSet(g_iNumberGluePort, 0);//�(���d)
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
	if (lLineStop>lR)
	{
		lBackXar = lXarUp;
		lBackYar = lYarUp;
	}
	else if (lXarEnd == 0 && lYarEnd == 0 && lXarUp == 0 && lYarUp == 0)
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
	if (lStopZar>lZarEnd)
	{
		lStopZar = lZarEnd;
	}            
	if (lStopZar>lBackZar)
	{
		lStopZar = lBackZar;
	}
	if (lBackZar>lZarEnd)
	{
		lBackZar = lZarEnd;
	}
	switch (iType) //0~5
	{
	case 0://�L�ʧ@
	{
		break;
	}
	case 1://z�b�W�ɨ�q�t
	{
		if (lStopZar == 0)
		{
			if (!g_bIsStop)
			{
				MO_Do3DLineMove(0, 0, (lZarEnd - lBackZar) - lZarEnd, lHighSpeed, lAcceleration, lInitSpeed);//�S����^���ץH���t��^�̰��I
				PreventMoveError();//����b�d�X��
			}
		}
		else
		{
			if (!g_bIsStop)
			{
				MO_Do3DLineMove(0, 0, (lZarEnd - lStopZar) - lZarEnd, lLowSpeed, lAcceleration, lInitSpeed);//�C�t
				PreventMoveError();//����b�d�X��
			}
			if (!g_bIsStop)
			{
				MO_Do3DLineMove(0, 0, (lZarEnd - lBackZar) - (lZarEnd - lStopZar), lHighSpeed, lAcceleration, lInitSpeed);//���t
				PreventMoveError();//����b�d�X��
			}
		}
		break;
	}
	case 2: //�V��T�b����(�C�t)�A�Az�b�W��(���t)
	{
		if (lStopZar == 0)
		{
			if (!g_bIsStop)
			{
				MO_Do3DLineMove(lBackXar - lNowX, lBackYar - lNowY, 0, lLowSpeed, lAcceleration, lInitSpeed);//�C�t
				PreventMoveError();//����b�d�X��

			}
			if (!g_bIsStop)
			{
				MO_Do3DLineMove(0, 0, (lZarEnd - lBackZar) - lZarEnd, lHighSpeed, lAcceleration, lInitSpeed);//���t
				PreventMoveError();//����b�d�X��
			}
		}
		else
		{
			if (!g_bIsStop)
			{
				MO_Do3DLineMove(lBackXar - lNowX, lBackYar - lNowY, (lZarEnd - lStopZar) - lZarEnd, lLowSpeed, lAcceleration, lInitSpeed);//�C�t
				PreventMoveError();//����b�d�X��
			}
			if (!g_bIsStop)
			{
				MO_Do3DLineMove(0, 0, (lZarEnd - lBackZar) - (lZarEnd - lStopZar), lHighSpeed, lAcceleration, lInitSpeed);//���t
				PreventMoveError();//����b�d�X��
			}
		}

		break;
	}
	case 3://�����V��Az�b���W��(�C�t)�Axy��b���ɦV�Ჾ��(���t)�A�Az�b�W��(���t)
	{
		if (lStopZar != 0)
		{
			if (!g_bIsStop)
			{
				MO_Do3DLineMove(0, 0, (lZarEnd - lStopZar) - lZarEnd, lLowSpeed, lAcceleration, lInitSpeed);//�C�tz�b�W��
				PreventMoveError();//����b�d�X��
			}
		}
		if (!g_bIsStop)
		{
			MO_Do3DLineMove(lBackXar - lNowX, lBackYar - lNowY, 0, lHighSpeed, lAcceleration, lInitSpeed);//�V�Ჾ�ʰ��t
			PreventMoveError();//����b�d�X��
		}
		if (!g_bIsStop)
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
		if (lStopZar == 0)
		{
			if (!g_bIsStop)
			{
				MO_Do3DLineMove(lBackXar - lNowX, lBackYar - lNowY, 0, lLowSpeed, lAcceleration, lInitSpeed);//�C�t
				PreventMoveError();//����b�d�X��
			}
			if (!g_bIsStop)
			{
				MO_Do3DLineMove(0, 0, (lZarEnd - lBackZar) - lZarEnd, lLowSpeed, lAcceleration, lInitSpeed);//���t
				PreventMoveError();//����b�d�X��
			}
		}
		else
		{
			if (!g_bIsStop)
			{
				MO_Do3DLineMove(lBackXar - lNowX, lBackYar - lNowY, (lZarEnd - lStopZar) - lZarEnd, lLowSpeed, lAcceleration, lInitSpeed);//�C�t
				PreventMoveError();//����b�d�X��
			}
			if (!g_bIsStop)
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

		if (lStopZar != 0)
		{
			if (!g_bIsStop)
			{
				MO_Do3DLineMove(0, 0, (lZarEnd - lStopZar) - lZarEnd, lLowSpeed, lAcceleration, lInitSpeed);//z�b�C�t�W��
				PreventMoveError();//����b�d�X��
			}
		}
		if (!g_bIsStop)
		{
			MO_Do3DLineMove(lBackXar - lNowX, lBackYar - lNowY, 0, lHighSpeed, lAcceleration, lInitSpeed);//��b�V�e����(���t)
			PreventMoveError();//����b�d�X��
		}
		if (!g_bIsStop)
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
	//DOUBLE dM, dB; //(X0,Y0)�_�l�I; (X1,Y1)�����I; Y0=M*X0+B �ײv����;L�w������
	//if ((lX1 - lX0) == 0)
	//{
	//	dM = 0;
	//}
	//else
	//{
	//	dM = ((float)lY1 - (float)lY0) / ((float)lX1 - (float)lX0);
	//}
	//dB = lY0 - (dM*lX0);
	//LONG la = lX1 - lX0, lb = lY1 - lY0;
	//if (la == 0)
	//{
	//	if (lY0 > lY1)
	//	{
	//		lYClose = lY1 + lLineClose;
	//		lXClose = lX1;
	//	}
	//	else
	//	{
	//		lYClose = lY1 - lLineClose;
	//		lXClose = lX1;
	//	}
	//}
	//else if (lb == 0)
	//{
	//	if (lX0 > lX1)
	//	{
	//		lXClose = lX1 + lLineClose;
	//		lYClose = lY1;
	//	}
	//	else
	//	{
	//		lXClose = lX1 - lLineClose;
	//		lYClose = lY1;
	//	}
	//}
	//else
	//{
	//	if (abs(la) > abs(lb))
	//	{
	//		if (la > 0)
	//		{
	//			lXClose = lX1 - lLineClose;
	//			lYClose = LONG((dM*lX1) + dB);
	//		}
	//		else
	//		{
	//			lXClose = lX1 + lLineClose;
	//			lYClose = LONG((dM*lX1) + dB);
	//		}
	//	}
	//	else if (abs(la) == abs(lb))
	//	{
	//		if (la > 0)
	//		{
	//			lXClose = LONG(lX1 - ((float)lLineClose / sqrt(2)));
	//			lYClose = LONG(lY1 - ((float)lLineClose / sqrt(2)));
	//		}
	//		else
	//		{
	//			lXClose = LONG(lX1 + ((float)lLineClose / sqrt(2)));
	//			lYClose = LONG(lY1 + ((float)lLineClose / sqrt(2)));
	//		}
	//	}
	//	else if (abs(la) < abs(lb))
	//	{
	//		if (lb > 0)
	//		{
	//			lYClose = lY1 - lLineClose;
	//			lXClose = LONG((lYClose - dB) / dM);
	//		}
	//		else
	//		{
	//			lYClose = lY1 + lLineClose;
	//			lXClose = LONG((lYClose - dB) / dM);
	//		}
	//	}
	//}
}
/*
*���u�Z���ഫ���y���I--�h��
*/
void CAction::LineGetToPoint(LONG &lXClose, LONG &lYClose, LONG &lZClose, LONG lX0, LONG lY0, LONG lX1, LONG lY1, LONG lZ0, LONG lZ1, LONG &lLineClose)
{
	LONG lLength = 0;
	lLength = LONG(sqrt(pow(lX0 - lX1, 2) + pow(lY0 - lY1, 2) + pow(lZ0 - lZ1, 2)));
	if (lZ0 == lZ1)
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
	if (lAcceleration == 0)
	{
		t = (DOUBLE)lStartDistance / (DOUBLE)lWorkVelociy;
		return (LONG)round(t * 1000000);
	}
	t = (DOUBLE)(lWorkVelociy - lInitVelociy) / (DOUBLE)lAcceleration;
	S = (DOUBLE)lInitVelociy*t + (DOUBLE)lAcceleration*pow(t, 2) / 2.0;
	if (lStartDistance >= (LONG)round(S))
	{
		lDistance = lStartDistance - (LONG)round(S);
		dStartTime = (DOUBLE)lDistance / (DOUBLE)lWorkVelociy;
		dStartTime += t;
		return (LONG)round(dStartTime * 1000000);
	}
	else
	{
		AfxMessageBox(L"�]�m�Z�����A��");
		return (LONG)round(t * 1000000);
	}
}
LONG CAction::CalPreglue(LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
	double S = 0, t = 0;
	if (lAcceleration == 0)
	{
		return 0;
	}
	t = (double)(lWorkVelociy - lInitVelociy) / (double)lAcceleration;
	S = (double)lInitVelociy*t + (double)lAcceleration*pow(t, 2) / 2.0;
	return (LONG)round(S);
}
LONG CAction::CalPreglueTime(LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
	double S = 0, t = 0;
	if (lAcceleration == 0)
	{
		return 0;
	}
	t = (double)(lWorkVelociy - lInitVelociy) / (double)lAcceleration;
	S = (double)lInitVelociy*t + (double)lAcceleration*pow(t, 2) / 2.0;
	return (LONG)round(t * 1000000);
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
	int h = int( round((e*c - b*f) / (a*e - b*d)));
	int k = int( round((a*f - d*c) / (a*e - b*d)));
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
	//DOUBLE v1x = LocatX1 - LocatX;
	//DOUBLE v1y = LocatY1 - LocatY;
	//DOUBLE v2x = LocatX2 - LocatX;
	//DOUBLE v2y = LocatY2 - LocatY;
	//DOUBLE dCosAng = (v1x*v2x + v1y*v2y) / (sqrt(pow(v1x, 2) + pow(v1y, 2))*sqrt(pow(v2x, 2) + pow(v2y, 2)));
	//dCosAng = acos(dCosAng)*180.0 / M_PI;
	//CString csbuff;
	//csbuff.Format(L"����=%lf,(%lf,%lf),(%lf,%lf)\n", dCosAng, v1x, v1y, v2x, v2y);
	//AfxMessageBox(csbuff);
	//return dCosAng;
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
	if (pt1 == pt2 || lWidth == 0)
	{
		return;
	}

	std::vector<DATA_2MOVE> vecData2do;
	vecData2do.clear();
	LONG width = lWidth * 1000;

	//�ϥ�(1)���ʫe����(lStartDelayTime)
	if (lStartDelayTime>0)
	{
		if (!g_bIsStop)
		{
			MO_Do3DLineMove(lX1 - MO_ReadLogicPosition(0), lY1 - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration,
				lInitVelociy);//�^��_�l�I!
			PreventMoveError();
		}
		if (!g_bIsStop)
		{
			MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//�^��_�l�I!
			PreventMoveError();
		}
		PauseDoGlue();//�Ȱ��^�_�᭫�s�(Ū���Ȱ��ѼơA��ѼƬ�0�ɥX���A�B�I�����n���}�C)
		if (!g_bIsStop && g_bIsDispend == 1)
		{
			MO_GummingSet();//�(���d)
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
		/*���J�Ĥ@�I*/
		MO_Do2dDataLine(pt1.x, pt1.y, vecData2do);
	}
	//�ϥ�(2)�p��X���Z���A�]�w�X���I
	else if (lStartDistance>0)
	{
		LONG glueDist = lStartDistance;

		if (pt1.x == pt2.x)//���I����
		{
			/*�Z����xy���q*/
			glueDist = LONG(glueDist*M_SQRT1_2);
			glueDist = LONG((pt1.y<pt2.y) ? -glueDist : glueDist);
			MO_Do2dDataLine(pt1.x + glueDist, pt1.y + glueDist, vecData2do);
		}
		else if (pt1.y == pt2.y)//���I����
		{
			/*�Z����xy���q*/
			glueDist = LONG(glueDist*M_SQRT1_2);
			glueDist = (pt1.x<pt2.x) ? glueDist : -glueDist;
			MO_Do2dDataLine(pt1.x + glueDist, pt1.y - glueDist, vecData2do);
		}
		else
		{
			glueDist = (pt1.x<pt2.x) ? -glueDist : glueDist;
			MO_Do2dDataLine(pt1.x + glueDist, pt1.y, vecData2do);
		}
	}
	//(1)(2)�Ҥ��ϥ�
	else
	{
		/*���J�Ĥ@�I*/
		MO_Do2dDataLine(pt1.x, pt1.y, vecData2do);
	}

	//���I����
	if (pt1.x == pt2.x)
	{
		/*��R�ϰ�(�x��)���*/
		LONG lHeightOfRect = (LONG)abs(round(double(pt2.y - pt1.y)*M_SQRT1_2));
		/*���j�e��xy�첾�q�A��Rxy�첾�q�p��AoddEven���f�V����*/
		LONG shiftWidth = LONG(width*M_SQRT1_2);
		LONG shift = (LONG)abs(round((pt1.y - pt2.y) / 2.0));
		BOOL oddEven = TRUE;
		if (pt2.y<pt1.y)
		{
			oddEven = FALSE;
		}
		while (lHeightOfRect >= 0)
		{
			ptNow.x = (oddEven) ? (ptNow.x + shift) : (ptNow.x - shift);
			ptNow.y = (oddEven) ? (ptNow.y + shift) : (ptNow.y - shift);
			MO_Do2dDataLine(ptNow.x, ptNow.y, vecData2do);

			lHeightOfRect -= width;
			if (lHeightOfRect >= 0)
			{
				ptNow.x = (pt2.y>pt1.y) ? (ptNow.x - shiftWidth) : (ptNow.x + shiftWidth);
				ptNow.y = (pt2.y>pt1.y) ? (ptNow.y + shiftWidth) : (ptNow.y - shiftWidth);
				MO_Do2dDataLine(ptNow.x, ptNow.y, vecData2do);
			}
			else
			{
				if (oddEven)
				{
					ptNow = pt2;
					MO_Do2dDataLine(ptNow.x, ptNow.y, vecData2do);
				}
				else
				{
					ptNow.x = (pt2.y>pt1.y) ? pt1.x - shift : pt1.x + shift;
					ptNow.y = LONG((pt1.y + pt2.y) / 2.0);
					MO_Do2dDataLine(ptNow.x, ptNow.y, vecData2do);
					ptNow = pt2;
					MO_Do2dDataLine(ptNow.x, ptNow.y, vecData2do);
				}
			}
			oddEven = !oddEven;
		}
	}
	//���I����
	else if (pt1.y == pt2.y)
	{
		LONG lHeightOfRect = (LONG)abs(round(double(pt2.x - pt1.x)*M_SQRT1_2));
		LONG shiftWidth = LONG(width*M_SQRT1_2);
		LONG shift = (LONG)abs(round((pt1.x - pt2.x) / 2.0));
		BOOL oddEven = TRUE;
		if (pt2.x<pt1.x)
		{
			oddEven = FALSE;
		}
		while (lHeightOfRect >= 0)
		{
			ptNow.x = (oddEven) ? (ptNow.x + shift) : (ptNow.x - shift);
			ptNow.y = (oddEven) ? (ptNow.y - shift) : (ptNow.y + shift);
			MO_Do2dDataLine(ptNow.x, ptNow.y, vecData2do);

			lHeightOfRect -= width;
			if (lHeightOfRect >= 0)
			{
				ptNow.x = (pt2.x>pt1.x) ? (ptNow.x + shiftWidth) : (ptNow.x - shiftWidth);
				ptNow.y = (pt2.x>pt1.x) ? (ptNow.y + shiftWidth) : (ptNow.y - shiftWidth);
				MO_Do2dDataLine(ptNow.x, ptNow.y, vecData2do);
			}
			else
			{
				if (oddEven)
				{
					ptNow.x = LONG((pt1.x + pt2.x) / 2.0);
					ptNow.y = (pt2.x>pt1.x) ? pt1.x + shift : pt1.x - shift;
					MO_Do2dDataLine(ptNow.x, ptNow.y, vecData2do);
					ptNow = pt2;
					MO_Do2dDataLine(ptNow.x, ptNow.y, vecData2do);
				}
				else
				{
					ptNow = pt2;
					MO_Do2dDataLine(ptNow.x, ptNow.y, vecData2do);
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
		while (lHeightOfRect >= 0)
		{
			/*x�����U�@�I*/
			ptNow.x = (oddEven) ? (ptNow.x + shiftX) : (ptNow.x - shiftX);
			MO_Do2dDataLine(ptNow.x, ptNow.y, vecData2do);
			if (ptNow == pt2)
			{
				break;
			}

			/*�P�_���j�e�׬O�_�٦��Ѿl�A��s���j�e�ת��y��*/
			lHeightOfRect -= width;
			if (lHeightOfRect >= 0)
			{
				/*y�e�ץ����U�@�I*/
				ptNow.y = (pt2.y>pt1.y) ? (ptNow.y + width) : (ptNow.y - width);
				MO_Do2dDataLine(ptNow.x, ptNow.y, vecData2do);
			}
			else
			{
				/*���j�e�פ������B�z*/
				if (oddEven)
				{
					/*�����쵲���I*/
					ptNow = pt2;
					MO_Do2dDataLine(ptNow.x, ptNow.y, vecData2do);
				}
				else
				{
					/*�ɨ��e�צܵ����I*/
					ptNow.y = pt2.y;
					MO_Do2dDataLine(ptNow.x, ptNow.y, vecData2do);
					ptNow = pt2;
					MO_Do2dDataLine(ptNow.x, ptNow.y, vecData2do);
				}
			}
			oddEven = !oddEven;
		}
	}

	//�I�}�C�ഫ
	LA_AbsToOppo2Move(vecData2do);
	for (UINT i = 1; i < vecData2do.size(); i++)
	{
		DATA_2Do[i - 1] = vecData2do.at(i);
	}
#ifdef MOVE
	if (!g_bIsStop)
	{
		MO_Do3DLineMove(vecData2do.at(0).EndP.x, vecData2do.at(0).EndP.y, 0, lWorkVelociy, lAcceleration,
			lInitVelociy);//�^��_�l�I!
		PreventMoveError();
	}
	if (!g_bIsStop)
	{
		MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//�^��_�l�I!
		PreventMoveError();
	}
#endif
	//�ϥ�(2)�]�m�Z��
	if (!(lStartDelayTime>0) && lStartDistance>0)
	{
		//timeUpGlue ���us
		LONG timeUpGlue = CalPreglue(lStartDistance, lWorkVelociy, lAcceleration, lInitVelociy);
		if (!g_bIsStop)
		{		
                        CAction::g_YtimeOutGlueSet = TRUE;
#ifdef MOVE
		MO_TimerSetIntter(timeUpGlue, 0);//�ϥ�Y timer���_�X��
#endif
                 }
	}

	//�ϥ�(5)�����Z��(lCloseDistance)
	if (lCloseDistance>0)
	{
		LONG sumPath = 0;
		LONG finishTime = 0;
		DOUBLE avgTime = 0;
		LONG accLength = CalPreglue(lWorkVelociy, lAcceleration, lInitVelociy);
		for (UINT i = 1; i<vecData2do.size(); i++)
		{
			sumPath += vecData2do.at(i).Distance;
		}
		avgTime = ((DOUBLE)sumPath - (DOUBLE)accLength) / (DOUBLE)lWorkVelociy;
		finishTime = (LONG)round(avgTime * 1000000) + CalPreglueTime(lWorkVelociy, lAcceleration, lInitVelociy);
		LONG closeDistTime = CalPreglue(lCloseDistance, lWorkVelociy, 0, lInitVelociy);
		if (!g_bIsStop)
		{
                CAction::g_ZtimeOutGlueSet = FALSE;
		MO_TimerSetIntter(finishTime - closeDistTime, 1);
                }
	}

	if (lStartDelayTime == 0 && lStartDistance == 0)
	{
		if (!g_bIsStop && g_bIsDispend == 1)
		{
			MO_GummingSet();
		}
	}

	MO_DO2Curve(DATA_2Do, vecData2do.size() - 1, lWorkVelociy);
	PreventMoveError();//�����X�ʿ��~
	Sleep(200);
	vecData2do.clear();



	//�ϥ�(3)���d�ɶ�(lCloseOffDelayTime)
	if (lCloseOffDelayTime>0)
	{
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
	}
	PauseStopGlue();//�Ȱ��ɰ����(g_bIsPause=1)
	MO_StopGumming();//����X��

					 //�ϥ�(6)��������(lCloseONDelayTime)
	if (!(lCloseDistance>0) && lCloseONDelayTime>0)
	{
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
	if (!g_bIsStop)
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
	if (LONG(dRadius) == 0)
	{
		return;
	}
	if (lStartDistance>0)//�ϥ�--(2)�]�m�Z��(lStartDistance)
	{
		LONG lStartX = 0, lStartY = 0;
		ArcGetToPoint(lStartX, lStartY, lStartDistance, lX1, lY1, lCenX, lCenY, LONG(dRadius), bRev);//��X�_�l�I
		cPt0.x = lStartX;
		cPt0.y = lStartY;
		//      m_ptVec.push_back(cPt0);
		if (!g_bIsStop)
		{
			MO_Do3DLineMove(lStartX - MO_ReadLogicPosition(0), lStartY - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration,
				lInitVelociy);//�^��_�l�I!
			PreventMoveError();
		}
		if (!g_bIsStop)
		{
			MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//�^��_�l�I!
			PreventMoveError();
		}
		lTime = LONG(1000000 * (DOUBLE)lStartDistance / (DOUBLE)lWorkVelociy);
		_cwprintf(_T("�p�ɾ��]�m�Z�����ɶ�=%lf \n"), DOUBLE(lTime / 1000000.0));
		/*======�p�ɾ���Ĳ�o���_����X���A�ϥ�y���_����================*/
        	if (!g_bIsStop)
		{
                        CAction::g_YtimeOutGlueSet = TRUE;
               		MO_TimerSetIntter(lTime, 0);//�p�ɨ���ܰ����
                }
	}
	else
	{

		if (!g_bIsStop)
		{
			MO_Do3DLineMove(lX1 - MO_ReadLogicPosition(0), lY1 - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration,
				lInitVelociy);//�^��_�l�I!
			PreventMoveError();
		}
		if (!g_bIsStop)
		{
			MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//�^��_�l�I!
			PreventMoveError();
		}
		PauseDoGlue();//�Ȱ��^�_�᭫�s�(Ū���Ȱ��ѼơA��ѼƬ�0�ɥX���A�B�I�����n���}�C)
		if (!g_bIsStop && g_bIsDispend == 1)
		{
			MO_GummingSet();//�(���d)
		}
		if (lStartDelayTime > 0)//�ϥ�(1)���ʫe����(lStartDelayTime)
		{
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
	while (1)
	{
		lDistance = LONG(lDistance - dWidth);
		if (lDistance < dWidth)
		{
                       	dSumPath -= M_PI / 2 * (sqrt(pow(cPt3.x - cPt4.x, 2) + pow(cPt3.y - cPt4.y, 2)));
			dSumPath += sqrt(pow(cPt3.x - cPt4.x, 2) + pow(cPt3.y - cPt4.y, 2));
			break;
		}
		LineGetToPoint(lXClose, lYClose, cPt1.x, cPt1.y, cPtCen1.x, cPtCen1.y,
			lDistance);
		cPt3.x = lXClose;
		cPt3.y = lYClose;
		if (icnt == 0)
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
	if (lCloseDistance > 0)//�ϥ� --(5)�����Z��
	{
		dCloseTime = 1000000 * ((dSumPath - lCloseDistance) / (DOUBLE)lWorkVelociy);
		/*======�p�ɾ���Ĳ�o���_�����_���A�ϥ�z���_����================*/
		if (!g_bIsStop)
		{
			CAction::g_ZtimeOutGlueSet = FALSE;
			if (lStartDistance > 0)
			{
				_cwprintf(_T("End�����Z�����ɶ�=%lf \n"), DOUBLE(lTime) + DOUBLE(dCloseTime / 1000000.0));
				MO_TimerSetIntter(lTime + LONG(dCloseTime), 1);//�p�ɨ���ܰ����
			}
			else
			{
				_cwprintf(_T("End�����Z�����ɶ�=%lf \n"), DOUBLE(dCloseTime / 1000000.0));
				MO_TimerSetIntter(LONG(dCloseTime), 1);//�p�ɨ���ܰ����
			}
		}
	}
#pragma endregion

	std::vector<DATA_2MOVE> DATA_2DO;
	UINT cnt = 0;
	DATA_2DO.clear();
	icnt = 0;
	for (ptIter = m_ptVec.begin(); ptIter != m_ptVec.end(); ptIter++)
	{
		if (lStartDistance < 0 && cnt == 0)
		{
			cnt++;
		}
		else if ((iData != 0) && (cnt == m_ptVec.size() - 1))
		{
			MO_Do2dDataLine((*ptIter).x, (*ptIter).y, DATA_2DO);
		}
		else if ((iData == 0) && (cnt == m_ptVec.size() - 1))
		{
			MO_Do2dDataCir((*ptIter).x, (*ptIter).y, cPtCen2.x, cPtCen2.y, bRev, DATA_2DO);
		}
		else
		{
			if (cnt % 2 == 0)
			{
				MO_Do2dDataCir((*ptIter).x, (*ptIter).y, cPtCen2.x, cPtCen2.y, bRev, DATA_2DO);
			}
			else
			{
				MO_Do2dDataCir((*ptIter).x, (*ptIter).y, lCenX, lCenY, bRev, DATA_2DO);
			}
		}
		cnt++;
	}
	LA_AbsToOppo2Move(DATA_2DO);
	if (lStartDistance > 0)
	{
		for (UINT i = 0; i < DATA_2DO.size(); i++)
		{
			DATA_2Do[i] = DATA_2DO.at(i);
		}
		MO_DO2Curve(DATA_2Do, DATA_2DO.size(), lWorkVelociy);
	}
	else
	{
		for (UINT i = 1; i < DATA_2DO.size(); i++)
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
	while (MO_Timer(3, 0, 0))
	{
		if (g_bIsStop == 1)
		{
			break;
		}
		Sleep(1);
	}
	MO_StopGumming();//����X��
	if (!g_bIsStop)
	{
		//*************************��R�^��z�b�t�ץثe���X�ʳt�ת��⭿******************
		MO_Do3DLineMove(0, 0, (lZ - lZBackDistance) - lZ, lWorkVelociy * 2, lAcceleration,
			lInitVelociy);//Z�b��^
		PreventMoveError();//����b�d�X��
	}
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
	if (dRadius == 0)
	{
		return;
	}
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
	cPt2.x = LONG(dRadius*cos(dAngCenCos + M_PI_2) + cPtCen.x);
	cPt2.y = LONG(dRadius*sin(dAngCenSin + M_PI_2) + cPtCen.y);
	cPt4.x = LONG(dRadius*cos(dAngCenCos + M_PI_2 * 3) + cPtCen.x);
	cPt4.y = LONG(dRadius*sin(dAngCenSin + M_PI_2 * 3) + cPtCen.y);
	dDistance = sqrt(pow((cPt1.x - cPt4.x), 2) + pow((cPt1.y - cPt4.y), 2));

#pragma region ****�u�q��]�m****
	//�ϥ�(1)���ʫe����(lStartDelayTime)
	if (lStartDelayTime>0)
	{
		if (!g_bIsStop)
		{
			MO_Do3DLineMove(lX1 - MO_ReadLogicPosition(0), lY1 - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration,
				lInitVelociy);//�^��_�l�I!
			PreventMoveError();
		}
		if (!g_bIsStop)
		{
			MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//�^��_�l�I!
			PreventMoveError();
		}
		PauseDoGlue();//�Ȱ��^�_�᭫�s�(Ū���Ȱ��ѼơA��ѼƬ�0�ɥX���A�B�I�����n���}�C)
		if (!g_bIsStop && g_bIsDispend == 1)
		{
			MO_GummingSet();//�(���d)
		}
		MO_Timer(0, 0, lStartDelayTime * 1000);
		MO_Timer(1, 0, lStartDelayTime * 1000);//�u�q�I���]�w---(1)���ʫe����(�b�u�q�}�l�I�W)

		Sleep(1);//����X���A�קK�p�ɾ��쪽��0

                 //�p��s1�Ps2
        S1 = LONG(sqrt(pow(cPt2.x - cPt1.x, 2) + (cPt2.y - cPt1.y, 2)));
        S2 = LONG(sqrt(pow(cPt3.x - cPt2.x, 2) + (cPt3.y - cPt2.y, 2)));
		while (MO_Timer(3, 0, 0))
		{
			if (g_bIsStop)
			{
				break;
			}
			Sleep(1);
		}
		/*���J�Ĥ@�I*/
		m_ptVec.push_back(cPt1);
	}
	//�ϥ�(2)�p��X���Z���A�]�w�X���I
	else if (lStartDistance>0)
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
        icnt = 1;
	while (1)
	{
		dRadius = dRadius - dWidth*sqrt(2);
		dDistance = dDistance - dWidth;
		if (dDistance < dWidth)
		{
                    if (icnt == 1)
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
		if (dDistance < dWidth)
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
	for (ptIter = m_ptVec.begin(); ptIter != m_ptVec.end(); ptIter++)
	{
		MO_Do2dDataLine((*ptIter).x, (*ptIter).y, DATA_2DO);
	}
	//�I�}�C�ഫ
	LA_AbsToOppo2Move(DATA_2DO);
	for (UINT i = 1; i < DATA_2DO.size(); i++)
	{
		DATA_2Do[i - 1] = DATA_2DO.at(i);
	}
	if (!g_bIsStop)
	{
		MO_Do3DLineMove(DATA_2DO.at(0).EndP.x, DATA_2DO.at(0).EndP.y, 0, lWorkVelociy, lAcceleration,
			lInitVelociy);//�^��_�l�I!
		PreventMoveError();
	}
	if (!g_bIsStop)
	{
		MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//�^��_�l�I!
		PreventMoveError();
	}
	//�ϥ�(2)�]�m�Z��
	if (lStartDistance>0)
	{
		//timeUpGlue ���us
                timeUpGlue = CalPreglue(lStartDistance, lWorkVelociy, lAcceleration, lInitVelociy);
                if (!g_bIsStop)
                {
	    	    CAction::g_YtimeOutGlueSet = TRUE;
		    MO_TimerSetIntter(timeUpGlue, 0);//�ϥ�y timer���_ �X��
                }
	}
	//�ϥ�(5)�����Z��(lCloseDistance)
        DOUBLE T1 = 0, T2 = 0;
        LONG V1 = 0, V2 = 0, AllS = 0, AllT = 0, FinallV = 0;
	if (lCloseDistance>0)
	{
        //�Ĥ@�϶��s�򴡸�
        T1 = (-lInitVelociy / 1000 + (sqrt(pow(lInitVelociy / 1000, 2) + 2 * lAcceleration / 1000 * S1 / 1000))) / (DOUBLE)(lAcceleration / 1000.0);
        V1 = LONG(lInitVelociy + lAcceleration*T1);
        if (lWorkVelociy > V1)
        {
            AllT = LONG(T1 * 1000000);
            //�ĤG�϶��s�򴡸�
            T2 = (-V1 / 1000 + (sqrt(pow(V1 / 1000, 2) + 2 * lAcceleration / 1000 * S1 / 1000))) / (DOUBLE)(lAcceleration / 1000.0);
            V2 = LONG(lAcceleration* LONG(T1) + V1);
            if (lWorkVelociy > V2)
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
        if (lStartDistance > 0)
        {
            dCloseTime = 1000000 * ((dSumPath - lCloseDistance) / (DOUBLE)lWorkVelociy);
            /*======�p�ɾ���Ĳ�o���_�����_���A�ϥ�z���_����================*/
            if (!g_bIsStop)
            {
                CAction::g_ZtimeOutGlueSet = FALSE;
                _cwprintf(_T("End�����Z�����ɶ�=%lf \n"), DOUBLE(timeUpGlue) + DOUBLE(dCloseTime / 1000000.0));
                MO_TimerSetIntter(timeUpGlue + LONG(dCloseTime), 1);//�p�ɨ���ܰ����
            }
        }
        else
        {
            LONG lCloseTime = 0;
            lCloseTime = LONG(CalPreglue(LONG(dSumPath - AllS - lCloseDistance), FinallV, lAcceleration, lInitVelociy));
            lCloseTime = lCloseTime - AllT;
            /*======�p�ɾ���Ĳ�o���_�����_���A�ϥ�z���_����================*/
            if (!g_bIsStop)
            {
                CAction::g_ZtimeOutGlueSet = FALSE;
                _cwprintf(_T("End�����Z�����ɶ�=%d \n"), lCloseTime);
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
        CAction::g_ZtimeOutGlueSet = FALSE;
        MO_TimerSetIntter(finishTime-closeDistTime, 1);*/
	}

	if (lStartDelayTime == 0 && lStartDistance == 0)
	{
		PauseDoGlue();//�Ȱ���_���~��X��(g_bIsPause=0)�X��
	}

	MO_DO2Curve(DATA_2Do, DATA_2DO.size() - 1, lWorkVelociy);
	PreventMoveError();//�����X�ʿ��~
	Sleep(200);
	DATA_2DO.clear();

	//�ϥ�(3)���d�ɶ�(lCloseOffDelayTime)
	if (lCloseOffDelayTime>0)
	{
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
	}

	PauseStopGlue();//�Ȱ��ɰ����(g_bIsPause=1)
	MO_StopGumming();//����X��

					 //�ϥ�(6)��������(lCloseONDelayTime)
	if (!(lCloseDistance>0) && lCloseONDelayTime>0)
	{
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

	if (!g_bIsStop)
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
	if (dRadius == 0)
	{
		return;
	}
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
	cPt2.x = LONG(dRadius*cos(dAngCenCos + M_PI_2) + cPtCen.x);
	cPt2.y = LONG(dRadius*sin(dAngCenSin + M_PI_2) + cPtCen.y);
	cPt4.x = LONG(dRadius*cos(dAngCenCos + M_PI_2 * 3) + cPtCen.x);
	cPt4.y = LONG(dRadius*sin(dAngCenSin + M_PI_2 * 3) + cPtCen.y);
	dDistance = sqrt(pow((cPt1.x - cPt4.x), 2) + pow((cPt1.y - cPt4.y), 2));


#pragma region ****�u�q��]�m****
	//�ϥ�(1)���ʫe����(lStartDelayTime)
	if (lStartDelayTime>0)
	{
		if (!g_bIsStop)
		{
			MO_Do3DLineMove(lX1 - MO_ReadLogicPosition(0), lY1 - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration,
				lInitVelociy);//�^��_�l�I!
			PreventMoveError();
		}
		if (!g_bIsStop)
		{
			MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//�^��_�l�I!
			PreventMoveError();
		}
		PauseDoGlue();//�Ȱ��^�_�᭫�s�(Ū���Ȱ��ѼơA��ѼƬ�0�ɥX���A�B�I�����n���}�C)
		if (!g_bIsStop && g_bIsDispend == 1)
		{
			MO_GummingSet();//�(���d)
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
		/*���J�Ĥ@�I*/
		m_ptVec.push_back(cPt1);
	}
	//�ϥ�(2)�p��X���Z���A�]�w�X���I
	else if (lStartDistance>0)
	{
		CPoint ptSetDist(0, 0);
		LONG glueDist = lStartDistance;
		DOUBLE dLength = sqrt(pow(cPt1.x - cPt2.x, 2) + pow(cPt1.y - cPt2.y, 2));
		if (cPt1.x == cPt2.x)
		{
			ptSetDist = cPt1;
			glueDist = LONG(glueDist*M_SQRT1_2);
			ptSetDist.y = (cPt1.y<cPt2.y) ? cPt1.y - glueDist : cPt1.y + glueDist;
		}
		else if (cPt1.y == cPt2.y)
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
#ifdef PRINTF
			_cwprintf(_T("��ݼe�׹L�j \n"));
#endif
			m_ptVec.push_back(cPt1);
			break;
		}
		dRadius = dRadius - dWidth*sqrt(2);
		dDistance = dDistance - dWidth;
		if (iBuff*dWidth >  dWidth2)   //dWidth2��ݼe��
		{
			dDistance += dWidth;
			cPt1.x = LONG(dDistance*cos(dAngCos) + cPt4.x);
			cPt1.y = LONG(dDistance*sin(dAngSin) + cPt4.y);
			m_ptVec.push_back(cPt1);
			break;
		}
		if (dDistance < dWidth)
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
		if (dDistance < dWidth)
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
	for (ptIter = m_ptVec.begin(); ptIter != m_ptVec.end(); ptIter++)
	{
		MO_Do2dDataLine((*ptIter).x, (*ptIter).y, DATA_2DO);
	}
	//�I�}�C�ഫ
	LA_AbsToOppo2Move(DATA_2DO);
	for (UINT i = 1; i < DATA_2DO.size(); i++)
	{
		DATA_2Do[i - 1] = DATA_2DO.at(i);
	}
	if (!g_bIsStop)
	{
		MO_Do3DLineMove(DATA_2DO.at(0).EndP.x, DATA_2DO.at(0).EndP.y, 0, lWorkVelociy, lAcceleration,
			lInitVelociy);//�^��_�l�I!
		PreventMoveError();
	}
	if (!g_bIsStop)
	{
		MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//�^��_�l�I!
		PreventMoveError();
	}
	//�ϥ�(2)�]�m�Z��
	if (lStartDistance>0)
	{
		//timeUpGlue ���us
		LONG timeUpGlue = CalPreglue(lStartDistance, lWorkVelociy, lAcceleration, lInitVelociy);
		if (!g_bIsStop)
		{
		        g_YtimeOutGlueSet = TRUE;
		        MO_TimerSetIntter(timeUpGlue, 0);//�ϥ�z timer���_ �X��
                }
	}
	//�ϥ�(5)�����Z��(lCloseDistance)
	if (lCloseDistance>0)
	{
		LONG sumPath = 0;
		LONG finishTime = 0;
		DOUBLE avgTime = 0;
		LONG accLength = CalPreglue(lWorkVelociy, lAcceleration, lInitVelociy);
		for (UINT i = 1; i<DATA_2DO.size(); i++)
		{
			sumPath += DATA_2DO.at(i).Distance;
		}
		avgTime = ((DOUBLE)sumPath - (DOUBLE)accLength) / (DOUBLE)lWorkVelociy;
		finishTime = (LONG)round(avgTime * 1000000) + CalPreglueTime(lWorkVelociy, lAcceleration, lInitVelociy);
		LONG closeDistTime = CalPreglue(lCloseDistance, lWorkVelociy, 0, lInitVelociy);
		if (!g_bIsStop)
		{
	    	        CAction::g_ZtimeOutGlueSet = FALSE;
		        MO_TimerSetIntter(finishTime - closeDistTime, 1);
                }
	}

	if (lStartDelayTime == 0 && lStartDistance == 0)
	{
		PauseDoGlue();//�Ȱ���_���~��X��(g_bIsPause=0)�X��
	}
	MO_DO2Curve(DATA_2Do, DATA_2DO.size() - 1, lWorkVelociy);
	PreventMoveError();//�����X�ʿ��~
	Sleep(200);
	DATA_2DO.clear();

	//�ϥ�(3)���d�ɶ�(lCloseOffDelayTime)
	if (lCloseOffDelayTime>0)
	{
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
	}

	PauseStopGlue();//�Ȱ��ɰ����(g_bIsPause=1)
	MO_StopGumming();//����X��
					 //�ϥ�(6)��������(lCloseONDelayTime)
	if (!(lCloseDistance>0) && lCloseONDelayTime>0)
	{
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
	if (!g_bIsStop)
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
	if (LONG(dRadius) == 0)
	{
		return;
	}
	if (LONG(dRadius) == 0)
	{
		return;
	}
	if (lStartDistance>0)//�ϥ�--(2)�]�m�Z��(lStartDistance)
	{
		LONG lStartX = 0, lStartY = 0;
		ArcGetToPoint(lStartX, lStartY, lStartDistance, lX1, lY1, lCenX, lCenY, LONG(dRadius), bRev);//��X�_�l�I
		if (!g_bIsStop)
		{
			MO_Do3DLineMove(lStartX - MO_ReadLogicPosition(0), lStartY - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration,
				lInitVelociy);//�^��_�l�I!
			PreventMoveError();
		}
		if (!g_bIsStop)
		{
			MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//�^��_�l�I!
			PreventMoveError();
		}
		lTime = LONG(1000000 * (DOUBLE)lStartDistance / (DOUBLE)lWorkVelociy);
#ifdef PRINTF
		_cwprintf(_T("�p�ɾ��]�m�Z�����ɶ�=%lf \n"), DOUBLE(lTime / 1000000.0));
#endif
		/*======�p�ɾ���Ĳ�o���_����X���A�ϥ�y���_����================*/
		if (!g_bIsStop)
		{
		         CAction::g_YtimeOutGlueSet = TRUE;
	    	         MO_TimerSetIntter(lTime, 0);//�p�ɨ���ܰ����
                }
	}
	else
	{

		if (!g_bIsStop)
		{
			MO_Do3DLineMove(lX1 - MO_ReadLogicPosition(0), lY1 - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration,
				lInitVelociy);//�^��_�l�I!
			PreventMoveError();
		}
		if (!g_bIsStop)
		{
			MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//�^��_�l�I!
			PreventMoveError();
		}
		PauseDoGlue();//�Ȱ��^�_�᭫�s�(Ū���Ȱ��ѼơA��ѼƬ�0�ɥX���A�B�I�����n���}�C)
		if (!g_bIsStop && g_bIsDispend == 1)
		{
			MO_GummingSet();//�(���d)
		}
		if (lStartDelayTime > 0)//�ϥ�(1)���ʫe����(lStartDelayTime)
		{
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
	while (1)
	{
		if ((iBuff == 1) && (lDistance - (2 * dWidth2) < 0))//��ܨϥγ����������
		{
#ifdef PRINTF
			_cwprintf(_T("��ݼe�׹L�j \n"));
#endif
			dSumPath += M_PI*dRadius;
			iBuff = 1;
			m_ptVec.push_back(cPt1);//������
			break;
		}
		else if ((iBuff == 1) && dWidth2 == 0)//��ܨϥγ����������
		{
#ifdef PRINTF
			_cwprintf(_T("����� \n"));
#endif
			dSumPath += M_PI*dRadius;
			iBuff = 1;
			m_ptVec.push_back(cPt1);//������
			break;
		}
		dRadius = dRadius - dWidth;
		lDistance = LONG(lDistance - dWidth);
		if (iBuff * dWidth > dWidth2)
		{
			lDistance += LONG(dWidth);
			LineGetToPoint(lXClose, lYClose, cPt1.x, cPt1.y, cPtCen1.x, cPtCen1.y,
				lDistance);
			cPt3.x = lXClose;
			cPt3.y = lYClose;
			m_ptVec.push_back(cPt3);
			if (iBuff == 1)
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
		if (iBuff == 1)
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
	if (lCloseDistance > 0)//�ϥ� --(5)�����Z��
	{
		dCloseTime = 1000000 * ((dSumPath - lCloseDistance) / (DOUBLE)lWorkVelociy);
		/*======�p�ɾ���Ĳ�o���_�����_���A�ϥ�z���_����================*/
		if (!g_bIsStop)
		{
	 	        CAction::g_ZtimeOutGlueSet = FALSE;
		        if (lStartDistance > 0)
		        {
#ifdef PRINTF
	        		_cwprintf(_T("End�����Z�����ɶ�=%lf \n"), DOUBLE(lTime) + DOUBLE(dCloseTime / 1000000.0));
#endif
	        		MO_TimerSetIntter(lTime + LONG(dCloseTime), 1);//�p�ɨ���ܰ����
	        	}
	        	else
	        	{
#ifdef PRINTF
	        		_cwprintf(_T("End�����Z�����ɶ�=%lf \n"), DOUBLE(dCloseTime / 1000000.0));
#endif
	        		MO_TimerSetIntter(LONG(dCloseTime), 1);//�p�ɨ���ܰ����
	        	}
                }
	}
#pragma endregion
	std::vector<DATA_2MOVE> DATA_2DO;
	DATA_2DO.clear();
	if (iBuff == 1)//��ܨ�ݼe�׬��s  ���e�X�@�ӳ����
	{
		for (UINT i = 1; i < m_ptVec.size(); i++)
		{
			{
				MO_Do2dDataCir(m_ptVec.at(i).x, m_ptVec.at(i).y, cPtCen1.x, cPtCen1.y, bRev, DATA_2DO);//�W�b��
			}
		}
	}
	else
	{
		for (UINT i = 1; i < m_ptVec.size(); i++)
		{
			if (i == m_ptVec.size() - 1)
			{
				if (i % 2 == 0)
				{
					MO_Do2dDataCir(m_ptVec.at(i).x, m_ptVec.at(i).y, cPtCen1.x, cPtCen1.y, bRev, DATA_2DO);//�W�b��
				}
				else
				{
					MO_Do2dDataCir(m_ptVec.at(i).x, m_ptVec.at(i).y, cPtCen2.x, cPtCen2.y, bRev, DATA_2DO);//�U�b��
				}
			}
			else
			{
				if (i % 2 != 0)
				{
					MO_Do2dDataCir(m_ptVec.at(i).x, m_ptVec.at(i).y, cPtCen1.x, cPtCen1.y, bRev, DATA_2DO);//�W�b��
				}
				else
				{
					MO_Do2dDataCir(m_ptVec.at(i).x, m_ptVec.at(i).y, cPtCen2.x, cPtCen2.y, bRev, DATA_2DO);//�U�b��
				}
			}
		}
	}
	LA_AbsToOppo2Move(DATA_2DO);
	for (UINT i = 0; i < DATA_2DO.size(); i++)
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
	while (MO_Timer(3, 0, 0))
	{
		if (g_bIsStop == 1)
		{
			break;
		}
		Sleep(1);
	}
	MO_StopGumming();//����X��
	if (!g_bIsStop)
	{
		//*************************��R�^��z�b�t�ץثe���X�ʳt�ת��⭿******************
		MO_Do3DLineMove(0, 0, (lZ - lZBackDistance) - lZ, lWorkVelociy * 2, lAcceleration,
			lInitVelociy);//Z�b��^
		PreventMoveError();//����b�d�X��
	}
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
	if (dRadius == 0)
	{
		return;
	}
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
		cPt1.x = LONG(dDistance*cos(dAngCos) + cPt4.x);
		cPt1.y = LONG(dDistance*sin(dAngSin) + cPt4.y);
		m_ptVec.push_back(cPt1);
		cPt2.x = LONG(dRadius*cos(dAngCenCos + M_PI_2) + cPtCen.x);
		cPt2.y = LONG(dRadius*sin(dAngCenSin + M_PI_2) + cPtCen.y);
		m_ptVec.push_back(cPt2);
		dDistance = dDistance - dWidth;
		if (dDistance < dWidth)
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
	if (lStartDelayTime>0)
	{
		if (!g_bIsStop)
		{
			rptIter = m_ptVec.rbegin();
			MO_Do3DLineMove(rptIter->x - MO_ReadLogicPosition(0), rptIter->y - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration,
				lInitVelociy);//�^��_�l�I!
			PreventMoveError();
		}
		if (!g_bIsStop)
		{
			MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//�^��_�l�I!
			PreventMoveError();
		}
		PauseDoGlue();//�Ȱ��^�_�᭫�s�(Ū���Ȱ��ѼơA��ѼƬ�0�ɥX���A�B�I�����n���}�C)
		if (!g_bIsStop && g_bIsDispend == 1)
		{
			MO_GummingSet();//�(���d)
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
	}
	//�ϥ�(2)�p��X���Z���A�]�w�X���I
	else if (lStartDistance>0)
	{
		rptIter = m_ptVec.rbegin();
		CPoint rPt1 = *rptIter;
		CPoint rPt2 = *(rptIter + 1);
		CPoint ptSetDist(0, 0);
		LONG glueDist = lStartDistance;
		DOUBLE dLength = sqrt(pow(rPt1.x - rPt2.x, 2) + pow(rPt1.y - rPt2.y, 2));
		if (rPt1.x == rPt2.x)
		{
			ptSetDist = rPt1;
			glueDist = LONG(glueDist*M_SQRT1_2);
			ptSetDist.y = (rPt1.y<rPt2.y) ? rPt1.y - glueDist : rPt1.y + glueDist;
		}
		else if (rPt1.y == rPt2.y)
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
	for (rptIter = m_ptVec.rbegin(); rptIter != m_ptVec.rend(); rptIter++)
	{
		MO_Do2dDataLine((*rptIter).x, (*rptIter).y, DATA_2DO);
	}

	LA_AbsToOppo2Move(DATA_2DO);
	for (UINT i = 1; i < DATA_2DO.size(); i++)
	{
		DATA_2Do[i - 1] = DATA_2DO.at(i);
	}
	if (!g_bIsStop)
	{
		MO_Do3DLineMove(DATA_2DO.at(0).EndP.x, DATA_2DO.at(0).EndP.y, 0, lWorkVelociy,
			lAcceleration, lInitVelociy);//����
		PreventMoveError();
	}
	if (!g_bIsStop)
	{
		MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//Z�b���U
		PreventMoveError();//����b�d�X��
	}
	//�ϥ�(2)�]�m�Z��
	if (lStartDistance>0)
	{
		//timeUpGlue ���us
		LONG timeUpGlue = CalPreglue(lStartDistance, lWorkVelociy, lAcceleration, lInitVelociy);
		if (!g_bIsStop)
		{
	        	g_YtimeOutGlueSet = TRUE;
	        	MO_TimerSetIntter(timeUpGlue, 0);//�ϥ�Y timer���_ �X��
                }
	}
	//�ϥ�(5)�����Z��(lCloseDistance)
	if (lCloseDistance>0)
	{
		LONG sumPath = 0;
		LONG finishTime = 0;
		DOUBLE avgTime = 0;
		LONG accLength = CalPreglue(lWorkVelociy, lAcceleration, lInitVelociy);
		for (UINT i = 1; i<DATA_2DO.size(); i++)
		{
			sumPath += DATA_2DO.at(i).Distance;
		}
		avgTime = ((DOUBLE)sumPath - (DOUBLE)accLength) / (DOUBLE)lWorkVelociy;
		finishTime = (LONG)round(avgTime * 1000000) + CalPreglueTime(lWorkVelociy, lAcceleration, lInitVelociy);
		LONG closeDistTime = CalPreglue(lCloseDistance, lWorkVelociy, 0, lInitVelociy);
		if (!g_bIsStop)
		{
	        	CAction::g_ZtimeOutGlueSet = FALSE;
        		MO_TimerSetIntter(finishTime - closeDistTime, 1);
                }
	}

	if (lStartDelayTime == 0 && lStartDistance == 0)
	{
		PauseDoGlue();//�Ȱ���_���~��X��(g_bIsPause=0)�X��
	}
	MO_DO2Curve(DATA_2Do, DATA_2DO.size() - 1, lWorkVelociy);
	PreventMoveError();//�����X�ʿ��~
	Sleep(200);
	DATA_2DO.clear();
	//�ϥ�(3)���d�ɶ�(lCloseOffDelayTime)
	if (lCloseOffDelayTime>0)
	{
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
	}
	PauseStopGlue();//�Ȱ��ɰ����(g_bIsPause=1)
	MO_StopGumming();//����X��
					 //�ϥ�(6)��������(lCloseONDelayTime)
	if (!(lCloseDistance>0) && lCloseONDelayTime>0)
	{
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
	if (!g_bIsStop)
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
	if (LONG(dRadius) == 0)
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
	while (1)
	{
		lDistance = LONG(lDistance - dWidth);
		if (lDistance < dWidth)
		{
                    dSumPath -= M_PI / 2 * (sqrt(pow(cPt3.x - cPt4.x, 2) + pow(cPt3.y - cPt4.y, 2)));
                    dSumPath += sqrt(pow(cPt3.x - cPt4.x, 2) + pow(cPt3.y - cPt4.y, 2));
			break;
		}
		LineGetToPoint(lXClose, lYClose, cPt1.x, cPt1.y, cPtCen1.x, cPtCen1.y,
			lDistance);
		cPt3.x = lXClose;
		cPt3.y = lYClose;
		if (icnt == 0)
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
	if (lStartDistance>0)//�ϥ�--(2)�]�m�Z��(lStartDistance)
	{
		LONG lStartX = 0, lStartY = 0;
		LineGetToPoint(lStartX, lStartY, lX1, lY1, lCenX, lCenY, lStartDistance);//��X�_�l�I
		if (!g_bIsStop)
		{
			MO_Do3DLineMove(lStartX - MO_ReadLogicPosition(0), lStartY - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration,
				lInitVelociy);//�^��_�l�I!
			PreventMoveError();
		}
		if (!g_bIsStop)
		{
			MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//�^��_�l�I!
			PreventMoveError();
		}
		lTime = CalPreglue(lStartDistance, lWorkVelociy, lAcceleration, lInitVelociy);
#ifdef PRINTF
		_cwprintf(_T("�p�ɾ��]�m�Z�����ɶ�=%lf \n"), DOUBLE(lTime / 1000000.0));
#endif
		/*======�p�ɾ���Ĳ�o���_����X���A�ϥ�y���_����================*/
		if (!g_bIsStop)
		{
		        CAction::g_YtimeOutGlueSet = TRUE;
		        MO_TimerSetIntter(lTime, 0);//�p�ɨ���ܰ����
                }
	}
	else
	{
		//����ɡA���ʨ줤���I�b�U��
		if (!g_bIsStop)
		{
			MO_Do3DLineMove(lCenX - MO_ReadLogicPosition(0), lCenY - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration,
				lInitVelociy);//���u���ʦܶ��
			PreventMoveError();
		}
		if (!g_bIsStop)
		{
			MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration,
				lInitVelociy);//Z�b���U
			PreventMoveError();//����b�d�X��
		}
		PauseDoGlue();//�Ȱ��^�_�᭫�s�(Ū���Ȱ��ѼơA��ѼƬ�0�ɥX���A�B�I�����n���}�C)
		if (!g_bIsStop && g_bIsDispend == 1)
		{
			MO_GummingSet();//�(���d)
		}
		if (lStartDelayTime > 0)//�ϥ�(1)���ʫe����(lStartDelayTime)
		{
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
		}
	}
	if (lCloseDistance > 0)//�ϥ� --(5)�����Z��
	{
		dCloseTime = 1000000 * ((dSumPath - lCloseDistance) / (DOUBLE)lWorkVelociy);
		/*======�p�ɾ���Ĳ�o���_�����_���A�ϥ�z���_����================*/
		if (!g_bIsStop)
		{
	        	CAction::g_ZtimeOutGlueSet = FALSE;
	        	if (lStartDistance > 0)
        		{
#ifdef PRINTF
	        		_cwprintf(_T("End�����Z�����ɶ�=%lf \n"), DOUBLE(lTime) + DOUBLE(dCloseTime / 1000000.0));
#endif
	        		MO_TimerSetIntter(lTime + LONG(dCloseTime), 1);//�p�ɨ���ܰ����
	        	}
	        	else
	        	{
#ifdef PRINTF
	        		_cwprintf(_T("End�����Z�����ɶ�=%lf \n"), DOUBLE(dCloseTime / 1000000.0));
#endif
	        		MO_TimerSetIntter(LONG(dCloseTime), 1);//�p�ɨ���ܰ����
	        	}
                }
	}
	std::vector<DATA_2MOVE> DATA_2DO;
	DATA_2DO.clear();
	for (rptIter = m_ptVec.rbegin(); rptIter != m_ptVec.rend(); rptIter++)
	{
		if ((iData != 0) && (iOdd == 0))
		{
			MO_Do2dDataLine((*rptIter).x, (*rptIter).y, DATA_2DO);
		}
		else if ((iData == 0) && (iOdd == 0))
		{
			MO_Do2dDataCir((*rptIter).x, (*rptIter).y, cPtCen2.x, cPtCen2.y, bRev, DATA_2DO);
		}
		else
		{
			if (iOdd % 2 == 0)
			{
				MO_Do2dDataCir((*rptIter).x, (*rptIter).y, cPtCen2.x, cPtCen2.y, bRev, DATA_2DO);
			}
			else
			{
				MO_Do2dDataCir((*rptIter).x, (*rptIter).y, lCenX, lCenY, bRev, DATA_2DO);
			}
		}
		iOdd++;
	}
	LA_AbsToOppo2Move(DATA_2DO);
	for (UINT i = 0; i < DATA_2DO.size(); i++)
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
	while (MO_Timer(3, 0, 0))
	{
		if (g_bIsStop == 1)
		{
			break;
		}
		Sleep(1);
	}
	MO_StopGumming();//����X��
	if (!g_bIsStop)
	{
		//*************************��R�^��z�b�t�ץثe���X�ʳt�ת��⭿******************
		MO_Do3DLineMove(0, 0, (lZ - lZBackDistance) - lZ, lWorkVelociy * 2, lAcceleration,
			lInitVelociy);//Z�b��^
		PreventMoveError();//����b�d�X��
	}
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
#endif // MOVE
}
//����
//RefX����I/OffSetX�����q(��쪺�����q)/Andgle���઺����/CameraToTipOffsetX (ccd��w�Y�������q)
void CAction::LA_CorrectLocation(LONG &PointX, LONG &PointY, LONG RefX, LONG RefY, DOUBLE OffSetX, DOUBLE OffSetY, DOUBLE Andgle, DOUBLE CameraToTipOffsetX, DOUBLE CameraToTipOffsetY, BOOL Mode, LONG lSubOffsetX, LONG lSubOffsetY)
{
	double X, Y;
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
	if (Mode)
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
	if (dRadius == 0)
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
	while (1)
	{
		lDistance = LONG(lDistance - dWidth);
		if (lDistance < dWidth)
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
	for (ptIter = m_ptVec.begin(); ptIter != m_ptVec.end(); ptIter++)
	{
		if ((iData != 0) && (cnt == m_ptVec.size() - 1))
		{
			MO_Do2dDataLine((*ptIter).x, (*ptIter).y, DATA_2DO);
		}
		else if ((iData == 0) && (cnt == m_ptVec.size() - 1))
		{
			MO_Do2dDataCir((*ptIter).x, (*ptIter).y, cPtCen2.x, cPtCen2.y, bRev, DATA_2DO);
		}
		else
		{
			if (cnt % 2 == 0)
			{
				MO_Do2dDataCir((*ptIter).x, (*ptIter).y, cPtCen2.x, cPtCen2.y, bRev, DATA_2DO);
			}
			else
			{
				MO_Do2dDataCir((*ptIter).x, (*ptIter).y, lCenX, lCenY, bRev, DATA_2DO);
			}
		}
		cnt++;
	}
	LA_AbsToOppo2Move(DATA_2DO);

	if ((iData != 0))
	{
		if (!g_bIsStop)
		{
			MO_Do3DLineMove(DATA_2DO.at(0).EndP.x, DATA_2DO.at(0).EndP.y, 0, lWorkVelociy, lAcceleration, lInitVelociy);//���u����
			PreventMoveError();
		}
		PauseDoGlue();//�Ȱ���_���~��X��(g_bIsPause=0) �X��
	}
	else
	{
		if (!g_bIsStop)
		{
			MO_Do2DArcMove(DATA_2DO.at(0).EndP.x, DATA_2DO.at(0).EndP.y, DATA_2DO.at(0).CirCentP.x, DATA_2DO.at(0).CirCentP.y,
				lInitVelociy, lWorkVelociy, DATA_2DO.at(0).CirRev);//��l�b��
			PreventMoveError();
		}
		PauseDoGlue();//�Ȱ���_���~��X��(g_bIsPause=0) �X��
	}

	for (UINT i = 1; i < DATA_2DO.size(); i++)
	{
		DATA_2Do[i - 1] = DATA_2DO.at(i);
	}
	MO_DO2Curve(DATA_2Do, DATA_2DO.size() - 1, lWorkVelociy);
	PreventMoveError();//�����X�ʿ��~
	Sleep(200);
	DATA_2DO.clear();

	PauseStopGlue();//�Ȱ��ɰ����(g_bIsPause=1)
	MO_StopGumming();//����X��
	if (!g_bIsStop)
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
	if (dRadius == 0)
	{
		return;
	}
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
		cPt1.x = LONG(dDistance*cos(dAngCos) + cPt4.x);
		cPt1.y = LONG(dDistance*sin(dAngSin) + cPt4.y);
		m_ptVec.push_back(cPt1);
		cPt2.x = LONG(dRadius*cos(dAngCenCos + M_PI_2) + cPtCen.x);
		cPt2.y = LONG(dRadius*sin(dAngCenSin + M_PI_2) + cPtCen.y);
		m_ptVec.push_back(cPt2);
		dDistance = dDistance - dWidth;
		if (dDistance < dWidth)
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
	if (dRadius == 0)
	{
		return;
	}
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
#ifdef PRINTF
			_cwprintf(_T("��ݼe�׹L�j \n"));
#endif
			m_ptVec.push_back(cPt1);
			break;
		}
		dRadius = dRadius - dWidth*sqrt(2);
		dDistance = dDistance - dWidth;
		if (iBuff*dWidth >  dWidth2) //dWidth2��ݼe��
		{
			dDistance += dWidth;
			cPt1.x = LONG(dDistance*cos(dAngCos) + cPt4.x);
			cPt1.y = LONG(dDistance*sin(dAngSin) + cPt4.y);
			m_ptVec.push_back(cPt1);
			break;
		}
		if (dDistance < dWidth)
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
		if (dDistance < dWidth)
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
	if (dRadius == 0)
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
	while (1)
	{
		if ((iBuff == 1) && (lDistance - (2 * dWidth2) < 0))
		{
#ifdef PRINTF
			_cwprintf(_T("��ݼe�׹L�j \n"));
#endif
			break;
		}
		dRadius = dRadius - dWidth;
		lDistance = LONG(lDistance - dWidth);
		if (iBuff * dWidth > dWidth2)
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
	for (UINT i = 1; i < m_ptVec.size(); i++)
	{
		if (i == m_ptVec.size() - 1)
		{
			if (i % 2 == 0)
			{
				MO_Do2dDataCir(m_ptVec.at(i).x, m_ptVec.at(i).y, cPtCen1.x, cPtCen1.y, bRev, DATA_2DO);//�W�b��
			}
			else
			{
				MO_Do2dDataCir(m_ptVec.at(i).x, m_ptVec.at(i).y, cPtCen2.x, cPtCen2.y, bRev, DATA_2DO);//�U�b��
			}
		}
		else
		{
			if (i % 2 != 0)
			{
				MO_Do2dDataCir(m_ptVec.at(i).x, m_ptVec.at(i).y, cPtCen1.x, cPtCen1.y, bRev, DATA_2DO);//�W�b��
			}
			else
			{
				MO_Do2dDataCir(m_ptVec.at(i).x, m_ptVec.at(i).y, cPtCen2.x, cPtCen2.y, bRev, DATA_2DO);//�U�b��
			}
		}
	}
	LA_AbsToOppo2Move(DATA_2DO);
	PauseDoGlue();//�Ȱ���_���~��X��(g_bIsPause=0) �X��
	for (UINT i = 0; i < DATA_2DO.size(); i++)
	{
		DATA_2Do[i] = DATA_2DO.at(i);
	}
	MO_DO2Curve(DATA_2Do, DATA_2DO.size(), lWorkVelociy);
	PreventMoveError();//�����X�ʿ��~
	Sleep(200);
	DATA_2DO.clear();

	PauseStopGlue();//�Ȱ��ɰ����(g_bIsPause=1)
	MO_StopGumming();//����X��
	if (!g_bIsStop)
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
	if (dRadius == 0)
	{
		return;
	}
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
		cPt1.x = LONG(dDistance*cos(dAngCos) + cPt4.x);
		cPt1.y = LONG(dDistance*sin(dAngSin) + cPt4.y);
		m_ptVec.push_back(cPt1);
		cPt2.x = LONG(dRadius*cos(dAngCenCos + M_PI_2) + cPtCen.x);
		cPt2.y = LONG(dRadius*sin(dAngCenSin + M_PI_2) + cPtCen.y);
		m_ptVec.push_back(cPt2);
		dDistance = dDistance - dWidth;
		if (dDistance < dWidth)
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
	if (!g_bIsStop)
	{
		MO_Do3DLineMove(0, 0, (lZ - lZBackDistance) - lZ, lWorkVelociy, lAcceleration, lInitVelociy);//Z�b��^
		PreventMoveError();//����b�d�X��
	}
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
			if (!g_bIsStop)
			{
				lNowZ = MO_ReadLogicPosition(2);
				MO_Do3DLineMove(0, 0, lZ - lNowZ, lWorkVelociy, lAcceleration, lInitVelociy);//Z�b���U
				PreventMoveError();//����b�d�X��
			}
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
	if (dRadius == 0)
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
	while (1)
	{
		lDistance = LONG(lDistance - dWidth);
		if (lDistance < dWidth)
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
	if (!g_bIsStop)
	{
		MO_Do3DLineMove(lCenX - MO_ReadLogicPosition(0), lCenY - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration,
			lInitVelociy);//���u���ʦܶ��
		PreventMoveError();
	}
	if (!g_bIsStop)
	{
		MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration,
			lInitVelociy);//Z�b���U
		PreventMoveError();//����b�d�X��
	}

	std::vector<DATA_2MOVE> DATA_2DO;
	DATA_2DO.clear();
	for (rptIter = m_ptVec.rbegin(); rptIter != m_ptVec.rend(); rptIter++)
	{
		if ((iData != 0) && (iOdd == 0))
		{
			MO_Do2dDataLine((*rptIter).x, (*rptIter).y, DATA_2DO);
		}
		else if ((iData == 0) && (iOdd == 0))
		{
			MO_Do2dDataCir((*rptIter).x, (*rptIter).y, cPtCen2.x, cPtCen2.y, bRev, DATA_2DO);
		}
		else
		{
			if (iOdd % 2 == 0)
			{
				MO_Do2dDataCir((*rptIter).x, (*rptIter).y, cPtCen2.x, cPtCen2.y, bRev, DATA_2DO);
			}
			else
			{
				MO_Do2dDataCir((*rptIter).x, (*rptIter).y, lCenX, lCenY, bRev, DATA_2DO);
			}
		}
		iOdd++;
	}
	LA_AbsToOppo2Move(DATA_2DO);
	PauseDoGlue();//�Ȱ���_���~��X��(g_bIsPause=0) �X��
	for (UINT i = 0; i < DATA_2DO.size(); i++)
	{
		DATA_2Do[i] = DATA_2DO.at(i);
	}
	MO_DO2Curve(DATA_2Do, DATA_2DO.size(), lWorkVelociy);
	PreventMoveError();//�����X�ʿ��~
	Sleep(200);
	DATA_2DO.clear();

	PauseStopGlue();//�Ȱ��ɰ����(g_bIsPause=1)
	MO_StopGumming();//����X��
	if (!g_bIsStop)
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
	if (dRadius == 0)
	{
		return;
	}
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
		cPt1.x = LONG(dDistance*cos(dAngCos) + cPt4.x);
		cPt1.y = LONG(dDistance*sin(dAngSin) + cPt4.y);
		m_ptVec.push_back(cPt1);
		cPt2.x = LONG(dRadius*cos(dAngCenCos + M_PI_2) + cPtCen.x);
		cPt2.y = LONG(dRadius*sin(dAngCenSin + M_PI_2) + cPtCen.y);
		m_ptVec.push_back(cPt2);
		dDistance = dDistance - dWidth;
		if (dDistance < dWidth)
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
	for (ptIter = m_ptVec.begin(); ptIter != m_ptVec.end(); ptIter++)
	{
		MO_Do2dDataLine((*ptIter).x, (*ptIter).y, DATA_2DO);
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
	if (dRadius == 0)
	{
		return;
	}
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
#ifdef PRINTF
			_cwprintf(_T("��ݼe�׹L�j \n"));
#endif
			m_ptVec.push_back(cPt1);
			break;
		}
		dRadius = dRadius - dWidth*sqrt(2);
		dDistance = dDistance - dWidth;
		if (iBuff*dWidth >  dWidth2)  //dWidth2��ݼe��
		{
			dDistance += dWidth;
			cPt1.x = LONG(dDistance*cos(dAngCos) + cPt4.x);
			cPt1.y = LONG(dDistance*sin(dAngSin) + cPt4.y);
			m_ptVec.push_back(cPt1);
			break;
		}
		if (dDistance < dWidth)
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
		if (dDistance < dWidth)
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
	for (ptIter = m_ptVec.begin(); ptIter != m_ptVec.end(); ptIter++)
	{
		MO_Do2dDataLine((*ptIter).x, (*ptIter).y, DATA_2DO);
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
	if (dRadius == 0)
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
	while (1)
	{
		if ((iBuff == 1) && (lDistance - (2 * dWidth2) < 0))
		{
#ifdef PRINTF
			_cwprintf(_T("��ݼe�׹L�j \n"));
#endif
			break;
		}
		dRadius = dRadius - dWidth;
		lDistance = LONG(lDistance - dWidth);
		if (iBuff * dWidth > dWidth2)
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
	for (UINT i = 1; i < m_ptVec.size(); i++)
	{
		if (i == m_ptVec.size() - 1)
		{
			if (i % 2 == 0)
			{
				MO_Do2dDataCir(m_ptVec.at(i).x, m_ptVec.at(i).y, cPtCen1.x, cPtCen1.y, bRev, DATA_2DO);//�W�b��
			}
			else
			{
				MO_Do2dDataCir(m_ptVec.at(i).x, m_ptVec.at(i).y, cPtCen2.x, cPtCen2.y, bRev, DATA_2DO);//�U�b��
			}
		}
		else
		{
			if (i % 2 != 0)
			{
				MO_Do2dDataCir(m_ptVec.at(i).x, m_ptVec.at(i).y, cPtCen1.x, cPtCen1.y, bRev, DATA_2DO);//�W�b��
			}
			else
			{
				MO_Do2dDataCir(m_ptVec.at(i).x, m_ptVec.at(i).y, cPtCen2.x, cPtCen2.y, bRev, DATA_2DO);//�U�b��
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
void CAction::MO_Do2dDataLine(LONG EndPX, LONG EndPY, std::vector<DATA_2MOVE>& str)
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
//��R�Ψ�b�s��t��(����--��)
void CAction::MO_Do2dDataCir(LONG EndPX, LONG EndPY, LONG CenX, LONG CenY, BOOL bRev, std::vector<DATA_2MOVE>& str)
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
/*����y����۹�y��3�b�s�򴡸ɨϥ�*/
void CAction::LA_AbsToOppo3Move(std::vector<DATA_3MOVE> &str)
{
#ifdef LA
	std::vector<DATA_3MOVE> vecBuf;
	DATA_3MOVE mData;
	LONG lNowX = MO_ReadLogicPosition(0), lNowY = MO_ReadLogicPosition(1), lNowZ = MO_ReadLogicPosition(2);
	vecBuf.clear();
	mData.EndPX = str.at(0).EndPX - lNowX;
	mData.EndPY = str.at(0).EndPY - lNowY;
	mData.EndPZ = str.at(0).EndPZ - lNowZ;
	mData.Distance = LONG(sqrt(pow(str.at(0).EndPX, 2) + pow(str.at(0).EndPY, 2)));
	vecBuf.push_back(mData);
	for (UINT i = 1; i < str.size(); i++)
	{
		if ((str.at(i).EndPX == LA_SCANEND) && (str.at(i).EndPY == LA_SCANEND) && (str.at(i).EndPZ == LA_SCANEND))
		{
			mData.EndPX = str.at(i).EndPX;
			mData.EndPY = str.at(i).EndPY;
			mData.EndPZ = str.at(i).EndPZ;
			//vecBuf.push_back(mData);
			//�p�G-99999���O�̫�@�����
			if (i + 1 < str.size())
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
	for (UINT i = 0; i < str.size() - 1; i++)
	{
		DATA_3Do[i] = { 0 };
		DATA_3Do[i] = str[i + 1];
	}
#endif
}
#endif
/*����y����۹�y��2�b�s�򴡸ɨϥ�*/
#ifdef MOVE
void CAction::LA_AbsToOppo2Move(std::vector<DATA_2MOVE>& str)
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
	for (UINT i = 1; i < str.size(); i++)
	{
		mData.Speed = str.at(i).Speed;
		mData.Type = str.at(i).Type;
		mData.EndP = str.at(i).EndP - str.at(i - 1).EndP;
		mData.Distance = (LONG)round(sqrt(pow(mData.EndP.x, 2) + pow(mData.EndP.y, 2)));
		if (str.at(i).Type)
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