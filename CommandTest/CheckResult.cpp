// CheckResult.cpp : 實作檔
//

#include "stdafx.h"
#include "CommandTest.h"
#include "CheckResult.h"
#include "afxdialogex.h"


// CCheckResult 對話方塊

IMPLEMENT_DYNAMIC(CCheckResult, CDialogEx)

CCheckResult::CCheckResult(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG12, pParent)
{

}

CCheckResult::~CCheckResult()
{
}

void CCheckResult::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CCheckResult, CDialogEx)
END_MESSAGE_MAP()


// CCheckResult 訊息處理常式
