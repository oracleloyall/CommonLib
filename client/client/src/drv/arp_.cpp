/*
 * ack.cpp
 *
 *  Created on: 2015-9-18
 *   Author: zhaoxi
 */
/*
 * ack.hpp
 *
 *  Created on: 2015-9-15
 *   Author: zhaoxi
 */
#include"arp_.hpp"
#include<string.h>
#include <sstream>
#include"../hlib/logout.hpp"
#include "../proj/proj_utils.hpp"
extern string read_mac(pByte buf);
string con_collect::IntToStr(const int n)
{
  std::ostringstream result;
  result << n;
  return result.str();
}
string con_collect::read_ip(unsigned char * buf)
{
	string s = "";
	for (int i = 0; i < 8; i++)
	{
		s += IntToStr(*(buf+i));
		if ((i == 7) || ((i == 3) && (*(buf + 4) == 0))) break;
		s += '.';
	}
	return s;
}


con_collect::con_collect(const char *name)
{
	//cout<<"name="<<name<<endl;
	print_arp(name);
}
int con_collect::get_mtime()
{
	time_t timep;
	struct tm *p;
	time(&timep);
	p=localtime(&timep); /*取得当地时间*/
    return (p->tm_hour*60+p->tm_min);
}

void con_collect::write_mac(unsigned char* buf, string mac)
{
	memset(buf, 0, 6);
	for(int i = 0; i < 6; i++)
	{
		*(buf+i) = HexToInt(mac[i * 2]) * 16 + HexToInt(mac[i * 2 + 1]);
	}
}

int con_collect::HexToInt(char c)
{
	if (c >= '0' && c <= '9')
		return (c - '0');
	if (c >= 'A' && c <= 'F')
		return (c - 'A' + 10);
	if (c >= 'a' && c <= 'f')
		return (c - 'a' + 10);
	return 0;
}
void con_collect::update_list(const char* ifname)
{
	char buf[1024];
	FILE* file = fopen(DEFAULT_ARP, "r");
	if (!file)
	{
		logout("fopen %s error\n",DEFAULT_ARP);
		return;
	}

	bzero(buf, 1024);
	while (fgets(buf, 1024, file))
	{
		static int i;
		//printf("-------i=%d\n", i);
		i++;
		char *cp = buf;
		char *ip = NULL, *type = NULL, *Flags = NULL, *mac = NULL, *mask = NULL,
				*device = NULL;
		while (*cp++)
			if ((*cp) == '\n' || (*cp == '\r'))
				*cp = 0;
		if (!buf[0])
			continue;
		ip = buf;
		if (!isxdigit(*ip))
			continue;
		while (isspace(*ip))
			ip++;

		type = strstr(ip, " ");
		*type++ = 0;	// 获取ip地址的字符串
		while (isspace(*type))
			type++;

		Flags = strstr(type, " ");									// 获取type字符串
		*Flags++ = 0;
		while (isspace(*Flags))
			Flags++;

		mac = strstr(Flags, " ");									// 获取flag字符串
		*mac++ = 0;
		while (isspace(*mac))
			mac++;

		mask = strstr(mac, " ");
		*mask++ = 0;												// 获取mac字符串
		while (isspace(*mask))
			mask++;

		device = strstr(mask, " ");
		*device++ = 0;												// 获取mas字符串
		while (isblank(*device))
			device++;	// 获取device字符串
		Arp_iterator arp_iterator;
		int m_flag = atoi(Flags + 2);
		if (!strcmp(ifname, device))
		{
			string Ip(ip);
			unsigned char buf1[16],buf2[10];
			bzero(buf1,16);
			bzero(buf2,10);
			write_ip(buf1, Ip);
			string m_p = read_ip(buf1);
			char* cp1 = NULL;
			char* cp =  NULL;
			cp1 = cp = mac;
			while(*cp1)												// 去掉MAC冒号
			{
				if(*cp1 == ':')
				{
					cp1 ++;
					continue;
				}
				*cp ++ = *cp1 ++;
			}
			mac[12] = 0;

			write_mac(buf2,string(mac));
			string m_mac = read_mac(buf2);
//			logout("Flag(%s) = %d IP = %s MAC = %s mac = %s\n",Flags,m_flag,ip,m_mac.c_str(),mac);
			for (arp_iterator = p_arp.begin(); arp_iterator != p_arp.end();arp_iterator++)
			{
				string m_p2 = read_ip((*arp_iterator)->arp_ip);
				string m_p3 = read_mac((*arp_iterator)->arp_mac);
				if (!strcmp(m_p.c_str(), m_p2.c_str()) && (m_mac == m_p3))
				{
					if(m_flag)
					{
//						logout("Update time\n");
						int time_now = time(NULL);
						(*arp_iterator)->update_time = time_now;
						if((*arp_iterator)->state == 3)
							(*arp_iterator)->state = 1;
					}else
					{
//						if(((*arp_iterator)->state == 1) || ((*arp_iterator)->state == 2))
						logout("%s is offline\n",ip);
						(*arp_iterator)->state = 3;
					}
					break;
				}else
				{
#if 1
					if(m_mac == m_p3)
					{
						logout("Mod  IP %s to  IP %s\n",m_p2.c_str(),m_p.c_str());
						write_ip((*arp_iterator)->arp_ip,m_p);
						break;
					}
#endif
				}
			}
			if(m_flag && (arp_iterator == p_arp.end()))
			{
				logout("m_flag Insert node into list IP %s\n",ip);
				print_arp1(ip, type, Flags, mac, mask, device);
			} else
				continue;
		}
		bzero(buf, 1024);
	}
	fclose(file);
}
string con_collect::read_mac(unsigned char *buf)
{
	char temp[16];
	temp[16] = 0;
	for (int i = 0; i < 6; i++) {
		sprintf(&temp[i * 2], "%02X", *(buf + i));
	}
	string s = temp;
	return s;
}

void con_collect::del_list()
{
	unsigned int nowtime = time(NULL);
	for (arp_iterator = p_arp.begin(); arp_iterator != p_arp.end();)
	{
		if(!(*arp_iterator) && ((*arp_iterator)->state == 3))
		{
			logout("*arp_iterator is NULL\n");
			return ;
		}
		int tm = (nowtime - (*arp_iterator)->update_time);
		if( tm >= 150)
		{
			logout("Now %d  Date %d\n",nowtime,(*arp_iterator)->update_time);
			if(*arp_iterator)
				delete *arp_iterator;
			Arp_iterator its = arp_iterator ++ ;
			p_arp.erase(its);
			continue;
		}
		arp_iterator++;
	}
}

void con_collect::up_stat1(const char *ifname,unsigned char *mac,unsigned char status)
{
		unsigned char buf1[6];
		memcpy(buf1,mac,sizeof(mac));
		for (arp_iterator = p_arp.begin(); arp_iterator != p_arp.end();
				arp_iterator++)
		{
			unsigned char buf[6];
			memcpy(buf, (*arp_iterator)->arp_mac, sizeof(buf));
			int re = strcmp((const char *) buf,(const char*)buf1);
			if (re == 0)
			{
				if(status==2||status==3)
				{
					//认证状态修改
					(*arp_iterator)->state=status;
				}
				//return;
			} else
			{

			}
		}
}
void con_collect::up_stat(const char *ifname,string &mac,unsigned char status)
{
	unsigned char buf1[6];
	write_mac(buf1,mac);
	for (arp_iterator = p_arp.begin(); arp_iterator != p_arp.end();
			arp_iterator++)
	{
		unsigned char buf[6];
		memcpy(buf, (*arp_iterator)->arp_mac, sizeof(buf));
		string w=read_mac(buf);
		int re = strcmp(mac.c_str(),w.c_str());
		if (re == 0)
		{

			logout("mac匹配到\n");
			if(status==2||status==3)
			{
				//认证状态修改
				(*arp_iterator)->state=status;
				logout("up -stat 状态是%d\n",(*arp_iterator)->state);
			}
			//return;
		} else
		{
		}
	}
}
void con_collect::write_ip(unsigned char* buf, string ip)
{
	memset(buf, 0, 8);
	int j = ip.size();
	int head = 0;
	int index = 0;
	for (int i = 0; i < j; i++)
	{
		if (ip[i] == '.')
		{
			*(buf + index++) = StrToInt(ip.substr(head, i - head));
			head = i + 1;
		}
	}
	*(buf + index) = StrToInt(ip.substr(head, j - 1));

}
int con_collect::StrToInt(string s)
{
	return atoi(s.c_str());
}
void con_collect::remove(string &str)
{
	int i = 0, j = 0, len, len2;
    len=str.length();
	char b[20];
	bzero(b,20);
	for (i = 0; i < len; i++)
	if (str[i] != ':')
	{
		b[j] = str[i];
				j++;
	}
	len2 = strlen(b);
	for (i = 0; i < len2; i++)
		str[i] = b[i];
	if(len2)
		str[i] = '\0';
}
void con_collect::print_arp2(const char *device)//去读配置文件
{
	char buf[1024];
	FILE* file = fopen(DEFAULT_ARP, "r");
	if (!file)
	{
		perror("fopen");
		return;
	}
	bzero(buf, 1024);
	while (fgets(buf, 1024, file))
	{
		static int i;
		//printf("-------i=%d\n", i);
		i++;
		char *cp = buf;
		char *ip = NULL, *type = NULL, *Flags = NULL, *mac = NULL, *mask = NULL,
			*device = NULL;
		while (*cp++)
		if ((*cp) == '\n' || (*cp == '\r'))
			*cp = 0;
	    if (!buf[0])
			continue;
		ip = buf;
		//printf("------------------------------------------ip=%s\n",ip);
		if (!isxdigit(*ip))
			continue;
		while (isspace(*ip))
			ip++;

		type = strstr(ip, " ");
		*type++ = 0;	// 获取ip地址的字符串
		while (isspace(*type))
			type++;

		Flags = strstr(type, " ");									// 获取type字符串
		*Flags++ = 0;
		while (isspace(*Flags))
			Flags++;

		mac = strstr(Flags, " ");									// 获取flag字符串
		*mac++ = 0;
		while (isspace(*mac))
			mac++;

		mask = strstr(mac, " ");
		*mask++ = 0;												// 获取mac字符串
		while (isspace(*mask))
			mask++;

		device = strstr(mask, " ");
		*device++ = 0;												// 获取mas字符串
		while (isblank(*device))
		  device++;	// 获取device字符串
}
}
void con_collect::print_arp(const char* ifname)
{
	char buf[1024];
	FILE* file = fopen(DEFAULT_ARP, "r");
	if (!file)
	{
		perror("fopen");
		return;
	}
	bzero(buf, 1024);
	while (fgets(buf, 1024, file))
	{
		static int i;
		//printf("-------i=%d\n", i);
		i++;
		char *cp = buf;
		char *ip = NULL, *type = NULL, *Flags = NULL, *mac = NULL, *mask = NULL,
				*device = NULL;
		while (*cp++)
			if ((*cp) == '\n' || (*cp == '\r'))
				*cp = 0;
		if (!buf[0])
			continue;
		ip = buf;
		//printf("------------------------------------------ip=%s\n",ip);
		if (!isxdigit(*ip))
			continue;
		while (isspace(*ip))
			ip++;

		type = strstr(ip, " ");
		*type++ = 0;	// 获取ip地址的字符串
		while (isspace(*type))
			type++;

		Flags = strstr(type, " ");									// 获取type字符串
		*Flags++ = 0;
		while (isspace(*Flags))
			Flags++;

		mac = strstr(Flags, " ");									// 获取flag字符串
		*mac++ = 0;
		while (isspace(*mac))
			mac++;

		mask = strstr(mac, " ");
		*mask++ = 0;												// 获取mac字符串
		while (isspace(*mask))
			mask++;

		device = strstr(mask, " ");
		*device++ = 0;												// 获取mas字符串
		while (isblank(*device))
			device++;	// 获取device字符串

		if (atoi(Flags + 2) && !strcmp(ifname, device)) {
			//cout<<"添加新的元素\n";
			print_arp1(ip , type ,Flags , mac ,mask ,device );
		}
		bzero(buf, 1024);
	}
	fclose(file);
}
void con_collect::print_arp1(const char *ip , const char *type ,const  char *Flags , char *mac ,const char *mask ,
		const char *device )//添加节点
{
	arp_accept *a = new arp_accept;
	if(!a)
		return;
	memset(a, 0, sizeof(arp_accept));

//	logout("step 1.10\n");
	string Ip= string(ip);
	//cout<<"string 下de "<<Ip<<endl;
	unsigned char buf1[16];
	bzero(buf1,16);
//	logout("step 1.11\n");
    write_ip(buf1,Ip);
	memcpy(a->arp_ip,buf1,16);
//	logout("step 1.0\n");
	string ap;
	ap=read_ip(a->arp_ip);
//	logout("step 1.1 %s\n",mac);
	string Ma(mac);
	remove(Ma);
//	logout("step 1.2.1\n");
	unsigned char buf[6];
	memset(buf,0,sizeof(buf));
	write_mac(buf,Ma);
//	logout("step 1.2\n");
	memcpy(a->arp_mac, buf, 6);
//	logout("step 1.3\n");
    string f=read_mac(a->arp_mac);
//    logout("step 1.4\n");
    unsigned char sta;
    sta = atoi(Flags + 2);
    if(sta>0)
		a->state = 1;//1上线
    else
        a->state = 3;//下线
    a->oldstate = 0;

    logout("Insert IP %s Flag %d %d\n",ip,sta,a->state);
    a->update_time = time(NULL);
	p_arp.push_front(a);
	logout("Inset into list end\n");
}
void con_collect::Cpy(unsigned char *p,unsigned char *s)
{

	int m=sizeof(s);
	for(int i=0;i<m;i++)
	{
		  *(p+i) = *(s+i);
	}
}
con_collect::~con_collect()
{
	Arp_iterator it;
	for ( it = p_arp.begin();it != p_arp.end();it++)
	{
		delete (*it);
	}
	p_arp.clear();
}
