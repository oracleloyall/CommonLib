#ifdef TT
#include "epoll/udp_peer.h"
#include"define.h"
#include "epoll/log.h"
#include <stdlib.h>
#include <stdio.h>
#if 1
static loop_t *g_loop;
static udp_peer_t *g_udp_peer;
static inetaddr_t g_remote_addr;
long num=0;
static 
void on_message(udp_peer_t *udp_peer, void *message, unsigned size, void* userdata, const inetaddr_t *peer_addr)
{
	printf("hash message :%s %d\n\n",peer_addr->ip,peer_addr->port);
	int ret=udp_peer_send(udp_peer, message,1024, peer_addr);
	//获取type
	//回调-》head校验
	//Head校验-》分发（传输的多线程处理，其他当前进程处理）
    return;
}

static char data[4096];
static 
void on_writable(udp_peer_t *udp_peer, void* userdata)
{

   // udp_peer_send(g_udp_peer, data, sizeof(data), &g_remote_addr);
	udp_peer_send(udp_peer, data, sizeof(data), &g_remote_addr);//
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("usage: %s <remote ip>\n", argv[0]);
        return 0;
    }
    log_setlevel(LOG_LEVEL_DEBUG);

    inetaddr_initbyipport(&g_remote_addr, argv[1], 5588);
    g_loop = loop_new(1);
    g_udp_peer = udp_peer_new(g_loop, "0.0.0.0", 5588, on_message, NULL, NULL);
    loop_loop(g_loop);
    udp_peer_destroy(g_udp_peer);
    loop_destroy(g_loop);
    return 0;
}
#endif
#endif
