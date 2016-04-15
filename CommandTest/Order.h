#pragma once
#include <vector>
#include "Action.h"
// COrder
static CWinThread* g_pThread = NULL;
static CWinThread* g_pSubroutineThread = NULL;
class COrder : public CWnd
{
	DECLARE_DYNAMIC(COrder)
private:    //�Ѽ�
    //�t�׵��c(�[�t�סB�X�ʳt��)
    struct Speed {
        LONG AccSpeed;
        LONG EndSpeed;
    };
    //�I���]�m���c(�I���}�Үɶ��B�I���������d�ɶ�)
    struct DispenseDotSet {
        LONG GlueOpenTime;
        LONG GlueCloseTime;
    };
    //�I���������c(�Ĥ@���q��ɶZ���B�C�^�{�t�סB���^�{�t��)
    struct DispenseDotEnd {
        LONG RiseDistance;
        LONG RiseLowSpeed;
        LONG RiseHightSpeed;
    }; 
    //�u�q�]�m���c(���ʫe����B���ʫe�Z���B�`�I�ɶ��B���d�ɶ��B�����Z���B��������)
    struct DispenseLineSet {
        LONG BeforeMoveDelay;
        LONG BeforeMoveDistance;
        LONG NodeTime;
        LONG StayTime;
        LONG ShutdownDistance;
        LONG ShutdownDelay;
    };
    //�u�q�������c(��^�����B�C�t�סB���סB���סB���t��)
    struct DispenseLineEnd {
        LONG Type;
        LONG LowSpeed;
        LONG Height;
        LONG Width;
        LONG HighSpeed;
    };
    //�{�Ǳ��޵��c(���ҦW�١B�l�{�ǦW�١B�^�l�{�Ǧa�})
    struct Program {
        CString LabelName;
        CString SubroutineName;
        int BackSubroutineAddress;
    };
    //Z�b��T���c(Z�b�^�ɰ���)
    struct ZSet {
        LONG ZBackHeight;
    };
    //Arc�B�@���c
    struct ArcData {
        BOOL Status;
        LONG X;
        LONG Y;
    };

private:    //�ܼ�
    HANDLE  wakeEvent;
    
private:    //���
    static  UINT Thread(LPVOID pParam);
    static  UINT SubroutineThread(LPVOID pParam);
    static  CString CommandResolve(CString Command,UINT Choose);
    void ParameterDefult();
    void DecideClear();
public:     //�ܼ�
    
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
    ZSet            ZSet;
    ArcData         ArcData;
public:     //���
	COrder();
	virtual ~COrder();
    //�}�l�R�O��Ķ(���\return 1����return 0)
	BOOL    Run();
    //�ߧY����R�O��Ķ(���\return 1����return 0)
	BOOL    Stop();
    //�Ȱ��R�O��Ķ(���\return 1����return 0)
	BOOL    Pause();
    //�~��R�O��Ķ(���\return 1����return 0)
    BOOL    Continue();
protected:
	DECLARE_MESSAGE_MAP()
};


