// Model.cpp : ��@��
//

#include "stdafx.h"
#include "CommandTest.h"
#include "Model.h"
#include "afxdialogex.h"


// CModel ��ܤ��

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

// CModel �T���B�z�`��
BOOL CModel::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	m_ModelList.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
	m_ModelList.InsertColumn(0, _T("�s��"), LVCFMT_CENTER, 36, -1);
	m_ModelList.InsertColumn(1, _T("�ɮצW��"), LVCFMT_LEFT, 230, -1);
	OnBnClickedButton1();
	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX �ݩʭ����Ǧ^ FALSE
}
/*���*/
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
	_cprintf("%d", iSelPos);
#ifdef VI
	VI_LoadModel(MilModel, lpszText, AllModelName.at(iSelPos));
	VI_SetPatternMatch(MilModel, 1, 1, 80, 0, 360);
	VI_FindMark(MilModel, OffsetX, OffsetY);
	VI_ModelFree(MilModel);
	free(MilModel);
#endif
#ifdef MOVE
	MO_Do3DLineMove(OffsetX, OffsetY, 0, 30000, 100000, 6000);
#endif
	//CDialogEx::OnOK();

}
/*���}*/
void CModel::OnCancel()
{
	CDialogEx::OnCancel();
}    
/*�j�M�ɮצW*/
BOOL CModel::ListAllFileInDirectory(LPTSTR szPath, LPTSTR szName) {
	HANDLE hListFile;
	TCHAR szFilePath[MAX_PATH]; //�ɮצW 
	TCHAR szFullPath[MAX_PATH];	//�ɮץؿ� 
	WIN32_FIND_DATA FindFileData;

	lstrcpy(szFilePath, szPath);
	lstrcat(szFilePath, szName);

	hListFile = FindFirstFile(szFilePath, &FindFileData);
	if (hListFile == INVALID_HANDLE_VALUE) {
		MessageBox(_T("�L�ɮ�"));
		return 1;
	}
	else {
		do {
			if (lstrcmp(FindFileData.cFileName, TEXT(".")) == 0 ||
				lstrcmp(FindFileData.cFileName, TEXT("..")) == 0)
				continue;
			wsprintf(szFullPath, L"%s\\%s", szPath, FindFileData.cFileName);//�NszPath�MFindFileData.cFileName �r��ۥ[���szFullPath�� 
			/*�W�ٰ}�C*/
			AllModelName.push_back(FindFileData.cFileName);
			/*�C�����*/
			m_ModelList.InsertItem(ModelCount, NULL);
			CString StrBuff;
			(ModelCount>9) ? StrBuff.Format(_T("0%d"), ModelCount) : StrBuff.Format(_T("00%d"), ModelCount);
			m_ModelList.SetItemText(ModelCount, 0, StrBuff);
			m_ModelList.SetItemText(ModelCount, 1, AllModelName.at(ModelCount).Left(AllModelName.at(ModelCount).Find(_T("."))));
			/*�ɮ׭p��*/
			ModelCount++;

			if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {//�p�G�ؿ��U�٦��ؿ��~�򩹤U�M��
																		   //printf("<DIR>");
				ListAllFileInDirectory(szFullPath, szName);
			}
		} while (FindNextFile(hListFile, &FindFileData));
	}
	return 0;
}
/*��s�ɮ�*/
void CModel::OnBnClickedButton1()
{
	ModelCount = 0;
	//�R���Ҧ��C��
	m_ModelList.DeleteAllItems();
	//����exe�ؿ�
	CString path;
	GetModuleFileName(NULL, path.GetBufferSetLength(MAX_PATH + 1), MAX_PATH);
	path.ReleaseBuffer();
	int pos = path.ReverseFind('\\');
	path = path.Left(pos) + _T("\\Temp\\");
	LPTSTR lpszText = new TCHAR[path.GetLength() + 1];
	lstrcpy(lpszText, path);
	//�}�C�M��
	AllModelName.clear();
	//�j�M�ɮ�
	ListAllFileInDirectory(lpszText, TEXT("*�~*��*��*��*��*��*.mod"));
}
/*�C���I��k���*/
void CModel::OnNMRClickList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	CListCtrl *CList = (CListCtrl *)GetDlgItem(IDC_LIST1);
	CMenu menu, *pSubMenu;
	CPoint CurPnt;
	int ItemCount = m_ModelList.GetItemCount();//��������`��
	NM_LISTVIEW  *pEditCtrl = (NM_LISTVIEW *)pNMHDR;
	if (pEditCtrl->iItem != -1 || pEditCtrl->iSubItem != 0) {
		menu.LoadMenu(IDR_MENU2);//�[�J���
		pSubMenu = menu.GetSubMenu(0);
		GetCursorPos(&CurPnt);
		pSubMenu->TrackPopupMenu(TPM_LEFTALIGN, CurPnt.x, CurPnt.y, this);//�I�k��X�{������m
	}
	*pResult = 0;
}
/*�R���ɮ�*/
void CModel::OnDeletefile()
{        
	//����exe�ؿ�
	CString path;
	GetModuleFileName(NULL, path.GetBufferSetLength(MAX_PATH + 1), MAX_PATH);
	path.ReleaseBuffer();
	int pos = path.ReverseFind('\\');
	path = path.Left(pos) + _T("\\Temp\\");
	LPTSTR lpszText = new TCHAR[path.GetLength() + 1];
	lstrcpy(lpszText, path);
	int iSelPos = m_ModelList.GetNextItem(-1, LVIS_SELECTED);
	DeleteFile(lpszText + AllModelName.at(iSelPos).Left(AllModelName.at(iSelPos).Find(_T("."))) + _T(".mod"));//�R��.mod��
	DeleteFile(lpszText + AllModelName.at(iSelPos).Left(AllModelName.at(iSelPos).Find(_T("."))) + _T(".bmp"));//�R��.bmp��
	AllModelName.erase(AllModelName.begin() + iSelPos);
	OnBnClickedButton1();
}
