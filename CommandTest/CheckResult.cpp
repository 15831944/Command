// CheckResult.cpp : ��@��
//

#include "stdafx.h"
#include "CommandTest.h"
#include "CheckResult.h"
#include "afxdialogex.h"
#include "PictureViewDlg.h"

// CCheckResult ��ܤ��

IMPLEMENT_DYNAMIC(CCheckResult, CDialogEx)

CCheckResult::CCheckResult(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG12, pParent)
{
    NoPushEsc = TRUE;
    m_pLoadlist = NULL;
    lEndthread = 0;
    pCCommandTestDlg = (CCommandTestDlg*)AfxGetApp()->m_pMainWnd;
    m_pPictureViewDlg = NULL;
}
CCheckResult::~CCheckResult()
{
    if (m_pPictureViewDlg != NULL)
    {
        delete (CDialog*)m_pPictureViewDlg;
        m_pPictureViewDlg = NULL;
    }
}
void CCheckResult::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST1, m_ListCheck);
}
BEGIN_MESSAGE_MAP(CCheckResult, CDialogEx)
    ON_WM_CLOSE()
    ON_WM_DESTROY()
    ON_NOTIFY(NM_DBLCLK, IDC_LIST1, &CCheckResult::OnNMDblclkList1)
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
                ((CCheckResult*)pParam)->m_ListCheck.SetItemColor(ListCount, RGB(255, 102, 0));//��
            else if (((CCheckResult*)pParam)->pCCommandTestDlg->a.CheckFinishRecord.at(ListCount).Result == L"Err")
                ((CCheckResult*)pParam)->m_ListCheck.SetItemColor(ListCount, RGB(255,0,0));
        }   
        if (((CCheckResult*)pParam)->lEndthread == 0)
        str_position.Format(L"%d,%d", ((CCheckResult*)pParam)->pCCommandTestDlg->a.CheckFinishRecord.at(ListCount).CheckData.Position.X, ((CCheckResult*)pParam)->pCCommandTestDlg->a.CheckFinishRecord.at(ListCount).CheckData.Position.Y);
        if (((CCheckResult*)pParam)->lEndthread == 0)
            ((CCheckResult*)pParam)->m_ListCheck.SetItemText(ListCount, 3, str_position);
        ListCount++;
    } 
    UINT i = 0;
    while (((CCheckResult*)pParam)->lEndthread == 0 && ((CCheckResult*)pParam)->pCCommandTestDlg->a.AreaCheckFinishRecord.size() > i)
    {
        CString str_no, str_position;
        str_no.Format(L"%d", ListCount + 1);
        if (((CCheckResult*)pParam)->lEndthread == 0)
            ((CCheckResult*)pParam)->m_ListCheck.InsertItem(ListCount, str_no);
        if (((CCheckResult*)pParam)->lEndthread == 0)
            ((CCheckResult*)pParam)->m_ListCheck.SetItemText(ListCount, 1, L"AreaCheck");
        if (((CCheckResult*)pParam)->pCCommandTestDlg->a.AreaCheckFinishRecord.at(i).Result)
        {
            if (((CCheckResult*)pParam)->lEndthread == 0)
                ((CCheckResult*)pParam)->m_ListCheck.SetItemText(ListCount, 2, L"OK");
            if (((CCheckResult*)pParam)->lEndthread == 0)
                ((CCheckResult*)pParam)->m_ListCheck.SetItemColor(ListCount, RGB(0, 255, 0));
        }
        else
        {
            if (((CCheckResult*)pParam)->lEndthread == 0)
                ((CCheckResult*)pParam)->m_ListCheck.SetItemText(ListCount, 2, L"NG");
            if (((CCheckResult*)pParam)->lEndthread == 0)
                ((CCheckResult*)pParam)->m_ListCheck.SetItemColor(ListCount, RGB(255, 0, 0));
        }
        if (((CCheckResult*)pParam)->lEndthread == 0)
        ListCount++;
        i++;
    }
    return 0;//�w����^����� 
    //ExitThread(CheckEndDlgcode);
}
//�ƹ������U
void CCheckResult::OnNMDblclkList1(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
    NM_LISTVIEW  *pEditCtrl = (NM_LISTVIEW *)pNMHDR;
    if (pEditCtrl->iItem != -1 || pEditCtrl->iSubItem != 0) {
        if (m_ListCheck.GetItemText(pEditCtrl->iItem, 1) == L"AreaCheck")
        {
#ifdef PRINTF
            _cwprintf(L"%s\n", pCCommandTestDlg->a.AreaCheckFinishRecord.at(pEditCtrl->iItem - pCCommandTestDlg->a.CheckFinishRecord.size()).ResultImage.Path +
#endif
                pCCommandTestDlg->a.AreaCheckFinishRecord.at(pEditCtrl->iItem - pCCommandTestDlg->a.CheckFinishRecord.size()).ResultImage.Name);
            if (m_pPictureViewDlg == NULL)
            {
                //�q�X�Ϥ�:�覡��(�I�sWindowns�w�]�{��)
                /*
                ShellExecute(NULL, L"open", _T("rundll32.exe"), _T("shimgvw.dll,ImageView_Fullscreen ") + pCCommandTestDlg->a.AreaCheckFinishRecord.at(pEditCtrl->iItem - pCCommandTestDlg->a.CheckFinishRecord.size()).ResultImage.Path +
                    pCCommandTestDlg->a.AreaCheckFinishRecord.at(pEditCtrl->iItem - pCCommandTestDlg->a.CheckFinishRecord.size()).ResultImage.Name, NULL, SW_SHOW);
                */
                //�q�X�Ϥ�:�覡�G(�ϥ�MIL���n���\��)
                m_pPictureViewDlg = new CPictureViewDlg();
                ((CPictureViewDlg*)m_pPictureViewDlg)->FilePath = pCCommandTestDlg->a.AreaCheckFinishRecord.at(pEditCtrl->iItem - pCCommandTestDlg->a.CheckFinishRecord.size()).ResultImage.Path;
                ((CPictureViewDlg*)m_pPictureViewDlg)->FileName = pCCommandTestDlg->a.AreaCheckFinishRecord.at(pEditCtrl->iItem - pCCommandTestDlg->a.CheckFinishRecord.size()).ResultImage.Name;
                m_pPictureViewDlg->Create(IDD_DIALOG13, this); 
                m_pPictureViewDlg->ShowWindow(SW_SHOW);
            }
            else
            {
                //�q�X�Ϥ�:�覡�G(�ϥ�MIL���n���\��)
                if (::IsWindow(((CPictureViewDlg*)m_pPictureViewDlg)->m_hWnd))//�P�_�����O�_���P��
                {
                    ((CPictureViewDlg*)m_pPictureViewDlg)->OnCancel();
                }
                if (m_pPictureViewDlg != NULL)
                {
                    delete (CDialog*)m_pPictureViewDlg;
                    m_pPictureViewDlg = NULL;
                }
                m_pPictureViewDlg = new CPictureViewDlg();
                ((CPictureViewDlg*)m_pPictureViewDlg)->FilePath = pCCommandTestDlg->a.AreaCheckFinishRecord.at(pEditCtrl->iItem - pCCommandTestDlg->a.CheckFinishRecord.size()).ResultImage.Path;
                ((CPictureViewDlg*)m_pPictureViewDlg)->FileName = pCCommandTestDlg->a.AreaCheckFinishRecord.at(pEditCtrl->iItem - pCCommandTestDlg->a.CheckFinishRecord.size()).ResultImage.Name;
                m_pPictureViewDlg->Create(IDD_DIALOG13, this);
                m_pPictureViewDlg->ShowWindow(SW_SHOW);
            }
        }
    }
    *pResult = 0;
}
//��������
void CCheckResult::OnClose()
{
    _cwprintf(L"OnClose\n");
    InterlockedIncrement(&lEndthread);
    NoPushEsc = FALSE;
    CDialogEx::OnClose();
}
//��������
void CCheckResult::OnCancel()
{
    _cwprintf(L"OnCancel\n");
    if(!NoPushEsc)
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
//OK �M Enter��
void CCheckResult::OnOK()
{
    //CDialogEx::OnOK();
}
