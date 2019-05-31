
#include "stdafx.h"
#include "MyListCtrl.h"
#include "MyCombo.h"
#include "SortClass.h"

#define COL_NUMBER    _T("No")
#define COL_ISENROLL  _T("Reg")
#define COL_NUM   1
#define COL_TEXT  32
#define IDC_CBO_FILENAME  5005


CMyListCtrl::CMyListCtrl() : m_bComboMode(false), m_bCheckAll(false), m_bAsc(true)
{
	m_nRow      = 0;
	m_nCol      = -1;
}

CMyListCtrl::~CMyListCtrl()
{
}


BEGIN_MESSAGE_MAP(CMyListCtrl, CListCtrl)
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnclick)
	ON_NOTIFY_REFLECT(NM_CLICK, OnClick)
END_MESSAGE_MAP()



void CMyListCtrl::SetColumnTextAlign(int nCol, int fmt)
{
	LV_COLUMN lvCol;
	lvCol.mask = LVCF_FMT;

	if (GetColumn(nCol, &lvCol))
	{
		lvCol.fmt = fmt;
		SetColumn(nCol, &lvCol);
	}
}


BOOL CMyListCtrl::AddItem(int nItem, int nSubItem, LPCSTR strText, int nImageIndex /*= -1*/)
{
	UNREFERENCED_PARAMETER( nImageIndex );

	int    nLength=0, nReturn=0, nError=0;
	TCHAR  szItemText[ MAX_PATH ];
	RtlZeroMemory( szItemText, MAX_PATH );

	nLength = (int)strlen( strText )+1;
	MultiByteToWideChar( CP_ACP, 0, strText, nLength, szItemText, MAX_PATH );
	
	LV_ITEM   lvit;
	lvit.mask     = LVIF_TEXT;
	lvit.iItem    = nItem;
	lvit.iSubItem = nSubItem;
	lvit.pszText  = szItemText;

	if(nImageIndex != -1)
	{
		lvit.mask |= LVIF_IMAGE;
		lvit.iImage = nImageIndex;
	}

	if(nSubItem == 0)
	{
		return InsertItem(&lvit);
	}

	return SetItem(&lvit);

}







BOOL CMyListCtrl::PreTranslateMessage(MSG* pMsg) 
{
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_TAB )
	{	
		if(GetKeyState(VK_SHIFT)>>8 == -1) //go backwards user is pressing SHIFT + TAB
		{
			m_nCol--;
			if(m_nCol < 0)
			{
				m_nCol = MAX_COLS;
				m_nRow--;

				if(m_nRow < 0)
				{
					m_nCol=0;
					m_nRow=0;				
				}
			}		
		}
		else //go forward, user is pressing TAB
		{
			m_nCol++;
			if(m_nCol > MAX_COLS )
			{
				m_nCol = 0;
				m_nRow++;
				
				if(m_nRow > MAX_ROWS-1)
				{					
					m_nCol = MAX_COLS;
					m_nRow = MAX_ROWS-1;					
				}
	
			}	
		}

		return 1;
	}
	
	return CListCtrl::PreTranslateMessage(pMsg);
}


void CMyListCtrl::OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	int  nItem=0, nSubItem=0, i=0, nMaxCount = 0;
	CHeaderCtrl*  pHeaderCtrl = NULL;
	HDITEM        HeaderItem;
	TCHAR         szBuffer[ MAX_PATH ];

	ASSERT( pResult );
	if(!pResult) return;

	nItem    = pNMListView->iItem;
	nSubItem = pNMListView->iSubItem;

	RtlZeroMemory( szBuffer, MAX_PATH );
	RtlZeroMemory( &HeaderItem, sizeof(HeaderItem) );
	pHeaderCtrl = GetHeaderCtrl();
	ASSERT( pHeaderCtrl );
	if(!pHeaderCtrl) return;
			
	HeaderItem.mask       = HDI_TEXT | HDI_IMAGE | HDI_HEIGHT;		
	HeaderItem.pszText    = szBuffer;
	HeaderItem.cchTextMax = MAX_PATH;		
	HeaderItem.iImage     = 0;
	HeaderItem.cxy        = 0;
	pHeaderCtrl->GetItem( pNMListView->iSubItem, &HeaderItem );

	if(!nSubItem)
	{
		if(m_bComboMode)
		{
			m_bCheckAll = !m_bCheckAll;		

			RtlZeroMemory( &HeaderItem, sizeof(HeaderItem) );
			HeaderItem.mask = HDI_IMAGE;
			if(!m_bCheckAll) HeaderItem.iImage = 0;
			else             HeaderItem.iImage = 1;

			pHeaderCtrl->SetItem( pNMListView->iSubItem, &HeaderItem );
			nMaxCount = GetItemCount();
			for(i=0; i<nMaxCount; i++) 
			{
				SetCheck( i, m_bCheckAll );
			}
		}
		else  // Å¬¸¯
		{
			m_bAsc = !m_bAsc;
			SortList( nSubItem, m_bAsc, true );
		}
	}
	else if(m_bComboMode && nSubItem == COL_NUM)
	{
		m_bAsc = !m_bAsc;
		SortList( nSubItem, m_bAsc, true );
	}
	else
	{
		m_bAsc = !m_bAsc;
		SortList( nSubItem, m_bAsc, false );
	}

	

	*pResult = 0;
}



void CMyListCtrl::AdjustWhenClickedOnPartialSqures(NMHDR* pNMHDR)
{
	
	char buf[123]={0x00};
	sprintf_s( buf, 123, "msg = %x",AfxGetThreadState()->m_lastSentMsg.wParam );
	

	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	char sBuf[123]={0x00};
	sprintf_s(sBuf, 123, "row = %d,col = %d",pNMListView->iItem,pNMListView->iSubItem);


	CRect rect;
	GetSubItemRect(pNMListView->iItem,pNMListView->iSubItem,LVIR_LABEL,rect);
	ClientToScreen(rect);
	CPoint pt(rect.right,rect.bottom);
	CRect rc;
	GetWindowRect(rc);

	//ScreenToClient(&rc);
	sprintf_s(sBuf, 123, "SB=%d,SR=%d,\nL=%d ,T=%d,B=%d,R=%d",rect.bottom,rect.right,rc.left,rc.top,rc.bottom,rc.right);
	
	if(!rc.PtInRect(pt))
	{			
		SendMessage(WM_VSCROLL,SB_LINEDOWN,0);
	}
	
	pt.x=rect.left;
	pt.y =rect.top;
	if(!rc.PtInRect(pt))
	{
		SendMessage(WM_VSCROLL,SB_LINEUP,0);
	}	
}




void CMyListCtrl::OnClick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	if(pNMListView->iItem == -1 || pNMListView->iSubItem == -1)
	{
		*pResult =0;
		return;
	}
	if(!m_bComboMode) return;

	AdjustWhenClickedOnPartialSqures(pNMHDR);

	m_nRow = pNMListView->iItem;
    m_nCol = pNMListView->iSubItem;	
	*pResult = 0;
}


BOOL CMyListCtrl::SortList( int nCol, bool bAsc, bool bNum/*=false*/ )
{
	SetFocus();
	CSortClass csc( this, nCol, bNum );
	csc.Sort( bAsc );
	return TRUE;
}








