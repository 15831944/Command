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
        RunCount = 0;
        Time = 0;
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
            CloseHandle(wakeEvent);
            return TRUE;
        }
        else
        {
            m_Action.g_bIsStop = TRUE;
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
    if (g_pThread) {
        g_pThread->SuspendThread();
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
        if (SuspendThread(g_pThread) != GetLastError())
        {
            g_pThread->ResumeThread();//啟動線程
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
    while ((!((COrder*)pParam)->m_Action.g_bIsStop) && ((COrder*)pParam)->Commanding != _T("End")) {
        if (((COrder*)pParam)->Program.LabelName != _T(""))
        {
            if (((COrder*)pParam)->Program.LabelName == ((COrder*)pParam)->Command.at(((COrder*)pParam)->RunCount))
            {
                ((COrder*)pParam)->Program.LabelName = _T("");
            }
        }
        else
        {
            ((COrder*)pParam)->Commanding = ((COrder*)pParam)->Command.at(((COrder*)pParam)->RunCount);
            g_pSubroutineThread = AfxBeginThread(((COrder*)pParam)->SubroutineThread, pParam);
            while (g_pSubroutineThread) {
                Sleep(1);//while 程式負載問題 無限迴圈，並讓 CPU 休息一下
            }
        }
        if (((COrder*)pParam)->RunCount == ((COrder*)pParam)->Command.size()-1)
        {
            ((COrder*)pParam)->RunCount = 0;
        }
        else
        {
            ((COrder*)pParam)->RunCount++;
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
    if (CommandResolve(Command, 0) == L"GotoLabel") 
    {
        ((COrder*)pParam)->Program.LabelName = _T("Label,") + CommandResolve(Command, 1);
    }
    if (CommandResolve(Command, 0) == L"GotoAddress") 
    {
        ((COrder*)pParam)->RunCount = _ttoi(CommandResolve(Command, 1)) - 2;
    }
#ifdef MOVE
    if (CommandResolve(Command, 0) == L"DispenseDot")
    {
       ((COrder*)pParam)->Time++;
       ((COrder*)pParam)->m_Action.DecidePointGlue(_ttoi(CommandResolve(Command, 1)), _ttoi(CommandResolve(Command, 2)), _ttoi(CommandResolve(Command, 3)),
           ((COrder*)pParam)->DispenseDotSet.GlueOpenTime, ((COrder*)pParam)->DispenseDotSet.GlueCloseTime,
           20000, ((COrder*)pParam)->DispenseDotEnd.RiseDistance, ((COrder*)pParam)->DispenseDotEnd.RiseHightSpeed, ((COrder*)pParam)->DispenseDotEnd.RiseLowSpeed,
           ((COrder*)pParam)->DotSpeedSet.EndSpeed, ((COrder*)pParam)->DotSpeedSet.AccSpeed,6000);
    }
#endif
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
        ((COrder*)pParam)->DotSpeedSet.AccSpeed = _ttol(CommandResolve(Command, 2));
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




