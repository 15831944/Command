#pragma once
#include "MoveButton.h"

// CCamera 對話方塊

class CCamera : public CDialogEx
{
	DECLARE_DYNAMIC(CCamera)

public:
	CCamera(CWnd* pParent = NULL);   // 標準建構函式
	virtual ~CCamera();

// 對話方塊資料
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG2 };
#endif
public: //變數
    CMoveButton m_Yup, m_Ydown, m_Xup, m_Xdown, m_Zup, m_Zdown;
    UINT RaiChoose;
    
public:// 函數
    void MoveXYZ(int MoveX, int MoveY, int MoveZ);
    virtual BOOL OnInitDialog();
    virtual void OnCancel();
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    afx_msg void OnSpeedTypeChange(UINT uId);
    afx_msg void OnBnClickedBtnmove();
    afx_msg void OnBnClickedBtnmodel();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支援
    
	DECLARE_MESSAGE_MAP()
   
};
