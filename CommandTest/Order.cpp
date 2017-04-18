/*
*檔案名稱:Order.cpp
*檔案用途:4Axis
*檔案擁有功能:Move、Laser、Vision、Check
*適用軸卡:Nova
*適用雷射:松下
*更新日期:2017/04/07
*作者名稱:Rong
*/
#include "stdafx.h"
#include "Order.h"
#include <math.h>

#define VI_MICaptureDelayTime 200 //用於重組拍照停留時間 
#define VI_DCheckDelayTime 400 //用於直徑檢測影像停留時間
#define VI_TCheckDelayTime 400 //用於模板檢測拍照的停留時間
#define VI_AutoCalculationMosaicAddOffset 1000 //用於自動抓取重組區大小增加量
#define VI_MosaicAreaDefault -1000 //用於自動抓取重組區大小判斷值

// COrder

IMPLEMENT_DYNAMIC(COrder, CWnd)

COrder::COrder()
{
	//wakeEvent = NULL;
	//系統移動速度
	MoveSpeedSet = { 100000,30000,2000 };
	LMPSpeedSet = { 100000,30000,2000 };
	LMCSpeedSet = { 10000,5000,2000 };
	VMSpeedSet = { 100000,30000,2000 };
	//運動參數
	DispenseDotSet = { 0,0 };
	DispenseDotEnd = { 0,0,0 };
	DispenseLineSet = { 0,0,0,0,0,0 };
	DispenseLineEnd = { 0,0,0,0,0 };
	DotSpeedSet = {0,0,1000};
	LineSpeedSet = {0,0,2000};
	ZSet = {0,0}; 
	GlueData = { {0,0,0,0,0},0,0,0,0 };
	//影像參數
	VisionOffset = { { 0,0,0,0,0 },0,0,0 };
	VisionSet = { 0,0,0,0,0,0,0,0,0,0 };
	VisionDefault.VisionSerchError.Manuallymode = FALSE;
	VisionDefault.VisionSerchError.Pausemode = FALSE;
	//雷射參數
	LaserSwitch = { 0,0,0,0,0 };
	//運行狀態
	RunStatusRead = { 0,0,1,0,0,0,0,0,0 };
	RunLoopData = { 0,0,0,-1 };
	//虛擬位置
	VirtualCoordinateData = { 0,0,0,0,0 };
	//模組控制參數
	ModelControl = { 0,0,0,0 };
	//IO參數
	IOControl = { 0,0 };
	IOParam = { NULL };
	//檢測參數
	CheckResult = { 0,0,0 };
	CheckSwitch = { 0,0,0,0,0 };
	//區域檢測檔案名稱
	AreaCheckParamterDefault.ImageSave.Name = L"Image";
	AreaCheckParamterDefault.DotTrainSave.Name = L"DotTrain";
	AreaCheckParamterDefault.LineTrainSave.Name = L"LineTrain";
	//檢測畫圖參數初始化
	CallFunction.pObject = NULL;
	//載入命令參數初始化
	LoadCommandData.FirstRun = TRUE;
	/****************************DemoTemprerily*******************************/
	DemoTemprarilySwitch = FALSE;
}
COrder::~COrder()
{
	//釋放影像詢問視窗
	if (VisionDefault.VisionSerchError.pQuestion != NULL)
		delete VisionDefault.VisionSerchError.pQuestion;
	//釋放影像檔案目錄指針
	if (VisionDefault.VisionFile.ModelPath != NULL)
	{
		delete VisionDefault.VisionFile.ModelPath;//釋放目錄指針記憶體
	}
	//釋放緊急停止彈跳視窗
	if (IOParam.pEMGDlg != NULL)
		delete IOParam.pEMGDlg;
	//釋放重組中視窗
	if (AreaCheckParamterDefault.pMosaicDlg != NULL)
		delete AreaCheckParamterDefault.pMosaicDlg;
}
BEGIN_MESSAGE_MAP(COrder, CWnd)
END_MESSAGE_MAP()
// COrder 訊息處理常式
/**************************************************************************函數動作區塊***************************************************************************/
/*開始*/
BOOL COrder::Run()
{
	if (!g_pThread) {
		Commanding = _T("Start");
		if (!CommandMemory.empty())
		{
			//時間計時開始
			QueryPerformanceFrequency(&fre); //取得CPU頻率
			QueryPerformanceCounter(&startTime); //取得開機到現在經過幾個CPU Cycle
			/*****可以不用在檢查ㄧ次命令*****/
			int Test = 0;
			if (CheckCommandRule(Test))//檢查命令表
			{
#ifdef PRINTF
				_cwprintf(L"Run()::錯誤代碼:%d,命令地址為:%d\n", CheckCommandRule(Test), Test);
#endif
				return FALSE;
			}
			//刪除檢測結果檔案
			//FileDelete(AreaCheckParamterDefault.Result.Path);
			int pos = AreaCheckParamterDefault.Result.Path.ReverseFind('\\');
			SearchDirectory(AreaCheckParamterDefault.Result.Path.Left(pos));
			//針頭模式 不減offset
			VisionDefault.VisionSet.ModifyMode = FALSE;
			//參數設定為預設
			ParameterDefult();
			//劃分主副程序
			MainSubProgramSeparate();
			//判斷初始化模組選擇
			DecideBeginModel(FALSE);
			//狀態初始化
			DecideInit();
			//載入所有檔案名
			ListAllFileInDirectory(VisionFile.ModelPath, TEXT("*_*_*_*_*.mod"));
			//出膠控制器模式
			m_Action.m_bIsDispend = TRUE;
			//判斷是否循環執行
			if (g_pRunLoopThread)
			{
				::ResetEvent(ThreadEvent.RunLoopThread);//釋放事件
			}
			//開啟執行緒
			g_pThread = AfxBeginThread(Thread, (LPVOID)this);
			//wakeEvent = CreateEvent(NULL, TRUE, FALSE, NULL);//創建事件(測試用)
			return TRUE;
		}
	}
	return FALSE;
}
/*循環開始*/
BOOL COrder::RunLoop(int LoopNumber) {
	if (LoopNumber > 0 && !g_pRunLoopThread && !g_pThread)
	{
		RunLoopData.RunSwitch = TRUE;
		RunLoopData.LoopNumber = LoopNumber;
		RunLoopData.LoopCount = 0;  
		//初始化事件
		ThreadEvent.RunLoopThread = ::CreateEvent(NULL, TRUE, TRUE, L"MY_RUNTHREAD_EVT");
		g_pRunLoopThread = AfxBeginThread(RunLoopThread, (LPVOID)this);
		return TRUE;
	}
	return FALSE;
}
/*停止*/
BOOL COrder::Stop()
{
	RunLoopData.RunSwitch = FALSE;//關閉循環
	if (g_pThread)//判斷是否有在運作
	{
		if (SuspendThread(g_pThread) != GetLastError())
		{
			m_Action.m_bIsStop = TRUE;
			m_Action.m_bIsPause = FALSE;
			g_pThread->ResumeThread();//啟動線程
			if (g_pCheckCoordinateScanThread)//判斷檢測是否有在運作
			{
				if (SuspendThread(g_pCheckCoordinateScanThread) != GetLastError())//判斷是否在區間檢測
				{
					g_pCheckCoordinateScanThread->ResumeThread();//啟動線程
				}
			}
			#ifdef MOVE
				MO_DecSTOP();//立即減速停止運動指令
			#endif
			/*測試用*/
			//SetEvent(wakeEvent);//設置事件
			//WaitForSingleObject(g_pThread->m_hThread, INFINITE);//等待线程安全返回
			//CloseHandle(wakeEvent);//關閉事件句柄
			return TRUE;
		}
		else
		{
			m_Action.m_bIsStop = TRUE;
			#ifdef MOVE
				MO_DecSTOP();//立即減速停止運動指令
			#endif
			return TRUE;
		}
	}
	return FALSE;
}
/*暫停*/
BOOL COrder::Pause()
{
	if (g_pThread && RunStatusRead.RunStatus == 1 && RunStatusRead.GoHomeStatus) {
		m_Action.m_bIsPause = TRUE;
		g_pThread->SuspendThread();//暫停線程
		if (g_pCheckCoordinateScanThread)//判斷是否在區間檢測
		{
			g_pCheckCoordinateScanThread->SuspendThread();//暫停線程
		}
		RunStatusRead.RunStatus = 2;//狀態改變成暫停中
		m_Action.m_bIsPause = TRUE;
		if (g_pRunLoopThread && RunStatusRead.RunLoopStatus)
		{
			RunStatusRead.RunLoopStatus = FALSE;
		}		
		return TRUE;
	}
	return FALSE;
}
/*繼續*/
BOOL COrder::Continue()
{
	if (g_pThread)//判斷是否有在運作
	{
		if (SuspendThread(g_pThread) != GetLastError() && RunStatusRead.RunStatus == 2 && RunStatusRead.GoHomeStatus)
		{
			m_Action.m_bIsPause = FALSE;
			g_pThread->ResumeThread();//啟動線程
			if (g_pCheckCoordinateScanThread)//判斷檢測是否有在運作
			{
				if (SuspendThread(g_pCheckCoordinateScanThread) != GetLastError())//判斷是否在區間檢測
				{
					g_pCheckCoordinateScanThread->ResumeThread();//啟動線程
				}
			}
			else
			{
				AreaCheckChangTemp.Status = TRUE;//開啟
			}
			RunStatusRead.RunStatus = 1;//狀態改變成運作中
			if (g_pRunLoopThread && !RunStatusRead.RunLoopStatus)
			{
				RunStatusRead.RunLoopStatus = TRUE;
			}
			return TRUE;
		}
	}
	return FALSE;
}
/*原點賦歸*/
BOOL COrder::Home(BOOL mode)
{
	if (!g_pThread)
	{
		//將停止狀態清除
		m_Action.m_bIsStop = FALSE;
		//確定GoHome參數賦值
		GoHome = Default.GoHome;
		//回原點模式設定
		GoHome.VisionGoHome = mode;
		//回歸狀態設為FALSE
		RunStatusRead.GoHomeStatus = FALSE;
		g_pThread = AfxBeginThread(HomeThread, (LPVOID)this);  
		return TRUE;
	}
	return FALSE;
}
/*View查看*/
BOOL COrder::View(BOOL mode)
{
	if (!g_pThread) {
		Commanding = _T("Start");
		if (!CommandMemory.empty())
		{
			/*****可以不用在檢查ㄧ次命令*****/
			int Test = 0;
			if (CheckCommandRule(Test))//檢查命令表
			{
#ifdef PRINTF
				_cwprintf(L"View()::錯誤代碼:%d,命令地址為:%d\n", CheckCommandRule(Test), Test);
#endif
				return FALSE;
			}
			//針頭模式 不減offset 或 CCD模式 減offset
			VisionDefault.VisionSet.ModifyMode = mode;
			//參數設定為預設
			ParameterDefult();
			//劃分主副程序
			MainSubProgramSeparate();
			//判斷初始化模組選擇
			DecideBeginModel(TRUE);
			//狀態初始化
			DecideInit();
			//載入所有檔案名
			ListAllFileInDirectory(VisionFile.ModelPath, TEXT("*_*_*_*_*.mod"));
			//出膠控制器模式
			m_Action.m_bIsDispend = FALSE;
			//開啟執行緒
			g_pThread = AfxBeginThread(Thread, (LPVOID)this);

			//wakeEvent = CreateEvent(NULL, TRUE, FALSE, NULL);//創建事件(測試用)
			return TRUE;
		}
	}
	return FALSE;
}
/*I/O偵測執行緒開*/
BOOL COrder::IODetectionSwitch(BOOL Switch, int mode)
{
	if (Switch)
	{
		if (!g_pIODetectionThread)
		{
			IOControl.SwitchInformation = TRUE;
			g_pIODetectionThread = AfxBeginThread(IODetection, (LPVOID)this);
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
	else
	{
		if (!g_pIODetectionThread)
		{
			return FALSE;
		}
		else
		{
			IOControl.SwitchInformation = FALSE;
			return TRUE;   
		}
	}
}
/**************************************************************************執行續動作區塊*************************************************************************/
/*重複運行執行續*/
UINT COrder::RunLoopThread(LPVOID pParam) {
	((COrder*)pParam)->RunStatusRead.RunLoopStatus = TRUE;
	((COrder*)pParam)->RunStatusRead.PaintClearClose = FALSE;
	while (((COrder*)pParam)->RunLoopData.RunSwitch)   
	{
#ifdef PRINTF
		//_cprintf("RunLoopThread():%d\n", (((COrder*)pParam)->RunLoopData.MaxRunNumber - int(((COrder*)pParam)->RunStatusRead.FinishProgramCount)));
#endif
#ifdef MOVE
		//if (!MO_ReadEMG())
		//{
#endif
			if (((COrder*)pParam)->RunLoopData.LoopCount == (((COrder*)pParam)->RunLoopData.LoopNumber - 1) ||
				(((COrder*)pParam)->RunLoopData.MaxRunNumber - int(((COrder*)pParam)->RunStatusRead.FinishProgramCount)) > 1)
			{
				((COrder*)pParam)->RunStatusRead.PaintClearClose = TRUE;
			}
			if (((COrder*)pParam)->RunLoopData.MaxRunNumber >= 0 && (((COrder*)pParam)->RunLoopData.MaxRunNumber - int(((COrder*)pParam)->RunStatusRead.FinishProgramCount)) > 0)
			{
				if (((COrder*)pParam)->RunLoopData.LoopCount == ((COrder*)pParam)->RunLoopData.LoopNumber)
				{
					((COrder*)pParam)->RunLoopData.RunSwitch = FALSE;
				}
				else if (((COrder*)pParam)->RunStatusRead.RunStatus == 0 && g_pThread == NULL)
				{
					if (((COrder*)pParam)->Run()) //成功運行
					{
						::WaitForSingleObject(((COrder*)pParam)->ThreadEvent.RunLoopThread, INFINITE);//等待事件被設置
					}
					else//運行開啟失敗
					{
						((COrder*)pParam)->RunLoopData.RunSwitch = FALSE;
					}
				}
			}
			else if (((COrder*)pParam)->RunLoopData.MaxRunNumber < 0 && ((COrder*)pParam)->RunLoopData.LoopCount != ((COrder*)pParam)->RunLoopData.LoopNumber)
			{
				if (((COrder*)pParam)->RunStatusRead.RunStatus == 0 && g_pThread == NULL)
				{
					if (((COrder*)pParam)->Run()) {//成功運行
						::WaitForSingleObject(((COrder*)pParam)->ThreadEvent.RunLoopThread, INFINITE);//等待事件被設置
					}
					else//運行開啟失敗
					{
						((COrder*)pParam)->RunLoopData.RunSwitch = FALSE;
					}
				}
			}
			else if (((COrder*)pParam)->RunLoopData.LoopCount == ((COrder*)pParam)->RunLoopData.LoopNumber)
			{
				((COrder*)pParam)->RunLoopData.RunSwitch = FALSE;
			}
			else
			{
				((COrder*)pParam)->RunLoopData.RunSwitch = FALSE;
			}
#ifdef MOVE
		//}
#endif
	}
	((COrder*)pParam)->RunStatusRead.RunLoopStatus = FALSE;
	::CloseHandle(((COrder*)pParam)->ThreadEvent.RunLoopThread);//關閉事件
	g_pRunLoopThread = NULL;
	return 0;
}
/*原點賦歸執行緒*/
UINT COrder::HomeThread(LPVOID pParam)
{
#ifdef MOVE
	//先移動到(0,0,0)位置 Z軸抬升目前35000絕對 
	//((COrder*)pParam)->m_Action.DecideVirtualHome(0, 0, 0, 35000, 0,
	//	((COrder*)pParam)->MoveSpeedSet.EndSpeed, ((COrder*)pParam)->MoveSpeedSet.AccSpeed, ((COrder*)pParam)->MoveSpeedSet.InitSpeed);
	//((COrder*)pParam)->m_Action.DecideInitializationMachine(((COrder*)pParam)->GoHome.Speed1, ((COrder*)pParam)->GoHome.Speed2, ((COrder*)pParam)->GoHome.Axis, ((COrder*)pParam)->GoHome.MoveX, ((COrder*)pParam)->GoHome.MoveY, ((COrder*)pParam)->GoHome.MoveZ, ((COrder*)pParam)->GoHome.MoveW);
	((COrder*)pParam)->m_Action.W_NeedleGoHoming(((COrder*)pParam)->GoHome.Speed1, ((COrder*)pParam)->GoHome.Speed2);
	/*****有機會產生死結*****/
	while (((COrder*)pParam)->m_Action.m_IsHomingOK) {
		Sleep(10);//while 程式負載問題 無限迴圈，並讓 CPU 休息一下
	}
	if (((COrder*)pParam)->GoHome.VisionGoHome)//做完賦歸移動位置
	{
		((COrder*)pParam)->VisionSet.AdjustOffsetX = ((COrder*)pParam)->VisionDefault.VisionSet.AdjustOffsetX;
		((COrder*)pParam)->VisionSet.AdjustOffsetY = ((COrder*)pParam)->VisionDefault.VisionSet.AdjustOffsetY;
		((COrder*)pParam)->m_Action.DecideVirtualPoint(-(((COrder*)pParam)->VisionSet.AdjustOffsetX), -(((COrder*)pParam)->VisionSet.AdjustOffsetY), 0, 0,
			((COrder*)pParam)->MoveSpeedSet.EndSpeed, ((COrder*)pParam)->MoveSpeedSet.AccSpeed, ((COrder*)pParam)->MoveSpeedSet.InitSpeed);
	}
#endif
	((COrder*)pParam)->RunStatusRead.GoHomeStatus = TRUE;//原點賦歸完成
	g_pThread = NULL;
	return 0;
}
/*主執行緒*/
UINT COrder::Thread(LPVOID pParam)
{
#ifdef PRINTF
	(((COrder*)pParam)->DemoTemprarilySwitch) ? _cwprintf(L"Thread()::有載入Demo檔\n") : _cwprintf(L"Thread()::沒有載入Demo檔\n");
#endif
	((COrder*)pParam)->RunStatusRead.RunStatus = 1;//狀態改變成運作中
#ifdef MOVE
	//運行前先抬升至工件高度上
	((COrder*)pParam)->m_Action.DecideLineEndMove(0, 0, ((COrder*)pParam)->ZSet.ZBackHeight, ((COrder*)pParam)->ZSet.ZBackType, 0, 0, 0, 0,
		((COrder*)pParam)->MoveSpeedSet.EndSpeed, ((COrder*)pParam)->MoveSpeedSet.AccSpeed, ((COrder*)pParam)->MoveSpeedSet.InitSpeed, 1);
#endif
	while ((!((COrder*)pParam)->m_Action.m_bIsStop) && ((COrder*)pParam)->ModelControl.Mode != 4/*&& ((COrder*)pParam)->Commanding != _T("End")*/)//新增模式判斷
	{
		if (((COrder*)pParam)->RunData.SubProgramName != _T(""))//雙程式使用(目前尚未用到)
		{
			for (UINT i = 1; i < ((COrder*)pParam)->Command.size(); i++)
			{
				if (((COrder*)pParam)->Command.at(i).at(0) == ((COrder*)pParam)->RunData.SubProgramName)
				{
					((COrder*)pParam)->RunData.MSChange.push_back(i);
					((COrder*)pParam)->RunData.StackingCount++;
				}
			}
			((COrder*)pParam)->RunData.SubProgramName = _T("");
		}
		/***********程序命令執行***********/
		if (((COrder*)pParam)->Program.LabelName != _T(""))//Label控制
		{
			((COrder*)pParam)->Program.LabelCount++;
			if (((COrder*)pParam)->Program.LabelName == ((COrder*)pParam)->Command.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)).at(((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount))))
			{
				//找到此標籤時
				if (((COrder*)pParam)->Program.CallSubroutineStatus)//子程序的標籤
				{
#ifdef PRINTF
					_cwprintf(L"Thread()::找到Subroutine的Label\n");
#endif
					if (((COrder*)pParam)->SubroutinePretreatmentFind(pParam))//執行子程序命令預處理
					{
#ifdef LOG
						InitFileLog(L"找到預命令:" + ((COrder*)pParam)->Program.SubroutineCommandPretreatment + L"\n");
#endif
					}
					((COrder*)pParam)->Program.CallSubroutineStatus = FALSE;//清除子程序呼叫狀態
				}
				//找到Loop標籤時
				else if (((COrder*)pParam)->RepeatData.LoopSwitch)
				{
#ifdef PRINTF
					_cwprintf(L"Thread()::找到Loop的Label\n");
#endif
					((COrder*)pParam)->RepeatData.LoopSwitch = FALSE;//清除Loop呼叫狀態
					//判斷StepRepeat是否有強行跳轉
					StepRepeatJumpforciblyJudge(pParam, ((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)));
				}
				//找到GotoLabel或Intput標籤
				else
				{
#ifdef PRINTF
					_cwprintf(L"Thread()::找到Label\n");
#endif
					//判斷StepRepeat是否有強行跳轉
					StepRepeatJumpforciblyJudge(pParam, ((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)));
				}
				//標籤清除
				((COrder*)pParam)->Program.LabelName = _T("");
				((COrder*)pParam)->Program.LabelCount = 0;
			}
			else if (((COrder*)pParam)->Program.LabelCount == ((COrder*)pParam)->Command.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)).size())
			{
				//全部尋找完畢，未找到標籤
#ifdef PRINTF
				_cwprintf(L"Thread()::未找到Label\n");
#endif
				if (((COrder*)pParam)->Program.CallSubroutineStatus)//子程序的標籤
				{
					if (!((COrder*)pParam)->Program.SubroutineStack.empty())
					{
						//堆疊程序計數--
						((COrder*)pParam)->Program.SubroutinCount--;
						//紀錄程序位置堆疊釋放
						((COrder*)pParam)->Program.SubroutineStack.pop_back();
						//紀錄呼叫手臂位置堆疊釋放
						((COrder*)pParam)->Program.SubroutinePointStack.pop_back();
						//記錄呼叫時模式堆疊釋放
						((COrder*)pParam)->Program.SubroutineModel.pop_back();
						//釋放offset堆疊
						((COrder*)pParam)->OffsetData.pop_back();
						//釋放狀態堆疊
						((COrder*)pParam)->ArcData.pop_back();
						((COrder*)pParam)->CircleData1.pop_back();
						((COrder*)pParam)->CircleData2.pop_back();
						((COrder*)pParam)->StartData.pop_back();
						((COrder*)pParam)->RunData.ActionStatus.pop_back();
						//將Call子程序狀態設為否
						((COrder*)pParam)->Program.CallSubroutineStatus = FALSE;
					}
				}
				else if (((COrder*)pParam)->RepeatData.LoopSwitch)//Loop的標籤
				{
					if (!((COrder*)pParam)->RepeatData.LoopAddressNum.empty())
					{
						((COrder*)pParam)->RepeatData.LoopAddressNum.pop_back();
						((COrder*)pParam)->RepeatData.LoopCount.pop_back();
						((COrder*)pParam)->RepeatData.LoopSwitch = FALSE;
					}
				}
				//標籤清除
				((COrder*)pParam)->Program.LabelName = _T("");
				((COrder*)pParam)->Program.LabelCount = 0;
			}
			else
			{
#ifdef PRINTF
				//_cwprintf(L"Thread()::Label查找指令(%d)...\n", ((COrder*)pParam)->Program.LabelCount);
#endif
			}
		}
		else if (((COrder*)pParam)->RepeatData.StepRepeatLabel != _T(""))//StepRepeatLabel控制
		{
			//讓StepRepeat尋找下一個StepRepeat
			if (((COrder*)pParam)->RepeatData.StepRepeatLabelLock)//利用StepRepeaLabel編號尋找是否有下一個StepRepeat
			{
#ifdef PRINTF
				_cwprintf(L"Thread()::%s=%s\n", ((COrder*)pParam)->RepeatData.StepRepeatLabel, CommandResolve(((COrder*)pParam)->Command.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)).at(((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount))), 6));
#endif
				if (((COrder*)pParam)->RepeatData.StepRepeatLabel ==
					CommandResolve(((COrder*)pParam)->Command.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)).at(((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount))), 6) && 
					(CommandResolve(((COrder*)pParam)->Command.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)).at(((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount))), 0) == L"StepRepeatX" ||
					CommandResolve(((COrder*)pParam)->Command.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)).at(((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount))), 0) == L"StepRepeatY") )
				{
					((COrder*)pParam)->RepeatData.StepRepeatLabel = _T("");
					//++是為了讓他執行StepRepeat
					((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount))++;
				}
			}
			//讓StepRepeat尋找StepRepeatLabel
			else
			{   
				if (((COrder*)pParam)->RepeatData.StepRepeatLabel == ((COrder*)pParam)->Command.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)).at(((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount))))
				{
					((COrder*)pParam)->RepeatData.StepRepeatLabel = _T("");//標籤清除
				}
#ifdef PRINTF
				_cwprintf(L"Thread()::尋找%s中...\n", ((COrder*)pParam)->RepeatData.StepRepeatLabel);
#endif
			}
		}
		else//執行指令
		{
			((COrder*)pParam)->Commanding = ((COrder*)pParam)->Command.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)).at(((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)));
			((COrder*)pParam)->RunStatusRead.CurrentRunCommandNum = ((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount));//主程式編號
#ifdef LOG
			//LARGE_INTEGER   StartTime, EndTime, Fre;
			//QueryPerformanceFrequency(&Fre); //取得CPU頻率
			//QueryPerformanceCounter(&StartTime); //取得開機到現在經過幾個CPU Cycle
			CString Temp;
			Temp.Format(L"%s:%d\nStatus:%d,OffsetX=%d,OffsetY=%d,OffsetZ=%d\n\n", ((COrder*)pParam)->Commanding, ((COrder*)pParam)->Program.SubroutinCount,
				((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Status,
				((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X,
				((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
				((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z);
			InitFileLog(Temp);
#endif
			/*****命令執行緒*****/
			::ResetEvent(((COrder*)pParam)->ThreadEvent.Thread);//釋放事件
			g_pSubroutineThread = AfxBeginThread(((COrder*)pParam)->SubroutineThread, pParam);
			::WaitForSingleObject(((COrder*)pParam)->ThreadEvent.Thread, INFINITE);//等待事件被設置
			/*舊版2017/03/21*/
			//while (g_pSubroutineThread) {
			//	Sleep(1);//while 程式負載問題 無限迴圈，並讓 CPU 休息一下
			//}

			/*****檢測執行緒*****/
			if (((COrder*)pParam)->CheckModel == 1 && ((COrder*)pParam)->CheckSwitch.ImmediateCheck)//判斷即時檢測是否執行
			{
				::ResetEvent(((COrder*)pParam)->ThreadEvent.CheckActionThread);//釋放事件
				g_pCheckActionThread = AfxBeginThread(((COrder*)pParam)->CheckAction, pParam);
				::WaitForSingleObject(((COrder*)pParam)->ThreadEvent.CheckActionThread, INFINITE);//等待事件被設置
				/*舊版2017/04/05*/
				//while (g_pCheckActionThread) {
				//	Sleep(1);//while 程式負載問題 無限迴圈，並讓 CPU 休息一下
				//}
			}
			if (((COrder*)pParam)->CheckSwitch.RunCheck)//判斷區間檢測是否執行
			{
				((COrder*)pParam)->AreaCheckChangTemp.Status = TRUE;//開啟中間點
#ifdef MOVE
				MO_StopGumming();//關閉出膠
#endif
				::ResetEvent(((COrder*)pParam)->ThreadEvent.CheckCoordinateScanThread);//釋放事件
				g_pCheckCoordinateScanThread = AfxBeginThread(((COrder*)pParam)->CheckCoordinateScan, pParam);
				::WaitForSingleObject(((COrder*)pParam)->ThreadEvent.CheckCoordinateScanThread, INFINITE);//等待事件被設置			
				/*舊版2017/04/05*/
				//while (g_pCheckCoordinateScanThread) {
				//	Sleep(1);//while 程式負載問題 無限迴圈，並讓 CPU 休息一下
				//}
			}
#ifdef LOG
			//QueryPerformanceCounter(&EndTime); //取得開機到程式執行完成經過幾個CPU Cycle
			//CString Temp;
			//Temp.Format(L"%s at RunTime:%.6f\n", ((COrder*)pParam)->Commanding,((((double)EndTime.QuadPart - (double)StartTime.QuadPart)) / Fre.QuadPart));
			//InitFileLog(Temp);
#endif
		}   
		/***********程序命令計數***********/
		if (((COrder*)pParam)->RepeatData.StepRepeatLabelLock)//在StepRepeat階段用來尋找N層StepRepeat用
		{
			if (((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)) == 0)//(由下往上掃描)掃描到當命令地址為0時停止
			{
#ifdef PRINTF
				_cwprintf(L"Thread()::沒有發現其他StepRepeat\n");
#endif
				if (CommandResolve(((COrder*)pParam)->RepeatData.StepRepeatLabel, 0) == L"StepRepeatLabel")
				{
#ifdef PRINTF
					_cwprintf(L"Thread()::發生StepRepeatLabel尋找錯誤立即停止程序!\n");
#endif
					((COrder*)pParam)->m_Action.m_bIsStop = TRUE;
				}
				else
				{
					//將尋找StepRepeat模式改成尋找StepRepeatLabel模式
					((COrder*)pParam)->RepeatData.StepRepeatLabel = _T("StepRepeatLabel,") + ((COrder*)pParam)->RepeatData.StepRepeatLabel;
#ifdef PRINTF
					_cwprintf(L"Thread()::將模式改成尋找StepRepeatLabel\n");
#endif
				}
				((COrder*)pParam)->RepeatData.StepRepeatLabelLock = FALSE;
			}
			else
			{
				((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount))--;
#ifdef PRINTF 
				((COrder*)pParam)->RepeatData.StepRepeatLabel != _T("") ? _cwprintf(L"Thread()::StepRpeat尋找中...\n") : _cwprintf(L"Thread()::StepRpeat找到了!\n");
#endif
			}
		}
		else//正常程序命令計數用
		{
			if (((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)) == ((COrder*)pParam)->Command.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)).size() - 1)
			{
				((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)) = 0;
			}
			else
			{
				((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount))++;
			}
		}
#ifdef PRINTF
		//檢查計數用
		//_cwprintf(L"Thread():Model:%d,%d\n", ((COrder*)pParam)->ModelControl.Mode, ((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)));
		//_cwprintf(L"大家都進來吧!%s\n", ((COrder*)pParam)->RepeatData.StepRepeatLabel);
#endif 
	}
	LineGotoActionJudge(pParam);//判斷動作狀態
	if (((COrder*)pParam)->Commanding == _T("End"))//計數運行次數使用
	{
		((COrder*)pParam)->RunStatusRead.FinishProgramCount++;//完成加工計數+1
		if (((COrder*)pParam)->RunLoopData.RunSwitch)//判斷是否開啟循環迴圈(變數有衝突)
		{
			((COrder*)pParam)->RunLoopData.LoopCount++;//循環迴圈次數+1
		}
	}
	if (((COrder*)pParam)->GoHome.PrecycleInitialize)//開啟結束初始化(原點賦歸)
	{
		((COrder*)pParam)->m_Action.BackGOZero(((COrder*)pParam)->MoveSpeedSet.EndSpeed, ((COrder*)pParam)->MoveSpeedSet.AccSpeed, ((COrder*)pParam)->MoveSpeedSet.InitSpeed);
	}
	//TODO::DEMO所以加入
	/*
	if (AfxMessageBox(_T("資料即將清除，是否儲存?"), MB_OKCANCEL, 0) == IDOK) 
	{
		SavePointData(pParam);
	} 
	if (!((COrder*)pParam)->DemoTemprarilySwitch)//如果DemoTemprarilySwitch為FALSE清除
	{
		((COrder*)pParam)->m_Action.LA_Clear();//清除連續線段陣列
	} 
	*/
	((COrder*)pParam)->DecideClear();//清除所有陣列
	((COrder*)pParam)->m_Action.m_bIsDispend = TRUE;//將控制出膠設回可出膠(防止View後人機要使用)
	//計算執行時間
	QueryPerformanceCounter(&((COrder*)pParam)->endTime); //取得開機到程式執行完成經過幾個CPU Cycle
	((COrder*)pParam)->RunStatusRead.RunTotalTime = ((double)((COrder*)pParam)->endTime.QuadPart - (double)((COrder*)pParam)->startTime.QuadPart) / ((COrder*)pParam)->fre.QuadPart;//計算程式運行時間
	
	((COrder*)pParam)->RunStatusRead.RunStatus = 0;//狀態設為未運行
	g_pThread = NULL;
	if(g_pRunLoopThread)//判斷是否是循環迴圈
		::SetEvent(((COrder*)pParam)->ThreadEvent.RunLoopThread);
	return 0;
}
/*命令動作(子)執行緒*/
UINT COrder::SubroutineThread(LPVOID pParam) {
	//HANDLE wakeEvent = (HANDLE)((COrder*)pParam)->wakeEvent;//獲取事件句柄(測試用)
	((COrder*)pParam)->RunStatusRead.StepCommandStatus = TRUE;//執行命令狀態
	CString Command = ((COrder*)pParam)->Commanding;//命令字串 
	UINT CurrentRunCommandNum = ((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount));//命令地址
	if (CommandResolve(Command, 0) == L"Printf")//命令測試用
	{
#ifdef PRINTF
		/*for (UINT i = 0; i < ((COrder*)pParam)->IntervalTemplateCheck.size(); i++)
		{
			_cwprintf(L"目前地址:%s\n", ((COrder*)pParam)->IntervalTemplateCheck.at(i).Address);
		}*/
		// _cwprintf(L"目前地址:%s\n", ((COrder*)pParam)->GetCommandAddress());
#endif  
		//((COrder*)pParam)->m_Action.WaitTime(wakeEvent, _ttoi(CommandResolve(Command, 1)));//啟動事件計時
		//_cwprintf(L"%d...\n", ((COrder*)pParam)->PointAreaJudge({ 500,500 }, { 1000,0,0,0 }));
		//((COrder*)pParam)->CircleAreaJudge({ 145378,78612 }, { 199096,110359 }, { 183319,140973 }, 0);
	}
	/************************************************************程序**************************************************************/
	else if (CommandResolve(Command, 0) == L"GotoLabel") 
	{
		if (((COrder*)pParam)->ModelControl.Mode != 0)//建表模式
		{
			((COrder*)pParam)->Program.LabelName = _T("Label,") + CommandResolve(Command, 1);
		}
	}
	else if (CommandResolve(Command, 0) == L"GotoAddress")
	{
		if (((COrder*)pParam)->ModelControl.Mode != 0)//建表模式
		{
			if (_ttoi(CommandResolve(Command, 1)) && _ttoi(CommandResolve(Command, 1)) < (int)((COrder*)pParam)->Command.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)).size())//判斷Goto地址是否在命令列中
			{
				((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)) = _ttoi(CommandResolve(Command, 1)) - 2;
				//判斷StepRepeat是否有強行跳轉
				StepRepeatJumpforciblyJudge(pParam, ((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)));
			}
		}
	}
	else if (CommandResolve(Command, 0) == L"CallSubroutine")
	{
		//TODO::可思考 如果LS->CallSubroutine 結果沒有此Subroutine時還是會移到LS，已使用限制命令解決
		if (((COrder*)pParam)->RunData.ActionStatus.at(((COrder*)pParam)->Program.SubroutinCount) == 1)//LS時 移動至LS虛擬點
		{
			((COrder*)pParam)->m_Action.DecideVirtualPoint(((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Y, ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Z, ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).W,
				((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, ((COrder*)pParam)->LineSpeedSet.InitSpeed);
			((COrder*)pParam)->VirtualCoordinateData = ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount);
		}
		//CallSubroutineStatus狀態打開
		((COrder*)pParam)->Program.CallSubroutineStatus = TRUE;//為了判斷是否有這個子程序
		((COrder*)pParam)->Program.LabelName = _T("Label,") + CommandResolve(Command, 1);
		//將目前程序地址紀錄
		((COrder*)pParam)->Program.SubroutineStack.push_back(((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)));
		//新增紀錄目前機械手臂(虛擬座標)位置堆疊
		//實際座標取法
		((COrder*)pParam)->Program.SubroutinePointStack.push_back({ 0,((COrder*)pParam)->VirtualCoordinateData.X ,((COrder*)pParam)->VirtualCoordinateData.Y ,((COrder*)pParam)->VirtualCoordinateData.Z });
		//新增Subroutine呼叫時模式堆疊
		((COrder*)pParam)->Program.SubroutineModel.push_back(((COrder*)pParam)->ModelControl.Mode);
		//新增offset堆疊
		((COrder*)pParam)->OffsetData.push_back(((COrder*)pParam)->InitData);
		//新增狀態堆疊
		((COrder*)pParam)->ArcData.push_back(((COrder*)pParam)->InitData);
		((COrder*)pParam)->CircleData1.push_back(((COrder*)pParam)->InitData);
		((COrder*)pParam)->CircleData2.push_back(((COrder*)pParam)->InitData);
		((COrder*)pParam)->StartData.push_back(((COrder*)pParam)->InitData);
		((COrder*)pParam)->RunData.ActionStatus.push_back(0);
		//新增影像修正狀態
		((COrder*)pParam)->Program.SubroutineVisioModifyJudge.push_back(((COrder*)pParam)->VisioModifyJudge);
		//堆疊計數加1
		((COrder*)pParam)->Program.SubroutinCount++; 
#ifdef PRINTF
		_cwprintf(L"SubroutineThread()::呼叫子程序:數量(%d)地址(%d)\n", ((COrder*)pParam)->Program.SubroutinCount, ((COrder*)pParam)->Program.SubroutineStack.back());
#endif
	}
	else if (CommandResolve(Command, 0) == L"SubroutineEnd")
	{
		//判斷子程序堆疊中是否為空
		if (!((COrder*)pParam)->Program.SubroutineStack.empty())
		{
			if (((COrder*)pParam)->Program.SubroutineModel.back() == ((COrder*)pParam)->ModelControl.Mode)//判斷跳出模式是否等於目前模式
			{
#ifdef PRINTF
				_cwprintf(L"SubroutineThread()::結束子程序:數量(%d)地址(%d)\n", ((COrder*)pParam)->Program.SubroutinCount, ((COrder*)pParam)->Program.SubroutineStack.back());
#endif
				//TODO::可選擇是否在影像模式不移動回CallSuboutine位置
				//將機器手臂移動至呼叫時位置
#ifdef MOVE
				if (((COrder*)pParam)->ModelControl.Mode == 3)//在運動模式下才做回歸動作
				{
					((COrder*)pParam)->m_Action.DecideVirtualPoint(
						((COrder*)pParam)->Program.SubroutinePointStack.at(((COrder*)pParam)->Program.SubroutinCount).X,
						((COrder*)pParam)->Program.SubroutinePointStack.at(((COrder*)pParam)->Program.SubroutinCount).Y,
						((COrder*)pParam)->Program.SubroutinePointStack.at(((COrder*)pParam)->Program.SubroutinCount).Z,
						((COrder*)pParam)->Program.SubroutinePointStack.at(((COrder*)pParam)->Program.SubroutinCount).W,
						((COrder*)pParam)->DotSpeedSet.EndSpeed, ((COrder*)pParam)->DotSpeedSet.AccSpeed, ((COrder*)pParam)->DotSpeedSet.InitSpeed);
				}
#endif
				if (((COrder*)pParam)->Program.SubroutineModelControlSwitch)//用在模式二進入模式三結束子程序
				{
					//將程式地址設為雷射轉換模式地址
					((COrder*)pParam)->ModelControl.Mode = 3;
					((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)) = ((COrder*)pParam)->ModelControl.ModeChangeAddress - 1;
					//判斷StepRepeat是否有強行跳轉
					//StepRepeatJumpforciblyJudge(pParam, ((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)));
					//在模式轉換後必須將雷射狀態初始化
					((COrder*)pParam)->LaserSwitch = { 0,0,0,0,0 };
					((COrder*)pParam)->Program.SubroutineModelControlSwitch = FALSE;
#ifdef PRINTF
					_cwprintf(L"SubroutineThread()::SubroutineEnd模式轉換(2->3)跳至地址:%d\n\n下一個模式即將開始...\n", ((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)));
#endif
				}
				else//用在相同模式進入和結束子程序
				{
					//將程序地址設為呼叫子程序時地址
					((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)) = ((COrder*)pParam)->Program.SubroutineStack.back();
					//判斷StepRepeat是否有強行跳轉
					//StepRepeatJumpforciblyJudge(pParam, ((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)));
#ifdef PRINTF
					_cwprintf(L"SubroutineThread()::SubroutineEnd子程序結束跳至地址:%d\n\n下一個模式即將開始...\n", ((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)));
#endif
				}
				//判斷CallSubroutine中動作是否完全
				LineGotoActionJudge(pParam);
				//釋放紀錄程序位置堆疊
				((COrder*)pParam)->Program.SubroutineStack.pop_back();   
				//釋放紀錄手臂位置堆疊
				((COrder*)pParam)->Program.SubroutinePointStack.pop_back();
				//釋放記錄呼叫時模式堆疊
				((COrder*)pParam)->Program.SubroutineModel.pop_back();
				//釋放offset堆疊
				((COrder*)pParam)->OffsetData.pop_back();
				//釋放狀態堆疊
				((COrder*)pParam)->ArcData.pop_back();
				((COrder*)pParam)->CircleData1.pop_back();
				((COrder*)pParam)->CircleData2.pop_back();
				((COrder*)pParam)->StartData.pop_back();
				((COrder*)pParam)->RunData.ActionStatus.pop_back();
				//釋放影像修正狀態
				((COrder*)pParam)->Program.SubroutineVisioModifyJudge.pop_back();
				//將堆疊計數減1
				((COrder*)pParam)->Program.SubroutinCount--;
			}
			else//用在模式三進入模式二結束子程序
			{
				((COrder*)pParam)->ModelControl.Mode = 3;
				((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)) = ((COrder*)pParam)->ModelControl.ModeChangeAddress - 1;
				//在模式轉換後必須將雷射狀態初始化
				((COrder*)pParam)->LaserSwitch = { 0,0,0,0,0 };
				((COrder*)pParam)->Program.SubroutineModelControlSwitch = FALSE;
#ifdef PRINTF
				_cwprintf(L"SubroutineThread()::SubroutineEnd模式轉換(3->2)跳至地址:%d\n\n下一個模式即將開始...\n", ((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)));
#endif
			}
		}
	}
	else if (CommandResolve(Command, 0) == L"StepRepeatLabel")
	{
		if (!((COrder*)pParam)->RepeatData.StepRepeatLabelLock)//目的用於跳到StepRepeat最外層迴圈
		{
#ifdef PRINTF
			_cwprintf(L"SubroutineThread()::進入StepRepeatLabel\n");
#endif  
			CString CommandBuff = _T("");   
			for (UINT i = CurrentRunCommandNum; i < ((COrder*)pParam)->Command.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)).size(); i++)
			{
				CommandBuff = ((COrder*)pParam)->Command.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)).at(i);
				if (_ttol(CommandResolve(CommandBuff, 6)) == _ttol(CommandResolve(Command, 1)) && (CommandResolve(CommandBuff, 0) == L"StepRepeatX" || CommandResolve(CommandBuff, 0) == L"StepRepeatY"))
				{
					((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)) = i + 1;//改變目前執行地址(之後會減1所以+1)
					((COrder*)pParam)->RepeatData.StepRepeatLabelLock = TRUE;//標籤鎖打開
				}
				//判斷命令是否掃描到最後一項且標籤鎖打開
				if ((i == (((COrder*)pParam)->Command.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)).size() - 1)) && ((COrder*)pParam)->RepeatData.StepRepeatLabelLock)
				{
					//判斷StepRepeat是否有強行跳轉
					StepRepeatJumpforciblyJudge(pParam, ((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)));
					//((COrder*)pParam)->RepeatData.StepRepeatIntervel.push_back({ L"StepRepeat",CurrentRunCommandNum, ((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)) - 1 });//新增StepRepeat區間
					((COrder*)pParam)->RepeatData.StepRepeatAddress = CurrentRunCommandNum;
#ifdef PRINTF
					
					_cwprintf(L"SubroutineThread()::跳躍至地址為%d的StepRepeat\n", ((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)));
#endif 
				}
			}
		}   
	}
	else if (CommandResolve(Command, 0) == L"Loop")
	{
		if (((COrder*)pParam)->ModelControl.Mode == 3)//建表模式
		{
			if (_ttol(CommandResolve(Command, 2)))
			{
				if (!((COrder*)pParam)->RepeatData.LoopAddressNum.size())//都沒有Loop時
				{
					((COrder*)pParam)->RepeatData.LoopSwitch = TRUE;
					((COrder*)pParam)->RepeatData.LoopAddressNum.push_back(((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)));
					((COrder*)pParam)->RepeatData.LoopCount.push_back(_ttol(CommandResolve(Command, 2)));
					((COrder*)pParam)->Program.LabelName = _T("Label,") + CommandResolve(Command, 1);
				}
				else
				{
					UINT LoopAddressNumSize = ((COrder*)pParam)->RepeatData.LoopAddressNum.size();
					for (UINT i = 0; i < LoopAddressNumSize; i++)
					{
						if (((COrder*)pParam)->RepeatData.LoopAddressNum.at(i) == ((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)))
						{
							if (((COrder*)pParam)->RepeatData.LoopCount.at(i) > 1)
							{
								((COrder*)pParam)->RepeatData.LoopSwitch = TRUE;
								((COrder*)pParam)->Program.LabelName = _T("Label,") + CommandResolve(Command, 1);
								--((COrder*)pParam)->RepeatData.LoopCount.at(i);
							}
							else
							{
								((COrder*)pParam)->RepeatData.LoopAddressNum.erase(((COrder*)pParam)->RepeatData.LoopAddressNum.begin() + i);
								((COrder*)pParam)->RepeatData.LoopCount.erase(((COrder*)pParam)->RepeatData.LoopCount.begin() + i);
							}
						}
						else
						{
							if (i == LoopAddressNumSize - 1)//掃到最後一個時
							{
								((COrder*)pParam)->RepeatData.LoopSwitch = TRUE;
								((COrder*)pParam)->RepeatData.LoopAddressNum.push_back(((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)));
								((COrder*)pParam)->RepeatData.LoopCount.push_back(_ttol(CommandResolve(Command, 2)));
								((COrder*)pParam)->Program.LabelName = _T("Label,") + CommandResolve(Command, 1);
							}

						}
					}
				}
			}
		}
	}
	else if (CommandResolve(Command, 0) == L"StepRepeatX")
	{
#ifdef PRINTF
		_cprintf("SubroutineThread()::Inside StepRepeatX\n");
#endif 
		//StepRepeat陣列是否大於1*1 && 模式是否 1or2 
		if ((_ttol(CommandResolve(Command, 3)) * _ttol(CommandResolve(Command, 4))) > 0 && (_ttol(CommandResolve(Command,5)) == 1 || _ttol(CommandResolve(Command, 5)) == 2))
		{
			if (!((COrder*)pParam)->RepeatData.StepRepeatNum.size())//都沒有StepRepeatXY時
			{
				if (((COrder*)pParam)->RepeatData.StepRepeatLabelLock)//第一次進入SetpRepeat(代表最外層)
				{
#ifdef PRINTF
					_cwprintf(L"SubroutineThread()::沒有StepRepeat時 First Inside StepRepeatX\n");
#endif
					//StepRepeat新增計數總數歸零(防止出錯)
					((COrder*)pParam)->RepeatData.AllNewStepRepeatNum = 0;
					//StepRepeat刪除計數總數歸零(防止出錯)
					((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum = 0;
					//StepRepeat新增計數總數++
					((COrder*)pParam)->RepeatData.AllNewStepRepeatNum++;
					//紀錄StepRepeat地址
					((COrder*)pParam)->RepeatData.StepRepeatNum.push_back(((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)));
					//紀錄初始offset位置
					if (((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Status)//防止CallSubroutine第一個為StepRepeat時
					{
						((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.push_back(((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X);
						((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.push_back(((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y);
					}
					else
					{
						ModifyPointOffSet(pParam,((COrder*)pParam)->Program.SubroutineCommandPretreatment);
					}
					//紀錄X、Y計數
					((COrder*)pParam)->RepeatData.StepRepeatCountX.push_back(_ttol(CommandResolve(Command, 3)));
					((COrder*)pParam)->RepeatData.StepRepeatCountY.push_back(_ttol(CommandResolve(Command, 4)));
					//紀錄X、Y總數
					((COrder*)pParam)->RepeatData.StepRepeatTotalX.push_back(_ttol(CommandResolve(Command, 3)));//只有用在建立修正表時
					((COrder*)pParam)->RepeatData.StepRepeatTotalY.push_back(_ttol(CommandResolve(Command, 4)));//只有用在建立修正表時
					//記錄StepRepeat區間
					((COrder*)pParam)->RepeatData.StepRepeatIntervel.push_back({ L"StepRepeat",((COrder*)pParam)->RepeatData.StepRepeatAddress,CurrentRunCommandNum });
					//S行迴圈狀態初始化
					((COrder*)pParam)->RepeatData.SSwitch.push_back(TRUE);
					//紀錄Block
					((COrder*)pParam)->InitBlockData.BlockPosition.clear();
					((COrder*)pParam)->InitBlockData.BlockNumber = _ttol(CommandResolve(Command, 8));
					for (int i = 0; i < ((COrder*)pParam)->InitBlockData.BlockNumber; i++)
					{
						((COrder*)pParam)->InitBlockData.BlockPosition.push_back(CommandResolve(Command, i + 9));
					}
					((COrder*)pParam)->RepeatData.StepRepeatBlockData.push_back(((COrder*)pParam)->InitBlockData);
					//阻斷陣列排序
					BlockSort(((COrder*)pParam)->RepeatData.StepRepeatBlockData.back().BlockPosition, 1, _ttol(CommandResolve(Command, 5)));
					//判斷是否有阻斷
					if (_ttol(CommandResolve(Command, 7)))//有阻斷處理阻斷
					{
#ifdef PRINTF
						_cwprintf(L"SubroutineThread()::StepRepeatX 處理阻斷位置:");
						for (UINT i = 0; i < ((COrder*)pParam)->RepeatData.StepRepeatBlockData.back().BlockPosition.size(); i++)
						{
							_cwprintf(L"%s,", ((COrder*)pParam)->RepeatData.StepRepeatBlockData.back().BlockPosition.at(i));
						}
						_cwprintf(L"\n");
#endif
						BlockProcessStartX(Command, pParam, FALSE);
					}
					else//無阻斷繼續往下尋找StepRepeat
					{
#ifdef PRINTF
						_cwprintf(L"SubroutineThread()::StepRepeatX 沒有阻斷\n");
#endif
						((COrder*)pParam)->RepeatData.StepRepeatLabel = CommandResolve(Command, 6);
					}   
				}
				else 
				{
#ifdef PRINTF
					_cwprintf(L"SubroutineThread()::所有條件不成立\n");
#endif
				}
			}
			else//有StepRepeatX時
			{
				UINT StepRepeatNumSize = ((COrder*)pParam)->RepeatData.StepRepeatNum.size();//獲取目前有幾個StepRepeat
#ifdef PRINTF
				_cwprintf(L"SubroutineThread()::目前StepRepeatNumSize:%d\n", StepRepeatNumSize);
#endif 
				for (UINT i = 0; i < StepRepeatNumSize; i++)
				{
					//判斷目前遇到的StepRepeat是否在StepRepeat陣列中
					if (((COrder*)pParam)->RepeatData.StepRepeatNum.at(i) == ((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)))
					{
						if (((COrder*)pParam)->RepeatData.StepRepeatCountX.at(i) > 1)
						{
							//S型
							if (_ttol(CommandResolve(Command, 5)) == 1)
							{ 
								if (((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X == ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.at(i) + (_ttol(CommandResolve(Command, 1))*(_ttol(CommandResolve(Command, 3)) - 1)))
								{
									((COrder*)pParam)->RepeatData.SSwitch.at(i) = FALSE;
#ifdef PRINTF
									_cwprintf(L"SubroutineThread()::第%d:SSwitch轉換:%d\n", i, ((COrder*)pParam)->RepeatData.SSwitch.at(i));
#endif 
								}
								else if (((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X == ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.at(i))
								{
									((COrder*)pParam)->RepeatData.SSwitch.at(i) = TRUE;
#ifdef PRINTF
									_cwprintf(L"SubroutineThread()::第%d:SSwitch轉換:%d\n", i, ((COrder*)pParam)->RepeatData.SSwitch.at(i));
#endif 
								}
								if (!((COrder*)pParam)->RepeatData.SSwitch.at(i))
								{
									((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X = ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X - _ttol(CommandResolve(Command, 1));
								}
								else
								{
									((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X = ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X + _ttol(CommandResolve(Command, 1));
								}
							}
							//N型
							if (_ttol(CommandResolve(Command, 5)) == 2)
							{
								((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X = ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X + _ttol(CommandResolve(Command, 1));
							}
							((COrder*)pParam)->RepeatData.StepRepeatCountX.at(i)--;
							((COrder*)pParam)->RepeatData.StepRepeatLabel = _T("StepRepeatLabel,") + CommandResolve(Command, 6);
							//阻斷開啟時
							if (_ttol(CommandResolve(Command, 7)))
							{
								if (!BlockProcessExecuteX(Command, pParam, i))
									break;
							} 
						}
						else if(((COrder*)pParam)->RepeatData.StepRepeatCountY.at(i) > 1)
						{
							((COrder*)pParam)->RepeatData.SSwitch.at(i) = !((COrder*)pParam)->RepeatData.SSwitch.at(i);
#ifdef PRINTF
							_cwprintf(L"SubroutineThread()::第%d:SSwitch轉換:%d\n", i, ((COrder*)pParam)->RepeatData.SSwitch.at(i));
#endif 
							if (_ttol(CommandResolve(Command, 5)) == 2)//N型X回最初位置
							{
								((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X = ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.at(i);
							}  
							((COrder*)pParam)->RepeatData.StepRepeatCountX.at(i) = _ttol(CommandResolve(Command, 3));
							((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y = ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y + _ttol(CommandResolve(Command, 2));
							((COrder*)pParam)->RepeatData.StepRepeatCountY.at(i)--;
							((COrder*)pParam)->RepeatData.StepRepeatLabel = _T("StepRepeatLabel,") + CommandResolve(Command, 6);
							//阻斷開啟時
							if (_ttol(CommandResolve(Command, 7)))
							{
								if (!BlockProcessExecuteX(Command, pParam, i))
									break;
							} 
						}
						else
						{
#ifdef PRINTF
							_cwprintf(L"SubroutineThread()::刪除所有陣列\n");
#endif 
							((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum++;
#ifdef PRINTF
							_cwprintf(L"SubroutineThread()::刪除總數+1=%d\n", ((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum);
#endif 
							((COrder*)pParam)->RepeatData.StepRepeatBlockData.erase(((COrder*)pParam)->RepeatData.StepRepeatBlockData.begin() + i);
#ifdef PRINTF
							_cwprintf(L"SubroutineThread()::刪除StepRepeatBlockData陣列\n");
#endif 
							((COrder*)pParam)->RepeatData.SSwitch.erase(((COrder*)pParam)->RepeatData.SSwitch.begin() + i);
#ifdef PRINTF
							_cwprintf(L"SubroutineThread()::刪除SSwitch陣列\n");
#endif 
							((COrder*)pParam)->RepeatData.StepRepeatNum.erase(((COrder*)pParam)->RepeatData.StepRepeatNum.begin() + i);
#ifdef PRINTF
							_cwprintf(L"SubroutineThread()::刪除StepRepeatNum陣列\n");
#endif 
							//Offset復原
							((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X = ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.at(i);
							((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y = ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.at(i);
							((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.erase(((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.begin() + i);
#ifdef PRINTF
							_cwprintf(L"SubroutineThread()::刪除StepRepeatInitOffsetX陣列\n");
#endif 
							((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.erase(((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.begin() + i);
#ifdef PRINTF
							_cwprintf(L"SubroutineThread()::刪除StepRepeatInitOffsetY陣列\n");
#endif 
							((COrder*)pParam)->RepeatData.StepRepeatCountX.erase(((COrder*)pParam)->RepeatData.StepRepeatCountX.begin() + i);
#ifdef PRINTF
							_cwprintf(L"SubroutineThread()::刪除StepRepeatCountX陣列\n");
#endif 
							((COrder*)pParam)->RepeatData.StepRepeatCountY.erase(((COrder*)pParam)->RepeatData.StepRepeatCountY.begin() + i);
#ifdef PRINTF
							_cwprintf(L"SubroutineThread()::刪除StepRepeatCountY陣列\n");
#endif 
							((COrder*)pParam)->RepeatData.StepRepeatTotalX.erase(((COrder*)pParam)->RepeatData.StepRepeatTotalX.begin() + i);//只有用在建立修正表時
#ifdef PRINTF
							_cwprintf(L"SubroutineThread()::刪除StepRepeatTotalX陣列\n");
#endif 
							((COrder*)pParam)->RepeatData.StepRepeatTotalY.erase(((COrder*)pParam)->RepeatData.StepRepeatTotalY.begin() + i);//只有用在建立修正表時
#ifdef PRINTF
							_cwprintf(L"SubroutineThread()::刪除StepRepeatTotalY陣列\n");
#endif 
							((COrder*)pParam)->RepeatData.StepRepeatIntervel.erase(((COrder*)pParam)->RepeatData.StepRepeatIntervel.begin() + i);
#ifdef PRINTF
							_cwprintf(L"SubroutineThread()::刪除StepRepeatIntervel區間陣列\n");
#endif 
							//判斷是否為最後一個StepRepeat,如果是清除刪除、新增計數
							if (!((COrder*)pParam)->RepeatData.StepRepeatNum.size())
							{
								((COrder*)pParam)->RepeatData.AllNewStepRepeatNum = 0;
								((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum = 0;
#ifdef PRINTF
								_cwprintf(L"SubroutineThread()::新增、刪除總數:%d,%d\n", ((COrder*)pParam)->RepeatData.AllNewStepRepeatNum, ((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum);
#endif 
							}
							break;
						}
						if (((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum != 0 && ((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum != ((COrder*)pParam)->RepeatData.AllNewStepRepeatNum )
						{
#ifdef PRINTF
							_cwprintf(L"SubroutineThread()::需要新增內層迴圈\n");
#endif 
							((COrder*)pParam)->RepeatData.StepRepeatLabelLock = TRUE;
							((COrder*)pParam)->RepeatData.AddInStepRepeatSwitch = TRUE;
							((COrder*)pParam)->RepeatData.StepRepeatLabel = CommandResolve(Command, 6);
							((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum--;
							break;
						}
					}
					else//不再此StepRepeat陣列中，新增一個StepRepeat位置
					{
						if (i == StepRepeatNumSize - 1)
						{
							//未執行時新增StepRepeatX
							if (((COrder*)pParam)->RepeatData.StepRepeatLabelLock && !((COrder*)pParam)->RepeatData.AddInStepRepeatSwitch)//第一次進入SetpRepeat時(內層迴圈)
							{
#ifdef PRINTF
								_cwprintf(L"SubroutineThread()::未執行時新增StepRepeatX內層\n");
#endif 
								//StepRepeat新增計數總數++
								((COrder*)pParam)->RepeatData.AllNewStepRepeatNum++;
								//紀錄StepRepeat地址
								((COrder*)pParam)->RepeatData.StepRepeatNum.push_back(((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)));
								//紀錄初始offset位置
								if (((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Status)//防止CallSubroutine第一個為StepRepeat時
								{
									((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.push_back(((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X);
									((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.push_back(((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y);
								}
								else
								{
									ModifyPointOffSet(pParam, ((COrder*)pParam)->Program.SubroutineCommandPretreatment);
								}
								//紀錄X、Y計數
								((COrder*)pParam)->RepeatData.StepRepeatCountX.push_back(_ttol(CommandResolve(Command, 3)));
								((COrder*)pParam)->RepeatData.StepRepeatCountY.push_back(_ttol(CommandResolve(Command, 4)));
								//紀錄X、Y總數
								((COrder*)pParam)->RepeatData.StepRepeatTotalX.push_back(_ttol(CommandResolve(Command, 3)));//只有用在建立修正表時
								((COrder*)pParam)->RepeatData.StepRepeatTotalY.push_back(_ttol(CommandResolve(Command, 4)));//只有用在建立修正表時
								//記錄StepRepeat區間
								((COrder*)pParam)->RepeatData.StepRepeatIntervel.push_back({ L"StepRepeat",((COrder*)pParam)->RepeatData.StepRepeatAddress,CurrentRunCommandNum });
								//S行迴圈狀態初始化
								((COrder*)pParam)->RepeatData.SSwitch.push_back(TRUE);
								//紀錄Block
								((COrder*)pParam)->InitBlockData.BlockPosition.clear();
								((COrder*)pParam)->InitBlockData.BlockNumber = _ttol(CommandResolve(Command, 8));
								for (int i = 0; i < ((COrder*)pParam)->InitBlockData.BlockNumber; i++)
								{
									((COrder*)pParam)->InitBlockData.BlockPosition.push_back(CommandResolve(Command, i + 9));
								}
								((COrder*)pParam)->RepeatData.StepRepeatBlockData.push_back(((COrder*)pParam)->InitBlockData);
								//阻斷陣列排序
								BlockSort(((COrder*)pParam)->RepeatData.StepRepeatBlockData.back().BlockPosition, 1, _ttol(CommandResolve(Command, 5)));
								if (_ttol(CommandResolve(Command, 7)))//有阻斷
								{
#ifdef PRINTF
									_cwprintf(L"SubroutineThread()::StepRepeatX 處理阻斷位置:");
									for (UINT i = 0; i < ((COrder*)pParam)->RepeatData.StepRepeatBlockData.back().BlockPosition.size(); i++)
									{
										_cwprintf(L"%s,", ((COrder*)pParam)->RepeatData.StepRepeatBlockData.back().BlockPosition.at(i));
									}
									_cwprintf(L"\n");
#endif 
									BlockProcessStartX(Command, pParam, FALSE);
								}
								else//無阻斷不記錄繼續往下尋找StepRepeat
								{
#ifdef PRINTF
									_cwprintf(L"SubroutineThread()::StepRepeatX 沒有阻斷\n");
#endif 
									((COrder*)pParam)->RepeatData.StepRepeatLabel = CommandResolve(Command, 6);
								}
							}
							//執行時新增StepRepeatX
							else if(((COrder*)pParam)->RepeatData.StepRepeatLabelLock) //第二次進入StepRepeatX時(內層迴圈做第N次)
							{
#ifdef PRINTF
								_cwprintf(L"SubroutineThread()::執行時新增StepRepeatX內層\n"); 
#endif 
								((COrder*)pParam)->RepeatData.AddInStepRepeatSwitch = FALSE;//執行新增控制關閉
								((COrder*)pParam)->RepeatData.StepRepeatLabelLock = FALSE;
								//紀錄StepRepeat地址
								((COrder*)pParam)->RepeatData.StepRepeatNum.push_back(((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)));
								//紀錄初始offset位置
								if (((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Status)//防止CallSubroutine第一個為StepRepeat時
								{
									((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.push_back(((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X);
									((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.push_back(((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y);
								}
								else/***2017/03/24***/
								{
									ModifyPointOffSet(pParam, ((COrder*)pParam)->Program.SubroutineCommandPretreatment);
								}
								//紀錄X、Y計數
								((COrder*)pParam)->RepeatData.StepRepeatCountX.push_back(_ttol(CommandResolve(Command, 3)));
								((COrder*)pParam)->RepeatData.StepRepeatCountY.push_back(_ttol(CommandResolve(Command, 4)));
								//紀錄X、Y總數
								((COrder*)pParam)->RepeatData.StepRepeatTotalX.push_back(_ttol(CommandResolve(Command, 3)));//只有用在建立修正表時
								((COrder*)pParam)->RepeatData.StepRepeatTotalY.push_back(_ttol(CommandResolve(Command, 4)));//只有用在建立修正表時
								//記錄StepRepeat區間
								((COrder*)pParam)->RepeatData.StepRepeatIntervel.push_back({ L"StepRepeat",((COrder*)pParam)->RepeatData.StepRepeatIntervel.back().BeginAddress,CurrentRunCommandNum });
								//S行迴圈狀態初始化
								((COrder*)pParam)->RepeatData.SSwitch.push_back(TRUE);
								//紀錄Block
								((COrder*)pParam)->InitBlockData.BlockPosition.clear();
								((COrder*)pParam)->InitBlockData.BlockNumber = _ttol(CommandResolve(Command, 8));
								for (int i = 0; i < ((COrder*)pParam)->InitBlockData.BlockNumber; i++)
								{
									((COrder*)pParam)->InitBlockData.BlockPosition.push_back(CommandResolve(Command, i + 9));
								}
								((COrder*)pParam)->RepeatData.StepRepeatBlockData.push_back(((COrder*)pParam)->InitBlockData);
								//阻斷陣列排序
								BlockSort(((COrder*)pParam)->RepeatData.StepRepeatBlockData.back().BlockPosition, 1, _ttol(CommandResolve(Command, 5)));
								if (_ttol(CommandResolve(Command, 7)))//有阻斷
								{
#ifdef PRINTF
									_cwprintf(L"SubroutineThread()::StepRepeatX 處理阻斷位置:");
									for (UINT i = 0; i < ((COrder*)pParam)->RepeatData.StepRepeatBlockData.back().BlockPosition.size(); i++)
									{
										_cwprintf(L"%s,", ((COrder*)pParam)->RepeatData.StepRepeatBlockData.back().BlockPosition.at(i));
									}
									_cwprintf(L"\n");
#endif 
									BlockProcessStartX(Command, pParam, TRUE);
								}
								else//無阻斷不記錄繼續往下尋找StepRepeat
								{
#ifdef PRINTF
									_cwprintf(L"SubroutineThread()::StepRepeatX 沒有阻斷\n");
#endif 
									if (((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum != 0 && ((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum != ((COrder*)pParam)->RepeatData.AllNewStepRepeatNum)
									{
#ifdef PRINTF
										_cwprintf(L"SubroutineThread()::繼續補償內層迴圈\n");
#endif 
										((COrder*)pParam)->RepeatData.StepRepeatLabelLock = TRUE;
										((COrder*)pParam)->RepeatData.AddInStepRepeatSwitch = TRUE;
										((COrder*)pParam)->RepeatData.StepRepeatLabel = CommandResolve(Command, 6);
										((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum--;
										break;
									}
									else
									{
										((COrder*)pParam)->RepeatData.StepRepeatLabel = _T("StepRepeatLabel,") + CommandResolve(Command, 6);
									}
								}
							}
						}
					}
				}
			}
		}
		else//當0-1 或 1-0 時
		{
			((COrder*)pParam)->RepeatData.StepRepeatLabelLock = FALSE;
			((COrder*)pParam)->RepeatData.StepRepeatLabel = _T("");
		}
	}
	else if (CommandResolve(Command, 0) == L"StepRepeatY")
	{
#ifdef PRINTF
		_cprintf("SubroutineThread()::Inside StepRepeatY\n");
#endif 
		//StepRepeat陣列是否大於1*1 && 模式是否 1or2
		if ((_ttol(CommandResolve(Command, 3)) * _ttol(CommandResolve(Command, 4))) > 0 && (_ttol(CommandResolve(Command, 5)) == 1 || _ttol(CommandResolve(Command, 5)) == 2))
		{
			if (!((COrder*)pParam)->RepeatData.StepRepeatNum.size())//都沒有RepeatXY時
			{
				if (((COrder*)pParam)->RepeatData.StepRepeatLabelLock)//第一次進入SetpRepeat(代表最外層)
				{
#ifdef PRINTF
					_cwprintf(L"SubroutineThread()::沒有StepRepeat時 First Inside StepRepeatY\n");
#endif 
					//StepRepeat新增計數總數歸零(防止出錯)
					((COrder*)pParam)->RepeatData.AllNewStepRepeatNum = 0;
					//StepRepeat刪除計數總數歸零(防止出錯)
					((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum = 0;
					//StepRepeat新增計數總數++
					((COrder*)pParam)->RepeatData.AllNewStepRepeatNum++;
					//紀錄StepRepeat地址
					((COrder*)pParam)->RepeatData.StepRepeatNum.push_back(((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)));
					//紀錄初始offset位置
					if (((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Status)//防止CallSubroutine第一個為StepRepeat時
					{
						((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.push_back(((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X);
						((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.push_back(((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y);
					}
					else
					{
						ModifyPointOffSet(pParam, ((COrder*)pParam)->Program.SubroutineCommandPretreatment);
					}
					//紀錄X、Y計數
					((COrder*)pParam)->RepeatData.StepRepeatCountX.push_back(_ttol(CommandResolve(Command, 3)));
					((COrder*)pParam)->RepeatData.StepRepeatCountY.push_back(_ttol(CommandResolve(Command, 4)));
					//紀錄X、Y總數
					((COrder*)pParam)->RepeatData.StepRepeatTotalX.push_back(_ttol(CommandResolve(Command, 3)));//只有用在建立修正表時
					((COrder*)pParam)->RepeatData.StepRepeatTotalY.push_back(_ttol(CommandResolve(Command, 4)));//只有用在建立修正表時
					//記錄StepRepeat區間
					((COrder*)pParam)->RepeatData.StepRepeatIntervel.push_back({ L"StepRepeat",((COrder*)pParam)->RepeatData.StepRepeatAddress,CurrentRunCommandNum });
					//S行迴圈狀態初始化
					((COrder*)pParam)->RepeatData.SSwitch.push_back(TRUE);
					//紀錄Block
					((COrder*)pParam)->InitBlockData.BlockPosition.clear();
					((COrder*)pParam)->InitBlockData.BlockNumber = _ttol(CommandResolve(Command, 8));
					for (int i = 0; i < ((COrder*)pParam)->InitBlockData.BlockNumber; i++)
					{
						((COrder*)pParam)->InitBlockData.BlockPosition.push_back(CommandResolve(Command, i + 9));
					}
					((COrder*)pParam)->RepeatData.StepRepeatBlockData.push_back(((COrder*)pParam)->InitBlockData);
					//阻斷陣列排序
					/**/BlockSort(((COrder*)pParam)->RepeatData.StepRepeatBlockData.back().BlockPosition, 2, _ttol(CommandResolve(Command, 5)));
					//判斷是否有阻斷
					if (_ttol(CommandResolve(Command, 7)))//有阻斷處理阻斷
					{
#ifdef PRINTF
						_cwprintf(L"SubroutineThread()::StepRepeatY 處理阻斷位置:");
						for (UINT i = 0; i < ((COrder*)pParam)->RepeatData.StepRepeatBlockData.back().BlockPosition.size(); i++)
						{
							_cwprintf(L"%s,", ((COrder*)pParam)->RepeatData.StepRepeatBlockData.back().BlockPosition.at(i));
						}
						_cwprintf(L"\n");
#endif 
						/**/BlockProcessStartY(Command, pParam, FALSE);
					}
					else//無阻斷繼續往下尋找StepRepeat
					{
#ifdef PRINTF
						_cwprintf(L"SubroutineThread()::StepRepeatX 沒有阻斷\n");
#endif 
						((COrder*)pParam)->RepeatData.StepRepeatLabel = CommandResolve(Command, 6);
					}
				}
				else
				{
#ifdef PRINTF
					_cwprintf(L"SubroutineThread()::所有條件不成立\n");
#endif 
				}
			}
			else//有StepRepeatY時
			{
				UINT StepRepeatNumSize = ((COrder*)pParam)->RepeatData.StepRepeatNum.size();
#ifdef PRINTF
				_cwprintf(L"SubroutineThread()::目前StepRepeatNumSize:%d\n", StepRepeatNumSize);
#endif 
				for (UINT i = 0; i < StepRepeatNumSize; i++)
				{
					//判斷目前遇到的StepRepeat是否在StepRepeat陣列中
					if (((COrder*)pParam)->RepeatData.StepRepeatNum.at(i) == ((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)))
					{
						if (((COrder*)pParam)->RepeatData.StepRepeatCountY.at(i) > 1)/**/
						{
							//S型
							if (_ttol(CommandResolve(Command, 5)) == 1)
							{
								if (((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y == ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.at(i) + (_ttol(CommandResolve(Command, 2))*(_ttol(CommandResolve(Command, 4)) - 1)))/**/
								{
									((COrder*)pParam)->RepeatData.SSwitch.at(i) = FALSE;
#ifdef PRINTF
									_cwprintf(L"SubroutineThread()::第%d:SSwitch轉換:%d\n", i, ((COrder*)pParam)->RepeatData.SSwitch.at(i));
#endif 
								}
								else  if (((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y == ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.at(i))
								{
									((COrder*)pParam)->RepeatData.SSwitch.at(i) = TRUE; 
#ifdef PRINTF
									_cwprintf(L"SubroutineThread()::第%d:SSwitch轉換:%d\n", i, ((COrder*)pParam)->RepeatData.SSwitch.at(i));
#endif 
								}
								if (!((COrder*)pParam)->RepeatData.SSwitch.at(i))
								{
									/**/((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y = ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y - _ttol(CommandResolve(Command, 2));
								}
								else
								{
									/**/((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y = ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y + _ttol(CommandResolve(Command, 2));
								}
							}
							//N型
							if (_ttol(CommandResolve(Command, 5)) == 2)
							{
								/**/((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y = ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y + _ttol(CommandResolve(Command, 2));
							}
							/**/((COrder*)pParam)->RepeatData.StepRepeatCountY.at(i)--;
							((COrder*)pParam)->RepeatData.StepRepeatLabel = _T("StepRepeatLabel,") + CommandResolve(Command, 6);
							//阻斷開啟時
							if (_ttol(CommandResolve(Command, 7)))
							{
								/**/if (!BlockProcessExecuteY(Command, pParam, i))
										break;
							}
						}
						else if (((COrder*)pParam)->RepeatData.StepRepeatCountX.at(i) > 1)/**/
						{
							((COrder*)pParam)->RepeatData.SSwitch.at(i) = !((COrder*)pParam)->RepeatData.SSwitch.at(i);
#ifdef PRINTF
							_cwprintf(L"SubroutineThread()::第%d:SSwitch轉換:%d\n", i, ((COrder*)pParam)->RepeatData.SSwitch.at(i));
#endif 
							if (_ttol(CommandResolve(Command, 5)) == 2)//N型Y回最初位置
							{
								/**/((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y = ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.at(i);
							}
							/**/((COrder*)pParam)->RepeatData.StepRepeatCountY.at(i) = _ttol(CommandResolve(Command, 4));
							/**/((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X = ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X + _ttol(CommandResolve(Command, 1));
							/**/((COrder*)pParam)->RepeatData.StepRepeatCountX.at(i)--;
							((COrder*)pParam)->RepeatData.StepRepeatLabel = _T("StepRepeatLabel,") + CommandResolve(Command, 6);
							//阻斷開啟時
							if (_ttol(CommandResolve(Command, 7)))
							{
								/**/if (!BlockProcessExecuteY(Command, pParam, i))
										break;
							}
						}
						else
						{
#ifdef PRINTF
							_cwprintf(L"SubroutineThread()::刪除所有陣列\n");
#endif 
							((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum++;
#ifdef PRINTF
							_cwprintf(L"SubroutineThread()::刪除總數+1=%d\n", ((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum);
#endif 
							((COrder*)pParam)->RepeatData.StepRepeatBlockData.erase(((COrder*)pParam)->RepeatData.StepRepeatBlockData.begin() + i);
#ifdef PRINTF
							_cwprintf(L"SubroutineThread()::刪除StepRepeatBlockData陣列\n");
#endif 
							((COrder*)pParam)->RepeatData.SSwitch.erase(((COrder*)pParam)->RepeatData.SSwitch.begin() + i);
#ifdef PRINTF
							_cwprintf(L"SubroutineThread()::刪除SSwitch陣列\n");
#endif 
							((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X = ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.at(i);
							((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y = ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.at(i);
							((COrder*)pParam)->RepeatData.StepRepeatNum.erase(((COrder*)pParam)->RepeatData.StepRepeatNum.begin() + i);
#ifdef PRINTF
							_cwprintf(L"SubroutineThread()::刪除StepRepeatNum陣列\n");
#endif 
							((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.erase(((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.begin() + i);
#ifdef PRINTF
							_cwprintf(L"SubroutineThread()::刪除StepRepeatInitOffsetX陣列\n");
#endif 
							((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.erase(((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.begin() + i);
#ifdef PRINTF
							_cwprintf(L"SubroutineThread()::刪除StepRepeatInitOffsetY陣列\n");
#endif 
							((COrder*)pParam)->RepeatData.StepRepeatCountX.erase(((COrder*)pParam)->RepeatData.StepRepeatCountX.begin() + i);
#ifdef PRINTF
							_cwprintf(L"SubroutineThread()::刪除StepRepeatCountX陣列\n");
#endif 
							((COrder*)pParam)->RepeatData.StepRepeatCountY.erase(((COrder*)pParam)->RepeatData.StepRepeatCountY.begin() + i);
#ifdef PRINTF
							_cwprintf(L"SubroutineThread()::刪除StepRepeatCountY陣列\n");
#endif 
							((COrder*)pParam)->RepeatData.StepRepeatTotalX.erase(((COrder*)pParam)->RepeatData.StepRepeatTotalX.begin() + i);
#ifdef PRINTF
							_cwprintf(L"SubroutineThread()::刪除StepRepeatToatlX陣列\n");
#endif 
							((COrder*)pParam)->RepeatData.StepRepeatTotalY.erase(((COrder*)pParam)->RepeatData.StepRepeatTotalY.begin() + i);
#ifdef PRINTF
							_cwprintf(L"SubroutineThread()::刪除StepRepeatTotalY陣列\n");
#endif
							((COrder*)pParam)->RepeatData.StepRepeatIntervel.erase(((COrder*)pParam)->RepeatData.StepRepeatIntervel.begin() + i);
#ifdef PRINTF
							_cwprintf(L"SubroutineThread()::刪除StepRepeatIntervel區間陣列\n");
#endif 
							//判斷是否為最後一個StepRepeat,如果是清除刪除、新增計數
							if (!((COrder*)pParam)->RepeatData.StepRepeatNum.size())
							{
								((COrder*)pParam)->RepeatData.AllNewStepRepeatNum = 0;
								((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum = 0;
#ifdef PRINTF
								_cwprintf(L"SubroutineThread()::新增、刪除總數:%d,%d\n", ((COrder*)pParam)->RepeatData.AllNewStepRepeatNum, ((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum);
#endif 
							}
							break;
						}
						if (((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum != 0 && ((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum != ((COrder*)pParam)->RepeatData.AllNewStepRepeatNum)
						{
#ifdef PRINTF
							_cwprintf(L"SubroutineThread()::進入新增內層迴圈\n");
#endif 
							((COrder*)pParam)->RepeatData.StepRepeatLabelLock = TRUE;
							((COrder*)pParam)->RepeatData.AddInStepRepeatSwitch = TRUE;
							((COrder*)pParam)->RepeatData.StepRepeatLabel = CommandResolve(Command, 6);
							((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum--;
							break;
						}
					}
					else//不再StepRepeat陣列中，新增一個StepRepeat位置
					{
						if (i == StepRepeatNumSize - 1)
						{
							//未執行時新增StepRepeatY
							if (((COrder*)pParam)->RepeatData.StepRepeatLabelLock && !((COrder*)pParam)->RepeatData.AddInStepRepeatSwitch)//第一次進入SetpRepeat時(內層迴圈)
							{
#ifdef PRINTF
								_cwprintf(L"SubroutineThread()::未執行時新增StepRepeatY內層\n");
#endif 
								//StepRepeat計數總數++
								((COrder*)pParam)->RepeatData.AllNewStepRepeatNum++;
								//紀錄StepRepeat地址
								((COrder*)pParam)->RepeatData.StepRepeatNum.push_back(((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)));
								//紀錄初始offset位置
								if (((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Status)//防止CallSubroutine第一個為StepRepeat時
								{
									((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.push_back(((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X);
									((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.push_back(((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y);
								}
								else
								{
									ModifyPointOffSet(pParam, ((COrder*)pParam)->Program.SubroutineCommandPretreatment);
								}
								//紀錄X、Y計數
								((COrder*)pParam)->RepeatData.StepRepeatCountX.push_back(_ttol(CommandResolve(Command, 3)));
								((COrder*)pParam)->RepeatData.StepRepeatCountY.push_back(_ttol(CommandResolve(Command, 4)));
								//紀錄X、Y總數
								((COrder*)pParam)->RepeatData.StepRepeatTotalX.push_back(_ttol(CommandResolve(Command, 3)));//只有用在建立修正表時
								((COrder*)pParam)->RepeatData.StepRepeatTotalY.push_back(_ttol(CommandResolve(Command, 4)));//只有用在建立修正表時
								//記錄StepRepeat區間
								((COrder*)pParam)->RepeatData.StepRepeatIntervel.push_back({ L"StepRepeat",((COrder*)pParam)->RepeatData.StepRepeatAddress,CurrentRunCommandNum });
								//S行迴圈狀態初始化
								((COrder*)pParam)->RepeatData.SSwitch.push_back(TRUE);
								//紀錄Block
								((COrder*)pParam)->InitBlockData.BlockPosition.clear();
								((COrder*)pParam)->InitBlockData.BlockNumber = _ttol(CommandResolve(Command, 8));
								for (int i = 0; i < ((COrder*)pParam)->InitBlockData.BlockNumber; i++)
								{
									((COrder*)pParam)->InitBlockData.BlockPosition.push_back(CommandResolve(Command, i + 9));
								}
								((COrder*)pParam)->RepeatData.StepRepeatBlockData.push_back(((COrder*)pParam)->InitBlockData);
								//阻斷陣列排序
								/**/BlockSort(((COrder*)pParam)->RepeatData.StepRepeatBlockData.back().BlockPosition, 2, _ttol(CommandResolve(Command, 5)));
								if (_ttol(CommandResolve(Command, 7)))//有阻斷
								{
#ifdef PRINTF
									_cwprintf(L"SubroutineThread()::StepRepeatY 處理阻斷位置:");
									for (UINT i = 0; i < ((COrder*)pParam)->RepeatData.StepRepeatBlockData.back().BlockPosition.size(); i++)
									{
										_cwprintf(L"%s,", ((COrder*)pParam)->RepeatData.StepRepeatBlockData.back().BlockPosition.at(i));
									}
									_cwprintf(L"\n");
#endif 
									/**/BlockProcessStartY(Command, pParam, FALSE);
								}
								else//無阻斷不記錄繼續往下尋找StepRepeat
								{
#ifdef PRINTF
									_cwprintf(L"SubroutineThread()::StepRepeatY 沒有阻斷\n");
#endif 
									((COrder*)pParam)->RepeatData.StepRepeatLabel = CommandResolve(Command, 6);
								}
							}
							//執行時新增StepRepeatY
							else if (((COrder*)pParam)->RepeatData.StepRepeatLabelLock)//第二次進入StepRepeatY時(內層迴圈做第N次)
							{
#ifdef PRINTF
								_cwprintf(L"SubroutineThread()::執行時新增StepRepeatY內層\n");
#endif 
								((COrder*)pParam)->RepeatData.AddInStepRepeatSwitch = FALSE;//執行新增控制關閉
								((COrder*)pParam)->RepeatData.StepRepeatLabelLock = FALSE;
								//紀錄StepRepeat地址
								((COrder*)pParam)->RepeatData.StepRepeatNum.push_back(((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)));
								//紀錄初始offset位置
								if (((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Status)//防止CallSubroutine第一個為StepRepeat時
								{
									((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.push_back(((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X);
									((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.push_back(((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y);
								}
								else/***2017/03/24***/
								{
									ModifyPointOffSet(pParam, ((COrder*)pParam)->Program.SubroutineCommandPretreatment);
								}
								//紀錄X、Y計數
								((COrder*)pParam)->RepeatData.StepRepeatCountX.push_back(_ttol(CommandResolve(Command, 3)));
								((COrder*)pParam)->RepeatData.StepRepeatCountY.push_back(_ttol(CommandResolve(Command, 4)));
								//紀錄X、Y總數
								((COrder*)pParam)->RepeatData.StepRepeatTotalX.push_back(_ttol(CommandResolve(Command, 3)));//只有用在建立修正表時
								((COrder*)pParam)->RepeatData.StepRepeatTotalY.push_back(_ttol(CommandResolve(Command, 4)));//只有用在建立修正表時
								//記錄StepRepeat區間
								((COrder*)pParam)->RepeatData.StepRepeatIntervel.push_back({ L"StepRepeat",((COrder*)pParam)->RepeatData.StepRepeatIntervel.back().BeginAddress,CurrentRunCommandNum });
								//S行迴圈狀態初始化
								((COrder*)pParam)->RepeatData.SSwitch.push_back(TRUE);
								//紀錄Block
								((COrder*)pParam)->InitBlockData.BlockPosition.clear();
								((COrder*)pParam)->InitBlockData.BlockNumber = _ttol(CommandResolve(Command, 8));
								for (int i = 0; i < ((COrder*)pParam)->InitBlockData.BlockNumber; i++)
								{
									((COrder*)pParam)->InitBlockData.BlockPosition.push_back(CommandResolve(Command, i + 9));
								}
								((COrder*)pParam)->RepeatData.StepRepeatBlockData.push_back(((COrder*)pParam)->InitBlockData);
								//阻斷陣列排序
								/**/BlockSort(((COrder*)pParam)->RepeatData.StepRepeatBlockData.back().BlockPosition, 2, _ttol(CommandResolve(Command, 5)));
								if (_ttol(CommandResolve(Command, 7)))//有阻斷
								{
#ifdef PRINTF
									_cwprintf(L"SubroutineThread()::StepRepeatY 處理阻斷位置:");
									for (UINT i = 0; i < ((COrder*)pParam)->RepeatData.StepRepeatBlockData.back().BlockPosition.size(); i++)
									{
										_cwprintf(L"%s,", ((COrder*)pParam)->RepeatData.StepRepeatBlockData.back().BlockPosition.at(i));
									}
									_cwprintf(L"\n");
#endif 
									/**/BlockProcessStartY(Command, pParam, TRUE);
								}
								else//無阻斷不記錄繼續往下尋找StepRepeat
								{
#ifdef PRINTF
									_cwprintf(L"SubroutineThread()::StepRepeatY 沒有阻斷\n");
#endif 
									if (((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum != 0 && ((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum != ((COrder*)pParam)->RepeatData.AllNewStepRepeatNum)
									{
#ifdef PRINTF
										_cwprintf(L"SubroutineThread()::繼續補償內層迴圈\n");
#endif 
										((COrder*)pParam)->RepeatData.StepRepeatLabelLock = TRUE;
										((COrder*)pParam)->RepeatData.AddInStepRepeatSwitch = TRUE;
										((COrder*)pParam)->RepeatData.StepRepeatLabel = CommandResolve(Command, 6);
										((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum--;
										break;
									}
									else
									{
										((COrder*)pParam)->RepeatData.StepRepeatLabel = _T("StepRepeatLabel,") + CommandResolve(Command, 6);
									}
								}
							}

						}
					}
				}
			}
		}
		else //當 0-1 或 1-0 時
		{   
			((COrder*)pParam)->RepeatData.StepRepeatLabelLock = FALSE;
			((COrder*)pParam)->RepeatData.StepRepeatLabel = _T("");
		}
	}
	else if (CommandResolve(Command, 0) == L"CallSubProgram")//目前尚未用到
	{
		//((COrder*)pParam)->RunData.SubProgramName = _T("SubProgramStart,") + CommandResolve(Command, 1);
	}
	else if (CommandResolve(Command, 0) == L"SubProgramEnd")//目前尚未用到
	{
		//((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)) = 0;//副程序計數清零
		//((COrder*)pParam)->RunData.MSChange.pop_back();
		//((COrder*)pParam)->RunData.StackingCount--; 
		//((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount))--;//上次執行過後有+1所以要減回來
	}

	else if (CommandResolve(Command, 0) == L"End")
	{
		if (((COrder*)pParam)->ModelControl.Mode == 0)
		{
			((COrder*)pParam)->LoadCommandData.FirstRun = FALSE;
			if (!((COrder*)pParam)->ModelControl.VisionModeJump)
			{
				((COrder*)pParam)->ModelControl.Mode = 1;
#ifdef PRINTF
				_cwprintf(L"SubroutineThread()::End模式轉換(0->1)\n\n下一個模式即將開始...\n");
#endif
			} 
			else
			{
				((COrder*)pParam)->ModelControl.Mode = 3;
#ifdef PRINTF
				_cwprintf(L"SubroutineThread()::End模式轉換(0->3)\n\n下一個模式即將開始...\n");
#endif
			}
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 1)
		{
			((COrder*)pParam)->ModelControl.Mode = 3;
			//在模式轉換後必須將雷射狀態初始化
			((COrder*)pParam)->LaserSwitch = { 0,0,0,0,0 };
#ifdef PRINTF
			_cwprintf(L"SubroutineThread()::End模式轉換(1->3)\n\n下一個模式即將開始...\n");
#endif
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 2)
		{
			if (((COrder*)pParam)->Program.SubroutineStack.empty())//沒有Subroutine時才能轉換模式
			{
				((COrder*)pParam)->ModelControl.Mode = 3;
				((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)) = ((COrder*)pParam)->ModelControl.ModeChangeAddress - 1;
				//在模式轉換後必須將雷射狀態初始化
				((COrder*)pParam)->LaserSwitch = { 0,0,0,0,0 };
#ifdef PRINTF
				_cwprintf(L"SubroutineThread()::End模式轉換(2->3)地址跳至:%d\n\n下一個模式即將開始...\n", ((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)));
#endif
			}
			else//代表Subroutine沒有SubroutineEnd
			{
				AfxMessageBox(L"程式發生違法撰寫");
			}
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 3)
		{
			if (((COrder*)pParam)->CheckSwitch.Template || ((COrder*)pParam)->CheckSwitch.Diameter || ((COrder*)pParam)->CheckSwitch.Area)
			{
				if (((COrder*)pParam)->CheckSwitch.Area)
				{
					for (UINT i = 0; i < ((COrder*)pParam)->IntervalAreaCheck.size(); i++)
					{
						if (((COrder*)pParam)->IntervalAreaCheck.at(i).Address == ((COrder*)pParam)->CurrentCheckAddress)//找出目前的檢測指令
						{
							if (((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData.size())
							{
								if (((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData.back().x != -1 &&
									((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData.back().y != -1)//判斷是否有換線點
								{
									((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData.push_back({ -1,-1 });
								}
							}
						}
					}
				}
				((COrder*)pParam)->CheckSwitch.RunCheck = TRUE;
				((COrder*)pParam)->CheckSwitch.Template = FALSE;
				((COrder*)pParam)->CheckSwitch.Diameter = FALSE;
				((COrder*)pParam)->CheckSwitch.Area = FALSE;
				((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount))--;//讓End指令重新執行一次，為了讓程序可結束
			}
			else
			{
				((COrder*)pParam)->ModelControl.Mode = 4;//結束程序
			}
		}
	}
	/************************************************************動作**************************************************************/
	else if (CommandResolve(Command, 0) == L"Dot")
	{
		if (((COrder*)pParam)->ModelControl.Mode == 0)//建表模式
		{
			RecordCorrectionTable(pParam);//寫入修正表
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 1)//影像模式
		{
			RecordCorrectionTable(pParam);//寫入修正表
			VirtualCoordinateMove(pParam, Command, 1);//虛擬座標移動
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 2)//雷射模式
		{
			LaserDetectHandle(pParam, Command);//雷射測高和虛擬座標移動
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 3)//運動模式
		{
			((COrder*)pParam)->ActionCount++;//動作計數++
			ChooseVisionModify(pParam);//選擇影像Offset
			ChooseLaserModify(pParam);//選擇雷射高度
			ModifyPointOffSet(pParam, Command);//CallSubroutin相對位修正
			LineGotoActionJudge(pParam);//判斷動作狀態
			((COrder*)pParam)->FinalWorkCoordinateData.X = _ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X;
			((COrder*)pParam)->FinalWorkCoordinateData.Y = _ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y;
			((COrder*)pParam)->FinalWorkCoordinateData.Z = _ttol(CommandResolve(Command, 3)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z;
			((COrder*)pParam)->FinalWorkCoordinateData.W = _tstof(CommandResolve(Command, 4)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).W;

			((COrder*)pParam)->NVMVirtualCoordinateData = ((COrder*)pParam)->FinalWorkCoordinateData;//紀錄CallSubroutine點(不加影像修正時的值)

			VisionModify(pParam);//影像修正
			LaserModify(pParam);//雷射修正
#ifdef PRINTF
			_cwprintf(L"SubroutineThread()::%s(%d,%d,%d,%.3f)\n", CommandResolve(Command, 0), ((COrder*)pParam)->FinalWorkCoordinateData.X, ((COrder*)pParam)->FinalWorkCoordinateData.Y, ((COrder*)pParam)->FinalWorkCoordinateData.Z, ((COrder*)pParam)->FinalWorkCoordinateData.W);
#endif
#ifdef MOVE
			((COrder*)pParam)->m_Action.DecidePointGlue(
				//_ttoi(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X + ((COrder*)pParam)->VisionOffset.OffsetX,
				//_ttoi(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y + ((COrder*)pParam)->VisionOffset.OffsetY,
				//_ttoi(CommandResolve(Command, 3)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z + ((COrder*)pParam)->VisionOffset.OffsetZ,
				((COrder*)pParam)->FinalWorkCoordinateData.X,
				((COrder*)pParam)->FinalWorkCoordinateData.Y,
				((COrder*)pParam)->FinalWorkCoordinateData.Z,
				((COrder*)pParam)->FinalWorkCoordinateData.W,
				((COrder*)pParam)->DispenseDotSet.GlueOpenTime, ((COrder*)pParam)->DispenseDotSet.GlueCloseTime,
				((COrder*)pParam)->ZSet.ZBackHeight, ((COrder*)pParam)->ZSet.ZBackType, ((COrder*)pParam)->DispenseDotEnd.RiseDistance, ((COrder*)pParam)->DispenseDotEnd.RiseHightSpeed, ((COrder*)pParam)->DispenseDotEnd.RiseLowSpeed,
				((COrder*)pParam)->DotSpeedSet.EndSpeed, ((COrder*)pParam)->DotSpeedSet.AccSpeed, ((COrder*)pParam)->DotSpeedSet.InitSpeed);
#endif
			((COrder*)pParam)->VirtualCoordinateData = ((COrder*)pParam)->FinalWorkCoordinateData;//紀錄移動虛擬座標
			if (((COrder*)pParam)->CheckSwitch.ImmediateCheck)//即時檢測(模板、區間)
			{
				((COrder*)pParam)->CheckModel = 1;//檢測模式設為即時檢測
			}
			else//區間檢測(模板、直徑、區域)
			{
				if (((COrder*)pParam)->CheckSwitch.Template || ((COrder*)pParam)->CheckSwitch.Diameter)//模板、直徑區間檢測點新增
				{
					CheckCoordinate CheckCoordinateInit;
					if (((COrder*)pParam)->CheckSwitch.Template)
					{
						CheckCoordinateInit.CheckMode = L"TemplateCheck";
					}
					else if (((COrder*)pParam)->CheckSwitch.Diameter)
					{
						CheckCoordinateInit.CheckMode = L"DiameterCheck";
					}
					CheckCoordinateInit.CheckModeAddress = ((COrder*)pParam)->CurrentCheckAddress;//加入目前區間檢測地址
					CheckCoordinateInit.Address = ((COrder*)pParam)->GetCommandAddress();//加入命令地址	
					CheckCoordinateInit.Position = { 0, ((COrder*)pParam)->FinalWorkCoordinateData.X , ((COrder*)pParam)->FinalWorkCoordinateData.Y , 0 };
					//加入區間檢測點陣列
					((COrder*)pParam)->IntervalCheckCoordinate.push_back(CheckCoordinateInit);
				}   
				else if (((COrder*)pParam)->CheckSwitch.Area)//區域區間檢測點新增
				{
					for (UINT i = 0; i < ((COrder*)pParam)->IntervalAreaCheck.size(); i++)//判斷目前使用的區間 加入點陣列
					{
						if (((COrder*)pParam)->IntervalAreaCheck.at(i).Address == ((COrder*)pParam)->CurrentCheckAddress)
						{
							//判斷區域是否重組
							if (((COrder*)pParam)->IntervalAreaCheck.at(i).Image.Start.x == ((COrder*)pParam)->IntervalAreaCheck.at(i).Image.End.x && ((COrder*)pParam)->IntervalAreaCheck.at(i).Image.Start.y == ((COrder*)pParam)->IntervalAreaCheck.at(i).Image.End.y
								&& ((COrder*)pParam)->IntervalAreaCheck.at(i).Image.Start.x != VI_MosaicAreaDefault && ((COrder*)pParam)->IntervalAreaCheck.at(i).Image.Start.y != VI_MosaicAreaDefault
								&& ((COrder*)pParam)->IntervalAreaCheck.at(i).Image.End.x != VI_MosaicAreaDefault && ((COrder*)pParam)->IntervalAreaCheck.at(i).Image.End.y != VI_MosaicAreaDefault)//不重組
							{
								//判斷點是否在區域內
								if (((COrder*)pParam)->PointAreaJudge({ ((COrder*)pParam)->FinalWorkCoordinateData.X ,((COrder*)pParam)->FinalWorkCoordinateData.Y },
									((COrder*)pParam)->MosaicAreaJudge(((COrder*)pParam)->IntervalAreaCheck.at(i))
									/*{ ((COrder*)pParam)->IntervalAreaCheck.at(i).Image.Start.x,((COrder*)pParam)->IntervalAreaCheck.at(i).Image.Start.y,
									((COrder*)pParam)->IntervalAreaCheck.at(i).Image.End.x,((COrder*)pParam)->IntervalAreaCheck.at(i).Image.End.y }*/))
								{
									((COrder*)pParam)->IntervalAreaCheck.at(i).DotTrain.PointData.push_back({ ((COrder*)pParam)->FinalWorkCoordinateData.X ,((COrder*)pParam)->FinalWorkCoordinateData.Y });//加入訓練點
								}
							}
						}
					}
				}
			}
		}
	}
	else if (CommandResolve(Command, 0) == L"LineStart")
	{
		if (((COrder*)pParam)->ModelControl.Mode == 0)//建表模式
		{
			RecordCorrectionTable(pParam);//寫入修正表
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 1)//影像模式
		{
			RecordCorrectionTable(pParam);//寫入修正表
			VirtualCoordinateMove(pParam, Command, 1);//虛擬座標移動
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 2)//雷射模式
		{
			LaserDetectHandle(pParam, Command);//雷射測高和虛擬座標移動
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 3)//運動模式
		{
			((COrder*)pParam)->ActionCount++;//動作計數++
			ChooseVisionModify(pParam);//選擇影像Offset
			ChooseLaserModify(pParam);//選擇雷射高度
			ModifyPointOffSet(pParam, Command);//CallSubroutin相對位修正
			LineGotoActionJudge(pParam);//判斷動作狀態
			((COrder*)pParam)->FinalWorkCoordinateData.X = _ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X;
			((COrder*)pParam)->FinalWorkCoordinateData.Y = _ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y;
			((COrder*)pParam)->FinalWorkCoordinateData.Z = _ttol(CommandResolve(Command, 3)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z;
			((COrder*)pParam)->FinalWorkCoordinateData.W = _tstof(CommandResolve(Command, 4)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).W;
			/*因為不加入修正所以必須在這裡做汰換*/
			((COrder*)pParam)->NVMVirtualCoordinateData = ((COrder*)pParam)->FinalWorkCoordinateData;//紀錄CallSubroutine點(不加影像修正時的值)
			
			VisionModify(pParam);//影像修正
			LaserModify(pParam);//雷射修正
#ifdef PRINTF
			_cwprintf(L"SubroutineThread()::%s(%d,%d,%d,%.3f)\n", CommandResolve(Command, 0), ((COrder*)pParam)->FinalWorkCoordinateData.X, ((COrder*)pParam)->FinalWorkCoordinateData.Y, ((COrder*)pParam)->FinalWorkCoordinateData.Z, ((COrder*)pParam)->FinalWorkCoordinateData.W);
#endif
			((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Status = TRUE;
			//((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).X = _ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X;
			//((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Y = _ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y;
			//((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Z = _ttol(CommandResolve(Command, 3)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z;
			((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).X = ((COrder*)pParam)->FinalWorkCoordinateData.X;
			((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Y = ((COrder*)pParam)->FinalWorkCoordinateData.Y;
			((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Z = ((COrder*)pParam)->FinalWorkCoordinateData.Z;
			((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).W = ((COrder*)pParam)->FinalWorkCoordinateData.W;
			((COrder*)pParam)->RunData.ActionStatus.at(((COrder*)pParam)->Program.SubroutinCount) = 1;
		} 
	}
	else if (CommandResolve(Command, 0) == L"LinePassing")
	{
		if (((COrder*)pParam)->ModelControl.Mode == 0)//建表模式
		{
			RecordCorrectionTable(pParam);//寫入修正表
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 1)//影像模式
		{
			RecordCorrectionTable(pParam);//寫入修正表
			VirtualCoordinateMove(pParam, Command, 2);//虛擬座標移動
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 2)//雷射模式
		{
			LaserDetectHandle(pParam, Command);//雷射測高和虛擬座標移動
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 3)//運動模式
		{
			((COrder*)pParam)->ActionCount++;//動作計數++
			ChooseVisionModify(pParam);//選擇影像Offset
			ChooseLaserModify(pParam);//選擇雷射高度
			ModifyPointOffSet(pParam, Command);//CallSubroutin相對位修正
			if (((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Status)//有線段開始
			{
				((COrder*)pParam)->FinalWorkCoordinateData.X = _ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X;
				((COrder*)pParam)->FinalWorkCoordinateData.Y = _ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y;
				((COrder*)pParam)->FinalWorkCoordinateData.Z = _ttol(CommandResolve(Command, 3)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z;
				((COrder*)pParam)->FinalWorkCoordinateData.W = _tstof(CommandResolve(Command, 4)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).W;

				((COrder*)pParam)->NVMVirtualCoordinateData = ((COrder*)pParam)->FinalWorkCoordinateData;//紀錄CallSubroutine點(不加影像修正時的值)
				
				VisionModify(pParam);//影像修正
				LaserModify(pParam);//雷射修正
#ifdef PRINTF
				_cwprintf(L"SubroutineThread()::%s(%d,%d,%d,%.3f)\n", CommandResolve(Command, 0), ((COrder*)pParam)->FinalWorkCoordinateData.X, ((COrder*)pParam)->FinalWorkCoordinateData.Y, ((COrder*)pParam)->FinalWorkCoordinateData.Z, ((COrder*)pParam)->FinalWorkCoordinateData.W);
#endif
				//判斷是否是連續線段
				if (!((COrder*)pParam)->LaserContinuousControl.ContinuousSwitch)//不為連續線段
				{
					if (((COrder*)pParam)->RunData.ActionStatus.at(((COrder*)pParam)->Program.SubroutinCount) == 1)//LS存在尚未執行過LP
					{
						if (((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Status)//執行圓弧
						{
#ifdef MOVE
							//呼叫LS 
							((COrder*)pParam)->m_Action.DecideLineStartMove(
								((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).X,
								((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
								((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Z,
								((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).W,
								((COrder*)pParam)->DispenseLineSet.BeforeMoveDelay, ((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, ((COrder*)pParam)->LineSpeedSet.InitSpeed);
							//呼叫ARC
							((COrder*)pParam)->m_Action.DecideArc(
								((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).X,
								((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
								((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Z,
								((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).W,
								((COrder*)pParam)->FinalWorkCoordinateData.X,
								((COrder*)pParam)->FinalWorkCoordinateData.Y,
								((COrder*)pParam)->FinalWorkCoordinateData.Z,
								((COrder*)pParam)->FinalWorkCoordinateData.W,
								//_ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X,
								//_ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
								((COrder*)pParam)->LineSpeedSet.EndSpeed, 1000);
#endif
							//清除完成動作弧
							((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Status = FALSE;

							if (((COrder*)pParam)->CheckSwitch.Area)//區域區間檢測線段新增
							{
								for (UINT i = 0; i < ((COrder*)pParam)->IntervalAreaCheck.size(); i++)//判斷目前使用的區域區間 加入線段陣列
								{
									if (((COrder*)pParam)->IntervalAreaCheck.at(i).Address == ((COrder*)pParam)->CurrentCheckAddress)
									{
										//加入圓弧切點路徑+判斷是否在區域內
										((COrder*)pParam)->NewCutPathPoint(((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount),
											((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount),
											((COrder*)pParam)->FinalWorkCoordinateData, 
											((COrder*)pParam)->IntervalAreaCheck.at(i), 1);
									}
								}
							}
						}
						else if (((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Status)//執行圓
						{
#ifdef MOVE
							//呼叫LS
							((COrder*)pParam)->m_Action.DecideLineStartMove(
								((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).X,
								((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
								((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Z,
								((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).W,
								((COrder*)pParam)->DispenseLineSet.BeforeMoveDelay, ((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, ((COrder*)pParam)->LineSpeedSet.InitSpeed);
							//呼叫Circle
							((COrder*)pParam)->m_Action.DecideCircle(
								((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).X,
								((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Y,
								((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Z,
								((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).W,
								((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount).X,
								((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount).Y,
								((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount).Z,
								((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount).W,
								((COrder*)pParam)->LineSpeedSet.EndSpeed, 1000);
							//呼叫LP
							((COrder*)pParam)->m_Action.DecideLineMidMove(
								//_ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X,
								//_ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
								//_ttol(CommandResolve(Command, 3)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z,
								((COrder*)pParam)->FinalWorkCoordinateData.X,
								((COrder*)pParam)->FinalWorkCoordinateData.Y,
								((COrder*)pParam)->FinalWorkCoordinateData.Z,
								((COrder*)pParam)->FinalWorkCoordinateData.W,

								((COrder*)pParam)->DispenseLineSet.NodeTime, ((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, ((COrder*)pParam)->LineSpeedSet.InitSpeed);
#endif
							//清除完成動作圓
							((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Status = FALSE;

							if (((COrder*)pParam)->CheckSwitch.Area)//區域區間檢測線段新增
							{
								for (UINT i = 0; i < ((COrder*)pParam)->IntervalAreaCheck.size(); i++)//判斷目前使用的區域區間 加入線段陣列
								{
									if (((COrder*)pParam)->IntervalAreaCheck.at(i).Address == ((COrder*)pParam)->CurrentCheckAddress)
									{
										//加入圓切點路徑+判斷是否在區域內
										if (((COrder*)pParam)->NewCutPathPoint(((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount),
											((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount),
											((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount),
											((COrder*)pParam)->IntervalAreaCheck.at(i), 2))//圓在區域內	
										{
											//判斷線段是否在區域內
											if (((COrder*)pParam)->LineAreaJudge({ ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).X,((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Y },
											{ ((COrder*)pParam)->FinalWorkCoordinateData.X ,((COrder*)pParam)->FinalWorkCoordinateData.Y },
												((COrder*)pParam)->MosaicAreaJudge(((COrder*)pParam)->IntervalAreaCheck.at(i))
												/*{ ((COrder*)pParam)->IntervalAreaCheck.at(i).Image.Start.x,((COrder*)pParam)->IntervalAreaCheck.at(i).Image.Start.y,
													((COrder*)pParam)->IntervalAreaCheck.at(i).Image.End.x,((COrder*)pParam)->IntervalAreaCheck.at(i).Image.End.y }*/))
											{
												((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData.push_back({ ((COrder*)pParam)->FinalWorkCoordinateData.X ,((COrder*)pParam)->FinalWorkCoordinateData.Y });//中間點
												/**/((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData.push_back({ -1,-1 });
											}
										}
										else//圓不在區域內
										{
											//判斷線段是否在區域內
											if (((COrder*)pParam)->LineAreaJudge({ ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).X,((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Y },
											{ ((COrder*)pParam)->FinalWorkCoordinateData.X ,((COrder*)pParam)->FinalWorkCoordinateData.Y },
												((COrder*)pParam)->MosaicAreaJudge(((COrder*)pParam)->IntervalAreaCheck.at(i))
												/*{ ((COrder*)pParam)->IntervalAreaCheck.at(i).Image.Start.x,((COrder*)pParam)->IntervalAreaCheck.at(i).Image.Start.y,
													((COrder*)pParam)->IntervalAreaCheck.at(i).Image.End.x,((COrder*)pParam)->IntervalAreaCheck.at(i).Image.End.y }*/))//線段在區域內
											{
												((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData.push_back({ ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).X ,((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Y });//起始點
												((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData.push_back({ ((COrder*)pParam)->FinalWorkCoordinateData.X ,((COrder*)pParam)->FinalWorkCoordinateData.Y });//中間點
												/**/((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData.push_back({ -1,-1 });
											}
										}

										//2017/02/16前
										/*
											((COrder*)pParam)->NewCutPathPoint(((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount),
												((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount),
												((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount),
												((COrder*)pParam)->IntervalAreaCheck.at(i), 2);//加入圓切點路徑	
											((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData.push_back({ ((COrder*)pParam)->FinalWorkCoordinateData.X ,((COrder*)pParam)->FinalWorkCoordinateData.Y });//中間點
										*/
									}
								}
							}
						}
						else//執行線段
						{
#ifdef MOVE
							//呼叫LS-LP
							((COrder*)pParam)->m_Action.DecideLineSToP(
								((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).X,
								((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
								((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Z,
								((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).W,
								//_ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X,
								//_ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
								//_ttol(CommandResolve(Command, 3)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z,
								((COrder*)pParam)->FinalWorkCoordinateData.X,
								((COrder*)pParam)->FinalWorkCoordinateData.Y,
								((COrder*)pParam)->FinalWorkCoordinateData.Z,
								((COrder*)pParam)->FinalWorkCoordinateData.W,
								((COrder*)pParam)->DispenseLineSet.BeforeMoveDelay, ((COrder*)pParam)->DispenseLineSet.BeforeMoveDistance, ((COrder*)pParam)->DispenseLineSet.NodeTime,
								((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, ((COrder*)pParam)->LineSpeedSet.InitSpeed);
#endif
							if (((COrder*)pParam)->CheckSwitch.Area)//區域區間檢測線段新增
							{
								for (UINT i = 0; i < ((COrder*)pParam)->IntervalAreaCheck.size(); i++)//判斷目前使用的區域區間 加入線段陣列
								{
									if (((COrder*)pParam)->IntervalAreaCheck.at(i).Address == ((COrder*)pParam)->CurrentCheckAddress)
									{
										//判斷線段是否在區域內
										if (((COrder*)pParam)->LineAreaJudge({ ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).X,((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Y },
										{ ((COrder*)pParam)->FinalWorkCoordinateData.X ,((COrder*)pParam)->FinalWorkCoordinateData.Y },
											((COrder*)pParam)->MosaicAreaJudge(((COrder*)pParam)->IntervalAreaCheck.at(i))
											/*{ ((COrder*)pParam)->IntervalAreaCheck.at(i).Image.Start.x,((COrder*)pParam)->IntervalAreaCheck.at(i).Image.Start.y,
												((COrder*)pParam)->IntervalAreaCheck.at(i).Image.End.x,((COrder*)pParam)->IntervalAreaCheck.at(i).Image.End.y }*/))//線段在區域內
										{
											((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData.push_back({ ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).X ,((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Y });//起始點
											((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData.push_back({ ((COrder*)pParam)->FinalWorkCoordinateData.X ,((COrder*)pParam)->FinalWorkCoordinateData.Y });//中間點
											/**/((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData.push_back({ -1,-1 });
										}
									}
								}
							}
						}
						//不管何種檢測都要記錄中間點
						((COrder*)pParam)->AreaCheckChangTemp = { 0,((COrder*)pParam)->FinalWorkCoordinateData.X ,((COrder*)pParam)->FinalWorkCoordinateData.Y ,((COrder*)pParam)->FinalWorkCoordinateData.Z,((COrder*)pParam)->FinalWorkCoordinateData.W };//紀錄中間點
					}
					else if (((COrder*)pParam)->RunData.ActionStatus.at(((COrder*)pParam)->Program.SubroutinCount) == 2)//LS存在執行過LP
					{
						PassingException(pParam);//呼叫例外中間點判斷
						if (((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Status)//執行圓弧
						{
#ifdef MOVE
							//呼叫ARC
							((COrder*)pParam)->m_Action.DecideArc(
								((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).X,
								((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
								((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Z,
								((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).W,
								//_ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X,
								//_ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
								((COrder*)pParam)->FinalWorkCoordinateData.X,
								((COrder*)pParam)->FinalWorkCoordinateData.Y,
								((COrder*)pParam)->FinalWorkCoordinateData.Z,
								((COrder*)pParam)->FinalWorkCoordinateData.W,
								((COrder*)pParam)->LineSpeedSet.EndSpeed, 1000);
#endif
							//清除完成動作圓弧
							((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Status = FALSE;

							if (((COrder*)pParam)->CheckSwitch.Area)//區域區間檢測線段新增
							{
								for (UINT i = 0; i < ((COrder*)pParam)->IntervalAreaCheck.size(); i++)//判斷目前使用的區域區間 加入線段陣列
								{
									if (((COrder*)pParam)->IntervalAreaCheck.at(i).Address == ((COrder*)pParam)->CurrentCheckAddress)
									{
										//加入圓弧切點路徑+判斷是否在區域內
										((COrder*)pParam)->NewCutPathPoint(((COrder*)pParam)->AreaCheckChangTemp,
											((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount),
											((COrder*)pParam)->FinalWorkCoordinateData,
											((COrder*)pParam)->IntervalAreaCheck.at(i), 1);

										//2017/02/16前
										/*
											if (((COrder*)pParam)->AreaCheckChangTemp.Status)//成立的話代表為新的檢測區間
											{
												((COrder*)pParam)->NewCutPathPoint(((COrder*)pParam)->AreaCheckChangTemp,
													((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount),
													((COrder*)pParam)->FinalWorkCoordinateData,
													((COrder*)pParam)->IntervalAreaCheck.at(i), 1);//加入圓弧切點路徑
											}
											else
											{
												CoordinateData CoordinateTemp = { 0,((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData.back().x,((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData.back().y,0 };
												((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData.pop_back();//移除多的中間點
												((COrder*)pParam)->NewCutPathPoint(CoordinateTemp,
													((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount),
													((COrder*)pParam)->FinalWorkCoordinateData,
													((COrder*)pParam)->IntervalAreaCheck.at(i), 1);//加入圓弧切點路徑
											}
										*/
									}
								}
							}
						}
						else if (((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Status)//執行圓
						{
#ifdef MOVE
							//呼叫Circle
							((COrder*)pParam)->m_Action.DecideCircle(
								((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).X,
								((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Y,
								((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Z,
								((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).W,
								((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount).X,
								((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount).Y,
								((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount).Z,
								((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount).W,
								((COrder*)pParam)->LineSpeedSet.EndSpeed, 1000);
							//呼叫LP
							((COrder*)pParam)->m_Action.DecideLineMidMove(
								//_ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X,
								//_ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
								//_ttol(CommandResolve(Command, 3)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z,
								((COrder*)pParam)->FinalWorkCoordinateData.X,
								((COrder*)pParam)->FinalWorkCoordinateData.Y,
								((COrder*)pParam)->FinalWorkCoordinateData.Z,
								((COrder*)pParam)->FinalWorkCoordinateData.W,
								((COrder*)pParam)->DispenseLineSet.NodeTime, ((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, ((COrder*)pParam)->LineSpeedSet.InitSpeed);
#endif
							//清除完成動作圓
							((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Status = FALSE;

							if (((COrder*)pParam)->CheckSwitch.Area)//區域區間檢測線段新增
							{
								for (UINT i = 0; i < ((COrder*)pParam)->IntervalAreaCheck.size(); i++)//判斷目前使用的區域區間 加入線段陣列
								{
									if (((COrder*)pParam)->IntervalAreaCheck.at(i).Address == ((COrder*)pParam)->CurrentCheckAddress)
									{
										//加入圓切點路徑+判斷是否在區域內
										if (((COrder*)pParam)->NewCutPathPoint(((COrder*)pParam)->AreaCheckChangTemp,
											((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount),
											((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount),
											((COrder*)pParam)->IntervalAreaCheck.at(i), 2))//圓在區域內	
										{
											//判斷線段是否在區域內
											if (((COrder*)pParam)->LineAreaJudge({ ((COrder*)pParam)->AreaCheckChangTemp.X,((COrder*)pParam)->AreaCheckChangTemp.Y },
											{ ((COrder*)pParam)->FinalWorkCoordinateData.X ,((COrder*)pParam)->FinalWorkCoordinateData.Y },
												((COrder*)pParam)->MosaicAreaJudge(((COrder*)pParam)->IntervalAreaCheck.at(i))
												/*{ ((COrder*)pParam)->IntervalAreaCheck.at(i).Image.Start.x,((COrder*)pParam)->IntervalAreaCheck.at(i).Image.Start.y,
													((COrder*)pParam)->IntervalAreaCheck.at(i).Image.End.x,((COrder*)pParam)->IntervalAreaCheck.at(i).Image.End.y }*/))
											{
												((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData.push_back({ ((COrder*)pParam)->FinalWorkCoordinateData.X ,((COrder*)pParam)->FinalWorkCoordinateData.Y });//中間點
												/**/((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData.push_back({ -1,-1 });
											}
										}
										else//圓不在區域內
										{
											//判斷線段是否在區域內
											if (((COrder*)pParam)->LineAreaJudge({ ((COrder*)pParam)->AreaCheckChangTemp.X,((COrder*)pParam)->AreaCheckChangTemp.Y },
											{ ((COrder*)pParam)->FinalWorkCoordinateData.X ,((COrder*)pParam)->FinalWorkCoordinateData.Y },
												((COrder*)pParam)->MosaicAreaJudge(((COrder*)pParam)->IntervalAreaCheck.at(i))
												/*{ ((COrder*)pParam)->IntervalAreaCheck.at(i).Image.Start.x,((COrder*)pParam)->IntervalAreaCheck.at(i).Image.Start.y,
													((COrder*)pParam)->IntervalAreaCheck.at(i).Image.End.x,((COrder*)pParam)->IntervalAreaCheck.at(i).Image.End.y }*/))//線段在區域內
											{
												((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData.push_back({ ((COrder*)pParam)->AreaCheckChangTemp.X ,((COrder*)pParam)->AreaCheckChangTemp.Y });//換線點
												((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData.push_back({ ((COrder*)pParam)->FinalWorkCoordinateData.X ,((COrder*)pParam)->FinalWorkCoordinateData.Y });//中間點
												/**/((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData.push_back({ -1,-1 });
											}
										}

										//2017/02/16前
										/*
											if (((COrder*)pParam)->AreaCheckChangTemp.Status)//成立的話代表為新的檢測區間
											{
												NewCutPathPoint(((COrder*)pParam)->AreaCheckChangTemp,
													((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount),
													((COrder*)pParam)->FinalWorkCoordinateData,
													((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData, 1);//加入圓弧切點路徑
											}
											else
											{
												CoordinateData CoordinateTemp = { 0,((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData.back().x,((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData.back().y,0 };
												((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData.pop_back();//移除多的中間點
												NewCutPathPoint(CoordinateTemp,
													((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount),
													((COrder*)pParam)->FinalWorkCoordinateData,
													((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData, 1);//加入圓弧切點路徑
											}
											((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData.push_back({ ((COrder*)pParam)->FinalWorkCoordinateData.X ,((COrder*)pParam)->FinalWorkCoordinateData.Y });//中間點
										*/
									}
								}
							}
						}
						else//執行線段
						{
#ifdef MOVE
							//呼叫LP
							((COrder*)pParam)->m_Action.DecideLineMidMove(
								//_ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X,
								//_ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
								//_ttol(CommandResolve(Command, 3)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z,
								((COrder*)pParam)->FinalWorkCoordinateData.X,
								((COrder*)pParam)->FinalWorkCoordinateData.Y,
								((COrder*)pParam)->FinalWorkCoordinateData.Z,
								((COrder*)pParam)->FinalWorkCoordinateData.W,
								((COrder*)pParam)->DispenseLineSet.NodeTime, ((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, ((COrder*)pParam)->LineSpeedSet.InitSpeed);
#endif
							if (((COrder*)pParam)->CheckSwitch.Area)//區域區間檢測線段新增
							{
								for (UINT i = 0; i < ((COrder*)pParam)->IntervalAreaCheck.size(); i++)//判斷目前使用的區間 加入線段陣列
								{
									if (((COrder*)pParam)->IntervalAreaCheck.at(i).Address == ((COrder*)pParam)->CurrentCheckAddress)
									{
										//判斷線段是否在區域內
										if (((COrder*)pParam)->LineAreaJudge({ ((COrder*)pParam)->AreaCheckChangTemp.X,((COrder*)pParam)->AreaCheckChangTemp.Y },
										{ ((COrder*)pParam)->FinalWorkCoordinateData.X ,((COrder*)pParam)->FinalWorkCoordinateData.Y },
											((COrder*)pParam)->MosaicAreaJudge(((COrder*)pParam)->IntervalAreaCheck.at(i))
											/*{ ((COrder*)pParam)->IntervalAreaCheck.at(i).Image.Start.x,((COrder*)pParam)->IntervalAreaCheck.at(i).Image.Start.y,
												((COrder*)pParam)->IntervalAreaCheck.at(i).Image.End.x,((COrder*)pParam)->IntervalAreaCheck.at(i).Image.End.y }*/))//線段在區域內
										{
											((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData.push_back({ ((COrder*)pParam)->AreaCheckChangTemp.X,((COrder*)pParam)->AreaCheckChangTemp.Y });//起始點
											((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData.push_back({ ((COrder*)pParam)->FinalWorkCoordinateData.X ,((COrder*)pParam)->FinalWorkCoordinateData.Y });//中間點
											/**/((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData.push_back({ -1,-1 });
										}

										//2017/02/16前
										/*
											if (((COrder*)pParam)->AreaCheckChangTemp.Status)
											{
												((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData.push_back({ ((COrder*)pParam)->AreaCheckChangTemp.X,((COrder*)pParam)->AreaCheckChangTemp.Y });//區域中間點轉換點
											}
											((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData.push_back({ ((COrder*)pParam)->FinalWorkCoordinateData.X ,((COrder*)pParam)->FinalWorkCoordinateData.Y });//中間點
											*/
									}
								}
							}
						}
						//不管何種檢測都要記錄中間點
						((COrder*)pParam)->AreaCheckChangTemp = { 0,((COrder*)pParam)->FinalWorkCoordinateData.X ,((COrder*)pParam)->FinalWorkCoordinateData.Y ,((COrder*)pParam)->FinalWorkCoordinateData.Z,((COrder*)pParam)->FinalWorkCoordinateData.W };//紀錄中間點
					}
				}
				else//連續線段
				{
					/*DEMO中間點不選擇線段*/
#ifdef PRINTF
					_cwprintf(L"SubroutineThread()::選擇線段:%d\n", abs(((COrder*)pParam)->FinalWorkCoordinateData.Z + 10000));  
					
#endif  
					if (!((COrder*)pParam)->DemoTemprarilySwitch)//正常使用
					{
#ifdef MOVE
						((COrder*)pParam)->m_Action.LA_Line3DtoDo(abs(((COrder*)pParam)->FinalWorkCoordinateData.Z + 10000),
							((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, ((COrder*)pParam)->LineSpeedSet.InitSpeed);
						((COrder*)pParam)->LaserContinuousControl.ContinuousSwitch = FALSE;
#endif
					}
				}
				((COrder*)pParam)->RunData.ActionStatus.at(((COrder*)pParam)->Program.SubroutinCount) = 2;//狀態變為線段中
				((COrder*)pParam)->VirtualCoordinateData = ((COrder*)pParam)->FinalWorkCoordinateData;//紀錄移動虛擬座標
			}
		} 
	}
	else if (CommandResolve(Command, 0) == L"LineEnd")
	{
		if (((COrder*)pParam)->ModelControl.Mode == 0)//建表模式
		{
			RecordCorrectionTable(pParam);//寫入修正表
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 1)//影像模式
		{
			RecordCorrectionTable(pParam);//寫入修正表
			VirtualCoordinateMove(pParam, Command, 2);//虛擬座標移動
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 2)//雷射模式
		{
			LaserDetectHandle(pParam, Command);//雷射測高和虛擬座標移動
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 3)//運動模式
		{
			((COrder*)pParam)->ActionCount++;//動作計數++
			ChooseVisionModify(pParam);//選擇影像Offset
			ChooseLaserModify(pParam);//選擇雷射高度
			ModifyPointOffSet(pParam, Command);//CallSubroutin相對位修正
			if (((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Status)//有線段開始
			{
				((COrder*)pParam)->FinalWorkCoordinateData.X = _ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X;
				((COrder*)pParam)->FinalWorkCoordinateData.Y = _ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y;
				((COrder*)pParam)->FinalWorkCoordinateData.Z = _ttol(CommandResolve(Command, 3)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z;
				((COrder*)pParam)->FinalWorkCoordinateData.W = _tstof(CommandResolve(Command, 4)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).W;

				((COrder*)pParam)->NVMVirtualCoordinateData = ((COrder*)pParam)->FinalWorkCoordinateData;//紀錄CallSubroutine點(不加影像修正時的值)
				
				VisionModify(pParam);//影像修正
				LaserModify(pParam);//雷射修正
#ifdef PRINTF
				_cwprintf(L"SubroutineThread()::%s(%d,%d,%d,%.3f)\n", CommandResolve(Command, 0), ((COrder*)pParam)->FinalWorkCoordinateData.X, ((COrder*)pParam)->FinalWorkCoordinateData.Y, ((COrder*)pParam)->FinalWorkCoordinateData.Z, ((COrder*)pParam)->FinalWorkCoordinateData.W);
#endif
				//判斷是否為連續線段
				if (!((COrder*)pParam)->LaserContinuousControl.ContinuousSwitch)//不為連續線段
				{
					if (((COrder*)pParam)->RunData.ActionStatus.at(((COrder*)pParam)->Program.SubroutinCount) == 1)//LS存在且尚未執行過
					{
						if (((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Status)//執行圓弧
						{
#ifdef MOVE
							//呼叫一個LS
							((COrder*)pParam)->m_Action.DecideLineStartMove(
								((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).X,
								((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
								((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Z,
								((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).W,
								((COrder*)pParam)->DispenseLineSet.BeforeMoveDelay, ((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, ((COrder*)pParam)->LineSpeedSet.InitSpeed);
							//再呼叫一個ARC-LE
							((COrder*)pParam)->m_Action.DecideArclePToEnd(
								((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).X,
								((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
								((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Z,
								((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).W,
								//_ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X,
								//_ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
								((COrder*)pParam)->FinalWorkCoordinateData.X,
								((COrder*)pParam)->FinalWorkCoordinateData.Y,
								((COrder*)pParam)->FinalWorkCoordinateData.Z,
								((COrder*)pParam)->FinalWorkCoordinateData.W,
								((COrder*)pParam)->DispenseLineSet.StayTime, ((COrder*)pParam)->DispenseLineSet.ShutdownDistance, ((COrder*)pParam)->DispenseLineSet.ShutdownDelay,
								((COrder*)pParam)->ZSet.ZBackHeight, ((COrder*)pParam)->ZSet.ZBackType,
								((COrder*)pParam)->DispenseLineEnd.HighSpeed, ((COrder*)pParam)->DispenseLineEnd.Width, ((COrder*)pParam)->DispenseLineEnd.Height, ((COrder*)pParam)->DispenseLineEnd.LowSpeed, ((COrder*)pParam)->DispenseLineEnd.Type,
								((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, 1000);
#endif
							//清除完成動作圓弧
							((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Status = FALSE;

							if (((COrder*)pParam)->CheckSwitch.Area)//區域區間檢測線段新增
							{
								for (UINT i = 0; i < ((COrder*)pParam)->IntervalAreaCheck.size(); i++)//判斷目前使用的區間 加入線段陣列
								{
									if (((COrder*)pParam)->IntervalAreaCheck.at(i).Address == ((COrder*)pParam)->CurrentCheckAddress)
									{
										((COrder*)pParam)->NewCutPathPoint(((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount),
											((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount),
											((COrder*)pParam)->FinalWorkCoordinateData,
											((COrder*)pParam)->IntervalAreaCheck.at(i), 1);//加入圓弧切點路徑

										//2017/02/16前
										/*
											((COrder*)pParam)->NewCutPathPoint(((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount),
												((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount),
												((COrder*)pParam)->FinalWorkCoordinateData,
												((COrder*)pParam)->IntervalAreaCheck.at(i), 1);//加入圓弧切點路徑
											((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData.push_back({ -1,-1 });//換線點	
										*/
									}
								}
							}
						}
						else if (((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Status)//執行圓
						{
#ifdef MOVE
							//呼叫LS
							((COrder*)pParam)->m_Action.DecideLineStartMove(
								((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).X,
								((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
								((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Z,
								((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).W,
								((COrder*)pParam)->DispenseLineSet.BeforeMoveDelay, ((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, ((COrder*)pParam)->LineSpeedSet.InitSpeed);
							//呼叫Circle-LE
							((COrder*)pParam)->m_Action.DecideCirclePToEnd(
								((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).X,
								((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Y,
								((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Z,
								((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).W,
								((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount).X,
								((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount).Y,
								((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount).Z,
								((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount).W,
								//_ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X,
								//_ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
								((COrder*)pParam)->FinalWorkCoordinateData.X,
								((COrder*)pParam)->FinalWorkCoordinateData.Y,
								((COrder*)pParam)->FinalWorkCoordinateData.Z,
								((COrder*)pParam)->FinalWorkCoordinateData.W,
								((COrder*)pParam)->DispenseLineSet.StayTime, ((COrder*)pParam)->DispenseLineSet.ShutdownDistance, ((COrder*)pParam)->DispenseLineSet.ShutdownDelay,
								((COrder*)pParam)->ZSet.ZBackHeight, ((COrder*)pParam)->ZSet.ZBackType,
								((COrder*)pParam)->DispenseLineEnd.HighSpeed, ((COrder*)pParam)->DispenseLineEnd.Width, ((COrder*)pParam)->DispenseLineEnd.Height, ((COrder*)pParam)->DispenseLineEnd.LowSpeed, ((COrder*)pParam)->DispenseLineEnd.Type,
								((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, 1000);
#endif
							//清除完成動作圓
							((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Status = FALSE;

							if (((COrder*)pParam)->CheckSwitch.Area)//區域區間檢測線段新增
							{
								for (UINT i = 0; i < ((COrder*)pParam)->IntervalAreaCheck.size(); i++)//判斷目前使用的區間 加入線段陣列
								{
									if (((COrder*)pParam)->IntervalAreaCheck.at(i).Address == ((COrder*)pParam)->CurrentCheckAddress)
									{
										//加入圓切點路徑+判斷是否在區域內
										if (((COrder*)pParam)->NewCutPathPoint(((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount),
											((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount),
											((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount),
											((COrder*)pParam)->IntervalAreaCheck.at(i), 2))//圓在區域內	
										{
											if (((COrder*)pParam)->LineAreaJudge({ ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).X,((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Y },
											{ ((COrder*)pParam)->FinalWorkCoordinateData.X ,((COrder*)pParam)->FinalWorkCoordinateData.Y },
												((COrder*)pParam)->MosaicAreaJudge(((COrder*)pParam)->IntervalAreaCheck.at(i))
												/*{ ((COrder*)pParam)->IntervalAreaCheck.at(i).Image.Start.x,((COrder*)pParam)->IntervalAreaCheck.at(i).Image.Start.y,
													((COrder*)pParam)->IntervalAreaCheck.at(i).Image.End.x,((COrder*)pParam)->IntervalAreaCheck.at(i).Image.End.y }*/))
											{
												((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData.push_back({ ((COrder*)pParam)->FinalWorkCoordinateData.X ,((COrder*)pParam)->FinalWorkCoordinateData.Y });//中間點
												((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData.push_back({ -1,-1 });
											}
										}
										else//圓不在區域內
										{
											//判斷線段是否在區域內
											if (((COrder*)pParam)->LineAreaJudge({ ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).X,((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Y },
											{ ((COrder*)pParam)->FinalWorkCoordinateData.X ,((COrder*)pParam)->FinalWorkCoordinateData.Y },
												((COrder*)pParam)->MosaicAreaJudge(((COrder*)pParam)->IntervalAreaCheck.at(i))
												/*{ ((COrder*)pParam)->IntervalAreaCheck.at(i).Image.Start.x,((COrder*)pParam)->IntervalAreaCheck.at(i).Image.Start.y,
													((COrder*)pParam)->IntervalAreaCheck.at(i).Image.End.x,((COrder*)pParam)->IntervalAreaCheck.at(i).Image.End.y }*/))//線段在區域內
											{
												((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData.push_back({ ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).X ,((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Y });//起始點
												((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData.push_back({ ((COrder*)pParam)->FinalWorkCoordinateData.X ,((COrder*)pParam)->FinalWorkCoordinateData.Y });//中間點
												((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData.push_back({ -1,-1 });
											}
										}
										//2017/02/16前
										/*
										((COrder*)pParam)->NewCutPathPoint(((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount),
											((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount),
											((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount),
											((COrder*)pParam)->IntervalAreaCheck.at(i), 2);//加入圓切點路徑
										((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData.push_back({ ((COrder*)pParam)->FinalWorkCoordinateData.X ,((COrder*)pParam)->FinalWorkCoordinateData.Y });//結束點
										((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData.push_back({ -1,-1 });//換線點
										*/
									}
								}
							}
						}
						else//執行線段
						{
#ifdef MOVE
							//呼叫LS-LE
							((COrder*)pParam)->m_Action.DecideLineSToE(
								((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).X,
								((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
								((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Z,
								((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).W,
								//_ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X,
								//_ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
								//_ttol(CommandResolve(Command, 3)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z,
								((COrder*)pParam)->FinalWorkCoordinateData.X,
								((COrder*)pParam)->FinalWorkCoordinateData.Y,
								((COrder*)pParam)->FinalWorkCoordinateData.Z,
								((COrder*)pParam)->FinalWorkCoordinateData.W,
								((COrder*)pParam)->DispenseLineSet.BeforeMoveDelay, ((COrder*)pParam)->DispenseLineSet.BeforeMoveDistance,
								((COrder*)pParam)->DispenseLineSet.StayTime, ((COrder*)pParam)->DispenseLineSet.ShutdownDistance, ((COrder*)pParam)->DispenseLineSet.ShutdownDelay,
								((COrder*)pParam)->ZSet.ZBackHeight, ((COrder*)pParam)->ZSet.ZBackType,
								((COrder*)pParam)->DispenseLineEnd.HighSpeed, ((COrder*)pParam)->DispenseLineEnd.Width, ((COrder*)pParam)->DispenseLineEnd.Height, ((COrder*)pParam)->DispenseLineEnd.LowSpeed, ((COrder*)pParam)->DispenseLineEnd.Type,
								((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, ((COrder*)pParam)->LineSpeedSet.InitSpeed);
#endif
							if (((COrder*)pParam)->CheckSwitch.Area)//區域區間檢測線段新增
							{
								for (UINT i = 0; i < ((COrder*)pParam)->IntervalAreaCheck.size(); i++)//判斷目前使用的區間 加入線段陣列
								{
									if (((COrder*)pParam)->IntervalAreaCheck.at(i).Address == ((COrder*)pParam)->CurrentCheckAddress)
									{
										//判斷線段是否在區域內
										if (((COrder*)pParam)->LineAreaJudge({ ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).X,((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Y },
										{ ((COrder*)pParam)->FinalWorkCoordinateData.X ,((COrder*)pParam)->FinalWorkCoordinateData.Y },
											((COrder*)pParam)->MosaicAreaJudge(((COrder*)pParam)->IntervalAreaCheck.at(i))
											/*{ ((COrder*)pParam)->IntervalAreaCheck.at(i).Image.Start.x,((COrder*)pParam)->IntervalAreaCheck.at(i).Image.Start.y,
												((COrder*)pParam)->IntervalAreaCheck.at(i).Image.End.x,((COrder*)pParam)->IntervalAreaCheck.at(i).Image.End.y }*/))//線段在區域內
										{
											((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData.push_back({ ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).X ,((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Y });//起始點
											((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData.push_back({ ((COrder*)pParam)->FinalWorkCoordinateData.X ,((COrder*)pParam)->FinalWorkCoordinateData.Y });//中間點
											((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData.push_back({ -1,-1 });
										}

										//2017/02/16前
										/*
										((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData.push_back({ ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).X ,((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Y });//起始點
										((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData.push_back({ ((COrder*)pParam)->FinalWorkCoordinateData.X ,((COrder*)pParam)->FinalWorkCoordinateData.Y });//結束點
										((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData.push_back({ -1,-1 });//換線點
										*/
									}
								}
							}
						}
						((COrder*)pParam)->AreaCheckChangTemp = { 0,-1,-1,-1,-1 };//區域中間點轉換檢測暫存參數清零
					}
					else if (((COrder*)pParam)->RunData.ActionStatus.at(((COrder*)pParam)->Program.SubroutinCount) == 2)//LS存在執行過LP
					{
						PassingException(pParam);//呼叫例外中間點判斷
						if (((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Status)//執行圓弧
						{
#ifdef MOVE
							//呼叫ARC-LE
							((COrder*)pParam)->m_Action.DecideArclePToEnd(
								((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).X,
								((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
								((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Z,
								((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).W,
								//_ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X,
								//_ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
								((COrder*)pParam)->FinalWorkCoordinateData.X,
								((COrder*)pParam)->FinalWorkCoordinateData.Y,
								((COrder*)pParam)->FinalWorkCoordinateData.Z,
								((COrder*)pParam)->FinalWorkCoordinateData.W,
								((COrder*)pParam)->DispenseLineSet.StayTime, ((COrder*)pParam)->DispenseLineSet.ShutdownDistance, ((COrder*)pParam)->DispenseLineSet.ShutdownDelay,
								((COrder*)pParam)->ZSet.ZBackHeight, ((COrder*)pParam)->ZSet.ZBackType,
								((COrder*)pParam)->DispenseLineEnd.HighSpeed, ((COrder*)pParam)->DispenseLineEnd.Width, ((COrder*)pParam)->DispenseLineEnd.Height, ((COrder*)pParam)->DispenseLineEnd.LowSpeed, ((COrder*)pParam)->DispenseLineEnd.Type,
								((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, 1000);
#endif
							//清除完成動作弧
							((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Status = FALSE;

							if (((COrder*)pParam)->CheckSwitch.Area)//區域區間檢測線段新增
							{
								for (UINT i = 0; i < ((COrder*)pParam)->IntervalAreaCheck.size(); i++)//判斷目前使用的區間 加入線段陣列
								{
									if (((COrder*)pParam)->IntervalAreaCheck.at(i).Address == ((COrder*)pParam)->CurrentCheckAddress)
									{
										((COrder*)pParam)->NewCutPathPoint(((COrder*)pParam)->AreaCheckChangTemp,
											((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount),
											((COrder*)pParam)->FinalWorkCoordinateData,
											((COrder*)pParam)->IntervalAreaCheck.at(i), 1);//加入圓切點路徑
										//2017/02/16前
										/*
										if (((COrder*)pParam)->AreaCheckChangTemp.Status)
										{
											((COrder*)pParam)->NewCutPathPoint(((COrder*)pParam)->AreaCheckChangTemp,
												((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount),
												((COrder*)pParam)->FinalWorkCoordinateData,
												((COrder*)pParam)->IntervalAreaCheck.at(i), 1);//加入圓切點路徑
											((COrder*)pParam)->AreaCheckChangTemp = { 0,0,0,0 };//區域中間點轉換檢測暫存參數清零	
										}
										else
										{
											CoordinateData CoordinateTemp = { 0,((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData.back().x,((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData.back().y,0 };
											((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData.pop_back();//移除多的中間點
											((COrder*)pParam)->NewCutPathPoint(CoordinateTemp,
												((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount),
												((COrder*)pParam)->FinalWorkCoordinateData,
												((COrder*)pParam)->IntervalAreaCheck.at(i), 1);//加入圓切點路徑
										}
										((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData.push_back({ -1,-1 });//換線點
										*/
									}
								}
							}
						}
						else if (((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Status)//執行圓
						{
#ifdef MOVE
							//呼叫Circle-LE
							((COrder*)pParam)->m_Action.DecideCirclePToEnd(
								((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).X,
								((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Y,
								((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Z,
								((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).W,
								((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount).X,
								((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount).Y,
								((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount).Z,
								((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount).W,
								//_ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X,
								//_ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
								((COrder*)pParam)->FinalWorkCoordinateData.X,
								((COrder*)pParam)->FinalWorkCoordinateData.Y,
								((COrder*)pParam)->FinalWorkCoordinateData.Z,
								((COrder*)pParam)->FinalWorkCoordinateData.W,
								((COrder*)pParam)->DispenseLineSet.StayTime, ((COrder*)pParam)->DispenseLineSet.ShutdownDistance, ((COrder*)pParam)->DispenseLineSet.ShutdownDelay,
								((COrder*)pParam)->ZSet.ZBackHeight, ((COrder*)pParam)->ZSet.ZBackType,
								((COrder*)pParam)->DispenseLineEnd.HighSpeed, ((COrder*)pParam)->DispenseLineEnd.Width, ((COrder*)pParam)->DispenseLineEnd.Height, ((COrder*)pParam)->DispenseLineEnd.LowSpeed, ((COrder*)pParam)->DispenseLineEnd.Type,
								((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, 1000);
#endif
							//清除完成動作圓
							((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Status = FALSE;

							if (((COrder*)pParam)->CheckSwitch.Area)//區域區間檢測線段新增
							{
								for (UINT i = 0; i < ((COrder*)pParam)->IntervalAreaCheck.size(); i++)//判斷目前使用的區間 加入線段陣列
								{
									if (((COrder*)pParam)->IntervalAreaCheck.at(i).Address == ((COrder*)pParam)->CurrentCheckAddress)
									{
										//加入圓切點路徑+判斷是否在區域內
										if (((COrder*)pParam)->NewCutPathPoint(((COrder*)pParam)->AreaCheckChangTemp,
											((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount),
											((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount),
											((COrder*)pParam)->IntervalAreaCheck.at(i), 2))//圓在區域內	
										{
											//判斷線段是否在區域內
											if (((COrder*)pParam)->LineAreaJudge({ ((COrder*)pParam)->AreaCheckChangTemp.X,((COrder*)pParam)->AreaCheckChangTemp.Y },
											{ ((COrder*)pParam)->FinalWorkCoordinateData.X ,((COrder*)pParam)->FinalWorkCoordinateData.Y },
												((COrder*)pParam)->MosaicAreaJudge(((COrder*)pParam)->IntervalAreaCheck.at(i))
												/*{ ((COrder*)pParam)->IntervalAreaCheck.at(i).Image.Start.x,((COrder*)pParam)->IntervalAreaCheck.at(i).Image.Start.y,
													((COrder*)pParam)->IntervalAreaCheck.at(i).Image.End.x,((COrder*)pParam)->IntervalAreaCheck.at(i).Image.End.y }*/))
											{
												((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData.push_back({ ((COrder*)pParam)->FinalWorkCoordinateData.X ,((COrder*)pParam)->FinalWorkCoordinateData.Y });//中間點
												((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData.push_back({ -1,-1 });
											}
										}
										else//圓不在區域內
										{
											//判斷線段是否在區域內
											if (((COrder*)pParam)->LineAreaJudge({ ((COrder*)pParam)->AreaCheckChangTemp.X,((COrder*)pParam)->AreaCheckChangTemp.Y },
											{ ((COrder*)pParam)->FinalWorkCoordinateData.X ,((COrder*)pParam)->FinalWorkCoordinateData.Y },
												((COrder*)pParam)->MosaicAreaJudge(((COrder*)pParam)->IntervalAreaCheck.at(i))
												/*{ ((COrder*)pParam)->IntervalAreaCheck.at(i).Image.Start.x,((COrder*)pParam)->IntervalAreaCheck.at(i).Image.Start.y,
													((COrder*)pParam)->IntervalAreaCheck.at(i).Image.End.x,((COrder*)pParam)->IntervalAreaCheck.at(i).Image.End.y }*/))//線段在區域內
											{
												((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData.push_back({ ((COrder*)pParam)->AreaCheckChangTemp.X,((COrder*)pParam)->AreaCheckChangTemp.Y });//起始點
												((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData.push_back({ ((COrder*)pParam)->FinalWorkCoordinateData.X ,((COrder*)pParam)->FinalWorkCoordinateData.Y });//中間點
												((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData.push_back({ -1,-1 });
											}
										}

										//2017/02/16前
										/*
										if (((COrder*)pParam)->AreaCheckChangTemp.Status)
										{
											((COrder*)pParam)->NewCutPathPoint(((COrder*)pParam)->AreaCheckChangTemp,
												((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount),
												((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount),
												((COrder*)pParam)->IntervalAreaCheck.at(i), 2);//加入圓切點路徑
											((COrder*)pParam)->AreaCheckChangTemp = { 0,0,0,0 };//區域中間點轉換檢測暫存參數清零
										}
										else
										{
											CoordinateData CoordinateTemp = { 0,((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData.back().x,((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData.back().y,0 };
											((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData.pop_back();//移除多的中間點
											((COrder*)pParam)->NewCutPathPoint(CoordinateTemp,
												((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount),
												((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount),
												((COrder*)pParam)->IntervalAreaCheck.at(i), 2);//加入圓切點路徑
										}
										((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData.push_back({ ((COrder*)pParam)->FinalWorkCoordinateData.X ,((COrder*)pParam)->FinalWorkCoordinateData.Y });//結束點
										((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData.push_back({ -1,-1 });//換線點
										*/
									}
								}
							}
						}
						else//執行線段
						{
#ifdef MOVE
							//呼叫LE
							((COrder*)pParam)->m_Action.DecideLineEndMove(
								//_ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X,
								//_ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
								//_ttol(CommandResolve(Command, 3)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z,
								((COrder*)pParam)->FinalWorkCoordinateData.X,
								((COrder*)pParam)->FinalWorkCoordinateData.Y,
								((COrder*)pParam)->FinalWorkCoordinateData.Z,
								((COrder*)pParam)->FinalWorkCoordinateData.W,
								((COrder*)pParam)->DispenseLineSet.StayTime, ((COrder*)pParam)->DispenseLineSet.ShutdownDistance, ((COrder*)pParam)->DispenseLineSet.ShutdownDelay,
								((COrder*)pParam)->ZSet.ZBackHeight, ((COrder*)pParam)->ZSet.ZBackType,
								((COrder*)pParam)->DispenseLineEnd.HighSpeed, ((COrder*)pParam)->DispenseLineEnd.Width, ((COrder*)pParam)->DispenseLineEnd.Height, ((COrder*)pParam)->DispenseLineEnd.LowSpeed, ((COrder*)pParam)->DispenseLineEnd.Type,
								((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, ((COrder*)pParam)->LineSpeedSet.InitSpeed);
#endif
							if (((COrder*)pParam)->CheckSwitch.Area)//區域區間檢測線段新增
							{
								for (UINT i = 0; i < ((COrder*)pParam)->IntervalAreaCheck.size(); i++)//判斷目前使用的區間 加入線段陣列
								{
									if (((COrder*)pParam)->IntervalAreaCheck.at(i).Address == ((COrder*)pParam)->CurrentCheckAddress)
									{
										//判斷線段是否在區域內
										if (((COrder*)pParam)->LineAreaJudge({ ((COrder*)pParam)->AreaCheckChangTemp.X,((COrder*)pParam)->AreaCheckChangTemp.Y },
										{ ((COrder*)pParam)->FinalWorkCoordinateData.X ,((COrder*)pParam)->FinalWorkCoordinateData.Y },
										{ ((COrder*)pParam)->IntervalAreaCheck.at(i).Image.Start.x,((COrder*)pParam)->IntervalAreaCheck.at(i).Image.Start.y,
											((COrder*)pParam)->IntervalAreaCheck.at(i).Image.End.x,((COrder*)pParam)->IntervalAreaCheck.at(i).Image.End.y }))//線段在區域內
										{
											((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData.push_back({ ((COrder*)pParam)->AreaCheckChangTemp.X,((COrder*)pParam)->AreaCheckChangTemp.Y });//起始點
											((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData.push_back({ ((COrder*)pParam)->FinalWorkCoordinateData.X ,((COrder*)pParam)->FinalWorkCoordinateData.Y });//中間點
											((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData.push_back({ -1,-1 });
										}

										//2017/02/16前
										/*
										if (((COrder*)pParam)->AreaCheckChangTemp.Status)
										{
											((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData.push_back({ ((COrder*)pParam)->AreaCheckChangTemp.X,((COrder*)pParam)->AreaCheckChangTemp.Y });//區域中間點轉換點
											((COrder*)pParam)->AreaCheckChangTemp = { 0,0,0,0 };//區域中間點轉換檢測暫存參數清零
										}
										((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData.push_back({ ((COrder*)pParam)->FinalWorkCoordinateData.X ,((COrder*)pParam)->FinalWorkCoordinateData.Y });//結束點
										((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData.push_back({ -1,-1 });//換線點
										*/
									}
								}
							}
						}
						((COrder*)pParam)->AreaCheckChangTemp = { 0,-1,-1,-1,-1 };//區域中間點轉換檢測暫存參數清零
					}
				}
				else//連續線段
				{
#ifdef PRINTF
					_cwprintf(L"SubroutineThread()::選擇線段:%d\n", abs(((COrder*)pParam)->FinalWorkCoordinateData.Z + 10000));
#endif  
					if (!((COrder*)pParam)->DemoTemprarilySwitch)//正常用
					{
#ifdef MOVE
						((COrder*)pParam)->m_Action.LA_Line3DtoDo(abs(((COrder*)pParam)->FinalWorkCoordinateData.Z + 10000),
							((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, ((COrder*)pParam)->LineSpeedSet.InitSpeed);
						//呼叫斷膠抬升
						((COrder*)pParam)->m_Action.DecideLineEndMove(((COrder*)pParam)->DispenseLineSet.StayTime, ((COrder*)pParam)->DispenseLineSet.ShutdownDelay,
							((COrder*)pParam)->ZSet.ZBackHeight, ((COrder*)pParam)->ZSet.ZBackType, ((COrder*)pParam)->DispenseLineEnd.HighSpeed, ((COrder*)pParam)->DispenseLineEnd.Width, ((COrder*)pParam)->DispenseLineEnd.Height, ((COrder*)pParam)->DispenseLineEnd.LowSpeed,
							((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, ((COrder*)pParam)->LineSpeedSet.InitSpeed);
						((COrder*)pParam)->LaserContinuousControl.ContinuousSwitch = FALSE;
#endif
					}
					else//Demo用
					{
#ifdef MOVE
						((COrder*)pParam)->m_Action.LA_CorrectVectorToDo(((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, ((COrder*)pParam)->LineSpeedSet.InitSpeed,
							((COrder*)pParam)->VisionOffset.Contraposition.X, ((COrder*)pParam)->VisionOffset.Contraposition.Y,
							((COrder*)pParam)->VisionOffset.OffsetX, ((COrder*)pParam)->VisionOffset.OffsetY, ((COrder*)pParam)->VisionOffset.Angle,
							((COrder*)pParam)->VisionSet.AdjustOffsetX, ((COrder*)pParam)->VisionSet.AdjustOffsetY, ((COrder*)pParam)->VisionSet.ModifyMode,
							((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y);
						//呼叫斷膠抬升
						((COrder*)pParam)->m_Action.DecideLineEndMove(((COrder*)pParam)->DispenseLineSet.StayTime, ((COrder*)pParam)->DispenseLineSet.ShutdownDelay,
							((COrder*)pParam)->ZSet.ZBackHeight, ((COrder*)pParam)->ZSet.ZBackType, ((COrder*)pParam)->DispenseLineEnd.HighSpeed, ((COrder*)pParam)->DispenseLineEnd.Width, ((COrder*)pParam)->DispenseLineEnd.Height, ((COrder*)pParam)->DispenseLineEnd.LowSpeed,
							((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, ((COrder*)pParam)->LineSpeedSet.InitSpeed);
						((COrder*)pParam)->LaserContinuousControl.ContinuousSwitch = FALSE;
#endif
					}
				}
			}
			((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Status = FALSE;
			((COrder*)pParam)->RunData.ActionStatus.at(((COrder*)pParam)->Program.SubroutinCount) = 0;

			((COrder*)pParam)->VirtualCoordinateData = ((COrder*)pParam)->FinalWorkCoordinateData;//紀錄移動虛擬座標
		}
	}
	else if (CommandResolve(Command, 0) == L"ArcPoint")
	{
		if (((COrder*)pParam)->ModelControl.Mode == 0)//建表模式
		{
			RecordCorrectionTable(pParam);//寫入修正表
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 1)//影像模式
		{
			RecordCorrectionTable(pParam);//寫入修正表
			ModifyPointOffSet(pParam, Command);//CallSubroutin相對位修正
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 2)//雷射模式
		{
			ModifyPointOffSet(pParam, Command);//CallSubroutin相對位修正
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 3)//運動模式
		{
			((COrder*)pParam)->ActionCount++;//動作計數++
			ChooseVisionModify(pParam);//選擇影像Offset
			ModifyPointOffSet(pParam, Command);//CallSubroutin相對位修正
			if (((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Status == TRUE)//限定圓弧和圓只能存在一個
			{
				((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Status = FALSE;
			}
			((COrder*)pParam)->FinalWorkCoordinateData.X = _ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X;
			((COrder*)pParam)->FinalWorkCoordinateData.Y = _ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y;
			((COrder*)pParam)->FinalWorkCoordinateData.Z = _ttol(CommandResolve(Command, 3)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z;
			((COrder*)pParam)->FinalWorkCoordinateData.W = _tstof(CommandResolve(Command, 4)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).W;
			VisionModify(pParam);//影像修正

			((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Status = TRUE;
			((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).X = ((COrder*)pParam)->FinalWorkCoordinateData.X;
			((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Y = ((COrder*)pParam)->FinalWorkCoordinateData.Y;
			((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Z = ((COrder*)pParam)->FinalWorkCoordinateData.Z;
			((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).W = ((COrder*)pParam)->FinalWorkCoordinateData.W;
			//((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).X = _ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X;
			//((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Y = _ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y;

#ifdef PRINTF
			if (((COrder*)pParam)->ModelControl.Mode == 3)
				_cwprintf(L"SubroutineThread()::%s(%d,%d,%d,%.3f)\n", CommandResolve(Command, 0), ((COrder*)pParam)->FinalWorkCoordinateData.X, ((COrder*)pParam)->FinalWorkCoordinateData.Y, ((COrder*)pParam)->FinalWorkCoordinateData.Z, ((COrder*)pParam)->FinalWorkCoordinateData.W);
#endif
		}   
	}
	else if (CommandResolve(Command, 0) == L"CirclePoint")
	{
		if (((COrder*)pParam)->ModelControl.Mode == 0)//建表模式
		{
			RecordCorrectionTable(pParam);//寫入修正表
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 1)//影像模式
		{
			RecordCorrectionTable(pParam);//寫入修正表
			ModifyPointOffSet(pParam, Command);//CallSubroutin相對位修正
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 2)//雷射模式
		{
			ModifyPointOffSet(pParam, Command);//CallSubroutin相對位修正
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 3)//運動、雷射模式
		{
			((COrder*)pParam)->ActionCount++;//動作計數++
			ChooseVisionModify(pParam);//選擇影像Offset
			ModifyPointOffSet(pParam, Command);//CallSubroutin相對位修正
			if (((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Status == TRUE)//限定圓弧和圓只能存在一個
			{
				((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Status = FALSE;
			}
			//圓第一點
			((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Status = TRUE;
			((COrder*)pParam)->FinalWorkCoordinateData.X = _ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X;
			((COrder*)pParam)->FinalWorkCoordinateData.Y = _ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y;
			VisionModify(pParam);//影像修正
			((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).X = ((COrder*)pParam)->FinalWorkCoordinateData.X;
			((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Y = ((COrder*)pParam)->FinalWorkCoordinateData.Y;
			//圓第二點
			((COrder*)pParam)->FinalWorkCoordinateData.X = _ttol(CommandResolve(Command, 5)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X;
			((COrder*)pParam)->FinalWorkCoordinateData.Y = _ttol(CommandResolve(Command, 6)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y;
			VisionModify(pParam);//影像修正
			((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount).X = ((COrder*)pParam)->FinalWorkCoordinateData.X;
			((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount).Y = ((COrder*)pParam)->FinalWorkCoordinateData.Y;


			//((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).X = _ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X;
			//((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Y = _ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y;
			((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Z = _ttol(CommandResolve(Command, 3)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z;
			((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).W = _tstof(CommandResolve(Command, 4)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).W;
			//((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount).X = _ttol(CommandResolve(Command, 4)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X;
			//((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount).Y = _ttol(CommandResolve(Command, 5)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y;
			((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount).Z = _ttol(CommandResolve(Command, 7)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z;
			((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount).W = _tstof(CommandResolve(Command, 8)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).W;
#ifdef PRINTF
			_cwprintf(L"SubroutineThread()::%s(%d,%d,%d,%.3f-%d,%d,%d,%.3f)\n", CommandResolve(Command, 0),
				((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Y, ((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Z, ((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).W,
				((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount).Y, ((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount).Z, ((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount).W);
#endif
		}
	}
	else if (CommandResolve(Command, 0) == L"GoHome")
	{
		if (((COrder*)pParam)->ModelControl.Mode == 1)//影像模式
		{
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 2)//雷射模式
		{
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 3)//運動模式
		{
			LineGotoActionJudge(pParam);//判斷動作狀態
#ifdef PRINTF
			_cwprintf(L"SubroutineThread()::%s(%d,%d,%d,%.3f)\n", CommandResolve(Command, 0), 0, 0, 0, 0);
#endif
			if (((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Status)//已經有offset修正
			{
#ifdef MOVE
				//先移動到(0,0,0)位置 Z軸抬升目前35000絕對 
				((COrder*)pParam)->m_Action.DecideVirtualHome(0, 0, 0, 35000, 0,
					((COrder*)pParam)->MoveSpeedSet.EndSpeed, ((COrder*)pParam)->MoveSpeedSet.AccSpeed, ((COrder*)pParam)->MoveSpeedSet.InitSpeed);
				((COrder*)pParam)->m_Action.DecideInitializationMachine(((COrder*)pParam)->GoHome.Speed1, ((COrder*)pParam)->GoHome.Speed2, ((COrder*)pParam)->GoHome.Axis, ((COrder*)pParam)->GoHome.MoveX, ((COrder*)pParam)->GoHome.MoveY, ((COrder*)pParam)->GoHome.MoveZ, ((COrder*)pParam)->GoHome.MoveW);
#endif   
				((COrder*)pParam)->VirtualCoordinateData = { 0,0,0,0,0 };//紀錄移動虛擬座標
			}
			else
			{
#ifdef MOVE
				((COrder*)pParam)->m_Action.DecideInitializationMachine(((COrder*)pParam)->GoHome.Speed1, ((COrder*)pParam)->GoHome.Speed2, ((COrder*)pParam)->GoHome.Axis, ((COrder*)pParam)->GoHome.MoveX, ((COrder*)pParam)->GoHome.MoveY, ((COrder*)pParam)->GoHome.MoveZ, ((COrder*)pParam)->GoHome.MoveW);
				((COrder*)pParam)->m_Action.DecideVirtualPoint(
					((COrder*)pParam)->Program.SubroutinePointStack.at(((COrder*)pParam)->Program.SubroutinCount).X,
					((COrder*)pParam)->Program.SubroutinePointStack.at(((COrder*)pParam)->Program.SubroutinCount).Y,
					((COrder*)pParam)->Program.SubroutinePointStack.at(((COrder*)pParam)->Program.SubroutinCount).Z,
					((COrder*)pParam)->Program.SubroutinePointStack.at(((COrder*)pParam)->Program.SubroutinCount).W,
					((COrder*)pParam)->DotSpeedSet.EndSpeed, ((COrder*)pParam)->DotSpeedSet.AccSpeed, ((COrder*)pParam)->DotSpeedSet.InitSpeed);//回CallSuboutine紀錄點
#endif  
				((COrder*)pParam)->VirtualCoordinateData = { 0,((COrder*)pParam)->Program.SubroutinePointStack.at(((COrder*)pParam)->Program.SubroutinCount).X,((COrder*)pParam)->Program.SubroutinePointStack.at(((COrder*)pParam)->Program.SubroutinCount).Y,((COrder*)pParam)->Program.SubroutinePointStack.at(((COrder*)pParam)->Program.SubroutinCount).Z ,((COrder*)pParam)->Program.SubroutinePointStack.at(((COrder*)pParam)->Program.SubroutinCount).W };//紀錄移動虛擬座標
			}
		} 
	}
	else if (CommandResolve(Command, 0) == L"VirtualPoint")
	{
		if (((COrder*)pParam)->ModelControl.Mode == 0)//建表模式
		{
			RecordCorrectionTable(pParam);//寫入修正表
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 1)//影像模式
		{
			RecordCorrectionTable(pParam);//寫入修正表
			VirtualCoordinateMove(pParam, Command, 1);//虛擬座標移動
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 2)//雷射模式
		{
			LaserDetectHandle(pParam, Command);//雷射測高和虛擬座標移動
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 3)//運動模式
		{
			ChooseVisionModify(pParam);//選擇影像Offset
			ChooseLaserModify(pParam);//選擇雷射高度
			ModifyPointOffSet(pParam, Command);//CallSubroutin相對位修正
			LineGotoActionJudge(pParam);//判斷動作狀態
			((COrder*)pParam)->FinalWorkCoordinateData.X = _ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X;
			((COrder*)pParam)->FinalWorkCoordinateData.Y = _ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y;
			((COrder*)pParam)->FinalWorkCoordinateData.Z = _ttol(CommandResolve(Command, 3)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z;
			((COrder*)pParam)->FinalWorkCoordinateData.W = _tstof(CommandResolve(Command, 4)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).W;

			((COrder*)pParam)->NVMVirtualCoordinateData = ((COrder*)pParam)->FinalWorkCoordinateData;//紀錄CallSubroutine點(不加影像修正時的值)
			
			if (_ttol(CommandResolve(Command, 5)) == 1)
			{
				VisionModify(pParam);//影像修正
			}
			else
			{
				((COrder*)pParam)->VisioModifyJudge = FALSE;//影像開關改變為不修正(用在CallSubroutine)
			}

			LaserModify(pParam);//雷射修正
#ifdef PRINTF
			_cwprintf(L"SubroutineThread()::%s(%d,%d,%d,%.3f)\n", CommandResolve(Command, 0), ((COrder*)pParam)->FinalWorkCoordinateData.X, ((COrder*)pParam)->FinalWorkCoordinateData.Y, ((COrder*)pParam)->FinalWorkCoordinateData.Z, ((COrder*)pParam)->FinalWorkCoordinateData.W);
#endif
#ifdef MOVE
			((COrder*)pParam)->m_Action.DecideVirtualPoint(
				//_ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X,
				//_ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
				//_ttol(CommandResolve(Command, 3)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z,
				((COrder*)pParam)->FinalWorkCoordinateData.X,
				((COrder*)pParam)->FinalWorkCoordinateData.Y,
				((COrder*)pParam)->FinalWorkCoordinateData.Z,
				((COrder*)pParam)->FinalWorkCoordinateData.W,
				((COrder*)pParam)->DotSpeedSet.EndSpeed, ((COrder*)pParam)->DotSpeedSet.AccSpeed, ((COrder*)pParam)->DotSpeedSet.InitSpeed);
#endif
			((COrder*)pParam)->VirtualCoordinateData = ((COrder*)pParam)->FinalWorkCoordinateData;//紀錄移動虛擬座標
		} 
	}
	else if (CommandResolve(Command, 0) == L"WaitPoint")
	{
		if (((COrder*)pParam)->ModelControl.Mode == 0)//建表模式
		{
			RecordCorrectionTable(pParam);//寫入修正表
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 1)//影像模式
		{
			RecordCorrectionTable(pParam);//寫入修正表
			VirtualCoordinateMove(pParam, Command, 1);//虛擬座標移動
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 2)//雷射模式
		{
			LaserDetectHandle(pParam, Command);//雷射測高和虛擬座標移動
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 3)//運動模式
		{
			ChooseVisionModify(pParam);//選擇影像Offset
			ChooseLaserModify(pParam);//選擇雷射高度
			ModifyPointOffSet(pParam, Command);//CallSubroutin相對位修正
			LineGotoActionJudge(pParam);//判斷動作狀態
			((COrder*)pParam)->FinalWorkCoordinateData.X = _ttol(CommandResolve(Command, 1)) +((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X;
			((COrder*)pParam)->FinalWorkCoordinateData.Y = _ttol(CommandResolve(Command, 2)) +((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y;
			((COrder*)pParam)->FinalWorkCoordinateData.Z = _ttol(CommandResolve(Command, 3)) +((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z;
			((COrder*)pParam)->FinalWorkCoordinateData.W = _tstof(CommandResolve(Command, 4)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).W;

			((COrder*)pParam)->NVMVirtualCoordinateData = ((COrder*)pParam)->FinalWorkCoordinateData;//紀錄CallSubroutine點(不加影像修正時的值)
			
			if (_ttol(CommandResolve(Command, 6)) == 1)
			{
				VisionModify(pParam);//影像修正
			}
			else
			{
				((COrder*)pParam)->VisioModifyJudge = FALSE;//影像開關改變為不修正(用在CallSubroutine)
			}
			LaserModify(pParam);//雷射修正
#ifdef PRINTF
			_cwprintf(L"SubroutineThread()::%s(%d,%d,%d,%.3f)\n", CommandResolve(Command, 0), ((COrder*)pParam)->FinalWorkCoordinateData.X, ((COrder*)pParam)->FinalWorkCoordinateData.Y, ((COrder*)pParam)->FinalWorkCoordinateData.Z, ((COrder*)pParam)->FinalWorkCoordinateData.W);
#endif
#ifdef MOVE
			((COrder*)pParam)->m_Action.DecideWaitPoint(
				//_ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X,
				//_ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
				//_ttol(CommandResolve(Command, 3)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z,
				((COrder*)pParam)->FinalWorkCoordinateData.X,
				((COrder*)pParam)->FinalWorkCoordinateData.Y,
				((COrder*)pParam)->FinalWorkCoordinateData.Z,
				((COrder*)pParam)->FinalWorkCoordinateData.W,
				_ttol(CommandResolve(Command, 5)), ((COrder*)pParam)->DotSpeedSet.EndSpeed, ((COrder*)pParam)->DotSpeedSet.AccSpeed, ((COrder*)pParam)->DotSpeedSet.InitSpeed);
#endif
			((COrder*)pParam)->VirtualCoordinateData = ((COrder*)pParam)->FinalWorkCoordinateData;//紀錄移動虛擬座標
		}  
	}
	else if (CommandResolve(Command, 0) == L"ParkPosition")
	{
		if (((COrder*)pParam)->ModelControl.Mode == 1)//影像模式
		{
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 2)//雷射模式
		{
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 3)//運動模式
		{
			LineGotoActionJudge(pParam);//判斷動作狀態
#ifdef PRINTF
			_cwprintf(L"SubroutineThread()::%s(%d,%d,%d,%.3f)\n", CommandResolve(Command, 0), ((COrder*)pParam)->GlueData.ParkPositionData.X, ((COrder*)pParam)->GlueData.ParkPositionData.Y, ((COrder*)pParam)->GlueData.ParkPositionData.Z, ((COrder*)pParam)->GlueData.ParkPositionData.W);
#endif
			if (((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Status)//已經有offset修正
			{

				if (((COrder*)pParam)->GlueData.GlueAuto)
				{
#ifdef MOVE
					((COrder*)pParam)->m_Action.DecideParkPoint(((COrder*)pParam)->GlueData.ParkPositionData.X, ((COrder*)pParam)->GlueData.ParkPositionData.Y, ((COrder*)pParam)->GlueData.ParkPositionData.Z, ((COrder*)pParam)->GlueData.ParkPositionData.W,
						((COrder*)pParam)->GlueData.GlueTime, ((COrder*)pParam)->GlueData.GlueWaitTime, ((COrder*)pParam)->GlueData.GlueStayTime,
						((COrder*)pParam)->ZSet.ZBackHeight, ((COrder*)pParam)->ZSet.ZBackType, ((COrder*)pParam)->DispenseDotEnd.RiseDistance, ((COrder*)pParam)->DispenseDotEnd.RiseHightSpeed, ((COrder*)pParam)->DispenseDotEnd.RiseLowSpeed,
						((COrder*)pParam)->DotSpeedSet.EndSpeed, ((COrder*)pParam)->DotSpeedSet.AccSpeed, ((COrder*)pParam)->DotSpeedSet.InitSpeed);
#endif
					((COrder*)pParam)->VirtualCoordinateData = { 0,((COrder*)pParam)->GlueData.ParkPositionData.X, ((COrder*)pParam)->GlueData.ParkPositionData.Y, ((COrder*)pParam)->GlueData.ParkPositionData.Z };//紀錄移動虛擬座標
				}
			}
			else
			{
#ifdef MOVE
				if (((COrder*)pParam)->GlueData.GlueAuto)
				{

					((COrder*)pParam)->m_Action.DecideParkPoint(((COrder*)pParam)->GlueData.ParkPositionData.X, ((COrder*)pParam)->GlueData.ParkPositionData.Y, ((COrder*)pParam)->GlueData.ParkPositionData.Z, ((COrder*)pParam)->GlueData.ParkPositionData.W,
						((COrder*)pParam)->GlueData.GlueTime, ((COrder*)pParam)->GlueData.GlueWaitTime, ((COrder*)pParam)->GlueData.GlueStayTime,
						((COrder*)pParam)->ZSet.ZBackHeight, ((COrder*)pParam)->ZSet.ZBackType,((COrder*)pParam)->DispenseDotEnd.RiseDistance, ((COrder*)pParam)->DispenseDotEnd.RiseHightSpeed, ((COrder*)pParam)->DispenseDotEnd.RiseLowSpeed,
						((COrder*)pParam)->DotSpeedSet.EndSpeed, ((COrder*)pParam)->DotSpeedSet.AccSpeed, ((COrder*)pParam)->DotSpeedSet.InitSpeed);

				}
				((COrder*)pParam)->m_Action.DecideVirtualPoint(
					((COrder*)pParam)->Program.SubroutinePointStack.at(((COrder*)pParam)->Program.SubroutinCount).X,
					((COrder*)pParam)->Program.SubroutinePointStack.at(((COrder*)pParam)->Program.SubroutinCount).Y,
					((COrder*)pParam)->Program.SubroutinePointStack.at(((COrder*)pParam)->Program.SubroutinCount).Z,
					((COrder*)pParam)->Program.SubroutinePointStack.at(((COrder*)pParam)->Program.SubroutinCount).W,
					((COrder*)pParam)->DotSpeedSet.EndSpeed, ((COrder*)pParam)->DotSpeedSet.AccSpeed, ((COrder*)pParam)->DotSpeedSet.InitSpeed);
#endif
				((COrder*)pParam)->VirtualCoordinateData = { 0,((COrder*)pParam)->Program.SubroutinePointStack.at(((COrder*)pParam)->Program.SubroutinCount).X,((COrder*)pParam)->Program.SubroutinePointStack.at(((COrder*)pParam)->Program.SubroutinCount).Y,((COrder*)pParam)->Program.SubroutinePointStack.at(((COrder*)pParam)->Program.SubroutinCount).Z };//紀錄移動虛擬座標
			}
		} 
	}
	else if (CommandResolve(Command, 0) == L"StopPoint")
	{
		if (((COrder*)pParam)->ModelControl.Mode == 0)//建表模式
		{
			RecordCorrectionTable(pParam);//寫入修正表
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 1)//影像模式
		{
			RecordCorrectionTable(pParam);//寫入修正表
			VirtualCoordinateMove(pParam, Command, 1);//虛擬座標移動
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 2)//雷射模式
		{
			LaserDetectHandle(pParam, Command);//雷射測高和虛擬座標移動 
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 3)//運動模式
		{
			ChooseVisionModify(pParam);//選擇影像Offset
			ChooseLaserModify(pParam);//選擇雷射高度
			ModifyPointOffSet(pParam, Command);//CallSubroutin相對位修正
			LineGotoActionJudge(pParam);//判斷動作狀態
			((COrder*)pParam)->FinalWorkCoordinateData.X = _ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X;
			((COrder*)pParam)->FinalWorkCoordinateData.Y = _ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y;
			((COrder*)pParam)->FinalWorkCoordinateData.Z = _ttol(CommandResolve(Command, 3)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z;
			((COrder*)pParam)->FinalWorkCoordinateData.W = _tstof(CommandResolve(Command, 4)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).W;

			((COrder*)pParam)->NVMVirtualCoordinateData = ((COrder*)pParam)->FinalWorkCoordinateData;//紀錄CallSubroutine點(不加影像修正時的值)
				   
			if (_ttol(CommandResolve(Command, 5)) == 1)
			{
				VisionModify(pParam);//影像修正
			}
			else
			{
				((COrder*)pParam)->VisioModifyJudge = FALSE;//影像開關改變為不修正(用在CallSubroutine)
			}

			LaserModify(pParam);
#ifdef PRINTF
			_cwprintf(L"SubroutineThread()::%s(%d,%d,%d,%.3f)\n", CommandResolve(Command, 0), ((COrder*)pParam)->FinalWorkCoordinateData.X, ((COrder*)pParam)->FinalWorkCoordinateData.Y, ((COrder*)pParam)->FinalWorkCoordinateData.Z, ((COrder*)pParam)->FinalWorkCoordinateData.W);
#endif
#ifdef MOVE
			((COrder*)pParam)->m_Action.DecideVirtualPoint(
				//_ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X,
				//_ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
				//_ttol(CommandResolve(Command, 3)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z,
				((COrder*)pParam)->FinalWorkCoordinateData.X,
				((COrder*)pParam)->FinalWorkCoordinateData.Y,
				((COrder*)pParam)->FinalWorkCoordinateData.Z,
				((COrder*)pParam)->FinalWorkCoordinateData.W,
				((COrder*)pParam)->DotSpeedSet.EndSpeed, ((COrder*)pParam)->DotSpeedSet.AccSpeed, ((COrder*)pParam)->DotSpeedSet.InitSpeed);
#endif
			((COrder*)pParam)->Pause();
			((COrder*)pParam)->VirtualCoordinateData = ((COrder*)pParam)->FinalWorkCoordinateData;//紀錄移動虛擬座標
		}  
	}
	else if (CommandResolve(Command, 0) == L"FillArea")
	{
		if (((COrder*)pParam)->ModelControl.Mode == 0)//建表模式
		{
			RecordCorrectionTable(pParam);//寫入修正表
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 1)//影像模式
		{
			RecordCorrectionTable(pParam);//寫入修正表
			VirtualCoordinateMove(pParam, Command, 1);//虛擬座標移動
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 2)//雷射模式(測高依起始點)
		{
			LaserDetectHandle(pParam, Command);//雷射測高和虛擬座標移動
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 3)//運動模式
		{
			CString CommandBuff;
			CommandBuff.Format(_T("FillArea,%d,%d,%d"), _ttol(CommandResolve(Command, 4)), _ttol(CommandResolve(Command, 5)), _ttol(CommandResolve(Command, 6)));
			ChooseVisionModify(pParam);//選擇影像Offset
			ChooseLaserModify(pParam);//選擇雷射高度
			ModifyPointOffSet(pParam, CommandBuff);//CallSubroutin相對位修正
			LineGotoActionJudge(pParam);//判斷動作狀態
			
			//紀錄FillArea運動完畢的點給CallSubroutine時使用(不加影像修正時的值)
			((COrder*)pParam)->m_Action.Fill_EndPoint(((COrder*)pParam)->NVMVirtualCoordinateData.X, ((COrder*)pParam)->NVMVirtualCoordinateData.Y,
				_ttol(CommandResolve(Command, 4)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X,
				_ttol(CommandResolve(Command, 5)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
				_ttol(CommandResolve(Command, 6)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z,
				_ttol(CommandResolve(Command, 7)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X,
				_ttol(CommandResolve(Command, 8)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
				_ttol(CommandResolve(Command, 9)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z,
				_ttol(CommandResolve(Command, 1)), _ttol(CommandResolve(Command, 2)), _ttol(CommandResolve(Command, 3))
				);

			((COrder*)pParam)->NVMVirtualCoordinateData.Z = _ttol(CommandResolve(Command, 9)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z; //紀錄CallSubroutine點(不加影像修正時的值)
										
			//填充修正
			LONG FillAreaPoint1X, FillAreaPoint1Y, FillAreaPoint1Z, FillAreaPoint2X, FillAreaPoint2Y, FillAreaPoint2Z;
			((COrder*)pParam)->FinalWorkCoordinateData.X = _ttol(CommandResolve(Command, 4)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X;
			((COrder*)pParam)->FinalWorkCoordinateData.Y = _ttol(CommandResolve(Command, 5)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y;
			((COrder*)pParam)->FinalWorkCoordinateData.Z = _ttol(CommandResolve(Command, 6)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z;
			
			VisionModify(pParam);//影像修正
			LaserModify(pParam);//雷射修正
			FillAreaPoint1X = ((COrder*)pParam)->FinalWorkCoordinateData.X;
			FillAreaPoint1Y = ((COrder*)pParam)->FinalWorkCoordinateData.Y;
			FillAreaPoint1Z = ((COrder*)pParam)->FinalWorkCoordinateData.Z;

			((COrder*)pParam)->FinalWorkCoordinateData.X = _ttol(CommandResolve(Command, 7)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X;
			((COrder*)pParam)->FinalWorkCoordinateData.Y = _ttol(CommandResolve(Command, 8)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y;
			((COrder*)pParam)->FinalWorkCoordinateData.Z = _ttol(CommandResolve(Command, 9)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z;

			VisionModify(pParam);//影像修正
			LaserModify(pParam);//雷射修正
			FillAreaPoint2X = ((COrder*)pParam)->FinalWorkCoordinateData.X;
			FillAreaPoint2Y = ((COrder*)pParam)->FinalWorkCoordinateData.Y;
			FillAreaPoint2Z = FillAreaPoint1Z;//限定為統一高度

#ifdef MOVE
			((COrder*)pParam)->m_Action.DecideFill(
				//_ttol(CommandResolve(Command, 4)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X,
				//_ttol(CommandResolve(Command, 5)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
				//_ttol(CommandResolve(Command, 6)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z,
				FillAreaPoint1X, FillAreaPoint1Y, FillAreaPoint1Z,
				//_ttol(CommandResolve(Command, 7)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X,
				//_ttol(CommandResolve(Command, 8)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
				//_ttol(CommandResolve(Command, 9)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z,
				FillAreaPoint2X, FillAreaPoint2Y, FillAreaPoint2Z,
				((COrder*)pParam)->ZSet.ZBackHeight, ((COrder*)pParam)->ZSet.ZBackType,
				_ttol(CommandResolve(Command, 1)), _ttol(CommandResolve(Command, 2)), _ttol(CommandResolve(Command, 3)),
				((COrder*)pParam)->DispenseLineSet.BeforeMoveDelay, ((COrder*)pParam)->DispenseLineSet.BeforeMoveDistance,
				((COrder*)pParam)->DispenseLineSet.ShutdownDelay, ((COrder*)pParam)->DispenseLineSet.ShutdownDistance, ((COrder*)pParam)->DispenseLineSet.StayTime,
				((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, ((COrder*)pParam)->LineSpeedSet.InitSpeed);
#endif 
			//紀錄移動虛擬座標
			((COrder*)pParam)->m_Action.Fill_EndPoint(((COrder*)pParam)->VirtualCoordinateData.X, ((COrder*)pParam)->VirtualCoordinateData.Y,
				FillAreaPoint1X, FillAreaPoint1Y, FillAreaPoint1Z,
				FillAreaPoint2X, FillAreaPoint2Y, FillAreaPoint2Z,
				_ttol(CommandResolve(Command, 1)), _ttol(CommandResolve(Command, 2)), _ttol(CommandResolve(Command, 3))
			);
			((COrder*)pParam)->VirtualCoordinateData.Z = FillAreaPoint2Z;
		}  
	}
	else if (CommandResolve(Command, 0) == L"CleanerPosition")
	{
		if (((COrder*)pParam)->ModelControl.Mode == 1)//影像模式
		{
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 2)//雷射模式
		{
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 3)//運動模式
		{
			LineGotoActionJudge(pParam);//判斷動作狀態
#ifdef PRINTF
			_cwprintf(L"SubroutineThread()::%s(%d,%d,%d,%.3f)\n", CommandResolve(Command, 0), ((COrder*)pParam)->GlueData.ParkPositionData.X, ((COrder*)pParam)->GlueData.ParkPositionData.Y, ((COrder*)pParam)->GlueData.ParkPositionData.Z, ((COrder*)pParam)->GlueData.ParkPositionData.W);
#endif
			if (((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Status)//已經有offset修正
			{

				if (((COrder*)pParam)->CleanerData.CleanerSwitch)
				{
#ifdef MOVE
					((COrder*)pParam)->m_Action.DispenClear(((COrder*)pParam)->CleanerData.CleanerPositionData.X, ((COrder*)pParam)->CleanerData.CleanerPositionData.Y, ((COrder*)pParam)->CleanerData.CleanerPositionData.Z,
						((COrder*)pParam)->CleanerData.OutputPort, ((COrder*)pParam)->ZSet.ZBackHeight, ((COrder*)pParam)->ZSet.ZBackType,
						((COrder*)pParam)->DotSpeedSet.EndSpeed, ((COrder*)pParam)->DotSpeedSet.AccSpeed, ((COrder*)pParam)->DotSpeedSet.InitSpeed);
#endif
					((COrder*)pParam)->VirtualCoordinateData = { 0,((COrder*)pParam)->CleanerData.CleanerPositionData.X, ((COrder*)pParam)->CleanerData.CleanerPositionData.Y, ((COrder*)pParam)->CleanerData.CleanerPositionData.Z };//紀錄移動虛擬座標
				}
			}
			else
			{
#ifdef MOVE
				if (((COrder*)pParam)->CleanerData.CleanerSwitch)
				{
					((COrder*)pParam)->m_Action.DispenClear(((COrder*)pParam)->CleanerData.CleanerPositionData.X, ((COrder*)pParam)->CleanerData.CleanerPositionData.Y, ((COrder*)pParam)->CleanerData.CleanerPositionData.Z,
						((COrder*)pParam)->CleanerData.OutputPort, ((COrder*)pParam)->ZSet.ZBackHeight, ((COrder*)pParam)->ZSet.ZBackType,
						((COrder*)pParam)->DotSpeedSet.EndSpeed, ((COrder*)pParam)->DotSpeedSet.AccSpeed, ((COrder*)pParam)->DotSpeedSet.InitSpeed);
				}
				((COrder*)pParam)->m_Action.DecideVirtualPoint(
					((COrder*)pParam)->Program.SubroutinePointStack.at(((COrder*)pParam)->Program.SubroutinCount).X,
					((COrder*)pParam)->Program.SubroutinePointStack.at(((COrder*)pParam)->Program.SubroutinCount).Y,
					((COrder*)pParam)->Program.SubroutinePointStack.at(((COrder*)pParam)->Program.SubroutinCount).Z,
					((COrder*)pParam)->Program.SubroutinePointStack.at(((COrder*)pParam)->Program.SubroutinCount).W,
					((COrder*)pParam)->DotSpeedSet.EndSpeed, ((COrder*)pParam)->DotSpeedSet.AccSpeed, ((COrder*)pParam)->DotSpeedSet.InitSpeed);
#endif
				((COrder*)pParam)->VirtualCoordinateData = { 0,((COrder*)pParam)->Program.SubroutinePointStack.at(((COrder*)pParam)->Program.SubroutinCount).X,((COrder*)pParam)->Program.SubroutinePointStack.at(((COrder*)pParam)->Program.SubroutinCount).Y,((COrder*)pParam)->Program.SubroutinePointStack.at(((COrder*)pParam)->Program.SubroutinCount).Z };//紀錄移動虛擬座標
			}
		}
	}
	else if (CommandResolve(Command, 0) == L"Output")
	{
		if (((COrder*)pParam)->ModelControl.Mode == 0)//建表模式
		{
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 1)//影像模式
		{
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 2)//雷射模式
		{
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 3)//運動模式
		{
#ifdef MOVE
			((COrder*)pParam)->m_Action.DecideOutPutSign(_ttol(CommandResolve(Command, 1)), _ttol(CommandResolve(Command, 2)));
#endif
		} 
	}
	else if (CommandResolve(Command, 0) == L"Input")
	{
		if (((COrder*)pParam)->ModelControl.Mode == 0)//建表模式
		{
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 1)//影像模式
		{
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 2)//雷射模式
		{
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 3)//運動模式
		{
			//if (_ttol(CommandResolve(Command, 1)))
			//{
			//	((COrder*)pParam)->Program.LabelName = _T("Label,") + CommandResolve(Command, 3);//跳到標籤
			//}
#ifdef MOVE
			if (((COrder*)pParam)->m_Action.DecideInPutSign(_ttol(CommandResolve(Command, 1)), _ttol(CommandResolve(Command, 2))) /*&& _ttol(CommandResolve(Command, 3))*/)
			{
				((COrder*)pParam)->Program.LabelName = _T("Label,") + CommandResolve(Command, 3);//跳到標籤
				//((COrder*)pParam)->RunData.RunCount = _ttol(CommandResolve(Command, 3)) - 2;//跳到地址
			}
#endif
		} 
	}
	/************************************************************運動參數***********************************************************/
	else if (CommandResolve(Command, 0) == L"DispenseDotSet")
	{
		if (((COrder*)pParam)->ModelControl.Mode == 0)//建表模式
		{
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 1)//影像模式
		{
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 2)//雷射模式
		{
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 3)//運動模式
		{
			((COrder*)pParam)->DispenseDotSet.GlueOpenTime = _ttol(CommandResolve(Command, 1));
			((COrder*)pParam)->DispenseDotSet.GlueCloseTime = _ttol(CommandResolve(Command, 2));
		}
	}
	else if (CommandResolve(Command, 0) == L"DispenseDotEnd")
	{
		if (((COrder*)pParam)->ModelControl.Mode == 0)//建表模式
		{
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 1)//影像模式
		{
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 2)//雷射模式
		{
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 3)//運動模式
		{
			((COrder*)pParam)->DispenseDotEnd.RiseDistance = _ttol(CommandResolve(Command, 1));
			((COrder*)pParam)->DispenseDotEnd.RiseLowSpeed = _ttol(CommandResolve(Command, 2));
			((COrder*)pParam)->DispenseDotEnd.RiseHightSpeed = _ttol(CommandResolve(Command, 3));
		}
	}
	else if (CommandResolve(Command, 0) == L"DotSpeedSet")
	{
		if (((COrder*)pParam)->ModelControl.Mode == 0)//建表模式
		{
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 1)//影像模式
		{
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 2)//雷射模式
		{
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 3)//運動模式
		{
			((COrder*)pParam)->DotSpeedSet.EndSpeed = _ttol(CommandResolve(Command, 1));
		}
	}
	else if (CommandResolve(Command, 0) == L"DotAccPercent")
	{
		if (((COrder*)pParam)->ModelControl.Mode == 0)//建表模式
		{
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 1)//影像模式
		{
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 2)//雷射模式
		{
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 3)//運動模式
		{
			((COrder*)pParam)->DotSpeedSet.AccSpeed = ((COrder*)pParam)->DotSpeedSet.AccSpeed * _ttol(CommandResolve(Command, 1)) / 100;
		}
	}
	else if (CommandResolve(Command, 0) == L"DispenseLineSet")
	{
		if (((COrder*)pParam)->ModelControl.Mode == 0)//建表模式
		{
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 1)//影像模式
		{
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 2)//雷射模式
		{
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 3)//運動模式
		{
			((COrder*)pParam)->DispenseLineSet.BeforeMoveDelay = _ttol(CommandResolve(Command, 1));
			((COrder*)pParam)->DispenseLineSet.BeforeMoveDistance = _ttol(CommandResolve(Command, 2));
			((COrder*)pParam)->DispenseLineSet.NodeTime = _ttol(CommandResolve(Command, 3));
			((COrder*)pParam)->DispenseLineSet.StayTime = _ttol(CommandResolve(Command, 4));
			((COrder*)pParam)->DispenseLineSet.ShutdownDistance = _ttol(CommandResolve(Command, 5));
			((COrder*)pParam)->DispenseLineSet.ShutdownDelay = _ttol(CommandResolve(Command, 6));
		}
	}
	else if (CommandResolve(Command, 0) == L"DispenseLineEnd")
	{
		if (((COrder*)pParam)->ModelControl.Mode == 0)//建表模式
		{
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 1)//影像模式
		{
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 2)//雷射模式
		{
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 3)//運動模式
		{
			((COrder*)pParam)->DispenseLineEnd.Type = _ttol(CommandResolve(Command, 1));
			((COrder*)pParam)->DispenseLineEnd.LowSpeed = _ttol(CommandResolve(Command, 2));
			((COrder*)pParam)->DispenseLineEnd.Height = _ttol(CommandResolve(Command, 3));
			((COrder*)pParam)->DispenseLineEnd.Width = _ttol(CommandResolve(Command, 4));
			((COrder*)pParam)->DispenseLineEnd.HighSpeed = _ttol(CommandResolve(Command, 5));
		}
	}
	else if (CommandResolve(Command, 0) == L"LineSpeedSet")
	{
		if (((COrder*)pParam)->ModelControl.Mode == 0)//建表模式
		{
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 1)//影像模式
		{
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 2)//雷射模式
		{
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 3)//運動模式
		{
			((COrder*)pParam)->LineSpeedSet.EndSpeed = _ttol(CommandResolve(Command, 1));
		} 
	}
	else if (CommandResolve(Command, 0) == L"ZGoBack")
	{
		if (((COrder*)pParam)->ModelControl.Mode == 1)//影像模式
		{
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 2)//雷射模式
		{
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 3)//運動模式
		{
			((COrder*)pParam)->ZSet.ZBackHeight = _ttol(CommandResolve(Command, 1));
			((COrder*)pParam)->ZSet.ZBackType = _ttol(CommandResolve(Command, 2));
		}  
	}
	else if (CommandResolve(Command, 0) == L"DispenseAccSet")
	{
		if (((COrder*)pParam)->ModelControl.Mode == 0)//建表模式
		{
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 1)//影像模式
		{
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 2)//雷射模式
		{
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 3)//運動模式
		{
			if (_ttol(CommandResolve(Command, 1)) == 0)
			{
				((COrder*)pParam)->DotSpeedSet.AccSpeed = _ttol(CommandResolve(Command, 2));
			}
			else
			{
				((COrder*)pParam)->LineSpeedSet.AccSpeed = _ttol(CommandResolve(Command, 2));
			}
		}
	}
	else if (CommandResolve(Command, 0) == L"Initialize")
	{ 
		if (((COrder*)pParam)->ModelControl.Mode == 0)//建表模式
		{
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 1)//影像模式
		{
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 2)//雷射模式
		{
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 3)//運動模式
		{
			((COrder*)pParam)->ParameterDefult();
		}
	}
	//else if (CommandResolve(Command, 0) == L"DispenserSwitch")
	//{
	//	if (((COrder*)pParam)->ModelControl.Mode == 0)//建表模式
	//	{
	//	}
	//	else if (((COrder*)pParam)->ModelControl.Mode == 1)//影像模式
	//	{
	//	}
	//	else if (((COrder*)pParam)->ModelControl.Mode == 2)//雷射模式
	//	{
	//	}
	//	else if (((COrder*)pParam)->ModelControl.Mode == 3)//運動模式
	//	{
	//		((COrder*)pParam)->m_Action.m_bIsDispend = _ttol(CommandResolve(Command, 1));
	//	}  
	//}
	//else if (CommandResolve(Command, 0) == L"DispenserSwitchSet")
	//{
	//	if (((COrder*)pParam)->ModelControl.Mode == 0)//建表模式
	//	{
	//	}
	//	else if (((COrder*)pParam)->ModelControl.Mode == 1)//影像模式
	//	{
	//	}
	//	else if (((COrder*)pParam)->ModelControl.Mode == 2)//雷射模式
	//	{
	//	}
	//	else if (((COrder*)pParam)->ModelControl.Mode == 3)//運動模式
	//	{
	//		((COrder*)pParam)->m_Action.g_iNumberGluePort = _ttol(CommandResolve(Command, 1));
	//	}
	//}
	/************************************************************影像***************************************************************/
	else if (CommandResolve(Command, 0) == L"FindMark")
	{
		if (((COrder*)pParam)->ModelControl.Mode == 0)//建表模式
		{
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 1)//當模式1執行動作
		{
#ifdef VI
			ModifyPointOffSet(pParam, Command);//CallSubroutin相對位修正
			//TODO::影像加入點時Z軸沒有加入CallSubroutine相對量修正
			//紀錄影像隊位位置
			((COrder*)pParam)->FindMark.Point.Status = TRUE;
			((COrder*)pParam)->FindMark.Point.X = _ttol(CommandResolve(Command, 1)) - ((COrder*)pParam)->VisionSet.AdjustOffsetX + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X;
			((COrder*)pParam)->FindMark.Point.Y = _ttol(CommandResolve(Command, 2)) - ((COrder*)pParam)->VisionSet.AdjustOffsetY + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y;
			((COrder*)pParam)->FindMark.Point.Z = _ttol(CommandResolve(Command, 3));// +((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z;
			((COrder*)pParam)->FindMark.LoadModelNum = _ttol(CommandResolve(Command, 4)) - 1; //編號從1開始
			((COrder*)pParam)->FindMark.FocusDistance = _ttol(CommandResolve(Command, 5));
			if (((COrder*)pParam)->FindMark.LoadModelNum >= 0 && ((COrder*)pParam)->FindMark.LoadModelNum < ((COrder*)pParam)->VisionFile.AllModelName.size())
			{
				/*每次要載入時必須先清除Model*/
				//影像釋放記憶體
				if (*(int*)((COrder*)pParam)->FindMark.MilModel != 0)
				{
					VI_ModelFree(((COrder*)pParam)->FindMark.MilModel);
				}
				//影像記憶體初始化
				*(int*)((COrder*)pParam)->FindMark.MilModel = 0;
	  
				if (FileExist(((COrder*)pParam)->VisionFile.ModelPath + ((COrder*)pParam)->VisionFile.AllModelName.at(((COrder*)pParam)->FindMark.LoadModelNum)))//判斷檔案是否存在
				{
					//載入影像Model
					VI_LoadModel(((COrder*)pParam)->FindMark.MilModel, ((COrder*)pParam)->VisionFile.ModelPath, ((COrder*)pParam)->VisionFile.AllModelName.at(((COrder*)pParam)->FindMark.LoadModelNum));
					((COrder*)pParam)->VisionTrigger.Trigger1Switch = TRUE;
				}
				else
				{
					AfxMessageBox(_T("沒有此編號的檔案!"));
				}
			}
			else
			{
				AfxMessageBox(_T("沒有此編號的檔案!"));
			}
#endif
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 2 || ((COrder*)pParam)->ModelControl.Mode == 3)
		{
			ModifyPointOffSet(pParam, Command);//CallSubroutin相對位修正
		}
	}
	else if (CommandResolve(Command, 0) == L"FindMarkAdjust")
	{
		if (((COrder*)pParam)->ModelControl.Mode == 0)//建表模式
		{
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 1)//當模式1執行動作
		{
#ifdef VI
			if (*(int*)((COrder*)pParam)->FindMark.MilModel != 0)
			{
				if (!((COrder*)pParam)->VisionSerchError.Manuallymode)//手動模式未開啟時
				{
					//移動至查找點
#ifdef MOVE
					((COrder*)pParam)->m_Action.DoCCDMove(
						((COrder*)pParam)->FindMark.Point.X,
						((COrder*)pParam)->FindMark.Point.Y,
						((COrder*)pParam)->FindMark.Point.Z,
						((COrder*)pParam)->DotSpeedSet.EndSpeed, ((COrder*)pParam)->DotSpeedSet.AccSpeed, ((COrder*)pParam)->DotSpeedSet.InitSpeed);
#endif  
#ifdef PRINTF
					_cwprintf(L"SubroutineThread()::移動至查找標記\r\n");
#endif
				}
				//設定影像參數
				VI_SetPatternMatch(((COrder*)pParam)->FindMark.MilModel, ((COrder*)pParam)->VisionSet.Accuracy, ((COrder*)pParam)->VisionSet.Speed, ((COrder*)pParam)->VisionSet.Score, 0, 0);//設定不找旋轉過標記
				VI_SetSearchRange(((COrder*)pParam)->FindMark.MilModel, ((COrder*)pParam)->VisionSet.width, ((COrder*)pParam)->VisionSet.height);
				if (!((COrder*)pParam)->VisionSerchError.Manuallymode)
				{
					if (!VI_FindMark(((COrder*)pParam)->FindMark.MilModel, ((COrder*)pParam)->VisionOffset.OffsetX, ((COrder*)pParam)->VisionOffset.OffsetY))
					{
						//沒有找到
#ifdef PRINTF
						_cwprintf(L"SubroutineThread()::未找到\r\n");
#endif
						((COrder*)pParam)->VisionTrigger.AdjustStatus = 1;
						((COrder*)pParam)->VisionFindMarkError(pParam);
					}
					else
					{
						//找到抬升
#ifdef MOVE
						//對位完畢不出膠回升
						((COrder*)pParam)->m_Action.DecideLineEndMove(0, 0,
							((COrder*)pParam)->ZSet.ZBackHeight, ((COrder*)pParam)->ZSet.ZBackType, 0, 0, 0, 0,
							((COrder*)pParam)->DotSpeedSet.EndSpeed, ((COrder*)pParam)->DotSpeedSet.AccSpeed, ((COrder*)pParam)->DotSpeedSet.InitSpeed,1);
#endif
						//紀錄對位點
						((COrder*)pParam)->VisionOffset.Contraposition.Status = TRUE;
						((COrder*)pParam)->VisionOffset.Contraposition.X = ((COrder*)pParam)->FindMark.Point.X;
						((COrder*)pParam)->VisionOffset.Contraposition.Y = ((COrder*)pParam)->FindMark.Point.Y;

						//紀錄影像Offset至影像修正表 
						((COrder*)pParam)->VisionCount++;
						((COrder*)pParam)->VisionAdjust.push_back({ ((COrder*)pParam)->VisionOffset });

						//影像釋放記憶體
						if (*(int*)((COrder*)pParam)->FindMark.MilModel != 0)
						{
							VI_ModelFree(((COrder*)pParam)->FindMark.MilModel);
						}
						//影像記憶體初始化
						*(int*)((COrder*)pParam)->FindMark.MilModel = 0;
						//清除對位Offset資料
						((COrder*)pParam)->VisionOffset = { { 0,0,0,0 },0,0,0 };
					}
				}
				else//手動模式時
				{
#if defined VI &&  defined MOVE
					if (!VI_CameraTrigger(((COrder*)pParam)->FindMark.MilModel, ((COrder*)pParam)->FindMark.Point.X, ((COrder*)pParam)->FindMark.Point.Y, ((COrder*)pParam)->m_Action.MCO_ReadPosition().x, ((COrder*)pParam)->m_Action.MCO_ReadPosition().y, ((COrder*)pParam)->VisionOffset.OffsetX, ((COrder*)pParam)->VisionOffset.OffsetY))
					{
						//沒有找到
#ifdef PRINTF
						_cwprintf(L"SubroutineThread()::手動模式未找到\r\n");
#endif
						//清除手動狀態
						((COrder*)pParam)->VisionSerchError.Manuallymode = FALSE;
						((COrder*)pParam)->VisionTrigger.AdjustStatus = 1;
						((COrder*)pParam)->VisionFindMarkError(pParam);
					}
					else
					{
						//清除手動狀態
						((COrder*)pParam)->VisionSerchError.Manuallymode = FALSE;
						//找到抬升
#ifdef MOVE
						//對位完畢不出膠回升
						((COrder*)pParam)->m_Action.DecideLineEndMove(0, 0,
							((COrder*)pParam)->ZSet.ZBackHeight, ((COrder*)pParam)->ZSet.ZBackType, 0, 0, 0, 0,
							((COrder*)pParam)->DotSpeedSet.EndSpeed, ((COrder*)pParam)->DotSpeedSet.AccSpeed, ((COrder*)pParam)->DotSpeedSet.InitSpeed,1);
#endif
#ifdef PRINTF
						_cwprintf(L"SubroutineThread()::手動模式找到\r\n");
#endif
						//紀錄對位點
						((COrder*)pParam)->VisionOffset.Contraposition.Status = TRUE;
						((COrder*)pParam)->VisionOffset.Contraposition.X = ((COrder*)pParam)->FindMark.Point.X;
						((COrder*)pParam)->VisionOffset.Contraposition.Y = ((COrder*)pParam)->FindMark.Point.Y;

						//紀錄影像Offset至影像修正表 
						((COrder*)pParam)->VisionCount++;
						((COrder*)pParam)->VisionAdjust.push_back({ ((COrder*)pParam)->VisionOffset });

					   

						//影像釋放記憶體
						if (*(int*)((COrder*)pParam)->FindMark.MilModel != 0)
						{
							VI_ModelFree(((COrder*)pParam)->FindMark.MilModel);
						}
						//影像記憶體初始化
						*(int*)((COrder*)pParam)->FindMark.MilModel = 0;
						//清除對位Offset資料
						((COrder*)pParam)->VisionOffset = { { 0,0,0,0 },0,0,0 };
					}
#endif
				}
			}
			else
			{
				AfxMessageBox(_T("FindMarkAdjust need one FindMark"));
			}
#endif
		}
	}
	else if (CommandResolve(Command, 0) == L"FiducialMark")
	{
		if (((COrder*)pParam)->ModelControl.Mode == 0)//建表模式
		{
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 1) //當模式1執行動作
		{
#ifdef VI
			ModifyPointOffSet(pParam, Command);//CallSubroutin相對位修正
			//if (!((COrder*)pParam)->FindMark.Point.Status)//防止混用設計
			if (!((COrder*)pParam)->FiducialMark1.Point.Status)
			{
				//TODO::影像加入點時Z軸沒有加入CallSubroutine相對量修正
				//紀錄影像隊位位置
				((COrder*)pParam)->FiducialMark1.Point.Status = TRUE;
				((COrder*)pParam)->FiducialMark1.Point.X = _ttol(CommandResolve(Command, 1)) - ((COrder*)pParam)->VisionSet.AdjustOffsetX + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X;
				((COrder*)pParam)->FiducialMark1.Point.Y = _ttol(CommandResolve(Command, 2)) - ((COrder*)pParam)->VisionSet.AdjustOffsetY + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y;
				((COrder*)pParam)->FiducialMark1.Point.Z = _ttol(CommandResolve(Command, 3));// +((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z;
				((COrder*)pParam)->FiducialMark1.LoadModelNum = _ttol(CommandResolve(Command, 4)) - 1; //編號從1開始
				((COrder*)pParam)->FiducialMark1.FocusDistance = _ttol(CommandResolve(Command, 5));
				if (((COrder*)pParam)->FiducialMark1.LoadModelNum >= 0 && ((COrder*)pParam)->FiducialMark1.LoadModelNum < ((COrder*)pParam)->VisionFile.AllModelName.size())//判斷編號是否大於檔案標號最大值
				{
					/*每次載入時必須清除Model*/
					//影像釋放記憶體
					if (*(int*)((COrder*)pParam)->FiducialMark1.MilModel != 0)
					{
						VI_ModelFree(((COrder*)pParam)->FiducialMark1.MilModel);
					}  
					//影像記憶體初始化
					*(int*)((COrder*)pParam)->FiducialMark1.MilModel = 0;
					if (FileExist(((COrder*)pParam)->VisionFile.ModelPath + ((COrder*)pParam)->VisionFile.AllModelName.at(((COrder*)pParam)->FiducialMark1.LoadModelNum)))//判斷檔案是否存在
					{
						//載入影像Model
						VI_LoadModel(((COrder*)pParam)->FiducialMark1.MilModel, ((COrder*)pParam)->VisionFile.ModelPath, ((COrder*)pParam)->VisionFile.AllModelName.at(((COrder*)pParam)->FiducialMark1.LoadModelNum));
						((COrder*)pParam)->VisionTrigger.Trigger1Switch = TRUE;
					}
					else
					{
						AfxMessageBox(_T("沒有此編號的檔案!"));
					}
				}
				else
				{
					AfxMessageBox(_T("沒有此編號的檔案!"));
				}
			}
			else
			{
				if (!((COrder*)pParam)->FiducialMark2.Point.Status)
				{
					((COrder*)pParam)->FiducialMark2.Point.Status = TRUE;
					((COrder*)pParam)->FiducialMark2.Point.X = _ttol(CommandResolve(Command, 1)) - ((COrder*)pParam)->VisionSet.AdjustOffsetX + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X;
					((COrder*)pParam)->FiducialMark2.Point.Y = _ttol(CommandResolve(Command, 2)) - ((COrder*)pParam)->VisionSet.AdjustOffsetY + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y;
					((COrder*)pParam)->FiducialMark2.Point.Z = _ttol(CommandResolve(Command, 3));// +((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z;
					((COrder*)pParam)->FiducialMark2.LoadModelNum = _ttol(CommandResolve(Command, 4)) - 1; //編號從1開始
					((COrder*)pParam)->FiducialMark2.FocusDistance = _ttol(CommandResolve(Command, 5));
					if (((COrder*)pParam)->FiducialMark2.LoadModelNum >= 0 && ((COrder*)pParam)->FiducialMark2.LoadModelNum < ((COrder*)pParam)->VisionFile.AllModelName.size())//判斷編號是否大於檔案標號最大值
					{
						/*每次載入時必須清除Model*/
						//影像釋放記憶體
						if (*(int*)((COrder*)pParam)->FiducialMark2.MilModel != 0)
						{
							VI_ModelFree(((COrder*)pParam)->FiducialMark2.MilModel);
						}
						//影像記憶體初始化
						*(int*)((COrder*)pParam)->FiducialMark2.MilModel = 0;
						if (FileExist(((COrder*)pParam)->VisionFile.ModelPath + ((COrder*)pParam)->VisionFile.AllModelName.at(((COrder*)pParam)->FiducialMark2.LoadModelNum)))//判斷檔案是否存在
						{
							//載入影像Model
							VI_LoadModel(((COrder*)pParam)->FiducialMark2.MilModel, ((COrder*)pParam)->VisionFile.ModelPath, ((COrder*)pParam)->VisionFile.AllModelName.at(((COrder*)pParam)->FiducialMark2.LoadModelNum));
							((COrder*)pParam)->VisionTrigger.Trigger1Switch = FALSE;
							((COrder*)pParam)->VisionTrigger.Trigger2Switch = TRUE;
						}
						else
						{
							AfxMessageBox(_T("沒有此編號的檔案!"));
						}
					}
					else
					{
						AfxMessageBox(_T("沒有此編號的檔案!"));
					}
				}
				else
				{
					((COrder*)pParam)->FiducialMark2.Point.Status = FALSE;
					((COrder*)pParam)->FiducialMark1.Point.X = _ttol(CommandResolve(Command, 1)) - ((COrder*)pParam)->VisionSet.AdjustOffsetX + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X;
					((COrder*)pParam)->FiducialMark1.Point.Y = _ttol(CommandResolve(Command, 2)) - ((COrder*)pParam)->VisionSet.AdjustOffsetY + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y;
					((COrder*)pParam)->FiducialMark1.Point.Z = _ttol(CommandResolve(Command, 3));// +((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z;
					((COrder*)pParam)->FiducialMark1.LoadModelNum = _ttol(CommandResolve(Command, 4)) - 1; //編號從1開始
					((COrder*)pParam)->FiducialMark1.FocusDistance = _ttol(CommandResolve(Command, 5));
					if (((COrder*)pParam)->FiducialMark1.LoadModelNum >= 0 && ((COrder*)pParam)->FiducialMark1.LoadModelNum < ((COrder*)pParam)->VisionFile.AllModelName.size())//判斷編號是否大於檔案標號最大值
					{
						/*每次載入時必須清除Model*/
						//影像釋放記憶體
						if (*(int*)((COrder*)pParam)->FiducialMark1.MilModel != 0)
						{
							VI_ModelFree(((COrder*)pParam)->FiducialMark1.MilModel);
						}
						//影像記憶體初始化
						*(int*)((COrder*)pParam)->FiducialMark1.MilModel = 0;
						if (FileExist(((COrder*)pParam)->VisionFile.ModelPath + ((COrder*)pParam)->VisionFile.AllModelName.at(((COrder*)pParam)->FiducialMark1.LoadModelNum)))//判斷檔案是否存在
						{
							//載入影像Model
							VI_LoadModel(((COrder*)pParam)->FiducialMark1.MilModel, ((COrder*)pParam)->VisionFile.ModelPath, ((COrder*)pParam)->VisionFile.AllModelName.at(((COrder*)pParam)->FiducialMark1.LoadModelNum));
							((COrder*)pParam)->VisionTrigger.Trigger1Switch = TRUE;
							((COrder*)pParam)->VisionTrigger.Trigger2Switch = FALSE;
						}
						else
						{
							AfxMessageBox(_T("沒有此編號的檔案!"));
						}
					}
					else
					{
						AfxMessageBox(_T("沒有此編號的檔案!"));
					}
				}
			}
#endif
		} 
		else if (((COrder*)pParam)->ModelControl.Mode == 2 || ((COrder*)pParam)->ModelControl.Mode == 3)
		{
			ModifyPointOffSet(pParam, Command);//CallSubroutin相對位修正
		}
	}
	else if (CommandResolve(Command, 0) == L"FiducialMarkAdjust")
	{
		if (((COrder*)pParam)->ModelControl.Mode == 0)//建表模式
		{
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 1) //當模式1執行動作
		{
#ifdef VI
			if (*(int*)((COrder*)pParam)->FiducialMark1.MilModel != 0 && *(int*)((COrder*)pParam)->FiducialMark2.MilModel != 0)
			{
				//第一點處理
				if (!((COrder*)pParam)->VisionSerchError.Manuallymode && !((COrder*)pParam)->FiducialMark1.FindMarkStatus)//非手動未找到
				{
					//移動至第一點
#ifdef MOVE
					((COrder*)pParam)->m_Action.DoCCDMove(
						((COrder*)pParam)->FiducialMark1.Point.X,
						((COrder*)pParam)->FiducialMark1.Point.Y,
						((COrder*)pParam)->FiducialMark1.Point.Z,
						((COrder*)pParam)->DotSpeedSet.EndSpeed, ((COrder*)pParam)->DotSpeedSet.AccSpeed, ((COrder*)pParam)->DotSpeedSet.InitSpeed);
#endif
#ifdef PRINTF
					_cwprintf(L"SubroutineThread()::移動至第一點\r\n");
#endif
				}
				VI_SetPatternMatch(((COrder*)pParam)->FiducialMark1.MilModel, ((COrder*)pParam)->VisionSet.Accuracy, ((COrder*)pParam)->VisionSet.Speed, ((COrder*)pParam)->VisionSet.Score, ((COrder*)pParam)->VisionSet.Startangle, ((COrder*)pParam)->VisionSet.Endangle);
				VI_SetSearchRange(((COrder*)pParam)->FiducialMark1.MilModel, ((COrder*)pParam)->VisionSet.width, ((COrder*)pParam)->VisionSet.height);
				if (!((COrder*)pParam)->VisionSerchError.Manuallymode && !((COrder*)pParam)->FiducialMark1.FindMarkStatus)//非手動未找到
				{
					if (!VI_FindMark(((COrder*)pParam)->FiducialMark1.MilModel, ((COrder*)pParam)->FiducialMark1.OffsetX, ((COrder*)pParam)->FiducialMark1.OffsetY))
					{
#ifdef PRINTF
						_cwprintf(L"SubroutineThread()::第一點未找到\r\n");
#endif
						//沒有找到
						((COrder*)pParam)->VisionTrigger.AdjustStatus = 2;
						((COrder*)pParam)->VisionFindMarkError(pParam);
					}
					else
					{
#ifdef PRINTF
						_cwprintf(L"SubroutineThread()::第一點找到\r\n");
#endif
						//找到抬升
#ifdef MOVE
						//對位完畢不出膠回升
						((COrder*)pParam)->m_Action.DecideLineEndMove(0, 0,
							((COrder*)pParam)->ZSet.ZBackHeight, ((COrder*)pParam)->ZSet.ZBackType, 0, 0, 0, 0,
							((COrder*)pParam)->DotSpeedSet.EndSpeed, ((COrder*)pParam)->DotSpeedSet.AccSpeed, ((COrder*)pParam)->DotSpeedSet.InitSpeed,1);
#endif
						((COrder*)pParam)->FiducialMark1.FindMarkStatus = TRUE;
					}
				}
				else if (((COrder*)pParam)->VisionSerchError.Manuallymode && !((COrder*)pParam)->FiducialMark1.FindMarkStatus)//手動未找到
				{
#if defined VI &&  defined MOVE
					if (!VI_CameraTrigger(((COrder*)pParam)->FiducialMark1.MilModel, ((COrder*)pParam)->FiducialMark1.Point.X, ((COrder*)pParam)->FiducialMark1.Point.Y, ((COrder*)pParam)->m_Action.MCO_ReadPosition().x, ((COrder*)pParam)->m_Action.MCO_ReadPosition().y, ((COrder*)pParam)->FiducialMark1.OffsetX, ((COrder*)pParam)->FiducialMark1.OffsetY))
					{
#ifdef PRINTF
						_cwprintf(L"SubroutineThread()::手動模式第一點未找到\r\n");
#endif
						//沒有找到
						((COrder*)pParam)->VisionTrigger.AdjustStatus = 2;
						((COrder*)pParam)->VisionFindMarkError(pParam);
					}
					else
					{
						//找到清除手動狀態、將對位點依設置已經有Offset
						//找到抬升
#ifdef MOVE
						//對位完畢不出膠回升
						((COrder*)pParam)->m_Action.DecideLineEndMove(0, 0,
							((COrder*)pParam)->ZSet.ZBackHeight, ((COrder*)pParam)->ZSet.ZBackType, 0, 0, 0, 0,
							((COrder*)pParam)->DotSpeedSet.EndSpeed, ((COrder*)pParam)->DotSpeedSet.AccSpeed, ((COrder*)pParam)->DotSpeedSet.InitSpeed,1);
#endif
						((COrder*)pParam)->VisionSerchError.Manuallymode = FALSE;
						((COrder*)pParam)->FiducialMark1.FindMarkStatus = TRUE;
#ifdef PRINTF
						_cwprintf(L"SubroutineThread()::手動模式第一點找到\r\n");
#endif
					}
#endif
				}
				//第二點處理
				if (!((COrder*)pParam)->VisionSerchError.Manuallymode && !((COrder*)pParam)->FiducialMark2.FindMarkStatus && ((COrder*)pParam)->FiducialMark1.FindMarkStatus)//非手動且第二點未尋找到但第一點找到
				{
					//移動至第二點
#ifdef MOVE
					((COrder*)pParam)->m_Action.DoCCDMove(
						((COrder*)pParam)->FiducialMark2.Point.X,
						((COrder*)pParam)->FiducialMark2.Point.Y,
						((COrder*)pParam)->FiducialMark2.Point.Z,
						((COrder*)pParam)->DotSpeedSet.EndSpeed, ((COrder*)pParam)->DotSpeedSet.AccSpeed, ((COrder*)pParam)->DotSpeedSet.InitSpeed);
#endif
#ifdef PRINTF
					_cwprintf(L"SubroutineThread()::第一點找到移動至第二點\r\n");
#endif
				}
				VI_SetPatternMatch(((COrder*)pParam)->FiducialMark2.MilModel, ((COrder*)pParam)->VisionSet.Accuracy, ((COrder*)pParam)->VisionSet.Speed, ((COrder*)pParam)->VisionSet.Score, ((COrder*)pParam)->VisionSet.Startangle, ((COrder*)pParam)->VisionSet.Endangle);
				VI_SetSearchRange(((COrder*)pParam)->FiducialMark2.MilModel, ((COrder*)pParam)->VisionSet.width, ((COrder*)pParam)->VisionSet.height);
				if (!((COrder*)pParam)->VisionSerchError.Manuallymode && !((COrder*)pParam)->FiducialMark2.FindMarkStatus && ((COrder*)pParam)->FiducialMark1.FindMarkStatus)//非手動且第二點未尋找到但第一點找到
				{
					if (!VI_FindMark(((COrder*)pParam)->FiducialMark2.MilModel, ((COrder*)pParam)->FiducialMark2.OffsetX, ((COrder*)pParam)->FiducialMark2.OffsetY))
					{
#ifdef PRINTF
						_cwprintf(L"SubroutineThread()::第一點找到移動至第二點未找到\r\n");
#endif
						//沒有找到
						((COrder*)pParam)->VisionTrigger.AdjustStatus = 3;
						((COrder*)pParam)->VisionFindMarkError(pParam);
					}
					else
					{
						//找到抬升
#ifdef MOVE
						//對位完畢不出膠回升
						((COrder*)pParam)->m_Action.DecideLineEndMove(0, 0,
							((COrder*)pParam)->ZSet.ZBackHeight, ((COrder*)pParam)->ZSet.ZBackType, 0, 0, 0, 0,
							((COrder*)pParam)->DotSpeedSet.EndSpeed, ((COrder*)pParam)->DotSpeedSet.AccSpeed, ((COrder*)pParam)->DotSpeedSet.InitSpeed,1);
#endif
						((COrder*)pParam)->FiducialMark2.FindMarkStatus = TRUE;
#ifdef PRINTF
						_cwprintf(L"SubroutineThread()::第一點找到移動至第二點找到\r\n");
#endif
					}
				}
				else if (((COrder*)pParam)->VisionSerchError.Manuallymode && !((COrder*)pParam)->FiducialMark2.FindMarkStatus && ((COrder*)pParam)->FiducialMark1.FindMarkStatus)//如果手動模式開啟 且對位點2未找到但對位點1找到
				{
#if defined VI &&  defined MOVE
					if (!VI_CameraTrigger(((COrder*)pParam)->FiducialMark2.MilModel, ((COrder*)pParam)->FiducialMark2.Point.X, ((COrder*)pParam)->FiducialMark2.Point.Y, ((COrder*)pParam)->m_Action.MCO_ReadPosition().x, ((COrder*)pParam)->m_Action.MCO_ReadPosition().y, ((COrder*)pParam)->FiducialMark2.OffsetX, ((COrder*)pParam)->FiducialMark2.OffsetY))
					{
						//沒有找到
#ifdef PRINTF
						_cwprintf(L"SubroutineThread()::手動模式第一點找到第二點未找到\r\n");
#endif
						//找到清除手動狀態、將對位點依設置已經有Offset
						((COrder*)pParam)->VisionSerchError.Manuallymode = FALSE;
						((COrder*)pParam)->VisionTrigger.AdjustStatus = 3;
						((COrder*)pParam)->VisionFindMarkError(pParam);
					}
					else
					{
						//找到抬升
#ifdef MOVE
						((COrder*)pParam)->m_Action.DecideLineEndMove(0, 0,
							((COrder*)pParam)->ZSet.ZBackHeight, ((COrder*)pParam)->ZSet.ZBackType, 0, 0, 0, 0,
							((COrder*)pParam)->DotSpeedSet.EndSpeed, ((COrder*)pParam)->DotSpeedSet.AccSpeed, ((COrder*)pParam)->DotSpeedSet.InitSpeed,1);
#endif
						//找到清除手動狀態、將對位點依設置已經有Offset
						((COrder*)pParam)->VisionSerchError.Manuallymode = FALSE;
						((COrder*)pParam)->FiducialMark2.FindMarkStatus = TRUE;
#ifdef PRINTF
						_cwprintf(L"SubroutineThread()::手動模式第一點找到第二點找到\r\n");
#endif
					}
#endif
				}

				/*都找到後處理*/
				if (((COrder*)pParam)->FiducialMark1.FindMarkStatus && ((COrder*)pParam)->FiducialMark2.FindMarkStatus) //兩個都找到
				{
#ifdef LOG
					//LARGE_INTEGER   StartTime, EndTime, Fre;
					//QueryPerformanceFrequency(&Fre); //取得CPU頻率
					//QueryPerformanceCounter(&StartTime); //取得開機到現在經過幾個CPU Cycle
#endif
					//算出偏移角度
					((COrder*)pParam)->VisionOffset.Angle = VI_AngleCount(
						((COrder*)pParam)->FiducialMark1.Point.X, ((COrder*)pParam)->FiducialMark1.Point.Y,
						((COrder*)pParam)->FiducialMark2.Point.X, ((COrder*)pParam)->FiducialMark2.Point.Y,
						((COrder*)pParam)->FiducialMark1.OffsetX, ((COrder*)pParam)->FiducialMark1.OffsetY,
						((COrder*)pParam)->FiducialMark2.OffsetX, ((COrder*)pParam)->FiducialMark2.OffsetY);

					//寫入資料
					((COrder*)pParam)->VisionOffset.Contraposition.X = ((COrder*)pParam)->FiducialMark1.Point.X;
					((COrder*)pParam)->VisionOffset.Contraposition.Y = ((COrder*)pParam)->FiducialMark1.Point.Y;
					((COrder*)pParam)->VisionOffset.OffsetX = ((COrder*)pParam)->FiducialMark1.OffsetX;
					((COrder*)pParam)->VisionOffset.OffsetY = ((COrder*)pParam)->FiducialMark1.OffsetY;

					//初始標記為未尋找
					((COrder*)pParam)->FiducialMark1.FindMarkStatus = FALSE;
					((COrder*)pParam)->FiducialMark2.FindMarkStatus = FALSE;
#ifdef PRINTF
					_cwprintf(L"SubroutineThread()::第一點找到第二點找到Offset計算完成\n");
#endif

					//紀錄影像Offset至影像修正表 
					((COrder*)pParam)->VisionCount++;
					((COrder*)pParam)->VisionAdjust.push_back({ ((COrder*)pParam)->VisionOffset });
	  
					//影像釋放記憶體
					if (*(int*)((COrder*)pParam)->FiducialMark1.MilModel != 0)
					{
						VI_ModelFree(((COrder*)pParam)->FiducialMark1.MilModel);
					}
					if (*(int*)((COrder*)pParam)->FiducialMark2.MilModel != 0)
					{
						VI_ModelFree(((COrder*)pParam)->FiducialMark2.MilModel);
					}
					//影像記憶體初始化
					*(int*)((COrder*)pParam)->FiducialMark1.MilModel = 0;
					*(int*)((COrder*)pParam)->FiducialMark2.MilModel = 0;
					//清除對位Offset資料
					((COrder*)pParam)->VisionOffset = { { 0,0,0,0 },0,0,0 }; 

#ifdef LOG
					//QueryPerformanceCounter(&EndTime); //取得開機到程式執行完成經過幾個CPU Cycle
					//CString Temp;
					//Temp.Format(L"CCD at RunTime:%.6f\n", ((((double)EndTime.QuadPart - (double)StartTime.QuadPart)) / Fre.QuadPart));
					//InitFileLog(Temp);
#endif
				}

				/*判斷暫停模式是否開啟*/
				if (((COrder*)pParam)->VisionSerchError.Pausemode)
				{
					if (((COrder*)pParam)->VisionTrigger.AdjustStatus == 2)
					{
						((COrder*)pParam)->FiducialMark1.FindMarkStatus = TRUE;
					}
					else if (((COrder*)pParam)->VisionTrigger.AdjustStatus == 3)
					{
						((COrder*)pParam)->FiducialMark2.FindMarkStatus = TRUE;
					}
					((COrder*)pParam)->VisionSerchError.Pausemode = FALSE;
				}
			}
			else
			{
				AfxMessageBox(_T("FiducialMarkAdjust need two FiducialMark"));
			}
#endif
		}
	}
	else if (CommandResolve(Command, 0) == L"FindFiducialAngle")
	{
		if (((COrder*)pParam)->ModelControl.Mode == 1)
		{
			((COrder*)pParam)->VisionSet.Startangle = _ttol(CommandResolve(Command, 1));
			((COrder*)pParam)->VisionSet.Endangle = _ttol(CommandResolve(Command, 2));
		}
	}
	else if (CommandResolve(Command, 0) == L"CameraTrigger")
	{
		if (((COrder*)pParam)->ModelControl.Mode == 0)//建表模式
		{
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 1)
		{
			ModifyPointOffSet(pParam, Command);//CallSubroutin相對位修正
			//TODO::影像加入點時Z軸沒有加入CallSubroutine相對量修正
			if (((COrder*)pParam)->VisionTrigger.Trigger1Switch)
			{
				((COrder*)pParam)->VisionTrigger.Trigger1.push_back({ 0,_ttol(CommandResolve(Command,1)) - ((COrder*)pParam)->VisionSet.AdjustOffsetX + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X,
					_ttol(CommandResolve(Command,2)) - ((COrder*)pParam)->VisionSet.AdjustOffsetY + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y ,_ttol(CommandResolve(Command,3)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z });
			}
			else if (((COrder*)pParam)->VisionTrigger.Trigger2Switch)
			{
				((COrder*)pParam)->VisionTrigger.Trigger2.push_back({ 0,_ttol(CommandResolve(Command,1)) - ((COrder*)pParam)->VisionSet.AdjustOffsetX + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X,
					_ttol(CommandResolve(Command,2)) - ((COrder*)pParam)->VisionSet.AdjustOffsetY + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y,_ttol(CommandResolve(Command,3)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z });
			}
			else
			{

			}
		}
		if (((COrder*)pParam)->ModelControl.Mode == 2 || ((COrder*)pParam)->ModelControl.Mode == 3)
		{
			ModifyPointOffSet(pParam, Command);//CallSubroutin相對位修正
		}   
	}
	/************************************************************雷射****************************************************************/
	else if (CommandResolve(Command, 0) == L"LaserHeight") {
		if (((COrder*)pParam)->ModelControl.Mode == 1 || ((COrder*)pParam)->ModelControl.Mode == 2 || ((COrder*)pParam)->ModelControl.Mode == 3)//在任何模式下動作
		{
			CString CommandBuff;
			CommandBuff.Format(_T("LaserHeight,%d,%d,%d"), _ttol(CommandResolve(Command, 2)), _ttol(CommandResolve(Command, 3)), ((COrder*)pParam)->m_Action.m_HeightLaserZero);
			ModifyPointOffSet(pParam, CommandBuff);//CallSubroutin相對位修正
			//判斷是否轉換模式
			if (!((COrder*)pParam)->LaserSwitch.LaserHeight && !((COrder*)pParam)->LaserSwitch.LaserDetect && ((COrder*)pParam)->ModelControl.Mode == 3 && _ttol(CommandResolve(Command, 1)) && !((COrder*)pParam)->ModelControl.LaserAndCheckModeJump)
			{
				if (((COrder*)pParam)->ModelControl.ModeChangeAddress == -1)//尚未有雷射模式跳轉地址
				{
					//判斷現在是否有StepRepeat
					if (((COrder*)pParam)->RepeatData.StepRepeatNum.size())
					{
						((COrder*)pParam)->RepeatDataRecord = ((COrder*)pParam)->RepeatData;//保留StepRepeat執行參數
					}
					//將模式轉換為雷射模式
					((COrder*)pParam)->ModelControl.Mode = 2;
					//紀錄模式轉換地址
					((COrder*)pParam)->ModelControl.ModeChangeAddress = ((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount));
#ifdef PRINTF
					_cwprintf(L"SubroutineThread()::模式轉換紀錄地址:%d\n", ((COrder*)pParam)->ModelControl.ModeChangeAddress);
#endif
				}
				else//已經有雷射跳轉地址
				{
#ifdef PRINTF
					_cwprintf(L"SubroutineThread()::目前執行指令地址:%d\t雷射執行過紀錄地址:%d\n", CurrentRunCommandNum, ((COrder*)pParam)->ModelControl.ModeChangeAddress);
#endif
					if (((COrder*)pParam)->ModelControl.ModeChangeAddress == CurrentRunCommandNum)
					{
						((COrder*)pParam)->ModelControl.ModeChangeAddress = -1;//初始化雷射模式跳轉地址
#ifdef PRINTF
						_cwprintf(L"SubroutineThread()::%d\n", ((COrder*)pParam)->ModelControl.ModeChangeAddress);
#endif
					}
					else
					{
#ifdef PRINTF
						_cwprintf(L"SubroutineThread()::雷射出現錯誤!\n");
#endif 
					} 
				}
			}
			//判斷雷射高度開，雷射模式中，正要關閉雷射高度
			if (((COrder*)pParam)->LaserSwitch.LaserHeight && ((COrder*)pParam)->ModelControl.Mode == 2 && !_ttol(CommandResolve(Command, 1)) && !((COrder*)pParam)->ModelControl.LaserAndCheckModeJump)
			{
				//判斷是否有Subroutine存在
				if (!((COrder*)pParam)->Program.SubroutineStack.size())//不存在
				{
					//將模式轉換為運動模式
					((COrder*)pParam)->ModelControl.Mode = 3;
					//跳至雷射模式轉換地址
					((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)) = ((COrder*)pParam)->ModelControl.ModeChangeAddress - 1;
					//判斷StepRepeat是否有強行跳轉
					StepRepeatJumpforciblyJudge(pParam, ((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)));
					((COrder*)pParam)->LaserSwitch = { 0,0,0,0,0 };
					((COrder*)pParam)->RepeatData = ((COrder*)pParam)->RepeatDataRecord;//恢復StepRepeat參數
#ifdef PRINTF
					_cwprintf(L"SubroutineThread()::LaserHeight(0)模式轉換(2->3)跳至地址:%d\n\n下一個模式即將開始...\n", ((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)));
#endif
				}
				else//存在
				{
					if (((COrder*)pParam)->Program.SubroutineModel.back() == 2)//判斷存在的Subroutine呼叫時的模式
					{
						((COrder*)pParam)->Program.SubroutineModelControlSwitch = TRUE;
					}
				}
			}
			//紀錄掃描點和開關
			((COrder*)pParam)->LaserSwitch.LaserHeight = _ttol(CommandResolve(Command, 1));
			if (((COrder*)pParam)->LaserSwitch.LaserHeight)
			{
				((COrder*)pParam)->LaserData.LaserHeightPoint.X = _ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X;
				((COrder*)pParam)->LaserData.LaserHeightPoint.Y = _ttol(CommandResolve(Command, 3)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y;
			}
			//判斷是否模式轉換
			if (((COrder*)pParam)->LaserSwitch.LaserHeight && ((COrder*)pParam)->LaserSwitch.LaserDetect)//兩種模式都存在取消一種
			{
				((COrder*)pParam)->LaserSwitch.LaserDetect = FALSE;

				if (((COrder*)pParam)->ModelControl.Mode == 2)//在雷射模式下動作
				{
					((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Status = FALSE;
				}
			}
		}  
	}
	else if (CommandResolve(Command, 0) == L"LaserPointAdjust")
	{
		if (((COrder*)pParam)->ModelControl.Mode == 0)//建表模式
		{
			RecordCorrectionTable(pParam);//寫入修正表(因為需要做影像修正)
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 1)
		{
			RecordCorrectionTable(pParam);//寫入修正表
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 2)//在雷射模式下動作
		{
			((COrder*)pParam)->LaserSwitch.LaserPointAdjust = _ttol(CommandResolve(Command, 1));
			//模式轉換
			if (((COrder*)pParam)->LaserSwitch.LaserAdjust && ((COrder*)pParam)->LaserSwitch.LaserPointAdjust)//兩個模式都成立取消一個
			{
				((COrder*)pParam)->LaserSwitch.LaserAdjust = FALSE;

				((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Status = FALSE;
			}
			//雷射掃描模式一、模式四
			if (((COrder*)pParam)->LaserSwitch.LaserPointAdjust && ((COrder*)pParam)->LaserSwitch.LaserHeight)//雷射模式一掃描
			{
				/*單點測高*/
				ChooseVisionModify(pParam);//選擇影像Offset
				((COrder*)pParam)->FinalWorkCoordinateData.X = ((COrder*)pParam)->LaserData.LaserHeightPoint.X;
				((COrder*)pParam)->FinalWorkCoordinateData.Y = ((COrder*)pParam)->LaserData.LaserHeightPoint.Y;
				((COrder*)pParam)->FinalWorkCoordinateData.Z = ((COrder*)pParam)->m_Action.m_HeightLaserZero;
				if (((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Status)
				{
					((COrder*)pParam)->NVMVirtualCoordinateData = ((COrder*)pParam)->FinalWorkCoordinateData;//紀錄CallSubroutine點(不加影像修正時的值)
				}
				VisionModify(pParam);//影像修正
#ifdef MOVE
				((COrder*)pParam)->m_Action.LA_Dot3D(
					((COrder*)pParam)->FinalWorkCoordinateData.X,
					((COrder*)pParam)->FinalWorkCoordinateData.Y,
					((COrder*)pParam)->LaserData.LaserMeasureHeight,
					((COrder*)pParam)->LMPSpeedSet.EndSpeed, ((COrder*)pParam)->LMPSpeedSet.AccSpeed, ((COrder*)pParam)->LMPSpeedSet.InitSpeed);
#endif
				//紀錄測量高度至雷射修正表
				((COrder*)pParam)->LaserCount++;
				((COrder*)pParam)->LaserAdjust.push_back({ ((COrder*)pParam)->LaserData.LaserMeasureHeight });
			}
			else if (((COrder*)pParam)->LaserSwitch.LaserPointAdjust && ((COrder*)pParam)->LaserSwitch.LaserDetect)//雷射模式四掃描
			{
				/*線段測高平均*/
				CPoint MeasureLS = NULL;
				CPoint MeasureLE = NULL;
				ChooseVisionModify(pParam);//選擇影像Offset
				((COrder*)pParam)->FinalWorkCoordinateData.X = ((COrder*)pParam)->LaserData.LaserDetectLS.X;
				((COrder*)pParam)->FinalWorkCoordinateData.Y = ((COrder*)pParam)->LaserData.LaserDetectLS.Y;
				VisionModify(pParam);//影像修正
				MeasureLS.x = ((COrder*)pParam)->FinalWorkCoordinateData.X;
				MeasureLS.y = ((COrder*)pParam)->FinalWorkCoordinateData.Y;
				((COrder*)pParam)->FinalWorkCoordinateData.X = ((COrder*)pParam)->LaserData.LaserDetectLE.X;
				((COrder*)pParam)->FinalWorkCoordinateData.Y = ((COrder*)pParam)->LaserData.LaserDetectLE.Y;
				((COrder*)pParam)->FinalWorkCoordinateData.Z = ((COrder*)pParam)->m_Action.m_HeightLaserZero;

				if (((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Status)//線段時取結束點
				{
					((COrder*)pParam)->NVMVirtualCoordinateData = ((COrder*)pParam)->FinalWorkCoordinateData;//紀錄CallSubroutine點(不加影像修正時的值)
				}

				VisionModify(pParam);//影像修正
				MeasureLE.x = ((COrder*)pParam)->FinalWorkCoordinateData.X;
				MeasureLE.y = ((COrder*)pParam)->FinalWorkCoordinateData.Y;
				//測量線段平均高度
#ifdef MOVE
				((COrder*)pParam)->m_Action.LA_AverageZ(MeasureLS.x, MeasureLS.y,
					MeasureLE.x, MeasureLE.y,
					((COrder*)pParam)->LaserData.LaserMeasureHeight,
					((COrder*)pParam)->LMPSpeedSet.EndSpeed, ((COrder*)pParam)->LMPSpeedSet.AccSpeed, ((COrder*)pParam)->LMPSpeedSet.InitSpeed,
					((COrder*)pParam)->LMCSpeedSet.EndSpeed, ((COrder*)pParam)->LMCSpeedSet.AccSpeed, ((COrder*)pParam)->LMCSpeedSet.InitSpeed
					);
#endif
				//紀錄測量高度至雷射修正表
				((COrder*)pParam)->LaserCount++;
				((COrder*)pParam)->LaserAdjust.push_back({ ((COrder*)pParam)->LaserData.LaserMeasureHeight });
			}
#ifdef PRINTF
			_cwprintf(L"SubroutineThread()::檢測完畢LaserCount:%d\n", ((COrder*)pParam)->LaserCount);
#endif
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 3)//在運動模式下動作
		{   
			if (_ttol(CommandResolve(Command, 1)) && ((COrder*)pParam)->LaserSwitch.LaserHeight)
			{
				ChooseVisionModify(pParam);//選擇影像Offset
				((COrder*)pParam)->FinalWorkCoordinateData.X = ((COrder*)pParam)->LaserData.LaserHeightPoint.X;
				((COrder*)pParam)->FinalWorkCoordinateData.Y = ((COrder*)pParam)->LaserData.LaserHeightPoint.Y;
				((COrder*)pParam)->FinalWorkCoordinateData.Z = ((COrder*)pParam)->m_Action.m_HeightLaserZero;
				if (((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Status)
				{
					((COrder*)pParam)->NVMVirtualCoordinateData = ((COrder*)pParam)->FinalWorkCoordinateData;//紀錄CallSubroutine點(不加影像修正時的值)
				}
				VisionModify(pParam);//影像修正
				((COrder*)pParam)->VirtualCoordinateData = ((COrder*)pParam)->FinalWorkCoordinateData;//紀錄移動虛擬座標
			}
			else if (_ttol(CommandResolve(Command, 1)) && ((COrder*)pParam)->LaserSwitch.LaserDetect)
			{
				/*線段測高(依結束點)*/
				ChooseVisionModify(pParam);//選擇影像Offset
				((COrder*)pParam)->FinalWorkCoordinateData.X = ((COrder*)pParam)->LaserData.LaserDetectLE.X;
				((COrder*)pParam)->FinalWorkCoordinateData.Y = ((COrder*)pParam)->LaserData.LaserDetectLE.Y;
				((COrder*)pParam)->FinalWorkCoordinateData.Z = ((COrder*)pParam)->m_Action.m_HeightLaserZero;
				if (((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Status)//線段時取結束點
				{
					((COrder*)pParam)->NVMVirtualCoordinateData = ((COrder*)pParam)->FinalWorkCoordinateData;//紀錄CallSubroutine點(不加影像修正時的值)
				}
				VisionModify(pParam);//影像修正
				((COrder*)pParam)->VirtualCoordinateData = ((COrder*)pParam)->FinalWorkCoordinateData;//紀錄移動虛擬座標
			}
		}
		
	}
	else if (CommandResolve(Command, 0) == L"LaserDetect")
	{
		if (((COrder*)pParam)->ModelControl.Mode == 1 || ((COrder*)pParam)->ModelControl.Mode == 2 || ((COrder*)pParam)->ModelControl.Mode == 3)//在任何模式下動作
		{
			CString CommandBuff;
			CommandBuff.Format(_T("LaserDetect,%d,%d,%d"), _ttol(CommandResolve(Command, 2)), _ttol(CommandResolve(Command, 3)), ((COrder*)pParam)->m_Action.m_HeightLaserZero);
			ModifyPointOffSet(pParam, CommandBuff);//CallSubroutin相對位修正
			if (!((COrder*)pParam)->LaserSwitch.LaserHeight && !((COrder*)pParam)->LaserSwitch.LaserDetect && ((COrder*)pParam)->ModelControl.Mode == 3 && _ttol(CommandResolve(Command, 1)) && !((COrder*)pParam)->ModelControl.LaserAndCheckModeJump)
			{
				if (((COrder*)pParam)->ModelControl.ModeChangeAddress == -1)
				{
					//判斷現在是否有StepRepeat
					if (((COrder*)pParam)->RepeatData.StepRepeatNum.size())
					{
						((COrder*)pParam)->RepeatDataRecord = ((COrder*)pParam)->RepeatData;//保留StepRepeat執行參數
					}
					//將模式轉換為雷射模式
					((COrder*)pParam)->ModelControl.Mode = 2;
					//紀錄模式轉換地址
					((COrder*)pParam)->ModelControl.ModeChangeAddress = ((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount));
#ifdef PRINTF
					_cwprintf(L"SubroutineThread()::模式轉換紀錄地址:%d\n", ((COrder*)pParam)->ModelControl.ModeChangeAddress);
#endif
				}
				else
				{
#ifdef PRINTF
					_cwprintf(L"SubroutineThread()::目前執行指令地址:%d\t雷射執行過紀錄陣列:%d", CurrentRunCommandNum, ((COrder*)pParam)->ModelControl.ModeChangeAddress);
#endif
					if (((COrder*)pParam)->ModelControl.ModeChangeAddress == CurrentRunCommandNum)
					{
						((COrder*)pParam)->ModelControl.ModeChangeAddress = -1;
#ifdef PRINTF
						_cwprintf(L"SubroutineThread()::%d\n", ((COrder*)pParam)->ModelControl.ModeChangeAddress);
#endif
					}
					else
					{
#ifdef PRINTF
						_cwprintf(L"SubroutineThread()::雷射出現錯誤!\n");
#endif 
					}
				}
			}
			if (((COrder*)pParam)->LaserSwitch.LaserDetect && ((COrder*)pParam)->ModelControl.Mode == 2 && !_ttol(CommandResolve(Command, 1)) && !((COrder*)pParam)->ModelControl.LaserAndCheckModeJump)
			{
				if (!((COrder*)pParam)->Program.SubroutineStack.size())
				{
					//將模式轉換為運動模式
					((COrder*)pParam)->ModelControl.Mode = 3;
					//紀錄模式轉換地址
					((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)) = ((COrder*)pParam)->ModelControl.ModeChangeAddress - 1;
					//判斷StepRepeat是否有強行跳轉
					StepRepeatJumpforciblyJudge(pParam, ((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)));
					((COrder*)pParam)->RepeatData = ((COrder*)pParam)->RepeatDataRecord;//恢復StepRepeat參數
					((COrder*)pParam)->LaserSwitch = { 0,0,0,0,0 };
#ifdef PRINTF
					_cwprintf(L"SubroutineThread()::LaserDetect(0)模式轉換跳至地址:%d\n\n下一個模式即將開始...\n", ((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)));
#endif
				}
				else
				{
					if (((COrder*)pParam)->Program.SubroutineModel.back() == 2)
					{
						((COrder*)pParam)->Program.SubroutineModelControlSwitch = TRUE;
					}
				}
			}
			//紀錄掃描點和開關
			((COrder*)pParam)->LaserSwitch.LaserDetect = _ttol(CommandResolve(Command, 1));
			if (((COrder*)pParam)->LaserSwitch.LaserDetect)
			{
				((COrder*)pParam)->LaserData.LaserDetectLS.X = _ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X;
				((COrder*)pParam)->LaserData.LaserDetectLS.Y = _ttol(CommandResolve(Command, 3)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y;
				((COrder*)pParam)->LaserData.LaserDetectLE.X = _ttol(CommandResolve(Command, 4)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X;
				((COrder*)pParam)->LaserData.LaserDetectLE.Y = _ttol(CommandResolve(Command, 5)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y;
			}
			//判斷是否模式轉換
			if (((COrder*)pParam)->LaserSwitch.LaserHeight && ((COrder*)pParam)->LaserSwitch.LaserDetect)//兩種模式都存在取消一種
			{
				((COrder*)pParam)->LaserSwitch.LaserHeight = FALSE;
				if (((COrder*)pParam)->ModelControl.Mode == 2)//在雷射模式下動作
				{
					((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Status = FALSE;
				}
			}
		}
	}
	else if (CommandResolve(Command, 0) == L"LaserAdjust")
	{
		if (((COrder*)pParam)->ModelControl.Mode == 2)//在雷射模式下動作
		{
			((COrder*)pParam)->LaserSwitch.LaserAdjust = _ttol(CommandResolve(Command, 1));
			if (((COrder*)pParam)->LaserSwitch.LaserAdjust && ((COrder*)pParam)->LaserSwitch.LaserPointAdjust)//兩個模式都成立取消一個
			{
				((COrder*)pParam)->LaserSwitch.LaserPointAdjust = FALSE;

				((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Status = FALSE;
			}
		}
	} 
	else if (CommandResolve(Command, 0) == L"LaserSkip")
	{
		if (((COrder*)pParam)->ModelControl.Mode == 2)//在雷射模式下動作
		{
			((COrder*)pParam)->LaserSwitch.LaserSkip = _ttol(CommandResolve(Command, 1));
		}
	}
	/************************************************************檢測****************************************************************/
	else if (CommandResolve(Command, 0) == L"TemplateCheck")
	{
		if (((COrder*)pParam)->ModelControl.Mode == 3 && !((COrder*)pParam)->ModelControl.LaserAndCheckModeJump)//在運動模式下動作
		{
			if (_ttol(CommandResolve(Command, 1)) == 1)//即時有效
			{
				((COrder*)pParam)->ClearCheckData(TRUE, FALSE);//即時檢測資料清除
				((COrder*)pParam)->CheckSwitch.ImmediateCheck = 1;//模板即時檢測開關開啟
				((COrder*)pParam)->TemplateChecking.Address = ((COrder*)pParam)->GetCommandAddress();//加入檢測地址
				((COrder*)pParam)->TemplateChecking.VisionParam = { 0,((COrder*)pParam)->VisionSet.Accuracy,((COrder*)pParam)->VisionSet.Speed,
					(BYTE)_ttoi(CommandResolve(Command,5)),((COrder*)pParam)->VisionSet.width,((COrder*)pParam)->VisionSet.height,
					((COrder*)pParam)->VisionSet.Startangle,((COrder*)pParam)->VisionSet.Endangle,0,0,0 };//加入比對參數
				ModelLoad(1, pParam, CommandResolve(Command, 3), ((COrder*)pParam)->TemplateChecking);//載入OK模板(指針、數量)
				ModelLoad(0, pParam, CommandResolve(Command, 4), ((COrder*)pParam)->TemplateChecking);//載入NG模板(指針、數量) 
			}
			else if (_ttol(CommandResolve(Command, 1)) == 0)//區間有效
			{
				//開啟模板檢測
				if (_ttol(CommandResolve(Command, 2)))
				{
					//LineTrainDataCheck(pParam);//判斷是否有新增換線點
					((COrder*)pParam)->CheckSwitch.Template = TRUE;
					((COrder*)pParam)->CheckSwitch.Diameter = FALSE;
					((COrder*)pParam)->CheckSwitch.Area = FALSE; 
					((COrder*)pParam)->CurrentCheckAddress = ((COrder*)pParam)->GetCommandAddress();//更改目前區間檢測地址
					//判斷模板檢測是否新增過
					BOOL ScanResult = FALSE;
					for (UINT i = 0; i < ((COrder*)pParam)->IntervalTemplateCheck.size(); i++)
					{
						if (((COrder*)pParam)->CurrentCheckAddress == ((COrder*)pParam)->IntervalTemplateCheck.at(i).Address)//判斷是否有重複的模板檢測地址
						{
							ScanResult = TRUE;
						}
					}
					if (!ScanResult)
					{
						TemplateCheck TemplateCheckInit;
						TemplateCheckInit.Address = ((COrder*)pParam)->GetCommandAddress();//加入檢測地址
						TemplateCheckInit.VisionParam = { 0,((COrder*)pParam)->VisionSet.Accuracy,((COrder*)pParam)->VisionSet.Speed,
							(BYTE)_ttoi(CommandResolve(Command,5)),((COrder*)pParam)->VisionSet.width,((COrder*)pParam)->VisionSet.height,
							((COrder*)pParam)->VisionSet.Startangle,((COrder*)pParam)->VisionSet.Endangle,0,0,0 };//加入比對參數
						ModelLoad(1, pParam, CommandResolve(Command, 3), TemplateCheckInit);//載入OK模板(指針、數量)
						ModelLoad(0, pParam, CommandResolve(Command, 4), TemplateCheckInit);//載入NG模板(指針、數量)
						((COrder*)pParam)->IntervalTemplateCheck.push_back(TemplateCheckInit);
					}
				} 
				//目前已經有開啟模板檢測並且要關閉
				else if (((COrder*)pParam)->CheckSwitch.Template && !((COrder*)pParam)->CheckSwitch.Diameter && !((COrder*)pParam)->CheckSwitch.Area && !_ttol(CommandResolve(Command, 2)))
				{
					((COrder*)pParam)->CheckSwitch.Template = FALSE;
					((COrder*)pParam)->CheckSwitch.Diameter = FALSE;
					((COrder*)pParam)->CheckSwitch.Area = FALSE;
					((COrder*)pParam)->CheckSwitch.RunCheck = TRUE;
				}
			}
		}
	}
	else if (CommandResolve(Command, 0) == L"DiameterCheck")
	{
		if (((COrder*)pParam)->ModelControl.Mode == 3 && !((COrder*)pParam)->ModelControl.LaserAndCheckModeJump)//在運動模式下動作
		{
			if (_ttol(CommandResolve(Command, 1)) == 1)//即時有效
			{
				((COrder*)pParam)->ClearCheckData(TRUE, FALSE);//即時檢測資料清除
				((COrder*)pParam)->CheckSwitch.ImmediateCheck = 2;//直徑即時檢測開關開啟 
				((COrder*)pParam)->DiameterChecking.Address = ((COrder*)pParam)->GetCommandAddress();//加入檢測地址
				((COrder*)pParam)->DiameterChecking.Diameter = _tstof(CommandResolve(Command, 3));//設定直徑
				((COrder*)pParam)->DiameterChecking.Tolerance = _tstof(CommandResolve(Command, 4));//設定容許誤差
				((COrder*)pParam)->DiameterChecking.Color = _ttol(CommandResolve(Command, 5));//設定色階
				((COrder*)pParam)->DiameterChecking.Binarization = _tstof(CommandResolve(Command, 6));//設定二值化界限值
			}
			else if (_ttol(CommandResolve(Command, 1)) == 0)//區間有效
			{
				//開啟直徑檢測
				if (_ttol(CommandResolve(Command, 2)))
				{
					//LineTrainDataCheck(pParam);//判斷是否有新增換線點
					((COrder*)pParam)->CheckSwitch.Template = FALSE;
					((COrder*)pParam)->CheckSwitch.Diameter = TRUE;
					((COrder*)pParam)->CheckSwitch.Area = FALSE;
					((COrder*)pParam)->CurrentCheckAddress = ((COrder*)pParam)->GetCommandAddress();//更改目前區間檢測地址
					//判斷直徑檢測是否新增過
					BOOL ScanResult = FALSE;
					for (UINT i = 0; i < ((COrder*)pParam)->IntervalDiameterCheck.size(); i++)
					{
						if (((COrder*)pParam)->CurrentCheckAddress == ((COrder*)pParam)->IntervalDiameterCheck.at(i).Address)//判斷是否有重複的直徑檢測地址
						{
							ScanResult = TRUE;
						}
					}
					if (!ScanResult)
					{
						DiameterCheck DiameterCheckInit;
						DiameterCheckInit.Address = ((COrder*)pParam)->GetCommandAddress();//加入檢測地址
						DiameterCheckInit.Diameter = _tstof(CommandResolve(Command, 3));//設定直徑
						DiameterCheckInit.Tolerance = _tstof(CommandResolve(Command, 4));//設定容許誤差
						DiameterCheckInit.Color = _ttol(CommandResolve(Command, 5));//設定色階
						DiameterCheckInit.Binarization = _tstof(CommandResolve(Command, 6));//設定二值化界限值
						((COrder*)pParam)->IntervalDiameterCheck.push_back(DiameterCheckInit);//加入陣列中
					}
				}
				//目前已經有開啟模板檢測並且要關閉
				else if (((COrder*)pParam)->CheckSwitch.Diameter && !((COrder*)pParam)->CheckSwitch.Template && !((COrder*)pParam)->CheckSwitch.Area && !_ttol(CommandResolve(Command, 2)))
				{
					((COrder*)pParam)->CheckSwitch.Template = FALSE;
					((COrder*)pParam)->CheckSwitch.Diameter = FALSE;
					((COrder*)pParam)->CheckSwitch.Area = FALSE;
					((COrder*)pParam)->CheckSwitch.RunCheck = TRUE;
				}
			}
		}
	}
	else if (CommandResolve(Command, 0) == L"AreaCheck")
	{
		if (((COrder*)pParam)->ModelControl.Mode == 0)//建表模式
		{
			RecordCorrectionTable(pParam);//寫入修正表(因為需要做影像修正)
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 1)
		{
			RecordCorrectionTable(pParam);//寫入修正表
		}
		if (((COrder*)pParam)->ModelControl.Mode == 3 && !((COrder*)pParam)->ModelControl.LaserAndCheckModeJump)//在運動模式下動作
		{
			if (_ttol(CommandResolve(Command, 1)))//開啟區域檢測
			{  
				//LineTrainDataCheck(pParam);//判斷是否有新增換線點
				//((COrder*)pParam)->AreaCheckChangTemp.Status = TRUE;//開啟中間點智能
				((COrder*)pParam)->CheckSwitch.Template = FALSE;
				((COrder*)pParam)->CheckSwitch.Diameter = FALSE;
				((COrder*)pParam)->CheckSwitch.Area = TRUE;
				((COrder*)pParam)->CurrentCheckAddress = ((COrder*)pParam)->GetCommandAddress();//更改目前區間檢測地址
				BOOL ScanResult = FALSE;
				for (UINT i = 0; i < ((COrder*)pParam)->IntervalAreaCheck.size(); i++)
				{
					if (((COrder*)pParam)->CurrentCheckAddress == ((COrder*)pParam)->IntervalAreaCheck.at(i).Address)//判斷是否有重複的直徑檢測地址
					{
						ScanResult = TRUE;
					}
				}
				if (!ScanResult)
				{
					AreaCheck AreaCheckInit;
					AreaCheckInit.Address = ((COrder*)pParam)->CurrentCheckAddress;//加入檢測地址
					AreaCheckInit.Image.ImageSave = ((COrder*)pParam)->AreaCheckParamterDefault.ImageSave;//加入重組圖路徑
					AreaCheckInit.Image.Start = { _ttol(CommandResolve(Command, 2)), _ttol(CommandResolve(Command, 3)) };//加入重組圖起始點
					AreaCheckInit.Image.End = { _ttol(CommandResolve(Command, 4)), _ttol(CommandResolve(Command, 5)) };//加入重組圖結束點
					AreaCheckInit.Image.ViewMove = ((COrder*)pParam)->AreaCheckParamterDefault.ViewMove;//加入重組圖視野移動量

					AreaCheckInit.DotTrain.TrainSave = ((COrder*)pParam)->AreaCheckParamterDefault.DotTrainSave;//加入點訓練路徑
					AreaCheckInit.DotTrain.MeasureLimit = _tstof(CommandResolve(Command, 6));//加入點訓練直徑
					AreaCheckInit.DotTrain.MaxOffset = _tstof(CommandResolve(Command, 7));//加入點訓練容許誤差
					AreaCheckInit.DotTrain.WhiteOrBlack = _ttol(CommandResolve(Command, 10));//加入點訓練色階
					AreaCheckInit.DotTrain.Threshold = _tstof(CommandResolve(Command, 11));//加入點訓練二值化

					AreaCheckInit.LineTrain.TrainSave = ((COrder*)pParam)->AreaCheckParamterDefault.LineTrainSave;//加入線訓練路徑
					AreaCheckInit.LineTrain.MeasureLimit = _tstof(CommandResolve(Command, 8));//加入線訓練線寬
					AreaCheckInit.LineTrain.MaxOffset = _tstof(CommandResolve(Command, 9));//加入線訓練容許誤差
					AreaCheckInit.LineTrain.WhiteOrBlack = _ttol(CommandResolve(Command, 10));//加入線訓練色階
					AreaCheckInit.LineTrain.Threshold = _tstof(CommandResolve(Command, 11));//加入線訓練二值化

					AreaCheckInit.Result.Path = ((COrder*)pParam)->AreaCheckParamterDefault.Result.Path;//加入結果圖存放路徑
					

					((COrder*)pParam)->IntervalAreaCheck.push_back(AreaCheckInit);//加入區域檢測陣列中
				}
			}
			else if (((COrder*)pParam)->CheckSwitch.Area && !((COrder*)pParam)->CheckSwitch.Diameter && !((COrder*)pParam)->CheckSwitch.Template && !_ttol(CommandResolve(Command, 1)))
			{
				for (UINT i = 0; i < ((COrder*)pParam)->IntervalAreaCheck.size(); i++)
				{
					if (((COrder*)pParam)->IntervalAreaCheck.at(i).Address == ((COrder*)pParam)->CurrentCheckAddress)//找出先前的檢測指令
					{
						if (((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData.size())
						{
							if (((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData.back().x != -1 &&
								((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData.back().y != -1)//判斷是否有換線點
							{
								((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData.push_back({ -1,-1 });
							}
						}
					}
				}
				((COrder*)pParam)->CheckSwitch.Template = FALSE;
				((COrder*)pParam)->CheckSwitch.Diameter = FALSE;
				((COrder*)pParam)->CheckSwitch.Area = FALSE;
				((COrder*)pParam)->CheckSwitch.RunCheck = TRUE;
			}
		}
	}
	((COrder*)pParam)->RunStatusRead.StepCommandStatus = FALSE;
	g_pSubroutineThread = NULL;
	::SetEvent(((COrder*)pParam)->ThreadEvent.Thread);
	return 0;
}
/*I/O偵測執行緒*/
UINT COrder::IODetection(LPVOID pParam)
{
#ifdef MOVE
	while (((COrder*)pParam)->IOControl.SwitchInformation)
	{
		//讀取出膠按鈕
		if (MO_ReadGlueOutBtn())
		{
			if (((COrder*)pParam)->RunStatusRead.RunStatus != 1)//不在運行中才可以使用
			{
				((COrder*)pParam)->IOControl.GlueInformation = TRUE;
				MO_GummingSet();//出膠
			}
		}
		else
		{
			if (((COrder*)pParam)->IOControl.GlueInformation)
			{
				MO_StopGumming();//停止出膠，清除Timer
				((COrder*)pParam)->IOControl.GlueInformation = FALSE;
			}
		}
		//讀取開始按鈕
		if (MO_ReadStartBtn())
		{
			((COrder*)pParam)->Home(((COrder*)pParam)->GoHome.VisionGoHome);
		}
		//讀取EMG按鈕
		if (MO_ReadEMG())
		{
			((COrder*)pParam)->Stop();
			if (((COrder*)pParam)->IOParam.pEMGDlg != NULL)
			{
				switch (((COrder*)pParam)->IOParam.pEMGDlg->DoModal())
				{
				case 0:
					((COrder*)pParam)->Home(((COrder*)pParam)->GoHome.VisionGoHome);
					Sleep(10);
					break;
				default:
					break;
				}
			}
		} 
		//讀取Switch修改出膠動作
		((COrder*)pParam)->m_Action.m_bIsDispend = MO_ReadSwitchBtn();
	}
#endif
	g_pIODetectionThread = NULL;
	return 0;
}
/*區間檢測點掃描執行續*/
UINT COrder::CheckCoordinateScan(LPVOID pParam)
{
	//判斷是否有模板、直徑的區間檢測點
	if (((COrder*)pParam)->IntervalCheckCoordinate.size())
	{
		for (UINT i = 0; i < ((COrder*)pParam)->IntervalCheckCoordinate.size(); i++)//掃描檢測點
		{
			if (!((COrder*)pParam)->m_Action.m_bIsStop)
			{
				((COrder*)pParam)->CheckCoordinateRun = ((COrder*)pParam)->IntervalCheckCoordinate.at(i);//設定檢測座標
				((COrder*)pParam)->CheckModel = 2;//設定檢測模式
				::ResetEvent(((COrder*)pParam)->ThreadEvent.CheckActionThread);//釋放事件
				g_pCheckActionThread = AfxBeginThread(((COrder*)pParam)->CheckAction, pParam);
				::WaitForSingleObject(((COrder*)pParam)->ThreadEvent.CheckActionThread, INFINITE);//等待事件被設置
				/*舊版2017/04/05*/
				//while (g_pCheckActionThread) {
				//	Sleep(1);//while 程式負載問題 無限迴圈，並讓 CPU 休息一下
				//}
			} 
		} 
		((COrder*)pParam)->IntervalCheckCoordinate.clear();//清除檢測點陣列
#ifdef VI
		VI_DrawFOVDefault();//清除CCD畫面 
#endif
	}
	//判斷區域檢測是否有資料
	if (((COrder*)pParam)->IntervalAreaCheck.size())
	{
		for (UINT i = 0; i < ((COrder*)pParam)->IntervalAreaCheck.size(); i++)
		{
			if (((COrder*)pParam)->IntervalAreaCheck.at(i).DotTrain.PointData.size() || ((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData.size())//判斷是否有訓練點
			{
				((COrder*)pParam)->AreaCheckRun = ((COrder*)pParam)->IntervalAreaCheck.at(i);//設定區域檢測資料
				((COrder*)pParam)->CheckModel = 3;//設定檢測模式
				::ResetEvent(((COrder*)pParam)->ThreadEvent.CheckActionThread);//釋放事件
				g_pCheckActionThread = AfxBeginThread(((COrder*)pParam)->CheckAction, pParam);
				::WaitForSingleObject(((COrder*)pParam)->ThreadEvent.CheckActionThread, INFINITE);//等待事件被設置		
				/*舊版2017/04/05*/
				//while (g_pCheckActionThread) {
				//	Sleep(1);//while 程式負載問題 無限迴圈，並讓 CPU 休息一下
				//}
			}
		}
	}
	((COrder*)pParam)->ClearCheckData(FALSE,TRUE);//區間檢測資料釋放
	((COrder*)pParam)->CheckSwitch.RunCheck = FALSE;//區間檢測運行關閉
	g_pCheckCoordinateScanThread = NULL;
	::SetEvent(((COrder*)pParam)->ThreadEvent.CheckCoordinateScanThread);
	return 0;
}
/*檢測點執行執行續*/
UINT COrder::CheckAction(LPVOID pParam)
{
	if (((COrder*)pParam)->CheckModel == 1)//即時檢測動作
	{
		if (!((COrder*)pParam)->m_Action.m_bIsStop)
		{
#ifdef MOVE
			//影像移動至檢測點上(使用對焦高度)
			((COrder*)pParam)->m_Action.DecideVirtualPoint(
				((COrder*)pParam)->FinalWorkCoordinateData.X - ((COrder*)pParam)->VisionSet.AdjustOffsetX,
				((COrder*)pParam)->FinalWorkCoordinateData.Y - ((COrder*)pParam)->VisionSet.AdjustOffsetY,
				((COrder*)pParam)->VisionSet.FocusHeight,
				0,
				((COrder*)pParam)->DotSpeedSet.EndSpeed, ((COrder*)pParam)->DotSpeedSet.AccSpeed, ((COrder*)pParam)->DotSpeedSet.InitSpeed);
#endif 
		}   
		if (!((COrder*)pParam)->m_Action.m_bIsStop)
		{
			CString Buff = L"";
			if (((COrder*)pParam)->CheckSwitch.ImmediateCheck == 1)//模板即時檢測
			{
				BOOL OKCheck = FALSE;
				BOOL NGCheck = FALSE;
#ifdef VI
				//判斷OK是否有模板
				if (((COrder*)pParam)->TemplateChecking.OKModelCount)
				{	
					Sleep(VI_TCheckDelayTime);
					OKCheck = VI_FindMatrixModel(((COrder*)pParam)->TemplateChecking.OKModel, ((COrder*)pParam)->TemplateChecking.OKModelCount);//OK比對
				}
				//判斷NG是否有模板
				if (((COrder*)pParam)->TemplateChecking.NGModelCount)
				{
					Sleep(VI_TCheckDelayTime);
					NGCheck = VI_FindMatrixModel(((COrder*)pParam)->TemplateChecking.NGModel, ((COrder*)pParam)->TemplateChecking.NGModelCount);//NG比對
				}
				//判斷OK、NG計數
				if (((COrder*)pParam)->TemplateChecking.OKModelCount && !((COrder*)pParam)->TemplateChecking.NGModelCount)//只有OK模板
				{
					if (OKCheck)
					{
						((COrder*)pParam)->CheckResult.OKCount++;
						Buff = L"OK";
					}
					else
					{
						((COrder*)pParam)->CheckResult.NGCount++;
						Buff = L"NG";
					}
				}
				else if (!((COrder*)pParam)->TemplateChecking.OKModelCount && ((COrder*)pParam)->TemplateChecking.NGModelCount)//只有NG模板
				{
					if (NGCheck)
					{
						((COrder*)pParam)->CheckResult.NGCount++;
						Buff = L"NG";
					}
					else
					{
						((COrder*)pParam)->CheckResult.OKCount++;
						Buff = L"OK";
					}
				}
				else if (((COrder*)pParam)->TemplateChecking.OKModelCount && ((COrder*)pParam)->TemplateChecking.NGModelCount)//OK、NG都模板
				{
					if (OKCheck && !NGCheck)
					{
						((COrder*)pParam)->CheckResult.OKCount++;
						Buff = L"OK";
					}
					else
					{
						if (OKCheck && NGCheck)//兩種模板都找到跳Err
						{
							((COrder*)pParam)->CheckResult.Error++;
							Buff = L"Err";
						}
						else
						{
							((COrder*)pParam)->CheckResult.NGCount++;
							Buff = L"NG";
						}
					}
				}
				else
				{
					AfxMessageBox(L"請選擇模板!");
				}
				//紀錄檢測結果
				((COrder*)pParam)->CheckFinishRecord.push_back({ Buff, {NULL,NULL},
				{ L"TemplateCheck",((COrder*)pParam)->TemplateChecking.Address, ((COrder*)pParam)->GetCommandAddress() ,
				{ 1,((COrder*)pParam)->FinalWorkCoordinateData.X, ((COrder*)pParam)->FinalWorkCoordinateData.Y ,((COrder*)pParam)->FinalWorkCoordinateData.Z } } });
				//畫出檢測結果
				if (!((COrder*)pParam)->CheckDraw())
				{
					AfxMessageBox(L"檢測結果繪圖失敗!");
				}
#endif
			}
			else if (((COrder*)pParam)->CheckSwitch.ImmediateCheck == 2)//直徑即時檢測
			{
#ifdef VI
				//建立直徑檢測模板
				VI_CircleBeadTrain(((COrder*)pParam)->DiameterChecking.Diameter, ((COrder*)pParam)->DiameterChecking.Tolerance,
					((COrder*)pParam)->DiameterChecking.Color, ((COrder*)pParam)->DiameterChecking.Binarization);
				//檢測延遲
				Sleep(VI_DCheckDelayTime);
				//直徑檢測
				if (FilePathExist(((COrder*)pParam)->DTCheckParamterDefault.DiameterResultSave.Path))//判斷檔案路徑是否存在
				{
					((COrder*)pParam)->DTCheckParamterDefault.DiameterResultSave.Name = GetDataFileName();
					int tmpBeadVerify = VI_CircleBeadVerify(0, ((COrder*)pParam)->DiameterChecking.Tolerance,
						((COrder*)pParam)->DTCheckParamterDefault.DiameterResultSave.Path, ((COrder*)pParam)->DTCheckParamterDefault.DiameterResultSave.Name);
					if (tmpBeadVerify == 1)
					{
						((COrder*)pParam)->CheckResult.OKCount++;
						Buff = L"OK";
					}
					else if (tmpBeadVerify == 0)
					{
						((COrder*)pParam)->CheckResult.NGCount++;
						Buff = L"NG";
					}
					else
					{
						((COrder*)pParam)->CheckResult.Error++;
						Buff = L"Err";
					}
				}
				//直徑檢測訓練清除
				VI_CircleBeadFree();
				//紀錄檢測結果
				((COrder*)pParam)->CheckFinishRecord.push_back({ Buff, ((COrder*)pParam)->DTCheckParamterDefault.DiameterResultSave,
				{ L"DiameterCheck",((COrder*)pParam)->DiameterChecking.Address, ((COrder*)pParam)->GetCommandAddress() ,
				{ 1,((COrder*)pParam)->FinalWorkCoordinateData.X, ((COrder*)pParam)->FinalWorkCoordinateData.Y ,((COrder*)pParam)->FinalWorkCoordinateData.Z } } });
				//畫出檢測結果
				if (!((COrder*)pParam)->CheckDraw())
				{
					AfxMessageBox(L"檢測結果繪圖失敗!");
				}
#endif
			}
			((COrder*)pParam)->ClearCheckData(TRUE, FALSE);//即時檢測資料清除
#ifdef VI
			VI_DrawFOVDefault();//清除CCD畫面 
#endif
		}
	}
	if (((COrder*)pParam)->CheckModel == 2)//區間檢測動作
	{
		if (!((COrder*)pParam)->m_Action.m_bIsStop)
		{
#ifdef MOVE
			//影像移動至檢測點上
			((COrder*)pParam)->m_Action.DecideVirtualPoint(
				((COrder*)pParam)->CheckCoordinateRun.Position.X - ((COrder*)pParam)->VisionSet.AdjustOffsetX,
				((COrder*)pParam)->CheckCoordinateRun.Position.Y - ((COrder*)pParam)->VisionSet.AdjustOffsetY,
				((COrder*)pParam)->VisionSet.FocusHeight,
				0,
				((COrder*)pParam)->DotSpeedSet.EndSpeed, ((COrder*)pParam)->DotSpeedSet.AccSpeed, ((COrder*)pParam)->DotSpeedSet.InitSpeed);
#endif
		}
		if (!((COrder*)pParam)->m_Action.m_bIsStop)
		{
			CString Buff = L"";
			//掃描檢測點的檢測模式
			for (UINT i = 0; i < ((COrder*)pParam)->IntervalTemplateCheck.size(); i++)//模板檢測搜尋
			{
				if (((COrder*)pParam)->IntervalTemplateCheck.at(i).Address == ((COrder*)pParam)->CheckCoordinateRun.CheckModeAddress)
				{
					BOOL OKCheck = FALSE;
					BOOL NGCheck = FALSE;
#ifdef VI
					//判斷OK是否有模板
					if (((COrder*)pParam)->IntervalTemplateCheck.at(i).OKModelCount)
					{			
						Sleep(VI_TCheckDelayTime);
						OKCheck = VI_FindMatrixModel(((COrder*)pParam)->IntervalTemplateCheck.at(i).OKModel, ((COrder*)pParam)->IntervalTemplateCheck.at(i).OKModelCount);//OK比對
					}
					//判斷NG是否有模板
					if (((COrder*)pParam)->IntervalTemplateCheck.at(i).NGModelCount)
					{		
						Sleep(VI_TCheckDelayTime);
						NGCheck = VI_FindMatrixModel(((COrder*)pParam)->IntervalTemplateCheck.at(i).NGModel, ((COrder*)pParam)->IntervalTemplateCheck.at(i).NGModelCount);//NG比對
					}
#endif
					//判斷OK、NG計數
					if (((COrder*)pParam)->IntervalTemplateCheck.at(i).OKModelCount && !((COrder*)pParam)->IntervalTemplateCheck.at(i).NGModelCount)//只有OK模板
					{
						if (OKCheck)
						{
							((COrder*)pParam)->CheckResult.OKCount++;
							Buff = L"OK";
						}
						else
						{
							((COrder*)pParam)->CheckResult.NGCount++;
							Buff = L"NG";
						}
					}
					else if (!((COrder*)pParam)->IntervalTemplateCheck.at(i).OKModelCount && ((COrder*)pParam)->IntervalTemplateCheck.at(i).NGModelCount)//只有NG模板
					{
						if (NGCheck)
						{
							((COrder*)pParam)->CheckResult.NGCount++;
							Buff = L"NG";
						}
						else
						{
							((COrder*)pParam)->CheckResult.OKCount++;
							Buff = L"OK";
						}
					}
					else if (((COrder*)pParam)->IntervalTemplateCheck.at(i).OKModelCount && ((COrder*)pParam)->IntervalTemplateCheck.at(i).NGModelCount)//OK、NG都模板
					{
						if (OKCheck && !NGCheck)
						{
							((COrder*)pParam)->CheckResult.OKCount++;
							Buff = L"OK";
						}
						else
						{
							if (OKCheck && NGCheck)
							{
								((COrder*)pParam)->CheckResult.Error++;
								Buff = L"Err";
							}
							else
							{
								((COrder*)pParam)->CheckResult.NGCount++;
								Buff = L"NG";
							}
						}
					}
					else
					{
						AfxMessageBox(L"請選擇模板!");
					}
					//紀錄檢測結果
					((COrder*)pParam)->CheckFinishRecord.push_back({ Buff,{ NULL,NULL },((COrder*)pParam)->CheckCoordinateRun });
					//畫出檢測結果
					if (!((COrder*)pParam)->CheckDraw())
					{
						AfxMessageBox(L"檢測結果繪圖失敗!");
					}
				}
			}
			for (UINT i = 0; i < ((COrder*)pParam)->IntervalDiameterCheck.size(); i++)//直徑檢測搜尋
			{
				if (((COrder*)pParam)->IntervalDiameterCheck.at(i).Address == ((COrder*)pParam)->CheckCoordinateRun.CheckModeAddress)
				{
#ifdef VI
					//建立直徑檢測模板
					VI_CircleBeadTrain(((COrder*)pParam)->IntervalDiameterCheck.at(i).Diameter, ((COrder*)pParam)->IntervalDiameterCheck.at(i).Tolerance,
						((COrder*)pParam)->IntervalDiameterCheck.at(i).Color, ((COrder*)pParam)->IntervalDiameterCheck.at(i).Binarization);
					//檢測延遲
					Sleep(VI_DCheckDelayTime);
					//直徑檢測
					if (FilePathExist(((COrder*)pParam)->DTCheckParamterDefault.DiameterResultSave.Path))//判斷檔案路徑是否存在
					{
						((COrder*)pParam)->DTCheckParamterDefault.DiameterResultSave.Name = GetDataFileName();
						int tmpBeadVerify = VI_CircleBeadVerify(0, ((COrder*)pParam)->IntervalDiameterCheck.at(i).Tolerance,
							((COrder*)pParam)->DTCheckParamterDefault.DiameterResultSave.Path, ((COrder*)pParam)->DTCheckParamterDefault.DiameterResultSave.Name);
						if (tmpBeadVerify == 1)
						{
							((COrder*)pParam)->CheckResult.OKCount++;
							Buff = L"OK";
						}
						else if (tmpBeadVerify == 0)
						{
							((COrder*)pParam)->CheckResult.NGCount++;
							Buff = L"NG";
						}
						else
						{
							((COrder*)pParam)->CheckResult.Error++;
							Buff = L"Err";
						}
					}
					//直徑檢測訓練清除
					VI_CircleBeadFree();
					//紀錄檢測結果
					((COrder*)pParam)->CheckFinishRecord.push_back({ Buff,((COrder*)pParam)->DTCheckParamterDefault.DiameterResultSave,((COrder*)pParam)->CheckCoordinateRun });
					//畫出檢測結果
					if (!((COrder*)pParam)->CheckDraw())
					{
						AfxMessageBox(L"檢測結果繪圖失敗!");
					}
#endif
				}
			}
		}
	} 
	if (((COrder*)pParam)->CheckModel == 3)
	{
		int symbolx = 1;//用於起始和結束位置的正負號
		int symboly = 1;//用於起始和結束位置的正負號
		BOOL Switch = FALSE;//控制S型
		if (!((COrder*)pParam)->m_Action.m_bIsStop)
		{
			if (((COrder*)pParam)->AreaCheckRun.Image.Start.x == VI_MosaicAreaDefault &&
				((COrder*)pParam)->AreaCheckRun.Image.Start.y == VI_MosaicAreaDefault &&
				((COrder*)pParam)->AreaCheckRun.Image.End.x == VI_MosaicAreaDefault &&
				((COrder*)pParam)->AreaCheckRun.Image.End.y == VI_MosaicAreaDefault)//判斷區域是否為VI_MosaicAreaDefault
			{
				((COrder*)pParam)->AutoCalculationArea(((COrder*)pParam)->AreaCheckRun);
#ifdef PRINTF
				_cwprintf(L"CheckAction()::重組區域(%d,%d,%d,%d)\n", ((COrder*)pParam)->AreaCheckRun.Image.Start.x, ((COrder*)pParam)->AreaCheckRun.Image.Start.y, ((COrder*)pParam)->AreaCheckRun.Image.End.x, ((COrder*)pParam)->AreaCheckRun.Image.End.y);
#endif
			}
#ifdef VI
			VI_MosaicingImagesSizeCalc(((COrder*)pParam)->AreaCheckRun.Image.Start.x, ((COrder*)pParam)->AreaCheckRun.Image.Start.y, ((COrder*)pParam)->AreaCheckRun.Image.End.x, ((COrder*)pParam)->AreaCheckRun.Image.End.y, ((COrder*)pParam)->AreaCheckRun.Image.MoveCountX, ((COrder*)pParam)->AreaCheckRun.Image.MoveCountY);//計算影像拍圖移動次數
#endif
			//計算移動方向 
			if ((((COrder*)pParam)->AreaCheckRun.Image.End.x - ((COrder*)pParam)->AreaCheckRun.Image.Start.x) < 0)
			{
				symbolx = -1;
			}
			if ((((COrder*)pParam)->AreaCheckRun.Image.End.y - ((COrder*)pParam)->AreaCheckRun.Image.Start.y) < 0)
			{
				symboly = -1;
			}
		}
		if (!((COrder*)pParam)->m_Action.m_bIsStop)
		{
#ifdef PRINTF
			_cwprintf(L"CheckAction()::重組次數X:%d,Y:%d\n", ((COrder*)pParam)->AreaCheckRun.Image.MoveCountX - 1, ((COrder*)pParam)->AreaCheckRun.Image.MoveCountY - 1);
#endif
			for (int i = 0; i <= ((COrder*)pParam)->AreaCheckRun.Image.MoveCountY - 1; i++)
			{
				((COrder*)pParam)->FinalWorkCoordinateData.Y = ((COrder*)pParam)->AreaCheckRun.Image.Start.y + (symboly * i * ((COrder*)pParam)->AreaCheckRun.Image.ViewMove.y);
				if (!((COrder*)pParam)->m_Action.m_bIsStop)
				{
					for (int j = 0; j <= ((COrder*)pParam)->AreaCheckRun.Image.MoveCountX - 1; j++)
					{
						if (!Switch)
							((COrder*)pParam)->FinalWorkCoordinateData.X = ((COrder*)pParam)->AreaCheckRun.Image.Start.x + (symbolx * j * ((COrder*)pParam)->AreaCheckRun.Image.ViewMove.x);
						else
							((COrder*)pParam)->FinalWorkCoordinateData.X = ((COrder*)pParam)->AreaCheckRun.Image.Start.x + (symbolx * (((COrder*)pParam)->AreaCheckRun.Image.MoveCountX - 1 - j) * ((COrder*)pParam)->AreaCheckRun.Image.ViewMove.x);

						ChooseVisionModify(pParam);//選擇影像Offset
						VisionModify(pParam);//影像修正
						if (!((COrder*)pParam)->m_Action.m_bIsStop)
						{
#ifdef MOVE
							//影像移動至拍攝點上
							((COrder*)pParam)->m_Action.DecideVirtualPoint(
								((COrder*)pParam)->FinalWorkCoordinateData.X - ((COrder*)pParam)->VisionSet.AdjustOffsetX,
								((COrder*)pParam)->FinalWorkCoordinateData.Y - ((COrder*)pParam)->VisionSet.AdjustOffsetY,
								((COrder*)pParam)->VisionSet.FocusHeight,
								0,
								((COrder*)pParam)->DotSpeedSet.EndSpeed, ((COrder*)pParam)->DotSpeedSet.AccSpeed, ((COrder*)pParam)->DotSpeedSet.InitSpeed);
#endif
						}
#ifdef VI
						Sleep(VI_MICaptureDelayTime);
						VI_MosaicingImagesCapture();//擷取影像
#endif
					}
					Switch = !Switch;//S形開關轉換
				}
			}
		}
		if (!((COrder*)pParam)->m_Action.m_bIsStop)
		{
#ifdef VI
			if (FilePathExist(((COrder*)pParam)->AreaCheckRun.Image.ImageSave.Path))//判斷檔案路徑是否存在
			{
				((COrder*)pParam)->RunStatusRead.MosaicStatus = -1;//將狀態設為重組中
				::ResetEvent(((COrder*)pParam)->ThreadEvent.MosaicDlgThread);//釋放事件
				g_pMosaicDlgThread = AfxBeginThread(((COrder*)pParam)->MosaicDlg, pParam);//開啟重組中視窗執行緒
				((COrder*)pParam)->RunStatusRead.MosaicStatus = VI_MosaicingImagesProcess(((COrder*)pParam)->AreaCheckRun.Image.ImageSave.Path, ((COrder*)pParam)->AreaCheckRun.Image.ImageSave.Name, 0);//影像重組計算
				::WaitForSingleObject(((COrder*)pParam)->ThreadEvent.MosaicDlgThread, INFINITE);//等待事件被設置
				/*舊版2017/04/05*/
				//while (g_pMosaicDlgThread) {
				//	Sleep(1);//while 程式負載問題 無限迴圈，並讓 CPU 休息一下
				//}
			}
#endif   
		}
		else
		{
#ifdef VI
			VI_MosaicingImagesProcessStop();//發生中斷重組
#endif
		}
		if (!((COrder*)pParam)->m_Action.m_bIsStop)
		{
			if (((COrder*)pParam)->RunStatusRead.MosaicStatus == 1)//重組成功
			{
				CPoint *DotPosition;
				CPoint *LinePosition;
				BOOL DotSwitch = FALSE;
				BOOL LineSwitch = FALSE;
				int Mode = 0;
				//點訓練
				if (((COrder*)pParam)->AreaCheckRun.DotTrain.PointData.size())
				{
					DotSwitch = TRUE;
					DotPosition = new CPoint[((COrder*)pParam)->AreaCheckRun.DotTrain.PointData.size()];
					for (UINT i = 0; i < ((COrder*)pParam)->AreaCheckRun.DotTrain.PointData.size(); i++)
					{
						DotPosition[i] = ((COrder*)pParam)->AreaCheckRun.DotTrain.PointData.at(i);
					}
#ifdef VI
					if (FilePathExist(((COrder*)pParam)->AreaCheckRun.DotTrain.TrainSave.Path))//判斷檔案路徑是否存在
					{
						VI_ImagesCircleBeadTrain(((COrder*)pParam)->AreaCheckRun.DotTrain.TrainSave.Path, ((COrder*)pParam)->AreaCheckRun.DotTrain.TrainSave.Name,
							DotPosition, ((COrder*)pParam)->AreaCheckRun.DotTrain.MeasureLimit, ((COrder*)pParam)->AreaCheckRun.DotTrain.MaxOffset, ((COrder*)pParam)->AreaCheckRun.DotTrain.WhiteOrBlack, ((COrder*)pParam)->AreaCheckRun.DotTrain.Threshold, ((COrder*)pParam)->AreaCheckRun.DotTrain.PointData.size());
					}
#endif
					if (DotPosition != NULL)
						delete DotPosition;//釋放記憶體
				}
				//線訓練
				if (((COrder*)pParam)->AreaCheckRun.LineTrain.PointData.size())
				{
					LineSwitch = TRUE;
					LinePosition = new CPoint[((COrder*)pParam)->AreaCheckRun.LineTrain.PointData.size()];
					for (UINT i = 0; i < ((COrder*)pParam)->AreaCheckRun.LineTrain.PointData.size(); i++)
					{
						LinePosition[i] = ((COrder*)pParam)->AreaCheckRun.LineTrain.PointData.at(i);
					}
#ifdef VI
					if (FilePathExist(((COrder*)pParam)->AreaCheckRun.LineTrain.TrainSave.Path))//判斷檔案路徑是否存在
					{
						VI_ImagesLineBeadTrain(((COrder*)pParam)->AreaCheckRun.LineTrain.TrainSave.Path, ((COrder*)pParam)->AreaCheckRun.LineTrain.TrainSave.Name,
							LinePosition, ((COrder*)pParam)->AreaCheckRun.LineTrain.MeasureLimit, ((COrder*)pParam)->AreaCheckRun.LineTrain.MaxOffset, ((COrder*)pParam)->AreaCheckRun.LineTrain.WhiteOrBlack, ((COrder*)pParam)->AreaCheckRun.LineTrain.PointData.size());
					}
#endif
					if (LinePosition != NULL)
						delete LinePosition;//釋放記憶體
				}
				//判斷檢測模式
				if (DotSwitch && LineSwitch)
					Mode = 3;
				else if (LineSwitch)
					Mode = 2;
				else if (DotSwitch)
					Mode = 1;
				//檢測
#ifdef VI
				if (FilePathExist(((COrder*)pParam)->AreaCheckRun.Result.Path))//判斷檔案路徑是否存在
				{
					((COrder*)pParam)->AreaCheckRun.Result.Name = GetDataFileName();
					((COrder*)pParam)->AreaCheckFinishRecord.push_back({ ((COrder*)pParam)->AreaCheckRun.Address ,
						((COrder*)pParam)->AreaCheckRun.Result,
						VI_AreaBeadVerify(((COrder*)pParam)->AreaCheckRun.Image.ImageSave.Path, ((COrder*)pParam)->AreaCheckRun.Image.ImageSave.Name,
						((COrder*)pParam)->AreaCheckRun.DotTrain.TrainSave.Path, ((COrder*)pParam)->AreaCheckRun.DotTrain.TrainSave.Name,
							((COrder*)pParam)->AreaCheckRun.LineTrain.TrainSave.Path, ((COrder*)pParam)->AreaCheckRun.LineTrain.TrainSave.Name,
							((COrder*)pParam)->AreaCheckRun.Result.Path, ((COrder*)pParam)->AreaCheckRun.Result.Name, Mode)
					});//儲存檢測結果
				}
#endif
			}
			else if (((COrder*)pParam)->RunStatusRead.MosaicStatus == 0)
			{
				AfxMessageBox(L"Error:重組失敗!");
			}
		}
	}
	((COrder*)pParam)->CheckModel = 0;//執行完後檢測模式改為不執行檢測
	g_pCheckActionThread = NULL;
	::SetEvent(((COrder*)pParam)->ThreadEvent.CheckActionThread);
	return 0;
}
//重組中對話框執行緒
UINT COrder::MosaicDlg(LPVOID pParam)
{
	//判斷是否開啟重組
	if (((COrder*)pParam)->RunStatusRead.MosaicStatus == -1)
	{
		((COrder*)pParam)->AreaCheckParamterDefault.pMosaicDlg->DoModal();
	}
	g_pMosaicDlgThread = NULL;
	::SetEvent(((COrder*)pParam)->ThreadEvent.MosaicDlgThread);
	return 0;
}
/**************************************************************************動作處理區塊*************************************************************************/
/*運動狀態判斷*/
void COrder::LineGotoActionJudge(LPVOID pParam)
{
	if (((COrder*)pParam)->ModelControl.Mode == 3 || ((COrder*)pParam)->ModelControl.Mode == 4)
	{
		if (((COrder*)pParam)->RunData.ActionStatus.at(((COrder*)pParam)->Program.SubroutinCount) == 1)//LS時 移動至LS虛擬點
		{
#ifdef MOVE
			((COrder*)pParam)->m_Action.DecideVirtualPoint(((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Y, ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Z, ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).W,
				((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, ((COrder*)pParam)->LineSpeedSet.InitSpeed);
#endif
			((COrder*)pParam)->VirtualCoordinateData = ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount);//紀錄移動虛擬座標
		}
		else if (((COrder*)pParam)->RunData.ActionStatus.at(((COrder*)pParam)->Program.SubroutinCount) == 2)//LP時 斷膠抬升
		{
#ifdef MOVE
			((COrder*)pParam)->m_Action.DecideLineEndMove(((COrder*)pParam)->DispenseLineSet.StayTime, ((COrder*)pParam)->DispenseLineSet.ShutdownDelay,
				((COrder*)pParam)->ZSet.ZBackHeight, ((COrder*)pParam)->ZSet.ZBackType, ((COrder*)pParam)->DispenseLineEnd.HighSpeed, ((COrder*)pParam)->DispenseLineEnd.Width, ((COrder*)pParam)->DispenseLineEnd.Height, ((COrder*)pParam)->DispenseLineEnd.LowSpeed,
				((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, ((COrder*)pParam)->LineSpeedSet.InitSpeed);
#endif
			if (((COrder*)pParam)->CheckSwitch.Area)//區域區間檢測線段新增
			{
				for (UINT i = 0; i < ((COrder*)pParam)->IntervalAreaCheck.size(); i++)//判斷目前使用的區間 加入線段陣列
				{
					if (((COrder*)pParam)->IntervalAreaCheck.at(i).Address == ((COrder*)pParam)->CurrentCheckAddress &&
						((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData.size())
					{
						((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData.push_back({ -1,-1 });//換線點
					}
				}
			}
		}
	} 
	if (((COrder*)pParam)->ModelControl.Mode == 2)
	{
		//插入狀態下
		if (((COrder*)pParam)->RunData.ActionStatus.at(((COrder*)pParam)->Program.SubroutinCount) == 2)//LS存在執行過LP
		{
#ifdef MOVE
			//呼叫執行掃描
			((COrder*)pParam)->m_Action.LA_Line2D(((COrder*)pParam)->LMPSpeedSet.EndSpeed, ((COrder*)pParam)->LMPSpeedSet.AccSpeed, ((COrder*)pParam)->LMPSpeedSet.InitSpeed,
				((COrder*)pParam)->LMCSpeedSet.EndSpeed, ((COrder*)pParam)->LMCSpeedSet.AccSpeed, ((COrder*)pParam)->LMCSpeedSet.InitSpeed);
#endif
			//紀錄測量高度至雷射修正表
			((COrder*)pParam)->LaserContinuousControl.ContinuousLineCount--;
			((COrder*)pParam)->LaserCount++;
			((COrder*)pParam)->LaserAdjust.push_back({ ((COrder*)pParam)->LaserContinuousControl.ContinuousLineCount });
			/*修改紀錄修正表*/
			UINT D1 = 0, D2 = 0;
			((COrder*)pParam)->GetHashAddress(((COrder*)pParam)->CurrentTableAddress, D1, D2);//獲取雜湊表地址
			for (UINT i = 0; i < ((COrder*)pParam)->PositionModifyNumber.at(D1).at(D2).size(); i++)//判斷表中地址是否存在
			{
				if (((COrder*)pParam)->PositionModifyNumber.at(D1).at(D2).at(i).Address == ((COrder*)pParam)->CurrentTableAddress)
				{
					((COrder*)pParam)->PositionModifyNumber.at(D1).at(D2).at(i).LaserNumber = ((COrder*)pParam)->LaserCount;
					break;
				}
			}
#ifdef  PRINTF
			_cwprintf(_T("LineGotoActionJudge()::執行完特殊線段雷射掃描(LP接點)\n"));
#endif
		}
	}
	//狀態清除
	((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Status = FALSE;
	((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Status = FALSE;
	((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Status = FALSE;
	((COrder*)pParam)->RunData.ActionStatus.at(((COrder*)pParam)->Program.SubroutinCount) = 0;
}
/*CallSubroutin修正處理*/
void COrder::ModifyPointOffSet(LPVOID pParam ,CString Command)
{
	if (!((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Status)
	{
		//舊版CallSubroutine 修正
		/*if (((COrder*)pParam)->ModelControl.Mode == 1 || ((COrder*)pParam)->ModelControl.Mode == 2)
		{
		XYZOffset = ((COrder*)pParam)->VirtualNowOffSet(pParam, Command);
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 3)
		{
		XYZOffset = ((COrder*)pParam)->m_Action.NowOffSet(_ttol(CommandResolve(Command, 1)), _ttol(CommandResolve(Command, 2)), _ttol(CommandResolve(Command, 3)));
		}*/

		CString XYZOffset;
		XYZOffset = ((COrder*)pParam)->VirtualNowOffSet(pParam, Command);//計算offset
		((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X = ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X + _ttol(CommandResolve(XYZOffset, 0));
		((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y = ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y + _ttol(CommandResolve(XYZOffset, 1));
		((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z = ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z + _ttol(CommandResolve(XYZOffset, 2));
		//將Switch 轉換成已修正
		((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Status = TRUE;
		//修正完畢斷膠抬升(或直接抬升)
#ifdef MOVE
		if (((COrder*)pParam)->ModelControl.Mode != 1 && ((COrder*)pParam)->ModelControl.Mode != 2)//在影像、雷射模式下不做回升
		{
			((COrder*)pParam)->m_Action.DecideLineEndMove(((COrder*)pParam)->DispenseLineSet.StayTime, ((COrder*)pParam)->DispenseLineSet.ShutdownDelay,
			((COrder*)pParam)->ZSet.ZBackHeight, ((COrder*)pParam)->ZSet.ZBackType,
				((COrder*)pParam)->DispenseLineEnd.HighSpeed, ((COrder*)pParam)->DispenseLineEnd.Width, ((COrder*)pParam)->DispenseLineEnd.Height, ((COrder*)pParam)->DispenseLineEnd.LowSpeed,
				((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, ((COrder*)pParam)->LineSpeedSet.InitSpeed,1);
		}
#endif
		//當有CallSubroutine新增StepRepeat時加入StepRepeatInintOffset
		if (((COrder*)pParam)->RepeatData.StepRepeatNum.size() > ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.size())
		{
			//紀錄初始offset位置
			((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.push_back(((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X);
			((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.push_back(((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y);
		}
	} 
}
/*虛擬座標計算偏差值*/
CString COrder::VirtualNowOffSet(LPVOID pParam, CString Command)
{
	LONG lNowX = 0, lNowY = 0, lNowZ = 0;
	CString csBuff = 0;
	/*統一使未修正的XYZ值做CallSubroutine計算相對量位移*/
	lNowX = ((COrder*)pParam)->NVMVirtualCoordinateData.X;
	lNowY = ((COrder*)pParam)->NVMVirtualCoordinateData.Y;
	lNowZ = ((COrder*)pParam)->NVMVirtualCoordinateData.Z;
	csBuff.Format(_T("%ld,%ld,%ld"), (lNowX - _ttol(CommandResolve(Command, 1))), (lNowY - _ttol(CommandResolve(Command, 2))), (lNowZ - _ttol(CommandResolve(Command, 3))));
#ifdef PRINTF
	_cwprintf(_T("VirtualNowOffSet()::子程序偏差值:%ld,%ld,%ld\n"), (lNowX - _ttol(CommandResolve(Command, 1))), (lNowY - _ttol(CommandResolve(Command, 2))), (lNowZ - _ttol(CommandResolve(Command, 3))));
#endif
	((COrder*)pParam)->NVMVirtualCoordinateData = { 0,0,0,0,0 };//計算完畢後數值規0
	return csBuff;
}
/*影像修正*/
void COrder::VisionModify(LPVOID pParam)
{
	/*之後測試是否有問題，修正完畢後是否清除影像值*/
#ifdef VI 
	if (((COrder*)pParam)->Program.SubroutineStack.empty())//沒有CallSubroutine時修正方式
	{
		VI_CorrectLocation(((COrder*)pParam)->FinalWorkCoordinateData.X, ((COrder*)pParam)->FinalWorkCoordinateData.Y,
			((COrder*)pParam)->VisionOffset.Contraposition.X, ((COrder*)pParam)->VisionOffset.Contraposition.Y,
			((COrder*)pParam)->VisionOffset.OffsetX, ((COrder*)pParam)->VisionOffset.OffsetY, ((COrder*)pParam)->VisionOffset.Angle, ((COrder*)pParam)->VisionSet.ModifyMode);
		((COrder*)pParam)->VisioModifyJudge = TRUE;
	}
	else
	{
		if (((COrder*)pParam)->Program.SubroutineVisioModifyJudge.at((((COrder*)pParam)->Program.SubroutinCount-1)))//判斷Subroutine是否執行修正
		{
			VI_CorrectLocation(((COrder*)pParam)->FinalWorkCoordinateData.X, ((COrder*)pParam)->FinalWorkCoordinateData.Y,
				((COrder*)pParam)->VisionOffset.Contraposition.X, ((COrder*)pParam)->VisionOffset.Contraposition.Y,
				((COrder*)pParam)->VisionOffset.OffsetX, ((COrder*)pParam)->VisionOffset.OffsetY, ((COrder*)pParam)->VisionOffset.Angle, ((COrder*)pParam)->VisionSet.ModifyMode);
#ifdef PRINTF
			_cwprintf(_T("VisionModify()::CallSubroutine進入修正!\n"));
#endif 
		}
	}
#endif
}
/*影像未找到處理方法*/
void COrder::VisionFindMarkError(LPVOID pParam)
{
	switch (((COrder*)pParam)->VisionSerchError.SearchError)
	{
	case 1://略過或Trigger
#ifdef VI
		//沒找到抬升
		#ifdef MOVE
		//對位完畢不出膠回升
		((COrder*)pParam)->m_Action.DecideLineEndMove(0, 0,
			((COrder*)pParam)->ZSet.ZBackHeight, ((COrder*)pParam)->ZSet.ZBackType, 0, 0, 0, 0,
			((COrder*)pParam)->DotSpeedSet.EndSpeed, ((COrder*)pParam)->DotSpeedSet.AccSpeed, ((COrder*)pParam)->DotSpeedSet.InitSpeed,1);
		#endif
		if (((COrder*)pParam)->VisionTrigger.AdjustStatus == 1 || ((COrder*)pParam)->VisionTrigger.AdjustStatus == 2)
		{
			if (((COrder*)pParam)->VisionTrigger.Trigger1.size())//有Trigger時
			{
				for (UINT i = 0; i < ((COrder*)pParam)->VisionTrigger.Trigger1.size(); i++)
				{
					#ifdef MOVE
					((COrder*)pParam)->m_Action.DoCCDMove(
						((COrder*)pParam)->VisionTrigger.Trigger1.at(i).X,
						((COrder*)pParam)->VisionTrigger.Trigger1.at(i).Y,
						((COrder*)pParam)->VisionTrigger.Trigger1.at(i).Z,
						((COrder*)pParam)->DotSpeedSet.EndSpeed, ((COrder*)pParam)->DotSpeedSet.AccSpeed, ((COrder*)pParam)->DotSpeedSet.InitSpeed);
					#endif
					if (((COrder*)pParam)->VisionTrigger.AdjustStatus == 1)
					{
						if (VI_CameraTrigger(((COrder*)pParam)->FindMark.MilModel, ((COrder*)pParam)->FindMark.Point.X, ((COrder*)pParam)->FindMark.Point.Y, ((COrder*)pParam)->VisionTrigger.Trigger1.at(i).X, ((COrder*)pParam)->VisionTrigger.Trigger1.at(i).Y, ((COrder*)pParam)->VisionOffset.OffsetX, ((COrder*)pParam)->VisionOffset.OffsetY))
						{
						#ifdef MOVE
						   //對位完畢不出膠回升
							((COrder*)pParam)->m_Action.DecideLineEndMove(0, 0,
								((COrder*)pParam)->ZSet.ZBackHeight, ((COrder*)pParam)->ZSet.ZBackType, 0, 0, 0, 0,
								((COrder*)pParam)->DotSpeedSet.EndSpeed, ((COrder*)pParam)->DotSpeedSet.AccSpeed, ((COrder*)pParam)->DotSpeedSet.InitSpeed,1);
						#endif
							//紀錄影像Offset至影像修正表 
							((COrder*)pParam)->VisionCount++;
							((COrder*)pParam)->VisionAdjust.push_back({ ((COrder*)pParam)->VisionOffset });
#ifdef PRINTF
							_cwprintf(L"VisionFindMarkError()::影像定位完畢成功寫入數值VisionCount:%d,VisionAdjust:%d,%d\n", ((COrder*)pParam)->VisionCount, ((COrder*)pParam)->VisionAdjust.back().VisionOffset.OffsetX, ((COrder*)pParam)->VisionAdjust.back().VisionOffset.OffsetY);
#endif
							//影像釋放記憶體
							if (*(int*)((COrder*)pParam)->FindMark.MilModel != 0)
							{
								VI_ModelFree(((COrder*)pParam)->FindMark.MilModel);
							}
							//影像記憶體初始化
							*(int*)((COrder*)pParam)->FindMark.MilModel = 0;
							//清除對位Offset資料
							((COrder*)pParam)->VisionOffset = { { 0,0,0,0 },0,0,0 };
							break;
						}
					}
					else
					{
						((COrder*)pParam)->FiducialMark1.FindMarkStatus = TRUE;
						if (VI_CameraTrigger(((COrder*)pParam)->FiducialMark1.MilModel, ((COrder*)pParam)->FiducialMark1.Point.X, ((COrder*)pParam)->FiducialMark1.Point.Y, ((COrder*)pParam)->VisionTrigger.Trigger1.at(i).X, ((COrder*)pParam)->VisionTrigger.Trigger1.at(i).Y, ((COrder*)pParam)->FiducialMark1.OffsetX, ((COrder*)pParam)->FiducialMark1.OffsetY))
						{
						#ifdef MOVE
							//對位完畢不出膠回升
							((COrder*)pParam)->m_Action.DecideLineEndMove(0, 0,
								((COrder*)pParam)->ZSet.ZBackHeight, ((COrder*)pParam)->ZSet.ZBackType, 0, 0, 0, 0,
								((COrder*)pParam)->DotSpeedSet.EndSpeed, ((COrder*)pParam)->DotSpeedSet.AccSpeed, ((COrder*)pParam)->DotSpeedSet.InitSpeed,1);
						#endif
							break;
						}
					}
				}
			}
			else
			{
				((COrder*)pParam)->FiducialMark1.FindMarkStatus = TRUE;
			}
		}
		else if (((COrder*)pParam)->VisionTrigger.AdjustStatus == 3)
		{
			if (((COrder*)pParam)->VisionTrigger.Trigger2.size())//有Trigger時
			{
				for (UINT i = 0; i < ((COrder*)pParam)->VisionTrigger.Trigger2.size(); i++)
				{
#ifdef MOVE
					((COrder*)pParam)->m_Action.DecideVirtualPoint(
						((COrder*)pParam)->VisionTrigger.Trigger2.at(i).X,
						((COrder*)pParam)->VisionTrigger.Trigger2.at(i).Y,
						((COrder*)pParam)->VisionTrigger.Trigger2.at(i).Z,
						0,
						((COrder*)pParam)->DotSpeedSet.EndSpeed, ((COrder*)pParam)->DotSpeedSet.AccSpeed, ((COrder*)pParam)->DotSpeedSet.InitSpeed);
#endif
					if (VI_CameraTrigger(((COrder*)pParam)->FiducialMark2.MilModel, ((COrder*)pParam)->FiducialMark2.Point.X, ((COrder*)pParam)->FiducialMark2.Point.Y, ((COrder*)pParam)->VisionTrigger.Trigger2.at(i).X, ((COrder*)pParam)->VisionTrigger.Trigger2.at(i).Y, ((COrder*)pParam)->FiducialMark2.OffsetX, ((COrder*)pParam)->FiducialMark2.OffsetY))
					{
					#ifdef MOVE
						//對位完畢不出膠回升
						((COrder*)pParam)->m_Action.DecideLineEndMove(0, 0,
							((COrder*)pParam)->ZSet.ZBackHeight, ((COrder*)pParam)->ZSet.ZBackType, 0, 0, 0, 0,
							((COrder*)pParam)->DotSpeedSet.EndSpeed, ((COrder*)pParam)->DotSpeedSet.AccSpeed, ((COrder*)pParam)->DotSpeedSet.InitSpeed,1);
					#endif
						break;
					}
				}
			}
			((COrder*)pParam)->FiducialMark2.FindMarkStatus = TRUE;
		}
		((COrder*)pParam)->VisionSerchError.SearchError = ((COrder*)pParam)->VisionDefault.VisionSerchError.SearchError;//參數回歸
#ifdef PRINTF
		_cwprintf(L"VisionFindMarkError()::Tirrger運行完畢\n");
#endif
		break;
#endif  
	case 2://停止
		((COrder*)pParam)->Stop();
		break;
	case 3://暫停
		//無找到抬升
		#ifdef MOVE
		//對位完畢不出膠回升
		((COrder*)pParam)->m_Action.DecideLineEndMove(0, 0,
			((COrder*)pParam)->ZSet.ZBackHeight, ((COrder*)pParam)->ZSet.ZBackType, 0, 0, 0, 0,
			((COrder*)pParam)->DotSpeedSet.EndSpeed, ((COrder*)pParam)->DotSpeedSet.AccSpeed, ((COrder*)pParam)->DotSpeedSet.InitSpeed,1);
		#endif
		((COrder*)pParam)->Pause(); 
		if (((COrder*)pParam)->VisionTrigger.AdjustStatus == 2 || ((COrder*)pParam)->VisionTrigger.AdjustStatus == 3)
		{
			((COrder*)pParam)->VisionSerchError.Pausemode = TRUE;
			((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount))--;//重複一次命令
		}
		break;
	case 4://跳出選擇對話窗
		if (((COrder*)pParam)->VisionSerchError.pQuestion != NULL)
		{
			switch (((COrder*)pParam)->VisionSerchError.pQuestion->DoModal())
			{
			case 0://略過或Trigger
				((COrder*)pParam)->VisionSerchError.SearchError = 1;
				VisionFindMarkError(pParam);
				break;
			case 1://停止
				((COrder*)pParam)->Stop();
				break;
			case 2://重新尋找
				((COrder*)pParam)->FiducialMark1.FindMarkStatus = FALSE;
				((COrder*)pParam)->FiducialMark2.FindMarkStatus = FALSE;
				((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount))--;
				break;
			case 3://手動
				((COrder*)pParam)->VisionSerchError.SearchError = 5;
				VisionFindMarkError(pParam);
				break;
			case 4:
				break;
			default:
				break;
			}
		}
		break;
	case 5://手動模式
		((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount))--;
		((COrder*)pParam)->Pause();
		((COrder*)pParam)->VisionSerchError.Manuallymode = TRUE;
		((COrder*)pParam)->VisionSerchError.SearchError = ((COrder*)pParam)->VisionDefault.VisionSerchError.SearchError;//參數回歸
		break;
	case 6:
		break;
	default:
		break;
	}
}
/*雷射修正*/
void COrder::LaserModify(LPVOID pParam) 
{
	if (((COrder*)pParam)->LaserData.LaserMeasureHeight != -999999999)//如果值不等於-999999999執行修正
	{
#ifdef PRINTF
		_cwprintf(L"LaserModify()::FinalWorkCoordinateData.Z = %d\n", ((COrder*)pParam)->LaserData.LaserMeasureHeight);
#endif 
		((COrder*)pParam)->FinalWorkCoordinateData.Z = ((COrder*)pParam)->LaserData.LaserMeasureHeight;
		if (((COrder*)pParam)->FinalWorkCoordinateData.Z <= -10000)
		{
			((COrder*)pParam)->LaserContinuousControl.ContinuousSwitch = TRUE;
#ifdef PRINTF
			_cwprintf(L"LaserModify()::選擇線段:%d\n", abs(((COrder*)pParam)->FinalWorkCoordinateData.Z + 10000));
#endif  
		}
		((COrder*)pParam)->LaserData.LaserMeasureHeight = -999999999;//修正完畢後清除值
	} 
}
/*雷射檢測處理*/
void COrder::LaserDetectHandle(LPVOID pParam, CString Command)
{
	CString CommandBuff = _T("");
	if (!((COrder*)pParam)->LaserSwitch.LaserSkip && !((COrder*)pParam)->Program.SubroutineModelControlSwitch && !((COrder*)pParam)->LaserPointDetect())//雷射跳過未開啟
	{
		if ((((COrder*)pParam)->LaserSwitch.LaserDetect && ((COrder*)pParam)->LaserSwitch.LaserPointAdjust) || 
			(((COrder*)pParam)->LaserSwitch.LaserHeight && ((COrder*)pParam)->LaserSwitch.LaserPointAdjust))//模式一、四
		{
			RecordCorrectionTable(pParam);//寫入修正表
		}
		else if (((COrder*)pParam)->LaserSwitch.LaserHeight && ((COrder*)pParam)->LaserSwitch.LaserAdjust)//模式二
		{
			CommandBuff = Command;//暫存原有的命令
			ChooseVisionModify(pParam);//選擇影像Offset  
			if (CommandResolve(Command, 0) == L"FillArea")
			{
				Command = CommandResolve(Command, 0) + L"," + CommandResolve(Command, 4) + L"," + CommandResolve(Command, 5) + L"," + CommandResolve(Command, 6);
			}
			ModifyPointOffSet(pParam, Command);//CallSubutine用
			((COrder*)pParam)->FinalWorkCoordinateData.X = _ttoi(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X;
			((COrder*)pParam)->FinalWorkCoordinateData.Y = _ttoi(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y;
			((COrder*)pParam)->FinalWorkCoordinateData.Z = _ttoi(CommandResolve(Command, 3)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z;
			
			//影像修正選擇
			if (CommandResolve(Command, 0) == L"VirtualPoint" || CommandResolve(Command, 0) == L"StopPoint")
			{
				if (_ttol(CommandResolve(Command, 4)) == 1)
				{
					VisionModify(pParam);//影像修正
				}
			}
			else if (CommandResolve(Command, 0) == L"WaitPoint")
			{
				if (_ttol(CommandResolve(Command, 5)) == 1)
				{
					VisionModify(pParam);//影像修正
				}
			}
			else
			{
				VisionModify(pParam);//影像修正
			}
#ifdef LA
			//雷射運動掃描
			((COrder*)pParam)->m_Action.LA_Dot3D(
				((COrder*)pParam)->FinalWorkCoordinateData.X,
				((COrder*)pParam)->FinalWorkCoordinateData.Y,
				((COrder*)pParam)->LaserData.LaserMeasureHeight,
				((COrder*)pParam)->LMPSpeedSet.EndSpeed, ((COrder*)pParam)->LMPSpeedSet.AccSpeed, ((COrder*)pParam)->LMPSpeedSet.InitSpeed);
			//紀錄測量高度至雷射修正表
#endif
			Command = CommandBuff;//還原原有的命令
			((COrder*)pParam)->LaserCount++;
			((COrder*)pParam)->LaserAdjust.push_back({ ((COrder*)pParam)->LaserData.LaserMeasureHeight });
			RecordCorrectionTable(pParam);//寫入修正表
		}
		else if (((COrder*)pParam)->LaserSwitch.LaserDetect && ((COrder*)pParam)->LaserSwitch.LaserAdjust)//模式三
		{
			CommandBuff = Command;//暫存原有的命令
			ChooseVisionModify(pParam);//選擇影像Offset  
			if (CommandResolve(Command, 0) == L"FillArea")
			{
				Command = CommandResolve(Command, 0) + L"," + CommandResolve(Command, 4) + L"," + CommandResolve(Command, 5) + L"," + CommandResolve(Command, 6);
			}
			ModifyPointOffSet(pParam, Command);//CallSubutine用
			((COrder*)pParam)->FinalWorkCoordinateData.X = _ttoi(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X;
			((COrder*)pParam)->FinalWorkCoordinateData.Y = _ttoi(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y;
			((COrder*)pParam)->FinalWorkCoordinateData.Z = _ttoi(CommandResolve(Command, 3)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z;

			if (CommandResolve(Command, 0) == L"Dot" || CommandResolve(Command, 0) == L"VirtualPoint" ||
				CommandResolve(Command, 0) == L"StopPoint" || CommandResolve(Command, 0) == L"WaitPoint" ||
				CommandResolve(Command, 0) == L"FillArea")
			{
				LineGotoActionJudge(pParam);//判斷動作狀態
				if (CommandResolve(Command, 0) == L"VirtualPoint" || CommandResolve(Command, 0) == L"StopPoint")
				{
					if (_ttol(CommandResolve(Command, 4)) == 1)
					{
						VisionModify(pParam);//影像修正
					}
				}
				else if (CommandResolve(Command, 0) == L"WaitPoint")
				{
					if (_ttol(CommandResolve(Command, 5)) == 1)
					{
						VisionModify(pParam);//影像修正
					}
				}
#ifdef LA
				//雷射運動掃描
				((COrder*)pParam)->m_Action.LA_Dot3D(
					((COrder*)pParam)->FinalWorkCoordinateData.X,
					((COrder*)pParam)->FinalWorkCoordinateData.Y,
					((COrder*)pParam)->LaserData.LaserMeasureHeight,
					((COrder*)pParam)->LMPSpeedSet.EndSpeed, ((COrder*)pParam)->LMPSpeedSet.AccSpeed, ((COrder*)pParam)->LMPSpeedSet.InitSpeed);
				//紀錄測量高度至雷射修正表
#endif
				Command = CommandBuff;//還原原有的命令
				((COrder*)pParam)->LaserCount++;
				((COrder*)pParam)->LaserAdjust.push_back({ ((COrder*)pParam)->LaserData.LaserMeasureHeight });
			}
			else if (CommandResolve(Command, 0) == L"LineStart")
			{
				LineGotoActionJudge(pParam);//判斷動作狀態
				VisionModify(pParam);//影像修正
				((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Status = TRUE;
				((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).X = ((COrder*)pParam)->FinalWorkCoordinateData.X;
				((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Y = ((COrder*)pParam)->FinalWorkCoordinateData.Y;
				((COrder*)pParam)->RunData.ActionStatus.at(((COrder*)pParam)->Program.SubroutinCount) = 1;
			}
			else if (CommandResolve(Command, 0) == L"LinePassing")
			{
				if (((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Status)
				{
					VisionModify(pParam);//影像修正
					if (((COrder*)pParam)->RunData.ActionStatus.at(((COrder*)pParam)->Program.SubroutinCount) == 1)//LS存在尚未執行過LP
					{
						if (((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Status)//執行圓弧
						{
#ifdef LA
							//呼叫LS
							((COrder*)pParam)->m_Action.LA_Do2dDataLine(((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Y);
							//呼叫ARC-LP測高(API 點順序倒著放)
							((COrder*)pParam)->m_Action.LA_Do2dDataArc(((COrder*)pParam)->FinalWorkCoordinateData.X, ((COrder*)pParam)->FinalWorkCoordinateData.Y,
								((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Y);
							//清除完成動作弧
#endif
							((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Status = FALSE;

						}
						else if (((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Status)//執行圓
						{
#ifdef LA
							//呼叫LS
							((COrder*)pParam)->m_Action.LA_Do2dDataLine(((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Y);
							//呼叫Circle-LP(API 點順序倒著放)
							((COrder*)pParam)->m_Action.LA_Do2dDataCircle(((COrder*)pParam)->FinalWorkCoordinateData.X, ((COrder*)pParam)->FinalWorkCoordinateData.Y,
								((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Y,
								((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount).Y);
							//清除完成動作圓
#endif
							((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Status = FALSE;
						}
						else//執行線段
						{
#ifdef LA
							//呼叫LS
							((COrder*)pParam)->m_Action.LA_Do2dDataLine(((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Y);
							//呼叫LP
							((COrder*)pParam)->m_Action.LA_Do2dDataLine(((COrder*)pParam)->FinalWorkCoordinateData.X, ((COrder*)pParam)->FinalWorkCoordinateData.Y);
#endif
						}
					}
					else if (((COrder*)pParam)->RunData.ActionStatus.at(((COrder*)pParam)->Program.SubroutinCount) == 2)//LS存在執行過LP
					{
						if (((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Status)//執行圓弧
						{
#ifdef LA
							//呼叫Now-ARC-LP (API 點順序倒著放)
							((COrder*)pParam)->m_Action.LA_Do2dDataArc(((COrder*)pParam)->FinalWorkCoordinateData.X, ((COrder*)pParam)->FinalWorkCoordinateData.Y,
								((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Y);
#endif
							//清除完成動作弧
							((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Status = FALSE;
						}
						else if (((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Status)//執行圓
						{
#ifdef LA
							//呼叫Now-Circle-LP
							((COrder*)pParam)->m_Action.LA_Do2dDataCircle(((COrder*)pParam)->FinalWorkCoordinateData.X, ((COrder*)pParam)->FinalWorkCoordinateData.Y,
								((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Y,
								((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount).Y);
#endif
							//清除完成動作圓
							((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Status = FALSE;
						}
						else//執行線段
						{
#ifdef LA
							//呼叫LP
							((COrder*)pParam)->m_Action.LA_Do2dDataLine(((COrder*)pParam)->FinalWorkCoordinateData.X, ((COrder*)pParam)->FinalWorkCoordinateData.Y);
#endif
						}
					}
					//紀錄雷射表
					((COrder*)pParam)->RunData.ActionStatus.at(((COrder*)pParam)->Program.SubroutinCount) = 2;//狀態變為線段中
					((COrder*)pParam)->LaserCount++;
					((COrder*)pParam)->LaserAdjust.push_back({ -10000 });
				}   
			}
			else if (CommandResolve(Command, 0) == L"LineEnd")
			{
				if (((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Status)
				{
					VisionModify(pParam);//影像修正
					if (((COrder*)pParam)->RunData.ActionStatus.at(((COrder*)pParam)->Program.SubroutinCount) == 1)//LS存在且尚未執行過
					{
						if (((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Status)//執行圓弧
						{
#ifdef LA
							//呼叫LS
							((COrder*)pParam)->m_Action.LA_Do2dDataLine(((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Y);
							//呼叫ARC-LP測高(API 點順序倒著放)
							((COrder*)pParam)->m_Action.LA_Do2dDataArc(((COrder*)pParam)->FinalWorkCoordinateData.X, ((COrder*)pParam)->FinalWorkCoordinateData.Y,
								((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Y);   
#endif
							//清除完成動作弧
							((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Status = FALSE;
						}
						else if (((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Status)//執行圓
						{
#ifdef LA
							//呼叫LS
							((COrder*)pParam)->m_Action.LA_Do2dDataLine(((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Y);
							//呼叫Circle-LE
							((COrder*)pParam)->m_Action.LA_Do2dDataCircle(((COrder*)pParam)->FinalWorkCoordinateData.X, ((COrder*)pParam)->FinalWorkCoordinateData.Y,
								((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Y,
								((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount).Y);
#endif
							//清除完成動作圓
							((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Status = FALSE;
						}
						else//執行線段
						{
#ifdef LA
							//呼叫LS
							((COrder*)pParam)->m_Action.LA_Do2dDataLine(((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Y);
							//呼叫LE
							((COrder*)pParam)->m_Action.LA_Do2dDataLine(((COrder*)pParam)->FinalWorkCoordinateData.X, ((COrder*)pParam)->FinalWorkCoordinateData.Y);
#endif
						}
					}
					else if (((COrder*)pParam)->RunData.ActionStatus.at(((COrder*)pParam)->Program.SubroutinCount) == 2)////LS存在執行過LP
					{
						if (((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Status)//執行圓弧
						{
#ifdef LA
							//呼叫Now-ARC-LE (API 點順序倒著放)
							((COrder*)pParam)->m_Action.LA_Do2dDataArc(((COrder*)pParam)->FinalWorkCoordinateData.X, ((COrder*)pParam)->FinalWorkCoordinateData.Y,
								((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Y);
#endif
							//清除完成動作弧
							((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Status = FALSE;
						}
						else if (((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Status)//執行圓
						{
#ifdef LA
							//呼叫Now-Circle-LE
							((COrder*)pParam)->m_Action.LA_Do2dDataCircle(((COrder*)pParam)->FinalWorkCoordinateData.X, ((COrder*)pParam)->FinalWorkCoordinateData.Y,
								((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Y,
								((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount).Y);
#endif
							//清除完成動作圓
							((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Status = FALSE;
						}
						else//執行線段
						{
#ifdef LA
							//呼叫LE
							((COrder*)pParam)->m_Action.LA_Do2dDataLine(((COrder*)pParam)->FinalWorkCoordinateData.X, ((COrder*)pParam)->FinalWorkCoordinateData.Y);
#endif
						}
					}
					//呼叫執行掃描
					((COrder*)pParam)->m_Action.LA_Line2D(((COrder*)pParam)->LMPSpeedSet.EndSpeed, ((COrder*)pParam)->LMPSpeedSet.AccSpeed, ((COrder*)pParam)->LMPSpeedSet.InitSpeed,
						((COrder*)pParam)->LMCSpeedSet.EndSpeed, ((COrder*)pParam)->LMCSpeedSet.AccSpeed, ((COrder*)pParam)->LMCSpeedSet.InitSpeed);
					//紀錄雷射表
					((COrder*)pParam)->LaserContinuousControl.ContinuousLineCount--;//加入一條連續線段
					((COrder*)pParam)->LaserCount++;
					((COrder*)pParam)->LaserAdjust.push_back({ ((COrder*)pParam)->LaserContinuousControl.ContinuousLineCount });
				}
				((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Status = FALSE;
				((COrder*)pParam)->RunData.ActionStatus.at(((COrder*)pParam)->Program.SubroutinCount) = 0;
			}
			RecordCorrectionTable(pParam);//寫入修正表
		}
#ifdef PRINTF
		_cwprintf(L"LaserDetectHandle()::雷射高度成功寫入雷射表\n");
#endif  
	}
	VirtualCoordinateMove(pParam, Command, 1);//虛擬座標移動
}
/*檢查雷射檢測點是否重複*/
BOOL COrder::LaserPointDetect()
{
	CString CurrentAddress = GetCommandAddress();
	UINT D1 = 0, D2 = 0;
	GetHashAddress(CurrentAddress, D1, D2);//獲取雜湊表地址
	for (UINT i = 0; i < PositionModifyNumber.at(D1).at(D2).size(); i++)
	{
		if (PositionModifyNumber.at(D1).at(D2).at(i).Address == GetCommandAddress())
		{
			if (PositionModifyNumber.at(D1).at(D2).at(i).LaserNumber != -1)
			{
				return TRUE;//該地址點檢查過
			}
		}
	}
	return FALSE;//該地址點未檢查
}
/*虛擬座標移動
*type:保留，目前未使用到
*/
void COrder::VirtualCoordinateMove(LPVOID pParam, CString Command ,LONG type)
{
	if (CommandResolve(Command, 0) == L"Dot" ||
		CommandResolve(Command, 0) == L"VirtualPoint" ||
		CommandResolve(Command, 0) == L"WaitPoint" ||
		CommandResolve(Command, 0) == L"StopPoint" ||
		CommandResolve(Command, 0) == L"LineStart")
	{
		ModifyPointOffSet(pParam, Command);//CallSubutine用 
		((COrder*)pParam)->FinalWorkCoordinateData.Status = FALSE;
		if (CommandResolve(Command, 0) == L"LineStart")
		{
			((COrder*)pParam)->FinalWorkCoordinateData.Status = TRUE;
		}
		((COrder*)pParam)->FinalWorkCoordinateData.X = _ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X;
		((COrder*)pParam)->FinalWorkCoordinateData.Y = _ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y;
		((COrder*)pParam)->FinalWorkCoordinateData.Z = _ttol(CommandResolve(Command, 3)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z;
		((COrder*)pParam)->FinalWorkCoordinateData.W = _tstof(CommandResolve(Command, 4)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).W;
		((COrder*)pParam)->NVMVirtualCoordinateData = ((COrder*)pParam)->FinalWorkCoordinateData;//紀錄CallSubroutine點(不加影像修正時的值)
	}
	else if (CommandResolve(Command, 0) == L"FillArea")
	{
		CString CommandBuff;
		CommandBuff.Format(_T("FillArea,%d,%d,%d"), _ttol(CommandResolve(Command, 4)), _ttol(CommandResolve(Command, 5)), _ttol(CommandResolve(Command, 6)));
		ModifyPointOffSet(pParam, CommandBuff);//CallSubroutin相對位修正
		((COrder*)pParam)->FinalWorkCoordinateData.Status = FALSE;//將線段取消
		//紀錄CallSubroutine點(不加影像修正時的值)
		((COrder*)pParam)->m_Action.Fill_EndPoint(((COrder*)pParam)->NVMVirtualCoordinateData.X, ((COrder*)pParam)->NVMVirtualCoordinateData.Y,
			_ttol(CommandResolve(Command, 4)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X,
			_ttol(CommandResolve(Command, 5)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
			_ttol(CommandResolve(Command, 6)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z,
			_ttol(CommandResolve(Command, 7)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X,
			_ttol(CommandResolve(Command, 8)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
			_ttol(CommandResolve(Command, 9)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z,
			_ttol(CommandResolve(Command, 1)), _ttol(CommandResolve(Command, 2)), _ttol(CommandResolve(Command, 3))
		);
		((COrder*)pParam)->NVMVirtualCoordinateData.Z = _ttol(CommandResolve(Command, 9)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z; //紀錄CallSubroutine點(不加影像修正時的值)
		((COrder*)pParam)->NVMVirtualCoordinateData.Status = ((COrder*)pParam)->FinalWorkCoordinateData.Status;
	}
	else if (CommandResolve(Command, 0) == L"LineEnd" ||
		CommandResolve(Command, 0) == L"LinePassing")
	{
		ModifyPointOffSet(pParam, Command);//CallSubutine用 
		if (((COrder*)pParam)->NVMVirtualCoordinateData.Status)//判斷是否有LS
		{ 
			((COrder*)pParam)->FinalWorkCoordinateData.X = _ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X;
			((COrder*)pParam)->FinalWorkCoordinateData.Y = _ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y;
			((COrder*)pParam)->FinalWorkCoordinateData.Z = _ttol(CommandResolve(Command, 3)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z;
			((COrder*)pParam)->FinalWorkCoordinateData.W = _tstof(CommandResolve(Command, 4)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).W;
			if (CommandResolve(Command, 0) == L"LineEnd")
			{
				((COrder*)pParam)->FinalWorkCoordinateData.Status = FALSE;
			}
			((COrder*)pParam)->NVMVirtualCoordinateData = ((COrder*)pParam)->FinalWorkCoordinateData;//紀錄CallSubroutine點(不加影像修正時的值)
		}
	}
}
/*檢測畫圖處理*/
BOOL COrder::CheckDraw()
{
	if (CallFunction.pObject)//判斷物件指針是否為空
	{
		int Pencolor = 0;
		if (CheckFinishRecord.back().Result == L"NG")
			Pencolor = 3;
		else if(CheckFinishRecord.back().Result == L"OK")
			Pencolor = 2;
		else if (CheckFinishRecord.back().Result == L"Err")
			Pencolor = 1;
		(*CallFunction.CDrawFunction)(CallFunction.pObject, { CheckFinishRecord.back().CheckData.Position.X,CheckFinishRecord.back().CheckData.Position.Y }, Pencolor);
		return 1;
	}   
	return 0;
}
/*中間點例外處理*/
void COrder::PassingException(LPVOID pParam)
{
	if (((COrder*)pParam)->AreaCheckChangTemp.Status && ((COrder*)pParam)->AreaCheckChangTemp.X != -1 && ((COrder*)pParam)->AreaCheckChangTemp.Y != -1 && ((COrder*)pParam)->AreaCheckChangTemp.Z != -1)
	{
#ifdef MOVE
		//呼叫LS 
		((COrder*)pParam)->m_Action.DecideLineStartMove(
			((COrder*)pParam)->AreaCheckChangTemp.X,
			((COrder*)pParam)->AreaCheckChangTemp.Y,
			((COrder*)pParam)->AreaCheckChangTemp.Z,
			((COrder*)pParam)->AreaCheckChangTemp.W,
			0, ((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, ((COrder*)pParam)->LineSpeedSet.InitSpeed);
#endif
		((COrder*)pParam)->AreaCheckChangTemp.Status = FALSE;
	}
#ifdef PRINTF
	_cwprintf(L"PassingException()::座標(%d,%d,%d,%.3f)\n", ((COrder*)pParam)->AreaCheckChangTemp.X, ((COrder*)pParam)->AreaCheckChangTemp.Y, ((COrder*)pParam)->AreaCheckChangTemp.Z, ((COrder*)pParam)->AreaCheckChangTemp.W);
#endif
}
/**************************************************************************資料表處理區塊*************************************************************************/
/*選擇影像修正*/
void COrder::ChooseVisionModify(LPVOID pParam) {
	CString StrBuff = ((COrder*)pParam)->GetCommandAddress();//獲取命令地址
	UINT D1 = 0, D2 = 0;
	((COrder*)pParam)->GetHashAddress(StrBuff, D1, D2);//獲取雜湊表地址
	for (UINT i = 0; i < ((COrder*)pParam)->PositionModifyNumber.at(D1).at(D2).size(); i++)
	{
		if (((COrder*)pParam)->PositionModifyNumber.at(D1).at(D2).at(i).Address == StrBuff)
		{
			if (((COrder*)pParam)->PositionModifyNumber.at(D1).at(D2).at(i).VisionNumber == -1)
			{
				((COrder*)pParam)->PositionModifyNumber.at(D1).at(D2).at(i).VisionNumber = 0;
			}
			((COrder*)pParam)->VisionOffset = ((COrder*)pParam)->VisionAdjust.at(((COrder*)pParam)->PositionModifyNumber.at(D1).at(D2).at(i).VisionNumber).VisionOffset;
		}
	}
}
/*選擇雷射修正*/
void COrder::ChooseLaserModify(LPVOID pParam){
	CString StrBuff = ((COrder*)pParam)->GetCommandAddress();//獲取命令地址
	UINT D1 = 0, D2 = 0;
	((COrder*)pParam)->GetHashAddress(StrBuff, D1, D2);//獲取雜湊表地址
	for (UINT i = 0; i < ((COrder*)pParam)->PositionModifyNumber.at(D1).at(D2).size(); i++)
	{
		if (((COrder*)pParam)->PositionModifyNumber.at(D1).at(D2).at(i).Address == StrBuff)
		{
			if (((COrder*)pParam)->PositionModifyNumber.at(D1).at(D2).at(i).LaserNumber == -1)
			{
				((COrder*)pParam)->PositionModifyNumber.at(D1).at(D2).at(i).LaserNumber = 0;
			}
			((COrder*)pParam)->LaserData.LaserMeasureHeight = ((COrder*)pParam)->LaserAdjust.at(((COrder*)pParam)->PositionModifyNumber.at(D1).at(D2).at(i).LaserNumber).LaserMeasureHeight;
		}
	}
}
/*紀錄修正表*/
void COrder::RecordCorrectionTable(LPVOID pParam) {
	CString StrBuff = ((COrder*)pParam)->GetCommandAddress();//獲取命令地址
	UINT D1 = 0, D2 = 0;
	((COrder*)pParam)->GetHashAddress(StrBuff, D1, D2);//獲取雜湊表地址
	if (((COrder*)pParam)->PositionModifyNumber.size() - 1 < D1)//判斷雜湊表數量是否足夠  size必須-1才是陣列編號
	{
		((COrder*)pParam)->PositionModifyNumber.resize(((COrder*)pParam)->PositionModifyNumber.size() + 10000);
	}
	if (D2 == 0)
	{
		if (((COrder*)pParam)->PositionModifyNumber.at(D1).size() == 0)
		{
			((COrder*)pParam)->PositionModifyNumber.at(D1).resize(1);
		}
	}
	else
	{
		if (((COrder*)pParam)->PositionModifyNumber.at(D1).size() < D2)
		{
			((COrder*)pParam)->PositionModifyNumber.at(D1).resize(D2 + 1000);
		}
	}
	//判斷雜湊表
	if (((COrder*)pParam)->PositionModifyNumber.at(D1).at(D2).size() == 0)//尚未新增過
	{
		if (((COrder*)pParam)->ModelControl.Mode == 0)//建表模式時
		{
			((COrder*)pParam)->PositionModifyNumber.at(D1).at(D2).push_back({ StrBuff, -1, -1 });
		}
		else if (((COrder*)pParam)->ModelControl.Mode == 1)//影像模式時
		{
			((COrder*)pParam)->PositionModifyNumber.at(D1).at(D2).push_back({ StrBuff, ((COrder*)pParam)->VisionCount, -1 });
		}
		else  if (((COrder*)pParam)->ModelControl.Mode == 2)//雷射模式時
		{
			((COrder*)pParam)->PositionModifyNumber.at(D1).at(D2).push_back({ StrBuff, ((COrder*)pParam)->VisionCount, ((COrder*)pParam)->LaserCount });
		}
		((COrder*)pParam)->CurrentTableAddress = StrBuff;
#ifdef PRINTF
		_cwprintf(L"RecordCorrectionTable()::地址%s成功加入修正表:%d,%d\n", ((COrder*)pParam)->PositionModifyNumber.at(D1).at(D2).back().Address,
			((COrder*)pParam)->PositionModifyNumber.at(D1).at(D2).back().VisionNumber,
			((COrder*)pParam)->PositionModifyNumber.at(D1).at(D2).back().LaserNumber);
#endif
	}
	else//新增過
	{
		for (UINT i = 0; i < ((COrder*)pParam)->PositionModifyNumber.at(D1).at(D2).size(); i++)//尋找是否相同地址
		{
			if (((COrder*)pParam)->PositionModifyNumber.at(D1).at(D2).at(i).Address == StrBuff)//已經新增過相同地址
			{
				if (((COrder*)pParam)->ModelControl.Mode == 1)//目前是影像模式
				{
					if (((COrder*)pParam)->PositionModifyNumber.at(D1).at(D2).at(i).VisionNumber == -1)//且此地址影像表沒有值
					{
						((COrder*)pParam)->PositionModifyNumber.at(D1).at(D2).at(i).VisionNumber = ((COrder*)pParam)->VisionCount;
					}
				}
				else if (((COrder*)pParam)->ModelControl.Mode == 2)//目前是雷射模式時
				{
					if (((COrder*)pParam)->PositionModifyNumber.at(D1).at(D2).at(i).LaserNumber == -1)//且此地址雷射表沒有值
					{
						((COrder*)pParam)->PositionModifyNumber.at(D1).at(D2).at(i).LaserNumber = ((COrder*)pParam)->LaserCount;
					}
				}
				((COrder*)pParam)->CurrentTableAddress = StrBuff;//紀錄表中目前的地址
#ifdef PRINTF
				_cwprintf(L"RecordCorrectionTable()::地址%s成功加入數值:%d,%d\n", ((COrder*)pParam)->PositionModifyNumber.at(D1).at(D2).at(i).Address,
					((COrder*)pParam)->PositionModifyNumber.at(D1).at(D2).at(i).VisionNumber,
					((COrder*)pParam)->PositionModifyNumber.at(D1).at(D2).at(i).LaserNumber);
#endif
				break;
			}
			else if (i == ((COrder*)pParam)->PositionModifyNumber.at(D1).at(D2).size() - 1)//地址不存在表中(step 或 sub 時發生)
			{
				if (((COrder*)pParam)->ModelControl.Mode == 0)//建表模式時
				{
					((COrder*)pParam)->PositionModifyNumber.at(D1).at(D2).push_back({ StrBuff,-1,-1 });
				}
				else if (((COrder*)pParam)->ModelControl.Mode == 1)//影像模式時
				{
					((COrder*)pParam)->PositionModifyNumber.at(D1).at(D2).push_back({ StrBuff, ((COrder*)pParam)->VisionCount, -1 });
				}
				else  if (((COrder*)pParam)->ModelControl.Mode == 2)//雷射模式時
				{
					((COrder*)pParam)->PositionModifyNumber.at(D1).at(D2).push_back({ StrBuff, ((COrder*)pParam)->VisionCount, ((COrder*)pParam)->LaserCount });
				}
				((COrder*)pParam)->CurrentTableAddress = StrBuff;
#ifdef PRINTF
				_cwprintf(L"RecordCorrectionTable()::地址%s成功加入修正表:%d,%d\n", ((COrder*)pParam)->PositionModifyNumber.at(D1).at(D2).back().Address,
					((COrder*)pParam)->PositionModifyNumber.at(D1).at(D2).back().VisionNumber,
					((COrder*)pParam)->PositionModifyNumber.at(D1).at(D2).back().LaserNumber);
#endif
				break;
			}
		}
	}
	/*舊版2017/03/20以前*/
//	if (((COrder*)pParam)->PositionModifyNumber.size())//修正表有值
//	{ 
//		for (UINT i = 0; i < ((COrder*)pParam)->PositionModifyNumber.size(); i++)//判斷表中地址是否存在
//		{
//			if (((COrder*)pParam)->PositionModifyNumber.at(i).Address == StrBuff)
//			{
//				if (((COrder*)pParam)->ModelControl.Mode == 1)//目前是影像模式
//				{
//					if (((COrder*)pParam)->PositionModifyNumber.at(i).VisionNumber == -1)//且此地址影像表沒有值
//					{
//						((COrder*)pParam)->PositionModifyNumber.at(i).VisionNumber = ((COrder*)pParam)->VisionCount;
//					}
//				}
//				else if (((COrder*)pParam)->ModelControl.Mode == 2)
//				{
//					if (((COrder*)pParam)->PositionModifyNumber.at(i).LaserNumber == -1)//且此地址雷射表沒有值
//					{
//						((COrder*)pParam)->PositionModifyNumber.at(i).LaserNumber = ((COrder*)pParam)->LaserCount;
//					}
//				}
//				((COrder*)pParam)->CurrentTableAddress = StrBuff;//紀錄表中目前的地址
//#ifdef PRINTF
//				_cwprintf(L"RecordCorrectionTable()::地址%s成功加入數值:%d,%d\n", ((COrder*)pParam)->PositionModifyNumber.at(i).Address, ((COrder*)pParam)->PositionModifyNumber.at(i).VisionNumber, ((COrder*)pParam)->PositionModifyNumber.at(i).LaserNumber);
//#endif
//				break;
//			} 
//			else if (i == ((COrder*)pParam)->PositionModifyNumber.size() - 1)//地址不存在表中
//			{
//				if (((COrder*)pParam)->ModelControl.Mode == 0)//建表模式時
//				{
//					((COrder*)pParam)->PositionModifyNumber.push_back({ StrBuff, -1, -1 });
//				}
//				else if (((COrder*)pParam)->ModelControl.Mode == 1)//影像模式時
//				{
//					((COrder*)pParam)->PositionModifyNumber.push_back({ StrBuff, ((COrder*)pParam)->VisionCount, -1 });
//				}
//				else  if (((COrder*)pParam)->ModelControl.Mode == 2)//雷射模式時
//				{
//					((COrder*)pParam)->PositionModifyNumber.push_back({ StrBuff, ((COrder*)pParam)->VisionCount, ((COrder*)pParam)->LaserCount });
//				}
//				((COrder*)pParam)->CurrentTableAddress = StrBuff;
//#ifdef PRINTF
//				_cwprintf(L"RecordCorrectionTable()::地址%s成功加入修正表:%d,%d\n", ((COrder*)pParam)->PositionModifyNumber.back().Address, ((COrder*)pParam)->PositionModifyNumber.back().VisionNumber, ((COrder*)pParam)->PositionModifyNumber.back().LaserNumber);
//#endif
//				break;
//			}
//		}   
//	}
//	else
//	{
//		if (((COrder*)pParam)->ModelControl.Mode == 0)//建表模式時
//		{
//			((COrder*)pParam)->PositionModifyNumber.push_back({ StrBuff, -1, -1 });
//#ifdef PRINTF
//			_cwprintf(L"RecordCorrectionTable()::進入建表模式增加PositionModifNumber\n");
//#endif
//		}
//		else if (((COrder*)pParam)->ModelControl.Mode == 1)//影像模式時/*****理論上不發生特殊防呆用*****/
//		{
//			((COrder*)pParam)->PositionModifyNumber.push_back({ StrBuff, ((COrder*)pParam)->VisionCount, -1 });
//#ifdef PRINTF
//			_cwprintf(L"RecordCorrectionTable()::進入影像模式增加PositionModifNumber\n");
//#endif
//		}
//		else if (((COrder*)pParam)->ModelControl.Mode == 2)//跳過影像模式直接雷射模式/*****理論上不發生特殊防呆用*****/
//		{
//			((COrder*)pParam)->PositionModifyNumber.push_back({ StrBuff, 0, ((COrder*)pParam)->LaserCount });
//#ifdef PRINTF
//			_cwprintf(L"RecordCorrectionTable()::跳過影像進入雷射模式增加PositionModifNumber\n");
//#endif
//		}
//#ifdef PRINTF
//		_cwprintf(L"RecordCorrectionTable()::地址%s成功加入修正表:%d,%d\n", ((COrder*)pParam)->PositionModifyNumber.back().Address, ((COrder*)pParam)->PositionModifyNumber.back().VisionNumber, ((COrder*)pParam)->PositionModifyNumber.back().LaserNumber);
//#endif
//	}
}
/*獲取雜湊表地址*/
void COrder::GetHashAddress(CString CommandAddress, UINT &D1, UINT &D2)
{
	D1 = _ttol(CommandResolve(CommandAddress, 0));
	if (RepeatData.StepRepeatNum.size())
	{
		D2 = 1;
		for (UINT i = RepeatData.StepRepeatNum.size(); i > 0; i--)
		{
			UINT TotalY = 1;
			for (UINT j = RepeatData.StepRepeatNum.size(); j > i; j--)
			{
				TotalY = TotalY * RepeatData.StepRepeatTotalX.at(j - 1) * RepeatData.StepRepeatTotalY.at(j - 1);
			}
			D2 = D2 + ((RepeatData.StepRepeatCountX.at(i - 1) - 1) * (RepeatData.StepRepeatTotalY.at(i - 1) * TotalY))
				+ ((RepeatData.StepRepeatCountY.at(i - 1) - 1) * TotalY);
		}
	}
#ifdef LOG
	CString Temp;
	Temp.Format(L"%s:D1 = %d,D2 = %d\n", CommandAddress, D1, D2);
	InitFileLog(Temp);
#endif
}
/**************************************************************************命令處理區塊***************************************************************************/
/*命令分解*/
CString COrder::CommandResolve(CString Command, UINT Choose)
{
	int iLength = Command.Find(_T(','));
	if (iLength <= 0)
	{
		iLength = Command.GetLength();
	}
	if (Choose <= 0)
	{
		return Command.Left(iLength);
	}
	else
	{
		return CommandResolve(Command.Right(Command.GetLength() - iLength - 1), --Choose);
	}
}
/*模板編號分解*/
CString COrder::ModelNumResolve(CString ModelNum, UINT Choose)
{
	int iLength = ModelNum.Find(_T('>'));
	if (iLength <= 0)
	{
		iLength = ModelNum.GetLength();
	}
	if (Choose <= 0)
	{
		return ModelNum.Left(iLength);
	}
	else
	{
		return ModelNumResolve(ModelNum.Right(ModelNum.GetLength() - iLength - 1), --Choose);
	}
}
/*命令單位轉換(初始為um)
*multiple:單位倍數
*Timemultiple:時間倍數
*/
CString COrder::CommandUnitConversinon(CString Command, DOUBLE multiple, DOUBLE Timemultiple)
{
	DOUBLE temp = 0;
	DOUBLE dvalue;
	LONG lvalue;
	CString result, exchange;
	//參數(X,Y,Z)
	if (CommandResolve(Command, 0) == L"Dot" ||
		CommandResolve(Command, 0) == L"LineStart" ||
		CommandResolve(Command, 0) == L"LinePassing" ||
		CommandResolve(Command, 0) == L"LineEnd" ||
		CommandResolve(Command, 0) == L"ArcPoint" ||
		CommandResolve(Command, 0) == L"CameraTrigger")
	{
		for (int i = 0; i < 4; i++)
		{
			if (i == 0)
			{
				result = CommandResolve(Command, i);
			}
			else
			{
				temp = _tstof(CommandResolve(Command, i));
				if (multiple < 1)
				{
					dvalue = (DOUBLE)(temp * multiple);
					exchange.Format(L",%.3f", dvalue);
				}
				else
				{
					lvalue = (LONG)(temp * multiple);
					exchange.Format(L",%d", lvalue);
				}
				result += exchange;
			}
		}
	}
	else if (CommandResolve(Command, 0) == L"CirclePoint")
	{
		for (int i = 0; i < 7; i++)
		{
			if (i == 0)
			{
				result = CommandResolve(Command, i);
			}
			else
			{
				temp = _tstof(CommandResolve(Command, i));
				if (multiple < 1)
				{
					dvalue = (DOUBLE)(temp * multiple);
					exchange.Format(L",%.3f", dvalue);
				}
				else
				{
					lvalue = (LONG)(temp * multiple);
					exchange.Format(L",%d", lvalue);
				}
				result += exchange;
			}
		}
	}
	else if (CommandResolve(Command, 0) == L"VirtualPoint" || CommandResolve(Command, 0) == L"StopPoint")
	{
		for (int i = 0; i < 5; i++)
		{
			if (i == 0)
			{
				result = CommandResolve(Command, i);
			}
			else if (i == 4)
			{
				result += L",";
				result += CommandResolve(Command, i);
			}
			else
			{
				temp = _tstof(CommandResolve(Command, i));
				if (multiple < 1)
				{
					dvalue = (DOUBLE)(temp * multiple);
					exchange.Format(L",%.3f", dvalue);
				}
				else
				{
					lvalue = (LONG)(temp * multiple);
					exchange.Format(L",%d", lvalue);
				}
				result += exchange;
			}
		}
	}
	else if (CommandResolve(Command, 0) == L"WaitPoint")
	{
		for (int i = 0; i < 6; i++)
		{
			if (i == 0)
			{
				result = CommandResolve(Command, i);
			}
			else if (i == 4)
			{
				temp = _tstof(CommandResolve(Command, i));
				if (Timemultiple < 1)
				{
					dvalue = (DOUBLE)(temp * Timemultiple);
					exchange.Format(L",%.3f", dvalue);
				}
				else
				{
					lvalue = (LONG)(temp * Timemultiple);
					exchange.Format(L",%d", lvalue);
				}
				result += exchange;
			}
			else if (i > 4)
			{
				result += L",";
				result += CommandResolve(Command, i);
			}
			else
			{
				temp = _tstof(CommandResolve(Command, i));
				if (multiple < 1)
				{
					dvalue = (DOUBLE)(temp * multiple);
					exchange.Format(L",%.3f", dvalue);
				}
				else
				{
					lvalue = (LONG)(temp * multiple);
					exchange.Format(L",%d", lvalue);
				}
				result += exchange;
			}
		}
	}
	else if (CommandResolve(Command, 0) == L"FillArea")
	{
		for (int i = 0; i < 10; i++)
		{
			if (i == 0)
			{
				result = CommandResolve(Command, i);
			}
			else if (i < 4)
			{
				result += L",";
				result += CommandResolve(Command, i);
			}
			else
			{
				temp = _tstof(CommandResolve(Command, i));
				if (multiple < 1)
				{
					dvalue = (DOUBLE)(temp * multiple);
					exchange.Format(L",%.3f", dvalue);
				}
				else
				{
					lvalue = (LONG)(temp * multiple);
					exchange.Format(L",%d", lvalue);
				}
				result += exchange;
			}
		}
	}
	else if (CommandResolve(Command, 0) == L"ZGoBack")
	{
		for (int i = 0; i < 3; i++)
		{
			if (i == 0)
			{
				result = CommandResolve(Command, i);
			}
			else if (i == 2)
			{
				result += L",";
				result += CommandResolve(Command, i);
			}
			else
			{
				temp = _tstof(CommandResolve(Command, i));
				if (multiple < 1)
				{
					dvalue = (DOUBLE)(temp * multiple);
					exchange.Format(L",%.3f", dvalue);
				}
				else
				{
					lvalue = (LONG)(temp * multiple);
					exchange.Format(L",%d", lvalue);
				}
				result += exchange;
			}
		}
	}
	else if (CommandResolve(Command, 0) == L"StepRepeatX" || CommandResolve(Command, 0) == L"StepRepeatY")
	{
		for (int i = 0; i < 9; i++)
		{
			if (i == 0)
			{
				result = CommandResolve(Command, i);
			}
			else if (i > 2)
			{
				result += L",";
				result += CommandResolve(Command, i);
				if (i == 8)
				{
					if (_ttol(CommandResolve(Command, 8)) != 0)
					{
						for (int j = 0; j < _ttol(CommandResolve(Command, 8)); j++)
						{
							result += L",";
							result += CommandResolve(Command, j + 9);
						}
					}
				}
			}
			else
			{
				temp = _tstof(CommandResolve(Command, i));
				if (multiple < 1)
				{
					dvalue = (DOUBLE)(temp * multiple);
					exchange.Format(L",%.3f", dvalue);
				}
				else
				{
					lvalue = (LONG)(temp * multiple);
					exchange.Format(L",%d", lvalue);
				}
				result += exchange;
			}
		}
	}
	else if (CommandResolve(Command, 0) == L"FindMark" || CommandResolve(Command, 0) == L"FiducialMark")
	{
		for (int i = 0; i < 6; i++)
		{
			if (i == 0)
			{
				result = CommandResolve(Command, i);
			}
			else if (i > 3)
			{
				result += L",";
				result += CommandResolve(Command, i);
			}
			else
			{
				temp = _tstof(CommandResolve(Command, i));
				if (multiple < 1)
				{
					dvalue = (DOUBLE)(temp * multiple);
					exchange.Format(L",%.3f", dvalue);
				}
				else
				{
					lvalue = (LONG)(temp * multiple);
					exchange.Format(L",%d", lvalue);
				}
				result += exchange;
			}
		}
	}
	else if (CommandResolve(Command, 0) == L"LaserHeight")
	{
		for (int i = 0; i < 4; i++)
		{
			if (i == 0)
			{
				result = CommandResolve(Command, i);
			}
			else if (i == 1)
			{
				result += L",";
				result += CommandResolve(Command, i);
			}
			else
			{
				temp = _tstof(CommandResolve(Command, i));
				if (multiple < 1)
				{
					dvalue = (DOUBLE)(temp * multiple);
					exchange.Format(L",%.3f", dvalue);
				}
				else
				{
					lvalue = (LONG)(temp * multiple);
					exchange.Format(L",%d", lvalue);
				}
				result += exchange;
			}
		}
	}
	else if (CommandResolve(Command, 0) == L"LaserDetect")
	{
		for (int i = 0; i < 6; i++)
		{
			if (i == 0)
			{
				result = CommandResolve(Command, i);
			}
			else if (i == 1)
			{
				result += L",";
				result += CommandResolve(Command, i);
			}
			else
			{
				temp = _tstof(CommandResolve(Command, i));
				if (multiple < 1)
				{
					dvalue = (DOUBLE)(temp * multiple);
					exchange.Format(L",%.3f", dvalue);
				}
				else
				{
					lvalue = (LONG)(temp * multiple);
					exchange.Format(L",%d", lvalue);
				}
				result += exchange;
			}
		}
	}
	else if (CommandResolve(Command, 0) == L"DispenseDotSet")
	{
		for (int i = 0; i < 3; i++)
		{
			if (i == 0)
			{
				result = CommandResolve(Command, i);
			}
			else
			{
				temp = _tstof(CommandResolve(Command, i));
				if (Timemultiple < 1)
				{
					dvalue = (DOUBLE)(temp * Timemultiple);
					exchange.Format(L",%.3f", dvalue);
				}
				else
				{
					lvalue = (LONG)(temp * Timemultiple);
					exchange.Format(L",%d", lvalue);
				}
				result += exchange;
			}
		}
	}
	else if (CommandResolve(Command, 0) == L"DispenseDotEnd")
	{
		for (int i = 0; i < 4; i++)
		{
			if (i == 0)
			{
				result = CommandResolve(Command, i);
			}
			else
			{
				temp = _tstof(CommandResolve(Command, i));
				if (multiple < 1)
				{
					dvalue = (DOUBLE)(temp * multiple);
					exchange.Format(L",%.3f", dvalue);
				}
				else
				{
					lvalue = (LONG)(temp * multiple);
					exchange.Format(L",%d", lvalue);
				}
				result += exchange;
			}
		}
	}
	else if (CommandResolve(Command, 0) == L"DotSpeedSet")
	{
		for (int i = 0; i < 2; i++)
		{
			if (i == 0)
			{
				result = CommandResolve(Command, i);
			}
			else
			{
				temp = _tstof(CommandResolve(Command, i));
				if (multiple < 1)
				{
					dvalue = (DOUBLE)(temp * multiple);
					exchange.Format(L",%.3f", dvalue);
				}
				else
				{
					lvalue = (LONG)(temp * multiple);
					exchange.Format(L",%d", lvalue);
				}
				result += exchange;
			}
		}
	}
	else if (CommandResolve(Command, 0) == L"DispenseLineSet")
	{
		for (int i = 0; i < 7; i++)
		{
			if (i == 0)
			{
				result = CommandResolve(Command, i);
			}
			else if (i == 1 || i == 3 || i == 4 || i == 6)
			{
				temp = _tstof(CommandResolve(Command, i));
				if (Timemultiple < 1)
				{
					dvalue = (DOUBLE)(temp * Timemultiple);
					exchange.Format(L",%.3f", dvalue);
				}
				else
				{
					lvalue = (LONG)(temp * Timemultiple);
					exchange.Format(L",%d", lvalue);
				}
				result += exchange;
			}
			else
			{
				temp = _tstof(CommandResolve(Command, i));
				if (multiple < 1)
				{
					dvalue = (DOUBLE)(temp * multiple);
					exchange.Format(L",%.3f", dvalue);
				}
				else
				{
					lvalue = (LONG)(temp * multiple);
					exchange.Format(L",%d", lvalue);
				}
				result += exchange;
			}
		}
	}
	else if (CommandResolve(Command, 0) == L"DispenseLineEnd")
	{
		for (int i = 0; i < 6; i++)
		{
			if (i == 0)
			{
				result = CommandResolve(Command, i);
			}
			else if (i == 1)
			{
				result += L",";
				result += CommandResolve(Command, i);
			}
			else
			{
				temp = _tstof(CommandResolve(Command, i));
				if (multiple < 1)
				{
					dvalue = (DOUBLE)(temp * multiple);
					exchange.Format(L",%.3f", dvalue);
				}
				else
				{
					lvalue = (LONG)(temp * multiple);
					exchange.Format(L",%d", lvalue);
				}
				result += exchange;
			}
		}
	}
	else if (CommandResolve(Command, 0) == L"LineSpeedSet")
	{
		for (int i = 0; i < 2; i++)
		{
			if (i == 0)
			{
				result = CommandResolve(Command, i);
			}
			else
			{
				temp = _tstof(CommandResolve(Command, i));
				if (multiple < 1)
				{
					dvalue = (DOUBLE)(temp * multiple);
					exchange.Format(L",%.3f", dvalue);
				}
				else
				{
					lvalue = (LONG)(temp * multiple);
					exchange.Format(L",%d", lvalue);
				}
				result += exchange;
			}
		}
	}
	else if (CommandResolve(Command, 0) == L"DispenseAccSet")
	{
		for (int i = 0; i < 3; i++)
		{
			if (i == 0)
			{
				result = CommandResolve(Command, i);
			}
			else if (i == 1)
			{
				result += L",";
				result += CommandResolve(Command, i);
			}
			else
			{
				temp = _tstof(CommandResolve(Command, i));
				if (multiple < 1)
				{
					dvalue = (DOUBLE)(temp * multiple);
					exchange.Format(L",%.3f", dvalue);
				}
				else
				{
					lvalue = (LONG)(temp * multiple);
					exchange.Format(L",%d", lvalue);
				}
				result += exchange;
			}
		}
	}
	else
	{
		result = Command;
	}
	return result;
}
/*命令地址獲取*/
CString COrder::GetCommandAddress()
{
	CString StrBuff, Temp;
	StrBuff.Format(_T("%d"), RunData.RunCount.at(RunData.MSChange.at(RunData.StackingCount)));//獲取命令編號
	if (RepeatData.StepRepeatNum.size())//有StepRepeat時地址紀錄的方式
	{
		for (UINT i = 0; i < RepeatData.StepRepeatNum.size(); i++)
		{
			Temp.Format(_T(",%d-%d"), RepeatData.StepRepeatCountX.at(i), RepeatData.StepRepeatCountY.at(i));
			StrBuff = StrBuff + Temp;
		}
	}
	if (!Program.SubroutineStack.empty())//有CallSubroutine時地址紀錄的方式
	{
		for (UINT i = 0; i < Program.SubroutineStack.size(); i++)
		{
			Temp.Format(_T(",%d"), Program.SubroutineStack.at(i));
			StrBuff = StrBuff + Temp;
		}
	}
	return StrBuff;
}
/**************************************************************************程序變數處理區塊************************************************************************/
/*參數設為Default*/
void COrder::ParameterDefult() {
	//運動
	DispenseDotSet = Default.DispenseDotSet;
	DispenseDotEnd = Default.DispenseDotEnd;
	DotSpeedSet = Default.DotSpeedSet;
	DotSpeedSet.InitSpeed = 2000;//防止初速度被初始化為0
	DispenseLineSet = Default.DispenseLineSet;
	DispenseLineEnd = Default.DispenseLineEnd;
	LineSpeedSet = Default.LineSpeedSet;
	LineSpeedSet.InitSpeed = 2000;//防止初速度被初始化為0
	ZSet = Default.ZSet;
	GlueData = Default.GlueData;
	GoHome = Default.GoHome;
	CleanerData = Default.CleanerData;
	//影像
	VisionSet = VisionDefault.VisionSet;
	VisionFile = VisionDefault.VisionFile;
	VisionSerchError = VisionDefault.VisionSerchError;
	//檢測
#ifdef VI
	VI_GetMosaicMoveDist(50, AreaCheckParamterDefault.ViewMove.x, AreaCheckParamterDefault.ViewMove.y);
#endif 
}
/*判斷指標初始化*/
void COrder::DecideInit()
{
	//初始化事件
	ThreadEvent.Thread = ::CreateEvent(NULL, TRUE, TRUE, L"MY_THREAD_EVT");
	ThreadEvent.CheckActionThread = ::CreateEvent(NULL, TRUE, TRUE, L"MY_CHECKACTIONTHREAD_EVT");
	ThreadEvent.CheckCoordinateScanThread = ::CreateEvent(NULL, TRUE, TRUE, L"MY_CHECKCOORDINATESCANTHREAD_EVT");
	ThreadEvent.MosaicDlgThread = ::CreateEvent(NULL, TRUE, TRUE, L"MY_MOSAICDLGTHREAD_EVT");
	//OutThreadEvent = ::OpenEvent(EVENT_ALL_ACCESS, FALSE, L"MY_THREAD_EVT");//測試用
	//原點賦歸狀態設為賦歸完成
	RunStatusRead.GoHomeStatus = TRUE;
	//運動運行狀態清除
	m_Action.m_bIsStop = FALSE;
	m_Action.m_bIsPause = FALSE;
	//程序狀態堆疊
	InitData = { 0,0,0,0,0 };
	ArcData.push_back(InitData);
	CircleData1.push_back(InitData);
	CircleData2.push_back(InitData);
	StartData.push_back(InitData);
	OffsetData.push_back(InitData);
	OffsetData.at(0).Status = TRUE;//將主程序offset設為已修正
	//子程序和標籤
	Program.LabelName = _T("");
	Program.LabelCount = 0;
	Program.CallSubroutineStatus = FALSE;
	Program.SubroutineStack.clear();
	Program.SubroutinCount = 0;
	Program.SubroutinePointStack.push_back(InitData);
	Program.SubroutineVisioModifyJudge.clear();
	Program.SubroutineModelControlSwitch = FALSE;
	//副程式和主程式控制結構
	RunData.MSChange.push_back(0); //副程式改變控制程序的堆疊
	for (UINT i = 0; i < RunData.MSChange.size(); i++)
	{
		RunData.MSChange.at(i) = 0;
	}
	RunStatusRead.CurrentRunCommandNum = 0;//目前運行命令計數清0
	//TODO::之後做總進度
	//RunStatusRead.RegistrationStatus = FALSE;//對位完畢狀態清除
	//RunStatusRead.CommandTotalCount = 0;//命令總數量清0
	//RunStatusRead.CurrentRunCommandCount = 0;//命令總數量計數清0
	RunData.StackingCount = 0;//主副程式計數  
	RunData.SubProgramName = _T("");//副程式判斷標籤
	RunData.ActionStatus.push_back(0);//運動狀態清0
	//運行計數清0
	for (UINT i = 0; i < RunData.RunCount.size(); i++)
	{
		RunData.RunCount.at(i) = 0;
	}
	//Repeat狀態、標籤清除
	RepeatData.LoopSwitch = FALSE;
	RepeatData.StepRepeatLabel = _T("");
	RepeatData.StepRepeatLabelLock = FALSE;
	RepeatData.AddInStepRepeatSwitch = FALSE;
	RepeatData.AllNewStepRepeatNum = 0;
	RepeatData.AllDeleteStepRepeatNum = 0;
	//影像狀態初始化
	FindMark.Point.Status = FALSE;
	FiducialMark1.Point.Status = FALSE;
	FiducialMark2.Point.Status = FALSE;
	//查找狀態設成未找到
	FiducialMark1.FindMarkStatus = FALSE;
	FiducialMark2.FindMarkStatus = FALSE;
	//影像Model 指針初始化 /*****注意記憶體分配問題*****/
	FindMark.MilModel = malloc(sizeof(int));
	FiducialMark1.MilModel = malloc(sizeof(int));
	FiducialMark2.MilModel = malloc(sizeof(int));
	*(int*)FindMark.MilModel = 0;
	*(int*)FiducialMark1.MilModel = 0;
	*(int*)FiducialMark2.MilModel = 0;
	//影像Offset初始化
	VisionOffset = { {0,0,0,0},0,0,0 };
	//影像Trigger初始化
	VisionTrigger.AdjustStatus = 0;
	VisionTrigger.Trigger1Switch = FALSE;
	VisionTrigger.Trigger2Switch = FALSE;
	//雷射狀態初始化
	LaserSwitch.LaserHeight = FALSE;
	LaserSwitch.LaserPointAdjust = FALSE;
	LaserSwitch.LaserDetect = FALSE;
	LaserSwitch.LaserAdjust = FALSE;
	LaserSwitch.LaserSkip = FALSE;
	//修正計數初始化
	VisionCount = 0;
	LaserCount = 0;
	//修正表、影像表、雷射表存值初始化
	//TODO::DEMO所以加入
	if (!DemoTemprarilySwitch)//如果DemoTemprarilySwitch為FALSE 清除
	{
#ifdef PRINTF
		_cwprintf(L"DecideInit()::我有進入歸零PositionModifyNumber&&LaserAdjust\n");
#endif
		if (LoadCommandData.FirstRun)
			PositionModifyNumber.resize(10000);//預設resize 10000筆資料
		m_Action.LA_Clear();//清除連續線段陣列()
		LaserAdjust.push_back({ -999999999 });//初始化雷射高度記錄表
	}  
	VisionAdjust.push_back({ { { 0,0,0,0 },0,0,0 } });
	//判斷是否有影像修正(給CallSubroutine使用)
	VisioModifyJudge = FALSE;
	//虛擬座標初始化(修正過)
	VirtualCoordinateData = { 0,0,0,0 };
	//虛擬座標初始化(未修正)
	NVMVirtualCoordinateData = { 0,0,0,0 };
	//計數連續線段初始化
	LaserContinuousControl.ContinuousLineCount = -10000;
	//控制連續線段執行開關初始化(讓呼叫連續線段使用)
	LaserContinuousControl.ContinuousSwitch = FALSE;
	//雷射測高數值初始化
	LaserData.LaserMeasureHeight = -999999999;
	LaserData.LaserExecutedAddress = -1;//雷射執行過地址初始化
	//檢測參數初始化
	CheckSwitch = { 0,0,0,0,0 };//檢測開關初始化
	CheckResult = { 0,0,0 };//OK、NG檢測計算初始化
	CheckFinishRecord.clear();//直徑、模板檢測完成資料存放陣列
	CheckModel = 0;//檢測模式初始化
	AreaCheckChangTemp = { 0,0,0,0 };//區域中間點轉換檢測暫存參數清零
	AreaCheckFinishRecord.clear();//區域檢測完成資料存放陣列
	
	/****************************************************************/
	ActionCount = 0;
	V_ActionCount = 0;
#ifdef PRINTF
	_cwprintf(L"DecideInit()::Init()\n");
#endif
}
/*判斷指標清除*/
void COrder::DecideClear()
{
	//關閉事件
	//::CloseHandle(OutThreadEvent);
	::CloseHandle(ThreadEvent.Thread);
	::CloseHandle(ThreadEvent.CheckActionThread);
	::CloseHandle(ThreadEvent.CheckCoordinateScanThread);
	::CloseHandle(ThreadEvent.MosaicDlgThread);
	//狀態堆疊清除
	ArcData.clear();
	CircleData1.clear();
	CircleData2.clear();
	StartData.clear();
	OffsetData.clear();
	//子程序狀態堆疊清除
	Program.SubroutineStack.clear();
	Program.SubroutinePointStack.clear();
	Program.SubroutineVisioModifyJudge.clear();
	//主副程式狀態堆疊清除
	RunData.MSChange.clear();
	RunData.ActionStatus.clear();
	RunData.RunCount.clear();
	//迴圈陣列清除
	RepeatData.LoopAddressNum.clear();
	RepeatData.LoopCount.clear();
	RepeatData.StepRepeatNum.clear();
	RepeatData.SSwitch.clear();
	RepeatData.StepRepeatInitOffsetX.clear();
	RepeatData.StepRepeatInitOffsetY.clear();
	RepeatData.StepRepeatCountX.clear();
	RepeatData.StepRepeatCountY.clear();
	RepeatData.StepRepeatTotalX.clear();
	RepeatData.StepRepeatTotalY.clear();
	RepeatData.StepRepeatBlockData.clear();
	RepeatData.StepRepeatIntervel.clear();
	//影像釋放記憶體
#ifdef VI
	if (*(int*)FindMark.MilModel != 0)
	{
		VI_ModelFree(FindMark.MilModel);
	}
	if (*(int*)FiducialMark1.MilModel != 0)
	{
		VI_ModelFree(FiducialMark1.MilModel);
	}
	if (*(int*)FiducialMark2.MilModel != 0)
	{
		VI_ModelFree(FiducialMark2.MilModel);
	}
#endif
	free(FindMark.MilModel);
	FindMark.MilModel = NULL;//釋放後必須將指針賦予初值，避免產生野指針
	free(FiducialMark1.MilModel);
	FiducialMark1.MilModel = NULL;//釋放後必須將指針賦予初值，避免產生野指針
	free(FiducialMark2.MilModel);
	FiducialMark2.MilModel = NULL;//釋放後必須將指針賦予初值，避免產生野指針
	//影像檔案清除
	VisionFile.ModelCount = 0;
	VisionFile.AllModelName.clear();
	//影像Trigger陣列清除
	VisionTrigger.Trigger1.clear();
	VisionTrigger.Trigger2.clear();
	//控制模組陣列清除 
	//TODO::DEMO所以加入
	if (!DemoTemprarilySwitch) //如果DemoTemprarilySwitch為FALSE 清除
	{
#ifdef PRINTF
		_cwprintf(L"DecideClear()::我有進入清除PositionModifyNumber&&LaserAdjust\n");
#endif
		//修正表初始數值
		for (UINT i = 0; i < PositionModifyNumber.size(); i++)
			for (UINT j = 0; j < PositionModifyNumber.at(i).size(); j++)
				for (UINT k = 0; k < PositionModifyNumber.at(i).at(j).size(); k++)
				{
					PositionModifyNumber.at(i).at(j).at(k).VisionNumber = -1;
					PositionModifyNumber.at(i).at(j).at(k).LaserNumber = -1;
				}
		LaserAdjust.clear();//清除雷射高度記錄表
	}
	VisionAdjust.clear();//清除影像修正記錄表
	//檢測資料釋放
	ClearCheckData(TRUE,TRUE);//清除檢測資料 
	IntervalCheckCoordinate.clear();//清除直徑、模板檢測點陣列
	IntervalAreaCheck.clear();//清除區域檢測資料

#ifdef PRINTF
		_cwprintf(L"DecideClear()::Clear()\n");
#endif
}
/*劃分主程式和副程式*/
void COrder::MainSubProgramSeparate()
{
	Command.clear();
	std::vector<UINT> SubProgramCount;
	BOOL SubroutineFlag = FALSE;
	for (UINT i = 0; i < CommandMemory.size(); i++) {//搜尋所有Command分區塊
		if (CommandResolve(CommandMemory.at(i), 0) == L"SubProgramStart")
		{
			SubProgramCount.push_back(i);
			SubroutineFlag = TRUE;
		}
		if (!SubroutineFlag)
		{
			CommandSwap.push_back(CommandMemory.at(i));
		}
		if (CommandResolve(CommandMemory.at(i), 0) == L"SubProgramEnd" && SubroutineFlag)
		{
			SubProgramCount.push_back(i);
			SubroutineFlag = FALSE;
		}
	}
	Command.push_back(CommandSwap);
	CommandSwap.clear();
	RunData.RunCount.push_back(0);
	for (UINT i = 0; i < SubProgramCount.size(); i++)//將所有區塊中的命令加入
	{
		for (UINT j = SubProgramCount.at(i); j <= SubProgramCount.at(i + 1); j++)
		{
			CommandSwap.push_back(CommandMemory.at(j));
		}
		Command.push_back(CommandSwap);
		CommandSwap.clear();
		RunData.RunCount.push_back(0);
		i++;
	}
}
/*判斷解譯起始模組*/
void COrder::DecideBeginModel(BOOL ViewMode)
{
	//TODO::模組方式目前尚未加入副程式
	//模式清除
	if (LoadCommandData.FirstRun)
		ModelControl.Mode = 0;
	else
		ModelControl.Mode = 3;
	ModelControl.ModeChangeAddress = -1;
	ModelControl.VisionModeJump = TRUE;
	ModelControl.LaserAndCheckModeJump = ViewMode;
	//模式初始化判斷
	for (UINT i = 1; i < Command.at(0).size(); i++) //不考慮副程式
	{
		if (CommandResolve(Command.at(0).at(i), 0) == L"FindMarkAdjust" ||
			CommandResolve(Command.at(0).at(i), 0) == L"FiducialMarkAdjust")
		{ 
			ModelControl.VisionModeJump = FALSE;
			if (!LoadCommandData.FirstRun)
				ModelControl.Mode = 1;
		}
	}
}
/**************************************************************************影像檔案處理區塊************************************************************************/
/*搜尋檔案名
*szPath:目錄路徑
*szName:搜尋檔名的格式
*/
BOOL COrder::ListAllFileInDirectory(LPTSTR szPath, LPTSTR szName) {
	HANDLE hListFile;
	TCHAR szFilePath[MAX_PATH]; //檔案名 
	TCHAR szFullPath[MAX_PATH];	//檔案目錄 
	WIN32_FIND_DATA FindFileData;

	lstrcpy(szFilePath, szPath);
	lstrcat(szFilePath, szName);

	hListFile = FindFirstFile(szFilePath, &FindFileData);
	if (hListFile == INVALID_HANDLE_VALUE) {
		//MessageBox(_T("無檔案"));
		return 1;
	}
	else {
		do {
			if (lstrcmp(FindFileData.cFileName, TEXT(".")) == 0 ||
				lstrcmp(FindFileData.cFileName, TEXT("..")) == 0)
				continue;
			wsprintf(szFullPath, L"%s\\%s", szPath, FindFileData.cFileName);//將szPath和FindFileData.cFileName 字串相加放到szFullPath裡 
			VisionFile.AllModelName.push_back(FindFileData.cFileName);
			VisionFile.ModelCount++;
			//MessageBox(szFullPath);
			if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {//如果目錄下還有目錄繼續往下尋找
				//printf("<DIR>");
				ListAllFileInDirectory(szFullPath, szName);
			}
		} while (FindNextFile(hListFile, &FindFileData));
	}
	return 0;
}
/*判斷檔案是否存在
*FilePathName:檔案路徑名稱
*/
BOOL COrder::FileExist(LPCWSTR FilePathName)
{
	HANDLE hFile;
	WIN32_FIND_DATA FindData;
	hFile = FindFirstFile(FilePathName, &FindData);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFile);
		return FALSE;
	}
	return TRUE;
}
/*判斷路徑創建目錄
*FilePathName:檔案路徑名稱
*/
BOOL COrder::FilePathExist(CString FilePathName)
{
	int pos = FilePathName.ReverseFind('\\');
	CString NewFilePathName = FilePathName.Left(pos);//刪掉 '\\'
	/*判斷路徑，路径不存在则创建该路径*/
	CFileFind m_FileFind;
	if (!m_FileFind.FindFile(NewFilePathName))
	{
		if (CreateDirectory(NewFilePathName,NULL))
		{
			return TRUE;
		}
#ifdef PRINTF
		_cwprintf(L"FilePathExist()::路徑創建失敗%s\n", NewFilePathName);
#endif
		return FALSE;
	} 
	return TRUE;
}
/*獲取日期檔名*/
CString COrder::GetDataFileName()
{
	/*獲取現在時間*/
	SYSTEMTIME st;
	GetLocalTime(&st);
	CString StrBuff;
	StrBuff.Format(_T("Result_%04d%02d%02d_%02d_%02d_%02d_%02d"), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
	return StrBuff;
}
/*刪除目錄內檔案(利用system)*/
BOOL COrder::FileDelete(CString FilePathName) 
{
	int pos = FilePathName.ReverseFind('\\');
	CString NewFilePathName = L"del \"" + FilePathName.Left(pos) + L"\" /Q";
	const size_t newsizew = (NewFilePathName.GetLength() + 1) * 2;
	char *nstringw = new char[newsizew];
	size_t convertedCharsw = 0;
	errno_t err = wcstombs_s(&convertedCharsw, nstringw, newsizew, NewFilePathName, _TRUNCATE);
	if (err == STRUNCATE)
	{
		_cwprintf(L"FileDelete()::字串發生截斷!,%s\n", nstringw);
	}
	system(nstringw);//刪除該文件夾所有資料
	//RemoveDirectory(NewFilePathName);//刪除空的資料夾
	delete nstringw;//釋放記憶體
	return 0;
}
/*資料夾搜尋檔案刪除所有檔案*/
BOOL COrder::SearchDirectory(CString FilePathName)
{
	HANDLE hListFile;
	TCHAR szFilePath[MAX_PATH];//檔案名 
	TCHAR szFullPath[MAX_PATH];//檔案目錄 
	WIN32_FIND_DATA FindFileData;
	DWORD dwTotalFileNum = 0;

	lstrcpy(szFilePath, FilePathName);
	lstrcat(szFilePath, TEXT("\\*"));

	hListFile = FindFirstFile(szFilePath, &FindFileData);
	if (hListFile == INVALID_HANDLE_VALUE) {
#ifdef PRINTF 
		_cwprintf(L"SearchDirectory()::錯誤:%d\n", GetLastError());
#endif
		return 1;
	}
	else {
		do {
			if (lstrcmp(FindFileData.cFileName, TEXT(".")) == 0 ||
				lstrcmp(FindFileData.cFileName, TEXT("..")) == 0)
				continue;
			wsprintf(szFullPath, L"%s\\%s", FilePathName, FindFileData.cFileName);//將szPath和FindFileData.cFileName 字串相加放到szFullPath裡 
			if(DeleteFile(szFullPath))//刪除檔案
				dwTotalFileNum++;
#ifdef PRINTF 
			//_cwprintf(L"\nSearchDirectory()::%d\tpath:%s", dwTotalFileNum, szFullPath);
#endif
			if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				SearchDirectory(szFullPath);
			}
		} while (FindNextFile(hListFile, &FindFileData));
	}
#ifdef PRINTF
	_cwprintf(L"SearchDirectory()::<DIR>%s\n共刪除%d個檔案\n", FilePathName,dwTotalFileNum);
#endif
	return 0;
}
/************************************************************************StepRepeat特殊處理區塊********************************************************************/
/*Subroutine預處理尋找指令(讓StepRepeat設定初始offset)*/
BOOL  COrder::SubroutinePretreatmentFind(LPVOID pParam)
{
	UINT CurrentRunCount = ((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)) + 1;
	UINT MaxRunNum = ((COrder*)pParam)->Command.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)).size() - 1;
	CString Command;
	for (UINT i = CurrentRunCount; i < MaxRunNum; i++)
	{
		Command = ((COrder*)pParam)->Command.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)).at(i);
		if (CommandResolve(Command, 0) == L"Dot" ||
			CommandResolve(Command, 0) == L"LineStart" ||
			CommandResolve(Command, 0) == L"LinePassing" ||
			CommandResolve(Command, 0) == L"LineEnd" ||
			CommandResolve(Command, 0) == L"ArcPoint" ||
			CommandResolve(Command, 0) == L"CirclePoint" ||
			CommandResolve(Command, 0) == L"VirtualPoint" ||
			CommandResolve(Command, 0) == L"WaitPoint" ||
			CommandResolve(Command, 0) == L"StopPoint" ||
			CommandResolve(Command, 0) == L"FindMark" ||
			CommandResolve(Command, 0) == L"FiducialMark" ||
			CommandResolve(Command, 0) == L"CameraTrigger")
		{
			Program.SubroutineCommandPretreatment = Command;
			return TRUE;
		}
		else if (CommandResolve(Command, 0) == L"FillArea")
		{
			Program.SubroutineCommandPretreatment = CommandResolve(Command, 0) + _T(",") + CommandResolve(Command, 4) + _T(",") + CommandResolve(Command, 5);
			return TRUE;
		}
		else if (CommandResolve(Command, 0) == L"LaserHeight" ||
			CommandResolve(Command, 0) == L"LaserDetect")
		{
			Program.SubroutineCommandPretreatment = CommandResolve(Command, 0) + _T(",") + CommandResolve(Command, 2) + _T(",") + CommandResolve(Command, 3);
			return TRUE;
		}
	}
	return FALSE;
}
/*StepRepeat強行跳轉判斷(防止跳出StepRepeat區間出現錯誤)
*Address:跳出的地址
*/
void COrder::StepRepeatJumpforciblyJudge(LPVOID pParam, UINT Address)
{
	//判斷是否有StepRepeat時且跳出區間
	if (((COrder*)pParam)->Program.SubroutineStack.empty())//有CallSubroutine時不做此種判斷
	{
		if (((COrder*)pParam)->RepeatData.StepRepeatNum.size())//判斷是否有StepRepeat
		{
			if (((COrder*)pParam)->RepeatData.StepRepeatIntervel.size())//判斷是否有區間
			{
				for (UINT i = 0; i < ((COrder*)pParam)->RepeatData.StepRepeatIntervel.size(); i++)
				{
					if (((COrder*)pParam)->RepeatData.StepRepeatIntervel.back().BeginAddress > Address ||
						Address > ((COrder*)pParam)->RepeatData.StepRepeatIntervel.back().EndAddress)
					{
#ifdef PRINTF
						_cwprintf(L"StepRepeatJumpforciblyJudge()::發生跳出StepRepeat刪除StepRepeat陣列\n");
#endif
						//刪除最後一項StepRepeat
						((COrder*)pParam)->RepeatData.StepRepeatBlockData.pop_back();
						((COrder*)pParam)->RepeatData.SSwitch.pop_back();
						((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X = ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.back();
						((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y = ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.back();
						((COrder*)pParam)->RepeatData.StepRepeatNum.pop_back();
						((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.pop_back();
						((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.pop_back();
						((COrder*)pParam)->RepeatData.StepRepeatCountX.pop_back();
						((COrder*)pParam)->RepeatData.StepRepeatCountY.pop_back();
						((COrder*)pParam)->RepeatData.StepRepeatTotalX.pop_back();
						((COrder*)pParam)->RepeatData.StepRepeatTotalY.pop_back();
						((COrder*)pParam)->RepeatData.StepRepeatIntervel.pop_back();
						//判斷是否為最後一個StepRepeat,如果是清除刪除、新增計數
						if (!((COrder*)pParam)->RepeatData.StepRepeatNum.size())
						{
							((COrder*)pParam)->RepeatData.AllNewStepRepeatNum = 0;
							((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum = 0;
#ifdef PRINTF
							_cwprintf(L"StepRepeatJumpforciblyJudge()::新增、刪除總數:%d,%d\n", ((COrder*)pParam)->RepeatData.AllNewStepRepeatNum, ((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum);
#endif 
						}
					}
					else//越內層區間會越小所以找一個不成立就可以Break
					{
						break;
					}
				}
			}
		}
	}
}
/**************************************************************************阻斷處理區塊****************************************************************************/
/*阻斷處理方式(加入StepRepeatX時)
*RepeatStatus:判斷要檢查阻斷時是否相同StepRepeatLabel
*/
void COrder::BlockProcessStartX(CString Command, LPVOID pParam, BOOL RepeatStatus)
{
	CString BlockBuff;
	BOOL DeleteStepRepeat = FALSE;//用來判斷是否有刪除StepRepeat
	int StepRepeatBlockSize = ((COrder*)pParam)->RepeatData.StepRepeatBlockData.back().BlockPosition.size();
	for (int i = 0; i < StepRepeatBlockSize; i++)//搜尋第一項阻斷陣列 是否有1-1
	{
#ifdef PRINTF
		_cwprintf(_T("BlockProcessStartX()::%d-%d.StepRepeatX 檢查阻斷\n"), ((COrder*)pParam)->RepeatData.StepRepeatCountX.back(), ((COrder*)pParam)->RepeatData.StepRepeatCountY.back());
#endif
		//S型
		if (_ttol(CommandResolve(Command, 5)) == 1)
		{
			//S型 阻斷編號
			if (!((COrder*)pParam)->RepeatData.SSwitch.back())
			{
				BlockBuff.Format(_T("%d-%d"), ((COrder*)pParam)->RepeatData.StepRepeatCountX.back(),
					(_ttol(CommandResolve(Command, 4)) + 1 - ((COrder*)pParam)->RepeatData.StepRepeatCountY.back()));
#ifdef PRINTF
				_cwprintf(L"BlockProcessStartX()::標籤只有Y變X正常:%s\n", BlockBuff);
#endif
			}
			else
			{
				BlockBuff.Format(_T("%d-%d"), (_ttol(CommandResolve(Command, 3)) + 1 - ((COrder*)pParam)->RepeatData.StepRepeatCountX.back()),
					(_ttol(CommandResolve(Command, 4)) + 1 - ((COrder*)pParam)->RepeatData.StepRepeatCountY.back()));
#ifdef PRINTF
				_cwprintf(L"BlockProcessStartX()::標籤XY變:%s\n", BlockBuff);
#endif
			}
			if (((COrder*)pParam)->RepeatData.StepRepeatBlockData.back().BlockPosition.at(i) == BlockBuff)
			{
#ifdef PRINTF
				_cwprintf(L"BlockProcessStartX()::有阻斷\n");
#endif
				if (((COrder*)pParam)->RepeatData.StepRepeatCountX.back() > 1)
				{
					if (((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X == ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.back() + (_ttol(CommandResolve(Command, 1))*(_ttol(CommandResolve(Command, 3)) - 1)))
					{
						((COrder*)pParam)->RepeatData.SSwitch.back() = FALSE;
					}
					else if (((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X == ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.back())
					{
						((COrder*)pParam)->RepeatData.SSwitch.back() = TRUE;
					}

					if (!((COrder*)pParam)->RepeatData.SSwitch.back())
					{
						((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X = ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X - _ttol(CommandResolve(Command, 1));
					}
					else
					{
						((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X = ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X + _ttol(CommandResolve(Command, 1));
					}
					((COrder*)pParam)->RepeatData.StepRepeatCountX.back()--;
				}
				else if (((COrder*)pParam)->RepeatData.StepRepeatCountY.back() > 1)
				{
					((COrder*)pParam)->RepeatData.SSwitch.back() = !((COrder*)pParam)->RepeatData.SSwitch.back();
#ifdef PRINTF
					_cwprintf(L"BlockProcessStartX()::最後一個:SSwitch轉換:%d\n", ((COrder*)pParam)->RepeatData.SSwitch.back());
#endif
					((COrder*)pParam)->RepeatData.StepRepeatCountX.back() = _ttol(CommandResolve(Command, 3));
					((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y = ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y + _ttol(CommandResolve(Command, 2));
					((COrder*)pParam)->RepeatData.StepRepeatCountY.back()--;
				}
				else
				{
#ifdef PRINTF
					_cwprintf(L"BlockProcessStartX()::刪除陣列\n");
#endif
					DeleteStepRepeat = TRUE;
					((COrder*)pParam)->RepeatData.StepRepeatBlockData.pop_back();
					((COrder*)pParam)->RepeatData.SSwitch.pop_back();
					((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X = ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.back();
					((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y = ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.back();
					((COrder*)pParam)->RepeatData.StepRepeatNum.pop_back();
					((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.pop_back();
					((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.pop_back();
					((COrder*)pParam)->RepeatData.StepRepeatCountX.pop_back();
					((COrder*)pParam)->RepeatData.StepRepeatCountY.pop_back();
					((COrder*)pParam)->RepeatData.StepRepeatTotalX.pop_back();//只有用在建立修正表時
					((COrder*)pParam)->RepeatData.StepRepeatTotalY.pop_back();//只有用在建立修正表時
					((COrder*)pParam)->RepeatData.StepRepeatIntervel.pop_back();
					//判斷是否為最後一個StepRepeat,如果是清除刪除、新增計數
					if (!((COrder*)pParam)->RepeatData.StepRepeatNum.size())
					{
						((COrder*)pParam)->RepeatData.AllNewStepRepeatNum = 0;
						((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum = 0;
#ifdef PRINTF
						_cwprintf(L"BlockProcessStartX()::新增、刪除總數:%d,%d\n", ((COrder*)pParam)->RepeatData.AllNewStepRepeatNum, ((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum);
#endif 
					}
				}  
			}
		}
		//N型
		if (_ttol(CommandResolve(Command, 5)) == 2)
		{
			//N型 阻斷編號
			BlockBuff.Format(_T("%d-%d"), (_ttol(CommandResolve(Command, 3)) + 1 - ((COrder*)pParam)->RepeatData.StepRepeatCountX.back()),
				(_ttol(CommandResolve(Command, 4)) + 1 - ((COrder*)pParam)->RepeatData.StepRepeatCountY.back()));
			if (((COrder*)pParam)->RepeatData.StepRepeatBlockData.back().BlockPosition.at(i) == BlockBuff)
			{
#ifdef PRINTF
				_cwprintf(L"BlockProcessStartX()::有阻斷\n");
#endif
				if (((COrder*)pParam)->RepeatData.StepRepeatCountX.back() > 1)
				{
					((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X = ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X + _ttol(CommandResolve(Command, 1));
					((COrder*)pParam)->RepeatData.StepRepeatCountX.back()--;
				}
				else if (((COrder*)pParam)->RepeatData.StepRepeatCountY.back() > 1)
				{
					//N型X回最初位置
					((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X = ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.back();
					((COrder*)pParam)->RepeatData.StepRepeatCountX.back() = _ttol(CommandResolve(Command, 3));
					((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y = ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y + _ttol(CommandResolve(Command, 2));
					((COrder*)pParam)->RepeatData.StepRepeatCountY.back()--;
				}
				else
				{
#ifdef PRINTF
					_cwprintf(L"BlockProcessStartX()::刪除陣列\n");
#endif
					DeleteStepRepeat = TRUE;
					((COrder*)pParam)->RepeatData.StepRepeatBlockData.pop_back();
					((COrder*)pParam)->RepeatData.SSwitch.pop_back();
					((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X = ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.back();
					((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y = ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.back();
					((COrder*)pParam)->RepeatData.StepRepeatNum.pop_back();
					((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.pop_back();
					((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.pop_back();
					((COrder*)pParam)->RepeatData.StepRepeatCountX.pop_back();
					((COrder*)pParam)->RepeatData.StepRepeatCountY.pop_back();
					((COrder*)pParam)->RepeatData.StepRepeatTotalX.pop_back();//只有用在建立修正表時
					((COrder*)pParam)->RepeatData.StepRepeatTotalY.pop_back();//只有用在建立修正表時
					((COrder*)pParam)->RepeatData.StepRepeatIntervel.pop_back();
					//判斷是否為最後一個StepRepeat,如果是清除刪除、新增計數
					if (!((COrder*)pParam)->RepeatData.StepRepeatNum.size())
					{
						((COrder*)pParam)->RepeatData.AllNewStepRepeatNum = 0;
						((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum = 0;
#ifdef PRINTF
						_cwprintf(L"BlockProcessStartX()::新增、刪除總數:%d,%d\n", ((COrder*)pParam)->RepeatData.AllNewStepRepeatNum, ((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum);
#endif 
					}
				}
			}
		}
	}
	if (!RepeatStatus)
	{
		if (DeleteStepRepeat)
		{
			((COrder*)pParam)->RepeatData.StepRepeatLabel = _T("");
			((COrder*)pParam)->RepeatData.StepRepeatLabelLock = FALSE;
			((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum++;
		}
		else
		{
			((COrder*)pParam)->RepeatData.StepRepeatLabel = CommandResolve(Command, 6);
		}
	}  
	else
	{
		if (((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum != 0 && ((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum != ((COrder*)pParam)->RepeatData.AllNewStepRepeatNum)
		{
#ifdef PRINTF
			_cwprintf(L"BlockProcessStartX()::進入新增內層迴圈\n");
#endif
			((COrder*)pParam)->RepeatData.StepRepeatLabelLock = TRUE;
			((COrder*)pParam)->RepeatData.AddInStepRepeatSwitch = TRUE;
			((COrder*)pParam)->RepeatData.StepRepeatLabel = CommandResolve(Command, 6);
			((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum--;
		}
		else
		{
			if (DeleteStepRepeat)
			{
				((COrder*)pParam)->RepeatData.StepRepeatLabel = _T("");
				((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum++;
			}
			else
			{
				((COrder*)pParam)->RepeatData.StepRepeatLabel = _T("StepRepeatLabel,") + CommandResolve(Command, 6);
			}  
		}
	}
}
/*阻斷處理方式(執行中StepRepeatX時)
*NowCount:StepRepeat要檢查阻斷的陣列編號
*/
BOOL COrder::BlockProcessExecuteX(CString Command, LPVOID pParam, int NowCount)
{
	CString BlockBuff;
	int BlockSize = ((COrder*)pParam)->RepeatData.StepRepeatBlockData.at(NowCount).BlockPosition.size();
	for (int i = 0; i < BlockSize; i++)//搜尋最後一項阻斷陣列 是否有1-1
	{
#ifdef PRINTF
		_cwprintf(L"BlockProcessExecuteX()::第%d:%d-%d.StepRepeatX 檢查阻斷\n", NowCount,((COrder*)pParam)->RepeatData.StepRepeatCountX.at(NowCount), ((COrder*)pParam)->RepeatData.StepRepeatCountY.at(NowCount));
#endif
		//S型
		if (_ttol(CommandResolve(Command, 5)) == 1)
		{
			//S型 阻斷編號
			if (!((COrder*)pParam)->RepeatData.SSwitch.at(NowCount))
			{
				BlockBuff.Format(_T("%d-%d"), ((COrder*)pParam)->RepeatData.StepRepeatCountX.at(NowCount),
					(_ttol(CommandResolve(Command, 4)) + 1 - ((COrder*)pParam)->RepeatData.StepRepeatCountY.at(NowCount)));
#ifdef PRINTF
				_cwprintf(L"BlockProcessExecuteX()::第%d:標籤只有Y變X正常:%s\n", NowCount, BlockBuff);
#endif
			}
			else
			{ 
				BlockBuff.Format(_T("%d-%d"), (_ttol(CommandResolve(Command, 3)) + 1 - ((COrder*)pParam)->RepeatData.StepRepeatCountX.at(NowCount)),
					(_ttol(CommandResolve(Command, 4)) + 1 - ((COrder*)pParam)->RepeatData.StepRepeatCountY.at(NowCount)));
#ifdef PRINTF
				_cwprintf(L"BlockProcessExecuteX()::第%d:標籤XY變:%s\n", NowCount, BlockBuff);
#endif
			}
			if (((COrder*)pParam)->RepeatData.StepRepeatBlockData.at(NowCount).BlockPosition.at(i) == BlockBuff)
			{
#ifdef PRINTF
				_cwprintf(L"BlockProcessExecuteX()::第%d:有阻斷\n", NowCount);
#endif
				if (((COrder*)pParam)->RepeatData.StepRepeatCountX.at(NowCount) > 1)
				{
					if (((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X == ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.at(NowCount) + (_ttol(CommandResolve(Command, 1))*(_ttol(CommandResolve(Command, 3)) - 1)))
					{
						((COrder*)pParam)->RepeatData.SSwitch.at(NowCount) = FALSE;
					}
					else if (((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X == ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.at(NowCount))
					{
						((COrder*)pParam)->RepeatData.SSwitch.at(NowCount) = TRUE;
					}

					if (!((COrder*)pParam)->RepeatData.SSwitch.at(NowCount))
					{
						((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X = ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X - _ttol(CommandResolve(Command, 1));
					}
					else
					{
						((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X = ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X + _ttol(CommandResolve(Command, 1));
					}
					((COrder*)pParam)->RepeatData.StepRepeatCountX.at(NowCount)--;
					((COrder*)pParam)->RepeatData.StepRepeatLabel = _T("StepRepeatLabel,") + CommandResolve(Command, 6);
#ifdef PRINTF
					_cwprintf(L"BlockProcessExecuteX()::第%d:執行%d-%d\n", NowCount, ((COrder*)pParam)->RepeatData.StepRepeatCountX.at(NowCount), ((COrder*)pParam)->RepeatData.StepRepeatCountY.at(NowCount));
#endif
				}
				else if (((COrder*)pParam)->RepeatData.StepRepeatCountY.at(NowCount) > 1)
				{
					((COrder*)pParam)->RepeatData.SSwitch.at(NowCount) = !((COrder*)pParam)->RepeatData.SSwitch.at(NowCount);
#ifdef PRINTF
					_cwprintf(L"BlockProcessExecuteX()::第%d:SSwitch轉換:%d\n", i, ((COrder*)pParam)->RepeatData.SSwitch.at(NowCount));
#endif
					((COrder*)pParam)->RepeatData.StepRepeatCountX.at(NowCount) = _ttol(CommandResolve(Command, 3));
					((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y = ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y + _ttol(CommandResolve(Command, 2));
					((COrder*)pParam)->RepeatData.StepRepeatCountY.at(NowCount)--;
					((COrder*)pParam)->RepeatData.StepRepeatLabel = _T("StepRepeatLabel,") + CommandResolve(Command, 6);
#ifdef PRINTF
					_cwprintf(L"BlockProcessExecuteX()::第%d:執行%d-%d\n", NowCount, ((COrder*)pParam)->RepeatData.StepRepeatCountX.at(NowCount), ((COrder*)pParam)->RepeatData.StepRepeatCountY.at(NowCount));
#endif
				}
				else
				{
#ifdef PRINTF
					_cwprintf(L"BlockProcessExecuteX()::第%d:刪除所有陣列\n", NowCount);
#endif
					((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum++;
#ifdef PRINTF
					_cwprintf(L"BlockProcessExecuteX()::刪除總數+1=%d\n", ((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum);
#endif
					((COrder*)pParam)->RepeatData.StepRepeatBlockData.erase(((COrder*)pParam)->RepeatData.StepRepeatBlockData.begin() + NowCount);
					((COrder*)pParam)->RepeatData.SSwitch.erase(((COrder*)pParam)->RepeatData.SSwitch.begin() + NowCount);
					((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X = ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.at(NowCount);
					((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y = ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.at(NowCount);
					((COrder*)pParam)->RepeatData.StepRepeatNum.erase(((COrder*)pParam)->RepeatData.StepRepeatNum.begin() + NowCount);
					((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.erase(((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.begin() + NowCount);
					((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.erase(((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.begin() + NowCount);
					((COrder*)pParam)->RepeatData.StepRepeatCountX.erase(((COrder*)pParam)->RepeatData.StepRepeatCountX.begin() + NowCount);
					((COrder*)pParam)->RepeatData.StepRepeatCountY.erase(((COrder*)pParam)->RepeatData.StepRepeatCountY.begin() + NowCount);
					((COrder*)pParam)->RepeatData.StepRepeatTotalX.erase(((COrder*)pParam)->RepeatData.StepRepeatTotalX.begin() + NowCount);//只有用在建立修正表時
					((COrder*)pParam)->RepeatData.StepRepeatTotalY.erase(((COrder*)pParam)->RepeatData.StepRepeatTotalY.begin() + NowCount);//只有用在建立修正表時
					((COrder*)pParam)->RepeatData.StepRepeatIntervel.erase(((COrder*)pParam)->RepeatData.StepRepeatIntervel.begin() + NowCount);
					//判斷是否為最後一個StepRepeat,如果是清除刪除、新增計數
					if (!((COrder*)pParam)->RepeatData.StepRepeatNum.size())
					{
						((COrder*)pParam)->RepeatData.AllNewStepRepeatNum = 0;
						((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum = 0;
#ifdef PRINTF
						_cwprintf(L"BlockProcessExecuteX()::新增、刪除總數:%d,%d\n", ((COrder*)pParam)->RepeatData.AllNewStepRepeatNum, ((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum);
#endif 
					}
					((COrder*)pParam)->RepeatData.StepRepeatLabel = _T("");
#ifdef PRINTF
					_cwprintf(L"BlockProcessExecuteX()::第%d:刪除成功\n", NowCount);
#endif
					return FALSE;
				}
			}
		}
		//N型
		if (_ttol(CommandResolve(Command, 5)) == 2)
		{
			//N型 阻斷編號
			BlockBuff.Format(_T("%d-%d"), (_ttol(CommandResolve(Command, 3)) + 1 - ((COrder*)pParam)->RepeatData.StepRepeatCountX.at(NowCount)),
				(_ttol(CommandResolve(Command, 4)) + 1 - ((COrder*)pParam)->RepeatData.StepRepeatCountY.at(NowCount)));
#ifdef PRINTF
			_cwprintf(L"BlockProcessExecuteX():第%d::標籤:%s\n", NowCount, BlockBuff);
#endif
			if (((COrder*)pParam)->RepeatData.StepRepeatBlockData.at(NowCount).BlockPosition.at(i) == BlockBuff)
			{
#ifdef PRINTF
				_cwprintf(L"BlockProcessExecuteX():第%d::有阻斷\n", NowCount);
#endif
				if (((COrder*)pParam)->RepeatData.StepRepeatCountX.at(NowCount) > 1)
				{
					((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X = ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X + _ttol(CommandResolve(Command, 1));
					((COrder*)pParam)->RepeatData.StepRepeatCountX.at(NowCount)--;
					((COrder*)pParam)->RepeatData.StepRepeatLabel = _T("StepRepeatLabel,") + CommandResolve(Command, 6);
#ifdef PRINTF
					_cwprintf(L"BlockProcessExecuteX()::第%d:執行%d-%d\n", NowCount, ((COrder*)pParam)->RepeatData.StepRepeatCountX.at(NowCount), ((COrder*)pParam)->RepeatData.StepRepeatCountY.at(NowCount));
#endif
				}
				else if (((COrder*)pParam)->RepeatData.StepRepeatCountY.at(NowCount) > 1)
				{
					//N型X回最初位置
					((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X = ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.at(NowCount);
					((COrder*)pParam)->RepeatData.StepRepeatCountX.at(NowCount) = _ttol(CommandResolve(Command, 3));
					((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y = ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y + _ttol(CommandResolve(Command, 2));
					((COrder*)pParam)->RepeatData.StepRepeatCountY.at(NowCount)--;
					((COrder*)pParam)->RepeatData.StepRepeatLabel = _T("StepRepeatLabel,") + CommandResolve(Command, 6);
#ifdef PRINTF
					_cwprintf(L"BlockProcessExecuteX()::第%d:執行%d-%d\n", NowCount, ((COrder*)pParam)->RepeatData.StepRepeatCountX.at(NowCount), ((COrder*)pParam)->RepeatData.StepRepeatCountY.at(NowCount));
#endif
				}
				else
				{
#ifdef PRINTF
					_cwprintf(L"BlockProcessExecuteX()::第%d:刪除所有陣列\n", NowCount);
#endif
					((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum++;
#ifdef PRINTF
					_cwprintf(L"BlockProcessExecuteX()::刪除總數+1=%d\n", ((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum);
#endif
					((COrder*)pParam)->RepeatData.StepRepeatBlockData.erase(((COrder*)pParam)->RepeatData.StepRepeatBlockData.begin() + NowCount);
					((COrder*)pParam)->RepeatData.SSwitch.erase(((COrder*)pParam)->RepeatData.SSwitch.begin() + NowCount);
					((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X = ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.at(NowCount);
					((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y = ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.at(NowCount);
					((COrder*)pParam)->RepeatData.StepRepeatNum.erase(((COrder*)pParam)->RepeatData.StepRepeatNum.begin() + NowCount);
					((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.erase(((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.begin() + NowCount);
					((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.erase(((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.begin() + NowCount);
					((COrder*)pParam)->RepeatData.StepRepeatCountX.erase(((COrder*)pParam)->RepeatData.StepRepeatCountX.begin() + NowCount);
					((COrder*)pParam)->RepeatData.StepRepeatCountY.erase(((COrder*)pParam)->RepeatData.StepRepeatCountY.begin() + NowCount);
					((COrder*)pParam)->RepeatData.StepRepeatTotalX.erase(((COrder*)pParam)->RepeatData.StepRepeatTotalX.begin() + NowCount);//只有用在建立修正表時
					((COrder*)pParam)->RepeatData.StepRepeatTotalY.erase(((COrder*)pParam)->RepeatData.StepRepeatTotalY.begin() + NowCount);//只有用在建立修正表時
					((COrder*)pParam)->RepeatData.StepRepeatIntervel.erase(((COrder*)pParam)->RepeatData.StepRepeatIntervel.begin() + NowCount);
					//判斷是否為最後一個StepRepeat,如果是清除刪除、新增計數
					if (!((COrder*)pParam)->RepeatData.StepRepeatNum.size())
					{
						((COrder*)pParam)->RepeatData.AllNewStepRepeatNum = 0;
						((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum = 0;
#ifdef PRINTF
						_cwprintf(L"BlockProcessExecuteX()::新增、刪除總數:%d,%d\n", ((COrder*)pParam)->RepeatData.AllNewStepRepeatNum, ((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum);
#endif 
					}
					((COrder*)pParam)->RepeatData.StepRepeatLabel = _T("");
#ifdef PRINTF
					_cwprintf(L"BlockProcessExecuteX()::第%d:刪除成功\n", NowCount);
#endif
					return FALSE;
				}
			}
		}
	}
#ifdef PRINTF
	_cwprintf(L"BlockProcessExecute()::結束阻斷\n"); 
#endif
	return TRUE;
}
/*阻斷處理方式(加入StepRepeatY時)
*RepeatStatus:判斷要檢查阻斷時是否相同StepRepeatLabel
*/
void COrder::BlockProcessStartY(CString Command, LPVOID pParam, BOOL RepeatStatus)
{
	CString BlockBuff;
	BOOL DeleteStepRepeat = FALSE;
	int StepRepeatBlockSize = ((COrder*)pParam)->RepeatData.StepRepeatBlockData.back().BlockPosition.size();
	for (int i = 0; i < StepRepeatBlockSize; i++)//搜尋第一項阻斷陣列 是否有1-1
	{
#ifdef PRINTF
		_cwprintf(_T("BlockProcessStartY()::%d-%d.StepRepeatY 檢查阻斷\n"), ((COrder*)pParam)->RepeatData.StepRepeatCountX.back(), ((COrder*)pParam)->RepeatData.StepRepeatCountY.back());
#endif
		//S型
		if (_ttol(CommandResolve(Command, 5)) == 1)
		{
			//S型 阻斷編號
			if (!((COrder*)pParam)->RepeatData.SSwitch.back())
			{
				BlockBuff.Format(_T("%d-%d"), (_ttol(CommandResolve(Command, 3)) + 1 - ((COrder*)pParam)->RepeatData.StepRepeatCountX.back()),
					((COrder*)pParam)->RepeatData.StepRepeatCountY.back());/**/
#ifdef PRINTF
				_cwprintf(L"BlockProcessStartY()::標籤只有X變Y正常:%s\n", BlockBuff);
#endif
			}
			else
			{
				BlockBuff.Format(_T("%d-%d"), (_ttol(CommandResolve(Command, 3)) + 1 - ((COrder*)pParam)->RepeatData.StepRepeatCountX.back()),
					(_ttol(CommandResolve(Command, 4)) + 1 - ((COrder*)pParam)->RepeatData.StepRepeatCountY.back()));
#ifdef PRINTF
				_cwprintf(L"BlockProcessStartY()::標籤XY變:%s\n", BlockBuff);
#endif
			}
			if (((COrder*)pParam)->RepeatData.StepRepeatBlockData.back().BlockPosition.at(i) == BlockBuff)
			{
#ifdef PRINTF
				_cwprintf(L"BlockProcessStartY()::有阻斷\n");
#endif
				if (((COrder*)pParam)->RepeatData.StepRepeatCountY.back() > 1)/**/
				{
					if (((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y == ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.back() + (_ttol(CommandResolve(Command, 2))*(_ttol(CommandResolve(Command, 4)) - 1)))/**/
					{
						((COrder*)pParam)->RepeatData.SSwitch.back() = FALSE;
#ifdef PRINTF
						_cwprintf(L"BlockProcessStartY()::最後一個:SSwitch轉換:%d\n",((COrder*)pParam)->RepeatData.SSwitch.back());
#endif
					}
					else  if (((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y == ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.back())
					{
						((COrder*)pParam)->RepeatData.SSwitch.back() = TRUE;
#ifdef PRINTF
						_cwprintf(L"BlockProcessStartY()::最後一個:SSwitch轉換:%d\n",((COrder*)pParam)->RepeatData.SSwitch.back());
#endif
					}
					if (!((COrder*)pParam)->RepeatData.SSwitch.back())
					{
						/**/((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y = ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y - _ttol(CommandResolve(Command, 2));
					}
					else
					{
						/**/((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y = ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y + _ttol(CommandResolve(Command, 2));
					}
					((COrder*)pParam)->RepeatData.StepRepeatCountY.back()--;
				}
				else if (((COrder*)pParam)->RepeatData.StepRepeatCountX.back() > 1)/**/
				{
					((COrder*)pParam)->RepeatData.SSwitch.back() = !((COrder*)pParam)->RepeatData.SSwitch.back();
#ifdef PRINTF
					_cwprintf(L"BlockProcessStartY()::最後一個:SSwitch轉換:%d\n", ((COrder*)pParam)->RepeatData.SSwitch.back());
#endif
					/**/((COrder*)pParam)->RepeatData.StepRepeatCountY.back() = _ttol(CommandResolve(Command, 4));
					/**/((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X = ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X + _ttol(CommandResolve(Command, 1));
					/**/((COrder*)pParam)->RepeatData.StepRepeatCountX.back()--;
				}
				else
				{
#ifdef PRINTF
					_cwprintf(L"BlockProcessStartY()::刪除陣列\n");
#endif
					DeleteStepRepeat = TRUE;
					((COrder*)pParam)->RepeatData.StepRepeatBlockData.pop_back();
					((COrder*)pParam)->RepeatData.SSwitch.pop_back();
					((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X = ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.back();
					((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y = ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.back();
					((COrder*)pParam)->RepeatData.StepRepeatNum.pop_back();
					((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.pop_back();
					((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.pop_back();
					((COrder*)pParam)->RepeatData.StepRepeatCountX.pop_back();
					((COrder*)pParam)->RepeatData.StepRepeatCountY.pop_back();
					((COrder*)pParam)->RepeatData.StepRepeatTotalX.pop_back();//只有用在建立修正表時
					((COrder*)pParam)->RepeatData.StepRepeatTotalY.pop_back();//只有用在建立修正表時
					((COrder*)pParam)->RepeatData.StepRepeatIntervel.pop_back();
					//判斷是否為最後一個StepRepeat,如果是清除刪除、新增計數
					if (!((COrder*)pParam)->RepeatData.StepRepeatNum.size())
					{
						((COrder*)pParam)->RepeatData.AllNewStepRepeatNum = 0;
						((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum = 0;
#ifdef PRINTF
						_cwprintf(L"BlockProcessStartY()::新增、刪除總數:%d,%d\n", ((COrder*)pParam)->RepeatData.AllNewStepRepeatNum, ((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum);
#endif 
					}
				}
			}
		}
		//N型
		if (_ttol(CommandResolve(Command, 5)) == 2)
		{
			//N型 阻斷編號
			BlockBuff.Format(_T("%d-%d"), (_ttol(CommandResolve(Command, 3)) + 1 - ((COrder*)pParam)->RepeatData.StepRepeatCountX.back()),
				(_ttol(CommandResolve(Command, 4)) + 1 - ((COrder*)pParam)->RepeatData.StepRepeatCountY.back()));
			if (((COrder*)pParam)->RepeatData.StepRepeatBlockData.back().BlockPosition.at(i) == BlockBuff)
			{
#ifdef PRINTF
				_cwprintf(L"BlockProcessStartY()::有阻斷\n");
#endif
				if (((COrder*)pParam)->RepeatData.StepRepeatCountY.back() > 1)/**/
				{
					/**/((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y = ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y + _ttol(CommandResolve(Command, 2));
					/**/((COrder*)pParam)->RepeatData.StepRepeatCountY.back()--;
				}
				else if (((COrder*)pParam)->RepeatData.StepRepeatCountX.back() > 1)/**/
				{
					//N型X回最初位置
					/**/((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y = ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.back();
					/**/((COrder*)pParam)->RepeatData.StepRepeatCountY.back() = _ttol(CommandResolve(Command, 4));
					/**/((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X = ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X + _ttol(CommandResolve(Command, 1));
					/**/((COrder*)pParam)->RepeatData.StepRepeatCountX.back()--;
				}
				else
				{
#ifdef PRINTF
					_cwprintf(L"BlockProcessStartY()::刪除陣列\n");
#endif
					DeleteStepRepeat = TRUE;
					((COrder*)pParam)->RepeatData.StepRepeatBlockData.pop_back();
					((COrder*)pParam)->RepeatData.SSwitch.pop_back();
					((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X = ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.back();
					((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y = ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.back();
					((COrder*)pParam)->RepeatData.StepRepeatNum.pop_back();
					((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.pop_back();
					((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.pop_back();
					((COrder*)pParam)->RepeatData.StepRepeatCountX.pop_back();
					((COrder*)pParam)->RepeatData.StepRepeatCountY.pop_back();
					((COrder*)pParam)->RepeatData.StepRepeatTotalX.pop_back();//只有用在建立修正表時
					((COrder*)pParam)->RepeatData.StepRepeatTotalY.pop_back();//只有用在建立修正表時
					((COrder*)pParam)->RepeatData.StepRepeatIntervel.pop_back();
					//判斷是否為最後一個StepRepeat,如果是清除刪除、新增計數
					if (!((COrder*)pParam)->RepeatData.StepRepeatNum.size())
					{
						((COrder*)pParam)->RepeatData.AllNewStepRepeatNum = 0;
						((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum = 0;
#ifdef PRINTF
						_cwprintf(L"BlockProcessStartY()::新增、刪除總數:%d,%d\n", ((COrder*)pParam)->RepeatData.AllNewStepRepeatNum, ((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum);
#endif 
					}
				}
			}
		}
	}
	if (!RepeatStatus)
	{
		if (DeleteStepRepeat)
		{
			((COrder*)pParam)->RepeatData.StepRepeatLabel = _T("");
			((COrder*)pParam)->RepeatData.StepRepeatLabelLock = FALSE;
			((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum++;
		}
		else
		{
			((COrder*)pParam)->RepeatData.StepRepeatLabel = CommandResolve(Command, 6);
		}
	}
	else
	{
		if (((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum != 0 && ((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum != ((COrder*)pParam)->RepeatData.AllNewStepRepeatNum)
		{
#ifdef PRINTF
			_cwprintf(L"BlockProcessStartY()::進入新增內層迴圈\n");
#endif
			((COrder*)pParam)->RepeatData.StepRepeatLabelLock = TRUE;
			((COrder*)pParam)->RepeatData.AddInStepRepeatSwitch = TRUE;
			((COrder*)pParam)->RepeatData.StepRepeatLabel = CommandResolve(Command, 6);
			((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum--;
		}
		else
		{
			if (DeleteStepRepeat)
			{
				((COrder*)pParam)->RepeatData.StepRepeatLabel = _T("");
				((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum++;
			}
			else
			{
				((COrder*)pParam)->RepeatData.StepRepeatLabel = _T("StepRepeatLabel,") + CommandResolve(Command, 6);
			}
		}
	}
}
/*阻斷處理方式(執行中StepRepeatY時)
*NowCount:StepRepeat要檢查阻斷的陣列編號
*/
BOOL COrder::BlockProcessExecuteY(CString Command, LPVOID pParam, int NowCount)
{
	CString BlockBuff;
	int BlockSize = ((COrder*)pParam)->RepeatData.StepRepeatBlockData.at(NowCount).BlockPosition.size();
	for (int i = 0; i < BlockSize; i++)//搜尋最後一項阻斷陣列 是否有1-1
	{
#ifdef PRINTF
		_cwprintf(L"BlockProcessExecuteY()::第%d:%d-%d.StepRepeatY 檢查阻斷\n", NowCount, ((COrder*)pParam)->RepeatData.StepRepeatCountX.at(NowCount), ((COrder*)pParam)->RepeatData.StepRepeatCountY.at(NowCount));
#endif
		//S型
		if (_ttol(CommandResolve(Command, 5)) == 1)
		{
			//S型 阻斷編號
			if (!((COrder*)pParam)->RepeatData.SSwitch.at(NowCount))
			{
				BlockBuff.Format(_T("%d-%d"), (_ttol(CommandResolve(Command, 3)) + 1 - ((COrder*)pParam)->RepeatData.StepRepeatCountX.at(NowCount)),
					((COrder*)pParam)->RepeatData.StepRepeatCountY.at(NowCount));/**/
#ifdef PRINTF
				_cwprintf(L"BlockProcessExecuteY()::第%d:標籤只有Y變X正常:%s\n", NowCount, BlockBuff);
#endif
			}
			else
			{
				BlockBuff.Format(_T("%d-%d"), (_ttol(CommandResolve(Command, 3)) + 1 - ((COrder*)pParam)->RepeatData.StepRepeatCountX.at(NowCount)),
					(_ttol(CommandResolve(Command, 4)) + 1 - ((COrder*)pParam)->RepeatData.StepRepeatCountY.at(NowCount)));
#ifdef PRINTF
				_cwprintf(L"BlockProcessExecuteY()::第%d:標籤XY變:%s\n", NowCount, BlockBuff);
#endif
			}
			if (((COrder*)pParam)->RepeatData.StepRepeatBlockData.at(NowCount).BlockPosition.at(i) == BlockBuff)
			{
#ifdef PRINTF
				_cwprintf(L"BlockProcessExecuteY()::第%d:有阻斷\n", NowCount);
#endif
				if (((COrder*)pParam)->RepeatData.StepRepeatCountY.at(NowCount) > 1)/**/
				{
					if (((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y == ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.at(NowCount) + (_ttol(CommandResolve(Command, 2))*(_ttol(CommandResolve(Command, 4)) - 1)))/**/
					{
						((COrder*)pParam)->RepeatData.SSwitch.at(NowCount) = FALSE;
					}
					else if (((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y == ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.at(NowCount))/**/
					{
						((COrder*)pParam)->RepeatData.SSwitch.at(NowCount) = TRUE;
					}

					if (!((COrder*)pParam)->RepeatData.SSwitch.at(NowCount))
					{
						/**/((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y = ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y - _ttol(CommandResolve(Command, 2));
					}
					else
					{
						/**/((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y = ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y + _ttol(CommandResolve(Command, 2));
					}
					/**/((COrder*)pParam)->RepeatData.StepRepeatCountY.at(NowCount)--;
					((COrder*)pParam)->RepeatData.StepRepeatLabel = _T("StepRepeatLabel,") + CommandResolve(Command, 6);
#ifdef PRINTF
					_cwprintf(L"BlockProcessExecuteY()::第%d:執行%d-%d\n", NowCount, ((COrder*)pParam)->RepeatData.StepRepeatCountX.at(NowCount), ((COrder*)pParam)->RepeatData.StepRepeatCountY.at(NowCount));
#endif
				}
				else if (((COrder*)pParam)->RepeatData.StepRepeatCountX.at(NowCount) > 1)
				{
					((COrder*)pParam)->RepeatData.SSwitch.at(NowCount) = !((COrder*)pParam)->RepeatData.SSwitch.at(NowCount);
#ifdef PRINTF
					_cwprintf(L"BlockProcessExecuteY()::第%d:SSwitch轉換:%d\n", i, ((COrder*)pParam)->RepeatData.SSwitch.at(NowCount));
#endif
					/**/((COrder*)pParam)->RepeatData.StepRepeatCountY.at(NowCount) = _ttol(CommandResolve(Command, 4));
					/**/((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X = ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X + _ttol(CommandResolve(Command, 1));
					/**/((COrder*)pParam)->RepeatData.StepRepeatCountX.at(NowCount)--;
					((COrder*)pParam)->RepeatData.StepRepeatLabel = _T("StepRepeatLabel,") + CommandResolve(Command, 6);
#ifdef PRINTF
					_cwprintf(L"BlockProcessExecuteY()::第%d:執行%d-%d\n", NowCount, ((COrder*)pParam)->RepeatData.StepRepeatCountX.at(NowCount), ((COrder*)pParam)->RepeatData.StepRepeatCountY.at(NowCount));
#endif
				}
				else
				{
#ifdef PRINTF
					_cwprintf(L"BlockProcessExecuteY()::第%d:刪除所有陣列\n", NowCount);
#endif
					((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum++;
#ifdef PRINTF
					_cwprintf(L"BlockProcessExecuteY():刪除總數+1=%d\n", ((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum);
#endif
					((COrder*)pParam)->RepeatData.StepRepeatBlockData.erase(((COrder*)pParam)->RepeatData.StepRepeatBlockData.begin() + NowCount);
					((COrder*)pParam)->RepeatData.SSwitch.erase(((COrder*)pParam)->RepeatData.SSwitch.begin() + NowCount);
					((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X = ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.at(NowCount);
					((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y = ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.at(NowCount);
					((COrder*)pParam)->RepeatData.StepRepeatNum.erase(((COrder*)pParam)->RepeatData.StepRepeatNum.begin() + NowCount);
					((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.erase(((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.begin() + NowCount);
					((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.erase(((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.begin() + NowCount);
					((COrder*)pParam)->RepeatData.StepRepeatCountX.erase(((COrder*)pParam)->RepeatData.StepRepeatCountX.begin() + NowCount);
					((COrder*)pParam)->RepeatData.StepRepeatCountY.erase(((COrder*)pParam)->RepeatData.StepRepeatCountY.begin() + NowCount);
					((COrder*)pParam)->RepeatData.StepRepeatTotalX.erase(((COrder*)pParam)->RepeatData.StepRepeatTotalX.begin() + NowCount);//只有用在建立修正表時
					((COrder*)pParam)->RepeatData.StepRepeatTotalY.erase(((COrder*)pParam)->RepeatData.StepRepeatTotalY.begin() + NowCount);//只有用在建立修正表時
					((COrder*)pParam)->RepeatData.StepRepeatIntervel.erase(((COrder*)pParam)->RepeatData.StepRepeatIntervel.begin() + NowCount);
					//判斷是否為最後一個StepRepeat,如果是清除刪除、新增計數
					if (!((COrder*)pParam)->RepeatData.StepRepeatNum.size())
					{
						((COrder*)pParam)->RepeatData.AllNewStepRepeatNum = 0;
						((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum = 0;
#ifdef PRINTF
						_cwprintf(L"BlockProcessExecuteY()::新增、刪除總數:%d,%d\n", ((COrder*)pParam)->RepeatData.AllNewStepRepeatNum, ((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum);
#endif 
					}
					((COrder*)pParam)->RepeatData.StepRepeatLabel = _T("");
#ifdef PRINTF
					_cwprintf(L"BlockProcessExecuteY()::第%d:刪除成功\n", NowCount);
#endif
					return FALSE;
				}
			}
		}
		//N型
		if (_ttol(CommandResolve(Command, 5)) == 2)
		{
			//N型 阻斷編號
			BlockBuff.Format(_T("%d-%d"), (_ttol(CommandResolve(Command, 3)) + 1 - ((COrder*)pParam)->RepeatData.StepRepeatCountX.at(NowCount)),
				(_ttol(CommandResolve(Command, 4)) + 1 - ((COrder*)pParam)->RepeatData.StepRepeatCountY.at(NowCount)));
#ifdef PRINTF
			_cwprintf(L"BlockProcessExecuteY()::第%d:標籤:%s\n", NowCount, BlockBuff);
#endif
			if (((COrder*)pParam)->RepeatData.StepRepeatBlockData.at(NowCount).BlockPosition.at(i) == BlockBuff)
			{
#ifdef PRINTF
				_cwprintf(L"BlockProcessExecuteY()::第%d:有阻斷\n", NowCount);
#endif
				if (((COrder*)pParam)->RepeatData.StepRepeatCountY.at(NowCount) > 1)/**/
				{
					/**/((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y = ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y + _ttol(CommandResolve(Command, 2));
					/**/((COrder*)pParam)->RepeatData.StepRepeatCountY.at(NowCount)--;
					((COrder*)pParam)->RepeatData.StepRepeatLabel = _T("StepRepeatLabel,") + CommandResolve(Command, 6);
#ifdef PRINTF
					_cwprintf(L"BlockProcessExecuteY()::第%d:執行%d-%d\n", NowCount, ((COrder*)pParam)->RepeatData.StepRepeatCountX.at(NowCount), ((COrder*)pParam)->RepeatData.StepRepeatCountY.at(NowCount));
#endif
				}
				else if (((COrder*)pParam)->RepeatData.StepRepeatCountX.at(NowCount) > 1)/**/
				{
					//N型Y回最初位置
					/**/((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y = ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.at(NowCount);
					/**/((COrder*)pParam)->RepeatData.StepRepeatCountY.at(NowCount) = _ttol(CommandResolve(Command, 4));
					/**/((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X = ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X + _ttol(CommandResolve(Command, 1));
					/**/((COrder*)pParam)->RepeatData.StepRepeatCountX.at(NowCount)--;
					((COrder*)pParam)->RepeatData.StepRepeatLabel = _T("StepRepeatLabel,") + CommandResolve(Command, 6);
#ifdef PRINTF
					_cwprintf(L"BlockProcessExecuteY()::第%d:執行%d-%d\n", NowCount, ((COrder*)pParam)->RepeatData.StepRepeatCountX.at(NowCount), ((COrder*)pParam)->RepeatData.StepRepeatCountY.at(NowCount));
#endif
				}
				else
				{
#ifdef PRINTF
					_cwprintf(L"BlockProcessExecuteY()::第%d:刪除所有陣列\n", NowCount);
#endif
					((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum++;
#ifdef PRINTF
					_cwprintf(L"BlockProcessExecuteY()::刪除總數+1=%d\n", ((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum);
#endif
					((COrder*)pParam)->RepeatData.StepRepeatBlockData.erase(((COrder*)pParam)->RepeatData.StepRepeatBlockData.begin() + NowCount);
					((COrder*)pParam)->RepeatData.SSwitch.erase(((COrder*)pParam)->RepeatData.SSwitch.begin() + NowCount);
					((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X = ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.at(NowCount);
					((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y = ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.at(NowCount);
					((COrder*)pParam)->RepeatData.StepRepeatNum.erase(((COrder*)pParam)->RepeatData.StepRepeatNum.begin() + NowCount);
					((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.erase(((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.begin() + NowCount);
					((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.erase(((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.begin() + NowCount);
					((COrder*)pParam)->RepeatData.StepRepeatCountX.erase(((COrder*)pParam)->RepeatData.StepRepeatCountX.begin() + NowCount);
					((COrder*)pParam)->RepeatData.StepRepeatCountY.erase(((COrder*)pParam)->RepeatData.StepRepeatCountY.begin() + NowCount);
					((COrder*)pParam)->RepeatData.StepRepeatTotalX.erase(((COrder*)pParam)->RepeatData.StepRepeatTotalX.begin() + NowCount);//只有用在建立修正表時
					((COrder*)pParam)->RepeatData.StepRepeatTotalY.erase(((COrder*)pParam)->RepeatData.StepRepeatTotalY.begin() + NowCount);//只有用在建立修正表時
					((COrder*)pParam)->RepeatData.StepRepeatIntervel.erase(((COrder*)pParam)->RepeatData.StepRepeatIntervel.begin() + NowCount);
					//判斷是否為最後一個StepRepeat,如果是清除刪除、新增計數
					if (!((COrder*)pParam)->RepeatData.StepRepeatNum.size())
					{
						((COrder*)pParam)->RepeatData.AllNewStepRepeatNum = 0;
						((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum = 0;
#ifdef PRINTF
						_cwprintf(L"BlockProcessExecuteY()::新增、刪除總數:%d,%d\n", ((COrder*)pParam)->RepeatData.AllNewStepRepeatNum, ((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum);
#endif 
					}
					((COrder*)pParam)->RepeatData.StepRepeatLabel = _T("");
#ifdef PRINTF
					_cwprintf(L"BlockProcessExecuteY()::第%d:刪除成功\n", NowCount);
#endif
					return FALSE;
				}
			}
		}
	}
#ifdef PRINTF
	_cwprintf(L"BlockProcessExecuteY()::結束阻斷\n");
#endif
	return TRUE;
} 
/*阻斷陣列排序
*Type:選擇排序的StepRepeat類型(X or Y)
*mode:選擇排序的StepRepeat路徑模式(S or N)
*/
void COrder::BlockSort(std::vector<CString> &BlockPosition, int Type, int mode)
{
	CString temp;
	if (Type == 1)//StepRepeatX
	{
		for (UINT i = 0; i < BlockPosition.size(); i++)
		{
			for (UINT j = i + 1; j < BlockPosition.size(); j++)
			{
				if (_ttol(BlockResolve(BlockPosition.at(i), 1)) > _ttol(BlockResolve(BlockPosition.at(j), 1)))//前者Y大於後者Y
				{
					//交換資料
					temp = BlockPosition.at(i);
					BlockPosition.at(i) = BlockPosition.at(j);
					BlockPosition.at(j) = temp;
				}
				else if (_ttol(BlockResolve(BlockPosition.at(i), 1)) == _ttol(BlockResolve(BlockPosition.at(j), 1)))//兩者Y等於
				{
					//比較X
					if (mode == 1)//模式1
					{
						if ((_ttol(BlockResolve(BlockPosition.at(i), 1)) % 2))//當基數列
						{
							if (_ttol(BlockResolve(BlockPosition.at(i), 0)) > _ttol(BlockResolve(BlockPosition.at(j), 0)))//前者X大於後者X
							{
								//交換資料
								temp = BlockPosition.at(i);
								BlockPosition.at(i) = BlockPosition.at(j);
								BlockPosition.at(j) = temp;
							}
						}
						else//當偶數列
						{
							if (_ttol(BlockResolve(BlockPosition.at(i), 0)) < _ttol(BlockResolve(BlockPosition.at(j), 0)))//比較前者X小於後者X
							{
								//交換資料
								temp = BlockPosition.at(i);
								BlockPosition.at(i) = BlockPosition.at(j);
								BlockPosition.at(j) = temp;
							}
						}
					}
					else if (mode == 2)//模式2
					{
						if (_ttol(BlockResolve(BlockPosition.at(i), 0)) > _ttol(BlockResolve(BlockPosition.at(j), 0)))//前者X大於後者X
						{
							//交換資料
							temp = BlockPosition.at(i);
							BlockPosition.at(i) = BlockPosition.at(j);
							BlockPosition.at(j) = temp;
						}
					}
				}
			}
		}
	}
	else if (Type == 2)
	{
		for (UINT i = 0; i < BlockPosition.size(); i++)
		{
			for (UINT j = i + 1; j < BlockPosition.size(); j++)
			{
				if (_ttol(BlockResolve(BlockPosition.at(i), 0)) > _ttol(BlockResolve(BlockPosition.at(j), 0)))//前者X大於後者X
				{
					//交換資料
					temp = BlockPosition.at(i);
					BlockPosition.at(i) = BlockPosition.at(j);
					BlockPosition.at(j) = temp;
				}
				else if (_ttol(BlockResolve(BlockPosition.at(i), 0)) == _ttol(BlockResolve(BlockPosition.at(j), 0)))//兩者Y等於
				{
					//比較X
					if (mode == 1)//模式1
					{
						if ((_ttol(BlockResolve(BlockPosition.at(i), 0)) % 2))//當基數列
						{
							if (_ttol(BlockResolve(BlockPosition.at(i), 1)) > _ttol(BlockResolve(BlockPosition.at(j), 1)))//前者X大於後者X
							{
								//交換資料
								temp = BlockPosition.at(i);
								BlockPosition.at(i) = BlockPosition.at(j);
								BlockPosition.at(j) = temp;
							}
						}
						else//當偶數列
						{
							if (_ttol(BlockResolve(BlockPosition.at(i), 1)) < _ttol(BlockResolve(BlockPosition.at(j), 1)))//比較前者X小於後者X
							{
								//交換資料
								temp = BlockPosition.at(i);
								BlockPosition.at(i) = BlockPosition.at(j);
								BlockPosition.at(j) = temp;
							}
						}
					}
					else if (mode == 2)//模式2
					{
						if (_ttol(BlockResolve(BlockPosition.at(i), 1)) > _ttol(BlockResolve(BlockPosition.at(j), 1)))//前者X大於後者X
						{
							//交換資料
							temp = BlockPosition.at(i);
							BlockPosition.at(i) = BlockPosition.at(j);
							BlockPosition.at(j) = temp;
						}
					}
				}

			}
		}
	}
#ifdef PRINTF
	_cwprintf(L"BlockSort()::排序完成後:");
	for (UINT i = 0; i < BlockPosition.size(); i++)
	{
		_cwprintf(L"%s,", BlockPosition.at(i));
	}
	_cwprintf(L"\n");
#endif
}
/*阻斷字串處理*/
CString COrder::BlockResolve(CString String, UINT Choose)
{
	int iLength = String.Find(_T('-'));
	if (iLength <= 0)
	{
		iLength = String.GetLength();
	}
	if (Choose <= 0)
	{
		return String.Left(iLength);
	}
	else
	{
		return BlockResolve(String.Right(String.GetLength() - iLength - 1), --Choose);
	}
}
/**************************************************************************檢測處理區塊*****************************************************************************/
/*模板載入
*Choose:選擇載入的模板(OK or NG)
*ModelNum:模板編號字串
*TemplateCheck:模板檢測結構地址
*/
void COrder::ModelLoad(BOOL Choose, LPVOID pParam, CString ModelNum , TemplateCheck &TemplateCheck)
{
	if (Choose)//載入OK模組
	{
#ifdef PRINTF
		_cwprintf(L"ModelLoad()::載入OK模組\n");
#endif
		std::vector<CString> TempFileName;
		int Count = 0;
		int ModelNumber = _ttol(ModelNumResolve(ModelNum, Count)) - 1;//模組起始編號
		while (ModelNumber >= 0 && ModelNumber < (int)((COrder*)pParam)->VisionFile.AllModelName.size())
		{
			Count++;
			TempFileName.push_back(((COrder*)pParam)->VisionFile.AllModelName.at(ModelNumber));
			ModelNumber = _ttol(ModelNumResolve(ModelNum, Count)) - 1;
		}
		//判斷模板數量
		if (TempFileName.size() > 0)
		{
			//配置模板數量
			TemplateCheck.OKModelCount = TempFileName.size();
			//分配模板記憶體
			TemplateCheck.OKModel = new void*[TempFileName.size()];
			for (UINT i = 0; i < TempFileName.size(); i++)
			{
				TemplateCheck.OKModel[i] = malloc(sizeof(int));
			}
			//檔名陣列創建
			CString *FileName = NULL;
			FileName = new CString[TempFileName.size()];
			for (UINT i = 0; i < TempFileName.size(); i++)
			{
				FileName[i] = TempFileName.at(i);
			}
			//載入模板、配置屬性
#ifdef VI
			VI_LoadMatrixModel(TemplateCheck.OKModel, ((COrder*)pParam)->VisionFile.ModelPath, FileName, TemplateCheck.OKModelCount);//載入模板
			VI_SetMultipleModel(TemplateCheck.OKModel, 1, 1, TemplateCheck.VisionParam.Score, 0, 360, TemplateCheck.OKModelCount);//設定模板參數
#endif
		}   
		else
		{
			TemplateCheck.OKModelCount = 0;
		}
	}
	else//載入NG模組
	{
#ifdef PRINTF
		_cwprintf(L"ModelLoad()::載入NG模組\n");
#endif
		std::vector<CString> TempFileName;
		int Count = 0;
		int ModelNumber = _ttol(ModelNumResolve(ModelNum, Count)) - 1;//模組起始編號
		while (ModelNumber >= 0 && (UINT)ModelNumber < ((COrder*)pParam)->VisionFile.AllModelName.size())
		{
			Count++;
			TempFileName.push_back(((COrder*)pParam)->VisionFile.AllModelName.at(ModelNumber));
			ModelNumber = _ttol(ModelNumResolve(ModelNum, Count)) - 1;
		}
		//判斷模板數量
		if (TempFileName.size() > 0)
		{
			//配置模板數量
			TemplateCheck.NGModelCount = TempFileName.size();
			//分配模板記憶體
			TemplateCheck.NGModel = new void*[TempFileName.size()];
			for (UINT i = 0; i < TempFileName.size(); i++)
			{
				TemplateCheck.NGModel[i] = malloc(sizeof(int));
			}
			//檔名陣列創建
			CString *FileName = NULL;
			FileName = new CString[TempFileName.size()];
			for (UINT i = 0; i < TempFileName.size(); i++)
			{
				FileName[i] = TempFileName.at(i);
			}
			//載入模板、配置屬性
#ifdef VI
			VI_LoadMatrixModel(TemplateCheck.NGModel, ((COrder*)pParam)->VisionFile.ModelPath, FileName, TemplateCheck.NGModelCount);//載入模板
			VI_SetMultipleModel(TemplateCheck.NGModel, 1, 1, TemplateCheck.VisionParam.Score, 0, 360, TemplateCheck.NGModelCount);//設定模板參數
#endif
		} 
		else
		{
			TemplateCheck.NGModelCount = 0;
		}
	}
}
/*檢測資料清除
*Moment:即時資料是否清除
*Interval:區間資料是否清除
*return : 目前無用途
*/
BOOL COrder::ClearCheckData(BOOL Moment,BOOL Interval)
{
	if (Moment == TRUE)
	{
		//清除即時模板檢測資料
		if (CheckSwitch.ImmediateCheck == 1)
		{
#ifdef VI
			//釋放影像
			VI_MatrixModelFree(TemplateChecking.OKModel, TemplateChecking.OKModelCount);
			VI_MatrixModelFree(TemplateChecking.NGModel, TemplateChecking.NGModelCount);
#endif
			//釋放記憶體
			free(TemplateChecking.OKModel);
			free(TemplateChecking.NGModel);
			TemplateChecking = { L"",0,NULL,0,NULL,{ 0,0,0,0,0,0,0,0,0,0,0 } };//初始化模板即時檢測資料
		}
		//清除即時直徑檢測資料
		if (CheckSwitch.ImmediateCheck == 2)
		{
			DiameterChecking = { L"",0,0,0,0 };//初始化直徑即時檢測資料
		}
		CheckSwitch.ImmediateCheck = 0;//關閉即時檢測
	}
	if (Interval == TRUE)
	{
		//清除區間模板檢測資料
		if (IntervalTemplateCheck.size())//判斷模板是否有資料
		{
			for (UINT i = 0; i < IntervalTemplateCheck.size(); i++)//釋放陣列影像、記憶體
			{
				//判斷OK是否有模板
#ifdef VI
				if (IntervalTemplateCheck.at(i).OKModelCount)
				{
					//釋放影像
					VI_MatrixModelFree(IntervalTemplateCheck.at(i).OKModel, IntervalTemplateCheck.at(i).OKModelCount);
					//釋放記憶體
					free(IntervalTemplateCheck.at(i).OKModel);
				}
				//判斷NG是否有模板
				if (IntervalTemplateCheck.at(i).NGModelCount)
				{

					//釋放影像
					VI_MatrixModelFree(IntervalTemplateCheck.at(i).NGModel, IntervalTemplateCheck.at(i).NGModelCount);
					//釋放記憶體
					free(IntervalTemplateCheck.at(i).NGModel);
				}
#endif
			}
			IntervalTemplateCheck.clear();
		}
		//清除區間直徑檢測資料
		if (IntervalDiameterCheck.size())//判斷模板是否有資料
		{
			IntervalDiameterCheck.clear();
		}
	}
	return 0;
}
/*檢查線段訓練------2017/02/16改版後用不到
*主要用於判斷是否有加入換線點，在轉換模式時候
*/
void COrder::LineTrainDataCheck(LPVOID pParam)
{
	if (((COrder*)pParam)->CheckSwitch.Area &&
		(((COrder*)pParam)->CurrentCheckAddress != ((COrder*)pParam)->GetCommandAddress()) &&
		(((COrder*)pParam)->RunData.ActionStatus.at(((COrder*)pParam)->Program.SubroutinCount) == 2))//判斷進入時檢測模式、檢測命令、線段狀態
	{
		for (UINT i = 0; i < ((COrder*)pParam)->IntervalAreaCheck.size(); i++)
		{
			if (((COrder*)pParam)->IntervalAreaCheck.at(i).Address == ((COrder*)pParam)->CurrentCheckAddress)//找出先前的檢測指令
			{
				if (((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData.size())
				{
					if (((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData.back().x == ((COrder*)pParam)->AreaCheckChangTemp.X &&
						((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData.back().y == ((COrder*)pParam)->AreaCheckChangTemp.Y)//判斷是否加換線點
					{
						((COrder*)pParam)->IntervalAreaCheck.at(i).LineTrain.PointData.push_back({ -1,-1 });
					}
				}
			}
		}
	}
}
/*加入圓、圓弧路徑切點
*Start、Passing、End:起始、中間點、結束點座標
*PointData:要新增的線訓練地址
*Type:1圓弧 2圓
*return 1:新增切割點成功 0:新增切割點失敗
*/
BOOL COrder::NewCutPathPoint(CoordinateData Start, CoordinateData Passing, CoordinateData End, AreaCheck &IntervalAreaCheck, int Type)
{
	CCircleFormula CCircleFormula;//宣告一個計算物件
	AxeSpace  Point1, Point2, Point3;
	Point1 = { Start.X ,Start.Y ,0,0 };
	Point2 = { Passing.X,Passing.Y ,0,0 };
	Point3 = { End.X,End.Y ,0,0 };
	std::vector<AxeSpace> AxeSpaceInit;//宣告一個切割點存放陣列
	if (Type == 1)
	{ 
		//判斷圓弧是否在重組區域內
		if (ArcAreaJudge({ Start.X ,Start.Y }, { Passing.X,Passing.Y }, { End.X,End.Y }, MosaicAreaJudge(IntervalAreaCheck)/*{ IntervalAreaCheck.Image.Start.x,IntervalAreaCheck.Image.Start.y,IntervalAreaCheck.Image.End.x,IntervalAreaCheck.Image.End.y }*/))
		{
			CCircleFormula.CircleCutPath_2D_unit(Point1, Point2, Point3, 0, 1000, AxeSpaceInit);//切割圓弧API
			for (UINT i = 0; i < AxeSpaceInit.size(); i++)
			{
				IntervalAreaCheck.LineTrain.PointData.push_back({ AxeSpaceInit.at(i).x, AxeSpaceInit.at(i).y });
			}
			IntervalAreaCheck.LineTrain.PointData.push_back({ -1,-1 });
			return 1;
		}
	}
	else if (Type == 2)
	{
		//判斷圓是否在重組區域內
		if (CircleAreaJudge({ Start.X ,Start.Y }, { Passing.X,Passing.Y }, { End.X,End.Y }, MosaicAreaJudge(IntervalAreaCheck)/*{ IntervalAreaCheck.Image.Start.x,IntervalAreaCheck.Image.Start.y,IntervalAreaCheck.Image.End.x,IntervalAreaCheck.Image.End.y }*/))
		{
			CCircleFormula.CircleCutPath_2D_unit(Point1, Point2, Point3, 1, 1000, AxeSpaceInit);//切割圓API
			for (UINT i = 0; i < AxeSpaceInit.size(); i++)
			{
				IntervalAreaCheck.LineTrain.PointData.push_back({ AxeSpaceInit.at(i).x, AxeSpaceInit.at(i).y });
			}
			return 1;
		}
	}
	return 0;
}
/*判斷一個點是否在區域內
*Point:判斷點
*Area:區域的起始、結束點
return 1:在區域內or自動抓取重組範圍 0:在區域外
*/
BOOL COrder::PointAreaJudge(POINT Point, CRect Area)
{
	if (Area.left == VI_MosaicAreaDefault &&
		Area.top == VI_MosaicAreaDefault &&
		Area.right == VI_MosaicAreaDefault &&
		Area.bottom == VI_MosaicAreaDefault)//判斷區域是否為VI_MosaicAreaDefault
	{
		return 1;
	}
	Area.NormalizeRect();//區域正規劃
	return Area.PtInRect(Point);
}
/*判斷一條線是否在區域內
*PointS:線段起始點
*PointE:線段結束點
*Area:區域的起始、結束點
*return 1:在區域內or自動抓取重組範圍 0:在區域外
*/
BOOL COrder::LineAreaJudge(POINT PointS, POINT PointE, CRect Area)
{
	if (Area.left == VI_MosaicAreaDefault &&
		Area.top == VI_MosaicAreaDefault &&
		Area.right == VI_MosaicAreaDefault &&
		Area.bottom == VI_MosaicAreaDefault)//判斷區域是否為VI_MosaicAreaDefault
	{
		return 1;
	}
	if (PointAreaJudge(PointS, Area) && PointAreaJudge(PointE, Area))
	{
		return 1;
	}
	return 0;
}
/*判斷一個圓弧是否在區域內
*PointS:線段起始點
*PointA:圓弧中間點
*PointE:線段結束點
*Area:區域的起始、結束點
*return 1:在區域內or自動抓取重組範圍 0:在區域外
*/
BOOL COrder::ArcAreaJudge(POINT PointS, POINT PointA, POINT PointE, CRect Area)
{
	if (Area.left == VI_MosaicAreaDefault &&
		Area.top == VI_MosaicAreaDefault &&
		Area.right == VI_MosaicAreaDefault &&
		Area.bottom == VI_MosaicAreaDefault)//判斷區域是否為VI_MosaicAreaDefault
	{
		return 1;
	}
	CCircleFormula CCircleFormula;//宣告一個計算物件
	RectangleSpace RectangleCoordinate = { 0 };
	DPoint p1, p2, pA;
	p1 = { (double)PointS.x,(double)PointS.y,0,0 };
	p2 = { (double)PointE.x,(double)PointE.y,0,0 };
	pA = { (double)PointA.x,(double)PointA.y,0,0 };
	CCircleFormula.SpaceRectanglePointCalculation(p1, p2, pA, RectangleCoordinate, 1);//呼叫圓弧空間三點求矩形座標
#ifdef PRINTF
	/*_cwprintf(L"ArcAreaJudge::(%d,%d),(%d,%d),(%d,%d),(%d,%d)\n",
		RectangleCoordinate.P1.x, RectangleCoordinate.P1.y,
		RectangleCoordinate.P2.x, RectangleCoordinate.P2.y,
		RectangleCoordinate.P3.x, RectangleCoordinate.P3.y,
		RectangleCoordinate.P4.x, RectangleCoordinate.P4.y);*/
#endif
	if (PointAreaJudge({ RectangleCoordinate.P1.x,RectangleCoordinate.P1.y }, Area) &&
		PointAreaJudge({ RectangleCoordinate.P2.x,RectangleCoordinate.P2.y }, Area) &&
		PointAreaJudge({ RectangleCoordinate.P3.x,RectangleCoordinate.P3.y }, Area) &&
		PointAreaJudge({ RectangleCoordinate.P4.x,RectangleCoordinate.P4.y }, Area))
	{
		return 1;
	}
	return 0;
}
/*判斷一個圓是否在區域內
*PointS:線段起始點
*PointC1:圓中點1
*PointC2:圓中點2
*Area:區域的起始、結束點
*return 1:在區域內or自動抓取重組範圍 0:在區域外
*/
BOOL COrder::CircleAreaJudge(POINT PointS, POINT PointC1, POINT PointC2, CRect Area)
{
	if (Area.left == VI_MosaicAreaDefault &&
		Area.top == VI_MosaicAreaDefault &&
		Area.right == VI_MosaicAreaDefault &&
		Area.bottom == VI_MosaicAreaDefault)//判斷區域是否為VI_MosaicAreaDefault
	{
		return 1;
	}
	CCircleFormula CCircleFormula;//宣告一個計算物件
	RectangleSpace RectangleCoordinate = { 0 };
	DPoint p1, pA1, pA2;
	p1 = { (double)PointS.x,(double)PointS.y,0,0 };
	pA1 = { (double)PointC1.x,(double)PointC1.y,0,0 };
	pA2 = { (double)PointC2.x,(double)PointC2.y,0,0 };
	CCircleFormula.SpaceRectanglePointCalculation(p1, pA2, pA1, RectangleCoordinate, 0);//呼叫圓空間三點求矩形座標
#ifdef PRINTF
	/*_cwprintf(L"CircleAreaJudge::(%d,%d),(%d,%d),(%d,%d),(%d,%d)",
		RectangleCoordinate.P1.x, RectangleCoordinate.P1.y,
		RectangleCoordinate.P2.x, RectangleCoordinate.P2.y,
		RectangleCoordinate.P3.x, RectangleCoordinate.P3.y,
		RectangleCoordinate.P4.x, RectangleCoordinate.P4.y);*/
#endif
	if (PointAreaJudge({ RectangleCoordinate.P1.x,RectangleCoordinate.P1.y }, Area) &&
		PointAreaJudge({ RectangleCoordinate.P2.x,RectangleCoordinate.P2.y }, Area) &&
		PointAreaJudge({ RectangleCoordinate.P3.x,RectangleCoordinate.P3.y }, Area) &&
		PointAreaJudge({ RectangleCoordinate.P4.x,RectangleCoordinate.P4.y }, Area))
	{
		return 1;
	}
	return 0;
}
/*自動計算重組區間
*AreaCheckRun:區域檢測結構
return:目前無用途
*/
BOOL COrder::AutoCalculationArea(AreaCheck & AreaCheckRun)
{
	for (UINT i = 0; i < AreaCheckRun.DotTrain.PointData.size(); i++)
	{
		AreaCheckRun.Image.End.x = max(AreaCheckRun.DotTrain.PointData.at(i).x, AreaCheckRun.Image.End.x);//取最大值
		AreaCheckRun.Image.End.y = max(AreaCheckRun.DotTrain.PointData.at(i).y, AreaCheckRun.Image.End.y);//取最大值
	}
	for (UINT i = 0; i < AreaCheckRun.LineTrain.PointData.size(); i++)
	{
		if (AreaCheckRun.LineTrain.PointData.at(i).x != -1)//排除換線點
		{
			AreaCheckRun.Image.End.x = max(AreaCheckRun.LineTrain.PointData.at(i).x, AreaCheckRun.Image.End.x);//取最大值
			AreaCheckRun.Image.End.y = max(AreaCheckRun.LineTrain.PointData.at(i).y, AreaCheckRun.Image.End.y);//取最大值
		}
	}
	AreaCheckRun.Image.Start = AreaCheckRun.Image.End;
	for (UINT i = 0; i < AreaCheckRun.DotTrain.PointData.size(); i++)
	{
		AreaCheckRun.Image.Start.x = min(AreaCheckRun.DotTrain.PointData.at(i).x, AreaCheckRun.Image.Start.x);//取最小值
		AreaCheckRun.Image.Start.y = min(AreaCheckRun.DotTrain.PointData.at(i).y, AreaCheckRun.Image.Start.y);//取最小值
	}
	for (UINT i = 0; i < AreaCheckRun.LineTrain.PointData.size(); i++)
	{
		if (AreaCheckRun.LineTrain.PointData.at(i).x != -1)//排除換線點
		{
			AreaCheckRun.Image.Start.x = min(AreaCheckRun.LineTrain.PointData.at(i).x, AreaCheckRun.Image.Start.x);//取最小值
			AreaCheckRun.Image.Start.y = min(AreaCheckRun.LineTrain.PointData.at(i).y, AreaCheckRun.Image.Start.y);//取最小值
		}
	}
	AreaCheckRun.Image.Start.x = AreaCheckRun.Image.Start.x - VI_AutoCalculationMosaicAddOffset;
	AreaCheckRun.Image.Start.y = AreaCheckRun.Image.Start.y - VI_AutoCalculationMosaicAddOffset;
	AreaCheckRun.Image.End.x = AreaCheckRun.Image.End.x + VI_AutoCalculationMosaicAddOffset;
	AreaCheckRun.Image.End.y = AreaCheckRun.Image.End.y + VI_AutoCalculationMosaicAddOffset;
	return 0;
}
/*判斷重組區域是否為同一點
*Area:區域的起始、結束點
*return 修正過的區域
*/
CRect COrder::MosaicAreaJudge(AreaCheck &AreaCheck)
{
	if (AreaCheck.Image.Start.x == VI_MosaicAreaDefault &&
		AreaCheck.Image.Start.y == VI_MosaicAreaDefault &&
		AreaCheck.Image.End.x == VI_MosaicAreaDefault &&
		AreaCheck.Image.End.y == VI_MosaicAreaDefault)//判斷區域是否為VI_MosaicAreaDefault
	{
		return CRect(VI_MosaicAreaDefault, VI_MosaicAreaDefault, VI_MosaicAreaDefault, VI_MosaicAreaDefault);//用於自動抓取重組圖大小
	}
	else if (AreaCheck.Image.Start.x == AreaCheck.Image.End.x && AreaCheck.Image.Start.y == AreaCheck.Image.End.y)//用於單張拍圖
	{
#ifdef PRINTF
		_cwprintf(L"MosaicAreaJudge()::%d,%d,%d,%d\n", AreaCheck.Image.Start.x - AreaCheck.Image.ViewMove.x,
			AreaCheck.Image.Start.y - AreaCheck.Image.ViewMove.y,
			AreaCheck.Image.Start.x + AreaCheck.Image.ViewMove.x,
			AreaCheck.Image.Start.y + AreaCheck.Image.ViewMove.y);
#endif
		return CRect(
			AreaCheck.Image.Start.x - AreaCheck.Image.ViewMove.x,
			AreaCheck.Image.Start.y - AreaCheck.Image.ViewMove.y,
			AreaCheck.Image.Start.x + AreaCheck.Image.ViewMove.x,
			AreaCheck.Image.Start.y + AreaCheck.Image.ViewMove.y);
	}
	return CRect(AreaCheck.Image.Start.x, AreaCheck.Image.Start.y, AreaCheck.Image.End.x, AreaCheck.Image.End.y);//用於重組圖
}
/**************************************************************************額外功能*********************************************************************************/
/*儲存所有陣列*/
void COrder::SavePointData(LPVOID pParam)
{
	CString  Distinguish0 = _T("Command");
	CString  Distinguish1 = _T("PositionModifyNumber");
	CString  Distinguish2 = _T("LaserAdjust");
	CString  Distinguish3 = _T("m_ptVec");
	CString  StrBuff;
	TCHAR szFilters[] = _T("文字文件(*.txt)|*.txt|所有檔案(*.*)|*.*||");
	CFileDialog FileDlg(FALSE, L".txt", NULL, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST, szFilters);
	FileDlg.m_ofn.lpstrTitle = _T("儲存點資訊");
	if (FileDlg.DoModal() == IDOK)
	{
		CFile File;
		if (File.Open(FileDlg.GetPathName(), CFile::modeCreate | CFile::modeWrite))
		{
			CArchive ar(&File, CArchive::store);//儲存檔案
			//命令訊息
			ar << Distinguish0;
			for (UINT i = 0; i < ((COrder*)pParam)->CommandMemory.size(); i++)
			{
				ar << ((COrder*)pParam)->CommandMemory.at(i);
			}
			//對照表
			ar << Distinguish1;
			for (UINT i = 0; i < ((COrder*)pParam)->PositionModifyNumber.size(); i++)
			{
				for (UINT j = 0; j < ((COrder*)pParam)->PositionModifyNumber.at(i).size(); j++)
				{
					for (UINT k = 0; k < ((COrder*)pParam)->PositionModifyNumber.at(i).at(j).size(); k++)
					{
						ar << ((COrder*)pParam)->PositionModifyNumber.at(i).at(j).at(k).Address;
						StrBuff.Format(L"%d", ((COrder*)pParam)->PositionModifyNumber.at(i).at(j).at(k).LaserNumber);
						ar << StrBuff;
						StrBuff.Format(L"%d", ((COrder*)pParam)->PositionModifyNumber.at(i).at(j).at(k).VisionNumber);
						ar << StrBuff;
					}
				}
			}
			//高度訊息
			ar << Distinguish2;
			for (UINT i = 0; i < ((COrder*)pParam)->LaserAdjust.size(); i++)
			{
				StrBuff.Format(L"%d", ((COrder*)pParam)->LaserAdjust.at(i).LaserMeasureHeight);
				ar << StrBuff;
			}
			//連續線段點
			ar << Distinguish3;
#ifdef MOVE
			for (UINT i = 0; i < ((COrder*)pParam)->m_Action.LA_m_ptVec.size(); i++)
			{
				StrBuff.Format(L"%d", ((COrder*)pParam)->m_Action.LA_m_ptVec.at(i).EndPX);
				ar << StrBuff;
				StrBuff.Format(L"%d", ((COrder*)pParam)->m_Action.LA_m_ptVec.at(i).EndPY);
				ar << StrBuff;
				StrBuff.Format(L"%d", ((COrder*)pParam)->m_Action.LA_m_ptVec.at(i).EndPZ);
				ar << StrBuff;
			}
#endif
			ar.Close();
		}
		File.Close();
	}
}
/*載入所有陣列*/
void COrder::LoadPointData()
{
	//清空所有陣列
	CommandMemory.clear();
	PositionModifyNumber.clear();
	PositionModifyNumber.resize(10000);
	LaserAdjust.clear();
	m_Action.LA_Clear();
	//判斷字串
	CString  Distinguish0 = _T("Command");
	CString  Distinguish1 = _T("PositionModifyNumber");
	CString  Distinguish2 = _T("LaserAdjust");
	CString  Distinguish3 = _T("m_ptVec");
	LONG Control = 0;
	LONG Count = 0;
	TCHAR szFilters[] = _T("文字文件(*.txt)|*.txt|所有檔案(*.*)|*.*||");
	CFileDialog FileDlg(TRUE, NULL, L".txt", OFN_HIDEREADONLY | OFN_FILEMUSTEXIST, szFilters);
	FileDlg.m_ofn.lpstrTitle = _T("儲存命令表");
	if (FileDlg.DoModal() == IDOK)
	{
		CFile File;
		CString StrBuff;
		CString Address;
		if (File.Open(FileDlg.GetPathName(), CFile::modeRead))
		{
			CArchive ar(&File, CArchive::load);//讀取檔案
			while (1)
			{
				ar >> StrBuff;
				if (StrBuff == Distinguish0 || StrBuff == Distinguish1 || StrBuff == Distinguish2 || StrBuff == Distinguish3)
				{
					Control++;
				}
				else if (Control == 1)
				{
					CommandMemory.push_back(StrBuff);
				}
				else if (Control == 2)
				{
					Count++;
					if (Count == 1)
					{
						Address = StrBuff;
						UINT D1 = 0, D2 = 0;
						GetHashAddress(Address, D1, D2);//獲取雜湊表地址
						if (PositionModifyNumber.size() - 1 < D1)//判斷雜湊表數量是否足夠  size必須-1才是陣列編號
						{
							PositionModifyNumber.resize(PositionModifyNumber.size() + 10000);
						}
						if (D2 == 0)
						{
							if (PositionModifyNumber.at(D1).size() == 0)
							{
								PositionModifyNumber.at(D1).resize(1);
							}
						}
						else
						{
							if (PositionModifyNumber.at(D1).size() < D2)
							{
								PositionModifyNumber.at(D1).resize(D2 + 1000);
							}
						}
						PositionModifyNumber.at(D1).at(D2).push_back({ StrBuff,0,0 });
					}
					else if (Count == 2)
					{
						UINT D1 = 0, D2 = 0;
						GetHashAddress(Address, D1, D2);//獲取雜湊表地址
						PositionModifyNumber.at(D1).at(D2).back().LaserNumber = _ttoi(StrBuff);
					}
					else if (Count == 3)
					{
						UINT D1 = 0, D2 = 0;
						GetHashAddress(Address, D1, D2);//獲取雜湊表地址
						PositionModifyNumber.at(D1).at(D2).back().VisionNumber = -1;
						Count = 0;
					}
				}
				else if (Control == 3)
				{
					LaserAdjust.push_back({ _ttol(StrBuff) });
					Count = 0;
				}
#ifdef MOVE
				else if (Control == 4)
				{
					Count++;
					if (Count == 1)
					{
						m_Action.LA_m_ptVec.push_back({ 0,0,0,0,0 });
						m_Action.LA_m_ptVec.back().EndPX = _ttol(StrBuff);
					}
					else if (Count == 2)
					{
						m_Action.LA_m_ptVec.back().EndPY = _ttol(StrBuff);
					}
					else if (Count == 3)
					{
						m_Action.LA_m_ptVec.back().EndPZ = _ttol(StrBuff);
						Count = 0;
					}
				}
#endif
				if (ar.IsBufferEmpty() == 1)
					break;
			}
			ar.Close();
			File.Close();
			/*寫入連續線段SP*/
#ifdef MOVE
			m_Action.LA_m_iVecSP.clear();
			m_Action.LA_m_iVecSP.push_back(1);
			for (UINT i = 0; i < m_Action.LA_m_ptVec.size(); i++)
			{
				if (m_Action.LA_m_ptVec.at(i).EndPX == -99999 && m_Action.LA_m_ptVec.at(i).EndPY == -99999 && m_Action.LA_m_ptVec.at(i).EndPZ == -99999)
				{
					m_Action.LA_m_iVecSP.push_back(i + 1);
				}
			}
#endif
			/*Command轉換單位*/
			for (UINT i = 0; i < CommandMemory.size(); i++)
			{
				mmCommandMemory.push_back(CommandUnitConversinon(CommandMemory.at(i), 0.001, 0.001));
			}
			//CommandMemory.clear();
		}
	}
}
/*檢查命令用法*/
int COrder::CheckCommandRule(int &ErrorAddress)
{
	struct LabelData {
		UINT Address;
		LONG Num;
	};
	struct IntervalQueue {
		CString Command;
		UINT Begin;
		UINT End;
	};
	struct TempIntervalQueue {
		CString Command;
		UINT CallSubroutienAddress;
		UINT Begin;
	};
	struct LaserIntervalQueue {
		LONG Type;
		BOOL Switch;
		UINT Begin;
	};
	std::vector<LabelData> Label, StepRepeatLabel;
	std::vector<IntervalQueue> IntervalQueue;
	std::vector<TempIntervalQueue> SubIntervalQueue;
	LaserIntervalQueue  LaserIntervalQueue;
	LaserIntervalQueue = { 0,0,0 };
	//判斷命令列是否有End沒有自動加入
	if (CommandMemory.size())
	{
		if (CommandMemory.at(CommandMemory.size() - 1) != _T("End"))
		{
			CommandMemory.push_back(_T("End"));
		}
	}
	//第一次檢查(檢查Label、StepRepeatLabel是否有重複、End是否在最後一項、GotoAddress是否正確)
	for (UINT i = 0; i < CommandMemory.size(); i++)
	{
		if (CommandResolve(CommandMemory.at(i), 0) == L"Label")
		{
			if (!Label.size())
			{
				Label.push_back({ i,_ttol(CommandResolve(CommandMemory.at(i),1)) });
			}
			else
			{
				//檢查標籤是否重複
				for (UINT j = 0; j < Label.size(); j++)
				{
					if (Label.at(j).Num == _ttol(CommandResolve(CommandMemory.at(i), 1)))
					{
						ErrorAddress = i + 1;
						return 1;//有重複標籤存在
					}
				}
				Label.push_back({ i,_ttol(CommandResolve(CommandMemory.at(i),1)) });
			}
		}
		if (CommandResolve(CommandMemory.at(i), 0) == L"StepRepeatLabel")
		{
			if (!StepRepeatLabel.size())
			{
				StepRepeatLabel.push_back({ i,_ttol(CommandResolve(CommandMemory.at(i),1)) });
			}
			else
			{
				//檢查StepRepeat標籤是否重複
				for (UINT j = 0; j < StepRepeatLabel.size(); j++)
				{
					if (StepRepeatLabel.at(j).Num == _ttol(CommandResolve(CommandMemory.at(i), 1)))
					{
						ErrorAddress = i + 1;
						return 2;//有重複StepRepeat標籤存在
					}
				}
				StepRepeatLabel.push_back({ i,_ttol(CommandResolve(CommandMemory.at(i),1)) });
			}
		}
		if (CommandResolve(CommandMemory.at(i), 0) == L"GotoAddress")
		{
			//檢查是否形成無窮回圈
			if (_ttol(CommandResolve(CommandMemory.at(i), 1)) > 0 && _ttol(CommandResolve(CommandMemory.at(i), 1)) < (long)CommandMemory.size())
			{
				if (i >= (UINT)(_ttol(CommandResolve(CommandMemory.at(i), 1)) - 1))
				{
					ErrorAddress = i + 1;
					return 3;//GotoAddress形成無窮迴圈
				}
			}
			else
			{
				ErrorAddress = i + 1;
				return 4;//GotoAddress地址無效
			}
		}
		if (CommandResolve(CommandMemory.at(i), 0) == L"End")
		{
			//檢查End是否在最後一個
			if (i != CommandMemory.size() - 1)
			{
				ErrorAddress = i + 1;
				return 5;//End不是在最後一個
			}
		}
	}
#ifdef PRINTF
	_cwprintf(L"第一次檢查完畢\n");
#endif
	//第二次檢查(檢查GotoLabel、CallSubroutine是否有交錯和建立所有區間)
	for (UINT i = 0; i < CommandMemory.size(); i++)
	{
		if (CommandResolve(CommandMemory.at(i), 0) == L"GotoLabel")
		{
			for (UINT j = 0; j < Label.size(); j++)
			{
				if (_ttol(CommandResolve(CommandMemory.at(i), 1)) == Label.at(j).Num)
				{
					if (Label.at(j).Address < i)
					{
						ErrorAddress = i + 1;
						return 6;//GotoLabel形成無窮迴圈
					}
				}
			}
		}
		if (CommandResolve(CommandMemory.at(i), 0) == L"Loop")
		{
			for (UINT j = 0; j < Label.size(); j++)
			{
				if (_ttol(CommandResolve(CommandMemory.at(i), 1)) == Label.at(j).Num)
				{
					if (Label.at(j).Address > i)
					{
						ErrorAddress = i + 1;
						return 7;//Loop往下跳 Loop無效
					}
					else//加入區間
					{
						BOOL AddSwitch = TRUE;
						for (UINT k = 0; k < IntervalQueue.size(); k++)
						{
							if (IntervalQueue.at(k).End == i)//判斷是否新增過此區間
							{
								AddSwitch = FALSE;
							}
						}
						if (AddSwitch)
						{
							IntervalQueue.push_back({ CommandMemory.at(i) ,Label.at(j).Address,i });//儲存區間 
						}
					}
				}
			}
		}
		if (CommandResolve(CommandMemory.at(i), 0) == L"StepRepeatX" || CommandResolve(CommandMemory.at(i), 0) == L"StepRepeatY")
		{
			for (UINT j = 0; j < StepRepeatLabel.size(); j++)
			{
				if (_ttol(CommandResolve(CommandMemory.at(i), 6)) == StepRepeatLabel.at(j).Num)
				{
					if (StepRepeatLabel.at(j).Address > i)
					{
						ErrorAddress = i + 1;
						return 8;//StepRepeat往下跳 StepRepeat無效
					}
					else
					{
						BOOL AddSwitch = TRUE;
						for (UINT k = 0; k < IntervalQueue.size(); k++)
						{
							if (IntervalQueue.at(k).End == i)//判斷是否新增過此區間
							{
								AddSwitch = FALSE;
							}
						}
						if (AddSwitch)
						{
							IntervalQueue.push_back({ CommandMemory.at(i),StepRepeatLabel.at(j).Address,i });//儲存區間
						}
					}
				}
			}
		}
		if (CommandResolve(CommandMemory.at(i), 0) == L"CallSubroutine")
		{
			if (SubIntervalQueue.size())//如果已經有其他Subroutine呼叫中
			{
				for (UINT j = 0; j < SubIntervalQueue.size(); j++)
				{
					if (SubIntervalQueue.at(j).CallSubroutienAddress == i)
					{
						ErrorAddress = i + 1;
						return 9;//造成Subroutine重複呼叫
					}
				}
			}
			for (UINT j = 0; j < Label.size(); j++)
			{
				if (_ttol(CommandResolve(CommandMemory.at(i), 1)) == Label.at(j).Num)
				{
					SubIntervalQueue.push_back({ CommandMemory.at(i) ,i,Label.at(j).Address });
					i = Label.at(j).Address;
				}

			}
		}
		if (CommandResolve(CommandMemory.at(i), 0) == L"SubroutineEnd")
		{
			if (SubIntervalQueue.size())
			{
				BOOL AddSwitch = TRUE;
				for (UINT j = 0; j < IntervalQueue.size(); j++)
				{
					if (IntervalQueue.at(j).Command == SubIntervalQueue.back().Command)//已經有新增過區間
					{
						AddSwitch = FALSE;
					}
				}
				if (AddSwitch)
				{
					IntervalQueue.push_back({ SubIntervalQueue.back().Command,SubIntervalQueue.back().Begin,i });
				}
				i = SubIntervalQueue.back().CallSubroutienAddress;
				SubIntervalQueue.pop_back();
			}
		}
		if (CommandResolve(CommandMemory.at(i), 0) == L"LaserHeight")
		{
			if (_ttol(CommandResolve(CommandMemory.at(i), 1)))//雷射開
			{
				if (LaserIntervalQueue.Switch)
				{
					LaserIntervalQueue.Type = 1;//改變雷射模式
				}
				else
				{
					LaserIntervalQueue.Type = 1;
					LaserIntervalQueue.Switch = TRUE;
					LaserIntervalQueue.Begin = i;
				}
			}
			else//雷射關
			{
				if (LaserIntervalQueue.Switch)
				{
					if (LaserIntervalQueue.Type == 1)
					{
						BOOL AddSwitch = TRUE;
						for (UINT j = 0; j < IntervalQueue.size(); j++)
						{
							if (IntervalQueue.at(j).End == i)//已經有新增過區間
							{
								AddSwitch = FALSE;
							}
						}
						if (AddSwitch)
						{
							IntervalQueue.push_back({ CommandMemory.at(LaserIntervalQueue.Begin),LaserIntervalQueue.Begin,i });
						}
						LaserIntervalQueue = { 0,0,0 };
					}
				}
			}
		}
		if (CommandResolve(CommandMemory.at(i), 0) == L"LaserDetect")
		{
			if (_ttol(CommandResolve(CommandMemory.at(i), 1)))//雷射開
			{
				if (LaserIntervalQueue.Switch)
				{
					LaserIntervalQueue.Type = 2;//改變雷射模式
				}
				else
				{
					LaserIntervalQueue.Type = 2;
					LaserIntervalQueue.Switch = TRUE;
					LaserIntervalQueue.Begin = i;
				}
			}
			else//雷射關
			{
				if (LaserIntervalQueue.Switch)
				{
					if (LaserIntervalQueue.Type == 2)
					{
						BOOL AddSwitch = TRUE;
						for (UINT j = 0; j < IntervalQueue.size(); j++)
						{
							if (IntervalQueue.at(j).End == i)//已經有新增過區間
							{
								AddSwitch = FALSE;
							}
						}
						if (AddSwitch)
						{
							IntervalQueue.push_back({ CommandMemory.at(LaserIntervalQueue.Begin),LaserIntervalQueue.Begin,i });
						}
						LaserIntervalQueue = { 0,0,0 };
					}
				}
			}
		}
		if (CommandResolve(CommandMemory.at(i), 0) == L"End")
		{
			if (SubIntervalQueue.size())
			{
				ErrorAddress = SubIntervalQueue.back().CallSubroutienAddress + 1;
				return 10;//子程序沒有SubroutineEnd 
			}
			if (LaserIntervalQueue.Switch)
			{
				BOOL AddSwitch = TRUE;
				for (UINT j = 0; j < IntervalQueue.size(); j++)
				{
					if (IntervalQueue.at(j).End == i)//已經有新增過區間
					{
						AddSwitch = FALSE;
					}
				}
				if (AddSwitch)
				{
					IntervalQueue.push_back({ CommandMemory.at(LaserIntervalQueue.Begin),LaserIntervalQueue.Begin,i });
				}
				LaserIntervalQueue = { 0,0,0 };
			}
		}
	}
#ifdef PRINTF
	_cwprintf(L"第二次檢查完畢\n");
	for (UINT i = 0; i < IntervalQueue.size(); i++)
	{
		_cwprintf(L"CheckCommandRule():%s:%d:%d\n", IntervalQueue.at(i).Command, IntervalQueue.at(i).Begin, IntervalQueue.at(i).End);
	}
#endif
	//第三次檢查(檢查所有區間是否交錯)
	for (UINT i = 0; i < IntervalQueue.size(); i++)
	{
		for (UINT j = i + 1; j < IntervalQueue.size(); j++)
		{
			if (IntervalQueue.at(i).Begin > IntervalQueue.at(j).Begin)
			{
				if (IntervalQueue.at(i).End > IntervalQueue.at(j).End)
				{
					if (IntervalQueue.at(j).End > IntervalQueue.at(i).Begin)
					{
#ifdef PRINTF
						_cwprintf(L"%s和%s交錯\n", CommandResolve(IntervalQueue.at(i).Command, 0), CommandResolve(IntervalQueue.at(j).Command, 0));
#endif
						ErrorAddress = IntervalQueue.at(i).Begin + 1;
						return 11;//區間形成交錯
					}
				}
				/*else if(IntervalQueue.at(i).End == IntervalQueue.at(j).End)不用判斷 因為SubroutineEnd可以相同*/
			}
			else if (IntervalQueue.at(i).Begin == IntervalQueue.at(j).Begin)//Loop 或 StepRepeat發生
			{
				if (IntervalQueue.at(i).End == IntervalQueue.at(j).End)
				{
#ifdef PRINTF
					_cwprintf(L"%s和%s區間有重複\n", CommandResolve(IntervalQueue.at(i).Command, 0), CommandResolve(IntervalQueue.at(j).Command, 0));
#endif
					return 12;//出現重複區間判斷
				}
			}
			else if (IntervalQueue.at(i).Begin < IntervalQueue.at(j).Begin)
			{
				if (IntervalQueue.at(i).End < IntervalQueue.at(j).End)
				{
					if (IntervalQueue.at(i).End > IntervalQueue.at(j).Begin)
					{
#ifdef PRINTF
						_cwprintf(L"%s和%s交錯\n", CommandResolve(IntervalQueue.at(i).Command, 0), CommandResolve(IntervalQueue.at(j).Command, 0));
#endif
						ErrorAddress = IntervalQueue.at(j).Begin + 1;
						return 11;//區間形成交錯
					}
				}
			}
		}
	}
#ifdef PRINTF
	_cwprintf(L"第三次檢查完畢\n");
#endif
	return 0;
}
/*顯示所有狀態值*/
void COrder::ShowAllStatus()
{
	CString StrBuff, StrBuff1;
	StrBuff.Format(L"g_pThread:%x\ng_pSubroutineThread:%x\ng_pRunLoopThread:%x\ng_pIODetectionThread:%x\ng_pCheckCoordinateScanThread:%x\ng_pCheckActionThread:%x\ng_pMosaicDlgThread:%x\n",
		g_pThread, g_pSubroutineThread, g_pRunLoopThread, g_pIODetectionThread, g_pCheckCoordinateScanThread, g_pCheckActionThread, g_pMosaicDlgThread);
	StrBuff1.Format(L"RunStatus:%d\nStepCommandStatus:%d\nRunLoopStatus:%d\nGoHomeStatus:%d\n",
		RunStatusRead.RunStatus, RunStatusRead.StepCommandStatus, RunStatusRead.RunLoopStatus, RunStatusRead.GoHomeStatus);
	StrBuff = StrBuff + StrBuff1;
	AfxMessageBox(StrBuff);
}
/*載入命令*/
BOOL COrder::LoadCommand()
{
	if (!g_pThread && !g_pRunLoopThread)
	{
		LoadCommandData.FirstRun = TRUE;//第一次運行開啟
		if (PositionModifyNumber.size())//清除修正表
		{
			PositionModifyNumber.clear();
		}
		return 1;
	}
	return 0;
}
/*設置畫圖呼叫函式*/
BOOL COrder::SetDrawFunction(CDrawFunction Funtion, void * pObject)
{
	CallFunction.CDrawFunction = Funtion;
	CallFunction.pObject = pObject;
	if (CallFunction.pObject)
		return 1;
	return 0;
}
/*設置平台高度Z值*/
BOOL COrder::SetTabelZ(int *TableZ)
{
	if (TableZ != NULL)
	{
#ifdef MOVE
		m_Action.m_TablelZ = MO_ReadLogicPosition(2);
		*TableZ = m_Action.m_TablelZ;
#endif
	}
	else
	{
		return 0;
	}
	return 1;
}



