
#include "stdafx.h"
#include "MalwFinder.h"
#include "MalwFinderDlg.h"


BEGIN_MESSAGE_MAP(CMalwFinderApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


CMalwFinderApp::CMalwFinderApp() : m_pDlg(NULL)
{
}


CMalwFinderApp theApp;


BOOL CMalwFinderApp::InitInstance()
{	
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();
	CreateGlobalFont();

	m_pDlg = new CMalwFinderDlg();
	if (m_pDlg)
	{
		m_pMainWnd = m_pDlg;
		INT_PTR nResponse = m_pDlg->DoModal();
		if (nResponse == IDOK)
		{
		}
		else if (nResponse == IDCANCEL)
		{
		}

		delete m_pDlg;
		m_pDlg = NULL;
	}

	return FALSE;
}



int CMalwFinderApp::ExitInstance()
{

	return CWinApp::ExitInstance();
}
