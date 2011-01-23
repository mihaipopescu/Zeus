// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <windows.h>


#define SAFE_DELETE(p) { delete p; p = NULL; }
#define SAFE_DELETE_ARR(a) { delete [] a; a = NULL; }