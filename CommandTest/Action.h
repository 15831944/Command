#pragma once
class CAction
{
public:     //變數
    BOOL    g_bIsStop;
public:     //析構函數
	CAction();
	virtual ~CAction();
public:     //運動API
    void WaitTime(HANDLE wakeEvent, int Time);
    void DecidePointGlue(LONG lX, LONG lY, LONG lZ, LONG lDoTime, LONG lDelayStopTime,
        LONG lZBackDistance, LONG lZdistance, LONG lHighVelocity, LONG lLowVelocity, LONG lWorkVelociy,
        LONG lAcceleration, LONG lInitVelociy);//單點點膠
private:    //自行運用函數
    void PreventMoveError();//防止軸卡出錯
    void PreventGlueError();//防止出膠出錯
    void DoGlue(LONG lTime, LONG lDelayTime, LPTHREAD_START_ROUTINE GummingTimeOutThread);//出膠，多載有延遲時間(配合執行緒使用)
protected:  //執行緒
    static DWORD WINAPI GummingTimeOutThread(LPVOID);
};


