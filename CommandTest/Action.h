#pragma once
class CAction
{
public:     //變數
    BOOL    g_bIsStop;
    BOOL    g_bIsDispend;
    UINT    g_iNumberGluePort;
public:     //析構函數
	CAction();
	virtual ~CAction();
public:     //運動API
    void WaitTime(HANDLE wakeEvent, int Time);
    //單點點膠動作--(單點點膠X,單點點膠Y,單點點膠Z,出膠時間,斷膠延遲,Z軸回升高度(相對)最高點,Z軸距離(相對),高速度,低速度,驅動速度,加速度,初速度)
    void DecidePointGlue(LONG lX, LONG lY, LONG lZ, LONG lDoTime, LONG lDelayStopTime,LONG lZBackDistance, LONG lZdistance, LONG lHighVelocity, LONG lLowVelocity, LONG lWorkVelociy,LONG lAcceleration, LONG lInitVelociy);
    //線段開始動作--(線段開始X,線段開始Y,線段開始Z,移動前延遲,開機前從起點移動距離(設置距離),驅動速度,加速度,初速度)
    void DecideLineStartMove(LONG lX, LONG lY, LONG lZ, LONG lStartDelayTime, LONG lStartDistance, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
    //線段中點動作--(線段中點X,線段中點Y,線段中點Z,對線段中點的停留時間(節點時間),驅動速度,加速度,初速度)
    void DecideLineMidMove(LONG lX, LONG lY, LONG lZ, LONG lMidDelayTime, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
    //線段結束動作--(線段結束X,線段結束Y,線段結束Z,關機後在結束點停留時間(停留時間),距離結束點多遠距離關機(關機距離),關機後的延遲時間(關機延遲),Z軸回升相對距離,點膠結束設定(高速度),返回長度,z返回高度,返回速度(低速),返回類型,驅動速度,加速度,初速度)
    void DecideLineEndMove(LONG lX, LONG lY, LONG lZ, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime, LONG lZBack, LONG lHighVelocity, LONG lDistance, LONG lHigh, LONG lLowVelocity, int iType, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
    //圓形動作--(圓點座標X,Y,圓點座標X2,Y2,驅動速度,初速度)
    void DecideCircle(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lWorkVelociy, LONG lInitVelociy);
    //圓弧動作--(圓弧座標X,Y,圓弧座標X2,Y2,驅動速度,初速度)
    void DecideArc(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lWorkVelociy, LONG lInitVelociy);
    //虛擬點動作--(虛擬點座標X,Y,Z,驅動速度,加速度,初速度)
    void DecideVirtualPoint(LONG lX, LONG lY, LONG lZ, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
    //等待點動作--(等待點座標X,Y,Z,等待時間,驅動速度,加速度,初速度)
    void DecideWaitPoint(LONG lX, LONG lY, LONG lZ, LONG lWaitTime, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
    //停駐點動作--(停駐點座標X,Y,Z,排膠時間,結束後等待時間,驅動速度,加速度,初速度)
    void DecideParkPoint(LONG lX, LONG lY, LONG lZ, LONG lTimeGlue, LONG lWaitTime, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
    //原點賦歸動作--(原點復歸速度1,原點復歸速度2,復歸軸(7),偏移量(0))
    void DecideInitializationMachine(LONG lSpeed1, LONG lSpeed2, LONG lAxis, LONG lMove);
    //輸出-16個輸出(選擇埠(0~15),開啟關閉(0~1))
    BOOL DecideOutPutSign(int iPort, BOOL bChoose);
    //輸入-12個輸入(選擇埠(0~11),開啟關閉(0~1))
    BOOL DecideInPutSign(int iPort, BOOL bChoose);
private:    //自行運用函數
    void AttachPointMove(LONG lX, LONG lY, LONG lZ, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy); //附屬--- 移動點動作
    void DoGlue(LONG lTime, LONG lDelayTime, LPTHREAD_START_ROUTINE GummingTimeOutThread);//出膠，多載有延遲時間(配合執行緒使用)
    void PreventMoveError();//防止軸卡出錯
    void PreventGlueError();//防止出膠出錯
    void GelatinizeBack(int iType, LONG lXarEnd, LONG lYarEnd, LONG lZarEnd, LONG lXarUp, LONG lYarUp, LONG lLineStop, LONG lStopZar, LONG lBackZar, LONG lLowSpeed, LONG lHighSpeed, LONG lAcceleration, LONG lInitSpeed);//返回設定
    void LineGetToPoint(LONG &lXClose, LONG &lYClose, LONG lX0, LONG lY0, LONG lX1, LONG lY1, LONG &lLineClose);//直線距離轉換成座標點
    CString TRoundCCalculation(CString Origin, CString End, CString Between);//三點計算圓心
    LONG CStringToLong(CString csData, int iChoose);//字串轉長整數，使用的結尾一定要是"，"
protected:  //執行緒
    static DWORD WINAPI GummingTimeOutThread(LPVOID);
};


