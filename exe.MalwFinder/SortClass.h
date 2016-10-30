

#pragma once


class CSortClass  
{
public:
	CSortClass(CListCtrl * _pWnd, const int nCol, const BOOL bIsNumeric);
	virtual ~CSortClass();
	
	int m_nCol;
	CListCtrl* pWnd;

	BOOL  m_bIsNumeric;

	void Sort(const BOOL bAsc);
	
	static int CALLBACK CompareAsc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
	static int CALLBACK CompareDes(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
	
	static int CALLBACK CompareAscI(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
	static int CALLBACK CompareDesI(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
	
public:
	class CSortItem
	{
	public:
		virtual  ~CSortItem();
		CSortItem(const DWORD _dw, const CString &_txt);
		CString txt;
		DWORD dw;
	};

	class CSortItemInt
	{
	public:
		CSortItemInt(const DWORD _dw, const CString &_txt);
		int iInt ;
		DWORD dw;
	};
};
