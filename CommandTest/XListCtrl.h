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
    CMap<DWORD64, DWORD64&, COLORREF, COLORREF&> MapItemColor;

public:  //自定義涵式
    afx_msg void OnNMCustomdraw(NMHDR * pNMHDR, LRESULT * pResult);
    void SetItemColor(DWORD64 iItem, COLORREF color);
    void SetAllItemColor(DWORD64 iItemBegin, DWORD64 iItemEnd, COLORREF TextBkColor);
};


