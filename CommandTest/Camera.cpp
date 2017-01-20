// Camera.cpp : 實作檔
//

#include "stdafx.h"
#include "CommandTest.h"
#include "Camera.h"
#include "afxdialogex.h"
#include "CommandTestDlg.h"
#define EPOCHFILETIME   (116444736000000000UL)

// CCamera 對話方塊

IMPLEMENT_DYNAMIC(CCamera, CDialogEx)

CCamera::CCamera(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG2, pParent)
{
	RaiChoose = 1;
	MilModel = malloc(sizeof(int));
	*((int*)MilModel) = 0;
}

CCamera::~CCamera()
{
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
}


BEGIN_MESSAGE_MAP(CCamera, CDialogEx)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_RADH, IDC_RADL, &CCamera::OnSpeedTypeChange)
	ON_BN_CLICKED(IDC_BTNMOVE, &CCamera::OnBnClickedBtnmove)
	ON_BN_CLICKED(IDC_BTNMODEL, &CCamera::OnBnClickedBtnmodel)
	ON_BN_CLICKED(IDC_BUTTON1, &CCamera::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CCamera::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CCamera::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON4, &CCamera::OnBnClickedButton4)
	ON_BN_CLICKED(IDC_BTNFOCUSSET, &CCamera::OnBnClickedBtnfocusset)
	ON_BN_CLICKED(IDC_BTNFOCUS, &CCamera::OnBnClickedBtnfocus)
	ON_WM_SHOWWINDOW()
	ON_WM_MOUSEACTIVATE()
	
    ON_BN_CLICKED(IDC_BTNTABLESET, &CCamera::OnBnClickedBtntableset)
END_MESSAGE_MAP()

// CCamera 訊息處理常式

BOOL CCamera::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	CWnd* pMain = AfxGetApp()->m_pMainWnd;
	/*初始化按鈕移動*/
	m_Xup.MoveX = 350000;
	m_Xdown.MoveX = -350000;
	m_Yup.MoveY = 340000;
	m_Ydown.MoveY = -340000;
	m_Zup.MoveZ = 85000;
	m_Zdown.MoveZ = -85000;
	/*初始化Radio*/
	((CButton *)GetDlgItem(IDC_RADH))->SetCheck(TRUE);
	RaiChoose = 1;
	CString StrBuff;
	StrBuff.Format(_T("PixToPuls:X = %.6f,Y = %.6f"), ((CCommandTestDlg*)pMain)->PixToPulsX, ((CCommandTestDlg*)pMain)->PixToPulsY);
	SetDlgItemText(IDC_PIXTOPULS, StrBuff);
	StrBuff.Format(_T("TipToCCD:X = %d,Y = %d"), ((CCommandTestDlg*)pMain)->TipOffset.x, ((CCommandTestDlg*)pMain)->TipOffset.y);
	SetDlgItemText(IDC_TIPTOCCD, StrBuff);
	StrBuff.Format(_T("FocusHeight:%d"), ((CCommandTestDlg*)pMain)->FocusPoint);
	SetDlgItemText(IDC_FOCUSHEIGHT, StrBuff);
    StrBuff.Format(_T("TableZ:%d"), ((CCommandTestDlg*)pMain)->a.m_Action.g_TablelZ);
    SetDlgItemText(IDC_TABLEZ, StrBuff);
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
//絕對位置移動
void CCamera::OnBnClickedBtnmove()
{
#ifdef MOVE
	if (GetDlgItemInt(IDC_EDITX) == 0 && GetDlgItemInt(IDC_EDITY) == 0)
	{
		MO_Do3DLineMove(0, 0, GetDlgItemInt(IDC_EDITZ) - MO_ReadLogicPosition(2), 85000, 1000000, 5000);
	}
	else if (GetDlgItemInt(IDC_EDITY) == 0 && GetDlgItemInt(IDC_EDITZ) == 0)
	{
		MO_Do3DLineMove(GetDlgItemInt(IDC_EDITX) - MO_ReadLogicPosition(0), 0, 0, 85000, 1000000, 5000);
	}
	else if (GetDlgItemInt(IDC_EDITX) == 0 && GetDlgItemInt(IDC_EDITZ) == 0)
	{
		MO_Do3DLineMove(0, GetDlgItemInt(IDC_EDITY) - MO_ReadLogicPosition(1), 0, 85000, 1000000, 5000);
	}
	else
	{
		MO_Do3DLineMove(GetDlgItemInt(IDC_EDITX) - MO_ReadLogicPosition(0), GetDlgItemInt(IDC_EDITY) - MO_ReadLogicPosition(1), GetDlgItemInt(IDC_EDITZ) - MO_ReadLogicPosition(2), 85000, 1000000, 5000);
	}
#endif

}
//建立model
void CCamera::OnBnClickedBtnmodel()
{
	/*獲取目前.exe目錄*/
	CString path;
	GetModuleFileName(NULL, path.GetBufferSetLength(MAX_PATH + 1), MAX_PATH);
	path.ReleaseBuffer();
	int pos = path.ReverseFind('\\');
	path = path.Left(pos) + _T("\\Temp");
	LPTSTR lpszText = new TCHAR[path.GetLength() + 1];
	lstrcpy(lpszText, path);
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
/*鍵盤偵測*/
BOOL CCamera::PreTranslateMessage(MSG* pMsg)
{

	if (pMsg->message == WM_KEYDOWN) {
#ifdef MOVE
		if (pMsg->wParam == VK_LEFT) {
			MoveXYZ(-(MO_ReadLogicPosition(0)), 0, 0);
			pMsg->message = WM_NULL;
		}
		if (pMsg->wParam == VK_RIGHT) {
			MoveXYZ((350000 - MO_ReadLogicPosition(0)), 0, 0);
			pMsg->message = WM_NULL;
		}
		if (pMsg->wParam == VK_UP) {
			MoveXYZ(0,-(MO_ReadLogicPosition(1)), 0);
			pMsg->message = WM_NULL;
		}
		if (pMsg->wParam == VK_DOWN) {
			MoveXYZ(0, (340000 - MO_ReadLogicPosition(1)), 0);
			pMsg->message = WM_NULL;
		}
		if (pMsg->wParam == VK_HOME) {
			MoveXYZ(0, 0, -(MO_ReadLogicPosition(2)));
			pMsg->message = WM_NULL;
		}
		if (pMsg->wParam == VK_END) {
			MoveXYZ(0, 0, (85000 - MO_ReadLogicPosition(2)));
			pMsg->message = WM_NULL;
		}
#endif
	}
	if (pMsg->message == WM_KEYUP) {
		if (pMsg->wParam == VK_LEFT || pMsg->wParam == VK_RIGHT
			|| pMsg->wParam == VK_UP || pMsg->wParam == VK_DOWN
			|| pMsg->wParam == VK_HOME || pMsg->wParam == VK_END) {
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
	return CDialogEx::PreTranslateMessage(pMsg);
}
/*XYZ鍵盤移動*/
void CCamera::MoveXYZ(int MoveX, int MoveY, int MoveZ) {
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
						if (!MO_ReadIsDriving(7))
							MO_Do3DLineMove(MoveX, MoveY, MoveZ, 80000, 1200000, 6000);
						break;
					case 2:
						if (!MO_ReadIsDriving(7))
							MO_Do3DLineMove(MoveX, MoveY, MoveZ, 50000, 800000, 5000);
						break;
					case 3:
						if (!MO_ReadIsDriving(7))
							MO_Do3DLineMove(MoveX, MoveY, MoveZ, 5000, 50000, 1000);
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
/*設置針頭offset*/
void CCamera::OnBnClickedButton1()
{        
	CString StrBuff;
	static LONG Point1X = 0;
	static LONG Point1Y = 0;
	GetDlgItemText(IDC_BUTTON1, StrBuff);
	CWnd* pMain = AfxGetApp()->m_pMainWnd;
#ifdef MOVE
	if (StrBuff == L"SetTipToCCD")
	{
		Point1X = MO_ReadLogicPosition(0);
		Point1Y = MO_ReadLogicPosition(1);
		SetDlgItemText(IDC_BUTTON1, _T("SetTipToCCD1"));
	}
	else if (StrBuff == L"SetTipToCCD1")
	{
		((CCommandTestDlg*)pMain)->TipOffset.x = Point1X - MO_ReadLogicPosition(0);
		((CCommandTestDlg*)pMain)->TipOffset.y = Point1Y - MO_ReadLogicPosition(1);
		SetDlgItemText(IDC_BUTTON1, _T("SetTipToCCD"));
		StrBuff.Format(_T("TipToCCD:X = %d,Y = %d"), ((CCommandTestDlg*)pMain)->TipOffset.x, ((CCommandTestDlg*)pMain)->TipOffset.y);
		SetDlgItemText(IDC_TIPTOCCD, StrBuff);
		((CCommandTestDlg*)pMain)->a.VisionDefault.VisionSet.AdjustOffsetX = ((CCommandTestDlg*)pMain)->TipOffset.x;
		((CCommandTestDlg*)pMain)->a.VisionDefault.VisionSet.AdjustOffsetY = ((CCommandTestDlg*)pMain)->TipOffset.y;
	}
	else
	{
		SetDlgItemText(IDC_BUTTON1, _T("SetTipToCCD"));
	}
#endif
#ifdef VI
	VI_SetCameraToTipOffset(((CCommandTestDlg*)pMain)->TipOffset.x, ((CCommandTestDlg*)pMain)->TipOffset.y);
#endif
}
/*解除極限*/
void CCamera::OnBnClickedButton2()
{
#ifdef  MOVE
	MO_AlarmCClean();
#endif
}
/*像素實際距離轉換*/
void CCamera::OnBnClickedButton3()
{
	CString StrBuff;
	static LONG Point1X = 0;
	static LONG Point1Y=0;
	CWnd* pMain = AfxGetApp()->m_pMainWnd;
	GetDlgItemText(IDC_BUTTON3,StrBuff);
#ifdef VI
	if (StrBuff == L"SetPixToPuls")
	{
		VI_DrawFOVFrame(2, GetDlgItem(IDC_PIC), 150, 150);
		SetDlgItemText(IDC_BUTTON3, _T("SetPixToPuls1"));     
	}
	else if (StrBuff == L"SetPixToPuls1")
	{
#ifdef MOVE
		Point1X = MO_ReadLogicPosition(0);
		Point1Y = MO_ReadLogicPosition(1);
#endif
		VI_CreateModelFromBox(2, GetDlgItem(IDC_PIC),MilModel, 150, 150);
		VI_DrawFOVFrame(3, GetDlgItem(IDC_PIC), 150, 150);
		SetDlgItemText(IDC_BUTTON3, _T("SetPixToPuls2"));
	}
	else if (StrBuff == L"SetPixToPuls2")
	{
		VI_SetPatternMatch(MilModel, 1, 1, 80, 0, 360);
#ifdef MOVE
		VI_SetPixelPulseRelation(GetDlgItem(IDC_PIC),MilModel, Point1X, Point1Y, MO_ReadLogicPosition(0), MO_ReadLogicPosition(1), ((CCommandTestDlg*)pMain)->PixToPulsX, ((CCommandTestDlg*)pMain)->PixToPulsY);
#endif
		if (((CCommandTestDlg*)pMain)->PixToPulsX != 0.0)
		{
			StrBuff.Format(_T("PixToPuls:X = %.6f,Y = %.6f"), ((CCommandTestDlg*)pMain)->PixToPulsX, ((CCommandTestDlg*)pMain)->PixToPulsY);
			SetDlgItemText(IDC_PIXTOPULS, StrBuff);
			VI_ModelFree(MilModel);
			//free(MilModel);
			*((int*)MilModel) = 0;
			SetDlgItemText(IDC_BUTTON3, _T("SetPixToPuls"));
			VI_DrawFOVFrame(1, GetDlgItem(IDC_PIC), 150, 150);

			//計算重組圖移動量
			VI_MosaicingMoveSet(((CCommandTestDlg*)pMain)->PixToPulsX * 640 / 1000, ((CCommandTestDlg*)pMain)->PixToPulsY * 480 / 1000, 50, ((CCommandTestDlg*)pMain)->a.AreaCheckParamterDefault.ViewMove.x, ((CCommandTestDlg*)pMain)->a.AreaCheckParamterDefault.ViewMove.y);
		}    
	}
	else
	{
		SetDlgItemText(IDC_BUTTON3, _T("SetPixToPuls"));
	}
#endif
}   
//對焦
void CCamera::OnBnClickedBtnfocus()
{
	CWnd* pMain = AfxGetApp()->m_pMainWnd;
#ifdef MOVE
	MO_Do3DLineMove(0, 0, ((CCommandTestDlg*)pMain)->a.VisionDefault.VisionSet.FocusHeight - MO_ReadLogicPosition(2), 30000, 100000, 5000);
#endif
}
//對焦點設置
void CCamera::OnBnClickedBtnfocusset()
{
	CString StrBuff;
	CWnd* pMain = AfxGetApp()->m_pMainWnd;
#ifdef MOVE
	((CCommandTestDlg*)pMain)->FocusPoint = MO_ReadLogicPosition(2);
	((CCommandTestDlg*)pMain)->a.VisionDefault.VisionSet.FocusHeight = ((CCommandTestDlg*)pMain)->FocusPoint;
	StrBuff.Format(_T("FocusHeight:%d"), ((CCommandTestDlg*)pMain)->FocusPoint);
	SetDlgItemText(IDC_FOCUSHEIGHT, StrBuff); 
#endif
	
}
//平台高度設置
void CCamera::OnBnClickedBtntableset()
{
    CString StrBuff;
    int TableZ = 0;
    CWnd* pMain = AfxGetApp()->m_pMainWnd;
    ((CCommandTestDlg*)pMain)->a.SetTabelZ(&TableZ);
    StrBuff.Format(L"TableZ:%d", TableZ);
    SetDlgItemText(IDC_TABLEZ, StrBuff);
}
//模組管理
void CCamera::OnBnClickedButton4()
{
	m_pCModel = new CModel();
	m_pCModel->Create(IDD_DIALOG3, this);
	m_pCModel->ShowWindow(SW_SHOW);
}
//關閉對話框
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
void CCamera::OnOK()
{
	//CDialogEx::OnOK();
}
//顯示視窗時設定
void CCamera::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);
	SetWindowLong(this->m_hWnd, GWL_EXSTYLE, GetWindowLong(this->m_hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);//設置視窗為可以透明化
	this->SetLayeredWindowAttributes(0, (255 * 100) / 100, LWA_ALPHA);//不透明
}
//非活動轉活動事件
int CCamera::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
{
	//從非活動轉為活動改成不透明
	this->SetLayeredWindowAttributes(0, (255 * 100) / 100, LWA_ALPHA);
	return CDialogEx::OnMouseActivate(pDesktopWnd, nHitTest, message);
}
//銷毀視窗事件
BOOL CCamera::DestroyWindow()
{
#ifdef VI
	//VI_DisplayAlloc(NULL, 1);
	if (*((int*)MilModel) == 0)
	{
		free(MilModel);
#ifdef PRINTF
		_cwprintf(_T("free"));
#endif
	}
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

