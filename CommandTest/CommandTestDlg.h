
// CommandTestDlg.h : ���Y��
//

#pragma once
#include "Order.h"
#include "afxcmn.h"
#include "afxwin.h"
// CCommandTestDlg ��ܤ��
class CCommandTestDlg : public CDialogEx
{
// �غc
public:
	CCommandTestDlg(CWnd* pParent = NULL);	// �зǫغc�禡

// ��ܤ�����
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_COMMANDTEST_DIALOG };
#endif
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV �䴩
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
// �{���X��@
protected:
	HICON m_hIcon;
	CListCtrl m_CommandList;
	CListCtrl m_ParamList;
	CString StrBuff;
	BOOL Insert;
	UINT InsertNum;
	LONG OffsetX, OffsetY;
public:
	COrder a;
	CDialog* m_pCameraDlg;  
	BOOL InputAuto;
	CPoint TipOffset;
	BOOL CcdMode;
    BOOL m_LoopRun;
    BOOL RunSwitch;
    int MaxRunNumber;
public:
	/*��s*/
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	/*�\��*/
	afx_msg void OnBnClickedStart();
	afx_msg void OnBnClickedPause();
	afx_msg void OnBnClickedStop();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedBtnhome();
	afx_msg void OnBnClickedBtnview();
    static  UINT RunThread(LPVOID pParam);
    //�]�m
    afx_msg void OnBnClickedBtndefault();
    afx_msg void OnBnClickedBtnvision();
    afx_msg void OnBnClickedBtnmodefyz();
    afx_msg void OnBnClickedBtnmodechange();
    afx_msg void OnBnClickedBtncleancount();
	/*�C��*/
	afx_msg void OnInsert();
	afx_msg void OnDelete();
	afx_msg void OnNMRClickList1(NMHDR *pNMHDR, LRESULT *pResult);
	void ListRefresh(BOOL ScrollBarRefresh);
	/*�R�O*/
	afx_msg void OnBnClickedBtncommand1();
	afx_msg void OnBnClickedBtncommand2();
	afx_msg void OnBnClickedBtncommand3();
	afx_msg void OnBnClickedBtncommand4();
	afx_msg void OnBnClickedBtncommand4_2();
	afx_msg void OnBnClickedBtncommand5();
	afx_msg void OnBnClickedBtncommand6();
	afx_msg void OnBnClickedBtncommand7();
	afx_msg void OnBnClickedBtncommand8();
	afx_msg void OnBnClickedBtncommand9();
	afx_msg void OnBnClickedBtncommand10();
	afx_msg void OnBnClickedBtncommand11();
	afx_msg void OnBnClickedBtncommand12();
	afx_msg void OnBnClickedBtncommand13();
	afx_msg void OnBnClickedBtncommand14();
	afx_msg void OnBnClickedBtncommand15();
	afx_msg void OnBnClickedBtncommand16();
	afx_msg void OnBnClickedBtncommand17();
	afx_msg void OnBnClickedBtncommand18();
	afx_msg void OnBnClickedBtncommand19();
	afx_msg void OnBnClickedBtncommand19_2();
	afx_msg void OnBnClickedBtncommand20();
	afx_msg void OnBnClickedBtncommand21();
	afx_msg void OnBnClickedBtncommand22();
	afx_msg void OnBnClickedBtncommand23();
	afx_msg void OnBnClickedBtncommand24();
	afx_msg void OnBnClickedBtncommand25();
	afx_msg void OnBnClickedBtncommand26();
	afx_msg void OnBnClickedBtncommand27();
	afx_msg void OnBnClickedBtncommand28();
	afx_msg void OnBnClickedBtncommand29();
	afx_msg void OnBnClickedBtncommand30();
	afx_msg void OnBnClickedBtncommand31();
    afx_msg void OnBnClickedBtncommand32_0();
	afx_msg void OnBnClickedBtncommand32();
	afx_msg void OnBnClickedBtncommand33();
	afx_msg void OnBnClickedBtncommand34();
	afx_msg void OnBnClickedBtncommand35();
	afx_msg void OnBnClickedBtncommand36();
	afx_msg void OnBnClickedBtncommand37();
	afx_msg void OnBnClickedBtncommand38();
	afx_msg void OnBnClickedBtncommand40();
	afx_msg void OnBnClickedBtncommand41();
	//�Ƶ{��
	afx_msg void OnBnClickedBtncommand43();
	afx_msg void OnBnClickedBtncommand44();
	afx_msg void OnBnClickedBtncommand45();
    //�~�����
    CString CommandResolve(CString Command, UINT Choose);
};


