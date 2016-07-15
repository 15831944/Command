// Block.cpp : 實作檔
//

#include "stdafx.h"
#include "CommandTest.h"
#include "Block.h"
#include "afxdialogex.h"
#include "CommandTestDlg.h"
#define BlockBtnNum 10000
#define MaxBlockBtnNum 15000

// CBlock 對話方塊

IMPLEMENT_DYNAMIC(CBlock, CDialogEx)

CBlock::CBlock(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG5, pParent)
{
	Button = NULL;
    TotalBlock = 0;
}

CBlock::~CBlock()
{
	if (Button)
		delete[]Button;
}

void CBlock::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CBlock, CDialogEx)
	ON_CONTROL_RANGE(BN_CLICKED, BlockBtnNum, MaxBlockBtnNum, &CBlock::OnBlockClick)
END_MESSAGE_MAP()


// CBlock 訊息處理常式


BOOL CBlock::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	/*設定陣列大小*/
	CWnd* pMain = AfxGetApp()->m_pMainWnd;
	CRect rect(10, 10, 30, 30);
	Button = new CButton[((CCommandTestDlg*)pMain)->XNumber * ((CCommandTestDlg*)pMain)->YNumber];
	/*新增控件*/
	for (UINT i = 0; i < ((CCommandTestDlg*)pMain)->YNumber; i++)
	{
		for (UINT j = 0; j < ((CCommandTestDlg*)pMain)->XNumber; j++)
		{   
			Button[TotalBlock].Create(_T(""), WS_CHILD | WS_VISIBLE, rect, this, BlockBtnNum + TotalBlock);
			rect.left = rect.left + 25;
			rect.right = rect.right + 25;
            TotalBlock++;
		}
		rect.top = rect.top + 25;
		rect.bottom = rect.bottom + 25;
		rect.left = 10;
		rect.right = 30;
	}
	/*設定視窗大小*/
	int Width = ((CCommandTestDlg*)pMain)->XNumber * 25 + 20;
	int Height = ((CCommandTestDlg*)pMain)->YNumber * 25 + 20 + 50;
	SetWindowPos(this, 0, 0, Width, Height, SWP_NOZORDER);
	CenterWindow();
	/*設定OK按鈕位置*/
	CRect ClientRect;
	this->GetClientRect(ClientRect);
	GetDlgItem(IDOK)->MoveWindow(ClientRect.left, ClientRect.Height() - 30, ClientRect.Width(), 30);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX 屬性頁應傳回 FALSE
}
void CBlock::OnCancel()
{
	// TODO: 在此加入特定的程式碼和 (或) 呼叫基底類別

	CDialogEx::OnCancel();
}
void CBlock::OnOK()
{
    CString StrBuff;
    CWnd* pMain = AfxGetApp()->m_pMainWnd;
    ((CCommandTestDlg*)pMain)->BlockCount = 0;//阻斷數量清除
    ((CCommandTestDlg*)pMain)->BlockStr = _T("");//阻斷陣列清空
    for (int i = 0; i <= TotalBlock; i++)
    {
        GetDlgItemText(BlockBtnNum +i, StrBuff);
        if (StrBuff == _T("x"))
        {
            StrBuff.Format(_T(",%d-%d"), (i % ((CCommandTestDlg*)pMain)->XNumber) + 1, (i / ((CCommandTestDlg*)pMain)->XNumber) + 1);
            ((CCommandTestDlg*)pMain)->BlockStr = ((CCommandTestDlg*)pMain)->BlockStr + StrBuff;
            ((CCommandTestDlg*)pMain)->BlockCount++;
        }
    }
#ifdef PRINTF
    _cwprintf(L"%s", ((CCommandTestDlg*)pMain)->BlockStr);
#endif
	CDialogEx::OnOK();
}
void CBlock::OnBlockClick(UINT uId)
{
    CString StrBuff;
    GetDlgItem(uId)->GetWindowTextW(StrBuff);
    if (StrBuff == _T(""))
    {
        GetDlgItem(uId)->SetWindowTextW(_T("x"));
    }
    else
    {
        GetDlgItem(uId)->SetWindowTextW(_T(""));
    }
}
