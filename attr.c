/**
 * @file attr.c
 * @description
 * Set/get environment/connection/statement attributes.
 *      07-Jul-2014
 *          Only a minimal subset is implemented at this moment. The driver shall work well for general applications.
 *      
 * @author Rivers Zhang <hzhang320@bloomberg.net>
 * @history
 * 07-Jul-2014 Created.
 */

#include "driver.h"
#include <stdint.h>

/**
 * FIXME Currently we can only use this function to set transation-related settings.
 */
static SQLRETURN comdb2_SQLSetConnectAttr(
        SQLHDBC       hdbc,
        SQLINTEGER    attr,
        SQLPOINTER    buf,
        SQLINTEGER    str_len)
{
    dbc_t *phdbc = (dbc_t *)hdbc;

    __debug("enters method. attr = %d", attr);

	(void)str_len;

    if(!hdbc)
        return SQL_INVALID_HANDLE;

    if(phdbc->in_txn)
        return DBC_ODBC_ERR_MSG(ERROR_FUNCTION_SEQ_ERR, "A transation is executing.");

    switch(attr) {
        case SQL_ATTR_AUTOCOMMIT:
            /* @buf is a ptr to an SQLUINTEGER value. */
            phdbc->auto_commit = (bool)(intptr_t)buf;
            break;

        case SQL_ATTR_CURRENT_CATALOG:
            /* Unusable for comdb2. Put it here to make JdbcOdbcBridge work. */
            break;

        case SQL_ATTR_TXN_ISOLATION:
            /* @buf is a ptr to a 32bit mask. */
            if((intptr_t)buf & (intptr_t)(SQL_TXN_READ_UNCOMMITTED | SQL_TXN_REPEATABLE_READ))
                return DBC_ODBC_ERR_MSG(ERROR_WTH, "Unsupported transaction isolation mode.");
            phdbc->txn_isolation = (int)(intptr_t)buf;
            phdbc->txn_changed = true;
            break;

        default:
            return DBC_ODBC_ERR(ERROR_UNIMPL_ATTR);
    }

    __debug("leaves method.");
    return SQL_SUCCESS;
}

SQLRETURN SQL_API SQLSetConnectAttr(
        SQLHDBC       hdbc,
        SQLINTEGER    attr,
        SQLPOINTER    buf,
        SQLINTEGER    str_len)
{
    return comdb2_SQLSetConnectAttr(hdbc, attr, buf, str_len);
}

SQLRETURN SQL_API SQLSetConnectOption(
        SQLHDBC         hdbc,
        SQLUSMALLINT    option,
        SQLULEN         param)
{
    return comdb2_SQLSetConnectAttr(hdbc, option, (SQLPOINTER)(intptr_t)param, 0);
}

static SQLRETURN comdb2_SQLGetConnectAttr(
        SQLHDBC        hdbc,
        SQLINTEGER     attr,
        SQLPOINTER     buf,
        SQLINTEGER     buflen,
        SQLINTEGER     *str_len)
{
    dbc_t *phdbc = (dbc_t *)hdbc;
    int minimum_length_required = -1;
    bool is_str_attr = false;

    __debug("enters method. attr = %d", attr);

    if(!hdbc)
        return SQL_INVALID_HANDLE;

    switch(attr) {
        case SQL_ATTR_AUTOCOMMIT:
            SET_SQLUINT(buf, phdbc->auto_commit, minimum_length_required);
            break;

        case SQL_ATTR_TXN_ISOLATION:
            SET_SQLUINT(buf, (SQL_TXN_READ_UNCOMMITTED | SQL_TXN_REPEATABLE_READ), minimum_length_required);
            break;

        default:
            return DBC_ODBC_ERR(ERROR_UNIMPL_ATTR);
    }

    if(str_len)
        *str_len = minimum_length_required;

    __debug("leaves method.");
    return (is_str_attr && minimum_length_required >= buflen) ? 
        DBC_ODBC_ERR(ERROR_STR_TRUNCATED) : SQL_SUCCESS;
}

SQLRETURN SQL_API SQLGetConnectAttr(
        SQLHDBC        hdbc,
        SQLINTEGER     attr,
        SQLPOINTER     buf,
        SQLINTEGER     buflen,
        SQLINTEGER     *str_len)
{
    return comdb2_SQLGetConnectAttr(hdbc, attr, buf, buflen, str_len);
}

SQLRETURN SQL_API SQLGetConnectOption(
        SQLHDBC         hdbc,
        SQLUSMALLINT    option,
        SQLPOINTER      value_ptr)
{
    return comdb2_SQLGetConnectAttr(hdbc, option, value_ptr, 0, NULL);
}

static SQLRETURN comdb2_SQLSetStmtAttr(
        SQLHSTMT      hstmt,
        SQLINTEGER    attr,
        SQLPOINTER    buf,
        SQLINTEGER    str_len)
{
    stmt_t *phstmt = (stmt_t *)hstmt;

    __debug("enters method. attr = %d", attr);
	(void)str_len;

    if(!hstmt)
        return SQL_INVALID_HANDLE;

    switch(attr) {
        case SQL_ATTR_CURSOR_TYPE: /* Currently cusor can only scroll forward. */
            if((SQLULEN)(intptr_t)buf != SQL_CURSOR_FORWARD_ONLY)
                return STMT_ODBC_ERR(ERROR_UNSUPPORTED_OPTION_VALUE);
            break;

        case SQL_ATTR_CONCURRENCY:
            if((SQLULEN)(intptr_t)buf != SQL_CONCUR_READ_ONLY)
                return STMT_ODBC_ERR(ERROR_UNSUPPORTED_OPTION_VALUE);
            break;

        default:
            return STMT_ODBC_ERR(ERROR_UNIMPL_ATTR);
    }

    __debug("leaves method.");
    return SQL_SUCCESS;
}

SQLRETURN SQL_API SQLSetStmtAttr(
        SQLHSTMT      hstmt,
        SQLINTEGER    attr,
        SQLPOINTER    buf,
        SQLINTEGER    str_len)
{
    return comdb2_SQLSetStmtAttr(hstmt, attr, buf, str_len);
}

SQLRETURN SQL_API SQLSetStmtOption(
        SQLHSTMT        hstmt,
        SQLUSMALLINT    option,
        SQLULEN         param)
{
    return comdb2_SQLSetStmtAttr(hstmt, option, (SQLPOINTER)(intptr_t)param, 0);
}

/**
 * FIXME Currently SQLGetStmtAttr is only for reporting SQL_UB_OFF to driver manager.
 */
static SQLRETURN comdb2_SQLGetStmtAttr(
        SQLHSTMT        stmt,
        SQLINTEGER      attr,
        SQLPOINTER      buf,
        SQLINTEGER      buflen,
        SQLINTEGER      *str_len)
{
    stmt_t *phstmt = (stmt_t *)stmt;

    __debug("enters method. attr = %d", attr);
	(void)buflen;
	(void)str_len;
	size_t len = 0;

    if(!stmt)
        return SQL_INVALID_HANDLE;

    switch(attr) {
        case SQL_ATTR_USE_BOOKMARKS:
            *((SQLULEN *)buf) = SQL_UB_OFF;
			len = sizeof(SQLULEN);
            break;
        
        case SQL_ATTR_CONCURRENCY:
            *((SQLULEN *)buf) = SQL_CONCUR_ROWVER;
			len = sizeof(SQLULEN);
            break;

        case SQL_ATTR_APP_ROW_DESC:
        case SQL_ATTR_APP_PARAM_DESC:
        case SQL_ATTR_IMP_ROW_DESC:
        case SQL_ATTR_IMP_PARAM_DESC:
			*(SQLHDESC *)buf = (SQLHDESC)(intptr_t)MAGIK;
			len = sizeof(SQLHDESC);
            break;

        default:
            return STMT_ODBC_ERR(ERROR_UNIMPL_ATTR);
    }

	if (str_len != NULL)
		*str_len = (SQLINTEGER)len;

    __debug("leaves method.");
    return SQL_SUCCESS;
}

SQLRETURN SQL_API SQLGetStmtAttr(
        SQLHSTMT        stmt,
        SQLINTEGER      attr,
        SQLPOINTER      buf,
        SQLINTEGER      buflen,
        SQLINTEGER      *str_len)
{
    return comdb2_SQLGetStmtAttr(stmt, attr, buf, buflen, str_len);
}

SQLRETURN SQL_API SQLGetStmtOption(
        SQLHSTMT        stmt,
        SQLUSMALLINT    option,
        SQLPOINTER      value_ptr)
{
    return comdb2_SQLGetStmtAttr(stmt, option, value_ptr, 0, NULL);
}

SQLRETURN SQL_API SQLGetDescField(SQLHDESC handle, SQLSMALLINT recno,
		SQLSMALLINT fieldid, SQLPOINTER value,
		SQLINTEGER buflen, SQLINTEGER *strlen)
{
	NOT_IMPL;
}
