#pragma once
#include "../ZeusCache/cZeusCacheManager.h"


class CCacheMemoryView
{
public:
	CCacheMemoryView(void);
	~CCacheMemoryView(void);

	void Draw(HDC hdc);
	void Update();

private:		
	struct sCachePageView
	{
	public:
		void Draw(HDC hdc);
		void Update(UINT _uiCachePageIndex);

		COLORREF m_dwColor;
		int m_x, m_y;
	};

	sCachePageView m_pCachePageView[CACHE_PAGE_COUNT];
};
