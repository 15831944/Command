// LaserAdjust.cpp : 實作檔
//

#include "stdafx.h"
#include "CommandTest.h"
#include "LaserAdjust.h"
#include "afxdialogex.h"
#include "CommandTestDlg.h"

#define  IDC_EDITBUFF2 12000
// CLaserAdjust 對話方塊

IMPLEMENT_DYNAMIC(CLaserAdjust, CDialogEx)

CLaserAdjust::CLaserAdjust(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG9, pParent)
{
	pMain = AfxGetApp()->m_pMainWnd;
}

CLaserAdjust::~CLaserAdjust()
{
}

void CLaserAdjust::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_ListLaserAdjust);
}

BEGIN_MESSAGE_MAP(CLaserAdjust, CDialogEx)
	ON_WM_MOUSEACTIVATE()
	ON_WM_SHOWWINDOW()
	ON_NOTIFY(NM_RCLICK, IDC_LIST1, &CLaserAdjust::OnNMRClickList1)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST1, &CLaserAdjust::OnNMDblclkList1)
	ON_NOTIFY(NM_CLICK, IDC_LIST1, &CLaserAdjust::OnNMClickList1)
	ON_COMMAND(IDM_LASERINSTER, &CLaserAdjust::OnLaserinster)
	ON_COMMAND(IDM_LASERDELETE, &CLaserAdjust::OnLaserdelete)
END_MESSAGE_MAP()

// CLaserAdjust 訊息處理常式

BOOL CLaserAdjust::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	m_ListLaserAdjust.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
	m_ListLaserAdjust.InsertColumn(0, _T("順序"), LVCFMT_CENTER, 36, -1);
	m_ListLaserAdjust.InsertColumn(1, _T("雷射值"), LVCFMT_LEFT, 100, -1);
	ListRefresh();
	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX 屬性頁應傳回 FALSE
}
//取消
void CLaserAdjust::OnCancel()
{
	CDialogEx::OnCancel();
}
//新增Enter
void CLaserAdjust::OnOK()
{
    ((CCommandTestDlg*)pMain)->a.LaserAdjust.push_back({ 0 });
    ListRefresh();
    // CDialogEx::OnOK();
}
//列表左鍵一下
void CLaserAdjust::OnNMClickList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	NM_LISTVIEW  *pEditCtrl = (NM_LISTVIEW *)pNMHDR;
	if (m_Edit.GetSafeHwnd())
	{
		if (!(m_OldListRow == pEditCtrl->iItem && m_OldListColumn == pEditCtrl->iSubItem)) //條件三:是否按的為同一格
		{
			DistroyEdit(&m_ListLaserAdjust, &m_Edit, m_OldListRow, m_OldListColumn);
		}
	}
	*pResult = 0;
}
//列表左鍵兩下
void CLaserAdjust::OnNMDblclkList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	NM_LISTVIEW  *pEditCtrl = (NM_LISTVIEW *)pNMHDR;
	if (pEditCtrl->iItem == -1)  //條件一 : 不在表單上
	{
		if (m_Edit.GetSafeHwnd())
		{
			DistroyEdit(&m_ListLaserAdjust, &m_Edit, m_OldListRow, m_OldListColumn);
		}
	}
	else //條件一 :  在表單上
	{
		if (pEditCtrl->iSubItem != 1)
		{
			//如果Edit已創建，創建
			if (m_Edit.GetSafeHwnd())
			{
				DistroyEdit(&m_ListLaserAdjust, &m_Edit, m_OldListRow, m_OldListColumn);
			}
			m_OldListRow = pEditCtrl->iItem;
			m_OldListColumn = pEditCtrl->iSubItem;
		}
		else //代表選到"工作方式"
		{
			if (m_Edit.GetSafeHwnd()) //條件二:是否創建CEdit
			{
				if (!(m_OldListRow == pEditCtrl->iItem && m_OldListColumn == pEditCtrl->iSubItem)) //條件三:是否按的為同一格
				{
					DistroyEdit(&m_ListLaserAdjust, &m_Edit, m_OldListRow, m_OldListColumn);
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
//CreateEdit
void CLaserAdjust::CreateEdit(NM_LISTVIEW * pEditCtrl, CEdit * m_Editbox1)
{
    CRect EditRect;
    //點處理
    m_OldListRow = pEditCtrl->iItem;
    m_OldListColumn = pEditCtrl->iSubItem;
    //創建編輯框
    m_Editbox1->Create(ES_AUTOHSCROLL | WS_CHILD | ES_LEFT | ES_WANTRETURN, CRect(0, 0, 0, 0), this, IDC_EDITBUFF2);
    m_Editbox1->SetParent(&m_ListLaserAdjust); //List 設置父窗口*important
    m_ListLaserAdjust.GetSubItemRect(m_OldListRow, m_OldListColumn, LVIR_LABEL, EditRect); //獲取空間訊息
    EditRect.SetRect(EditRect.left + 1, EditRect.top + 1, EditRect.left + m_ListLaserAdjust.GetColumnWidth(m_OldListColumn) - 1, EditRect.bottom - 1); //+1和-1使編輯框不檔住網格線
    m_Editbox1->MoveWindow(&EditRect); //將編輯框放在滑鼠點擊位置上
    m_Editbox1->ShowWindow(SW_SHOW); //顯示編輯框
                                     //編輯框設置
    CString str;
    str = m_ListLaserAdjust.GetItemText(m_OldListRow, m_OldListColumn); //抓取格子內狀態
    m_Editbox1->SetWindowText(NULL); //將文字顯示在格子上
    m_Editbox1->SetFont(this->GetFont(), FALSE);  //設置字體，不然會有突兀的感覺
    m_Editbox1->SetFocus(); //設其為焦點
    m_Editbox1->SetSel(-1);
}
//DistroyEditBox
void CLaserAdjust::DistroyEdit(CListCtrl *list, CEdit *distroyedit, int &Item, int &SubItem)
{
    CString  editdata;
    distroyedit->GetWindowTextW(editdata);
    if (editdata == _T(""))
        editdata = _T("0");
    list->SetItemText(Item, SubItem, editdata); //type in 對應Item
    distroyedit->DestroyWindow(); //銷毀
    LaserAdjustModify();
}
//列表右鍵一下
void CLaserAdjust::OnNMRClickList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	CListCtrl *CList = (CListCtrl *)GetDlgItem(IDC_MODIFYLIST);
	CMenu menu, *pSubMenu;
	CPoint CurPnt;
	int ItemCount = m_ListLaserAdjust.GetItemCount();//獲取項目總數
	NM_LISTVIEW  *pEditCtrl = (NM_LISTVIEW *)pNMHDR;

	if (pEditCtrl->iItem != -1 || pEditCtrl->iSubItem != 0) {
		menu.LoadMenu(IDR_MENU4);//加入菜單
		pSubMenu = menu.GetSubMenu(0);
		GetCursorPos(&CurPnt);
		pSubMenu->TrackPopupMenu(TPM_LEFTALIGN, CurPnt.x, CurPnt.y, this);//點右鍵出現的菜單位置
	}
	*pResult = 0;
}
//插入
void CLaserAdjust::OnLaserinster()
{
    int istat = m_ListLaserAdjust.GetSelectionMark();//獲取選擇的項
    m_ListLaserAdjust.InsertItem(istat, NULL);
    ((CCommandTestDlg*)pMain)->a.LaserAdjust.insert(((CCommandTestDlg*)pMain)->a.LaserAdjust.begin() + istat, { 0 });
    ListRefresh();
}
//刪除
void CLaserAdjust::OnLaserdelete()
{
    int istat = m_ListLaserAdjust.GetSelectionMark();//獲取選擇的項
    ((CCommandTestDlg*)pMain)->a.LaserAdjust.erase(((CCommandTestDlg*)pMain)->a.LaserAdjust.begin() + istat);
    ListRefresh();
}
//修改陣列值
void CLaserAdjust::LaserAdjustModify()
{
	for (UINT i = 0; i < ((CCommandTestDlg*)pMain)->a.LaserAdjust.size(); i++)
	{
		((CCommandTestDlg*)pMain)->a.LaserAdjust.at(i).LaserMeasureHeight = _ttol(m_ListLaserAdjust.GetItemText(i, 1));
	}
}
//刷新列表
void CLaserAdjust::ListRefresh()
{
	CString StrBuff;
	m_ListLaserAdjust.DeleteAllItems();
	int nCount = ((CCommandTestDlg*)pMain)->a.LaserAdjust.size();
	for (int i = 0; i < nCount; i++)
	{
		m_ListLaserAdjust.InsertItem(i, NULL);
		(i > 8) ? StrBuff.Format(_T("0%d"), i + 1) : StrBuff.Format(_T("00%d"), i + 1);
		m_ListLaserAdjust.SetItemText(i, 0, StrBuff);
		StrBuff.Format(L"%d", ((CCommandTestDlg*)pMain)->a.LaserAdjust.at(i).LaserMeasureHeight);
		m_ListLaserAdjust.SetItemText(i, 1, StrBuff);
	}
	int ListnCount = m_ListLaserAdjust.GetItemCount();
	m_ListLaserAdjust.EnsureVisible(ListnCount - 1, FALSE);//使List中一項可見(如滾動條向下滾)
}
//顯示視窗時設定
void CLaserAdjust::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);
	SetWindowLong(this->m_hWnd, GWL_EXSTYLE, GetWindowLong(this->m_hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);//設置視窗為可以透明化
	this->SetLayeredWindowAttributes(0, (255 * 100) / 100, LWA_ALPHA);//不透明
}
//非活動轉活動事件
int CLaserAdjust::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
{
	//從非活動轉為活動改成不透明
	this->SetLayeredWindowAttributes(0, (255 * 100) / 100, LWA_ALPHA);
	return CDialogEx::OnMouseActivate(pDesktopWnd, nHitTest, message);
}
