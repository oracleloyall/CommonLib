#ifndef DEFINE_H_
#define DEFINE_H_
#define SECRET
#include"log/plog.h"
//#define printf PLOG_DEBUG
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <sys/un.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/syslog.h>
#include <sys/wait.h>
#include <time.h>
#include <asm/byteorder.h>
#include "openssl/sm3.h"
#include "openssl/evp.h"
#include  "param.h"
#include "epoll/log.h"
#ifdef __cplusplus
extern "C" {
#endif


//#define printf log_debug
#define TEST
//#define TRANS
#define TU

//0x01 data
typedef struct device_time
{
	int time;
}DEV_TIME;
//0x02 data
typedef struct device_rand
{
  unsigned char data[1024];
}DEV_RAND;
//
//设备基本信息结构体
typedef struct device_info
{
	unsigned char dev_name[40];
	unsigned int dev_version;
	unsigned int  dev_type;
	unsigned int  dev_factory;
	unsigned int  dev_runtime;
}DEV_INFO;
//系统参数结构体
//#define
typedef struct device_system
{
	unsigned char  sys_type;//设备类型，三种
	unsigned char sys_policy[50];
	unsigned char sys_net[50];
	unsigned char sys_serial[50];
	unsigned char sys_dial[50];
	unsigned char sys_101TCP[50];
}DEV_SYS;
typedef struct
{
	unsigned char devini[1024];
}DEVSYS;
// 终端设备map表结构体
typedef struct device
{
	unsigned int remote_ip;
	struct sockaddr_in addr;
	unsigned int sn;
	unsigned inuse:1;
	unsigned online:1;
	unsigned mark:1;
	#ifdef SECRET
	char key[16];
	char iv[16];
	char hmac_key[32];
	void *eckey;
	int old_time;//生成秘钥的当前秒数
    DEV_INFO info;
    DEV_SYS sys;
	#endif
}DEV;

//发起协商
//请求报文
typedef struct send_ike
{
	unsigned char data[0];
}SNDIKE;
//应答报文
typedef struct recv_ike
{
	unsigned char data[0];
}RCVIKE;
//发起升级命令
//请求报文
typedef struct request_msg
{
    int len;
    unsigned char msg[32];
}REQMSG;
//应答报文
typedef struct response_msg
{
   int recv_len;
   int max_len;
}RSPMSG;
//升级文件传输命令
//请求报文
typedef struct send_packet
{
  int digest;//摘要
  int offset;//偏移
  int len;//
  unsigned char packet[1000];
}SEDPAC;
//应答报文
typedef struct res_packet
{
    int digest;
    int offset;
    int len;
}RESPAC;
//0x01
typedef struct
{
	int time;
}TIMEPAC;
//0x02
typedef struct
{
	unsigned char rand[1024];
}RANDPAC;
//结束升级命令同 request_msg
typedef union
{
    SNDIKE sndike;//0x03
	RCVIKE rcvike;//0x03
    REQMSG reqmsg;//0x09 0x0A
    RSPMSG rsqmsg;//0x09
    SEDPAC senpac;//0x08
    RESPAC respac;//0x08;
    TIMEPAC timepac;//0x01
    RANDPAC randpac;//0x02
    DEV_INFO devinfo;
    DEVSYS   devsys;
}UPGRADE;

typedef struct dms_update
{
#if defined(__LITTLE_ENDIAN_BITFIELD)
	__u8 	type:4,
		version:4;
#elif defined(__BIG_ENDIAN_BITFIELD)
	__u8 	version:4,
		type:4;
#else
#error	"Please fix <asm/byteorder.h>"
#endif
	__u8 company;
	__u8 hmac[2];
	__u32 sn;
	__u8 action;
	__u8 para;
	__u16 len;
    UPGRADE packet;
}UDP __attribute__((packed));

typedef struct
{
	//map_t mymap;
	char key_string[16];
	struct sockaddr_in Ip;
	unsigned char state;//1 start  2 break  3 error 4 success
    unsigned int ip;
    unsigned long time;
    unsigned int len;//bone
    unsigned long send_size;
    unsigned long total_size;
    unsigned long offset;
    unsigned char file[50];
}UPDEV_INFO;

//timer data
typedef struct
{
   unsigned char type;//function type
   UDP udp;
   struct sockaddr_in in;
}TM_DATA;//balloc

typedef struct
{
	unsigned int ip;
	struct timer *t;
}TM_MAP;
//


int read_cfg(const unsigned char *config);
int init();
int send_packet(int fd, struct sockaddr *to,int type);//
DEV *find_device(int ip);
unsigned int sn_ret(int ip);
void sn_set(int ip,unsigned int num);
int device_insert(DEV *pt);
void device_print();
void update_key();
int check_param(int param,int ip,int type);

#ifdef __cplusplus
}
#endif
#endif

