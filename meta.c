#include "driver.h"

#include <string.h>
#include <stdio.h>

SQLRETURN SQL_API SQLGetTypeInfo(SQLHSTMT hstmt, SQLSMALLINT type)
{
    stmt_t *phstmt = (stmt_t *)hstmt;
    char metaquery[MAX_INTERNAL_QUERY_LEN + 1];
    size_t pos = 0;
    int variant = 0;
    const char *tn = NULL;

    __debug("enters method.");

    if(!hstmt)
        return SQL_INVALID_HANDLE;

    if (type == SQL_CHAR ||
        type == SQL_VARCHAR ||
        type == SQL_LONGVARCHAR ||
        type == SQL_WCHAR ||
        type == SQL_WVARCHAR ||
        type == SQL_WLONGVARCHAR ||
        type == SQL_BINARY ||
        type == SQL_VARBINARY ||
        type == SQL_LONGVARBINARY)
        variant = 1;

    metaquery[MAX_INTERNAL_QUERY_LEN] = 0;
    pos += snprintf(metaquery, MAX_INTERNAL_QUERY_LEN,
            "SELECT tn AS TYPE_NAME,"
            "dt AS DATA_TYPE,"
            "0 AS COLUMN_SIZE,"
            "null AS LITERAL_PREFIX,"
            "null AS LITERAL_SUFFIX,"
            "'%s' as CREATE_PARAMS,"
            "%d as NULLABLE,"
            "%d as CASE_SENSITIVE,"
            "%d as SEARCHABLE,"
            "%d as UNSIGNED_ATTRIBUTE,"
            "%d as FIXED_PREC_SCALE," // <-- true for decimals
            "%d as AUTO_UNIQUE_VALUE,"
            "null as LOCAL_TYPE_NAME,"
            "null as MINIMUM_SCALE,"
            "dt as SQL_DATA_TYPE,"
            "null as SQL_DATETIME_SUB,"
            "10 as NUM_PREC_RADIX,"
            "null as INTERVAL_PRECISION ",
            variant ? "length" : "null",
            SQL_NULLABLE,
            SQL_TRUE,
            SQL_SEARCHABLE,
            SQL_FALSE,
            (type == SQL_DECIMAL || type == SQL_NUMERIC) ? SQL_TRUE : SQL_FALSE,
            SQL_FALSE);

    if (type == SQL_ALL_TYPES) {
        pos += snprintf(&metaquery[pos], MAX_INTERNAL_QUERY_LEN - pos,
                "FROM (SELECT '%s' AS TN, %d AS DT union ",
                "cstring", SQL_CHAR);
        pos += snprintf(&metaquery[pos], MAX_INTERNAL_QUERY_LEN - pos,
                "SELECT '%s' AS TN, %d AS DT union ",
                "vutf8", SQL_VARCHAR);
        pos += snprintf(&metaquery[pos], MAX_INTERNAL_QUERY_LEN - pos,
                "SELECT '%s' AS TN, %d AS DT union ",
                "vutf8", SQL_LONGVARCHAR);
        pos += snprintf(&metaquery[pos], MAX_INTERNAL_QUERY_LEN - pos,
                "SELECT '%s' AS TN, %d AS DT union ",
                "vutf8", SQL_WCHAR);
        pos += snprintf(&metaquery[pos], MAX_INTERNAL_QUERY_LEN - pos,
                "SELECT '%s' AS TN, %d AS DT union ",
                "vutf8", SQL_WVARCHAR);
        pos += snprintf(&metaquery[pos], MAX_INTERNAL_QUERY_LEN - pos,
                "SELECT '%s' AS TN, %d AS DT union ",
                "vutf8", SQL_WLONGVARCHAR);
        pos += snprintf(&metaquery[pos], MAX_INTERNAL_QUERY_LEN - pos,
                "SELECT '%s' AS TN, %d AS DT union ",
                "decimal128", SQL_DECIMAL);
        pos += snprintf(&metaquery[pos], MAX_INTERNAL_QUERY_LEN - pos,
                "SELECT '%s' AS TN, %d AS DT union ",
                "decimal128", SQL_NUMERIC);
        pos += snprintf(&metaquery[pos], MAX_INTERNAL_QUERY_LEN - pos,
                "SELECT '%s' AS TN, %d AS DT union ",
                "short", SQL_SMALLINT);
        pos += snprintf(&metaquery[pos], MAX_INTERNAL_QUERY_LEN - pos,
                "SELECT '%s' AS TN, %d AS DT union ",
                "short", SQL_BIT);
        pos += snprintf(&metaquery[pos], MAX_INTERNAL_QUERY_LEN - pos,
                "SELECT '%s' AS TN, %d AS DT union ",
                "short", SQL_TINYINT);
        pos += snprintf(&metaquery[pos], MAX_INTERNAL_QUERY_LEN - pos,
                "SELECT '%s' AS TN, %d AS DT union ",
                "int", SQL_INTEGER);
        pos += snprintf(&metaquery[pos], MAX_INTERNAL_QUERY_LEN - pos,
                "SELECT '%s' AS TN, %d AS DT union ",
                "longlong", SQL_BIGINT);
        pos += snprintf(&metaquery[pos], MAX_INTERNAL_QUERY_LEN - pos,
                "SELECT '%s' AS TN, %d AS DT union ",
                "float", SQL_FLOAT);
        pos += snprintf(&metaquery[pos], MAX_INTERNAL_QUERY_LEN - pos,
                "SELECT '%s' AS TN, %d AS DT union ",
                "double", SQL_REAL);
        pos += snprintf(&metaquery[pos], MAX_INTERNAL_QUERY_LEN - pos,
                "SELECT '%s' AS TN, %d AS DT union ",
                "double", SQL_DOUBLE);
        pos += snprintf(&metaquery[pos], MAX_INTERNAL_QUERY_LEN - pos,
                "SELECT '%s' AS TN, %d AS DT union ",
                "byte", SQL_BINARY);
        pos += snprintf(&metaquery[pos], MAX_INTERNAL_QUERY_LEN - pos,
                "SELECT '%s' AS TN, %d AS DT union ",
                "blob", SQL_VARBINARY);
        pos += snprintf(&metaquery[pos], MAX_INTERNAL_QUERY_LEN - pos,
                "SELECT '%s' AS TN, %d AS DT union ",
                "blob", SQL_LONGVARBINARY);
        pos += snprintf(&metaquery[pos], MAX_INTERNAL_QUERY_LEN - pos,
                "SELECT '%s' AS TN, %d AS DT union ",
                "datetime", SQL_TIMESTAMP);
        pos += snprintf(&metaquery[pos], MAX_INTERNAL_QUERY_LEN - pos,
                "SELECT '%s' AS TN, %d AS DT union ",
                "intervalym", SQL_INTERVAL_YEAR_TO_MONTH);
        pos += snprintf(&metaquery[pos], MAX_INTERNAL_QUERY_LEN - pos,
                "SELECT '%s' AS TN, %d AS DT) ORDER BY TYPE_NAME",
                "intervalds", SQL_INTERVAL_DAY_TO_SECOND);
    } else {
        switch (type) {
            case SQL_CHAR:
                tn = "cstring";
                break;
            case SQL_VARCHAR:
            case SQL_LONGVARCHAR:
            case SQL_WCHAR:
            case SQL_WVARCHAR:
            case SQL_WLONGVARCHAR:
                tn = "vutf8";
                break;
            case SQL_DECIMAL:
            case SQL_NUMERIC:
                /* use the highest */
                tn = "decimal128";
                break;
            case SQL_SMALLINT:
            case SQL_BIT:
            case SQL_TINYINT:
                tn = "short";
                break;
            case SQL_INTEGER:
                tn = "int";
                break;
            case SQL_BIGINT:
                tn = "longlong";
                break;
            case SQL_FLOAT:
                tn = "float";
                break;
            case SQL_DOUBLE:
            case SQL_REAL:
                tn = "double";
                break;
            case SQL_BINARY:
                tn = "byte";
                break;
            case SQL_VARBINARY:
            case SQL_LONGVARBINARY:
                tn = "blob";
                break;
            case SQL_TIMESTAMP:
                tn = "datetime";
                break;
            case SQL_INTERVAL_YEAR_TO_MONTH:
                tn = "intervalym";
                break;
            case SQL_INTERVAL_DAY_TO_SECOND:
                tn = "intervalds";
                break;
            default:
                tn = NULL;
                break;
        }
        if (tn != NULL) {
            pos += snprintf(&metaquery[pos], MAX_INTERNAL_QUERY_LEN - pos,
                    " FROM (SELECT '%s' AS TN, %d AS DT)",
                    tn, type);
        } else {
            strncpy(&metaquery[pos],
                    " LIMIT 0", MAX_INTERNAL_QUERY_LEN - pos);
        }
    }

    __debug("metaquery is %s", metaquery);
    __debug("leaves method.");
    return comdb2_SQLExecDirect(phstmt, (SQLCHAR *)metaquery, SQL_NTS);
}

/**
 * ODBC API.
 * SQLGetInfo returns general information about the driver and data source associated with a connection.
 * TODO Complete all necessary attributes.
 */

SQLRETURN SQL_API SQLGetInfo(
        SQLHDBC         hdbc,
        SQLUSMALLINT    type,
        SQLPOINTER      value_ptr,
        SQLSMALLINT     buflen,
        SQLSMALLINT     *str_len)
{
    dbc_t *phdbc = (dbc_t *)hdbc;
    int minimum_length_required = -1;
    SQLRETURN ret = SQL_SUCCESS;
    bool handled;
    int t_ret;
    char *dbver = "UNKNOWN";

    __debug("enters method. %d", type);

    if(!hdbc)
        return SQL_INVALID_HANDLE;

    /* First deal with string attributes. */
    switch(type) {
        case SQL_DATABASE_NAME:
            SET_CSTRING(value_ptr, phdbc->ci.database, buflen, minimum_length_required);
            break;

        case SQL_DBMS_NAME:
            SET_CSTRING(value_ptr, DBNAME, buflen, minimum_length_required);
            break;
        
        case SQL_DBMS_VER:
            if (!phdbc->connected && (ret = comdb2_SQLConnect(phdbc)) !=
                    SQL_SUCCESS)
                return ret;
            t_ret = cdb2_run_statement(phdbc->sqlh, "SELECT COMDB2_VERSION()");
            if (t_ret != 0)
                return set_dbc_error(phdbc, ERROR_WTH, cdb2_errstr(phdbc->sqlh), t_ret);
            while ((t_ret = cdb2_next_record(phdbc->sqlh)) == CDB2_OK)
                dbver = cdb2_column_value(phdbc->sqlh, 0);
            if (t_ret != CDB2_OK_DONE)
                return set_dbc_error(phdbc, ERROR_WTH, cdb2_errstr(phdbc->sqlh), t_ret);
            SET_CSTRING(value_ptr, dbver, buflen, minimum_length_required);
            break;
        
        case SQL_DRIVER_NAME:
            SET_CSTRING(value_ptr, DRVNAME, buflen, minimum_length_required);
            break;

        case SQL_DRIVER_VER:
            SET_CSTRING(value_ptr, DRVVER, buflen, minimum_length_required);
            break;

        case SQL_DRIVER_ODBC_VER:
            SET_CSTRING(value_ptr, DRVODBCVER, buflen, minimum_length_required);
            break;
    }

    /* If @minimum_length_required has been altered, set @handled to true. */
    handled = (minimum_length_required != -1) ? true : false;

    if(minimum_length_required >= buflen)
        /* For a string attribute, if the required length exceeds @buflen, give a warning.
           For other types, since @minimum_length_required was initialized to -1, 
           this branch will not be executed. */
        ret = DBC_ODBC_ERR(ERROR_STR_TRUNCATED);

    /* Next deal with fixed length attributes (meaning we assume the buffer is large enough). */
    switch(type) {
        case SQL_BATCH_ROW_COUNT:
            SET_SQLUINT(value_ptr, SQL_BRC_EXPLICIT, minimum_length_required);
            break;

        case SQL_BATCH_SUPPORT:
            SET_SQLUINT(value_ptr, (SQL_BS_SELECT_EXPLICIT | SQL_BS_ROW_COUNT_EXPLICIT), minimum_length_required);
            break;

        case SQL_CATALOG_USAGE:
            SET_SQLUINT(value_ptr, (SQL_CU_DML_STATEMENTS | SQL_CU_PROCEDURE_INVOCATION), minimum_length_required);
            break;

        case SQL_PARAM_ARRAY_ROW_COUNTS:
            SET_SQLUINT(value_ptr, SQL_PARC_NO_BATCH, minimum_length_required);
            break;

        case SQL_SCHEMA_USAGE:
            SET_SQLUINT(value_ptr, (SQL_SU_DML_STATEMENTS | SQL_SU_PROCEDURE_INVOCATION), minimum_length_required);
            break;

        case SQL_SCROLL_OPTIONS:
            SET_SQLUINT(value_ptr, SQL_SO_FORWARD_ONLY, minimum_length_required);
            break;

        case SQL_TIMEDATE_FUNCTIONS:
            SET_SQLUINT(value_ptr, SQL_FN_TD_NOW, minimum_length_required);
            break;
        
        case SQL_TXN_CAPABLE:
            SET_SQLUSMALLINT(value_ptr, SQL_TC_DML, minimum_length_required);
            break;
        
        case SQL_TXN_ISOLATION_OPTION:
            SET_SQLUINT(value_ptr, (SQL_TXN_READ_COMMITTED | SQL_TXN_SERIALIZABLE), minimum_length_required);
            break;

        default:
            if(!handled)
                ret = DBC_ODBC_ERR(ERROR_TYPE_OUT_OF_RANGE);
            break;
    }

    if(str_len)
        *str_len = (SQLSMALLINT)minimum_length_required;

    __debug("leaves method.");
    return ret;
}

SQLRETURN SQL_API SQLColumns(
        SQLHSTMT       hstmt,
        SQLCHAR *      catalog,
        SQLSMALLINT    catalog_len,
        SQLCHAR *      schema,
        SQLSMALLINT    schema_len,
        SQLCHAR *      tbl,
        SQLSMALLINT    tbl_len,
        SQLCHAR *      column,
        SQLSMALLINT    column_len)
{
    SQLRETURN ret;
    stmt_t *phstmt = (stmt_t *)hstmt;
    char metaquery[MAX_INTERNAL_QUERY_LEN + 1];
    size_t pos = 0;

    __debug("enters method.");

    /* Ignore catalog and schema */
    (void)catalog;
    (void)catalog_len;
    (void)schema;
    (void)schema_len;

    metaquery[MAX_INTERNAL_QUERY_LEN] = 0;
    pos = snprintf(metaquery, MAX_INTERNAL_QUERY_LEN,
                   "SELECT '%s' AS TABLE_CAT, NULL AS TABLE_SCHEM,"
                   "tablename AS TABLE_NAME, columnname AS COLUMN_NAME,"
                   "0 AS DATA_TYPE," /* <-- We will convert it later */
                   "type AS TYPE_NAME, (size - 1) AS COLUMN_SIZE, "
                   "(size - 1) AS BUFFER_LENGTH, NULL AS DECIMAL_DIGITS,"
                   "10 AS NUM_PREC_RADIX, "
                   "(UPPER(isnullable) == 'Y') AS NULLABLE, null AS REMARKS,"
                   "trim(defaultvalue) AS COLUMN_DEF, 0 AS SQL_DATA_TYPE,"
                   "0 AS SQL_DATETIME_SUB, (size - 1) AS CHAR_OCTET_LENGTH,"
                   "0 AS ORDINAL_POSITION," /* <-- We will convert it later */
                   "CASE WHEN (UPPER(isnullable) == 'Y') THEN 'YES' ELSE 'NO' END AS IS_NULLABLE,"
                   "sqltype " /* <-- Convert this to DATA_TYPE */
                   "FROM comdb2sys_columns WHERE 1=1",
                   phstmt->dbc->ci.database);

    if (tbl != NULL) {
        if (tbl_len == SQL_NTS)
            tbl_len = (SQLSMALLINT)strlen((const char *)tbl);
        pos += snprintf(&(metaquery[pos]), MAX_INTERNAL_QUERY_LEN - pos,
                        " AND TABLE_NAME LIKE '%*s'", tbl_len, tbl);
    }

    if (column != NULL) {
        if (column_len == SQL_NTS)
            column_len = (SQLSMALLINT)strlen((const char *)column);
        pos += snprintf(&metaquery[pos], MAX_INTERNAL_QUERY_LEN - pos,
                        " AND COLUMN_NAME LIKE '%*s'", column_len, column);
    }
    ret = comdb2_SQLExecDirect(phstmt, (SQLCHAR *)metaquery, SQL_NTS);
    if (ret == SQL_SUCCESS) {
        phstmt->status |= STMT_SQLCOLUMNS;
        phstmt->ord_pos = 0;
    }

    __debug("leaves method.");
    return ret;
}

SQLRETURN SQL_API SQLProcedures(
    SQLHSTMT       hstmt,
    SQLCHAR *      catalog,
    SQLSMALLINT    catalog_len,
    SQLCHAR *      schema,
    SQLSMALLINT    schema_len,
    SQLCHAR *      proc,
    SQLSMALLINT    proc_len)
{
    SQLRETURN ret;
    stmt_t *phstmt = (stmt_t *)hstmt;
    char metaquery[MAX_INTERNAL_QUERY_LEN + 1];
    size_t pos = 0;

    __debug("enters method.");

    /* Ignore catalog and schema */
    (void)catalog;
    (void)catalog_len;
    (void)schema;
    (void)schema_len;

    metaquery[MAX_INTERNAL_QUERY_LEN] = 0;
    pos = snprintf(metaquery, MAX_INTERNAL_QUERY_LEN,
                   "SELECT '%s' AS PROCEDURE_CAT,"
                   "NULL AS PROCEDURE_SCHEM,"
                   "name AS PROCEDURE_NAME,"
                   "null AS NUM_INPUT_PARAMS,"
                   "null AS NUM_OUTPUT_PARAMS,"
                   "null AS NUM_RESULT_SETS,"
                   "'' AS REMARKS,"
                   "%d AS PROCEDURE_TYPE "
                   "FROM comdb2sys_procedures WHERE 1=1 ",
                   phstmt->dbc->ci.database,
                   SQL_PT_UNKNOWN);

    if (proc != NULL) {
        if (proc_len == SQL_NTS)
            proc_len = (SQLSMALLINT)strlen((const char *)proc);
        pos += snprintf(&metaquery[pos], MAX_INTERNAL_QUERY_LEN - pos,
                        " AND name LIKE '%*s'", proc_len, proc);
    }

    ret = comdb2_SQLExecDirect(phstmt, (SQLCHAR *)metaquery, SQL_NTS);
    __debug("leaves method.");
    return ret;
}

SQLRETURN SQL_API SQLProcedureColumns(
    SQLHSTMT       hstmt,
    SQLCHAR *      catalog,
    SQLSMALLINT    catalog_len,
    SQLCHAR *      schema,
    SQLSMALLINT    schema_len,
    SQLCHAR *      proc,
    SQLSMALLINT    proc_len,
    SQLCHAR *      column,
    SQLSMALLINT    column_len)
{
    SQLRETURN ret;
    stmt_t *phstmt = (stmt_t *)hstmt;
    char metaquery[MAX_INTERNAL_QUERY_LEN + 1];

    __debug("enters method.");

    /* Ignore catalog and schema */
    (void)catalog;
    (void)catalog_len;
    (void)schema;
    (void)schema_len;
    (void)proc;
    (void)proc_len;
    (void)column;
    (void)column_len;

    /* Returns an empty resultset. */
    metaquery[MAX_INTERNAL_QUERY_LEN] = 0;
    snprintf(metaquery, MAX_INTERNAL_QUERY_LEN,
             "SELECT '' AS PROCEDURE_CAT,"
             "'' AS PROCEDURE_SCHEM,"
             "'' AS PROCEDURE_NAME,"
             "'' AS COLUMN_NAME,"
             "0 AS COLUMN_TYPE,"
             "0 AS DATA_TYPE,"
             "'' AS TYPE_NAME,"
             "0 AS COLUMN_SIZE,"
             "0 AS BUFFER_LENGTH,"
             "0 AS DECIMAL_DIGITS,"
             "0 AS NUM_PREC_RADIX,"
             "0 AS NULLABLE,"
             "'' AS REMARKS,"
             "'' AS COLUMN_DEF,"
             "0 AS SQL_DATA_TYPE,"
             "0 AS SQL_DATIME_SUB,"
             "0 AS CHAR_OCTET_LENGTH,"
             "0 AS ORDINAL_POSITION,"
             "'' AS IS_NULLABLE "
             "LIMIT 0");
    ret = comdb2_SQLExecDirect(phstmt, (SQLCHAR *)metaquery, SQL_NTS);
    __debug("leaves method.");
    return ret;
}

SQLRETURN SQL_API SQLColumnPrivileges(
    SQLHSTMT       hstmt,
    SQLCHAR *      catalog,
    SQLSMALLINT    catalog_len,
    SQLCHAR *      schema,
    SQLSMALLINT    schema_len,
    SQLCHAR        *tbl,
    SQLSMALLINT    tbl_len,
    SQLCHAR *      column,
    SQLSMALLINT    column_len)
{
    SQLRETURN ret;
    stmt_t *phstmt = (stmt_t *)hstmt;
    char metaquery[MAX_INTERNAL_QUERY_LEN + 1];

    __debug("enters method.");

    /* Ignore catalog and schema */
    (void)catalog;
    (void)catalog_len;
    (void)schema;
    (void)schema_len;
    (void)tbl;
    (void)tbl_len;
    (void)column;
    (void)column_len;

    /* Returns an empty resultset. */
    metaquery[MAX_INTERNAL_QUERY_LEN] = 0;
    snprintf(metaquery, MAX_INTERNAL_QUERY_LEN,
             "SELECT null AS TABLE_CAT,"
             "null AS TABLE_SCHEM,"
             "'' AS TABLE_NAME,"
             "'' AS COLUMN_NAME,"
             "NULL AS GRANTOR,"
             "'' AS GRANTEE,"
             "'' AS PRIVILEGE,"
             "null AS IS_GRANTABLE "
             "LIMIT 0");
    ret = comdb2_SQLExecDirect(phstmt, (SQLCHAR *)metaquery, SQL_NTS);
    __debug("leaves method.");
    return ret;
}

SQLRETURN SQL_API SQLTablePrivileges(
    SQLHSTMT       hstmt,
    SQLCHAR *      catalog,
    SQLSMALLINT    catalog_len,
    SQLCHAR *      schema,
    SQLSMALLINT    schema_len,
    SQLCHAR        *tbl,
    SQLSMALLINT    tbl_len)
{
    SQLRETURN ret;
    stmt_t *phstmt = (stmt_t *)hstmt;
    char metaquery[MAX_INTERNAL_QUERY_LEN + 1];

    __debug("enters method.");

    /* Ignore catalog and schema */
    (void)catalog;
    (void)catalog_len;
    (void)schema;
    (void)schema_len;
    (void)tbl;
    (void)tbl_len;

    /* Returns an empty resultset. */
    metaquery[MAX_INTERNAL_QUERY_LEN] = 0;
    snprintf(metaquery, MAX_INTERNAL_QUERY_LEN,
             "SELECT null AS TABLE_CAT,"
             "null AS TABLE_SCHEM,"
             "'' AS TABLE_NAME,"
             "NULL AS GRANTOR,"
             "'' AS GRANTEE,"
             "'' AS PRIVILEGE,"
             "null AS IS_GRANTABLE "
             "LIMIT 0");
    ret = comdb2_SQLExecDirect(phstmt, (SQLCHAR *)metaquery, SQL_NTS);
    __debug("leaves method.");
    return ret;
}

SQLRETURN SQL_API SQLSpecialColumns(
    SQLHSTMT       hstmt,
    SQLUSMALLINT   type,
    SQLCHAR *      catalog,
    SQLSMALLINT    catalog_len,
    SQLCHAR *      schema,
    SQLSMALLINT    schema_len,
    SQLCHAR        *tbl,
    SQLSMALLINT    tbl_len,
    SQLUSMALLINT   scope,
    SQLUSMALLINT   nullable)
{
    SQLRETURN ret;
    stmt_t *phstmt = (stmt_t *)hstmt;
    char metaquery[MAX_INTERNAL_QUERY_LEN + 1];

    __debug("enters method.");

    /* Ignore catalog and schema */
    (void)type;
    (void)catalog;
    (void)catalog_len;
    (void)schema;
    (void)schema_len;
    (void)tbl;
    (void)tbl_len;
    (void)scope;
    (void)nullable;

    metaquery[MAX_INTERNAL_QUERY_LEN] = 0;
    snprintf(metaquery, MAX_INTERNAL_QUERY_LEN,
             "SELECT %d AS SCOPE,"
             "'rowid' AS COLUMN_NAME,"
             "%d AS DATA_TYPE,"
             "'GENID' AS TYPE_NAME,"
             "19 AS COLUMN_SIZE,"
             "8 AS BUFFER_LENGTH,"
             "0 AS DECIMAL_DEGITS,"
             "%d AS PSEUDO_COLUMN",
             SQL_SCOPE_SESSION,
             SQL_BIGINT,
             SQL_PC_PSEUDO);

    ret = comdb2_SQLExecDirect(phstmt, (SQLCHAR *)metaquery, SQL_NTS);
    __debug("leaves method.");
    return ret;
}

SQLRETURN SQL_API SQLPrimaryKeys(
    SQLHSTMT       hstmt,
    SQLCHAR *      catalog,
    SQLSMALLINT    catalog_len,
    SQLCHAR *      schema,
    SQLSMALLINT    schema_len,
    SQLCHAR        *tbl,
    SQLSMALLINT    tbl_len)
{
    SQLRETURN ret;
    stmt_t *phstmt = (stmt_t *)hstmt;
    char metaquery[MAX_INTERNAL_QUERY_LEN + 1];
    size_t pos = 0;

    __debug("enters method.");

    /* Ignore catalog and schema */
    (void)catalog;
    (void)catalog_len;
    (void)schema;
    (void)schema_len;

    metaquery[MAX_INTERNAL_QUERY_LEN] = 0;
    pos = snprintf(metaquery, MAX_INTERNAL_QUERY_LEN,
                   "SELECT '%s' AS TABLE_CAT, NULL AS TABLE_SCHEM,"
                   "a.tablename AS TABLE_NAME,"
                   "a.columnname AS COLUMN_NAME,"
                   "(columnnumber + 1) AS KEY_SEQ,"
                   "a.keyname AS PK_NAME "
                   "FROM comdb2sys_keycomponents a, comdb2sys_keys b "
                   "WHERE a.tablename = b.tablename "
                   "AND a.keyname = b.keyname "
                   "AND (UPPER(isunique) = 'Y' or UPPER(isunique) = 'YES') "
                   "AND 1=1 ",
                   phstmt->dbc->ci.database);

    if (tbl != NULL) {
        if (tbl_len == SQL_NTS)
            tbl_len = (SQLSMALLINT)strlen((const char *)tbl);
        pos += snprintf(&metaquery[pos], MAX_INTERNAL_QUERY_LEN - pos,
                        " AND a.tablename LIKE '%*s'", tbl_len, tbl);
    }

    strncpy(&metaquery[pos],
            " ORDER BY a.tablename, columnnumber",
            MAX_INTERNAL_QUERY_LEN - pos);

    ret = comdb2_SQLExecDirect(phstmt, (SQLCHAR *)metaquery, SQL_NTS);
    __debug("leaves method.");
    return ret;
}


SQLRETURN SQL_API SQLForeignKeys(
    SQLHSTMT           hstmt,
    SQLCHAR 		   *pkcat,
    SQLSMALLINT        pkcatlen,
    SQLCHAR 		   *pkschem,
    SQLSMALLINT        pkschemlen,
    SQLCHAR 		   *pktbl,
    SQLSMALLINT        pktbllen,
    SQLCHAR 		   *fkcat,
    SQLSMALLINT        fkcatlen,
    SQLCHAR 		   *fkschem,
    SQLSMALLINT        fkschemlen,
    SQLCHAR 		   *fktbl,
    SQLSMALLINT        fktbllen)
{
    SQLRETURN ret;
    stmt_t *phstmt = (stmt_t *)hstmt;
    char metaquery[MAX_INTERNAL_QUERY_LEN + 1];
    size_t pos = 0;

    __debug("enters method.");

    (void)pkcat;
    (void)pkcatlen;
    (void)pkschem;
    (void)pkschemlen;
    (void)fkcat;
    (void)fkcatlen;
    (void)fkschem;
    (void)fkschemlen;


    metaquery[MAX_INTERNAL_QUERY_LEN] = 0;
    pos = snprintf(metaquery, MAX_INTERNAL_QUERY_LEN,
                   "SELECT '%s' AS PKTABLE_CAT,"
                   "NULL AS PKTABLE_SCHEM,"
                   "c.tablename AS PKTABLE_NAME,"
                   "c.columnname as PKCOLUMN_NAME,"
                   "'%s' as FKTABLE_CAT,"
                   "NULL as FKTABLE_SCHEM,"
                   "a.tablename as FKTABLE_NAME,"
                   "a.columnname as FKCOLUMN_NAME,"
                   "(a.columnnumber + 1) as KEY_SEQ,"
                   "CASE WHEN (upper(iscascadingupdate)='YES' or upper(iscascadingupdate)='Y') THEN %d ELSE %d END as UPDATE_RULE,"
                   "CASE WHEN (upper(iscascadingdelete)='YES' or upper(iscascadingdelete)='Y') THEN %d ELSE %d END as DELETE_RULE,"
                   "a.keyname as FK_NAME, c.keyname as PK_NAME,"
                   "%d as DEFERRABILITY "
                   "FROM comdb2sys_keycomponents a,"
                   "comdb2sys_constraints b,"
                   "comdb2sys_keycomponents c "
                   "WHERE a.tablename = b.tablename "
                   "AND a.keyname = b.keyname "
                   "AND b.foreigntablename = c.tablename "
                   "AND b.foreignkeyname = c.keyname",
                   phstmt->dbc->ci.database,
                   phstmt->dbc->ci.database,
                   SQL_CASCADE,
                   SQL_NO_ACTION,
                   SQL_CASCADE,
                   SQL_NO_ACTION,
                   SQL_INITIALLY_DEFERRED);

    if (pktbl != NULL) {
        if (pktbllen == SQL_NTS)
            pktbllen = (SQLSMALLINT)strlen((const char *)pktbl);
        pos += snprintf(&(metaquery[pos]), MAX_INTERNAL_QUERY_LEN - pos,
                        " AND b.foreigntablename LIKE '%*s'", pktbllen, pktbl);
    }

    if (fktbl != NULL) {
        if (fktbllen == SQL_NTS)
            fktbllen = (SQLSMALLINT)strlen((const char *)fktbl);
        pos += snprintf(&(metaquery[pos]), MAX_INTERNAL_QUERY_LEN - pos,
                        " AND b.tablename LIKE '%*s'", fktbllen, fktbl);
    }

    strncpy(&metaquery[pos],
            " ORDER BY PKTABLE_NAME, KEY_SEQ",
            MAX_INTERNAL_QUERY_LEN - pos);

    ret = comdb2_SQLExecDirect(phstmt, (SQLCHAR *)metaquery, SQL_NTS);
    __debug("leaves method.");
    return ret;
}


SQLRETURN SQL_API SQLStatistics(
    SQLHSTMT       hstmt,
    SQLCHAR *      catalog,
    SQLSMALLINT    catalog_len,
    SQLCHAR *      schema,
    SQLSMALLINT    schema_len,
    SQLCHAR *      tbl,
    SQLSMALLINT    tbl_len,
    SQLUSMALLINT   unique,
    SQLUSMALLINT   reserve)
{
    SQLRETURN ret;
    stmt_t *phstmt = (stmt_t *)hstmt;
    char metaquery[MAX_INTERNAL_QUERY_LEN + 1];
    size_t pos = 0;

    __debug("enters method.");

    /* Ignore catalog and schema */
    (void)catalog;
    (void)catalog_len;
    (void)schema;
    (void)schema_len;
    (void)reserve;

    metaquery[MAX_INTERNAL_QUERY_LEN] = 0;
    pos = snprintf(metaquery, MAX_INTERNAL_QUERY_LEN,
                   "SELECT '%s' AS TABLE_CAT, NULL AS TABLE_SCHEM,"
                   "a.tablename AS TABLE_NAME, "
                   "(UPPER(isunique) = 'NO' or UPPER(isunique) = 'N') AS NON_UNIQUE, "
                   "'' AS INDEX_QUALIFIER,"
                   "a.keyname AS INDEX_NAME,"
                   "%d AS TYPE,"
                   "(columnnumber + 1) AS ORDINAL_POSITION,"
                   "a.columnname AS COLUMN_NAME,"
                   "CASE WHEN (upper(isdescending) = 'NO' or upper(isdescending) = 'N') THEN 'A' ELSE 'D' END as ASC_OR_DESC,"
                   "0 as CARDINALITY, 0 as PAGES, null as FILTER_CONDITION "
                   "FROM comdb2sys_keycomponents a, comdb2sys_keys b "
                   "WHERE a.tablename = b.tablename "
                   "AND a.keyname = b.keyname",
                   phstmt->dbc->ci.database,
                   SQL_INDEX_OTHER);

    if (!!unique) {
        strncpy(&metaquery[pos],
                " AND (upper(isunique)='YES' or upper(isunique)='Y')",
                MAX_INTERNAL_QUERY_LEN - pos);
    }

    if (tbl != NULL) {
        if (tbl_len == SQL_NTS)
            tbl_len = (SQLSMALLINT)strlen((const char *)tbl);
        pos += snprintf(&metaquery[pos], MAX_INTERNAL_QUERY_LEN - pos,
                        " AND a.tablename LIKE '%*s'", tbl_len, tbl);
    }

    strncpy(&metaquery[pos],
            " ORDER BY a.tablename, isdescending, a.keyname, columnnumber",
            MAX_INTERNAL_QUERY_LEN - pos);

    ret = comdb2_SQLExecDirect(phstmt, (SQLCHAR *)metaquery, SQL_NTS);
    __debug("leaves method.");
    return ret;
}

SQLRETURN SQL_API SQLTables(
        SQLHSTMT       hstmt,
        SQLCHAR        *catalog,
        SQLSMALLINT    catalog_len,
        SQLCHAR        *schema,
        SQLSMALLINT    schema_len,
        SQLCHAR        *tbl,
        SQLSMALLINT    tbl_len,
        SQLCHAR        *tbl_tp,
        SQLSMALLINT    tbl_tp_len)
{
    stmt_t *phstmt = (stmt_t *)hstmt;
    char metaquery[MAX_INTERNAL_QUERY_LEN + 1];
    size_t pos;

    if(!hstmt)
        return SQL_INVALID_HANDLE;

    /* Ignore catalog and schema */
    (void)catalog;
    (void)catalog_len;
    (void)schema;
    (void)schema_len;

    metaquery[MAX_INTERNAL_QUERY_LEN] = 0;
    pos = snprintf(metaquery, MAX_INTERNAL_QUERY_LEN,
                   "SELECT '%s' AS TABLE_CAT, NULL AS TABLE_SCHEM,"
                   "name as TABLE_NAME, UPPER(type) AS TABLE_TYPE,"
                   "null AS REMARKS FROM sqlite_master WHERE 1=1",
                   phstmt->dbc->ci.database);

    if (tbl != NULL) {
        if (tbl_len == SQL_NTS)
            tbl_len = (SQLSMALLINT)strlen((const char *)tbl);
        if (tbl_len > 0) {
            pos += snprintf(&metaquery[pos], MAX_INTERNAL_QUERY_LEN - pos,
                    " AND TABLE_NAME LIKE '%*s'", tbl_len, tbl);
        }
    }

    if (tbl_tp != NULL) {
        if (tbl_tp_len == SQL_NTS)
            tbl_tp_len = (SQLSMALLINT)strlen((const char *)tbl_tp);
        if (tbl_tp_len > 0) {
            pos += snprintf(&metaquery[pos], MAX_INTERNAL_QUERY_LEN - pos,
                    " AND (0=1 ");
            char *tok, *last;
            tok = strtok_r((char *)tbl_tp, ",", &last);
            while (tok != NULL) {
                pos += snprintf(&metaquery[pos], MAX_INTERNAL_QUERY_LEN - pos,
                        " OR TABLE_TYPE LIKE '%s'", tok);
                tok = strtok_r (NULL, ",", &last);
            }
            pos += snprintf(&metaquery[pos], MAX_INTERNAL_QUERY_LEN - pos,
                    ")");
        }
    } else {
        pos += snprintf(&metaquery[pos], MAX_INTERNAL_QUERY_LEN - pos,
                        " AND TABLE_TYPE IN('TABLE','VIEW')");
    }
    return comdb2_SQLExecDirect(phstmt, (SQLCHAR *)metaquery, SQL_NTS);
}
