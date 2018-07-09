#ifndef _IN_crb3_clienth_
#define _IN_crb3_clienth_

#include <pthread.h>
#include "hashLib.h"
#include "ihashLib.h"
#include "rbtLib.h"
#include "flstLib.h"
#include "logLib.h"
#include "queLib.h"
#include "rateLib.h"
#include "crb3_define.h"
#include "crb3_crl.h"

#ifdef NAMESPACE
namespace common{
#endif

#ifndef CRB3_MAX_CLIENT_RSP_SIZE
#define CRB3_MAX_CLIENT_RSP_SIZE			10000
#endif

#ifndef CRB3_MAX_CLIENT_NAME_SIZE
#define CRB3_MAX_CLIENT_NAME_SIZE			64
#endif

#ifndef CRB3_MAX_CLIENT_DIO
#define CRB3_MAX_CLIENT_DIO					256
#endif

#ifndef CRB3_MAX_HASH_NODES
#define CRB3_MAX_HASH_NODES					1024
#endif

/*************************************************************
*	check server status every 2 seconds
*/
#ifndef CRB3_CHECK_CONNECT_GAP
#define CRB3_CHECK_CONNECT_GAP		2
#endif

/*************************************************************
*	change status to exception when time gap of notify exceed this value
*/
#ifndef CRB3_CHECK_NOTIFY_TIMEOUT
#define CRB3_CHECK_NOTIFY_TIMEOUT	10
#endif

typedef struct
{
	uint32_t CRId;
	uint32_t CRSeq;
	CRB3Param * ParamOut;
}CRB3_RSP;

typedef struct
{
	CRB3_ADDRESS ServerAddr;
	uint32_t Status;
	uint32_t Weight;
}CRB3_STATUS_RSP;

typedef struct
{
	NODE node;
	IRBT_NODE node_nch;
	CRB3_ADDRESS ServerAddr;
	int Status;				// CRB3_STATUS_xxxx
	int Flag;				// bitwise of CRB3_FLAG_xxxx
	int Protocol;			// SOCK_DGRAM or SOCK_STREAM
	int Pending;
	int PendMax;
	int Reference;
	int ToDelete;
	int	Hash;				// consistent hashing value, -1 for not assigned
	LIST * RefList;
	time_t LastNotifyTime;	// timestamp of last notify received
	RateInfo * Rate;
	DelayInfo * Delay;
	double PendWeight;
}CRB3_CONNECTION;

typedef struct
{
	NODE node;
	int Socket;
	int Port;
}CRB3_DIO_NODE;

typedef struct
{
	int InitNodes;			// 0 for auto calculate
	int MaxNodes;			// default is CRB3_MAX_HASH_NODES (1024)
	int NCHTable[CRB3_MAX_HASH_NODES];
}CRB3_NCH_MAP;

typedef enum
{
	ALG_TYPE_FIXED = 0,
	ALG_TYPE_PMASK,
	ALG_TYPE_CUSTOM
}CRB3_NCH_ALG_TYPE;

typedef uint64_t (*NCH_ALG_HASH_FUNC)(CRB3Param * param, uint64_t spare);

typedef struct
{
	IHASH_NODE node;		//------------------------
	CRB3_NCH_ALG_TYPE type;	// PMASK	|	FIXED
	uint64_t value;			// pmask	|	hash-value
	NCH_ALG_HASH_FUNC func;
}CRB3_NCH_ALG_NODE;

class CRB3Client : public SoStream
{
	char ClientName[CRB3_MAX_CLIENT_NAME_SIZE];
	
	// log parameter
	LogManager * LogMng;
	
	// connect parameter
	pthread_rwlock_t ConnectLocker;
	pthread_t ReceiveThid;
	LIST WorkingList;		// <CRB3_CONNECTION>
	LIST BackupList;		// <CRB3_CONNECTION>
	LIST ExceptionList;		// <CRB3_CONNECTION>
	int ServiceMode;		// 0: Master-Slaver  1: Balance  2: Consistent-Hashing
	int EPoll;
	int DgramSocket;
	int DgramSocket6;
	int MaxConcurrent;
	int NotifyTimeout;
	int NotifyGap;

	// consistent hashing parameter
	CRB3_NCH_MAP NCHMap;
	IHASH NCHAlg;			// <CRB3_NCH_ALG_NODE>
	CRB3_NCH_ALG_NODE NCHAlgDef;
	IRBT_TREE NCHTree;		// <CRB3_CONNECTION>

	// direct io parameter
	bool DioEnable;
	pthread_mutex_t DioMutex;
	int DioPortStart;
	int DioSequence;
	LIST DioList;			// <CRB3_DIO_NODE>

	// CRL-Map parameter (for API-Mode)
	CRLMap CrlMgr;

	// Rsp-Queue parameter (for PKT-Mode)
	QUEUE RspQueue;			// <CRB3_RSP>
	pthread_mutex_t RspMutex;
	pthread_cond_t RspCond;

	// Check-Queue parameter
	QUEUE CheckQueue;		// <CRB3_NOTIFY>
	pthread_mutex_t CheckMutex;
	pthread_cond_t CheckCond;
	pthread_t CheckThid;

	// exit signal variable
	bool CheckExit;
	bool Running;

public:
	// statistics
	uint64_t PktRxSucceed;
	uint64_t PktRxDiscard;
	RateInfo * ReqQps;
	
public:
	CRB3Client(IN int Mode, IN char * Name = NULL);
	~CRB3Client();

	// configure interface
	int ConfigFromFile(IN char * CfgFile);
	int CustomHashAlg(IN uint32_t CRId, IN NCH_ALG_HASH_FUNC Func, IN uint64_t Spare);
	int CustomHashAlgDefault(IN NCH_ALG_HASH_FUNC Func, IN uint64_t Spare);
	int AddConnect(IN char * IpAddress, IN int Port, IN int Protocol = SOCK_DGRAM, IN char * RateLogPath = NULL);
	int DelConnect(IN char * IpAddress, IN int Port);

	// obsolete interface
	int ConfigLogPath(IN char * File);
	int ConfigRatePath(IN char * File);
	int ConfigMaxConcurrent(IN int Concurrent);
	int ConfigNotify(IN int TimeoutSec, IN int GapSec);
	int ConfigDio(IN bool IsEnable, IN int PortStart);

	// control interface
	int Start(void);
	int Stop(void);

	// request interface (API-Mode)
	int Request(IN uint32_t CRId, IN CRB3Param * ParamIn, IN int TimeoutMS, OUT CRB3Param ** ParamOut);
	int RequestAsync(IN uint32_t CRId, IN CRB3Param * ParamIn, IN CRB3_CLIENT_CALLBACK CbFunc, IN void * CbArg, IN int TimeoutMS);
	int RequestDio(IN uint32_t CRId, IN CRB3Param * ParamIn, IN int TimeoutMS, OUT CRB3Param ** ParamOut);

	// request interface (PKT-Mode)
	int CRSend(IN uint32_t CRId, IN uint32_t CRSeq, IN CRB3Param * ParamIn, IN int TimeoutMS);
	int CRRecv(OUT uint32_t * CRId, OUT uint32_t * CRSeq, OUT CRB3Param ** ParamOut, IN int TimeoutMS);

	// show interface
	int ShowConnect(OUT char * OutputBuf, IN int BufLen);
	int ShowQueue(OUT char * OutputBuf, IN int BufLen);

private:
	struct timespec * _InitTimeSpec(IN int TimeoutMS, INOUT struct timespec * TimeSpec);
	int _CRSend(IN CRB3_ADDRESS * Addr, IN int Protocol, IN uint8_t * PacketBuf, IN int BufSize);
	int _SQSend(IN CRB3_ADDRESS * Addr);
	int _ARSend(IN CRB3_ADDRESS * Addr);
	int _Response(IN uint32_t CRSeq, IN uint32_t CRId, IN uint16_t CRFlag, IN CRB3Param * ParamOut);
	CRB3_CONNECTION * _FindConnect(IN char * IpAddress, IN int Port, OUT LIST ** FromList);
	CRB3_CONNECTION * _FindConnect2(IN CRB3_ADDRESS Addr, OUT LIST ** FromList);
	CRB3_CONNECTION * _GetConnect(IN uint32_t CRId, IN CRB3Param * ParamIn);
	void _PutConnect(IN CRB3_CONNECTION * cnode);
	int _SelectConnect(IN uint32_t CRId, IN CRB3Param * ParamIn, OUT CRB3_ADDRESS * Addr, OUT int * Protocol);
	int _ShowConnectList(IN LIST * List, OUT char * OutputBuf, IN int BufLen);
	int _ReceiveDatagram(IN int sockfd);
	int _ReceiveLoop(void);
	int _CheckLoop(void);
	void _GoAction(IN LIST * FromList, IN char * FromStatus);
	void _CheckStream(IN CRB3_CONNECTION * cnode);
	int _PacketProcess(IN CRB3_ADDRESS * from_addr, IN char * packet, IN int packet_size);
	int _ProcessDioResponse(IN char * packet_buf, IN int packet_size, OUT CRB3Param ** ParamOut);
	int _NCHInit(void);
	int _NCHLoad(IN int init_nodes);
	int _NCHGet(IN int srv_id);
	int _NCHAlgConfig(IN const char * nch_alg);
	int _NCHAlgSetFixed(IN uint32_t CRId, IN int ServerId);
	int _NCHAlgSetPMask(IN uint32_t CRId, IN uint64_t ParamMask);
	int _NCHAlgSetCustom(IN uint32_t CRId, IN NCH_ALG_HASH_FUNC Func, IN uint64_t Spare);
	CRB3_CONNECTION * _NCHAlgCalc(IN uint32_t CRId, IN CRB3Param * Param);
	int _NCHFree(void);

private:
	virtual int PacketSink(IN uint8_t * PacketBuf, IN int PacketSize, IN SO_ADDRESS * FromAddr, IN int FromSocket, IN void * Spare);

private:
	static void * ReceiveThread(IN void * arg);
	static void * CheckThread(IN void * arg);
};

#ifdef NAMESPACE
}
#endif
#endif

