#include"client.hpp"
#include<string>
#include<iostream>
using namespace std;
Client::Client() :
		THSockServer() {
	head_len = sizeof(struct msg_head1);
	body_max = H_BUFFER_LEN;
	content_size = H_BUFFER_LEN << 1;
	FDate = 0;
	UsrDate = 0;
	ListDate = 0;
	DevDate = 0;
}
Client::~Client() {
	for (int i = 0; i < 4; i++)
		delete DDRs[i];
}
bool Client::Reconnect() {
}
void Client::OnNew(THContext* FClientContext) {
	Log(
			"(++) New connection [ " + IntToStr(FClientContext->Socket)
					+ " ] to [ " + FClientContext->PeerIP + " : "
					+ IntToStr(FClientContext->PeerPort) + " ]..", H_CAUTION);
	THSockContext* cc = (THSockContext*) FClientContext;
	Client* srv = (Client*) (cc->Server);
	cc->local_seq = 0;
	cc->peer_seq = 0;
	cc->Response->AutoAcknowledge = false;
	cc->Response->AbortGZIP = false;
	cc->Connected = true;
	cc->Logined = true;
	cc->Response->ResponseCode = msg_cmd_user_action2;
	cc->Response->ResponseStream.Clear();
	msg_head1 head;
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

	cc->Response->ResponseStream.WriteBuffer(&head, sizeof(head));
	cc->Response->ResponseStream.WriteBuffer(ptr, strlen(body));

	srv->BuildResponse(cc, true);
}
void Client::OnClose(THContext* FClientContext) {
	Log(
			"(--) Disconnected [ " + IntToStr(FClientContext->Socket)
					+ " ] to [ " + FClientContext->PeerIP + " : "
					+ IntToStr(FClientContext->PeerPort) + " ]..", H_CAUTION);
	THSockContext* cc = (THSockContext*) FClientContext;
	cc->SessionStatus = ssWaitHead;
	cc->Response->ResponseCode = 200;
	cc->Response->AbortGZIP = false;
	cc->Response->ResponseStream.Clear();
	cc->Connected = false;
	cc->Logined = false;
	if ((cc->Tag > -1) && (cc->Tag < Option.Net.ServerIndex->Count())) { // need reconnect
		Option.Net.Servers[cc->Tag].stat = ssDisconnected;
		Option.Net.Servers[cc->Tag].date = GetTickCount();
		Log(
				"[**] Mark channel " + IntToStr(cc->Tag)
						+ " as disconnected, waiting batch reconnection.");
		cc->Tag = -1;
	} // simple end
}
void Client::ClearConnection() {
	int cc = GetContextCount();
	double n = Now();
	for (int i = 0; i < cc; i++) {
		THSockContext* sc = (THSockContext*) GetContext(i);
		if (!sc || !sc->Connected)
			continue;
		if (!sc->Logined && (sc->ConnectDate == sc->LastDate)
				&& ((n - sc->ConnectDate) > (H_SEC * 30)))
				{
			Log(
					"(xx) Clean [ " + IntToStr(sc->Socket) + " ] of [ "
							+ sc->PeerIP + " : " + IntToStr(sc->PeerPort)
							+ " ]: Too long to wait 1st packet.", H_CAUTION);
			CloseContext(sc);
			continue;
		}
		if (!sc->Logined && ((n - sc->ConnectDate) > (H_SEC * 120))) // 120绉掍笉鐧诲綍
				{
			Log(
					"(xx) Clean [ " + IntToStr(sc->Socket) + " ] of [ "
							+ sc->PeerIP + " : " + IntToStr(sc->PeerPort)
							+ " ]: Too long to login.", H_CAUTION);
			CloseContext(sc);
			continue;
		}
		if ((n - sc->LastDate) > (H_SEC * 300))
				{
			Log(
					"(xx) Clean [ " + IntToStr(sc->Socket) + " ] of [ "
							+ sc->PeerIP + " : " + IntToStr(sc->PeerPort)
							+ " ]: Idle was too long.", H_CAUTION);
			CloseContext(sc);
			continue;
		}
	}
}
THSockContext* Client::FindContext(bool Auth) {
	int j = GetContextCount();
	for (int i = 0; i < j; i++) {

		THSockContext* sc = (THSockContext*) GetContext(i);
		if ((sc != NULL) && (sc->Connected) && (sc->Logined)) {
			cout << "connected:" << sc->Connected << " Logined:" << sc->Logined
					<< endl;
			if (sc->Logined == true) {
				cout << "login is true\n";
				if ((sc->Tag > -1)
						&& (sc->Tag < Option.Net.ServerIndex->Count())) {
					if (Option.Net.Servers[sc->Tag].stat >= ssConnected) {
						if (((Option.Net.Servers[sc->Tag].type == stManage)
								&& Auth)
								|| ((Option.Net.Servers[sc->Tag].type == stAudit)
										&& !Auth)) {
							return sc;
						}
					}
				}
			} else {
				Log("Logic Connect is Connected--i=%d!\n", i);
				continue;
			}
		}
	}
	return NULL;
}

void Client::OnStarted() {
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
	memset(&stat, 0, sizeof(THStatistics));
	DDRs[0] = new THDataRec("server_send", dtBytes, dmInt64, cmTotal, psSecond,
			false);
	DDRs[1] = new THDataRec("server_recv", dtBytes, dmInt64, cmTotal, psSecond,
			false);
	DDRs[2] = new THDataRec("server_time", dtMicroSeconds, dmInt64, cmAverage,
			psSecond, false);
	DDRs[3] = new THDataRec("server_err", dtBytes, dmInt64, cmTotal, psSecond,
			false);

}
void Client::HandleUnix(const char*cmd, const char* body) {
}
extern Client *client;
static void Heart() {
	cout << "client Patrol\n";
	THSockContext* sc = client->FindContext(true);
	if ((sc == NULL) || !(sc->Logined)) {
		logout("client is not login to server\n");
		return;
	}
	string ip = "10.10.10.10";
	string mac = "AA:ss:d2:22:34:24";
	__u32 date = 1000;
	string url = "www.taobao.com";
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
	sc->Response->ResponseStream.WriteBuffer((void*) url.c_str(), url.length());

	sc->Response->AbortGZIP = false;
	sc->Response->AutoAcknowledge = false;
	client->BuildResponse(sc, true);
}
static void heart() {
	THSockContext* sc = client->FindContext(true);
	if ((sc == NULL) || !(sc->Logined)) {
		logout("client is not login to server\n");
		return;
	}
	msg_head1 head;
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
	PHResponse resp = sc->Response;
	sc->Response->ResponseStream.WriteBuffer(&head, sizeof(head));
	sc->Response->ResponseStream.WriteBuffer(ptr, strlen(body));
	client->BuildResponse(sc, true);
}
void Client::Patrol(THEpollServer* server, bool LowSpeedEvent) {
	//定时的心跳等任务
	double a = Now();
	if ((a - ListDate) > (H_SEC * 5)) {
		//Heart();
		heart();

	}

}
void Client::SendDemoPacks(bool compress) {
	int cc = GetContextCount();
	//printf("SendDemoPacks: %d\n", cc);
	for (int i = 0; i < cc; i++) {
		THSockContext* sc = (THSockContext*) GetContext(i);
		if (!sc)
			continue;
		if (!sc->Logined)
			continue;
		int noir = random() % 3;
		int dice = random() % 3; // 1/2
		if (dice == 0) // send command msg_cmd_ap_dns_white
				{
			string white = "";
			switch (noir) {
			case 0: {
				white = ".sina.com.cn";
				break;
			}
			case 1: {
				white = ".qq.com";
				break;
			}
			case 2: {
				white = ".alipay.com";
				break;
			}
			}
			SendMsg(sc, msg_cmd_ap_dns_white2, (void*) white.c_str(),
					white.length(), compress);
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
			switch (noir) {
			case 0: {
				upg = "ftp://just.a.sample/useless.zip";
				break;
			}
			case 1: {
				upg = "https://task.it.easy/never_exists.bin";
				break;
			}
			case 2: {
				upg = "http://this.is.only/a#demo";
				break;
			}
			}
			SendMsg(sc, msg_cmd_ap_upgrade2, (void*) upg.c_str(), upg.length(),
					compress);
		}
	}
}
void Client::SendMsg(THSockContext* FClientContext, __u8 cmd, void* buf,
		int len,
		bool compress) {
	// 1. local var prepare
	Byte tmp_buf[H_BUF_LEN];
	msg_head1 head;
	// 2. output
	head.sequence = htonl(FClientContext->local_seq++);
	// process body
	head.type = htonl(cmd);
	pByte data;
	__u16 data_len;
	Log("[**] SendMsg size: [ " + IntToStr(len) + " ].", H_CAUTION);
	if (compress && (len > 0)) {
		int buf_len = content_size;
		if (GZCompress((const char*) buf, len, (char*) tmp_buf, &buf_len)) {
//			head.flag |= mhf_GZIP;
//			data_len = buf_len;
//			data = tmp_buf;
			Log("[**] GZip to [ " + IntToStr(buf_len) + " ] bytes.", H_COMMON);
		} else {
			data_len = len;
			data = (pByte) buf;
		}
	} else {
		data_len = len;
		data = (pByte) buf;
	}
	head.len = htonl(data_len);
//	head.crc32c = htonl(crc32c((__u32 ) 0, (char*) data, data_len));

	// 3. write to buffer
	FClientContext->SendData((pByte) &head, sizeof(msg_head1));
	FClientContext->SendData(data, data_len);
	//FClientContext->SessionStatus = ssWaitOutput;
	FClientContext->SessionStatus = ssWaitHead;
	Log(
			"(<<) cmd [ " + GetCommandDisplayName(cmd) + " ] of socket [ "
					+ IntToStr(FClientContext->Socket) + " ]: wrote out "
					+ IntToStr(data_len + sizeof(msg_head2)) + " byte(s).",
			H_COMMON);
}
// check: if there is a good packet?
bool Client::OnCheckPacketHead(THContext* FClientContext) {
	PHRequest req = GetRequest(FClientContext);
	pmsg_head1 head = (pmsg_head1) req->Head;
	Log("OnCheckPacketHead:head len:%d", ntohl(head->len));
	__u32 hl = ntohl(head->len);
	if (hl <= H_PROTO_MAX_LEN) {
		req->FormLength = hl;
		req->PacketLength = hl + head_len;
		return true;
	}
	return false;
}
bool Client::OnCheckPacketBody(THContext* FClientContext) {
	Log("OnCheckPacketBody");
	PHRequest req = GetRequest(FClientContext);
	pmsg_head1 head = (pmsg_head1) req->Head;
	if (head->len == 0)
		return true;
	req->ContentLength = req->FormLength;
	req->Data = req->Body;
#ifdef CRC
	__u32 crc = 0;
	__u32 hcrc = ntohl(head->crc32c);
	crc = crc32c(crc, (char*) req->Body, req->FormLength);
	if (crc != hcrc) {
#ifdef H_DEBUG
		printf("<!!> bad crc: %d, should be %d\n", hcrc, crc);
#endif
		return false;
	}
	if (head->flag & mhf_GZIP) {
		int buf_len = content_size;
		int ret = GZDecompress((const char*) req->Body, req->FormLength,
				(char*) req->Content, &buf_len);
		if (!ret) {
#ifdef H_DEBUG
			printf("<!!> UNGZIP error\n");
#endif
			req->ContentLength = req->FormLength;
			req->Data = req->Body;
		} else {
#ifdef H_DEBUG
			printf("<!!> UNGZIP OK, len: %d\n", buf_len);
#endif
			req->ContentLength = buf_len;
			req->Data = req->Content;
		}
	} else {
#ifdef H_DEBUG
		printf("<!!> NO ZIP\n");
#endif
		req->ContentLength = req->FormLength;
		req->Data = req->Body;
	}
#endif

	return true;
}
// do: an validated packet arrived
void Client::ProcessRequest(THSockContext* FClientContext) {
	Log("ProcessRequest");

	logout("coming into ProcessReques\n");
	PHRequest req = GetRequest(FClientContext);
	pmsg_head1 head = (pmsg_head1) req->Head;
	__u32 hc = ntohl(head->type);
#ifndef H_PRINT_ACK
	if (hc != msg_cmd_ack2)
#endif
		Log(
				"(>>) cmd [ " + GetCommandDisplayName(hc) + " : "
						+ IntToStr(FClientContext->Socket) + " # "
						+ IntToStr(ntohl(head->sequence)) + " @ "
						+ FClientContext->PeerIP + " : "
						+ IntToStr(FClientContext->PeerPort) + " ] ",
				H_CAUTION);
	logout("begin into DispathCommand\n");
	do {
		// dynamic page, put it first for performance
		if (DispatchCommand(FClientContext, hc))
			break;

		// cannot be recognized
		Log("[!!] Invalid incoming command, close connection.", H_EMERGENCY);
		CloseContext(FClientContext);
		return;
	} while (false);
	BuildResponse(FClientContext);
}
void Client::BuildResponse(THSockContext* FClientContext, bool safe) {
	Log("BuildResponse");
	// 1. local var prepare
	logout("coming into BuildResponse\n");
	PHRequest req = FClientContext->Request;
	PHResponse resp = FClientContext->Response;
	pmsg_head1 head;
	msg_head1 local_head;

	if (safe) {
		memset(&local_head, 0, sizeof(msg_head2));
		head = &local_head;
//		head->logo = htons(H_PROTO_LOGO);
	} else
		head = (pmsg_head1) req->Head;
	// 2. output
	//确认是否需要应答
	if (resp->AutoAcknowledge)
		Acknoledge(FClientContext);
	if (resp->ResponseCode == msg_cmd_unknown)
		return;
//	head->sequence = htonl(FClientContext->local_seq++);

	// process body
	head->type = htons(resp->ResponseCode);
	pByte data;
	__u16 data_len;
	Log("[**] Response size: [ " + IntToStr(resp->ResponseStream.Size) + " ].",
			H_COMMON);
	//报文压缩
//	if ((!resp->AbortGZIP) && (resp->ResponseStream.Size > 1000))
//			{
//		int buf_len = content_size;
//		if (GZCompress((const char*) resp->ResponseStream.Memory,
//				resp->ResponseStream.Size, (char*) req->Content, &buf_len)) {
//			head->flag |= mhf_GZIP;
//			data_len = buf_len;
//			data = req->Content;
//			Log("[**] GZip to [ " + IntToStr(buf_len) + " ] bytes.", H_COMMON);
//		} else {
//			data_len = (__u16 ) resp->ResponseStream.Size;
//			data = resp->ResponseStream.Memory;
//		}
//	} else {
		data_len = (__u16 ) resp->ResponseStream.Size;
		data = resp->ResponseStream.Memory;
//	}
	head->len = htonl(data_len);
	//head->crc32c = htonl(crc32c((__u32 ) 0, (char*) data, data_len));

	// 3. write to buffer
	FClientContext->SendData((pByte) head, sizeof(msg_head1));
	FClientContext->SendData(data, data_len);
	resp->ResponseStream.Clear();
	//FClientContext->SessionStatus = ssWaitOutput;
	FClientContext->SessionStatus = ssWaitHead;
	Log("Data len :" + IntToStr(data_len));
	Log(
			"(<<) cmd [ " + GetCommandDisplayName(resp->ResponseCode)
					+ " ] of socket [ " + IntToStr(FClientContext->Socket)
					+ " ]: wrote out " + IntToStr(data_len + sizeof(msg_head1))
					+ " byte(s).", H_COMMON);
}
//是否需要应答确认包
void Client::Acknoledge(THSockContext* FClientContext) {
	Log("Acknoledge");
	// 1. local var prepare
	PHRequest req = FClientContext->Request;
	pmsg_head1 head = (pmsg_head1) req->Head;
	// 2. output
	// logo, no need reset, 鍊熺敤涓�涓�
	head->type = 1;
	head->sequence = htons(msg_cmd_ack2);
	head->len = 0;
	FClientContext->SendData((pByte) head, sizeof(msg_head2));
#ifdef H_PRINT_ACK
	Log("(<<) ACK [ " + IntToStr(ntohl(head->seq)) + " ]: sent. ", H_COMMON);
#endif
}
bool Client::Power_On() {
	return PowerOn();
}
