/*
*檔案名稱:Action.cpp(3D用)
*內容簡述:運動命令API，詳細參數請查看excel
*＠author 作者名稱:R
*＠data 更新日期:2016/09/26
*@更新內容三軸兩軸連續差補&雷射API*/
#include "stdafx.h"
#include "Action.h"
#define LA_SCANEND -99999
BOOL CAction::g_ZtimeOutGlueSet = FALSE;
BOOL CAction::g_YtimeOutGlueSet = FALSE;
/***********************************************************
**                                                        **
**          運動模組-運動指令命令 (對應動作判斷)             **
**                                                        **
************************************************************/
CAction::CAction()
{
	// 需宣告初始值
	pAction = this;
	g_bIsPause = FALSE;//暫停
	g_bIsStop = FALSE;//停止
	g_bIsGetLAend = FALSE;//掃描完成否
	g_bIsDispend = TRUE;//點膠機開
	g_iNumberGluePort = 1;//使用一個點膠埠
	g_interruptLock = FALSE;//中斷鎖
	g_OffSetLaserX = 0;//雷射偏移量初始化X
	g_OffSetLaserY = 0;//雷射偏移量初始化Y
	g_OffSetLaserZ = 0;//雷射偏移量初始化Z
	g_HeightLaserZero = 0;//雷射用Z軸歸零完成後高度(掃描高度)   
	g_LaserCnt = 0;//雷射線段計數器(使用次數)
	g_OffSetScan = 500;//雷射位移補償
	g_LaserAverage = FALSE; //雷射平均(1使用/0不使用)
	g_LaserAveBuffZ = 0; // 雷射用平均暫存值(絕對位置z值)
	m_hComm = NULL;
#ifdef MOVE
	LA_m_ptVec2D.clear();
	LA_m_ptVec.clear();
#endif
}
CAction::~CAction()
{
}
/******************************************************運動API**********************************************************/
//等待時間(執行續測試用)
void CAction::WaitTime(HANDLE wakeEvent, int Time)
{
	if (!g_bIsStop)
	{
		DWORD rc = WaitForSingleObject(wakeEvent, Time);
		switch (rc)
		{
		case WAIT_OBJECT_0:
			// wakeEvent signaled
			break;
		case WAIT_TIMEOUT:
			// 10-second timer passed 
			break;
		case WAIT_ABANDONED:
			// main thread ended 
			break;
		}
	}
	//for (int i = 0; i < 10; i++)
	//{
	//    if (!((COrder*)pParam)->StopFlag)
	//    {
	//        DWORD rc = WaitForSingleObject(wakeEvent, 1000);
	//        switch (rc)
	//        {
	//        case WAIT_OBJECT_0:
	//            // wakeEvent signaled
	//            ((COrder*)pParam)->Time = 100;
	//            break;
	//        case WAIT_TIMEOUT:
	//            // 10-second timer passed 
	//            ((COrder*)pParam)->Time++;
	//            break;
	//        case WAIT_ABANDONED:
	//            // main thread ended 
	//            return 0;
	//        }
	//    }
	//}
}
/*
*單點點膠動作(多載-結束設置+加速度+點到點速度)
*輸入(單點點膠、單點點膠設定、Z軸工作高度設定、點膠結束設定、加速度、（點到點）速度、系統預設參數)
*/
void CAction::DecidePointGlue(LONG lX, LONG lY, LONG lZ, LONG lDoTime, LONG lDelayStopTime,
	LONG lZBackDistance, BOOL bZDisType, LONG lZdistance, LONG lHighVelocity, LONG lLowVelocity, LONG lWorkVelociy,
	LONG lAcceleration, LONG lInitVelociy)
{
	/*單點點膠
	LONG lX, LONG lY, LONG lZ
	*/
	/*單點點膠設定
	LONG lDDoTime,LONG lDDelayStopTime
	*/
	/*Z軸工作高度設定-Z軸回升高度(相對)最高點
	LONG lDZBackDistance
	BOOL bZDisType(0絕對位置/1相對位置)
	*/
	/*點膠結束設定(Z軸相對距離，高速度，低速度，)
	lZdistance
	lHighVelocity
	lLowVelocity
	*/
	/*系統預設參數(驅動速度，加速度，初速度)
	LONG lWorkVelociy,LONG lAcceleration, LONG lInitVelociy
	*/
#ifdef MOVE
	LONG lNowX = 0, lNowY = 0, lNowZ = 0;
	lNowX = MO_ReadLogicPosition(0);
	lNowY = MO_ReadLogicPosition(1);
	lNowZ = MO_ReadLogicPosition(2);
	if (lHighVelocity == 0)
	{
		lHighVelocity = lWorkVelociy;
	}
	if (lLowVelocity == 0)
	{
		lLowVelocity = lWorkVelociy;
	}
	if (!bZDisType) //絕對位置
	{
		if (lZBackDistance > lZ)
		{
			lZBackDistance = lZ;
		}
		lZBackDistance = abs(lZBackDistance - lZ);
	}
	if (lZBackDistance>lZ)
	{
		lZBackDistance = lZ;
	}
	if (!g_bIsStop)
	{
		MO_Do3DLineMove(lX - lNowX, lY - lNowY, 0, lWorkVelociy, lAcceleration, lInitVelociy);//移動到點膠點
		PreventMoveError();//防止軸卡出錯
	}
	if (!g_bIsStop)
	{
		MO_Do3DLineMove(0, 0, lZ - lNowZ, lWorkVelociy, lAcceleration,lInitVelociy);//移動到點膠點
		PreventMoveError();//防止軸卡出錯
	}
	if (!g_bIsStop)
	{
		if (lDoTime == 0)
		{
			Sleep(1);
		}
		else
		{
			DoGlue(lDoTime, lDelayStopTime);//執行點膠
		}
	}
	if (lZdistance == 0)
	{
		if (!g_bIsStop)
		{
			MO_Do3DLineMove(0, 0, (lZ - lZBackDistance) - lZ, lWorkVelociy, lAcceleration, lInitVelociy);//點膠結束設置
			PreventMoveError();//防止軸卡出錯
		}
	}
	else
	{
		if (lZBackDistance == 0)
		{
			lZdistance = lZBackDistance;
		}
		if (lZdistance>lZBackDistance && lZBackDistance != 0)
		{
			lZdistance = lZBackDistance;
			MO_Do3DLineMove(0, 0, (lZ - lZBackDistance) - lZ, lHighVelocity, lAcceleration, lInitVelociy);//點膠結束設置
			PreventMoveError();//防止軸卡出錯
		}
		else
		{
			if (!g_bIsStop)
			{
				MO_Do3DLineMove(0, 0, (lZ - lZdistance) - lZ, lLowVelocity, lAcceleration, lInitVelociy);//點膠結束設置
				PreventMoveError();//防止軸卡出錯
			}
			if (!g_bIsStop)
			{
				MO_Do3DLineMove(0, 0, (lZ - lZBackDistance) - (lZ - lZdistance), lHighVelocity, lAcceleration, lInitVelociy);//點膠結束設置
				PreventMoveError();//防止軸卡出錯
			}
		}

	}
#endif
}
/*
*線段開始動作
*輸入(線段開始、線段點膠設定、加速度、線速度、系統預設參數)
*/
void CAction::DecideLineStartMove(LONG lX, LONG lY, LONG lZ, LONG lStartDelayTime,
	LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
	/*線段開始(x座標，y座標，z座標，線段起始點，)
	LONG lX, LONG lY, LONG lZ
	*/
	/*線段點膠設定(1.移動前延遲，)
	LONG lStartDelayTime
	*/
	//1.移動前點膠機在一條線段起始點處保持打開的時長。 此延時可防止針頭在流體流動之前沿線段發生移動。
	//2. 點膠機開啟前， 馬達離開直線線段起始點的移動距離。 該距離為馬達提供了足夠的起速時間，主要用來消除過量流體在線段起始處的積聚。
	/*系統參數(驅動速度，加速度，初速度)
	LONG lWorkVelociy,LONG lAcceleration, LONG lInitVelociy
	*/
#ifdef MOVE
	if (!g_bIsStop)
	{
		MO_Do3DLineMove(lX - MO_ReadLogicPosition(0), lY - MO_ReadLogicPosition(1),0, lWorkVelociy, lAcceleration, lInitVelociy);//x,y,z軸移動
		PreventMoveError();//防止軸卡出錯
	}
	if (!g_bIsStop)
	{
		MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//x,y,z軸移動
		PreventMoveError();//防止軸卡出錯
	}
	if (!g_bIsStop && g_bIsDispend == 1)
	{
		MO_GummingSet(g_iNumberGluePort, 0);//塗膠(不卡)
	}
	MO_Timer(0, 0, lStartDelayTime * 1000);
	MO_Timer(1, 0, lStartDelayTime * 1000);//線段點膠設定---(1)移動前延遲(在線段開始點上)
	while (MO_Timer(3, 0, 0))
	{
		if (g_bIsStop)
		{
			break;
		}
		Sleep(1);
	}
	PauseStopGlue();//暫停時停指塗膠(g_bIsPause=1)
#endif
}
/*
*線段中點動作
*輸入(線段中點、線段點膠設定、加速度、線速度、系統預設參數)
*/
void CAction::DecideLineMidMove(LONG lX, LONG lY, LONG lZ, LONG lMidDelayTime, LONG lWorkVelociy,
	LONG lAcceleration, LONG lInitVelociy)
{
	/*線段中點(x座標，y座標，z座標，線段起始點，)
	LONG lX, LONG lY, LONG lZ
	*/
	/*線段點膠設定(節點時間)
	LONG lMidDelayTime
	*/
#ifdef MOVE
	PauseDoGlue();//暫停恢復後繼續出膠(g_bIsPause=0) 出膠
	if (!g_bIsStop)
	{
		MO_Do3DLineMove(lX - MO_ReadLogicPosition(0), lY - MO_ReadLogicPosition(1), lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);// x,y軸移動
		PreventMoveError();//防止軸卡出錯
	}
	MO_Timer(0, 0, lMidDelayTime * 1000);
	MO_Timer(1, 0, lMidDelayTime * 1000);//線段點膠設定---(4)節點時間
	Sleep(1);//防止出錯，避免計時器初直為0
	while (MO_Timer(3, 0, 0))
	{
		if (g_bIsStop)
		{
			break;
		}
		Sleep(1);
	}
	PauseStopGlue();//暫停時停指塗膠(g_bIsPause=1)
#endif
}
/*
*線段結束動作
*輸入(線段結束點、線段點膠設定、加速度、線速度、Z軸工作高度設定、點膠結束設定、返回設定、系統參數)
*/
void CAction::DecideLineEndMove(LONG lX, LONG lY, LONG lZ, LONG lCloseOffDelayTime, LONG lCloseDistance,
	LONG lCloseONDelayTime, LONG lZBackDistance, BOOL bZDisType, LONG lHighVelocity, LONG lDistance, LONG lHigh,
	LONG lLowVelocity, int iType, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
	/*線段結束點(x座標，y座標，z座標，線段起始點，)
	LONG lX, LONG lY, LONG lZ
	*/
	/*線段點膠設定(停留時間，關機距離，關機延遲)
	LONG lCloseOffDelayTime ,LONG lCloseDistance ,LONG lCloseONDelayTime
	*/
	//3.點膠機關閉後，為了讓壓力在針頭移至下一點前變得均衡而在線段點膠結束點處產生的延時。
	//5.為防止過量流體在線段結束點處發生堆積，點膠機在距離線段結束點前多遠處關閉。
	//6.點膠機在線段結束點處停止後保持開啟的時長。
	/*返回設定(返回長度，z返回高度，返回速度，類型)
	LONG lDistance ,LONG lHigh ,LONG lLowVelocity,int iType
	*/
	/*Z軸工作高度設定(Z軸回升相對距離)
	LONG lZBackDistance
	BOOL bZDisType
	*/
	/*點膠結束設定(高速度)
	LONG lHighVelocity
	*/
#ifdef MOVE
	if(!bZDisType) //絕對位置
	{
		if (lZBackDistance > lZ)
		{
			lZBackDistance = lZ;
		}
		lZBackDistance = abs(lZBackDistance - lZ);
	}
	LONG lNowX = 0, lNowY = 0, lNowZ = 0, LineLength=0;
	LONG lLineClose = 0, lXClose = 0, lYClose = 0;
	PauseDoGlue();//暫停恢復後繼續出膠(g_bIsPause=0)
	lNowX = MO_ReadLogicPosition(0);
	lNowY = MO_ReadLogicPosition(1);
	lNowZ = MO_ReadLogicPosition(2);
	LineLength = LONG(sqrt(DOUBLE(pow(lX - lNowX, 2) + pow(lY - lNowY, 2))));
	if (lCloseDistance != 0 && lCloseDistance > LineLength)
	{
		lCloseDistance = LineLength;
	}
	if (lHighVelocity == 0)
	{
		lHighVelocity = lWorkVelociy;
	}
	if (lLowVelocity == 0)
	{
		lLowVelocity = lWorkVelociy;
	}
	if (lX == lNowX && lY == lNowY)//已經在結束點上不做關機距離
	{
		MO_Timer(0, 0, lCloseONDelayTime * 1000);
		MO_Timer(1, 0, lCloseONDelayTime * 1000);//線段點膠設定---(6)關機延遲 
		Sleep(1);//防止出錯，避免計時器初直為0
		while (MO_Timer(3, 0, 0))
		{
			if (g_bIsStop)
			{
				break;
			}
			Sleep(1);
		}
		MO_StopGumming();//停止出膠
		return;
	}
	else
	{
		if (lCloseDistance == 0) //線段點膠設定---(5)關機距離
		{
			if (!g_bIsStop)
			{
				MO_Do3DLineMove(lX - MO_ReadLogicPosition(0), lY - MO_ReadLogicPosition(1), lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);// z軸先下降
				PreventMoveError();//防止軸卡出錯
			}
			MO_Timer(0, 0, lCloseONDelayTime * 1000);
			MO_Timer(1, 0, lCloseONDelayTime * 1000);//線段點膠設定---(6)關機延遲  
			Sleep(1);//防止出錯，避免計時器初直為0
			while (MO_Timer(3, 0, 0))
			{
				if (g_bIsStop == 1)
				{
					break;
				}
				Sleep(1);
			}
			MO_StopGumming();//停止出膠
		}
		else
		{
			lLineClose = lCloseDistance;
			LineGetToPoint(lXClose, lYClose, lNowX, lNowY, lX, lY, lLineClose);
			if (!g_bIsStop)
			{
				MO_Do3DLineMove(lXClose - MO_ReadLogicPosition(0), lYClose - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration, lInitVelociy);//線段點膠設定---(5)關機距離
				PreventMoveError();//防止軸卡出錯
			}
			MO_StopGumming();//停止出膠
			if (!g_bIsStop)
			{
				MO_Do3DLineMove(lX - MO_ReadLogicPosition(0), lY - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration, lInitVelociy);// z軸先下降
				PreventMoveError();//防止軸卡出錯
			}
		}
	}

	GelatinizeBack(iType, lX, lY, lZ, lNowX, lNowY, lDistance, lHigh, lZBackDistance, lLowVelocity, lHighVelocity, lAcceleration, lInitVelociy);//返回設定
	MO_Timer(0, 0, lCloseOffDelayTime * 1000);
	MO_Timer(1, 0, lCloseOffDelayTime * 1000);//線段點膠設定---(3)停留時間 
	Sleep(1);//防止出錯，避免計時器初直為0
	while (MO_Timer(3, 0, 0))
	{
		if (g_bIsStop == 1)
		{
			break;
		}
		Sleep(1);
	}
#endif
}
/*
*(現在位置在結束點上)線段結束動作--多載
*輸入(線段結束點、線段點膠設定、加速度、線速度、Z軸工作高度設定、點膠結束設定、返回設定、系統參數、一般結束點使用)
*執行Z軸回升型態1
*/
void CAction::DecideLineEndMove(LONG lCloseOffDelayTime,
	LONG lCloseONDelayTime, LONG lZBackDistance, BOOL bZDisType, LONG lHighVelocity, LONG lDistance, LONG lHigh,
	LONG lLowVelocity, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy,BOOL bGeneral)
{
	/*線段點膠設定(停留時間，關機距離，關機延遲)
	LONG lCloseOffDelayTime ,LONG lCloseDistance ,LONG lCloseONDelayTime
	*/
	//3.點膠機關閉後，為了讓壓力在針頭移至下一點前變得均衡而在線段點膠結束點處產生的延時。
	//5.為防止過量流體在線段結束點處發生堆積，點膠機在距離線段結束點前多遠處關閉。
	//6.點膠機在線段結束點處停止後保持開啟的時長。
	/*返回設定(返回長度，z返回高度，返回速度，類型)
	LONG lDistance ,LONG lHigh ,LONG lLowVelocity
	*(iType 必為1使用兩段速回升)
	*/
	/*Z軸工作高度設定-Z軸回升高度(相對)最高點
	LONG lZBackDistance ,BOOL bZDisType(0絕對位置/1相對位置)
	*/
	/*點膠結束設定(高速度)
	LONG lHighVelocity
	*/
	/*
	BOOL bGeneral 用於不塗膠出膠的線段使用(1使用/0不使用)
	*/
#ifdef MOVE
	LONG lNowX = 0, lNowY = 0, lNowZ = 0;
	LONG lLineClose = 0, lXClose = 0, lYClose = 0;
	if (!bGeneral)
	{
		PauseDoGlue();//暫停恢復後繼續出膠(g_bIsPause=0)
	}
	lNowX = MO_ReadLogicPosition(0);
	lNowY = MO_ReadLogicPosition(1);
	lNowZ = MO_ReadLogicPosition(2);
	if (!bZDisType) //絕對位置
	{
		if (lZBackDistance > lNowZ)
		{
			lZBackDistance = lNowZ;
		}
		lZBackDistance = abs(lZBackDistance - lNowZ);
	}
	if (lHighVelocity == 0)
	{
		lHighVelocity = lWorkVelociy;
	}
	if (lLowVelocity == 0)
	{
		lLowVelocity = lWorkVelociy;
	}
	MO_Timer(0, 0, lCloseONDelayTime * 1000);
	MO_Timer(1, 0, lCloseONDelayTime * 1000);//線段點膠設定---(6)關機延遲
	Sleep(1);//防止出錯，避免計時器初直為0
	while (MO_Timer(3, 0, 0))
	{
		if (g_bIsStop == 1)
		{
			break;
		}
		Sleep(1);
	}
	if (!bGeneral || MO_ReadGumming())
	{
		MO_StopGumming();//停止出膠
	}
	GelatinizeBack(1, lNowX, lNowY, lNowZ, lNowX, lNowY, lDistance, lHigh, lZBackDistance, lLowVelocity, lHighVelocity, lAcceleration, lInitVelociy);//返回設定
	MO_Timer(0, 0, lCloseOffDelayTime * 1000);
	MO_Timer(1, 0, lCloseOffDelayTime * 1000);//線段點膠設定---(3)停留時間 
	Sleep(1);//防止出錯，避免計時器初直為0
	while (MO_Timer(3, 0, 0))
	{
		if (g_bIsStop == 1)
		{
			break;
		}
		Sleep(1);
	}
#endif
}
/*
*線段開始接線斷中間點動作
*輸入(線段開始X,Y,Z,線段中點X,Y,Z,移動前延遲,開機前從起點移動距離(設置距離),對線段中點的停留時間(節點時間ms),驅動速度,加速度,初速度)
*/
void CAction::DecideLineSToP(LONG lX, LONG lY, LONG lZ, LONG lX2, LONG lY2, LONG lZ2,
	LONG lStartDelayTime, LONG lStartDistance, LONG lMidDelayTime, 
	LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
	/*線段開始(x座標，y座標，z座標，線段起始點，)
	LONG lX, LONG lY, LONG lZ
	*/
	/*線段中點(x座標，y座標，z座標，線段起始點，)
	LONG lX2, LONG lY2, LONG lZ2
	*/
	/*線段點膠設定(1.移動前延遲，2.設置距離，)
	LONG lStartDelayTime ,LONG lStartDistance
	*/
	/*線段點膠設定(4節點時間)
	LONG lMidDelayTime
	*/
	//1.移動前點膠機在一條線段起始點處保持打開的時長。 此延時可防止針頭在流體流動之前沿線段發生移動。
	//2. 點膠機開啟前， 馬達離開直線線段起始點的移動距離。 該距離為馬達提供了足夠的起速時間，主要用來消除過量流體在線段起始處的積聚。
	//4.僅針對“線段中間點”命令而產生的延時。
	/*系統參數(驅動速度，加速度，初速度)
	LONG lWorkVelociy,LONG lAcceleration, LONG lInitVelociy
	*/
#ifdef MOVE
	LONG lNowX = 0, lNowY = 0;
	LONG lXClose = 0, lYClose = 0, lZClose = 0, lLineClose = 0;
	LONG lBuffX = 0, lBuffY = 0;
	LONG lTime = 0;
	BOOL bSame = 0;
	lNowX = MO_ReadLogicPosition(0);
	lNowY = MO_ReadLogicPosition(1);
	lLineClose = lStartDistance;
	LineGetToPoint(lXClose, lYClose, lZClose, lX2, lY2, lX, lY, lZ2, lZ, lLineClose);
	lBuffX = (-(lXClose - lX)) + lX;
	lBuffY = (-(lYClose - lY)) + lY;
	if (lStartDelayTime > 0 && lStartDistance > 0) //(當兩者都有值時以"移動前延遲"優先)
	{
		bSame = 1;
	}
	else
	{
		bSame = 0;
	}
	if (bSame == 1 || lStartDistance == 0)
	{
		if (!g_bIsStop)
		{
			MO_Do3DLineMove(lX - MO_ReadLogicPosition(0), lY - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration, lInitVelociy);//x,y,z軸移動
			PreventMoveError();//防止軸卡出錯
		}
		if (!g_bIsStop)
		{
			MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//x,y,z軸移動
			PreventMoveError();//防止軸卡出錯
		}
		if (!g_bIsStop)
		{
		if (!g_bIsStop && g_bIsDispend == 1)
		{
			MO_GummingSet(g_iNumberGluePort, 0);//塗膠(不卡)
		}
		MO_Timer(0, 0, lStartDelayTime * 1000);
		MO_Timer(1, 0, lStartDelayTime * 1000);//線段點膠設定---(1)移動前延遲(在線段開始點上)
		Sleep(1);//防止出錯，避免計時器初直為0
		while (MO_Timer(3, 0, 0))
		{
			if (g_bIsStop)
			{
				break;
			}
			Sleep(1);
		}
	}
		if (!g_bIsStop)
		{
			MO_Do3DLineMove(lX2 - MO_ReadLogicPosition(0), lY2 - MO_ReadLogicPosition(1), lZ2 - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//x,y,z軸(移動到點2)
			PreventMoveError();//防止軸卡出錯
		}
	}
	else
	{
		if (!g_bIsStop)
		{
			MO_Do3DLineMove(lBuffX - MO_ReadLogicPosition(0), lBuffY - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration, lInitVelociy);//x,y軸
			PreventMoveError();//防止軸卡出錯
		}
		if (!g_bIsStop)
		{
			MO_Do3DLineMove(0, 0, lZClose - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//x,y軸
			PreventMoveError();//防止軸卡出錯
		}
		if (!g_bIsStop)
		{
			MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//z軸先下降
			PreventMoveError();//防止軸卡出錯
		}
		lTime = CalPreglue(lStartDistance, lWorkVelociy, lAcceleration, lInitVelociy);
		/*======計時器到觸發中斷執行出膠，使用y中斷執行================*/
		if (!g_bIsStop)
		{
			CAction::g_YtimeOutGlueSet = TRUE;
			MO_TimerSetIntter(lTime, 0);//計時到跳至執行序
		}
		if (!g_bIsStop)
		{
			MO_Do3DLineMove(lX2 - MO_ReadLogicPosition(0), lY2 - MO_ReadLogicPosition(1), lZ2 - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//x,y軸(移動到點2)
			PreventMoveError();//防止軸卡出錯
		}
	}
	MO_Timer(0, 0, lMidDelayTime * 1000);
	MO_Timer(1, 0, lMidDelayTime * 1000);//線段點膠設定---(4)節點時間
	Sleep(1);//防止出錯，避免計時器初直為0
	while (MO_Timer(3, 0, 0))
	{
		if (g_bIsStop == 1)
		{
			break;
		}
		Sleep(1);
	}
	PauseStopGlue();//暫停時停指塗膠(g_bIsPause=1)
#endif
}
/*
*線段開始接線段結束動作
*輸入(線段開始X,Y,Z,結束點X,Y,Z,移動前延遲,開機前從起點移動距離(設置距離),關機後在結束點停留時間(停留時間),距離結束點多遠距離關機(關機距離),關機後的延遲時間(關機延遲),Z軸回升距離,Z軸回升型態,點膠結束設定(高速度),返回長度,z返回高度,返回速度(低速),返回類型,驅動速度,加速度,初速度)
*/
void CAction::DecideLineSToE(LONG lX, LONG lY, LONG lZ, LONG lX2, LONG lY2, LONG lZ2,
	LONG lStartDelayTime, LONG lStartDistance, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime, 
	LONG lZBackDistance, BOOL bZDisType, LONG lHighVelocity, LONG lDistance, LONG lHigh, 
	LONG lLowVelocity, int iType, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
	/*線段開始(x座標，y座標，z座標，線段起始點，)
	LONG lX, LONG lY, LONG lZ
	*/
	/*線段結束點(x座標，y座標，z座標，線段起始點，)
	LONG lX2, LONG lY2, LONG lZ2
	/*返回設定(返回長度，z返回高度，返回速度，類型，高速度)
	LONG lDistance ,LONG lHigh ,LONG lLowVelocity,int iType,lHighVelocity
	*/
	/*線段點膠設定(1.移動前延遲，2.設置距離，3停留時間，4關機距離，5關機延遲)
	LONG lStartDelayTime ,LONG lStartDistance ,LONG lCloseOffDelayTime ,LONG lCloseDistance ,LONG lCloseONDelayTime
	*/
	//1.移動前點膠機在一條線段起始點處保持打開的時長。 此延時可防止針頭在流體流動之前沿線段發生移動。
	//2. 點膠機開啟前， 馬達離開直線線段起始點的移動距離。 該距離為馬達提供了足夠的起速時間，主要用來消除過量流體在線段起始處的積聚。
	//3.點膠機關閉後，為了讓壓力在針頭移至下一點前變得均衡而在線段點膠結束點處產生的延時。
	//5.為防止過量流體在線段結束點處發生堆積，點膠機在距離線段結束點前多遠處關閉。
	//6.點膠機在線段結束點處停止後保持開啟的時長。
	/*系統參數(驅動速度，加速度，初速度)
	LONG lWorkVelociy,LONG lAcceleration, LONG lInitVelociy
	*/
#ifdef MOVE
	LONG lNowZ = 0;
	LONG lXClose = 0, lYClose = 0, lZClose = 0, lLineClose = 0;
	LONG lBuffX = 0, lBuffY = 0, LineLength=0;
	LONG lTime = 0;
	BOOL bSame = 0;
	lNowZ = MO_ReadLogicPosition(2);
	lLineClose = lStartDistance;
	LineGetToPoint(lXClose, lYClose, lZClose, lX2, lY2, lX, lY, lZ2, lZ, lLineClose);
	lBuffX = (-(lXClose - lX)) + lX;
	lBuffY = (-(lYClose - lY)) + lY;
	LineLength = LONG(sqrt(DOUBLE(pow(lX2 - lX, 2) + pow(lY2 - lY, 2))));
	if (lCloseDistance != 0 && lCloseDistance > LineLength)
	{
		lCloseDistance = LineLength;
	}
	if (!bZDisType)//絕對位置
	{
		if (lZBackDistance > lZ)
		{
			lZBackDistance = lZ;
		}
		lZBackDistance = abs(lZBackDistance - lZ);
	}
	if (lHighVelocity == 0)
	{
		lHighVelocity = lWorkVelociy;
	}
	if (lLowVelocity == 0)
	{
		lLowVelocity = lWorkVelociy;
	}
	if (lStartDelayTime > 0 && lStartDistance > 0) //(當兩者都有值時以"移動前延遲"優先)
	{
		bSame = 1;
	}
	else
	{
		bSame = 0;
	}
	if (bSame == 1 || lStartDistance == 0)
	{
		if (!g_bIsStop)
		{
			MO_Do3DLineMove(lX - MO_ReadLogicPosition(0), lY - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration, lInitVelociy);//x,y軸移動
			PreventMoveError();//防止軸卡出錯
		}
		if (!g_bIsStop)
		{
			MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//x,y軸移動
			PreventMoveError();//防止軸卡出錯
		}
		if (!g_bIsStop && g_bIsDispend == 1)
		{
			MO_GummingSet(g_iNumberGluePort, 0);//塗膠(不卡)
		}
		MO_Timer(0, 0, lStartDelayTime * 1000);
		MO_Timer(1, 0, lStartDelayTime * 1000);//線段點膠設定---(1)移動前延遲(在線段開始點上)
		Sleep(1);//防止出錯，避免計時器初直為0
		while (MO_Timer(3, 0, 0))
		{
			if (g_bIsStop)
			{
				break;
			}
			Sleep(1);
		}
		if (lCloseDistance == 0)//線段點膠設定---(5)關機距離
		{
			if (!g_bIsStop)
			{
				MO_Do3DLineMove(lX2 - MO_ReadLogicPosition(0), lY2 - MO_ReadLogicPosition(1), lZ2 - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//x,y軸移動(2)
				PreventMoveError();//防止軸卡出錯
			}
			MO_Timer(0, 0, lCloseONDelayTime * 1000);
			MO_Timer(1, 0, lCloseONDelayTime * 1000);//線段點膠設定---(6)關機延遲
			Sleep(1);//防止出錯，避免計時器初直為0
			while (MO_Timer(3, 0, 0))
			{		
				if (g_bIsStop == 1)
				{
					break;
				}
				Sleep(1);
			}
		}
		else
		{
			lLineClose = lCloseDistance;
			LineGetToPoint(lXClose, lYClose, lZClose, lX, lY, lX2, lY2, lZ, lZ2, lLineClose);
			if (!g_bIsStop)
			{
				MO_Do3DLineMove(lXClose - MO_ReadLogicPosition(0), lYClose - MO_ReadLogicPosition(1), lZClose - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//線段點膠設定---(5)關機距離
				PreventMoveError();//防止軸卡出錯
			}
			MO_StopGumming();//停止出膠
			if (!g_bIsStop)
			{
				MO_Do3DLineMove(lX2 - MO_ReadLogicPosition(0), lY2 - MO_ReadLogicPosition(1), lZ2- MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//x,y,Z軸移動
				PreventMoveError();//防止軸卡出錯
			}
		}
	}
	else
	{
		if (!g_bIsStop)
		{
			MO_Do3DLineMove(lBuffX - MO_ReadLogicPosition(0), lBuffY - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration, lInitVelociy);//x,y,Z軸移動
			PreventMoveError();//防止軸卡出錯
		}
		if (!g_bIsStop)
		{
			MO_Do3DLineMove(0, 0, lZClose - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//x,y,Z軸移動
			PreventMoveError();//防止軸卡出錯
		}
		lTime = CalPreglue(lStartDistance, lWorkVelociy, lAcceleration, lInitVelociy);//計算提前距離出膠時間
		/*======計時器到觸發中斷執行出膠，使用y中斷執行================*/
		if (!g_bIsStop)
		{
			CAction::g_YtimeOutGlueSet = TRUE;
			MO_TimerSetIntter(lTime, 0);//計時到跳至執行序
		}
		if (lCloseDistance == 0)//線段點膠設定---(5)關機距離
		{
			if (!g_bIsStop)
			{
				MO_Do3DLineMove(lX2 - MO_ReadLogicPosition(0), lY2 - MO_ReadLogicPosition(1), lZ2 - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//x,y,Z軸移動
				PreventMoveError();//防止軸卡出錯
			}
			MO_Timer(0, 0, lCloseONDelayTime * 1000);
			MO_Timer(1, 0, lCloseONDelayTime * 1000);//線段點膠設定---(6)關機延遲
			Sleep(1);//防止出錯，避免計時器初直為0
			while (MO_Timer(3, 0, 0))
			{		
				if (g_bIsStop == 1)
				{
					break;
				}
				Sleep(1);
			}
		}
		else
		{
			lLineClose = lCloseDistance;
			LineGetToPoint(lXClose, lYClose, lZClose, lX, lY, lX2, lY2, lZ, lZ2, lLineClose);
			if (!g_bIsStop)
			{
				MO_Do3DLineMove(lXClose - MO_ReadLogicPosition(0), lYClose - MO_ReadLogicPosition(1), lZClose - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);// 線段點膠設定-- - (5)關機距離
				PreventMoveError();//防止軸卡出錯
			}
			MO_StopGumming();//停止出膠
			if (!g_bIsStop)
			{
				MO_Do3DLineMove(lX2 - MO_ReadLogicPosition(0), lY2 - MO_ReadLogicPosition(1), lZ2 - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//x,y軸移動
				PreventMoveError();//防止軸卡出錯
			}
		}

	}
	MO_StopGumming();//停止出膠
	GelatinizeBack(iType, lX2, lY2, lZ2, lX, lY, lDistance, lHigh, lZBackDistance, lLowVelocity, lHighVelocity, lAcceleration, lInitVelociy);//返回設定
	MO_Timer(0, 0, lCloseOffDelayTime * 1000);
	MO_Timer(1, 0, lCloseOffDelayTime * 1000);//線段點膠設定---(3)停留時間 
	Sleep(1);//防止出錯，避免計時器初直為0
	while (MO_Timer(3, 0, 0))
	{
		if (g_bIsStop == 1)
		{
			break;
		}
		Sleep(1);
	}
#endif
}
/*
*圓形動作
*輸入(圓形、圓形結束點、線速、系統參數)
*/
void CAction::DecideCircle(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lWorkVelociy, LONG lInitVelociy)
{
	/*圓形(x座標，y座標，)
	LONG lX1, LONG lY1
	*/
	/*圓形結束點(x座標，y座標，)
	LONG lX2, LONG lY2
	*/
	/*系統參數(驅動速度，加速度，初速度)
	LONG lWorkVelociy, LONG lInitVelociy
	*/
#ifdef MOVE
	LONG lSpeed = 0, lInit = 0, lSSpeed = 0;
	LONG lCircleX = 0, lCircleY = 0;
	LONG lNowX = 0, lNowY = 0;
	BOOL bRev = 0;
	CString csX = 0, csY = 0;
	CString csBuff = 0, csNowPonit = 0, csLineCircle = 0, csLineCircleEnd = 0;
	lNowX = MO_ReadLogicPosition(0);
	lNowY = MO_ReadLogicPosition(1);
	csX.Format(L"%ld", lNowX);
	csY.Format(L"%ld", lNowY);
	csNowPonit = csX + _T("，") + csY + _T("，");
	csX.Format(L"%ld", lX1);
	csY.Format(L"%ld", lY1);
	csLineCircle = csX + _T("，") + csY + _T("，");
	csX.Format(L"%ld", lX2);
	csY.Format(L"%ld", lY2);
	csLineCircleEnd = csX + _T("，") + csY + _T("，");
	csBuff = TRoundCCalculation(csNowPonit, csLineCircleEnd, csLineCircle);
	lCircleX = CStringToLong(csBuff, 0);
	lCircleY = CStringToLong(csBuff, 1);
	bRev = CStringToLong(csBuff, 2);//取得圓心(X，Y，Rev，)
	PauseDoGlue();//暫停恢復後繼續出膠(g_bIsPause=0)
	if (!g_bIsStop)
	{
		MO_Do2DArcMove(0, 0, lCircleX - lNowX, lCircleY - lNowY, lInitVelociy, lWorkVelociy, bRev);
		PreventMoveError();//防止軸卡出錯
	}
	PauseStopGlue();//暫停時停指塗膠(g_bIsPause=1)
#endif
}
/*
*圓弧動作
*輸入(圓弧、圓弧結束點、線速、系統參數)
*/
void CAction::DecideArc(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lWorkVelociy, LONG lInitVelociy)
{
	/*圓弧(x座標，y座標，)
	LONG lX1, LONG lY1
	*/
	/*圓弧結束點(x座標，y座標，)
	LONG lX2, LONG lY2
	*/
	/*系統參數(驅動速度，加速度，初速度)
	LONG lWorkVelociy, LONG lInitVelociy
	*/
#ifdef MOVE
	LONG lSpeed = 0, lInit = 0, lSSpeed = 0;
	LONG lCircleX = 0, lCircleY = 0;
	LONG lNowX = 0, lNowY = 0;
	BOOL bRev = 0;
	CString csX = 0, csY = 0;
	CString csBuff = 0, csNowPonit = 0, csLineCircle = 0, csLineCircleEnd = 0;
	lNowX = MO_ReadLogicPosition(0);
	lNowY = MO_ReadLogicPosition(1);
	csX.Format(L"%ld", lNowX);
	csY.Format(L"%ld", lNowY);
	csNowPonit = csX + _T("，") + csY + _T("，");
	csX.Format(L"%ld", lX1);
	csY.Format(L"%ld", lY1);
	csLineCircle = csX + _T("，") + csY + _T("，");
	csX.Format(L"%ld", lX2);
	csY.Format(L"%ld", lY2);
	csLineCircleEnd = csX + _T("，") + csY + _T("，");
	csBuff = TRoundCCalculation(csNowPonit, csLineCircleEnd, csLineCircle);
	lCircleX = CStringToLong(csBuff, 0);
	lCircleY = CStringToLong(csBuff, 1);
	bRev = CStringToLong(csBuff, 2);//取得圓心(X，Y，Rev，)
	PauseDoGlue();//暫停恢復後繼續出膠(g_bIsPause=0)
	if (!g_bIsStop)
	{
		MO_Do2DArcMove(lX2 - lNowX, lY2 - lNowY, lCircleX - lNowX, lCircleY - lNowY, lInitVelociy, lWorkVelociy, bRev);
		PreventMoveError();//防止軸卡出錯
	}
	PauseStopGlue();//暫停時停指塗膠(g_bIsPause=1)
#endif
}
/*
*圓到結束點
*(輸入參數:圓形1，圓形2，線段結束點，線段點膠設定，返回設定，Z軸工作高度設定，點膠結束設定，系統參數)
*內容:輸入的結束點會判斷是否與現在位置相同，做出相對應的動作
*/
void CAction::DecideCircleToEnd(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lX3, LONG lY3, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime, LONG lZBackDistance, BOOL bZDisType, LONG lHighVelocity, LONG lDistance, LONG lHigh, LONG lLowVelocity, int iType, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
	/*圓形(x座標，y座標，)
	LONG lX1, LONG lY1
	*/
	/*圓形結束點(x座標，y座標，)線段結束點(x座標，y座標，z座標，線段起始點，)
	LONG lX2, LONG lY2
	*/
	/*線段結束點(x座標，y座標，z座標，線段起始點，)
	LONG lX3, LONG lY3
	*/
	/*線段點膠設定(停留時間，關機距離，關機延遲)
	LONG lCloseOffDelayTime ,LONG lCloseDistance ,LONG lCloseONDelayTime
	//3.點膠機關閉後，為了讓壓力在針頭移至下一點前變得均衡而在線段點膠結束點處產生的延時。
	//5.為防止過量流體在線段結束點處發生堆積，點膠機在距離線段結束點前多遠處關閉。
	//6.點膠機在線段結束點處停止後保持開啟的時長。
	*/
	/*返回設定(返回長度，z返回高度，返回速度，類型)
	LONG lDistance ,LONG lHigh ,LONG lLowVelocity,int iType
	*/
	/*Z軸工作高度設定-Z軸回升高度(相對)最高點
	LONG lZBackDistance ,BOOL bZDisType(0絕對位置/1相對位置)
	*/
	/*點膠結束設定(高速度)
	LONG lHighVelocity
	*/
	/*系統參數(驅動速度，加速度，初速度)
	LONG lWorkVelociy,LONG lAcceleration, LONG lInitVelociy
	*/
#ifdef MOVE
	LONG lCircleX = 0, lCircleY = 0;
	LONG lNowX = 0, lNowY = 0, lNowZ = 0;
	BOOL bRev = 0;
	CString csX = 0, csY = 0;
	CString csBuff = 0, csNowPonit = 0, csLineCircle = 0, csLineCircleEnd = 0;
	LONG lLineClose = 0, lXClose = 0, lYClose = 0;
	LONG lR = 0, LineLength = 0;//斷膠點與半徑
	LONG lEndX = 0, lEndY = 0;
	lNowX = MO_ReadLogicPosition(0);
	lNowY = MO_ReadLogicPosition(1);
	lNowZ = MO_ReadLogicPosition(2);
	csX.Format(L"%ld", lNowX);
	csY.Format(L"%ld", lNowY);
	csNowPonit = csX + _T("，") + csY + _T("，");
	csX.Format(L"%ld", lX1);
	csY.Format(L"%ld", lY1);
	csLineCircle = csX + _T("，") + csY + _T("，");
	csX.Format(L"%ld", lX2);
	csY.Format(L"%ld", lY2);
	csLineCircleEnd = csX + _T("，") + csY + _T("，");
	csBuff = TRoundCCalculation(csNowPonit, csLineCircleEnd, csLineCircle);
	lCircleX = CStringToLong(csBuff, 0);
	lCircleY = CStringToLong(csBuff, 1);
	bRev = CStringToLong(csBuff, 2);//取得圓心(X，Y，Rev，)
	lR = LONG(sqrt(pow(lNowX - lCircleX, 2) + pow(lNowY - lCircleY, 2)));//半徑
	if (lHighVelocity == 0)
	{
		lHighVelocity = lWorkVelociy;
	}
	if (lLowVelocity == 0)
	{
		lLowVelocity = lWorkVelociy;
	}
	if (!bZDisType)//絕對位置
	{
		if (lZBackDistance > lNowZ)
		{
			lZBackDistance = lNowZ;
		}
		lZBackDistance = abs(lZBackDistance - lNowZ);
	}
	PauseDoGlue();//暫停恢復後繼續出膠(g_bIsPause=0)
	if ((lNowX >= lX3 - 5) && (lNowX <= lX3 + 5) && (lNowY >= lY3 - 5) && (lNowY <= lY3 + 5)) //表示結束點在起始點上
	{
		LineLength = LONG((2 * M_PI*lR));
		if (lCloseDistance != 0 && lCloseDistance > LineLength)
		{
			lCloseDistance = LineLength;
		}
		if (lCloseDistance == 0) //線段點膠設定---(5)關機距離
		{
			if (!g_bIsStop)
			{
				MO_Do2DArcMove(0, 0, lCircleX - lNowX, lCircleY - lNowY, lInitVelociy, lWorkVelociy, bRev);//圓
				PreventMoveError();//防止軸卡出錯
			}
			MO_Timer(0, 0, lCloseONDelayTime * 1000);
			MO_Timer(1, 0, lCloseONDelayTime * 1000);//線段點膠設定---(6)關機延遲
			Sleep(1);//防止出錯，避免計時器初直為0
			while (MO_Timer(3, 0, 0))
			{
				if (g_bIsStop == 1)
				{
					break;
				}
				Sleep(1);
			}
		}
		else
		{
			ArcGetToPoint(lEndX, lEndY, lCloseDistance, lX3, lY3, lCircleX, lCircleY, lR, bRev);//算出斷膠點
			lCloseDistance = LONG((2 * M_PI*lR)) - lCloseDistance;
			if (!g_bIsStop)
			{
				MO_Do2DArcMove(lEndX - lNowX, lEndY - lNowY, lCircleX - lNowX, lCircleY - lNowY, lInitVelociy, lWorkVelociy, bRev);//圓斷膠
				PreventMoveError();//防止軸卡出錯
			}
			MO_StopGumming();//斷膠
			if (!g_bIsStop)
			{
				MO_Do2DArcMove(lX3 - lEndX, lY3 - lEndY, lCircleX - lEndX, lCircleY - lEndY, lInitVelociy, lWorkVelociy, bRev);//圓斷膠
				PreventMoveError();//防止軸卡出錯
			}
		}
		MO_StopGumming();//停止出膠
						 //返回設定的參考點(結束點是lx3,ly3,出發參考點使用圓2)
		GelatinizeBack(iType, lX3, lY3, lNowZ, lX2, lY2, lDistance, lHigh, lZBackDistance, lLowVelocity, lHighVelocity, lAcceleration, lInitVelociy);//返回設定
		MO_Timer(0, 0, lCloseOffDelayTime * 1000);
		MO_Timer(1, 0, lCloseOffDelayTime * 1000);//線段點膠設定---(3)停留時間 
		Sleep(1);//防止出錯，避免計時器初直為0
		while (MO_Timer(3, 0, 0))
		{
			if (g_bIsStop == 1)
			{
				break;
			}
		}
	}
	else//表示結束點不在圓上
	{
		LineLength = LONG(sqrt(DOUBLE(pow(lX3 - lX1, 2) + pow(lY3 - lY1, 2))));
		if (lCloseDistance != 0 && lCloseDistance > LineLength)
		{
			lCloseDistance = LineLength;
		}
		if (!g_bIsStop)
		{
			MO_Do2DArcMove(0, 0, lCircleX - lNowX, lCircleY - lNowY, lInitVelociy, lWorkVelociy, bRev);//圓
			PreventMoveError();//防止軸卡出錯
		}
		DecideLineEndMove(lX3, lY3, lNowZ, lCloseOffDelayTime, lCloseDistance, lCloseONDelayTime, lZBackDistance, bZDisType, lHighVelocity, lDistance, lHigh, lLowVelocity, iType, lWorkVelociy, lAcceleration, lInitVelociy);
		PreventMoveError();//防止軸卡出錯
	}
#endif
}
/*
*圓弧到結束點
*(輸入參數:圓弧，線段結束點，線段點膠設定，返回設定，Z軸工作高度設定，點膠結束設定，系統參數)
*內容:輸入的圓弧是連接結束點，做出相對應的動作
*/
void CAction::DecideArcleToEnd(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime, LONG lZBackDistance, BOOL bZDisType, LONG lHighVelocity, LONG lDistance, LONG lHigh, LONG lLowVelocity, int iType, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
	/*圓弧(x座標，y座標，)
	LONG lX1, LONG lY1
	*/
	/*線段結束點(x座標，y座標，z座標，線段起始點，)
	LONG lX2, LONG lY2
	*/
	/*線段點膠設定(停留時間，關機距離，關機延遲)
	LONG lCloseOffDelayTime ,LONG lCloseDistance ,LONG lCloseONDelayTime
	//3.點膠機關閉後，為了讓壓力在針頭移至下一點前變得均衡而在線段點膠結束點處產生的延時。
	//5.為防止過量流體在線段結束點處發生堆積，點膠機在距離線段結束點前多遠處關閉。
	//6.點膠機在線段結束點處停止後保持開啟的時長。
	*/
	/*返回設定(返回長度，z返回高度，返回速度，類型)
	LONG lDistance ,LONG lHigh ,LONG lLowVelocity,int iType
	*/
	/*Z軸工作高度設定-Z軸回升高度(相對)最高點
	LONG lZBackDistance ,BOOL bZDisType(0絕對位置/1相對位置)
	*/
	/*點膠結束設定(高速度)
	LONG lHighVelocity
	*/
	/*系統參數(驅動速度，加速度，初速度)
	LONG lWorkVelociy,LONG lAcceleration, LONG lInitVelociy
	*/
#ifdef MOVE
	LONG lCircleX = 0, lCircleY = 0;
	LONG lNowX = 0, lNowY = 0, lNowZ = 0;
	BOOL bRev = 0;
	CString csX = 0, csY = 0;
	CString csBuff = 0, csNowPonit = 0, csLineCircle = 0, csLineCircleEnd = 0;
	LONG lLineClose = 0, lXClose = 0, lYClose = 0;
	LONG lR = 0;//斷膠點與半徑
	LONG lEndX = 0, lEndY = 0, lCrev = 0;
	DOUBLE dAngl = 0;
	lNowX = MO_ReadLogicPosition(0);
	lNowY = MO_ReadLogicPosition(1);
	lNowZ = MO_ReadLogicPosition(2);
	csX.Format(L"%ld", lNowX);
	csY.Format(L"%ld", lNowY);
	csNowPonit = csX + _T("，") + csY + _T("，");
	csX.Format(L"%ld", lX1);
	csY.Format(L"%ld", lY1);
	csLineCircle = csX + _T("，") + csY + _T("，");
	csX.Format(L"%ld", lX2);
	csY.Format(L"%ld", lY2);
	csLineCircleEnd = csX + _T("，") + csY + _T("，");
	csBuff = TRoundCCalculation(csNowPonit, csLineCircleEnd, csLineCircle);
	lCircleX = CStringToLong(csBuff, 0);
	lCircleY = CStringToLong(csBuff, 1);
	bRev = CStringToLong(csBuff, 2);//取得圓心(X，Y，Rev，)
	lR = LONG(sqrt(pow(lNowX - lCircleX, 2) + pow(lNowY - lCircleY, 2)));//半徑
	dAngl = AngleCount(lCircleX, lCircleY, lNowX, lNowY, lX2, lY2, bRev);//角度
	lCrev = LONG(2 * lR * M_PI * dAngl / 360.0);
	if (lCloseDistance != 0 && lCloseDistance > lCrev)
	{
		lCloseDistance = lCrev;
	}
	if (lHighVelocity == 0)
	{
		lHighVelocity = lWorkVelociy;
	}
	if (lLowVelocity == 0)
	{
		lLowVelocity = lWorkVelociy;
	}
	if (!bZDisType)//絕對位置
	{
		if (lZBackDistance > lNowZ)
		{
			lZBackDistance = lNowZ;
		}
		lZBackDistance = abs(lZBackDistance - lNowZ);
	}

	PauseDoGlue();//暫停恢復後繼續出膠(g_bIsPause=0)
	if (lCloseDistance == 0) //線段點膠設定---(5)關機距離
	{
		if (!g_bIsStop)
		{
			MO_Do2DArcMove(lX2 - lNowX, lY2 - lNowY, lCircleX - lNowX, lCircleY - lNowY, lInitVelociy, lWorkVelociy, bRev);//圓
			PreventMoveError();//防止軸卡出錯
		}
		MO_Timer(0, 0, lCloseONDelayTime * 1000);
		MO_Timer(1, 0, lCloseONDelayTime * 1000);//線段點膠設定---(6)關機延遲
		Sleep(1);//防止出錯，避免計時器初直為0
		while (MO_Timer(3, 0, 0))
		{
			if (g_bIsStop == 1)
			{
				break;
			}
			Sleep(1);
		}
	}
	else
	{
		ArcGetToPoint(lEndX, lEndY, lCloseDistance, lX2, lY2, lCircleX, lCircleY, lR, bRev);//算出斷膠點
		if (lCrev > lCloseDistance)
		{
			if (!g_bIsStop)
			{
				MO_Do2DArcMove(lEndX - lNowX, lEndY - lNowY, lCircleX - lNowX, lCircleY - lNowY, lInitVelociy, lWorkVelociy, bRev);//圓斷膠
				PreventMoveError();//防止軸卡出錯
			}
			MO_StopGumming();//斷膠
			if (!g_bIsStop)
			{
				MO_Do2DArcMove(lX2 - lEndX, lY2 - lEndY, lCircleX - lEndX, lCircleY - lEndY, lInitVelociy, lWorkVelociy, bRev);//圓斷膠
				PreventMoveError();//防止軸卡出錯
			}
		}
		else
		{
					MO_StopGumming();//停止出膠
			AfxMessageBox(L"斷膠距離過大");
		}
	}
	MO_StopGumming();//停止出膠
					 //返回設定的參考點(結束點是lx2,ly2,出發參考點使用圓弧中點)
	GelatinizeBack(iType, lX2, lY2, lNowZ, lX1, lY1, lDistance, lHigh, lZBackDistance, lLowVelocity, lHighVelocity, lAcceleration, lInitVelociy);//返回設定
	MO_Timer(0, 0, lCloseOffDelayTime * 1000);
	MO_Timer(1, 0, lCloseOffDelayTime * 1000);//線段點膠設定---(3)停留時間 
	Sleep(1);//防止出錯，避免計時器初直為0
	while (MO_Timer(3, 0, 0))
	{
		if (g_bIsStop == 1)
		{
			break;
		}
	}
#endif
}
/*
*虛擬點動作
*輸入(虛擬點、系統參數)
*/
void CAction::DecideVirtualPoint(LONG lX, LONG lY, LONG lZ, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
	/*虛擬點(x座標，y座標，z座標，)
	LONG lX, LONG lY, LONG lZ
	*/
	/*系統參數(驅動速度，加速度，初速度)
	LONG lWorkVelociy,LONG lAcceleration, LONG lInitVelociy
	*/
#ifdef MOVE
	if (!g_bIsStop)
	{
		MO_Do3DLineMove(lX - MO_ReadLogicPosition(0), lY - MO_ReadLogicPosition(1), lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//x,y軸移動
		PreventMoveError();//防止軸卡出錯
	}
#endif
}
/*
*等待點動作
*輸入(等待點、系統參數)
*/
void CAction::DecideWaitPoint(LONG lX, LONG lY, LONG lZ, LONG lWaitTime, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
	/*等待點(x座標，y座標，z座標，等待時間，)
	LONG lX, LONG lY, LONG lZ ,LONG lWaitTime
	*/
	/*系統參數(驅動速度，加速度，初速度)
	LONG lWorkVelociy,LONG lAcceleration, LONG lInitVelociy
	*/
#ifdef MOVE
	if (!g_bIsStop)
	{
		MO_Do3DLineMove(lX - MO_ReadLogicPosition(0), lY - MO_ReadLogicPosition(1), lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//x,y軸移動
		PreventMoveError();//防止軸卡出錯
	}
	MO_Timer(0, 0, lWaitTime * 1000);
	MO_Timer(1, 0, lWaitTime * 1000);//等待時間(us→ms)
	Sleep(1);//防止出錯，避免計時器初直為0
	while (MO_Timer(3, 0, 0))
	{
		if (g_bIsStop == 1)
		{
			break;
		}
		Sleep(1);
	}
#endif
}
/*
*停駐點動作
*輸入(停駐點、系統參數)
*/
void CAction::DecideParkPoint(LONG lX, LONG lY, LONG lZ, LONG lTimeGlue, LONG lWaitTime, LONG lStayTime, LONG lZBackDistance, BOOL bZDisType, LONG lZdistance, LONG lHighVelocity, LONG lLowVelocity, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
	/*停駐點(x座標，y座標，z座標，排膠時間，結束後等待時間，)
	LONG lX, LONG lY, LONG lZ,LONG lTimeGlue,LONG lWaitTime
	*/
	/*Z軸工作高度設定-Z軸回升高度(相對)最高點
	LONG lZBackDistance ,BOOL bZDisType(0絕對位置/1相對位置)
	*/
	/*點膠結束設定(Z軸相對距離，高速度，低速度，)
	LONG lZdistance,LONG lHighVelocity,LONG lLowVelocity
	*/
	/*系統參數(驅動速度，加速度，初速度)
	LONG lWorkVelociy,LONG lAcceleration, LONG lInitVelociy
	*/
#ifdef MOVE
	if (lHighVelocity == 0)
	{
		lHighVelocity = lWorkVelociy;
	}
	if (lLowVelocity == 0)
	{
		lLowVelocity = lWorkVelociy;
	}
	if (!bZDisType) //絕對位置
	{
		if (lZBackDistance > lZ)
		{
			lZBackDistance = lZ;
		}
		lZBackDistance = abs(lZBackDistance - lZ);
	}

	if (!g_bIsStop)
	{
		MO_Do3DLineMove(lX - MO_ReadLogicPosition(0), lY - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration, lInitVelociy);//x,y軸移動
		PreventMoveError();//防止軸卡出錯
	}
	if (!g_bIsStop)
	{
		MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//z軸移動
		PreventMoveError();//防止軸卡出錯
	}
	MO_Timer(0, 0, lWaitTime * 1000);
	MO_Timer(1, 0, 0);//(ms)
	Sleep(1);//防止出錯，避免計時器初直為0
	while (MO_Timer(3, 0, 0))
	{
		if (g_bIsStop == 1)
		{
			MO_Timer(2, 0, 0);//停止計時器
			break;
		}
		Sleep(1);
	}
	if (lTimeGlue == 0)
	{
		Sleep(1);
	}
	else
	{
		DoGlue(lTimeGlue, lStayTime);//執行排膠
	}
	Sleep(10);//防止程式出錯
			  //排膠完後的回升動作
	if (!MO_ReadGumming())
	{
		if (lZdistance == 0)
		{
			if (!g_bIsStop)
			{
				MO_Do3DLineMove(0, 0, (lZ - lZBackDistance) - lZ, lWorkVelociy, lAcceleration,
					lInitVelociy);//點膠結束設置
				PreventMoveError();//防止軸卡出錯
			}
		}
		else
		{
			if (lZdistance>lZBackDistance && lZBackDistance != 0)
			{
				lZdistance = lZBackDistance;
				MO_Do3DLineMove(0, 0, (lZ - lZBackDistance) - lZ, lHighVelocity, lAcceleration,
					lInitVelociy);//點膠結束設置
				PreventMoveError();//防止軸卡出錯
			}
			else
			{
				if (!g_bIsStop)
				{
					MO_Do3DLineMove(0, 0, (lZ - lZdistance) - lZ, lLowVelocity, lAcceleration,
						lInitVelociy);//點膠結束設置
					PreventMoveError();//防止軸卡出錯
				}
				if (!g_bIsStop)
				{
					MO_Do3DLineMove(0, 0, (lZ - lZBackDistance) - (lZ - lZdistance), lHighVelocity,
						lAcceleration, lInitVelociy);//點膠結束設置
					PreventMoveError();//防止軸卡出錯
				}
			}

		}
	}
#endif
}
/*
*原點賦歸動作
*輸入(LONG速度1，LONG速度2，LONG指定軸(0~7)，LONG偏移量)
*/
void CAction::DecideInitializationMachine(LONG lSpeed1, LONG lSpeed2, LONG lAxis, LONG lMoveX, LONG lMoveY, LONG lMoveZ)
{
#ifdef MOVE
	MO_SetHardLim(lAxis, 1);
	//TODO::軟體極限要記得改
	MO_SetSoftLim(lAxis, 0);
	MO_FinishGumming();
	if (!g_bIsStop)
	{
		MO_MoveToHome(lSpeed1, lSpeed2, lAxis, lMoveX, lMoveY, lMoveZ);
		PreventMoveError();//防止軸卡出錯
	}
	MO_SetSoftLim(lAxis, 1);
#endif
}
/*
*bZDisType  TRUE:相對座標/FALSE:絕對座標
*/
void CAction::DecideVirtualHome(LONG lX, LONG lY, LONG lZ, LONG lZBackDistance, BOOL bZDisType, LONG lWorkVelociy,LONG lAcceleration, LONG lInitVelociy)
{
#ifdef MOVE
	if (lZBackDistance <= 0)
	{
		return;
	}
	if (bZDisType)
	{
		//相對座標
		if (!g_bIsStop)
		{
			//lZBackDistance超過可移動距離
			lZBackDistance = (lZBackDistance > MO_ReadLogicPosition(2)) ? MO_ReadLogicPosition(2) : lZBackDistance;
			//z go back
			MO_Do3DLineMove(0, 0, -lZBackDistance, lWorkVelociy, lAcceleration, lInitVelociy);
			PreventMoveError();//防止軸卡出錯
		}
	}
	else
	{
		//絕對座標
		if (!g_bIsStop)
		{
			if (lZBackDistance < MO_ReadLogicPosition(2))
			{
				//z go back
				MO_Do3DLineMove(0, 0, lZBackDistance - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);
				PreventMoveError();//防止軸卡出錯
			}
		}
	}
	if (!g_bIsStop)
	{
		//xyz move
		MO_Do3DLineMove(lX - MO_ReadLogicPosition(0), lY - MO_ReadLogicPosition(1), lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);
		PreventMoveError();//防止軸卡出錯
	}
#endif
}
/*
針頭清潔裝置
*/
void CAction::DispenClear(LONG lX, LONG lY, LONG lZ, int ClreaPort, LONG lZBackDistance, BOOL bZDisType, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
	/*參數說明
	針頭清潔裝置位置
	LONG lX,LONG lY,LONG lZ
	裝置使用的輸出IO腳位(輸入0~7)
	int ClreaPort
	Z軸工作高度設定-Z軸回升高度(相對)最高點
	LONG lZBackDistance ,BOOL bZDisType(0絕對位置/1相對位置)
	系統參數(驅動速度，加速度，初速度)
	LONG lWorkVelociy,LONG lAcceleration, LONG lInitVelociy
	*/
	if (bZDisType) //絕對位置
	{
		if (lZBackDistance > lZ)
		{
			lZBackDistance = lZ;
		}
		lZBackDistance = abs(lZBackDistance - lZ);
	}
	if (lZBackDistance > lZ)
	{
		lZBackDistance = lZ;
	}
#ifdef MOVE
	//移動X,Y,Z至針頭清潔裝置位置
	if (!g_bIsStop)
	{
		MO_Do3DLineMove(lX - MO_ReadLogicPosition(0), lY - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration, lInitVelociy);//x,y軸移動
		PreventMoveError();//防止軸卡出錯
	}
	if (!g_bIsStop)
	{
		MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//z軸移動
		PreventMoveError();//防止軸卡出錯
	}
	//啟動針頭清潔io裝置
	DecideOutPutSign(ClreaPort, 1);
	//執行單點點膠
	DoGlue(1000, 1000);//出膠一秒延遲一秒
	Sleep(10);
	//關閉針頭清潔io裝置
	DecideOutPutSign(ClreaPort, 0);
	//z軸回升到最高點
	if (!g_bIsStop)
	{
		MO_Do3DLineMove(0, 0, lZBackDistance - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//z軸移動
		PreventMoveError();//防止軸卡出錯
	}
#endif
}
/*
*填充動作
*輸入(線段開始，線段結束，Z軸工作高度設定，填充命令，系統參數)
*/
void CAction::DecideFill(LONG lX1, LONG lY1, LONG lZ1, LONG lX2, LONG lY2, LONG lZ2,
	LONG lZBackDistance, BOOL bZDisType, int iType,LONG lWidth, LONG lWidth2,
	LONG lStartDelayTime, LONG lStartDistance, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime, 
	LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
	/*線段開始(x座標，y座標，z座標，線段起始點，)
	LONG lX1, LONG lY1, LONG lZ1
	*/
	/*線段結束(x座標，y座標，z座標，線段結束點，)
	LONG lX2, LONG lY2, LONG lZ2
	*/
	/*Z軸工作高度設定-Z軸回升高度(相對)最高點
	LONG lZBackDistance ,BOOL bZDisType(0絕對位置/1相對位置)
	*/
	/*填充命令(填充形式(1~7)，寬度(mm)，兩端寬度(mm)，)
	int iType, LONG lWidth, LONG lWidth2
	*/
	/*系統參數(驅動速度，加速度，初速度)
	LONG lWorkVelociy,LONG lAcceleration, LONG lInitVelociy
	*/
	/*線段點膠設定(1.移動前延遲，2.設置距離，3停留時間，5關機距離，6關機延遲)
	LONG lStartDelayTime ,LONG lStartDistance ,LONG lCloseOffDelayTime ,LONG lCloseDistance ,LONG lCloseONDelayTime
	//1.移動前點膠機在一條線段起始點處保持打開的時長。 此延時可防止針頭在流體流動之前沿線段發生移動。
	//2. 點膠機開啟前， 馬達離開直線線段起始點的移動距離。 該距離為馬達提供了足夠的起速時間，主要用來消除過量流體在線段起始處的積聚。
	//3.點膠機關閉後，為了讓壓力在針頭移至下一點前變得均衡而在線段點膠結束點處產生的延時。
	//5.為防止過量流體在線段結束點處發生堆積，點膠機在距離線段結束點前多遠處關閉。
	//6.點膠機在線段結束點處停止後保持開啟的時長。
	*/
#ifdef MOVE
	LONG lNowX = 0, lNowY = 0, lNowZ = 0;
	lNowX = MO_ReadLogicPosition(0);
	lNowY = MO_ReadLogicPosition(1);
	lNowZ = MO_ReadLogicPosition(2);
	if (lStartDelayTime > 0 && lStartDistance > 0)
	{
		lStartDistance = 0;
	}
	if (lCloseDistance > 0 && lCloseONDelayTime > 0)
	{
		lCloseONDelayTime = 0;
	}


	if (!bZDisType) //絕對位置
	{
		if (lZBackDistance > lZ1)
		{
			lZBackDistance = lZ1;
		}
		lZBackDistance = abs(lZBackDistance - lZ1);
	}
	if (lZBackDistance > lZ1)
	{
		lZBackDistance = lZ1;
	}
	if (lZ1 == lZ2)
	{
		switch (iType)
		{
		case 0:
		{
			break;
		}
		case 1:
		{
			AttachFillType1(lX1, lY1, lX2, lY2, lZ1, lZBackDistance, lWidth, lStartDelayTime, lStartDistance, lCloseOffDelayTime,
				lCloseDistance, lCloseONDelayTime, lWorkVelociy, lAcceleration, lInitVelociy);
			break;
		}
		case 2:
		{
			AttachFillType2(lX1, lY1, lX2, lY2, lZ1, lZBackDistance, lWidth, lStartDelayTime, lStartDistance, lCloseOffDelayTime,
				lCloseDistance, lCloseONDelayTime, lWorkVelociy, lAcceleration, lInitVelociy);
			break;
		}
		case 3:
		{
			AttachFillType3(lX1, lY1, lX2, lY2, lZ1, lZBackDistance, lWidth, lStartDelayTime, lStartDistance, lCloseOffDelayTime,
				lCloseDistance, lCloseONDelayTime, lWorkVelociy, lAcceleration, lInitVelociy);
			break;
		}
		case 4:
		{
			AttachFillType4(lX1, lY1, lX2, lY2, lZ1, lZBackDistance, lWidth, lWidth2, lStartDelayTime, lStartDistance,
				lCloseOffDelayTime, lCloseDistance, lCloseONDelayTime, lWorkVelociy, lAcceleration, lInitVelociy);
			break;
		}
		case 5:
		{
			AttachFillType5(lX1, lY1, lX2, lY2, lZ1, lZBackDistance, lWidth, lWidth2, lStartDelayTime, lStartDistance,
				lCloseOffDelayTime, lCloseDistance, lCloseONDelayTime, lWorkVelociy, lAcceleration, lInitVelociy);
			break;
		}
		case 6:
		{
			AttachFillType6(lX1, lY1, lX2, lY2, lZ1, lZBackDistance, lWidth, lStartDelayTime, lStartDistance, lCloseOffDelayTime,
				lCloseDistance, lCloseONDelayTime, lWorkVelociy, lAcceleration, lInitVelociy);
			break;
		}
		case 7:
		{
			AttachFillType7(lX1, lY1, lX2, lY2, lZ1, lZBackDistance, lWidth, lStartDelayTime, lStartDistance, lCloseOffDelayTime,
				lCloseDistance, lCloseONDelayTime, lWorkVelociy, lAcceleration, lInitVelociy);
			break;
		}
		default:
			break;
		}
	}
	else
	{
		AfxMessageBox(L"Z軸高度不同，請修正");
	}
#endif
}
/*
*輸出
*寫入指定輸出埠，並且開啟或關閉(共16個輸出)
*iPort 選擇埠(可以輸入0~15)
*bChoose 開啟關閉
*return BOOL(1開啟 0關閉)
*/
BOOL CAction::DecideOutPutSign(int iPort, BOOL bChoose)
{
#ifdef MOVE
	MO_SetPIOOutput(iPort, bChoose);
#endif
	return bChoose;
}
/*
*輸入
*讀取指定輸入埠，並且開啟或關閉(共12個輸入)
*iPort 選擇埠(可以輸入0~11)
*bChoose 開啟關閉
*return BOOL(1開啟 0關閉)
*/
BOOL CAction::DecideInPutSign(int iPort, BOOL bChoose)
{
#ifdef MOVE
	if (MO_ReadPIOInput(iPort) == bChoose)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
#endif 
#ifndef MOVE
	return bChoose;
#endif
}
/*
*位置偏移
*輸入一個(X,Y,Z)座標，會回傳扣掉現在位置的相對座標
*return CStirng(X,Y,Z)
*/
CString CAction::NowOffSet(LONG lX, LONG lY, LONG lZ)
{
#ifdef MOVE
	LONG lNowX = 0, lNowY = 0, lNowZ = 0;
	CString csBuff = 0;
	lNowX = MO_ReadLogicPosition(0);
	lNowY = MO_ReadLogicPosition(1);
	lNowZ = MO_ReadLogicPosition(2);
	csBuff.Format(_T("%ld,%ld,%ld"), (lNowX - lX), (lNowY - lY), (lNowZ - lZ));
	return csBuff;
#endif
#ifndef MOVE
	return NULL;
#endif
}
/*
*回傳目前機械手臂位置
*return CString(X,Y,Z)
*/
CString CAction::NowLocation()
{
	CString Location;
#ifdef MOVE
	Location.Format(_T("%d,%d,%d"), MO_ReadLogicPosition(0), MO_ReadLogicPosition(1), MO_ReadLogicPosition(2));
#endif
	return Location;
}
/*
*CCD移動動作
*輸入(目標點、系統參數)
*/
void CAction::DoCCDMove(LONG lX, LONG lY, LONG lZ, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
	/*目標點(x座標，y座標，z座標，)
	LONG lX, LONG lY, LONG lZ
	*/
	/*系統參數(驅動速度，加速度，初速度)
	LONG lWorkVelociy,LONG lAcceleration, LONG lInitVelociy
	*/
#ifdef MOVE
	if (!g_bIsStop)
	{
		MO_Do3DLineMove(lX - MO_ReadLogicPosition(0), lY - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration, lInitVelociy);//x,y軸移動
		PreventMoveError();//防止軸卡出錯
	}
	if (!g_bIsStop)
	{
		MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//z軸移動
		PreventMoveError();//防止軸卡出錯
	}
#endif
}
/*執行完畢回(0,0,0)位置*/
void CAction::BackGOZero(LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
#ifdef MOVE
	if (!g_bIsStop)
	{
		MO_Do3DLineMove(0, 0, 0 - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);
		PreventMoveError();
	}
	if (!g_bIsStop)
	{
		MO_Do3DLineMove(0 - MO_ReadLogicPosition(0), 0 - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration, lInitVelociy);
		PreventMoveError();
	}
#endif
}
//----------------------------------------------------
//人機使用API--使用前請使用原點復歸偏移量
//----------------------------------------------------
//軟體負極限(x,y,z預設為-10)
void CAction::HMNegLim(LONG lX, LONG lY, LONG lZ)
{
#ifdef MOVE
	MO_SetSoftLim(7, 1);
	MO_SetCompSoft(1, -lX, -lY, -lZ);
#endif
}
//軟體正極限(x,y,z)
void CAction::HMPosLim(LONG lX, LONG lY, LONG lZ)
{
#ifdef MOVE
	MO_SetSoftLim(7, 1);
	MO_SetCompSoft(0, lX, lY, lZ);
#endif
}
/***********************************************************
**                                                        **
**          運動模組-雷射3D任意路徑 (對應動作判斷)           **
**                                                        **
************************************************************/
/*
*雷射B點設定
*/
void CAction::LA_Butt_GoBPoint()
{
#ifdef LA
	LAS_GoBPoint(DATA_3ZERO_B);
#endif
}
/*
*雷射到B點
*/
void CAction::LA_Butt_GoLAtoBPoint()
{
#ifdef LA
	DATA_3ZERO_LA = LAS_GoLAtoBPoint(DATA_3ZERO_B);
	g_OffSetLaserX = DATA_3ZERO_LA.EndPX;
	g_OffSetLaserY = DATA_3ZERO_LA.EndPY;
	//TODO::雷射OFFSET修改在這
	//g_OffSetLaserX = 48436; 
	//g_OffSetLaserY = 0;

#endif
}
/*
*雷射初始化
*/
void CAction::LA_SetInit()
{
#ifdef LA
#pragma region Omron Laser

	if (m_hComm != NULL)
	{
		AfxMessageBox(L"Already Opened!");
		return;
	}

	m_hComm = CreateFile(ComportNo, GENERIC_READ | GENERIC_WRITE, 0, NULL,
		OPEN_EXISTING, false ? FILE_FLAG_OVERLAPPED : 0, NULL);
	if (m_hComm == INVALID_HANDLE_VALUE)
	{
		// MessageBox(L"Open failed");
		return;
	}
	LAS_SetInit(&m_hComm);

#pragma endregion //Omron
	////LAS_SetInit();
	////TODO::雷射OFFSET修改在這
	////*************目前寫死動作***************************************
	//g_OffSetLaserX = 40324;
	//g_OffSetLaserY = -647;
	//g_HeightLaserZero = 31079;//雷射用Z軸歸零點高度
	//g_OffSetLaserZ = 43274;//B點的Z高度到雷射歸零Z高度的位移值(+)
#endif
}
/*
*單點掃描
*/
BOOL CAction::LA_Dot3D(LONG lX, LONG lY, LONG &lZ, LONG lWorkVelociy,
	LONG lAcceleration, LONG lInitVelociy)
{
#ifdef LA
	LONG lCalcData1;
	if (!g_bIsStop)
	{
		MO_Do3DLineMove(0, 0, g_HeightLaserZero - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);
		PreventMoveError();//防止軸卡驅動錯誤
	}
	if (!g_bIsStop)
	{
		MO_Do3DLineMove(lX - g_OffSetLaserX - MO_ReadLogicPosition(0), lY - g_OffSetLaserY - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration, lInitVelociy);
		PreventMoveError();//防止軸卡驅動錯誤
	}
	if (LAS_GetLaserData(lCalcData1))
	{
		if (lCalcData1 == LAS_LONGMIN)
		{
			lZ = MO_ReadLogicPosition(2);
			return FALSE;//取不到值(ffffff)
		}
		else
		{
			lZ = MO_ReadLogicPosition(2) - lCalcData1 + g_OffSetLaserZ;//30000為感測範圍
			return TRUE;
		}
	}
	else
	{
		lZ = MO_ReadLogicPosition(2);
		return FALSE;//取不到值(ffffff)
	}
#endif
#ifndef LA 
	return FALSE;
#endif
}
/*
*雷射規零
*雷射使用前須先進行歸零與偏移植的設定。
*lZ的值為平台到雷射距離為65mm。
*/
BOOL CAction::LA_SetZero()
{
#ifdef LA
#ifdef MOVE
	if (LAS_SetZero())//執行歸零點
	{
		if (!g_bIsStop)
		{
			MO_Do3DLineMove(0, 0, -LAS_MEASURE_RANGE, 20000, 100000, 1000);//上升3cm再次做雷射歸零
			PreventMoveError();//防止軸卡出錯
		}
		Sleep(1000);
		if (LAS_SetZero())
		{
			g_HeightLaserZero = MO_ReadLogicPosition(2);//雷射用Z軸歸零點高度
			g_OffSetLaserZ = DATA_3ZERO_B.EndPZ - g_HeightLaserZero;//B點的Z高度到雷射歸零Z高度的位移值(+)
			return TRUE;//雷射歸零完成
		}
		else
		{
			AfxMessageBox(L"歸零高度不正確");
			return FALSE;
		}
	}
	else
	{
		AfxMessageBox(L"歸零錯誤");
		return FALSE;//z軸高度不正確雷射無法取得值
	}
#endif
#endif
#ifndef LA
	return FALSE;
#endif
#ifndef MOVE
	return FALSE;
#endif
}
/*兩軸連續插補*/
//以(lx3,ly3)為結束點
void CAction::LA_Do2DVetor(LONG lX3, LONG lY3, LONG lX2, LONG lY2, LONG lX1, LONG lY1)
{          
#ifdef MOVE
	BOOL bType = 0;
	DATA_2MOVE DATA_2D;
	if (lX1 == 0 && lY1 == 0 && lX2 == 0 && lY2 == 0)
	{
		//直線
		bType = 0;
		DATA_2D.CirCentP.x = 0;
		DATA_2D.CirCentP.y = 0;
		DATA_2D.CirRev = 0;
	}
	else
	{
		//圓弧
		bType = 1;
		CString csbuff;
		CString csP1, csP2, csP3;
		csP1.Format(L"%d，%d，", lX1, lY1);
		csP2.Format(L"%d，%d，", lX2, lY2);
		csP3.Format(L"%d，%d，", lX3, lY3);
		csbuff = TRoundCCalculation(csP1, csP3, csP2);
		DATA_2D.CirCentP.x = CStringToLong(csbuff, 0);
		DATA_2D.CirCentP.y = CStringToLong(csbuff, 1);
		DATA_2D.CirRev = CStringToLong(csbuff, 2);//取得圓心(X，Y，Rev，)
	}
	DATA_2D.EndP.x = lX3;
	DATA_2D.EndP.y = lY3;
	DATA_2D.Type = bType;
	DATA_2D.Speed = 0;
	LA_m_ptVec2D.push_back(DATA_2D);
#endif
}
/*兩軸線段點*/
void CAction::LA_Do2dDataLine(LONG EndPX, LONG EndPY)
{
#ifdef MOVE
	DATA_2MOVE DATA_2D;
	DATA_2D.EndP.x = EndPX;
	DATA_2D.EndP.y = EndPY;
	DATA_2D.Type = 0;//直線
	DATA_2D.Speed = 0;
	DATA_2D.CirCentP.x = 0;
	DATA_2D.CirCentP.y = 0;
	DATA_2D.CirRev = 0;
	LA_m_ptVec2D.push_back(DATA_2D);
#endif
}
/*兩軸線段圓弧*/
void CAction::LA_Do2dDataArc(LONG EndPX, LONG EndPY, LONG ArcX, LONG ArcY)
{
#ifdef MOVE
	if (LA_m_ptVec2D.empty())
	{
		return;
	}
	DATA_2MOVE DATA_2D;
	CString csbuff;
	CString csP1, csP2, csP3;

	csP1.Format(L"%d，%d，", LA_m_ptVec2D.back().EndP.x, LA_m_ptVec2D.back().EndP.y);
	csP2.Format(L"%d，%d，", ArcX, ArcY);
	csP3.Format(L"%d，%d，", EndPX, EndPY);
	csbuff = TRoundCCalculation(csP1, csP3, csP2);
	DATA_2D.Type = 1;//圓
	DATA_2D.EndP.x = EndPX;
	DATA_2D.EndP.y = EndPY;
	DATA_2D.CirCentP.x = CStringToLong(csbuff, 0);
	DATA_2D.CirCentP.y = CStringToLong(csbuff, 1);
	DATA_2D.CirRev = CStringToLong(csbuff, 2);//取得圓心(X，Y，Rev，)
	LA_m_ptVec2D.push_back(DATA_2D);
#endif
}
/*兩軸線段圓*/
void CAction::LA_Do2dDataCircle(LONG EndPX, LONG EndPY, LONG CirP1X, LONG CirP1Y, LONG CirP2X, LONG CirP2Y)
{
#ifdef MOVE
	if (LA_m_ptVec2D.empty())
	{
		return;
	}
	DATA_2MOVE DATA_2D;
	CString csbuff;
	CString csP1, csP2, csP3;

	csP1.Format(L"%d，%d，", EndPX, EndPY);
	csP2.Format(L"%d，%d，", CirP1X, CirP1Y);
	csP3.Format(L"%d，%d，", CirP2X, CirP2Y);
	csbuff = TRoundCCalculation(csP1, csP3, csP2);
	DATA_2D.Type = 1;//圓
	DATA_2D.EndP.x = EndPX;
	DATA_2D.EndP.y = EndPY;
	DATA_2D.CirCentP.x = CStringToLong(csbuff, 0);
	DATA_2D.CirCentP.y = CStringToLong(csbuff, 1);
	DATA_2D.CirRev = CStringToLong(csbuff, 2);//取得圓心(X，Y，Rev，)
	LA_m_ptVec2D.push_back(DATA_2D);
#endif
}
/*
*連續線段取值
*lStartVe, lStartAcc, lStartInitVe  移動到掃描啟始點的驅動速度、加速度、初速度
*lWorkVelociy, lAcceleration, lInitVelociy  掃描的驅動速度、加速度、初速度
*/
void CAction::LA_Line2D(LONG lStartVe, LONG lStartAcc, LONG lStartInitVe, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
#ifdef LA
#ifdef MOVE
	if (g_LaserAverage == FALSE)
	{
		g_LaserCnt++;
	}
	LA_AbsToOppo2Move(LA_m_ptVec2D);
	if (!g_bIsStop)
	{
		MO_Do3DLineMove(0, 0, g_HeightLaserZero - MO_ReadLogicPosition(2), lStartVe, lStartAcc, lStartInitVe);
		PreventMoveError();//起始點準備移動
	}
	if (!g_bIsStop)
	{
		MO_Do3DLineMove(LA_m_ptVec2D.at(0).EndP.x - g_OffSetLaserX,
			LA_m_ptVec2D.at(0).EndP.y - g_OffSetLaserY, 0, lStartVe, lStartAcc, lStartInitVe);
		PreventMoveError();//起始點準備移動
	}

	/*停止觸發時，結束掃描*/
	if (g_bIsStop)
	{
		return;
	}

	MO_InterruptCase(1, 2);
	MO_InterruptCase(1, 3);
	MO_Timer(0, 100000);//設定計時器(0.1s觸發一次)
						/*插入開始點一筆*/
	LONG lCalcData1;

	if (LAS_GetLaserData(lCalcData1))
	{
		if (lCalcData1 == LAS_LONGMIN)
		{
			g_LaserErrCnt++;
		}
		else
		{
			DATA_3Do[0].EndPX = MO_ReadLogicPosition(0) + g_OffSetLaserX;
			DATA_3Do[0].EndPY = MO_ReadLogicPosition(1) + g_OffSetLaserY;
			DATA_3Do[0].EndPZ = MO_ReadLogicPosition(2) - lCalcData1 + g_OffSetLaserZ;//30000為感測範圍
			if (g_LaserAverage == FALSE)
			{
				LA_m_ptVec.push_back(DATA_3Do[0]);
			}
			if (g_LaserCnt == 1 && g_LaserAverage == FALSE)
			{
				LA_m_iVecSP.push_back(g_LaserCnt);//main
			}
		}
	}
	for (UINT i = 1; i < LA_m_ptVec2D.size(); i++)
	{
		DATA_2Do[i - 1] = LA_m_ptVec2D.at(i);
	}
	MO_DO2Curve(DATA_2Do, LA_m_ptVec2D.size() - 1, lWorkVelociy);
	PreventMoveError();
	Sleep(200);
	LA_m_ptVec2D.clear();
#endif
#endif
}
/*
連續線段動作--(三軸連續插補)
*/
void CAction::LA_Line3DtoDo(int iData, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy, BOOL bDoAll)
{
#ifdef MOVE
	/*停止觸發時，結束掃描*/
	if (g_bIsStop)
	{
		return;
	}

	if (iData <= 0)
	{
		return;
	}

	while (g_bIsGetLAend == FALSE);
	std::vector<DATA_3MOVE>::iterator LA_ptIter;//迭代器
	std::vector<DATA_3MOVE>LA_Buff;//線段值判斷
	LONG lNowZHigh = 0;
	LA_Buff.clear();
	if (bDoAll)
	{
		LA_Buff.assign(LA_m_ptVec.begin(), LA_m_ptVec.begin() + LA_m_iVecSP.at(1) - 1);
		lNowZHigh = g_HeightLaserZero - MO_ReadLogicPosition(2);//相對位置
	}
	else
	{
		if ((UINT)iData > LA_m_iVecSP.size())
		{
			return;
		}
		else if (iData == 1)
		{
			LA_Buff.assign(LA_m_ptVec.begin(), LA_m_ptVec.begin() + LA_m_iVecSP.at(1) - 1);
			lNowZHigh = g_HeightLaserZero - MO_ReadLogicPosition(2);//相對位置
		}
		else
		{
			LA_Buff.assign(LA_m_ptVec.begin() + LA_m_iVecSP.at(iData - 1), LA_m_ptVec.begin() + LA_m_iVecSP.at(iData) - 1);
			lNowZHigh = 0;//相對位置
		}
	}
	if (!g_bIsStop)
	{
		MO_Do3DLineMove(0, 0, lNowZHigh, lWorkVelociy, lAcceleration,lInitVelociy);//掃描高度回去
		PreventMoveError();//起始點準備移動
	}
	LA_AbsToOppo3Move(LA_Buff);
	if (!g_bIsStop)
	{
		LA_ptIter = LA_Buff.begin();
		MO_Do3DLineMove(LA_ptIter->EndPX, LA_ptIter->EndPY, 0, lWorkVelociy,lAcceleration, lInitVelociy);//移動起始點
		PreventMoveError();//防止驅動錯誤
	}
	if (!g_bIsStop)
	{
		LA_ptIter = LA_Buff.begin();
		MO_Do3DLineMove(0, 0, LA_ptIter->EndPZ, lWorkVelociy, lAcceleration,lInitVelociy);//z軸下降到起始點高度
		PreventMoveError();//防止驅動錯誤
	}
	if (!g_bIsStop)
	{
		if (!g_bIsStop && g_bIsDispend == 1)
		{
			MO_GummingSet(1, 0);//塗膠(不卡)
		}
		MO_DO3Curve(DATA_3Do, LA_Buff.size() - 1, lWorkVelociy);//連續插補開始
		PreventMoveError();//防止驅動錯誤
	}
#endif
}
/*
雷射清除指令
*/
void CAction::LA_Clear()
{
#ifdef MOVE
	LA_m_ptVec.clear();//雷射連續切點儲存vector
	LA_m_ptVec2D.clear();//兩軸連續插補vector
	LA_m_iVecSP.clear();//主要雷射vector(SP:Scan End)
	g_LaserCnt = 0; //雷射線段計數器(掃描用)
#endif
}

/*
*雷射平均高度
*座標(lStrX, lStrY)掃描至(lEndX, lEndY)
*&lZ    掃描執行完後寫入Z的平均高度值
*lStartVe, lStartAcc, lStartInitVe  移動到掃描啟始點的驅動速度、加速度、初速度
*lWorkVelociy, lAcceleration, lInitVelociy  掃描的驅動速度、加速度、初速度
*/
void CAction::LA_AverageZ(LONG lStrX, LONG lStrY, LONG lEndX, LONG lEndY, LONG &lZ, LONG lStartVe, LONG lStartAcc, LONG lStartInitVe, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
#ifdef LA
#ifdef MOVE
	g_LaserAverage = TRUE;
	g_LaserAveBuffZ = 0;
	LA_Do2dDataLine(lStrX, lStrY);
	LA_Do2dDataLine(lEndX, lEndY);
	LA_Line2D(lStartVe, lStartAcc, lStartInitVe, lWorkVelociy, lAcceleration, lInitVelociy);
	Sleep(200);
	lZ = g_LaserAveBuffZ;
	g_LaserAverage = FALSE;
#endif
#endif
}
//修正加執行連續線段
void CAction::LA_CorrectVectorToDo(LONG  lWorkVelociy, LONG lAcceleration, LONG lInitVelociy, LONG RefX, LONG RefY, DOUBLE OffSetX, DOUBLE OffSetY, DOUBLE Andgle, DOUBLE CameraToTipOffsetX, DOUBLE CameraToTipOffsetY, BOOL Mode, LONG lSubOffsetX, LONG lSubOffsetY)
{
#ifdef MOVE
	std::vector<DATA_3MOVE>::iterator LA_ptIter;//迭代器
	std::vector<DATA_3MOVE>LA_Buff;//線段值判斷
	LONG lNowZHigh = 0;
	g_LaserCnt = 1;
	//while (g_bIsGetLAend == FALSE);
	LA_Buff.clear();

	LA_Buff.assign(LA_m_ptVec.begin(), LA_m_ptVec.begin() + LA_m_iVecSP.at(1) - 1);
	lNowZHigh = g_HeightLaserZero - MO_ReadLogicPosition(2);//相對位置
	for (UINT i = 0; i < LA_Buff.size(); i++)
	{
		if (!(LA_Buff.at(i).EndPX == LA_SCANEND && LA_m_ptVec.at(i).EndPY == LA_SCANEND))
		{
			LA_CorrectLocation(LA_Buff.at(i).EndPX, LA_Buff.at(i).EndPY, RefX, RefY, OffSetX, OffSetY, Andgle, CameraToTipOffsetX, CameraToTipOffsetY, Mode, lSubOffsetX, lSubOffsetY);
		}
	}
	//將值在塞回LA_m_ptVec
	if (!g_bIsStop)
	{
		MO_Do3DLineMove(0, 0, lNowZHigh, lWorkVelociy, lAcceleration, lInitVelociy);//掃描高度回去
		PreventMoveError();//起始點準備移動
	}

	LA_AbsToOppo3Move(LA_Buff);

	if (!g_bIsStop)
	{
		LA_ptIter = LA_Buff.begin();
		MO_Do3DLineMove(LA_ptIter->EndPX, LA_ptIter->EndPY, 0, lWorkVelociy, lAcceleration, lInitVelociy);//移動起始點
		PreventMoveError();//防止驅動錯誤
	}
	if (!g_bIsStop)
	{
		LA_ptIter = LA_Buff.begin();
		MO_Do3DLineMove(0, 0, LA_ptIter->EndPZ, lWorkVelociy, lAcceleration, lInitVelociy);//z軸下降到起始點高度
		PreventMoveError();//防止驅動錯誤
	}
	if (!g_bIsStop)
	{
		if (!g_bIsStop && g_bIsDispend == 1)
		{
			MO_GummingSet(1, 0);//塗膠(不卡)
		}
		MO_DO3Curve(DATA_3Do, LA_Buff.size() - 1, lWorkVelociy);//連續插補開始
		PreventMoveError();//防止驅動錯誤
	}
#endif
}
//填充選擇拿取最後一點座標(EndX,EndY)
void CAction::Fill_EndPoint(LONG & lEndX, LONG & lEndY, LONG lX1, LONG lY1, LONG lZ1, LONG lX2, LONG lY2, LONG lZ2, int iType, LONG lWidth, LONG lWidth2)
{
	/*線段開始(x座標，y座標，z座標，線段起始點，)
	LONG lX1, LONG lY1, LONG lZ1
	*/
	/*線段結束(x座標，y座標，z座標，線段結束點，)
	LONG lX2, LONG lY2, LONG lZ2
	*/
	/*填充命令(填充形式(1~7)，寬度(mm)，兩端寬度(mm)，)
	int iType, LONG lWidth, LONG lWidth2
	*/
	LONG lBufX = 0, lBufY = 0;
	if (lZ1 == lZ2)
	{
		switch (iType)
		{
		case 0:
		{
			break;
		}
		case 1:
		{
			lEndX = lX2;
			lEndY = lY2;
			break;
		}
		case 2:
		{
			lEndX = lX2;
			lEndY = lY2;
			break;
		}
		case 3:
		{
			AttachFillType3_End(lBufX, lBufY, lX1, lY1, lX2, lY2, lWidth, lWidth2);
			lEndX = lBufX;
			lEndY = lBufY;
			break;
		}
		case 4:
		{
			AttachFillType4_End(lBufX, lBufY, lX1, lY1, lX2, lY2, lWidth, lWidth2);
			lEndX = lBufX;
			lEndY = lBufY;
			break;
		}
		case 5:
		{
			AttachFillType5_End(lBufX, lBufY, lX1, lY1, lX2, lY2, lWidth, lWidth2);
			lEndX = lBufX;
			lEndY = lBufY;
			break;
		}
		case 6:
		{
			lEndX = lX1;
			lEndY = lY1;
			break;
		}
		case 7:
		{
			lEndX = lX1;
			lEndY = lY1;
			break;
		}
		default:
			break;
		}
	}
	else
	{
		AfxMessageBox(L"Z軸高度不同，請修正");
	}
}

/*********************************************************執行緒************************************************************/
//3d任意路徑執行緒
DWORD CAction::MoInterrupt(LPVOID param)
{
#ifdef MOVE
#ifdef PRINTF
	_cwprintf(L"%s\n", L"Interrupt");
#endif
	LONG RR1X, RR1Y, RR1Z, RR1U;
	MO_ReadEvent(&RR1X, &RR1Y, &RR1Z, &RR1U);
	if (RR1X & 0x0010)//原本為RR1X&0x0020 驅動開始中斷
	{
		g_LaserErrCnt = 0;//計數初始化
		g_LaserNuCnt = 1;//計數初始化
		//MO_Timer(0, 100000);//設定計時器(0.1s觸發一次)
		MO_Timer(1, 100000);//啟動計時器
		((CAction *)param)->g_getHeightFlag = TRUE;//雷射測高旗標：允許測高
		((CAction *)param)->g_bIsGetLAend = FALSE;//掃描尚未完成
	}
	if ((RR1X & 0x0080) || (g_LaserErrCnt == 10))//原本的為((RR1X&0x0040)|| (g_LaserErrCnt == 10)) 驅動結束中斷
	{
#ifdef LA
#ifdef PRINTF
		_cwprintf(L"%s\n", L"驅動結束中斷!");
#endif
		((CAction *)param)->g_getHeightFlag = FALSE;
		//((CAction *)param)->g_bIsGetLAend = TRUE;
		if ((g_LaserErrCnt >= 10))
		{
			AfxMessageBox(L"雷射點請修正");
			return 0;
		}
		/*插入結束點一筆*/
		LONG lCalcData1;
		if (LAS_GetLaserData(lCalcData1))
		{
			if (lCalcData1 == LAS_LONGMIN)
			{
				g_LaserErrCnt++;
			}
			else//雷射取到的值為(0~+30)
			{
				((CAction *)param)->DATA_3Do[g_LaserNuCnt].EndPX = MO_ReadLogicPosition(0) + ((CAction *)param)->g_OffSetLaserX;
				((CAction *)param)->DATA_3Do[g_LaserNuCnt].EndPY = MO_ReadLogicPosition(1) + ((CAction *)param)->g_OffSetLaserY;
				((CAction *)param)->DATA_3Do[g_LaserNuCnt].EndPZ = MO_ReadLogicPosition(2) - lCalcData1 + ((CAction *)param)->g_OffSetLaserZ;//30000為感測範圍
				if (((CAction *)param)->g_LaserAverage == TRUE)
				{
					((CAction *)param)->g_LaserAveBuffZ += ((CAction *)param)->DATA_3Do[g_LaserNuCnt].EndPZ;
				}
				else
				{
					((CAction *)param)->LA_m_ptVec.push_back(((CAction *)param)->DATA_3Do[g_LaserNuCnt]);
				}
				g_LaserNuCnt++;
			}
		}

		((CAction *)param)->DATA_3Do[g_LaserNuCnt].EndPX = LA_SCANEND;//-99999為線段結束
		((CAction *)param)->DATA_3Do[g_LaserNuCnt].EndPY = LA_SCANEND;//-99999為線段結束
		((CAction *)param)->DATA_3Do[g_LaserNuCnt].EndPZ = LA_SCANEND;//-99999為線段結束

		if (((CAction *)param)->g_LaserAverage == TRUE)
		{
			((CAction *)param)->g_LaserAveBuffZ = LONG(round(((DOUBLE)((CAction *)param)->g_LaserAveBuffZ) / (g_LaserNuCnt-1)));
		}
		else
		{
			((CAction *)param)->LA_m_ptVec.push_back(((CAction *)param)->DATA_3Do[g_LaserNuCnt]);
			((CAction *)param)->LA_m_iVecSP.push_back(((CAction *)param)->LA_m_ptVec.size());
		}
#endif
		MO_Timer(2, 1000000);//關閉計時器
		g_LaserErrCnt = 0;//測高錯誤計數器歸零
		g_LaserNuCnt = 1;//計數初始化
		MO_InterruptCase(0, 2);//關閉中斷
		MO_InterruptCase(0, 3);//關閉中斷 
		((CAction *)param)->g_bIsGetLAend = TRUE;// 掃描完成
	}
	if (RR1Y & 0x0200) //Y計時器中斷(出膠與斷膠)
	{
#ifdef PRINTF
		_cwprintf(L"y\n");
#endif
		if (!((CAction*)param)->g_bIsStop)
		{
			(CAction::g_YtimeOutGlueSet) ? MO_GummingSet() : MO_FinishGumming();
			CAction::g_YtimeOutGlueSet = FALSE;
		}			
	}
	if (RR1Z & 0x0200) //Z計時器中斷
	{
#ifdef PRINTF
		_cwprintf(L"z\n");
#endif
		if (!((CAction*)param)->g_bIsStop)
		{
			(CAction::g_ZtimeOutGlueSet) ? MO_GummingSet() : MO_FinishGumming();
			CAction::g_ZtimeOutGlueSet = FALSE;
		} 
	}
	if (RR1U & 0x0200 && ((CAction *)param)->g_getHeightFlag) //U計時器中斷
	{
#ifdef LA
		LONG lCalcData1;
		if (LAS_GetLaserData(lCalcData1))
		{
			if (lCalcData1 == LAS_LONGMIN)
			{
				g_LaserErrCnt++;
			}
			else
			{
				((CAction *)param)->DATA_3Do[g_LaserNuCnt].EndPX = MO_ReadLogicPosition(0) + ((CAction *)param)->g_OffSetLaserX - ((CAction *)param)->g_OffSetScan;
				((CAction *)param)->DATA_3Do[g_LaserNuCnt].EndPY = MO_ReadLogicPosition(1) + ((CAction *)param)->g_OffSetLaserY - ((CAction *)param)->g_OffSetScan;
				((CAction *)param)->DATA_3Do[g_LaserNuCnt].EndPZ = MO_ReadLogicPosition(2) - lCalcData1 + ((CAction *)param)->g_OffSetLaserZ;//30000為感測範圍
				if (((CAction *)param)->g_LaserAverage == TRUE)
				{
					((CAction *)param)->g_LaserAveBuffZ += ((CAction *)param)->DATA_3Do[g_LaserNuCnt].EndPZ;
				}
				else
				{
					((CAction *)param)->LA_m_ptVec.push_back(((CAction *)param)->DATA_3Do[g_LaserNuCnt]);
#ifdef PRINTF
					_cwprintf(L"%s\n", L"Get Point");
#endif
				}
				g_LaserNuCnt++;
			}
		}

#endif
	}
	return 0;
#endif
#ifndef MOVE
	return 0;
#endif
}
/******************************************************自行運用函數**********************************************************/
/*
*附屬---移動點動作
*輸入(點、驅動速度、加速度、初速度、是否使用中斷)
*/
void CAction::AttachPointMove(LONG lX, LONG lY, LONG lZ, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy, BOOL bIntt)
{
#ifdef MOVE
	LONG lNowX = 0, lNowY = 0, lNowZ = 0;
	lNowX = MO_ReadLogicPosition(0);
	lNowY = MO_ReadLogicPosition(1);
	lNowZ = MO_ReadLogicPosition(2);
	if (lX == 0)
	{
		lX = lNowX;
	}
	if (lY == 0)
	{
		lY = lNowY;
	}
	if (lZ == 0)
	{
		lZ = lNowZ;
	}
	if (bIntt == 1)
	{
		MO_Do3DLineMoveInterr(lX - lNowX, lY - lNowY, lZ - lNowZ, lWorkVelociy,
			lAcceleration, lInitVelociy, MoInterrupt);//執行執行緒中斷
		PreventMoveError();//防止軸卡出錯
	}
	else
	{
		if (!g_bIsStop)
		{
			MO_Do3DLineMove(lX - lNowX, lY - lNowY, lZ - lNowZ, lWorkVelociy, lAcceleration,
				lInitVelociy);
			PreventMoveError();//防止軸卡出錯
		}
	}
#endif 
}
/*
*出膠(配合執行緒使用)
*輸入(點膠機開啟or點膠機關閉)解譯後的LONG值
*/
void CAction::DoGlue(LONG lTime, LONG lDelayTime, LPTHREAD_START_ROUTINE MoInterrupt)
{
#ifdef MOVE
	if (!g_bIsStop)
	{
		if (!g_bIsStop && g_bIsDispend == 1)
		{
			MO_GummingSet(g_iNumberGluePort, lTime * 1000);
		}
		PreventGlueError();//防止出膠出錯
	}
	if (!g_bIsStop)
	{
		// lTime / 1000 =(us→ms)
		MO_Timer(0, 0, lDelayTime * 1000);
		MO_Timer(1, 0, 0);//(ms)
		Sleep(1);//防止出錯，避免計時器初直為0
		while (MO_Timer(3, 0, 0))
		{
			if (g_bIsStop == 1)
			{
				MO_Timer(2, 0, 0);//停止計時器
				break;
			}
			Sleep(1);
		}
	}
#endif
}


/*
*防止軸卡出錯
*讀取各軸驅動狀態，當動作時停止。
*/
void CAction::PreventMoveError()
{
#ifdef MOVE
	while (MO_ReadIsDriving(7) && !g_bIsStop)
	{
		Sleep(1);
	}
#endif
}
/*
*防止出膠出錯
*讀取出膠狀態，當動作時停止。
*/
void CAction::PreventGlueError()
{
#ifdef MOVE
	while (MO_ReadGumming() && !g_bIsStop)
	{
		Sleep(1);
	}
	MO_StopGumming();//停止出膠
#endif
}
/*
*暫停回復後繼續塗膠
*讀取暫停參數，當參數為0時出膠，且點膠機要為開。
*/
void CAction::PauseDoGlue()
{
#ifdef MOVE
	if (!g_bIsPause)  //暫停恢復後繼續出膠(g_bIsPause=0)
	{
		if ((g_bIsPause == 0) && (g_bIsDispend == 1))
		{
			MO_GummingSet(g_iNumberGluePort, 0);//塗膠(不卡)
		}
	}
#endif
}
/*
*暫停時停指塗膠
*讀取暫停參數，當參數為1時斷膠，或點膠機關參數為0也斷膠。
*/
void CAction::PauseStopGlue()
{
#ifdef MOVE
	if ((g_bIsPause == 1) || (g_bIsDispend == 0)) //暫停時停指塗膠(g_bIsPause=1)
	{
		MO_StopGumming();//停止出膠
	}
#endif
}
/*
*返回設置
*iType 返回型態,lXarEnd X軸結束點座標(絕對),lYarEnd Y軸結束點座標(絕對),lZarEnd Z軸結束點座標(絕對),
*lXarUp X軸上一點座標(絕對),lYarUp Y軸上一點座標(絕對),lLineStop 回程距離(相對),lStopZar回程距離(相對),lBackZar回升距離(相對),
*lLowSpeed低回程速度,lHighSpeed高回程速度,lAcceleration加速度,lInitSpeed初速度
*/
void CAction::GelatinizeBack(int iType, LONG lXarEnd, LONG lYarEnd, LONG lZarEnd,
	LONG lXarUp, LONG lYarUp, LONG lLineStop, LONG lStopZar, LONG lBackZar,
	LONG lLowSpeed, LONG lHighSpeed, LONG lAcceleration, LONG lInitSpeed)
{
	/*線段結束點(x座標，y座標，z座標，)
	LONG lXarEnd, LONG lYarEnd, LONG lZarEnd
	*/
	/*線段開始(x座標，y座標，)
	LONG lXarUp, LONG lYarUp
	/*
	/*返回設定(返回長度，z返回高度，返回速度，類型，高速度)
	LONG lLineStop ,LONG lStopZar ,LONG lLowSpeed,int iType,lHighSpeed
	*/
	/*Z軸工作高度設定-Z軸回升高度(相對)最高點
	LONG lBackZar (使用相對 所以型態必為,1)
	*/
	/*系統參數(加速度，初速度)
	LONG lAcceleration, LONG lInitSpeed
	*/
#ifdef MOVE
	LONG lBackXar, lBackYar, lR; //lBackXar x軸移動座標(絕對), lBackYar Y軸移動座標(絕對)
	LONG lNowX = 0, lNowY = 0, lNowZ = 0;
	lNowX = MO_ReadLogicPosition(0);
	lNowY = MO_ReadLogicPosition(1);
	lR = LONG(sqrt(pow((DOUBLE)(lXarEnd - lXarUp), 2) + pow((DOUBLE)(lYarEnd - lYarUp), 2)));
	if (lLineStop>lR)
	{
		lBackXar = lXarUp;
		lBackYar = lYarUp;
	}
	else if (lXarEnd == 0 && lYarEnd == 0 && lXarUp == 0 && lYarUp == 0)
	{
		lBackXar = lNowX;
		lBackYar = lNowY;
	}
	else
	{
		LONG lXClose, lYClose, lLineClose;
		lLineClose = lLineStop;
		LineGetToPoint(lXClose, lYClose, lXarUp, lYarUp, lXarEnd, lYarEnd, lLineClose);
		lBackXar = lXClose;
		lBackYar = lYClose;//返回長度座標
	}
	if (lStopZar>lZarEnd)
	{
		lStopZar = lZarEnd;
	}            
	if (lStopZar>lBackZar)
	{
		lStopZar = lBackZar;
	}
	if (lBackZar>lZarEnd)
	{
		lBackZar = lZarEnd;
	}
	switch (iType) //0~5
	{
	case 0://無動作
	{
		break;
	}
	case 1://z軸上升兩段速
	{
		if (lStopZar == 0)
		{
			if (!g_bIsStop)
			{
				MO_Do3DLineMove(0, 0, (lZarEnd - lBackZar) - lZarEnd, lHighSpeed, lAcceleration, lInitSpeed);//沒有返回長度以高速返回最高點
				PreventMoveError();//防止軸卡出錯
			}
		}
		else
		{
			if (!g_bIsStop)
			{
				MO_Do3DLineMove(0, 0, (lZarEnd - lStopZar) - lZarEnd, lLowSpeed, lAcceleration, lInitSpeed);//低速
				PreventMoveError();//防止軸卡出錯
			}
			if (!g_bIsStop)
			{
				MO_Do3DLineMove(0, 0, (lZarEnd - lBackZar) - (lZarEnd - lStopZar), lHighSpeed, lAcceleration, lInitSpeed);//高速
				PreventMoveError();//防止軸卡出錯
			}
		}
		break;
	}
	case 2: //向後三軸插補(低速)，再z軸上升(高速)
	{
		if (lStopZar == 0)
		{
			if (!g_bIsStop)
			{
				MO_Do3DLineMove(lBackXar - lNowX, lBackYar - lNowY, 0, lLowSpeed, lAcceleration, lInitSpeed);//低速
				PreventMoveError();//防止軸卡出錯

			}
			if (!g_bIsStop)
			{
				MO_Do3DLineMove(0, 0, (lZarEnd - lBackZar) - lZarEnd, lHighSpeed, lAcceleration, lInitSpeed);//高速
				PreventMoveError();//防止軸卡出錯
			}
		}
		else
		{
			if (!g_bIsStop)
			{
				MO_Do3DLineMove(lBackXar - lNowX, lBackYar - lNowY, (lZarEnd - lStopZar) - lZarEnd, lLowSpeed, lAcceleration, lInitSpeed);//低速
				PreventMoveError();//防止軸卡出錯
			}
			if (!g_bIsStop)
			{
				MO_Do3DLineMove(0, 0, (lZarEnd - lBackZar) - (lZarEnd - lStopZar), lHighSpeed, lAcceleration, lInitSpeed);//高速
				PreventMoveError();//防止軸卡出錯
			}
		}

		break;
	}
	case 3://直角向後，z軸先上升(低速)，xy兩軸插補向後移動(高速)，再z軸上升(高速)
	{
		if (lStopZar != 0)
		{
			if (!g_bIsStop)
			{
				MO_Do3DLineMove(0, 0, (lZarEnd - lStopZar) - lZarEnd, lLowSpeed, lAcceleration, lInitSpeed);//低速z軸上升
				PreventMoveError();//防止軸卡出錯
			}
		}
		if (!g_bIsStop)
		{
			MO_Do3DLineMove(lBackXar - lNowX, lBackYar - lNowY, 0, lHighSpeed, lAcceleration, lInitSpeed);//向後移動高速
			PreventMoveError();//防止軸卡出錯
		}
		if (!g_bIsStop)
		{
			MO_Do3DLineMove(0, 0, (lZarEnd - lBackZar) - (lZarEnd - lStopZar), lHighSpeed, lAcceleration, lInitSpeed);//向後移動高速
			PreventMoveError();//防止軸卡出錯
		}
		break;
	}
	case 4://向前三軸插補(低速)，再z軸上升(高速)
	{
		lBackXar = (lXarEnd - lBackXar) + lXarEnd;
		lBackYar = (lYarEnd - lBackYar) + lYarEnd;
		if (lStopZar == 0)
		{
			if (!g_bIsStop)
			{
				MO_Do3DLineMove(lBackXar - lNowX, lBackYar - lNowY, 0, lLowSpeed, lAcceleration, lInitSpeed);//低速
				PreventMoveError();//防止軸卡出錯
			}
			if (!g_bIsStop)
			{
				MO_Do3DLineMove(0, 0, (lZarEnd - lBackZar) - lZarEnd, lLowSpeed, lAcceleration, lInitSpeed);//高速
				PreventMoveError();//防止軸卡出錯
			}
		}
		else
		{
			if (!g_bIsStop)
			{
				MO_Do3DLineMove(lBackXar - lNowX, lBackYar - lNowY, (lZarEnd - lStopZar) - lZarEnd, lLowSpeed, lAcceleration, lInitSpeed);//低速
				PreventMoveError();//防止軸卡出錯
			}
			if (!g_bIsStop)
			{
				MO_Do3DLineMove(0, 0, (lZarEnd - lBackZar) - (lZarEnd - lStopZar), lHighSpeed, lAcceleration, lInitSpeed);//高速
				PreventMoveError();//防止軸卡出錯
			}
		}
		break;
	}
	case 5: //直角向前，z軸先上升(低速)，xy兩軸插補向前移動(高速)，再z軸上升(高速)
	{
		lBackXar = (lXarEnd - lBackXar) + lXarEnd;
		lBackYar = (lYarEnd - lBackYar) + lYarEnd;

		if (lStopZar != 0)
		{
			if (!g_bIsStop)
			{
				MO_Do3DLineMove(0, 0, (lZarEnd - lStopZar) - lZarEnd, lLowSpeed, lAcceleration, lInitSpeed);//z軸低速上升
				PreventMoveError();//防止軸卡出錯
			}
		}
		if (!g_bIsStop)
		{
			MO_Do3DLineMove(lBackXar - lNowX, lBackYar - lNowY, 0, lHighSpeed, lAcceleration, lInitSpeed);//兩軸向前移動(高速)
			PreventMoveError();//防止軸卡出錯
		}
		if (!g_bIsStop)
		{
			MO_Do3DLineMove(0, 0, (lZarEnd - lBackZar) - (lZarEnd - lStopZar), lHighSpeed, lAcceleration, lInitSpeed);//高速z軸上升
			PreventMoveError();//防止軸卡出錯
		}
		break;
	}
	default:
	{
		break;
	}
	}
#endif
}
/*
*直線距離轉換成座標點
*/
void CAction::LineGetToPoint(LONG &lXClose, LONG &lYClose, LONG lX0, LONG lY0, LONG lX1, LONG lY1, LONG &lLineClose)
{
	DOUBLE dX = lX0 - lX1;
	DOUBLE dY = lY0 - lY1;
	DOUBLE D = sqrt(pow(dX, 2) + pow(dY, 2));
	DOUBLE dXc = dX*(DOUBLE)lLineClose / D;
	DOUBLE dYc = dY*(DOUBLE)lLineClose / D;
	lXClose = lX1 + (LONG)round(dXc);
	lYClose = lY1 + (LONG)round(dYc);
	//DOUBLE dM, dB; //(X0,Y0)起始點; (X1,Y1)結束點; Y0=M*X0+B 斜率公式;L已知長度
	//if ((lX1 - lX0) == 0)
	//{
	//	dM = 0;
	//}
	//else
	//{
	//	dM = ((float)lY1 - (float)lY0) / ((float)lX1 - (float)lX0);
	//}
	//dB = lY0 - (dM*lX0);
	//LONG la = lX1 - lX0, lb = lY1 - lY0;
	//if (la == 0)
	//{
	//	if (lY0 > lY1)
	//	{
	//		lYClose = lY1 + lLineClose;
	//		lXClose = lX1;
	//	}
	//	else
	//	{
	//		lYClose = lY1 - lLineClose;
	//		lXClose = lX1;
	//	}
	//}
	//else if (lb == 0)
	//{
	//	if (lX0 > lX1)
	//	{
	//		lXClose = lX1 + lLineClose;
	//		lYClose = lY1;
	//	}
	//	else
	//	{
	//		lXClose = lX1 - lLineClose;
	//		lYClose = lY1;
	//	}
	//}
	//else
	//{
	//	if (abs(la) > abs(lb))
	//	{
	//		if (la > 0)
	//		{
	//			lXClose = lX1 - lLineClose;
	//			lYClose = LONG((dM*lX1) + dB);
	//		}
	//		else
	//		{
	//			lXClose = lX1 + lLineClose;
	//			lYClose = LONG((dM*lX1) + dB);
	//		}
	//	}
	//	else if (abs(la) == abs(lb))
	//	{
	//		if (la > 0)
	//		{
	//			lXClose = LONG(lX1 - ((float)lLineClose / sqrt(2)));
	//			lYClose = LONG(lY1 - ((float)lLineClose / sqrt(2)));
	//		}
	//		else
	//		{
	//			lXClose = LONG(lX1 + ((float)lLineClose / sqrt(2)));
	//			lYClose = LONG(lY1 + ((float)lLineClose / sqrt(2)));
	//		}
	//	}
	//	else if (abs(la) < abs(lb))
	//	{
	//		if (lb > 0)
	//		{
	//			lYClose = lY1 - lLineClose;
	//			lXClose = LONG((lYClose - dB) / dM);
	//		}
	//		else
	//		{
	//			lYClose = lY1 + lLineClose;
	//			lXClose = LONG((lYClose - dB) / dM);
	//		}
	//	}
	//}
}
/*
*直線距離轉換成座標點--多載
*/
void CAction::LineGetToPoint(LONG &lXClose, LONG &lYClose, LONG &lZClose, LONG lX0, LONG lY0, LONG lX1, LONG lY1, LONG lZ0, LONG lZ1, LONG &lLineClose)
{
	LONG lLength = 0;
	lLength = LONG(sqrt(pow(lX0 - lX1, 2) + pow(lY0 - lY1, 2) + pow(lZ0 - lZ1, 2)));
	if (lZ0 == lZ1)
	{
		lZClose = lZ0;
	}
	else
	{
		lZClose = (lLineClose * (lZ1 - lZ0) / lLength) + lZ0;
	}
	LineGetToPoint(lXClose, lYClose, lX0, lY0, lX1, lY1, lLineClose);
}
/*直線段使用，設置距離在起始點出膠*/
LONG CAction::CalPreglue(LONG lStartDistance, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
	DOUBLE dStartTime = 0;
	LONG lDistance = 0;
	DOUBLE S = 0, t = 0;
	if (lAcceleration == 0)
	{
		t = (DOUBLE)lStartDistance / (DOUBLE)lWorkVelociy;
		return (LONG)round(t * 1000000);
	}
	t = (DOUBLE)(lWorkVelociy - lInitVelociy) / (DOUBLE)lAcceleration;
	S = (DOUBLE)lInitVelociy*t + (DOUBLE)lAcceleration*pow(t, 2) / 2.0;
	if (lStartDistance >= (LONG)round(S))
	{
		lDistance = lStartDistance - (LONG)round(S);
		dStartTime = (DOUBLE)lDistance / (DOUBLE)lWorkVelociy;
		dStartTime += t;
		return (LONG)round(dStartTime * 1000000);
	}
	else
	{
		AfxMessageBox(L"設置距離不適當");
		return (LONG)round(t * 1000000);
	}
}
LONG CAction::CalPreglue(LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
	double S = 0, t = 0;
	if (lAcceleration == 0)
	{
		return 0;
	}
	t = (double)(lWorkVelociy - lInitVelociy) / (double)lAcceleration;
	S = (double)lInitVelociy*t + (double)lAcceleration*pow(t, 2) / 2.0;
	return (LONG)round(S);
}
LONG CAction::CalPreglueTime(LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
	double S = 0, t = 0;
	if (lAcceleration == 0)
	{
		return 0;
	}
	t = (double)(lWorkVelociy - lInitVelociy) / (double)lAcceleration;
	S = (double)lInitVelociy*t + (double)lAcceleration*pow(t, 2) / 2.0;
	return (LONG)round(t * 1000000);
}
/*
*三點計算圓心
*return CString(x軸圓心，y軸圓心，0逆轉/1順轉，)
*/
CString CAction::TRoundCCalculation(CString Origin, CString End, CString Between)
{
	if (Origin == End || End == Between || Origin == Between)
	{
		AfxMessageBox(_T("What the fucking you type in my system."));
		return _T("F");
	}
	CString Ans;
	double x1 = double(CStringToLong(Origin, 0)); double y1 = double(CStringToLong(Origin, 1));
	double x2 = double(CStringToLong(End, 0)); double y2 = double(CStringToLong(End, 1));
	double x3 = double(CStringToLong(Between, 0)); double y3 = double(CStringToLong(Between, 1));

	double tan13 = (y3 - y1) / (x3 - x1);
	if ((tan13) >= 200) tan13 = 200;
	else if ((tan13) <= -200) tan13 = -200;
	double tan12 = (y2 - y1) / (x2 - x1);
	if ((tan12) >= 200) tan12 = 200;
	else if ((tan12) <= -200) tan13 = -200;

	double a = 2 * (x1 - x2);
	double b = 2 * (y1 - y2);
	double c = y1*y1 + x1*x1 - x2*x2 - y2*y2;
	double d = 2 * (x1 - x3);
	double e = 2 * (y1 - y3);
	double f = y1*y1 + x1*x1 - x3*x3 - y3*y3;
	int h = int( round((e*c - b*f) / (a*e - b*d)));
	int k = int( round((a*f - d*c) / (a*e - b*d)));
	Ans.Format(_T("%d，%d，"), h, k);
	if ((x2 - x1) > 0)
	{
		if ((y3 - tan12*x3) > (y1 - tan12*x1))
		{
			Ans = Ans + _T("0") + _T("，");
		}
		else if ((y3 - tan12*x3) < (y1 - tan12*x1))
		{
			Ans = Ans + _T("1") + _T("，");
		}
		else
		{
			Ans = _T("F");
			AfxMessageBox(_T("三點共線"));
		}
	}
	else if ((x2 - x1) < 0)
	{
		if ((y3 - tan12*x3) > (y1 - tan12*x1))
		{
			Ans = Ans + _T("1") + _T("，");
		}
		else if ((y3 - tan12*x3) < (y1 - tan12*x1))
		{
			Ans = Ans + _T("0") + _T("，");
		}
		else
		{
			Ans = _T("F");
			AfxMessageBox(_T("三點共線"));
		}
	}
	else
	{
		if ((x3 > x1))
		{
			Ans = Ans + _T("1") + _T("，");
		}
		else if ((x3 < x1))
		{
			Ans = Ans + _T("0") + _T("，");
		}
		else
		{
			Ans = _T("F");
			AfxMessageBox(_T("三點共線"));
		}
	}
	return Ans;
}
/*
*字串轉長整數
*使用的結尾一定要是"，"
*/
LONG CAction::CStringToLong(CString csData, int iChoose)
{
	LONG lOne[256] = { 0 };//轉換的值
	int ione[256] = { 0 };//標記位置
	ione[0] = csData.Find(L"，");
	for (int i = 0; i<csData.GetLength(); i++)
	{
		ione[i + 1] = csData.Find(L"，", ione[i] + 1);
	}
	lOne[0] = _ttol(csData.Left(ione[0]));
	for (int j = 0; j<iChoose; j++)
	{
		lOne[j + 1] = _ttol(csData.Mid(ione[j] + 1, ione[j + 1] - ione[j] - 1));
	}
	return lOne[iChoose];
}
/*
*三點取得角度
*(向量夾角)
*(圓心x,y座標x1,y1,座標x2,y2,正逆轉)
*/
DOUBLE CAction::AngleCount(DOUBLE LocatX, DOUBLE LocatY, DOUBLE LocatX1, DOUBLE LocatY1, DOUBLE LocatX2, DOUBLE LocatY2, BOOL bRev)
{
	DOUBLE offsetx, offsety;
	DOUBLE Xa, Ya;
	DOUBLE theta_a;
	DOUBLE offset_x, offset_y;
	DOUBLE X_a, Y_a;
	DOUBLE theta_b;

	offsetx = LocatX1 - LocatX;
	offsety = LocatY1 - LocatY;

	Xa = offsetx / pow(((pow(offsetx, 2)) + (pow(offsety, 2))), 0.5);
	Ya = offsety / pow(((pow(offsetx, 2)) + (pow(offsety, 2))), 0.5);

	theta_a = (atan(fabs((Ya / Xa)))) * 180 / M_PI;

	if (offsetx<0 && offsety >= 0)
		theta_a = 180 - theta_a;
	else if (offsetx<0 && offsety<0)
		theta_a = 180 + theta_a;
	else if (offsetx >= 0 && offsety<0)
		theta_a = 360 - theta_a;

	offset_x = LocatX2 - LocatX;
	offset_y = LocatY2 - LocatY;

	X_a = offset_x / pow(((pow(offset_x, 2)) + (pow(offset_y, 2))), 0.5);
	Y_a = offset_y / pow(((pow(offset_x, 2)) + (pow(offset_y, 2))), 0.5);

	theta_b = (atan(fabs((Y_a / X_a)))) * 180 / M_PI;

	if (offset_x<0 && offset_y >= 0)
		theta_b = 180 - theta_b;
	else if (offset_x<0 && offset_y<0)
		theta_b = 180 + theta_b;
	else if (offset_x >= 0 && offset_y<0)
		theta_b = 360 - theta_b;

	if (theta_a<theta_b)
		theta_a = theta_b - theta_a;
	else
		theta_a = 360 - (theta_a - theta_b);

	if (bRev == 1)
	{
		theta_a = 360 - theta_a;
	}

	return theta_a;
	//DOUBLE v1x = LocatX1 - LocatX;
	//DOUBLE v1y = LocatY1 - LocatY;
	//DOUBLE v2x = LocatX2 - LocatX;
	//DOUBLE v2y = LocatY2 - LocatY;
	//DOUBLE dCosAng = (v1x*v2x + v1y*v2y) / (sqrt(pow(v1x, 2) + pow(v1y, 2))*sqrt(pow(v2x, 2) + pow(v2y, 2)));
	//dCosAng = acos(dCosAng)*180.0 / M_PI;
	//CString csbuff;
	//csbuff.Format(L"角度=%lf,(%lf,%lf),(%lf,%lf)\n", dCosAng, v1x, v1y, v2x, v2y);
	//AfxMessageBox(csbuff);
	//return dCosAng;
}
/*圓弧長距離轉點做標
*由圓弧長，推出點座標
*bDir 0逆轉/1順轉
*/
void CAction::ArcGetToPoint(LONG &lArcX, LONG &lArcY, LONG lDistance, LONG lX, LONG lY, LONG lCenX, LONG lCenY, LONG lRadius, BOOL bDir)
{
	DOUBLE dAngl = 0;
	dAngl = (DOUBLE)lDistance / (DOUBLE)lRadius;
	if (bDir == 0)
	{
		lArcX = LONG((((DOUBLE)lX - (DOUBLE)lCenX)*cos(dAngl)) - (((DOUBLE)lY - (DOUBLE)lCenY)*sin(dAngl)) + lCenX);
		lArcY = LONG((((DOUBLE)lX - (DOUBLE)lCenX)*sin(dAngl)) + (((DOUBLE)lY - (DOUBLE)lCenY)*cos(dAngl)) + lCenY);
	}
	else
	{
		lArcX = LONG((((DOUBLE)lX - (DOUBLE)lCenX)*cos(-dAngl)) - (((DOUBLE)lY - (DOUBLE)lCenY)*sin(-dAngl)) + lCenX);
		lArcY = LONG((((DOUBLE)lX - (DOUBLE)lCenX)*sin(-dAngl)) + (((DOUBLE)lY - (DOUBLE)lCenY)*cos(-dAngl)) + lCenY);
	}
}
/*附屬---填充形態(型態1矩形s路徑填充)
*輸入(起始點x1,y1,結束點x2,y2,寬度,驅動速度,加速度,初速度)
*/
void CAction::AttachFillType1(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lZ, LONG lZBackDistance, LONG lWidth,
	LONG lStartDelayTime, LONG lStartDistance, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime,
	LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
#ifdef MOVE
	/*起始/結束設定
	*1.lStartDelayTime       移動前延遲
	*2.lStartDistance        開機前從起點移動距離(設置距離)
	*3.lCloseOffDelayTime    關機後在結束點停留時間(停留時間)
	*5.lCloseDistance        距離結束點多遠距離關機(關機距離)
	*6.lCloseONDelayTime     關機後的延遲時間(關機延遲)
	*/
	CPoint pt1(lX1, lY1);
	CPoint pt2(lX2, lY2);
	CPoint ptNow = pt1;
	if (pt1 == pt2 || lWidth == 0)
	{
		return;
	}

	std::vector<DATA_2MOVE> vecData2do;
	vecData2do.clear();
	LONG width = lWidth * 1000;

	//使用(1)移動前延遲(lStartDelayTime)
	if (lStartDelayTime>0)
	{
		if (!g_bIsStop)
		{
			MO_Do3DLineMove(lX1 - MO_ReadLogicPosition(0), lY1 - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration,
				lInitVelociy);//回到起始點!
			PreventMoveError();
		}
		if (!g_bIsStop)
		{
			MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//回到起始點!
			PreventMoveError();
		}
		PauseDoGlue();//暫停回復後重新塗膠(讀取暫停參數，當參數為0時出膠，且點膠機要為開。)
		if (!g_bIsStop && g_bIsDispend == 1)
		{
			MO_GummingSet();//塗膠(不卡)
		}
		MO_Timer(0, 0, lStartDelayTime * 1000);
		MO_Timer(1, 0, lStartDelayTime * 1000);//線段點膠設定---(1)移動前延遲(在線段開始點上)

		Sleep(1);//防止出錯，避免計時器初直為0
		while (MO_Timer(3, 0, 0))
		{
			if (g_bIsStop)
			{
				break;
			}
			Sleep(1);
		}
		/*插入第一點*/
		MO_Do2dDataLine(pt1.x, pt1.y, vecData2do);
	}
	//使用(2)計算出膠距離，設定出膠點
	else if (lStartDistance>0)
	{
		LONG glueDist = lStartDistance;

		if (pt1.x == pt2.x)//兩點垂直
		{
			/*距離轉xy分量*/
			glueDist = LONG(glueDist*M_SQRT1_2);
			glueDist = LONG((pt1.y<pt2.y) ? -glueDist : glueDist);
			MO_Do2dDataLine(pt1.x + glueDist, pt1.y + glueDist, vecData2do);
		}
		else if (pt1.y == pt2.y)//兩點平行
		{
			/*距離轉xy分量*/
			glueDist = LONG(glueDist*M_SQRT1_2);
			glueDist = (pt1.x<pt2.x) ? glueDist : -glueDist;
			MO_Do2dDataLine(pt1.x + glueDist, pt1.y - glueDist, vecData2do);
		}
		else
		{
			glueDist = (pt1.x<pt2.x) ? -glueDist : glueDist;
			MO_Do2dDataLine(pt1.x + glueDist, pt1.y, vecData2do);
		}
	}
	//(1)(2)皆不使用
	else
	{
		/*插入第一點*/
		MO_Do2dDataLine(pt1.x, pt1.y, vecData2do);
	}

	//兩點垂直
	if (pt1.x == pt2.x)
	{
		/*填充區域(矩形)邊長*/
		LONG lHeightOfRect = (LONG)abs(round(double(pt2.y - pt1.y)*M_SQRT1_2));
		/*間隔寬度xy位移量，填充xy位移量計算，oddEven正逆向控制*/
		LONG shiftWidth = LONG(width*M_SQRT1_2);
		LONG shift = (LONG)abs(round((pt1.y - pt2.y) / 2.0));
		BOOL oddEven = TRUE;
		if (pt2.y<pt1.y)
		{
			oddEven = FALSE;
		}
		while (lHeightOfRect >= 0)
		{
			ptNow.x = (oddEven) ? (ptNow.x + shift) : (ptNow.x - shift);
			ptNow.y = (oddEven) ? (ptNow.y + shift) : (ptNow.y - shift);
			MO_Do2dDataLine(ptNow.x, ptNow.y, vecData2do);

			lHeightOfRect -= width;
			if (lHeightOfRect >= 0)
			{
				ptNow.x = (pt2.y>pt1.y) ? (ptNow.x - shiftWidth) : (ptNow.x + shiftWidth);
				ptNow.y = (pt2.y>pt1.y) ? (ptNow.y + shiftWidth) : (ptNow.y - shiftWidth);
				MO_Do2dDataLine(ptNow.x, ptNow.y, vecData2do);
			}
			else
			{
				if (oddEven)
				{
					ptNow = pt2;
					MO_Do2dDataLine(ptNow.x, ptNow.y, vecData2do);
				}
				else
				{
					ptNow.x = (pt2.y>pt1.y) ? pt1.x - shift : pt1.x + shift;
					ptNow.y = LONG((pt1.y + pt2.y) / 2.0);
					MO_Do2dDataLine(ptNow.x, ptNow.y, vecData2do);
					ptNow = pt2;
					MO_Do2dDataLine(ptNow.x, ptNow.y, vecData2do);
				}
			}
			oddEven = !oddEven;
		}
	}
	//兩點平行
	else if (pt1.y == pt2.y)
	{
		LONG lHeightOfRect = (LONG)abs(round(double(pt2.x - pt1.x)*M_SQRT1_2));
		LONG shiftWidth = LONG(width*M_SQRT1_2);
		LONG shift = (LONG)abs(round((pt1.x - pt2.x) / 2.0));
		BOOL oddEven = TRUE;
		if (pt2.x<pt1.x)
		{
			oddEven = FALSE;
		}
		while (lHeightOfRect >= 0)
		{
			ptNow.x = (oddEven) ? (ptNow.x + shift) : (ptNow.x - shift);
			ptNow.y = (oddEven) ? (ptNow.y - shift) : (ptNow.y + shift);
			MO_Do2dDataLine(ptNow.x, ptNow.y, vecData2do);

			lHeightOfRect -= width;
			if (lHeightOfRect >= 0)
			{
				ptNow.x = (pt2.x>pt1.x) ? (ptNow.x + shiftWidth) : (ptNow.x - shiftWidth);
				ptNow.y = (pt2.x>pt1.x) ? (ptNow.y + shiftWidth) : (ptNow.y - shiftWidth);
				MO_Do2dDataLine(ptNow.x, ptNow.y, vecData2do);
			}
			else
			{
				if (oddEven)
				{
					ptNow.x = LONG((pt1.x + pt2.x) / 2.0);
					ptNow.y = (pt2.x>pt1.x) ? pt1.x + shift : pt1.x - shift;
					MO_Do2dDataLine(ptNow.x, ptNow.y, vecData2do);
					ptNow = pt2;
					MO_Do2dDataLine(ptNow.x, ptNow.y, vecData2do);
				}
				else
				{
					ptNow = pt2;
					MO_Do2dDataLine(ptNow.x, ptNow.y, vecData2do);
				}
			}
			oddEven = !oddEven;
		}
	}
	else
	{
		LONG lHeightOfRect = abs(pt2.y - pt1.y);
		LONG shiftX = pt2.x - pt1.x;
		CPoint ptNow = pt1;
		BOOL oddEven = TRUE;
		while (lHeightOfRect >= 0)
		{
			/*x平移下一點*/
			ptNow.x = (oddEven) ? (ptNow.x + shiftX) : (ptNow.x - shiftX);
			MO_Do2dDataLine(ptNow.x, ptNow.y, vecData2do);
			if (ptNow == pt2)
			{
				break;
			}

			/*判斷間隔寬度是否還有剩餘，更新間隔寬度的座標*/
			lHeightOfRect -= width;
			if (lHeightOfRect >= 0)
			{
				/*y寬度平移下一點*/
				ptNow.y = (pt2.y>pt1.y) ? (ptNow.y + width) : (ptNow.y - width);
				MO_Do2dDataLine(ptNow.x, ptNow.y, vecData2do);
			}
			else
			{
				/*間隔寬度不足的處理*/
				if (oddEven)
				{
					/*直接到結束點*/
					ptNow = pt2;
					MO_Do2dDataLine(ptNow.x, ptNow.y, vecData2do);
				}
				else
				{
					/*補足寬度至結束點*/
					ptNow.y = pt2.y;
					MO_Do2dDataLine(ptNow.x, ptNow.y, vecData2do);
					ptNow = pt2;
					MO_Do2dDataLine(ptNow.x, ptNow.y, vecData2do);
				}
			}
			oddEven = !oddEven;
		}
	}

	//點陣列轉換
	LA_AbsToOppo2Move(vecData2do);
	for (UINT i = 1; i < vecData2do.size(); i++)
	{
		DATA_2Do[i - 1] = vecData2do.at(i);
	}
#ifdef MOVE
	if (!g_bIsStop)
	{
		MO_Do3DLineMove(vecData2do.at(0).EndP.x, vecData2do.at(0).EndP.y, 0, lWorkVelociy, lAcceleration,
			lInitVelociy);//回到起始點!
		PreventMoveError();
	}
	if (!g_bIsStop)
	{
		MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//回到起始點!
		PreventMoveError();
	}
#endif
	//使用(2)設置距離
	if (!(lStartDelayTime>0) && lStartDistance>0)
	{
		//timeUpGlue 單位us
		LONG timeUpGlue = CalPreglue(lStartDistance, lWorkVelociy, lAcceleration, lInitVelociy);
		if (!g_bIsStop)
		{		
						CAction::g_YtimeOutGlueSet = TRUE;
#ifdef MOVE
		MO_TimerSetIntter(timeUpGlue, 0);//使用Y timer中斷出膠
#endif
				 }
	}

	//使用(5)關機距離(lCloseDistance)
	if (lCloseDistance>0)
	{
		LONG sumPath = 0;
		LONG finishTime = 0;
		DOUBLE avgTime = 0;
		LONG accLength = CalPreglue(lWorkVelociy, lAcceleration, lInitVelociy);
		for (UINT i = 1; i<vecData2do.size(); i++)
		{
			sumPath += vecData2do.at(i).Distance;
		}
		avgTime = ((DOUBLE)sumPath - (DOUBLE)accLength) / (DOUBLE)lWorkVelociy;
		finishTime = (LONG)round(avgTime * 1000000) + CalPreglueTime(lWorkVelociy, lAcceleration, lInitVelociy);
		LONG closeDistTime = CalPreglue(lCloseDistance, lWorkVelociy, 0, lInitVelociy);
		if (!g_bIsStop)
		{
				CAction::g_ZtimeOutGlueSet = FALSE;
		MO_TimerSetIntter(finishTime - closeDistTime, 1);
				}
	}

	if (lStartDelayTime == 0 && lStartDistance == 0)
	{
		if (!g_bIsStop && g_bIsDispend == 1)
		{
			MO_GummingSet();
		}
	}

	MO_DO2Curve(DATA_2Do, vecData2do.size() - 1, lWorkVelociy);
	PreventMoveError();//防止驅動錯誤
	Sleep(200);
	vecData2do.clear();



	//使用(3)停留時間(lCloseOffDelayTime)
	if (lCloseOffDelayTime>0)
	{
		MO_Timer(0, 0, lCloseOffDelayTime * 1000);
		MO_Timer(1, 0, lCloseOffDelayTime * 1000);//線段點膠設定---(3)停留時間
		Sleep(1);//防止出錯，避免計時器初直為0
		while (MO_Timer(3, 0, 0))
		{
			if (g_bIsStop == 1)
			{
				break;
			}
			Sleep(1);
		}
	}
	PauseStopGlue();//暫停時停指塗膠(g_bIsPause=1)
	MO_StopGumming();//停止出膠

					 //使用(6)關機延遲(lCloseONDelayTime)
	if (!(lCloseDistance>0) && lCloseONDelayTime>0)
	{
		MO_Timer(0, 0, lCloseONDelayTime * 1000);
		MO_Timer(1, 0, lCloseONDelayTime * 1000);//線段點膠設定---(6)關機延遲
		Sleep(1);//防止出錯，避免計時器初直為0
		while (MO_Timer(3, 0, 0))
		{
			if (g_bIsStop == 1)
			{
				break;
			}
			Sleep(1);
		}
	}
	if (!g_bIsStop)
	{
		//*************************填充回升z軸速度目前為驅動速度的兩倍******************
		MO_Do3DLineMove(0, 0, (lZ - lZBackDistance) - lZ, lWorkVelociy * 2, lAcceleration, lInitVelociy);//Z軸返回
		PreventMoveError();//防止軸卡出錯
	}
#endif
}
/*附屬---填充形態(型態2圓形螺旋填充.由外而內)
*輸入(起始點x1,y1,圓心x2,y2,寬度,驅動速度,加速度,初速度)
*/
/*附屬---填充形態(型態2圓形螺旋填充.由外而內)
*輸入(起始點x1,y1,圓心x2,y2,寬度,驅動速度,加速度,初速度)
*/
void CAction::AttachFillType2(LONG lX1, LONG lY1, LONG lCenX, LONG lCenY,
	LONG lZ, LONG lZBackDistance, LONG lWidth,
	LONG lStartDelayTime, LONG lStartDistance, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime,
	LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
	/*線段點膠設定(1.移動前延遲，2.設置距離，3停留時間，5關機距離，6關機延遲)
	LONG lStartDelayTime ,LONG lStartDistance ,LONG lCloseOffDelayTime ,LONG lCloseDistance ,LONG lCloseONDelayTime
	//1.移動前點膠機在一條線段起始點處保持打開的時長。 此延時可防止針頭在流體流動之前沿線段發生移動。
	//2. 點膠機開啟前， 馬達離開直線線段起始點的移動距離。 該距離為馬達提供了足夠的起速時間，主要用來消除過量流體在線段起始處的積聚。
	//3.點膠機關閉後，為了讓壓力在針頭移至下一點前變得均衡而在線段點膠結束點處產生的延時。
	//5.為防止過量流體在線段結束點處發生堆積，點膠機在距離線段結束點前多遠處關閉。
	//6.點膠機在線段結束點處停止後保持開啟的時長。
	*/
#pragma region ****圓型螺旋功能****
#ifdef MOVE
	DOUBLE dRadius = 0, dWidth = 0, dAng0 = 0, dAng1 = 0, dAng2 = 0;
	BOOL bRev = 1;//0逆轉/1順轉
	LONG lLineClose = 0, lXClose = 0, lYClose = 0, lDistance = 0;
	LONG lNowX = 0, lNowY = 0, lTime = 0;
	CPoint cPt1 = 0, cPt2 = 0, cPt3 = 0, cPt4 = 0, cPtCen1 = 0, cPtCen2 = 0, cPt0 = 0;
	int iData = 0, iOdd = 0;//判斷奇偶(奇做上半圓/偶做下半圓)
	CString csbuff = 0;
	DOUBLE dSumPath = 0, dCloseTime = 0;//總路徑長.關機時間
	int icnt = 0;//計數器
	std::vector<CPoint>m_ptVec;
	std::vector<CPoint>::iterator ptIter;//反向迭代器
	m_ptVec.clear();
	cPt1.x = lX1;
	cPt1.y = lY1;
	cPtCen1.x = lCenX;
	cPtCen1.y = lCenY;//上半圓圓心
	dRadius = sqrt(pow(cPtCen1.x - cPt1.x, 2) + pow(cPtCen1.y - cPt1.y, 2));//半徑
	if (LONG(dRadius) == 0)
	{
		return;
	}
	if (lStartDistance>0)//使用--(2)設置距離(lStartDistance)
	{
		LONG lStartX = 0, lStartY = 0;
		ArcGetToPoint(lStartX, lStartY, lStartDistance, lX1, lY1, lCenX, lCenY, LONG(dRadius), bRev);//算出起始點
		cPt0.x = lStartX;
		cPt0.y = lStartY;
		//      m_ptVec.push_back(cPt0);
		if (!g_bIsStop)
		{
			MO_Do3DLineMove(lStartX - MO_ReadLogicPosition(0), lStartY - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration,
				lInitVelociy);//回到起始點!
			PreventMoveError();
		}
		if (!g_bIsStop)
		{
			MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//回到起始點!
			PreventMoveError();
		}
		lTime = LONG(1000000 * (DOUBLE)lStartDistance / (DOUBLE)lWorkVelociy);
		_cwprintf(_T("計時器設置距離的時間=%lf \n"), DOUBLE(lTime / 1000000.0));
		/*======計時器到觸發中斷執行出膠，使用y中斷執行================*/
			if (!g_bIsStop)
		{
						CAction::g_YtimeOutGlueSet = TRUE;
					MO_TimerSetIntter(lTime, 0);//計時到跳至執行序
				}
	}
	else
	{

		if (!g_bIsStop)
		{
			MO_Do3DLineMove(lX1 - MO_ReadLogicPosition(0), lY1 - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration,
				lInitVelociy);//回到起始點!
			PreventMoveError();
		}
		if (!g_bIsStop)
		{
			MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//回到起始點!
			PreventMoveError();
		}
		PauseDoGlue();//暫停回復後重新塗膠(讀取暫停參數，當參數為0時出膠，且點膠機要為開。)
		if (!g_bIsStop && g_bIsDispend == 1)
		{
			MO_GummingSet();//塗膠(不卡)
		}
		if (lStartDelayTime > 0)//使用(1)移動前延遲(lStartDelayTime)
		{
			MO_Timer(0, 0, lStartDelayTime * 1000);
			MO_Timer(1, 0, lStartDelayTime * 1000);//線段點膠設定---(1)移動前延遲(在線段開始點上)
			Sleep(1);//防止出錯，避免計時器初直為0
			while (MO_Timer(3, 0, 0))
			{
				if (g_bIsStop)
				{
					break;
				}
				Sleep(1);
			}
		}
	}
	lDistance = LONG(dRadius);
	dWidth = lWidth * 1000;
	dAng1 = acos((cPt1.x - cPtCen1.x) / dRadius);
	dAng2 = asin((cPt1.y - cPtCen1.y) / dRadius);
	/*cPt2.x = LONG(dRadius*cos(dAng1 + M_PI) + cPtCen1.x);
	cPt2.y = LONG(dRadius*sin(dAng2 + M_PI) + cPtCen1.y);*/
	cPt2.x = (LONG)((cPt1.x - cPtCen1.x)*cos(M_PI) - (cPt1.y - cPtCen1.y)*sin(M_PI) + cPtCen1.x);
	cPt2.y = (LONG)((cPt1.x - cPtCen1.x)*sin(M_PI) + (cPt1.y - cPtCen1.y)*cos(M_PI) + cPtCen1.y);
	dSumPath = M_PI*dRadius;//移動的總長度
	m_ptVec.push_back(cPt1);
	m_ptVec.push_back(cPt2);
	lDistance = LONG(lDistance - dWidth);
	LineGetToPoint(lXClose, lYClose, cPt1.x, cPt1.y, cPtCen1.x, cPtCen1.y,
		lDistance);
	cPt3.x = lXClose;
	cPt3.y = lYClose;
	cPtCen2.x = LONG((cPt3.x + cPt2.x) / 2.0);
	cPtCen2.y = LONG((cPt3.y + cPt2.y) / 2.0);//下半圓圓心
	iData = (int)dRadius % (int)dWidth;
	lDistance = LONG(lDistance + dWidth);
	while (1)
	{
		lDistance = LONG(lDistance - dWidth);
		if (lDistance < dWidth)
		{
						dSumPath -= M_PI / 2 * (sqrt(pow(cPt3.x - cPt4.x, 2) + pow(cPt3.y - cPt4.y, 2)));
			dSumPath += sqrt(pow(cPt3.x - cPt4.x, 2) + pow(cPt3.y - cPt4.y, 2));
			break;
		}
		LineGetToPoint(lXClose, lYClose, cPt1.x, cPt1.y, cPtCen1.x, cPtCen1.y,
			lDistance);
		cPt3.x = lXClose;
		cPt3.y = lYClose;
		if (icnt == 0)
		{
			dSumPath += M_PI / 2 * (sqrt(pow(cPt2.x - cPt3.x, 2) + pow(cPt2.y - cPt3.y, 2)));//移動的總長度
		}
		else
		{
			dSumPath += M_PI / 2 * (sqrt(pow(cPt4.x - cPt3.x, 2) + pow(cPt4.y - cPt3.y, 2)));//移動的總長度
		}
		m_ptVec.push_back(cPt3);
		cPt4.x = (LONG)((cPt3.x - cPtCen1.x)*cos(M_PI) - (cPt3.y - cPtCen1.y)*sin(M_PI) + cPtCen1.x);
		cPt4.y = (LONG)((cPt3.x - cPtCen1.x)*sin(M_PI) + (cPt3.y - cPtCen1.y)*cos(M_PI) + cPtCen1.y);
		dSumPath += M_PI / 2 * (sqrt(pow(cPt3.x - cPt4.x, 2) + pow(cPt3.y - cPt4.y, 2)));//移動的總長度
		m_ptVec.push_back(cPt4);
		icnt++;
	}
	if (lCloseDistance > 0)//使用 --(5)關機距離
	{
		dCloseTime = 1000000 * ((dSumPath - lCloseDistance) / (DOUBLE)lWorkVelociy);
		/*======計時器到觸發中斷執行斷膠，使用z中斷執行================*/
		if (!g_bIsStop)
		{
			CAction::g_ZtimeOutGlueSet = FALSE;
			if (lStartDistance > 0)
			{
				_cwprintf(_T("End關機距離的時間=%lf \n"), DOUBLE(lTime) + DOUBLE(dCloseTime / 1000000.0));
				MO_TimerSetIntter(lTime + LONG(dCloseTime), 1);//計時到跳至執行序
			}
			else
			{
				_cwprintf(_T("End關機距離的時間=%lf \n"), DOUBLE(dCloseTime / 1000000.0));
				MO_TimerSetIntter(LONG(dCloseTime), 1);//計時到跳至執行序
			}
		}
	}
#pragma endregion

	std::vector<DATA_2MOVE> DATA_2DO;
	UINT cnt = 0;
	DATA_2DO.clear();
	icnt = 0;
	for (ptIter = m_ptVec.begin(); ptIter != m_ptVec.end(); ptIter++)
	{
		if (lStartDistance < 0 && cnt == 0)
		{
			cnt++;
		}
		else if ((iData != 0) && (cnt == m_ptVec.size() - 1))
		{
			MO_Do2dDataLine((*ptIter).x, (*ptIter).y, DATA_2DO);
		}
		else if ((iData == 0) && (cnt == m_ptVec.size() - 1))
		{
			MO_Do2dDataCir((*ptIter).x, (*ptIter).y, cPtCen2.x, cPtCen2.y, bRev, DATA_2DO);
		}
		else
		{
			if (cnt % 2 == 0)
			{
				MO_Do2dDataCir((*ptIter).x, (*ptIter).y, cPtCen2.x, cPtCen2.y, bRev, DATA_2DO);
			}
			else
			{
				MO_Do2dDataCir((*ptIter).x, (*ptIter).y, lCenX, lCenY, bRev, DATA_2DO);
			}
		}
		cnt++;
	}
	LA_AbsToOppo2Move(DATA_2DO);
	if (lStartDistance > 0)
	{
		for (UINT i = 0; i < DATA_2DO.size(); i++)
		{
			DATA_2Do[i] = DATA_2DO.at(i);
		}
		MO_DO2Curve(DATA_2Do, DATA_2DO.size(), lWorkVelociy);
	}
	else
	{
		for (UINT i = 1; i < DATA_2DO.size(); i++)
		{
			DATA_2Do[i - 1] = DATA_2DO.at(i);
		}
		MO_DO2Curve(DATA_2Do, DATA_2DO.size() - 1, lWorkVelociy);
	}
	PreventMoveError();//防止驅動錯誤
	Sleep(200);
	DATA_2DO.clear();

	MO_Timer(0, 0, lCloseONDelayTime * 1000);
	MO_Timer(1, 0, lCloseONDelayTime * 1000);//線段點膠設定---(6)關機延遲

	Sleep(1);//防止出錯，避免計時器初直為0
	while (MO_Timer(3, 0, 0))
	{
		if (g_bIsStop == 1)
		{
			break;
		}
		Sleep(1);
	}
	MO_StopGumming();//停止出膠
	if (!g_bIsStop)
	{
		//*************************填充回升z軸速度目前為驅動速度的兩倍******************
		MO_Do3DLineMove(0, 0, (lZ - lZBackDistance) - lZ, lWorkVelociy * 2, lAcceleration,
			lInitVelociy);//Z軸返回
		PreventMoveError();//防止軸卡出錯
	}
	MO_Timer(0, 0, lCloseOffDelayTime * 1000);
	MO_Timer(1, 0, lCloseOffDelayTime * 1000);//線段點膠設定---(3)停留時間
	Sleep(1);//防止出錯，避免計時器初直為0
	while (MO_Timer(3, 0, 0))
	{
		if (g_bIsStop == 1)
		{
			break;
		}
		Sleep(1);
	}
#endif // MOVE
}
/*附屬---填充形態(型態3矩形填充.由外而內)
*輸入(起始點x1,y1,結束點x2,y2,寬度,驅動速度,加速度,初速度)
*/
void CAction::AttachFillType3(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lZ, LONG lZBackDistance, LONG lWidth,
	LONG lStartDelayTime, LONG lStartDistance, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime,
	LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
#ifdef MOVE
#pragma region ****排方型內縮4點功能****
	LONG lNowX = 0, lNowY = 0;
	CPoint cPt1 = 0, cPt2 = 0, cPt3 = 0, cPt4 = 0, cPtCen = 0;
	DOUBLE dRadius = 0, dDistance = 0, dWidth = 0, dAngCenCos = 0, dAngCenSin = 0, dAngCos = 0,
		dAngSin = 0;
	DOUBLE dAngCenCos2 = 0, dAngCenSin2 = 0, dAngCos2 = 0, dAngSin2 = 0;
	DOUBLE dSumPath = 0, dCloseTime = 0;///總距離,關機時間
		LONG timeUpGlue = 0;//設置距離的移動時間
		int icnt = 0;//計數器
		LONG S1 = 0, S2 = 0;//移動第一二段的距離
		std::vector<CPoint>::iterator ptIter;//迭代器
	std::vector<CPoint> m_ptVec;
	m_ptVec.clear();
	cPt1.x = lX1;
	cPt1.y = lY1;
	cPt3.x = lX2;
	cPt3.y = lY2;
	dWidth = lWidth * 1000;
	cPtCen.x = LONG(cPt1.x + (cPt3.x - cPt1.x) / 2.0);
	cPtCen.y = LONG(cPt1.y + (cPt3.y - cPt1.y) / 2.0);
	dRadius = sqrt(pow(cPt1.x - cPtCen.x, 2) + pow(cPt1.y - cPtCen.y, 2));
	if (dRadius == 0)
	{
		return;
	}
	dAngCenCos = acos(double(cPt1.x - cPtCen.x) / dRadius);
	dAngCenSin = asin(double(cPt1.y - cPtCen.y) / dRadius);
	dAngCenCos2 = M_PI * 2 - dAngCenCos;
	dAngCenSin2 = M_PI - dAngCenSin;
	if (abs(dAngCenCos - dAngCenSin) > 0.01)
	{
		if (abs(dAngCenCos - dAngCenSin2) < 0.01)
		{
			dAngCenSin = dAngCenSin2;
		}
		else if (abs(dAngCenCos2 - dAngCenSin) < 0.01)
		{
			dAngCenCos = dAngCenCos2;
		}
		else
		{
			dAngCenSin = dAngCenSin2;
			dAngCenCos = dAngCenCos2;
		}
	}
	else
	{
		dAngCenSin = dAngCenCos;
	}
	cPt2.x = LONG(dRadius*cos(dAngCenCos + M_PI_2) + cPtCen.x);
	cPt2.y = LONG(dRadius*sin(dAngCenSin + M_PI_2) + cPtCen.y);
	cPt4.x = LONG(dRadius*cos(dAngCenCos + M_PI_2 * 3) + cPtCen.x);
	cPt4.y = LONG(dRadius*sin(dAngCenSin + M_PI_2 * 3) + cPtCen.y);
	dDistance = sqrt(pow((cPt1.x - cPt4.x), 2) + pow((cPt1.y - cPt4.y), 2));

#pragma region ****線段塗膠設置****
	//使用(1)移動前延遲(lStartDelayTime)
	if (lStartDelayTime>0)
	{
		if (!g_bIsStop)
		{
			MO_Do3DLineMove(lX1 - MO_ReadLogicPosition(0), lY1 - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration,
				lInitVelociy);//回到起始點!
			PreventMoveError();
		}
		if (!g_bIsStop)
		{
			MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//回到起始點!
			PreventMoveError();
		}
		PauseDoGlue();//暫停回復後重新塗膠(讀取暫停參數，當參數為0時出膠，且點膠機要為開。)
		if (!g_bIsStop && g_bIsDispend == 1)
		{
			MO_GummingSet();//塗膠(不卡)
		}
		MO_Timer(0, 0, lStartDelayTime * 1000);
		MO_Timer(1, 0, lStartDelayTime * 1000);//線段點膠設定---(1)移動前延遲(在線段開始點上)

		Sleep(1);//防止出錯，避免計時器初直為0

				 //計算s1與s2
		S1 = LONG(sqrt(pow(cPt2.x - cPt1.x, 2) + (cPt2.y - cPt1.y, 2)));
		S2 = LONG(sqrt(pow(cPt3.x - cPt2.x, 2) + (cPt3.y - cPt2.y, 2)));
		while (MO_Timer(3, 0, 0))
		{
			if (g_bIsStop)
			{
				break;
			}
			Sleep(1);
		}
		/*插入第一點*/
		m_ptVec.push_back(cPt1);
	}
	//使用(2)計算出膠距離，設定出膠點
	else if (lStartDistance>0)
	{
		CPoint ptSetDist(0, 0);
				LONG lXClose = 0, lYClose = 0;
				LineGetToPoint(lXClose, lYClose, cPt2.x, cPt2.y, lX1, lY1, lStartDistance);
				ptSetDist.x = (-(lXClose - lX1)) + lX1;
				ptSetDist.y = (-(lYClose - lY1)) + lY1;
		//插入設置距離的座標為第一點
		m_ptVec.push_back(ptSetDist);

		//計算s1與s2
		S1 = LONG(sqrt(pow(cPt2.x - ptSetDist.x, 2) + (cPt2.y - ptSetDist.y, 2)));
		S2 = LONG(sqrt(pow(cPt3.x - cPt2.x, 2) + (cPt3.y - cPt2.y, 2)));

	}
	//(1)(2)皆不使用
	else
	{
		m_ptVec.push_back(cPt1);

		//計算s1與s2
		S1 = LONG(sqrt(pow(cPt2.x - cPt1.x, 2) + (cPt2.y - cPt1.y, 2)));
		S2 = LONG(sqrt(pow(cPt3.x - cPt2.x, 2) + (cPt3.y - cPt2.y, 2)));
	}

#pragma endregion

	m_ptVec.push_back(cPt2);
	m_ptVec.push_back(cPt3);
	m_ptVec.push_back(cPt4);
		/*總距離計算dSumPath*/
		dSumPath = sqrt(pow(cPt1.x - cPt2.x, 2) + pow(cPt1.y - cPt2.y, 2));
		dSumPath += sqrt(pow(cPt2.x - cPt3.x, 2) + pow(cPt2.y - cPt3.y, 2));
		dSumPath += sqrt(pow(cPt3.x - cPt4.x, 2) + pow(cPt3.y - cPt4.y, 2));//移動的總長度


	dAngCos = acos((cPt1.x - cPt4.x) / dDistance);
	dAngSin = asin((cPt1.y - cPt4.y) / dDistance);
	if (dAngCos < 0)
	{
		dAngCos += M_PI * 2;
	}
	if (dAngSin < 0)
	{
		dAngSin += M_PI * 2;
	}
	dAngCos2 = M_PI * 2 - dAngCos;
	dAngSin2 = M_PI - dAngSin;
	if (abs(dAngCos - dAngSin) > 0.01)
	{
		if (abs(dAngCos - dAngSin2) < 0.01)
		{
			dAngSin = dAngSin2;
		}
		else if (abs(dAngCos2 - dAngSin) < 0.01)
		{
			dAngCos = dAngCos2;
		}
		else
		{
			dAngCos = dAngCos2;
			dAngSin = dAngSin2;
		}
	}
	else
	{
		dAngSin = dAngCos;
	}
		icnt = 1;
	while (1)
	{
		dRadius = dRadius - dWidth*sqrt(2);
		dDistance = dDistance - dWidth;
		if (dDistance < dWidth)
		{
					if (icnt == 1)
					{
						 dSumPath += sqrt(pow(cPt4.x - cPt1.x, 2) + pow(cPt4.y - cPt1.y, 2));//移動的總長度
					}
			break;
		}
		cPt1.x = LONG(dDistance*cos(dAngCos) + cPt4.x);
		cPt1.y = LONG(dDistance*sin(dAngSin) + cPt4.y);
		m_ptVec.push_back(cPt1);
				dSumPath += sqrt(pow(cPt4.x - cPt1.x, 2) + pow(cPt4.y - cPt1.y, 2));//移動的總長度
		cPt2.x = LONG(dRadius*cos(dAngCenCos + M_PI_2) + cPtCen.x);
		cPt2.y = LONG(dRadius*sin(dAngCenSin + M_PI_2) + cPtCen.y);
		m_ptVec.push_back(cPt2);
				dSumPath += sqrt(pow(cPt1.x - cPt2.x, 2) + pow(cPt1.y - cPt2.y, 2));//移動的總長度
		dDistance = dDistance - dWidth;
		if (dDistance < dWidth)
		{
			break;
		}
		cPt3.x = LONG(dRadius*cos(dAngCenCos + M_PI) + cPtCen.x);
		cPt3.y = LONG(dRadius*sin(dAngCenSin + M_PI) + cPtCen.y);
		m_ptVec.push_back(cPt3);
				dSumPath += sqrt(pow(cPt2.x - cPt3.x, 2) + pow(cPt2.y - cPt3.y, 2));//移動的總長度
		cPt4.x = LONG(dRadius*cos(dAngCenCos + M_PI_2 * 3.0) + cPtCen.x);
		cPt4.y = LONG(dRadius*sin(dAngCenSin + M_PI_2 * 3.0) + cPtCen.y);
		m_ptVec.push_back(cPt4);
				dSumPath += sqrt(pow(cPt3.x - cPt4.x, 2) + pow(cPt3.y - cPt4.y, 2));//移動的總長度
				icnt++;
	}
#pragma endregion
	std::vector<DATA_2MOVE>DATA_2DO;
	DATA_2DO.clear();
	for (ptIter = m_ptVec.begin(); ptIter != m_ptVec.end(); ptIter++)
	{
		MO_Do2dDataLine((*ptIter).x, (*ptIter).y, DATA_2DO);
	}
	//點陣列轉換
	LA_AbsToOppo2Move(DATA_2DO);
	for (UINT i = 1; i < DATA_2DO.size(); i++)
	{
		DATA_2Do[i - 1] = DATA_2DO.at(i);
	}
	if (!g_bIsStop)
	{
		MO_Do3DLineMove(DATA_2DO.at(0).EndP.x, DATA_2DO.at(0).EndP.y, 0, lWorkVelociy, lAcceleration,
			lInitVelociy);//回到起始點!
		PreventMoveError();
	}
	if (!g_bIsStop)
	{
		MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//回到起始點!
		PreventMoveError();
	}
	//使用(2)設置距離
	if (lStartDistance>0)
	{
		//timeUpGlue 單位us
				timeUpGlue = CalPreglue(lStartDistance, lWorkVelociy, lAcceleration, lInitVelociy);
				if (!g_bIsStop)
				{
				CAction::g_YtimeOutGlueSet = TRUE;
			MO_TimerSetIntter(timeUpGlue, 0);//使用y timer中斷 出膠
				}
	}
	//使用(5)關機距離(lCloseDistance)
		DOUBLE T1 = 0, T2 = 0;
		LONG V1 = 0, V2 = 0, AllS = 0, AllT = 0, FinallV = 0;
	if (lCloseDistance>0)
	{
		//第一區塊連續插補
		T1 = (-lInitVelociy / 1000 + (sqrt(pow(lInitVelociy / 1000, 2) + 2 * lAcceleration / 1000 * S1 / 1000))) / (DOUBLE)(lAcceleration / 1000.0);
		V1 = LONG(lInitVelociy + lAcceleration*T1);
		if (lWorkVelociy > V1)
		{
			AllT = LONG(T1 * 1000000);
			//第二區塊連續插補
			T2 = (-V1 / 1000 + (sqrt(pow(V1 / 1000, 2) + 2 * lAcceleration / 1000 * S1 / 1000))) / (DOUBLE)(lAcceleration / 1000.0);
			V2 = LONG(lAcceleration* LONG(T1) + V1);
			if (lWorkVelociy > V2)
			{
				AllT = LONG(AllT + (T2 * 1000000));
				AllS = S1 + S2;
				FinallV = V2;
			}
			else
			{
				AllS = S1;
				FinallV = lWorkVelociy;
			}
		}
		else
		{
			FinallV = lWorkVelociy;
			AllS = 0;
		}
		if (lStartDistance > 0)
		{
			dCloseTime = 1000000 * ((dSumPath - lCloseDistance) / (DOUBLE)lWorkVelociy);
			/*======計時器到觸發中斷執行斷膠，使用z中斷執行================*/
			if (!g_bIsStop)
			{
				CAction::g_ZtimeOutGlueSet = FALSE;
				_cwprintf(_T("End關機距離的時間=%lf \n"), DOUBLE(timeUpGlue) + DOUBLE(dCloseTime / 1000000.0));
				MO_TimerSetIntter(timeUpGlue + LONG(dCloseTime), 1);//計時到跳至執行序
			}
		}
		else
		{
			LONG lCloseTime = 0;
			lCloseTime = LONG(CalPreglue(LONG(dSumPath - AllS - lCloseDistance), FinallV, lAcceleration, lInitVelociy));
			lCloseTime = lCloseTime - AllT;
			/*======計時器到觸發中斷執行斷膠，使用z中斷執行================*/
			if (!g_bIsStop)
			{
				CAction::g_ZtimeOutGlueSet = FALSE;
				_cwprintf(_T("End關機距離的時間=%d \n"), lCloseTime);
				MO_TimerSetIntter(lCloseTime, 1);//計時到跳至執行序
			}
		}
		/*LONG sumPath = 0;
		LONG finishTime = 0;
		DOUBLE avgTime = 0;
		LONG accLength = CalPreglue(lWorkVelociy, lAcceleration, lInitVelociy);
		for(UINT i = 1; i<DATA_2DO.size(); i++)
		{
		sumPath += DATA_2DO.at(i).Distance;
		}
		avgTime = ((DOUBLE)sumPath-(DOUBLE)accLength)/(DOUBLE)lWorkVelociy;
		finishTime = (LONG)round(avgTime*1000000)+CalPreglueTime(lWorkVelociy, lAcceleration, lInitVelociy);
		LONG closeDistTime = CalPreglue(lCloseDistance, lWorkVelociy, 0, lInitVelociy);
		CAction::g_ZtimeOutGlueSet = FALSE;
		MO_TimerSetIntter(finishTime-closeDistTime, 1);*/
	}

	if (lStartDelayTime == 0 && lStartDistance == 0)
	{
		PauseDoGlue();//暫停恢復後繼續出膠(g_bIsPause=0)出膠
	}

	MO_DO2Curve(DATA_2Do, DATA_2DO.size() - 1, lWorkVelociy);
	PreventMoveError();//防止驅動錯誤
	Sleep(200);
	DATA_2DO.clear();

	//使用(3)停留時間(lCloseOffDelayTime)
	if (lCloseOffDelayTime>0)
	{
		MO_Timer(0, 0, lCloseOffDelayTime * 1000);
		MO_Timer(1, 0, lCloseOffDelayTime * 1000);//線段點膠設定---(3)停留時間
		Sleep(1);//防止出錯，避免計時器初直為0
		while (MO_Timer(3, 0, 0))
		{
			if (g_bIsStop == 1)
			{
				break;
			}
			Sleep(1);
		}
	}

	PauseStopGlue();//暫停時停指塗膠(g_bIsPause=1)
	MO_StopGumming();//停止出膠

					 //使用(6)關機延遲(lCloseONDelayTime)
	if (!(lCloseDistance>0) && lCloseONDelayTime>0)
	{
		MO_Timer(0, 0, lCloseONDelayTime * 1000);
		MO_Timer(1, 0, lCloseONDelayTime * 1000);//線段點膠設定---(6)關機延遲
		Sleep(1);//防止出錯，避免計時器初直為0
		while (MO_Timer(3, 0, 0))
		{
			if (g_bIsStop == 1)
			{
				break;
			}
			Sleep(1);
		}
	}

	if (!g_bIsStop)
	{
		//*************************填充回升z軸速度目前為驅動速度的兩倍******************
		MO_Do3DLineMove(0, 0, (lZ - lZBackDistance) - lZ, lWorkVelociy * 2, lAcceleration,
			lInitVelociy);//Z軸返回
		PreventMoveError();//防止軸卡出錯
	}
#endif
}
/*附屬---填充形態(型態4矩形環)
*輸入(起始點x1,y1,結束點x2,y2,寬度,兩端寬度,驅動速度,加速度,初速度)
*/
void CAction::AttachFillType4(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lZ, LONG lZBackDistance, LONG lWidth,
	LONG lWidth2,
	LONG lStartDelayTime, LONG lStartDistance, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime,
	LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
#ifdef MOVE
#pragma region ****排方型內縮4點功能****
	LONG lNowX = 0, lNowY = 0;
	CPoint cPt1 = 0, cPt2 = 0, cPt3 = 0, cPt4 = 0, cPtCen = 0;
	DOUBLE dRadius = 0, dDistance = 0, dWidth = 0, dWidth2 = 0, dAngCenCos = 0,
		dAngCenSin = 0, dAngCos = 0, dAngSin = 0;
	DOUBLE dAngCenCos2 = 0, dAngCenSin2 = 0, dAngCos2 = 0, dAngSin2 = 0;
	int iBuff = 0;//判斷兩端寬度用
	std::vector<CPoint>::iterator ptIter;//迭代器
	std::vector<CPoint> m_ptVec;
	m_ptVec.clear();
	cPt1.x = lX1;
	cPt1.y = lY1;
	cPt3.x = lX2;
	cPt3.y = lY2;
	dWidth = lWidth * 1000;
	dWidth2 = lWidth2 * 1000;
	cPtCen.x = LONG(cPt1.x + (cPt3.x - cPt1.x) / 2.0);
	cPtCen.y = LONG(cPt1.y + (cPt3.y - cPt1.y) / 2.0);
	dRadius = sqrt(pow(cPt1.x - cPtCen.x, 2) + pow(cPt1.y - cPtCen.y, 2));
	if (dRadius == 0)
	{
		return;
	}
	dAngCenCos = acos(DOUBLE(cPt1.x - cPtCen.x) / dRadius);
	dAngCenSin = asin(DOUBLE(cPt1.y - cPtCen.y) / dRadius);
	dAngCenCos2 = M_PI * 2 - dAngCenCos;
	dAngCenSin2 = M_PI - dAngCenSin;
	if (abs(dAngCenCos - dAngCenSin) > 0.01)
	{
		if (abs(dAngCenCos - dAngCenSin2) < 0.01)
		{
			dAngCenSin = dAngCenSin2;
		}
		else if (abs(dAngCenCos2 - dAngCenSin) < 0.01)
		{
			dAngCenCos = dAngCenCos2;
		}
		else
		{
			dAngCenSin = dAngCenSin2;
			dAngCenCos = dAngCenCos2;
		}
	}
	else
	{
		dAngCenSin = dAngCenCos;
	}
	cPt2.x = LONG(dRadius*cos(dAngCenCos + M_PI_2) + cPtCen.x);
	cPt2.y = LONG(dRadius*sin(dAngCenSin + M_PI_2) + cPtCen.y);
	cPt4.x = LONG(dRadius*cos(dAngCenCos + M_PI_2 * 3) + cPtCen.x);
	cPt4.y = LONG(dRadius*sin(dAngCenSin + M_PI_2 * 3) + cPtCen.y);
	dDistance = sqrt(pow((cPt1.x - cPt4.x), 2) + pow((cPt1.y - cPt4.y), 2));


#pragma region ****線段塗膠設置****
	//使用(1)移動前延遲(lStartDelayTime)
	if (lStartDelayTime>0)
	{
		if (!g_bIsStop)
		{
			MO_Do3DLineMove(lX1 - MO_ReadLogicPosition(0), lY1 - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration,
				lInitVelociy);//回到起始點!
			PreventMoveError();
		}
		if (!g_bIsStop)
		{
			MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//回到起始點!
			PreventMoveError();
		}
		PauseDoGlue();//暫停回復後重新塗膠(讀取暫停參數，當參數為0時出膠，且點膠機要為開。)
		if (!g_bIsStop && g_bIsDispend == 1)
		{
			MO_GummingSet();//塗膠(不卡)
		}
		MO_Timer(0, 0, lStartDelayTime * 1000);
		MO_Timer(1, 0, lStartDelayTime * 1000);//線段點膠設定---(1)移動前延遲(在線段開始點上)

		Sleep(1);//防止出錯，避免計時器初直為0
		while (MO_Timer(3, 0, 0))
		{
			if (g_bIsStop)
			{
				break;
			}
			Sleep(1);
		}
		/*插入第一點*/
		m_ptVec.push_back(cPt1);
	}
	//使用(2)計算出膠距離，設定出膠點
	else if (lStartDistance>0)
	{
		CPoint ptSetDist(0, 0);
		LONG glueDist = lStartDistance;
		DOUBLE dLength = sqrt(pow(cPt1.x - cPt2.x, 2) + pow(cPt1.y - cPt2.y, 2));
		if (cPt1.x == cPt2.x)
		{
			ptSetDist = cPt1;
			glueDist = LONG(glueDist*M_SQRT1_2);
			ptSetDist.y = (cPt1.y<cPt2.y) ? cPt1.y - glueDist : cPt1.y + glueDist;
		}
		else if (cPt1.y == cPt2.y)
		{
			ptSetDist = cPt1;
			glueDist = LONG(glueDist*M_SQRT1_2);
			ptSetDist.x = (cPt1.x<cPt2.x) ? cPt1.x - glueDist : cPt1.x + glueDist;
		}
		else
		{
			CPoint ptDist(0, 0);
			ptDist.x = (LONG)round(glueDist*abs(cPt1.x - cPt2.x) / dLength);
			ptDist.y = (LONG)round(glueDist*abs(cPt1.y - cPt2.y) / dLength);

			ptSetDist.x = (cPt1.x<cPt2.x) ? cPt1.x - ptDist.x : cPt1.x + ptDist.x;
			ptSetDist.y = (cPt1.y<cPt2.y) ? cPt1.y - ptDist.y : cPt1.y + ptDist.y;
		}
		//插入設置距離的座標為第一點
		m_ptVec.push_back(ptSetDist);
	}
	//(1)(2)皆不使用
	else
	{
		m_ptVec.push_back(cPt1);
	}
#pragma endregion //結束線段塗膠設置

	m_ptVec.push_back(cPt2);
	m_ptVec.push_back(cPt3);
	m_ptVec.push_back(cPt4);

	dAngCos = acos((cPt1.x - cPt4.x) / dDistance);
	dAngSin = asin((cPt1.y - cPt4.y) / dDistance);
	if (dAngCos < 0)
	{
		dAngCos += M_PI * 2;
	}
	if (dAngSin < 0)
	{
		dAngSin += M_PI * 2;
	}
	dAngCos2 = M_PI * 2 - dAngCos;
	dAngSin2 = M_PI - dAngSin;
	if (abs(dAngCos - dAngSin) > 0.01)
	{
		if (abs(dAngCos - dAngSin2) < 0.01)
		{
			dAngSin = dAngSin2;
		}
		else if (abs(dAngCos2 - dAngSin) < 0.01)
		{
			dAngCos = dAngCos2;
		}
		else
		{
			dAngCos = dAngCos2;
			dAngSin = dAngSin2;
		}
	}
	else
	{
		dAngSin = dAngCos;
	}
	iBuff = 1;
	while (1)
	{
		if ((iBuff == 1) && (dRadius - (2 * dWidth2) < 0))
		{
#ifdef PRINTF
			_cwprintf(_T("兩端寬度過大 \n"));
#endif
			m_ptVec.push_back(cPt1);
			break;
		}
		dRadius = dRadius - dWidth*sqrt(2);
		dDistance = dDistance - dWidth;
		if (iBuff*dWidth >  dWidth2)   //dWidth2兩端寬度
		{
			dDistance += dWidth;
			cPt1.x = LONG(dDistance*cos(dAngCos) + cPt4.x);
			cPt1.y = LONG(dDistance*sin(dAngSin) + cPt4.y);
			m_ptVec.push_back(cPt1);
			break;
		}
		if (dDistance < dWidth)
		{
			break;
		}
		cPt1.x = LONG(dDistance*cos(dAngCos) + cPt4.x);
		cPt1.y = LONG(dDistance*sin(dAngSin) + cPt4.y);
		m_ptVec.push_back(cPt1);
		cPt2.x = LONG(dRadius*cos(dAngCenCos + M_PI_2) + cPtCen.x);
		cPt2.y = LONG(dRadius*sin(dAngCenSin + M_PI_2) + cPtCen.y);
		m_ptVec.push_back(cPt2);
		dDistance = dDistance - dWidth;
		if (dDistance < dWidth)
		{
			break;
		}
		cPt3.x = LONG(dRadius*cos(dAngCenCos + M_PI) + cPtCen.x);
		cPt3.y = LONG(dRadius*sin(dAngCenSin + M_PI) + cPtCen.y);
		m_ptVec.push_back(cPt3);
		cPt4.x = LONG(dRadius*cos(dAngCenCos + M_PI_2 * 3) + cPtCen.x);
		cPt4.y = LONG(dRadius*sin(dAngCenSin + M_PI_2 * 3) + cPtCen.y);
		m_ptVec.push_back(cPt4);
		iBuff++;
	}
#pragma endregion

	std::vector<DATA_2MOVE> DATA_2DO;
	DATA_2DO.clear();
	for (ptIter = m_ptVec.begin(); ptIter != m_ptVec.end(); ptIter++)
	{
		MO_Do2dDataLine((*ptIter).x, (*ptIter).y, DATA_2DO);
	}
	//點陣列轉換
	LA_AbsToOppo2Move(DATA_2DO);
	for (UINT i = 1; i < DATA_2DO.size(); i++)
	{
		DATA_2Do[i - 1] = DATA_2DO.at(i);
	}
	if (!g_bIsStop)
	{
		MO_Do3DLineMove(DATA_2DO.at(0).EndP.x, DATA_2DO.at(0).EndP.y, 0, lWorkVelociy, lAcceleration,
			lInitVelociy);//回到起始點!
		PreventMoveError();
	}
	if (!g_bIsStop)
	{
		MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//回到起始點!
		PreventMoveError();
	}
	//使用(2)設置距離
	if (lStartDistance>0)
	{
		//timeUpGlue 單位us
		LONG timeUpGlue = CalPreglue(lStartDistance, lWorkVelociy, lAcceleration, lInitVelociy);
		if (!g_bIsStop)
		{
				g_YtimeOutGlueSet = TRUE;
				MO_TimerSetIntter(timeUpGlue, 0);//使用z timer中斷 出膠
				}
	}
	//使用(5)關機距離(lCloseDistance)
	if (lCloseDistance>0)
	{
		LONG sumPath = 0;
		LONG finishTime = 0;
		DOUBLE avgTime = 0;
		LONG accLength = CalPreglue(lWorkVelociy, lAcceleration, lInitVelociy);
		for (UINT i = 1; i<DATA_2DO.size(); i++)
		{
			sumPath += DATA_2DO.at(i).Distance;
		}
		avgTime = ((DOUBLE)sumPath - (DOUBLE)accLength) / (DOUBLE)lWorkVelociy;
		finishTime = (LONG)round(avgTime * 1000000) + CalPreglueTime(lWorkVelociy, lAcceleration, lInitVelociy);
		LONG closeDistTime = CalPreglue(lCloseDistance, lWorkVelociy, 0, lInitVelociy);
		if (!g_bIsStop)
		{
					CAction::g_ZtimeOutGlueSet = FALSE;
				MO_TimerSetIntter(finishTime - closeDistTime, 1);
				}
	}

	if (lStartDelayTime == 0 && lStartDistance == 0)
	{
		PauseDoGlue();//暫停恢復後繼續出膠(g_bIsPause=0)出膠
	}
	MO_DO2Curve(DATA_2Do, DATA_2DO.size() - 1, lWorkVelociy);
	PreventMoveError();//防止驅動錯誤
	Sleep(200);
	DATA_2DO.clear();

	//使用(3)停留時間(lCloseOffDelayTime)
	if (lCloseOffDelayTime>0)
	{
		MO_Timer(0, 0, lCloseOffDelayTime * 1000);
		MO_Timer(1, 0, lCloseOffDelayTime * 1000);//線段點膠設定---(3)停留時間
		Sleep(1);//防止出錯，避免計時器初直為0
		while (MO_Timer(3, 0, 0))
		{
			if (g_bIsStop == 1)
			{
				break;
			}
			Sleep(1);
		}
	}

	PauseStopGlue();//暫停時停指塗膠(g_bIsPause=1)
	MO_StopGumming();//停止出膠
					 //使用(6)關機延遲(lCloseONDelayTime)
	if (!(lCloseDistance>0) && lCloseONDelayTime>0)
	{
		MO_Timer(0, 0, lCloseONDelayTime * 1000);
		MO_Timer(1, 0, lCloseONDelayTime * 1000);//線段點膠設定---(6)關機延遲
		Sleep(1);//防止出錯，避免計時器初直為0
		while (MO_Timer(3, 0, 0))
		{
			if (g_bIsStop == 1)
			{
				break;
			}
			Sleep(1);
		}
	}
	if (!g_bIsStop)
	{
		//*************************填充回升z軸速度目前為驅動速度的兩倍******************
		MO_Do3DLineMove(0, 0, (lZ - lZBackDistance) - lZ, lWorkVelociy * 2, lAcceleration,
			lInitVelociy);//Z軸返回
		PreventMoveError();//防止軸卡出錯
	}
#endif
}
/*附屬---填充形態(型態5圓環)
*輸入(起始點x1,y1,結束點x2,y2,寬度,兩端寬度,驅動速度,加速度,初速度)
*/
void CAction::AttachFillType5(LONG lX1, LONG lY1, LONG lCenX, LONG lCenY,
	LONG lZ, LONG lZBackDistance, LONG lWidth, LONG lWidth2,
	LONG lStartDelayTime, LONG lStartDistance, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime,
	LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
	/*線段點膠設定(1.移動前延遲，2.設置距離，3停留時間，5關機距離，6關機延遲)
	LONG lStartDelayTime ,LONG lStartDistance ,LONG lCloseOffDelayTime ,LONG lCloseDistance ,LONG lCloseONDelayTime
	//1.移動前點膠機在一條線段起始點處保持打開的時長。 此延時可防止針頭在流體流動之前沿線段發生移動。
	//2. 點膠機開啟前， 馬達離開直線線段起始點的移動距離。 該距離為馬達提供了足夠的起速時間，主要用來消除過量流體在線段起始處的積聚。
	//3.點膠機關閉後，為了讓壓力在針頭移至下一點前變得均衡而在線段點膠結束點處產生的延時。
	//5.為防止過量流體在線段結束點處發生堆積，點膠機在距離線段結束點前多遠處關閉。
	//6.點膠機在線段結束點處停止後保持開啟的時長。
	*/
#ifdef MOVE
#pragma region ****圓型螺旋功能****
	DOUBLE dRadius = 0, dWidth = 0, dWidth2 = 0, dAng0 = 0, dAng1 = 0, dAng2 = 0;
	BOOL bRev = 1;//0逆轉/1順轉
	LONG lLineClose = 0, lXClose = 0, lYClose = 0, lDistance = 0;
	LONG lNowX = 0, lNowY = 0, lTime = 0;
	CPoint cPt1 = 0, cPt2 = 0, cPt3 = 0, cPt4 = 0, cPtCen1 = 0, cPtCen2 = 0;
	DOUBLE dSumPath = 0, dCloseTime = 0;//總路徑長.關機時間
	int iData = 0, iBuff = 0;//判斷是否有餘數，buff用於計數兩端寬度
	std::vector<CPoint>m_ptVec;
	std::vector<CPoint>::iterator ptIter;//迭代器
	m_ptVec.clear();
	cPt1.x = lX1;
	cPt1.y = lY1;
	cPtCen1.x = lCenX;
	cPtCen1.y = lCenY;//上半圓圓心
	dRadius = sqrt(pow(cPtCen1.x - cPt1.x, 2) + pow(cPtCen1.y - cPt1.y, 2));//半徑
	if (LONG(dRadius) == 0)
	{
		return;
	}
	if (LONG(dRadius) == 0)
	{
		return;
	}
	if (lStartDistance>0)//使用--(2)設置距離(lStartDistance)
	{
		LONG lStartX = 0, lStartY = 0;
		ArcGetToPoint(lStartX, lStartY, lStartDistance, lX1, lY1, lCenX, lCenY, LONG(dRadius), bRev);//算出起始點
		if (!g_bIsStop)
		{
			MO_Do3DLineMove(lStartX - MO_ReadLogicPosition(0), lStartY - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration,
				lInitVelociy);//回到起始點!
			PreventMoveError();
		}
		if (!g_bIsStop)
		{
			MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//回到起始點!
			PreventMoveError();
		}
		lTime = LONG(1000000 * (DOUBLE)lStartDistance / (DOUBLE)lWorkVelociy);
#ifdef PRINTF
		_cwprintf(_T("計時器設置距離的時間=%lf \n"), DOUBLE(lTime / 1000000.0));
#endif
		/*======計時器到觸發中斷執行出膠，使用y中斷執行================*/
		if (!g_bIsStop)
		{
				 CAction::g_YtimeOutGlueSet = TRUE;
					 MO_TimerSetIntter(lTime, 0);//計時到跳至執行序
				}
	}
	else
	{

		if (!g_bIsStop)
		{
			MO_Do3DLineMove(lX1 - MO_ReadLogicPosition(0), lY1 - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration,
				lInitVelociy);//回到起始點!
			PreventMoveError();
		}
		if (!g_bIsStop)
		{
			MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//回到起始點!
			PreventMoveError();
		}
		PauseDoGlue();//暫停回復後重新塗膠(讀取暫停參數，當參數為0時出膠，且點膠機要為開。)
		if (!g_bIsStop && g_bIsDispend == 1)
		{
			MO_GummingSet();//塗膠(不卡)
		}
		if (lStartDelayTime > 0)//使用(1)移動前延遲(lStartDelayTime)
		{
			MO_Timer(0, 0, lStartDelayTime * 1000);
			MO_Timer(1, 0, lStartDelayTime * 1000);//線段點膠設定---(1)移動前延遲(在線段開始點上)
			Sleep(1);//防止出錯，避免計時器初直為0
			while (MO_Timer(3, 0, 0))
			{
				if (g_bIsStop)
				{
					break;
				}
				Sleep(1);
			}
		}
	}
	lDistance = LONG(dRadius);
	dWidth = lWidth * 1000;
	dWidth2 = lWidth2 * 1000;
	dAng1 = acos((cPt1.x - cPtCen1.x) / dRadius);
	dAng2 = asin((cPt1.y - cPtCen1.y) / dRadius);
	//cPt2.x = LONG(dRadius*cos(dAng1 + M_PI) + cPtCen1.x);
	//cPt2.y = LONG(dRadius*sin(dAng2 + M_PI) + cPtCen1.y);
	cPt2.x = (LONG)((cPt1.x - cPtCen1.x)*cos(M_PI) - (cPt1.y - cPtCen1.y)*sin(M_PI) + cPtCen1.x);
	cPt2.y = (LONG)((cPt1.x - cPtCen1.x)*sin(M_PI) + (cPt1.y - cPtCen1.y)*cos(M_PI) + cPtCen1.y);
	dSumPath = M_PI*dRadius;//移動的總長度
	m_ptVec.push_back(cPt1);
	m_ptVec.push_back(cPt2);
	lDistance = LONG(lDistance - dWidth);
	LineGetToPoint(lXClose, lYClose, cPt1.x, cPt1.y, cPtCen1.x, cPtCen1.y,
		lDistance);
	cPt3.x = lXClose;
	cPt3.y = lYClose;
	cPtCen2.x = LONG((cPt3.x + cPt2.x) / 2.0);
	cPtCen2.y = LONG((cPt3.y + cPt2.y) / 2.0);//下半圓圓心
	lDistance = LONG(lDistance + dWidth);
	iData = (int)dRadius % (int)dWidth;
	iBuff = 1;
	while (1)
	{
		if ((iBuff == 1) && (lDistance - (2 * dWidth2) < 0))//表示使用單圓環不螺旋
		{
#ifdef PRINTF
			_cwprintf(_T("兩端寬度過大 \n"));
#endif
			dSumPath += M_PI*dRadius;
			iBuff = 1;
			m_ptVec.push_back(cPt1);//走單圈圓
			break;
		}
		else if ((iBuff == 1) && dWidth2 == 0)//表示使用單圓環不螺旋
		{
#ifdef PRINTF
			_cwprintf(_T("單圓環 \n"));
#endif
			dSumPath += M_PI*dRadius;
			iBuff = 1;
			m_ptVec.push_back(cPt1);//走單圈圓
			break;
		}
		dRadius = dRadius - dWidth;
		lDistance = LONG(lDistance - dWidth);
		if (iBuff * dWidth > dWidth2)
		{
			lDistance += LONG(dWidth);
			LineGetToPoint(lXClose, lYClose, cPt1.x, cPt1.y, cPtCen1.x, cPtCen1.y,
				lDistance);
			cPt3.x = lXClose;
			cPt3.y = lYClose;
			m_ptVec.push_back(cPt3);
			if (iBuff == 1)
			{
				dSumPath += M_PI / 2 * (sqrt(pow(cPt2.x - cPt3.x, 2) + pow(cPt2.y - cPt3.y, 2)));//移動的總長度
			}
			else
			{
				dSumPath += M_PI / 2 * (sqrt(pow(cPt3.x - cPt4.x, 2) + pow(cPt3.y - cPt4.y, 2)));//移動的總長度
			}
			break;
		}
		LineGetToPoint(lXClose, lYClose, cPt1.x, cPt1.y, cPtCen1.x, cPtCen1.y,
			lDistance);
		cPt3.x = lXClose;
		cPt3.y = lYClose;
		if (iBuff == 1)
		{
			dSumPath += M_PI / 2 * (sqrt(pow(cPt2.x - cPt3.x, 2) + pow(cPt2.y - cPt3.y, 2)));//移動的總長度
		}
		else
		{
			dSumPath += M_PI / 2 * (sqrt(pow(cPt4.x - cPt3.x, 2) + pow(cPt4.y - cPt3.y, 2)));//移動的總長度
		}
		m_ptVec.push_back(cPt3);
		cPt4.x = LONG(dRadius*cos(dAng1 + M_PI) + cPtCen1.x);
		cPt4.y = LONG(lDistance*sin(dAng2 + M_PI) + cPtCen1.y);
		dSumPath += M_PI / 2 * (sqrt(pow(cPt3.x - cPt4.x, 2) + pow(cPt3.y - cPt4.y, 2)));//移動的總長度
		m_ptVec.push_back(cPt4);
		iBuff++;
	}
	if (lCloseDistance > 0)//使用 --(5)關機距離
	{
		dCloseTime = 1000000 * ((dSumPath - lCloseDistance) / (DOUBLE)lWorkVelociy);
		/*======計時器到觸發中斷執行斷膠，使用z中斷執行================*/
		if (!g_bIsStop)
		{
				CAction::g_ZtimeOutGlueSet = FALSE;
				if (lStartDistance > 0)
				{
#ifdef PRINTF
					_cwprintf(_T("End關機距離的時間=%lf \n"), DOUBLE(lTime) + DOUBLE(dCloseTime / 1000000.0));
#endif
					MO_TimerSetIntter(lTime + LONG(dCloseTime), 1);//計時到跳至執行序
				}
				else
				{
#ifdef PRINTF
					_cwprintf(_T("End關機距離的時間=%lf \n"), DOUBLE(dCloseTime / 1000000.0));
#endif
					MO_TimerSetIntter(LONG(dCloseTime), 1);//計時到跳至執行序
				}
				}
	}
#pragma endregion
	std::vector<DATA_2MOVE> DATA_2DO;
	DATA_2DO.clear();
	if (iBuff == 1)//表示兩端寬度為零  為畫出一個單圓環
	{
		for (UINT i = 1; i < m_ptVec.size(); i++)
		{
			{
				MO_Do2dDataCir(m_ptVec.at(i).x, m_ptVec.at(i).y, cPtCen1.x, cPtCen1.y, bRev, DATA_2DO);//上半圓
			}
		}
	}
	else
	{
		for (UINT i = 1; i < m_ptVec.size(); i++)
		{
			if (i == m_ptVec.size() - 1)
			{
				if (i % 2 == 0)
				{
					MO_Do2dDataCir(m_ptVec.at(i).x, m_ptVec.at(i).y, cPtCen1.x, cPtCen1.y, bRev, DATA_2DO);//上半圓
				}
				else
				{
					MO_Do2dDataCir(m_ptVec.at(i).x, m_ptVec.at(i).y, cPtCen2.x, cPtCen2.y, bRev, DATA_2DO);//下半圓
				}
			}
			else
			{
				if (i % 2 != 0)
				{
					MO_Do2dDataCir(m_ptVec.at(i).x, m_ptVec.at(i).y, cPtCen1.x, cPtCen1.y, bRev, DATA_2DO);//上半圓
				}
				else
				{
					MO_Do2dDataCir(m_ptVec.at(i).x, m_ptVec.at(i).y, cPtCen2.x, cPtCen2.y, bRev, DATA_2DO);//下半圓
				}
			}
		}
	}
	LA_AbsToOppo2Move(DATA_2DO);
	for (UINT i = 0; i < DATA_2DO.size(); i++)
	{
		DATA_2Do[i] = DATA_2DO.at(i);
	}
	MO_DO2Curve(DATA_2Do, DATA_2DO.size(), lWorkVelociy);
	PreventMoveError();//防止驅動錯誤
	Sleep(200);
	DATA_2DO.clear();
	MO_Timer(0, 0, lCloseONDelayTime * 1000);
	MO_Timer(1, 0, lCloseONDelayTime * 1000);//線段點膠設定---(6)關機延遲
	Sleep(1);//防止出錯，避免計時器初直為0
	while (MO_Timer(3, 0, 0))
	{
		if (g_bIsStop == 1)
		{
			break;
		}
		Sleep(1);
	}
	MO_StopGumming();//停止出膠
	if (!g_bIsStop)
	{
		//*************************填充回升z軸速度目前為驅動速度的兩倍******************
		MO_Do3DLineMove(0, 0, (lZ - lZBackDistance) - lZ, lWorkVelociy * 2, lAcceleration,
			lInitVelociy);//Z軸返回
		PreventMoveError();//防止軸卡出錯
	}
	MO_Timer(0, 0, lCloseOffDelayTime * 1000);
	MO_Timer(1, 0, lCloseOffDelayTime * 1000);//線段點膠設定---(3)停留時間
	Sleep(1);//防止出錯，避免計時器初直為0
	while (MO_Timer(3, 0, 0))
	{
		if (g_bIsStop == 1)
		{
			break;
		}
		Sleep(1);
	}
#endif // MOVE
}
/*附屬---填充形態(型態6矩形填充.由內而外)
*輸入(起始點x1,y1,結束點x2,y2,寬度,驅動速度,加速度,初速度)
*/
void CAction::AttachFillType6(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lZ, LONG lZBackDistance, LONG lWidth,
							  LONG lStartDelayTime, LONG lStartDistance, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime,
							  LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
#ifdef MOVE
#pragma region ****排方型內縮4點功能****
	LONG lNowX = 0, lNowY = 0, lNowZ = 0;
	CPoint cPt1 = 0, cPt2 = 0, cPt3 = 0, cPt4 = 0, cPtCen = 0;
	DOUBLE dRadius = 0, dDistance = 0, dWidth = 0, dWidth2 = 0, dAngCenCos = 0,
		dAngCenSin = 0, dAngCos = 0, dAngSin = 0;
	DOUBLE dAngCenCos2 = 0, dAngCenSin2 = 0, dAngCos2 = 0, dAngSin2 = 0;
	std::vector<CPoint>::reverse_iterator rptIter;//反向迭代器
	std::vector<CPoint> m_ptVec;
	m_ptVec.clear();
	cPt1.x = lX1;
	cPt1.y = lY1;
	cPt3.x = lX2;
	cPt3.y = lY2;
	dWidth = lWidth * 1000;
	cPtCen.x = LONG(cPt1.x + (cPt3.x - cPt1.x) / 2.0);
	cPtCen.y = LONG(cPt1.y + (cPt3.y - cPt1.y) / 2.0);
	dRadius = sqrt(pow(cPt1.x - cPtCen.x, 2) + pow(cPt1.y - cPtCen.y, 2));
	if (dRadius == 0)
	{
		return;
	}
	dAngCenCos = acos(DOUBLE(cPt1.x - cPtCen.x) / dRadius);
	dAngCenSin = asin(DOUBLE(cPt1.y - cPtCen.y) / dRadius);
	dAngCenCos2 = M_PI * 2 - dAngCenCos;
	dAngCenSin2 = M_PI - dAngCenSin;
	if (abs(dAngCenCos - dAngCenSin) > 0.01)
	{
		if (abs(dAngCenCos - dAngCenSin2) < 0.01)
		{
			dAngCenSin = dAngCenSin2;
		}
		else if (abs(dAngCenCos2 - dAngCenSin) < 0.01)
		{
			dAngCenCos = dAngCenCos2;
		}
		else
		{
			dAngCenSin = dAngCenSin2;
			dAngCenCos = dAngCenCos2;
		}
	}
	else
	{
		dAngCenSin = dAngCenCos;
	}
	cPt2.x = LONG(dRadius*cos(dAngCenCos + M_PI_2) + cPtCen.x);
	cPt2.y = LONG(dRadius*sin(dAngCenSin + M_PI_2) + cPtCen.y);
	cPt4.x = LONG(dRadius*cos(dAngCenCos + M_PI_2 * 3) + cPtCen.x);
	cPt4.y = LONG(dRadius*sin(dAngCenSin + M_PI_2 * 3) + cPtCen.y);
	dDistance = sqrt(pow((cPt1.x - cPt4.x), 2) + pow((cPt1.y - cPt4.y), 2));
	m_ptVec.push_back(cPt1);
	m_ptVec.push_back(cPt2);
	m_ptVec.push_back(cPt3);
	m_ptVec.push_back(cPt4);

	dAngCos = acos((cPt1.x - cPt4.x) / dDistance);
	dAngSin = asin((cPt1.y - cPt4.y) / dDistance);
	if (dAngCos < 0)
	{
		dAngCos += M_PI * 2;
	}
	if (dAngSin < 0)
	{
		dAngSin += M_PI * 2;
	}
	dAngCos2 = M_PI * 2 - dAngCos;
	dAngSin2 = M_PI - dAngSin;
	if (abs(dAngCos - dAngSin) > 0.01)
	{
		if (abs(dAngCos - dAngSin2) < 0.01)
		{
			dAngSin = dAngSin2;
		}
		else if (abs(dAngCos2 - dAngSin) < 0.01)
		{
			dAngCos = dAngCos2;
		}
		else
		{
			dAngCos = dAngCos2;
			dAngSin = dAngSin2;
		}
	}
	else
	{
		dAngSin = dAngCos;
	}
	while (1)
	{
		dRadius = dRadius - dWidth*sqrt(2);
		dDistance = dDistance - dWidth;
		if (dDistance < dWidth)
		{
			break;
		}
		cPt1.x = LONG(dDistance*cos(dAngCos) + cPt4.x);
		cPt1.y = LONG(dDistance*sin(dAngSin) + cPt4.y);
		m_ptVec.push_back(cPt1);
		cPt2.x = LONG(dRadius*cos(dAngCenCos + M_PI_2) + cPtCen.x);
		cPt2.y = LONG(dRadius*sin(dAngCenSin + M_PI_2) + cPtCen.y);
		m_ptVec.push_back(cPt2);
		dDistance = dDistance - dWidth;
		if (dDistance < dWidth)
		{
			break;
		}
		cPt3.x = LONG(dRadius*cos(dAngCenCos + M_PI) + cPtCen.x);
		cPt3.y = LONG(dRadius*sin(dAngCenSin + M_PI) + cPtCen.y);
		m_ptVec.push_back(cPt3);
		cPt4.x = LONG(dRadius*cos(dAngCenCos + M_PI_2 * 3) + cPtCen.x);
		cPt4.y = LONG(dRadius*sin(dAngCenSin + M_PI_2 * 3) + cPtCen.y);
		m_ptVec.push_back(cPt4);
	}
#pragma endregion

#pragma region ****線段塗膠設置****
	//使用(1)移動前延遲(lStartDelayTime)
	if (lStartDelayTime>0)
	{
		if (!g_bIsStop)
		{
			rptIter = m_ptVec.rbegin();
			MO_Do3DLineMove(rptIter->x - MO_ReadLogicPosition(0), rptIter->y - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration,
				lInitVelociy);//回到起始點!
			PreventMoveError();
		}
		if (!g_bIsStop)
		{
			MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//回到起始點!
			PreventMoveError();
		}
		PauseDoGlue();//暫停回復後重新塗膠(讀取暫停參數，當參數為0時出膠，且點膠機要為開。)
		if (!g_bIsStop && g_bIsDispend == 1)
		{
			MO_GummingSet();//塗膠(不卡)
		}
		MO_Timer(0, 0, lStartDelayTime * 1000);
		MO_Timer(1, 0, lStartDelayTime * 1000);//線段點膠設定---(1)移動前延遲(在線段開始點上)

		Sleep(1);//防止出錯，避免計時器初直為0
		while (MO_Timer(3, 0, 0))
		{
			if (g_bIsStop)
			{
				break;
			}
			Sleep(1);
		}
	}
	//使用(2)計算出膠距離，設定出膠點
	else if (lStartDistance>0)
	{
		rptIter = m_ptVec.rbegin();
		CPoint rPt1 = *rptIter;
		CPoint rPt2 = *(rptIter + 1);
		CPoint ptSetDist(0, 0);
		LONG glueDist = lStartDistance;
		DOUBLE dLength = sqrt(pow(rPt1.x - rPt2.x, 2) + pow(rPt1.y - rPt2.y, 2));
		if (rPt1.x == rPt2.x)
		{
			ptSetDist = rPt1;
			glueDist = LONG(glueDist*M_SQRT1_2);
			ptSetDist.y = (rPt1.y<rPt2.y) ? rPt1.y - glueDist : rPt1.y + glueDist;
		}
		else if (rPt1.y == rPt2.y)
		{
			ptSetDist = rPt1;
			glueDist = LONG(glueDist*M_SQRT1_2);
			ptSetDist.x = (rPt1.x<rPt2.x) ? rPt1.x - glueDist : rPt1.x + glueDist;
		}
		else
		{
			CPoint ptDist(0, 0);
			ptDist.x = (LONG)round(glueDist*abs(rPt1.x - rPt2.x) / dLength);
			ptDist.y = (LONG)round(glueDist*abs(rPt1.y - rPt2.y) / dLength);

			ptSetDist.x = (rPt1.x<rPt2.x) ? rPt1.x - ptDist.x : rPt1.x + ptDist.x;
			ptSetDist.y = (rPt1.y<rPt2.y) ? rPt1.y - ptDist.y : rPt1.y + ptDist.y;
		}
		//去掉最後一點，插入設置距離的座標為最後一點(反向即為第一點)
		m_ptVec.pop_back();
		m_ptVec.push_back(ptSetDist);
	}
	else
	{
	}

	std::vector<DATA_2MOVE> DATA_2DO;
	DATA_2DO.clear();
	for (rptIter = m_ptVec.rbegin(); rptIter != m_ptVec.rend(); rptIter++)
	{
		MO_Do2dDataLine((*rptIter).x, (*rptIter).y, DATA_2DO);
	}

	LA_AbsToOppo2Move(DATA_2DO);
	for (UINT i = 1; i < DATA_2DO.size(); i++)
	{
		DATA_2Do[i - 1] = DATA_2DO.at(i);
	}
	if (!g_bIsStop)
	{
		MO_Do3DLineMove(DATA_2DO.at(0).EndP.x, DATA_2DO.at(0).EndP.y, 0, lWorkVelociy,
			lAcceleration, lInitVelociy);//移動
		PreventMoveError();
	}
	if (!g_bIsStop)
	{
		MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//Z軸往下
		PreventMoveError();//防止軸卡出錯
	}
	//使用(2)設置距離
	if (lStartDistance>0)
	{
		//timeUpGlue 單位us
		LONG timeUpGlue = CalPreglue(lStartDistance, lWorkVelociy, lAcceleration, lInitVelociy);
		if (!g_bIsStop)
		{
				g_YtimeOutGlueSet = TRUE;
				MO_TimerSetIntter(timeUpGlue, 0);//使用Y timer中斷 出膠
				}
	}
	//使用(5)關機距離(lCloseDistance)
	if (lCloseDistance>0)
	{
		LONG sumPath = 0;
		LONG finishTime = 0;
		DOUBLE avgTime = 0;
		LONG accLength = CalPreglue(lWorkVelociy, lAcceleration, lInitVelociy);
		for (UINT i = 1; i<DATA_2DO.size(); i++)
		{
			sumPath += DATA_2DO.at(i).Distance;
		}
		avgTime = ((DOUBLE)sumPath - (DOUBLE)accLength) / (DOUBLE)lWorkVelociy;
		finishTime = (LONG)round(avgTime * 1000000) + CalPreglueTime(lWorkVelociy, lAcceleration, lInitVelociy);
		LONG closeDistTime = CalPreglue(lCloseDistance, lWorkVelociy, 0, lInitVelociy);
		if (!g_bIsStop)
		{
				CAction::g_ZtimeOutGlueSet = FALSE;
				MO_TimerSetIntter(finishTime - closeDistTime, 1);
				}
	}

	if (lStartDelayTime == 0 && lStartDistance == 0)
	{
		PauseDoGlue();//暫停恢復後繼續出膠(g_bIsPause=0)出膠
	}
	MO_DO2Curve(DATA_2Do, DATA_2DO.size() - 1, lWorkVelociy);
	PreventMoveError();//防止驅動錯誤
	Sleep(200);
	DATA_2DO.clear();
	//使用(3)停留時間(lCloseOffDelayTime)
	if (lCloseOffDelayTime>0)
	{
		MO_Timer(0, 0, lCloseOffDelayTime * 1000);
		MO_Timer(1, 0, lCloseOffDelayTime * 1000);//線段點膠設定---(3)停留時間
		Sleep(1);//防止出錯，避免計時器初直為0
		while (MO_Timer(3, 0, 0))
		{
			if (g_bIsStop == 1)
			{
				break;
			}
			Sleep(1);
		}
	}
	PauseStopGlue();//暫停時停指塗膠(g_bIsPause=1)
	MO_StopGumming();//停止出膠
					 //使用(6)關機延遲(lCloseONDelayTime)
	if (!(lCloseDistance>0) && lCloseONDelayTime>0)
	{
		MO_Timer(0, 0, lCloseONDelayTime * 1000);
		MO_Timer(1, 0, lCloseONDelayTime * 1000);//線段點膠設定---(6)關機延遲
		Sleep(1);//防止出錯，避免計時器初直為0
		while (MO_Timer(3, 0, 0))
		{
			if (g_bIsStop == 1)
			{
				break;
			}
			Sleep(1);
		}
	}
	if (!g_bIsStop)
	{
		//*************************填充回升z軸速度目前為驅動速度的兩倍******************
		MO_Do3DLineMove(0, 0, (lZ - lZBackDistance) - lZ, lWorkVelociy * 2, lAcceleration,
			lInitVelociy);//Z軸返回
		PreventMoveError();//防止軸卡出錯
	}
#endif
}
/*附屬---填充形態(型態7圓形螺旋填充.由內而外)
*輸入(起始點x1,y1,中心點x2,y2,寬度,驅動速度,加速度,初速度)
*/
void CAction::AttachFillType7(LONG lX1, LONG lY1, LONG lCenX, LONG lCenY,
	LONG lZ, LONG lZBackDistance, LONG lWidth,
	LONG lStartDelayTime, LONG lStartDistance, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime,
	LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
	/*線段點膠設定(1.移動前延遲，2.設置距離，3停留時間，5關機距離，6關機延遲)
	LONG lStartDelayTime ,LONG lStartDistance ,LONG lCloseOffDelayTime ,LONG lCloseDistance ,LONG lCloseONDelayTime
	//1.移動前點膠機在一條線段起始點處保持打開的時長。 此延時可防止針頭在流體流動之前沿線段發生移動。
	//2. 點膠機開啟前， 馬達離開直線線段起始點的移動距離。 該距離為馬達提供了足夠的起速時間，主要用來消除過量流體在線段起始處的積聚。
	//3.點膠機關閉後，為了讓壓力在針頭移至下一點前變得均衡而在線段點膠結束點處產生的延時。
	//5.為防止過量流體在線段結束點處發生堆積，點膠機在距離線段結束點前多遠處關閉。
	//6.點膠機在線段結束點處停止後保持開啟的時長。
	*/
#ifdef MOVE
#pragma region ****圓型螺旋功能****
	DOUBLE dRadius = 0, dWidth = 0, dAng0 = 0, dAng1 = 0, dAng2 = 0;
	BOOL bRev = 0;//0逆轉/1順轉
	DOUBLE dSumPath = 0, dCloseTime = 0;//總路徑長.關機時間
	LONG lLineClose = 0, lXClose = 0, lYClose = 0, lDistance = 0;
	LONG lNowX = 0, lNowY = 0, lNowZ = 0, lTime = 0;
	CPoint cPt1 = 0, cPt2 = 0, cPt3 = 0, cPt4 = 0, cPtCen1 = 0, cPtCen2 = 0;
	int iData = 0, iOdd = 0, icnt = 0;//判斷奇偶(奇做上半圓/偶做下半圓)
	CString csbuff = 0;
	std::vector<CPoint>m_ptVec;
	std::vector<CPoint>::reverse_iterator rptIter;//反向迭代器
	m_ptVec.clear();
	cPt1.x = lX1;
	cPt1.y = lY1;
	cPtCen1.x = lCenX;
	cPtCen1.y = lCenY;//上半圓圓心
	dRadius = sqrt(pow(cPtCen1.x - cPt1.x, 2) + pow(cPtCen1.y - cPt1.y, 2));//半徑
	if (LONG(dRadius) == 0)
	{
		return;
	}
	lDistance = LONG(dRadius);
	dWidth = lWidth * 1000;
	dAng1 = acos((cPt1.x - cPtCen1.x) / dRadius);
	dAng2 = asin((cPt1.y - cPtCen1.y) / dRadius);
	//cPt2.x = LONG(dRadius*cos(dAng1 + M_PI) + cPtCen1.x);
	//cPt2.y = LONG(dRadius*sin(dAng2 + M_PI) + cPtCen1.y);
	cPt2.x = (LONG)((cPt1.x - cPtCen1.x)*cos(M_PI) - (cPt1.y - cPtCen1.y)*sin(M_PI) + cPtCen1.x);
	cPt2.y = (LONG)((cPt1.x - cPtCen1.x)*sin(M_PI) + (cPt1.y - cPtCen1.y)*cos(M_PI) + cPtCen1.y);
	dSumPath = M_PI*dRadius;//移動的總長度
	m_ptVec.push_back(cPt1);
	m_ptVec.push_back(cPt2);
	lDistance = LONG(lDistance - dWidth);
	LineGetToPoint(lXClose, lYClose, cPt1.x, cPt1.y, cPtCen1.x, cPtCen1.y,
		lDistance);
	cPt3.x = lXClose;
	cPt3.y = lYClose;
	cPtCen2.x = LONG((cPt3.x + cPt2.x) / 2.0);
	cPtCen2.y = LONG((cPt3.y + cPt2.y) / 2.0);//下半圓圓心
	iData = (int)dRadius % (int)dWidth;
	lDistance = LONG(lDistance + dWidth);
	while (1)
	{
		lDistance = LONG(lDistance - dWidth);
		if (lDistance < dWidth)
		{
					dSumPath -= M_PI / 2 * (sqrt(pow(cPt3.x - cPt4.x, 2) + pow(cPt3.y - cPt4.y, 2)));
					dSumPath += sqrt(pow(cPt3.x - cPt4.x, 2) + pow(cPt3.y - cPt4.y, 2));
			break;
		}
		LineGetToPoint(lXClose, lYClose, cPt1.x, cPt1.y, cPtCen1.x, cPtCen1.y,
			lDistance);
		cPt3.x = lXClose;
		cPt3.y = lYClose;
		if (icnt == 0)
		{
			dSumPath += M_PI / 2 * (sqrt(pow(cPt2.x - cPt3.x, 2) + pow(cPt2.y - cPt3.y, 2)));//移動的總長度
		}
		else
		{
			dSumPath += M_PI / 2 * (sqrt(pow(cPt4.x - cPt3.x, 2) + pow(cPt4.y - cPt3.y, 2)));//移動的總長度
		}
		m_ptVec.push_back(cPt3);
		cPt4.x = (LONG)((cPt3.x - cPtCen1.x)*cos(M_PI) - (cPt3.y - cPtCen1.y)*sin(M_PI) + cPtCen1.x);
		cPt4.y = (LONG)((cPt3.x - cPtCen1.x)*sin(M_PI) + (cPt3.y - cPtCen1.y)*cos(M_PI) + cPtCen1.y);
		dSumPath += M_PI / 2 * (sqrt(pow(cPt3.x - cPt4.x, 2) + pow(cPt3.y - cPt4.y, 2)));//移動的總長度
		m_ptVec.push_back(cPt4);
		icnt++;
	}
#pragma endregion
	if (lStartDistance>0)//使用--(2)設置距離(lStartDistance)
	{
		LONG lStartX = 0, lStartY = 0;
		LineGetToPoint(lStartX, lStartY, lX1, lY1, lCenX, lCenY, lStartDistance);//算出起始點
		if (!g_bIsStop)
		{
			MO_Do3DLineMove(lStartX - MO_ReadLogicPosition(0), lStartY - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration,
				lInitVelociy);//回到起始點!
			PreventMoveError();
		}
		if (!g_bIsStop)
		{
			MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//回到起始點!
			PreventMoveError();
		}
		lTime = CalPreglue(lStartDistance, lWorkVelociy, lAcceleration, lInitVelociy);
#ifdef PRINTF
		_cwprintf(_T("計時器設置距離的時間=%lf \n"), DOUBLE(lTime / 1000000.0));
#endif
		/*======計時器到觸發中斷執行出膠，使用y中斷執行================*/
		if (!g_bIsStop)
		{
				CAction::g_YtimeOutGlueSet = TRUE;
				MO_TimerSetIntter(lTime, 0);//計時到跳至執行序
				}
	}
	else
	{
		//先抬升，移動到中心點在下降
		if (!g_bIsStop)
		{
			MO_Do3DLineMove(lCenX - MO_ReadLogicPosition(0), lCenY - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration,
				lInitVelociy);//直線移動至圓心
			PreventMoveError();
		}
		if (!g_bIsStop)
		{
			MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration,
				lInitVelociy);//Z軸往下
			PreventMoveError();//防止軸卡出錯
		}
		PauseDoGlue();//暫停回復後重新塗膠(讀取暫停參數，當參數為0時出膠，且點膠機要為開。)
		if (!g_bIsStop && g_bIsDispend == 1)
		{
			MO_GummingSet();//塗膠(不卡)
		}
		if (lStartDelayTime > 0)//使用(1)移動前延遲(lStartDelayTime)
		{
			MO_Timer(0, 0, lStartDelayTime * 1000);
			MO_Timer(1, 0, lStartDelayTime * 1000);//線段點膠設定---(1)移動前延遲(在線段開始點上)
			Sleep(1);//防止出錯，避免計時器初直為0
			while (MO_Timer(3, 0, 0))
			{
				if (g_bIsStop)
				{
					break;
				}
				Sleep(1);
			}
		}
	}
	if (lCloseDistance > 0)//使用 --(5)關機距離
	{
		dCloseTime = 1000000 * ((dSumPath - lCloseDistance) / (DOUBLE)lWorkVelociy);
		/*======計時器到觸發中斷執行斷膠，使用z中斷執行================*/
		if (!g_bIsStop)
		{
				CAction::g_ZtimeOutGlueSet = FALSE;
				if (lStartDistance > 0)
				{
#ifdef PRINTF
					_cwprintf(_T("End關機距離的時間=%lf \n"), DOUBLE(lTime) + DOUBLE(dCloseTime / 1000000.0));
#endif
					MO_TimerSetIntter(lTime + LONG(dCloseTime), 1);//計時到跳至執行序
				}
				else
				{
#ifdef PRINTF
					_cwprintf(_T("End關機距離的時間=%lf \n"), DOUBLE(dCloseTime / 1000000.0));
#endif
					MO_TimerSetIntter(LONG(dCloseTime), 1);//計時到跳至執行序
				}
				}
	}
	std::vector<DATA_2MOVE> DATA_2DO;
	DATA_2DO.clear();
	for (rptIter = m_ptVec.rbegin(); rptIter != m_ptVec.rend(); rptIter++)
	{
		if ((iData != 0) && (iOdd == 0))
		{
			MO_Do2dDataLine((*rptIter).x, (*rptIter).y, DATA_2DO);
		}
		else if ((iData == 0) && (iOdd == 0))
		{
			MO_Do2dDataCir((*rptIter).x, (*rptIter).y, cPtCen2.x, cPtCen2.y, bRev, DATA_2DO);
		}
		else
		{
			if (iOdd % 2 == 0)
			{
				MO_Do2dDataCir((*rptIter).x, (*rptIter).y, cPtCen2.x, cPtCen2.y, bRev, DATA_2DO);
			}
			else
			{
				MO_Do2dDataCir((*rptIter).x, (*rptIter).y, lCenX, lCenY, bRev, DATA_2DO);
			}
		}
		iOdd++;
	}
	LA_AbsToOppo2Move(DATA_2DO);
	for (UINT i = 0; i < DATA_2DO.size(); i++)
	{
		DATA_2Do[i] = DATA_2DO.at(i);
	}
	MO_DO2Curve(DATA_2Do, DATA_2DO.size(), lWorkVelociy);
	PreventMoveError();//防止驅動錯誤
	Sleep(200);
	DATA_2DO.clear();

	MO_Timer(0, 0, lCloseONDelayTime * 1000);
	MO_Timer(1, 0, lCloseONDelayTime * 1000);//線段點膠設定---(6)關機延遲
	Sleep(1);//防止出錯，避免計時器初直為0
	while (MO_Timer(3, 0, 0))
	{
		if (g_bIsStop == 1)
		{
			break;
		}
		Sleep(1);
	}
	MO_StopGumming();//停止出膠
	if (!g_bIsStop)
	{
		//*************************填充回升z軸速度目前為驅動速度的兩倍******************
		MO_Do3DLineMove(0, 0, (lZ - lZBackDistance) - lZ, lWorkVelociy * 2, lAcceleration,
			lInitVelociy);//Z軸返回
		PreventMoveError();//防止軸卡出錯
	}
	MO_Timer(0, 0, lCloseOffDelayTime * 1000);
	MO_Timer(1, 0, lCloseOffDelayTime * 1000);//線段點膠設定---(3)停留時間
	Sleep(1);//防止出錯，避免計時器初直為0
	while (MO_Timer(3, 0, 0))
	{
		if (g_bIsStop == 1)
		{
			break;
		}
		Sleep(1);
	}
#endif // MOVE
}
//旋轉
//RefX對位點/OffSetX偏移量(對位的偏移量)/Andgle旋轉的角度/CameraToTipOffsetX (ccd對針頭的偏移量)
void CAction::LA_CorrectLocation(LONG &PointX, LONG &PointY, LONG RefX, LONG RefY, DOUBLE OffSetX, DOUBLE OffSetY, DOUBLE Andgle, DOUBLE CameraToTipOffsetX, DOUBLE CameraToTipOffsetY, BOOL Mode, LONG lSubOffsetX, LONG lSubOffsetY)
{
	double X, Y;
	DOUBLE PX, PY, RX, RY;
	PointX += lSubOffsetX;
	PointY += lSubOffsetY;
	RX = DOUBLE(RefX);
	RY = DOUBLE(RefY);
	//攝影機教導。將座標還原為攝影機模式
	PX = DOUBLE(PointX) - CameraToTipOffsetX;
	PY = DOUBLE(PointY) - CameraToTipOffsetY;

	//非原點的旋轉矩陣、參考原點、位移
	//以攝影機為中心，做座標修正。
	X = cos(Andgle* M_PI / 180)*(PX - RX) - sin(Andgle* M_PI / 180)*(PY - RY) + (RX + OffSetX);
	Y = sin(Andgle* M_PI / 180)*(PX - RX) + cos(Andgle* M_PI / 180)*(PY - RY) + (RY + OffSetY);
	//最後變更為點膠針頭模式
	X = X + CameraToTipOffsetX;
	Y = Y + CameraToTipOffsetY;
	if (Mode)
	{
		//Camera 攝影機模式
		//攝影機和點膠針頭之間的距離，四捨五入
		PointX = LONG(X - CameraToTipOffsetX + 0.5);
		PointY = LONG(Y - CameraToTipOffsetY + 0.5);
	}
	else
	{
		//Tip 點膠針頭模式，四捨五入
		PointX = LONG(X + 0.5);
		PointY = LONG(Y + 0.5);
	}
}
/*附屬---填充形態(型態2圓形螺旋填充.由外而內)
*輸入(起始點x1,y1,圓心x2,y2,寬度,驅動速度,加速度,初速度)
*/
void CAction::AttachFillType2_1(LONG lX1, LONG lY1, LONG lCenX, LONG lCenY, LONG lZ, LONG lZBackDistance, LONG lWidth, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
#ifdef MOVE
#pragma region ****圓型螺旋功能****
	DOUBLE dRadius = 0, dWidth = 0, dAng0 = 0, dAng1 = 0, dAng2 = 0;
	BOOL bRev = 1;//0逆轉/1順轉
	LONG lLineClose = 0, lXClose = 0, lYClose = 0, lDistance = 0;
	LONG lNowX = 0, lNowY = 0;
	CPoint cPt1 = 0, cPt2 = 0, cPt3 = 0, cPt4 = 0, cPtCen1 = 0, cPtCen2 = 0;
	int iData = 0, iOdd = 0;//判斷奇偶(奇做上半圓/偶做下半圓)
	CString csbuff = 0;
	std::vector<CPoint>m_ptVec;
	std::vector<CPoint>::iterator ptIter;//反向迭代器
	m_ptVec.clear();
	cPt1.x = lX1;
	cPt1.y = lY1;
	cPtCen1.x = lCenX;
	cPtCen1.y = lCenY;//上半圓圓心
	dRadius = sqrt(pow(cPtCen1.x - cPt1.x, 2) + pow(cPtCen1.y - cPt1.y, 2));//半徑
	if (dRadius == 0)
	{
		return;
	}
	lDistance = LONG(dRadius);
	dWidth = lWidth * 1000;
	dAng1 = acos((cPt1.x - cPtCen1.x) / dRadius);
	dAng2 = asin((cPt1.y - cPtCen1.y) / dRadius);
	cPt2.x = LONG(dRadius*cos(dAng1 + M_PI) + cPtCen1.x);
	cPt2.y = LONG(dRadius*sin(dAng2 + M_PI) + cPtCen1.y);
	m_ptVec.push_back(cPt1);
	m_ptVec.push_back(cPt2);
	lDistance = LONG(lDistance - dWidth);
	LineGetToPoint(lXClose, lYClose, cPt1.x, cPt1.y, cPtCen1.x, cPtCen1.y,
		lDistance);
	cPt3.x = lXClose;
	cPt3.y = lYClose;
	cPtCen2.x = (cPt3.x + cPt2.x) / 2;
	cPtCen2.y = (cPt3.y + cPt2.y) / 2;//下半圓圓心
	iData = (int)dRadius % (int)dWidth;
	lDistance = LONG(lDistance + dWidth);
	while (1)
	{
		lDistance = LONG(lDistance - dWidth);
		if (lDistance < dWidth)
		{
			break;
		}
		LineGetToPoint(lXClose, lYClose, cPt1.x, cPt1.y, cPtCen1.x, cPtCen1.y,
			lDistance);
		cPt3.x = lXClose;
		cPt3.y = lYClose;
		m_ptVec.push_back(cPt3);
		cPt4.x = LONG(lDistance*cos(dAng1 + M_PI) + cPtCen1.x);
		cPt4.y = LONG(lDistance*sin(dAng2 + M_PI) + cPtCen1.y);
		m_ptVec.push_back(cPt4);
	}
#pragma endregion
	std::vector<DATA_2MOVE> DATA_2DO;
	UINT cnt = 0;
	DATA_2DO.clear();
	for (ptIter = m_ptVec.begin(); ptIter != m_ptVec.end(); ptIter++)
	{
		if ((iData != 0) && (cnt == m_ptVec.size() - 1))
		{
			MO_Do2dDataLine((*ptIter).x, (*ptIter).y, DATA_2DO);
		}
		else if ((iData == 0) && (cnt == m_ptVec.size() - 1))
		{
			MO_Do2dDataCir((*ptIter).x, (*ptIter).y, cPtCen2.x, cPtCen2.y, bRev, DATA_2DO);
		}
		else
		{
			if (cnt % 2 == 0)
			{
				MO_Do2dDataCir((*ptIter).x, (*ptIter).y, cPtCen2.x, cPtCen2.y, bRev, DATA_2DO);
			}
			else
			{
				MO_Do2dDataCir((*ptIter).x, (*ptIter).y, lCenX, lCenY, bRev, DATA_2DO);
			}
		}
		cnt++;
	}
	LA_AbsToOppo2Move(DATA_2DO);

	if ((iData != 0))
	{
		if (!g_bIsStop)
		{
			MO_Do3DLineMove(DATA_2DO.at(0).EndP.x, DATA_2DO.at(0).EndP.y, 0, lWorkVelociy, lAcceleration, lInitVelociy);//直線移動
			PreventMoveError();
		}
		PauseDoGlue();//暫停恢復後繼續出膠(g_bIsPause=0) 出膠
	}
	else
	{
		if (!g_bIsStop)
		{
			MO_Do2DArcMove(DATA_2DO.at(0).EndP.x, DATA_2DO.at(0).EndP.y, DATA_2DO.at(0).CirCentP.x, DATA_2DO.at(0).CirCentP.y,
				lInitVelociy, lWorkVelociy, DATA_2DO.at(0).CirRev);//初始半圓
			PreventMoveError();
		}
		PauseDoGlue();//暫停恢復後繼續出膠(g_bIsPause=0) 出膠
	}

	for (UINT i = 1; i < DATA_2DO.size(); i++)
	{
		DATA_2Do[i - 1] = DATA_2DO.at(i);
	}
	MO_DO2Curve(DATA_2Do, DATA_2DO.size() - 1, lWorkVelociy);
	PreventMoveError();//防止驅動錯誤
	Sleep(200);
	DATA_2DO.clear();

	PauseStopGlue();//暫停時停指塗膠(g_bIsPause=1)
	MO_StopGumming();//停止出膠
	if (!g_bIsStop)
	{
		MO_Do3DLineMove(0, 0, (lZ - lZBackDistance) - lZ, lWorkVelociy, lAcceleration,
			lInitVelociy);//Z軸返回
		PreventMoveError();//防止軸卡出錯
	}
#endif
}
/*附屬---填充形態(型態3矩形填充.由外而內)
*輸入(起始點x1,y1,結束點x2,y2,寬度,驅動速度,加速度,初速度)
*/
void CAction::AttachFillType3_1(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lZ, LONG lZBackDistance, LONG lWidth, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
#ifdef MOVE
#pragma region ****排方型內縮4點功能****
	LONG lNowX = 0, lNowY = 0;
	CPoint cPt1 = 0, cPt2 = 0, cPt3 = 0, cPt4 = 0, cPtCen = 0;
	DOUBLE dRadius = 0, dDistance = 0, dWidth = 0, dAngCenCos = 0, dAngCenSin = 0, dAngCos = 0, dAngSin = 0;
	DOUBLE dAngCenCos2 = 0, dAngCenSin2 = 0, dAngCos2 = 0, dAngSin2 = 0;
	std::vector<CPoint>::iterator ptIter;//迭代器
	std::vector<CPoint> m_ptVec;
	m_ptVec.clear();
	cPt1.x = lX1;
	cPt1.y = lY1;
	cPt3.x = lX2;
	cPt3.y = lY2;
	dWidth = lWidth * 1000;
	cPtCen.x = cPt1.x + (cPt3.x - cPt1.x) / 2;
	cPtCen.y = cPt1.y + (cPt3.y - cPt1.y) / 2;
	dRadius = sqrt(pow(cPt1.x - cPtCen.x, 2) + pow(cPt1.y - cPtCen.y, 2));
	if (dRadius == 0)
	{
		return;
	}
	dAngCenCos = acos(double(cPt1.x - cPtCen.x) / dRadius);
	dAngCenSin = asin(double(cPt1.y - cPtCen.y) / dRadius);
	dAngCenCos2 = M_PI * 2 - dAngCenCos;
	dAngCenSin2 = M_PI - dAngCenSin;
	if (abs(dAngCenCos - dAngCenSin) > 0.01)
	{
		if (abs(dAngCenCos - dAngCenSin2) < 0.01)
		{
			dAngCenSin = dAngCenSin2;
		}
		else if (abs(dAngCenCos2 - dAngCenSin) < 0.01)
		{
			dAngCenCos = dAngCenCos2;
		}
		else
		{
			dAngCenSin = dAngCenSin2;
			dAngCenCos = dAngCenCos2;
		}
	}
	else
	{
		dAngCenSin = dAngCenCos;
	}
	cPt2.x = LONG(dRadius*cos(dAngCenCos + M_PI_2) + cPtCen.x);
	cPt2.y = LONG(dRadius*sin(dAngCenSin + M_PI_2) + cPtCen.y);
	cPt4.x = LONG(dRadius*cos(dAngCenCos + M_PI_2 * 3) + cPtCen.x);
	cPt4.y = LONG(dRadius*sin(dAngCenSin + M_PI_2 * 3) + cPtCen.y);
	dDistance = sqrt(pow((cPt1.x - cPt4.x), 2) + pow((cPt1.y - cPt4.y), 2));
	m_ptVec.push_back(cPt1);
	m_ptVec.push_back(cPt2);
	m_ptVec.push_back(cPt3);
	m_ptVec.push_back(cPt4);
	dAngCos = acos((cPt1.x - cPt4.x) / dDistance);
	dAngSin = asin((cPt1.y - cPt4.y) / dDistance);
	if (dAngCos < 0)
	{
		dAngCos += M_PI * 2;
	}
	if (dAngSin < 0)
	{
		dAngSin += M_PI * 2;
	}
	dAngCos2 = M_PI * 2 - dAngCos;
	dAngSin2 = M_PI - dAngSin;
	if (abs(dAngCos - dAngSin) > 0.01)
	{
		if (abs(dAngCos - dAngSin2) < 0.01)
		{
			dAngSin = dAngSin2;
		}
		else if (abs(dAngCos2 - dAngSin) < 0.01)
		{
			dAngCos = dAngCos2;
		}
		else
		{
			dAngCos = dAngCos2;
			dAngSin = dAngSin2;
		}
	}
	else
	{
		dAngSin = dAngCos;
	}
	while (1)
	{
		dRadius = dRadius - dWidth*sqrt(2);
		dDistance = dDistance - dWidth;
		if (dDistance < dWidth)
		{
			break;
		}
		cPt1.x = LONG(dDistance*cos(dAngCos) + cPt4.x);
		cPt1.y = LONG(dDistance*sin(dAngSin) + cPt4.y);
		m_ptVec.push_back(cPt1);
		cPt2.x = LONG(dRadius*cos(dAngCenCos + M_PI_2) + cPtCen.x);
		cPt2.y = LONG(dRadius*sin(dAngCenSin + M_PI_2) + cPtCen.y);
		m_ptVec.push_back(cPt2);
		dDistance = dDistance - dWidth;
		if (dDistance < dWidth)
		{
			break;
		}
		cPt3.x = LONG(dRadius*cos(dAngCenCos + M_PI) + cPtCen.x);
		cPt3.y = LONG(dRadius*sin(dAngCenSin + M_PI) + cPtCen.y);
		m_ptVec.push_back(cPt3);
		cPt4.x = LONG(dRadius*cos(dAngCenCos + M_PI_2 * 3) + cPtCen.x);
		cPt4.y = LONG(dRadius*sin(dAngCenSin + M_PI_2 * 3) + cPtCen.y);
		m_ptVec.push_back(cPt4);
	}
#pragma endregion
	PauseDoGlue();//暫停恢復後繼續出膠(g_bIsPause=0)出膠
	for (ptIter = m_ptVec.begin(); ptIter != m_ptVec.end(); ptIter++)
	{
		lNowX = MO_ReadLogicPosition(0);
		lNowY = MO_ReadLogicPosition(1);
		if (!g_bIsStop)
		{
			MO_Do3DLineMove((*ptIter).x - lNowX, (*ptIter).y - lNowY, 0, lWorkVelociy, lAcceleration, lInitVelociy);//移動
			PreventMoveError();
		}
	}
	PauseStopGlue();//暫停時停指塗膠(g_bIsPause=1)
	MO_StopGumming();//停止出膠
	if (!g_bIsStop)
	{
		MO_Do3DLineMove(0, 0, (lZ - lZBackDistance) - lZ, lWorkVelociy, lAcceleration, lInitVelociy);//Z軸返回
		PreventMoveError();//防止軸卡出錯
	}
#endif
}
/*附屬---填充形態(型態4矩形環)
*輸入(起始點x1,y1,結束點x2,y2,寬度,兩端寬度,驅動速度,加速度,初速度)
*/
void CAction::AttachFillType4_1(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lZ, LONG lZBackDistance, LONG lWidth, LONG lWidth2, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
#ifdef MOVE
#pragma region ****排方型內縮4點功能****
	LONG lNowX = 0, lNowY = 0;
	CPoint cPt1 = 0, cPt2 = 0, cPt3 = 0, cPt4 = 0, cPtCen = 0;
	DOUBLE dRadius = 0, dDistance = 0, dWidth = 0, dWidth2 = 0, dAngCenCos = 0, dAngCenSin = 0, dAngCos = 0, dAngSin = 0;
	DOUBLE dAngCenCos2 = 0, dAngCenSin2 = 0, dAngCos2 = 0, dAngSin2 = 0;
	int iBuff = 0;//判斷兩端寬度用
	std::vector<CPoint>::iterator ptIter;//迭代器
	std::vector<CPoint> m_ptVec;
	m_ptVec.clear();
	cPt1.x = lX1;
	cPt1.y = lY1;
	cPt3.x = lX2;
	cPt3.y = lY2;
	dWidth = lWidth * 1000;
	dWidth2 = lWidth2 * 1000;
	cPtCen.x = cPt1.x + (cPt3.x - cPt1.x) / 2;
	cPtCen.y = cPt1.y + (cPt3.y - cPt1.y) / 2;
	dRadius = sqrt(pow(cPt1.x - cPtCen.x, 2) + pow(cPt1.y - cPtCen.y, 2));
	if (dRadius == 0)
	{
		return;
	}
	dAngCenCos = acos(DOUBLE(cPt1.x - cPtCen.x) / dRadius);
	dAngCenSin = asin(DOUBLE(cPt1.y - cPtCen.y) / dRadius);
	dAngCenCos2 = M_PI * 2 - dAngCenCos;
	dAngCenSin2 = M_PI - dAngCenSin;
	if (abs(dAngCenCos - dAngCenSin) > 0.01)
	{
		if (abs(dAngCenCos - dAngCenSin2) < 0.01)
		{
			dAngCenSin = dAngCenSin2;
		}
		else if (abs(dAngCenCos2 - dAngCenSin) < 0.01)
		{
			dAngCenCos = dAngCenCos2;
		}
		else
		{
			dAngCenSin = dAngCenSin2;
			dAngCenCos = dAngCenCos2;
		}
	}
	else
	{
		dAngCenSin = dAngCenCos;
	}
	cPt2.x = LONG(dRadius*cos(dAngCenCos + M_PI_2) + cPtCen.x);
	cPt2.y = LONG(dRadius*sin(dAngCenSin + M_PI_2) + cPtCen.y);
	cPt4.x = LONG(dRadius*cos(dAngCenCos + M_PI_2 * 3) + cPtCen.x);
	cPt4.y = LONG(dRadius*sin(dAngCenSin + M_PI_2 * 3) + cPtCen.y);
	dDistance = sqrt(pow((cPt1.x - cPt4.x), 2) + pow((cPt1.y - cPt4.y), 2));
	m_ptVec.push_back(cPt1);
	m_ptVec.push_back(cPt2);
	m_ptVec.push_back(cPt3);
	m_ptVec.push_back(cPt4);

	dAngCos = acos((cPt1.x - cPt4.x) / dDistance);
	dAngSin = asin((cPt1.y - cPt4.y) / dDistance);
	if (dAngCos < 0)
	{
		dAngCos += M_PI * 2;
	}
	if (dAngSin < 0)
	{
		dAngSin += M_PI * 2;
	}
	dAngCos2 = M_PI * 2 - dAngCos;
	dAngSin2 = M_PI - dAngSin;
	if (abs(dAngCos - dAngSin) > 0.01)
	{
		if (abs(dAngCos - dAngSin2) < 0.01)
		{
			dAngSin = dAngSin2;
		}
		else if (abs(dAngCos2 - dAngSin) < 0.01)
		{
			dAngCos = dAngCos2;
		}
		else
		{
			dAngCos = dAngCos2;
			dAngSin = dAngSin2;
		}
	}
	else
	{
		dAngSin = dAngCos;
	}
	iBuff = 1;
	while (1)
	{
		if ((iBuff == 1) && (dRadius - (2 * dWidth2) < 0))
		{
#ifdef PRINTF
			_cwprintf(_T("兩端寬度過大 \n"));
#endif
			m_ptVec.push_back(cPt1);
			break;
		}
		dRadius = dRadius - dWidth*sqrt(2);
		dDistance = dDistance - dWidth;
		if (iBuff*dWidth >  dWidth2) //dWidth2兩端寬度
		{
			dDistance += dWidth;
			cPt1.x = LONG(dDistance*cos(dAngCos) + cPt4.x);
			cPt1.y = LONG(dDistance*sin(dAngSin) + cPt4.y);
			m_ptVec.push_back(cPt1);
			break;
		}
		if (dDistance < dWidth)
		{
			break;
		}
		cPt1.x = LONG(dDistance*cos(dAngCos) + cPt4.x);
		cPt1.y = LONG(dDistance*sin(dAngSin) + cPt4.y);
		m_ptVec.push_back(cPt1);
		cPt2.x = LONG(dRadius*cos(dAngCenCos + M_PI_2) + cPtCen.x);
		cPt2.y = LONG(dRadius*sin(dAngCenSin + M_PI_2) + cPtCen.y);
		m_ptVec.push_back(cPt2);
		dDistance = dDistance - dWidth;
		if (dDistance < dWidth)
		{
			break;
		}
		cPt3.x = LONG(dRadius*cos(dAngCenCos + M_PI) + cPtCen.x);
		cPt3.y = LONG(dRadius*sin(dAngCenSin + M_PI) + cPtCen.y);
		m_ptVec.push_back(cPt3);
		cPt4.x = LONG(dRadius*cos(dAngCenCos + M_PI_2 * 3) + cPtCen.x);
		cPt4.y = LONG(dRadius*sin(dAngCenSin + M_PI_2 * 3) + cPtCen.y);
		m_ptVec.push_back(cPt4);
		iBuff++;
	}
#pragma endregion	
	PauseDoGlue();//暫停恢復後繼續出膠(g_bIsPause=0)出膠
	for (ptIter = m_ptVec.begin(); ptIter != m_ptVec.end(); ptIter++)
	{
		lNowX = MO_ReadLogicPosition(0);
		lNowY = MO_ReadLogicPosition(1);
		if (!g_bIsStop)
		{
			MO_Do3DLineMove((*ptIter).x - lNowX, (*ptIter).y - lNowY, 0, lWorkVelociy, lAcceleration, lInitVelociy);//移動
			PreventMoveError();
		}
	}
	PauseStopGlue();//暫停時停指塗膠(g_bIsPause=1)
	MO_StopGumming();//停止出膠
	if (!g_bIsStop)
	{
		MO_Do3DLineMove(0, 0, (lZ - lZBackDistance) - lZ, lWorkVelociy, lAcceleration, lInitVelociy);//Z軸返回
		PreventMoveError();//防止軸卡出錯
	}
#endif
}
/*附屬---填充形態(型態5圓環)
*輸入(起始點x1,y1,結束點x2,y2,寬度,兩端寬度,驅動速度,加速度,初速度)
*/
void CAction::AttachFillType5_1(LONG lX1, LONG lY1, LONG lCenX, LONG lCenY, LONG lZ, LONG lZBackDistance, LONG lWidth, LONG lWidth2, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
#ifdef MOVE
#pragma region ****圓型螺旋功能****
	DOUBLE dRadius = 0, dWidth = 0, dWidth2 = 0, dAng0 = 0, dAng1 = 0, dAng2 = 0;
	BOOL bRev = 1;//0逆轉/1順轉
	LONG lLineClose = 0, lXClose = 0, lYClose = 0, lDistance = 0;
	LONG lNowX = 0, lNowY = 0;
	CPoint cPt1 = 0, cPt2 = 0, cPt3 = 0, cPt4 = 0, cPtCen1 = 0, cPtCen2 = 0;
	int iData = 0, iBuff = 0;//判斷是否有餘數，buff用於計數兩端寬度
	std::vector<CPoint>m_ptVec;
	std::vector<CPoint>::iterator ptIter;//迭代器
	m_ptVec.clear();
	cPt1.x = lX1;
	cPt1.y = lY1;
	cPtCen1.x = lCenX;
	cPtCen1.y = lCenY;//上半圓圓心
	dRadius = sqrt(pow(cPtCen1.x - cPt1.x, 2) + pow(cPtCen1.y - cPt1.y, 2));//半徑
	if (dRadius == 0)
	{
		return;
	}
	lDistance = LONG(dRadius);
	dWidth = lWidth * 1000;
	dWidth2 = lWidth2 * 1000;
	dAng1 = acos((cPt1.x - cPtCen1.x) / dRadius);
	dAng2 = asin((cPt1.y - cPtCen1.y) / dRadius);
	cPt2.x = LONG(dRadius*cos(dAng1 + M_PI) + cPtCen1.x);
	cPt2.y = LONG(dRadius*sin(dAng2 + M_PI) + cPtCen1.y);
	m_ptVec.push_back(cPt1);
	m_ptVec.push_back(cPt2);
	lDistance = LONG(lDistance - dWidth);
	LineGetToPoint(lXClose, lYClose, cPt1.x, cPt1.y, cPtCen1.x, cPtCen1.y,
		lDistance);
	cPt3.x = lXClose;
	cPt3.y = lYClose;
	cPtCen2.x = (cPt3.x + cPt2.x) / 2;
	cPtCen2.y = (cPt3.y + cPt2.y) / 2;//下半圓圓心
	lDistance = LONG(lDistance + dWidth);
	iData = (int)dRadius % (int)dWidth;
	iBuff = 1;
	while (1)
	{
		if ((iBuff == 1) && (lDistance - (2 * dWidth2) < 0))
		{
#ifdef PRINTF
			_cwprintf(_T("兩端寬度過大 \n"));
#endif
			break;
		}
		dRadius = dRadius - dWidth;
		lDistance = LONG(lDistance - dWidth);
		if (iBuff * dWidth > dWidth2)
		{
			lDistance += LONG(dWidth);
			LineGetToPoint(lXClose, lYClose, cPt1.x, cPt1.y, cPtCen1.x, cPtCen1.y,
				lDistance);
			cPt3.x = lXClose;
			cPt3.y = lYClose;
			m_ptVec.push_back(cPt3);
			break;
		}
		LineGetToPoint(lXClose, lYClose, cPt1.x, cPt1.y, cPtCen1.x, cPtCen1.y,
			lDistance);
		cPt3.x = lXClose;
		cPt3.y = lYClose;
		m_ptVec.push_back(cPt3);
		cPt4.x = LONG(dRadius*cos(dAng1 + M_PI) + cPtCen1.x);
		cPt4.y = LONG(lDistance*sin(dAng2 + M_PI) + cPtCen1.y);
		m_ptVec.push_back(cPt4);
		iBuff++;
	}
#pragma endregion
	std::vector<DATA_2MOVE> DATA_2DO;
	DATA_2DO.clear();
	for (UINT i = 1; i < m_ptVec.size(); i++)
	{
		if (i == m_ptVec.size() - 1)
		{
			if (i % 2 == 0)
			{
				MO_Do2dDataCir(m_ptVec.at(i).x, m_ptVec.at(i).y, cPtCen1.x, cPtCen1.y, bRev, DATA_2DO);//上半圓
			}
			else
			{
				MO_Do2dDataCir(m_ptVec.at(i).x, m_ptVec.at(i).y, cPtCen2.x, cPtCen2.y, bRev, DATA_2DO);//下半圓
			}
		}
		else
		{
			if (i % 2 != 0)
			{
				MO_Do2dDataCir(m_ptVec.at(i).x, m_ptVec.at(i).y, cPtCen1.x, cPtCen1.y, bRev, DATA_2DO);//上半圓
			}
			else
			{
				MO_Do2dDataCir(m_ptVec.at(i).x, m_ptVec.at(i).y, cPtCen2.x, cPtCen2.y, bRev, DATA_2DO);//下半圓
			}
		}
	}
	LA_AbsToOppo2Move(DATA_2DO);
	PauseDoGlue();//暫停恢復後繼續出膠(g_bIsPause=0) 出膠
	for (UINT i = 0; i < DATA_2DO.size(); i++)
	{
		DATA_2Do[i] = DATA_2DO.at(i);
	}
	MO_DO2Curve(DATA_2Do, DATA_2DO.size(), lWorkVelociy);
	PreventMoveError();//防止驅動錯誤
	Sleep(200);
	DATA_2DO.clear();

	PauseStopGlue();//暫停時停指塗膠(g_bIsPause=1)
	MO_StopGumming();//停止出膠
	if (!g_bIsStop)
	{
		MO_Do3DLineMove(0, 0, (lZ - lZBackDistance) - lZ, lWorkVelociy, lAcceleration,
			lInitVelociy);//Z軸返回
		PreventMoveError();//防止軸卡出錯
	}
#endif
}
/*附屬---填充形態(型態6矩形填充.由內而外)
*輸入(起始點x1,y1,結束點x2,y2,寬度,驅動速度,加速度,初速度)
*/
void CAction::AttachFillType6_1(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lZ, LONG lZBackDistance, LONG lWidth, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
#ifdef MOVE
#pragma region ****排方型內縮4點功能****
	LONG lNowX = 0, lNowY = 0, lNowZ = 0;
	CPoint cPt1 = 0, cPt2 = 0, cPt3 = 0, cPt4 = 0, cPtCen = 0;
	DOUBLE dRadius = 0, dDistance = 0, dWidth = 0, dWidth2 = 0, dAngCenCos = 0, dAngCenSin = 0, dAngCos = 0, dAngSin = 0;
	DOUBLE dAngCenCos2 = 0, dAngCenSin2 = 0, dAngCos2 = 0, dAngSin2 = 0;
	std::vector<CPoint>::reverse_iterator rptIter;//反向迭代器
	std::vector<CPoint> m_ptVec;
	m_ptVec.clear();
	cPt1.x = lX1;
	cPt1.y = lY1;
	cPt3.x = lX2;
	cPt3.y = lY2;
	dWidth = lWidth * 1000;
	cPtCen.x = cPt1.x + (cPt3.x - cPt1.x) / 2;
	cPtCen.y = cPt1.y + (cPt3.y - cPt1.y) / 2;
	dRadius = sqrt(pow(cPt1.x - cPtCen.x, 2) + pow(cPt1.y - cPtCen.y, 2));
	if (dRadius == 0)
	{
		return;
	}
	dAngCenCos = acos(DOUBLE(cPt1.x - cPtCen.x) / dRadius);
	dAngCenSin = asin(DOUBLE(cPt1.y - cPtCen.y) / dRadius);
	dAngCenCos2 = M_PI * 2 - dAngCenCos;
	dAngCenSin2 = M_PI - dAngCenSin;
	if (abs(dAngCenCos - dAngCenSin) > 0.01)
	{
		if (abs(dAngCenCos - dAngCenSin2) < 0.01)
		{
			dAngCenSin = dAngCenSin2;
		}
		else if (abs(dAngCenCos2 - dAngCenSin) < 0.01)
		{
			dAngCenCos = dAngCenCos2;
		}
		else
		{
			dAngCenSin = dAngCenSin2;
			dAngCenCos = dAngCenCos2;
		}
	}
	else
	{
		dAngCenSin = dAngCenCos;
	}
	cPt2.x = LONG(dRadius*cos(dAngCenCos + M_PI_2) + cPtCen.x);
	cPt2.y = LONG(dRadius*sin(dAngCenSin + M_PI_2) + cPtCen.y);
	cPt4.x = LONG(dRadius*cos(dAngCenCos + M_PI_2 * 3) + cPtCen.x);
	cPt4.y = LONG(dRadius*sin(dAngCenSin + M_PI_2 * 3) + cPtCen.y);
	dDistance = sqrt(pow((cPt1.x - cPt4.x), 2) + pow((cPt1.y - cPt4.y), 2));
	m_ptVec.push_back(cPt1);
	m_ptVec.push_back(cPt2);
	m_ptVec.push_back(cPt3);
	m_ptVec.push_back(cPt4);

	dAngCos = acos((cPt1.x - cPt4.x) / dDistance);
	dAngSin = asin((cPt1.y - cPt4.y) / dDistance);
	if (dAngCos < 0)
	{
		dAngCos += M_PI * 2;
	}
	if (dAngSin < 0)
	{
		dAngSin += M_PI * 2;
	}
	dAngCos2 = M_PI * 2 - dAngCos;
	dAngSin2 = M_PI - dAngSin;
	if (abs(dAngCos - dAngSin) > 0.01)
	{
		if (abs(dAngCos - dAngSin2) < 0.01)
		{
			dAngSin = dAngSin2;
		}
		else if (abs(dAngCos2 - dAngSin) < 0.01)
		{
			dAngCos = dAngCos2;
		}
		else
		{
			dAngCos = dAngCos2;
			dAngSin = dAngSin2;
		}
	}
	else
	{
		dAngSin = dAngCos;
	}
	while (1)
	{
		dRadius = dRadius - dWidth*sqrt(2);
		dDistance = dDistance - dWidth;
		if (dDistance < dWidth)
		{
			break;
		}
		cPt1.x = LONG(dDistance*cos(dAngCos) + cPt4.x);
		cPt1.y = LONG(dDistance*sin(dAngSin) + cPt4.y);
		m_ptVec.push_back(cPt1);
		cPt2.x = LONG(dRadius*cos(dAngCenCos + M_PI_2) + cPtCen.x);
		cPt2.y = LONG(dRadius*sin(dAngCenSin + M_PI_2) + cPtCen.y);
		m_ptVec.push_back(cPt2);
		dDistance = dDistance - dWidth;
		if (dDistance < dWidth)
		{
			break;
		}
		cPt3.x = LONG(dRadius*cos(dAngCenCos + M_PI) + cPtCen.x);
		cPt3.y = LONG(dRadius*sin(dAngCenSin + M_PI) + cPtCen.y);
		m_ptVec.push_back(cPt3);
		cPt4.x = LONG(dRadius*cos(dAngCenCos + M_PI_2 * 3) + cPtCen.x);
		cPt4.y = LONG(dRadius*sin(dAngCenSin + M_PI_2 * 3) + cPtCen.y);
		m_ptVec.push_back(cPt4);
	}
#pragma endregion	
	//先抬升，移動到中心點在下降
	if (!g_bIsStop)
	{
		MO_Do3DLineMove(0, 0, (lZ - lZBackDistance) - lZ, lWorkVelociy, lAcceleration, lInitVelociy);//Z軸返回
		PreventMoveError();//防止軸卡出錯
	}
	for (rptIter = m_ptVec.rbegin(); rptIter != m_ptVec.rend(); rptIter++)
	{
		lNowX = MO_ReadLogicPosition(0);
		lNowY = MO_ReadLogicPosition(1);
		if (!g_bIsStop)
		{
			MO_Do3DLineMove((*rptIter).x - lNowX, (*rptIter).y - lNowY, 0, lWorkVelociy, lAcceleration, lInitVelociy);//移動
			PreventMoveError();
		}
		if (rptIter == m_ptVec.rbegin())
		{
			if (!g_bIsStop)
			{
				lNowZ = MO_ReadLogicPosition(2);
				MO_Do3DLineMove(0, 0, lZ - lNowZ, lWorkVelociy, lAcceleration, lInitVelociy);//Z軸往下
				PreventMoveError();//防止軸卡出錯
			}
			PauseDoGlue();//暫停恢復後繼續出膠(g_bIsPause=0) 出膠
		}
	}
	PauseStopGlue();//暫停時停指塗膠(g_bIsPause=1)
	MO_StopGumming();//停止出膠
	if (!g_bIsStop)
	{
		MO_Do3DLineMove(0, 0, (lZ - lZBackDistance) - lZ, lWorkVelociy, lAcceleration, lInitVelociy);//Z軸返回
		PreventMoveError();//防止軸卡出錯
	}
#endif
}
/*附屬---填充形態(型態7圓形螺旋填充.由內而外)
*輸入(起始點x1,y1,中心點x2,y2,寬度,驅動速度,加速度,初速度)
*/
void CAction::AttachFillType7_1(LONG lX1, LONG lY1, LONG lCenX, LONG lCenY, LONG lZ, LONG lZBackDistance, LONG lWidth, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
#ifdef MOVE
#pragma region ****圓型螺旋功能****
	DOUBLE dRadius = 0, dWidth = 0, dAng0 = 0, dAng1 = 0, dAng2 = 0;
	BOOL bRev = 0;//0逆轉/1順轉
	LONG lLineClose = 0, lXClose = 0, lYClose = 0, lDistance = 0;
	LONG lNowX = 0, lNowY = 0, lNowZ = 0;
	CPoint cPt1 = 0, cPt2 = 0, cPt3 = 0, cPt4 = 0, cPtCen1 = 0, cPtCen2 = 0;
	int iData = 0, iOdd = 0;//判斷奇偶(奇做上半圓/偶做下半圓)
	CString csbuff = 0;
	std::vector<CPoint>m_ptVec;
	std::vector<CPoint>::reverse_iterator rptIter;//反向迭代器
	m_ptVec.clear();
	cPt1.x = lX1;
	cPt1.y = lY1;
	cPtCen1.x = lCenX;
	cPtCen1.y = lCenY;//上半圓圓心
	dRadius = sqrt(pow(cPtCen1.x - cPt1.x, 2) + pow(cPtCen1.y - cPt1.y, 2));//半徑
	if (dRadius == 0)
	{
		return;
	}
	lDistance = LONG(dRadius);
	dWidth = lWidth * 1000;
	dAng1 = acos((cPt1.x - cPtCen1.x) / dRadius);
	dAng2 = asin((cPt1.y - cPtCen1.y) / dRadius);
	cPt2.x = LONG(dRadius*cos(dAng1 + M_PI) + cPtCen1.x);
	cPt2.y = LONG(dRadius*sin(dAng2 + M_PI) + cPtCen1.y);
	m_ptVec.push_back(cPt1);
	m_ptVec.push_back(cPt2);
	lDistance = LONG(lDistance - dWidth);
	LineGetToPoint(lXClose, lYClose, cPt1.x, cPt1.y, cPtCen1.x, cPtCen1.y,
		lDistance);
	cPt3.x = lXClose;
	cPt3.y = lYClose;
	cPtCen2.x = (cPt3.x + cPt2.x) / 2;
	cPtCen2.y = (cPt3.y + cPt2.y) / 2;//下半圓圓心
	iData = (int)dRadius % (int)dWidth;
	lDistance = LONG(lDistance + dWidth);
	while (1)
	{
		lDistance = LONG(lDistance - dWidth);
		if (lDistance < dWidth)
		{
			break;
		}
		LineGetToPoint(lXClose, lYClose, cPt1.x, cPt1.y, cPtCen1.x, cPtCen1.y,
			lDistance);
		cPt3.x = lXClose;
		cPt3.y = lYClose;
		m_ptVec.push_back(cPt3);
		cPt4.x = LONG(lDistance*cos(dAng1 + M_PI) + cPtCen1.x);
		cPt4.y = LONG(lDistance*sin(dAng2 + M_PI) + cPtCen1.y);
		m_ptVec.push_back(cPt4);
	}
#pragma endregion
	//先抬升，移動到中心點在下降
	if (!g_bIsStop)
	{
		MO_Do3DLineMove(lCenX - MO_ReadLogicPosition(0), lCenY - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration,
			lInitVelociy);//直線移動至圓心
		PreventMoveError();
	}
	if (!g_bIsStop)
	{
		MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration,
			lInitVelociy);//Z軸往下
		PreventMoveError();//防止軸卡出錯
	}

	std::vector<DATA_2MOVE> DATA_2DO;
	DATA_2DO.clear();
	for (rptIter = m_ptVec.rbegin(); rptIter != m_ptVec.rend(); rptIter++)
	{
		if ((iData != 0) && (iOdd == 0))
		{
			MO_Do2dDataLine((*rptIter).x, (*rptIter).y, DATA_2DO);
		}
		else if ((iData == 0) && (iOdd == 0))
		{
			MO_Do2dDataCir((*rptIter).x, (*rptIter).y, cPtCen2.x, cPtCen2.y, bRev, DATA_2DO);
		}
		else
		{
			if (iOdd % 2 == 0)
			{
				MO_Do2dDataCir((*rptIter).x, (*rptIter).y, cPtCen2.x, cPtCen2.y, bRev, DATA_2DO);
			}
			else
			{
				MO_Do2dDataCir((*rptIter).x, (*rptIter).y, lCenX, lCenY, bRev, DATA_2DO);
			}
		}
		iOdd++;
	}
	LA_AbsToOppo2Move(DATA_2DO);
	PauseDoGlue();//暫停恢復後繼續出膠(g_bIsPause=0) 出膠
	for (UINT i = 0; i < DATA_2DO.size(); i++)
	{
		DATA_2Do[i] = DATA_2DO.at(i);
	}
	MO_DO2Curve(DATA_2Do, DATA_2DO.size(), lWorkVelociy);
	PreventMoveError();//防止驅動錯誤
	Sleep(200);
	DATA_2DO.clear();

	PauseStopGlue();//暫停時停指塗膠(g_bIsPause=1)
	MO_StopGumming();//停止出膠
	if (!g_bIsStop)
	{
		MO_Do3DLineMove(0, 0, (lZ - lZBackDistance) - lZ, lWorkVelociy, lAcceleration,
			lInitVelociy);//Z軸返回
		PreventMoveError();//防止軸卡出錯
	}
#endif
}
/*附屬---填充形態(型態3矩形填充.由外而內)
*輸入(起始點x1,y1,結束點x2,y2,寬度,驅動速度,加速度,初速度)
*/
void CAction::AttachFillType3_End(LONG &EndX, LONG &EndY, LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lWidth, LONG lWidth2)
{
#ifdef MOVE
#pragma region ****排方型內縮4點功能****
	LONG lNowX = 0, lNowY = 0;
	CPoint cPt1 = 0, cPt2 = 0, cPt3 = 0, cPt4 = 0, cPtCen = 0;
	DOUBLE dRadius = 0, dDistance = 0, dWidth = 0, dAngCenCos = 0, dAngCenSin = 0, dAngCos = 0,
		dAngSin = 0;
	DOUBLE dAngCenCos2 = 0, dAngCenSin2 = 0, dAngCos2 = 0, dAngSin2 = 0;
	std::vector<CPoint>::iterator ptIter;//迭代器
	std::vector<CPoint> m_ptVec;
	m_ptVec.clear();
	cPt1.x = lX1;
	cPt1.y = lY1;
	cPt3.x = lX2;
	cPt3.y = lY2;
	dWidth = lWidth * 1000;
	cPtCen.x = cPt1.x + (cPt3.x - cPt1.x) / 2;
	cPtCen.y = cPt1.y + (cPt3.y - cPt1.y) / 2;
	dRadius = sqrt(pow(cPt1.x - cPtCen.x, 2) + pow(cPt1.y - cPtCen.y, 2));
	if (dRadius == 0)
	{
		return;
	}
	dAngCenCos = acos(double(cPt1.x - cPtCen.x) / dRadius);
	dAngCenSin = asin(double(cPt1.y - cPtCen.y) / dRadius);
	dAngCenCos2 = M_PI * 2 - dAngCenCos;
	dAngCenSin2 = M_PI - dAngCenSin;
	if (abs(dAngCenCos - dAngCenSin) > 0.01)
	{
		if (abs(dAngCenCos - dAngCenSin2) < 0.01)
		{
			dAngCenSin = dAngCenSin2;
		}
		else if (abs(dAngCenCos2 - dAngCenSin) < 0.01)
		{
			dAngCenCos = dAngCenCos2;
		}
		else
		{
			dAngCenSin = dAngCenSin2;
			dAngCenCos = dAngCenCos2;
		}
	}
	else
	{
		dAngCenSin = dAngCenCos;
	}
	cPt2.x = LONG(dRadius*cos(dAngCenCos + M_PI_2) + cPtCen.x);
	cPt2.y = LONG(dRadius*sin(dAngCenSin + M_PI_2) + cPtCen.y);
	cPt4.x = LONG(dRadius*cos(dAngCenCos + M_PI_2 * 3) + cPtCen.x);
	cPt4.y = LONG(dRadius*sin(dAngCenSin + M_PI_2 * 3) + cPtCen.y);
	dDistance = sqrt(pow((cPt1.x - cPt4.x), 2) + pow((cPt1.y - cPt4.y), 2));
	m_ptVec.push_back(cPt1);
	m_ptVec.push_back(cPt2);
	m_ptVec.push_back(cPt3);
	m_ptVec.push_back(cPt4);
	dAngCos = acos((cPt1.x - cPt4.x) / dDistance);
	dAngSin = asin((cPt1.y - cPt4.y) / dDistance);
	if (dAngCos < 0)
	{
		dAngCos += M_PI * 2;
	}
	if (dAngSin < 0)
	{
		dAngSin += M_PI * 2;
	}
	dAngCos2 = M_PI * 2 - dAngCos;
	dAngSin2 = M_PI - dAngSin;
	if (abs(dAngCos - dAngSin) > 0.01)
	{
		if (abs(dAngCos - dAngSin2) < 0.01)
		{
			dAngSin = dAngSin2;
		}
		else if (abs(dAngCos2 - dAngSin) < 0.01)
		{
			dAngCos = dAngCos2;
		}
		else
		{
			dAngCos = dAngCos2;
			dAngSin = dAngSin2;
		}
	}
	else
	{
		dAngSin = dAngCos;
	}
	while (1)
	{
		dRadius = dRadius - dWidth*sqrt(2);
		dDistance = dDistance - dWidth;
		if (dDistance < dWidth)
		{
			break;
		}
		cPt1.x = LONG(dDistance*cos(dAngCos) + cPt4.x);
		cPt1.y = LONG(dDistance*sin(dAngSin) + cPt4.y);
		m_ptVec.push_back(cPt1);
		cPt2.x = LONG(dRadius*cos(dAngCenCos + M_PI_2) + cPtCen.x);
		cPt2.y = LONG(dRadius*sin(dAngCenSin + M_PI_2) + cPtCen.y);
		m_ptVec.push_back(cPt2);
		dDistance = dDistance - dWidth;
		if (dDistance < dWidth)
		{
			break;
		}
		cPt3.x = LONG(dRadius*cos(dAngCenCos + M_PI) + cPtCen.x);
		cPt3.y = LONG(dRadius*sin(dAngCenSin + M_PI) + cPtCen.y);
		m_ptVec.push_back(cPt3);
		cPt4.x = LONG(dRadius*cos(dAngCenCos + M_PI_2 * 3) + cPtCen.x);
		cPt4.y = LONG(dRadius*sin(dAngCenSin + M_PI_2 * 3) + cPtCen.y);
		m_ptVec.push_back(cPt4);
	}
#pragma endregion
	std::vector<DATA_2MOVE>DATA_2DO;
	DATA_2DO.clear();
	for (ptIter = m_ptVec.begin(); ptIter != m_ptVec.end(); ptIter++)
	{
		MO_Do2dDataLine((*ptIter).x, (*ptIter).y, DATA_2DO);
	}
	EndX = DATA_2DO.back().EndP.x;
	EndY = DATA_2DO.back().EndP.y;
	Sleep(1);
	DATA_2DO.clear();
#endif
}
/*附屬---填充形態(型態4矩形環)
*輸入(起始點x1,y1,結束點x2,y2,寬度,兩端寬度,驅動速度,加速度,初速度)
*/
void CAction::AttachFillType4_End(LONG &EndX, LONG &EndY, LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lWidth, LONG lWidth2)
{
#ifdef MOVE
#pragma region ****排方型內縮4點功能****
	LONG lNowX = 0, lNowY = 0;
	CPoint cPt1 = 0, cPt2 = 0, cPt3 = 0, cPt4 = 0, cPtCen = 0;
	DOUBLE dRadius = 0, dDistance = 0, dWidth = 0, dWidth2 = 0, dAngCenCos = 0,
		dAngCenSin = 0, dAngCos = 0, dAngSin = 0;
	DOUBLE dAngCenCos2 = 0, dAngCenSin2 = 0, dAngCos2 = 0, dAngSin2 = 0;
	int iBuff = 0;//判斷兩端寬度用
	std::vector<CPoint>::iterator ptIter;//迭代器
	std::vector<CPoint> m_ptVec;
	m_ptVec.clear();
	cPt1.x = lX1;
	cPt1.y = lY1;
	cPt3.x = lX2;
	cPt3.y = lY2;
	dWidth = lWidth * 1000;
	dWidth2 = lWidth2 * 1000;
	cPtCen.x = cPt1.x + (cPt3.x - cPt1.x) / 2;
	cPtCen.y = cPt1.y + (cPt3.y - cPt1.y) / 2;
	dRadius = sqrt(pow(cPt1.x - cPtCen.x, 2) + pow(cPt1.y - cPtCen.y, 2));
	if (dRadius == 0)
	{
		return;
	}
	dAngCenCos = acos(DOUBLE(cPt1.x - cPtCen.x) / dRadius);
	dAngCenSin = asin(DOUBLE(cPt1.y - cPtCen.y) / dRadius);
	dAngCenCos2 = M_PI * 2 - dAngCenCos;
	dAngCenSin2 = M_PI - dAngCenSin;
	if (abs(dAngCenCos - dAngCenSin) > 0.01)
	{
		if (abs(dAngCenCos - dAngCenSin2) < 0.01)
		{
			dAngCenSin = dAngCenSin2;
		}
		else if (abs(dAngCenCos2 - dAngCenSin) < 0.01)
		{
			dAngCenCos = dAngCenCos2;
		}
		else
		{
			dAngCenSin = dAngCenSin2;
			dAngCenCos = dAngCenCos2;
		}
	}
	else
	{
		dAngCenSin = dAngCenCos;
	}
	cPt2.x = LONG(dRadius*cos(dAngCenCos + M_PI_2) + cPtCen.x);
	cPt2.y = LONG(dRadius*sin(dAngCenSin + M_PI_2) + cPtCen.y);
	cPt4.x = LONG(dRadius*cos(dAngCenCos + M_PI_2 * 3) + cPtCen.x);
	cPt4.y = LONG(dRadius*sin(dAngCenSin + M_PI_2 * 3) + cPtCen.y);
	dDistance = sqrt(pow((cPt1.x - cPt4.x), 2) + pow((cPt1.y - cPt4.y), 2));
	m_ptVec.push_back(cPt1);
	m_ptVec.push_back(cPt2);
	m_ptVec.push_back(cPt3);
	m_ptVec.push_back(cPt4);

	dAngCos = acos((cPt1.x - cPt4.x) / dDistance);
	dAngSin = asin((cPt1.y - cPt4.y) / dDistance);
	if (dAngCos < 0)
	{
		dAngCos += M_PI * 2;
	}
	if (dAngSin < 0)
	{
		dAngSin += M_PI * 2;
	}
	dAngCos2 = M_PI * 2 - dAngCos;
	dAngSin2 = M_PI - dAngSin;
	if (abs(dAngCos - dAngSin) > 0.01)
	{
		if (abs(dAngCos - dAngSin2) < 0.01)
		{
			dAngSin = dAngSin2;
		}
		else if (abs(dAngCos2 - dAngSin) < 0.01)
		{
			dAngCos = dAngCos2;
		}
		else
		{
			dAngCos = dAngCos2;
			dAngSin = dAngSin2;
		}
	}
	else
	{
		dAngSin = dAngCos;
	}
	iBuff = 1;
	while (1)
	{
		if ((iBuff == 1) && (dRadius - (2 * dWidth2) < 0))
		{
#ifdef PRINTF
			_cwprintf(_T("兩端寬度過大 \n"));
#endif
			m_ptVec.push_back(cPt1);
			break;
		}
		dRadius = dRadius - dWidth*sqrt(2);
		dDistance = dDistance - dWidth;
		if (iBuff*dWidth >  dWidth2)  //dWidth2兩端寬度
		{
			dDistance += dWidth;
			cPt1.x = LONG(dDistance*cos(dAngCos) + cPt4.x);
			cPt1.y = LONG(dDistance*sin(dAngSin) + cPt4.y);
			m_ptVec.push_back(cPt1);
			break;
		}
		if (dDistance < dWidth)
		{
			break;
		}
		cPt1.x = LONG(dDistance*cos(dAngCos) + cPt4.x);
		cPt1.y = LONG(dDistance*sin(dAngSin) + cPt4.y);
		m_ptVec.push_back(cPt1);
		cPt2.x = LONG(dRadius*cos(dAngCenCos + M_PI_2) + cPtCen.x);
		cPt2.y = LONG(dRadius*sin(dAngCenSin + M_PI_2) + cPtCen.y);
		m_ptVec.push_back(cPt2);
		dDistance = dDistance - dWidth;
		if (dDistance < dWidth)
		{
			break;
		}
		cPt3.x = LONG(dRadius*cos(dAngCenCos + M_PI) + cPtCen.x);
		cPt3.y = LONG(dRadius*sin(dAngCenSin + M_PI) + cPtCen.y);
		m_ptVec.push_back(cPt3);
		cPt4.x = LONG(dRadius*cos(dAngCenCos + M_PI_2 * 3) + cPtCen.x);
		cPt4.y = LONG(dRadius*sin(dAngCenSin + M_PI_2 * 3) + cPtCen.y);
		m_ptVec.push_back(cPt4);
		iBuff++;
	}
#pragma endregion
	std::vector<DATA_2MOVE> DATA_2DO;
	DATA_2DO.clear();
	for (ptIter = m_ptVec.begin(); ptIter != m_ptVec.end(); ptIter++)
	{
		MO_Do2dDataLine((*ptIter).x, (*ptIter).y, DATA_2DO);
	}
	EndX = DATA_2DO.back().EndP.x;
	EndY = DATA_2DO.back().EndP.y;
	Sleep(1);
	DATA_2DO.clear();
#endif
}
/*附屬---填充形態(型態5圓環)
*輸入(起始點x1,y1,結束點x2,y2,寬度,兩端寬度,驅動速度,加速度,初速度)
*/
void CAction::AttachFillType5_End(LONG &EndX, LONG &EndY, LONG lX1, LONG lY1, LONG lCenX, LONG lCenY, LONG lWidth, LONG lWidth2)
{
#ifdef MOVE
#pragma region ****圓型螺旋功能****
	DOUBLE dRadius = 0, dWidth = 0, dWidth2 = 0, dAng0 = 0, dAng1 = 0, dAng2 = 0;
	BOOL bRev = 1;//0逆轉/1順轉
	LONG lLineClose = 0, lXClose = 0, lYClose = 0, lDistance = 0;
	LONG lNowX = 0, lNowY = 0;
	CPoint cPt1 = 0, cPt2 = 0, cPt3 = 0, cPt4 = 0, cPtCen1 = 0, cPtCen2 = 0;
	int iData = 0, iBuff = 0;//判斷是否有餘數，buff用於計數兩端寬度
	std::vector<CPoint>m_ptVec;
	std::vector<CPoint>::iterator ptIter;//迭代器
	m_ptVec.clear();
	cPt1.x = lX1;
	cPt1.y = lY1;
	cPtCen1.x = lCenX;
	cPtCen1.y = lCenY;//上半圓圓心
	dRadius = sqrt(pow(cPtCen1.x - cPt1.x, 2) + pow(cPtCen1.y - cPt1.y, 2));//半徑
	if (dRadius == 0)
	{
		return;
	}
	lDistance = LONG(dRadius);
	dWidth = lWidth * 1000;
	dWidth2 = lWidth2 * 1000;
	dAng1 = acos((cPt1.x - cPtCen1.x) / dRadius);
	dAng2 = asin((cPt1.y - cPtCen1.y) / dRadius);
	cPt2.x = LONG(dRadius*cos(dAng1 + M_PI) + cPtCen1.x);
	cPt2.y = LONG(dRadius*sin(dAng2 + M_PI) + cPtCen1.y);
	m_ptVec.push_back(cPt1);
	m_ptVec.push_back(cPt2);
	lDistance = LONG(lDistance - dWidth);
	LineGetToPoint(lXClose, lYClose, cPt1.x, cPt1.y, cPtCen1.x, cPtCen1.y,
		lDistance);
	cPt3.x = lXClose;
	cPt3.y = lYClose;
	cPtCen2.x = (cPt3.x + cPt2.x) / 2;
	cPtCen2.y = (cPt3.y + cPt2.y) / 2;//下半圓圓心
	lDistance = LONG(lDistance + dWidth);
	iData = (int)dRadius % (int)dWidth;
	iBuff = 1;
	while (1)
	{
		if ((iBuff == 1) && (lDistance - (2 * dWidth2) < 0))
		{
#ifdef PRINTF
			_cwprintf(_T("兩端寬度過大 \n"));
#endif
			break;
		}
		dRadius = dRadius - dWidth;
		lDistance = LONG(lDistance - dWidth);
		if (iBuff * dWidth > dWidth2)
		{
			lDistance += LONG(dWidth);
			LineGetToPoint(lXClose, lYClose, cPt1.x, cPt1.y, cPtCen1.x, cPtCen1.y,
				lDistance);
			cPt3.x = lXClose;
			cPt3.y = lYClose;
			m_ptVec.push_back(cPt3);
			break;
		}
		LineGetToPoint(lXClose, lYClose, cPt1.x, cPt1.y, cPtCen1.x, cPtCen1.y,
			lDistance);
		cPt3.x = lXClose;
		cPt3.y = lYClose;
		m_ptVec.push_back(cPt3);
		cPt4.x = LONG(dRadius*cos(dAng1 + M_PI) + cPtCen1.x);
		cPt4.y = LONG(lDistance*sin(dAng2 + M_PI) + cPtCen1.y);
		m_ptVec.push_back(cPt4);
		iBuff++;
	}
#pragma endregion
	std::vector<DATA_2MOVE> DATA_2DO;
	DATA_2DO.clear();
	for (UINT i = 1; i < m_ptVec.size(); i++)
	{
		if (i == m_ptVec.size() - 1)
		{
			if (i % 2 == 0)
			{
				MO_Do2dDataCir(m_ptVec.at(i).x, m_ptVec.at(i).y, cPtCen1.x, cPtCen1.y, bRev, DATA_2DO);//上半圓
			}
			else
			{
				MO_Do2dDataCir(m_ptVec.at(i).x, m_ptVec.at(i).y, cPtCen2.x, cPtCen2.y, bRev, DATA_2DO);//下半圓
			}
		}
		else
		{
			if (i % 2 != 0)
			{
				MO_Do2dDataCir(m_ptVec.at(i).x, m_ptVec.at(i).y, cPtCen1.x, cPtCen1.y, bRev, DATA_2DO);//上半圓
			}
			else
			{
				MO_Do2dDataCir(m_ptVec.at(i).x, m_ptVec.at(i).y, cPtCen2.x, cPtCen2.y, bRev, DATA_2DO);//下半圓
			}
		}
	}
	EndX = DATA_2DO.back().EndP.x;
	EndY = DATA_2DO.back().EndP.y;
	Sleep(1);
	DATA_2DO.clear();
#endif
}
/***********************************************************
**                                                        **
**          運動模組-連續差補.                             **
**                                                        **
************************************************************/
#ifdef MOVE
//填充用兩軸連續差補(給值--直線)
void CAction::MO_Do2dDataLine(LONG EndPX, LONG EndPY, std::vector<DATA_2MOVE>& str)
{
	DATA_2MOVE DATA_2D;
	DATA_2D.EndP.x = EndPX;
	DATA_2D.EndP.y = EndPY;
	DATA_2D.Type = 0;//直線
	DATA_2D.Speed = 0;
	DATA_2D.CirCentP.x = 0;
	DATA_2D.CirCentP.y = 0;
	DATA_2D.CirRev = 0;
	str.push_back(DATA_2D);
}
//填充用兩軸連續差補(給值--圓)
void CAction::MO_Do2dDataCir(LONG EndPX, LONG EndPY, LONG CenX, LONG CenY, BOOL bRev, std::vector<DATA_2MOVE>& str)
{
	DATA_2MOVE DATA_2D;
	DATA_2D.Type = 1;//圓
	DATA_2D.EndP.x = EndPX;
	DATA_2D.EndP.y = EndPY;
	DATA_2D.CirCentP.x = CenX;
	DATA_2D.CirCentP.y = CenY;
	DATA_2D.CirRev = bRev;
	str.push_back(DATA_2D);
}
/*絕對座標轉相對座標3軸連續插補使用*/
void CAction::LA_AbsToOppo3Move(std::vector<DATA_3MOVE> &str)
{
#ifdef LA
	std::vector<DATA_3MOVE> vecBuf;
	DATA_3MOVE mData;
	LONG lNowX = MO_ReadLogicPosition(0), lNowY = MO_ReadLogicPosition(1), lNowZ = MO_ReadLogicPosition(2);
	vecBuf.clear();
	mData.EndPX = str.at(0).EndPX - lNowX;
	mData.EndPY = str.at(0).EndPY - lNowY;
	mData.EndPZ = str.at(0).EndPZ - lNowZ;
	mData.Distance = LONG(sqrt(pow(str.at(0).EndPX, 2) + pow(str.at(0).EndPY, 2)));
	vecBuf.push_back(mData);
	for (UINT i = 1; i < str.size(); i++)
	{
		if ((str.at(i).EndPX == LA_SCANEND) && (str.at(i).EndPY == LA_SCANEND) && (str.at(i).EndPZ == LA_SCANEND))
		{
			mData.EndPX = str.at(i).EndPX;
			mData.EndPY = str.at(i).EndPY;
			mData.EndPZ = str.at(i).EndPZ;
			//vecBuf.push_back(mData);
			//如果-99999不是最後一筆資料
			if (i + 1 < str.size())
			{
				mData.EndPX = str.at(i + 1).EndPX - str.at(i - 1).EndPX;
				mData.EndPY = str.at(i + 1).EndPY - str.at(i - 1).EndPY;
				mData.EndPZ = str.at(i + 1).EndPZ - str.at(i - 1).EndPZ;
				vecBuf.push_back(mData);
			}
			i++;
		}
		else
		{
			mData.Speed = str.at(i).Speed;
			mData.EndPX = str.at(i).EndPX - str.at(i - 1).EndPX;
			mData.EndPY = str.at(i).EndPY - str.at(i - 1).EndPY;
			mData.EndPZ = str.at(i).EndPZ - str.at(i - 1).EndPZ;
			mData.Distance = LONG(sqrt(pow(mData.EndPX, 2) + pow(mData.EndPY, 2)));
			vecBuf.push_back(mData);
		}

	}
	str.clear();
	str.insert(str.end(), vecBuf.begin(), vecBuf.end());
	for (UINT i = 0; i < str.size() - 1; i++)
	{
		DATA_3Do[i] = { 0 };
		DATA_3Do[i] = str[i + 1];
	}
#endif
}
#endif
/*絕對座標轉相對座標2軸連續插補使用*/
#ifdef MOVE
void CAction::LA_AbsToOppo2Move(std::vector<DATA_2MOVE>& str)
{
	std::vector<DATA_2MOVE> vecBuf;
	DATA_2MOVE mData;
	vecBuf.clear();
	mData.Type = str.at(0).Type;
	mData.EndP.x = str.at(0).EndP.x - MO_ReadLogicPosition(0);
	mData.EndP.y = str.at(0).EndP.y - MO_ReadLogicPosition(1);
	mData.CirCentP.x = str.at(0).CirCentP.x - MO_ReadLogicPosition(0);
	mData.CirCentP.y = str.at(0).CirCentP.y - MO_ReadLogicPosition(1);
	mData.CirRev = str.at(0).CirRev;
	mData.Distance = (LONG)round(sqrt(pow(str.at(0).EndP.x, 2) + pow(str.at(0).EndP.y, 2)));
	mData.Speed = 0;
	vecBuf.push_back(mData);
	for (UINT i = 1; i < str.size(); i++)
	{
		mData.Speed = str.at(i).Speed;
		mData.Type = str.at(i).Type;
		mData.EndP = str.at(i).EndP - str.at(i - 1).EndP;
		mData.Distance = (LONG)round(sqrt(pow(mData.EndP.x, 2) + pow(mData.EndP.y, 2)));
		if (str.at(i).Type)
		{
			mData.CirCentP = str.at(i).CirCentP - str.at(i - 1).EndP;
			mData.CirRev = str.at(i).CirRev;
		}
		else
		{
			mData.CirCentP = 0;
			mData.CirRev = 0;
		}
		vecBuf.push_back(mData);
	}
	str.clear();
	str.insert(str.end(), vecBuf.begin(), vecBuf.end());
}
#endif