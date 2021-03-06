#pragma once
#include <vector>
#include "Action.h"
// COrder
static CWinThread* g_pThread = NULL;
static CWinThread* g_pSubroutineThread = NULL;
static CWinThread* g_pRunLoopThread = NULL;
static CWinThread* g_pIODetectionThread = NULL;
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
	//速度結構(加速度、驅動速度、初速度)
	struct Speed {
		LONG AccSpeed;
		LONG EndSpeed;
        LONG InitSpeed;
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
	//回原點結構(回歸速度1、回歸速度2、軸數、偏移X、偏移Y、偏移Z、循環是否復歸、是否為影像賦歸)
	struct GoHome {
		LONG Speed1;
		LONG Speed2;
		LONG Axis;
		LONG MoveX;
		LONG MoveY;
		LONG MoveZ;
		BOOL PrecycleInitialize;
		BOOL VisionGoHome;
	};
	//初始化結構(點膠設置結構、點膠結束結構、點膠速度、線段設置結構、線段結束結構、線段速度、Z軸結構、排膠結構、回原點結構)
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
	//修正表結構(命令地址、影像修正編號、雷射修正編號)
	struct PositionModifyNumber {
		CString Address;
		int VisionNumber;
		int LaserNumber;
	};
	/************************************************************I/O參數結構*******************************************************/
	//IO控制結構(IO偵測開關、出膠鈕判斷)
    struct IOControl {
        BOOL SwitchInformation;
        BOOL GlueInformation;
	};
    //IO參數結構(EMG對話框指針)
    struct IOParam{
        CDialog* pEMGDlg;
    };
	/************************************************************影像參數結構*******************************************************/
	//影像對位點結構(標記查找狀態(TRUE = 找到 FALSE = 未找到)、對位點、對焦距離、LoadModel編號、存放Model指針、對位後偏移量X、對位後偏移量Y)
	struct Vision {
		BOOL FindMarkStatus;
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
	//影像擴大搜尋(調整狀態、擴大對位1開關、擴大對位2開關、擴大對位區間1、擴大對位區間2)
	struct VisionTrigger {
		UINT AdjustStatus;
		BOOL Trigger1Switch;
		BOOL Trigger2Switch;
		std::vector<CoordinateData> Trigger1;
		std::vector<CoordinateData> Trigger2;
	};
	//影像搜尋錯誤結構(搜尋錯誤方式、尋問對話框指針、手動模式開關、暫停模式判斷)
	struct VisionSerchError {
		int SearchError;
		CDialog* pQuestion;
		BOOL Manuallymode;
		BOOL Pausemode;
	};
	//影像初始化結構(影像設置結構、影像對位點檔案結構、影像搜尋錯誤結構)
	struct VisionDefault{
		VisionSet VisionSet;
		VisionFile VisionFile;
		VisionSerchError VisionSerchError;
	};
	//影像Offset值紀錄陣列(影像修正計算結構)
	struct VisionAdjust {
		VisionOffset VisionOffset;
	};
	/************************************************************雷射參數結構*******************************************************/ 
	//Laser模式轉換開關(雷射高度、雷射點調節、雷射檢測、雷射調整、雷射跳過)
	struct LaserSwitch {
		BOOL LaserHeight;
		BOOL LaserPointAdjust;
		BOOL LaserDetect;
		BOOL LaserAdjust;
		BOOL LaserSkip;
	};
	//Laser資料紀錄(雷射地址(之後用來記錄關閉開啟位置)、雷射單點測高座標、雷射線段平均測高開始和結束座標、雷射測高數據)
	struct LaserData {
		UINT LaserAddress;
		CoordinateData LaserHeightPoint;
		CoordinateData LaserDetectLS, LaserDetectLE;
		LONG LaserMeasureHeight;		
	};
	//Laser測高值紀錄陣列(雷射測高數據)
	struct LaserAdjust {
		LONG LaserMeasureHeight; 
	};
	/************************************************************模組參數結構*******************************************************/
	//控管模組結構(模式選擇、模式轉換地址(目前不使用)、影像模組跳過、雷射模組跳過)
	struct ModelControl{
		UINT Mode;
		UINT ModeChangeAddress;
		UINT VisionModeJump;
		UINT LaserModeJump;
	};
	/************************************************************程序參數結構*******************************************************/
	//Label&Subroutine控管結構(標籤計數、標籤名稱、子程序計數、子程序狀態、子程序地址堆疊、子程序座標堆疊、子程序控制是否修正影像、子程序命令預處理)
	struct Program {
		int LabelCount;
		CString LabelName;
		UINT SubroutinCount;
		BOOL CallSubroutineStatus;
		std::vector<UINT> SubroutineStack;
		std::vector<CoordinateData> SubroutinePointStack;
		std::vector<BOOL> SubroutineVisioModifyJudge;
        CString SubroutineCommandPretreatment;
	};
	//運行結構(副程式名稱、運行計數、控制主副程序、主副程序堆疊計數、動作狀態)  
	/*	
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
	//運行狀態讀取結構(運行狀態、目前命令進度、回原點狀態、程序完成計數、一次程序運行時間、運行迴圈狀態、單次命令運行狀態、畫布刷新關狀態)
	/*
    *運作狀態(0:未運作 1 : 運行中 2 : 暫停中)
	*回原點狀態(TRUE = 賦歸完成 FLASE = 賦歸中)
    *運行迴圈狀態(0:未運行、暫停中 1:執行中)
    *單次命令運行狀態(0:執行完畢、未執行 1:正在執行中)
    *畫布刷新關狀態(0:不關閉 1:關閉清除)//人機要求
	*/
	struct RunStatusRead {
		UINT RunStatus;
		UINT CurrentRunCommandNum;
		BOOL GoHomeStatus;
		int FinishProgramCount;
        DOUBLE RunTotalTime;
        BOOL RunLoopStatus;
        BOOL StepCommandStatus;
        BOOL PaintClearClose;
        //TODO::之後做總進度使用
        //BOOL RegistrationStatus;
        //UINT CommandTotalCount;
        //UINT CurrentRunCommandCount;
	};
	//阻斷控管結構(阻斷數量、阻斷陣列)
	struct StepRepeatBlockData {
		int BlockNumber;
		std::vector<CString> BlockPosition;
	};
	//Step&Loop控管結構(循環開關、循環地址紀錄、循環計數、步驟跳躍標籤、步驟跳躍開關、增加步驟內層迴圈開關、記錄步驟內層迴圈新增次數、記錄步驟內層迴圈刪除次數、記錄S型轉換開關、步驟地址紀錄、步驟初始offsetX紀錄、步驟初始offsetY紀錄、步驟計數X、步驟計數Y、記錄組斷資料)
	/*
	*循環開關:用來判別沒有此標籤時狀況
	*步驟跳躍標籤:用於執行迴圈時跳躍指令用
    *步驟跳躍開關:目的用於跳到StepRepeat最外層迴圈
	*增加步驟內層迴圈開關:用於判斷是否第二次新增內層迴圈
	*記錄步驟內層迴圈新增次數:記錄總共有幾個內層，用於判斷刪除後須新增最大數
	*記錄步驟內層迴圈刪除次數:記錄刪除掉幾個內層，用於必須在新增回來
	*記錄S型轉換開關:用於S型態判斷Offset需要加還減
	*步驟地址紀錄:用於判斷指令目前的StepRepeat是屬於哪一個陣列
	*記錄組斷資料:包含數量、阻段陣列的字串
	*/
	struct RepeatData {
		BOOL LoopSwitch;
		std::vector<UINT> LoopAddressNum;
		std::vector<UINT> LoopCount;
		CString StepRepeatLabel;
		BOOL StepRepeatLabelLock;
		BOOL AddInStepRepeatSwitch;
		int AllNewStepRepeatNum;
		int AllDeleteStepRepeatNum;
		std::vector<BOOL> SSwitch;
		std::vector<UINT> StepRepeatNum;
		std::vector<UINT> StepRepeatInitOffsetX;
		std::vector<UINT> StepRepeatInitOffsetY;
		std::vector<int> StepRepeatCountX;
		std::vector<int> StepRepeatCountY;
		std::vector<StepRepeatBlockData> StepRepeatBlockData;
	};
	/*程序循環運行結構(控制循環開關、循環次數、循環計數、最大運行次數(設 -1 =沒有限制))*/
	struct RunLoopData {      
		BOOL RunSwitch;
		int LoopNumber;
		int LoopCount;
		int MaxRunNumber;
	};
	
private:    //變數
	HANDLE          wakeEvent;
    LARGE_INTEGER   startTime, endTime, fre;
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
	//IO
	IOControl       IOControl;
    //未修正虛擬模擬座標
    CoordinateData  NVMVirtualCoordinateData;
   
	
private:    //函數
	//執行續
	static  UINT    HomeThread(LPVOID pParam);
	static  UINT    Thread(LPVOID pParam);
	static  UINT    SubroutineThread(LPVOID pParam);
	static  UINT    RunLoopThread(LPVOID pParam);
	static  UINT    IODetection(LPVOID pParam);
	//動作處理
	static  void    LineGotoActionJudge(LPVOID pParam);
	static  void    ModifyPointOffSet(LPVOID pParam, CString XYZPoint);
	static  void    VisionModify(LPVOID pParam);
	void            VisionFindMarkError(LPVOID pParam);
	static  void    LaserModify(LPVOID pParam);
	//阻斷處理
	static  void    BlockProcessStart(CString Command, LPVOID pParam, BOOL RepeatStatus);
	static  BOOL    BlockProcessExecute(CString Command, LPVOID pParam, int NowCount);
	static  void    BlockProcessStartY(CString Command, LPVOID pParam, BOOL RepeatStatus);
	static  BOOL    BlockProcessExecuteY(CString Command, LPVOID pParam, int NowCount);
	static  void    BlockSort(std::vector<CString> &BlockPosition, int Type, int mode);
	static  CString BlockResolve(CString String, UINT Choose);
	//命令處理
	static  CString CommandResolve(CString Command,UINT Choose);
	//初始化處理
	void            ParameterDefult();
	void            DecideInit();
	void            DecideClear();
	void            MainSubProgramSeparate();
	void            DecideBeginModel();
	//檔案處理
	BOOL            ListAllFileInDirectory(LPTSTR szPath, LPTSTR szName);
	static  BOOL    FileExist(LPCWSTR FilePathName);
	//模組控管處理
	static  void    RecordCorrectionTable(LPVOID pParam);
	static  void    ChooseVisionModify(LPVOID pParam);
	static  void    ChooseLaserModify(LPVOID pParam);
	//雷射處理
	static  void    LaserDetectHandle(LPVOID pParam, CString Command);
	//虛擬修正處理
	static  CString VirtualNowOffSet(LPVOID pParam , CString Command);
	//虛擬座標模擬
	static  void    VirtualCoordinateMove(LPVOID pParam, CString Command, LONG type);
    //單位轉換
    CString         CommandUnitConversinon(CString Command, DOUBLE multiple, DOUBLE Timemultiple);
	//其他功能(Demo用)
	static  void    SavePointData(LPVOID pParam);
    //CallSubroutin預處理尋找
    BOOL            SubroutinePretreatmentFind(LPVOID pParam);
    
	
public:     //變數
	//主運動物件
	CAction         m_Action;
	//動作總數
	LONG            ActionCount;
	LONG            V_ActionCount;
	//程序陣列
	std::vector<CString> CommandMemory;
    std::vector<CString> mmCommandMemory;
	//運動參數值
	Default         Default;

	DispenseDotSet  DispenseDotSet;
	DispenseDotEnd  DispenseDotEnd;
	DispenseLineSet DispenseLineSet;
	DispenseLineEnd DispenseLineEnd;
	Speed           DotSpeedSet, LineSpeedSet,MoveSpeedSet;
	ZSet            ZSet;
	GlueData        GlueData;
	GoHome          GoHome;

	//運行程序資料
	RunStatusRead   RunStatusRead;
	//RunLoop
	RunLoopData     RunLoopData;
	//影像參數 
	VisionDefault   VisionDefault;

	VisionSet       VisionSet;
	VisionFile      VisionFile;
	VisionSerchError VisionSerchError;

	//影像
	Vision          FindMark, FiducialMark1, FiducialMark2;
	VisionOffset    VisionOffset;
	VisionTrigger   VisionTrigger;
	//雷射
	LaserSwitch     LaserSwitch;
	LaserData       LaserData;
	//影像修正後工作座標
	CoordinateData  FinalWorkCoordinateData;
	//*虛擬模擬座標
	CoordinateData  VirtualCoordinateData;
	
	//模組控管
	ModelControl    ModelControl;

	//動作修正表
	std::vector<PositionModifyNumber> PositionModifyNumber;
	//影像資料
	std::vector<VisionAdjust> VisionAdjust;
	//雷射資料
	std::vector<LaserAdjust> LaserAdjust;

	//影像、雷射資料計數
	int             VisionCount;
	int             LaserCount;

	//紀錄目前紀錄到表中地址(特殊線段使用)
	CString         CurrentTableAddress;
	//(暫存)判斷影像是否修正
	BOOL            VisioModifyJudge;
	
	//連續線段計數(連續掃描使用)
	LONG            ContinuousLineCount;
	//控制是否切換連續線段(呼叫執行連續線段使用)
	BOOL            ContinuousSwitch;

	//Demo載入用判斷符號(DEMO 用)
	BOOL            DemoTemprarilySwitch;

    //IO參數設定
    IOParam         IOParam;

public:     //函數
	COrder();
	virtual ~COrder();
	//開始命令解譯(成功return 1失敗return 0)
	BOOL    Run();
	//連續開始命令解譯(參數:循環次數)(回傳值:成功 return 1 失敗 return 0)
	BOOL    RunLoop(int LoopNumber);
	//立即停止命令解譯(成功return 1失敗return 0)
	BOOL    Stop();
	//暫停命令解譯(成功return 1失敗return 0)
	BOOL    Pause();
	//繼續命令解譯(成功return 1失敗return 0)
	BOOL    Continue();
	//原點賦歸(參數:模式(FALSE 針頭 TRUE CCD))(回傳值:成功return  1 失敗 return 0 )
	BOOL    Home(BOOL mode);
	//View命令解譯(參數:模式(FALSE 針頭 TRUE CCD))(成功return 1 失敗 return 0)
	BOOL    View(BOOL mode);
	//I/O偵測執行續開(參數:開關(TRUE 開啟 FALSE 關閉),模式)
    BOOL    IODetectionSwitch(BOOL Switch, int mode);
	//載入檔案
	void    LoadPointData();
    
protected:
	DECLARE_MESSAGE_MAP()
};


