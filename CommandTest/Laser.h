// Laser.h : Laser DLL ���D�n���Y��
//

#pragma once

#ifndef __AFXWIN_H__
#error "�� PCH �]�t���ɮ׫e���]�t 'stdafx.h'"
#endif

#include "resource.h"       // �D�n�Ÿ�
#include "mcc.h"

#define KEYENCE_LKG85

#ifndef LAS_DLL
#define LAS_DLL __declspec(dllimport)
#endif

#ifndef LAS_LONGMIN
#define LAS_LONGMIN -2147483647-1
#endif


// CLaserApp
// �o�����O����@�аѾ\ Laser.cpp
//

class CLaserApp : public CWinApp
{
    public:
        CLaserApp();

        // �мg
    public:
        virtual BOOL InitInstance();

        DECLARE_MESSAGE_MAP()
};

#ifdef KEYENCE_LKG85

LAS_DLL void LAS_SetInit();
LAS_DLL BOOL LAS_GetLaserData(LONG &lZ);
LAS_DLL BOOL LAS_SetZero();
LAS_DLL void LAS_GoBPoint(DATA_3MOVE &DATA_3ZERO_B);
LAS_DLL DATA_3MOVE LAS_GoLAtoBPoint(DATA_3MOVE DATA_3ZERO_B);

#endif


#pragma region �ǦC�ǿ�

HANDLE hComm;
LAS_DLL bool LAS_RS_Open(int iComPort, int iBaudRate);//�}�ҧǦC�q�T��w
LAS_DLL bool LAS_RS_Close();//�����ǦC�q�T��w
LAS_DLL CString LAS_RS_ReadData(int iCommand);//�ƭ�Ū�X
LAS_DLL CString LAS_RS_SetSamplingCycle(int
                                        iCommand);//�ѼƳ]�w-���˶g��
LAS_DLL CString LAS_RS_SetAverageTimes(int iCommand);//�ѼƳ]�w-��������
LAS_DLL CString LAS_RS_SetZero(int iCommand);//�ѼƳ]�w-�չs
LAS_DLL CString LAS_RS_SetInitialize(int iCommand);//�ѼƳ]�w-��l��
LAS_DLL CString LAS_RS_SetSave(int iCommand);//�ѼƳ]�w-�O�s
LAS_DLL CString LAS_RS_SetSpan(CString csData);//�ѼƳ]�w - �Y��
LAS_DLL CString LAS_RS_SetOff(CString csData);//�ѼƳ]�w-�����q
CString SubOutDate(CString csData, char *chBuffer1);
CString SuboutCommand(CString csBuffer);

#pragma endregion