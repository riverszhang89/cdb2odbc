#include "driver.h"
#include "convert.h"

#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN 1234
#endif
#ifndef BIG_ENDIAN
#define BIG_ENDIAN 4321
#endif

#if defined(__linux__) || defined(__linux) || defined(linux) /* Linux */
# include <endian.h>
#elif defined(sparc) || defined(sun) /* sparc */
# include <sys/isa_defs.h>
# ifdef _LITTLE_ENDIAN
#  define BYTE_ORDER LITTLE_ENDIAN
# endif

# ifdef _BIG_ENDIAN
#  define BYTE_ORDER BIG_ENDIAN
# endif

#elif defined(_AIX) /* IBM */
# include <sys/machine.h>
#elif defined(_WIN32) /* _WIN32 */
# define BYTE_ORDER LITTLE_ENDIAN
#endif

#ifndef BYTE_ORDER
#error "Could not determine endianness."
#endif

static struct {
    sql_type_t sql_type;              /* Statement type */  
    char *sql_keyword;                /* Statement keyword */
    unsigned long long int sql_value; /* Fast magic number for detecting statement type. */
} STMT_TYPES[] = {
    /* Avoid the inconsistency of little- and big- endian machines, 
       by adding trailing 0s (0x00).
       Please do not remove them. */
    {STMT_SELECT, "SELECT", 
#if BYTE_ORDER == LITTLE_ENDIAN
    0x5443454c4553ULL
#elif BYTE_ORDER == BIG_ENDIAN
    0x53454c4543540000ULL
		fdas
#endif  
    },
    {STMT_INSERT, "INSERT", 
#if BYTE_ORDER == LITTLE_ENDIAN
    0x545245534e49ULL
#elif BYTE_ORDER == BIG_ENDIAN
    0x494e534552540000ULL
#endif
    },
    {STMT_UPDATE, "UPDATE",
#if BYTE_ORDER == LITTLE_ENDIAN
    0x455441445055ULL
#elif BYTE_ORDER == BIG_ENDIAN
    0x5550444154450000ULL
#endif
    },
    {STMT_DELETE, "DELETE", 
#if BYTE_ORDER == LITTLE_ENDIAN
    0x4554454c4544ULL   
#elif BYTE_ORDER == BIG_ENDIAN  
    0x44454c4554450000ULL
#endif  
    }
};

#define MAGIC_SQL_TYPE_NUMBER 0xDFDFDFDFDFDFDFDFULL

const static int TOTAL_TYPE_COUNT = ALEN(STMT_TYPES); 

/**
 * Fast determine statement type.
 */
static sql_type_t stmt_type(const char *sql)
{
    /* Set to 0 to make the most significant 2 bytes become 0s. */
    ULL val = 0; 
    int i;

    __debug("enters method.");

    memcpy(&val, sql, 6);

    for(i = 0; i != TOTAL_TYPE_COUNT; ++i)
        if( !(MAGIC_SQL_TYPE_NUMBER & (val ^ STMT_TYPES[i].sql_value)) ) {
            /* O(1) method equivalent to strncasecmp(a, b, 8) */
            __info("Type determined: %s", STMT_TYPES[i].sql_keyword);
            return STMT_TYPES[i].sql_type;
        }
    
    __debug("leaves method.");

    return STMT_UNDEFINED;
}

/**
 * Fast calculate the occurrence of '?' in @sql.
 */
static unsigned int num_question_markers(const char *sql)
{
    ULL *pword, temp;
    char *pchar;
    unsigned int cnt = 0;

    /* First, count '?' longlong-wise. */
    for(pword = (ULL *)sql; !(((*pword) - 0x0101010101010101ULL) & ~(*pword) & 0x8080808080808080ULL); ++pword) {
        temp = (*pword) ^ 0x3F3F3F3F3F3F3F3FULL;
        cnt += ( ( ( (0x8080808080808080ULL - (temp & 0x7F7F7F7F7F7F7F7FULL)) & ~temp & 0x8080808080808080ULL ) >> 7 ) % 255 );
    }

    /* Now, *ptr contains a zero byte. We count '?' byte-wise. */
    pchar = (char *)pword;
    while(*pchar)
        if(*pchar++ == '?') ++cnt;

    return cnt;
}

/**
 * 1. Determine statement type.
 * 2. Split SQL.
 */
static bool analyze_stmt(stmt_t * phstmt)
{
    char *sql; 

    __debug("enters method.");

    if(!phstmt || !strlen(phstmt->query)) {
        /* Driver manager shall take care of this. Just in case, we set the error manually. */
        STMT_ODBC_ERR(ERROR_INVALID_NULL_PTR);
        return false;
    }

    sql = phstmt->query;

    /* Okay first get rid of leading spaces. */
    while( *sql && isspace(*sql) ) ++sql;

    phstmt->sql_type = stmt_type(sql);
    phstmt->num_params = num_question_markers(sql);

    __debug("leaves method.");
    return true;
}

bool recycle_stmt(stmt_t *phstmt);
SQLRETURN comdb2_SQLPrepare(stmt_t *phstmt, SQLCHAR *str, SQLINTEGER str_len)
{
    __debug("enters method.");
    __info("Prepare %s", str);

    if(!phstmt)
        return SQL_INVALID_HANDLE;

    if(!str)
        return STMT_ODBC_ERR(ERROR_INVALID_NULL_PTR);

    if(str_len < 0) {
        if(str_len != SQL_NTS)
            return STMT_ODBC_ERR(ERROR_INVALID_LENGTH);

        str_len = (int)strlen((char *)str);  
    }

    switch(phstmt->status) {
        case STMT_PREMATURE:
        case STMT_FINISHED:
        case STMT_EXTRACTED:
            __info("Recycle.");
            recycle_stmt(phstmt);

        case STMT_ALLOCATED:
            __info("Allocated. Change status to ready.");
            phstmt->status = STMT_READY;
            break;

        case STMT_READY:
            __warn("Ready. Unrecommended but if you insist, I will change SQL.");
            break;

        case STMT_EXECUTING:
            __warn("Executing.");
            return STMT_ODBC_ERR_MSG(ERROR_FUNCTION_SEQ_ERR, "Statement executing.");

        default:
            __fatal("Unknown status.");
            return STMT_ODBC_ERR_MSG(ERROR_WTH, "Unknown statement status.");
    }

    phstmt->query = strdup((char *)str);
    if(!phstmt->query) 
        return STMT_ODBC_ERR(ERROR_MEM_ALLOC_FAIL);
    
    if(!analyze_stmt(phstmt))
        /* Error message should be set by analyze_stmt. */
        return SQL_ERROR;

    __debug("leaves method.");

    return SQL_SUCCESS;
}

SQLRETURN SQL_API SQLPrepare(SQLHSTMT hstmt, SQLCHAR *str, SQLINTEGER str_len)
{
    return comdb2_SQLPrepare((stmt_t *)hstmt, str, str_len);
}

/**
 * @sqlh
 * @param
 * @name
 */
static conv_resp __bind_param(cdb2_hndl_tp *sqlh, param_t *param)
{
    int i;
    conv_resp resp;

    if(!param) /* Just in case. */
        return CONV_OOPS;

    /* Execute a chain of functions. */
    for(i = 0; i != NUM_CDB2_BINDS; ++i) {
        resp = (*CDB2_BINDS[i])(sqlh, param);
        if(resp != CONV_UNSUPPORTED_C_TYPE)
            /* @resp is not equal to CONV_UNSUPPORTED_C_TYPE ==> the type of @param can be handled by current function. 
               However, errors may be returned due to memory allocation failure, impossible conversion etc. */
            break;
    }
    return resp;
}

/*
   Naive implementation of pow(int, int).
 */
static inline int pow_int(int base, int exp)
{
    int ret = 0;
    while(exp-- > 0)
        ret *= base;
    return ret;
}

/*
   Fast version of fill_out_parameters().
   This function replace a question marker with a named parameter.
   The actual binding is done by CDB2API.
   Instead of reallocating memory for query every time we encounter a bound parameter 
   (that's what fill_out_parameters() does), fast_fill_out_parameters() calculates 
   the space needed by the query before actually filling out the query.
 */
static char *fast_fill_out_parameters(stmt_t *phstmt)
{
    char *param_query, *query = phstmt->query;
    param_t *param = phstmt->params;
    int i, len;
    int param_len = phstmt->params_allocated, stmt_len = (int)strlen(query) - param_len;
    param_t *end_of_params = phstmt->params + param_len;
    errid_t eid = ERROR_NA;
    
    if(!phstmt) /* Just in case. */
        return NULL;

    /* No question marker presented. Just return the plain query. */
    if(phstmt->params_allocated <= 0 || !phstmt->params)
        return query;

    for(cdb2_clearbindings(phstmt->sqlh); param < end_of_params && param->used; ++param) {
        switch(__bind_param(phstmt->sqlh, param)) {
            case CONV_BUF_OVERFLOW:
            case CONV_TRUNCATED:
                /* According to ODBC standard, string truncated warning should not be reported when binding a parameter. */
            case CONV_YEAH:
                eid = ERROR_NA;
                break;

            case CONV_MEM_FAIL:
                eid = ERROR_MEM_ALLOC_FAIL;
                break;

            case CONV_IMPOSSIBLE:
                /* FIXME HY004 (Invalid SQL type) is currently treated as 07006. */
                eid = ERROR_CANNOT_CONVERT;
                break;

            case CONV_UNSUPPORTED_C_TYPE:
                eid = ERROR_INVALID_APP_BUF_TYPE;
                break;

            case CONV_TRUNCATED_WHOLE:
                eid = ERROR_INVALID_STRING_FOR_CASTING;
                break;

            case CONV_INVALID_BUFLEN:
                eid = ERROR_INVALID_LENGTH;
                break;

            case CONV_INTERNAL_ERR:
            default:
                eid = ERROR_WTH;
                break;
        }
    }

    if(eid != ERROR_NA) {
        STMT_ODBC_ERR(eid);
        return NULL;
    }

    if(!phstmt->changed) 
        /* Parameters are unchanged. No need to refill out all parameters. 
           Just return what we have. */
        return phstmt->query_with_params;

    /****************************
     * Parameters were changed. *
     ****************************/

    if(param_len) {
        for(i = 0, len = param_len; (len /= 10) > 0; ++i) ; 
        stmt_len += (param_len - (int) pow_int(10, i)) * ((i + 1) + 6); /* `6' is for "@param". */

        for(; i > 0; --i) 
            stmt_len += 9 * pow_int(10, i - 1) * (6 + i);
    }

    if((phstmt->query_with_params = realloc(phstmt->query_with_params, stmt_len + 1)) == NULL) {
        STMT_ODBC_ERR(ERROR_MEM_ALLOC_FAIL);
        return NULL;
    }

    __info("%d bytes are allocated for parameterized statement.", stmt_len + 1);

    for(param = phstmt->params, param_query = phstmt->query_with_params; *query != '\0'; ++query) {
        if(*query != '?' || param >= end_of_params)
            *param_query++ = *query;
        else {
            if(!param->used)
                *param_query++ = *query;
            else {
                strcpy(param_query, param->name);
                param_query += strlen(param->name);
            }
            ++param;
        }
    }
    *param_query = '\0';

    /* Once we fill out all markers, mark the statement as unchanged. */
    phstmt->changed = false;
    __info("%zd bytes are used for parameterized statement.", strlen(phstmt->query_with_params) + 1);

    return phstmt->query_with_params;
}

static int ctype_to_cdb2type(int ctype)
{
    switch(ctype) {
        case SQL_C_CHAR:
        case SQL_C_WCHAR:
        case SQL_C_NUMERIC:
            return CDB2_CSTRING;

        case SQL_C_SSHORT:
        case SQL_C_SHORT:
        case SQL_C_USHORT:
        case SQL_C_SLONG:
        case SQL_C_LONG:
        case SQL_C_ULONG:
        case SQL_C_STINYINT:
        case SQL_C_TINYINT:
        case SQL_C_UTINYINT:
        case SQL_C_SBIGINT:
        case SQL_C_UBIGINT:
        case SQL_C_BIT:
            return CDB2_INTEGER;

        case SQL_C_FLOAT:
        case SQL_C_DOUBLE:
            return CDB2_REAL;

        case SQL_C_TYPE_DATE:
        case SQL_C_TYPE_TIME:
        case SQL_C_TYPE_TIMESTAMP:
            return CDB2_DATETIME;
        
        case SQL_C_INTERVAL_MONTH:
        case SQL_C_INTERVAL_YEAR:
        case SQL_C_INTERVAL_YEAR_TO_MONTH:
            return CDB2_INTERVALYM;

        case SQL_C_INTERVAL_DAY:
        case SQL_C_INTERVAL_HOUR:
        case SQL_C_INTERVAL_MINUTE:
        case SQL_C_INTERVAL_SECOND:
        case SQL_C_INTERVAL_DAY_TO_HOUR:
        case SQL_C_INTERVAL_DAY_TO_MINUTE:
        case SQL_C_INTERVAL_DAY_TO_SECOND:
        case SQL_C_INTERVAL_HOUR_TO_MINUTE:
        case SQL_C_INTERVAL_HOUR_TO_SECOND:
        case SQL_C_INTERVAL_MINUTE_TO_SECOND:
            return CDB2_INTERVALDS;

        case SQL_C_BINARY:
            return CDB2_BLOB;
        default:
            return CONV_UNSUPPORTED_C_TYPE;
    }
}

SQLRETURN comdb2_SQLExecute(stmt_t *phstmt)
{
    cdb2_hndl_tp *sqlh;
    int rc, i, *types = NULL;
    char *actual_query;
    data_buffer_t *data;

    __debug("enters method.");

    if(!phstmt)
        return SQL_INVALID_HANDLE;

    switch(phstmt->status) {
        case STMT_ALLOCATED:
            return STMT_ODBC_ERR_MSG(ERROR_FUNCTION_SEQ_ERR, "No query is attached.");

        case STMT_PREMATURE:
            return SQL_SUCCESS;
        
        case STMT_EXECUTING:
            return SQL_STILL_EXECUTING;

        case STMT_FINISHED:
            if(SQLH_STATUS(phstmt) == SQLH_FINISHED) {
                __info("Clear unextracted rows.");
                while((rc = cdb2_next_record(phstmt->sqlh)) == CDB2_OK);
                SET_SQLH_IDLE(phstmt);
            }
            break;

        default:
            break;
    }

    /* Check status of cdb2 handle. */
    switch(SQLH_STATUS(phstmt)) {
        case SQLH_EXECUTING:
            /* Another statement handle is executing. */
            return SQL_STILL_EXECUTING;

        case SQLH_FINISHED:
            /* Another statement handle completed. */
            __warn("Clear unextracted rows. All pending results will be discarded.");
            while((rc = cdb2_next_record(phstmt->sqlh)) == CDB2_OK);
            break;
        default:
            break;
    }

    sqlh = phstmt->sqlh;

    if(!AUTO_COMMIT(phstmt) && IS_OUT_OF_TXN(phstmt)) {
        /* If the driver is set to commit transaction manually and currrent statement
           is not in transaction, the driver sends 'BEGIN' to comdb2 to open a new txn session. */
        if(phstmt->dbc->txn_changed && IS_VALID_TXN_MODE(phstmt)) {
            __info("Sets transaction mode: %s", TXN_MODE(phstmt));
            if((rc = cdb2_run_statement(sqlh, TXN_MODE(phstmt))) != 0) {
                __fatal("Cannot set txn mode.");
                return set_stmt_error(phstmt, ERROR_WTH, cdb2_errstr(sqlh), rc);
            }
            phstmt->dbc->txn_changed = false;
            while((rc = cdb2_next_record(phstmt->sqlh)) == CDB2_OK);
        }

        __info("implicitly sending `BEGIN' to comdb2.");

        if((rc = cdb2_run_statement(sqlh, "BEGIN")) != 0) {
            __fatal("Cannot open new txn session: %s", cdb2_errstr(sqlh));
            SET_EXTRACTED(phstmt);
            return set_stmt_error(phstmt, ERROR_WTH, cdb2_errstr(sqlh), rc);
        }
        __info("Enters txn session.");
        IN_TXN(phstmt);
        /* FIXME Seems a bug here. After executing `BEGIN', @cdb2_next_record() must be called to clear cache. */
        while((rc = cdb2_next_record(phstmt->sqlh)) == CDB2_OK);
    }

    if(phstmt->num_data_buffers > 0 && (data = phstmt->buffers) != NULL) {
        types = malloc(phstmt->num_data_buffers);
		if (types == NULL)
            return STMT_ODBC_ERR(ERROR_MEM_ALLOC_FAIL);
        for(i = 0; i != phstmt->num_data_buffers; ++i) {
            if(data->used)
                types[i] = ctype_to_cdb2type(data->c_type);
            else {
                __warn("Column %d is unused. Its type is set to CDB2_BLOB", i);
                types[i] = CDB2_BLOB;
            }
        }
    }

    /* Copy parameters to query string. */
    if((actual_query = fast_fill_out_parameters(phstmt)) == NULL) 
        /* Error message is set in fill_out_parameter(). */
        return SQL_ERROR;

    __info("sending `%s' along with %d param(s) to comdb2.", actual_query, phstmt->params_allocated);

    /* TODO It makes more sense in multithreaded code. */
    SET_EXECUTING(phstmt);
    rc = cdb2_run_statement_typed(sqlh, actual_query, phstmt->num_data_buffers, types);
	free(types);
    if(rc) {
        /* Set status to extracted to prevent applications from calling SQLFetch 
           (may cause CDB2API stuck in an infinite loop). */
        __fatal("error: %s", cdb2_errstr(sqlh) ? cdb2_errstr(sqlh) : "<null>");
        SET_EXTRACTED(phstmt);
        return set_stmt_error(phstmt, ERROR_WTH, cdb2_errstr(sqlh), rc);
    }

    SET_FINISHED(phstmt);
    phstmt->col_count = cdb2_numcolumns(sqlh);
    
    __debug("col_count: %d", phstmt->col_count);
    __debug("leaves method.");

    return SQL_SUCCESS;
}

SQLRETURN SQL_API SQLExecute(SQLHSTMT hstmt)
{
    return comdb2_SQLExecute((stmt_t *)hstmt);
}

SQLRETURN comdb2_SQLExecDirect(
        stmt_t     *phstmt,
        SQLCHAR    *sql,
        SQLINTEGER len)
{
    SQLRETURN ret;

    __debug("enters method.");

    if(!phstmt) 
        return SQL_INVALID_HANDLE;

    if(SQL_SUCCEEDED(ret = comdb2_SQLPrepare(phstmt, sql, len))) 
        ret = comdb2_SQLExecute(phstmt);

    __debug("leaves method.");

    /* The error message should be set inside comdb2_SQL*.
       So we simply return the retval. */
    return ret;
}

SQLRETURN SQL_API SQLExecDirect(SQLHSTMT        hstmt,
                                SQLCHAR         *sql,
                                SQLINTEGER      len)
{
    return comdb2_SQLExecDirect((stmt_t *)hstmt, sql, len);
}

SQLRETURN SQL_API SQLNumParams(
        SQLHSTMT        stmt,
        SQLSMALLINT     *count)
{
    stmt_t *phstmt = (stmt_t *)stmt;

    if(!stmt)
        return SQL_INVALID_HANDLE;

    *count = (SQLSMALLINT) phstmt->num_params;
    return SQL_SUCCESS;
}

SQLRETURN SQL_API SQLBindParameter(SQLHSTMT         hstmt,
                                   SQLUSMALLINT     param_num,
                                   SQLSMALLINT      io_type,
                                   SQLSMALLINT      c_type,
                                   SQLSMALLINT      sql_type,
                                   SQLULEN          col_size,
                                   SQLSMALLINT      dec_digits,
                                   SQLPOINTER       param_val_ptr,
                                   SQLLEN           buf_len,
                                   SQLLEN           *str_len)
{
    stmt_t *phstmt = (stmt_t *)hstmt;
    param_t *param, *params;
    int i;

    __debug("enters method.");

    if(!hstmt)
        return SQL_INVALID_HANDLE;
    
    /* TODO Currently the driver only supports INPUT mode. */
    if(io_type != SQL_PARAM_INPUT)
        return STMT_ODBC_ERR_MSG(ERROR_NOT_IMPL, "OUTPUT mode is not supported. Please use SQLGetData instead.");
    
    if(str_len && (*str_len == SQL_DATA_AT_EXEC || *str_len <= SQL_LEN_DATA_AT_EXEC_OFFSET))
        /* https://mariadb.com/kb/en/about-sqlcli-deferred-parameter-functions 
           The link above might serve as a reason for not supporting data-at-exec. */
        return STMT_ODBC_ERR_MSG(ERROR_NOT_IMPL, "Data-at-Execution is not supported.");

    phstmt->changed = true;
	params = phstmt->params;

    if(param_num > phstmt->params_allocated) {
        /* Realloc an array of new length. */
        params = (param_t *)realloc(phstmt->params, param_num * sizeof(param_t));
        if(!params)
            return STMT_ODBC_ERR(ERROR_MEM_ALLOC_FAIL);
        
        /* Mark implicitly allocated parameter as unused. Consider a very rare case.
           There were no bound parameter, and someone binds ONLY the 3rd parameter. As a result,
           the 1st and 2nd parameter are allocated implicitly and shouldn't be used. */

        for(i = phstmt->params_allocated; i < param_num - 1; ++i)
            params[i].used = false;

        /* Allocation succeeded. 
           1. Assign the new addr to the struct member variable.
           2. Update the number. */
        phstmt->params = params;
        phstmt->params_allocated = param_num;
    }

    // 0-based index
    --param_num;
    param = &params[param_num];

    /* Copy all given information. */
    param->used = true;
    param->io_type = io_type;
    param->c_type = c_type;
    param->sql_type = sql_type;
    param->precision = (unsigned int)col_size;
    param->scale = dec_digits;
    param->buflen = buf_len;
    param->str_len = str_len;
    param->buf = param_val_ptr;
    param->internal_buffer = NULL;
    sprintf(param->name, "@_odbc_autogen_param%d", param_num);

    __debug("leaves method.");
    return SQL_SUCCESS;
}

/*
   ODBC API.
   SQLBindCol binds application data buffers to columns in the result set.

   FIXME Currently, SQLBindCol MUST be called BEFORE calling SQLExecute.
   We rely on comdb2 server to do conversions. Hence, type information must be sent to comdb2
   along with the statement. As a result, applications cannot alter field types after the execution.
   Although ODBC states it's legal to binding columns after executing a statement, this shouldn't
   be a big issue for applications.

   For forward compatibility, the order of function calls is not strictly restricted.
   If an application calls SQLBindCol after executing a statement, it still can get expected data as long as
   the @c_type is the same as the type defined in the csc2 schema file. (e.g., if the type in the schema is int,
   and @c_type is an integral type, it is ok. On the other hand, if @c_type is a string type, an error
   shall be reported to the application.
   Again, to alter the retrieved type (different from the schema), applications must call SQLBindCol before the execution.
 */
SQLRETURN SQL_API SQLBindCol(SQLHSTMT       hstmt,
                             SQLUSMALLINT   col,
                             SQLSMALLINT    c_type,
                             SQLPOINTER     target_ptr,
                             SQLLEN         target_len,
                             SQLLEN         *strlen_or_ind)
{
    stmt_t *phstmt = (stmt_t *)hstmt;
    data_buffer_t *data, *data_list;
    int i;
    
    __debug("enters method.");

    if(!hstmt)
        return SQL_INVALID_HANDLE;
    
	data_list = phstmt->buffers;
    if(col > phstmt->num_data_buffers) {
        data_list = (data_buffer_t *)realloc(phstmt->buffers, col * sizeof(data_buffer_t));
        if(!data_list)
            return STMT_ODBC_ERR(ERROR_MEM_ALLOC_FAIL);

        for(i = phstmt->num_data_buffers; i < col - 1; ++i)
            data_list[i].used = 0;
        
        phstmt->buffers = data_list;
        phstmt->num_data_buffers = col;
    }

    --col;
    data = &data_list[col];
    data->c_type = c_type;
    data->buffer_length = target_len;
    data->required = strlen_or_ind;
    data->buffer = target_ptr;
	/* @target_ptr serves as a flag to determine if the column needs to be unbinded.
	   See http://msdn.microsoft.com/en-us/library/ms711010(v=vs.85).aspx for details. */
	data->used = target_ptr ? 1 : 0;

    __debug("leaves method.");

    return SQL_SUCCESS;
}

static SQLRETURN comdb2_conn_transact(
        dbc_t           *phdbc,
        SQLSMALLINT     commit_or_rollback)
{
    int rc = 0;
    __debug("Enters method.");

    if (phdbc && !phdbc->auto_commit && phdbc->in_txn && phdbc->sqlh) {
        
        if(phdbc->sqlh_status == SQLH_EXECUTING)
            return SQL_STILL_EXECUTING;

        __info("ENDS transaction.");

        if(phdbc->sqlh_status == SQLH_FINISHED)
            /* In case we have any unfetched rows. */
            while(cdb2_next_record(phdbc->sqlh) == CDB2_OK);

        switch(commit_or_rollback) {
            case SQL_COMMIT:
                rc = cdb2_run_statement(phdbc->sqlh, "COMMIT");
                break;
            case SQL_ROLLBACK:
                rc = cdb2_run_statement(phdbc->sqlh, "ROLLBACK");
                break;
        }

        /* Either success or failure, the @sql_status needs to set to IDLE. */
        phdbc->sqlh_status = SQLH_IDLE;

        if(rc) {
            __fatal("Cannot commit/rollback txn session: %s", cdb2_errstr(phdbc->sqlh));
            return set_dbc_error(phdbc, ERROR_WTH, cdb2_errstr(phdbc->sqlh), rc);
        }

        /* FIXME Seems a bug here. After executing `COMMIT/ROLLBACK', @cdb2_next_record() must be called to clear cache. */
        while(cdb2_next_record(phdbc->sqlh) == CDB2_OK);
        phdbc->in_txn = false;
    }

    __debug("Leaves method.");

    return (rc ? set_dbc_error(phdbc, ERROR_WTH, cdb2_errstr(phdbc->sqlh), rc) : SQL_SUCCESS);
}

static SQLRETURN comdb2_SQLEndTran(
        SQLSMALLINT   type,
        SQLHANDLE     handle,
        SQLSMALLINT   commit_or_rollback)
{
    env_t *phenv;
    dbc_t *phdbc;
    
    switch(type) {
        case SQL_HANDLE_ENV:
            phenv = (env_t *)handle;
            list_iterate(phdbc, &phenv->conns, list, dbc_t)
                comdb2_conn_transact(phdbc, commit_or_rollback);
            break;

        case SQL_HANDLE_DBC:
            phdbc = (dbc_t *)handle;
            return comdb2_conn_transact(phdbc, commit_or_rollback);
        
        default:
            return SQL_ERROR;
    }

    return SQL_SUCCESS;
}

SQLRETURN SQL_API SQLEndTran(
        SQLSMALLINT   type,
        SQLHANDLE     handle,
        SQLSMALLINT   commit_or_rollback)
{
    return comdb2_SQLEndTran(type, handle, commit_or_rollback);
}

SQLRETURN SQL_API SQLTransact(
        SQLHENV         henv,
        SQLHDBC         hdbc,
        SQLUSMALLINT    commit_or_rollback)
{
    return ( hdbc ? comdb2_SQLEndTran(SQL_HANDLE_DBC, hdbc, commit_or_rollback)
            : comdb2_SQLEndTran(SQL_HANDLE_ENV, henv, commit_or_rollback) );
}
