// LaserDlg.cpp : ��@��
//

#include "stdafx.h"
#include "CommandTest.h"
#include "LaserDlg.h"
#include "afxdialogex.h"
#include "CommandTestDlg.h"


// CLaserDlg ��ܤ��

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
END_MESSAGE_MAP()


// CLaserDlg �T���B�z�`��
/*�p�g�k�s*/
void CLaserDlg::OnBnClickedBtnzero()
{
    ((CCommandTestDlg*)pMain)->a.m_Action.LA_SetZero();
}
/*�p�g��B�I*/
void CLaserDlg::OnBnClickedBtnlasertob()
{
    ((CCommandTestDlg*)pMain)->a.m_Action.LA_Butt_GoLAtoBPoint();
    CString StrBuff;
    StrBuff.Format(_T("LaserToTip:%d,%d"), ((CCommandTestDlg*)pMain)->a.m_Action.g_OffSetLaserX, ((CCommandTestDlg*)pMain)->a.m_Action.g_OffSetLaserY);
    SetDlgItemText(IDC_STALASERTOTIP, StrBuff);
}
/*�p�gB�I*/
void CLaserDlg::OnBnClickedBtnb()
{
    ((CCommandTestDlg*)pMain)->a.m_Action.LA_Butt_GoBPoint();
}
/*�p�g�]�w*/
void CLaserDlg::OnBnClickedBtnsetlaser()
{
    ((CCommandTestDlg*)pMain)->a.m_Action.LA_SetInit();
}
