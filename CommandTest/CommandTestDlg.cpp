
// CommandTestDlg.cpp : ��@��
//

#include "stdafx.h"
#include "CommandTest.h"
#include "CommandTestDlg.h"
#include "afxdialogex.h"
#include "mcc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CCommandTestDlg ��ܤ��



CCommandTestDlg::CCommandTestDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_COMMANDTEST_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CCommandTestDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST1, m_CommandList);
}

BEGIN_MESSAGE_MAP(CCommandTestDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDC_START, &CCommandTestDlg::OnBnClickedStart)
    ON_WM_TIMER()
    ON_BN_CLICKED(IDC_PAUSE, &CCommandTestDlg::OnBnClickedPause)
    ON_BN_CLICKED(IDC_STOP, &CCommandTestDlg::OnBnClickedStop)
    ON_BN_CLICKED(IDOK, &CCommandTestDlg::OnBnClickedOk)
    ON_BN_CLICKED(IDC_BTNCOMMAND27, &CCommandTestDlg::OnBnClickedBtncommand27)
    ON_BN_CLICKED(IDC_BTNCOMMAND1, &CCommandTestDlg::OnBnClickedBtncommand1)
    ON_BN_CLICKED(IDC_BTNCOMMAND2, &CCommandTestDlg::OnBnClickedBtncommand2)
    ON_BN_CLICKED(IDC_BTNHOME, &CCommandTestDlg::OnBnClickedBtnhome)
    ON_BN_CLICKED(IDC_BTNCOMMAND3, &CCommandTestDlg::OnBnClickedBtncommand3)
    ON_BN_CLICKED(IDC_BTNCOMMAND4, &CCommandTestDlg::OnBnClickedBtncommand4)
    ON_BN_CLICKED(IDC_BTNCOMMAND5, &CCommandTestDlg::OnBnClickedBtncommand5)
    ON_BN_CLICKED(IDC_BTNCOMMAND6, &CCommandTestDlg::OnBnClickedBtncommand6)
    ON_BN_CLICKED(IDC_BTNCOMMAND7, &CCommandTestDlg::OnBnClickedBtncommand7)
    ON_BN_CLICKED(IDC_BTNCOMMAND8, &CCommandTestDlg::OnBnClickedBtncommand8)
    ON_BN_CLICKED(IDC_BTNCOMMAND9, &CCommandTestDlg::OnBnClickedBtncommand9)
    ON_BN_CLICKED(IDC_BTNCOMMAND10, &CCommandTestDlg::OnBnClickedBtncommand10)
    ON_BN_CLICKED(IDC_BTNCOMMAND11, &CCommandTestDlg::OnBnClickedBtncommand11)
    ON_BN_CLICKED(IDC_BTNCOMMAND12, &CCommandTestDlg::OnBnClickedBtncommand12)
    ON_BN_CLICKED(IDC_BTNCOMMAND13, &CCommandTestDlg::OnBnClickedBtncommand13)
    ON_BN_CLICKED(IDC_BTNCOMMAND14, &CCommandTestDlg::OnBnClickedBtncommand14)
    ON_BN_CLICKED(IDC_BTNCOMMAND15, &CCommandTestDlg::OnBnClickedBtncommand15)
    ON_BN_CLICKED(IDC_BTNCOMMAND16, &CCommandTestDlg::OnBnClickedBtncommand16)
    ON_BN_CLICKED(IDC_BTNCOMMAND17, &CCommandTestDlg::OnBnClickedBtncommand17)
    ON_BN_CLICKED(IDC_BTNCOMMAND18, &CCommandTestDlg::OnBnClickedBtncommand18)
    ON_BN_CLICKED(IDC_BTNCOMMAND19, &CCommandTestDlg::OnBnClickedBtncommand19)
    ON_BN_CLICKED(IDC_BTNCOMMAND20, &CCommandTestDlg::OnBnClickedBtncommand20)
    ON_BN_CLICKED(IDC_BTNCOMMAND21, &CCommandTestDlg::OnBnClickedBtncommand21)
    ON_BN_CLICKED(IDC_BTNCOMMAND22, &CCommandTestDlg::OnBnClickedBtncommand22)
    ON_BN_CLICKED(IDC_BTNCOMMAND23, &CCommandTestDlg::OnBnClickedBtncommand23)
    ON_BN_CLICKED(IDC_BTNCOMMAND24, &CCommandTestDlg::OnBnClickedBtncommand24)
    ON_BN_CLICKED(IDC_BTNCOMMAND25, &CCommandTestDlg::OnBnClickedBtncommand25)
    ON_BN_CLICKED(IDC_BTNCOMMAND26, &CCommandTestDlg::OnBnClickedBtncommand26)
    ON_BN_CLICKED(IDC_BTNCOMMAND28, &CCommandTestDlg::OnBnClickedBtncommand28)
    ON_BN_CLICKED(IDC_BTNCOMMAND29, &CCommandTestDlg::OnBnClickedBtncommand29)
    ON_BN_CLICKED(IDC_BTNCOMMAND30, &CCommandTestDlg::OnBnClickedBtncommand30)
    ON_BN_CLICKED(IDC_BTNCOMMAND31, &CCommandTestDlg::OnBnClickedBtncommand31)
    ON_BN_CLICKED(IDC_BTNCOMMAND32, &CCommandTestDlg::OnBnClickedBtncommand32)
    ON_BN_CLICKED(IDC_BTNCOMMAND33, &CCommandTestDlg::OnBnClickedBtncommand33)
    ON_BN_CLICKED(IDC_BTNCOMMAND34, &CCommandTestDlg::OnBnClickedBtncommand34)
    ON_BN_CLICKED(IDC_BTNCOMMAND35, &CCommandTestDlg::OnBnClickedBtncommand35)
    ON_BN_CLICKED(IDC_BTNCOMMAND36, &CCommandTestDlg::OnBnClickedBtncommand36)
    ON_BN_CLICKED(IDC_BTNCOMMAND37, &CCommandTestDlg::OnBnClickedBtncommand37)
    ON_BN_CLICKED(IDC_BTNCOMMAND38, &CCommandTestDlg::OnBnClickedBtncommand38)
    ON_BN_CLICKED(IDC_BTNCOMMAND39, &CCommandTestDlg::OnBnClickedBtncommand39)
    ON_BN_CLICKED(IDC_BTNCOMMAND40, &CCommandTestDlg::OnBnClickedBtncommand40)
    ON_BN_CLICKED(IDC_BTNCOMMAND41, &CCommandTestDlg::OnBnClickedBtncommand41)
END_MESSAGE_MAP()


// CCommandTestDlg �T���B�z�`��
BOOL CCommandTestDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// �]�w����ܤ�����ϥܡC�����ε{�����D�������O��ܤ���ɡA
	// �ج[�|�۰ʱq�Ʀ��@�~
	SetIcon(m_hIcon, TRUE);			// �]�w�j�ϥ�
	SetIcon(m_hIcon, FALSE);		// �]�w�p�ϥ�
	// TODO: �b���[�J�B�~����l�]�w
    m_CommandList.InsertColumn(0, _T("�s��"), LVCFMT_CENTER, 36, -1);
    m_CommandList.InsertColumn(1, _T("Command"), LVCFMT_CENTER, 250, -1);
    SetTimer(1, 100, NULL);
	return TRUE;  // �Ǧ^ TRUE�A���D�z�ﱱ��]�w�J�I
}
// �p�G�N�̤p�ƫ��s�[�J�z����ܤ���A�z�ݭn�U�C���{���X�A
// �H�Kø�s�ϥܡC���ϥΤ��/�˵��Ҧ��� MFC ���ε{���A
// �ج[�|�۰ʧ������@�~�C
void CCommandTestDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ø�s���˸m���e

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// �N�ϥܸm����Τ�ݯx��
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// �yø�ϥ�
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}
// ��ϥΪ̩즲�̤p�Ƶ����ɡA
// �t�ΩI�s�o�ӥ\����o�����ܡC
HCURSOR CCommandTestDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CCommandTestDlg::OnBnClickedStart()
{
    a.Run();
}
void CCommandTestDlg::OnBnClickedPause()
{
    CString StrBuff;
    GetDlgItemText(IDC_PAUSE, StrBuff);
    if (StrBuff == L"Pause")
    {
        a.Pause();
        SetDlgItemText(IDC_PAUSE, L"Continue");
    }
    else
    {
        a.Continue();
        SetDlgItemText(IDC_PAUSE, L"Pause");
    }
}
void CCommandTestDlg::OnBnClickedStop()
{
    a.Stop();
    SetDlgItemText(IDC_PAUSE, L"Pause");
}
/*�M���}�C*/
void CCommandTestDlg::OnBnClickedOk()
{
    a.Command.clear();
    ListRefresh(NULL);
}
/*���I���k*/
void CCommandTestDlg::OnBnClickedBtnhome()
{
#ifdef MOVE
    MO_Open(1);
    MO_SetHardLim(7, 1);
    MO_SetDecOK(1);//�}�Ҵ�t���� 
    MO_MoveToHome(20000, 1000, 7, 0);//���I�_�k
#endif
}
void CCommandTestDlg::OnTimer(UINT_PTR nIDEvent)
{
    CString StrBuff;
    StrBuff.Format(_T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"), 
        a.Time, a.RunCount,
        a.DispenseDotEnd.RiseDistance,a.DispenseDotEnd.RiseHightSpeed,a.DispenseDotEnd.RiseLowSpeed,
        a.DispenseDotSet.GlueCloseTime,a.DispenseDotSet.GlueOpenTime,
        a.DotSpeedSet.AccSpeed,a.DotSpeedSet.EndSpeed,
        a.Command.size());
    StrBuff = StrBuff + a.Program.LabelName;
    SetDlgItemText(IDC_EDIT1, StrBuff);
    CDialogEx::OnTimer(nIDEvent);
}
void CCommandTestDlg::ListRefresh(BOOL ScrollBarRefresh) {
    CString StrBuff;
    m_CommandList.DeleteAllItems();
    int nCount = a.Command.size();
    for (int i = 0; i < nCount; i++) {
        m_CommandList.InsertItem(i, NULL);
        (i>9) ? StrBuff.Format(_T("0%d"), i+1) : StrBuff.Format(_T("00%d"), i+1);
        m_CommandList.SetItemText(i, 0, StrBuff);
        m_CommandList.SetItemText(i, 1, a.Command.at(i));
    }
}
/************************************************************�R�O*/
/*���I�I��*/
void CCommandTestDlg::OnBnClickedBtncommand1()
{
    a.Command.push_back(_T("DispenseDot,10000,10000,50000"));
    ListRefresh(NULL);
}
/*���I�I���]�m*/
void CCommandTestDlg::OnBnClickedBtncommand2()
{
    a.Command.push_back(_T("DispenseDotSet,1000,1000"));
    ListRefresh(NULL);
}
/*���I�����]�m*/
void CCommandTestDlg::OnBnClickedBtncommand3()
{
    a.Command.push_back(_T("DispenseDotEnd,1000,100,10000"));
    ListRefresh(NULL);
}
/*PTP�X�ʳt��*/
void CCommandTestDlg::OnBnClickedBtncommand4()
{
    a.Command.push_back(_T("DotSpeedSet,30000,100000"));
    ListRefresh(NULL);
}
/*�u�q�}�l*/
void CCommandTestDlg::OnBnClickedBtncommand5()
{
    a.Command.push_back(_T("Line Start,20000,20000,600000"));
    ListRefresh(NULL);
}
/*�u�q�����I*/
void CCommandTestDlg::OnBnClickedBtncommand6()
{
    a.Command.push_back(_T("Line Passing,20000,30000,600000"));
    ListRefresh(NULL);
}
/*�u�q�����I*/
void CCommandTestDlg::OnBnClickedBtncommand7()
{
    a.Command.push_back(_T("Line End,30000,20000,600000"));
    ListRefresh(NULL);
}
/*�꩷���I*/
void CCommandTestDlg::OnBnClickedBtncommand8()
{
    // TODO: �b���[�J����i���B�z�`���{���X
}
/*�ꤤ�I*/
void CCommandTestDlg::OnBnClickedBtncommand9()
{
    // TODO: �b���[�J����i���B�z�`���{���X
}
/*�ꤤ�I2*/
void CCommandTestDlg::OnBnClickedBtncommand10()
{
    // TODO: �b���[�J����i���B�z�`���{���X
}
/*�u�q��]�m*/
void CCommandTestDlg::OnBnClickedBtncommand11()
{
    // TODO: �b���[�J����i���B�z�`���{���X
}
/*�u�q�t�׳]�m*/
void CCommandTestDlg::OnBnClickedBtncommand12()
{
    // TODO: �b���[�J����i���B�z�`���{���X
}
/*������]�m*/
void CCommandTestDlg::OnBnClickedBtncommand13()
{
    // TODO: �b���[�J����i���B�z�`���{���X
}
/*Z�b�u�@����*/
void CCommandTestDlg::OnBnClickedBtncommand14()
{
    // TODO: �b���[�J����i���B�z�`���{���X
}
/*�[�t�׳]�m*/
void CCommandTestDlg::OnBnClickedBtncommand15()
{
    // TODO: �b���[�J����i���B�z�`���{���X
}
/*�Ѽƪ�l��*/
void CCommandTestDlg::OnBnClickedBtncommand16()
{
    // TODO: �b���[�J����i���B�z�`���{���X
}
/*��J*/
void CCommandTestDlg::OnBnClickedBtncommand17()
{
    // TODO: �b���[�J����i���B�z�`���{���X
}
/*��X*/
void CCommandTestDlg::OnBnClickedBtncommand18()
{
    // TODO: �b���[�J����i���B�z�`���{���X
}
/*�I�����}��*/
void CCommandTestDlg::OnBnClickedBtncommand19()
{
    // TODO: �b���[�J����i���B�z�`���{���X
}
/*�����I*/
void CCommandTestDlg::OnBnClickedBtncommand20()
{
    // TODO: �b���[�J����i���B�z�`���{���X
}
/*�����I*/
void CCommandTestDlg::OnBnClickedBtncommand21()
{
    // TODO: �b���[�J����i���B�z�`���{���X
}
/*���n�I*/
void CCommandTestDlg::OnBnClickedBtncommand22()
{
    // TODO: �b���[�J����i���B�z�`���{���X
}
/*�����I*/
void CCommandTestDlg::OnBnClickedBtncommand23()
{
    // TODO: �b���[�J����i���B�z�`���{���X
}
/*��R�ϰ�*/
void CCommandTestDlg::OnBnClickedBtncommand24()
{
    // TODO: �b���[�J����i���B�z�`���{���X
}
/*�^���I�R�O*/
void CCommandTestDlg::OnBnClickedBtncommand25()
{
    // TODO: �b���[�J����i���B�z�`���{���X
}
/*����*/           
void CCommandTestDlg::OnBnClickedBtncommand26()
{
    a.Command.push_back(_T("Label,100"));
    ListRefresh(NULL);
}
/*�e���a�}*/
void CCommandTestDlg::OnBnClickedBtncommand27()
{
    a.Command.push_back(_T("GotoAddress,3"));
    ListRefresh(NULL);
}
/*�e������*/
void CCommandTestDlg::OnBnClickedBtncommand28()
{
    a.Command.push_back(_T("GotoLabel,100"));
    ListRefresh(NULL);
}
/*�եΤl�{��*/
void CCommandTestDlg::OnBnClickedBtncommand29()
{
    // TODO: �b���[�J����i���B�z�`���{���X
}
/*�����l�{��*/
void CCommandTestDlg::OnBnClickedBtncommand30()
{
    // TODO: �b���[�J����i���B�z�`���{���X
}
/*�`���a�}*/
void CCommandTestDlg::OnBnClickedBtncommand31()
{
    // TODO: �b���[�J����i���B�z�`���{���X
}
/*�B�J����X*/
void CCommandTestDlg::OnBnClickedBtncommand32()
{
    // TODO: �b���[�J����i���B�z�`���{���X
}
/*�B�J����Y*/
void CCommandTestDlg::OnBnClickedBtncommand33()
{
    // TODO: �b���[�J����i���B�z�`���{���X
}
/*�d��аO*/
void CCommandTestDlg::OnBnClickedBtncommand34()
{
    // TODO: �b���[�J����i���B�z�`���{���X
}
/*��ǼаO*/
void CCommandTestDlg::OnBnClickedBtncommand35()
{
    // TODO: �b���[�J����i���B�z�`���{���X
}
/*�d��аO�վ�*/
void CCommandTestDlg::OnBnClickedBtncommand36()
{
    // TODO: �b���[�J����i���B�z�`���{���X
}
/*��ǼаO�վ�*/
void CCommandTestDlg::OnBnClickedBtncommand37()
{
    // TODO: �b���[�J����i���B�z�`���{���X
}
/*�d�䨤�׼аO*/
void CCommandTestDlg::OnBnClickedBtncommand38()
{
    // TODO: �b���[�J����i���B�z�`���{���X
}
/*����*/
void CCommandTestDlg::OnBnClickedBtncommand39()
{
    // TODO: �b���[�J����i���B�z�`���{���X
}
/*CCDĲ�o��*/
void CCommandTestDlg::OnBnClickedBtncommand40()
{
    // TODO: �b���[�J����i���B�z�`���{���X
}
/*�X�i�B�J����*/
void CCommandTestDlg::OnBnClickedBtncommand41()
{
    // TODO: �b���[�J����i���B�z�`���{���X
}
