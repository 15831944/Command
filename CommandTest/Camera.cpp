// Camera.cpp : 實作檔
//

#include "stdafx.h"
#include "CommandTest.h"
#include "Camera.h"
#include "afxdialogex.h"
#define EPOCHFILETIME   (116444736000000000UL)

// CCamera 對話方塊

IMPLEMENT_DYNAMIC(CCamera, CDialogEx)

CCamera::CCamera(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG2, pParent)
{
    RaiChoose = 1;
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
    MO_Do3DLineMove(GetDlgItemInt(IDC_EDITX), GetDlgItemInt(IDC_EDITY), GetDlgItemInt(IDC_EDITZ), 30000, 100000, 5000);
#endif

}
//關閉對話框
void CCamera::OnCancel()
{
#ifdef VI
    //VI_DisplayAlloc(NULL, 1);
#endif
    CDialogEx::OnCancel();
}
//建立model
void CCamera::OnBnClickedBtnmodel()
{
    /*獲取目前.exe目錄*/
    CString path;
    GetModuleFileName(NULL, path.GetBufferSetLength(MAX_PATH + 1), MAX_PATH);
    path.ReleaseBuffer();
    int pos = path.ReverseFind('\\');
    path = path.Left(pos) + _T("\\Model");
    LPTSTR lpszText = new TCHAR[path.GetLength() + 1];
    lstrcpy(lpszText, path);
    /*路径不存在则创建该路径*/
    CFileFind m_FileFind;
    if (!m_FileFind.FindFile(path))
    {
        CreateDirectory(path, NULL);
    }
    path = path.Left(pos) + _T("\\Model\\");
    MessageBox(path);
    /*獲取現在時間*/
    SYSTEMTIME st;
    GetLocalTime(&st);
    CString StrBuff;
    StrBuff.Format(_T("%4d%2d%2d%2d%2d%2d%2d"), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
    /*建立model*/
#ifdef VI
    void* Model;
    Model = malloc(sizeof(char));
    VI_CreateModelFromBox(1, GetDlgItem(IDC_PIC), Model, 150, 150);
    VI_SaveModel(Model, path, StrBuff + _T(".Model"));
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
            MoveXYZ(-5000, 0, 0);
        }
        if (pMsg->wParam == VK_RIGHT) {
            MoveXYZ(5000, 0, 0);
        }
        if (pMsg->wParam == VK_UP) {
            MoveXYZ(0, -5000, 0);
        }
        if (pMsg->wParam == VK_DOWN) {
            MoveXYZ(0, 5000, 0);
        }
        if (pMsg->wParam == VK_HOME) {
            MoveXYZ(0, 0, -5000);
        }
        if (pMsg->wParam == VK_END) {
            MoveXYZ(0, 0, 5000);
        }
    }
    if (pMsg->message == WM_KEYUP) {
        if (pMsg->wParam == VK_LEFT || pMsg->wParam == VK_RIGHT
            || pMsg->wParam == VK_UP || pMsg->wParam == VK_DOWN
            || pMsg->wParam == VK_HOME || pMsg->wParam == VK_END) {
#ifdef MOVE
            MO_DecSTOP();
#endif // MOVE
        }
    }
    return CDialogEx::PreTranslateMessage(pMsg);
}
/*XYZ鍵盤移動*/
void CCamera::MoveXYZ(int MoveX, int MoveY, int MoveZ) {
#ifdef MOVE
    switch (RaiChoose)
    {
    case 1:
        MO_Do3DLineMove(MoveX, MoveY, MoveZ, 30000, 100000, 5000);
        break;
    case 2:
        MO_Do3DLineMove(MoveX, MoveY, MoveZ, 15000, 80000, 5000);
        break;
    case 3:
        MO_Do3DLineMove(MoveX, MoveY, MoveZ, 5000, 60000, 1000);
        break;
    default:
        //MessageBox(_T("程式出現錯誤!"));
        break;
    }
#endif // MOVE
}
