/**********************************************************
* crb3_server.cpp
* Description: server module of common request broker v3 library source code
*	   
* Copyright 2013 miaozz
*
* Create : 13 Sep. 2013 , miaozz
*          changed from crb2:
*          1) support serialize or un-serialize process for each client
*          2) not support SetStatus interface, but support GetStatus virtual interface
*          3) data channel both support tcp stream and udp datagram
*          4) control tunnel is separated from data tunnel
*
* Modify : 11 Aug. 2013 , miaozz
*          add async interface
*          6 Nov. 2015 , miaozz
*          add CRB3_NO_RSP option for service process result
*          12 Nov. 2015 , miaozz
*          CRB3Server::_SendCRResponse will not return SOERR_PEND
*          add flow control function
*          24 Dec. 2015 , miaozz
*          set server socket option 'SO_KEEPALIVE'
*          31 Mar. 2016 , miaozz
*          discard control socket.
*          add log & rate function
*          add alive management function
*          14 Jun. 2016 , miaozz
*          security coding: replace snprintf with seprintf
*          22 May. 2018 , miaozz
*          optimized packet process for single thread
***********************************************************/
#include <sys/types.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>
#include <sys/epoll.h>
#include "Profile.h"
#include "crb3_param.h"
#include "crb3_server.h"

#ifdef NAMESPACE
namespace common{
#endif

#undef USE_SCHED

#ifndef CRB3_ALIVE_TIMEOUT
#define CRB3_ALIVE_TIMEOUT		10
#endif

#ifndef CRB3_ALIVE_IDLE
#define CRB3_ALIVE_IDLE			5
#endif

#ifndef CRB3_ALIVE_CHECK
#define CRB3_ALIVE_CHECK		1
#endif

#define ALLOC_PACKET_BUFFER(mutex, flist, packet)	\
				do\
				{\
					pthread_mutex_lock(mutex);\
					packet = (CRB3_PACKET_NODE *)flstAlloc(flist);\
					pthread_mutex_unlock(mutex);\
				}while(0)
				
#define FREE_PACKET_BUFFER(mutex, flist, packet)	\
				do\
				{\
					pthread_mutex_lock(mutex);\
					flstFree(flist, packet);\
					pthread_mutex_unlock(mutex);\
				}while(0)

#define LOCAL_FUNCTION
#ifdef LOCAL_FUNCTION
#endif

/********************************************************
* ������: CRB3Server::CRB3Server
*
* ��������: ��ʼ��CRB3�����
*
* ����: 
*      Name - (����) ��������ƣ�NULL��ʾʹ��ȱʡ����"default"
*
* ����ֵ: ��
*
********************************************************/
CRB3Server::CRB3Server(IN char * Name)
			:SoStream()
{
	char format[CRB3_MAX_SERVER_NAME_SIZE + 32];
	
	// initialize client name
	if(Name)
	{
		snprintf(ServerName, sizeof(ServerName), "%s", Name);
	}
	else
	{
		strcpy(ServerName, "default");
	}
	snprintf(format, sizeof(format), "[%%t - %s] %%y: %%s", ServerName);
	lstInit(&AliveList);
	pthread_mutex_init(&AliveLock, NULL);

	// initialize log manager
	LogMng = new LogManager(0, LOGFLAG_KEEP_THREADSAFE);
	LogMng->SetLogFile("./crb.%Y%m%d");
	LogMng->SetLogFormat(format, SEVERITY_ALL);
	LogMng->SetSeverityMask(SEVERITY_MASK_ALL);
	
	EPoll = -1;
	DgramSocket = -1;
	StreamSocket = -1;
	ReceiveThid = (pthread_t)-1;
	RxSuccess = 0;
	RxErrorLen = 0;
	RxErrorBuf = 0;
	ThreadTable = (CRB3_THREAD_CTRL *)malloc(sizeof(CRB3_THREAD_CTRL) * CRB3_MAX_SERVICE_THREADS);
	assert(ThreadTable);
	memset(ThreadTable, 0, sizeof(CRB3_THREAD_CTRL) * CRB3_MAX_SERVICE_THREADS);
	PacketCache = flstCreate(NULL, CRB3_MAX_QUEUE_SIZE,
						sizeof(CRB3_PACKET_NODE), FLST_EXTEND_NONE, 0);
	assert(PacketCache);
	pthread_mutex_init(&CacheLock, NULL);
	CfgSerializeEn = false;
	CfgAsyncEn = false;
	CfgThreadNumber = 1;
	CfgServicePort = 0;
	CfgMaxClient = 0;
	memset(CfgBindAddress, 0, sizeof(CfgBindAddress));
	ReqQps = NULL;
	
	// set decode header rule of crb protocol
	SetLengthLocation(0, 2, 0);
}

/********************************************************
* ������: CRB3Server::~CRB3Server
*
* ��������: ����CRB3�����
*
* ����: ��
*
* ����ֵ: ��
*
********************************************************/
CRB3Server::~CRB3Server()
{
	lstFree(&AliveList);
	pthread_mutex_destroy(&AliveLock);
	free(ThreadTable);
	flstDestroy(PacketCache);
	pthread_mutex_destroy(&CacheLock);
}

/********************************************************
* ������: CRB3Server::ConfigFromFile
*
* ��������: �������ļ��м�������
*
* ����:
*      CfgFile - (����) �����ļ���
*
* ����ֵ:
*      CRB3_OK - �����ɹ�
*      CRB3_NOT_FIND - δ�ҵ���Ӧ������
*      
********************************************************/
int CRB3Server::ConfigFromFile(IN char * CfgFile)
{
	PROFILE cfg;

	if(cfg.Load(CfgFile))
	{
		PF_SECTION * section;
		char sec_name[80];

		snprintf(sec_name, sizeof(sec_name), "%s", ServerName);
		section = cfg.GetSection(sec_name);
		if(section)
		{
			char * str;
			int val;
			int max_client_num = 32;

			printf("loading configure from file '%s' section '%s' ... ", CfgFile, sec_name);
			if(NULL != (str = cfg.GetRecordString(section, "log_output")))
			{
				LogMng->SetLogFile(str);
			}
			if(true == cfg.GetRecordInteger(section, "log_level", &val))
			{
				LogMng->SetSeverityLevel(val);
			}
			if(NULL != (str = cfg.GetRecordString(section, "rate_output")))
			{
				ConfigRatePath(str);
			}
			if(true == cfg.GetRecordInteger(section, "serialize", &val))
			{
				ConfigSerialize(val ? true : false);
			}
			if(true == cfg.GetRecordInteger(section, "async", &val))
			{
				ConfigAsync(val ? true : false);
			}
			if(true == cfg.GetRecordInteger(section, "thread_num", &val))
			{
				ConfigThreadNumber(val);
			}
			if(true == cfg.GetRecordInteger(section, "max_client_num", &val))
			{
				max_client_num = val;
			}
			if(NULL != (str = cfg.GetRecordString(section, "bind_address")))
			{
				char buf[1024];
				char * str_ip;
				char * str_port;
				int port;

				memset(buf, 0, 1024);
				strncpy(buf, str, 1024);
				str_ip = strtok_r(buf, ",", &str_port);
				if(str_ip && str_port)
				{
					port = atoi(str_port);
					ConfigAddress(str_ip, port, max_client_num);
				}
				else
				{
					printf("syntax error of bind_address for %s! ", sec_name);
				}
			}
			printf("done\n");
			return CRB3_OK;
		}
	}
	return CRB3_NOT_FIND;
}

/********************************************************
* ������: CRB3Server::ConfigLogPath
*
* ��������: ������־·��������
*
* ����:
*      File - (����) ��־�ļ���������·��
*
* ����ֵ:
*      CRB3_OK - �����ɹ�
*      CRB3_INVALID_PARAMETER - ��Ч�Ĳ���
*
********************************************************/
int CRB3Server::ConfigLogPath(IN char * File)
{
	if(File)
	{
		LogMng->SetLogFile(File);
		return CRB3_OK;
	}
	return CRB3_INVALID_PARAMETER;
}

/********************************************************
* ������: CRB3Server::ConfigRatePath
*
* ��������: ��������ͳ����־·��
*
* ����:
*      File - (����) ����ͳ����־�ļ�����·����NULL��ʾ�����ͳ����Ϣ
*
* ����ֵ:
*      CRB3_OK - �����ɹ�
*      
********************************************************/
int CRB3Server::ConfigRatePath(IN char * File)
{
	if(ReqQps)
	{
		delete ReqQps;
		ReqQps = NULL;
	}
	if(File)
	{
		ReqQps = new RateInfo;
		ReqQps->AddRateCalc(1, 2, File);
	}
	return CRB3_OK;
}

/********************************************************
* ������: CRB3Server::ConfigSerialize
*
* ��������: ������ֹCRB3�ķ�������Կͻ��˽������л�
*
* ����:
*      IsEnable - (����) CRB3�����Ƿ񰴿ͻ��˽������л�
*
* ����ֵ:
*      CRB3_OK     - �����ɹ�
*      CRB3_FAILED - ����ʧ��: �������������������޸ĸ�����
*
********************************************************/
int CRB3Server::ConfigSerialize(IN bool IsEnable)
{
	if(DgramSocket != -1)
	{
		return CRB3_FAILED;
	}
	CfgSerializeEn = IsEnable;
	return CRB3_OK;
}

/********************************************************
* ������: CRB3Server::ConfigAsync
*
* ��������: ����CRB3�ķ���������첽�ӿ�"DoServiceAsync"
*
* ����:
*      IsEnable - (����) CRB3�����Ƿ�����첽�ӿ�
*
* ����ֵ:
*      CRB3_OK     - �����ɹ�
*      CRB3_FAILED - ����ʧ��: �������������������޸ĸ�����
*
********************************************************/
int CRB3Server::ConfigAsync(IN bool IsEnable)
{
	if(DgramSocket != -1)
	{
		return CRB3_FAILED;
	}
	CfgAsyncEn = IsEnable;
	return CRB3_OK;
}

/********************************************************
* ������: CRB3Server::ConfigThreadNumber
*
* ��������: ���÷������߳���Ŀ
*
* ����:
*      Num - (����) �������߳���Ŀ����Χ: 1 ~ CRB3_MAX_SERVICE_THREADS
*
* ����ֵ:
*      CRB3_OK     - �����ɹ�
*      CRB3_FAILED - ����ʧ��: �������������������޸ĸ�����
*      CRB3_INVALID_PARAMETER - ��Ч�Ĳ���
*      
********************************************************/
int CRB3Server::ConfigThreadNumber(IN int Num)
{
	int idx;
	
	if(Num <= 0 || Num > CRB3_MAX_SERVICE_THREADS)
	{
		LogMng->Printf(SEVERITY_ALERT, "configure thread number failed for invalid parameter '%d'", Num);
		return CRB3_INVALID_PARAMETER;
	}
	if(DgramSocket != -1)
	{
		if(Num > CfgThreadNumber)
		{
			for(idx=CfgThreadNumber;idx<Num;idx++)
			{
				_StartThread(idx);
			}
			#ifdef USE_SCHED
			schedAdjust(&Sched, Num);
			#endif
		}
		else if(Num < CfgThreadNumber)
		{
			#ifdef USE_SCHED
			schedAdjust(&Sched, Num);
			#endif
			for(idx=Num;idx<CfgThreadNumber;idx++)
			{
				_StopThread(idx);
			}
		}
	}
	CfgThreadNumber = Num;
	return CRB3_OK;
}

/********************************************************
* ������: CRB3Server::ConfigAddress
*
* ��������: ���÷���󶨵�ַ���˿ڣ��Լ���������ͻ�����Ŀ
*
* ����:
*      BindAddress - (����) �󶨵�ַ��"0.0.0.0"��"::"��ʾANY
*      ListenPort  - (����) ����˿�
*      MaxClient   - (����) ���ͻ�����Ŀ
*
* ����ֵ:
*      CRB3_OK     - �����ɹ�
*      CRB3_FAILED - ����ʧ��: �������������������޸ĸ�����
*      
********************************************************/
int CRB3Server::ConfigAddress(IN char * BindAddress, IN int ListenPort, IN int MaxClient)
{
	if(DgramSocket != -1)
	{
		return CRB3_FAILED;
	}
	memset(CfgBindAddress, 0, 64);
	strncpy(CfgBindAddress, BindAddress, 63);
	CfgServicePort = ListenPort;
	CfgMaxClient = MaxClient;
	return CRB3_OK;
}

/********************************************************
* ������: CRB3Server::Start
*
* ��������: ����CRB3����
*
* ����: ��
*
* ����ֵ:
*      CRB3_OK     - �����ɹ�
*      CRB3_FAILED - ����ʧ��: �����׽��ִ�����߰󶨵�ַ����
*      CRB3_DUPLICATE    - �����Ѿ�����
*      CRB3_EMPTY_MEMORY - �������Ļ�����ʧ��
*      
********************************************************/
int CRB3Server::Start(void)
{
	CRB3_ADDRESS data_addr;
	int idx;

	if(DgramSocket != -1)
	{
		return CRB3_DUPLICATE;
	}

	// create epoll & configure stream
	EPoll = epoll_create(CfgMaxClient);
	if(EPoll < 0)
	{
		goto crb3_start_error;
	}
	SetEPollFd(EPoll);

	// initialize bind address
	CRB3String2Address(CfgBindAddress, CfgServicePort, &data_addr);
	
	// create dgram & stream socket
	DgramSocket = _CreateSocket(&data_addr, SOCK_DGRAM);
	StreamSocket = _CreateSocket(&data_addr, SOCK_STREAM);
	if(DgramSocket < 0 || StreamSocket < 0)
	{
		goto crb3_start_error;
	}

	// create schedule & process threads
	#ifdef USE_SCHED
	schedInit(&Sched, CfgThreadNumber, CRB3_MAX_QUEUE_SIZE, GRADING_MIXED);
	#endif
	for(idx=0; idx<CfgThreadNumber; idx++)
	{
		_StartThread(idx);
	}
	
	// create receive thread
	pthread_create(&ReceiveThid, NULL, ReceiveThread, this);
	return CRB3_OK;

crb3_start_error:
	if(DgramSocket >= 0)
	{
		_DestroySocket(DgramSocket);
		DgramSocket = -1;
	}
	if(StreamSocket >= 0)
	{
		_DestroySocket(StreamSocket);
		StreamSocket = -1;
	}
	if(EPoll >= 0)
	{
		close(EPoll);
		EPoll = -1;
	}
	return CRB3_FAILED;
}

/********************************************************
* ������: CRB3Server::Stop
*
* ��������: ֹͣCRB3����
*
* ����: ��
*
* ����ֵ:
*      CRB3_OK - �����ɹ�
*      
********************************************************/
int CRB3Server::Stop(void)
{
	int idx;
	void * thrc;
	
	if(DgramSocket >= 0)
	{
		_DestroySocket(DgramSocket);
		DgramSocket = -1;
	}
	if(StreamSocket >= 0)
	{
		_DestroySocket(StreamSocket);
		StreamSocket = -1;
	}
	if(ReceiveThid != (pthread_t)-1)
	{
		pthread_join(ReceiveThid, &thrc);
		ReceiveThid = (pthread_t)-1;
	}
	DisconnectAll();
	if(EPoll >= 0)
	{
		close(EPoll);
		EPoll = -1;
	}
	for(idx=0; idx<CfgThreadNumber; idx++)
	{
		_StopThread(idx);
	}
	#ifdef USE_SCHED
	schedFree(&Sched);
	#endif
	return CRB3_OK;
}

/********************************************************
* ������: CRB3Server::ShowQueue
*
* ��������: ��ʾ�������
*
* ����:
*      OutputBuf - (���) �����������ַ
*      BufLen    - (����) �����������С����λ: �ֽ�
*
* ����ֵ: ʵ�������С����λ: �ֽ�
*      
********************************************************/
int CRB3Server::ShowQueue(OUT char * OutputBuf, IN int BufLen)
{
	int irc = 0;
	int idx;

	seprintf(OutputBuf, BufLen, irc,
			"Index    Queue      Weight\n");
	seprintf(OutputBuf, BufLen, irc,
			"-------------------------------------\n");
	for(idx=0; idx<CfgThreadNumber; idx++)
	{
		#ifdef CRB3_QFIFO
		uint32_t count = qfifoSize(&ThreadTable[idx].PacketQueue);
		#ifdef USE_SCHED
		seprintf(OutputBuf, BufLen, irc, "%-5d    %-9d  %d\n",
					idx, count, Sched.sch_node[idx].weight);
		#else
		seprintf(OutputBuf, BufLen, irc, "%-5d    %-9d  %s\n",
					idx, count, "N/A");
		#endif
		#else
		pthread_mutex_lock(&ThreadTable[idx].QueueMutex);
		#ifdef USE_SCHED
		seprintf(OutputBuf, BufLen, irc, "%-5d    %-9d  %d\n",
					idx, ThreadTable[idx].PacketQueue.count, Sched.sch_node[idx].weight);
		#else
		seprintf(OutputBuf, BufLen, irc, "%-5d    %-9d  %s\n",
					idx, ThreadTable[idx].PacketQueue.count, "N/A");
		#endif
		pthread_mutex_unlock(&ThreadTable[idx].QueueMutex);
		#endif
	}
	return irc;
}

/********************************************************
* ������: CRB3Server::ShowClient
*
* ��������: ��ʾ�ͻ����б�
*
* ����:
*      OutputBuf - (���) �����������ַ
*      BufLen    - (����) �����������С����λ: �ֽ�
*
* ����ֵ: ʵ�������С����λ: �ֽ�
*      
********************************************************/
int CRB3Server::ShowClient(OUT char * OutputBuf, IN int BufLen)
{
	int irc = 0;
	CRB3_ALIVE_NODE * anode;

	seprintf(OutputBuf, BufLen, irc,
			"Client IP          Port    LastNotify           Socket\n");
	seprintf(OutputBuf, BufLen, irc,
			"------------------------------------------------------\n");
	
	pthread_mutex_lock(&AliveLock);
	anode = (CRB3_ALIVE_NODE *)lstFirst(&AliveList);
	while(anode)
	{
		int sfd;
		int dir;
		bool cng;
		int port;
		char ip[64];
		char tbuf[64];
		char sock[8] = "N/A";
		struct tm datetime;

		CRB3Address2String(&anode->addr, ip, 64, &port);
		localtime_r(&anode->last_notify, &datetime);
		strftime(tbuf, 64, "%Y-%m-%d %X", &datetime);
		if(SOERR_OK == GetInfo(&anode->addr, &sfd, &dir, &cng))
		{
			snprintf(sock, 8, "%d", sfd);
		}
		seprintf(OutputBuf, BufLen, irc,
			"%-18s %-7d %-20s %s\n", ip, port, tbuf, sock);
		anode = (CRB3_ALIVE_NODE *)lstNext(&anode->node);
	}
	pthread_mutex_unlock(&AliveLock);
	return(0);
}

/********************************************************
* ������: CRB3Server::ShowClientStatistic
*
* ��������: ��ʾ�ͻ���ͳ��ֵ(�ݲ�֧��)
*
* ����:
*      Fd        - (����) �ͻ����׽���
*      OutputBuf - (���) �����������ַ
*      BufLen    - (����) �����������С����λ: �ֽ�
*
* ����ֵ: ʵ�������С����λ: �ֽ�
*      
********************************************************/
int CRB3Server::ShowClientStatistic(IN int Fd, OUT char * OutputBuf, IN int BufLen)
{
	return(0);
}

/********************************************************
* ������: CRB3Server::DoServiceResponse
*
* ��������: ����������(��������CRB3������첽�ӿ�ʱ����)
*
* ����:
*      ClientAddr - (����) �ͻ��˵�ַ
*      CRId       - (����) ����ID
*      ParamOut   - (����) ����������
*      RspArg     - (����) ����Ĳ�������ֵ��DoServiceAsync����
*
* ����ֵ:
*      SOERR_OK   - ���ͳɹ��������Ѳ��ֻ�ȫ������
*      SOERR_PEND - ����ʧ�ܣ�Pend�����е������ڵȴ�(��TCP)
*      SOERR_INVALID_CLIENT - ����ʧ�ܣ���Ч�Ŀͻ��˵�ַ(��TCP)
*      SOERR_SOCKET_ERROR   - ����ʧ�ܣ��׽��ִ��󣬾�����Ϣ�鿴errno
********************************************************/
int CRB3Server::DoServiceResponse(IN CRB3_ADDRESS * ClientAddr, IN uint32_t CRId, IN CRB3Param * ParamOut, IN uint64_t RspArg)
{
	return _SendCRResponse(ClientAddr, (int)(RspArg & 0xffff), (uint32_t)(RspArg >> 32),
							CRId, (uint16_t)((RspArg >> 16) & 0xffff), ParamOut);
}

void CRB3Server::_StartThread(IN int ThreadIndex)
{
	ThreadTable[ThreadIndex].ServerRef = this;
	ThreadTable[ThreadIndex].Thid = (pthread_t)-1;
	ThreadTable[ThreadIndex].IsExit = false;
	#ifdef CRB3_QFIFO
	qfifoInit(&ThreadTable[ThreadIndex].PacketQueue, CRB3_MAX_QUEUE_SIZE * sizeof(void *), NULL);
	#else
	pthread_mutex_init(&ThreadTable[ThreadIndex].QueueMutex, NULL);
	pthread_cond_init(&ThreadTable[ThreadIndex].QueueCond, NULL);
	lstInit(&ThreadTable[ThreadIndex].PacketQueue);
	#endif
	pthread_create(&ThreadTable[ThreadIndex].Thid, NULL, ProcessThread, &ThreadTable[ThreadIndex]);
}

void CRB3Server::_StopThread(IN int ThreadIndex)
{
	if(ThreadTable[ThreadIndex].Thid != (pthread_t)-1)
	{
		void * thrc;
		
		#ifdef CRB3_QFIFO
		ThreadTable[ThreadIndex].IsExit = true;
		#else
		pthread_mutex_lock(&ThreadTable[ThreadIndex].QueueMutex);
		ThreadTable[ThreadIndex].IsExit = true;
		pthread_cond_signal(&ThreadTable[ThreadIndex].QueueCond);
		pthread_mutex_unlock(&ThreadTable[ThreadIndex].QueueMutex);
		#endif
		pthread_join(ThreadTable[ThreadIndex].Thid, &thrc);
		ThreadTable[ThreadIndex].Thid = (pthread_t)-1;

		#ifdef CRB3_QFIFO
		qfifoFree(&ThreadTable[ThreadIndex].PacketQueue);
		#endif
	}
}

int CRB3Server::_PacketPush(IN CRB3_PACKET_NODE * Packet, IN int FromSocket)
{
	int index;

	// optimized for single thread
	if(CfgThreadNumber == 1)
	{
		RxSuccess++;
		_PacketProcess(Packet);
		_PacketFree(Packet);
		return CRB3_OK;
	}
	
	if(!CfgSerializeEn || Packet->protocol == SOCK_DGRAM)
	{
		#ifdef USE_SCHED
		index = schedGet(&Sched, 1);
		if(index < 0)
		{
			RxErrorBuf++;
			return CRB3_EMPTY_QUEUE;
		}
		#else
		static int next_proc_id = 0;

        index = next_proc_id;
        next_proc_id = (next_proc_id + 1) % CfgThreadNumber;
		#endif
	}
	else
	{
		index = FromSocket % CfgThreadNumber;
	}

	// push packet to process queue (it will not failed because the queue is larger enough for all pool packets)
	RxSuccess++;
	#ifdef CRB3_QFIFO 
	qfifoPush(&ThreadTable[index].PacketQueue, (uint8_t *)&Packet, sizeof(CRB3_PACKET_NODE *));
	#else
	pthread_mutex_lock(&ThreadTable[index].QueueMutex);
	lstAdd(&ThreadTable[index].PacketQueue, &Packet->node);
	if(ThreadTable[index].PacketQueue.count == 1)
	{
		pthread_cond_signal(&ThreadTable[index].QueueCond);
	}
	pthread_mutex_unlock(&ThreadTable[index].QueueMutex);
	#endif

	return CRB3_OK;
}

CRB3_PACKET_NODE * CRB3Server::_PacketPop(IN CRB3_THREAD_CTRL * ThreadCtrl)
{
	CRB3_PACKET_NODE * packet;

	#ifdef CRB3_QFIFO
	uint32_t pop_size;
	uint32_t pop_retry = 0;
	uint8_t * pop_buf = (uint8_t *)&packet;

	pop_size = qfifoPop(&ThreadCtrl->PacketQueue, pop_buf, sizeof(CRB3_PACKET_NODE *));
	while(pop_size < sizeof(CRB3_PACKET_NODE *) && !ThreadCtrl->IsExit)
	{
		if(++pop_retry < 100)
		{
			pthread_yield();
		}
		else
		{
			usleep(1000);
		}
		pop_size += qfifoPop(&ThreadCtrl->PacketQueue, pop_buf + pop_size, sizeof(CRB3_PACKET_NODE *) - pop_size);
	}
	if(ThreadCtrl->IsExit)
	{
		return NULL;
	}
	#else
	pthread_mutex_lock(&ThreadCtrl->QueueMutex);
	while(!ThreadCtrl->IsExit && !ThreadCtrl->PacketQueue.count)
	{
		pthread_cond_wait(&ThreadCtrl->QueueCond, &ThreadCtrl->QueueMutex);
	}
	if(ThreadCtrl->IsExit)
	{
		pthread_mutex_unlock(&ThreadCtrl->QueueMutex);
		return NULL;
	}
	packet = (CRB3_PACKET_NODE *)lstGet(&ThreadCtrl->PacketQueue);
	pthread_mutex_unlock(&ThreadCtrl->QueueMutex);
	#endif

	#ifdef USE_SCHED
	if(!CfgSerializeEn || packet->protocol == SOCK_DGRAM)
	{
		schedPut(&Sched, (int)(ThreadCtrl - ThreadTable), 1);
	}
	#endif
	return packet;
}

void CRB3Server::_PacketProcess(IN CRB3_PACKET_NODE * Packet)
{
	uint16_t hdr_len;
	uint16_t hdr_id;
	uint32_t hdr_status;
	char * ptr = Packet->packet_buf;
	char * ptr_end = ptr + Packet->packet_size;

	// decode packet & do service
	CRB3_FETCH_INT16_A(ptr, hdr_len);
	CRB3_FETCH_INT16_A(ptr, hdr_id);
	CRB3_FETCH_INT32_A(ptr, hdr_status);
	if(hdr_id == CRB3_PKT_CR_REQ)
	{
		uint16_t req_num;
		uint16_t req_flag;
		uint16_t idx;
		uint32_t rsid;

		if(ReqQps)
		{
			ReqQps->Pulse(1);
		}
		CRB3_FETCH_INT16_A(ptr, req_num);
		CRB3_FETCH_INT16_A(ptr, req_flag);
		for(idx=0;idx<req_num;idx++)
		{
			uint32_t cr_seq;
			uint32_t cr_id;
			uint16_t cr_flag;
			uint16_t pnum;
			uint32_t psize;

			CRB3_FETCH_INT32(ptr, cr_seq);
			CRB3_FETCH_INT32(ptr, cr_id);
			CRB3_FETCH_INT16(ptr, cr_flag);
			CRB3_FETCH_INT16(ptr, pnum);
			CRB3_FETCH_INT32(ptr, psize);
			if(ptr + psize <= ptr_end)
			{
				CRB3Param param_in((int)pnum, ptr, (int)psize);
				CRB3Param * param_out = NULL;
				int irc;
				
				ptr += psize;
				if(!CfgAsyncEn)
				{
					irc = DoService(&Packet->from_addr, cr_id, &param_in, &param_out);
					if(irc == CRB3_OK)
					{
						_SendCRResponse(&Packet->from_addr, Packet->protocol, cr_seq, cr_id, cr_flag, param_out);
					}
					else if(irc < CRB3_OK)
					{
						_SendCRResponseError(&Packet->from_addr, Packet->protocol, cr_seq, cr_id, cr_flag, irc);
					}
					if(param_out)
					{
						delete param_out;
					}
				}
				else
				{
					uint64_t rsp_arg;

					rsp_arg = ((uint64_t)cr_seq << 32) | (cr_flag << 16) | (Packet->protocol & 0xffff);
					irc = DoServiceAsync(&Packet->from_addr, cr_id, &param_in, rsp_arg);
					if(irc < CRB3_OK)
					{
						_SendCRResponseError(&Packet->from_addr, Packet->protocol, cr_seq, cr_id, cr_flag, irc);
					}
				}
			}
			else
			{
				printf("[CRB3] decode cr-request failed\n");
				break;
			}
		}
	}
}

CRB3_PACKET_NODE * CRB3Server::_PacketAlloc(void)
{
	CRB3_PACKET_NODE * packet;
	
	pthread_mutex_lock(&CacheLock);
	packet = (CRB3_PACKET_NODE *)flstAlloc(PacketCache);
	pthread_mutex_unlock(&CacheLock);

	return packet;
}

void CRB3Server::_PacketFree(IN CRB3_PACKET_NODE * Packet)
{
	pthread_mutex_lock(&CacheLock);
	flstFree(PacketCache, Packet);
	pthread_mutex_unlock(&CacheLock);
}

/********************************************************
* ������: CRB3Server::_CreateSocket
*
* ��������: �����׽��ֲ�����EPoll
*
* ����:
*      BindAddr - (����) �󶨵ĵ�ַ
*      Protocol - (����) Э��(SOCK_DGRAM/SOCK_STREAM)
*
* ����ֵ: �������׽��֣�-1��ʾʧ��
*      
********************************************************/
int CRB3Server::_CreateSocket(IN CRB3_ADDRESS * BindAddr, IN int Protocol)
{
	// create socket file description
	int fd = socket(BindAddr->sa.sa_family, Protocol, 0);

	if(fd >= 0)
	{
		socklen_t addr_len;
		int idx = 1;
		struct epoll_event ev;

		// bind address
		setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &idx, sizeof(idx));
		addr_len = BindAddr->sa.sa_family == AF_INET ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6);
		if(0 != bind(fd, &BindAddr->sa, addr_len))
		{
			char ip[64];
			int port;
			
			CRB3Address2String(BindAddr, ip, 64, &port);
			LogMng->Printf(SEVERITY_ALERT, "bind ip '%s' port '%d' failed", ip, port);
			close(fd);
			return(-1);
		}

		// listen if it's protocol is tcp
		if(Protocol == SOCK_STREAM)
		{
			setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &idx, sizeof(idx));
			listen(fd, 5);
		}

		// add to epoll
		ev.events = EPOLLIN;
		ev.data.fd = fd;
		if(-1 == epoll_ctl(EPoll, EPOLL_CTL_ADD, fd, &ev))
		{
			close(fd);
			fd = -1;
		}
	}
	return fd;
}

/********************************************************
* ������: CRB3Server::_DestroySocket
*
* ��������: ��EPoll���Ƴ��׽��֣����ر���
*
* ����:
*      SocketFd - (����) ��ɾ�����׽���
*
* ����ֵ: ��
********************************************************/
void CRB3Server::_DestroySocket(IN int SocketFd)
{
	struct epoll_event ev;
						
	// remove from epoll
	epoll_ctl(EPoll, EPOLL_CTL_DEL, SocketFd, &ev);

	// close it
	close(SocketFd);
}

/********************************************************
* ������: CRB3Server::_SendCRResponse
*
* ��������: ��ͻ��˷���������
*
* ����:
*      addr     - (����) �ͻ�������ͨ����ַ
*      protocol - (����) ����ͨ��Э��(SOCK_DGRAM / SOCK_STREAM)
*      crseq    - (����) ����˳���
*      crid     - (����) ����ID
*      crflag   - (����) �����־
*      param    - (����) �����������б�
*
* ����ֵ:
*      SOERR_OK   - ���ͳɹ��������Ѳ��ֻ�ȫ������
*      SOERR_INVALID_CLIENT - ����ʧ�ܣ���Ч�Ŀͻ��˵�ַ(��TCP)
*      SOERR_SOCKET_ERROR   - ����ʧ�ܣ��׽��ִ��󣬾�����Ϣ�鿴errno
********************************************************/
int CRB3Server::_SendCRResponse(IN CRB3_ADDRESS * addr, IN int protocol, IN uint32_t crseq, IN uint32_t crid, IN uint16_t crflag, IN CRB3Param * param)
{
	char packet_buf[CRB3_MAX_PACKET_SIZE];
	char * ptr = packet_buf;
	uint16_t pnum = 0;
	uint16_t psize = 0;
	int irc;

	if(param)
	{
		pnum = (uint16_t)param->ParamCount;
		psize = (uint16_t)param->GetTLVLength();
	}
	CRB3_SUBMIT_INT16(ptr, 28 + psize);
	CRB3_SUBMIT_INT16(ptr, CRB3_PKT_CR_RSP);
	CRB3_SUBMIT_INT32(ptr, 0);
	CRB3_SUBMIT_INT16(ptr, 1);		// RspNum
	CRB3_SUBMIT_INT16(ptr, 0);		// RspFlag
	CRB3_SUBMIT_INT32(ptr, crseq);
	CRB3_SUBMIT_INT32(ptr, crid);
	CRB3_SUBMIT_INT16(ptr, crflag);
	CRB3_SUBMIT_INT16(ptr, pnum);
	CRB3_SUBMIT_INT32(ptr, psize);
	if(param && (psize > 0))
	{
		param->OutputTLV(ptr);
	}
	if(protocol == SOCK_DGRAM)
	{
		irc = sendto(DgramSocket, packet_buf, 28 + psize, 0, &addr->sa, CRB3_SOCKLEN(addr));
		return irc > 0 ? SOERR_OK : SOERR_SOCKET_ERROR;
	}
	while(SOERR_PEND == (irc = Send(addr, packet_buf, 28 + psize)))
	{
		usleep(100);
	}
	return irc;
}

/********************************************************
* ������: CRB3Server::_SendCRResponseError
*
* ��������: ��ͻ��˷������������Ϣ
*
* ����:
*      addr     - (����) �ͻ�������ͨ����ַ
*      protocol - (����) ����ͨ��Э��(SOCK_DGRAM / SOCK_STREAM)
*      crseq    - (����) ����˳���
*      crid     - (����) ����ID
*      crflag   - (����) �����־
*      status   - (����) �������
*
* ����ֵ:
*      SOERR_OK   - ���ͳɹ��������Ѳ��ֻ�ȫ������
*      SOERR_INVALID_CLIENT - ����ʧ�ܣ���Ч�Ŀͻ��˵�ַ(��TCP)
*      SOERR_SOCKET_ERROR   - ����ʧ�ܣ��׽��ִ��󣬾�����Ϣ�鿴errno
********************************************************/
int CRB3Server::_SendCRResponseError(IN CRB3_ADDRESS * addr, IN int protocol, IN uint32_t crseq, IN uint32_t crid, IN uint16_t crflag, IN int status)
{
	char packet_buf[28];
	char * ptr = packet_buf;
	int irc;

	CRB3_SUBMIT_INT16(ptr, 28);
	CRB3_SUBMIT_INT16(ptr, CRB3_PKT_CR_RSP);
	CRB3_SUBMIT_INT32(ptr, status);
	CRB3_SUBMIT_INT16(ptr, 1);		// RspNum
	CRB3_SUBMIT_INT16(ptr, 0);		// RspFlag
	CRB3_SUBMIT_INT32(ptr, crseq);
	CRB3_SUBMIT_INT32(ptr, crid);
	CRB3_SUBMIT_INT16(ptr, crflag);
	CRB3_SUBMIT_INT16(ptr, 0);
	CRB3_SUBMIT_INT32(ptr, 0);

	if(protocol == SOCK_DGRAM)
	{
		irc = sendto(DgramSocket, packet_buf, 28, 0, &addr->sa, CRB3_SOCKLEN(addr));
		return irc > 0 ? SOERR_OK : SOERR_SOCKET_ERROR;
	}
	while(SOERR_PEND == (irc = Send(addr, packet_buf, 28)))
	{
		usleep(100);
	}
	return irc;
}

/********************************************************
* ������: CRB3Server::_SendStatusNotify
*
* ��������: ��ͻ��˷���״̬��Ӧ��Ϣ
*
* ����:
*      addr   - (����) �ͻ��˿���ͨ����ַ
*      status - (����) ��ǰ����״̬
*      weight - (����) ��ǰ����Ȩֵ
*
* ����ֵ: ���͵��ֽ�����0��ֵ��ʾ�д���
*      
********************************************************/
int CRB3Server::_SendStatusNotify(IN CRB3_ADDRESS * addr, IN uint32_t status, IN uint32_t weight)
{
	char packet_buf[16];
	char * ptr = packet_buf;

	CRB3_SUBMIT_INT16(ptr, 16);
	CRB3_SUBMIT_INT16(ptr, CRB3_PKT_STATUS_RSP);
	CRB3_SUBMIT_INT32(ptr, 0);
	CRB3_SUBMIT_INT32(ptr, status);
	CRB3_SUBMIT_INT32(ptr, weight);
	return sendto(DgramSocket, packet_buf, 16, 0, &addr->sa, CRB3_SOCKLEN(addr));
}

/********************************************************
* ������: CRB3Server::_SendAliveNotify
*
* ��������: ��ͻ��˷���ALIVEѯ����Ϣ
*
* ����:
*      addr - (����) �ͻ��˵�ַ
*
* ����ֵ: ���͵��ֽ�����0��ֵ��ʾ�д���
*      
********************************************************/
int CRB3Server::_SendAliveNotify(IN CRB3_ADDRESS * addr)
{
	char packet_buf[8];
	char * ptr = packet_buf;

	CRB3_SUBMIT_INT16(ptr, 8);
	CRB3_SUBMIT_INT16(ptr, CRB3_PKT_ALIVE_REQ);
	CRB3_SUBMIT_INT32(ptr, 0);
	return Send(addr, packet_buf, 8);
}

/********************************************************
* ������: CRB3Server::_ReceiveConnection
*
* ��������: ���ղ�����ͻ��˵�TCP��������
*
* ����:
*      sockfd        - (����) �����׽���
*
* ����ֵ:
*      CRB3_OK - ����ɹ�
*      CRB3_NOT_CONNECTED - ����ʧ��: �׽����ѹر�
********************************************************/
int CRB3Server::_ReceiveConnection(IN int sockfd)
{
	CRB3_ALIVE_NODE * alive;
	CRB3_ADDRESS remote;
	int addr_len = sizeof(CRB3_ADDRESS);
	int cfd;

	// accept incoming connection
	cfd = accept(sockfd, (struct sockaddr *)&remote, (socklen_t *)&addr_len);
	if(cfd < 0)
	{
		return CRB3_NOT_CONNECTED;
	}

	// create alive node
	alive = _UpdateAlive(&remote);

	// process stream connection
	ConnectFrom(&remote, cfd, alive);
	
	return CRB3_OK;
}

/********************************************************
* ������: CRB3Server::_ReceiveDatagram
*
* ��������: �����ݱ��׽����н��ղ�������(UDP)
*
* ����:
*      sockfd - (����) UDP�׽���
*
* ����ֵ:
*      CRB3_OK - ����ɹ�
*      CRB3_NOT_CONNECTED     - ����ʧ��: �׽����ѹر�
*      CRB3_INVALID_PARAMETER - ����ʧ��: ��Ч�ı���ͷ
*      CRB3_NOT_FIND          - ����ʧ��: δ֪�Ŀͻ���
*      CRB3_EMPTY_QUEUE       - ����ʧ��: ���Ļ�����
*      CRB3_NOT_SERVICE       - ����ʧ��: Ӧ�÷���δ����
********************************************************/
int CRB3Server::_ReceiveDatagram(IN int sockfd)
{
	CRB3_PACKET_NODE * packet;
	socklen_t len;
	uint16_t hdr_len;
	uint16_t hdr_id;
	
	// alloc packet buffer
	packet = _PacketAlloc();
	if(packet == NULL)
	{
		RxErrorBuf++;
		return CRB3_EMPTY_QUEUE;
	}

	// read data packet
	len = sizeof(packet->from_addr);
	packet->protocol = SOCK_DGRAM;
	packet->packet_size = recvfrom(sockfd, packet->packet_buf, CRB3_MAX_PACKET_SIZE, 0, &packet->from_addr.sa, &len);
	if(packet->packet_size <= 0)
	{
		// data socket closed
		_PacketFree(packet);
		RxErrorLen++;
		return CRB3_NOT_CONNECTED;
	}

	// check packet header
	else if(packet->packet_size < 8)
	{
		// discard packet for invalid packet header
		_PacketFree(packet);
		return CRB3_INVALID_PARAMETER;
	}

	// process status-req packet
	hdr_len = ntohs(*(uint16_t *)(packet->packet_buf));
	hdr_id = ntohs(*(uint16_t *)(packet->packet_buf + 2));
	if(hdr_id == CRB3_PKT_STATUS_REQ)
	{
		_SendStatusNotify(&packet->from_addr, (uint32_t)GetStatus(), 0);
		_PacketFree(packet);
		return CRB3_OK;
	}

	// process cr-req packet
	if(GetStatus() == CRB3_STATUS_WORKING)
	{
		int proc_id;
		
		// select process queue & push (udp not support serialize)
		if(CRB3_OK != _PacketPush(packet, sockfd))
		{
			_PacketFree(packet);
		}
		return CRB3_OK;
	}
	
	// discard packet for not service
	_PacketFree(packet);
	return CRB3_NOT_SERVICE;
}

/********************************************************
* ������: CRB3Server::_ReceiveLoop
*
* ��������: ���Ľ���ѭ��
*
* ����: ��
*
* ����ֵ: CRB3_OK - �����˳�
*      
********************************************************/
int CRB3Server::_ReceiveLoop(void)
{
	struct epoll_event events[8];
	time_t prev_time = time(NULL);
	time_t cur_time;

	memset(events, 0, sizeof(struct epoll_event) * 8);
	while(DgramSocket >= 0)
	{
		int nfds;
		int idx;
		int irc;

		// wait epoll event
		nfds = epoll_wait(EPoll, events, 8, 100);
		if(nfds == -1)
		{
			if(errno == EINTR)continue;
			break;
		}

		// clean alive node list
		if((cur_time = time(NULL)) - prev_time >= CRB3_ALIVE_CHECK)
		{
			prev_time = cur_time;
			_KeepAlive();
		}

		for(idx=0;idx<nfds;idx++)
		{
			// receive udp data packet (include status query packet)
			if(events[idx].data.fd == DgramSocket)
			{
				if(events[idx].events == EPOLLIN &&
					CRB3_OK != (irc = _ReceiveDatagram(DgramSocket)))
				{
				}
			}

			// receive tcp incoming connection
			else if(events[idx].data.fd == StreamSocket)
			{
				if(events[idx].events == EPOLLIN &&
					CRB3_OK != (irc = _ReceiveConnection(StreamSocket)))
				{
				}
			}

			// receive tcp data packet
			else
			{
				OnEvent(&events[idx]);
			}
		}
	}

	printf("[CRB3] receive thread exit.\n");
	return CRB3_OK;
}

/********************************************************
* ������: CRB3Server::_ProcessLoop
*
* ��������: ������ѭ��
*
* ����:
*      ThreadCtrl - (����) �󶨵Ĵ����߳���Ϣ
*
* ����ֵ: CRB3_OK - �����˳�
*      
********************************************************/
int CRB3Server::_ProcessLoop(IN CRB3_THREAD_CTRL * ThreadCtrl)
{
	int index = (int)(ThreadCtrl - ThreadTable);
	
	while(1)
	{
		CRB3_PACKET_NODE * packet;

		// pop packet node from queue
		if(NULL == (packet = _PacketPop(ThreadCtrl)))
		{
			// thread exit
			break;
		}

		// process & free packet node
		_PacketProcess(packet);
		_PacketFree(packet);
	}

	printf("[CRB3] process thread exit.\n");
	return CRB3_OK;
}

/********************************************************
* ������: CRB3Server::_UpdateAlive
*
* ��������: ���¿ͻ���keep_alive�ڵ�
*
* ����:
*      Addr - (����) �����µĿͻ��˵�ַ
*
* ����ֵ: keep_alive�ڵ�
********************************************************/
CRB3_ALIVE_NODE * CRB3Server::_UpdateAlive(IN CRB3_ADDRESS * Addr)
{
	CRB3_ALIVE_NODE * anode;

	// find existing node
	pthread_mutex_lock(&AliveLock);
	anode = (CRB3_ALIVE_NODE *)lstFirst(&AliveList);
	while(anode)
	{
		if(0 == CRB3AddressCompare(&anode->addr, Addr))
		{
			lstDelete(&AliveList, &anode->node);
			break;
		}
		anode = (CRB3_ALIVE_NODE *)lstNext(&anode->node);
	}

	if(anode == NULL)
	{
		// create new node
		anode = (CRB3_ALIVE_NODE *)malloc(sizeof(CRB3_ALIVE_NODE));
		assert(anode);
		anode->addr = *Addr;

		char ip[64];
		int port;
		CRB3Address2String(Addr, ip, 64, &port);
		LogMng->Printf(SEVERITY_NOTICE, "new alive client: %s,%d", ip, port);
	}

	// update node
	anode->last_notify = time(NULL);
	lstInsert(&AliveList, NULL, &anode->node);
	pthread_mutex_unlock(&AliveLock);

	return anode;
}

/********************************************************
* ������: CRB3Server::_KeepAlive
*
* ��������: ���ֿͻ��˽ڵ�
*
* ����: ��
*
* ����ֵ: ��
********************************************************/
void CRB3Server::_KeepAlive(void)
{
	CRB3_ALIVE_NODE * anode;
	CRB3_ALIVE_NODE * anode_prev;
	time_t curtime = time(NULL);
	time_t deltatime;

	pthread_mutex_lock(&AliveLock);
	anode = (CRB3_ALIVE_NODE *)lstLast(&AliveList);
	while(anode)
	{
		anode_prev = (CRB3_ALIVE_NODE *)lstPrevious(&anode->node);
		deltatime = curtime - anode->last_notify;
		
		if(deltatime >= CRB3_ALIVE_TIMEOUT)
		{
			char ip[64];
			int port;
			CRB3Address2String(&anode->addr, ip, 64, &port);
			LogMng->Printf(SEVERITY_NOTICE, "delete alive client: %s,%d", ip, port);
		
			lstDelete(&AliveList, &anode->node);
			Disconnect(&anode->addr);
			free(anode);
		}
		else if(deltatime >= CRB3_ALIVE_IDLE)
		{
			char ip[64];
			int port;
			CRB3Address2String(&anode->addr, ip, 64, &port);
			
			LogMng->Printf(SEVERITY_NOTICE, "send alive to client: %s,%d", ip, port);
			_SendAliveNotify(&anode->addr);
		}
		anode = anode_prev;
	}
	pthread_mutex_unlock(&AliveLock);
}

/********************************************************
* ������: CRB3Server::PacketSink
*
* ��������: �����Ľ���
*
* ����:
*      PacketBuf  - (����) ������ı������ݻ���
*      PacketSize - (����) ������ı������ݳ��ȣ���λ: �ֽ�
*      FromAddr   - (����) ������Դ��ַ
*      FromSocket - (����) ������Դ�׽���
*      Spare      - (����) �û�����
*
* ����ֵ:
*      SOERR_OK   - ����ɹ�
*      SOERR_SOCKET_CLOSED - ����ʧ�ܣ���Ҫ�����Ͽ�����
********************************************************/
int CRB3Server::PacketSink(IN uint8_t * PacketBuf, IN int PacketSize, IN SO_ADDRESS * FromAddr, IN int FromSocket, IN void * Spare)
{
	CRB3_ALIVE_NODE * alive = (CRB3_ALIVE_NODE *)Spare;
	
	if(PacketSize > CRB3_MAX_PACKET_SIZE)
	{
		RxErrorLen++;
		return SOERR_SOCKET_CLOSED;
	}

	if(alive)
	{
		char ip[64];
		int port;
		CRB3Address2String(&alive->addr, ip, 64, &port);
		
		LogMng->Printf(SEVERITY_NOTICE, "recv alive from client: %s,%d", ip, port);
		alive->last_notify = time(NULL);
	}
	
	if(GetStatus() == CRB3_STATUS_WORKING)
	{
		int proc_id;
		CRB3_PACKET_NODE * packet;
		
		// try to alloc packet node
		if(NULL == (packet = _PacketAlloc()))
		{
			RxErrorBuf++;
			return SOERR_OK;
		}

		// fill packet node
		packet->from_addr = *FromAddr;
		packet->protocol = SOCK_STREAM;
		packet->packet_size = PacketSize;
		memcpy(packet->packet_buf, PacketBuf, PacketSize);
		
		// select process queue & push
		if(CRB3_OK != _PacketPush(packet, FromSocket))
		{
			_PacketFree(packet);
		}
	}
	return SOERR_OK;
}

/********************************************************
* ������: CRB3Server::CheckXON
*
* ��������: ��������Ƿ���Ҫ����
*
* ����:
*      FromAddr   - (����) ��������ַ
*      FromSocket - (����) �������׽���
*
* ����ֵ:
*      true  - ��������
*      false - ��������
********************************************************/
bool CRB3Server::CheckXON(IN SO_ADDRESS * FromAddr, IN int FromSocket)
{
	bool brc;
	
	pthread_mutex_lock(&CacheLock);
	if(PacketCache->pool[0].free >= CRB3_MAX_QUEUE_SIZE * 3 / 10)
	{
		brc = true;
	}
	else
	{
		brc = false;
	}
	pthread_mutex_unlock(&CacheLock);

	return brc;
}

/********************************************************
* ������: CRB3Server::CheckXOFF
*
* ��������: ��������Ƿ���Ҫ��ʼ
*
* ����:
*      FromAddr   - (����) ��������ַ
*      FromSocket - (����) �������׽���
*
* ����ֵ:
*      true  - ��ʼ����
*      false - ��������
********************************************************/
bool CRB3Server::CheckXOFF(IN SO_ADDRESS * FromAddr, IN int FromSocket)
{
	bool brc;
	
	pthread_mutex_lock(&CacheLock);
	if(PacketCache->pool[0].free <= CRB3_MAX_QUEUE_SIZE / 10)
	{
		brc = true;
	}
	else
	{
		brc = false;
	}
	pthread_mutex_unlock(&CacheLock);

	return brc;
}


/********************************************************
* ������: CRB3Server::ReceiveThread
*
* ��������: ���Ľ����߳�
*
* ����:
*      arg - (����) �󶨵Ĵ����߳���Ϣ
*
* ����ֵ: CRB3_OK - �����˳�
*      
********************************************************/
void * CRB3Server::ReceiveThread(IN void * arg)
{
	CRB3Server * server_ref = (CRB3Server *)arg;

	return (void *)(long)server_ref->_ReceiveLoop();
}

/********************************************************
* ������: CRB3Server::ProcessThread
*
* ��������: �������߳�
*
* ����:
*      arg - (����) �󶨵Ĵ����߳���Ϣ
*
* ����ֵ: CRB3_OK - �����˳�
*      
********************************************************/
void * CRB3Server::ProcessThread(IN void * arg)
{
	CRB3_THREAD_CTRL * thread_ctrl = (CRB3_THREAD_CTRL *)arg;

	return (void *)(long)thread_ctrl->ServerRef->_ProcessLoop(thread_ctrl);
}

#ifdef NAMESPACE
}
#endif
