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
BOOL CLaserDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();
    CString StrBuff;
    StrBuff.Format(_T("LaserToTip:%d,%d,%d,%d"), ((CCommandTestDlg*)pMain)->LaserOffset.x, ((CCommandTestDlg*)pMain)->LaserOffset.y, ((CCommandTestDlg*)pMain)->LaserOffsetz, ((CCommandTestDlg*)pMain)->HeightLaserZero);
    SetDlgItemText(IDC_STALASERTOTIP, StrBuff);
    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX �ݩʭ����Ǧ^ FALSE
}
/*�p�g�k�s*/
void CLaserDlg::OnBnClickedBtnzero()
{
	((CCommandTestDlg*)pMain)->a.m_Action.LA_SetZero();
    ((CCommandTestDlg*)pMain)->HeightLaserZero = ((CCommandTestDlg*)pMain)->a.m_Action.g_HeightLaserZero;
    ((CCommandTestDlg*)pMain)->LaserOffsetz = ((CCommandTestDlg*)pMain)->a.m_Action.g_OffSetLaserZ;
    CString StrBuff,StrBuff1;
    GetDlgItemTextW(IDC_STALASERTOTIP, StrBuff);
    StrBuff1.Format(_T(",%d,%d"), ((CCommandTestDlg*)pMain)->HeightLaserZero, ((CCommandTestDlg*)pMain)->LaserOffsetz);
    StrBuff = StrBuff + StrBuff1;
    SetDlgItemText(IDC_STALASERTOTIP, StrBuff);
}
/*�p�g��B�I*/
void CLaserDlg::OnBnClickedBtnlasertob()
{
	((CCommandTestDlg*)pMain)->a.m_Action.LA_Butt_GoLAtoBPoint();
    ((CCommandTestDlg*)pMain)->LaserOffset.x = ((CCommandTestDlg*)pMain)->a.m_Action.g_OffSetLaserX;
    ((CCommandTestDlg*)pMain)->LaserOffset.y = ((CCommandTestDlg*)pMain)->a.m_Action.g_OffSetLaserY;
	CString StrBuff;
	StrBuff.Format(_T("LaserToTip:%d,%d"), ((CCommandTestDlg*)pMain)->LaserOffset.x, ((CCommandTestDlg*)pMain)->LaserOffset.y);
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



