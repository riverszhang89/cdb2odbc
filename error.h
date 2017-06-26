#ifndef _ERROR_H_
#define _ERROR_H_

typedef struct err {
    char sql_state[6];      /* a state is a 5-character string. */
    int native_err_code;    /* native error code of comdb2 API */


    /* Keep the line below at the bottom of the struct definition.
       Additional information should be added above it. */

    char msg[1];            /* this can be a customized message or an odbc-standard message */
} err_t;

err_t *before_christ_error;     /* error handle. it is used only before any handle (env, conn or stmt) is initialized. */

#define unset_bc_error before_christ_error.sql_state[0] = '\0'
#define is_bc_error strlen(before_christ_error.sql_state)

typedef enum {

    /* ODBC error codes. 
        See http://msdn.microsoft.com/en-us/library/ms714687(v=vs.85).aspx for details. */

    
      ERROR_NA = -1                         /* Not available */
    , ERROR_NVM = 0                         /* General warning */
    , ERROR_MEM_ALLOC_FAIL
    , ERROR_FUNCTION_SEQ_ERR
    , ERROR_OPTION_OUT_OF_RANGE
    , ERROR_WTH                             /* General error */
    , ERROR_UNIMPL_ATTR
    , ERROR_STR_TRUNCATED
    , ERROR_UNABLE_TO_CONN
    , ERROR_CONN_IN_USE
    , ERROR_CONN_NOT_OPEN
    , ERROR_NOT_IMPL
    , ERROR_NO_CONF
    , ERROR_INVALID_TRANS_STATE
    , ERROR_INVALID_DESC_IDX
    , ERROR_INVALID_DESC_FIELD_ID
    , ERROR_INVALID_NULL_PTR
    , ERROR_INVALID_LENGTH
    , ERROR_IND_REQUIRED
    , ERROR_PROG_OUT_OF_RANGE
    , ERROR_TYPE_OUT_OF_RANGE
    , ERROR_CANNOT_CONVERT
    , ERROR_INVALID_APP_BUF_TYPE
    , ERROR_INVALID_STRING_FOR_CASTING
    , ERROR_UNSUPPORTED_OPTION_VALUE

} errid_t;

/* -1 is used as native error code for ODBC-defined errors (e.g., memory allocation failure, null pointer...). */
#define ODBC_ERROR_CODE -1

struct env;
struct dbc;
struct stmt;

extern SQLRETURN comdb2_set_error(err_t **, errid_t, const char *, int);

#define ERR_FUNC_PROTO(type) \
extern SQLRETURN set_ ## type ## _error(struct type *, errid_t, const char *, int)

#define ERR_FUNC(type)  \
SQLRETURN set_ ## type ## _error(struct type *handle, errid_t errid,     \
        const char *customized_msg, int native_id)                              \
{                                                                               \
    if(!handle) return SQL_INVALID_HANDLE;                                      \
    return comdb2_set_error(&handle->error, errid, customized_msg, native_id);  \
}                                                                               \
extern int no_such_variable

ERR_FUNC_PROTO(env);
ERR_FUNC_PROTO(dbc);
ERR_FUNC_PROTO(stmt);

#define ENV_ODBC_ERR(errid) set_env_error(phenv, errid, NULL, ODBC_ERROR_CODE)
#define DBC_ODBC_ERR(errid) set_dbc_error(phdbc, errid, NULL, ODBC_ERROR_CODE)
#define STMT_ODBC_ERR(errid) set_stmt_error(phstmt, errid, NULL, ODBC_ERROR_CODE)
#define ENV_ODBC_ERR_MSG(errid, msg) set_env_error(phenv, errid, msg, ODBC_ERROR_CODE)
#define DBC_ODBC_ERR_MSG(errid, msg) set_dbc_error(phdbc, errid, msg, ODBC_ERROR_CODE)
#define STMT_ODBC_ERR_MSG(errid, msg) set_stmt_error(phstmt, errid, msg, ODBC_ERROR_CODE)

#define NOT_IMPL return SQL_ERROR 
#define NOT_IMPL_ENV(env) return set_env_error(env, ERROR_NOT_IMPL, NULL, ODBC_ERROR_CODE)
#define NOT_IMPL_CONN(dbc) return set_dbc_error(dbc, ERROR_NOT_IMPL, NULL, ODBC_ERROR_CODE)
#define NOT_IMPL_STMT(stmt) return set_stmt_error(stmt, ERROR_NOT_IMPL, NULL, ODBC_ERROR_CODE)

#endif /* _ERROR_H_ */
