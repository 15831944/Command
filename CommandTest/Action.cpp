/*
*檔案名稱:Action.cpp(3D用)
*內容簡述:運動命令API，詳細參數請查看excel
*＠author 作者名稱:R
*＠data 更新日期:2016/06/07	
*@更新內容線段z值改變時，三軸同動移動，原來的為x,y先移動再移動z軸*/
#include "stdafx.h"
#include "Action.h"
/***********************************************************
**                                                        **
**          運動模組-運動指令命令 (對應動作判斷)             **
**                                                        **
************************************************************/
CAction::CAction()
{
    pAction = this;
    g_bIsPause = FALSE;//暫停
    g_bIsStop = FALSE;//停止
    g_bIsDispend = TRUE;//點膠機開
    g_iNumberGluePort = 1;//使用一個點膠埠
}
CAction::~CAction()
{
}
/***************************************************************************************************************運動API*/
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
    if (!bZDisType)//絕對位置
    {
        if (lZBackDistance > lZ)
        {
            return;
        }
        lZBackDistance = abs(lZBackDistance - lZ);
    }
    if (!g_bIsStop)
    {
        MO_Do3DLineMove(lX - lNowX, lY - lNowY, 0, lWorkVelociy, lAcceleration, lInitVelociy);//移動到點膠點
        PreventMoveError();//防止軸卡出錯
    }
    if (!g_bIsStop)
    {
        MO_Do3DLineMove(0, 0, lZ - lNowZ, lWorkVelociy, lAcceleration, lInitVelociy);//移動到點膠點
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
            DoGlue(lDoTime, lDelayStopTime, GummingTimeOutThread);//執行點膠
        }
    }
    if (lZBackDistance > lZ) 
    { 
        return;
    }
    if (lZBackDistance == 0)
    {
        Sleep(1);
    }
    else
    {
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
            if (lZdistance >  lZBackDistance)
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
        MO_Do3DLineMove(lX - MO_ReadLogicPosition(0), lY - MO_ReadLogicPosition(1), lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//x,y,z軸移動
        PreventMoveError();//防止軸卡出錯
    }
    if (g_bIsDispend == 1)
    {
        MO_GummingSet(g_iNumberGluePort, 0, GummingTimeOutThread);//塗膠(不卡)
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
    if (!bZDisType)//絕對位置
    {
        lZBackDistance = abs(lZBackDistance - lZ);
    }
    LONG lNowX = 0, lNowY = 0, lNowZ = 0;
    LONG lLineClose = 0, lXClose = 0, lYClose = 0;
    PauseDoGlue();//暫停恢復後繼續出膠(g_bIsPause=0)
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
*輸入(線段結束點、線段點膠設定、加速度、線速度、Z軸工作高度設定、點膠結束設定、返回設定、系統參數)
*執行Z軸回升型態1
*/
void CAction::DecideLineEndMove(LONG lCloseOffDelayTime,
    LONG lCloseONDelayTime, LONG lZBackDistance, BOOL bZDisType, LONG lHighVelocity, LONG lDistance, LONG lHigh,
    LONG lLowVelocity, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
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
#ifdef MOVE
    LONG lNowX = 0, lNowY = 0, lNowZ = 0;
    LONG lLineClose = 0, lXClose = 0, lYClose = 0;
    PauseDoGlue();//暫停恢復後繼續出膠(g_bIsPause=0)
    lNowX = MO_ReadLogicPosition(0);
    lNowY = MO_ReadLogicPosition(1);
    lNowZ = MO_ReadLogicPosition(2);
    if (!bZDisType)//絕對位置
    {
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
    MO_StopGumming();//停止出膠
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
void CAction::DecideLineSToP(LONG lX, LONG lY, LONG lZ, LONG lX2, LONG lY2, LONG lZ2, LONG lStartDelayTime, LONG lStartDistance, LONG lMidDelayTime, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
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
    DOUBLE dTime = 0;
    int iBuf = 0;
    dTime = (sqrt((DOUBLE)lInitVelociy*(DOUBLE)lInitVelociy + ((DOUBLE)lAcceleration*(DOUBLE)lStartDistance / 2)) - (DOUBLE)lInitVelociy) / (DOUBLE)lAcceleration;
    lNowX = MO_ReadLogicPosition(0);
    lNowY = MO_ReadLogicPosition(1);
    lLineClose = lStartDistance;
    LineGetToPoint(lXClose, lYClose, lZClose, lX2, lY2, lX, lY,lZ2,lZ, lLineClose);
    lBuffX = (-(lXClose - lX)) + lX;
    lBuffY = (-(lYClose - lY)) + lY;
    iBuf = lStartDelayTime ^ lStartDistance;//互斥或移動前延遲跟設置距離 如果兩者都相同結果為0(當兩者都有值時以"移動前延遲"優先)
    if (iBuf == 0 || lStartDistance == 0)
    {
        if (!g_bIsStop)
        {
            MO_Do3DLineMove(lX - MO_ReadLogicPosition(0), lY - MO_ReadLogicPosition(1), lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//x,y,z軸移動
            PreventMoveError();//防止軸卡出錯
        }
        if (g_bIsDispend == 1)
        {
            MO_GummingSet(g_iNumberGluePort, 0, GummingTimeOutThread);//塗膠(不卡)
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
            MO_Do3DLineMove(lBuffX - MO_ReadLogicPosition(0), lBuffY - MO_ReadLogicPosition(1), lZClose - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//x,y軸
            PreventMoveError();//防止軸卡出錯
        }
        if (!g_bIsStop)
        {
            MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//z軸先下降
            PreventMoveError();//防止軸卡出錯
        }
        MO_TimerSetIntter(dTime * 1000000, LPInterrupt);//計時到跳至執行序
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
void CAction::DecideLineSToE(LONG lX, LONG lY, LONG lZ, LONG lX2, LONG lY2, LONG lZ2, LONG lStartDelayTime, LONG lStartDistance, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime, LONG lZBackDistance, BOOL bZDisType, LONG lHighVelocity, LONG lDistance, LONG lHigh, LONG lLowVelocity, int iType, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
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
    LONG lBuffX = 0, lBuffY = 0;
    DOUBLE dTime = 0;
    int iBuf = 0;
    dTime = (sqrt((DOUBLE)lInitVelociy*(DOUBLE)lInitVelociy + ((DOUBLE)lAcceleration*(DOUBLE)lStartDistance / 2)) - (DOUBLE)lInitVelociy) / (DOUBLE)lAcceleration;
    lNowZ = MO_ReadLogicPosition(2);
    lLineClose = lStartDistance;
    LineGetToPoint(lXClose, lYClose, lZClose, lX2, lY2, lX, lY, lZ2, lZ, lLineClose);
    lBuffX = (-(lXClose - lX)) + lX;
    lBuffY = (-(lYClose - lY)) + lY;
    if (!bZDisType)//絕對位置
    {
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
    iBuf = lStartDelayTime ^ lStartDistance;//互斥或移動前延遲跟設置距離 如果兩者都相同結果為0(當兩者都有值時以"移動前延遲"優先)
    if (iBuf == 0 || lStartDistance == 0)
    {
        if (!g_bIsStop)
        {
            MO_Do3DLineMove(lX - MO_ReadLogicPosition(0), lY - MO_ReadLogicPosition(1), lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//x,y軸移動
            PreventMoveError();//防止軸卡出錯
        }
        if (g_bIsDispend == 1)
        {
            MO_GummingSet(g_iNumberGluePort, 0, GummingTimeOutThread);//塗膠(不卡)
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
            MO_Do3DLineMove(lBuffX - MO_ReadLogicPosition(0), lBuffY - MO_ReadLogicPosition(1), lZClose - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//x,y,Z軸移動
            PreventMoveError();//防止軸卡出錯
        }
        if (lCloseDistance == 0)//線段點膠設定---(5)關機距離
        {
            MO_TimerSetIntter(dTime * 1000000, LPInterrupt);//計時到跳至執行序
            if (!g_bIsStop)
            {
                MO_Do3DLineMove(lX2 - MO_ReadLogicPosition(0), lY2 - MO_ReadLogicPosition(1), lZ2 - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//x,y,Z軸移動
                PreventMoveError();//防止軸卡出錯
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
    LONG lR = 0;//斷膠點與半徑
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
    lR = sqrt(pow(lNowX - lCircleX, 2) + pow(lNowY - lCircleY, 2));//半徑
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
        lZBackDistance = abs(lZBackDistance - lNowZ);
    }
    PauseDoGlue();//暫停恢復後繼續出膠(g_bIsPause=0)
    if (lNowX == lX3 && lNowY == lY3)//表示結束點在起始點上
    {
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
            lCloseDistance = (2 * M_PI*lR) - lCloseDistance;
            ArcGetToPoint(lEndX, lEndY, lCloseDistance, lX3, lY3, lCircleX, lCircleY, lR, bRev);//算出斷膠點
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
    lR = sqrt(pow(lNowX - lCircleX, 2) + pow(lNowY - lCircleY, 2));//半徑
    dAngl = AngleCount(lCircleX, lCircleY, lX1, lY1, lX2, lY2, bRev);//角度
    lCrev = 2 * lR * M_PI * dAngl / 360;
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
        lCloseDistance = (2 * M_PI*lR) - lCloseDistance;
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
void CAction::DecideParkPoint(LONG lX, LONG lY, LONG lZ, LONG lTimeGlue, LONG lWaitTime, LONG lStayTime, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
    /*停駐點(x座標，y座標，z座標，排膠時間，結束後等待時間，)
    LONG lX, LONG lY, LONG lZ,LONG lTimeGlue,LONG lWaitTime
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
        DoGlue(lTimeGlue, lStayTime, GummingTimeOutThread);//執行排膠
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
*填充動作
*輸入(線段開始，線段結束，Z軸工作高度設定，填充命令，系統參數)
*/
void CAction::DecideFill(LONG lX1, LONG lY1, LONG lZ1, LONG lX2, LONG lY2, LONG lZ2, LONG lZBackDistance, BOOL bZDisType, int iType, LONG lWidth, LONG lWidth2, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
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
#ifdef MOVE
    LONG lNowX = 0, lNowY = 0, lNowZ = 0;


    lNowX = MO_ReadLogicPosition(0);
    lNowY = MO_ReadLogicPosition(1);
    lNowZ = MO_ReadLogicPosition(2);

    if (!g_bIsStop)
    {
        MO_Do3DLineMove(lX1 - lNowX, lY1 - lNowY, lZ1 - lNowZ, lWorkVelociy, lAcceleration, lInitVelociy);//回到起始點!
        PreventMoveError();
    }
    if (!bZDisType)//絕對位置
    {
        lZBackDistance = abs(lZBackDistance - lZ1);
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
            AttachFillType1(lX1, lY1, lX2, lY2, lZ1, lZBackDistance, lWidth, lWorkVelociy, lAcceleration, lInitVelociy);
            break;
        }
        case 2:
        {
            AttachFillType2(lX1, lY1, lX2, lY2, lZ1, lZBackDistance, lWidth, lWorkVelociy, lAcceleration, lInitVelociy);
            break;
        }
        case 3:
        {
            AttachFillType3(lX1, lY1, lX2, lY2, lZ1, lZBackDistance, lWidth, lWorkVelociy, lAcceleration, lInitVelociy);
            break;
        }
        case 4:
        {
            AttachFillType4(lX1, lY1, lX2, lY2, lZ1, lZBackDistance, lWidth, lWidth2, lWorkVelociy, lAcceleration, lInitVelociy);
            break;
        }
        case 5:
        {
            AttachFillType5(lX1, lY1, lX2, lY2, lZ1, lZBackDistance, lWidth, lWidth2, lWorkVelociy, lAcceleration, lInitVelociy);
            break;
        }
        case 6:
        {
            AttachFillType6(lX1, lY1, lX2, lY2, lZ1, lZBackDistance, lWidth, lWorkVelociy, lAcceleration, lInitVelociy);
            break;
        }
        case 7:
        {
            AttachFillType7(lX1, lY1, lX2, lY2, lZ1, lZBackDistance, lWidth, lWorkVelociy, lAcceleration, lInitVelociy);
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

/***************************************************************************************************************自行運用函數*/
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
        MO_Do3DLineMoveInterr(lX - lNowX, lY - lNowY, lZ - lNowZ, lWorkVelociy, lAcceleration, lInitVelociy, LPInterrupt);//執行執行緒中斷
        PreventMoveError();//防止軸卡出錯
    }
    else
    {
        if (!g_bIsStop)
        {
            MO_Do3DLineMove(lX - lNowX, lY - lNowY, lZ - lNowZ, lWorkVelociy, lAcceleration, lInitVelociy);
            PreventMoveError();//防止軸卡出錯
        }
    }
#endif 
}
/*
*出膠(配合執行緒使用)
*輸入(點膠機開啟or點膠機關閉)解譯後的LONG值
*/
void CAction::DoGlue(LONG lTime, LONG lDelayTime, LPTHREAD_START_ROUTINE GummingTimeOutThread)
{
#ifdef MOVE
    if (!g_bIsStop)
    {
        if (g_bIsDispend == 1)
        {
            MO_GummingSet(g_iNumberGluePort, lTime * 1000, GummingTimeOutThread);
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
*執行緒
*中斷時執行完成點膠。
*/
DWORD CAction::GummingTimeOutThread(LPVOID)
{
#ifdef MOVE
    MO_FinishGumming();
#endif
    return 0;
}
/*
*執行緒
*中斷時執行塗膠。
*/
DWORD CAction::LPInterrupt(LPVOID)
{
#ifdef MOVE
    if (((CAction*)pAction)->g_bIsDispend == 1)
    {
        MO_GummingSet(((CAction*)pAction)->g_iNumberGluePort, 0, GummingTimeOutThread);//塗膠(不卡)
    }
#endif
    return 0;
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
#endif
}
/*
*暫停回復後繼續塗膠
*讀取暫停參數，當參數為0時出膠，且點膠機要為開。
*/
void CAction::PauseDoGlue()
{
#ifdef MOVE
    if (!g_bIsPause) //暫停恢復後繼續出膠(g_bIsPause=0)
    {
        if ((g_bIsPause == 0) && (g_bIsDispend == 1))
        {
            MO_GummingSet(g_iNumberGluePort, 0, GummingTimeOutThread);//塗膠(不卡)
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
    lR = sqrt(pow((DOUBLE)(lXarEnd - lXarUp), 2) + pow((DOUBLE)(lYarEnd - lYarUp), 2));
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
    DOUBLE dM, dB; //(X0,Y0)起始點; (X1,Y1)結束點; Y0=M*X0+B 斜率公式;L已知長度
    if ((lX1 - lX0) == 0)
    {
        dM = 0;
    }
    else
    {
        dM = ((float)lY1 - (float)lY0) / ((float)lX1 - (float)lX0);
    }
    dB = lY0 - (dM*lX0);
    LONG la = lX1 - lX0, lb = lY1 - lY0;
    if (la == 0)
    {
        if (lY0 > lY1)
        {
            lYClose = lY1 + lLineClose;
            lXClose = lX1;
        }
        else
        {
            lYClose = lY1 - lLineClose;
            lXClose = lX1;
        }
    }
    else if (lb == 0)
    {
        if (lX0 > lX1)
        {
            lXClose = lX1 + lLineClose;
            lYClose = lY1;
        }
        else
        {
            lXClose = lX1 - lLineClose;
            lYClose = lY1;
        }
    }
    else
    {
        if (abs(la) > abs(lb))
        {
            if (la > 0)
            {
                lXClose = lX1 - lLineClose;
                lYClose = (dM*lX1) + dB;
            }
            else
            {
                lXClose = lX1 + lLineClose;
                lYClose = (dM*lX1) + dB;
            }
        }
        else if (abs(la) == abs(lb))
        {
            if (la > 0)
            {
                lXClose = lX1 - ((float)lLineClose / sqrt(2));
                lYClose = lY1 - ((float)lLineClose / sqrt(2));
            }
            else
            {
                lXClose = lX1 + ((float)lLineClose / sqrt(2));
                lYClose = lY1 + ((float)lLineClose / sqrt(2));
            }
        }
        else if (abs(la) < abs(lb))
        {
            if (lb > 0)
            {
                lYClose = lY1 - lLineClose;
                lXClose = (lYClose - dB) / dM;
            }
            else
            {
                lYClose = lY1 + lLineClose;
                lXClose = (lYClose - dB) / dM;
            }
        }
    }
}
/*
*直線距離轉換成座標點--多載
*/
void CAction::LineGetToPoint(LONG &lXClose, LONG &lYClose, LONG &lZClose, LONG lX0, LONG lY0, LONG lX1, LONG lY1, LONG lZ0, LONG lZ1, LONG &lLineClose)
{
    LONG lLength = 0;
    lLength = sqrt(pow(lX0 - lX1, 2) + pow(lY0 - lY1, 2) + pow(lZ0 - lZ1, 2));
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
    int h = round((e*c - b*f) / (a*e - b*d));
    int k = round((a*f - d*c) / (a*e - b*d));
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
    //offsetx=offset x,offsety=offset y,sum=Θ,
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
        lArcX = (((DOUBLE)lX - (DOUBLE)lCenX)*cos(dAngl)) + (((DOUBLE)lY - (DOUBLE)lCenY)*sin(dAngl)) + lCenX;
        lArcY = (((DOUBLE)lX - (DOUBLE)lCenX)*sin(dAngl)) + (((DOUBLE)lY - (DOUBLE)lCenY)*cos(dAngl)) + lCenY;
    }
    else
    {
        lArcX = (((DOUBLE)lX - (DOUBLE)lCenX)*cos(dAngl)) - (((DOUBLE)lY - (DOUBLE)lCenY)*sin(dAngl)) + lCenX;
        lArcY = (((DOUBLE)lX - (DOUBLE)lCenX)*sin(dAngl)) + (((DOUBLE)lY - (DOUBLE)lCenY)*cos(dAngl)) + lCenY;
    }
}


/*附屬---填充形態(型態1矩形s路徑填充)
*輸入(起始點x1,y1,結束點x2,y2,寬度,驅動速度,加速度,初速度)
*/
void CAction::AttachFillType1(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lZ, LONG lZBackDistance, LONG lWidth, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
#ifdef MOVE
    LONG lBuffX = 0, lBuffY = 0, lBuffMove = 0;
    LONG lPointX1 = 0, lPointY1 = 0, lPointX2 = 0, lPointY2 = 0;
    LONG lLineClose = 0, lXClose = 0, lYClose = 0;
    LONG lLingX1 = 0, lLingY1 = 0;
    LONG lNowX = 0, lNowY = 0;
    BOOL bNum1 = 0;
    int iType1 = 0, iCnt1 = 0, iData1 = 0;
    if ((lX1 == lX2) || (lY1 == lY2))//起始點與結束點為一直線
    {
        if ((lX2>lX1) == 1 || (lY2>lY1))//iType1 == 1 || iType1 == 3
        {
            bNum1 = 1;
        }
        else  //((lX2<lX1) || (lY2<lY1))iType1 == 2 || iType1 == 4
        {
            bNum1 = 0;
        }
        if ((lX2 > lX1) || (lX2 < lX1))//iType1 == 1 || iType1 == 2
        {
            lPointX1 = lX1 + (lX2 - lX1) / 2;
            lPointY1 = lY1 + (abs(lX2 - lX1) / 2);
            lPointX2 = lPointX1;
            lPointY2 = lY1 - (abs(lX2 - lX1) / 2);
        }
        else //((lY2 > lY1) || (lY2 < lY1))iType1 == 3 || iType1 == 4
        {
            lPointY1 = lY1 + ((lY2 - lY1) / 2);
            lPointX1 = lX1 + (abs(lY2 - lY1) / 2);
            lPointX2 = lX1 - (abs(lY2 - lY1) / 2);
            lPointY2 = lPointY1;
        }
        if (lPointY2 < 0 || lPointX2 < 0) //判斷矩形四個點是否超過負極限
        {
            AfxMessageBox(L"超出極限範圍");
            g_bIsStop = 1;//停止動作
        }
        lBuffMove = abs(lPointX1 - lX1)*sqrt(2);//移動量
        iCnt1 = abs(lBuffMove) / (lWidth * 1000);  //執行次數
        iData1 = abs(lBuffMove) % (lWidth * 1000); //補償的寬度
        lBuffX = abs(lPointX1 - lX1);
        lBuffY = abs(lPointY1 - lY1);//X移動量
        lLineClose = (lWidth * 1000);
        LineGetToPoint(lXClose, lYClose, lPointX1, lPointY1, lX1, lY1, lLineClose);
        lLingX1 = abs(lXClose - lX1);
        lLingY1 = abs(lYClose - lY1);//Y移動量

        PauseDoGlue();//暫停恢復後繼續出膠(g_bIsPause=0)出膠
        for (int i = 1; i <= iCnt1; i++)
        {
            if ((lX2 > lX1) || (lX2 < lX1))//iType1 == 1 || iType1 == 2
            {
                if (i % 2 == bNum1)// (bNum1=1)X2>X1 則X往右移動/(bNum1=0) X2<X1 則X往左移動   1+2-
                {
                    if (!g_bIsStop)
                    {
                        MO_Do3DLineMove(lBuffX, lBuffY, 0, lWorkVelociy, lAcceleration, lInitVelociy);//X往右下移動(+)
                        PreventMoveError();//防止軸卡出錯
                    }
                }
                else
                {
                    if (!g_bIsStop)
                    {
                        MO_Do3DLineMove((~lBuffX), (~lBuffY), 0, lWorkVelociy, lAcceleration, lInitVelociy);//X往左上移動(-)
                        PreventMoveError();//防止軸卡出錯
                    }
                }
            }
            else//((lY2 > lY1) || (lY2 < lY1))iType1 == 3 || iType1 == 4
            {
                if (i % 2 == bNum1)// (bNum1=1)X2>X1 則X往右移動/(bNum1=0) X2<X1 則X往左移動   3+4-
                {
                    if (!g_bIsStop)
                    {
                        MO_Do3DLineMove((~lBuffX), lBuffY, 0, lWorkVelociy, lAcceleration, lInitVelociy);//x往左下移動(x-,y+)
                        PreventMoveError();//防止軸卡出錯
                    }
                }
                else
                {
                    if (!g_bIsStop)
                    {
                        MO_Do3DLineMove(lBuffX, (~lBuffY), 0, lWorkVelociy, lAcceleration, lInitVelociy);//x往右上移動(x+,y-)
                        PreventMoveError();//防止軸卡出錯
                    }
                }
            }
            if (lX2>lX1)//iType1 = 1;
            {
                if (!g_bIsStop)
                {
                    MO_Do3DLineMove(lLingX1, (~lLingY1), 0, lWorkVelociy, lAcceleration, lInitVelociy);//Y往右上移動(x+,y-)
                    PreventMoveError();//防止軸卡出錯
                }
            }
            else if (lX2<lX1)//iType1 = 2;
            {
                if (!g_bIsStop)
                {
                    MO_Do3DLineMove((~lLingX1), lLingY1, 0, lWorkVelociy, lAcceleration, lInitVelociy);//Y左下移動(x-,y+)
                    PreventMoveError();//防止軸卡出錯
                }
            }
            else if (lY2>lY1)//iType1 = 3;
            {
                if (!g_bIsStop)
                {
                    MO_Do3DLineMove(lLingX1, lLingY1, 0, lWorkVelociy, lAcceleration, lInitVelociy);//Y往右下移動(+)
                    PreventMoveError();//防止軸卡出錯
                }
            }
            else if (lY2<lY1)//iType1 = 4;
            {
                if (!g_bIsStop)
                {
                    MO_Do3DLineMove((~lLingX1), (~lLingY1), 0, lWorkVelociy, lAcceleration, lInitVelociy);//Y左上移動(-)
                    PreventMoveError();//防止軸卡出錯
                }
            }
            if (i == iCnt1) //最後一次，判斷是否需要補償
            {
                if ((lX2 > lX1) || (lX2 < lX1))//iType1 == 1 || iType1 == 2
                {
                    if (i % 2 != bNum1)// (bNum1=1)X2>X1 則X往右移動/(bNum1=0) X2<X1 則X往左移動   1+2-
                    {
                        if (!g_bIsStop)
                        {
                            MO_Do3DLineMove(lBuffX, lBuffY, 0, lWorkVelociy, lAcceleration, lInitVelociy);//X往右下移動(+)
                            PreventMoveError();//防止軸卡出錯
                        }
                    }
                    else
                    {
                        if (!g_bIsStop)
                        {
                            MO_Do3DLineMove((~lBuffX), (~lBuffY), 0, lWorkVelociy, lAcceleration, lInitVelociy);//X往左上移動(-)
                            PreventMoveError();//防止軸卡出錯
                        }
                    }
                }
                else//((lY2 > lY1) || (lY2 < lY1))iType1 == 3 || iType1 == 4
                {
                    if (i % 2 != bNum1)// (bNum1=1)X2>X1 則X往右移動/(bNum1=0) X2<X1 則X往左移動   3+4-
                    {
                        if (!g_bIsStop)
                        {
                            MO_Do3DLineMove((~lBuffX), lBuffY, 0, lWorkVelociy, lAcceleration, lInitVelociy);//x往左下移動(x-,y+)
                            PreventMoveError();//防止軸卡出錯
                        }
                    }
                    else
                    {
                        if (!g_bIsStop)
                        {
                            MO_Do3DLineMove(lBuffX, (~lBuffY), 0, lWorkVelociy, lAcceleration, lInitVelociy);//x往右上移動(x+,y-)
                            PreventMoveError();//防止軸卡出錯
                        }
                    }
                }
                if (iData1 != 0 && i % 2 == 1)
                {
                    lNowX = MO_ReadLogicPosition(0);
                    lNowY = MO_ReadLogicPosition(1);
                    if ((lX2 > lX1) || (lY2 < lY1))//iType1 = 1,4;
                    {
                        if (!g_bIsStop)
                        {
                            MO_Do3DLineMove(lPointX2 - lNowX, lPointY2 - lNowY, 0, lWorkVelociy, lAcceleration, lInitVelociy);//移動到(px2,py2)
                            PreventMoveError();//防止軸卡出錯
                        }
                    }
                    else if ((lX2<lX1) || (lY2>lY1))//iType1 = 2,3;
                    {
                        if (!g_bIsStop)
                        {
                            MO_Do3DLineMove(lPointX1 - lNowX, lPointY1 - lNowY, 0, lWorkVelociy, lAcceleration, lInitVelociy);//移動到(px1,py1)
                            PreventMoveError();//防止軸卡出錯
                        }
                    }
                }
                lNowX = MO_ReadLogicPosition(0);
                lNowY = MO_ReadLogicPosition(1);
                if (!g_bIsStop)
                {
                    MO_Do3DLineMove(lX2 - lNowX, lY2 - lNowY, 0, lWorkVelociy, lAcceleration, lInitVelociy);//移動到結束點
                    PreventMoveError();//防止軸卡出錯
                }

            }
        }

    }
    else //起始點與結束點為斜線
    {
        lBuffX = abs(lX2 - lX1);//X移動量
        lBuffY = abs(lY2 - lY1);//Y移動量
        iCnt1 = abs(lBuffY) / (lWidth * 1000);  //執行次數
        iData1 = abs(lBuffY) % (lWidth * 1000); //補償的寬度
        if (lX2>lX1 && lY2>lY1)
        {
            iType1 = 1;
        }
        else if (lX2>lX1 && lY2<lY1)
        {
            iType1 = 2;
        }
        else if (lX2<lX1 &&lY2>lY1)
        {
            iType1 = 3;
        }
        else if (lX2<lX1 &&lY2<lY1)
        {
            iType1 = 4;
        }

        if (iType1 == 1 || iType1 == 2)
        {
            bNum1 = 1;
        }
        else if (iType1 == 3 || iType1 == 4)
        {
            bNum1 = 0;
        }

        for (int i = 1; i <= iCnt1; i++)
        {
            if (i % 2 == bNum1)// (bNum1=1)X2>X1 則X往右移動/(bNum1=0) X2<X1 則X往左移動
            {
                if (!g_bIsStop)
                {
                    MO_Do3DLineMove(lBuffX, 0, 0, lWorkVelociy, lAcceleration, lInitVelociy);//X往右移動(+)
                    PreventMoveError();//防止軸卡出錯
                }
            }
            else
            {
                if (!g_bIsStop)
                {
                    MO_Do3DLineMove((~lBuffX), 0, 0, lWorkVelociy, lAcceleration, lInitVelociy);//X往左移動(-)
                    PreventMoveError();//防止軸卡出錯
                }
            }
            if (iType1 == 1 || iType1 == 3)
            {
                if (!g_bIsStop)
                {
                    MO_Do3DLineMove(0, (lWidth * 1000), 0, lWorkVelociy, lAcceleration, lInitVelociy);//Y往下移動(+)
                    PreventMoveError();//防止軸卡出錯
                }
            }
            else
            {
                if (!g_bIsStop)
                {
                    MO_Do3DLineMove(0, (~lWidth * 1000), 0, lWorkVelociy, lAcceleration, lInitVelociy);//Y往上移動(-)
                    PreventMoveError();//防止軸卡出錯
                }
            }
            if (i == iCnt1) //最後一次，判斷是否需要補償
            {
                if (i % 2 != bNum1)// (bNum1=1)X2>X1 則X往右移動/(bNum1=0) X2<X1 則X往左移動
                {
                    if (!g_bIsStop)
                    {
                        MO_Do3DLineMove(lBuffX, 0, 0, lWorkVelociy, lAcceleration, lInitVelociy);//X往右移動(+)
                        PreventMoveError();//防止軸卡出錯
                    }
                }
                else
                {
                    if (!g_bIsStop)
                    {
                        MO_Do3DLineMove((~lBuffX), 0, 0, lWorkVelociy, lAcceleration, lInitVelociy);//X往左移動(-)
                        PreventMoveError();//防止軸卡出錯
                    }
                }
                if (iData1 != 0)
                {
                    if (iType1 == 1 || iType1 == 3)
                    {
                        if (!g_bIsStop)
                        {
                            MO_Do3DLineMove(0, iData1, 0, lWorkVelociy, lAcceleration, lInitVelociy);//Y往下移動(+)
                            PreventMoveError();//防止軸卡出錯
                        }
                    }
                    else
                    {
                        if (!g_bIsStop)
                        {
                            MO_Do3DLineMove(0, (~iData1), 0, lWorkVelociy, lAcceleration, lInitVelociy);//Y往上移動(-)
                            PreventMoveError();//防止軸卡出錯
                        }
                    }
                    if (i % 2 == bNum1)
                    {
                        if (!g_bIsStop)
                        {
                            MO_Do3DLineMove(lBuffX, 0, 0, lWorkVelociy, lAcceleration, lInitVelociy);//X往右移動(+)
                            PreventMoveError();//防止軸卡出錯
                        }
                    }
                    else
                    {
                        if (!g_bIsStop)
                        {
                            MO_Do3DLineMove((~lBuffX), 0, 0, lWorkVelociy, lAcceleration, lInitVelociy);//X往左移動(-)
                            PreventMoveError();//防止軸卡出錯
                        }
                    }
                }
            }
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
/*附屬---填充形態(型態2圓形螺旋填充.由外而內)
*輸入(起始點x1,y1,圓心x2,y2,寬度,驅動速度,加速度,初速度)
*/
void CAction::AttachFillType2(LONG lX1, LONG lY1, LONG lCenX, LONG lCenY, LONG lZ, LONG lZBackDistance, LONG lWidth, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
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
    lDistance = dRadius;
    dWidth = lWidth * 1000;
    dAng1 = acos((cPt1.x - cPtCen1.x) / dRadius);
    dAng2 = asin((cPt1.y - cPtCen1.y) / dRadius);
    cPt2.x = dRadius*cos(dAng1 + M_PI) + cPtCen1.x;
    cPt2.y = dRadius*sin(dAng2 + M_PI) + cPtCen1.y;
    m_ptVec.push_back(cPt1);
    m_ptVec.push_back(cPt2);
    lDistance = lDistance - dWidth;
    LineGetToPoint(lXClose, lYClose, cPt1.x, cPt1.y, cPtCen1.x, cPtCen1.y, lDistance);
    cPt3.x = lXClose;
    cPt3.y = lYClose;
    cPtCen2.x = (cPt3.x + cPt2.x) / 2;
    cPtCen2.y = (cPt3.y + cPt2.y) / 2;//下半圓圓心
    iData = (int)dRadius % (int)dWidth;
    lDistance = lDistance + dWidth;
    while (1)
    {
        if (lDistance < dWidth)
        {
            break;
        }
        lDistance = lDistance - dWidth;
        LineGetToPoint(lXClose, lYClose, cPt1.x, cPt1.y, cPtCen1.x, cPtCen1.y, lDistance);
        cPt3.x = lXClose;
        cPt3.y = lYClose;
        m_ptVec.push_back(cPt3);
        cPt4.x = lDistance*cos(dAng1 + M_PI) + cPtCen1.x;
        cPt4.y = lDistance*sin(dAng2 + M_PI) + cPtCen1.y;
        m_ptVec.push_back(cPt4);
    }
#pragma endregion
    UINT cnt = 1;
    for (ptIter = m_ptVec.begin() + 1; ptIter != m_ptVec.end(); ptIter++)
    {
        lNowX = MO_ReadLogicPosition(0);
        lNowY = MO_ReadLogicPosition(1);
        if (cnt % 2 == 0)
        {
            if (!g_bIsStop)
            {
                MO_Do2DArcMove((*ptIter).x - lNowX, (*ptIter).y - lNowY, cPtCen2.x - lNowX, cPtCen2.y - lNowY, lInitVelociy, lWorkVelociy, bRev);//初始半圓

                PreventMoveError();
            }

        }
        else
        {
            if (!g_bIsStop)
            {
                MO_Do2DArcMove((*ptIter).x - lNowX, (*ptIter).y - lNowY, lCenX - lNowX, lCenY - lNowY, lInitVelociy, lWorkVelociy, bRev);//初始半圓
                PreventMoveError();
            }

        }
        if (cnt == m_ptVec.size() - 1)
        {
            if (!g_bIsStop)
            {
                MO_Do3DLineMove(lCenX - (*ptIter).x, lCenY - (*ptIter).y, 0, lWorkVelociy, lAcceleration, lInitVelociy);//直線移動
                PreventMoveError();
            }
            PauseDoGlue();//暫停恢復後繼續出膠(g_bIsPause=0) 出膠
        }
        cnt++;
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
/*附屬---填充形態(型態3矩形填充.由外而內)
*輸入(起始點x1,y1,結束點x2,y2,寬度,驅動速度,加速度,初速度)
*/
void CAction::AttachFillType3(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lZ, LONG lZBackDistance, LONG lWidth, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
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
    cPt2.x = dRadius*cos(dAngCenCos + M_PI_2) + cPtCen.x;
    cPt2.y = dRadius*sin(dAngCenSin + M_PI_2) + cPtCen.y;
    cPt4.x = dRadius*cos(dAngCenCos + M_PI_2 * 3) + cPtCen.x;
    cPt4.y = dRadius*sin(dAngCenSin + M_PI_2 * 3) + cPtCen.y;
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
        cPt1.x = dDistance*cos(dAngCos) + cPt4.x;
        cPt1.y = dDistance*sin(dAngSin) + cPt4.y;
        m_ptVec.push_back(cPt1);
        cPt2.x = dRadius*cos(dAngCenCos + M_PI_2) + cPtCen.x;
        cPt2.y = dRadius*sin(dAngCenSin + M_PI_2) + cPtCen.y;
        m_ptVec.push_back(cPt2);
        dDistance = dDistance - dWidth;
        if (dDistance < dWidth)
        {
            break;
        }
        cPt3.x = dRadius*cos(dAngCenCos + M_PI) + cPtCen.x;
        cPt3.y = dRadius*sin(dAngCenSin + M_PI) + cPtCen.y;
        m_ptVec.push_back(cPt3);
        cPt4.x = dRadius*cos(dAngCenCos + M_PI_2 * 3) + cPtCen.x;
        cPt4.y = dRadius*sin(dAngCenSin + M_PI_2 * 3) + cPtCen.y;
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
void CAction::AttachFillType4(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lZ, LONG lZBackDistance, LONG lWidth, LONG lWidth2, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
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
    cPt2.x = dRadius*cos(dAngCenCos + M_PI_2) + cPtCen.x;
    cPt2.y = dRadius*sin(dAngCenSin + M_PI_2) + cPtCen.y;
    cPt4.x = dRadius*cos(dAngCenCos + M_PI_2 * 3) + cPtCen.x;
    cPt4.y = dRadius*sin(dAngCenSin + M_PI_2 * 3) + cPtCen.y;
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
            AfxMessageBox(L"兩端寬度過大");
            m_ptVec.push_back(cPt1);
            break;
        }
        dRadius = dRadius - dWidth*sqrt(2);
        dDistance = dDistance - dWidth;
        if (iBuff*dWidth >  dWidth2) //dWidth2兩端寬度
        {
            dDistance += dWidth;
            cPt1.x = dDistance*cos(dAngCos) + cPt4.x;
            cPt1.y = dDistance*sin(dAngSin) + cPt4.y;
            m_ptVec.push_back(cPt1);
            break;
        }
        if (dDistance < dWidth)
        {
            break;
        }
        cPt1.x = dDistance*cos(dAngCos) + cPt4.x;
        cPt1.y = dDistance*sin(dAngSin) + cPt4.y;
        m_ptVec.push_back(cPt1);
        cPt2.x = dRadius*cos(dAngCenCos + M_PI_2) + cPtCen.x;
        cPt2.y = dRadius*sin(dAngCenSin + M_PI_2) + cPtCen.y;
        m_ptVec.push_back(cPt2);
        dDistance = dDistance - dWidth;
        if (dDistance < dWidth)
        {
            break;
        }
        cPt3.x = dRadius*cos(dAngCenCos + M_PI) + cPtCen.x;
        cPt3.y = dRadius*sin(dAngCenSin + M_PI) + cPtCen.y;
        m_ptVec.push_back(cPt3);
        cPt4.x = dRadius*cos(dAngCenCos + M_PI_2 * 3) + cPtCen.x;
        cPt4.y = dRadius*sin(dAngCenSin + M_PI_2 * 3) + cPtCen.y;
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
void CAction::AttachFillType5(LONG lX1, LONG lY1, LONG lCenX, LONG lCenY, LONG lZ, LONG lZBackDistance, LONG lWidth, LONG lWidth2, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
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
    lDistance = dRadius;
    dWidth = lWidth * 1000;
    dWidth2 = lWidth2 * 1000;
    dAng1 = acos((cPt1.x - cPtCen1.x) / dRadius);
    dAng2 = asin((cPt1.y - cPtCen1.y) / dRadius);
    cPt2.x = dRadius*cos(dAng1 + M_PI) + cPtCen1.x;
    cPt2.y = dRadius*sin(dAng2 + M_PI) + cPtCen1.y;
    m_ptVec.push_back(cPt1);
    m_ptVec.push_back(cPt2);
    lDistance = lDistance - dWidth;
    LineGetToPoint(lXClose, lYClose, cPt1.x, cPt1.y, cPtCen1.x, cPtCen1.y, lDistance);
    cPt3.x = lXClose;
    cPt3.y = lYClose;
    cPtCen2.x = (cPt3.x + cPt2.x) / 2;
    cPtCen2.y = (cPt3.y + cPt2.y) / 2;//下半圓圓心
    lDistance = lDistance + dWidth;
    iData = (int)dRadius % (int)dWidth;
    iBuff = 1;
    while (1)
    {
        if ((iBuff == 1) && (lDistance - (2 * dWidth2) < 0))
        {
            AfxMessageBox(L"兩端寬度過大");
            break;
        }
        dRadius = dRadius - dWidth;
        lDistance = lDistance - dWidth;
        if (iBuff * dWidth > dWidth2)
        {
            lDistance += dWidth;
            LineGetToPoint(lXClose, lYClose, cPt1.x, cPt1.y, cPtCen1.x, cPtCen1.y, lDistance);
            cPt3.x = lXClose;
            cPt3.y = lYClose;
            m_ptVec.push_back(cPt3);
            break;
        }
        LineGetToPoint(lXClose, lYClose, cPt1.x, cPt1.y, cPtCen1.x, cPtCen1.y, lDistance);
        cPt3.x = lXClose;
        cPt3.y = lYClose;
        m_ptVec.push_back(cPt3);
        cPt4.x = dRadius*cos(dAng1 + M_PI) + cPtCen1.x;
        cPt4.y = lDistance*sin(dAng2 + M_PI) + cPtCen1.y;
        m_ptVec.push_back(cPt4);
        iBuff++;
    }
#pragma endregion
    PauseDoGlue();//暫停恢復後繼續出膠(g_bIsPause=0)出膠
    for (UINT i = 1; i < m_ptVec.size(); i++)
    {
        lNowX = MO_ReadLogicPosition(0);
        lNowY = MO_ReadLogicPosition(1);
        if (i == m_ptVec.size() - 1)
        {
            if (i % 2 == 0)
            {
                if (!g_bIsStop)
                {
                    MO_Do2DArcMove(m_ptVec.at(i).x - lNowX, m_ptVec.at(i).y - lNowY, cPtCen1.x - lNowX, cPtCen1.y - lNowY, lInitVelociy, lWorkVelociy, bRev);//上半圓
                    PreventMoveError();
                }
            }
            else
            {
                if (!g_bIsStop)
                {
                    MO_Do2DArcMove(m_ptVec.at(i).x - lNowX, m_ptVec.at(i).y - lNowY, cPtCen2.x - lNowX, cPtCen2.y - lNowY, lInitVelociy, lWorkVelociy, bRev);//下半圓
                    PreventMoveError();
                }
            }
        }
        else
        {
            if (i % 2 != 0)
            {
                if (!g_bIsStop)
                {
                    MO_Do2DArcMove(m_ptVec.at(i).x - lNowX, m_ptVec.at(i).y - lNowY, cPtCen1.x - lNowX, cPtCen1.y - lNowY, lInitVelociy, lWorkVelociy, bRev);//上半圓
                    PreventMoveError();
                }
            }
            else
            {
                if (!g_bIsStop)
                {
                    MO_Do2DArcMove(m_ptVec.at(i).x - lNowX, m_ptVec.at(i).y - lNowY, cPtCen2.x - lNowX, cPtCen2.y - lNowY, lInitVelociy, lWorkVelociy, bRev);//下半圓
                    PreventMoveError();
                }
            }
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
/*附屬---填充形態(型態6矩形填充.由內而外)
*輸入(起始點x1,y1,結束點x2,y2,寬度,驅動速度,加速度,初速度)
*/
void CAction::AttachFillType6(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lZ, LONG lZBackDistance, LONG lWidth, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
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
    cPt2.x = dRadius*cos(dAngCenCos + M_PI_2) + cPtCen.x;
    cPt2.y = dRadius*sin(dAngCenSin + M_PI_2) + cPtCen.y;
    cPt4.x = dRadius*cos(dAngCenCos + M_PI_2 * 3) + cPtCen.x;
    cPt4.y = dRadius*sin(dAngCenSin + M_PI_2 * 3) + cPtCen.y;
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
        cPt1.x = dDistance*cos(dAngCos) + cPt4.x;
        cPt1.y = dDistance*sin(dAngSin) + cPt4.y;
        m_ptVec.push_back(cPt1);
        cPt2.x = dRadius*cos(dAngCenCos + M_PI_2) + cPtCen.x;
        cPt2.y = dRadius*sin(dAngCenSin + M_PI_2) + cPtCen.y;
        m_ptVec.push_back(cPt2);
        dDistance = dDistance - dWidth;
        if (dDistance < dWidth)
        {
            break;
        }
        cPt3.x = dRadius*cos(dAngCenCos + M_PI) + cPtCen.x;
        cPt3.y = dRadius*sin(dAngCenSin + M_PI) + cPtCen.y;
        m_ptVec.push_back(cPt3);
        cPt4.x = dRadius*cos(dAngCenCos + M_PI_2 * 3) + cPtCen.x;
        cPt4.y = dRadius*sin(dAngCenSin + M_PI_2 * 3) + cPtCen.y;
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
void CAction::AttachFillType7(LONG lX1, LONG lY1, LONG lCenX, LONG lCenY, LONG lZ, LONG lZBackDistance, LONG lWidth, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
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
    lDistance = dRadius;
    dWidth = lWidth * 1000;
    dAng1 = acos((cPt1.x - cPtCen1.x) / dRadius);
    dAng2 = asin((cPt1.y - cPtCen1.y) / dRadius);
    cPt2.x = dRadius*cos(dAng1 + M_PI) + cPtCen1.x;
    cPt2.y = dRadius*sin(dAng2 + M_PI) + cPtCen1.y;
    m_ptVec.push_back(cPt1);
    m_ptVec.push_back(cPt2);
    lDistance = lDistance - dWidth;
    LineGetToPoint(lXClose, lYClose, cPt1.x, cPt1.y, cPtCen1.x, cPtCen1.y, lDistance);
    cPt3.x = lXClose;
    cPt3.y = lYClose;
    cPtCen2.x = (cPt3.x + cPt2.x) / 2;
    cPtCen2.y = (cPt3.y + cPt2.y) / 2;//下半圓圓心
    iData = (int)dRadius % (int)dWidth;
    lDistance = lDistance + dWidth;
    while (1)
    {
        if (lDistance < dWidth)
        {
            break;
        }
        lDistance = lDistance - dWidth;
        LineGetToPoint(lXClose, lYClose, cPt1.x, cPt1.y, cPtCen1.x, cPtCen1.y, lDistance);
        cPt3.x = lXClose;
        cPt3.y = lYClose;
        m_ptVec.push_back(cPt3);
        cPt4.x = lDistance*cos(dAng1 + M_PI) + cPtCen1.x;
        cPt4.y = lDistance*sin(dAng2 + M_PI) + cPtCen1.y;
        m_ptVec.push_back(cPt4);
    }
#pragma endregion
    //先抬升，移動到中心點在下降
    if (!g_bIsStop)
    {
        MO_Do3DLineMove(0, 0, (lZ - lZBackDistance) - lZ, lWorkVelociy, lAcceleration, lInitVelociy);//Z軸返回
        PreventMoveError();//防止軸卡出錯
    }
    lNowX = MO_ReadLogicPosition(0);
    lNowY = MO_ReadLogicPosition(1);
    lNowZ = MO_ReadLogicPosition(2);
    if (!g_bIsStop)
    {
        MO_Do3DLineMove(lCenX - lNowX, lCenY - lNowY, 0, lWorkVelociy, lAcceleration, lInitVelociy);//直線移動至圓心
        PreventMoveError();
    }
    if (!g_bIsStop)
    {
        MO_Do3DLineMove(0, 0, lZ - lNowZ, lWorkVelociy, lAcceleration, lInitVelociy);//Z軸往下
        PreventMoveError();//防止軸卡出錯
    }
    for (rptIter = m_ptVec.rbegin(); rptIter != m_ptVec.rend(); rptIter++)
    {
        lNowX = MO_ReadLogicPosition(0);
        lNowY = MO_ReadLogicPosition(1);
        if ((iData != 0) && (iOdd == 0))
        {
            if (!g_bIsStop)
            {
                MO_Do3DLineMove((*rptIter).x - lNowX, (*rptIter).y - lNowY, 0, lWorkVelociy, lAcceleration, lInitVelociy);//直線移動
                PreventMoveError();
            }
            PauseDoGlue();//暫停恢復後繼續出膠(g_bIsPause=0) 出膠
        }
        else if ((iData == 0) && (iOdd == 0))
        {
            if (!g_bIsStop)
            {
                MO_Do2DArcMove((*rptIter).x - lNowX, (*rptIter).y - lNowY, cPtCen2.x - lNowX, cPtCen2.y - lNowY, lInitVelociy, lWorkVelociy, bRev);//初始半圓
                PreventMoveError();
            }
            PauseDoGlue();//暫停恢復後繼續出膠(g_bIsPause=0) 出膠
        }
        else
        {
            if (iOdd % 2 == 0)
            {
                if (!g_bIsStop)
                {
                    MO_Do2DArcMove((*rptIter).x - lNowX, (*rptIter).y - lNowY, cPtCen2.x - lNowX, cPtCen2.y - lNowY, lInitVelociy, lWorkVelociy, bRev);//初始半圓
                    PreventMoveError();
                }
            }
            else
            {
                if (!g_bIsStop)
                {
                    MO_Do2DArcMove((*rptIter).x - lNowX, (*rptIter).y - lNowY, lCenX - lNowX, lCenY - lNowY, lInitVelociy, lWorkVelociy, bRev);//初始半圓
                    PreventMoveError();
                }
            }
        }
        iOdd++;
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




