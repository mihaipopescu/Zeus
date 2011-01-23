#include "StdAfx.h"
#include "cErrorManager.h"

SINGLETON_IMPL(cErrorManager)

cErrorManager::cErrorManager(void)
{
	m_dwLastError = ERR_NONE;
}

cErrorManager::~cErrorManager(void)
{
}
