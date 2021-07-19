#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sql.h>
#include <sqltypes.h>
#include <sqlext.h>

/* 检测返回代码是否为成功标志，当为成功标志返回 TRUE，否则返回 FALSE */
#define RC_SUCCESSFUL(rc) ((rc) == SQL_SUCCESS || (rc) == SQL_SUCCESS_WITH_INFO)
/* 检测返回代码是否为失败标志，当为失败标志返回 TRUE，否则返回 FALSE */
#define RC_NOTSUCCESSFUL(rc) (!(RC_SUCCESSFUL(rc)))

#define IN_FILE "./file/DM8_SQL.pdf"
#define OUT_FILE "./file/DM8_SQL1.pdf"
#define CHARS 80*1024 //一次读取和写入的字节数 80 KB

HENV henv;/* 环境句柄 */
HDBC hdbc;/* 连接句柄 */
HSTMT hstmt;/* 语句句柄 */
SQLRETURN sret; /* 返回代码 */

SQLSMALLINT errmsglen;
SQLINTEGER errnative;
UCHAR errmsg[255];
UCHAR errstate[5];

int main(void)
{
    FILE*   pfile = NULL;
    SQLCHAR tmpbuf[CHARS];
    SQLLEN  len = 0;
    SQLLEN  val_len = 0;

    SQLLEN      c1 =1; 
    SQLLEN      c2 = SQL_DATA_AT_EXEC;
    SQLLEN      c1_ind_ptr = 0;
    SQLLEN      c2_ind_ptr = SQL_DATA_AT_EXEC;
    PTR         c2_val_ptr;

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
    sret = SQLExecDirect(hstmt, (SQLCHAR *)"drop table odbc_demo", SQL_NTS);
    sret = SQLExecDirect(hstmt, (SQLCHAR *)"create table odbc_demo(c1 int, c2 blob)", SQL_NTS);

    //读取文件，插入到 LOB 列
    pfile = fopen(IN_FILE, "rb");
    if (pfile == NULL)
    {
        printf("open %s fail\n", IN_FILE);
        return SQL_ERROR;
    }

    sret = SQLPrepare(hstmt, (SQLCHAR *)"insert into odbc_demo(c1,c2) values(?,?)", SQL_NTS);
    sret = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG,  SQL_INTEGER, sizeof(c1), 0, &c1, sizeof(c1), NULL);
    sret = SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_BINARY,  SQL_VARBINARY, sizeof(c2), 0, (void *)1, sizeof(c2), &c2_ind_ptr);
    sret = SQLExecute(hstmt);
    if (sret == SQL_NEED_DATA)
    {
        if (SQLParamData(hstmt, &c2_val_ptr) == SQL_NEED_DATA) /* 绑定数据 */
        {
            while (!feof(pfile))
            {
                len = fread(tmpbuf, sizeof(char), CHARS, pfile);
                if (len <= 0)
                {
                    return SQL_ERROR;
                }
                SQLPutData(hstmt, tmpbuf, len);
            }
        }
        SQLParamData(hstmt, &c2_val_ptr); /* 绑定数据 */
    }else if( sret == SQL_ERROR ) 
    {
        SQLGetDiagRec(SQL_HANDLE_STMT, hstmt, 1, errstate, &errnative, errmsg, sizeof(errmsg), &errmsglen);
        printf( "error:%s\n", errmsg );
    }

    printf("odbc: insesret data into col of lob success\n");
    fclose(pfile);

    //读取 LOB 列数据，写入文件
    pfile = fopen((const char *)OUT_FILE, "wb");
    if (pfile == NULL)
    {
        printf("open %s fail\n", OUT_FILE);
        return SQL_ERROR;
    }

    sret = SQLExecDirect(hstmt, (SQLCHAR *)"select c1, c2 from odbc_demo", SQL_NTS);
    sret = SQLBindCol(hstmt, 1, SQL_C_SLONG, &c1, sizeof(c1), &c1_ind_ptr); 
    while(SQLFetch(hstmt) != SQL_NO_DATA)
    {
        while(1)
        {
            sret = SQLGetData(hstmt, 2, SQL_C_BINARY, tmpbuf, CHARS, &val_len);
            if ((sret) == SQL_SUCCESS || (sret) == SQL_SUCCESS_WITH_INFO)
            {
                len = val_len > CHARS ? CHARS : val_len;
                fwrite(tmpbuf, sizeof(char), len, pfile);
                continue;
            }
            break;
        }
    }

    fclose(pfile);
    printf("odbc: get data from col of lob success\n");

    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);

    SQLDisconnect(hdbc);
    SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
    SQLFreeHandle(SQL_HANDLE_ENV, henv);
    return 0;
}

 
