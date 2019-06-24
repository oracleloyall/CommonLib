/*
 *  Disign for udp server with epoll
 *
 *  Created on: Mar 24, 2017
 *      Author: zhaoxi
 */
#include <stdlib.h>
#include <stdio.h>
#include "linux/net/udp_peer.h"
#include "linux/net/log.h"
#include "define.h"
#include "pool/thpool.h"
#include "hashmap/hashmap.h"
#include "function/function.h"
#include <setjmp.h>
#ifdef TRANS
UDP packet;
unsigned long syn=0;
extern map_t timer;
RTT *info;
#endif
static loop_t *g_loop;
udp_peer_t *g_udp_peer;
#ifdef TRANS
#include"rto/rto.h"
int tim=0;
sigjmp_buf jmp_env;
static void connect_alarm(int err)
{
    siglongjmp(jmp_env, 1);
}
#endif
static inetaddr_t g_remote_addr;
static threadpool thpool;
struct sockaddr_in addr;
static 
void on_message(udp_peer_t *udp_peer, void *message, unsigned size, void* userdata, const inetaddr_t *peer_addr)
{
#ifdef DEBUG
	log_debug("has message :%s %d",peer_addr->ip,peer_addr->port);
#endif
	head_check(udp_peer,message, size, userdata, peer_addr);
}
static
void task1()
{
	sleep(5);
    while(1)
    {
		UDP udp;
		memset(&udp, 0, sizeof(UDP));
		udp.type = 0x01;
		udp.version = 0x01;
		udp.company = 0x04;
		memcpy(udp.hmac, "zx", 2);
		udp.sn = htonl(0);
		udp.action = 0x01;
		udp.para = 0;
		udp.len = 0;
		char buf[1048] = { 0 };
		memcpy(buf, &udp, sizeof(UDP));
		udp_peer_send2(g_udp_peer, buf, sizeof(udp), &addr);
		sleep(3);
    }
}

int main(int argc, char *argv[])
{
	log_setlevel(LOG_LEVEL_DEBUG);
	thpool = thpool_init(1);
	if (NULL == thpool) {
			//PLOG_ERROR("Init pool error\n");
			return 0;
	}
	thpool_add_work(thpool, (void*) task1, NULL);
	inetaddr_initbyipport(&g_remote_addr, "127.0.0.1", 5588);
	g_loop = loop_new(5);
	g_udp_peer = udp_peer_new(g_loop, "0.0.0.0", on_message, NULL, NULL);
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	//addr.sin_port = htons(5588);
	//init_key();
	init_register();

#ifdef TRANS
	info=(RTT*)malloc(sizeof(RTT));
	rtt_init(info);
	UDP udp;
	memset(&udp, 0, sizeof(UDP));
	udp.type = 0x01;
	udp.version = 0x01;
	udp.company = 0x04;
	memcpy(udp.hmac, "zx", 2);
	udp.sn = htonl(0);
	udp.action = 0x01;
	udp.para = 0;
	udp.len = 0;
	char buf[1048] = { 0 };
	memcpy(buf, &udp, sizeof(UDP));
	udp_peer_send2(g_udp_peer, buf, sizeof(udp), &addr);
        memcpy(&packet,&udp,sizeof(UDP));
        TRN *value = malloc(sizeof(CALLBACK));
        value->type = syn;
        memcpy(&value->udp,&udp,sizeof(UDP));
        hashmap_put(mymap, value->type, value);
        signal(SIGALRM, connect_alarm);
        alarm(1);
        if(sigsetjmp(jmp_env, 1)){/*由信号处理函数返回时*/
    	printf("enter\n");
		/*超时处理*/
		if (tim++ > 3) {
			alarm(0);
			return -1;
		} else {
			/*添加指数回退*/
			log_debug("time out send again\n");
			memcpy(buf, &packet, sizeof(UDP));
			udp_peer_send2(g_udp_peer, buf, sizeof(udp), &addr);
			alarm(0);
		}
       }
#endif
	loop_loop(g_loop);
printf("%s:%d\n",__FILE__,__LINE__);
	udp_peer_destroy(g_udp_peer);
printf("%s:%d\n",__FILE__,__LINE__);
	loop_destroy(g_loop);
    return 0;
}
