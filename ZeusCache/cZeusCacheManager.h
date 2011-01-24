#pragma once
#include "cSingleton.h"

#define CACHE_PAGE_COUNT 2560 // 2560 pages of 4KB = 10 MB of cache memory

#define CACHE_PAGE_SIZE 4096 // 4KB page size
#define NEXT_PAGE_BASE(addr) ((((addr) >> 12) + 1) << 12) // base address of a 4KB page
#define MEM_PAGE_BASE(addr) ((addr) & 0xFFFFF000) // base address of a 4KB page
#define MEM_PAGE_OFFSET(addr) ((addr) & 0xFFF) // the offset of an address on a 4KB page

class cZeusCacheManager
{
	SINGLETON_DECL(cZeusCacheManager)

public:
	BOOL MapDiskMemory(LPCSTR lpDirectory);

	HANDLE GetFile(LPCSTR lpFilename);
	BOOL ReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead);
	
private:
	// This SLL list is static and thus it doesn't have to manages free space
	struct sFileNode
	{
		CHAR filename[MAX_PATH];	// filename
		DWORD base;					// base virtual address
		DWORD size;					// file size
		DWORD offset;				// read offset within the file
		FILE* file;					// real file handle
		sFileNode* next;			// next file node pointer
	}* m_pRoot;

	struct sCacheEntry
	{
		BOOL bValid;
		BOOL bReferenced;
		DWORD dwReferenceCounter;
		DWORD dwAddress;
		BYTE _Data[CACHE_PAGE_SIZE];
	} _CacheMemory[CACHE_PAGE_COUNT];

	sCacheEntry* GetCacheEntry(DWORD dwBase, sFileNode* pNode);
};
