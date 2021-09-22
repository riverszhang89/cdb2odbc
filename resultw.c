#include "wcs.h"

SQLRETURN SQL_API SQLDescribeColW(
        SQLHSTMT       hstmt,
        SQLUSMALLINT   col,
        SQLWCHAR        *col_name,
        SQLSMALLINT    col_name_max,
        SQLSMALLINT    *col_name_len,
        SQLSMALLINT    *sql_data_type,
        SQLULEN        *col_size,
        SQLSMALLINT    *decimal_digits,
        SQLSMALLINT    *nullable)
{
    SQLRETURN ret;
    int len;
    SQLCHAR *col_name_ansi;

    col_name_ansi = malloc(col_name_max);
    ret = __SQLDescribeCol(hstmt, col, col_name_ansi, col_name_max - 1, col_name_len, sql_data_type, col_size, decimal_digits, nullable);

    if (ret == SQL_SUCCESS) {
        len = utf8_to_ucs2(col_name_ansi, col_name, col_name_max * sizeof(SQLWCHAR));
        if (len > 0 && col_name_len)
            *col_name_len = len - 1;
    }

    free(col_name_ansi);

    __debug("leaves method.");
    return ret;
}

SQLRETURN SQL_API SQLColAttributeW(
        SQLHSTMT      hstmt, 
        SQLUSMALLINT  col, 
        SQLUSMALLINT  field, 
        SQLPOINTER    text_attr, 
        SQLSMALLINT   attr_max, 
        SQLSMALLINT   *attr_len, 
        SQLLEN        *num_attr)
{
    int len = 0;
    SQLSMALLINT attr_len_ansi;
    SQLPOINTER text_ansi = malloc(attr_max + 1);

    __debug("enters method.");
    SQLRETURN ret = __SQLColAttribute(hstmt, col, field, text_ansi, attr_max, &attr_len_ansi, num_attr);

    if (ret == SQL_SUCCESS) {
        switch (field) {
            case SQL_DESC_NAME:
            case SQL_DESC_LABEL:
                len = utf8_to_ucs2((SQLCHAR *)text_ansi, (SQLWCHAR *)text_attr, attr_max);
                if (len > 0 && attr_len != NULL)
                    *attr_len = (len - 1) *  sizeof(SQLWCHAR);
                break;

        }
    }

    free(text_ansi);

    __debug("leaves method.");
    return ret;
}
