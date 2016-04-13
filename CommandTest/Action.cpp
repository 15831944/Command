/*
*檔案名稱:Action
*內容簡述:運動命令API，詳細參數請查看excel
*＠author 作者名稱:R
*＠data 更新日期:2016/04/11	*/
#include "stdafx.h"
#include "Action.h"
/***********************************************************
**                                                        **
**          運動模組-運動指令命令 (對應動作判斷)             **
**                                                        **
************************************************************/
CAction::CAction()
{     
    g_bIsStop = FALSE;
}
CAction::~CAction()
{
}
/***************************************************************************************************************運動API*/
//等待時間
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
/*單點點膠動作(多載-結束設置+加速度+點到點速度)
*輸入(單點點膠、單點點膠設定、Z軸工作高度設定、點膠結束設定、加速度、（點到點）速度、系統預設參數)*/
void CAction::DecidePointGlue(LONG lX, LONG lY, LONG lZ, LONG lDoTime, LONG lDelayStopTime,
    LONG lZBackDistance, LONG lZdistance, LONG lHighVelocity, LONG lLowVelocity, LONG lWorkVelociy,
    LONG lAcceleration, LONG lInitVelociy)
{
#ifdef MOVE
    /*單點點膠
    LONG lX, LONG lY, LONG lZ
    */
    /*單點點膠設定
    LONG lDDoTime,LONG lDDelayStopTime
    */
    /*Z軸工作高度設定-Z軸回升高度(相對)最高點
    LONG lDZBackDistance
    */
    /*點膠結束設定(Z軸相對距離，高速度，低速度，)
    lZdistance
    lHighVelocity
    lLowVelocity
    */
    /*系統預設參數(驅動速度，加速度，初速度)
    LONG lWorkVelociy,LONG lAcceleration, LONG lInitVelociy
    */
    LONG lNowX = 0, lNowY = 0, lNowZ = 0;
    lNowX = MO_ReadLogicPosition(0);
    lNowY = MO_ReadLogicPosition(1);
    lNowZ = MO_ReadLogicPosition(2);
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
/***************************************************************************************************************自行運用函數*/
/*出膠(配合執行緒使用)
*輸入(點膠機開啟or點膠機關閉)解譯後的LONG值
*/
void CAction::DoGlue(LONG lTime, LONG lDelayTime, LPTHREAD_START_ROUTINE GummingTimeOutThread)
{
#ifdef MOVE
    if (!g_bIsStop)
    {
        MO_GummingSet((lTime), GummingTimeOutThread);
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
            Sleep(1);
        }
    }
#endif
}
/*防止軸卡出錯
*讀取各軸驅動狀態，當動作時停止。*/
void CAction::PreventMoveError()
{
#ifdef MOVE
    while (MO_ReadIsDriving(7) && !g_bIsStop)
    {
        Sleep(1);
    }
#endif
}
/*防止出膠出錯
*讀取出膠狀態，當動作時停止。*/
void CAction::PreventGlueError()
{
#ifdef MOVE
    while (MO_ReadGumming() && !g_bIsStop)
    {
        Sleep(1);
    }
#endif
}
/*執行緒
*中斷時執行完成點膠。*/
DWORD CAction::GummingTimeOutThread(LPVOID)
{
#ifdef MOVE
    MO_FinishGumming();
#endif
    return 0;
}




