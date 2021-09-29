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
    int len;
    SQLRETURN ret;

    __debug("enters method.");

    len = wcstombs(NULL, dsn, 0);
    dsn_ansi = calloc(len + 1, sizeof(SQLCHAR));
    wcstombs(dsn_ansi, dsn, len + 1);

    len = wcstombs(NULL, uid, 0);
    uid_ansi = calloc(len + 1, sizeof(SQLCHAR));
    wcstombs(uid_ansi, uid, len + 1);

    len = wcstombs(NULL, auth, 0);
    auth_ansi = calloc(len + 1, sizeof(SQLCHAR));
    wcstombs(auth_ansi, uid, len + 1);

    __info("Connecting to %s.", dsn_ansi);

    ret = SQLConnect(hdbc, dsn_ansi, SQL_NTS, uid_ansi, uid_len, auth_ansi, auth_len);
    free(dsn_ansi);
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

    int len = WideCharToMultiByte(CP_ACP, 0, in_conn_str, -1, inansi, MAX_CONN_INFO_LEN, NULL, NULL);

    if (len <= 0)
	return SQL_ERROR;

    SQLCHAR outansi[MAX_CONN_INFO_LEN];
    SQLSMALLINT outputlen;

    ret = SQLDriverConnect(hdbc, hwnd, inansi, len, outansi, MAX_CONN_INFO_LEN, &outputlen, drv_completion);

    if (ret == SQL_SUCCESS) {
	len = MultiByteToWideChar(CP_UTF8, 0, outansi, MAX_CONN_INFO_LEN, out_conn_str, out_conn_str_max);
	if (len > 0 && out_conn_strlen)
	    *out_conn_strlen = len;
    }

    return ret;
}
