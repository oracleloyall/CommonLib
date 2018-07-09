/*
 * upgrade.cpp
 *
 *  Created on: 2015-4-1
 *      Author: root
 */

#ifndef UPGRADE_CPP_
#define UPGRADE_CPP_

#include "../hlib/global.hpp"
#include "../hlib/scksvr.hpp"
#include "../proj/business.hpp"
#include "../proj/ap_type2s.hpp"
#include "../proj/ewifi_client.hpp"
#include "../proj/proj_utils.hpp"
#include "../drv/driver.hpp"

namespace cmd_upgrade // modify here for each cmd!!
{
	void CommandHandler(THSockContext* Context)
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
			srv->Log("[xx] upgrade before conn_req, cut.");
			Context->Close();
			return;
		}

		// 3. send notice
		try
		{
			srv->Log("[^^] Upgrade: " + body);
			on_upgrade(body);
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

	void Initialization()
	{
		RegisterCommandHandler(msg_cmd_ap_upgrade2, &CommandHandler);
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

#endif /* UPGRADE_CPP_ */
