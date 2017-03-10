// Camera.cpp : 實作檔
//

#include "stdafx.h"
#include "CommandTest.h"
#include "Camera.h"
#include "afxdialogex.h"
#include "CommandTestDlg.h"
#define EPOCHFILETIME   (116444736000000000UL)
#define PosXWorkRange 350000
#define NegXWorkRange -350000
#define PosYWorkRange 340000
#define NegYWorkRange -340000
#define PosZWorkRange 85000
#define NegZWorkRange -85000
#define PosWWorkRange 360
#define NegWWorkRange -360
#define TCXYOffsetInit -99999
#define AbsoluteSpeedW 30000
#define AbsoluteSpeedA 90000
#define AbsoluteSpeedI 2000
#define JOGHSpeedW 30000
#define JOGHSpeedA 90000
#define JOGHSpeedI 2000
#define JOGMSpeedW 20000
#define JOGMSpeedA 60000
#define JOGMSpeedI 2000
#define JOGLSpeedW 5000
#define JOGLSpeedA 15000
#define JOGLSpeedI 2000
#define TCXYOffsetSpeedW 40000
#define TCXYOffsetSpeedA 120000
#define TCXYOffsetSpeedI 2000 
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

    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDITX, m_labsx);
    DDX_Control(pDX, IDC_EDITY, m_labsy);
    DDX_Control(pDX, IDC_EDITZ, m_labsz);
    DDX_Control(pDX, IDC_EDITW, m_dabsw);
}

BEGIN_MESSAGE_MAP(CCamera, CDialogEx)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_RADH, IDC_RADL, &CCamera::OnSpeedTypeChange)
	ON_BN_CLICKED(IDC_BTNMOVE, &CCamera::OnBnClickedBtnmove)
	ON_BN_CLICKED(IDC_BTNMODEL, &CCamera::OnBnClickedBtnmodel)
	ON_BN_CLICKED(IDC_BTNSETTIPTOCCD, &CCamera::OnBnClickedBtnsettiptoccd)
    ON_BN_CLICKED(IDC_BTNLIMITUNLOCK, &CCamera::OnBnClickedBtnlimitunlock)
	ON_BN_CLICKED(IDC_BTNSETPIXTOPULS, &CCamera::OnBnClickedBtnsetpixtopuls)
    ON_BN_CLICKED(IDC_BTNMODELMATCH, &CCamera::OnBnClickedBtnmodelmatch)
	ON_BN_CLICKED(IDC_BTNFOCUSSET, &CCamera::OnBnClickedBtnfocusset)
    ON_BN_CLICKED(IDC_BTNTABLESET, &CCamera::OnBnClickedBtntableset)
	ON_BN_CLICKED(IDC_BTNFOCUS, &CCamera::OnBnClickedBtnfocus)
	ON_WM_SHOWWINDOW()
	ON_WM_MOUSEACTIVATE()
	
    
    ON_BN_CLICKED(IDC_BTNWREGULATE, &CCamera::OnBnClickedBtnwregulate)
    
    
END_MESSAGE_MAP()

// CCamera 訊息處理常式
 
BOOL CCamera::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	CWnd* pMain = AfxGetApp()->m_pMainWnd;
	/*初始化按鈕移動*/
	m_Xup.MoveX = PosXWorkRange;
	m_Xdown.MoveX = NegXWorkRange;
	m_Yup.MoveY = PosYWorkRange;
	m_Ydown.MoveY = NegYWorkRange;
	m_Zup.MoveZ = PosZWorkRange;
	m_Zdown.MoveZ = NegZWorkRange;
	/*初始化Radio*/
	((CButton *)GetDlgItem(IDC_RADH))->SetCheck(TRUE);
	RaiChoose = 1;                
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
	//影像開啟
#ifdef VI
	VI_DisplayAlloc(GetDlgItem(IDC_PIC), 1);
	VI_DrawBox(1, GetDlgItem(IDC_PIC), 150, 150);
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
        ((CCommandTestDlg*)pMain)->a.m_Action.HMGoPosition(((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition().x, ((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition().y, ((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition().z, _tstof(StrBuff), AbsoluteSpeedW, AbsoluteSpeedA, AbsoluteSpeedI);//絕對
        //MO_Do3DLineMove(0, 0, GetDlgItemInt(IDC_EDITZ) - MO_ReadLogicPosition(2), 85000, 1000000, 5000);//相對
    }
	else if (GetDlgItemInt(IDC_EDITX) == 0 && GetDlgItemInt(IDC_EDITY) == 0 && _tstof(StrBuff) == 0)//單動 Z
	{
        ((CCommandTestDlg*)pMain)->a.m_Action.HMGoPosition(((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition().x, ((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition().y, GetDlgItemInt(IDC_EDITZ), ((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition().w, AbsoluteSpeedW, AbsoluteSpeedA, AbsoluteSpeedI);//絕對
		//MO_Do3DLineMove(0, 0, GetDlgItemInt(IDC_EDITZ) - MO_ReadLogicPosition(2), 85000, 1000000, 5000);//相對
	}
	else if (GetDlgItemInt(IDC_EDITX) == 0 && GetDlgItemInt(IDC_EDITZ) == 0 && _tstof(StrBuff) == 0)//單動 Y
	{
        ((CCommandTestDlg*)pMain)->a.m_Action.HMGoPosition(((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition().x, GetDlgItemInt(IDC_EDITY), ((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition().z, ((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition().w, AbsoluteSpeedW, AbsoluteSpeedA, AbsoluteSpeedI);//絕對
		//MO_Do3DLineMove(GetDlgItemInt(IDC_EDITX) - MO_ReadLogicPosition(0), 0, 0, 85000, 1000000, 5000);//相對
	}
	else if (GetDlgItemInt(IDC_EDITY) == 0 && GetDlgItemInt(IDC_EDITZ) == 0 && _tstof(StrBuff) == 0)//單動 X
	{
        ((CCommandTestDlg*)pMain)->a.m_Action.HMGoPosition(GetDlgItemInt(IDC_EDITX), ((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition().y, ((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition().z, ((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition().w, AbsoluteSpeedW, AbsoluteSpeedA, AbsoluteSpeedI);//絕對
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
        GetDlgItem(IDC_EDITW)->GetSafeHwnd() != ::GetFocus())
    {
        CWnd* pMain = AfxGetApp()->m_pMainWnd;
        if (pMsg->message == WM_KEYDOWN) {
#ifdef MOVE
            if (pMsg->wParam == VK_NUMPAD4) {
                if (((CCommandTestDlg*)pMain)->a.m_Action.m_MachineOffSet.x == TCXYOffsetInit && ((CCommandTestDlg*)pMain)->a.m_Action.m_MachineOffSet.y == TCXYOffsetInit || TCOffstAdjust)//尚未執行非同軸校正
                    MoveXYZ(-(MO_ReadLogicPosition(0)) - 195000, 0, 0, 0);//往-行程距為200000
                else
                    MoveXYZ(-(((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition().x), 0, 0, 0);
                pMsg->message = WM_NULL;
            }
            if (pMsg->wParam == VK_NUMPAD6) {
                if (((CCommandTestDlg*)pMain)->a.m_Action.m_MachineOffSet.x == TCXYOffsetInit && ((CCommandTestDlg*)pMain)->a.m_Action.m_MachineOffSet.y == TCXYOffsetInit || TCOffstAdjust)//尚未執行非同軸校正
                    MoveXYZ((280000 - MO_ReadLogicPosition(0)), 0, 0, 0);//往+行程距為285000
                else
                    MoveXYZ((PosXWorkRange - ((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition().x), 0, 0, 0);
                pMsg->message = WM_NULL;
            }
            if (pMsg->wParam == VK_NUMPAD8) {
                if (((CCommandTestDlg*)pMain)->a.m_Action.m_MachineOffSet.x == TCXYOffsetInit && ((CCommandTestDlg*)pMain)->a.m_Action.m_MachineOffSet.y == TCXYOffsetInit || TCOffstAdjust)//尚未執行非同軸校正
                    MoveXYZ(0, -(MO_ReadLogicPosition(1)) - 195000, 0, 0);//往-行程距為200000
                else
                    MoveXYZ(0, -(((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition().y), 0, 0);
                pMsg->message = WM_NULL;
            }
            if (pMsg->wParam == VK_NUMPAD2) {
                if (((CCommandTestDlg*)pMain)->a.m_Action.m_MachineOffSet.x == TCXYOffsetInit && ((CCommandTestDlg*)pMain)->a.m_Action.m_MachineOffSet.y == TCXYOffsetInit || TCOffstAdjust)//尚未執行非同軸校正
                    MoveXYZ(0, (280000 - MO_ReadLogicPosition(1)), 0, 0);//往+行程距為285000
                else
                    MoveXYZ(0, (PosYWorkRange - ((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition().y), 0, 0);
                pMsg->message = WM_NULL;
            }
            if (pMsg->wParam == VK_NUMPAD7) {
                if (((CCommandTestDlg*)pMain)->a.m_Action.m_MachineOffSet.x == TCXYOffsetInit && ((CCommandTestDlg*)pMain)->a.m_Action.m_MachineOffSet.y == TCXYOffsetInit || TCOffstAdjust)//尚未執行非同軸校正
                    MoveXYZ(0, 0, -(MO_ReadLogicPosition(2)) - 5000, 0);//往-行程距為10000
                else
                    MoveXYZ(0, 0, -(((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition().z), 0);
                pMsg->message = WM_NULL;
            }
            if (pMsg->wParam == VK_NUMPAD1) {
                if (((CCommandTestDlg*)pMain)->a.m_Action.m_MachineOffSet.x == TCXYOffsetInit && ((CCommandTestDlg*)pMain)->a.m_Action.m_MachineOffSet.y == TCXYOffsetInit || TCOffstAdjust)//尚未執行非同軸校正
                    MoveXYZ(0, 0, (60000 - MO_ReadLogicPosition(2)), 0);//往+行程距為60000
                else
                    MoveXYZ(0, 0, (PosZWorkRange - ((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition().z), 0);
                pMsg->message = WM_NULL;
            }
            if (pMsg->wParam == VK_NUMPAD9) {
                if (((CCommandTestDlg*)pMain)->a.m_Action.m_MachineOffSet.x == TCXYOffsetInit && ((CCommandTestDlg*)pMain)->a.m_Action.m_MachineOffSet.y == TCXYOffsetInit || TCOffstAdjust)//尚未執行非同軸校正
                    MoveXYZ(0, 0, 0, -(MO_ReadLogicPositionW()) - 359);//往-行程距為360度
                else
                    MoveXYZ(0, 0, 0, -(((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition().w));
                pMsg->message = WM_NULL;
            }
            if (pMsg->wParam == VK_NUMPAD3) {
                if (((CCommandTestDlg*)pMain)->a.m_Action.m_MachineOffSet.x == TCXYOffsetInit && ((CCommandTestDlg*)pMain)->a.m_Action.m_MachineOffSet.y == TCXYOffsetInit || TCOffstAdjust)//尚未執行非同軸校正
                    MoveXYZ(0, 0, 0, (359 - MO_ReadLogicPositionW()));//往+行程距為360度
                else
                    MoveXYZ(0, 0, 0, (PosWWorkRange - ((CCommandTestDlg*)pMain)->a.m_Action.MCO_ReadPosition().w - 1));//最大動到359度
                pMsg->message = WM_NULL;
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
/*XYZ鍵盤移動*/
void CCamera::MoveXYZ(int MoveX, int MoveY, int MoveZ, double MoveW) {
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
                            ((CCommandTestDlg*)pMain)->a.m_Action.MCO_JogMove(MoveX, MoveY, MoveZ, JOGHSpeedW, JOGHSpeedA, JOGHSpeedI, MoveW, 0);
							//MO_Do3DLineMove(MoveX, MoveY, MoveZ, 80000, 1200000, 6000);
						break;
					case 2:
						if (!MO_ReadIsDriving(15))
                            ((CCommandTestDlg*)pMain)->a.m_Action.MCO_JogMove(MoveX, MoveY, MoveZ, JOGMSpeedW, JOGMSpeedA, JOGMSpeedI, MoveW, 0);
							//MO_Do3DLineMove(MoveX, MoveY, MoveZ, 50000, 800000, 5000);
						break;
					case 3:
						if (!MO_ReadIsDriving(15))
                            ((CCommandTestDlg*)pMain)->a.m_Action.MCO_JogMove(MoveX, MoveY, MoveZ, JOGLSpeedW, JOGLSpeedA, JOGLSpeedI, MoveW, 0);
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
/*對焦*//*****尚未修正*****/
void CCamera::OnBnClickedBtnfocus()
{
    CWnd* pMain = AfxGetApp()->m_pMainWnd;
#ifdef MOVE
    MO_Do3DLineMove(0, 0, ((CCommandTestDlg*)pMain)->a.VisionDefault.VisionSet.FocusHeight - MO_ReadLogicPosition(2), 30000, 100000, 5000);
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
/*對焦點設置*/
void CCamera::OnBnClickedBtnfocusset()
{
    CString StrBuff;
    CWnd* pMain = AfxGetApp()->m_pMainWnd;
#ifdef MOVE
    ((CCommandTestDlg*)pMain)->FocusPoint = MO_ReadLogicPosition(2);
    ((CCommandTestDlg*)pMain)->a.VisionDefault.VisionSet.FocusHeight = ((CCommandTestDlg*)pMain)->FocusPoint;
    StrBuff.Format(_T("FocusHeight:%d"), ((CCommandTestDlg*)pMain)->FocusPoint);
    SetDlgItemText(IDC_STAFOCUSHEIGHT, StrBuff);
#endif

}
/*平台高度設置*/
void CCamera::OnBnClickedBtntableset()
{
    CString StrBuff;
    int TableZ = 0;
    CWnd* pMain = AfxGetApp()->m_pMainWnd;
    ((CCommandTestDlg*)pMain)->a.SetTabelZ(&TableZ);
    StrBuff.Format(L"TableZ:%d", TableZ);
    SetDlgItemText(IDC_STATABLEZ, StrBuff);
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
		Point1X = MO_ReadLogicPosition(0);
		Point1Y = MO_ReadLogicPosition(1);
		SetDlgItemText(IDC_BTNSETTIPTOCCD, _T("SetTipToCCD1"));
	}
	else if (StrBuff == L"SetTipToCCD1")
	{
		((CCommandTestDlg*)pMain)->TipOffset.x = Point1X - MO_ReadLogicPosition(0);
		((CCommandTestDlg*)pMain)->TipOffset.y = Point1Y - MO_ReadLogicPosition(1);
		SetDlgItemText(IDC_BTNSETTIPTOCCD, _T("SetTipToCCD"));
		StrBuff.Format(_T("TipToCCD:X = %d,Y = %d"), ((CCommandTestDlg*)pMain)->TipOffset.x, ((CCommandTestDlg*)pMain)->TipOffset.y);
		SetDlgItemText(IDC_STATIPTOCCD, StrBuff);
		((CCommandTestDlg*)pMain)->a.VisionDefault.VisionSet.AdjustOffsetX = ((CCommandTestDlg*)pMain)->TipOffset.x;
		((CCommandTestDlg*)pMain)->a.VisionDefault.VisionSet.AdjustOffsetY = ((CCommandTestDlg*)pMain)->TipOffset.y;
	}
	else
	{
		SetDlgItemText(IDC_BTNSETTIPTOCCD, _T("SetTipToCCD"));
	}
#endif
#ifdef VI
	VI_SetCameraToTipOffset(((CCommandTestDlg*)pMain)->TipOffset.x, ((CCommandTestDlg*)pMain)->TipOffset.y);
#endif
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
		VI_DrawFOVFrame(2, GetDlgItem(IDC_PIC), 150, 150);
		SetDlgItemText(IDC_BTNSETPIXTOPULS, _T("SetPixToPuls1"));
	}
	else if (StrBuff == L"SetPixToPuls1")
	{
#ifdef MOVE
		Point1X = MO_ReadLogicPosition(0);
		Point1Y = MO_ReadLogicPosition(1);
#endif
		VI_CreateModelFromBox(2, GetDlgItem(IDC_PIC),MilModel, 150, 150);
		VI_DrawFOVFrame(3, GetDlgItem(IDC_PIC), 150, 150);
		SetDlgItemText(IDC_BTNSETPIXTOPULS, _T("SetPixToPuls2"));
	}
	else if (StrBuff == L"SetPixToPuls2")
	{
		VI_SetPatternMatch(MilModel, 1, 1, 80, 0, 360);
#ifdef MOVE
		VI_SetPixelPulseRelation(GetDlgItem(IDC_PIC),MilModel, Point1X, Point1Y, MO_ReadLogicPosition(0), MO_ReadLogicPosition(1), ((CCommandTestDlg*)pMain)->PixToPulsX, ((CCommandTestDlg*)pMain)->PixToPulsY);
#endif
		if (((CCommandTestDlg*)pMain)->PixToPulsX != 0.0)
		{
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
		SetDlgItemText(IDC_BTNSETPIXTOPULS, _T("SetPixToPuls"));
	}
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
        ((CCommandTestDlg*)pMain)->a.m_Action.W_Correction(0, TCXYOffsetSpeedW, TCXYOffsetSpeedA, TCXYOffsetSpeedI);//速度值沒用途
        SetDlgItemText(IDC_BTNWREGULATE, _T("W校正2"));
    }
    else if (StrBuff == L"W校正2")
    {
        ((CCommandTestDlg*)pMain)->a.m_Action.W_Correction(1, TCXYOffsetSpeedW, TCXYOffsetSpeedA, TCXYOffsetSpeedI);//速度值沒用途
        ((CCommandTestDlg*)pMain)->a.Home(0);
        TCOffstAdjust = FALSE;
        SetDlgItemText(IDC_BTNWREGULATE, _T("W校正"));
        StrBuff.Format(_T("TCX:%d"), ((CCommandTestDlg*)pMain)->a.m_Action.m_MachineOffSet.x);
        SetDlgItemText(IDC_STATCX, StrBuff);
        StrBuff.Format(_T("TCY:%d"), ((CCommandTestDlg*)pMain)->a.m_Action.m_MachineOffSet.y);
        SetDlgItemText(IDC_STATCY, StrBuff);
    }
    else
    {
        SetDlgItemText(IDC_BTNWREGULATE, _T("W校正"));
        TCOffstAdjust = FALSE;
    }
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
    return CDialogEx::DestroyWindow();
}
/*顯示視窗時設定*/
void CCamera::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);
	SetWindowLong(this->m_hWnd, GWL_EXSTYLE, GetWindowLong(this->m_hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);//設置視窗為可以透明化
	this->SetLayeredWindowAttributes(0, (255 * 100) / 100, LWA_ALPHA);//不透明
}
/*非活動轉活動事件*/
int CCamera::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
{
	//從非活動轉為活動改成不透明
	this->SetLayeredWindowAttributes(0, (255 * 100) / 100, LWA_ALPHA);
	return CDialogEx::OnMouseActivate(pDesktopWnd, nHitTest, message);
}

