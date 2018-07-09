/*
 * user_stat.cpp
 *
 *  Created on: 2015-1-19
 *      Author: masanari
 */


#ifndef USER_STAT_CPP_
#define USER_STAT_CPP_

#include "../hlib/global.hpp"
#include "../hlib/scksvr.hpp"
#include "../proj/business.hpp"
#include "../proj/ap_type2s.hpp"
#include "../proj/ewifi_client.hpp"
#include "../proj/proj_utils.hpp"
#include "../drv/driver.hpp"
#include "../hlib/logout.hpp"
#include"../drv/arp_.hpp"
#include"../proj/wdctl.hpp"
extern con_collect con;
static void update_con(string mac,unsigned char sta)
{
	//cout<<"收到itts数据更新数据\n";
	logout("收到itts数据更细\n");
	con.up_stat("br-lan",mac,sta);
}
static void update_con1(unsigned char *mac,unsigned char sta)
{
	//cout<<"收到itts数据更新数据\n";
	con.up_stat1("br-lan",mac,sta);
}
namespace cmd_user_stat // modify here for each cmd!!
{
	void CommandHandler(THSockContext* Context)
	{
		logout("coming into cmd_user_stat\n");

		TeWiFiClient* srv = (TeWiFiClient*)(Context->Server);
		// 1. validate command at first
		PHRequest req = Context->Request;
		PHResponse resp = Context->Response;
#if 1
		if (req->ContentLength < (int)sizeof(user_stat2))
		{
			srv->Log("[xx] Bad length: " + IntToStr(req->ContentLength) + ", disconnect.");
			// length is bad
			Context->Close();
			return;
		}
		// 2. read data
		puser_stat2 body = (puser_stat2)(req->Data);
		// 2.1 check time
		// 2.2 check soft_id in support list
		// 2.3 check hard_id in support list
		// 2.4 record hard_seq ???
		if (!Context->Logined)
		{
			srv->Log("[xx] user_stat before conn_req, cut.");
			Context->Close();
			return;
		}

		// 3. send notice
		try
		{
			//cout<<"\n\n\n-------------收到服务端信息-----------\n";
		string mac = read_mac(body->mac);
		__u8 sta = body->stat;
		logout("body->stat=%d\n", body->stat);
		__u8 MaC[6];
		memcpy(MaC, body->mac, sizeof(body->mac));
		int sock = connect_to_server(DEFAULT_SOCK);
		//logout(" 认证通过数据出去\n");
		if (sock == -1)
		{
			for (int i = 0; i < 3; ++i) {
				logout("失败重新连接\n");
				if (sock = connect_to_server(DEFAULT_SOCK) != -1)
					break;
			}
		} else
			logout("sock 成功\n");
		char request[1024];
		if (sta == 2)
		{
			sprintf(request, "allow %s\r\n\r\n", mac.c_str());
			int m = send_request(sock, request);
			logout("send 2 认证通过数据出去\n");

		} else if (sta == 3 || sta == 1)
		{
			sprintf(request, "del %s\r\n\r\n", mac.c_str());
			//string MAC = read_mac((*it)->arp_mac);
			//cout << "发送的MAc地址:" << MAC << endl;
			//printf("request 是%s\n", request);
			send_request(sock, request);
			logout("send 3认证通过数据出去\n");
			//cout << "send 3  离线数据出去\n";
		}
		close(sock);
		//update_con1(MaC,sta);
		update_con(mac, sta);

		}
		catch(exception &e)
		{
			string errmsg = e.what();
			srv->Log("[xx] on_user_stat error: " + errmsg);
		}
#endif
		// 4. cancel write out
		resp->AutoAcknowledge = true;
		resp->AbortGZIP = false;
		resp->ResponseCode = msg_cmd_unknown;
	}

	void Initialization()
	{
		RegisterCommandHandler(msg_cmd_user_stat2, &CommandHandler);
	}

	void Finalization()
	{

	}

	class TUnitController
	{
		public:
		TUnitController();
		~TUnitController();
	};

	TUnitController::TUnitController()
	{
		// Initialization
		Initialization();
	}

	TUnitController::~TUnitController()
	{
		// Finalization();
		Finalization();
	}

	TUnitController ThisUnit;
}

#endif /* USER_STAT_CPP_ */


