#pragma once
#include <vector>
#include "Action.h"
// COrder
static CWinThread* g_pThread = NULL;
static CWinThread* g_pSubroutineThread = NULL;
class COrder : public CWnd
{
    DECLARE_DYNAMIC(COrder)
private:
    /************************************************************運動參數結構*******************************************************/
    //座標結構 (狀態、紀錄X、紀錄Y、紀錄Z)
    struct CoordinateData {
        BOOL Status;
        LONG X;
        LONG Y;
        LONG Z;
    };
    //速度結構(加速度、驅動速度)
    struct Speed {
        LONG AccSpeed;
        LONG EndSpeed;
    };
    //點膠設置結構(點膠開啟時間、點膠關閉停留時間)
    struct DispenseDotSet {
        LONG GlueOpenTime;
        LONG GlueCloseTime;
    };
    //點膠結束結構(第一階段抬升距離、低回程速度、高回程速度)
    struct DispenseDotEnd {
        LONG RiseDistance;
        LONG RiseLowSpeed;
        LONG RiseHightSpeed;
    };
    //線段設置結構(移動前延遲、移動前距離、節點時間、停留時間、關機距離、關機延遲)
    struct DispenseLineSet {
        LONG BeforeMoveDelay;
        LONG BeforeMoveDistance;
        LONG NodeTime;
        LONG StayTime;
        LONG ShutdownDistance;
        LONG ShutdownDelay;
    };
    //線段結束結構(返回類型、低速度、高度、長度、高速度)
    struct DispenseLineEnd {
        LONG Type;
        LONG LowSpeed;
        LONG Height;
        LONG Width;
        LONG HighSpeed;
    };
    //Z軸資訊結構(Z軸回升高度、Z軸回升型態(0絕對1相對))
    struct ZSet {
        LONG ZBackHeight;
        BOOL ZBackType;
    };
    //排膠結構 (停駐位置、自動排膠開關、等待時間、出膠時間、斷膠停留時間)
    struct GlueData {
        CoordinateData  ParkPositionData;
        BOOL GlueAuto;
        LONG GlueWaitTime;
        LONG GlueTime;
        LONG GlueStayTime;
    };
    //初始化結構(點膠設置結構、點膠結束結構、點膠速度、線段設置結構、線段結束結構、線段速度、Z軸結構、排膠結構)
    struct Default {
        DispenseDotSet  DispenseDotSet;
        DispenseDotEnd  DispenseDotEnd;
        Speed           DotSpeedSet;
        DispenseLineSet DispenseLineSet;
        DispenseLineEnd DispenseLineEnd;
        Speed           LineSpeedSet;
        ZSet            ZSet;
        GlueData        GlueData;
    };
    /************************************************************影像參數結構*******************************************************/
    //影像對位點結構(對位點、對焦距離、LoadModel編號、存放Model指針、對位後偏移量X、對位後偏移量Y)
    struct Vision {
        CoordinateData Point;
        LONG FocusDistance;
        UINT LoadModelNum;
        void* MilModel; 
        DOUBLE OffsetX;
        DOUBLE OffsetY;
    };
    //影像修正計算結構(對位點、對位點後偏移量X、對位點後偏移量Y、對位點後偏移角度)
    struct VisionOffset {
        CoordinateData Contraposition;
        DOUBLE OffsetX;
        DOUBLE OffsetY;
        DOUBLE Angle;
    };
    //影像LodaModel設置結構(精確度、尋找速度、匹配值、搜尋範圍寬度、搜尋範圍高度、搜尋範圍起始角度、搜尋範圍結束角度)
    struct VisionSet {
        BOOL Accuracy;
        BOOL Speed;
        BYTE Score;
        int width;
        int height;
        int Startangle;
        int Endangle;
    };
    //影像LoadModel檔案位置結構(路徑、檔案總數、檔案名稱)
    struct VisionFile {
        LPTSTR ModelPath;
        UINT ModelCount;
        std::vector<CString> AllModelName;
    };
    //初始化結構
    struct VisionDefault{
        VisionSet VisionSet;
        VisionFile VisionFile;
    };
    /************************************************************程序參數結構*******************************************************/
	//Label&Subroutine控管結構(標籤計數、標籤名稱、子程序計數、子程序狀態、子程序地址堆疊、子程序座標堆疊)
	struct Program {
		int LabelCount;
		CString LabelName;
		UINT SubroutinCount;
		BOOL CallSubroutineStatus;
		std::vector<UINT> SubroutineStack;
		std::vector<CoordinateData> SubroutinePointStack;
	};
	//運行結構(副程式名稱、運行狀態、運行計數、控制主副程序、主副程序堆疊計數、動作狀態)  
	/*
    *運作狀態(0:未運作 1:運行中 2:暫停中)
	*運行計數(目前做到第幾個指令 0:主程序 1-X:副程序)
    *目前所要讀取的堆疊計數
	*動作狀態(0:線段執行完畢 1:線段尚未執行完成 2:Goto過的線段)
    */
	struct RunData {
		CString SubProgramName;
		UINT RunStatus;
		std::vector<UINT> RunCount;//計數程序命令
		std::vector<UINT> MSChange; //控管目前所讀的程序
        UINT StackingCount;
		std::vector<UINT> ActionStatus;
	};
    //Step&Loop控管結構(循環開關、循環地址紀錄、循環計數、步驟開關、步驟地址紀錄、步驟初始offsetX紀錄、步驟初始offsetY紀錄、步驟計數X、步驟計數Y)
    /*
    *循環開關&步驟開關(用來判別沒有此標籤時狀況)
    */
    struct RepeatData {
        BOOL LoopSwitch;
        std::vector<UINT> LoopAddressNum;
        std::vector<UINT> LoopCount; 
        BOOL StepRepeatSwitch;
        std::vector<UINT> StepRepeatNum;
        std::vector<UINT> StepRepeatInitOffsetX;
        std::vector<UINT> StepRepeatInitOffsetY;
        std::vector<UINT> StepRepeatCountX;
        std::vector<UINT> StepRepeatCountY;
    };
private:    //變數
	HANDLE          wakeEvent;
    CAction         m_Action;
    CoordinateData  InitData;
    CString         Commanding;
    std::vector<CString> CommandSwap;
    std::vector<std::vector<CString>> Command;
    RepeatData      RepeatData;
    Program         Program;
    std::vector<CoordinateData> ArcData, CircleData1, CircleData2, StartData, OffsetData;

private:    //函數
	static  UINT    Thread(LPVOID pParam);
	static  UINT    SubroutineThread(LPVOID pParam);
	static  void    LineGotoActionJudge(LPVOID pParam);
	static  void    ModifyPointOffSet(LPVOID pParam, CString XYZPoint);
    static  void    VisionModify(LPVOID pParam);
	static  CString CommandResolve(CString Command,UINT Choose);
	void            ParameterDefult();
	void            DecideInit();
	void            DecideClear();
	void            MainSubProgramSeparate();
    
	
public:     //變數
    //動作總數
	LONG            Time;
    //程序陣列
	std::vector<CString> CommandMemory;
    //參數值
    Default         Default;
    DispenseDotSet  DispenseDotSet;
    DispenseDotEnd  DispenseDotEnd;
    DispenseLineSet DispenseLineSet;
    DispenseLineEnd DispenseLineEnd;
    Speed           DotSpeedSet, LineSpeedSet;
    ZSet            ZSet;
    GlueData        GlueData;
    //運行程序資料
    RunData         RunData;
    //影像參數 
    Vision          FindMark, FiducialMark1,FiducialMark2;
    VisionSet       VisionSet;
    VisionFile      VisionFile;
    VisionOffset    VisionOffset;
    VisionDefault   VisionDefault;
    BOOL            ListAllFileInDirectory(LPTSTR szPath, LPTSTR szName);
    //影像修正後工作座標
    CoordinateData  FinalWorkCoordinateData;
public:     //函數
	COrder();
	virtual ~COrder();
	//開始命令解譯(成功return 1失敗return 0)
	BOOL    Run();
	//立即停止命令解譯(成功return 1失敗return 0)
	BOOL    Stop();
	//暫停命令解譯(成功return 1失敗return 0)
	BOOL    Pause();
	//繼續命令解譯(成功return 1失敗return 0)
	BOOL    Continue(); 

protected:
	DECLARE_MESSAGE_MAP()
};


