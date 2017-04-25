// Camera.cpp : 實作檔
//

#include "stdafx.h"
#include "CommandTest.h"
#include "Camera.h"
#include "afxdialogex.h"
#include "CommandTestDlg.h"
#define EPOCHFILETIME   (116444736000000000UL)

#define AbsoluteSpeedW 30000
#define AbsoluteSpeedA 90000
#define AbsoluteSpeedI 2000

#define AdjustSpeedW 40000
#define AdjustSpeedA 120000
#define AdjustSpeedI 2000 
// CCamera 對話方塊

IMPLEMENT_DYNAMIC(CCamera, CDialogEx)

CCamera::CCamera(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG2, pParent)
{
    m_pCModel = NULL; //初始化影像匹配頁面指針
	RaiChoose = 1;
	MilModel = malloc(sizeof(int));
    if(MilModel != NULL)
	    *((int*)MilModel) = 0;
    TCOffstAdjust = FALSE;
    TipToCCDAdjust = FALSE;
    PixToPlusAdjust = FALSE;
    WJOGMode = FALSE;
	m_pCCDDlalog = NULL;//CCD視窗指針
}

CCamera::~CCamera()
{
    if (MilModel != NULL)
        free(MilModel);
    if (m_pCModel != NULL)
        delete (CDialog*)m_pCModel;//釋放影像匹配頁面記憶體
}

void CCamera::DoDataExchange(CDataExchange* pDX)
{
    DDX_Control(pDX, IDC_BTNXUP, m_Xup);
    DDX_Control(pDX, IDC_BTNXDOWN, m_Xdown);
    DDX_Control(pDX, IDC_BTNYUP, m_Yup);
    DDX_Control(pDX, IDC_BTNYDOWN, m_Ydown);
    DDX_Control(pDX, IDC_BTNZUP, m_Zup);
    DDX_Control(pDX, IDC_BTNZDOWN, m_Zdown);
    DDX_Control(pDX, IDC_BTNWUP, m_Wup);
    DDX_Control(pDX, IDC_BTNWDOWN, m_Wdown);

    DDX_Control(pDX, IDC_BTNWREGULATE, m_WRegulate);
    DDX_Control(pDX, IDC_BTNSETTIPTOCCD, m_SetTipToCcd);
    DDX_Control(pDX, IDC_BTNSETPIXTOPULS, m_SetPixToPuls);

    DDX_Control(pDX, IDC_EDITX, m_labsx);
    DDX_Control(pDX, IDC_EDITY, m_labsy);
    DDX_Control(pDX, IDC_EDITZ, m_labsz);
    DDX_Control(pDX, IDC_EDITW, m_dabsw);
    CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CCamera, CDialogEx)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_RADH, IDC_RADL, &CCamera::OnSpeedTypeChange)
	ON_BN_CLICKED(IDC_BTNMOVE, &CCamera::OnBnClickedBtnmove)
    ON_BN_CLICKED(IDC_BTNWMODE, &CCamera::OnBnClickedBtnwmode)
    ON_BN_CLICKED(IDC_BTNFOCUS, &CCamera::OnBnClickedBtnfocus)
    ON_BN_CLICKED(IDC_BTNFOCUSSET, &CCamera::OnBnClickedBtnfocusset)
    ON_BN_CLICKED(IDC_BTNTABLESET, &CCamera::OnBnClickedBtntableset)
    ON_BN_CLICKED(IDC_BTNWHOME, &CCamera::OnBnClickedBtnwhome)
    ON_BN_CLICKED(IDC_BTNLIMITUNLOCK, &CCamera::OnBnClickedBtnlimitunlock)
	ON_BN_CLICKED(IDC_BTNMODEL, &CCamera::OnBnClickedBtnmodel)
    ON_BN_CLICKED(IDC_BTNMODELMATCH, &CCamera::OnBnClickedBtnmodelmatch)
	ON_BN_CLICKED(IDC_BTNSETTIPTOCCD, &CCamera::OnBnClickedBtnsettiptoccd)
    ON_MESSAGE(WM_MY_MESSAGE(IDC_BTNSETTIPTOCCD, 1), OnBnLClickedBtnsettiptoccd)
	ON_BN_CLICKED(IDC_BTNSETPIXTOPULS, &CCamera::OnBnClickedBtnsetpixtopuls)
    ON_MESSAGE(WM_MY_MESSAGE(IDC_BTNSETPIXTOPULS, 1), OnBnLClickedBtnsetpixtopuls)
    ON_BN_CLICKED(IDC_BTNWREGULATE, &CCamera::OnBnClickedBtnwregulate)
    ON_MESSAGE(WM_MY_MESSAGE(IDC_BTNWREGULATE, 1), OnBnLClickedBtnwregulate)
    ON_WM_SHOWWINDOW()
    ON_WM_MOUSEACTIVATE()
	ON_BN_CLICKED(IDC_BTNTOUCHMOVE, &CCamera::OnBnClickedBtntouchmove)
END_MESSAGE_MAP()

// CCamera 訊息處理常式
 
BOOL CCamera::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	CWnd* pMain = AfxGetApp()->m_pMainWnd;
	/*初始化按鈕移動*/
	m_Xup.MoveX = 6;
	m_Xdown.MoveX = 4;
	m_Yup.MoveY = 8;
	m_Ydown.MoveY = 2;
	m_Zup.MoveZ = 7;
	m_Zdown.MoveZ = 1;
    m_Wup.MoveW = 9;
    m_Wdown.MoveW = 3;
    /*初始化按鈕長按功能*/
    m_WRegulate.ControlIDMsg = WM_MY_MESSAGE(IDC_BTNWREGULATE, 1);
    m_WRegulate.hwnd = this->m_hWnd;
    m_SetTipToCcd.ControlIDMsg = WM_MY_MESSAGE(IDC_BTNSETTIPTOCCD, 1);
    m_SetTipToCcd.hwnd = this->m_hWnd;
    m_SetPixToPuls.ControlIDMsg = WM_MY_MESSAGE(IDC_BTNSETPIXTOPULS, 1);
    m_SetPixToPuls.hwnd = this->m_hWnd;
	/*初始化Radio*/
	((CButton *)GetDlgItem(IDC_RADH))->SetCheck(TRUE);
	RaiChoose = 1;
    /*初始化數值顯示*/
	CString StrBuff;             
	StrBuff.Format(_T("PixToPuls:X = %.5f,Y = %.5f"), ((CCommandTestDlg*)pMain)->PixToPulsX, ((CCommandTestDlg*)pMain)->PixToPulsY);
	SetDlgItemText(IDC_STAPIXTOPULS, StrBuff);
	StrBuff.Format(_T("TipToCCD:X = %d,Y = %d"), ((CCommandTestDlg*)pMain)->TipOffset.x, ((CCommandTestDlg*)pMain)->TipOffset.y);
	SetDlgItemText(IDC_STATIPTOCCD, StrBuff);
	StrBuff.Format(_T("FocusHeight:%d"), ((CCommandTestDlg*)pMain)->FocusPoint);
	SetDlgItemText(IDC_STAFOCUSHEIGHT, StrBuff);
    StrBuff.Format(_T("TableZ:%d"), ((CCommandTestDlg*)pMain)->a.m_Action.m_TablelZ);
    SetDlgItemText(IDC_STATABLEZ, StrBuff);
    StrBuff.Format(_T("TCX:%d"), ((CCommandTestDlg*)pMain)->a.m_Action.m_MachineOffSet.x);
    SetDlgItemText(IDC_STATCX, StrBuff);
    StrBuff.Format(_T("TCY:%d"), ((CCommandTestDlg*)pMain)->a.m_Action.m_MachineOffSet.y);
    SetDlgItemText(IDC_STATCY, StrBuff);
	/*初始化影像Dialog*/
	GetDlgItem(IDC_PIC)->GetWindowRect(CCDrect);
	ScreenToClient(CCDrect);
	CCDrectOld = CCDrect;
	//GetDlgItem(IDC_PIC)->MoveWindow(CCDrect.left, CCDrect.top, 1292, 964);
	m_pCCDDlalog = new CCCD();
	m_pCCDDlalog->Create(IDD_CCD, this);
	m_pCCDDlalog->MoveWindow(CCDrect.left, CCDrect.top, CCDrect.Width(), CCDrect.Height());
	m_pCCDDlalog->ShowWindow(SW_SHOW);
	
	_cwprintf(L"CCDrect:%d,%d\n", CCDrect.Width(), CCDrect.Height());
	_cwprintf(L"CameraDlgrect:%d,%d\n", CameraDlgrect.Width(), CameraDlgrect.Height());
	
	/*影像開啟*/
#ifdef VI
	//VI_DisplayAlloc(GetDlgItem(IDC_PIC), 1);
	VI_DisplayAlloc(m_pCCDDlalog, 1);
	VI_DrawBox(1, m_pCCDDlalog, 150, 150);
	VI_DisplayFitWindow(m_pCCDDlalog, 1, CCDrect.Width(), CCDrect.Height(),0);//縮放畫面
	VI_MousePosFuncEable();//啟用MIL滑鼠取值
#endif
	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX 屬性頁應傳回 FALSE
}
/*速度選擇改變*/
void CCamera::OnSpeedTypeChange(UINT uId)
{
	if (uId == IDC_RADH)
	{
		RaiChoose = 1;
	}
	else if (uId == IDC_RADM)
	{
		RaiChoose = 2;
	}
	else
	{
		RaiChoose = 3;
	}
}
/*絕對位置移動*/
void CCamera::OnBnClickedBtnmove()
{
    CWnd* pMain = AfxGetApp()->m_pMainWnd;
#ifdef MOVE
    CString StrBuff;
    GetDlgItemText(IDC_EDITW, StrBuff);
    if (GetDlgItemInt(IDC_EDITX) == 0 && GetDlgItemInt(IDC_EDITY) == 0 && GetDlgItemInt(IDC_EDITZ) == 0)//單動 W
    {
        ((CCommandTestDlg*)pMain)->a.m_Action.HMGoPosition(((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition(UseNeedleMode).x, ((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition(UseNeedleMode).y, ((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition(UseNeedleMode).z, _tstof(StrBuff), AbsoluteSpeedW, AbsoluteSpeedA, AbsoluteSpeedI);//絕對
        //MO_Do3DLineMove(0, 0, GetDlgItemInt(IDC_EDITZ) - MO_ReadLogicPosition(2), 85000, 1000000, 5000);//相對
    }
	else if (GetDlgItemInt(IDC_EDITX) == 0 && GetDlgItemInt(IDC_EDITY) == 0 && _tstof(StrBuff) == 0)//單動 Z
	{
        ((CCommandTestDlg*)pMain)->a.m_Action.HMGoPosition(((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition(UseNeedleMode).x, ((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition(UseNeedleMode).y, GetDlgItemInt(IDC_EDITZ), ((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition(UseNeedleMode).w, AbsoluteSpeedW, AbsoluteSpeedA, AbsoluteSpeedI);//絕對
		//MO_Do3DLineMove(0, 0, GetDlgItemInt(IDC_EDITZ) - MO_ReadLogicPosition(2), 85000, 1000000, 5000);//相對
	}
	else if (GetDlgItemInt(IDC_EDITX) == 0 && GetDlgItemInt(IDC_EDITZ) == 0 && _tstof(StrBuff) == 0)//單動 Y
	{
        ((CCommandTestDlg*)pMain)->a.m_Action.HMGoPosition(((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition(UseNeedleMode).x, GetDlgItemInt(IDC_EDITY), ((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition(UseNeedleMode).z, ((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition(UseNeedleMode).w, AbsoluteSpeedW, AbsoluteSpeedA, AbsoluteSpeedI);//絕對
		//MO_Do3DLineMove(GetDlgItemInt(IDC_EDITX) - MO_ReadLogicPosition(0), 0, 0, 85000, 1000000, 5000);//相對
	}
	else if (GetDlgItemInt(IDC_EDITY) == 0 && GetDlgItemInt(IDC_EDITZ) == 0 && _tstof(StrBuff) == 0)//單動 X
	{
        ((CCommandTestDlg*)pMain)->a.m_Action.HMGoPosition(GetDlgItemInt(IDC_EDITX), ((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition(UseNeedleMode).y, ((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition(UseNeedleMode).z, ((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition(UseNeedleMode).w, AbsoluteSpeedW, AbsoluteSpeedA, AbsoluteSpeedI);//絕對
		//MO_Do3DLineMove(0, GetDlgItemInt(IDC_EDITY) - MO_ReadLogicPosition(1), 0, 85000, 1000000, 5000);//相對
	}
	else//四軸插補
	{
        ((CCommandTestDlg*)pMain)->a.m_Action.HMGoPosition(GetDlgItemInt(IDC_EDITX),GetDlgItemInt(IDC_EDITY),GetDlgItemInt(IDC_EDITZ),_tstof(StrBuff), AbsoluteSpeedW, AbsoluteSpeedA, AbsoluteSpeedI);//絕對
		//MO_Do3DLineMove(GetDlgItemInt(IDC_EDITX) - MO_ReadLogicPosition(0), GetDlgItemInt(IDC_EDITY) - MO_ReadLogicPosition(1), GetDlgItemInt(IDC_EDITZ) - MO_ReadLogicPosition(2), 85000, 1000000, 5000);
	}
#endif

}
/*鍵盤偵測*/
BOOL CCamera::PreTranslateMessage(MSG* pMsg)
{
    if (GetDlgItem(IDC_EDITX)->GetSafeHwnd() != ::GetFocus() &&
        GetDlgItem(IDC_EDITY)->GetSafeHwnd() != ::GetFocus() &&
        GetDlgItem(IDC_EDITZ)->GetSafeHwnd() != ::GetFocus() &&
        GetDlgItem(IDC_EDITW)->GetSafeHwnd() != ::GetFocus())//判斷輸入控件是否在焦點上
    {
        /*備註:再校正時的機械座標偏移量不會歸0*/
        CWnd* pMain = AfxGetApp()->m_pMainWnd;
        if (pMsg->message == WM_KEYDOWN) {
#ifdef MOVE
            if (pMsg->wParam == VK_NUMPAD4) {
                if (((CCommandTestDlg*)pMain)->a.m_Action.m_MachineOffSet.x == TCXYOffsetInit && ((CCommandTestDlg*)pMain)->a.m_Action.m_MachineOffSet.y == TCXYOffsetInit || TCOffstAdjust)//尚未執行非同軸校正
                    MoveXYZW(-(((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition(UseMachineMode).x) + 5000, 0, 0, 0);//往-行程距為200000
                else
                    MoveXYZW(-(((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition(UseNeedleMode).x), 0, 0, 0);
                pMsg->message = WM_NULL;
            }
            if (pMsg->wParam == VK_NUMPAD6) {
                if (((CCommandTestDlg*)pMain)->a.m_Action.m_MachineOffSet.x == TCXYOffsetInit && ((CCommandTestDlg*)pMain)->a.m_Action.m_MachineOffSet.y == TCXYOffsetInit || TCOffstAdjust)//尚未執行非同軸校正
                    MoveXYZW((405000 - ((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition(UseMachineMode).x), 0, 0, 0);//往+行程距為285000//總行程距410000
                else
                    MoveXYZW((PosXWorkRange - ((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition(UseNeedleMode).x), 0, 0, 0);
                pMsg->message = WM_NULL;
            }
            if (pMsg->wParam == VK_NUMPAD8) {
                if (((CCommandTestDlg*)pMain)->a.m_Action.m_MachineOffSet.x == TCXYOffsetInit && ((CCommandTestDlg*)pMain)->a.m_Action.m_MachineOffSet.y == TCXYOffsetInit || TCOffstAdjust)//尚未執行非同軸校正
                    MoveXYZW(0, -(((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition(UseMachineMode).y) + 5000, 0, 0);//往-行程距為200000
                else
                    MoveXYZW(0, -(((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition(UseNeedleMode).y), 0, 0);
                pMsg->message = WM_NULL;
            }
            if (pMsg->wParam == VK_NUMPAD2) {
                if (((CCommandTestDlg*)pMain)->a.m_Action.m_MachineOffSet.x == TCXYOffsetInit && ((CCommandTestDlg*)pMain)->a.m_Action.m_MachineOffSet.y == TCXYOffsetInit || TCOffstAdjust)//尚未執行非同軸校正
                    MoveXYZW(0, (405000 - ((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition(UseMachineMode).y), 0, 0);//往+行程距為285000//總行程距410000
                else
                    MoveXYZW(0, (PosYWorkRange - ((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition(UseNeedleMode).y), 0, 0);
                pMsg->message = WM_NULL;
            }
            if (pMsg->wParam == VK_NUMPAD7) {
                if (((CCommandTestDlg*)pMain)->a.m_Action.m_MachineOffSet.x == TCXYOffsetInit && ((CCommandTestDlg*)pMain)->a.m_Action.m_MachineOffSet.y == TCXYOffsetInit || TCOffstAdjust)//尚未執行非同軸校正
                    MoveXYZW(0, 0, -(((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition(UseMachineMode).z) + 5000, 0);//往-行程距為10000
                else
                    MoveXYZW(0, 0, -(((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition(UseNeedleMode).z), 0);
                pMsg->message = WM_NULL;
            }
            if (pMsg->wParam == VK_NUMPAD1) {
                if (((CCommandTestDlg*)pMain)->a.m_Action.m_MachineOffSet.x == TCXYOffsetInit && ((CCommandTestDlg*)pMain)->a.m_Action.m_MachineOffSet.y == TCXYOffsetInit || TCOffstAdjust)//尚未執行非同軸校正
                    MoveXYZW(0, 0, (110000 - ((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition(UseMachineMode).z), 0);//往+行程距為110000
                else
                    MoveXYZW(0, 0, (((CCommandTestDlg*)pMain)->a.m_Action.m_TablelZ - ((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition(UseNeedleMode).z), 0);
                pMsg->message = WM_NULL;
            }
            if (!TCOffstAdjust && !TipToCCDAdjust && !PixToPlusAdjust)
            {
                if (pMsg->wParam == VK_NUMPAD9) {
                    if (((CCommandTestDlg*)pMain)->a.m_Action.m_MachineOffSet.x == TCXYOffsetInit && ((CCommandTestDlg*)pMain)->a.m_Action.m_MachineOffSet.y == TCXYOffsetInit || TCOffstAdjust)//尚未執行非同軸校正
                    {
                        if (WJOGMode)
                        {
                            MoveXYZW(0, 0, 0, -360);//絕對位置-360
                        }
                        else
                        {
                            MoveXYZW(0, 0, 0, -(((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition(UseMachineMode).w) - 360);//相對往-行程距為360度
                        }
                    }
                    else
                    {
                        if (WJOGMode)
                        {
                            MoveXYZW(0, 0, 0, NegWWorkRange);//絕對位置360
                        }
                        else
                        {
                            MoveXYZW(0, 0, 0, -(((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition(UseNeedleMode).w) + NegWWorkRange);
                        }
                    }              
                    pMsg->message = WM_NULL;
                }
                if (pMsg->wParam == VK_NUMPAD3) {
                    if (((CCommandTestDlg*)pMain)->a.m_Action.m_MachineOffSet.x == TCXYOffsetInit && ((CCommandTestDlg*)pMain)->a.m_Action.m_MachineOffSet.y == TCXYOffsetInit || TCOffstAdjust)//尚未執行非同軸校正
                    {
                        if (WJOGMode)
                        {
                            MoveXYZW(0, 0, 0, 360);//絕對位置360
                        }
                        else
                        {
                            MoveXYZW(0, 0, 0, (360 - ((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition(UseMachineMode).w));//往+行程距為360度
                        }
                    }
                    else
                    {
                        if (WJOGMode)
                        {
                            MoveXYZW(0, 0, 0, PosWWorkRange);//絕對位置360
                        }
                        else
                        {
                            MoveXYZW(0, 0, 0, (PosWWorkRange - ((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition(UseNeedleMode).w));//最大動到360度
                        }
                    }
                    pMsg->message = WM_NULL;
                }
            }       
#endif
        }
        if (pMsg->message == WM_KEYUP) {
            if (pMsg->wParam == VK_NUMPAD4 || pMsg->wParam == VK_NUMPAD6
                || pMsg->wParam == VK_NUMPAD8 || pMsg->wParam == VK_NUMPAD2
                || pMsg->wParam == VK_NUMPAD7 || pMsg->wParam == VK_NUMPAD1
                || pMsg->wParam == VK_NUMPAD9 || pMsg->wParam == VK_NUMPAD3) {
#ifdef MOVE
                CWnd* pMain = AfxGetApp()->m_pMainWnd;
                if (((CCommandTestDlg*)pMain)->a.RunStatusRead.RunLoopStatus == 0)
                {
                    if (((CCommandTestDlg*)pMain)->a.RunStatusRead.RunStatus == 0 || ((CCommandTestDlg*)pMain)->a.RunStatusRead.RunStatus == 2)
                    {
                        if (((CCommandTestDlg*)pMain)->a.RunStatusRead.GoHomeStatus == TRUE)
                        {
                            if (!((CCommandTestDlg*)pMain)->a.RunStatusRead.StepCommandStatus)
                            {
                                MO_DecSTOP();
                            }
                        }
                    }
                }
#endif // MOVE
                pMsg->message = WM_NULL;
            }
        }
    }
	return CDialogEx::PreTranslateMessage(pMsg);
}
/*XYZW鍵盤移動*/
void CCamera::MoveXYZW(int MoveX, int MoveY, int MoveZ, double MoveW) {
#ifdef MOVE
	CWnd* pMain = AfxGetApp()->m_pMainWnd;
	if (((CCommandTestDlg*)pMain)->a.RunStatusRead.RunLoopStatus == 0)
	{
		if (((CCommandTestDlg*)pMain)->a.RunStatusRead.RunStatus == 0 || ((CCommandTestDlg*)pMain)->a.RunStatusRead.RunStatus == 2)
		{
			if (((CCommandTestDlg*)pMain)->a.RunStatusRead.GoHomeStatus == TRUE)
			{
				if (!((CCommandTestDlg*)pMain)->a.RunStatusRead.StepCommandStatus)
				{
					switch (RaiChoose)
					{
					case 1:
                        if (!MO_ReadIsDriving(15))
                        {
                            if (WJOGMode && MoveW != 0)
                            {
                                ((CCommandTestDlg*)pMain)->a.m_Action.MCO_JogMove(MoveX, MoveY, MoveZ, JOGHSpeedW * 2, JOGHSpeedA * 6, JOGHSpeedI, MoveW, WJOGMode);
                            }
                            else
                            {
                                ((CCommandTestDlg*)pMain)->a.m_Action.MCO_JogMove(MoveX, MoveY, MoveZ, JOGHSpeedW, JOGHSpeedA, JOGHSpeedI, MoveW, WJOGMode);
                            }
                        }                 
							//MO_Do3DLineMove(MoveX, MoveY, MoveZ, 80000, 1200000, 6000);
						break;
					case 2:
                        if (!MO_ReadIsDriving(15))
                        {
                            if (WJOGMode && MoveW != 0)
                            {
                                ((CCommandTestDlg*)pMain)->a.m_Action.MCO_JogMove(MoveX, MoveY, MoveZ, JOGMSpeedW * 2, JOGMSpeedA * 6, JOGMSpeedI, MoveW, WJOGMode);
                            }
                            else
                            {
                                ((CCommandTestDlg*)pMain)->a.m_Action.MCO_JogMove(MoveX, MoveY, MoveZ, JOGMSpeedW, JOGMSpeedA, JOGMSpeedI, MoveW, WJOGMode);
                            }
                        }                 
							//MO_Do3DLineMove(MoveX, MoveY, MoveZ, 50000, 800000, 5000);
						break;
					case 3:
                        if (!MO_ReadIsDriving(15))
                        {
                            if (WJOGMode && MoveW != 0)
                            {
                                ((CCommandTestDlg*)pMain)->a.m_Action.MCO_JogMove(MoveX, MoveY, MoveZ, JOGLSpeedW * 2, JOGLSpeedA * 6, JOGLSpeedI, MoveW, WJOGMode);
                            }
                            else
                            {
                                ((CCommandTestDlg*)pMain)->a.m_Action.MCO_JogMove(MoveX, MoveY, MoveZ, JOGLSpeedW, JOGLSpeedA, JOGLSpeedI, MoveW, WJOGMode);
                            }
                        }               
							//MO_Do3DLineMove(MoveX, MoveY, MoveZ, 5000, 50000, 1000);
						break;
					default:
						//MessageBox(_T("程式出現錯誤!"));
						break;
					}
#ifdef PRINTF
					//_cwprintf("進來了:%d", ((CCommandTestDlg*)pMain)->a.RunStatusRead.RunStatus);
#endif
				}         
			}
		}
	}
#ifdef PRINTF
	//_cprintf("%d", ((CCommandTestDlg*)pMain)->a.RunStatusRead.RunStatus);
#endif
#endif // MOVE
}
/*WJOG模式轉換*/
void CCamera::OnBnClickedBtnwmode()
{
    WJOGMode = !WJOGMode;
    if(WJOGMode)
        SetDlgItemText(IDC_BTNWMODE, _T("Fixed"));
    else
        SetDlgItemText(IDC_BTNWMODE, _T("Single"));
}
/*對焦*//*****尚未修正*****/
void CCamera::OnBnClickedBtnfocus()
{
    CWnd* pMain = AfxGetApp()->m_pMainWnd;
#ifdef MOVE
    MO_Do3DLineMove(0, 0, ((CCommandTestDlg*)pMain)->a.VisionDefault.VisionSet.FocusHeight - MO_ReadLogicPosition(2), 30000, 90000, 2000);
#endif
}
/*對焦點設置*/
void CCamera::OnBnClickedBtnfocusset()
{
    CString StrBuff;
    CWnd* pMain = AfxGetApp()->m_pMainWnd;
#ifdef MOVE
    ((CCommandTestDlg*)pMain)->FocusPoint = ((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition(UseMachineMode).z;
    ((CCommandTestDlg*)pMain)->a.VisionDefault.VisionSet.FocusHeight = ((CCommandTestDlg*)pMain)->FocusPoint;
    StrBuff.Format(_T("FocusHeight:%d"), ((CCommandTestDlg*)pMain)->FocusPoint);
    SetDlgItemText(IDC_STAFOCUSHEIGHT, StrBuff);
#endif

}
/*平台高度設置*//***與W軸offset校正做一起***/
void CCamera::OnBnClickedBtntableset()
{
    CString StrBuff;
    int TableZ = 0;
    CWnd* pMain = AfxGetApp()->m_pMainWnd;
    ((CCommandTestDlg*)pMain)->a.SetTabelZ(&TableZ);
    StrBuff.Format(L"TableZ:%d", TableZ);
    SetDlgItemText(IDC_STATABLEZ, StrBuff);
}
/*W軸賦歸*/
void CCamera::OnBnClickedBtnwhome()
{
    CWnd* pMain = AfxGetApp()->m_pMainWnd;
#ifdef MOVE
    ((CCommandTestDlg*)pMain)->a.m_Action.DecideGoHomeW(15000, 500);
#endif
}
/*解除極限*/
void CCamera::OnBnClickedBtnlimitunlock()
{
#ifdef  MOVE
    MO_AlarmCClean();
#endif
}
/*建立model*/
void CCamera::OnBnClickedBtnmodel()
{
    /*獲取目前.exe目錄*/
    CString path = GetCurrentPath(L"\\Temp");
    GetModuleFileName(NULL, path.GetBufferSetLength(MAX_PATH + 1), MAX_PATH);
    path.ReleaseBuffer();
    int pos = path.ReverseFind('\\');
    path = path.Left(pos) + _T("\\Temp");
    LPTSTR lpszText = new TCHAR[path.GetLength() + 1];
    lstrcpy(lpszText, path);
    if (lpszText != NULL)
        delete lpszText;//釋放變數記憶體
                        /*路径不存在则创建该路径*/
    CFileFind m_FileFind;
    if (!m_FileFind.FindFile(path))
    {
        CreateDirectory(path, NULL);
    }
    path = path.Left(pos) + _T("\\Temp\\");
    //MessageBox(path);
    /*獲取現在時間*/
    SYSTEMTIME st;
    GetLocalTime(&st);
    CString StrBuff;
    StrBuff.Format(_T("%04d%02d%02d_%02d_%02d_%02d_%02d"), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
    /*建立model*/
#ifdef VI
    void* Model = NULL;
    //TODO::記憶體配置在Debug中須注意型態
    Model = malloc(4);
    VI_CreateModelFromBox(1, GetDlgItem(IDC_PIC), Model, 150, 150);
    VI_SaveModel(Model, path, StrBuff + _T(".mod"));
    VI_ModelFree(Model);
    free(Model);
    VI_GetPicture(path, StrBuff + _T(".bmp"), 0, 150, 150);
#endif
}
/*模組匹配管理*/
void CCamera::OnBnClickedBtnmodelmatch()
{
    if (m_pCModel == NULL)
    {
        m_pCModel = new CModel();
        m_pCModel->Create(IDD_DIALOG3, this);
        m_pCModel->ShowWindow(SW_SHOW);
    }
    else
    {
        ((CModel*)m_pCModel)->DestroyWindow();
        if (m_pCModel != NULL)
        {
            delete (CDialog*)m_pCModel;
            m_pCModel = NULL;
        }
        m_pCModel = new CModel();
        m_pCModel->Create(IDD_DIALOG3, this);
        m_pCModel->ShowWindow(SW_SHOW);
    }
}
/*設置針頭offset*/
void CCamera::OnBnClickedBtnsettiptoccd()
{        
	CString StrBuff;
	static LONG Point1X = 0;
	static LONG Point1Y = 0;
	GetDlgItemText(IDC_BTNSETTIPTOCCD, StrBuff);
	CWnd* pMain = AfxGetApp()->m_pMainWnd;
#ifdef MOVE
    if (StrBuff == L"SetTipToCCD")
    {
        if (((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition(UseMachineMode).w != dCCDOffsetW)
        {  
            ((CCommandTestDlg*)pMain)->a.m_Action.MCO_Do4DLineMove(dCCDOffsetW, AdjustSpeedW, AdjustSpeedA, AdjustSpeedI, UseMachineMode);//W軸轉置影像角度
        }
        TipToCCDAdjust = TRUE;//針頭offset校正開啟
        SetDlgItemText(IDC_BTNSETTIPTOCCD, _T("SetTipToCCD1"));
    }
	else if (StrBuff == L"SetTipToCCD1")
	{
		Point1X = ((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition(UseMachineMode).x;
		Point1Y = ((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition(UseMachineMode).y;
		SetDlgItemText(IDC_BTNSETTIPTOCCD, _T("SetTipToCCD2"));
	}
	else if (StrBuff == L"SetTipToCCD2")
	{
		((CCommandTestDlg*)pMain)->TipOffset.x = Point1X - ((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition(UseMachineMode).x;
		((CCommandTestDlg*)pMain)->TipOffset.y = Point1Y - ((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition(UseMachineMode).y;
        TipToCCDAdjust = FALSE;//針頭offset校正關閉
		SetDlgItemText(IDC_BTNSETTIPTOCCD, _T("SetTipToCCD"));//設置按鈕名稱
		StrBuff.Format(_T("TipToCCD:X = %d,Y = %d"), ((CCommandTestDlg*)pMain)->TipOffset.x, ((CCommandTestDlg*)pMain)->TipOffset.y);
		SetDlgItemText(IDC_STATIPTOCCD, StrBuff);
		((CCommandTestDlg*)pMain)->a.VisionDefault.VisionSet.AdjustOffsetX = ((CCommandTestDlg*)pMain)->TipOffset.x;
		((CCommandTestDlg*)pMain)->a.VisionDefault.VisionSet.AdjustOffsetY = ((CCommandTestDlg*)pMain)->TipOffset.y;
#ifdef VI
        VI_SetCameraToTipOffset(((CCommandTestDlg*)pMain)->TipOffset.x, ((CCommandTestDlg*)pMain)->TipOffset.y);
#endif
	}
	else
	{
        TipToCCDAdjust = FALSE;//針頭offset校正關閉
		SetDlgItemText(IDC_BTNSETTIPTOCCD, _T("SetTipToCCD"));
	}
#endif
}
/*設置針頭長按取消*/
LRESULT CCamera::OnBnLClickedBtnsettiptoccd(WPARAM wParam, LPARAM lParam)
{
    SetDlgItemText(IDC_BTNSETTIPTOCCD, _T("SetTipToCCDCancel"));
    return LRESULT();
}
/*像素實際距離轉換*/
void CCamera::OnBnClickedBtnsetpixtopuls()
{
	CString StrBuff;
	static LONG Point1X = 0;
	static LONG Point1Y=0;
	CWnd* pMain = AfxGetApp()->m_pMainWnd;
	GetDlgItemText(IDC_BTNSETPIXTOPULS,StrBuff);
#ifdef VI
    if (StrBuff == L"SetPixToPuls")
	{
        if (((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition(UseMachineMode).w != dCCDOffsetW)
        {
            ((CCommandTestDlg*)pMain)->a.m_Action.MCO_Do4DLineMove(dCCDOffsetW, AdjustSpeedW, AdjustSpeedA, AdjustSpeedI, UseMachineMode);//W軸轉置影像角度
        }
        PixToPlusAdjust = TRUE;//像素轉實際距離轉換開啟
		VI_DrawFOVFrame(2, GetDlgItem(IDC_PIC), 150, 150);
		SetDlgItemText(IDC_BTNSETPIXTOPULS, _T("SetPixToPuls1"));
	}
	else if (StrBuff == L"SetPixToPuls1")
	{
#ifdef MOVE
		Point1X = ((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition(UseMachineMode).x;
		Point1Y = ((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition(UseMachineMode).y;
#endif
		VI_CreateModelFromBox(2, GetDlgItem(IDC_PIC),MilModel, 150, 150);
		VI_DrawFOVFrame(3, GetDlgItem(IDC_PIC), 150, 150);
		SetDlgItemText(IDC_BTNSETPIXTOPULS, _T("SetPixToPuls2"));
	}
	else if (StrBuff == L"SetPixToPuls2")
	{
		VI_SetPatternMatch(MilModel, 1, 1, 80, 0, 360);
#ifdef MOVE
		VI_SetPixelPulseRelation(GetDlgItem(IDC_PIC),MilModel, Point1X, Point1Y, ((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition(UseMachineMode).x, ((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition(UseMachineMode).y, ((CCommandTestDlg*)pMain)->PixToPulsX, ((CCommandTestDlg*)pMain)->PixToPulsY);
#endif
		if (((CCommandTestDlg*)pMain)->PixToPulsX != 0.0)
		{
            PixToPlusAdjust = FALSE;//像素轉實際距離轉換關閉
			StrBuff.Format(_T("PixToPuls:X = %.5f,Y = %.5f"), ((CCommandTestDlg*)pMain)->PixToPulsX, ((CCommandTestDlg*)pMain)->PixToPulsY);
			SetDlgItemText(IDC_STAPIXTOPULS, StrBuff);
			VI_ModelFree(MilModel);
			//free(MilModel);
			*((int*)MilModel) = 0;
			SetDlgItemText(IDC_BTNSETPIXTOPULS, _T("SetPixToPuls"));
			VI_DrawFOVFrame(1, GetDlgItem(IDC_PIC), 150, 150);
		}    
	}
	else
	{
        PixToPlusAdjust = FALSE;//像素轉實際距離轉換關閉
		SetDlgItemText(IDC_BTNSETPIXTOPULS, _T("SetPixToPuls"));
	}
#endif
}  
/*像素實際距離轉換長按取消*/
LRESULT CCamera::OnBnLClickedBtnsetpixtopuls(WPARAM wParam, LPARAM lParam)
{
    SetDlgItemText(IDC_BTNSETPIXTOPULS, _T("SetPixToPulsCancel"));
    return LRESULT();
}
/*W軸校正按鈕*/
void CCamera::OnBnClickedBtnwregulate()
{
    CString StrBuff;
    static LONG Point1X = 0;
    static LONG Point1Y = 0;
    CWnd* pMain = AfxGetApp()->m_pMainWnd;
    GetDlgItemText(IDC_BTNWREGULATE, StrBuff);
    if (StrBuff == L"W校正")
    {
        /*((CCommandTestDlg*)pMain)->a.m_Action.m_MachineOffSet = { TCXYOffsetInit,TCXYOffsetInit };
        ((CCommandTestDlg*)pMain)->a.Home(0);*/
        TCOffstAdjust = TRUE;
        ((CCommandTestDlg*)pMain)->a.m_Action.W_NeedleGoHoming(((CCommandTestDlg*)pMain)->a.Default.GoHome.Speed1, ((CCommandTestDlg*)pMain)->a.Default.GoHome.Speed2, 0);
        SetDlgItemText(IDC_BTNWREGULATE, _T("W校正1"));
    }
    else if (StrBuff == L"W校正1")
    {
        ((CCommandTestDlg*)pMain)->a.m_Action.W_Correction(0, AdjustSpeedW, AdjustSpeedA, AdjustSpeedI);
        SetDlgItemText(IDC_BTNWREGULATE, _T("W校正2"));
    }
    else if (StrBuff == L"W校正2")
    {
        ((CCommandTestDlg*)pMain)->a.m_Action.W_Correction(1, AdjustSpeedW, AdjustSpeedA, AdjustSpeedI);
        //((CCommandTestDlg*)pMain)->a.Home(0);
        TCOffstAdjust = FALSE;
        SetDlgItemText(IDC_BTNWREGULATE, _T("W校正"));
        StrBuff.Format(_T("TCX:%d"), ((CCommandTestDlg*)pMain)->a.m_Action.m_MachineOffSet.x);
        SetDlgItemText(IDC_STATCX, StrBuff);
        StrBuff.Format(_T("TCY:%d"), ((CCommandTestDlg*)pMain)->a.m_Action.m_MachineOffSet.y);
        SetDlgItemText(IDC_STATCY, StrBuff);
        StrBuff.Format(_T("TableZ:%d"), ((CCommandTestDlg*)pMain)->a.m_Action.m_TablelZ);
        SetDlgItemText(IDC_STATABLEZ, StrBuff);
    }
    else
    {
        TCOffstAdjust = FALSE;
        SetDlgItemText(IDC_BTNWREGULATE, _T("W校正"));   
    }
}
/*W軸校正長按取消*/
LRESULT CCamera::OnBnLClickedBtnwregulate(WPARAM wParam, LPARAM lParam)
{
    SetDlgItemText(IDC_BTNWREGULATE, _T("W校正取消"));
    return LRESULT();
}
/*關閉對話框*/
void CCamera::OnCancel()
{
	if (*((int*)MilModel) != 0)
	{
#ifdef VI
		VI_ModelFree(MilModel);
		*((int*)MilModel) = 0;
#endif
	}
	CDialogEx::OnCancel();
}
/*Enter鍵功能改為改變控鍵焦點*/
void CCamera::OnOK()                                                                                            
{
    ::SetFocus(NULL);
	//CDialogEx::OnOK();
}
/*觸控式移動*/
void CCamera::OnBnClickedBtntouchmove()
{
	CString StrBuff;
	GetDlgItemText(IDC_BTNTOUCHMOVE, StrBuff);
	if (StrBuff == L"新功能!\r觸控式移動\r停用")
	{
		VI_MousePosFuncDisable();
		((CCCD*)m_pCCDDlalog)->m_TouchMoveSwitch = FALSE;
		SetDlgItemText(IDC_BTNTOUCHMOVE, L"新功能!\r觸控式移動\r啟用");
	}
	else if (StrBuff == L"新功能!\r觸控式移動\r啟用")
	{
		VI_MousePosFuncEable();
		((CCCD*)m_pCCDDlalog)->m_TouchMoveSwitch = TRUE;
		SetDlgItemText(IDC_BTNTOUCHMOVE, L"新功能!\r觸控式移動\r停用");
	}
}
/*銷毀視窗事件*/
BOOL CCamera::DestroyWindow()
{

    //VI_DisplayAlloc(NULL, 1);
    if (*((int*)MilModel) == 0)
    {
        free(MilModel);
        MilModel = NULL;//釋放後記得將指針賦予初值
#ifdef PRINTF
        _cwprintf(_T("free"));
#endif
    }
#ifdef VI
    else
    {
        VI_ModelFree(MilModel);
        free(MilModel);
#ifdef PRINTF
        _cwprintf(_T("VI_ModelFree"));
#endif
    }
#endif
	if (m_pCCDDlalog != NULL)
	{
		delete m_pCCDDlalog;//清除影像記憶體
	}
    return CDialogEx::DestroyWindow();
}
/*顯示視窗時設定*/
void CCamera::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);
	SetWindowLong(this->m_hWnd, GWL_EXSTYLE, GetWindowLong(this->m_hWnd, GWL_EXSTYLE) | WS_EX_LAYERED );//設置視窗為可以透明化
	this->SetLayeredWindowAttributes(0, (255 * 100) / 100, LWA_ALPHA);//不透明
}
/*非活動轉活動事件*/
int CCamera::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
{
	//從非活動轉為活動改成不透明
	this->SetLayeredWindowAttributes(0, (255 * 100) / 100, LWA_ALPHA);
	
	return CDialogEx::OnMouseActivate(pDesktopWnd, nHitTest, message);
}







