SQLRETURN SQL_API SQLExecDirectW(SQLHSTMT        hstmt,
                                SQLWCHAR         *sql,
                                SQLINTEGER      len)
{
    SQLCHAR *sqlansi;
    int lenansi = wcstombs(NULL, sql, 0);
    sqlansi = malloc(lenansi + 1);
    wcstombs(sqlansi, sql, lenansi + 1);
    return SQLExecDirect(hstmt, sqlansi, SQL_NTS);
}