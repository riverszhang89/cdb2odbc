#include "convert.h"

/*
   Converts a real number to a @c_data_type value. 

   FIXME Currently, this function only supports real-to-float/double conversion.
 */
conv_resp convert_cdb2real(const void *value, int size, SQLSMALLINT c_data_type, SQLPOINTER target_ptr, SQLLEN target_len, SQLLEN *str_len)
{
    double num = *((double*) value);

    switch(c_data_type) {
        case SQL_C_CHAR:
            *str_len = snprintf((char *)target_ptr, target_len, "%f", num);
            if(*str_len >= target_len)
                return CONV_TRUNCATED;
            break;

        case SQL_C_WCHAR:
            *str_len = swprintf((wchar_t *)target_ptr, target_len / sizeof(wchar_t), L"%f", num);
            if(*str_len >= target_len / (SQLLEN)sizeof(wchar_t))
                return CONV_TRUNCATED;
            break;

        case SQL_C_FLOAT:
            SET_SQLREAL(target_ptr, num, *str_len);
            break;
        
        case SQL_C_DOUBLE:
        case SQL_C_DEFAULT:
            SET_SQLDOUBLE(target_ptr, num, *str_len);
            break;
        
        default:
            return CONV_IMPOSSIBLE;
    }

    return CONV_YEAH;
}

/*
   Converts a datetime value to a @c_data_type value. 

   FIXME Currently, this function only supports datetime-to-date/time/timestamp conversion.
 */
conv_resp convert_cdb2datetime(const void *value, int size, SQLSMALLINT c_data_type, SQLPOINTER target_ptr, SQLLEN target_len, SQLLEN *str_len)
{
    cdb2_client_datetime_t *datetime = (cdb2_client_datetime_t *)value;
    DATE_STRUCT *date;
    TIME_STRUCT *time;
    TIMESTAMP_STRUCT *timestamp;

    switch(c_data_type) {
        case SQL_C_CHAR:
            *str_len = snprintf((char *)target_ptr, target_len, "%.4d-%.2d-%.2d %.2d:%.2d:%.2d.%.3d %.*s", 
                    datetime->tm.tm_year + 1900, datetime->tm.tm_mon + 1, datetime->tm.tm_mday, datetime->tm.tm_hour,
                    datetime->tm.tm_min, datetime->tm.tm_sec, datetime->msec, CDB2_MAX_TZNAME, datetime->tzname);
            if(*str_len >= target_len)
                return CONV_TRUNCATED;
            break;

        case SQL_C_WCHAR:
            *str_len = swprintf((wchar_t *)target_ptr, target_len / sizeof(wchar_t),
                    L"%.4d-%.2d-%.2d %.2d:%.2d:%.2d.%.3d %.*hs", datetime->tm.tm_year + 1900,
                    datetime->tm.tm_mon + 1, datetime->tm.tm_mday, datetime->tm.tm_hour,
                    datetime->tm.tm_min, datetime->tm.tm_sec, datetime->msec, CDB2_MAX_TZNAME, datetime->tzname);
            if(*str_len >= target_len / (SQLLEN)sizeof(wchar_t))
                return CONV_TRUNCATED;
            break;
            
        case SQL_C_TYPE_DATE:
            date = (DATE_STRUCT *)target_ptr;
            date->year = (SQLSMALLINT)datetime->tm.tm_year + 1900;
            date->month = (SQLSMALLINT)datetime->tm.tm_mon + 1;
            date->day = (SQLSMALLINT)datetime->tm.tm_mday;
            *str_len = sizeof(DATE_STRUCT);
            break;

        case SQL_C_TYPE_TIME:
            time = (TIME_STRUCT *)target_ptr;
			time->hour = (SQLUSMALLINT)datetime->tm.tm_hour;
            time->minute = (SQLUSMALLINT)datetime->tm.tm_min;
            time->second = (SQLUSMALLINT)datetime->tm.tm_sec;
            *str_len = sizeof(TIME_STRUCT);
            break;

        case SQL_C_TYPE_TIMESTAMP:
        case SQL_C_DEFAULT:
            timestamp = (TIMESTAMP_STRUCT *)target_ptr;
            timestamp->year = (SQLSMALLINT)datetime->tm.tm_year + 1900;
            timestamp->month = (SQLUSMALLINT)datetime->tm.tm_mon + 1;
            timestamp->day = (SQLUSMALLINT)datetime->tm.tm_mday;
            timestamp->hour = (SQLUSMALLINT)datetime->tm.tm_hour;
            timestamp->minute = (SQLUSMALLINT)datetime->tm.tm_min;
            timestamp->second = (SQLUSMALLINT)datetime->tm.tm_sec;
            timestamp->fraction = datetime->msec * 1000000;
            *str_len = sizeof(TIMESTAMP_STRUCT);
            break;

        default:
            return CONV_IMPOSSIBLE;
    }
    return CONV_YEAH;
}

/*
   Converts a intervalds value to a @c_data_type value. 

   FIXME Currently, this function only supports ds-to-interval day/hour/min/sec... conversion.
 */
conv_resp convert_cdb2inds(const void *value, int size, SQLSMALLINT c_data_type, SQLPOINTER target_ptr, SQLLEN target_len, SQLLEN *str_len)
{
    cdb2_client_intv_ds_t *intv_cdb2 = (cdb2_client_intv_ds_t *)value;
    SQL_INTERVAL_STRUCT *intv_odbc = (SQL_INTERVAL_STRUCT *)target_ptr;

    switch(c_data_type) {
        case SQL_C_CHAR:
            *str_len = snprintf((char *)target_ptr, target_len, "%d %d:%d:%d.%d",
                    intv_cdb2->sign * intv_cdb2->days, intv_cdb2->hours, intv_cdb2->mins, intv_cdb2->sec, intv_cdb2->msec);
            if(*str_len >= target_len)
                return CONV_TRUNCATED;
            break;
        case SQL_C_WCHAR:
            *str_len = swprintf((wchar_t *)target_ptr, target_len / sizeof(wchar_t), L"%d %d:%d:%d.%d",
                    intv_cdb2->sign * intv_cdb2->days, intv_cdb2->hours, intv_cdb2->mins, intv_cdb2->sec, intv_cdb2->msec);
            if(*str_len >= target_len / (SQLLEN)sizeof(wchar_t))
                return CONV_TRUNCATED;
            break;
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
        case SQL_C_DEFAULT:
            /* I will not do any calculation here (like 2 days and 6 hrs ==> 54 hrs). 
               The flag @interval_type will always set to SQL_IS_DAY_TO_SECOND. */
			intv_odbc->interval_type = SQL_IS_DAY_TO_SECOND;
			intv_odbc->interval_sign = (SQLSMALLINT)intv_cdb2->sign;
            intv_odbc->intval.day_second.day = intv_cdb2->days;
            intv_odbc->intval.day_second.hour = intv_cdb2->hours;
            intv_odbc->intval.day_second.minute = intv_cdb2->mins;
            intv_odbc->intval.day_second.second = intv_cdb2->sec;
            intv_odbc->intval.day_second.fraction = intv_cdb2->msec * 1000000;
            *str_len = sizeof(SQL_INTERVAL_STRUCT);
            break;

        default:
            return CONV_IMPOSSIBLE;
    }
    return CONV_YEAH;
}

/*
   Converts a intervalym value to a @c_data_type value. 

   FIXME Currently, this function only supports ym-to-interval month/year/year&month conversion.
 */
conv_resp convert_cdb2inym(const void *value, int size, SQLSMALLINT c_data_type, SQLPOINTER target_ptr, SQLLEN target_len, SQLLEN *str_len)
{
    cdb2_client_intv_ym_t *intv_cdb2 = (cdb2_client_intv_ym_t *)value;
    SQL_INTERVAL_STRUCT *intv_odbc = (SQL_INTERVAL_STRUCT *)target_ptr;

    switch(c_data_type) {
        case SQL_C_CHAR:
            *str_len = snprintf((char *)target_ptr, target_len, "%d-%d",
                    intv_cdb2->sign * intv_cdb2->years, intv_cdb2->months);
            if(*str_len >= target_len)
                return CONV_TRUNCATED;
            break;

        case SQL_C_WCHAR:
            *str_len = swprintf((wchar_t *)target_ptr, target_len / sizeof(wchar_t), L"%d-%d",
                    intv_cdb2->sign * intv_cdb2->years, intv_cdb2->months);
            if(*str_len >= target_len / (SQLLEN)sizeof(wchar_t))
                return CONV_TRUNCATED;
            break;

        case SQL_C_INTERVAL_MONTH:
        case SQL_C_INTERVAL_YEAR:
        case SQL_C_INTERVAL_YEAR_TO_MONTH:
        case SQL_C_DEFAULT:
			intv_odbc->interval_type = SQL_IS_YEAR_TO_MONTH;
			intv_odbc->interval_sign = (SQLSMALLINT)intv_cdb2->sign;
            intv_odbc->intval.year_month.year = intv_cdb2->years;
            intv_odbc->intval.year_month.month = intv_cdb2->months;
            *str_len = sizeof(SQL_INTERVAL_STRUCT);
            break;

        default:
            return CONV_IMPOSSIBLE;
    }

    return CONV_YEAH;
}

/*
   Converts a large object to a @c_data_type value. 

   FIXME Currently, this function only supports blob-to-binary conversion.
 */
conv_resp convert_cdb2blob(const void *value, int size, SQLSMALLINT c_data_type, SQLPOINTER target_ptr, SQLLEN target_len, SQLLEN *str_len)
{
    conv_resp resp = CONV_YEAH;

    switch(c_data_type) {
        case SQL_C_BINARY:
        case SQL_C_DEFAULT:
            *str_len = size;
            if(size > target_len) {
                size = (int)target_len;
                resp = CONV_TRUNCATED;
            }
            memcpy(target_ptr, value, size);
            break;

        case SQL_C_CHAR:
            *str_len = size - 1;
            if(size >= target_len - 1) {
                size = (int)(target_len - 1);
                resp = CONV_TRUNCATED;
            }
            memcpy(target_ptr, value, size);
            ((char *)target_ptr)[size] = '\0';
            break;

        default:
            return CONV_IMPOSSIBLE;
    }

    return resp;
}

/*
   Converts a cdb2 integer to a @c_data_type value.

   This function exactly follows ODBC standard and is fully functional.
 */
conv_resp convert_cdb2int(const void *value, int size, SQLSMALLINT c_data_type, SQLPOINTER target_ptr, SQLLEN target_len, SQLLEN *str_len)
{
    LL num = *((LL*) value);

    switch(c_data_type) {
        case SQL_C_CHAR:
            *str_len = snprintf((char *)target_ptr, target_len, "%lld", num);
            if(*str_len >= target_len)
                return CONV_TRUNCATED;
            break;

        case SQL_C_WCHAR:
            *str_len = swprintf((wchar_t *)target_ptr, target_len / sizeof(wchar_t), L"%lld", num);
            if(*str_len >= target_len / (SQLLEN)sizeof(wchar_t))
                return CONV_TRUNCATED;
            break;

        case SQL_C_STINYINT:
        case SQL_C_TINYINT:
            SET_SQLSCHAR(target_ptr, num, *str_len);
            break;

        case SQL_C_UTINYINT:
            SET_SQLCHAR(target_ptr, num, *str_len);
            break;

        case SQL_C_SBIGINT:
        case SQL_C_DEFAULT:
            SET_SQLBIGINT(target_ptr, num, *str_len);
            break;

        case SQL_C_UBIGINT:
            SET_SQLUBIGINT(target_ptr, num, *str_len);
            break;

        case SQL_C_SSHORT:
        case SQL_C_SHORT:
            SET_SQLSMALLINT(target_ptr, num, *str_len);
            break;

        case SQL_C_USHORT:
            SET_SQLUSMALLINT(target_ptr, num, *str_len);
            break;

        case SQL_C_SLONG:
        case SQL_C_LONG:
            SET_SQLINT(target_ptr, num, *str_len);
            break;

        case SQL_C_ULONG:
            SET_SQLUINT(target_ptr, num, *str_len);
            break;

        case SQL_C_FLOAT:
            SET_SQLFLOAT(target_ptr, num, *str_len);
            break;

        case SQL_C_DOUBLE:
            SET_SQLDOUBLE(target_ptr, num, *str_len);
            break;

        case SQL_C_BIT:
            if(num != 1 && num != 0)
                return CONV_IMPOSSIBLE; 
            SET_SQLCHAR(target_ptr, num, *str_len);
            break;

        case SQL_C_BINARY:
            memcpy(target_ptr, value, target_len > size ? size : target_len);
            *str_len = size;
            break;

        default:
            return CONV_IMPOSSIBLE;
    }

    return CONV_YEAH;
}

/*
   Converts a cdb2 cstring to a @c_data_type value.
   FIXME Only cstring-to-char/numeric conversion is supported currently.
 */
conv_resp convert_cdb2cstring(const void *value, int size, SQLSMALLINT c_data_type, SQLPOINTER target_ptr, SQLLEN target_len, SQLLEN *str_len)
{
    conv_resp resp = CONV_YEAH;

    switch(c_data_type) {
        case SQL_C_CHAR:
        case SQL_C_DEFAULT:
            /* size is str_len + 1 */
            my_strncpy_out((char *)target_ptr, value, target_len);
            *str_len = size - 1;
            if(size > target_len)
                resp = CONV_TRUNCATED;
            break;

        case SQL_C_WCHAR:
            mbstowcs((wchar_t *)target_ptr, value, target_len / sizeof(wchar_t) - 1);
            *str_len = size - 1;
            if(size > target_len)
                resp = CONV_TRUNCATED;
            break;
        default:
            return CONV_IMPOSSIBLE;
    }

    return resp;
}

/*
   Binds long long int value.
 */
conv_resp cdb2_bind_int(const char *name, LL val, void **buf, cdb2_hndl_tp *sqlh)
{
    if((*buf = malloc(sizeof(LL))) == NULL)
        return CONV_MEM_FAIL;

    *(LL *)(*buf) = val;

    /* For interger-to-integer, or integer-to-characters, we use CDB2_INTEGER type. */
    if(cdb2_bind_param(sqlh, name, CDB2_INTEGER, *buf, sizeof(LL)))
        return CONV_INTERNAL_ERR;
    return CONV_YEAH;
}

/*
   Binds real value.
 */
conv_resp cdb2_bind_real(const char *name, double val, void **buf, cdb2_hndl_tp *sqlh)
{
    if((*buf = malloc(sizeof(double))) == NULL)
        return CONV_MEM_FAIL;

    *(double *)(*buf) = val;

    /* For interger-to-integer, or integer-to-characters, we use CDB2_INTEGER type. */
    if(cdb2_bind_param(sqlh, name, CDB2_REAL, *buf, sizeof(double)))
        return CONV_INTERNAL_ERR;
    return CONV_YEAH;
}

/* Converts an integral value to an interval. */
#define INT_TO_INTERVALYM(field)                                                                    \
conv_resp  int_to_interval_ ## field (                                                              \
    const char *name, LL val, void **buf,                                                           \
    bool is_u, cdb2_hndl_tp *sqlh)                                                                  \
{                                                                                                   \
    cdb2_client_intv_ym_t *intv_ym;                                                                 \
    if((intv_ym = my_calloc(cdb2_client_intv_ym_t, 1)) == NULL)                                     \
        return CONV_MEM_FAIL;                                                                       \
                                                                                                    \
    if(is_u) { intv_ym->sign = 1; intv_ym->field = (SQLUINTEGER)val; }                              \
    else {                                                                                          \
        intv_ym->sign = val < 0 ? -1 : 1;                                                           \
        intv_ym->field = (SQLUINTEGER)(val < 0 ? -val : val);                                       \
    }                                                                                               \
    *buf = (void *)intv_ym;                                                                         \
    if(cdb2_bind_param(sqlh, name, CDB2_INTERVALYM, *buf, sizeof(cdb2_client_intv_ym_t)))           \
        return CONV_INTERNAL_ERR;                                                                   \
    return CONV_YEAH;                                                                               \
}                                                                                                   \
extern int no_such_variable

INT_TO_INTERVALYM(years);
INT_TO_INTERVALYM(months);

#define INT_TO_INTERVALDS(field)                                                                    \
conv_resp  int_to_interval_ ## field (                                                              \
    const char *name, LL val, void **buf,                                                           \
    bool is_u, cdb2_hndl_tp *sqlh)                                                                  \
{                                                                                                   \
    cdb2_client_intv_ds_t *intv_ds;                                                                 \
    if((intv_ds = my_calloc(cdb2_client_intv_ds_t, 1)) == NULL)                                     \
        return CONV_MEM_FAIL;                                                                       \
                                                                                                    \
    if(is_u) { intv_ds->sign = 1; intv_ds->field = (SQLUINTEGER)val; }                              \
    else {                                                                                          \
        intv_ds->sign = val < 0 ? -1 : 1;                                                           \
        intv_ds->field = (SQLUINTEGER)(val < 0 ? -val : val);                                       \
    }                                                                                               \
    *buf = (void *)intv_ds;                                                                         \
    if(cdb2_bind_param(sqlh, name, CDB2_INTERVALDS, *buf, sizeof(cdb2_client_intv_ds_t)))           \
        return CONV_INTERNAL_ERR;                                                                   \
    return CONV_YEAH;                                                                               \
}                                                                                                   \
extern int no_such_variable

INT_TO_INTERVALDS(days);
INT_TO_INTERVALDS(hours);
INT_TO_INTERVALDS(mins);
INT_TO_INTERVALDS(sec);

/* Converts a real number to an interval. */
#define REAL_TO_INTERVALYM(field)                                                                   \
static conv_resp  real_to_interval_ ## field (                                                      \
    const char *name, double val, void **buf, cdb2_hndl_tp *sqlh)                                   \
{                                                                                                   \
    cdb2_client_intv_ym_t *intv_ym;                                                                 \
    if((intv_ym = my_calloc(cdb2_client_intv_ym_t, 1)) == NULL)                                     \
        return CONV_MEM_FAIL;                                                                       \
                                                                                                    \
    intv_ym->sign = val < 0 ? -1 : 1;                                                               \
    intv_ym->field = (SQLUINTEGER)(val < 0 ? -val : val);                                           \
    *buf = (void *)intv_ym;                                                                         \
    if(cdb2_bind_param(sqlh, name, CDB2_INTERVALYM, *buf, sizeof(cdb2_client_intv_ym_t)))           \
        return CONV_INTERNAL_ERR;                                                                   \
    return CONV_YEAH;                                                                               \
}                                                                                                   \
extern int no_such_variable

REAL_TO_INTERVALYM(years);
REAL_TO_INTERVALYM(months);

#define REAL_TO_INTERVALDS(field)                                                                   \
static conv_resp  real_to_interval_ ## field (                                                      \
    const char *name, double val, void **buf, cdb2_hndl_tp *sqlh)                                   \
{                                                                                                   \
    cdb2_client_intv_ds_t *intv_ds;                                                                 \
    if((intv_ds = my_calloc(cdb2_client_intv_ds_t, 1)) == NULL)                                     \
        return CONV_MEM_FAIL;                                                                       \
                                                                                                    \
    intv_ds->sign = val < 0 ? -1 : 1;                                                               \
    intv_ds->field = (SQLUINTEGER)(val < 0 ? -val : val);                                           \
    *buf = (void *)intv_ds;                                                                         \
    if(cdb2_bind_param(sqlh, name, CDB2_INTERVALDS, *buf, sizeof(cdb2_client_intv_ds_t)))           \
        return CONV_INTERNAL_ERR;                                                                   \
    return CONV_YEAH;                                                                               \
}                                                                                                   \
extern int no_such_variable

REAL_TO_INTERVALDS(days);
REAL_TO_INTERVALDS(hours);
REAL_TO_INTERVALDS(mins);
REAL_TO_INTERVALDS(sec);

/**
 * Convert an integer from C type to CDB2 type and bind the parameter using CDB2 API.
 * See http://msdn.microsoft.com/en-us/library/ms714147(v=vs.85).aspx for details.
 *
 * @return
 *  CONV_MEM_FAIL           -   memory allocation failure.
 *  CONV_OOPS               -   what the hell?
 *  CONV_UNSUPPORTED_C_TYPE -   type unsupported.
 *  CONV_IMPOSSIBLE         -   conversion impossible.
 *  CONV_INTERNAL_ERR       -   cdb2 api reports an error.
 *  CONV_YEAH               -   yeah!
 */
conv_resp convert_and_bind_int(cdb2_hndl_tp *sqlh, struct param *param)
{
    bool is_u;
    LL val; 
    const void *buf = param->buf;
    char *name = param->name + 1;

    __debug("Try to bind an integral number.");

    if(!param->buf) { /* A NULL value */
        if(cdb2_bind_param(sqlh, name, CDB2_INTEGER, NULL, 0))
            return CONV_INTERNAL_ERR;
        return CONV_YEAH;
    }

    switch(param->c_type) {
        case SQL_C_STINYINT:
        case SQL_C_TINYINT:
            val = *(SQLSCHAR *) buf;
            is_u = 0;
            break;

        case SQL_C_SLONG:
        case SQL_C_LONG:
            val = *(SQLINTEGER *)buf;
            is_u = 0;
            break;

        case SQL_C_SSHORT:
        case SQL_C_SHORT:
            val = *(SQLSMALLINT *)buf;
            is_u = 0;
            break;

        case SQL_C_SBIGINT:
        case SQL_C_DEFAULT:
            val = *(SQLBIGINT *)buf;
            is_u = 0;
            break;

        case SQL_C_BIT:
        case SQL_C_UTINYINT:
            val = *(SQLCHAR *) buf;
            is_u = 1;
            break;

        case SQL_C_ULONG:
            val = *(SQLUINTEGER *)buf;
            is_u = 1;
            break;

        case SQL_C_USHORT:
            val = *(SQLUSMALLINT *)buf;
            is_u = 1;
            break;

        case SQL_C_UBIGINT:
            val = *(SQLUBIGINT *)buf;
            is_u = 1;
            break;

        default:    /* Returns a flag and let other convertors take it over. */
            __debug("Not a valid integral type.");
            return CONV_UNSUPPORTED_C_TYPE;
    }

    free(param->internal_buffer);
    param->internal_buffer = NULL;

    switch(param->sql_type) {

        case SQL_CHAR:
        case SQL_VARCHAR:
        case SQL_LONGVARCHAR:
        case SQL_WCHAR:
        case SQL_WVARCHAR:
        case SQL_WLONGVARCHAR:
            /* I am not sure if comdb2 has wchar type. */

        case SQL_REAL:
        case SQL_FLOAT:
        case SQL_DOUBLE:
            /* Handle precision. */
        case SQL_DECIMAL:
        case SQL_NUMERIC:
            /* DECIMAL and NUMERIC also need scale. However for an integral type, there's no scale. */
            if((param->internal_buffer = malloc(param->precision + 1)) == NULL)
                return CONV_MEM_FAIL;

            is_u ? snprintf(param->internal_buffer, param->precision + 1, "%llu", val) 
                : snprintf(param->internal_buffer, param->precision + 1, "%lld", val);

            if(cdb2_bind_param(sqlh, name, CDB2_CSTRING, param->internal_buffer, (int)strlen(param->internal_buffer)))
                return CONV_INTERNAL_ERR;
            break;

        case SQL_BIT:
            /* SQL_BIT is a special case: the source must be 0 or 1. */
            if(val != 0 && val != 1)
                return CONV_IMPOSSIBLE;

        case SQL_TINYINT:
        case SQL_SMALLINT:
        case SQL_INTEGER:
        case SQL_BIGINT:
            return cdb2_bind_int(name, val, &param->internal_buffer, sqlh);

            /* Next, deal with interval types. */
        case SQL_INTERVAL_YEAR:     
            return int_to_interval_years(name, val, &param->internal_buffer, is_u, sqlh);

        case SQL_INTERVAL_MONTH:    
            return int_to_interval_months(name, val, &param->internal_buffer, is_u, sqlh);

        case SQL_INTERVAL_DAY:
            return int_to_interval_days(name, val, &param->internal_buffer, is_u, sqlh);

        case SQL_INTERVAL_HOUR:
            return int_to_interval_hours(name, val, &param->internal_buffer, is_u, sqlh);

        case SQL_INTERVAL_MINUTE:
            return int_to_interval_mins(name, val, &param->internal_buffer, is_u, sqlh);

        case SQL_INTERVAL_SECOND:
            return int_to_interval_sec(name, val, &param->internal_buffer, is_u, sqlh);

        default:
            return CONV_IMPOSSIBLE;
    }

    return CONV_YEAH;
}

/**
 * Convert a real parameter from C type to CDB2 type and bind the parameter using CDB2 API.
 * See http://msdn.microsoft.com/en-us/library/ms714147(v=vs.85).aspx for details.
 *
 * @return
 *  CONV_MEM_FAIL           -   memory allocation failure.
 *  CONV_OOPS               -   what the hell?
 *  CONV_UNSUPPORTED_C_TYPE -   type unsupported.
 *  CONV_IMPOSSIBLE         -   conversion impossible.
 *  CONV_INTERNAL_ERR       -   cdb2 api reports an error.
 *  CONV_YEAH               -   yeah!
 */
conv_resp convert_and_bind_real(cdb2_hndl_tp *sqlh, struct param *param)
{
    double val;
    char *name = param->name + 1;

    __debug("Try to bind a real number.");

    if(!param->buf) { /* A NULL value */
        if(cdb2_bind_param(sqlh, name, CDB2_REAL, NULL, 0))
            return CONV_INTERNAL_ERR;
        return CONV_YEAH;
    }

        /* float and double. */
    switch(param->c_type) {
        case SQL_C_FLOAT:
            val = *((float *)param->buf);
            break;

        case SQL_C_DOUBLE:
        case SQL_C_DEFAULT:
            val = *((double *)param->buf);
            break;

        default:    /* Returns a flag and let other convertors take it over. */
            __debug("Not a valid real type.");
            return CONV_UNSUPPORTED_C_TYPE;
    }

    free(param->internal_buffer);
    param->internal_buffer = NULL;

    switch(param->sql_type) {
        case SQL_CHAR:
        case SQL_VARCHAR:
        case SQL_LONGVARCHAR:
        case SQL_WCHAR:
        case SQL_WVARCHAR:
        case SQL_WLONGVARCHAR:

        case SQL_REAL:
        case SQL_FLOAT:
        case SQL_DOUBLE:
        case SQL_DECIMAL:
        case SQL_NUMERIC:
            if((param->internal_buffer = malloc(param->precision + 1)) == NULL)
                return CONV_MEM_FAIL;

            snprintf(param->internal_buffer, param->precision + 1, "%.*f", param->scale, val); 
            if(cdb2_bind_param(sqlh, name, CDB2_CSTRING, param->internal_buffer, (int)strlen(param->internal_buffer)))
                return CONV_INTERNAL_ERR;
            return CONV_YEAH;
    }

    /* @val now is guaranteed to be a valid value. */

    switch(param->sql_type) {
        /* Convert a real number to an integral number. But... Who would do this? */
        case SQL_BIT:
            /* SQL_BIT is a special case: the source must be 0 or 1. */
            if(val < 0 || val >= 2)
                return CONV_IMPOSSIBLE;
        case SQL_TINYINT:
        case SQL_SMALLINT:
        case SQL_INTEGER:
        case SQL_BIGINT:
            return cdb2_bind_int(name, (LL)val, &param->internal_buffer, sqlh);

        case SQL_INTERVAL_YEAR:     
            return real_to_interval_years(name, val, &param->internal_buffer, sqlh);

        case SQL_INTERVAL_MONTH:    
            return real_to_interval_months(name, val, &param->internal_buffer, sqlh);

        case SQL_INTERVAL_DAY:
            return real_to_interval_days(name, val, &param->internal_buffer, sqlh);

        case SQL_INTERVAL_HOUR:
            return real_to_interval_hours(name, val, &param->internal_buffer, sqlh);

        case SQL_INTERVAL_MINUTE:
            return real_to_interval_mins(name, val, &param->internal_buffer, sqlh);

        case SQL_INTERVAL_SECOND:
            return real_to_interval_sec(name, val, &param->internal_buffer, sqlh);

        default:
            return CONV_IMPOSSIBLE;
    }
}

/**
 * Convert a cstring from C type to CDB2 type and bind the parameter using CDB2 API.
 * See http://msdn.microsoft.com/en-us/library/ms714147(v=vs.85).aspx for details.
 *
 * @return
 *  CONV_MEM_FAIL           -   memory allocation failure.
 *  CONV_OOPS               -   what the hell?
 *  CONV_UNSUPPORTED_C_TYPE -   type unsupported.
 *  CONV_IMPOSSIBLE         -   conversion impossible.
 *  CONV_INTERNAL_ERR       -   cdb2 api reports an error.
 *  CONV_YEAH               -   yeah!
 */
conv_resp convert_and_bind_cstring(cdb2_hndl_tp *sqlh, struct param *param)
{
    const wchar_t *wcs;
    const void *bound_buffer;
    SQLULEN len, width;
    double dval;
    LL lval;
    conv_resp resp = CONV_YEAH;
    char *name = param->name + 1;

    __debug("Try to bind a string.");

    if(!param->buf) { /* A NULL value */
        if(cdb2_bind_param(sqlh, name, CDB2_CSTRING, NULL, 0))
            return CONV_INTERNAL_ERR;
        return CONV_YEAH;
    }

    switch(param->c_type) {
        case SQL_C_CHAR:
        case SQL_C_DEFAULT:
            len = strlen((char *)param->buf);
            break;
        case SQL_C_WCHAR:
            /* Convert wchar_t to char. */
            wcs = (wchar_t *)param->buf;
            len = wcslen((wchar_t *)param->buf) * sizeof(wchar_t);

            free(param->internal_buffer);
            if((param->internal_buffer = malloc(len + 1)) == NULL)
                return CONV_MEM_FAIL;

            wcstombs((char *)param->internal_buffer, wcs, len);
            break;

        default:    /* Returns a flag and let other convertors take it over. */
            __debug("Not a valid cstring type.");
            return CONV_UNSUPPORTED_C_TYPE;
    }

    bound_buffer = param->internal_buffer ? param->internal_buffer : param->buf;

    switch(param->sql_type) {
        case SQL_CHAR:
        case SQL_VARCHAR:
        case SQL_LONGVARCHAR:
        case SQL_WCHAR:
        case SQL_WVARCHAR:
        case SQL_WLONGVARCHAR:

            /* 1. Check deferred input. If the length is specified in the deferred input, use it.
               2. No deferred input, we use @BufferLength. 
               3. Make sure the length is less than precision. */

            /* The order cannot be altered. */
            if(param->str_len) { /* deferred input */
                width = *param->str_len;
                if(width < 0) {
                    if(width == SQL_NTS)
                        width = len;
                    else 
                        return CONV_INVALID_BUFLEN;
                }
            } else
                width = param->buflen;

            width = MIN(width, len);

            if(width >= param->precision) 
                /* #1 Unlike binary data(see #1 below), a '\0' is needed to terminate the string. */
                resp = CONV_BUF_OVERFLOW;

            if(cdb2_bind_param(sqlh, name, CDB2_CSTRING, bound_buffer, (int)width))
                resp = CONV_INTERNAL_ERR;
            break;

        case SQL_BINARY:
        case SQL_VARBINARY:
        case SQL_LONGVARBINARY:

            if(param->str_len) { /* deferred input */
                width = *param->str_len;
                if(width < 0) {
                    if(width == SQL_NTS)
                        width = len;
                    else
                        return CONV_INVALID_BUFLEN;
                }
            } else
                width = param->buflen;

            width = MIN(width, len);

            if(width > param->precision) /* #1 */
                resp = CONV_BUF_OVERFLOW;

            if(cdb2_bind_param(sqlh, name, CDB2_BLOB, bound_buffer, (int)width))
                resp = CONV_INTERNAL_ERR;
            break;

            /* For the next 3 types, let COMDB2 handle them (They must be valid literals). 
               Just send the orignial string representation. */
        case SQL_TYPE_DATE:
        case SQL_TYPE_TIME:
        case SQL_TYPE_TIMESTAMP:

            if(cdb2_bind_param(sqlh, name, CDB2_CSTRING, bound_buffer, (int)len))
                resp =  CONV_INTERNAL_ERR;
            break;

        case SQL_BIT:
            if(strcmp(bound_buffer, "0") || strcmp(bound_buffer, "1"))
                resp = CONV_IMPOSSIBLE;

        case SQL_REAL:
        case SQL_FLOAT:
        case SQL_DOUBLE:
        case SQL_DECIMAL:
        case SQL_NUMERIC:
            dval = atof(bound_buffer);

            /* If the param is not wchar_t *, no harm to free a NULL pointer. 
               If the param is wchar_t *, internal_buffer is gone and new space is allocated for it. */
            free(param->internal_buffer);
            if((param->internal_buffer = malloc(param->precision + 1)) == NULL)
                return CONV_MEM_FAIL;

            snprintf(param->internal_buffer, param->precision + 1, "%.*f", param->scale, dval); 
            if(cdb2_bind_param(sqlh, name, CDB2_CSTRING, param->internal_buffer, (int)strlen(param->internal_buffer)))
                resp = CONV_INTERNAL_ERR;
            break;

        case SQL_TINYINT:
        case SQL_SMALLINT:
        case SQL_INTEGER:
        case SQL_BIGINT:
            lval = atoll(bound_buffer);
            free(param->internal_buffer);
            resp = cdb2_bind_int(name, lval, &param->internal_buffer, sqlh);

        default:
            resp = CONV_IMPOSSIBLE;
    }

    return resp;
}

/**
 * Convert a large object to CDB2 type and bind the parameter using CDB2 API.
 * See http://msdn.microsoft.com/en-us/library/ms714147(v=vs.85).aspx for details.
 *
 * @return
 *  CONV_MEM_FAIL           -   memory allocation failure.
 *  CONV_OOPS               -   what the hell?
 *  CONV_UNSUPPORTED_C_TYPE -   type unsupported.
 *  CONV_IMPOSSIBLE         -   conversion impossible.
 *  CONV_INTERNAL_ERR       -   cdb2 api reports an error.
 *  CONV_YEAH               -   yeah!
 */
conv_resp convert_and_bind_blob(cdb2_hndl_tp *sqlh, struct param *param)
{
    SQLULEN width;
    conv_resp resp = CONV_YEAH;
    char *name = param->name + 1;

    __debug("Try to bind a large object.");

    if(!param->buf) { /* A NULL value */
        if(cdb2_bind_param(sqlh, name, CDB2_BLOB, NULL, 0))
            return CONV_INTERNAL_ERR;
        return CONV_YEAH;
    }

    if(param->c_type != SQL_C_BINARY && param->c_type != SQL_C_DEFAULT) {
        __debug("Not a valid blob type.");
        return CONV_UNSUPPORTED_C_TYPE;
    }

    if(param->str_len) { /* deferred input */
        width = *param->str_len;
        if(width < 0) 
            return CONV_INVALID_BUFLEN;
    } else /* a valid param->buflen is guaranteed by driver manager. */
        width = param->buflen;

    if(width > param->precision)
        resp = CONV_BUF_OVERFLOW;

    if(cdb2_bind_param(sqlh, name, CDB2_BLOB, param->buf, (int)width))
        resp = CONV_INTERNAL_ERR;

    return resp;
}

/**
 * Convert a datetime to CDB2 type and bind the parameter using CDB2 API.
 * See http://msdn.microsoft.com/en-us/library/ms714147(v=vs.85).aspx for details.
 *
 * @return
 *  CONV_MEM_FAIL           -   memory allocation failure.
 *  CONV_OOPS               -   what the hell?
 *  CONV_UNSUPPORTED_C_TYPE -   type unsupported.
 *  CONV_IMPOSSIBLE         -   conversion impossible.
 *  CONV_INTERNAL_ERR       -   cdb2 api reports an error.
 *  CONV_YEAH               -   yeah!
 */
conv_resp convert_and_bind_datetime(cdb2_hndl_tp *sqlh, struct param *param)
{
    conv_resp resp = CONV_YEAH;
    cdb2_client_datetime_t *datetime;
    DATE_STRUCT *date_struct;
    TIME_STRUCT *time_struct;
    TIMESTAMP_STRUCT *ts_struct;
    char *name = param->name + 1;

    __debug("Try to bind datetime.");

    if(!param->buf) { /* A NULL value */
        if(cdb2_bind_param(sqlh, name, CDB2_DATETIME, NULL, 0))
            return CONV_INTERNAL_ERR;
        return CONV_YEAH;
    }

    switch(param->c_type) {
        case SQL_C_TYPE_DATE:
            if((datetime = my_calloc(cdb2_client_datetime_t, 1)) == NULL)
                return CONV_MEM_FAIL;

            date_struct = (DATE_STRUCT *)param->buf;
            datetime->tm.tm_year = date_struct->year - 1900;
            datetime->tm.tm_mon  = date_struct->month - 1;
            datetime->tm.tm_mday = date_struct->day;

            break;

        case SQL_C_TYPE_TIME:
            if((datetime = my_calloc(cdb2_client_datetime_t, 1)) == NULL)
                return CONV_MEM_FAIL;

            time_struct = (TIME_STRUCT *)param->buf;
            datetime->tm.tm_hour = time_struct->hour;
            datetime->tm.tm_min  = time_struct->minute;
            datetime->tm.tm_sec  = time_struct->second;

            break;

        case SQL_C_TYPE_TIMESTAMP:
        case SQL_C_DEFAULT:
            if((datetime = my_calloc(cdb2_client_datetime_t, 1)) == NULL)
                return CONV_MEM_FAIL;

            ts_struct = (TIMESTAMP_STRUCT *)param->buf;
            datetime->tm.tm_year = ts_struct->year - 1900;
            datetime->tm.tm_mon  = ts_struct->month - 1;
            datetime->tm.tm_mday = ts_struct->day;
            datetime->tm.tm_hour = ts_struct->hour;
            datetime->tm.tm_min  = ts_struct->minute;
            datetime->tm.tm_sec  = ts_struct->second;

            /* @fraction is nanoseconds. 
               See http://msdn.microsoft.com/en-us/library/ms714556(v=vs.85).aspx for details. */
            datetime->msec       = (int)(ts_struct->fraction / 1000000);
			break;
        
        default:
            __debug("Not a valid datetime type.");
            return CONV_UNSUPPORTED_C_TYPE;
    }

    /* Timezone info will be populated by CDB2API. */
    param->internal_buffer = datetime;

    switch(param->sql_type) {
        case SQL_CHAR:
        case SQL_VARCHAR:
        case SQL_LONGVARCHAR:
        case SQL_WCHAR:
        case SQL_WVARCHAR:
        case SQL_WLONGVARCHAR:
        case SQL_TYPE_DATE:
        case SQL_TYPE_TIME:
        case SQL_TYPE_TIMESTAMP:
            if(cdb2_bind_param(sqlh, name, CDB2_DATETIME, param->internal_buffer, sizeof(cdb2_client_datetime_t)))
                resp = CONV_INTERNAL_ERR;
            break;

        default:
            resp = CONV_IMPOSSIBLE;
    }

    return resp;
}

/**
 * Convert a year-month interval to CDB2 type and bind the parameter using CDB2 API.
 * See http://msdn.microsoft.com/en-us/library/ms714147(v=vs.85).aspx for details.
 *
 * @return
 *  CONV_MEM_FAIL           -   memory allocation failure.
 *  CONV_OOPS               -   what the hell?
 *  CONV_UNSUPPORTED_C_TYPE -   type unsupported.
 *  CONV_IMPOSSIBLE         -   conversion impossible.
 *  CONV_INTERNAL_ERR       -   cdb2 api reports an error.
 *  CONV_YEAH               -   yeah!
 */
conv_resp convert_and_bind_intv_ym(cdb2_hndl_tp *sqlh, struct param *param)
{
    cdb2_client_intv_ym_t *intv_cdb2;
    SQL_INTERVAL_STRUCT *intv_odbc;
    LL year_or_mon;
    char *name = param->name + 1;

    __debug("Try to bind year-month interval.");

    if(!param->buf) { /* A NULL value */
        if(cdb2_bind_param(sqlh, name, CDB2_INTERVALYM, NULL, 0))
            return CONV_INTERNAL_ERR;
        return CONV_YEAH;
    }

    switch(param->c_type) {
        case SQL_C_INTERVAL_YEAR:
        case SQL_C_INTERVAL_MONTH:
        case SQL_C_INTERVAL_YEAR_TO_MONTH:
        case SQL_C_DEFAULT:
            intv_odbc = (SQL_INTERVAL_STRUCT *)param->buf;
            break;

        default:
            __debug("Not a valid year-month interval type.");
            return CONV_UNSUPPORTED_C_TYPE;
    }

    switch(param->sql_type) {
        case SQL_CHAR:
        case SQL_VARCHAR:
        case SQL_LONGVARCHAR:
        case SQL_WCHAR:
        case SQL_WVARCHAR:
        case SQL_WLONGVARCHAR:

        case SQL_INTERVAL_YEAR:
        case SQL_INTERVAL_MONTH:
            /* TODO see #2 & #3 */

        case SQL_INTERVAL_YEAR_TO_MONTH:

            /* FIXME see #3 */
            if(intv_odbc->interval_type != SQL_IS_YEAR_TO_MONTH)
                return CONV_IMPOSSIBLE;

            if((intv_cdb2 = my_malloc(cdb2_client_intv_ym_t, 1)) == NULL)
                return CONV_MEM_FAIL;

            intv_cdb2->sign   = intv_odbc->interval_sign;
            intv_cdb2->years  = intv_odbc->intval.year_month.year;
            intv_cdb2->months = intv_odbc->intval.year_month.month;

            param->internal_buffer = (void *)intv_cdb2;

            if(cdb2_bind_param(sqlh, name, CDB2_INTERVALYM, param->internal_buffer, sizeof(cdb2_client_intv_ym_t)))
                return CONV_INTERNAL_ERR;

        case SQL_TINYINT:
        case SQL_SMALLINT:
        case SQL_INTEGER:
        case SQL_BIGINT:
        case SQL_NUMERIC:
        case SQL_DECIMAL:

            if(param->c_type == SQL_C_INTERVAL_YEAR) {
                year_or_mon = intv_odbc->intval.year_month.year * intv_odbc->interval_sign;
                return cdb2_bind_int(name, year_or_mon, &param->internal_buffer, sqlh);
            } else if(param->c_type == SQL_C_INTERVAL_MONTH) {
                year_or_mon = intv_odbc->intval.year_month.month * intv_odbc->interval_sign;
                return cdb2_bind_int(name, year_or_mon, &param->internal_buffer, sqlh);
            } else
                return CONV_IMPOSSIBLE;


            /* #2 To be consistent with #3, the following code is commented out. */
#if 0
        case SQL_INTERVAL_YEAR:
            if(param->c_type == SQL_C_INTERVAL_MONTH)
                /* Months to years, divided month by 12. */
                year_or_mon = intv_odbc->intval.year_month.month * intv_odbc->interval_sign / 12;
            else 
                year_or_mon = intv_odbc->intval.year_month.year * intv_odbc->interval_sign;

            return int_to_interval_years(name, year_or_mon, &param->internal_buffer, 0, sqlh);

        case SQL_INTERVAL_MONTH:
            if(param->c_type == SQL_C_INTERVAL_YEAR)
                /* Years to months, multiply year by 12 */
                year_or_mon = intv_odbc->intval.year_month.year * intv_odbc->interval_sign * 12;

            else if(param->c_type == SQL_C_INTERVAL_YEAR_TO_MONTH)
                /* Years-and-months to months, calculate months as (year * 12 + month). */
                year_or_mon = (intv_odbc->intval.year_month.month + intv_odbc->intval.year_month.year * 12 ) 
                    * intv_odbc->interval_sign;

            else if(param->c_type == SQL_C_INTERVAL_MONTH)
                year_or_mon = intv_odbc->intval.year_month.month * intv_odbc->interval_sign;
            
            return int_to_interval_months(name, year_or_mon, &param->internal_buffer, 0, sqlh);
#endif          

        default:
            return CONV_IMPOSSIBLE;
    }
}

/**
 * Convert a day-second interval to CDB2 type and bind the parameter using CDB2 API.
 * See http://msdn.microsoft.com/en-us/library/ms714147(v=vs.85).aspx for details.
 *
 * @return
 *  CONV_MEM_FAIL           -   memory allocation failure.
 *  CONV_OOPS               -   what the hell?
 *  CONV_UNSUPPORTED_C_TYPE -   type unsupported.
 *  CONV_IMPOSSIBLE         -   conversion impossible.
 *  CONV_INTERNAL_ERR       -   cdb2 api reports an error.
 *  CONV_YEAH               -   yeah!
 */
conv_resp convert_and_bind_intv_ds(cdb2_hndl_tp *sqlh, struct param *param)
{
    cdb2_client_intv_ds_t *intv_cdb2;
    SQL_INTERVAL_STRUCT *intv_odbc;
    LL dhmsf; /* day or hour or min or sec or frac */
    char *name = param->name + 1;

    __debug("Try to bind day-time interval.");

    if(!param->buf) { /* A NULL value */
        if(cdb2_bind_param(sqlh, name, CDB2_INTERVALDS, NULL, 0))
            return CONV_INTERNAL_ERR;
        return CONV_YEAH;
    }

    switch(param->c_type) {
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
        case SQL_C_DEFAULT:
            intv_odbc = (SQL_INTERVAL_STRUCT *)param->buf;
            break;

        default:
            __debug("Not a valid day-second interval type.");
            return CONV_UNSUPPORTED_C_TYPE;
    }

    switch(param->sql_type) {
        case SQL_CHAR:
        case SQL_VARCHAR:
        case SQL_LONGVARCHAR:
        case SQL_WCHAR:
        case SQL_WVARCHAR:
        case SQL_WLONGVARCHAR:
            /* #3 TODO Support among-interval-types conversion (like what I did in bind_intv_ym).
             For example, a 3-day interval gives a 72-hours interval, a 4-hour interval gives a 240-minutes interval. 
             Because 1. a day-time interval has 10 combinations (versus 3 combination of a year-mon interval). 
             2. intv_ds has 5 fields (versus 2 fields of intv_ym, conversion rules for day-time intervals are much much more 'labour working' 
             than year-month intervals. */
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

            /* FIXME see #3 */
            if(intv_odbc->interval_type != SQL_IS_DAY_TO_SECOND)
                return CONV_IMPOSSIBLE;

            if((intv_cdb2 = my_malloc(cdb2_client_intv_ds_t, 1)) == NULL)
                return CONV_MEM_FAIL;

            intv_cdb2->sign   = intv_odbc->interval_sign;
            intv_cdb2->days   = intv_odbc->intval.day_second.day;
            intv_cdb2->hours  = intv_odbc->intval.day_second.hour;
            intv_cdb2->mins   = intv_odbc->intval.day_second.minute;
            intv_cdb2->sec    = intv_odbc->intval.day_second.second;
            intv_cdb2->msec   = (int)(intv_odbc->intval.day_second.fraction / 1000000);

            param->internal_buffer = (void *)intv_cdb2;

            if(cdb2_bind_param(sqlh, name, CDB2_INTERVALDS, param->internal_buffer, sizeof(cdb2_client_intv_ds_t)))
                return CONV_INTERNAL_ERR;

        case SQL_TINYINT:
        case SQL_SMALLINT:
        case SQL_INTEGER:
        case SQL_BIGINT:
        case SQL_NUMERIC:
        case SQL_DECIMAL:

            if(param->c_type == SQL_C_INTERVAL_DAY) {
                dhmsf = intv_odbc->intval.day_second.day * intv_odbc->interval_sign;
                return cdb2_bind_int(name, dhmsf, &param->internal_buffer, sqlh);
            } else if(param->c_type == SQL_C_INTERVAL_HOUR) {
                dhmsf = intv_odbc->intval.day_second.hour * intv_odbc->interval_sign;
                return cdb2_bind_int(name, dhmsf, &param->internal_buffer, sqlh);
            } else if(param->c_type == SQL_C_INTERVAL_MINUTE) {
                dhmsf = intv_odbc->intval.day_second.minute * intv_odbc->interval_sign;
                return cdb2_bind_int(name, dhmsf, &param->internal_buffer, sqlh);
            } else if(param->c_type == SQL_C_INTERVAL_SECOND) {
                dhmsf = intv_odbc->intval.day_second.second * intv_odbc->interval_sign;
                return cdb2_bind_int(name, dhmsf, &param->internal_buffer, sqlh);
            } else
                return CONV_IMPOSSIBLE;

        default:
            return CONV_IMPOSSIBLE;
    }
}
