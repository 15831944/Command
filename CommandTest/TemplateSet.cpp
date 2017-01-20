// TemplateSet.cpp : 實作檔
//

#include "stdafx.h"
#include "CommandTest.h"
#include "TemplateSet.h"
#include "afxdialogex.h"

#include "CommandTestDlg.h"

// CTemplateSet 對話方塊

IMPLEMENT_DYNAMIC(CTemplateSet, CDialogEx)

CTemplateSet::CTemplateSet(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG11, pParent)
{
	pMain = AfxGetApp()->m_pMainWnd;
}

CTemplateSet::~CTemplateSet()
{
}

void CTemplateSet::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LISTBOX1, m_ListOK);
	DDX_Control(pDX, IDC_LISTBOX2, m_ListNG);
}


BEGIN_MESSAGE_MAP(CTemplateSet, CDialogEx)
	ON_BN_CLICKED(IDOK, &CTemplateSet::OnBnClickedOk)
END_MESSAGE_MAP()


// CTemplateSet 訊息處理常式


BOOL CTemplateSet::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	//找到該exe目錄
	CString path;
	GetModuleFileName(NULL, path.GetBufferSetLength(MAX_PATH + 1), MAX_PATH);
	path.ReleaseBuffer();
	int pos = path.ReverseFind('\\');
	path = path.Left(pos) + _T("\\Temp\\");
	LPTSTR lpszText = new TCHAR[path.GetLength() + 1];
	lstrcpy(lpszText, path);
	//搜尋檔案
	ListAllFileInDirectory(lpszText, TEXT("*_*_*_*_*.mod"));
	//RAD初始化設定
	((CButton *)GetDlgItem(IDC_RADIMOPEN))->SetCheck(TRUE);
	((CButton *)GetDlgItem(IDC_RADAREAOPEN))->SetCheck(TRUE);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX 屬性頁應傳回 FALSE
}
/*搜尋檔案名*/
BOOL CTemplateSet::ListAllFileInDirectory(LPTSTR szPath, LPTSTR szName) {
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
			m_ListOK.AddString(FindFileData.cFileName);
			m_ListNG.AddString(FindFileData.cFileName);
			if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {//如果目錄下還有目錄繼續往下尋找
																		   //printf("<DIR>");
				ListAllFileInDirectory(szFullPath, szName);
			}
		} while (FindNextFile(hListFile, &FindFileData));
	}
	return 0;
}
/*確定*/
void CTemplateSet::OnBnClickedOk()
{   
	CString StrBuff = L"";
	((CCommandTestDlg*)pMain)->CheckStrBuff = L"TemplateCheck,";
	int okSel = m_ListOK.GetSelCount();
	int ngSel = m_ListNG.GetSelCount();
	CArray< int, int& > ListSelOK,ListSelNG;
	ListSelOK.SetSize(okSel);
	ListSelNG.SetSize(ngSel);
	m_ListOK.GetSelItems(okSel, ListSelOK.GetData());
	m_ListNG.GetSelItems(ngSel, ListSelNG.GetData());
	if (((CButton *)GetDlgItem(IDC_RADIMOPEN))->GetCheck())
		((CCommandTestDlg*)pMain)->CheckStrBuff += L"1,";
	else
		((CCommandTestDlg*)pMain)->CheckStrBuff += L"0,";
	if (((CButton *)GetDlgItem(IDC_RADAREAOPEN))->GetCheck())
		((CCommandTestDlg*)pMain)->CheckStrBuff += L"1,";
	else
		((CCommandTestDlg*)pMain)->CheckStrBuff += L"0,";
    if (okSel == 0)
    {
        ((CCommandTestDlg*)pMain)->CheckStrBuff += L"0,";
    }
    else
    {
        for (int i = 0; i < okSel; i++)
        {
            StrBuff.Format(L"%d", ListSelOK[i] + 1);
            if (i < okSel - 1)
            {
                StrBuff += L">";
            }
            else
            {
                StrBuff += L",";
            }
            ((CCommandTestDlg*)pMain)->CheckStrBuff += StrBuff;
        }
    }
    if (ngSel == 0)
    {
        ((CCommandTestDlg*)pMain)->CheckStrBuff += L"0";
    }
    else
    {
        for (int i = 0; i < ngSel; i++)
        {
            StrBuff.Format(L"%d", ListSelNG[i] + 1);
            if (i < ngSel - 1)
            {
                StrBuff += L">";
            }
            ((CCommandTestDlg*)pMain)->CheckStrBuff += StrBuff;
        }
    }	
    StrBuff.Format(L",%d", GetDlgItemInt(IDC_EDITSCORE));
    ((CCommandTestDlg*)pMain)->CheckStrBuff += StrBuff;
	CDialogEx::OnOK();
}
