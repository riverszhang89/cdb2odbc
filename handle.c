#include "driver.h"
#include "list.h"

/**
 * Declarations for internal comdb2_Free* functions.
 */
static SQLRETURN comdb2_SQLFreeEnv(env_t *phenv);
static SQLRETURN comdb2_SQLFreeConnect(dbc_t *phdbc);
static SQLRETURN comdb2_SQLFreeStmt(stmt_t *phstmt);

/*
   COMDB2-ODBC internal.

   Allocates an environment handle.
 */
static SQLRETURN comdb2_SQLAllocEnv(SQLHENV *phenv)
{
    SQLRETURN ret;
    env_t *env;
    __debug("enters method.");

    if((*phenv = my_calloc_typed(env_t, 1, SQLHENV)) == NULL) 
        ret = SQL_ERROR;
    else {
        env = (env_t *) *phenv;
        zerofill(env, env_t, 1);
        INIT_LIST_HEAD(&env->conns);
        ret = SQL_SUCCESS;
    }

    __debug("leaves method.");
    return ret;
}

/*
   COMDB2-ODBC internal.

   Frees an environment handle.
 */
static SQLRETURN comdb2_SQLFreeEnv(env_t *phenv)
{
    SQLRETURN ret = SQL_SUCCESS;
    dbc_t *dbc, *next;

    __debug("enters method.");

    if(!phenv)
        return SQL_INVALID_HANDLE;

    /* Free all associated connection handles. */   
    list_iterate_safe(dbc, next, &phenv->conns, list, dbc_t)
        if(SQL_FAILED(ret = comdb2_SQLFreeConnect(dbc)))
            return ret;
    
    free(phenv->error);
    free(phenv);

    __debug("leaves method.");
    
    return ret;
}

/*
   COMDB2-ODBC internal.

   Allocates a connection handle.
 */
static SQLRETURN comdb2_SQLAllocConnect(SQLHENV henv,
                                 SQLHDBC *phdbc)
{
    SQLRETURN ret;
    dbc_t *dbc;
    env_t *phenv = (env_t *)henv;

    __debug("enters method.");

    if(!henv)
        return SQL_INVALID_HANDLE;

    if((*phdbc = my_calloc_typed(dbc_t, 1, SQLHDBC)) == NULL) 
        return ENV_ODBC_ERR(ERROR_MEM_ALLOC_FAIL);
    else {
        dbc = (dbc_t *) *phdbc;
        zerofill(*phdbc, dbc_t, 1);
        dbc->env = phenv;
        dbc->auto_commit = SQL_AUTOCOMMIT_DEFAULT;
        dbc->brand_new = true;
        list_append(&dbc->list, &phenv->conns); 
        INIT_LIST_HEAD(&dbc->stmts);
        ret = SQL_SUCCESS;
    }
    __debug("leaves method.");
    return ret;
}

/*
   COMDB2-ODBC internal.

   Frees a connection handle.
 */
static SQLRETURN comdb2_SQLFreeConnect(dbc_t *phdbc)
{
    SQLRETURN ret = SQL_SUCCESS;
    stmt_t *stmt, *next;

    __debug("enters method.");

    if(!phdbc)
        return SQL_INVALID_HANDLE;
    
    if(phdbc->in_txn) {
        __warn("Transaction executing.");
        return DBC_ODBC_ERR_MSG(ERROR_FUNCTION_SEQ_ERR, "A transaction is executing.");
    }

    /* Free all associated statement handles. */
    list_iterate_safe(stmt, next, &phdbc->stmts, list, stmt_t)
        if(SQL_SUCCEEDED(ret = comdb2_SQLFreeStmt(stmt)))
            return ret;

    /* If the connection is still connected, it should be closed. */
    if(phdbc->connected && SQL_FAILED(ret = SQLDisconnect(phdbc)))
        return ret;
        
    /* Free error struct */
    free(phdbc->error);
    phdbc->error = NULL;

    /* Remove myself from the environment. */
    list_rm(&phdbc->list);
    free(phdbc);
        
    __debug("leaves method.");
    
    return ret;
}

/*
   COMDB2-ODBC internal.

   Allocates a statement handle.
 */
static SQLRETURN comdb2_SQLAllocStmt(SQLHDBC hdbc, SQLHSTMT *phstmt)
{
    stmt_t *stmt;
    dbc_t *phdbc = (dbc_t *)hdbc;
    __debug("enters method.");

    if(SQL_NULL_HDBC == hdbc)
        return SQL_INVALID_HANDLE;

    if((*phstmt = my_calloc_typed(stmt_t, 1, SQLHSTMT)) == NULL) {
        *phstmt = SQL_NULL_HSTMT;
        return DBC_ODBC_ERR(ERROR_MEM_ALLOC_FAIL);
    }

    stmt = (stmt_t *) *phstmt;

    zerofill(*phstmt, stmt_t, 1);
    stmt->dbc = phdbc;
    stmt->sqlh = phdbc->sqlh;
    stmt->status = STMT_ALLOCATED;
    stmt->changed = true;

    list_append(&stmt->list, &phdbc->stmts);

    __debug("leaves method.");
    return SQL_SUCCESS;
}

/*
   Recycles a statement handle.

   Returns true/false to indicate success/failure.
 */
bool recycle_stmt(stmt_t *phstmt)
{
    if(!phstmt) 
        return false;

    __debug("enters method.");

    switch(phstmt->status) {
        case STMT_ALLOCATED:
            __info("Allocated, nothing to do.");
            return true;
        case STMT_EXECUTING:
            __warn("Statement executing.");
            return false;

        case STMT_READY:
        case STMT_PREMATURE:
        case STMT_FINISHED:
        case STMT_EXTRACTED:
            break;

        default:    
            __fatal("Unknown status.");
            return false;
    }

    free(phstmt->query);
    phstmt->query = NULL;

    free(phstmt->query_with_params);
    phstmt->query_with_params = NULL;

    free(phstmt->effects);
    phstmt->effects = NULL;

    free(phstmt->error);
    phstmt->error = NULL;

    /* Free bound parameters and clear cdb2 bindings. */
    for(; phstmt->params_allocated > 0; --phstmt->params_allocated) {
        /* Only internal_buffer needs to be freed. Other pointers will be freed by user programs. */
        free(phstmt->params[phstmt->params_allocated - 1].internal_buffer);
        phstmt->params[phstmt->params_allocated - 1].internal_buffer = NULL;
    }
    free(phstmt->params);
    phstmt->params = NULL;
    cdb2_clearbindings(phstmt->sqlh);

    /* Free data buffers */
    free(phstmt->buffers);
    phstmt->buffers = NULL;
    
    phstmt->sql_type = STMT_UNDEFINED;
    phstmt->status = STMT_ALLOCATED;
    phstmt->changed = true;
    phstmt->col_count = 0;

    __debug("leaves method.");

    return true;
}

/*
   COMDB2-ODBC internal.

   After calling this function, the statement handle cannot bu used again.
 */
static SQLRETURN comdb2_SQLFreeStmt(stmt_t *phstmt)
{
    __debug("enters method.");

    if(!phstmt)
        return SQL_INVALID_HANDLE;
    
    if(phstmt->status & STMT_EXECUTING) {
        /* If the statement is still executing (possibly a long query), 
           it cannot be freed until execution is done. */
        __fatal("Statement executing.");
        return STMT_ODBC_ERR_MSG(ERROR_FUNCTION_SEQ_ERR, "Statement is executing.");
    }
 
    free(phstmt->query);
    free(phstmt->query_with_params);
    free(phstmt->effects);
    free(phstmt->error);

    /* Free bound parameters and clear cdb2 bindings. */
    for(; phstmt->params_allocated > 0; --phstmt->params_allocated) {
        /* Only internal_buffer needs to be freed. Other pointers will be freed by user programs. */
        free(phstmt->params[phstmt->params_allocated - 1].internal_buffer);
        phstmt->params[phstmt->params_allocated - 1].internal_buffer = NULL;
    }

    free(phstmt->params);
    cdb2_clearbindings(phstmt->sqlh);

    /* Free data buffers */
    free(phstmt->buffers);
    
    list_rm(&phstmt->list); 
    free(phstmt);

    __debug("leaves method.");
    return SQL_SUCCESS;
}

/*
   COMDB2-ODBC internal.

   Unbinds a statement handle. It means all bound columns will be discarded.
 */
static SQLRETURN comdb2_unbind(stmt_t *phstmt)
{
    __debug("enters method.");

    if(!phstmt)
        return SQL_INVALID_HANDLE;
    
    free(phstmt->buffers);
    phstmt->buffers = NULL;
    phstmt->num_data_buffers = 0;

    __debug("leaves method.");
    return SQL_SUCCESS;
}

/*
   COMDB2-ODBC internal.

   Resets a statement handle. It means all bound parameters will be discarded.
 */
static SQLRETURN comdb2_reset_params(stmt_t *phstmt)
{
    __debug("enters method.");

    if(!phstmt)
        return SQL_INVALID_HANDLE;
    
    /* Free bound parameters and clear cdb2 bindings. */
    for(; phstmt->params_allocated > 0; --phstmt->params_allocated) {
        /* Only internal_buffer needs to be freed. Other pointers will be freed by user programs. */
        free(phstmt->params[phstmt->params_allocated - 1].internal_buffer);
        phstmt->params[phstmt->params_allocated - 1].internal_buffer = NULL;
    }

    free(phstmt->params);
    phstmt->params = NULL;
    phstmt->changed = true;
    cdb2_clearbindings(phstmt->sqlh);

    __debug("leaves method.");
    return SQL_SUCCESS;
}

/*
   COMDB2-ODBC internal.

   Closes a statement handle.
 */
static SQLRETURN comdb2_SQLCloseCursor(stmt_t* phstmt)
{
    int rc = CDB2_OK_DONE;

    __debug("enters method.");

    if(!phstmt)
        return SQL_INVALID_HANDLE;
    
    if(phstmt->status & STMT_FINISHED && SQLH_STATUS(phstmt) == SQLH_FINISHED) {
        __info("Clear unextracted rows.");
        while((rc = cdb2_next_record(phstmt->sqlh)) == CDB2_OK);
        SET_SQLH_IDLE(phstmt);
    }

    phstmt->status = STMT_EXTRACTED;

    /* Clear effects and error. */
    free(phstmt->effects);
    phstmt->effects = NULL;
    free(phstmt->error);
    phstmt->error = NULL;
    phstmt->col_count = 0;

    if(rc == CDB2_OK_DONE) {
        __debug("leaves method.");
        return SQL_SUCCESS;
    }
    
    return set_stmt_error(phstmt, ERROR_NVM, "Internal error.", rc);
}

/******************
 * ODBC API       *
 *****************/

SQLRETURN SQL_API SQLCloseCursor(SQLHSTMT hstmt)
{
    return comdb2_SQLCloseCursor((stmt_t *)hstmt);
}

SQLRETURN SQL_API SQLAllocStmt(SQLHDBC hdbc, SQLHSTMT *phstmt)
{
    return comdb2_SQLAllocStmt(hdbc, phstmt);
}

SQLRETURN SQL_API SQLFreeStmt(SQLHSTMT hstmt, SQLUSMALLINT option)
{
    SQLRETURN ret = SQL_ERROR;
    stmt_t *phstmt = (stmt_t *)hstmt;

    if(!hstmt)
        return SQL_INVALID_HANDLE;

    switch(option) {
        case SQL_CLOSE:
            ret = comdb2_SQLCloseCursor(phstmt);
            break;
        case SQL_DROP:
            ret = comdb2_SQLFreeStmt(phstmt);
            break;
        case SQL_UNBIND:
            ret = comdb2_unbind(phstmt);
            break;
        case SQL_RESET_PARAMS:
            ret = comdb2_reset_params(phstmt);
            break;
        default:
            ret = STMT_ODBC_ERR(ERROR_OPTION_OUT_OF_RANGE);
            break;
    }

    return ret;
}

SQLRETURN SQL_API SQLAllocEnv(SQLHENV *phenv)
{
    return comdb2_SQLAllocEnv(phenv);
}

SQLRETURN SQL_API SQLFreeEnv(SQLHENV henv)
{
    return comdb2_SQLFreeEnv(henv);
}

SQLRETURN SQL_API SQLAllocConnect(SQLHENV henv, SQLHDBC *phdbc)
{
    return comdb2_SQLAllocConnect(henv, phdbc);
}

SQLRETURN SQL_API SQLFreeConnect(SQLHDBC hdbc)
{
    return comdb2_SQLFreeConnect(hdbc);
}

SQLRETURN SQL_API SQLAllocHandle(
        SQLSMALLINT HandleType, 
        SQLHANDLE   InputHandle, 
        SQLHANDLE   *OutputHandlePtr)
{
    /* handle allocation is forwarded to an ODBC 2.x method according to the handle type. 
        In this way we can guarantee backward compatibility. */

    SQLRETURN ret = SQL_ERROR;
    
    switch(HandleType) {
        case SQL_HANDLE_ENV:
            ret = comdb2_SQLAllocEnv(OutputHandlePtr);
            break;

        case SQL_HANDLE_DBC:
            ret = comdb2_SQLAllocConnect(InputHandle, OutputHandlePtr);
            break;

        case SQL_HANDLE_STMT:
            ret = comdb2_SQLAllocStmt(InputHandle, OutputHandlePtr);
            break;

        case SQL_HANDLE_DESC:
        default:
            break;
    }

    return ret;
}

SQLRETURN SQL_API SQLFreeHandle(SQLSMALLINT HandleType,
                                SQLHANDLE Handle)
{
    SQLRETURN ret = SQL_ERROR;

    switch(HandleType) {
        case SQL_HANDLE_ENV:
            ret = comdb2_SQLFreeEnv((env_t *)Handle);
            break;

        case SQL_HANDLE_DBC:
            ret = comdb2_SQLFreeConnect((dbc_t *)Handle);
            break;

        case SQL_HANDLE_STMT:
            ret = comdb2_SQLFreeStmt((stmt_t *)Handle);
            break;

        case SQL_HANDLE_DESC:
        default:
            break;
    }

    return ret;
}
