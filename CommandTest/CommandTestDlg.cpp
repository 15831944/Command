
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


    ON_BN_CLICKED(IDC_BTNCOMMAND42, &CCommandTestDlg::OnBnClickedBtncommand42)
    ON_BN_CLICKED(IDC_BTNCOMMAND43, &CCommandTestDlg::OnBnClickedBtncommand43)
    ON_BN_CLICKED(IDC_BTNCOMMAND44, &CCommandTestDlg::OnBnClickedBtncommand44)
    ON_BN_CLICKED(IDC_BTNCOMMAND45, &CCommandTestDlg::OnBnClickedBtncommand45)
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
    m_CommandList.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
    m_CommandList.InsertColumn(0, _T("編號"), LVCFMT_CENTER, 36, -1);
    m_CommandList.InsertColumn(1, _T("Command"), LVCFMT_LEFT, 300, -1);
    SetTimer(1, 100, NULL);
#ifdef MOVE
    MO_Open(1);
    MO_SetHardLim(7, 1);
    MO_SetDecOK(1);//開啟減速有效
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
    MO_MoveToHome(20000, 1000, 7, 0);//原點復歸
#endif
}
void CCommandTestDlg::OnTimer(UINT_PTR nIDEvent)
{
    CString DotStrBuff, LinStrBuff, RunStrBuff;
    CString XYZlocation;
    DotStrBuff.Format(_T("第一階段抬升距離:%d,高速:%d,低速:%d\r\t\t關閉時間:%d,開啟時間:%d,加速:%d,驅動:%d\r\n"),
        a.DispenseDotEnd.RiseDistance,a.DispenseDotEnd.RiseHightSpeed,a.DispenseDotEnd.RiseLowSpeed,
        a.DispenseDotSet.GlueCloseTime,a.DispenseDotSet.GlueOpenTime,
        a.DotSpeedSet.AccSpeed,a.DotSpeedSet.EndSpeed
    );
    LinStrBuff.Format(_T("運動狀態:%d,線段開始狀態:%d,圓弧狀態:%d,圓狀態:%d\r\t前延遲:%d,前距離:%d,節點:%d,停留:%d,後延遲:%d後距離:%d\r\n類型:%d,高速:%d,低速:%d,長度:%d,高度:%d\r\t\t加速:%d,驅動:%d\r\n"),
        a.RunData.ActionStatus, a.StartData.at(0).Status, a.ArcData.at(0).Status,a.CircleData1.at(0).Status,
        a.DispenseLineSet.BeforeMoveDelay, a.DispenseLineSet.BeforeMoveDistance, a.DispenseLineSet.NodeTime, a.DispenseLineSet.StayTime, a.DispenseLineSet.ShutdownDelay, a.DispenseLineSet.ShutdownDistance,
        a.DispenseLineEnd.Type, a.DispenseLineEnd.HighSpeed, a.DispenseLineEnd.LowSpeed, a.DispenseLineEnd.Width, a.DispenseLineEnd.Height,
        a.LineSpeedSet.AccSpeed, a.LineSpeedSet.EndSpeed
    );
    RunStrBuff = DotStrBuff + LinStrBuff;//檢測標籤用 
    SetDlgItemText(IDC_EDIT1, RunStrBuff);
    if (a.RunData.RunStatus == 2)
    {
        SetDlgItemText(IDC_PAUSE, L"Continue");
    }
#ifdef MOVE
    XYZlocation.Format(_T("X:%d,Y:%d,Z:%d,GlueStatus:%d"), MO_ReadLogicPosition(0), MO_ReadLogicPosition(1), MO_ReadLogicPosition(2), MO_ReadGumming());
#endif 
    SetDlgItemText(IDC_EDIT2, XYZlocation);
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
        for (UINT i = 0; i < a.Command.size(); i++)
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
        m_CommandList.SetItemText(1, 1, StrBuff);

    }
    
}
/*列表點下左鍵*/
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
    StrBuff.Format(_T("Dot,%d,%d,%d"),GetDlgItemInt(IDC_EDITPARAM1), GetDlgItemInt(IDC_EDITPARAM2), GetDlgItemInt(IDC_EDITPARAM3));
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
    StrBuff.Format(_T("LineStart,%d,%d,%d"), GetDlgItemInt(IDC_EDITPARAM1), GetDlgItemInt(IDC_EDITPARAM2), GetDlgItemInt(IDC_EDITPARAM3));
    (Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
    Insert = FALSE;
    ListRefresh(NULL);
}
/*線段中間點*/
void CCommandTestDlg::OnBnClickedBtncommand6()
{
    StrBuff.Format(_T("LinePassing,%d,%d,%d"), GetDlgItemInt(IDC_EDITPARAM1), GetDlgItemInt(IDC_EDITPARAM2), GetDlgItemInt(IDC_EDITPARAM3));
    (Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
    Insert = FALSE;
    ListRefresh(NULL);
}
/*線段結束點*/
void CCommandTestDlg::OnBnClickedBtncommand7()
{
    StrBuff.Format(_T("LineEnd,%d,%d,%d"), GetDlgItemInt(IDC_EDITPARAM1), GetDlgItemInt(IDC_EDITPARAM2), GetDlgItemInt(IDC_EDITPARAM3));
    (Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
    Insert = FALSE;
    ListRefresh(NULL);
}
/*圓弧中點*/
void CCommandTestDlg::OnBnClickedBtncommand8()
{
    StrBuff.Format(_T("ArcPoint,%d,%d,%d"), GetDlgItemInt(IDC_EDITPARAM1), GetDlgItemInt(IDC_EDITPARAM2), GetDlgItemInt(IDC_EDITPARAM3));
    (Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
    Insert = FALSE;
    ListRefresh(NULL);
}
/*圓中點*/
void CCommandTestDlg::OnBnClickedBtncommand9()
{
    StrBuff.Format(_T("CirclePoint,%d,%d,%d,%d,%d,%d"), GetDlgItemInt(IDC_EDITPARAM1), GetDlgItemInt(IDC_EDITPARAM2), GetDlgItemInt(IDC_EDITPARAM3), GetDlgItemInt(IDC_EDITPARAM4), GetDlgItemInt(IDC_EDITPARAM5), GetDlgItemInt(IDC_EDITPARAM6));
    (Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
    Insert = FALSE;
    ListRefresh(NULL);
}
/*圓中點2*/
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
    StrBuff.Format(_T("VirtualPoint,%d,%d,%d"), GetDlgItemInt(IDC_EDITPARAM1), GetDlgItemInt(IDC_EDITPARAM2), GetDlgItemInt(IDC_EDITPARAM3));
    (Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
    Insert = FALSE;
    ListRefresh(NULL);
}
/*等待點*/
void CCommandTestDlg::OnBnClickedBtncommand21()
{
    StrBuff.Format(_T("WaitPoint,%d,%d,%d,%d"), GetDlgItemInt(IDC_EDITPARAM1), GetDlgItemInt(IDC_EDITPARAM2), GetDlgItemInt(IDC_EDITPARAM3), GetDlgItemInt(IDC_EDITPARAM4));
    (Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
    Insert = FALSE;
    ListRefresh(NULL);
}
/*停駐點*/
void CCommandTestDlg::OnBnClickedBtncommand22()
{
    StrBuff.Format(_T("ParkPoint,%d,%d,%d"), GetDlgItemInt(IDC_EDITPARAM1), GetDlgItemInt(IDC_EDITPARAM2), GetDlgItemInt(IDC_EDITPARAM3));
    (Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
    Insert = FALSE;
    ListRefresh(NULL);
}
/*停止點*/
void CCommandTestDlg::OnBnClickedBtncommand23()
{
    StrBuff.Format(_T("StopPoint,%d,%d,%d"), GetDlgItemInt(IDC_EDITPARAM1), GetDlgItemInt(IDC_EDITPARAM2), GetDlgItemInt(IDC_EDITPARAM3));
    (Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
    Insert = FALSE;
    ListRefresh(NULL);
}
/*填充區域*/
void CCommandTestDlg::OnBnClickedBtncommand24()
{
    StrBuff.Format(_T("FillArea,%d,%d,%d,%d,%d,%d,50000,50000,60000"), GetDlgItemInt(IDC_EDITPARAM1), GetDlgItemInt(IDC_EDITPARAM2), GetDlgItemInt(IDC_EDITPARAM3), GetDlgItemInt(IDC_EDITPARAM4), GetDlgItemInt(IDC_EDITPARAM5), GetDlgItemInt(IDC_EDITPARAM6));
    (Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
    Insert = FALSE;
    ListRefresh(NULL);
}
/*回原點命令*/
void CCommandTestDlg::OnBnClickedBtncommand25()
{
    StrBuff = _T("HomePoint");
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
/*排膠設置*/
void CCommandTestDlg::OnBnClickedBtncommand42()
{
    a.GlueData.GlueTime = GetDlgItemInt(IDC_EDITPARAM1);
    a.GlueData.GlueStayTime = GetDlgItemInt(IDC_EDITPARAM2);
}

/*測試*/
void CCommandTestDlg::OnBnClickedBtnview()
{
    a.CommandMemory.push_back(_T("End"));
    a.MainSubroutineSeparate();
    ListRefresh(TRUE);
}







