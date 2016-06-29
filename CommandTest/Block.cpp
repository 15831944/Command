// Block.cpp : ��@��
//

#include "stdafx.h"
#include "CommandTest.h"
#include "Block.h"
#include "afxdialogex.h"
#include "CommandTestDlg.h"
#define BlockBtnNum 10000
#define MaxBlockBtnNum 15000

// CBlock ��ܤ��

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


// CBlock �T���B�z�`��


BOOL CBlock::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	/*�]�w�}�C�j�p*/
	CWnd* pMain = AfxGetApp()->m_pMainWnd;
	CRect rect(10, 10, 30, 30);
	Button = new CButton[((CCommandTestDlg*)pMain)->XNumber * ((CCommandTestDlg*)pMain)->YNumber];
	/*�s�W����*/
	for (int i = 0; i < ((CCommandTestDlg*)pMain)->YNumber; i++)
	{
		for (int j = 0; j < ((CCommandTestDlg*)pMain)->XNumber; j++)
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
	/*�]�w�����j�p*/
	int Width = ((CCommandTestDlg*)pMain)->XNumber * 25 + 20;
	int Height = ((CCommandTestDlg*)pMain)->YNumber * 25 + 20 + 50;
	SetWindowPos(this, 0, 0, Width, Height, SWP_NOZORDER);
	CenterWindow();
	/*�]�wOK���s��m*/
	CRect ClientRect;
	this->GetClientRect(ClientRect);
	GetDlgItem(IDOK)->MoveWindow(ClientRect.left, ClientRect.Height() - 30, ClientRect.Width(), 30);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX �ݩʭ����Ǧ^ FALSE
}
void CBlock::OnCancel()
{
	// TODO: �b���[�J�S�w���{���X�M (��) �I�s�����O

	CDialogEx::OnCancel();
}
void CBlock::OnOK()
{
    CString StrBuff;
    CWnd* pMain = AfxGetApp()->m_pMainWnd;
    ((CCommandTestDlg*)pMain)->BlockCount = 0;//���_�ƶq�M��
    ((CCommandTestDlg*)pMain)->BlockStr = _T("");//���_�}�C�M��
    for (int i = 0; i <= TotalBlock; i++)
    {
        GetDlgItemText(BlockBtnNum +i, StrBuff);
        if (StrBuff == _T("x"))
        {
            StrBuff.Format(_T(",%d-%d"), (i % ((CCommandTestDlg*)pMain)->XNumber) + 1, (i / ((CCommandTestDlg*)pMain)->YNumber) + 1);
            ((CCommandTestDlg*)pMain)->BlockStr = ((CCommandTestDlg*)pMain)->BlockStr + StrBuff;
            ((CCommandTestDlg*)pMain)->BlockCount++;
        }
    }
    _cwprintf(L"%s", ((CCommandTestDlg*)pMain)->BlockStr);
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
