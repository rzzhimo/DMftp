/*
common.h
*/
#ifndef COMMON_H
#define COMMON_H

#include <arpa/inet.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>        
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

/*dm database*/
#include <sql.h>
#include <sqltypes.h>
#include <sqlext.h>

#define DEBUG                1
#define MAXSIZE             512     
#define CLIENT_PORT_ID        30020

//dm databse
/* 检测返回代码是否为成功标志，当为成功标志返回 TRUE，否则返回 FALSE */
#define RC_SUCCESSFUL(rc) ((rc) == SQL_SUCCESS || (rc) == SQL_SUCCESS_WITH_INFO)
/* 检测返回代码是否为失败标志，当为失败标志返回 TRUE，否则返回 FALSE */
#define RC_NOTSUCCESSFUL(rc) (!(RC_SUCCESSFUL(rc)))

HENV henv;/* 环境句柄 */
HDBC hdbc;/* 连接句柄 */
HSTMT hsmt;/* 语句句柄 */
SQLRETURN sret; /* 返回代码 */

struct command 
{
    char arg[255];
    char code[5];
};

int socket_create(int port);

int socket_accept(int sock_listen);

int socket_connect(int port, char *host);

int recv_data(int sockfd, char* buf, int bufsize);

int send_response(int sockfd, int rc);

void trimstr(char *str, int n);

void read_input(char* buffer, int size);

void initsql();
void freesql();

#endif