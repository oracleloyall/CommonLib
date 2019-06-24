/*
 * Callback.h
 *
 *  Created on: Mar 1, 2017
 *      Author: oracle
 */

#ifndef CALLBACK_H_
#define CALLBACK_H_
#include<sys/types.h>
#include<sys/stat.h>
#include<openssl/ec.h>
#include"define.h"
#include"packet.h"
#include"hashmap.h"
#ifdef __cplusplus
extern "C" {
#endif


#define CHECKHEAD 0

extern unsigned char hmac_key[];
extern EVP_PKEY *sigkey;

//register function
typedef union{
	int (*check)(unsigned char *buff,struct sockaddr_in *from);
	int (*function)(int type,unsigned char *buff,struct sockaddr_in *from);
}FUNC;
typedef struct call
{
	int type;
	//int (*fun)(int type,unsigned char *buff,struct sockaddr_in *from);
    FUNC func;
}CALLBACK;
int  init_register();
//head check
int head_check(unsigned char *buff,struct sockaddr_in *from);
//function time return
int time_funciton(int type,unsigned char *buff,struct sockaddr_in *from);
int state_funciton(int type,unsigned char *buff,struct sockaddr_in *from);
int negotiate_funciton(int type,unsigned char *buff,struct sockaddr_in *from);
int restart_funciton(int type,unsigned char *buff,struct sockaddr_in *from);
int devinfo_funciton(int type,unsigned char *buff,struct sockaddr_in *from);
int sysinfo_function(int type,unsigned char *buff,struct sockaddr_in *from);
int set_sysinfo_function(int type,unsigned char *buff,struct sockaddr_in *from);
//update
int start_response(int type,unsigned char *buff,struct sockaddr_in *from);
int trans_data(int type,unsigned char *buff,struct sockaddr_in *from);
int end_response(int type,unsigned char *buff,struct sockaddr_in *from,unsigned long size);
int end_res(int type,unsigned char *buff,struct sockaddr_in *from);
int RegisterCommandHandler(int type);

//send
int send_restart(int ip);
int send_devinfo(int ip);
int send_sysinfo(int ip);
int send_set_sysinfo(int ip,char *buff);
int send_upgrade(int ip);
//int send_negotiate(int ip);

unsigned long get_file_size(const char *filename);
unsigned long get_file_pos(const  FILE * file,unsigned int size);
//unsigned long get_file_size(const char *filename);

//other function

int load_cfg(const unsigned char *config);
void task4();//check timer for trans

#ifdef __cplusplus
}
#endif
#endif /* CALLBACK_H_ */
