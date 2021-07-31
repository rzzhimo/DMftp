/*
ftserver.h
*/
#ifndef FTSERVER_H
#define FTSERVER_H

#include "../common/common.h"
char root[MAXSIZE];
int init_root();
int ftserver_dwld(int sock_control, int sock_data, char* filename);

int ftserver_upload(int sock_control, int sock_data, char* filename);

int ftserver_list(int sock_control,int sock_data, char* dir);
int ftserver_list1(int sock_control,int sock_data, char* dir);
int ftserver_start_data_conn(int sock_control);

int ftserver_check_user_dm(char*user, char*pass);

int ftserver_login(int sock_control,char *guser);

int ftserver_recv_cmd(int sock_control, char*cmd, char*arg);

void ftserver_process(int sock_control);

int ftserver_delete(int sock_control, int sock_data, char* filename);

int ftserver_history(int sock_control, int sock_data, char* filenums,char* guser);

int getidx(char* guser);
int storehis(char* guser,int idx,char* code,char* arg,char* res);
int updatehis(char* guser,int idx,char* res);
#endif