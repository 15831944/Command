// CCD.cpp : 實作檔
//

#include "stdafx.h"
#include "CommandTest.h"
#include "CCD.h"
#include "afxdialogex.h"
#include "CommandTestDlg.h"


// CCCD 對話方塊

IMPLEMENT_DYNAMIC(CCCD, CDialogEx)

CCCD::CCCD(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_CCD, pParent)
{
	m_TouchMoveThreadStop = FALSE;
	m_TouchMoveSwitch = TRUE;
}

CCCD::~CCCD()
{
}

void CCCD::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CCCD, CDialogEx)
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()



// CCCD 訊息處理常式
BOOL CCCD::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	// TODO:  在此加入額外的初始化

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX 屬性頁應傳回 FALSE
}
/*觸碰式移動*/
void CCCD::OnLButtonDown(UINT nFlags, CPoint point)
{
#ifdef VI
	CWnd* pMain = AfxGetApp()->m_pMainWnd;
	if (VI_GetMousePos(m_MeasPosX, m_MeasPosY) && m_TouchMoveSwitch)//取得座標
	{
		if (((CCommandTestDlg*)pMain)->a.RunStatusRead.RunStatus == 0 //判斷1.編譯狀態，永遠只有一個執行緒
			&& pTouchMoveThread == NULL//判斷2.觸碰移動執行續是否執行中
			&& ((CCommandTestDlg*)pMain)->a.RunStatusRead.GoHomeStatus == 1) //, 判斷3.GoHome是否完成
		{
			VI_GetTouchMoveDist(m_MeasPosX, m_MeasPosY, m_TouchReturnXPos, m_TouchReturnYPos);
			pTouchMoveThread = AfxBeginThread(TouchMove, this, THREAD_PRIORITY_ABOVE_NORMAL);
		}
	}
#endif
	CDialogEx::OnLButtonDown(nFlags, point);
}
/*觸碰式移動執行緒*/
UINT CCCD::TouchMove(LPVOID pParam)
{
#ifdef MOVE
	MO_Do3DLineMove((LONG)((CCCD*)pParam)->m_TouchReturnXPos, (LONG)((CCCD*)pParam)->m_TouchReturnYPos, 0, 70000, 100000, 10000);
	while (MO_ReadIsDriving(15))
	{
		if (((CCCD*)pParam)->m_TouchMoveThreadStop)    //按停止就中止執行緒
		{
			((CCCD*)pParam)->m_TouchMoveThreadStop = FALSE;
			MO_STOP();
		}
		Sleep(1);
	}
#endif
	pTouchMoveThread = NULL;
	return 0;
}
