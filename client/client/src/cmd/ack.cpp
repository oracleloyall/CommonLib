/*
 * ack.cpp
 *
 *  Created on: 2015-1-19
 *      Author: masanari
 */


#ifndef ACK_CPP_
#define ACK_CPP_

#include "../hlib/global.hpp"
#include "../hlib/scksvr.hpp"
#include "../proj/business.hpp"
#include "../proj/ap_type2s.hpp"
#include "../proj/ewifi_client.hpp"

namespace cmd_ack // modify here for each cmd!!
{
	void CommandHandler(THSockContext* Context)
	{
		// 1. validate command at first
		PHRequest req = Context->Request;
		if (req->FormLength != 0)
		{
			// length is bad
			Context->Close();
			return;
		}
		// 2. read data
		TeWiFiClient* srv = (TeWiFiClient*)(Context->Server);
		srv->Log("[*>] ACK " + IntToStr(ntohl(((pmsg_head2)req->Head)->seq)) + " [ " + IntToStr(Context->Socket) + " ] received.");
		// 2.1 check time
		// 2.2 check soft_id in support list
		// 2.3 check hard_id in support list
		// 2.4 record hard_seq ???

		// 3. write out
		PHResponse resp = Context->Response;
		resp->AutoAcknowledge = false;
		resp->AbortGZIP = true;
		resp->ResponseCode = msg_cmd_unknown;
	}

	void Initialization()
	{
		RegisterCommandHandler(msg_cmd_ack2, &CommandHandler);
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

#endif /* ACK_CPP_ */


