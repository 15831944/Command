
// CommandTestDlg.cpp : 實作檔
//

#include "stdafx.h"
#include "CommandTest.h"
#include "CommandTestDlg.h"
#include "afxdialogex.h"
#include "mcc.h"
#include "Default.h"
#include "Camera.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CCommandTestDlg 對話方塊



CCommandTestDlg::CCommandTestDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_COMMANDTEST_DIALOG, pParent)
	, InputAuto(FALSE)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	TipOffset.x = 0;
	TipOffset.y = 0;
	CcdMode = FALSE;
	MaxRunNumber = 0;
	
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
	ON_BN_CLICKED(IDC_BTNDEFAULT, &CCommandTestDlg::OnBnClickedBtndefault)
	ON_BN_CLICKED(IDC_BTNVISION, &CCommandTestDlg::OnBnClickedBtnvision)
    ON_BN_CLICKED(IDC_BTNMODECHANGE, &CCommandTestDlg::OnBnClickedBtnmodechange)
    ON_BN_CLICKED(IDC_BTNCLEANCOUNT, &CCommandTestDlg::OnBnClickedBtncleancount)
    ON_BN_CLICKED(IDC_BTNMODEFYZ, &CCommandTestDlg::OnBnClickedBtnmodefyz)
END_MESSAGE_MAP()


// CCommandTestDlg 訊息處理常式
BOOL CCommandTestDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 設定此對話方塊的圖示。當應用程式的主視窗不是對話方塊時，
	// 框架會自動從事此作業
	SetIcon(m_hIcon, TRUE);			// 設定大圖示
	SetIcon(m_hIcon, FALSE);		// 設定小圖示
	// TODO: 在此加入額外的初始設定
	/*終端控制台開啟*/
	InitConsoleWindow();
	_cprintf("str = %s\n ", "Debug output goes to terminal\n");
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
	for (int i = 0; i < StrTable.GetSize(); i++) {
		m_ParamList.InsertItem(i, NULL);
		m_ParamList.SetItemText(i, 0, StrTable[i]);
		m_ParamList.SetItemText(i, 1, 0);
	}
	a.Default.GoHome = { 20000,1000,7,50000,10000,5000 };
	a.Default.DotSpeedSet = { 100000,30000 };
	a.Default.LineSpeedSet = { 100000,30000 };
	a.Default.ZSet = { 5000,1 };

	SetTimer(1, 500, NULL);
	//軸卡運動開啟
#ifdef MOVE
	MO_Open(1);
	MO_SetHardLim(7, 1);
	MO_SetDecOK(1);//開啟減速有效
	MO_SetSoftLim(7, 1);
	MO_SetCompSoft(1, -10, -10, -10);
	MO_SetCompSoft(0, 150000, 180000, 70000); //0, 150000, 190000, 80000
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

/*開始*/
void CCommandTestDlg::OnBnClickedStart()
{	
	if (m_LoopRun)
	{
		AfxBeginThread(RunThread, (LPVOID)this);
		RunSwitch = TRUE;
	}
	else
	{
		if ((MaxRunNumber - int(a.RunStatusRead.FinishProgramCount)) >= 0)
		{
			if (a.RunStatusRead.RunStatus == 0)
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
	RunSwitch = FALSE;
	a.Stop();
	SetDlgItemText(IDC_PAUSE, L"Pause");
}
/*清除陣列*/
void CCommandTestDlg::OnBnClickedOk()
{
	a.CommandMemory.clear();
	ListRefresh(NULL);
}
/*原點賦歸*/
void CCommandTestDlg::OnBnClickedBtnhome()
{
	a.Home();
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
	LONG DataArray[43] = { a.DispenseDotSet.GlueOpenTime, a.DispenseDotSet.GlueCloseTime,
		a.DispenseDotEnd.RiseDistance,a.DispenseDotEnd.RiseLowSpeed,a.DispenseDotEnd.RiseHightSpeed,
		a.DotSpeedSet.AccSpeed,a.DotSpeedSet.EndSpeed,
		a.DispenseLineSet.BeforeMoveDelay, a.DispenseLineSet.BeforeMoveDistance, a.DispenseLineSet.NodeTime, a.DispenseLineSet.StayTime, a.DispenseLineSet.ShutdownDistance, a.DispenseLineSet.ShutdownDelay,
		a.DispenseLineEnd.Type, a.DispenseLineEnd.HighSpeed, a.DispenseLineEnd.LowSpeed, a.DispenseLineEnd.Width, a.DispenseLineEnd.Height,
		a.LineSpeedSet.AccSpeed, a.LineSpeedSet.EndSpeed,
		a.ZSet.ZBackHeight,a.ZSet.ZBackType,
		a.GlueData.ParkPositionData.X,a.GlueData.ParkPositionData.Y,a.GlueData.ParkPositionData.Z,a.GlueData.GlueAuto,a.GlueData.GlueWaitTime,a.GlueData.GlueTime,a.GlueData.GlueStayTime,
		a.Time,a.VisionOffset.OffsetX,a.VisionOffset.OffsetY,a.VisionOffset.Angle,
		a.FiducialMark1.Point.X, a.FiducialMark1.Point.Y,a.FiducialMark1.OffsetX,a.FiducialMark1.OffsetY,
		a.FiducialMark2.Point.X, a.FiducialMark2.Point.Y,a.FiducialMark2.OffsetX,a.FiducialMark2.OffsetY,
		a.FinalWorkCoordinateData.X,a.FinalWorkCoordinateData.Y
	};
	
	int ArrayCount = 0;
	for (int i = 0; i < 41; i++)
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
		XYZlocation.Format(_T("機械手臂位置(X:%d,Y:%d,Z:%d),GlueStatus:%d"), MO_ReadLogicPosition(0), MO_ReadLogicPosition(1), MO_ReadLogicPosition(2), MO_ReadGumming());
	#endif 
	SetDlgItemText(IDC_ARMSTATUS, XYZlocation);
	/*程序運行狀態*/
	if (a.RunStatusRead.RunStatus == 0)
	{
		SetDlgItemText(IDC_RUNSTATUS, _T("程序運行狀態:未運行"));
	}
	else if (a.RunStatusRead.RunStatus == 2)
	{
		SetDlgItemText(IDC_PAUSE, L"Continue");
		SetDlgItemText(IDC_RUNSTATUS, _T("程序運行狀態:暫停中"));
	}
	else if (a.RunStatusRead.RunStatus == 1)
	{
		SetDlgItemText(IDC_RUNSTATUS, _T("程序運行狀態:運行中"));
	}
	/*程序計數*/
	FinishCountBuff.Format(_T("完整程序運行次數:%d"), a.RunStatusRead.FinishProgramCount);
	SetDlgItemText(IDC_FINISHCOUNT, FinishCountBuff);
	/*綠色原點賦歸按鈕*/
	#ifdef MOVE
	if (MO_ReadStartBtn())
	{
        a.Home();
	}
	#endif
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
	CDialogEx::OnTimer(nIDEvent);
}
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
/*插入*/
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
/*刪除*/
void CCommandTestDlg::OnDelete()
{
	if (!Insert)
	{
		int istat = m_CommandList.GetSelectionMark();//獲取選擇的項
		a.CommandMemory.erase(a.CommandMemory.begin() + istat);
		ListRefresh(NULL);
	}
}
/************************************************************命令*/
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
	StrBuff.Format(_T("FillArea,%d,%d,%d,%d,%d,%d,%d,%d,%d"), GetDlgItemInt(IDC_EDITPARAM1), GetDlgItemInt(IDC_EDITPARAM2), GetDlgItemInt(IDC_EDITPARAM3), GetDlgItemInt(IDC_EDITPARAM4) + OffsetX, GetDlgItemInt(IDC_EDITPARAM5) + OffsetY, GetDlgItemInt(IDC_EDITPARAM6), GetDlgItemInt(IDC_EDITPARAM7) + OffsetX, GetDlgItemInt(IDC_EDITPARAM8) + OffsetY, GetDlgItemInt(IDC_EDITPARAM9));
	(Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
	Insert = FALSE;
	ListRefresh(NULL);
}
/*回原點命令*/
void CCommandTestDlg::OnBnClickedBtncommand25()
{
	StrBuff = _T("GoHome");
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
	StrBuff.Format(_T("StepRepeatX,%d,%d,%d,%d,%d,%d,%d,%d,1-1,2-2,3-3"), GetDlgItemInt(IDC_EDITPARAM1), GetDlgItemInt(IDC_EDITPARAM2), GetDlgItemInt(IDC_EDITPARAM3), GetDlgItemInt(IDC_EDITPARAM4), GetDlgItemInt(IDC_EDITPARAM5), GetDlgItemInt(IDC_EDITPARAM6), GetDlgItemInt(IDC_EDITPARAM7), GetDlgItemInt(IDC_EDITPARAM8));
	(Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
	Insert = FALSE;
	ListRefresh(NULL);
}
/*步驟重複Y*/
void CCommandTestDlg::OnBnClickedBtncommand33()
{
	StrBuff.Format(_T("StepRepeatY,%d,%d,%d,%d,%d,%d"), GetDlgItemInt(IDC_EDITPARAM1), GetDlgItemInt(IDC_EDITPARAM2), GetDlgItemInt(IDC_EDITPARAM3), GetDlgItemInt(IDC_EDITPARAM4), GetDlgItemInt(IDC_EDITPARAM5), GetDlgItemInt(IDC_EDITPARAM6));
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
	// TODO: 在此加入控制項告知處理常式程式碼
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
/*******************************************************************************************外部設定**********************************************************/
/*Default設置*/
void CCommandTestDlg::OnBnClickedBtndefault()
{
	CDialog* m_pDlg;
	m_pDlg = new CDefault();
	m_pDlg->Create(IDD_DIALOG1, this);
	m_pDlg->ShowWindow(SW_SHOW);
}
/*影像操作*/
void CCommandTestDlg::OnBnClickedBtnvision()
{
	m_pCameraDlg = new CCamera();
	m_pCameraDlg->Create(IDD_DIALOG2, this);
	m_pCameraDlg->ShowWindow(SW_SHOW);
}
/*模式切換*/
void CCommandTestDlg::OnBnClickedBtnmodechange()
{
    CString StrBuff;
    GetDlgItemText(IDC_BUTTON1, StrBuff);
    if (StrBuff == L"切換CCD模式")
    {
        CcdMode = TRUE;
        OffsetX = TipOffset.x;
        OffsetY = TipOffset.y;
        SetDlgItemText(IDC_BUTTON1, _T("切換Tip模式"));
    }
    else
    {
        CcdMode = FALSE;
        OffsetX = 0;
        OffsetY = 0;
        SetDlgItemText(IDC_BUTTON1, _T("切換CCD模式"));
    }
}
/*修改點線段Z值*/
void CCommandTestDlg::OnBnClickedBtnmodefyz()
{
    CString StrBuff;
    for (int i = 0; i < a.CommandMemory.size(); i++)
    {
        if (CommandResolve(a.CommandMemory.at(i), 0) == L"Dot" ||
            CommandResolve(a.CommandMemory.at(i), 0) == L"LineStart" ||
            CommandResolve(a.CommandMemory.at(i), 0) == L"LinePassing" ||
            CommandResolve(a.CommandMemory.at(i), 0) == L"LineEnd" ||
            CommandResolve(a.CommandMemory.at(i), 0) == L"ArcPoint" ||
            CommandResolve(a.CommandMemory.at(i), 0) == L"WaitPoint" ||
            CommandResolve(a.CommandMemory.at(i), 0) == L"VirtualPoint" ||
            CommandResolve(a.CommandMemory.at(i), 0) == L"StopPoint")
        {
            StrBuff.Format(_T(",%d,%d,%d"), _ttol(CommandResolve(a.CommandMemory.at(i), 1)), _ttol(CommandResolve(a.CommandMemory.at(i), 2)), GetDlgItemInt(IDC_EDITPARAM1));
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
/*RunRepeat執行緒*/
UINT CCommandTestDlg::RunThread(LPVOID pParam)
{
	while (((CCommandTestDlg*)pParam)->RunSwitch)
	{
		_cprintf("%d", (((CCommandTestDlg*)pParam)->MaxRunNumber - int(((CCommandTestDlg*)pParam)->a.RunStatusRead.FinishProgramCount)));
		if ((((CCommandTestDlg*)pParam)->MaxRunNumber - int(((CCommandTestDlg*)pParam)->a.RunStatusRead.FinishProgramCount)) >= 0)
		{
			if (((CCommandTestDlg*)pParam)->a.RunStatusRead.RunStatus == 0)
			{
				/*列表停用*/
				((CCommandTestDlg*)pParam)->m_CommandList.EnableWindow(FALSE);
				/*列表改為單選*/
				DWORD dwStyle = ((CCommandTestDlg*)pParam)->m_CommandList.GetExtendedStyle();
				dwStyle |= LVS_SHOWSELALWAYS;
				((CCommandTestDlg*)pParam)->m_CommandList.SetExtendedStyle(dwStyle); //設置擴展風格
																					 /*設置Model當前目錄*/
				CString path;
				GetModuleFileName(NULL, path.GetBufferSetLength(MAX_PATH + 1), MAX_PATH);
				path.ReleaseBuffer();
				int pos = path.ReverseFind('\\');
				path = path.Left(pos) + _T("\\Temp\\");
				LPTSTR lpszText = new TCHAR[path.GetLength() + 1];
				lstrcpy(lpszText, path);
				((CCommandTestDlg*)pParam)->a.VisionDefault.VisionFile.ModelPath = lpszText;
				/*運行*/
				((CCommandTestDlg*)pParam)->a.Run();
			}  
		}
		else
		{
			((CCommandTestDlg*)pParam)->RunSwitch = FALSE;
		}
		Sleep(10);
	}
	return 0;
}










