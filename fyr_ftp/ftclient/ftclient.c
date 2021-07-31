/*
ftclient.c
*/
#include "ftclient.h"
    
int sock_control; 
int dellast(char *szBuf)
{
	int i; 
    //printf("szBuf len is :%d\n",strlen(szBuf));
    i = strlen(szBuf)-1; 
    while(szBuf[i] == ' '&&i >0) 
    i--; 
    szBuf[i+1] = '\0';

    return 0;
	
 } 
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
            printf("226 download file action successful.\n");
            break;
        case 225:
            printf("225 upload remote file  successful.\n");
            break;
        case 550:
            printf("550 Delete Requested action not taken. File unavailable.\n");
            break;
        case 551:
            printf("551 Uplpad requested action not taken. File has existed.\n");
            break;
        case 222:
            printf("222 File delete successfully.\n");
            break;
        case 522:
            printf("522 File delete failed.\n");
            break;
        case -1:
            printf("-1 path is illegal.\n");
            break;
        case 0:
            printf("0 file is end.\n");
            break;
        case 553:
            printf("upload failed.\n");
            break;
        case 223:
            printf("upload success.\n");
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

    while (arg != NULL)
    {
        strcat(cstruct->arg, arg);
        strcat(cstruct->arg, " ");
        arg = strtok (NULL, " ");
    }
    dellast(cstruct->arg);
    //printf("cstruct->arg is %s \n",cstruct->arg);

    if (strcmp(buf, "ls") == 0) 
        strcpy(cstruct->code, "LIST");

    else if (strcmp(buf, "download") == 0)
        strcpy(cstruct->code, "DOWN");
    else if (strcmp(buf, "upload") == 0)
        strcpy(cstruct->code, "UPLD");
    else if (strcmp(buf, "quit") == 0) 
        strcpy(cstruct->code, "QUIT");
    //delete指令
    else if (strcmp(buf, "delete") == 0) 
        strcpy(cstruct->code, "DELE");
    //history指令
    else if (strcmp(buf, "history") == 0) 
        strcpy(cstruct->code, "HIST");
    else 
        return -1; // 不合法

    memset(buf, 0, 400);
    strcpy(buf, cstruct->code);  // 存储命令到 buf 开始处

    /* 如果命令带有参数，追加到 buf */
    if(strlen(cstruct->arg)>0)
    {
        strcat(buf, " ");
        strncat(buf, cstruct->arg, strlen(cstruct->arg));
    }
    
    printf("本次执行命令为：\'%s\' \n",buf);
    return 0;
}

/**
 * 实现 download  <remote_file>  <local_file>命令行
 */
int ftclient_dwld(int data_sock, int sock_control, char* arg)
{
    //printf("%s",arg);
    char data[MAXSIZE];
    int size;
    char* l_file;
    char*  r_file;
    r_file = strtok (arg," ");
    l_file = strtok (NULL," ");
    //printf("%s",r_file);
    //printf("%s",l_file);
    FILE* fd = fopen(l_file, "w"); // 创建并打开名字为 arg 的文件

    /* 将服务器传来的数据（文件内容）写入本地建立的文件 */
    while ((size = recv(data_sock, data, MAXSIZE, 0)) > 0) 
        fwrite(data, 1, size, fd); 

    if (size < 0) 
        perror("error\n");

    fclose(fd);
    return 0;
}

/*判断upload时本地文件是否存在，并通知服务端*/
int up_check_path(char *arg){  
    char* l_file;
    char* f;
    f = strtok(arg," ");
    if(strcmp(f,"-f")==0)
    {
        l_file = strtok (NULL," ");
        
    }
    else
    {
        l_file = f;
    }
    char *p = "..";
    if(access(l_file,0)<0 || strstr(l_file,p)){
        send_response(sock_control, -1); 
        printf("%s 该文件路径不合法\n",l_file);
        return -1;
    }else{
        send_response(sock_control, 1); 
    }   
    return 0;
}

/**
 * 实现 upload [-f] <local_file> <remote_file> #上传文件， -f参数表示强制覆盖
 */
int ftclient_upload(int data_sock, int sock_control, char* arg)
{
    printf("开始执行ftclient_upload\n"); 
    
    FILE* fd = NULL;
    char data[MAXSIZE];
    size_t num_read;      
    char* l_file;
    char*  r_file;
   
    char* f;
    f = strtok(arg," ");
    if(strcmp(f,"-f")==0)
    {
        l_file = strtok (NULL," ");
        r_file = strtok (NULL," "); 
    }
    else
    {
        l_file = f;
        r_file = strtok (NULL," "); 
    }
    printf("ftclient r_file is: \"%s\",l_file is: \"%s\"\n",r_file,l_file);  

    
    FILE *pFile = fopen(l_file, "r" );// 打开文件
    if (!pFile)//打开失败
    {
        perror("upload open local file error\n");
        return -1;
    } 
    int num = 0;
    while (getc( pFile ) != EOF )
    {
    num++ ;
    }
    fclose( pFile );  
    printf("字节数为:%d\n",num);     
    int conv = htonl(num);
    if (send(sock_control, &conv, sizeof conv, 0) < 0 ) 
    {
        perror("error sending...\n");
        return -1;
    }     
    fd = fopen(l_file, "r"); // 打开文件

    if (!fd)
    {
        perror("upload open local file error\n");
        return -1;
    } 
    else
    {    
        //print_reply(read_reply());
        do 
        {
            num_read = fread(data, 1, MAXSIZE, fd); // 读文件内容
            if (num_read < 0) 
                printf("error in upload fread()\n");

            if (send(data_sock, data, num_read, 0) < 0) // 发送数据（文件内容）
                perror("error in upload sending file\n");
            
            printf("从本地发送数据成功\n");
            
        }
        while (num_read > 0);                      
        fclose(fd);
        }
    printf("从本地发送数据完成\n");  
    int reply = read_reply();
    print_reply(reply);
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
/**实现history -n命令
 */
int ftclient_hist(int data_sock, int sock_control){
    size_t num_recvd;            
    char buf[552];            
    int tmp = 0;

    /* 等待服务器启动的信息 */ 
    if (recv(sock_control, &tmp, sizeof tmp, 0) < 0) 
    {
        perror("client: error reading message from server\n");
        return -1;
    }
    
    memset(buf, 0, sizeof(buf));

    /* 接收服务器传来的数据 */
    while ((num_recvd = recv(data_sock, buf, 552, 0)) > 0) 
    {
        printf("%s\n", buf);
        memset(buf, 0, sizeof(buf));
    }
    
    if (num_recvd < 0) 
        perror("error");
    
    /* 等待服务器完成的消息 */ 
    if (recv(sock_control, &tmp, sizeof tmp, 0) < 0) 
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
    if (argc != 2)
    {
        printf("usage: ./ftclient hostname:port\n");
        exit(0);
    }

    char *host = strtok(argv[1],":"); //所要连接的服务器主机名
    char *port = strtok(NULL,":"); //所要链接到服务器程序端口号

    /* 获得和服务器名匹配的地址 */
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    s = getaddrinfo(host, port, &hints, &res);
    if (s != 0) 
    {
        printf("getaddrinfo() error %s\n", gai_strerror(s));
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
    printf("this sock_control is:%d\n",sock_control);

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
        printf("发送命令到服务器\n");
        if (send(sock_control, buffer, (int)strlen(buffer), 0) < 0 )
        {
            close(sock_control);
            exit(1);
        }
        
        
        retcode = read_reply();    //读取服务器响应（服务器是否可以支持该命令？）
        printf("读取服务器响应 %d \n",retcode);
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
            printf("命令合法\n");
            /* 打开数据连接 */
            if ((data_sock = ftclient_open_conn(sock_control)) < 0) 
            {
                perror("Error opening socket for data connection");
                exit(1);
            }            
            printf("this data_sock is:%d\n",data_sock);
            /* 执行命令 */
            printf("start execute...\n");
            if (strcmp(cmd.code, "LIST") == 0) 
                {
                    if (read_reply()==-1)
                    {
                        print_reply(-1);
                        continue;
                    }else{
                        ftclient_list(data_sock, sock_control);
                    }
                }
            if (strcmp(cmd.code, "HIST") == 0) 
                ftclient_hist(data_sock, sock_control);
            else if (strcmp(cmd.code, "DOWN") == 0) 
            {
                int path_flag = read_reply();//用于判断远程路径是否合法
                if (path_flag == -1)
                {
                    print_reply(-1);
                    continue;
                }
                //it means reply is 1

                printf("start execute download...\n");
                if (read_reply() == 550) // 等待回复
                {
                    print_reply(550);        
                    close(data_sock);
                    continue; 
                }
                //it means reply is 150
                ftclient_dwld(data_sock, sock_control, cmd.arg);
                
                print_reply(read_reply()); 
            }
            else if (strcmp(cmd.code, "UPLD") == 0) 
            {
                char  tmp[512]={0};
                strcpy(tmp,cmd.arg);
                int lpath_flag=up_check_path(tmp);//检查本地路径
                if(lpath_flag == -1){
                    print_reply(-1);
                    continue;
                }
                int path_flag = read_reply();//用于判断远程路径是否合法
                if (path_flag == -1)
                {
                    print_reply(-1);
                    continue;
                }
                int icode = read_reply();
                if (icode == 550) // 550 Requested action not taken
                {
                    print_reply(550);        
                    close(data_sock);
                    continue; 
                }
                else if (icode == 551) // 文件已存在
                {
                    print_reply(551);        
                    close(data_sock);
                    continue; 
                }
                else if(icode == 150){
                    printf("start execute upload3...\n ");
                    ftclient_upload(data_sock, sock_control, cmd.arg);
                }
                
            }
            else if (strcmp(cmd.code, "DELE") == 0)
            {
                retcode = read_reply();
                if(retcode == -1){
                    print_reply(-1);
                }
                else if(retcode == 222){
                    print_reply(222);
                }
                else if (retcode == 522)
                {
                    print_reply(522);
                }
            }   
            close(data_sock);
        }

    } // 循环得到更多的用户输入

    close(sock_control); // 关闭套接字控制连接
    return 0;  
}