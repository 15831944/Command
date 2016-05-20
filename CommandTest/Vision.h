// Vision.h : Vision DLL ���D�n���Y��
//
#pragma once

#ifndef __AFXWIN_H__
	#error "�� PCH �]�t���ɮ׫e���]�t 'stdafx.h'"
#endif

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
extern "C" _declspec(dllexport) void VI_VisionAlloc(BOOL color);
//�Y�ɼv��:�}1/��0
extern "C" _declspec(dllexport) void VI_CameraSwitch(BOOL OnOff);
//��v���Y�ɼv�����������
extern "C" _declspec(dllexport) void VI_CameraInit(BOOL color, BOOL move);
//Display ������ܦ�m�t�m:move:1->���ܤp������
extern "C" _declspec(dllexport) void VI_DisplayAlloc(CWnd* DisplayWindow, BOOL move);
//�]�mø�s�Ϊ��z���ϼh
extern "C" _declspec(dllexport) void VI_SetOverlayImage();
//�M���z���ϼh���e
extern "C" _declspec(dllexport) void VI_ClearOverlayImage();
//ø�s�����Q�r
extern "C" _declspec(dllexport) void VI_DrawCross(BYTE mode, CWnd* DisplayWindow);
//ø�s�x�ή�
extern "C" _declspec(dllexport) void VI_DrawBox(BYTE mode, CWnd* DisplayWindow, int width, int height);
//�̼Ҧ����ø�s
extern "C" _declspec(dllexport) void VI_DrawFOVFrame(BYTE mode, CWnd* DisplayWindow, int width, int height);

//---------------------------------------------------------------------------------------------------
// Pattern Match �������
//---------------------------------------------------------------------------------------------------
//�q�����x�ήثإ�PatternMatch Model
extern "C" _declspec(dllexport) void VI_CreateModelFromBox(BYTE mode, CWnd* DisplayWindow, void* MilModel, int width, int height);
//��J�j�M�_�l���סB�������סC�p��j�M���I�B�f�ɰw(positive����V���f)�B���ɰw�j�M����(negaitive�t��V����). 
extern "C" _declspec(dllexport) void VI_SearchAngleCalc(int startangle, int endangle, int &origin, int &delta);
//PatternMatch �ѼƳ]�m
extern "C" _declspec(dllexport) void VI_SetPatternMatch(void* MilModel, BOOL accuracy, BOOL speed, BYTE score, int startangle, int endangle);
//�]�wPatternMatch �j�M����
extern "C" _declspec(dllexport) void VI_SetSearchAngle(void* MilModel, int startangle, int endangle);
//�]�wPatternMatch �j�M�d��A�������ߦ�m�A���w�e�סB����
extern "C" _declspec(dllexport) void VI_SetSearchRange(void* MilModel, int width, int height);
//�j�M�аO�A�̼Ҧ��\��
extern "C" _declspec(dllexport) BOOL VI_FindMaskProc(BYTE mode, void* MilModel, DOUBLE &X, DOUBLE &Y, DOUBLE &A);
//�j�M�аO�A�M��ø�s���m
extern "C" _declspec(dllexport) BOOL VI_FindMaskDrawResult(CWnd* DisplayWindow, void* MilModel, DOUBLE &Score);
//�j�M�аO�A�M��^�Ǩ䤤�ߦ�m
extern "C" _declspec(dllexport) BOOL VI_FindMaskCenterPos(void* MilTargetImage, void* MilModel, DOUBLE &PosX, DOUBLE &PosY);
//�j�M�аO�A�M��^�ǻP�������߬۹�Z��
extern "C" _declspec(dllexport) BOOL VI_FindMaskRelDist(void* MilTargetImage, void* MilModel, BOOL PixelOrRel, DOUBLE &DistX, DOUBLE &DistY, DOUBLE &Score);
//�d��аO���O�A�^�ǻP�������߬۹��ڶZ��
extern "C" _declspec(dllexport) BOOL VI_FindMask(void* MilModel, DOUBLE &DistX, DOUBLE &DistY);
//�إߤ��ʯx�ή�
extern "C" _declspec(dllexport) void VI_SetInteractiveBox(int width, int height);
//�q���ʯx�ήثإ�PatternMatch Model 
extern "C" _declspec(dllexport) void VI_CreateModelFromInteractiveBox(void* MilModel, CString path, CString name);
//�������ʼҦ��A�������ʯx�ή�
extern "C" _declspec(dllexport) void VI_RemoveInteractiveBox();
//���J PatternMatch Model
extern "C" _declspec(dllexport) void VI_LoadModel(void* MilModel, CString path, CString name);
//�x�s PatternMatch Model
extern "C" _declspec(dllexport) void VI_SaveModel(void* MilModel, CString path, CString name);

//---------------------------------------------------------------------------------------------------
// ����v���������
//---------------------------------------------------------------------------------------------------
//Capture ������� ���Ѽv���B�z��
extern "C" _declspec(dllexport) void VI_Capture(BOOL full, int width, int height);
//GetImage ���o�v���Ȧs�A�H���ϥ�
extern "C" _declspec(dllexport) void VI_GetImage(void* MilImage, BOOL full, int width, int height);
//GetPicture ���o�Ϥ��ɮסA�ë��w�x�s���|
extern "C" _declspec(dllexport) void VI_GetPicture(CString path, CString name, BOOL full, int width, int height);

//---------------------------------------------------------------------------------------------------
// MIL Free() �O����B�귽����������
//---------------------------------------------------------------------------------------------------
//PatternMatch Model Free
extern "C" _declspec(dllexport) void VI_ModelFree(void* MilModel);
//Image Buffer Free
extern "C" _declspec(dllexport) void VI_ImageBufferFree(void* MilImage);
//MIL Vision Free
extern "C" _declspec(dllexport) void VI_VisionFree();

//---------------------------------------------------------------------------------------------------
// �v���ѼơA�������
//---------------------------------------------------------------------------------------------------
//�]�w1�ӹ��������
extern "C" _declspec(dllexport) void VI_SetOnePixelUnit(DOUBLE xUnit, DOUBLE yUnit);
//�إ�Pixel�BPulse�ഫ���Y
extern "C" _declspec(dllexport) void VI_SetPixelPulseRelation(CWnd* DisplayWindow, void* MilModel, double LocatX1, double LocatY1, double LocatX2, double LocatY2, double &xUnit, double &yUnit);
////�]�w��v���M�I���w�Y�������Z��Camera-to-Needle(Tip) Offset 
extern "C" _declspec(dllexport) void VI_SetCameraToTipOffset(DOUBLE OffsetX, DOUBLE OffsetY);

//---------------------------------------------------------------------------------------------------
// �ץ��p��A�������
//---------------------------------------------------------------------------------------------------
//����y�Ц�m�ץ�
extern "C" _declspec(dllexport) void VI_CorrectLocation(long &PointX, long &PointY, long RefX, long RefY, double OffSetX, double OffSetY, double Andgle);
//�u����ਤ�׭p��
//�����I��ﱵ�����F��m�p��V�q����,�^�ǹ���I1��offset�Ψ�V�q����
//1=�����I�Ҧ����A0=�u���@�I
extern "C" _declspec(dllexport) void VI_FindModel(void *PicTemp, void *PicTemp1, void* Model, void* Model1, double LocatX, double LocatY, double LocatX1, double LocatY1, double &OffSetX, double &OffSetY, double &Angle);
//�p��V�q����
extern "C" _declspec(dllexport) double VI_AngleCount(double LocatX, double LocatY, double LocatX1, double LocatY1, double sumx, double sumy, double sumx1, double sumy1);
//�p��V�q���� (�Ÿ��ק睊)
extern "C" _declspec(dllexport) double VI_AngleOfRotationCalc(double LocatX1, double LocatY1, double LocatX2, double LocatY2, double OffsetX1, double OffsetY1, double  OffsetX2, double  OffsetY2);

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