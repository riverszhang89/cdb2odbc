#include "wcs.h"

SQLRETURN SQL_API SQLConnectW(
        SQLHDBC         hdbc,
        SQLWCHAR        *dsn,
        SQLSMALLINT     dsn_len,
        SQLWCHAR        *uid,
        SQLSMALLINT     uid_len,
        SQLWCHAR        *auth,
        SQLSMALLINT     auth_len)
{
    SQLCHAR *dsn_ansi, *uid_ansi, *auth_ansi;
    int needed;
    SQLRETURN ret;

    __debug("enters method.");

    dsn_ansi = uid_ansi = auth_ansi = NULL;

    /* dsn */
    needed = sqlwcharbytelen(dsn) + sizeof(SQLWCHAR);
    dsn_ansi = malloc(needed);
    if (ucs2_to_utf8(dsn, dsn_ansi, needed) > 0) {
        if (uid == NULL)
            uid_ansi = NULL;
        else {
            needed = sqlwcharbytelen(uid) + sizeof(SQLWCHAR);
            uid_ansi = malloc(needed);
            ucs2_to_utf8(uid, uid_ansi, needed);
        }

        if (auth == NULL)
            auth_ansi = NULL;
        else {
            needed = sqlwcharbytelen(auth) + sizeof(SQLWCHAR);
            auth_ansi = malloc(needed);
            ucs2_to_utf8(auth, auth_ansi, needed);
        }
        __info("Connecting to %s.", dsn_ansi);
        /* call the ANSI version. */
        ret = __SQLConnect(hdbc, dsn_ansi, SQL_NTS, uid_ansi, SQL_NTS, auth_ansi, SQL_NTS);
        free(dsn_ansi);
        free(uid_ansi);
        free(auth_ansi);
    }

    __debug("leaves method.");
    return ret;
}

SQLRETURN SQL_API SQLDriverConnectW(
        SQLHDBC         hdbc,
        SQLHWND         hwnd,
        SQLWCHAR        *in_conn_str,
        SQLSMALLINT     in_conn_strlen,
        SQLWCHAR        *out_conn_str,
        SQLSMALLINT     out_conn_str_max,
        SQLSMALLINT     *out_conn_strlen,
        SQLUSMALLINT    drv_completion)
{
    SQLRETURN ret;
    SQLCHAR *inansi;

    __debug("enters method.");

    int needed = sqlwcharbytelen(in_conn_str) + sizeof(SQLWCHAR);
    inansi = malloc(needed);

    int len = ucs2_to_utf8(in_conn_str, inansi, needed);

    if (len <= 0)
        return SQL_ERROR;

    SQLCHAR outansi[MAX_CONN_INFO_LEN];
    SQLSMALLINT outputlen;

    ret = __SQLDriverConnect(hdbc, hwnd, inansi, SQL_NTS, outansi, MAX_CONN_INFO_LEN, &outputlen, drv_completion);

    if (ret == SQL_SUCCESS && out_conn_str != NULL) {
        len = utf8_to_ucs2(outansi, out_conn_str, out_conn_str_max * sizeof(SQLWCHAR));
        if (len > 0 && out_conn_strlen)
            *out_conn_strlen = len - 1;
    }

    __debug("leaves method.");
    return ret;
}
