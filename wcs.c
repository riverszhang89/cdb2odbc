#include "driver.h"
#include "wcs.h"


int sqlwcharbytelen(SQLWCHAR *str)
{
    int len;
    for (len = 0; str[len] != 0; ++len) ;
    return len * sizeof(SQLWCHAR);
}



#ifdef _MSC_VER
#include <stringapiset.h>
int utf8_to_ucs2(SQLCHAR *src, SQLWCHAR *dest, size_t len)
{
	__debug(__func__);
    return MultiByteToWideChar(CP_UTF8, 0, src, -1, dest, len);
}

int ucs2_to_utf8(SQLWCHAR *src, SQLCHAR *dest, size_t len)
{
	__debug(__func__);
    return WideCharToMultiByte(CP_UTF8, 0, src, -1, dest, len, NULL, NULL);
}
#else

#include <iconv.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>

int utf8_to_ucs2(SQLCHAR *src, SQLWCHAR *dest, size_t len)
{
    int rv = 0;
    iconv_t cb = iconv_open("UTF-16//IGNORE", "UTF-8");
    if (cb == (iconv_t)(-1))
        return 0;

    size_t inbytesleft = strlen((char *)src) + 1;
    size_t outbytesleft = len;

    size_t ret = iconv(cb, (char**)&src, (size_t*)&inbytesleft, (char**)&dest, (size_t*)&outbytesleft);
    if (ret == (size_t) - 1) {
        __warn("error:%s, %d", strerror(errno), errno);
        rv = -1;
    } else {
        rv = len - outbytesleft;
    }
    iconv_close(cb);
    return rv;
}

int ucs2_to_utf8(SQLWCHAR *src, SQLCHAR *dest, size_t len)
{
    int rv = 0;
    iconv_t cb = iconv_open("UTF-8//IGNORE", "UTF-16");
    if (cb == (iconv_t)(-1))
        return 0;

    size_t inbytesleft = sqlwcharbytelen(src) + sizeof(SQLWCHAR);
    size_t outbytesleft = len;

    size_t ret = iconv(cb, (char**)&src, (size_t*)&inbytesleft, (char**)&dest, (size_t*)&outbytesleft);
    if (ret == (size_t) - 1) {
        abort();
        __warn("error:%s, %d", strerror(errno), errno);
        rv = -1;
    } else {
        rv = len - outbytesleft;
    }
    iconv_close(cb);
    return rv;
}
#endif
