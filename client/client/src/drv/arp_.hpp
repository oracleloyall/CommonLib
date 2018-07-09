/*
 * ack.hpp
 *
 *  Created on: 2015-9-15
 *   Author: zhaoxi
 */
/*
 * ack.hpp
 *
 *  Created on: 2015-9-15
 *   Author: zhaoxi
 */
#ifndef _ARP_HPP_
#define _ARP_HPP_
#include<iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "time.h"
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include  <list>
#include <algorithm>
using namespace std;
#include "../proj/ap_type2s.hpp"
//#include "ap_type2s.hpp"
#define DEFAULT_ARP  "/proc/net/arp"
typedef struct Arp_accept
{
	__u8	arp_mac[6];     //mac
	__u8	arp_ip[16];		// ip
	__u8    state ;//1上线，2认证，3下线
	__u8    oldstate;
    unsigned int update_time;//更新的时间
}arp_accept;

class con_collect
{
public:
	explicit con_collect(const char * name);
	~con_collect();
	void Cpy(unsigned char *p,unsigned char *s);
	int get_mtime();
	void print_arp(const char* ifname);
	void print_arp1(const char *ip , const char *type ,const  char *Flags , char *mac ,const char *mask ,
			const char *device );//添加新的元素
	void print_arp2(const char *device);//去读配置文件
	void write_mac(unsigned char* buf, string mac);
	string  read_mac(unsigned char * buf);
	void remove(string &str);
	int HexToInt(char c);
	void write_ip(unsigned char* buf, string ip);
	int StrToInt(const string s);
	void update_list(const char* ifname);
	void del_list();
	void up_stat(const char *ifname,string &mac,unsigned char status);
	void up_stat1(const char *ifname,unsigned char *mac,unsigned char status);
	string IntToStr(const int n);
	string read_ip(unsigned char * buf);
	typedef list<arp_accept* > ArpContainer;
	typedef ArpContainer::iterator Arp_iterator;
	Arp_iterator arp_iterator;
	list<arp_accept* > p_arp;
private:
	con_collect(const con_collect&);
	con_collect& operator=(const con_collect&);
};
#endif

