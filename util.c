#include "driver.h"
#include "util.h"

/**
 * Copy the first @len - 1 bytes of @src (will also terminate at '\0') to @dst. 
 * @dst will be null-terminated anyway.
 * Notice that @len refers to the length of @dst. So this method is usually used for returning information in application's buffer.
 * The address of @dst will be returned.
 */
char *my_strncpy_out_fn(char *dst, const char *src, int len)
{
    my_strncpy_out(dst, src, len);
    return dst;
}

/**
 * Copy the first @len bytes of @src (will also terminate at '\0') to @dst. 
 * @dst will be null-terminated anyway.
 * Notice that @len refers to the length of @src. So this method is usually used for copying application's buffer to driver-internal buffer.
 * The address of @dst will be returned.
 */
char *my_strncpy_in_fn(char *dst, int len_dst, const char *src, int len_src)
{
    my_strncpy_in(dst, len_dst, src, len_src);
    return dst;
}

/**
 * copy @s to @buf.
 * Notice: If any memory allocation failure, @buf will not be eliminated. 
 * The caller will decide if @buf should be discarded.
 *
 * @s - source
 * @len - length of @s (or SQL_NTS, in this case, @len is computed as strlen(@s))
 * @buf - destination. If @buf is null, allocate it and return the addr of the newly allocated memory.
 */
char *make_string(const char *s, int len, char *buf)
{
    char *str;

    if(!s) return NULL;
    if((len < 0 && len != SQL_NTS) ||
       (len == SQL_NTS && (len = (int)strlen(s)) <= 0))
		return NULL;

    ++len; /* an extra char for \0. */

    if(buf) {
        if((str = realloc(buf, strlen(buf) + len + 1)) == NULL)
			return NULL;
        /* append @s to @buf */

        strcat(str, s);
        return str;
    }

    if((str = my_malloc(char, len)) == NULL)
        return NULL;

    my_strncpy_out(str, s, len);    
    return str;
}

/**
 * Split a string by @delim.
 * The usage is the same as strtok. Except it accepts a char instead of a char *.
 */
char *chrtok(char *str, const char delim)
{
    static char *src = NULL;
    char *p, *ret = 0;

    if (str != NULL)
        src = str;

    if (src == NULL)
        return NULL;

    if ((p = strchr(src, delim)) != NULL) {
        *p  = 0;
        ret = src;
        src = ++p;
    }

    return ret;
}

/**
 * Thread-safe version of chrtok.
 * The usage is the same as strtok. Except it accepts a char instead of a char *.
 */
char *chrtok_r(char *str, const char delim, char **last)
{
    char *src = NULL;
    char *p, *ret = 0;

    if(str != NULL) src = str;
    else src = *last;

    if(src == NULL) return NULL;

    if ((p = strchr(src, delim)) != NULL) {
        *p  = 0;
        ret = src;
        *last = ++p;
    }

    return ret;
}
