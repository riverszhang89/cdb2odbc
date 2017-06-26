#ifndef _UTIL_H_
#define _UTIL_H_

#include <stdlib.h>
#include <string.h>

#include <sql.h> /* We need this header for SQL_NTS stuff. */

/* Max and min. */
#define MAX(x, y) (x) ^ (((x) ^ (y)) & -((x) < (y)))
#define MIN(x, y) (y) ^ (((x) ^ (y)) & -((x) < (y)))

/* Return the number of elements in @array. */
#define ALEN(array) sizeof(array) / sizeof(array[0])

/**
 * Typed malloc.
 * @type    - type to be allocated.
 * @num     - how many?
 * @cast_to - type to be converted to.
 */
#define my_malloc_typed(type, num, cast_to) (cast_to) malloc(sizeof(type) * (num))

/**
 * malloc and casts to (@type *).
 * @type    - type to be allocated.
 * @num     - how many?
 */
#define my_malloc(type, num) malloc(sizeof(type) * (num))

/**
 * fill @ptr with 0.
 * @ptr     - where
 * @type    - type
 * @num     - how many
 */
#define zerofill(ptr, type, num) memset(ptr, 0, sizeof(type) * (num)) 

/**
 * Typed calloc.
 * @type    - type to be allocated.
 * @num     - how many?
 * @cast_to - type to be converted to.
 */
#define my_calloc_typed(type, num, cast_to) (cast_to) calloc(num, sizeof(type))

/**
 * calloc and casts to (@type *).
 * @type    - type to be allocated.
 * @num     - how many?
 */
#define my_calloc(type, num) calloc(num, sizeof(type))

/**
 * strncpy. It appends a '\0' to dst after copy. It should be used when the length of @dst is given.
 * @dst - destination,
 * @src - source,
 * @len - length of @dst 
 */
#define my_strncpy_out(dst, src, len) do {                              \
    if(len > 0) { strncpy(dst, src, len - 1); (dst)[len - 1] = '\0'; }  \
}while(0)

/**
 * Function version of my_strncpy_out.
 */
extern char *my_strncpy_out_fn(char *, const char *, int);

/**
 * strncpy. It appends a '\0' to dst after copy. It should be used when the length of @src is given.
 * @dst - destination,
 * @len_dst - length of @dst
 * @src - source,
 * @len_src - STRING length of @src 
 */
#define my_strncpy_in(dst, len_dst, src, len_src) do {                      \
    int len_src_in = (int)((len_src == SQL_NTS) ? strlen(src) : len_src),   \
        min_len    = MIN(len_dst - 1, len_src_in);                          \
    if(min_len > 0) { strncpy(dst, src, min_len); (dst)[min_len] = '\0';}   \
}while(0)

/**
 * Function version of macro my_strncpy_in.
 */
extern char *my_strncpy_in_fn(char *, int, const char *, int);

/* Convert @src to @type, return the result in the address pointed to by @dst, and set @len */
#define CAST_FIXED_LENGTH_TYPE(dst, src, type, len) do {    \
    *((type *)(dst)) = (type)(src);                         \
    len = (int)sizeof(type);                                \
} while(0)

#define SET_SQLREAL(dst, src, len)              CAST_FIXED_LENGTH_TYPE(dst, src, SQLREAL, len)
#define SET_SQLDOUBLE(dst, src, len)            CAST_FIXED_LENGTH_TYPE(dst, src, SQLDOUBLE, len)
#define SET_SQLFLOAT(dst, src, len)             CAST_FIXED_LENGTH_TYPE(dst, src, SQLFLOAT, len)
#define SET_SQLUINT(dst, src, len)              CAST_FIXED_LENGTH_TYPE(dst, src, SQLUINTEGER, len)
#define SET_SQLINT(dst, src, len)               CAST_FIXED_LENGTH_TYPE(dst, src, SQLINTEGER, len)
#define SET_SQLUSMALLINT(dst, src, len)         CAST_FIXED_LENGTH_TYPE(dst, src, SQLUSMALLINT, len)
#define SET_SQLSMALLINT(dst, src, len)          CAST_FIXED_LENGTH_TYPE(dst, src, SQLSMALLINT, len)
#define SET_SQLSCHAR(dst, src, len)             CAST_FIXED_LENGTH_TYPE(dst, src, SQLSCHAR, len)
#define SET_SQLCHAR(dst, src, len)              CAST_FIXED_LENGTH_TYPE(dst, src, SQLCHAR, len)
#define SET_SQLBIGINT(dst, src, len)            CAST_FIXED_LENGTH_TYPE(dst, src, SQLBIGINT, len)
#define SET_SQLUBIGINT(dst, src, len)           CAST_FIXED_LENGTH_TYPE(dst, src, SQLUBIGINT, len)
#define SET_SQLLEN(dst, src ,len)               CAST_FIXED_LENGTH_TYPE(dst, src, SQLLEN, len); 

/* Copy @src to @dst, @buflen characters at most, and set @len. */
#define SET_CSTRING(dst, src, buflen, len) do {     \
    my_strncpy_out_fn((char *)dst, src, buflen);    \
    len = (int)strlen(src);                         \
} while(0)

/**
 * Create a null-terminated string.
 * @param0 - source
 * @param1 - length of source (could be SQL_NTS)
 * @param2 - buffer (optional) 
 */
extern char *make_string(const char *, int, char *);

/**
 * Splits a string by a character.
 */
extern char *chrtok(char *, const char);

/**
 * Thread-safe version of chrtok.
 */
extern char *chrtok_r(char *, const char, char**);

#endif /* _UTIL_H_ */
