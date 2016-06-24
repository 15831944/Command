#pragma once
#include <vector>
#include "Action.h"
// COrder
static CWinThread* g_pThread = NULL;
static CWinThread* g_pSubroutineThread = NULL;
static CWinThread* g_pRunLoopThread = NULL;
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
	//�^���I���c(�^�k�t��1�B�^�k�t��2�B�b�ơB����X�B����Y�B����Z�B�`���O�_�_�k�B�O�_���v�����k)
	struct GoHome {
		LONG Speed1;
		LONG Speed2;
		LONG Axis;
		LONG MoveX;
		LONG MoveY;
		LONG MoveZ;
		BOOL PrecycleInitialize;
        BOOL VisionGoHome;
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
		GoHome          GoHome;
	};
	/************************************************************�v���ѼƵ��c*******************************************************/
	//�v������I���c(�аO�d�䪬�A(TRUE = ��� FALSE = �����)�B����I�B��J�Z���BLoadModel�s���B�s��Model���w�B���᰾���qX�B���᰾���qY)
	struct Vision {
		BOOL FindMarkStatus;
		CoordinateData Point;
		LONG FocusDistance;
		LONG LoadModelNum;
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
	//�v��LodaModel�]�m���c(�ץ��Ҧ��B��T�סB�M��t�סB�ǰt�ȡB�j�M�d��e�סB�j�M�d�򰪫סB�j�M�d��_�l���סB�j�M�d�򵲧����סB�j�M���~�s���B�M�ݹ�ܮ�)
	struct VisionSet {
		BOOL ModifyMode;
		BOOL Accuracy;
		BOOL Speed;
		BYTE Score;
		int width;
		int height;
		int Startangle;
		int Endangle;
		LONG AdjustOffsetX;
		LONG AdjustOffsetY;
	};
	//�v��LoadModel�ɮצ�m���c(���|�B�ɮ��`�ơB�ɮצW��)
	struct VisionFile {
		LPTSTR ModelPath;
		UINT ModelCount;
		std::vector<CString> AllModelName;
	};
	//�v���X�j�j�M ()
	struct VisionTrigger {
		UINT AdjustStatus;
		BOOL Trigger1Switch;
		BOOL Trigger2Switch;
		std::vector<CoordinateData> Trigger1;
		std::vector<CoordinateData> Trigger2;
	};
	struct VisionSerchError {
		int SearchError;
		CDialog* pQuestion;
		BOOL Manuallymode;
		BOOL Pausemode;
	};
	//�v����l�Ƶ��c
	struct VisionDefault{
		VisionSet VisionSet;
		VisionFile VisionFile;
		VisionSerchError VisionSerchError;
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
		std::vector<UINT> RunCount;//�p�Ƶ{�ǩR�O
		std::vector<UINT> MSChange; //���ޥثe��Ū���{��
		UINT StackingCount;
		std::vector<UINT> ActionStatus;  
	};
	//�B�檬�AŪ�����c(�B�檬�A�B�ثe�R�O�i�סB�^���I���A(TRUE = ���k���� FLASE = ���k��))
	struct RunStatusRead {
		UINT RunStatus;
		UINT CurrentRunCommandNum;
		BOOL GoHomeStatus;
        UINT FinishProgramCount;
	};
	//���_���޵��c(���_�ƶq�B���_�}�C)
	struct StepRepeatBlockData {
		int BlockNumber;
		std::vector<CString> BlockPosition;
	};
	//Step&Loop���޵��c(�`���}���B�`���a�}�����B�`���p�ơB�B�J���D���ҡB�W�[�B�J���h�j��}���B�O���B�J���h�j��s�W���ơB�O���B�J���h�j��R�����ơB�O��S���ഫ�}���B�B�J�a�}�����B�B�J��loffsetX�����B�B�J��loffsetY�����B�B�J�p��X�B�B�J�p��Y�B�O�����_���)
	/*
	*�`���}��:�ΨӧP�O�S�������Үɪ��p
    *�B�J���D����:�Ω����j��ɸ��D���O��
    *�W�[�B�J���h�j��}��:�Ω�P�_�O�_�ĤG���s�W���h�j��
    *�O���B�J���h�j��s�W����:�O���`�@���X�Ӥ��h�A�Ω�P�_�R���ᶷ�s�W�̤j��
    *�O���B�J���h�j��R������:�O���R�����X�Ӥ��h�A�Ω󥲶��b�s�W�^��
    *�O��S���ഫ�}��:�Ω�S���A�P�_Offset�ݭn�[�ٴ�
    *�B�J�a�}����:�Ω�P�_���O�ثe��StepRepeat�O�ݩ���@�Ӱ}�C
    *�O�����_���:�]�t�ƶq�B���q�}�C���r��
	*/
	struct RepeatData {
		BOOL LoopSwitch;
		std::vector<UINT> LoopAddressNum;
		std::vector<UINT> LoopCount;
		CString StepRepeatLabel;
		BOOL StepRepeatLabelLock;
        BOOL AddInStepRepeatSwitch;
		int AllNewStepRepeatNum;
		int AllDeleteStepRepeatNum;
		std::vector<BOOL> SSwitch;
		std::vector<UINT> StepRepeatNum;
		std::vector<UINT> StepRepeatInitOffsetX;
		std::vector<UINT> StepRepeatInitOffsetY;
		std::vector<int> StepRepeatCountX;
		std::vector<int> StepRepeatCountY;
		std::vector<StepRepeatBlockData> StepRepeatBlockData;
	};
    /*�{�Ǵ`���B�浲�c(����`���}���B�`�����ơB�̤j�B�榸��(�] -1 =�S������))*/
    struct RunLoopData {      
        BOOL RunSwitch;
        int LoopNumber;
        int LoopCount;
        int MaxRunNumber;
    };
private:    //�ܼ�
	HANDLE          wakeEvent;
	//�D�B�ʪ���
	CAction         m_Action;
	//�R�O
	StepRepeatBlockData InitBlockData;
	CoordinateData  InitData;
	CString         Commanding;
	std::vector<CString> CommandSwap;
	std::vector<std::vector<CString>> Command;
	//�{��
	RepeatData      RepeatData;
	Program         Program;
	RunData         RunData;
	//���A
	std::vector<CoordinateData> ArcData, CircleData1, CircleData2, StartData, OffsetData;
   
	
private:    //���
	//������
	static  UINT    HomeThread(LPVOID pParam);
	static  UINT    Thread(LPVOID pParam);
	static  UINT    SubroutineThread(LPVOID pParam);
    static  UINT    RunLoopThread(LPVOID pParam);
	//�ʧ@�B�z
	static  void    LineGotoActionJudge(LPVOID pParam);
	static  void    ModifyPointOffSet(LPVOID pParam, CString XYZPoint);
	static  void    VisionModify(LPVOID pParam);
	void            VisionFindMarkError(LPVOID pParam);
    //���_�B�z
	static  void    BlockProcessStart(CString Command, LPVOID pParam, BOOL RepeatStatus);
	static  BOOL    BlockProcessExecute(CString Command, LPVOID pParam, int NowCount);
    static  void    BlockProcessStartY(CString Command, LPVOID pParam, BOOL RepeatStatus);
    static  BOOL    BlockProcessExecuteY(CString Command, LPVOID pParam, int NowCount);
    static  void    BlockSort(std::vector<CString> &BlockPosition, int Type, int mode);
    static  CString BlockResolve(CString String, UINT Choose);
	//�R�O�B�z
	static  CString CommandResolve(CString Command,UINT Choose);
	//��l�ƳB�z
	void            ParameterDefult();
	void            DecideInit();
	void            DecideClear();
	void            MainSubProgramSeparate();
	//�ɮ׳B�z
	BOOL            ListAllFileInDirectory(LPTSTR szPath, LPTSTR szName);
	static  BOOL    FileExist(LPCWSTR FilePathName);
  
	
public:     //�ܼ�
	//�ʧ@�`��
	LONG            Time;
	//�{�ǰ}�C
	std::vector<CString> CommandMemory;
	//�B�ʰѼƭ�
	Default         Default;

	DispenseDotSet  DispenseDotSet;
	DispenseDotEnd  DispenseDotEnd;
	DispenseLineSet DispenseLineSet;
	DispenseLineEnd DispenseLineEnd;
	Speed           DotSpeedSet, LineSpeedSet;
	ZSet            ZSet;
	GlueData        GlueData;
	GoHome          GoHome;

	//�B��{�Ǹ��
	RunStatusRead   RunStatusRead;
    //RunLoop
    RunLoopData     RunLoopData;
	//�v���Ѽ� 
	VisionDefault   VisionDefault;
	VisionSet       VisionSet;
	VisionFile      VisionFile;
	VisionSerchError VisionSerchError;

	//�v��
	Vision          FindMark, FiducialMark1, FiducialMark2;
	VisionOffset    VisionOffset;
	VisionTrigger   VisionTrigger;

	//�v���ץ���u�@�y��
	CoordinateData  FinalWorkCoordinateData;
public:     //���
	COrder();
	virtual ~COrder();
	//�}�l�R�O��Ķ(���\return 1����return 0)
	BOOL    Run();
    //�s��}�l�R�O��Ķ(�Ѽ�:�`������)(�^�ǭ�:���\ return 1 ���� return 0)
    BOOL    RunLoop(int LoopNumber);
	//�ߧY����R�O��Ķ(���\return 1����return 0)
	BOOL    Stop();
	//�Ȱ��R�O��Ķ(���\return 1����return 0)
	BOOL    Pause();
	//�~��R�O��Ķ(���\return 1����return 0)
	BOOL    Continue();
	//���I���k(�Ѽ�:�Ҧ�(FALSE �w�Y TRUE CCD))(�^�ǭ�:���\return  1 ���� return 0 )
	BOOL    Home(BOOL mode);
	//View�R�O��Ķ(�Ѽ�:�Ҧ�(FALSE �w�Y TRUE CCD))(���\return 1 ���� return 0)
	BOOL    View(BOOL mode);

protected:
	DECLARE_MESSAGE_MAP()
};


