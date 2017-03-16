/*
*檔案名稱:mcc.h
*內容簡述:運動軸卡API，詳細參數請查看excel
*＠author 作者名稱:R
*＠date 更新日期:2017/03/14
*@虹橋2號W軸機專用 */
// mcc.h : mcc DLL 的主要標頭檔
//
#pragma once
#include <vector>
#ifndef __AFXWIN_H__
#error "對 PCH 包含此檔案前先包含 'stdafx.h'"
#endif
#include "resource.h"       // 主要符號

#ifdef MCC_DLL
#define MO_DLL __declspec(dllexport)
#else
#define MO_DLL __declspec(dllimport)
#endif
/***********軸卡編號************************/
#ifndef g_iId
#define g_iId 0
#endif
/***********馬達解析度************************/
//X,Y皮帶導程40mm(20齒1齒2mm)
//Z軸螺桿導程25mm
//解析度對照 XY:2  Z:2.5 W:1
//馬達驅動器上數值X: 20000,Y:10000, Z:10000 ,W: 4000;
#ifndef RESOLUTION
#define RESOLUTION 2.0
#endif

#ifndef RESOLUTION_Z
#define RESOLUTION_Z 2.5
#endif

#ifndef RESOLUTION_W
#define RESOLUTION_W 1
#endif

/***********極限開關專用************************/
//0正向 / 1反向
#ifndef HLM_DIC
#define HLM_DIC 1
#endif

/***********W軸專用************************/
//旋轉機構與馬達齒輪比1:2.5
#ifndef W_GEAR_RATIO
#define W_GEAR_RATIO 2.5
#endif
//步進馬達驅動器設定解析度
#ifndef W_MOTOR_DRIVER
#define W_MOTOR_DRIVER 4000.0
#endif

/***********雷射掃描使用********************/
#ifndef LA_SCANEND
#define LA_SCANEND -99999
#endif
//iAxe 指定軸名稱
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
//====================================資料型態定義===============================================================================================================
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
//=============四軸連續插補用===============================================
typedef struct _DATA_4MOVE
{
    LONG Speed;        //速度
    LONG Distance;     //距離
    LONG EndPX;        //終點x
    LONG EndPY;        //終點y
    LONG EndPZ;        //終點z
    DOUBLE AngleW;     //角度w
} DATA_4MOVE;
typedef std::vector<DATA_4MOVE> DATA_4MOVE_VECTOR;
//====================================API命令列表===============================================================================================================

//====================================基本動作========================================================

//開啟軸卡並將邏輯位置歸零
MO_DLL BOOL MO_Open(int interrupt);
//關閉軸卡
MO_DLL void MO_Close();
//停止驅動
MO_DLL void MO_STOP();
//減速停止
MO_DLL void MO_DecSTOP();
//停止連續/位元插補驅動
MO_DLL void MO_IPSTOP();
//DLL 版本資訊
MO_DLL void MO_DllVersion(CString &dllVersion);
//DLL 更新時間
MO_DLL void MO_DllDate(CString &dllDate);

//====================================運動動作========================================================

//原點復歸(速度1，速度2，指定軸，偏移量X,Y,Z)
MO_DLL void MO_MoveToHome(LONG lSpeed1, LONG lSpeed2, int iAxis, LONG lMoveX, LONG lMoveY, LONG lMoveZ);
//原點復歸W軸使用(速度1，速度2，指定軸，偏移量X,Y,Z,W,復歸方向0:負/1:正)
MO_DLL void MO_MoveToHomeW(LONG lSpeed1, LONG lSpeed2, int iAxe, LONG lMoveX, LONG lMoveY, LONG lMoveZ, DOUBLE dMoveW,BOOL bDir=0);
//XY兩軸直線補間移動(X,Y,驅動速度,加速度,初速度)
MO_DLL void MO_Do2DLineMove(LONG lXTar, LONG lYTar, LONG lSpeed, LONG lAcc, LONG lInit);
//Z軸直線補間移動(Z,驅動速度,加速度,初速度)
MO_DLL void MO_DoZLineMove(LONG lZTar, LONG lSpeed, LONG lAcc, LONG lInit);
//XY兩軸圓弧補間移動(X,Y,X中心點,Y中心點,驅動速度,初速度,1:正轉(CW)/0:逆轉(CCW))
MO_DLL void MO_Do2DArcMove(LONG lXTar, LONG lYTar, LONG XCenter, LONG YCenter, LONG lSpeed, LONG lInit, BOOL bRevolve);
//三軸直線插補(X,Y,Z,驅動速度,加速度,初速度)
MO_DLL void MO_Do3DLineMove(LONG lXTar, LONG lYTar, LONG lZTar, LONG lSpeed, LONG lAcc, LONG lInit);
//四軸直線插補(X,Y,Z,W,驅動速度,加速度,初速度)
MO_DLL void MO_Do4DLineMove(LONG lXTar, LONG lYTar, LONG lZTar, DOUBLE dWAngle, LONG lSpeed, LONG lAcc, LONG lInit);

//====================================設定數值========================================================

//設置硬體極限開關有效(指定軸,1=開/0=關)
MO_DLL void MO_SetHardLim(int iAxe, BOOL bSet);
//設置軟體極限開關有效(指定軸,1=開/0=關)
MO_DLL void MO_SetSoftLim(int iAxe, BOOL bSet);
//設置軟體極限範圍(0:正/1:負,x,y,z,w)
MO_DLL void MO_SetCompSoft(BOOL bPM, LONG lCompX, LONG lCompY, LONG lCompZ, DOUBLE dAngleW);
//設置減速度(指定軸,減速度)
MO_DLL void MO_SetDeceleration(int iAxe, LONG lDeceleration);
//設置加減速型態((0:對稱梯形/1:對稱S型/2:不對稱梯形/3:不對稱S型))
MO_DLL void MO_SetAccType(int iType);
//設置加速度增加率_S型曲線(指定軸,加速度率)
MO_DLL void MO_SetJerk(int iAxe, LONG lSpeed);
//設置減速度增加率_非對稱S型曲線(指定軸,減速度率)
MO_DLL void MO_SetDJerk(int iAxe, LONG lSpeed);
//設置減速度點_非對稱S型曲線(X,Y,Z,W)
MO_DLL void MO_SetDecPation(ULONG ulPationX, ULONG ulPationY, ULONG ulPationZ, DOUBLE dAngleW);
//設置插補減速停止有無效(1:有效/0:無效)
MO_DLL void MO_SetDecOK(BOOL bData);
//計時器(選擇timer模式 0：設置TIMER/1：TIMER啟動/2：TIMER停止/3：讀取當下TIMER值)
MO_DLL LONG MO_Timer(int iMode, int iTimerNo, LONG lData);
//設置TIMER多載(計時器觸發中斷輪詢), 選擇timer模式 0：設置TIMER/1：TIMER啟動/2：TIMER停止
MO_DLL LONG MO_Timer(int iMode, LONG lData);
//設定計時器到後觸發中斷(usec)(中斷秒數,中斷選擇0:Y/1:Z)
MO_DLL void MO_TimerSetIntter(LONG lMicroSecond, BOOL timerY0orZ1);
//設置出膠秒數，單位[us]
MO_DLL void MO_GummingSet(LONG lMicroSecond = 0);

//====================================讀取數值========================================================

//讀取邏輯位置(x=0,y=1,z=2,w=3)
MO_DLL LONG MO_ReadLogicPosition(int iAxis);
//讀取邏輯角度W
MO_DLL DOUBLE MO_ReadLogicPositionW();
//讀取實際位置(encoder專用)
MO_DLL LONG MO_ReadRealPosition(int iAxis);
//讀取實際位置角度w(encoder專用)
DOUBLE MO_ReadRealPositionW();
//讀取軸驅動速度(x=0,y=1,z=2,w=3)
MO_DLL LONG MO_ReadSpeed(int iAxis);
//讀取軸驅動速度角度W
MO_DLL DOUBLE MO_ReadSpeedW();
//讀取軸加減速度(x=0,y=1,z=2,w=3)
MO_DLL LONG MO_ReadAccDec(int iAxis);
//讀取軸加減速度角度
MO_DLL DOUBLE MO_ReadAccDecW();
//讀取是否正在驅動中(指定軸0~15)
MO_DLL BOOL MO_ReadIsDriving(int iAxe);
//讀取硬體極限開關設定狀態(讀取值為1=開/0=關)
MO_DLL void MO_ReadHardLimSet(BOOL &bHardX, BOOL &bHardY, BOOL &bHardZ, BOOL &bHardW);
//讀取軟體極限開關設定狀態(讀取值為1=開/0=關)
MO_DLL void MO_ReadSoftLimSet(BOOL &bSoftX, BOOL &bSoftY, BOOL &bSoftZ, BOOL &bSoftW);
//讀取是否在硬體極限開關上以及運行狀態(0:在負極限上/1:在正極限上/-1:不在極限上,0:極限故障/1:極限正常)
MO_DLL void MO_ReadRunHardLim(int &iPMLimX, BOOL &bIsOkLimX, int &iPMLimY, BOOL &bIsOkLimY, int &iPMLimZ, BOOL &bIsOkLimZ, int &iPMLimW, BOOL &bIsOkLimW);
//讀取軟體極限開關執行狀態(1:錯誤/0:正常)
MO_DLL void MO_ReadSoftLimError(BOOL bPM, BOOL &bSoftX, BOOL &bSoftY, BOOL &bSoftZ, BOOL &bSoftW);
//讀取伺服馬達輸入訊號異常(伺服馬達專用)
MO_DLL void MO_ReadMotoAlarm(BOOL &bMotoSignalX, BOOL &bMotoSignalY, BOOL &bMotoSignalZ, BOOL &bMotoSignalW);
//讀取各軸原點復歸錯誤狀態(1:錯誤/0:正常)
MO_DLL void MO_ReadHomeError(BOOL &bHomeErrorX, BOOL &bHomeErrorY, BOOL &bHomeErrorZ, BOOL &bHomeErrorW);
//讀取各軸插補錯誤狀態(1:錯誤/0:正常)
MO_DLL void MO_ReadIP(BOOL &bIPX, BOOL &bIPY, BOOL &bIPZ, BOOL &bIPW);
//讀取驅動中緊急停止狀態(1:錯誤/0:正常)
MO_DLL BOOL MO_ReadEMG();
//讀取通用暫存器PIO輸入(輸入0~11)
MO_DLL BOOL MO_ReadPIOInput(int iBit);
//設置通用暫存器PIO輸出(輸出0~15,1=開/0=關)
MO_DLL void MO_SetPIOOutput(int iBit, BOOL bData);
//讀取機台Start按鈕狀態(1:觸發/0:未觸發)
MO_DLL BOOL MO_ReadStartBtn();
//讀取機台切換按鈕狀態(1:觸發/0:未觸發)
MO_DLL BOOL MO_ReadSwitchBtn();
//讀取機台排膠按鈕狀態(1:觸發/0:未觸發)
MO_DLL BOOL MO_ReadGlueOutBtn();
//讀取出膠控制狀態(1:出膠/0:無)
MO_DLL BOOL MO_ReadGumming();
//讀取暫存器RR0,RR2,RR3
MO_DLL LONG MO_ReadReg(int iRegSelect, int iAxis);
//讀取暫存器設定值
MO_DLL LONG MO_ReadSetData(int iSelect, int iAxis);
//讀取中斷暫存器RR1
MO_DLL void MO_ReadEvent(long *RR1X, long *RR1Y, long *RR1Z, long *RR1U);

//====================================連續插補========================================================

//3軸連續補間(8筆)
MO_DLL void MO_DO3Continuous(LONG lInitSpeed, LONG lAcceleration, LONG lSpeed,DATA_3MOVE *DATA_3M, int iNum);
//4軸連續補間_資料
MO_DLL void MO_DO4Curve(DATA_4MOVE *DATA_4M, int dataCnt, LONG lSpeed);
//3軸連續補間_資料
MO_DLL void MO_DO3Curve(DATA_3MOVE *DATA_3M, int dataCnt, LONG lSpeed);
//2軸連續補間_資料
MO_DLL void MO_DO2Curve(DATA_2MOVE *DATA_2M, int dataCnt, LONG lSpeed);
//連續插補的減速點計算
LONG CalCipDecP(LONG InitSpd, LONG Dec, LONG Spd);
//絕對位置轉成相對位置-3軸插補
void MO_AbsToOppo3Move(std::vector<DATA_3MOVE> &);

//====================================其他設定========================================================

//錯誤狀態清除
MO_DLL void MO_AlarmCClean();
//出膠完成
MO_DLL void MO_FinishGumming();
//停止出膠，清除Timer
MO_DLL void MO_StopGumming();
//數字轉換成CString 16-bit binary
MO_DLL CString MO_StreamToUnit(int iValue);
//中斷條件選擇(0:等速脈波輸出開始/ 1:輸出結束/ 2:驅動開始/ 3:驅動結束/ 4:釋放驅動)
MO_DLL void MO_InterruptCase(BOOL bIsClose, int iData, LPTHREAD_START_ROUTINE Thread = NULL, LPVOID lpParameter = NULL);

//====================================馬達解析度========================================================
//馬達解析度X,Y(傳入)
void ResolutionChange(LONG &lValue);
//馬達解析度X,Y(傳出)
void ResolutionInsChange(LONG &lValue);
//馬達解析度Z(傳入)
void ResolutionChangeZ(LONG &lValue);
//馬達解析度Z(傳出)
void ResolutionInsChangeZ(LONG &lValue);
//馬達解析度W(傳入)
void ResolutionChangeW(LONG &lValue);
//馬達解析度W(傳出)
void ResolutionInsChangeW(LONG &lValue);
//====================================W軸解析度轉換========================================================
//W軸專用角度轉換脈波
MO_DLL LONG MO_AngleToPulse(DOUBLE dAngle);
//W軸專用脈波轉換角度
MO_DLL DOUBLE MO_PulseToAngle(LONG lPulse);


//MO_DLL void MO_Do2ContData(DATA_2MOVE DATA_2MOVE);//2軸結構資料區間
//MO_DLL void MO_DO2ContStart(LONG lInitSpeed, LONG lAcceleration, LONG lSpeed,
//                            DATA_2MOVE DATA_2MOVE);//2軸連續補間開始
//MO_DLL void MO_Do3ContData(DATA_3MOVE DATA_3MOVE);//3軸結構資料區間
//MO_DLL void MO_DO3ContStart(LONG lInitSpeed, LONG lAcceleration, LONG lSpeed,
//                            DATA_3MOVE DATA_3MOVE);//3軸連續補間開始
//MO_DLL void MO_DO3DCurveStart(DATA_3CIP_MC8500P *data3CIP, int dataSize);
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
