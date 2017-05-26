// CCD.cpp : ��@��
//

#include "stdafx.h"
#include "CommandTest.h"
#include "CCD.h"
#include "afxdialogex.h"
#include "CommandTestDlg.h"


// CCCD ��ܤ��

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



// CCCD �T���B�z�`��
BOOL CCCD::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	// TODO:  �b���[�J�B�~����l��

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX �ݩʭ����Ǧ^ FALSE
}
/*Ĳ�I������*/
void CCCD::OnLButtonDown(UINT nFlags, CPoint point)
{
#ifdef VI
	CWnd* pMain = AfxGetApp()->m_pMainWnd;
	if (VI_GetMousePos(m_MeasPosX, m_MeasPosY) && m_TouchMoveSwitch)//���o�y��
	{
		if (((CCommandTestDlg*)pMain)->a.RunStatusRead.RunStatus == 0 //�P�_1.�sĶ���A�A�û��u���@�Ӱ����
			&& pTouchMoveThread == NULL//�P�_2.Ĳ�I���ʰ�����O�_���椤
			&& ((CCommandTestDlg*)pMain)->a.RunStatusRead.GoHomeStatus == 1) //, �P�_3.GoHome�O�_����
		{
			VI_GetTouchMoveDist(m_MeasPosX, m_MeasPosY, m_TouchReturnXPos, m_TouchReturnYPos);
			pTouchMoveThread = AfxBeginThread(TouchMove, this, THREAD_PRIORITY_ABOVE_NORMAL);
		}
	}
#endif
	CDialogEx::OnLButtonDown(nFlags, point);
}
/*Ĳ�I�����ʰ����*/
UINT CCCD::TouchMove(LPVOID pParam)
{
#ifdef MOVE
	MO_Do3DLineMove((LONG)((CCCD*)pParam)->m_TouchReturnXPos, (LONG)((CCCD*)pParam)->m_TouchReturnYPos, 0, 70000, 100000, 10000);
	while (MO_ReadIsDriving(15))
	{
		if (((CCCD*)pParam)->m_TouchMoveThreadStop)    //������N��������
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
