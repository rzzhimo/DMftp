#include <stdio.h>
#include <stdlib.h>
#include <sql.h>
#include <sqltypes.h>
#include <sqlext.h>

/* 检测返回代码是否为成功标志，当为成功标志返回 TRUE，否则返回 FALSE */
#define RC_SUCCESSFUL(rc) ((rc) == SQL_SUCCESS || (rc) == SQL_SUCCESS_WITH_INFO)
/* 检测返回代码是否为失败标志，当为失败标志返回 TRUE，否则返回 FALSE */
#define RC_NOTSUCCESSFUL(rc) (!(RC_SUCCESSFUL(rc)))

HENV henv;/* 环境句柄 */
HDBC hdbc;/* 连接句柄 */
HSTMT hsmt;/* 语句句柄 */
SQLRETURN sret; /* 返回代码 */
int errcode;
char err_msg[512];

int main(void)
{
    SQLCHAR     sql[]="select * from SYSDBA.dbuser where dbuser=?";
    SQLCHAR     in_c1[20] = { 0 };
    SQLLEN      in_c1_ind_ptr;

    memcpy(in_c1, "user", 4);
    in_c1_ind_ptr = 4; 
    char     out_c1[20];
    char    out_c2 [20];
    SQLLEN  out_c1_ind = 0;
    SQLLEN  out_c2_ind = 0;

    /* 申请一个环境句柄 */
    SQLAllocHandle(SQL_HANDLE_ENV, NULL, &henv);
    /* 设置环境句柄的 ODBC 版本 */
    SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3,
        SQL_IS_INTEGER);
    /* 申请一个连接句柄 */
    SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);

    sret = SQLConnect(hdbc, (SQLCHAR *)"DM", SQL_NTS, (SQLCHAR *)"SYSDBA", SQL_NTS, (SQLCHAR *)"SYSDBA", SQL_NTS);
    if (RC_NOTSUCCESSFUL(sret)) {
        /* 连接数据源失败! */
        printf("odbc: fail to connect to server!\n");
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        exit(0);
    }
    printf("odbc: connect to server success!\n");

    SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hsmt);

    printf("odbc: select * from table...\n");
    sret = SQLPrepare(hsmt, sql, SQL_NTS);
    sret = SQLBindParameter(hsmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, sizeof(in_c1), 0, in_c1, 0, &in_c1_ind_ptr);
    sret = SQLExecute(hsmt);
    if (RC_NOTSUCCESSFUL(sret)) {
        printf( "odbc: select * from  table with bind fail!\n" );
    }
    printf( "odbc: select * from  table with bind success!\n" );

    if (RC_NOTSUCCESSFUL(sret)) {
        /* 数据选取失败! */
        SQLGetDiagRec(SQL_HANDLE_STMT, hsmt, 1, NULL,&errcode, err_msg, sizeof(err_msg), NULL);
        printf("the err code is:%d\n",errcode);
        printf(err_msg);
        printf("odbc: select from server fail!\n");
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        exit(0);
    }
    
    SQLBindCol(hsmt, 1, SQL_C_CHAR, out_c1, sizeof(out_c1), &out_c1_ind);
    SQLBindCol(hsmt, 2, SQL_C_CHAR, out_c2, sizeof(out_c2), &out_c2_ind);

    // while(SQLFetchScroll(hsmt,SQL_FETCH_NEXT,0) != SQL_NO_DATA_FOUND)
    // {
    //     printf("c1 = %d, c2 = %d ,\n", out_c1, out_c2);
    // }
    for ( ;;)
    {
        sret = SQLFetchScroll(hsmt,SQL_FETCH_NEXT,0);
        if (sret == SQL_NO_DATA_FOUND)
        {
            break;
        }
        printf("c1 = %s, c2 = %s ,\n", out_c1, out_c2);
        
    }
    SQLFreeHandle(SQL_HANDLE_STMT, hsmt);
    /* 断开与数据源之间的连接 */
    SQLDisconnect(hdbc);
    /* 释放连接句柄 */
    SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
    /* 释放环境句柄 */
    SQLFreeHandle(SQL_HANDLE_ENV, henv);

    return 0;
}

 
