// stdafx.h : �i�b�����Y�ɤ��]�t�зǪ��t�� Include �ɡA
// �άO�g�`�ϥΫo�ܤ��ܧ�
// �M�ױM�� Include �ɮ�

#pragma once
#define _CRT_NON_CONFORMING_SWPRINTFS
#define _CRT_SECURE_NO_WARNINGS
#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // �q Windows ���Y�ư����`�ϥΪ�����
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // ���T�w�q������ CString �غc�禡

// ���� MFC ���ä@�Ǳ`���Υi����ĵ�i�T�����\��
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC �֤߻P�зǤ���
#include <afxext.h>         // MFC �X�R�\��
#include <afxdisp.h>        // MFC Automation ���O

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC �䴩�� Internet Explorer 4 �q�α��
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // MFC �䴩�� Windows �q�α��
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcontrolbars.h>     // �\��ϩM����C�� MFC �䴩

//�w�q�}�ҥ\��϶�
#define MOVE
//#define VI                                                     
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

//�禡�϶�
CString GetCurrentPath(CString Folder);
BOOL FileExist(LPCWSTR FilePathName);

//MFC���ذ϶�
#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif
//�۩w�q����
#define WM_MY_MESSAGE(x,y) (x+WM_USER+y)
//�w�q�Ȱ϶�
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
#define UseCoordinateMode 1
#define dCCDOffsetW 0.0
#define dFillAreaW 0.0



