// mcc.h : mcc DLL 的主要標頭檔
//

#pragma once
#include <vector>
#ifndef __AFXWIN_H__
#error "對 PCH 包含此檔案前先包含 'stdafx.h'"
#endif
#include "resource.h"       // 主要符號

#ifndef MO_DLL
#define MO_DLL __declspec(dllimport)
#endif

/*DLL 更新時間*/
#ifndef MCC_DLL_DATE
#define MCC_DLL_DATE 20160715
#endif

/*DLL 版本*/
#ifndef MCC_DLL_VERSION
#define MCC_DLL_VERSION "虹橋1號機版本"
#endif

#ifndef g_iId
#define g_iId 0
#endif

#ifndef RESOLUTION
#define RESOLUTION 1.92
#endif

#ifndef RESOLUTION_Z 
#define RESOLUTION_Z 1.25
#endif

#ifndef LA_SCANEND
#define LA_SCANEND -99999
#endif
//=====================兩軸連續插補用=======================================
typedef struct _DATA_2MOVE
{
    BOOL  Type;        //型態 0:直線 1:圓弧
    LONG Speed;        //速度
    LONG Distance;     //距離
    CPoint EndP;       //終點
    CPoint CirCentP;   //圓心
    BOOL CirRev;       //正逆轉 0:順 1:逆
} DATA_2MOVE;
typedef std::vector<DATA_2MOVE> DATA_2MOVE_VECTOR;
void MO_AbsToOppo2Move(std::vector<DATA_2MOVE>
                       &);//絕對位置轉成相對位置-2軸插補
//=============三軸連續插補用===============================================
typedef struct _DATA_3MOVE
{
    LONG Speed;        //速度
    LONG Distance;     //距離
    LONG EndPX;       //終點x
    LONG EndPY;       //終點y
    LONG EndPZ;       //終點z
} DATA_3MOVE;
typedef std::vector<DATA_3MOVE> DATA_3MOVE_VECTOR;
void MO_AbsToOppo3Move(std::vector<DATA_3MOVE>
                       &);//絕對位置轉成相對位置-3軸插補


//===============================API命令列表================================
MO_DLL BOOL MO_Open(int interrupt);//開啟軸卡並將邏輯位置歸零
MO_DLL void MO_Close();//關閉軸卡
MO_DLL void MO_STOP();//停止驅動
MO_DLL void MO_DecSTOP();//減速停止
MO_DLL void MO_MoveToHome(LONG lSpeed1, LONG lSpeed2, int iAxis);//原點復歸
MO_DLL void MO_MoveToHome(LONG lSpeed1, LONG lSpeed2, int iAxis, LONG lMoveX,
                          LONG lMoveY, LONG
                          lMoveZ);//原點復歸-多載(速度1，速度2，指定軸7，偏移量X,Y,Z)
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
MO_DLL BOOL MO_ReadSwitchBtn();//讀取機台切換按鈕狀態
MO_DLL BOOL MO_ReadGlueOutBtn();//讀取機台排膠按鈕狀態
MO_DLL void MO_SetJerk(int iAxis,
                       LONG lSpeed);//設置加速度增加率(用於S型曲線)
MO_DLL void MO_SetDJerk(int iAxis,
                        LONG lSpeed); //設置減速度增加率(用於非對稱S型曲線)
MO_DLL void MO_SetDecPation(ULONG ulPationX, ULONG ulPationY,
                            ULONG ulPationZ);//設置減速度點(用於非對稱S型曲線)
MO_DLL void MO_SetDecOK(BOOL bData);//設置插補減速停止有無效
MO_DLL LONG MO_Timer(int iMode, int iTimerNo,
                     LONG lData);//計時器(選擇timer模式    0：設置TIMER/1：TIMER啟動/2：TIMER停止/3：讀取當下TIMER值)
MO_DLL BOOL MO_ReadGumming();//讀取出膠控制狀態
MO_DLL void MO_GummingSet(LONG lMicroSecond,
                          LPTHREAD_START_ROUTINE GummingTimeOutThread);//設置出膠秒數，單位[us]
MO_DLL void MO_GummingSet(int iData, LONG lMicroSecond,
                          LPTHREAD_START_ROUTINE
                          GummingTimeOutThread);//設定出膠時間-多載(選擇多個點膠埠1~3,出膠時間(usec),中斷程序名稱)
MO_DLL void MO_FinishGumming();//出膠完成
MO_DLL void MO_StopGumming();//停止出膠，清除Timer
MO_DLL void MO_Do3DLineMove(LONG lXTar, LONG lYTar, LONG lZTar, LONG lSpeed,
                            LONG lAcceleration, LONG lInitSpeed);
MO_DLL LONG MO_ReadReg(int iRegSelect, int iAxis);//讀取暫存器RR0,RR2,RR3
MO_DLL LONG MO_ReadSetData(int iSelect, int iAxis);//讀取暫存器設定值
MO_DLL void MO_ReadEvent(long *RR1X, long *RR1Y, long *RR1Z,
                         long *RR1U);//讀取中斷暫存器RR1
MO_DLL CString MO_StreamToUnit(int
                               iValue);//數字轉換成CString 16-bit binary
MO_DLL void MO_Do2ArcReduce(LONG lXTar, LONG lYTar, LONG XCenter, LONG YCenter,
                            LONG lInitSpeed, LONG lSpeed, BOOL bRevolve, LONG lAcceleration, LONG  lPation,
                            LPTHREAD_START_ROUTINE StopGumming);//XY兩軸圓弧補間移動(減速點)
MO_DLL void MO_Do3DLineMoveInterr(LONG lXTar, LONG lYTar, LONG lZTar,
                                  LONG lSpeed, LONG lAcceleration, LONG lInitSpeed,
                                  LPTHREAD_START_ROUTINE
                                  LPInterrupt); //XYZ三軸直線補間移動 - 多載(中斷)
MO_DLL void MO_TimerSetIntter(LONG lMicroSecond,
                              LPTHREAD_START_ROUTINE TimeOutThread);//設定計時器到後觸發中斷(usec)
//MO_DLL void MO_Do2ContData(DATA_2MOVE DATA_2MOVE);//2軸結構資料區間
//MO_DLL void MO_DO2ContStart(LONG lInitSpeed, LONG lAcceleration, LONG lSpeed,
//                            DATA_2MOVE DATA_2MOVE);//2軸連續補間開始
//MO_DLL void MO_Do3ContData(DATA_3MOVE DATA_3MOVE);//3軸結構資料區間
//MO_DLL void MO_DO3ContStart(LONG lInitSpeed, LONG lAcceleration, LONG lSpeed,
//                            DATA_3MOVE DATA_3MOVE);//3軸連續補間開始
//MO_DLL void MO_DO3DCurveStart(DATA_3CIP_MC8500P *data3CIP, int dataSize);
MO_DLL void MO_DO3Continuous(LONG lInitSpeed, LONG lAcceleration, LONG lSpeed,
                             DATA_3MOVE *DATA_3M, int iNum);//3軸連續補間(8筆)
MO_DLL void  MO_InterruptCase(BOOL bIsClose, int iData,
                              LPTHREAD_START_ROUTINE
                              Thread);//中斷條件選擇(0:等速脈波輸出開始/ 1:輸出結束/ 2:驅動開始/ 3:驅動結束/ 4:釋放驅動)
MO_DLL LONG  MO_Timer(int iMode, LONG lData, BOOL bIsUse,
                      LPTHREAD_START_ROUTINE
                      Thread);//設置TIMER((反複)多載---限定使用u軸), 選擇timer模式   0：設置TIMER/1：TIMER啟動/2：TIMER停止/3：讀取當下TIMER值, 是否使用執行緒(1用/01不用)

// CmccApp
// 這個類別的實作請參閱 mcc.cpp
//
void ResolutionChange(LONG &lValue);
void ResolutionInsChange(LONG &lValue);
void ResolutionChangeZ(LONG &lValue);
void ResolutionInsChangeZ(LONG &lValue);


class CmccApp : public CWinApp
{
    public:
        CmccApp();

        // 覆寫
    public:
        virtual BOOL InitInstance();

        DECLARE_MESSAGE_MAP()

};
