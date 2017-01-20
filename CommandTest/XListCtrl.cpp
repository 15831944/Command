// XListCtrl.cpp : 實作檔
//

#include "stdafx.h"
#include "XListCtrl.h"

// CXListCtrl

IMPLEMENT_DYNAMIC(CXListCtrl, CListCtrl)

CXListCtrl::CXListCtrl()
{

}
CXListCtrl::~CXListCtrl()
{
}
BEGIN_MESSAGE_MAP(CXListCtrl, CListCtrl)
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnNMCustomdraw)
END_MESSAGE_MAP()

// CXListCtrl 訊息處理常式
void CXListCtrl::OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	*pResult = CDRF_DODEFAULT;
	NMLVCUSTOMDRAW * lplvdr = (NMLVCUSTOMDRAW*)pNMHDR;
	NMCUSTOMDRAW &nmcd = lplvdr->nmcd;
	switch (lplvdr->nmcd.dwDrawStage)//判斷狀態
	{
	case CDDS_PREPAINT:
	{
		*pResult = CDRF_NOTIFYITEMDRAW;
		break;
	}
	case CDDS_ITEMPREPAINT://如果畫ITEM之前要作顏色改變
	{
		COLORREF ItemColor;
		if (MapItemColor.Lookup(nmcd.dwItemSpec, ItemColor))
			// 根据在 SetItemColor(DWORD iItem, COLORREF color) 设置的
			// ITEM号和COLORREF 在摸板中查找，然后进行颜色赋值。
		{
			//lplvdr->clrText = RGB(0,0,0);//ItemColor;
			lplvdr->clrTextBk = ItemColor;
			*pResult = CDRF_DODEFAULT;
		}
	}
	break;
	}
}
//設置一行顏色
void CXListCtrl::SetItemColor(DWORD iItem, COLORREF color)
{
	//m_iCurrentItem = iItem;
	//m_CurrentColor = color;

	MapItemColor.SetAt(iItem, color);//设置某行的颜色。
	this->RedrawItems(iItem, iItem);//重新染色

	//this->SetCheck(iItem,1);
	this->SetFocus();//設置焦點
	UpdateWindow();
}
//設置X行到X行顏色
void CXListCtrl::SetAllItemColor(DWORD iItemBegin ,DWORD iItemEnd, COLORREF TextBkColor)
{
	//INT_PTR ncount = this->GetItemCount();
	if (iItemBegin > 0 && iItemEnd > 0)
	{
		for (DWORD numItem = iItemBegin; numItem < iItemEnd; numItem++)
		{
			MapItemColor.SetAt(numItem, TextBkColor);
			this->RedrawItems(numItem, numItem);
		}
	}
	return;
}
