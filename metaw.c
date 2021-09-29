SQLRETURN SQL_API SQLGetInfoW(
        SQLHDBC         hdbc,
        SQLUSMALLINT    type,
        SQLPOINTER      value_ptr,
        SQLSMALLINT     buflen,
        SQLSMALLINT     *str_len)
{
    SQLPOINTER *val = malloc(buflen);
    SQLRETURN ret = SQLGetInfo(hdbc, type, val, buflen, str_len);

    switch (type) {
        case SQL_DATABASE_NAME:
        case SQL_DBMS_NAME:
        case SQL_DBMS_VER:
        case SQL_DRIVER_NAME:
        case SQL_DRIVER_VER:
        case SQL_DRIVER_ODBC_VER:
            mbstowcs(value_ptr, (const char *)val, str_len + 1);
            if (str_len)
                *str_len *= sizeof(SQLWCHAR);
            break;
    }
    free(val);
    return ret;
}
