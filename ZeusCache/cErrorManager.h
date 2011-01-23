#pragma once
#include "cSingleton.h"

enum ZEUS_ERROR
{
	ERR_NONE = 0,
	ERR_INIT_FROM_DISK,
	ERR_INIT_FROM_ZEUS,


	ERR_FORCE_DWORD = 0xffffffff
};

class cErrorManager
{
	SINGLETON_DECL(cErrorManager);

public:
	void SetError(DWORD dwError) { m_dwLastError = dwError; } 
	DWORD GetLastError() { return m_dwLastError; }

private:
	DWORD m_dwLastError;
};
