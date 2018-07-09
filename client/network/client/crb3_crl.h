#ifndef _IN_crb3_crlh_
#define _IN_crb3_crlh_

#include <pthread.h>
#include "hashLib.h"
#include "flstLib.h"
#include "heapTimer.h"
#include "crb3_define.h"
#include "crb3_param.h"

#ifdef NAMESPACE
namespace common{
#endif

#define CRB3_CRL_NUM		16
#define CRB3_CRL_MASK		(CRB3_CRL_NUM - 1)

typedef void (*CRB3_CLIENT_CALLBACK)(uint32_t CRId, CRB3Param * ParamOut, void * CbArg);

typedef struct
{
	IHASH_NODE node;			// int_key is 'CRSeq'
	HTIMER_NODE tnode;			// valid for async-interface
	uint32_t CRId;
	uint16_t IfType;			// 0: Sync  1: Async
	CRB3Param * ParamOut;
	CRB3_CLIENT_CALLBACK CbFunc;	// valid for async-interface
	void * CbArg;					// valid for async-interface
}CRB3_CRL;

class CRLMap
{
	IHASH CRLTable;					// <CRB3_CRL>
	HTIMER_POOL CbTimerPool;		// <CRB3_CRL>, used by async-interface
	FLIST_ID CRLPool;
	void * CRLBuf;					// include hash bank buffer & freelist pool buffer
	uint32_t CRLSequence;
	uint32_t CRLRefNum;				// used by sync-interface
	pthread_mutex_t CRLMutex;
	pthread_cond_t CRLCond[CRB3_CRL_NUM];
	bool Ready;

	// statistics
	uint64_t ApiRxSucceed;
	uint64_t ApiRxDiscard;
	uint64_t AsyncTimeout;

	// configure
	int CfgMaxCRLSize;

public:
	CRLMap();
	~CRLMap();

	int ConfigMaxCRLSize(IN int MaxCRLSize);

	int Start(void);

	int Stop(void);

	int AddCRLSync(IN uint32_t CRId, IN CRB3Param * ParamIn,
					OUT uint32_t * CRSeq, OUT uint8_t * ParamBuf, INOUT int * BufSize)
	{
		return _AddCRL(CRId, ParamIn, 0, NULL, NULL, 0, CRSeq, ParamBuf, BufSize);
	}
	
	int AddCRLAsync(IN uint32_t CRId, IN CRB3Param * ParamIn,
					IN CRB3_CLIENT_CALLBACK CbFunc, IN void * CbArg, IN int CbTimeoutMS,
					OUT uint32_t * CRSeq, OUT uint8_t * ParamBuf, INOUT int * BufSize)
	{
		return _AddCRL(CRId, ParamIn, 1, CbFunc, CbArg, CbTimeoutMS, CRSeq, ParamBuf, BufSize);
	}
	
	int WaitCRL(IN uint32_t CRSeq, IN struct timespec * TimeSpec, OUT CRB3Param ** ParamOut);
	
	int DelCRL(IN uint32_t CRSeq);
	
	int FillCRL(IN uint32_t CRSeq, IN CRB3Param * ParamOut);
	
private:
	int _AddCRL(IN uint32_t CRId, IN CRB3Param * ParamIn, IN uint16_t IfType,
					IN CRB3_CLIENT_CALLBACK CbFunc, IN void * CbArg, IN int CbTimeoutMS,
					OUT uint32_t * CRSeq, OUT uint8_t * ParamBuf, INOUT int * BufSize);
	void _CRLTimeOutFunc(IN CRB3_CRL * CrlEntry);
	
private:
	static void CRLTimeOutFunc(IN HTIMER_NODE * pNode, IN void * Arg);
};

#ifdef NAMESPACE
}
#endif
#endif

