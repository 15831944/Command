/*
*�ɮצW��:Action(W�b��)
*���e²�z:�B�ʩR�OAPI�A�ԲӰѼƽЬd��excel
*�Iauthor �@�̦W��:R
*�Idata ��s���:2017/3/6
*@��s���e:nova�b�d�ϥΦb�|�b�I�����W*/
/****************************************************/
//*
//* �I�s�B�ʨ禡�ǤJ��X,Y,Z,W�ȳ���  "�w�Y�y��"
//*
/****************************************************/
#define PRINTF
#define UseIsNeedle
#ifdef UseIsNeedle
#define USEMODE_W 0
#endif // UseIsNeedle
#ifdef UseIsMachine
#define USEMODE_W 1
#endif // UseIsMachine
#pragma once
#include <vector>
#include "CInterpolationCircle.h"
//static LPVOID  pAction;
static int     g_LaserErrCnt;//�p�g�ɶ����p�ƾ�(�������~��)
static int     g_LaserNuCnt;//�p�g�ɶ����p�ƾ�(�����ƶq��)
class CAction
{
public:     //�ܼ�
	LPVOID  pAction;          //this���w
	BOOL    m_bIsPause;       //�Ȱ���
	BOOL    m_bIsStop;        //�����
	BOOL    m_bIsDispend;     //�����I����
	BOOL    m_bIsGetLAend;    //�p�g���ȳ̫�@�I
    LONG    m_TablelZ;        //�u�@���x����(Z�b�`�u�@����)
	LONG    m_OffSetLaserX;   //�p�g��x�b�����q
	LONG    m_OffSetLaserY;   //�p�g��y�b�����q
	LONG    m_OffSetLaserZ;   //�p�g��z�b�����q(B�I��Z���ר�p�g�k�sZ���ת��첾��(+))
	LONG    m_HeightLaserZero;//�p�g��Z�b�k�s�����ᰪ��(���y����)
	LONG    m_laserBuff;      //�p�g�μȦs��(test)
	LONG    m_OffSetScan;     //�p�g���y���v��
	LONG    m_LaserAveBuffZ;  //�p�g�Υ����Ȧs��(�����mz��)
	int     m_LaserCnt;       //�p�g�u�q�p�ƾ�(���y��)
	BOOL    m_LaserAverage;   //�p�g����(1�ϥ�/0���ϥ�)
	BOOL    m_interruptLock;  //���_��
	BOOL    m_getHeightFlag;  //�p�g�����X�СG���\����
	CPoint  m_MachineCirMid;  //����P�߶�y��
	CPoint  m_MachineOffSet;  //����P�߶갾���q
	DOUBLE  m_Wangle;		  //����ե�����W
	DOUBLE  WangBuff;		  //w�b���ਤ��buff
	CPoint  cpCirMidBuff[2];  //���P�߶�y�Ъ�buff
	LONG    m_MachineCirRad;  //����P�߶�b�|
#ifdef MOVE
    std::vector<DATA_4MOVE> W_m_ptVec;//W�s����I�x�svector
#endif

	std::vector<UINT>  LA_m_iVecSP;//�D�n�p�gvector(SP:Scan End)
	static BOOL    m_YtimeOutGlueSet;//Y�p�ɾ����_�ɥX�_������
	static BOOL    m_ZtimeOutGlueSet;//Z�p�ɾ����_�ɥX�_������
	HANDLE m_hComm;
	CString ComportNo = _T("COM6"); //RS232 COMPort
#ifdef MOVE
	std::vector<DATA_3MOVE> LA_m_ptVec;//�p�g�s����I�x�svector
	std::vector<DATA_2MOVE> LA_m_ptVec2D;//�p�g�s����I�x�svector
    DATA_4MOVE DATA_4Do[512];//�s����Ȧs
	DATA_3MOVE DATA_3Do[512];//�s����Ȧs
	DATA_2MOVE DATA_2Do[128];
	DATA_3MOVE DATA_3ZERO_B;//�p�g�k�s_�w�YB�I
	DATA_3MOVE DATA_3ZERO_LA;//�p�g�k�s_�p�gB�I
#endif 
public:     //�R�c���
	CAction();
	virtual ~CAction();
public:
	//=======================================�\��Ҧ�========================================//

	/**********���I********************************************/
	//���I�I���ʧ@--(���I�I��X,���I�I��Y,���I�I��Z,����w,�X���ɶ�(ms),�_������(ms),Z�b�^�ɰ���(�۹�)�̰��I,Z�b�Z��(�۹�),Z�b���A(0�����m/1�۹��m),���t��,�C�t��,�X�ʳt��,�[�t��,��t��)
	void DecidePointGlue(LONG lX, LONG lY, LONG lZ, DOUBLE dAng, LONG lDoTime, LONG lDelayStopTime, LONG lZBackDistance, BOOL bZDisType, LONG lZdistance, LONG lHighVelocity, LONG lLowVelocity, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);

	/**********�u�q********************************************/

	//�u�q�}�l�ʧ@--(�u�q�}�lX,�u�q�}�lY,�u�q�}�lZ,����w,���ʫe����,�X�ʳt��,�[�t��,��t��)
	void DecideLineStartMove(LONG lX, LONG lY, LONG lZ,DOUBLE dAng, LONG lStartDelayTime, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy, BOOL bIsNeedleP = 0);
	//�u�q���I�ʧ@--(�u�q���IX,�u�q���IY,�u�q���IZ,����w,��u�q���I�����d�ɶ�(�`�I�ɶ�),�X�ʳt��,�[�t��,��t��)
	void DecideLineMidMove(LONG lX, LONG lY, LONG lZ, DOUBLE dAng, LONG lMidDelayTime, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy, BOOL bIsNeedleP = 0);
	//�u�q�����ʧ@--(�u�q����X,�u�q����Y,�u�q����Z,����w,������b�����I���d�ɶ�(���d�ɶ�),�Z�������I�h���Z������(�����Z��),�����᪺����ɶ�(��������),Z�b�^�ɶZ��,Z�b�^�ɪ����A,�u�q�����]�w(���t��),��^����,z��^����,��^�t��(�C�t),��^����,�X�ʳt��,�[�t��,��t��)
	void DecideLineEndMove(LONG lX, LONG lY, LONG lZ, DOUBLE dAng, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime, LONG lZBackDistance, BOOL bZDisType, LONG lHighVelocity, LONG lDistance, LONG lHigh, LONG lLowVelocity, int iType, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
	//(�{�b)�u�q�����ʧ@�h��--(������b�����I���d�ɶ�(���d�ɶ�),�����᪺����ɶ�(��������),Z�b�^�ɬ۹�Z��,Z�b���A(0�����m/1�۹��m),�I�������]�w(���t��),��^����,z��^����,��^�t��(�C�t),�X�ʳt��,�[�t��,��t��,���A1:�b/0:���A)
	void DecideLineEndMove(LONG lCloseOffDelayTime, LONG lCloseONDelayTime,LONG lZBackDistance, BOOL bZDisType, LONG lHighVelocity, LONG lDistance,LONG lHigh, LONG lLowVelocity, LONG lWorkVelociy, LONG lAcceleration,LONG lInitVelociy, BOOL bGeneral = 0);
	
	/**********�꩷********************************************/

	//��ΰʧ@--(���I�y��X,Y,Z,W,���I�y��X2,Y2,,Z2,W2,�X�ʳt��,��t��)
	void DecideCircle(LONG lX1, LONG lY1, LONG lZ1, DOUBLE dAng1, LONG lX2, LONG lY2, LONG lZ2, DOUBLE dAng2, LONG lWorkVelociy, LONG lInitVelociy);
	//�꩷�ʧ@--(�꩷�y��X1,Y1,Z1,W1,�꩷�y��X2,Y2,Z2,W2,�X�ʳt��,��t��)
	void DecideArc(LONG lX1, LONG lY1, LONG lZ1, DOUBLE dAng1, LONG lX2, LONG lY2, LONG lZ2, DOUBLE dAng2, LONG lWorkVelociy, LONG lInitVelociy);
	//���׶�--(��߮y��X,Y,���|R,�_�l����,��������,�X�ʳt��,��t��)
	void AnglCir(LONG lCenX, LONG lCenY, LONG lR, DOUBLE dStartAngl, DOUBLE dEndAngl, LONG lWorkVelociy, LONG lInitVelociy);
	//3D��(xyz1,xyz2,xyz3,�X�ʳt��)
	void Do3AxisCirle(LONG x1, LONG y1, LONG z1, LONG x2, LONG y2, LONG z2, LONG x3, LONG y3, LONG z3, LONG speed);

	/**********�s���ʧ@****************************************/

	//�u�q�}�l�줤���I�ʧ@--(�u�q�}�lX,Y,Z,W,�u�q���IX,Y,Z,W,���ʫe����,�}���e�q�_�I���ʶZ��(�]�m�Z��),��u�q���I�����d�ɶ�(�`�I�ɶ�ms),�X�ʳt��,�[�t��,��t��)
	void DecideLineSToP(LONG lX, LONG lY, LONG lZ, DOUBLE dAng, LONG lX2, LONG lY2, LONG lZ2, DOUBLE dAng2, LONG lStartDelayTime, LONG lStartDistance, LONG lMidDelayTime, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
	//�u�q�}�l�쵲���ʧ@--(�u�q�}�lX,Y,Z,W,�����IX,Y,Z,W,���ʫe����,�}���e�q�_�I���ʶZ��(�]�m�Z��),������b�����I���d�ɶ�(���d�ɶ�),�Z�������I�h���Z������(�����Z��),�����᪺����ɶ�(��������),Z�b�^�ɶZ��,Z�b�^�ɫ��A,�I�������]�w(���t��),��^����,z��^����,��^�t��(�C�t),��^����,�X�ʳt��,�[�t��,��t��)
	void DecideLineSToE(LONG lX, LONG lY, LONG lZ, DOUBLE dAng, LONG lX2, LONG lY2, LONG lZ2, DOUBLE dAng2, LONG lStartDelayTime, LONG lStartDistance, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime, LONG lZBackDistance, BOOL bZDisType, LONG lHighVelocity, LONG lDistance, LONG lHigh, LONG lLowVelocity, int iType, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
    //�u�q�}�l��ꤤ���I�ʧ@--(��ΰʧ@--(�u�q�}�lX1,Y1,Z1,W1,��ήy��X2,Y2,Z2,W2��ήy��X3,Y3,Z3,W3,���ʫe����,�}���e�q�_�I���ʶZ��(�]�m�Z��),��u�q���I�����d�ɶ�(�`�I�ɶ�ms),�X�ʳt��,�[�t��,��t��)
    void DecideLineSToCirP(LONG lX1, LONG lY1, LONG lZ1, DOUBLE dAng1, LONG lX2, LONG lY2, LONG lZ2, DOUBLE dAng2, LONG lX3, LONG lY3, LONG lZ3, DOUBLE dAng3, LONG lStartDelayTime, LONG lStartDistance, LONG lMidDelayTime, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
    //�u�q��-�����I�쵲���ʧ@--(��ΰʧ@--(��ήy��X1,Y1,Z1,W1��ήy��X2,Y2,Z2,W2,�u�q����X3,Y3,Z3,W3,������b�����I���d�ɶ�(���d�ɶ�),�Z�������I�h���Z������(�����Z��),�����᪺����ɶ�(��������),Z�b�^�ɬ۹�Z��,Z�b���A(0�����m/1�۹��m),�I�������]�w(���t��),��^����,z��^����,��^�t��(�C�t),��^����,�X�ʳt��,�[�t��,��t��)
	void DecideCirclePToEnd(LONG lX1, LONG lY1, LONG lZ1, DOUBLE dAng1, LONG lX2, LONG lY2, LONG lZ2, DOUBLE dAng2, LONG lX3, LONG lY3, LONG lZ3, DOUBLE dAng3, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime, LONG lZBackDistance, BOOL bZDisType, LONG lHighVelocity, LONG lDistance, LONG lHigh, LONG lLowVelocity, int iType, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
	//�u�q�꩷-�����I�쵲���ʧ@--(�꩷�ʧ@--(�꩷�y��X1,Y1,Z1,W1,�u�q����X2,Y2,Z2,W2,������b�����I���d�ɶ�(���d�ɶ�),�Z�������I�h���Z������(�����Z��),�����᪺����ɶ�(��������),Z�b�^�ɬ۹�Z��,Z�b���A(0�����m/1�۹��m),�I�������]�w(���t��),��^����,z��^����,��^�t��(�C�t),��^����,�X�ʳt��,�[�t��,��t��)
	void DecideArclePToEnd(LONG lX1, LONG lY1, LONG lZ1, DOUBLE dAng1, LONG lX2, LONG lY2, LONG lZ2, DOUBLE dAng2, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime, LONG lZBackDistance, BOOL bZDisType, LONG lHighVelocity, LONG lDistance, LONG lHigh, LONG lLowVelocity, int iType, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);

	/**********���I&�_�k****************************************/
	//�����I�ʧ@--(�����I�y��X,Y,Z,w,�X�ʳt��,�[�t��,��t��)
	void DecideVirtualPoint(LONG lX, LONG lY, LONG lZ, DOUBLE dAng, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy, BOOL bIsNeedleP = 0);
	//�����I�ʧ@--(�����I�y��X,Y,Z,w,���ݮɶ�,�X�ʳt��,�[�t��,��t��)
	void DecideWaitPoint(LONG lX, LONG lY, LONG lZ, DOUBLE dAng, LONG lWaitTime, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
	//���n�I�ʧ@--(���n�I�y��X,Y,Z,w,�ƽ��ɶ�,�����ᵥ�ݮɶ�,Z�b�^�ɰ���(�۹�)�̰��I,Z�b�Z��(�۹�),���t��,�C�t��,�X�ʳt��,�[�t��,��t��)
	void DecideParkPoint(LONG lX, LONG lY, LONG lZ, DOUBLE dAng, LONG lTimeGlue, LONG lWaitTime, LONG lStayTime, LONG lZBackDistance, BOOL bZDisType, LONG lZdistance, LONG lHighVelocity, LONG lLowVelocity, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
	//���I�_�k--(���I�_�k�t��1,���I�_�k�t��2,�_�k�b(7),�����q(x,y,z,w))
	void DecideInitializationMachine(LONG lSpeed1, LONG lSpeed2, LONG lAxis,LONG lMoveX, LONG lMoveY, LONG lMoveZ, DOUBLE dMoveW);
	//W�b���I�_�k--(���I�_�k�t��1,���I�_�k�t��2)
	void DecideGoHomeW(LONG lSpeed1, LONG lSpeed2);
	//�����I���I�_�k
	void DecideVirtualHome(LONG lX, LONG lY, LONG lZ, LONG lZBackDistance, BOOL bZDisType, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
	//���I�_�k�A�^�����m(0,0,0,0)
	void BackGOZero(LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
	//�w�Y�M��ʧ@(�M��˸m��mX,Y,Z,�M��˸m�ϥΪ�io,Z�b�^�ɬ۹�Z��,Z�b���A(0�����m/1�۹��m),�X�ʳt��,�a�t��,��t��)
	void DispenClear(LONG lX, LONG lY, LONG lZ, int ClreaPort, LONG lZBackDistance, BOOL bZDisType, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);

	/**********IO�R�O******************************************/

	//��X-16�ӿ�X(��ܰ�(0~15),�}������(0~1))
	BOOL DecideOutPutSign(int iPort, BOOL bChoose);
	//��J-12�ӿ�J(��ܰ�(0~11),�}������(0~1))
	BOOL DecideInPutSign(int iPort, BOOL bChoose);

	/**********�H���Ψ��****************************************/

	//�H���Ψ��-�n��t����(x,y,z,w���̤p�u�@�d��)
	void HMNegLim(LONG lX, LONG lY, LONG lZ, DOUBLE lW);
	//�H���Ψ��-�n�饿����(x,y,z,w���̤j�u�@�d��)
	void HMPosLim(LONG lX, LONG lY, LONG lZ, DOUBLE lW);
     
	/**********�䥦�R�O*****************************************/

	//��m����(��J�@��(X,Y,Z,W)�y��)
	CString NowOffSet(LONG lX, LONG lY, LONG lZ, DOUBLE dAng);
	//�^�ǥثe��m
	CString NowLocation();
	//CCD���ʥ�--(�I�y��X,Y,Z,�X�ʳt��,�[�t��,��t��)
	void DoCCDMove(LONG lX, LONG lY, LONG lZ, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);

	/**********�p�g********************************************/

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
	//�T�b�s�򴡸�
	void LA_Line3DtoDo(int iData, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy, BOOL bDoAll = FALSE);
	//�p�g�M�����O
	void LA_Clear();
	//�p�g�����^�ǥ���z��
	void LA_AverageZ(LONG lStrX, LONG lStrY, LONG lEndX, LONG lEndY, LONG &lZ, LONG lStartVe, LONG lStartAcc, LONG lStartInitVe, LONG  lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);


	/**********�s�򴡸�********************************************/

	//2D�s��t�ɸ�ƿ�J
	void LA_Do2DVetor(LONG lX3, LONG lY3, LONG lX2 = 0, LONG lY2 = 0, LONG lX1 = 0, LONG lY1 = 0);//!!�`�N�I�y�ж���3.2.1    
	void LA_Do2dDataLine(LONG EndPX, LONG EndPY,BOOL bIsNeedleP=0);
	void LA_Do2dDataArc(LONG EndPX, LONG EndPY, LONG ArcX, LONG ArcY);
	void LA_Do2dDataCircle(LONG EndPX, LONG EndPY, LONG CirP1X, LONG CirP1Y, LONG CirP2X, LONG CirP2Y);
	//���I�p�g����(�ϥ�DATA_2MOVE���c)+�����q
	void LA_Line2D(LONG lStartVe, LONG lStartAcc, LONG lStartInitVe, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
			  
	/**********��R��***********************************************/

	//��R�ʧ@(�u�q�}�lX,Y,Z�A�u�q����X,Y,Z�AZ�b�Z��(�۹�)�AZ�b���A(0�����m/1�۹��m)�A��R�Φ�(1~7)�A�e��(mm)�A��ݼe��(mm)�A�u�q�I���]�w(1.���ʫe����A2.�]�m�Z���A3���d�ɶ��A5�����Z���A6��������)�A�X�ʳt�סA�[�t�סA��t��)
	void DecideFill(LONG lX1, LONG lY1, LONG lZ1, LONG lX2, LONG lY2, LONG lZ2, LONG lZBackDistance, BOOL bZDisType, int iType, LONG lWidth, LONG lWidth2, LONG lStartDelayTime, LONG lStartDistance, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
	//��R�ʧ@_�h��(�̫�@�I��m(EndX,EndY),�u�q�}�lX,Y,Z�A�u�q����X,Y,Z�AZ�b�Z��(�۹�)�AZ�b���A(0�����m/1�۹��m)�A��R�Φ�(1~7)�A�e��(mm)�A��ݼe��(mm))
	void Fill_EndPoint(LONG &lEndX, LONG &lEndY, LONG lX1, LONG lY1, LONG lZ1, LONG lX2, LONG lY2, LONG lZ2, int iType, LONG lWidth, LONG lWidth2);

	/**********�B�ʪ��ݨ禡******************************************/
	void WaitTime(HANDLE wakeEvent, int Time);
	//�b�d���_thread
	static DWORD WINAPI MoInterrupt(LPVOID);
	static UINT WINAPIV MoMoveThread(LPVOID param);//�����
	//��ܪ���
	CString ShowVersion();
	//�X���A�h��������ɶ�(�t�X������ϥ�)(���:ms)
	void DoGlue(LONG lTime, LONG lDelayTime, LPTHREAD_START_ROUTINE MoInterrupt = NULL);
	//����b�d�X��
	void PreventMoveError();
	//�Ȱ��^�_�᭫�s�(Ū���Ȱ��ѼơA��ѼƬ�0�ɥX���A�B�I�����n���}�C)
	void PauseDoGlue();
	//����X���X��
	void PreventGlueError();
	//�Ȱ��ɰ����(Ū���Ȱ��ѼơA��ѼƬ�1���_���A���I�������ѼƬ�0�]�_���C)
	void PauseStopGlue();
	//��^�]�w
	void GelatinizeBack(int iType, LONG lXarEnd, LONG lYarEnd, LONG lZarEnd, LONG lXarUp, LONG lYarUp, LONG lLineStop, LONG lStopZar, LONG lBackZar, LONG lLowSpeed, LONG lHighSpeed, LONG lAcceleration, LONG lInitSpeed);
	//���u�Z���ഫ���y���I
	void LineGetToPoint(LONG &lXClose, LONG &lYClose, LONG lX0, LONG lY0, LONG lX1, LONG lY1, LONG &lLineClose);
	//���u�Z���ഫ���y���I--�h��3D
	void LineGetToPoint(LONG &lXClose, LONG &lYClose, LONG &lZClose, LONG lX0, LONG lY0, LONG lX1, LONG lY1, LONG lZ0, LONG lZ1, LONG &lLineClose);
	//�p�ⴣ�e�X�_���Z��
	LONG CalPreglue(LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
	//�p�ⴣ�e�X�_���ɶ�(us)
	LONG CalPreglueTime(LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
	//�p�ⴣ�e�X�_���Z���ήɶ�(ms)
	LONG CalPreglue(LONG lStartDistance, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
	//�T�I�p����
	CString TRoundCCalculation(CString Origin, CString End, CString Between);
	//�r�������ơA�ϥΪ������@�w�n�O"�A"
	LONG CStringToLong(CString csData, int iChoose);
	//�T�I���o����(�V�q����)(���x,y�y��x1,y1,�y��x2,y2,���f��)
	DOUBLE AngleCount(DOUBLE LocatX, DOUBLE LocatY, DOUBLE LocatX1, DOUBLE LocatY1, DOUBLE LocatX2, DOUBLE LocatY2, BOOL bRev);
		//�꩷���Z�����I����
	void ArcGetToPoint(LONG &lArcX, LONG &lArcY, LONG lDistance, LONG lX, LONG lY, LONG lCenX, LONG lCenY, LONG lRadius, BOOL bDir);
	//�N3Dvetor�ȧ@���ా���ץ�
	void LA_CorrectVectorToDo(LONG  lWorkVelociy, LONG lAcceleration, LONG lInitVelociy, LONG RefX = 0, LONG RefY = 0, DOUBLE OffSetX = 0, DOUBLE OffSetY = 0, DOUBLE Andgle = 0, DOUBLE CameraToTipOffsetX = 0, DOUBLE CameraToTipOffsetY = 0, BOOL Mode = 0, LONG lSubOffsetX = 0, LONG lSubOffsetY = 0);
	//�p�g�α��७��
	void LA_CorrectLocation(LONG &PointX, LONG &PointY, LONG RefX, LONG RefY, DOUBLE OffSetX, DOUBLE OffSetY, DOUBLE Andgle, DOUBLE CameraToTipOffsetX, DOUBLE CameraToTipOffsetY, BOOL Mode, LONG lSubOffsetX, LONG lSubOffsetY);
#ifdef MOVE
    //����y����۹�y��3�b�s�򴡸ɨϥ�
	void LA_AbsToOppo3Move(std::vector<DATA_3MOVE> &str);
	//����y����۹�y��2�b�s�򴡸ɨϥ�
	void LA_AbsToOppo2Move(std::vector<DATA_2MOVE> &str);
	//��R�Ψ�b�s��t��(����--���u)
	void MCO_Do2dDataLine(LONG EndPX, LONG EndPY, std::vector<DATA_2MOVE> &str);
	//��R�Ψ�b�s��t��(����--��)
	void MCO_Do2dDataCir(LONG EndPX, LONG EndPY, LONG CenX, LONG CenY, BOOL bRev, std::vector<DATA_2MOVE> &str);
#endif
    /**********w�b�Ψ禡********************************************/
	//w�boffset��s
	void W_UpdateNeedleMotorOffset(CPoint &offset, DOUBLE degree);
    //w�b�s���s�ƭ�(�ǤJ����y��)
    void W_UpdateNeedleMotor_Robot(LONG lX, LONG lY, LONG lZ, DOUBLE dAngle0, LONG lX2, LONG lY2, LONG lZ2, DOUBLE dAngle1,DOUBLE DisAngle =1);
	//w�b�s���s�ƭ�(�ǤJ�w�Y�y��)
	void W_UpdateNeedleMotor_Needle(LONG lX, LONG lY, LONG lZ, DOUBLE dAngle0, LONG lX2, LONG lY2, LONG lZ2, DOUBLE dAngle1, DOUBLE DisAngle = 1);
	//w�b����(�w�Y�y�ЩT�w�P�@���I)
	void W_Rotation(DOUBLE dAngle, LONG lWorkVelociy, LONG lAcceleration,LONG lInitVelociy, DOUBLE DisAngle = 1);
	//�o��w�Y�y��(�ǤJ����y��)
    CPoint W_GetNeedlePoint(LONG lRobotX = 0, LONG lRobotY = 0, DOUBLE dRobotW = 0,BOOL bMode=0);
    //�o�����y��(�ǤJ�w�Y�y��)
    CPoint W_GetMachinePoint(LONG lNeedleX = 0, LONG lNeedleY = 0, DOUBLE dNeedleW = 0, BOOL bMode = 0);
#ifdef MOVE
    //����y����۹�y��4�b�s�򴡸ɨϥ�
    void W_AbsToOppo4Move(std::vector<DATA_4MOVE> &str);
#endif
    //�s��u�q�ʧ@--(�|�b�s�򴡸�)
    void W_Line4DtoDo(LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
	//W�b�|�s�򴡸ɳ�²���
	void W_Line4DtoMove(LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
	//W�b�ե��ʧ@--�B�J1.2(�B�J�B�t�סB�[�t�סB�X�ʳt�סBz�b��ɰ���)
	void W_Correction(BOOL bStep, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy,LONG lMoveZ = 10000);
    //Ū���{�b��m����(�w�]�w�Y�y��0/����y��1)
    AxeSpace MCO_ReadPosition(BOOL NedMah = 0);

    /**********JOG�Ҧ�********************************************/
    //��b����(X,Y,Z,W�۹�q����/WType:0��b,1�P�b����/������0�ϥδ�t����)
    void MCO_JogMove(LONG lX, LONG lY, LONG lZ, LONG lWorkVelociy , LONG lAcceleration , LONG lInitVelociy, DOUBLE dW = 0, BOOL WType = 0);
private:
	//����--- ��R�κA1-�x��S
	void AttachFillType1(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lZ, LONG lZBackDistance, LONG lWidth, LONG lStartDelayTime, LONG lStartDistance, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
	//����--- ��R�κA2-���۶�
	void AttachFillType2(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lZ, LONG lZBackDistance, LONG lWidth, LONG lStartDelayTime, LONG lStartDistance, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
	//����--- ��R�κA3-�x������
	void AttachFillType3(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lZ, LONG lZBackDistance, LONG lWidth, LONG lStartDelayTime, LONG lStartDistance, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
	//����--- ��R�κA4-�x����
	void AttachFillType4(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lZ, LONG lZBackDistance, LONG lWidth, LONG lWidth2, LONG lStartDelayTime, LONG lStartDistance, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
	//����--- ��R�κA5-����
	void AttachFillType5(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lZ, LONG lZBackDistance, LONG lWidth, LONG lWidth2, LONG lStartDelayTime, LONG lStartDistance, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
	//����--- ��R�κA6-�x�Τ�����
	void AttachFillType6(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lZ, LONG lZBackDistance, LONG lWidth, LONG lStartDelayTime, LONG lStartDistance, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
	//����--- ��R�κA7-�����۶�
	void AttachFillType7(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lZ, LONG lZBackDistance, LONG lWidth, LONG lStartDelayTime, LONG lStartDistance, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
	//����--- ��R�κA3-�x������end
	void AttachFillType3_End(LONG &EndX, LONG &EndY, LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lWidth, LONG lWidth2);
	//����--- ��R�κA4-�x����end
	void AttachFillType4_End(LONG &EndX, LONG &EndY, LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lWidth, LONG lWidth2);
	//����--- ��R�κA5-����end
	void AttachFillType5_End(LONG &EndX, LONG &EndY, LONG lX1, LONG lY1, LONG lCenX, LONG lCenY, LONG lWidth, LONG lWidth2);

	//******************�H�U���O�d�t��****************************************************************************************
	//===============���ϥγs�򴡸ɪ���R���A=================================================================================================================
		void AttachFillType2_1(LONG lX1, LONG lY1, LONG lCenX, LONG lCenY, LONG lZ,LONG lZBackDistance, LONG lWidth, LONG lWorkVelociy, LONG lAcceleration,LONG lInitVelociy);//����--- ��R�κA2-1
		void AttachFillType3_1(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lZ,LONG lZBackDistance, LONG lWidth, LONG lWorkVelociy, LONG lAcceleration,LONG lInitVelociy);//����--- ��R�κA3
		void AttachFillType4_1(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lZ,LONG lZBackDistance, LONG lWidth, LONG lWidth2, LONG lWorkVelociy,LONG lAcceleration, LONG lInitVelociy);//����--- ��R�κA4
		void AttachFillType5_1(LONG lX1, LONG lY1, LONG lCenX, LONG lCenY, LONG lZ,LONG lZBackDistance, LONG lWidth, LONG lWidth2, LONG lWorkVelociy,LONG lAcceleration, LONG lInitVelociy);//����--- ��R�κA5
		void AttachFillType6_1(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lZ,LONG lZBackDistance, LONG lWidth, LONG lWorkVelociy, LONG lAcceleration,LONG lInitVelociy);//����--- ��R�κA6
		void AttachFillType7_1(LONG lX1, LONG lY1, LONG lCenX, LONG lCenY, LONG lZ,LONG lZBackDistance, LONG lWidth, LONG lWorkVelociy, LONG lAcceleration,LONG lInitVelociy);//����--- ��R�κA7

};


