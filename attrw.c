#include "wcs.h"

SQLRETURN SQL_API SQLGetConnectAttrW(
        SQLHDBC        hdbc,
        SQLINTEGER     attr,
        SQLPOINTER     buf,
        SQLINTEGER     buflen,
        SQLINTEGER     *str_len)
{
    return __SQLGetConnectAttr(hdbc, attr, buf, buflen, str_len);
}

SQLRETURN SQL_API SQLGetConnectOptionW(
        SQLHDBC         hdbc,
        SQLUSMALLINT    option,
        SQLPOINTER      value_ptr)
{
    return __SQLGetConnectAttr(hdbc, option, value_ptr, 0, NULL);
}

SQLRETURN SQL_API SQLGetDescFieldW(SQLHDESC handle, SQLSMALLINT recno,
		SQLSMALLINT fieldid, SQLPOINTER value,
		SQLINTEGER buflen, SQLINTEGER *strlen)
{
	NOT_IMPL;
}

SQLRETURN SQL_API SQLGetStmtAttrW(
        SQLHSTMT        stmt,
        SQLINTEGER      attr,
        SQLPOINTER      buf,
        SQLINTEGER      buflen,
        SQLINTEGER      *str_len)
{
    return __SQLGetStmtAttr(stmt, attr, buf, buflen, str_len);
}


SQLRETURN SQL_API SQLGetStmtOptionW(
        SQLHSTMT        stmt,
        SQLUSMALLINT    option,
        SQLPOINTER      value_ptr)
{
    return __SQLGetStmtAttr(stmt, option, value_ptr, 0, NULL);
}

SQLRETURN SQL_API SQLSetConnectAttrW(
        SQLHDBC       hdbc,
        SQLINTEGER    attr,
        SQLPOINTER    buf,
        SQLINTEGER    str_len)
{
    return __SQLSetConnectAttr(hdbc, attr, buf, str_len);
}

SQLRETURN SQL_API SQLSetConnectOptionW(
        SQLHDBC         hdbc,
        SQLUSMALLINT    option,
        SQLULEN         param)
{
    return __SQLSetConnectAttr(hdbc, option, (SQLPOINTER)(intptr_t)param, 0);
}

SQLRETURN SQL_API SQLSetStmtAttrW(
        SQLHSTMT      hstmt,
        SQLINTEGER    attr,
        SQLPOINTER    buf,
        SQLINTEGER    str_len)
{
    return __SQLSetStmtAttr(hstmt, attr, buf, str_len);
}
