/*
ftclient.c
*/
#include "ftclient.h"
    
int sock_control; 

/**
 * 接收服务器响应
 * 错误返回 -1，正确返回状态码
 */
int read_reply()
{
    int retcode = 0;
    if (recv(sock_control, &retcode, sizeof retcode, 0) < 0) 
    {
        perror("client: error reading message from server\n");
        return -1;
    }    
    return ntohl(retcode);
}

/**
 * 打印响应信息
 */
void print_reply(int rc) 
{
    switch (rc)
    {
        case 220:
            printf("220 Welcome, server ready.\n");
            break;
        case 221:
            printf("221 Goodbye!\n");
            break;
        case 226:
            printf("226 Closing data connection. Requested file action successful.\n");
            break;
        case 550:
            printf("550 Requested action not taken. File unavailable.\n");
            break;
    }
}

/**
 * 解析命令行到结构体
 */ 
int ftclient_read_command(char* buf, int size, struct command *cstruct)
{
    memset(cstruct->code, 0, sizeof(cstruct->code));
    memset(cstruct->arg, 0, sizeof(cstruct->arg));
    
    printf("ftclient> ");    // 输入提示符    
    fflush(stdout);     
    read_input(buf, size); // 等待用户输入命令
    char *arg = NULL;
    arg = strtok (buf," ");
    arg = strtok (NULL, " ");

    if (arg != NULL)
        strncpy(cstruct->arg, arg, strlen(arg));

    if (strcmp(buf, "list") == 0) 
        strcpy(cstruct->code, "LIST");

    else if (strcmp(buf, "get") == 0)
        strcpy(cstruct->code, "RETR");

    else if (strcmp(buf, "quit") == 0) 
        strcpy(cstruct->code, "QUIT");
    
    else 
        return -1; // 不合法

    memset(buf, 0, 400);
    strcpy(buf, cstruct->code);  // 存储命令到 buf 开始处

    /* 如果命令带有参数，追加到 buf */
    if (arg != NULL) 
    {
        strcat(buf, " ");
        strncat(buf, cstruct->arg, strlen(cstruct->arg));
    }
    return 0;
}

/**
 * 实现 get <filename> 命令行
 */
int ftclient_get(int data_sock, int sock_control, char* arg)
{
    char data[MAXSIZE];
    int size;
    FILE* fd = fopen(arg, "w"); // 创建并打开名字为 arg 的文件

    /* 将服务器传来的数据（文件内容）写入本地建立的文件 */
    while ((size = recv(data_sock, data, MAXSIZE, 0)) > 0) 
        fwrite(data, 1, size, fd); 

    if (size < 0) 
        perror("error\n");

    fclose(fd);
    return 0;
}

/**
 * 打开数据连接
 */
int ftclient_open_conn(int sock_con)
{
    int sock_listen = socket_create(CLIENT_PORT_ID);

    /* 在控制连接上发起一个 ACK 确认 */
    int ack = 1;
    if ((send(sock_con, (char*) &ack, sizeof(ack), 0)) < 0) 
    {
        printf("client: ack write error :%d\n", errno);
        exit(1);
    }        

    int sock_conn = socket_accept(sock_listen);
    close(sock_listen);
    return sock_conn;
}

/** 
 * 实现 list 命令
 */
int ftclient_list(int sock_data, int sock_con)
{
    size_t num_recvd;            
    char buf[MAXSIZE];            
    int tmp = 0;

    /* 等待服务器启动的信息 */ 
    if (recv(sock_con, &tmp, sizeof tmp, 0) < 0) 
    {
        perror("client: error reading message from server\n");
        return -1;
    }
    
    memset(buf, 0, sizeof(buf));

    /* 接收服务器传来的数据 */
    while ((num_recvd = recv(sock_data, buf, MAXSIZE, 0)) > 0) 
    {
        printf("%s", buf);
        memset(buf, 0, sizeof(buf));
    }
    
    if (num_recvd < 0) 
        perror("error");
    
    /* 等待服务器完成的消息 */ 
    if (recv(sock_con, &tmp, sizeof tmp, 0) < 0) 
    {
        perror("client: error reading message from server\n");
        return -1;
    }
    return 0;
}

/**
 * 输入含有命令(code)和参数(arg)的 command(cmd) 结构
 * 连接 code + arg,并放进一个字符串，然后发送给服务器
 */
int ftclient_send_cmd(struct command *cmd)
{
    char buffer[MAXSIZE];
    int rc;

    sprintf(buffer, "%s %s", cmd->code, cmd->arg);
    
    /* 发送命令字符串到服务器 */ 
    rc = send(sock_control, buffer, (int)strlen(buffer), 0);    
    if (rc < 0) 
    {
        perror("Error sending command to server");
        return -1;
    }
    
    return 0;
}

/**
 * 获取登录信息
 * 发送到服务器认证
 */
void ftclient_login()
{
    struct command cmd;
    char user[256];
    memset(user, 0, 256);

    /* 获取用户名 */ 
    printf("Name: ");    
    fflush(stdout);         
    read_input(user, 256);

    /* 发送用户名到服务器 */ 
    strcpy(cmd.code, "USER");
    strcpy(cmd.arg, user);
    ftclient_send_cmd(&cmd);
    
    /* 等待应答码 331 */
    int wait;
    recv(sock_control, &wait, sizeof wait, 0);

    /* 获得密码 */
    fflush(stdout);    
    char *pass = getpass("Password: ");    

    /* 发送密码到服务器 */ 
    strcpy(cmd.code, "PASS");
    strcpy(cmd.arg, pass);
    ftclient_send_cmd(&cmd);
    
    /* 等待响应 */ 
    int retcode = read_reply();
    switch (retcode) 
    {
        case 430:
            printf("Invalid username/password.\n");
            exit(0);
        case 230:
            printf("Successful login.\n");
            break;
        default:
            perror("error reading message from server");
            exit(1);        
            break;
    }
}

/* 主函数入口 */
int main(int argc, char* argv[]) 
{        
    int data_sock, retcode, s;
    char buffer[MAXSIZE];
    struct command cmd;    
    struct addrinfo hints, *res, *rp;

    /* 命令行参数合法性检测 */
    if (argc != 3)
    {
        printf("usage: ./ftclient hostname port\n");
        exit(0);
    }

    char *host = argv[1]; //所要连接的服务器主机名
    char *port = argv[2]; //所要链接到服务器程序端口号

    /* 获得和服务器名匹配的地址 */
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    s = getaddrinfo(host, port, &hints, &res);
    if (s != 0) 
    {
        printf("getaddrinfo() error %s", gai_strerror(s));
        exit(1);
    }
    
    /* 找到对应的服务器地址并连接 */ 
    for (rp = res; rp != NULL; rp = rp->ai_next) 
    {
        sock_control = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol); // 创建控制套接字

        if (sock_control < 0)
            continue;

        if(connect(sock_control, res->ai_addr, res->ai_addrlen)==0)   // 和服务器连接
            break;
        
        else 
        {
            perror("connecting stream socket");
            exit(1);
        }
        close(sock_control);
    }
    freeaddrinfo(rp);


    /* 连接成功，打印信息 */
    printf("Connected to %s.\n", host);
    print_reply(read_reply()); 
    

    /* 获取用户的名字和密码 */
    ftclient_login();

    while (1) 
    { // 循环，直到用户输入 quit

        /* 得到用户输入的命令 */ 
        if ( ftclient_read_command(buffer, sizeof buffer, &cmd) < 0)
        {
            printf("Invalid command\n");
            continue;    // 跳过本次循环，处理下一个命令
        }

        /* 发送命令到服务器 */ 
        if (send(sock_control, buffer, (int)strlen(buffer), 0) < 0 )
        {
            close(sock_control);
            exit(1);
        }

        retcode = read_reply();    //读取服务器响应（服务器是否可以支持该命令？）

        if (retcode == 221)  // 退出命令
        {
            print_reply(221);        
            break;
        }
        
        if (retcode == 502) 
            printf("%d Invalid command.\n", retcode);// 不合法的输入，显示错误信息

        else 
        {            
            // 命令合法 (RC = 200),处理命令
        
            /* 打开数据连接 */
            if ((data_sock = ftclient_open_conn(sock_control)) < 0) 
            {
                perror("Error opening socket for data connection");
                exit(1);
            }            
            
            /* 执行命令 */
            if (strcmp(cmd.code, "LIST") == 0) 
                ftclient_list(data_sock, sock_control);
            
            else if (strcmp(cmd.code, "RETR") == 0) 
            {
                if (read_reply() == 550) // 等待回复
                {
                    print_reply(550);        
                    close(data_sock);
                    continue; 
                }
                ftclient_get(data_sock, sock_control, cmd.arg);
                print_reply(read_reply()); 
            }
            close(data_sock);
        }

    } // 循环得到更多的用户输入

    close(sock_control); // 关闭套接字控制连接
    return 0;  
}