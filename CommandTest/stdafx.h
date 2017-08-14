// stdafx.h : 可在此標頭檔中包含標準的系統 Include 檔，
// 或是經常使用卻很少變更的
// 專案專用 Include 檔案

#pragma once
#define _CRT_NON_CONFORMING_SWPRINTFS
#define _CRT_SECURE_NO_WARNINGS
#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // 從 Windows 標頭排除不常使用的成員
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // 明確定義部分的 CString 建構函式

// 關閉 MFC 隱藏一些常見或可忽略警告訊息的功能
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC 核心與標準元件
#include <afxext.h>         // MFC 擴充功能
#include <afxdisp.h>        // MFC Automation 類別

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC 支援的 Internet Explorer 4 通用控制項
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // MFC 支援的 Windows 通用控制項
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcontrolbars.h>     // 功能區和控制列的 MFC 支援

//定義開啟功能區塊
#define MOVE
#define VI                                                     
//#define LA
#define PRINTF
//#define LOG

#ifdef MOVE
    #include "mcc.h"
#endif
#ifdef VI
    #include "Vision.h"
#endif
#ifdef LA
    #include "Laser.h"
#endif
#ifdef PRINTF
    #include <io.h> 
    #include <fcntl.h>
    #include <conio.h>
    void InitConsoleWindow();
#endif
#ifdef LOG
    BOOL InitFileLog(CString sentence);
#endif

//函式區塊
CString GetCurrentPath(CString Folder);
BOOL FileExist(LPCWSTR FilePathName);

//MFC內建區塊
#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif
//自定義消息
#define WM_MY_MESSAGE(x,y) (x+WM_USER+y)
//定義值區塊
#define TCXYOffsetInit -99999
#define JOGHSpeedW 30000
#define JOGHSpeedA 90000
#define JOGHSpeedI 2000
#define JOGMSpeedW 20000
#define JOGMSpeedA 60000
#define JOGMSpeedI 2000
#define JOGLSpeedW 5000
#define JOGLSpeedA 15000
#define JOGLSpeedI 2000

#define PosXWorkRange 200000
#define NegXWorkRange 0
#define PosYWorkRange 200000
#define NegYWorkRange 0
#define PosWWorkRange 360
#define NegWWorkRange -360

#define CheckEndDlgcode 0x000
#define UseNeedleMode 0
#define UseMachineMode 1
#define UseCoordinateMode 0
#define dCCDOffsetW 0.0
#define dFillAreaW 0.0



