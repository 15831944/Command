#pragma once
#include <vector>
#include "Action.h"
// COrder
static CWinThread* g_pThread = NULL;
static CWinThread* g_pSubroutineThread = NULL;
class COrder : public CWnd
{
    DECLARE_DYNAMIC(COrder)
private:
    /************************************************************�B�ʰѼƵ��c*******************************************************/
    //�y�е��c (���A�B����X�B����Y�B����Z)
    struct CoordinateData {
        BOOL Status;
        LONG X;
        LONG Y;
        LONG Z;
    };
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
    //Z�b��T���c(Z�b�^�ɰ��סBZ�b�^�ɫ��A(0����1�۹�))
    struct ZSet {
        LONG ZBackHeight;
        BOOL ZBackType;
    };
    //�ƽ����c (���n��m�B�۰ʱƽ��}���B���ݮɶ��B�X���ɶ��B�_�����d�ɶ�)
    struct GlueData {
        CoordinateData  ParkPositionData;
        BOOL GlueAuto;
        LONG GlueWaitTime;
        LONG GlueTime;
        LONG GlueStayTime;
    };
    //��l�Ƶ��c(�I���]�m���c�B�I���������c�B�I���t�סB�u�q�]�m���c�B�u�q�������c�B�u�q�t�סBZ�b���c�B�ƽ����c)
    struct Default {
        DispenseDotSet  DispenseDotSet;
        DispenseDotEnd  DispenseDotEnd;
        Speed           DotSpeedSet;
        DispenseLineSet DispenseLineSet;
        DispenseLineEnd DispenseLineEnd;
        Speed           LineSpeedSet;
        ZSet            ZSet;
        GlueData        GlueData;
    };
    /************************************************************�v���ѼƵ��c*******************************************************/
    //�v������I���c(����I�B��J�Z���BLoadModel�s���B�s��Model���w�B���᰾���qX�B���᰾���qY)
    struct Vision {
        CoordinateData Point;
        LONG FocusDistance;
        UINT LoadModelNum;
        void* MilModel; 
        DOUBLE OffsetX;
        DOUBLE OffsetY;
    };
    //�v���ץ��p�⵲�c(����I�B����I�᰾���qX�B����I�᰾���qY�B����I�᰾������)
    struct VisionOffset {
        CoordinateData Contraposition;
        DOUBLE OffsetX;
        DOUBLE OffsetY;
        DOUBLE Angle;
    };
    //�v��LodaModel�]�m���c(��T�סB�M��t�סB�ǰt�ȡB�j�M�d��e�סB�j�M�d�򰪫סB�j�M�d��_�l���סB�j�M�d�򵲧�����)
    struct VisionSet {
        BOOL Accuracy;
        BOOL Speed;
        BYTE Score;
        int width;
        int height;
        int Startangle;
        int Endangle;
    };
    //�v��LoadModel�ɮצ�m���c(���|�B�ɮ��`�ơB�ɮצW��)
    struct VisionFile {
        LPTSTR ModelPath;
        UINT ModelCount;
        std::vector<CString> AllModelName;
    };
    //��l�Ƶ��c
    struct VisionDefault{
        VisionSet VisionSet;
        VisionFile VisionFile;
    };
    /************************************************************�{�ǰѼƵ��c*******************************************************/
	//Label&Subroutine���޵��c(���ҭp�ơB���ҦW�١B�l�{�ǭp�ơB�l�{�Ǫ��A�B�l�{�Ǧa�}���|�B�l�{�Ǯy�а��|)
	struct Program {
		int LabelCount;
		CString LabelName;
		UINT SubroutinCount;
		BOOL CallSubroutineStatus;
		std::vector<UINT> SubroutineStack;
		std::vector<CoordinateData> SubroutinePointStack;
	};
	//�B�浲�c(�Ƶ{���W�١B�B�檬�A�B�B��p�ơB����D�Ƶ{�ǡB�D�Ƶ{�ǰ��|�p�ơB�ʧ@���A)  
	/*
    *�B�@���A(0:���B�@ 1:�B�椤 2:�Ȱ���)
	*�B��p��(�ثe����ĴX�ӫ��O 0:�D�{�� 1-X:�Ƶ{��)
    *�ثe�ҭnŪ�������|�p��
	*�ʧ@���A(0:�u�q���槹�� 1:�u�q�|�����槹�� 2:Goto�L���u�q)
    */
	struct RunData {
		CString SubProgramName;
		UINT RunStatus;
		std::vector<UINT> RunCount;//�p�Ƶ{�ǩR�O
		std::vector<UINT> MSChange; //���ޥثe��Ū���{��
        UINT StackingCount;
		std::vector<UINT> ActionStatus;
	};
    //Step&Loop���޵��c(�`���}���B�`���a�}�����B�`���p�ơB�B�J�}���B�B�J�a�}�����B�B�J��loffsetX�����B�B�J��loffsetY�����B�B�J�p��X�B�B�J�p��Y)
    /*
    *�`���}��&�B�J�}��(�ΨӧP�O�S�������Үɪ��p)
    */
    struct RepeatData {
        BOOL LoopSwitch;
        std::vector<UINT> LoopAddressNum;
        std::vector<UINT> LoopCount; 
        BOOL StepRepeatSwitch;
        std::vector<UINT> StepRepeatNum;
        std::vector<UINT> StepRepeatInitOffsetX;
        std::vector<UINT> StepRepeatInitOffsetY;
        std::vector<UINT> StepRepeatCountX;
        std::vector<UINT> StepRepeatCountY;
    };
private:    //�ܼ�
	HANDLE          wakeEvent;
    CAction         m_Action;
    CoordinateData  InitData;
    CString         Commanding;
    std::vector<CString> CommandSwap;
    std::vector<std::vector<CString>> Command;
    RepeatData      RepeatData;
    Program         Program;
    std::vector<CoordinateData> ArcData, CircleData1, CircleData2, StartData, OffsetData;

private:    //���
	static  UINT    Thread(LPVOID pParam);
	static  UINT    SubroutineThread(LPVOID pParam);
	static  void    LineGotoActionJudge(LPVOID pParam);
	static  void    ModifyPointOffSet(LPVOID pParam, CString XYZPoint);
    static  void    VisionModify(LPVOID pParam);
	static  CString CommandResolve(CString Command,UINT Choose);
	void            ParameterDefult();
	void            DecideInit();
	void            DecideClear();
	void            MainSubProgramSeparate();
    
	
public:     //�ܼ�
    //�ʧ@�`��
	LONG            Time;
    //�{�ǰ}�C
	std::vector<CString> CommandMemory;
    //�Ѽƭ�
    Default         Default;
    DispenseDotSet  DispenseDotSet;
    DispenseDotEnd  DispenseDotEnd;
    DispenseLineSet DispenseLineSet;
    DispenseLineEnd DispenseLineEnd;
    Speed           DotSpeedSet, LineSpeedSet;
    ZSet            ZSet;
    GlueData        GlueData;
    //�B��{�Ǹ��
    RunData         RunData;
    //�v���Ѽ� 
    Vision          FindMark, FiducialMark1,FiducialMark2;
    VisionSet       VisionSet;
    VisionFile      VisionFile;
    VisionOffset    VisionOffset;
    VisionDefault   VisionDefault;
    BOOL            ListAllFileInDirectory(LPTSTR szPath, LPTSTR szName);
    //�v���ץ���u�@�y��
    CoordinateData  FinalWorkCoordinateData;
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


