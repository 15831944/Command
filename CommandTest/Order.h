/*
*檔案名稱:Order.h
*檔案用途:3Axis
*檔案擁有功能:Move、Laser、Vision、Check
*適用軸卡:Nova
*適用雷射:松下
*更新日期:2017/02/21
*作者名稱:Rong
*/
#pragma once
#include <vector>
#include "Action.h"
#include "CInterpolationCircle.h"

static CWinThread* g_pThread = NULL;
static CWinThread* g_pSubroutineThread = NULL;
static CWinThread* g_pRunLoopThread = NULL;
static CWinThread* g_pIODetectionThread = NULL;
static CWinThread* g_pCheckCoordinateScanThread = NULL;
static CWinThread* g_pCheckActionThread = NULL;
static CWinThread* g_pMosaicDlgThread = NULL;
class COrder : public CWnd
{
	DECLARE_DYNAMIC(COrder)
private:
	/************************************************************運動參數結構*******************************************************/
	//座標結構(狀態、紀錄X、紀錄Y、紀錄Z)
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
	//清潔針頭結構(清潔裝置位置、清潔裝置開關、IOport)
	struct CleanerData {
		CoordinateData  CleanerPositionData;
		BOOL CleanerSwitch;
		LONG OutputPort;
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
		CleanerData     CleanerData;
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
	//影像LodaModel設置結構(修正模式、精確度、尋找速度、匹配值、搜尋範圍寬度、搜尋範圍高度、搜尋範圍起始角度、搜尋範圍結束角度、CCD針頭OffsetX、CCD針頭OffsetY、對焦高度)
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
		LONG FocusHeight;
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
	//Laser資料紀錄(雷射階段執行過地址、雷射單點測高座標、雷射線段平均測高開始和結束座標、雷射測高數據)
	struct LaserData {
		int LaserExecutedAddress;
		CoordinateData LaserHeightPoint;
		CoordinateData LaserDetectLS, LaserDetectLE;
		LONG LaserMeasureHeight;		
	};
	//Laser測高值紀錄陣列(雷射測高數據)
	struct LaserAdjust {
		LONG LaserMeasureHeight;
	};
	/*Laser連續線段控制參數(連續線段計數、連續線段開關)
	*連續線段開關:控制是否切換連續線段(呼叫執行連續線段使用)
	*/
	struct LaserContinuousControl {
		LONG ContinuousLineCount;
		BOOL ContinuousSwitch;
	};
	/************************************************************檢測參數結構*******************************************************/
	//檢測座標結構(檢測模式地址、命令地址、座標位置)
	struct CheckCoordinate{
		CString CheckMode;
		CString CheckModeAddress;
		CString Address;
		CoordinateData Position;
	};
	//模板開關結構(模板檢測、直徑檢測、區域檢測、運行檢測、即時檢測(0:不檢測1:模板2:直徑))
	struct CheckSwitch {
		BOOL Template;
		BOOL Diameter;
		BOOL Area;
		BOOL RunCheck;
		UINT ImmediateCheck;
	};
	//模板檢測結構(模板檢測地址、OK模板數量、OK模板指針陣列、NG模板數量、NG模板指針陣列、比對參數)
	struct TemplateCheck {
		CString Address;
		LONG OKModelCount;
		void** OKModel;
		LONG NGModelCount;
		void** NGModel;
		VisionSet   VisionParam;
	};
	//直徑檢測結構(模板檢測地址、直徑、容許誤差、色階、二值化界限值)
	struct DiameterCheck{
		CString Address;
		DOUBLE Diameter;
		DOUBLE Tolerance;
		BOOL Color;
		DOUBLE Binarization;
	};
	//紀錄檢測結果數量結構(OK數量、NG數量、都否數量、OK重組圖、NG重組圖)
	struct CheckResult {
		LONG OKCount;
		LONG NGCount;
		LONG Error;
		LONG OKMosaicingImage;
		LONG NGMosaicingImage;
	};
	//紀錄完成點檢測結構(檢測結果字串,檢測座標結構{檢測模式地址,檢測地址,檢測座標})
	struct CheckFinishRecord {
		CString Result;
		CheckCoordinate CheckData;
	};
	//檢測設置目標結構(檢測結果目標數量、目標達成動作)
	struct CheckSetGoal{
		CheckResult Goal;
		LONG Action;
	};
	//檔案結構(檔案路徑、檔案名稱)
	struct File{
		CString Path;
		CString Name;
	};
	//訓練結構(儲存檔案結構、點資料、測量限制(直徑、線寬)、容許誤差量、色階、二值化界限值)
	struct TrainData {
		File TrainSave;
		std::vector<CPoint> PointData;
		DOUBLE MeasureLimit;
		DOUBLE MaxOffset;
		BOOL WhiteOrBlack;
		DOUBLE Threshold;
	};
	//重組結構(儲存檔案結構、視野移動量、起始位置、結束位置、移動數量X,移動數量Y)
	struct MosaicingData {
		File ImageSave;
		POINT ViewMove;
		POINT Start;
		POINT End;
		int MoveCountX;
		int MoveCountY;
	};
	//區域檢測結構(區域檢測地址、重組圖結構、點訓練結構、線訓練結構、儲存結果檔案結構)
	struct AreaCheck {
		CString Address;
		MosaicingData Image;
		TrainData DotTrain;
		TrainData LineTrain;
		File Result;     
	};
	//紀錄完成區域檢測結果結構(檢測模式地址、檢測結果檔案結構、檢測結果)
	struct AreaCheckFinishRecord {
		CString CheckModeAddress;
		File ResultImage;
		BOOL Result;
	};
	//區域檢測人機預設值結構(視野移動量、重組圖儲存路徑、點訓練儲存路徑、線訓練儲存路徑、結果圖儲存路徑、重組中對話窗)
	struct AreaCheckParamterDefault {
		POINT ViewMove;
		File ImageSave;
		File DotTrainSave;
		File LineTrainSave;
		File Result;
        CDialog* pMosaicDlg;
	};
	/************************************************************模組參數結構*******************************************************/
	//控管模組結構(模式選擇、模式轉換地址、影像模組跳過、雷射和檢測模組跳過)
	struct ModelControl{
		UINT Mode;
		int  ModeChangeAddress;
		BOOL VisionModeJump;
		BOOL LaserAndCheckModeJump;
	};
	/************************************************************程序參數結構*******************************************************/
	//區間資料結構(類別、起始地址、結束地址)
	struct IntervalData {
		CString Type;
		UINT BeginAddress;
		UINT EndAddress;
	};
	/*Label&Subroutine控管結構(標籤計數、標籤名稱、子程序計數、子程序狀態、子程序地址堆疊、子程序座標堆疊、子程序模式堆疊、子程序影像修正判斷堆疊、子程序命令預處理、子程序模組跳換開關) 
	*標籤計數:用來計算進入標籤時命令是否全部尋找完畢
	*標籤名稱:用來記錄跳要的標名稱
	*子程序計數:用來計數目前有呼叫幾個子程序
	*子程序地址堆疊:用來記錄呼叫子程序時的命令地址
	*子程序座標堆疊:用來記錄呼叫子程序時的機械手臂位置
	*子程序模式堆疊:用來記錄呼叫子程序時目前命令運作的模式(ex:1影像2雷射...
	*子程序影像修正判斷堆疊:控制子程序內的命令是否做影像修正
	*子程序命令預處理:用來儲存和目前位置做相對量修正的命令
	*子程序模組跳換開關:用來防止雷射關閉強行跳出子程序和讓之後命令失效只剩EndSubroutine有效
	*/
	struct Program {
		int LabelCount;
		CString LabelName;
		UINT SubroutinCount;
		BOOL CallSubroutineStatus;
		std::vector<UINT> SubroutineStack;
		std::vector<CoordinateData> SubroutinePointStack;
		std::vector<UINT> SubroutineModel;
		std::vector<BOOL> SubroutineVisioModifyJudge;
		CString SubroutineCommandPretreatment;
		BOOL SubroutineModelControlSwitch;
	};
	/*運行結構(副程式名稱、運行計數、控制主副程序、主副程序堆疊計數、動作狀態)	
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
	/*運行狀態讀取結構(運行狀態、目前命令進度、回原點狀態、程序完成計數、一次程序運行時間、運行迴圈狀態、單次命令運行狀態、畫布刷新關狀態、重組狀態)
	*運作狀態(0:未運作 1 : 運行中 2 : 暫停中)
	*回原點狀態(TRUE = 賦歸完成 FLASE = 賦歸中)
	*運行迴圈狀態(0:未運行、暫停中 1:執行中)
	*單次命令運行狀態(0:執行完畢、未執行 1:正在執行中)
	*畫布刷新關狀態(0:不關閉 1:關閉清除)//人機要求
    *重組狀態(-1:重組中 0:重組失敗 1:重組成功 2:未重組)
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
        int MosaicStatus;
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
	/*Step&Loop控管結構(循環開關、循環地址紀錄、循環計數、步驟跳躍標籤、步驟跳躍開關、步驟標籤起始地址、增加步驟內層迴圈開關、記錄步驟內層迴圈新增次數、記錄步驟內層迴圈
	
	次數、記錄S型轉換開關、步驟地址紀錄、步驟初始offsetX紀錄、步驟初始offsetY紀錄、步驟計數X、步驟計數Y、記錄組斷資料、StepRepeat區間堆疊)
	*循環開關:用來判別沒有此標籤時狀況
	*步驟跳躍標籤:用於執行迴圈時跳躍指令用
	*步驟跳躍開關:目的用於跳到StepRepeat最外層迴圈
	*步驟標籤起始地址:用來記錄當前StepRepeatLabel地址
	*增加步驟內層迴圈開關:用於判斷是否第二次新增內層迴圈
	*記錄步驟內層迴圈新增次數:記錄總共有幾個內層，用於判斷刪除後須新增最大數
	*記錄步驟內層迴圈刪除次數:記錄刪除掉幾個內層，用於必須在新增回來
	*記錄S型轉換開關:用於S型態判斷Offset需要加還減
	*步驟地址紀錄:用於判斷指令目前的StepRepeat是屬於哪一個陣列
	*記錄組斷資料:包含數量、阻段陣列的字串
	*StepRepeat區間堆疊:記錄StepRepeat的工作區間
	*/
	struct RepeatData {
		BOOL LoopSwitch;
		std::vector<UINT> LoopAddressNum;
		std::vector<UINT> LoopCount;
		CString StepRepeatLabel;
		CString StepRepeatLabelPrevious;
		BOOL StepRepeatLabelLock;
		UINT StepRepeatAddress;
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
		std::vector<IntervalData> StepRepeatIntervel;
	};
	//程序循環運行結構(控制循環開關、循環次數、循環計數、最大運行次數(設 -1 =沒有限制))
	struct RunLoopData {      
		BOOL RunSwitch;
		int LoopNumber;
		int LoopCount;
		int MaxRunNumber;
	};
	//外部函式呼叫結構
	typedef void(*CDrawFunction) (void*,CPoint,int);
	struct CallFunction {
		CDrawFunction CDrawFunction;
		void* pObject;
	};
private:    //變數
	HANDLE          wakeEvent;
	/*運行時間計算*/
	LARGE_INTEGER   startTime, endTime, fre;
	/*代換暫存變數*/ 
	StepRepeatBlockData InitBlockData;
	CoordinateData  InitData;
	/*命令*/
	CString         Commanding;
	std::vector<CString> CommandSwap;
	std::vector<std::vector<CString>> Command;
	/*程序*/
	RepeatData      RepeatDataRecord;
	RepeatData      RepeatData;
	Program         Program;
	RunData         RunData;
	/*模組控管*/
	ModelControl    ModelControl;
	/*狀態*/
	std::vector<CoordinateData> ArcData, CircleData1, CircleData2, StartData, OffsetData;
	/*IO*/
	IOControl       IOControl;
	/*未修正虛擬模擬座標(目前用在拿來計算Subroutine偏差量)*/
	CoordinateData  NVMVirtualCoordinateData; 
	/*系統移動速度*/
	Speed           MoveSpeedSet,LMPSpeedSet,LMCSpeedSet,VMSpeedSet;
	/*(暫存)判斷影像是否修正(Subroutine使用)*/
	BOOL            VisioModifyJudge;
	
private:    //函式
	/*執行續*/
	static  UINT    HomeThread(LPVOID pParam);//原點賦歸程序
	static  UINT    Thread(LPVOID pParam);//主程序
	static  UINT    SubroutineThread(LPVOID pParam);//命令動作程序
	static  UINT    RunLoopThread(LPVOID pParam);//運行迴圈程序
	static  UINT    IODetection(LPVOID pParam);//IO偵測程序
	static  UINT    CheckCoordinateScan(LPVOID pParam);//區間檢測控制程序
	static  UINT    CheckAction(LPVOID pParam);//區間檢測執行程序
    static  UINT    MosaicDlg(LPVOID pParam);
	/*動作處理*/
	static  void    LineGotoActionJudge(LPVOID pParam);//判斷線段動作轉換
	static  void    ModifyPointOffSet(LPVOID pParam, CString XYZPoint);//CallSubroutin修正處理
	static  CString VirtualNowOffSet(LPVOID pParam, CString Command);//虛擬座標計算偏差值
	static  void    VisionModify(LPVOID pParam);//影像修正
	void            VisionFindMarkError(LPVOID pParam);//影像未找到處理方法
	static  void    LaserModify(LPVOID pParam);//雷射修正
	static  void    LaserDetectHandle(LPVOID pParam, CString Command);//雷射檢測處理
	BOOL            LaserPointDetect();//檢查雷射檢測點是否重複
	static  void    VirtualCoordinateMove(LPVOID pParam, CString Command, LONG type);//虛擬座標移動
	BOOL            CheckDraw();//點檢測畫圖
    static  void    PassingException(LPVOID pParam);//中間點例外處理(暫停、執行區域檢測)
	//虛擬座標模擬移動
	/*資料表處理區塊*/
	static  void    ChooseVisionModify(LPVOID pParam);//選擇影像修正值
	static  void    ChooseLaserModify(LPVOID pParam);//選擇雷射修正值
	static  void    RecordCorrectionTable(LPVOID pParam);//記錄運動修正表
	/*命令處理*/
	static  CString CommandResolve(CString Command, UINT Choose);//命令分解
	static  CString ModelNumResolve(CString ModelNum, UINT Choose);//模版編號分解
	CString         CommandUnitConversinon(CString Command, DOUBLE multiple, DOUBLE Timemultiple);//命令單位轉換
	CString         GetCommandAddress();//獲取編碼過命令地址
	/*程序變數處理區塊*/
	void            ParameterDefult();//運動參數初始化
	void            DecideInit();//程序初始化
	void            DecideClear();//程序結束清除
	void            MainSubProgramSeparate();//劃分主副程序
	void            DecideBeginModel(BOOL ViewMode);//判斷起始模組
	/*檔案處理*/
	BOOL            ListAllFileInDirectory(LPTSTR szPath, LPTSTR szName);//搜尋目錄檔案
	static  BOOL    FileExist(LPCWSTR FilePathName);
	static  BOOL    FilePathExist(CString FilePathName);
	static  CString GetDataFileName();
	static  BOOL    FileDelete(CString FilePathName);
	static  BOOL    SearchDirectory(CString FilePathName);
	//判斷檔案是否存在
	/*StepRepeat處理*/
	BOOL            SubroutinePretreatmentFind(LPVOID pParam);//CallSubroutin預處理尋找
	static  void    StepRepeatJumpforciblyJudge(LPVOID pParam, UINT Address);//StepRepeat強行跳轉判斷
	/*阻斷處理*/
	static  void    BlockProcessStartX(CString Command, LPVOID pParam, BOOL RepeatStatus);
	static  BOOL    BlockProcessExecuteX(CString Command, LPVOID pParam, int NowCount);
	static  void    BlockProcessStartY(CString Command, LPVOID pParam, BOOL RepeatStatus);
	static  BOOL    BlockProcessExecuteY(CString Command, LPVOID pParam, int NowCount);
	static  void    BlockSort(std::vector<CString> &BlockPosition, int Type, int mode);
	static  CString BlockResolve(CString String, UINT Choose);
	/*檢測處理*/
	static  void    ModelLoad(BOOL Choose, LPVOID pParam, CString ModelNum, TemplateCheck &TemplateCheck);
	BOOL            ClearCheckData(BOOL Moment, BOOL Interval);
	static  void    LineTrainDataCheck(LPVOID pParam);
	BOOL            NewCutPathPoint(CoordinateData Start, CoordinateData Passing, CoordinateData End, AreaCheck &IntervalAreaCheck, int Type);//新增點成功回傳1 失敗回傳0
    BOOL            PointAreaJudge(POINT Point, CRect Area);//判斷區域內回傳1 外回傳0
    BOOL            LineAreaJudge(POINT PointS, POINT PointE, CRect Area);//判斷區域內回傳1 外回傳0
    BOOL            ArcAreaJudge(POINT PointS, POINT PointA, POINT PointE, CRect Area);//判斷區域內回傳1 外回傳0
    BOOL            CircleAreaJudge(POINT PointS, POINT PointC1, POINT PointC2, CRect Area);//判斷區域內回傳1 外回傳0
    BOOL            AutoCalculationArea(AreaCheck &AreaCheckRun);//自動計算重組區域大小
    CRect           MosaicAreaJudge(AreaCheck &AreaCheck);//判斷重組區域是否為同一點
	/*其他功能(Demo用)*/
	static  void    SavePointData(LPVOID pParam);

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

	/*測試用暫時放在public*/
	DispenseDotSet  DispenseDotSet;
	DispenseDotEnd  DispenseDotEnd;
	DispenseLineSet DispenseLineSet;
	DispenseLineEnd DispenseLineEnd;   
	Speed           DotSpeedSet, LineSpeedSet;
	ZSet            ZSet;
	GlueData        GlueData;
	GoHome          GoHome;
	CleanerData     CleanerData;
	/********************/

	//運行程序資料
	RunStatusRead   RunStatusRead;
	//RunLoop
	RunLoopData     RunLoopData;
	//影像參數
	VisionDefault   VisionDefault;

	/*測試用暫時放在public*/
	VisionSet       VisionSet;
	VisionFile      VisionFile;
	VisionSerchError VisionSerchError;
	//影像資料
	Vision          FindMark, FiducialMark1, FiducialMark2;
	VisionOffset    VisionOffset;
	VisionTrigger   VisionTrigger;
	//雷射資料
	LaserSwitch     LaserSwitch;
	LaserData       LaserData;
	LaserContinuousControl LaserContinuousControl;
	/*影像雷射修正後工作座標*/
	CoordinateData  FinalWorkCoordinateData;
	/*虛擬模擬座標 (目前只用在CallSubroutine回歸點)*/
	CoordinateData  VirtualCoordinateData;
	
	//動作修正表
	std::vector<PositionModifyNumber> PositionModifyNumber;
	//影像修正資料
	std::vector<VisionAdjust> VisionAdjust;
	//雷射修正資料
	std::vector<LaserAdjust> LaserAdjust;
	//影像、雷射資料計數
	int             VisionCount;//用來計數第幾次的影像對位
	int             LaserCount;//用來計數第幾次的雷射測高
	//檢測資料                                                     
	CheckSwitch     CheckSwitch;
	CString         CurrentCheckAddress;//存放目前檢測地址(新增時用來判斷是否有重複的檢測區間資料和用於點資訊區間檢測資料的新增)
	TemplateCheck   TemplateChecking;
	std::vector<TemplateCheck> IntervalTemplateCheck;
	DiameterCheck   DiameterChecking;
	std::vector<DiameterCheck> IntervalDiameterCheck;
	std::vector<CheckCoordinate> IntervalCheckCoordinate;//直徑、模板區間檢測所有點資料
	std::vector<AreaCheck> IntervalAreaCheck;
	//檢測運行判斷資料
	CoordinateData  AreaCheckChangTemp;//區域中間點轉換檢測暫存參數
	CheckCoordinate CheckCoordinateRun;//暫存目前要運行的檢測點座標
	AreaCheck       AreaCheckRun;//暫存目前要運行的區域檢測資料
	LONG            CheckModel;//檢測模組判斷

	//畫圖呼叫函式設定
	CallFunction    CallFunction;

    //紀錄例外中間點
    CoordinateData  PassingExceptionTemp;//紀錄中間點
	/********************/

	//區域檢測預設值參數
	AreaCheckParamterDefault AreaCheckParamterDefault;
	//檢測結果存放
	std::vector<CheckFinishRecord> CheckFinishRecord;//檢測結果點資訊
	std::vector<AreaCheckFinishRecord> AreaCheckFinishRecord;//區域檢測結果圖資訊
	CheckResult     CheckResult;//檢測結果數量
	
	//紀錄目前紀錄到表中地址(雷射使用)(連續特殊線段使用)
	CString         CurrentTableAddress;
	//Demo載入用判斷符號(DEMO 用)
	BOOL            DemoTemprarilySwitch;
	//IO參數設定
	IOParam         IOParam;
   
public:     //運行類函式
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
	//I/O偵測執行續開(參數:開關(TRUE 開啟 FALSE 關閉),模式:目前尚未有功能)
	BOOL    IODetectionSwitch(BOOL Switch, int mode);
	//載入檔案
	void    LoadPointData();
	//檢查命令規則(return 錯誤代碼 , ErrorAddress 為錯誤命令地址)
	int     CheckCommandRule(int &ErrorAddress);
public:    //設定類函式
	//設置畫圖呼叫函式(成功return 1失敗return 0)
	BOOL    SetDrawFunction(CDrawFunction Funtion, void* pObject);
	//設置平台高度Z值(參數:回傳馬達Z值)(成功return 1 失敗 return 0)
	BOOL    SetTabelZ(int* TableZ);
	
protected:
	DECLARE_MESSAGE_MAP()
};


