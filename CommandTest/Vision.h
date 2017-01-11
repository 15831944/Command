// Vision.h : Vision DLL 的主要標頭檔
//
#pragma once

#ifndef __AFXWIN_H__
	#error "對 PCH 包含此檔案前先包含 'stdafx.h'"
#endif

//MIL初始函數配置差異
//#define ProjectUse
#define TestUse

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
#ifdef ProjectUse
extern "C" _declspec(dllexport) void VI_CameraInit(CWnd* DisplayWindow, BOOL color, BOOL move);
#endif
#ifdef TestUse
extern "C" _declspec(dllexport) void VI_CameraInit(BOOL color, BOOL move);
#endif
//Display 視窗顯示位置配置:move:1->移至小視窗用
extern "C" _declspec(dllexport) void VI_DisplayAlloc(CWnd* DisplayWindow, BOOL move);
//顯示位置配置。搭配VI_VisionAlloc使用。用於初始化不配置CWnd* DisplayWindow
extern "C" _declspec(dllexport) void VI_DisplaySet(CWnd* DisplayWindow, BOOL move);
//顯示位置配置，含左上原點offset偏移控制
extern "C" _declspec(dllexport) void VI_DisplayPosSet(CWnd* DisplayWindow, DOUBLE OffsetX, DOUBLE OffsetY);
//即時影像縮小顯示,自動Fit顯示視窗
extern "C" _declspec(dllexport) void VI_DisplayFitWindow(CWnd* DisplayWindow, BOOL move, double objwidth, double objheight);
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
//影像來源畫面，放大與縮小
extern "C" _declspec(dllexport) void VI_DisplayZoom(CWnd* DisplayWindow, DOUBLE XFactor, DOUBLE YFactor, BOOL move, BOOL disp);

//---------------------------------------------------------------------------------------------------
// Pattern Match 相關函數
//---------------------------------------------------------------------------------------------------
//從視野矩形框建立PatternMatch Model
extern "C" _declspec(dllexport) void VI_CreateModelFromBox(BYTE mode, CWnd* DisplayWindow, void* MilModel, int width, int height);
//輸入搜尋起始角度、結束角度。計算搜尋原點、逆時針(positive正方向為逆)、順時針搜尋角度(negaitive負方向為順). 
extern "C" _declspec(dllexport) void VI_SearchAngleCalc(double startangle, double endangle, double &origin, double &delta);
//PatternMatch 參數設置
extern "C" _declspec(dllexport) void VI_SetPatternMatch(void* MilModel, BOOL accuracy, BOOL speed, BYTE score, double startangle, double endangle);
//設定PatternMatch 搜尋角度
extern "C" _declspec(dllexport) void VI_SetSearchAngle(void* MilModel, double startangle, double endangle);
//設定PatternMatch 搜尋範圍，視野中心位置，指定寬度、高度
extern "C" _declspec(dllexport) void VI_SetSearchRange(void* MilModel, int width, int height);
//搜尋標記，依模式功能
extern "C" _declspec(dllexport) BOOL VI_FindMarkProc(BYTE mode, void* MilModel, DOUBLE &X, DOUBLE &Y, DOUBLE &Angle, DOUBLE &Score);
//搜尋標記，然後繪製其位置
extern "C" _declspec(dllexport) BOOL VI_FindMarkDrawResult(CWnd* DisplayWindow, void* MilModel, DOUBLE &Score);
//搜尋標記，然後回傳其中心位置
extern "C" _declspec(dllexport) BOOL VI_FindMarkCenterPos(void* MilTargetImage, void* MilModel, DOUBLE &PosX, DOUBLE &PosY);
//搜尋標記，然後回傳與視野中心相對距離
extern "C" _declspec(dllexport) BOOL VI_FindMarkRelDist(void* MilModel, BOOL PixelOrRel, DOUBLE &DistX, DOUBLE &DistY, DOUBLE &Score);
//查找標記指令，回傳與視野中心相對實際距離
extern "C" _declspec(dllexport) BOOL VI_FindMark(void* MilModel, DOUBLE &DistX, DOUBLE &DistY);
//動態查找標記，CameraTrigger指令。
extern "C" _declspec(dllexport) BOOL VI_CameraTrigger(void* MilModel, long &MarkPointX, long &MarkPointY, long TriggerPointX, long TriggerPointY, DOUBLE &DistX, DOUBLE &DistY);
//建立互動矩形框: 影像重組圖 or 參考來源圖大小
extern "C" _declspec(dllexport) void VI_SetInteractiveBoxRefImg(BOOL Source, CString path, CString name);
//建立互動矩形框
extern "C" _declspec(dllexport) void VI_SetInteractiveBox(int width, int height);
//從互動矩形框建立PatternMatch Model 
extern "C" _declspec(dllexport) BOOL VI_CreateModelFromInteractiveBox(void* MilModel, CString path, CString name, DOUBLE objwidth, DOUBLE objheight);
//取得InteractiveBox互動矩形框位置資訊
extern "C" _declspec(dllexport) void VI_GetInteractiveBoxPos(int &PosX, int &PosY, int &Width, int &Height);
//取消互動模式，移除互動矩形框
extern "C" _declspec(dllexport) void VI_RemoveInteractiveBox();
//配置Model空間  (搭配VI_LoadModel使用)
extern "C" _declspec(dllexport) void VI_ModelAlloc(void* MilModel);
//載入 PatternMatch Model檔到原有配置好的Model空間 
extern "C" _declspec(dllexport) void VI_LoadModel(void* MilModel, CString path, CString name);
//配置Model空間並且載入PatternMatch Model檔 
extern "C" _declspec(dllexport) void VI_RestoreModel(void* MilModel, CString path, CString name);
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
//量測，資源釋放
extern "C" _declspec(dllexport) void VI_MmeasFree();
//MmeasTool All Free
extern "C" _declspec(dllexport) void VI_MeasureToolFree();
//點膠檢測 Free
extern "C" _declspec(dllexport) void VI_CircleBeadFree();

//---------------------------------------------------------------------------------------------------
// 影像參數，相關函數
//---------------------------------------------------------------------------------------------------
//設定1個像素的單位
extern "C" _declspec(dllexport) void VI_SetOnePixelUnit(DOUBLE xUnit, DOUBLE yUnit);
//設定影像像素和實際距離的關係 (Uniform)
extern "C" _declspec(dllexport) void VI_SetUniform(BOOL ImgOrCal, double xUnits, double yUnits);
//建立Pixel、Pulse轉換關係
extern "C" _declspec(dllexport) void VI_SetPixelPulseRelation(CWnd* DisplayWindow, void* MilModel, double LocatX1, double LocatY1, double LocatX2, double LocatY2, double &xUnit, double &yUnit);
////設定攝影機和點膠針頭之間的距離Camera-to-Needle(Tip) Offset 
extern "C" _declspec(dllexport) void VI_SetCameraToTipOffset(DOUBLE OffsetX, DOUBLE OffsetY);

//---------------------------------------------------------------------------------------------------
// 修正計算，相關函數
//---------------------------------------------------------------------------------------------------
//絕對座標位置修正
extern "C" _declspec(dllexport) void VI_CorrectLocation(long &PointX, long &PointY, long RefX, long RefY, double OffSetX, double OffSetY, double Andgle, BOOL Mode);
//工件旋轉角度計算
//兩對位點比對接收馬達位置計算向量夾角,回傳對位點1的offset及兩向量夾角
//1=兩對位點皆有找到，0=只找到一點
extern "C" _declspec(dllexport) void VI_FindModel(void *PicTemp, void *PicTemp1, void* Model, void* Model1, double LocatX, double LocatY, double LocatX1, double LocatY1, double &OffSetX, double &OffSetY, double &Angle);
//取得 Model檔的位置資訊
extern "C" _declspec(dllexport) void VI_GetModelPos(void* MilModel, double &PosX, double &PosY);
//計算向量夾角
extern "C" _declspec(dllexport) double VI_AngleCount(double LocatX, double LocatY, double LocatX1, double LocatY1, double sumx, double sumy, double sumx1, double sumy1);
//計算向量夾角 (符號修改版)
extern "C" _declspec(dllexport) double VI_AngleOfRotationCalc(double LocatX1, double LocatY1, double LocatX2, double LocatY2, double OffsetX1, double OffsetY1, double  OffsetX2, double  OffsetY2);

//---------------------------------------------------------------------------------------------------
// 滑鼠座標 相關函數
//---------------------------------------------------------------------------------------------------
//滑鼠座標事件 Callback Function Eable
extern "C" _declspec(dllexport)	void VI_MousePosFuncEable();
//Callback Function Disable
extern "C" _declspec(dllexport) void VI_MousePosFuncDisable();
//取得滑鼠座標位置
extern "C" _declspec(dllexport) BOOL VI_GetMousePos(double &PosX, double &PosY);

//---------------------------------------------------------------------------------------------------
// 量測相關函數
//---------------------------------------------------------------------------------------------------
//量測繪圖配置
extern "C" _declspec(dllexport) void VI_MmeasGraphicsAlloc();
//量測工具設置
extern "C" _declspec(dllexport) void VI_MeasureToolSetup();
//量測類型配置
extern "C" _declspec(dllexport) void VI_MeasureMarkerAlloc(BYTE MarkType);
//量測長度：量測點1設置
extern "C" _declspec(dllexport) void VI_SetPointMarker1(double PosX, double PosY);
//量測長度：量測點2設置
extern "C" _declspec(dllexport) void VI_SetPointMarker2(double PosX, double PosY);
//量測距離計算
extern "C" _declspec(dllexport) BOOL VI_MeasureCalculate();
//量測長度計算
extern "C" _declspec(dllexport) BOOL VI_MeasureLengthCalc(BYTE Type);
//繪製量測位置
extern "C" _declspec(dllexport) void VI_DrawMeasurePos();
//繪製量測長度
extern "C" _declspec(dllexport) void VI_DrawMeasureLength();
//繪製長度距離
extern "C" _declspec(dllexport)	void VI_DrawLengthDist(BYTE Type);
//量測圓：量測點1設置
extern "C" _declspec(dllexport) BOOL VI_SetCircleMarker1(BOOL BlackOrWhite, double PosX, double PosY, double OuterRadius);
//量測圓：量測點2設置
extern "C" _declspec(dllexport) BOOL VI_SetCircleMarker2(BOOL BlackOrWhite, double PosX, double PosY, double OuterRadius);
//量測圓直徑
extern "C" _declspec(dllexport) BOOL VI_MeasCircleDiameter(BOOL BlackOrWhite, double PosX, double PosY, double OuterRadius);
//量測圓心位置
extern "C" _declspec(dllexport) BOOL VI_MeasCircleCenter(BOOL BlackOrWhite, double PosX, double PosY, double OuterRadius, double MotorLocatX, double MotorLocatY);
//影像畫面，滑鼠點擊，觸碰式移動
extern "C" _declspec(dllexport) void VI_GetTouchMoveDist(DOUBLE TouchPosX, DOUBLE TouchPosY, DOUBLE &DistX, DOUBLE &DistY);

//---------------------------------------------------------------------------------------------------
// 陣列式樣版載入、設置、比對
//---------------------------------------------------------------------------------------------------
//載入陣列 Model
extern "C" _declspec(dllexport) void VI_LoadMatrixModel(void * Model[], CString Path, CString Name[], int Num);
//載入陣列 Model
extern "C" _declspec(dllexport) void VI_SetMultipleModel(void * Model[], BOOL accuracy, BOOL speed, BYTE score, double startangle, double endangle, int Num);
//一次比對陣列 Model
extern "C" _declspec(dllexport) BOOL VI_FindMatrixModel(void* Model[], int Num);
//釋放陣列 Model
extern "C" _declspec(dllexport) void VI_MatrixModelFree(void* Model[], int Num);

//---------------------------------------------------------------------------------------------------
// Bead Inspection 檢測
//---------------------------------------------------------------------------------------------------
//點膠檢測樣板建立
extern "C" _declspec(dllexport) void VI_CircleBeadTrain(double Diameter, double MaxOffset, BOOL WhiteOrBlack, double Threshold);
//點膠檢測驗證
extern "C" _declspec(dllexport) BOOL VI_CircleBeadVerify(BOOL UnitsEnable, double MaxOffset);

//---------------------------------------------------------------------------------------------------
// 影像拼接重組 Registration & 影像尋邊 Edge Finder 
//---------------------------------------------------------------------------------------------------
//影像拼接重組配置
extern "C" _declspec(dllexport) void VI_MosaicingImagesAlloc(int MaxCNT);
//影像拼接重組配置 (除錯測試用)
extern "C" _declspec(dllexport) void VI_MosaicAllocDebugUse(int MaxCNT);
//影像拼接重組，移動量設置
extern "C" _declspec(dllexport) void VI_MosaicingMoveSet(double FOVx, double FOVy, double FOVPercent, long &MoveX, long &MoveY);
//影像拼接重組，機台座標原點計算
extern "C" _declspec(dllexport) void VI_MosaicingImagesOriginCalc(long LocatX, long LocatY);
//影像拼接，重組數量計算
extern "C" _declspec(dllexport) BOOL VI_MosaicingImagesSizeCalc(long StartPointX, long StartPointY, long EndPointX, long EndPointY, int &MovXcnt, int &MovYcnt);
//記錄待拼接重組的影像
extern "C" _declspec(dllexport) void VI_MosaicingImagesCapture(long POsitionX, long PositionY);
//影像拼接重組，資源釋放
extern "C" _declspec(dllexport) void VI_MosaicingImagesFree(int MaxCNT);
//取得DXF檔，座標轉換參數
extern "C" _declspec(dllexport) void VI_GetMosaicingImagesDXFinfo(double &OriginX, double &OriginY, double &ScaleX, double &ScaleY);
//影像拼接重組處理
extern "C" _declspec(dllexport) BOOL VI_MosaicingImagesProcess(CString path, CString name);
//Registration Context、Result Free
extern "C" _declspec(dllexport) void VI_RegistrationFree();
//中止影像拼接重組計算
extern "C" _declspec(dllexport) void VI_MosaicingImagesProcessStop();
//影像拼接重組，手動裁切處理
extern "C" _declspec(dllexport) BOOL VI_SetCutMosaicingImages(CString path, CString name);
//拼接重組影像顯示
extern "C" _declspec(dllexport) void VI_MosaicingImagesDisplay(CWnd* DisplayWindow, CString path, CString name, BOOL move, double objwidth, double objheight);
//拼接重組影像顯示，資源釋放
extern "C" _declspec(dllexport) void VI_MosaicingImagesDisplayFree();
//影像拼接重組圖,二值化處理
extern "C" _declspec(dllexport) void VI_BinarizeMosaicImage(double Threshold);
//影像拼接重組圖,尋邊處理
extern "C" _declspec(dllexport) BOOL VI_EdgeProcessMosaicImage(CString path, CString name, BOOL type, BOOL WhiteOrBlack, double Threshold, double EdgelSize, BOOL SimpleEnable, double tolerance);
//載入影像拼接重組圖，尋邊處理
extern "C" _declspec(dllexport) BOOL VI_LoadMosaicImageEdgeFind(CString path, CString name, BOOL type, BOOL WhiteOrBlack, double Threshold, double EdgelSize, BOOL SimpleEnable, double tolerance);

//---------------------------------------------------------------------------------------------------
// 區域Bead檢測 (單點點膠直徑和塗膠線段膠寬) 
//---------------------------------------------------------------------------------------------------
//區域Bead檢測：單點點膠直徑，檢測訓練
extern "C" _declspec(dllexport) void VI_ImagesCircleBeadTrain(CString DotPath, CString DotName, CPoint DotPosition[], double Diameter, double MaxOffset, BOOL WhiteOrBlack, double Threshold, int DotPositionNum);
//區域Bead檢測：塗膠線段膠寬，檢測訓練
extern "C" _declspec(dllexport) void VI_ImagesLineBeadTrain(CString LinePath, CString LineName, CPoint LinePosition[], double LineWidth, double MaxOffset, BOOL WhiteOrBlack, int LinePositionNum);
//區域Bead檢測與驗證
extern "C" _declspec(dllexport) BOOL VI_AreaBeadVerify(CString ImagePath, CString ImageName, CString DotPath, CString DotName, CString LinePath, CString LineName, CString ResultImagePath, CString ResultImageName, int Mode);


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
