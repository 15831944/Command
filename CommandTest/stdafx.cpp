
// stdafx.cpp : 僅包含標準 Include 檔的原始程式檔
// CommandTest.pch 會成為先行編譯標頭檔
// stdafx.obj 會包含先行編譯類型資訊

#include "stdafx.h"
#ifdef PRINTF
HHOOK g_hGetMessage = NULL;
HWND g_Consolehwnd;
LRESULT CALLBACK GetMsgProc(
    int    code,
    WPARAM wParam,
    LPARAM lParam
){
    if (code == HC_ACTION)
    {
        MSG * pMsg = (MSG*)lParam;
        switch (pMsg->message)
        {
        case WM_RBUTTONUP:
            //SendMessage(g_hwnd, WM_HOOK_MESSAGE, wParam, lParam);
            _cwprintf(L"WM_RBUTTONUP");
            break;
        case WM_RBUTTONDOWN:
            //SendMessage(g_hwnd, WM_HOOK_MESSAGE, wParam, lParam);
            _cwprintf(L"WM_RBUTTONDOWN");
            break;
        }
    }
    return CallNextHookEx(g_hGetMessage, code, wParam, lParam);
}
void InitConsoleWindow()
{
    /*AllocConsole();
    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    int hCrt = _open_osfhandle((long)handle, _O_TEXT);
    FILE * hf = _fdopen(hCrt, "w");
    *stdout = *hf;*/

    int nCrt = 0;
    FILE* fp;
    AllocConsole();
    nCrt = _open_osfhandle((long)GetStdHandle(STD_OUTPUT_HANDLE), _O_TEXT);
    fp = _fdopen(nCrt, "w");
    *stdout = *fp;
    setvbuf(stdout, NULL, _IONBF, 0);

    //刪除Menu 關閉按鈕 方式1
    //char oldTitle[100];
    //char newTitle[100];
    //GetConsoleTitleA(oldTitle, 100);
    //sprintf_s(newTitle, "%d-%d", GetTickCount(), GetCurrentProcessId());
    //SetConsoleTitleA(newTitle);
    //Sleep(100);
    //g_Consolehwnd = FindWindowA(NULL, newTitle);
    //SetConsoleTitleA(oldTitle);
    //if (g_Consolehwnd)
    //{
    //    HMENU h_Menu = GetSystemMenu(g_Consolehwnd, FALSE);
    //    if (h_Menu)
    //    {
    //        RemoveMenu(h_Menu, 0xF060, 0);
    //        //return ::EnableMenuItem( h_Menu, 0xF060, FALSE );
    //    }
    //}

    //刪除Menu 關閉按鈕 方式2
    g_Consolehwnd = ::GetConsoleWindow();
    if (g_Consolehwnd != NULL)
    {
        HMENU h_Menu = ::GetSystemMenu(g_Consolehwnd, FALSE);           
        if (h_Menu != NULL)
        {
            DeleteMenu(h_Menu, SC_CLOSE, MF_BYCOMMAND);
        }
    }
    
    //g_hGetMessage = SetWindowsHookEx(WH_GETMESSAGE, GetMsgProc, NULL, GetCurrentThreadId());
}
#endif
CString GetCurrentPath(CString Folder)
{
    CString path;
    GetModuleFileName(NULL, path.GetBufferSetLength(MAX_PATH + 1), MAX_PATH);
    path.ReleaseBuffer();
    int pos = path.ReverseFind('\\');
    path = path.Left(pos) + Folder;
    LPTSTR lpszText = new TCHAR[path.GetLength() + 1];
    lstrcpy(lpszText, path);
    return path;
}



