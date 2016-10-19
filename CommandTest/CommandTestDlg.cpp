
// CommandTestDlg.cpp : 實作檔
//

#include "stdafx.h"
#include "CommandTest.h"
#include "CommandTestDlg.h"
#include "afxdialogex.h"
#include "mcc.h"
#include "Default.h"
#include "Camera.h"
#include "Block.h"
#include "LaserDlg.h"
#include "PositionModify.h"
#include "LaserAdjust.h"
#include "LineContinuous.h"
#include "EmgDlg.h"
#include "Question.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CCommandTestDlg 對話方塊



CCommandTestDlg::CCommandTestDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_COMMANDTEST_DIALOG, pParent)
	, InputAuto(FALSE)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	//TODO:介面影像修改OFFSET在這
	TipOffset.x = 0;
	TipOffset.y = 0;
	LaserOffset.x = 0;
	LaserOffset.y = 0;
	LaserOffsetz = 0;
	HeightLaserZero = 0;
	CcdMode = FALSE;
	MaxRunNumber = 0;                                
	RunLoopNumber = 0;
	XNumber = 0;
	YNumber = 0;   
	BlockCount = 0;
	BlockStr = _T("");
	GlueInformation = FALSE;

	m_pDefaultDlg = NULL;
	m_pCameraDlg = NULL;
	m_pLaserDlg = NULL;
	m_pPositionModifyDlg = NULL;
	m_pLaserAdjustDlg = NULL;
	m_pLineContinuousDlg = NULL;
}

void CCommandTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_CommandList);
	DDX_Control(pDX, IDC_LIST2, m_ParamList);
	DDX_Check(pDX, IDC_CHECK1, InputAuto);
	DDX_Check(pDX, IDC_CHELOOPRUN, m_LoopRun);
}

BEGIN_MESSAGE_MAP(CCommandTestDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_START, &CCommandTestDlg::OnBnClickedStart)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_PAUSE, &CCommandTestDlg::OnBnClickedPause)
	ON_BN_CLICKED(IDC_STOP, &CCommandTestDlg::OnBnClickedStop)
	ON_BN_CLICKED(IDC_BTNHOME, &CCommandTestDlg::OnBnClickedBtnhome)
	ON_BN_CLICKED(IDC_BTNVIEW, &CCommandTestDlg::OnBnClickedBtnview)
	ON_BN_CLICKED(IDOK, &CCommandTestDlg::OnBnClickedOk)
	ON_COMMAND(IDM_INSERT, &CCommandTestDlg::OnInsert)
	ON_COMMAND(IDM_DELETE, &CCommandTestDlg::OnDelete)
	ON_NOTIFY(NM_RCLICK, IDC_LIST1, &CCommandTestDlg::OnNMRClickList1)
	ON_WM_DESTROY()

	ON_BN_CLICKED(IDC_BTNCOMMAND1, &CCommandTestDlg::OnBnClickedBtncommand1)
	ON_BN_CLICKED(IDC_BTNCOMMAND2, &CCommandTestDlg::OnBnClickedBtncommand2)
	ON_BN_CLICKED(IDC_BTNCOMMAND3, &CCommandTestDlg::OnBnClickedBtncommand3)
	ON_BN_CLICKED(IDC_BTNCOMMAND4, &CCommandTestDlg::OnBnClickedBtncommand4)
	ON_BN_CLICKED(IDC_BTNCOMMAND4_2, &CCommandTestDlg::OnBnClickedBtncommand4_2)
	ON_BN_CLICKED(IDC_BTNCOMMAND5, &CCommandTestDlg::OnBnClickedBtncommand5)
	ON_BN_CLICKED(IDC_BTNCOMMAND6, &CCommandTestDlg::OnBnClickedBtncommand6)
	ON_BN_CLICKED(IDC_BTNCOMMAND7, &CCommandTestDlg::OnBnClickedBtncommand7)
	ON_BN_CLICKED(IDC_BTNCOMMAND8, &CCommandTestDlg::OnBnClickedBtncommand8)
	ON_BN_CLICKED(IDC_BTNCOMMAND9, &CCommandTestDlg::OnBnClickedBtncommand9)
	ON_BN_CLICKED(IDC_BTNCOMMAND10, &CCommandTestDlg::OnBnClickedBtncommand10)
	ON_BN_CLICKED(IDC_BTNCOMMAND11, &CCommandTestDlg::OnBnClickedBtncommand11)
	ON_BN_CLICKED(IDC_BTNCOMMAND12, &CCommandTestDlg::OnBnClickedBtncommand12)
	ON_BN_CLICKED(IDC_BTNCOMMAND13, &CCommandTestDlg::OnBnClickedBtncommand13)    
	ON_BN_CLICKED(IDC_BTNCOMMAND14, &CCommandTestDlg::OnBnClickedBtncommand14)
	ON_BN_CLICKED(IDC_BTNCOMMAND15, &CCommandTestDlg::OnBnClickedBtncommand15)
	ON_BN_CLICKED(IDC_BTNCOMMAND16, &CCommandTestDlg::OnBnClickedBtncommand16)
	ON_BN_CLICKED(IDC_BTNCOMMAND17, &CCommandTestDlg::OnBnClickedBtncommand17)
	ON_BN_CLICKED(IDC_BTNCOMMAND18, &CCommandTestDlg::OnBnClickedBtncommand18)
	ON_BN_CLICKED(IDC_BTNCOMMAND19, &CCommandTestDlg::OnBnClickedBtncommand19)
	ON_BN_CLICKED(IDC_BTNCOMMAND19_2, &CCommandTestDlg::OnBnClickedBtncommand19_2)
	ON_BN_CLICKED(IDC_BTNCOMMAND20, &CCommandTestDlg::OnBnClickedBtncommand20)
	ON_BN_CLICKED(IDC_BTNCOMMAND21, &CCommandTestDlg::OnBnClickedBtncommand21)
	ON_BN_CLICKED(IDC_BTNCOMMAND22, &CCommandTestDlg::OnBnClickedBtncommand22)
	ON_BN_CLICKED(IDC_BTNCOMMAND23, &CCommandTestDlg::OnBnClickedBtncommand23)
	ON_BN_CLICKED(IDC_BTNCOMMAND24, &CCommandTestDlg::OnBnClickedBtncommand24)
	ON_BN_CLICKED(IDC_BTNCOMMAND25, &CCommandTestDlg::OnBnClickedBtncommand25)
    ON_BN_CLICKED(IDC_BTNCOMMAND25_1, &CCommandTestDlg::OnBnClickedBtncommand25_1)
	ON_BN_CLICKED(IDC_BTNCOMMAND26, &CCommandTestDlg::OnBnClickedBtncommand26)
	ON_BN_CLICKED(IDC_BTNCOMMAND27, &CCommandTestDlg::OnBnClickedBtncommand27)
	ON_BN_CLICKED(IDC_BTNCOMMAND28, &CCommandTestDlg::OnBnClickedBtncommand28)
	ON_BN_CLICKED(IDC_BTNCOMMAND29, &CCommandTestDlg::OnBnClickedBtncommand29)
	ON_BN_CLICKED(IDC_BTNCOMMAND30, &CCommandTestDlg::OnBnClickedBtncommand30)
	ON_BN_CLICKED(IDC_BTNCOMMAND31, &CCommandTestDlg::OnBnClickedBtncommand31)
	ON_BN_CLICKED(IDC_BTNCOMMAND32_0, &CCommandTestDlg::OnBnClickedBtncommand32_0)
	ON_BN_CLICKED(IDC_BTNCOMMAND32, &CCommandTestDlg::OnBnClickedBtncommand32)
	ON_BN_CLICKED(IDC_BTNCOMMAND33, &CCommandTestDlg::OnBnClickedBtncommand33)
	ON_BN_CLICKED(IDC_BTNCOMMAND34, &CCommandTestDlg::OnBnClickedBtncommand34)
	ON_BN_CLICKED(IDC_BTNCOMMAND35, &CCommandTestDlg::OnBnClickedBtncommand35)
	ON_BN_CLICKED(IDC_BTNCOMMAND36, &CCommandTestDlg::OnBnClickedBtncommand36)
	ON_BN_CLICKED(IDC_BTNCOMMAND37, &CCommandTestDlg::OnBnClickedBtncommand37)
	ON_BN_CLICKED(IDC_BTNCOMMAND38, &CCommandTestDlg::OnBnClickedBtncommand38)
	ON_BN_CLICKED(IDC_BTNCOMMAND40, &CCommandTestDlg::OnBnClickedBtncommand40)
	ON_BN_CLICKED(IDC_BTNCOMMAND41, &CCommandTestDlg::OnBnClickedBtncommand41)
	ON_BN_CLICKED(IDC_BTNCOMMAND43, &CCommandTestDlg::OnBnClickedBtncommand43)
	ON_BN_CLICKED(IDC_BTNCOMMAND44, &CCommandTestDlg::OnBnClickedBtncommand44)
	ON_BN_CLICKED(IDC_BTNCOMMAND45, &CCommandTestDlg::OnBnClickedBtncommand45)
	ON_BN_CLICKED(IDC_BTNCOMMAND46, &CCommandTestDlg::OnBnClickedBtncommand46)
	ON_BN_CLICKED(IDC_BTNCOMMAND47, &CCommandTestDlg::OnBnClickedBtncommand47)
	ON_BN_CLICKED(IDC_BTNCOMMAND48, &CCommandTestDlg::OnBnClickedBtncommand48)
	ON_BN_CLICKED(IDC_BTNCOMMAND49, &CCommandTestDlg::OnBnClickedBtncommand49)
	ON_BN_CLICKED(IDC_BTNCOMMAND50, &CCommandTestDlg::OnBnClickedBtncommand50)

	ON_BN_CLICKED(IDC_BTNDEFAULT, &CCommandTestDlg::OnBnClickedBtndefault)
	ON_BN_CLICKED(IDC_BTNVISION, &CCommandTestDlg::OnBnClickedBtnvision)
	ON_BN_CLICKED(IDC_BTNMODECHANGE, &CCommandTestDlg::OnBnClickedBtnmodechange)
	ON_BN_CLICKED(IDC_BTNCLEANCOUNT, &CCommandTestDlg::OnBnClickedBtncleancount)
	ON_BN_CLICKED(IDC_BTNMODEFYZ, &CCommandTestDlg::OnBnClickedBtnmodefyz)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST1, &CCommandTestDlg::OnNMDblclkList1)
	ON_BN_CLICKED(IDC_BTNLASER, &CCommandTestDlg::OnBnClickedBtnlaser)


	ON_BN_CLICKED(IDC_BTNALLOFFSET, &CCommandTestDlg::OnBnClickedBtnalloffset)
	ON_BN_CLICKED(IDC_BTNCOMMIT, &CCommandTestDlg::OnBnClickedBtncommit)
	ON_BN_CLICKED(IDC_BTNNOCOMMIT, &CCommandTestDlg::OnBnClickedBtnnocommit)
	ON_BN_CLICKED(IDC_BTNSAVE, &CCommandTestDlg::OnBnClickedBtnsave)
	ON_BN_CLICKED(IDC_BTNOPEN, &CCommandTestDlg::OnBnClickedBtnopen)
	ON_BN_CLICKED(IDC_BTNLOADDEMO, &CCommandTestDlg::OnBnClickedBtnloaddemo)
	ON_BN_CLICKED(IDC_BTNPRINTFLASER, &CCommandTestDlg::OnBnClickedBtnprintflaser)
	ON_BN_CLICKED(IDC_BTNMODIFY, &CCommandTestDlg::OnBnClickedBtnmodify)
	ON_BN_CLICKED(IDC_BTNPRINTCLINE, &CCommandTestDlg::OnBnClickedBtnprintcline)
	ON_COMMAND(IDM_MOVE, &CCommandTestDlg::OnMove)
	ON_WM_MOUSEACTIVATE()
	
	
END_MESSAGE_MAP()


// CCommandTestDlg 訊息處理常式
BOOL CCommandTestDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 設定此對話方塊的圖示。當應用程式的主視窗不是對話方塊時，
	// 框架會自動從事此作業
	SetIcon(m_hIcon, TRUE);			// 設定大圖示
	SetIcon(m_hIcon, FALSE);		// 設定小圖示

	/*終端控制台開啟*/
#ifdef PRINTF
	InitConsoleWindow();
	_cprintf("str = %s\n ", "Debug output goes to terminal\n");
#endif
	//命令列表
	m_CommandList.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
	m_CommandList.InsertColumn(0, _T("編號"), LVCFMT_CENTER, 36, -1);
	m_CommandList.InsertColumn(1, _T("Command"), LVCFMT_LEFT, 300, -1);

	DWORD dwStyle = m_CommandList.GetExtendedStyle();
	dwStyle |= LVS_EX_FULLROWSELECT;//選中某行使整行高亮（只適用與report風格的listctrl） 
	dwStyle |= LVS_EX_GRIDLINES;//格線（只適用與report風格的listctrl）
	//dwStyle |= LVS_SINGLESEL;
	m_CommandList.SetExtendedStyle(dwStyle); //設置擴展風格 
	//參數列表
	m_ParamList.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
	m_ParamList.InsertColumn(0, _T("名稱"), LVCFMT_CENTER, 100, -1);
	m_ParamList.InsertColumn(1, _T("參數值"), LVCFMT_LEFT, 150, -1);
	CStringArray StrTable;
	StrTable.Add(_T("點膠出膠時間")); StrTable.Add(_T("點膠停留時間"));
	StrTable.Add(_T("點膠回程距離")); StrTable.Add(_T("點膠低回程速度")); StrTable.Add(_T("點膠高回程速度"));
	StrTable.Add(_T("點膠加速度")); StrTable.Add(_T("點膠驅動速度"));
	StrTable.Add(_T("線段移動前延遲")); StrTable.Add(_T("線段設置距離")); StrTable.Add(_T("線段節點時間")); StrTable.Add(_T("線段停留時間")); StrTable.Add(_T("線段關機距離")); StrTable.Add(_T("線段關機延遲"));
	StrTable.Add(_T("線段返回類型")); StrTable.Add(_T("線段返回低速")); StrTable.Add(_T("線段返回高度")); StrTable.Add(_T("線段返回長度")); StrTable.Add(_T("線段返回高速"));
	StrTable.Add(_T("線段加速度")); StrTable.Add(_T("線段驅動速度"));
	StrTable.Add(_T("Z軸回升距離")); StrTable.Add(_T("Z軸回升型態"));
	StrTable.Add(_T("停駐點X，Y，Z")); StrTable.Add(_T("排膠開關")); StrTable.Add(_T("排膠等待時間")); StrTable.Add(_T("排膠時間")); StrTable.Add(_T("排膠後停留時間"));
	StrTable.Add(_T("動作總數")); StrTable.Add(_T("影像OffsetX")); StrTable.Add(_T("影像OffsetY")); StrTable.Add(_T("影像Angle"));
	StrTable.Add(_T("對位點1X")); StrTable.Add(_T("對位點1Y")); StrTable.Add(_T("對位點1offsetX")); StrTable.Add(_T("對位點1offsetY"));
	StrTable.Add(_T("對位點2X")); StrTable.Add(_T("對位點2Y")); StrTable.Add(_T("對位點2offsetX")); StrTable.Add(_T("對位點2offsetY"));
	StrTable.Add(_T("最終X")); StrTable.Add(_T("最終Y")); 
	StrTable.Add(_T("影像紀錄表計數")); StrTable.Add(_T("雷射紀錄表計數"));
	StrTable.Add(_T("虛擬座標X")); StrTable.Add(_T("虛擬座標Y")); StrTable.Add(_T("虛擬座標Z"));
	StrTable.Add(_T("雷射測高數值"));
	for (int i = 0; i < StrTable.GetSize(); i++) {
		m_ParamList.InsertItem(i, NULL);
		m_ParamList.SetItemText(i, 0, StrTable[i]);
		m_ParamList.SetItemText(i, 1, 0);
	}
	//TODO:參數修改在這
	//原點復歸參數
	LoadDefault();
	//載入參數檔案
	LoadParameter();
	SetTimer(1, 500, NULL);
	//軸卡運動開啟
#ifdef MOVE
	MO_Open(1);//軸卡開啟
	MO_SetHardLim(7, 1);//極限開啟
	MO_SetDecOK(1);//開啟減速有效
	MO_InterruptCase(1, 1, a.m_Action.MoInterrupt,a.m_Action.pAction);//中斷開啟
	MO_SetSoftLim(7, 1);//軟極限開啟
	MO_SetCompSoft(1, -47000, -62000, -10000);
	MO_SetCompSoft(0, 350000, 350000, 80000); //0, 150000, 190000, 80000
	a.m_Action.LA_SetInit();//雷射開啟
	OnBnClickedBtnhome();
#endif
#ifdef VI
	VI_CameraInit(0, 1);
#endif
	return TRUE;  // 傳回 TRUE，除非您對控制項設定焦點
}
// 如果將最小化按鈕加入您的對話方塊，您需要下列的程式碼，
// 以便繪製圖示。對於使用文件/檢視模式的 MFC 應用程式，
// 框架會自動完成此作業。
void CCommandTestDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 繪製的裝置內容

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 將圖示置中於用戶端矩形
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 描繪圖示
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}
// 當使用者拖曳最小化視窗時，
// 系統呼叫這個功能取得游標顯示。
HCURSOR CCommandTestDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}
/*******************************************************************************************主要功能**********************************************************/
/*開始*/
void CCommandTestDlg::OnBnClickedStart()
{	
	if (m_LoopRun)
	{
		/*列表停用*/
		m_CommandList.EnableWindow(FALSE);
		/*列表改為單選*/
		DWORD dwStyle = m_CommandList.GetExtendedStyle();
		dwStyle |= LVS_SHOWSELALWAYS;
		m_CommandList.SetExtendedStyle(dwStyle); //設置擴展風格

		CString path;
		GetModuleFileName(NULL, path.GetBufferSetLength(MAX_PATH + 1), MAX_PATH);
		path.ReleaseBuffer();
		int pos = path.ReverseFind('\\');
		path = path.Left(pos) + _T("\\Temp\\");
		LPTSTR lpszText = new TCHAR[path.GetLength() + 1];
		lstrcpy(lpszText, path);
		a.VisionDefault.VisionFile.ModelPath = lpszText;
		/*循環運行*/
		a.RunLoop(RunLoopNumber);
	}
	else
	{
		//判斷目前運行次數是否大於 最大運行次數限制
		if (a.RunLoopData.MaxRunNumber == -1 || a.RunStatusRead.FinishProgramCount < a.RunLoopData.MaxRunNumber)
		{
			/*列表停用*/
			m_CommandList.EnableWindow(FALSE);
			/*列表改為單選*/
			DWORD dwStyle = m_CommandList.GetExtendedStyle();
			dwStyle |= LVS_SHOWSELALWAYS;
			m_CommandList.SetExtendedStyle(dwStyle); //設置擴展風格
													 /*設置Model當前目錄*/
			CString path;
			GetModuleFileName(NULL, path.GetBufferSetLength(MAX_PATH + 1), MAX_PATH);
			path.ReleaseBuffer();
			int pos = path.ReverseFind('\\');
			path = path.Left(pos) + _T("\\Temp\\");
			LPTSTR lpszText = new TCHAR[path.GetLength() + 1];
			lstrcpy(lpszText, path);
			a.VisionDefault.VisionFile.ModelPath = lpszText;
			/*運行*/
			a.Run();
		}
	}
}
/*暫停&繼續*/
void CCommandTestDlg::OnBnClickedPause()
{
	CString StrBuff;
	GetDlgItemText(IDC_PAUSE, StrBuff);
	if (StrBuff == L"Pause")
	{
		if (a.Pause())
		{
			SetDlgItemText(IDC_PAUSE, L"Continue");
		}	
	}
	else
	{
		if (a.Continue())
		{
			SetDlgItemText(IDC_PAUSE, L"Pause");
		}	
	}
}
/*停止*/
void CCommandTestDlg::OnBnClickedStop()
{
	a.Stop();
	SetDlgItemText(IDC_PAUSE, L"Pause");
}
/*清除陣列*/
void CCommandTestDlg::OnBnClickedOk()
{
	a.CommandMemory.clear(); 
	Insert = FALSE;//取消插入
	//TODO::為了DEMO加入
	a.DemoTemprarilySwitch = FALSE;
	ListRefresh(NULL);
}
/*原點賦歸*/
void CCommandTestDlg::OnBnClickedBtnhome()
{
	a.Home(CcdMode);
}
/*View*/
void CCommandTestDlg::OnBnClickedBtnview()
{
	/*設置Model當前目錄*/
	CString path;
	GetModuleFileName(NULL, path.GetBufferSetLength(MAX_PATH + 1), MAX_PATH);
	path.ReleaseBuffer();
	int pos = path.ReverseFind('\\');
	path = path.Left(pos) + _T("\\Temp\\");
	LPTSTR lpszText = new TCHAR[path.GetLength() + 1];
	lstrcpy(lpszText, path);
	a.VisionDefault.VisionFile.ModelPath = lpszText;
	/*運行*/
	a.View(CcdMode);
}
/*清空運行次數*/
void CCommandTestDlg::OnBnClickedBtncleancount()
{
	a.RunStatusRead.FinishProgramCount = 0;	
}
/*刷新*/
void CCommandTestDlg::OnTimer(UINT_PTR nIDEvent)
{
	CString XYZlocation,StrBuff,FinishCountBuff;
	LONG DataArray[49] = { a.DispenseDotSet.GlueOpenTime, a.DispenseDotSet.GlueCloseTime,
		a.DispenseDotEnd.RiseDistance,a.DispenseDotEnd.RiseLowSpeed,a.DispenseDotEnd.RiseHightSpeed,
		a.DotSpeedSet.AccSpeed,a.DotSpeedSet.EndSpeed,
		a.DispenseLineSet.BeforeMoveDelay, a.DispenseLineSet.BeforeMoveDistance, a.DispenseLineSet.NodeTime, a.DispenseLineSet.StayTime, a.DispenseLineSet.ShutdownDistance, a.DispenseLineSet.ShutdownDelay,
		a.DispenseLineEnd.Type, a.DispenseLineEnd.HighSpeed, a.DispenseLineEnd.LowSpeed, a.DispenseLineEnd.Width, a.DispenseLineEnd.Height,
		a.LineSpeedSet.AccSpeed, a.LineSpeedSet.EndSpeed,
		a.ZSet.ZBackHeight,a.ZSet.ZBackType,
		a.GlueData.ParkPositionData.X,a.GlueData.ParkPositionData.Y,a.GlueData.ParkPositionData.Z,a.GlueData.GlueAuto,a.GlueData.GlueWaitTime,a.GlueData.GlueTime,a.GlueData.GlueStayTime,
		a.ActionCount,(LONG)a.VisionOffset.OffsetX,(LONG)a.VisionOffset.OffsetY,(LONG)a.VisionOffset.Angle,
		a.FiducialMark1.Point.X, a.FiducialMark1.Point.Y,(LONG)a.FiducialMark1.OffsetX,(LONG)a.FiducialMark1.OffsetY,
		a.FiducialMark2.Point.X, a.FiducialMark2.Point.Y,(LONG)a.FiducialMark2.OffsetX,(LONG)a.FiducialMark2.OffsetY,
		a.FinalWorkCoordinateData.X,a.FinalWorkCoordinateData.Y ,a.VisionCount,a.LaserCount,
		a.VirtualCoordinateData.X,a.VirtualCoordinateData.Y,a.VirtualCoordinateData.Z,
		a.LaserData.LaserMeasureHeight
	}; 
	int ArrayCount = 0;
	for (int i = 0; i < 47; i++)
	{   
		if (i == 22)
		{
			StrBuff.Format(_T("%d,%d,%d"), DataArray[22], DataArray[23], DataArray[24]);
			ArrayCount += 3;
		}
		else
		{
			StrBuff.Format(_T("%d"), DataArray[ArrayCount]);
			ArrayCount++;
		}    
		m_ParamList.SetItemText(i, 1, StrBuff);
	}
	/*手臂與出膠狀態*/
	#ifdef MOVE
		XYZlocation.Format(_T("機械手臂位置(X:%d,Y:%d,Z:%d)\tGlueStatus:%d\t驅動速度:%d\t加速度:%d\t加工時間:%f(s)"),
			MO_ReadLogicPosition(0), MO_ReadLogicPosition(1), MO_ReadLogicPosition(2), MO_ReadGumming(),
			MO_ReadSpeed(0),MO_ReadAccDec(0),a.RunStatusRead.RunTotalTime);
	#endif 
	SetDlgItemText(IDC_ARMSTATUS, XYZlocation);
	/*程序運行狀態*/
	if (a.RunStatusRead.RunStatus == 0)
	{
		SetDlgItemText(IDC_RUNSTATUS, _T("程序運行狀態:未運行"));
		if (StrBuff == L"Continue")
		{
			SetDlgItemText(IDC_PAUSE, L"Pause");
		}
	}
	else if (a.RunStatusRead.RunStatus == 2)
	{
		GetDlgItemText(IDC_PAUSE, StrBuff);
		if (StrBuff != L"Continue")
		{
			SetDlgItemText(IDC_PAUSE, L"Continue");
			SetDlgItemText(IDC_RUNSTATUS, _T("程序運行狀態:暫停中"));
		}
	}
	else if (a.RunStatusRead.RunStatus == 1)
	{
		SetDlgItemText(IDC_RUNSTATUS, _T("程序運行狀態:運行中"));
	}
	/*程序計數*/
	FinishCountBuff.Format(_T("完整程序運行次數:%d"), a.RunStatusRead.FinishProgramCount);
	SetDlgItemText(IDC_FINISHCOUNT, FinishCountBuff);
	if (a.RunStatusRead.RunStatus != 0)
	{
		m_CommandList.SetItemState(a.RunStatusRead.CurrentRunCommandNum, LVIS_SELECTED, LVIS_SELECTED);//將工作點設為高亮
		m_CommandList.SetSelectionMark(a.RunStatusRead.CurrentRunCommandNum);//改變選中索引
		m_CommandList.EnsureVisible(a.RunStatusRead.CurrentRunCommandNum, FALSE);//使List中一項可見(如滾動條向下滾)
	}
	else
	{
		DWORD dwStyle = m_CommandList.GetExtendedStyle();
		dwStyle |= LVS_SHAREIMAGELISTS;
		m_CommandList.SetExtendedStyle(dwStyle); //設置擴展風格
		m_CommandList.EnableWindow(TRUE);
	}
	UpdateData(TRUE);
	//判斷是否為CCD模式更新OFFSET
	if (CcdMode)
	{
		OffsetX = TipOffset.x;
		OffsetY = TipOffset.y;
	}
	else
	{
		OffsetX = 0;
		OffsetY = 0;
	}
	CDialogEx::OnTimer(nIDEvent);
}
/*刷新列表*/
void CCommandTestDlg::ListRefresh(BOOL ScrollBarRefresh) {
	CString StrBuff;
	m_CommandList.DeleteAllItems();
	int nCount = a.CommandMemory.size();
	if (!ScrollBarRefresh)
	{
		for (int i = 0; i < nCount; i++) {
			m_CommandList.InsertItem(i, NULL);
			(i>8) ? StrBuff.Format(_T("0%d"), i + 1) : StrBuff.Format(_T("00%d"), i + 1);
			m_CommandList.SetItemText(i, 0, StrBuff);
			m_CommandList.SetItemText(i, 1, a.CommandMemory.at(i));
		}
		m_CommandList.EnsureVisible(InsertNum, FALSE);//使List中一項可見(如滾動條向下滾)
	}
	else
	{
		//測試SubProgram時使用
		/*for (UINT i = 0; i < a.Command.size(); i++)
		{
			for (UINT j = 0; j<a.Command.at(i).size(); j++)
			{
				m_CommandList.InsertItem(j, NULL);
				(j>8) ? StrBuff.Format(_T("0%d"), j + 1) : StrBuff.Format(_T("00%d"), j + 1);
				m_CommandList.SetItemText(j, 0, StrBuff);
				m_CommandList.SetItemText(j, 1, a.Command.at(i).at(j));
			}
		}
		m_CommandList.InsertItem(0, NULL);
		StrBuff.Format(_T("%d"), a.Command.at(0).size());
		m_CommandList.SetItemText(0, 1, StrBuff);
		m_CommandList.InsertItem(1, NULL);
		StrBuff.Format(_T("%d"), a.Command.at(1).size());
		m_CommandList.SetItemText(1, 1, StrBuff);*/
	}
}
/*列表中按兩下左鍵*/
void CCommandTestDlg::OnNMDblclkList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	CListCtrl *CList = (CListCtrl *)GetDlgItem(IDC_LIST1);
	NM_LISTVIEW  *pEditCtrl = (NM_LISTVIEW *)pNMHDR;
	if (pEditCtrl->iItem != -1 || pEditCtrl->iSubItem != 0)
	{
		if (CommandResolve(m_CommandList.GetItemText(pEditCtrl->iItem, pEditCtrl->iSubItem),0) == L"StepRepeatX" ||
			CommandResolve(m_CommandList.GetItemText(pEditCtrl->iItem, pEditCtrl->iSubItem), 0) == L"StepRepeatY")
		{
			XNumber = _ttol(CommandResolve(m_CommandList.GetItemText(pEditCtrl->iItem, pEditCtrl->iSubItem), 3));
			YNumber = _ttol(CommandResolve(m_CommandList.GetItemText(pEditCtrl->iItem, pEditCtrl->iSubItem), 4));
			CBlock DlgBlock;
			if (DlgBlock.DoModal() == IDOK) {
				CString StrBuff;
				StrBuff.Format(_T("%d"), BlockCount);
				a.CommandMemory.at(pEditCtrl->iItem) = CommandResolve(a.CommandMemory.at(pEditCtrl->iItem), 0) + _T(",") +
					CommandResolve(a.CommandMemory.at(pEditCtrl->iItem), 1) + _T(",") +
					CommandResolve(a.CommandMemory.at(pEditCtrl->iItem), 2) + _T(",") +
					CommandResolve(a.CommandMemory.at(pEditCtrl->iItem), 3) + _T(",") +
					CommandResolve(a.CommandMemory.at(pEditCtrl->iItem), 4) + _T(",") +
					CommandResolve(a.CommandMemory.at(pEditCtrl->iItem), 5) + _T(",") +
					CommandResolve(a.CommandMemory.at(pEditCtrl->iItem), 6) + _T(",") +
					_T("1,") + StrBuff + BlockStr;
				ListRefresh(NULL);
			}
		}
	}
	*pResult = 0;
}
/*列表點下右鍵*/
void CCommandTestDlg::OnNMRClickList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	CListCtrl *CList = (CListCtrl *)GetDlgItem(IDC_LIST1);
	CMenu menu, *pSubMenu;
	CPoint CurPnt;
	int ItemCount = m_CommandList.GetItemCount();//獲取項目總數
	NM_LISTVIEW  *pEditCtrl = (NM_LISTVIEW *)pNMHDR;
	if (pEditCtrl->iItem != -1 || pEditCtrl->iSubItem != 0) {
		menu.LoadMenu(IDR_MENU1);//加入菜單
		pSubMenu = menu.GetSubMenu(0);
		GetCursorPos(&CurPnt);
		pSubMenu->TrackPopupMenu(TPM_LEFTALIGN, CurPnt.x, CurPnt.y, this);//點右鍵出現的菜單位置
	}
	*pResult = 0;
}
/*命令插入*/
void CCommandTestDlg::OnInsert()
{
	if (!Insert)
	{
		int istat = m_CommandList.GetSelectionMark();//獲取選擇的項
		m_CommandList.InsertItem(istat, NULL);
		Insert = TRUE;
		InsertNum = istat;
	}  
}
/*命令刪除*/
void CCommandTestDlg::OnDelete()
{
	if (!Insert)
	{
		int istat = m_CommandList.GetSelectionMark();//獲取選擇的項
		a.CommandMemory.erase(a.CommandMemory.begin() + istat);
		ListRefresh(NULL);
	}
}
/*移動至此命令*/
void CCommandTestDlg::OnMove()
{
	if (!Insert)
	{
#ifdef MOVE
		int istat = m_CommandList.GetSelectionMark();
		if (CommandResolve(a.CommandMemory.at(istat), 0) == L"Dot" ||
			CommandResolve(a.CommandMemory.at(istat), 0) == L"LineStart" ||
			CommandResolve(a.CommandMemory.at(istat), 0) == L"LinePassing" ||
			CommandResolve(a.CommandMemory.at(istat), 0) == L"LineEnd" ||
			CommandResolve(a.CommandMemory.at(istat), 0) == L"ArcPoint" ||
			CommandResolve(a.CommandMemory.at(istat), 0) == L"VirtualPoint" || 
			CommandResolve(a.CommandMemory.at(istat), 0) == L"StopPoint" || 
			CommandResolve(a.CommandMemory.at(istat), 0) == L"WaitPoint" ||
			CommandResolve(a.CommandMemory.at(istat), 0) == L"FindMark" ||
			CommandResolve(a.CommandMemory.at(istat), 0) == L"FiducialMark"||
			CommandResolve(a.CommandMemory.at(istat), 0) == L"CameraTrigger" )
		{

			MO_Do3DLineMove(_ttol(CommandResolve(a.CommandMemory.at(istat), 1)) - OffsetX - MO_ReadLogicPosition(0),
				_ttol(CommandResolve(a.CommandMemory.at(istat), 2)) - OffsetY - MO_ReadLogicPosition(1),
				_ttol(CommandResolve(a.CommandMemory.at(istat), 3)) - MO_ReadLogicPosition(2), 80000, 1000000, 5000);
		}
		else if (CommandResolve(a.CommandMemory.at(istat), 0) == L"CirclePoint")
		{
			if (MessageBox(L"選擇Yes則移動至第一點，選擇NO則移動至第二點。", L"提示", MB_YESNOCANCEL) == IDYES)
			{
				MO_Do3DLineMove(_ttol(CommandResolve(a.CommandMemory.at(istat), 1)) - OffsetX - MO_ReadLogicPosition(0),
					_ttol(CommandResolve(a.CommandMemory.at(istat), 2)) - OffsetY - MO_ReadLogicPosition(1),
					_ttol(CommandResolve(a.CommandMemory.at(istat), 3)) - MO_ReadLogicPosition(2), 80000, 1000000, 5000);
			}
			else
			{
				MO_Do3DLineMove(_ttol(CommandResolve(a.CommandMemory.at(istat), 4)) - OffsetX - MO_ReadLogicPosition(0),
					_ttol(CommandResolve(a.CommandMemory.at(istat), 5)) - OffsetY - MO_ReadLogicPosition(1),
					_ttol(CommandResolve(a.CommandMemory.at(istat), 6)) - MO_ReadLogicPosition(2), 80000, 1000000, 5000);
			}
		}
		else if (CommandResolve(a.CommandMemory.at(istat), 0) == L"FillArea")
		{
			if (MessageBox( L"選擇Yes則移動至第一點，選擇NO則移動至第二點。",L"提示", MB_YESNOCANCEL) == IDYES)
			{
				MO_Do3DLineMove(_ttol(CommandResolve(a.CommandMemory.at(istat), 4)) - OffsetX - MO_ReadLogicPosition(0),
					_ttol(CommandResolve(a.CommandMemory.at(istat), 5)) - OffsetY - MO_ReadLogicPosition(1),
					_ttol(CommandResolve(a.CommandMemory.at(istat), 6)) - MO_ReadLogicPosition(2), 80000, 1000000, 5000);
			}
			else
			{
				MO_Do3DLineMove(_ttol(CommandResolve(a.CommandMemory.at(istat), 7)) - OffsetX - MO_ReadLogicPosition(0),
					_ttol(CommandResolve(a.CommandMemory.at(istat), 8)) - OffsetY - MO_ReadLogicPosition(1),
					_ttol(CommandResolve(a.CommandMemory.at(istat), 9)) - MO_ReadLogicPosition(2), 80000, 1000000, 5000);
			}
		}   
		else if (CommandResolve(a.CommandMemory.at(istat), 0) == L"LaserHeight")
		{

			MO_Do3DLineMove(_ttol(CommandResolve(a.CommandMemory.at(istat), 2)) - OffsetX - MO_ReadLogicPosition(0),
				_ttol(CommandResolve(a.CommandMemory.at(istat), 3)) - OffsetY - MO_ReadLogicPosition(1),
				a.m_Action.g_HeightLaserZero - MO_ReadLogicPosition(2), 80000, 1000000, 5000);
		}
		else if (CommandResolve(a.CommandMemory.at(istat), 0) == L"LaserDetect")
		{
			if (MessageBox(L"選擇Yes則移動至第一點，選擇NO則移動至第二點。", L"提示", MB_YESNOCANCEL) == IDYES)
			{
				MO_Do3DLineMove(_ttol(CommandResolve(a.CommandMemory.at(istat), 2)) - OffsetX - MO_ReadLogicPosition(0),
					_ttol(CommandResolve(a.CommandMemory.at(istat), 3)) - OffsetY - MO_ReadLogicPosition(1),
					a.m_Action.g_HeightLaserZero - MO_ReadLogicPosition(2), 80000, 1000000, 5000);
			}
			else
			{
				MO_Do3DLineMove(_ttol(CommandResolve(a.CommandMemory.at(istat), 4)) - OffsetX - MO_ReadLogicPosition(0),
					_ttol(CommandResolve(a.CommandMemory.at(istat), 5)) - OffsetY - MO_ReadLogicPosition(1),
					a.m_Action.g_HeightLaserZero - MO_ReadLogicPosition(2), 80000, 1000000, 5000);
			}
		}
#endif
	}
}
/*******************************************************************************************外部功能**********************************************************/
/*Default設置*/
void CCommandTestDlg::OnBnClickedBtndefault()
{
	if (m_pDefaultDlg == NULL)
	{
		m_pDefaultDlg = new CDefault();
		m_pDefaultDlg->Create(IDD_DIALOG1, this);
		m_pDefaultDlg->ShowWindow(SW_SHOW);
	}
	else
	{
		((CDefault*)m_pDefaultDlg)->DestroyWindow();
		free(m_pDefaultDlg);
		m_pDefaultDlg = new CDefault();
		m_pDefaultDlg->Create(IDD_DIALOG1, this);
		m_pDefaultDlg->ShowWindow(SW_SHOW);
	}
}
/*影像操作*/
void CCommandTestDlg::OnBnClickedBtnvision()
{
	if (m_pCameraDlg == NULL)
	{
		m_pCameraDlg = new CCamera();
		m_pCameraDlg->Create(IDD_DIALOG2, this);
		m_pCameraDlg->ShowWindow(SW_SHOW);
	}
	else
	{
		if (::IsWindow(((CCamera*)m_pCameraDlg)->m_hWnd))//判斷視窗是否有銷毀
		{
			((CCamera*)m_pCameraDlg)->DestroyWindow();
		}
		free(m_pCameraDlg);
		m_pCameraDlg = new CCamera();
		m_pCameraDlg->Create(IDD_DIALOG2, this);
		m_pCameraDlg->ShowWindow(SW_SHOW);
	}
}
/*雷射操作*/
void CCommandTestDlg::OnBnClickedBtnlaser()
{
	if (m_pLaserDlg == NULL)
	{
		m_pLaserDlg = new CLaserDlg();
		m_pLaserDlg->Create(IDD_DIALOG6, this);
		m_pLaserDlg->ShowWindow(SW_SHOW);
	}
	else
	{
		((CCamera*)m_pLaserDlg)->DestroyWindow();
		free(m_pLaserDlg);
		m_pLaserDlg = new CLaserDlg();
		m_pLaserDlg->Create(IDD_DIALOG6, this);
		m_pLaserDlg->ShowWindow(SW_SHOW);
	}
}
/*模式切換*/
void CCommandTestDlg::OnBnClickedBtnmodechange()
{
	CString StrBuff;
	GetDlgItemText(IDC_BTNMODECHANGE, StrBuff);
	if (StrBuff == L"切換CCD模式")
	{
		CcdMode = TRUE;
		OffsetX = TipOffset.x;
		OffsetY = TipOffset.y;
		SetDlgItemText(IDC_BTNMODECHANGE, _T("切換Tip模式"));
	}
	else
	{
		CcdMode = FALSE;
		OffsetX = 0;
		OffsetY = 0;
		SetDlgItemText(IDC_BTNMODECHANGE, _T("切換CCD模式"));
	}
}
/*修改點線段Z值*/
void CCommandTestDlg::OnBnClickedBtnmodefyz()
{
	CString StrBuff;
	for (UINT i = 0; i < a.CommandMemory.size(); i++)
	{
		if (CommandResolve(a.CommandMemory.at(i), 0) == L"Dot" ||
			CommandResolve(a.CommandMemory.at(i), 0) == L"LineStart" ||
			CommandResolve(a.CommandMemory.at(i), 0) == L"LinePassing" ||
			CommandResolve(a.CommandMemory.at(i), 0) == L"LineEnd" ||
			CommandResolve(a.CommandMemory.at(i), 0) == L"ArcPoint")
		{
			StrBuff.Format(_T(",%d,%d,%d"), _ttol(CommandResolve(a.CommandMemory.at(i), 1)), _ttol(CommandResolve(a.CommandMemory.at(i), 2)), GetDlgItemInt(IDC_EDITPARAM1));
			a.CommandMemory.at(i) = CommandResolve(a.CommandMemory.at(i), 0) + StrBuff;
		}
		else if (CommandResolve(a.CommandMemory.at(i), 0) == L"VirtualPoint" ||
			CommandResolve(a.CommandMemory.at(i), 0) == L"StopPoint")
		{
			StrBuff.Format(_T(",%d,%d,%d"), _ttol(CommandResolve(a.CommandMemory.at(i), 1)), _ttol(CommandResolve(a.CommandMemory.at(i), 2)), GetDlgItemInt(IDC_EDITPARAM1), _ttol(CommandResolve(a.CommandMemory.at(i), 4)));
			a.CommandMemory.at(i) = CommandResolve(a.CommandMemory.at(i), 0) + StrBuff;
		}
		else if (CommandResolve(a.CommandMemory.at(i), 0) == L"WaitPoint")
		{
			StrBuff.Format(_T(",%d,%d,%d"), _ttol(CommandResolve(a.CommandMemory.at(i), 1)), _ttol(CommandResolve(a.CommandMemory.at(i), 2)), GetDlgItemInt(IDC_EDITPARAM1), _ttol(CommandResolve(a.CommandMemory.at(i), 4)), _ttol(CommandResolve(a.CommandMemory.at(i), 5)));
			a.CommandMemory.at(i) = CommandResolve(a.CommandMemory.at(i), 0) + StrBuff;
		}
		else if (CommandResolve(a.CommandMemory.at(i), 0) == L"CirclePoint")
		{
			StrBuff.Format(_T(",%d,%d,%d,%d,%d,%d"), _ttol(CommandResolve(a.CommandMemory.at(i), 1)), _ttol(CommandResolve(a.CommandMemory.at(i), 2)), GetDlgItemInt(IDC_EDITPARAM1),
				_ttol(CommandResolve(a.CommandMemory.at(i), 4)), _ttol(CommandResolve(a.CommandMemory.at(i), 5)), GetDlgItemInt(IDC_EDITPARAM1));
			a.CommandMemory.at(i) = CommandResolve(a.CommandMemory.at(i), 0) + StrBuff;
		}
	}
	ListRefresh(NULL);
}
//關閉程式視窗銷毀事件
void CCommandTestDlg::OnDestroy()
{
	CDialogEx::OnDestroy();
#ifdef VI
	if (m_pCameraDlg != NULL)
	{
		if (::IsWindow(((CCamera*)m_pCameraDlg)->m_hWnd))
		{
			((CCamera*)m_pCameraDlg)->DestroyWindow();
		}
	}
	VI_VisionFree();
#endif
	//儲存參數檔案
	SaveParameter();
	SaveDefault();
}
//全部位置偏移
void CCommandTestDlg::OnBnClickedBtnalloffset()
{
	for (UINT i = 0; i < a.CommandMemory.size(); i++)
	{
		if (CommandResolve(a.CommandMemory.at(i), 0) == L"Dot" ||
			CommandResolve(a.CommandMemory.at(i), 0) == L"LineStart" ||
			CommandResolve(a.CommandMemory.at(i), 0) == L"LinePassing" ||
			CommandResolve(a.CommandMemory.at(i), 0) == L"LineEnd" ||
			CommandResolve(a.CommandMemory.at(i), 0) == L"ArcPoint")
		{
			StrBuff.Format(_T(",%d,%d,%d"), _ttol(CommandResolve(a.CommandMemory.at(i), 1)) + GetDlgItemInt(IDC_EDITPARAM1),
				_ttol(CommandResolve(a.CommandMemory.at(i), 2)) + GetDlgItemInt(IDC_EDITPARAM2),
				_ttol(CommandResolve(a.CommandMemory.at(i), 3)) + GetDlgItemInt(IDC_EDITPARAM3));
			a.CommandMemory.at(i) = CommandResolve(a.CommandMemory.at(i), 0) + StrBuff;
		}
		else if (CommandResolve(a.CommandMemory.at(i), 0) == L"VirtualPoint" ||
			CommandResolve(a.CommandMemory.at(i), 0) == L"StopPoint")
		{
			StrBuff.Format(_T(",%d,%d,%d"), _ttol(CommandResolve(a.CommandMemory.at(i), 1)) + GetDlgItemInt(IDC_EDITPARAM1),
				_ttol(CommandResolve(a.CommandMemory.at(i), 2)) + GetDlgItemInt(IDC_EDITPARAM2),
				_ttol(CommandResolve(a.CommandMemory.at(i), 3)) + GetDlgItemInt(IDC_EDITPARAM3),
				_ttol(CommandResolve(a.CommandMemory.at(i), 4)));
			a.CommandMemory.at(i) = CommandResolve(a.CommandMemory.at(i), 0) + StrBuff;
		}
		else if (CommandResolve(a.CommandMemory.at(i), 0) == L"WaitPoint")
		{
			StrBuff.Format(_T(",%d,%d,%d"), _ttol(CommandResolve(a.CommandMemory.at(i), 1)) + GetDlgItemInt(IDC_EDITPARAM1),
				_ttol(CommandResolve(a.CommandMemory.at(i), 2)) + GetDlgItemInt(IDC_EDITPARAM2),
				_ttol(CommandResolve(a.CommandMemory.at(i), 3)) + GetDlgItemInt(IDC_EDITPARAM3),
				_ttol(CommandResolve(a.CommandMemory.at(i), 4)),
				_ttol(CommandResolve(a.CommandMemory.at(i), 5)));
			a.CommandMemory.at(i) = CommandResolve(a.CommandMemory.at(i), 0) + StrBuff;
		}
		else if (CommandResolve(a.CommandMemory.at(i), 0) == L"CirclePoint")
		{
			StrBuff.Format(_T(",%d,%d,%d,%d,%d,%d"), _ttol(CommandResolve(a.CommandMemory.at(i), 1)) + GetDlgItemInt(IDC_EDITPARAM1),
				_ttol(CommandResolve(a.CommandMemory.at(i), 2)) + GetDlgItemInt(IDC_EDITPARAM2),
				_ttol(CommandResolve(a.CommandMemory.at(i), 3)) + GetDlgItemInt(IDC_EDITPARAM3),
				_ttol(CommandResolve(a.CommandMemory.at(i), 4)) + GetDlgItemInt(IDC_EDITPARAM1),
				_ttol(CommandResolve(a.CommandMemory.at(i), 5)) + GetDlgItemInt(IDC_EDITPARAM2),
				_ttol(CommandResolve(a.CommandMemory.at(i), 6)) + GetDlgItemInt(IDC_EDITPARAM3));
			a.CommandMemory.at(i) = CommandResolve(a.CommandMemory.at(i), 0) + StrBuff;
		}
		else if (CommandResolve(a.CommandMemory.at(i), 0) == L"LaserHeight")
		{
			StrBuff.Format(_T(",%d,%d,%d"), _ttol(CommandResolve(a.CommandMemory.at(i), 1)),
				_ttol(CommandResolve(a.CommandMemory.at(i), 2)) + GetDlgItemInt(IDC_EDITPARAM1),
				_ttol(CommandResolve(a.CommandMemory.at(i), 3)) + GetDlgItemInt(IDC_EDITPARAM2));
			a.CommandMemory.at(i) = CommandResolve(a.CommandMemory.at(i), 0) + StrBuff;
		}
		else if (CommandResolve(a.CommandMemory.at(i), 0) == L"LaserDetect")
		{
			StrBuff.Format(_T(",%d,%d,%d"), _ttol(CommandResolve(a.CommandMemory.at(i), 1)),
				_ttol(CommandResolve(a.CommandMemory.at(i), 2)) + GetDlgItemInt(IDC_EDITPARAM1),
				_ttol(CommandResolve(a.CommandMemory.at(i), 3)) + GetDlgItemInt(IDC_EDITPARAM2),
				_ttol(CommandResolve(a.CommandMemory.at(i), 4)) + GetDlgItemInt(IDC_EDITPARAM1),
				_ttol(CommandResolve(a.CommandMemory.at(i), 5)) + GetDlgItemInt(IDC_EDITPARAM2));
			a.CommandMemory.at(i) = CommandResolve(a.CommandMemory.at(i), 0) + StrBuff;
		}
	}
	ListRefresh(NULL);
}
/*載入DEMO檔*/
void CCommandTestDlg::OnBnClickedBtnloaddemo()
{
	a.LoadPointData();
	a.DemoTemprarilySwitch = TRUE;
	ListRefresh(NULL);
}
/*讀取雷射表*/
void CCommandTestDlg::OnBnClickedBtnprintflaser()
{
	if (m_pLaserAdjustDlg == NULL)
	{
		m_pLaserAdjustDlg = new CLaserAdjust();
		m_pLaserAdjustDlg->Create(IDD_DIALOG9, this);
		m_pLaserAdjustDlg->ShowWindow(SW_SHOW);
	}
	else
	{
		((CCamera*)m_pLaserAdjustDlg)->DestroyWindow();
		free(m_pLaserAdjustDlg);
		m_pLaserAdjustDlg = new CLaserAdjust();
		m_pLaserAdjustDlg->Create(IDD_DIALOG9, this);
		m_pLaserAdjustDlg->ShowWindow(SW_SHOW);
	}
}
/*讀取修正表*/
void CCommandTestDlg::OnBnClickedBtnmodify()
{
	if (m_pPositionModifyDlg == NULL)
	{
		m_pPositionModifyDlg = new CPositionModify();
		m_pPositionModifyDlg->Create(IDD_DIALOG7, this);
		m_pPositionModifyDlg->ShowWindow(SW_SHOW);
	}
	else
	{
		((CCamera*)m_pPositionModifyDlg)->DestroyWindow();
		free(m_pPositionModifyDlg);
		m_pPositionModifyDlg = new CPositionModify();
		m_pPositionModifyDlg->Create(IDD_DIALOG7, this);
		m_pPositionModifyDlg->ShowWindow(SW_SHOW);
	}
}
/*讀取所有連續線段*/
void CCommandTestDlg::OnBnClickedBtnprintcline()
{
	if (m_pLineContinuousDlg == NULL)
	{
		m_pLineContinuousDlg = new CLineContinuous;
		m_pLineContinuousDlg->Create(IDD_DIALOG8, this);
		m_pLineContinuousDlg->ShowWindow(SW_SHOW);
	}
	else
	{
		((CCamera*)m_pLineContinuousDlg)->DestroyWindow();
		free(m_pLineContinuousDlg);
		m_pLineContinuousDlg = new CLineContinuous();
		m_pLineContinuousDlg->Create(IDD_DIALOG8, this);
		m_pLineContinuousDlg->ShowWindow(SW_SHOW);
	}
}
/*註解*/
void CCommandTestDlg::OnBnClickedBtncommit()
{
	if (!Insert)
	{
		int istat = m_CommandList.GetSelectionMark();//獲取選擇的項
		if (istat >= 0)
		{
			a.CommandMemory.at(istat) = L"//" + a.CommandMemory.at(istat);
			ListRefresh(NULL);
		}     
	}
}               
/*取消註解*/
void CCommandTestDlg::OnBnClickedBtnnocommit()
{
	if (!Insert)
	{
		int istat = m_CommandList.GetSelectionMark();//獲取選擇的項
		if (istat >= 0)
		{
			if (a.CommandMemory.at(istat).Find(L"//") != -1)
			{
				a.CommandMemory.at(istat) = a.CommandMemory.at(istat).Right(a.CommandMemory.at(istat).Delete(0, 2));
				ListRefresh(NULL);
			}        
		}
	}
}
/*儲存命令表*/
void CCommandTestDlg::OnBnClickedBtnsave()
{
	TCHAR szFilters[] = _T("文字文件(*.txt)|*.txt|所有檔案(*.*)|*.*||");
	CFileDialog FileDlg(FALSE, L".txt", NULL, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST, szFilters);
	FileDlg.m_ofn.lpstrTitle = _T("儲存命令表");
	if (FileDlg.DoModal() == IDOK)
	{
		CFile File;
		if (File.Open(FileDlg.GetPathName(), CFile::modeCreate | CFile::modeWrite))
		{
			CArchive ar(&File, CArchive::store);//儲存檔案
			for (UINT i = 0; i < a.CommandMemory.size(); i++)
			{
				ar << a.CommandMemory.at(i);
			}
			ar.Close();
		}
		File.Close();
	}
}
/*載入命令表*/
void CCommandTestDlg::OnBnClickedBtnopen()
{
	TCHAR szFilters[] = _T("文字文件(*.txt)|*.txt|所有檔案(*.*)|*.*||");
	CFileDialog FileDlg(TRUE, NULL, L".txt", OFN_HIDEREADONLY | OFN_FILEMUSTEXIST, szFilters);
	FileDlg.m_ofn.lpstrTitle = _T("儲存命令表");
	if (FileDlg.DoModal() == IDOK)
	{
		CFile File;
		CString StrBuff;
		a.CommandMemory.clear();
		if (File.Open(FileDlg.GetPathName(), CFile::modeRead))
		{
			CArchive ar(&File, CArchive::load);//讀取檔案
			while(1)
			{
				/******注意寫法******/
				ar >> StrBuff;
				a.CommandMemory.push_back(StrBuff);
				if (ar.IsBufferEmpty() == 1)
					break;		
			}  
			ar.Close();
			File.Close();
		}  
	}
	ListRefresh(NULL);
}
/*******************************************************************************************命令**********************************************************/
/*單點點膠*/
void CCommandTestDlg::OnBnClickedBtncommand1()
{
	if (InputAuto)
	{
#ifdef MOVE
		StrBuff.Format(_T("Dot,%d,%d,%d"), MO_ReadLogicPosition(0) + OffsetX, MO_ReadLogicPosition(1) + OffsetY, MO_ReadLogicPosition(2));
#endif
	}
	else
	{
		StrBuff.Format(_T("Dot,%d,%d,%d"),GetDlgItemInt(IDC_EDITPARAM1) + OffsetX, GetDlgItemInt(IDC_EDITPARAM2) + OffsetY, GetDlgItemInt(IDC_EDITPARAM3));
	}
	(Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum,StrBuff) : a.CommandMemory.push_back(StrBuff);
	Insert = FALSE;
	ListRefresh(NULL);
}
/*單點點膠設置*/
void CCommandTestDlg::OnBnClickedBtncommand2()
{
	StrBuff.Format(_T("DispenseDotSet,%d,%d"), GetDlgItemInt(IDC_EDITPARAM1), GetDlgItemInt(IDC_EDITPARAM2));
	(Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
	Insert = FALSE;
	ListRefresh(NULL);
}
/*單點結束設置*/
void CCommandTestDlg::OnBnClickedBtncommand3()
{
	StrBuff.Format(_T("DispenseDotEnd,%d,%d,%d"), GetDlgItemInt(IDC_EDITPARAM1), GetDlgItemInt(IDC_EDITPARAM2), GetDlgItemInt(IDC_EDITPARAM3));
	(Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
	Insert = FALSE;
	ListRefresh(NULL);
}
/*PTP驅動速度*/
void CCommandTestDlg::OnBnClickedBtncommand4()
{
	StrBuff.Format(_T("DotSpeedSet,%d"), GetDlgItemInt(IDC_EDITPARAM1));
	(Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
	Insert = FALSE;
	ListRefresh(NULL);
}
/*PTP加速度設置(%)*/
void CCommandTestDlg::OnBnClickedBtncommand4_2()
{
	StrBuff.Format(_T("DotAccPercent,%d"), GetDlgItemInt(IDC_EDITPARAM1));
	(Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
	Insert = FALSE;
	ListRefresh(NULL);
}
/*線段開始*/
void CCommandTestDlg::OnBnClickedBtncommand5()
{
	if (InputAuto)
	{
#ifdef MOVE
		StrBuff.Format(_T("LineStart,%d,%d,%d"), MO_ReadLogicPosition(0) + OffsetX, MO_ReadLogicPosition(1) + OffsetY, MO_ReadLogicPosition(2));
#endif
	}
	else
	{
		StrBuff.Format(_T("LineStart,%d,%d,%d"), GetDlgItemInt(IDC_EDITPARAM1) + OffsetX, GetDlgItemInt(IDC_EDITPARAM2) + OffsetY, GetDlgItemInt(IDC_EDITPARAM3));
	}
	(Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
	Insert = FALSE;
	ListRefresh(NULL);
}
/*線段中間點*/
void CCommandTestDlg::OnBnClickedBtncommand6()
{
	if (InputAuto)
	{
#ifdef MOVE
		StrBuff.Format(_T("LinePassing,%d,%d,%d"), MO_ReadLogicPosition(0) + OffsetX, MO_ReadLogicPosition(1) + OffsetY, MO_ReadLogicPosition(2));
#endif
	}
	else
	{
		StrBuff.Format(_T("LinePassing,%d,%d,%d"), GetDlgItemInt(IDC_EDITPARAM1) + OffsetX, GetDlgItemInt(IDC_EDITPARAM2) + OffsetY, GetDlgItemInt(IDC_EDITPARAM3));
	}
	(Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
	Insert = FALSE;
	ListRefresh(NULL);
}
/*線段結束點*/
void CCommandTestDlg::OnBnClickedBtncommand7()
{     
	if (InputAuto)
	{
#ifdef MOVE
		StrBuff.Format(_T("LineEnd,%d,%d,%d"), MO_ReadLogicPosition(0) + OffsetX, MO_ReadLogicPosition(1) + OffsetY, MO_ReadLogicPosition(2));
#endif
	}
	else
	{
		StrBuff.Format(_T("LineEnd,%d,%d,%d"), GetDlgItemInt(IDC_EDITPARAM1) + OffsetX, GetDlgItemInt(IDC_EDITPARAM2) + OffsetY, GetDlgItemInt(IDC_EDITPARAM3));
	}
	(Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
	Insert = FALSE;
	ListRefresh(NULL);
}
/*圓弧中點*/
void CCommandTestDlg::OnBnClickedBtncommand8()
{
	if (InputAuto)
	{
#ifdef MOVE
		StrBuff.Format(_T("ArcPoint,%d,%d,%d"), MO_ReadLogicPosition(0) + OffsetX, MO_ReadLogicPosition(1) + OffsetY, MO_ReadLogicPosition(2));
#endif
	}
	else
	{
		StrBuff.Format(_T("ArcPoint,%d,%d,%d"), GetDlgItemInt(IDC_EDITPARAM1) + OffsetX, GetDlgItemInt(IDC_EDITPARAM2) + OffsetY, GetDlgItemInt(IDC_EDITPARAM3));
	}
	(Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
	Insert = FALSE;
	ListRefresh(NULL);
}
/*圓中點*/
void CCommandTestDlg::OnBnClickedBtncommand9()
{
	CString ControlName;
	GetDlgItemText(IDC_BTNCOMMAND9, ControlName);
	if (InputAuto)
	{
#ifdef MOVE
		if (ControlName == L"圓中點")
		{

			StrBuff.Format(_T("CirclePoint,%d,%d,%d"), MO_ReadLogicPosition(0) + OffsetX, MO_ReadLogicPosition(1) + OffsetY, MO_ReadLogicPosition(2));
			SetDlgItemText(IDC_BTNCOMMAND9, _T("圓中點2"));
		}
		else
		{
			ControlName.Format(_T(",%d,%d,%d"), MO_ReadLogicPosition(0) + OffsetX, MO_ReadLogicPosition(1) + OffsetY, MO_ReadLogicPosition(2));
			StrBuff = StrBuff + ControlName;
			SetDlgItemText(IDC_BTNCOMMAND9, _T("圓中點"));
			(Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
			Insert = FALSE;
			ListRefresh(NULL);
		}
#endif
	}
	else
	{
		StrBuff.Format(_T("CirclePoint,%d,%d,%d,%d,%d,%d"), GetDlgItemInt(IDC_EDITPARAM1) + OffsetX, GetDlgItemInt(IDC_EDITPARAM2) + OffsetY, GetDlgItemInt(IDC_EDITPARAM3), GetDlgItemInt(IDC_EDITPARAM4) + OffsetX, GetDlgItemInt(IDC_EDITPARAM5) + OffsetY, GetDlgItemInt(IDC_EDITPARAM6));
		(Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
		Insert = FALSE;
		ListRefresh(NULL);
	}	
}
/*圓*/
void CCommandTestDlg::OnBnClickedBtncommand10()
{
	/*StrBuff.Format(_T("CirclePointTwo,%d,%d,%d"), GetDlgItemInt(IDC_EDITPARAM1), GetDlgItemInt(IDC_EDITPARAM2), GetDlgItemInt(IDC_EDITPARAM3));
	(Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
	Insert = FALSE;
	ListRefresh(NULL);*/
}
/*線段塗膠設置*/
void CCommandTestDlg::OnBnClickedBtncommand11()
{
	StrBuff.Format(_T("DispenseLineSet,%d,%d,%d,%d,%d,%d"), GetDlgItemInt(IDC_EDITPARAM1), GetDlgItemInt(IDC_EDITPARAM2), GetDlgItemInt(IDC_EDITPARAM3),
		GetDlgItemInt(IDC_EDITPARAM4), GetDlgItemInt(IDC_EDITPARAM5), GetDlgItemInt(IDC_EDITPARAM6));
	(Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
	Insert = FALSE;
	ListRefresh(NULL);
}
/*線段結束設置*/
void CCommandTestDlg::OnBnClickedBtncommand12()
{
	StrBuff.Format(_T("DispenseLineEnd,%d,%d,%d,%d,%d"), GetDlgItemInt(IDC_EDITPARAM1), GetDlgItemInt(IDC_EDITPARAM2), GetDlgItemInt(IDC_EDITPARAM3),
		GetDlgItemInt(IDC_EDITPARAM4), GetDlgItemInt(IDC_EDITPARAM5));
	(Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
	Insert = FALSE;
	ListRefresh(NULL);
}
/*塗膠速度設置*/
void CCommandTestDlg::OnBnClickedBtncommand13()
{
	StrBuff.Format(_T("LineSpeedSet,%d"), GetDlgItemInt(IDC_EDITPARAM1));
	(Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
	Insert = FALSE;
	ListRefresh(NULL);
}
/*Z軸工作高度*/
void CCommandTestDlg::OnBnClickedBtncommand14()
{
	StrBuff.Format(_T("ZGoBack,%d,%d"), GetDlgItemInt(IDC_EDITPARAM1), GetDlgItemInt(IDC_EDITPARAM2));
	(Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
	Insert = FALSE;
	ListRefresh(NULL);
}
/*加速度設置*/
void CCommandTestDlg::OnBnClickedBtncommand15()
{
	StrBuff.Format(_T("DispenseAccSet,%d,%d"), GetDlgItemInt(IDC_EDITPARAM1), GetDlgItemInt(IDC_EDITPARAM2));
	(Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
	Insert = FALSE;
	ListRefresh(NULL);
}
/*參數初始化*/
void CCommandTestDlg::OnBnClickedBtncommand16()
{
	StrBuff = _T("Initialize");
	(Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
	Insert = FALSE;
	ListRefresh(NULL);
}
/*輸入*/
void CCommandTestDlg::OnBnClickedBtncommand17()
{
	StrBuff.Format(_T("Input,%d,%d,%d"), GetDlgItemInt(IDC_EDITPARAM1), GetDlgItemInt(IDC_EDITPARAM2), GetDlgItemInt(IDC_EDITPARAM3));
	(Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
	Insert = FALSE;
	ListRefresh(NULL);
}
/*輸出*/
void CCommandTestDlg::OnBnClickedBtncommand18()
{
	StrBuff.Format(_T("Output,%d,%d"), GetDlgItemInt(IDC_EDITPARAM1), GetDlgItemInt(IDC_EDITPARAM2));
	(Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
	Insert = FALSE;
	ListRefresh(NULL);
}
/*點膠機開關*/
void CCommandTestDlg::OnBnClickedBtncommand19()
{
	StrBuff.Format(_T("DispenserSwitch,%d"), GetDlgItemInt(IDC_EDITPARAM1));
	(Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
	Insert = FALSE;
	ListRefresh(NULL);
}
/*設置點膠端口*/
void CCommandTestDlg::OnBnClickedBtncommand19_2()
{
	StrBuff.Format(_T("DispenserSwitchSet,%d"), GetDlgItemInt(IDC_EDITPARAM1));
	(Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
	Insert = FALSE;
	ListRefresh(NULL);
}
/*虛擬點*/
void CCommandTestDlg::OnBnClickedBtncommand20()
{
	if(InputAuto)
	{
#ifdef MOVE
		StrBuff.Format(_T("VirtualPoint,%d,%d,%d,%d"), MO_ReadLogicPosition(0) + OffsetX, MO_ReadLogicPosition(1) + OffsetY, MO_ReadLogicPosition(2), GetDlgItemInt(IDC_EDITPARAM1));
#endif
	}
	else
	{
		StrBuff.Format(_T("VirtualPoint,%d,%d,%d,%d"), GetDlgItemInt(IDC_EDITPARAM1) + OffsetX, GetDlgItemInt(IDC_EDITPARAM2) + OffsetY, GetDlgItemInt(IDC_EDITPARAM3), GetDlgItemInt(IDC_EDITPARAM4));
	}
	(Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
	Insert = FALSE;
	ListRefresh(NULL);
}
/*等待點*/
void CCommandTestDlg::OnBnClickedBtncommand21()
{
	if (InputAuto)
	{
#ifdef MOVE
		StrBuff.Format(_T("WaitPoint,%d,%d,%d,%d,%d"), MO_ReadLogicPosition(0) + OffsetX, MO_ReadLogicPosition(1) + OffsetY, MO_ReadLogicPosition(2), GetDlgItemInt(IDC_EDITPARAM1), GetDlgItemInt(IDC_EDITPARAM2));
#endif
	}
	else
	{
		StrBuff.Format(_T("WaitPoint,%d,%d,%d,%d,%d"), GetDlgItemInt(IDC_EDITPARAM1) + OffsetX, GetDlgItemInt(IDC_EDITPARAM2) + OffsetY, GetDlgItemInt(IDC_EDITPARAM3), GetDlgItemInt(IDC_EDITPARAM4), GetDlgItemInt(IDC_EDITPARAM5));
	}
	(Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
	Insert = FALSE;
	ListRefresh(NULL);
}
/*停駐點*/
void CCommandTestDlg::OnBnClickedBtncommand22()
{
	StrBuff.Format(_T("ParkPosition"));
	(Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
	Insert = FALSE;
	ListRefresh(NULL);
}
/*停止點*/
void CCommandTestDlg::OnBnClickedBtncommand23()
{
	if (InputAuto)
	{
#ifdef MOVE
		StrBuff.Format(_T("StopPoint,%d,%d,%d,%d"), MO_ReadLogicPosition(0) + OffsetX, MO_ReadLogicPosition(1) + OffsetY, MO_ReadLogicPosition(2), GetDlgItemInt(IDC_EDITPARAM1));
#endif
	}
	else
	{
		StrBuff.Format(_T("StopPoint,%d,%d,%d,%d"), GetDlgItemInt(IDC_EDITPARAM1) + OffsetX, GetDlgItemInt(IDC_EDITPARAM2) + OffsetY, GetDlgItemInt(IDC_EDITPARAM3), GetDlgItemInt(IDC_EDITPARAM4));
	}
	(Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
	Insert = FALSE;
	ListRefresh(NULL);
}                                                      
/*填充區域*/
void CCommandTestDlg::OnBnClickedBtncommand24()
{
	CString ControlName;
	GetDlgItemText(IDC_BTNCOMMAND24, ControlName);
	if (InputAuto)
	{
#ifdef MOVE
		if (ControlName == L"填充區域")
		{
			StrBuff.Format(_T("FillArea,%d,%d,%d,%d,%d,%d"), GetDlgItemInt(IDC_EDITPARAM1), GetDlgItemInt(IDC_EDITPARAM2), GetDlgItemInt(IDC_EDITPARAM3),
				MO_ReadLogicPosition(0) + OffsetX, MO_ReadLogicPosition(1) + OffsetY, MO_ReadLogicPosition(2));
			SetDlgItemText(IDC_BTNCOMMAND24, _T("填充點2"));
		}
		else
		{
			ControlName.Format(_T(",%d,%d,%d"), MO_ReadLogicPosition(0) + OffsetX, MO_ReadLogicPosition(1) + OffsetY, MO_ReadLogicPosition(2));
			StrBuff = StrBuff + ControlName;
			SetDlgItemText(IDC_BTNCOMMAND24, _T("填充區域"));
			(Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
			Insert = FALSE;
			ListRefresh(NULL);
		}
#endif
	}
	else
	{
		StrBuff.Format(_T("FillArea,%d,%d,%d,%d,%d,%d,%d,%d,%d"), GetDlgItemInt(IDC_EDITPARAM1), GetDlgItemInt(IDC_EDITPARAM2), GetDlgItemInt(IDC_EDITPARAM3), GetDlgItemInt(IDC_EDITPARAM4) + OffsetX, GetDlgItemInt(IDC_EDITPARAM5) + OffsetY, GetDlgItemInt(IDC_EDITPARAM6), GetDlgItemInt(IDC_EDITPARAM7) + OffsetX, GetDlgItemInt(IDC_EDITPARAM8) + OffsetY, GetDlgItemInt(IDC_EDITPARAM9));
		(Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
		Insert = FALSE;
		ListRefresh(NULL);
	}
}
/*回原點命令*/
void CCommandTestDlg::OnBnClickedBtncommand25()
{
	StrBuff = _T("GoHome");
	(Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
	Insert = FALSE;
	ListRefresh(NULL);
}
/*清潔點*/
void CCommandTestDlg::OnBnClickedBtncommand25_1()
{
	StrBuff.Format(_T("CleanerPosition"));
	(Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
	Insert = FALSE;
	ListRefresh(NULL);
}
/*標籤*/           
void CCommandTestDlg::OnBnClickedBtncommand26()
{
	StrBuff.Format(_T("Label,%d"), GetDlgItemInt(IDC_EDITPARAM1));
	(Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
	Insert = FALSE;
	ListRefresh(NULL);
}
/*前往地址*/
void CCommandTestDlg::OnBnClickedBtncommand27()
{
	StrBuff.Format(_T("GotoAddress,%d"),GetDlgItemInt(IDC_EDITPARAM1));
	(Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
	Insert = FALSE;
	ListRefresh(NULL);
}
/*前往標籤*/
void CCommandTestDlg::OnBnClickedBtncommand28()
{
	StrBuff.Format(_T("GotoLabel,%d"),GetDlgItemInt(IDC_EDITPARAM1));
	(Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
	Insert = FALSE;
	ListRefresh(NULL);
}
/*調用子程序*/
void CCommandTestDlg::OnBnClickedBtncommand29()
{
	StrBuff.Format(_T("CallSubroutine,%d"), GetDlgItemInt(IDC_EDITPARAM1));
	(Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
	Insert = FALSE;
	ListRefresh(NULL);
}
/*結束子程序*/
void CCommandTestDlg::OnBnClickedBtncommand30()
{
	StrBuff.Format(_T("SubroutineEnd"));
	(Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
	Insert = FALSE;
	ListRefresh(NULL);
}
/*循環*/
void CCommandTestDlg::OnBnClickedBtncommand31()
{
	StrBuff.Format(_T("Loop,%d,%d"), GetDlgItemInt(IDC_EDITPARAM1), GetDlgItemInt(IDC_EDITPARAM2));
	(Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
	Insert = FALSE;
	ListRefresh(NULL);
}
/*步驟重複標籤*/
void CCommandTestDlg::OnBnClickedBtncommand32_0()
{
	StrBuff.Format(_T("StepRepeatLabel,%d"), GetDlgItemInt(IDC_EDITPARAM1));
	(Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
	Insert = FALSE;
	ListRefresh(NULL);
}
/*步驟重複X*/
void CCommandTestDlg::OnBnClickedBtncommand32()
{
	CString Buff;
	StrBuff.Format(_T("StepRepeatX,%d,%d,%d,%d,%d,%d,0,0,"), GetDlgItemInt(IDC_EDITPARAM1), GetDlgItemInt(IDC_EDITPARAM2), GetDlgItemInt(IDC_EDITPARAM3), GetDlgItemInt(IDC_EDITPARAM4), GetDlgItemInt(IDC_EDITPARAM5), GetDlgItemInt(IDC_EDITPARAM6));
	(Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
	Insert = FALSE;
	ListRefresh(NULL);
}
/*步驟重複Y*/
void CCommandTestDlg::OnBnClickedBtncommand33()
{
	CString Buff;
	StrBuff.Format(_T("StepRepeatY,%d,%d,%d,%d,%d,%d,0,0,"), GetDlgItemInt(IDC_EDITPARAM1), GetDlgItemInt(IDC_EDITPARAM2), GetDlgItemInt(IDC_EDITPARAM3), GetDlgItemInt(IDC_EDITPARAM4), GetDlgItemInt(IDC_EDITPARAM5), GetDlgItemInt(IDC_EDITPARAM6));
	(Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
	Insert = FALSE;
	ListRefresh(NULL);
}
/*查找標記*/
void CCommandTestDlg::OnBnClickedBtncommand34()
{
	if (InputAuto)
	{
#ifdef MOVE
		StrBuff.Format(_T("FindMark,%d,%d,%d,%d,%d"), MO_ReadLogicPosition(0) + OffsetX, MO_ReadLogicPosition(1) + OffsetY, MO_ReadLogicPosition(2), GetDlgItemInt(IDC_EDITPARAM1), GetDlgItemInt(IDC_EDITPARAM2));
#endif
	}
	else
	{
		StrBuff.Format(_T("FindMark,%d,%d,%d,%d,%d"), GetDlgItemInt(IDC_EDITPARAM1) + OffsetX, GetDlgItemInt(IDC_EDITPARAM2) + OffsetY, GetDlgItemInt(IDC_EDITPARAM3), GetDlgItemInt(IDC_EDITPARAM4), GetDlgItemInt(IDC_EDITPARAM5));
	}
	(Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
	Insert = FALSE;
	ListRefresh(NULL);
}
/*基準標記*/
void CCommandTestDlg::OnBnClickedBtncommand35()
{
	if (InputAuto)
	{
#ifdef MOVE
		StrBuff.Format(_T("FiducialMark,%d,%d,%d,%d,%d"), MO_ReadLogicPosition(0) + OffsetX, MO_ReadLogicPosition(1) + OffsetY, MO_ReadLogicPosition(2), GetDlgItemInt(IDC_EDITPARAM1), GetDlgItemInt(IDC_EDITPARAM2));
#endif
	}
	else
	{
		StrBuff.Format(_T("FiducialMark,%d,%d,%d,%d,%d"), GetDlgItemInt(IDC_EDITPARAM1) + OffsetX, GetDlgItemInt(IDC_EDITPARAM2) + OffsetY, GetDlgItemInt(IDC_EDITPARAM3), GetDlgItemInt(IDC_EDITPARAM4), GetDlgItemInt(IDC_EDITPARAM5));
	}
	(Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
	Insert = FALSE;
	ListRefresh(NULL);
}
/*查找標記調整*/
void CCommandTestDlg::OnBnClickedBtncommand36()
{
	StrBuff.Format(_T("FindMarkAdjust"));
	(Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
	Insert = FALSE;
	ListRefresh(NULL);
}
/*基準標記調整*/
void CCommandTestDlg::OnBnClickedBtncommand37()
{
	StrBuff.Format(_T("FiducialMarkAdjust"));
	(Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
	Insert = FALSE;
	ListRefresh(NULL);
}
/*查找角度標記*/
void CCommandTestDlg::OnBnClickedBtncommand38()
{
	StrBuff.Format(_T("FindFiducialAngle,%d,%d"), GetDlgItemInt(IDC_EDITPARAM1), GetDlgItemInt(IDC_EDITPARAM2));
	(Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
	Insert = FALSE;
	ListRefresh(NULL);
}
/*CCD觸發器*/
void CCommandTestDlg::OnBnClickedBtncommand40()
{
	if (InputAuto)
	{
#ifdef MOVE
		StrBuff.Format(_T("CameraTrigger,%d,%d,%d"), MO_ReadLogicPosition(0) + OffsetX, MO_ReadLogicPosition(1) + OffsetY, MO_ReadLogicPosition(2));
#endif
	}
	else
	{
		StrBuff.Format(_T("CameraTrigger,%d,%d,%d"), GetDlgItemInt(IDC_EDITPARAM1) + OffsetX, GetDlgItemInt(IDC_EDITPARAM2) + OffsetY, GetDlgItemInt(IDC_EDITPARAM3));
	}
	(Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
	Insert = FALSE;
	ListRefresh(NULL);
}                                              
/*擴展步驟重複*/
void CCommandTestDlg::OnBnClickedBtncommand41()
{
	StrBuff.Format(_T("Printf"));
	(Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
	Insert = FALSE;
	ListRefresh(NULL);
}
/*開始副程式*/
void CCommandTestDlg::OnBnClickedBtncommand43()
{
	StrBuff.Format(_T("SubProgramStart,%d"), GetDlgItemInt(IDC_EDITPARAM1));
	(Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
	Insert = FALSE;
	ListRefresh(NULL);
}
/*結束副程式*/
void CCommandTestDlg::OnBnClickedBtncommand44()
{
	StrBuff.Format(_T("SubProgramEnd"));
	(Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
	Insert = FALSE;
	ListRefresh(NULL);
}
/*調用副程式*/
void CCommandTestDlg::OnBnClickedBtncommand45()
{
	StrBuff.Format(_T("CallSubProgram,%d"), GetDlgItemInt(IDC_EDITPARAM1));
	(Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
	Insert = FALSE;
	ListRefresh(NULL);
}
/*雷射高度*/
void CCommandTestDlg::OnBnClickedBtncommand46()
{
	if (InputAuto)
	{
#ifdef MOVE
		StrBuff.Format(_T("LaserHeight,%d,%d,%d"), GetDlgItemInt(IDC_EDITPARAM1), MO_ReadLogicPosition(0) + OffsetX, MO_ReadLogicPosition(1) + OffsetY);
#endif
	}
	else
	{
		StrBuff.Format(_T("LaserHeight,%d,%d,%d"), GetDlgItemInt(IDC_EDITPARAM1), GetDlgItemInt(IDC_EDITPARAM2) + OffsetX, GetDlgItemInt(IDC_EDITPARAM3) + OffsetY);
	}
	(Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
	Insert = FALSE;
	ListRefresh(NULL);
}
/*雷射點調節*/
void CCommandTestDlg::OnBnClickedBtncommand47()
{
	StrBuff.Format(_T("LaserPointAdjust,%d"), GetDlgItemInt(IDC_EDITPARAM1));
	(Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
	Insert = FALSE;
	ListRefresh(NULL);
}
/*雷射檢測*/
void CCommandTestDlg::OnBnClickedBtncommand48()
{
	CString ControlName;
	GetDlgItemText(IDC_BTNCOMMAND48, ControlName);
	if (InputAuto)
	{
#ifdef MOVE
		if (ControlName == L"雷射檢測")
		{
			StrBuff.Format(_T("LaserDetect,%d,%d,%d"), GetDlgItemInt(IDC_EDITPARAM1),MO_ReadLogicPosition(0) + OffsetX, MO_ReadLogicPosition(1) + OffsetY);
			SetDlgItemText(IDC_BTNCOMMAND48, _T("雷射檢測點2"));
		}
		else
		{
			ControlName.Format(_T(",%d,%d"), MO_ReadLogicPosition(0) + OffsetX, MO_ReadLogicPosition(1) + OffsetY);
			StrBuff = StrBuff + ControlName;
			SetDlgItemText(IDC_BTNCOMMAND48, _T("雷射檢測"));
			(Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
			Insert = FALSE;
			ListRefresh(NULL);
		}
#endif
	}
	else
	{
		StrBuff.Format(_T("LaserDetect,%d,%d,%d,%d,%d"), GetDlgItemInt(IDC_EDITPARAM1), GetDlgItemInt(IDC_EDITPARAM2) + OffsetX, GetDlgItemInt(IDC_EDITPARAM3) + OffsetY, GetDlgItemInt(IDC_EDITPARAM4) + OffsetX, GetDlgItemInt(IDC_EDITPARAM5) + OffsetY);
		(Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
		Insert = FALSE;
		ListRefresh(NULL);
	}
}
/*雷射調整*/
void CCommandTestDlg::OnBnClickedBtncommand49()
{
	StrBuff.Format(_T("LaserAdjust,%d"), GetDlgItemInt(IDC_EDITPARAM1));
	(Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
	Insert = FALSE;
	ListRefresh(NULL);
}
/*雷射跳過*/
void CCommandTestDlg::OnBnClickedBtncommand50()
{
	StrBuff.Format(_T("LaserSkip,%d"), GetDlgItemInt(IDC_EDITPARAM1));
	(Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
	Insert = FALSE;
	ListRefresh(NULL);
}
/*******************************************************************************************私有函數**********************************************************/
/*指令分解*/
CString CCommandTestDlg::CommandResolve(CString Command, UINT Choose)
{
	int iLength = Command.Find(_T(','));
	if (iLength <= 0)
	{
		iLength = Command.GetLength();
	}
	if (Choose <= 0)
	{
		return Command.Left(iLength);
	}
	else
	{
		return CommandResolve(Command.Right(Command.GetLength() - iLength - 1), --Choose);
	}
}
/*計算程序執行時間*/
void CCommandTestDlg::Counter()
{
	int a = 0, b = 0;
	LARGE_INTEGER startTime, endTime, fre;
	double times;
	QueryPerformanceFrequency(&fre); //取得CPU頻率
	QueryPerformanceCounter(&startTime); //取得開機到現在經過幾個CPU Cycle
										 //doing something
	_cwprintf(_T("%s"), L"Interrupt");
	//doing something
	QueryPerformanceCounter(&endTime); //取得開機到程式執行完成經過幾個CPU Cycle
	times = ((double)endTime.QuadPart - (double)startTime.QuadPart) / fre.QuadPart;
	_cwprintf(L"%f", times);
}
//儲存參數檔案
void CCommandTestDlg::SaveParameter()
{
	CString path = GetCurrentPath(_T("\\Param"));
	CFileFind m_FileFind;
	if (!m_FileFind.FindFile(path))
	{
		CreateDirectory(path, NULL);
	}
	CFile File;
	if (File.Open(path + _T("\\Paramter.txt"), CFile::modeCreate | CFile::modeWrite))
	{
		CArchive ar(&File, CArchive::store);//儲存檔案
		ar << PixToPulsX << PixToPulsY << a.VisionDefault.VisionSet.AdjustOffsetX << a.VisionDefault.VisionSet.AdjustOffsetY << a.m_Action.g_OffSetLaserX << a.m_Action.g_OffSetLaserY << a.m_Action.g_OffSetLaserZ << a.m_Action.g_HeightLaserZero;
	}
	File.Close();
}
//載入參數檔案
void CCommandTestDlg::LoadParameter()
{
	CString path = GetCurrentPath(_T("\\Param"));
	CFileFind m_FileFind;
	if (!m_FileFind.FindFile(path))
	{
		CreateDirectory(path, NULL);
	}
	CFile File;
	if (File.Open(path + _T("\\Paramter.txt"), CFile::modeRead))
	{
		CArchive ar(&File, CArchive::load);//讀取檔案
		ar >> PixToPulsX >> PixToPulsY >> a.VisionDefault.VisionSet.AdjustOffsetX >> a.VisionDefault.VisionSet.AdjustOffsetY >> a.m_Action.g_OffSetLaserX >> a.m_Action.g_OffSetLaserY >> a.m_Action.g_OffSetLaserZ >> a.m_Action.g_HeightLaserZero;
		File.Close();
	}
	TipOffset.x = a.VisionDefault.VisionSet.AdjustOffsetX;
	TipOffset.y = a.VisionDefault.VisionSet.AdjustOffsetY;
	LaserOffset.x = a.m_Action.g_OffSetLaserX;
	LaserOffset.y = a.m_Action.g_OffSetLaserY;
	LaserOffsetz = a.m_Action.g_OffSetLaserZ;
	HeightLaserZero = a.m_Action.g_HeightLaserZero;
#ifdef VI
	VI_SetOnePixelUnit(PixToPulsX, PixToPulsY);//設定Pixel轉實際距離
	VI_SetCameraToTipOffset(TipOffset.x, TipOffset.y);//設定針頭和影像Offset
#endif
}
//儲存Default參數檔案
void CCommandTestDlg::SaveDefault()
{
	CString path = GetCurrentPath(_T("\\Param"));
	CFileFind m_FileFind;
	if (!m_FileFind.FindFile(path))
	{
		CreateDirectory(path, NULL);
	}
	CFile File;
	if (File.Open(path + _T("\\Default.txt"), CFile::modeCreate | CFile::modeWrite))
	{
		CArchive ar(&File, CArchive::store);//儲存檔案
		ar << 
			a.Default.GoHome.Speed1 << 
			a.Default.GoHome.Speed2 <<
			a.Default.GoHome.Axis <<
			a.Default.GoHome.MoveX <<
			a.Default.GoHome.MoveY <<
			a.Default.GoHome.MoveZ <<
			a.Default.GoHome.PrecycleInitialize <<
			a.Default.DispenseDotSet.GlueOpenTime <<
			a.Default.DispenseDotSet.GlueCloseTime <<
			a.Default.DispenseDotEnd.RiseDistance <<
			a.Default.DispenseDotEnd.RiseLowSpeed <<
			a.Default.DispenseDotEnd.RiseHightSpeed <<
			a.Default.DotSpeedSet.AccSpeed <<
			a.Default.DotSpeedSet.EndSpeed <<
			a.Default.DispenseLineSet.BeforeMoveDelay <<
			a.Default.DispenseLineSet.BeforeMoveDistance <<
			a.Default.DispenseLineSet.NodeTime <<
			a.Default.DispenseLineSet.StayTime <<
			a.Default.DispenseLineSet.ShutdownDistance <<
			a.Default.DispenseLineSet.ShutdownDelay <<
			a.Default.DispenseLineEnd.Type <<
			a.Default.DispenseLineEnd.LowSpeed <<
			a.Default.DispenseLineEnd.Height <<
			a.Default.DispenseLineEnd.Width <<
			a.Default.DispenseLineEnd.HighSpeed <<
			a.Default.LineSpeedSet.AccSpeed <<
			a.Default.LineSpeedSet.EndSpeed <<
			a.Default.ZSet.ZBackHeight <<
			a.Default.ZSet.ZBackType <<
			a.Default.GlueData.ParkPositionData.X <<
			a.Default.GlueData.ParkPositionData.Y <<
			a.Default.GlueData.ParkPositionData.Z <<
			a.Default.GlueData.GlueAuto <<
			a.Default.GlueData.GlueWaitTime <<
			a.Default.GlueData.GlueTime <<
			a.Default.GlueData.GlueStayTime <<
			a.VisionDefault.VisionSet.Accuracy <<
			a.VisionDefault.VisionSet.Speed <<
			a.VisionDefault.VisionSet.Score <<
			a.VisionDefault.VisionSet.width <<
			a.VisionDefault.VisionSet.height <<
			a.VisionDefault.VisionSet.Startangle <<
			a.VisionDefault.VisionSet.Endangle <<
			a.VisionDefault.VisionSerchError.SearchError <<
			a.RunLoopData.MaxRunNumber <<
			RunLoopNumber;
	}
	File.Close();
}
//載入Default參數檔案
void CCommandTestDlg::LoadDefault()
{
	CString path = GetCurrentPath(_T("\\Param"));
	CFileFind m_FileFind;
	if (!m_FileFind.FindFile(path))
	{
		CreateDirectory(path, NULL);
	}
	CFile File;
	if (File.Open(path + _T("\\Default.txt"), CFile::modeRead))
	{
		CArchive ar(&File, CArchive::load);//讀取檔案
		ar >>
			a.Default.GoHome.Speed1 >>
			a.Default.GoHome.Speed2 >>
			a.Default.GoHome.Axis >>
			a.Default.GoHome.MoveX >>
			a.Default.GoHome.MoveY >>
			a.Default.GoHome.MoveZ >>
			a.Default.GoHome.PrecycleInitialize >>
			a.Default.DispenseDotSet.GlueOpenTime >>
			a.Default.DispenseDotSet.GlueCloseTime >>
			a.Default.DispenseDotEnd.RiseDistance >>
			a.Default.DispenseDotEnd.RiseLowSpeed >>
			a.Default.DispenseDotEnd.RiseHightSpeed >>
			a.Default.DotSpeedSet.AccSpeed >>
			a.Default.DotSpeedSet.EndSpeed >>
			a.Default.DispenseLineSet.BeforeMoveDelay >>
			a.Default.DispenseLineSet.BeforeMoveDistance >>
			a.Default.DispenseLineSet.NodeTime >>
			a.Default.DispenseLineSet.StayTime >>
			a.Default.DispenseLineSet.ShutdownDistance >>
			a.Default.DispenseLineSet.ShutdownDelay >>
			a.Default.DispenseLineEnd.Type >>
			a.Default.DispenseLineEnd.LowSpeed >>
			a.Default.DispenseLineEnd.Height >>
			a.Default.DispenseLineEnd.Width >>
			a.Default.DispenseLineEnd.HighSpeed >>
			a.Default.LineSpeedSet.AccSpeed >>
			a.Default.LineSpeedSet.EndSpeed >>
			a.Default.ZSet.ZBackHeight >>
			a.Default.ZSet.ZBackType >>
			a.Default.GlueData.ParkPositionData.X >>
			a.Default.GlueData.ParkPositionData.Y >>
			a.Default.GlueData.ParkPositionData.Z >>
			a.Default.GlueData.GlueAuto >>
			a.Default.GlueData.GlueWaitTime >>
			a.Default.GlueData.GlueTime >>
			a.Default.GlueData.GlueStayTime >>
			a.VisionDefault.VisionSet.Accuracy >>
			a.VisionDefault.VisionSet.Speed >>
			a.VisionDefault.VisionSet.Score >>
			a.VisionDefault.VisionSet.width >>
			a.VisionDefault.VisionSet.height >>
			a.VisionDefault.VisionSet.Startangle >>
			a.VisionDefault.VisionSet.Endangle >>
			a.VisionDefault.VisionSerchError.SearchError >>
			a.RunLoopData.MaxRunNumber >>
			RunLoopNumber;
		File.Close();
		a.VisionDefault.VisionSerchError.pQuestion = new CQuestion();
	}
	else
	{
		a.Default.GoHome = { 30000,5000,7,47000,62000,10000 };
		a.Default.DotSpeedSet = { 100000,30000 };
		a.Default.LineSpeedSet = { 100000,30000 };
		a.Default.ZSet = { 5000,1 };
		a.VisionDefault.VisionSet = { 0,1,1,50,640,480,0,360,0,0 };
		a.IOParam.pEMGDlg = new CEmgDlg;
		a.IODetectionSwitch(TRUE, 0);
		a.VisionDefault.VisionSerchError.pQuestion = new CQuestion();
	}
}

/*******************************************************************************************視窗處理**********************************************************/
//非活動轉活動事件
int CCommandTestDlg::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
{
	if (m_pCameraDlg != NULL)
	{
		m_pCameraDlg->SetLayeredWindowAttributes(0, (255 * 70) / 100, LWA_ALPHA);
	}
	if (m_pDefaultDlg != NULL)
	{
		m_pDefaultDlg->SetLayeredWindowAttributes(0, (255 * 70) / 100, LWA_ALPHA);
	}
	if (m_pLaserDlg != NULL)
	{
		m_pLaserDlg->SetLayeredWindowAttributes(0, (255 * 70) / 100, LWA_ALPHA);
	}
	if (m_pPositionModifyDlg != NULL)
	{
		m_pPositionModifyDlg->SetLayeredWindowAttributes(0, (255 * 70) / 100, LWA_ALPHA);
	}
	if (m_pLaserAdjustDlg != NULL)
	{
		m_pLaserAdjustDlg->SetLayeredWindowAttributes(0, (255 * 70) / 100, LWA_ALPHA);
	}
	if (m_pLineContinuousDlg != NULL)
	{
		m_pLineContinuousDlg->SetLayeredWindowAttributes(0, (250 * 70) / 100, LWA_ALPHA);
	}
	return CDialogEx::OnMouseActivate(pDesktopWnd, nHitTest, message);
}
