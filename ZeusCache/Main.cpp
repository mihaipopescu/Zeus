// ZeusCacheMemory.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include "cZeusCacheManager.h"
#include "cErrorManager.h"

int main(int argc, char** argv)
{
	cZeusCacheManager::Create();
	cErrorManager::Create();


	if( !cZeusCacheManager::Get()->MapDiskMemory(".") )
		cErrorManager::Get()->SetError(ERR_INIT_FROM_DISK);

	char szBuffer[1024] = {0};
	DWORD nRead = 0;
	HANDLE hFile = cZeusCacheManager::Get()->GetFile("ZeusCacheMemory.vcproj");
	
	printf("Read from file...\n");
	
	cZeusCacheManager::Get()->ReadFile(hFile, szBuffer, 1024, &nRead);
	//szBuffer[1023] = 0;
	//printf("%s", szBuffer);

	cZeusCacheManager::Get()->ReadFile(hFile, szBuffer, 1024, &nRead);
	//szBuffer[1023] = 0;
	//printf("%s", szBuffer);

	if( cErrorManager::Get()->GetLastError() != ERR_NONE )
		printf("ERROR %x", cErrorManager::Get()->GetLastError());
	
	getchar();
	cZeusCacheManager::Destroy();
	cErrorManager::Destroy();
	return 0;
}

