#pragma once
#include "MoveButton.h"
#include "Model.h"
#include "afxwin.h"
#include "NumEdit.h"

// CCamera ��ܤ��

class CCamera : public CDialogEx
{
	DECLARE_DYNAMIC(CCamera)

public:
	CCamera(CWnd* pParent = NULL);   // �зǫغc�禡
	virtual ~CCamera();

// ��ܤ�����
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG2 };
#endif
public: //�ܼ�
	CMoveButton m_Yup, m_Ydown, m_Xup, m_Xdown, m_Zup, m_Zdown;
	UINT RaiChoose;
	void* MilModel;
	CDialog* m_pCModel;
    CNumEdit m_labsx;
    CNumEdit m_labsy;
    CNumEdit m_labsz;
    CNumEdit m_dabsw;
    BOOL TCOffstAdjust;
public:// ���
    void MoveXYZ(int MoveX, int MoveY, int MoveZ, double MoveW);
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnSpeedTypeChange(UINT uId);
	afx_msg void OnBnClickedBtnmove();
	afx_msg void OnBnClickedBtnmodel();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �䴩
	
	DECLARE_MESSAGE_MAP()
   
public:
	afx_msg void OnBnClickedBtnsettiptoccd();
    afx_msg void OnBnClickedBtnlimitunlock();
	afx_msg void OnBnClickedBtnsetpixtopuls();
    afx_msg void OnBnClickedBtnmodelmatch();
    afx_msg void OnBnClickedBtnfocusset();
    afx_msg void OnBnClickedBtnfocus();
    afx_msg void OnBnClickedBtntableset();
    afx_msg void OnBnClickedBtnwregulate();
    virtual void OnOK();
    virtual BOOL DestroyWindow();
    afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
    afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
    
};
