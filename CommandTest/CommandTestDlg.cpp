
// CommandTestDlg.cpp : 實作檔
//

#include "stdafx.h"
#include "CommandTest.h"
#include "CommandTestDlg.h"
#include "afxdialogex.h"
#include "mcc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CCommandTestDlg 對話方塊



CCommandTestDlg::CCommandTestDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_COMMANDTEST_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CCommandTestDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST1, m_CommandList);
}

BEGIN_MESSAGE_MAP(CCommandTestDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDC_START, &CCommandTestDlg::OnBnClickedStart)
    ON_WM_TIMER()
    ON_BN_CLICKED(IDC_PAUSE, &CCommandTestDlg::OnBnClickedPause)
    ON_BN_CLICKED(IDC_STOP, &CCommandTestDlg::OnBnClickedStop)
    ON_BN_CLICKED(IDOK, &CCommandTestDlg::OnBnClickedOk)
    ON_BN_CLICKED(IDC_BTNCOMMAND27, &CCommandTestDlg::OnBnClickedBtncommand27)
    ON_BN_CLICKED(IDC_BTNCOMMAND1, &CCommandTestDlg::OnBnClickedBtncommand1)
    ON_BN_CLICKED(IDC_BTNCOMMAND2, &CCommandTestDlg::OnBnClickedBtncommand2)
    ON_BN_CLICKED(IDC_BTNHOME, &CCommandTestDlg::OnBnClickedBtnhome)
    ON_BN_CLICKED(IDC_BTNCOMMAND3, &CCommandTestDlg::OnBnClickedBtncommand3)
    ON_BN_CLICKED(IDC_BTNCOMMAND4, &CCommandTestDlg::OnBnClickedBtncommand4)
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
    ON_BN_CLICKED(IDC_BTNCOMMAND20, &CCommandTestDlg::OnBnClickedBtncommand20)
    ON_BN_CLICKED(IDC_BTNCOMMAND21, &CCommandTestDlg::OnBnClickedBtncommand21)
    ON_BN_CLICKED(IDC_BTNCOMMAND22, &CCommandTestDlg::OnBnClickedBtncommand22)
    ON_BN_CLICKED(IDC_BTNCOMMAND23, &CCommandTestDlg::OnBnClickedBtncommand23)
    ON_BN_CLICKED(IDC_BTNCOMMAND24, &CCommandTestDlg::OnBnClickedBtncommand24)
    ON_BN_CLICKED(IDC_BTNCOMMAND25, &CCommandTestDlg::OnBnClickedBtncommand25)
    ON_BN_CLICKED(IDC_BTNCOMMAND26, &CCommandTestDlg::OnBnClickedBtncommand26)
    ON_BN_CLICKED(IDC_BTNCOMMAND28, &CCommandTestDlg::OnBnClickedBtncommand28)
    ON_BN_CLICKED(IDC_BTNCOMMAND29, &CCommandTestDlg::OnBnClickedBtncommand29)
    ON_BN_CLICKED(IDC_BTNCOMMAND30, &CCommandTestDlg::OnBnClickedBtncommand30)
    ON_BN_CLICKED(IDC_BTNCOMMAND31, &CCommandTestDlg::OnBnClickedBtncommand31)
    ON_BN_CLICKED(IDC_BTNCOMMAND32, &CCommandTestDlg::OnBnClickedBtncommand32)
    ON_BN_CLICKED(IDC_BTNCOMMAND33, &CCommandTestDlg::OnBnClickedBtncommand33)
    ON_BN_CLICKED(IDC_BTNCOMMAND34, &CCommandTestDlg::OnBnClickedBtncommand34)
    ON_BN_CLICKED(IDC_BTNCOMMAND35, &CCommandTestDlg::OnBnClickedBtncommand35)
    ON_BN_CLICKED(IDC_BTNCOMMAND36, &CCommandTestDlg::OnBnClickedBtncommand36)
    ON_BN_CLICKED(IDC_BTNCOMMAND37, &CCommandTestDlg::OnBnClickedBtncommand37)
    ON_BN_CLICKED(IDC_BTNCOMMAND38, &CCommandTestDlg::OnBnClickedBtncommand38)
    ON_BN_CLICKED(IDC_BTNCOMMAND39, &CCommandTestDlg::OnBnClickedBtncommand39)
    ON_BN_CLICKED(IDC_BTNCOMMAND40, &CCommandTestDlg::OnBnClickedBtncommand40)
    ON_BN_CLICKED(IDC_BTNCOMMAND41, &CCommandTestDlg::OnBnClickedBtncommand41)
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
    m_CommandList.InsertColumn(0, _T("編號"), LVCFMT_CENTER, 36, -1);
    m_CommandList.InsertColumn(1, _T("Command"), LVCFMT_CENTER, 250, -1);
    SetTimer(1, 100, NULL);
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

void CCommandTestDlg::OnBnClickedStart()
{
    a.Run();
}
void CCommandTestDlg::OnBnClickedPause()
{
    CString StrBuff;
    GetDlgItemText(IDC_PAUSE, StrBuff);
    if (StrBuff == L"Pause")
    {
        a.Pause();
        SetDlgItemText(IDC_PAUSE, L"Continue");
    }
    else
    {
        a.Continue();
        SetDlgItemText(IDC_PAUSE, L"Pause");
    }
}
void CCommandTestDlg::OnBnClickedStop()
{
    a.Stop();
    SetDlgItemText(IDC_PAUSE, L"Pause");
}
/*清除陣列*/
void CCommandTestDlg::OnBnClickedOk()
{
    a.Command.clear();
    ListRefresh(NULL);
}
/*原點賦歸*/
void CCommandTestDlg::OnBnClickedBtnhome()
{
#ifdef MOVE
    MO_Open(1);
    MO_SetHardLim(7, 1);
    MO_SetDecOK(1);//開啟減速有效 
    MO_MoveToHome(20000, 1000, 7, 0);//原點復歸
#endif
}
void CCommandTestDlg::OnTimer(UINT_PTR nIDEvent)
{
    CString StrBuff;
    StrBuff.Format(_T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"), 
        a.Time, a.RunCount,
        a.DispenseDotEnd.RiseDistance,a.DispenseDotEnd.RiseHightSpeed,a.DispenseDotEnd.RiseLowSpeed,
        a.DispenseDotSet.GlueCloseTime,a.DispenseDotSet.GlueOpenTime,
        a.DotSpeedSet.AccSpeed,a.DotSpeedSet.EndSpeed,
        a.Command.size());
    StrBuff = StrBuff + a.Program.LabelName;
    SetDlgItemText(IDC_EDIT1, StrBuff);
    CDialogEx::OnTimer(nIDEvent);
}
void CCommandTestDlg::ListRefresh(BOOL ScrollBarRefresh) {
    CString StrBuff;
    m_CommandList.DeleteAllItems();
    int nCount = a.Command.size();
    for (int i = 0; i < nCount; i++) {
        m_CommandList.InsertItem(i, NULL);
        (i>9) ? StrBuff.Format(_T("0%d"), i+1) : StrBuff.Format(_T("00%d"), i+1);
        m_CommandList.SetItemText(i, 0, StrBuff);
        m_CommandList.SetItemText(i, 1, a.Command.at(i));
    }
}
/************************************************************命令*/
/*單點點膠*/
void CCommandTestDlg::OnBnClickedBtncommand1()
{
    a.Command.push_back(_T("DispenseDot,10000,10000,50000"));
    ListRefresh(NULL);
}
/*單點點膠設置*/
void CCommandTestDlg::OnBnClickedBtncommand2()
{
    a.Command.push_back(_T("DispenseDotSet,1000,1000"));
    ListRefresh(NULL);
}
/*單點結束設置*/
void CCommandTestDlg::OnBnClickedBtncommand3()
{
    a.Command.push_back(_T("DispenseDotEnd,1000,100,10000"));
    ListRefresh(NULL);
}
/*PTP驅動速度*/
void CCommandTestDlg::OnBnClickedBtncommand4()
{
    a.Command.push_back(_T("DotSpeedSet,30000,100000"));
    ListRefresh(NULL);
}
/*線段開始*/
void CCommandTestDlg::OnBnClickedBtncommand5()
{
    a.Command.push_back(_T("Line Start,20000,20000,600000"));
    ListRefresh(NULL);
}
/*線段中間點*/
void CCommandTestDlg::OnBnClickedBtncommand6()
{
    a.Command.push_back(_T("Line Passing,20000,30000,600000"));
    ListRefresh(NULL);
}
/*線段結束點*/
void CCommandTestDlg::OnBnClickedBtncommand7()
{
    a.Command.push_back(_T("Line End,30000,20000,600000"));
    ListRefresh(NULL);
}
/*圓弧中點*/
void CCommandTestDlg::OnBnClickedBtncommand8()
{
    // TODO: 在此加入控制項告知處理常式程式碼
}
/*圓中點*/
void CCommandTestDlg::OnBnClickedBtncommand9()
{
    // TODO: 在此加入控制項告知處理常式程式碼
}
/*圓中點2*/
void CCommandTestDlg::OnBnClickedBtncommand10()
{
    // TODO: 在此加入控制項告知處理常式程式碼
}
/*線段塗膠設置*/
void CCommandTestDlg::OnBnClickedBtncommand11()
{
    // TODO: 在此加入控制項告知處理常式程式碼
}
/*線段速度設置*/
void CCommandTestDlg::OnBnClickedBtncommand12()
{
    // TODO: 在此加入控制項告知處理常式程式碼
}
/*塗膠結束設置*/
void CCommandTestDlg::OnBnClickedBtncommand13()
{
    // TODO: 在此加入控制項告知處理常式程式碼
}
/*Z軸工作高度*/
void CCommandTestDlg::OnBnClickedBtncommand14()
{
    // TODO: 在此加入控制項告知處理常式程式碼
}
/*加速度設置*/
void CCommandTestDlg::OnBnClickedBtncommand15()
{
    // TODO: 在此加入控制項告知處理常式程式碼
}
/*參數初始化*/
void CCommandTestDlg::OnBnClickedBtncommand16()
{
    // TODO: 在此加入控制項告知處理常式程式碼
}
/*輸入*/
void CCommandTestDlg::OnBnClickedBtncommand17()
{
    // TODO: 在此加入控制項告知處理常式程式碼
}
/*輸出*/
void CCommandTestDlg::OnBnClickedBtncommand18()
{
    // TODO: 在此加入控制項告知處理常式程式碼
}
/*點膠機開關*/
void CCommandTestDlg::OnBnClickedBtncommand19()
{
    // TODO: 在此加入控制項告知處理常式程式碼
}
/*虛擬點*/
void CCommandTestDlg::OnBnClickedBtncommand20()
{
    // TODO: 在此加入控制項告知處理常式程式碼
}
/*等待點*/
void CCommandTestDlg::OnBnClickedBtncommand21()
{
    // TODO: 在此加入控制項告知處理常式程式碼
}
/*停駐點*/
void CCommandTestDlg::OnBnClickedBtncommand22()
{
    // TODO: 在此加入控制項告知處理常式程式碼
}
/*停止點*/
void CCommandTestDlg::OnBnClickedBtncommand23()
{
    // TODO: 在此加入控制項告知處理常式程式碼
}
/*填充區域*/
void CCommandTestDlg::OnBnClickedBtncommand24()
{
    // TODO: 在此加入控制項告知處理常式程式碼
}
/*回原點命令*/
void CCommandTestDlg::OnBnClickedBtncommand25()
{
    // TODO: 在此加入控制項告知處理常式程式碼
}
/*標籤*/           
void CCommandTestDlg::OnBnClickedBtncommand26()
{
    a.Command.push_back(_T("Label,100"));
    ListRefresh(NULL);
}
/*前往地址*/
void CCommandTestDlg::OnBnClickedBtncommand27()
{
    a.Command.push_back(_T("GotoAddress,3"));
    ListRefresh(NULL);
}
/*前往標籤*/
void CCommandTestDlg::OnBnClickedBtncommand28()
{
    a.Command.push_back(_T("GotoLabel,100"));
    ListRefresh(NULL);
}
/*調用子程序*/
void CCommandTestDlg::OnBnClickedBtncommand29()
{
    // TODO: 在此加入控制項告知處理常式程式碼
}
/*結束子程序*/
void CCommandTestDlg::OnBnClickedBtncommand30()
{
    // TODO: 在此加入控制項告知處理常式程式碼
}
/*循環地址*/
void CCommandTestDlg::OnBnClickedBtncommand31()
{
    // TODO: 在此加入控制項告知處理常式程式碼
}
/*步驟重複X*/
void CCommandTestDlg::OnBnClickedBtncommand32()
{
    // TODO: 在此加入控制項告知處理常式程式碼
}
/*步驟重複Y*/
void CCommandTestDlg::OnBnClickedBtncommand33()
{
    // TODO: 在此加入控制項告知處理常式程式碼
}
/*查找標記*/
void CCommandTestDlg::OnBnClickedBtncommand34()
{
    // TODO: 在此加入控制項告知處理常式程式碼
}
/*基準標記*/
void CCommandTestDlg::OnBnClickedBtncommand35()
{
    // TODO: 在此加入控制項告知處理常式程式碼
}
/*查找標記調整*/
void CCommandTestDlg::OnBnClickedBtncommand36()
{
    // TODO: 在此加入控制項告知處理常式程式碼
}
/*基準標記調整*/
void CCommandTestDlg::OnBnClickedBtncommand37()
{
    // TODO: 在此加入控制項告知處理常式程式碼
}
/*查找角度標記*/
void CCommandTestDlg::OnBnClickedBtncommand38()
{
    // TODO: 在此加入控制項告知處理常式程式碼
}
/*光源*/
void CCommandTestDlg::OnBnClickedBtncommand39()
{
    // TODO: 在此加入控制項告知處理常式程式碼
}
/*CCD觸發器*/
void CCommandTestDlg::OnBnClickedBtncommand40()
{
    // TODO: 在此加入控制項告知處理常式程式碼
}
/*擴展步驟重複*/
void CCommandTestDlg::OnBnClickedBtncommand41()
{
    // TODO: 在此加入控制項告知處理常式程式碼
}
