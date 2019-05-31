
#include "stdafx.h"
#include "MyCombo.h"




CMyCombo::CMyCombo()
{
}


CMyCombo::~CMyCombo()
{
}


BEGIN_MESSAGE_MAP(CMyCombo, CComboBox)
	ON_WM_SETCURSOR()
	ON_WM_SETFOCUS()
	ON_CONTROL_REFLECT(CBN_EDITCHANGE, OnEditchange)
END_MESSAGE_MAP()





BOOL CMyCombo::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	return CComboBox::OnSetCursor(pWnd, nHitTest, message);
}




void CMyCombo::OnSetFocus(CWnd* pOldWnd) 
{
	CComboBox::OnSetFocus(pOldWnd);
}



void CMyCombo::OnEditchange() 
{
}
