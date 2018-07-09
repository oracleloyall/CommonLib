#ifndef CLIENT_HPP_
#define CLIENT_HPP_
#include"head_type.hpp"
#include "../hlib/global.hpp"
#include "../hlib/hdatarec.hpp"
#include "../hlib/hdateutils.hpp"
#include "../hlib/hpasutils.hpp"
#include "../hlib/hhmsi.hpp"
#include "../hlib/hsrswc.hpp"
#include "../proj/proj_utils.hpp"
#include "../proj/proj_consts.hpp"
#include "../proj/business.hpp"
#include "../hlib/hzlib.hpp"
#include <arpa/inet.h>
typedef THDataRec* PHDataRec;
class Client: public THSockServer
{
public:
	Client();
	~Client();
	PHDataRec DDRs[4];
	bool Reconnect();
	void OnNew(THContext* FClientContext);
	void OnClose(THContext* FClientContext);
	void ClearConnection();
	THSockContext* FindContext(bool Auth);
	// end
	void OnStarted();
	void HandleUnix(const char*cmd, const char* body);
	void Patrol(THEpollServer* server, bool LowSpeedEvent);
	void SendDemoPacks(bool compress);
	void SendMsg(THSockContext* FClientContext, __u8 cmd, void* buf, int len,
			bool compress);
	// check: if there is a good packet?
	virtual bool OnCheckPacketHead(THContext* FClientContext);
	virtual bool OnCheckPacketBody(THContext* FClientContext);
	// do: an validated packet arrived
	virtual void ProcessRequest(THSockContext* FClientContext);
	virtual void BuildResponse(THSockContext* FClientContext,
			bool safe = false);
	virtual void Acknoledge(THSockContext* FClientContext);
	bool Power_On();
	void HeartBeat();
	THSRSWC* sc;
	THStatistics stat;
private:
protected:

};

#endif
