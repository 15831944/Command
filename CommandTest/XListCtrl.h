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

public:  //�۩w�q����
    CMap<DWORD, DWORD&, COLORREF, COLORREF&> MapItemColor;

public:  //�۩w�q�[��
    afx_msg void OnNMCustomdraw(NMHDR * pNMHDR, LRESULT * pResult);
    void SetItemColor(DWORD iItem, COLORREF color);
    void SetAllItemColor(DWORD iItemBegin, DWORD iItemEnd, COLORREF TextBkColor);
};


