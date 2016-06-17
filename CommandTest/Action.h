/*
*檔案名稱:Action.h(3D用)
*內容簡述:運動命令API，詳細參數請查看excel
*＠author 作者名稱:R
*＠data 更新日期:2016/06/07
*@更新內容線段z值改變時，三軸同動移動，原來的為x,y先移動再移動z軸*/
#pragma once
#include <vector>
#define _USE_MATH_DEFINES
#include <math.h>
static LPVOID pAction;
class CAction
{
public:     //變數
    BOOL    g_bIsPause;       //暫停用
	BOOL    g_bIsStop;
	BOOL    g_bIsDispend;
	UINT    g_iNumberGluePort;
public:     //析構函數
	CAction();
	virtual ~CAction();
public:     //運動API
	void WaitTime(HANDLE wakeEvent, int Time);
	//單點點膠動作--(單點點膠X,單點點膠Y,單點點膠Z,出膠時間,斷膠延遲,Z軸回升高度(相對)最高點,Z軸距離(相對),高速度,低速度,驅動速度,加速度,初速度)
	void DecidePointGlue(LONG lX, LONG lY, LONG lZ, LONG lDoTime, LONG lDelayStopTime,LONG lZBackDistance,BOOL bZDisType, LONG lZdistance, LONG lHighVelocity, LONG lLowVelocity, LONG lWorkVelociy,LONG lAcceleration, LONG lInitVelociy);
	//線段開始動作--(線段開始X,線段開始Y,線段開始Z,移動前延遲,驅動速度,加速度,初速度)
	void DecideLineStartMove(LONG lX, LONG lY, LONG lZ, LONG lStartDelayTime, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
	//線段中點動作--(線段中點X,線段中點Y,線段中點Z,對線段中點的停留時間(節點時間),驅動速度,加速度,初速度)
	void DecideLineMidMove(LONG lX, LONG lY, LONG lZ, LONG lMidDelayTime, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
	//線段結束動作--(線段結束X,線段結束Y,線段結束Z,關機後在結束點停留時間(停留時間),距離結束點多遠距離關機(關機距離),關機後的延遲時間(關機延遲),Z軸回升距離,Z軸回升的型態,線段結束設定(高速度),返回長度,z返回高度,返回速度(低速),返回類型,驅動速度,加速度,初速度)
	void DecideLineEndMove(LONG lX, LONG lY, LONG lZ, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime, LONG lZBackDistance,BOOL bZDisType, LONG lHighVelocity, LONG lDistance, LONG lHigh, LONG lLowVelocity, int iType, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
	//(現在)線段結束動作多載--(關機後在結束點停留時間(停留時間),關機後的延遲時間(關機延遲),Z軸回升距離,Z軸回升的型態,線段結束設定(高速度),返回長度,z返回高度,返回速度(低速),驅動速度,加速度,初速度)
	void DecideLineEndMove(LONG lCloseOffDelayTime, LONG lCloseONDelayTime, LONG lZBackDistance,BOOL bZDisType, LONG lHighVelocity, LONG lDistance, LONG lHigh, LONG lLowVelocity, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
    //線段開始到中間點動作--(線段開始X,Y,Z,線段中點X,Y,Z,移動前延遲,開機前從起點移動距離(設置距離),對線段中點的停留時間(節點時間ms),驅動速度,加速度,初速度)
	void DecideLineSToP(LONG lX, LONG lY, LONG lZ, LONG lX2, LONG lY2, LONG lZ2, LONG lStartDelayTime, LONG lStartDistance, LONG lMidDelayTime, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
    //線段開始到結束動作--(線段開始X,Y,Z,結束點X,Y,Z,移動前延遲,開機前從起點移動距離(設置距離),關機後在結束點停留時間(停留時間),距離結束點多遠距離關機(關機距離),關機後的延遲時間(關機延遲),Z軸回升距離,Z軸回升型態,點膠結束設定(高速度),返回長度,z返回高度,返回速度(低速),返回類型,驅動速度,加速度,初速度)
	void DecideLineSToE(LONG lX, LONG lY, LONG lZ, LONG lX2, LONG lY2, LONG lZ2, LONG lStartDelayTime, LONG lStartDistance, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime, LONG lZBackDistance, BOOL bZDisType, LONG lHighVelocity, LONG lDistance, LONG lHigh, LONG lLowVelocity, int iType, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
	//圓形動作--(圓點座標X,Y,圓點座標X2,Y2,驅動速度,初速度)
	void DecideCircle(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lWorkVelociy, LONG lInitVelociy);
	//圓弧動作--(圓弧座標X,Y,圓弧座標X2,Y2,驅動速度,初速度)
	void DecideArc(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lWorkVelociy, LONG lInitVelociy);
    //線段結束動作--(圓形動作--(圓形座標X1,Y1,圓形座標X2,Y2,線段結束X3,Y3,關機後在結束點停留時間(停留時間),距離結束點多遠距離關機(關機距離),關機後的延遲時間(關機延遲),Z軸回升相對距離,Z軸型態(0絕對位置/1相對位置),點膠結束設定(高速度),返回長度,z返回高度,返回速度(低速),返回類型,驅動速度,加速度,初速度)
    void DecideCircleToEnd(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lX3, LONG lY3, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime, LONG lZBackDistance, BOOL bZDisType, LONG lHighVelocity, LONG lDistance, LONG lHigh, LONG lLowVelocity, int iType, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
    //線段結束動作--(圓弧動作--(圓弧座標X1,Y1,線段結束X2,Y2,關機後在結束點停留時間(停留時間),距離結束點多遠距離關機(關機距離),關機後的延遲時間(關機延遲),Z軸回升相對距離,Z軸型態(0絕對位置/1相對位置),點膠結束設定(高速度),返回長度,z返回高度,返回速度(低速),返回類型,驅動速度,加速度,初速度)
    void DecideArcleToEnd(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime, LONG lZBackDistance, BOOL bZDisType, LONG lHighVelocity, LONG lDistance, LONG lHigh, LONG lLowVelocity, int iType, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
    //虛擬點動作--(虛擬點座標X,Y,Z,驅動速度,加速度,初速度)
	void DecideVirtualPoint(LONG lX, LONG lY, LONG lZ, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
	//等待點動作--(等待點座標X,Y,Z,等待時間,驅動速度,加速度,初速度)
	void DecideWaitPoint(LONG lX, LONG lY, LONG lZ, LONG lWaitTime, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
	//停駐點動作--(停駐點座標X,Y,Z,排膠時間,結束後等待時間,驅動速度,加速度,初速度)
	void DecideParkPoint(LONG lX, LONG lY, LONG lZ, LONG lTimeGlue, LONG lWaitTime, LONG lStayTime, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
	//原點賦歸動作--(原點復歸速度1,原點復歸速度2,復歸軸(7),偏移量(0))
	void DecideInitializationMachine(LONG lSpeed1, LONG lSpeed2, LONG lAxis, LONG lMoveX, LONG lMoveY, LONG lMoveZ);
    //填充動作(線段開始X,Y,Z，線段結束X,Y,Z，Z軸上升距離，Z軸型態(0絕對位置/1相對位置)，填充形式(1~7)，寬度(mm)，兩端寬度(mm)，驅動速度，加速度，初速度)
    void DecideFill(LONG lX1, LONG lY1, LONG lZ1, LONG lX2, LONG lY2, LONG lZ2, LONG lZBackDistance, BOOL bZDisType, int iType, LONG lWidth, LONG lWidth2, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
	//輸出-16個輸出(選擇埠(0~15),開啟關閉(0~1))
	BOOL DecideOutPutSign(int iPort, BOOL bChoose);
	//輸入-12個輸入(選擇埠(0~11),開啟關閉(0~1))
	BOOL DecideInPutSign(int iPort, BOOL bChoose);
    //位置偏移(輸入一個(X,Y,Z)座標)
    CString NowOffSet(LONG lX, LONG lY, LONG lZ);
    //回傳目前位置
    CString NowLocation();
    //CCD移動用--(點座標X,Y,Z,驅動速度,加速度,初速度)
    void DoCCDMove(LONG lX, LONG lY, LONG lZ, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
    //人機用函數-軟體負極限
    void HMNegLim(LONG lX, LONG lY, LONG lZ);
    //人機用函數-軟體正極限(x,y,z為最大工作範圍)
    void HMPosLim(LONG lX, LONG lY, LONG lZ);
private:    //自行運用函數
	void AttachPointMove(LONG lX, LONG lY, LONG lZ, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy, BOOL bIntt); //附屬--- 移動點動作
	void DoGlue(LONG lTime, LONG lDelayTime, LPTHREAD_START_ROUTINE GummingTimeOutThread);//出膠，多載有延遲時間(配合執行緒使用)
	void PreventMoveError();//防止軸卡出錯
	void PreventGlueError();//防止出膠出錯
    void PauseDoGlue();//暫停回復後重新塗膠(讀取暫停參數，當參數為0時出膠，且點膠機要為開。)
    void PauseStopGlue();//暫停時停指塗膠(讀取暫停參數，當參數為1時斷膠，或點膠機關參數為0也斷膠。)
	void GelatinizeBack(int iType, LONG lXarEnd, LONG lYarEnd, LONG lZarEnd, LONG lXarUp, LONG lYarUp, LONG lLineStop, LONG lStopZar, LONG lBackZar, LONG lLowSpeed, LONG lHighSpeed, LONG lAcceleration, LONG lInitSpeed);//返回設定
	void LineGetToPoint(LONG &lXClose, LONG &lYClose, LONG lX0, LONG lY0, LONG lX1, LONG lY1, LONG &lLineClose);//直線距離轉換成座標點
	void LineGetToPoint(LONG &lXClose, LONG &lYClose, LONG &lZClose, LONG lX0, LONG lY0, LONG lX1, LONG lY1, LONG lZ0, LONG lZ1, LONG &lLineClose);//直線距離轉換成座標點--多載3D
	CString TRoundCCalculation(CString Origin, CString End, CString Between);//三點計算圓心
	LONG CStringToLong(CString csData, int iChoose);//字串轉長整數，使用的結尾一定要是"，"
    DOUBLE AngleCount(DOUBLE LocatX, DOUBLE LocatY, DOUBLE LocatX1, DOUBLE LocatY1, DOUBLE LocatX2, DOUBLE LocatY2, BOOL bRev);//三點取得角度(向量夾角)(圓心x,y座標x1,y1,座標x2,y2,正逆轉)
    void ArcGetToPoint(LONG &lArcX, LONG &lArcY, LONG lDistance, LONG lX, LONG lY, LONG lCenX, LONG lCenY, LONG lRadius, BOOL bDir);//圓弧長距離轉點做標
    void AttachFillType1(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lZ, LONG lZBackDistance, LONG lWidth, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);//附屬--- 填充形態1
    void AttachFillType2(LONG lX1, LONG lY1, LONG lCenX, LONG lCenY, LONG lZ, LONG lZBackDistance, LONG lWidth, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);//附屬--- 填充形態2
    void AttachFillType3(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lZ, LONG lZBackDistance, LONG lWidth, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);//附屬--- 填充形態3
    void AttachFillType4(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lZ, LONG lZBackDistance, LONG lWidth, LONG lWidth2, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);//附屬--- 填充形態4
    void AttachFillType5(LONG lX1, LONG lY1, LONG lCenX, LONG lCenY, LONG lZ, LONG lZBackDistance, LONG lWidth, LONG lWidth2, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);//附屬--- 填充形態5
    void AttachFillType6(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lZ, LONG lZBackDistance, LONG lWidth, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);//附屬--- 填充形態6
    void AttachFillType7(LONG lX1, LONG lY1, LONG lCenX, LONG lCenY, LONG lZ, LONG lZBackDistance, LONG lWidth, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);//附屬--- 填充形態7
protected:  //執行緒
	static DWORD WINAPI GummingTimeOutThread(LPVOID);
    static DWORD WINAPI LPInterrupt(LPVOID);//中斷
};


