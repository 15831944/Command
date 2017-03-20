/*
*檔案名稱:Action(W軸用)
*內容簡述:運動命令API，詳細參數請查看excel
*＠author 作者名稱:R
*＠data 更新日期:2017/3/17
*@更新內容:nova軸卡使用在四軸點膠機上*/
#include "stdafx.h"
#include "Action.h"
#define LA_SCANEND -99999
BOOL CAction::m_ZtimeOutGlueSet = FALSE;
BOOL CAction::m_YtimeOutGlueSet = FALSE;
#define _USE_MATH_DEFINES
#include <math.h>
//**************中斷使用須知*********************
//必須要先進行連結(只須連結一次)     MO_InterruptCase(0, NULL, mo.MoInterrupt,mo.pAction);//中斷連結
/***********************************************************
**                                                        **
**          運動模組-運動指令命令 (對應動作判斷)             **
**                                                        **
************************************************************/
CAction::CAction()
{
    // 需宣告初始值
    pAction = this;
    m_hComm = NULL;
    m_bIsPause = FALSE;//暫停
    m_bIsStop = FALSE;//停止
    m_bIsGetLAend = FALSE;//掃描完成否
    m_bIsDispend = TRUE;//點膠機開
    m_interruptLock = FALSE;//中斷鎖
    m_OffSetLaserX = 0;//雷射偏移量初始化X
    m_OffSetLaserY = 0;//雷射偏移量初始化Y
    m_OffSetLaserZ = 0;//雷射偏移量初始化Z
    m_HeightLaserZero = 0;//雷射用Z軸歸零完成後高度(掃描高度)
    m_LaserCnt = 0;//雷射線段計數器(使用次數)
    m_OffSetScan = 500;//雷射位移補償
    m_LaserAverage = FALSE; //雷射平均(1使用/0不使用)
    m_LaserAveBuffZ = 0; // 雷射用平均暫存值(絕對位置z值)
    m_TablelZ = 100068;//工作平台高度(Z軸總工作高度)
    m_MachineCirMid.x = 0;//機械同心圓座標x
    m_MachineCirMid.y = 0;//機械同心圓座標y
    m_MachineOffSet.x = -99999;//機械同心圓偏移量x(預設未初始化為-99999)
    m_MachineOffSet.y = -99999;//機械同心圓偏移量y(預設未初始化為-99999)
    m_Wangle = 0;//機械校正角度w
    m_MachineCirRad = 0;//機械同心圓半徑
    WangBuff = 0;//w軸角度buff
    cpCirMidBuff[0] = { 0 };
    cpCirMidBuff[1] = { 0 };
    m_ThreadFlag = 0;//執行緒旗標(選擇動作用)
    m_IsCutError = FALSE;//切值錯誤
    m_HomingOffset_INIT.x = 200000;   //原點偏移量X(機械座標)
    m_HomingOffset_INIT.y = 200000;   //原點偏移量Y(機械座標)
    m_HomingOffset_INIT.z = 10000;    //原點偏移量Z(機械座標)
    m_HomingOffset_INIT.w = 0;      //原點偏移量W(機械座標)
	//機械座標為平台的機械座標，傾斜角度為90度
    m_HomingPoint.x = 92000;   //原點復歸點X(機械座標)
    m_HomingPoint.y = 96000;   //原點復歸點Y(機械座標)
    m_HomingPoint.z = 10000;    //原點復歸點Z(機械座標)
    m_HomingPoint.w = 0;        //原點復歸點W(機械座標)
	m_HomeSpeed_DEF = 15000;	//原點復歸預設速度(Z,W軸)
	m_WSpeed = 6.0;				//W軸速度調整變數(1~10)
	m_IsHomingOK = 0;           //原點復歸參數
#ifdef MOVE
    W_m_ptVec.clear();//W資料初使化
    LA_m_ptVec2D.clear();
    LA_m_ptVec.clear();
#endif
}
CAction::~CAction()
{
}
CString CAction::ShowVersion()
{
    CString csbuf, csa, csb, csc, csd;
#ifdef MOVE
    MO_DllVersion(csa);
    MO_DllDate(csb);
#ifdef LA
    LAS_DllVersion(csc);
    LAS_DllDate(csd);
#endif
    csbuf.Format(_T("運動版本=%s、運動時間=%s\n雷射版本=%s、雷射時間=%s\n"), csa, csb, csc, csd);
#endif
    return csbuf;
}

//等待時間
void CAction::WaitTime(HANDLE wakeEvent, int Time)
{
    if(!m_bIsStop)
    {
        DWORD rc = WaitForSingleObject(wakeEvent, Time);
        switch(rc)
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
}
/*************運動API***************************************************************************************************/
/*
*單點點膠動作(多載-結束設置+加速度+點到點速度)
*輸入(單點點膠、單點點膠設定、Z軸工作高度設定、點膠結束設定、加速度、（點到點）速度、系統預設參數)
*/
void CAction::DecidePointGlue(LONG lX, LONG lY, LONG lZ,DOUBLE dAng, LONG lDoTime, LONG lDelayStopTime,
                              LONG lZBackDistance, BOOL bZDisType, LONG lZdistance, LONG lHighVelocity, LONG lLowVelocity, LONG lWorkVelociy,
                              LONG lAcceleration, LONG lInitVelociy)
{
    /*單點點膠
    LONG lX, LONG lY, LONG lZ
    */
    /*W軸角度
    DOUBLE dAng
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
    //判斷W軸是否校正
    if(m_MachineOffSet.x == -99999 && m_MachineOffSet.y == -99999)
    {
        return;
    }
    //判斷使用針頭座標還是機械座標
    if(USEMODE_W == 1) //使用機械座標
    {
        CPoint cpbuf = W_GetNeedlePoint(lX, lY, dAng, 1);
        lX = cpbuf.x;
        lY = cpbuf.y;
    }
    /*******以下程式都會轉變使用針頭座標執行運動************************/
    //流程: 輸入針頭座標>運算針頭座標>轉換成機械座標輸出
    if(lHighVelocity == 0)
    {
        lHighVelocity = lWorkVelociy;
    }
    if(lLowVelocity == 0)
    {
        lLowVelocity = lWorkVelociy;
    }
    if(bZDisType) //當使用相對位置的時候
    {
        LONG lbuf = m_TablelZ - lZBackDistance;//從平台最低往上相對距離
        lZBackDistance = lbuf;
        bZDisType = 0;//改為絕對位置
    }
    if(!bZDisType)  //絕對位置
    {
        if(lZBackDistance > lZ)
        {
            lZBackDistance = lZ;
        }
        lZBackDistance = abs(lZBackDistance - lZ);
    }
    if(lZBackDistance>lZ)
    {
        lZBackDistance = lZ;
    }
    //W_Rotation(dAng, lWorkVelociy, lAcceleration, lInitVelociy);//w軸自轉
    if(!m_bIsStop)
    {
        MO_Do4DLineMove(0, 0, 0, dAng-MO_ReadLogicPositionW(), (LONG)round(lWorkVelociy/ m_WSpeed), (LONG)(lAcceleration/ m_WSpeed), lInitVelociy);//移動W
        PreventMoveError();//防止軸卡出錯
    }
    CPoint cpRobot=W_GetMachinePoint(lX,lY,dAng,1) ;//結束點的機械座標
    if(!m_bIsStop)
    {
        MO_Do3DLineMove(cpRobot.x - MO_ReadLogicPosition(0), cpRobot.y - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration, lInitVelociy);//移動到點膠點
        PreventMoveError();//防止軸卡出錯
    }
    if(!m_bIsStop)
    {
        MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//移動到點膠點
        PreventMoveError();//防止軸卡出錯
    }
    if(!m_bIsStop)
    {
        if(lDoTime == 0)
        {
            Sleep(1);
        }
        else
        {
            DoGlue(lDoTime, lDelayStopTime);//執行點膠
        }
    }
    if(lZdistance == 0)
    {
        if(!m_bIsStop)
        {
            MO_Do3DLineMove(0, 0, (lZ - lZBackDistance) - lZ, lWorkVelociy, lAcceleration, lInitVelociy);//點膠結束設置
            PreventMoveError();//防止軸卡出錯
        }
    }
    else
    {
        if(lZBackDistance == 0)
        {
            lZdistance = lZBackDistance;
        }
        if(lZdistance>lZBackDistance && lZBackDistance != 0)
        {
            lZdistance = lZBackDistance;
            MO_Do3DLineMove(0, 0, (lZ - lZBackDistance) - lZ, lHighVelocity, lAcceleration, lInitVelociy);//點膠結束設置
            PreventMoveError();//防止軸卡出錯
        }
        else
        {
            if(!m_bIsStop)
            {
                MO_Do3DLineMove(0, 0, (lZ - lZdistance) - lZ, lLowVelocity, lAcceleration, lInitVelociy);//點膠結束設置
                PreventMoveError();//防止軸卡出錯
            }
            if(!m_bIsStop)
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
void CAction::DecideLineStartMove(LONG lX, LONG lY, LONG lZ, DOUBLE dAng, LONG lStartDelayTime,LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy, BOOL bIsNeedleP)
{
    /*線段開始(x座標，y座標，z座標，線段起始點，)
    LONG lX, LONG lY, LONG lZ
    */
    /*W軸角度
    DOUBLE dAng
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
    //判斷W軸是否校正
    if(m_MachineOffSet.x == -99999 && m_MachineOffSet.y == -99999)
    {
        return;
    }
    //判斷使用針頭座標還是機械座標
    if(USEMODE_W == 1 && bIsNeedleP == 0) //使用機械座標
    {
        CPoint cpbuf = W_GetNeedlePoint(lX, lY, dAng, 1);
        lX = cpbuf.x;
        lY = cpbuf.y;
    }
    /*******以下程式都會轉變使用針頭座標執行運動************************/
    //流程: 輸入針頭座標>運算針頭座標>轉換成機械座標輸出
    //W_Rotation(dAng, lWorkVelociy, lAcceleration, lInitVelociy);//w軸自轉
    if(!m_bIsStop)
    {
        MO_Do4DLineMove(0,0, 0, dAng - MO_ReadLogicPositionW(), (LONG)round(lWorkVelociy / m_WSpeed), (LONG)(lAcceleration / m_WSpeed), lInitVelociy);//移動到點膠點
        PreventMoveError();//防止軸卡出錯
    }
    CPoint cpRobot = W_GetMachinePoint(lX, lY, dAng, 1);//結束點的機械座標
    if(!m_bIsStop)
    {
        MO_Do3DLineMove(cpRobot.x - MO_ReadLogicPosition(0), cpRobot.y - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration, lInitVelociy);//移動到點膠點
        PreventMoveError();//防止軸卡出錯
    }
    if(!m_bIsStop)
    {
        MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//x,y,z軸移動
        PreventMoveError();//防止軸卡出錯
    }
    if(!m_bIsStop && m_bIsDispend == 1)
    {
        MO_GummingSet();//塗膠(不卡)
    }
    MO_Timer(0, 0, lStartDelayTime * 1000);
    MO_Timer(1, 0, lStartDelayTime * 1000);//線段點膠設定---(1)移動前延遲(在線段開始點上)
    while(MO_Timer(3, 0, 0))
    {
        if(m_bIsStop)
        {
            break;
        }
        Sleep(1);
    }
    PauseStopGlue();//暫停時停指塗膠(m_bIsPause=1)
#endif
}
/*
*線段中點動作
*輸入(線段中點、線段點膠設定、加速度、線速度、系統預設參數)
*/
void CAction::DecideLineMidMove(LONG lX, LONG lY, LONG lZ, DOUBLE dAng, LONG lMidDelayTime, LONG lWorkVelociy,LONG lAcceleration, LONG lInitVelociy, BOOL bIsNeedleP)
{
    /*線段中點(x座標，y座標，z座標，線段起始點，)
    LONG lX, LONG lY, LONG lZ
    */
    /*W軸角度
    DOUBLE dAng
    */
    /*線段點膠設定(節點時間)
    LONG lMidDelayTime
    */
#ifdef MOVE
    //判斷W軸是否校正
    if(m_MachineOffSet.x == -99999 && m_MachineOffSet.y == -99999)
    {
        return;
    }
    //判斷使用針頭座標還是機械座標
    if(USEMODE_W == 1 && bIsNeedleP == 0) //使用機械座標
    {
        CPoint cpbuf = W_GetNeedlePoint(lX, lY, dAng, 1);
        lX = cpbuf.x;
        lY = cpbuf.y;
    }
    /*******以下程式都會轉變使用針頭座標執行運動************************/
    //流程: 輸入針頭座標>運算針頭座標>轉換成機械座標輸出
    CPoint cpNeedlePoint = W_GetNeedlePoint();//將當下位置轉換成針頭座標
    W_UpdateNeedleMotor_Needle(cpNeedlePoint.x, cpNeedlePoint.y, MO_ReadLogicPosition(2), MO_ReadLogicPositionW(), lX, lY, lZ, dAng);//切點取值(使用預設每1度取一個點)
    PauseDoGlue();//暫停恢復後繼續出膠(m_bIsPause=0) 出膠
    if(!m_bIsStop)
    {
        W_Line4DtoDo(lWorkVelociy, lAcceleration, lInitVelociy);//四軸連續插補
        PreventMoveError();//防止軸卡出錯
    }
	CPoint cpMachinePoint = W_GetMachinePoint(lX, lY, dAng, 1);
	lX = cpMachinePoint.x;
	lY = cpMachinePoint.y;
	//_cwprintf(_T("%s \n"), _T("補償修正"));
	//if (!m_bIsStop)
	//{
	//	MO_Do4DLineMove(lX - MO_ReadLogicPosition(0), lY - MO_ReadLogicPosition(1), lZ - MO_ReadLogicPosition(2),dAng- MO_ReadLogicPositionW(), lInitVelociy, lWorkVelociy, lInitVelociy);
	//	PreventMoveError();
	//}
    MO_Timer(0, 0, lMidDelayTime * 1000);
    MO_Timer(1, 0, lMidDelayTime * 1000);//線段點膠設定---(4)節點時間
    Sleep(1);//防止出錯，避免計時器初直為0
    while(MO_Timer(3, 0, 0))
    {
        if(m_bIsStop)
        {
            break;
        }
        Sleep(1);
    }
    PauseStopGlue();//暫停時停指塗膠(m_bIsPause=1)
#endif
}
/*
*線段結束動作
*輸入(線段結束點、線段點膠設定、加速度、線速度、Z軸工作高度設定、點膠結束設定、返回設定、系統參數)
*/
void CAction::DecideLineEndMove(LONG lX, LONG lY, LONG lZ, DOUBLE dAng, LONG lCloseOffDelayTime, LONG lCloseDistance,
                                LONG lCloseONDelayTime, LONG lZBackDistance, BOOL bZDisType, LONG lHighVelocity, LONG lDistance, LONG lHigh,
                                LONG lLowVelocity, int iType, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
    /*線段結束點(x座標，y座標，z座標，線段起始點，)
    LONG lX, LONG lY, LONG lZ
    */
    /*W軸角度
    DOUBLE dAng
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
    //判斷W軸是否校正
    if(m_MachineOffSet.x == -99999 && m_MachineOffSet.y == -99999)
    {
        return;
    }
    //判斷使用針頭座標還是機械座標
    if(USEMODE_W == 1) //使用機械座標
    {
        CPoint cpbuf = W_GetNeedlePoint(lX, lY, dAng, 1);
        lX = cpbuf.x;
        lY = cpbuf.y;
    }
    /*******以下程式都會轉變使用針頭座標執行運動************************/
    //流程: 輸入針頭座標>運算針頭座標>轉換成機械座標輸出
    if(bZDisType) //當使用相對位置的時候
    {
        LONG lbuf = m_TablelZ - lZBackDistance;//從平台最低往上相對距離
        lZBackDistance = lbuf;
        bZDisType = 0;//改為絕對位置
    }
    if(!bZDisType)  //絕對位置
    {
        if(lZBackDistance > lZ)
        {
            lZBackDistance = lZ;
        }
        lZBackDistance = abs(lZBackDistance - lZ);
    }
    LONG lLineClose = 0, lXClose = 0, lYClose = 0, lZClose = 0, LineLength = 0;
    PauseDoGlue();//暫停恢復後繼續出膠(m_bIsPause=0)
    CPoint cpNeedlePoint = W_GetNeedlePoint();//將當下位置轉換成針頭座標
    LineLength = LONG(sqrt(DOUBLE(pow(lX - cpNeedlePoint.x, 2) + pow(lY - cpNeedlePoint.y, 2)+ pow(lZ - MO_ReadLogicPosition(2), 2))));
    if(lCloseDistance != 0 && lCloseDistance > LineLength)
    {
        lCloseDistance = LineLength;
    }
    if(lHighVelocity == 0)
    {
        lHighVelocity = lWorkVelociy;
    }
    if(lLowVelocity == 0)
    {
        lLowVelocity = lWorkVelociy;
    }
    if(lX == cpNeedlePoint.x && lY == cpNeedlePoint.y) //已經在結束點上不做關機距離
    {
        MO_Timer(0, 0, lCloseONDelayTime * 1000);
        MO_Timer(1, 0, lCloseONDelayTime * 1000);//線段點膠設定---(6)關機延遲
        Sleep(1);//防止出錯，避免計時器初直為0
        while(MO_Timer(3, 0, 0))
        {
            if(m_bIsStop)
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
        if(lCloseDistance == 0)  //線段點膠設定---(5)關機距離
        {
            W_UpdateNeedleMotor_Needle(cpNeedlePoint.x, cpNeedlePoint.y, MO_ReadLogicPosition(2), MO_ReadLogicPositionW(), lX, lY, lZ, dAng);//切點取值(使用預設每1度取一個點)
            if(!m_bIsStop)
            {
                W_Line4DtoDo(lWorkVelociy, lAcceleration, lInitVelociy);//四軸連續插補
                PreventMoveError();//防止軸卡出錯
            }
            MO_Timer(0, 0, lCloseONDelayTime * 1000);
            MO_Timer(1, 0, lCloseONDelayTime * 1000);//線段點膠設定---(6)關機延遲
            Sleep(1);//防止出錯，避免計時器初直為0
            while(MO_Timer(3, 0, 0))
            {
                if(m_bIsStop == 1)
                {
                    break;
                }
                Sleep(1);
            }
            MO_StopGumming();//停止出膠
        }
        else
        {
            W_UpdateNeedleMotor_Needle(cpNeedlePoint.x, cpNeedlePoint.y, MO_ReadLogicPosition(2), MO_ReadLogicPositionW(), lX, lY, lZ, dAng);//切點取值(使用預設每1度取一個點)
            //使用(5)關機距離(lCloseDistance)
            LONG sumPath = lCloseDistance;
            LONG finishTime = 0;
            DOUBLE avgTime = 0;
            LONG accLength = CalPreglue(lWorkVelociy, lAcceleration, lInitVelociy);
            avgTime = ((DOUBLE)sumPath - (DOUBLE)accLength) / (DOUBLE)lWorkVelociy;
            finishTime = (LONG)round(avgTime * 1000000) + CalPreglueTime(lWorkVelociy, lAcceleration, lInitVelociy);
            LONG closeDistTime = CalPreglue(lCloseDistance, lWorkVelociy, 0, lInitVelociy);
            /*======計時器到觸發中斷執行斷膠，使用z中斷執行================*/
            if(!m_bIsStop)
            {
                CAction::m_ZtimeOutGlueSet = FALSE;
                MO_TimerSetIntter(finishTime - closeDistTime, 1);
            }
            if(!m_bIsStop)
            {
                W_Line4DtoDo(lWorkVelociy, lAcceleration, lInitVelociy);//四軸連續插補
                PreventMoveError();//防止軸卡出錯
            }
        }
    }
    CPoint cpBuf = W_GetNeedlePoint(lX, lY, dAng, 1);
    GelatinizeBack(iType, cpBuf.x, cpBuf.y, lZ, cpNeedlePoint.x, cpNeedlePoint.y, lDistance, lHigh, lZBackDistance, lLowVelocity, lHighVelocity, lAcceleration, lInitVelociy);//返回設定
    MO_Timer(0, 0, lCloseOffDelayTime * 1000);
    MO_Timer(1, 0, lCloseOffDelayTime * 1000);//線段點膠設定---(3)停留時間
    Sleep(1);//防止出錯，避免計時器初直為0
    while(MO_Timer(3, 0, 0))
    {
        if(m_bIsStop == 1)
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
                                LONG lLowVelocity, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy, BOOL bGeneral)
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
    //判斷W軸是否校正
    if(m_MachineOffSet.x == -99999 && m_MachineOffSet.y == -99999)
    {
        return;
    }
    LONG lLineClose = 0, lXClose = 0, lYClose = 0;
    if(!bGeneral)
    {
        PauseDoGlue();//暫停恢復後繼續出膠(m_bIsPause=0)
    }
    if(bZDisType) //當使用相對位置的時候
    {
        LONG lbuf = m_TablelZ - lZBackDistance;//從平台最低往上相對距離
        lZBackDistance = lbuf;
        bZDisType = 0;//改為絕對位置
    }
    if(!bZDisType)  //絕對位置
    {
        if(lZBackDistance > MO_ReadLogicPosition(2))
        {
            lZBackDistance = MO_ReadLogicPosition(2);
        }
        lZBackDistance = abs(lZBackDistance - MO_ReadLogicPosition(2));
    }
    if(lHighVelocity == 0)
    {
        lHighVelocity = lWorkVelociy;
    }
    if(lLowVelocity == 0)
    {
        lLowVelocity = lWorkVelociy;
    }
    MO_Timer(0, 0, lCloseONDelayTime * 1000);
    MO_Timer(1, 0, lCloseONDelayTime * 1000);//線段點膠設定---(6)關機延遲
    Sleep(1);//防止出錯，避免計時器初直為0
    while(MO_Timer(3, 0, 0))
    {
        if(m_bIsStop == 1)
        {
            break;
        }
        Sleep(1);
    }
    if(!bGeneral || MO_ReadGumming())
    {
        MO_StopGumming();//停止出膠
    }
    GelatinizeBack(1, MO_ReadLogicPosition(0), MO_ReadLogicPosition(1), MO_ReadLogicPosition(2), MO_ReadLogicPosition(0), MO_ReadLogicPosition(1), lDistance, lHigh, lZBackDistance, lLowVelocity, lHighVelocity, lAcceleration, lInitVelociy);//返回設定
    MO_Timer(0, 0, lCloseOffDelayTime * 1000);
    MO_Timer(1, 0, lCloseOffDelayTime * 1000);//線段點膠設定---(3)停留時間
    Sleep(1);//防止出錯，避免計時器初直為0
    while(MO_Timer(3, 0, 0))
    {
        if(m_bIsStop == 1)
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
void CAction::DecideLineSToP(LONG lX, LONG lY, LONG lZ, DOUBLE dAng, LONG lX2, LONG lY2, LONG lZ2, DOUBLE dAng2,
                             LONG lStartDelayTime, LONG lStartDistance, LONG lMidDelayTime,
                             LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
    /*線段開始(x座標，y座標，z座標，線段起始點，W軸角度)
    LONG lX, LONG lY, LONG lZ, DOUBLE dAng
    */
    /*線段中點(x座標，y座標，z座標，線段起始點，W軸角度)
    LONG lX2, LONG lY2, LONG lZ2, DOUBLE dAng
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
    //判斷W軸是否校正
    if(m_MachineOffSet.x == -99999 && m_MachineOffSet.y == -99999)
    {
        return;
    }
    //判斷使用針頭座標還是機械座標
    if(USEMODE_W == 1) //使用機械座標
    {
        CPoint cpbuf = W_GetNeedlePoint(lX, lY, dAng, 1);
        lX = cpbuf.x;
        lY = cpbuf.y;
        cpbuf = W_GetNeedlePoint(lX2, lY2, dAng2, 1);
        lX2 = cpbuf.x;
        lY2 = cpbuf.y;
    }
    /*******以下程式都會轉變使用針頭座標執行運動************************/
    //流程: 輸入針頭座標>運算針頭座標>轉換成機械座標輸出
    LONG lXClose = 0, lYClose = 0, lZClose = 0, lLineClose = 0;
    LONG lBuffX = 0, lBuffY = 0;
    LONG lTime = 0;
    lLineClose = lStartDistance;
    LineGetToPoint(lXClose, lYClose, lZClose, lX2, lY2, lX, lY, lZ2, lZ, lLineClose);
    lBuffX = (-(lXClose - lX)) + lX;
    lBuffY = (-(lYClose - lY)) + lY;

    if((lStartDelayTime > 0 && lStartDistance > 0) || lStartDistance == 0) //(當兩者都有值時以"移動前延遲"優先)  執行__移動前延遲
    {
        DecideLineStartMove(lX, lY, lZ, dAng, lStartDelayTime, lWorkVelociy, lAcceleration, lInitVelociy,1);//移動到起始點
        DecideLineMidMove(lX2, lY2, lZ2, dAng2, lMidDelayTime, lWorkVelociy, lAcceleration, lInitVelociy,1);//移動至中間點
    }
    else  //執行__設置距離
    {

        DecideLineStartMove(lBuffX, lBuffY, lZClose, dAng, 0, lWorkVelociy, lAcceleration, lInitVelociy,1);//移動到起始點
        CPoint cpNeedlePoint = W_GetNeedlePoint();//將當下位置轉換成針頭座標
        W_UpdateNeedleMotor_Needle(cpNeedlePoint.x, cpNeedlePoint.y, MO_ReadLogicPosition(2), MO_ReadLogicPositionW(), lX2, lY2, lZ2, dAng);//切點取值(使用預設每1度取一個點)
        lTime = CalPreglue(lStartDistance, lWorkVelociy, lAcceleration, lInitVelociy);
        /*======計時器到觸發中斷執行出膠，使用y中斷執行================*/
        if(!m_bIsStop)
        {
            CAction::m_YtimeOutGlueSet = TRUE;
            MO_TimerSetIntter(lTime, 0);//計時到跳至執行序
        }
        if(!m_bIsStop)
        {
            W_Line4DtoDo(lWorkVelociy, lAcceleration, lInitVelociy);//四軸連續插補
            PreventMoveError();//防止軸卡出錯
        }
        MO_Timer(0, 0, lMidDelayTime * 1000);
        MO_Timer(1, 0, lMidDelayTime * 1000);//線段點膠設定---(4)節點時間
        Sleep(1);//防止出錯，避免計時器初直為0
        while(MO_Timer(3, 0, 0))
        {
            if(m_bIsStop)
            {
                break;
            }
            Sleep(1);
        }
        PauseStopGlue();//暫停時停指塗膠(m_bIsPause=1)
    }

#endif
}
/*
*線段開始接線段結束動作
*輸入(線段開始X,Y,Z,結束點X,Y,Z,移動前延遲,開機前從起點移動距離(設置距離),關機後在結束點停留時間(停留時間),距離結束點多遠距離關機(關機距離),關機後的延遲時間(關機延遲),Z軸回升距離,Z軸回升型態,點膠結束設定(高速度),返回長度,z返回高度,返回速度(低速),返回類型,驅動速度,加速度,初速度)
*/
void CAction::DecideLineSToE(LONG lX, LONG lY, LONG lZ, DOUBLE dAng, LONG lX2, LONG lY2, LONG lZ2, DOUBLE dAng2,
                             LONG lStartDelayTime, LONG lStartDistance, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime,
                             LONG lZBackDistance, BOOL bZDisType, LONG lHighVelocity, LONG lDistance, LONG lHigh,
                             LONG lLowVelocity, int iType, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
    /*線段開始(x座標，y座標，z座標，W軸角度)
    LONG lX, LONG lY, LONG lZ, DOUBLE dAng
    */
    /*線段結束點(x座標，y座標，z座標，W軸角度)
    LONG lX2, LONG lY2, LONG lZ2, DOUBLE dAng
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
    //判斷W軸是否校正
    if(m_MachineOffSet.x == -99999 && m_MachineOffSet.y == -99999)
    {
        return;
    }
    //判斷使用針頭座標還是機械座標
    if(USEMODE_W == 1) //使用機械座標
    {
        CPoint cpbuf = W_GetNeedlePoint(lX, lY, dAng, 1);
        lX = cpbuf.x;
        lY = cpbuf.y;
        cpbuf = W_GetNeedlePoint(lX2, lY2, dAng2, 1);
        lX2 = cpbuf.x;
        lY2 = cpbuf.y;
    }
    /*******以下程式都會轉變使用針頭座標執行運動************************/
    //流程: 輸入針頭座標>運算針頭座標>轉換成機械座標輸出
    //LONG lNowZ = 0;
    LONG lXClose = 0, lYClose = 0, lZClose = 0, lLineClose = 0;
    LONG lBuffX = 0, lBuffY = 0, LineLength = 0;
    LONG lTime = 0;
    lLineClose = lStartDistance;
    LineGetToPoint(lXClose, lYClose, lZClose, lX2, lY2, lX, lY, lZ2, lZ, lLineClose);
    lBuffX = (-(lXClose - lX)) + lX;
    lBuffY = (-(lYClose - lY)) + lY;
    LineLength = LONG(sqrt(DOUBLE(pow(lX2 - lX, 2) + pow(lY2 - lY, 2))));
    if(lCloseDistance != 0 && lCloseDistance > LineLength)
    {
        lCloseDistance = LineLength;
    }
    if(bZDisType) //當使用相對位置的時候
    {
        LONG lbuf = m_TablelZ - lZBackDistance;//從平台最低往上相對距離
        lZBackDistance = lbuf;
        bZDisType = 0;//改為絕對位置
    }
    if(!bZDisType) //絕對位置
    {
        if(lZBackDistance > lZ)
        {
            lZBackDistance = lZ;
        }
        lZBackDistance = abs(lZBackDistance - lZ);
    }
    if(lHighVelocity == 0)
    {
        lHighVelocity = lWorkVelociy;
    }
    if(lLowVelocity == 0)
    {
        lLowVelocity = lWorkVelociy;
    }
    if((lStartDelayTime > 0 && lStartDistance > 0) || lStartDistance == 0) //(當兩者都有值時以"移動前延遲"優先)  執行__移動前延遲
    {
        DecideLineStartMove(lX, lY, lZ, dAng, lStartDelayTime, lWorkVelociy, lAcceleration, lInitVelociy,1);//移動到起始點
        CPoint cpNeedlePoint = W_GetNeedlePoint();//將當下位置轉換成針頭座標
        W_UpdateNeedleMotor_Needle(cpNeedlePoint.x, cpNeedlePoint.y, MO_ReadLogicPosition(2), MO_ReadLogicPositionW(), lX2, lY2, lZ2, dAng2);//切點取值(使用預設每1度取一個點)
        if(lCloseDistance != 0) //線段點膠設定---(5)關機距離
        {
            LONG lAddDis = CalPreglue(lWorkVelociy, lAcceleration, lInitVelociy);//加速度距離
            LONG lAddTime = CalPreglue(lWorkVelociy, lAcceleration, lInitVelociy);//加速度時間
            DOUBLE dSumPath = sqrt(pow(lX2 - lX, 2) + pow(lY2 - lY, 2) + pow(lZ2 - lZ, 2));//線段總長
            LONG lCloseTime =  1000000 * (LONG)round(((dSumPath - lCloseDistance - lAddDis) / (DOUBLE)lWorkVelociy));//關膠時間
            /*======計時器到觸發中斷執行斷膠，使用z中斷執行================*/
            if(!m_bIsStop)
            {
                CAction::m_ZtimeOutGlueSet = FALSE;
              //  _cwprintf(_T("End關機距離的時間=%lf \n"), (lCloseTime+ lAddTime) / 1000000.0);
                MO_TimerSetIntter(lCloseTime + lAddTime, 1);//計時到跳至執行序
            }
        }
        if(!m_bIsStop)
        {
            W_Line4DtoDo(lWorkVelociy, lAcceleration, lInitVelociy);//四軸連續插補__移動至結束點
            PreventMoveError();//防止軸卡出錯
        }
    }
    else
    {
        DecideLineStartMove(lBuffX, lBuffY, lZClose, dAng, lStartDelayTime, lWorkVelociy, lAcceleration, lInitVelociy,1);//移動到起始點
        CPoint cpNeedlePoint = W_GetNeedlePoint();//將當下位置轉換成針頭座標
        W_UpdateNeedleMotor_Needle(cpNeedlePoint.x, cpNeedlePoint.y, MO_ReadLogicPosition(2), MO_ReadLogicPositionW(), lX2, lY2, lZ2, dAng2);//切點取值(使用預設每1度取一個點)
        lTime = CalPreglue(lStartDistance, lWorkVelociy, lAcceleration, lInitVelociy);//計算提前距離出膠時間
        /*======計時器到觸發中斷執行出膠，使用y中斷執行================*/
        if(!m_bIsStop)
        {
            CAction::m_YtimeOutGlueSet = TRUE;
            MO_TimerSetIntter(lTime, 0);//計時到跳至執行序
        }
        if(lCloseDistance != 0) //線段點膠設定---(5)關機距離
        {
            LONG lAddDis = CalPreglue(lWorkVelociy, lAcceleration, lInitVelociy);//加速度距離
            LONG lAddTime = CalPreglue(lWorkVelociy, lAcceleration, lInitVelociy);//加速度時間
            DOUBLE dSumPath = sqrt(pow(lX2 - lBuffX, 2) + pow(lY2 - lBuffY, 2) + pow(lZ2 - lZClose, 2));//線段總長
            LONG lCloseTime = 1000000 * (LONG)round(((dSumPath - lCloseDistance - lAddDis) / (DOUBLE)lWorkVelociy));//關膠時間
            /*======計時器到觸發中斷執行斷膠，使用z中斷執行================*/
            if(!m_bIsStop)
            {
                CAction::m_ZtimeOutGlueSet = FALSE;
             //   _cwprintf(_T("End關機距離的時間=%lf \n"), (lCloseTime + lAddTime) / 1000000.0);
                MO_TimerSetIntter(lCloseTime + lAddTime, 1);//計時到跳至執行序
            }
        }
        if(!m_bIsStop)
        {
            W_Line4DtoDo(lWorkVelociy, lAcceleration, lInitVelociy);//四軸連續插補
            PreventMoveError();//防止軸卡出錯
        }
    }
    MO_StopGumming();//停止出膠
    GelatinizeBack(iType, lX2, lY2, lZ2, lX, lY, lDistance, lHigh, lZBackDistance, lLowVelocity, lHighVelocity, lAcceleration, lInitVelociy);//返回設定
    MO_Timer(0, 0, lCloseOffDelayTime * 1000);
    MO_Timer(1, 0, lCloseOffDelayTime * 1000);//線段點膠設定---(3)停留時間
    Sleep(1);//防止出錯，避免計時器初直為0
    while(MO_Timer(3, 0, 0))
    {
        if(m_bIsStop == 1)
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
void CAction::DecideCircle(LONG lX1, LONG lY1, LONG lZ1, DOUBLE dAng1, LONG lX2, LONG lY2, LONG lZ2, DOUBLE dAng2, LONG lWorkVelociy, LONG lInitVelociy)
{
    /*圓形(x座標，y座標，W軸角度)
    LONG lX1, LONG lY1,LONG lZ1, DOUBLE dAng1
    */
    /*圓形結束點(x座標，y座標，z座標，W軸角度)
    LONG lX2, LONG lY2,LONG lZ2, DOUBLE dAng2
    */
    /*系統參數(驅動速度，加速度，初速度)
    LONG lWorkVelociy, LONG lInitVelociy
    */
#ifdef MOVE
	//判斷W軸是否校正
	if (m_MachineOffSet.x == -99999 && m_MachineOffSet.y == -99999)
	{
		return;
	}
	//判斷使用針頭座標還是機械座標
	if (USEMODE_W == 1) //使用機械座標
	{
		CPoint cpbuf = W_GetNeedlePoint(lX1, lY1, dAng1, 1);
		lX1 = cpbuf.x;
		lY1 = cpbuf.y;
		cpbuf = W_GetNeedlePoint(lX2, lY2, dAng2, 1);
		lX2 = cpbuf.x;
		lY2 = cpbuf.y;
	}
	/*******以下程式都會轉變使用針頭座標執行運動************************/
	//流程: 輸入針頭座標>運算針頭座標>轉換成機械座標輸出
	CCircleFormula Circle;
	AxeSpace p1, p2, p3;

	p1 = { W_GetNeedlePoint().x, W_GetNeedlePoint().y,MO_ReadLogicPosition(2),MO_ReadLogicPositionW() };
	p2 = { lX1,lY1,lZ1,dAng1 };
	p3 = { lX2,lY2,lZ2,dAng2 };
	std::vector<AxeSpace> VecBuf;
	LONG cutRad = 1000;//以1mm，也就是1000um的弧長做切點
	if (lZ1 == lZ2&& lZ1 == MO_ReadLogicPosition(2))
	{
		Circle.CircleCutPath_2D_unit(p1, p2, p3, TRUE, cutRad, VecBuf);//2D切圓
	}
	else
	{
		Circle.m_cuttingLength = cutRad;//切圓弧單位長
		Circle.CircleCutPoint(p1, p2, p3, VecBuf); //3D切圓
	}
	DOUBLE SumAng = dAng2 - MO_ReadLogicPositionW();//W迴轉角度量(含+/-方向性)
	size_t num = VecBuf.size();
	DOUBLE num_Ang = 360.0 / (DOUBLE)num;//依圓弧插補點數切割360度
	num_Ang = floor(num_Ang / 0.036)*0.036;//依馬達 0.036 [deg/pulse] 轉換為合理的角度分量
	DOUBLE rest_Ang = (360.0 - num_Ang*(DOUBLE)num)*((SumAng>0) ? 1 : -1);//剩餘的角度
	DATA_4MOVE *pDataM = new DATA_4MOVE[num];
	DATA_4MOVE *pDataShift = pDataM;
	CPoint cpMpbuf(0, 0), cpMpLast(0, 0);
	int Fin_cnt = (int)floor(fabs(rest_Ang) / 0.072);//最後個數
	DOUBLE absAngle = MO_ReadLogicPositionW();

	for (size_t i = 0; i<VecBuf.size(); i++)
	{
		//Get 絕對角度
		if (Fin_cnt != 0)
		{
			//Get 絕對角度
			absAngle = absAngle + ((SumAng > 0) ? num_Ang + 0.072 : -num_Ang - 0.072);
			pDataShift->AngleW = ((SumAng > 0) ? num_Ang + 0.072 : -num_Ang - 0.072);
			Fin_cnt--;
		}
		else
		{
			absAngle = absAngle + ((SumAng > 0) ? num_Ang : -num_Ang);
			pDataShift->AngleW = ((SumAng > 0) ? num_Ang : -num_Ang);
		}
		//Get 機械絕對座標
		cpMpbuf = W_GetMachinePoint(VecBuf.at(i).x, VecBuf.at(i).y, absAngle, 1);

		//進行絕對/相對座標轉換
		if (i == 0)
		{
			pDataShift->EndPX = cpMpbuf.x - MO_ReadLogicPosition(0);
			pDataShift->EndPY = cpMpbuf.y - MO_ReadLogicPosition(1);
			pDataShift->EndPZ = VecBuf.at(i).z - MO_ReadLogicPosition(2);
			pDataShift->Distance = (LONG)sqrt(pow(pDataShift->EndPX, 2) + pow(pDataShift->EndPY, 2) + pow(pDataShift->EndPZ, 2));
			//_cwprintf(_T("%d,%d,%.3f\n"), pDataShift->EndPX, pDataShift->EndPY, pDataShift->AngleW);
			//TRACE(_T(",%d,%d,%.3f\n"), pDataShift->EndPX, pDataShift->EndPY, pDataShift->AngleW);
		}
		else
		{
			pDataShift->EndPX = cpMpbuf.x - cpMpLast.x;
			pDataShift->EndPY = cpMpbuf.y - cpMpLast.y;
			pDataShift->EndPZ = VecBuf.at(i).z - VecBuf.at(i - 1).z;
			pDataShift->Distance = (LONG)sqrt(pow(pDataShift->EndPX, 2) + pow(pDataShift->EndPY, 2) + pow(pDataShift->EndPZ, 2));
			//_cwprintf(_T("%d,%d,%.3f\n"), pDataShift->EndPX, pDataShift->EndPY, pDataShift->AngleW);
			// TRACE(_T(",%d,%d,%.3f\n"), pDataShift->EndPX, pDataShift->EndPY, pDataShift->AngleW);
		}
		cpMpLast = cpMpbuf;
		pDataShift++;
	}
	PauseDoGlue();//暫停恢復後繼續出膠(m_bIsPause=0)
	if (!m_bIsStop)
	{
		MO_DO4Curve(pDataM, num, lWorkVelociy);
		PreventMoveError();//防止驅動錯誤
	}
	pDataShift = NULL;
	delete[] pDataM;
	PauseStopGlue();//暫停時停指塗膠(m_bIsPause=1)
#endif
}
/*
*圓弧動作
*輸入(圓弧、圓弧結束點、線速、系統參數)
*/
void CAction::DecideArc(LONG lX1, LONG lY1, LONG lZ1, DOUBLE dAng1, LONG lX2, LONG lY2, LONG lZ2, DOUBLE dAng2, LONG lWorkVelociy, LONG lInitVelociy)
{
    /*圓弧(x座標，y座標，)
    LONG lX1, LONG lY1,LONG lZ1, DOUBLE dAng1
    */
    /*圓弧結束點(x座標，y座標，)
    LONG lX2, LONG lY2,LONG lZ2, DOUBLE dAng2
    */
    /*系統參數(驅動速度，加速度，初速度)
    LONG lWorkVelociy, LONG lInitVelociy
    */
#ifdef MOVE
	//判斷W軸是否校正
	if (m_MachineOffSet.x == -99999 && m_MachineOffSet.y == -99999)
	{
		return;
	}
	//判斷使用針頭座標還是機械座標
	if (USEMODE_W == 1) //使用機械座標
	{
		CPoint cpbuf = W_GetNeedlePoint(lX1, lY1, dAng1, 1);
		lX1 = cpbuf.x;
		lY1 = cpbuf.y;
		cpbuf = W_GetNeedlePoint(lX2, lY2, dAng2, 1);
		lX2 = cpbuf.x;
		lY2 = cpbuf.y;
	}
	/*******以下程式都會轉變使用針頭座標執行運動************************/
	//流程: 輸入針頭座標>運算針頭座標>轉換成機械座標輸出
	CCircleFormula Circle;
	AxeSpace p1, p2, p3;
	p1 = { W_GetNeedlePoint().x, W_GetNeedlePoint().y,MO_ReadLogicPosition(2),MO_ReadLogicPositionW() };
	p2 = { lX1,lY1,lZ1,dAng1 };
	p3 = { lX2,lY2,lZ2,dAng2 };
	std::vector<AxeSpace> VecBuf;
	LONG cutRad = 1000;//以1mm，也就是1000um的弧長做切點
	if (lZ1 == lZ2&&lZ1 == MO_ReadLogicPosition(2))
	{
		Circle.CircleCutPath_2D_unit(p1, p2, p3, false, cutRad, VecBuf);//2D切圓弧
	}
	else
	{
		Circle.m_cuttingLength = cutRad;//切圓弧單位長
		Circle.ArcCutPoint(p1, p2, p3, VecBuf);//3D切圓弧
	}
	DOUBLE SumAng = dAng2 - MO_ReadLogicPositionW();//W迴轉角度量(含+/-方向性)
	size_t num = VecBuf.size();
	DOUBLE num_Ang = fabs(SumAng) / (DOUBLE)num;//依圓弧插補點數切割迴轉角度量(不含+/-方向性)
	num_Ang = floor(num_Ang / 0.036)*0.036;//依馬達 0.036 [deg/pulse] 轉換為合理的角度分量(不含+/-方向性)
	DOUBLE rest_Ang = (fabs(SumAng) - num_Ang*(DOUBLE)num)*((SumAng>0) ? 1 : -1);//剩餘的角度(含+/-方向性)
	DATA_4MOVE *pDataM = new DATA_4MOVE[num];
	DATA_4MOVE *pDataShift = pDataM;
	CPoint cpMpbuf(0, 0), cpMpLast(0, 0);
	int Fin_cnt = (int)floor(fabs(rest_Ang) / 0.072);//最後個數
	DOUBLE absAngle = MO_ReadLogicPositionW();

	for (size_t i = 0; i<VecBuf.size(); i++)
	{
		if (Fin_cnt != 0)
		{
			//Get 絕對角度
			absAngle = absAngle + ((SumAng > 0) ? num_Ang + 0.072 : -num_Ang - 0.072);
			pDataShift->AngleW = ((SumAng > 0) ? num_Ang + 0.072 : -num_Ang - 0.072);
			Fin_cnt--;
		}
		else
		{
			absAngle = absAngle + ((SumAng > 0) ? num_Ang : -num_Ang);
			pDataShift->AngleW = ((SumAng > 0) ? num_Ang : -num_Ang);
		}
		//Get 機械絕對座標
		cpMpbuf = W_GetMachinePoint(VecBuf.at(i).x, VecBuf.at(i).y, absAngle, 1);

		//進行絕對/相對座標轉換
		if (i == 0)
		{
			pDataShift->EndPX = cpMpbuf.x - MO_ReadLogicPosition(0);
			pDataShift->EndPY = cpMpbuf.y - MO_ReadLogicPosition(1);
			pDataShift->EndPZ = VecBuf.at(i).z - MO_ReadLogicPosition(2);
			pDataShift->Distance = (LONG)sqrt(pow(pDataShift->EndPX, 2) + pow(pDataShift->EndPY, 2) + pow(pDataShift->EndPZ, 2));
			//_cwprintf(_T("%d,%d,%.3f\n"), pDataShift->EndPX, pDataShift->EndPY, pDataShift->AngleW);
			//TRACE(_T(",%d,%d,%.3f\n"), pDataShift->EndPX, pDataShift->EndPY, pDataShift->AngleW);
		}
		else
		{
			pDataShift->EndPX = cpMpbuf.x - cpMpLast.x;
			pDataShift->EndPY = cpMpbuf.y - cpMpLast.y;
			pDataShift->EndPZ = VecBuf.at(i).z - VecBuf.at(i - 1).z;
			pDataShift->Distance = (LONG)sqrt(pow(pDataShift->EndPX, 2) + pow(pDataShift->EndPY, 2) + pow(pDataShift->EndPZ, 2));
			if ((i == VecBuf.size() - 2) || (i == VecBuf.size() - 1))
			{
				_cwprintf(_T("lost_%d,%d,%.3f\n"), pDataShift->EndPX, pDataShift->EndPY, pDataShift->AngleW);
			}
			//_cwprintf(_T("%d,%d,%.3f\n"), pDataShift->EndPX, pDataShift->EndPY, pDataShift->AngleW);
			//TRACE(_T(",%d,%d,%.3f\n"), pDataShift->EndPX, pDataShift->EndPY, pDataShift->AngleW);
		}
		cpMpLast = cpMpbuf;
		pDataShift++;
	}
	PauseDoGlue();//暫停恢復後繼續出膠(m_bIsPause=0)
	if (!m_bIsStop)
	{
		MO_DO4Curve(pDataM, num, lWorkVelociy);
		PreventMoveError();//防止驅動錯誤
	}
	pDataShift = NULL;
	delete[] pDataM;
	PauseStopGlue();//暫停時停指塗膠(m_bIsPause=1)
#endif
}
/*線段開始到圓中間點動作
*(輸入參數:線段開始,圓形座標,圓形座標,移動前延遲,開機前從起點移動距離(設置距離),對線段中點的停留時間(節點時間ms),驅動速度,加速度,初速度)
*/
void CAction::DecideLineSToCirP(LONG lX1, LONG lY1, LONG lZ1, DOUBLE dAng1, LONG lX2, LONG lY2, LONG lZ2, DOUBLE dAng2, LONG lX3, LONG lY3, LONG lZ3, DOUBLE dAng3, LONG lStartDelayTime, LONG lStartDistance, LONG lMidDelayTime, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
    /*線段開始(x座標，y座標，z座標，線段起始點，W軸角度)
    LONG lX, LONG lY, LONG lZ, DOUBLE dAng
    */
    /*圓形點(x座標，y座標，z座標，W軸角度)
    LONG lX2, LONG lY2, LONG lZ2, DOUBLE dAng2
    */
    /*圓形點(x座標，y座標，z座標，W軸角度)
    LONG lX3, LONG lY3, LONG lZ3, DOUBLE dAng3
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
    //判斷W軸是否校正
    if(m_MachineOffSet.x == -99999 && m_MachineOffSet.y == -99999)
    {
        return;
    }
    //判斷使用針頭座標還是機械座標
    if(USEMODE_W == 1) //使用機械座標
    {
        CPoint cpbuf = W_GetNeedlePoint(lX1, lY1, dAng1, 1);
        lX1 = cpbuf.x;
        lY1 = cpbuf.y;
        cpbuf = W_GetNeedlePoint(lX2, lY2, dAng2, 1);
        lX2 = cpbuf.x;
        lY2 = cpbuf.y;
        cpbuf = W_GetNeedlePoint(lX3, lY3, dAng3, 1);
        lX3 = cpbuf.x;
        lY3 = cpbuf.y;
    }
    /*******以下程式都會轉變使用針頭座標執行運動************************/
    //流程: 輸入針頭座標>運算針頭座標>轉換成機械座標輸出
    //DOUBLE dSumPath = 0, dRad = 0, angle = 0;//線段總長度、半徑、斷膠時間、角度(弧度)
    //BOOL bRev = 0;//正逆轉
    //CCircleFormula Circle;
    //AxeSpace p1, p2, p3;
    //p1 = { W_GetNeedlePoint().x, W_GetNeedlePoint().y,MO_ReadLogicPosition(2),MO_ReadLogicPositionW() };
    //p2 = { lX1,lY1,lZ1,dAng1 };
    //p3 = { lX2,lY2,lZ2,dAng2 };
    //DPoint pCen, dp1, dp2, dp3;
    //dp1 = { W_GetNeedlePoint().x, W_GetNeedlePoint().y,MO_ReadLogicPosition(2),MO_ReadLogicPositionW() };
    //dp2 = { lX1,lY1,lZ1,dAng1 };
    //dp3 = { lX2,lY2,lZ2,dAng2 };
    //std::vector<AxeSpace> VecBuf;
    //LONG cutRad = 1000;//以1mm，也就是1000um的弧長做切點
    //if (lZ1 == lZ2)
    //{
    //    //求二維圓心與半徑
    //    pCen = Circle.ArcCentCalculation_2D(dp1, dp2, dp3, dRad, angle, bRev);
    //    Circle.CircleCutPath_2D_unit(p1, p2, p3, TRUE, cutRad, VecBuf);//2D切圓
    //}
    //else
    //{
    //    //求三維圓心與半徑
    //    Circle.ArcCentCalculation_2D(dp1, dp2, dp3, dRad, angle, bRev);//使用2d求角度與正逆轉
    //    pCen = Circle.SpaceCircleCenterCalculation(dp1, dp2, dp3);
    //    dRad = sqrt(pow(pCen.x - p2.x, 2) + pow(pCen.y - p2.y, 2) + pow(pCen.z - p2.z, 2));
    //    Circle.CircleCutPoint(p1, p2, p3, VecBuf); //3d切圓
    //}
    //DOUBLE SumAng = dAng2 - MO_ReadLogicPositionW();
    //size_t num = VecBuf.size();
    //DOUBLE num_Ang = 360 / (DOUBLE)num;
    //DATA_4MOVE *pDataM = new DATA_4MOVE[num];
    //DATA_4MOVE *pDataShift = pDataM;
    //for (size_t i = 0; i<VecBuf.size() - 1; i++)
    //{
    //    if (i == 0)
    //    {
    //        pDataShift->EndPX = W_GetMachinePoint(VecBuf.at(i).x, VecBuf.at(i).y, MO_ReadLogicPositionW(), 1).x - MO_ReadLogicPosition(0);
    //        pDataShift->EndPY = W_GetMachinePoint(VecBuf.at(i).x, VecBuf.at(i).y, MO_ReadLogicPositionW(), 1).y - MO_ReadLogicPosition(1);
    //        pDataShift->EndPZ = VecBuf.at(i).z - MO_ReadLogicPosition(2);
    //        pDataShift->AngleW = MO_ReadLogicPositionW();
    //        pDataShift->Distance = pDataShift->Distance = 0;
    //        //_cwprintf(_T("%d,%d,%.3f\n"), pDataShift->EndPX, pDataShift->EndPY, pDataShift->AngleW);
    //        //TRACE(_T(",%d,%d,%.3f\n"), pDataShift->EndPX, pDataShift->EndPY, pDataShift->AngleW);
    //    }
    //    else
    //    {

    //        pDataShift->EndPX = W_GetMachinePoint(VecBuf.at(i).x, VecBuf.at(i).y, MO_ReadLogicPositionW() + (num_Ang * i), 1).x - VecBuf.at(i - 1).x;
    //        pDataShift->EndPY = W_GetMachinePoint(VecBuf.at(i).x, VecBuf.at(i).y, MO_ReadLogicPositionW() + (num_Ang * i), 1).y - VecBuf.at(i - 1).y;
    //        pDataShift->EndPZ = VecBuf.at(i).z - VecBuf.at(i - 1).z;
    //        pDataShift->AngleW = MO_ReadLogicPositionW() + (num_Ang * i);
    //        pDataShift->Distance = pDataShift->Distance = 0;
    //        //_cwprintf(_T("%d,%d,%.3f\n"), pDataShift->EndPX, pDataShift->EndPY, pDataShift->AngleW);
    //        // TRACE(_T(",%d,%d,%.3f\n"), pDataShift->EndPX, pDataShift->EndPY, pDataShift->AngleW);

    //    }
    //    pDataShift++;
    //}
    //LONG lStartX = 0, lStartY = 0, lTime = 0;
    //ArcGetToPoint(lStartX, lStartY, lStartDistance, lX1, lY1, pCen.x, pCen.y, LONG(dRad), bRev);//算出起始點
    //DecideLineStartMove(lStartX, lStartY, MO_ReadLogicPosition(2), dAng1, 0, lWorkVelociy, lAcceleration, lInitVelociy);
    //CPoint cpNeedlePoint = W_GetNeedlePoint();//將當下位置轉換成針頭座標
    //W_UpdateNeedleMotor_Needle(cpNeedlePoint.x, cpNeedlePoint.y, MO_ReadLogicPosition(2), MO_ReadLogicPositionW(), lStartX, lStartY, lZ1, dAng1);//切點取值(使用預設每1度取一個點)
    //if (!m_bIsStop)
    //{
    //    W_Line4DtoDo(lWorkVelociy, lAcceleration, lInitVelociy);//四軸連續插補
    //    PreventMoveError();//防止軸卡出錯
    //}

    //lTime = CalPreglue(lStartDistance, lWorkVelociy, lAcceleration, lInitVelociy);//計算提前距離出膠時間
    ///*======計時器到觸發中斷執行出膠，使用y中斷執行================*/
    //if (!m_bIsStop)
    //{
    //    CAction::m_YtimeOutGlueSet = TRUE;
    //    MO_TimerSetIntter(lTime, 0);//計時到跳至執行序
    //}

    //LONG lXClose = 0, lYClose = 0, lZClose = 0, lLineClose = 0;
    //LONG lBuffX = 0, lBuffY = 0;
    //LONG lTime = 0;
    //lLineClose = lStartDistance;
    //LineGetToPoint(lXClose, lYClose, lZClose, lX2, lY2, lX, lY, lZ2, lZ, lLineClose);
    //lBuffX = (-(lXClose - lX)) + lX;
    //lBuffY = (-(lYClose - lY)) + lY;
    //if ((lStartDelayTime > 0 && lStartDistance > 0) || lStartDistance == 0)//(當兩者都有值時以"移動前延遲"優先)  執行__移動前延遲
    //{
    //    DecideLineStartMove(lX, lY, lZ, dAng, lStartDelayTime, lWorkVelociy, lAcceleration, lInitVelociy);//移動到起始點
    //    DecideLineMidMove(lX2, lY2, lZ2, dAng2, lMidDelayTime, lWorkVelociy, lAcceleration, lInitVelociy);//移動至中間點
    //}
    //else  //執行__設置距離
    //{

    //    DecideLineStartMove(lBuffX, lBuffY, lZClose, dAng, 0, lWorkVelociy, lAcceleration, lInitVelociy);//移動到起始點
    //    CPoint cpNeedlePoint = W_GetNeedlePoint();//將當下位置轉換成針頭座標
    //    W_UpdateNeedleMotor_Needle(cpNeedlePoint.x, cpNeedlePoint.y, MO_ReadLogicPosition(2), MO_ReadLogicPositionW(), lX2, lY2, lZ2, dAng);//切點取值(使用預設每1度取一個點)
    //    lTime = CalPreglue(lStartDistance, lWorkVelociy, lAcceleration, lInitVelociy);
    //    /*======計時器到觸發中斷執行出膠，使用y中斷執行================*/
    //    if (!m_bIsStop)
    //    {
    //        CAction::m_YtimeOutGlueSet = TRUE;
    //        MO_TimerSetIntter(lTime, 0);//計時到跳至執行序
    //    }
    //    if (!m_bIsStop)
    //    {
    //        W_Line4DtoDo(lWorkVelociy, lAcceleration, lInitVelociy);//四軸連續插補
    //        PreventMoveError();//防止軸卡出錯
    //    }
    //    MO_Timer(0, 0, lMidDelayTime * 1000);
    //    MO_Timer(1, 0, lMidDelayTime * 1000);//線段點膠設定---(4)節點時間
    //    Sleep(1);//防止出錯，避免計時器初直為0
    //    while (MO_Timer(3, 0, 0))
    //    {
    //        if (m_bIsStop)
    //        {
    //            break;
    //        }
    //        Sleep(1);
    //    }
    //    PauseStopGlue();//暫停時停指塗膠(m_bIsPause=1)
    //}

#endif
}

/*
*圓中間點到結束點
*(輸入參數:圓形1，圓形2，線段結束點，線段點膠設定，返回設定，Z軸工作高度設定，點膠結束設定，系統參數)
*內容:輸入的結束點會判斷是否與現在位置相同，做出相對應的動作
*/
void CAction::DecideCirclePToEnd(LONG lX1, LONG lY1, LONG lZ1, DOUBLE dAng1, LONG lX2, LONG lY2, LONG lZ2, DOUBLE dAng2, LONG lX3, LONG lY3, LONG lZ3, DOUBLE dAng3, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime, LONG lZBackDistance, BOOL bZDisType, LONG lHighVelocity, LONG lDistance, LONG lHigh, LONG lLowVelocity, int iType, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
    /*圓形(x座標，y座標，z座標，W軸角度)
    LONG lX1, LONG lY1, LONG lZ1, DOUBLE dAng1
    */
    /*圓形結束點(x座標，y座標，z座標，W軸角度)
    LONG lX2, LONG lY2, LONG lZ2, DOUBLE dAng2
    */
    /*線段結束點(x座標，y座標，z座標，W軸角度)
    LONG lX3, LONG lY3, LONG lZ3, DOUBLE dAng3
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
	//判斷W軸是否校正
	if (m_MachineOffSet.x == -99999 && m_MachineOffSet.y == -99999)
	{
		return;
	}
	//判斷使用針頭座標還是機械座標
	if (USEMODE_W == 1) //使用機械座標
	{
		CPoint cpbuf = W_GetNeedlePoint(lX1, lY1, dAng1, 1);
		lX1 = cpbuf.x;
		lY1 = cpbuf.y;
		cpbuf = W_GetNeedlePoint(lX2, lY2, dAng2, 1);
		lX2 = cpbuf.x;
		lY2 = cpbuf.y;
		cpbuf = W_GetNeedlePoint(lX3, lY3, dAng3, 1);
		lX3 = cpbuf.x;
		lY3 = cpbuf.y;
	}
	/*******以下程式都會轉變使用針頭座標執行運動************************/
	//流程: 輸入針頭座標>運算針頭座標>轉換成機械座標輸出
	DOUBLE dSumPath = 0, dRad = 0, dCloseTime = 0, angle = 0;;//線段總長度、半徑、斷膠時間、角度
	if (lHighVelocity == 0)
	{
		lHighVelocity = lWorkVelociy;
	}
	if (lLowVelocity == 0)
	{
		lLowVelocity = lWorkVelociy;
	}
	if (bZDisType) //當使用相對位置的時候
	{
		LONG lbuf = m_TablelZ - lZBackDistance;//從平台最低往上相對距離
		lZBackDistance = lbuf;
		bZDisType = 0;//改為絕對位置
	}
	if (!bZDisType) //絕對位置
	{
		if (lZBackDistance > MO_ReadLogicPosition(2))
		{
			lZBackDistance = MO_ReadLogicPosition(2);
		}
		lZBackDistance = abs(lZBackDistance - MO_ReadLogicPosition(2));
	}
	CCircleFormula Circle;
	AxeSpace p1, p2, p3;
	p1 = { W_GetNeedlePoint().x, W_GetNeedlePoint().y,MO_ReadLogicPosition(2),MO_ReadLogicPositionW() };
	p2 = { lX1,lY1,lZ1,dAng1 };
	p3 = { lX2,lY2,lZ2,dAng2 };
	DPoint pCen, dp1, dp2, dp3;
	dp1 = { (DOUBLE)W_GetNeedlePoint().x, (DOUBLE)W_GetNeedlePoint().y,(DOUBLE)MO_ReadLogicPosition(2),MO_ReadLogicPositionW() };
	dp2 = { (DOUBLE)(DOUBLE)lX1,(DOUBLE)lY1,(DOUBLE)lZ1,dAng1 };
	dp3 = { (DOUBLE)(DOUBLE)lX2,(DOUBLE)lY2,(DOUBLE)lZ2,dAng2 };
	std::vector<AxeSpace> VecBuf;
	LONG cutRad = 1000;//以1mm，也就是1000um的弧長做切點
	if (lZ1 == lZ2&& lZ1 == MO_ReadLogicPosition(2))
	{
		//求二維圓心、半徑、角度
		pCen = Circle.ArcCentCalculation_2D(dp1, dp2, dp3, dRad, angle);
		Circle.CircleCutPath_2D_unit(p1, p2, p3, TRUE, cutRad, VecBuf);//2D切圓
	}
	else
	{
		//求三維圓心、半徑、角度
		Circle.m_cuttingLength = cutRad;//切圓弧單位長
		pCen = Circle.SpaceCircleCenterCalculation(dp1, dp2, dp3, dRad, angle);
		angle = 360.0;
		Circle.CircleCutPoint(p1, p2, p3, VecBuf); //3D切圓
	}
	DOUBLE SumAng = dAng2 - MO_ReadLogicPositionW();//W迴轉角度量(含+/-方向性)
	size_t num = VecBuf.size();
	DOUBLE num_Ang = 360.0 / (DOUBLE)num;//依圓弧插補點數切割360度
	num_Ang = floor(num_Ang / 0.036)*0.036;//依馬達 0.036 [deg/pulse] 轉換為合理的角度分量
	DOUBLE rest_Ang = (360.0 - num_Ang*(DOUBLE)num)*((SumAng>0) ? 1 : -1);//剩餘的角度
	DATA_4MOVE *pDataM = new DATA_4MOVE[num];
	DATA_4MOVE *pDataShift = pDataM;
	CPoint cpMpbuf(0, 0), cpMpLast(0, 0);
	int Fin_cnt = (int)floor(fabs(rest_Ang) / 0.072);//最後個數
	DOUBLE absAngle = MO_ReadLogicPositionW();

	for (size_t i = 0; i<VecBuf.size(); i++)
	{
		//Get 絕對角度
		if (Fin_cnt != 0)
		{
			//Get 絕對角度
			absAngle = absAngle + ((SumAng > 0) ? num_Ang + 0.072 : -num_Ang - 0.072);
			pDataShift->AngleW = ((SumAng > 0) ? num_Ang + 0.072 : -num_Ang - 0.072);
			Fin_cnt--;
		}
		else
		{
			absAngle = absAngle + ((SumAng > 0) ? num_Ang : -num_Ang);
			pDataShift->AngleW = ((SumAng > 0) ? num_Ang : -num_Ang);
		}
		//Get 機械絕對座標
		cpMpbuf = W_GetMachinePoint(VecBuf.at(i).x, VecBuf.at(i).y, absAngle, 1);

		//進行絕對/相對座標轉換
		if (i == 0)
		{
			pDataShift->EndPX = cpMpbuf.x - MO_ReadLogicPosition(0);
			pDataShift->EndPY = cpMpbuf.y - MO_ReadLogicPosition(1);
			pDataShift->EndPZ = VecBuf.at(i).z - MO_ReadLogicPosition(2);
			pDataShift->Distance = (LONG)sqrt(pow(pDataShift->EndPX, 2) + pow(pDataShift->EndPY, 2) + pow(pDataShift->EndPZ, 2));
			//_cwprintf(_T("%d,%d,%.3f\n"), pDataShift->EndPX, pDataShift->EndPY, pDataShift->AngleW);
			//TRACE(_T(",%d,%d,%.3f\n"), pDataShift->EndPX, pDataShift->EndPY, pDataShift->AngleW);
		}
		else
		{
			pDataShift->EndPX = cpMpbuf.x - cpMpLast.x;
			pDataShift->EndPY = cpMpbuf.y - cpMpLast.y;
			pDataShift->EndPZ = VecBuf.at(i).z - VecBuf.at(i - 1).z;
			pDataShift->Distance = (LONG)sqrt(pow(pDataShift->EndPX, 2) + pow(pDataShift->EndPY, 2) + pow(pDataShift->EndPZ, 2));
			//_cwprintf(_T("%d,%d,%.3f\n"), pDataShift->EndPX, pDataShift->EndPY, pDataShift->AngleW);
			// TRACE(_T(",%d,%d,%.3f\n"), pDataShift->EndPX, pDataShift->EndPY, pDataShift->AngleW);
		}
		cpMpLast = cpMpbuf;
		pDataShift++;
	}
	PauseDoGlue();//暫停恢復後繼續出膠(m_bIsPause=0)
	if ((W_GetNeedlePoint().x >= lX3 - 5) && (W_GetNeedlePoint().x <= lX3 + 5) && (W_GetNeedlePoint().y >= lY3 - 5) && (W_GetNeedlePoint().y <= lY3 + 5))  //表示結束點在起始點上
	{
		if (lCloseDistance != 0)  //線段點膠設定---(5)關機距離
		{
			dSumPath = M_PI * dRad*2.0;//移動的總長度
			if (lCloseDistance != 0 && lCloseDistance > (LONG)round(dSumPath))
			{
				lCloseDistance = (LONG)round(dSumPath);
			}
			dCloseTime = 1000000 * (((LONG)round(dSumPath) - lCloseDistance) / (DOUBLE)lWorkVelociy);
			/*======計時器到觸發中斷執行斷膠，使用z中斷執行================*/
			if (!m_bIsStop)
			{
				CAction::m_ZtimeOutGlueSet = FALSE;
				_cwprintf(_T("End關機距離的時間=%l.3f \n"), DOUBLE(dCloseTime / 1000000.0));
				MO_TimerSetIntter((LONG)round(dCloseTime), 1);//計時到跳至執行序
			}
		}
		if (!m_bIsStop)
		{
			MO_DO4Curve(pDataM, num, lWorkVelociy);//執行四軸插補
			PreventMoveError();//防止驅動錯誤
		}
		pDataShift = NULL;
		delete[] pDataM;
		if (lCloseDistance == 0) //當沒有關機距離時有關機延遲就會執行
		{
			MO_Timer(0, 0, lCloseONDelayTime * 1000);
			MO_Timer(1, 0, lCloseONDelayTime * 1000);//線段點膠設定---(6)關機延遲
			Sleep(1);//防止出錯，避免計時器初直為0
			while (MO_Timer(3, 0, 0))
			{
				if (m_bIsStop == 1)
				{
					break;
				}
				Sleep(1);
			}
		}
		MO_StopGumming();//停止出膠
						 //返回設定的參考點(結束點是lx3,ly3,出發參考點使用圓2)
		GelatinizeBack(iType, lX3, lY3, MO_ReadLogicPosition(2), lX2, lY2, lDistance, lHigh, lZBackDistance, lLowVelocity, lHighVelocity, lAcceleration, lInitVelociy);//返回設定
	}
	else//表示結束點不在圓上
	{
		if (!m_bIsStop)
		{
			MO_DO4Curve(pDataM, num, lWorkVelociy);//執行四軸插補
			PreventMoveError();//防止驅動錯誤
		}
		pDataShift = NULL;
		delete[] pDataM;
		CPoint cpNeedlePoint = W_GetNeedlePoint();//將當下位置轉換成針頭座標
		W_UpdateNeedleMotor_Needle(cpNeedlePoint.x, cpNeedlePoint.y, MO_ReadLogicPosition(2), MO_ReadLogicPositionW(), lX3, lY3, lZ3, dAng3);//切點取值(使用預設每1度取一個點)
		if (lCloseDistance != 0)  //線段點膠設定---(5)關機距離
		{
			dSumPath = sqrt(pow(lX3 - W_GetNeedlePoint().x, 2) + pow(lY3 - W_GetNeedlePoint().y, 2) + pow(lZ3 - MO_ReadLogicPosition(2), 2));;//移動的總長度
			if (lCloseDistance != 0 && lCloseDistance > (LONG)round(dSumPath))
			{
				lCloseDistance = (LONG)round(dSumPath);
			}
			dCloseTime = 1000000 * (((LONG)round(dSumPath) - lCloseDistance) / (DOUBLE)lWorkVelociy);
			/*======計時器到觸發中斷執行斷膠，使用z中斷執行================*/
			if (!m_bIsStop)
			{
				CAction::m_ZtimeOutGlueSet = FALSE;
				_cwprintf(_T("End關機距離的時間=%l.3f \n"), DOUBLE(dCloseTime / 1000000.0));
				MO_TimerSetIntter((LONG)round(dCloseTime), 1);//計時到跳至執行序
			}
		}
		if (!m_bIsStop)
		{
			W_Line4DtoDo(lWorkVelociy, lAcceleration, lInitVelociy);//四軸連續插補
			PreventMoveError();//防止軸卡出錯
		}
		if (lCloseDistance == 0) //當沒有關機距離時有關機延遲就會執行
		{
			MO_Timer(0, 0, lCloseONDelayTime * 1000);
			MO_Timer(1, 0, lCloseONDelayTime * 1000);//線段點膠設定---(6)關機延遲
			Sleep(1);//防止出錯，避免計時器初直為0
			while (MO_Timer(3, 0, 0))
			{
				if (m_bIsStop == 1)
				{
					break;
				}
				Sleep(1);
			}
		}
		MO_StopGumming();//停止出膠
						 //返回設定的參考點(結束點是lx3,ly3,出發參考點使用圓2)
		GelatinizeBack(iType, lX3, lY3, MO_ReadLogicPosition(2), W_GetNeedlePoint().x, W_GetNeedlePoint().y, lDistance, lHigh, lZBackDistance, lLowVelocity, lHighVelocity, lAcceleration, lInitVelociy);//返回設定
	}
	MO_Timer(0, 0, lCloseOffDelayTime * 1000);
	MO_Timer(1, 0, lCloseOffDelayTime * 1000);//線段點膠設定---(3)停留時間
	Sleep(1);//防止出錯，避免計時器初直為0
	while (MO_Timer(3, 0, 0))
	{
		if (m_bIsStop == 1)
		{
			break;
		}
	}
#endif
}
/*
*圓弧中間點到結束
*(輸入參數:圓弧，線段結束點，線段點膠設定，返回設定，Z軸工作高度設定，點膠結束設定，系統參數)
*內容:輸入的圓弧是連接結束點，做出相對應的動作
*/
void CAction::DecideArclePToEnd(LONG lX1, LONG lY1, LONG lZ1, DOUBLE dAng1, LONG lX2, LONG lY2, LONG lZ2, DOUBLE dAng2, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime, LONG lZBackDistance, BOOL bZDisType, LONG lHighVelocity, LONG lDistance, LONG lHigh, LONG lLowVelocity, int iType, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
    /*圓弧(x座標，y座標，z座標，W軸角度)
    LONG lX1, LONG lY1, LONG lZ1, DOUBLE dAng1
    */
    /*線段結束點(x座標，y座標，z座標，W軸角度)
    LONG lX2, LONG lY2, LONG lZ2, DOUBLE dAng2
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
	//判斷W軸是否校正
	if (m_MachineOffSet.x == -99999 && m_MachineOffSet.y == -99999)
	{
		return;
	}
	//判斷使用針頭座標還是機械座標
	if (USEMODE_W == 1) //使用機械座標
	{
		CPoint cpbuf = W_GetNeedlePoint(lX1, lY1, dAng1, 1);
		lX1 = cpbuf.x;
		lY1 = cpbuf.y;
		cpbuf = W_GetNeedlePoint(lX2, lY2, dAng2, 1);
		lX2 = cpbuf.x;
		lY2 = cpbuf.y;
	}
	/*******以下程式都會轉變使用針頭座標執行運動************************/
	//流程: 輸入針頭座標>運算針頭座標>轉換成機械座標輸出
	DOUBLE dSumPath = 0, dRad = 0, dCloseTime = 0, angle = 0;//線段總長度、半徑、斷膠時間、角度(弧度)
	if (lHighVelocity == 0)
	{
		lHighVelocity = lWorkVelociy;
	}
	if (lLowVelocity == 0)
	{
		lLowVelocity = lWorkVelociy;
	}
	if (bZDisType) //當使用相對位置的時候
	{
		LONG lbuf = m_TablelZ - lZBackDistance;//從平台最低往上相對距離
		lZBackDistance = lbuf;
		bZDisType = 0;//改為絕對位置
	}
	if (!bZDisType) //絕對位置
	{
		if (lZBackDistance > MO_ReadLogicPosition(2))
		{
			lZBackDistance = MO_ReadLogicPosition(2);
		}
		lZBackDistance = abs(lZBackDistance - MO_ReadLogicPosition(2));
	}
	CCircleFormula Circle;
	AxeSpace p1, p2, p3;
	p1 = { W_GetNeedlePoint().x, W_GetNeedlePoint().y,MO_ReadLogicPosition(2),MO_ReadLogicPositionW() };
	p2 = { lX1,lY1,lZ1,dAng1 };
	p3 = { lX2,lY2,lZ2,dAng2 };
	DPoint pCen, dp1, dp2, dp3;
	dp1 = { (DOUBLE)W_GetNeedlePoint().x,(DOUBLE)W_GetNeedlePoint().y,(DOUBLE)MO_ReadLogicPosition(2),MO_ReadLogicPositionW() };
	dp2 = { (DOUBLE)lX1,(DOUBLE)lY1,(DOUBLE)lZ1,dAng1 };
	dp3 = { (DOUBLE)lX2,(DOUBLE)lY2,(DOUBLE)lZ2,dAng2 };
	std::vector<AxeSpace> VecBuf;
	LONG cutRad = 1000;//以1mm，也就是1000um的弧長做切點
	if (lZ1 == lZ2&&lZ1 == MO_ReadLogicPosition(2))
	{
		//求二維圓心與半徑
		pCen = Circle.ArcCentCalculation_2D(dp1, dp2, dp3, dRad, angle);
		Circle.CircleCutPath_2D_unit(p1, p2, p3, FALSE, cutRad, VecBuf);//2D切圓
	}
	else
	{
		//求三維圓心與半徑
		pCen = Circle.SpaceCircleCenterCalculation(dp1, dp2, dp3, dRad, angle);
		Circle.m_cuttingLength = cutRad;//切圓弧單位長
		Circle.ArcCutPoint(p1, p2, p3, VecBuf); //3D切圓; VecBuf存針頭絕對座標
	}
	DOUBLE SumAng = dAng2 - MO_ReadLogicPositionW();//W迴轉角度量(含+/-方向性)
	size_t num = VecBuf.size();
	DOUBLE num_Ang = fabs(SumAng) / (DOUBLE)num;//依圓弧插補點數切割迴轉角度量
	num_Ang = floor(num_Ang / 0.036)*0.036;//依馬達 0.036 [deg/pulse] 轉換為合理的角度分量
	DOUBLE rest_Ang = (fabs(SumAng) - num_Ang*(DOUBLE)num)*((SumAng>0) ? 1 : -1);//剩餘的角度
	DATA_4MOVE *pDataM = new DATA_4MOVE[num];
	DATA_4MOVE *pDataShift = pDataM;
	CPoint cpMpbuf(0, 0), cpMpLast(0, 0);
	int Fin_cnt = (int)floor(fabs(rest_Ang) / 0.072);//最後個數
	DOUBLE absAngle = MO_ReadLogicPositionW();

	//機械絕對座標=>機械相對座標
	for (size_t i = 0; i<VecBuf.size(); i++)
	{
		if (Fin_cnt != 0)
		{
			//Get 絕對角度
			absAngle = absAngle + ((SumAng > 0) ? num_Ang + 0.072 : -num_Ang - 0.072);
			pDataShift->AngleW = ((SumAng > 0) ? num_Ang + 0.072 : -num_Ang - 0.072);
			Fin_cnt--;
		}
		else
		{
			absAngle = absAngle + ((SumAng > 0) ? num_Ang : -num_Ang);
			pDataShift->AngleW = ((SumAng > 0) ? num_Ang : -num_Ang);
		}
		//Get 機械絕對座標
		cpMpbuf = W_GetMachinePoint(VecBuf.at(i).x, VecBuf.at(i).y, absAngle, 1);

		//進行絕對/相對座標轉換
		if (i == 0)
		{
			pDataShift->EndPX = cpMpbuf.x - MO_ReadLogicPosition(0);
			pDataShift->EndPY = cpMpbuf.y - MO_ReadLogicPosition(1);
			pDataShift->EndPZ = VecBuf.at(i).z - MO_ReadLogicPosition(2);
			pDataShift->Distance = (LONG)sqrt(pow(pDataShift->EndPX, 2) + pow(pDataShift->EndPY, 2) + pow(pDataShift->EndPZ, 2));
			//_cwprintf(_T("%d,%d,%.3f\n"), pDataShift->EndPX, pDataShift->EndPY, pDataShift->AngleW);
			//TRACE(_T(",%d,%d,%.3f\n"), pDataShift->EndPX, pDataShift->EndPY, pDataShift->AngleW);
		}
		else
		{
			pDataShift->EndPX = cpMpbuf.x - cpMpLast.x;
			pDataShift->EndPY = cpMpbuf.y - cpMpLast.y;
			pDataShift->EndPZ = VecBuf.at(i).z - VecBuf.at(i - 1).z;
			pDataShift->Distance = (LONG)sqrt(pow(pDataShift->EndPX, 2) + pow(pDataShift->EndPY, 2) + pow(pDataShift->EndPZ, 2));
			if ((i == VecBuf.size() - 2) || (i == VecBuf.size() - 1))
			{
				_cwprintf(_T("lost_%d,%d,%.3f\n"), pDataShift->EndPX, pDataShift->EndPY, pDataShift->AngleW);
			}
			//_cwprintf(_T("%d,%d,%.3f\n"), pDataShift->EndPX, pDataShift->EndPY, pDataShift->AngleW);
			// TRACE(_T(",%d,%d,%.3f\n"), pDataShift->EndPX, pDataShift->EndPY, pDataShift->AngleW);
		}
		//TRACE(_T(",%d,%d,%d,%.3f\n"), pDataShift->EndPX, pDataShift->EndPY, pDataShift->EndPZ, pDataShift->AngleW);
		//TRACE(_T(",%d,%d,%d,%.3f\n"), VecBuf.at(i).x, VecBuf.at(i).y, VecBuf.at(i).z, VecBuf.at(i).w);
		//_cwprintf(_T(",%d,%d,%d,%.3f\n"), pDataShift->EndPX, pDataShift->EndPY, pDataShift->EndPZ, pDataShift->AngleW);
		cpMpLast = cpMpbuf;
		pDataShift++;
	}
	PauseDoGlue();//暫停恢復後繼續出膠(m_bIsPause=0)
	if (lCloseDistance != 0)  //線段點膠設定---(5)關機距離
	{
		dSumPath = M_PI * dRad * (angle / 180.0);//移動的總長度
		if (lCloseDistance != 0 && lCloseDistance > (LONG)round(dSumPath))
		{
			lCloseDistance = (LONG)round(dSumPath);
		}
		dCloseTime = 1000000 * (((LONG)round(dSumPath) - lCloseDistance) / (DOUBLE)lWorkVelociy);
		/*======計時器到觸發中斷執行斷膠，使用z中斷執行================*/
		if (!m_bIsStop)
		{
			CAction::m_ZtimeOutGlueSet = FALSE;
			_cwprintf(_T("End關機距離的時間=%l.3f \n"), DOUBLE(dCloseTime / 1000000.0));
			MO_TimerSetIntter((LONG)round(dCloseTime), 1);//計時到跳至執行序
		}
	}
	if (!m_bIsStop)
	{
		MO_DO4Curve(pDataM, num, lWorkVelociy);//執行四軸插補
		PreventMoveError();//防止驅動錯誤
	}
	pDataShift = NULL;
	delete[] pDataM;
	if (lCloseDistance == 0) //當沒有關機距離時有關機延遲就會執行
	{
		MO_Timer(0, 0, lCloseONDelayTime * 1000);
		MO_Timer(1, 0, lCloseONDelayTime * 1000);//線段點膠設定---(6)關機延遲
		Sleep(1);//防止出錯，避免計時器初直為0
		while (MO_Timer(3, 0, 0))
		{
			if (m_bIsStop == 1)
			{
				break;
			}
			Sleep(1);
		}
	}
	MO_StopGumming();//停止出膠
					 //返回設定的參考點(結束點是lx3,ly3,出發參考點使用圓2)
	GelatinizeBack(iType, lX2, lY2, MO_ReadLogicPosition(2), lX1, lY1, lDistance, lHigh, lZBackDistance, lLowVelocity, lHighVelocity, lAcceleration, lInitVelociy);//返回設定
	MO_Timer(0, 0, lCloseOffDelayTime * 1000);
	MO_Timer(1, 0, lCloseOffDelayTime * 1000);//線段點膠設定---(3)停留時間
	Sleep(1);//防止出錯，避免計時器初直為0
	while (MO_Timer(3, 0, 0))
	{
		if (m_bIsStop == 1)
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
void CAction::DecideVirtualPoint(LONG lX, LONG lY, LONG lZ, DOUBLE dAng, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy, BOOL bIsNeedleP)
{
    /*虛擬點(x座標，y座標，z座標，w角度)
    LONG lX, LONG lY, LONG lZ, DOUBLE dAng
    */
    /*系統參數(驅動速度，加速度，初速度)
    LONG lWorkVelociy,LONG lAcceleration, LONG lInitVelociy
    */
#ifdef MOVE
    //判斷W軸是否校正
    if(m_MachineOffSet.x == -99999 && m_MachineOffSet.y == -99999)
    {
        return;
    }
    //判斷使用針頭座標還是機械座標
    if(USEMODE_W == 1 && bIsNeedleP == 0) //使用機械座標
    {
        CPoint cpbuf = W_GetNeedlePoint(lX, lY, dAng, 1);
        lX = cpbuf.x;
        lY = cpbuf.y;
    }
    /*******以下程式都會轉變使用針頭座標執行運動************************/
    //流程: 輸入針頭座標>運算針頭座標>轉換成機械座標輸出
    CPoint cpNeedlePoint = W_GetNeedlePoint();//將當下位置轉換成針頭座標
    W_UpdateNeedleMotor_Needle(cpNeedlePoint.x, cpNeedlePoint.y, MO_ReadLogicPosition(2), MO_ReadLogicPositionW(), lX, lY, lZ, dAng);//切點取值(使用預設每1度取一個點)
    PauseDoGlue();//暫停恢復後繼續出膠(m_bIsPause=0) 出膠
    if(!m_bIsStop)
    {
        W_Line4DtoDo(lWorkVelociy, lAcceleration, lInitVelociy);//四軸連續插補
        PreventMoveError();//防止軸卡出錯
    }
#endif
}
/*
*等待點動作
*輸入(等待點、系統參數)
*/
void CAction::DecideWaitPoint(LONG lX, LONG lY, LONG lZ, DOUBLE dAng, LONG lWaitTime, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
    /*等待點(x座標，y座標，z座標，w角度，等待時間，)
    LONG lX, LONG lY, LONG lZ , DOUBLE dAng,LONG lWaitTime
    */
    /*系統參數(驅動速度，加速度，初速度)
    LONG lWorkVelociy,LONG lAcceleration, LONG lInitVelociy
    */
#ifdef MOVE
    //判斷W軸是否校正
    if(m_MachineOffSet.x == -99999 && m_MachineOffSet.y == -99999)
    {
        return;
    }
    //判斷使用針頭座標還是機械座標
    if(USEMODE_W == 1) //使用機械座標
    {
        CPoint cpbuf = W_GetNeedlePoint(lX, lY, dAng, 1);
        lX = cpbuf.x;
        lY = cpbuf.y;
    }
    /*******以下程式都會轉變使用針頭座標執行運動************************/
    //流程: 輸入針頭座標>運算針頭座標>轉換成機械座標輸出
    DecideVirtualPoint(lX, lY, lZ, dAng, lWorkVelociy, lAcceleration, lInitVelociy, 1);//虛擬點移動
    MO_Timer(0, 0, lWaitTime * 1000);
    MO_Timer(1, 0, lWaitTime * 1000);//等待時間(us→ms)
    Sleep(1);//防止出錯，避免計時器初直為0
    while(MO_Timer(3, 0, 0))
    {
        if(m_bIsStop == 1)
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
void CAction::DecideParkPoint(LONG lX, LONG lY, LONG lZ, DOUBLE dAng, LONG lTimeGlue, LONG lWaitTime, LONG lStayTime, LONG lZBackDistance, BOOL bZDisType, LONG lZdistance, LONG lHighVelocity, LONG lLowVelocity, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
    /*停駐點(x座標，y座標，z座標，w角度，排膠時間，結束後等待時間，)
    LONG lX, LONG lY, LONG lZ, dAng, LONG lTimeGlue,LONG lWaitTime
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
    //判斷W軸是否校正
    if(m_MachineOffSet.x == -99999 && m_MachineOffSet.y == -99999)
    {
        return;
    }
    //判斷使用針頭座標還是機械座標
    if(USEMODE_W == 1) //使用機械座標
    {
        CPoint cpbuf = W_GetNeedlePoint(lX, lY, dAng, 1);
        lX = cpbuf.x;
        lY = cpbuf.y;
    }
    /*******以下程式都會轉變使用針頭座標執行運動************************/
    //流程: 輸入針頭座標>運算針頭座標>轉換成機械座標輸出
    //W_Rotation(dAng, lWorkVelociy, lAcceleration, lInitVelociy);//w軸旋轉
    if(!m_bIsStop)
    {
        MO_Do4DLineMove(0, 0, 0, dAng - MO_ReadLogicPositionW(), (LONG)round(lWorkVelociy / m_WSpeed), (LONG)(lAcceleration / m_WSpeed), lInitVelociy);//移動W
        PreventMoveError();//防止軸卡出錯
    }
    CPoint cpMpbuf(0, 0);
    cpMpbuf = W_GetMachinePoint(lX, lY, dAng, 1);
    lX = cpMpbuf.x;//針頭座標轉換成機械座標
    lY = cpMpbuf.y;//針頭座標轉換成機械座標

    if(lHighVelocity == 0)
    {
        lHighVelocity = lWorkVelociy;
    }
    if(lLowVelocity == 0)
    {
        lLowVelocity = lWorkVelociy;
    }
    if(bZDisType) //當使用相對位置的時候
    {
        LONG lbuf = m_TablelZ - lZBackDistance;//從平台最低往上相對距離
        lZBackDistance = lbuf;
        bZDisType = 0;//改為絕對位置
    }
    if(!bZDisType)  //絕對位置
    {
        if(lZBackDistance > lZ)
        {
            lZBackDistance = lZ;
        }
        lZBackDistance = abs(lZBackDistance - lZ);
    }
    if(!m_bIsStop)
    {
        MO_Do3DLineMove(lX - MO_ReadLogicPosition(0), lY - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration, lInitVelociy);//x,y軸移動
        PreventMoveError();//防止軸卡出錯
    }
    if(!m_bIsStop)
    {
        MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//z軸移動
        PreventMoveError();//防止軸卡出錯
    }
    MO_Timer(0, 0, lWaitTime * 1000);
    MO_Timer(1, 0, 0);//(ms)
    Sleep(1);//防止出錯，避免計時器初直為0
    while(MO_Timer(3, 0, 0))
    {
        if(m_bIsStop == 1)
        {
            MO_Timer(2, 0, 0);//停止計時器
            break;
        }
        Sleep(1);
    }
    if(lTimeGlue == 0)
    {
        Sleep(1);
    }
    else
    {
        DoGlue(lTimeGlue, lStayTime);//執行排膠
    }
    Sleep(10);//防止程式出錯
    //排膠完後的回升動作
    if(!MO_ReadGumming())
    {
        if(lZdistance == 0)
        {
            if(!m_bIsStop)
            {
                MO_Do3DLineMove(0, 0, (lZ - lZBackDistance) - lZ, lWorkVelociy, lAcceleration,
                                lInitVelociy);//點膠結束設置
                PreventMoveError();//防止軸卡出錯
            }
        }
        else
        {
            if(lZdistance>lZBackDistance && lZBackDistance != 0)
            {
                lZdistance = lZBackDistance;
                MO_Do3DLineMove(0, 0, (lZ - lZBackDistance) - lZ, lHighVelocity, lAcceleration,
                                lInitVelociy);//點膠結束設置
                PreventMoveError();//防止軸卡出錯
            }
            else
            {
                if(!m_bIsStop)
                {
                    MO_Do3DLineMove(0, 0, (lZ - lZdistance) - lZ, lLowVelocity, lAcceleration,
                                    lInitVelociy);//點膠結束設置
                    PreventMoveError();//防止軸卡出錯
                }
                if(!m_bIsStop)
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

/*==原點賦歸動作
*@輸入(LONG速度1，LONG速度2，LONG指定軸(0~15)，LONG偏移量)
*@使用 m_HomeSpeed_INIT 的速度進行Z,W
*@建議速度:原點復歸速度1:30000 速度2:2000 , m_HomeSpeed_INIT =2000
*/
void CAction::DecideInitializationMachine(LONG lSpeed1, LONG lSpeed2, LONG lAxis, LONG lMoveX, LONG lMoveY, LONG lMoveZ, DOUBLE dMoveW)
{
    /*系統參數(原點復歸速度1，原點復歸速度2，指定復歸軸，偏移量)
    LONG lSpeed1,LONG lSpeed2, LONG lAxis, LONG lMove
    */
#ifdef MOVE
    //移動模式:復歸依序:Z >> W >> X、Y
    MO_FinishGumming();
    MO_SetSoftLim(lAxis, 0);    //TODO::軟體極限關
    MO_SetHardLim(lAxis, 1);
    if(lAxis > 3 && lAxis != 8 && lAxis != 9 && lAxis != 10 && lAxis != 11)
    {
        MO_MoveToHome(m_HomeSpeed_DEF, lSpeed2, 4, 0, 0, lMoveZ);//*******************復歸速度依照 m_HomeSpeed_INIT 執行驅動速度
        PreventMoveError();//防止軸卡出錯
        MO_SetSoftLim(4, 1);
        lAxis = lAxis - 4;
    }
    if(lAxis >= 10 || lAxis == 8 || lAxis == 9)
    {
        DecideGoHomeW(m_HomeSpeed_DEF, 500);//*******************復歸速度依照 m_HomeSpeed_INIT 執行驅動速度,二階段速度建議值為500
        lAxis = lAxis - 8;
    }

    if(!m_bIsStop)
    {
        MO_MoveToHome(lSpeed1, lSpeed2, lAxis, lMoveX,lMoveY,0);
        PreventMoveError();//防止軸卡出錯
    }
    MO_SetSoftLim(lAxis, 1);
#endif
}
/*==W軸原點賦歸動作
*w軸限制角度0~360度
*/
void CAction::DecideGoHomeW(LONG lSpeed1, LONG lSpeed2)
{
    /*系統參數(原點復歸速度1，原點復歸速度2)
    LONG lSpeed1,LONG lSpeed2
    */
#ifdef MOVE
    MO_FinishGumming();
    MO_SetSoftLim(8, 0);    //TODO::軟體極限要記得改
    MO_SetHardLim(8, 0);//硬體極限關
    BOOL HMbuf[5] = { 0 };//極限訊號
    //BOOL Errbuf[4] = { 0 };//極限開關故障
    //MO_ReadRunHardLim(HMbuf[0], Errbuf[0], HMbuf[1], Errbuf[1], HMbuf[2], Errbuf[2], HMbuf[3], Errbuf[3]);
    MO_ReadSoftLimError(0, HMbuf[0], HMbuf[1], HMbuf[2], HMbuf[3]);//+
    MO_ReadSoftLimError(1, HMbuf[0], HMbuf[1], HMbuf[2], HMbuf[4]);//-

    //原點復歸時旋轉的方向
    if(MO_ReadLogicPositionW() < 0.0)
    {
        //下列動作為逆時針轉((負逆
        //AfxMessageBox(L"現在角度小於0");
        if (MO_ReadLogicPositionW() <= -340.0 || HMbuf[4] == 1)
        {
            //AfxMessageBox(L"角度小於-340");
            MO_AlarmCClean();//解除
        }
        if (!m_bIsStop)
        {
            MO_Do4DLineMove(0, 0, 0, abs(MO_ReadLogicPositionW()) + 10, lSpeed1, LONG(lSpeed1*m_WSpeed), lSpeed2);
            PreventMoveError();
        }
    }
    else
    {
        //下列動作為順時針轉((正順
       // AfxMessageBox(L"現在角度大於等於0");
        if (MO_ReadLogicPositionW() >= 340.0 || HMbuf[3] == 1)
        {
            //AfxMessageBox(L"角度大於340");
            MO_AlarmCClean();//解除
            MO_Do4DLineMove(0, 0, 0, -90, lSpeed1, LONG(lSpeed1*m_WSpeed), lSpeed2);
            PreventMoveError();
        }
    }
    MO_SetHardLim(8, 1);//硬體極限有效
    if(!m_bIsStop)
    {
		BOOL type = 0;//w軸旋轉方向--------------逆時鐘方向回歸(使用-極限訊號  +極限接地)
        MO_MoveToHomeW(lSpeed1, lSpeed2, 8, 0, 0, 0, 0, type);
        PreventMoveError();//防止軸卡出錯
    }
    MO_SetHardLim(8, 0);//W軸硬體極限關閉
    MO_SetSoftLim(8, 1);//軟體極限開
#endif
}
/*
*bZDisType  TRUE:相對座標/FALSE:絕對座標
*/
void CAction::DecideVirtualHome(LONG lX, LONG lY, LONG lZ, LONG lZBackDistance, BOOL bZDisType, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
#ifdef MOVE
    //判斷W軸是否校正
    if(m_MachineOffSet.x == -99999 && m_MachineOffSet.y == -99999)
    {
        return;
    }
    //判斷使用針頭座標還是機械座標
    if(USEMODE_W == 1) //使用機械座標
    {
        CPoint cpbuf = W_GetNeedlePoint(lX, lY, 0, 1);
        lX = cpbuf.x;
        lY = cpbuf.y;
    }
    /*******以下程式都會轉變使用針頭座標執行運動************************/
    //流程: 輸入針頭座標>運算針頭座標>轉換成機械座標輸出
    CPoint cpMpbuf(0, 0);
    DecideGoHomeW(lWorkVelociy, 2000);//w軸復歸
    cpMpbuf = W_GetMachinePoint(lX, lY, 0, 1);
    lX = cpMpbuf.x;//針頭座標轉換成機械座標
    lY = cpMpbuf.y;//針頭座標轉換成機械座標
    if(lZBackDistance <= 0)
    {
        return;
    }
    if(bZDisType) //當使用相對位置的時候
    {
        LONG lbuf = m_TablelZ - lZBackDistance;//從平台最低往上相對距離
        lZBackDistance = lbuf;
        bZDisType = 0;//改為絕對位置
    }
    //絕對座標
    if(!m_bIsStop)
    {
        if(lZBackDistance < MO_ReadLogicPosition(2))
        {
            //z go back
            MO_Do3DLineMove(0, 0, lZBackDistance - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);
            PreventMoveError();//防止軸卡出錯
        }
    }
    if(!m_bIsStop)
    {
        DecideGoHomeW(lWorkVelociy, lInitVelociy);
    }
    if(!m_bIsStop)
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
#ifdef MOVE
    //判斷W軸是否校正
    if(m_MachineOffSet.x == -99999 && m_MachineOffSet.y == -99999)
    {
        return;
    }
    //判斷使用針頭座標還是機械座標
    if(USEMODE_W == 1) //使用機械座標
    {
        CPoint cpbuf = W_GetNeedlePoint(lX, lY, 0, 1);
        lX = cpbuf.x;
        lY = cpbuf.y;
    }
    /*******以下程式都會轉變使用針頭座標執行運動************************/
    //流程: 輸入針頭座標>運算針頭座標>轉換成機械座標輸出
    CPoint cpMpbuf(0, 0);
    DecideGoHomeW(lWorkVelociy, 2000);//w軸復歸
    cpMpbuf = W_GetMachinePoint(lX, lY, 0, 1);
    lX = cpMpbuf.x;//針頭座標轉換成機械座標
    lY = cpMpbuf.y;//針頭座標轉換成機械座標
    if(bZDisType) //當使用相對位置的時候
    {
        LONG lbuf = m_TablelZ - lZBackDistance;//從平台最低往上相對距離
        lZBackDistance = lbuf;
        bZDisType = 0;//改為絕對位置
    }
    if(!bZDisType)  //絕對位置
    {
        if(lZBackDistance > lZ)
        {
            lZBackDistance = lZ;
        }
        lZBackDistance = abs(lZBackDistance - lZ);
    }
    if(lZBackDistance > lZ)
    {
        lZBackDistance = lZ;
    }
    //移動X,Y,Z至針頭清潔裝置位置
    if(!m_bIsStop)
    {
        MO_Do3DLineMove(lX - MO_ReadLogicPosition(0), lY - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration, lInitVelociy);//x,y軸移動
        PreventMoveError();//防止軸卡出錯
    }
    if(!m_bIsStop)
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
    if(!m_bIsStop)
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
                         LONG lZBackDistance, BOOL bZDisType, int iType, LONG lWidth, LONG lWidth2,
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
    //判斷W軸是否校正
    if(m_MachineOffSet.x == -99999 && m_MachineOffSet.y == -99999)
    {
        return;
    }
    //判斷使用針頭座標還是機械座標
    if(USEMODE_W == 1) //使用機械座標
    {
        CPoint cpbuf = W_GetNeedlePoint(lX1, lY1, MO_ReadLogicPositionW(), 1);
        lX1 = cpbuf.x;
        lY1 = cpbuf.y;
        cpbuf = W_GetNeedlePoint(lX2, lY2, MO_ReadLogicPositionW(), 1);
        lX2 = cpbuf.x;
        lY2 = cpbuf.y;
    }
    /*******以下程式都會轉變使用針頭座標執行運動************************/
    //流程: 輸入針頭座標>運算針頭座標>轉換成機械座標輸出
    CPoint cpMpbuf(0, 0);
    cpMpbuf = W_GetMachinePoint(lX1, lY1, MO_ReadLogicPositionW(), 1);
    lX1 = cpMpbuf.x;
    lY1 = cpMpbuf.y;
    cpMpbuf = W_GetMachinePoint(lX2, lY2, MO_ReadLogicPositionW(), 1);
    lX2 = cpMpbuf.x;
    lY2 = cpMpbuf.y;
    if(lStartDelayTime > 0 && lStartDistance > 0)
    {
        lStartDistance = 0;
    }
    if(lCloseDistance > 0 && lCloseONDelayTime > 0)
    {
        lCloseONDelayTime = 0;
    }
    if(bZDisType) //當使用相對位置的時候
    {
        LONG lbuf = m_TablelZ - lZBackDistance;//從平台最低往上相對距離
        lZBackDistance = lbuf;
        bZDisType = 0;//改為絕對位置
    }
    if(!bZDisType)  //絕對位置
    {
        if(lZBackDistance > lZ1)
        {
            lZBackDistance = lZ1;
        }
        lZBackDistance = abs(lZBackDistance - lZ1);
    }
    if(lZBackDistance > lZ1)
    {
        lZBackDistance = lZ1;
    }
    if(lZ1 == lZ2)
    {
        switch(iType)
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
    if(MO_ReadPIOInput(iPort) == bChoose)
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
*輸入一個(X,Y,Z,W)座標，會回傳扣掉現在位置的相對座標
*return CStirng(X,Y,Z,W)
*/
CString CAction::NowOffSet(LONG lX, LONG lY, LONG lZ,DOUBLE dAng)
{
#ifdef MOVE
    //判斷使用針頭座標還是機械座標
    if(USEMODE_W == 1) //使用機械座標
    {
        CPoint cpbuf = W_GetNeedlePoint(lX, lY, dAng, 1);
        lX = cpbuf.x;
        lY = cpbuf.y;
    }
    /*******以下程式都會轉變使用針頭座標執行運動************************/
    //流程: 輸入針頭座標>運算針頭座標>轉換成機械座標輸出
    CPoint cpMpbuf(0, 0);
    cpMpbuf = W_GetMachinePoint(lX, lY, dAng, 1);
    lX = cpMpbuf.x;
    lY = cpMpbuf.y;

    LONG lNowX = 0, lNowY = 0, lNowZ = 0;
    CString csBuff = 0;
    lNowX = MO_ReadLogicPosition(0);
    lNowY = MO_ReadLogicPosition(1);
    lNowZ = MO_ReadLogicPosition(2);
    csBuff.Format(_T("%ld,%ld,%ld,%.3f"), (lNowX - lX), (lNowY - lY), (lNowZ - lZ),(MO_ReadLogicPositionW()-dAng));
    return csBuff;
#endif
#ifndef MOVE
    return NULL;
#endif
}
/*
*回傳目前手臂位置
*return CString(X,Y,Z,w)
*/
CString CAction::NowLocation()
{
    CString Location;
#ifdef MOVE
    if(USEMODE_W == 1) //使用機械座標
    {
        Location.Format(_T("%d,%d,%d,%.3f"), MO_ReadLogicPosition(0), MO_ReadLogicPosition(1), MO_ReadLogicPosition(2), MO_ReadLogicPositionW());
    }
    else
    {
        Location.Format(_T("%d,%d,%d,%.3f"), W_GetNeedlePoint().x, W_GetNeedlePoint().y, MO_ReadLogicPosition(2), MO_ReadLogicPositionW());
    }
#endif
    return Location;
}

/*執行完畢回(0,0,0,0)位置*/
void CAction::BackGOZero(LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
#ifdef MOVE
    //判斷W軸是否校正
    if(m_MachineOffSet.x == -99999 && m_MachineOffSet.y == -99999)
    {
        return;
    }
    AxeSpace axe = m_HomingPoint;
    if(USEMODE_W == 1) //使用機械座標
    {
        axe = { 0 };
    }
    if(!m_bIsStop)
    {
        MO_Do3DLineMove(0, 0, axe.z - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);
        PreventMoveError();
    }
    if(!m_bIsStop)
    {
        DecideGoHomeW(lWorkVelociy, lInitVelociy);//w軸復歸
    }
    if(!m_bIsStop)
    {
        MO_Do3DLineMove(axe.x - MO_ReadLogicPosition(0), axe.y - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration, lInitVelociy);
        PreventMoveError();
    }
#endif
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
    //判斷W軸是否校正
    if(m_MachineOffSet.x == -99999 && m_MachineOffSet.y == -99999)
    {
        return;
    }
    //判斷使用針頭座標還是機械座標
    if(USEMODE_W == 1) //使用機械座標
    {
        CPoint cpbuf = W_GetNeedlePoint(lX, lY, MO_ReadLogicPositionW(), 1);
        lX = cpbuf.x;
        lY = cpbuf.y;
    }
    /*******以下程式都會轉變使用針頭座標執行運動************************/
    //流程: 輸入針頭座標>運算針頭座標>轉換成機械座標輸出
    CPoint cpMpbuf(0, 0);
    cpMpbuf = W_GetMachinePoint(lX, lY, MO_ReadLogicPositionW(), 1);
    lX = cpMpbuf.x;
    lY = cpMpbuf.y;
    if(!m_bIsStop)
    {
        MO_Do3DLineMove(lX - MO_ReadLogicPosition(0), lY - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration, lInitVelociy);//x,y軸移動
        PreventMoveError();//防止軸卡出錯
    }
    if(!m_bIsStop)
    {
        MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//z軸移動
        PreventMoveError();//防止軸卡出錯
    }
#endif
}

//----------------------------------------------------
//人機使用API--使用前請使用原點復歸偏移量
//----------------------------------------------------
//軟體負極限(x,y,z預設為-10)
void CAction::HMNegLim(LONG lX, LONG lY, LONG lZ, DOUBLE dW)
{
#ifdef MOVE
    MO_SetSoftLim(15, 1);
    MO_SetCompSoft(1, -lX, -lY, -lZ,-dW);
#endif
}
//軟體正極限(x,y,z)
void CAction::HMPosLim(LONG lX, LONG lY, LONG lZ,DOUBLE dW)
{
#ifdef MOVE
    MO_SetSoftLim(15, 1);
    MO_SetCompSoft(0, lX, lY, lZ, dW);
#endif
}
/*人機用函數-移動命令(Z軸抬生→W軸旋轉→X,Y移動→Z軸下降)
*@輸入參數:x,y,z,w位置、驅動速度、加速度、初速度
*/
void CAction::HMGoPosition(LONG lX, LONG lY, LONG lZ, DOUBLE dW, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
    /*位置(x座標，y座標，z座標)
    LONG lX, LONG lY, LONG lZ
    */
    /*W軸角度
    DOUBLE dAng
    */
    /*系統參數(驅動速度，加速度，初速度)
    LONG lWorkVelociy,LONG lAcceleration, LONG lInitVelociy
    */
#ifdef MOVE
    //判斷W軸是否校正
    if(m_MachineOffSet.x == -99999 && m_MachineOffSet.y == -99999)
    {
        return;
    }
    //判斷使用針頭座標還是機械座標
    if(USEMODE_W == 1) //使用機械座標
    {
        CPoint cpbuf = W_GetNeedlePoint(lX, lY, dW, 1);
        lX = cpbuf.x;
        lY = cpbuf.y;
    }
    /*******以下程式都會轉變使用針頭座標執行運動************************/
    //流程: 輸入針頭座標>運算針頭座標>轉換成機械座標輸出
    if(!m_bIsStop)
    {
        MO_Do3DLineMove(0, 0, 0 - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//z軸抬升至最高點
        PreventMoveError();//防止軸卡出錯
    }
    if(!m_bIsStop)
    {
        MO_Do4DLineMove(0, 0, 0, dW - MO_ReadLogicPositionW(), (LONG)round(lWorkVelociy / m_WSpeed), (LONG)(lAcceleration / m_WSpeed), lInitVelociy);//w軸旋轉
        PreventMoveError();//防止軸卡出錯
    }
    CPoint cpRobot = W_GetMachinePoint(lX, lY, dW, 1);//命令點的機械座標
    if(!m_bIsStop)
    {
        MO_Do3DLineMove(cpRobot.x - MO_ReadLogicPosition(0), cpRobot.y - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration, lInitVelociy);//x,y移動到命令點
        PreventMoveError();//防止軸卡出錯
    }
    if(!m_bIsStop)
    {
        MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//z軸移動
        PreventMoveError();//防止軸卡出錯
    }
#endif
}
/***********************************************************
**                                                        **
**          運動模組-雷射3D任意路徑 (對應動作判斷)           **
**                                                        **
************************************************************/
/*==================雷射按鈕=====================================
//功能說明:針頭移動至B點，Z軸需降到最低(要塗膠的高度)，按下按鈕。
//會將B點資訊儲存至DATA_3ZERO_B中。
*/
void CAction::LA_Butt_GoBPoint()
{
#ifdef LA
    LAS_GoBPoint(DATA_3ZERO_B);
#endif
}
/*==功能說明:雷射移動至B點，按下按鈕。
//會取得X,Y的偏移量儲存至m_OffSetLaserX,m_OffSetLaserY變數中*/
void CAction::LA_Butt_GoLAtoBPoint()
{
#ifdef LA
    CPoint cpbuf(0, 0);
    W_UpdateNeedleMotorOffset(cpbuf, MO_ReadLogicPositionW());//現在角度作為基準
    DATA_3ZERO_LA = LAS_GoLAtoBPoint(DATA_3ZERO_B);
    m_OffSetLaserX = DATA_3ZERO_LA.EndPX - cpbuf.x;
    m_OffSetLaserY = DATA_3ZERO_LA.EndPY - cpbuf.y;//與圓心做offset
    //TODO::雷射OFFSET修改在這
    //m_OffSetLaserX = 48436;
    //m_OffSetLaserY = 0;

#endif
}

/*==雷射初始化*/
void CAction::LA_SetInit()
{
#ifdef LA
    #pragma region Omron Laser

    if(m_hComm != NULL)
    {
        AfxMessageBox(L"Already Opened!");
        return;
    }

    m_hComm = CreateFile(ComportNo, GENERIC_READ | GENERIC_WRITE, 0, NULL,
                         OPEN_EXISTING, false ? FILE_FLAG_OVERLAPPED : 0, NULL);
    if(m_hComm == INVALID_HANDLE_VALUE)
    {
        // MessageBox(L"Open failed");
        return;
    }
    //LAS_SetInit(&m_hComm);
    LAS_SetInit();
    #pragma endregion //Omron
    ////LAS_SetInit();
    ////TODO::雷射OFFSET修改在這
    ////*************目前寫死動作***************************************
    //m_OffSetLaserX = 40324;
    //m_OffSetLaserY = -647;
    //m_HeightLaserZero = 31079;//雷射用Z軸歸零點高度
    //m_OffSetLaserZ = 43274;//B點的Z高度到雷射歸零Z高度的位移值(+)
#endif
}

/*
*單點掃描
*/
BOOL CAction::LA_Dot3D(LONG lX, LONG lY, LONG &lZ, LONG lWorkVelociy,LONG lAcceleration, LONG lInitVelociy)
{
#ifdef LA
    //判斷W軸是否校正
    if(m_MachineOffSet.x == -99999 && m_MachineOffSet.y == -99999)
    {
        return FALSE;
    }
    //判斷使用針頭座標還是機械座標
    if(USEMODE_W == 1) //使用機械座標
    {
        CPoint cpbuf = W_GetNeedlePoint(lX, lY, MO_ReadLogicPositionW(), 1);
        lX = cpbuf.x;
        lY = cpbuf.y;
    }
    /*******以下程式都會轉變使用針頭座標執行運動************************/
    //流程: 輸入針頭座標>運算針頭座標>轉換成機械座標輸出
    CPoint cpMpbuf(0, 0);
    cpMpbuf = W_GetMachinePoint(lX, lY, MO_ReadLogicPositionW(), 1);
    lX = cpMpbuf.x;
    lY = cpMpbuf.y;
    LONG lCalcData1;
    if(!m_bIsStop)
    {
        MO_Do3DLineMove(0, 0, m_HeightLaserZero - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);
        PreventMoveError();//防止軸卡驅動錯誤
    }
    if(!m_bIsStop)
    {
        MO_Do3DLineMove(lX - m_OffSetLaserX - MO_ReadLogicPosition(0), lY - m_OffSetLaserY - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration, lInitVelociy);
        PreventMoveError();//防止軸卡驅動錯誤
    }
    if(LAS_GetLaserData(lCalcData1))
    {
        if(lCalcData1 == LAS_LONGMIN)
        {
            lZ = MO_ReadLogicPosition(2);
            return FALSE;//取不到值(ffffff)
        }
        else
        {
            lZ = MO_ReadLogicPosition(2) - lCalcData1 + m_OffSetLaserZ;//30000為感測範圍
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
    if(LAS_SetZero()) //執行歸零點
    {
        if(!m_bIsStop)
        {
            MO_Do3DLineMove(0, 0, -LAS_MEASURE_RANGE, 20000, 100000, 1000);//上升3cm再次做雷射歸零
            PreventMoveError();//防止軸卡出錯
        }
        Sleep(1000);
        if(LAS_SetZero())
        {
            m_HeightLaserZero = MO_ReadLogicPosition(2);//雷射用Z軸歸零點高度
            m_OffSetLaserZ = DATA_3ZERO_B.EndPZ - m_HeightLaserZero;//B點的Z高度到雷射歸零Z高度的位移值(+)
            return TRUE;//雷射歸零完成
        }
        else
        {
            AfxMessageBox(L"歸零高度不正確_自動修正&請再次嘗試");
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
    //判斷W軸是否校正
    if(m_MachineOffSet.x == -99999 && m_MachineOffSet.y == -99999)
    {
        return;
    }
    CPoint cpbuf[2] = { 0 };
    //判斷使用針頭座標還是機械座標
    if(USEMODE_W == 1) //使用機械座標
    {
        cpbuf[0] = W_GetNeedlePoint(lX1, lY1, MO_ReadLogicPositionW(), 1);
        cpbuf[1] = W_GetNeedlePoint(lX2, lY2, MO_ReadLogicPositionW(), 1);
        CPoint cpBuff = W_GetNeedlePoint(lX3, lY3, MO_ReadLogicPositionW(), 1);
        lX3 = cpBuff.x;
        lY3 = cpBuff.y;
    }
    /*******以下程式都會轉變使用針頭座標執行運動************************/
    //流程: 輸入針頭座標>運算針頭座標>轉換成機械座標輸出
    BOOL bType = 0;
    DATA_2MOVE DATA_2D;
    CPoint cpMpbuf(0, 0);
    if(lX1 == 0 && lY1 == 0 && lX2 == 0 && lY2 == 0)
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
        LONG machine[2] = { 0 };
        csP1.Format(L"%d，%d，", cpbuf[0].x, cpbuf[0].y);
        csP2.Format(L"%d，%d，", cpbuf[1].x, cpbuf[1].y);
        csP3.Format(L"%d，%d，", lX3, lY3);
        csbuff = TRoundCCalculation(csP1, csP3, csP2);
        machine[0] = CStringToLong(csbuff, 0);
        machine[1] = CStringToLong(csbuff, 1);
        cpMpbuf = W_GetMachinePoint(machine[0], machine[1], MO_ReadLogicPositionW(), 1);
        DATA_2D.CirCentP.x = cpMpbuf.x;
        DATA_2D.CirCentP.y = cpMpbuf.y;
        DATA_2D.CirRev = CStringToLong(csbuff, 2);//取得圓心(X，Y，Rev，)
    }
    cpMpbuf = W_GetMachinePoint(lX3, lY3, MO_ReadLogicPositionW(), 1);
    DATA_2D.EndP.x = cpMpbuf.x;
    DATA_2D.EndP.y = cpMpbuf.y;
    DATA_2D.Type = bType;
    DATA_2D.Speed = 0;
    LA_m_ptVec2D.push_back(DATA_2D);
#endif
}
/*兩軸線段點*/
void CAction::LA_Do2dDataLine(LONG EndPX, LONG EndPY, BOOL bIsNeedleP)
{
#ifdef MOVE
    //判斷W軸是否校正
    if(m_MachineOffSet.x == -99999 && m_MachineOffSet.y == -99999)
    {
        return;
    }
    //判斷使用針頭座標還是機械座標
    if(USEMODE_W == 1 && bIsNeedleP == 0) //使用機械座標
    {
        CPoint cpbuf = W_GetNeedlePoint(EndPX, EndPY, MO_ReadLogicPositionW(), 1);
        EndPX = cpbuf.x;
        EndPY = cpbuf.y;
    }
    /*******以下程式都會轉變使用針頭座標執行運動************************/
    //流程: 輸入針頭座標>運算針頭座標>轉換成機械座標輸出
    DATA_2MOVE DATA_2D;
    CPoint cpMpbuf(0, 0);
    cpMpbuf = W_GetMachinePoint(EndPX, EndPY, MO_ReadLogicPositionW(), 1);
    DATA_2D.EndP.x = cpMpbuf.x;
    DATA_2D.EndP.y = cpMpbuf.y;
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
    //判斷W軸是否校正
    if(m_MachineOffSet.x == -99999 && m_MachineOffSet.y == -99999)
    {
        return;
    }
    if(LA_m_ptVec2D.empty())
    {
        return;
    }
    if(USEMODE_W == 1) //使用機械座標
    {
        CPoint cpbuf = W_GetNeedlePoint(EndPX, EndPY, MO_ReadLogicPositionW(), 1);
        EndPX = cpbuf.x;
        EndPY = cpbuf.y;
        cpbuf = W_GetNeedlePoint(ArcX, ArcY, MO_ReadLogicPositionW(), 1);
        ArcX = cpbuf.x;
        ArcY = cpbuf.y;
    }
    /*******以下程式都會轉變使用針頭座標執行運動************************/
    //流程: 輸入針頭座標>運算針頭座標>轉換成機械座標輸出
    DATA_2MOVE DATA_2D;
    CString csbuff;
    CString csP1, csP2, csP3;
    LONG machine[2] = { 0 };
    CPoint cpMpbuf(0, 0);
    csP1.Format(L"%d，%d，", LA_m_ptVec2D.back().EndP.x, LA_m_ptVec2D.back().EndP.y);
    csP2.Format(L"%d，%d，", ArcX, ArcY);
    csP3.Format(L"%d，%d，", EndPX, EndPY);
    csbuff = TRoundCCalculation(csP1, csP3, csP2);
    DATA_2D.Type = 1;//圓
    cpMpbuf = W_GetMachinePoint(EndPX, EndPY, MO_ReadLogicPositionW(), 1);
    DATA_2D.EndP.x = cpMpbuf.x;
    DATA_2D.EndP.y = cpMpbuf.y;
    machine[0] = CStringToLong(csbuff, 0);
    machine[1] = CStringToLong(csbuff, 1);
    cpMpbuf = W_GetMachinePoint(machine[0], machine[1], MO_ReadLogicPositionW(), 1);
    DATA_2D.CirCentP.x = cpMpbuf.x;
    DATA_2D.CirCentP.y = cpMpbuf.y;
    DATA_2D.CirRev = CStringToLong(csbuff, 2);//取得圓心(X，Y，Rev，)
    LA_m_ptVec2D.push_back(DATA_2D);
#endif
}
/*兩軸線段圓*/
void CAction::LA_Do2dDataCircle(LONG EndPX, LONG EndPY, LONG CirP1X, LONG CirP1Y, LONG CirP2X, LONG CirP2Y)
{
#ifdef MOVE
    //判斷W軸是否校正
    if(m_MachineOffSet.x == -99999 && m_MachineOffSet.y == -99999)
    {
        return;
    }
    if(LA_m_ptVec2D.empty())
    {
        return;
    }
    if(USEMODE_W == 1) //使用機械座標
    {
        CPoint cpbuf = W_GetNeedlePoint(EndPX, EndPY, MO_ReadLogicPositionW(), 1);
        EndPX = cpbuf.x;
        EndPY = cpbuf.y;
        cpbuf = W_GetNeedlePoint(CirP1X, CirP1Y, MO_ReadLogicPositionW(), 1);
        CirP1X = cpbuf.x;
        CirP1Y = cpbuf.y;
        cpbuf = W_GetNeedlePoint(CirP2X, CirP2Y, MO_ReadLogicPositionW(), 1);
        CirP2X = cpbuf.x;
        CirP2Y = cpbuf.y;
    }
    /*******以下程式都會轉變使用針頭座標執行運動************************/
    //流程: 輸入針頭座標>運算針頭座標>轉換成機械座標輸出
    DATA_2MOVE DATA_2D;
    CString csbuff;
    CString csP1, csP2, csP3;
    LONG machine[2] = { 0 };
    CPoint cpMpbuf(0, 0);
    csP1.Format(L"%d，%d，", EndPX, EndPY);
    csP2.Format(L"%d，%d，", CirP1X, CirP1Y);
    csP3.Format(L"%d，%d，", CirP2X, CirP2Y);
    csbuff = TRoundCCalculation(csP1, csP3, csP2);
    DATA_2D.Type = 1;//圓
    cpMpbuf = W_GetMachinePoint(EndPX, EndPY, MO_ReadLogicPositionW(), 1);
    DATA_2D.EndP.x = cpMpbuf.x;
    DATA_2D.EndP.y = cpMpbuf.y;
    machine[0] = CStringToLong(csbuff, 0);
    machine[1] = CStringToLong(csbuff, 1);
    cpMpbuf = W_GetMachinePoint(machine[0], machine[1], MO_ReadLogicPositionW(), 1);
    DATA_2D.CirCentP.x = cpMpbuf.x;
    DATA_2D.CirCentP.y = cpMpbuf.y;
    DATA_2D.CirRev = CStringToLong(csbuff, 2);//取得圓心(X，Y，Rev，)
    LA_m_ptVec2D.push_back(DATA_2D);
#endif
}
/***********************************************************
**                                                        **
**          運動模組-雷射3D任意路徑 (對應動作判斷)           **
**                                                        **
************************************************************/
#ifdef MOVE
/*絕對座標轉相對座標3軸連續插補使用*/
void CAction::LA_AbsToOppo3Move(std::vector<DATA_3MOVE> &str)
{
    std::vector<DATA_3MOVE> vecBuf;
    DATA_3MOVE mData;
    LONG lNowX = MO_ReadLogicPosition(0), lNowY = MO_ReadLogicPosition(1), lNowZ = MO_ReadLogicPosition(2);
    vecBuf.clear();
    mData.EndPX = str.at(0).EndPX - lNowX;
    mData.EndPY = str.at(0).EndPY - lNowY;
    mData.EndPZ = str.at(0).EndPZ - lNowZ;
    mData.Distance = LONG(sqrt(pow(str.at(0).EndPX, 2) + pow(str.at(0).EndPY, 2)));
    vecBuf.push_back(mData);
    for(UINT i = 1; i < str.size(); i++)
    {
        if((str.at(i).EndPX == LA_SCANEND) && (str.at(i).EndPY == LA_SCANEND) && (str.at(i).EndPZ == LA_SCANEND))
        {
            mData.EndPX = str.at(i).EndPX;
            mData.EndPY = str.at(i).EndPY;
            mData.EndPZ = str.at(i).EndPZ;
            //vecBuf.push_back(mData);
            //如果-99999不是最後一筆資料
            if(i + 1 < str.size())
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
    for(UINT i = 0; i < str.size() - 1; i++)
    {
        DATA_3Do[i] = { 0 };
        DATA_3Do[i] = str[i + 1];
    }
}
#endif
#ifdef MOVE
/*絕對座標轉相對座標2軸連續插補使用*/
void CAction::LA_AbsToOppo2Move(std::vector<DATA_2MOVE> &str)
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
    for(UINT i = 1; i < str.size(); i++)
    {
        mData.Speed = str.at(i).Speed;
        mData.Type = str.at(i).Type;
        mData.EndP = str.at(i).EndP - str.at(i - 1).EndP;
        mData.Distance = (LONG)round(sqrt(pow(mData.EndP.x, 2) + pow(mData.EndP.y, 2)));
        if(str.at(i).Type)
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
//旋轉
//RefX對位點/OffSetX偏移量(對位的偏移量)/Andgle旋轉的角度/CameraToTipOffsetX (ccd對針頭的偏移量)
void CAction::LA_CorrectLocation(LONG &PointX, LONG &PointY, LONG RefX, LONG RefY, DOUBLE OffSetX, DOUBLE OffSetY, DOUBLE Andgle, DOUBLE CameraToTipOffsetX, DOUBLE CameraToTipOffsetY, BOOL Mode, LONG lSubOffsetX, LONG lSubOffsetY)
{
    DOUBLE X, Y;
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
    if(Mode)
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
/*
*連續線段取值
*lStartVe, lStartAcc, lStartInitVe  移動到掃描啟始點的驅動速度、加速度、初速度
*lWorkVelociy, lAcceleration, lInitVelociy  掃描的驅動速度、加速度、初速度
*/
void CAction::LA_Line2D(LONG lStartVe, LONG lStartAcc, LONG lStartInitVe, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
#ifdef LA
#ifdef MOVE
    //判斷W軸是否校正
    if(m_MachineOffSet.x == -99999 && m_MachineOffSet.y == -99999)
    {
        return;
    }
    if(m_LaserAverage == FALSE)
    {
        m_LaserCnt++;
    }
    LA_AbsToOppo2Move(LA_m_ptVec2D);
    if(!m_bIsStop)
    {
        MO_Do3DLineMove(0, 0, m_HeightLaserZero - MO_ReadLogicPosition(2), lStartVe, lStartAcc, lStartInitVe);
        PreventMoveError();//起始點準備移動
    }
    if(!m_bIsStop)
    {
        MO_Do3DLineMove(LA_m_ptVec2D.at(0).EndP.x - m_OffSetLaserX,
                        LA_m_ptVec2D.at(0).EndP.y - m_OffSetLaserY, 0, lStartVe, lStartAcc, lStartInitVe);
        PreventMoveError();//起始點準備移動
    }

    /*停止觸發時，結束掃描*/
    if(m_bIsStop)
    {
        return;
    }

    MO_InterruptCase(1, 2);
    MO_InterruptCase(1, 3);
    MO_Timer(0, 100000);//設定計時器(0.1s觸發一次)
    /*插入開始點一筆*/
    LONG lCalcData1;

    if(LAS_GetLaserData(lCalcData1))
    {
        if(lCalcData1 == LAS_LONGMIN)
        {
            g_LaserErrCnt++;
        }
        else
        {
            DATA_3Do[0].EndPX = MO_ReadLogicPosition(0) + m_OffSetLaserX;
            DATA_3Do[0].EndPY = MO_ReadLogicPosition(1) + m_OffSetLaserY;
            DATA_3Do[0].EndPZ = MO_ReadLogicPosition(2) - lCalcData1 + m_OffSetLaserZ;//30000為感測範圍
            if(m_LaserAverage == FALSE)
            {
                LA_m_ptVec.push_back(DATA_3Do[0]);
            }
            if(m_LaserCnt == 1 && m_LaserAverage == FALSE)
            {
                LA_m_iVecSP.push_back(m_LaserCnt);//main
            }
        }
    }
    for(UINT i = 1; i < LA_m_ptVec2D.size(); i++)
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
    //判斷W軸是否校正
    if(m_MachineOffSet.x == -99999 && m_MachineOffSet.y == -99999)
    {
        return;
    }
    /*停止觸發時，結束掃描*/
    if(m_bIsStop)
    {
        return;
    }

    if(iData <= 0)
    {
        return;
    }

    while(m_bIsGetLAend == FALSE);
    std::vector<DATA_3MOVE>::iterator LA_ptIter;//迭代器
    std::vector<DATA_3MOVE>LA_Buff;//線段值判斷
    LONG lNowZHigh = 0;
    LA_Buff.clear();
    if(bDoAll)
    {
        LA_Buff.assign(LA_m_ptVec.begin(), LA_m_ptVec.begin() + LA_m_iVecSP.at(1) - 1);
        lNowZHigh = m_HeightLaserZero - MO_ReadLogicPosition(2);//相對位置
    }
    else
    {
        if((UINT)iData > LA_m_iVecSP.size())
        {
            return;
        }
        else if(iData == 1)
        {
            LA_Buff.assign(LA_m_ptVec.begin(), LA_m_ptVec.begin() + LA_m_iVecSP.at(1) - 1);
            lNowZHigh = m_HeightLaserZero - MO_ReadLogicPosition(2);//相對位置
        }
        else
        {
            LA_Buff.assign(LA_m_ptVec.begin() + LA_m_iVecSP.at(iData - 1), LA_m_ptVec.begin() + LA_m_iVecSP.at(iData) - 1);
            lNowZHigh = 0;//相對位置
        }
    }
    if(!m_bIsStop)
    {
        MO_Do3DLineMove(0, 0, lNowZHigh, lWorkVelociy, lAcceleration, lInitVelociy);//掃描高度回去
        PreventMoveError();//起始點準備移動
    }
    LA_AbsToOppo3Move(LA_Buff);
    if(!m_bIsStop)
    {
        LA_ptIter = LA_Buff.begin();
        MO_Do3DLineMove(LA_ptIter->EndPX, LA_ptIter->EndPY, 0, lWorkVelociy, lAcceleration, lInitVelociy);//移動起始點
        PreventMoveError();//防止驅動錯誤
    }
    if(!m_bIsStop)
    {
        LA_ptIter = LA_Buff.begin();
        MO_Do3DLineMove(0, 0, LA_ptIter->EndPZ, lWorkVelociy, lAcceleration, lInitVelociy);//z軸下降到起始點高度
        PreventMoveError();//防止驅動錯誤
    }
    if(!m_bIsStop)
    {
        if(!m_bIsStop && m_bIsDispend == 1)
        {
            MO_GummingSet();//塗膠(不卡)
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
    m_LaserCnt = 0; //雷射線段計數器(掃描用)
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
    //判斷W軸是否校正
    if(m_MachineOffSet.x == -99999 && m_MachineOffSet.y == -99999)
    {
        return;
    }
    //判斷使用針頭座標還是機械座標
    if(USEMODE_W == 1) //使用機械座標
    {
        CPoint cpbuf = W_GetNeedlePoint(lStrX, lStrY, MO_ReadLogicPositionW(), 1);
        lStrX = cpbuf.x;
        lStrY = cpbuf.y;
        cpbuf = W_GetNeedlePoint(lEndX, lEndY, MO_ReadLogicPositionW(), 1);
        lEndX = cpbuf.x;
        lEndY = cpbuf.y;
    }
    /*******以下程式都會轉變使用針頭座標執行運動************************/
    //流程: 輸入針頭座標>運算針頭座標>轉換成機械座標輸出
    m_LaserAverage = TRUE;
    m_LaserAveBuffZ = 0;
    LA_Do2dDataLine(lStrX, lStrY,1);
    LA_Do2dDataLine(lEndX, lEndY,1);
    LA_Line2D(lStartVe, lStartAcc, lStartInitVe, lWorkVelociy, lAcceleration, lInitVelociy);
    Sleep(200);
    lZ = m_LaserAveBuffZ;
    m_LaserAverage = FALSE;
#endif
#endif
}
//修正加執行連續線段
void CAction::LA_CorrectVectorToDo(LONG  lWorkVelociy, LONG lAcceleration, LONG lInitVelociy, LONG RefX, LONG RefY, DOUBLE OffSetX, DOUBLE OffSetY, DOUBLE Andgle, DOUBLE CameraToTipOffsetX, DOUBLE CameraToTipOffsetY, BOOL Mode, LONG lSubOffsetX, LONG lSubOffsetY)
{
#ifdef MOVE
    //判斷W軸是否校正
    if(m_MachineOffSet.x == -99999 && m_MachineOffSet.y == -99999)
    {
        return;
    }
    std::vector<DATA_3MOVE>::iterator LA_ptIter;//迭代器
    std::vector<DATA_3MOVE>LA_Buff;//線段值判斷
    LONG lNowZHigh = 0;
    m_LaserCnt = 1;
    //while (m_bIsGetLAend == FALSE);
    LA_Buff.clear();

    LA_Buff.assign(LA_m_ptVec.begin(), LA_m_ptVec.begin() + LA_m_iVecSP.at(1) - 1);
    lNowZHigh = m_HeightLaserZero - MO_ReadLogicPosition(2);//相對位置
    for(UINT i = 0; i < LA_Buff.size(); i++)
    {
        if(!(LA_Buff.at(i).EndPX == LA_SCANEND && LA_m_ptVec.at(i).EndPY == LA_SCANEND))
        {
            LA_CorrectLocation(LA_Buff.at(i).EndPX, LA_Buff.at(i).EndPY, RefX, RefY, OffSetX, OffSetY, Andgle, CameraToTipOffsetX, CameraToTipOffsetY, Mode, lSubOffsetX, lSubOffsetY);
        }
    }
    //將值在塞回LA_m_ptVec
    if(!m_bIsStop)
    {
        MO_Do3DLineMove(0, 0, lNowZHigh, lWorkVelociy, lAcceleration, lInitVelociy);//掃描高度回去
        PreventMoveError();//起始點準備移動
    }

    LA_AbsToOppo3Move(LA_Buff);

    if(!m_bIsStop)
    {
        LA_ptIter = LA_Buff.begin();
        MO_Do3DLineMove(LA_ptIter->EndPX, LA_ptIter->EndPY, 0, lWorkVelociy, lAcceleration, lInitVelociy);//移動起始點
        PreventMoveError();//防止驅動錯誤
    }
    if(!m_bIsStop)
    {
        LA_ptIter = LA_Buff.begin();
        MO_Do3DLineMove(0, 0, LA_ptIter->EndPZ, lWorkVelociy, lAcceleration, lInitVelociy);//z軸下降到起始點高度
        PreventMoveError();//防止驅動錯誤
    }
    if(!m_bIsStop)
    {
        if(!m_bIsStop && m_bIsDispend == 1)
        {
            MO_GummingSet();//塗膠(不卡)
        }
        MO_DO3Curve(DATA_3Do, LA_Buff.size() - 1, lWorkVelociy);//連續插補開始
        PreventMoveError();//防止驅動錯誤
    }
#endif
}
//填充選擇拿取最後一點座標(EndX,EndY)
void CAction::Fill_EndPoint(LONG &lEndX, LONG &lEndY, LONG lX1, LONG lY1, LONG lZ1, LONG lX2, LONG lY2, LONG lZ2, int iType, LONG lWidth, LONG lWidth2)
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
    //判斷W軸是否校正
#ifdef MOVE
    if(m_MachineOffSet.x == -99999 && m_MachineOffSet.y == -99999)
    {
        return;
    }
    //判斷使用針頭座標還是機械座標
    if(USEMODE_W == 1) //使用機械座標
    {
        CPoint cpbuf = W_GetNeedlePoint(lX1, lY1, MO_ReadLogicPositionW(), 1);
        lX1 = cpbuf.x;
        lY1 = cpbuf.y;
        cpbuf = W_GetNeedlePoint(lX2, lY2, MO_ReadLogicPositionW(), 1);
        lX2 = cpbuf.x;
        lY2 = cpbuf.y;
    }
    /*******以下程式都會轉變使用針頭座標執行運動************************/
    //流程: 輸入針頭座標>運算針頭座標>轉換成機械座標輸出-----地址位置依照輸入座標回傳對應的座標
    CPoint cpMpbuf(0, 0);
    cpMpbuf = W_GetMachinePoint(lX1, lY1, MO_ReadLogicPositionW(), 1);
    lX1 = cpMpbuf.x;
    lY1 = cpMpbuf.y;
    cpMpbuf = W_GetMachinePoint(lX2, lY2, MO_ReadLogicPositionW(), 1);
    lX2 = cpMpbuf.x;
    lY2 = cpMpbuf.y;
    LONG lBufX = 0, lBufY = 0;
    if(lZ1 == lZ2)
    {
        switch(iType)
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
    //依照所選用的狀態(針頭座標或是機械座標回傳數值!!!)
    if(USEMODE_W == 0) //使用針頭座標
    {
        CPoint cpbuf = W_GetNeedlePoint(lEndX, lEndY, MO_ReadLogicPositionW(), 1);
        lEndX = cpbuf.x;
        lEndY = cpbuf.y;
    }
#endif // MOVE
}

/***********************************************************
**                                                        **
**          運動模組-執行緒                                **
**                                                        **
************************************************************/
//3d任意路徑執行緒
DWORD CAction::MoInterrupt(LPVOID param)
{
#ifdef MOVE
#ifdef PRINTF
  //  _cwprintf(L"%s\n", L"Interrupt");
#endif
    LONG RR1X, RR1Y, RR1Z, RR1U;
    MO_ReadEvent(&RR1X, &RR1Y, &RR1Z, &RR1U);
    if(RR1X & 0x0010) //原本為RR1X&0x0020 驅動開始中斷
    {
        g_LaserErrCnt = 0;//計數初始化
        g_LaserNuCnt = 1;//計數初始化
        //MO_Timer(0, 100000);//設定計時器(0.1s觸發一次)
        MO_Timer(1, 100000);//啟動計時器
        ((CAction *)param)->m_getHeightFlag = TRUE;//雷射測高旗標：允許測高
        ((CAction *)param)->m_bIsGetLAend = FALSE;//掃描尚未完成
    }
    if((RR1X & 0x0080) || (g_LaserErrCnt == 10)) //原本的為((RR1X&0x0040)|| (g_LaserErrCnt == 10)) 驅動結束中斷
    {
#ifdef LA
#ifdef PRINTF
     //   _cwprintf(L"%s\n", L"驅動結束中斷!");
#endif
        ((CAction *)param)->m_getHeightFlag = FALSE;
        //((CAction *)param)->m_bIsGetLAend = TRUE;
        if((g_LaserErrCnt >= 10))
        {
            AfxMessageBox(L"雷射點請修正");
            return 0;
        }
        /*插入結束點一筆*/
        LONG lCalcData1;
        if(LAS_GetLaserData(lCalcData1))
        {
            if(lCalcData1 == LAS_LONGMIN)
            {
                g_LaserErrCnt++;
            }
            else//雷射取到的值為(0~+30)
            {
                ((CAction *)param)->DATA_3Do[g_LaserNuCnt].EndPX = MO_ReadLogicPosition(0) + ((CAction *)param)->m_OffSetLaserX;
                ((CAction *)param)->DATA_3Do[g_LaserNuCnt].EndPY = MO_ReadLogicPosition(1) + ((CAction *)param)->m_OffSetLaserY;
                ((CAction *)param)->DATA_3Do[g_LaserNuCnt].EndPZ = MO_ReadLogicPosition(2) - lCalcData1 + ((CAction *)param)->m_OffSetLaserZ;//30000為感測範圍
                if(((CAction *)param)->m_LaserAverage == TRUE)
                {
                    ((CAction *)param)->m_LaserAveBuffZ += ((CAction *)param)->DATA_3Do[g_LaserNuCnt].EndPZ;
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

        if(((CAction *)param)->m_LaserAverage == TRUE)
        {
            ((CAction *)param)->m_LaserAveBuffZ = LONG(round(((DOUBLE)((CAction *)param)->m_LaserAveBuffZ) / (g_LaserNuCnt - 1)));
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
        ((CAction *)param)->m_bIsGetLAend = TRUE;// 掃描完成
    }
    if(RR1Y & 0x0200)  //Y計時器中斷(出膠與斷膠)
    {
#ifdef PRINTF
    //    _cwprintf(L"y\n");
#endif
        if(!((CAction *)param)->m_bIsStop)
        {
            (CAction::m_YtimeOutGlueSet) ? MO_GummingSet() : MO_FinishGumming();
            CAction::m_YtimeOutGlueSet = FALSE;
        }
    }
    if(RR1Z & 0x0200)  //Z計時器中斷
    {
#ifdef PRINTF
  //      _cwprintf(L"z\n");
#endif
        if(!((CAction *)param)->m_bIsStop)
        {
            (CAction::m_ZtimeOutGlueSet) ? MO_GummingSet() : MO_FinishGumming();
            CAction::m_ZtimeOutGlueSet = FALSE;
        }
    }
    if(RR1U & 0x0200 && ((CAction *)param)->m_getHeightFlag)  //U計時器中斷
    {
#ifdef LA
        LONG lCalcData1;
        if(LAS_GetLaserData(lCalcData1))
        {
            if(lCalcData1 == LAS_LONGMIN)
            {
                g_LaserErrCnt++;
            }
            else
            {
                ((CAction *)param)->DATA_3Do[g_LaserNuCnt].EndPX = MO_ReadLogicPosition(0) + ((CAction *)param)->m_OffSetLaserX - ((CAction *)param)->m_OffSetScan;
                ((CAction *)param)->DATA_3Do[g_LaserNuCnt].EndPY = MO_ReadLogicPosition(1) + ((CAction *)param)->m_OffSetLaserY - ((CAction *)param)->m_OffSetScan;
                ((CAction *)param)->DATA_3Do[g_LaserNuCnt].EndPZ = MO_ReadLogicPosition(2) - lCalcData1 + ((CAction *)param)->m_OffSetLaserZ;//30000為感測範圍
                if(((CAction *)param)->m_LaserAverage == TRUE)
                {
                    ((CAction *)param)->m_LaserAveBuffZ += ((CAction *)param)->DATA_3Do[g_LaserNuCnt].EndPZ;
                }
                else
                {
                    ((CAction *)param)->LA_m_ptVec.push_back(((CAction *)param)->DATA_3Do[g_LaserNuCnt]);
#ifdef PRINTF
     //               _cwprintf(L"%s\n", L"Get Point");
#endif
                }
                g_LaserNuCnt++;
            }
        }

#endif
    }
#endif
    return 0;
}
//動作執行緒
UINT CAction::MoMoveThread(LPVOID param)
{
#ifdef MOVE
    //_cwprintf(L"%s\n", L"JogMove : W_Rotation");
    //m_ThreadFlag = 1 ------ W_Rotation
    //WangBuff         ------ dW
    //cpCirMidBuff[0].x------ lWorkVelociy;
    //cpCirMidBuff[0].y------ lAcceleration;
    //cpCirMidBuff[1].x------ lInitVelociy;
    if(((CAction *)param)->m_ThreadFlag == 1) //w軸自轉
    {
    //_cwprintf(L"%s\n", L"W_Rotation");
        ((CAction *)param)->W_Rotation(((CAction *)param)->WangBuff, ((CAction *)param)->cpCirMidBuff[0].x, ((CAction *)param)->cpCirMidBuff[0].y, ((CAction *)param)->cpCirMidBuff[1].x);
    }
    else if(((CAction *)param)->m_ThreadFlag == 2)
    {
    //_cwprintf(L"%s\n", L"GoHoming");
        //m_ThreadFlag = 2;------W_NeedleGoHoming
        //cpCirMidBuff[0].x------ Speed1;
        //cpCirMidBuff[0].y------ Speed2;
        //cpCirMidBuff[1].x------ bStep;
		((CAction *)param)->m_IsHomingOK = 1;//原點復歸執行中
        if(!((CAction *)param)->cpCirMidBuff[1].x)
        {
            ((CAction *)param)->DecideInitializationMachine(((CAction *)param)->cpCirMidBuff[0].x, ((CAction *)param)->cpCirMidBuff[0].x, 15,0, 0, 0, 0);
			if (!((CAction *)param)->m_bIsStop)
			{
				MO_Do3DLineMove(((CAction *)param)->m_HomingOffset_INIT.x, ((CAction *)param)->m_HomingOffset_INIT.y, ((CAction *)param)->m_HomingOffset_INIT.z,
					((CAction *)param)->cpCirMidBuff[0].x, (((CAction *)param)->cpCirMidBuff[0].x) * 3, ((CAction *)param)->cpCirMidBuff[0].y);
				((CAction *)param)->PreventMoveError();
			}
        }
        else
        {
            //判斷W軸是否校正
            if(((CAction *)param)->m_MachineOffSet.x == -99999 && ((CAction *)param)->m_MachineOffSet.y == -99999)
            {
                //((CAction *)param)->DecideInitializationMachine(((CAction *)param)->cpCirMidBuff[0].x, ((CAction *)param)->cpCirMidBuff[0].x, 15,
                //  ((CAction *)param)->m_HomingOffset_INIT.x, ((CAction *)param)->m_HomingOffset_INIT.y, ((CAction *)param)->m_HomingOffset_INIT.z, ((CAction *)param)->m_HomingOffset_INIT.w);
                return 0;
            }
            ((CAction *)param)->DecideInitializationMachine(((CAction *)param)->cpCirMidBuff[0].x, ((CAction *)param)->cpCirMidBuff[0].y, 15,
                    (((CAction *)param)->m_HomingPoint.x ), (((CAction *)param)->m_HomingPoint.y), ((CAction *)param)->m_HomingPoint.z, ((CAction *)param)->m_HomingPoint.w);

			if(!((CAction *)param)->m_bIsStop)
            {
               MO_Do3DLineMove(-(((CAction *)param)->m_MachineOffSet.x), -(((CAction *)param)->m_MachineOffSet.y), 0,
                                ((CAction *)param)->cpCirMidBuff[0].x, (((CAction *)param)->cpCirMidBuff[0].x) * 3, ((CAction *)param)->cpCirMidBuff[0].y);
                ((CAction *)param)->PreventMoveError();
            }
        }
    }
    else
    {
        Sleep(1);
    }
    //清除buff狀態
	((CAction *)param)->m_IsHomingOK = 0;//原點復歸初始化
    ((CAction *)param)->m_ThreadFlag = 0;//復歸旗標狀態
    ((CAction *)param)->cpCirMidBuff[0].x = 0;
    ((CAction *)param)->cpCirMidBuff[0].y = 0;
    ((CAction *)param)->cpCirMidBuff[1].x = 0;
    ((CAction *)param)->cpCirMidBuff[1].y = 0;
    ((CAction *)param)->WangBuff = 0;
#endif
    return 0;
}

/***********************************************************
**                                                        **
**          運動模組-自行運用函數                           **
**                                                        **
************************************************************/

/*
*出膠(配合執行緒使用)
*輸入(點膠機開啟or點膠機關閉)解譯後的LONG值
*/
void CAction::DoGlue(LONG lTime, LONG lDelayTime, LPTHREAD_START_ROUTINE MoInterrupt)
{
#ifdef MOVE
    if(!m_bIsStop)
    {
        if(!m_bIsStop && m_bIsDispend == 1)
        {
            MO_GummingSet(lTime * 1000);
        }
        PreventGlueError();//防止出膠出錯
    }
    if(!m_bIsStop)
    {
        // lTime / 1000 =(us→ms)
        MO_Timer(0, 0, lDelayTime * 1000);
        MO_Timer(1, 0, 0);//(ms)
        Sleep(1);//防止出錯，避免計時器初直為0
        while(MO_Timer(3, 0, 0))
        {
            if(m_bIsStop == 1)
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
    while(MO_ReadIsDriving(15) && !m_bIsStop)
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
    while(MO_ReadGumming() && !m_bIsStop)
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
    if(!m_bIsPause)   //暫停恢復後繼續出膠(m_bIsPause=0)
    {
        if((m_bIsPause == 0) && (m_bIsDispend == 1))
        {
            MO_GummingSet();//塗膠(不卡)
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
    if((m_bIsPause == 1) || (m_bIsDispend == 0))  //暫停時停指塗膠(m_bIsPause=1)
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
    if(lLineStop>lR)
    {
        lBackXar = lXarUp;
        lBackYar = lYarUp;
    }
    else if(lXarEnd == 0 && lYarEnd == 0 && lXarUp == 0 && lYarUp == 0)
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
    if(lStopZar>lZarEnd)
    {
        lStopZar = lZarEnd;
    }
    if(lStopZar>lBackZar)
    {
        lStopZar = lBackZar;
    }
    if(lBackZar>lZarEnd)
    {
        lBackZar = lZarEnd;
    }
    switch(iType)  //0~5
    {
        case 0://無動作
        {
            break;
        }
        case 1://z軸上升兩段速
        {
            if(lStopZar == 0)
            {
                if(!m_bIsStop)
                {
                    MO_Do3DLineMove(0, 0, (lZarEnd - lBackZar) - lZarEnd, lHighSpeed, lAcceleration, lInitSpeed);//沒有返回長度以高速返回最高點
                    PreventMoveError();//防止軸卡出錯
                }
            }
            else
            {
                if(!m_bIsStop)
                {
                    MO_Do3DLineMove(0, 0, (lZarEnd - lStopZar) - lZarEnd, lLowSpeed, lAcceleration, lInitSpeed);//低速
                    PreventMoveError();//防止軸卡出錯
                }
                if(!m_bIsStop)
                {
                    MO_Do3DLineMove(0, 0, (lZarEnd - lBackZar) - (lZarEnd - lStopZar), lHighSpeed, lAcceleration, lInitSpeed);//高速
                    PreventMoveError();//防止軸卡出錯
                }
            }
            break;
        }
        case 2: //向後三軸插補(低速)，再z軸上升(高速)
        {
            if(lStopZar == 0)
            {
                if(!m_bIsStop)
                {
                    MO_Do3DLineMove(lBackXar - lNowX, lBackYar - lNowY, 0, lLowSpeed, lAcceleration, lInitSpeed);//低速
                    PreventMoveError();//防止軸卡出錯

                }
                if(!m_bIsStop)
                {
                    MO_Do3DLineMove(0, 0, (lZarEnd - lBackZar) - lZarEnd, lHighSpeed, lAcceleration, lInitSpeed);//高速
                    PreventMoveError();//防止軸卡出錯
                }
            }
            else
            {
                if(!m_bIsStop)
                {
                    MO_Do3DLineMove(lBackXar - lNowX, lBackYar - lNowY, (lZarEnd - lStopZar) - lZarEnd, lLowSpeed, lAcceleration, lInitSpeed);//低速
                    PreventMoveError();//防止軸卡出錯
                }
                if(!m_bIsStop)
                {
                    MO_Do3DLineMove(0, 0, (lZarEnd - lBackZar) - (lZarEnd - lStopZar), lHighSpeed, lAcceleration, lInitSpeed);//高速
                    PreventMoveError();//防止軸卡出錯
                }
            }

            break;
        }
        case 3://直角向後，z軸先上升(低速)，xy兩軸插補向後移動(高速)，再z軸上升(高速)
        {
            if(lStopZar != 0)
            {
                if(!m_bIsStop)
                {
                    MO_Do3DLineMove(0, 0, (lZarEnd - lStopZar) - lZarEnd, lLowSpeed, lAcceleration, lInitSpeed);//低速z軸上升
                    PreventMoveError();//防止軸卡出錯
                }
            }
            if(!m_bIsStop)
            {
                MO_Do3DLineMove(lBackXar - lNowX, lBackYar - lNowY, 0, lHighSpeed, lAcceleration, lInitSpeed);//向後移動高速
                PreventMoveError();//防止軸卡出錯
            }
            if(!m_bIsStop)
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
            if(lStopZar == 0)
            {
                if(!m_bIsStop)
                {
                    MO_Do3DLineMove(lBackXar - lNowX, lBackYar - lNowY, 0, lLowSpeed, lAcceleration, lInitSpeed);//低速
                    PreventMoveError();//防止軸卡出錯
                }
                if(!m_bIsStop)
                {
                    MO_Do3DLineMove(0, 0, (lZarEnd - lBackZar) - lZarEnd, lLowSpeed, lAcceleration, lInitSpeed);//高速
                    PreventMoveError();//防止軸卡出錯
                }
            }
            else
            {
                if(!m_bIsStop)
                {
                    MO_Do3DLineMove(lBackXar - lNowX, lBackYar - lNowY, (lZarEnd - lStopZar) - lZarEnd, lLowSpeed, lAcceleration, lInitSpeed);//低速
                    PreventMoveError();//防止軸卡出錯
                }
                if(!m_bIsStop)
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

            if(lStopZar != 0)
            {
                if(!m_bIsStop)
                {
                    MO_Do3DLineMove(0, 0, (lZarEnd - lStopZar) - lZarEnd, lLowSpeed, lAcceleration, lInitSpeed);//z軸低速上升
                    PreventMoveError();//防止軸卡出錯
                }
            }
            if(!m_bIsStop)
            {
                MO_Do3DLineMove(lBackXar - lNowX, lBackYar - lNowY, 0, lHighSpeed, lAcceleration, lInitSpeed);//兩軸向前移動(高速)
                PreventMoveError();//防止軸卡出錯
            }
            if(!m_bIsStop)
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
}
/*
*直線距離轉換成座標點--多載
*/
void CAction::LineGetToPoint(LONG &lXClose, LONG &lYClose, LONG &lZClose, LONG lX0, LONG lY0, LONG lX1, LONG lY1, LONG lZ0, LONG lZ1, LONG &lLineClose)
{
    LONG lLength = 0;
    lLength = LONG(sqrt(pow(lX0 - lX1, 2) + pow(lY0 - lY1, 2) + pow(lZ0 - lZ1, 2)));
    if(lZ0 == lZ1)
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
    CString csS;
    if(lAcceleration == 0)
    {
        t = (DOUBLE)lStartDistance / (DOUBLE)lWorkVelociy;
        return (LONG)round(t * 1000000);
    }
    t = (DOUBLE)(lWorkVelociy - lInitVelociy) / (DOUBLE)lAcceleration;
    S = (DOUBLE)lInitVelociy*t + (DOUBLE)lAcceleration*pow(t, 2) / 2.0;
    if(lStartDistance >= (LONG)round(S))
    {
        lDistance = lStartDistance - (LONG)round(S);
        dStartTime = (DOUBLE)lDistance / (DOUBLE)lWorkVelociy;
        dStartTime += t;
        return (LONG)round(dStartTime * 1000000);
    }
    else
    {
        csS.Format(L"距離不適當,最大距離%.3f um\n",S);
        AfxMessageBox(csS);
        return (LONG)round(t * 1000000);
    }
}

/*==直線段使用，設置距離在起始點出膠*/
//計算到等速域時,所需要的"距離"
LONG CAction::CalPreglue(LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
    DOUBLE S = 0, t = 0;
    if(lAcceleration == 0)
    {
        return 0;
    }
    t = (DOUBLE)(lWorkVelociy - lInitVelociy) / (DOUBLE)lAcceleration;
    S = (DOUBLE)lInitVelociy*t + (DOUBLE)lAcceleration*pow(t, 2) / 2.0;
    return (LONG)round(S);
}

/*==直線段使用，設置距離在起始點出膠*/
//計算到等速域時,所需要的"時間"
LONG CAction::CalPreglueTime(LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
    DOUBLE S = 0, t = 0;
    if(lAcceleration == 0)
    {
        return 0;
    }
    t = (DOUBLE)(lWorkVelociy - lInitVelociy) / (DOUBLE)lAcceleration;
    S = (DOUBLE)lInitVelociy*t + (DOUBLE)lAcceleration*pow(t, 2) / 2.0;
    return (LONG)round(t * 1000000);
}
/*
*三點計算圓心
*return CString(x軸圓心，y軸圓心，0逆轉/1順轉，)
*/
CString CAction::TRoundCCalculation(CString Origin, CString End, CString Between)
{
    if(Origin == End || End == Between || Origin == Between)
    {
        AfxMessageBox(_T("What the fucking you type in my system."));
        return _T("F");
    }
    CString Ans;
    DOUBLE x1 = DOUBLE(CStringToLong(Origin, 0));
    DOUBLE y1 = DOUBLE(CStringToLong(Origin, 1));
    DOUBLE x2 = DOUBLE(CStringToLong(End, 0));
    DOUBLE y2 = DOUBLE(CStringToLong(End, 1));
    DOUBLE x3 = DOUBLE(CStringToLong(Between, 0));
    DOUBLE y3 = DOUBLE(CStringToLong(Between, 1));

    DOUBLE tan13 = (y3 - y1) / (x3 - x1);
    if((tan13) >= 200)
    {
        tan13 = 200;
    }
    else if((tan13) <= -200)
    {
        tan13 = -200;
    }
    DOUBLE tan12 = (y2 - y1) / (x2 - x1);
    if((tan12) >= 200)
    {
        tan12 = 200;
    }
    else if((tan12) <= -200)
    {
        tan13 = -200;
    }

    DOUBLE a = 2 * (x1 - x2);
    DOUBLE b = 2 * (y1 - y2);
    DOUBLE c = y1*y1 + x1*x1 - x2*x2 - y2*y2;
    DOUBLE d = 2 * (x1 - x3);
    DOUBLE e = 2 * (y1 - y3);
    DOUBLE f = y1*y1 + x1*x1 - x3*x3 - y3*y3;
    int h = int(round((e*c - b*f) / (a*e - b*d)));
    int k = int(round((a*f - d*c) / (a*e - b*d)));
    Ans.Format(_T("%d，%d，"), h, k);
    if((x2 - x1) > 0)
    {
        if((y3 - tan12*x3) > (y1 - tan12*x1))
        {
            Ans = Ans + _T("0") + _T("，");
        }
        else if((y3 - tan12*x3) < (y1 - tan12*x1))
        {
            Ans = Ans + _T("1") + _T("，");
        }
        else
        {
            Ans = _T("F");
            AfxMessageBox(_T("三點共線"));
        }
    }
    else if((x2 - x1) < 0)
    {
        if((y3 - tan12*x3) > (y1 - tan12*x1))
        {
            Ans = Ans + _T("1") + _T("，");
        }
        else if((y3 - tan12*x3) < (y1 - tan12*x1))
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
        if((x3 > x1))
        {
            Ans = Ans + _T("1") + _T("，");
        }
        else if((x3 < x1))
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
    for(int i = 0; i<csData.GetLength(); i++)
    {
        ione[i + 1] = csData.Find(L"，", ione[i] + 1);
    }
    lOne[0] = _ttol(csData.Left(ione[0]));
    for(int j = 0; j<iChoose; j++)
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

    if(offsetx<0 && offsety >= 0)
    {
        theta_a = 180 - theta_a;
    }
    else if(offsetx<0 && offsety<0)
    {
        theta_a = 180 + theta_a;
    }
    else if(offsetx >= 0 && offsety<0)
    {
        theta_a = 360 - theta_a;
    }

    offset_x = LocatX2 - LocatX;
    offset_y = LocatY2 - LocatY;

    X_a = offset_x / pow(((pow(offset_x, 2)) + (pow(offset_y, 2))), 0.5);
    Y_a = offset_y / pow(((pow(offset_x, 2)) + (pow(offset_y, 2))), 0.5);

    theta_b = (atan(fabs((Y_a / X_a)))) * 180 / M_PI;

    if(offset_x<0 && offset_y >= 0)
    {
        theta_b = 180 - theta_b;
    }
    else if(offset_x<0 && offset_y<0)
    {
        theta_b = 180 + theta_b;
    }
    else if(offset_x >= 0 && offset_y<0)
    {
        theta_b = 360 - theta_b;
    }

    if(theta_a<theta_b)
    {
        theta_a = theta_b - theta_a;
    }
    else
    {
        theta_a = 360 - (theta_a - theta_b);
    }

    if(bRev == 1)
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
    if(bDir == 0)
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
    if(pt1 == pt2 || lWidth == 0)
    {
        return;
    }

    std::vector<DATA_2MOVE> vecData2do;
    vecData2do.clear();
    LONG width = lWidth * 1000;

    //使用(1)移動前延遲(lStartDelayTime)
    if(lStartDelayTime>0)
    {
        if(!m_bIsStop)
        {
            MO_Do3DLineMove(lX1 - MO_ReadLogicPosition(0), lY1 - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration,
                            lInitVelociy);//回到起始點!
            PreventMoveError();
        }
        if(!m_bIsStop)
        {
            MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//回到起始點!
            PreventMoveError();
        }
        PauseDoGlue();//暫停回復後重新塗膠(讀取暫停參數，當參數為0時出膠，且點膠機要為開。)
        if(!m_bIsStop && m_bIsDispend == 1)
        {
            MO_GummingSet();//塗膠(不卡)
        }
        MO_Timer(0, 0, lStartDelayTime * 1000);
        MO_Timer(1, 0, lStartDelayTime * 1000);//線段點膠設定---(1)移動前延遲(在線段開始點上)

        Sleep(1);//防止出錯，避免計時器初直為0
        while(MO_Timer(3, 0, 0))
        {
            if(m_bIsStop)
            {
                break;
            }
            Sleep(1);
        }
        /*插入第一點*/
        MCO_Do2dDataLine(pt1.x, pt1.y, vecData2do);
    }
    //使用(2)計算出膠距離，設定出膠點
    else if(lStartDistance>0)
    {
        LONG glueDist = lStartDistance;

        if(pt1.x == pt2.x) //兩點垂直
        {
            /*距離轉xy分量*/
            glueDist = LONG(glueDist*M_SQRT1_2);
            glueDist = LONG((pt1.y<pt2.y) ? -glueDist : glueDist);
            MCO_Do2dDataLine(pt1.x + glueDist, pt1.y + glueDist, vecData2do);
        }
        else if(pt1.y == pt2.y) //兩點平行
        {
            /*距離轉xy分量*/
            glueDist = LONG(glueDist*M_SQRT1_2);
            glueDist = (pt1.x<pt2.x) ? glueDist : -glueDist;
            MCO_Do2dDataLine(pt1.x + glueDist, pt1.y - glueDist, vecData2do);
        }
        else
        {
            glueDist = (pt1.x<pt2.x) ? -glueDist : glueDist;
            MCO_Do2dDataLine(pt1.x + glueDist, pt1.y, vecData2do);
        }
    }
    //(1)(2)皆不使用
    else
    {
        /*插入第一點*/
        MCO_Do2dDataLine(pt1.x, pt1.y, vecData2do);
    }

    //兩點垂直
    if(pt1.x == pt2.x)
    {
        /*填充區域(矩形)邊長*/
        LONG lHeightOfRect = (LONG)abs(round(DOUBLE(pt2.y - pt1.y)*M_SQRT1_2));
        /*間隔寬度xy位移量，填充xy位移量計算，oddEven正逆向控制*/
        LONG shiftWidth = LONG(width*M_SQRT1_2);
        LONG shift = (LONG)abs(round((pt1.y - pt2.y) / 2.0));
        BOOL oddEven = TRUE;
        if(pt2.y<pt1.y)
        {
            oddEven = FALSE;
        }
        while(lHeightOfRect >= 0)
        {
            ptNow.x = (oddEven) ? (ptNow.x + shift) : (ptNow.x - shift);
            ptNow.y = (oddEven) ? (ptNow.y + shift) : (ptNow.y - shift);
            MCO_Do2dDataLine(ptNow.x, ptNow.y, vecData2do);

            lHeightOfRect -= width;
            if(lHeightOfRect >= 0)
            {
                ptNow.x = (pt2.y>pt1.y) ? (ptNow.x - shiftWidth) : (ptNow.x + shiftWidth);
                ptNow.y = (pt2.y>pt1.y) ? (ptNow.y + shiftWidth) : (ptNow.y - shiftWidth);
                MCO_Do2dDataLine(ptNow.x, ptNow.y, vecData2do);
            }
            else
            {
                if(oddEven)
                {
                    ptNow = pt2;
                    MCO_Do2dDataLine(ptNow.x, ptNow.y, vecData2do);
                }
                else
                {
                    ptNow.x = (pt2.y>pt1.y) ? pt1.x - shift : pt1.x + shift;
                    ptNow.y = LONG((pt1.y + pt2.y) / 2.0);
                    MCO_Do2dDataLine(ptNow.x, ptNow.y, vecData2do);
                    ptNow = pt2;
                    MCO_Do2dDataLine(ptNow.x, ptNow.y, vecData2do);
                }
            }
            oddEven = !oddEven;
        }
    }
    //兩點平行
    else if(pt1.y == pt2.y)
    {
        LONG lHeightOfRect = (LONG)abs(round(DOUBLE(pt2.x - pt1.x)*M_SQRT1_2));
        LONG shiftWidth = LONG(width*M_SQRT1_2);
        LONG shift = (LONG)abs(round((pt1.x - pt2.x) / 2.0));
        BOOL oddEven = TRUE;
        if(pt2.x<pt1.x)
        {
            oddEven = FALSE;
        }
        while(lHeightOfRect >= 0)
        {
            ptNow.x = (oddEven) ? (ptNow.x + shift) : (ptNow.x - shift);
            ptNow.y = (oddEven) ? (ptNow.y - shift) : (ptNow.y + shift);
            MCO_Do2dDataLine(ptNow.x, ptNow.y, vecData2do);

            lHeightOfRect -= width;
            if(lHeightOfRect >= 0)
            {
                ptNow.x = (pt2.x>pt1.x) ? (ptNow.x + shiftWidth) : (ptNow.x - shiftWidth);
                ptNow.y = (pt2.x>pt1.x) ? (ptNow.y + shiftWidth) : (ptNow.y - shiftWidth);
                MCO_Do2dDataLine(ptNow.x, ptNow.y, vecData2do);
            }
            else
            {
                if(oddEven)
                {
                    ptNow.x = LONG((pt1.x + pt2.x) / 2.0);
                    ptNow.y = (pt2.x>pt1.x) ? pt1.x + shift : pt1.x - shift;
                    MCO_Do2dDataLine(ptNow.x, ptNow.y, vecData2do);
                    ptNow = pt2;
                    MCO_Do2dDataLine(ptNow.x, ptNow.y, vecData2do);
                }
                else
                {
                    ptNow = pt2;
                    MCO_Do2dDataLine(ptNow.x, ptNow.y, vecData2do);
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
        while(lHeightOfRect >= 0)
        {
            /*x平移下一點*/
            ptNow.x = (oddEven) ? (ptNow.x + shiftX) : (ptNow.x - shiftX);
            MCO_Do2dDataLine(ptNow.x, ptNow.y, vecData2do);
            if(ptNow == pt2)
            {
                break;
            }

            /*判斷間隔寬度是否還有剩餘，更新間隔寬度的座標*/
            lHeightOfRect -= width;
            if(lHeightOfRect >= 0)
            {
                /*y寬度平移下一點*/
                ptNow.y = (pt2.y>pt1.y) ? (ptNow.y + width) : (ptNow.y - width);
                MCO_Do2dDataLine(ptNow.x, ptNow.y, vecData2do);
            }
            else
            {
                /*間隔寬度不足的處理*/
                if(oddEven)
                {
                    /*直接到結束點*/
                    ptNow = pt2;
                    MCO_Do2dDataLine(ptNow.x, ptNow.y, vecData2do);
                }
                else
                {
                    /*補足寬度至結束點*/
                    ptNow.y = pt2.y;
                    MCO_Do2dDataLine(ptNow.x, ptNow.y, vecData2do);
                    ptNow = pt2;
                    MCO_Do2dDataLine(ptNow.x, ptNow.y, vecData2do);
                }
            }
            oddEven = !oddEven;
        }
    }

    //點陣列轉換
    LA_AbsToOppo2Move(vecData2do);
    for(UINT i = 1; i < vecData2do.size(); i++)
    {
        DATA_2Do[i - 1] = vecData2do.at(i);
    }
#ifdef MOVE
    if(!m_bIsStop)
    {
        MO_Do3DLineMove(vecData2do.at(0).EndP.x, vecData2do.at(0).EndP.y, 0, lWorkVelociy, lAcceleration,
                        lInitVelociy);//回到起始點!
        PreventMoveError();
    }
    if(!m_bIsStop)
    {
        MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//回到起始點!
        PreventMoveError();
    }
#endif
    //使用(2)設置距離
    if(!(lStartDelayTime>0) && lStartDistance>0)
    {
        //timeUpGlue 單位us
        LONG timeUpGlue = CalPreglue(lStartDistance, lWorkVelociy, lAcceleration, lInitVelociy);
        if(!m_bIsStop)
        {
            CAction::m_YtimeOutGlueSet = TRUE;
#ifdef MOVE
            MO_TimerSetIntter(timeUpGlue, 0);//使用Y timer中斷出膠
#endif
        }
    }

    //使用(5)關機距離(lCloseDistance)
    if(lCloseDistance>0)
    {
        LONG sumPath = 0;
        LONG finishTime = 0;
        DOUBLE avgTime = 0;
        LONG accLength = CalPreglue(lWorkVelociy, lAcceleration, lInitVelociy);
        for(UINT i = 1; i<vecData2do.size(); i++)
        {
            sumPath += vecData2do.at(i).Distance;
        }
        avgTime = ((DOUBLE)sumPath - (DOUBLE)accLength) / (DOUBLE)lWorkVelociy;
        finishTime = (LONG)round(avgTime * 1000000) + CalPreglueTime(lWorkVelociy, lAcceleration, lInitVelociy);
        LONG closeDistTime = CalPreglue(lCloseDistance, lWorkVelociy, 0, lInitVelociy);
        if(!m_bIsStop)
        {
            CAction::m_ZtimeOutGlueSet = FALSE;
            MO_TimerSetIntter(finishTime - closeDistTime, 1);
        }
    }

    if(lStartDelayTime == 0 && lStartDistance == 0)
    {
        if(!m_bIsStop && m_bIsDispend == 1)
        {
            MO_GummingSet();
        }
    }

    MO_DO2Curve(DATA_2Do, vecData2do.size() - 1, lWorkVelociy);
    PreventMoveError();//防止驅動錯誤
    Sleep(200);
    vecData2do.clear();



    //使用(3)停留時間(lCloseOffDelayTime)
    if(lCloseOffDelayTime>0)
    {
        MO_Timer(0, 0, lCloseOffDelayTime * 1000);
        MO_Timer(1, 0, lCloseOffDelayTime * 1000);//線段點膠設定---(3)停留時間
        Sleep(1);//防止出錯，避免計時器初直為0
        while(MO_Timer(3, 0, 0))
        {
            if(m_bIsStop == 1)
            {
                break;
            }
            Sleep(1);
        }
    }
    PauseStopGlue();//暫停時停指塗膠(m_bIsPause=1)
    MO_StopGumming();//停止出膠

    //使用(6)關機延遲(lCloseONDelayTime)
    if(!(lCloseDistance>0) && lCloseONDelayTime>0)
    {
        MO_Timer(0, 0, lCloseONDelayTime * 1000);
        MO_Timer(1, 0, lCloseONDelayTime * 1000);//線段點膠設定---(6)關機延遲
        Sleep(1);//防止出錯，避免計時器初直為0
        while(MO_Timer(3, 0, 0))
        {
            if(m_bIsStop == 1)
            {
                break;
            }
            Sleep(1);
        }
    }
    if(!m_bIsStop)
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
    if(LONG(dRadius) == 0)
    {
        return;
    }
    if(lStartDistance>0) //使用--(2)設置距離(lStartDistance)
    {
        LONG lStartX = 0, lStartY = 0;
        ArcGetToPoint(lStartX, lStartY, lStartDistance, lX1, lY1, lCenX, lCenY, LONG(dRadius), bRev);//算出起始點
        cPt0.x = lStartX;
        cPt0.y = lStartY;
        //      m_ptVec.push_back(cPt0);
        if(!m_bIsStop)
        {
            MO_Do3DLineMove(lStartX - MO_ReadLogicPosition(0), lStartY - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration,
                            lInitVelociy);//回到起始點!
            PreventMoveError();
        }
        if(!m_bIsStop)
        {
            MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//回到起始點!
            PreventMoveError();
        }
        lTime = LONG(1000000 * (DOUBLE)lStartDistance / (DOUBLE)lWorkVelociy);
        //_cwprintf(_T("計時器設置距離的時間=%lf \n"), DOUBLE(lTime / 1000000.0));
        /*======計時器到觸發中斷執行出膠，使用y中斷執行================*/
        if(!m_bIsStop)
        {
            CAction::m_YtimeOutGlueSet = TRUE;
            MO_TimerSetIntter(lTime, 0);//計時到跳至執行序
        }
    }
    else
    {

        if(!m_bIsStop)
        {
            MO_Do3DLineMove(lX1 - MO_ReadLogicPosition(0), lY1 - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration,
                            lInitVelociy);//回到起始點!
            PreventMoveError();
        }
        if(!m_bIsStop)
        {
            MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//回到起始點!
            PreventMoveError();
        }
        PauseDoGlue();//暫停回復後重新塗膠(讀取暫停參數，當參數為0時出膠，且點膠機要為開。)
        if(!m_bIsStop && m_bIsDispend == 1)
        {
            MO_GummingSet();//塗膠(不卡)
        }
        if(lStartDelayTime > 0) //使用(1)移動前延遲(lStartDelayTime)
        {
            MO_Timer(0, 0, lStartDelayTime * 1000);
            MO_Timer(1, 0, lStartDelayTime * 1000);//線段點膠設定---(1)移動前延遲(在線段開始點上)
            Sleep(1);//防止出錯，避免計時器初直為0
            while(MO_Timer(3, 0, 0))
            {
                if(m_bIsStop)
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
    while(1)
    {
        lDistance = LONG(lDistance - dWidth);
        if(lDistance < dWidth)
        {
            dSumPath -= M_PI / 2 * (sqrt(pow(cPt3.x - cPt4.x, 2) + pow(cPt3.y - cPt4.y, 2)));
            dSumPath += sqrt(pow(cPt3.x - cPt4.x, 2) + pow(cPt3.y - cPt4.y, 2));
            break;
        }
        LineGetToPoint(lXClose, lYClose, cPt1.x, cPt1.y, cPtCen1.x, cPtCen1.y,
                       lDistance);
        cPt3.x = lXClose;
        cPt3.y = lYClose;
        if(icnt == 0)
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
    if(lCloseDistance > 0) //使用 --(5)關機距離
    {
        dCloseTime = 1000000 * ((dSumPath - lCloseDistance) / (DOUBLE)lWorkVelociy);
        /*======計時器到觸發中斷執行斷膠，使用z中斷執行================*/
        if(!m_bIsStop)
        {
            CAction::m_ZtimeOutGlueSet = FALSE;
            if(lStartDistance > 0)
            {
                //_cwprintf(_T("End關機距離的時間=%lf \n"), DOUBLE(lTime) + DOUBLE(dCloseTime / 1000000.0));
                MO_TimerSetIntter(lTime + LONG(dCloseTime), 1);//計時到跳至執行序
            }
            else
            {
                //_cwprintf(_T("End關機距離的時間=%lf \n"), DOUBLE(dCloseTime / 1000000.0));
                MO_TimerSetIntter(LONG(dCloseTime), 1);//計時到跳至執行序
            }
        }
    }
    #pragma endregion

    std::vector<DATA_2MOVE> DATA_2DO;
    UINT cnt = 0;
    DATA_2DO.clear();
    icnt = 0;
    for(ptIter = m_ptVec.begin(); ptIter != m_ptVec.end(); ptIter++)
    {
        if(lStartDistance < 0 && cnt == 0)
        {
            cnt++;
        }
        else if((iData != 0) && (cnt == m_ptVec.size() - 1))
        {
            MCO_Do2dDataLine((*ptIter).x, (*ptIter).y, DATA_2DO);
        }
        else if((iData == 0) && (cnt == m_ptVec.size() - 1))
        {
            MCO_Do2dDataCir((*ptIter).x, (*ptIter).y, cPtCen2.x, cPtCen2.y, bRev, DATA_2DO);
        }
        else
        {
            if(cnt % 2 == 0)
            {
                MCO_Do2dDataCir((*ptIter).x, (*ptIter).y, cPtCen2.x, cPtCen2.y, bRev, DATA_2DO);
            }
            else
            {
                MCO_Do2dDataCir((*ptIter).x, (*ptIter).y, lCenX, lCenY, bRev, DATA_2DO);
            }
        }
        cnt++;
    }
    LA_AbsToOppo2Move(DATA_2DO);
    if(lStartDistance > 0)
    {
        for(UINT i = 0; i < DATA_2DO.size(); i++)
        {
            DATA_2Do[i] = DATA_2DO.at(i);
        }
        MO_DO2Curve(DATA_2Do, DATA_2DO.size(), lWorkVelociy);
    }
    else
    {
        for(UINT i = 1; i < DATA_2DO.size(); i++)
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
    while(MO_Timer(3, 0, 0))
    {
        if(m_bIsStop == 1)
        {
            break;
        }
        Sleep(1);
    }
    MO_StopGumming();//停止出膠
    if(!m_bIsStop)
    {
        //*************************填充回升z軸速度目前為驅動速度的兩倍******************
        MO_Do3DLineMove(0, 0, (lZ - lZBackDistance) - lZ, lWorkVelociy * 2, lAcceleration,
                        lInitVelociy);//Z軸返回
        PreventMoveError();//防止軸卡出錯
    }
    MO_Timer(0, 0, lCloseOffDelayTime * 1000);
    MO_Timer(1, 0, lCloseOffDelayTime * 1000);//線段點膠設定---(3)停留時間
    Sleep(1);//防止出錯，避免計時器初直為0
    while(MO_Timer(3, 0, 0))
    {
        if(m_bIsStop == 1)
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
    if(dRadius == 0)
    {
        return;
    }
    dAngCenCos = acos(DOUBLE(cPt1.x - cPtCen.x) / dRadius);
    dAngCenSin = asin(DOUBLE(cPt1.y - cPtCen.y) / dRadius);
    dAngCenCos2 = M_PI * 2 - dAngCenCos;
    dAngCenSin2 = M_PI - dAngCenSin;
    if(abs(dAngCenCos - dAngCenSin) > 0.01)
    {
        if(abs(dAngCenCos - dAngCenSin2) < 0.01)
        {
            dAngCenSin = dAngCenSin2;
        }
        else if(abs(dAngCenCos2 - dAngCenSin) < 0.01)
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
    if(lStartDelayTime>0)
    {
        if(!m_bIsStop)
        {
            MO_Do3DLineMove(lX1 - MO_ReadLogicPosition(0), lY1 - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration,
                            lInitVelociy);//回到起始點!
            PreventMoveError();
        }
        if(!m_bIsStop)
        {
            MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//回到起始點!
            PreventMoveError();
        }
        PauseDoGlue();//暫停回復後重新塗膠(讀取暫停參數，當參數為0時出膠，且點膠機要為開。)
        if(!m_bIsStop && m_bIsDispend == 1)
        {
            MO_GummingSet();//塗膠(不卡)
        }
        MO_Timer(0, 0, lStartDelayTime * 1000);
        MO_Timer(1, 0, lStartDelayTime * 1000);//線段點膠設定---(1)移動前延遲(在線段開始點上)

        Sleep(1);//防止出錯，避免計時器初直為0

        //計算s1與s2
        S1 = LONG(sqrt(pow(cPt2.x - cPt1.x, 2) + (cPt2.y - cPt1.y, 2)));
        S2 = LONG(sqrt(pow(cPt3.x - cPt2.x, 2) + (cPt3.y - cPt2.y, 2)));
        while(MO_Timer(3, 0, 0))
        {
            if(m_bIsStop)
            {
                break;
            }
            Sleep(1);
        }
        /*插入第一點*/
        m_ptVec.push_back(cPt1);
    }
    //使用(2)計算出膠距離，設定出膠點
    else if(lStartDistance>0)
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
    if(dAngCos < 0)
    {
        dAngCos += M_PI * 2;
    }
    if(dAngSin < 0)
    {
        dAngSin += M_PI * 2;
    }
    dAngCos2 = M_PI * 2 - dAngCos;
    dAngSin2 = M_PI - dAngSin;
    if(abs(dAngCos - dAngSin) > 0.01)
    {
        if(abs(dAngCos - dAngSin2) < 0.01)
        {
            dAngSin = dAngSin2;
        }
        else if(abs(dAngCos2 - dAngSin) < 0.01)
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
    while(1)
    {
        dRadius = dRadius - dWidth*sqrt(2);
        dDistance = dDistance - dWidth;
        if(dDistance < dWidth)
        {
            if(icnt == 1)
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
        if(dDistance < dWidth)
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
    for(ptIter = m_ptVec.begin(); ptIter != m_ptVec.end(); ptIter++)
    {
        MCO_Do2dDataLine((*ptIter).x, (*ptIter).y, DATA_2DO);
    }
    //點陣列轉換
    LA_AbsToOppo2Move(DATA_2DO);
    for(UINT i = 1; i < DATA_2DO.size(); i++)
    {
        DATA_2Do[i - 1] = DATA_2DO.at(i);
    }
    if(!m_bIsStop)
    {
        MO_Do3DLineMove(DATA_2DO.at(0).EndP.x, DATA_2DO.at(0).EndP.y, 0, lWorkVelociy, lAcceleration,
                        lInitVelociy);//回到起始點!
        PreventMoveError();
    }
    if(!m_bIsStop)
    {
        MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//回到起始點!
        PreventMoveError();
    }
    //使用(2)設置距離
    if(lStartDistance>0)
    {
        //timeUpGlue 單位us
        timeUpGlue = CalPreglue(lStartDistance, lWorkVelociy, lAcceleration, lInitVelociy);
        if(!m_bIsStop)
        {
            CAction::m_YtimeOutGlueSet = TRUE;
            MO_TimerSetIntter(timeUpGlue, 0);//使用y timer中斷 出膠
        }
    }
    //使用(5)關機距離(lCloseDistance)
    DOUBLE T1 = 0, T2 = 0;
    LONG V1 = 0, V2 = 0, AllS = 0, AllT = 0, FinallV = 0;
    if(lCloseDistance>0)
    {
        //第一區塊連續插補
        T1 = (-lInitVelociy / 1000 + (sqrt(pow(lInitVelociy / 1000, 2) + 2 * lAcceleration / 1000 * S1 / 1000))) / (DOUBLE)(lAcceleration / 1000.0);
        V1 = LONG(lInitVelociy + lAcceleration*T1);
        if(lWorkVelociy > V1)
        {
            AllT = LONG(T1 * 1000000);
            //第二區塊連續插補
            T2 = (-V1 / 1000 + (sqrt(pow(V1 / 1000, 2) + 2 * lAcceleration / 1000 * S1 / 1000))) / (DOUBLE)(lAcceleration / 1000.0);
            V2 = LONG(lAcceleration* LONG(T1) + V1);
            if(lWorkVelociy > V2)
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
        if(lStartDistance > 0)
        {
            dCloseTime = 1000000 * ((dSumPath - lCloseDistance) / (DOUBLE)lWorkVelociy);
            /*======計時器到觸發中斷執行斷膠，使用z中斷執行================*/
            if(!m_bIsStop)
            {
                CAction::m_ZtimeOutGlueSet = FALSE;
                //_cwprintf(_T("End關機距離的時間=%lf \n"), DOUBLE(timeUpGlue) + DOUBLE(dCloseTime / 1000000.0));
                MO_TimerSetIntter(timeUpGlue + LONG(dCloseTime), 1);//計時到跳至執行序
            }
        }
        else
        {
            LONG lCloseTime = 0;
            lCloseTime = LONG(CalPreglue(LONG(dSumPath - AllS - lCloseDistance), FinallV, lAcceleration, lInitVelociy));
            lCloseTime = lCloseTime - AllT;
            /*======計時器到觸發中斷執行斷膠，使用z中斷執行================*/
            if(!m_bIsStop)
            {
                CAction::m_ZtimeOutGlueSet = FALSE;
                //_cwprintf(_T("End關機距離的時間=%d \n"), lCloseTime);
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
        CAction::m_ZtimeOutGlueSet = FALSE;
        MO_TimerSetIntter(finishTime-closeDistTime, 1);*/
    }

    if(lStartDelayTime == 0 && lStartDistance == 0)
    {
        PauseDoGlue();//暫停恢復後繼續出膠(m_bIsPause=0)出膠
    }

    MO_DO2Curve(DATA_2Do, DATA_2DO.size() - 1, lWorkVelociy);
    PreventMoveError();//防止驅動錯誤
    Sleep(200);
    DATA_2DO.clear();

    //使用(3)停留時間(lCloseOffDelayTime)
    if(lCloseOffDelayTime>0)
    {
        MO_Timer(0, 0, lCloseOffDelayTime * 1000);
        MO_Timer(1, 0, lCloseOffDelayTime * 1000);//線段點膠設定---(3)停留時間
        Sleep(1);//防止出錯，避免計時器初直為0
        while(MO_Timer(3, 0, 0))
        {
            if(m_bIsStop == 1)
            {
                break;
            }
            Sleep(1);
        }
    }

    PauseStopGlue();//暫停時停指塗膠(m_bIsPause=1)
    MO_StopGumming();//停止出膠

    //使用(6)關機延遲(lCloseONDelayTime)
    if(!(lCloseDistance>0) && lCloseONDelayTime>0)
    {
        MO_Timer(0, 0, lCloseONDelayTime * 1000);
        MO_Timer(1, 0, lCloseONDelayTime * 1000);//線段點膠設定---(6)關機延遲
        Sleep(1);//防止出錯，避免計時器初直為0
        while(MO_Timer(3, 0, 0))
        {
            if(m_bIsStop == 1)
            {
                break;
            }
            Sleep(1);
        }
    }

    if(!m_bIsStop)
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
    if(dRadius == 0)
    {
        return;
    }
    dAngCenCos = acos(DOUBLE(cPt1.x - cPtCen.x) / dRadius);
    dAngCenSin = asin(DOUBLE(cPt1.y - cPtCen.y) / dRadius);
    dAngCenCos2 = M_PI * 2 - dAngCenCos;
    dAngCenSin2 = M_PI - dAngCenSin;
    if(abs(dAngCenCos - dAngCenSin) > 0.01)
    {
        if(abs(dAngCenCos - dAngCenSin2) < 0.01)
        {
            dAngCenSin = dAngCenSin2;
        }
        else if(abs(dAngCenCos2 - dAngCenSin) < 0.01)
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
    if(lStartDelayTime>0)
    {
        if(!m_bIsStop)
        {
            MO_Do3DLineMove(lX1 - MO_ReadLogicPosition(0), lY1 - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration,
                            lInitVelociy);//回到起始點!
            PreventMoveError();
        }
        if(!m_bIsStop)
        {
            MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//回到起始點!
            PreventMoveError();
        }
        PauseDoGlue();//暫停回復後重新塗膠(讀取暫停參數，當參數為0時出膠，且點膠機要為開。)
        if(!m_bIsStop && m_bIsDispend == 1)
        {
            MO_GummingSet();//塗膠(不卡)
        }
        MO_Timer(0, 0, lStartDelayTime * 1000);
        MO_Timer(1, 0, lStartDelayTime * 1000);//線段點膠設定---(1)移動前延遲(在線段開始點上)

        Sleep(1);//防止出錯，避免計時器初直為0
        while(MO_Timer(3, 0, 0))
        {
            if(m_bIsStop)
            {
                break;
            }
            Sleep(1);
        }
        /*插入第一點*/
        m_ptVec.push_back(cPt1);
    }
    //使用(2)計算出膠距離，設定出膠點
    else if(lStartDistance>0)
    {
        CPoint ptSetDist(0, 0);
        LONG glueDist = lStartDistance;
        DOUBLE dLength = sqrt(pow(cPt1.x - cPt2.x, 2) + pow(cPt1.y - cPt2.y, 2));
        if(cPt1.x == cPt2.x)
        {
            ptSetDist = cPt1;
            glueDist = LONG(glueDist*M_SQRT1_2);
            ptSetDist.y = (cPt1.y<cPt2.y) ? cPt1.y - glueDist : cPt1.y + glueDist;
        }
        else if(cPt1.y == cPt2.y)
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
    if(dAngCos < 0)
    {
        dAngCos += M_PI * 2;
    }
    if(dAngSin < 0)
    {
        dAngSin += M_PI * 2;
    }
    dAngCos2 = M_PI * 2 - dAngCos;
    dAngSin2 = M_PI - dAngSin;
    if(abs(dAngCos - dAngSin) > 0.01)
    {
        if(abs(dAngCos - dAngSin2) < 0.01)
        {
            dAngSin = dAngSin2;
        }
        else if(abs(dAngCos2 - dAngSin) < 0.01)
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
    while(1)
    {
        if((iBuff == 1) && (dRadius - (2 * dWidth2) < 0))
        {
#ifdef PRINTF
      //      _cwprintf(_T("兩端寬度過大 \n"));
#endif
            m_ptVec.push_back(cPt1);
            break;
        }
        dRadius = dRadius - dWidth*sqrt(2);
        dDistance = dDistance - dWidth;
        if(iBuff*dWidth >  dWidth2)    //dWidth2兩端寬度
        {
            dDistance += dWidth;
            cPt1.x = LONG(dDistance*cos(dAngCos) + cPt4.x);
            cPt1.y = LONG(dDistance*sin(dAngSin) + cPt4.y);
            m_ptVec.push_back(cPt1);
            break;
        }
        if(dDistance < dWidth)
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
        if(dDistance < dWidth)
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
    for(ptIter = m_ptVec.begin(); ptIter != m_ptVec.end(); ptIter++)
    {
        MCO_Do2dDataLine((*ptIter).x, (*ptIter).y, DATA_2DO);
    }
    //點陣列轉換
    LA_AbsToOppo2Move(DATA_2DO);
    for(UINT i = 1; i < DATA_2DO.size(); i++)
    {
        DATA_2Do[i - 1] = DATA_2DO.at(i);
    }
    if(!m_bIsStop)
    {
        MO_Do3DLineMove(DATA_2DO.at(0).EndP.x, DATA_2DO.at(0).EndP.y, 0, lWorkVelociy, lAcceleration,
                        lInitVelociy);//回到起始點!
        PreventMoveError();
    }
    if(!m_bIsStop)
    {
        MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//回到起始點!
        PreventMoveError();
    }
    //使用(2)設置距離
    if(lStartDistance>0)
    {
        //timeUpGlue 單位us
        LONG timeUpGlue = CalPreglue(lStartDistance, lWorkVelociy, lAcceleration, lInitVelociy);
        if(!m_bIsStop)
        {
            m_YtimeOutGlueSet = TRUE;
            MO_TimerSetIntter(timeUpGlue, 0);//使用z timer中斷 出膠
        }
    }
    //使用(5)關機距離(lCloseDistance)
    if(lCloseDistance>0)
    {
        LONG sumPath = 0;
        LONG finishTime = 0;
        DOUBLE avgTime = 0;
        LONG accLength = CalPreglue(lWorkVelociy, lAcceleration, lInitVelociy);
        for(UINT i = 1; i<DATA_2DO.size(); i++)
        {
            sumPath += DATA_2DO.at(i).Distance;
        }
        avgTime = ((DOUBLE)sumPath - (DOUBLE)accLength) / (DOUBLE)lWorkVelociy;
        finishTime = (LONG)round(avgTime * 1000000) + CalPreglueTime(lWorkVelociy, lAcceleration, lInitVelociy);
        LONG closeDistTime = CalPreglue(lCloseDistance, lWorkVelociy, 0, lInitVelociy);
        if(!m_bIsStop)
        {
            CAction::m_ZtimeOutGlueSet = FALSE;
            MO_TimerSetIntter(finishTime - closeDistTime, 1);
        }
    }

    if(lStartDelayTime == 0 && lStartDistance == 0)
    {
        PauseDoGlue();//暫停恢復後繼續出膠(m_bIsPause=0)出膠
    }
    MO_DO2Curve(DATA_2Do, DATA_2DO.size() - 1, lWorkVelociy);
    PreventMoveError();//防止驅動錯誤
    Sleep(200);
    DATA_2DO.clear();

    //使用(3)停留時間(lCloseOffDelayTime)
    if(lCloseOffDelayTime>0)
    {
        MO_Timer(0, 0, lCloseOffDelayTime * 1000);
        MO_Timer(1, 0, lCloseOffDelayTime * 1000);//線段點膠設定---(3)停留時間
        Sleep(1);//防止出錯，避免計時器初直為0
        while(MO_Timer(3, 0, 0))
        {
            if(m_bIsStop == 1)
            {
                break;
            }
            Sleep(1);
        }
    }

    PauseStopGlue();//暫停時停指塗膠(m_bIsPause=1)
    MO_StopGumming();//停止出膠
    //使用(6)關機延遲(lCloseONDelayTime)
    if(!(lCloseDistance>0) && lCloseONDelayTime>0)
    {
        MO_Timer(0, 0, lCloseONDelayTime * 1000);
        MO_Timer(1, 0, lCloseONDelayTime * 1000);//線段點膠設定---(6)關機延遲
        Sleep(1);//防止出錯，避免計時器初直為0
        while(MO_Timer(3, 0, 0))
        {
            if(m_bIsStop == 1)
            {
                break;
            }
            Sleep(1);
        }
    }
    if(!m_bIsStop)
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
    if(LONG(dRadius) == 0)
    {
        return;
    }
    if(LONG(dRadius) == 0)
    {
        return;
    }
    if(lStartDistance>0) //使用--(2)設置距離(lStartDistance)
    {
        LONG lStartX = 0, lStartY = 0;
        ArcGetToPoint(lStartX, lStartY, lStartDistance, lX1, lY1, lCenX, lCenY, LONG(dRadius), bRev);//算出起始點
        if(!m_bIsStop)
        {
            MO_Do3DLineMove(lStartX - MO_ReadLogicPosition(0), lStartY - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration,
                            lInitVelociy);//回到起始點!
            PreventMoveError();
        }
        if(!m_bIsStop)
        {
            MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//回到起始點!
            PreventMoveError();
        }
        lTime = LONG(1000000 * (DOUBLE)lStartDistance / (DOUBLE)lWorkVelociy);
#ifdef PRINTF
        //_cwprintf(_T("計時器設置距離的時間=%lf \n"), DOUBLE(lTime / 1000000.0));
#endif
        /*======計時器到觸發中斷執行出膠，使用y中斷執行================*/
        if(!m_bIsStop)
        {
            CAction::m_YtimeOutGlueSet = TRUE;
            MO_TimerSetIntter(lTime, 0);//計時到跳至執行序
        }
    }
    else
    {

        if(!m_bIsStop)
        {
            MO_Do3DLineMove(lX1 - MO_ReadLogicPosition(0), lY1 - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration,
                            lInitVelociy);//回到起始點!
            PreventMoveError();
        }
        if(!m_bIsStop)
        {
            MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//回到起始點!
            PreventMoveError();
        }
        PauseDoGlue();//暫停回復後重新塗膠(讀取暫停參數，當參數為0時出膠，且點膠機要為開。)
        if(!m_bIsStop && m_bIsDispend == 1)
        {
            MO_GummingSet();//塗膠(不卡)
        }
        if(lStartDelayTime > 0) //使用(1)移動前延遲(lStartDelayTime)
        {
            MO_Timer(0, 0, lStartDelayTime * 1000);
            MO_Timer(1, 0, lStartDelayTime * 1000);//線段點膠設定---(1)移動前延遲(在線段開始點上)
            Sleep(1);//防止出錯，避免計時器初直為0
            while(MO_Timer(3, 0, 0))
            {
                if(m_bIsStop)
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
    while(1)
    {
        if((iBuff == 1) && (lDistance - (2 * dWidth2) < 0)) //表示使用單圓環不螺旋
        {
#ifdef PRINTF
            //_cwprintf(_T("兩端寬度過大 \n"));
#endif
            dSumPath += M_PI*dRadius;
            iBuff = 1;
            m_ptVec.push_back(cPt1);//走單圈圓
            break;
        }
        else if((iBuff == 1) && dWidth2 == 0) //表示使用單圓環不螺旋
        {
#ifdef PRINTF
            //_cwprintf(_T("單圓環 \n"));
#endif
            dSumPath += M_PI*dRadius;
            iBuff = 1;
            m_ptVec.push_back(cPt1);//走單圈圓
            break;
        }
        dRadius = dRadius - dWidth;
        lDistance = LONG(lDistance - dWidth);
        if(iBuff * dWidth > dWidth2)
        {
            lDistance += LONG(dWidth);
            LineGetToPoint(lXClose, lYClose, cPt1.x, cPt1.y, cPtCen1.x, cPtCen1.y,
                           lDistance);
            cPt3.x = lXClose;
            cPt3.y = lYClose;
            m_ptVec.push_back(cPt3);
            if(iBuff == 1)
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
        if(iBuff == 1)
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
    if(lCloseDistance > 0) //使用 --(5)關機距離
    {
        dCloseTime = 1000000 * ((dSumPath - lCloseDistance) / (DOUBLE)lWorkVelociy);
        /*======計時器到觸發中斷執行斷膠，使用z中斷執行================*/
        if(!m_bIsStop)
        {
            CAction::m_ZtimeOutGlueSet = FALSE;
            if(lStartDistance > 0)
            {
#ifdef PRINTF
               // _cwprintf(_T("End關機距離的時間=%lf \n"), DOUBLE(lTime) + DOUBLE(dCloseTime / 1000000.0));
#endif
                MO_TimerSetIntter(lTime + LONG(dCloseTime), 1);//計時到跳至執行序
            }
            else
            {
#ifdef PRINTF
             //   _cwprintf(_T("End關機距離的時間=%lf \n"), DOUBLE(dCloseTime / 1000000.0));
#endif
                MO_TimerSetIntter(LONG(dCloseTime), 1);//計時到跳至執行序
            }
        }
    }
    #pragma endregion
    std::vector<DATA_2MOVE> DATA_2DO;
    DATA_2DO.clear();
    if(iBuff == 1) //表示兩端寬度為零  為畫出一個單圓環
    {
        for(UINT i = 1; i < m_ptVec.size(); i++)
        {
            {
                MCO_Do2dDataCir(m_ptVec.at(i).x, m_ptVec.at(i).y, cPtCen1.x, cPtCen1.y, bRev, DATA_2DO);//上半圓
            }
        }
    }
    else
    {
        for(UINT i = 1; i < m_ptVec.size(); i++)
        {
            if(i == m_ptVec.size() - 1)
            {
                if(i % 2 == 0)
                {
                    MCO_Do2dDataCir(m_ptVec.at(i).x, m_ptVec.at(i).y, cPtCen1.x, cPtCen1.y, bRev, DATA_2DO);//上半圓
                }
                else
                {
                    MCO_Do2dDataCir(m_ptVec.at(i).x, m_ptVec.at(i).y, cPtCen2.x, cPtCen2.y, bRev, DATA_2DO);//下半圓
                }
            }
            else
            {
                if(i % 2 != 0)
                {
                    MCO_Do2dDataCir(m_ptVec.at(i).x, m_ptVec.at(i).y, cPtCen1.x, cPtCen1.y, bRev, DATA_2DO);//上半圓
                }
                else
                {
                    MCO_Do2dDataCir(m_ptVec.at(i).x, m_ptVec.at(i).y, cPtCen2.x, cPtCen2.y, bRev, DATA_2DO);//下半圓
                }
            }
        }
    }
    LA_AbsToOppo2Move(DATA_2DO);
    for(UINT i = 0; i < DATA_2DO.size(); i++)
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
    while(MO_Timer(3, 0, 0))
    {
        if(m_bIsStop == 1)
        {
            break;
        }
        Sleep(1);
    }
    MO_StopGumming();//停止出膠
    if(!m_bIsStop)
    {
        //*************************填充回升z軸速度目前為驅動速度的兩倍******************
        MO_Do3DLineMove(0, 0, (lZ - lZBackDistance) - lZ, lWorkVelociy * 2, lAcceleration,
                        lInitVelociy);//Z軸返回
        PreventMoveError();//防止軸卡出錯
    }
    MO_Timer(0, 0, lCloseOffDelayTime * 1000);
    MO_Timer(1, 0, lCloseOffDelayTime * 1000);//線段點膠設定---(3)停留時間
    Sleep(1);//防止出錯，避免計時器初直為0
    while(MO_Timer(3, 0, 0))
    {
        if(m_bIsStop == 1)
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
    if(dRadius == 0)
    {
        return;
    }
    dAngCenCos = acos(DOUBLE(cPt1.x - cPtCen.x) / dRadius);
    dAngCenSin = asin(DOUBLE(cPt1.y - cPtCen.y) / dRadius);
    dAngCenCos2 = M_PI * 2 - dAngCenCos;
    dAngCenSin2 = M_PI - dAngCenSin;
    if(abs(dAngCenCos - dAngCenSin) > 0.01)
    {
        if(abs(dAngCenCos - dAngCenSin2) < 0.01)
        {
            dAngCenSin = dAngCenSin2;
        }
        else if(abs(dAngCenCos2 - dAngCenSin) < 0.01)
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
    if(dAngCos < 0)
    {
        dAngCos += M_PI * 2;
    }
    if(dAngSin < 0)
    {
        dAngSin += M_PI * 2;
    }
    dAngCos2 = M_PI * 2 - dAngCos;
    dAngSin2 = M_PI - dAngSin;
    if(abs(dAngCos - dAngSin) > 0.01)
    {
        if(abs(dAngCos - dAngSin2) < 0.01)
        {
            dAngSin = dAngSin2;
        }
        else if(abs(dAngCos2 - dAngSin) < 0.01)
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
    while(1)
    {
        dRadius = dRadius - dWidth*sqrt(2);
        dDistance = dDistance - dWidth;
        if(dDistance < dWidth)
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
        if(dDistance < dWidth)
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
    if(lStartDelayTime>0)
    {
        if(!m_bIsStop)
        {
            rptIter = m_ptVec.rbegin();
            MO_Do3DLineMove(rptIter->x - MO_ReadLogicPosition(0), rptIter->y - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration,
                            lInitVelociy);//回到起始點!
            PreventMoveError();
        }
        if(!m_bIsStop)
        {
            MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//回到起始點!
            PreventMoveError();
        }
        PauseDoGlue();//暫停回復後重新塗膠(讀取暫停參數，當參數為0時出膠，且點膠機要為開。)
        if(!m_bIsStop && m_bIsDispend == 1)
        {
            MO_GummingSet();//塗膠(不卡)
        }
        MO_Timer(0, 0, lStartDelayTime * 1000);
        MO_Timer(1, 0, lStartDelayTime * 1000);//線段點膠設定---(1)移動前延遲(在線段開始點上)

        Sleep(1);//防止出錯，避免計時器初直為0
        while(MO_Timer(3, 0, 0))
        {
            if(m_bIsStop)
            {
                break;
            }
            Sleep(1);
        }
    }
    //使用(2)計算出膠距離，設定出膠點
    else if(lStartDistance>0)
    {
        rptIter = m_ptVec.rbegin();
        CPoint rPt1 = *rptIter;
        CPoint rPt2 = *(rptIter + 1);
        CPoint ptSetDist(0, 0);
        LONG glueDist = lStartDistance;
        DOUBLE dLength = sqrt(pow(rPt1.x - rPt2.x, 2) + pow(rPt1.y - rPt2.y, 2));
        if(rPt1.x == rPt2.x)
        {
            ptSetDist = rPt1;
            glueDist = LONG(glueDist*M_SQRT1_2);
            ptSetDist.y = (rPt1.y<rPt2.y) ? rPt1.y - glueDist : rPt1.y + glueDist;
        }
        else if(rPt1.y == rPt2.y)
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
    for(rptIter = m_ptVec.rbegin(); rptIter != m_ptVec.rend(); rptIter++)
    {
        MCO_Do2dDataLine((*rptIter).x, (*rptIter).y, DATA_2DO);
    }

    LA_AbsToOppo2Move(DATA_2DO);
    for(UINT i = 1; i < DATA_2DO.size(); i++)
    {
        DATA_2Do[i - 1] = DATA_2DO.at(i);
    }
    if(!m_bIsStop)
    {
        MO_Do3DLineMove(DATA_2DO.at(0).EndP.x, DATA_2DO.at(0).EndP.y, 0, lWorkVelociy,
                        lAcceleration, lInitVelociy);//移動
        PreventMoveError();
    }
    if(!m_bIsStop)
    {
        MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//Z軸往下
        PreventMoveError();//防止軸卡出錯
    }
    //使用(2)設置距離
    if(lStartDistance>0)
    {
        //timeUpGlue 單位us
        LONG timeUpGlue = CalPreglue(lStartDistance, lWorkVelociy, lAcceleration, lInitVelociy);
        if(!m_bIsStop)
        {
            m_YtimeOutGlueSet = TRUE;
            MO_TimerSetIntter(timeUpGlue, 0);//使用Y timer中斷 出膠
        }
    }
    //使用(5)關機距離(lCloseDistance)
    if(lCloseDistance>0)
    {
        LONG sumPath = 0;
        LONG finishTime = 0;
        DOUBLE avgTime = 0;
        LONG accLength = CalPreglue(lWorkVelociy, lAcceleration, lInitVelociy);
        for(UINT i = 1; i<DATA_2DO.size(); i++)
        {
            sumPath += DATA_2DO.at(i).Distance;
        }
        avgTime = ((DOUBLE)sumPath - (DOUBLE)accLength) / (DOUBLE)lWorkVelociy;
        finishTime = (LONG)round(avgTime * 1000000) + CalPreglueTime(lWorkVelociy, lAcceleration, lInitVelociy);
        LONG closeDistTime = CalPreglue(lCloseDistance, lWorkVelociy, 0, lInitVelociy);
        if(!m_bIsStop)
        {
            CAction::m_ZtimeOutGlueSet = FALSE;
            MO_TimerSetIntter(finishTime - closeDistTime, 1);
        }
    }

    if(lStartDelayTime == 0 && lStartDistance == 0)
    {
        PauseDoGlue();//暫停恢復後繼續出膠(m_bIsPause=0)出膠
    }
    MO_DO2Curve(DATA_2Do, DATA_2DO.size() - 1, lWorkVelociy);
    PreventMoveError();//防止驅動錯誤
    Sleep(200);
    DATA_2DO.clear();
    //使用(3)停留時間(lCloseOffDelayTime)
    if(lCloseOffDelayTime>0)
    {
        MO_Timer(0, 0, lCloseOffDelayTime * 1000);
        MO_Timer(1, 0, lCloseOffDelayTime * 1000);//線段點膠設定---(3)停留時間
        Sleep(1);//防止出錯，避免計時器初直為0
        while(MO_Timer(3, 0, 0))
        {
            if(m_bIsStop == 1)
            {
                break;
            }
            Sleep(1);
        }
    }
    PauseStopGlue();//暫停時停指塗膠(m_bIsPause=1)
    MO_StopGumming();//停止出膠
    //使用(6)關機延遲(lCloseONDelayTime)
    if(!(lCloseDistance>0) && lCloseONDelayTime>0)
    {
        MO_Timer(0, 0, lCloseONDelayTime * 1000);
        MO_Timer(1, 0, lCloseONDelayTime * 1000);//線段點膠設定---(6)關機延遲
        Sleep(1);//防止出錯，避免計時器初直為0
        while(MO_Timer(3, 0, 0))
        {
            if(m_bIsStop == 1)
            {
                break;
            }
            Sleep(1);
        }
    }
    if(!m_bIsStop)
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
    if(LONG(dRadius) == 0)
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
    while(1)
    {
        lDistance = LONG(lDistance - dWidth);
        if(lDistance < dWidth)
        {
            dSumPath -= M_PI / 2 * (sqrt(pow(cPt3.x - cPt4.x, 2) + pow(cPt3.y - cPt4.y, 2)));
            dSumPath += sqrt(pow(cPt3.x - cPt4.x, 2) + pow(cPt3.y - cPt4.y, 2));
            break;
        }
        LineGetToPoint(lXClose, lYClose, cPt1.x, cPt1.y, cPtCen1.x, cPtCen1.y,
                       lDistance);
        cPt3.x = lXClose;
        cPt3.y = lYClose;
        if(icnt == 0)
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
    if(lStartDistance>0) //使用--(2)設置距離(lStartDistance)
    {
        LONG lStartX = 0, lStartY = 0;
        LineGetToPoint(lStartX, lStartY, lX1, lY1, lCenX, lCenY, lStartDistance);//算出起始點
        if(!m_bIsStop)
        {
            MO_Do3DLineMove(lStartX - MO_ReadLogicPosition(0), lStartY - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration,
                            lInitVelociy);//回到起始點!
            PreventMoveError();
        }
        if(!m_bIsStop)
        {
            MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//回到起始點!
            PreventMoveError();
        }
        lTime = CalPreglue(lStartDistance, lWorkVelociy, lAcceleration, lInitVelociy);
#ifdef PRINTF
        //_cwprintf(_T("計時器設置距離的時間=%lf \n"), DOUBLE(lTime / 1000000.0));
#endif
        /*======計時器到觸發中斷執行出膠，使用y中斷執行================*/
        if(!m_bIsStop)
        {
            CAction::m_YtimeOutGlueSet = TRUE;
            MO_TimerSetIntter(lTime, 0);//計時到跳至執行序
        }
    }
    else
    {
        //先抬升，移動到中心點在下降
        if(!m_bIsStop)
        {
            MO_Do3DLineMove(lCenX - MO_ReadLogicPosition(0), lCenY - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration,
                            lInitVelociy);//直線移動至圓心
            PreventMoveError();
        }
        if(!m_bIsStop)
        {
            MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration,
                            lInitVelociy);//Z軸往下
            PreventMoveError();//防止軸卡出錯
        }
        PauseDoGlue();//暫停回復後重新塗膠(讀取暫停參數，當參數為0時出膠，且點膠機要為開。)
        if(!m_bIsStop && m_bIsDispend == 1)
        {
            MO_GummingSet();//塗膠(不卡)
        }
        if(lStartDelayTime > 0) //使用(1)移動前延遲(lStartDelayTime)
        {
            MO_Timer(0, 0, lStartDelayTime * 1000);
            MO_Timer(1, 0, lStartDelayTime * 1000);//線段點膠設定---(1)移動前延遲(在線段開始點上)
            Sleep(1);//防止出錯，避免計時器初直為0
            while(MO_Timer(3, 0, 0))
            {
                if(m_bIsStop)
                {
                    break;
                }
                Sleep(1);
            }
        }
    }
    if(lCloseDistance > 0) //使用 --(5)關機距離
    {
        dCloseTime = 1000000 * ((dSumPath - lCloseDistance) / (DOUBLE)lWorkVelociy);
        /*======計時器到觸發中斷執行斷膠，使用z中斷執行================*/
        if(!m_bIsStop)
        {
            CAction::m_ZtimeOutGlueSet = FALSE;
            if(lStartDistance > 0)
            {
#ifdef PRINTF
           //     _cwprintf(_T("End關機距離的時間=%lf \n"), DOUBLE(lTime) + DOUBLE(dCloseTime / 1000000.0));
#endif
                MO_TimerSetIntter(lTime + LONG(dCloseTime), 1);//計時到跳至執行序
            }
            else
            {
#ifdef PRINTF
            //    _cwprintf(_T("End關機距離的時間=%lf \n"), DOUBLE(dCloseTime / 1000000.0));
#endif
                MO_TimerSetIntter(LONG(dCloseTime), 1);//計時到跳至執行序
            }
        }
    }
    std::vector<DATA_2MOVE> DATA_2DO;
    DATA_2DO.clear();
    for(rptIter = m_ptVec.rbegin(); rptIter != m_ptVec.rend(); rptIter++)
    {
        if((iData != 0) && (iOdd == 0))
        {
            MCO_Do2dDataLine((*rptIter).x, (*rptIter).y, DATA_2DO);
        }
        else if((iData == 0) && (iOdd == 0))
        {
            MCO_Do2dDataCir((*rptIter).x, (*rptIter).y, cPtCen2.x, cPtCen2.y, bRev, DATA_2DO);
        }
        else
        {
            if(iOdd % 2 == 0)
            {
                MCO_Do2dDataCir((*rptIter).x, (*rptIter).y, cPtCen2.x, cPtCen2.y, bRev, DATA_2DO);
            }
            else
            {
                MCO_Do2dDataCir((*rptIter).x, (*rptIter).y, lCenX, lCenY, bRev, DATA_2DO);
            }
        }
        iOdd++;
    }
    LA_AbsToOppo2Move(DATA_2DO);
    for(UINT i = 0; i < DATA_2DO.size(); i++)
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
    while(MO_Timer(3, 0, 0))
    {
        if(m_bIsStop == 1)
        {
            break;
        }
        Sleep(1);
    }
    MO_StopGumming();//停止出膠
    if(!m_bIsStop)
    {
        //*************************填充回升z軸速度目前為驅動速度的兩倍******************
        MO_Do3DLineMove(0, 0, (lZ - lZBackDistance) - lZ, lWorkVelociy * 2, lAcceleration,
                        lInitVelociy);//Z軸返回
        PreventMoveError();//防止軸卡出錯
    }
    MO_Timer(0, 0, lCloseOffDelayTime * 1000);
    MO_Timer(1, 0, lCloseOffDelayTime * 1000);//線段點膠設定---(3)停留時間
    Sleep(1);//防止出錯，避免計時器初直為0
    while(MO_Timer(3, 0, 0))
    {
        if(m_bIsStop == 1)
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
    if(dRadius == 0)
    {
        return;
    }
    dAngCenCos = acos(DOUBLE(cPt1.x - cPtCen.x) / dRadius);
    dAngCenSin = asin(DOUBLE(cPt1.y - cPtCen.y) / dRadius);
    dAngCenCos2 = M_PI * 2 - dAngCenCos;
    dAngCenSin2 = M_PI - dAngCenSin;
    if(abs(dAngCenCos - dAngCenSin) > 0.01)
    {
        if(abs(dAngCenCos - dAngCenSin2) < 0.01)
        {
            dAngCenSin = dAngCenSin2;
        }
        else if(abs(dAngCenCos2 - dAngCenSin) < 0.01)
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
    if(dAngCos < 0)
    {
        dAngCos += M_PI * 2;
    }
    if(dAngSin < 0)
    {
        dAngSin += M_PI * 2;
    }
    dAngCos2 = M_PI * 2 - dAngCos;
    dAngSin2 = M_PI - dAngSin;
    if(abs(dAngCos - dAngSin) > 0.01)
    {
        if(abs(dAngCos - dAngSin2) < 0.01)
        {
            dAngSin = dAngSin2;
        }
        else if(abs(dAngCos2 - dAngSin) < 0.01)
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
    while(1)
    {
        dRadius = dRadius - dWidth*sqrt(2);
        dDistance = dDistance - dWidth;
        if(dDistance < dWidth)
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
        if(dDistance < dWidth)
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
    for(ptIter = m_ptVec.begin(); ptIter != m_ptVec.end(); ptIter++)
    {
        MCO_Do2dDataLine((*ptIter).x, (*ptIter).y, DATA_2DO);
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
    if(dRadius == 0)
    {
        return;
    }
    dAngCenCos = acos(DOUBLE(cPt1.x - cPtCen.x) / dRadius);
    dAngCenSin = asin(DOUBLE(cPt1.y - cPtCen.y) / dRadius);
    dAngCenCos2 = M_PI * 2 - dAngCenCos;
    dAngCenSin2 = M_PI - dAngCenSin;
    if(abs(dAngCenCos - dAngCenSin) > 0.01)
    {
        if(abs(dAngCenCos - dAngCenSin2) < 0.01)
        {
            dAngCenSin = dAngCenSin2;
        }
        else if(abs(dAngCenCos2 - dAngCenSin) < 0.01)
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
    if(dAngCos < 0)
    {
        dAngCos += M_PI * 2;
    }
    if(dAngSin < 0)
    {
        dAngSin += M_PI * 2;
    }
    dAngCos2 = M_PI * 2 - dAngCos;
    dAngSin2 = M_PI - dAngSin;
    if(abs(dAngCos - dAngSin) > 0.01)
    {
        if(abs(dAngCos - dAngSin2) < 0.01)
        {
            dAngSin = dAngSin2;
        }
        else if(abs(dAngCos2 - dAngSin) < 0.01)
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
    while(1)
    {
        if((iBuff == 1) && (dRadius - (2 * dWidth2) < 0))
        {
#ifdef PRINTF
         //   _cwprintf(_T("兩端寬度過大 \n"));
#endif
            m_ptVec.push_back(cPt1);
            break;
        }
        dRadius = dRadius - dWidth*sqrt(2);
        dDistance = dDistance - dWidth;
        if(iBuff*dWidth >  dWidth2)   //dWidth2兩端寬度
        {
            dDistance += dWidth;
            cPt1.x = LONG(dDistance*cos(dAngCos) + cPt4.x);
            cPt1.y = LONG(dDistance*sin(dAngSin) + cPt4.y);
            m_ptVec.push_back(cPt1);
            break;
        }
        if(dDistance < dWidth)
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
        if(dDistance < dWidth)
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
    for(ptIter = m_ptVec.begin(); ptIter != m_ptVec.end(); ptIter++)
    {
        MCO_Do2dDataLine((*ptIter).x, (*ptIter).y, DATA_2DO);
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
    if(dRadius == 0)
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
    while(1)
    {
        if((iBuff == 1) && (lDistance - (2 * dWidth2) < 0))
        {
#ifdef PRINTF
        //    _cwprintf(_T("兩端寬度過大 \n"));
#endif
            break;
        }
        dRadius = dRadius - dWidth;
        lDistance = LONG(lDistance - dWidth);
        if(iBuff * dWidth > dWidth2)
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
    for(UINT i = 1; i < m_ptVec.size(); i++)
    {
        if(i == m_ptVec.size() - 1)
        {
            if(i % 2 == 0)
            {
                MCO_Do2dDataCir(m_ptVec.at(i).x, m_ptVec.at(i).y, cPtCen1.x, cPtCen1.y, bRev, DATA_2DO);//上半圓
            }
            else
            {
                MCO_Do2dDataCir(m_ptVec.at(i).x, m_ptVec.at(i).y, cPtCen2.x, cPtCen2.y, bRev, DATA_2DO);//下半圓
            }
        }
        else
        {
            if(i % 2 != 0)
            {
                MCO_Do2dDataCir(m_ptVec.at(i).x, m_ptVec.at(i).y, cPtCen1.x, cPtCen1.y, bRev, DATA_2DO);//上半圓
            }
            else
            {
                MCO_Do2dDataCir(m_ptVec.at(i).x, m_ptVec.at(i).y, cPtCen2.x, cPtCen2.y, bRev, DATA_2DO);//下半圓
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
void CAction::MCO_Do2dDataLine(LONG EndPX, LONG EndPY, std::vector<DATA_2MOVE> &str)
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
#endif
//填充用兩軸連續差補(給值--圓)
#ifdef MOVE
void CAction::MCO_Do2dDataCir(LONG EndPX, LONG EndPY, LONG CenX, LONG CenY, BOOL bRev, std::vector<DATA_2MOVE> &str)
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
#endif
/***********************************************************
**                                                        **
**          運動模組-w軸 (對應動作解析)                     **
**                                                        **
************************************************************/
/*w軸offset更新(offset,w軸角度絕對)
*@目的:把機械位置修正成針頭位置!(針頭座標 = 機械座標 + offset)
*/
void CAction::W_UpdateNeedleMotorOffset(CPoint &offset, DOUBLE degree)
{
#ifdef MOVE
    CPoint tmpOffset(0, 0);
    tmpOffset = m_MachineOffSet;//取原始的offset
    //DOUBLE degreeNow = MO_ReadLogicPositionW(); 轉相對用
    offset.x = (LONG)round(tmpOffset.x*cos(degree *M_PI / 180.0) - tmpOffset.y*sin(degree *M_PI / 180.0));
    offset.y = (LONG)round(tmpOffset.x*sin(degree *M_PI / 180.0) + tmpOffset.y*cos(degree *M_PI / 180.0));
#endif
}

/*w軸連續更新數值
*@目的:傳入機械座標(機械座標會轉換成針頭座標進行切點 最後轉成機械座標輸出)
*/
void CAction::W_UpdateNeedleMotor_Robot(LONG lX, LONG lY, LONG lZ, DOUBLE dAngle0, LONG lX2, LONG lY2, LONG lZ2, DOUBLE dAngle1, DOUBLE DisAngle)
{
#ifdef MOVE
    //使用X,Y,Z三軸分量做計算
    CPoint cpoffsetbuf(0, 0);//offsetbuf
    W_UpdateNeedleMotorOffset(cpoffsetbuf, dAngle0);//OFFSET與角度轉換
    CPoint NeedlePointStart = { lX + cpoffsetbuf.x,lY + cpoffsetbuf.y };//機械座標轉換成針頭座標
    W_UpdateNeedleMotorOffset(cpoffsetbuf, dAngle1);//OFFSET與角度轉換
    CPoint NeedlePointEnd = { lX2 + cpoffsetbuf.x,lY2 + cpoffsetbuf.y };//機械座標轉換成針頭座標
    //  LONG Distance = (LONG)sqrt(pow(NeedlePointStart.x - NeedlePointEnd.x, 2) + pow(NeedlePointStart.y - NeedlePointEnd.y, 2));//線段總長度
	int idx = (int)((dAngle1 - dAngle0) / (DisAngle * 1.008));//固定角度走幾個點
	LONG idx_xVec = 0, idx_yVec = 0, idx_zVec = 0;
	if (fmod((dAngle1 - dAngle0), (DisAngle * 1.008)) != 0.0)
	{
		idx_xVec = (LONG)(DOUBLE(NeedlePointEnd.x - NeedlePointStart.x) / ((DOUBLE)abs(idx)+1.0));
		idx_yVec = (LONG)(DOUBLE(NeedlePointEnd.x - NeedlePointStart.x) / ((DOUBLE)abs(idx) + 1.0));
		idx_zVec = (LONG)fabs((DOUBLE)(NeedlePointEnd.y - NeedlePointStart.y) / ((DOUBLE)abs(idx) + 1.0));//每個點走多遠
	}
	else
	{
		idx_xVec = (LONG)(DOUBLE(NeedlePointEnd.x - NeedlePointStart.x) / (DOUBLE)abs(idx));
		idx_yVec = (LONG)(DOUBLE(NeedlePointEnd.x - NeedlePointStart.x) / (DOUBLE)abs(idx));
		idx_zVec = (LONG)fabs((DOUBLE)(NeedlePointEnd.y - NeedlePointStart.y) / (DOUBLE)abs(idx));//每個點走多遠
	}
    if(idx > 767)
    {
        AfxMessageBox(L"切值角度過小請重新設定");
        m_IsCutError = TRUE;
        return;
    }
    W_m_ptVec.clear();//第一次用清除
    DATA_4Do[0].EndPX = lX;
    DATA_4Do[0].EndPY = lY;
    DATA_4Do[0].EndPZ = lZ;
    DATA_4Do[0].AngleW = dAngle0;
    W_m_ptVec.push_back(DATA_4Do[0]);//輸出機械座標
    DATA_4MOVE Data4buf = { 0 };//傳值用
    DATA_4Do[0].EndPX = NeedlePointStart.x;
    DATA_4Do[0].EndPY = NeedlePointStart.y;
	DOUBLE absAngle = 0;
    for(int i = 1; i < abs(idx); i++)
    {
		absAngle = dAngle0 + DisAngle * i* 1.008* ((idx>0) ? 1 : -1);
        W_UpdateNeedleMotorOffset(cpoffsetbuf, dAngle0 + absAngle);//OFFSET與角度轉換
        //LineGetToPoint(DATA_4Do[i].EndPX, DATA_4Do[i].EndPY, NeedlePointEnd.x, NeedlePointEnd.y, DATA_4Do[i - 1].EndPX, DATA_4Do[i - 1].EndPY, idx_dis);
        DATA_4Do[i].EndPX = NeedlePointStart.x + idx_xVec * i;
        DATA_4Do[i].EndPY = NeedlePointStart.y + idx_yVec * i;
        Data4buf.EndPX = DATA_4Do[i].EndPX - cpoffsetbuf.x;//針頭座標-offset  會得到機械座標!!!!!
        Data4buf.EndPY = DATA_4Do[i].EndPY - cpoffsetbuf.y;//針頭座標-offset  會得到機械座標!!!!!
        if(lZ == lZ2)
        {
            Data4buf.EndPZ = lZ;
        }
        else if(lZ > lZ2)
        {
            Data4buf.EndPZ = lZ - idx_zVec * i;
        }
        else
        {
            Data4buf.EndPZ = lZ + idx_zVec * i;
        }
        Data4buf.AngleW = absAngle;//w絕對角度
        W_m_ptVec.push_back(Data4buf);  //輸出機械座標
    }
    DATA_4Do[0].EndPX = lX2;
    DATA_4Do[0].EndPY = lY2;
    DATA_4Do[0].EndPZ = lZ2;
    DATA_4Do[0].AngleW = dAngle1;
    W_m_ptVec.push_back(DATA_4Do[0]);//輸出機械座標
    m_IsCutError = FALSE;
#endif
}
/*w軸連續更新數值
*@目的:傳入針頭座標(針頭座標進行切點 最後轉成機械座標輸出)
*/
void CAction::W_UpdateNeedleMotor_Needle(LONG lX, LONG lY, LONG lZ, DOUBLE dAngle0, LONG lX2, LONG lY2, LONG lZ2, DOUBLE dAngle1, DOUBLE DisAngle)
{
#ifdef MOVE
   //使用X,Y,Z三軸分量做計算
    //LONG Distance = (LONG)sqrt(pow(lX - lX2, 2) + pow(lY - lY2, 2)+ pow(lZ - lZ2, 2));//線段總長度
	int idx = (int)((dAngle1 - dAngle0) / (DisAngle * 1.008));//固定角度走幾個點
	//DOUBLE dFin = dAngle1 - (DisAngle * 1.008)*idx;
	LONG idx_xVec = 0 , idx_yVec =0, idx_zVec =0;
	if (fmod((dAngle1 - dAngle0),(DisAngle * 1.008)) != 0.0)
	{
		idx_xVec = (LONG)(DOUBLE(lX2 - lX) / ((DOUBLE)abs(idx) + 1.0));
		idx_yVec = (LONG)(DOUBLE(lY2 - lY) / ((DOUBLE)abs(idx) + 1.0));
		idx_zVec = (LONG)abs((DOUBLE(lZ2 - lZ) / ((DOUBLE)abs(idx) + 1.0)));//每個點走多遠
	}
	else
	{
		idx_xVec = (LONG)(DOUBLE(lX2 - lX) / (DOUBLE)abs(idx));
		idx_yVec = (LONG)(DOUBLE(lY2 - lY) / (DOUBLE)abs(idx));
		idx_zVec = (LONG)abs((DOUBLE(lZ2 - lZ) / (DOUBLE)abs(idx)));//每個點走多遠
	}
    DATA_4MOVE Data4buf = { 0 };//傳值用
    if(abs(idx) > 767)
    {
        AfxMessageBox(L"切值角度過小請重新設定");
        m_IsCutError = TRUE;
        return;
    }
    W_m_ptVec.clear();//第一次用清除
    CPoint cpoffsetbuf(0,0);//offsetbuf
    W_UpdateNeedleMotorOffset(cpoffsetbuf, dAngle0);//OFFSET與角度轉換
    DATA_4Do[0].EndPX = lX - cpoffsetbuf.x;
    DATA_4Do[0].EndPY = lY - cpoffsetbuf.y;
    DATA_4Do[0].EndPZ = lZ;
    DATA_4Do[0].AngleW = dAngle0;
    W_m_ptVec.push_back(DATA_4Do[0]);//輸出機械座標
    DATA_4Do[0].EndPX = lX;
    DATA_4Do[0].EndPY = lY;
	DOUBLE absAngle=0;
    for(int i = 1; i < abs(idx); i++)
    {
		absAngle = dAngle0 + DisAngle * i* 1.008* ((idx>0) ? 1 : -1);
        W_UpdateNeedleMotorOffset(cpoffsetbuf, absAngle);//OFFSET與角度轉換
        DATA_4Do[i].EndPX = lX + idx_xVec * i;
        DATA_4Do[i].EndPY = lY + idx_yVec * i;
        Data4buf.EndPX = DATA_4Do[i].EndPX - cpoffsetbuf.x;//針頭座標-offset  會得到機械座標!!!!!
        Data4buf.EndPY = DATA_4Do[i].EndPY - cpoffsetbuf.y;//針頭座標-offset  會得到機械座標!!!!!
        if(lZ == lZ2)
        {
            Data4buf.EndPZ = lZ;
        }
        else if(lZ > lZ2)
        {
            Data4buf.EndPZ = lZ - idx_zVec * i;
        }
        else
        {
            Data4buf.EndPZ = lZ + idx_zVec * i;
        }
        Data4buf.AngleW = absAngle;//w絕對角度
        W_m_ptVec.push_back(Data4buf);  //輸出機械座標
    }
    W_UpdateNeedleMotorOffset(cpoffsetbuf, dAngle1);//OFFSET與角度轉換
    DATA_4Do[0].EndPX = lX2 - cpoffsetbuf.x;
    DATA_4Do[0].EndPY = lY2 - cpoffsetbuf.y;
    DATA_4Do[0].EndPZ = lZ2;
    DATA_4Do[0].AngleW = dAngle1;
    W_m_ptVec.push_back(DATA_4Do[0]);//輸出機械座標
    m_IsCutError = FALSE;
#endif
}
/*w軸自轉(針頭座標固定同一個點)
*@目的:切1度做旋轉
*/
void CAction::W_Rotation(DOUBLE dAngle, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy, DOUBLE DisAngle)
{
#ifdef MOVE
	if (MO_ReadLogicPositionW() == dAngle || dAngle >360 || dAngle <-360 || MO_ReadLogicPositionW() >361 || MO_ReadLogicPositionW() <-361)
	{
		return;
	}
    CPoint cpNeedlePoint[2] = { 0 };//針頭座標(0使用現在位置/1使用要移動的目標位置)
    cpNeedlePoint[0] = W_GetNeedlePoint();//將當下位置轉換成針頭座標
    //cpNeedlePoint[1] = W_GetNeedlePoint(MO_ReadLogicPosition(0), MO_ReadLogicPosition(1), dAngle, 1);//將W軸旋轉位置轉換成針頭座標
   // W_UpdateNeedleMotorOffset(cpNeedlePoint[1], dAngle);//拿到角度offset
    //W_UpdateNeedleMotor_Robot(MO_ReadLogicPosition(0), MO_ReadLogicPosition(1), MO_ReadLogicPosition(2), MO_ReadLogicPositionW(), cpNeedlePoint[0].x - cpNeedlePoint[1].x, cpNeedlePoint[0].y - cpNeedlePoint[1].y, MO_ReadLogicPosition(2), dAngle, DisAngle);//切1度轉
    //W_UpdateNeedleMotor_Needle(cpNeedlePoint[0].x, cpNeedlePoint[0].y, MO_ReadLogicPosition(2), MO_ReadLogicPositionW(), cpNeedlePoint[1].x, cpNeedlePoint[1].y, MO_ReadLogicPosition(2), dAngle, DisAngle);//切1度轉
    CPoint offset(0, 0);
    DATA_4MOVE data4M = { 0 };
	DOUBLE idx = (dAngle - MO_ReadLogicPositionW()) / (DisAngle * 1.008);
	DOUBLE absAngle = 0;
    W_m_ptVec.clear();
    for(int i = 0; i <=(LONG)fabs(idx); i++)
    {
		absAngle = MO_ReadLogicPositionW() + DisAngle*i* 1.008 * ((idx > 0) ? 1 : -1);
        W_UpdateNeedleMotorOffset(offset, absAngle);//依角度更新offset
        //針頭座標轉回機械座標(絕對座標)
        data4M.EndPX = cpNeedlePoint[0].x - offset.x;
        data4M.EndPY = cpNeedlePoint[0].y - offset.y;
        data4M.EndPZ = MO_ReadLogicPosition(2);
        data4M.AngleW = absAngle;
        W_m_ptVec.push_back(data4M);
    }
	//最後一筆
	W_UpdateNeedleMotorOffset(offset, dAngle);//依角度更新offset
	data4M.EndPX = cpNeedlePoint[0].x - offset.x;
	data4M.EndPY = cpNeedlePoint[0].y - offset.y;
	data4M.EndPZ = MO_ReadLogicPosition(2);
	data4M.AngleW = dAngle;
	W_m_ptVec.push_back(data4M);

    if(!m_bIsStop)
    {
        W_Line4DtoMove(lWorkVelociy, lAcceleration, lInitVelociy);
        PreventMoveError();//防止軸卡出錯
    }
#endif
}
/*得到針頭座標(兩種模式 0.拿到現在針頭位置 1.輸入機械座標拿到針頭位置)
*@模式0:得到"當下"針頭位置
*@模式1:輸入機械座標得到針頭位置
*/
CPoint CAction::W_GetNeedlePoint(LONG lRobotX, LONG lRobotY, DOUBLE dRobotW, BOOL bMode)
{
#ifdef MOVE
    CPoint NeedleOffset(0,0),Needlebuf(0, 0);
    if(!bMode) //模式0: 當下的機械座標轉成針頭座標輸出
    {
        W_UpdateNeedleMotorOffset(NeedleOffset, MO_ReadLogicPositionW());
        Needlebuf.x = MO_ReadLogicPosition(0) + NeedleOffset.x;
        Needlebuf.y = MO_ReadLogicPosition(1) + NeedleOffset.y;
    }
    else//模式1: 設定的機械座標轉成針頭座標輸出
    {
        W_UpdateNeedleMotorOffset(NeedleOffset, dRobotW);
        Needlebuf.x = lRobotX + NeedleOffset.x;
        Needlebuf.y = lRobotY + NeedleOffset.y;
    }
    return Needlebuf;
#endif
    return 0;
}
/*得到機械座標(兩種模式 0.拿到現在機械座標 1.輸入針頭位置拿到機械座標)
*@模式0:得到"當下"機械座標
*@模式1:輸入針頭座標得到機械座標
*/
CPoint CAction::W_GetMachinePoint(LONG lNeedleX, LONG lNeedleY, DOUBLE dNeedleW, BOOL bMode)
{
    CPoint NeedleOffset(0, 0),Machinebuf(0, 0);
    if(!bMode) //模式0: 當下的針頭座標轉成機械座標輸出
    {
#ifdef MOVE
        Machinebuf.x = MO_ReadLogicPosition(0);
        Machinebuf.y = MO_ReadLogicPosition(1);
#endif
    }
    else//模式1: 設定的針頭座標轉成機械座標輸出
    {
        W_UpdateNeedleMotorOffset(NeedleOffset, dNeedleW);
        Machinebuf.x = lNeedleX - NeedleOffset.x;
        Machinebuf.y = lNeedleY - NeedleOffset.y;
    }
    return Machinebuf;
}

#ifdef MOVE
/*絕對座標轉相對座標4軸連續插補使用*/
void CAction::W_AbsToOppo4Move(std::vector<DATA_4MOVE> &str)
{
    //機械座標
    DATA_4MOVE mData;
    std::vector<DATA_4MOVE> vecBuf;
    vecBuf.clear();
    mData.EndPX = str.at(0).EndPX - MO_ReadLogicPosition(0);
    mData.EndPY = str.at(0).EndPY - MO_ReadLogicPosition(1);
    mData.EndPZ = str.at(0).EndPZ - MO_ReadLogicPosition(2);
    mData.AngleW = str.at(0).AngleW - MO_ReadLogicPositionW();
    mData.Distance = LONG(sqrt(pow(str.at(0).EndPX, 2) + pow(str.at(0).EndPY, 2)));
    vecBuf.push_back(mData);
    for(UINT i = 1; i < str.size(); i++)
    {

        mData.Speed = str.at(i).Speed;
        mData.EndPX = str.at(i).EndPX - str.at(i - 1).EndPX;
        mData.EndPY = str.at(i).EndPY - str.at(i - 1).EndPY;
        mData.EndPZ = str.at(i).EndPZ - str.at(i - 1).EndPZ;
        mData.AngleW = str.at(i).AngleW - str.at(i - 1).AngleW;
        mData.Distance = LONG(sqrt(pow(mData.EndPX, 2) + pow(mData.EndPY, 2)));
        vecBuf.push_back(mData);
        //TRACE(_T(",%d,%d,%.3f\n"), mData.EndPX, mData.EndPY, mData.AngleW);
    }
    str.clear();
    str.insert(str.end(), vecBuf.begin(), vecBuf.end());
    for(UINT i = 0; i < str.size() - 1; i++)
    {
        DATA_4Do[i] = { 0 };
        DATA_4Do[i] = str[i + 1];
    }

}
#endif
/*連續線段動作--(四軸連續插補)
*@使用前需先寫入Vector數值(W_m_ptVec)
*/
void CAction::W_Line4DtoDo(LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
#ifdef MOVE
    //判斷W軸是否校正與取值錯誤
    if(m_MachineOffSet.x == -99999 && m_MachineOffSet.y == -99999 || m_IsCutError == TRUE)
    {
        return;
    }
    std::vector<DATA_4MOVE>W_Buff;//線段值判斷
    W_Buff.assign(W_m_ptVec.begin(), W_m_ptVec.end());
    W_AbsToOppo4Move(W_Buff);
    CPoint cpMachinePoint = W_GetMachinePoint();//將當下位置轉換成機械座標
    if(!m_bIsStop)
    {
        MO_Do4DLineMove(W_m_ptVec.at(0).EndPX - cpMachinePoint.x, W_m_ptVec.at(0).EndPY- cpMachinePoint.y,0, W_m_ptVec.at(0).AngleW - MO_ReadLogicPositionW(), lWorkVelociy, lAcceleration, lInitVelociy);//w旋轉(x,y,w同動)
        PreventMoveError();//防止軸卡出錯
    }

    if(!m_bIsStop)
    {
        MO_Do3DLineMove(0, 0, W_m_ptVec.at(0).EndPZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//z軸移動
        PreventMoveError();//防止軸卡出錯
    }
    if(!m_bIsStop)
    {
        if(!m_bIsStop && m_bIsDispend == 1)
        {
            MO_GummingSet();//塗膠(不卡)
        }
        MO_DO4Curve(DATA_4Do, W_Buff.size() - 1, lWorkVelociy);//連續插補開始
        PreventMoveError();//防止驅動錯誤
    }
    memset(&DATA_4Do, 0, sizeof(DATA_4MOVE) * (sizeof(DATA_4Do) / sizeof(DATA_4Do[0])));//清空陣列
    MO_GummingSet();//塗膠(不卡)
#endif
}
/*W軸四連續插補單純移動*/
/*連續線段動作--(四軸連續插補)
*@使用前需先寫入Vector數值(W_m_ptVec)
*/
void CAction::W_Line4DtoMove(LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy)
{
#ifdef MOVE
    //判斷W軸是否校正與取值錯誤
    if(m_MachineOffSet.x == -99999 && m_MachineOffSet.y == -99999 || m_IsCutError == TRUE)
    {
        return;
    }
    std::vector<DATA_4MOVE>W_Buff;//線段值判斷
    W_Buff.assign(W_m_ptVec.begin(), W_m_ptVec.end());
    W_AbsToOppo4Move(W_Buff);
    MO_DO4Curve(DATA_4Do, W_Buff.size() - 1, lWorkVelociy);//連續插補開始
#endif
}
/*W軸校正動作-步驟1
*@目的:取得同軸心圓圓心做標與Offset
*@參數:步驟1 bStep:0  步驟2  bStep:1
*/
void CAction::W_Correction(BOOL bStep, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy,LONG lMoveZ)
{
#ifdef MOVE
    if(!bStep)  //步驟1
    {
        m_Wangle = MO_ReadLogicPositionW();
        cpCirMidBuff[0].x = MO_ReadLogicPosition(0) ;
        cpCirMidBuff[0].y = MO_ReadLogicPosition(1) ;
		m_TablelZ = MO_ReadLogicPosition(2);//取得z軸總長度
        MO_Do3DLineMove(0, 0, lMoveZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration, lInitVelociy);//z軸抬升到10000位置
        PreventMoveError();
        MO_Do4DLineMove(0, 0, 0, 180,(LONG)round(lWorkVelociy / m_WSpeed), (LONG)(lAcceleration / m_WSpeed), lInitVelociy);//W軸旋轉180度
        PreventMoveError();

    }
    else  //步驟2
    {
        cpCirMidBuff[1].x = MO_ReadLogicPosition(0) ;
        cpCirMidBuff[1].y = MO_ReadLogicPosition(1) ;
        m_MachineCirMid.x = (LONG)round((cpCirMidBuff[0].x + cpCirMidBuff[1].x) / 2.0);//取得圓心(X)
        m_MachineCirMid.y = (LONG)round((cpCirMidBuff[0].y + cpCirMidBuff[1].y) / 2.0);//取得圓心(Y)
        m_MachineOffSet.x =  MO_ReadLogicPosition(0)- m_MachineCirMid.x;//取得offsetx
        m_MachineOffSet.y =  MO_ReadLogicPosition(1)- m_MachineCirMid.y;//取得offsety
		//奇偶校正(RESOLUTION)
		if (abs(m_MachineOffSet.x) % 2 == 1)
		{
			m_MachineOffSet.x = m_MachineOffSet.x + 1;
		}
		if (abs(m_MachineOffSet.y) % 2 == 1)
		{
			m_MachineOffSet.y = m_MachineOffSet.y + 1;
		}
        m_MachineCirRad = (LONG)sqrt(pow(m_MachineCirMid.x - MO_ReadLogicPosition(0), 2) + pow(m_MachineCirMid.y - MO_ReadLogicPosition(1), 2));//半徑
    }
#endif
}

/*讀取現在位置坐標
*@輸入:0:針頭座標 / 1:機械座標
*@
*/
AxeSpace CAction::MCO_ReadPosition(BOOL NedMah)
{
    AxeSpace SpaceBuf = {0};
#ifdef MOVE
    if(NedMah == 0) //得到針頭位置
    {
		//判斷W軸是否校正
		if (m_MachineOffSet.x == -99999 && m_MachineOffSet.y == -99999)
		{
			return SpaceBuf = {-99999,-99999,-99999 ,-99999 };
		}
        SpaceBuf.x = W_GetNeedlePoint().x;
        SpaceBuf.y = W_GetNeedlePoint().y;
        SpaceBuf.z = MO_ReadLogicPosition(2);
        SpaceBuf.w = MO_ReadLogicPositionW();
    }
    else//得到機械邏輯位置
    {
        SpaceBuf.x = MO_ReadLogicPosition(0);
        SpaceBuf.y = MO_ReadLogicPosition(1);
        SpaceBuf.z = MO_ReadLogicPosition(2);
        SpaceBuf.w = MO_ReadLogicPositionW();
    }
#endif // MOVE
    return SpaceBuf;

}
/*四軸插補移動
*@目的:四軸插補移動W軸，可切換相對與絕對(0:絕對/1:相對)
*/
void CAction::MCO_Do4DLineMove(DOUBLE dAng, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy, BOOL bIsType)
{
	/*四軸位置坐標(w角度)
	 DOUBLE dAng
	*/
	/*系統參數(驅動速度，加速度，初速度)
	LONG lWorkVelociy,LONG lAcceleration, LONG lInitVelociy
	*/
	/*相對絕對判斷
	BOOL bIsType
	*/
#ifdef MOVE
	//判斷W軸是否校正
	if (m_MachineOffSet.x == -99999 && m_MachineOffSet.y == -99999)
	{
		return;
	}
	//判斷使用針頭座標還是機械座標
	if (bIsType == 1) //使用相對座標
	{
		dAng = dAng + MO_ReadLogicPositionW();
	}
	/*******以下程式都會轉變使用絕對座標執行運動************************/
	if (!m_bIsStop)
	{
		MO_Do4DLineMove(0,0,0,dAng- MO_ReadLogicPositionW(), lWorkVelociy, lAcceleration, lInitVelociy);//四軸連續插補
		PreventMoveError();//防止軸卡出錯
	}
#endif
}
/*針頭模式原點復歸
*@輸入參數:(復歸速度1、復歸速度2、步驟0.1)
*@步驟0:初始化原點復歸移動到固定偏移量位置/
*@步驟1:正常原點復歸讓針頭座標為0
*/
void CAction::W_NeedleGoHoming(LONG Speed1, LONG Speed2,BOOL bStep)
{
#ifdef MOVE
    m_ThreadFlag = 2;//W_NeedleGoHoming
	m_IsHomingOK = 0;//原點復歸初始化
    cpCirMidBuff[0].x = Speed1;
    cpCirMidBuff[0].y = Speed2;
    cpCirMidBuff[1].x = bStep;
    AfxBeginThread(MoMoveThread, (LPVOID)this);
    /*************以下保留*******/
    //if (!bStep)
    //{
    //  DecideInitializationMachine(Speed1, Speed2, 15, m_HomingOffset_INIT.x, m_HomingOffset_INIT.y, m_HomingOffset_INIT.z, m_HomingOffset_INIT.w);
    //}
    //else
    //{
    //  //判斷W軸是否校正
    //  if (m_MachineOffSet.x == -99999 && m_MachineOffSet.y == -99999)
    //  {
    //      DecideInitializationMachine(Speed1, Speed2, 15, m_HomingOffset_INIT.x, m_HomingOffset_INIT.y, m_HomingOffset_INIT.z, m_HomingOffset_INIT.w);
    //      return;
    //  }
    //  DecideInitializationMachine(Speed1, Speed2, 15, m_HomingPoint.x + m_MachineOffSet.x, m_HomingPoint.y + m_MachineOffSet.y, m_HomingPoint.z, m_HomingPoint.w);
    //  if (!m_bIsStop)
    //  {
    //      MO_Do3DLineMove(-m_MachineOffSet.x, -m_MachineOffSet.y, 0, Speed1, Speed1 * 3, Speed2);
    //      PreventMoveError();
    //  }
    //}
#endif // MOVE
}

/*JOG手動移動模式
*@輸入相對量座標進行移動(X,Y,Z,W相對量移動/WType:0單軸,1同軸自轉/全部為0使用減速停止)
*@
*/
void CAction::MCO_JogMove(LONG lX, LONG lY, LONG lZ, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy, DOUBLE dW, BOOL WType)
{
#ifdef MOVE
    int sw=0;//選擇
    if(lX == 0 && lY == 0 && lZ == 0 && dW == 0)
    {
        sw = 0;//停止
    }
    else if(dW == 0)
    {
        sw = 1;//x,y,z單軸插補
    }
    else
    {
        sw = 2;//w軸插補
    }
    switch(sw)
    {
        case 0:
        {
            MO_DecSTOP();//減速停止
            break;
        }
        case 1:
        {
            MO_Do3DLineMove(lX, lY, lZ, lWorkVelociy, lAcceleration, lInitVelociy);//x,y,z軸相對量移動
            break;
        }
        case 2:
        {
            if(WType == 0) //W單軸移動
            {
                MO_Do4DLineMove(0, 0, 0, dW, (LONG)round(lWorkVelociy / m_WSpeed), (LONG)(lAcceleration / m_WSpeed), lInitVelociy);//w軸單軸移動
            }
            else //W自轉 已針頭為中心旋轉
            {
                //dW, lWorkVelociy, lAcceleration, lInitVelociy
                //W_Rotation(dW, lWorkVelociy, lAcceleration, lInitVelociy);
                m_ThreadFlag = 1;//W_Rotation
                WangBuff = dW;
                cpCirMidBuff[0].x = lWorkVelociy;
                cpCirMidBuff[0].y = lAcceleration;
                cpCirMidBuff[1].x = lInitVelociy;
                AfxBeginThread(MoMoveThread, (LPVOID)this);
            }
            break;
        }
        default:
            break;
    }
#endif
}

/*角度圓弧插補*/
void CAction::AnglCir(LONG lCenX, LONG lCenY, LONG lR, DOUBLE dStartAngl,DOUBLE dEndAngl, LONG lWorkVelociy, LONG lInitVelociy)
{
#ifdef MOVE
    //lR.圓直徑（mm）
    //dStartAngl.相對於圓所在處圓心的起點角度（單位：度）。 預設0度相當於時鐘3 : 00位置。 默認 = 0（度）值範圍：0 – 360
    //dEndAngl.位於角度起點值之後、點膠操作將在此停止的角度（單位：度）。 默認 = 0（度）要想按逆時針方向點膠，可輸入一個負值
    CPoint cptS, cptE;
    DOUBLE dRadStart = 0, dRadEnd = 0;
    LONG  lNowX = 0, lNowY = 0;
    BOOL bRev = 0;//0逆轉/1順轉
    lNowX = MO_ReadLogicPosition(0);
    lNowY = MO_ReadLogicPosition(1);
    dRadStart = dStartAngl * 180 / M_PI;
    dRadEnd = dEndAngl * 180 / M_PI;
    cptS.x = LONG(lR * cos(dRadStart) + lCenX);
    cptS.y = LONG(lR * sin(dRadStart) + lCenY);
    cptE.x = LONG(lR * cos(dRadEnd) + lCenX);
    cptE.y = LONG(lR* sin(dRadEnd) + lCenY);
    if(dEndAngl > 0)
    {
        bRev = 1;//順轉
    }
    if(!m_bIsStop)
    {
        MO_Do2DArcMove(cptS.x - lNowX, cptE.y - lNowY, lCenX - lNowX, lCenY - lNowY,
                       lInitVelociy, lWorkVelociy, bRev);//角度移動圓
        PreventMoveError();//防止軸卡出錯
    }
#endif
}
/*三軸圓弧插補*/
void CAction::Do3AxisCirle(LONG x1, LONG y1, LONG z1, LONG x2, LONG y2, LONG z2, LONG x3, LONG y3, LONG z3, LONG speed)
{
#ifdef MOVE
    CCircleFormula A;
    AxeSpace p1, p2, p3;
    p1 = { x1,y1,z1,0 };
    p2 = { x2,y2,z2,0 };
    p3 = { x3,y3,z3,0 };
    std::vector<AxeSpace> M;
    A.CircleCutPoint(p1, p2, p3, M);
    //A.ArcCutPoint(p1, p2, p3, M);


    size_t num = M.size();
    DATA_3MOVE *pDataM = new DATA_3MOVE[num];
    DATA_3MOVE *pDataShift = pDataM;
    for(size_t i = 0; i<M.size(); i++)
    {
        if(i == 0)
        {
            pDataShift->EndPX = M.at(i).x - MO_ReadLogicPosition(0);
            pDataShift->EndPY = M.at(i).y - MO_ReadLogicPosition(1);
            pDataShift->EndPZ = M.at(i).z - MO_ReadLogicPosition(2);
            pDataShift->Speed = pDataShift->Distance = 0;    //怪怪的!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        }
        else
        {
            pDataShift->EndPX = M.at(i).x - M.at(i - 1).x;
            pDataShift->EndPY = M.at(i).y - M.at(i - 1).y;
            pDataShift->EndPZ = M.at(i).z - M.at(i - 1).z;
            pDataShift->Speed = pDataShift->Distance = 0;    //怪怪的!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        }
        pDataShift++;
    }
    //_cwprintf(L"data count:%d\n", num);

    MO_Do3DLineMove(pDataM->EndPX, pDataM->EndPY, 0, speed, 100000, 2000);
    PreventMoveError();

    MO_Do3DLineMove(0, 0, pDataM->EndPZ, speed, 100000, 2000);
    PreventMoveError();
    pDataShift = pDataM + 1;
    //_cwprintf(L"共 %d 個點\n", num);

    /*for(int i = 0; i<num; i++)
    {
    _cwprintf(L"%d, %d, %d\n", pDataM[i].EndPX, pDataM[i].EndPY, pDataM[i].EndPZ);
    }*/

    MO_DO3Curve(pDataShift, num - 1, speed);

    PreventMoveError();//防止驅動錯誤
    pDataShift = NULL;
    delete[] pDataM;

    //LONG errorR2 = MO_ReadReg(2, 0);
    //_cwprintf(L"R2:%x\n", errorR2);

    /*LA_m_ptVec.clear();
    for(size_t i = 0; i<M.size(); i++)
    {
    DATA_3MOVE data3m;
    data3m.EndPX = M.at(i).x;
    data3m.EndPY = M.at(i).y;
    data3m.EndPZ = M.at(i).z;
    data3m.Speed = data3m.Distance = 0;
    LA_m_ptVec.push_back(data3m);
    }
    LA_m_iVecSP.push_back(1);
    LA_m_iVecSP.push_back(LA_m_ptVec.size());
    m_bIsGetLAend = TRUE;
    LA_Line3DtoDo(1, 200000, 500000, 3000);*/
#endif
}
/*======================================未使用連續插補的填充型態================================================================================================================*/
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
    if(dRadius == 0)
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
    while(1)
    {
        lDistance = LONG(lDistance - dWidth);
        if(lDistance < dWidth)
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
    for(ptIter = m_ptVec.begin(); ptIter != m_ptVec.end(); ptIter++)
    {
        if((iData != 0) && (cnt == m_ptVec.size() - 1))
        {
            MCO_Do2dDataLine((*ptIter).x, (*ptIter).y, DATA_2DO);
        }
        else if((iData == 0) && (cnt == m_ptVec.size() - 1))
        {
            MCO_Do2dDataCir((*ptIter).x, (*ptIter).y, cPtCen2.x, cPtCen2.y, bRev, DATA_2DO);
        }
        else
        {
            if(cnt % 2 == 0)
            {
                MCO_Do2dDataCir((*ptIter).x, (*ptIter).y, cPtCen2.x, cPtCen2.y, bRev, DATA_2DO);
            }
            else
            {
                MCO_Do2dDataCir((*ptIter).x, (*ptIter).y, lCenX, lCenY, bRev, DATA_2DO);
            }
        }
        cnt++;
    }
    LA_AbsToOppo2Move(DATA_2DO);

    if((iData != 0))
    {
        if(!m_bIsStop)
        {
            MO_Do3DLineMove(DATA_2DO.at(0).EndP.x, DATA_2DO.at(0).EndP.y, 0, lWorkVelociy, lAcceleration, lInitVelociy);//直線移動
            PreventMoveError();
        }
        PauseDoGlue();//暫停恢復後繼續出膠(m_bIsPause=0) 出膠
    }
    else
    {
        if(!m_bIsStop)
        {
            MO_Do2DArcMove(DATA_2DO.at(0).EndP.x, DATA_2DO.at(0).EndP.y, DATA_2DO.at(0).CirCentP.x, DATA_2DO.at(0).CirCentP.y,
                           lInitVelociy, lWorkVelociy, DATA_2DO.at(0).CirRev);//初始半圓
            PreventMoveError();
        }
        PauseDoGlue();//暫停恢復後繼續出膠(m_bIsPause=0) 出膠
    }

    for(UINT i = 1; i < DATA_2DO.size(); i++)
    {
        DATA_2Do[i - 1] = DATA_2DO.at(i);
    }
    MO_DO2Curve(DATA_2Do, DATA_2DO.size() - 1, lWorkVelociy);
    PreventMoveError();//防止驅動錯誤
    Sleep(200);
    DATA_2DO.clear();

    PauseStopGlue();//暫停時停指塗膠(m_bIsPause=1)
    MO_StopGumming();//停止出膠
    if(!m_bIsStop)
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
    if(dRadius == 0)
    {
        return;
    }
    dAngCenCos = acos(DOUBLE(cPt1.x - cPtCen.x) / dRadius);
    dAngCenSin = asin(DOUBLE(cPt1.y - cPtCen.y) / dRadius);
    dAngCenCos2 = M_PI * 2 - dAngCenCos;
    dAngCenSin2 = M_PI - dAngCenSin;
    if(abs(dAngCenCos - dAngCenSin) > 0.01)
    {
        if(abs(dAngCenCos - dAngCenSin2) < 0.01)
        {
            dAngCenSin = dAngCenSin2;
        }
        else if(abs(dAngCenCos2 - dAngCenSin) < 0.01)
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
    if(dAngCos < 0)
    {
        dAngCos += M_PI * 2;
    }
    if(dAngSin < 0)
    {
        dAngSin += M_PI * 2;
    }
    dAngCos2 = M_PI * 2 - dAngCos;
    dAngSin2 = M_PI - dAngSin;
    if(abs(dAngCos - dAngSin) > 0.01)
    {
        if(abs(dAngCos - dAngSin2) < 0.01)
        {
            dAngSin = dAngSin2;
        }
        else if(abs(dAngCos2 - dAngSin) < 0.01)
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
    while(1)
    {
        dRadius = dRadius - dWidth*sqrt(2);
        dDistance = dDistance - dWidth;
        if(dDistance < dWidth)
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
        if(dDistance < dWidth)
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
    PauseDoGlue();//暫停恢復後繼續出膠(m_bIsPause=0)出膠
    for(ptIter = m_ptVec.begin(); ptIter != m_ptVec.end(); ptIter++)
    {
        lNowX = MO_ReadLogicPosition(0);
        lNowY = MO_ReadLogicPosition(1);
        if(!m_bIsStop)
        {
            MO_Do3DLineMove((*ptIter).x - lNowX, (*ptIter).y - lNowY, 0, lWorkVelociy, lAcceleration, lInitVelociy);//移動
            PreventMoveError();
        }
    }
    PauseStopGlue();//暫停時停指塗膠(m_bIsPause=1)
    MO_StopGumming();//停止出膠
    if(!m_bIsStop)
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
    if(dRadius == 0)
    {
        return;
    }
    dAngCenCos = acos(DOUBLE(cPt1.x - cPtCen.x) / dRadius);
    dAngCenSin = asin(DOUBLE(cPt1.y - cPtCen.y) / dRadius);
    dAngCenCos2 = M_PI * 2 - dAngCenCos;
    dAngCenSin2 = M_PI - dAngCenSin;
    if(abs(dAngCenCos - dAngCenSin) > 0.01)
    {
        if(abs(dAngCenCos - dAngCenSin2) < 0.01)
        {
            dAngCenSin = dAngCenSin2;
        }
        else if(abs(dAngCenCos2 - dAngCenSin) < 0.01)
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
    if(dAngCos < 0)
    {
        dAngCos += M_PI * 2;
    }
    if(dAngSin < 0)
    {
        dAngSin += M_PI * 2;
    }
    dAngCos2 = M_PI * 2 - dAngCos;
    dAngSin2 = M_PI - dAngSin;
    if(abs(dAngCos - dAngSin) > 0.01)
    {
        if(abs(dAngCos - dAngSin2) < 0.01)
        {
            dAngSin = dAngSin2;
        }
        else if(abs(dAngCos2 - dAngSin) < 0.01)
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
    while(1)
    {
        if((iBuff == 1) && (dRadius - (2 * dWidth2) < 0))
        {
#ifdef PRINTF
        //    _cwprintf(_T("兩端寬度過大 \n"));
#endif
            m_ptVec.push_back(cPt1);
            break;
        }
        dRadius = dRadius - dWidth*sqrt(2);
        dDistance = dDistance - dWidth;
        if(iBuff*dWidth >  dWidth2)  //dWidth2兩端寬度
        {
            dDistance += dWidth;
            cPt1.x = LONG(dDistance*cos(dAngCos) + cPt4.x);
            cPt1.y = LONG(dDistance*sin(dAngSin) + cPt4.y);
            m_ptVec.push_back(cPt1);
            break;
        }
        if(dDistance < dWidth)
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
        if(dDistance < dWidth)
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
    PauseDoGlue();//暫停恢復後繼續出膠(m_bIsPause=0)出膠
    for(ptIter = m_ptVec.begin(); ptIter != m_ptVec.end(); ptIter++)
    {
        lNowX = MO_ReadLogicPosition(0);
        lNowY = MO_ReadLogicPosition(1);
        if(!m_bIsStop)
        {
            MO_Do3DLineMove((*ptIter).x - lNowX, (*ptIter).y - lNowY, 0, lWorkVelociy, lAcceleration, lInitVelociy);//移動
            PreventMoveError();
        }
    }
    PauseStopGlue();//暫停時停指塗膠(m_bIsPause=1)
    MO_StopGumming();//停止出膠
    if(!m_bIsStop)
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
    if(dRadius == 0)
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
    while(1)
    {
        if((iBuff == 1) && (lDistance - (2 * dWidth2) < 0))
        {
#ifdef PRINTF
      //      _cwprintf(_T("兩端寬度過大 \n"));
#endif
            break;
        }
        dRadius = dRadius - dWidth;
        lDistance = LONG(lDistance - dWidth);
        if(iBuff * dWidth > dWidth2)
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
    for(UINT i = 1; i < m_ptVec.size(); i++)
    {
        if(i == m_ptVec.size() - 1)
        {
            if(i % 2 == 0)
            {
                MCO_Do2dDataCir(m_ptVec.at(i).x, m_ptVec.at(i).y, cPtCen1.x, cPtCen1.y, bRev, DATA_2DO);//上半圓
            }
            else
            {
                MCO_Do2dDataCir(m_ptVec.at(i).x, m_ptVec.at(i).y, cPtCen2.x, cPtCen2.y, bRev, DATA_2DO);//下半圓
            }
        }
        else
        {
            if(i % 2 != 0)
            {
                MCO_Do2dDataCir(m_ptVec.at(i).x, m_ptVec.at(i).y, cPtCen1.x, cPtCen1.y, bRev, DATA_2DO);//上半圓
            }
            else
            {
                MCO_Do2dDataCir(m_ptVec.at(i).x, m_ptVec.at(i).y, cPtCen2.x, cPtCen2.y, bRev, DATA_2DO);//下半圓
            }
        }
    }
    LA_AbsToOppo2Move(DATA_2DO);
    PauseDoGlue();//暫停恢復後繼續出膠(m_bIsPause=0) 出膠
    for(UINT i = 0; i < DATA_2DO.size(); i++)
    {
        DATA_2Do[i] = DATA_2DO.at(i);
    }
    MO_DO2Curve(DATA_2Do, DATA_2DO.size(), lWorkVelociy);
    PreventMoveError();//防止驅動錯誤
    Sleep(200);
    DATA_2DO.clear();

    PauseStopGlue();//暫停時停指塗膠(m_bIsPause=1)
    MO_StopGumming();//停止出膠
    if(!m_bIsStop)
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
    if(dRadius == 0)
    {
        return;
    }
    dAngCenCos = acos(DOUBLE(cPt1.x - cPtCen.x) / dRadius);
    dAngCenSin = asin(DOUBLE(cPt1.y - cPtCen.y) / dRadius);
    dAngCenCos2 = M_PI * 2 - dAngCenCos;
    dAngCenSin2 = M_PI - dAngCenSin;
    if(abs(dAngCenCos - dAngCenSin) > 0.01)
    {
        if(abs(dAngCenCos - dAngCenSin2) < 0.01)
        {
            dAngCenSin = dAngCenSin2;
        }
        else if(abs(dAngCenCos2 - dAngCenSin) < 0.01)
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
    if(dAngCos < 0)
    {
        dAngCos += M_PI * 2;
    }
    if(dAngSin < 0)
    {
        dAngSin += M_PI * 2;
    }
    dAngCos2 = M_PI * 2 - dAngCos;
    dAngSin2 = M_PI - dAngSin;
    if(abs(dAngCos - dAngSin) > 0.01)
    {
        if(abs(dAngCos - dAngSin2) < 0.01)
        {
            dAngSin = dAngSin2;
        }
        else if(abs(dAngCos2 - dAngSin) < 0.01)
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
    while(1)
    {
        dRadius = dRadius - dWidth*sqrt(2);
        dDistance = dDistance - dWidth;
        if(dDistance < dWidth)
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
        if(dDistance < dWidth)
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
    if(!m_bIsStop)
    {
        MO_Do3DLineMove(0, 0, (lZ - lZBackDistance) - lZ, lWorkVelociy, lAcceleration, lInitVelociy);//Z軸返回
        PreventMoveError();//防止軸卡出錯
    }
    for(rptIter = m_ptVec.rbegin(); rptIter != m_ptVec.rend(); rptIter++)
    {
        lNowX = MO_ReadLogicPosition(0);
        lNowY = MO_ReadLogicPosition(1);
        if(!m_bIsStop)
        {
            MO_Do3DLineMove((*rptIter).x - lNowX, (*rptIter).y - lNowY, 0, lWorkVelociy, lAcceleration, lInitVelociy);//移動
            PreventMoveError();
        }
        if(rptIter == m_ptVec.rbegin())
        {
            if(!m_bIsStop)
            {
                lNowZ = MO_ReadLogicPosition(2);
                MO_Do3DLineMove(0, 0, lZ - lNowZ, lWorkVelociy, lAcceleration, lInitVelociy);//Z軸往下
                PreventMoveError();//防止軸卡出錯
            }
            PauseDoGlue();//暫停恢復後繼續出膠(m_bIsPause=0) 出膠
        }
    }
    PauseStopGlue();//暫停時停指塗膠(m_bIsPause=1)
    MO_StopGumming();//停止出膠
    if(!m_bIsStop)
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
    if(dRadius == 0)
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
    while(1)
    {
        lDistance = LONG(lDistance - dWidth);
        if(lDistance < dWidth)
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
    if(!m_bIsStop)
    {
        MO_Do3DLineMove(lCenX - MO_ReadLogicPosition(0), lCenY - MO_ReadLogicPosition(1), 0, lWorkVelociy, lAcceleration,
                        lInitVelociy);//直線移動至圓心
        PreventMoveError();
    }
    if(!m_bIsStop)
    {
        MO_Do3DLineMove(0, 0, lZ - MO_ReadLogicPosition(2), lWorkVelociy, lAcceleration,
                        lInitVelociy);//Z軸往下
        PreventMoveError();//防止軸卡出錯
    }

    std::vector<DATA_2MOVE> DATA_2DO;
    DATA_2DO.clear();
    for(rptIter = m_ptVec.rbegin(); rptIter != m_ptVec.rend(); rptIter++)
    {
        if((iData != 0) && (iOdd == 0))
        {
            MCO_Do2dDataLine((*rptIter).x, (*rptIter).y, DATA_2DO);
        }
        else if((iData == 0) && (iOdd == 0))
        {
            MCO_Do2dDataCir((*rptIter).x, (*rptIter).y, cPtCen2.x, cPtCen2.y, bRev, DATA_2DO);
        }
        else
        {
            if(iOdd % 2 == 0)
            {
                MCO_Do2dDataCir((*rptIter).x, (*rptIter).y, cPtCen2.x, cPtCen2.y, bRev, DATA_2DO);
            }
            else
            {
                MCO_Do2dDataCir((*rptIter).x, (*rptIter).y, lCenX, lCenY, bRev, DATA_2DO);
            }
        }
        iOdd++;
    }
    LA_AbsToOppo2Move(DATA_2DO);
    PauseDoGlue();//暫停恢復後繼續出膠(m_bIsPause=0) 出膠
    for(UINT i = 0; i < DATA_2DO.size(); i++)
    {
        DATA_2Do[i] = DATA_2DO.at(i);
    }
    MO_DO2Curve(DATA_2Do, DATA_2DO.size(), lWorkVelociy);
    PreventMoveError();//防止驅動錯誤
    Sleep(200);
    DATA_2DO.clear();

    PauseStopGlue();//暫停時停指塗膠(m_bIsPause=1)
    MO_StopGumming();//停止出膠
    if(!m_bIsStop)
    {
        MO_Do3DLineMove(0, 0, (lZ - lZBackDistance) - lZ, lWorkVelociy, lAcceleration,
                        lInitVelociy);//Z軸返回
        PreventMoveError();//防止軸卡出錯
    }
#endif
}