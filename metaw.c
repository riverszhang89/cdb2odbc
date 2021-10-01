#include <stringapiset.h>
#include <wchar.h>

SQLRETURN SQL_API SQLGetInfoW(
        SQLHDBC         hdbc,
        SQLUSMALLINT    type,
        SQLPOINTER      value_ptr,
        SQLSMALLINT     buflen,
        SQLSMALLINT     *str_len)
{
    int len = 0;
    SQLSMALLINT ansistrlen, writablelen;
    SQLPOINTER *val;
    __debug("enters method.");

    switch (type) {
    case SQL_DATABASE_NAME:
    case SQL_DBMS_NAME:
    case SQL_DBMS_VER:
    case SQL_DRIVER_NAME:
    case SQL_DRIVER_VER:
    case SQL_DRIVER_ODBC_VER:
	val = malloc(buflen / sizeof(SQLWCHAR));
	writablelen = buflen / sizeof(SQLWCHAR) - 1;
	break;
    default:
	val = value_ptr;
	writablelen = buflen;
	break;
    }

    SQLRETURN ret = SQLGetInfo(hdbc, type, val, writablelen, &ansistrlen);

    if (ret == SQL_SUCCESS) {
        switch (type) {
        case SQL_DATABASE_NAME:
        case SQL_DBMS_NAME:
        case SQL_DBMS_VER:
        case SQL_DRIVER_NAME:
        case SQL_DRIVER_VER:
        case SQL_DRIVER_ODBC_VER:
            len = MultiByteToWideChar(
                            CP_UTF8,
                            0,
                            (SQLCHAR *)val,
                            ansistrlen,
                            value_ptr,
                            writablelen + 1);
            if (len > 0 && str_len != NULL)
                *str_len = wcslen((SQLWCHAR *)val) *  sizeof(SQLWCHAR);
	    free(val);
            break;
        }
    }

    __debug("leaves method.");
    return 0;
}
