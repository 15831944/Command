// Vision.h : Vision DLL 的主要標頭檔
//
#pragma once

#ifndef __AFXWIN_H__
	#error "對 PCH 包含此檔案前先包含 'stdafx.h'"
#endif

#include "resource.h"		// 主要符號

//===================================================================================================
//
// Vision DLL 函數
//
//===================================================================================================

//---------------------------------------------------------------------------------------------------
// 基礎函數
//---------------------------------------------------------------------------------------------------
//MIL Vision 配置:Application,System,Display,Digitizer,Image相關宣告配置。color:0灰階,1:全彩
extern "C" _declspec(dllexport) void VI_VisionAlloc(BOOL color);
//即時影像:開1/關0
extern "C" _declspec(dllexport) void VI_CameraSwitch(BOOL OnOff);
//攝影機即時影像初紿化顯示
extern "C" _declspec(dllexport) void VI_CameraInit(BOOL color, BOOL move);
//Display 視窗顯示位置配置:move:1->移至小視窗用
extern "C" _declspec(dllexport) void VI_DisplayAlloc(CWnd* DisplayWindow, BOOL move);
//設置繪製用的透明圖層
extern "C" _declspec(dllexport) void VI_SetOverlayImage();
//清除透明圖層內容
extern "C" _declspec(dllexport) void VI_ClearOverlayImage();
//繪製視野十字
extern "C" _declspec(dllexport) void VI_DrawCross(BYTE mode, CWnd* DisplayWindow);
//繪製矩形框
extern "C" _declspec(dllexport) void VI_DrawBox(BYTE mode, CWnd* DisplayWindow, int width, int height);
//依模式顯示繪製
extern "C" _declspec(dllexport) void VI_DrawFOVFrame(BYTE mode, CWnd* DisplayWindow, int width, int height);

//---------------------------------------------------------------------------------------------------
// Pattern Match 相關函數
//---------------------------------------------------------------------------------------------------
//從視野矩形框建立PatternMatch Model
extern "C" _declspec(dllexport) void VI_CreateModelFromBox(BYTE mode, CWnd* DisplayWindow, void* MilModel, int width, int height);
//輸入搜尋起始角度、結束角度。計算搜尋原點、逆時針(positive正方向為逆)、順時針搜尋角度(negaitive負方向為順). 
extern "C" _declspec(dllexport) void VI_SearchAngleCalc(int startangle, int endangle, int &origin, int &delta);
//PatternMatch 參數設置
extern "C" _declspec(dllexport) void VI_SetPatternMatch(void* MilModel, BOOL accuracy, BOOL speed, BYTE score, int startangle, int endangle);
//設定PatternMatch 搜尋角度
extern "C" _declspec(dllexport) void VI_SetSearchAngle(void* MilModel, int startangle, int endangle);
//設定PatternMatch 搜尋範圍，視野中心位置，指定寬度、高度
extern "C" _declspec(dllexport) void VI_SetSearchRange(void* MilModel, int width, int height);
//搜尋標記，依模式功能
extern "C" _declspec(dllexport) BOOL VI_FindMaskProc(BYTE mode, void* MilModel, DOUBLE &X, DOUBLE &Y, DOUBLE &A);
//搜尋標記，然後繪製其位置
extern "C" _declspec(dllexport) BOOL VI_FindMaskDrawResult(CWnd* DisplayWindow, void* MilModel, DOUBLE &Score);
//搜尋標記，然後回傳其中心位置
extern "C" _declspec(dllexport) BOOL VI_FindMaskCenterPos(void* MilTargetImage, void* MilModel, DOUBLE &PosX, DOUBLE &PosY);
//搜尋標記，然後回傳與視野中心相對距離
extern "C" _declspec(dllexport) BOOL VI_FindMaskRelDist(void* MilTargetImage, void* MilModel, BOOL PixelOrRel, DOUBLE &DistX, DOUBLE &DistY, DOUBLE &Score);
//查找標記指令，回傳與視野中心相對實際距離
extern "C" _declspec(dllexport) BOOL VI_FindMask(void* MilModel, DOUBLE &DistX, DOUBLE &DistY);
//建立互動矩形框
extern "C" _declspec(dllexport) void VI_SetInteractiveBox(int width, int height);
//從互動矩形框建立PatternMatch Model 
extern "C" _declspec(dllexport) void VI_CreateModelFromInteractiveBox(void* MilModel, CString path, CString name);
//取消互動模式，移除互動矩形框
extern "C" _declspec(dllexport) void VI_RemoveInteractiveBox();
//載入 PatternMatch Model
extern "C" _declspec(dllexport) void VI_LoadModel(void* MilModel, CString path, CString name);
//儲存 PatternMatch Model
extern "C" _declspec(dllexport) void VI_SaveModel(void* MilModel, CString path, CString name);

//---------------------------------------------------------------------------------------------------
// 獲取影像相關函數
//---------------------------------------------------------------------------------------------------
//Capture 捕捉拍照 提供影像處理用
extern "C" _declspec(dllexport) void VI_Capture(BOOL full, int width, int height);
//GetImage 取得影像暫存，人機使用
extern "C" _declspec(dllexport) void VI_GetImage(void* MilImage, BOOL full, int width, int height);
//GetPicture 取得圖片檔案，並指定儲存路徑
extern "C" _declspec(dllexport) void VI_GetPicture(CString path, CString name, BOOL full, int width, int height);

//---------------------------------------------------------------------------------------------------
// MIL Free() 記憶體、資源釋放相關函數
//---------------------------------------------------------------------------------------------------
//PatternMatch Model Free
extern "C" _declspec(dllexport) void VI_ModelFree(void* MilModel);
//Image Buffer Free
extern "C" _declspec(dllexport) void VI_ImageBufferFree(void* MilImage);
//MIL Vision Free
extern "C" _declspec(dllexport) void VI_VisionFree();

//---------------------------------------------------------------------------------------------------
// 影像參數，相關函數
//---------------------------------------------------------------------------------------------------
//設定1個像素的單位
extern "C" _declspec(dllexport) void VI_SetOnePixelUnit(DOUBLE xUnit, DOUBLE yUnit);
//建立Pixel、Pulse轉換關係
extern "C" _declspec(dllexport) void VI_SetPixelPulseRelation(CWnd* DisplayWindow, void* MilModel, double LocatX1, double LocatY1, double LocatX2, double LocatY2, double &xUnit, double &yUnit);
////設定攝影機和點膠針頭之間的距離Camera-to-Needle(Tip) Offset 
extern "C" _declspec(dllexport) void VI_SetCameraToTipOffset(DOUBLE OffsetX, DOUBLE OffsetY);

//---------------------------------------------------------------------------------------------------
// 修正計算，相關函數
//---------------------------------------------------------------------------------------------------
//絕對座標位置修正
extern "C" _declspec(dllexport) void VI_CorrectLocation(long &PointX, long &PointY, long RefX, long RefY, double OffSetX, double OffSetY, double Andgle);
//工件旋轉角度計算
//兩對位點比對接收馬達位置計算向量夾角,回傳對位點1的offset及兩向量夾角
//1=兩對位點皆有找到，0=只找到一點
extern "C" _declspec(dllexport) void VI_FindModel(void *PicTemp, void *PicTemp1, void* Model, void* Model1, double LocatX, double LocatY, double LocatX1, double LocatY1, double &OffSetX, double &OffSetY, double &Angle);
//計算向量夾角
extern "C" _declspec(dllexport) double VI_AngleCount(double LocatX, double LocatY, double LocatX1, double LocatY1, double sumx, double sumy, double sumx1, double sumy1);
//計算向量夾角 (符號修改版)
extern "C" _declspec(dllexport) double VI_AngleOfRotationCalc(double LocatX1, double LocatY1, double LocatX2, double LocatY2, double OffsetX1, double OffsetY1, double  OffsetX2, double  OffsetY2);

//===================================================================================================
//------------------------------------------------------------------------------
// CVisionApp
// 這個類別的實作請參閱 Vision.cpp
//

class CVisionApp : public CWinApp
{
public:
	CVisionApp();

// 覆寫
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
