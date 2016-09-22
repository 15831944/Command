#pragma once
#include <vector>
#include "Action.h"
// COrder
static CWinThread* g_pThread = NULL;
static CWinThread* g_pSubroutineThread = NULL;
static CWinThread* g_pRunLoopThread = NULL;
static CWinThread* g_pIODetectionThread = NULL;
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
	//�t�׵��c(�[�t�סB�X�ʳt�סB��t��)
	struct Speed {
		LONG AccSpeed;
		LONG EndSpeed;
		LONG InitSpeed;
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
	//��l�Ƶ��c(�I���]�m���c�B�I���������c�B�I���t�סB�u�q�]�m���c�B�u�q�������c�B�u�q�t�סBZ�b���c�B�ƽ����c�B�^���I���c)
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
	//�ץ����c(�R�O�a�}�B�v���ץ��s���B�p�g�ץ��s��)
	struct PositionModifyNumber {
		CString Address;
		int VisionNumber;
		int LaserNumber;
	};
	/************************************************************I/O�ѼƵ��c*******************************************************/
	//IO����c(IO�����}���B�X���s�P�_)
	struct IOControl {
		BOOL SwitchInformation;
		BOOL GlueInformation;
	};
	//IO�ѼƵ��c(EMG��ܮث��w)
	struct IOParam{
		CDialog* pEMGDlg;
	};
	/************************************************************�v���ѼƵ��c*******************************************************/
	//�v������I���c(�аO�d�䪬�A(TRUE = ��� FALSE = �����)�B����I�B��J�Z���BLoadModel�s���B�s��Model���w�B���᰾���qX�B���᰾���qY)
	struct Vision {
		BOOL FindMarkStatus;
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
	//�v���X�j�j�M(�վ㪬�A�B�X�j���1�}���B�X�j���2�}���B�X�j���϶�1�B�X�j���϶�2)
	struct VisionTrigger {
		UINT AdjustStatus;
		BOOL Trigger1Switch;
		BOOL Trigger2Switch;
		std::vector<CoordinateData> Trigger1;
		std::vector<CoordinateData> Trigger2;
	};
	//�v���j�M���~���c(�j�M���~�覡�B�M�ݹ�ܮث��w�B��ʼҦ��}���B�Ȱ��Ҧ��P�_)
	struct VisionSerchError {
		int SearchError;
		CDialog* pQuestion;
		BOOL Manuallymode;
		BOOL Pausemode;
	};
	//�v����l�Ƶ��c(�v���]�m���c�B�v������I�ɮ׵��c�B�v���j�M���~���c)
	struct VisionDefault{
		VisionSet VisionSet;
		VisionFile VisionFile;
		VisionSerchError VisionSerchError;
	};
	//�v��Offset�Ȭ����}�C(�v���ץ��p�⵲�c)
	struct VisionAdjust {
		VisionOffset VisionOffset;
	};
	/************************************************************�p�g�ѼƵ��c*******************************************************/ 
	//Laser�Ҧ��ഫ�}��(�p�g���סB�p�g�I�ո`�B�p�g�˴��B�p�g�վ�B�p�g���L)
	struct LaserSwitch {
		BOOL LaserHeight;
		BOOL LaserPointAdjust;
		BOOL LaserDetect;
		BOOL LaserAdjust;
		BOOL LaserSkip;
	};
	//Laser��Ƭ���(�p�g�}�Ҧa�}�B�p�g���I�����y�СB�p�g�u�q���������}�l�M�����y�СB�p�g�����ƾ�)
	struct LaserData {
		UINT LaserOpenAddress;
		CoordinateData LaserHeightPoint;
		CoordinateData LaserDetectLS, LaserDetectLE;
		LONG LaserMeasureHeight;		
	};
	//Laser�����Ȭ����}�C(�p�g�����ƾ�)
	struct LaserAdjust {
		LONG LaserMeasureHeight; 
	};
	/************************************************************�ҲհѼƵ��c*******************************************************/
	//���޼Ҳյ��c(�Ҧ���ܡB�Ҧ��ഫ�a�}(�ثe���ϥ�)�B�v���Ҳո��L�B�p�g�Ҳո��L)
	struct ModelControl{
		UINT Mode;
		UINT ModeChangeAddress;
		UINT VisionModeJump;
		UINT LaserModeJump;
	};
	/************************************************************�{�ǰѼƵ��c*******************************************************/
	//Label&Subroutine���޵��c(���ҭp�ơB���ҦW�١B�l�{�ǭp�ơB�l�{�Ǫ��A�B�l�{�Ǧa�}���|�B�l�{�Ǯy�а��|�B�l�{�Ǳ���O�_�ץ��v���B�l�{�ǩR�O�w�B�z)
	struct Program {
		int LabelCount;
		CString LabelName;
		UINT SubroutinCount;
		BOOL CallSubroutineStatus;
		std::vector<UINT> SubroutineStack;
		std::vector<CoordinateData> SubroutinePointStack;
		std::vector<BOOL> SubroutineVisioModifyJudge;
		CString SubroutineCommandPretreatment;
	};
	//�B�浲�c(�Ƶ{���W�١B�B��p�ơB����D�Ƶ{�ǡB�D�Ƶ{�ǰ��|�p�ơB�ʧ@���A)  
	/*	
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
	//�B�檬�AŪ�����c(�B�檬�A�B�ثe�R�O�i�סB�^���I���A�B�{�ǧ����p�ơB�@���{�ǹB��ɶ��B�B��j�骬�A�B�榸�R�O�B�檬�A�B�e����s�����A)
	/*
	*�B�@���A(0:���B�@ 1 : �B�椤 2 : �Ȱ���)
	*�^���I���A(TRUE = ���k���� FLASE = ���k��)
	*�B��j�骬�A(0:���B��B�Ȱ��� 1:���椤)
	*�榸�R�O�B�檬�A(0:���槹���B������ 1:���b���椤)
	*�e����s�����A(0:������ 1:�����M��)//�H���n�D
	*/
	struct RunStatusRead {
		UINT RunStatus;
		UINT CurrentRunCommandNum;
		BOOL GoHomeStatus;
		int FinishProgramCount;
		DOUBLE RunTotalTime;
		BOOL RunLoopStatus;
		BOOL StepCommandStatus;
		BOOL PaintClearClose;
		//TODO::���ᰵ�`�i�רϥ�
		//BOOL RegistrationStatus;
		//UINT CommandTotalCount;
		//UINT CurrentRunCommandCount;
	};
	//���_���޵��c(���_�ƶq�B���_�}�C)
	struct StepRepeatBlockData {
		int BlockNumber;
		std::vector<CString> BlockPosition;
	};
	//Step&Loop���޵��c(�`���}���B�`���a�}�����B�`���p�ơB�B�J���D���ҡB�B�J���D�}���B�W�[�B�J���h�j��}���B�O���B�J���h�j��s�W���ơB�O���B�J���h�j��R�����ơB�O��S���ഫ�}���B�B�J�a�}�����B�B�J��loffsetX�����B�B�J��loffsetY�����B�B�J�p��X�B�B�J�p��Y�B�O�����_���)
	/*
	*�`���}��:�ΨӧP�O�S�������Үɪ��p
	*�B�J���D����:�Ω����j��ɸ��D���O��
	*�B�J���D�}��:�ت��Ω����StepRepeat�̥~�h�j��
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
	/*�{�Ǵ`���B�浲�c(����`���}���B�`�����ơB�`���p�ơB�̤j�B�榸��(�] -1 =�S������))*/
	struct RunLoopData {      
		BOOL RunSwitch;
		int LoopNumber;
		int LoopCount;
		int MaxRunNumber;
	};
	
private:    //�ܼ�
	HANDLE          wakeEvent;
	LARGE_INTEGER   startTime, endTime, fre;
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
	//IO
	IOControl       IOControl;
	//���ץ����������y��
	CoordinateData  NVMVirtualCoordinateData;
   
	
private:    //���
	//������
	static  UINT    HomeThread(LPVOID pParam);
	static  UINT    Thread(LPVOID pParam);
	static  UINT    SubroutineThread(LPVOID pParam);
	static  UINT    RunLoopThread(LPVOID pParam);
	static  UINT    IODetection(LPVOID pParam);
	//�ʧ@�B�z
	static  void    LineGotoActionJudge(LPVOID pParam);
	static  void    ModifyPointOffSet(LPVOID pParam, CString XYZPoint);
	static  void    VisionModify(LPVOID pParam);
	void            VisionFindMarkError(LPVOID pParam);
	static  void    LaserModify(LPVOID pParam);
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
	void            DecideBeginModel();
	//�ɮ׳B�z
	BOOL            ListAllFileInDirectory(LPTSTR szPath, LPTSTR szName);
	static  BOOL    FileExist(LPCWSTR FilePathName);
	//�Ҳձ��޳B�z
	static  void    RecordCorrectionTable(LPVOID pParam);
	static  void    ChooseVisionModify(LPVOID pParam);
	static  void    ChooseLaserModify(LPVOID pParam);
	//�p�g�B�z
	static  void    LaserDetectHandle(LPVOID pParam, CString Command);
	//�����ץ��B�z
	static  CString VirtualNowOffSet(LPVOID pParam , CString Command);
	//�����y�м���
	static  void    VirtualCoordinateMove(LPVOID pParam, CString Command, LONG type);
	//����ഫ
	CString         CommandUnitConversinon(CString Command, DOUBLE multiple, DOUBLE Timemultiple);
	//��L�\��(Demo��)
	static  void    SavePointData(LPVOID pParam);
	//CallSubroutin�w�B�z�M��
	BOOL            SubroutinePretreatmentFind(LPVOID pParam);
	
	
public:     //�ܼ�
	//�D�B�ʪ���
	CAction         m_Action;
	//�ʧ@�`��
	LONG            ActionCount;
	LONG            V_ActionCount;
	//�{�ǰ}�C
	std::vector<CString> CommandMemory;
	std::vector<CString> mmCommandMemory;
	//�B�ʰѼƭ�
	Default         Default;

	DispenseDotSet  DispenseDotSet;
	DispenseDotEnd  DispenseDotEnd;
	DispenseLineSet DispenseLineSet;
	DispenseLineEnd DispenseLineEnd;
	Speed           DotSpeedSet, LineSpeedSet,MoveSpeedSet;
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
	//�p�g
	LaserSwitch     LaserSwitch;
	LaserData       LaserData;
	//�v���ץ���u�@�y��
	CoordinateData  FinalWorkCoordinateData;
	//*���������y��
	CoordinateData  VirtualCoordinateData;
	
	//�Ҳձ���
	ModelControl    ModelControl;

	//�ʧ@�ץ���
	std::vector<PositionModifyNumber> PositionModifyNumber;
	//�v�����
	std::vector<VisionAdjust> VisionAdjust;
	//�p�g���
	std::vector<LaserAdjust> LaserAdjust;

	//�v���B�p�g��ƭp��
	int             VisionCount;
	int             LaserCount;

	//�����ثe��������a�}(�S��u�q�ϥ�)
	CString         CurrentTableAddress;
	//(�Ȧs)�P�_�v���O�_�ץ�
	BOOL            VisioModifyJudge;
	
	//�s��u�q�p��(�s�򱽴y�ϥ�)
	LONG            ContinuousLineCount;
	//����O�_�����s��u�q(�I�s����s��u�q�ϥ�)
	BOOL            ContinuousSwitch;

	//Demo���J�ΧP�_�Ÿ�(DEMO ��)
	BOOL            DemoTemprarilySwitch;

	//IO�ѼƳ]�w
	IOParam         IOParam;

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
	//I/O����������}(�Ѽ�:�}��(TRUE �}�� FALSE ����),�Ҧ�)
	BOOL    IODetectionSwitch(BOOL Switch, int mode);
	//���J�ɮ�
	void    LoadPointData();
	
protected:
	DECLARE_MESSAGE_MAP()
};


