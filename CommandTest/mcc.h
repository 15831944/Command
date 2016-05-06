// mcc.h : mcc DLL 的主要標頭檔
//

#pragma once

#ifndef __AFXWIN_H__
#error "對 PCH 包含此檔案前先包含 'stdafx.h'"
#endif
#include "resource.h"       // 主要符號

#ifndef MO_DLL
#define MO_DLL __declspec(dllimport)
#endif

#ifndef g_iId
#define g_iId 0
#endif


MO_DLL BOOL MO_Open(int interrupt);//開啟軸卡並將邏輯位置歸零
MO_DLL void MO_Close();//關閉軸卡
MO_DLL void MO_STOP();//停止驅動
MO_DLL void MO_DecSTOP();//減速停止
MO_DLL void MO_MoveToHome(LONG lSpeed1, LONG lSpeed2, int iAxis,
                          LONG lMove);//原點復歸
MO_DLL void MO_Do2DLineMove(LONG lXTar, LONG lYTar, LONG lSpeed,
                            LONG lAcceleration, LONG lInitSpeed);//XY兩軸直線補間移動
MO_DLL void MO_DoZLineMove(LONG lZTar, LONG lSpeed, LONG lAcceleration,
                           LONG lInitSpeed);//Z軸直線補間移動
MO_DLL void MO_Do2DArcMove(LONG lXTar, LONG lYTar, LONG XCenter, LONG YCenter,
                           LONG lInitSpeed, LONG lSpeed, BOOL bRevolve);//XY兩軸圓弧補間移動
MO_DLL void MO_Do3DLineMove(LONG lXTar, LONG lYTar, LONG lZTar, LONG lSpeed,
                            LONG lAcceleration, LONG lInitSpeed);//三軸直線插補
MO_DLL void MO_SetHardLim(int iAxis, BOOL bSet);//設置硬體極限開關有效
MO_DLL void MO_SetSoftLim(int iAxis, BOOL bSet);//設置軟體極限開關有效
MO_DLL void MO_SetCompSoft(BOOL bPM, LONG lCompX, LONG lCompY,
                           LONG lCompZ); //設置軟體極限範圍
MO_DLL void MO_SetDeceleration(int iAxis, LONG lDeceleration);//設置減速度
MO_DLL void MO_SetAccType(int iType);//設置加減速型態(梯型或S型)
MO_DLL LONG MO_ReadLogicPosition(int iAxis);//讀取邏輯位置
MO_DLL LONG MO_ReadRealPosition(int iAxis);//讀取實際位置(encoder專用)
MO_DLL LONG MO_ReadSpeed(int iAxis);//讀取軸驅動速度
MO_DLL LONG MO_ReadAccDec(int iAxis);//讀取軸加減速度
MO_DLL BOOL MO_ReadIsDriving(int iAxis);//確認是否正在驅動中
MO_DLL void MO_ReadHardLim(BOOL &bHardX, BOOL &bHardY,
                           BOOL &bHardZ);//讀取硬體極限開關設定狀態
MO_DLL void MO_ReadSoftLim(BOOL &bSoftX, BOOL &bSoftY,
                           BOOL &bSoftZ);//讀取軟體極限開關設定狀態
MO_DLL void MO_ReadRunHardLim(int &iPMLimX, BOOL &bIsOkLimX, int &iPMLimY,
                              BOOL &bIsOkLimY, int &iPMLimZ,
                              BOOL &bIsOkLimZ);//讀取是否在硬體極限開關上以及運行狀態
MO_DLL void MO_ReadMotoAlarm(BOOL &bMotoSignalX,BOOL &bMotoSignalY,
                             BOOL &bMotoSignalZ);//伺服馬達輸入訊號異常(伺服馬達專用)
MO_DLL BOOL MO_ReadEMG();//讀取驅動中緊急停止狀態
MO_DLL void MO_ReadSoftLimError(BOOL bPM, BOOL &bSoftX, BOOL &bSoftY,
                                BOOL &bSoftZ);//讀取軟體極限開關執行狀態
MO_DLL void MO_ReadHomeError(BOOL &bHomeErrorX,BOOL &bHomeErrorY,
                             BOOL &bHomeErrorZ);//讀取各軸原點復歸錯誤狀態
MO_DLL void MO_ReadIP(BOOL &bIPX,BOOL &bIPY,
                      BOOL &bIPZ);//讀取各軸插補錯誤狀態
MO_DLL void MO_AlarmCClean();//錯誤狀態清除
MO_DLL BOOL MO_ReadPIOInput(int iBit);//讀取通用暫存器PIO輸入
MO_DLL void MO_SetPIOOutput(int iBit,
                            BOOL bData); //設置通用暫存器PIO輸出
MO_DLL BOOL MO_ReadStartBtn();//讀取機台Start按鈕狀態
MO_DLL void MO_SetJerk(int iAxis,
                       LONG lSpeed);//設置加速度增加率(用於S型曲線)
MO_DLL void MO_SetDJerk(int iAxis,
                        LONG lSpeed); //設置減速度增加率(用於非對稱S型曲線)
MO_DLL void MO_SetDecPation(ULONG ulPationX, ULONG ulPationY,
                            ULONG ulPationZ);//設置減速度點(用於非對稱S型曲線)
MO_DLL void MO_SetDecOK(BOOL bData);//設置插補減速停止有無效
MO_DLL LONG MO_Timer(int iMode, int iTimerNo, LONG lData);//計時器
MO_DLL BOOL MO_ReadGumming();//讀取出膠控制狀態
MO_DLL void MO_GummingSet(LONG lMicroSecond,
                          LPTHREAD_START_ROUTINE GummingTimeOutThread);//設置出膠秒數，單位[us]
MO_DLL void MO_GummingSet(int iData, LONG lMicroSecond, LPTHREAD_START_ROUTINE GummingTimeOutThread);//設定出膠時間-多載(選擇多個點膠埠1~3,出膠時間(usec),中斷程序名稱)
MO_DLL void MO_FinishGumming();//出膠完成
MO_DLL void MO_StopGumming();//停止出膠，清除Timer
MO_DLL void MO_Do3DLineMove(LONG lXTar, LONG lYTar, LONG lZTar, LONG lSpeed,
                            LONG lAcceleration, LONG lInitSpeed);
MO_DLL LONG MO_ReadReg(int iRegSelect, int iAxis);//讀取暫存器RR0~RR3
MO_DLL CString MO_StreamToUnit(int iValue);//數字轉換成CString 16-bit binary
MO_DLL void MO_Do2ArcReduce(LONG lXTar, LONG lYTar, LONG XCenter, LONG YCenter,
    LONG lInitSpeed, LONG lSpeed, BOOL bRevolve, LONG lAcceleration, LONG  lPation, LPTHREAD_START_ROUTINE StopGumming);//XY兩軸圓弧補間移動(減速點)
MO_DLL void MO_Do3DLineMoveInterr(LONG lXTar, LONG lYTar, LONG lZTar, LONG lSpeed,
    LONG lAcceleration, LONG lInitSpeed, LPTHREAD_START_ROUTINE LPInterrupt); //XYZ三軸直線補間移動 - 多載(中斷)
MO_DLL void MO_TimerSetIntter(LONG lMicroSecond,LPTHREAD_START_ROUTINE TimeOutThread);//設定計時器到後觸發中斷(usec)
// CmccApp
// 這個類別的實作請參閱 mcc.cpp
//

class CmccApp : public CWinApp
{
    public:
        CmccApp();

        // 覆寫
    public:
        virtual BOOL InitInstance();

        DECLARE_MESSAGE_MAP()

};
