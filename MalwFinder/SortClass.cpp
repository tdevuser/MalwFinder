
#include "stdafx.h"
#include "SortClass.h"
// #include <intsafe.h>



CSortClass::CSortClass(CListCtrl * _pWnd, const int nCol, const BOOL bIsNumeric)
{
	ASSERT(pWnd);

	DWORD  dw = 0x00;
	DWORD_PTR dwPtrData = 0x00;
	CString txt;

	m_nCol = nCol;
	pWnd   = _pWnd;

	m_bIsNumeric = bIsNumeric;
	
	int max = 0;
	if(pWnd)
	{	
		max = pWnd->GetItemCount();
	}
	
	if(m_bIsNumeric)
	{
		for (int t = 0; t < max; t++)
		{
			dwPtrData = pWnd->GetItemData( t );

			dw = (DWORD)dwPtrData;
			// DWordPtrToDWord(dwPtrData, &dw);

			txt = pWnd->GetItemText(t, m_nCol);
			pWnd->SetItemData(t, (DWORD)new CSortItemInt(dw, txt));
		}
	}
	else
	{
		for(int t = 0; t < max; t++)
		{
			dwPtrData = pWnd->GetItemData(t);
			dw = (DWORD)dwPtrData;

			// DWordPtrToDWord(dwPtrData, &dw);

			txt = pWnd->GetItemText(t, m_nCol);
			pWnd->SetItemData(t, (DWORD) new CSortItem(dw, txt));
		}
	}
}

CSortClass::~CSortClass()
{
	ASSERT(pWnd);
	int max = 0;

	if(pWnd)
	{	
		max = pWnd->GetItemCount();
	}
	
	if(m_bIsNumeric)
	{
		CSortItemInt* pItem = NULL;
		for(int t = 0; t < max; t++)
		{
			pItem = (CSortItemInt *) pWnd->GetItemData(t);
			ASSERT(pItem);

			if(pWnd && pItem)
			{
				pWnd->SetItemData(t, pItem->dw);
				delete pItem;
				pItem = NULL;
			}

		}
	}
	else
	{
		CSortItem* pItem = NULL;
		for(int t = 0; t<max; t++)
		{
			pItem = (CSortItem *) pWnd->GetItemData(t);
			ASSERT(pItem);

			if(pItem)
			{
				pWnd->SetItemData(t, pItem->dw);
				delete pItem;
				pItem = NULL;
			}
		}
	}
}

void CSortClass::Sort(const BOOL bAsc)
{
	if(m_bIsNumeric)
	{
		if(bAsc)
		{
			pWnd->SortItems(CompareAscI, 0L);
		}
		else
		{
			pWnd->SortItems(CompareDesI, 0L);
		}

	}
	else
	{
		if(bAsc)
		{
			pWnd->SortItems(CompareAsc, 0L);
		}
		else
		{
			pWnd->SortItems(CompareDes, 0L);
		}

	}
}

int CALLBACK CSortClass::CompareAsc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	CSortItem * i1 = (CSortItem *) lParam1;
	CSortItem * i2 = (CSortItem *) lParam2;
	ASSERT(i1 && i2);

	if(i1 && i2)
	{
		return i1->txt.CompareNoCase(i2->txt);
	}

	return -1;
}

int CALLBACK CSortClass::CompareDes(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	CSortItem * i1 = (CSortItem *) lParam1;
	CSortItem * i2 = (CSortItem *) lParam2;
	ASSERT(i1 && i2);

	if(i1 && i2)
	{
		return i2->txt.CompareNoCase(i1->txt);
	}
	
	return -1;
}

int CALLBACK CSortClass::CompareAscI(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	CSortItemInt * i1 = (CSortItemInt *) lParam1;
	CSortItemInt * i2 = (CSortItemInt *) lParam2;
	ASSERT(i1 && i2);
	
	if(i1 && i2)
	{
		if(i1->iInt == i2->iInt) 
		{
			return 0;
		}
		return i1->iInt > i2->iInt ? 1 : -1;
	}

	return -1;
}

int CALLBACK CSortClass::CompareDesI(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	CSortItemInt * i1 = (CSortItemInt *) lParam1;
	CSortItemInt * i2 = (CSortItemInt *) lParam2;
	ASSERT(i1 && i2);

	if(i1 && i2)
	{
		if(i1->iInt == i2->iInt) 
		{
			return 0;
		}

		return i1->iInt < i2->iInt ? 1 : -1;
	}
	return -1;
}

CSortClass::CSortItem::CSortItem(const DWORD _dw, const CString & _txt)
{
	dw = _dw;
	txt = _txt;
}

CSortClass::CSortItem::~CSortItem()
{
}

CSortClass::CSortItemInt::CSortItemInt(const DWORD _dw, const CString & _txt)
{
	iInt = _ttoi(_txt);
	dw = _dw;
}

