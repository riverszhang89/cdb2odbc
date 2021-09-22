#include "wcs.h"

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
            val = malloc(buflen / sizeof(SQLWCHAR));
            writablelen = buflen / sizeof(SQLWCHAR);
            break;
        default:
            val = value_ptr;
            writablelen = buflen;
            break;
    }

    SQLRETURN ret = __SQLGetInfo(hdbc, type, val, writablelen, &ansistrlen);

    if (ret == SQL_SUCCESS) {
        switch (type) {
            case SQL_DATABASE_NAME:
            case SQL_DBMS_NAME:
            case SQL_DBMS_VER:
            case SQL_DRIVER_NAME:
            case SQL_DRIVER_VER:
                len = utf8_to_ucs2((SQLCHAR *)val, (SQLWCHAR *)value_ptr, buflen);
                if (len > 0 && str_len != NULL)
                    *str_len = (len - 1) *  sizeof(SQLWCHAR);
            case SQL_DRIVER_ODBC_VER:
                free(val);
                break;
            default:
                break;
        }
    }

    __debug("leaves method.");
    return SQL_SUCCESS;
}

SQLRETURN SQL_API SQLTablesW(
        SQLHSTMT        hstmt,
        SQLWCHAR        *catalog,
        SQLSMALLINT     catalog_len,
        SQLWCHAR        *schema,
        SQLSMALLINT     schema_len,
        SQLWCHAR        *tbl,
        SQLSMALLINT     tbl_len,
        SQLWCHAR        *tbl_tp,
        SQLSMALLINT     tbl_tp_len)
{ 
    int needed;
    SQLRETURN ret;
    SQLCHAR *catalog_ansi, *schema_ansi, *tbl_ansi, *tbl_tp_ansi;

    catalog_ansi = schema_ansi = tbl_ansi = tbl_tp_ansi = NULL;

    __debug("enters method.");

    if (catalog != NULL) {
        needed = sqlwcharbytelen(catalog) + sizeof(SQLWCHAR);
        catalog_ansi = malloc(needed);
        ucs2_to_utf8(catalog, catalog_ansi, needed);
    }

    if (schema != NULL) {
        needed = sqlwcharbytelen(schema) + sizeof(SQLWCHAR);
        schema_ansi = malloc(needed);
        ucs2_to_utf8(schema, schema_ansi, needed);
    }

    if (tbl != NULL) {
        needed = sqlwcharbytelen(tbl) + sizeof(SQLWCHAR);
        tbl_ansi = malloc(needed);
        ucs2_to_utf8(tbl, tbl_ansi, needed);
    }

    if (tbl_tp != NULL) {
        needed = sqlwcharbytelen(tbl_tp) + sizeof(SQLWCHAR);
        tbl_tp_ansi = malloc(needed);
        ucs2_to_utf8(tbl_tp, tbl_tp_ansi, needed);
    }

    ret = __SQLTables(hstmt, catalog_ansi, SQL_NTS, schema_ansi, SQL_NTS, tbl_ansi, SQL_NTS, tbl_tp_ansi, SQL_NTS);

    free(catalog_ansi);
    free(schema_ansi);
    free(tbl_ansi);
    free(tbl_tp_ansi);

    __debug("leaves method.");
    return ret;
}

SQLRETURN SQL_API SQLColumnPrivilegesW(
    SQLHSTMT       hstmt,
    SQLWCHAR *     catalog,
    SQLSMALLINT    catalog_len,
    SQLWCHAR *     schema,
    SQLSMALLINT    schema_len,
    SQLWCHAR       *tbl,
    SQLSMALLINT    tbl_len,
    SQLWCHAR *     column,
    SQLSMALLINT    column_len)
{
    int needed;
    SQLRETURN ret;
    SQLCHAR *catalog_ansi, *schema_ansi, *tbl_ansi, *column_ansi;

    catalog_ansi = schema_ansi = tbl_ansi = column_ansi = NULL;

    __debug("enters method.");

    if (catalog != NULL) {
        needed = sqlwcharbytelen(catalog) + sizeof(SQLWCHAR);
        catalog_ansi = malloc(needed);
        ucs2_to_utf8(catalog, catalog_ansi, needed);
    }

    if (schema != NULL) {
        needed = sqlwcharbytelen(schema) + sizeof(SQLWCHAR);
        schema_ansi = malloc(needed);
        ucs2_to_utf8(schema, schema_ansi, needed);
    }

    if (tbl != NULL) {
        needed = sqlwcharbytelen(tbl) + sizeof(SQLWCHAR);
        tbl_ansi = malloc(needed);
        ucs2_to_utf8(tbl, tbl_ansi, needed);
    }

    if (column != NULL) {
        needed = sqlwcharbytelen(column) + sizeof(SQLWCHAR);
        column_ansi = malloc(needed);
        ucs2_to_utf8(column, column_ansi, needed);
    }

    ret = __SQLColumnPrivileges(hstmt, catalog_ansi, SQL_NTS, schema_ansi, SQL_NTS, tbl_ansi, SQL_NTS, column_ansi, SQL_NTS);

    free(catalog_ansi);
    free(schema_ansi);
    free(tbl_ansi);
    free(column_ansi);

    __debug("leaves method.");
    return ret;

}

SQLRETURN SQL_API SQLColumnsW(
        SQLHSTMT       hstmt,
        SQLWCHAR *     catalog,
        SQLSMALLINT    catalog_len,
        SQLWCHAR *     schema,
        SQLSMALLINT    schema_len,
        SQLWCHAR *     tbl,
        SQLSMALLINT    tbl_len,
        SQLWCHAR *     column,
        SQLSMALLINT    column_len)
{
    int needed;
    SQLRETURN ret;
    SQLCHAR *catalog_ansi, *schema_ansi, *tbl_ansi, *column_ansi;

    catalog_ansi = schema_ansi = tbl_ansi = column_ansi = NULL;

    __debug("enters method.");

    if (catalog != NULL) {
        needed = sqlwcharbytelen(catalog) + sizeof(SQLWCHAR);
        catalog_ansi = malloc(needed);
        ucs2_to_utf8(catalog, catalog_ansi, needed);
    }

    if (schema != NULL) {
        needed = sqlwcharbytelen(schema) + sizeof(SQLWCHAR);
        schema_ansi = malloc(needed);
        ucs2_to_utf8(schema, schema_ansi, needed);
    }

    if (tbl != NULL) {
        needed = sqlwcharbytelen(tbl) + sizeof(SQLWCHAR);
        tbl_ansi = malloc(needed);
        ucs2_to_utf8(tbl, tbl_ansi, needed);
    }

    if (column != NULL) {
        needed = sqlwcharbytelen(column) + sizeof(SQLWCHAR);
        column_ansi = malloc(needed);
        ucs2_to_utf8(column, column_ansi, needed);
    }

    ret = __SQLColumns(hstmt, catalog_ansi, SQL_NTS, schema_ansi, SQL_NTS, tbl_ansi, SQL_NTS, column_ansi, SQL_NTS);

    free(catalog_ansi);
    free(schema_ansi);
    free(tbl_ansi);
    free(column_ansi);

    __debug("leaves method.");
    return ret;
}

SQLRETURN SQL_API SQLForeignKeysW(
    SQLHSTMT           hstmt,
    SQLWCHAR           *pkcat,
    SQLSMALLINT        pkcatlen,
    SQLWCHAR           *pkschem,
    SQLSMALLINT        pkschemlen,
    SQLWCHAR           *pktbl,
    SQLSMALLINT        pktbllen,
    SQLWCHAR           *fkcat,
    SQLSMALLINT        fkcatlen,
    SQLWCHAR           *fkschem,
    SQLSMALLINT        fkschemlen,
    SQLWCHAR           *fktbl,
    SQLSMALLINT        fktbllen)
{
    int needed;
    SQLRETURN ret;
    SQLCHAR *pkcat_ansi, *pkschem_ansi, *pktbl_ansi, *fkcat_ansi, *fkschem_ansi, *fktbl_ansi;
    pkcat_ansi = pkschem_ansi = pktbl_ansi = fkcat_ansi = fkschem_ansi = fktbl_ansi = NULL;

    __debug("enters method.");

    if (pkcat != NULL) {
        needed = sqlwcharbytelen(pkcat) + sizeof(SQLWCHAR);
        pkcat_ansi = malloc(needed);
        ucs2_to_utf8(pkcat, pkcat_ansi, needed);
    }

    if (pkschem != NULL) {
        needed = sqlwcharbytelen(pkschem) + sizeof(SQLWCHAR);
        pkschem_ansi = malloc(needed);
        ucs2_to_utf8(pkschem, pkschem_ansi, needed);
    }
    
    if (pktbl != NULL) {
        needed = sqlwcharbytelen(pktbl) + sizeof(SQLWCHAR);
        pktbl_ansi = malloc(needed);
        ucs2_to_utf8(pktbl, pktbl_ansi, needed);
    }

    if (fkcat != NULL) {
        needed = sqlwcharbytelen(fkcat) + sizeof(SQLWCHAR);
        fkcat_ansi = malloc(needed);
        ucs2_to_utf8(fkcat, fkcat_ansi, needed);
    }

    if (fkschem != NULL) {
        needed = sqlwcharbytelen(fkschem) + sizeof(SQLWCHAR);
        fkschem_ansi = malloc(needed);
        ucs2_to_utf8(fkschem, fkschem_ansi, needed);
    }

    if (fktbl != NULL) {
        needed = sqlwcharbytelen(fktbl) + sizeof(SQLWCHAR);
        fktbl_ansi = malloc(needed);
        ucs2_to_utf8(fktbl, fktbl_ansi, needed);
    }

    ret = __SQLForeignKeys(
                    hstmt,
                    pkcat_ansi, SQL_NTS,
                    pkschem_ansi, SQL_NTS,
                    pktbl_ansi, SQL_NTS,
                    fkcat_ansi, SQL_NTS,
                    fkschem_ansi, SQL_NTS,
                    fktbl_ansi, SQL_NTS);

    free(pkcat_ansi);
    free(pkschem_ansi);
    free(pktbl_ansi);
    free(fkcat_ansi);
    free(fkschem_ansi);
    free(fktbl_ansi);

    __debug("leaves method.");
    return ret;
}

SQLRETURN SQL_API SQLGetTypeInfoW(SQLHSTMT hstmt, SQLSMALLINT type)
{
	return __SQLGetTypeInfo(hstmt, type);
}

SQLRETURN SQL_API SQLPrimaryKeysW(
    SQLHSTMT       hstmt,
    SQLWCHAR *     catalog,
    SQLSMALLINT    catalog_len,
    SQLWCHAR *     schema,
    SQLSMALLINT    schema_len,
    SQLWCHAR       *tbl,
    SQLSMALLINT    tbl_len)
{
    int needed;
    SQLRETURN ret;
    SQLCHAR *catalog_ansi, *schema_ansi, *tbl_ansi;

    catalog_ansi = schema_ansi = tbl_ansi = NULL;

    __debug("enters method.");

    if (catalog != NULL) {
        needed = sqlwcharbytelen(catalog) + sizeof(SQLWCHAR);
        catalog_ansi = malloc(needed);
        ucs2_to_utf8(catalog, catalog_ansi, needed);
    }

    if (schema != NULL) {
        needed = sqlwcharbytelen(schema) + sizeof(SQLWCHAR);
        schema_ansi = malloc(needed);
        ucs2_to_utf8(schema, schema_ansi, needed);
    }

    if (tbl != NULL) {
        needed = sqlwcharbytelen(tbl) + sizeof(SQLWCHAR);
        tbl_ansi = malloc(needed);
        ucs2_to_utf8(tbl, tbl_ansi, needed);
    }

    ret = __SQLPrimaryKeys(hstmt, catalog_ansi, SQL_NTS, schema_ansi, SQL_NTS, tbl_ansi, SQL_NTS);

    free(catalog_ansi);
    free(schema_ansi);
    free(tbl_ansi);

    __debug("leaves method.");
    return ret;
}

SQLRETURN SQL_API SQLProcedureColumnsW(
    SQLHSTMT       hstmt,
    SQLWCHAR*      catalog,
    SQLSMALLINT    catalog_len,
    SQLWCHAR*      schema,
    SQLSMALLINT    schema_len,
    SQLWCHAR*      proc,
    SQLSMALLINT    proc_len,
    SQLWCHAR*      column,
    SQLSMALLINT    column_len)
{
    int needed;
    SQLRETURN ret;
    SQLCHAR *catalog_ansi, *schema_ansi, *proc_ansi, *column_ansi;

    catalog_ansi = schema_ansi = proc_ansi = column_ansi = NULL;

    __debug("enters method.");

    if (catalog != NULL) {
        needed = sqlwcharbytelen(catalog) + sizeof(SQLWCHAR);
        catalog_ansi = malloc(needed);
        ucs2_to_utf8(catalog, catalog_ansi, needed);
    }

    if (schema != NULL) {
        needed = sqlwcharbytelen(schema) + sizeof(SQLWCHAR);
        schema_ansi = malloc(needed);
        ucs2_to_utf8(schema, schema_ansi, needed);
    }

    if (proc != NULL) {
        needed = sqlwcharbytelen(proc) + sizeof(SQLWCHAR);
        proc_ansi = malloc(needed);
        ucs2_to_utf8(proc, proc_ansi, needed);
    }

    if (column != NULL) {
        needed = sqlwcharbytelen(column) + sizeof(SQLWCHAR);
        column_ansi = malloc(needed);
        ucs2_to_utf8(column, column_ansi, needed);
    }

    ret = __SQLColumns(hstmt, catalog_ansi, SQL_NTS, schema_ansi, SQL_NTS, proc_ansi, SQL_NTS, column_ansi, SQL_NTS);

    free(catalog_ansi);
    free(schema_ansi);
    free(proc_ansi);
    free(column_ansi);

    __debug("leaves method.");
    return ret;
}

SQLRETURN SQL_API SQLProceduresW(
    SQLHSTMT       hstmt,
    SQLWCHAR *     catalog,
    SQLSMALLINT    catalog_len,
    SQLWCHAR *     schema,
    SQLSMALLINT    schema_len,
    SQLWCHAR *     proc,
    SQLSMALLINT    proc_len)
{
    int needed;
    SQLRETURN ret;
    SQLCHAR *catalog_ansi, *schema_ansi, *proc_ansi;

    catalog_ansi = schema_ansi = proc_ansi = NULL;

    __debug("enters method.");

    if (catalog != NULL) {
        needed = sqlwcharbytelen(catalog) + sizeof(SQLWCHAR);
        catalog_ansi = malloc(needed);
        ucs2_to_utf8(catalog, catalog_ansi, needed);
    }

    if (schema != NULL) {
        needed = sqlwcharbytelen(schema) + sizeof(SQLWCHAR);
        schema_ansi = malloc(needed);
        ucs2_to_utf8(schema, schema_ansi, needed);
    }

    if (proc != NULL) {
        needed = sqlwcharbytelen(proc) + sizeof(SQLWCHAR);
        proc_ansi = malloc(needed);
        ucs2_to_utf8(proc, proc_ansi, needed);
    }

    ret = __SQLProcedures(hstmt, catalog_ansi, SQL_NTS, schema_ansi, SQL_NTS, proc_ansi, SQL_NTS);

    free(catalog_ansi);
    free(schema_ansi);
    free(proc_ansi);

    __debug("leaves method.");
    return ret;

}

SQLRETURN SQL_API SQLSpecialColumnsW(
    SQLHSTMT       hstmt,
    SQLUSMALLINT   type,
    SQLWCHAR*      catalog,
    SQLSMALLINT    catalog_len,
    SQLWCHAR*      schema,
    SQLSMALLINT    schema_len,
    SQLWCHAR       *tbl,
    SQLSMALLINT    tbl_len,
    SQLUSMALLINT   scope,
    SQLUSMALLINT   nullable)
{
    int needed;
    SQLRETURN ret;
    SQLCHAR *catalog_ansi, *schema_ansi, *tbl_ansi;

    catalog_ansi = schema_ansi = tbl_ansi = NULL;

    __debug("enters method.");

    if (catalog != NULL) {
        needed = sqlwcharbytelen(catalog) + sizeof(SQLWCHAR);
        catalog_ansi = malloc(needed);
        ucs2_to_utf8(catalog, catalog_ansi, needed);
    }

    if (schema != NULL) {
        needed = sqlwcharbytelen(schema) + sizeof(SQLWCHAR);
        schema_ansi = malloc(needed);
        ucs2_to_utf8(schema, schema_ansi, needed);
    }

    if (tbl != NULL) {
        needed = sqlwcharbytelen(tbl) + sizeof(SQLWCHAR);
        tbl_ansi = malloc(needed);
        ucs2_to_utf8(tbl, tbl_ansi, needed);
    }

    ret = __SQLSpecialColumns(
                    hstmt,
                    type,
                    catalog_ansi, SQL_NTS,
                    schema_ansi, SQL_NTS,
                    tbl_ansi, SQL_NTS,
                    scope,
                    nullable);

    free(catalog_ansi);
    free(schema_ansi);
    free(tbl_ansi);

    __debug("leaves method.");
    return ret;
}

SQLRETURN SQL_API SQLStatisticsW(
    SQLHSTMT       hstmt,
    SQLWCHAR*      catalog,
    SQLSMALLINT    catalog_len,
    SQLWCHAR*      schema,
    SQLSMALLINT    schema_len,
    SQLWCHAR*      tbl,
    SQLSMALLINT    tbl_len,
    SQLUSMALLINT   unique,
    SQLUSMALLINT   reserve)
{
    int needed;
    SQLRETURN ret;
    SQLCHAR *catalog_ansi, *schema_ansi, *tbl_ansi;

    catalog_ansi = schema_ansi = tbl_ansi = NULL;

    __debug("enters method.");

    if (catalog != NULL) {
        needed = sqlwcharbytelen(catalog) + sizeof(SQLWCHAR);
        catalog_ansi = malloc(needed);
        ucs2_to_utf8(catalog, catalog_ansi, needed);
    }

    if (schema != NULL) {
        needed = sqlwcharbytelen(schema) + sizeof(SQLWCHAR);
        schema_ansi = malloc(needed);
        ucs2_to_utf8(schema, schema_ansi, needed);
    }

    if (tbl != NULL) {
        needed = sqlwcharbytelen(tbl) + sizeof(SQLWCHAR);
        tbl_ansi = malloc(needed);
        ucs2_to_utf8(tbl, tbl_ansi, needed);
    }

    ret = __SQLStatistics(
                    hstmt,
                    catalog_ansi, SQL_NTS,
                    schema_ansi, SQL_NTS,
                    tbl_ansi, SQL_NTS,
                    unique,
                    reserve);

    free(catalog_ansi);
    free(schema_ansi);
    free(tbl_ansi);

    __debug("leaves method.");
    return ret;
}

SQLRETURN SQL_API SQLTablePrivilegesW(
    SQLHSTMT       hstmt,
    SQLWCHAR*      catalog,
    SQLSMALLINT    catalog_len,
    SQLWCHAR*      schema,
    SQLSMALLINT    schema_len,
    SQLWCHAR       *tbl,
    SQLSMALLINT    tbl_len)
{
    int needed;
    SQLRETURN ret;
    SQLCHAR *catalog_ansi, *schema_ansi, *tbl_ansi;

    catalog_ansi = schema_ansi = tbl_ansi = NULL;

    __debug("enters method.");

    if (catalog != NULL) {
        needed = sqlwcharbytelen(catalog) + sizeof(SQLWCHAR);
        catalog_ansi = malloc(needed);
        ucs2_to_utf8(catalog, catalog_ansi, needed);
    }

    if (schema != NULL) {
        needed = sqlwcharbytelen(schema) + sizeof(SQLWCHAR);
        schema_ansi = malloc(needed);
        ucs2_to_utf8(schema, schema_ansi, needed);
    }

    if (tbl != NULL) {
        needed = sqlwcharbytelen(tbl) + sizeof(SQLWCHAR);
        tbl_ansi = malloc(needed);
        ucs2_to_utf8(tbl, tbl_ansi, needed);
    }

    ret = __SQLTablePrivileges(
                    hstmt,
                    catalog_ansi, SQL_NTS,
                    schema_ansi, SQL_NTS,
                    tbl_ansi, SQL_NTS);

    free(catalog_ansi);
    free(schema_ansi);
    free(tbl_ansi);

    __debug("leaves method.");
    return ret;
}
