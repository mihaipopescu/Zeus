// ZeusCommander.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "ZeusCommander.h"
#include "..\zlib\unzip.h"
#include <stdio.h>
#include "malloc.h"
#include "wcxhead.h"

#ifdef _MANAGED
#pragma managed(push, off)
#endif

//#define WCXDEBUG
#ifdef WCXDEBUG
const char DebugFileName[] = "C:\\test.txt";
#pragma message( " \nATTENTION: WCXDEBUG is defined. Don't release this version!!!\n " )
#endif


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
    return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif

struct tZeusArchiveInfo
{
	HANDLE zip;
	int	eof;
	tChangeVolProc ChangeVolProc;
	tProcessDataProc ProcessDataProc;
};

extern "C"// __declspec(dllexport) 
tZeusArchiveInfo* __stdcall OpenArchive (tOpenArchiveData *ArchiveData)
{
	tZeusArchiveInfo *hArcData = NULL;

#ifdef WCXDEBUG
	FILE *f = fopen(DebugFileName, "a");
	fprintf(f, "---OpenArchive---");
	fclose(f);
#endif

	if (ArchiveData->OpenMode == PK_OM_LIST || ArchiveData->OpenMode == PK_OM_EXTRACT)
	{
		hArcData = new tZeusArchiveInfo;
		if (hArcData == NULL)
		{
			ArchiveData->OpenResult = E_NO_MEMORY;
			return NULL;
		}

		unzFile uzf = unzOpen(ArchiveData->ArcName);
	
		ArchiveData->OpenResult = uzf!=NULL ? 0 : E_EOPEN;

		hArcData->zip = uzf;
		hArcData->eof = 0;
	}
	else
	{
	  ArchiveData->OpenResult = E_NOT_SUPPORTED;
	  return NULL;
	}

	return hArcData;
}

extern "C"// __declspec(dllexport)
int  __stdcall ReadHeader (tZeusArchiveInfo* hArcData, tHeaderData *HeaderData)
{
#ifdef WCXDEBUG
	FILE *f = fopen(DebugFileName, "a");
	fprintf(f, "---ReadHeader---");
	fclose(f);
#endif

	unz_file_info file_info;
	
	if(hArcData->eof == 1)
	{
		return E_END_ARCHIVE;
	}

	// getting current file info
	if(unzGetCurrentFileInfo(hArcData->zip, &file_info, HeaderData->FileName, MAX_PATH, NULL,0, NULL,0) != UNZ_OK)
	{
		return E_BAD_ARCHIVE;
	}

	// checking the zip compresion method
	if (file_info.compression_method != 0 && file_info.compression_method != Z_DEFLATED 
		&& file_info.compression_method != Z_METHOD_AES)
	{
		return E_UNKNOWN_FORMAT;
	}

	HeaderData->Method	=	file_info.compression_method;
	HeaderData->Flags =		file_info.flag;
	HeaderData->UnpSize =	file_info.uncompressed_size;
	HeaderData->PackSize =	file_info.compressed_size;
	HeaderData->FileCRC =	file_info.crc;
	HeaderData->FileTime =	(file_info.tmu_date.tm_year - 1980) << 25 | file_info.tmu_date.tm_mon << 21 | 
							file_info.tmu_date.tm_mday << 16 | file_info.tmu_date.tm_hour << 11 | 
							file_info.tmu_date.tm_min << 5 | file_info.tmu_date.tm_sec / 2;

	return 0;
}

extern "C"// __declspec(dllexport) 
int __stdcall ProcessFile (tZeusArchiveInfo* hArcData, int Operation, char *DestPath, char *DestName)
{
#ifdef WCXDEBUG
	FILE *f = fopen(DebugFileName, "a");
	fprintf(f, "---ProcessFile---");
	fclose(f);
#endif

	if(Operation == PK_SKIP)
		goto SKIP;

	//if(Operation == PK_TEST)
	//	return 0;

	FILE *fout = fopen(DestName, "wb");
	int err;

	char buf[16330];
	do
	{
		err = unzReadCurrentFile(hArcData->zip, buf, sizeof(buf));
		if (err<0)
		{
			fclose(fout);
			goto ERR;
		}
		if (err>0)
		{
			if (fwrite(buf,err,1,fout)!=1)
			{
				fclose(fout);
				err=UNZ_ERRNO;
				goto ERR;
			}
		}
	}
	while (err>0);



SKIP:
	err = unzGoToNextFile(hArcData->zip);

ERR:
	switch(err)
	{
	case UNZ_OK:
		return 0;
	case UNZ_END_OF_LIST_OF_FILE:
		hArcData->eof = 1;
		return 0;
	default:
		return E_BAD_ARCHIVE;
	}

}

extern "C"// __declspec(dllexport) 
int __stdcall CloseArchive (tZeusArchiveInfo* hArcData)
{
	if(unzClose(hArcData->zip)!=UNZ_OK)
		return E_ECLOSE;

	delete hArcData;
	hArcData = NULL;
	return 0;
}

extern "C"// __declspec(dllexport) 
void __stdcall SetChangeVolProc (tZeusArchiveInfo* hArcData, tChangeVolProc pChangeVolProc)
{
#ifdef WCXDEBUG
	FILE *f = fopen(DebugFileName, "w");
	fprintf(f, "---SetChangeVolProc---");
	fclose(f);
#endif

	hArcData->ChangeVolProc = pChangeVolProc;
}

extern "C"// __declspec(dllexport) 
void __stdcall SetProcessDataProc (tZeusArchiveInfo* hArcData, tProcessDataProc pProcessDataProc)
{
#ifdef WCXDEBUG
	FILE *f = fopen(DebugFileName, "w");
	fprintf(f, "---SetProcessDataProc---");
	fclose(f);
#endif

	hArcData->ProcessDataProc = pProcessDataProc;
}

extern "C"// __declspec(dllexport)
int  __stdcall GetPackerCaps (void)
{
   return PK_CAPS_SEARCHTEXT;
}