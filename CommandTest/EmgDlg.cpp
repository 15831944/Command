// EmgDlg.cpp : ��@��
//

#include "stdafx.h"
#include "CommandTest.h"
#include "EmgDlg.h"
#include "afxdialogex.h"


// CEmgDlg ��ܤ��

IMPLEMENT_DYNAMIC(CEmgDlg, CDialogEx)

CEmgDlg::CEmgDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG10, pParent)
{

}

CEmgDlg::~CEmgDlg()
{
}

void CEmgDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CEmgDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CEmgDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CEmgDlg �T���B�z�`��


void CEmgDlg::OnBnClickedOk()
{
	// TODO: �b���[�J����i���B�z�`���{���X
	CDialogEx::EndDialog(0);
}
