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
HSTMT hstmt;/* 语句句柄 */
SQLRETURN sret; /* 返回代码 */

int main(void)
{
    int     out_c1 = 0;
    SQLCHAR out_c2[20]= { 0 };
    SQLLEN out_c1_ind = 0;
    SQLLEN out_c2_ind = 0;

    /* 申请句柄 */
    SQLAllocHandle(SQL_HANDLE_ENV, NULL, &henv);
    SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, SQL_IS_INTEGER);
    SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);

    sret = SQLConnect(hdbc, (SQLCHAR *)"DM", SQL_NTS, (SQLCHAR *)"SYSDBA", SQL_NTS, (SQLCHAR *)"SYSDBA", SQL_NTS);
    if (RC_NOTSUCCESSFUL(sret)) {
        printf("odbc: fail to connect to server!\n");
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        exit(0);
    }
    printf("odbc: connect to server success!\n");

    /* 申请一个语句句柄 */
    SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

    //清空表，初始化测试环境
    sret = SQLExecDirect(hstmt, (SQLCHAR *) "delete from SYSDBA.PRODUCT_CATEGORY", SQL_NTS);

    //插入数据
    sret = SQLExecDirect(hstmt, (SQLCHAR *) "insert into SYSDBA.PRODUCT_CATEGORY(NAME) values('语文'), ('数学'), ('英语'), ('体育') ", SQL_NTS);
    if (RC_NOTSUCCESSFUL(sret)) {
        printf("odbc: insert fail\n");
    }
    printf("odbc: insert success\n");

    //删除数据
    sret = SQLExecDirect(hstmt, (SQLCHAR *) "delete from SYSDBA.PRODUCT_CATEGORY where name='数学' ", SQL_NTS);
    if (RC_NOTSUCCESSFUL(sret)) {
        printf("odbc: delete fail\n");
    }
    printf("odbc: delete success\n");


    //更新数据
    sret = SQLExecDirect(hstmt, (SQLCHAR *) "update SYSDBA.PRODUCT_CATEGORY set name = '英语-新课标' where name='英语' ", SQL_NTS);
    if (RC_NOTSUCCESSFUL(sret)) {
        printf("odbc: update fail\n");
    }
    printf("odbc: update success\n");

    //查询数据
    SQLExecDirect(hstmt, (SQLCHAR *) "select * from SYSDBA.PRODUCT_CATEGORY", SQL_NTS);
    SQLBindCol(hstmt, 1, SQL_C_SLONG, &out_c1, sizeof(out_c1), &out_c1_ind);
    SQLBindCol(hstmt, 2, SQL_C_CHAR, &out_c2, sizeof(out_c2), &out_c2_ind);

    printf("odbc: select from table...\n");
    while(SQLFetch(hstmt) != SQL_NO_DATA)
    {
        printf("c1 = %d, c2 = %s ,\n", out_c1, out_c2);
    }
    printf("odbc: select success\n");

    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);

    SQLDisconnect(hdbc);
    SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
    SQLFreeHandle(SQL_HANDLE_ENV, henv);
    return 0;
}

 
