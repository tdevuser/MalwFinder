
// MalwFinder.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols
#include "MalwFinderDlg.h"


// CMalwFinderApp:
// See MalwFinder.cpp for the implementation of this class
//

class CMalwFinderApp : public CWinApp
{
public:
	CMalwFinderApp();

// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementation

	CMalwFinderDlg* m_pDlg;

	DECLARE_MESSAGE_MAP()
};

extern CMalwFinderApp theApp;