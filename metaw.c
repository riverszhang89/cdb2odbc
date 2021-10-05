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
    SQLPOINTER val;
    __debug("enters method.");

    switch (type) {
    case SQL_DATABASE_NAME:
    case SQL_DBMS_NAME:
    case SQL_DBMS_VER:
    case SQL_DRIVER_NAME:
    case SQL_DRIVER_VER:
    case SQL_DRIVER_ODBC_VER:
	val = malloc(buflen / sizeof(SQLWCHAR) + 10);
	writablelen = buflen / sizeof(SQLWCHAR);
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
                            -1,
                            value_ptr,
                            writablelen);
            if (len > 0 && str_len != NULL)
                *str_len = (len - 1) *  sizeof(SQLWCHAR);
	    free(val);
            break;
	default:
	    break;
        }
    }

    __debug("leaves method.");
    return SQL_SUCCESS;
}
