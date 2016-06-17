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
    VisionSet = { 0,0,0,0,0,0,0,0,0 };
    
    RunStatusRead = { 0,0,1,0 };
    VisionDefault.VisionSerchError.Manuallymode = FALSE;
    VisionDefault.VisionSerchError.Pausemode = FALSE;
}
COrder::~COrder()
{
}
BEGIN_MESSAGE_MAP(COrder, CWnd)
END_MESSAGE_MAP()
// COrder 訊息處理常式
/**************************************************************************執行續動作區塊*************************************************************************/
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
            //參數設定為預設
            ParameterDefult();
            //劃分主副程序
            MainSubProgramSeparate();
            //狀態初始化
            DecideInit();
            //載入所有檔案名
            ListAllFileInDirectory(VisionFile.ModelPath,TEXT("*年*月*日*時*分*秒*.mod"));
            //出膠控制器模式
            m_Action.g_bIsDispend = TRUE;
            //針頭模式 不減offset
            VisionSet.ModifyMode = FALSE;
            wakeEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
            g_pThread = AfxBeginThread(Thread, (LPVOID)this);
        }  
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
BOOL COrder::Home()
{
    if (!g_pThread)
    {
        //將停止狀態清除
        m_Action.g_bIsStop = FALSE;
        //確定GoHome參數賦值
        GoHome = Default.GoHome;
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
            //參數設定為預設
            ParameterDefult();
            //劃分主副程序
            MainSubProgramSeparate();
            //狀態初始化
            DecideInit();
            //載入所有檔案名
            ListAllFileInDirectory(VisionFile.ModelPath, TEXT("*年*月*日*時*分*秒*.mod"));
            //出膠控制器模式
            m_Action.g_bIsDispend = FALSE;
            //針頭模式 不減offset 或 CCD模式 減offset
            VisionSet.ModifyMode = mode;

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
/*原點賦歸執行緒*/
UINT COrder::HomeThread(LPVOID pParam)
{
#ifdef MOVE
    ((COrder*)pParam)->m_Action.DecideInitializationMachine(((COrder*)pParam)->GoHome.Speed1, ((COrder*)pParam)->GoHome.Speed2, ((COrder*)pParam)->GoHome.Axis, ((COrder*)pParam)->GoHome.MoveX, ((COrder*)pParam)->GoHome.MoveY, ((COrder*)pParam)->GoHome.MoveZ);
#endif
    ((COrder*)pParam)->RunStatusRead.GoHomeStatus = TRUE;
    g_pThread = NULL;
    return 0;
}
/*主執行緒*/
UINT COrder::Thread(LPVOID pParam)
{
    ((COrder*)pParam)->RunStatusRead.RunStatus = 1;//狀態改變成運作中
    while ((!((COrder*)pParam)->m_Action.g_bIsStop) && ((COrder*)pParam)->Commanding != _T("End")) {
        if (((COrder*)pParam)->RunData.SubProgramName != _T(""))
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
        if (((COrder*)pParam)->Program.LabelName != _T(""))
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
                ////執行StepRepeat，沒有此標籤時
                //if (((COrder*)pParam)->RepeatData.StepRepeatSwitch && ((COrder*)pParam)->Program.LabelCount == ((COrder*)pParam)->Command.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)).size())
                //{
                //    if (!((COrder*)pParam)->RepeatData.StepRepeatNum.empty())
                //    {
                //        ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X = ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.at(((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.size() - 1);
                //        ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y = ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.at(((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.size() - 1);
                //        ((COrder*)pParam)->RepeatData.StepRepeatNum.pop_back();
                //        ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.pop_back();
                //        ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.pop_back();
                //        ((COrder*)pParam)->RepeatData.StepRepeatCountX.pop_back();
                //        ((COrder*)pParam)->RepeatData.StepRepeatCountY.pop_back();
                //    }
                //}
                //標籤清除
                ((COrder*)pParam)->Program.LabelName = _T("");
                ((COrder*)pParam)->Program.LabelCount = 0;
            }
        }
        else if (((COrder*)pParam)->RepeatData.StepRepeatLabel != _T(""))
        {
            if (((COrder*)pParam)->RepeatData.StepRepeatLabelLock)
            {
                _cwprintf(L"%s=%s", ((COrder*)pParam)->RepeatData.StepRepeatLabel, CommandResolve(((COrder*)pParam)->Command.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)).at(((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount))), 6));
                if (((COrder*)pParam)->RepeatData.StepRepeatLabel ==
                    CommandResolve(((COrder*)pParam)->Command.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)).at(((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount))), 6) && 
                    (CommandResolve(((COrder*)pParam)->Command.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)).at(((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount))), 0) == L"StepRepeatX" ||
                    CommandResolve(((COrder*)pParam)->Command.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)).at(((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount))), 0) == L"StepRepeatY") )
                {
                    ((COrder*)pParam)->RepeatData.StepRepeatLabel = _T("");
                    ((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount))--;
                }
                else if (CommandResolve(((COrder*)pParam)->Command.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)).at(((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount))), 0) == L"End")
                {
                    _cwprintf(L"沒有發現其他StepRepeat\n");
                    ((COrder*)pParam)->RepeatData.StepRepeatLabel = _T("StepRepeatLabel,") + ((COrder*)pParam)->RepeatData.StepRepeatLabel;
                    ((COrder*)pParam)->RepeatData.StepRepeatLabelLock = FALSE;
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
        else
        {
            ((COrder*)pParam)->Commanding = ((COrder*)pParam)->Command.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)).at(((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)));
            ((COrder*)pParam)->RunStatusRead.CurrentRunCommandNum = ((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount));//主程式編號
            g_pSubroutineThread = AfxBeginThread(((COrder*)pParam)->SubroutineThread, pParam);
            while (g_pSubroutineThread) {
                Sleep(1);//while 程式負載問題 無限迴圈，並讓 CPU 休息一下
            }
        }
        if (((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)) == ((COrder*)pParam)->Command.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)).size()-1)
        {
            ((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)) = 0;
        }
        else
        {
            ((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount))++;
        } 
    }
    LineGotoActionJudge(pParam);
    if (((COrder*)pParam)->Commanding == _T("End"))
    {
        ((COrder*)pParam)->RunStatusRead.FinishProgramCount++;
    }
    if (((COrder*)pParam)->GoHome.PrecycleInitialize)
    {
        ((COrder*)pParam)->m_Action.DecideInitializationMachine(((COrder*)pParam)->GoHome.Speed1, ((COrder*)pParam)->GoHome.Speed2, ((COrder*)pParam)->GoHome.Axis, ((COrder*)pParam)->GoHome.MoveX, ((COrder*)pParam)->GoHome.MoveY, ((COrder*)pParam)->GoHome.MoveZ);
    }
    ((COrder*)pParam)->DecideClear();
    ((COrder*)pParam)->RunStatusRead.RunStatus = 0;//狀態設為未運行
    g_pThread = NULL;
    return 0;
}
/*子執行緒*/
UINT COrder::SubroutineThread(LPVOID pParam) {
    HANDLE wakeEvent = (HANDLE)((COrder*)pParam)->wakeEvent;
    CString Command = ((COrder*)pParam)->Commanding; 
    if (CommandResolve(Command, 0) == L"Wait Time")
    {
        ((COrder*)pParam)->m_Action.WaitTime(wakeEvent, _ttoi(CommandResolve(Command, 1)));
        ((COrder*)pParam)->Time++;
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
        //新增紀錄目前機械手臂位置堆疊
        CoordinateData Buff;
        Buff.X = _ttol(CommandResolve(((COrder*)pParam)->m_Action.NowLocation(), 0));
        Buff.Y = _ttol(CommandResolve(((COrder*)pParam)->m_Action.NowLocation(), 1));
        Buff.Z = _ttol(CommandResolve(((COrder*)pParam)->m_Action.NowLocation(), 2));
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
            //將機器手臂移動至呼叫時位置
            ((COrder*)pParam)->m_Action.DecideVirtualPoint(
                ((COrder*)pParam)->Program.SubroutinePointStack.at(((COrder*)pParam)->Program.SubroutinCount).X,
                ((COrder*)pParam)->Program.SubroutinePointStack.at(((COrder*)pParam)->Program.SubroutinCount).Y,
                ((COrder*)pParam)->Program.SubroutinePointStack.at(((COrder*)pParam)->Program.SubroutinCount).Z,
                ((COrder*)pParam)->DotSpeedSet.EndSpeed, ((COrder*)pParam)->DotSpeedSet.AccSpeed, 6000);
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
            //將堆疊計數減1
            ((COrder*)pParam)->Program.SubroutinCount--;
        }
    }
    if (CommandResolve(Command, 0) == L"StepRepeatLabel")
    {
        CString CommandBuff;
        if (!((COrder*)pParam)->RepeatData.StepRepeatLabelLock)
        {
            _cwprintf(L"進入StepRepeatLabel\n");
            for (int i = ((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)); i < ((COrder*)pParam)->Command.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)).size(); i++)
            {
                CommandBuff = ((COrder*)pParam)->Command.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)).at(i);
                if (_ttol(CommandResolve(CommandBuff, 6)) == _ttol(CommandResolve(Command, 1)) && (CommandResolve(CommandBuff, 0) == L"StepRepeatX" || CommandResolve(CommandBuff, 0) == L"StepRepeatY"))
                {
                    _cwprintf(L"跳躍至StepRepeat\n");
                    ((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)) = i - 1;
                    ((COrder*)pParam)->RepeatData.StepRepeatLabelLock = TRUE;
                    _cprintf("%d\n", ((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)));
                    break;
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
        _cprintf("Inside StepRepeatX\n");
        //StepRepeat陣列是否大於1*1 && 模式是否 1or2 
        if ((_ttol(CommandResolve(Command, 3)) * _ttol(CommandResolve(Command, 4))) > 1 && (_ttol(CommandResolve(Command,5)) == 1 || _ttol(CommandResolve(Command, 5)) == 2))
        {
            if (!((COrder*)pParam)->RepeatData.StepRepeatNum.size())//都沒有RepeatXY時
            {
                if (((COrder*)pParam)->RepeatData.StepRepeatLabelLock)//第一次進入SetpRepeat
                {

                    /*************************************************初始Offset有問題****/
                    _cwprintf(L"沒有StepRepeat時 第一次 Inside StepRepeatX\n");
                    ((COrder*)pParam)->RepeatData.StepRepeatSwitch = TRUE;
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
                    if (_ttol(CommandResolve(Command, 7)))//有阻斷
                    {
                        _cwprintf(L"StepRepeatX 有阻斷\n");
                        for (int i = 0; i < ((COrder*)pParam)->RepeatData.StepRepeatBlockData.back().BlockPosition.size(); i++)
                        {
                            _cwprintf(L"%s,", ((COrder*)pParam)->RepeatData.StepRepeatBlockData.back().BlockPosition.at(i));
                        }
                        _cwprintf(L"\n");
                        BlockProcessStart(Command, pParam);
                    }
                    else//無阻斷不記錄繼續往下尋找StepRepeat
                    {
                        _cwprintf(L"StepRepeatX 沒有阻斷\n");
                        ((COrder*)pParam)->RepeatData.StepRepeatLabel = CommandResolve(Command, 6);  
                    }   
                }
                else 
                {
                    _cprintf("Inside2 StepRepeatX\n");
                }
            }
            else
            {
                UINT StepRepeatNumSize = ((COrder*)pParam)->RepeatData.StepRepeatNum.size();
                _cwprintf(L"目前StepRepeatNumSize:%d\n", StepRepeatNumSize);
                for (UINT i = 0; i < StepRepeatNumSize; i++)
                {
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
                                    _cwprintf(L"第%d:SSwitch轉換:%d\n", i, ((COrder*)pParam)->RepeatData.SSwitch.at(i));
                                }
                                else if (((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X == ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.at(i))
                                {
                                    ((COrder*)pParam)->RepeatData.SSwitch.at(i) = TRUE;
                                    _cwprintf(L"第%d:SSwitch轉換:%d\n", i, ((COrder*)pParam)->RepeatData.SSwitch.at(i));
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
                                BlockProcessExecute(Command, pParam, i);
                            } 
                            _cwprintf(L"第%d:執行%d-%d\n", i,((COrder*)pParam)->RepeatData.StepRepeatCountX.at(i), ((COrder*)pParam)->RepeatData.StepRepeatCountY.at(i));
                        }
                        else if(((COrder*)pParam)->RepeatData.StepRepeatCountY.at(i) > 1)
                        {
                            ((COrder*)pParam)->RepeatData.SSwitch.at(i) = !((COrder*)pParam)->RepeatData.SSwitch.at(i);
                            _cwprintf(L"第%d:SSwitch轉換:%d\n", i, ((COrder*)pParam)->RepeatData.SSwitch.at(i));
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
                                BlockProcessExecute(Command, pParam, i);
                            }
                            _cwprintf(L"第%d:執行%d-%d\n", i,((COrder*)pParam)->RepeatData.StepRepeatCountX.at(i), ((COrder*)pParam)->RepeatData.StepRepeatCountY.at(i));
                        }
                        else
                        {
                            _cwprintf(L"刪除所有陣列\n");
                            ((COrder*)pParam)->RepeatData.StepRepeatBlockData.erase(((COrder*)pParam)->RepeatData.StepRepeatBlockData.begin() + i);
                            _cwprintf(L"刪除StepRepeatBlockData陣列\n");
                            ((COrder*)pParam)->RepeatData.SSwitch.erase(((COrder*)pParam)->RepeatData.SSwitch.begin() + i);
                            _cwprintf(L"刪除SSwitch陣列\n");
                            ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X = ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.at(i);
                            ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y = ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.at(i);
                            ((COrder*)pParam)->RepeatData.StepRepeatNum.erase(((COrder*)pParam)->RepeatData.StepRepeatNum.begin() + i);
                            _cwprintf(L"刪除StepRepeatNum陣列\n");
                            ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.erase(((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.begin() + i);
                            _cwprintf(L"刪除StepRepeatInitOffsetX陣列\n");
                            ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.erase(((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.begin() + i);
                            _cwprintf(L"刪除StepRepeatInitOffsetY陣列\n");
                            ((COrder*)pParam)->RepeatData.StepRepeatCountX.erase(((COrder*)pParam)->RepeatData.StepRepeatCountX.begin() + i);
                            _cwprintf(L"刪除StepRepeatCountX陣列\n");
                            ((COrder*)pParam)->RepeatData.StepRepeatCountY.erase(((COrder*)pParam)->RepeatData.StepRepeatCountY.begin() + i);
                            _cwprintf(L"刪除StepRepeatCountY陣列\n");    
                        }    
                    }
                    else
                    {
                        if (i == StepRepeatNumSize - 1)
                        {
                            if (((COrder*)pParam)->RepeatData.StepRepeatLabelLock)//第一次進入SetpRepeat
                            {
                                _cwprintf(L"新增StepRepeat\n");
                                //((COrder*)pParam)->RepeatData.StepRepeatSwitch = TRUE;
                                //紀錄StepRepeat地址
                                ((COrder*)pParam)->RepeatData.StepRepeatNum.insert(((COrder*)pParam)->RepeatData.StepRepeatNum.begin(),((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)));
                                //紀錄初始offset位置
                                ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.push_back(((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X);
                                ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.push_back(((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y);
                                //紀錄X、Y計數
                                ((COrder*)pParam)->RepeatData.StepRepeatCountX.insert(((COrder*)pParam)->RepeatData.StepRepeatCountX.begin(), _ttol(CommandResolve(Command, 3)));
                                ((COrder*)pParam)->RepeatData.StepRepeatCountY.insert(((COrder*)pParam)->RepeatData.StepRepeatCountY.begin(), _ttol(CommandResolve(Command, 4)));
                                //S行迴圈狀態初始化
                                ((COrder*)pParam)->RepeatData.SSwitch.insert(((COrder*)pParam)->RepeatData.SSwitch.begin(), TRUE);
                                //紀錄Block
                                ((COrder*)pParam)->InitBlockData.BlockPosition.clear();
                                ((COrder*)pParam)->InitBlockData.BlockNumber = _ttol(CommandResolve(Command, 8));
                                for (int i = 0; i < ((COrder*)pParam)->InitBlockData.BlockNumber; i++)
                                {
                                    ((COrder*)pParam)->InitBlockData.BlockPosition.push_back(CommandResolve(Command, i + 9));
                                }
                                ((COrder*)pParam)->RepeatData.StepRepeatBlockData.insert(((COrder*)pParam)->RepeatData.StepRepeatBlockData.begin(), ((COrder*)pParam)->InitBlockData);
                                if (_ttol(CommandResolve(Command, 7)))//有阻斷
                                {
                                    _cwprintf(L"StepRepeatX 有阻斷\n");
                                    BlockProcessStart(Command, pParam);
                                }
                                else//無阻斷不記錄繼續往下尋找StepRepeat
                                {
                                    _cwprintf(L"StepRepeatX 沒有阻斷\n");
                                    ((COrder*)pParam)->RepeatData.StepRepeatLabel = CommandResolve(Command, 6);
                                }
                            }
                            //else//第二次進入StepRepeat
                            //{
                            //    _cwprintf(L"新增StepRepeat\n");
                            //    _cwprintf(L"有StepRepeat 第2次 Inside StepRepeatX\n");
                            //    ((COrder*)pParam)->RepeatData.StepRepeatSwitch = TRUE;
                            //    //紀錄StepRepeat地址
                            //    ((COrder*)pParam)->RepeatData.StepRepeatNum.push_back(((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)));
                            //    //紀錄初始offset位置
                            //    ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.push_back(((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X);
                            //    ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.push_back(((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y);
                            //    //紀錄X、Y計數
                            //    ((COrder*)pParam)->RepeatData.StepRepeatCountX.push_back(_ttol(CommandResolve(Command, 3)));
                            //    ((COrder*)pParam)->RepeatData.StepRepeatCountY.push_back(_ttol(CommandResolve(Command, 4)));
                            //    //S行迴圈狀態初始化
                            //    ((COrder*)pParam)->RepeatData.SSwitch.push_back(TRUE);
                            //    //紀錄Block
                            //    ((COrder*)pParam)->InitBlockData.BlockPosition.clear();
                            //    ((COrder*)pParam)->InitBlockData.BlockNumber = _ttol(CommandResolve(Command, 8));
                            //    for (int i = 0; i < ((COrder*)pParam)->InitBlockData.BlockNumber; i++)
                            //    {
                            //        ((COrder*)pParam)->InitBlockData.BlockPosition.push_back(CommandResolve(Command, i + 9));
                            //    }
                            //    ((COrder*)pParam)->RepeatData.StepRepeatBlockData.push_back(((COrder*)pParam)->InitBlockData);
                            //    if (_ttol(CommandResolve(Command, 7)))//有阻斷
                            //    {
                            //        _cwprintf(L"StepRepeatX 有阻斷\n");
                            //        for (int i = 0; i < ((COrder*)pParam)->RepeatData.StepRepeatBlockData.back().BlockPosition.size(); i++)
                            //        {
                            //            _cwprintf(L"%s,", ((COrder*)pParam)->RepeatData.StepRepeatBlockData.back().BlockPosition.at(i));
                            //        }
                            //        _cwprintf(L"\n");
                            //        BlockProcessStart(Command, pParam);
                            //    }
                            //    else//無阻斷不記錄繼續往下尋找StepRepeat
                            //    {
                            //        _cwprintf(L"StepRepeatX 沒有阻斷\n");
                            //        ((COrder*)pParam)->RepeatData.StepRepeatLabel = CommandResolve(Command, 6);
                            //    }
                            //}
                        }
                    }
                }
            }
        }
        else
        {
            
        }
    }
    if (CommandResolve(Command, 0) == L"StepRepeatY")
    {
        if ((_ttol(CommandResolve(Command, 3)) * _ttol(CommandResolve(Command, 4))) > 1 && (_ttol(CommandResolve(Command, 5)) == 1 || _ttol(CommandResolve(Command, 5)) == 2))
        {
            if (!((COrder*)pParam)->RepeatData.StepRepeatNum.size())//都沒有RepeatXY時
            {
                ((COrder*)pParam)->RepeatData.StepRepeatSwitch = TRUE;
                ((COrder*)pParam)->RepeatData.StepRepeatNum.push_back(((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)));
                //紀錄初始offset位置
                ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.push_back(((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X);
                ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.push_back(((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y);
                if (_ttol(CommandResolve(Command, 4)) > 1)/**/
                {
                    /**/((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y = ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y + _ttol(CommandResolve(Command, 2));
                    /**/((COrder*)pParam)->RepeatData.StepRepeatCountX.push_back(_ttol(CommandResolve(Command, 3)));
                    /**/((COrder*)pParam)->RepeatData.StepRepeatCountY.push_back(_ttol(CommandResolve(Command, 4)) - 1);
                }
                else
                {
                    /**/((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X = ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X + _ttol(CommandResolve(Command, 1));
                    /**/((COrder*)pParam)->RepeatData.StepRepeatCountX.push_back(_ttol(CommandResolve(Command, 3)) - 1);
                    /**/((COrder*)pParam)->RepeatData.StepRepeatCountY.push_back(_ttol(CommandResolve(Command, 4)));
                }
                ((COrder*)pParam)->RepeatData.SSwitch.push_back(TRUE);
                ((COrder*)pParam)->Program.LabelName = _T("Label,") + CommandResolve(Command, 6);
            }
            else
            {
                UINT StepRepeatNumSize = ((COrder*)pParam)->RepeatData.StepRepeatNum.size();
                for (UINT i = 0; i < StepRepeatNumSize; i++)
                {
                    if (((COrder*)pParam)->RepeatData.StepRepeatNum.at(i) == ((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)))
                    {
                        if (((COrder*)pParam)->RepeatData.StepRepeatCountY.at(i) > 1)/**/
                        {                          
                            if (_ttol(CommandResolve(Command, 5)) == 1)
                            {
                                if (((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y == ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.at(i) + (_ttol(CommandResolve(Command, 2))*(_ttol(CommandResolve(Command, 4)) - 1)))/**/
                                {
                                    ((COrder*)pParam)->RepeatData.SSwitch.at(i) = FALSE;                          
                                }          
                                else  if (((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y == ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.at(i))
                                {
                                    ((COrder*)pParam)->RepeatData.SSwitch.at(i) = TRUE;                                  
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
                            if (_ttol(CommandResolve(Command, 5)) == 2)
                            {
                                /**/((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y = ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y + _ttol(CommandResolve(Command, 2));
                            }
                            /**/((COrder*)pParam)->RepeatData.StepRepeatCountY.at(i)--;
                            ((COrder*)pParam)->Program.LabelName = _T("Label,") + CommandResolve(Command, 6);
                        }
                        else if (((COrder*)pParam)->RepeatData.StepRepeatCountX.at(i) > 1)/**/
                        {
                            if (_ttol(CommandResolve(Command, 5)) == 2)//N型X回最初位置
                            {
                                /**/((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y = ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.at(i);
                            }
                            /**/((COrder*)pParam)->RepeatData.StepRepeatCountY.at(i) = _ttol(CommandResolve(Command, 4));
                            /**/((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X = ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X + _ttol(CommandResolve(Command, 1));
                            /**/((COrder*)pParam)->RepeatData.StepRepeatCountX.at(i)--;
                            ((COrder*)pParam)->Program.LabelName = _T("Label,") + CommandResolve(Command, 6);
                        }
                        else
                        {
                            ((COrder*)pParam)->RepeatData.SSwitch.erase(((COrder*)pParam)->RepeatData.SSwitch.begin() + i);
                            ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X = ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.at(i);
                            ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y = ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.at(i);
                            ((COrder*)pParam)->RepeatData.StepRepeatNum.erase(((COrder*)pParam)->RepeatData.StepRepeatNum.begin() + i);
                            ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.erase(((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.begin() + i);
                            ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.erase(((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.begin() + i);
                            ((COrder*)pParam)->RepeatData.StepRepeatCountX.erase(((COrder*)pParam)->RepeatData.StepRepeatCountX.begin() + i);
                            ((COrder*)pParam)->RepeatData.StepRepeatCountY.erase(((COrder*)pParam)->RepeatData.StepRepeatCountY.begin() + i);
                        }
                    }
                    else
                    {
                        if (i == StepRepeatNumSize - 1)
                        {
                            ((COrder*)pParam)->RepeatData.StepRepeatSwitch = TRUE;
                            ((COrder*)pParam)->RepeatData.StepRepeatNum.push_back(((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)));
                            //紀錄初始offset位置
                            ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.push_back(((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X);
                            ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.push_back(((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y);
                            if (_ttol(CommandResolve(Command, 4)) > 1)/**/
                            {
                                /**/((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y = ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y + _ttol(CommandResolve(Command, 2));
                                /**/((COrder*)pParam)->RepeatData.StepRepeatCountX.push_back(_ttol(CommandResolve(Command, 3)));
                                /**/((COrder*)pParam)->RepeatData.StepRepeatCountY.push_back(_ttol(CommandResolve(Command, 4)) - 1);
                            }
                            else
                            {
                                /**/((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X = ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X + _ttol(CommandResolve(Command, 1));
                                /**/((COrder*)pParam)->RepeatData.StepRepeatCountX.push_back(_ttol(CommandResolve(Command, 3)) - 1);
                                /**/((COrder*)pParam)->RepeatData.StepRepeatCountY.push_back(_ttol(CommandResolve(Command, 4)));
                            }
                            ((COrder*)pParam)->RepeatData.SSwitch.push_back(TRUE);
                            ((COrder*)pParam)->Program.LabelName = _T("Label,") + CommandResolve(Command, 6);
                        }
                    }
                }
            }
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
    /************************************************************動作**************************************************************/
#ifdef MOVE
    if (CommandResolve(Command, 0) == L"Dot")
    {
        ((COrder*)pParam)->Time++;
        ModifyPointOffSet(pParam, Command);
        LineGotoActionJudge(pParam);
        ((COrder*)pParam)->FinalWorkCoordinateData.X = _ttoi(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X;
        ((COrder*)pParam)->FinalWorkCoordinateData.Y = _ttoi(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y;
        VisionModify(pParam);
        ((COrder*)pParam)->m_Action.DecidePointGlue(
            //_ttoi(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X + ((COrder*)pParam)->VisionOffset.OffsetX,
            //_ttoi(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y + ((COrder*)pParam)->VisionOffset.OffsetY,
            ((COrder*)pParam)->FinalWorkCoordinateData.X,
            ((COrder*)pParam)->FinalWorkCoordinateData.Y,
            _ttoi(CommandResolve(Command, 3)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z,
           ((COrder*)pParam)->DispenseDotSet.GlueOpenTime, ((COrder*)pParam)->DispenseDotSet.GlueCloseTime,
           ((COrder*)pParam)->ZSet.ZBackHeight, ((COrder*)pParam)->ZSet.ZBackType, ((COrder*)pParam)->DispenseDotEnd.RiseDistance, ((COrder*)pParam)->DispenseDotEnd.RiseHightSpeed, ((COrder*)pParam)->DispenseDotEnd.RiseLowSpeed,
           ((COrder*)pParam)->DotSpeedSet.EndSpeed, ((COrder*)pParam)->DotSpeedSet.AccSpeed,6000);
    }
    if (CommandResolve(Command, 0) == L"LineStart")
    {
        ((COrder*)pParam)->Time++;
        ModifyPointOffSet(pParam, Command);
        LineGotoActionJudge(pParam);
        ((COrder*)pParam)->FinalWorkCoordinateData.X = _ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X;
        ((COrder*)pParam)->FinalWorkCoordinateData.Y = _ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y;
        VisionModify(pParam);
        ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Status = TRUE;
        //((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).X = _ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X;
        //((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Y = _ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y;
        ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).X = ((COrder*)pParam)->FinalWorkCoordinateData.X;
        ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Y = ((COrder*)pParam)->FinalWorkCoordinateData.Y;
        ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Z = _ttol(CommandResolve(Command, 3)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z;
        ((COrder*)pParam)->RunData.ActionStatus.at(((COrder*)pParam)->Program.SubroutinCount) = 1;
    }
    if (CommandResolve(Command, 0) == L"LinePassing")
    {
        ((COrder*)pParam)->Time++;
        ModifyPointOffSet(pParam, Command);
        if (((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Status)
        {
            ((COrder*)pParam)->FinalWorkCoordinateData.X = _ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X;
            ((COrder*)pParam)->FinalWorkCoordinateData.Y = _ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y;
            VisionModify(pParam);
            if (((COrder*)pParam)->RunData.ActionStatus.at(((COrder*)pParam)->Program.SubroutinCount) == 1)//LS存在尚未執行過LP
            {    
                if (((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Status)
                {
                    //呼叫LS 
                    ((COrder*)pParam)->m_Action.DecideLineStartMove(((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Y, ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Z,
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
                else if (((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Status)
                {
                    //呼叫LS
                    ((COrder*)pParam)->m_Action.DecideLineStartMove(((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Y, ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Z,
                        ((COrder*)pParam)->DispenseLineSet.BeforeMoveDelay, ((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, 6000);
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
                else
                {
                    //呼叫LS-LP
                    ((COrder*)pParam)->m_Action.DecideLineSToP(((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Y, ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Z,
                        //_ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X,
                        //_ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
                        ((COrder*)pParam)->FinalWorkCoordinateData.X,
                        ((COrder*)pParam)->FinalWorkCoordinateData.Y,
                        _ttol(CommandResolve(Command, 3)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z,
                        ((COrder*)pParam)->DispenseLineSet.BeforeMoveDelay, ((COrder*)pParam)->DispenseLineSet.BeforeMoveDistance, ((COrder*)pParam)->DispenseLineSet.NodeTime,
                        ((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, 6000);
                }
            }
            else if (((COrder*)pParam)->RunData.ActionStatus.at(((COrder*)pParam)->Program.SubroutinCount) == 2)//LS存在執行過LP
            {
                if (((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Status)
                {
                    //呼叫ARC
                    ((COrder*)pParam)->m_Action.DecideArc(((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Y, 
                        ((COrder*)pParam)->FinalWorkCoordinateData.X,
                        ((COrder*)pParam)->FinalWorkCoordinateData.Y,
                        //_ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X,
                        //_ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
                        ((COrder*)pParam)->LineSpeedSet.EndSpeed, 1000);
                    //清除完成動作弧
                    ((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Status = FALSE;
                }
                else if (((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Status)
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
                else
                {
                    //呼叫LP
                    ((COrder*)pParam)->m_Action.DecideLineMidMove(
                        //_ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X,
                        //_ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
                        ((COrder*)pParam)->FinalWorkCoordinateData.X,
                        ((COrder*)pParam)->FinalWorkCoordinateData.Y,
                        _ttol(CommandResolve(Command, 3)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z,
                        ((COrder*)pParam)->DispenseLineSet.NodeTime, ((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, 6000);
                }
            }
            ((COrder*)pParam)->RunData.ActionStatus.at(((COrder*)pParam)->Program.SubroutinCount) = 2;
        }
    }
    if (CommandResolve(Command, 0) == L"LineEnd")
    {
        ((COrder*)pParam)->Time++;
        ModifyPointOffSet(pParam, Command);
        if (((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Status)
        {
            ((COrder*)pParam)->FinalWorkCoordinateData.X = _ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X;
            ((COrder*)pParam)->FinalWorkCoordinateData.Y = _ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y;
            VisionModify(pParam);
            if (((COrder*)pParam)->RunData.ActionStatus.at(((COrder*)pParam)->Program.SubroutinCount) == 1)
            {  
                if (((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Status)
                {
                    //呼叫一個LS
                    ((COrder*)pParam)->m_Action.DecideLineStartMove(((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Y, ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Z,
                        ((COrder*)pParam)->DispenseLineSet.BeforeMoveDelay, ((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, 6000);
                    //再呼叫一個ARC-LE
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
                else if(((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Status)
                {
                    //呼叫LS
                    ((COrder*)pParam)->m_Action.DecideLineStartMove(((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Y, ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Z,
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
                else
                {
                    //呼叫LS-LE
                    ((COrder*)pParam)->m_Action.DecideLineSToE(
                        ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).X, 
                        ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Y, 
                        ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Z,
                        //_ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X,
                        //_ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
                        ((COrder*)pParam)->FinalWorkCoordinateData.X,
                        ((COrder*)pParam)->FinalWorkCoordinateData.Y,
                        _ttol(CommandResolve(Command, 3)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z,
                        ((COrder*)pParam)->DispenseLineSet.BeforeMoveDelay, ((COrder*)pParam)->DispenseLineSet.BeforeMoveDistance,
                        ((COrder*)pParam)->DispenseLineSet.StayTime, ((COrder*)pParam)->DispenseLineSet.ShutdownDistance, ((COrder*)pParam)->DispenseLineSet.ShutdownDelay,
                        ((COrder*)pParam)->ZSet.ZBackHeight, ((COrder*)pParam)->ZSet.ZBackType,
                        ((COrder*)pParam)->DispenseLineEnd.HighSpeed, ((COrder*)pParam)->DispenseLineEnd.Width, ((COrder*)pParam)->DispenseLineEnd.Height, ((COrder*)pParam)->DispenseLineEnd.LowSpeed, ((COrder*)pParam)->DispenseLineEnd.Type,
                        ((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, 6000);
                }
            }
            else if (((COrder*)pParam)->RunData.ActionStatus.at(((COrder*)pParam)->Program.SubroutinCount) == 2)
            {
                if (((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Status)
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
                else if (((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Status)
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
                else
                {
                    //呼叫LE
                    ((COrder*)pParam)->m_Action.DecideLineEndMove(
                        //_ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X,
                        //_ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
                        ((COrder*)pParam)->FinalWorkCoordinateData.X,
                        ((COrder*)pParam)->FinalWorkCoordinateData.Y,
                        _ttol(CommandResolve(Command, 3)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z,
                        ((COrder*)pParam)->DispenseLineSet.StayTime, ((COrder*)pParam)->DispenseLineSet.ShutdownDistance, ((COrder*)pParam)->DispenseLineSet.ShutdownDelay,
                        ((COrder*)pParam)->ZSet.ZBackHeight, ((COrder*)pParam)->ZSet.ZBackType,
                        ((COrder*)pParam)->DispenseLineEnd.HighSpeed, ((COrder*)pParam)->DispenseLineEnd.Width, ((COrder*)pParam)->DispenseLineEnd.Height, ((COrder*)pParam)->DispenseLineEnd.LowSpeed, ((COrder*)pParam)->DispenseLineEnd.Type,
                        ((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, 6000);
                }
            }   
        }
        ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Status = FALSE;
        ((COrder*)pParam)->RunData.ActionStatus.at(((COrder*)pParam)->Program.SubroutinCount) = 0;
    }
    if (CommandResolve(Command, 0) == L"ArcPoint")
    {
        ModifyPointOffSet(pParam, Command);
        if (((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Status == TRUE)
        {
            ((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Status = FALSE;
        }
        ((COrder*)pParam)->FinalWorkCoordinateData.X = _ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X;
        ((COrder*)pParam)->FinalWorkCoordinateData.Y = _ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y;
        VisionModify(pParam);
        ((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Status = TRUE;
        ((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).X = ((COrder*)pParam)->FinalWorkCoordinateData.X;
        ((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Y = ((COrder*)pParam)->FinalWorkCoordinateData.Y;
        //((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).X = _ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X;
        //((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Y = _ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y;
    }
    if (CommandResolve(Command, 0) == L"CirclePoint")
    {
        ModifyPointOffSet(pParam, Command);
        if (((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Status == TRUE)
        {
            ((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Status = FALSE;
        }
        ((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Status = TRUE;
        ((COrder*)pParam)->FinalWorkCoordinateData.X = _ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X;
        ((COrder*)pParam)->FinalWorkCoordinateData.Y = _ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y;
        VisionModify(pParam);
        ((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).X = ((COrder*)pParam)->FinalWorkCoordinateData.X;
        ((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Y = ((COrder*)pParam)->FinalWorkCoordinateData.Y;

        ((COrder*)pParam)->FinalWorkCoordinateData.X = _ttol(CommandResolve(Command, 4)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X;
        ((COrder*)pParam)->FinalWorkCoordinateData.Y = _ttol(CommandResolve(Command, 5)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y;
        VisionModify(pParam);
        ((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount).X = ((COrder*)pParam)->FinalWorkCoordinateData.X;
        ((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount).Y = ((COrder*)pParam)->FinalWorkCoordinateData.Y;

        
        //((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).X = _ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X;
        //((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Y = _ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y;
        ((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Z = _ttol(CommandResolve(Command, 3)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z;
        //((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount).X = _ttol(CommandResolve(Command, 4)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X;
        //((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount).Y = _ttol(CommandResolve(Command, 5)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y;
        ((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount).Z = _ttol(CommandResolve(Command, 6)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z;
    }
    if (CommandResolve(Command, 0) == L"GoHome")
    {
        LineGotoActionJudge(pParam);
        if (((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Status)//已經有offset修正
        {
            ((COrder*)pParam)->m_Action.DecideInitializationMachine(((COrder*)pParam)->GoHome.Speed1, ((COrder*)pParam)->GoHome.Speed2, ((COrder*)pParam)->GoHome.Axis, ((COrder*)pParam)->GoHome.MoveX, ((COrder*)pParam)->GoHome.MoveY, ((COrder*)pParam)->GoHome.MoveZ);
        }
        else
        {
            ((COrder*)pParam)->m_Action.DecideInitializationMachine(((COrder*)pParam)->GoHome.Speed1, ((COrder*)pParam)->GoHome.Speed2, ((COrder*)pParam)->GoHome.Axis, ((COrder*)pParam)->GoHome.MoveX, ((COrder*)pParam)->GoHome.MoveY, ((COrder*)pParam)->GoHome.MoveZ);
            ((COrder*)pParam)->m_Action.DecideVirtualPoint(((COrder*)pParam)->Program.SubroutinePointStack.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->Program.SubroutinePointStack.at(((COrder*)pParam)->Program.SubroutinCount).Y, ((COrder*)pParam)->Program.SubroutinePointStack.at(((COrder*)pParam)->Program.SubroutinCount).Z,
                ((COrder*)pParam)->DotSpeedSet.EndSpeed, ((COrder*)pParam)->DotSpeedSet.AccSpeed, 6000);
        }
    }
    if (CommandResolve(Command, 0) == L"VirtualPoint")
    {
        ModifyPointOffSet(pParam, Command);
        LineGotoActionJudge(pParam);
        ((COrder*)pParam)->FinalWorkCoordinateData.X = _ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X;
        ((COrder*)pParam)->FinalWorkCoordinateData.Y = _ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y;
        if (_ttol(CommandResolve(Command, 4)) == 1)
        {
            VisionModify(pParam);
        }
        ((COrder*)pParam)->m_Action.DecideVirtualPoint(
            //_ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X,
            //_ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
            ((COrder*)pParam)->FinalWorkCoordinateData.X,
            ((COrder*)pParam)->FinalWorkCoordinateData.Y,
            _ttol(CommandResolve(Command, 3)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z,
            ((COrder*)pParam)->DotSpeedSet.EndSpeed, ((COrder*)pParam)->DotSpeedSet.AccSpeed, 6000);
    }
    if (CommandResolve(Command, 0) == L"WaitPoint")
    {
        ModifyPointOffSet(pParam, Command);
        LineGotoActionJudge(pParam);
        ((COrder*)pParam)->FinalWorkCoordinateData.X = _ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X;
        ((COrder*)pParam)->FinalWorkCoordinateData.Y = _ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y;
        if (_ttol(CommandResolve(Command, 5)) == 1)
        {
            VisionModify(pParam);
        }
        ((COrder*)pParam)->m_Action.DecideWaitPoint(
            //_ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X,
            //_ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
            ((COrder*)pParam)->FinalWorkCoordinateData.X,
            ((COrder*)pParam)->FinalWorkCoordinateData.Y,
            _ttol(CommandResolve(Command, 3)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z,
            _ttol(CommandResolve(Command, 4)), ((COrder*)pParam)->DotSpeedSet.EndSpeed, ((COrder*)pParam)->DotSpeedSet.AccSpeed, 6000);
    }
    if (CommandResolve(Command, 0) == L"ParkPosition")
    {
        LineGotoActionJudge(pParam);
        if (((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Status)//已經有offset修正
        {
            if (((COrder*)pParam)->GlueData.GlueAuto)
            {
                ((COrder*)pParam)->m_Action.DecideParkPoint(((COrder*)pParam)->GlueData.ParkPositionData.X, ((COrder*)pParam)->GlueData.ParkPositionData.Y, ((COrder*)pParam)->GlueData.ParkPositionData.Z,
                    ((COrder*)pParam)->GlueData.GlueTime, ((COrder*)pParam)->GlueData.GlueWaitTime, ((COrder*)pParam)->GlueData.GlueStayTime, ((COrder*)pParam)->DotSpeedSet.EndSpeed, ((COrder*)pParam)->DotSpeedSet.AccSpeed, 6000);
            }
        }
        else
        {
            if (((COrder*)pParam)->GlueData.GlueAuto)
            {
                ((COrder*)pParam)->m_Action.DecideParkPoint(((COrder*)pParam)->GlueData.ParkPositionData.X, ((COrder*)pParam)->GlueData.ParkPositionData.Y, ((COrder*)pParam)->GlueData.ParkPositionData.Z,
                    ((COrder*)pParam)->GlueData.GlueTime, ((COrder*)pParam)->GlueData.GlueWaitTime, ((COrder*)pParam)->GlueData.GlueStayTime, ((COrder*)pParam)->DotSpeedSet.EndSpeed, ((COrder*)pParam)->DotSpeedSet.AccSpeed, 6000);
            }
            ((COrder*)pParam)->m_Action.DecideVirtualPoint(((COrder*)pParam)->Program.SubroutinePointStack.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->Program.SubroutinePointStack.at(((COrder*)pParam)->Program.SubroutinCount).Y, ((COrder*)pParam)->Program.SubroutinePointStack.at(((COrder*)pParam)->Program.SubroutinCount).Z,
                ((COrder*)pParam)->DotSpeedSet.EndSpeed, ((COrder*)pParam)->DotSpeedSet.AccSpeed, 6000);
        } 
    }
    if (CommandResolve(Command, 0) == L"StopPoint")
    {
        ModifyPointOffSet(pParam, Command);
        LineGotoActionJudge(pParam);
        ((COrder*)pParam)->FinalWorkCoordinateData.X = _ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X;
        ((COrder*)pParam)->FinalWorkCoordinateData.Y = _ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y;
        if (_ttol(CommandResolve(Command, 4)) == 1)
        {
            VisionModify(pParam);
        }
        ((COrder*)pParam)->m_Action.DecideVirtualPoint(
            //_ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X,
            //_ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
            ((COrder*)pParam)->FinalWorkCoordinateData.X,
            ((COrder*)pParam)->FinalWorkCoordinateData.Y,
            _ttol(CommandResolve(Command, 3)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z,
            ((COrder*)pParam)->DotSpeedSet.EndSpeed, ((COrder*)pParam)->DotSpeedSet.AccSpeed, 6000);
        ((COrder*)pParam)->Pause();
    }
    if (CommandResolve(Command, 0) == L"FillArea")
    {
        CString CommandBuff;
        CommandBuff.Format(_T("FillArea,%d,%d,%d"), _ttol(CommandResolve(Command, 4)), _ttol(CommandResolve(Command, 5)), _ttol(CommandResolve(Command, 6)));
        ModifyPointOffSet(pParam, CommandBuff);
        LineGotoActionJudge(pParam);
        //填充修正
        LONG FillAreaPoint1X, FillAreaPoint1Y, FillAreaPoint2X, FillAreaPoint2Y;
        ((COrder*)pParam)->FinalWorkCoordinateData.X = _ttol(CommandResolve(Command, 4)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X;
        ((COrder*)pParam)->FinalWorkCoordinateData.Y = _ttol(CommandResolve(Command, 5)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y;
        VisionModify(pParam);
        FillAreaPoint1X = ((COrder*)pParam)->FinalWorkCoordinateData.X;
        FillAreaPoint1Y = ((COrder*)pParam)->FinalWorkCoordinateData.Y;
        ((COrder*)pParam)->FinalWorkCoordinateData.X = _ttol(CommandResolve(Command, 7)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X;
        ((COrder*)pParam)->FinalWorkCoordinateData.Y = _ttol(CommandResolve(Command, 8)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y;
        VisionModify(pParam);
        FillAreaPoint2X = ((COrder*)pParam)->FinalWorkCoordinateData.X;
        FillAreaPoint2Y = ((COrder*)pParam)->FinalWorkCoordinateData.Y;

        ((COrder*)pParam)->m_Action.DecideFill(
            //_ttol(CommandResolve(Command, 4)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X,
            //_ttol(CommandResolve(Command, 5)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
            FillAreaPoint1X, FillAreaPoint1Y,
            _ttol(CommandResolve(Command, 6)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z,
            //_ttol(CommandResolve(Command, 7)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X,
            //_ttol(CommandResolve(Command, 8)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
            FillAreaPoint2X, FillAreaPoint2Y,
            _ttol(CommandResolve(Command, 9)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z,
            ((COrder*)pParam)->ZSet.ZBackHeight, ((COrder*)pParam)->ZSet.ZBackType,
            _ttol(CommandResolve(Command, 1)), _ttol(CommandResolve(Command, 2)), _ttol(CommandResolve(Command, 3)),
            ((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, 6000);
    }
    if (CommandResolve(Command, 0) == L"Output")
    {
        ((COrder*)pParam)->m_Action.DecideOutPutSign(_ttol(CommandResolve(Command, 1)), _ttol(CommandResolve(Command, 2)));
    }
    if (CommandResolve(Command, 0) == L"Input")
    {
        if (((COrder*)pParam)->m_Action.DecideInPutSign(_ttol(CommandResolve(Command, 1)), _ttol(CommandResolve(Command, 2))) && _ttol(CommandResolve(Command, 3)))
        {
            ((COrder*)pParam)->Program.LabelName = _T("Label,") + CommandResolve(Command, 3);//跳到標籤
            //((COrder*)pParam)->RunData.RunCount = _ttol(CommandResolve(Command, 3)) - 2;//跳到地址
        }
    }
#endif
    /************************************************************運動參數***********************************************************/
    if (CommandResolve(Command, 0) == L"DispenseDotSet")
    {
        ((COrder*)pParam)->DispenseDotSet.GlueOpenTime = _ttol(CommandResolve(Command, 1));
        ((COrder*)pParam)->DispenseDotSet.GlueCloseTime = _ttol(CommandResolve(Command, 2));
    }
    if (CommandResolve(Command, 0) == L"DispenseDotEnd")
    {
        ((COrder*)pParam)->DispenseDotEnd.RiseDistance = _ttol(CommandResolve(Command, 1));
        ((COrder*)pParam)->DispenseDotEnd.RiseLowSpeed = _ttol(CommandResolve(Command, 2));
        ((COrder*)pParam)->DispenseDotEnd.RiseHightSpeed = _ttol(CommandResolve(Command, 3));
    }
    if (CommandResolve(Command, 0) == L"DotSpeedSet")
    {
        ((COrder*)pParam)->DotSpeedSet.EndSpeed = _ttol(CommandResolve(Command, 1));
    }
    if (CommandResolve(Command, 0) == L"DotAccPercent")
    {
        ((COrder*)pParam)->DotSpeedSet.AccSpeed = ((COrder*)pParam)->DotSpeedSet.AccSpeed * _ttol(CommandResolve(Command, 1)) / 100;
    }
    if (CommandResolve(Command, 0) == L"DispenseLineSet")
    {
        ((COrder*)pParam)->DispenseLineSet.BeforeMoveDelay = _ttol(CommandResolve(Command, 1));
        ((COrder*)pParam)->DispenseLineSet.BeforeMoveDistance = _ttol(CommandResolve(Command, 2));
        ((COrder*)pParam)->DispenseLineSet.NodeTime = _ttol(CommandResolve(Command, 3));
        ((COrder*)pParam)->DispenseLineSet.StayTime = _ttol(CommandResolve(Command, 4));
        ((COrder*)pParam)->DispenseLineSet.ShutdownDistance = _ttol(CommandResolve(Command, 5));
        ((COrder*)pParam)->DispenseLineSet.ShutdownDelay = _ttol(CommandResolve(Command, 6));
    }
    if (CommandResolve(Command, 0) == L"DispenseLineEnd")
    {
        ((COrder*)pParam)->DispenseLineEnd.Type = _ttol(CommandResolve(Command, 1));
        ((COrder*)pParam)->DispenseLineEnd.LowSpeed = _ttol(CommandResolve(Command, 2));
        ((COrder*)pParam)->DispenseLineEnd.Height = _ttol(CommandResolve(Command, 3));
        ((COrder*)pParam)->DispenseLineEnd.Width = _ttol(CommandResolve(Command, 4));
        ((COrder*)pParam)->DispenseLineEnd.HighSpeed = _ttol(CommandResolve(Command, 5));
    }
    if (CommandResolve(Command, 0) == L"LineSpeedSet")
    {
        ((COrder*)pParam)->LineSpeedSet.EndSpeed = _ttol(CommandResolve(Command, 1));
    }
    if (CommandResolve(Command, 0) == L"ZGoBack")
    {
        ((COrder*)pParam)->ZSet.ZBackHeight = _ttol(CommandResolve(Command, 1));
        ((COrder*)pParam)->ZSet.ZBackType = _ttol(CommandResolve(Command, 2));
    }
    if (CommandResolve(Command, 0) == L"DispenseAccSet") 
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
    if (CommandResolve(Command, 0) == L"Initialize")
    { 
        ((COrder*)pParam)->ParameterDefult();
    }
    if (CommandResolve(Command, 0) == L"DispenserSwitch")
    {
        ((COrder*)pParam)->m_Action.g_bIsDispend = _ttol(CommandResolve(Command, 1));
    }
    if (CommandResolve(Command, 0) == L"DispenserSwitchSet")
    {
        ((COrder*)pParam)->m_Action.g_iNumberGluePort = _ttol(CommandResolve(Command, 1));
    }
    /************************************************************影像***************************************************************/
#ifdef VI
    if (CommandResolve(Command, 0) == L"FindMark")
    {
        ModifyPointOffSet(pParam, Command);
        ((COrder*)pParam)->FindMark.Point.Status = TRUE;
        ((COrder*)pParam)->FindMark.Point.X = _ttol(CommandResolve(Command, 1)) - ((COrder*)pParam)->VisionSet.AdjustOffsetX + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X;
        ((COrder*)pParam)->FindMark.Point.Y = _ttol(CommandResolve(Command, 2)) - ((COrder*)pParam)->VisionSet.AdjustOffsetY + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y;
        ((COrder*)pParam)->FindMark.Point.Z = _ttol(CommandResolve(Command, 3)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z;
        ((COrder*)pParam)->FindMark.LoadModelNum = _ttol(CommandResolve(Command, 4)) - 1; //編號從1開始
        ((COrder*)pParam)->FindMark.FocusDistance = _ttol(CommandResolve(Command, 5));
        if (((COrder*)pParam)->FindMark.LoadModelNum >= 0 && ((COrder*)pParam)->FindMark.LoadModelNum < ((COrder*)pParam)->VisionFile.AllModelName.size())
        {
            if (FileExist(((COrder*)pParam)->VisionFile.ModelPath + ((COrder*)pParam)->VisionFile.AllModelName.at(((COrder*)pParam)->FindMark.LoadModelNum)))//判斷檔案是否存在
            {
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
    if (CommandResolve(Command, 0) == L"FindMarkAdjust")
    {     
        if (*(int*)((COrder*)pParam)->FindMark.MilModel != 0)
        {
            if (!((COrder*)pParam)->VisionSerchError.Manuallymode)
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
                    ((COrder*)pParam)->m_Action.DecideLineEndMove(0, 0,
                        ((COrder*)pParam)->ZSet.ZBackHeight, ((COrder*)pParam)->ZSet.ZBackType, 0, 0, 0, 0,
                        ((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, 6000);
                    #endif
                }
            }
            else
            {
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
                    ((COrder*)pParam)->m_Action.DecideLineEndMove(0, 0,
                        ((COrder*)pParam)->ZSet.ZBackHeight, ((COrder*)pParam)->ZSet.ZBackType, 0, 0, 0, 0,
                        ((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, 6000);
                    #endif
                    _cwprintf(L"手動模式找到\r\n");
                }
            }    
            //紀錄對位點
            ((COrder*)pParam)->VisionOffset.Contraposition.Status = TRUE;
            ((COrder*)pParam)->VisionOffset.Contraposition.X = ((COrder*)pParam)->FindMark.Point.X;
            ((COrder*)pParam)->VisionOffset.Contraposition.Y = ((COrder*)pParam)->FindMark.Point.Y;
        }
        else
        {
            AfxMessageBox(_T("FindMarkAdjust need one FindMark"));
        }
    }
    if (CommandResolve(Command, 0) == L"FiducialMark")
    {
        ModifyPointOffSet(pParam, Command);
        if (!((COrder*)pParam)->FindMark.Point.Status)
        {
            if (!((COrder*)pParam)->FiducialMark1.Point.Status)
            {
                ((COrder*)pParam)->FiducialMark1.Point.Status = TRUE;
                ((COrder*)pParam)->FiducialMark1.Point.X = _ttol(CommandResolve(Command, 1)) - ((COrder*)pParam)->VisionSet.AdjustOffsetX + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X;
                ((COrder*)pParam)->FiducialMark1.Point.Y = _ttol(CommandResolve(Command, 2)) - ((COrder*)pParam)->VisionSet.AdjustOffsetY + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y;
                ((COrder*)pParam)->FiducialMark1.Point.Z = _ttol(CommandResolve(Command, 3)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z;
                ((COrder*)pParam)->FiducialMark1.LoadModelNum = _ttol(CommandResolve(Command, 4)) - 1; //編號從1開始
                ((COrder*)pParam)->FiducialMark1.FocusDistance = _ttol(CommandResolve(Command, 5));
                if (((COrder*)pParam)->FiducialMark1.LoadModelNum >= 0 && ((COrder*)pParam)->FiducialMark1.LoadModelNum < ((COrder*)pParam)->VisionFile.AllModelName.size())//判斷編號是否大於檔案標號最大值
                {
                    if (FileExist(((COrder*)pParam)->VisionFile.ModelPath + ((COrder*)pParam)->VisionFile.AllModelName.at(((COrder*)pParam)->FiducialMark1.LoadModelNum)))//判斷檔案是否存在
                    {
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
                    ((COrder*)pParam)->FiducialMark2.Point.Z = _ttol(CommandResolve(Command, 3)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z;
                    ((COrder*)pParam)->FiducialMark2.LoadModelNum = _ttol(CommandResolve(Command, 4)) - 1; //編號從1開始
                    ((COrder*)pParam)->FiducialMark2.FocusDistance = _ttol(CommandResolve(Command, 5));
                    if (((COrder*)pParam)->FiducialMark2.LoadModelNum >= 0 && ((COrder*)pParam)->FiducialMark2.LoadModelNum < ((COrder*)pParam)->VisionFile.AllModelName.size())//判斷編號是否大於檔案標號最大值
                    {
                        if (FileExist(((COrder*)pParam)->VisionFile.ModelPath +((COrder*)pParam)->VisionFile.AllModelName.at(((COrder*)pParam)->FiducialMark2.LoadModelNum)))//判斷檔案是否存在
                        {
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
                    ((COrder*)pParam)->FiducialMark1.Point.Z = _ttol(CommandResolve(Command, 3)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z;
                    ((COrder*)pParam)->FiducialMark1.LoadModelNum = _ttol(CommandResolve(Command, 4)) - 1; //編號從1開始
                    ((COrder*)pParam)->FiducialMark1.FocusDistance = _ttol(CommandResolve(Command, 5));
                    if (((COrder*)pParam)->FiducialMark1.LoadModelNum >= 0 && ((COrder*)pParam)->FiducialMark1.LoadModelNum < ((COrder*)pParam)->VisionFile.AllModelName.size())//判斷編號是否大於檔案標號最大值
                    {
                        if (FileExist(((COrder*)pParam)->VisionFile.ModelPath + ((COrder*)pParam)->VisionFile.AllModelName.at(((COrder*)pParam)->FiducialMark1.LoadModelNum)))//判斷檔案是否存在
                        {
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
        }  
    }
    if (CommandResolve(Command, 0) == L"FiducialMarkAdjust")
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
                    ((COrder*)pParam)->m_Action.DecideLineEndMove(0, 0,
                        ((COrder*)pParam)->ZSet.ZBackHeight, ((COrder*)pParam)->ZSet.ZBackType, 0, 0, 0, 0,
                        ((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, 6000);
                    #endif
                    ((COrder*)pParam)->FiducialMark1.FindMarkStatus = TRUE;
                }
            }
            else if(((COrder*)pParam)->VisionSerchError.Manuallymode && !((COrder*)pParam)->FiducialMark1.FindMarkStatus)//手動未找到
            {
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
                    ((COrder*)pParam)->m_Action.DecideLineEndMove(0, 0,
                        ((COrder*)pParam)->ZSet.ZBackHeight, ((COrder*)pParam)->ZSet.ZBackType, 0, 0, 0, 0,
                        ((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, 6000);
                    #endif
                    ((COrder*)pParam)->VisionSerchError.Manuallymode = FALSE;
                    ((COrder*)pParam)->FiducialMark1.FindMarkStatus = TRUE;
                    _cwprintf(L"手動模式第一點找到\r\n");
                }
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
                    ((COrder*)pParam)->m_Action.DecideLineEndMove(0, 0,
                        ((COrder*)pParam)->ZSet.ZBackHeight, ((COrder*)pParam)->ZSet.ZBackType, 0, 0, 0, 0,
                        ((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, 6000);
                    #endif
                    ((COrder*)pParam)->FiducialMark2.FindMarkStatus = TRUE;
                    _cwprintf(L"第一點找到移動至第二點找到\r\n");
                }
            }
            else if(((COrder*)pParam)->VisionSerchError.Manuallymode && !((COrder*)pParam)->FiducialMark2.FindMarkStatus && ((COrder*)pParam)->FiducialMark1.FindMarkStatus)//如果手動模式開啟 且對位點2未找到但對位點1找到
            {
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
                        ((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, 6000);
                    #endif
                    //找到清除手動狀態、將對位點依設置已經有Offset
                    ((COrder*)pParam)->VisionSerchError.Manuallymode = FALSE;
                    ((COrder*)pParam)->FiducialMark2.FindMarkStatus = TRUE;
                    _cwprintf(L"手動模式第一點找到第二點找到\r\n");
                }
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
#endif
    if (CommandResolve(Command, 0) == L"FindFiducialAngle")
    {
        ((COrder*)pParam)->VisionSet.Startangle = _ttol(CommandResolve(Command, 1));
        ((COrder*)pParam)->VisionSet.Endangle = _ttol(CommandResolve(Command, 2));
    }
    if (CommandResolve(Command, 0) == L"CameraTrigger")
    {
        ModifyPointOffSet(pParam, Command);
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
    g_pSubroutineThread = NULL;
    return 0;
}
/*運動狀態判斷*/
void COrder::LineGotoActionJudge(LPVOID pParam)
{
#ifdef MOVE
    if (((COrder*)pParam)->RunData.ActionStatus.at(((COrder*)pParam)->Program.SubroutinCount) == 1)//LS時 移動至LS虛擬點
    {
        ((COrder*)pParam)->m_Action.DecideVirtualPoint(((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Y, ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Z,
            ((COrder*)pParam)->DotSpeedSet.EndSpeed, ((COrder*)pParam)->DotSpeedSet.AccSpeed, 6000);
    }
    else if (((COrder*)pParam)->RunData.ActionStatus.at(((COrder*)pParam)->Program.SubroutinCount) == 2)//LP時 斷膠抬升
    {
        ((COrder*)pParam)->m_Action.DecideLineEndMove(((COrder*)pParam)->DispenseLineSet.StayTime, ((COrder*)pParam)->DispenseLineSet.ShutdownDelay,
            ((COrder*)pParam)->ZSet.ZBackHeight, ((COrder*)pParam)->ZSet.ZBackType, ((COrder*)pParam)->DispenseLineEnd.HighSpeed, ((COrder*)pParam)->DispenseLineEnd.Width, ((COrder*)pParam)->DispenseLineEnd.Height, ((COrder*)pParam)->DispenseLineEnd.LowSpeed,
            ((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, 6000);
    }
#endif
    //狀態清除
    ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Status = FALSE;
    ((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Status = FALSE;
    ((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Status = FALSE;
    ((COrder*)pParam)->RunData.ActionStatus.at(((COrder*)pParam)->Program.SubroutinCount) = 0;
}
/*Subroutine修正點偏移量*/
void COrder::ModifyPointOffSet(LPVOID pParam ,CString Command)
{
#ifdef MOVE
    if (!((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Status)
    { 
        CString XYZOffset = ((COrder*)pParam)->m_Action.NowOffSet(_ttol(CommandResolve(Command, 1)), _ttol(CommandResolve(Command, 2)), _ttol(CommandResolve(Command, 3)));
        ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X = _ttol(CommandResolve(XYZOffset, 0));
        ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y = _ttol(CommandResolve(XYZOffset, 1));
        ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z = _ttol(CommandResolve(XYZOffset, 2));
        ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Status = TRUE;
        //修正完畢斷膠抬升
        ((COrder*)pParam)->m_Action.DecideLineEndMove(((COrder*)pParam)->DispenseLineSet.StayTime, ((COrder*)pParam)->DispenseLineSet.ShutdownDelay,
            ((COrder*)pParam)->ZSet.ZBackHeight, ((COrder*)pParam)->ZSet.ZBackType, 
            ((COrder*)pParam)->DispenseLineEnd.HighSpeed, ((COrder*)pParam)->DispenseLineEnd.Width, ((COrder*)pParam)->DispenseLineEnd.Height, ((COrder*)pParam)->DispenseLineEnd.LowSpeed,
            ((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, 6000);
    }
#endif
}
/*影像修正*/
void COrder::VisionModify(LPVOID pParam)
{           
#ifdef VI
    VI_CorrectLocation(((COrder*)pParam)->FinalWorkCoordinateData.X, ((COrder*)pParam)->FinalWorkCoordinateData.Y,
        ((COrder*)pParam)->VisionOffset.Contraposition.X, ((COrder*)pParam)->VisionOffset.Contraposition.Y,
        ((COrder*)pParam)->VisionOffset.OffsetX, ((COrder*)pParam)->VisionOffset.OffsetY, ((COrder*)pParam)->VisionOffset.Angle, ((COrder*)pParam)->VisionSet.ModifyMode);
#endif
}
/*影像沒找到處理方法*/
void COrder::VisionFindMarkError(LPVOID pParam)
{
    switch (((COrder*)pParam)->VisionSerchError.SearchError)
    {
    case 1:
#ifdef VI
        //無找到抬升
        #ifdef MOVE
        ((COrder*)pParam)->m_Action.DecideLineEndMove(0, 0,
            ((COrder*)pParam)->ZSet.ZBackHeight, ((COrder*)pParam)->ZSet.ZBackType, 0, 0, 0, 0,
            ((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, 6000);
        #endif
        if (((COrder*)pParam)->VisionTrigger.AdjustStatus == 1 || ((COrder*)pParam)->VisionTrigger.AdjustStatus == 2)
        {
            if (((COrder*)pParam)->VisionTrigger.Trigger1.size())//有Trigger時
            {
                for (int i = 0; i < ((COrder*)pParam)->VisionTrigger.Trigger1.size(); i++)
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
                            ((COrder*)pParam)->m_Action.DecideLineEndMove(0, 0,
                                ((COrder*)pParam)->ZSet.ZBackHeight, ((COrder*)pParam)->ZSet.ZBackType, 0, 0, 0, 0,
                                ((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, 6000);
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
                            ((COrder*)pParam)->m_Action.DecideLineEndMove(0, 0,
                                ((COrder*)pParam)->ZSet.ZBackHeight, ((COrder*)pParam)->ZSet.ZBackType, 0, 0, 0, 0,
                                ((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, 6000);
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
                for (int i = 0; i < ((COrder*)pParam)->VisionTrigger.Trigger2.size(); i++)
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
                        ((COrder*)pParam)->m_Action.DecideLineEndMove(0, 0,
                            ((COrder*)pParam)->ZSet.ZBackHeight, ((COrder*)pParam)->ZSet.ZBackType, 0, 0, 0, 0,
                            ((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, 6000);
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
    case 2:                                         
        ((COrder*)pParam)->Stop();
        break;
    case 3:
        //無找到抬升
        #ifdef MOVE
        ((COrder*)pParam)->m_Action.DecideLineEndMove(0, 0,
            ((COrder*)pParam)->ZSet.ZBackHeight, ((COrder*)pParam)->ZSet.ZBackType, 0, 0, 0, 0,
            ((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, 6000);
        #endif
        ((COrder*)pParam)->Pause(); 
        if (((COrder*)pParam)->VisionTrigger.AdjustStatus == 2 || ((COrder*)pParam)->VisionTrigger.AdjustStatus == 3)
        {
            ((COrder*)pParam)->VisionSerchError.Pausemode = TRUE;
            ((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount))--;//重複一次命令
        }
        break;
    case 4:
        switch (((COrder*)pParam)->VisionSerchError.pQuestion->DoModal())
        {
        case 0:
            ((COrder*)pParam)->VisionSerchError.SearchError = 1;
            VisionFindMarkError(pParam);
            break;
        case 1:
            ((COrder*)pParam)->Stop();
            break;
        case 2:
            ((COrder*)pParam)->FiducialMark1.FindMarkStatus = FALSE;
            ((COrder*)pParam)->FiducialMark2.FindMarkStatus = FALSE;
            ((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount))--;
            break;
        case 3:
            ((COrder*)pParam)->VisionSerchError.SearchError = 5;
            VisionFindMarkError(pParam);
            break;
        case 4:
            break;
        default:
            break;
        }
        break;
    case 5:
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
/*指令分解*/
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
    //Repeat狀態清除
    RepeatData.LoopSwitch = FALSE;
    RepeatData.StepRepeatSwitch = FALSE;
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
    /****************************************************************/
    Time = 0;
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
}
/*劃分主程式和副程式*/
void COrder::MainSubProgramSeparate()
{
    Command.clear();
    std::vector<UINT> SubProgramCount;
    BOOL SubroutineFlag = FALSE;
    for (UINT i = 0; i < CommandMemory.size(); i++) {
        if (CommandResolve(CommandMemory.at(i), 0) == L"SubProgramStart")
        {
            SubProgramCount.push_back(i);
            SubroutineFlag = TRUE;
        }
        if (!SubroutineFlag)
        {
            CommandSwap.push_back(CommandMemory.at(i));
        }
        if (CommandResolve(CommandMemory.at(i), 0) == L"SubProgramEnd")
        {
            SubProgramCount.push_back(i);
            SubroutineFlag = FALSE;
        }
    }
    Command.push_back(CommandSwap);
    CommandSwap.clear();
    RunData.RunCount.push_back(0);
    for (UINT i = 0; i < SubProgramCount.size(); i++)
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
/*阻斷處理方式(加入StepRepeat時)*/
void COrder::BlockProcessStart(CString Command, LPVOID pParam)
{
    CString BlockBuff;
    for (int i = 0; i < ((COrder*)pParam)->RepeatData.StepRepeatBlockData.front().BlockPosition.size(); i++)//搜尋第一項阻斷陣列 是否有1-1
    {
        _cwprintf(_T("%d-%d.StepRepeatX 檢查阻斷\n"), ((COrder*)pParam)->RepeatData.StepRepeatCountX.front(), ((COrder*)pParam)->RepeatData.StepRepeatCountY.front());
        //S型
        if (_ttol(CommandResolve(Command, 5)) == 1)
        {
            //S型 阻斷編號
            if (!((COrder*)pParam)->RepeatData.SSwitch.front())
            {
                BlockBuff.Format(_T("%d-%d"), ((COrder*)pParam)->RepeatData.StepRepeatCountX.front(),
                    (_ttol(CommandResolve(Command, 4)) + 1 - ((COrder*)pParam)->RepeatData.StepRepeatCountY.front()));
                _cwprintf(L"標籤只有Y變X正常:%s\n", BlockBuff);
            }
            else
            {
                BlockBuff.Format(_T("%d-%d"), (_ttol(CommandResolve(Command, 3)) + 1 - ((COrder*)pParam)->RepeatData.StepRepeatCountX.front()),
                    (_ttol(CommandResolve(Command, 4)) + 1 - ((COrder*)pParam)->RepeatData.StepRepeatCountY.front()));
                _cwprintf(L"標籤XY變:%s\n", BlockBuff);
            }
            if (((COrder*)pParam)->RepeatData.StepRepeatBlockData.front().BlockPosition.at(i) == BlockBuff)
            {
                _cwprintf(L"有阻斷\n");
                if (((COrder*)pParam)->RepeatData.StepRepeatCountX.front() > 1)
                {
                    if (((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X == ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.front() + (_ttol(CommandResolve(Command, 1))*(_ttol(CommandResolve(Command, 3)) - 1)))
                    {
                        ((COrder*)pParam)->RepeatData.SSwitch.front() = FALSE;
                    }
                    else if (((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X == ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.front())
                    {
                        ((COrder*)pParam)->RepeatData.SSwitch.front() = TRUE;
                    }

                    if (!((COrder*)pParam)->RepeatData.SSwitch.front())
                    {
                        ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X = ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X - _ttol(CommandResolve(Command, 1));
                    }
                    else
                    {
                        ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X = ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X + _ttol(CommandResolve(Command, 1));
                    }
                    ((COrder*)pParam)->RepeatData.StepRepeatCountX.front()--;
                }
                else if (((COrder*)pParam)->RepeatData.StepRepeatCountY.front() > 1)
                {
                    ((COrder*)pParam)->RepeatData.StepRepeatCountX.front() = _ttol(CommandResolve(Command, 3));
                    ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y = ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y + _ttol(CommandResolve(Command, 2));
                    ((COrder*)pParam)->RepeatData.StepRepeatCountY.front()--;
                }
                else
                {
                    _cwprintf(L"刪除所有第一陣列\n");
                    ((COrder*)pParam)->RepeatData.StepRepeatBlockData.erase(((COrder*)pParam)->RepeatData.StepRepeatBlockData.begin());
                    ((COrder*)pParam)->RepeatData.SSwitch.erase(((COrder*)pParam)->RepeatData.SSwitch.begin());
                    ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X = ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.front();
                    ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y = ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.front();
                    ((COrder*)pParam)->RepeatData.StepRepeatNum.erase(((COrder*)pParam)->RepeatData.StepRepeatNum.begin());
                    ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.erase(((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.begin());
                    ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.erase(((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.begin());
                    ((COrder*)pParam)->RepeatData.StepRepeatCountX.erase(((COrder*)pParam)->RepeatData.StepRepeatCountX.begin());
                    ((COrder*)pParam)->RepeatData.StepRepeatCountY.erase(((COrder*)pParam)->RepeatData.StepRepeatCountY.begin());
                }  
            }
        }
        //N型
        if (_ttol(CommandResolve(Command, 5)) == 2)
        {
            //N型 阻斷編號
            BlockBuff.Format(_T("%d-%d"), (_ttol(CommandResolve(Command, 3)) + 1 - ((COrder*)pParam)->RepeatData.StepRepeatCountX.front()),
                (_ttol(CommandResolve(Command, 4)) + 1 - ((COrder*)pParam)->RepeatData.StepRepeatCountY.front()));
            if (((COrder*)pParam)->RepeatData.StepRepeatBlockData.front().BlockPosition.at(i) == BlockBuff)
            {
                _cwprintf(L"有阻斷\n");
                if (((COrder*)pParam)->RepeatData.StepRepeatCountX.front() > 1)
                {
                    ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X = ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X + _ttol(CommandResolve(Command, 1));
                    ((COrder*)pParam)->RepeatData.StepRepeatCountX.front()--;
                }
                else if (((COrder*)pParam)->RepeatData.StepRepeatCountY.front() > 1)
                {
                    //N型X回最初位置
                    ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X = ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.front();
                    ((COrder*)pParam)->RepeatData.StepRepeatCountX.front() = _ttol(CommandResolve(Command, 3));
                    ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y = ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y + _ttol(CommandResolve(Command, 2));
                    ((COrder*)pParam)->RepeatData.StepRepeatCountY.front()--;
                }
                else
                {
                    _cwprintf(L"刪除所有第一陣列\n");
                    ((COrder*)pParam)->RepeatData.StepRepeatBlockData.erase(((COrder*)pParam)->RepeatData.StepRepeatBlockData.begin());
                    ((COrder*)pParam)->RepeatData.SSwitch.erase(((COrder*)pParam)->RepeatData.SSwitch.begin());
                    ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X = ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.front();
                    ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y = ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.front();
                    ((COrder*)pParam)->RepeatData.StepRepeatNum.erase(((COrder*)pParam)->RepeatData.StepRepeatNum.begin());
                    ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.erase(((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.begin());
                    ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.erase(((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.begin());
                    ((COrder*)pParam)->RepeatData.StepRepeatCountX.erase(((COrder*)pParam)->RepeatData.StepRepeatCountX.begin());
                    ((COrder*)pParam)->RepeatData.StepRepeatCountY.erase(((COrder*)pParam)->RepeatData.StepRepeatCountY.begin());
                }      
            }
        }
    }
    ((COrder*)pParam)->RepeatData.StepRepeatLabel = CommandResolve(Command, 6);
}
/*阻斷處理方式(執行中StepRepeat時)*/
void COrder::BlockProcessExecute(CString Command, LPVOID pParam, int NowCount)
{
    CString BlockBuff;
    for (int i = 0; i < ((COrder*)pParam)->RepeatData.StepRepeatBlockData.at(NowCount).BlockPosition.size(); i++)//搜尋最後一項阻斷陣列 是否有1-1
    {
        _cwprintf(L"第%d:%d-%d.StepRepeatX 檢查阻斷\n", NowCount,((COrder*)pParam)->RepeatData.StepRepeatCountX.at(NowCount), ((COrder*)pParam)->RepeatData.StepRepeatCountY.at(NowCount));
        //S型
        if (_ttol(CommandResolve(Command, 5)) == 1)
        {
            //S型 阻斷編號
            if (!((COrder*)pParam)->RepeatData.SSwitch.at(NowCount))
            {
                BlockBuff.Format(_T("%d-%d"), ((COrder*)pParam)->RepeatData.StepRepeatCountX.at(NowCount),
                    (_ttol(CommandResolve(Command, 4)) + 1 - ((COrder*)pParam)->RepeatData.StepRepeatCountY.at(NowCount)));
                _cwprintf(L"第%d:標籤只有Y變X正常:%s\n", NowCount, BlockBuff);
            }
            else
            { 
                BlockBuff.Format(_T("%d-%d"), (_ttol(CommandResolve(Command, 3)) + 1 - ((COrder*)pParam)->RepeatData.StepRepeatCountX.at(NowCount)),
                    (_ttol(CommandResolve(Command, 4)) + 1 - ((COrder*)pParam)->RepeatData.StepRepeatCountY.at(NowCount)));
                _cwprintf(L"第%d:標籤XY變:%s\n", NowCount, BlockBuff);
            }
            if (((COrder*)pParam)->RepeatData.StepRepeatBlockData.at(NowCount).BlockPosition.at(i) == BlockBuff)
            {
                _cwprintf(L"第%d:有阻斷\n", NowCount);
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
                }
                else if (((COrder*)pParam)->RepeatData.StepRepeatCountY.at(NowCount) > 1)
                {
                    ((COrder*)pParam)->RepeatData.SSwitch.at(NowCount) = !((COrder*)pParam)->RepeatData.SSwitch.at(NowCount);
                    ((COrder*)pParam)->RepeatData.StepRepeatCountX.at(NowCount) = _ttol(CommandResolve(Command, 3));
                    ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y = ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y + _ttol(CommandResolve(Command, 2));
                    ((COrder*)pParam)->RepeatData.StepRepeatCountY.at(NowCount)--;
                    ((COrder*)pParam)->RepeatData.StepRepeatLabel = _T("StepRepeatLabel,") + CommandResolve(Command, 6);
                }
                else
                {
                    _cwprintf(L"第%d:刪除所有陣列\n", NowCount);
                    ((COrder*)pParam)->RepeatData.StepRepeatBlockData.erase(((COrder*)pParam)->RepeatData.StepRepeatBlockData.begin() + i);
                    ((COrder*)pParam)->RepeatData.SSwitch.erase(((COrder*)pParam)->RepeatData.SSwitch.begin() + i);;
                    ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X = ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.at(NowCount);
                    ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y = ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.at(NowCount);
                    ((COrder*)pParam)->RepeatData.StepRepeatNum.erase(((COrder*)pParam)->RepeatData.StepRepeatNum.begin() + i);;
                    ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.erase(((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.begin() + i);;
                    ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.erase(((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.begin() + i);;
                    ((COrder*)pParam)->RepeatData.StepRepeatCountX.erase(((COrder*)pParam)->RepeatData.StepRepeatCountX.begin() + i);;
                    ((COrder*)pParam)->RepeatData.StepRepeatCountY.erase(((COrder*)pParam)->RepeatData.StepRepeatCountY.begin() + i);;
                    ((COrder*)pParam)->RepeatData.StepRepeatLabel = _T("");
                }
            }
        }
        //N型
        if (_ttol(CommandResolve(Command, 5)) == 2)
        {
            //N型 阻斷編號
            BlockBuff.Format(_T("%d-%d"), (_ttol(CommandResolve(Command, 3)) + 1 - ((COrder*)pParam)->RepeatData.StepRepeatCountX.at(NowCount)),
                (_ttol(CommandResolve(Command, 4)) + 1 - ((COrder*)pParam)->RepeatData.StepRepeatCountY.at(NowCount)));
            _cwprintf(L"第%d:標籤:%s\n", NowCount, BlockBuff);
            if (((COrder*)pParam)->RepeatData.StepRepeatBlockData.at(NowCount).BlockPosition.at(i) == BlockBuff)
            {
                _cwprintf(L"第%d:有阻斷\n", NowCount);
                if (((COrder*)pParam)->RepeatData.StepRepeatCountX.at(NowCount) > 1)
                {
                    ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X = ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X + _ttol(CommandResolve(Command, 1));
                    ((COrder*)pParam)->RepeatData.StepRepeatCountX.at(NowCount)--;
                    ((COrder*)pParam)->RepeatData.StepRepeatLabel = _T("StepRepeatLabel,") + CommandResolve(Command, 6);
                }
                else if (((COrder*)pParam)->RepeatData.StepRepeatCountY.at(NowCount) > 1)
                {
                    //N型X回最初位置
                    ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X = ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.at(NowCount);
                    ((COrder*)pParam)->RepeatData.StepRepeatCountX.at(NowCount) = _ttol(CommandResolve(Command, 3));
                    ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y = ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y + _ttol(CommandResolve(Command, 2));
                    ((COrder*)pParam)->RepeatData.StepRepeatCountY.at(NowCount)--;
                    ((COrder*)pParam)->RepeatData.StepRepeatLabel = _T("StepRepeatLabel,") + CommandResolve(Command, 6);
                }
                else
                {
                    _cwprintf(L"第%d:刪除所有陣列\n", NowCount);
                    ((COrder*)pParam)->RepeatData.StepRepeatBlockData.erase(((COrder*)pParam)->RepeatData.StepRepeatBlockData.begin() + i);
                    ((COrder*)pParam)->RepeatData.SSwitch.erase(((COrder*)pParam)->RepeatData.SSwitch.begin() + i);;
                    ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X = ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.at(NowCount);
                    ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y = ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.at(NowCount);
                    ((COrder*)pParam)->RepeatData.StepRepeatNum.erase(((COrder*)pParam)->RepeatData.StepRepeatNum.begin() + i);;
                    ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.erase(((COrder*)pParam)->RepeatData.StepRepeatInitOffsetX.begin() + i);;
                    ((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.erase(((COrder*)pParam)->RepeatData.StepRepeatInitOffsetY.begin() + i);;
                    ((COrder*)pParam)->RepeatData.StepRepeatCountX.erase(((COrder*)pParam)->RepeatData.StepRepeatCountX.begin() + i);;
                    ((COrder*)pParam)->RepeatData.StepRepeatCountY.erase(((COrder*)pParam)->RepeatData.StepRepeatCountY.begin() + i);;
                    ((COrder*)pParam)->RepeatData.StepRepeatLabel = _T("");
                }
            }
        }
    }
}