// Order.cpp : 實作檔
//

#include "stdafx.h"
#include "Order.h"


// COrder

IMPLEMENT_DYNAMIC(COrder, CWnd)

COrder::COrder()
{
    wakeEvent = NULL;
    DispenseDotSet = { 0,0 };
    DispenseDotEnd = { 0,0,0 };
    DispenseLineSet = { 0,0,0,0,0,0 };
    DispenseLineEnd = { 0,0,0,0,0 };
    DotSpeedSet = {0,0};
    LineSpeedSet = {0,0};
    ZSet = {0,0}; 
    GlueData = { {0,0,0,0},0,0,0,0 };

    VisionOffset = { { 0,0,0,0 },0,0,0 };
    VisionSet = { 0,0,0,0,0,0,0,0,0,0 };
    
    RunStatusRead = { 0,0,1,0};
    VisionDefault.VisionSerchError.Manuallymode = FALSE;
    VisionDefault.VisionSerchError.Pausemode = FALSE;

    RunLoopData = { 0,0,0,-1 };
    LaserSwitch = { 0,0,0,0,0 };
    VirtualCoordinateData = { 0,0,0,0 };

    ModelControl = { 1,0,0,0 };

    /****************************DemoTemprerily*******************************/
    DemoTemprarilySwitch = FALSE;
}
COrder::~COrder()
{
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
            if (CommandMemory.at(CommandMemory.size() - 1) != _T("End"))
            {
                CommandMemory.push_back(_T("End"));
            }
            //針頭模式 不減offset
            VisionDefault.VisionSet.ModifyMode = FALSE;
            //參數設定為預設
            ParameterDefult();
            //劃分主副程序
            MainSubProgramSeparate();
            //判斷初始化模組選擇
            DecideBeginModel();
            //狀態初始化
            DecideInit();
            //載入所有檔案名
            ListAllFileInDirectory(VisionFile.ModelPath,TEXT("*年*月*日*時*分*秒*.mod"));
            //出膠控制器模式
            m_Action.g_bIsDispend = TRUE;   
            wakeEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
            g_pThread = AfxBeginThread(Thread, (LPVOID)this);
            return TRUE;
        }  
        else
        {
            return FALSE;
        }    
    }
    else
    {
        return FALSE;
    }
}
/*循環開始*/
BOOL COrder::RunLoop(int LoopNumber) {
    if (LoopNumber > 0 && !g_pRunLoopThread)
    {
        RunLoopData.RunSwitch = TRUE;
        RunLoopData.LoopNumber = LoopNumber;
        RunLoopData.LoopCount = 0;
        g_pRunLoopThread = AfxBeginThread(RunLoopThread, (LPVOID)this);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
/*停止*/
BOOL COrder::Stop()
{
    RunLoopData.RunSwitch = FALSE;//關閉循環
    if (g_pThread)//判斷是否有在運作
    {
        if (SuspendThread(g_pThread) != GetLastError())
        {
            m_Action.g_bIsStop = TRUE;
            m_Action.g_bIsPause = FALSE;
            g_pThread->ResumeThread();//啟動線程
            #ifdef MOVE
                MO_STOP();//立即停止運動指令
            #endif
            //SetEvent(wakeEvent);
            //WaitForSingleObject(g_pThread->m_hThread, INFINITE);        //等待线程安全返回
            CloseHandle(wakeEvent);
            return TRUE;
        }
        else
        {
            m_Action.g_bIsStop = TRUE;
            #ifdef MOVE
                MO_STOP();//立即停止運動指令
            #endif
            return TRUE;
        }
    }
    else
    {     
        return FALSE;
    }
}
/*暫停*/
BOOL COrder::Pause()
{
    if (g_pThread && RunStatusRead.RunStatus == 1 && RunStatusRead.GoHomeStatus) {
        m_Action.g_bIsPause = TRUE;
        g_pThread->SuspendThread();
        RunStatusRead.RunStatus = 2;//狀態改變成暫停中
        m_Action.g_bIsPause = TRUE;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
/*繼續*/
BOOL COrder::Continue()
{
    if (g_pThread)//判斷是否有在運作
    {
        if (SuspendThread(g_pThread) != GetLastError() && RunStatusRead.RunStatus == 2 && RunStatusRead.GoHomeStatus)
        {
            m_Action.g_bIsPause = FALSE;
            g_pThread->ResumeThread();//啟動線程
            RunStatusRead.RunStatus = 1;//狀態改變成運作中
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }
}
/*原點賦歸*/
BOOL COrder::Home(BOOL mode)
{
    if (!g_pThread)
    {           
        //將停止狀態清除
        m_Action.g_bIsStop = FALSE;
        //確定GoHome參數賦值
        GoHome = Default.GoHome;
        //回原點模式設定
        GoHome.VisionGoHome = mode;
        //回歸狀態設為FALSE
        RunStatusRead.GoHomeStatus = FALSE;
        //出膠控制器關
        m_Action.g_bIsDispend = FALSE;
        g_pThread = AfxBeginThread(HomeThread, (LPVOID)this);  
        return TRUE;
    }
    else
    {
        return FALSE;
    }     
}
/*View查看*/
BOOL COrder::View(BOOL mode)
{
    if (!g_pThread) {
        Commanding = _T("Start");
        if (!CommandMemory.empty())
        {
            if (CommandMemory.at(CommandMemory.size() - 1) != _T("End"))
            {
                CommandMemory.push_back(_T("End"));
            }
            //針頭模式 不減offset 或 CCD模式 減offset
            VisionDefault.VisionSet.ModifyMode = mode;
            //參數設定為預設
            ParameterDefult();
            //劃分主副程序
            MainSubProgramSeparate();
            //判斷初始化模組選擇
            DecideBeginModel();
            //狀態初始化
            DecideInit();
            //載入所有檔案名
            ListAllFileInDirectory(VisionFile.ModelPath, TEXT("*年*月*日*時*分*秒*.mod"));
            //出膠控制器模式
            m_Action.g_bIsDispend = FALSE;
            //wakeEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
            g_pThread = AfxBeginThread(Thread, (LPVOID)this);
        }
        return TRUE;
    }
    else {
        return FALSE;
    }
    return 0;
}
/**************************************************************************執行續動作區塊*************************************************************************/
/*重複運行執行續*/
UINT COrder::RunLoopThread(LPVOID pParam) {
    while (((COrder*)pParam)->RunLoopData.RunSwitch)   
    {
#ifdef PRINTF
        _cprintf("%d", (((COrder*)pParam)->RunLoopData.MaxRunNumber - int(((COrder*)pParam)->RunStatusRead.FinishProgramCount)));
#endif
        if (((COrder*)pParam)->RunLoopData.MaxRunNumber >= 0 && (((COrder*)pParam)->RunLoopData.MaxRunNumber - int(((COrder*)pParam)->RunStatusRead.FinishProgramCount)) > 0)
        {
            if (((COrder*)pParam)->RunLoopData.LoopCount == ((COrder*)pParam)->RunLoopData.LoopNumber)
            {
                ((COrder*)pParam)->RunLoopData.RunSwitch = FALSE;
            }
            else if (((COrder*)pParam)->RunStatusRead.RunStatus == 0)
            {          
                if (!((COrder*)pParam)->Run()) {
                    ((COrder*)pParam)->RunLoopData.RunSwitch = FALSE;
                }
            }
        }
        else if (((COrder*)pParam)->RunLoopData.MaxRunNumber < 0 && ((COrder*)pParam)->RunLoopData.LoopCount != ((COrder*)pParam)->RunLoopData.LoopNumber)
        {
            if (((COrder*)pParam)->RunStatusRead.RunStatus == 0)
            {
                if (!((COrder*)pParam)->Run()) {
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
        Sleep(10);
    }
    g_pRunLoopThread = NULL;
    return 0;
}
/*原點賦歸執行緒*/
UINT COrder::HomeThread(LPVOID pParam)
{
#ifdef MOVE
    ((COrder*)pParam)->m_Action.DecideInitializationMachine(((COrder*)pParam)->GoHome.Speed1, ((COrder*)pParam)->GoHome.Speed2, ((COrder*)pParam)->GoHome.Axis, ((COrder*)pParam)->GoHome.MoveX, ((COrder*)pParam)->GoHome.MoveY, ((COrder*)pParam)->GoHome.MoveZ);
    if (((COrder*)pParam)->GoHome.VisionGoHome)//做完賦歸移動位置
    {   
        ((COrder*)pParam)->VisionSet.AdjustOffsetX = ((COrder*)pParam)->VisionDefault.VisionSet.AdjustOffsetX;
        ((COrder*)pParam)->VisionSet.AdjustOffsetY = ((COrder*)pParam)->VisionDefault.VisionSet.AdjustOffsetY;
        ((COrder*)pParam)->m_Action.DecideVirtualPoint(-(((COrder*)pParam)->VisionSet.AdjustOffsetX), -(((COrder*)pParam)->VisionSet.AdjustOffsetY), 0,
            30000, 80000, 6000);
            //((COrder*)pParam)->DotSpeedSet.EndSpeed, ((COrder*)pParam)->DotSpeedSet.AccSpeed, 6000);
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
    _cwprintf(L"是否載入Demo檔:%d\n", ((COrder*)pParam)->DemoTemprarilySwitch);
#endif
    ((COrder*)pParam)->RunStatusRead.RunStatus = 1;//狀態改變成運作中
    while ((!((COrder*)pParam)->m_Action.g_bIsStop) && ((COrder*)pParam)->ModelControl.Mode != 4/*&& ((COrder*)pParam)->Commanding != _T("End")*/)//新增模式判斷
    {
        if (((COrder*)pParam)->RunData.SubProgramName != _T(""))//雙程式使用
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
        if (((COrder*)pParam)->Program.LabelName != _T(""))//普通標籤
        {
            ((COrder*)pParam)->Program.LabelCount++;
            if (((COrder*)pParam)->Program.LabelName == ((COrder*)pParam)->Command.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)).at(((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)))
                || ((COrder*)pParam)->Program.LabelCount == ((COrder*)pParam)->Command.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)).size())
            {  
                //執行Call子程序，但沒有此子程序時
                if (((COrder*)pParam)->Program.CallSubroutineStatus && ((COrder*)pParam)->Program.LabelCount == ((COrder*)pParam)->Command.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)).size())
                {
                    if (!((COrder*)pParam)->Program.SubroutineStack.empty())
                    {
                        //堆疊程序計數--
                        ((COrder*)pParam)->Program.SubroutinCount--;
                        //紀錄程序位置堆疊釋放
                        ((COrder*)pParam)->Program.SubroutineStack.pop_back();
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
                //執行Loop，沒有此標籤時
                if (((COrder*)pParam)->RepeatData.LoopSwitch && ((COrder*)pParam)->Program.LabelCount == ((COrder*)pParam)->Command.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)).size())
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
        }
        else if (((COrder*)pParam)->RepeatData.StepRepeatLabel != _T(""))//StepRepeat專用標籤
        {
#ifdef PRINTF
            _cwprintf(L"標籤%s\n", ((COrder*)pParam)->RepeatData.StepRepeatLabel);
#endif
            if (((COrder*)pParam)->RepeatData.StepRepeatLabelLock)
            {
#ifdef PRINTF
                _cwprintf(L"%s=%s\n", ((COrder*)pParam)->RepeatData.StepRepeatLabel, CommandResolve(((COrder*)pParam)->Command.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)).at(((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount))), 6));
#endif
                if (((COrder*)pParam)->RepeatData.StepRepeatLabel ==
                    CommandResolve(((COrder*)pParam)->Command.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)).at(((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount))), 6) && 
                    (CommandResolve(((COrder*)pParam)->Command.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)).at(((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount))), 0) == L"StepRepeatX" ||
                    CommandResolve(((COrder*)pParam)->Command.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)).at(((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount))), 0) == L"StepRepeatY") )
                {
                    ((COrder*)pParam)->RepeatData.StepRepeatLabel = _T("");
                    //++為了讓他執行StepRepeat
                    ((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount))++;
                }
            }
            else
            {
                if (((COrder*)pParam)->RepeatData.StepRepeatLabel == ((COrder*)pParam)->Command.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)).at(((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount))))
                {
                    ((COrder*)pParam)->RepeatData.StepRepeatLabel = _T("");//標籤清除
                }
            }
        }
        else//執行指令
        {
            ((COrder*)pParam)->Commanding = ((COrder*)pParam)->Command.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)).at(((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)));
            ((COrder*)pParam)->RunStatusRead.CurrentRunCommandNum = ((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount));//主程式編號
            g_pSubroutineThread = AfxBeginThread(((COrder*)pParam)->SubroutineThread, pParam);
            while (g_pSubroutineThread) {
                Sleep(1);//while 程式負載問題 無限迴圈，並讓 CPU 休息一下
            }
        }

        if (((COrder*)pParam)->RepeatData.StepRepeatLabelLock)//在StepRepeat階段用來尋找N層StepRepeat用
        {
            if (((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)) == 0)
            {
#ifdef PRINTF
                _cwprintf(L"沒有發現其他StepRepeat\n");
#endif
                if (CommandResolve(((COrder*)pParam)->RepeatData.StepRepeatLabel, 0) == L"StepRepeatLabel")
                {
#ifdef PRINTF
                    _cwprintf(L"發生尋找錯誤立即停止程序!\n");
#endif
                    ((COrder*)pParam)->m_Action.g_bIsStop = TRUE;
                }
                else
                {
                    ((COrder*)pParam)->RepeatData.StepRepeatLabel = _T("StepRepeatLabel,") + ((COrder*)pParam)->RepeatData.StepRepeatLabel;
                }       
                ((COrder*)pParam)->RepeatData.StepRepeatLabelLock = FALSE;
            }
            else
            {
                ((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount))--;
            }
        }
        else
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
        //_cwprintf(L"%d\n", ((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)));
        //_cwprintf(L"大家都進來吧!%s\n", ((COrder*)pParam)->RepeatData.StepRepeatLabel);
#endif 
    }
    LineGotoActionJudge(pParam);//判斷動作狀態
    if (((COrder*)pParam)->Commanding == _T("End"))//計數運行次數使用
    {
        ((COrder*)pParam)->RunStatusRead.FinishProgramCount++;
        if (((COrder*)pParam)->RunLoopData.RunSwitch)
        {
            ((COrder*)pParam)->RunLoopData.LoopCount++;
        }
    }
    if (((COrder*)pParam)->GoHome.PrecycleInitialize)//開啟結束初始化(原點賦歸)
    {
        ((COrder*)pParam)->m_Action.BackGOZero(120000, 1500000, 6000);
    }
    //TODO::DEMO所以加入
    /*if (AfxMessageBox(_T("資料即將清除，是否儲存?"), MB_OKCANCEL, 0) == IDOK) 
    {
        SavePointData(pParam);
    } */
    if (!((COrder*)pParam)->DemoTemprarilySwitch)//如果DemoTemprarilySwitch為FALSE清除
    {
        ((COrder*)pParam)->m_Action.LA_Clear();//清除連續線段陣列
    } 
    ((COrder*)pParam)->DecideClear();//清除所有陣列
    ((COrder*)pParam)->RunStatusRead.RunStatus = 0;//狀態設為未運行
    g_pThread = NULL;
    return 0;
}
/*命令動作(子)執行緒*/
UINT COrder::SubroutineThread(LPVOID pParam) {
    HANDLE wakeEvent = (HANDLE)((COrder*)pParam)->wakeEvent;
    CString Command = ((COrder*)pParam)->Commanding; 
    if (CommandResolve(Command, 0) == L"Printf")
    {
#ifdef PRINTF
        _cwprintf(L"Please don't Look me!\n");
#endif     
        //((COrder*)pParam)->m_Action.WaitTime(wakeEvent, _ttoi(CommandResolve(Command, 1)));
    }
    /************************************************************程序**************************************************************/
    if (CommandResolve(Command, 0) == L"GotoLabel") 
    {
        ((COrder*)pParam)->Program.LabelName = _T("Label,") + CommandResolve(Command, 1);
    }
    if (CommandResolve(Command, 0) == L"GotoAddress") 
    {
        if (_ttoi(CommandResolve(Command, 1)))
        {
            ((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)) = _ttoi(CommandResolve(Command, 1)) - 2; 
        }
    }
    if (CommandResolve(Command, 0) == L"CallSubroutine")
    {    
        //TODO::有問題 如果LS->CallSubroutine 結果沒有此Subroutine時
        if (((COrder*)pParam)->RunData.ActionStatus.at(((COrder*)pParam)->Program.SubroutinCount) == 1)//LS時 移動至LS虛擬點
        {
            ((COrder*)pParam)->m_Action.DecideVirtualPoint(((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Y, ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Z,
                ((COrder*)pParam)->DotSpeedSet.EndSpeed, ((COrder*)pParam)->DotSpeedSet.AccSpeed, 6000);
        }
        //CallSubroutineStatus狀態打開
        ((COrder*)pParam)->Program.CallSubroutineStatus = TRUE;//為了判斷是否有這個子程序
        ((COrder*)pParam)->Program.LabelName = _T("Label,") + CommandResolve(Command, 1);
        //將目前程序地址紀錄
        ((COrder*)pParam)->Program.SubroutineStack.push_back(((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)));
        //新增紀錄目前機械手臂(虛擬座標)位置堆疊
        CoordinateData Buff;
        Buff.X = ((COrder*)pParam)->VirtualCoordinateData.X;
        Buff.Y = ((COrder*)pParam)->VirtualCoordinateData.Y;
        Buff.Z = ((COrder*)pParam)->VirtualCoordinateData.Z;
        //實際座標取法
        //    Buff.X = _ttol(CommandResolve(((COrder*)pParam)->m_Action.NowLocation(), 0));
        //    Buff.Y = _ttol(CommandResolve(((COrder*)pParam)->m_Action.NowLocation(), 1));
        //    Buff.Z = _ttol(CommandResolve(((COrder*)pParam)->m_Action.NowLocation(), 2));

        ((COrder*)pParam)->Program.SubroutinePointStack.push_back(Buff);
        //新增offset堆疊
        Buff.Status = FALSE;
        Buff.X = 0;
        Buff.Y = 0;
        Buff.Z = 0;
        ((COrder*)pParam)->OffsetData.push_back(Buff);
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
    }
    if (CommandResolve(Command, 0) == L"SubroutineEnd")
    {
        //判斷子程序堆疊中是否為空
        if (!((COrder*)pParam)->Program.SubroutineStack.empty())
        {
            //將程序地址設為呼叫子程序時地址
            ((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)) = ((COrder*)pParam)->Program.SubroutineStack.back();
            //釋放紀錄程序位置堆疊
            ((COrder*)pParam)->Program.SubroutineStack.pop_back();
            //TODO::可選擇是否在影像模式不移動回CallSuboutine位置
            //將機器手臂移動至呼叫時位置
            if (((COrder*)pParam)->ModelControl.Mode != 1 && ((COrder*)pParam)->ModelControl.Mode != 2)//在影像、雷射模式下不做回點
            {
                ((COrder*)pParam)->m_Action.DecideVirtualPoint(
                    ((COrder*)pParam)->Program.SubroutinePointStack.at(((COrder*)pParam)->Program.SubroutinCount).X,
                    ((COrder*)pParam)->Program.SubroutinePointStack.at(((COrder*)pParam)->Program.SubroutinCount).Y,
                    ((COrder*)pParam)->Program.SubroutinePointStack.at(((COrder*)pParam)->Program.SubroutinCount).Z,
                    ((COrder*)pParam)->DotSpeedSet.EndSpeed, ((COrder*)pParam)->DotSpeedSet.AccSpeed, 6000);
            }
            //釋放紀錄手臂位置堆疊
            ((COrder*)pParam)->Program.SubroutinePointStack.pop_back();
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
    }
    if (CommandResolve(Command, 0) == L"StepRepeatLabel")
    {
        if (!((COrder*)pParam)->RepeatData.StepRepeatLabelLock)//目的用於跳到StepRepeat最外層迴圈
        {
#ifdef PRINTF
            _cwprintf(L"進入StepRepeatLabel\n");
#endif  
            CString CommandBuff;
            for (UINT i = ((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)); i < ((COrder*)pParam)->Command.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)).size(); i++)
            {
                CommandBuff = ((COrder*)pParam)->Command.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)).at(i);
                if (_ttol(CommandResolve(CommandBuff, 6)) == _ttol(CommandResolve(Command, 1)) && (CommandResolve(CommandBuff, 0) == L"StepRepeatX" || CommandResolve(CommandBuff, 0) == L"StepRepeatY"))
                {
#ifdef PRINTF
                    _cwprintf(L"跳躍至地址為%d的StepRepeat\n", i);
#endif               
                    ((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)) = i + 1;
                    ((COrder*)pParam)->RepeatData.StepRepeatLabelLock = TRUE;                
#ifdef PRINTF
                    _cprintf("%d\n", ((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)));
#endif     
                }
            }     
        }  
    }
    if (CommandResolve(Command, 0) == L"Loop")
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
    if (CommandResolve(Command, 0) == L"StepRepeatX")
    {
#ifdef PRINTF
        _cprintf("Inside StepRepeatX\n");
#endif 
        //StepRepeat陣列是否大於1*1 && 模式是否 1or2 
        if ((_ttol(CommandResolve(Command, 3)) * _ttol(CommandResolve(Command, 4))) > 1 && (_ttol(CommandResolve(Command,5)) == 1 || _ttol(CommandResolve(Command, 5)) == 2))
        {
            if (!((COrder*)pParam)->RepeatData.StepRepeatNum.size())//都沒有RepeatXY時
            {
                if (((COrder*)pParam)->RepeatData.StepRepeatLabelLock)//第一次進入SetpRepeat(代表最外層)
                {
#ifdef PRINTF
                    _cwprintf(L"沒有StepRepeat時 第一次 Inside StepRepeatX\n");
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
                    ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.push_back(((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X);
                    ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.push_back(((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y);
                    //紀錄X、Y計數
                    ((COrder*)pParam)->RepeatData.StepRepeatCountX.push_back(_ttol(CommandResolve(Command, 3)));
                    ((COrder*)pParam)->RepeatData.StepRepeatCountY.push_back(_ttol(CommandResolve(Command, 4)));
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
                        _cwprintf(L"StepRepeatX 處理阻斷位置:");
                        for (UINT i = 0; i < ((COrder*)pParam)->RepeatData.StepRepeatBlockData.back().BlockPosition.size(); i++)
                        {
                            _cwprintf(L"%s,", ((COrder*)pParam)->RepeatData.StepRepeatBlockData.back().BlockPosition.at(i));
                        }
                        _cwprintf(L"\n");
#endif
                        BlockProcessStart(Command, pParam, FALSE);
                        
                    }
                    else//無阻斷繼續往下尋找StepRepeat
                    {
#ifdef PRINTF
                        _cwprintf(L"StepRepeatX 沒有阻斷\n");
#endif
                        ((COrder*)pParam)->RepeatData.StepRepeatLabel = CommandResolve(Command, 6);  
                                    
                    }   
                }
                else 
                {
#ifdef PRINTF
                    _cwprintf(L"所有條件不成立\n");
#endif
                }
            }
            else//有StepRepeat時
            {
                UINT StepRepeatNumSize = ((COrder*)pParam)->RepeatData.StepRepeatNum.size();
#ifdef PRINTF
                _cwprintf(L"目前StepRepeatNumSize:%d\n", StepRepeatNumSize);
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
                                    _cwprintf(L"第%d:SSwitch轉換:%d\n", i, ((COrder*)pParam)->RepeatData.SSwitch.at(i));
#endif 
                                }
                                else if (((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X == ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.at(i))
                                {
                                    ((COrder*)pParam)->RepeatData.SSwitch.at(i) = TRUE;
#ifdef PRINTF
                                    _cwprintf(L"第%d:SSwitch轉換:%d\n", i, ((COrder*)pParam)->RepeatData.SSwitch.at(i));
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
                                if (!BlockProcessExecute(Command, pParam, i))
                                    break;
                            } 
                        }
                        else if(((COrder*)pParam)->RepeatData.StepRepeatCountY.at(i) > 1)
                        {
                            ((COrder*)pParam)->RepeatData.SSwitch.at(i) = !((COrder*)pParam)->RepeatData.SSwitch.at(i);
#ifdef PRINTF
                            _cwprintf(L"第%d:SSwitch轉換:%d\n", i, ((COrder*)pParam)->RepeatData.SSwitch.at(i));
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
                                if (!BlockProcessExecute(Command, pParam, i))
                                    break;
                            } 
                        }
                        else
                        {                      
#ifdef PRINTF
                            _cwprintf(L"刪除所有陣列\n");
#endif 
                            ((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum++;
#ifdef PRINTF
                            _cwprintf(L"刪除總數+1=%d\n", ((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum);
#endif 
                            ((COrder*)pParam)->RepeatData.StepRepeatBlockData.erase(((COrder*)pParam)->RepeatData.StepRepeatBlockData.begin() + i);
#ifdef PRINTF
                            _cwprintf(L"刪除StepRepeatBlockData陣列\n");
#endif 
                            ((COrder*)pParam)->RepeatData.SSwitch.erase(((COrder*)pParam)->RepeatData.SSwitch.begin() + i);
#ifdef PRINTF
                            _cwprintf(L"刪除SSwitch陣列\n");
#endif 
                            ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X = ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.at(i);
                            ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y = ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.at(i);
                            ((COrder*)pParam)->RepeatData.StepRepeatNum.erase(((COrder*)pParam)->RepeatData.StepRepeatNum.begin() + i);
#ifdef PRINTF
                            _cwprintf(L"刪除StepRepeatNum陣列\n");
#endif 
                            ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.erase(((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.begin() + i);
#ifdef PRINTF
                            _cwprintf(L"刪除StepRepeatInitOffsetX陣列\n");
#endif 
                            ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.erase(((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.begin() + i);
#ifdef PRINTF
                            _cwprintf(L"刪除StepRepeatInitOffsetY陣列\n");
#endif 
                            ((COrder*)pParam)->RepeatData.StepRepeatCountX.erase(((COrder*)pParam)->RepeatData.StepRepeatCountX.begin() + i);
#ifdef PRINTF
                            _cwprintf(L"刪除StepRepeatCountX陣列\n");
#endif 
                            ((COrder*)pParam)->RepeatData.StepRepeatCountY.erase(((COrder*)pParam)->RepeatData.StepRepeatCountY.begin() + i);
#ifdef PRINTF
                            _cwprintf(L"刪除StepRepeatCountY陣列\n");
#endif 
                            break;
                        }
                        if (((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum != 0 && ((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum != ((COrder*)pParam)->RepeatData.AllNewStepRepeatNum )
                        {
#ifdef PRINTF
                            _cwprintf(L"進入新增內層迴圈\n");
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
                            if (((COrder*)pParam)->RepeatData.StepRepeatLabelLock && !((COrder*)pParam)->RepeatData.AddInStepRepeatSwitch)//第一次進入SetpRepeat時(內層迴圈)
                            {
#ifdef PRINTF
                                _cwprintf(L"第一次新增內層StepRepeat\n");
#endif 
                                //StepRepeat計數總數++
                                ((COrder*)pParam)->RepeatData.AllNewStepRepeatNum++;
                                //紀錄StepRepeat地址
                                ((COrder*)pParam)->RepeatData.StepRepeatNum.push_back(((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)));
                                //紀錄初始offset位置
                                ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.push_back(((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X);
                                ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.push_back(((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y);
                                //紀錄X、Y計數
                                ((COrder*)pParam)->RepeatData.StepRepeatCountX.push_back(_ttol(CommandResolve(Command, 3)));
                                ((COrder*)pParam)->RepeatData.StepRepeatCountY.push_back(_ttol(CommandResolve(Command, 4)));
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
                                    _cwprintf(L"StepRepeatX 處理阻斷位置:");
                                    for (UINT i = 0; i < ((COrder*)pParam)->RepeatData.StepRepeatBlockData.back().BlockPosition.size(); i++)
                                    {
                                        _cwprintf(L"%s,", ((COrder*)pParam)->RepeatData.StepRepeatBlockData.back().BlockPosition.at(i));
                                    }
                                    _cwprintf(L"\n");
#endif 
                                    BlockProcessStart(Command, pParam, FALSE);
                                }
                                else//無阻斷不記錄繼續往下尋找StepRepeat
                                {
#ifdef PRINTF
                                    _cwprintf(L"StepRepeatX 沒有阻斷\n");
#endif 
                                    ((COrder*)pParam)->RepeatData.StepRepeatLabel = CommandResolve(Command, 6);
                                }
                            }
                            else //第二次進入StepRepeat時(內層迴圈做第N次)
                            {
#ifdef PRINTF
                                _cwprintf(L"第二次新增內層StepRepeat\n"); 
#endif 
                                ((COrder*)pParam)->RepeatData.AddInStepRepeatSwitch = FALSE;
                                ((COrder*)pParam)->RepeatData.StepRepeatLabelLock = FALSE;
                                //紀錄StepRepeat地址
                                ((COrder*)pParam)->RepeatData.StepRepeatNum.push_back(((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)));
                                //紀錄初始offset位置
                                ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.push_back(((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X);
                                ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.push_back(((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y);
                                //紀錄X、Y計數
                                ((COrder*)pParam)->RepeatData.StepRepeatCountX.push_back(_ttol(CommandResolve(Command, 3)));
                                ((COrder*)pParam)->RepeatData.StepRepeatCountY.push_back(_ttol(CommandResolve(Command, 4)));
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
                                    _cwprintf(L"StepRepeatX 處理阻斷位置:");
                                    for (UINT i = 0; i < ((COrder*)pParam)->RepeatData.StepRepeatBlockData.back().BlockPosition.size(); i++)
                                    {
                                        _cwprintf(L"%s,", ((COrder*)pParam)->RepeatData.StepRepeatBlockData.back().BlockPosition.at(i));
                                    }
                                    _cwprintf(L"\n");
#endif 
                                    BlockProcessStart(Command, pParam , TRUE);
                                }
                                else//無阻斷不記錄繼續往下尋找StepRepeat
                                {
#ifdef PRINTF
                                    _cwprintf(L"StepRepeatX 沒有阻斷\n");
#endif 
                                    if (((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum != 0 && ((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum != ((COrder*)pParam)->RepeatData.AllNewStepRepeatNum)
                                    {
#ifdef PRINTF
                                        _cwprintf(L"進入新增內層迴圈2");
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
        else//當只有1-1時
        {
            
        }
    }
    if (CommandResolve(Command, 0) == L"StepRepeatY")
    {
#ifdef PRINTF
        _cprintf("Inside StepRepeatY\n");
#endif 
        //StepRepeat陣列是否大於1*1 && 模式是否 1or2
        if ((_ttol(CommandResolve(Command, 3)) * _ttol(CommandResolve(Command, 4))) > 1 && (_ttol(CommandResolve(Command, 5)) == 1 || _ttol(CommandResolve(Command, 5)) == 2))
        {
            if (!((COrder*)pParam)->RepeatData.StepRepeatNum.size())//都沒有RepeatXY時
            {
                if (((COrder*)pParam)->RepeatData.StepRepeatLabelLock)//第一次進入SetpRepeat(代表最外層)
                {
#ifdef PRINTF
                    _cwprintf(L"沒有StepRepeat時 第一次 Inside StepRepeatY\n");
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
                    ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.push_back(((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X);
                    ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.push_back(((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y);
                    //紀錄X、Y計數
                    ((COrder*)pParam)->RepeatData.StepRepeatCountX.push_back(_ttol(CommandResolve(Command, 3)));
                    ((COrder*)pParam)->RepeatData.StepRepeatCountY.push_back(_ttol(CommandResolve(Command, 4)));
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
                        _cwprintf(L"StepRepeatY 處理阻斷位置:");
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
                        _cwprintf(L"StepRepeatX 沒有阻斷\n");
#endif 
                        ((COrder*)pParam)->RepeatData.StepRepeatLabel = CommandResolve(Command, 6);
                    }
                }
                else
                {
#ifdef PRINTF
                    _cwprintf(L"所有條件不成立\n");
#endif 
                }
            }
            else//有StepRepeat時
            {
                UINT StepRepeatNumSize = ((COrder*)pParam)->RepeatData.StepRepeatNum.size();
#ifdef PRINTF
                _cwprintf(L"目前StepRepeatNumSize:%d\n", StepRepeatNumSize);
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
                                    _cwprintf(L"第%d:SSwitch轉換:%d\n", i, ((COrder*)pParam)->RepeatData.SSwitch.at(i));
#endif 
                                }          
                                else  if (((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y == ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.at(i))
                                {
                                    ((COrder*)pParam)->RepeatData.SSwitch.at(i) = TRUE; 
#ifdef PRINTF
                                    _cwprintf(L"第%d:SSwitch轉換:%d\n", i, ((COrder*)pParam)->RepeatData.SSwitch.at(i));
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
                            _cwprintf(L"第%d:SSwitch轉換:%d\n", i, ((COrder*)pParam)->RepeatData.SSwitch.at(i));
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
                            _cwprintf(L"刪除所有陣列\n");
#endif 
                            ((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum++;
#ifdef PRINTF
                            _cwprintf(L"刪除總數+1=%d\n", ((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum);
#endif 
                            ((COrder*)pParam)->RepeatData.StepRepeatBlockData.erase(((COrder*)pParam)->RepeatData.StepRepeatBlockData.begin() + i);
#ifdef PRINTF
                            _cwprintf(L"刪除StepRepeatBlockData陣列\n");
#endif 
                            ((COrder*)pParam)->RepeatData.SSwitch.erase(((COrder*)pParam)->RepeatData.SSwitch.begin() + i);
#ifdef PRINTF
                            _cwprintf(L"刪除SSwitch陣列\n");
#endif 
                            ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X = ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.at(i);
                            ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y = ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.at(i);
                            ((COrder*)pParam)->RepeatData.StepRepeatNum.erase(((COrder*)pParam)->RepeatData.StepRepeatNum.begin() + i);
#ifdef PRINTF
                            _cwprintf(L"刪除StepRepeatNum陣列\n");
#endif 
                            ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.erase(((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.begin() + i);
#ifdef PRINTF
                            _cwprintf(L"刪除StepRepeatInitOffsetX陣列\n");
#endif 
                            ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.erase(((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.begin() + i);
#ifdef PRINTF
                            _cwprintf(L"刪除StepRepeatInitOffsetY陣列\n");
#endif 
                            ((COrder*)pParam)->RepeatData.StepRepeatCountX.erase(((COrder*)pParam)->RepeatData.StepRepeatCountX.begin() + i);
#ifdef PRINTF
                            _cwprintf(L"刪除StepRepeatCountX陣列\n");
#endif 
                            ((COrder*)pParam)->RepeatData.StepRepeatCountY.erase(((COrder*)pParam)->RepeatData.StepRepeatCountY.begin() + i);
#ifdef PRINTF
                            _cwprintf(L"刪除StepRepeatCountY陣列\n");
#endif 
                            break;          
                        }
                        if (((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum != 0 && ((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum != ((COrder*)pParam)->RepeatData.AllNewStepRepeatNum)
                        {
#ifdef PRINTF
                            _cwprintf(L"進入新增內層迴圈");
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
                            if (((COrder*)pParam)->RepeatData.StepRepeatLabelLock && !((COrder*)pParam)->RepeatData.AddInStepRepeatSwitch)//第一次進入SetpRepeat時(內層迴圈)
                            {
#ifdef PRINTF
                                _cwprintf(L"第一次新增內層StepRepeat\n");
#endif 
                                //StepRepeat計數總數++
                                ((COrder*)pParam)->RepeatData.AllNewStepRepeatNum++;
                                //紀錄StepRepeat地址
                                ((COrder*)pParam)->RepeatData.StepRepeatNum.push_back(((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)));
                                //紀錄初始offset位置
                                ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.push_back(((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X);
                                ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.push_back(((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y);
                                //紀錄X、Y計數
                                ((COrder*)pParam)->RepeatData.StepRepeatCountX.push_back(_ttol(CommandResolve(Command, 3)));
                                ((COrder*)pParam)->RepeatData.StepRepeatCountY.push_back(_ttol(CommandResolve(Command, 4)));
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
                                    _cwprintf(L"StepRepeatX 處理阻斷位置:");
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
                                    _cwprintf(L"StepRepeatX 沒有阻斷\n");
#endif 
                                    ((COrder*)pParam)->RepeatData.StepRepeatLabel = CommandResolve(Command, 6);
                                }
                            }
                            else //第二次進入StepRepeat時(內層迴圈做第N次)
                            {
#ifdef PRINTF
                                _cwprintf(L"第二次新增內層StepRepeat\n");
#endif 
                                ((COrder*)pParam)->RepeatData.AddInStepRepeatSwitch = FALSE;
                                ((COrder*)pParam)->RepeatData.StepRepeatLabelLock = FALSE;
                                //紀錄StepRepeat地址
                                ((COrder*)pParam)->RepeatData.StepRepeatNum.push_back(((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)));
                                //紀錄初始offset位置
                                ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.push_back(((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X);
                                ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.push_back(((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y);
                                //紀錄X、Y計數
                                ((COrder*)pParam)->RepeatData.StepRepeatCountX.push_back(_ttol(CommandResolve(Command, 3)));
                                ((COrder*)pParam)->RepeatData.StepRepeatCountY.push_back(_ttol(CommandResolve(Command, 4)));
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
                                    _cwprintf(L"StepRepeatX 處理阻斷位置:");
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
                                    _cwprintf(L"StepRepeatX 沒有阻斷\n");
#endif 
                                    if (((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum != 0 && ((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum != ((COrder*)pParam)->RepeatData.AllNewStepRepeatNum)
                                    {
#ifdef PRINTF
                                        _cwprintf(L"進入新增內層迴圈");
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
        else //當只有1-1時
        {   
        }
    }
    if (CommandResolve(Command, 0) == L"CallSubProgram")
    {
        ((COrder*)pParam)->RunData.SubProgramName = _T("SubProgramStart,") + CommandResolve(Command, 1);
    }
    if (CommandResolve(Command, 0) == L"SubProgramEnd")
    {
        ((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)) = 0;//副程序計數清零
        ((COrder*)pParam)->RunData.MSChange.pop_back();
        ((COrder*)pParam)->RunData.StackingCount--; 
        ((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount))--;//上次執行過後有+1所以要減回來
    }

    if (CommandResolve(Command, 0) == L"End") 
    {
        if (((COrder*)pParam)->ModelControl.Mode == 1 )
        {
            //TODO::DEMO所以加入
            if (!((COrder*)pParam)->DemoTemprarilySwitch)//如果DemoTemprarilySwitch為TRUE跳過模式二
            {
                if (((COrder*)pParam)->ModelControl.LaserModeJump)
                {
                    ((COrder*)pParam)->ModelControl.Mode = 3;
                }
                else
                {
                    ((COrder*)pParam)->ModelControl.Mode = 2;
                }    
            }
            else
            {
                ((COrder*)pParam)->ModelControl.Mode = 3;
            }
            //虛擬紀錄座標歸零
            ((COrder*)pParam)->VirtualCoordinateData = { 0,0,0,0 };
            ((COrder*)pParam)->NVMVirtualCoordinateData = { 0,0,0,0 };
        }
        else if (((COrder*)pParam)->ModelControl.Mode == 2)
        {
            ((COrder*)pParam)->ModelControl.Mode = 3;
            //虛擬紀錄座標歸零
            ((COrder*)pParam)->VirtualCoordinateData = { 0,0,0,0 };
            ((COrder*)pParam)->NVMVirtualCoordinateData = { 0,0,0,0 };
        }
        else if (((COrder*)pParam)->ModelControl.Mode == 3)
        {
            ((COrder*)pParam)->ModelControl.Mode = 4;//結束程序
        }
    }
    /************************************************************動作**************************************************************/
#ifdef MOVE
    if (CommandResolve(Command, 0) == L"Dot")
    {
        if (((COrder*)pParam)->ModelControl.Mode == 1)//影像模式
        {
            RecordCorrectionTable(pParam);//寫入修正表
            VirtualCoordinateMove(pParam, Command, 1);//虛擬座標移動
        }
        else if (((COrder*)pParam)->ModelControl.Mode == 2)//雷射模式
        {
            LaserDetectHandle(pParam, Command);//雷射測高
            VirtualCoordinateMove(pParam, Command, 1);//虛擬座標移動
        }
        else if (((COrder*)pParam)->ModelControl.Mode == 3)//運動模式
        {
            ((COrder*)pParam)->ActionCount++;//動作計數++
            ChooseVisionModify(pParam);//選擇影像Offset
            ChooseLaserModify(pParam);//選擇雷射高度
            ModifyPointOffSet(pParam, Command);//CallSubroutin相對位修正
            LineGotoActionJudge(pParam);//判斷動作狀態
            ((COrder*)pParam)->FinalWorkCoordinateData.X = _ttoi(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X;
            ((COrder*)pParam)->FinalWorkCoordinateData.Y = _ttoi(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y;
            ((COrder*)pParam)->FinalWorkCoordinateData.Z = _ttoi(CommandResolve(Command, 3)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z;
            
            ((COrder*)pParam)->NVMVirtualCoordinateData = ((COrder*)pParam)->FinalWorkCoordinateData;//紀錄CallSubroutine點(不加影像修正時的值)

            VisionModify(pParam);//影像修正
            LaserModify(pParam);//雷射修正
            ((COrder*)pParam)->m_Action.DecidePointGlue(
                //_ttoi(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X + ((COrder*)pParam)->VisionOffset.OffsetX,
                //_ttoi(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y + ((COrder*)pParam)->VisionOffset.OffsetY,
                //_ttoi(CommandResolve(Command, 3)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z + ((COrder*)pParam)->VisionOffset.OffsetZ,
                ((COrder*)pParam)->FinalWorkCoordinateData.X,
                ((COrder*)pParam)->FinalWorkCoordinateData.Y,
                ((COrder*)pParam)->FinalWorkCoordinateData.Z,
                ((COrder*)pParam)->DispenseDotSet.GlueOpenTime, ((COrder*)pParam)->DispenseDotSet.GlueCloseTime,
                ((COrder*)pParam)->ZSet.ZBackHeight, ((COrder*)pParam)->ZSet.ZBackType, ((COrder*)pParam)->DispenseDotEnd.RiseDistance, ((COrder*)pParam)->DispenseDotEnd.RiseHightSpeed, ((COrder*)pParam)->DispenseDotEnd.RiseLowSpeed,
                ((COrder*)pParam)->DotSpeedSet.EndSpeed, ((COrder*)pParam)->DotSpeedSet.AccSpeed, 6000);

            ((COrder*)pParam)->VirtualCoordinateData = ((COrder*)pParam)->FinalWorkCoordinateData;//紀錄移動虛擬座標
        }
    }
    if (CommandResolve(Command, 0) == L"LineStart")
    {
        if (((COrder*)pParam)->ModelControl.Mode == 1)//影像模式
        {
            RecordCorrectionTable(pParam);//寫入修正表
            VirtualCoordinateMove(pParam, Command, 1);//虛擬座標移動         
        }
        else if (((COrder*)pParam)->ModelControl.Mode == 2)//雷射模式
        {
            LaserDetectHandle(pParam, Command);//雷射測高
            VirtualCoordinateMove(pParam, Command, 1);//虛擬座標移動
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
            
            /*因為不加入修正所以必須在這裡做汰換*/
            ((COrder*)pParam)->NVMVirtualCoordinateData = ((COrder*)pParam)->FinalWorkCoordinateData;//紀錄CallSubroutine點(不加影像修正時的值)
            
            VisionModify(pParam);//影像修正
            LaserModify(pParam);//雷射修正
            ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Status = TRUE;
            //((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).X = _ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X;
            //((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Y = _ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y;
            //((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Z = _ttol(CommandResolve(Command, 3)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z;
            ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).X = ((COrder*)pParam)->FinalWorkCoordinateData.X;
            ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Y = ((COrder*)pParam)->FinalWorkCoordinateData.Y;
            ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Z = ((COrder*)pParam)->FinalWorkCoordinateData.Z;
            ((COrder*)pParam)->RunData.ActionStatus.at(((COrder*)pParam)->Program.SubroutinCount) = 1;
        }    
    }
    if (CommandResolve(Command, 0) == L"LinePassing")
    {
        if (((COrder*)pParam)->ModelControl.Mode == 1)//影像模式
        {
            RecordCorrectionTable(pParam);//寫入修正表
            VirtualCoordinateMove(pParam, Command, 2);//虛擬座標移動              
        }
        else if (((COrder*)pParam)->ModelControl.Mode == 2)//雷射模式
        {
            LaserDetectHandle(pParam, Command);//雷射測高
            VirtualCoordinateMove(pParam, Command, 2);//虛擬座標移動
        }
        else if (((COrder*)pParam)->ModelControl.Mode == 3)//運動模式
        {
            ((COrder*)pParam)->ActionCount++;//動作計數++
            ChooseVisionModify(pParam);//選擇影像Offset
            ChooseLaserModify(pParam);//選擇雷射高度
            ModifyPointOffSet(pParam, Command);//CallSubroutin相對位修正
            if (((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Status)
            {
                ((COrder*)pParam)->FinalWorkCoordinateData.X = _ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X;
                ((COrder*)pParam)->FinalWorkCoordinateData.Y = _ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y;
                ((COrder*)pParam)->FinalWorkCoordinateData.Z = _ttol(CommandResolve(Command, 3)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z;
                
                ((COrder*)pParam)->NVMVirtualCoordinateData = ((COrder*)pParam)->FinalWorkCoordinateData;//紀錄CallSubroutine點(不加影像修正時的值)
                
                VisionModify(pParam);//影像修正
                LaserModify(pParam);//雷射修正
                if (!((COrder*)pParam)->ContinuousSwitch)
                {
                    if (((COrder*)pParam)->RunData.ActionStatus.at(((COrder*)pParam)->Program.SubroutinCount) == 1)//LS存在尚未執行過LP
                    {
                        if (((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Status)//執行圓弧
                        {
                            //呼叫LS 
                            ((COrder*)pParam)->m_Action.DecideLineStartMove(
                                ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).X,
                                ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
                                ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Z,
                                ((COrder*)pParam)->DispenseLineSet.BeforeMoveDelay, ((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, 6000);
                            //呼叫ARC
                            ((COrder*)pParam)->m_Action.DecideArc(
                                ((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).X,
                                ((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
                                ((COrder*)pParam)->FinalWorkCoordinateData.X,
                                ((COrder*)pParam)->FinalWorkCoordinateData.Y,
                                //_ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X,
                                //_ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
                                ((COrder*)pParam)->LineSpeedSet.EndSpeed, 1000);
                            //清除完成動作弧
                            ((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Status = FALSE;
                        }
                        else if (((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Status)//執行圓
                        {
                            //呼叫LS
                            ((COrder*)pParam)->m_Action.DecideLineStartMove(
                                ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).X,
                                ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
                                ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Z,
                                ((COrder*)pParam)->DispenseLineSet.BeforeMoveDelay, ((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, 6000);
                            //呼叫Circle
                            ((COrder*)pParam)->m_Action.DecideCircle(((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Y, ((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount).Y,
                                ((COrder*)pParam)->LineSpeedSet.EndSpeed, 1000);
                            //呼叫LP
                            ((COrder*)pParam)->m_Action.DecideLineMidMove(
                                //_ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X,
                                //_ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
                                //_ttol(CommandResolve(Command, 3)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z,
                                ((COrder*)pParam)->FinalWorkCoordinateData.X,
                                ((COrder*)pParam)->FinalWorkCoordinateData.Y,
                                ((COrder*)pParam)->FinalWorkCoordinateData.Z,
                                ((COrder*)pParam)->DispenseLineSet.NodeTime, ((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, 6000);
                            //清除完成動作圓
                            ((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Status = FALSE;
                        }
                        else//執行線段
                        {
                            //呼叫LS-LP
                            ((COrder*)pParam)->m_Action.DecideLineSToP(((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Y, ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Z,
                                //_ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X,
                                //_ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
                                //_ttol(CommandResolve(Command, 3)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z,
                                ((COrder*)pParam)->FinalWorkCoordinateData.X,
                                ((COrder*)pParam)->FinalWorkCoordinateData.Y,
                                ((COrder*)pParam)->FinalWorkCoordinateData.Z,
                                ((COrder*)pParam)->DispenseLineSet.BeforeMoveDelay, ((COrder*)pParam)->DispenseLineSet.BeforeMoveDistance, ((COrder*)pParam)->DispenseLineSet.NodeTime,
                                ((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, 6000);
                        }
                    }
                    else if (((COrder*)pParam)->RunData.ActionStatus.at(((COrder*)pParam)->Program.SubroutinCount) == 2)//LS存在執行過LP
                    {
                        if (((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Status)//執行圓弧
                        {
                            //呼叫ARC
                            ((COrder*)pParam)->m_Action.DecideArc(((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
                                //_ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X,
                                //_ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
                                ((COrder*)pParam)->FinalWorkCoordinateData.X,
                                ((COrder*)pParam)->FinalWorkCoordinateData.Y,
                                ((COrder*)pParam)->LineSpeedSet.EndSpeed, 1000);
                            //清除完成動作弧
                            ((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Status = FALSE;
                        }
                        else if (((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Status)//執行圓
                        {
                            //呼叫Circle
                            ((COrder*)pParam)->m_Action.DecideCircle(((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Y, ((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount).Y,
                                ((COrder*)pParam)->LineSpeedSet.EndSpeed, 1000);
                            //呼叫LP
                            ((COrder*)pParam)->m_Action.DecideLineMidMove(
                                //_ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X,
                                //_ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
                                ((COrder*)pParam)->FinalWorkCoordinateData.X,
                                ((COrder*)pParam)->FinalWorkCoordinateData.Y,
                                _ttol(CommandResolve(Command, 3)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z,
                                ((COrder*)pParam)->DispenseLineSet.NodeTime, ((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, 6000);
                            //清除完成動作圓
                            ((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Status = FALSE;
                        }
                        else//執行線段
                        {
                            //呼叫LP
                            ((COrder*)pParam)->m_Action.DecideLineMidMove(
                                //_ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X,
                                //_ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
                                //_ttol(CommandResolve(Command, 3)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z,
                                ((COrder*)pParam)->FinalWorkCoordinateData.X,
                                ((COrder*)pParam)->FinalWorkCoordinateData.Y,
                                ((COrder*)pParam)->FinalWorkCoordinateData.Z,
                                ((COrder*)pParam)->DispenseLineSet.NodeTime, ((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, 6000);
                        }
                    }
                }
                else
                {
                    /*中間點不選擇線段*/
#ifdef PRINTF
                    _cwprintf(L"選擇線段:%d\n", abs(((COrder*)pParam)->FinalWorkCoordinateData.Z + 10000));  
                    
#endif  
                    //TODO::DEMO所以加入
                    if (!((COrder*)pParam)->DemoTemprarilySwitch)
                    {
                        ((COrder*)pParam)->m_Action.LA_Line3DtoDo(abs(((COrder*)pParam)->FinalWorkCoordinateData.Z + 10000),
                            ((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, 6000, 1);
                        ((COrder*)pParam)->ContinuousSwitch = FALSE;
                    }
                    else
                    {
                        /*((COrder*)pParam)->m_Action.LA_CorrectVectorToDo(((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, 6000,
                            ((COrder*)pParam)->VisionOffset.Contraposition.X, ((COrder*)pParam)->VisionOffset.Contraposition.Y,
                            ((COrder*)pParam)->VisionOffset.OffsetX, ((COrder*)pParam)->VisionOffset.OffsetY, ((COrder*)pParam)->VisionOffset.Angle,
                            ((COrder*)pParam)->VisionSet.AdjustOffsetX, ((COrder*)pParam)->VisionSet.AdjustOffsetY, ((COrder*)pParam)->VisionSet.ModifyMode);
                        ((COrder*)pParam)->ContinuousSwitch = FALSE;*/
                    }                
                }
                ((COrder*)pParam)->RunData.ActionStatus.at(((COrder*)pParam)->Program.SubroutinCount) = 2;//狀態變為線段中

                ((COrder*)pParam)->VirtualCoordinateData = ((COrder*)pParam)->FinalWorkCoordinateData;//紀錄移動虛擬座標
            }
        }    
    }
    if (CommandResolve(Command, 0) == L"LineEnd")
    {
        if (((COrder*)pParam)->ModelControl.Mode == 1)//影像模式
        {
            RecordCorrectionTable(pParam);//寫入修正表
            VirtualCoordinateMove(pParam, Command, 2);//虛擬座標移動            
        }
        else if (((COrder*)pParam)->ModelControl.Mode == 2)//雷射模式
        {
            LaserDetectHandle(pParam, Command);//雷射測高
            VirtualCoordinateMove(pParam, Command, 2);//虛擬座標移動
        }
        else if (((COrder*)pParam)->ModelControl.Mode == 3)//運動模式
        {
            ((COrder*)pParam)->ActionCount++;//動作計數++
            ChooseVisionModify(pParam);//選擇影像Offset
            ChooseLaserModify(pParam);//選擇雷射高度
            ModifyPointOffSet(pParam, Command);//CallSubroutin相對位修正
            if (((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Status)
            {
                ((COrder*)pParam)->FinalWorkCoordinateData.X = _ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X;
                ((COrder*)pParam)->FinalWorkCoordinateData.Y = _ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y;
                ((COrder*)pParam)->FinalWorkCoordinateData.Z = _ttol(CommandResolve(Command, 3)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z;

                ((COrder*)pParam)->NVMVirtualCoordinateData = ((COrder*)pParam)->FinalWorkCoordinateData;//紀錄CallSubroutine點(不加影像修正時的值)
                
                VisionModify(pParam);//影像修正
                LaserModify(pParam);//雷射修正
                if (!((COrder*)pParam)->ContinuousSwitch)
                {
                    if (((COrder*)pParam)->RunData.ActionStatus.at(((COrder*)pParam)->Program.SubroutinCount) == 1)//LS存在且尚未執行過
                    {
                        if (((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Status)//執行圓弧
                        {
                            //呼叫一個LS
                            ((COrder*)pParam)->m_Action.DecideLineStartMove(
                                ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).X,
                                ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
                                ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Z,
                                ((COrder*)pParam)->DispenseLineSet.BeforeMoveDelay, ((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, 6000);
                            //再呼叫一個ARC-LE
                            ((COrder*)pParam)->m_Action.DecideArcleToEnd(
                                ((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).X,
                                ((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
                                //_ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X,
                                //_ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
                                ((COrder*)pParam)->FinalWorkCoordinateData.X,
                                ((COrder*)pParam)->FinalWorkCoordinateData.Y,
                                ((COrder*)pParam)->DispenseLineSet.StayTime, ((COrder*)pParam)->DispenseLineSet.ShutdownDistance, ((COrder*)pParam)->DispenseLineSet.ShutdownDelay,
                                ((COrder*)pParam)->ZSet.ZBackHeight, ((COrder*)pParam)->ZSet.ZBackType,
                                ((COrder*)pParam)->DispenseLineEnd.HighSpeed, ((COrder*)pParam)->DispenseLineEnd.Width, ((COrder*)pParam)->DispenseLineEnd.Height, ((COrder*)pParam)->DispenseLineEnd.LowSpeed, ((COrder*)pParam)->DispenseLineEnd.Type,
                                ((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, 1000);
                            //清除完成動作弧
                            ((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Status = FALSE;
                        }
                        else if (((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Status)//執行圓
                        {
                            //呼叫LS
                            ((COrder*)pParam)->m_Action.DecideLineStartMove(
                                ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).X,
                                ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
                                ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Z,
                                ((COrder*)pParam)->DispenseLineSet.BeforeMoveDelay, ((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, 6000);
                            //呼叫Circle-LE
                            ((COrder*)pParam)->m_Action.DecideCircleToEnd(((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Y, ((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount).Y,
                                //_ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X,
                                //_ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
                                ((COrder*)pParam)->FinalWorkCoordinateData.X,
                                ((COrder*)pParam)->FinalWorkCoordinateData.Y,
                                ((COrder*)pParam)->DispenseLineSet.StayTime, ((COrder*)pParam)->DispenseLineSet.ShutdownDistance, ((COrder*)pParam)->DispenseLineSet.ShutdownDelay,
                                ((COrder*)pParam)->ZSet.ZBackHeight, ((COrder*)pParam)->ZSet.ZBackType,
                                ((COrder*)pParam)->DispenseLineEnd.HighSpeed, ((COrder*)pParam)->DispenseLineEnd.Width, ((COrder*)pParam)->DispenseLineEnd.Height, ((COrder*)pParam)->DispenseLineEnd.LowSpeed, ((COrder*)pParam)->DispenseLineEnd.Type,
                                ((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, 1000);
                            //清除完成動作圓
                            ((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Status = FALSE;
                        }
                        else//執行線段
                        {
                            //呼叫LS-LE
                            ((COrder*)pParam)->m_Action.DecideLineSToE(
                                ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).X,
                                ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
                                ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Z,
                                //_ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X,
                                //_ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
                                //_ttol(CommandResolve(Command, 3)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z,
                                ((COrder*)pParam)->FinalWorkCoordinateData.X,
                                ((COrder*)pParam)->FinalWorkCoordinateData.Y,
                                ((COrder*)pParam)->FinalWorkCoordinateData.Z,
                                ((COrder*)pParam)->DispenseLineSet.BeforeMoveDelay, ((COrder*)pParam)->DispenseLineSet.BeforeMoveDistance,
                                ((COrder*)pParam)->DispenseLineSet.StayTime, ((COrder*)pParam)->DispenseLineSet.ShutdownDistance, ((COrder*)pParam)->DispenseLineSet.ShutdownDelay,
                                ((COrder*)pParam)->ZSet.ZBackHeight, ((COrder*)pParam)->ZSet.ZBackType,
                                ((COrder*)pParam)->DispenseLineEnd.HighSpeed, ((COrder*)pParam)->DispenseLineEnd.Width, ((COrder*)pParam)->DispenseLineEnd.Height, ((COrder*)pParam)->DispenseLineEnd.LowSpeed, ((COrder*)pParam)->DispenseLineEnd.Type,
                                ((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, 6000);
                        }
                    }
                    else if (((COrder*)pParam)->RunData.ActionStatus.at(((COrder*)pParam)->Program.SubroutinCount) == 2)////LS存在執行過LP
                    {
                        if (((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Status)//執行圓弧
                        {
                            //呼叫ARC-LE
                            ((COrder*)pParam)->m_Action.DecideArcleToEnd(((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
                                //_ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X,
                                //_ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
                                ((COrder*)pParam)->FinalWorkCoordinateData.X,
                                ((COrder*)pParam)->FinalWorkCoordinateData.Y,
                                ((COrder*)pParam)->DispenseLineSet.StayTime, ((COrder*)pParam)->DispenseLineSet.ShutdownDistance, ((COrder*)pParam)->DispenseLineSet.ShutdownDelay,
                                ((COrder*)pParam)->ZSet.ZBackHeight, ((COrder*)pParam)->ZSet.ZBackType,
                                ((COrder*)pParam)->DispenseLineEnd.HighSpeed, ((COrder*)pParam)->DispenseLineEnd.Width, ((COrder*)pParam)->DispenseLineEnd.Height, ((COrder*)pParam)->DispenseLineEnd.LowSpeed, ((COrder*)pParam)->DispenseLineEnd.Type,
                                ((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, 1000);
                            //清除完成動作弧
                            ((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Status = FALSE;
                        }
                        else if (((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Status)//執行圓
                        {
                            //呼叫Circle-LE
                            ((COrder*)pParam)->m_Action.DecideCircleToEnd(((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Y, ((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount).Y,
                                //_ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X,
                                //_ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
                                ((COrder*)pParam)->FinalWorkCoordinateData.X,
                                ((COrder*)pParam)->FinalWorkCoordinateData.Y,
                                ((COrder*)pParam)->DispenseLineSet.StayTime, ((COrder*)pParam)->DispenseLineSet.ShutdownDistance, ((COrder*)pParam)->DispenseLineSet.ShutdownDelay,
                                ((COrder*)pParam)->ZSet.ZBackHeight, ((COrder*)pParam)->ZSet.ZBackType,
                                ((COrder*)pParam)->DispenseLineEnd.HighSpeed, ((COrder*)pParam)->DispenseLineEnd.Width, ((COrder*)pParam)->DispenseLineEnd.Height, ((COrder*)pParam)->DispenseLineEnd.LowSpeed, ((COrder*)pParam)->DispenseLineEnd.Type,
                                ((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, 1000);
                            //清除完成動作圓
                            ((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Status = FALSE;
                        }
                        else//執行線段
                        {
                            //呼叫LE
                            ((COrder*)pParam)->m_Action.DecideLineEndMove(
                                //_ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X,
                                //_ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
                                //_ttol(CommandResolve(Command, 3)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z,
                                ((COrder*)pParam)->FinalWorkCoordinateData.X,
                                ((COrder*)pParam)->FinalWorkCoordinateData.Y,
                                ((COrder*)pParam)->FinalWorkCoordinateData.Z,
                                ((COrder*)pParam)->DispenseLineSet.StayTime, ((COrder*)pParam)->DispenseLineSet.ShutdownDistance, ((COrder*)pParam)->DispenseLineSet.ShutdownDelay,
                                ((COrder*)pParam)->ZSet.ZBackHeight, ((COrder*)pParam)->ZSet.ZBackType,
                                ((COrder*)pParam)->DispenseLineEnd.HighSpeed, ((COrder*)pParam)->DispenseLineEnd.Width, ((COrder*)pParam)->DispenseLineEnd.Height, ((COrder*)pParam)->DispenseLineEnd.LowSpeed, ((COrder*)pParam)->DispenseLineEnd.Type,
                                ((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, 6000);
                        }
                    }
                }
                else
                {
#ifdef PRINTF
                    _cwprintf(L"選擇線段:%d\n", abs(((COrder*)pParam)->FinalWorkCoordinateData.Z + 10000));
#endif  
                    if (!((COrder*)pParam)->DemoTemprarilySwitch)
                    {
                        ((COrder*)pParam)->m_Action.LA_Line3DtoDo(abs(((COrder*)pParam)->FinalWorkCoordinateData.Z + 10000),
                            ((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, 6000, 1);
                        //呼叫斷膠抬升
                        ((COrder*)pParam)->m_Action.DecideLineEndMove(((COrder*)pParam)->DispenseLineSet.StayTime, ((COrder*)pParam)->DispenseLineSet.ShutdownDelay,
                            ((COrder*)pParam)->ZSet.ZBackHeight, ((COrder*)pParam)->ZSet.ZBackType, ((COrder*)pParam)->DispenseLineEnd.HighSpeed, ((COrder*)pParam)->DispenseLineEnd.Width, ((COrder*)pParam)->DispenseLineEnd.Height, ((COrder*)pParam)->DispenseLineEnd.LowSpeed,
                            ((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, 6000);
                        ((COrder*)pParam)->ContinuousSwitch = FALSE;
                    }
                    else
                    {
                        ((COrder*)pParam)->m_Action.LA_CorrectVectorToDo(((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, 6000,
                            ((COrder*)pParam)->VisionOffset.Contraposition.X, ((COrder*)pParam)->VisionOffset.Contraposition.Y,
                            ((COrder*)pParam)->VisionOffset.OffsetX, ((COrder*)pParam)->VisionOffset.OffsetY, ((COrder*)pParam)->VisionOffset.Angle,
                            ((COrder*)pParam)->VisionSet.AdjustOffsetX, ((COrder*)pParam)->VisionSet.AdjustOffsetY, ((COrder*)pParam)->VisionSet.ModifyMode,
                            ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y);
                        //呼叫斷膠抬升
                        ((COrder*)pParam)->m_Action.DecideLineEndMove(((COrder*)pParam)->DispenseLineSet.StayTime, ((COrder*)pParam)->DispenseLineSet.ShutdownDelay,
                            ((COrder*)pParam)->ZSet.ZBackHeight, ((COrder*)pParam)->ZSet.ZBackType, ((COrder*)pParam)->DispenseLineEnd.HighSpeed, ((COrder*)pParam)->DispenseLineEnd.Width, ((COrder*)pParam)->DispenseLineEnd.Height, ((COrder*)pParam)->DispenseLineEnd.LowSpeed,
                            ((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, 6000);
                        ((COrder*)pParam)->ContinuousSwitch = FALSE;

                    }
                }
            }
            ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Status = FALSE;
            ((COrder*)pParam)->RunData.ActionStatus.at(((COrder*)pParam)->Program.SubroutinCount) = 0;

            ((COrder*)pParam)->VirtualCoordinateData = ((COrder*)pParam)->FinalWorkCoordinateData;//紀錄移動虛擬座標
        }   
    }
    if (CommandResolve(Command, 0) == L"ArcPoint")
    {
        if (((COrder*)pParam)->ModelControl.Mode == 1)//影像模式
        {
            RecordCorrectionTable(pParam);//寫入修正表
            ModifyPointOffSet(pParam, Command);//CallSubroutin相對位修正
        }
        else if (((COrder*)pParam)->ModelControl.Mode == 3 || ((COrder*)pParam)->ModelControl.Mode == 2)//運動、雷射模式
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
            VisionModify(pParam);//影像修正
            ((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Status = TRUE;
            ((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).X = ((COrder*)pParam)->FinalWorkCoordinateData.X;
            ((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Y = ((COrder*)pParam)->FinalWorkCoordinateData.Y;
            //((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).X = _ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X;
            //((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Y = _ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y;
        }   
    }
    if (CommandResolve(Command, 0) == L"CirclePoint")
    {
        if (((COrder*)pParam)->ModelControl.Mode == 1)//影像模式
        {
            RecordCorrectionTable(pParam);//寫入修正表
            ModifyPointOffSet(pParam, Command);//CallSubroutin相對位修正
        }
        else if (((COrder*)pParam)->ModelControl.Mode == 3 || ((COrder*)pParam)->ModelControl.Mode == 2)//運動、雷射模式
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
            ((COrder*)pParam)->FinalWorkCoordinateData.X = _ttol(CommandResolve(Command, 4)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X;
            ((COrder*)pParam)->FinalWorkCoordinateData.Y = _ttol(CommandResolve(Command, 5)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y;
            VisionModify(pParam);//影像修正
            ((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount).X = ((COrder*)pParam)->FinalWorkCoordinateData.X;
            ((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount).Y = ((COrder*)pParam)->FinalWorkCoordinateData.Y;


            //((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).X = _ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X;
            //((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Y = _ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y;
            ((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Z = _ttol(CommandResolve(Command, 3)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z;
            //((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount).X = _ttol(CommandResolve(Command, 4)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X;
            //((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount).Y = _ttol(CommandResolve(Command, 5)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y;
            ((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount).Z = _ttol(CommandResolve(Command, 6)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z;
        }
    }
    if (CommandResolve(Command, 0) == L"GoHome")
    {
        if (((COrder*)pParam)->ModelControl.Mode == 1)//影像模式
        {    
            VirtualCoordinateMove(pParam, Command, 1);//虛擬座標移動
        }
        else if (((COrder*)pParam)->ModelControl.Mode == 2)//雷射模式
        {
            VirtualCoordinateMove(pParam, Command, 1);//虛擬座標移動
        }
        else if (((COrder*)pParam)->ModelControl.Mode == 3)//運動模式
        {
            LineGotoActionJudge(pParam);//判斷動作狀態
            if (((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Status)//已經有offset修正
            {
                ((COrder*)pParam)->m_Action.DecideInitializationMachine(((COrder*)pParam)->GoHome.Speed1, ((COrder*)pParam)->GoHome.Speed2, ((COrder*)pParam)->GoHome.Axis, ((COrder*)pParam)->GoHome.MoveX, ((COrder*)pParam)->GoHome.MoveY, ((COrder*)pParam)->GoHome.MoveZ);
                
                ((COrder*)pParam)->VirtualCoordinateData = { 0,0,0,0 };//紀錄移動虛擬座標
            }
            else
            {
                ((COrder*)pParam)->m_Action.DecideInitializationMachine(((COrder*)pParam)->GoHome.Speed1, ((COrder*)pParam)->GoHome.Speed2, ((COrder*)pParam)->GoHome.Axis, ((COrder*)pParam)->GoHome.MoveX, ((COrder*)pParam)->GoHome.MoveY, ((COrder*)pParam)->GoHome.MoveZ);
                ((COrder*)pParam)->m_Action.DecideVirtualPoint(((COrder*)pParam)->Program.SubroutinePointStack.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->Program.SubroutinePointStack.at(((COrder*)pParam)->Program.SubroutinCount).Y, ((COrder*)pParam)->Program.SubroutinePointStack.at(((COrder*)pParam)->Program.SubroutinCount).Z,
                    ((COrder*)pParam)->DotSpeedSet.EndSpeed, ((COrder*)pParam)->DotSpeedSet.AccSpeed, 6000);//回CallSuboutine紀錄點

                ((COrder*)pParam)->VirtualCoordinateData = { 0,((COrder*)pParam)->Program.SubroutinePointStack.at(((COrder*)pParam)->Program.SubroutinCount).X,((COrder*)pParam)->Program.SubroutinePointStack.at(((COrder*)pParam)->Program.SubroutinCount).Y,((COrder*)pParam)->Program.SubroutinePointStack.at(((COrder*)pParam)->Program.SubroutinCount).Z };//紀錄移動虛擬座標
            }
        } 
    }
    if (CommandResolve(Command, 0) == L"VirtualPoint")
    {
        if (((COrder*)pParam)->ModelControl.Mode == 1)//影像模式
        {
            RecordCorrectionTable(pParam);//寫入修正表
            VirtualCoordinateMove(pParam, Command, 1);//虛擬座標移動
        }
        else if (((COrder*)pParam)->ModelControl.Mode == 2)//雷射模式
        {
            LaserDetectHandle(pParam, Command);//雷射測高
            VirtualCoordinateMove(pParam, Command, 1);//虛擬座標移動
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
           
            ((COrder*)pParam)->NVMVirtualCoordinateData = ((COrder*)pParam)->FinalWorkCoordinateData;//紀錄CallSubroutine點(不加影像修正時的值)
            
            if (_ttol(CommandResolve(Command, 4)) == 1)
            {
                VisionModify(pParam);//影像修正
            }
            else
            {
                ((COrder*)pParam)->VisioModifyJudge = FALSE;//影像開關改變為不修正(用在CallSubroutine)
            }

            LaserModify(pParam);//雷射修正
            ((COrder*)pParam)->m_Action.DecideVirtualPoint(
                //_ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X,
                //_ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
                //_ttol(CommandResolve(Command, 3)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z,
                ((COrder*)pParam)->FinalWorkCoordinateData.X,
                ((COrder*)pParam)->FinalWorkCoordinateData.Y,
                ((COrder*)pParam)->FinalWorkCoordinateData.Z,
                ((COrder*)pParam)->DotSpeedSet.EndSpeed, ((COrder*)pParam)->DotSpeedSet.AccSpeed, 6000);

            ((COrder*)pParam)->VirtualCoordinateData = ((COrder*)pParam)->FinalWorkCoordinateData;//紀錄移動虛擬座標
        } 
    }
    if (CommandResolve(Command, 0) == L"WaitPoint")
    {
        if (((COrder*)pParam)->ModelControl.Mode == 1)//影像模式
        {
            RecordCorrectionTable(pParam);//寫入修正表
            VirtualCoordinateMove(pParam, Command, 1);//虛擬座標移動
        }
        else if (((COrder*)pParam)->ModelControl.Mode == 2)//雷射模式
        {
            LaserDetectHandle(pParam, Command);//雷射測高
            VirtualCoordinateMove(pParam, Command, 1);//虛擬座標移動
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
            ((COrder*)pParam)->m_Action.DecideWaitPoint(
                //_ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X,
                //_ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
                //_ttol(CommandResolve(Command, 3)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z,
                ((COrder*)pParam)->FinalWorkCoordinateData.X,
                ((COrder*)pParam)->FinalWorkCoordinateData.Y,
                ((COrder*)pParam)->FinalWorkCoordinateData.Z,
                _ttol(CommandResolve(Command, 4)), ((COrder*)pParam)->DotSpeedSet.EndSpeed, ((COrder*)pParam)->DotSpeedSet.AccSpeed, 6000);

            ((COrder*)pParam)->VirtualCoordinateData = ((COrder*)pParam)->FinalWorkCoordinateData;//紀錄移動虛擬座標
        }  
    }
    if (CommandResolve(Command, 0) == L"ParkPosition")
    {
        if (((COrder*)pParam)->ModelControl.Mode == 1)//影像模式
        {
            VirtualCoordinateMove(pParam, Command, 1);//虛擬座標移動
        }
        else if (((COrder*)pParam)->ModelControl.Mode == 2)//雷射模式
        {
            VirtualCoordinateMove(pParam, Command, 1);//虛擬座標移動
        }
        else if (((COrder*)pParam)->ModelControl.Mode == 3)//運動模式
        {
            LineGotoActionJudge(pParam);//判斷動作狀態
            if (((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Status)//已經有offset修正
            {
                if (((COrder*)pParam)->GlueData.GlueAuto)
                {
                    ((COrder*)pParam)->m_Action.DecideParkPoint(((COrder*)pParam)->GlueData.ParkPositionData.X, ((COrder*)pParam)->GlueData.ParkPositionData.Y, ((COrder*)pParam)->GlueData.ParkPositionData.Z,
                        ((COrder*)pParam)->GlueData.GlueTime, ((COrder*)pParam)->GlueData.GlueWaitTime, ((COrder*)pParam)->GlueData.GlueStayTime,
                        ((COrder*)pParam)->ZSet.ZBackHeight, ((COrder*)pParam)->ZSet.ZBackType, ((COrder*)pParam)->DispenseDotEnd.RiseDistance, ((COrder*)pParam)->DispenseDotEnd.RiseHightSpeed, ((COrder*)pParam)->DispenseDotEnd.RiseLowSpeed,
                        ((COrder*)pParam)->DotSpeedSet.EndSpeed, ((COrder*)pParam)->DotSpeedSet.AccSpeed, 6000);
                    
                    ((COrder*)pParam)->VirtualCoordinateData = { 0,((COrder*)pParam)->GlueData.ParkPositionData.X, ((COrder*)pParam)->GlueData.ParkPositionData.Y, ((COrder*)pParam)->GlueData.ParkPositionData.Z };//紀錄移動虛擬座標
                }
            }
            else
            {
                if (((COrder*)pParam)->GlueData.GlueAuto)
                {
                    ((COrder*)pParam)->m_Action.DecideParkPoint(((COrder*)pParam)->GlueData.ParkPositionData.X, ((COrder*)pParam)->GlueData.ParkPositionData.Y, ((COrder*)pParam)->GlueData.ParkPositionData.Z,
                        ((COrder*)pParam)->GlueData.GlueTime, ((COrder*)pParam)->GlueData.GlueWaitTime, ((COrder*)pParam)->GlueData.GlueStayTime,
                        ((COrder*)pParam)->ZSet.ZBackHeight, ((COrder*)pParam)->ZSet.ZBackType,((COrder*)pParam)->DispenseDotEnd.RiseDistance, ((COrder*)pParam)->DispenseDotEnd.RiseHightSpeed, ((COrder*)pParam)->DispenseDotEnd.RiseLowSpeed,
                        ((COrder*)pParam)->DotSpeedSet.EndSpeed, ((COrder*)pParam)->DotSpeedSet.AccSpeed, 6000);
                }
                ((COrder*)pParam)->m_Action.DecideVirtualPoint(((COrder*)pParam)->Program.SubroutinePointStack.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->Program.SubroutinePointStack.at(((COrder*)pParam)->Program.SubroutinCount).Y, ((COrder*)pParam)->Program.SubroutinePointStack.at(((COrder*)pParam)->Program.SubroutinCount).Z,
                    ((COrder*)pParam)->DotSpeedSet.EndSpeed, ((COrder*)pParam)->DotSpeedSet.AccSpeed, 6000);
                
                ((COrder*)pParam)->VirtualCoordinateData = { 0,((COrder*)pParam)->Program.SubroutinePointStack.at(((COrder*)pParam)->Program.SubroutinCount).X,((COrder*)pParam)->Program.SubroutinePointStack.at(((COrder*)pParam)->Program.SubroutinCount).Y,((COrder*)pParam)->Program.SubroutinePointStack.at(((COrder*)pParam)->Program.SubroutinCount).Z };//紀錄移動虛擬座標
            }
        } 
    }
    if (CommandResolve(Command, 0) == L"StopPoint")
    {
        if (((COrder*)pParam)->ModelControl.Mode == 1)//影像模式
        {
            RecordCorrectionTable(pParam);//寫入修正表
            VirtualCoordinateMove(pParam, Command, 1);//虛擬座標移動
        }
        else if (((COrder*)pParam)->ModelControl.Mode == 2)//雷射模式
        {
            LaserDetectHandle(pParam, Command);//雷射測高 
            VirtualCoordinateMove(pParam, Command, 1);//虛擬座標移動
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
            
            ((COrder*)pParam)->NVMVirtualCoordinateData = ((COrder*)pParam)->FinalWorkCoordinateData;//紀錄CallSubroutine點(不加影像修正時的值)
                   
            if (_ttol(CommandResolve(Command, 4)) == 1)
            {
                VisionModify(pParam);//影像修正
            }
            else
            {
                ((COrder*)pParam)->VisioModifyJudge = FALSE;//影像開關改變為不修正(用在CallSubroutine)
            }

            LaserModify(pParam);
            ((COrder*)pParam)->m_Action.DecideVirtualPoint(
                //_ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X,
                //_ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
                //_ttol(CommandResolve(Command, 3)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z,
                ((COrder*)pParam)->FinalWorkCoordinateData.X,
                ((COrder*)pParam)->FinalWorkCoordinateData.Y,
                ((COrder*)pParam)->FinalWorkCoordinateData.Z,
                ((COrder*)pParam)->DotSpeedSet.EndSpeed, ((COrder*)pParam)->DotSpeedSet.AccSpeed, 6000);
            ((COrder*)pParam)->Pause();

            ((COrder*)pParam)->VirtualCoordinateData = ((COrder*)pParam)->FinalWorkCoordinateData;//紀錄移動虛擬座標
        }     
    }
    if (CommandResolve(Command, 0) == L"FillArea")
    {
        if (((COrder*)pParam)->ModelControl.Mode == 1)//影像模式
        {
            RecordCorrectionTable(pParam);//寫入修正表
            VirtualCoordinateMove(pParam, Command, 1);//虛擬座標移動
        }
        else if (((COrder*)pParam)->ModelControl.Mode == 2)//雷射模式(超高依起始點)
        {
            VirtualCoordinateMove(pParam, Command, 1);//虛擬座標移動
        }
        else if (((COrder*)pParam)->ModelControl.Mode == 3)//運動模式
        {
            CString CommandBuff;
            CommandBuff.Format(_T("FillArea,%d,%d,%d"), _ttol(CommandResolve(Command, 4)), _ttol(CommandResolve(Command, 5)), _ttol(CommandResolve(Command, 6)));
            ChooseVisionModify(pParam);//選擇影像Offset
            ChooseLaserModify(pParam);//選擇雷射高度
            ModifyPointOffSet(pParam, CommandBuff);//CallSubroutin相對位修正
            LineGotoActionJudge(pParam);//判斷動作狀態
            
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
            FillAreaPoint2Z = ((COrder*)pParam)->FinalWorkCoordinateData.Z;

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
                ((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, 6000);

            //紀錄移動虛擬座標
            ((COrder*)pParam)->m_Action.Fill_EndPoint(((COrder*)pParam)->VirtualCoordinateData.X, ((COrder*)pParam)->VirtualCoordinateData.Y,
                FillAreaPoint1X, FillAreaPoint1Y, FillAreaPoint1Z,
                FillAreaPoint2X, FillAreaPoint2Y, FillAreaPoint2Z,
                _ttol(CommandResolve(Command, 1)), _ttol(CommandResolve(Command, 2)), _ttol(CommandResolve(Command, 3))
            );
            ((COrder*)pParam)->VirtualCoordinateData.Z = FillAreaPoint2Z;
        }   
    }
    if (CommandResolve(Command, 0) == L"Output")
    {
        if (((COrder*)pParam)->ModelControl.Mode == 1)//影像模式
        {
        }
        else if (((COrder*)pParam)->ModelControl.Mode == 2)//雷射模式
        {
        }
        else if (((COrder*)pParam)->ModelControl.Mode == 3)//運動模式
        {
            ((COrder*)pParam)->m_Action.DecideOutPutSign(_ttol(CommandResolve(Command, 1)), _ttol(CommandResolve(Command, 2)));
        }    
    }
    if (CommandResolve(Command, 0) == L"Input")
    {
        if (((COrder*)pParam)->ModelControl.Mode == 1)//影像模式
        {
        }
        else if (((COrder*)pParam)->ModelControl.Mode == 2)//雷射模式
        {
        }
        else if (((COrder*)pParam)->ModelControl.Mode == 3)//運動模式
        {
            if (((COrder*)pParam)->m_Action.DecideInPutSign(_ttol(CommandResolve(Command, 1)), _ttol(CommandResolve(Command, 2))) /*&& _ttol(CommandResolve(Command, 3))*/)
            {
                ((COrder*)pParam)->Program.LabelName = _T("Label,") + CommandResolve(Command, 3);//跳到標籤
                                                                                                 //((COrder*)pParam)->RunData.RunCount = _ttol(CommandResolve(Command, 3)) - 2;//跳到地址
            }
        }    
    }
#endif
    /************************************************************運動參數***********************************************************/
    if (CommandResolve(Command, 0) == L"DispenseDotSet")
    {
        if (((COrder*)pParam)->ModelControl.Mode == 1)//影像模式
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
    if (CommandResolve(Command, 0) == L"DispenseDotEnd")
    {
        if (((COrder*)pParam)->ModelControl.Mode == 1)//影像模式
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
    if (CommandResolve(Command, 0) == L"DotSpeedSet")
    {
        if (((COrder*)pParam)->ModelControl.Mode == 1)//影像模式
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
    if (CommandResolve(Command, 0) == L"DotAccPercent")
    {
        if (((COrder*)pParam)->ModelControl.Mode == 1)//影像模式
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
    if (CommandResolve(Command, 0) == L"DispenseLineSet")
    {
        if (((COrder*)pParam)->ModelControl.Mode == 1)//影像模式
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
    if (CommandResolve(Command, 0) == L"DispenseLineEnd")
    {
        if (((COrder*)pParam)->ModelControl.Mode == 1)//影像模式
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
    if (CommandResolve(Command, 0) == L"LineSpeedSet")
    {
        if (((COrder*)pParam)->ModelControl.Mode == 1)//影像模式
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
    if (CommandResolve(Command, 0) == L"ZGoBack")
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
    if (CommandResolve(Command, 0) == L"DispenseAccSet") 
    {
        if (((COrder*)pParam)->ModelControl.Mode == 1)//影像模式
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
    if (CommandResolve(Command, 0) == L"Initialize")
    { 
        if (((COrder*)pParam)->ModelControl.Mode == 1)//影像模式
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
    if (CommandResolve(Command, 0) == L"DispenserSwitch")
    {
        if (((COrder*)pParam)->ModelControl.Mode == 1)//影像模式
        {
        }
        else if (((COrder*)pParam)->ModelControl.Mode == 2)//雷射模式
        {
        }
        else if (((COrder*)pParam)->ModelControl.Mode == 3)//運動模式
        {
            ((COrder*)pParam)->m_Action.g_bIsDispend = _ttol(CommandResolve(Command, 1));
        }  
    }
    if (CommandResolve(Command, 0) == L"DispenserSwitchSet")
    {
        if (((COrder*)pParam)->ModelControl.Mode == 1)//影像模式
        {
        }
        else if (((COrder*)pParam)->ModelControl.Mode == 2)//雷射模式
        {
        }
        else if (((COrder*)pParam)->ModelControl.Mode == 3)//運動模式
        {
            ((COrder*)pParam)->m_Action.g_iNumberGluePort = _ttol(CommandResolve(Command, 1));
        }      
    }
    /************************************************************影像***************************************************************/
#ifdef VI
    if (CommandResolve(Command, 0) == L"FindMark")
    {
        if (((COrder*)pParam)->ModelControl.Mode == 1)//當模式1執行動作
        {
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
        }
        if (((COrder*)pParam)->ModelControl.Mode == 2 || ((COrder*)pParam)->ModelControl.Mode == 3)
        {
            ModifyPointOffSet(pParam, Command);//CallSubroutin相對位修正
        }
    }
    if (CommandResolve(Command, 0) == L"FindMarkAdjust")
    {     
        if (((COrder*)pParam)->ModelControl.Mode == 1)//當模式1執行動作
        {
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
                        ((COrder*)pParam)->DotSpeedSet.EndSpeed, ((COrder*)pParam)->DotSpeedSet.AccSpeed, 6000);
#endif  
                    _cwprintf(L"移動至查找標記\r\n");
                }
                //設定影像參數
                VI_SetPatternMatch(((COrder*)pParam)->FindMark.MilModel, ((COrder*)pParam)->VisionSet.Accuracy, ((COrder*)pParam)->VisionSet.Speed, ((COrder*)pParam)->VisionSet.Score, 0, 360);
                VI_SetSearchRange(((COrder*)pParam)->FindMark.MilModel, ((COrder*)pParam)->VisionSet.width, ((COrder*)pParam)->VisionSet.height);
                if (!((COrder*)pParam)->VisionSerchError.Manuallymode)
                {
                    if (!VI_FindMark(((COrder*)pParam)->FindMark.MilModel, ((COrder*)pParam)->VisionOffset.OffsetX, ((COrder*)pParam)->VisionOffset.OffsetY))
                    {
                        //沒有找到
                        _cwprintf(L"未找到\r\n");
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
                            ((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, 6000,1);
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
                    if (!VI_CameraTrigger(((COrder*)pParam)->FindMark.MilModel, ((COrder*)pParam)->FindMark.Point.X, ((COrder*)pParam)->FindMark.Point.Y, MO_ReadLogicPosition(0), MO_ReadLogicPosition(1), ((COrder*)pParam)->VisionOffset.OffsetX, ((COrder*)pParam)->VisionOffset.OffsetY))
                    {
                        //沒有找到
                        _cwprintf(L"手動模式未找到\r\n");
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
                            ((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, 6000,1);
#endif
                        _cwprintf(L"手動模式找到\r\n");
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
        }
    }
    if (CommandResolve(Command, 0) == L"FiducialMark")
    {
        if (((COrder*)pParam)->ModelControl.Mode == 1) //當模式1執行動作
        {
            ModifyPointOffSet(pParam, Command);//CallSubroutin相對位修正
           /* if (!((COrder*)pParam)->FindMark.Point.Status)//防止混用設計
            {*/
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
            //}
        } 
        if (((COrder*)pParam)->ModelControl.Mode == 2 || ((COrder*)pParam)->ModelControl.Mode == 3)
        {
            ModifyPointOffSet(pParam, Command);//CallSubroutin相對位修正
        }
    }
    if (CommandResolve(Command, 0) == L"FiducialMarkAdjust")
    {
        if (((COrder*)pParam)->ModelControl.Mode == 1) //當模式1執行動作
        {
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
                        ((COrder*)pParam)->DotSpeedSet.EndSpeed, ((COrder*)pParam)->DotSpeedSet.AccSpeed, 6000);
#endif
                    _cwprintf(L"移動至第一點\r\n");
                }
                VI_SetPatternMatch(((COrder*)pParam)->FiducialMark1.MilModel, ((COrder*)pParam)->VisionSet.Accuracy, ((COrder*)pParam)->VisionSet.Speed, ((COrder*)pParam)->VisionSet.Score, ((COrder*)pParam)->VisionSet.Startangle, ((COrder*)pParam)->VisionSet.Endangle);
                VI_SetSearchRange(((COrder*)pParam)->FiducialMark1.MilModel, ((COrder*)pParam)->VisionSet.width, ((COrder*)pParam)->VisionSet.height);
                if (!((COrder*)pParam)->VisionSerchError.Manuallymode && !((COrder*)pParam)->FiducialMark1.FindMarkStatus)//非手動未找到
                {
                    if (!VI_FindMark(((COrder*)pParam)->FiducialMark1.MilModel, ((COrder*)pParam)->FiducialMark1.OffsetX, ((COrder*)pParam)->FiducialMark1.OffsetY))
                    {
                        _cwprintf(L"第一點未找到\r\n");
                        //沒有找到
                        ((COrder*)pParam)->VisionTrigger.AdjustStatus = 2;
                        ((COrder*)pParam)->VisionFindMarkError(pParam);
                    }
                    else
                    {
                        _cwprintf(L"第一點找到\r\n");
                        //找到抬升
#ifdef MOVE
                        //對位完畢不出膠回升
                        ((COrder*)pParam)->m_Action.DecideLineEndMove(0, 0,
                            ((COrder*)pParam)->ZSet.ZBackHeight, ((COrder*)pParam)->ZSet.ZBackType, 0, 0, 0, 0,
                            ((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, 6000,1);
#endif
                        ((COrder*)pParam)->FiducialMark1.FindMarkStatus = TRUE;
                    }
                }
                else if (((COrder*)pParam)->VisionSerchError.Manuallymode && !((COrder*)pParam)->FiducialMark1.FindMarkStatus)//手動未找到
                {
#if defined VI &&  defined MOVE
                    if (!VI_CameraTrigger(((COrder*)pParam)->FiducialMark1.MilModel, ((COrder*)pParam)->FiducialMark1.Point.X, ((COrder*)pParam)->FiducialMark1.Point.Y, MO_ReadLogicPosition(0), MO_ReadLogicPosition(1), ((COrder*)pParam)->FiducialMark1.OffsetX, ((COrder*)pParam)->FiducialMark1.OffsetY))
                    {
                        _cwprintf(L"手動模式第一點未找到\r\n");
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
                            ((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, 6000,1);
#endif
                        ((COrder*)pParam)->VisionSerchError.Manuallymode = FALSE;
                        ((COrder*)pParam)->FiducialMark1.FindMarkStatus = TRUE;
                        _cwprintf(L"手動模式第一點找到\r\n");
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
                        ((COrder*)pParam)->DotSpeedSet.EndSpeed, ((COrder*)pParam)->DotSpeedSet.AccSpeed, 6000);
#endif
                    _cwprintf(L"第一點找到移動至第二點\r\n");
                }
                VI_SetPatternMatch(((COrder*)pParam)->FiducialMark2.MilModel, ((COrder*)pParam)->VisionSet.Accuracy, ((COrder*)pParam)->VisionSet.Speed, ((COrder*)pParam)->VisionSet.Score, ((COrder*)pParam)->VisionSet.Startangle, ((COrder*)pParam)->VisionSet.Endangle);
                VI_SetSearchRange(((COrder*)pParam)->FiducialMark2.MilModel, ((COrder*)pParam)->VisionSet.width, ((COrder*)pParam)->VisionSet.height);
                if (!((COrder*)pParam)->VisionSerchError.Manuallymode && !((COrder*)pParam)->FiducialMark2.FindMarkStatus && ((COrder*)pParam)->FiducialMark1.FindMarkStatus)//非手動且第二點未尋找到但第一點找到
                {
                    if (!VI_FindMark(((COrder*)pParam)->FiducialMark2.MilModel, ((COrder*)pParam)->FiducialMark2.OffsetX, ((COrder*)pParam)->FiducialMark2.OffsetY))
                    {
                        _cwprintf(L"第一點找到移動至第二點未找到\r\n");
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
                            ((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, 6000,1);
#endif
                        ((COrder*)pParam)->FiducialMark2.FindMarkStatus = TRUE;
                        _cwprintf(L"第一點找到移動至第二點找到\r\n");
                    }
                }
                else if (((COrder*)pParam)->VisionSerchError.Manuallymode && !((COrder*)pParam)->FiducialMark2.FindMarkStatus && ((COrder*)pParam)->FiducialMark1.FindMarkStatus)//如果手動模式開啟 且對位點2未找到但對位點1找到
                {
#if defined VI &&  defined MOVE
                    if (!VI_CameraTrigger(((COrder*)pParam)->FiducialMark2.MilModel, ((COrder*)pParam)->FiducialMark2.Point.X, ((COrder*)pParam)->FiducialMark2.Point.Y, MO_ReadLogicPosition(0), MO_ReadLogicPosition(1), ((COrder*)pParam)->FiducialMark2.OffsetX, ((COrder*)pParam)->FiducialMark2.OffsetY))
                    {
                        //沒有找到
                        _cwprintf(L"手動模式第一點找到第二點未找到\r\n");
                        //找到清除手動狀態、將對位點依設置已經有Offset
                        ((COrder*)pParam)->VisionSerchError.Manuallymode = FALSE;
                        ((COrder*)pParam)->VisionTrigger.AdjustStatus = 3;
                        ((COrder*)pParam)->VisionFindMarkError(pParam);
                    }
                    else
                    {
                        //找到都抬升
#ifdef MOVE
                        ((COrder*)pParam)->m_Action.DecideLineEndMove(0, 0,
                            ((COrder*)pParam)->ZSet.ZBackHeight, ((COrder*)pParam)->ZSet.ZBackType, 0, 0, 0, 0,
                            ((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, 6000,1);
#endif
                        //找到清除手動狀態、將對位點依設置已經有Offset
                        ((COrder*)pParam)->VisionSerchError.Manuallymode = FALSE;
                        ((COrder*)pParam)->FiducialMark2.FindMarkStatus = TRUE;
                        _cwprintf(L"手動模式第一點找到第二點找到\r\n");
                    }
#endif
                }

                /*都找到後處理*/
                if (((COrder*)pParam)->FiducialMark1.FindMarkStatus && ((COrder*)pParam)->FiducialMark2.FindMarkStatus) //兩個都找到
                {
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
                    _cwprintf(L"第一點找到第二點找到Offset計算完成");

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
        }
    }
#endif
    if (CommandResolve(Command, 0) == L"FindFiducialAngle")
    {
        if (((COrder*)pParam)->ModelControl.Mode == 1)
        {
            ((COrder*)pParam)->VisionSet.Startangle = _ttol(CommandResolve(Command, 1));
            ((COrder*)pParam)->VisionSet.Endangle = _ttol(CommandResolve(Command, 2));
        }
    }
    if (CommandResolve(Command, 0) == L"CameraTrigger")
    {
        if (((COrder*)pParam)->ModelControl.Mode == 1)
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
#ifdef LA
    if (CommandResolve(Command, 0) == L"LaserHeight") { 
        if (((COrder*)pParam)->ModelControl.Mode == 1 || ((COrder*)pParam)->ModelControl.Mode == 2 || ((COrder*)pParam)->ModelControl.Mode == 3)//在任何模式下動作
        {
            CString CommandBuff;
            CommandBuff.Format(_T("LaserHeight,%d,%d,%d"), _ttol(CommandResolve(Command, 2)), _ttol(CommandResolve(Command, 3)), ((COrder*)pParam)->m_Action.g_HeightLaserZero);
            ModifyPointOffSet(pParam, CommandBuff);//CallSubroutin相對位修正
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
            }
        }
    }
    if (CommandResolve(Command, 0) == L"LaserPointAdjust")
    {
        if (((COrder*)pParam)->ModelControl.Mode == 1 || ((COrder*)pParam)->ModelControl.Mode == 3)//在影像、運動模式下動作
        {
            if (((COrder*)pParam)->ModelControl.Mode == 1)
            {
                RecordCorrectionTable(pParam);//寫入修正表
            }    
            ((COrder*)pParam)->LaserSwitch.LaserPointAdjust = _ttol(CommandResolve(Command, 1));
            //模式轉換
            if (((COrder*)pParam)->LaserSwitch.LaserAdjust && ((COrder*)pParam)->LaserSwitch.LaserPointAdjust)//兩個模式都成立取消一個
            {
                ((COrder*)pParam)->LaserSwitch.LaserAdjust = FALSE;
            }
            if (((COrder*)pParam)->LaserSwitch.LaserPointAdjust && ((COrder*)pParam)->LaserSwitch.LaserHeight)
            {
                ChooseVisionModify(pParam);//選擇影像Offset
                ((COrder*)pParam)->FinalWorkCoordinateData.X = ((COrder*)pParam)->LaserData.LaserHeightPoint.X;
                ((COrder*)pParam)->FinalWorkCoordinateData.Y = ((COrder*)pParam)->LaserData.LaserHeightPoint.Y;
                ((COrder*)pParam)->FinalWorkCoordinateData.Z = ((COrder*)pParam)->m_Action.g_HeightLaserZero;
                
                ((COrder*)pParam)->NVMVirtualCoordinateData = ((COrder*)pParam)->FinalWorkCoordinateData;//紀錄CallSubroutine點(不加影像修正時的值)
                
                VisionModify(pParam);//影像修正
                ((COrder*)pParam)->VirtualCoordinateData = ((COrder*)pParam)->FinalWorkCoordinateData;//紀錄移動虛擬座標
            }
            else if (((COrder*)pParam)->LaserSwitch.LaserPointAdjust && ((COrder*)pParam)->LaserSwitch.LaserDetect)
            {
                /*線段測高(依結束點)*/
                ChooseVisionModify(pParam);//選擇影像Offset
                ((COrder*)pParam)->FinalWorkCoordinateData.X = ((COrder*)pParam)->LaserData.LaserDetectLE.X;
                ((COrder*)pParam)->FinalWorkCoordinateData.Y = ((COrder*)pParam)->LaserData.LaserDetectLE.Y;
                ((COrder*)pParam)->FinalWorkCoordinateData.Z = ((COrder*)pParam)->m_Action.g_HeightLaserZero;

                ((COrder*)pParam)->NVMVirtualCoordinateData = ((COrder*)pParam)->FinalWorkCoordinateData;//紀錄CallSubroutine點(不加影像修正時的值)

                VisionModify(pParam);//影像修正
                ((COrder*)pParam)->VirtualCoordinateData = ((COrder*)pParam)->FinalWorkCoordinateData;//紀錄移動虛擬座標
            }
        }
        if (((COrder*)pParam)->ModelControl.Mode == 2)//在雷射模式下動作
        {
            ((COrder*)pParam)->LaserSwitch.LaserPointAdjust = _ttol(CommandResolve(Command, 1));
            //模式轉換
            if (((COrder*)pParam)->LaserSwitch.LaserAdjust && ((COrder*)pParam)->LaserSwitch.LaserPointAdjust)//兩個模式都成立取消一個
            {
                ((COrder*)pParam)->LaserSwitch.LaserAdjust = FALSE;
            }
            //開啟雷射模式
            if (((COrder*)pParam)->LaserSwitch.LaserPointAdjust && ((COrder*)pParam)->LaserSwitch.LaserHeight)
            {
                /*單點測高*/
                ChooseVisionModify(pParam);//選擇影像Offset
                ((COrder*)pParam)->FinalWorkCoordinateData.X = ((COrder*)pParam)->LaserData.LaserHeightPoint.X;
                ((COrder*)pParam)->FinalWorkCoordinateData.Y = ((COrder*)pParam)->LaserData.LaserHeightPoint.Y;

                ((COrder*)pParam)->NVMVirtualCoordinateData = { 0,((COrder*)pParam)->LaserData.LaserHeightPoint.X ,((COrder*)pParam)->LaserData.LaserHeightPoint.Y,((COrder*)pParam)->m_Action.g_HeightLaserZero };//紀錄CallSubroutine點(不加影像修正時的值)

                VisionModify(pParam);//影像修正
                ((COrder*)pParam)->m_Action.LA_Dot3D(
                    ((COrder*)pParam)->FinalWorkCoordinateData.X,
                    ((COrder*)pParam)->FinalWorkCoordinateData.Y,
                    ((COrder*)pParam)->LaserData.LaserMeasureHeight,
                    ((COrder*)pParam)->DotSpeedSet.EndSpeed, ((COrder*)pParam)->DotSpeedSet.AccSpeed, 6000);

                ((COrder*)pParam)->VirtualCoordinateData = { 0,((COrder*)pParam)->FinalWorkCoordinateData.X,((COrder*)pParam)->FinalWorkCoordinateData.Y,((COrder*)pParam)->m_Action.g_HeightLaserZero };//紀錄移動虛擬座標
                //紀錄測量高度至雷射修正表
                ((COrder*)pParam)->LaserCount++;
                ((COrder*)pParam)->LaserAdjust.push_back({ ((COrder*)pParam)->LaserData.LaserMeasureHeight });
            }
            else if (((COrder*)pParam)->LaserSwitch.LaserPointAdjust && ((COrder*)pParam)->LaserSwitch.LaserDetect)
            {
                /*線段測高*/
                CPoint MeasureLS, MeasureLE;
                ChooseVisionModify(pParam);//選擇影像Offset
                ((COrder*)pParam)->FinalWorkCoordinateData.X = ((COrder*)pParam)->LaserData.LaserDetectLS.X;
                ((COrder*)pParam)->FinalWorkCoordinateData.Y = ((COrder*)pParam)->LaserData.LaserDetectLS.Y;
                VisionModify(pParam);//影像修正
                MeasureLS.x = ((COrder*)pParam)->FinalWorkCoordinateData.X;
                MeasureLS.y = ((COrder*)pParam)->FinalWorkCoordinateData.Y;
                ((COrder*)pParam)->FinalWorkCoordinateData.X = ((COrder*)pParam)->LaserData.LaserDetectLE.X;
                ((COrder*)pParam)->FinalWorkCoordinateData.Y = ((COrder*)pParam)->LaserData.LaserDetectLE.Y;
                VisionModify(pParam);//影像修正
                MeasureLE.x = ((COrder*)pParam)->FinalWorkCoordinateData.X;
                MeasureLE.y = ((COrder*)pParam)->FinalWorkCoordinateData.Y;
                //測量線段平均高度
                ((COrder*)pParam)->m_Action.LA_AverageZ(MeasureLS.x, MeasureLS.y, 
                    MeasureLE.x, MeasureLE.y,
                    ((COrder*)pParam)->LaserData.LaserMeasureHeight);

                ((COrder*)pParam)->VirtualCoordinateData = { 0,MeasureLE.x,MeasureLE.y,((COrder*)pParam)->m_Action.g_HeightLaserZero };//紀錄移動虛擬座標
                //紀錄測量高度至雷射修正表
                ((COrder*)pParam)->LaserCount++;
                ((COrder*)pParam)->LaserAdjust.push_back({ ((COrder*)pParam)->LaserData.LaserMeasureHeight });
            }
        }
    }
    if (CommandResolve(Command, 0) == L"LaserDetect")
    {
        if (((COrder*)pParam)->ModelControl.Mode == 1 || ((COrder*)pParam)->ModelControl.Mode == 2 || ((COrder*)pParam)->ModelControl.Mode == 3)//在任何模式下動作
        {
            CString CommandBuff;
            CommandBuff.Format(_T("LaserHeight,%d,%d,%d"), _ttol(CommandResolve(Command, 2)), _ttol(CommandResolve(Command, 3)), ((COrder*)pParam)->m_Action.g_HeightLaserZero);
            ModifyPointOffSet(pParam, CommandBuff);//CallSubroutin相對位修正
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
            }
        }
    }
    if (CommandResolve(Command, 0) == L"LaserAdjust")
    {
        if (((COrder*)pParam)->ModelControl.Mode == 2)//在雷射模式下動作
        {
            ((COrder*)pParam)->LaserSwitch.LaserAdjust = _ttol(CommandResolve(Command, 1));
            if (((COrder*)pParam)->LaserSwitch.LaserAdjust && ((COrder*)pParam)->LaserSwitch.LaserPointAdjust)//兩個模式都成立取消一個
            {
                ((COrder*)pParam)->LaserSwitch.LaserPointAdjust = FALSE;
            }
        }
    } 
    if (CommandResolve(Command, 0) == L"LaserSkip")
    {
        if (((COrder*)pParam)->ModelControl.Mode == 2)//在雷射模式下動作
        {
            ((COrder*)pParam)->LaserSwitch.LaserSkip = _ttol(CommandResolve(Command, 1));
        }
    }
#endif
    g_pSubroutineThread = NULL;
    return 0;
}
/**************************************************************************動作影像判斷修正區塊********************************************************************/
/*運動狀態判斷*/
void COrder::LineGotoActionJudge(LPVOID pParam)
{
#ifdef MOVE
    if (((COrder*)pParam)->ModelControl.Mode == 3 || ((COrder*)pParam)->ModelControl.Mode == 4)
    {
        if (((COrder*)pParam)->RunData.ActionStatus.at(((COrder*)pParam)->Program.SubroutinCount) == 1)//LS時 移動至LS虛擬點
        {
            ((COrder*)pParam)->m_Action.DecideVirtualPoint(((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Y, ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Z,
                ((COrder*)pParam)->DotSpeedSet.EndSpeed, ((COrder*)pParam)->DotSpeedSet.AccSpeed, 6000);

            ((COrder*)pParam)->VirtualCoordinateData = ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount);//紀錄移動虛擬座標
        }
        else if (((COrder*)pParam)->RunData.ActionStatus.at(((COrder*)pParam)->Program.SubroutinCount) == 2)//LP時 斷膠抬升
        {
            ((COrder*)pParam)->m_Action.DecideLineEndMove(((COrder*)pParam)->DispenseLineSet.StayTime, ((COrder*)pParam)->DispenseLineSet.ShutdownDelay,
                ((COrder*)pParam)->ZSet.ZBackHeight, ((COrder*)pParam)->ZSet.ZBackType, ((COrder*)pParam)->DispenseLineEnd.HighSpeed, ((COrder*)pParam)->DispenseLineEnd.Width, ((COrder*)pParam)->DispenseLineEnd.Height, ((COrder*)pParam)->DispenseLineEnd.LowSpeed,
                ((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, 6000);
        }
    } 
    if (((COrder*)pParam)->ModelControl.Mode == 2)
    {
        //插入狀態下
        if (((COrder*)pParam)->RunData.ActionStatus.at(((COrder*)pParam)->Program.SubroutinCount) == 2)
        {
            //呼叫執行掃描
            ((COrder*)pParam)->m_Action.LA_Line2D(10000, 10000, 6000);
            //紀錄測量高度至雷射修正表
            ((COrder*)pParam)->ContinuousLineCount--;
            ((COrder*)pParam)->LaserCount++;
            ((COrder*)pParam)->LaserAdjust.push_back({ ((COrder*)pParam)->ContinuousLineCount });
            /*修改紀錄修正表*/
            for (UINT i = 0; i < ((COrder*)pParam)->PositionModifyNumber.size(); i++)//判斷表中地址是否存在
            {
                if (((COrder*)pParam)->PositionModifyNumber.at(i).Address == ((COrder*)pParam)->CurrentTableAddress)
                {      
                    ((COrder*)pParam)->PositionModifyNumber.at(i).LaserNumber = ((COrder*)pParam)->LaserCount;
                    break;
                }
            }
#ifdef  PRINTF
            _cwprintf(_T("執行完特殊線段雷射掃描(LP接點)"));
#endif
        }
    }
#endif
    //狀態清除
    ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Status = FALSE;
    ((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Status = FALSE;
    ((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Status = FALSE;
    ((COrder*)pParam)->RunData.ActionStatus.at(((COrder*)pParam)->Program.SubroutinCount) = 0;
}
/*Subroutine計算修正點偏移量*/
void COrder::ModifyPointOffSet(LPVOID pParam ,CString Command)
{
#ifdef MOVE
    if (!((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Status)
    {
        CString XYZOffset;
        /*if (((COrder*)pParam)->ModelControl.Mode == 1 || ((COrder*)pParam)->ModelControl.Mode == 2)
        {
            XYZOffset = ((COrder*)pParam)->VirtualNowOffSet(pParam, Command);
        }
        else if (((COrder*)pParam)->ModelControl.Mode == 3)
        {
            XYZOffset = ((COrder*)pParam)->m_Action.NowOffSet(_ttol(CommandResolve(Command, 1)), _ttol(CommandResolve(Command, 2)), _ttol(CommandResolve(Command, 3)));
        }*/
        XYZOffset = ((COrder*)pParam)->VirtualNowOffSet(pParam, Command);

        ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X = _ttol(CommandResolve(XYZOffset, 0));
        ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y = _ttol(CommandResolve(XYZOffset, 1));
        ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z = _ttol(CommandResolve(XYZOffset, 2));
        //將Switch 轉換成已修正
        ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Status = TRUE;
        //修正完畢斷膠抬升(或直接抬升)
        if (((COrder*)pParam)->ModelControl.Mode != 1 && ((COrder*)pParam)->ModelControl.Mode != 2)//在影像、雷射模式下不做回升
        {
            ((COrder*)pParam)->m_Action.DecideLineEndMove(((COrder*)pParam)->DispenseLineSet.StayTime, ((COrder*)pParam)->DispenseLineSet.ShutdownDelay,
                ((COrder*)pParam)->ZSet.ZBackHeight, ((COrder*)pParam)->ZSet.ZBackType,
                ((COrder*)pParam)->DispenseLineEnd.HighSpeed, ((COrder*)pParam)->DispenseLineEnd.Width, ((COrder*)pParam)->DispenseLineEnd.Height, ((COrder*)pParam)->DispenseLineEnd.LowSpeed,
                ((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, 6000,1);
        }
    } 
#endif
}
/*影像修正*/
void COrder::VisionModify(LPVOID pParam)
{      
    /*之後測試是否有問題，修正完畢後始否清除影像值*/
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
            _cwprintf(_T("CallSubroutine進入修正!\n"));
#endif 
        }
    }
#endif
}
/*影像沒找到處理方法*/
void COrder::VisionFindMarkError(LPVOID pParam)
{
    switch (((COrder*)pParam)->VisionSerchError.SearchError)
    {
    case 1://略過或Trigger
#ifdef VI
        //無找到抬升
        #ifdef MOVE
        //對位完畢不出膠回升
        ((COrder*)pParam)->m_Action.DecideLineEndMove(0, 0,
            ((COrder*)pParam)->ZSet.ZBackHeight, ((COrder*)pParam)->ZSet.ZBackType, 0, 0, 0, 0,
            ((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, 6000,1);
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
                        ((COrder*)pParam)->DotSpeedSet.EndSpeed, ((COrder*)pParam)->DotSpeedSet.AccSpeed, 6000);
                    #endif
                    if (((COrder*)pParam)->VisionTrigger.AdjustStatus == 1)
                    {
                        if (VI_CameraTrigger(((COrder*)pParam)->FindMark.MilModel, ((COrder*)pParam)->FindMark.Point.X, ((COrder*)pParam)->FindMark.Point.Y, ((COrder*)pParam)->VisionTrigger.Trigger1.at(i).X, ((COrder*)pParam)->VisionTrigger.Trigger1.at(i).Y, ((COrder*)pParam)->VisionOffset.OffsetX, ((COrder*)pParam)->VisionOffset.OffsetY))
                        {                      
                        #ifdef MOVE
                            //對位完畢不出膠回升
                            ((COrder*)pParam)->m_Action.DecideLineEndMove(0, 0,
                                ((COrder*)pParam)->ZSet.ZBackHeight, ((COrder*)pParam)->ZSet.ZBackType, 0, 0, 0, 0,
                                ((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, 6000,1);
                        #endif
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
                                ((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, 6000,1);
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
                        ((COrder*)pParam)->DotSpeedSet.EndSpeed, ((COrder*)pParam)->DotSpeedSet.AccSpeed, 6000);
#endif              
                    if (VI_CameraTrigger(((COrder*)pParam)->FiducialMark2.MilModel, ((COrder*)pParam)->FiducialMark2.Point.X, ((COrder*)pParam)->FiducialMark2.Point.Y, ((COrder*)pParam)->VisionTrigger.Trigger2.at(i).X, ((COrder*)pParam)->VisionTrigger.Trigger2.at(i).Y, ((COrder*)pParam)->FiducialMark2.OffsetX, ((COrder*)pParam)->FiducialMark2.OffsetY))
                    {
                    #ifdef MOVE
                        //對位完畢不出膠回升
                        ((COrder*)pParam)->m_Action.DecideLineEndMove(0, 0,
                            ((COrder*)pParam)->ZSet.ZBackHeight, ((COrder*)pParam)->ZSet.ZBackType, 0, 0, 0, 0,
                            ((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, 6000,1);
                    #endif
                        break;
                    }
                }
            }
            ((COrder*)pParam)->FiducialMark2.FindMarkStatus = TRUE;
        }
        ((COrder*)pParam)->VisionSerchError.SearchError = ((COrder*)pParam)->VisionDefault.VisionSerchError.SearchError;//參數回歸
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
            ((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, 6000,1);
        #endif
        ((COrder*)pParam)->Pause(); 
        if (((COrder*)pParam)->VisionTrigger.AdjustStatus == 2 || ((COrder*)pParam)->VisionTrigger.AdjustStatus == 3)
        {
            ((COrder*)pParam)->VisionSerchError.Pausemode = TRUE;
            ((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount))--;//重複一次命令
        }
        break;
    case 4://跳出選擇對話窗
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
#ifdef LA
    if (((COrder*)pParam)->LaserData.LaserMeasureHeight != -999999999)//如果值不等於-999999999執行修正
    {
#ifdef PRINTF
        _cwprintf(L"進入雷射修正:FinalWorkCoordinateData.Z = %d\n", ((COrder*)pParam)->LaserData.LaserMeasureHeight);
#endif 
        ((COrder*)pParam)->FinalWorkCoordinateData.Z = ((COrder*)pParam)->LaserData.LaserMeasureHeight;
        if (((COrder*)pParam)->FinalWorkCoordinateData.Z <= -10000)
        {
            ((COrder*)pParam)->ContinuousSwitch = TRUE;
#ifdef PRINTF
            _cwprintf(L"選擇線段:%d\n", abs(((COrder*)pParam)->FinalWorkCoordinateData.Z + 10000));
#endif  
        }
        ((COrder*)pParam)->LaserData.LaserMeasureHeight = -999999999;//修正完畢後清除值
    }
#endif 
}
/*雷射檢測處理*/
void COrder::LaserDetectHandle(LPVOID pParam, CString Command)
{
#ifdef LA 
    if (!((COrder*)pParam)->LaserSwitch.LaserSkip)//雷射跳過未開啟
    {
        if (((COrder*)pParam)->LaserSwitch.LaserHeight && ((COrder*)pParam)->LaserSwitch.LaserAdjust)//模式二
        {
            ChooseVisionModify(pParam);//選擇影像Offset      
            ModifyPointOffSet(pParam, Command);//CallSubutine用            
            ((COrder*)pParam)->FinalWorkCoordinateData.X = _ttoi(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X;
            ((COrder*)pParam)->FinalWorkCoordinateData.Y = _ttoi(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y;
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
            //雷射運動掃描
            ((COrder*)pParam)->m_Action.LA_Dot3D(
                ((COrder*)pParam)->FinalWorkCoordinateData.X,
                ((COrder*)pParam)->FinalWorkCoordinateData.Y,
                ((COrder*)pParam)->LaserData.LaserMeasureHeight,
                ((COrder*)pParam)->DotSpeedSet.EndSpeed, ((COrder*)pParam)->DotSpeedSet.AccSpeed, 6000);
            //紀錄測量高度至雷射修正表
            ((COrder*)pParam)->LaserCount++;
            ((COrder*)pParam)->LaserAdjust.push_back({ ((COrder*)pParam)->LaserData.LaserMeasureHeight });
        }
        else if (((COrder*)pParam)->LaserSwitch.LaserDetect && ((COrder*)pParam)->LaserSwitch.LaserAdjust)//模式三
        {
            ChooseVisionModify(pParam);//選擇影像Offset      
            ModifyPointOffSet(pParam, Command);//CallSubutine用      
            ((COrder*)pParam)->FinalWorkCoordinateData.X = _ttoi(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X;
            ((COrder*)pParam)->FinalWorkCoordinateData.Y = _ttoi(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y;
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
                //雷射運動掃描
                ((COrder*)pParam)->m_Action.LA_Dot3D(
                    ((COrder*)pParam)->FinalWorkCoordinateData.X,
                    ((COrder*)pParam)->FinalWorkCoordinateData.Y,
                    ((COrder*)pParam)->LaserData.LaserMeasureHeight,
                    ((COrder*)pParam)->DotSpeedSet.EndSpeed, ((COrder*)pParam)->DotSpeedSet.AccSpeed, 6000);
                //紀錄測量高度至雷射修正表
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
                            //呼叫LS
                            ((COrder*)pParam)->m_Action.LA_Do2dDataLine(((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Y);
                            //呼叫ARC-LP測高(API 點順序倒著放)
                            ((COrder*)pParam)->m_Action.LA_Do2dDataArc(((COrder*)pParam)->FinalWorkCoordinateData.X, ((COrder*)pParam)->FinalWorkCoordinateData.Y,
                                ((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Y);
                            //清除完成動作弧
                            ((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Status = FALSE;
                        }
                        else if (((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Status)//執行圓
                        {
                            //呼叫LS
                            ((COrder*)pParam)->m_Action.LA_Do2dDataLine(((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Y);
                            //呼叫Circle-LP(API 點順序倒著放)
                            ((COrder*)pParam)->m_Action.LA_Do2dDataCircle(((COrder*)pParam)->FinalWorkCoordinateData.X, ((COrder*)pParam)->FinalWorkCoordinateData.Y,
                                ((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Y,
                                ((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount).Y);
                            //清除完成動作圓
                            ((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Status = FALSE;
                        }
                        else//執行線段
                        {
                            //呼叫LS
                            ((COrder*)pParam)->m_Action.LA_Do2dDataLine(((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Y);
                            //呼叫LP                  
                            ((COrder*)pParam)->m_Action.LA_Do2dDataLine(((COrder*)pParam)->FinalWorkCoordinateData.X, ((COrder*)pParam)->FinalWorkCoordinateData.Y);
                        }
                    }
                    else if (((COrder*)pParam)->RunData.ActionStatus.at(((COrder*)pParam)->Program.SubroutinCount) == 2)//LS存在執行過LP
                    {
                        if (((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Status)//執行圓弧
                        {
                            //呼叫Now-ARC-LP (API 點順序倒著放)
                            ((COrder*)pParam)->m_Action.LA_Do2dDataArc(((COrder*)pParam)->FinalWorkCoordinateData.X, ((COrder*)pParam)->FinalWorkCoordinateData.Y,
                                ((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Y);
                            //清除完成動作弧
                            ((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Status = FALSE;
                        }
                        else if (((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Status)//執行圓
                        {
                            //呼叫Now-Circle-LP
                            ((COrder*)pParam)->m_Action.LA_Do2dDataCircle(((COrder*)pParam)->FinalWorkCoordinateData.X, ((COrder*)pParam)->FinalWorkCoordinateData.Y,
                                ((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Y,
                                ((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount).Y);
                            //清除完成動作圓
                            ((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Status = FALSE;
                        }
                        else//執行線段
                        {
                            //呼叫LP
                            ((COrder*)pParam)->m_Action.LA_Do2dDataLine(((COrder*)pParam)->FinalWorkCoordinateData.X, ((COrder*)pParam)->FinalWorkCoordinateData.Y);
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
                            //呼叫LS
                            ((COrder*)pParam)->m_Action.LA_Do2dDataLine(((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Y);
                            //呼叫ARC-LP測高(API 點順序倒著放)
                            ((COrder*)pParam)->m_Action.LA_Do2dDataArc(((COrder*)pParam)->FinalWorkCoordinateData.X, ((COrder*)pParam)->FinalWorkCoordinateData.Y,
                                ((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Y);   
                            //清除完成動作弧
                            ((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Status = FALSE;
                        }
                        else if (((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Status)//執行圓
                        {
                            //呼叫LS
                            ((COrder*)pParam)->m_Action.LA_Do2dDataLine(((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Y);
                            //呼叫Circle-LE
                            ((COrder*)pParam)->m_Action.LA_Do2dDataCircle(((COrder*)pParam)->FinalWorkCoordinateData.X, ((COrder*)pParam)->FinalWorkCoordinateData.Y,
                                ((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Y,
                                ((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount).Y);
                            //清除完成動作圓
                            ((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Status = FALSE;
                        }
                        else//執行線段
                        {
                            //呼叫LS
                            ((COrder*)pParam)->m_Action.LA_Do2dDataLine(((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Y);
                            //呼叫LE                       
                            ((COrder*)pParam)->m_Action.LA_Do2dDataLine(((COrder*)pParam)->FinalWorkCoordinateData.X, ((COrder*)pParam)->FinalWorkCoordinateData.Y);
                        }
                    }
                    else if (((COrder*)pParam)->RunData.ActionStatus.at(((COrder*)pParam)->Program.SubroutinCount) == 2)////LS存在執行過LP
                    {
                        if (((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Status)//執行圓弧
                        {
                            //呼叫Now-ARC-LE (API 點順序倒著放)
                            ((COrder*)pParam)->m_Action.LA_Do2dDataArc(((COrder*)pParam)->FinalWorkCoordinateData.X, ((COrder*)pParam)->FinalWorkCoordinateData.Y,
                                ((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Y);
                            //清除完成動作弧
                            ((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Status = FALSE;                
                        }
                        else if (((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Status)//執行圓
                        {
                            //呼叫Now-Circle-LE
                            ((COrder*)pParam)->m_Action.LA_Do2dDataCircle(((COrder*)pParam)->FinalWorkCoordinateData.X, ((COrder*)pParam)->FinalWorkCoordinateData.Y,
                                ((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Y,
                                ((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount).Y);
                            //清除完成動作圓
                            ((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Status = FALSE;
                        }
                        else//執行線段
                        {
                            //呼叫LE
                            ((COrder*)pParam)->m_Action.LA_Do2dDataLine(((COrder*)pParam)->FinalWorkCoordinateData.X, ((COrder*)pParam)->FinalWorkCoordinateData.Y);
                        }
                    }
#ifdef PRINTF
         /*           for (UINT i= 0; i < ((COrder*)pParam)->m_Action.LA_m_ptVec2D.size(); i++)
                    {
                        _cwprintf(L"%d,%d\n", ((COrder*)pParam)->m_Action.LA_m_ptVec2D.at(i).EndP.x, ((COrder*)pParam)->m_Action.LA_m_ptVec2D.at(i).EndP.y);
                    }*/
#endif
                    //呼叫執行掃描
                    ((COrder*)pParam)->m_Action.LA_Line2D(10000, 10000, 6000);
                    //紀錄雷射表
                    ((COrder*)pParam)->ContinuousLineCount--;
                    ((COrder*)pParam)->LaserCount++;
                    ((COrder*)pParam)->LaserAdjust.push_back({ ((COrder*)pParam)->ContinuousLineCount });                
                }
                ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Status = FALSE;
                ((COrder*)pParam)->RunData.ActionStatus.at(((COrder*)pParam)->Program.SubroutinCount) = 0;      
            }
        }           

        RecordCorrectionTable(pParam);//寫入修正表
#ifdef PRINTF
        _cwprintf(L"雷射高度成功寫入雷射表\n");
#endif  
    }
#endif
}
/*虛擬位置偏移*/
CString COrder::VirtualNowOffSet(LPVOID pParam, CString Command)
{
    LONG lNowX = 0, lNowY = 0, lNowZ = 0;
    CString csBuff = 0;   
    /*統一使未修正的XYZ值做CallSubroutine計算相對量位移*/
    lNowX = ((COrder*)pParam)->NVMVirtualCoordinateData.X;
    lNowY = ((COrder*)pParam)->NVMVirtualCoordinateData.Y;
    lNowZ = ((COrder*)pParam)->NVMVirtualCoordinateData.Z;
    csBuff.Format(_T("%ld,%ld,%ld"), (lNowX - _ttol(CommandResolve(Command, 1))), (lNowY - _ttol(CommandResolve(Command, 2))), (lNowZ - _ttol(CommandResolve(Command, 3))));
    return csBuff;
}
/*虛擬座標移動*/
void COrder::VirtualCoordinateMove(LPVOID pParam, CString Command ,LONG type)
{
    if (CommandResolve(Command, 0) == L"GoHome")
    {
        ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Status = FALSE;
        if (((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Status)//已經有offset修正
        {
            ((COrder*)pParam)->VirtualCoordinateData = { 0,0,0,0 };//紀錄移動虛擬座標
        }
        else
        {
            ((COrder*)pParam)->VirtualCoordinateData = { 0,((COrder*)pParam)->Program.SubroutinePointStack.at(((COrder*)pParam)->Program.SubroutinCount).X,((COrder*)pParam)->Program.SubroutinePointStack.at(((COrder*)pParam)->Program.SubroutinCount).Y,((COrder*)pParam)->Program.SubroutinePointStack.at(((COrder*)pParam)->Program.SubroutinCount).Z };//紀錄移動虛擬座標
        }
    }
    else if (CommandResolve(Command, 0) == L"ParkPosition")
    {
        ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Status = FALSE;
        if (((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Status)//已經有offset修正
        {
            if (((COrder*)pParam)->GlueData.GlueAuto)
            {
                ((COrder*)pParam)->VirtualCoordinateData = { 0,((COrder*)pParam)->GlueData.ParkPositionData.X, ((COrder*)pParam)->GlueData.ParkPositionData.Y, ((COrder*)pParam)->GlueData.ParkPositionData.Z };//紀錄移動虛擬座標
            }
        }
        else
        {
            ((COrder*)pParam)->VirtualCoordinateData = { 0,((COrder*)pParam)->Program.SubroutinePointStack.at(((COrder*)pParam)->Program.SubroutinCount).X,((COrder*)pParam)->Program.SubroutinePointStack.at(((COrder*)pParam)->Program.SubroutinCount).Y,((COrder*)pParam)->Program.SubroutinePointStack.at(((COrder*)pParam)->Program.SubroutinCount).Z };//紀錄移動虛擬座標
        }
    }
    else if (CommandResolve(Command, 0) == L"FillArea")
    {
        CString CommandBuff;
        CommandBuff.Format(_T("FillArea,%d,%d,%d"), _ttol(CommandResolve(Command, 4)), _ttol(CommandResolve(Command, 5)), _ttol(CommandResolve(Command, 6)));
        ((COrder*)pParam)->V_ActionCount++;//虛擬動作計數++
        ChooseVisionModify(pParam);//選擇影像Offset
        ModifyPointOffSet(pParam, CommandBuff);//CallSubroutin相對位修正
        ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Status = FALSE;

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

        //紀錄移動虛擬座標
        LONG FillAreaPoint1X, FillAreaPoint1Y, FillAreaPoint1Z, FillAreaPoint2X, FillAreaPoint2Y, FillAreaPoint2Z;
        ((COrder*)pParam)->FinalWorkCoordinateData.X = _ttol(CommandResolve(Command, 4)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X;
        ((COrder*)pParam)->FinalWorkCoordinateData.Y = _ttol(CommandResolve(Command, 5)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y;
        ((COrder*)pParam)->FinalWorkCoordinateData.Z = _ttol(CommandResolve(Command, 6)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z;

        VisionModify(pParam);//影像修正
        FillAreaPoint1X = ((COrder*)pParam)->FinalWorkCoordinateData.X;
        FillAreaPoint1Y = ((COrder*)pParam)->FinalWorkCoordinateData.Y;
        FillAreaPoint1Z = ((COrder*)pParam)->FinalWorkCoordinateData.Z;

        ((COrder*)pParam)->FinalWorkCoordinateData.X = _ttol(CommandResolve(Command, 7)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X;
        ((COrder*)pParam)->FinalWorkCoordinateData.Y = _ttol(CommandResolve(Command, 8)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y;
        ((COrder*)pParam)->FinalWorkCoordinateData.Z = _ttol(CommandResolve(Command, 9)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z;

        VisionModify(pParam);//影像修正
        FillAreaPoint2X = ((COrder*)pParam)->FinalWorkCoordinateData.X;
        FillAreaPoint2Y = ((COrder*)pParam)->FinalWorkCoordinateData.Y;
        FillAreaPoint2Z = ((COrder*)pParam)->FinalWorkCoordinateData.Z;

                                                                                                                                                                           //紀錄移動虛擬座標
        ((COrder*)pParam)->m_Action.Fill_EndPoint(((COrder*)pParam)->VirtualCoordinateData.X, ((COrder*)pParam)->VirtualCoordinateData.Y,
            FillAreaPoint1X, FillAreaPoint1Y, FillAreaPoint1Z,
            FillAreaPoint2X, FillAreaPoint2Y, FillAreaPoint2Z,
            _ttol(CommandResolve(Command, 1)), _ttol(CommandResolve(Command, 2)), _ttol(CommandResolve(Command, 3))
        );
        ((COrder*)pParam)->VirtualCoordinateData.Z = FillAreaPoint2Z;
    }
    else
    {
        ((COrder*)pParam)->V_ActionCount++;//虛擬動作計數++
        ChooseVisionModify(pParam);//選擇影像Offset
        ModifyPointOffSet(pParam, Command);//CallSubroutin相對位修正
        if (type == 1)//類型1單點運動
        {
            ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Status = FALSE;
            if (CommandResolve(Command, 0) == L"LineStart")
            {
                ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Status = TRUE;
            }
            ((COrder*)pParam)->FinalWorkCoordinateData.X = _ttoi(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X;
            ((COrder*)pParam)->FinalWorkCoordinateData.Y = _ttoi(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y;
            ((COrder*)pParam)->FinalWorkCoordinateData.Z = _ttoi(CommandResolve(Command, 3)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z;
        }
        else if (type == 2)//類型二線段運動
        {
            if (((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Status)//判斷是否有LS
            {
                ((COrder*)pParam)->FinalWorkCoordinateData.X = _ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X;
                ((COrder*)pParam)->FinalWorkCoordinateData.Y = _ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y;
                ((COrder*)pParam)->FinalWorkCoordinateData.Z = _ttol(CommandResolve(Command, 3)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z;
                if (CommandResolve(Command, 0) == L"LineEnd")
                {
                    ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Status = FALSE;
                }
            }
        }

        ((COrder*)pParam)->NVMVirtualCoordinateData = ((COrder*)pParam)->FinalWorkCoordinateData;//紀錄CallSubroutine點(不加影像修正時的值)

        //影像修正選擇
        if (CommandResolve(Command, 0) == L"VirtualPoint" || CommandResolve(Command, 0) == L"StopPoint")
        {
            if (_ttol(CommandResolve(Command, 4)) == 1)
            {
                VisionModify(pParam);//影像修正
            }
            else
            {
                ((COrder*)pParam)->VisioModifyJudge = FALSE;//影像開關改變為不修正(用在CallSubroutine)
            }
        }
        else if (CommandResolve(Command, 0) == L"WaitPoint")
        {
            if (_ttol(CommandResolve(Command, 5)) == 1)
            {
                VisionModify(pParam);//影像修正
            }
            else
            {
                ((COrder*)pParam)->VisioModifyJudge = FALSE;//影像開關改變為不修正(用在CallSubroutine)
            }
        }
        else
        {
            VisionModify(pParam);//影像修正
        }
        ((COrder*)pParam)->VirtualCoordinateData = ((COrder*)pParam)->FinalWorkCoordinateData;//紀錄移動虛擬座標
    } 
}
/**************************************************************************資料表處理區塊*************************************************************************/
/*選擇影像修正*/
void COrder::ChooseVisionModify(LPVOID pParam) {
    CString StrBuff,Temp;
    StrBuff.Format(_T("%d"), ((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)));
    if (((COrder*)pParam)->RepeatData.StepRepeatNum.size())//有StepRepeat時地址紀錄的方式
    {
        for (UINT i = 0; i < ((COrder*)pParam)->RepeatData.StepRepeatNum.size(); i++)
        {
            Temp.Format(_T(",%d-%d"), ((COrder*)pParam)->RepeatData.StepRepeatCountX.at(i), ((COrder*)pParam)->RepeatData.StepRepeatCountY.at(i));
            StrBuff = StrBuff + Temp;
        }
    }
    if (!((COrder*)pParam)->Program.SubroutineStack.empty())//有CallSubroutine時地址紀錄的方式
    {
        for (UINT i = 0; i < ((COrder*)pParam)->Program.SubroutineStack.size(); i++)
        {
            Temp.Format(_T(",%d"), ((COrder*)pParam)->Program.SubroutineStack.at(i));
            StrBuff = StrBuff + Temp;
        }
    }
    for (UINT i = 0; i < ((COrder*)pParam)->PositionModifyNumber.size(); i++)
    {
        if (((COrder*)pParam)->PositionModifyNumber.at(i).Address == StrBuff)
        {
            if (((COrder*)pParam)->PositionModifyNumber.at(i).VisionNumber == -1)
            {
                ((COrder*)pParam)->PositionModifyNumber.at(i).VisionNumber = 0;
            }
            ((COrder*)pParam)->VisionOffset = ((COrder*)pParam)->VisionAdjust.at(((COrder*)pParam)->PositionModifyNumber.at(i).VisionNumber).VisionOffset;
        }
    }
}
/*選擇雷射修正*/
void COrder::ChooseLaserModify(LPVOID pParam){
    CString StrBuff, Temp;
    StrBuff.Format(_T("%d"), ((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)));
    if (((COrder*)pParam)->RepeatData.StepRepeatNum.size())//有StepRepeat時地址紀錄的方式
    {
        for (UINT i = 0; i < ((COrder*)pParam)->RepeatData.StepRepeatNum.size(); i++)
        {
            Temp.Format(_T(",%d-%d"), ((COrder*)pParam)->RepeatData.StepRepeatCountX.at(i), ((COrder*)pParam)->RepeatData.StepRepeatCountY.at(i));
            StrBuff = StrBuff + Temp;
        }
    }
    if (!((COrder*)pParam)->Program.SubroutineStack.empty())//有CallSubroutine時地址紀錄的方式
    {
        for (UINT i = 0; i < ((COrder*)pParam)->Program.SubroutineStack.size(); i++)
        {
            Temp.Format(_T(",%d"), ((COrder*)pParam)->Program.SubroutineStack.at(i));
            StrBuff = StrBuff + Temp;
        }
    }
    for (UINT i = 0; i < ((COrder*)pParam)->PositionModifyNumber.size(); i++)
    {
        if (((COrder*)pParam)->PositionModifyNumber.at(i).Address == StrBuff)
        {
            if (((COrder*)pParam)->PositionModifyNumber.at(i).LaserNumber == -1)
            {
                ((COrder*)pParam)->PositionModifyNumber.at(i).LaserNumber = 0;
            }
            ((COrder*)pParam)->LaserData.LaserMeasureHeight = ((COrder*)pParam)->LaserAdjust.at(((COrder*)pParam)->PositionModifyNumber.at(i).LaserNumber).LaserMeasureHeight;
        }
    }
}
/*紀錄修正表*/
void COrder::RecordCorrectionTable(LPVOID pParam) {
    CString StrBuff,Temp;
    StrBuff.Format(_T("%d"), ((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)));
    if (((COrder*)pParam)->RepeatData.StepRepeatNum.size())//有StepRepeat時地址紀錄的方式
    {
        for (UINT i = 0; i < ((COrder*)pParam)->RepeatData.StepRepeatNum.size(); i++)
        {
            Temp.Format(_T(",%d-%d"), ((COrder*)pParam)->RepeatData.StepRepeatCountX.at(i), ((COrder*)pParam)->RepeatData.StepRepeatCountY.at(i));
            StrBuff = StrBuff + Temp;
        }
    }
    if (!((COrder*)pParam)->Program.SubroutineStack.empty())//有CallSubroutine時地址紀錄的方式
    {
        for (UINT i = 0; i < ((COrder*)pParam)->Program.SubroutineStack.size(); i++)
        {
            Temp.Format(_T(",%d"), ((COrder*)pParam)->Program.SubroutineStack.at(i));
            StrBuff = StrBuff + Temp;
        }
    }
#ifdef PRINTF
    _cwprintf(L"Address:%s\n", StrBuff);
#endif
    if (((COrder*)pParam)->PositionModifyNumber.size())//修正表有值
    { 
#ifdef PRINTF
        _cwprintf(L"表中有值增加PositionModifNumber:%d\n", ((COrder*)pParam)->PositionModifyNumber.size());
#endif
        for (UINT i = 0; i < ((COrder*)pParam)->PositionModifyNumber.size(); i++)//判斷表中地址是否存在
        {
            if (((COrder*)pParam)->PositionModifyNumber.at(i).Address == StrBuff)
            {
                if (((COrder*)pParam)->ModelControl.Mode == 1)//目前是影像模式
                {
                    if (((COrder*)pParam)->PositionModifyNumber.at(i).VisionNumber == -1)//且此地址影像表沒有值
                    {
                        ((COrder*)pParam)->PositionModifyNumber.at(i).VisionNumber = ((COrder*)pParam)->VisionCount;
                    }            
                }
                else if (((COrder*)pParam)->ModelControl.Mode == 2)
                {
                    if (((COrder*)pParam)->PositionModifyNumber.at(i).LaserNumber == -1)//且此地址雷射表沒有值
                    {
                        ((COrder*)pParam)->PositionModifyNumber.at(i).LaserNumber = ((COrder*)pParam)->LaserCount;
                    }
                }
                ((COrder*)pParam)->CurrentTableAddress = StrBuff;
                break;
            } 
            else if (i == ((COrder*)pParam)->PositionModifyNumber.size() - 1)//地址不存在表中
            {
                if (((COrder*)pParam)->ModelControl.Mode == 1)//影像模式時
                {
                    ((COrder*)pParam)->PositionModifyNumber.push_back({ StrBuff, ((COrder*)pParam)->VisionCount, -1 });
                }
                else  if (((COrder*)pParam)->ModelControl.Mode == 2)//雷射模式時
                {
#ifdef PRINTF
                    _cwprintf(L"新增(雷射模式時):%d\n", ((COrder*)pParam)->LaserCount);
#endif
                    ((COrder*)pParam)->PositionModifyNumber.push_back({ StrBuff, ((COrder*)pParam)->VisionCount, ((COrder*)pParam)->LaserCount });
                }
                ((COrder*)pParam)->CurrentTableAddress = StrBuff;
            }
        }   
    }
    else
    {
        if (((COrder*)pParam)->ModelControl.Mode == 1)//影像模式時
        {
            ((COrder*)pParam)->PositionModifyNumber.push_back({ StrBuff, ((COrder*)pParam)->VisionCount, -1 });
#ifdef PRINTF
            _cwprintf(L"進入影像模式增加PositionModifNumber");
#endif
        }
        else  if (((COrder*)pParam)->ModelControl.Mode == 2)//跳過影像模式直接雷射模式
        {
            ((COrder*)pParam)->PositionModifyNumber.push_back({ StrBuff, 0, ((COrder*)pParam)->LaserCount });
#ifdef PRINTF
            _cwprintf(L"跳過影像進入雷射模式增加PositionModifNumber");
#endif
        }
    }
}
/**************************************************************************程序變數處理區塊************************************************************************/
/*命令分解*/
CString COrder::CommandResolve(CString Command,UINT Choose)
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
/*參數設為Default*/
void COrder::ParameterDefult() {
    //運動
    DispenseDotSet = Default.DispenseDotSet;
    DispenseDotEnd = Default.DispenseDotEnd;
    DotSpeedSet = Default.DotSpeedSet;
    DispenseLineSet = Default.DispenseLineSet;
    DispenseLineEnd = Default.DispenseLineEnd;
    LineSpeedSet = Default.LineSpeedSet;
    ZSet = Default.ZSet;
    GlueData = Default.GlueData;
    GoHome = Default.GoHome;
    //影像
    VisionSet = VisionDefault.VisionSet;
    VisionFile = VisionDefault.VisionFile;
    VisionSerchError = VisionDefault.VisionSerchError;
}
/*判斷指標初始化*/
void COrder::DecideInit()
{
    //原點賦歸狀態設為賦歸完成
    RunStatusRead.GoHomeStatus = TRUE;
    //運動運行狀態清除
    m_Action.g_bIsStop = FALSE;
    m_Action.g_bIsPause = FALSE;
    //程序狀態堆疊
    InitData.Status = FALSE;
    InitData.X = 0;
    InitData.Y = 0;
    InitData.Z = 0;
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
    //副程式和主程式控制結構
    RunData.MSChange.push_back(0); //副程式改變控制程序的堆疊
    for (UINT i = 0; i < RunData.MSChange.size(); i++)
    {
        RunData.MSChange.at(i) = 0;
    }
    RunStatusRead.RunStatus = 0;//狀態改變成未運行
    RunData.StackingCount = 0;//主副程式計數  
    RunData.SubProgramName = _T("");//副程式判斷標籤
    RunData.ActionStatus.push_back(0);//運動狀態清0
    RunStatusRead.CurrentRunCommandNum = 0;//目前運行命令計數清0
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
    //影像Model 指針初始化 /**********注意記憶體分配問題*
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
        _cwprintf(L"我有進入歸零PositionModifyNumber&&LaserAdjust\n");
#endif
        PositionModifyNumber.clear();
        m_Action.LA_Clear();//清除連續線段陣列
        LaserAdjust.clear();
        LaserAdjust.push_back({ -999999999 });
    }  
    VisionAdjust.push_back({ { { 0,0,0,0 },0,0,0 } });
    
    //判斷是否有影像修正(給CallSubroutine使用)
    VisioModifyJudge = FALSE;
    //虛擬座標初始化(修正過)
    VirtualCoordinateData = { 0,0,0,0 };
    //虛擬座標初始化(未修正)
    NVMVirtualCoordinateData = { 0,0,0,0 };
    //計數連續線段初始化
    ContinuousLineCount = -10000;
    //控制連續線段執行開關初始化(讓呼叫連續線段使用)
    ContinuousSwitch = FALSE;
    //雷射測高數值初始化
    LaserData.LaserMeasureHeight = -999999999;
 
    /****************************************************************/
    ActionCount = 0;
    V_ActionCount = 0;
#ifdef PRINTF
    _cwprintf(L"Init()\n");
#endif
}
/*判斷指標清除*/
void COrder::DecideClear() 
{ 
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
    RepeatData.StepRepeatBlockData.clear();
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
    free(FiducialMark1.MilModel);
    free(FiducialMark2.MilModel);
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
        _cwprintf(L"我有進入清除PositionModifyNumber&&LaserAdjust\n");
#endif
        PositionModifyNumber.clear();
        LaserAdjust.clear();      
    }
    VisionAdjust.clear();
    
#ifdef PRINTF
        _cwprintf(L"Clear()\n");
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
void COrder::DecideBeginModel()
{
    //TODO::模組方式目前尚未加入副程式
    //模式清除
    ModelControl.Mode = 0;
    ModelControl.ModeChangeAddress = 0; //目前尚未用到
    ModelControl.VisionModeJump = TRUE;
    ModelControl.LaserModeJump = TRUE;
    //模式初始化判斷
    for (UINT i = 1; i < Command.at(0).size(); i++) //不考慮副程式
    {
        if (CommandResolve(Command.at(0).at(i), 0) == L"FindMarkAdjust" ||
            CommandResolve(Command.at(0).at(i), 0) == L"FiducialMarkAdjust")
        { 
            ModelControl.VisionModeJump = FALSE;          
        }
        if (CommandResolve(Command.at(0).at(i), 0) == L"LaserPointAdjust" ||
            CommandResolve(Command.at(0).at(i), 0) == L"LaserAdjust")
        {
            ModelControl.LaserModeJump = FALSE;
        }
    }
    if (!ModelControl.VisionModeJump)
    {
        ModelControl.Mode = 1;
    }
    else if (!ModelControl.LaserModeJump)
    {
        //TODO::DEMO所以加入
        if (!DemoTemprarilySwitch)//如果DemoTemprarilySwitch為TRUE清除跳過模式二
        {
            ModelControl.Mode = 2;
        }
        else
        {
            ModelControl.Mode = 3;       
        }   
    }
    else
    {
        ModelControl.Mode = 3;
    }  
}
/**************************************************************************影像檔案處理區塊************************************************************************/
/*搜尋檔案名*/
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
/*判斷檔案是否存在*/
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
/**************************************************************************阻斷處理區塊****************************************************************************/
/*阻斷處理方式(加入StepRepeatX時)*/
void COrder::BlockProcessStart(CString Command, LPVOID pParam, BOOL RepeatStatus)
{
    CString BlockBuff;
    BOOL DeleteStepRepeat = FALSE;
    int StepRepeatBlockSize = ((COrder*)pParam)->RepeatData.StepRepeatBlockData.back().BlockPosition.size();
    for (int i = 0; i < StepRepeatBlockSize; i++)//搜尋第一項阻斷陣列 是否有1-1
    {
#ifdef PRINTF
        _cwprintf(_T("%d-%d.StepRepeatX 檢查阻斷\n"), ((COrder*)pParam)->RepeatData.StepRepeatCountX.back(), ((COrder*)pParam)->RepeatData.StepRepeatCountY.back());
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
                _cwprintf(L"標籤只有Y變X正常:%s\n", BlockBuff);
#endif
            }
            else
            {
                BlockBuff.Format(_T("%d-%d"), (_ttol(CommandResolve(Command, 3)) + 1 - ((COrder*)pParam)->RepeatData.StepRepeatCountX.back()),
                    (_ttol(CommandResolve(Command, 4)) + 1 - ((COrder*)pParam)->RepeatData.StepRepeatCountY.back()));
#ifdef PRINTF
                _cwprintf(L"標籤XY變:%s\n", BlockBuff);
#endif
            }
            if (((COrder*)pParam)->RepeatData.StepRepeatBlockData.back().BlockPosition.at(i) == BlockBuff)
            {
#ifdef PRINTF
                _cwprintf(L"有阻斷\n");
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
                    _cwprintf(L"最後一個:SSwitch轉換:%d\n", ((COrder*)pParam)->RepeatData.SSwitch.back());
#endif
                    ((COrder*)pParam)->RepeatData.StepRepeatCountX.back() = _ttol(CommandResolve(Command, 3));
                    ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y = ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y + _ttol(CommandResolve(Command, 2));
                    ((COrder*)pParam)->RepeatData.StepRepeatCountY.back()--;
                }
                else
                {
#ifdef PRINTF
                    _cwprintf(L"刪除陣列\n");
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
                _cwprintf(L"有阻斷\n");
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
                    _cwprintf(L"刪除陣列\n");
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
            _cwprintf(L"進入新增內層迴圈\n");
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
/*阻斷處理方式(執行中StepRepeatX時)*/
BOOL COrder::BlockProcessExecute(CString Command, LPVOID pParam, int NowCount)
{
    CString BlockBuff;
    int BlockSize = ((COrder*)pParam)->RepeatData.StepRepeatBlockData.at(NowCount).BlockPosition.size();
    for (int i = 0; i < BlockSize; i++)//搜尋最後一項阻斷陣列 是否有1-1
    {
#ifdef PRINTF
        _cwprintf(L"第%d:%d-%d.StepRepeatX 檢查阻斷\n", NowCount,((COrder*)pParam)->RepeatData.StepRepeatCountX.at(NowCount), ((COrder*)pParam)->RepeatData.StepRepeatCountY.at(NowCount));
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
                _cwprintf(L"第%d:標籤只有Y變X正常:%s\n", NowCount, BlockBuff);
#endif
            }
            else
            { 
                BlockBuff.Format(_T("%d-%d"), (_ttol(CommandResolve(Command, 3)) + 1 - ((COrder*)pParam)->RepeatData.StepRepeatCountX.at(NowCount)),
                    (_ttol(CommandResolve(Command, 4)) + 1 - ((COrder*)pParam)->RepeatData.StepRepeatCountY.at(NowCount)));
#ifdef PRINTF
                _cwprintf(L"第%d:標籤XY變:%s\n", NowCount, BlockBuff);
#endif
            }
            if (((COrder*)pParam)->RepeatData.StepRepeatBlockData.at(NowCount).BlockPosition.at(i) == BlockBuff)
            {
#ifdef PRINTF
                _cwprintf(L"第%d:有阻斷\n", NowCount);
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
                    _cwprintf(L"第%d:執行%d-%d\n", NowCount, ((COrder*)pParam)->RepeatData.StepRepeatCountX.at(NowCount), ((COrder*)pParam)->RepeatData.StepRepeatCountY.at(NowCount));
#endif
                }
                else if (((COrder*)pParam)->RepeatData.StepRepeatCountY.at(NowCount) > 1)
                {
                    ((COrder*)pParam)->RepeatData.SSwitch.at(NowCount) = !((COrder*)pParam)->RepeatData.SSwitch.at(NowCount);
#ifdef PRINTF
                    _cwprintf(L"第%d:SSwitch轉換:%d\n", i, ((COrder*)pParam)->RepeatData.SSwitch.at(NowCount));
#endif
                    ((COrder*)pParam)->RepeatData.StepRepeatCountX.at(NowCount) = _ttol(CommandResolve(Command, 3));
                    ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y = ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y + _ttol(CommandResolve(Command, 2));
                    ((COrder*)pParam)->RepeatData.StepRepeatCountY.at(NowCount)--;
                    ((COrder*)pParam)->RepeatData.StepRepeatLabel = _T("StepRepeatLabel,") + CommandResolve(Command, 6);
#ifdef PRINTF
                    _cwprintf(L"第%d:執行%d-%d\n", NowCount, ((COrder*)pParam)->RepeatData.StepRepeatCountX.at(NowCount), ((COrder*)pParam)->RepeatData.StepRepeatCountY.at(NowCount));
#endif
                }
                else
                {
#ifdef PRINTF
                    _cwprintf(L"第%d:刪除所有陣列\n", NowCount);
#endif
                    ((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum++;
#ifdef PRINTF
                    _cwprintf(L"刪除總數+1=%d\n", ((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum);
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
                    ((COrder*)pParam)->RepeatData.StepRepeatLabel = _T("");
#ifdef PRINTF
                    _cwprintf(L"第%d:刪除成功\n", NowCount);
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
            _cwprintf(L"第%d:標籤:%s\n", NowCount, BlockBuff);
#endif
            if (((COrder*)pParam)->RepeatData.StepRepeatBlockData.at(NowCount).BlockPosition.at(i) == BlockBuff)
            {
#ifdef PRINTF
                _cwprintf(L"第%d:有阻斷\n", NowCount);
#endif
                if (((COrder*)pParam)->RepeatData.StepRepeatCountX.at(NowCount) > 1)
                {
                    ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X = ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X + _ttol(CommandResolve(Command, 1));
                    ((COrder*)pParam)->RepeatData.StepRepeatCountX.at(NowCount)--;
                    ((COrder*)pParam)->RepeatData.StepRepeatLabel = _T("StepRepeatLabel,") + CommandResolve(Command, 6);
#ifdef PRINTF
                    _cwprintf(L"第%d:執行%d-%d\n", NowCount, ((COrder*)pParam)->RepeatData.StepRepeatCountX.at(NowCount), ((COrder*)pParam)->RepeatData.StepRepeatCountY.at(NowCount));
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
                    _cwprintf(L"第%d:執行%d-%d\n", NowCount, ((COrder*)pParam)->RepeatData.StepRepeatCountX.at(NowCount), ((COrder*)pParam)->RepeatData.StepRepeatCountY.at(NowCount));
#endif
                }
                else
                {
#ifdef PRINTF
                    _cwprintf(L"第%d:刪除所有陣列\n", NowCount);
#endif
                    ((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum++;
#ifdef PRINTF
                    _cwprintf(L"刪除總數+1=%d\n", ((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum);
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
                    ((COrder*)pParam)->RepeatData.StepRepeatLabel = _T("");
#ifdef PRINTF
                    _cwprintf(L"第%d:刪除成功\n", NowCount);
#endif
                    return FALSE;
                }
            }
        }
    }
#ifdef PRINTF
    _cwprintf(L"結束阻斷\n"); 
#endif
    return TRUE;
}
/*阻斷處理方式(加入StepRepeatY時)*/
void COrder::BlockProcessStartY(CString Command, LPVOID pParam, BOOL RepeatStatus)
{
    CString BlockBuff;
    BOOL DeleteStepRepeat = FALSE;
    int StepRepeatBlockSize = ((COrder*)pParam)->RepeatData.StepRepeatBlockData.back().BlockPosition.size();
    for (int i = 0; i < StepRepeatBlockSize; i++)//搜尋第一項阻斷陣列 是否有1-1
    {
#ifdef PRINTF
        _cwprintf(_T("%d-%d.StepRepeatY 檢查阻斷\n"), ((COrder*)pParam)->RepeatData.StepRepeatCountX.back(), ((COrder*)pParam)->RepeatData.StepRepeatCountY.back());
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
                _cwprintf(L"標籤只有X變Y正常:%s\n", BlockBuff);
#endif
            }
            else
            {
                BlockBuff.Format(_T("%d-%d"), (_ttol(CommandResolve(Command, 3)) + 1 - ((COrder*)pParam)->RepeatData.StepRepeatCountX.back()),
                    (_ttol(CommandResolve(Command, 4)) + 1 - ((COrder*)pParam)->RepeatData.StepRepeatCountY.back()));
#ifdef PRINTF
                _cwprintf(L"標籤XY變:%s\n", BlockBuff);
#endif
            }
            if (((COrder*)pParam)->RepeatData.StepRepeatBlockData.back().BlockPosition.at(i) == BlockBuff)
            {
#ifdef PRINTF
                _cwprintf(L"有阻斷\n");
#endif
                if (((COrder*)pParam)->RepeatData.StepRepeatCountY.back() > 1)/**/
                {
                    if (((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y == ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.back() + (_ttol(CommandResolve(Command, 2))*(_ttol(CommandResolve(Command, 4)) - 1)))/**/
                    {
                        ((COrder*)pParam)->RepeatData.SSwitch.back() = FALSE;
#ifdef PRINTF
                        _cwprintf(L"最後一個:SSwitch轉換:%d\n",((COrder*)pParam)->RepeatData.SSwitch.back());
#endif
                    }
                    else  if (((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y == ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.back())
                    {
                        ((COrder*)pParam)->RepeatData.SSwitch.back() = TRUE;
#ifdef PRINTF
                        _cwprintf(L"最後一個:SSwitch轉換:%d\n",((COrder*)pParam)->RepeatData.SSwitch.back());
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
                    _cwprintf(L"最後一個:SSwitch轉換:%d\n", ((COrder*)pParam)->RepeatData.SSwitch.back());
#endif
                    /**/((COrder*)pParam)->RepeatData.StepRepeatCountY.back() = _ttol(CommandResolve(Command, 4));
                    /**/((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X = ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X + _ttol(CommandResolve(Command, 1));
                    /**/((COrder*)pParam)->RepeatData.StepRepeatCountX.back()--;
                }
                else
                {
#ifdef PRINTF
                    _cwprintf(L"刪除陣列\n");
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
                _cwprintf(L"有阻斷\n");
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
                    _cwprintf(L"刪除陣列\n");
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
            _cwprintf(L"進入新增內層迴圈\n");
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
/*阻斷處理方式(執行中StepRepeatY時)*/
BOOL COrder::BlockProcessExecuteY(CString Command, LPVOID pParam, int NowCount)
{
    CString BlockBuff;
    int BlockSize = ((COrder*)pParam)->RepeatData.StepRepeatBlockData.at(NowCount).BlockPosition.size();
    for (int i = 0; i < BlockSize; i++)//搜尋最後一項阻斷陣列 是否有1-1
    {
#ifdef PRINTF
        _cwprintf(L"第%d:%d-%d.StepRepeatY 檢查阻斷\n", NowCount, ((COrder*)pParam)->RepeatData.StepRepeatCountX.at(NowCount), ((COrder*)pParam)->RepeatData.StepRepeatCountY.at(NowCount));
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
                _cwprintf(L"第%d:標籤只有Y變X正常:%s\n", NowCount, BlockBuff);
#endif
            }
            else
            {
                BlockBuff.Format(_T("%d-%d"), (_ttol(CommandResolve(Command, 3)) + 1 - ((COrder*)pParam)->RepeatData.StepRepeatCountX.at(NowCount)),
                    (_ttol(CommandResolve(Command, 4)) + 1 - ((COrder*)pParam)->RepeatData.StepRepeatCountY.at(NowCount)));
#ifdef PRINTF
                _cwprintf(L"第%d:標籤XY變:%s\n", NowCount, BlockBuff);
#endif
            }
            if (((COrder*)pParam)->RepeatData.StepRepeatBlockData.at(NowCount).BlockPosition.at(i) == BlockBuff)
            {
#ifdef PRINTF
                _cwprintf(L"第%d:有阻斷\n", NowCount);
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
                    _cwprintf(L"第%d:執行%d-%d\n", NowCount, ((COrder*)pParam)->RepeatData.StepRepeatCountX.at(NowCount), ((COrder*)pParam)->RepeatData.StepRepeatCountY.at(NowCount));
#endif
                }
                else if (((COrder*)pParam)->RepeatData.StepRepeatCountX.at(NowCount) > 1)
                {
                    ((COrder*)pParam)->RepeatData.SSwitch.at(NowCount) = !((COrder*)pParam)->RepeatData.SSwitch.at(NowCount);
#ifdef PRINTF
                    _cwprintf(L"第%d:SSwitch轉換:%d\n", i, ((COrder*)pParam)->RepeatData.SSwitch.at(NowCount));
#endif
                    /**/((COrder*)pParam)->RepeatData.StepRepeatCountY.at(NowCount) = _ttol(CommandResolve(Command, 4));
                    /**/((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X = ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X + _ttol(CommandResolve(Command, 1));
                    /**/((COrder*)pParam)->RepeatData.StepRepeatCountX.at(NowCount)--;
                    ((COrder*)pParam)->RepeatData.StepRepeatLabel = _T("StepRepeatLabel,") + CommandResolve(Command, 6);
#ifdef PRINTF
                    _cwprintf(L"第%d:執行%d-%d\n", NowCount, ((COrder*)pParam)->RepeatData.StepRepeatCountX.at(NowCount), ((COrder*)pParam)->RepeatData.StepRepeatCountY.at(NowCount));
#endif
                }
                else
                {
#ifdef PRINTF
                    _cwprintf(L"第%d:刪除所有陣列\n", NowCount);
#endif
                    ((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum++;
#ifdef PRINTF
                    _cwprintf(L"刪除總數+1=%d\n", ((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum);
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
                    ((COrder*)pParam)->RepeatData.StepRepeatLabel = _T("");
#ifdef PRINTF
                    _cwprintf(L"第%d:刪除成功\n", NowCount);
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
            _cwprintf(L"第%d:標籤:%s\n", NowCount, BlockBuff);
#endif
            if (((COrder*)pParam)->RepeatData.StepRepeatBlockData.at(NowCount).BlockPosition.at(i) == BlockBuff)
            {
#ifdef PRINTF
                _cwprintf(L"第%d:有阻斷\n", NowCount);
#endif
                if (((COrder*)pParam)->RepeatData.StepRepeatCountY.at(NowCount) > 1)/**/
                {
                    /**/((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y = ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y + _ttol(CommandResolve(Command, 2));
                    /**/((COrder*)pParam)->RepeatData.StepRepeatCountY.at(NowCount)--;
                    ((COrder*)pParam)->RepeatData.StepRepeatLabel = _T("StepRepeatLabel,") + CommandResolve(Command, 6);
#ifdef PRINTF
                    _cwprintf(L"第%d:執行%d-%d\n", NowCount, ((COrder*)pParam)->RepeatData.StepRepeatCountX.at(NowCount), ((COrder*)pParam)->RepeatData.StepRepeatCountY.at(NowCount));
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
                    _cwprintf(L"第%d:執行%d-%d\n", NowCount, ((COrder*)pParam)->RepeatData.StepRepeatCountX.at(NowCount), ((COrder*)pParam)->RepeatData.StepRepeatCountY.at(NowCount));
#endif
                }
                else
                {
#ifdef PRINTF
                    _cwprintf(L"第%d:刪除所有陣列\n", NowCount);
#endif
                    ((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum++;
#ifdef PRINTF
                    _cwprintf(L"刪除總數+1=%d\n", ((COrder*)pParam)->RepeatData.AllDeleteStepRepeatNum);
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
                    ((COrder*)pParam)->RepeatData.StepRepeatLabel = _T("");
#ifdef PRINTF
                    _cwprintf(L"第%d:刪除成功\n", NowCount);
#endif
                    return FALSE;
                }
            }
        }
    }
#ifdef PRINTF
    _cwprintf(L"結束阻斷\n");
#endif
    return TRUE;
} 
/*阻斷陣列排序*/
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
#ifdef PRINTF
        _cwprintf(L"\n");
#endif
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
    _cwprintf(L"排序完成後:");
    for (UINT i = 0; i < BlockPosition.size(); i++)
    {
        _cwprintf(L"%s,", BlockPosition.at(i));
    }
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
                ar << ((COrder*)pParam)->PositionModifyNumber.at(i).Address;
                StrBuff.Format(L"%d", ((COrder*)pParam)->PositionModifyNumber.at(i).LaserNumber);
                ar << StrBuff;
                StrBuff.Format(L"%d", ((COrder*)pParam)->PositionModifyNumber.at(i).VisionNumber);
                ar << StrBuff;
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
                        PositionModifyNumber.push_back({ StrBuff,0,0 });
                    }
                    else if (Count == 2)
                    {
                        PositionModifyNumber.back().LaserNumber = _ttoi(StrBuff);
                    }
                    else if (Count == 3)
                    {
                        PositionModifyNumber.back().VisionNumber = -1;
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
        }
    }
}
/*命令mm轉um*/
CString COrder::CommandmmToum(CString Command) 
{
     if(CommandResolve(Command,0) == L"Dot" || 
         CommandResolve(Command, 0) == L"LineStart" || 
         CommandResolve(Command, 0) == L"LinePassing" || 
         CommandResolve(Command, 0) == L"LineEnd" || 
         CommandResolve(Command, 0) == L"ArcPoint" || 
         CommandResolve(Command, 0) == L"CyclePoint" || 
         CommandResolve(Command, 0) == L"" || )
}
