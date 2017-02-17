// MosaicProcessing.cpp : 實作檔
//

#include "stdafx.h"
#include "CommandTest.h"
#include "MosaicProcessing.h"
#include "afxdialogex.h"



// CMosaicProcessing 對話方塊

IMPLEMENT_DYNAMIC(CMosaicProcessing, CDialogEx)

CMosaicProcessing::CMosaicProcessing(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG14, pParent)
{
    pCCommandTestDlg = (CCommandTestDlg*)AfxGetApp()->m_pMainWnd;
    num_stepadd = 0;
}

CMosaicProcessing::~CMosaicProcessing()
{
}

void CMosaicProcessing::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_PROGRESS1, m_Progress);
}


BEGIN_MESSAGE_MAP(CMosaicProcessing, CDialogEx)
    ON_WM_TIMER()
END_MESSAGE_MAP()


// CMosaicProcessing 訊息處理常式


BOOL CMosaicProcessing::OnInitDialog()
{
    CDialogEx::OnInitDialog();
    m_Progress.SetRange(0, 32);
    SetTimer(1, 100, 0);
    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX 屬性頁應傳回 FALSE
}

void CMosaicProcessing::OnTimer(UINT_PTR nIDEvent)
{
    if (nIDEvent == 1) //做動態請稍候和ProgressBar
    {
        if (num_stepadd == 37)
            num_stepadd = 0;
        num_stepadd++;
        m_Progress.SetPos(num_stepadd);
    }
    if (pCCommandTestDlg->a.RunStatusRead.MosaicStatus == 0)
    {
        KillTimer(1);
        CDialogEx::OnCancel();
    }
    else if (pCCommandTestDlg->a.RunStatusRead.MosaicStatus == 1)
    {
        KillTimer(1);
        CDialogEx::OnOK();
    }
    CDialogEx::OnTimer(nIDEvent);
}


void CMosaicProcessing::OnCancel()
{
    //CDialogEx::OnCancel();
}


void CMosaicProcessing::OnOK()
{
    //CDialogEx::OnOK();
}
