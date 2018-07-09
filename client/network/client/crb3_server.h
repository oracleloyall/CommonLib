#ifndef _IN_crb3_serverh_
#define _IN_crb3_serverh_

#include <pthread.h>
#include <sys/epoll.h>
#include "hashLib.h"
#include "flstLib.h"
#include "schedLib.h"
#include "logLib.h"
#include "rateLib.h"

#define CRB3_QFIFO
#ifdef CRB3_QFIFO
#include "qfifoLib.h"
#endif
#include "crb3_define.h"

#ifdef NAMESPACE
namespace common{
#endif

#ifndef CRB3_SERVER_RX_BURST
#define CRB3_SERVER_RX_BURST		100
#endif

#ifndef CRB3_MAX_SERVER_NAME_SIZE
#define CRB3_MAX_SERVER_NAME_SIZE	64
#endif


class CRB3Server;

#ifdef CRB3_QFIFO
typedef struct
{
	CRB3Server * ServerRef;
	pthread_t Thid;
	volatile bool IsExit;
	QFIFO PacketQueue;		// <CRB3_PACKET_NODE *>
}CRB3_THREAD_CTRL;
#else
typedef struct
{
	CRB3Server * ServerRef;
	pthread_t Thid;
	pthread_mutex_t QueueMutex;
	pthread_cond_t QueueCond;
	bool IsExit;
	LIST PacketQueue;		// <CRB3_PACKET_NODE>
}CRB3_THREAD_CTRL;
#endif

typedef struct
{
	NODE node;
	CRB3_ADDRESS addr;
	time_t last_notify;
}CRB3_ALIVE_NODE;

class CRB3Server : public SoStream
{
	char ServerName[CRB3_MAX_SERVER_NAME_SIZE];
	LIST AliveList;			// <CRB3_ALIVE_NODE>
	pthread_mutex_t AliveLock;

	// log parameter
	LogManager * LogMng;
	
	// receive thread parameter
	int EPoll;
	int DgramSocket;		// UDP:CfgServicePort
	int StreamSocket;		// TCP:CfgServicePort
	pthread_t ReceiveThid;
	uint64_t RxSuccess;
	uint64_t RxErrorLen;	// counter of discrad for invalid packet length
	uint64_t RxErrorBuf;	// counter of discrad for empty packet buffer

	// process thread parameter
	CRB3_THREAD_CTRL * ThreadTable;
	SCHED Sched;

	// packet cache parameter
	FLIST_ID PacketCache;
	pthread_mutex_t CacheLock;

	// configure parameter
	int CfgThreadNumber;
	int CfgServicePort;
	int CfgMaxClient;
	bool CfgSerializeEn;
	bool CfgAsyncEn;
	char CfgBindAddress[64];

public:
	RateInfo * ReqQps;
	
public:
	CRB3Server(IN char * Name = NULL);
	~CRB3Server();

	// configure interface
	int ConfigFromFile(IN char * CfgFile);
	int ConfigLogPath(IN char * File);
	int ConfigRatePath(IN char * File);
	int ConfigSerialize(IN bool IsEnable);
	int ConfigAsync(IN bool IsEnable);
	int ConfigThreadNumber(IN int Num);
	int ConfigAddress(IN char * BindAddress, IN int ListenPort, IN int MaxClient);

	// control interface
	virtual int Start(void);
	virtual int Stop(void);

	// query interface
	int ShowQueue(OUT char * OutputBuf, IN int BufLen);
	int ShowClient(OUT char * OutputBuf, IN int BufLen);
	int ShowClientStatistic(IN int Fd, OUT char * OutputBuf, IN int BufLen);

	// internal interface
	virtual int GetStatus(void) = 0;	// CRB3_STATUS_WORKING, CRB3_STATUS_BACKUP, CRB3_STATUS_EXCEPTION
	
	virtual int DoService(
				IN CRB3_ADDRESS * ClientAddr,		// ip address of request client
				IN uint32_t CRId,					// id of request
				IN CRB3Param * ParamIn,				// parameters of request
				OUT CRB3Param ** pParamOut) = 0;	// parameters of response
	
	virtual int DoServiceAsync(
				IN CRB3_ADDRESS * ClientAddr,		// ip address of request client
				IN uint32_t CRId,					// id of request
				IN CRB3Param * ParamIn,				// parameters of request (it must not be referenced when function return)
				IN uint64_t RspArg) = 0;			// argument when response

	// async response
	int DoServiceResponse(IN CRB3_ADDRESS * ClientAddr, IN uint32_t CRId, IN CRB3Param * ParamOut, IN uint64_t RspArg);
	
private:
	void _StartThread(IN int ThreadIndex);
	void _StopThread(IN int ThreadIndex);
	void _PacketProcess(IN CRB3_PACKET_NODE * Packet);
	int _PacketPush(IN CRB3_PACKET_NODE * Packet, IN int FromSocket);
	CRB3_PACKET_NODE * _PacketPop(IN CRB3_THREAD_CTRL * ThreadCtrl);
	CRB3_PACKET_NODE * _PacketAlloc(void);
	void _PacketFree(IN CRB3_PACKET_NODE * Packet);
	int _CreateSocket(IN CRB3_ADDRESS * BindAddr, IN int Protocol);
	void _DestroySocket(IN int SocketFd);
	int _SendCRResponse(IN CRB3_ADDRESS * addr, IN int protocol, IN uint32_t crseq, IN uint32_t crid, IN uint16_t crflag, IN CRB3Param * param);
	int _SendCRResponseError(IN CRB3_ADDRESS * addr, IN int protocol, IN uint32_t crseq, IN uint32_t crid, IN uint16_t crflag, IN int status);
	int _SendStatusNotify(IN CRB3_ADDRESS * addr, IN uint32_t status, IN uint32_t weight);
	int _SendAliveNotify(IN CRB3_ADDRESS * addr);
	int _ReceiveConnection(IN int sockfd);
	int _ReceiveDatagram(IN int sockfd);
	int _ReceiveLoop(void);
	int _ProcessLoop(IN CRB3_THREAD_CTRL * ThreadCtrl);
	CRB3_ALIVE_NODE * _UpdateAlive(IN CRB3_ADDRESS * Addr);
	void _KeepAlive(void);

private:
	virtual int PacketSink(IN uint8_t * PacketBuf, IN int PacketSize, IN SO_ADDRESS * FromAddr, IN int FromSocket, IN void * Spare);
	virtual bool CheckXON(IN SO_ADDRESS * FromAddr, IN int FromSocket);
	virtual bool CheckXOFF(IN SO_ADDRESS * FromAddr, IN int FromSocket);

private:
	static void * ReceiveThread(IN void * arg);
	static void * ProcessThread(IN void * arg);
};

#ifdef NAMESPACE
}
#endif
#endif

