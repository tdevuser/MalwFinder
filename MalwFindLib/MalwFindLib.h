// MalwFindLib.h : main header file for the MalwFindLib DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CMalwFindLibApp
// See MalwFindLib.cpp for the implementation of this class
//

class CMalwFindLibApp : public CWinApp
{
public:
	CMalwFindLibApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
