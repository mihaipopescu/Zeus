#include "StdAfx.h"
#include "CacheMemoryView.h"

#define ORIGIN_X 185
#define ORIGIN_Y 10

void CCacheMemoryView::sCachePageView::Draw(HDC hdc)
{
	HBRUSH hBrush = CreateSolidBrush(m_dwColor);
	HGDIOBJ hOldObj = SelectObject(hdc, hBrush);
	Rectangle(hdc, m_x, m_y, m_x+9, m_y+9);
	SelectObject(hdc, hOldObj);
	DeleteObject(hBrush);
}

void CCacheMemoryView::sCachePageView::Update(UINT _uiCachePageIndex)
{
	m_dwColor = cZeusCacheManager::Get()->GetCachePageColor(_uiCachePageIndex);
	m_x = ORIGIN_X + 8*(_uiCachePageIndex % 128);
	m_y = ORIGIN_Y + 8*(_uiCachePageIndex / 128) + _uiCachePageIndex/256*2;
}

CCacheMemoryView::CCacheMemoryView(void)
{
}

CCacheMemoryView::~CCacheMemoryView(void)
{
}

void CCacheMemoryView::Draw(HDC hdc)
{
	for(int i=0;i<CACHE_PAGE_COUNT;++i)
		m_pCachePageView[i].Draw(hdc);
}

void CCacheMemoryView::Update()
{
	for(int i=0;i<CACHE_PAGE_COUNT;++i)
		m_pCachePageView[i].Update(i);
}