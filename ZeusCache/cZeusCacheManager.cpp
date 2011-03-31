#include "StdAfx.h"
#include "cZeusCacheManager.h"
#include "cErrorManager.h"

SINGLETON_IMPL(cZeusCacheManager)



cZeusCacheManager::cZeusCacheManager(void)
{
	m_pRoot = NULL;
	ZeroMemory(_CacheMemory, CACHE_PAGE_COUNT * sizeof(sCacheEntry));
}

cZeusCacheManager::~cZeusCacheManager(void)
{
	sFileNode* fn = m_pRoot;
	while(fn)
	{
		sFileNode* node = fn;
		fn = fn->next;
		delete node;
	}
}

BOOL cZeusCacheManager::MapDiskMemory(LPCSTR lpDirectory)
{
	WIN32_FIND_DATA ffd;
	HANDLE hFind;

	printf("Initializing ZeusCacheManager...\n");

	if( m_pRoot != NULL ) 
		return FALSE;

	if( !SetCurrentDirectory(lpDirectory) )
		return FALSE;

	hFind = FindFirstFile("*", &ffd);
	if( hFind == INVALID_HANDLE_VALUE )
		return FALSE;

	sFileNode* last = NULL;
	do
	{
		if( 0 == ( ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) ) // only files
		{
			// create new node
			sFileNode* node = new sFileNode;
			strcpy_s(node->filename, MAX_PATH, ffd.cFileName);
			node->size = ffd.nFileSizeLow;
			node->next = NULL;
			node->offset = 0;
			node->file = NULL;

			if( last == NULL )
			{
				node->base = 0;
				m_pRoot = node;
			}
			else
			{
				node->base = NEXT_PAGE_BASE(last->base + last->size);
				last->next = node;
			}

			last = node;

			printf("[%s] base=0x%08X size=%d/%d bytes\n", ffd.cFileName, last->base, last->size, NEXT_PAGE_BASE(last->size));
		}
	} while( FindNextFile(hFind, &ffd) != 0 );

	DWORD dwErr = GetLastError();
	if( dwErr != ERROR_NO_MORE_FILES )
	{
		FindClose(hFind);
		return FALSE;
	}
	
	FindClose(hFind);
	return TRUE;
}

HANDLE cZeusCacheManager::OpenFile(LPCSTR lpFilename)
{
	sFileNode* node = m_pRoot;
	while( node )
	{
		if( 0 == strcmp(lpFilename, node->filename) )
			return (HANDLE)node;

		node = node->next;
	}

	return NULL;
}

DWORD cZeusCacheManager::GetFileSize(HANDLE hFile)
{
	sFileNode* node = (sFileNode*)hFile;
	if( node )
		return node->size;
	else
		return 0;
}

BOOL cZeusCacheManager::ReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead)
{
	if( NULL == hFile )
		return FALSE;

	sFileNode* node = (sFileNode*)hFile;
	
	DWORD dwReadAddress = node->base + node->offset;
	DWORD base = MEM_PAGE_BASE(dwReadAddress);
	DWORD offset = MEM_PAGE_OFFSET(dwReadAddress);

	for(int i=0; i<CACHE_PAGE_COUNT; ++i)
	{
		_CacheMemory[i].bReferenced = FALSE;
	}

	DWORD nBytesToRead = nNumberOfBytesToRead;
	do
	{
		sCacheEntry* element = GetCacheEntry(base, node);
		if( element != NULL )
		{
			DWORD nSize = CACHE_PAGE_SIZE;
			if( nBytesToRead < nSize )
				nSize = nBytesToRead;
			memcpy((char*)lpBuffer + offset, element->_Data, nSize);
			offset += nSize;
			node->offset = offset;
			nBytesToRead -= nSize;
			base += CACHE_PAGE_SIZE;
			*lpNumberOfBytesRead += nSize;
		}
		else
		{
			printf("FATAL: GetCacheEntry failed!\n");
			break;
		}
	} while(nBytesToRead != 0);

	// update cache reference counter
	for(int i=0; i<CACHE_PAGE_COUNT; ++i)
	{
		_CacheMemory[i].dwReferenceCounter = (_CacheMemory[i].bReferenced << 31) | (_CacheMemory[i].dwReferenceCounter >> 1);
	}

	return nBytesToRead == 0;
}

VOID cZeusCacheManager::CloseFile(HANDLE hFile)
{
	if( NULL == hFile )
		return;

	sFileNode* node = (sFileNode*)hFile;

	// close file if opened
	if( node->file )
	{
		fclose(node->file);
		node->file = NULL;
		node->offset = 0;
	}
}

cZeusCacheManager::sCacheEntry* cZeusCacheManager::GetCacheEntry(DWORD dwBase, sFileNode* pNode)
{
	sCacheEntry* pFreeEntry = NULL;
	sCacheEntry* pEntryToReplace = NULL;	
	sCacheEntry* pEntry = NULL;
	DWORD dwBestCounter = 0xffffffff;

	// check out the cache first...
	for(int i=0; i<CACHE_PAGE_COUNT; ++i)
	{
		// the address is in cache and valid !
		if( dwBase == _CacheMemory[i].dwAddress && _CacheMemory[i].bValid )
		{
			_CacheMemory[i].bReferenced = TRUE;
			return &_CacheMemory[i];
		}

		// if we didn't found our free cache entry
		if( pFreeEntry == NULL )
		{
			if( !_CacheMemory[i].bValid )
			{
				// try to find a free not valid cache entry
				pFreeEntry = &_CacheMemory[i];
				pEntry = pFreeEntry;
			}
			else if( _CacheMemory[i].dwReferenceCounter < dwBestCounter )
			{
				// or find the not frequently used cache entry to replace in case of a cache miss
				dwBestCounter = _CacheMemory[i].dwReferenceCounter;
				pEntryToReplace = &_CacheMemory[i];
				pEntry = pEntryToReplace;
			}
		}
	}

	printf("\nZEUS: Cache miss at 0x%08X\n", dwBase);

	if( pFreeEntry == NULL )
	{
		printf("ERROR: No available free page!");
		return NULL;
	}

	// cache miss, try reading the file and update cache
	if( pNode->file == NULL && (0 != fopen_s(&pNode->file, pNode->filename, "rb") || NULL == pNode->file) )
	{
		printf("ERROR: Cannot open file %s \n", pNode->file);
		return NULL;;
	}

	// position read cursor within the file
	fseek(pNode->file, pNode->offset, SEEK_SET);

	DWORD nBytesToRead = pNode->size - pNode->offset;
	if( nBytesToRead > CACHE_PAGE_SIZE )
		nBytesToRead = CACHE_PAGE_SIZE;

	if( nBytesToRead != fread_s(pEntry->_Data, CACHE_PAGE_SIZE, 1, nBytesToRead, pNode->file) )
		return NULL;

	pFreeEntry->bValid = TRUE;
	pFreeEntry->bReferenced = TRUE;
	pFreeEntry->dwAddress = dwBase;

	return pFreeEntry;
}

COLORREF cZeusCacheManager::GetCachePageColor(UINT _uiCachePageIndex)
{
	if( _uiCachePageIndex < CACHE_PAGE_COUNT )
	{
		const sCacheEntry * pEntry = _CacheMemory + _uiCachePageIndex;
		DWORD rc = pEntry->dwReferenceCounter;
		unsigned char count = 0;
		while(rc != 0)
		{
			count++;
			rc>>=1;
		}
		return RGB( !pEntry->bValid * 255, pEntry->bReferenced * 255, count * 8);
	}

	return RGB(255, 0, 0);
}