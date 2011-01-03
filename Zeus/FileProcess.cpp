// FileProcess.cpp: implementation of the CFileProcess class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Zeus.h"
#include "FileProcess.h"
#include "SHUtils.h"
#include "..\ZipAES\ZeusProtection.h"
#include <stack>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// in this example, this does nothing. but, in a real app, 
// you could use this class to do the real file processing work.
class CFileProcess
{
public:
	int ProcessFile(ThreadInfo *pThreadInfo)
	{
		int err = 0;

		// get the name of the file we'll be working on.
		// we're not really doing anything with the file here, so
		// this call is really just an example
		CStringArray &csaPaths = pThreadInfo->csaFiles;
		CString csOutFilename;
		CString csPassword;
		BOOL bProtect;
		int iLevel;
		int iPow;

		csOutFilename = pThreadInfo->csOutFile;
		csPassword = pThreadInfo->csPasswrod;
		bProtect = pThreadInfo->bProtect;
		if(bProtect)
		{
			iLevel = pThreadInfo->iLevel;
			iPow = pThreadInfo->iPower;
		}

		// sets the current directory as the working dir
		int last = csaPaths.GetAt(0).ReverseFind('\\');
		CString root = csaPaths.GetAt(0).Left(last);
		SetCurrentDirectory((LPCSTR)(root));

		ZeusProtection &zp = pThreadInfo->zp;
		pThreadInfo->iOverallDone = 0;

		if(bProtect)
		{
			zp.CreateArchive((LPCSTR)csOutFilename, (LPCSTR)csPassword, pThreadInfo->iExOp, pThreadInfo->iLevel, iPow, pThreadInfo->bRemoveDone);

			last = csaPaths.GetSize();

			for(int i=0;i<last;i++)
			{
				CString file = csaPaths.GetAt(i);
				int back = file.ReverseFind('\\');
				file = file.Mid(back + 1);
				const char *fn = (LPCSTR)(file);
				char szDir[MAX_PATH];
				std::stack<CString> dirs, dds;

				bool ret = true;

				if(GetFileAttributes(fn) & FILE_ATTRIBUTE_DIRECTORY)
				{
					dirs.push(CString(fn));

					while(!dirs.empty())
					{
						CString dir = dirs.top();
						dds.push(dir);
						dirs.pop();

						strcpy(szDir, (LPCSTR)dir);
						strcat(szDir, "\\*");

						HANDLE hFind = INVALID_HANDLE_VALUE;
						WIN32_FIND_DATA ffd;

						hFind = FindFirstFile(szDir, &ffd);
						if (INVALID_HANDLE_VALUE == hFind) 
						{
							ret = false;
							break;
						}

						while(FindNextFile(hFind, &ffd) != 0)
						{
							// Skip "." and ".."
							if (strcmp(ffd.cFileName, ".") == 0 ||
								strcmp(ffd.cFileName, "..") == 0)
								continue;

							strcpy(szDir, (LPCSTR)dir);
							strcat(szDir, "\\");
							strcat(szDir, ffd.cFileName);
							
							if(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
								dirs.push(CString(szDir));
							else
							{
								pThreadInfo->csCrtFile = CString(szDir);
								ret = zp.ProtectFile(szDir);
							}

							if(ret==false)
							{
								FindClose(hFind);
								break;
							}

						}

						if(ret==false)
							break;

						if(::GetLastError() != ERROR_NO_MORE_FILES)
						{
							ret = false;
							FindClose(hFind);
							break;
						}

						FindClose(hFind);
					}


					if(ret==false)
						break;

					// remove directory tree
					if(pThreadInfo->bRemoveDone)
					{
						pThreadInfo->csCrtFile = "Remove directory tree";
						while(!dds.empty())
						{
							RemoveDirectory((LPCSTR)dds.top());
							dds.pop();
						}
					}
				}
				else
				{
					pThreadInfo->csCrtFile = CString(fn);
					ret = zp.ProtectFile(fn);
				}

				if(ret==false)
					break;

				pThreadInfo->iOverallDone = (int)(100.0*(i+1)/last);
			}
		}
		else
		{
			CString csFile = csaPaths.GetAt(0);
			zp.OpenArchive((LPCSTR)csFile, (LPCSTR)csPassword, pThreadInfo->iExOp);
			char path[MAX_PATH];

			last = zp.GetNumberOfFiles();
			int i=0;

			do
			{
				zp.GetCurrentFileNameInZip(path, MAX_PATH);
				
				pThreadInfo->csCrtFile = CString(path);

				if(zp.UnprotectCurrentFile()==false)
					break;

				pThreadInfo->iOverallDone = (int)(100.0*(i+1)/last);
				i++;

			} while(zp.UnprotectNextFile());
		}

		zp.CloseArchive();

		return zp.GetLastError();
	}

};


// this is the worker thread function.

UINT FileProcessThreadFunc(LPVOID data)
{
	if (data==NULL) 
	{
		AfxEndThread(0);
		ASSERT(0);
	}

	ThreadInfo *pThreadInfo = (ThreadInfo *)data;

	// we're not done yet!
	pThreadInfo->bDone =  FALSE;

	// no errors
	pThreadInfo->iErr = 0;

	// process that file
	CFileProcess fp;
	int res = fp.ProcessFile(pThreadInfo);

	// tell the caller about any errors
	pThreadInfo->iErr = res;

	// now we're done.
	// the main thread is watching this value. when it sees
	// we've set bDone = TRUE, it will stop watching us
	pThreadInfo->bDone = TRUE;

	return 0;
}

