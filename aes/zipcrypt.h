#ifndef _ZIPCRYPT_H_
#define _ZIPCRYPT_H_


#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <windows.h>

#include "prng.h"
#include "fileenc.h"

#define AE1		0x01
#define AE2		0x02

#define AES_128	0x01
#define AES_192 0x02
#define AES_256 0x03
#define AES_RND	0x04

#define AES_HEADERID 0x9901
#define MAX_CRYPT_HEADER	18
#define MAX_CRYPT_TAIL		10

#if defined(__cplusplus)
extern "C"
{
#endif

	
#pragma pack(push)  /* push current alignment to stack */
#pragma pack(1)     /* set alignment to 1 byte boundary */

typedef struct AESEF
{
	UINT16 iHeaderID;				//0x9901
	UINT16 iDataSize;				//currently 7
	UINT16 iVendorVersion;			//0x0001: AE-1, 0x0002: AE-2
	UINT16 iVendorID;				//"AE"
	UCHAR cEncryptionStrength;		//0x01: 128bit, 0x02: 192bit, 0x03: 256bit
	UINT16 iCompresionMethod;		//the actual compresion method
} AES_ExtraField;
#pragma pack(pop)   /* restore original alignment from stack */


int zipaesHeader(const char *passwd, unsigned char *buf, int bufSize, int mode, prng_ctx *rng, fcrypt_ctx *zcx);

int zipaesTail(unsigned char *buf, int bufSize, prng_ctx *rng, fcrypt_ctx *zcx);

int zipaesGetExtraField(AES_ExtraField* aef, const void* exbuf, int size);

#if defined(__cplusplus)
}
#endif


#endif