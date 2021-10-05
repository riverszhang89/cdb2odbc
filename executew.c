#include <stringapiset.h>

SQLRETURN SQL_API SQLExecDirectW(SQLHSTMT        hstmt,
                                SQLWCHAR         *sql,
                                SQLINTEGER      len)
{
    SQLRETURN ret;
    SQLCHAR *sqlansi;

    __debug("enters method.");

    int needed = WideCharToMultiByte(CP_UTF8, 0, sql, -1, NULL, 0, NULL, NULL);
    if (needed <= 0)
        return SQL_ERROR;

    sqlansi = malloc(needed + 1);
    WideCharToMultiByte(CP_UTF8, 0, sql, -1, sqlansi, needed + 1, NULL, NULL);

    ret = SQLExecDirect(hstmt, sqlansi, needed);

    free(sqlansi);

    __debug("leaves method.");
    return ret;
}
