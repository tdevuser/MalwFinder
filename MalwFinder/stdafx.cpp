
// stdafx.cpp : ǥ�� ���� ���ϸ� ��� �ִ� �ҽ� �����Դϴ�.
// MalwFinder.pch�� �̸� �����ϵ� ����� �˴ϴ�.
// stdafx.obj���� �̸� �����ϵ� ���� ������ ���Ե˴ϴ�.

#include "stdafx.h"


CFont  g_FontDefault;
HFONT  g_hFontObject;

void CreateGlobalFont(void)
{
	LOGFONT LogFont;
	g_hFontObject = (HFONT)::GetStockObject(DEFAULT_GUI_FONT);
	::GetObject(g_hFontObject, sizeof(LogFont), &LogFont);
	LogFont.lfHeight = 12;
	g_FontDefault.CreateFontIndirect(&LogFont);
}


CString GetSetupDirectory(void)
{
	TCHAR szFileName[_MAX_PATH] = _T("");
	TCHAR szDrive[_MAX_DRIVE] = _T("");
	TCHAR szDir[_MAX_DIR] = _T("");
	TCHAR szFName[_MAX_FNAME] = _T("");
	TCHAR szExt[_MAX_EXT] = _T("");

	GetModuleFileName(NULL, szFileName, _MAX_PATH);
	_tsplitpath_s(szFileName,
		szDrive, _MAX_DRIVE,
		szDir, _MAX_DIR,
		szFName, _MAX_FNAME,
		szExt, _MAX_EXT);

	CString strVal(CString(szDrive) + CString(szDir));
	INT nLen = strVal.GetLength();
	if (strVal.GetAt(nLen - 1) != _T('\\'))
	{
		strVal += _T("\\");
	}

	return strVal;

}
