
#ifndef UDP_PEER_H
#define UDP_PEER_H

struct udp_peer;
typedef struct udp_peer udp_peer_t;

#include "loop.h"
#include "inetaddr.h"

#include <netinet/in.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*on_message_f)(udp_peer_t *peer, void *message, unsigned size, void* userdata, const inetaddr_t *peer_addr);

typedef void (*on_writable_f)(udp_peer_t *peer, void* userdata);

udp_peer_t* udp_peer_new(loop_t *loop, const char *ip, on_message_f messagecb, on_writable_f writecb, void *userdata);

unsigned short udp_peer_getport(udp_peer_t* peer);

on_message_f udp_peer_onmessage(udp_peer_t* peer, on_message_f messagecb, void *userdata);

on_writable_f udp_peer_onwrite(udp_peer_t* peer, on_writable_f writecb, void *userdata);

void udp_peer_destroy(udp_peer_t* peer);


int udp_peer_send(udp_peer_t* peer, const void *message, unsigned len, const inetaddr_t *peer_addr);
int udp_peer_send2(udp_peer_t* peer, const void *message, unsigned len, const struct sockaddr_in *peer_addr);


void udp_peer_expand_send_buffer(udp_peer_t* peer, unsigned size);
void udp_peer_expand_recv_buffer(udp_peer_t* peer, unsigned size);

#ifdef __cplusplus
}
#endif

#endif /* !UDP_PEER_H */
