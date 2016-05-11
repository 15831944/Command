// Order.cpp : 實作檔
//

#include "stdafx.h"
#include "CommandTest.h"
#include "Order.h"


// COrder

IMPLEMENT_DYNAMIC(COrder, CWnd)

COrder::COrder()
{
    wakeEvent = NULL;
    DispenseDotSet.GlueOpenTime = 0;
    DispenseDotSet.GlueCloseTime = 0;
    DispenseDotEnd.RiseDistance = 0;
    DispenseDotEnd.RiseHightSpeed = 0;
    DispenseDotEnd.RiseLowSpeed = 0;
    DispenseLineSet.BeforeMoveDelay = 0;
    DispenseLineSet.BeforeMoveDistance = 0;
    DispenseLineSet.NodeTime = 0;
    DispenseLineSet.ShutdownDelay = 0;
    DispenseLineSet.ShutdownDistance = 0;
    DispenseLineSet.StayTime = 0;
    DispenseLineEnd.Type = 0;
    DispenseLineEnd.HighSpeed = 0;
    DispenseLineEnd.LowSpeed = 0;
    DispenseLineEnd.Width = 0;
    DispenseLineEnd.Height = 0;
    DotSpeedSet.AccSpeed = 100000;
    DotSpeedSet.EndSpeed = 30000;
    LineSpeedSet.AccSpeed = 100000;
    LineSpeedSet.EndSpeed = 30000;
    ZSet.ZBackHeight = 0;
    ZSet.ZBackType = 0;   
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
        if (CommandMemory.at(CommandMemory.size() - 1) != _T("End"))
        {
            CommandMemory.push_back(_T("End"));
        }
        MainSubProgramSeparate();
        DecideInit();
        wakeEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        g_pThread = AfxBeginThread(Thread, (LPVOID)this);
        return TRUE;
    }
    else {
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
            g_pThread->ResumeThread();//啟動線程
            m_Action.g_bIsStop = TRUE;
            RunData.RunStatus = 1;//狀態設為運作中
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
            RunData.RunStatus = 1;//狀態設為運作中
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
    if (g_pThread && RunData.RunStatus == 1) {
        g_pThread->SuspendThread();
        RunData.RunStatus = 2;//狀態改變成暫停中
        m_Action.g_bIsPause = TRUE;
        return TRUE;
    }
    else {
        return FALSE;
    }
}
/*繼續*/
BOOL COrder::Continue()
{
    if (g_pThread)//判斷是否有在運作
    {
        if (SuspendThread(g_pThread) != GetLastError() && RunData.RunStatus == 2)
        {
            g_pThread->ResumeThread();//啟動線程
            RunData.RunStatus = 1;//狀態改變成運作中
            m_Action.g_bIsPause = FALSE;
            return TRUE;
        }
    }
    else
    {
        return FALSE;
    }
}
/*主執行緒*/
UINT COrder::Thread(LPVOID pParam)
{
    ((COrder*)pParam)->RunData.RunStatus = 1;//狀態改變成運作中
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
                ((COrder*)pParam)->Program.LabelName = _T("");
                ((COrder*)pParam)->Program.LabelCount = 0;
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
            }
        }
        else
        {
            ((COrder*)pParam)->Commanding = ((COrder*)pParam)->Command.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)).at(((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)));
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
    //((COrder*)pParam)->m_Action.DecideInitializationMachine(20000,1000,7,0);
    ((COrder*)pParam)->m_Action.g_bIsStop = FALSE;
    ((COrder*)pParam)->DecideClear();
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
    /************************************************************程序**********************************************************/
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
        ACSData Buff;
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
    if (CommandResolve(Command, 0) == L"Loop")
    {
        if (_ttol(CommandResolve(Command, 2)))
        {
            if (!((COrder*)pParam)->RepeatData.LoopAddressNum.size())
            {
                ((COrder*)pParam)->RepeatData.LoopAddressNum.push_back(((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)));
                ((COrder*)pParam)->RepeatData.LoopCount.push_back(_ttol(CommandResolve(Command, 2)));
                ((COrder*)pParam)->Program.LabelName = _T("Label,") + CommandResolve(Command, 1);
            }
            else
            {
                UINT LoopAddressNumSize = ((COrder*)pParam)->RepeatData.LoopAddressNum.size();
                for (int i = 0; i < LoopAddressNumSize; i++)
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
                            ((COrder*)pParam)->RepeatData.LoopAddressNum.push_back(((COrder*)pParam)->RunData.RunCount.at(((COrder*)pParam)->RunData.MSChange.at(((COrder*)pParam)->RunData.StackingCount)));
                            ((COrder*)pParam)->RepeatData.LoopCount.push_back(_ttol(CommandResolve(Command, 2)));
                            ((COrder*)pParam)->Program.LabelName = _T("Label,") + CommandResolve(Command, 1);
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
    /************************************************************動作**********************************************************/
#ifdef MOVE
    if (CommandResolve(Command, 0) == L"Dot")
    {
        ((COrder*)pParam)->Time++;
        ModifyPointOffSet(pParam, Command);
        LineGotoActionJudge(pParam);
        ((COrder*)pParam)->m_Action.DecidePointGlue(
            _ttoi(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X,
            _ttoi(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
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
        ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Status = TRUE;
        ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).X = _ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X;
        ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Y = _ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y;
        ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Z = _ttol(CommandResolve(Command, 3)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z;
        ((COrder*)pParam)->RunData.ActionStatus.at(((COrder*)pParam)->Program.SubroutinCount) = 1;
    }
    if (CommandResolve(Command, 0) == L"LinePassing")
    {
        ((COrder*)pParam)->Time++;
        ModifyPointOffSet(pParam, Command);
        if (((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Status)
        {
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
                        _ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X,
                        _ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
                        ((COrder*)pParam)->LineSpeedSet.EndSpeed, 1000);
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
                        _ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X,
                        _ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
                        _ttol(CommandResolve(Command, 3)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z,
                        ((COrder*)pParam)->DispenseLineSet.NodeTime, ((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, 6000);
                }
                else
                {
                    //呼叫LS-LP
                    ((COrder*)pParam)->m_Action.DecideLineSToP(((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Y, ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Z,
                        _ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X,
                        _ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
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
                        _ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X,
                        _ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
                        ((COrder*)pParam)->LineSpeedSet.EndSpeed, 1000);
                }
                else if (((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Status)
                {
                    //呼叫Circle
                    ((COrder*)pParam)->m_Action.DecideCircle(((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Y, ((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount).Y,
                        ((COrder*)pParam)->LineSpeedSet.EndSpeed, 1000);
                    //呼叫LP
                    ((COrder*)pParam)->m_Action.DecideLineMidMove(
                        _ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X,
                        _ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
                        _ttol(CommandResolve(Command, 3)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z,
                        ((COrder*)pParam)->DispenseLineSet.NodeTime, ((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, 6000);
                }
                else
                {
                    //呼叫LP
                    ((COrder*)pParam)->m_Action.DecideLineMidMove(
                        _ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X,
                        _ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
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
            if (((COrder*)pParam)->RunData.ActionStatus.at(((COrder*)pParam)->Program.SubroutinCount) == 1)
            {
                if (((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Status)
                {
                    //呼叫一個LS
                    ((COrder*)pParam)->m_Action.DecideLineStartMove(((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Y, ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Z,
                        ((COrder*)pParam)->DispenseLineSet.BeforeMoveDelay, ((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, 6000);
                    //再呼叫一個ARC-LE
                    ((COrder*)pParam)->m_Action.DecideArcleToEnd(((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Y, 
                        _ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X,
                        _ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
                        ((COrder*)pParam)->DispenseLineSet.StayTime, ((COrder*)pParam)->DispenseLineSet.ShutdownDistance, ((COrder*)pParam)->DispenseLineSet.ShutdownDelay,
                        ((COrder*)pParam)->ZSet.ZBackHeight, ((COrder*)pParam)->ZSet.ZBackType,
                        ((COrder*)pParam)->DispenseLineEnd.HighSpeed, ((COrder*)pParam)->DispenseLineEnd.Width, ((COrder*)pParam)->DispenseLineEnd.Height, ((COrder*)pParam)->DispenseLineEnd.LowSpeed, ((COrder*)pParam)->DispenseLineEnd.Type,
                        ((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, 1000);
                }
                else if(((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Status)
                {
                    //呼叫LS
                    ((COrder*)pParam)->m_Action.DecideLineStartMove(((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Y, ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Z,
                        ((COrder*)pParam)->DispenseLineSet.BeforeMoveDelay, ((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, 6000);
                    //呼叫Circle-LE
                    ((COrder*)pParam)->m_Action.DecideCircleToEnd(((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Y, ((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount).Y, 
                        _ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X,
                        _ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
                        ((COrder*)pParam)->DispenseLineSet.StayTime, ((COrder*)pParam)->DispenseLineSet.ShutdownDistance, ((COrder*)pParam)->DispenseLineSet.ShutdownDelay,
                        ((COrder*)pParam)->ZSet.ZBackHeight, ((COrder*)pParam)->ZSet.ZBackType,
                        ((COrder*)pParam)->DispenseLineEnd.HighSpeed, ((COrder*)pParam)->DispenseLineEnd.Width, ((COrder*)pParam)->DispenseLineEnd.Height, ((COrder*)pParam)->DispenseLineEnd.LowSpeed, ((COrder*)pParam)->DispenseLineEnd.Type,
                        ((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, 1000);
                }
                else
                {
                    //呼叫LS-LE
                    ((COrder*)pParam)->m_Action.DecideLineSToE(((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Y, ((COrder*)pParam)->StartData.at(((COrder*)pParam)->Program.SubroutinCount).Z,
                        _ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X,
                        _ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
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
                        _ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X,
                        _ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
                        ((COrder*)pParam)->DispenseLineSet.StayTime, ((COrder*)pParam)->DispenseLineSet.ShutdownDistance, ((COrder*)pParam)->DispenseLineSet.ShutdownDelay,
                        ((COrder*)pParam)->ZSet.ZBackHeight, ((COrder*)pParam)->ZSet.ZBackType,
                        ((COrder*)pParam)->DispenseLineEnd.HighSpeed, ((COrder*)pParam)->DispenseLineEnd.Width, ((COrder*)pParam)->DispenseLineEnd.Height, ((COrder*)pParam)->DispenseLineEnd.LowSpeed, ((COrder*)pParam)->DispenseLineEnd.Type,
                        ((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, 1000);
                }
                else if (((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Status)
                {
                    //呼叫Circle-LE
                    ((COrder*)pParam)->m_Action.DecideCircleToEnd(((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Y, ((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount).Y, 
                        _ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X,
                        _ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
                        ((COrder*)pParam)->DispenseLineSet.StayTime, ((COrder*)pParam)->DispenseLineSet.ShutdownDistance, ((COrder*)pParam)->DispenseLineSet.ShutdownDelay,
                        ((COrder*)pParam)->ZSet.ZBackHeight, ((COrder*)pParam)->ZSet.ZBackType,
                        ((COrder*)pParam)->DispenseLineEnd.HighSpeed, ((COrder*)pParam)->DispenseLineEnd.Width, ((COrder*)pParam)->DispenseLineEnd.Height, ((COrder*)pParam)->DispenseLineEnd.LowSpeed, ((COrder*)pParam)->DispenseLineEnd.Type,
                        ((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, 1000);
                }
                else
                {
                    //呼叫LE
                    ((COrder*)pParam)->m_Action.DecideLineEndMove(
                        _ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X,
                        _ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
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
        ((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Status = TRUE;
        ((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).X = _ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X;
        ((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Y = _ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y;
    }
    if (CommandResolve(Command, 0) == L"CirclePoint")
    {
        ModifyPointOffSet(pParam, Command);
        if (((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Status == TRUE)
        {
            ((COrder*)pParam)->ArcData.at(((COrder*)pParam)->Program.SubroutinCount).Status = FALSE;
        }
        ((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Status = TRUE;
        ((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).X = _ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X;
        ((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Y = _ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y;
        ((COrder*)pParam)->CircleData1.at(((COrder*)pParam)->Program.SubroutinCount).Z = _ttol(CommandResolve(Command, 3)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z;
        ((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount).X = _ttol(CommandResolve(Command, 4)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X;
        ((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount).Y = _ttol(CommandResolve(Command, 5)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y;
        ((COrder*)pParam)->CircleData2.at(((COrder*)pParam)->Program.SubroutinCount).Z = _ttol(CommandResolve(Command, 6)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z;
    }
    if (CommandResolve(Command, 0) == L"GoHome")
    {
        LineGotoActionJudge(pParam);
        if (((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Status)//已經有offset修正
        {
            ((COrder*)pParam)->m_Action.DecideInitializationMachine(20000, 1000, 7, 0);
        }
        else
        {
            ((COrder*)pParam)->m_Action.DecideInitializationMachine(20000, 1000, 7, 0);
            ((COrder*)pParam)->m_Action.DecideVirtualPoint(((COrder*)pParam)->Program.SubroutinePointStack.at(((COrder*)pParam)->Program.SubroutinCount).X, ((COrder*)pParam)->Program.SubroutinePointStack.at(((COrder*)pParam)->Program.SubroutinCount).Y, ((COrder*)pParam)->Program.SubroutinePointStack.at(((COrder*)pParam)->Program.SubroutinCount).Z,
                ((COrder*)pParam)->DotSpeedSet.EndSpeed, ((COrder*)pParam)->DotSpeedSet.AccSpeed, 6000);
        }
    }
    if (CommandResolve(Command, 0) == L"VirtualPoint")
    {
        ModifyPointOffSet(pParam, Command);
        LineGotoActionJudge(pParam);
        ((COrder*)pParam)->m_Action.DecideVirtualPoint(
            _ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X,
            _ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
            _ttol(CommandResolve(Command, 3)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z,
            ((COrder*)pParam)->DotSpeedSet.EndSpeed, ((COrder*)pParam)->DotSpeedSet.AccSpeed, 6000);
    }
    if (CommandResolve(Command, 0) == L"WaitPoint")
    {
        ModifyPointOffSet(pParam, Command);
        LineGotoActionJudge(pParam);
        ((COrder*)pParam)->m_Action.DecideWaitPoint(
            _ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X,
            _ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
            _ttol(CommandResolve(Command, 3)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z,
            _ttol(CommandResolve(Command, 4)), ((COrder*)pParam)->DotSpeedSet.EndSpeed, ((COrder*)pParam)->DotSpeedSet.AccSpeed, 6000);
    }
    if (CommandResolve(Command, 0) == L"ParkPoint")
    {
        ModifyPointOffSet(pParam, Command);
        LineGotoActionJudge(pParam);
        ((COrder*)pParam)->m_Action.DecideParkPoint(
            _ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X,
            _ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
            _ttol(CommandResolve(Command, 3)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z,
            ((COrder*)pParam)->GlueData.GlueTime, ((COrder*)pParam)->GlueData.GlueStayTime, ((COrder*)pParam)->DotSpeedSet.EndSpeed, ((COrder*)pParam)->DotSpeedSet.AccSpeed, 6000);
    }
    if (CommandResolve(Command, 0) == L"StopPoint")
    {
        ModifyPointOffSet(pParam, Command);
        LineGotoActionJudge(pParam);
        ((COrder*)pParam)->m_Action.DecideVirtualPoint(
            _ttol(CommandResolve(Command, 1)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X,
            _ttol(CommandResolve(Command, 2)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
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
        ((COrder*)pParam)->m_Action.DecideFill(
            _ttol(CommandResolve(Command, 4)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X,
            _ttol(CommandResolve(Command, 5)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
            _ttol(CommandResolve(Command, 6)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Z,
            _ttol(CommandResolve(Command, 7)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).X,
            _ttol(CommandResolve(Command, 8)) + ((COrder*)pParam)->OffsetData.at(((COrder*)pParam)->Program.SubroutinCount).Y,
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
    /************************************************************參數***********************************************************/
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
    g_pSubroutineThread = NULL;
    return 0;
}
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
    DispenseDotSet.GlueOpenTime = 0;
    DispenseDotSet.GlueCloseTime = 0;
    DispenseDotEnd.RiseDistance = 0;
    DispenseDotEnd.RiseHightSpeed = 0;
    DispenseDotEnd.RiseLowSpeed = 0;
    DispenseLineSet.BeforeMoveDelay = 0;
    DispenseLineSet.BeforeMoveDistance = 0;
    DispenseLineSet.NodeTime = 0;
    DispenseLineSet.ShutdownDelay = 0;
    DispenseLineSet.ShutdownDistance = 0;
    DispenseLineSet.StayTime = 0;
    DispenseLineEnd.Type = 0;
    DispenseLineEnd.HighSpeed = 0;
    DispenseLineEnd.LowSpeed = 0;
    DispenseLineEnd.Width = 0;
    DispenseLineEnd.Height = 0;
    DotSpeedSet.AccSpeed = 0;
    DotSpeedSet.EndSpeed = 0;
    LineSpeedSet.AccSpeed = 0;
    LineSpeedSet.EndSpeed = 0;
    ZSet.ZBackHeight = 0;
    GlueData.GlueTime = 0;
    GlueData.GlueStayTime = 0;
}
void COrder::DecideInit()
{
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
    RunData.RunStatus = 0;//狀態改變成未運行
    RunData.StackingCount = 0;//主副程式計數  
    RunData.SubProgramName = _T("");//副程式判斷標籤
    RunData.ActionStatus.push_back(0);//運動狀態清0
    //運行計數清0
    for (UINT i = 0; i < RunData.RunCount.size(); i++)
    {
        RunData.RunCount.at(i) = 0;
    }
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




