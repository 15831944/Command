// PictureViewDlg.cpp : 實作檔
//

#include "stdafx.h"
#include "CommandTest.h"
#include "PictureViewDlg.h"
#include "afxdialogex.h"


// CPictureViewDlg 對話方塊

IMPLEMENT_DYNAMIC(CPictureViewDlg, CDialogEx)

CPictureViewDlg::CPictureViewDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG13, pParent)
{
    FileName = L"";
    FilePath = L"";
}
CPictureViewDlg::~CPictureViewDlg()
{
}
void CPictureViewDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CPictureViewDlg, CDialogEx)
END_MESSAGE_MAP()

// CPictureViewDlg 訊息處理常式
BOOL CPictureViewDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();
    CRect rcClient, rcWindow;
    if (!FileExist(FilePath + FileName + _T(".bmp")))
    {
        return FALSE;
    }
    CImage image;
    image.Load(FilePath + FileName + _T(".bmp"));
    HBITMAP hBmp = image.Detach();
    BITMAP bitmap;
    GetObject(hBmp, sizeof(BITMAP), &bitmap);
    //讀取系統解析度,判斷式窗大小
    if (bitmap.bmHeight > GetSystemMetrics(SM_CYSCREEN))
        bitmap.bmHeight = GetSystemMetrics(SM_CYSCREEN) - 100;
    if (bitmap.bmWidth > GetSystemMetrics(SM_CXSCREEN))
        bitmap.bmWidth = GetSystemMetrics(SM_CXSCREEN) - 100;
    GetClientRect(rcClient);
    GetWindowRect(rcWindow);
    //MoveWindow((GetSystemMetrics(SM_CXSCREEN) / 2) - (bitmap.bmWidth / 2), (GetSystemMetrics(SM_CYSCREEN) / 2) - (bitmap.bmHeight / 2), bitmap.bmWidth, bitmap.bmHeight);//未修正視窗
    MoveWindow((GetSystemMetrics(SM_CXSCREEN) / 2) - (bitmap.bmWidth / 2), (GetSystemMetrics(SM_CYSCREEN) / 2) - (bitmap.bmHeight / 2),
        bitmap.bmWidth + (rcWindow.right - rcWindow.left - rcClient.right), bitmap.bmHeight + (rcWindow.bottom - rcWindow.top - rcClient.bottom));//修正後視窗
#ifdef VI
    ShowResult = VI_ShowPic(FilePath, FileName, this->m_hWnd);
#endif
    return TRUE;
}
//取消
void CPictureViewDlg::OnCancel()
{
#ifdef VI
    if (ShowResult)
    {
        VI_AreaCheckDispFree();
    }
#endif  
    CDialogEx::OnCancel();
    DestroyWindow();
}
//確定
void CPictureViewDlg::OnOK()
{
#ifdef VI
    if (ShowResult)
    {
        VI_AreaCheckDispFree();
    }
#endif 
    CDialogEx::OnOK();
    DestroyWindow();
}
//銷毀視窗
BOOL CPictureViewDlg::DestroyWindow()
{
    return CDialogEx::DestroyWindow();
}
/*判斷檔案是否存在
*FilePathName:檔案路徑名稱
*/
BOOL CPictureViewDlg::FileExist(LPCWSTR FilePathName)
{
    HANDLE hFile;
    WIN32_FIND_DATA FindData;
    hFile = FindFirstFile(FilePathName, &FindData);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        CloseHandle(hFile);
        return FALSE;
    }
    return TRUE;
}