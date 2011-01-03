#pragma once
/*
#include <string.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <direct.h>
#include <io.h>
*/
#include "..\aes\zipcrypt.h"
#include "..\zlib\zip.h"
#include "..\zlib\unzip.h"

int check_exist_file(const char* filename);
int getFileCrc(const char* filenameinzip, void*buf, unsigned long size_buf, unsigned long* result_crc);
int filetime(const char *f, tm_zip *tmzip, uLong *dt);
void change_file_date(const char *filename, uLong dosdate, tm_unz tmu_date);
int makedir (char *newdir);
