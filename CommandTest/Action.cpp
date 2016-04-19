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
    g_bIsDispend = TRUE;
    g_iNumberGluePort = 1;
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
#ifdef MOVE
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
/*
*線段開始動作
*輸入(線段開始、線段點膠設定、加速度、線速度、系統預設參數)
*/
void CAction::DecideLineStartMove(LONG lX, LONG lY, LONG lZ, LONG lStartDelayTime, LONG lStartDistance,
    LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
    /*線段開始(x座標，y座標，z座標，線段起始點，)
    LONG lX, LONG lY, LONG lZ
    */
    /*線段點膠設定(移動前延遲，設置距離，)
    LONG lStartDelayTime ,LONG lStartDistance
    */
    //1.移動前點膠機在一條線段起始點處保持打開的時長。 此延時可防止針頭在流體流動之前沿線段發生移動。
    //2. 點膠機開啟前， 馬達離開直線線段起始點的移動距離。 該距離為馬達提供了足夠的起速時間，主要用來消除過量流體在線段起始處的積聚。
#ifdef MOVE
    LONG lNowX = 0, lNowY = 0;
    LONG lXClose = 0, lYClose = 0, lLineClose = 0;
    int iBuf = 0;
    lNowX = MO_ReadLogicPosition(0);
    lNowY = MO_ReadLogicPosition(1);
    lLineClose = lStartDistance;
    LineGetToPoint(lXClose, lYClose, lNowX, lNowY, lX, lY, lLineClose);
    iBuf = lStartDelayTime ^ lStartDistance;//互斥或移動前延遲跟設置距離 如果兩者都相同結果為0(當兩者都有值時以"移動前延遲"優先)
    AttachPointMove(lX, lY, 0, lWorkVelociy, lAcceleration, lInitVelociy);//使用附屬執行
    AttachPointMove(0, 0, lZ, lWorkVelociy, lAcceleration, lInitVelociy);//使用附屬執行
    PreventMoveError();//防止軸卡出錯
    if (iBuf == 0)
    {
        if (g_bIsDispend == 1)
        {
            MO_GummingSet(g_iNumberGluePort,0, GummingTimeOutThread);//塗膠(不卡)
        }
        MO_Timer(0, 0, lStartDelayTime * 1000);
        MO_Timer(1, 0, lStartDelayTime * 1000);//線段點膠設定---(1)移動前延遲(在線段開始點上)
        Sleep(1);//防止出錯，避免計時器初直為0
        while (MO_Timer(3, 0, 0))
        {
            Sleep(1);
        }
    }
    else
    {
        if (lStartDistance == 0)
        {
            if (g_bIsDispend == 1)
            {
                MO_GummingSet(g_iNumberGluePort,0, GummingTimeOutThread);//塗膠(不卡)
            }
            MO_Timer(0, 0, lStartDelayTime * 1000);
            MO_Timer(1, 0, lStartDelayTime * 1000);//線段點膠設定---(1)移動前延遲(在線段開始點上)
            Sleep(1);//防止出錯，避免計時器初直為0
            while (MO_Timer(3, 0, 0))
            {
                Sleep(1);
            }
        }
        else
        {
            if (!g_bIsStop)
            {
                MO_Do3DLineMove(lXClose, lYClose, 0, lWorkVelociy, lAcceleration, lInitVelociy); //線段點膠設定---(2)設置距離
                PreventMoveError();//防止軸卡出錯
                if (g_bIsDispend == 1)
                {
                    MO_GummingSet(g_iNumberGluePort,0, GummingTimeOutThread);//塗膠(不卡)
                }
            }
        }
    }
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
    if (!g_bIsStop)
    {
        AttachPointMove(lX, lY, lZ, lWorkVelociy, lAcceleration, lInitVelociy);//使用附屬執行
        PreventMoveError();//防止軸卡出錯
    }
    MO_Timer(0, 0, lMidDelayTime * 1000);
    MO_Timer(1, 0, lMidDelayTime * 1000);//線段點膠設定---(4)節點時間
    Sleep(1);//防止出錯，避免計時器初直為0
    while (MO_Timer(3, 0, 0))
    {
        Sleep(1);
    }
#endif
}
/*
*線段結束動作
*輸入(線段結束點、線段點膠設定、加速度、線速度、Z軸工作高度設定、點膠結束設定、返回設定、系統參數)
*/
void CAction::DecideLineEndMove(LONG lX, LONG lY, LONG lZ, LONG lCloseOffDelayTime, LONG lCloseDistance,
    LONG lCloseONDelayTime, LONG lZBack, LONG lHighVelocity, LONG lDistance, LONG lHigh,
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
    LONG lZBack
    */
    /*點膠結束設定(高速度)
    LONG lHighVelocity
    */
#ifdef MOVE
    LONG lNowX = 0, lNowY = 0, lNowZ = 0;
    LONG lLineClose = 0, lXClose = 0, lYClose = 0;
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
            Sleep(1);
        }
    }
    else
    {
        if (lCloseDistance == 0) //線段點膠設定---(5)關機距離
        {
            AttachPointMove(lX, lY, lZ, lWorkVelociy, lAcceleration, lInitVelociy);//使用附屬執行
            PreventMoveError();//防止軸卡出錯
            MO_Timer(0, 0, lCloseONDelayTime * 1000);
            MO_Timer(1, 0, lCloseONDelayTime * 1000);//線段點膠設定---(6)關機延遲  
            Sleep(1);//防止出錯，避免計時器初直為0
            while (MO_Timer(3, 0, 0))
            {
                Sleep(1);
            }
        }
        else
        {
            lLineClose = lCloseDistance;
            LineGetToPoint(lXClose, lYClose, lX, lY, lNowX, lNowY, lLineClose);
            if (!g_bIsStop)
            {
                MO_Do3DLineMove(lXClose, lYClose, 0, lWorkVelociy, lAcceleration, lInitVelociy); //線段點膠設定---(5)關機距離
                PreventMoveError();//防止軸卡出錯
            }
        }
    }
    MO_StopGumming();//停止出膠
    GelatinizeBack(iType, lX, lY, lZ, lNowX, lNowY, lDistance, lHigh, lZBack, lLowVelocity, lHighVelocity, lAcceleration, lInitVelociy);//返回設定
    MO_Timer(0, 0, lCloseOffDelayTime * 1000);
    MO_Timer(1, 0, lCloseOffDelayTime * 1000);//線段點膠設定---(3)停留時間 
    Sleep(1);//防止出錯，避免計時器初直為0
    while (MO_Timer(3, 0, 0))
    {
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
    LONG lCircleX = 0, lCircleY = 0, lRev = 0;
    LONG lNowX = 0, lNowY = 0;
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
    lRev = CStringToLong(csBuff, 2);//取得圓心(X，Y，Rev，)
    if (!g_bIsStop)
    {
        MO_Do2DArcMove(0, 0, lCircleX - lNowX, lCircleY - lNowY, lInitVelociy, lWorkVelociy, lRev);
        PreventMoveError();//防止軸卡出錯
    }
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
    LONG lCircleX = 0, lCircleY = 0, lRev = 0;
    LONG lNowX = 0, lNowY = 0;
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
    lRev = CStringToLong(csBuff, 2);//取得圓心(X，Y，Rev，)
    if (!g_bIsStop)
    {
        MO_Do2DArcMove(lX2 - lNowX, lY2 - lNowY, lCircleX - lNowX, lCircleY - lNowY, lInitVelociy, lWorkVelociy, lRev);
        PreventMoveError();//防止軸卡出錯
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
    AttachPointMove(lX, lY, lZ, lWorkVelociy, lAcceleration, lInitVelociy);//使用附屬執行
    PreventMoveError();//防止軸卡出錯
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
    AttachPointMove(lX, lY, lZ, lWorkVelociy, lAcceleration, lInitVelociy);//使用附屬執行
    PreventMoveError();//防止軸卡出錯
    MO_Timer(0, 0, lWaitTime * 1000);
    MO_Timer(1, 0, lWaitTime * 1000);//等待時間(us→ms)
    Sleep(1);//防止出錯，避免計時器初直為0
    while (MO_Timer(3, 0, 0))
    {
        Sleep(1);
    }
#endif
}
/*
*停駐點動作
*輸入(停駐點、系統參數)
*/
void CAction::DecideParkPoint(LONG lX, LONG lY, LONG lZ, LONG lTimeGlue, LONG lWaitTime, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
    /*停駐點(x座標，y座標，z座標，排膠時間，結束後等待時間，)
    LONG lX, LONG lY, LONG lZ,LONG lTimeGlue,LONG lWaitTime
    */
    /*系統參數(驅動速度，加速度，初速度)
    LONG lWorkVelociy,LONG lAcceleration, LONG lInitVelociy
    */
#ifdef MOVE
    AttachPointMove(lX, lY, lZ, lWorkVelociy, lAcceleration, lInitVelociy);//使用附屬執行
    PreventMoveError();//防止軸卡出錯

    if (lTimeGlue == 0)
    {
        Sleep(1);
    }
    else
    {
        DoGlue(lTimeGlue, lWaitTime, GummingTimeOutThread);//執行排膠
    }
#endif
}
/*
*原點賦歸動作
*輸入(LONG速度1，LONG速度2，LONG指定軸(0~7)，LONG偏移量)
*/
void CAction::DecideInitializationMachine(LONG lSpeed1, LONG lSpeed2, LONG lAxis, LONG lMove)
{
#ifdef MOVE
    MO_SetHardLim(7, 1);
    if (!g_bIsStop)
    {
        MO_MoveToHome(lSpeed1, lSpeed2, 7, 0);
        PreventMoveError();//防止軸卡出錯
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
/***************************************************************************************************************自行運用函數*/
/*
*附屬---移動點動作
*輸入(點、驅動速度、加速度、初速度)
*/
void CAction::AttachPointMove(LONG lX, LONG lY, LONG lZ, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
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
    if (!g_bIsStop)
    {
        MO_Do3DLineMove(lX - lNowX, lY - lNowY, lZ - lNowZ, lWorkVelociy, lAcceleration, lInitVelociy);
        PreventMoveError();//防止軸卡出錯
    }
#endif 
}
/*出膠(配合執行緒使用)
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
            Sleep(1);
        }
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
#ifdef MOVE
    LONG lBackXar, lBackYar, lR; //lBackXar x軸移動座標(絕對), lBackYar Y軸移動座標(絕對)
    lR = sqrt(pow(lXarEnd - lXarUp, 2) + pow(lYarEnd - lYarUp, 2));
    if (lLineStop > lR)
    {
        lBackXar = lXarUp;
        lBackYar = lYarUp;
    }
    else
    {
        LONG lXClose, lYClose, lLineClose;
        lLineClose = lLineStop;
        LineGetToPoint(lXClose, lYClose, lXarEnd, lYarEnd, lXarUp, lYarUp, lLineClose);
        lBackXar = lXClose;
        lBackYar = lYClose;
    }
    switch (iType) //0~5
    {
    case 0:
    {
        break;
    }
    case 1:
    {
        if (lStopZar > lZarEnd)
        {
            lStopZar = lZarEnd;
        }
        if (lStopZar == 0)
        {
            if (!g_bIsStop)
            {
                MO_Do3DLineMove(0, 0, (lZarEnd - lBackZar) - lZarEnd, lHighSpeed, lAcceleration, lInitSpeed);
                PreventMoveError();//防止軸卡出錯
            }
        }
        else
        {
            if (!g_bIsStop)
            {
                MO_Do3DLineMove(0, 0, (lZarEnd - lStopZar) - lZarEnd, lLowSpeed, lAcceleration, lInitSpeed);
            }
            if (!g_bIsStop)
            {
                MO_Do3DLineMove(0, 0, (lZarEnd - lBackZar) - (lZarEnd - lStopZar), lHighSpeed, lAcceleration, lInitSpeed);
                PreventMoveError();//防止軸卡出錯
            }
        }
        break;
    }
    case 2:
    {

        if (lStopZar == 0)
        {
            if (!g_bIsStop)
            {
                MO_Do3DLineMove(lBackXar - lXarEnd, lBackYar - lYarEnd, 0, lLowSpeed, lAcceleration, lInitSpeed);
            }
            if (!g_bIsStop)
            {
                MO_Do3DLineMove(0, 0, lBackZar - lZarEnd, lLowSpeed, lAcceleration, lInitSpeed);
                PreventMoveError();//防止軸卡出錯
            }


        }
        else
        {
            if (!g_bIsStop)
            {
                MO_Do3DLineMove(lBackXar - lXarEnd, lBackYar - lYarEnd, (lZarEnd - lStopZar) - lZarEnd, lLowSpeed, lAcceleration, lInitSpeed);
            }
            if (!g_bIsStop)
            {
                MO_Do3DLineMove(0, 0, lBackZar - (lZarEnd - lStopZar), lHighSpeed, lAcceleration, lInitSpeed);
                PreventMoveError();//防止軸卡出錯
            }
        }
        break;
    }
    case 3:
    {
        if (lStopZar == 0)
        {
            if (!g_bIsStop)
            {
                MO_Do3DLineMove(0, 0, 0, lLowSpeed, lAcceleration, lInitSpeed);
                PreventMoveError();//防止軸卡出錯
            }
        }
        else
        {
            if (!g_bIsStop)
            {
                MO_Do3DLineMove(0, 0, (lZarEnd - lStopZar) - lZarEnd, lLowSpeed, lAcceleration, lInitSpeed);
                PreventMoveError();//防止軸卡出錯
            }
        }
        if (!g_bIsStop)
        {
            MO_Do3DLineMove(lBackXar - lXarEnd, lBackYar - lYarEnd, 0, lHighSpeed, lAcceleration, lInitSpeed);
        }
        if (!g_bIsStop)
        {
            MO_Do3DLineMove(0, 0, lBackZar - (lZarEnd - lStopZar), lHighSpeed, lAcceleration, lInitSpeed);
            PreventMoveError();//防止軸卡出錯
        }
        break;
    }
    case 4:
    {
        lBackXar = (lXarEnd - lBackXar) + lXarEnd;
        lBackYar = (lYarEnd - lBackYar) + lYarEnd;
        if (lStopZar == 0)
        {
            if (!g_bIsStop)
            {
                MO_Do3DLineMove(lBackXar - lXarEnd, lBackYar - lYarEnd, 0, lLowSpeed, lAcceleration, lInitSpeed);
            }
            if (!g_bIsStop)
            {
                MO_Do3DLineMove(0, 0, lBackZar - lZarEnd, lLowSpeed, lAcceleration, lInitSpeed);
                PreventMoveError();//防止軸卡出錯
            }
        }
        else
        {
            if (!g_bIsStop)
            {
                MO_Do3DLineMove(lBackXar - lXarEnd, lBackYar - lYarEnd, (lZarEnd - lStopZar) - lZarEnd, lLowSpeed, lAcceleration, lInitSpeed);
            }
            if (!g_bIsStop)
            {
                MO_Do3DLineMove(0, 0, lBackZar - (lZarEnd - lStopZar), lHighSpeed, lAcceleration, lInitSpeed);
                PreventMoveError();//防止軸卡出錯
            }
        }
        break;
    }
    case 5:
    {
        lBackXar = (lXarEnd - lBackXar) + lXarEnd;
        lBackYar = (lYarEnd - lBackYar) + lYarEnd;
        if (lStopZar == 0)
        {
            if (!g_bIsStop)
            {
                MO_Do3DLineMove(0, 0, 0, lLowSpeed, lAcceleration, lInitSpeed);
                PreventMoveError();//防止軸卡出錯
            }
        }
        else
        {
            if (!g_bIsStop)
            {
                MO_Do3DLineMove(0, 0, (lZarEnd - lStopZar) - lZarEnd, lLowSpeed, lAcceleration, lInitSpeed);
                PreventMoveError();//防止軸卡出錯
            }
        }
        if (!g_bIsStop)
        {
            MO_Do3DLineMove(lBackXar - lXarEnd, lBackYar - lYarEnd, 0, lHighSpeed, lAcceleration, lInitSpeed);
        }
        if (!g_bIsStop)
        {
            MO_Do3DLineMove(0, 0, lBackZar - (lZarEnd - lStopZar), lHighSpeed, lAcceleration, lInitSpeed);
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
/*直線距離轉換成座標點*/
void CAction::LineGetToPoint(LONG &lXClose, LONG &lYClose, LONG lX0, LONG lY0, LONG lX1, LONG lY1, LONG &lLineClose)
{
    LONG lM, lB; //(X0,Y0)  起始點;(X1,Y1) 結束點; Y0=M*X0+B 斜率公式;L已知長度
    if ((lX1 - lX0) == 0)
    {
        lM = 0;
    }
    else
    {
        lM = (lY1 - lY0) / (lX1 - lX0);
    }
    lB = lY0 - (lM*lX0);
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
                lYClose = (lM*lX1) + lB;
            }
            else
            {
                lXClose = lX1 + lLineClose;
                lYClose = (lM*lX1) + lB;
            }
        }
        else if (abs(la) == abs(lb))
        {
            if (la > 0)
            {
                lXClose = lX1 - lLineClose;
                lYClose = (lM*lX1) + lB;
            }
            else
            {
                lXClose = lX1 + lLineClose;
                lYClose = (lM*lX1) + lB;
            }
        }
        else if (abs(la) < abs(lb))
        {
            if (lb > 0)
            {
                lYClose = lY1 - lLineClose;
                lXClose = (lYClose - lB) / lM;
            }
            else
            {
                lYClose = lY1 + lLineClose;
                lXClose = (lYClose - lB) / lM;
            }
        }
    }
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
/*字串轉長整數
使用的結尾一定要是"，"
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





