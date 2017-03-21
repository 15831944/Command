// Vision.h : Vision DLL ���D�n���Y��
//
#pragma once

#ifndef __AFXWIN_H__
	#error "�� PCH �]�t���ɮ׫e���]�t 'stdafx.h'"
#endif

//MIL��l��ưt�m�t��
//#define ProjectUse
#define TestUse

#include "resource.h"		// �D�n�Ÿ�

//===================================================================================================
//
// Vision DLL ���
//
//===================================================================================================

//---------------------------------------------------------------------------------------------------
// ��¦���
//---------------------------------------------------------------------------------------------------
//MIL Vision �t�m:Application,System,Display,Digitizer,Image�����ŧi�t�m�Ccolor:0�Ƕ�,1:���m
extern "C" _declspec(dllexport) void VI_VisionAlloc(bool color);
//�Y�ɼv��:�}1/��0
extern "C" _declspec(dllexport) void VI_CameraSwitch(bool OnOff);
//��v���Y�ɼv����������
#ifdef ProjectUse
extern "C" _declspec(dllexport) void VI_CameraInit(CWnd* DisplayWindow, bool color, bool move);
#endif
#ifdef TestUse
extern "C" _declspec(dllexport) void VI_CameraInit(bool color, bool move);
#endif
//���o��v�� FrameRate
extern "C" _declspec(dllexport) void VI_GetCameraFrameRate(int &FrameRate);
//���o��v�� �����j�p�B�ѪR��
extern "C" _declspec(dllexport) void VI_GetCameraResolution(int &PixelWidth, int &PixelHeight);
//��v���v��,�������]�m
extern "C" _declspec(dllexport) void VI_DiaplayImageUnitSetup(BYTE DisplayID);
//Display ������ܦ�m�t�m:move:1->���ܤp������
extern "C" _declspec(dllexport) void VI_DisplayAlloc(CWnd* DisplayWindow, bool move);
//��ܦ�m�t�m�C�f�tVI_VisionAlloc�ϥΡC�Ω��l�Ƥ��t�mCWnd* DisplayWindow
extern "C" _declspec(dllexport) void VI_DisplaySet(CWnd* DisplayWindow, bool move);
//��ܦ�m�t�m�A�t���W���Ioffset��������
extern "C" _declspec(dllexport) void VI_DisplayPosSet(CWnd* DisplayWindow, double OffsetX, double OffsetY);
//�Y�ɼv���Y�p���,�۰�Fit��ܵ���
extern "C" _declspec(dllexport) void VI_DisplayFitWindow(CWnd* DisplayWindow, bool move, double objwidth, double objheight, double Yoffset);
//�]�mø�s�Ϊ��z���ϼh
extern "C" _declspec(dllexport) void VI_SetOverlayImage();
//�M���z���ϼh���e
extern "C" _declspec(dllexport) void VI_ClearOverlayImage();
////��v���w�]���ø�s
extern "C" _declspec(dllexport) void VI_DrawFOVDefault();
//ø�s�����Q�r
extern "C" _declspec(dllexport) void VI_DrawCross(BYTE mode, CWnd* DisplayWindow);
//ø�s�x�ή�
extern "C" _declspec(dllexport) void VI_DrawBox(BYTE mode, CWnd* DisplayWindow, int width, int height);
//�̼Ҧ����ø�s
extern "C" _declspec(dllexport) void VI_DrawFOVFrame(BYTE mode, CWnd* DisplayWindow, int width, int height);
//�v���ӷ��e���A��j�P�Y�p
extern "C" _declspec(dllexport) void VI_DisplayZoom(CWnd* DisplayWindow, double XFactor, double YFactor, bool move, bool disp);

//---------------------------------------------------------------------------------------------------
// Pattern Match �������
//---------------------------------------------------------------------------------------------------
//�q�����x�ήثإ�PatternMatch Model
extern "C" _declspec(dllexport) void VI_CreateModelFromBox(BYTE mode, CWnd* DisplayWindow, void* MilModel, int width, int height);
//��J�j�M�_�l���סB�������סC�p��j�M���I�B�f�ɰw(positive����V���f)�B���ɰw�j�M����(negaitive�t��V����). 
extern "C" _declspec(dllexport) void VI_SearchAngleCalc(double startangle, double endangle, double &origin, double &delta);
//PatternMatch �ѼƳ]�m
extern "C" _declspec(dllexport) void VI_SetPatternMatch(void* MilModel, bool accuracy, bool speed, BYTE score, double startangle, double endangle);
//�]�wPatternMatch �j�M����
extern "C" _declspec(dllexport) void VI_SetSearchAngle(void* MilModel, double startangle, double endangle);
//�]�wPatternMatch �j�M�d��A�������ߦ�m�A���w�e�סB����
extern "C" _declspec(dllexport) void VI_SetSearchRange(void* MilModel, int width, int height);
//�]�wPatternMatch �j�M�d��AOffsetX�BOffsetY�A�e�סB����
extern "C" _declspec(dllexport) void VI_SetSearchScope(void* MilModel, int offsetX, int offsetY, int width, int height);
//�j�M�аO�A�̼Ҧ��\��
extern "C" _declspec(dllexport) bool VI_FindMarkProc(BYTE mode, void* MilModel, double &X, double &Y, double &Angle, double &Score);
//�j�M�аO�A�M��ø�s���m
extern "C" _declspec(dllexport) bool VI_FindMarkDrawResult(CWnd* DisplayWindow, void* MilModel, double &Score);
//�j�M�аO�A�M��^�Ǩ䤤�ߦ�m
extern "C" _declspec(dllexport) bool VI_FindMarkCenterPos(void* MilTargetImage, void* MilModel, double &PosX, double &PosY);
//�j�M�аO�A�M��^�ǻP�������߬۹�Z��
extern "C" _declspec(dllexport) bool VI_FindMarkRelDist(void* MilModel, bool PixelOrRel, double &DistX, double &DistY, double &Score);
//�d��аO���O�A�^�ǻP�������߬۹��ڶZ��
extern "C" _declspec(dllexport) bool VI_FindMark(void* MilModel, double &DistX, double &DistY);
//�ʺA�d��аO�ACameraTrigger���O�C
extern "C" _declspec(dllexport) bool VI_CameraTrigger(void* MilModel, long &MarkPointX, long &MarkPointY, long TriggerPointX, long TriggerPointY, double &DistX, double &DistY);
//�إߤ��ʯx�ή�: �v�����չ� or �ѦҨӷ��Ϥj�p
extern "C" _declspec(dllexport) void VI_SetInteractiveBoxRefImg(bool Source, CString path, CString name);
//�إߤ��ʯx�ή�
extern "C" _declspec(dllexport) void VI_SetInteractiveBox(int width, int height);
//�إߤ��ʯx�ή� (�i����)
extern "C" _declspec(dllexport) void VI_SetInteractiveAngleBox(int width, int height, double angle);
//�q���ʯx�ήثإ�PatternMatch Model 
extern "C" _declspec(dllexport) bool VI_CreateModelFromInteractiveBox(void* MilModel, CString path, CString name, double objwidth, double objheight);
//���oInteractiveBox���ʯx�ήئ�m��T
extern "C" _declspec(dllexport) void VI_GetInteractiveBoxPos(int &PosX, int &PosY, int &Width, int &Height);
//���oInteractiveBox���ʯx�ήئ�m�M���׸�T
extern "C" _declspec(dllexport) void VI_GetInteractiveBoxAnglePos(int &CenterX, int &CenterY, int &Width, int &Height, double &Angle);
//�������ʼҦ��A�������ʯx�ή�
extern "C" _declspec(dllexport) void VI_RemoveInteractiveBox();
//Ū���ɮסA�˴��ɮצs�b�P�_
extern "C" _declspec(dllexport) bool VI_CheckFileExist(CString path, CString name);
//�t�mModel�Ŷ�  (�f�tVI_LoadModel�ϥ�)
extern "C" _declspec(dllexport) void VI_ModelAlloc(void* MilModel);
//���J PatternMatch Model�ɨ�즳�t�m�n��Model�Ŷ� 
extern "C" _declspec(dllexport) void VI_LoadModel(void* MilModel, CString path, CString name);
//�t�mModel�Ŷ��åB���JPatternMatch Model�� 
extern "C" _declspec(dllexport) void VI_RestoreModel(void* MilModel, CString path, CString name);
//�x�s PatternMatch Model
extern "C" _declspec(dllexport) void VI_SaveModel(void* MilModel, CString path, CString name);

//---------------------------------------------------------------------------------------------------
// ����v���������
//---------------------------------------------------------------------------------------------------
//Capture ������� ���Ѽv���B�z��
extern "C" _declspec(dllexport) void VI_Capture(bool full, int width, int height);
//GetImage ���o�v���Ȧs�A�H���ϥ�
extern "C" _declspec(dllexport) void VI_GetImage(void* MilImage, bool full, int width, int height);
//GetPicture ���o�Ϥ��ɮסA�ë��w�x�s���|
extern "C" _declspec(dllexport) void VI_GetPicture(CString path, CString name, bool full, int width, int height);
//���w��ܵ������e���A���s�վ�Ϥ���j�Y�p
extern "C" _declspec(dllexport) void VI_ReSizeImage(CString ImportPath, CString ImportName, CString ExportPath, CString ExportName, double objwidth, double objheight);
//�N�Ϥ���ܨ���whandle�C//�p��0�h���Application�����إߡA���|��ܹϤ]���ݭn����
extern "C" _declspec(dllexport) BOOL VI_ShowPic(CString path, CString name, HWND &DisplayWindow);

//---------------------------------------------------------------------------------------------------
// MIL Free() �O����B�귽����������
//---------------------------------------------------------------------------------------------------
//PatternMatch Model Free
extern "C" _declspec(dllexport) void VI_ModelFree(void* MilModel);
//Image Buffer Free
extern "C" _declspec(dllexport) void VI_ImageBufferFree(void* MilImage);
//MIL Vision Free
extern "C" _declspec(dllexport) void VI_VisionFree();
//�q���A�귽����
extern "C" _declspec(dllexport) void VI_MmeasFree();
//MmeasTool All Free
extern "C" _declspec(dllexport) void VI_MeasureToolFree();
//�I���˴� Free
extern "C" _declspec(dllexport) void VI_CircleBeadFree();
//AreaCheckDisplay Free
extern "C" _declspec(dllexport) void VI_AreaCheckDispFree();

//---------------------------------------------------------------------------------------------------
// �v���ѼơA�������
//---------------------------------------------------------------------------------------------------
//�]�w1�ӹ��������
extern "C" _declspec(dllexport) void VI_SetOnePixelUnit(double xUnit, double yUnit);
//�]�w�v�������M��ڶZ�������Y (Uniform)
extern "C" _declspec(dllexport) void VI_SetUniform(bool ImgOrCal, double xUnits, double yUnits);
//�إ�Pixel�BPulse�ഫ���Y
extern "C" _declspec(dllexport) void VI_SetPixelPulseRelation(CWnd* DisplayWindow, void* MilModel, double LocatX1, double LocatY1, double LocatX2, double LocatY2, double &xUnit, double &yUnit);
////�]�w��v���M�I���w�Y�������Z��Camera-to-Needle(Tip) Offset 
extern "C" _declspec(dllexport) void VI_SetCameraToTipOffset(double OffsetX, double OffsetY);

//---------------------------------------------------------------------------------------------------
// �ץ��p��A�������
//---------------------------------------------------------------------------------------------------
//����y�Ц�m�ץ�
extern "C" _declspec(dllexport) void VI_CorrectLocation(long &PointX, long &PointY, long RefX, long RefY, double OffSetX, double OffSetY, double Andgle, bool Mode);
//�u����ਤ�׭p��
//�����I��ﱵ�����F��m�p��V�q����,�^�ǹ���I1��offset�Ψ�V�q����
//1=�����I�Ҧ����A0=�u���@�I
extern "C" _declspec(dllexport) void VI_FindModel(void *PicTemp, void *PicTemp1, void* Model, void* Model1, double LocatX, double LocatY, double LocatX1, double LocatY1, double &OffSetX, double &OffSetY, double &Angle);
//���o Model�ɪ���m��T
extern "C" _declspec(dllexport) void VI_GetModelPos(void* MilModel, double &PosX, double &PosY);
//�p��V�q����
extern "C" _declspec(dllexport) double VI_AngleCount(double LocatX, double LocatY, double LocatX1, double LocatY1, double sumx, double sumy, double sumx1, double sumy1);
//�p��V�q���� (�Ÿ��ק睊)
extern "C" _declspec(dllexport) double VI_AngleOfRotationCalc(double LocatX1, double LocatY1, double LocatX2, double LocatY2, double OffsetX1, double OffsetY1, double  OffsetX2, double  OffsetY2);

//---------------------------------------------------------------------------------------------------
// �ƹ��y�� �������
//---------------------------------------------------------------------------------------------------
//�ƹ��y�Шƥ� Callback Function Eable
extern "C" _declspec(dllexport)	void VI_MousePosFuncEable();
//Callback Function Disable
extern "C" _declspec(dllexport) void VI_MousePosFuncDisable();
//���o�ƹ��y�Ц�m
extern "C" _declspec(dllexport) bool VI_GetMousePos(double &PosX, double &PosY);

//---------------------------------------------------------------------------------------------------
// �q���������
//---------------------------------------------------------------------------------------------------
//�q��ø�ϰt�m
extern "C" _declspec(dllexport) void VI_MmeasGraphicsAlloc();
//�q���u��]�m
extern "C" _declspec(dllexport) void VI_MeasureToolSetup();
//�q�������t�m
extern "C" _declspec(dllexport) void VI_MeasureMarkerAlloc(BYTE MarkType);
//�q�����סG�q���I1�]�m
extern "C" _declspec(dllexport) void VI_SetPointMarker1(double PosX, double PosY);
//�q�����סG�q���I2�]�m
extern "C" _declspec(dllexport) void VI_SetPointMarker2(double PosX, double PosY);
//�q���Z���p��
extern "C" _declspec(dllexport) bool VI_MeasureCalculate();
//�q�����׭p��
extern "C" _declspec(dllexport) bool VI_MeasureLengthCalc(BYTE Type);
//ø�s�q����m
extern "C" _declspec(dllexport) void VI_DrawMeasurePos();
//ø�s�q������
extern "C" _declspec(dllexport) void VI_DrawMeasureLength();
//ø�s���׶Z��
extern "C" _declspec(dllexport)	void VI_DrawLengthDist(BYTE Type);
//�q����G�q���I1�]�m
extern "C" _declspec(dllexport) bool VI_SetCircleMarker1(bool BlackOrWhite, double PosX, double PosY, double OuterRadius);
//�q����G�q���I2�]�m
extern "C" _declspec(dllexport) bool VI_SetCircleMarker2(bool BlackOrWhite, double PosX, double PosY, double OuterRadius);
//�q���ꪽ�|
extern "C" _declspec(dllexport) bool VI_MeasCircleDiameter(bool BlackOrWhite, double PosX, double PosY, double OuterRadius);
//�q����ߦ�m
extern "C" _declspec(dllexport) bool VI_MeasCircleCenter(bool BlackOrWhite, double PosX, double PosY, double OuterRadius, double MotorLocatX, double MotorLocatY);
//Stripe Width �q��:�]�w�q���x�νd�򪺼e���B���ץH�Τ��ߦ�m
extern "C" _declspec(dllexport) bool VI_MeasureStripeWidth(double CenterX, double CenterY, double Width, double Height, double Angle, double &Max, double &Mean, double &Min);
//Stripe Width �q��:�����N��V���I�A�_�l�I�B�����I�����q���d��,�B�ݭn�]�w�u����V
extern "C" _declspec(dllexport) bool VI_GetStripeWidthFromTwoPoint(double StartPointX, double StartPointY, double EndPointX, double EndPointY, bool HV, double &Max, double &Mean, double &Min);
//�v���e���A�ƹ��I���AĲ�I������
extern "C" _declspec(dllexport) void VI_GetTouchMoveDist(double TouchPosX, double TouchPosY, double &DistX, double &DistY);

//---------------------------------------------------------------------------------------------------
// �}�C���˪����J�B�]�m�B���
//---------------------------------------------------------------------------------------------------
//���J�}�C Model
extern "C" _declspec(dllexport) void VI_LoadMatrixModel(void * Model[], CString Path, CString Name[], int Num);
//���J�}�C Model
extern "C" _declspec(dllexport) void VI_SetMultipleModel(void * Model[], bool accuracy, bool speed, BYTE score, double startangle, double endangle, int Num);
//�@�����}�C Model
extern "C" _declspec(dllexport) bool VI_FindMatrixModel(void* Model[], int Num);
//����}�C Model
extern "C" _declspec(dllexport) void VI_MatrixModelFree(void* Model[], int Num);

//---------------------------------------------------------------------------------------------------
// Bead Inspection �˴�
//---------------------------------------------------------------------------------------------------
//�I���˴��˪O�إ�
extern "C" _declspec(dllexport) void VI_CircleBeadTrain(double Diameter, double MaxOffset, bool WhiteOrBlack, double Threshold);
//�I���˴�����
extern "C" _declspec(dllexport) 	int VI_CircleBeadVerify(bool UnitsEnable, double MaxOffset, CString Path, CString Name);

//---------------------------------------------------------------------------------------------------
// �v���������� Registration & �v���M�� Edge Finder 
//---------------------------------------------------------------------------------------------------
//�v���������հt�m
extern "C" _declspec(dllexport) void VI_MosaicingImagesAlloc(int MaxCNT);
//�v���������հt�m (�������ե�)
extern "C" _declspec(dllexport) void VI_MosaicAllocDebugUse(int MaxCNT);
//�v���������աA���ʶq�]�m
extern "C" _declspec(dllexport) void VI_MosaicingMoveSet(double FOVx, double FOVy, double FOVPercent, long &MoveX, long &MoveY);
//�v���������աA���o���ʶq (FOVPercent:%�ʤ���BMoveXY���um)
extern "C" _declspec(dllexport) void VI_GetMosaicMoveDist(double FOVPercent, long &MoveX, long &MoveY);
//�v���������աA���x�y�Э��I�p��
extern "C" _declspec(dllexport) void VI_MosaicingImagesOriginCalc(long LocatX, long LocatY);
//�v�������A���ռƶq�p��
extern "C" _declspec(dllexport) bool VI_MosaicingImagesSizeCalc(long StartPointX, long StartPointY, long EndPointX, long EndPointY, int &MovXcnt, int &MovYcnt);
//�O���ݫ������ժ��v��
extern "C" _declspec(dllexport) void VI_MosaicingImagesCapture();
//�v���������աA�귽����
extern "C" _declspec(dllexport) void VI_MosaicingImagesFree(int MaxCNT);
//���oDXF�ɡA�y���ഫ�Ѽ�
extern "C" _declspec(dllexport) void VI_GetMosaicingImagesDXFinfo(double &OriginX, double &OriginY, double &ScaleX, double &ScaleY);
//�v���������ճB�z
extern "C" _declspec(dllexport) bool VI_MosaicingImagesProcess(CString path, CString name, bool type);
//Registration Context�BResult Free
extern "C" _declspec(dllexport) void VI_RegistrationFree();
//����v���������խp��
extern "C" _declspec(dllexport) void VI_MosaicingImagesProcessStop();
//�v���������աA��ʵ����B�z
extern "C" _declspec(dllexport) bool VI_SetCutMosaicingImages(CString path, CString name);
//�������ռv�����
extern "C" _declspec(dllexport) bool VI_MosaicingImagesDisplay(CWnd* DisplayWindow, CString path, CString name, bool move, double objwidth, double objheight);
//�������ռv����ܡA�귽����
extern "C" _declspec(dllexport) void VI_MosaicingImagesDisplayFree();
//�v���������չ�,�G�ȤƳB�z
extern "C" _declspec(dllexport) void VI_BinarizeMosaicImage(double Threshold);
//���o��ĳ���G�ȤƼƭ�
extern "C" _declspec(dllexport) void VI_RecommendThreshold(BYTE Select, int &Threshold);
//�v���������չ�,�M��B�z
extern "C" _declspec(dllexport) bool VI_EdgeProcessMosaicImage(CString path, CString name, bool type, bool WhiteOrBlack, double Threshold, double EdgelSize, bool SimpleEnable, double tolerance);
//���J�v���������չϡA�M��B�z
extern "C" _declspec(dllexport) bool VI_LoadMosaicImageEdgeFind(CString path, CString name, bool type, bool WhiteOrBlack, double Threshold, double EdgelSize, bool SimpleEnable, double tolerance);

//---------------------------------------------------------------------------------------------------
// �ϰ�Bead�˴� (���I�I�����|�M��u�q���e) 
//---------------------------------------------------------------------------------------------------
//�ϰ�Bead�˴��G���I�I�����|�A�˴��V�m
extern "C" _declspec(dllexport) bool VI_ImagesCircleBeadTrain(CString DotPath, CString DotName, CPoint DotPosition[], double Diameter, double MaxOffset, bool WhiteOrBlack, double Threshold, int DotPositionNum);
//�ϰ�Bead�˴��G��u�q���e�A�˴��V�m
extern "C" _declspec(dllexport) bool VI_ImagesLineBeadTrain(CString LinePath, CString LineName, CPoint LinePosition[], double LineWidth, double MaxOffset, bool WhiteOrBlack, int LinePositionNum);
//�ϰ�Bead�˴��P����
extern "C" _declspec(dllexport) bool VI_AreaBeadVerify(CString ImagePath, CString ImageName, CString DotPath, CString DotName, CString LinePath, CString LineName, CString ResultImagePath, CString ResultImageName, int Mode);


//===================================================================================================

//------------------------------------------------------------------------------
// CVisionApp
// �o�����O����@�аѾ\ Vision.cpp
//

class CVisionApp : public CWinApp
{
public:
	CVisionApp();

// �мg
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
