#include "driver.h"

#include <string.h>

const static struct err_msg {
    char *state;        /* sql state */
    char *msg;          /* error message */
    SQLRETURN ret;      /* SQLRETURN */
} COMDB2_ERR_MSGS[] = {

    /* The @state can be duplicated. 
       But different messages are presented if multiple elements share a common @state */

      [ERROR_WTH] = {"HY000", "General Error", SQL_ERROR}
    , [ERROR_UNIMPL_ATTR] = {"HY000", "Unimplemented attribute", SQL_ERROR}
    , [ERROR_MEM_ALLOC_FAIL] = {"HY001", "Memory allocation error", SQL_ERROR}
    , [ERROR_FUNCTION_SEQ_ERR] = {"HY010", "Function sequence error", SQL_ERROR}
    , [ERROR_OPTION_OUT_OF_RANGE] = {"HY091", "Option type out of range", SQL_ERROR}
    , [ERROR_NVM] = {"01000", "General warning", SQL_SUCCESS_WITH_INFO}
    , [ERROR_STR_TRUNCATED] = {"01004", "String data, right truncated", SQL_SUCCESS_WITH_INFO}
    , [ERROR_UNABLE_TO_CONN] = {"08001", "Client unable to establish connection", SQL_ERROR}
    , [ERROR_CONN_IN_USE] = {"08002", "Connection name in use", SQL_ERROR}
    , [ERROR_CONN_NOT_OPEN] = {"08003", "Connection not open", SQL_SUCCESS_WITH_INFO}
    , [ERROR_NOT_IMPL] = {"IM001", "Driver does not support this function", SQL_ERROR}
    , [ERROR_NO_CONF] = {"IM002", "Data source not found and no default driver specified", SQL_ERROR}
    , [ERROR_INVALID_TRANS_STATE] = {"25000", "Invalid transaction state", SQL_ERROR}
    , [ERROR_INVALID_DESC_IDX] = {"07009", "Invalid descriptor index", SQL_ERROR}
    , [ERROR_INVALID_DESC_FIELD_ID] = {"HY091", "Invalid descriptor field identifier", SQL_ERROR}
    , [ERROR_INVALID_NULL_PTR] = {"HY009", "Invalid use of null pointer", SQL_ERROR}
    , [ERROR_INVALID_LENGTH] = {"HY090", "Invalid string or buffer length", SQL_ERROR}
    , [ERROR_IND_REQUIRED] = {"22002", "Indicator variable required but not supplied", SQL_ERROR}
    , [ERROR_PROG_OUT_OF_RANGE] = {"HY003", "Program type out of range", SQL_ERROR}
    , [ERROR_TYPE_OUT_OF_RANGE] = {"HY096", "Information type out of range", SQL_ERROR}
    , [ERROR_CANNOT_CONVERT] = {"07006", "Restricted data type attribute violation", SQL_ERROR}
    , [ERROR_INVALID_APP_BUF_TYPE] = {"HY003", "Invalid application buffer type", SQL_ERROR}
    , [ERROR_INVALID_STRING_FOR_CASTING] = {"22018", "Invalid character value for cast specification", SQL_ERROR}
    , [ERROR_UNSUPPORTED_OPTION_VALUE] = {"01S02", "The option value is not supported", SQL_ERROR}
};

const static int TOTAL_ERR_COUNT = ALEN(COMDB2_ERR_MSGS);

/* Copy @errid, native_code to @err. If @customized_msg given, copy it as well. 
    Otherwise the message field shall be filled by a default error message. */
SQLRETURN comdb2_set_error(struct err **err, errid_t errid, const char *customized_msg, int native_code)
{
    struct err_msg default_msg;
    const char *state, *msg;
    SQLRETURN retcode;
    struct err *deref_err;

    __debug("enters method.");
    __debug("errid = %d, custom msg = %s", errid, customized_msg);

    if(!err)
        return SQL_INVALID_HANDLE;

    if(errid < 0 || errid >= TOTAL_ERR_COUNT) 
        return comdb2_set_error(err, ERROR_WTH, NULL, ODBC_ERROR_CODE);

    /* default message */
    default_msg = COMDB2_ERR_MSGS[errid];
    state = default_msg.state;
    msg = (customized_msg ? customized_msg : default_msg.msg);
    retcode = default_msg.ret;

    deref_err = (struct err *)realloc(*err, sizeof(struct err) + strlen(msg));
    
    if(!deref_err) {
        /* Memory allocation failure indicates lack of memory. In this case, we don't 
           set error again in case of infinite loop. Simply return SQL_ERROR. */
        return SQL_ERROR;
    }

    strcpy(deref_err->msg, msg);

    /* copy error */
    deref_err->sql_state[0] = '\0';
    strcpy(deref_err->sql_state, state);
    deref_err->native_err_code = native_code;

    *err = deref_err;

    return retcode;
}

ERR_FUNC(env);
ERR_FUNC(dbc);
ERR_FUNC(stmt);

SQLRETURN SQL_API SQLGetDiagRec(SQLSMALLINT     handle_type, 
                                SQLHANDLE       hndl, 
                                SQLSMALLINT     rec_no, 
                                SQLCHAR         *sql_state, 
                                SQLINTEGER      *native_ptr, 
                                SQLCHAR         *msg, 
                                SQLSMALLINT     msg_max, 
                                SQLSMALLINT     *msg_len)
{
    err_t *err;

    __debug("enters method.");
    __debug("handle_type is %d, sql_state is %s, native_ptr is %p, recno is %d",
            handle_type, sql_state, native_ptr, rec_no);

    if(!hndl)
        return SQL_INVALID_HANDLE;

    /* We only support 1 error per handle. */
    if (rec_no > 1)
        return SQL_NO_DATA;
    
    if(!sql_state || !native_ptr || msg_max < 0 || rec_no > 1)
        return SQL_ERROR;

    switch(handle_type) {
        case SQL_HANDLE_STMT:
            err = ((struct stmt *)hndl)->error;
            break;
        case SQL_HANDLE_DBC:
            err = ((struct dbc *)hndl)->error;
            break;
        case SQL_HANDLE_ENV:
            err = ((struct env *)hndl)->error;
            break;
        case SQL_HANDLE_DESC:
        default:
            return SQL_ERROR;
    }

    __debug("sql state %s", err->sql_state);

    if(!err->sql_state || !strlen(err->sql_state))
        return SQL_NO_DATA;

    /* copy error */
    strcpy((char *)sql_state, err->sql_state);
    *native_ptr = err->native_err_code;

    if(msg) my_strncpy_out_fn((char *)msg, err->msg, msg_max);

    *msg_len = (SQLSMALLINT)strlen(err->msg);

    __debug("leaves method.");
    return SQL_SUCCESS;
}

SQLRETURN SQL_API SQLGetDiagField(SQLSMALLINT   handle_type, 
                                  SQLHANDLE     hndl, 
                                  SQLSMALLINT   rec_no, 
                                  SQLSMALLINT   diag, 
                                  SQLPOINTER    diag_ptr, 
                                  SQLSMALLINT   diag_max, 
                                  SQLSMALLINT   *diag_len)
{
    /* TODO Not implemented yet. We keep this stub because unixODBC need it. */
    __debug("enters method.");
    NOT_IMPL;
}
