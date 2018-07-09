/*
 * wdctl.cpp
 *
 *  Created on: 2015骞�9鏈�23鏃�
 *      Author: sw
 */

#include<iostream>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/un.h>
#include <unistd.h>
#include <syslog.h>
#include <errno.h>
#include <net/if.h>

#include <fcntl.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netpacket/packet.h>
#include <arpa/inet.h>

#include "wdctl.hpp"
#include "../hlib/logout.hpp"



int connect_to_server(const char *sock_name)
{
    int sock;
    struct sockaddr_un sa_un;

    /* Connect to socket */
    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock < 0)
    {
    	logout("Get Unix Socket Error\n");
    	return -1;
    }
    memset(&sa_un, 0, sizeof(sa_un));
    sa_un.sun_family = AF_UNIX;
    strncpy(sa_un.sun_path, sock_name, (sizeof(sa_un.sun_path) - 1));

    if (connect(sock, (struct sockaddr *)&sa_un, strlen(sa_un.sun_path) + sizeof(sa_un.sun_family)))
    {
 //   	logout("Connecting to Unix server is error\n");
    	close(sock);
    	return -1;
    }

    return sock;
}
size_t send_request(int sock, const char *request)
{
    size_t len;
    ssize_t written;
    len = 0;
    while (len != strlen(request)) {
        written = write(sock, (request + len), strlen(request) - len);
        if (written == -1) {
        	logout("Write %s to Unix Server error\n",request);
        	close(sock);
        	return -1;
        }
        len += (size_t) written;
    }
    return len;
}
/*
 * 姝ゅ嚱鏁板悜wifi-dog鍑芥暟璇曟帰鏄惁devid鍙戠敓鏀瑰彉,濡傛灉鍙戠敓鏀瑰彉浼氳繑鍥炴渶鏂扮殑dev_id,娌℃湁鏀瑰彉浼氭敹鍒板瓧绗︿覆no
 */
char* wdctl_getdevid()
{
    int sock;
    char buffer[64];
    char request[16];
    ssize_t len = 0;
    int size = 0;

    bzero(buffer,64);
    sock = connect_to_server(DEFAULT_SOCK);
    if(sock < 0){
//    	logout("connect server has something error\n");
    	return NULL;
    }

    strncpy(request, "devid\r\n\r\n", 15);

    if(send_request(sock, request) < 0){
 //   	logout("send %s to server error\n",request);
    	close(sock);
    	return NULL;
    }
//    logout("send cmd  devid to server suceeful\n");
    while ((len = read(sock, buffer + size, 63)) > 0) {
    	size += len;
    }
    buffer[size] = '\0';
    shutdown(sock, 2);
    close(sock);

    return size > 0 ? strdup(buffer): NULL;
}
int wdctl_OnPowr()
{
    int sock;
    char request[16];

    sock = connect_to_server(DEFAULT_SOCK);
    if(sock < 0){
    	logout("connect server has something error\n");
    	return -1;
    }

    strncpy(request, "onpower\r\n\r\n", 15);

    if(send_request(sock, request) < 0){
    	logout("send %s to server error\n",request);
    	close(sock);
    	return -2;
    }
    logout("send cmd onpower to server suceeful\n");

    shutdown(sock, 2);
    close(sock);
    return 0;
}
int wdctl_down()
{
    int sock;
    char request[16];

    sock = connect_to_server(DEFAULT_SOCK);
    if(sock < 0){
    	logout("connect server has something error\n");
    	return -1;
    }

    strncpy(request, "down\r\n\r\n", 15);

    if(send_request(sock, request) < 0){
    	logout("send %s to server error\n",request);
    	close(sock);
    	return -2;
    }
    logout("send cmd down to server suceeful\n");

    shutdown(sock, 2);
    close(sock);
    return 0;
}

char *get_iface_mac(const char *ifname)
{
#ifdef WIFI
    int r, s;
    struct ifreq ifr;
    char *hwaddr, mac[13];

    strncpy(ifr.ifr_name, ifname, 15);
    ifr.ifr_name[15] = '\0';

    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == s) {
        logout("get_iface_mac socket: %s", strerror(errno));
        return NULL;
    }

    r = ioctl(s, SIOCGIFHWADDR, &ifr);
    if (r == -1) {
        logout("get_iface_mac ioctl(SIOCGIFHWADDR): %s", strerror(errno));
        close(s);
        return NULL;
    }

    hwaddr = ifr.ifr_hwaddr.sa_data;
    close(s);
    snprintf(mac, sizeof(mac), "%02X%02X%02X%02X%02X%02X",
             hwaddr[0] & 0xFF,
             hwaddr[1] & 0xFF, hwaddr[2] & 0xFF, hwaddr[3] & 0xFF, hwaddr[4] & 0xFF, hwaddr[5] & 0xFF);

    return strdup(mac);
#else
	return NULL;
#endif
}

char *get_iface_ip(const char *ifname)
{
    struct ifreq if_data;
    struct in_addr in;
    char *ip_str;
    int sockd;
    u_int32_t ip;

    /* Create a socket */
    if ((sockd = socket(AF_INET, SOCK_RAW, htons(0x8086))) < 0) {
        logout("socket(): %s", strerror(errno));
        return NULL;
    }

    /* Get IP of internal interface */
    strncpy(if_data.ifr_name, ifname, 15);
    if_data.ifr_name[15] = '\0';

    /* Get the IP address */
    if (ioctl(sockd, SIOCGIFADDR, &if_data) < 0) {
        logout("ioctl(): SIOCGIFADDR %s", strerror(errno));
        close(sockd);
        return NULL;
    }
    memcpy((void *)&ip, (void *)&if_data.ifr_addr.sa_data + 2, 4);
    in.s_addr = ip;

    ip_str = inet_ntoa(in);
    close(sockd);
    return strdup(ip_str);
}
