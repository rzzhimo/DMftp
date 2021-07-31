/*
common.c
*/
#include "common.h"

/**
 * 创建监听套接字
 * 错误返回 -1，正确返回套接字描述符
 */
int socket_create(int port)
{
    int sockfd;
    int yes = 1;
    struct sockaddr_in sock_addr;

    // 创建套接字
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    {
        perror("socket() error"); 

        return -1; 
    }

    // 设置本地套接字地址
    sock_addr.sin_family = AF_INET;/*该属性表示接收本机或其他机器传输*/
    sock_addr.sin_port = htons(port);
    sock_addr.sin_addr.s_addr = htonl(INADDR_ANY); //转换过来就是0.0.0.0，泛指本机的意思，也就是表示本机的所有IP   

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) //SO_REUSEADDR是让端口释放后立即就可以被再次使用
    {
        close(sockfd);
        perror("setsockopt() error");
        return -1; 
    }

    // 绑定本地套接字地址到套接字
    if (bind(sockfd, (struct sockaddr *) &sock_addr, sizeof(sock_addr)) < 0) 
    {
        close(sockfd);
        perror("bind() error"); 
        return -1; 
    }
   
    // 将套接字设置为监听状态
    if (listen(sockfd, 5) < 0) 
    {
        close(sockfd);
        perror("listen() error");
        return -1;
    }              
    return sockfd;
}

/**
 * 套接字接受请求
 * 错误返回 -1，正确返回新的连接套接字
 */
int socket_accept(int sock_listen)
{
    int sockfd;
    struct sockaddr_in client_addr;
    socklen_t len = sizeof(client_addr);
    sockfd = accept(sock_listen, (struct sockaddr *) &client_addr, &len); // 等待连接
    
    if (sockfd < 0)
    {
        perror("accept() error"); 
        return -1; 
    }
    return sockfd;
}

/**
 * 连接远端主机
 *  成功返回套接字描述符，失败返回 -1
*/
int socket_connect(int port, char*host)
{
    int sockfd;                      
    struct sockaddr_in dest_addr;

    /* 创建套接字 */
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    { 
            perror("error creating socket");
            return -1;
    }

    /* 设置协议地址 */
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(port);
    dest_addr.sin_addr.s_addr = inet_addr(host);

    /* 在套接字上创建连接 */
    if(connect(sockfd, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0 )
    {
        perror("error connecting to server");
        return -1;
    }    
    return sockfd;
}

/**
 * 接收数据
 * 错误返回 -1，正确返回接收的字节数 
 */
int recv_data(int sockfd, char* buf, int bufsize)
{
    size_t num_bytes;
    memset(buf, 0, bufsize);

    /* 调用 recv 函数读取套接字数据 */
    num_bytes = recv(sockfd, buf, bufsize, 0);
    if (num_bytes < 0) 
        return -1;

    return num_bytes;
}

/**
 * 去除字符串中的空格和换行符
 */
void trimstr(char *str, int n)
{
    int i;
    for (i = 0; i < n; i++) 
    {
        if (isspace(str[i])) str[i] = 0;
        if (str[i] == '\n') str[i] = 0;
    }
}

/**
 * 发送响应码到 sockfd
 * 错误返回 -1，正确返回 0
 */
int send_response(int sockfd, int rc)
{
    int conv = htonl(rc);
    if (send(sockfd, &conv, sizeof conv, 0) < 0 ) 
    {
        perror("error sending...\n");
        freesql();
        return -1;
    }
    return 0;
}

/** 
 * 从命令行中读取输入
 */
void read_input(char* buffer, int size)
{
    char *nl = NULL;
    memset(buffer, 0, size);

    if ( fgets(buffer, size, stdin) != NULL ) 
    {
        nl = strchr(buffer, '\n');
        if (nl) 
            *nl = '\0'; // 出现换行符，则将该位置部位'\0'（字符串结尾）
    }
}

/*初始化环境
*/
void initsql()
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
}
/*结束环境
*/
void freesql(){
    /* 断开与数据源之间的连接 */
    SQLDisconnect(hdbc);
    /* 释放连接句柄 */
    SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
    /* 释放环境句柄 */
    SQLFreeHandle(SQL_HANDLE_ENV, henv);
}