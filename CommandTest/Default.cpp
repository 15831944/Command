// Default.cpp : 實作檔
//

#include "stdafx.h"
#include "CommandTest.h"
#include "Default.h"
#include "afxdialogex.h"
#include "CommandTestDlg.h"
#include "Question.h"

#define IDC_EDITBUFF  10000
// CDefault 對話方塊

IMPLEMENT_DYNAMIC(CDefault, CDialogEx)

CDefault::CDefault(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG1, pParent)
{

}

CDefault::~CDefault()
{
}

void CDefault::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_ListDefault);
}


BEGIN_MESSAGE_MAP(CDefault, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON2, &CDefault::OnBnClickedButton2)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST1, &CDefault::OnNMDblclkList1)
	ON_NOTIFY(NM_CLICK, IDC_LIST1, &CDefault::OnNMClickList1)
	ON_NOTIFY(NM_RCLICK, IDC_LIST1, &CDefault::OnNMRClickList1)
	ON_WM_SHOWWINDOW()
	ON_WM_MOUSEACTIVATE()
END_MESSAGE_MAP()


// CDefault 訊息處理常式

BOOL CDefault::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	m_ListDefault.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
	m_ListDefault.InsertColumn(0, _T("名稱"), LVCFMT_CENTER, 100, -1);
	m_ListDefault.InsertColumn(1, _T("參數值"), LVCFMT_LEFT, 150, -1);
	CStringArray StrTable;
	StrTable.Add(_T("原點復歸速度1")); StrTable.Add(_T("原點復歸速度2")); StrTable.Add(_T("原點復歸軸數")); StrTable.Add(_T("原點復歸X偏差")); StrTable.Add(_T("原點復歸Y偏差")); StrTable.Add(_T("原點復歸Z偏差")); StrTable.Add(_T("迴圈賦歸"));
	StrTable.Add(_T("點膠出膠時間")); StrTable.Add(_T("點膠停留時間"));
	StrTable.Add(_T("點膠回程距離")); StrTable.Add(_T("點膠低回程速度")); StrTable.Add(_T("點膠高回程速度"));
	StrTable.Add(_T("點膠加速度")); StrTable.Add(_T("點膠驅動速度"));
	StrTable.Add(_T("線段移動前延遲")); StrTable.Add(_T("線段設置距離")); StrTable.Add(_T("線段節點時間")); StrTable.Add(_T("線段停留時間")); StrTable.Add(_T("線段關機距離")); StrTable.Add(_T("線段關機延遲"));
	StrTable.Add(_T("線段返回類型")); StrTable.Add(_T("線段返回低速")); StrTable.Add(_T("線段返回高度")); StrTable.Add(_T("線段返回長度")); StrTable.Add(_T("線段返回高速"));
	StrTable.Add(_T("線段加速度")); StrTable.Add(_T("線段驅動速度"));
	StrTable.Add(_T("Z軸回升距離")); StrTable.Add(_T("Z軸回升型態"));
	StrTable.Add(_T("停駐點X")); StrTable.Add(_T("停駐點Y")); StrTable.Add(_T("停駐點Z"));
	StrTable.Add(_T("排膠開關")); StrTable.Add(_T("排膠等待時間")); StrTable.Add(_T("排膠時間")); StrTable.Add(_T("排膠後停留時間"));
	StrTable.Add(_T("影像尋找精度")); StrTable.Add(_T("影像尋找速度")); StrTable.Add(_T("影像比對分數"));
	StrTable.Add(_T("影像尋找寬度")); StrTable.Add(_T("影像尋找高度")); StrTable.Add(_T("影像尋找起始角度")); StrTable.Add(_T("影像尋找結束角度")); StrTable.Add(_T("影像未搜尋到方法"));
	StrTable.Add(_T("最大運行次數")); StrTable.Add(_T("循環次數"));
	
	for (int i = 0; i < 46; i++) {
		m_ListDefault.InsertItem(i, NULL);
		m_ListDefault.SetItemText(i, 0, StrTable[i]);
	}
	ParamShow();
	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX 屬性頁應傳回 FALSE
}
void CDefault::ParamShow()
{
	CWnd* pMain = AfxGetApp()->m_pMainWnd;
	LONG ParamArray[46];
	ParamArray[0] = ((CCommandTestDlg*)pMain)->a.Default.GoHome.Speed1;
	ParamArray[1] = ((CCommandTestDlg*)pMain)->a.Default.GoHome.Speed2;
	ParamArray[2] = ((CCommandTestDlg*)pMain)->a.Default.GoHome.Axis;
	ParamArray[3] = ((CCommandTestDlg*)pMain)->a.Default.GoHome.MoveX;
	ParamArray[4] = ((CCommandTestDlg*)pMain)->a.Default.GoHome.MoveY;
	ParamArray[5] = ((CCommandTestDlg*)pMain)->a.Default.GoHome.MoveZ;
	ParamArray[6] = ((CCommandTestDlg*)pMain)->a.Default.GoHome.PrecycleInitialize;
	ParamArray[7] = ((CCommandTestDlg*)pMain)->a.Default.DispenseDotSet.GlueOpenTime;
	ParamArray[8] = ((CCommandTestDlg*)pMain)->a.Default.DispenseDotSet.GlueCloseTime;
	ParamArray[9] = ((CCommandTestDlg*)pMain)->a.Default.DispenseDotEnd.RiseDistance;
	ParamArray[10] = ((CCommandTestDlg*)pMain)->a.Default.DispenseDotEnd.RiseLowSpeed;
	ParamArray[11] = ((CCommandTestDlg*)pMain)->a.Default.DispenseDotEnd.RiseHightSpeed;
	ParamArray[12] = ((CCommandTestDlg*)pMain)->a.Default.DotSpeedSet.AccSpeed;
	ParamArray[13] = ((CCommandTestDlg*)pMain)->a.Default.DotSpeedSet.EndSpeed;
	ParamArray[14] = ((CCommandTestDlg*)pMain)->a.Default.DispenseLineSet.BeforeMoveDelay;
	ParamArray[15] = ((CCommandTestDlg*)pMain)->a.Default.DispenseLineSet.BeforeMoveDistance;
	ParamArray[16] = ((CCommandTestDlg*)pMain)->a.Default.DispenseLineSet.NodeTime;
	ParamArray[17] = ((CCommandTestDlg*)pMain)->a.Default.DispenseLineSet.StayTime;
	ParamArray[18] = ((CCommandTestDlg*)pMain)->a.Default.DispenseLineSet.ShutdownDistance;
	ParamArray[19] = ((CCommandTestDlg*)pMain)->a.Default.DispenseLineSet.ShutdownDelay;
	ParamArray[20] = ((CCommandTestDlg*)pMain)->a.Default.DispenseLineEnd.Type;
	ParamArray[21] = ((CCommandTestDlg*)pMain)->a.Default.DispenseLineEnd.LowSpeed;
	ParamArray[22] = ((CCommandTestDlg*)pMain)->a.Default.DispenseLineEnd.Height;
	ParamArray[23] = ((CCommandTestDlg*)pMain)->a.Default.DispenseLineEnd.Width;
	ParamArray[24] = ((CCommandTestDlg*)pMain)->a.Default.DispenseLineEnd.HighSpeed;
	ParamArray[25] = ((CCommandTestDlg*)pMain)->a.Default.LineSpeedSet.AccSpeed;
	ParamArray[26] = ((CCommandTestDlg*)pMain)->a.Default.LineSpeedSet.EndSpeed;
	ParamArray[27] = ((CCommandTestDlg*)pMain)->a.Default.ZSet.ZBackHeight;
	ParamArray[28] = ((CCommandTestDlg*)pMain)->a.Default.ZSet.ZBackType;
	ParamArray[29] = ((CCommandTestDlg*)pMain)->a.Default.GlueData.ParkPositionData.X;
	ParamArray[30] = ((CCommandTestDlg*)pMain)->a.Default.GlueData.ParkPositionData.Y;
	ParamArray[31] = ((CCommandTestDlg*)pMain)->a.Default.GlueData.ParkPositionData.Z;
	ParamArray[32] = ((CCommandTestDlg*)pMain)->a.Default.GlueData.GlueAuto;
	ParamArray[33] = ((CCommandTestDlg*)pMain)->a.Default.GlueData.GlueWaitTime;
	ParamArray[34] = ((CCommandTestDlg*)pMain)->a.Default.GlueData.GlueTime;
	ParamArray[35] = ((CCommandTestDlg*)pMain)->a.Default.GlueData.GlueStayTime;
	ParamArray[36] = ((CCommandTestDlg*)pMain)->a.VisionDefault.VisionSet.Accuracy;
	ParamArray[37] = ((CCommandTestDlg*)pMain)->a.VisionDefault.VisionSet.Speed;
	ParamArray[38] = ((CCommandTestDlg*)pMain)->a.VisionDefault.VisionSet.Score;
	ParamArray[39] = ((CCommandTestDlg*)pMain)->a.VisionDefault.VisionSet.width;
	ParamArray[40] = ((CCommandTestDlg*)pMain)->a.VisionDefault.VisionSet.height;
	ParamArray[41] = ((CCommandTestDlg*)pMain)->a.VisionDefault.VisionSet.Startangle;
	ParamArray[42] = ((CCommandTestDlg*)pMain)->a.VisionDefault.VisionSet.Endangle;
	ParamArray[43] = ((CCommandTestDlg*)pMain)->a.VisionDefault.VisionSerchError.SearchError;
	ParamArray[44] = ((CCommandTestDlg*)pMain)->a.RunLoopData.MaxRunNumber;
	ParamArray[45] = ((CCommandTestDlg*)pMain)->RunLoopNumber;
	for (int i = 0; i < 46; i++) {
		CString StrBuff;
		StrBuff.Format(_T("%d"), ParamArray[i]);
		m_ListDefault.SetItemText(i, 1, StrBuff);
	}
}
void CDefault::ParamModify()
{
	CWnd* pMain = AfxGetApp()->m_pMainWnd;
	//原點復歸參數
	((CCommandTestDlg*)pMain)->a.Default.GoHome.Speed1 = _ttol(m_ListDefault.GetItemText(0, 1));
	((CCommandTestDlg*)pMain)->a.Default.GoHome.Speed2 = _ttol(m_ListDefault.GetItemText(1, 1));
	((CCommandTestDlg*)pMain)->a.Default.GoHome.Axis = _ttol(m_ListDefault.GetItemText(2, 1));
	((CCommandTestDlg*)pMain)->a.Default.GoHome.MoveX = _ttol(m_ListDefault.GetItemText(3, 1));
	((CCommandTestDlg*)pMain)->a.Default.GoHome.MoveY= _ttol(m_ListDefault.GetItemText(4, 1));
	((CCommandTestDlg*)pMain)->a.Default.GoHome.MoveZ= _ttol(m_ListDefault.GetItemText(5, 1));
	((CCommandTestDlg*)pMain)->a.Default.GoHome.PrecycleInitialize = _ttol(m_ListDefault.GetItemText(6, 1));
	//運動參數
	((CCommandTestDlg*)pMain)->a.Default.DispenseDotSet.GlueOpenTime = _ttol(m_ListDefault.GetItemText(7, 1));
	((CCommandTestDlg*)pMain)->a.Default.DispenseDotSet.GlueCloseTime = _ttol(m_ListDefault.GetItemText(8, 1));
	((CCommandTestDlg*)pMain)->a.Default.DispenseDotEnd.RiseDistance = _ttol(m_ListDefault.GetItemText(9, 1));
	((CCommandTestDlg*)pMain)->a.Default.DispenseDotEnd.RiseLowSpeed = _ttol(m_ListDefault.GetItemText(10, 1));
	((CCommandTestDlg*)pMain)->a.Default.DispenseDotEnd.RiseHightSpeed = _ttol(m_ListDefault.GetItemText(11, 1));
	((CCommandTestDlg*)pMain)->a.Default.DotSpeedSet.AccSpeed = _ttol(m_ListDefault.GetItemText(12, 1));
	((CCommandTestDlg*)pMain)->a.Default.DotSpeedSet.EndSpeed = _ttol(m_ListDefault.GetItemText(13, 1));
	((CCommandTestDlg*)pMain)->a.Default.DispenseLineSet.BeforeMoveDelay = _ttol(m_ListDefault.GetItemText(14, 1));
	((CCommandTestDlg*)pMain)->a.Default.DispenseLineSet.BeforeMoveDistance = _ttol(m_ListDefault.GetItemText(15, 1));
	((CCommandTestDlg*)pMain)->a.Default.DispenseLineSet.NodeTime = _ttol(m_ListDefault.GetItemText(16, 1));
	((CCommandTestDlg*)pMain)->a.Default.DispenseLineSet.StayTime = _ttol(m_ListDefault.GetItemText(17, 1));
	((CCommandTestDlg*)pMain)->a.Default.DispenseLineSet.ShutdownDistance = _ttol(m_ListDefault.GetItemText(18, 1));
	((CCommandTestDlg*)pMain)->a.Default.DispenseLineSet.ShutdownDelay = _ttol(m_ListDefault.GetItemText(19, 1));
	((CCommandTestDlg*)pMain)->a.Default.DispenseLineEnd.Type = _ttol(m_ListDefault.GetItemText(20, 1));
	((CCommandTestDlg*)pMain)->a.Default.DispenseLineEnd.LowSpeed = _ttol(m_ListDefault.GetItemText(21, 1));
	((CCommandTestDlg*)pMain)->a.Default.DispenseLineEnd.Height = _ttol(m_ListDefault.GetItemText(22, 1));
	((CCommandTestDlg*)pMain)->a.Default.DispenseLineEnd.Width = _ttol(m_ListDefault.GetItemText(23, 1));
	((CCommandTestDlg*)pMain)->a.Default.DispenseLineEnd.HighSpeed = _ttol(m_ListDefault.GetItemText(24, 1));
	((CCommandTestDlg*)pMain)->a.Default.LineSpeedSet.AccSpeed= _ttol(m_ListDefault.GetItemText(25, 1));
	((CCommandTestDlg*)pMain)->a.Default.LineSpeedSet.EndSpeed = _ttol(m_ListDefault.GetItemText(26, 1));
	((CCommandTestDlg*)pMain)->a.Default.ZSet.ZBackHeight = _ttol(m_ListDefault.GetItemText(27, 1));
	((CCommandTestDlg*)pMain)->a.Default.ZSet.ZBackType= _ttol(m_ListDefault.GetItemText(28, 1));
	((CCommandTestDlg*)pMain)->a.Default.GlueData.ParkPositionData.X = _ttol(m_ListDefault.GetItemText(29, 1));
	((CCommandTestDlg*)pMain)->a.Default.GlueData.ParkPositionData.Y = _ttol(m_ListDefault.GetItemText(30, 1));
	((CCommandTestDlg*)pMain)->a.Default.GlueData.ParkPositionData.Z = _ttol(m_ListDefault.GetItemText(31, 1));
	((CCommandTestDlg*)pMain)->a.Default.GlueData.GlueAuto = _ttol(m_ListDefault.GetItemText(32, 1));
	((CCommandTestDlg*)pMain)->a.Default.GlueData.GlueWaitTime = _ttol(m_ListDefault.GetItemText(33, 1));
	((CCommandTestDlg*)pMain)->a.Default.GlueData.GlueTime = _ttol(m_ListDefault.GetItemText(34, 1));
	((CCommandTestDlg*)pMain)->a.Default.GlueData.GlueStayTime = _ttol(m_ListDefault.GetItemText(35, 1)); 
	//影像參數
	((CCommandTestDlg*)pMain)->a.VisionDefault.VisionSet.Accuracy = _ttol(m_ListDefault.GetItemText(36, 1));
	((CCommandTestDlg*)pMain)->a.VisionDefault.VisionSet.Speed = _ttol(m_ListDefault.GetItemText(37, 1));
	((CCommandTestDlg*)pMain)->a.VisionDefault.VisionSet.Score = (BOOL)(_ttol(m_ListDefault.GetItemText(38, 1)));
	((CCommandTestDlg*)pMain)->a.VisionDefault.VisionSet.width = _ttol(m_ListDefault.GetItemText(39, 1));
	((CCommandTestDlg*)pMain)->a.VisionDefault.VisionSet.height = _ttol(m_ListDefault.GetItemText(40, 1));
	((CCommandTestDlg*)pMain)->a.VisionDefault.VisionSet.Startangle = _ttol(m_ListDefault.GetItemText(41, 1));
	((CCommandTestDlg*)pMain)->a.VisionDefault.VisionSet.Endangle = _ttol(m_ListDefault.GetItemText(42, 1));
	((CCommandTestDlg*)pMain)->a.VisionDefault.VisionSerchError.SearchError = _ttol(m_ListDefault.GetItemText(43, 1));
	//尋問對話框
	if (pQuestion == NULL)
	{
		pQuestion = new CQuestion;
	}
	((CCommandTestDlg*)pMain)->a.VisionDefault.VisionSerchError.pQuestion = pQuestion;
	//最大運行次數
	((CCommandTestDlg*)pMain)->a.RunLoopData.MaxRunNumber = _ttol(m_ListDefault.GetItemText(44, 1));
	((CCommandTestDlg*)pMain)->RunLoopNumber = _ttol(m_ListDefault.GetItemText(45, 1));
}
#include "ExcelToList.h"
#include "ListToExcel.h"
void CDefault::OnBnClickedButton2()
{
	for (int i = 0; i < 46; i++) {
		m_ListDefault.SetItemText(i, 1, _T("0"));
	}
	ParamModify();
	//ExportListToExcel(&m_ListDefault, L"LineContinuous");
    //ExportExcelToList(&m_ListDefault, L"LineContinuous");
}
void CDefault::OnNMDblclkList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	NM_LISTVIEW  *pEditCtrl = (NM_LISTVIEW *)pNMHDR;
	if (pEditCtrl->iItem == -1)  //條件一 : 不在表單上
	{
		if (m_Edit.GetSafeHwnd())
		{
			DistroyEdit(&m_ListDefault, &m_Edit, m_OldListRow, m_OldListColumn);
		}
	}
	else //條件一 :  在表單上
	{
		if (pEditCtrl->iSubItem != 1)
		{
			//如果Edit已創建，創建
			if (m_Edit.GetSafeHwnd())
			{
				DistroyEdit(&m_ListDefault, &m_Edit, m_OldListRow, m_OldListColumn);
			} 
			m_OldListRow = pEditCtrl->iItem;
			m_OldListColumn = pEditCtrl->iSubItem;
		}
		else //代表選到"工作方式"
		{
			if (m_Edit.GetSafeHwnd()) //條件二:是否創建combobox
			{
				if (!(m_OldListRow == pEditCtrl->iItem && m_OldListColumn == pEditCtrl->iSubItem)) //條件三:是否按的為同一格
				{
					DistroyEdit(&m_ListDefault, &m_Edit, m_OldListRow, m_OldListColumn);
					CreateEdit(pEditCtrl, &m_Edit);
				}
				else//点中的单元格是之前创建好的  
				{
					m_Edit.SetFocus();//设置为焦点   
				}
			}
			else //還沒創建combobox
			{
				CreateEdit(pEditCtrl, &m_Edit);
			}
		}
	}
	*pResult = 0;
}
/*確定修改*/
void CDefault::OnNMClickList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	NM_LISTVIEW  *pEditCtrl = (NM_LISTVIEW *)pNMHDR;
	if (m_Edit.GetSafeHwnd())
	{
		if (!(m_OldListRow == pEditCtrl->iItem && m_OldListColumn == pEditCtrl->iSubItem)) //條件三:是否按的為同一格
		{
			DistroyEdit(&m_ListDefault, &m_Edit, m_OldListRow, m_OldListColumn);
		}
	}
	*pResult = 0;
}
/*取消修改*/
void CDefault::OnNMRClickList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	NM_LISTVIEW  *pEditCtrl = (NM_LISTVIEW *)pNMHDR;
	if (m_Edit.GetSafeHwnd())
	{
		if (!(m_OldListRow == pEditCtrl->iItem && m_OldListColumn == pEditCtrl->iSubItem)) //條件三:是否按的為同一格
		{
			m_Edit.DestroyWindow();
		}
	}
	*pResult = 0;
}
//CreateEdit
void CDefault::CreateEdit(NM_LISTVIEW * pEditCtrl, CEdit * m_Editbox1)
{
	CRect EditRect;
	//點處理
	m_OldListRow = pEditCtrl->iItem;
	m_OldListColumn = pEditCtrl->iSubItem;
	//創建編輯框
	m_Editbox1->Create(ES_AUTOHSCROLL | WS_CHILD | ES_LEFT | ES_WANTRETURN, CRect(0, 0, 0, 0), this, IDC_EDITBUFF);
	m_Editbox1->SetParent(&m_ListDefault); //List 設置父窗口*important
	m_ListDefault.GetSubItemRect(m_OldListRow, m_OldListColumn, LVIR_LABEL, EditRect); //獲取空間訊息
	EditRect.SetRect(EditRect.left + 1, EditRect.top + 1, EditRect.left + m_ListDefault.GetColumnWidth(m_OldListColumn) - 1, EditRect.bottom - 1); //+1和-1使編輯框不檔住網格線
	m_Editbox1->MoveWindow(&EditRect); //將編輯框放在滑鼠點擊位置上
	m_Editbox1->ShowWindow(SW_SHOW); //顯示編輯框
	//編輯框設置
	CString str;
	str = m_ListDefault.GetItemText(m_OldListRow, m_OldListColumn); //抓取格子內狀態
	m_Editbox1->SetWindowText(NULL); //將文字顯示在格子上
	m_Editbox1->SetFont(this->GetFont(), FALSE);  //設置字體，不然會有突兀的感覺
	m_Editbox1->SetFocus(); //設其為焦點
	m_Editbox1->SetSel(-1);
}
//DistroyEditBox
void CDefault::DistroyEdit(CListCtrl *list, CEdit *distroyedit, int &Item, int &SubItem)
{
	CString  editdata;
	distroyedit->GetWindowTextW(editdata);
	if (editdata == _T(""))
		editdata = _T("0");
	list->SetItemText(Item, SubItem, editdata); //type in 對應Item
	distroyedit->DestroyWindow(); //銷毀
	ParamModify();
}
//顯示視窗時設定
void CDefault::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);
	SetWindowLong(this->m_hWnd, GWL_EXSTYLE, GetWindowLong(this->m_hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);//設置視窗為可以透明化
	this->SetLayeredWindowAttributes(0, (255 * 100) / 100, LWA_ALPHA);//不透明
}
//非活動轉活動事件
int CDefault::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
{
	//從非活動轉為活動改成不透明
	this->SetLayeredWindowAttributes(0, (255 * 100) / 100, LWA_ALPHA);
	return CDialogEx::OnMouseActivate(pDesktopWnd, nHitTest, message);
}
