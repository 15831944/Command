// Model.cpp : 實作檔
//

#include "stdafx.h"
#include "CommandTest.h"
#include "Model.h"
#include "afxdialogex.h"


// CModel 對話方塊

IMPLEMENT_DYNAMIC(CModel, CDialogEx)

CModel::CModel(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG3, pParent)
{
	ModelCount = 0;
}

CModel::~CModel()
{
}

void CModel::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_ModelList);
}

BEGIN_MESSAGE_MAP(CModel, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON1, &CModel::OnBnClickedButton1)
	ON_NOTIFY(NM_RCLICK, IDC_LIST1, &CModel::OnNMRClickList1)
	ON_COMMAND(IDM_DELETEFILE, &CModel::OnDeletefile)
END_MESSAGE_MAP()

// CModel 訊息處理常式
BOOL CModel::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	m_ModelList.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
	m_ModelList.InsertColumn(0, _T("編號"), LVCFMT_CENTER, 36, -1);
	m_ModelList.InsertColumn(1, _T("檔案名稱"), LVCFMT_LEFT, 230, -1);
	OnBnClickedButton1();
	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX 屬性頁應傳回 FALSE
}
/*比對*/
void CModel::OnOK()
{   
	void* MilModel;
	CString path;
	DOUBLE OffsetX = 0;
	DOUBLE OffsetY = 0;
	MilModel = malloc(sizeof(int));
	GetModuleFileName(NULL, path.GetBufferSetLength(MAX_PATH + 1), MAX_PATH);
	path.ReleaseBuffer();
	int pos = path.ReverseFind('\\');
	path = path.Left(pos) + _T("\\Temp\\");
	LPTSTR lpszText = new TCHAR[path.GetLength() + 1];
	lstrcpy(lpszText, path);
	int iSelPos = m_ModelList.GetNextItem(-1, LVIS_SELECTED);
#ifdef PRINTF
	_cprintf("%d", iSelPos);
#endif
#ifdef VI
	if (iSelPos != -1)
	{
		VI_LoadModel(MilModel, lpszText, AllModelName.at(iSelPos));
		VI_SetPatternMatch(MilModel, 1, 1, 80, 0, 360);
		VI_FindMark(MilModel, OffsetX, OffsetY);
		VI_ModelFree(MilModel);
		free(MilModel);
	}
#endif
#ifdef MOVE
	MO_Do3DLineMove((LONG)OffsetX, (LONG)OffsetY, 0, 30000, 100000, 6000);
#endif
    if (lpszText != NULL)
        delete lpszText;
	//CDialogEx::OnOK();
}
/*離開*/
void CModel::OnCancel()
{
	CDialogEx::OnCancel();
}    
/*搜尋檔案名*/
BOOL CModel::ListAllFileInDirectory(LPTSTR szPath, LPTSTR szName) {
	HANDLE hListFile;
	TCHAR szFilePath[MAX_PATH]; //檔案名 
	TCHAR szFullPath[MAX_PATH];	//檔案目錄 
	WIN32_FIND_DATA FindFileData;

	lstrcpy(szFilePath, szPath);
	lstrcat(szFilePath, szName);

	hListFile = FindFirstFile(szFilePath, &FindFileData);
	if (hListFile == INVALID_HANDLE_VALUE) {
		MessageBox(_T("無檔案"));
		return 1;
	}
	else {
		do {
			if (lstrcmp(FindFileData.cFileName, TEXT(".")) == 0 ||
				lstrcmp(FindFileData.cFileName, TEXT("..")) == 0)
				continue;
			wsprintf(szFullPath, L"%s\\%s", szPath, FindFileData.cFileName);//將szPath和FindFileData.cFileName 字串相加放到szFullPath裡 
			/*名稱陣列*/
			AllModelName.push_back(FindFileData.cFileName);
			/*列表顯示*/
			m_ModelList.InsertItem(ModelCount, NULL);
			CString StrBuff;
			(ModelCount>9) ? StrBuff.Format(_T("0%d"), ModelCount) : StrBuff.Format(_T("00%d"), ModelCount);
			m_ModelList.SetItemText(ModelCount, 0, StrBuff);
			m_ModelList.SetItemText(ModelCount, 1, AllModelName.at(ModelCount).Left(AllModelName.at(ModelCount).Find(_T("."))));
			/*檔案計數*/
			ModelCount++;

			if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {//如果目錄下還有目錄繼續往下尋找
																		   //printf("<DIR>");
				ListAllFileInDirectory(szFullPath, szName);
			}
		} while (FindNextFile(hListFile, &FindFileData));
	}
	return 0;
}
/*刷新檔案*/
void CModel::OnBnClickedButton1()
{
	ModelCount = 0;
	//刪除所有列表
	m_ModelList.DeleteAllItems();
	//找到該exe目錄
	CString path;
	GetModuleFileName(NULL, path.GetBufferSetLength(MAX_PATH + 1), MAX_PATH);
	path.ReleaseBuffer();
	int pos = path.ReverseFind('\\');
	path = path.Left(pos) + _T("\\Temp\\");
	LPTSTR lpszText = new TCHAR[path.GetLength() + 1];
	lstrcpy(lpszText, path);
	//陣列清除
	AllModelName.clear();
	//搜尋檔案
	ListAllFileInDirectory(lpszText, TEXT("*_*_*_*_*.mod"));
    if (lpszText != NULL)
        delete lpszText;
}
/*列表點選右鍵時*/
void CModel::OnNMRClickList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	CListCtrl *CList = (CListCtrl *)GetDlgItem(IDC_LIST1);
	CMenu menu, *pSubMenu;
	CPoint CurPnt;
	int ItemCount = m_ModelList.GetItemCount();//獲取項目總數
	NM_LISTVIEW  *pEditCtrl = (NM_LISTVIEW *)pNMHDR;
	if (pEditCtrl->iItem != -1 || pEditCtrl->iSubItem != 0) {
		menu.LoadMenu(IDR_MENU2);//加入菜單
		pSubMenu = menu.GetSubMenu(0);
		GetCursorPos(&CurPnt);
		pSubMenu->TrackPopupMenu(TPM_LEFTALIGN, CurPnt.x, CurPnt.y, this);//點右鍵出現的菜單位置
	}
	*pResult = 0;
}
/*刪除檔案*/
void CModel::OnDeletefile()
{        
	//找到該exe目錄
	CString path;
	GetModuleFileName(NULL, path.GetBufferSetLength(MAX_PATH + 1), MAX_PATH);
	path.ReleaseBuffer();
	int pos = path.ReverseFind('\\');
	path = path.Left(pos) + _T("\\Temp\\");
	LPTSTR lpszText = new TCHAR[path.GetLength() + 1];
	lstrcpy(lpszText, path);
	int iSelPos = m_ModelList.GetNextItem(-1, LVIS_SELECTED);
	DeleteFile(lpszText + AllModelName.at(iSelPos).Left(AllModelName.at(iSelPos).Find(_T("."))) + _T(".mod"));//刪除.mod檔
	DeleteFile(lpszText + AllModelName.at(iSelPos).Left(AllModelName.at(iSelPos).Find(_T("."))) + _T(".bmp"));//刪除.bmp檔
	AllModelName.erase(AllModelName.begin() + iSelPos);
	OnBnClickedButton1();
    if (lpszText != NULL)
        delete lpszText;
}
