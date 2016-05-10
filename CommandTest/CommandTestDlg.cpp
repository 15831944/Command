
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
    ON_BN_CLICKED(IDC_BTNHOME, &CCommandTestDlg::OnBnClickedBtnhome)
    ON_BN_CLICKED(IDC_BTNVIEW, &CCommandTestDlg::OnBnClickedBtnview)
    ON_BN_CLICKED(IDOK, &CCommandTestDlg::OnBnClickedOk)
    ON_COMMAND(IDM_INSERT, &CCommandTestDlg::OnInsert)
    ON_COMMAND(IDM_DELETE, &CCommandTestDlg::OnDelete)
    ON_NOTIFY(NM_RCLICK, IDC_LIST1, &CCommandTestDlg::OnNMRClickList1)
       
    ON_BN_CLICKED(IDC_BTNCOMMAND1, &CCommandTestDlg::OnBnClickedBtncommand1)
    ON_BN_CLICKED(IDC_BTNCOMMAND2, &CCommandTestDlg::OnBnClickedBtncommand2)
    ON_BN_CLICKED(IDC_BTNCOMMAND3, &CCommandTestDlg::OnBnClickedBtncommand3)
    ON_BN_CLICKED(IDC_BTNCOMMAND4, &CCommandTestDlg::OnBnClickedBtncommand4)
    ON_BN_CLICKED(IDC_BTNCOMMAND4_2, &CCommandTestDlg::OnBnClickedBtncommand4_2)
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
    ON_BN_CLICKED(IDC_BTNCOMMAND19_2, &CCommandTestDlg::OnBnClickedBtncommand19_2)
    ON_BN_CLICKED(IDC_BTNCOMMAND20, &CCommandTestDlg::OnBnClickedBtncommand20)
    ON_BN_CLICKED(IDC_BTNCOMMAND21, &CCommandTestDlg::OnBnClickedBtncommand21)
    ON_BN_CLICKED(IDC_BTNCOMMAND22, &CCommandTestDlg::OnBnClickedBtncommand22)
    ON_BN_CLICKED(IDC_BTNCOMMAND23, &CCommandTestDlg::OnBnClickedBtncommand23)
    ON_BN_CLICKED(IDC_BTNCOMMAND24, &CCommandTestDlg::OnBnClickedBtncommand24)
    ON_BN_CLICKED(IDC_BTNCOMMAND25, &CCommandTestDlg::OnBnClickedBtncommand25)
    ON_BN_CLICKED(IDC_BTNCOMMAND26, &CCommandTestDlg::OnBnClickedBtncommand26)
    ON_BN_CLICKED(IDC_BTNCOMMAND27, &CCommandTestDlg::OnBnClickedBtncommand27)
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


    ON_BN_CLICKED(IDC_BTNCOMMAND42, &CCommandTestDlg::OnBnClickedBtncommand42)
    ON_BN_CLICKED(IDC_BTNCOMMAND43, &CCommandTestDlg::OnBnClickedBtncommand43)
    ON_BN_CLICKED(IDC_BTNCOMMAND44, &CCommandTestDlg::OnBnClickedBtncommand44)
    ON_BN_CLICKED(IDC_BTNCOMMAND45, &CCommandTestDlg::OnBnClickedBtncommand45)
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
    m_CommandList.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
    m_CommandList.InsertColumn(0, _T("�s��"), LVCFMT_CENTER, 36, -1);
    m_CommandList.InsertColumn(1, _T("Command"), LVCFMT_LEFT, 300, -1);
    SetTimer(1, 100, NULL);
#ifdef MOVE
    MO_Open(1);
    MO_SetHardLim(7, 1);
    MO_SetDecOK(1);//�}�Ҵ�t����
#endif
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
    MO_MoveToHome(20000, 1000, 7, 0);//���I�_�k
#endif
}
void CCommandTestDlg::OnTimer(UINT_PTR nIDEvent)
{
    CString DotStrBuff, LinStrBuff, RunStrBuff , SubroutineBuff;
    CString XYZlocation;
    DotStrBuff.Format(_T("�Ĥ@���q��ɶZ��:%d,���t:%d,�C�t:%d\r\t\t�����ɶ�:%d,�}�Үɶ�:%d,�[�t:%d,�X��:%d\r\n"),
        a.DispenseDotEnd.RiseDistance,a.DispenseDotEnd.RiseHightSpeed,a.DispenseDotEnd.RiseLowSpeed,
        a.DispenseDotSet.GlueCloseTime,a.DispenseDotSet.GlueOpenTime,
        a.DotSpeedSet.AccSpeed,a.DotSpeedSet.EndSpeed
    );
    /*�B�ʪ��A:%d,�u�q�}�l���A:%d,�꩷���A:%d,�ꪬ�A:%d\r\t*/
    LinStrBuff.Format(_T("�e����:%d,�e�Z��:%d,�`�I:%d,���d:%d,�᩵��:%d��Z��:%d\r\n����:%d,���t:%d,�C�t:%d,����:%d,����:%d\r\t\t�[�t:%d,�X��:%d\r\n"),
        //a.RunData.ActionStatus, a.StartData.at(0).Status, a.ArcData.at(0).Status,a.CircleData1.at(0).Status,
        a.DispenseLineSet.BeforeMoveDelay, a.DispenseLineSet.BeforeMoveDistance, a.DispenseLineSet.NodeTime, a.DispenseLineSet.StayTime, a.DispenseLineSet.ShutdownDelay, a.DispenseLineSet.ShutdownDistance,
        a.DispenseLineEnd.Type, a.DispenseLineEnd.HighSpeed, a.DispenseLineEnd.LowSpeed, a.DispenseLineEnd.Width, a.DispenseLineEnd.Height,
        a.LineSpeedSet.AccSpeed, a.LineSpeedSet.EndSpeed
    );
    SubroutineBuff.Format(_T("�l�{�ǭp��:%d,�B�ʪ��A�ƶq:%d,�}�l���A�ƶq:%d�꩷���A�ƶq:%d,�ꪬ�A�ƶq:%d,offset�ƶq:%d,�l�{�Ǧ�m�ƶq:%d,������u�ƶq:%d"),
        a.Program.SubroutinCount, a.RunData.ActionStatus.size(), a.StartData.size(), a.ArcData.size(), a.CircleData1.size(), a.OffsetData.size(), a.Program.SubroutineStack.size(), a.Program.SubroutinePointStack.size());
    RunStrBuff = DotStrBuff + LinStrBuff + SubroutineBuff;//�˴����ҥ� 
    SetDlgItemText(IDC_EDIT1, RunStrBuff);
    if (a.RunData.RunStatus == 2)
    {
        SetDlgItemText(IDC_PAUSE, L"Continue");
    }
#ifdef MOVE
    XYZlocation.Format(_T("X:%d,Y:%d,Z:%d,GlueStatus:%d"), MO_ReadLogicPosition(0), MO_ReadLogicPosition(1), MO_ReadLogicPosition(2), MO_ReadGumming());
#endif 
    SetDlgItemText(IDC_EDIT2, XYZlocation);
    CDialogEx::OnTimer(nIDEvent);
}
void CCommandTestDlg::ListRefresh(BOOL ScrollBarRefresh) {
    CString StrBuff;
    m_CommandList.DeleteAllItems();
    int nCount = a.CommandMemory.size();
    if (!ScrollBarRefresh)
    {
        for (int i = 0; i < nCount; i++) {
            m_CommandList.InsertItem(i, NULL);
            (i>8) ? StrBuff.Format(_T("0%d"), i + 1) : StrBuff.Format(_T("00%d"), i + 1);
            m_CommandList.SetItemText(i, 0, StrBuff);
            m_CommandList.SetItemText(i, 1, a.CommandMemory.at(i));
        }
    }
    else
    {
        for (UINT i = 0; i < a.Command.size(); i++)
        {
            for (UINT j = 0; j<a.Command.at(i).size(); j++)
            {
                m_CommandList.InsertItem(j, NULL);
                (j>8) ? StrBuff.Format(_T("0%d"), j + 1) : StrBuff.Format(_T("00%d"), j + 1);
                m_CommandList.SetItemText(j, 0, StrBuff);
                m_CommandList.SetItemText(j, 1, a.Command.at(i).at(j));
            }
        }
        m_CommandList.InsertItem(0, NULL);
        StrBuff.Format(_T("%d"), a.Command.at(0).size());
        m_CommandList.SetItemText(0, 1, StrBuff);
        m_CommandList.InsertItem(1, NULL);
        StrBuff.Format(_T("%d"), a.Command.at(1).size());
        m_CommandList.SetItemText(1, 1, StrBuff);

    }
    
}
/*�C���I�U����*/
void CCommandTestDlg::OnNMRClickList1(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
    CListCtrl *CList = (CListCtrl *)GetDlgItem(IDC_LIST1);
    CMenu menu, *pSubMenu;
    CPoint CurPnt;
    int ItemCount = m_CommandList.GetItemCount();//��������`��
    NM_LISTVIEW  *pEditCtrl = (NM_LISTVIEW *)pNMHDR;
    if (pEditCtrl->iItem != -1 || pEditCtrl->iSubItem != 0) {
        menu.LoadMenu(IDR_MENU1);//�[�J���
        pSubMenu = menu.GetSubMenu(0);
        GetCursorPos(&CurPnt);
        pSubMenu->TrackPopupMenu(TPM_LEFTALIGN, CurPnt.x, CurPnt.y, this);//�I�k��X�{������m
    }
    *pResult = 0;
}
/*���J*/
void CCommandTestDlg::OnInsert()
{
    if (!Insert)
    {
        int istat = m_CommandList.GetSelectionMark();//�����ܪ���
        m_CommandList.InsertItem(istat, NULL);
        Insert = TRUE;
        InsertNum = istat;
    }  
}
/*�R��*/
void CCommandTestDlg::OnDelete()
{
    if (!Insert)
    {
        int istat = m_CommandList.GetSelectionMark();//�����ܪ���
        a.CommandMemory.erase(a.CommandMemory.begin() + istat);
        ListRefresh(NULL);
    }
}
/************************************************************�R�O*/
/*���I�I��*/
void CCommandTestDlg::OnBnClickedBtncommand1()
{
    StrBuff.Format(_T("Dot,%d,%d,%d"),GetDlgItemInt(IDC_EDITPARAM1), GetDlgItemInt(IDC_EDITPARAM2), GetDlgItemInt(IDC_EDITPARAM3));
    (Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum,StrBuff) : a.CommandMemory.push_back(StrBuff);
    Insert = FALSE;
    ListRefresh(NULL);
}
/*���I�I���]�m*/
void CCommandTestDlg::OnBnClickedBtncommand2()
{
    StrBuff.Format(_T("DispenseDotSet,%d,%d"), GetDlgItemInt(IDC_EDITPARAM1), GetDlgItemInt(IDC_EDITPARAM2));
    (Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
    Insert = FALSE;
    ListRefresh(NULL);
}
/*���I�����]�m*/
void CCommandTestDlg::OnBnClickedBtncommand3()
{
    StrBuff.Format(_T("DispenseDotEnd,%d,%d,%d"), GetDlgItemInt(IDC_EDITPARAM1), GetDlgItemInt(IDC_EDITPARAM2), GetDlgItemInt(IDC_EDITPARAM3));
    (Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
    Insert = FALSE;
    ListRefresh(NULL);
}
/*PTP�X�ʳt��*/
void CCommandTestDlg::OnBnClickedBtncommand4()
{
    StrBuff.Format(_T("DotSpeedSet,%d"), GetDlgItemInt(IDC_EDITPARAM1));
    (Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
    Insert = FALSE;
    ListRefresh(NULL);
}
/*PTP�[�t�׳]�m(%)*/
void CCommandTestDlg::OnBnClickedBtncommand4_2()
{
    StrBuff.Format(_T("DotAccPercent,%d"), GetDlgItemInt(IDC_EDITPARAM1));
    (Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
    Insert = FALSE;
    ListRefresh(NULL);
}
/*�u�q�}�l*/
void CCommandTestDlg::OnBnClickedBtncommand5()
{
    StrBuff.Format(_T("LineStart,%d,%d,%d"), GetDlgItemInt(IDC_EDITPARAM1), GetDlgItemInt(IDC_EDITPARAM2), GetDlgItemInt(IDC_EDITPARAM3));
    (Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
    Insert = FALSE;
    ListRefresh(NULL);
}
/*�u�q�����I*/
void CCommandTestDlg::OnBnClickedBtncommand6()
{
    StrBuff.Format(_T("LinePassing,%d,%d,%d"), GetDlgItemInt(IDC_EDITPARAM1), GetDlgItemInt(IDC_EDITPARAM2), GetDlgItemInt(IDC_EDITPARAM3));
    (Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
    Insert = FALSE;
    ListRefresh(NULL);
}
/*�u�q�����I*/
void CCommandTestDlg::OnBnClickedBtncommand7()
{
    StrBuff.Format(_T("LineEnd,%d,%d,%d"), GetDlgItemInt(IDC_EDITPARAM1), GetDlgItemInt(IDC_EDITPARAM2), GetDlgItemInt(IDC_EDITPARAM3));
    (Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
    Insert = FALSE;
    ListRefresh(NULL);
}
/*�꩷���I*/
void CCommandTestDlg::OnBnClickedBtncommand8()
{
    StrBuff.Format(_T("ArcPoint,%d,%d,%d"), GetDlgItemInt(IDC_EDITPARAM1), GetDlgItemInt(IDC_EDITPARAM2), GetDlgItemInt(IDC_EDITPARAM3));
    (Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
    Insert = FALSE;
    ListRefresh(NULL);
}
/*�ꤤ�I*/
void CCommandTestDlg::OnBnClickedBtncommand9()
{
    StrBuff.Format(_T("CirclePoint,%d,%d,%d,%d,%d,%d"), GetDlgItemInt(IDC_EDITPARAM1), GetDlgItemInt(IDC_EDITPARAM2), GetDlgItemInt(IDC_EDITPARAM3), GetDlgItemInt(IDC_EDITPARAM4), GetDlgItemInt(IDC_EDITPARAM5), GetDlgItemInt(IDC_EDITPARAM6));
    (Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
    Insert = FALSE;
    ListRefresh(NULL);
}
/*�ꤤ�I2*/
void CCommandTestDlg::OnBnClickedBtncommand10()
{
    /*StrBuff.Format(_T("CirclePointTwo,%d,%d,%d"), GetDlgItemInt(IDC_EDITPARAM1), GetDlgItemInt(IDC_EDITPARAM2), GetDlgItemInt(IDC_EDITPARAM3));
    (Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
    Insert = FALSE;
    ListRefresh(NULL);*/
}
/*�u�q��]�m*/
void CCommandTestDlg::OnBnClickedBtncommand11()
{
    StrBuff.Format(_T("DispenseLineSet,%d,%d,%d,%d,%d,%d"), GetDlgItemInt(IDC_EDITPARAM1), GetDlgItemInt(IDC_EDITPARAM2), GetDlgItemInt(IDC_EDITPARAM3),
        GetDlgItemInt(IDC_EDITPARAM4), GetDlgItemInt(IDC_EDITPARAM5), GetDlgItemInt(IDC_EDITPARAM6));
    (Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
    Insert = FALSE;
    ListRefresh(NULL);
}
/*�u�q�����]�m*/
void CCommandTestDlg::OnBnClickedBtncommand12()
{
    StrBuff.Format(_T("DispenseLineEnd,%d,%d,%d,%d,%d"), GetDlgItemInt(IDC_EDITPARAM1), GetDlgItemInt(IDC_EDITPARAM2), GetDlgItemInt(IDC_EDITPARAM3),
        GetDlgItemInt(IDC_EDITPARAM4), GetDlgItemInt(IDC_EDITPARAM5));
    (Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
    Insert = FALSE;
    ListRefresh(NULL);
}
/*��t�׳]�m*/
void CCommandTestDlg::OnBnClickedBtncommand13()
{
    StrBuff.Format(_T("LineSpeedSet,%d"), GetDlgItemInt(IDC_EDITPARAM1));
    (Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
    Insert = FALSE;
    ListRefresh(NULL);
}
/*Z�b�u�@����*/
void CCommandTestDlg::OnBnClickedBtncommand14()
{
    StrBuff.Format(_T("ZGoBack,%d,%d"), GetDlgItemInt(IDC_EDITPARAM1), GetDlgItemInt(IDC_EDITPARAM2));
    (Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
    Insert = FALSE;
    ListRefresh(NULL);
}
/*�[�t�׳]�m*/
void CCommandTestDlg::OnBnClickedBtncommand15()
{
    StrBuff.Format(_T("DispenseAccSet,%d,%d"), GetDlgItemInt(IDC_EDITPARAM1), GetDlgItemInt(IDC_EDITPARAM2));
    (Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
    Insert = FALSE;
    ListRefresh(NULL);
}
/*�Ѽƪ�l��*/
void CCommandTestDlg::OnBnClickedBtncommand16()
{
    StrBuff = _T("Initialize");
    (Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
    Insert = FALSE;
    ListRefresh(NULL);
}
/*��J*/
void CCommandTestDlg::OnBnClickedBtncommand17()
{
    StrBuff.Format(_T("Input,%d,%d,%d"), GetDlgItemInt(IDC_EDITPARAM1), GetDlgItemInt(IDC_EDITPARAM2), GetDlgItemInt(IDC_EDITPARAM3));
    (Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
    Insert = FALSE;
    ListRefresh(NULL);
}
/*��X*/
void CCommandTestDlg::OnBnClickedBtncommand18()
{
    StrBuff.Format(_T("Output,%d,%d"), GetDlgItemInt(IDC_EDITPARAM1), GetDlgItemInt(IDC_EDITPARAM2));
    (Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
    Insert = FALSE;
    ListRefresh(NULL);
}
/*�I�����}��*/
void CCommandTestDlg::OnBnClickedBtncommand19()
{
    StrBuff.Format(_T("DispenserSwitch,%d"), GetDlgItemInt(IDC_EDITPARAM1));
    (Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
    Insert = FALSE;
    ListRefresh(NULL);
}
/*�]�m�I���ݤf*/
void CCommandTestDlg::OnBnClickedBtncommand19_2()
{
    StrBuff.Format(_T("DispenserSwitchSet,%d"), GetDlgItemInt(IDC_EDITPARAM1));
    (Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
    Insert = FALSE;
    ListRefresh(NULL);
}
/*�����I*/
void CCommandTestDlg::OnBnClickedBtncommand20()
{
    StrBuff.Format(_T("VirtualPoint,%d,%d,%d"), GetDlgItemInt(IDC_EDITPARAM1), GetDlgItemInt(IDC_EDITPARAM2), GetDlgItemInt(IDC_EDITPARAM3));
    (Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
    Insert = FALSE;
    ListRefresh(NULL);
}
/*�����I*/
void CCommandTestDlg::OnBnClickedBtncommand21()
{
    StrBuff.Format(_T("WaitPoint,%d,%d,%d,%d"), GetDlgItemInt(IDC_EDITPARAM1), GetDlgItemInt(IDC_EDITPARAM2), GetDlgItemInt(IDC_EDITPARAM3), GetDlgItemInt(IDC_EDITPARAM4));
    (Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
    Insert = FALSE;
    ListRefresh(NULL);
}
/*���n�I*/
void CCommandTestDlg::OnBnClickedBtncommand22()
{
    StrBuff.Format(_T("ParkPoint,%d,%d,%d"), GetDlgItemInt(IDC_EDITPARAM1), GetDlgItemInt(IDC_EDITPARAM2), GetDlgItemInt(IDC_EDITPARAM3));
    (Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
    Insert = FALSE;
    ListRefresh(NULL);
}
/*�����I*/
void CCommandTestDlg::OnBnClickedBtncommand23()
{
    StrBuff.Format(_T("StopPoint,%d,%d,%d"), GetDlgItemInt(IDC_EDITPARAM1), GetDlgItemInt(IDC_EDITPARAM2), GetDlgItemInt(IDC_EDITPARAM3));
    (Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
    Insert = FALSE;
    ListRefresh(NULL);
}
/*��R�ϰ�*/
void CCommandTestDlg::OnBnClickedBtncommand24()
{
    StrBuff.Format(_T("FillArea,%d,%d,%d,%d,%d,%d,50000,50000,60000"), GetDlgItemInt(IDC_EDITPARAM1), GetDlgItemInt(IDC_EDITPARAM2), GetDlgItemInt(IDC_EDITPARAM3), GetDlgItemInt(IDC_EDITPARAM4), GetDlgItemInt(IDC_EDITPARAM5), GetDlgItemInt(IDC_EDITPARAM6));
    (Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
    Insert = FALSE;
    ListRefresh(NULL);
}
/*�^���I�R�O*/
void CCommandTestDlg::OnBnClickedBtncommand25()
{
    StrBuff = _T("GoHome");
    (Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
    Insert = FALSE;
    ListRefresh(NULL);
}
/*����*/           
void CCommandTestDlg::OnBnClickedBtncommand26()
{
    StrBuff.Format(_T("Label,%d"), GetDlgItemInt(IDC_EDITPARAM1));
    (Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
    Insert = FALSE;
    ListRefresh(NULL);
}
/*�e���a�}*/
void CCommandTestDlg::OnBnClickedBtncommand27()
{
    StrBuff.Format(_T("GotoAddress,%d"),GetDlgItemInt(IDC_EDITPARAM1));
    (Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
    Insert = FALSE;
    ListRefresh(NULL);
}
/*�e������*/
void CCommandTestDlg::OnBnClickedBtncommand28()
{
    StrBuff.Format(_T("GotoLabel,%d"),GetDlgItemInt(IDC_EDITPARAM1));
    (Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
    Insert = FALSE;
    ListRefresh(NULL);
}
/*�եΤl�{��*/
void CCommandTestDlg::OnBnClickedBtncommand29()
{
    StrBuff.Format(_T("CallSubroutine,%d"), GetDlgItemInt(IDC_EDITPARAM1));
    (Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
    Insert = FALSE;
    ListRefresh(NULL);
}
/*�����l�{��*/
void CCommandTestDlg::OnBnClickedBtncommand30()
{
    StrBuff.Format(_T("SubroutineEnd"));
    (Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
    Insert = FALSE;
    ListRefresh(NULL);
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
/*�}�l�Ƶ{��*/
void CCommandTestDlg::OnBnClickedBtncommand43()
{
    StrBuff.Format(_T("SubProgramStart,%d"), GetDlgItemInt(IDC_EDITPARAM1));
    (Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
    Insert = FALSE;
    ListRefresh(NULL);
}
/*�����Ƶ{��*/
void CCommandTestDlg::OnBnClickedBtncommand44()
{
    StrBuff.Format(_T("SubProgramEnd"));
    (Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
    Insert = FALSE;
    ListRefresh(NULL);
}
/*�եΰƵ{��*/
void CCommandTestDlg::OnBnClickedBtncommand45()
{
    StrBuff.Format(_T("CallSubProgram,%d"), GetDlgItemInt(IDC_EDITPARAM1));
    (Insert) ? a.CommandMemory.emplace(a.CommandMemory.begin() + InsertNum, StrBuff) : a.CommandMemory.push_back(StrBuff);
    Insert = FALSE;
    ListRefresh(NULL);
}
/*******************************************************************************************�~���]�w**********************************************************/
/*�ƽ��]�m*/
void CCommandTestDlg::OnBnClickedBtncommand42()
{
    a.GlueData.GlueTime = GetDlgItemInt(IDC_EDITPARAM1);
    a.GlueData.GlueStayTime = GetDlgItemInt(IDC_EDITPARAM2);
}

/*����*/
void CCommandTestDlg::OnBnClickedBtnview()
{
}







