// Laser.h : Laser DLL ���D�n���Y��
//

#pragma once

#ifndef __AFXWIN_H__
#error "�� PCH �]�t���ɮ׫e���]�t 'stdafx.h'"
#endif

#include "resource.h"       // �D�n�Ÿ�
#include "mcc.h"

#define KEYENCE_LKG85

//#define SERIALPORT
#ifdef SERIALPORT
#define OMRON_ZX2_LD100
#endif

#ifndef LAS_DLL
#define LAS_DLL __declspec(dllimport)
#endif

#ifndef LAS_LONGMIN
#define LAS_LONGMIN -2147483647-1
#endif

#ifndef LAS_MEASURE_RANGE
#define LAS_MEASURE_RANGE 30000
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

LAS_DLL void LAS_DllVersion(CString &dllVersion);
LAS_DLL void LAS_DllDate(CString &dllDate);
LAS_DLL void LAS_Device(CString &device);

#ifdef KEYENCE_LKG85

LAS_DLL void LAS_SetInit();
LAS_DLL BOOL LAS_GetLaserData(LONG &lZ);
LAS_DLL BOOL LAS_SetZero();
LAS_DLL void LAS_GoBPoint(DATA_3MOVE &DATA_3ZERO_B);
LAS_DLL DATA_3MOVE LAS_GoLAtoBPoint(DATA_3MOVE DATA_3ZERO_B);

#endif


#pragma region �ǦC�ǿ�
#ifdef SERIALPORT

LAS_DLL void LAS_SetInit();
LAS_DLL void LAS_SetInit(HANDLE *phComm);
LAS_DLL BOOL LAS_GetLaserData(LONG &lZ);
LAS_DLL BOOL LAS_SetZero();
LAS_DLL void LAS_GoBPoint(DATA_3MOVE &DATA_3ZERO_B);
LAS_DLL DATA_3MOVE LAS_GoLAtoBPoint(DATA_3MOVE DATA_3ZERO_B);

HANDLE hComm;
bool RS_Open(int iComPort, int iBaudRate);//�}�ҧǦC�q�T��w
bool RS_Open(HANDLE *phComm);
bool RS_Close();//�����ǦC�q�T��w
CString RS_ReadData(int iCommand);//�ƭ�Ū�X
CString RS_ReadData_Omron(int iCommand);//�ƭ�Ū�X
CString RS_SetSamplingCycle(int iCommand);//�ѼƳ]�w-���˶g��
CString RS_SetAverageTimes(int iCommand);//�ѼƳ]�w-��������
CString RS_SetZero(int iCommand);//�ѼƳ]�w-�չs
CString RS_SetInitialize(int iCommand);//�ѼƳ]�w-��l��
CString RS_SetSave(int iCommand);//�ѼƳ]�w-�O�s
CString RS_SetSpan(CString csData);//�ѼƳ]�w - �Y��
CString RS_SetOff(CString csData);//�ѼƳ]�w-�����q
CString SubOutDate(CString csData, char *chBuffer1);
CString SuboutCommand(CString csBuffer);
#endif
#pragma endregion