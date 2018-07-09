/*
 * ewifi-client.cpp
 *
 *  Created on: 2013-9-28
 *      Author: root
 */

#ifndef EWIFI_CLIENT_CPP_
#define EWIFI_CLIENT_CPP_
#include"../proj/proj_utils.hpp"
#include "ewifi_client.hpp"
#include"../drv/arp_.hpp"

#include "../hlib/logout.hpp"
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <string.h>
#include <sys/un.h>

#include "wdctl.hpp"
extern int H_BUFFER_LEN;
extern string get_dev_seq();
extern void ap_usraction(usr_action* action);
extern void ap_heartbeat(pap_stat2 as);
extern void Rotation();
extern void Cpy(unsigned char *p,unsigned char *s);
extern void sta_heartbeat(puser_stat2 us, int len);
extern string get_dev_lan_ip();
extern string get_dev_wan_mac();
extern void get_dev_lan_ip(pByte buf);
extern void get_dev_wan_mac(pByte buf);
char* buf = ( char * )malloc(MAX_BODY_SIZE);
//{* main obj *}

extern con_collect con;
#include "../hlib/wdctl.hpp"

bool restart()
{
	sync();
	return reboot(RB_AUTOBOOT);
}

void TeWiFiClient::OnStarted()
{
  //Inherited;
  THEpollServer::OnStarted();
	Log("TeWiFiClient On started");
  #ifdef H_SAFE_MODE
  sc = new THSRSWC(Option.Net.MaxConnection << 2);
  #else
  sc = NULL;
  #endif
  // 0. do some test temporarily

  // 1. load config now
  // statistics
  memset(&stat, 0, sizeof(THStatistics));
  DDRs[0] = new THDataRec("server_send", dtBytes, dmInt64, cmTotal, psSecond, false);
  DDRs[1] = new THDataRec("server_recv", dtBytes, dmInt64, cmTotal, psSecond, false);
  DDRs[2] = new THDataRec("server_time", dtMicroSeconds, dmInt64, cmAverage, psSecond, false);
  DDRs[3] = new THDataRec("server_err", dtBytes, dmInt64, cmTotal, psSecond, false);
  // end;
  reboot = false;

}

bool TeWiFiClient::OnCheckPacketHead(THContext* FClientContext)
{
	PHRequest req = GetRequest(FClientContext);
	pmsg_head2 head = (pmsg_head2)req->Head;
	/*
	if ((head->logo == H_PROTO_LOGO) && (head->ver <= H_PROTO_VER) && (head->len <= H_PROTO_MAX_LEN))
	{
	    req->FormLength = head->len;
		req->PacketLength = head->len + head_len;
		return true;
	}*/
	if (head->logo == htons(H_PROTO_LOGO))
	{
		if (head->ver <= H_PROTO_NEO_VER)
		{
			__u16 hl = ntohs(head->len);
			if (hl <= H_PROTO_MAX_LEN)
			{
				req->FormLength = hl;
				req->PacketLength = hl + head_len;
				return true;
			}
			#ifdef H_DEBUG
			else
				printf("/");
			#endif
		}
		#ifdef H_DEBUG
		else
			printf("|");
		#endif
	}
	#ifdef H_DEBUG
	else
		printf("*");
	#endif
	return false;
}

bool TeWiFiClient::OnCheckPacketBody(THContext* FClientContext)
{
	PHRequest req = GetRequest(FClientContext);
	pmsg_head2 head = (pmsg_head2)req->Head;
	if (head->len == 0) return true;
	__u32 crc = 0; __u32 hcrc = ntohl(head->crc32c);
	crc = crc32c(crc, (char*)req->Body, req->FormLength);
	if (crc != hcrc)
	{
		#ifdef H_DEBUG
		printf("<!!> bad crc: %d, should be %d\n", hcrc, crc);
		#endif
		return false;
	}
	if (head->flag & mhf_GZIP)
	{
		int buf_len = content_size;
		int ret = GZDecompress((const char*)req->Body, req->FormLength, (char*)req->Content, &buf_len);
		if (!ret)
		{
			#ifdef H_DEBUG
			printf("<!!> UNGZIP error\n");
			#endif
			req->ContentLength = req->FormLength;
			req->Data = req->Body;
		}
		else
		{
			#ifdef H_DEBUG
			printf("<!!> UNGZIP OK, len: %d\n", buf_len);
			#endif
			req->ContentLength = buf_len;
			req->Data = req->Content;
		}
	}
	else
	{
		#ifdef H_DEBUG
		printf("<!!> NO ZIP\n");
		#endif
		req->ContentLength = req->FormLength;
		req->Data = req->Body;
	}
	return true;
}

void TeWiFiClient::ProcessRequest(THSockContext* FClientContext)
{
	logout("coming into ProcessReques\n");
	PHRequest req = GetRequest(FClientContext);
	pmsg_head2 head = (pmsg_head2)req->Head;
	__u16 hc = ntohs(head->cmd);
	logout("cmd=%d,head->cmd=%d\n",hc,head->cmd);
	#ifndef H_PRINT_ACK
	if (hc != msg_cmd_ack2)
	#endif
	Log("(>>) cmd [ " + GetCommandDisplayName(hc) + " : " + IntToStr(FClientContext->Socket) + " # "
	  + IntToStr(ntohl(head->seq)) + " @ " + FClientContext->PeerIP
	  + " : " + IntToStr(FClientContext->PeerPort) + " ] ", H_CAUTION);
	logout("begin into DispathCommand\n");
    do {
    	// dynamic page, put it first for performance
    	if (DispatchCommand(FClientContext, hc)) break;

    	// cannot be recognized
    	Log("[!!] Invalid incoming command, close connection.", H_EMERGENCY);
    	CloseContext(FClientContext);
    	return;
    } while (false);
    BuildResponse(FClientContext);
}

void TeWiFiClient::BuildResponse(THSockContext* FClientContext, bool safe)
{
	// 1. local var prepare
	logout("coming into BuildResponse\n");
	PHRequest req = FClientContext->Request;
	PHResponse resp = FClientContext->Response;
	pmsg_head2 head;
	msg_head2 local_head;
	if (safe)
	{
		memset(&local_head, 0, sizeof(msg_head2));
		head = &local_head;
		head->logo = htons(H_PROTO_LOGO);
	}
	else
	   head = (pmsg_head2)req->Head;
    // 2. output
	if (resp->AutoAcknowledge) Acknoledge(FClientContext);
	if (resp->ResponseCode == msg_cmd_unknown) return;
	// logo, no need reset, 鍊熺敤涓�涓�
	head->ver = H_PROTO_NEO_VER;
	head->flag = mhf_NONE;
	head->seq = htonl(FClientContext->local_seq++);
	// process body
	head->cmd = htons(resp->ResponseCode);
	pByte data; __u16 data_len;
	Log("[**] Response size: [ " + IntToStr(resp->ResponseStream.Size) + " ].", H_COMMON);
	if ((!resp->AbortGZIP) &&       // 娌¤姹備笉鍘嬬缉
			(resp->ResponseStream.Size > 1000)) // 纭疄闇�瑕佸帇缂�
	{
		int buf_len = content_size;
		if (GZCompress((const char*)resp->ResponseStream.Memory, resp->ResponseStream.Size, (char*)req->Content, &buf_len))
		{
			head->flag |= mhf_GZIP;
			data_len = buf_len;
			data = req->Content;
			Log("[**] GZip to [ " + IntToStr(buf_len) + " ] bytes.", H_COMMON);
		}
		else
		{
			data_len = (__u16)resp->ResponseStream.Size;
			data = resp->ResponseStream.Memory;
		}
	}
	else
	{
		data_len = (__u16)resp->ResponseStream.Size;
		data = resp->ResponseStream.Memory;
	}
	head->len = htons(data_len);
	head->crc32c = htonl(crc32c((__u32)0, (char*)data, data_len));

	// 3. write to buffer
	FClientContext->SendData((pByte)head, sizeof(msg_head2));
	FClientContext->SendData(data, data_len);
	resp->ResponseStream.Clear();
	//FClientContext->SessionStatus = ssWaitOutput;
	FClientContext->SessionStatus = ssWaitHead;
	Log("(<<) cmd [ " + GetCommandDisplayName(resp->ResponseCode) + " ] of socket [ " + IntToStr(FClientContext->Socket) + " ]: wrote out " + IntToStr(data_len + sizeof(msg_head2)) + " byte(s).", H_COMMON);
}

void TeWiFiClient::Acknoledge(THSockContext* FClientContext)
{
	// 1. local var prepare
	PHRequest req = FClientContext->Request;
	pmsg_head2 head = (pmsg_head2)req->Head;
	// 2. output
	// logo, no need reset, 鍊熺敤涓�涓�
	head->ver = H_PROTO_NEO_VER;
	head->flag = mhf_NONE; //len == 0 ? mhf_NONE : mhf_ACK;
	head->cmd = htons(msg_cmd_ack2);
	head->len = 0;
	FClientContext->SendData((pByte)head, sizeof(msg_head2));
    #ifdef H_PRINT_ACK
	Log("(<<) ACK [ " + IntToStr(ntohl(head->seq)) + " ]: sent. ", H_COMMON);
	#endif
}

TeWiFiClient::TeWiFiClient():THSockServer()
{
	// reset project settings
	head_len = H_HEAD_LEN2;
	body_max = H_BUFFER_LEN;
	content_size = H_BUFFER_LEN << 1;
	// go
	AlarmCount = 0;
	reboot = false;

	FDate = 0;
	UsrDate = 0;
	ListDate = 0;
	DevDate = 0;
#ifndef POWER
	PowerOn();
#endif
	Log("PowerOn End.", H_DEADLY);
}

TeWiFiClient::~TeWiFiClient()
{
    for(int i = 0; i < 4; i++) delete DDRs[i];
}

void TeWiFiClient::ClearConnection()
{
	int cc = GetContextCount();
	double n = Now();
	for (int i = 0; i < cc; i++)
	{
		THSockContext* sc = (THSockContext*)GetContext(i);
		//printf("%s: %d: %d\n", FormatDateTime("hh:nn:ss", Now()).c_str(), i, (THPointer)sc);
		if (!sc || !sc->Connected) continue;
		if (!sc->Logined && (sc->ConnectDate == sc->LastDate)
				&& ((n - sc->ConnectDate) > (H_SEC * 30))) // 30绉掍笉鍙戝寘
		{
			Log("(xx) Clean [ " + IntToStr(sc->Socket) + " ] of [ " + sc->PeerIP
						+ " : " + IntToStr(sc->PeerPort) + " ]: Too long to wait 1st packet.", H_CAUTION);
			CloseContext(sc);
			continue;
		}
		if (!sc->Logined && ((n - sc->ConnectDate) > (H_SEC * 120))) // 120绉掍笉鐧诲綍
		{
			Log("(xx) Clean [ " + IntToStr(sc->Socket) + " ] of [ " + sc->PeerIP
						+ " : " + IntToStr(sc->PeerPort) + " ]: Too long to login.", H_CAUTION);
			CloseContext(sc);
			continue;
		}
		if ((n - sc->LastDate) > (H_SEC * 300)) // 5鍒嗛挓涓嶆搷浣�
		{
			Log("(xx) Clean [ " + IntToStr(sc->Socket) + " ] of [ " + sc->PeerIP
						+ " : " + IntToStr(sc->PeerPort) + " ]: Idle was too long.", H_CAUTION);
			CloseContext(sc);
			continue;
		}
	}
}
#ifdef	USER_INTERVAL
extern __u16 UserInterval;
extern __u16 UserMsgInterval;
#endif

void TeWiFiClient::SplitStr(string str)
{
	char buf[1024] = "";
	memset(buf,0,1024);
	memcpy(buf,str.c_str(),str.length() > 1023 ? 1023 : str.length());
	char *cp,*cp2;
	cp = cp2 = buf;
	while(isalpha(*cp2))
		cp2++;
	if(cp2 >= (buf + str.length()))
		return ;
	*cp2++ = 0;
	while(*cp2 == ' ')
		cp2++;

	string type = string(cp);
	string data = string(cp2);

	if(type == "devid")
		Option.Other.dev_id = data;
	else
		if(type == "interface")
			Option.Other.interface = data;
		else
			if(type == "MAC")
				Option.Other.MAC = data;
	logout("%s %s\n",type.c_str(),data.c_str());
}


static int connect_serv(void)
{
	int sockfd;
	struct sockaddr_un addr;

	sockfd = socket(AF_LOCAL,SOCK_STREAM,0);
	if(sockfd < 0)
		return -1;

	bzero(&addr,sizeof(addr));
	addr.sun_family = AF_LOCAL;
	strcpy(addr.sun_path,UNIXSTR_PATH);
#if 0
	if(!connect(sockfd,(struct sockaddr*)&addr,SUN_LEN(&addr)))
		printf("connect is succeful\n");
#endif
	if(!connect(sockfd,(struct sockaddr*)&addr,SUN_LEN(&addr)))
		return sockfd;
	else
	{
		close(sockfd);
		return 0;
	}
}

static int write_srv(int fd,char* argv)
{
	char cbuf[1024];
	bzero(cbuf,1024);
	strncpy(cbuf,argv,1024);
	int len = strlen(cbuf);
	sprintf(cbuf + len,"\r\n");
	return send(fd,cbuf,strlen(cbuf),0);
}

usr_action* ReadMsg( void )
{
	int sockfd;

	char argv[] = "GET True";

	sockfd = connect_serv();
	if(sockfd <= 0)
	{
//		logout("Conect_serv is error\n");
		return NULL;
	}
	if(-1 == fcntl(sockfd, F_SETFL, O_NONBLOCK))
	{
		close(sockfd);
		return NULL;
	}
	int ret = write_srv(sockfd,argv);
	if(ret <= 0)
	{
		close(sockfd);
		return NULL;
	}
	bzero(buf,MAX_BODY_SIZE);
	int count = 0,size = 0;
	char ch,old;

	while(1)
	{
		int ret = recv(sockfd,&ch,1,0);
		if(ret == 1)
		{
			buf[size] = ch;
			size += ret;
			if((ch == '\n') && (old == '\r'))
			{
				buf[size - 1] = 0;
				buf[size - 2] = 0;
				shutdown(sockfd,2);
				close(sockfd);
				return (usr_action*)buf;
			}
			old = ch;
		}
		else
		{
			shutdown(sockfd,2);
			close(sockfd);
			break;
		}
	}
	close(sockfd);
	return NULL;
}

void TeWiFiClient::HandleUnix(const char* cmd,const char* body)
{
	logout("cmd %s Body %s\n",cmd,body);
	if (strncasecmp(cmd, "stop", 6) == 0)
		exit(0);
	else if (strncasecmp(cmd, "SaveLog", 6) == 0)
			MyLog->SaveToFile();
}

void TeWiFiClient::Patrol(THEpollServer* server, bool LowSpeedEvent)
{
	// -1. process high level events first
	// RUN MAIN FUNCTIONS HERE
	//if (!LowSpeedEvent) return;
	//Int64 t = GetTickCount();
	double a = Now();
	if ((a - DevDate) > (H_SEC * 2))
	{
//		DevDate = a;
//		char* dev =  wdctl_getdevid();
//		if(dev)
//		{
//			if((strlen(dev) == 16))
//			{
//				Option.Other.dev_id = string(dev);
//				SaveConfig(ChangeFileExt(AppName(), ".ini"));
//				Log("update devid " + Option.Other.dev_id + " begin reconnect to server\n");
//				THSockContext* sc = FindContext(true);
//				if(sc)
//					sc->KillSocket();
//			}
//			free(dev);
//		}
	}

	if ((a - ListDate) > (H_SEC * 2))
	{
		ListDate = a;
		con.update_list(Option.Other.interface.c_str());
		Rotation();
	}

	// 0. do common jobs
	if ((a - FDate) > (H_SEC * 30))
	{
		FDate = a;
		// 1. do anothing loops
		ClearConnection();
		//return ;
		{
			ap_stat2 as;
			memset(&as, 0, sizeof(ap_stat2));
			ap_heartbeat(&as);
		}
		con.del_list();
		user_stat2 als;
		memset(&als, 0, sizeof(user_stat2));
		logout("coming into Send Usr_stat\n");
		sta_heartbeat(&als, sizeof(als));
		// TODO: add calling

		list<arp_accept *>::iterator it;
		for (it = con.p_arp.begin(); it != con.p_arp.end(); it++)
		{
			if ((*it)->state == 3)
			{
				int sock = connect_to_server(DEFAULT_SOCK);
				char request[1024];
				string mac = read_mac((*it)->arp_mac);
				sprintf(request, "del %s\r\n\r\n", mac.c_str());
				string MAC = read_mac((*it)->arp_mac);
				send_request(sock, request);
				close(sock);
			}
		}
		ClearSession();
	}

	if ((a - UsrDate) > ((H_SEC / 1000) * UserMsgInterval))
	{
		UsrDate = a;
		{
			usr_action* action = ReadMsg();
			if (action) {
				logout("get usr_action_msg\n");
				ap_usraction(action);
			}
		}
	}

	// 9. wrote log out;
#ifdef H_DEBUG
#ifdef H_PRINT_MEMORY
	printf("*****************\n");
#ifdef H_MIPS
	malloc_stats(stdout);
#else
	malloc_stats();
#endif
#endif
#ifdef H_PRINT_NETWORK
	Log(server->RunDiagnosis(), H_COMMON);
#endif
#endif

}

// only for demo

void TeWiFiClient::SendMsg(THSockContext* FClientContext, __u8 cmd, void* buf, int len, bool compress)
{
	// 1. local var prepare
	Byte tmp_buf[H_BUF_LEN];
	msg_head2 head;
	head.logo = htons(H_PROTO_LOGO);
    // 2. output
	head.ver = H_PROTO_VER;
	head.flag = mhf_NONE;
	head.seq = htonl(FClientContext->local_seq++);
	// process body
	head.cmd = htons(cmd);
	pByte data; __u16 data_len;
	Log("[**] SendMsg size: [ " + IntToStr(len) + " ].", H_CAUTION);
	if (compress && (len > 0))
	{
		int buf_len = content_size;
		if (GZCompress((const char*)buf, len, (char*)tmp_buf, &buf_len))
		{
			head.flag |= mhf_GZIP;
			data_len = buf_len;
			data = tmp_buf;
			Log("[**] GZip to [ " + IntToStr(buf_len) + " ] bytes.", H_COMMON);
		}
		else
		{
			data_len = len;
			data = (pByte)buf;
		}
	}
	else
	{
		data_len = len;
		data = (pByte)buf;
	}
	head.len = htons(data_len);
	head.crc32c = htonl(crc32c((__u32)0, (char*)data, data_len));

	// 3. write to buffer
	FClientContext->SendData((pByte)&head, sizeof(msg_head2));
	FClientContext->SendData(data, data_len);
	//FClientContext->SessionStatus = ssWaitOutput;
	FClientContext->SessionStatus = ssWaitHead;
	Log("(<<) cmd [ " + GetCommandDisplayName(cmd) + " ] of socket [ " + IntToStr(FClientContext->Socket) + " ]: wrote out " + IntToStr(data_len + sizeof(msg_head2)) + " byte(s).", H_COMMON);
}

void TeWiFiClient::SendDemoPacks(bool compress)
{
	int cc = GetContextCount();
	//printf("SendDemoPacks: %d\n", cc);
	for (int i = 0; i < cc; i++)
	{
		THSockContext* sc = (THSockContext*)GetContext(i);
		//printf("%s: %d: %d\n", FormatDateTime("hh:nn:ss", Now()).c_str(), i, (THPointer)sc);
		if (!sc) continue;
		if (!sc->Logined) continue;
		int noir = random() % 3;
		int dice = random() % 3; // 1/2
		if (dice == 0) // send command msg_cmd_ap_dns_white
		{
			string white = "";
			switch(noir)
			{
				case 0:
				{
					white = ".sina.com.cn";
					break;
				}
				case 1:
				{
					white = ".qq.com";
					break;
				}
				case 2:
				{
					white = ".alipay.com";
					break;
				}
			}
			SendMsg(sc, msg_cmd_ap_dns_white2, (void*)white.c_str(), white.length(), compress);
		}
		if (dice == 1) // send command msg_cmd_ap_conf
		{
#if 0
			ap_conf ac;
			ac.ap_interval = htons(30);
			ac.audit_interval = htons(30);
			ac.audit_mode = noir > 0 ? aam_AUDIT : aam_NONE;
			ac.auth_mode = noir > 0 ? aam_AUTH : aam_PASS;
			ac.biz_ibw_limit = htonl((noir + 1) << 16);
			ac.biz_obw_limit = htonl((noir + 1) << 12);
			ac.ext_conf_len = 0;
			ac.wan_conf_count = 0;
			ac.wlan_conf_count = 0;
			SendMsg(sc, msg_cmd_ap_conf, &ac, sizeof(ap_conf), compress);
#endif
		}
		if (dice == 2) // send command msg_cmd_ap_upgrade
		{
			string upg = "";
			switch(noir)
			{
				case 0:
				{
					upg = "ftp://just.a.sample/useless.zip";
					break;
				}
				case 1:
				{
					upg = "https://task.it.easy/never_exists.bin";
					break;
				}
				case 2:
				{
					upg = "http://this.is.only/a#demo";
					break;
				}
			}
			SendMsg(sc, msg_cmd_ap_upgrade2, (void*)upg.c_str(), upg.length(), compress);
		}
	}
}

void TeWiFiClient::OnNew(THContext* FClientContext)
{
	Log("(++) New connection [ " + IntToStr(FClientContext->Socket) + " ] to [ " + FClientContext->PeerIP + " : " + IntToStr(FClientContext->PeerPort) + " ]..", H_CAUTION);
	// 褰撻噸鏂拌繛涓婅璇佸钩鍙板簲璇ラ噸鏂版竻闄よ鍒�
	wdctl_OnPowr();

	THSockContext* cc = (THSockContext*) FClientContext;
	TeWiFiClient* srv = (TeWiFiClient*) (cc->Server);
	cc->local_seq = 0;
	cc->peer_seq = 0;
	cc->Response->AutoAcknowledge = false;
	cc->Response->AbortGZIP = false;
	cc->Connected = true;
	cc->Logined = false;
	cc->MAC = get_dev_wan_mac();
	cc->IP = get_dev_lan_ip();
	// 1. automatic conn-request
//	ap_conn_req2 body;
//	memset(&body, 0, sizeof(ap_conn_req2));
//	body.soft_id = htonl(srv->Option.Other.hard_id);
//	body.hard_id = htonl(mi_ewifi);
////	write_hardseq(body.hard_seq, get_dev_seq());
//	strcpy((char*)body.hard_seq,VERSION_SOFT);
//	strcpy((char*)(body.hard_seq + 32),VERSION_HARD);
//
//	write_devid(body.dev_id, srv->Option.Other.dev_id);
//	body.ability = htonl(0x8);
//	body.ap_time = GetSeconds();
//	char *cp = get_iface_ip(srv->Option.Other.interface.c_str());
//	if(cp)
//	{
//		write_ip(body.ip,string(cp));
//		free(cp);
//	}else
//		write_ip(body.ip, string("127.0.0.1"));
//	write_mac(body.mac, srv->Option.Other.MAC);
//
//	PHResponse resp = cc->Response;
//	resp->AutoAcknowledge = false;
//	resp->AbortGZIP = false;
//	resp->ResponseCode = msg_cmd_conn_req2;
//	resp->ResponseStream.Clear();
//	resp->ResponseStream.WriteBuffer(&body, sizeof(ap_conn_req2));
//  TeWiFiClient* srv = (TeWiFiClient*)(cc->Server);
	HEAD head;
	//两类处理 recv/replay
	char body[500] = { "\0" };
	char * ptr = NULL;
	uint32_t i_val;
	//need replay
	head.type = htonl(1);
	head.sequence = htonl(10);
	head.len = htonl(6);
	ptr = body;
	strncpy(ptr, "zhaoxi", 6);
	PHResponse resp = cc->Response;

	srv->BuildResponse(cc, true);
}

void TeWiFiClient::OnClose(THContext* FClientContext)
{
	Log("(--) Disconnected [ " + IntToStr(FClientContext->Socket) + " ] to [ " + FClientContext->PeerIP
			+ " : " + IntToStr(FClientContext->PeerPort) + " ]..", H_CAUTION);
	  // clear context for reuse
	//down();
//	if(wdctl_down() < 0){
//		printf("send Down has error\n");
//		}
	THSockContext* cc = (THSockContext*)FClientContext;
	cc->SessionStatus = ssWaitHead;
	cc->Response->ResponseCode = 200;
	cc->Response->AbortGZIP = false;
	cc->Response->ResponseStream.Clear();
	cc->Connected = false;
	cc->Logined = false;
	if ((cc->Tag > -1) && (cc->Tag < Option.Net.ServerIndex->Count()))
	{ // need reconnect
	  Option.Net.Servers[cc->Tag].stat = ssDisconnected;
	  Option.Net.Servers[cc->Tag].date = GetTickCount();
	  Log("[**] Mark channel " + IntToStr(cc->Tag) + " as disconnected, waiting batch reconnection.");
	  cc->Tag = -1;
	} // simple end
}

THSockContext* TeWiFiClient::FindContext(bool Auth)
{
	int j = GetContextCount();
	for (int i = 0; i < j; i++)
	{
		THSockContext* sc = (THSockContext*)GetContext(i);
		if ((sc != NULL)&&(sc->Connected) && (sc->Logined))
		{
			if(sc->Logined == true)
			{
				if ((sc->Tag > -1) && (sc->Tag < Option.Net.ServerIndex->Count()))
				{
					if (Option.Net.Servers[sc->Tag].stat >= ssConnected)
					{
						if (((Option.Net.Servers[sc->Tag].type == stManage) && Auth) ||
								((Option.Net.Servers[sc->Tag].type == stAudit) && !Auth))
						{
							return sc;
						}
					}
				}
			}else
			{
				printf("Logic Connect is Connected--i=%d!\n",i);
				continue;
			}
		}
	}
	return NULL;
}
#ifdef POWER
void TeWiFiClient:: power()
{
	   PowerOn();
}
#endif

#endif /* EWIFI_CLIENT_CPP_ */
