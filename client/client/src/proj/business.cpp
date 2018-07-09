/*
 * business.cpp
 *
 *  Created on: 2013-9-27
 *      Author: root
 */

#ifndef BUSINESS_CPP_
#define BUSINESS_CPP_

#include "business.hpp"
#include "../hlib/logout.hpp"

//const int MAX_GPR = 256;
const int MAX_GPR = 64;
const int MAX_GUID = 32768;

hhmii::THHMII* GPI = NULL;
THCmdResponder* GPR = NULL;
int GPC = 0;

//THHMSI* GUIDs = NULL;

string GetCommandDisplayName(msg_cmd_type mct)
{
	switch(mct)
	{
		case msg_cmd_unknown:
		{
			return "msg_cmd_unknown";
			break;
		}
		case msg_cmd_conn_req2:
		{
			return "msg_cmd_conn_req";
			break;
		}
		case msg_cmd_conn_resp2:
		{
			return "msg_cmd_conn_resp";
			break;
		}
		case msg_cmd_ap_stat2:
		{
			return "msg_cmd_ap_stat";
			break;
		}
		case msg_cmd_ap_dns_white2:
		{
			return "msg_cmd_ap_dns_white";
			break;
		}
		case msg_cmd_ap_net_conf2:
		{
			return "msg_cmd_ap_net_conf2";
			break;
		}
		case msg_cmd_ap_upgrade2:
		{
			return "msg_cmd_ap_upgrade";
			break;
		}
		case msg_cmd_ap_dev_conf2:
		{
			return "msg_cmd_ap_dev_conf";
			break;
		}
		case msg_cmd_user_stat2:
		{
			return "msg_cmd_user_stat";
			break;
		}
		case msg_cmd_user_action2:
		{
			return "msg_cmd_user_action";
			break;
		}
		case msg_cmd_ack2:
		{
			return "msg_cmd_ack";
			break;
		}
		case msg_cmd_conn_heartbeat2:
		{
			return "msg_cmd_conn_heartbeat";
			break;
		}
		case msg_cmd_black_list2:
		{
			return "msg_cmd_conn_heartbeat";
			break;
		}
		case msg_cmd_reboot2:
		{
			return "msg_cmd_conn_heartbeat";
			break;
		}
		case msg_cmd_max:
		{
			return "msg_cmd_max";
			break;
		}
		default:
		{
			return "unknown command";
		}
	}
}

namespace business
{
	bool inited = false;

	class TUnitController
	{
		public:
		TUnitController();
		~TUnitController();
	};

	void Initialization()
	{
		if (!inited)
		{
			GPI = new hhmii::THHMII(MAX_GPR);
			GPR = new THCmdResponder[MAX_GPR];
//			GUIDs = new THHMSI(MAX_GUID);
			inited = true;
		}
	}

	void Finalization()
	{
		if (inited)
		{
			if (GPI) delete GPI;
			if (GPR) delete [] GPR;
//			if (GUIDs) delete GUIDs;
			inited = false;
		}
	}

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

//{* framework *}

bool InitDB()
{
	return true;
}

void FreeDB()
{

}
#if 0
int GUIDCount()
{
	return GUIDs->Count();
}

bool AddGUID(const string GID, int i)
{
	return GUIDs->AddS(GID, i) == harOK;
}

void DelGUID(const string GID)
{
	GUIDs->RemoveS(GID);
}

string GetGUID()
{
	string s = RandomString(6);
	while (GUIDs->ExistsS(s)) s = RandomString(6);
	return s;
}

int SeekGUID(const string GID)
{
	PHElement x = GUIDs->FindS(GID);
	return x ? x->Value : -1;
}

bool SetGUID(const string GID, int i)
{
	PHElement x = GUIDs->FindS(GID);
	if (x)
	{
		x->Value = i;
		return true;
	}
	return AddGUID(GID, i);
}
#endif
bool RegisterCommandHandler(const int cmd, THCmdResponder Responder)
{
	business::Initialization();
	hhmii::PHElement p = GPI->Find(cmd);
	if (!p)
	    if (GPI->IsFull())
	    	return false;
	    else
	    {
	    	GPR[GPC] = Responder;
	    	GPI->Add(cmd, GPC++);
	    	return true;
	    }
	else
	{
	    GPR[p->Value] = Responder;
	    return true;
	}
}

bool DispatchCommand(THSockContext* Context, const int cmd)
{
	logout("coming into DispathCommand\n");
	business::Initialization();
	hhmii::PHElement p = GPI->Find(cmd);
	if (!p)
	{
		Context->Response->ResponseCode = msg_cmd_unknown;
		return false;
	}
	try
	{
	    Context->ScriptTick = GetMicroSecondsDraft();
	    (GPR[p->Value])(Context);
	    return true;
	}
	catch(exception& e)
	{
		string s = e.what();
		((THSockServer*)(Context->Server))->Log("<!!> Error occured when DispatchCmd: " + s);
	    Context->Response->ResponseCode = msg_cmd_unknown;
	    return false;
	}
}

//{* business *}

string GenerateProductID(const string Mac)
{
    // 1. normalize
	string mac = Mac;
	StringReplace(mac, ":", "");
	StringReplace(mac, ",", "");
	StringReplace(mac, " ", "");
	string t = "";
	int j = Length(mac) >> 1;
	t.resize(j, ' ');
	// 2. text
	for (int i = 0; i < j; i++)
	{
		t[i] = (fromHex(mac[i << 1]) << 4) + fromHex(mac[(i << 1) + 1]);
	}
	string s = H_BOX_NAME + t;
	// 3. cipher
	Seal(s);
	s = SealToString(s);
	// 4. format
	t = "";
	j = Length(s);
	while (j > 8)
	{
	    t = t + LeftStr(s, 5) + "-";
	    s = RightStr(s, Length(s) - 5);
	    j -= 5;
	}
	return t + s;
}
#endif /* BUSINESS_CPP_ */
