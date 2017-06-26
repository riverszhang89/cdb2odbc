#include "driver.h"

#if defined(__IODBC__)
# include <iodbcinst.h>
#else
# include <odbcinst.h>
#endif

#ifndef INSTAPI
#define INSTAPI
#endif

BOOL INSTAPI ConfigDriver(HWND hwnd,
                          WORD fRequest,
                          LPCSTR lpszDriver,
                          LPCSTR lpszArgs,
                          LPSTR  lpszMsg,
                          WORD   cbMsgMax,
                          WORD  *pcbMsgOut)
{
	if (fRequest != ODBC_INSTALL_DRIVER)
		return FALSE;

	if (lpszDriver == NULL)
		return FALSE;

	if (!SQLWritePrivateProfileString(lpszDriver, "APILevel", "1", ODBCINST_INI))
		return FALSE;
	if (!SQLWritePrivateProfileString(lpszDriver, "ConnectFunctions", "YYN", ODBCINST_INI))
		return FALSE;
	if (!SQLWritePrivateProfileString(lpszDriver, "FileUsage", "0", ODBCINST_INI))
		return FALSE;
	if (!SQLWritePrivateProfileString(lpszDriver, "SQLLevel", "1", ODBCINST_INI))
		return FALSE;
	if (!SQLWritePrivateProfileString(lpszDriver, "DriverODBCVer", DRVODBCVER, ODBCINST_INI))
		return FALSE;
	return TRUE;
}

BOOL INSTAPI ConfigDSN(
     HWND     hwnd,
     WORD     fRequest,
     LPCSTR   lpszDriver,
     LPCSTR   lpszAttributes)
{
	char *dsn, *db, *cluster, **dst;
	const char *pos, *kv;
	dsn = db = cluster = NULL;
    dst = NULL;

	if (lpszDriver == NULL)
		return FALSE;

	for (pos = lpszAttributes; *pos; ++pos) {
        dst = NULL;
		kv = pos;
		for ( ; ; ++pos) {
			if (*pos == '\0')
				return FALSE;
			else if (*pos == '=')
				break;
		}

        if (strncasecmp("dsn", kv, 3) == 0)
            dst = &dsn;
        else if (strncasecmp("database", kv, 8) == 0)
            dst = &db;
        else if (strncasecmp("cluster", kv, 7) == 0)
            dst = &cluster;

        /* Skip values */
        kv = ++pos;
        for (; *pos != '\0'; ++pos) { /* blank */ };
        if (dst != NULL)
            *dst = strdup(kv);
	}

	switch (fRequest) {
		case ODBC_ADD_DSN:
		case ODBC_CONFIG_DSN:
            if (dsn == NULL || db == NULL || cluster == NULL) {
                if (hwnd != NULL) {
                    MessageBox(hwnd,
                            "Please use odbcconf.exe to configure DSN",
                            "Unsupported GUI feature",
                            MB_ICONEXCLAMATION | MB_OK);
                }
                return FALSE;
			}
            if (!SQLWriteDSNToIni(dsn, lpszDriver))
                return FALSE;
            if (!SQLWritePrivateProfileString(dsn, "DATABASE", db, ODBC_INI))
                return FALSE;
            if (!SQLWritePrivateProfileString(dsn, "CLUSTER", cluster, ODBC_INI))
                return FALSE;
            free(dsn);
            free(db);
            free(cluster);
            return TRUE;
		case ODBC_REMOVE_DSN:
            if (dsn == NULL)
                return FALSE;
            return SQLRemoveDSNFromIni(dsn);
	}
    return FALSE;
}

#define SQL_FUNC_SET(arr, id) \
	(*(((SQLSMALLINT*)(arr)) + ((id) >> 4)) |= (1 << ((id) & 0x000F)))
SQLRETURN SQL_API SQLGetFunctions(
		HDBC hdbc,
		SQLUSMALLINT func,
	   	SQLUSMALLINT *supported)
{
    (void)hdbc;

    __debug("enters method. attr = %d", attr);

	if (func == SQL_API_ODBC3_ALL_FUNCTIONS) {
		memset(supported, 0, sizeof(SQLSMALLINT) * SQL_API_ODBC3_ALL_FUNCTIONS_SIZE);
		SQL_FUNC_SET(supported, SQL_API_SQLBINDCOL);
		SQL_FUNC_SET(supported, SQL_API_SQLCOLATTRIBUTE);
		SQL_FUNC_SET(supported, SQL_API_SQLCONNECT);
		SQL_FUNC_SET(supported, SQL_API_SQLDESCRIBECOL);
		SQL_FUNC_SET(supported, SQL_API_SQLDISCONNECT);
		SQL_FUNC_SET(supported, SQL_API_SQLEXECDIRECT);
		SQL_FUNC_SET(supported, SQL_API_SQLEXECUTE);
		SQL_FUNC_SET(supported, SQL_API_SQLFETCH);
		SQL_FUNC_SET(supported, SQL_API_SQLFREECONNECT);
		SQL_FUNC_SET(supported, SQL_API_SQLFREEENV);
		SQL_FUNC_SET(supported, SQL_API_SQLFREESTMT);
		SQL_FUNC_SET(supported, SQL_API_SQLNUMRESULTCOLS);
		SQL_FUNC_SET(supported, SQL_API_SQLPREPARE);
		SQL_FUNC_SET(supported, SQL_API_SQLROWCOUNT);
		SQL_FUNC_SET(supported, SQL_API_SQLTRANSACT);
		SQL_FUNC_SET(supported, SQL_API_SQLCOLUMNS);
		SQL_FUNC_SET(supported, SQL_API_SQLDRIVERCONNECT);
		SQL_FUNC_SET(supported, SQL_API_SQLGETDATA);
		SQL_FUNC_SET(supported, SQL_API_SQLGETFUNCTIONS);
		SQL_FUNC_SET(supported, SQL_API_SQLGETINFO);
		SQL_FUNC_SET(supported, SQL_API_SQLGETSTMTOPTION);
		SQL_FUNC_SET(supported, SQL_API_SQLGETTYPEINFO);
		SQL_FUNC_SET(supported, SQL_API_SQLSETSTMTOPTION);
		SQL_FUNC_SET(supported, SQL_API_SQLSPECIALCOLUMNS);
		SQL_FUNC_SET(supported, SQL_API_SQLSTATISTICS);
		SQL_FUNC_SET(supported, SQL_API_SQLTABLES);
		SQL_FUNC_SET(supported, SQL_API_SQLCOLUMNPRIVILEGES);
		SQL_FUNC_SET(supported, SQL_API_SQLFOREIGNKEYS);
		SQL_FUNC_SET(supported, SQL_API_SQLMORERESULTS);
		SQL_FUNC_SET(supported, SQL_API_SQLNUMPARAMS);
		SQL_FUNC_SET(supported, SQL_API_SQLPRIMARYKEYS);
		SQL_FUNC_SET(supported, SQL_API_SQLPROCEDURECOLUMNS);
		SQL_FUNC_SET(supported, SQL_API_SQLPROCEDURES);
		SQL_FUNC_SET(supported, SQL_API_SQLBINDPARAMETER);
		SQL_FUNC_SET(supported, SQL_API_SQLALLOCHANDLE);
		SQL_FUNC_SET(supported, SQL_API_SQLENDTRAN);
		SQL_FUNC_SET(supported, SQL_API_SQLFREEHANDLE);
		SQL_FUNC_SET(supported, SQL_API_SQLGETCONNECTATTR);
		SQL_FUNC_SET(supported, SQL_API_SQLGETDIAGFIELD);
		SQL_FUNC_SET(supported, SQL_API_SQLGETDIAGREC);
		SQL_FUNC_SET(supported, SQL_API_SQLGETSTMTATTR);
		SQL_FUNC_SET(supported, SQL_API_SQLSETCONNECTATTR);
		SQL_FUNC_SET(supported, SQL_API_SQLSETDESCFIELD);
		SQL_FUNC_SET(supported, SQL_API_SQLSETSTMTATTR);
	} else if (func == SQL_API_ALL_FUNCTIONS) {
		memset(supported, 0, sizeof(SQLSMALLINT) * 100);
		supported[SQL_API_SQLALLOCCONNECT] = TRUE;
		supported[SQL_API_SQLALLOCENV] = TRUE;
		supported[SQL_API_SQLALLOCSTMT] = TRUE;
		supported[SQL_API_SQLBINDCOL] = TRUE;
		supported[SQL_API_SQLCOLATTRIBUTES] = TRUE;
		supported[SQL_API_SQLCONNECT] = TRUE;
		supported[SQL_API_SQLDESCRIBECOL] = TRUE;
		supported[SQL_API_SQLDISCONNECT] = TRUE;
		supported[SQL_API_SQLEXECDIRECT] = TRUE;
		supported[SQL_API_SQLEXECUTE] = TRUE;
		supported[SQL_API_SQLFETCH] = TRUE;
		supported[SQL_API_SQLFREECONNECT] = TRUE;
		supported[SQL_API_SQLFREEENV] = TRUE;
		supported[SQL_API_SQLFREESTMT] = TRUE;
		supported[SQL_API_SQLNUMRESULTCOLS] = TRUE;
		supported[SQL_API_SQLPREPARE] = TRUE;
		supported[SQL_API_SQLROWCOUNT] = TRUE;
		supported[SQL_API_SQLTRANSACT] = TRUE;
		supported[SQL_API_SQLBINDPARAMETER] = TRUE;
		supported[SQL_API_SQLCOLUMNS] = TRUE;
		supported[SQL_API_SQLDRIVERCONNECT] = TRUE;
		supported[SQL_API_SQLGETDATA] = TRUE;
		supported[SQL_API_SQLGETFUNCTIONS] = TRUE;
		supported[SQL_API_SQLGETINFO] = TRUE;
		supported[SQL_API_SQLGETSTMTOPTION] = TRUE;
		supported[SQL_API_SQLGETTYPEINFO] = TRUE;
		supported[SQL_API_SQLSETCONNECTOPTION] = TRUE;
		supported[SQL_API_SQLSETSTMTOPTION] = TRUE;
		supported[SQL_API_SQLSPECIALCOLUMNS] = TRUE;
		supported[SQL_API_SQLSTATISTICS] = TRUE;
		supported[SQL_API_SQLTABLES] = TRUE;
		supported[SQL_API_SQLCOLUMNPRIVILEGES] = FALSE;
		supported[SQL_API_SQLFOREIGNKEYS] = TRUE;
		supported[SQL_API_SQLMORERESULTS] = TRUE;
		supported[SQL_API_SQLNUMPARAMS] = TRUE;
		supported[SQL_API_SQLPRIMARYKEYS] = TRUE;
		supported[SQL_API_SQLPROCEDURECOLUMNS] = TRUE;
		supported[SQL_API_SQLPROCEDURES] = TRUE;
		supported[SQL_API_SQLTABLEPRIVILEGES] = TRUE;
	} else {
		*supported = FALSE;
		switch (func) {
			case SQL_API_SQLALLOCCONNECT:
			case SQL_API_SQLALLOCENV:
			case SQL_API_SQLALLOCSTMT:
			case SQL_API_SQLBINDCOL:
			case SQL_API_SQLCOLATTRIBUTES:
			case SQL_API_SQLCONNECT:
			case SQL_API_SQLDESCRIBECOL:
			case SQL_API_SQLDISCONNECT:
			case SQL_API_SQLEXECDIRECT:
			case SQL_API_SQLEXECUTE:
			case SQL_API_SQLFETCH:
			case SQL_API_SQLFREECONNECT:
			case SQL_API_SQLFREEENV:
			case SQL_API_SQLFREESTMT:
			case SQL_API_SQLNUMRESULTCOLS:
			case SQL_API_SQLPREPARE:
			case SQL_API_SQLROWCOUNT:
			case SQL_API_SQLTRANSACT:
			case SQL_API_SQLBINDPARAMETER:
			case SQL_API_SQLCOLUMNS:
			case SQL_API_SQLDRIVERCONNECT:
			case SQL_API_SQLGETDATA:
			case SQL_API_SQLGETFUNCTIONS:
			case SQL_API_SQLGETINFO:
			case SQL_API_SQLGETSTMTOPTION:
			case SQL_API_SQLGETTYPEINFO:
			case SQL_API_SQLSETCONNECTOPTION:
			case SQL_API_SQLSETSTMTOPTION:
			case SQL_API_SQLSPECIALCOLUMNS:
			case SQL_API_SQLSTATISTICS:
			case SQL_API_SQLTABLES:
			case SQL_API_SQLCOLUMNPRIVILEGES:
			case SQL_API_SQLFOREIGNKEYS:
			case SQL_API_SQLMORERESULTS:
			case SQL_API_SQLNUMPARAMS:
			case SQL_API_SQLPRIMARYKEYS:
			case SQL_API_SQLPROCEDURECOLUMNS:
			case SQL_API_SQLPROCEDURES:
			case SQL_API_SQLTABLEPRIVILEGES:
				*supported = TRUE;
		}
	}

	return SQL_SUCCESS;
}
