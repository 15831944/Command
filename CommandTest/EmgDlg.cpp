// EmgDlg.cpp : 實作檔
//

#include "stdafx.h"
#include "CommandTest.h"
#include "EmgDlg.h"
#include "afxdialogex.h"


// CEmgDlg 對話方塊

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


// CEmgDlg 訊息處理常式


void CEmgDlg::OnBnClickedOk()
{
	// TODO: 在此加入控制項告知處理常式程式碼
	CDialogEx::EndDialog(0);
}
