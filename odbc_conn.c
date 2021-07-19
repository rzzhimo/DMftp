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

int main(void)
{
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

    /* 断开与数据源之间的连接 */
    SQLDisconnect(hdbc);
    /* 释放连接句柄 */
    SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
    /* 释放环境句柄 */
    SQLFreeHandle(SQL_HANDLE_ENV, henv);

    return 0;
}

 
