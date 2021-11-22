#include "wcs.h"

SQLRETURN SQL_API SQLGetDiagFieldW(SQLSMALLINT   handle_type, 
                                  SQLHANDLE     hndl, 
                                  SQLSMALLINT   rec_no, 
                                  SQLSMALLINT   diag, 
                                  SQLPOINTER    diag_ptr, 
                                  SQLSMALLINT   diag_max, 
                                  SQLSMALLINT   *diag_len)
{
    /* TODO Not implemented yet. We keep this stub because unixODBC need it. */
    NOT_IMPL;
}


SQLRETURN SQL_API SQLGetDiagRecW(
                SQLSMALLINT     handle_type,
                SQLHANDLE       hndl,
                SQLSMALLINT     rec_no,
                SQLWCHAR        *sql_state,
                SQLINTEGER      *native_ptr,
                SQLWCHAR        *msg,
                SQLSMALLINT     msg_max,
                SQLSMALLINT     *msg_len)
{
    SQLRETURN ret;
    SQLCHAR *msg_ansi;
    SQLCHAR sql_state_ansi[6];
    SQLSMALLINT msg_len_ansi;

    __debug("enters method.");

    msg_ansi = malloc(msg_max);

    ret = __SQLGetDiagRec(handle_type, hndl, rec_no, sql_state_ansi, native_ptr, msg_ansi, msg_max, &msg_len_ansi);

    if (ret == SQL_SUCCESS) {
        utf8_to_ucs2(sql_state_ansi, sql_state, 6);
        int len = utf8_to_ucs2(msg_ansi, msg, msg_max);
        if (len > 0 && msg_len != NULL)
             *msg_len = (len - 1);
    }

    free(msg_ansi);
    __debug("leaves method.");
    return SQL_SUCCESS;
}

SQLRETURN SQL_API SQLErrorW(SQLHENV       henv,
                    	    SQLHDBC       hdbc,
                    	    SQLHSTMT      hstmt,
                    	    SQLWCHAR      *szSqlState,
                    	    SQLINTEGER    *pfNativeError,
                    	    SQLWCHAR      *szErrorMsg,
                    	    SQLSMALLINT   cbErrorMsgMax,
                    	    SQLSMALLINT   *pcbErrorMsg)
{
    SQLRETURN ret;
    SQLCHAR *msg_ansi;
    SQLCHAR sql_state_ansi[6];
    SQLSMALLINT msg_len_ansi;

    __debug("enters method.");

    msg_ansi = malloc(cbErrorMsgMax);

    ret = __SQLError(henv, hdbc, hstmt, sql_state_ansi, pfNativeError, msg_ansi, cbErrorMsgMax, &msg_len_ansi);

    if (ret == SQL_SUCCESS) {
        utf8_to_ucs2(sql_state_ansi, szSqlState, 6);
        int len = utf8_to_ucs2(msg_ansi, szErrorMsg, cbErrorMsgMax);
        if (len > 0 && pcbErrorMsg != NULL)
             *pcbErrorMsg = (len - 1);
    }

    free(msg_ansi);
    __debug("leaves method.");
    return SQL_SUCCESS;
}
