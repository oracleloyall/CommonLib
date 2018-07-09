/*
 * white_list.cpp
 *
 *  Created on: 2015-4-1
 *      Author: root
 */

#ifndef WHITE_LIST_CPP_
#define WHITE_LIST_CPP_

#include "../hlib/global.hpp"
#include "../hlib/scksvr.hpp"
#include "../proj/business.hpp"
#include "../proj/ap_type2s.hpp"
#include "../proj/ewifi_client.hpp"
#include "../proj/proj_utils.hpp"
#include "../drv/driver.hpp"

namespace cmd_white_list // modify here for each cmd!!
{
	void CommandHandlerWhite(THSockContext* Context)
	{

		TeWiFiClient* srv = (TeWiFiClient*)(Context->Server);
		// 1. validate command at first
		PHRequest req = Context->Request;
		PHResponse resp = Context->Response;
		// 2. read data
		string body = "";
		body.append((char*)req->Data, req->ContentLength);
		// 2.4 record hard_seq ???
		if (!Context->Logined)
		{
			srv->Log("[xx] whitelist before conn_req, cut.");
			Context->Close();
			return;
		}

		// 3. send notice
		try
		{
			srv->Log("[^^] Modify whitelist: " + body);
			if(body.length() > 0)
			{
				srv->Option.Other.whitelist = body;
				srv->SaveConfig(ChangeFileExt(AppName(), ".ini"));
				on_white_list(srv->Option.Other.whitelist);
			}
		}
		catch(exception &e)
		{
			string errmsg = e.what();
			srv->Log("[xx] Event whitelist handler error: " + errmsg);
		}

		// 4. cancel write out
		resp->AutoAcknowledge = true;
		resp->AbortGZIP = false;
		resp->ResponseCode = msg_cmd_unknown;

	}
	void CommandHandlerBlack(THSockContext* Context)
	{
		TeWiFiClient* srv = (TeWiFiClient*)(Context->Server);
		// 1. validate command at first
		PHRequest req = Context->Request;
		PHResponse resp = Context->Response;
		// 2. read data
		string body = "";
		body.append((char*)req->Data, req->ContentLength);

		// 2.4 record hard_seq ???
		if (!Context->Logined)
		{
			srv->Log("[xx] blacklist before conn_req, cut.");
			Context->Close();
			return;
		}

		// 3. send notice
		try
		{
			srv->Log("[^^] Modify blacklist: " + body);
			if(body.length() > 0)
			{
				srv->Option.Other.blacklist = body;
				srv->SaveConfig(ChangeFileExt(AppName(), ".ini"));
				on_black_list(srv->Option.Other.blacklist);
			}
		}
		catch(exception &e)
		{
			string errmsg = e.what();
			srv->Log("[xx] Event blacklist handler error: " + errmsg);
		}

		// 4. cancel write out
		resp->AutoAcknowledge = true;
		resp->AbortGZIP = false;
		resp->ResponseCode = msg_cmd_unknown;
	}

	void Initialization()
	{
		RegisterCommandHandler(msg_cmd_ap_dns_white2, &CommandHandlerWhite);
		RegisterCommandHandler(msg_cmd_black_list2, &CommandHandlerBlack);
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

#endif /* WHITE_LIST_CPP_ */
