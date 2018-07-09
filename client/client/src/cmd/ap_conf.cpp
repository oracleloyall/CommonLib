/*
 * ap_conf.cpp
 *
 *  Created on: 2015-4-1
 *      Author: root
 */

#ifndef AP_CONF_CPP_
#define AP_CONF_CPP_

#include "../hlib/global.hpp"
#include "../hlib/scksvr.hpp"
#include "../hlib/logout.hpp"
#include "../proj/business.hpp"
#include "../proj/ap_type2s.hpp"
#include "../proj/ewifi_client.hpp"
#include "../proj/proj_utils.hpp"
#include "../drv/driver.hpp"

namespace cmd_ap_conf // modify here for each cmd!!
{
	void CommandHandler(THSockContext* Context)
	{

		TeWiFiClient* srv = (TeWiFiClient*)(Context->Server);
		// 1. validate command at first
		PHRequest req = Context->Request;
		PHResponse resp = Context->Response;
		// 2. read data
		pap_net_conf2 body = (pap_net_conf2)(req->Data);

		// 2.4 record hard_seq ???
		if (!Context->Logined)
		{
			srv->Log("[xx] ap_conf before conn_req, cut.");
			Context->Close();
			return;
		}

		// 3. send notice
		try
		{
			srv->Log("[^^] Modify config: ");
			on_ap_runtime_conf(body);
		}
		catch(exception &e)
		{
			string errmsg = e.what();
			srv->Log("[xx] Event ap_conf handler error: " + errmsg);
		}

		// 4. cancel write out
		resp->AutoAcknowledge = true;
		resp->AbortGZIP = false;
		resp->ResponseCode = msg_cmd_unknown;

	}

	void DummyHandler(THSockContext* Context)
	{
		TeWiFiClient* srv = (TeWiFiClient*)(Context->Server);

		PHRequest req = Context->Request;
		PHResponse resp = Context->Response;

		if(!Context->Logined)
		{
			srv->Log("[xx] ap_conf before conn_req, cut.");
			Context->Close();
			return;
		}

		pap_dev_conf2 body = (pap_dev_conf2)req->Data;
		on_ap_dev_conf(body);

		resp->AutoAcknowledge = true;
		resp->AbortGZIP = false;
		resp->ResponseCode = msg_cmd_unknown;
	}

	void Initialization()
	{
		RegisterCommandHandler(msg_cmd_ap_net_conf2, &CommandHandler);
		RegisterCommandHandler(msg_cmd_ap_dev_conf2, &DummyHandler);
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

#endif /* AP_CONF_CPP_ */
