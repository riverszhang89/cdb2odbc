#include "wcs.h"

SQLRETURN SQL_API SQLExecDirectW(SQLHSTMT        hstmt,
                                SQLWCHAR         *sql,
                                SQLINTEGER      len)
{
    SQLRETURN ret;
    SQLCHAR *sqlansi;

    __debug("enters method.");

    int needed = sqlwcharbytelen(sql) + sizeof(SQLWCHAR);
    sqlansi = malloc(needed);
    ucs2_to_utf8(sql, sqlansi, needed);
    ret = __SQLExecDirect(hstmt, sqlansi, needed);
    free(sqlansi);

    __debug("leaves method.");
    return ret;
}


SQLRETURN SQL_API SQLPrepareW(SQLHSTMT hstmt, SQLWCHAR *sql, SQLINTEGER str_len)
{
    SQLRETURN ret;
    SQLCHAR *sqlansi;

    __debug("enters method.");

    int needed = sqlwcharbytelen(sql) + sizeof(SQLWCHAR);
    sqlansi = malloc(needed);
    ucs2_to_utf8(sql, sqlansi, needed);
    ret = __SQLPrepare(hstmt, sqlansi, needed);
    free(sqlansi);

    __debug("leaves method.");
    return ret;

}
