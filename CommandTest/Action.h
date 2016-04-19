#pragma once
class CAction
{
public:     //�ܼ�
    BOOL    g_bIsStop;
    BOOL    g_bIsDispend;
    UINT    g_iNumberGluePort;
public:     //�R�c���
	CAction();
	virtual ~CAction();
public:     //�B��API
    void WaitTime(HANDLE wakeEvent, int Time);
    //���I�I���ʧ@--(���I�I��X,���I�I��Y,���I�I��Z,�X���ɶ�,�_������,Z�b�^�ɰ���(�۹�)�̰��I,Z�b�Z��(�۹�),���t��,�C�t��,�X�ʳt��,�[�t��,��t��)
    void DecidePointGlue(LONG lX, LONG lY, LONG lZ, LONG lDoTime, LONG lDelayStopTime,LONG lZBackDistance, LONG lZdistance, LONG lHighVelocity, LONG lLowVelocity, LONG lWorkVelociy,LONG lAcceleration, LONG lInitVelociy);
    //�u�q�}�l�ʧ@--(�u�q�}�lX,�u�q�}�lY,�u�q�}�lZ,���ʫe����,�}���e�q�_�I���ʶZ��(�]�m�Z��),�X�ʳt��,�[�t��,��t��)
    void DecideLineStartMove(LONG lX, LONG lY, LONG lZ, LONG lStartDelayTime, LONG lStartDistance, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
    //�u�q���I�ʧ@--(�u�q���IX,�u�q���IY,�u�q���IZ,��u�q���I�����d�ɶ�(�`�I�ɶ�),�X�ʳt��,�[�t��,��t��)
    void DecideLineMidMove(LONG lX, LONG lY, LONG lZ, LONG lMidDelayTime, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
    //�u�q�����ʧ@--(�u�q����X,�u�q����Y,�u�q����Z,������b�����I���d�ɶ�(���d�ɶ�),�Z�������I�h���Z������(�����Z��),�����᪺����ɶ�(��������),Z�b�^�ɬ۹�Z��,�I�������]�w(���t��),��^����,z��^����,��^�t��(�C�t),��^����,�X�ʳt��,�[�t��,��t��)
    void DecideLineEndMove(LONG lX, LONG lY, LONG lZ, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime, LONG lZBack, LONG lHighVelocity, LONG lDistance, LONG lHigh, LONG lLowVelocity, int iType, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
    //��ΰʧ@--(���I�y��X,Y,���I�y��X2,Y2,�X�ʳt��,��t��)
    void DecideCircle(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lWorkVelociy, LONG lInitVelociy);
    //�꩷�ʧ@--(�꩷�y��X,Y,�꩷�y��X2,Y2,�X�ʳt��,��t��)
    void DecideArc(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lWorkVelociy, LONG lInitVelociy);
    //�����I�ʧ@--(�����I�y��X,Y,Z,�X�ʳt��,�[�t��,��t��)
    void DecideVirtualPoint(LONG lX, LONG lY, LONG lZ, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
    //�����I�ʧ@--(�����I�y��X,Y,Z,���ݮɶ�,�X�ʳt��,�[�t��,��t��)
    void DecideWaitPoint(LONG lX, LONG lY, LONG lZ, LONG lWaitTime, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
    //���n�I�ʧ@--(���n�I�y��X,Y,Z,�ƽ��ɶ�,�����ᵥ�ݮɶ�,�X�ʳt��,�[�t��,��t��)
    void DecideParkPoint(LONG lX, LONG lY, LONG lZ, LONG lTimeGlue, LONG lWaitTime, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
    //���I���k�ʧ@--(���I�_�k�t��1,���I�_�k�t��2,�_�k�b(7),�����q(0))
    void DecideInitializationMachine(LONG lSpeed1, LONG lSpeed2, LONG lAxis, LONG lMove);
    //��X-16�ӿ�X(��ܰ�(0~15),�}������(0~1))
    BOOL DecideOutPutSign(int iPort, BOOL bChoose);
    //��J-12�ӿ�J(��ܰ�(0~11),�}������(0~1))
    BOOL DecideInPutSign(int iPort, BOOL bChoose);
private:    //�ۦ�B�Ψ��
    void AttachPointMove(LONG lX, LONG lY, LONG lZ, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy); //����--- �����I�ʧ@
    void DoGlue(LONG lTime, LONG lDelayTime, LPTHREAD_START_ROUTINE GummingTimeOutThread);//�X���A�h��������ɶ�(�t�X������ϥ�)
    void PreventMoveError();//����b�d�X��
    void PreventGlueError();//����X���X��
    void GelatinizeBack(int iType, LONG lXarEnd, LONG lYarEnd, LONG lZarEnd, LONG lXarUp, LONG lYarUp, LONG lLineStop, LONG lStopZar, LONG lBackZar, LONG lLowSpeed, LONG lHighSpeed, LONG lAcceleration, LONG lInitSpeed);//��^�]�w
    void LineGetToPoint(LONG &lXClose, LONG &lYClose, LONG lX0, LONG lY0, LONG lX1, LONG lY1, LONG &lLineClose);//���u�Z���ഫ���y���I
    CString TRoundCCalculation(CString Origin, CString End, CString Between);//�T�I�p����
    LONG CStringToLong(CString csData, int iChoose);//�r�������ơA�ϥΪ������@�w�n�O"�A"
protected:  //�����
    static DWORD WINAPI GummingTimeOutThread(LPVOID);
};


