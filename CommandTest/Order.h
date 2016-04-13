#pragma once
#include <vector>
#include "Action.h"
// COrder
static CWinThread* g_pThread = NULL;
static CWinThread* g_pSubroutineThread = NULL;
class COrder : public CWnd
{
	DECLARE_DYNAMIC(COrder)
private:    //把计
    struct Speed {
        LONG AccSpeed;
        LONG EndSpeed;
    };
    struct DispenseDotSet {
        LONG GlueOpenTime;
        LONG GlueCloseTime;
    };
    struct DispenseDotEnd {
        LONG RiseDistance;
        LONG RiseLowSpeed;
        LONG RiseHightSpeed;
    }; 
    struct DispenseLineSet {
        LONG BeforeMoveDelay;
        LONG BeforeMoveDistance;
        LONG StayTime;
        LONG NodeTime;
        LONG ShutdownDistance;
        LONG ShutdownDelay;
    };
    struct DispenseLineEnd {
        LONG Type;
        LONG LowSpeed;
        LONG HighSpeed;
        LONG Height;
        LONG Width;
    };
    struct Program {
        CString LabelName;
        CString SubroutineName;
        int BackSubroutineAddress;
    };

    
private:    //跑计
    HANDLE  wakeEvent;
    
private:    //ㄧ计
    static  UINT Thread(LPVOID pParam);
    static  UINT SubroutineThread(LPVOID pParam);
    static  CString CommandResolve(CString Command,UINT Choose);
public:     //跑计
    
    int             RunCount;
    int             LabelCount;
    LONG            Time;
    CAction         m_Action;
    CString         Commanding;
    std::vector<CString> Command;

    DispenseDotSet  DispenseDotSet;
    DispenseDotEnd  DispenseDotEnd;
    DispenseLineSet DispenseLineSet;
    DispenseLineEnd DispenseLineEnd;
    Speed           DotSpeedSet,LineSpeedSet;
    Program         Program;
public:     //ㄧ计
	COrder();
	virtual ~COrder();
	BOOL    Run();
	BOOL    Stop();
	BOOL    Pause();
    BOOL    Continue();
protected:
	DECLARE_MESSAGE_MAP()
};


