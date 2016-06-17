/*
*�ɮצW��:Action.h(3D��)
*���e²�z:�B�ʩR�OAPI�A�ԲӰѼƽЬd��excel
*�Iauthor �@�̦W��:R
*�Idata ��s���:2016/06/07
*@��s���e�u�qz�ȧ��ܮɡA�T�b�P�ʲ��ʡA��Ӫ���x,y�����ʦA����z�b*/
#pragma once
#include <vector>
#define _USE_MATH_DEFINES
#include <math.h>
static LPVOID pAction;
class CAction
{
public:     //�ܼ�
    BOOL    g_bIsPause;       //�Ȱ���
	BOOL    g_bIsStop;
	BOOL    g_bIsDispend;
	UINT    g_iNumberGluePort;
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
	void DecideLineEndMove(LONG lCloseOffDelayTime, LONG lCloseONDelayTime, LONG lZBackDistance,BOOL bZDisType, LONG lHighVelocity, LONG lDistance, LONG lHigh, LONG lLowVelocity, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
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
	//���n�I�ʧ@--(���n�I�y��X,Y,Z,�ƽ��ɶ�,�����ᵥ�ݮɶ�,�X�ʳt��,�[�t��,��t��)
	void DecideParkPoint(LONG lX, LONG lY, LONG lZ, LONG lTimeGlue, LONG lWaitTime, LONG lStayTime, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
	//���I���k�ʧ@--(���I�_�k�t��1,���I�_�k�t��2,�_�k�b(7),�����q(0))
	void DecideInitializationMachine(LONG lSpeed1, LONG lSpeed2, LONG lAxis, LONG lMoveX, LONG lMoveY, LONG lMoveZ);
    //��R�ʧ@(�u�q�}�lX,Y,Z�A�u�q����X,Y,Z�AZ�b�W�ɶZ���AZ�b���A(0�����m/1�۹��m)�A��R�Φ�(1~7)�A�e��(mm)�A��ݼe��(mm)�A�X�ʳt�סA�[�t�סA��t��)
    void DecideFill(LONG lX1, LONG lY1, LONG lZ1, LONG lX2, LONG lY2, LONG lZ2, LONG lZBackDistance, BOOL bZDisType, int iType, LONG lWidth, LONG lWidth2, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
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
    //�H���Ψ��-�n��t����
    void HMNegLim(LONG lX, LONG lY, LONG lZ);
    //�H���Ψ��-�n�饿����(x,y,z���̤j�u�@�d��)
    void HMPosLim(LONG lX, LONG lY, LONG lZ);
private:    //�ۦ�B�Ψ��
	void AttachPointMove(LONG lX, LONG lY, LONG lZ, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy, BOOL bIntt); //����--- �����I�ʧ@
	void DoGlue(LONG lTime, LONG lDelayTime, LPTHREAD_START_ROUTINE GummingTimeOutThread);//�X���A�h��������ɶ�(�t�X������ϥ�)
	void PreventMoveError();//����b�d�X��
	void PreventGlueError();//����X���X��
    void PauseDoGlue();//�Ȱ��^�_�᭫�s�(Ū���Ȱ��ѼơA��ѼƬ�0�ɥX���A�B�I�����n���}�C)
    void PauseStopGlue();//�Ȱ��ɰ����(Ū���Ȱ��ѼơA��ѼƬ�1���_���A���I�������ѼƬ�0�]�_���C)
	void GelatinizeBack(int iType, LONG lXarEnd, LONG lYarEnd, LONG lZarEnd, LONG lXarUp, LONG lYarUp, LONG lLineStop, LONG lStopZar, LONG lBackZar, LONG lLowSpeed, LONG lHighSpeed, LONG lAcceleration, LONG lInitSpeed);//��^�]�w
	void LineGetToPoint(LONG &lXClose, LONG &lYClose, LONG lX0, LONG lY0, LONG lX1, LONG lY1, LONG &lLineClose);//���u�Z���ഫ���y���I
	void LineGetToPoint(LONG &lXClose, LONG &lYClose, LONG &lZClose, LONG lX0, LONG lY0, LONG lX1, LONG lY1, LONG lZ0, LONG lZ1, LONG &lLineClose);//���u�Z���ഫ���y���I--�h��3D
	CString TRoundCCalculation(CString Origin, CString End, CString Between);//�T�I�p����
	LONG CStringToLong(CString csData, int iChoose);//�r�������ơA�ϥΪ������@�w�n�O"�A"
    DOUBLE AngleCount(DOUBLE LocatX, DOUBLE LocatY, DOUBLE LocatX1, DOUBLE LocatY1, DOUBLE LocatX2, DOUBLE LocatY2, BOOL bRev);//�T�I���o����(�V�q����)(���x,y�y��x1,y1,�y��x2,y2,���f��)
    void ArcGetToPoint(LONG &lArcX, LONG &lArcY, LONG lDistance, LONG lX, LONG lY, LONG lCenX, LONG lCenY, LONG lRadius, BOOL bDir);//�꩷���Z�����I����
    void AttachFillType1(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lZ, LONG lZBackDistance, LONG lWidth, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);//����--- ��R�κA1
    void AttachFillType2(LONG lX1, LONG lY1, LONG lCenX, LONG lCenY, LONG lZ, LONG lZBackDistance, LONG lWidth, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);//����--- ��R�κA2
    void AttachFillType3(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lZ, LONG lZBackDistance, LONG lWidth, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);//����--- ��R�κA3
    void AttachFillType4(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lZ, LONG lZBackDistance, LONG lWidth, LONG lWidth2, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);//����--- ��R�κA4
    void AttachFillType5(LONG lX1, LONG lY1, LONG lCenX, LONG lCenY, LONG lZ, LONG lZBackDistance, LONG lWidth, LONG lWidth2, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);//����--- ��R�κA5
    void AttachFillType6(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lZ, LONG lZBackDistance, LONG lWidth, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);//����--- ��R�κA6
    void AttachFillType7(LONG lX1, LONG lY1, LONG lCenX, LONG lCenY, LONG lZ, LONG lZBackDistance, LONG lWidth, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);//����--- ��R�κA7
protected:  //�����
	static DWORD WINAPI GummingTimeOutThread(LPVOID);
    static DWORD WINAPI LPInterrupt(LPVOID);//���_
};


