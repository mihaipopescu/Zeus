#ifndef THREADSTRUCTH
#define THREADSTRUCTH

#include "..\ZipAES\ZeusProtection.h"

/////////////////////////////////////////////////////////////
// thread info structure
// use the GET_SAFE / SET_SAFE functions with anything shared between threads!
typedef struct ThreadInfo_tag
{
	ZeusProtection zp;

	CString csCrtFile;
	CString	csOutFile;				// output file
	CStringArray csaFiles;			// files to process

	BOOL	bProtect;
	bool    bRemoveDone;
	CString csPasswrod;
	int		iLevel;
	int		iPower;
	int		iExOp;

	BOOL     bDone;            // set to TRUE when done 
	int      iPercentDone;     // set by thread as work progresses
	int		 iOverallDone;
	int      iErr;             // set by thread
} ThreadInfo;

#endif