/*
 * main.c
 *
 *  Created on: 2015年10月9日
 *      Author: sw
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <stdarg.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "randommac.hpp"

#if 1
#define DEFAULT_PATH "/etc/config/telin"
#else
#define DEFAULT_PATH "/home/sw/workspace/MacApi/telin"
#endif
//#define H_DEBUG
#define DEAULT_INTERFACE "lan"
//#define 	ReadOutLine

void debug(const char *format, ...)
{
#ifdef H_DEBUG
    char buf[1024] = "";
    va_list vlist;
    time_t ts;

    time(&ts);
	struct tm *ptm = localtime(&ts);
	if(!ptm)
	{
		return;
	}
	snprintf(buf,1024,"%02d:%02d:%02d [sw] %s\n",ptm->tm_hour,ptm->tm_min,ptm->tm_sec,format);

	va_start(vlist,(const char*)buf);
	vprintf((const char*)buf, vlist);
	va_end(vlist);
#endif
}

char* NowLocalTime(void)
{
	char buf[128] = "";
    time_t ts;
    time(&ts);
	struct tm *ptm = localtime(&ts);
	if(!ptm)
	{
		return NULL;
	}
	snprintf(buf,128,"%04d-%02d-%02d %02d:%02d:%02d",
			ptm->tm_year + 1900,ptm->tm_mon + 1,ptm->tm_mday,ptm->tm_hour,ptm->tm_min,ptm->tm_sec);
	return strdup(buf);
}

static int HexToInt(char c)
{
   if (c >= '0' && c <= '9') return (c - '0');
   if (c >= 'A' && c <= 'F') return (c - 'A' + 10);
   if (c >= 'a' && c <= 'f') return (c - 'a' + 10);
   return 0;
}

void MacStrToInt(const char* Mac,INT8U* UMac)
{
	int i = 0;
	bzero(UMac,6);
	for(i = 0; i < 6; i++)
	{
		*(UMac+i) = HexToInt(Mac[i * 3]) * 16 + HexToInt(Mac[i * 3 + 1]);
	}
}
void MacIntToStr(const INT8U* Mac,char* MacStr)
{
	sprintf(MacStr,"%02x:%02x:%02x:%02x:%02x:%02x",
			(INT8U)Mac[0],(INT8U)Mac[1],(INT8U)Mac[2],(INT8U)Mac[3],(INT8U)Mac[4],(INT8U)Mac[5]);
	debug("MAC %s\n",MacStr);
}

int IFileExit(const char* filename)
{
	FILE* fp = fopen(filename,"r");
	if(!fp)
		return 0;
	fclose(fp);
	return 1;
}

//add by lightd, 2014-06-04
//============================================================================
//Function:    ifconfig_ethx_down_API
//Description: 关闭本地指定网卡 - eg: ifconfig eth0 down
//Input:
//Output:
//Return:
//Others:	   None
//============================================================================
INT8S ifconfig_ethx_down_API(const INT8U *interface_name)
{
	INT32S 		 sock_fd;
	struct ifreq ifr;
	int			 selector;

	//传入参数合法性检测
	if(interface_name == NULL)
	{
		fprintf(stdout, "%s:%d: args invalid!", __FUNCTION__, __LINE__);
		return -1;
	}

	//禁止关闭回环
	if(strncmp((char *)interface_name, (char *)"lo", 2) == 0)
    {
       fprintf(stdout, "%s:%d: You can't pull down interface lo!",  __FUNCTION__, __LINE__);
       return 0;
    }

    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock_fd < 0)
    {
        fprintf(stdout, "%s:%d: socket failed!",  __FUNCTION__, __LINE__);
        return -2;
    }

    sprintf(ifr.ifr_name, "%s", interface_name);

    if(ioctl(sock_fd, SIOCGIFFLAGS, &ifr) < 0)
    {
        fprintf(stdout, "%s:%d: ioctl failed 1!",  __FUNCTION__, __LINE__);
        return -3;
    }

	selector = IFF_UP;
    ifr.ifr_flags &= ~selector;
    if(ioctl(sock_fd, SIOCSIFFLAGS, &ifr) < 0)
    {
        fprintf(stdout, "%s:%d: ioctl failed 2!",  __FUNCTION__, __LINE__);
        return -4;
    }

	close( sock_fd );

    return 0;
}

//add by lightd, 2014-06-04
//============================================================================
//Function:    ifconfig_ethx_up_API
//Description: 打开本地指定网卡 - eg: ifconfig eth0 up
//Input:
//Output:
//Return:
//Others:			 None
//============================================================================
INT8S ifconfig_ethx_up_API(const INT8U *interface_name)
{
	INT32S		 	sock_fd;
	struct ifreq 	ifr;
	int			 	selector;

	//传入参数合法性检测
	if(interface_name == NULL)
	{
		fprintf(stdout, "%s:%d: args invalid!",  __FUNCTION__, __LINE__);
		return -1;
	}

	sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock_fd < 0)
	{
		fprintf(stdout, "%s:%d: create socket failed!",  __FUNCTION__, __LINE__);
		return -2;
	}

	sprintf(ifr.ifr_name, "%s", interface_name);
	if(ioctl(sock_fd, SIOCGIFFLAGS, &ifr) < 0)
	{
		fprintf(stdout, "%s:%d: ioctl error 1",  __FUNCTION__, __LINE__);
		return -3;
	}

	selector = (IFF_UP | IFF_RUNNING);
	ifr.ifr_flags |= selector;
	if(ioctl(sock_fd, SIOCSIFFLAGS, &ifr) < 0)
	{
		fprintf(stdout, "%s:%d: ioctl error 2",  __FUNCTION__, __LINE__);
		return -4;
	}

	close( sock_fd );

	return 0;
}



//add by lightd, 2014-06-04
//============================================================================
//Function:    SetLocalMACAddr_API
//Description: 设置本地指定网卡的MAC
//Input:
//Output:
//Return:
//Others:	   None
//Test result: 测试结果 - 连续调用20次不出错
//============================================================================
INT8S SetLocalMACAddr_API(const INT8U *interface_name, const INT8U *str_macaddr)
{
	int 			ret;
	int 			sock_fd;
    struct ifreq 	ifr;
	INT32U 			mac2bit[6];

	//传入参数合法性检测
	if(interface_name == NULL || str_macaddr == NULL)
	{
		fprintf(stdout, "%s:%d: args invalid!",  __FUNCTION__, __LINE__);
		return -1;
	}

	//提取mac格式
	sscanf((char *)str_macaddr, "%02X:%02X:%02X:%02X:%02X:%02X",
			(INT8U *)&mac2bit[0], (INT8U *)&mac2bit[1], (INT8U *)&mac2bit[2], (INT8U *)&mac2bit[3], (INT8U *)&mac2bit[4], (INT8U *)&mac2bit[5]);

    sock_fd = socket(PF_INET, SOCK_DGRAM, 0);
    if (sock_fd < 0)
    {
		perror("socket error");
        return -2;
    }

	//设置mac前，必须关闭对应的网卡 - 否则出错
    ret = ifconfig_ethx_down_API( interface_name );
	if(ret < 0)
	{
		fprintf(stdout, "%s:%d: close eth0 error",  __FUNCTION__, __LINE__);
		return -3;
	}
	sleep(1); //等待网卡关闭OK

    sprintf(ifr.ifr_ifrn.ifrn_name, "%s", interface_name);
    ifr.ifr_ifru.ifru_hwaddr.sa_family = 1;
	ifr.ifr_ifru.ifru_hwaddr.sa_data[0] = mac2bit[0];
	ifr.ifr_ifru.ifru_hwaddr.sa_data[1] = mac2bit[1];
	ifr.ifr_ifru.ifru_hwaddr.sa_data[2] = mac2bit[2];
	ifr.ifr_ifru.ifru_hwaddr.sa_data[3] = mac2bit[3];
	ifr.ifr_ifru.ifru_hwaddr.sa_data[4] = mac2bit[4];
	ifr.ifr_ifru.ifru_hwaddr.sa_data[5] = mac2bit[5];

    ret = ioctl(sock_fd, SIOCSIFHWADDR, &ifr);
    if (ret != 0)
    {
		perror("set mac address erorr");
        return -4;
    }

	close( sock_fd );

    ret = ifconfig_ethx_up_API( interface_name );
	if(ret < 0)
	{
		fprintf(stdout, "%s:%d: open eth0 error!",  __FUNCTION__, __LINE__);
		return -5;
	}

	sleep(2); //等待网卡打开OK

    return 0;
}

//add by lightd, 2014-06-04int IFileExit(const char* filename)
//============================================================================
//Function:    GetLocalMACAddr_API
//Description: 获取本地指定网卡的MAC
//Input:
//Output:
//Return:
//Others:	   None
//============================================================================
INT8S GetLocalMACAddr_API(const INT8U *interface_name, INT8U *str_macaddr)
{
    INT32S 		 sock_fd;
    struct ifreq ifr_mac;

	//传入参数合法性检测
	if(interface_name == NULL || str_macaddr == NULL)
	{
		fprintf(stdout, "%s:%d: args invalid!",  __FUNCTION__, __LINE__);
		return -1;
	}

    sock_fd = socket( AF_INET, SOCK_STREAM, 0 );
    if( sock_fd == -1)
    {
        perror("create socket failed");
		sprintf((char *)str_macaddr, "00:00:00:00:00:00");
        return -2;
    }

	//指定网卡NowLocalTime
    memset(&ifr_mac, 0, sizeof(ifr_mac));
    sprintf(ifr_mac.ifr_name, "%s", interface_name);

	//获取指定网卡的mac地址
    if( (ioctl( sock_fd, SIOCGIFHWADDR, &ifr_mac)) < 0 )
    {
        perror("mac ioctl error");
		sprintf((char *)str_macaddr, "00:00:00:00:00:00");
        return -3;
    }

	close( sock_fd );

    sprintf((char *)str_macaddr,"%02x:%02x:%02x:%02x:%02x:%02x",
            (unsigned char)ifr_mac.ifr_hwaddr.sa_data[0],
            (unsigned char)ifr_mac.ifr_hwaddr.sa_data[1],
            (unsigned char)ifr_mac.ifr_hwaddr.sa_data[2],
            (unsigned char)ifr_mac.ifr_hwaddr.sa_data[3],
            (unsigned char)ifr_mac.ifr_hwaddr.sa_data[4],
            (unsigned char)ifr_mac.ifr_hwaddr.sa_data[5]);

    debug("local mac:<%s> \n", str_macaddr);

    return 0;
}

void MyExec(const char* cmd)
{
    FILE *pp = popen(cmd, "r"); //建立管道
    if (!pp)
        return ;
#ifdef ReadOutLine
    char tmp[1024]; //设置一个合适的长度，以存储每一行输出
    bzero(tmp,1024);
    while (fgets(tmp, 1023, pp) != NULL) {
        fprintf(stdout,"%s",tmp);
        bzero(tmp,1024);
    }
#endif
    pclose(pp); //关闭管道
}

int SetMacByUci(const char* interface,const INT8U* Mac)
{
	char MacStr[20] = "";
	char Cmd[1024] = "";
#if 0
	sprintf(MacStr,"%02x:%02x:%02x:%02x:%02x:%02x",
			(INT8U)Mac[0],(INT8U)Mac[1],(INT8U)Mac[2],(INT8U)Mac[3],(INT8U)Mac[4],(INT8U)Mac[5]);
#else
	MacIntToStr(Mac,(char*)MacStr);
#endif
	debug("interface %s MAC %s",interface,MacStr);

	sprintf(Cmd,"uci set network.%s.macaddr=%s",interface,MacStr);
	MyExec(Cmd);
	debug("%s\n",Cmd);

	debug("uci commit");
	MyExec("uci commit");

	debug("Beging to restart network");
	MyExec("/etc/init.d/network restart");

	MyExec("ifconfig");
	return 0;
}

void RandomMac(void)
{
	if(!IFileExit(DEFAULT_PATH))
	{
		int i = 0;

		INT8U MacInt[6];
		bzero(MacInt,6);
		struct timeval tv;
		bzero(&tv,sizeof(tv));
		gettimeofday(&tv,NULL);

		srand(tv.tv_usec);
		for(i = 0 ; i < 6; i++)
			if(!i)
				MacInt[i] = (rand()/256) & 0xfe;
			else
				MacInt[i] = rand()/256;
		SetMacByUci(DEAULT_INTERFACE,MacInt);

		FILE* fp = fopen(DEFAULT_PATH,"w+");
		if(!fp)
		{
			debug("Open %s error",DEFAULT_PATH);
			exit(1);
		}
		char MacStr[20] = "";
		MacIntToStr(MacInt,MacStr);
		char * tp = NowLocalTime();
		if(!tp)
			exit(2);
		fprintf(fp,"%s %s\n",tp,MacStr);
		fclose(fp);
		free(tp);
	}else
	{
		debug("%s is exit",DEFAULT_PATH);
	}
}
