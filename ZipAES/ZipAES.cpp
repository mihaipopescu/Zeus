// ZipAES.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "ZipAES.h"
#include "direct.h"
#include "errno.h"

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

/*

ZIPAES_API BOOL doList(const char *lpszFileName, char *listStr, ULONG *ratio)
{	
	int err;
	unz_global_info gi;
	unz_file_info file_info; 
	AES_ExtraField aef;
	unzFile uf=NULL;
	char string_method[10], ext[3];


	uf = unzOpen(lpszFileName);
 
	if (uf==NULL)
		return FALSE;

	err = unzGetGlobalInfo (uf, &gi);

    if (err!=UNZ_OK)
        return FALSE;

	err = unzGetCurrentFileInfo(uf,&file_info, NULL,0,&aef,sizeof(AES_ExtraField),NULL,0);

    if (err!=UNZ_OK)
        return FALSE;
     
    if (file_info.uncompressed_size>0)
        *ratio = (file_info.compressed_size*100)/file_info.uncompressed_size;

    if (file_info.compression_method==0)
        strcpy(string_method,"Stored");
    else
    if (file_info.compression_method==Z_DEFLATED)
    {
		uInt iLevel=(uInt)((file_info.flag & 0x6)/2);
		strcpy(string_method,"Deflated ");		
		_itoa(iLevel, ext, 10);
		strcat(string_method, ext);		

    }
	if (file_info.compression_method==Z_METHOD_AES)
	{
		strcpy(string_method,"AES-");
		switch(aef.cEncryptionStrength)
		{
			case AES_128:
				strcat(string_method, "128");
				break;
			case AES_192:
				strcat(string_method, "192");
				break;
			case AES_256:
				strcat(string_method, "256");
				break;
			default:
				strcat(string_method, "N/A");
				break;
		}	

	}
    else
        strcpy(string_method,"Unknown");

    sprintf(listStr, "%7lu\n%6s\n%7lu\n%lu% \n%2.2lu-%2.2lu-%2.2lu\n%2.2lu:%2.2lu\n%8.8lx\n",
            file_info.uncompressed_size, 
			string_method,
            file_info.compressed_size,
            *ratio,
            (uLong)file_info.tmu_date.tm_mon + 1,
            (uLong)file_info.tmu_date.tm_mday,
            (uLong)file_info.tmu_date.tm_year % 100,
            (uLong)file_info.tmu_date.tm_hour,(uLong)file_info.tmu_date.tm_min,
            (uLong)file_info.crc);

	//error closing ?
	if (unzClose(uf) != UNZ_OK)
		return FALSE;

	//all good
	return TRUE;
}

*/

//void PasswordTessellation(const char *smallPwd, char *bigPwd, unsigned int bigPwdLen)
//{
//	UINT i, pwd_len = strlen(smallPwd);
//
//	if(bigPwdLen > MAX_PWD_LENGTH)
//		return;
//
//	for(i=0;i<bigPwdLen-1;i++)
//		bigPwd[i] = smallPwd[ i%pwd_len ];
//
//	bigPwd[i] = 0;
//}

int filetime(const char *f,                /* name of file to get info on */
    tm_zip *tmzip,             /* return value: access, modific. and creation times */
    uLong *dt)             /* dostime */
{
  int ret = 0;
  {
      FILETIME ftLocal;
      HANDLE hFind;
      WIN32_FIND_DATA  ff32;

      hFind = FindFirstFile(f,&ff32);
      if (hFind != INVALID_HANDLE_VALUE)
      {
        FileTimeToLocalFileTime(&(ff32.ftLastWriteTime),&ftLocal);
        FileTimeToDosDateTime(&ftLocal,((LPWORD)dt)+1,((LPWORD)dt)+0);
        FindClose(hFind);
        ret = 1;
      }
  }
  return ret;
}

/* change_file_date : change the date/time of a file
    filename : the filename of the file where date/time must be modified
    dosdate : the new date at the MSDos format (4 bytes)
    tmu_date : the SAME new date at the tm_unz format */
void change_file_date(const char *filename, uLong dosdate, tm_unz tmu_date)
{
  HANDLE hFile;
  FILETIME ftm,ftLocal,ftCreate,ftLastAcc,ftLastWrite;

  hFile = CreateFile(filename,GENERIC_READ | GENERIC_WRITE,
                      0,NULL,OPEN_EXISTING,0,NULL);
  GetFileTime(hFile,&ftCreate,&ftLastAcc,&ftLastWrite);
  DosDateTimeToFileTime((WORD)(dosdate>>16),(WORD)dosdate,&ftLocal);
  LocalFileTimeToFileTime(&ftLocal,&ftm);
  SetFileTime(hFile,&ftm,&ftLastAcc,&ftm);
  CloseHandle(hFile);
}

/* calculate the CRC32 of a file,
   because to encrypt a file, we need known the CRC32 of the file before */
int getFileCrc(const char* filenameinzip, void*buf, unsigned long size_buf, unsigned long* result_crc)
{
   unsigned long calculate_crc=0;
   int err=ZIP_OK;
   FILE * fin = fopen(filenameinzip, "rb");
   unsigned long size_read = 0;
   unsigned long total_read = 0;
   if (fin==NULL)
   {
       err = ZIP_ERRNO;
   }

    if (err == ZIP_OK)
        do
        {
            err = ZIP_OK;
            size_read = (int)fread(buf,1,size_buf,fin);
            if (size_read < size_buf)
				if (feof(fin)==0)
                err = ZIP_ERRNO;

            if (size_read>0)
                calculate_crc = crc32(calculate_crc,(const Bytef*)buf,size_read);
            total_read += size_read;

        } while ((err == ZIP_OK) && (size_read>0));

    if (fin)
        fclose(fin);

    *result_crc=calculate_crc;
    return err;
}

int check_exist_file(const char* filename)
{
    FILE* ftestexist;
    int ret = 1;
    ftestexist = fopen(filename,"rb");
    if (ftestexist==NULL)
        ret = 0;
    else
        fclose(ftestexist);
    return ret;
}

int makedir (char *newdir)
{
	char *buffer ;
	char *p;
	int  len = (int)strlen(newdir);

	if (len <= 0)
		return 0;

	buffer = (char*)malloc(len+1);
	strcpy(buffer,newdir);

	if (buffer[len-1] == '/') {
		buffer[len-1] = '\0';
	}
	if (mkdir(buffer) == 0)
	{
		free(buffer);
		return 1;
	}

	p = buffer+1;
	while (1)
	{
		char hold;

		while(*p && *p != '\\' && *p != '/')
			p++;
		hold = *p;
		*p = 0;
		if((mkdir(buffer) == -1) && (errno == ENOENT))
		{
			free(buffer);
			return 0;
		}
		if (hold == 0)
			break;
		*p++ = hold;
	}
	free(buffer);
	return 1;
}