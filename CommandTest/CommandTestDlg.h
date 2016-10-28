
// CommandTestDlg.h : 標頭檔
//

#pragma once
#include "Order.h"
#include "afxcmn.h"
#include "afxwin.h"
#include "NumEdit.h"
// CCommandTestDlg 對話方塊
class CCommandTestDlg : public CDialogEx
{
// 建構
public:
	CCommandTestDlg(CWnd* pParent = NULL);	// 標準建構函式

// 對話方塊資料
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_COMMANDTEST_DIALOG };
#endif
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支援
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
// 程式碼實作
protected:
	HICON m_hIcon;
	CListCtrl m_CommandList;
	CListCtrl m_ParamList;
	CString StrBuff;
	BOOL Insert;
	UINT InsertNum;
	LONG OffsetX, OffsetY;
	BOOL GlueInformation;
    BOOL EMGInformation;
    CNumEdit NumEdit1, NumEdit2, NumEdit3, NumEdit4, NumEdit5, NumEdit6, NumEdit7, NumEdit8, NumEdit9;
public:
	COrder a;
	CDialog* m_pCameraDlg;  
	CDialog* m_pLaserDlg;
	CDialog* m_pDefaultDlg;
	CDialog* m_pPositionModifyDlg;
	CDialog* m_pLaserAdjustDlg;
	CDialog* m_pLineContinuousDlg;
    DOUBLE PixToPulsX, PixToPulsY;
	BOOL InputAuto;
	CPoint TipOffset,LaserOffset;
	LONG LaserOffsetz;
	LONG HeightLaserZero;
	BOOL CcdMode;
	BOOL m_LoopRun;
	int MaxRunNumber;
	int RunLoopNumber;

	UINT XNumber, YNumber;
	UINT BlockCount;
	CString BlockStr;
    CString CheckStrBuff;

public:
	/*刷新*/
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	/*功能*/
	afx_msg void OnBnClickedStart();
	afx_msg void OnBnClickedPause();
	afx_msg void OnBnClickedStop();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedBtnhome();
	afx_msg void OnBnClickedBtnview();
	afx_msg void OnBnClickedBtnalloffset();
	afx_msg void OnBnClickedBtncommit();
	afx_msg void OnBnClickedBtnnocommit();
	afx_msg void OnBnClickedBtnsave();
	afx_msg void OnBnClickedBtnopen();
	afx_msg void OnBnClickedBtnloaddemo();
	afx_msg void OnBnClickedBtnprintflaser();
	afx_msg void OnBnClickedBtnmodify();
	afx_msg void OnBnClickedBtnprintcline();
	//設置
	afx_msg void OnBnClickedBtndefault();
	afx_msg void OnBnClickedBtnvision();
	afx_msg void OnBnClickedBtnmodefyz();
	afx_msg void OnBnClickedBtnmodechange();
	afx_msg void OnBnClickedBtncleancount();
	afx_msg void OnBnClickedBtnlaser();
	/*列表*/
	afx_msg void OnInsert();
	afx_msg void OnDelete();
	afx_msg void OnMove();
	afx_msg void OnNMRClickList1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclkList1(NMHDR *pNMHDR, LRESULT *pResult);
	void ListRefresh(BOOL ScrollBarRefresh);
	/*命令*/
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
    afx_msg void OnBnClickedBtncommand25_1();
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
	//雷射
	afx_msg void OnBnClickedBtncommand46();
	afx_msg void OnBnClickedBtncommand47();
	afx_msg void OnBnClickedBtncommand48();
	afx_msg void OnBnClickedBtncommand49();
	afx_msg void OnBnClickedBtncommand50();
	//副程式
	afx_msg void OnBnClickedBtncommand43();
	afx_msg void OnBnClickedBtncommand44();
	afx_msg void OnBnClickedBtncommand45();
	//外部函數
	CString CommandResolve(CString Command, UINT Choose);
	void SaveParameter();
	void LoadParameter();
    void SaveDefault();
    void LoadDefault();
	void Counter();
	//介面透明化
	afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
    afx_msg void OnDestroy();

    
    afx_msg void OnBnClickedBtncommand51();
    afx_msg void OnBnClickedBtncommand52();
    afx_msg void OnBnClickedBtntest();
};


