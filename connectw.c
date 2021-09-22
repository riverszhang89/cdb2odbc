#include <stringapiset.h>

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



    needed = WideCharToMultiByte(CP_UTF8, 0, dsn, -1, NULL, 0, NULL, NULL);
    if (needed <= 0)
        return SQL_ERROR;

    dsn_ansi = calloc(needed + 1, sizeof(SQLCHAR));
    WideCharToMultiByte(CP_UTF8, 0, dsn, len, dsn_ansi, needed + 1, NULL, NULL);


    needed = WideCharToMultiByte(CP_UTF8, 0, uid, -1, NULL, 0, NULL, NULL);
    if (needed <= 0)
        return SQL_ERROR;

    uid_ansi = calloc(needed + 1, sizeof(SQLCHAR));
    WideCharToMultiByte(CP_UTF8, 0, uid, len, uid_ansi, needed + 1, NULL, NULL);


    needed = WideCharToMultiByte(CP_UTF8, 0, auth, -1, NULL, 0, NULL, NULL);
    if (needed <= 0)
        return SQL_ERROR;

    auth_ansi = calloc(needed + 1, sizeof(SQLCHAR));
    WideCharToMultiByte(CP_UTF8, 0, auth, len, auth_ansi, needed + 1, NULL, NULL);

    __info("Connecting to %s.", dsn_ansi);

    ret = SQLConnect(hdbc, dsn_ansi, SQL_NTS, uid_ansi, uid_len, auth_ansi, auth_len);
    free(dsn_ansi);

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
    SQLCHAR inansi[MAX_CONN_INFO_LEN];

    __debug("enters method.");

    int len = WideCharToMultiByte(CP_UTF8, 0, in_conn_str, in_conn_strlen, inansi, MAX_CONN_INFO_LEN, NULL, NULL);

    if (len <= 0)
	return SQL_ERROR;

    SQLCHAR outansi[MAX_CONN_INFO_LEN];
    SQLSMALLINT outputlen;

    ret = SQLDriverConnect(hdbc, hwnd, inansi, len, outansi, MAX_CONN_INFO_LEN, &outputlen, drv_completion);

    if (ret == SQL_SUCCESS) {
	len = MultiByteToWideChar(CP_UTF8, 0, outansi, -1, out_conn_str, out_conn_str_max);
	if (len > 0 && out_conn_strlen)
            *out_conn_strlen = wcslen(out_conn_str);
    }

    __debug("leaves method.");
    return ret;
}
