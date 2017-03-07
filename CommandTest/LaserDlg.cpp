// LaserDlg.cpp : 實作檔
//

#include "stdafx.h"
#include "CommandTest.h"
#include "LaserDlg.h"
#include "afxdialogex.h"
#include "CommandTestDlg.h"


// CLaserDlg 對話方塊

IMPLEMENT_DYNAMIC(CLaserDlg, CDialogEx)

CLaserDlg::CLaserDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG6, pParent)
{
	pMain = AfxGetApp()->m_pMainWnd;
}

CLaserDlg::~CLaserDlg()
{
}

void CLaserDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CLaserDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BTNZERO, &CLaserDlg::OnBnClickedBtnzero)
	ON_BN_CLICKED(IDC_BTNLASERTOB, &CLaserDlg::OnBnClickedBtnlasertob)
	ON_BN_CLICKED(IDC_BTNB, &CLaserDlg::OnBnClickedBtnb)
	ON_BN_CLICKED(IDC_BTNSETLASER, &CLaserDlg::OnBnClickedBtnsetlaser)
    ON_WM_SHOWWINDOW()
    ON_WM_MOUSEACTIVATE()
END_MESSAGE_MAP()


// CLaserDlg 訊息處理常式
BOOL CLaserDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();
    CString StrBuff;
    StrBuff.Format(_T("LaserToTip:%d,%d,%d,%d"), ((CCommandTestDlg*)pMain)->LaserOffset.x, ((CCommandTestDlg*)pMain)->LaserOffset.y, ((CCommandTestDlg*)pMain)->LaserOffsetz, ((CCommandTestDlg*)pMain)->HeightLaserZero);
    SetDlgItemText(IDC_STALASERTOTIP, StrBuff);
    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX 屬性頁應傳回 FALSE
}
/*雷射歸零*/
void CLaserDlg::OnBnClickedBtnzero()
{
	((CCommandTestDlg*)pMain)->a.m_Action.LA_SetZero();
    ((CCommandTestDlg*)pMain)->HeightLaserZero = ((CCommandTestDlg*)pMain)->a.m_Action.m_HeightLaserZero;
    ((CCommandTestDlg*)pMain)->LaserOffsetz = ((CCommandTestDlg*)pMain)->a.m_Action.m_OffSetLaserZ;
    CString StrBuff,StrBuff1;
    GetDlgItemTextW(IDC_STALASERTOTIP, StrBuff);
    StrBuff1.Format(_T(",%d,%d"), ((CCommandTestDlg*)pMain)->HeightLaserZero, ((CCommandTestDlg*)pMain)->LaserOffsetz);
    StrBuff = StrBuff + StrBuff1;
    SetDlgItemText(IDC_STALASERTOTIP, StrBuff);
}
/*雷射到B點*/
void CLaserDlg::OnBnClickedBtnlasertob()
{
	((CCommandTestDlg*)pMain)->a.m_Action.LA_Butt_GoLAtoBPoint();
    ((CCommandTestDlg*)pMain)->LaserOffset.x = ((CCommandTestDlg*)pMain)->a.m_Action.m_OffSetLaserX;
    ((CCommandTestDlg*)pMain)->LaserOffset.y = ((CCommandTestDlg*)pMain)->a.m_Action.m_OffSetLaserY;
	CString StrBuff;
	StrBuff.Format(_T("LaserToTip:%d,%d"), ((CCommandTestDlg*)pMain)->LaserOffset.x, ((CCommandTestDlg*)pMain)->LaserOffset.y);
	SetDlgItemText(IDC_STALASERTOTIP, StrBuff);
}
/*雷射B點*/
void CLaserDlg::OnBnClickedBtnb()
{
	((CCommandTestDlg*)pMain)->a.m_Action.LA_Butt_GoBPoint();
}
/*雷射設定*/
void CLaserDlg::OnBnClickedBtnsetlaser()
{
	((CCommandTestDlg*)pMain)->a.m_Action.LA_SetInit();
}
//顯示視窗時設定
void CLaserDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
    CDialogEx::OnShowWindow(bShow, nStatus);

    SetWindowLong(this->m_hWnd, GWL_EXSTYLE, GetWindowLong(this->m_hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);//設置視窗為可以透明化
    this->SetLayeredWindowAttributes(0, (255 * 100) / 100, LWA_ALPHA);//不透明

}
//非活動轉活動事件
int CLaserDlg::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
{
    //從非活動轉為活動改成不透明
    this->SetLayeredWindowAttributes(0, (255 * 100) / 100, LWA_ALPHA);
    return CDialogEx::OnMouseActivate(pDesktopWnd, nHitTest, message);
}
