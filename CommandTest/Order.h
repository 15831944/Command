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
    //�{�Ǳ��޵��c(���ҭp�ơB���ҦW�١B�^�l�{�Ǧa�})
    struct Program {
        int LabelCount;
        CString LabelName;
        std::vector<UINT> SubroutineStack;
        std::vector<CPoint> SubroutinePointStack;
    };
    //Z�b��T���c(Z�b�^�ɰ���)
    struct ZSet {
        LONG ZBackHeight;
    };
    //Arc And Circle And Start�B�@���c (���A�B����X�B����Y)
    struct AACData {
        BOOL Status;
        LONG X;
        LONG Y;
        LONG Z;
    };
    //�X�����c (�X���ɶ��B�_�����d�ɶ�)
    struct GlueData {
        LONG GlueTime;
        LONG GlueStayTime;
    };
    //�B��ѼƵ��c(�Ƶ{���W�l�B�B�檬�A�B�B��p�ơB����D�Ƶ{�ǡB�D�Ƶ{�ǰ��|�p�ơB�ʧ@���A)  
    //�B�@���A(0:���B�@ 1:�B�椤 2:�Ȱ���)
    //�B��p��(�ثe����ĴX�ӫ��O 0:�D�{�� 1-X:�Ƶ{��)
    struct RunData {
        CString SubProgramName;
        UINT RunStatus;
        std::vector<UINT> RunCount;//�p�Ƶ{�ǩR�O
        std::vector<UINT> MSChange; //���ޥثe��Ū���{��
        UINT StackingCount;
        UINT ActionStatus;
    };
    
private:    //�ܼ�
    HANDLE  wakeEvent;
    
private:    //���
    static  UINT    Thread(LPVOID pParam);
    static  UINT    SubroutineThread(LPVOID pParam);
    static  CString CommandResolve(CString Command,UINT Choose);
    void            ParameterDefult();
    void            DecideClear();
    
public:     //�ܼ�
    LONG            Time;
    CAction         m_Action;
    CString         Commanding;
    std::vector<CString> CommandMemory;
    std::vector<CString> CommandSwap;
    std::vector<std::vector<CString>> Command;
    
    

    DispenseDotSet  DispenseDotSet;
    DispenseDotEnd  DispenseDotEnd;
    DispenseLineSet DispenseLineSet;
    DispenseLineEnd DispenseLineEnd;
    Speed           DotSpeedSet,LineSpeedSet;
    Program         Program;
    ZSet            ZSet;
    AACData         ArcData,CircleData,StartData;
    GlueData        GlueData;
    RunData         RunData;
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
    void            MainSubroutineSeparate();
protected:
	DECLARE_MESSAGE_MAP()
};


