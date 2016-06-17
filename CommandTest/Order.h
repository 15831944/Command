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
	//回原點結構(回歸速度1、回歸速度2、軸數、偏移X、偏移Y、偏移Z、循環是否復歸)
	struct GoHome {
		LONG Speed1;
		LONG Speed2;
		LONG Axis;
		LONG MoveX;
		LONG MoveY;
		LONG MoveZ;
		BOOL PrecycleInitialize;
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
		GoHome          GoHome;
	};
	/************************************************************影像參數結構*******************************************************/
	//影像對位點結構(標記查找狀態(TRUE = 找到 FALSE = 未找到)、對位點、對焦距離、LoadModel編號、存放Model指針、對位後偏移量X、對位後偏移量Y)
	struct Vision {
		BOOL FindMarkStatus;
		CoordinateData Point;
		LONG FocusDistance;
		LONG LoadModelNum;
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
	//影像LodaModel設置結構(修正模式、精確度、尋找速度、匹配值、搜尋範圍寬度、搜尋範圍高度、搜尋範圍起始角度、搜尋範圍結束角度、搜尋錯誤編號、尋問對話框)
	struct VisionSet {
		BOOL ModifyMode;
		BOOL Accuracy;
		BOOL Speed;
		BYTE Score;
		int width;
		int height;
		int Startangle;
		int Endangle;
		LONG AdjustOffsetX;
		LONG AdjustOffsetY;
	};
	//影像LoadModel檔案位置結構(路徑、檔案總數、檔案名稱)
	struct VisionFile {
		LPTSTR ModelPath;
		UINT ModelCount;
		std::vector<CString> AllModelName;
	};
	//影像擴大搜尋 ()
	struct VisionTrigger {
		UINT AdjustStatus;
		BOOL Trigger1Switch;
		BOOL Trigger2Switch;
		std::vector<CoordinateData> Trigger1;
		std::vector<CoordinateData> Trigger2;
	};
	struct VisionSerchError {
		int SearchError;
		CDialog* pQuestion;
		BOOL Manuallymode;
		BOOL Pausemode;
	};
	//影像初始化結構
	struct VisionDefault{
		VisionSet VisionSet;
		VisionFile VisionFile;
		VisionSerchError VisionSerchError;
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
		std::vector<UINT> RunCount;//計數程序命令
		std::vector<UINT> MSChange; //控管目前所讀的程序
		UINT StackingCount;
		std::vector<UINT> ActionStatus;  
	};
	//運行狀態讀取結構(運行狀態、目前命令進度、回原點狀態(TRUE = 賦歸完成 FLASE = 賦歸中))
	struct RunStatusRead {
		UINT RunStatus;
		UINT CurrentRunCommandNum;
		BOOL GoHomeStatus;
		UINT FinishProgramCount;
	};
    //阻斷控管結構
    struct StepRepeatBlockData {
        int BlockNumber;
        std::vector<CString> BlockPosition;
    };

	//Step&Loop控管結構(循環開關、循環地址紀錄、循環計數、步驟開關、步驟地址紀錄、步驟初始offsetX紀錄、步驟初始offsetY紀錄、步驟計數X、步驟計數Y)
	/*
	*循環開關&步驟開關(用來判別沒有此標籤時狀況)
	*/
	struct RepeatData {
		BOOL LoopSwitch;
		std::vector<UINT> LoopAddressNum;
		std::vector<UINT> LoopCount;
        CString StepRepeatLabel;
        BOOL StepRepeatLabelLock;
		BOOL StepRepeatSwitch;
        std::vector<BOOL> SSwitch;
		std::vector<UINT> StepRepeatNum;
		std::vector<UINT> StepRepeatInitOffsetX;
		std::vector<UINT> StepRepeatInitOffsetY;
		std::vector<int> StepRepeatCountX;
		std::vector<int> StepRepeatCountY;
        std::vector<StepRepeatBlockData> StepRepeatBlockData;
	};
private:    //變數
	HANDLE          wakeEvent;
	//主運動物件
	CAction         m_Action;
	//命令
    StepRepeatBlockData InitBlockData;
	CoordinateData  InitData;
	CString         Commanding;
	std::vector<CString> CommandSwap;
	std::vector<std::vector<CString>> Command;
	//程序
	RepeatData      RepeatData;
	Program         Program;
	RunData         RunData;
	//狀態
	std::vector<CoordinateData> ArcData, CircleData1, CircleData2, StartData, OffsetData;
	
private:    //函數
	//執行續
	static  UINT    HomeThread(LPVOID pParam);
	static  UINT    Thread(LPVOID pParam);
	static  UINT    SubroutineThread(LPVOID pParam);
	//動作處理
	static  void    LineGotoActionJudge(LPVOID pParam);
	static  void    ModifyPointOffSet(LPVOID pParam, CString XYZPoint);
	static  void    VisionModify(LPVOID pParam);
	void            VisionFindMarkError(LPVOID pParam);
    static  void    BlockProcessStart(CString Command, LPVOID pParam);
    static  void    BlockProcessExecute(CString Command, LPVOID pParam, int NowCount);
	//命令處理
	static  CString CommandResolve(CString Command,UINT Choose);
	//初始化處理
	void            ParameterDefult();
	void            DecideInit();
	void            DecideClear();
	void            MainSubProgramSeparate();
	//檔案處理
	BOOL            ListAllFileInDirectory(LPTSTR szPath, LPTSTR szName);
	static  BOOL    FileExist(LPCWSTR FilePathName);
  
    
public:     //變數
	//動作總數
	LONG            Time;
	//程序陣列
	std::vector<CString> CommandMemory;
	//運動參數值
	Default         Default;

	DispenseDotSet  DispenseDotSet;
	DispenseDotEnd  DispenseDotEnd;
	DispenseLineSet DispenseLineSet;
	DispenseLineEnd DispenseLineEnd;
	Speed           DotSpeedSet, LineSpeedSet;
	ZSet            ZSet;
	GlueData        GlueData;
	GoHome          GoHome;
	//運行程序資料
	RunStatusRead   RunStatusRead;
	//影像參數 
	VisionDefault   VisionDefault;
	VisionSet       VisionSet;
	VisionFile      VisionFile;
	VisionSerchError VisionSerchError;
	//影像
	Vision          FindMark, FiducialMark1, FiducialMark2;
	VisionOffset    VisionOffset;
	VisionTrigger   VisionTrigger;
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
	//原點賦歸(成功return  1 失敗 return 0 )
	BOOL    Home();
	//View命令解譯(參數:模式(FALSE 針頭 TRUE CCD))(成功return 1 失敗 return 0)
	BOOL    View(BOOL mode);

protected:
	DECLARE_MESSAGE_MAP()
};


