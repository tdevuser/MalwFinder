#pragma once

class CMyListCtrl : public CListCtrl
{
public:
	CMyListCtrl();
	virtual ~CMyListCtrl();

private:
	bool  m_bCheckAll;

	// 오름차순인지 
	bool  m_bAsc;
public:
	bool        m_bComboMode;
	CImageList  m_HeaderImgList;

public:
	int        m_nRow;
	int        m_nCol;
	int        m_MaxRowsPerView;

public:
	void  SetColumnTextAlign(int nCol, int fmt);
	BOOL  AddItem(int nItem, int nSubItem, LPCSTR strItem, int nImageIndex = -1 );
	
	void  AdjustWhenClickedOnPartialSqures(NMHDR* pNMHDR);
	
	BOOL  SortList( int nCol, bool bAsc, bool bNum=false );

public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:	
	afx_msg void OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKeydown(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()

};


