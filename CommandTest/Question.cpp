// Question.cpp : 實作檔
//

#include "stdafx.h"
#include "CommandTest.h"
#include "Question.h"
#include "afxdialogex.h"


// CQuestion 對話方塊

IMPLEMENT_DYNAMIC(CQuestion, CDialogEx)

CQuestion::CQuestion(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG4, pParent)
{

}

CQuestion::~CQuestion()
{
}

void CQuestion::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CQuestion, CDialogEx)
    ON_BN_CLICKED(IDOK, &CQuestion::OnBnClickedOk)
    ON_BN_CLICKED(IDC_BUTTON1, &CQuestion::OnBnClickedButton1)
    ON_BN_CLICKED(IDC_BUTTON2, &CQuestion::OnBnClickedButton2)
    ON_BN_CLICKED(IDCANCEL, &CQuestion::OnBnClickedCancel)
    ON_BN_CLICKED(IDC_BUTTON3, &CQuestion::OnBnClickedButton3)
END_MESSAGE_MAP()


// CQuestion 訊息處理常式


void CQuestion::OnBnClickedOk()
{
    CDialogEx::EndDialog(0);
}
void CQuestion::OnBnClickedCancel()
{
    CDialogEx::EndDialog(1);
}
void CQuestion::OnBnClickedButton1()
{
    CDialogEx::EndDialog(2);
}
void CQuestion::OnBnClickedButton2()
{
    CDialogEx::EndDialog(3);
}
void CQuestion::OnBnClickedButton3()
{
    CDialogEx::EndDialog(4);
}
