#include "zipcrypt.h"

#if defined(__cplusplus)
extern "C"
{
#endif

/* simple entropy collection function that uses the fast timer      */
/* since we are not using the random pool for generating secret     */
/* keys we don't need to be too worried about the entropy quality   */

int entropy_fun(unsigned char buf[], unsigned int len)
{   unsigned __int64    pentium_tsc[1];
    unsigned int        i;

    QueryPerformanceCounter((LARGE_INTEGER *)pentium_tsc);
    for(i = 0; i < 8 && i < len; ++i)
        buf[i] = ((unsigned char*)pentium_tsc)[i];
    return i;
}



//
//POURPOSE: Writes the encryption header in a buffer
//RETURNS:	Number of bytes written in the buffer
//			0	if password was too small
//			-1	if out of buffer memory
//
int zipaesHeader(const char *passwd, unsigned char *buf, int bufSize, int mode, prng_ctx *rng, fcrypt_ctx *zcx)
{
	UINT pwdlen = (UINT)strlen(passwd);
	unsigned char tmp_buf[16], salt[16];
	unsigned char *pbuf = buf;	

	//if(pwdlen < 1) return -1;	// password too short

#ifdef PASSWORD_VERIFIER
	if(bufSize < SALT_LENGTH(mode) + PWD_VER_LENGTH) return -1;	//buffer too small
#else
	if(bufSize < SALT_LENGTH(mode)) return -1;					//buffer too small
#endif

    prng_init(entropy_fun, rng);                /* initialise RNG,  */
    prng_rand(salt, SALT_LENGTH(mode), rng);    /* the salt and     */

	//write salt to buff
	memcpy(pbuf, salt, SALT_LENGTH(mode));
	pbuf += SALT_LENGTH(mode);

#ifdef PASSWORD_VERIFIER
	fcrypt_init(mode, passwd, pwdlen, salt, tmp_buf, zcx);

    /* put the password verifier (if used)    */
	memcpy(pbuf, tmp_buf, PWD_VER_LENGTH); 
	pbuf += PWD_VER_LENGTH;
#else
    fcrypt_init(mode, passwd, (unsigned int)pwdlen, salt, zcx);
#endif

	return (INT)(pbuf - buf);
}


int zipaesTail(unsigned char *buf, int bufSize, prng_ctx *rng, fcrypt_ctx *zcx)
{
	unsigned char tmp_buf[MAX_CRYPT_TAIL];

	if (bufSize < MAX_CRYPT_TAIL) return -1;

	/* write the MAC            */
    fcrypt_end(tmp_buf, zcx);

	memcpy(buf, tmp_buf, MAC_LENGTH(0));

    /* and close random pool    */
    prng_end(rng);

	return MAC_LENGTH(0);
}


int zipaesGetExtraField(AES_ExtraField* aef, const void* exbuf, int size)
{
	unsigned char *ex = (unsigned char*)exbuf;
	unsigned short sh;

	// try to find AES ExtraField in extrafield buffer
	while(size>0)
	{
		// HeaderID
		sh = *(unsigned short*)ex;

		// did we find the AES extra field ?
		if(sh==(unsigned short)AES_HEADERID)
		{
			memcpy(aef, ex, sizeof(AES_ExtraField));
			return 1;
		}

		// offset position to read size
		ex+=2;
		// go to next extra field
		// skipping current size
		sh = *(unsigned short*)ex;
		size-=sh+2;
		ex+=sh+2;
	}

	return 0;
}



#if defined(__cplusplus)
}
#endif