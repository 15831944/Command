// LineContinuous.cpp : 實作檔
//

#include "stdafx.h"
#include "CommandTest.h"
#include "LineContinuous.h"
#include "afxdialogex.h"
#include "CommandTestDlg.h"
#include "ListToExcel.h"
#include "ExcelToList.h"

#define IDC_EDITBUFF3 13000
// CLineContinuous 對話方塊

IMPLEMENT_DYNAMIC(CLineContinuous, CDialogEx)

CLineContinuous::CLineContinuous(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG8, pParent)
{
	pMain = AfxGetApp()->m_pMainWnd;
}

CLineContinuous::~CLineContinuous()
{
}

void CLineContinuous::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_ListLCPoint);
}

BEGIN_MESSAGE_MAP(CLineContinuous, CDialogEx)
	ON_WM_MOUSEACTIVATE()
	ON_WM_SHOWWINDOW()
	ON_NOTIFY(NM_CLICK, IDC_LIST1, &CLineContinuous::OnNMClickList1)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST1, &CLineContinuous::OnNMDblclkList1)
	ON_NOTIFY(NM_RCLICK, IDC_LIST1, &CLineContinuous::OnNMRClickList1)
	ON_COMMAND(IDM_INSERT, &CLineContinuous::OnInsert)
	ON_COMMAND(IDM_DELETE, &CLineContinuous::OnDelete)
	ON_COMMAND(IDM_MOVE, &CLineContinuous::OnMove)
	ON_BN_CLICKED(IDC_BTNLISTTOEXCEL, &CLineContinuous::OnBnClickedBtnlisttoexcel)
	ON_BN_CLICKED(IDC_BTNEXCELTOLIST, &CLineContinuous::OnBnClickedBtnexceltolist)
END_MESSAGE_MAP()

// CLineContinuous 訊息處理常式
BOOL CLineContinuous::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	m_ListLCPoint.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
	m_ListLCPoint.InsertColumn(0, _T("編號"), LVCFMT_CENTER, 36, -1);
	m_ListLCPoint.InsertColumn(1, _T("X"), LVCFMT_LEFT, 100, -1);
	m_ListLCPoint.InsertColumn(2, _T("Y"), LVCFMT_LEFT, 100, -1);
	m_ListLCPoint.InsertColumn(3, _T("Z"), LVCFMT_LEFT, 100, -1);
	ListRefresh();
	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX 屬性頁應傳回 FALSE
}
//取消
void CLineContinuous::OnCancel()
{
	CDialogEx::OnCancel();
}
//CreateEdit
void CLineContinuous::CreateEdit(NM_LISTVIEW * pEditCtrl, CEdit * m_Editbox1)
{
	CRect EditRect;
	//點處理
	m_OldListRow = pEditCtrl->iItem;
	m_OldListColumn = pEditCtrl->iSubItem;
	//創建編輯框
	m_Editbox1->Create(ES_AUTOHSCROLL | WS_CHILD | ES_LEFT | ES_WANTRETURN, CRect(0, 0, 0, 0), this, IDC_EDITBUFF3);
	m_Editbox1->SetParent(&m_ListLCPoint); //List 設置父窗口*important
	m_ListLCPoint.GetSubItemRect(m_OldListRow, m_OldListColumn, LVIR_LABEL, EditRect); //獲取空間訊息
	EditRect.SetRect(EditRect.left + 1, EditRect.top + 1, EditRect.left + m_ListLCPoint.GetColumnWidth(m_OldListColumn) - 1, EditRect.bottom - 1); //+1和-1使編輯框不檔住網格線
	m_Editbox1->MoveWindow(&EditRect); //將編輯框放在滑鼠點擊位置上
	m_Editbox1->ShowWindow(SW_SHOW); //顯示編輯框
									 //編輯框設置
	CString str;
	str = m_ListLCPoint.GetItemText(m_OldListRow, m_OldListColumn); //抓取格子內狀態
	m_Editbox1->SetWindowText(NULL); //將文字顯示在格子上
	m_Editbox1->SetFont(this->GetFont(), FALSE);  //設置字體，不然會有突兀的感覺
	m_Editbox1->SetFocus(); //設其為焦點
	m_Editbox1->SetSel(-1);
}
//DistroyEditBox
void CLineContinuous::DistroyEdit(CListCtrl *list, CEdit *distroyedit, int &Item, int &SubItem)
{
	CString  editdata;
	distroyedit->GetWindowTextW(editdata);
	if (editdata == _T(""))
		editdata = _T("0");
	list->SetItemText(Item, SubItem, editdata); //type in 對應Item
	distroyedit->DestroyWindow(); //銷毀
	LineCoutinuousModify();
}
//左鍵一下
void CLineContinuous::OnNMClickList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	NM_LISTVIEW  *pEditCtrl = (NM_LISTVIEW *)pNMHDR;
	if (m_Edit.GetSafeHwnd())
	{
		if (!(m_OldListRow == pEditCtrl->iItem && m_OldListColumn == pEditCtrl->iSubItem)) //條件三:是否按的為同一格
		{
			DistroyEdit(&m_ListLCPoint, &m_Edit, m_OldListRow, m_OldListColumn);
		}
	}
	*pResult = 0;
}
//左鍵兩下
void CLineContinuous::OnNMDblclkList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	NM_LISTVIEW  *pEditCtrl = (NM_LISTVIEW *)pNMHDR;
	if (pEditCtrl->iItem == -1)  //條件一 : 不在表單上
	{
		if (m_Edit.GetSafeHwnd())
		{
			DistroyEdit(&m_ListLCPoint, &m_Edit, m_OldListRow, m_OldListColumn);
		}
	}
	else //條件一 :  在表單上
	{
		if (pEditCtrl->iSubItem != 1 && pEditCtrl->iSubItem != 2 && pEditCtrl->iSubItem != 3)
		{
			//如果Edit已創建，創建
			if (m_Edit.GetSafeHwnd())
			{
				DistroyEdit(&m_ListLCPoint, &m_Edit, m_OldListRow, m_OldListColumn);
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
					DistroyEdit(&m_ListLCPoint, &m_Edit, m_OldListRow, m_OldListColumn);
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
//右鍵一下
void CLineContinuous::OnNMRClickList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	CListCtrl *CList = (CListCtrl *)GetDlgItem(IDC_MODIFYLIST);
	CMenu menu, *pSubMenu;
	CPoint CurPnt;
	int ItemCount = m_ListLCPoint.GetItemCount();//獲取項目總數
	NM_LISTVIEW  *pEditCtrl = (NM_LISTVIEW *)pNMHDR;

	if (pEditCtrl->iItem != -1 || pEditCtrl->iSubItem != 0) {
		menu.LoadMenu(IDR_MENU1);//加入菜單
		pSubMenu = menu.GetSubMenu(0);
		GetCursorPos(&CurPnt);
		pSubMenu->TrackPopupMenu(TPM_LEFTALIGN, CurPnt.x, CurPnt.y, this);//點右鍵出現的菜單位置
	}
	*pResult = 0;
}
//取消確定關閉視窗
void CLineContinuous::OnOK()
{
	//CDialogEx::OnOK();
}
//修改點值
void CLineContinuous::LineCoutinuousModify()
{
#ifdef MOVE
	for (UINT i = 0; i < ((CCommandTestDlg*)pMain)->a.m_Action.LA_m_ptVec.size(); i++)
	{
		((CCommandTestDlg*)pMain)->a.m_Action.LA_m_ptVec.at(i).EndPX = _ttol(m_ListLCPoint.GetItemText(i, 1));
		((CCommandTestDlg*)pMain)->a.m_Action.LA_m_ptVec.at(i).EndPY = _ttol(m_ListLCPoint.GetItemText(i, 2));
		((CCommandTestDlg*)pMain)->a.m_Action.LA_m_ptVec.at(i).EndPZ = _ttol(m_ListLCPoint.GetItemText(i, 3));
	}
#endif
}
//刷新列表
void CLineContinuous::ListRefresh()
{
#ifdef MOVE
	CString StrBuff;
	m_ListLCPoint.DeleteAllItems();
	int nCount = ((CCommandTestDlg*)pMain)->a.m_Action.LA_m_ptVec.size();
	for (int i = 0; i < nCount; i++)
	{
		m_ListLCPoint.InsertItem(i, NULL);
		(i > 8) ? StrBuff.Format(_T("0%d"), i + 1) : StrBuff.Format(_T("00%d"), i + 1);
		m_ListLCPoint.SetItemText(i, 0, StrBuff);
		StrBuff.Format(L"%d", ((CCommandTestDlg*)pMain)->a.m_Action.LA_m_ptVec.at(i).EndPX);
		m_ListLCPoint.SetItemText(i, 1, StrBuff);
		StrBuff.Format(L"%d", ((CCommandTestDlg*)pMain)->a.m_Action.LA_m_ptVec.at(i).EndPY);
		m_ListLCPoint.SetItemText(i, 2, StrBuff);
		StrBuff.Format(L"%d", ((CCommandTestDlg*)pMain)->a.m_Action.LA_m_ptVec.at(i).EndPZ);
		m_ListLCPoint.SetItemText(i, 3, StrBuff);

	}
#endif
	int ListnCount = m_ListLCPoint.GetItemCount();
	m_ListLCPoint.EnsureVisible(ListnCount - 1, FALSE);//使List中一項可見(如滾動條向下滾)
}
//插入
void CLineContinuous::OnInsert()
{
	int istat = m_ListLCPoint.GetSelectionMark();//獲取選擇的項
#ifdef MOVE
	m_ListLCPoint.InsertItem(istat, NULL);
	((CCommandTestDlg*)pMain)->a.m_Action.LA_m_ptVec.insert(((CCommandTestDlg*)pMain)->a.m_Action.LA_m_ptVec.begin() + istat, { 0,0,0,0,0 });
	ListRefresh();
#endif
}
//刪除
void CLineContinuous::OnDelete()
{
	int istat = m_ListLCPoint.GetSelectionMark();//獲取選擇的項
#ifdef MOVE
	((CCommandTestDlg*)pMain)->a.m_Action.LA_m_ptVec.erase(((CCommandTestDlg*)pMain)->a.m_Action.LA_m_ptVec.begin() + istat);
	ListRefresh();
#endif
}
//移動此命令/*****尚未修正*****/
void CLineContinuous::OnMove()
{
	int istat = m_ListLCPoint.GetSelectionMark();//獲取選擇的項
#ifdef MOVE
	MO_Do3DLineMove(0, 0, 51544 - MO_ReadLogicPosition(2), 80000, 1200000, 6000);
	while (MO_ReadIsDriving(7))
	{
		Sleep(1);
	}
	MO_Do3DLineMove(((CCommandTestDlg*)pMain)->a.m_Action.LA_m_ptVec.at(istat).EndPX - MO_ReadLogicPosition(0),
		((CCommandTestDlg*)pMain)->a.m_Action.LA_m_ptVec.at(istat).EndPY - MO_ReadLogicPosition(1),0,
		80000, 1200000, 6000);
	while (MO_ReadIsDriving(7))
	{
		Sleep(1);
	}
	MO_Do3DLineMove(0,0,((CCommandTestDlg*)pMain)->a.m_Action.LA_m_ptVec.at(istat).EndPZ - MO_ReadLogicPosition(2),
		80000, 1200000, 6000);
#endif   
}
//導出檔案至Excel
void CLineContinuous::OnBnClickedBtnlisttoexcel()
{
	ExportListToExcel(&m_ListLCPoint, L"LineContinuous");
}
//導入檔案至List
void CLineContinuous::OnBnClickedBtnexceltolist()
{
	ExportExcelToList(&m_ListLCPoint, L"LineContinuous");
#ifdef MOVE
    ((CCommandTestDlg*)pMain)->a.m_Action.LA_Clear();
    for (int i = 0; i < m_ListLCPoint.GetItemCount(); i++)
    {
        ((CCommandTestDlg*)pMain)->a.m_Action.LA_m_ptVec.push_back({ 0,0,0,0,0 });
        ((CCommandTestDlg*)pMain)->a.m_Action.LA_m_ptVec.back().EndPX = _ttol(m_ListLCPoint.GetItemText(i, 1));
        ((CCommandTestDlg*)pMain)->a.m_Action.LA_m_ptVec.back().EndPY = _ttol(m_ListLCPoint.GetItemText(i, 2));
        ((CCommandTestDlg*)pMain)->a.m_Action.LA_m_ptVec.back().EndPZ = _ttol(m_ListLCPoint.GetItemText(i, 3));
    }
    ((CCommandTestDlg*)pMain)->a.m_Action.LA_m_iVecSP.push_back(1);
    for (UINT i = 0; i < ((CCommandTestDlg*)pMain)->a.m_Action.LA_m_ptVec.size(); i++)
    {
        if (((CCommandTestDlg*)pMain)->a.m_Action.LA_m_ptVec.at(i).EndPX == -99999 && ((CCommandTestDlg*)pMain)->a.m_Action.LA_m_ptVec.at(i).EndPY == -99999 && ((CCommandTestDlg*)pMain)->a.m_Action.LA_m_ptVec.at(i).EndPZ == -99999)
        {
            ((CCommandTestDlg*)pMain)->a.m_Action.LA_m_iVecSP.push_back(i + 1);
        }
    }
#endif
}
//顯示視窗時設定
void CLineContinuous::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);
	SetWindowLong(this->m_hWnd, GWL_EXSTYLE, GetWindowLong(this->m_hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);//設置視窗為可以透明化
	this->SetLayeredWindowAttributes(0, (255 * 100) / 100, LWA_ALPHA);//不透明
}
//非活動轉活動事件
int CLineContinuous::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
{
	//從非活動轉為活動改成不透明
	this->SetLayeredWindowAttributes(0, (255 * 100) / 100, LWA_ALPHA);
	return CDialogEx::OnMouseActivate(pDesktopWnd, nHitTest, message);
}







