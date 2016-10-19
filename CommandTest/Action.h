/*
*�ɮצW��:Action.h(3D��)
*���e²�z:�B�ʩR�OAPI�A�ԲӰѼƽЬd��excel
*�Iauthor �@�̦W��:R
*�Idata ��s���:2016/09/26
*@��s���e�T�b��b�s��t��&�p�gAPI*/
#pragma once
#include <vector>
#define _USE_MATH_DEFINES
#include <math.h>
#ifdef MOVE
#include "mcc.h"
#endif
//static LPVOID  pAction;
static int     g_LaserErrCnt;//�p�g�ɶ����p�ƾ�(�������~��)
static int     g_LaserNuCnt;//�p�g�ɶ����p�ƾ�(�����ƶq��)
class CAction
{
public:     //�ܼ�
	LPVOID  pAction;          //this���w
	BOOL    g_bIsPause;       //�Ȱ���
	BOOL    g_bIsStop;        //�����
	BOOL    g_bIsDispend;     //�����I����
	BOOL    g_bIsGetLAend;     
	int     g_iNumberGluePort;//�I����ҥμƶq
	LONG    g_OffSetLaserX;//�p�g��x�b�����q
	LONG    g_OffSetLaserY;//�p�g��y�b�����q
	LONG    g_OffSetLaserZ;//�p�g��z�b�����q(B�I��Z���ר�p�g�k�sZ���ת��첾��(+))
	LONG    g_HeightLaserZero;//�p�g��Z�b�k�s�����ᰪ��(���y����)
	LONG    g_laserBuff;//�p�g�μȦs��(test)
	LONG    g_OffSetScan;//�p�g���y���v��
	LONG    g_LaserAveBuffZ;//�p�g�Υ����Ȧs��(�����mz��)
	int     g_LaserCnt;//�p�g�u�q�p�ƾ�(���y��)
	BOOL    g_LaserAverage;//�p�g����(1�ϥ�/0���ϥ�)
	BOOL    g_interruptLock;//���_��
	BOOL    g_getHeightFlag;//�p�g�����X�СG���\����
	std::vector<UINT>  LA_m_iVecSP;//�D�n�p�gvector(SP:Scan End)
	static BOOL    g_YtimeOutGlueSet;//Y�p�ɾ����_�ɥX�_������
	static BOOL    g_ZtimeOutGlueSet;//Z�p�ɾ����_�ɥX�_������
	HANDLE m_hComm;
	CString ComportNo = _T("COM6"); //RS232 COMPort

#ifdef MOVE
	std::vector<DATA_3MOVE> LA_m_ptVec;//�p�g�s����I�x�svector
	std::vector<DATA_2MOVE> LA_m_ptVec2D;//�p�g�s����I�x�svector
	DATA_3MOVE DATA_3Do[512];//�s����Ȧs
	DATA_2MOVE DATA_2Do[128]; 
#endif
#ifdef LA
	DATA_3MOVE DATA_3ZERO_B;//�p�g�k�s_�w�YB�I
	DATA_3MOVE DATA_3ZERO_LA;//�p�g�k�s_�p�gB�I
#endif
public:     //�R�c���
	CAction();
	virtual ~CAction();
public:     //�B��API
	void WaitTime(HANDLE wakeEvent, int Time);
	//���I�I���ʧ@--(���I�I��X,���I�I��Y,���I�I��Z,�X���ɶ�,�_������,Z�b�^�ɰ���(�۹�)�̰��I,Z�b�Z��(�۹�),���t��,�C�t��,�X�ʳt��,�[�t��,��t��)
	void DecidePointGlue(LONG lX, LONG lY, LONG lZ, LONG lDoTime, LONG lDelayStopTime,LONG lZBackDistance,BOOL bZDisType, LONG lZdistance, LONG lHighVelocity, LONG lLowVelocity, LONG lWorkVelociy,LONG lAcceleration, LONG lInitVelociy);
	//�u�q�}�l�ʧ@--(�u�q�}�lX,�u�q�}�lY,�u�q�}�lZ,���ʫe����,�X�ʳt��,�[�t��,��t��)
	void DecideLineStartMove(LONG lX, LONG lY, LONG lZ, LONG lStartDelayTime, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
	//�u�q���I�ʧ@--(�u�q���IX,�u�q���IY,�u�q���IZ,��u�q���I�����d�ɶ�(�`�I�ɶ�),�X�ʳt��,�[�t��,��t��)
	void DecideLineMidMove(LONG lX, LONG lY, LONG lZ, LONG lMidDelayTime, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
	//�u�q�����ʧ@--(�u�q����X,�u�q����Y,�u�q����Z,������b�����I���d�ɶ�(���d�ɶ�),�Z�������I�h���Z������(�����Z��),�����᪺����ɶ�(��������),Z�b�^�ɶZ��,Z�b�^�ɪ����A,�u�q�����]�w(���t��),��^����,z��^����,��^�t��(�C�t),��^����,�X�ʳt��,�[�t��,��t��)
	void DecideLineEndMove(LONG lX, LONG lY, LONG lZ, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime, LONG lZBackDistance,BOOL bZDisType, LONG lHighVelocity, LONG lDistance, LONG lHigh, LONG lLowVelocity, int iType, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
	//(�{�b)�u�q�����ʧ@�h��--(������b�����I���d�ɶ�(���d�ɶ�),�����᪺����ɶ�(��������),Z�b�^�ɶZ��,Z�b�^�ɪ����A,�u�q�����]�w(���t��),��^����,z��^����,��^�t��(�C�t),�X�ʳt��,�[�t��,��t��)
	void DecideLineEndMove(LONG lCloseOffDelayTime, LONG lCloseONDelayTime, LONG lZBackDistance,BOOL bZDisType, LONG lHighVelocity, LONG lDistance, LONG lHigh, LONG lLowVelocity, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy,BOOL bGeneral=0);
	//�u�q�}�l�줤���I�ʧ@--(�u�q�}�lX,Y,Z,�u�q���IX,Y,Z,���ʫe����,�}���e�q�_�I���ʶZ��(�]�m�Z��),��u�q���I�����d�ɶ�(�`�I�ɶ�ms),�X�ʳt��,�[�t��,��t��)
	void DecideLineSToP(LONG lX, LONG lY, LONG lZ, LONG lX2, LONG lY2, LONG lZ2, LONG lStartDelayTime, LONG lStartDistance, LONG lMidDelayTime, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
	//�u�q�}�l�쵲���ʧ@--(�u�q�}�lX,Y,Z,�����IX,Y,Z,���ʫe����,�}���e�q�_�I���ʶZ��(�]�m�Z��),������b�����I���d�ɶ�(���d�ɶ�),�Z�������I�h���Z������(�����Z��),�����᪺����ɶ�(��������),Z�b�^�ɶZ��,Z�b�^�ɫ��A,�I�������]�w(���t��),��^����,z��^����,��^�t��(�C�t),��^����,�X�ʳt��,�[�t��,��t��)
	void DecideLineSToE(LONG lX, LONG lY, LONG lZ, LONG lX2, LONG lY2, LONG lZ2, LONG lStartDelayTime, LONG lStartDistance, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime, LONG lZBackDistance, BOOL bZDisType, LONG lHighVelocity, LONG lDistance, LONG lHigh, LONG lLowVelocity, int iType, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
	//��ΰʧ@--(���I�y��X,Y,���I�y��X2,Y2,�X�ʳt��,��t��)
	void DecideCircle(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lWorkVelociy, LONG lInitVelociy);
	//�꩷�ʧ@--(�꩷�y��X,Y,�꩷�y��X2,Y2,�X�ʳt��,��t��)
	void DecideArc(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lWorkVelociy, LONG lInitVelociy);
	//�u�q�����ʧ@--(��ΰʧ@--(��ήy��X1,Y1,��ήy��X2,Y2,�u�q����X3,Y3,������b�����I���d�ɶ�(���d�ɶ�),�Z�������I�h���Z������(�����Z��),�����᪺����ɶ�(��������),Z�b�^�ɬ۹�Z��,Z�b���A(0�����m/1�۹��m),�I�������]�w(���t��),��^����,z��^����,��^�t��(�C�t),��^����,�X�ʳt��,�[�t��,��t��)
	void DecideCircleToEnd(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lX3, LONG lY3, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime, LONG lZBackDistance, BOOL bZDisType, LONG lHighVelocity, LONG lDistance, LONG lHigh, LONG lLowVelocity, int iType, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
	//�u�q�����ʧ@--(�꩷�ʧ@--(�꩷�y��X1,Y1,�u�q����X2,Y2,������b�����I���d�ɶ�(���d�ɶ�),�Z�������I�h���Z������(�����Z��),�����᪺����ɶ�(��������),Z�b�^�ɬ۹�Z��,Z�b���A(0�����m/1�۹��m),�I�������]�w(���t��),��^����,z��^����,��^�t��(�C�t),��^����,�X�ʳt��,�[�t��,��t��)
	void DecideArcleToEnd(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime, LONG lZBackDistance, BOOL bZDisType, LONG lHighVelocity, LONG lDistance, LONG lHigh, LONG lLowVelocity, int iType, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
	//�����I�ʧ@--(�����I�y��X,Y,Z,�X�ʳt��,�[�t��,��t��)
	void DecideVirtualPoint(LONG lX, LONG lY, LONG lZ, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
	//�����I�ʧ@--(�����I�y��X,Y,Z,���ݮɶ�,�X�ʳt��,�[�t��,��t��)
	void DecideWaitPoint(LONG lX, LONG lY, LONG lZ, LONG lWaitTime, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
	//���n�I�ʧ@--(���n�I�y��X,Y,Z,�ƽ��ɶ�,�����ᵥ�ݮɶ�,Z�b�^�ɰ���(�۹�)�̰��I,Z�b�Z��(�۹�),���t��,�C�t��,�X�ʳt��,�[�t��,��t��)
	void DecideParkPoint(LONG lX, LONG lY, LONG lZ, LONG lTimeGlue, LONG lWaitTime, LONG lStayTime, LONG lZBackDistance, BOOL bZDisType, LONG lZdistance, LONG lHighVelocity, LONG lLowVelocity, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
	//���I���k�ʧ@--(���I�_�k�t��1,���I�_�k�t��2,�_�k�b(7),�����q(0))
	void DecideInitializationMachine(LONG lSpeed1, LONG lSpeed2, LONG lAxis, LONG lMoveX, LONG lMoveY, LONG lMoveZ);
    //�����I���I�_�k
    void DecideVirtualHome(LONG lX, LONG lY, LONG lZ, LONG lZBackDistance, BOOL bZDisType, LONG lWorkVelociy,LONG lAcceleration, LONG lInitVelociy);
	//�w�Y�M��ʧ@(�M��˸m��mX,Y,Z,�M��˸m�ϥΪ�io,Z�b�^�ɬ۹�Z��,Z�b���A(0�����m/1�۹��m),�X�ʳt��,�a�t��,��t��)
	void DispenClear(LONG lX, LONG lY, LONG lZ, int ClreaPort, LONG lZBackDistance, BOOL bZDisType, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
	//��R�ʧ@(�u�q�}�lX,Y,Z�A�u�q����X,Y,Z�AZ�b�Z��(�۹�)�AZ�b���A(0�����m/1�۹��m)�A��R�Φ�(1~7)�A�e��(mm)�A��ݼe��(mm)�A�u�q�I���]�w(1.���ʫe����A2.�]�m�Z���A3���d�ɶ��A5�����Z���A6��������)�A�X�ʳt�סA�[�t�סA��t��)
	void DecideFill(LONG lX1, LONG lY1, LONG lZ1, LONG lX2, LONG lY2, LONG lZ2, LONG lZBackDistance, BOOL bZDisType,int iType, LONG lWidth, LONG lWidth2, LONG lStartDelayTime, LONG lStartDistance, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
	//��X-16�ӿ�X(��ܰ�(0~15),�}������(0~1))
	BOOL DecideOutPutSign(int iPort, BOOL bChoose);
	//��J-12�ӿ�J(��ܰ�(0~11),�}������(0~1))
	BOOL DecideInPutSign(int iPort, BOOL bChoose);
	//��m����(��J�@��(X,Y,Z)�y��)
	CString NowOffSet(LONG lX, LONG lY, LONG lZ);
	//�^�ǥثe��m
	CString NowLocation();
	//CCD���ʥ�--(�I�y��X,Y,Z,�X�ʳt��,�[�t��,��t��)
	void DoCCDMove(LONG lX, LONG lY, LONG lZ, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
	//���I�_�k�A�^�����m(0,0,0)
	void BackGOZero(LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
	//�H���Ψ��-�n��t����
	void HMNegLim(LONG lX, LONG lY, LONG lZ);
	//�H���Ψ��-�n�饿����(x,y,z���̤j�u�@�d��)
	void HMPosLim(LONG lX, LONG lY, LONG lZ);
public:     //�p�gAPI
	//�p�g�k�s���s_�w�YB�I
	void LA_Butt_GoBPoint();
	//�p�g�k�s���s_�p�g��w�YB�I
	void LA_Butt_GoLAtoBPoint();
	//�p�g�]�w��l��
	void LA_SetInit();
	//���I�p�g����(�ϥ�DATA_3MOVE���c)+�����q
	BOOL LA_Dot3D(LONG lX, LONG lY, LONG &lZ, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
	//�p�g�W�s(�U����69999���k�s)
	BOOL LA_SetZero();
	//2D�s��t�ɸ�ƿ�J
	void LA_Do2DVetor(LONG lX3, LONG lY3, LONG lX2 = 0, LONG lY2 = 0, LONG lX1 = 0, LONG lY1 = 0);//!!�`�N�I�y�ж���3.2.1    
	void LA_Do2dDataLine(LONG EndPX, LONG EndPY);
	void LA_Do2dDataArc(LONG EndPX, LONG EndPY, LONG ArcX, LONG ArcY);
	void LA_Do2dDataCircle(LONG EndPX, LONG EndPY, LONG CirP1X, LONG CirP1Y, LONG CirP2X, LONG CirP2Y);
	//2D�s��t�ɱ��y
    void LA_Line2D(LONG lStartVe, LONG lStartAcc, LONG lStartInitVe, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);//���I�p�g����(�ϥ�DATA_2MOVE���c)+�����q
	//�T�b�s�򴡸�
	void LA_Line3DtoDo(int iData, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy, BOOL bDoAll = FALSE);
	//�p�g�M�����O
	void LA_Clear();
	//�p�g�����^�ǥ���z��
    void LA_AverageZ(LONG lStrX, LONG lStrY, LONG lEndX, LONG lEndY, LONG &lZ, LONG lStartVe, LONG lStartAcc, LONG lStartInitVe, LONG  lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
	//�N3Dvetor�ȧ@���ా���ץ�
	void LA_CorrectVectorToDo(LONG  lWorkVelociy, LONG lAcceleration, LONG lInitVelociy, LONG RefX = 0, LONG RefY = 0, DOUBLE OffSetX = 0, DOUBLE OffSetY = 0, DOUBLE Andgle = 0, DOUBLE CameraToTipOffsetX = 0, DOUBLE CameraToTipOffsetY = 0, BOOL Mode = 0, LONG lSubOffsetX = 0, LONG lSubOffsetY = 0);//�N3Dvetor�ȧ@���ా���ץ�
	//��R�ʧ@_�h��(�̫�@�I��m(EndX,EndY),�u�q�}�lX,Y,Z�A�u�q����X,Y,Z�AZ�b�Z��(�۹�)�AZ�b���A(0�����m/1�۹��m)�A��R�Φ�(1~7)�A�e��(mm)�A��ݼe��(mm))
	void Fill_EndPoint(LONG &lEndX, LONG &lEndY, LONG lX1, LONG lY1, LONG lZ1, LONG lX2, LONG lY2, LONG lZ2, int iType, LONG lWidth, LONG lWidth2);
public:     //������
	static DWORD WINAPI MoInterrupt(LPVOID);//�b�d���_thread
private:    //�ۦ�B�Ψ��
	void AttachPointMove(LONG lX, LONG lY, LONG lZ, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy, BOOL bIntt); //����--- �����I�ʧ@
	void DoGlue(LONG lTime, LONG lDelayTime, LPTHREAD_START_ROUTINE MoInterrupt =0);//�X���A�h��������ɶ�(�t�X������ϥ�)
	void PreventMoveError();//����b�d�X��
	void PreventGlueError();//����X���X��
	void PauseDoGlue();//�Ȱ��^�_�᭫�s�(Ū���Ȱ��ѼơA��ѼƬ�0�ɥX���A�B�I�����n���}�C)
	void PauseStopGlue();//�Ȱ��ɰ����(Ū���Ȱ��ѼơA��ѼƬ�1���_���A���I�������ѼƬ�0�]�_���C)
	void GelatinizeBack(int iType, LONG lXarEnd, LONG lYarEnd, LONG lZarEnd, LONG lXarUp, LONG lYarUp, LONG lLineStop, LONG lStopZar, LONG lBackZar, LONG lLowSpeed, LONG lHighSpeed, LONG lAcceleration, LONG lInitSpeed);//��^�]�w
	void LineGetToPoint(LONG &lXClose, LONG &lYClose, LONG lX0, LONG lY0, LONG lX1, LONG lY1, LONG &lLineClose);//���u�Z���ഫ���y���I
	void LineGetToPoint(LONG &lXClose, LONG &lYClose, LONG &lZClose, LONG lX0, LONG lY0, LONG lX1, LONG lY1, LONG lZ0, LONG lZ1, LONG &lLineClose);//���u�Z���ഫ���y���I--�h��3D
	LONG CalPreglue(LONG lStartDistance, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);//�p�ⴣ�e�X�_���Z���ήɶ�(us)
	LONG CalPreglue(LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);//�p�ⴣ�e�X�_���Z��
	LONG CalPreglueTime(LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);//�p�ⴣ�e�X�_���ɶ�(us)
	CString TRoundCCalculation(CString Origin, CString End, CString Between);//�T�I�p����
	LONG CStringToLong(CString csData, int iChoose);//�r�������ơA�ϥΪ������@�w�n�O"�A"
	DOUBLE AngleCount(DOUBLE LocatX, DOUBLE LocatY, DOUBLE LocatX1, DOUBLE LocatY1, DOUBLE LocatX2, DOUBLE LocatY2, BOOL bRev);//�T�I���o����(�V�q����)(���x,y�y��x1,y1,�y��x2,y2,���f��)
	void ArcGetToPoint(LONG &lArcX, LONG &lArcY, LONG lDistance, LONG lX, LONG lY, LONG lCenX, LONG lCenY, LONG lRadius, BOOL bDir);//�꩷���Z�����I����
	void AttachFillType1(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lZ, LONG lZBackDistance, LONG lWidth, LONG lStartDelayTime, LONG lStartDistance, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime,LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);//����--- ��R�κA1-�x��S
	void AttachFillType2(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lZ, LONG lZBackDistance, LONG lWidth, LONG lStartDelayTime, LONG lStartDistance, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);//����--- ��R�κA2-���۶�
	void AttachFillType3(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lZ, LONG lZBackDistance, LONG lWidth, LONG lStartDelayTime, LONG lStartDistance, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);//����--- ��R�κA3-�x������
	void AttachFillType4(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lZ, LONG lZBackDistance, LONG lWidth, LONG lWidth2, LONG lStartDelayTime, LONG lStartDistance, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);//����--- ��R�κA4-�x����
	void AttachFillType5(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lZ, LONG lZBackDistance, LONG lWidth, LONG lWidth2, LONG lStartDelayTime, LONG lStartDistance, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);//����--- ��R�κA5-����
	void AttachFillType6(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lZ, LONG lZBackDistance, LONG lWidth, LONG lStartDelayTime, LONG lStartDistance, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);//����--- ��R�κA6-�x�Τ�����
	void AttachFillType7(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lZ, LONG lZBackDistance, LONG lWidth, LONG lStartDelayTime, LONG lStartDistance, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);//����--- ��R�κA7-�����۶�                                                                                                                                                                      //����y����۹�y��3�b�s�򴡸ɨϥ�
	void LA_CorrectLocation(LONG &PointX, LONG &PointY, LONG RefX, LONG RefY, DOUBLE OffSetX, DOUBLE OffSetY, DOUBLE Andgle, DOUBLE CameraToTipOffsetX, DOUBLE CameraToTipOffsetY, BOOL Mode, LONG lSubOffsetX, LONG lSubOffsetY);//�p�g�α��७��
	//===============���ϥγs�򴡸ɪ���R���A=========================
	void AttachFillType2_1(LONG lX1, LONG lY1, LONG lCenX, LONG lCenY, LONG lZ,
		LONG lZBackDistance, LONG lWidth, LONG lWorkVelociy, LONG lAcceleration,
		LONG lInitVelociy);//����--- ��R�κA2-1
	void AttachFillType3_1(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lZ,
		LONG lZBackDistance, LONG lWidth, LONG lWorkVelociy, LONG lAcceleration,
		LONG lInitVelociy);//����--- ��R�κA3
	void AttachFillType4_1(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lZ,
		LONG lZBackDistance, LONG lWidth, LONG lWidth2, LONG lWorkVelociy,
		LONG lAcceleration, LONG lInitVelociy);//����--- ��R�κA4
	void AttachFillType5_1(LONG lX1, LONG lY1, LONG lCenX, LONG lCenY, LONG lZ,
		LONG lZBackDistance, LONG lWidth, LONG lWidth2, LONG lWorkVelociy,
		LONG lAcceleration, LONG lInitVelociy);//����--- ��R�κA5
	void AttachFillType6_1(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lZ,
		LONG lZBackDistance, LONG lWidth, LONG lWorkVelociy, LONG lAcceleration,
		LONG lInitVelociy);//����--- ��R�κA6
	void AttachFillType7_1(LONG lX1, LONG lY1, LONG lCenX, LONG lCenY, LONG lZ,
		LONG lZBackDistance, LONG lWidth, LONG lWorkVelociy, LONG lAcceleration,
		LONG lInitVelociy);//����--- ��R�κA7
	void AttachFillType3_End(LONG &EndX, LONG &EndY, LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lWidth, LONG lWidth2);
	void AttachFillType4_End(LONG &EndX, LONG &EndY, LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lWidth, LONG lWidth2);
	void AttachFillType5_End(LONG &EndX, LONG &EndY, LONG lX1, LONG lY1, LONG lCenX, LONG lCenY, LONG lWidth, LONG lWidth2);
#ifdef MOVE 
	void MO_Do2dDataLine(LONG EndPX, LONG EndPY, std::vector<DATA_2MOVE> &str);//��R�Ψ�b�s��t��(����--���u)
	void MO_Do2dDataCir(LONG EndPX, LONG EndPY, LONG CenX, LONG CenY, BOOL bRev, std::vector<DATA_2MOVE> &str);//��R�Ψ�b�s��t��(����--��)
	void LA_AbsToOppo3Move(std::vector<DATA_3MOVE> &str);//����y����۹�y��3�b�s�򴡸ɨϥ�
	void LA_AbsToOppo2Move(std::vector<DATA_2MOVE> &str);//����y����۹�y��2�b�s�򴡸ɨϥ�
#endif
};


