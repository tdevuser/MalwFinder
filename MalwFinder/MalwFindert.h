
// NHCAFltTest.h : PROJECT_NAME ���� ���α׷��� ���� �� ��� �����Դϴ�.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"		// �� ��ȣ�Դϴ�.

#include "NHCAFltTestDlg.h"


class CNHCAFltTestApp : public CWinApp
{
public:
	CNHCAFltTestApp();

public:
	CNHCAFltTestDlg  m_dlg;

// �������Դϴ�.
public:
	virtual BOOL InitInstance();

// �����Դϴ�.

	DECLARE_MESSAGE_MAP()
	virtual int ExitInstance();
};

extern CNHCAFltTestApp theApp;