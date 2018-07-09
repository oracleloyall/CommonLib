/*
 * conn_req.cpp
 *
 *  Created on: 2015-1-5
 *      Author: root
 */

#ifndef CONN_RESP_CPP_
#define CONN_RESP_CPP_

#include "../hlib/global.hpp"
#include "../hlib/scksvr.hpp"
#include "../proj/business.hpp"
#include "../proj/ap_type2s.hpp"
#include "../proj/ewifi_client.hpp"
#include "../proj/proj_utils.hpp"
#include "../drv/driver.hpp"
#include "../hlib/logout.hpp"

/*
static string white_list = ".alipay.com;.qq.com;.weibo.com;.sina.com.cn;.baidu.com";
static string auth_srv = "122.224.64.245:3000";
static string audit_srv = "122.224.64.245:3001";
static string def_302 = "http://122.224.64.245:6000/api10/login?prot_type=1&";
*/
namespace cmd_conn_req // modify here for each cmd!!
{
	void CommandHandler(THSockContext* Context)
	{
		logout("coming into CommandHandler...\n");
		// 1. validate command at first
		PHRequest req = Context->Request;
		PHResponse resp = Context->Response;
		TeWiFiClient* srv = (TeWiFiClient*)(Context->Server);
		// 2. read data
		pap_conn_resp2 acr = (pap_conn_resp2)(req->Data);
		bool need_save = false;
		bool need_reconnect = false;
		// 2.4 record hard_seq ???
		switch(acr->result)
		{ // case
			case acr_DENIED:
			{
				string s = "";
				s.append((char*)acr->white, ntohs(acr->white_len));
				srv->Log("[!!] Connection denied: " + s);
				need_save = false;
				Context->Close();
				break;
			}
			case acr_REDIRECT:
			{
				srv->Log("[!!] Redirect required.");
				need_reconnect = true;
				need_save = true;
				break;
			}
			case acr_OK:
			{
				Context->Logined = true;
				srv->Log("[OK] Connected. ");
				need_save = false;
				break;
			}
		} // case end
		if(acr->white_len > 0)
		{
			string white = "";
			white.append((char*)acr->white,ntohs(acr->white_len));
			srv->Option.Other.whitelist = "";
			srv->Option.Other.whitelist = white;
			logout("white=%s\n",srv->Option.Other.whitelist.c_str());
		}
		if(acr->black_len > 0)
		{
			string black = "";
			black.append(((char*)acr->white) + ntohs(acr->white_len),ntohs(acr->black_len));
			logout("black:%s\n",black.c_str());
			srv->Option.Other.blacklist = "";
			srv->Option.Other.blacklist = black;
			logout("black=%s\n",srv->Option.Other.blacklist.c_str());
		}

		// whatever, save it in memory;
		srv->Option.Other.should_auth = (acr->auth_mode == aam_PASS);
		srv->Option.Other.should_audit = (acr->audit_mode == aam_NONE);
		srv->Option.Other.ap_interval = ntohs(acr->ap_interval);
		srv->Option.Other.user_interval = ntohs(acr->user_interval);
		srv->Option.Other.audit_interval = ntohs(acr->audit_interval);
		srv->Option.Other.audit_serv = (char*)acr->audit_srv;
		srv->Option.Other.auth_serv = (char*)acr->auth_srv;
		try
		{
			srv->Log("     Call initial config handler..");
			on_ap_initial_conf(&srv->Option.Other);
		}
		catch(exception &e)
		{
			string errmsg = e.what();
			srv->Log("[xx] Event whitelist handler error: " + errmsg);
		}
		try
		{
			if(acr->result != acr_DENIED)
			{
//				srv->Log("     Call whitelist handler..");
//				on_white_list(srv->Option.Other.whitelist);
			}
		}
		catch(exception &e)
		{
			string errmsg = e.what();
			srv->Log("[xx] Event whitelist handler error: " + errmsg);
		}
		if (need_save)
		{
			srv->Log("[**] Update config file.");
			srv->SaveConfig(ChangeFileExt(AppName(), ".ini"));
		}
		if (need_reconnect)
		{
			// 1. disconnect
			int i; int j = srv->GetContextCount();
			for(i = 0; i < j; i++)
			{
				THSockContext* sc = (THSockContext*)srv->GetContext(i);
				sc->Tag = -1;
				sc->Close();
			}
			// 2. clear index
			j = srv->Option.Net.ServerIndex->Count();
			for(i = 0; i < j; i++)
			{
				srv->Option.Net.Servers[i].stat = ssUnknown;
			}
			srv->Option.Net.ServerIndex->Clear();
			// 3. rebuild index
			int cnt = 0;
			TStringList* tmp = new TStringList();
			if (srv->Option.Other.should_auth)
		   { // check
			   Split(srv->Option.Other.auth_serv, ";", *tmp);
			   for(int k = 0; k < tmp->Count(); k++)
			   { //ad
				  string svr = tmp->Lines(k);
				  Trim(svr);
				  if (svr.length() > 0)
				  {
					  j = PosChar(':', (pByte)svr.c_str(), svr.length());
			        if (j > 0)
					  { //a
					    string p = RightStr(svr, svr.length() - j);
						 if (TryStrToInt(p, srv->Option.Net.Servers[cnt].port))
						 {
							 srv->Option.Net.Servers[cnt].ip = LeftStr(svr, j - 1);
							 srv->Option.Net.Servers[cnt].type = stManage;
							 srv->Option.Net.Servers[cnt].stat = ssDisconnected;
							 srv->Option.Net.Servers[cnt].date = GetTickCount() - 6000;
							 srv->Option.Net.ServerIndex->AddS(svr, cnt);
							if (cnt++ >= 16) break;
						 }
					   }
				    }
			     }
			  } // check end
			  if (srv->Option.Other.should_audit)
			  { // check
			    tmp->Clear();
				 Split(srv->Option.Other.audit_serv, ";", *tmp);
				 for(int k = 0; k < tmp->Count(); k++)
				 { // for
					string svr = tmp->Lines(k);
					Trim(svr);
					if ((svr.length() > 0) && (cnt < 16))
					{
					  int j = PosChar(':', (pByte)svr.c_str(), svr.length());
					  if (j > 0)
					  { // if
					    string p = RightStr(svr, svr.length() - j);
						 if (TryStrToInt(p, srv->Option.Net.Servers[cnt].port))
						 { // if
							srv->Option.Net.Servers[cnt].ip = LeftStr(svr, j - 1);
							srv->Option.Net.Servers[cnt].type = stAudit;
							srv->Option.Net.Servers[cnt].stat = ssDisconnected;
							srv->Option.Net.ServerIndex->AddS(svr, cnt);
							if (cnt++ >= 16) break;
						 } // end if
					   } // end if
				    } // end if
			     } // end for
			   } // end
			   delete tmp;
		}  // endif
		// 3. write out

		resp->AbortGZIP = false;
		resp->ResponseCode = msg_cmd_unknown;
		resp->AutoAcknowledge = false;
	}

	void DummyHandler(THSockContext* Context)
	{
		PHResponse resp = Context->Response;
		resp->AutoAcknowledge = true;
		resp->AbortGZIP = true;
		resp->ResponseCode = msg_cmd_unknown;
		resp->ResponseStream.Clear();
	}

	void Initialization()
	{
		RegisterCommandHandler(msg_cmd_conn_resp2, &CommandHandler);
		RegisterCommandHandler(msg_cmd_conn_heartbeat2, &DummyHandler);
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

#endif /* CONN_RESP_CPP_ */


