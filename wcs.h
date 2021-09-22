#ifndef _WSC_H_
#define _WSC_H_
#include "driver.h"
int utf8_to_ucs2(SQLCHAR *src, SQLWCHAR *dest, size_t len);
int ucs2_to_utf8(SQLWCHAR *src, SQLCHAR *dest, size_t len);
int sqlwcharbytelen(SQLWCHAR *str);
#endif
