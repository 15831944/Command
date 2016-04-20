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
    Program.LabelName = _T("");
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
}
COrder::~COrder()
{
}
BEGIN_MESSAGE_MAP(COrder, CWnd)
END_MESSAGE_MAP()
// COrder 訊息處理常式
/*開始*/
BOOL COrder::Run()
{
    if (!g_pThread) {
        Commanding = _T("Start");
        DecideClear();
        Command.push_back(_T("End"));
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
            //SetEvent(wakeEvent);
            //WaitForSingleObject(g_pThread->m_hThread, INFINITE);        //等待线程安全返回
            DecideClear();
            CloseHandle(wakeEvent);
            return TRUE;
        }
        else
        {
            m_Action.g_bIsStop = TRUE;
            DecideClear();
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
        if (((COrder*)pParam)->Program.LabelName != _T(""))
        {
            ((COrder*)pParam)->Program.LabelCount++;
            if (((COrder*)pParam)->Program.LabelName == ((COrder*)pParam)->Command.at(((COrder*)pParam)->RunData.RunCount)
                || ((COrder*)pParam)->Program.LabelCount == ((COrder*)pParam)->Command.size())
            {
                ((COrder*)pParam)->Program.LabelName = _T("");
                ((COrder*)pParam)->Program.LabelCount = 0;
            }
        }
        else
        {
            ((COrder*)pParam)->Commanding = ((COrder*)pParam)->Command.at(((COrder*)pParam)->RunData.RunCount);
            g_pSubroutineThread = AfxBeginThread(((COrder*)pParam)->SubroutineThread, pParam);
            while (g_pSubroutineThread) {
                Sleep(1);//while 程式負載問題 無限迴圈，並讓 CPU 休息一下
            }
        }
        if (((COrder*)pParam)->RunData.RunCount == ((COrder*)pParam)->Command.size()-1)
        {
            ((COrder*)pParam)->RunData.RunCount = 0;
        }
        else
        {
            ((COrder*)pParam)->RunData.RunCount++;
        }   
    }
    ((COrder*)pParam)->m_Action.g_bIsStop = FALSE;
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
        if (!_ttoi(CommandResolve(Command, 1)))
        {
            ((COrder*)pParam)->RunData.RunCount = _ttoi(CommandResolve(Command, 1)) - 2;
        }
    }
    /************************************************************動作**********************************************************/
#ifdef MOVE
    if (CommandResolve(Command, 0) == L"Dot")
    {
        ((COrder*)pParam)->Time++;
        ((COrder*)pParam)->m_Action.DecidePointGlue(_ttoi(CommandResolve(Command, 1)), _ttoi(CommandResolve(Command, 2)), _ttoi(CommandResolve(Command, 3)),
           ((COrder*)pParam)->DispenseDotSet.GlueOpenTime, ((COrder*)pParam)->DispenseDotSet.GlueCloseTime,
           ((COrder*)pParam)->ZSet.ZBackHeight, ((COrder*)pParam)->DispenseDotEnd.RiseDistance, ((COrder*)pParam)->DispenseDotEnd.RiseHightSpeed, ((COrder*)pParam)->DispenseDotEnd.RiseLowSpeed,
           ((COrder*)pParam)->DotSpeedSet.EndSpeed, ((COrder*)pParam)->DotSpeedSet.AccSpeed,6000);
    }
    if (CommandResolve(Command, 0) == L"LineStart")
    {
        ((COrder*)pParam)->Time++;
        ((COrder*)pParam)->m_Action.DecideLineStartMove(_ttol(CommandResolve(Command, 1)), _ttol(CommandResolve(Command, 2)), _ttol(CommandResolve(Command, 3)),
            ((COrder*)pParam)->DispenseLineSet.BeforeMoveDelay, ((COrder*)pParam)->DispenseLineSet.BeforeMoveDistance,
            ((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, 6000);
    }
    if (CommandResolve(Command, 0) == L"LinePassing")
    {
        ((COrder*)pParam)->Time++;
        if (((COrder*)pParam)->ArcData.Status)
        {
            ((COrder*)pParam)->m_Action.DecideArc(((COrder*)pParam)->ArcData.X, ((COrder*)pParam)->ArcData.Y, _ttol(CommandResolve(Command, 1)), _ttol(CommandResolve(Command, 2)),
                ((COrder*)pParam)->LineSpeedSet.EndSpeed, 1000);
            ((COrder*)pParam)->ArcData.Status = FALSE;
        }
        else
        {
            ((COrder*)pParam)->m_Action.DecideLineMidMove(_ttol(CommandResolve(Command, 1)), _ttol(CommandResolve(Command, 2)), _ttol(CommandResolve(Command, 3)),
                ((COrder*)pParam)->DispenseLineSet.NodeTime, ((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, 6000);
        }  
    }
    if (CommandResolve(Command, 0) == L"LineEnd")
    {
        ((COrder*)pParam)->Time++;
        if (((COrder*)pParam)->ArcData.Status)
        {
            ((COrder*)pParam)->m_Action.DecideArc(((COrder*)pParam)->ArcData.X, ((COrder*)pParam)->ArcData.Y, _ttol(CommandResolve(Command, 1)), _ttol(CommandResolve(Command, 2)),
                ((COrder*)pParam)->LineSpeedSet.EndSpeed, 1000);
            ((COrder*)pParam)->ArcData.Status = FALSE;
        }
        ((COrder*)pParam)->m_Action.DecideLineEndMove(_ttol(CommandResolve(Command, 1)), _ttol(CommandResolve(Command, 2)), _ttol(CommandResolve(Command, 3)),
            ((COrder*)pParam)->DispenseLineSet.StayTime, ((COrder*)pParam)->DispenseLineSet.ShutdownDistance, ((COrder*)pParam)->DispenseLineSet.ShutdownDelay,
            ((COrder*)pParam)->ZSet.ZBackHeight, ((COrder*)pParam)->DispenseLineEnd.HighSpeed, ((COrder*)pParam)->DispenseLineEnd.Width, ((COrder*)pParam)->DispenseLineEnd.Height,
            1000/*((COrder*)pParam)->DispenseLineEnd.LowSpeed*/, ((COrder*)pParam)->DispenseLineEnd.Type,
            ((COrder*)pParam)->LineSpeedSet.EndSpeed, ((COrder*)pParam)->LineSpeedSet.AccSpeed, 6000);
    }
    if (CommandResolve(Command, 0) == L"ArcPoint")
    {
        ((COrder*)pParam)->ArcData.Status = TRUE;
        ((COrder*)pParam)->ArcData.X = _ttol(CommandResolve(Command, 1));
        ((COrder*)pParam)->ArcData.Y = _ttol(CommandResolve(Command, 2));
    }
    if (CommandResolve(Command, 0) == L"HomePoint")
    {
        ((COrder*)pParam)->m_Action.DecideInitializationMachine(20000,1000,7,0);
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
    if (CommandResolve(Command, 0) == L"CirclePointOne")
    {
        ((COrder*)pParam)->CircleData.Status = TRUE;
        ((COrder*)pParam)->CircleData.X = _ttol(CommandResolve(Command, 1));
        ((COrder*)pParam)->CircleData.Y = _ttol(CommandResolve(Command, 2));
    }
    if (CommandResolve(Command, 0) == L"CirclePointTwo")
    {
        if (((COrder*)pParam)->CircleData.Status)
        {
            ((COrder*)pParam)->m_Action.DecideCircle(((COrder*)pParam)->CircleData.X, ((COrder*)pParam)->CircleData.Y, _ttol(CommandResolve(Command, 1)), _ttol(CommandResolve(Command, 2)),
                ((COrder*)pParam)->LineSpeedSet.EndSpeed, 1000);
            ((COrder*)pParam)->CircleData.Status = FALSE;
        }
    }
    if (CommandResolve(Command, 0) == L"VirtualPoint")
    {
        ((COrder*)pParam)->m_Action.DecideVirtualPoint(_ttol(CommandResolve(Command, 1)), _ttol(CommandResolve(Command, 2)), _ttol(CommandResolve(Command, 3)),
            ((COrder*)pParam)->DotSpeedSet.EndSpeed, ((COrder*)pParam)->DotSpeedSet.AccSpeed, 6000);
    }    
    if (CommandResolve(Command, 0) == L"WaitPoint")
    {
        ((COrder*)pParam)->m_Action.DecideWaitPoint(_ttol(CommandResolve(Command, 1)), _ttol(CommandResolve(Command, 2)), _ttol(CommandResolve(Command, 3)),
            _ttol(CommandResolve(Command, 4)), ((COrder*)pParam)->DotSpeedSet.EndSpeed, ((COrder*)pParam)->DotSpeedSet.AccSpeed, 6000);
    }
    if (CommandResolve(Command, 0) == L"ParkPoint")
    {
        ((COrder*)pParam)->m_Action.DecideParkPoint(_ttol(CommandResolve(Command, 1)), _ttol(CommandResolve(Command, 2)), _ttol(CommandResolve(Command, 3)),
            ((COrder*)pParam)->GlueData.GlueTime, ((COrder*)pParam)->GlueData.GlueStayTime, ((COrder*)pParam)->DotSpeedSet.EndSpeed, ((COrder*)pParam)->DotSpeedSet.AccSpeed, 6000);
    }
    if (CommandResolve(Command, 0) == L"StopPoint")
    {
        ((COrder*)pParam)->m_Action.DecideVirtualPoint(_ttol(CommandResolve(Command, 1)), _ttol(CommandResolve(Command, 2)), _ttol(CommandResolve(Command, 3)),
            ((COrder*)pParam)->DotSpeedSet.EndSpeed, ((COrder*)pParam)->DotSpeedSet.AccSpeed, 6000);
        ((COrder*)pParam)->Pause();
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
    if (CommandResolve(Command, 0) == L"LineSpeed")
    {
        ((COrder*)pParam)->LineSpeedSet.EndSpeed = _ttol(CommandResolve(Command, 1));
    }
    if (CommandResolve(Command, 0) == L"ZGoBack")
    {
        ((COrder*)pParam)->ZSet.ZBackHeight = _ttol(CommandResolve(Command, 1));
    }
    if (CommandResolve(Command, 0) == L"DispenseAcc") 
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
/*判斷指標歸零*/
void COrder::DecideClear() {
    ArcData.Status = FALSE;
    CircleData.Status = FALSE;
    RunData.RunCount = 0;
    RunData.RunStatus = 0;//狀態改變成未運行
    Program.LabelCount = 0;
    Program.LabelName = _T("");
    Time = 0;
}




