/*
 * driver.cpp
 *
 *  Created on: 2015-3-30
 *      Author: root
 */

#ifndef DRIVER_CPP_
#define DRIVER_CPP_

#include "driver.hpp"
#include "../hlib/global.hpp"
#include "../hlib/logout.hpp"
#include "../hlib/list.hpp"
#include "../proj/proj_utils.hpp"
#include "../proj/ap_type2s.hpp"
#include "../hlib/hpasutils.hpp"
#include "../hlib/md5.h"
#include "../proj/randommac.hpp"
#include"arp_.hpp"

#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/sysinfo.h>
#include <sys/statfs.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <net/if.h>


#define COM_LEN 200
//("br-lan");
con_collect con("br-lan");

string DEV_MAC = "FEDCBA987663";
char  *g_pcUsrAction = ( char * )malloc( MAX_BODY_SIZE );
#ifdef	USER_INTERVAL
__u16 UserInterval    = 30;  //s
__u16 UserMsgInterval = 100; //ms
#endif
#ifdef POWER
extern TeWiFiClient* ec;
#endif

string get_dev_seq()
{
	char buf[65] = "";
	bzero(buf,35);
	snprintf(buf,32,"%32s","version 15.10.08");
	snprintf(buf + 32,32,"%32s","ewifi");
	return string(buf);
//	return "Dummy device of Telincn.com";
}

string get_dev_lan_ip()
{
   return "192.168.11.1";
}

string get_dev_wan_mac()
{
	return DEV_MAC;
}

void get_dev_wan_mac(pByte buf)
{
   buf[0] = 0xFE;
   buf[1] = 0xDC;
   buf[2] = 0xBA;
   buf[3] = 0x98;
   buf[4] = 0x76;
   buf[5] = 0x54;
}

void get_dev_lan_ip(pByte buf)
{
	buf[0] = 192;
	buf[1] = 168;
	buf[2] = 1;
	buf[3] = 100;
}

void on_white_list(string wl)
{
	logout("coming into on_white_list\n");
	logout("on_white_list end\n");
}

void on_black_list(string bl)
{
	logout("coming into set blacklist\n");
	logout("blacklist:%s\n",bl.c_str());
	logout("blacklist end\n");
}

void on_ap_initial_conf(PHOtherOption aic)
{
	logout("coming into on_ap_initial_conf\n");
	if((aic->ap_interval >=10) && (aic->ap_interval < 600))
		UserInterval = aic->ap_interval;
	logout("on_ap_initial_conf end\n");
}
void cutIPAddr(const char ip[],int arr[])
{
    bool flag=true;
    int times=0;
    for(int i=0;i<static_cast<int>(strlen(ip));i++)
    {
        if(ip[i]!='\0')
        {
            if(ip[i]!='.')
            {
                if(flag==true)
                {
                    arr[times]=atoi(ip+i);
                    times++;
                    flag=false;
                }
            }
            else
            {
                flag=true;
            }
        }
     }
}

static void on_ap_lan_conf(plan_entity lan)
{
	logout("coming into lan config\n");
//	printf("lan->mode:%d\n", lan->dhcp_mode);
//	printf("lan->leasetiem:%d\n", lan->leasetime);
//	printf("lan->vlan_id:%d\n", lan->vlan_id);
//	printf("lan->dhcp_start:%s\n", lan->dhcp_start);
//	printf("lan->dhcp_end:%s\n", lan->dhcp_end);
//	printf("uci set network.lan.mask=%s\n", read_ip(lan->mask).c_str());
//	printf("uci set network.lan.mac=%s\n", read_mac(lan->gw_mac).c_str());
//	printf("uci set network.lan.ipaddr=%s,%s,%d\n", read_ip(lan->gw_ip).c_str(),lan->gw_ip,lan->gw_ip);
//	string start_ip = read_ip(lan->dhcp_start);
//	printf("start_ip=%s", start_ip.c_str());
//	string end_ip = read_ip(lan->dhcp_end);
//	printf("end_ip=%s\n", end_ip.c_str());

	//修改dhcp服务
	if (lan->dhcp_mode == 0 || lan->dhcp_mode == 1)
	{
		if (lan->dhcp_mode == 0)
			system("uci set dhcp.lan.ignore=1");
		else
			system("uci set dhcp.lan.ignore=0");
	}
	//修改leasetime
	if (0 != lan->leasetime)
	{
		//printf("lease 不为空\n");
		char buf[100];
		bzero(buf, sizeof(buf));
		sprintf(buf, "uci set dhcp.lan.leasetime=%dh", lan->leasetime / 60);
		system(buf);
	}
	//修改ip
	if (lan->gw_ip[0]!= '\0')
	{
		//printf("网关ip 不为空进入\n");
		char buf[100];
		bzero(buf, sizeof(buf));
		sprintf(buf, "uci set network.lan.ipaddr=%s",
				read_ip(lan->gw_ip).c_str());
		system(buf);
	}

	//修改mask
	if (lan->mask[0] != '\0')
	{
		//printf("mask 不为空进入\n");
		char buf[100];
		bzero(buf, sizeof(buf));
		sprintf(buf, "uci set network.lan.netmask=%s",
				read_ip(lan->mask).c_str());
		system(buf);
	}
	//修改mac
	if (lan->gw_mac[0] !='\0')
	{
		//printf("mac 不为空进入\n");
		char buf[100];
		bzero(buf, sizeof(buf));
		sprintf(buf, "uci set network.lan.macaddr=%s",
				read_mac(lan->gw_mac).c_str());
		system(buf);
	}
	//修改开始和结束的客户数
	if (lan->dhcp_start[0] !='\0' && lan->dhcp_end[0] !='\0')
	{
		//printf("dhcp开始和结束 不为空进入\n");
		string start_ip = read_ip(lan->dhcp_start);
		string end_ip = read_ip(lan->dhcp_end);
		int arr1[4];
		int arr2[4];
		cutIPAddr(start_ip.c_str(), arr1);
		cutIPAddr(end_ip.c_str(), arr2);
		int res = 1;
	    static int StartIp;
		for (int i = 0; i < 4; i++)
		{
			res *= (abs(arr1[i] - arr2[i]) + 1);
			if (3 == i)
				StartIp = arr1[i];
		}
		if (res != 0)
		{
			//if(arr1[3]<arr2[3])
			//	return ;
			char buf[100];
			bzero(buf, sizeof(buf));
			char buf1[100];
			bzero(buf1, sizeof(buf1));
			sprintf(buf1, "uci set dhcp.lan.start=%d", arr1[3]);
			sprintf(buf, "uci set dhcp.lan.limit=%d", res);
//			printf("当前的uci命令设置dhcp start:%s\n",buf1);
//			printf("当前uci命令为设置dhcp limit：%s\n",buf);
			system(buf1);
			system(buf);
		}
	}
	//system("uci commit dhcp");
	//system("uci commit network");
	logout("lan end config\n");
}

string read_ipV4(pByte buf)
{
	string s = "";
	for (int i = 0; i < 8; i++)
	{
		s += IntToStr(*(buf+i));
		if(i == 3) break;
		s += '.';
	}
	return s;
}

static __u16 on_ap_wan_conf(pwan_entity2 wan)
{

	logout("coming into wan config\n");
	logout("wan end config\n");
    char cCmd[ 255 ]     = { 0 };
    char cWanName[ 10 ]  = { 0 };
    char cUserName[ 31 ] = { 0 };
    char cPassword[ 31 ] = { 0 };
	unsigned char cValue;
//	unsigned int  iIp;
//	unsigned int  iMask;
	unsigned int  iMode;
	unsigned int  iDNSNum;
	unsigned int  i;
	cValue = wan->wan_index ;

	if( cValue & 0x80 )
	{
		cValue = ( cValue & 0x7f );
		sprintf( (char *)cWanName,"wan%d", (char)cValue );
		sprintf( (char *)cCmd,"uci delete network.%s", (char *)cWanName );
		//printf("wanname--------------%s\n",cCmd);
		system( cCmd );
		return( 1 );
	}
	else
	{
		sprintf( (char *)cWanName,"wan%d", (char)cValue );
		sprintf( (char *)cCmd, "uci set network.%s=interface",(char *)cWanName );
		//printf("wanname--------------%s\n",cCmd);
		system( cCmd );
	}
   if( wan->wan_mac[ 0 ] != 0 )
   {
	   bzero( cCmd, sizeof( cCmd ) );
	   sprintf( (char *)cCmd, "uci set network.%s.macaddr=%s", (char *)cWanName, read_mac( ( Byte* )wan->wan_mac ).c_str() );
	   //printf("macaddr--------------%s\n",cCmd);
	   system( cCmd );
   }
   printf( "*******wan mtu:%d\n",wan->wan_mtu );
   if( wan->wan_mtu > 0 )
   {
	   bzero( cCmd, sizeof( cCmd ) );
	   sprintf( (char *)cCmd, "uci set network.%s.mtu=%s", (char *)cWanName, UIntToStr( wan->wan_mtu ).c_str() );
	   //printf("mtu--------------%s\n",cCmd);
	   system( cCmd );
   }
   if( ( wan->auto_dns == 0 ) || ( wan->auto_dns == 1 ) )
   {
	   bzero( cCmd, sizeof( cCmd ) );
	   sprintf( (char *)cCmd, "uci set network.%s.auto=%d", (char *)cWanName, wan->auto_dns );
	   //printf("auto--------------%s\n",cCmd);
	   system( cCmd );
   }

	iMode = wan->conn_mode;

	if( iMode == 1 )           //PPPoE
	{
		if( wan->dns_mode == 0 )
		{
			iDNSNum = wan->dns_count;
			for( i = 0; i < iDNSNum; i++ )
			{
				if( strlen( (char *)wan->wan_ip ) == 4 )
				{
					if( wan->dns[ i * 4 ] != 0 )
					{
						bzero( cCmd, sizeof( cCmd ) );
						sprintf( (char *)cCmd, "uci add_list network.%s.dns=%s", (char *)cWanName, read_ipV4( ( Byte* )( wan->dns + i * 4 ) ).c_str() );
						//printf("dns--------------%s\n",cCmd);
						system( cCmd );
					}
				}
				else
				{
					//IPV6
				}
			}
		}
		if( wan->usr_len > 0 )
		{
			for( i = 0; i < wan->usr_len; i++ )
			{
				cUserName[ i ] = wan->usr[ i ];
			}
			bzero( cCmd, sizeof( cCmd ) );
			sprintf( (char *)cCmd, "uci set network.%s.username=%s", (char *)cWanName, (char *)cUserName );
			//printf("username--------------%s\n",cCmd);
			system( cCmd );
		}

		if( wan->pswd_len > 0 )
		{
			for( i = 0; i < wan->pswd_len; i++ )
			{
				cPassword[ i ] = wan->pswd[ i ];
			}
			bzero( cCmd, sizeof( cCmd ) );
			sprintf( (char *)cCmd, "uci set network.%s.password=%s", (char *)cWanName, (char *)cPassword );
			//printf("passwd--------------%s\n",cCmd);
			system( cCmd);
		}
	}
	else if( iMode == 2 )      //DHCP
	{
	/*	if( wan->wan_gw[ 0 ] == 0 )
		{
			bzero( cCmd, sizeof( cCmd ) );
			sprintf( (char *)cCmd, "uci set network.%s.netmask=%s", (char *)cWanName, read_ip( ( Byte* )wan->wan_gw ).c_str() );
			printf("gateway--------------%s\n",cCmd);
			system( cCmd );
		}*/
		if( wan->dns_mode == 0 )
		{
			 iDNSNum = wan->dns_count;
			 for( i = 0; i < iDNSNum; i++ )
			 {
			     if( strlen(  (char *)wan->wan_ip ) == 4 )
				  {
				      if( wan->dns[ i * 4 ] != 0 )
						{
						    bzero( cCmd, sizeof( cCmd ) );
						    sprintf( (char *)cCmd, "uci add_list network.%s.dns=%s", (char *)cWanName, read_ipV4( ( Byte* )( wan->dns + i * 4 ) ).c_str() );
						   // printf("dns--------------%s\n",cCmd);
						    system( cCmd );
					    }
				   }
				  else
				  {

				  }
			  }
		}

	}
	else if( iMode == 3 )     //static
	{
		if( wan->wan_ip[ 0 ] == 0 )
		{
			sprintf( (char *)cCmd, "uci set network.%s.ipaddr=%s", (char *)cWanName, read_ip( ( Byte* )wan->wan_ip ).c_str() );
			//printf("ipaddr--------------%s\n",cCmd);
			system( cCmd );
		}
/*
		if( wan->wan_gw[ 0 ] == 0 )
		{
			bzero( cCmd, sizeof( cCmd ) );
			sprintf( (char *)cCmd, "uci set network.%s.ip6gw=%s", (char *)cWanName, read_ip( ( Byte* )wan->wan_gw ).c_str() );
			printf("ip6gw--------------%s\n",cCmd);
			system( cCmd );
		}*/

		if( wan->wan_gw[ 0 ] == 0 )
		{
			bzero( cCmd, sizeof( cCmd ) );
			sprintf( (char *)cCmd, "uci set network.%s.gateway=%s", (char *)cWanName, read_ip( ( Byte* )wan->wan_gw ).c_str() );
			//printf("network--------------%s\n",cCmd);
			system(cCmd);
		}
		if( wan->dns_mode == 0 )
		{
		    iDNSNum = wan->dns_count;
		    for( i = 0; i < iDNSNum; i++ )
		    {
			    if( strlen(  (char *)wan->wan_ip ) == 4 )
			    {
			        if( wan->dns[ i * 4 ] != 0 )
			         {
			            bzero( cCmd, sizeof( cCmd ) );
			            sprintf( (char *)cCmd, "uci add_list network.%s.dns=%s", (char *)cWanName, read_ipV4( ( Byte* )( wan->dns + i * 4 ) ).c_str() );
			            system( cCmd );
			         }
			    }
			    else
			    {/*
				if( dns[ i * 16 ] != 0 )
				{
					bzero( cCmd, sizeof( cCmd ) );
					sprintf( cCmd, "uci add_list network.%s.dns=%s", cWanName, read_ip( ( Byte* )wan->dns ).c_str() );
					system( "cCmd" );
				}*/
			    }
		    }
		}
	}
	else
	{

	}

	//system("uci commit network");
    return( 1 );
}

static int on_ap_wlan_conf(pwlan_entity2 wlan)
{
	logout("coming into wlan config\n");

			unsigned char cValue;
			int  iValue;
			unsigned char cCmd[ 255 ]        = { 0 };
			unsigned char cWlanName[ 10 ]    = { 0 };
			unsigned char cWlanSsid[ 255 ]   = { 0 };
			unsigned int  iLimit             = 0;
			unsigned char cDhcpIpStart[ 16 ] = { 0 };
			unsigned char cDhcpIpEnd[ 16 ]   = { 0 };
		//	unsigned char cWlanSsid2[ 255 ] = { 0 };

			cValue = wlan->wlan_index;
			iValue = wlan->wlan_index;
			if( cValue & 0x80 )
			{
			}
			else
			{
				sprintf( (char *)cWlanName,"wlan%d", ( char )cValue );
			}
			bzero( cCmd, sizeof( cCmd ) );
			sprintf( (char *)cCmd, "uci set wireless.@wifi-iface[%d].mode=ap",iValue );
			system( ( char * )cCmd );
			bzero( cCmd, sizeof( cCmd ) );
			sprintf( (char *)cCmd, "uci set wireless.@wifi-iface[%d].network=lan",iValue );
			system( ( char * )cCmd );

				if( wlan->wlan_ssid_len > 0 )
				{
					for( int i = 0; i < wlan->wlan_ssid_len; i++ )
					{
						cWlanSsid[ i ] = wlan->wlan_ssid[ i ];
					}
					bzero( cCmd, sizeof( cCmd ) );
					sprintf( (char *)cCmd, "uci set wireless.@wifi-iface[%d].ssid=%s",iValue, (char *)cWlanSsid );
					//printf("wlan_ssid_len>0-----------wlan ssid====%s\n",cCmd);
					system( ( char * )cCmd );
				}


			if( wlan->wlan_hidden == 1 )
			{
				 bzero( cCmd, sizeof( cCmd ) );
			    sprintf( (char *)cCmd, "uci set wireless.@wifi-iface[%d].hidden=1",iValue );
			    //printf("wlan_hidden==1-----------wlan hidden=0时候的uci命令==%s\n",cCmd);
			    system( ( char * )cCmd );
			}
			else if( wlan->wlan_hidden == 0 )
			{
				bzero( cCmd, sizeof( cCmd ) );
				sprintf( (char *)cCmd, "uci set wireless.@wifi-iface[%d].hidden=0",iValue );
				//printf("wlan_hideent==0-----------wlan hidden=1时候的uci命令====%s\n",cCmd);
				system( ( char * )cCmd );
			}
			else
			{

			}
			printf( "****dhcp_enabled:%d\n",wlan->dhcp_enabled );
		   if( wlan->wlan_enabled == 1 )
		   {
			    bzero( cCmd, sizeof( cCmd ) );
			    sprintf( (char *)cCmd, "uci set wireless.@wifi-device[%d].disabled=0",iValue );
			   // printf("dhcp_enabled==1-----------wlan disabled=====%s\n",cCmd);
			    system( ( char * )cCmd );
		   }
		   else if( wlan->wlan_enabled == 0 )
		   {
			   bzero( cCmd, sizeof( cCmd ) );
			   sprintf( (char *)cCmd, "uci set wireless.@wifi-device[%d].disabled=1",iValue );
			   //printf("dhcp_enabled==0-----------wlan disabled=====%s\n",cCmd);
			   system( ( char * )cCmd );
		   }
		   if( wlan->wlan_txpower > 0 )
		   {
			   bzero( cCmd, sizeof( cCmd ) );
			   sprintf( (char *)cCmd, "uci set wireless.@wifi-device[%d].txpower=%d", wlan->wlan_txpower,iValue );
			  // printf("----------wlan txpower=====%s\n",cCmd);
			   system( ( char * )cCmd );
		   }
		 //  if( ( wlan->wlan_channel >= 0 ) || ( wlan->wlan_channel <= 255 ) )
		 //  {
			   bzero( cCmd, sizeof( cCmd ) );
	//		   sprintf( (char *)cCmd, "uci set wireless.@wifi-device[0].channel=%d", wlan->wlan_channel );
			   sprintf( (char *)cCmd, "uci set wireless.@wifi-device[%d].channel=auto",iValue );
			  // printf("----------wlan channel=====%s\n",cCmd);
			   system( ( char * )cCmd );
		//   }

			switch( wlan->dev_type )
			{
			case 1:
				bzero( cCmd, sizeof( cCmd ) );
				sprintf( (char *)cCmd, "uci set wireless.@wifi-device[%d].hwmode=11b",iValue );
				//printf("-case 1---------wlan hwmode=====%s\n",cCmd);
				system( ( char * )cCmd );
				break;
			case 2:
				bzero( cCmd, sizeof( cCmd ) );
				sprintf( (char *)cCmd, "uci set wireless.@wifi-device[%d].hwmode=11g",iValue );
				//printf("-case 2---------wlan hwmode=====%s\n",cCmd);
				system( ( char * )cCmd );
				break;
			case 3:
				bzero( cCmd, sizeof( cCmd ) );
				sprintf( (char *)cCmd, "uci set wireless.@wifi-device[%d].hwmode=11bg",iValue );
				//printf("case 3----------wlan 11bg=====%s\n",cCmd);
				system( ( char * )cCmd );
				break;
			case 4:
				bzero( cCmd, sizeof( cCmd ) );
				sprintf( (char *)cCmd, "uci set wireless.@wifi-device[%d].hwmode=11n",iValue );
				//printf("case 4----------wlan hwmode=====%s\n",cCmd);
				system( ( char * )cCmd );
				break;
			case 7:
				bzero( cCmd, sizeof( cCmd ) );
				sprintf( (char *)cCmd, "uci set wireless.@wifi-device[%d].hwmode=11bgn",iValue );
				//printf("case 7----------wlan hwmode=====%s\n",cCmd);
				system( ( char * )cCmd );
				break;
			case 8:
				bzero( cCmd, sizeof( cCmd ) );
				sprintf( (char *)cCmd, "uci set wireless.@wifi-device[%d].hwmode=11a",iValue );
				//printf("case 8----------wlan hwmode====%s\n",cCmd);
				system( ( char * )cCmd );
				break;
			case 15:
				bzero( cCmd, sizeof( cCmd ) );
				sprintf( (char *)cCmd, "uci set wireless.@wifi-device[%d].hwmode=11abgn",iValue );
				//printf("case 15----------wlan hwmode====%s\n",cCmd);
				system( ( char * )cCmd );
				break;
			default:
			   break;
			}
			if( wlan->sofe_mode == 1 )  //none
			{
				bzero( cCmd, sizeof( cCmd ) );
				sprintf( (char *)cCmd, "uci set wireless.@wifi-iface[%d].encryption=none",iValue );
				//printf("sofe_mode==1----------wlan none====%s\n",cCmd);
				system( ( char * )cCmd );
			}
			else if( wlan->sofe_mode == 2 )  //wep
			{
				bzero( cCmd, sizeof( cCmd ) );
				sprintf( (char *)cCmd, "uci set wireless.@wifi-iface[%d].encryption=wep",iValue );
				//printf("sofe_mode==2----------wlan wep====%s\n",cCmd);
				system( ( char * )cCmd );

				bzero( cCmd, sizeof( cCmd ) );
				sprintf( (char *)cCmd, "uci set wireless.@wifi-iface[%d].key=%s",iValue, ( char * )wlan->secret_key );
				system( ( char * )cCmd );
			}
			else if( wlan->sofe_mode == 3 )  //wpa
			{
				if( wlan->encrtpt_mode == 0 )
				{
					bzero( cCmd, sizeof( cCmd ) );
					sprintf( (char *)cCmd, "uci set wireless.@wifi-iface[%d].encryption=psk",iValue );
					system( ( char * )cCmd );
				}
				else if( wlan->encrtpt_mode == 1 )
				{
					bzero( cCmd, sizeof( cCmd ) );
					sprintf( (char *)cCmd, "uci set wireless.@wifi-iface[%d].encryption=psk+aes",iValue );
					system( ( char * )cCmd );
				}
				else if( wlan->encrtpt_mode == 2 )
				{
					bzero( cCmd, sizeof( cCmd ) );
					sprintf( (char *)cCmd, "uci set wireless.@wifi-iface[%d].encryption=psk+tkip",iValue );
					system( ( char * )cCmd );
				}
				else
				{
					bzero( cCmd, sizeof( cCmd ) );
					sprintf( (char *)cCmd, "uci set wireless.@wifi-iface[%d].encryption=psk",iValue );
					system( ( char * )cCmd );
				}
				bzero( cCmd, sizeof( cCmd ) );
				sprintf( (char *)cCmd, "uci set wireless.@wifi-iface[%d].key=%s",iValue, ( char * )wlan->secret_key );
				system( ( char * )cCmd );
			}
	      if( wlan->session_limit > 0 )
	      {
	    	  bzero( cCmd, sizeof( cCmd ) );
	    	  sprintf( (char *)cCmd, "uci set wireless.@wifi-iface[%d].max_listen_int=%d",iValue, wlan->session_limit );
	    	  system( ( char * )cCmd );
	      }


			if( wlan->dhcp_enabled == 1 )
			{
				memcpy( cDhcpIpStart, read_ip( ( Byte* )( wlan->dhcp_start ) ).c_str(), read_ip( ( Byte* )( wlan->dhcp_start ) ).size() );
				memcpy( cDhcpIpEnd, read_ip( ( Byte* )( wlan->dhcp_end ) ).c_str(), read_ip( ( Byte* )( wlan->dhcp_end ) ).size() );
				if( ( cDhcpIpStart[ 2 ] <= cDhcpIpEnd[ 2 ] ) && ( cDhcpIpStart[ 3 ] < cDhcpIpEnd[ 3 ] ) )
				{
		    	    bzero( cCmd, sizeof( cCmd ) );
		    	    sprintf( (char *)cCmd, "uci set dhcp.lan.start=%d", cDhcpIpStart[ 3 ] );
		    	    system( ( char * )cCmd );
		    	    iLimit = ( cDhcpIpEnd[ 3 ] - cDhcpIpStart[ 3 ] ) * ( cDhcpIpEnd[ 2 ] -wlan->dhcp_start[ 2 ] + 1 ) * 255;
		    	    if( iLimit < 255  )
		    	    {
		    	    	bzero( cCmd, sizeof( cCmd ) );
		    	    	sprintf( (char *)cCmd, "uci set dhcp.lan.limit=%d", iLimit );
		    	    	system( ( char * )cCmd );
		    	    }
				}
			}
			else if( wlan->dhcp_enabled == 0 )
			{

			}

			logout("wlan end config\n");
			//system("uci commit dhcp");
			//system("uci commit wireless");
			//system("/etc/init.d/wireless restart");
			//system("uci commit wireless");
			return( 1 );
}

static void on_ap_vlan_conf(pvlan_entity vlan)
{
	logout("coming into vlan config\n");
	//删除，添加修改vlan
//	printf("----------------------------------------------\n");
//	printf("vlan 的index=%d\n", vlan->vlan_index);
	if(vlan->vlan_index!=255 && vlan->vlan_index!=0)
{
	if (vlan->vlan_index & 0x80)
	{
		//printf("进入vlan设置\n");
		char buf[100];
		char buf1[100];
		int m;
		m = vlan->vlan_index & 0x7F;
		sprintf(buf1, "vlan%d", m);
		sprintf(buf, "uci delete network.%s", buf1);
		system(buf);
		system("uci network commit");
		return;
	}
	else
	{
		//非删除则是修改或者添加
		//拼接口名--vlan+id形式
		if (vlan->vlan_index != 0)
		{
			char buf1[100];
			int m;
			m = vlan->vlan_index;
			if (m < 0)
				return;
			sprintf(buf1, "vlan%d", m);
			char buf[100];
			sprintf(buf, "uci set network.%s=interface", buf1);
			system(buf);
			char buf2[100];
			sprintf(buf2, "uci set  network.%s.ifname='eth0.1'", buf1);
			system(buf2);
			//终端认证模式
			//telnet_auth();
			//vlan整体带宽
			//vlan_bw();
			//商业wifi整体tcp/udp session限制
			// vlan_session();
			//ip划分
			//vlan_ip();
			//终端带宽设置
			//net_bw();
		}
	}
}
	logout("vlan end config\n");
}

void on_ap_runtime_conf(pap_net_conf2 body)										// 配置配置相关
{
	logout("coming into an_ap_runtime_conf\n");
	void* tp = (void*)body->lan;
	int lan_count = htons(body->lan_count);										// 打印各种接受配置
	for(int i = 0;i < lan_count;i++)
	{
		plan_entity tlan = (plan_entity)tp + i;
		on_ap_lan_conf(tlan);
	}
	int vlan_count = htons(body->vlan_count);

	tp += sizeof(lan_entity) * lan_count;
	for(int i = 0;i < vlan_count;i++)
	{
		pvlan_entity tvlan = (pvlan_entity)tp + i;
		on_ap_vlan_conf(tvlan);
		tp += htons(tvlan->node_len)  * sizeof(vlan_node);
	}
	int wan_count = htons(body->wan_count);

	tp += vlan_count * sizeof(vlan_entity);
	for(int i = 0;i < wan_count;i++)
	{
		pwan_entity2 twan = (pwan_entity2)tp + i;
		on_ap_wan_conf(twan);
	}
	int wlan_count = htons(body->wlan_count);

	tp += wan_count * sizeof(wan_entity2);
	int len = 0;
	for(int i = 0;i < wlan_count;i++)
	{
		void* temp_tp = tp + len;
		pwlan_entity2 twlan =  (pwlan_entity2)temp_tp + i ;
		logout(">>[%d]white_len:%d black_len:%d\n",i,htons(twlan->white_len),htons(twlan->black_len));
		len += htons(twlan->white_len) + htons(twlan->black_len);
		on_ap_wlan_conf(twlan);
	}
	system("uci commit wireless");
	system("uci commit dhcp");
	system("uci commit network");
	system("/etc/init.d/network restart");
	system("/etc/init.d/wireless restart");
	system("/etc/init.d/dhcp restart");

}

static void ap_base_conf(pap_dev_base_conf tp)
{
	logout("coming into ap_base_conf\n");

	logout("ap_base_conf end\n");
}

static void ap_safe_conf(pap_safe_dev_conf tp)
{
	logout("coming into ap_safe_conf\n");

	logout("ap_safe_conf end\n");
}

void on_ap_dev_conf(pap_dev_conf2 body)
{
	logout("coming into on_ap_dev_conf\n");
	switch(body->type)
	{
		case base_dev_conf:
		{
			pap_dev_base_conf tp = (pap_dev_base_conf)body->dev_conf;
			ap_base_conf(tp);
			break;
		}
		case safe_dev_conf:
		{
			pap_safe_dev_conf tp = (pap_safe_dev_conf)body->dev_conf;
			ap_safe_conf(tp);
			break;
		}
	}
	logout("dev config end\n");
}

static __u32 getfreestroge()
{
	struct statfs diskInfo;
	char* path = (char*)NULL;
	path = getenv("HOME");
	if(!path)
		return -1;

 	statfs(path, &diskInfo);
	__u32 freeblocksize = (diskInfo.f_bsize *  diskInfo.f_bfree) >> 10;

    return freeblocksize;
}

#if 0
int MyExec( const char* cmd )
{
    FILE *pp = popen( cmd, "r" ); //建立管道
    if ( !pp )
    {
    	printf( "[ MyExec ]:open pipe failed\n" );
      return( 0 );
    }
    char buf[ 1024 ] = { 0 };
    while( fgets( buf, 1024 - 1, pp ) != NULL )
     {
        printf( "*********%s", buf );
     }
    pclose( pp ); //关闭管道
    return( 1 );
}
#endif
int CheckFileIsIntact( const char *pcFileName , unsigned char *pcFileCode )
{
	if( ( pcFileName == NULL ) || ( pcFileCode == NULL ) )
	{
		printf( "[ CheckFileIsIntact ]:param error" );
		return( 0 );
	}
	 MD5_CTX       context;
	 FILE *        fp;
	 unsigned char digest[ 16 ] = { 0 };
	 unsigned char cBuf[ 4096 ] = { 0 };
	 int           iNum;
	 if( ( fp = fopen ( pcFileName, "rb" ) ) == NULL )
	 {
		 printf( "[ CheckFileIsIntact ]:open file:%s failed\n", pcFileName );
		 return( 0 );
	 }

	 MD5Init( &context );
	 while ( ( iNum = fread( cBuf, 1, sizeof( cBuf ), fp ) ) != 0 )
	 {
	     MD5Update( &context, cBuf, iNum);
	 }
	 MD5Final( digest, &context );
	 fclose (fp);

	 if( strncmp( ( char * )digest, ( char * )pcFileCode, 16 ) != 0 )
	 {
		 //printf( "[ CheckFileIsIntact ]:file is intact, src code:%s, get code:%s\n", digest, pcFileCode );
		 for( int i = 0; i < 16; i++ )
		 {
			 printf( "src code:%02x, txt code:%02x\n", digest[i], pcFileCode[i] );
		 }
		 return( 0 );
	 }
	 return( 1 );
}

int getFileCode( const char *pcFileName , unsigned char *pcFileCode )
{
	 if( pcFileName == NULL )
	 {
		 printf( "[ getFileCode ]:param error" );
	 	 return( 0 );
	 }
	 FILE * fp;
    int    iLen;

	 if( ( fp = fopen ( pcFileName, "r" ) ) == NULL )
	 {
		 printf( "[ getFileCode ]:open file:%s failed\n", pcFileName );
		 return( 0 );
	 }
/*	 while( fgets( ( char * )pcFileCode, 16, fp ) != NULL )
	 {
		 iLen = strlen( ( char * )pcFileCode );
		 if( pcFileCode[ iLen - 1 ] == '\n' )
		 {
			 pcFileCode[ iLen - 1 ] = 0;
		 }
	 }*/
	 fgets( ( char * )pcFileCode, 33, fp );
	 fclose( fp );
	 return( 1 );
}

void clean_upgrade_file( )
{
	MyExec( "rm /tmp/upgrade.tar.gz" );
	MyExec( "rm /tmp/exe.tar.gz" );
	MyExec( "rm /tmp/execode.txt" );
	MyExec( "rm /tmp/upgrade.sh" );
	MyExec( "rm /tmp/shcode.txt" );
}

void on_upgrade(string url)
{
	logout("on_upgrade start!\n");
    char  cCmd[ 1024 ]    = { 0 };
    unsigned char  cFileCode[ 33 ] = { 0 };

    sprintf( cCmd, "wget -O /tmp/upgrade.tar.gz %s", url.c_str() );

    MyExec( cCmd );

    if( access( "/tmp/upgrade.tar.gz", F_OK ) == -1 )
    {
    	printf( "this file is not upgrade.tar.gz\n" );
    	return;
    }
    MyExec( "tar -zvxf /tmp/upgrade.tar.gz -C /tmp" );

    if( ( access( "/tmp/exe.tar.gz", F_OK ) == -1 ) || ( access( "/tmp/execode.txt", F_OK ) == -1 ) || ( access( "/tmp/upgrade.sh", F_OK ) == -1 ) || ( access( "/tmp/shcode.txt", F_OK ) == -1 ) )
     {
        printf( "[ main ]:file client or clientcode.txt unexit\n" );
        clean_upgrade_file();
        return;
     }
    else
     {
    	bzero( cFileCode, sizeof( cFileCode ) );
    	if( getFileCode( "/tmp/execode.txt", cFileCode ) == 0 )
    	{
    		printf( "get execode failed\n" );
    		clean_upgrade_file();
    		return;
    	}

    	if( CheckFileIsIntact( "/tmp/exe.tar.gz", cFileCode ) == 0 )
    	{
    		printf( "execode error\n" );
    		clean_upgrade_file();
    		return;
    	}

    	bzero( cFileCode, sizeof( cFileCode ) );
    	if( getFileCode( "/tmp/shcode.txt", cFileCode ) == 0 )
    	{
    		printf( "get chcode failed\n" );
    		clean_upgrade_file();
    		return;
    	}

    	if( CheckFileIsIntact( "/tmp/upgrade.sh", cFileCode ) == 0 )
    	{
    		printf( "chcode error\n" );
    		clean_upgrade_file();
    		return;
    	}

    	MyExec( "chmod +x /tmp/upgrade.sh" );
    	printf( "execuate upgrade.sh\n" );
    	execl( "/tmp/upgrade.sh", NULL );

     }
 //   clean_upgrade_file();
    logout("on_upgrade end!\n");
}

static __u16 cpuinfo(CPU_t* cpus)
{
	FILE *fp;
	int num;
	char buf[SMLBUFSIZ];
    if((fp = fopen("/proc/stat", "r")));
	if(!fp)
	{
		logout("open /proc/stat error\n");
		return -1;
	}
	rewind(fp);
	fflush(fp);

	if (!fgets(buf, sizeof(buf), fp))
	{
		logout("failed /proc/stat read\n");
		fclose(fp);
		return -2;
	}
	num = sscanf(buf, "cpu %Lu %Lu %Lu %Lu %Lu %Lu %Lu %Lu",
	      &cpus->u,
	      &cpus->n,
	      &cpus->s,
	      &cpus->i,
	      &cpus->w,
	      &cpus->x,
	      &cpus->y,
	      &cpus->z
	);
	if (num < 4)
		logout("failed /proc/stat read\n");

	double temp1 = cpus->u + cpus->n + cpus->s;
	double temp2 = temp1 + cpus->i + cpus->w + cpus->x + cpus->y + cpus->z;

	fclose(fp);
	return (temp1/temp2)*10000;
}
static void ApInfoByStdUC(pap_stat2 ap)
{
	/* 获取AP剩余内存 */
	struct sysinfo s_info;
	int err = sysinfo(&s_info);//获取系内存情况
	if(err < -1)
		return ;
	ap->free_mem = ntohl((__u32)(s_info.freeram >> 10));  			//内存单位为KB
	/* 获取剩余硬盘大小 */
	ap->free_storage = ntohl(getfreestroge());						//硬盘单位KB
	/* 获取AP运行时间 */
	struct timespec pt = {0,0};
//	clock_gettime(CLOCK_MONOTONIC,(struct timespec*)&pt);
	clock_gettime(CLOCK_MONOTONIC,&pt);
	ap->uptime = ntohl(pt.tv_sec);									// 获取ap在线时间
	/* 获取系统时间 */
	struct timeval TV;
	gettimeofday((struct timeval*)&TV, (struct timezone*)NULL);
	ap->ap_time = ntohl(TV.tv_sec);									//获取系统时间
	/* 获取CPU使用率 */
	CPU_t cpu;
	ap->cpu = htons(cpuinfo(&cpu));
	ap->user_interval = ntohs(UserInterval);
	ap->audit_interval = ntohs(0);
	logout("free_mem:%u\t\nfree_storage:%u\t\nuptime:%u\t\nap_time:%u\t\ncpu:%u\t\n",
			htonl(ap->free_mem),htonl(ap->free_storage),htonl(ap->uptime),htonl(ap->ap_time),htons(ap->cpu));
}

int  GetSysIpBySocket(const char* interface,void* pAddr)
{
    if((interface == NULL)||(pAddr == NULL))
        return -1;

    struct sockaddr_in *sin;
    struct ifreq        ifr;

    memset(&ifr,0,sizeof(struct ifreq));
    int sock = socket(AF_INET,SOCK_STREAM,0);
    if(sock <= 0)
    {
        perror("socket error!");
        return -2;
    }

    memcpy(ifr.ifr_name,interface,strlen(interface) > IFNAMSIZ ? IFNAMSIZ:strlen(interface));

    if(ioctl(sock,SIOCGIFADDR,&ifr) < 0)
    {
        perror("ioctl error");
        close(sock);
        return -3;
    }else
    {
        sin = (struct sockaddr_in *)&(ifr.ifr_addr);
        memcpy(sin,pAddr,4);
//        strcpy(pAddr, inet_ntoa(sin->sin_addr));
        close(sock);
    }
    return 0;
}

int  GetSysMaskBySocket(const char* interface,void* pMask)
{
    if((interface == NULL)||(pMask == NULL))
        return -1;

    struct sockaddr_in *sin;
    struct ifreq        ifr;

    bzero(&ifr,sizeof(struct ifreq));
    int sock = socket(AF_INET,SOCK_DGRAM,0);
    if(sock <= 0)
    {
        perror("socket error!");
        return -2;
    }

    memcpy(ifr.ifr_name,interface,strlen(interface) > IFNAMSIZ ? IFNAMSIZ : strlen(interface));
    printf("interface:%s len = %d\n",ifr.ifr_name,strlen(interface));

    if(ioctl(sock,SIOCGIFNETMASK,&ifr) < 0)
    {
        perror("ioctl error");
        close(sock);
        return -3;
    }else
    {
        sin = (struct sockaddr_in *)&(ifr.ifr_netmask);
        memcpy(sin,pMask,4);
 //       strcpy(pMask, inet_ntoa(sin->sin_addr));
        close(sock);
    }

    return 0;
}

void ap_usraction(usr_action* action)
{
	THSockContext* sc = ec->FindContext(true);
	if ((sc == NULL)||!(sc->Logined)) return;
	struct sockaddr_in local_ip;
	struct sockaddr_in local_mask;
	__u32 *pip = (__u32*)&local_ip;
	__u32 *pmask = (__u32*)&local_mask;
	memset(&local_ip,0,sizeof(local_ip));
	memset(&local_mask,0,sizeof(local_mask));
	GetSysMaskBySocket(ec->Option.Other.interface.c_str(),&local_mask);
	GetSysIpBySocket(ec->Option.Other.interface.c_str(),&local_ip);

	user_action2      usraction2;
	user_action_entry entry;
	struct in_addr    addr;
	char dst[32] = "";
	char mac[6] = "";

	if((*(__u32*)&(action->src.addr) & *pmask) == ( *pip & *pmask))
	{
		addr.s_addr  = action->src.addr;
		memcpy(mac,action->src.mac,6);
		snprintf(dst,32,"%s:%d",inet_ntoa(*(struct in_addr*)&(action->dst.addr)),action->dst.port);
	}else
	{
		addr.s_addr = action->dst.addr;
		memcpy(mac,action->dst.mac,6);
		snprintf(dst,32,"%s:%d",inet_ntoa(*(struct in_addr*)&(action->src.addr)),action->src.port);
	}

	bzero( g_pcUsrAction, MAX_BODY_SIZE);
	bzero(&usraction2,sizeof(usraction2));
	bzero(&entry,sizeof(entry));
	memcpy(usraction2.user_ip ,&addr,4);
	memcpy(usraction2.mac,mac,6);

	usraction2.entry_count = htons( 1 );
	memcpy( g_pcUsrAction, ( char * )&usraction2, sizeof( user_action2 ) );
	entry.action_date = htonl( action->actdate );
	entry.action      = htons( action->type );

	if( action->type == 0 )
	{
		entry.data_len = htons( action->len );
		memcpy( g_pcUsrAction + sizeof( user_action2 ), ( char* )&entry, sizeof( user_action_entry ) );
		memcpy( g_pcUsrAction + sizeof( user_action2 ) + sizeof( user_action_entry ), ( char * )action + sizeof( usr_action ), action->len );

		sc->Response->ResponseCode = msg_cmd_user_action2;//继续填
		sc->Response->ResponseStream.Clear();
		sc->Response->ResponseStream.WriteBuffer(g_pcUsrAction, sizeof( user_action2 ) + sizeof( user_action_entry ) + action->len );
		sc->Response->AbortGZIP = false;
		sc->Response->AutoAcknowledge = false;
		ec->BuildResponse(sc, true);
	}
	else
	{
//		entry.data_len = htons( sizeof( UIntToStr(action->src.port).c_str() ) );
		entry.data_len = 32;
		memcpy( g_pcUsrAction + sizeof( user_action2 ), ( char* )&entry, sizeof( user_action_entry ) );
//		memcpy( g_pcUsrAction + sizeof( user_action2 ) + sizeof( user_action_entry ), UIntToStr(action->src.port).c_str(), strlen( UIntToStr(action->src.port).c_str() ) );
		memcpy( g_pcUsrAction + sizeof( user_action2 ) + sizeof( user_action_entry ), dst,32);

		sc->Response->ResponseCode = msg_cmd_user_action2;//继续填
		sc->Response->ResponseStream.Clear();
//		sc->Response->ResponseStream.WriteBuffer(g_pcUsrAction, sizeof( user_action2 ) + sizeof( user_action_entry ) + strlen( UIntToStr(action->src.port).c_str()) );
		sc->Response->ResponseStream.WriteBuffer(g_pcUsrAction, sizeof( user_action2 ) + sizeof( user_action_entry ) + 32 );
		sc->Response->AbortGZIP = false;
		sc->Response->AutoAcknowledge = false;
		ec->BuildResponse(sc, true);
	}
}

void ap_heartbeat(pap_stat2 as)
{
	 THSockContext* sc = ec->FindContext(true);
	 if ((sc == NULL) || !(sc->Logined))
    {
		 logout("client is not login to server\n");
		return;
	}
	 TeWiFiClient* sv = (TeWiFiClient*) sc->Server;
	 as->auth_mode = sv->Option.Other.should_auth;
	 as->audit_mode = sv->Option.Other.should_audit;

	 ApInfoByStdUC(as);												// 标准

	 sc->Response->ResponseCode = msg_cmd_ap_stat2;//继续填
	 sc->Response->ResponseStream.Clear();
	 sc->Response->ResponseStream.WriteBuffer(as, sizeof(ap_stat2));
	 sc->Response->AbortGZIP = false;
	 sc->Response->AutoAcknowledge = false;
	 ec->BuildResponse(sc, true);

}
void Cpy(unsigned char *p,unsigned char *s,int n)
{
  int m=strlen((const char *)s);
  for(int i=0;i<m;i++)
{
    *(p+i) = *(s+i);
}
}
void sta_heartbeat(puser_stat2 us, int len)
{
	THSockContext* sc = ec->FindContext(true);
	if (sc == NULL)
	{
		logout("client is not login to server\n");
		return;
	}

	list<arp_accept *>::iterator it;
	int k = 0;
	for(it = con.p_arp.begin();it!=con.p_arp.end();it++)
	{
		logout(" [ %d ]  Send Userheartbeat to server\n ",k++);
		if((*it)->state == 2)
		{
			bzero(us,len);
			memcpy(us->mac,(*it)->arp_mac,6);
	        memcpy(us->user_ip,(*it)->arp_ip,12);
	        us->stat = 2;

			sc->Response->ResponseCode = msg_cmd_user_stat2;
			sc->Response->ResponseStream.Clear();
			sc->Response->ResponseStream.WriteBuffer(us, len);
			sc->Response->AbortGZIP = false;
			sc->Response->AutoAcknowledge = false;
			ec->BuildResponse(sc, true);
		}
	}
}

void Rotation()
{
	THSockContext* sc = ec->FindContext(true);
	if (sc == NULL) return;
	user_stat2  us;

	list<arp_accept *>::iterator it;
	int k = 0;
	for(it = con.p_arp.begin();it!=con.p_arp.end();it++)
	{
//		logout("Now state = %d Old state = %d\n",(*it)->state,(*it)->oldstate);
		if(((*it)->state == 1) || ((*it)->state == 3))
		{
			if((*it)->oldstate != (*it)->state)
			{
				bzero(&us,sizeof(us));
				logout(" [ %d ]  Send edge to server\n ",k++);
				memcpy(us.mac,(*it)->arp_mac,6);
		        memcpy(us.user_ip,(*it)->arp_ip,12);
				us.stat = (*it)->state;
				sc->Response->ResponseCode = msg_cmd_user_stat2;
				sc->Response->ResponseStream.Clear();
				sc->Response->ResponseStream.WriteBuffer(&us, sizeof(us));
				sc->Response->AbortGZIP = false;
				sc->Response->AutoAcknowledge = false;
				ec->BuildResponse(sc, true);
			}
			(*it)->oldstate = (*it)->state;
		}
	}
}

void sta_audit(string ip, string mac, __u32 date, string url)
{
	 THSockContext* sc = ec->FindContext(true);
	 if (sc == NULL) return;
	 sc->Response->ResponseCode = msg_cmd_user_action2;
	 sc->Response->ResponseStream.Clear();

	 user_action2 uc;
	 write_ip(uc.user_ip, ip);
	 write_mac(uc.mac, mac);
	 uc.entry_count = htons(1);
	 sc->Response->ResponseStream.WriteBuffer(&uc, sizeof(uc));

	 user_action_entry uae;
	 uae.action = htons(uatURL);
	 uae.action_date = htonl(date);
	 uae.data_len = htons(url.length());
	 sc->Response->ResponseStream.WriteBuffer(&uae, sizeof(uae));
	 sc->Response->ResponseStream.WriteBuffer((void*)url.c_str(), url.length());

	 sc->Response->AbortGZIP = false;
	 sc->Response->AutoAcknowledge = false;
	 ec->BuildResponse(sc, true);
}

#endif /* DRIVER_CPP_ */
