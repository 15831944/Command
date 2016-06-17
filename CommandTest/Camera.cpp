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
	PixToPulsX = 0.0;
	PixToPulsY = 0.0;
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
END_MESSAGE_MAP()

// CCamera 訊息處理常式

BOOL CCamera::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	/*初始化按鈕移動*/
	m_Xup.MoveX = 5000;
	m_Xdown.MoveX = -5000;
	m_Yup.MoveY = 5000;
	m_Ydown.MoveY = -5000;
	m_Zup.MoveZ = 5000;
	m_Zdown.MoveZ = -5000;
	/*初始化Radio*/
	((CButton *)GetDlgItem(IDC_RADH))->SetCheck(TRUE);
	RaiChoose = 1;
	CString StrBuff;
	StrBuff.Format(_T("%.6f,%.6f"), PixToPulsX, PixToPulsY);
	SetDlgItemText(IDC_PIXTOPULS, StrBuff);
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
		MO_Do3DLineMove(0, 0, GetDlgItemInt(IDC_EDITZ) - MO_ReadLogicPosition(2), 30000, 100000, 5000);
	}
	else if (GetDlgItemInt(IDC_EDITY) == 0 && GetDlgItemInt(IDC_EDITZ) == 0)
	{
		MO_Do3DLineMove(GetDlgItemInt(IDC_EDITX) - MO_ReadLogicPosition(0), 0, 0, 30000, 100000, 5000);
	}
	else if (GetDlgItemInt(IDC_EDITX) == 0 && GetDlgItemInt(IDC_EDITZ) == 0)
	{
		MO_Do3DLineMove(0, GetDlgItemInt(IDC_EDITY) - MO_ReadLogicPosition(1), 0, 30000, 100000, 5000);
	}
	else
	{
		MO_Do3DLineMove(GetDlgItemInt(IDC_EDITX) - MO_ReadLogicPosition(0), GetDlgItemInt(IDC_EDITY) - MO_ReadLogicPosition(1), GetDlgItemInt(IDC_EDITZ) - MO_ReadLogicPosition(2), 30000, 100000, 5000);
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
	StrBuff.Format(_T("%4d年%2d月%2d日%2d時%2d分%2d秒%2d"), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
	/*建立model*/
#ifdef VI
	void* Model;
	Model = malloc(sizeof(char));
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
	// TODO: 在此加入特定的程式碼和 (或) 呼叫基底類別
	if (pMsg->message == WM_KEYDOWN) {
		if (pMsg->wParam == VK_LEFT) {
			MoveXYZ(-10000, 0, 0);
			pMsg->message = WM_NULL;
		}
		if (pMsg->wParam == VK_RIGHT) {
			MoveXYZ(10000, 0, 0);
			pMsg->message = WM_NULL;
		}
		if (pMsg->wParam == VK_UP) {
			MoveXYZ(0, -10000, 0);
			pMsg->message = WM_NULL;
		}
		if (pMsg->wParam == VK_DOWN) {
			MoveXYZ(0, 10000, 0);
			pMsg->message = WM_NULL;
		}
		if (pMsg->wParam == VK_HOME) {
			MoveXYZ(0, 0, -10000);
			pMsg->message = WM_NULL;
		}
		if (pMsg->wParam == VK_END) {
			MoveXYZ(0, 0, 10000);
			pMsg->message = WM_NULL;
		}
		
	}
	if (pMsg->message == WM_KEYUP) {
		if (pMsg->wParam == VK_LEFT || pMsg->wParam == VK_RIGHT
			|| pMsg->wParam == VK_UP || pMsg->wParam == VK_DOWN
			|| pMsg->wParam == VK_HOME || pMsg->wParam == VK_END) {
#ifdef MOVE
			CWnd* pMain = AfxGetApp()->m_pMainWnd;
			if (((CCommandTestDlg*)pMain)->a.RunStatusRead.RunStatus == 0 || ((CCommandTestDlg*)pMain)->a.RunStatusRead.RunStatus == 2)
			{
				MO_DecSTOP();
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
	if (((CCommandTestDlg*)pMain)->a.RunStatusRead.RunStatus == 0 || ((CCommandTestDlg*)pMain)->a.RunStatusRead.RunStatus == 2)
	{
		switch (RaiChoose)
		{
		case 1:
			MO_Do3DLineMove(MoveX, MoveY, MoveZ, 30000, 100000, 6000);
			break;
		case 2:
			MO_Do3DLineMove(MoveX, MoveY, MoveZ, 15000, 80000, 5000);
			break;
		case 3:
			MO_Do3DLineMove(MoveX, MoveY, MoveZ, 5000, 50000, 1000);
			break;
		default:
			//MessageBox(_T("程式出現錯誤!"));
			break;
		}
		//_cprintf("進來了:%d", ((CCommandTestDlg*)pMain)->a.RunStatusRead.RunStatus);
	}
	//_cprintf("%d", ((CCommandTestDlg*)pMain)->a.RunStatusRead.RunStatus);
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
	GetDlgItemText(IDC_BUTTON3,StrBuff);
#ifdef VI
	if (StrBuff == L"SetPixToPuls")
	{
		VI_DrawFOVFrame(2, GetDlgItem(IDC_PIC), 150, 150);
		MilModel = malloc(sizeof(int));
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
		VI_SetPixelPulseRelation(GetDlgItem(IDC_PIC),MilModel, Point1X, Point1Y, MO_ReadLogicPosition(0), MO_ReadLogicPosition(1), PixToPulsX, PixToPulsY);
		if (PixToPulsX != 0.0)
		{
			StrBuff.Format(_T("PixToPuls:X = %.6f,Y = %.6f"), PixToPulsX, PixToPulsY);
			SetDlgItemText(IDC_PIXTOPULS, StrBuff);
			VI_ModelFree(MilModel);
			free(MilModel);
			SetDlgItemText(IDC_BUTTON3, _T("SetPixToPuls"));
			VI_DrawFOVFrame(1, GetDlgItem(IDC_PIC), 150, 150);
		}    
	}
	else
	{
		SetDlgItemText(IDC_BUTTON3, _T("SetPixToPuls"));
	}
#endif
}
void CCamera::OnBnClickedButton4()
{
	m_pCModel = new CModel();
	m_pCModel->Create(IDD_DIALOG3, this);
	m_pCModel->ShowWindow(SW_SHOW);
}
//關閉對話框
void CCamera::OnCancel()
{
#ifdef VI
	//VI_DisplayAlloc(NULL, 1);
#endif
	CDialogEx::OnCancel();
}
void CCamera::OnOK()
{
	//CDialogEx::OnOK();
}
