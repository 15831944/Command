// PositionModify.cpp : 實作檔
//

#include "stdafx.h"
#include "CommandTest.h"
#include "PositionModify.h"
#include "afxdialogex.h"
#include "CommandTestDlg.h"

#define IDC_EDITBUFF1  11000
// CPositionModify 對話方塊

IMPLEMENT_DYNAMIC(CPositionModify, CDialogEx)

CPositionModify::CPositionModify(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG7, pParent)
{
    pMain = AfxGetApp()->m_pMainWnd;
}

CPositionModify::~CPositionModify()
{
}

void CPositionModify::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MODIFYLIST, m_Listposition);
}


BEGIN_MESSAGE_MAP(CPositionModify, CDialogEx)
	ON_NOTIFY(NM_RCLICK, IDC_MODIFYLIST, &CPositionModify::OnNMRClickModifylist)
	ON_NOTIFY(NM_DBLCLK, IDC_MODIFYLIST, &CPositionModify::OnNMDblclkModifylist)
	ON_NOTIFY(NM_CLICK, IDC_MODIFYLIST, &CPositionModify::OnNMClickModifylist)
	ON_COMMAND(IDM_INSERT3, &CPositionModify::OnInsert3)
	ON_COMMAND(IDM_DELETE3, &CPositionModify::OnDelete3)
    ON_BN_CLICKED(IDC_BTNNEW, &CPositionModify::OnBnClickedBtnnew)
    ON_BN_CLICKED(IDC_BTNCLEAR, &CPositionModify::OnBnClickedBtnclear)
    ON_WM_SHOWWINDOW()
    ON_WM_MOUSEACTIVATE()
END_MESSAGE_MAP()


// CPositionModify 訊息處理常式


BOOL CPositionModify::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	m_Listposition.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
	m_Listposition.InsertColumn(0, _T("順序"), LVCFMT_CENTER, 36, -1);
	m_Listposition.InsertColumn(1, _T("地址"), LVCFMT_LEFT, 100, -1);
	m_Listposition.InsertColumn(2, _T("影像編號"), LVCFMT_LEFT, 66, -1);
	m_Listposition.InsertColumn(3, _T("雷射編號"), LVCFMT_LEFT, 66, -1);
	ListRefresh();
	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX 屬性頁應傳回 FALSE
}
//取消Enter 關閉視窗
void CPositionModify::OnOK()
{
	//CDialogEx::OnOK();
}
//取消
void CPositionModify::OnCancel()
{
	CDialogEx::OnCancel();
}
//CreateEdit
void CPositionModify::CreateEdit(NM_LISTVIEW * pEditCtrl, CEdit * m_Editbox1)
{
	CRect EditRect;
	//點處理
	m_OldListRow = pEditCtrl->iItem;
	m_OldListColumn = pEditCtrl->iSubItem;
	//創建編輯框
	m_Editbox1->Create(ES_AUTOHSCROLL | WS_CHILD | ES_LEFT | ES_WANTRETURN, CRect(0, 0, 0, 0), this, IDC_EDITBUFF1);
	m_Editbox1->SetParent(&m_Listposition); //List 設置父窗口*important
	m_Listposition.GetSubItemRect(m_OldListRow, m_OldListColumn, LVIR_LABEL, EditRect); //獲取空間訊息
	EditRect.SetRect(EditRect.left + 1, EditRect.top + 1, EditRect.left + m_Listposition.GetColumnWidth(m_OldListColumn) - 1, EditRect.bottom - 1); //+1和-1使編輯框不檔住網格線
	m_Editbox1->MoveWindow(&EditRect); //將編輯框放在滑鼠點擊位置上
	m_Editbox1->ShowWindow(SW_SHOW); //顯示編輯框
									 //編輯框設置
	CString str;
	str = m_Listposition.GetItemText(m_OldListRow, m_OldListColumn); //抓取格子內狀態
	m_Editbox1->SetWindowText(NULL); //將文字顯示在格子上
	m_Editbox1->SetFont(this->GetFont(), FALSE);  //設置字體，不然會有突兀的感覺
	m_Editbox1->SetFocus(); //設其為焦點
	m_Editbox1->SetSel(-1);
}
//DistroyEditBox
void CPositionModify::DistroyEdit(CListCtrl *list, CEdit *distroyedit, int &Item, int &SubItem)
{
	CString  editdata;
	distroyedit->GetWindowTextW(editdata);
	if (editdata == _T(""))
		editdata = _T("0");
	list->SetItemText(Item, SubItem, editdata); //type in 對應Item
	distroyedit->DestroyWindow(); //銷毀
	PositionModifyMO();
}
//左鍵一下
void CPositionModify::OnNMClickModifylist(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	NM_LISTVIEW  *pEditCtrl = (NM_LISTVIEW *)pNMHDR;
	if (m_Edit.GetSafeHwnd())
	{
		if (!(m_OldListRow == pEditCtrl->iItem && m_OldListColumn == pEditCtrl->iSubItem)) //條件三:是否按的為同一格
		{
			DistroyEdit(&m_Listposition, &m_Edit, m_OldListRow, m_OldListColumn);
		}
	}
	*pResult = 0;
}
//兩下左鍵
void CPositionModify::OnNMDblclkModifylist(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	NM_LISTVIEW  *pEditCtrl = (NM_LISTVIEW *)pNMHDR;
	if (pEditCtrl->iItem == -1)  //條件一 : 不在表單上
	{
		if (m_Edit.GetSafeHwnd())
		{
			DistroyEdit(&m_Listposition, &m_Edit, m_OldListRow, m_OldListColumn);
		}
	}
	else //條件一 :  在表單上
	{
		if (pEditCtrl->iSubItem != 1 && pEditCtrl->iSubItem != 2 && pEditCtrl->iSubItem != 3)
		{
			//如果Edit已創建，創建
			if (m_Edit.GetSafeHwnd())
            {
                DistroyEdit(&m_Listposition, &m_Edit, m_OldListRow, m_OldListColumn);
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
                    DistroyEdit(&m_Listposition, &m_Edit, m_OldListRow, m_OldListColumn);
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
void CPositionModify::OnNMRClickModifylist(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
    CListCtrl *CList = (CListCtrl *)GetDlgItem(IDC_MODIFYLIST);
    CMenu menu, *pSubMenu;
    CPoint CurPnt;
    int ItemCount = m_Listposition.GetItemCount();//獲取項目總數
    NM_LISTVIEW  *pEditCtrl = (NM_LISTVIEW *)pNMHDR;

    if (pEditCtrl->iItem != -1 || pEditCtrl->iSubItem != 0) {
        menu.LoadMenu(IDR_MENU3);//加入菜單
        pSubMenu = menu.GetSubMenu(0);
        GetCursorPos(&CurPnt);
        pSubMenu->TrackPopupMenu(TPM_LEFTALIGN, CurPnt.x, CurPnt.y, this);//點右鍵出現的菜單位置
    }
    *pResult = 0;
}
//修改陣列值
void CPositionModify::PositionModifyMO()
{
    CWnd* pMain = AfxGetApp()->m_pMainWnd;
    for (UINT i = 0; i < ((CCommandTestDlg*)pMain)->a.PositionModifyNumber.size(); i++)
    {
        ((CCommandTestDlg*)pMain)->a.PositionModifyNumber.at(i).Address = m_Listposition.GetItemText(i, 1);
        ((CCommandTestDlg*)pMain)->a.PositionModifyNumber.at(i).VisionNumber = _ttol(m_Listposition.GetItemText(i, 2));
        ((CCommandTestDlg*)pMain)->a.PositionModifyNumber.at(i).LaserNumber = _ttol(m_Listposition.GetItemText(i, 3));
    }
}
//刷新列表
void CPositionModify::ListRefresh()
{
    CString StrBuff;
    m_Listposition.DeleteAllItems();
    int nCount = ((CCommandTestDlg*)pMain)->a.PositionModifyNumber.size();
    for (int i = 0; i < nCount; i++)
    {
        m_Listposition.InsertItem(i, NULL);
        (i > 8) ? StrBuff.Format(_T("0%d"), i + 1) : StrBuff.Format(_T("00%d"), i + 1);
        m_Listposition.SetItemText(i, 0, StrBuff);
        m_Listposition.SetItemText(i, 1, ((CCommandTestDlg*)pMain)->a.PositionModifyNumber.at(i).Address);
        StrBuff.Format(L"%d", ((CCommandTestDlg*)pMain)->a.PositionModifyNumber.at(i).VisionNumber);
        m_Listposition.SetItemText(i, 2, StrBuff);
        StrBuff.Format(L"%d", ((CCommandTestDlg*)pMain)->a.PositionModifyNumber.at(i).LaserNumber);
        m_Listposition.SetItemText(i, 3, StrBuff);
    }
    int ListnCount = m_Listposition.GetItemCount();
    m_Listposition.EnsureVisible(ListnCount - 1, FALSE);//使List中一項可見(如滾動條向下滾)
}
//插入
void CPositionModify::OnInsert3()
{
    int istat = m_Listposition.GetSelectionMark();//獲取選擇的項
    m_Listposition.InsertItem(istat, NULL);
    ((CCommandTestDlg*)pMain)->a.PositionModifyNumber.insert(((CCommandTestDlg*)pMain)->a.PositionModifyNumber.begin() + istat, { L"",-1,-1 });
    ListRefresh();
}
//刪除
void CPositionModify::OnDelete3()
{
    int istat = m_Listposition.GetSelectionMark();//獲取選擇的項
    ((CCommandTestDlg*)pMain)->a.PositionModifyNumber.erase(((CCommandTestDlg*)pMain)->a.PositionModifyNumber.begin() + istat);
    ListRefresh();
}
/*新增*/
void CPositionModify::OnBnClickedBtnnew()
{
    ((CCommandTestDlg*)pMain)->a.PositionModifyNumber.push_back({ L"",-1,-1 });
    ListRefresh();
}
/*清空*/
void CPositionModify::OnBnClickedBtnclear()
{
    if (MessageBox(L"你確定要清除?",L"提示", MB_OKCANCEL) == IDOK)
    {
        ((CCommandTestDlg*)pMain)->a.PositionModifyNumber.clear();
        ListRefresh();
    }
}

//顯示視窗時設定
void CPositionModify::OnShowWindow(BOOL bShow, UINT nStatus)
{
    CDialogEx::OnShowWindow(bShow, nStatus);
    SetWindowLong(this->m_hWnd, GWL_EXSTYLE, GetWindowLong(this->m_hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);//設置視窗為可以透明化
    this->SetLayeredWindowAttributes(0, (255 * 100) / 100, LWA_ALPHA);//不透明

}
//非活動轉活動事件
int CPositionModify::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
{
    //從非活動轉為活動改成不透明
    this->SetLayeredWindowAttributes(0, (255 * 100) / 100, LWA_ALPHA);
    return CDialogEx::OnMouseActivate(pDesktopWnd, nHitTest, message);
}
