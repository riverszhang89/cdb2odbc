SQLRETURN SQL_API SQLGetInfoW(
        SQLHDBC         hdbc,
        SQLUSMALLINT    type,
        SQLPOINTER      value_ptr,
        SQLSMALLINT     buflen,
        SQLSMALLINT     *str_len)
{
    int len = 2;
    SQLSMALLINT ansistrlen;
    SQLPOINTER *val = malloc(buflen);

#if 0
    SQLRETURN ret = SQLGetInfo(hdbc, type, val, buflen, &ansistrlen);

    if (ret == SQL_SUCCESS) {
       switch (type) {
       case SQL_DATABASE_NAME:
       case SQL_DBMS_NAME:
       case SQL_DBMS_VER:
       case SQL_DRIVER_NAME:
       case SQL_DRIVER_VER:
       case SQL_DRIVER_ODBC_VER:
           len = MultiByteToWideChar(CP_UTF8, 0, (SQLCHAR *)val, ansistrlen, (SQLWCHAR *)value_ptr, buflen / sizeof(SQLWCHAR));
           if (len > 0)
               *str_len = len * sizeof(SQLWCHAR);
           break;
       }
    }
#endif
    free(val);
    return 0;
}
