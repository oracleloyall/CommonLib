/*
 * function.h
 *
 *  Created on: Mar 24, 2017
 *      Author: oracle
 */

#ifndef FUNCTION_FUNCTION_H_
#define FUNCTION_FUNCTION_H_

#include "../packet.h"
#include "../define.h"
#include "../cert/crypt.h"
#include "../hashmap/hashmap.h"

#define CHECKHEAD 0
//register function
typedef union{
	int (*check)(unsigned char *buff,struct sockaddr_in *from);
	int (*function)(udp_peer_t *udp_peer, void *message, unsigned size, void* userdata, const inetaddr_t *peer_addr);
}FUNC;
typedef struct call
{
	int type;
	//int (*fun)(int type,unsigned char *buff,struct sockaddr_in *from);
    FUNC func;
}CALLBACK;
int  init_register();
void print_hex(unsigned char *buf, int len);
//head check
int head_check(udp_peer_t *udp_peer, void *message, unsigned size, void* userdata, const inetaddr_t *peer_addr);
//function time return
int time_funciton(udp_peer_t *udp_peer, void *message, unsigned size, void* userdata, const inetaddr_t *peer_addr);
int state_funciton(udp_peer_t *udp_peer, void *message, unsigned size, void* userdata, const inetaddr_t *peer_addr);
int negotiate_funciton(udp_peer_t *udp_peer, void *message, unsigned size, void* userdata, const inetaddr_t *peer_addr);
int restart_funciton(udp_peer_t *udp_peer, void *message, unsigned size, void* userdata, const inetaddr_t *peer_addr);
int devinfo_funciton(udp_peer_t *udp_peer, void *message, unsigned size, void* userdata, const inetaddr_t *peer_addr);
int sysinfo_function(udp_peer_t *udp_peer, void *message, unsigned size, void* userdata, const inetaddr_t *peer_addr);
int set_sysinfo_function(udp_peer_t *udp_peer, void *message, unsigned size, void* userdata, const inetaddr_t *peer_addr);
//update
int start_response(udp_peer_t *udp_peer, void *message, unsigned size, void* userdata, const inetaddr_t *peer_addr);
int trans_data(udp_peer_t *udp_peer, void *message, unsigned size, void* userdata, const inetaddr_t *peer_addr);
int end_response(udp_peer_t *udp_peer, void *message, unsigned size, void* userdata, const inetaddr_t *peer_addr);
int end_res(udp_peer_t *udp_peer, void *message, unsigned size, void* userdata, const inetaddr_t *peer_addr);
int init_register() ;
//test
int test_funciton(udp_peer_t *udp_peer, void *message, unsigned size, void* userdata, const inetaddr_t *peer_addr);
#endif /* FUNCTION_FUNCTION_H_ */
