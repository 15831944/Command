/*
*檔案名稱:Action(W軸用)
*內容簡述:運動命令API，詳細參數請查看excel
*＠author 作者名稱:R
*＠data 更新日期:2017/04/18
*@更新內容:nova軸卡使用在四軸點膠機上*/
/****************************************************/
//*
//* 呼叫運動函式傳入的X,Y,Z,W值都為  "針頭座標"
//*
/****************************************************/
#define UseIsNeedle
#ifdef UseIsNeedle
#define USEMODE_W 0
#endif // UseIsNeedle
#ifdef UseIsMachine
#define USEMODE_W 1
#endif // UseIsMachine
#pragma once
#include <vector>
#include "CInterpolationCircle.h"
//static LPVOID  pAction;
static int     g_LaserErrCnt;//雷射時間切計數器(紀錄錯誤用)
static int     g_LaserNuCnt;//雷射時間切計數器(紀錄數量用)
struct LINEGLUESET
{
    LONG StartDelayTime;
    LONG StartDistance;
    LONG lMidDelayTime;
    LONG CloseOffDelayTime;
    LONG CloseDistance;
    LONG CloseONDelayTime;
};

class CAction
{
    public:     //變數
        LPVOID  pAction;          //this指針
        BOOL    m_bIsPause;       //暫停用
        BOOL    m_bIsStop;        //停止用
        BOOL    m_bIsDispend;     //關閉點膠用
        BOOL    m_bIsGetLAend;    //雷射取值最後一點
        LONG    m_TablelZ;        //工作平台高度(Z軸總工作高度)
        LONG    m_OffSetLaserX;   //雷射用x軸偏移量
        LONG    m_OffSetLaserY;   //雷射用y軸偏移量
        LONG    m_OffSetLaserZ;   //雷射用z軸偏移量(B點的Z高度到雷射歸零Z高度的位移值(+))
        LONG    m_HeightLaserZero;//雷射用Z軸歸零完成後高度(掃描高度)
        LONG    m_laserBuff;      //雷射用暫存值(test)
        LONG    m_OffSetScan;     //雷射掃描補償值
        LONG    m_LaserAveBuffZ;  //雷射用平均暫存值(絕對位置z值)
        int     m_LaserCnt;       //雷射線段計數器(掃描用)
        BOOL    m_LaserAverage;   //雷射平均(1使用/0不使用)
        BOOL    m_interruptLock;  //中斷鎖
        BOOL    m_getHeightFlag;  //雷射測高旗標：允許測高
        CPoint  m_MachineCirMid;  //機械同心圓座標
        CPoint  m_MachineOffSet;  //機械同心圓偏移量
        DOUBLE  m_Wangle;         //機械校正角度W
        DOUBLE  WangBuff;         //w軸旋轉角度buff
        CPoint  cpCirMidBuff[2];  //取同心圓座標的buff
        LONG    m_MachineCirRad;  //機械同心圓半徑
        AxeSpace m_HomingOffset_INIT;//原點復歸偏移量--初次設定用
        AxeSpace m_HomingPoint;   //原點復歸點(機械座標)
        BOOL    m_IsCutError;     //切值錯誤
        int     m_ThreadFlag;     //執行緒旗標(再MoMoveThread中執行)
        LONG    m_HomeSpeed_DEF; //原點復歸預設速度(Z,W軸)
        DOUBLE  m_WSpeed;         //W速度變數
        BOOL    m_IsHomingOK;     //原點復歸完成參數(沒有執行0,執行中1)
        BOOL    m_IsCorrection;   //W軸offset是否校正(沒有校正0,校正完成1)
        CPoint  m_WorkRange;      //針頭的工作範圍
        BOOL    m_IsUseLineSet;   //線段設定使用(沒有使用0,有設定1)
        BOOL    m_IsSetLineOK;    //線段設置距離是否正常
        const int m_MaxRangeDeg90 = 302000;//針頭在90度的最大長度(210000+92000)
        const int m_CirRadDeg90 = 92000;//針頭在90度的半徑長度(92000)
        LONG    m_lCutDis;        //四軸線段切點距離
#ifdef MOVE
        std::vector<DATA_4MOVE> W_m_ptVec;//W連續切點儲存vector
#endif

        std::vector<UINT>  LA_m_iVecSP;//主要雷射vector(SP:Scan End)
        static BOOL    m_YtimeOutGlueSet;//Y計時器中斷時出斷膠控制
        static BOOL    m_ZtimeOutGlueSet;//Z計時器中斷時出斷膠控制
        HANDLE m_hComm;
        CString ComportNo = _T("COM6"); //RS232 COMPort
#ifdef MOVE
        std::vector<DATA_3MOVE> LA_m_ptVec;//雷射連續切點儲存vector
        std::vector<DATA_2MOVE> LA_m_ptVec2D;//雷射連續切點儲存vector
        DATA_4MOVE DATA_4Do[768];//連續切暫存
        DATA_3MOVE DATA_3Do[768];//連續切暫存
        DATA_2MOVE DATA_2Do[128];
        DATA_3MOVE DATA_3ZERO_B;//雷射歸零_針頭B點
        DATA_3MOVE DATA_3ZERO_LA;//雷射歸零_雷射B點
#endif
    public:     //析構函數
        CAction();
        virtual ~CAction();
    public:
        //=======================================功能模式========================================//

        /**********單點********************************************/
        //單點點膠動作--(單點點膠X,單點點膠Y,單點點膠Z,角度w,出膠時間(ms),斷膠延遲(ms),Z軸回升高度(相對)最高點,Z軸距離(相對),Z軸型態(0絕對位置/1相對位置),高速度,低速度,驅動速度,加速度,初速度)
        void DecidePointGlue(LONG lX, LONG lY, LONG lZ, DOUBLE dAng, LONG lDoTime, LONG lDelayStopTime, LONG lZBackDistance, BOOL bZDisType, LONG lZdistance, LONG lHighVelocity, LONG lLowVelocity, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);

        /**********線段********************************************/

        //線段開始動作--(線段開始X,線段開始Y,線段開始Z,角度w,移動前延遲,驅動速度,加速度,初速度)
        void DecideLineStartMove(LONG lX, LONG lY, LONG lZ,DOUBLE dAng, LONG lStartDelayTime, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy, BOOL bIsNeedleP = 0);
        //線段中點動作--(線段中點X,線段中點Y,線段中點Z,角度w,對線段中點的停留時間(節點時間),驅動速度,加速度,初速度)
        void DecideLineMidMove(LONG lX, LONG lY, LONG lZ, DOUBLE dAng, LONG lMidDelayTime, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy, BOOL bIsNeedleP = 0);
        //線段結束動作--(線段結束X,線段結束Y,線段結束Z,角度w,關機後在結束點停留時間(停留時間),距離結束點多遠距離關機(關機距離),關機後的延遲時間(關機延遲),Z軸回升距離,Z軸回升的型態,線段結束設定(高速度),返回長度,z返回高度,返回速度(低速),返回類型,驅動速度,加速度,初速度)
        void DecideLineEndMove(LONG lX, LONG lY, LONG lZ, DOUBLE dAng, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime, LONG lZBackDistance, BOOL bZDisType, LONG lHighVelocity, LONG lDistance, LONG lHigh, LONG lLowVelocity, int iType, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy, BOOL bIsNeedleP =0);
        //(現在)線段結束動作多載--(關機後在結束點停留時間(停留時間),關機後的延遲時間(關機延遲),Z軸回升相對距離,Z軸型態(0絕對位置/1相對位置),點膠結束設定(高速度),返回長度,z返回高度,返回速度(低速),驅動速度,加速度,初速度,型態1:在/0:不再)
        void DecideLineEndMove(LONG lCloseOffDelayTime, LONG lCloseONDelayTime,LONG lZBackDistance, BOOL bZDisType, LONG lHighVelocity, LONG lDistance,LONG lHigh, LONG lLowVelocity, LONG lWorkVelociy, LONG lAcceleration,LONG lInitVelociy, BOOL bGeneral = 0);

        /**********圓與圓弧********************************************/

        //圓形動作--(圓點座標X,Y,Z,W,圓點座標X2,Y2,,Z2,W2,驅動速度,初速度)
        void DecideCircle(LONG lX1, LONG lY1, LONG lZ1, DOUBLE dAng1, LONG lX2, LONG lY2, LONG lZ2, DOUBLE dAng2, LONG lWorkVelociy, LONG lInitVelociy);
        //圓弧動作--(圓弧座標X1,Y1,Z1,W1,圓弧座標X2,Y2,Z2,W2,驅動速度,初速度)
        void DecideArc(LONG lX1, LONG lY1, LONG lZ1, DOUBLE dAng1, LONG lX2, LONG lY2, LONG lZ2, DOUBLE dAng2, LONG lWorkVelociy, LONG lInitVelociy);
        //角度圓--(圓心座標X,Y,直徑R,起始角度,結束角度,驅動速度,初速度)
        void AnglCir(LONG lCenX, LONG lCenY, LONG lR, DOUBLE dStartAngl, DOUBLE dEndAngl, LONG lWorkVelociy, LONG lInitVelociy);
        //3D圓(xyz1,xyz2,xyz3,驅動速度)
        void Do3AxisCirle(LONG x1, LONG y1, LONG z1, LONG x2, LONG y2, LONG z2, LONG x3, LONG y3, LONG z3, LONG speed);

        /**********連接動作****************************************/

        //線段開始到中間點動作(S-P)--(線段開始X,Y,Z,W,線段中點X,Y,Z,W,移動前延遲,開機前從起點移動距離(設置距離),對線段中點的停留時間(節點時間ms),驅動速度,加速度,初速度)
        void DecideLineSToP(LONG lX, LONG lY, LONG lZ, DOUBLE dAng, LONG lX2, LONG lY2, LONG lZ2, DOUBLE dAng2, LONG lStartDelayTime, LONG lStartDistance, LONG lMidDelayTime, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
        //線段開始到結束動作(S-E)--(線段開始X,Y,Z,W,結束點X,Y,Z,W,移動前延遲,開機前從起點移動距離(設置距離),關機後在結束點停留時間(停留時間),距離結束點多遠距離關機(關機距離),關機後的延遲時間(關機延遲),Z軸回升距離,Z軸回升型態,點膠結束設定(高速度),返回長度,z返回高度,返回速度(低速),返回類型,驅動速度,加速度,初速度)
        void DecideLineSToE(LONG lX, LONG lY, LONG lZ, DOUBLE dAng, LONG lX2, LONG lY2, LONG lZ2, DOUBLE dAng2, LONG lStartDelayTime, LONG lStartDistance, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime, LONG lZBackDistance, BOOL bZDisType, LONG lHighVelocity, LONG lDistance, LONG lHigh, LONG lLowVelocity, int iType, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
        //線段開始到圓中間點動作(S-C-P)--(圓形動作--(線段開始X1,Y1,Z1,W1,圓形座標X2,Y2,Z2,W2圓形座標X3,Y3,Z3,W3,中間點X4,Y4,Z4,W4,移動前延遲,開機前從起點移動距離(設置距離),對線段中點的停留時間(節點時間ms),驅動速度,加速度,初速度)
        void DecideLineSToCirP(LONG lX1, LONG lY1, LONG lZ1, DOUBLE dAng1, LONG lX2, LONG lY2, LONG lZ2, DOUBLE dAng2, LONG lX3, LONG lY3, LONG lZ3, DOUBLE dAng3, LONG lX4, LONG lY4, LONG lZ4, DOUBLE dAng4, LONG lStartDelayTime, LONG lStartDistance, LONG lMidDelayTime, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
        //線段開始到圓弧中間點動作(S-A-P)--(圓形動作--(線段開始X1,Y1,Z1,W1,圓弧座標X2,Y2,Z2,W2圓弧座標X3,Y3,Z3,W3,移動前延遲,開機前從起點移動距離(設置距離),對線段中點的停留時間(節點時間ms),驅動速度,加速度,初速度)
        void DecideLineSToArcP(LONG lX1, LONG lY1, LONG lZ1, DOUBLE dAng1, LONG lX2, LONG lY2, LONG lZ2, DOUBLE dAng2, LONG lX3, LONG lY3, LONG lZ3, DOUBLE dAng3, LONG lStartDelayTime, LONG lStartDistance, LONG lMidDelayTime, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
        //線段開始到圓中間點到結束點動作(S-C-E)--(圓形動作--(線段開始X1,Y1,Z1,W1,圓形座標X2,Y2,Z2,W2圓形座標X3,Y3,Z3,W3,結束座標X4,Y4,Z4,W4,移動前延遲,開機前從起點移動距離(設置距離),關機後在結束點停留時間(停留時間),距離結束點多遠距離關機(關機距離),關機後的延遲時間(關機延遲),Z軸回升相對距離,Z軸型態(0絕對位置/1相對位置),點膠結束設定(高速度),返回長度,z返回高度,返回速度(低速),返回類型,驅動速度,加速度,初速度)
        void DecideLineSToCirEnd(LONG lX1, LONG lY1, LONG lZ1, DOUBLE dAng1, LONG lX2, LONG lY2, LONG lZ2, DOUBLE dAng2, LONG lX3, LONG lY3, LONG lZ3, DOUBLE dAng3, LONG lX4, LONG lY4, LONG lZ4, DOUBLE dAng4, LONG lStartDelayTime, LONG lStartDistance, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime, LONG lZBackDistance, BOOL bZDisType, LONG lHighVelocity, LONG lDistance, LONG lHigh, LONG lLowVelocity, int iType, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
        //線段開始到圓弧中間點到結束點動作(S-A-E)--(圓弧動作--(線段開始X1,Y1,Z1,W1,圓弧座標X2,Y2,Z2,W2結束座標X3,Y3,Z3,W3,移動前延遲,開機前從起點移動距離(設置距離),關機後在結束點停留時間(停留時間),距離結束點多遠距離關機(關機距離),關機後的延遲時間(關機延遲),Z軸回升相對距離,Z軸型態(0絕對位置/1相對位置),點膠結束設定(高速度),返回長度,z返回高度,返回速度(低速),返回類型,驅動速度,加速度,初速度)
        void DecideLineSToArcEnd(LONG lX1, LONG lY1, LONG lZ1, DOUBLE dAng1, LONG lX2, LONG lY2, LONG lZ2, DOUBLE dAng2, LONG lX3, LONG lY3, LONG lZ3, DOUBLE dAng3, LONG lStartDelayTime, LONG lStartDistance, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime, LONG lZBackDistance, BOOL bZDisType, LONG lHighVelocity, LONG lDistance, LONG lHigh, LONG lLowVelocity, int iType, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
        //線段圓-中間點到結束動作(C-P-E)--(圓形動作--(圓形座標X1,Y1,Z1,W1圓形座標X2,Y2,Z2,W2,線段結束X3,Y3,Z3,W3,關機後在結束點停留時間(停留時間),距離結束點多遠距離關機(關機距離),關機後的延遲時間(關機延遲),Z軸回升相對距離,Z軸型態(0絕對位置/1相對位置),點膠結束設定(高速度),返回長度,z返回高度,返回速度(低速),返回類型,驅動速度,加速度,初速度)
        void DecideCirclePToEnd(LONG lX1, LONG lY1, LONG lZ1, DOUBLE dAng1, LONG lX2, LONG lY2, LONG lZ2, DOUBLE dAng2, LONG lX3, LONG lY3, LONG lZ3, DOUBLE dAng3, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime, LONG lZBackDistance, BOOL bZDisType, LONG lHighVelocity, LONG lDistance, LONG lHigh, LONG lLowVelocity, int iType, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy, BOOL bIsNeedleP = 0);
        //線段圓弧-中間點到結束動作(A-P-E)--(圓弧動作--(圓弧座標X1,Y1,Z1,W1,線段結束X2,Y2,Z2,W2,關機後在結束點停留時間(停留時間),距離結束點多遠距離關機(關機距離),關機後的延遲時間(關機延遲),Z軸回升相對距離,Z軸型態(0絕對位置/1相對位置),點膠結束設定(高速度),返回長度,z返回高度,返回速度(低速),返回類型,驅動速度,加速度,初速度)
        void DecideArclePToEnd(LONG lX1, LONG lY1, LONG lZ1, DOUBLE dAng1, LONG lX2, LONG lY2, LONG lZ2, DOUBLE dAng2, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime, LONG lZBackDistance, BOOL bZDisType, LONG lHighVelocity, LONG lDistance, LONG lHigh, LONG lLowVelocity, int iType, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy, BOOL bIsNeedleP = 0);

        /**********單點&復歸****************************************/
        //虛擬點動作--(虛擬點座標X,Y,Z,w,驅動速度,加速度,初速度)
        void DecideVirtualPoint(LONG lX, LONG lY, LONG lZ, DOUBLE dAng, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy, BOOL bIsNeedleP = 0);
        //等待點動作--(等待點座標X,Y,Z,w,等待時間,驅動速度,加速度,初速度)
        void DecideWaitPoint(LONG lX, LONG lY, LONG lZ, DOUBLE dAng, LONG lWaitTime, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
        //停駐點動作--(停駐點座標X,Y,Z,w,排膠時間,結束後等待時間,Z軸回升高度(相對)最高點,Z軸距離(相對),高速度,低速度,驅動速度,加速度,初速度)
        void DecideParkPoint(LONG lX, LONG lY, LONG lZ, DOUBLE dAng, LONG lTimeGlue, LONG lWaitTime, LONG lStayTime, LONG lZBackDistance, BOOL bZDisType, LONG lZdistance, LONG lHighVelocity, LONG lLowVelocity, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
        //原點復歸--(原點復歸速度1,原點復歸速度2,復歸軸(7),偏移量(x,y,z,w))
        void DecideInitializationMachine(LONG lSpeed1, LONG lSpeed2, LONG lAxis,LONG lMoveX, LONG lMoveY, LONG lMoveZ, DOUBLE dMoveW);
        //W軸原點復歸--(原點復歸速度1,原點復歸速度2)
        void DecideGoHomeW(LONG lSpeed1, LONG lSpeed2);
        //虛擬點原點復歸
        void DecideVirtualHome(LONG lX, LONG lY, LONG lZ, LONG lZBackDistance, BOOL bZDisType, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
        //原點復歸，回絕對位置(0,0,0,0)
        void BackGOZero(LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
        //針頭清潔動作(清潔裝置位置X,Y,Z,清潔裝置使用的io,Z軸回升相對距離,Z軸型態(0絕對位置/1相對位置),驅動速度,家速度,初速度)
        void DispenClear(LONG lX, LONG lY, LONG lZ, int ClreaPort, LONG lZBackDistance, BOOL bZDisType, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);

        /**********IO命令******************************************/

        //輸出-16個輸出(選擇埠(0~15),開啟關閉(0~1))
        BOOL DecideOutPutSign(int iPort, BOOL bChoose);
        //輸入-12個輸入(選擇埠(0~11),開啟關閉(0~1))
        BOOL DecideInPutSign(int iPort, BOOL bChoose);

        /**********人機用函數****************************************/

        //人機用函數-軟體負極限(x,y,z,w為最小工作範圍)
        void HMNegLim(LONG lX, LONG lY, LONG lZ, DOUBLE dW);
        //人機用函數-軟體正極限(x,y,z,w為最大工作範圍)
        void HMPosLim(LONG lX, LONG lY, LONG lZ, DOUBLE dW);
        //人機用函數-移動命令(Z軸抬生→W軸旋轉→X,Y移動→Z軸下降)
        void HMGoPosition(LONG lX, LONG lY, LONG lZ, DOUBLE dW, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
        //人機用函數-參數寫入(機械同心圓偏移量X.Y,機械同心圓半徑,工作平台高度(Z軸總工作高度))
        void HMSetWOffset(LONG Xoffset, LONG Yoffset, LONG MachineCirRad, LONG TablelZ);

        /**********其它命令*****************************************/

        //位置偏移(輸入一個(X,Y,Z,W)座標)
        CString NowOffSet(LONG lX, LONG lY, LONG lZ, DOUBLE dAng);
        //回傳目前位置
        CString NowLocation();
        //CCD移動用--(點座標X,Y,Z,驅動速度,加速度,初速度)
        void DoCCDMove(LONG lX, LONG lY, LONG lZ, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);

        /**********雷射********************************************/

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
        //三軸連續插補
        void LA_Line3DtoDo(int iData, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy, BOOL bDoAll = FALSE);
        //雷射清除指令
        void LA_Clear();
        //雷射平均回傳平均z值
        void LA_AverageZ(LONG lStrX, LONG lStrY, LONG lEndX, LONG lEndY, LONG &lZ, LONG lStartVe, LONG lStartAcc, LONG lStartInitVe, LONG  lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);


        /**********連續插補********************************************/

        //2D連續差補資料輸入
        void LA_Do2DVetor(LONG lX3, LONG lY3, LONG lX2 = 0, LONG lY2 = 0, LONG lX1 = 0, LONG lY1 = 0);//!!注意點座標順序3.2.1
        void LA_Do2dDataLine(LONG EndPX, LONG EndPY,BOOL bIsNeedleP=0);
        void LA_Do2dDataArc(LONG EndPX, LONG EndPY, LONG ArcX, LONG ArcY);
        void LA_Do2dDataCircle(LONG EndPX, LONG EndPY, LONG CirP1X, LONG CirP1Y, LONG CirP2X, LONG CirP2Y);
        //單點雷射取值(使用DATA_2MOVE結構)+偏移量
        void LA_Line2D(LONG lStartVe, LONG lStartAcc, LONG lStartInitVe, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);

        /**********填充區***********************************************/

        //填充動作(線段開始X,Y,Z，線段結束X,Y,Z，Z軸距離(相對)，Z軸型態(0絕對位置/1相對位置)，填充形式(1~7)，寬度(mm)，兩端寬度(mm)，線段點膠設定(1.移動前延遲，2.設置距離，3停留時間，5關機距離，6關機延遲)，驅動速度，加速度，初速度)
        void DecideFill(LONG lX1, LONG lY1, LONG lZ1, LONG lX2, LONG lY2, LONG lZ2, LONG lZBackDistance, BOOL bZDisType, int iType, LONG lWidth, LONG lWidth2, LONG lStartDelayTime, LONG lStartDistance, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
        //填充動作_多載(最後一點位置(EndX,EndY),線段開始X,Y,Z，線段結束X,Y,Z，Z軸距離(相對)，Z軸型態(0絕對位置/1相對位置)，填充形式(1~7)，寬度(mm)，兩端寬度(mm))
        void Fill_EndPoint(LONG &lEndX, LONG &lEndY, LONG lX1, LONG lY1, LONG lZ1, LONG lX2, LONG lY2, LONG lZ2, int iType, LONG lWidth, LONG lWidth2);

        /**********運動附屬函式******************************************/
        void WaitTime(HANDLE wakeEvent, int Time);
        //軸卡中斷thread
        static DWORD WINAPI MoInterrupt(LPVOID);
        static UINT WINAPIV MoMoveThread(LPVOID param);//執行序
        //顯示版本
        CString ShowVersion();
        //出膠，多載有延遲時間(配合執行緒使用)(單位:ms)
        void DoGlue(LONG lTime, LONG lDelayTime, LPTHREAD_START_ROUTINE MoInterrupt = NULL);
        //防止軸卡出錯
        void PreventMoveError();
        //暫停回復後重新塗膠(讀取暫停參數，當參數為0時出膠，且點膠機要為開。)
        void PauseDoGlue();
        //防止出膠出錯
        void PreventGlueError();
        //暫停時停指塗膠(讀取暫停參數，當參數為1時斷膠，或點膠機關參數為0也斷膠。)
        void PauseStopGlue();
        //返回設定
        void GelatinizeBack(int iType, LONG lXarEnd, LONG lYarEnd, LONG lZarEnd, LONG lXarUp, LONG lYarUp, LONG lLineStop, LONG lStopZar, LONG lBackZar, LONG lLowSpeed, LONG lHighSpeed, LONG lAcceleration, LONG lInitSpeed);
        //直線距離轉換成座標點
        void LineGetToPoint(LONG &lXClose, LONG &lYClose, LONG lX0, LONG lY0, LONG lX1, LONG lY1, LONG &lLineClose);
        //直線距離轉換成座標點--多載3D
        void LineGetToPoint(LONG &lXClose, LONG &lYClose, LONG &lZClose, LONG lX0, LONG lY0, LONG lX1, LONG lY1, LONG lZ0, LONG lZ1, LONG &lLineClose, BOOL bStart0End1);
        //計算提前出斷膠距離
        LONG CalPreglue(LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
        //計算提前出斷膠時間(us)
        LONG CalPreglueTime(LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
        //計算提前出斷膠距離或時間(ms)
        LONG CalPreglue(LONG lStartDistance, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
        //直線段使用，關機距離在結束點斷膠ms(lCutDis:切點距離,驅動速度,加速度,初速度)--回傳等速速度值(um/s)
        LONG CalPreglueEnd(LONG lCutDis, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
        //三點計算圓心
        CString TRoundCCalculation(CString Origin, CString End, CString Between);
        //字串轉長整數，使用的結尾一定要是"，"
        LONG CStringToLong(CString csData, int iChoose);
        //三點取得角度(向量夾角)(圓心x,y座標x1,y1,座標x2,y2,正逆轉)
        DOUBLE AngleCount(DOUBLE LocatX, DOUBLE LocatY, DOUBLE LocatX1, DOUBLE LocatY1, DOUBLE LocatX2, DOUBLE LocatY2, BOOL bRev);
        //圓弧長距離轉點做標
        void ArcGetToPoint(LONG &lArcX, LONG &lArcY, LONG lDistance, LONG lX, LONG lY, LONG lCenX, LONG lCenY, LONG lRadius, BOOL bDir);
        //將3Dvetor值作旋轉偏移修正
        void LA_CorrectVectorToDo(LONG  lWorkVelociy, LONG lAcceleration, LONG lInitVelociy, LONG RefX = 0, LONG RefY = 0, DOUBLE OffSetX = 0, DOUBLE OffSetY = 0, DOUBLE Andgle = 0, DOUBLE CameraToTipOffsetX = 0, DOUBLE CameraToTipOffsetY = 0, BOOL Mode = 0, LONG lSubOffsetX = 0, LONG lSubOffsetY = 0);
        //雷射用旋轉平移
        void LA_CorrectLocation(LONG &PointX, LONG &PointY, LONG RefX, LONG RefY, DOUBLE OffSetX, DOUBLE OffSetY, DOUBLE Andgle, DOUBLE CameraToTipOffsetX, DOUBLE CameraToTipOffsetY, BOOL Mode, LONG lSubOffsetX, LONG lSubOffsetY);
#ifdef MOVE
        //絕對座標轉相對座標3軸連續插補使用
        void LA_AbsToOppo3Move(std::vector<DATA_3MOVE> &str);
        //絕對座標轉相對座標2軸連續插補使用
        void LA_AbsToOppo2Move(std::vector<DATA_2MOVE> &str);
        //填充用兩軸連續差補(給值--直線)
        void MCO_Do2dDataLine(LONG EndPX, LONG EndPY, std::vector<DATA_2MOVE> &str);
        //填充用兩軸連續差補(給值--圓)
        void MCO_Do2dDataCir(LONG EndPX, LONG EndPY, LONG CenX, LONG CenY, BOOL bRev, std::vector<DATA_2MOVE> &str);
#endif
        /**********w軸用函式********************************************/
        //w軸offset更新
        void W_UpdateNeedleMotorOffset(CPoint &offset, DOUBLE degree);
        //w軸連續更新數值(傳入機械座標)
        void W_UpdateNeedleMotor_Robot(LONG lX, LONG lY, LONG lZ, DOUBLE dAngle0, LONG lX2, LONG lY2, LONG lZ2, DOUBLE dAngle1,DOUBLE DisAngle =1);
        //w軸連續更新數值(傳入針頭座標)
        void W_UpdateNeedleMotor_Needle(LONG lX, LONG lY, LONG lZ, DOUBLE dAngle0, LONG lX2, LONG lY2, LONG lZ2, DOUBLE dAngle1, DOUBLE DisAngle = 1);
        //w軸自轉(針頭座標固定同一個點)
        void W_Rotation(DOUBLE dAngle, LONG lWorkVelociy, LONG lAcceleration,LONG lInitVelociy, DOUBLE DisAngle = 1);
        //得到針頭座標(傳入機械座標)
        CPoint W_GetNeedlePoint(LONG lRobotX = 0, LONG lRobotY = 0, DOUBLE dRobotW = 0,BOOL bMode=0);
        //得到機械座標(傳入針頭座標)
        CPoint W_GetMachinePoint(LONG lNeedleX = 0, LONG lNeedleY = 0, DOUBLE dNeedleW = 0, BOOL bMode = 0);
#ifdef MOVE
        //絕對座標轉相對座標4軸連續插補使用
        void W_AbsToOppo4Move(std::vector<DATA_4MOVE> &str);
#endif
        //連續線段動作--(先移動到起始點x,y在走z___進行四軸連續插補)
        void W_Line4DtoDo(LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy, BOOL bIsUseLineSet = 0);
        void W_Line4DtoDo(LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy, LINEGLUESET glueSet);

        //W軸四連續插補單純移動
        void W_Line4DtoMove(LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
        //W軸校正動作--步驟1.2(步驟(0,1)、驅動速度、加速度、初速度、z軸抬升高度)
        void W_Correction(BOOL bStep, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy,LONG lMoveZ = 10000);
        //讀取現在位置坐標(預設針頭座標0/機械座標1)
        AxeSpace MCO_ReadPosition(BOOL NedMah = 0);
        //四軸插補移動命令(角度、驅動速度、加速度、初速度、0:相對/1:絕對)預設相對角度
        void MCO_Do4DLineMove(DOUBLE dAng, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy, BOOL bIsType = 0);
        //針頭模式原點復歸(步驟0為初始化第一步/步驟1為一般原點復歸)
        void W_NeedleGoHoming(LONG Speed1,LONG Speed2, BOOL bStep=1);

        /**********JOG模式********************************************/
        //單軸移動(X,Y,Z,W相對量移動/WType:0單軸,1同軸自轉/全部為0使用減速停止)
        void MCO_JogMove(LONG lX, LONG lY, LONG lZ, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy, DOUBLE dW = 0, BOOL WType = 0);
    private:

        /**********************四軸轉換專用************************/
        //w軸連續更新數值(傳入針頭座標)_擇一使用-線段設定
        void W_UpdateNeedleMotor_Setting(LONG lX, LONG lY, LONG lZ, DOUBLE dAngle0, LONG lX2, LONG lY2, LONG lZ2, DOUBLE dAngle1, AxeSpace APoint, BOOL bStart0End1, DOUBLE DisAngle = 1);
        //w軸連續更新數值(多載_設置距離與關機距離用)_階使用-線段設定
        void W_UpdateNeedleMotor_Setting(LONG lX, LONG lY, LONG lZ, DOUBLE dAngle0, LONG lX2, LONG lY2, LONG lZ2, DOUBLE dAngle1, AxeSpace APoint_Start, AxeSpace APoint_End, int iS0E1O2 = 2, DOUBLE DisAngle = 1);
        //圓形/圓弧(True/False)_加入設置距離APoint--(圓點座標X,Y,Z,W,圓點座標X2,Y2,,Z2,W2,APoint,驅動速度,初速度)
        void W_CirArcMove(LONG lX1, LONG lY1, LONG lZ1, DOUBLE dAng1, LONG lX2, LONG lY2, LONG lZ2, DOUBLE dAng2, AxeSpace APoint_Start, AxeSpace APoint_Set, LINEGLUESET glueSet, LONG lWorkVelocity, LONG lAcceleration, LONG lInitVelocity, BOOL cirArcType = TRUE);
        //圓弧多載_加入設置距離APoint--(圓弧點座標X,Y,Z,W,圓弧點座標X2,Y2,,Z2,W2,APoint,驅動速度,初速度)
        void W_ArcMove(LONG lX1, LONG lY1, LONG lZ1, DOUBLE dAng1, LONG lX2, LONG lY2, LONG lZ2, DOUBLE dAng2, AxeSpace APoint_Start, AxeSpace APoint_Set, LINEGLUESET glueSet, LONG lWorkVelocity, LONG lAcceleration, LONG lInitVelocity);
        /**********************填充專用************************/
        //附屬--- 填充形態1-矩形S
        void AttachFillType1(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lZ, LONG lZBackDistance, LONG lWidth, LONG lStartDelayTime, LONG lStartDistance, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
        //附屬--- 填充形態2-螺旋圓
        void AttachFillType2(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lZ, LONG lZBackDistance, LONG lWidth, LONG lStartDelayTime, LONG lStartDistance, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
        //附屬--- 填充形態3-矩形螺旋
        void AttachFillType3(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lZ, LONG lZBackDistance, LONG lWidth, LONG lStartDelayTime, LONG lStartDistance, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
        //附屬--- 填充形態4-矩形環
        void AttachFillType4(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lZ, LONG lZBackDistance, LONG lWidth, LONG lWidth2, LONG lStartDelayTime, LONG lStartDistance, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
        //附屬--- 填充形態5-圓環
        void AttachFillType5(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lZ, LONG lZBackDistance, LONG lWidth, LONG lWidth2, LONG lStartDelayTime, LONG lStartDistance, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
        //附屬--- 填充形態6-矩形反螺旋
        void AttachFillType6(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lZ, LONG lZBackDistance, LONG lWidth, LONG lStartDelayTime, LONG lStartDistance, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
        //附屬--- 填充形態7-反螺旋圓
        void AttachFillType7(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lZ, LONG lZBackDistance, LONG lWidth, LONG lStartDelayTime, LONG lStartDistance, LONG lCloseOffDelayTime, LONG lCloseDistance, LONG lCloseONDelayTime, LONG lWorkVelociy, LONG lAcceleration, LONG lInitVelociy);
        //附屬--- 填充形態3-矩形螺旋end
        void AttachFillType3_End(LONG &EndX, LONG &EndY, LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lWidth, LONG lWidth2);
        //附屬--- 填充形態4-矩形環end
        void AttachFillType4_End(LONG &EndX, LONG &EndY, LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lWidth, LONG lWidth2);
        //附屬--- 填充形態5-圓環end
        void AttachFillType5_End(LONG &EndX, LONG &EndY, LONG lX1, LONG lY1, LONG lCenX, LONG lCenY, LONG lWidth, LONG lWidth2);

        //******************以下為保留含式****************************************************************************************
        //===============未使用連續插補的填充型態=================================================================================================================
        void AttachFillType2_1(LONG lX1, LONG lY1, LONG lCenX, LONG lCenY, LONG lZ,LONG lZBackDistance, LONG lWidth, LONG lWorkVelociy, LONG lAcceleration,LONG lInitVelociy);//附屬--- 填充形態2-1
        void AttachFillType3_1(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lZ,LONG lZBackDistance, LONG lWidth, LONG lWorkVelociy, LONG lAcceleration,LONG lInitVelociy);//附屬--- 填充形態3
        void AttachFillType4_1(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lZ,LONG lZBackDistance, LONG lWidth, LONG lWidth2, LONG lWorkVelociy,LONG lAcceleration, LONG lInitVelociy);//附屬--- 填充形態4
        void AttachFillType5_1(LONG lX1, LONG lY1, LONG lCenX, LONG lCenY, LONG lZ,LONG lZBackDistance, LONG lWidth, LONG lWidth2, LONG lWorkVelociy,LONG lAcceleration, LONG lInitVelociy);//附屬--- 填充形態5
        void AttachFillType6_1(LONG lX1, LONG lY1, LONG lX2, LONG lY2, LONG lZ,LONG lZBackDistance, LONG lWidth, LONG lWorkVelociy, LONG lAcceleration,LONG lInitVelociy);//附屬--- 填充形態6
        void AttachFillType7_1(LONG lX1, LONG lY1, LONG lCenX, LONG lCenY, LONG lZ,LONG lZBackDistance, LONG lWidth, LONG lWorkVelociy, LONG lAcceleration,LONG lInitVelociy);//附屬--- 填充形態7

};


