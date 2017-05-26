/*
*�ɮצW��:mcc.h
*���e²�z:�B�ʶb�dAPI�A�ԲӰѼƽЬd��excel
*�Iauthor �@�̦W��:R
*�Idate ��s���:2017/04/20
*@�i��2��W�b���M�� */
// mcc.h : mcc DLL ���D�n���Y��
//
#pragma once
#include <vector>
#ifndef __AFXWIN_H__
#error "�� PCH �]�t���ɮ׫e���]�t 'stdafx.h'"
#endif
#include "resource.h"       // �D�n�Ÿ�

#ifdef MCC_DLL
#define MO_DLL __declspec(dllexport)
#else
#define MO_DLL __declspec(dllimport)
#endif
/***********�b�d�s��************************/
#ifndef g_iId
#define g_iId 0
#endif
/***********���F�ѪR��************************/
//X,Y�ֱa�ɵ{40mm(20��1��2mm)
//Z�b����ɵ{25mm
//�ѪR�׹�� XY:2  Z:2.5 W:1
//���F�X�ʾ��W�ƭ�X: 20000,Y:20000, Z:10000 ,W: 20000;
#ifndef RESOLUTION
#define RESOLUTION 2.0
#endif

#ifndef RESOLUTION_Z
#define RESOLUTION_Z 2.5
#endif

#ifndef RESOLUTION_W
#define RESOLUTION_W 1
#endif

/***********�����}���M��************************/
//0���V / 1�ϦV
#ifndef HLM_DIC
#define HLM_DIC 1
#endif

/***********W�b�M��************************/
//������c�P���F������1:2.5
#ifndef W_GEAR_RATIO
#define W_GEAR_RATIO 2.5
#endif
//�B�i���F�X�ʾ��]�w�ѪR��
#ifndef W_MOTOR_DRIVER
#define W_MOTOR_DRIVER 20000.0
#endif

/***********�p�g���y�ϥ�********************/
#ifndef LA_SCANEND
#define LA_SCANEND -99999
#endif
//iAxe ���w�b�W��
#ifndef MCC_AXE_ALL
#define MCC_AXE_ALL 0xF
#endif
#ifndef MCC_AXE_X
#define MCC_AXE_X  0x1
#endif
#ifndef MCC_AXE_Y
#define MCC_AXE_Y  0x2
#endif
#ifndef MCC_AXE_Z
#define MCC_AXE_Z  0x4
#endif
#ifndef MCC_AXE_W
#define MCC_AXE_W  0x8
#endif
//====================================��ƫ��A�w�q===============================================================================================================
//=====================��b�s�򴡸ɥ�=======================================
typedef struct _DATA_2MOVE
{
    BOOL  Type;        //���A 0:���u 1:�꩷
    LONG Speed;        //�t��
    LONG Distance;     //�Z��
    CPoint EndP;       //���I
    CPoint CirCentP;   //���
    BOOL CirRev;       //���f�� 0:�� 1:�f
} DATA_2MOVE;
typedef std::vector<DATA_2MOVE> DATA_2MOVE_VECTOR;
void MO_AbsToOppo2Move(std::vector<DATA_2MOVE>
                       &);//�����m�ন�۹��m-2�b����
//=============�T�b�s�򴡸ɥ�===============================================
typedef struct _DATA_3MOVE
{
    LONG Speed;        //�t��
    LONG Distance;     //�Z��
    LONG EndPX;       //���Ix
    LONG EndPY;       //���Iy
    LONG EndPZ;       //���Iz
} DATA_3MOVE;
typedef std::vector<DATA_3MOVE> DATA_3MOVE_VECTOR;
//=============�|�b�s�򴡸ɥ�===============================================
typedef struct _DATA_4MOVE
{
    LONG Speed;        //�t��
    LONG Distance;     //�Z��
    LONG EndPX;        //���Ix
    LONG EndPY;        //���Iy
    LONG EndPZ;        //���Iz
    DOUBLE AngleW;     //����w
} DATA_4MOVE;
typedef std::vector<DATA_4MOVE> DATA_4MOVE_VECTOR;
//====================================API�R�O�C��===============================================================================================================

//====================================�򥻰ʧ@========================================================

//�}�Ҷb�d�ñN�޿��m�k�s
MO_DLL BOOL MO_Open(int interrupt);
//�����b�d
MO_DLL void MO_Close();
//���]�b�d
MO_DLL void MO_Reset();
//�����X��
MO_DLL void MO_STOP();
//��t����
MO_DLL void MO_DecSTOP();
//����s��/�줸�����X��
MO_DLL void MO_IPSTOP();
//DLL ������T
MO_DLL void MO_DllVersion(CString &dllVersion);
//DLL ��s�ɶ�
MO_DLL void MO_DllDate(CString &dllDate);

//====================================�B�ʰʧ@========================================================

//���I�_�k(�t��1�A�t��2�A���w�b�A�����qX,Y,Z)
MO_DLL void MO_MoveToHome(LONG lSpeed1, LONG lSpeed2, int iAxis, LONG lMoveX, LONG lMoveY, LONG lMoveZ);
//���I�_�kW�b�ϥ�(�t��1�A�t��2�A���w�b�A�����qX,Y,Z,W,�_�k��V0:�t/1:��)
MO_DLL void MO_MoveToHomeW(LONG lSpeed1, LONG lSpeed2, int iAxe, LONG lMoveX, LONG lMoveY, LONG lMoveZ, DOUBLE dMoveW,BOOL bDir=0);
//XY��b���u�ɶ�����(X,Y,�X�ʳt��,�[�t��,��t��)
MO_DLL void MO_Do2DLineMove(LONG lXTar, LONG lYTar, LONG lSpeed, LONG lAcc, LONG lInit);
//Z�b���u�ɶ�����(Z,�X�ʳt��,�[�t��,��t��)
MO_DLL void MO_DoZLineMove(LONG lZTar, LONG lSpeed, LONG lAcc, LONG lInit);
//XY��b�꩷�ɶ�����(X,Y,X�����I,Y�����I,�X�ʳt��,��t��,1:����(CW)/0:�f��(CCW))
MO_DLL void MO_Do2DArcMove(LONG lXTar, LONG lYTar, LONG XCenter, LONG YCenter, LONG lSpeed, LONG lInit, BOOL bRevolve);
//�T�b���u����(X,Y,Z,�X�ʳt��,�[�t��,��t��)
MO_DLL void MO_Do3DLineMove(LONG lXTar, LONG lYTar, LONG lZTar, LONG lSpeed, LONG lAcc, LONG lInit);
//�|�b���u����(X,Y,Z,W,�X�ʳt��,�[�t��,��t��)
MO_DLL void MO_Do4DLineMove(LONG lXTar, LONG lYTar, LONG lZTar, DOUBLE dWAngle, LONG lSpeed, LONG lAcc, LONG lInit);

//====================================�]�w�ƭ�========================================================

//�]�m�w�鷥���}������(���w�b,1=�}/0=��)
MO_DLL void MO_SetHardLim(int iAxe, BOOL bSet);
//�]�m�n�鷥���}������(���w�b,1=�}/0=��)
MO_DLL void MO_SetSoftLim(int iAxe, BOOL bSet);
//�]�m�n�鷥���d��(0:��/1:�t,x,y,z,w)
MO_DLL void MO_SetCompSoft(BOOL bPM, LONG lCompX, LONG lCompY, LONG lCompZ, DOUBLE dAngleW);
//�]�m��t��(���w�b,��t��)
MO_DLL void MO_SetDeceleration(int iAxe, LONG lDeceleration);
//�]�m�[��t���A((0:��ٱ��/1:���S��/2:����ٱ��/3:�����S��))
MO_DLL void MO_SetAccType(int iType);
//�]�m�[�t�׼W�[�v_S�����u(���w�b,�[�t�ײv)
MO_DLL void MO_SetJerk(int iAxe, LONG lSpeed);
//�]�m��t�׼W�[�v_�D���S�����u(���w�b,��t�ײv)
MO_DLL void MO_SetDJerk(int iAxe, LONG lSpeed);
//�]�m��t���I_�D���S�����u(X,Y,Z,W)
MO_DLL void MO_SetDecPation(ULONG ulPationX, ULONG ulPationY, ULONG ulPationZ, DOUBLE dAngleW);
//�]�m���ɴ�t����L��(1:����/0:�L��)
MO_DLL void MO_SetDecOK(BOOL bData);
//�p�ɾ�(���timer�Ҧ� 0�G�]�mTIMER/1�GTIMER�Ұ�/2�GTIMER����/3�GŪ����UTIMER��)
MO_DLL LONG MO_Timer(int iMode, int iTimerNo, LONG lData);
//�]�mTIMER�h��(�p�ɾ�Ĳ�o���_����), ���timer�Ҧ� 0�G�]�mTIMER/1�GTIMER�Ұ�/2�GTIMER����
MO_DLL LONG MO_Timer(int iMode, LONG lData);
//�]�w�p�ɾ����Ĳ�o���_(usec)(���_���,���_���0:Y/1:Z)
MO_DLL void MO_TimerSetIntter(LONG lMicroSecond, BOOL timerY0orZ1);
//�]�m�X����ơA���[us]
MO_DLL void MO_GummingSet(LONG lMicroSecond = 0);

//====================================Ū���ƭ�========================================================

//Ū���޿��m(x=0,y=1,z=2,w=3)
MO_DLL LONG MO_ReadLogicPosition(int iAxis);
//Ū���޿訤��W
MO_DLL DOUBLE MO_ReadLogicPositionW();
//Ū����ڦ�m(encoder�M��)
MO_DLL LONG MO_ReadRealPosition(int iAxis);
//Ū����ڦ�m����w(encoder�M��)
DOUBLE MO_ReadRealPositionW();
//Ū���b�X�ʳt��(x=0,y=1,z=2,w=3)
MO_DLL LONG MO_ReadSpeed(int iAxis);
//Ū���b�X�ʳt�ר���W
MO_DLL DOUBLE MO_ReadSpeedW();
//Ū���b�[��t��(x=0,y=1,z=2,w=3)
MO_DLL LONG MO_ReadAccDec(int iAxis);
//Ū���b�[��t�ר���
MO_DLL DOUBLE MO_ReadAccDecW();
//Ū���O�_���b�X�ʤ�(���w�b0~15)
MO_DLL BOOL MO_ReadIsDriving(int iAxe);
//Ū���w�鷥���}���]�w���A(Ū���Ȭ�1=�}/0=��)
MO_DLL void MO_ReadHardLimSet(BOOL &bHardX, BOOL &bHardY, BOOL &bHardZ, BOOL &bHardW);
//Ū���n�鷥���}���]�w���A(Ū���Ȭ�1=�}/0=��)
MO_DLL void MO_ReadSoftLimSet(BOOL &bSoftX, BOOL &bSoftY, BOOL &bSoftZ, BOOL &bSoftW);
//Ū���O�_�b�w�鷥���}���W�H�ιB�檬�A(0:�b�t�����W/1:�b�������W/-1:���b�����W,0:�����G��/1:�������`)
MO_DLL void MO_ReadRunHardLim(int &iPMLimX, BOOL &bIsOkLimX, int &iPMLimY, BOOL &bIsOkLimY, int &iPMLimZ, BOOL &bIsOkLimZ, int &iPMLimW, BOOL &bIsOkLimW);
//Ū���n�鷥���}�����檬�A(1:���~/0:���`)
MO_DLL void MO_ReadSoftLimError(BOOL bPM, BOOL &bSoftX, BOOL &bSoftY, BOOL &bSoftZ, BOOL &bSoftW);
//Ū�����A���F��J�T�����`(���A���F�M��)
MO_DLL void MO_ReadMotoAlarm(BOOL &bMotoSignalX, BOOL &bMotoSignalY, BOOL &bMotoSignalZ, BOOL &bMotoSignalW);
//Ū���U�b���I�_�k���~���A(1:���~/0:���`)
MO_DLL void MO_ReadHomeError(BOOL &bHomeErrorX, BOOL &bHomeErrorY, BOOL &bHomeErrorZ, BOOL &bHomeErrorW);
//Ū���U�b���ɿ��~���A(1:���~/0:���`)
MO_DLL void MO_ReadIP(BOOL &bIPX, BOOL &bIPY, BOOL &bIPZ, BOOL &bIPW);
//Ū���X�ʤ���氱��A(1:���~/0:���`)
MO_DLL BOOL MO_ReadEMG();
//Ū���q�μȦs��PIO��J(��J0~11)
MO_DLL BOOL MO_ReadPIOInput(int iBit);
//�]�m�q�μȦs��PIO��X(��X0~15,1=�}/0=��)
MO_DLL void MO_SetPIOOutput(int iBit, BOOL bData);
//Ū�����xStart���s���A(1:Ĳ�o/0:��Ĳ�o)
MO_DLL BOOL MO_ReadStartBtn();
//Ū�����x�������s���A(1:Ĳ�o/0:��Ĳ�o)
MO_DLL BOOL MO_ReadSwitchBtn();
//Ū�����x�ƽ����s���A(1:Ĳ�o/0:��Ĳ�o)
MO_DLL BOOL MO_ReadGlueOutBtn();
//Ū���X������A(1:�X��/0:�L)
MO_DLL BOOL MO_ReadGumming();
//Ū���Ȧs��RR0,RR2,RR3
MO_DLL LONG MO_ReadReg(int iRegSelect, int iAxis);
//Ū���Ȧs���]�w��
MO_DLL LONG MO_ReadSetData(int iSelect, int iAxis);
//Ū�����_�Ȧs��RR1
MO_DLL void MO_ReadEvent(long *RR1X, long *RR1Y, long *RR1Z, long *RR1U);
//Ū���s�򴡸ɹw�s�w�ļȦs�����A
MO_DLL int MO_ReadStackCntCIP();

//====================================�s�򴡸�========================================================

//3�b�s��ɶ�(8��)
MO_DLL void MO_DO3Continuous(LONG lInitSpeed, LONG lAcceleration, LONG lSpeed,DATA_3MOVE *DATA_3M, int iNum);
//4�b�s��ɶ�_���
MO_DLL void MO_DO4Curve(DATA_4MOVE *DATA_4M, int dataCnt, LONG lSpeed, LONG lAcceleration);
//3�b�s��ɶ�_���
MO_DLL void MO_DO3Curve(DATA_3MOVE *DATA_3M, int dataCnt, LONG lSpeed, LONG lAcceleration);
//2�b�s��ɶ�_���
MO_DLL void MO_DO2Curve(DATA_2MOVE *DATA_2M, int dataCnt, LONG lSpeed, LONG lAcceleration);
//4�b�s��ɶ�_�p�⧹���ɶ��P�u��t��
MO_DLL void MO_CalFinishTimeOf4ContIP(LONG &finishTime, LONG &TrueSpeed, LONG lSpeed, LONG lAcceleration, DATA_4MOVE *DATA_4M, int dataCnt);
//�s�򴡸ɪ���t�I�p��
LONG CalCipDecP(LONG InitSpd, LONG Dec, LONG Spd);
//�����m�ন�۹��m-3�b����
void MO_AbsToOppo3Move(std::vector<DATA_3MOVE> &);



//====================================��L�]�w========================================================

//���~���A�M��
MO_DLL void MO_AlarmCClean();
//�X������
MO_DLL void MO_FinishGumming();
//����X���A�M��Timer
MO_DLL void MO_StopGumming();
//�Ʀr�ഫ��CString 16-bit binary
MO_DLL CString MO_StreamToUnit(int iValue);
//���_������(0:���t�ߪi��X�}�l/ 1:��X����/ 2:�X�ʶ}�l/ 3:�X�ʵ���/ 4:�����X��)
MO_DLL void MO_InterruptCase(BOOL bIsClose, int iData, LPTHREAD_START_ROUTINE Thread = NULL, LPVOID lpParameter = NULL);
//���~���A�ˬd
MO_DLL void MO_ErrorStateCheck();

//====================================���F�ѪR��========================================================
//���F�ѪR��X,Y(�ǤJ)
void ResolutionChange(LONG &lValue);
//���F�ѪR��X,Y(�ǥX)
void ResolutionInsChange(LONG &lValue);
//���F�ѪR��Z(�ǤJ)
void ResolutionChangeZ(LONG &lValue);
//���F�ѪR��Z(�ǥX)
void ResolutionInsChangeZ(LONG &lValue);
//���F�ѪR��W(�ǤJ)
void ResolutionChangeW(LONG &lValue);
//���F�ѪR��W(�ǥX)
void ResolutionInsChangeW(LONG &lValue);
//====================================W�b�ѪR���ഫ========================================================
//W�b�M�Ψ����ഫ�ߪi
MO_DLL LONG MO_AngleToPulse(DOUBLE dAngle);
//W�b�M�ίߪi�ഫ����
MO_DLL DOUBLE MO_PulseToAngle(LONG lPulse);


//MO_DLL void MO_Do2ContData(DATA_2MOVE DATA_2MOVE);//2�b���c��ư϶�
//MO_DLL void MO_DO2ContStart(LONG lInitSpeed, LONG lAcceleration, LONG lSpeed,
//                            DATA_2MOVE DATA_2MOVE);//2�b�s��ɶ��}�l
//MO_DLL void MO_Do3ContData(DATA_3MOVE DATA_3MOVE);//3�b���c��ư϶�
//MO_DLL void MO_DO3ContStart(LONG lInitSpeed, LONG lAcceleration, LONG lSpeed,
//                            DATA_3MOVE DATA_3MOVE);//3�b�s��ɶ��}�l
//MO_DLL void MO_DO3DCurveStart(DATA_3CIP_MC8500P *data3CIP, int dataSize);
// CmccApp
// �o�����O����@�аѾ\ mcc.cpp
//

class CmccApp : public CWinApp
{
    public:
        CmccApp();

        // �мg
    public:
        virtual BOOL InitInstance();

        DECLARE_MESSAGE_MAP()

};
