// Laser.h : Laser DLL 璶夹繷郎
//

#pragma once

#ifndef __AFXWIN_H__
#error "癸 PCH 郎玡 'stdafx.h'"
#endif

#include "resource.h"       // 璶才腹
#include "mcc.h"

//#define KEYENCE_LKG85
#define SERIALPORT


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
// 硂摸龟叫把綷 Laser.cpp
//

class CLaserApp : public CWinApp
{
    public:
        CLaserApp();

        // 滦糶
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


#pragma region 肚块
#ifdef SERIALPORT

LAS_DLL void LAS_SetInit();
LAS_DLL void LAS_SetInit(HANDLE *phComm);
LAS_DLL BOOL LAS_GetLaserData(LONG &lZ);
LAS_DLL BOOL LAS_SetZero();
LAS_DLL void LAS_GoBPoint(DATA_3MOVE &DATA_3ZERO_B);
LAS_DLL DATA_3MOVE LAS_GoLAtoBPoint(DATA_3MOVE DATA_3ZERO_B);

HANDLE hComm;
bool RS_Open(int iComPort, int iBaudRate);//秨币硄癟﹚
bool RS_Open(HANDLE *phComm);
bool RS_Close();//闽超硄癟﹚
CString RS_ReadData(int iCommand);//计弄
CString RS_ReadData_Omron(int iCommand);//计弄
CString RS_SetSamplingCycle(int iCommand);//把计砞﹚-妓秅戳
CString RS_SetAverageTimes(int iCommand);//把计砞﹚-キАΩ计
CString RS_SetZero(int iCommand);//把计砞﹚-秸箂
CString RS_SetInitialize(int iCommand);//把计砞﹚-﹍て
CString RS_SetSave(int iCommand);//把计砞﹚-玂
CString RS_SetSpan(CString csData);//把计砞﹚ - 玒计
CString RS_SetOff(CString csData);//把计砞﹚-熬簿秖
CString SubOutDate(CString csData, char *chBuffer1);
CString SuboutCommand(CString csBuffer);
#endif
#pragma endregion