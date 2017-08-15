/*
*�ɮצW��:Order.h
*�ɮץγ~:3Axis
*�ɮ׾֦��\��:Move�BLaser�BVision�BCheck
*�A�ζb�d:Nova
*�A�ιp�g:�Q�U
*��s���:2017/06/21
*�@�̦W��:Rong
*/
#pragma once
#include <vector>
#include "Action.h"
#include "CInterpolationCircle.h"

static CWinThread* g_pThread = NULL;
static CWinThread* g_pSubroutineThread = NULL;
static CWinThread* g_pRunLoopThread = NULL;
static CWinThread* g_pIODetectionThread = NULL;
static CWinThread* g_pCheckCoordinateScanThread = NULL;
static CWinThread* g_pCheckActionThread = NULL;
static CWinThread* g_pMosaicDlgThread = NULL;
class COrder : public CWnd
{
	DECLARE_DYNAMIC(COrder)
private:
	/************************************************************�B�ʰѼƵ��c*******************************************************/
	//�y�е��c(���A�B����X�B����Y�B����Z)
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
	//�M��w�Y���c(�M��˸m��m�B�M��˸m�}���BIOport)
	struct CleanerData {
		CoordinateData  CleanerPositionData;
		BOOL CleanerSwitch;
		LONG OutputPort;
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
		CleanerData     CleanerData;
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
    //�v���Ҧ��ഫ�}��(FindMark�BFiducialMark1�BFiducialMark2�BFindMarkAdjust�BFiducialMarkAdjust)
    struct VisionSwitch {
        BOOL FindMark;
        BOOL FiducialMark;
        BOOL FindMarkFinish;
        BOOL FiducialMarkFinish;
    };
	//�v������I���c(�аO�d�䪬�A(TRUE = ��� FALSE = �����)�B����I�B��J�Z���BLoadModel�s���B�s��Model���w�B���᰾���qX�B���᰾���qY)
	struct Vision {
		BOOL FindMarkStatus;
		CoordinateData Point;
		//LONG FocusDistance;//2017/08/14�w�ް�
		UINT LoadModelNum;
		void* MilModel; 
		DOUBLE OffsetX;
		DOUBLE OffsetY;
        std::vector<CoordinateData> Trigger;
	};
	//�v���ץ��p�⵲�c(����I�B����I�᰾���qX�B����I�᰾���qY�B����I�᰾������)
	struct VisionOffset {
		CoordinateData Contraposition;
		DOUBLE OffsetX;
		DOUBLE OffsetY;
		DOUBLE Angle;
	};
	//�v��LodaModel�]�m���c(�ץ��Ҧ��B��T�סB�M��t�סB�ǰt�ȡB�j�M�d��e�סB�j�M�d�򰪫סB�j�M�d��_�l���סB�j�M�d�򵲧����סBCCD�w�YOffsetX�BCCD�w�YOffsetY�B��J����)
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
		LONG FocusHeight;
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
        int TriggerSwitch;
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
	//Laser��Ƭ���(�p�g���q����L�a�}�B�p�g���I�����y�СB�p�g�u�q���������}�l�M�����y�СB�p�g�����ƾ�)
	struct LaserData {
		int LaserExecutedAddress;
		CoordinateData LaserHeightPoint;
		CoordinateData LaserDetectLS, LaserDetectLE;
		LONG LaserMeasureHeight;		
	};
	//Laser�����Ȭ����}�C(�p�g�����ƾ�)
	struct LaserAdjust {
		LONG LaserMeasureHeight;
	};
	/*Laser�s��u�q����Ѽ�(�s��u�q�p�ơB�s��u�q�}��)
	*�s��u�q�}��:����O�_�����s��u�q(�I�s����s��u�q�ϥ�)
	*/
	struct LaserContinuousControl {
		LONG ContinuousLineCount;
		BOOL ContinuousSwitch;
	};
	/************************************************************�˴��ѼƵ��c*******************************************************/
	//�˴��y�е��c(�˴��Ҧ��B�˴��Ҧ��a�}�B�R�O�a�}�B�y�Ц�m)
	struct CheckCoordinate{
		CString CheckMode;
		CString CheckModeAddress;
		CString Address;
		CoordinateData Position;
	};
	//�ҪO�}�����c(�ҪO�˴��B���|�˴��B�ϰ��˴��B�B���˴��B�Y���˴�(0:���˴�1:�ҪO2:���|))
	struct CheckSwitch {
		BOOL Template;
		BOOL Diameter;
		BOOL Area;
		BOOL RunCheck;
		UINT ImmediateCheck;
	};
	//�ҪO�˴����c(�ҪO�˴��a�}�BOK�ҪO�ƶq�BOK�ҪO���w�}�C�BNG�ҪO�ƶq�BNG�ҪO���w�}�C�B���Ѽ�)
	struct TemplateCheck {
		CString Address;
		LONG OKModelCount;
		void** OKModel;
		LONG NGModelCount;
		void** NGModel;
		VisionSet   VisionParam;
	};
	//���|�˴����c(�ҪO�˴��a�}�B���|�B�e�\�~�t�B�ⶥ�B�G�ȤƬɭ���)
	struct DiameterCheck{
		CString Address;
		DOUBLE Diameter;
		DOUBLE Tolerance;
		BOOL Color;
		DOUBLE Binarization;
	};
	//�ɮ׵��c(�ɮ׸��|�B�ɮצW��)
	struct File {
		CString Path;
		CString Name;
	};
	//�����˴����G�ƶq���c(OK�ƶq�BNG�ƶq�B���_�ƶq�BOK���չϡBNG���չ�)
	struct CheckResult {
		LONG OKCount;
		LONG NGCount;
		LONG Error;
		LONG OKMosaicingImage;
		LONG NGMosaicingImage;
	};
	//���������I�˴����c(�˴����G�r��,�˴��y�е��c{�˴��Ҧ��a�},�˴��a�},�˴��y��})
	struct CheckFinishRecord {
		CString Result;
		File ResultFile;
		CheckCoordinate CheckData;
	};
	//���|�B�ҪO�H���w�]�ȵ��c(���|�˴����G�ϸ��|)
	struct DTCheckParamterDefault {
		File DiameterResultSave;
	};
	//�˴��]�m�ؼе��c(�˴����G�ؼмƶq�B�ؼйF���ʧ@)
	struct CheckSetGoal{
		CheckResult Goal;
		LONG Action;
	};
	//�V�m���c(�x�s�ɮ׵��c�B�I��ơB���q����(���|�B�u�e)�B�e�\�~�t�q�B�ⶥ�B�G�ȤƬɭ���)
	struct TrainData {
		File TrainSave;
		std::vector<CPoint> PointData;
		DOUBLE MeasureLimit;
		DOUBLE MaxOffset;
		BOOL WhiteOrBlack;
		DOUBLE Threshold;
	};
	//���յ��c(�x�s�ɮ׵��c�B�������ʶq�B�_�l��m�B������m�B���ʼƶqX,���ʼƶqY)
	struct MosaicingData {
		File ImageSave;
		POINT ViewMove;
		POINT Start;
		POINT End;
		int MoveCountX;
		int MoveCountY;
	};
	//�ϰ��˴����c(�ϰ��˴��a�}�B���չϵ��c�B�I�V�m���c�B�u�V�m���c�B�x�s���G�ɮ׵��c)
	struct AreaCheck {
		CString Address;
		MosaicingData Image;
		TrainData DotTrain;
		TrainData LineTrain;
		File Result;
	};
	//���������ϰ��˴����G���c(�˴��Ҧ��a�}�B�˴����G�ɮ׵��c�B�˴����G)
	struct AreaCheckFinishRecord {
		CString CheckModeAddress;
		File ResultImage;
		BOOL Result;
	};
	//�ϰ��˴��H���w�]�ȵ��c(�������ʶq�B���չ��x�s���|�B�I�V�m�x�s���|�B�u�V�m�x�s���|�B���G���x�s���|�B���դ���ܵ�)
	struct AreaCheckParamterDefault {
		POINT ViewMove;
		File ImageSave;
		File DotTrainSave;
		File LineTrainSave;
		File Result;
		CDialog* pMosaicDlg;
	};	
	/************************************************************�{�ǰѼƵ��c*******************************************************/
	/*Label&Subroutine���޵��c(���ҭp�ơB���ҦW�١B�l�{�ǭp�ơB�l�{�Ǫ��A�B�l�{�Ǧa�}���|�B�l�{�Ǯy�а��|�B�l�{�ǼҦ����|�B�l�{�Ǽv���ץ��P�_���|�B�l�{�ǩR�O�w�B�z�B�l�{�ǼҲո����}��) 
	*���ҭp��:�Ψӭp��i�J���ҮɩR�O�O�_�����M�䧹��
	*���ҦW��:�ΨӰO�����n���ЦW��
	*�l�{�ǭp��:�Ψӭp�ƥثe���I�s�X�Ӥl�{��
	*�l�{�Ǧa�}���|:�ΨӰO���I�s�l�{�Ǯɪ��R�O�a�}
	*�l�{�Ǯy�а��|:�ΨӰO���I�s�l�{�Ǯɪ�������u��m
	*�l�{�ǼҦ����|:�ΨӰO���I�s�l�{�Ǯɥثe�R�O�B�@���Ҧ�(ex:1�v��2�p�g...
	*�l�{�Ǽv���ץ��P�_���|:����l�{�Ǥ����R�O�O�_���v���ץ�
	*�l�{�ǩR�O�w�B�z:�Ψ��x�s�M�ثe��m���۹�q�ץ����R�O
	*�l�{�ǼҲո����}��:�ΨӨ���p�g�����j����X�l�{�ǩM������R�O���ĥu��EndSubroutine����
	*/
	struct Program {
		int LabelCount;
		CString LabelName;
		UINT SubroutinCount;
		BOOL CallSubroutineStatus;
		std::vector<UINT> SubroutineStack;
		std::vector<CoordinateData> SubroutinePointStack;
		std::vector<UINT> SubroutineModel;
		std::vector<BOOL> SubroutineVisioModifyJudge;
		CString SubroutineCommandPretreatment;
		BOOL SubroutineModelControlSwitch;
	};
	/*�B�浲�c(�Ƶ{���W�١B�B��p�ơB����D�Ƶ{�ǡB�D�Ƶ{�ǰ��|�p�ơB�ʧ@���A)	
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
	/*�B�檬�AŪ�����c(�B�檬�A�B�ثe�R�O�i�סB�^���I���A�B�{�ǧ����p�ơB�@���{�ǹB��ɶ��B�B��j�骬�A�B�榸�R�O�B�檬�A�B�e����s�����A�B���ժ��A)
	*�B�@���A(0:���B�@ 1 : �B�椤 2 : �Ȱ���)
	*�^���I���A(TRUE = ���k���� FLASE = ���k��)
	*�B��j�骬�A(0:���B��B�Ȱ��� 1:���椤)
	*�榸�R�O�B�檬�A(0:���槹���B������ 1:���b���椤)
	*�e����s�����A(0:������ 1:�����M��)//�H���n�D
	*���ժ��A(-1:���դ� 0:���ե��� 1:���զ��\ 2:������)
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
		int MosaicStatus;
		//TODO::���ᰵ�`�i�רϥ�
		//BOOL RegistrationStatus;
		//UINT CommandTotalCount;
		//UINT CurrentRunCommandCount;
	};
    //�϶���Ƶ��c(���O�B�_�l�a�}�B�����a�})
    struct IntervalData {
        CString Type;
        UINT BeginAddress;
        UINT EndAddress;
    };
	//���_���޵��c(���_�ƶq�B���_�}�C)
	struct StepRepeatBlockData {
		int BlockNumber;
		std::vector<CString> BlockPosition;
	};
	/*Step&Loop���޵��c(�`���}���B�`���a�}�����B�`���p�ơB�B�J���D���ҡB�W�@�ӨB�J���D���ҡB�B�J���D�}���B�B�J���Ұ_�l�a�}�B�W�[�B�J���h�j��}���B�O���B�J���h�j��s�W���ơB
	�O���B�J���h�j�馸�ơB�O��S���ഫ�}���B�B�J�a�}�����B�B�J��loffsetX�����B�B�J��loffsetY�����B�B�J�p��X�B�B�J�p��Y�B�O�����_��ơBStepRepeat�϶����|�B�B�J�p���`��X�B�B�J�p���`��Y)
	*�`���}��:�ΨӧP�O�S�������Үɪ��p
	*�B�J���D����:�Ω����j��ɸ��D���O��
	*�B�J���D�}��:�ت��Ω����StepRepeat�̥~�h�j��
	*�B�J���Ұ_�l�a�}:�ΨӰO����eStepRepeatLabel�a�}
	*�W�[�B�J���h�j��}��:�Ω�P�_�O�_�ĤG���s�W���h�j��
	*�O���B�J���h�j��s�W����:�O���`�@���X�Ӥ��h�A�Ω�P�_�R���ᶷ�s�W�̤j��
	*�O���B�J���h�j��R������:�O���R�����X�Ӥ��h�A�Ω󥲶��b�s�W�^��
	*�O��S���ഫ�}��:�Ω�S���A�P�_Offset�ݭn�[�ٴ�
	*�B�J�a�}����:�Ω�P�_���O�ثe��StepRepeat�O�ݩ���@�Ӱ}�C
	*�B�J�p���`��X�BY:�O��X�`���Ʀ��ơA�D�n�Ω�p�������key�ȥ�
	*�O�����_���:�]�t�ƶq�B���q�}�C���r��
	*StepRepeat�϶����|:�O��StepRepeat���u�@�϶�
	*/
	struct RepeatData {
		BOOL LoopSwitch;
		std::vector<UINT> LoopAddressNum;
		std::vector<UINT> LoopCount;
		CString StepRepeatLabel;
		CString StepRepeatLabelPrevious;//�ثe�S�Ψ�
		BOOL StepRepeatLabelLock;
		UINT StepRepeatAddress;
		BOOL AddInStepRepeatSwitch;
		int AllNewStepRepeatNum;
		int AllDeleteStepRepeatNum;
		std::vector<BOOL> SSwitch;
		std::vector<UINT> StepRepeatNum;
		std::vector<UINT> StepRepeatInitOffsetX;
		std::vector<UINT> StepRepeatInitOffsetY;
		std::vector<int> StepRepeatCountX;
		std::vector<int> StepRepeatCountY;
		std::vector<UINT> StepRepeatTotalX;//�u���Φb�إ߭ץ����
		std::vector<UINT> StepRepeatTotalY;//�u���Φb�إ߭ץ����
		std::vector<StepRepeatBlockData> StepRepeatBlockData;
		std::vector<IntervalData> StepRepeatIntervel;
	};
	//�{�Ǵ`���B�浲�c(����`���}���B�`�����ơB�`���p�ơB�̤j�B�榸��(�] -1 =�S������))
	struct RunLoopData {
		BOOL RunSwitch;
		int LoopNumber;
		int LoopCount;
		int MaxRunNumber;
	};
	//���J�R�O�ѼƵ��c(�Ĥ@���B��}��)
	struct LoadCommandData {
		BOOL FirstRun;
	};
	//�~���禡�I�s���c
	typedef void(*CDrawFunction) (void*,CPoint,int);
	struct CallFunction {
		CDrawFunction CDrawFunction;
		void* pObject;
	};
	//���������P�B�ƥ󵲺c
	struct ThreadEvent {
		HANDLE Thread;
		HANDLE RunLoopThread;
		HANDLE CheckActionThread;
		HANDLE CheckCoordinateScanThread;
		HANDLE MosaicDlgThread;
	};
    /************************************************************�ҲհѼƵ��c*******************************************************/
    //���޼Ҳյ��c(�Ҧ���ܡB�v���Ҧ��ഫ�a�}�B�p�g�Ҧ��ഫ�a�}�B�v���Ҳո��L�B�p�g�M�˴��Ҳո��L)
    struct ModelControl {
        UINT Mode;
        int  VisionModeChangeAddress;
        int  LaserModeChangeAddress;
        BOOL VisionModeJump;
        BOOL LaserAndCheckModeJump;
    };
    //�Ҧ��ഫ�Ȧs���c(StepRepeat�������c,Subroutine�������c,�I�sSubroutine�ɬ������A���c)
    struct ModelConversionData {
        RepeatData RepeatDataRecord;
        Program ProgramRecord;
        std::vector<CoordinateData> ArcData, CircleData1, CircleData2, StartData, OffsetData;
        std::vector<UINT> ActionStatus;
    };
private:    //�ܼ�
	ThreadEvent     ThreadEvent;
	//HANDLE          OutThreadEvent;//����OpenEvent��
	HANDLE          wakeEvent;
	/*�B��ɶ��p��*/
	LARGE_INTEGER   startTime, endTime, fre;
	/*�N���Ȧs�ܼ�*/ 
	StepRepeatBlockData InitBlockData;
	CoordinateData  InitData;
	/*�R�O*/
	CString         Commanding;
	std::vector<CString> CommandSwap;
	std::vector<std::vector<CString>> Command;
	/*�{��*/
	//RepeatData      RepeatDataRecord;//�ª�2017/08/14�ק�
	RepeatData      RepeatData;
	Program         Program;
	RunData         RunData;
	LoadCommandData LoadCommandData;
	/*�Ҳձ���*/
	ModelControl    ModelControl;
    ModelConversionData ModelConversionData;//�ഫ�ҲծɼȦs���
	/*���A*/
	std::vector<CoordinateData> ArcData, CircleData1, CircleData2, StartData, OffsetData;
	/*IO*/
	IOControl       IOControl;
	/*���ץ����������y��(�ثe�Φb���ӭp��Subroutine���t�q)*/
	CoordinateData  NVMVirtualCoordinateData; 
	/*�t�β��ʳt��*/
	Speed           MoveSpeedSet,LMPSpeedSet,LMCSpeedSet,VMSpeedSet;
	/*(�Ȧs)�P�_�v���O�_�ץ�(Subroutine�ϥ�)*/
	BOOL            VisioModifyJudge;
	
private:    //�禡
	/*������*/
	static  UINT    HomeThread(LPVOID pParam);//���I���k�{��
	static  UINT    Thread(LPVOID pParam);//�D�{��
	static  UINT    SubroutineThread(LPVOID pParam);//�R�O�ʧ@�{��
	static  UINT    RunLoopThread(LPVOID pParam);//�B��j��{��
	static  UINT    IODetection(LPVOID pParam);//IO�����{��
	static  UINT    CheckCoordinateScan(LPVOID pParam);//�϶��˴�����{��
	static  UINT    CheckAction(LPVOID pParam);//�϶��˴�����{��
	static  UINT    MosaicDlg(LPVOID pParam);//�ҪO���դ�����
	/*�ʧ@�B�z*/
	static  void    LineGotoActionJudge(LPVOID pParam);//�P�_�u�q�ʧ@�ഫ
	static  void    ModifyPointOffSet(LPVOID pParam, CString XYZPoint);//CallSubroutin�ץ��B�z
	static  CString VirtualNowOffSet(LPVOID pParam, CString Command);//�����y�Эp�ⰾ�t��
	static  void    VisionModify(LPVOID pParam);//�v���ץ�
	void            VisionFindMarkError(LPVOID pParam);//�v�������B�z��k
	static  void    LaserModify(LPVOID pParam);//�p�g�ץ�
	static  void    LaserDetectHandle(LPVOID pParam, CString Command);//�p�g�˴��B�z
	BOOL            LaserPointDetect();//�ˬd�p�g�˴��I�O�_����
	static  void    VirtualCoordinateMove(LPVOID pParam, CString Command, LONG type);//�����y�в���
	BOOL            CheckDraw();//�I�˴��e��
	static  void    PassingException(LPVOID pParam);//�����I�ҥ~�B�z(�Ȱ��B����ϰ��˴�)
	//�����y�м�������
	/*��ƪ�B�z�϶�*/
	static  void    ChooseVisionModify(LPVOID pParam);//��ܼv���ץ���
	static  void    ChooseLaserModify(LPVOID pParam);//��ܹp�g�ץ���
	static  void    RecordCorrectionTable(LPVOID pParam);//�O���B�ʭץ���
	void            GetHashAddress(CString CommandAddress, UINT &D1, UINT &D2);//���hash��a�}
	/*�R�O�B�z*/
	static  CString CommandResolve(CString Command, UINT Choose);//�R�O����
	static  CString ModelNumResolve(CString ModelNum, UINT Choose);//�Ҫ��s������
	CString         CommandUnitConversinon(CString Command, DOUBLE multiple, DOUBLE Timemultiple);//�R�O����ഫ
	CString         GetCommandAddress();//����s�X�L�R�O�a�}
	/*�{���ܼƳB�z�϶�*/
	void            ParameterDefult();//�B�ʰѼƪ�l��
	void            DecideInit();//�{�Ǫ�l��
	void            DecideClear();//�{�ǵ����M��
	void            MainSubProgramSeparate();//�����D�Ƶ{��
	void            DecideBeginModel(BOOL ViewMode);//�P�_�_�l�Ҳ�
	/*�ɮ׳B�z*/
	BOOL            ListAllFileInDirectory(LPTSTR szPath, LPTSTR szName);//�j�M�ؿ��ɮ�
	static  BOOL    FileExist(LPCWSTR FilePathName);
	static  BOOL    FilePathExist(CString FilePathName);
	static  CString GetDataFileName();
	static  BOOL    FileDelete(CString FilePathName);
	static  BOOL    SearchDirectory(CString FilePathName);
	//�P�_�ɮ׬O�_�s�b
	/*StepRepeat�B�z*/
	BOOL            SubroutinePretreatmentFind(LPVOID pParam);//CallSubroutin�w�B�z�M��
	static  void    StepRepeatJumpforciblyJudge(LPVOID pParam, UINT Address);//StepRepeat�j�����P�_
	/*���_�B�z*/
	static  void    BlockProcessStartX(CString Command, LPVOID pParam, BOOL RepeatStatus);
	static  BOOL    BlockProcessExecuteX(CString Command, LPVOID pParam, int NowCount);
	static  void    BlockProcessStartY(CString Command, LPVOID pParam, BOOL RepeatStatus);
	static  BOOL    BlockProcessExecuteY(CString Command, LPVOID pParam, int NowCount);
	static  void    BlockSort(std::vector<CString> &BlockPosition, int Type, int mode);
	static  CString BlockResolve(CString String, UINT Choose);
	/*�˴��B�z*/
	static  void    ModelLoad(BOOL Choose, LPVOID pParam, CString ModelNum, TemplateCheck &TemplateCheck);
	BOOL            ClearCheckData(BOOL Moment, BOOL Interval);
	static  void    LineTrainDataCheck(LPVOID pParam);
	BOOL            NewCutPathPoint(CoordinateData Start, CoordinateData Passing, CoordinateData End, AreaCheck &IntervalAreaCheck, int Type);//�s�W�I���\�^��1 ���Ѧ^��0
	BOOL            PointAreaJudge(POINT Point, CRect Area);//�P�_�ϰ줺�^��1 �~�^��0
	BOOL            LineAreaJudge(POINT PointS, POINT PointE, CRect Area);//�P�_�ϰ줺�^��1 �~�^��0
	BOOL            ArcAreaJudge(POINT PointS, POINT PointA, POINT PointE, CRect Area);//�P�_�ϰ줺�^��1 �~�^��0
	BOOL            CircleAreaJudge(POINT PointS, POINT PointC1, POINT PointC2, CRect Area);//�P�_�ϰ줺�^��1 �~�^��0
	BOOL            AutoCalculationArea(AreaCheck &AreaCheckRun);//�۰ʭp�⭫�հϰ�j�p
	CRect           MosaicAreaJudge(AreaCheck &AreaCheck);//�P�_���հϰ�O�_���P�@�I
	/*��L�\��(Demo��)*/
	static  void    SavePointData(LPVOID pParam);

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

	/*���եμȮɩ�bpublic*/
	DispenseDotSet  DispenseDotSet;
	DispenseDotEnd  DispenseDotEnd;
	DispenseLineSet DispenseLineSet;
	DispenseLineEnd DispenseLineEnd;   
	Speed           DotSpeedSet, LineSpeedSet;
	ZSet            ZSet;
	GlueData        GlueData;
	GoHome          GoHome;
	CleanerData     CleanerData;
	/********************/

	//�B��{�Ǹ��
	RunStatusRead   RunStatusRead;
	//RunLoop
	RunLoopData     RunLoopData;
	//�v���Ѽ�
	VisionDefault   VisionDefault;

	/*���եμȮɩ�bpublic*/
	VisionSet       VisionSet;
	VisionFile      VisionFile;
	VisionSerchError VisionSerchError;
	//�v�����
    VisionSwitch    VisionSwitch;
	Vision          FindMark, FiducialMark1, FiducialMark2;
	VisionOffset    VisionOffset;
	VisionTrigger   VisionTrigger;
	//�p�g���
	LaserSwitch     LaserSwitch;
	LaserData       LaserData;
	LaserContinuousControl LaserContinuousControl;
	/*�v���p�g�ץ���u�@�y��*/
	CoordinateData  FinalWorkCoordinateData;
	/*���������y�� (�ثe�u�ΦbCallSubroutine�^�k�I)*/
	CoordinateData  VirtualCoordinateData;
	
	//�ʧ@�ץ���
	std::vector<std::vector<std::vector<PositionModifyNumber>>> PositionModifyNumber;
	//�v���ץ����
	std::vector<VisionAdjust> VisionAdjust;
	//�p�g�ץ����
	std::vector<LaserAdjust> LaserAdjust;
	//�v���B�p�g��ƭp��
	int             VisionCount;//�Ψӭp�ƲĴX�����v�����
    int             VisionCountTemp;//�ΨӼȦs�p�ƲĴX�����v�����
	int             LaserCount;//�Ψӭp�ƲĴX�����p�g����
	//�˴����                                                     
	CheckSwitch     CheckSwitch;
	CString         CurrentCheckAddress;//�s��ثe�˴��a�}(�s�W�ɥΨӧP�_�O�_�����ƪ��˴��϶���ƩM�Ω��I��T�϶��˴���ƪ��s�W)
	TemplateCheck   TemplateChecking;
	std::vector<TemplateCheck> IntervalTemplateCheck;
	DiameterCheck   DiameterChecking;
	std::vector<DiameterCheck> IntervalDiameterCheck;
	std::vector<CheckCoordinate> IntervalCheckCoordinate;//���|�B�ҪO�϶��˴��Ҧ��I���
	std::vector<AreaCheck> IntervalAreaCheck;
	//�˴��B��P�_���
	CoordinateData  AreaCheckChangTemp;//�ϰ줤���I�ഫ�˴��Ȧs�Ѽ�
	CheckCoordinate CheckCoordinateRun;//�Ȧs�ثe�n�B�檺�˴��I�y��
	AreaCheck       AreaCheckRun;//�Ȧs�ثe�n�B�檺�ϰ��˴����
	LONG            CheckModel;//�˴��ҲէP�_

	//�e�ϩI�s�禡�]�w
	CallFunction    CallFunction;

	//�����ҥ~�����I
	CoordinateData  PassingExceptionTemp;//���������I
	/********************/
	//���|�B�ҪO�˴��w�]�ȰѼ�
	DTCheckParamterDefault DTCheckParamterDefault;
	//�ϰ��˴��w�]�ȰѼ�
	AreaCheckParamterDefault AreaCheckParamterDefault;
	//�˴����G�s��
	std::vector<CheckFinishRecord> CheckFinishRecord;//�˴����G�I��T
	std::vector<AreaCheckFinishRecord> AreaCheckFinishRecord;//�ϰ��˴����G�ϸ�T
	CheckResult     CheckResult;//�˴����G�ƶq
	
	//�����ثe��������a�}(�p�g�ϥ�)(�s��S��u�q�ϥ�)
	CString         CurrentTableAddress;
	//Demo���J�ΧP�_�Ÿ�(DEMO ��)
	BOOL            DemoTemprarilySwitch;
	//IO�ѼƳ]�w
	IOParam         IOParam;
   
public:     //�B�����禡
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
	//I/O����������}(�Ѽ�:�}��(TRUE �}�� FALSE ����),�Ҧ�:�ثe�|�����\��)
	BOOL    IODetectionSwitch(BOOL Switch, int mode);
	//���J�ɮ�
	void    LoadPointData();
	//�ˬd�R�O�W�h(return ���~�N�X , ErrorAddress �����~�R�O�a�})
	int     CheckCommandRule(int &ErrorAddress);
	//�d�ݩҦ����A��T
	void    ShowAllStatus();
	//���J�s�[�u�R�O(���\return 1 ����return 0)
	BOOL    LoadCommand();
public:    //�]�w���禡
	//�]�m�e�ϩI�s�禡(���\return 1����return 0)
	BOOL    SetDrawFunction(CDrawFunction Funtion, void* pObject);
	//�]�m���x����Z��(�Ѽ�:�^�ǰ��FZ��)(���\return 1 ���� return 0)
	BOOL    SetTabelZ(int* TableZ);
	
protected:
	DECLARE_MESSAGE_MAP()
};


