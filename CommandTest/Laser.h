// Laser.h : Laser DLL 璶夹繷郎
//

#pragma once

#ifndef __AFXWIN_H__
#error "癸 PCH 郎玡 'stdafx.h'"
#endif

#include "resource.h"       // 璶才腹
#include "mcc.h"

#define KEYENCE_LKG85

#ifndef LAS_DLL
#define LAS_DLL __declspec(dllimport)
#endif

#ifndef LAS_LONGMIN
#define LAS_LONGMIN -2147483647-1
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

#ifdef KEYENCE_LKG85

LAS_DLL void LAS_SetInit();
LAS_DLL BOOL LAS_GetLaserData(LONG &lZ);
LAS_DLL BOOL LAS_SetZero();
LAS_DLL void LAS_GoBPoint(DATA_3MOVE &DATA_3ZERO_B);
LAS_DLL DATA_3MOVE LAS_GoLAtoBPoint(DATA_3MOVE DATA_3ZERO_B);

#endif


#pragma region 肚块

HANDLE hComm;
LAS_DLL bool LAS_RS_Open(int iComPort, int iBaudRate);//秨币硄癟﹚
LAS_DLL bool LAS_RS_Close();//闽超硄癟﹚
LAS_DLL CString LAS_RS_ReadData(int iCommand);//计弄
LAS_DLL CString LAS_RS_SetSamplingCycle(int
                                        iCommand);//把计砞﹚-妓秅戳
LAS_DLL CString LAS_RS_SetAverageTimes(int iCommand);//把计砞﹚-キАΩ计
LAS_DLL CString LAS_RS_SetZero(int iCommand);//把计砞﹚-秸箂
LAS_DLL CString LAS_RS_SetInitialize(int iCommand);//把计砞﹚-﹍て
LAS_DLL CString LAS_RS_SetSave(int iCommand);//把计砞﹚-玂
LAS_DLL CString LAS_RS_SetSpan(CString csData);//把计砞﹚ - 玒计
LAS_DLL CString LAS_RS_SetOff(CString csData);//把计砞﹚-熬簿秖
CString SubOutDate(CString csData, char *chBuffer1);
CString SuboutCommand(CString csBuffer);

#pragma endregion