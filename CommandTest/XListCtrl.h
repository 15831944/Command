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
    CMap<DWORD64, DWORD64&, COLORREF, COLORREF&> MapItemColor;

public:  //�۩w�q�[��
    afx_msg void OnNMCustomdraw(NMHDR * pNMHDR, LRESULT * pResult);
    void SetItemColor(DWORD64 iItem, COLORREF color);
    void SetAllItemColor(DWORD64 iItemBegin, DWORD64 iItemEnd, COLORREF TextBkColor);
};


