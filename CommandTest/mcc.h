// mcc.h : mcc DLL ���D�n���Y��
//

#pragma once

#ifndef __AFXWIN_H__
#error "�� PCH �]�t���ɮ׫e���]�t 'stdafx.h'"
#endif
#include "resource.h"       // �D�n�Ÿ�

#ifndef MO_DLL
#define MO_DLL __declspec(dllimport)
#endif

#ifndef g_iId
#define g_iId 0
#endif


MO_DLL BOOL MO_Open(int interrupt);//�}�Ҷb�d�ñN�޿��m�k�s
MO_DLL void MO_Close();//�����b�d
MO_DLL void MO_STOP();//�����X��
MO_DLL void MO_DecSTOP();//��t����
MO_DLL void MO_MoveToHome(LONG lSpeed1, LONG lSpeed2, int iAxis);//���I�_�k
MO_DLL void MO_MoveToHome(LONG lSpeed1, LONG lSpeed2, int iAxis, LONG lMoveX, LONG lMoveY, LONG lMoveZ);//���I�_�k-�h��(�t��1�A�t��2�A���w�b7�A�����qX,Y,Z)
MO_DLL void MO_Do2DLineMove(LONG lXTar, LONG lYTar, LONG lSpeed,
                            LONG lAcceleration, LONG lInitSpeed);//XY��b���u�ɶ�����
MO_DLL void MO_DoZLineMove(LONG lZTar, LONG lSpeed, LONG lAcceleration,
                           LONG lInitSpeed);//Z�b���u�ɶ�����
MO_DLL void MO_Do2DArcMove(LONG lXTar, LONG lYTar, LONG XCenter, LONG YCenter,
                           LONG lInitSpeed, LONG lSpeed, BOOL bRevolve);//XY��b�꩷�ɶ�����
MO_DLL void MO_Do3DLineMove(LONG lXTar, LONG lYTar, LONG lZTar, LONG lSpeed,
                            LONG lAcceleration, LONG lInitSpeed);//�T�b���u����
MO_DLL void MO_SetHardLim(int iAxis, BOOL bSet);//�]�m�w�鷥���}������
MO_DLL void MO_SetSoftLim(int iAxis, BOOL bSet);//�]�m�n�鷥���}������
MO_DLL void MO_SetCompSoft(BOOL bPM, LONG lCompX, LONG lCompY,
                           LONG lCompZ); //�]�m�n�鷥���d��
MO_DLL void MO_SetDeceleration(int iAxis, LONG lDeceleration);//�]�m��t��
MO_DLL void MO_SetAccType(int iType);//�]�m�[��t���A(�諬��S��)
MO_DLL LONG MO_ReadLogicPosition(int iAxis);//Ū���޿��m
MO_DLL LONG MO_ReadRealPosition(int iAxis);//Ū����ڦ�m(encoder�M��)
MO_DLL LONG MO_ReadSpeed(int iAxis);//Ū���b�X�ʳt��
MO_DLL LONG MO_ReadAccDec(int iAxis);//Ū���b�[��t��
MO_DLL BOOL MO_ReadIsDriving(int iAxis);//�T�{�O�_���b�X�ʤ�
MO_DLL void MO_ReadHardLim(BOOL &bHardX, BOOL &bHardY,
                           BOOL &bHardZ);//Ū���w�鷥���}���]�w���A
MO_DLL void MO_ReadSoftLim(BOOL &bSoftX, BOOL &bSoftY,
                           BOOL &bSoftZ);//Ū���n�鷥���}���]�w���A
MO_DLL void MO_ReadRunHardLim(int &iPMLimX, BOOL &bIsOkLimX, int &iPMLimY,
                              BOOL &bIsOkLimY, int &iPMLimZ,
                              BOOL &bIsOkLimZ);//Ū���O�_�b�w�鷥���}���W�H�ιB�檬�A
MO_DLL void MO_ReadMotoAlarm(BOOL &bMotoSignalX,BOOL &bMotoSignalY,
                             BOOL &bMotoSignalZ);//���A���F��J�T�����`(���A���F�M��)
MO_DLL BOOL MO_ReadEMG();//Ū���X�ʤ���氱��A
MO_DLL void MO_ReadSoftLimError(BOOL bPM, BOOL &bSoftX, BOOL &bSoftY,
                                BOOL &bSoftZ);//Ū���n�鷥���}�����檬�A
MO_DLL void MO_ReadHomeError(BOOL &bHomeErrorX,BOOL &bHomeErrorY,
                             BOOL &bHomeErrorZ);//Ū���U�b���I�_�k���~���A
MO_DLL void MO_ReadIP(BOOL &bIPX,BOOL &bIPY,
                      BOOL &bIPZ);//Ū���U�b���ɿ��~���A
MO_DLL void MO_AlarmCClean();//���~���A�M��
MO_DLL BOOL MO_ReadPIOInput(int iBit);//Ū���q�μȦs��PIO��J
MO_DLL void MO_SetPIOOutput(int iBit,
                            BOOL bData); //�]�m�q�μȦs��PIO��X
MO_DLL BOOL MO_ReadStartBtn();//Ū�����xStart���s���A
MO_DLL void MO_SetJerk(int iAxis,
                       LONG lSpeed);//�]�m�[�t�׼W�[�v(�Ω�S�����u)
MO_DLL void MO_SetDJerk(int iAxis,
                        LONG lSpeed); //�]�m��t�׼W�[�v(�Ω�D���S�����u)
MO_DLL void MO_SetDecPation(ULONG ulPationX, ULONG ulPationY,
                            ULONG ulPationZ);//�]�m��t���I(�Ω�D���S�����u)
MO_DLL void MO_SetDecOK(BOOL bData);//�]�m���ɴ�t����L��
MO_DLL LONG MO_Timer(int iMode, int iTimerNo, LONG lData);//�p�ɾ�
MO_DLL BOOL MO_ReadGumming();//Ū���X������A
MO_DLL void MO_GummingSet(LONG lMicroSecond,
                          LPTHREAD_START_ROUTINE GummingTimeOutThread);//�]�m�X����ơA���[us]
MO_DLL void MO_GummingSet(int iData, LONG lMicroSecond, LPTHREAD_START_ROUTINE GummingTimeOutThread);//�]�w�X���ɶ�-�h��(��ܦh���I����1~3,�X���ɶ�(usec),���_�{�ǦW��)
MO_DLL void MO_FinishGumming();//�X������
MO_DLL void MO_StopGumming();//����X���A�M��Timer
MO_DLL void MO_Do3DLineMove(LONG lXTar, LONG lYTar, LONG lZTar, LONG lSpeed,
                            LONG lAcceleration, LONG lInitSpeed);
MO_DLL LONG MO_ReadReg(int iRegSelect, int iAxis);//Ū���Ȧs��RR0~RR3
MO_DLL CString MO_StreamToUnit(int iValue);//�Ʀr�ഫ��CString 16-bit binary
MO_DLL void MO_Do2ArcReduce(LONG lXTar, LONG lYTar, LONG XCenter, LONG YCenter,
    LONG lInitSpeed, LONG lSpeed, BOOL bRevolve, LONG lAcceleration, LONG  lPation, LPTHREAD_START_ROUTINE StopGumming);//XY��b�꩷�ɶ�����(��t�I)
MO_DLL void MO_Do3DLineMoveInterr(LONG lXTar, LONG lYTar, LONG lZTar, LONG lSpeed,
    LONG lAcceleration, LONG lInitSpeed, LPTHREAD_START_ROUTINE LPInterrupt); //XYZ�T�b���u�ɶ����� - �h��(���_)
MO_DLL void MO_TimerSetIntter(LONG lMicroSecond,LPTHREAD_START_ROUTINE TimeOutThread);//�]�w�p�ɾ����Ĳ�o���_(usec)

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
