/*
 * ewifi_client.hpp
 *
 *  Created on: 2013-9-28
 *      Author: zhaoxi
 */

#ifndef EWIFI_CLIENT_HPP_
#define EWIFI_CLIENT_HPP_

#include "../hlib/global.hpp"
#include "../hlib/hdatarec.hpp"
#include "../hlib/hdateutils.hpp"
#include "../hlib/hpasutils.hpp"
#include "../hlib/hhmsi.hpp"
#include "../hlib/hsrswc.hpp"
#include "proj_utils.hpp"
//#include "../drv/driver.hpp"

#include "proj_consts.hpp"
#include "business.hpp"
#include "../hlib/hzlib.hpp"
#include "sys/reboot.h"
#ifndef H_ICONV
#include "../hlib/hcp936.hpp"
#endif
#ifdef H_64BIT
//#include "../hlib/mcrc32c.hpp"
#else

#endif

#include "../hlib/hcrc32c.hpp"

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <memory>
#include <unistd.h>
#include "../3rd/rapidjson/document.h"
#include "../3rd/rapidjson/writer.h"
#include "../3rd/rapidjson/stringbuffer.h"

using namespace std;
using namespace rapidjson;
#define UNIXSTR_PATH	"/var/pcap"
#define VERSION_SOFT	"version 15.10.08"
#define VERSION_HARD	"ewifi_mips_64_openwrt"
// include command here

//#include "../cmd/conn_req.cpp"

// include end;

#include"wdctl.hpp"


typedef struct THStatistics
{
   Int64 Sent;
   Int64 SentC;
   Int64 Recv;
   Int64 RecvC;
   Int64 TimeCost;
   Int64 TimeC;
   Int64 ErrC;
} *PHStatistics;

typedef struct IPCMAG
{
	int type;
	char buf[32];
}IPCMSG_t;

struct mymsg
{
	long 		mtype;
	IPCMSG_t 	data;
};

/***************************************/

typedef unsigned char uchar;
typedef unsigned int uint;
typedef unsigned short uint16;

typedef struct
{
	uint16 		port;
	uint16 		ser;
	in_addr_t 	addr;
	uchar		mac[6];
}addr_usr;



typedef struct
{
	time_t 				actdate;
	addr_usr 			src;
	addr_usr			dst;
	user_action_type 	type;
	uint16				len;
	//char*				url;				// 褰撳彂鐜版槸http鍗忚鏃�,鍚庢倲璺焨rl
}usr_action;

const int MAX_BODY_SIZE = 4096;
/*********************************************/

typedef THDataRec* PHDataRec;

class TeWiFiClient:public THSockServer
{
  public:
   int AlarmCount;
   bool reboot;
   // business
   // statistics
   THStatistics stat;
   PHDataRec DDRs[4];
   int msgfd;

   string am_url;

   THSRSWC* sc;
   bool Reconnect();
   void OnNew(THContext* FClientContext);
   void OnClose(THContext* FClientContext);
   void ClearConnection();
   THSockContext* FindContext(bool Auth);
   // end
   void OnStarted();
   void HandleUnix(const char*cmd,const char* body);
   void Patrol(THEpollServer* server, bool LowSpeedEvent);
   void SendDemoPacks(bool compress);
   void SplitStr(string str);
   void SendMsg(THSockContext* FClientContext, __u8 cmd, void* buf, int len, bool compress);
   // check: if there is a good packet?
   virtual bool OnCheckPacketHead(THContext* FClientContext);
   virtual bool OnCheckPacketBody(THContext* FClientContext);
   // do: an validated packet arrived
   virtual void ProcessRequest(THSockContext* FClientContext);
   virtual void BuildResponse(THSockContext* FClientContext, bool safe = false);
   virtual void Acknoledge(THSockContext* FClientContext);
#ifdef POWER
   void power();
#endif
   TeWiFiClient();
   virtual ~TeWiFiClient();
};

void IPCSendMsg(string data);

#ifndef POWER
static TeWiFiClient* ec = NULL;
#endif

//{* utils *}

#endif /* WEBCONSVR_HPP_ */
