#pragma once


// CXListCtrl

class CXListCtrl : public CListCtrl
{
	DECLARE_DYNAMIC(CXListCtrl)

public:
	CXListCtrl();
	virtual ~CXListCtrl();
protected:
	DECLARE_MESSAGE_MAP()

public:  //自定義成員
    CMap<DWORD, DWORD&, COLORREF, COLORREF&> MapItemColor;

public:  //自定義涵式
    afx_msg void OnNMCustomdraw(NMHDR * pNMHDR, LRESULT * pResult);
    void SetItemColor(DWORD iItem, COLORREF color);
    void SetAllItemColor(DWORD iItemBegin, DWORD iItemEnd, COLORREF TextBkColor);
};


