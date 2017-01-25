// CheckResult.cpp : ��@��
//

#include "stdafx.h"
#include "CommandTest.h"
#include "CheckResult.h"
#include "afxdialogex.h"


// CCheckResult ��ܤ��

IMPLEMENT_DYNAMIC(CCheckResult, CDialogEx)

CCheckResult::CCheckResult(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG12, pParent)
{
    m_pLoadlist = NULL;
    lEndthread = 0;
    pCCommandTestDlg = (CCommandTestDlg*)AfxGetApp()->m_pMainWnd;
}

CCheckResult::~CCheckResult()
{
}

void CCheckResult::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST1, m_ListCheck);
}


BEGIN_MESSAGE_MAP(CCheckResult, CDialogEx)
    ON_WM_CLOSE()
    ON_WM_DESTROY()
END_MESSAGE_MAP()


// CCheckResult �T���B�z�`��



BOOL CCheckResult::OnInitDialog()
{
    CDialogEx::OnInitDialog();
    ASSERT(::IsWindow(m_ListCheck.m_hWnd));
    ASSERT(m_ListCheck.InsertColumn(1, _T("No."), LVCFMT_FIXED_WIDTH | LVCFMT_CENTER, 40, -1) != -1);
    ASSERT(m_ListCheck.InsertColumn(2, _T("Command"), LVCFMT_FIXED_WIDTH | LVCFMT_CENTER, 125, -1) != -1);
    ASSERT(m_ListCheck.InsertColumn(3, _T("Result"), LVCFMT_FIXED_WIDTH | LVCFMT_CENTER, 75, -1) != -1);
    ASSERT(m_ListCheck.InsertColumn(4, _T("�y��"), LVCFMT_FIXED_WIDTH | LVCFMT_CENTER, 150, -1) != -1);
    ASSERT(!m_ListCheck.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT));    //LVS_SHOWSELALWAYS �۰���ܴ�в��ʨ쪺��m
    if (m_pLoadlist != NULL)
    {
        DWORD aaaa = 0;
        if (GetExitCodeThread(m_pLoadlist->m_hThread, &aaaa))
            _cwprintf(L"%d\n", aaaa);
        else
            _cwprintf(L"%d\n", GetLastError());
    }
    if (m_pLoadlist == NULL)
    {
        lEndthread = 0;
        m_pLoadlist = AfxBeginThread(LoadListThread, this);
    }
    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX �ݩʭ����Ǧ^ FALSE
}
//���J List �����
UINT CCheckResult::LoadListThread(LPVOID pParam)
{ 
    UINT ListCount = 0;
    while (((CCheckResult*)pParam)->lEndthread == 0 && ((CCheckResult*)pParam)->pCCommandTestDlg->a.CheckFinishRecord.size() > ListCount)
    {
        CString str_no, str_position;
        str_no.Format(L"%d", ListCount + 1);
        if(((CCheckResult*)pParam)->lEndthread == 0)
            ((CCheckResult*)pParam)->m_ListCheck.InsertItem(ListCount, str_no);
        if (((CCheckResult*)pParam)->lEndthread == 0)
            ((CCheckResult*)pParam)->m_ListCheck.SetItemText(ListCount, 1, ((CCheckResult*)pParam)->pCCommandTestDlg->a.CheckFinishRecord.at(ListCount).CheckData.CheckMode);
        if (((CCheckResult*)pParam)->lEndthread == 0)
            ((CCheckResult*)pParam)->m_ListCheck.SetItemText(ListCount, 2, ((CCheckResult*)pParam)->pCCommandTestDlg->a.CheckFinishRecord.at(ListCount).Result);
        if (((CCheckResult*)pParam)->lEndthread == 0)
        {
            if (((CCheckResult*)pParam)->pCCommandTestDlg->a.CheckFinishRecord.at(ListCount).Result == L"OK")
                ((CCheckResult*)pParam)->m_ListCheck.SetItemColor(ListCount, RGB(0, 255, 0));
            else if (((CCheckResult*)pParam)->pCCommandTestDlg->a.CheckFinishRecord.at(ListCount).Result == L"NG")
                ((CCheckResult*)pParam)->m_ListCheck.SetItemColor(ListCount, RGB(255, 0, 0));
            else if (((CCheckResult*)pParam)->pCCommandTestDlg->a.CheckFinishRecord.at(ListCount).Result == L"Err")
                ((CCheckResult*)pParam)->m_ListCheck.SetItemColor(ListCount, RGB(255, 255, 0));
        }   
        str_position.Format(L"%d,%d", ((CCheckResult*)pParam)->pCCommandTestDlg->a.CheckFinishRecord.at(ListCount).CheckData.Position.X, ((CCheckResult*)pParam)->pCCommandTestDlg->a.CheckFinishRecord.at(ListCount).CheckData.Position.Y);
        if (((CCheckResult*)pParam)->lEndthread == 0)
            ((CCheckResult*)pParam)->m_ListCheck.SetItemText(ListCount, 3, str_position);
        ListCount++;
    } 
    ExitThread(456);
}
//��������
void CCheckResult::OnClose()
{
    _cwprintf(L"OnClose\n");
    InterlockedIncrement(&lEndthread);
    CDialogEx::OnClose();
}
//��������
void CCheckResult::OnCancel()
{
    _cwprintf(L"OnCancel\n");
    CDialogEx::OnCancel();
}
//�P������
BOOL CCheckResult::DestroyWindow()
{
    _cwprintf(L"DestroyWindow\n");
    return CDialogEx::DestroyWindow();
}
//�P��
void CCheckResult::OnDestroy()
{
    _cwprintf(L"OnDestroy\n");
    CDialogEx::OnDestroy();
}
