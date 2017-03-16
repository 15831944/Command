#pragma once
#include "MoveButton.h"
#include "Model.h"
#include "afxwin.h"
#include "NumEdit.h"
#include "XButton.h"

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
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支援

    DECLARE_MESSAGE_MAP()

public: //變數
    CMoveButton m_Yup, m_Ydown, m_Xup, m_Xdown, m_Zup, m_Zdown, m_Wup, m_Wdown;
	UINT RaiChoose;
	void* MilModel;
	CDialog* m_pCModel;
    CNumEdit m_labsx;
    CNumEdit m_labsy;
    CNumEdit m_labsz;
    CNumEdit m_dabsw;
    BOOL TCOffstAdjust;
    BOOL TipToCCDAdjust;
    BOOL PixToPlusAdjust;
    CXButton m_WRegulate;     
    CXButton m_SetTipToCcd;     
    CXButton m_SetPixToPuls;
    BOOL WJOGMode;
public:// 函數
    void MoveXYZW(int MoveX, int MoveY, int MoveZ, double MoveW);
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    virtual void OnOK();
    virtual BOOL DestroyWindow();
	afx_msg void OnSpeedTypeChange(UINT uId);
	afx_msg void OnBnClickedBtnmove();
    afx_msg void OnBnClickedBtnwmode();
    afx_msg void OnBnClickedBtnfocus();
    afx_msg void OnBnClickedBtnfocusset();
    afx_msg void OnBnClickedBtntableset();
    afx_msg void OnBnClickedBtnwhome();
    afx_msg void OnBnClickedBtnlimitunlock();
	afx_msg void OnBnClickedBtnmodel();
    afx_msg void OnBnClickedBtnmodelmatch();
    afx_msg void OnBnClickedBtnsettiptoccd();
    afx_msg LRESULT OnBnLClickedBtnsettiptoccd(WPARAM wParam, LPARAM lParam);
    afx_msg void OnBnClickedBtnsetpixtopuls();
    afx_msg LRESULT OnBnLClickedBtnsetpixtopuls(WPARAM wParam, LPARAM lParam);
    afx_msg void OnBnClickedBtnwregulate();
    afx_msg LRESULT OnBnLClickedBtnwregulate(WPARAM wParam, LPARAM lParam);
    afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
    afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
  
};
