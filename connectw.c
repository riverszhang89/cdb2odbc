SQLRETURN SQL_API SQLConnectW(
        SQLHDBC         hdbc,
        SQLWCHAR        *dsn,
        SQLSMALLINT     dsn_len,
        SQLWCHAR        *uid,
        SQLSMALLINT     uid_len,
        SQLWCHAR        *auth,
        SQLSMALLINT     auth_len)
{
    SQLCHAR *dsn_ansi;
    SQLRETURN ret;

    __debug("enters method.");

    int len = wcstombs(NULL, dsn, 0);
    dsn_ansi = calloc(len + 1, sizeof(SQLCHAR));
    wcstombs(dsn_ansi, dsn, len + 1);

    __info("Connecting to %s.", dsn_ansi);

    ret = SQLConnect(hdbc, dsn_ansi, SQL_NTS, uid, uid_len, auth, auth_len);
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

    SQLCHAR *inansi;
    int len = wcstombs(NULL, in_conn_str, 0);
    inansi = calloc(len + 1, sizeof(SQLCHAR));
    wcstombs(inansi, in_conn_str, len + 1);

    SQLCHAR outansi[MAX_CONN_INFO_LEN];
    SQLSMALLINT outputlen;

    ret = SQLDriverConnect(hdbc, hwnd, inansi, len, outansi, MAX_CONN_INFO_LEN, &outputlen, drv_completion);

    SQLSMALLINT wcslen = mbstowcs(out_conn_str, outansi, (out_conn_str_max == SQL_NTS) ? MAX_CONN_INFO_LEN : out_conn_str_max);
    if (out_conn_strlen)
        *out_conn_strlen = wcslen * sizeof(SQLWCHAR);

    free(inansi);
    return ret;
}
