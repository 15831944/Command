#pragma once
#include "MoveButton.h"
#include "Model.h"

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
	void* MilModel;
	DOUBLE PixToPulsX, PixToPulsY;
	CDialog* m_pCModel;
    LONG FocusPoint;
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
   
public:
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnBnClickedButton4();

	virtual void OnOK();
	afx_msg void OnBnClickedBtnfocus();
    afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
    afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
    virtual BOOL DestroyWindow();
};
