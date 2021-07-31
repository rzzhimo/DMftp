/*
ftserver.c
*/

#include "ftserver.h"



/* 主函数入口 */
int main(int argc, char *argv[])
{    
    int sock_listen, sock_control, port, pid;

    /* 命令行合法性检测 */
    if (argc != 2)
    {
        printf("usage: ./ftserver port\n");
        exit(0);
    }

    /* 将命令行传进来的服务器端口号（字符串）转换为整数 */
    port = atoi(argv[1]);

    /* 创建监听套接字 */
    if ((sock_listen = socket_create(port)) < 0 )
    {
        perror("Error creating socket");
        exit(1);
    }        
    
    /* 循环接受不同的客户机请求 */
    while(1) 
    {    
        /* 监听套接字接受连接请求，得到控制套接字，用于传递控制信息 */
        if ((sock_control = socket_accept(sock_listen))    < 0 )
            break;            
        
        /* 创建子进程处理用户请求 */
        if ((pid = fork()) < 0) 
            perror("Error forking child process");
    
        /* 子进程调用 ftserver_process 函数与客户端交互 */
        else if (pid == 0)
        { 
            close(sock_listen);  // 子进程关闭父进程的监听套接字
            ftserver_process(sock_control);        
            close(sock_control); //用户请求处理完毕，关闭该套接字
            exit(0);
        }
         
        close(sock_control); // 父进程关闭子进程的控制套接字
    }

    close(sock_listen);    

    return 0;
}

/**
 * 通过数据套接字发送特定的文件
 * 控制信息交互通过控制套接字
 * 处理无效的或者不存在的文件名
 */
int ftserver_dwld(int sock_control, int sock_data, char* filename)
{    
    //printf("%s ",filename);
    FILE* fd = NULL;
    char data[MAXSIZE];
    char r_path[2*MAXSIZE] = {0};
    size_t num_read;      
    char* l_file;
    char*  r_file;
    r_file = strtok (filename," ");
    l_file = strtok (NULL," ");   

    if (r_file[0]!='/')
    {
        sprintf(r_path,"%s/%s",root,r_file);
    }else{
        sprintf(r_path,"%s%s",root,r_file);
    }
    char *p = "..";
    if(strstr(r_path,p)||access(r_path,0)<0){
        send_response(sock_control, -1); 
        printf("%s 该文件路径不合法\n",r_path);
        return -1;
    }else{
        send_response(sock_control, 1);
    }                     

    fd = fopen(r_file, "r"); // 打开文件

    if (!fd)
        send_response(sock_control, 550); // 发送错误码 (550 Requested action not taken)
    
    else
    {    
        send_response(sock_control, 150); // 发送 okay (150 File status okay)
        
        do 
        {
            num_read = fread(data, 1, MAXSIZE, fd); // 读文件内容
            if (num_read < 0) 
                printf("error in fread()\n");

            if (send(sock_data, data, num_read, 0) < 0) // 发送数据（文件内容）
                perror("error sending file\n");

            
        }
        while (num_read > 0);                                                    
            
        send_response(sock_control, 226); // 发送消息：226: closing conn, file transfer successful

        fclose(fd);
    }
    return 0;
}
/**
 * 通过数据套接字接收特定的文件
 * 控制信息交互通过控制套接字
 * 处理无效的或者不存在的文件名
 */
int ftserver_upload(int sock_control, int sock_data, char* filename)
{
    //printf("ftserver break dot");
    printf("ftserver_upload:this sock_control is:%d\n",sock_control);
    char data[MAXSIZE];
    int size;
    char* l_file;
    char*  r_file;
    char* f;
    char r_path[2*MAXSIZE] = {0};
    f = strtok(filename," ");
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
    printf("ftserver r_file is: \"%s\",l_file is: \"%s\" \n",r_file,l_file);

    //客户端路径问题，必须存在
    int lpath_flag = 0;
    if (recv(sock_control, &lpath_flag, sizeof lpath_flag, 0) < 0) 
    { 
        perror("client: error reading message from client\n");
        return -1;
    }    
    lpath_flag =  ntohl(lpath_flag);
    printf("lpath_flag is %d\n",lpath_flag);
    if(lpath_flag<0){
        return -1;~
    }
    //远程路径问题，必须在root以下
    if (r_file[0]!='/')
    {
        sprintf(r_path,"%s/%s",root,r_file);
    }else{
        sprintf(r_path,"%s%s",root,r_file);
    }
    char *p = "..";
    if(strstr(r_path,p)){
        send_response(sock_control, -1); 
        printf("%s 该文件路径不合法\n",r_path);
        return -1;
    }else{
        send_response(sock_control, 1);
    }                     
    printf("this r_path is \'%s\'",r_path);
    if((strcmp(f,"-f")==0) || (access(r_path,0)==-1))
    {
        FILE* fd = fopen(r_path, "w"); // 创建并打开名字为 r_path 的文件
        if (!fd)
        {
            send_response(sock_control, 550); // 发送错误码 (550 Requested action not taken)
            return -1;
        }
        else
        {
            send_response(sock_control, 150); // 发送 okay (150 File status okay)
            printf("打开远程文件成功：%s\n",r_path);

            int retnum = 0;
            if (recv(sock_control, &retnum, sizeof retnum, 0) < 0) 
            { 
                perror("client: error reading message from client\n");
                return -1;
            }    
            retnum =  ntohl(retnum);
            printf("要接收的字节数为：%d\n",retnum);
            int realret = 0;
            /* 将服务器传来的数据（文件内容）写入本地建立的文件 */
            while ((size = recv(sock_data, data, MAXSIZE, 0)) > 0) 
            {
                fwrite(data, 1, size, fd);
                printf("写入远程文件：\n");
                realret = size+realret;
                printf("new realret is %d \n",realret);
                if (realret == retnum)
                {
                    break;
                }
                           
            }
            if (size < 0) 
                perror("error\n");
            printf("写入远程文件完成：\n");
            send_response(sock_control, 225); // 发送消息：225: upload remote file  successful
            fclose(fd);
        }
    
    }else
    {
        send_response(sock_control, 551); // 发送错误码 (文件已存在)
        return -1;
    }

    return 0;
    
}
/**
 * 通过数据套接字发送删除文件的结果
 * 控制信息交互通过控制套接字
 * 处理无效的或者不存在的文件名
 * 错误返回 -1， 正确返回0
 */
int ftserver_delete(int sock_control, int sock_data, char* filename)
{
    char path[2*MAXSIZE] = {0};
    if (filename[0]!='/')
    {
        sprintf(path,"%s/%s",root,filename);
    }else{
        sprintf(path,"%s%s",root,filename);
    }
    printf("要删除的文件为\"%s\" \n",path);
    char *p = "..";
    if(access(path,0)<0 || strstr(path,p)){
        send_response(sock_control, -1); 
        printf("%s 该文件路径不合法\n",path);
        return -1;
    }   
    if (remove(path) == 0) {
		printf("Deleted file %s successfully\n", path);
        send_response(sock_control, 222);    // 发送应答码 222（关闭数据连接，请求的文件操作成功）
        return 0; 
	} else {
		printf("Error in deleting file : %s\n", path);
        send_response(sock_control, 522);    // 发送应答码 522（关闭数据连接，请求的文件操作失败）
        return -1; 
	}
    

    
}
/*初始化ftserver所在的根目录*/
int init_root(){
    FILE * fp;
    char   buffer[MAXSIZE];
    fp=popen("pwd","r");
    fgets(buffer,  sizeof(buffer),fp);
    trimstr(buffer,(int)strlen(buffer));
    strcpy(root ,buffer);
    printf("root is \'%s\'\n",root);
    pclose(fp);
    return 0;
}
/**
 * 响应请求：发送当前所在目录的目录项列表
 * 关闭数据连接
 * 错误返回 -1，正确返回 0
 */
int ftserver_list(int sock_control,int sock_data, char* dir)
{
    char data[MAXSIZE];
    char sys_ls[3*MAXSIZE];
    size_t num_read; 
    char path[2*MAXSIZE] = {0};
    if (dir[0]!='/')
    {
        sprintf(path,"%s/%s",root,dir);
    }else{
        sprintf(path,"%s%s",root,dir);
    }
    char *p = "..";
    if(access(path,0)<0 || strstr(path,p)){
        send_response(sock_control, -1); 
        printf("%s 该文件路径不存在\n",path);
        return -1;
    }else{
        send_response(sock_control, 1);
    }                                   
    FILE* fd;
    sprintf(sys_ls,"ls -l %s | tail -n+2 > tmp.txt",path);
    int rs = system(sys_ls); //利用系统调用函数 system 执行命令，并重定向到 tmp.txt 文件
    if ( rs < 0)
    {
        exit(1);
        return -1;
    }
    
    fd = fopen("tmp.txt", "r");    
    if (!fd) 
    {
        exit(1); 
        return -1;
    }
    
    /* 定位到文件的开始处 */
    fseek(fd, SEEK_SET, 0);

    send_response(sock_control, 1); 

    memset(data, 0, MAXSIZE);

    /* 通过数据连接，发送tmp.txt 文件的内容 */
    while ((num_read = fread(data, 1, MAXSIZE, fd)) > 0) 
    {
        if (send(sock_data, data, num_read, 0) < 0) 
            perror("err");
    
        memset(data, 0, MAXSIZE);
    }

    fclose(fd);

    send_response(sock_control, 226);    // 发送应答码 226（关闭数据连接，请求的文件操作成功）

    return 0;    
}
/**
 * 响应请求：发送当前所在目录的目录项列表
 * 关闭数据连接
 * 错误返回 -1，正确返回 0
 */
int ftserver_list1(int sock_control,int sock_data, char* dir){
    char data[MAXSIZE];
    char sys_ls[3*MAXSIZE];
    size_t num_read; 
    char path[2*MAXSIZE] = {0};
    if (dir[0]!='/')
    {
        sprintf(path,"%s/%s",root,dir);
    }else{
        sprintf(path,"%s%s",root,dir);
    }
    char *p = "..";
    if(access(path,0)<0 || strstr(path,p)){
        send_response(sock_control, -1); 
        printf("%s 该文件路径不存在\n",path);
        return -1;
    }else{
        send_response(sock_control, 1);
    }   
    FILE * fp;
    sprintf(sys_ls,"ls -l %s | tail -n+2",path);
    fp=popen(sys_ls,"r");
    if (!fp)
    {
        exit(1);
        return -1;
    }
    
    //fgets(buffer,  sizeof(buffer),fp);
    //trimstr(buffer,(int)strlen(buffer));
    /* 定位到文件的开始处 */
    fseek(fp, SEEK_SET, 0);

    send_response(sock_control, 1); 

    memset(data, 0, MAXSIZE);

    /* 通过数据连接，发送tmp.txt 文件的内容 */
    while ((num_read = fread(data, 1, MAXSIZE, fp)) > 0) 
    {
        if (send(sock_data, data, num_read, 0) < 0) 
            perror("err");
    
        memset(data, 0, MAXSIZE);
    }

    pclose(fp);
    send_response(sock_control, 226);    // 发送应答码 226（关闭数据连接，请求的文件操作成功）
    return 0;       
}
/**
 * 创建到客户机的一条数据连接
 * 成功返回数据连接的套接字
 * 失败返回 -1
 */
int ftserver_start_data_conn(int sock_control)
{
    
    char buf[1024];    
    int wait, sock_data;

    if (recv(sock_control, &wait, sizeof wait, 0) < 0 ) 
    {
        perror("Error while waiting");
        return -1;
    }

    
    struct sockaddr_in client_addr;
    socklen_t len = sizeof client_addr;
    getpeername(sock_control, (struct sockaddr*)&client_addr, &len); // 获得与控制套接字关联的外部地址（客户端地址）
    inet_ntop(AF_INET, &client_addr.sin_addr, buf, sizeof(buf));//将数值格式转化为点分十进制的ip地址格式

    /* 创建到客户机的数据连接 */
    if ((sock_data = socket_connect(CLIENT_PORT_ID, buf)) < 0)
        return -1;

    return sock_data;        
}
/**
 * 通过数据套接字发送查看历史记录的结果
 * 控制信息交互通过控制套接字
 * 处理 -n 参数
 * 错误返回 -1， 正确返回0
 */
int ftserver_history(int sock_control, int sock_data, char* filenums,char* guser)
{
    SQLCHAR     hissql[512]={0};
    char     hissql1[512];
    sprintf(hissql1,"select * from SYSDBA.dbhistory where dbuser=?");
    memcpy(hissql,hissql1,(int)strlen(hissql1));
    char * n;
    if ((int)strlen(filenums)>0)
    {
        n = strtok(filenums," ");
    
        if (strcmp(n,"-n")==0)
        {
            char* nums = strtok(NULL," ");
            char     hissql2[512];
            sprintf(hissql2,"SELECT * FROM( SELECT * FROM SYSDBA.dbhistory where dbuser=? ORDER BY idx DESC )WHERE ROWNUM <= %s ORDER BY idx ;",nums);
            memcpy(hissql,hissql2,(int)strlen(hissql2));
        }
    }
    
    //printf("sql is %s\n",hissql);
    
    SQLCHAR     in_dbuser[10] = { 0 };
    SQLLEN      in_dbuser_ind_ptr;
    SQLLEN   out_idx = 0;
    char    out_usr[10];
    char    out_arg [512];
    char    out_code[10];
    char    out_res[10];
    SQLLEN  out_idx_ind = 0;
    SQLLEN  out_usr_ind = 0;
    SQLLEN  out_arg_ind = 0;
    SQLLEN  out_code_ind = 0;
    SQLLEN  out_res_ind = 0;
    char content[552];
    //分配语句句柄
    SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hsmt);

    printf("odbc: select history from table...\n");

    sret = SQLPrepare(hsmt, hissql, SQL_NTS);
    memcpy(in_dbuser, guser, (int)strlen(guser));
    in_dbuser_ind_ptr = (int)strlen(guser); 
    if(0){
        printf("username is:\"%s\"\n",in_dbuser);
        printf("username length is:\"%ld\"\n",in_dbuser_ind_ptr);
    }
    sret = SQLBindParameter(hsmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, sizeof(in_dbuser), 0, in_dbuser, 10, &in_dbuser_ind_ptr);
    sret = SQLExecute(hsmt);

    if (RC_NOTSUCCESSFUL(sret)) {
        /* 数据选取失败! */
        printf( "odbc: select dbhistory from table with bind fail!\n" );
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        exit(0);
    }
    printf("odbc: select dbhistory from table success\n");
    
    SQLBindCol(hsmt, 1, SQL_C_CHAR, out_usr, sizeof(out_usr), &out_usr_ind);
    SQLBindCol(hsmt, 2, SQL_C_SLONG, &out_idx, sizeof(out_idx), &out_idx_ind);
    SQLBindCol(hsmt, 3, SQL_C_CHAR, out_code, sizeof(out_code), &out_code_ind);
    SQLBindCol(hsmt, 4, SQL_C_CHAR, out_arg, sizeof(out_arg), &out_arg_ind);
    SQLBindCol(hsmt, 5, SQL_C_CHAR, out_res, sizeof(out_res), &out_res_ind);
    send_response(sock_control, 1);
    for ( ;;)
    {
        sret = SQLFetchScroll(hsmt,SQL_FETCH_NEXT,0);
        if (sret == SQL_NO_DATA_FOUND)
        {
            break;
        }
        printf("usr=%s,idx = %ld,code=%s,arg=%s,res=%s\n",out_usr, out_idx,out_code,out_arg,out_res);
        
        sprintf(content,"%s %ld %s %s %s",out_usr,out_idx,out_code,out_arg,out_res);
        if (send(sock_data, content, sizeof(content), 0) < 0) 
            perror("err");
        memset(content, 0, 552);
        
    }
     
    printf("odbc: select dbhistory from table end\n");
    send_response(sock_control, 226);    // 发送应答码 226（关闭数据连接，请求的文件操作成功）
    //释放语句句柄
    SQLFreeHandle(SQL_HANDLE_STMT, hsmt);
    return 0;
}
/*获取用户的index*/
int getidx(char* guser)
{
    SQLCHAR     idxsql[]="select max(idx) from SYSDBA.dbhistory where dbuser=?";
    SQLCHAR     in_dbuser[10] = { 0 };
    SQLLEN      in_dbuser_ind_ptr;
    SQLLEN    out_idx = 0;
   
    SQLLEN  out_idx_ind = 0;
   
    //分配语句句柄
    SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hsmt);

    printf("odbc: select idx from table...\n");

    sret = SQLPrepare(hsmt, idxsql, SQL_NTS);
    memcpy(in_dbuser, guser, (int)strlen(guser));
    in_dbuser_ind_ptr = (int)strlen(guser); 
    if(0){
         printf("username is:\"%s\"\n",in_dbuser);
        printf("username length is:\"%ld\"\n",in_dbuser_ind_ptr);
    }
    sret = SQLBindParameter(hsmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, sizeof(in_dbuser), 0, in_dbuser, 10, &in_dbuser_ind_ptr);
    sret = SQLExecute(hsmt);

    if (RC_NOTSUCCESSFUL(sret)) {
        /* 数据选取失败! */
        printf( "odbc: select idx from table with bind fail!\n" );
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        exit(0);
    }
    printf("odbc: select idx from table success\n");
    SQLBindCol(hsmt, 1, SQL_C_SLONG, &out_idx, sizeof(out_idx), &out_idx_ind);
    for ( ;;)
    {
        sret = SQLFetchScroll(hsmt,SQL_FETCH_NEXT,0);
        if (sret == SQL_NO_DATA_FOUND)
        {
            break;
        }
        printf("idx = %ld\n", out_idx);
        
    }
    printf("odbc: select idx from table end\n");
    //释放语句句柄
    SQLFreeHandle(SQL_HANDLE_STMT, hsmt);
    return out_idx;
}
/**
 * 用户资格认证
 * 认证成功返回 1，否则返回 0 
 */
int ftserver_check_user_dm(char*user, char*pass)
{
    SQLCHAR     sql[]="select * from SYSDBA.dbuser where dbuser=?";
    SQLCHAR     in_c1[10] = { 0 };
    SQLLEN      in_c1_ind_ptr;
    
    char    out_c1[10];
    char    out_c2 [10];
    SQLLEN  out_c1_ind = 0;
    SQLLEN  out_c2_ind = 0;
    int auth = 0;
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
    //分配语句句柄
    SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hsmt);

    printf("odbc: select passwd from table...\n");

    sret = SQLPrepare(hsmt, sql, SQL_NTS);
    memcpy(in_c1, user, (int)strlen(user));
    in_c1_ind_ptr = (int)strlen(user); 
    if(0){
          printf("username is:\"%s\"\n",in_c1);
        printf("username length is:\"%ld\"\n",in_c1_ind_ptr);
    }
    sret = SQLBindParameter(hsmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, sizeof(in_c1), 0, in_c1, 10, &in_c1_ind_ptr);
    sret = SQLExecute(hsmt);

    if (RC_NOTSUCCESSFUL(sret)) {
        /* 数据选取失败! */
        printf( "odbc: select from table with bind fail!\n" );
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        exit(0);
    }
    printf("odbc: select passwd from table success\n");
    SQLBindCol(hsmt, 1, SQL_C_CHAR, out_c1, sizeof(out_c1), &out_c1_ind);
    SQLBindCol(hsmt, 2, SQL_C_CHAR, out_c2, sizeof(out_c2), &out_c2_ind);
    for ( ;;)
    {
        sret = SQLFetchScroll(hsmt,SQL_FETCH_NEXT,0);
        if (sret == SQL_NO_DATA_FOUND)
        {
            break;
        }
        printf("c1 = %s, c2 = %s,\n", out_c1, out_c2);
        if ((strcmp(user,out_c1)==0) && (strcmp(pass,out_c2)==0)) 
        {
            printf("账号密码正确！\n");
            
            auth =1;
        }   

        
    }
    printf("odbc: select passwd from table end\n");
    //释放语句句柄
    SQLFreeHandle(SQL_HANDLE_STMT, hsmt);
    /* 断开与数据源之间的连接 */
    SQLDisconnect(hdbc);
    /* 释放连接句柄 */
    SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
    /* 释放环境句柄 */
    SQLFreeHandle(SQL_HANDLE_ENV, henv);
    return auth;
}
/* 用户登录*/
int ftserver_login(int sock_control,char* guser)
{    
    char buf[MAXSIZE];
    char user[MAXSIZE];
    char pass[MAXSIZE];    
    memset(user, 0, MAXSIZE);
    memset(pass, 0, MAXSIZE);
    memset(buf, 0, MAXSIZE);
    
    /* 获得客户端传来的用户名 */
    if ( (recv_data(sock_control, buf, sizeof(buf)) ) == -1) 
    {
        perror("recv error\n"); 
        exit(1);
    }    

    int i = 5;
    int n = 0;
    while (buf[i] != 0) //buf[0-4]="USER"
        user[n++] = buf[i++];
    
    /* 用户名正确，通知用户输入密码 */
    send_response(sock_control, 331);                    
    
    /* 获得客户端传来的密码 */
    memset(buf, 0, MAXSIZE);
    if ( (recv_data(sock_control, buf, sizeof(buf)) ) == -1) 
    {
        perror("recv error\n"); 
        exit(1);
    }
    
    i = 5;
    n = 0;
    while (buf[i] != 0) // buf[0 - 4] = "PASS"
        pass[n++] = buf[i++];
    
    int ret= (ftserver_check_user_dm(user, pass)); // 用户名和密码验证，并返回
    if (ret==1)
    {
        strcpy(guser,user);
        printf("guser is : %s\n",guser);
        return 1;
    }
    return -1;
}

/* 接收客户端的命令并响应，返回响应码 */
int ftserver_recv_cmd(int sock_control, char*cmd, char*arg)
{    
    int rc = 200;
    char buffer[MAXSIZE];
    
    memset(buffer, 0, MAXSIZE);
    memset(cmd, 0, 5);
    memset(arg, 0, MAXSIZE);
        
    /* 接受客户端的命令 */
    if ((recv_data(sock_control, buffer, sizeof(buffer)) ) == -1) 
    {
        perror("recv error\n"); 
        return -1;
    }
    
    /* 解析出用户的命令和参数 */
    strncpy(cmd, buffer, 4);
    char *tmp = buffer + 5;
    strcpy(arg, tmp);
    
    if (strcmp(cmd, "QUIT")==0) 
        rc = 221;

    else if ((strcmp(cmd, "UPLD")==0)|| (strcmp(cmd, "LIST") == 0) || (strcmp(cmd, "DOWN") == 0)|| (strcmp(cmd, "DELE") == 0)|| (strcmp(cmd, "HIST") == 0))
        rc = 200;

    else 
        rc = 502; // 无效的命令

    send_response(sock_control, rc);    
    return rc;
}
/* 将用户操作记录存储到表中
*/
int storehis(char* guser,int idx,char* code,char* arg,char* res)
{
    SQLCHAR     idxsql[]="insert into SYSDBA.dbhistory values(?,?,?,?,?)";
    SQLCHAR     in_dbuser[10] = { 0 };
    SQLLEN      in_dbuser_ind_ptr;

    SQLCHAR     in_dbarg[512] = { 0 };
    SQLLEN      in_dbarg_ind_ptr;
    SQLCHAR     in_dbcode[10] = { 0 };
    SQLLEN      in_dbcode_ind_ptr;
    SQLLEN      in_dbidx = idx;

    SQLCHAR     in_dbres[10] = {0};
    SQLLEN      in_dbres_ind_ptr;
    //分配语句句柄
    SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hsmt);

    printf("odbc: insert into SYSDBA.dbhistory ...\n");

    sret = SQLPrepare(hsmt, idxsql, SQL_NTS);
    memcpy(in_dbuser, guser, (int)strlen(guser));
    in_dbuser_ind_ptr = (int)strlen(guser); 

    memcpy(in_dbarg, arg, (int)strlen(arg));
    in_dbarg_ind_ptr = (int)strlen(arg);  
    memcpy(in_dbcode, code, (int)strlen(code));
    in_dbcode_ind_ptr = (int)strlen(code);
    memcpy(in_dbres, res, (int)strlen(res));
    in_dbres_ind_ptr = (int)strlen(res);
    if (0)
    {
        printf("username is:\"%s\"\n",in_dbuser);
        printf("username length is:\"%ld\"\n",in_dbuser_ind_ptr);
        printf("idx is:\"%ld\"\n",in_dbidx);
        printf("idx length is:\"%ld\"\n",sizeof(in_dbidx));

        printf("code is:\"%s\"\n",in_dbcode);
        printf("code length is:\"%ld\"\n",sizeof(in_dbcode_ind_ptr));
        printf("arg is:\"%s\"\n",in_dbarg);
        printf("arg length is:\"%ld\"\n",in_dbarg_ind_ptr);
        printf("res is:\"%s\"\n",in_dbres);
        printf("res length is:\"%ld\"\n",in_dbres_ind_ptr);
    }
    sret = SQLBindParameter(hsmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, sizeof(in_dbuser), 0, in_dbuser, 0, &in_dbuser_ind_ptr);
    sret = SQLBindParameter(hsmt, 2, SQL_PARAM_INPUT, SQL_C_SLONG,  SQL_INTEGER, sizeof(in_dbidx), 0, &in_dbidx, sizeof(in_dbidx), NULL);

    sret = SQLBindParameter(hsmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, sizeof(in_dbcode), 0, in_dbcode, 0, &in_dbcode_ind_ptr);
    sret = SQLBindParameter(hsmt, 4, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, sizeof(in_dbarg), 0, in_dbarg, 0, &in_dbarg_ind_ptr);
    sret = SQLBindParameter(hsmt, 5, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, sizeof(in_dbres), 0, in_dbres, 0, &in_dbres_ind_ptr);
    sret = SQLExecute(hsmt);

    if (RC_NOTSUCCESSFUL(sret)) {
        /* 数据插入失败! */
        printf( "odbc: history insert into table with bind fail!\n" );
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        exit(0);
    }
    printf("odbc: history insert into table success\n");
    //释放语句句柄
    SQLFreeHandle(SQL_HANDLE_STMT, hsmt);
    return 0;

}

/*更新操作记录结果，failed or successed*/
int updatehis(char* guser,int idx,char* res){
    SQLCHAR     updsql[] = "update dbhistory set res = ? where idx=? and dbuser=?";
    SQLCHAR     in_dbuser[10] = { 0 };
    SQLLEN      in_dbuser_ind_ptr;
    SQLLEN      in_dbidx = idx;

    SQLCHAR     in_dbres[10] = {0};
    SQLLEN      in_dbres_ind_ptr;

    //分配语句句柄
    SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hsmt);

    printf("odbc: update res in SYSDBA.dbhistory ...\n");

    sret = SQLPrepare(hsmt, updsql, SQL_NTS);
    memcpy(in_dbuser, guser, (int)strlen(guser));
    in_dbuser_ind_ptr = (int)strlen(guser); 
    memcpy(in_dbres, res, (int)strlen(res));
    in_dbres_ind_ptr = (int)strlen(res);

    sret = SQLBindParameter(hsmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, sizeof(in_dbres), 0, in_dbres, 0, &in_dbres_ind_ptr);
    sret = SQLBindParameter(hsmt, 2, SQL_PARAM_INPUT, SQL_C_SLONG,  SQL_INTEGER, sizeof(in_dbidx), 0, &in_dbidx, sizeof(in_dbidx), NULL);
    sret = SQLBindParameter(hsmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, sizeof(in_dbuser), 0, in_dbuser, 0, &in_dbuser_ind_ptr);

    sret = SQLExecute(hsmt);
    if (RC_NOTSUCCESSFUL(sret)) {
        /* 数据插入失败! */
        printf( "odbc: update res in SYSDBA.dbhistory\n" );
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        exit(0);
    }
    printf("odbc: update res in SYSDBA.dbhistory success\n");
    //释放语句句柄
    SQLFreeHandle(SQL_HANDLE_STMT, hsmt);
    return 0;
}
/* 处理客户端请求 */
void ftserver_process(int sock_control)
{
    printf("this sock_control is:%d\n",sock_control);
    char  guser[10];//用于存储同户名
    int sock_data;
    char cmd[5];
    char arg[MAXSIZE];
    char code[10];
    send_response(sock_control, 220); // 发送欢迎应答码

    /* 用户认证 */
    if (ftserver_login(sock_control,guser) == 1)  // 认证成功
        send_response(sock_control, 230);
    else 
    {
        send_response(sock_control, 430);    // 认证失败
        exit(0);
    }  

    /*限制ftserver访问目录，初始化root*/
    init_root();  
    /*
    为了存储history，
    要与数据库建立连接：
    */
    initsql();
    /* history的id */
    /* 处理用户的请求 */
    int index = -1;
    while (1) 
    {
        /* 接收命令，并解析,获得命令和参数 */
        int rc = ftserver_recv_cmd(sock_control, cmd, arg);
        //存储命令到dbhistory中
        index = getidx(guser)+1;
        if(strcmp(cmd,"LIST")==0){
            strcpy(code, "ls");
        }
        else if (strcmp(cmd,"HIST")==0)
        {
            strcpy(code, "history");
        }
        else if (strcmp(cmd,"DELE")==0)
        {
            strcpy(code, "delete");
        }
        else if (strcmp(cmd,"UPLD")==0)
        {
            strcpy(code, "upload");
        }
        else if (strcmp(cmd,"DOWN")==0)
        {
            strcpy(code, "download");
        }
        else if (strcmp(cmd,"QUIT")==0)
        {
            strcpy(code, "quit");
        }
        printf("%s %d 执行命令为：\"%s\" \"%s\"  \n",guser,index,code,arg);
        
        
        if ((rc < 0) || (rc == 221))  // 用户输入命令 "QUIT"
            break;
        
        if (rc == 200 ) 
        {
            /* 创建和客户端的数据连接 */
            if ((sock_data = ftserver_start_data_conn(sock_control)) < 0) 
            {
                close(sock_control);
                
                exit(1); 
            }
            printf("this sock_data is:%d\n",sock_data);
            storehis(guser,index,code,arg,"start");
            /* 执行指令 */
            int resflag =-1;
            if (strcmp(cmd, "LIST")==0) 
                resflag=ftserver_list1(sock_control,sock_data, arg);
            
            else if (strcmp(cmd, "DOWN")==0) 
                resflag=ftserver_dwld(sock_control, sock_data, arg);

            else if(strcmp(cmd, "UPLD")==0)
                {
                    resflag=ftserver_upload(sock_control,sock_data,arg);
                }
                   
            else if(strcmp(cmd, "DELE")==0)
                resflag=ftserver_delete(sock_control,sock_data,arg);

            else if(strcmp(cmd, "HIST")==0)
                resflag=ftserver_history(sock_control,sock_data,arg,guser);
            if (resflag==0)
            {
                updatehis(guser,index,"successed");
            }else{
                updatehis(guser,index,"failed");
            }     
            close(sock_data);// 关闭数据连接
            
        } 
        
    }
    freesql();
    
}