#include "StdAfx.h"
#include "ZipAES.h"
#include "ZeusProtection.h"
#include "..\aes\zipcrypt.h"
#include "time.h"

#define WRITEBUFFERSIZE (8192) //(16384)

#define ZEUS_PROTECTION		1
#define ZEUS_UNPROTECTION	2

#define ZEUS_COMMENTSTAMP "Zeus 2.0 Protection"


ZeusProtection::ZeusProtection(void)
{
	zf = NULL;
	ArchiveName = NULL;
	Password = NULL;
	ExistFileOp = 0;
	CompresionLevel = 0;
	EncryptionStrength = 0;
	Error = 0;
	Mode = 0;
	RemoveDone = false;

	ZeusHalt = false;
	ZeusProgress = 0;

	// init pseudorandom generator
	time_t lt;
	time(&lt);
	srand((int)lt);

	memset(ErrTxt, 0, MAX_PATH);
}

ZeusProtection::~ZeusProtection(void)
{
	CloseArchive();
}



bool ZeusProtection::CreateArchive(const char *lpszArchiveName, const char *lpszPassword, int iExistFileOp, int iCompresionLevel, int iEncryptionStrength, bool bRemove)
{
	// close existing unclosed zip file
	if(zf!=NULL)
		CloseArchive();

	// checks and solves the preexistance of the output file
	if(check_exist_file(lpszArchiveName))
	{

		switch(iExistFileOp)
		{

		case EXFILE_CREATEBACKUP:
			{
				char filename[MAX_PATH];

				// create a backup file 
				strcpy(filename, lpszArchiveName);

				// add .bak extension
				strcat(filename, ".bak");

				// check if name available and if not remove the ugly file
				if(check_exist_file(filename) && remove(filename)!=0)
				{
					Error = ZEUSERR_FILEREMOVE;
					strcpy(ErrTxt, filename);
					return false;
				}

				// renaming the existing file
				if(rename(lpszArchiveName, filename)!=0)
				{
					Error = ZEUSERR_FILERENAME;
					strcpy(ErrTxt, lpszArchiveName);
					return false;
				}
			}
			break;

		case EXFILE_OVERWRITE:

			if(remove(lpszArchiveName)!=0)
			{
				Error = ZEUSERR_FILEREMOVE;
				strcpy(ErrTxt, lpszArchiveName);
				return false;
			}

			iExistFileOp = APPEND_STATUS_CREATE;
			break;

		case EXFILE_APPEND:
			iExistFileOp = APPEND_STATUS_ADDINZIP;
			break;
		}
	}
	else
		iExistFileOp = APPEND_STATUS_CREATE;

	// open the file
	zf = zipOpen(lpszArchiveName, iExistFileOp);

	if(zf==NULL)
	{
		Error = ZEUSERR_ZIPCREATE;
		strcpy(ErrTxt, lpszArchiveName);
		return false;
	}

	// sets the mode
	Mode = ZEUS_PROTECTION;

	// setting protection class state
	ArchiveName = (char*)lpszArchiveName;
	ExistFileOp = iExistFileOp;
	Password = (char*)lpszPassword;
	RemoveDone = bRemove;

	if(iCompresionLevel < 0 && iCompresionLevel > 9)
		CompresionLevel = Z_DEFAULT_COMPRESSION;
	else
		CompresionLevel = iCompresionLevel;

	if(iEncryptionStrength < AES_128 && iEncryptionStrength > AES_RND)
		EncryptionStrength = AES_128;
	else
		EncryptionStrength = iEncryptionStrength;


	return true;
}


bool ZeusProtection::CloseArchive()
{
	bool ret = true;

	if(zf!=NULL)
	{
		switch(Mode)
		{
			case ZEUS_PROTECTION:
					ret = zipClose(zf, Error == 0?ZEUS_COMMENTSTAMP:NULL)==ZIP_OK;
				break;
			case ZEUS_UNPROTECTION:
					ret = unzClose(zf)==UNZ_OK;
				break;
		}
	
		zf = NULL;
	}

	// reset mode
	Mode = 0;

	return ret;
}

int ZeusProtection::GetLastError(char *str)
{
	if(str!=NULL)
	{
		switch(Error)
		{
		case ZEUSERR_INTERNALZIPERROR:
			strcpy(str, "Zip internal error");
			break;
		case ZEUSERR_USERHALT:
			strcpy(str, "User halt");
			break;
		case ZEUSERR_FILEREMOVE:
			strcpy(str, "Error removing file");
			break;
		case ZEUSERR_FILERENAME:
			strcpy(str, "Error renaming file");
			break;
		case ZEUSERR_ZIPCREATE:
			strcpy(str, "Error creating encrypted zip file");
			break;
		case ZEUSERR_ZIPOPEN:
			strcpy(str, "Error opening encrypted zip file");
			break;
		case ZEUSERR_FILECREATE:
			strcpy(str, "Error creating file");
		case ZEUSERR_FILEOPEN:
			strcpy(str, "Error opening file");
			break;
		case ZEUSERR_FILEREAD:
			strcpy(str, "Error reading file");
			break;
		case ZEUSERR_DIRCREATE:
			strcpy(str, "Error creating directory");
			break;
		}

		strcat(str, "\n");
		strcat(str, ErrTxt);
	}

	return Error;
}


bool ZeusProtection::ProtectFile(const char *lpszFileName)
{
	unsigned long crcFile=0;
	AES_ExtraField aef;
	zip_fileinfo zi;
	int size_read, err = ZIP_OK;
	FILE *fin=NULL;
	long flen, readsize;

	// wrong mode
	if(Mode!=ZEUS_PROTECTION)
		return false;

	// first of all check if the container zip file is ok
	if(zf == NULL)
	{
		Error = ZEUSERR_INTERNALZIPERROR;
		strcpy(ErrTxt, "Archive not created beforehand! Call CreateArchive().");
		return false;
	}

	// to protect the file first it should exist
	if(check_exist_file(lpszFileName)==0)
	{
		Error = ZEUSERR_FILEOPEN;
		strcpy(ErrTxt, lpszFileName);
		return false;
	}

	// fill AES extra field
	memset(&aef, 0, sizeof(AES_ExtraField));
	aef.iHeaderID = AES_HEADERID; //magic ID
	aef.iDataSize = 0x0007;	//extra field data size
	aef.iVendorVersion = AE1; // AE-1 with crc
	aef.iVendorID = 0x4541; //'AE'

	if(EncryptionStrength == AES_RND)
	{
		if((rand()%100)>50)
			aef.cEncryptionStrength = AES_128;
		else
			aef.cEncryptionStrength = AES_256;
	}
	else
		aef.cEncryptionStrength = EncryptionStrength;
	aef.iCompresionMethod = (unsigned short)((CompresionLevel != 0) ? Z_DEFLATED : 0);

	// get file datetime
	memset(&zi, 0, sizeof(zip_fileinfo));
	filetime(lpszFileName, &zi.tmz_date, &zi.dosDate);

	// begin protection
	char buf[WRITEBUFFERSIZE];
	ZeusProgress = -1;
	flen = 0;
	readsize = 0;
	crcFile = 0;

	// open the file to be protected
	if((fin = fopen(lpszFileName, "rb"))==NULL)
	{
		Error = ZEUSERR_FILEOPEN;
		strcpy(ErrTxt, lpszFileName);
		return false;
	}

	// getting the CRC of the file
    do
    {
		if(ZeusHalt)
		{
			Error = err = ZEUSERR_USERHALT;
			fclose(fin);
			return false;
		}

        err = ZIP_OK;
        size_read = (int)fread(buf, 1, WRITEBUFFERSIZE, fin);

        if (size_read < WRITEBUFFERSIZE)
			if (feof(fin)==0)
				err = ZIP_ERRNO;

        if (size_read>0)
            crcFile = crc32(crcFile,(const Bytef*)buf, size_read);

        flen += size_read;

    } while ((err == ZIP_OK) && (size_read>0));

	if(err!=ZIP_OK)
	{
		Error = ZEUSERR_INTERNALZIPERROR;
		strcpy(ErrTxt, "Error computing CRC of the file.");
		return false;
	}

	// creatig file in zip
	err = zipOpenNewFileInZip3(zf, lpszFileName, &zi,
			(const void*)&aef, sizeof(AES_ExtraField), (const void*)&aef, sizeof(AES_ExtraField), 
			//"File protected by Zeus 2.0" /* comment*/,
			NULL,
			Z_METHOD_AES,
			CompresionLevel, 0,
			/* -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, */
			-MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
			Password, crcFile);

	if(err!=ZIP_OK)
	{
		Error = ZEUSERR_INTERNALZIPERROR;
		strcpy(ErrTxt, "Error creating file in zip.");
		return false;
	}

	ZeusProgress = 0;
	fseek(fin, 0, SEEK_SET);

	// buffering the input file and protecting
	do
	{
		if(ZeusHalt)
		{
			Error = err = ZEUSERR_USERHALT;
			fclose(fin);
			return false;
		}

		// buffering
		size_read = (int)fread(buf, 1, WRITEBUFFERSIZE, fin);
		if (size_read < WRITEBUFFERSIZE && (feof(fin)==0))
		{
			err = ZIP_ERRNO;
			break;
		}
		
		// deflating with encryption
		if (size_read>0)
		{
			err = zipWriteInFileInZip (zf, buf, size_read);
			if (err<0)
			{
				break;
			}
		}

		readsize+=size_read;

		if(size_read>0)
			ZeusProgress = (int)(100.0*readsize/flen);

	} while ((err == ZIP_OK) && (size_read>0));

	// closing...
	if (err>=0)
		err = zipCloseFileInZip(zf);

	// close file
	if (fin)
		fclose(fin);

	// do not assume things could not go wrong
	if(err!=ZIP_OK)
	{
		Error = ZEUSERR_INTERNALZIPERROR;
		strcpy(ErrTxt, "Error protecting file ");
		strcat(ErrTxt, lpszFileName);
		return false;
	}

	//removing fie if flag set
	if(RemoveDone)
		remove(lpszFileName);

	return err == ZIP_OK;
}


bool ZeusProtection::OpenArchive(const char *lpszArchiveName, const char *lpszPassword, int iExistFileOp)
{
	// close existing unclosed zip file
	if(zf!=NULL)
		CloseArchive();

	zf = unzOpen(lpszArchiveName);

	if (zf==NULL)
	{
		Error = ZEUSERR_ZIPOPEN;
		strcpy(ErrTxt, lpszArchiveName);
		return false;
	}

	// sets the mode
	Mode = ZEUS_UNPROTECTION;

	ArchiveName = (char*)lpszArchiveName;
	Password = (char*)lpszPassword;
	ExistFileOp = iExistFileOp;

	return true;
}

bool ZeusProtection::IsProtected()
{
	char com[32];

	if(Mode!=ZEUS_UNPROTECTION)
		return false;

	unzGetGlobalComment(zf, com, 32);

	// ensure we have a zero terminated string
	com[31]=0;

	// checking zeus stamp
	return strcmp(com, ZEUS_COMMENTSTAMP)==0;
}

bool ZeusProtection::UnprotectNextFile()
{
	if(Mode!=ZEUS_UNPROTECTION)
		return false;

	int err = unzGoToNextFile(zf);
	
	switch(err)
	{
	case UNZ_OK:
		return true;
	case UNZ_END_OF_LIST_OF_FILE:
		return false;
	default:
		Error = ZEUSERR_INTERNALZIPERROR;
		strcpy(ErrTxt, "Error protecting next file.");
		return false;
	}
}

bool ZeusProtection::GetCurrentFileNameInZip(char *lpszFileNameInZip, int size, unz_file_info *file_info)
{
	if(Mode!=ZEUS_UNPROTECTION)
		return false;

	int err = unzGetCurrentFileInfo(zf,file_info,lpszFileNameInZip,size,NULL,0,NULL,0);

	if(err==UNZ_OK)
		return true;

	Error = ZEUSERR_INTERNALZIPERROR;
	strcpy(ErrTxt, "Error retrieving info from current file.");
	return false;	
}

bool ZeusProtection::UnprotectCurrentFile()
{
	if(Mode!=ZEUS_UNPROTECTION)
		return false;

	FILE *fout=NULL;
	char fn[MAX_PATH], path[MAX_PATH];
	unz_file_info fnfo;

	// opens the archive with the specified password
	int err = unzOpenCurrentFilePassword(zf, Password);

	if(err!=UNZ_OK)
	{
		Error = ZEUSERR_INTERNALZIPERROR;
		strcpy(ErrTxt, "Error unprotecting file with password.");
		return false;
	}

	// retrieve file name in zip and file info
	err = unzGetCurrentFileInfo(zf, &fnfo, fn, MAX_PATH, NULL,0, NULL,0);
	if(err!=UNZ_OK)
	{
		Error = ZEUSERR_INTERNALZIPERROR;
		strcpy(ErrTxt, "Error retrieving file name from zip file. Possibly corrupt zip file.");
		unzCloseCurrentFile(zf);
		return false;
	}


	// if the file already exist
	if(check_exist_file(fn))
	{
		if(ExistFileOp==EXFILE_CREATEBACKUP)
		{
			// create backup file
			strcpy(path, fn);
			strcat(path, ".bak");

			// check if name available and if not remove the ugly file
			if(check_exist_file(path) && remove(path)!=0)
			{
				Error = ZEUSERR_FILEREMOVE;
				strcpy(ErrTxt, path);
				unzCloseCurrentFile(zf);
				return false;
			}

			// rename old file to a backup file
			if(rename(fn, path)!=0)
			{
				Error = ZEUSERR_FILERENAME;
				strcpy(ErrTxt, fn);
				unzCloseCurrentFile(zf);
				return false;
			}
		}
	}
	else
		ExistFileOp = EXFILE_OVERWRITE;

	// find if the file name has a directory path we need to create
	char *p=fn, *s=NULL;

	while(*p++)
	{
		if(*p=='\\')
			s = p;
	}

	// if so we need to create that directory path 
	//  in order to create the output file
	if(s!=NULL)
	{
		*s=0;
		if(makedir(fn)==0)
		{
			Error = ZEUSERR_DIRCREATE;
			strcpy(ErrTxt, fn);
			unzCloseCurrentFile(zf);
			return false;
		}
		*s='\\';
	}

	// now we can write our file
	if(ExistFileOp == EXFILE_APPEND)
		fout = fopen(fn, "ab");
	else
		fout = fopen(fn, "wb");

	if(fout==NULL)
	{
		Error = ZEUSERR_FILECREATE;
		strcpy(ErrTxt, fn);
		unzCloseCurrentFile(zf);
		return false;
	}

	// unprotect the file
	char buf[WRITEBUFFERSIZE];
	long flen = fnfo.uncompressed_size, writesize = 0;

	ZeusProgress = 0;

	do
	{
		if(ZeusHalt)
		{
			Error = ZEUSERR_USERHALT;
			err = -1;
			break;

		}
		err = unzReadCurrentFile(zf, buf, WRITEBUFFERSIZE);
		if (err<0)
		{
			Error = ZEUSERR_INTERNALZIPERROR;
			strcpy(ErrTxt, "Error unprotecting archive.");
			break;
		}
		if (err>0)
		{
			if (fwrite(buf,err,1,fout)!=1)
			{
				Error = ZEUSERR_INTERNALZIPERROR;
				strcpy(ErrTxt, "Error writting unprotected file.");
				err=UNZ_ERRNO;
				break;
			}

			writesize += err;
			ZeusProgress = (int)(100.0*writesize/flen);
		}
	}
	while (err>0);

	// closing output file
	if (fout)
		fclose(fout);

	// changing it's date
	if (err==UNZ_OK)
		change_file_date(fn, fnfo.dosDate, fnfo.tmu_date);
	
	// closing file in zip wheather or not we had an error
	if (err==UNZ_OK)
	{
		err = unzCloseCurrentFile (zf);
	}
	else
		unzCloseCurrentFile(zf); 

	return err==UNZ_OK;
}


int ZeusProtection::GetNumberOfFiles()
{
	if(Mode!=ZEUS_UNPROTECTION)
		return -1;

	unz_global_info gi;

	if(unzGetGlobalInfo(zf, &gi)!=UNZ_OK)
		return -1;

	return gi.number_entry;
}

int ZeusProtection::GetArchiveRatio()
{
	if(Mode!=ZEUS_UNPROTECTION)
		return -1;

	if(IsProtected()==false)
		return -1;

	unz_global_info gi;

	if(unzGetGlobalInfo(zf, &gi)!=UNZ_OK)
		return -1;

	unz_file_info file_info;
	long zipsize = 0, unzsize = 0;

	for(int i=0;i<gi.number_entry;i++)
	{
		unzGetCurrentFileInfo(zf, &file_info, NULL, 0, NULL, 0, NULL, 0);

		zipsize += file_info.compressed_size;
		unzsize += file_info.uncompressed_size;

		if(i+1<gi.number_entry)
			unzGoToNextFile(zf);
	}

	return (int)(100.0*zipsize/unzsize);
}