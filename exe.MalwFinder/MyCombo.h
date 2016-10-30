#pragma once




	class CMyCombo : public CComboBox
	{
	public:
		CMyCombo();
		virtual ~CMyCombo();

	protected:
		afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
		afx_msg void OnSetFocus(CWnd* pOldWnd);
		afx_msg void OnEditchange();


		DECLARE_MESSAGE_MAP()


	};


