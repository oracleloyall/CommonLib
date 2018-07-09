/**********************************************************
* crb3_crl.cpp
* Description: crl module of common request broker v3 library source code
*	   
* Copyright 2013 miaozz
*
* Create : 16 Sep. 2013 , miaozz
*
* Modify : 15 May. 2015 , miaozz
*		optimize CRLMap with multi-crl-channel
*
***********************************************************/
#include <sys/types.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include "crb3_crl.h"

#ifdef NAMESPACE
namespace common{
#endif

#ifndef MAX_CLIENT_CRL_SIZE
#define MAX_CLIENT_CRL_SIZE			10000
#endif

CRLMap::CRLMap()
{
	int idx;
	
	// initialize variable
	CRLSequence = 0;
	CRLRefNum = 0;
	pthread_mutex_init(&CRLMutex, NULL);
	for(idx=0;idx<CRB3_CRL_NUM;idx++)
	{
		pthread_cond_init(&CRLCond[idx], NULL);
	}
	Ready = false;
	ApiRxSucceed = 0;
	ApiRxDiscard = 0;
	
	CRLBuf = NULL;
	memset(&CRLTable, 0, sizeof(IHASH));
	memset(&CbTimerPool, 0, sizeof(HTIMER_POOL));
	CRLPool = NULL;

	CfgMaxCRLSize = MAX_CLIENT_CRL_SIZE;
}

CRLMap::~CRLMap()
{
	int idx;
	
	Stop();
	pthread_mutex_destroy(&CRLMutex);
	for(idx=0;idx<CRB3_CRL_NUM;idx++)
	{
		pthread_cond_destroy(&CRLCond[idx]);
	}
}

/********************************************************
* ������: CRLMap::ConfigMaxCRLSize
*
* ��������: ����CRLPool�Ĵ�С��(ֻ����CRLMap::Start�ӿڵ���ǰ����)
*
* ����:
*      MaxCRLSize - (����) �����õ�CRLPool�������Ŀ����0��ֵ��ʾ������
*
* ����ֵ:
*      CRB3_OK - �����ɹ�
*      CRB3_FAILED  - ����ʧ��: CRLPool��ʹ���У��޷��ı�
*      
********************************************************/
int CRLMap::ConfigMaxCRLSize(IN int MaxCRLSize)
{
	if(CRLPool)
	{
		return CRB3_FAILED;
	}
	CfgMaxCRLSize = MaxCRLSize;
	return CRB3_OK;
}

/********************************************************
* ������: CRLMap::Start
*
* ��������: ����CRLMap
*
* ����: ��
*
* ����ֵ:
*      CRB3_OK - �����ɹ�
*      CRB3_FAILED    - ����ʧ��: CRL hash table/timer/pool����ʧ��
*      CRB3_DUPLICATE - ����ʧ��: �Ѿ�����
*      
********************************************************/
int CRLMap::Start(void)
{
	int crl_size = CfgMaxCRLSize <= 0 ? MAX_CLIENT_CRL_SIZE : CfgMaxCRLSize;
	unsigned long bank_size = ihashGetBankSize(crl_size);
	unsigned long pool_size = flstGetMemSize(crl_size, sizeof(CRB3_CRL));

	// alloc crl buffer
	pthread_mutex_lock(&CRLMutex);
	if(Ready == true)
	{
		pthread_mutex_unlock(&CRLMutex);
		return CRB3_DUPLICATE;
	}
	CRLBuf = malloc(bank_size + pool_size);
	assert(CRLBuf);
	
	// initialize crl hash table
	if(0 != ihashInit(&CRLTable, crl_size, CRLBuf))
	{
		free(CRLBuf);
		CRLBuf = NULL;
		pthread_mutex_unlock(&CRLMutex);
		return CRB3_FAILED;
	}

	// initialize crl timer
	if(0 != htimerInit(&CbTimerPool, crl_size, CRLTimeOutFunc, this, &CRLMutex))
	{
		ihashFree(&CRLTable);
		free(CRLBuf);
		CRLBuf = NULL;
		pthread_mutex_unlock(&CRLMutex);
		return CRB3_FAILED;
	}

	// initialize crl pool
	if(NULL == (CRLPool = flstCreate((void *)((unsigned long)CRLBuf + bank_size),
					crl_size, sizeof(CRB3_CRL), CfgMaxCRLSize <= 0 ? FLST_EXTEND_LINEAR : FLST_EXTEND_NONE, MAX_CLIENT_CRL_SIZE)))
	{
		htimerDestroySignal(&CbTimerPool);
		ihashFree(&CRLTable);
		free(CRLBuf);
		CRLBuf = NULL;
		pthread_mutex_unlock(&CRLMutex);
		htimerDestroyJoin(&CbTimerPool);
		return CRB3_FAILED;
	}
	Ready = true;
	pthread_mutex_unlock(&CRLMutex);
	return CRB3_OK;
}

/********************************************************
* ������: CRLMap::Stop
*
* ��������: ֹͣCRLMap
*
* ����: ��
*
* ����ֵ: 
*      CRB3_OK - �����ɹ�
*
********************************************************/
int CRLMap::Stop(void)
{
	int rc_sig = -1;
	
	pthread_mutex_lock(&CRLMutex);
	if(Ready == true)
	{
		struct timespec ts;
		int idx;
		int retry = 100;
		
		// wait interface 'WaitCRL' completed
		Ready = false;
		for(idx=0;idx<CRB3_CRL_NUM;idx++)
		{
			pthread_cond_broadcast(&CRLCond[idx]);
		}
		while(CRLRefNum > 0 && (--retry))
		{
			pthread_mutex_unlock(&CRLMutex);
			usleep(1000);
			pthread_mutex_lock(&CRLMutex);
		}

		// destroy crl pool
		flstDestroy(CRLPool);

		// delete all timer, and destroy it.
		htimerDeleteAll(&CbTimerPool);
		rc_sig = htimerDestroySignal(&CbTimerPool);

		// destroy crl table
		ihashFree(&CRLTable);
	}
	pthread_mutex_unlock(&CRLMutex);

	// join timer thread
	if(rc_sig == 0)
	{
		htimerDestroyJoin(&CbTimerPool);
	}

	return CRB3_OK;
}

/********************************************************
* ������: CRLMap::WaitCRL
*
* ��������: �ȴ�CRL��Ŀ��ɡ�
*           �˺�������ͬ���ӿ�CRB3Client::Request���á�
*
* ����:
*      CRSeq     - (����) ����˳���
*      TimeSpec  - (����) �ȴ������ĳ�ʱʱ��㣬NULl��ʾ���ȴ�
*      ParamOut  - (���) CRL��ɺ������������ָ��
*
* ����ֵ:
*      CRB3_OK - �����ɹ�
*      CRB3_TIMEOUT - �ȴ���ʱ
*      CRB3_FAILED  - ����ȡ����CRL�ڲ�����
*      CRB3_NOT_SERVICE - ����ʧ��: ����δ����
*      
********************************************************/
int CRLMap::WaitCRL(IN uint32_t CRSeq, IN struct timespec * TimeSpec, OUT CRB3Param ** ParamOut)
{
	int irc;
	CRB3_CRL * crl_entry;
	pthread_cond_t * cond = &CRLCond[CRSeq & CRB3_CRL_MASK];

	// wait crl entry completed
	irc = 0;
	pthread_mutex_lock(&CRLMutex);
	if(!Ready)
	{
		pthread_mutex_unlock(&CRLMutex);
		return CRB3_NOT_SERVICE;
	}
	CRLRefNum++;
	crl_entry = (CRB3_CRL *)ihashSearch(&CRLTable, (unsigned long)CRSeq);
	while(crl_entry && (crl_entry->ParamOut == NULL) && Ready == true && irc != ETIMEDOUT)
	{
		if(TimeSpec == NULL)
		{
			irc = pthread_cond_wait(cond, &CRLMutex);
		}
		else
		{
			irc = pthread_cond_timedwait(cond, &CRLMutex, TimeSpec);
		}
		if(Ready == true)
		{
			crl_entry = (CRB3_CRL *)ihashSearch(&CRLTable, (unsigned long)CRSeq);
		}
		else
		{
			crl_entry = NULL;
		}
	}
	if(crl_entry == NULL || Ready == false)
	{
		CRLRefNum--;
		pthread_mutex_unlock(&CRLMutex);
		return CRB3_FAILED;
	}
	if(crl_entry->ParamOut)
	{
		if(ParamOut)
		{
			*ParamOut = crl_entry->ParamOut;
		}
		else
		{
			delete crl_entry->ParamOut;
		}
		irc = CRB3_OK;
	}
	else
	{
		irc = CRB3_TIMEOUT;
	}

	// delete crl entry, and free it's buffer
	crl_entry = (CRB3_CRL *)ihashDelete(&CRLTable, (unsigned long)CRSeq);
	flstFree(CRLPool, crl_entry);
	CRLRefNum--;
	pthread_mutex_unlock(&CRLMutex);

	return irc;
}

/********************************************************
* ������: CRLMap::DelCRL
*
* ��������: ɾ��CRL��Ŀ
*
* ����:
*      CRSeq - (����) ����˳���
*
* ����ֵ:
*      CRB3_OK - �����ɹ�
*      CRB3_NOT_FIND - ����ʧ��: CRL��Ŀ������
*      CRB3_NOT_SERVICE - ����ʧ��: ����δ����
*
********************************************************/
int CRLMap::DelCRL(IN uint32_t CRSeq)
{
	CRB3_CRL * crl_entry;
	int irc = CRB3_NOT_FIND;
	
	pthread_mutex_lock(&CRLMutex);
	if(!Ready)
	{
		pthread_mutex_unlock(&CRLMutex);
		return CRB3_NOT_SERVICE;
	}
	crl_entry = (CRB3_CRL *)ihashDelete(&CRLTable, (unsigned long)CRSeq);
	if(crl_entry)
	{
		// �첽�ӿ���Ҫɾ����ʱ���ڵ�
		if(crl_entry->IfType == 1 && crl_entry->tnode.sched_time.tv_sec > 0)
		{
			htimerDelete(&CbTimerPool, &crl_entry->tnode);
		}
		flstFree(CRLPool, crl_entry);
		irc = CRB3_OK;
	}
	pthread_mutex_unlock(&CRLMutex);

	return irc;
}

/********************************************************
* ������: CRLMap::FillCRL
*
* ��������: ��䲢���CRL��Ŀ
*
* ����:
*      CRSeq    - (����) ����˳���
*      ParamOut - (����) ������
*
* ����ֵ:
*      CRB3_OK - �����ɹ�
*      CRB3_NOT_FIND - ����ʧ��: CRL��Ŀ������
*      CRB3_NOT_SERVICE - ����ʧ��: ����δ����
*      
********************************************************/
int CRLMap::FillCRL(IN uint32_t CRSeq, IN CRB3Param * ParamOut)
{
	CRB3_CRL * crl_entry;

	pthread_mutex_lock(&CRLMutex);
	if(!Ready)
	{
		pthread_mutex_unlock(&CRLMutex);
		return CRB3_NOT_SERVICE;
	}
	crl_entry = (CRB3_CRL *)ihashSearch(&CRLTable, (unsigned long)CRSeq);
	if(crl_entry == NULL)
	{
		printf("not find crl entry for sequence %u\n", CRSeq);
		ApiRxDiscard++;
		pthread_mutex_unlock(&CRLMutex);
		return CRB3_NOT_FIND;
	}
	ApiRxSucceed++;
	if(crl_entry->IfType == 0)
	{
		crl_entry->ParamOut = ParamOut;
		pthread_cond_broadcast(&CRLCond[CRSeq & CRB3_CRL_MASK]);
		pthread_mutex_unlock(&CRLMutex);
	}
	else
	{
		CRB3_CLIENT_CALLBACK cb_func = crl_entry->CbFunc;
		void * cb_arg = crl_entry->CbArg;
		uint32_t crid = crl_entry->CRId;

		ihashDelete(&CRLTable, (unsigned long)CRSeq);
		if(crl_entry->tnode.sched_time.tv_sec > 0)
		{
			htimerDelete(&CbTimerPool, &crl_entry->tnode);
		}
		flstFree(CRLPool, crl_entry);
		pthread_mutex_unlock(&CRLMutex);

		cb_func(crid, ParamOut, cb_arg);
	}
	return CRB3_OK;
}

/********************************************************
* ������: CRLMap::_AddCRL
*
* ��������: ���CRL��Ŀ
*
* ����:
*      CRId        - (����) ����ID
*      ParamIn     - (����) ��������б�
*      IfType      - (����) �ӿ�����: 0��ʾͬ���ӿڣ�1��ʾ�첽�ӿ�
*      CbFunc      - (����) �ص�����ָ��(�������첽�ӿ�)
*      CbArg       - (����) �ص���������(�������첽�ӿ�)
*      CbTimeoutMS - (����) CRL��Ŀ��ʱʱ�䣬��λ: ���룬0��ֵ��ʾһֱ�ȴ�(�������첽�ӿ�)
*      CRSeq       - (���) ����˳���
*      ParamBuf    - (���) �������ݵĻ�������ַ
*      BufSize     - (����) �������ݵĻ�������С����λ: �ֽ�
*                    (���) �������ݵ�ʵ�ʴ�С����λ: �ֽ�
*
* ����ֵ:
*      CRB3_OK - �����ɹ�
*      CRB3_FULL        - ����ʧ��: CRL������
*      CRB3_DUPLICATE   - ����ʧ��: ˳����ظ�
*      CRB3_NOT_SERVICE - ����ʧ��: ����δ����
*
********************************************************/
int CRLMap::_AddCRL(IN uint32_t CRId, IN CRB3Param * ParamIn, IN uint16_t IfType,
					IN CRB3_CLIENT_CALLBACK CbFunc, IN void * CbArg, IN int CbTimeoutMS,
					OUT uint32_t * CRSeq, OUT uint8_t * ParamBuf, INOUT int * BufSize)
{
	int tlv_len = ParamIn->GetTLVLength();
	CRB3_CRL * crl_entry;
	uint8_t * ptr;
	uint32_t psize = (uint32_t)tlv_len;
	uint32_t sequence;

	// check buffer size
	if(tlv_len + CRB3_PARAM_RESERVE_LEN > *BufSize)
	{
		return CRB3_INVALID_PARAMETER;
	}
	
	// alloc crl entry
	pthread_mutex_lock(&CRLMutex);
	if(!Ready)
	{
		pthread_mutex_unlock(&CRLMutex);
		return CRB3_NOT_SERVICE;
	}
	crl_entry = (CRB3_CRL *)flstAlloc(CRLPool);
	if(crl_entry == NULL)
	{
		pthread_mutex_unlock(&CRLMutex);
		return CRB3_FULL;
	}

	// fill crl entry
	sequence = CRLSequence++;
	memset(crl_entry, 0, sizeof(CRB3_CRL));
	crl_entry->CRId = CRId;
	crl_entry->IfType = IfType;
	crl_entry->ParamOut = NULL;
	crl_entry->CbFunc = CbFunc;
	crl_entry->CbArg = CbArg;
	if(IfType == 1 && CbTimeoutMS > 0)
	{
		clock_gettime(CLOCK_REALTIME, &crl_entry->tnode.sched_time);
		crl_entry->tnode.sched_time.tv_nsec += (CbTimeoutMS % 1000) * 1000000;
		crl_entry->tnode.sched_time.tv_sec += CbTimeoutMS / 1000;
		if(crl_entry->tnode.sched_time.tv_nsec >= 1000000000)
		{
			crl_entry->tnode.sched_time.tv_nsec -= 1000000000;
			crl_entry->tnode.sched_time.tv_sec++;
		}
		htimerInsert(&CbTimerPool, &crl_entry->tnode);
	}

	// add to hash table
	if(0 != ihashInsert(&CRLTable, (IHASH_NODE *)crl_entry, (unsigned long)sequence))
	{
		flstFree(CRLPool, crl_entry);
		pthread_mutex_unlock(&CRLMutex);
		return CRB3_DUPLICATE;
	}
	pthread_mutex_unlock(&CRLMutex);
	
	// fill parameter buffer header
	ptr = ParamBuf;
	CRB3_SUBMIT_INT16_A(ptr, 1);							// ReqNum
	CRB3_SUBMIT_INT16_A(ptr, 0);							// ReqFlag
	CRB3_SUBMIT_INT32_A(ptr, sequence);						// CR-Seq
	CRB3_SUBMIT_INT32_A(ptr, crl_entry->CRId);				// CR-Id
	CRB3_SUBMIT_INT16_A(ptr, 0);							// CR-Flag: API-Mode
	CRB3_SUBMIT_INT16_A(ptr, (int16_t)ParamIn->ParamCount);	// PNum
	CRB3_SUBMIT_INT32_A(ptr, psize);						// PSize

	// fill parameter buffer body
	ParamIn->OutputTLV((char *)ptr);

	*CRSeq = sequence;
	*BufSize = tlv_len + CRB3_PARAM_RESERVE_LEN;
	return CRB3_OK;
}

void CRLMap::_CRLTimeOutFunc(IN CRB3_CRL * CrlEntry)
{
	CRB3_CRL * crl_entry;
	
	crl_entry = (CRB3_CRL *)ihashDelete(&CRLTable, CrlEntry->node.int_key);
	if(crl_entry)
	{
		crl_entry->CbFunc(crl_entry->CRId, NULL, crl_entry->CbArg);
		flstFree(CRLPool, crl_entry);
		AsyncTimeout++;
	}
}

void CRLMap::CRLTimeOutFunc(IN HTIMER_NODE * pNode, IN void * Arg)
{
	CRLMap * MapObj = (CRLMap *)Arg;

	MapObj->_CRLTimeOutFunc((CRB3_CRL *)((unsigned long)pNode - offsetof(CRB3_CRL, tnode)));
}

#ifdef NAMESPACE
}
#endif
