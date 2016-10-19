
// CommandTest.cpp : �w�q���ε{�������O�欰�C
//

#include "stdafx.h"
#include "CommandTest.h"
#include "CommandTestDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CCommandTestApp

BEGIN_MESSAGE_MAP(CCommandTestApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CCommandTestApp �غc

CCommandTestApp::CCommandTestApp()
{
	// �䴩���s�Ұʺ޲z��
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: �b���[�J�غc�{���X�A
	// �N�Ҧ����n����l�]�w�[�J InitInstance ��
}


// �Ȧ����@�� CCommandTestApp ����

CCommandTestApp theApp;


// CCommandTestApp ��l�]�w

BOOL CCommandTestApp::InitInstance()
{
	// ���p���ε{����T�M����w�ϥ� ComCtl32.dll 6 (�t) �H�᪩���A
	// �ӱҰʵ�ı�Ƽ˦��A�b Windows XP �W�A�h�ݭn InitCommonControls()�C
	// �_�h����������إ߳��N���ѡC
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// �]�w�n�]�t�Ҧ��z�Q�n�Ω����ε{������
	// �q�α�����O�C
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	AfxEnableControlContainer();

	// �إߴ߼h�޲z���A�H����ܤ���]�t
	// ����߼h���˵��δ߼h�M���˵�����C
	CShellManager *pShellManager = new CShellManager;

	// �Ұ� [Windows ���] ��ı�ƺ޲z���i�ҥ� MFC ��������D�D
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// �зǪ�l�]�w
	// �p�G�z���ϥγo�ǥ\��åB�Q���
	// �̫᧹�����i�����ɤj�p�A�z�i�H
	// �q�U�C�{���X�������ݭn����l�Ʊ`���A
	// �ܧ��x�s�]�w�Ȫ��n�����X
	// TODO: �z���ӾA�׭ק惡�r��
	// (�Ҧp�A���q�W�٩β�´�W��)
	SetRegistryKey(_T("���� AppWizard �Ҳ��ͪ����ε{��"));

	CCommandTestDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: �b����m��ϥ� [�T�w] �Ӱ���ϥι�ܤ����
		// �B�z���{���X
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: �b����m��ϥ� [����] �Ӱ���ϥι�ܤ����
		#ifdef VI
		 VI_VisionFree();
		#endif
		// �B�z���{���X
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "ĵ�i: ��ܤ���إߥ��ѡA�]���A���ε{���N�~�פ�C\n");
		TRACE(traceAppMsg, 0, "ĵ�i: �p�G�z�n�b��ܤ���W�ϥ� MFC ����A�h�L�k #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS�C\n");
	}

	// �R���W���ҫإߪ��߼h�޲z���C
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

	// �]���w�g������ܤ���A�Ǧ^ FALSE�A�ҥH�ڭ̷|�������ε{���A
	// �ӫD���ܶ}�l���ε{�����T���C
	return FALSE;
}
//����IO����
BOOL CCommandTestApp::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_F1)
		{
		   ((CCommandTestDlg*)theApp.m_pMainWnd)->a.IODetectionSwitch(FALSE, 0);
		}
		else if (pMsg->wParam == VK_F2)
		{
			((CCommandTestDlg*)theApp.m_pMainWnd)->a.IODetectionSwitch(TRUE, 0);
		}
		else if (pMsg->wParam == VK_F3)
		{
			for (UINT i = 0; i < ((CCommandTestDlg*)theApp.m_pMainWnd)->a.mmCommandMemory.size(); i++)
			{
#ifdef PRINTF
				_cwprintf(L"%s\n", ((CCommandTestDlg*)theApp.m_pMainWnd)->a.mmCommandMemory.at(i));
#endif
			}
		}
		else if (pMsg->wParam == VK_F4)
		{
#ifdef MOVE
			for (UINT i = 0; i < ((CCommandTestDlg*)theApp.m_pMainWnd)->a.m_Action.LA_m_ptVec.size(); i++)
			{
#ifdef PRINTF
				_cwprintf(L"%d,%d,%d\n", ((CCommandTestDlg*)theApp.m_pMainWnd)->a.m_Action.LA_m_ptVec.at(i).EndPX,
					((CCommandTestDlg*)theApp.m_pMainWnd)->a.m_Action.LA_m_ptVec.at(i).EndPY,
					((CCommandTestDlg*)theApp.m_pMainWnd)->a.m_Action.LA_m_ptVec.at(i).EndPZ);
#endif
			}
#endif
		}  
	}
	return CWinApp::PreTranslateMessage(pMsg);
}
//�̽�F1Help��ܮ�
BOOL CCommandTestApp::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// TODO: �b���[�J�S�w���{���X�M (��) �I�s�����O
	return TRUE;
}
