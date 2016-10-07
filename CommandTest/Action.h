/*
*檔案名稱:Action.h(3D用)
*內容簡述:運動命令API，詳細參數請查看excel
*＠author 作者名稱:R
*＠data 更新日期:2016/09/26
*@更新內容三軸兩軸連續差補&雷射API*/
#pragma once
#include <vector>
#define _USE_MATH_DEFINES
#include <math.h>
#ifdef MOVE
#include "mcc.h"
#endif
//static LPVOID  pAction;
static int     g_LaserErrCnt;//雷射時間切計數器(紀錄錯誤用)
static int     g_LaserNuCnt;//雷射時間切計數器(紀錄數量用)
class CAction
{
public:     //變數
    LPVOID  pAction;          //this指針
	BOOL    g_bIsPause;       //暫停用
	BOOL    g_bIsStop;        //停止用
	BOOL    g_bIsDispend;     //關閉點膠用
	BOOL    g_bIsGetLAend;     
	int     g_iNumberGluePort;//點膠埠啟用數量
	LONG    g_OffSetLaserX;//雷射用x軸偏移量
	LONG    g_OffSetLaserY;//雷射用y軸偏移量
	LONG    g_OffSetLaserZ;//雷射用z軸偏移量(B點的Z高度到雷射歸零Z高度的位移值(+))
	LONG    g_HeightLaserZero;//雷射用Z軸歸零完成後高度(掃描高度)
	LONG    g_laserBuff;//雷射用暫存值(test)
	LONG    g_OffSetScan;//雷射掃描補償值
	LONG    g_LaserAveBuffZ;//雷射用平均暫存值(絕對位置z值)
	int     g_LaserCnt;//雷射線段計數器(掃描用)
	BOOL    g_LaserAverage;//雷射平均(1使用/0不使用)
	BOOL    g_interruptLock;//中斷鎖
    BOOL    g_getHeightFlag;//雷射測高旗標：允許測高
	std::vector<UINT>  LA_m_iVecSP;//主要雷射vector(SP:Scan End)
	static BOOL    g_YtimeOutGlueSet;//Y計時器中斷時出斷膠控制
	static BOOL    g_ZtimeOutGlueSet;//Z計時器中斷時出斷膠控制
    HANDLE m_hComm;
    CString ComportNo = _T("COM6"); //RS232 COMPort

#ifdef MOVE
	std::vector<DATA_3MOVE> LA_m_ptVec;//雷射連續切點儲存vector
	std::vector<DATA_2MOVE> LA_m_ptVec2D;//雷射連續切點儲存vector
	DATA_3MOVE DATA_3Do[512];//連續切暫存
	DATA_2MOVE DATA_2Do[128]; 
#endif
#ifdef LA
	DATA_3MOVE DATA_3ZERO_B;//雷射歸零_針頭B點
	DATA_3MOVE DATA_3ZERO_LA;//雷射歸零_雷射B點
#endif
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
	void DecideLineEndMove(LONG lCloseOffDelayTime, LONG lCloseONDelayTime, LONG lZBackDistance,BOOL bZDisType, LONG lHighVelocity, LONG lDistance, LONG lHigh, LONG lLowVelocity, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy,BOOL bGeneral=0);
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
	//停駐點動作--(停駐點座標X,Y,Z,排膠時間,結束後等待時間,Z軸回升高度(相對)最高點,Z軸距離(相對),高速度,低速度,驅動速度,加速度,初速度)
	void DecideParkPoint(LONG lX, LONG lY, LONG lZ, LONG lTimeGlue, LONG lWaitTime, LONG lStayTime, LONG lZBackDistance, BOOL bZDisType, LONG lZdistance, LONG lHighVelocity, LONG lLowVelocity, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
	//原點賦歸動作--(原點復歸速度1,原點復歸速度2,復歸軸(7),偏移量(0))
	void DecideInitializationMachine(LONG lSpeed1, LONG lSpeed2, LONG lAxis, LONG lMoveX, LONG lMoveY, LONG lMoveZ);
	////填充動作(線段開始X,Y,Z，線段結束X,Y,Z，Z軸距離(相對)，Z軸型態(0絕對位置/1相對位置)，填充形式(1~7)，寬度(mm)，兩端寬度(mm)，線段點膠設定(1.移動前延遲，2.設置距離，3停留時間，5關機距離，6關機延遲)，驅動速度，加速度，初速度)
	void DecideFill(LONG lX1, LONG lY1, LONG lZ1, LONG lX2, LONG lY2, LONG lZ2, LONG lZBackDistance, BOOL bZDisType,int iType, LONG lWidth, LONG lWidth2, LONG lStartDelayTime, LONG lStartDistance, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
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
	//原點復歸，回絕對位置(0,0,0)
	void BackGOZero(LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
	//人機用函數-軟體負極限
	void HMNegLim(LONG lX, LONG lY, LONG lZ);
	//人機用函數-軟體正極限(x,y,z為最大工作範圍)
	void HMPosLim(LONG lX, LONG lY, LONG lZ);
public:     //雷射API
	//雷射歸零按鈕_針頭B點
	void LA_Butt_GoBPoint();
	//雷射歸零按鈕_雷射到針頭B點
	void LA_Butt_GoLAtoBPoint();
	//雷射設定初始化
	void LA_SetInit();
	//單點雷射取值(使用DATA_3MOVE結構)+偏移量
	BOOL LA_Dot3D(LONG lX, LONG lY, LONG &lZ, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
	//雷射規零(下降至69999做歸零)
	BOOL LA_SetZero();
	//2D連續差補資料輸入
	void LA_Do2DVetor(LONG lX3, LONG lY3, LONG lX2 = 0, LONG lY2 = 0, LONG lX1 = 0, LONG lY1 = 0);//!!注意點座標順序3.2.1    
	void LA_Do2dDataLine(LONG EndPX, LONG EndPY);
	void LA_Do2dDataArc(LONG EndPX, LONG EndPY, LONG ArcX, LONG ArcY);
	void LA_Do2dDataCircle(LONG EndPX, LONG EndPY, LONG CirP1X, LONG CirP1Y, LONG CirP2X, LONG CirP2Y);
	//2D連續差補掃描
	void LA_Line2D(LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);//單點雷射取值(使用DATA_2MOVE結構)+偏移量
	//三軸連續插補
	void LA_Line3DtoDo(int iData, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy, BOOL bDoAll = FALSE);
	//雷射清除指令
	void LA_Clear();
	//雷射平均回傳平均z值
	void LA_AverageZ(LONG lStrX, LONG lStrY, LONG lEndX, LONG lEndY, LONG &lZ);
	//將3Dvetor值作旋轉偏移修正
	void LA_CorrectVectorToDo(LONG  lWorkVelociy, LONG lAcceleration, LONG lInitVelociy, LONG RefX = 0, LONG RefY = 0, DOUBLE OffSetX = 0, DOUBLE OffSetY = 0, DOUBLE Andgle = 0, DOUBLE CameraToTipOffsetX = 0, DOUBLE CameraToTipOffsetY = 0, BOOL Mode = 0, LONG lSubOffsetX = 0, LONG lSubOffsetY = 0);//將3Dvetor值作旋轉偏移修正
	//填充動作_多載(最後一點位置(EndX,EndY),線段開始X,Y,Z，線段結束X,Y,Z，Z軸距離(相對)，Z軸型態(0絕對位置/1相對位置)，填充形式(1~7)，寬度(mm)，兩端寬度(mm))
	void Fill_EndPoint(LONG &lEndX, LONG &lEndY, LONG lX1, LONG lY1, LONG lZ1, LONG lX2, LONG lY2, LONG lZ2, int iType, LONG lWidth, LONG lWidth2);
public:     //執行續
	static DWORD WINAPI MoInterrupt(LPVOID);//軸卡中斷thread
private:    //自行運用函數
	void AttachPointMove(LONG lX, LONG lY, LONG lZ, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy, BOOL bIntt); //附屬--- 移動點動作
	void DoGlue(LONG lTime, LONG lDelayTime, LPTHREAD_START_ROUTINE MoInterrupt =0);//出膠，多載有延遲時間(配合執行緒使用)
	void PreventMoveError();//防止軸卡出錯
	void PreventGlueError();//防止出膠出錯
	void PauseDoGlue();//暫停回復後重新塗膠(讀取暫停參數，當參數為0時出膠，且點膠機要為開。)
	void PauseStopGlue();//暫停時停指塗膠(讀取暫停參數，當參數為1時斷膠，或點膠機關參數為0也斷膠。)
	void GelatinizeBack(int iType, LONG lXarEnd, LONG lYarEnd, LONG lZarEnd, LONG lXarUp, LONG lYarUp, LONG lLineStop, LONG lStopZar, LONG lBackZar, LONG lLowSpeed, LONG lHighSpeed, LONG lAcceleration, LONG lInitSpeed);//返回設定
	void LineGetToPoint(LONG &lXClose, LONG &lYClose, LONG lX0, LONG lY0, LONG lX1, LONG lY1, LONG &lLineClose);//直線距離轉換成座標點
	void LineGetToPoint(LONG &lXClose, LONG &lYClose, LONG &lZClose, LONG lX0, LONG lY0, LONG lX1, LONG lY1, LONG lZ0, LONG lZ1, LONG &lLineClose);//直線距離轉換成座標點--多載3D
	LONG CalPreglue(LONG lStartDistance, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);//計算提前出斷膠距離或時間(us)
	LONG CalPreglue(LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);//計算提前出斷膠距離
	LONG CalPreglueTime(LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);//計算提前出斷膠時間(us)
	CString TRoundCCalculation(CString Origin, CString End, CString Between);//三點計算圓心
	LONG CStringToLong(CString csData, int iChoose);//字串轉長整數，使用的結尾一定要是"，"
	DOUBLE AngleCount(DOUBLE LocatX, DOUBLE LocatY, DOUBLE LocatX1, DOUBLE LocatY1, DOUBLE LocatX2, DOUBLE LocatY2, BOOL bRev);//三點取得角度(向量夾角)(圓心x,y座標x1,y1,座標x2,y2,正逆轉)
	void ArcGetToPoint(LONG &lArcX, LONG &lArcY, LONG lDistance, LONG lX, LONG lY, LONG lCenX, LONG lCenY, LONG lRadius, BOOL bDir);//圓弧長距離轉點做標
	void AttachFillType1(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lZ, LONG lZBackDistance, LONG lWidth, LONG lStartDelayTime, LONG lStartDistance, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime,LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);//附屬--- 填充形態1-矩形S
	void AttachFillType2(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lZ, LONG lZBackDistance, LONG lWidth, LONG lStartDelayTime, LONG lStartDistance, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);//附屬--- 填充形態2-螺旋圓
	void AttachFillType3(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lZ, LONG lZBackDistance, LONG lWidth, LONG lStartDelayTime, LONG lStartDistance, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);//附屬--- 填充形態3-矩形螺旋
	void AttachFillType4(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lZ, LONG lZBackDistance, LONG lWidth, LONG lWidth2, LONG lStartDelayTime, LONG lStartDistance, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);//附屬--- 填充形態4-矩形環
	void AttachFillType5(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lZ, LONG lZBackDistance, LONG lWidth, LONG lWidth2, LONG lStartDelayTime, LONG lStartDistance, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);//附屬--- 填充形態5-圓環
	void AttachFillType6(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lZ, LONG lZBackDistance, LONG lWidth, LONG lStartDelayTime, LONG lStartDistance, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);//附屬--- 填充形態6-矩形反螺旋
	void AttachFillType7(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lZ, LONG lZBackDistance, LONG lWidth, LONG lStartDelayTime, LONG lStartDistance, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);//附屬--- 填充形態7-反螺旋圓                                                                                                                                                                      //絕對座標轉相對座標3軸連續插補使用
	void LA_CorrectLocation(LONG &PointX, LONG &PointY, LONG RefX, LONG RefY, DOUBLE OffSetX, DOUBLE OffSetY, DOUBLE Andgle, DOUBLE CameraToTipOffsetX, DOUBLE CameraToTipOffsetY, BOOL Mode, LONG lSubOffsetX, LONG lSubOffsetY);//雷射用旋轉平移
	//===============未使用連續插補的填充型態=========================
	void AttachFillType2_1(LONG lX1, LONG lY1, LONG lCenX, LONG lCenY, LONG lZ,
		LONG lZBackDistance, LONG lWidth, LONG lWorkVelociy, LONG lAcceleration,
		LONG lInitVelociy);//附屬--- 填充形態2-1
	void AttachFillType3_1(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lZ,
		LONG lZBackDistance, LONG lWidth, LONG lWorkVelociy, LONG lAcceleration,
		LONG lInitVelociy);//附屬--- 填充形態3
	void AttachFillType4_1(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lZ,
		LONG lZBackDistance, LONG lWidth, LONG lWidth2, LONG lWorkVelociy,
		LONG lAcceleration, LONG lInitVelociy);//附屬--- 填充形態4
	void AttachFillType5_1(LONG lX1, LONG lY1, LONG lCenX, LONG lCenY, LONG lZ,
		LONG lZBackDistance, LONG lWidth, LONG lWidth2, LONG lWorkVelociy,
		LONG lAcceleration, LONG lInitVelociy);//附屬--- 填充形態5
	void AttachFillType6_1(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lZ,
		LONG lZBackDistance, LONG lWidth, LONG lWorkVelociy, LONG lAcceleration,
		LONG lInitVelociy);//附屬--- 填充形態6
	void AttachFillType7_1(LONG lX1, LONG lY1, LONG lCenX, LONG lCenY, LONG lZ,
		LONG lZBackDistance, LONG lWidth, LONG lWorkVelociy, LONG lAcceleration,
		LONG lInitVelociy);//附屬--- 填充形態7
	void AttachFillType3_End(LONG &EndX, LONG &EndY, LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lWidth, LONG lWidth2);
	void AttachFillType4_End(LONG &EndX, LONG &EndY, LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lWidth, LONG lWidth2);
	void AttachFillType5_End(LONG &EndX, LONG &EndY, LONG lX1, LONG lY1, LONG lCenX, LONG lCenY, LONG lWidth, LONG lWidth2);
#ifdef MOVE 
	void MO_Do2dDataLine(LONG EndPX, LONG EndPY, std::vector<DATA_2MOVE> &str);//填充用兩軸連續差補(給值--直線)
	void MO_Do2dDataCir(LONG EndPX, LONG EndPY, LONG CenX, LONG CenY, BOOL bRev, std::vector<DATA_2MOVE> &str);//填充用兩軸連續差補(給值--圓)
	void LA_AbsToOppo3Move(std::vector<DATA_3MOVE> &str);//絕對座標轉相對座標3軸連續插補使用
	void LA_AbsToOppo2Move(std::vector<DATA_2MOVE> &str);//絕對座標轉相對座標2軸連續插補使用
#endif
};


