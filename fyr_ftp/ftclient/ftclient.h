/*
ftclient.h
*/
#ifndef FTCLIENT_H
#define FTCLIENT_H

#include "../common/common.h"

int read_reply();

void print_reply(int rc);

int ftclient_read_command(char* buf, int size, struct command *cstruct);

int ftclient_dwld(int data_sock, int sock_control, char* arg);

int ftclient_upload(int data_sock, int sock_control, char* arg);

int ftclient_open_conn(int sock_con);

int ftclient_list(int sock_data, int sock_con);

int ftclient_send_cmd(struct command *cmd);

void ftclient_login();

int ftclient_hist(int data_sock, int sock_control);

int up_check_path(char *arg);

#endif