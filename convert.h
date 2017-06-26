#ifndef _CONVERT_H_
#define _CONVERT_H_

#include "driver.h"

/* Return types of conversion functions. */
typedef enum {
      CONV_UNKNOWN_CDB2_TYPE = -2
    , CONV_UNSUPPORTED_C_TYPE = -1                   
    , CONV_YEAH = 0
    , CONV_BUF_OVERFLOW 
    , CONV_INVALID_BUFLEN 
    , CONV_TRUNCATED 
    , CONV_TRUNCATED_WHOLE 
    , CONV_UNKNOWN_C_TYPE                    
    , CONV_IMPOSSIBLE                    
    , CONV_MEM_FAIL                  
    , CONV_NULL
    , CONV_INTERNAL_ERR
    , CONV_OOPS
} conv_resp;

/* ============ Convert CDB2API types to native types. ============= */

#define CDB2_CONV(type) convert_cdb2 ## type
#define CDB2_CONV_PROTO(type) conv_resp CDB2_CONV(type) (const void *, int, SQLSMALLINT, SQLPOINTER, SQLLEN, SQLLEN *)

/* Convert cdb2 types to c data types. 
   Internal usage to convert (void *) values retrieved using CDB2API. */
CDB2_CONV_PROTO(int);
CDB2_CONV_PROTO(real);
CDB2_CONV_PROTO(cstring);
CDB2_CONV_PROTO(blob);
CDB2_CONV_PROTO(datetime);
CDB2_CONV_PROTO(inym);
CDB2_CONV_PROTO(inds);

/* Function prototype of cdb2 convertors. */
typedef conv_resp (*cdb2_conv_func_t)(const void *,         /* data retrieved */
                                      int,                  /* length of @param1 */ 
                                      SQLSMALLINT,          /* c data type */
                                      SQLPOINTER,           /* buffer in which converted data will be returned */
                                      SQLLEN,               /* length of buffer */
                                      SQLLEN *              /* strlen_or_ind */);

/* This array maps cdb2 data type values to conversion functions. 
   The index may serve as a quick reference to its corresponding conversion function.
   So if.. elseif.. or switch/case is no longer needed. */
const static cdb2_conv_func_t CDB2_CONVS[] = {
    [CDB2_INTEGER]  =       CDB2_CONV(int)
    ,[CDB2_REAL]    =       CDB2_CONV(real)
    ,[CDB2_CSTRING] =       CDB2_CONV(cstring)
    ,[CDB2_BLOB]    =       CDB2_CONV(blob)
    ,[CDB2_DATETIME]    =   CDB2_CONV(datetime)
    ,[CDB2_INTERVALYM]  =   CDB2_CONV(inym)
    ,[CDB2_INTERVALDS]  =   CDB2_CONV(inds)
};
const static int NUM_CDB2_CONVS = ALEN(CDB2_CONVS);

/* ============ Convert and bind native types using CDB2API. ============= */

struct param;
#define CDB2_BIND(type) convert_and_bind_ ## type
#define CDB2_BIND_PROTO(type) conv_resp CDB2_BIND(type) (cdb2_hndl_tp *, struct param *)

CDB2_BIND_PROTO(int);
CDB2_BIND_PROTO(real);
CDB2_BIND_PROTO(cstring);
CDB2_BIND_PROTO(blob);
CDB2_BIND_PROTO(datetime);
CDB2_BIND_PROTO(intv_ym);
CDB2_BIND_PROTO(intv_ds);

typedef conv_resp (*cdb2_bind_func_t)(cdb2_hndl_tp *,           /* CDB2 handle */
                                      struct param *);          /* Parameter to be processed */
const static cdb2_bind_func_t CDB2_BINDS[] = {
      CDB2_BIND(int)
    , CDB2_BIND(real)
    , CDB2_BIND(cstring)
    , CDB2_BIND(blob)
    , CDB2_BIND(datetime)
    , CDB2_BIND(intv_ym)
    , CDB2_BIND(intv_ds)
};
const static int NUM_CDB2_BINDS = ALEN(CDB2_BINDS);

#endif /* _CONVERT_H_ */
