#pragma once
// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the ZIPAES_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// ZIPAES_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef ZIPAES_EXPORTS
#define ZIPAES_API __declspec(dllexport)
#else
#define ZIPAES_API __declspec(dllimport)
#endif

#include "..\aes\zipcrypt.h"
#include "..\zlib\zip.h"
#include "..\zlib\unzip.h"

#define EXFILE_CREATEBACKUP 0
#define EXFILE_APPEND		1
#define EXFILE_OVERWRITE	2

#define ZEUSERR_FILEOPEN			-1001
#define ZEUSERR_FILEREAD			-1002
#define ZEUSERR_FILEREMOVE			-1003
#define ZEUSERR_FILERENAME			-1004
#define ZEUSERR_INTERNALZIPERROR	-1005
#define ZEUSERR_USERHALT			-1006
#define ZEUSERR_ZIPCREATE			-1007
#define ZEUSERR_ZIPOPEN				-1008
#define ZEUSERR_FILECREATE			-1009
#define ZEUSERR_DIRCREATE			-1010

class ZIPAES_API ZeusProtection
{
public:
	ZeusProtection(void);

public:
	// creates the archive and initializes the internal state
	bool CreateArchive(const char *lpszArchiveName, const char *lpszPassword, 
		int iExistFileOp = EXFILE_CREATEBACKUP, int iCompresionLevel = 0, int iEncryptionStrength = AES_128, bool bRemove = false);

	// protects a file in the current zip file
	bool ProtectFile(const char *lpszFileName);

public:
	// opens an archive for unprotecting
	bool OpenArchive(const char *lpszArchiveName, const char *lpszPassword=NULL, 
					int iExistFileOp=EXFILE_CREATEBACKUP);

	// returns true if the archive is protected with zeus
	bool IsProtected();

	// unprotects the current file
	bool UnprotectCurrentFile();

	// goes to the next file in zip
	bool UnprotectNextFile();

	// retrieves the current filename in zip and the associated file info
	bool GetCurrentFileNameInZip(char *lpszFileNameInZip, int size, unz_file_info *file_info = NULL);

	// retrieves the number of files in the zip
	int GetNumberOfFiles();

public:
	// closing an archive
	bool CloseArchive();

	// Getting the last error index and string if str != NULL
	int GetLastError(char *str=NULL);

	// gets the archive ratio
	int GetArchiveRatio();

public:
	~ZeusProtection(void);

private:
	void* zf;
	int ExistFileOp;
	char *ArchiveName, *Password;
	char ErrTxt[MAX_PATH];
	bool RemoveDone;
	int CompresionLevel, EncryptionStrength;
	int Error;
	int Mode;

public:
	volatile bool ZeusHalt;
	volatile int ZeusProgress;
};
