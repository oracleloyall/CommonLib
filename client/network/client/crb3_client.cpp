/**********************************************************
* crb3_client.cpp
* Description: client module of common request broker v3 library source code
*	   
* Copyright 2013 miaozz
*
* Create : 24 Sep. 2013 , miaozz
*          changed from crb2:
*          1) support PKT-mode & API-Mode for the same instance at the same time
*          2) support master-slave service mode or balance service mode
*          3) replace tcp stream with udp datagram
*
* Modify :
*          10 Nov. 2015 , miaozz
*          crack bug for 'CRB3Client::CRSend' : invalid calculation of head length
*          crack bug for 'CRB3Client::CRB3Client' : missing call of 'SetLengthLocation'
*          31 Mar. 2016 , miaozz
*          discard control socket.
*          14 Jun. 2016 , miaozz
*          security coding: replace snprintf with seprintf
*          10 May. 2018 , miaozz
*          new client mode: Node Consistent Hashing (NCH)
*          22 May. 2018 , miaozz
*          support ipv6 for SOCK_DGRAM
***********************************************************/
#include <sys/types.h>
#include <sys/epoll.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include "Profile.h"
#include "crb3_param.h"
#include "crb3_client.h"

#ifdef NAMESPACE
namespace common{
#endif

#ifndef CRB3_CONCURRENT_DEFAULT
#define CRB3_CONCURRENT_DEFAULT		100
#endif

#ifndef CRB3_DIO_PORT_START_DEFAULT
#define CRB3_DIO_PORT_START_DEFAULT	18000
#endif

#define CRB3_ATOMIC32_INC(v)			__sync_fetch_and_add(&v, 1)
#define CRB3_ATOMIC32_DEC(v)			__sync_fetch_and_add(&v, -1)
#define CRB3_ATOMIC32_INC_AND_TEST(v)	__sync_add_and_fetch(&v, 1)
#define CRB3_ATOMIC32_DEC_AND_TEST(v)	__sync_sub_and_fetch(&v, 1)

#define LOCAL_FUNCTION
#ifdef LOCAL_FUNCTION
static int node_hash[CRB3_MAX_HASH_NODES];

static int genid(int x)
{
	static char id_map[CRB3_MAX_HASH_NODES];
	static int m = CRB3_MAX_HASH_NODES;
	static int n = 0;
	static int k = 0;
	int r;

	switch(k)
	{
	case 0:
		k++;
		id_map[0] = 1;
		return(0);

	case 1:
		n++;
		if(n < x)
		{
			r = (n * m) / x;
			id_map[r] = 1;
			return r;
		}
		k++;
		n = -1;

	default:
		n += 2;
		if(n >= x * k)
		{
			k = k * 2;
			n = 1;
		}
		r = (n * m) / (x * k);
		while(id_map[r] && (r < m - 1))
		{
			r++;
		}
		if(id_map[r])
		{
			return(-1);
		}
		id_map[r] = 1;
		return r;
	}
}

static char * get_cfg_string(IN PROFILE * cfg,
				IN PF_SECTION * sec, IN PF_SECTION * sec_def,
				IN const char * record_name)
{
	char * str = NULL;
	
	if(!str && sec)
	{
		str = cfg->GetRecordString(sec, record_name);
	}
	if(!str && sec_def)
	{
		str = cfg->GetRecordString(sec_def, record_name);
	}
	return str;
}

static bool get_cfg_integer(IN PROFILE * cfg,
				IN PF_SECTION * sec, IN PF_SECTION * sec_def,
				IN const char * record_name,
				OUT int * value)
{
	bool brc = false;
	
	if(!brc && sec)
	{
		brc = cfg->GetRecordInteger(sec, record_name, value);
	}
	if(!brc && sec_def)
	{
		brc = cfg->GetRecordInteger(sec_def, record_name, value);
	}
	return brc;
}
#endif

/********************************************************
* 函数名: CRB3Client::CRB3Client
*
* 功能描述: 初始化CRB3客户端
*
* 参数:
*      Mode - (输入) 服务模式:
*                    0 - 主备模式。该模式下总是选择处于工作状态下的第一个服务端发送请求。
*                    1 - 均衡模式。该模式下轮流选择处于工作状态下的服务端发送请求。
*                    2 - 一致性哈希模式。该模式下根据请求的一致性哈希选择处于工作状态下的服务端发送请求。
*      Name - (输入) 客户端名称，NULL表示使用缺省名称"default"
*
* 返回值: 无
*      
********************************************************/
CRB3Client::CRB3Client(IN int Mode, IN char * Name)
			:SoStream()
{
	unsigned long crl_size;
	char format[CRB3_MAX_CLIENT_NAME_SIZE + 32];

	// initialize client name
	if(Name)
	{
		snprintf(ClientName, sizeof(ClientName), "%s", Name);
	}
	else
	{
		strcpy(ClientName, "default");
	}
	snprintf(format, sizeof(format), "[%%t - %s] %%y: %%s", ClientName);
	
	// initialize log manager
	LogMng = new LogManager(0, LOGFLAG_KEEP_THREADSAFE);
	LogMng->SetLogFile("./crb.%Y%m%d");
	LogMng->SetLogFormat(format, SEVERITY_ALL);
	LogMng->SetSeverityMask(SEVERITY_MASK_ALL);

	// initialize server connection list
	pthread_rwlock_init(&ConnectLocker, NULL);
	ReceiveThid = (pthread_t)-1;
	lstInit(&WorkingList);
	lstInit(&BackupList);
	lstInit(&ExceptionList);
	ServiceMode = Mode;
	EPoll = -1;
	DgramSocket = -1;
	DgramSocket6 = -1;
	MaxConcurrent = CRB3_CONCURRENT_DEFAULT;
	NotifyTimeout = CRB3_CHECK_NOTIFY_TIMEOUT;
	NotifyGap = CRB3_CHECK_CONNECT_GAP;
	SetLengthLocation(0, 2, 0);
	DioEnable = false;
	DioSequence = 0;
	DioPortStart = CRB3_DIO_PORT_START_DEFAULT;
	lstInit(&DioList);
	pthread_mutex_init(&DioMutex, NULL);

	// initialize consistent hashing parameters
	_NCHInit();

	// initialize Rsp-Queue
	queInit(&RspQueue, sizeof(CRB3_RSP) * CRB3_MAX_CLIENT_RSP_SIZE, QUEUE_FIFO, NULL);
	pthread_mutex_init(&RspMutex, NULL);
	pthread_cond_init(&RspCond, NULL);

	// initialize Check-Queue
	queInit(&CheckQueue, sizeof(CRB3_STATUS_RSP) * 100, QUEUE_FIFO, NULL);
	pthread_mutex_init(&CheckMutex, NULL);
	pthread_cond_init(&CheckCond, NULL);
	CheckThid = (pthread_t)-1;
	CheckExit = false;
	Running = false;

	// initialize statistics
	PktRxSucceed = 0;
	PktRxDiscard = 0;
	ReqQps = NULL;
}

/********************************************************
* 函数名: CRB3Client::CRB3Client
*
* 功能描述: 销毁CRB3客户端
*
* 参数: 无
*
* 返回值: 无
*      
********************************************************/
CRB3Client::~CRB3Client()
{
	if(Running)
	{
		Stop();
	}
	if(ReqQps)
	{
		delete ReqQps;
		ReqQps = NULL;
	}
	queFree(&RspQueue);
	queFree(&CheckQueue);
	pthread_rwlock_destroy(&ConnectLocker);
	pthread_mutex_destroy(&RspMutex);
	pthread_cond_destroy(&RspCond);
	pthread_mutex_destroy(&CheckMutex);
	pthread_cond_destroy(&CheckCond);
	_NCHFree();
	delete LogMng;
}

/********************************************************
* 函数名: CRB3Client::ConfigFromFile
*
* 功能描述: 从配置文件中加载配置
*
* 参数:
*      CfgFile - (输入) 配置文件名
*
* 返回值:
*      CRB3_OK - 操作成功
*      CRB3_NOT_FIND - 未找到对应的配置
*      
********************************************************/
int CRB3Client::ConfigFromFile(IN char * CfgFile)
{
	PROFILE cfg;

	if(cfg.Load(CfgFile))
	{
		PF_SECTION * section_default;
		PF_SECTION * section;
		char sec_name[80];
		char * str;
		int val;

		snprintf(sec_name, sizeof(sec_name), "%s", ClientName);
		section = cfg.GetSection(sec_name);
		section_default = cfg.GetSection("DEFAULT");

		// configure log
		if(NULL != (str = get_cfg_string(&cfg, section, section_default, "log_output")))
		{
			LogMng->SetLogFile(str);
		}
		if(true == get_cfg_integer(&cfg, section, section_default, "log_level", &val))
		{
			LogMng->SetSeverityLevel(val);
		}
		
		// configure perference
		if(true == get_cfg_integer(&cfg, section, section_default, "concurrent_num", &val))
		{
			ConfigMaxConcurrent(val);
		}
		
		// configure notify
		if(true == get_cfg_integer(&cfg, section, section_default, "notify_gap", &val))
		{
			NotifyGap = val;
		}
		if(true == get_cfg_integer(&cfg, section, section_default, "notify_timeout", &val))
		{
			NotifyTimeout = val;
		}

		// configure consistent hashing
		int init_nodes = 0;
		if(true == get_cfg_integer(&cfg, section, section_default, "nch_init_nodes", &val))
		{
			init_nodes = val;
		}
		if(init_nodes <= 0 && NULL != (str = get_cfg_string(&cfg, section, section_default, "connect_list")))
		{
			int token_num = 1;
			char * token = strchr(str, '|');
			while(token)
			{
				++token_num;
				token = strchr(token+1, '|');
			}
			init_nodes = token_num;
		}
		_NCHLoad(init_nodes);
		if(NULL != (str = get_cfg_string(&cfg, section, section_default, "nch_alg")))
		{
			_NCHAlgConfig(str);
		}

		// load connection configure
		if(NULL != (str = get_cfg_string(&cfg, section, section_default, "connect_list")))
		{
			char buf[1024];
			char * token;
			char * nexttoken;

			memset(buf, 0, 1024);
			strncpy(buf, str, 1024);
			token = strtok_r(buf, "|", &nexttoken);
			while(token)
			{
				char * subtoken;
				char * str_ip = strtok_r(token, ",", &subtoken);
				char * str_port = strtok_r(NULL, ",", &subtoken);
				char * str_protocol = strtok_r(NULL, ",", &subtoken);
				int protocol = SOCK_STREAM;
				int port;
				int irc;

				if(str_protocol && 0 == strcasecmp(str_protocol, "udp"))
				{
					protocol = SOCK_DGRAM;
				}
				if(str_ip && str_port)
				{
					port = atoi(str_port);
					AddConnect(str_ip, port, protocol, NULL);
				}
				else
				{
					LogMng->Printf(SEVERITY_EMERG, "[CRB] syntax error of connect_list configure");
				}
				token = strtok_r(NULL, "|", &nexttoken);
			}
		}

		// load DIO configure
		if(true == get_cfg_integer(&cfg, section, section_default, "dio_enable", &val))
		{
			DioEnable = (val == 0 ? false : true);
		}
		if(true == get_cfg_integer(&cfg, section, section_default, "dio_port_start", &val))
		{
			DioPortStart = val;
		}
		return CRB3_OK;
	}
	return CRB3_NOT_FIND;
}

/********************************************************
* 函数名: CRB3Client::ConfigLogPath
*
* 功能描述: 配置日志路径及名称
*
* 参数:
*      File - (输入) 日志文件名，包含路径
*
* 返回值:
*      CRB3_OK - 操作成功
*      CRB3_INVALID_PARAMETER - 无效的参数
*
********************************************************/
int CRB3Client::ConfigLogPath(IN char * File)
{
	if(File)
	{
		LogMng->SetLogFile(File);
		return CRB3_OK;
	}
	return CRB3_INVALID_PARAMETER;
}

/********************************************************
* 函数名: CRB3Client::ConfigRatePath
*
* 功能描述: 配置速率统计日志路径
*
* 参数:
*      File - (输入) 速率统计日志文件名及路径，NULL表示不输出统计信息
*
* 返回值:
*      CRB3_OK - 操作成功
*      
********************************************************/
int CRB3Client::ConfigRatePath(IN char * File)
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
* 函数名: CRB3Client::ConfigMaxConcurrent
*
* 功能描述: 配置最大并发请求数目(缺省值由MAX_CLIENT_CRL_SIZE定义)
*
* 参数:
*      Concurrent - (输入) 待配置的最大并发请求数目，0或负值表示不限制
*
* 返回值:
*      CRB3_OK - 操作成功
*      CRB3_FAILED  - 操作失败: CRB3Client已启动，无法改变该配置
*      
********************************************************/
int CRB3Client::ConfigMaxConcurrent(IN int Concurrent)
{
	MaxConcurrent = Concurrent;
	return CrlMgr.ConfigMaxCRLSize(Concurrent * 2);
}

/********************************************************
* 函数名: CRB3Client::ConfigNotify
*
* 功能描述: 配置通告消息的超时时间和发送间隔时间
*
* 参数:
*      TimeoutSec - (输入) 通告消息的超时时间，单位: 秒
*      GapSec     - (输入) 通告消息的发送间隔，单位: 秒
*
* 返回值:
*      CRB3_OK - 操作成功
********************************************************/
int CRB3Client::ConfigNotify(IN int TimeoutSec, IN int GapSec)
{
	NotifyTimeout = TimeoutSec;
	NotifyGap = GapSec;
	return CRB3_OK;
}

/********************************************************
* 函数名: CRB3Client::ConfigDio
*
* 功能描述: 配置Direct IO模式
*
* 参数:
*      IsEnable  - (输入) 是否打开direct io模式
*      PortStart - (输入) direct io端口范围初始值，0表示使用缺省值
*
* 返回值:
*      CRB3_OK - 操作成功
********************************************************/
int CRB3Client::ConfigDio(IN bool IsEnable, IN int PortStart)
{
	if(IsEnable)
	{
		DioEnable = true;
		if(PortStart > 0)
		{
			DioPortStart = PortStart;
		}
	}
	else
	{
		DioEnable = false;
	}
	return CRB3_OK;
}

/********************************************************
* 函数名: CRB3Client::CustomHashAlg
*
* 功能描述: 自定义一致性哈希算法
*
* 参数:
*      CRId  - (输入) 算法对应的请求ID
*      Func  - (输入) 算法实现函数
*      Spare - (输入) 算法实现函数的额外参数
*
* 返回值:
*      CRB3_OK - 操作成功
********************************************************/
int CRB3Client::CustomHashAlg(IN uint32_t CRId, IN NCH_ALG_HASH_FUNC Func, IN uint64_t Spare)
{
	return _NCHAlgSetCustom(CRId, Func, Spare);
}

/********************************************************
* 函数名: CRB3Client::CustomHashAlg
*
* 功能描述: 自定义默认的一致性哈希算法
*
* 参数:
*      Func  - (输入) 算法实现函数
*      Spare - (输入) 算法实现函数的额外参数
*
* 返回值:
*      CRB3_OK - 操作成功
********************************************************/
int CRB3Client::CustomHashAlgDefault(IN NCH_ALG_HASH_FUNC Func, IN uint64_t Spare)
{
	NCHAlgDef.type = ALG_TYPE_CUSTOM;
	NCHAlgDef.value = Spare;
	NCHAlgDef.func = Func;
	return CRB3_OK;
}

/********************************************************
* 函数名: CRB3Client::AddConnect
*
* 功能描述: 添加服务端节点，包含IP地址和服务端口
*
* 参数:
*      IpAddress   - (输入) 待添加的服务端的IP地址，支持IPv4和IPv6地址
*      Port        - (输入) 待添加的服务端的服务端口
*      Protocol    - (输入) 使用的协议(SOCK_DGRAM or SOCK_STREAM)
*      RateLogPath - (输入) 速率统计输出文件，NULL表示不统计速率
*
* 返回值:
*      CRB3_OK - 操作成功
*      CRB3_EMPTY_MEMORY      - 内存不足
*      CRB3_DUPLICATE         - 该服务端节点已存在
*      CRB3_INVALID_PARAMETER - 参数不正确
*
********************************************************/
int CRB3Client::AddConnect(IN char * IpAddress, IN int Port, IN int Protocol, IN char * RateLogPath)
{
	static int sequence = 0;
	CRB3_CONNECTION * cnode;
	CRB3_CONNECTION * cnode_new;

	// check parameter
	if(Protocol == SOCK_STREAM && DioEnable)
	{
		LogMng->Printf(SEVERITY_ALERT, "can not use stream protocol in direct-io mode");
		return CRB3_INVALID_PARAMETER;
	}

	// alloc connect node
	cnode_new = (CRB3_CONNECTION *)malloc(sizeof(CRB3_CONNECTION));
	if(cnode_new == NULL)
	{
		LogMng->Printf(SEVERITY_ALERT, "add connector '%s:%d' error: memory alloc failed", IpAddress, Port);
		return CRB3_EMPTY_MEMORY;
	}
	CRB3String2Address(IpAddress, Port, &cnode_new->ServerAddr);
	cnode_new->Status = CRB3_STATUS_EXCEPTION;
	cnode_new->Flag = 0;
	cnode_new->Protocol = Protocol;
	cnode_new->Pending = 0;
	cnode_new->PendMax = MaxConcurrent;
	cnode_new->Reference = 0;
	cnode_new->ToDelete = 0;
	cnode_new->Hash = _NCHGet(sequence++);
	if(cnode_new->Hash >= 0)
	{
		cnode_new->node_nch.int_key = (unsigned long)cnode_new->Hash;
		irbtInsert(&NCHTree, &cnode_new->node_nch);
	}
	cnode_new->RefList = NULL;
	cnode_new->LastNotifyTime = 0;
	cnode_new->Rate = new RateInfo;
	cnode_new->Delay = new DelayInfo(100);
	cnode_new->Rate->AddRateCalc(1, 2, RateLogPath);

	// add to exception list
	pthread_rwlock_wrlock(&ConnectLocker);
	cnode = _FindConnect(IpAddress, Port, NULL);
	if(cnode)
	{
		pthread_rwlock_unlock(&ConnectLocker);
		free(cnode_new);
		LogMng->Printf(SEVERITY_ALERT, "add connector '%s:%d' error: duplicate address", IpAddress, Port);
		return CRB3_DUPLICATE;
	}
	lstInsert(&ExceptionList, NULL, (NODE *)cnode_new);
	pthread_rwlock_unlock(&ConnectLocker);
	
	LogMng->Printf(SEVERITY_NOTICE, "add connector '%s:%d' succeed", IpAddress, Port);
	return CRB3_OK;
}

/********************************************************
* 函数名: CRB3Client::DelConnect
*
* 功能描述: 删除服务端节点
*
* 参数:
*      IpAddress - (输入) 待删除的服务端的IP地址，支持IPv4和IPv6地址
*      Port      - (输入) 待删除的服务端的服务端口
*
* 返回值:
*      CRB3_OK - 操作成功
*      CRB3_NOT_FIND - 该服务端节点不存在
*      
********************************************************/
int CRB3Client::DelConnect(IN char * IpAddress, IN int Port)
{
	CRB3_CONNECTION * cnode;
	LIST * FromList;
	
	pthread_rwlock_wrlock(&ConnectLocker);
	cnode = _FindConnect(IpAddress, Port, &FromList);
	if(cnode == NULL)
	{
		pthread_rwlock_unlock(&ConnectLocker);
		LogMng->Printf(SEVERITY_ALERT, "delete connector '%s:%d' error: not find", IpAddress, Port);
		return CRB3_NOT_FIND;
	}
	if(cnode->Protocol == SOCK_STREAM)
	{
		Disconnect(&cnode->ServerAddr);
	}
	if(cnode->Reference == 0)
	{
		if(cnode->Rate)delete cnode->Rate;
		delete cnode->Delay;
		lstDelete(FromList, (NODE *)cnode);
		free(cnode);
	}
	else
	{
		cnode->ToDelete = 1;
	}
	pthread_rwlock_unlock(&ConnectLocker);
	
	LogMng->Printf(SEVERITY_NOTICE, "delete connector '%s:%d' succeed", IpAddress, Port);
	return CRB3_OK;
}

/********************************************************
* 函数名: CRB3Client::Start
*
* 功能描述: 启动CRB3客户端
*
* 参数: 无
*
* 返回值:
*      CRB3_OK - 操作成功
*      CRB3_DUPLICATE     - 已经启动
*      CRB3_NOT_CONNECTED - 创建套接字失败
*      CRB3_FAILED        - CRLMap启动失败
*      CRB3_EMPTY_MEMORY  - 内存不足
********************************************************/
int CRB3Client::Start(void)
{
	struct epoll_event ev;
	
	if(Running)
	{
		return CRB3_DUPLICATE;
	}

	// create epoll & configure stream
	EPoll = epoll_create(8);
	if(EPoll < 0)
	{
		return CRB3_NOT_CONNECTED;
	}
	SetEPollFd(EPoll);

	// create datagram socket
	DgramSocket = socket(AF_INET, SOCK_DGRAM, 0);
	DgramSocket6 = socket(AF_INET6, SOCK_DGRAM, 0);
	if(DgramSocket == -1 && DgramSocket6 == -1)
	{
		close(EPoll);
		EPoll = -1;
		return CRB3_NOT_CONNECTED;
	}
	if(DgramSocket >= 0)
	{
		ev.events = EPOLLIN;
		ev.data.fd = DgramSocket;
		if(-1 == epoll_ctl(EPoll, EPOLL_CTL_ADD, DgramSocket, &ev))
		{
			close(EPoll);
			EPoll = -1;
			close(DgramSocket);
			DgramSocket = -1;
			return CRB3_NOT_CONNECTED;
		}
	}
	if(DgramSocket6 >= 0)
	{
		ev.events = EPOLLIN;
		ev.data.fd = DgramSocket6;
		if(-1 == epoll_ctl(EPoll, EPOLL_CTL_ADD, DgramSocket6, &ev))
		{
			close(EPoll);
			EPoll = -1;
			close(DgramSocket6);
			DgramSocket6 = -1;
			return CRB3_NOT_CONNECTED;
		}
	}
	
	// create dio socket
	if(DioEnable)
	{
		int idx;
		
		for(idx=0;idx<MaxConcurrent;idx++)
		{
			CRB3_DIO_NODE * dnode = (CRB3_DIO_NODE *)malloc(sizeof(CRB3_DIO_NODE));
			
			dnode->Socket = socket(AF_INET, SOCK_DGRAM, 0);
			if(dnode->Socket == -1)
			{
				free(dnode);
				LogMng->Printf(SEVERITY_NOTICE, "create dio socket[%d] failed", idx);
				break;
			}
			dnode->Port = DioPortStart + idx;

			struct sockaddr_in dio_addr;
			memset(&dio_addr, 0, sizeof(dio_addr));
			dio_addr.sin_family = AF_INET;
			dio_addr.sin_port = htons(dnode->Port);
			if(-1 == bind(dnode->Socket, (struct sockaddr *)&dio_addr, sizeof(dio_addr)))
			{
				close(dnode->Socket);
				free(dnode);
				LogMng->Printf(SEVERITY_NOTICE, "bind dio socket[%d] port %d failed", idx, DioPortStart + idx);
				break;
			}

			lstAdd(&DioList, &dnode->node);
		}

		if(idx < MaxConcurrent)
		{
			// free dio list
			CRB3_DIO_NODE * dnode;
			while(NULL != (dnode = (CRB3_DIO_NODE *)lstGet(&DioList)))
			{
				close(dnode->Socket);
				free(dnode);
			}
			
			close(EPoll);
			EPoll = -1;
			if(DgramSocket >= 0)
			{
				close(DgramSocket);
				DgramSocket = -1;
			}
			if(DgramSocket6 >= 0)
			{
				close(DgramSocket6);
				DgramSocket6 = -1;
			}
			return CRB3_FAILED;
		}
	}
	else
	{
		// start CRLMap
		if(CRB3_OK != CrlMgr.Start())
		{
			close(EPoll);
			EPoll = -1;
			if(DgramSocket >= 0)
			{
				close(DgramSocket);
				DgramSocket = -1;
			}
			if(DgramSocket6 >= 0)
			{
				close(DgramSocket6);
				DgramSocket6 = -1;
			}
			return CRB3_FAILED;
		}
	}

	// create thread
	pthread_create(&ReceiveThid, NULL, ReceiveThread, this);
	pthread_create(&CheckThid, NULL, CheckThread, this);
	Running = true;
	
	return CRB3_OK;
}

/********************************************************
* 函数名: CRB3Client::Stop
*
* 功能描述: 停止CRB3客户端。
*           该操作会中断当前的同步接口调用，等待调用结束，然后返回。
*
* 参数: 无
*
* 返回值:
*      CRB3_OK - 操作成功
*      
********************************************************/
int CRB3Client::Stop(void)
{
	void * thrc;
	CRB3_RSP rsp_entry;

	// destroy check thread
	if(CheckThid != (pthread_t)-1)
	{
		pthread_mutex_lock(&CheckMutex);
		CheckExit = true;
		pthread_cond_signal(&CheckCond);
		pthread_mutex_unlock(&CheckMutex);
		pthread_join(CheckThid, &thrc);
		CheckThid = (pthread_t)-1;
	}

	// destroy dio socket
	if(DioEnable)
	{
		CRB3_DIO_NODE * dnode;
		while(NULL != (dnode = (CRB3_DIO_NODE *)lstGet(&DioList)))
		{
			close(dnode->Socket);
			free(dnode);
		}
	}

	// destroy datagram socket
	if(DgramSocket >= 0)
	{
		struct epoll_event ev;

		epoll_ctl(EPoll, EPOLL_CTL_DEL, DgramSocket, &ev);
		close(DgramSocket);
		DgramSocket = -1;
	}
	if(DgramSocket6 >= 0)
	{
		struct epoll_event ev;

		epoll_ctl(EPoll, EPOLL_CTL_DEL, DgramSocket6, &ev);
		close(DgramSocket6);
		DgramSocket6 = -1;
	}

	// destroy stream socket
	DisconnectAll();

	// destroy epoll
	if(EPoll >= 0)
	{
		close(EPoll);
		EPoll = -1;
	}
	
	// destroy receive thread
	if(ReceiveThid != (pthread_t)-1)
	{
		pthread_join(ReceiveThid, &thrc);
		ReceiveThid = (pthread_t)-1;
	}

	// destroy connector list
	pthread_rwlock_wrlock(&ConnectLocker);
	lstFree(&WorkingList);
	lstFree(&BackupList);
	lstFree(&ExceptionList);
	pthread_rwlock_unlock(&ConnectLocker);

	// delete entry from Rsp-Queue
	pthread_mutex_lock(&RspMutex);
	while(0 != quePop(&RspQueue, &rsp_entry, sizeof(CRB3_RSP)))
	{
		if(rsp_entry.ParamOut)
		{
			delete rsp_entry.ParamOut;
		}
	}
	pthread_mutex_unlock(&RspMutex);

	if(!DioEnable)
	{
		// stop CRLMap
		CrlMgr.Stop();
	}

	Running = false;
	LogMng->Printf(SEVERITY_NOTICE, "stop crb client succeed");
	return CRB3_OK;
}

int CRB3Client::RequestDio(IN uint32_t CRId, IN CRB3Param * ParamIn, IN int TimeoutMS, OUT CRB3Param ** ParamOut)
{
	uint8_t packet_buf[CRB3_MAX_PACKET_SIZE];
	uint8_t * ptr = packet_buf;
	int tlv_len = ParamIn->GetTLVLength();
	int buf_size = tlv_len + 8 + CRB3_PARAM_RESERVE_LEN;
	uint32_t psize = (uint32_t)tlv_len;
	uint32_t sequence;
	CRB3_DIO_NODE * dnode;

	// check buffer size
	if(tlv_len + CRB3_PARAM_RESERVE_LEN > CRB3_MAX_PACKET_SIZE)
	{
		return CRB3_INVALID_PARAMETER;
	}

	// fetch dio index
	pthread_mutex_lock(&DioMutex);
	sequence = DioSequence++;
	dnode = (CRB3_DIO_NODE *)lstGet(&DioList);
	if(!dnode)
	{
		pthread_mutex_unlock(&DioMutex);
		return CRB3_EMPTY_QUEUE;
	}
	pthread_mutex_unlock(&DioMutex);

	// fill parameter buffer header
	CRB3_SUBMIT_INT16_A(ptr, buf_size);
	CRB3_SUBMIT_INT16_A(ptr, CRB3_PKT_CR_REQ);
	CRB3_SUBMIT_INT32_A(ptr, CRB3_OK);
	CRB3_SUBMIT_INT16_A(ptr, 1);							// ReqNum
	CRB3_SUBMIT_INT16_A(ptr, 0);							// ReqFlag
	CRB3_SUBMIT_INT32_A(ptr, sequence);						// CR-Seq
	CRB3_SUBMIT_INT32_A(ptr, CRId);							// CR-Id
	CRB3_SUBMIT_INT16_A(ptr, 0);							// CR-Flag: API-Mode
	CRB3_SUBMIT_INT16_A(ptr, (int16_t)ParamIn->ParamCount);	// PNum
	CRB3_SUBMIT_INT32_A(ptr, psize);						// PSize

	// fill parameter buffer body
	ParamIn->OutputTLV((char *)ptr);

	// calc timeout value
	struct timespec ts;
	struct timespec cur_ts;
	struct timespec * ts_ptr;
	ts_ptr = _InitTimeSpec(TimeoutMS, &ts);
	do
	{
		CRB3_CONNECTION * cnode;

		cnode = _GetConnect(CRId, ParamIn);
		if(cnode)
		{
			int irc;

			irc = sendto(dnode->Socket, packet_buf, buf_size, MSG_DONTWAIT,
						&cnode->ServerAddr.sa, sizeof(struct sockaddr_in));
			if(irc > 0)
			{
				DelayStat ds;
				struct timeval tv;
				fd_set rdset;
				int max_fd = 0;
				int rc = 0;
				
				if(ReqQps)
				{
					ReqQps->Pulse(1);
				}
				if(cnode->Rate)
				{
					cnode->Rate->Pulse(1);
				}
				cnode->Delay->Begin(ds);

				irc = CRB3_FAILED;
				FD_ZERO(&rdset);
				FD_SET(dnode->Socket, &rdset);
				tv.tv_sec = TimeoutMS / 1000;
				tv.tv_usec = (TimeoutMS % 1000) * 1000;
				rc = select(dnode->Socket+1, &rdset, NULL, NULL, &tv);
				if(FD_ISSET(dnode->Socket, &rdset))
				{
					struct sockaddr_in server_addr;
					socklen_t addrlen = sizeof(server_addr);
					int packet_size = recvfrom(dnode->Socket, packet_buf, CRB3_MAX_PACKET_SIZE, 0,
										(struct sockaddr *)&server_addr, &addrlen);
					if(packet_size > 0)
					{
						irc = _ProcessDioResponse((char *)packet_buf, packet_size, ParamOut);
					}
				}
				cnode->Delay->End(ds);
				_PutConnect(cnode);

				pthread_mutex_lock(&DioMutex);
				lstAdd(&DioList, &dnode->node);
				pthread_mutex_unlock(&DioMutex);
				return irc;
			}
			_PutConnect(cnode);
		}

		// sleep & retry if send failed
		if(ts_ptr)
		{
			usleep(1000);
			clock_gettime(CLOCK_REALTIME, &cur_ts);
		}
	}while(ts_ptr && (cur_ts.tv_sec < ts_ptr->tv_sec ||
			(cur_ts.tv_sec == ts_ptr->tv_sec && cur_ts.tv_nsec < ts_ptr->tv_nsec)));

	pthread_mutex_lock(&DioMutex);
	lstAdd(&DioList, &dnode->node);
	pthread_mutex_unlock(&DioMutex);
	return CRB3_TIMEOUT;
}

/********************************************************
* 函数名: CRB3Client::Request
*
* 功能描述: 发送请求并等待结果(API模式/同步接口)
*
* 参数:
*      CRId      - (输入) 请求ID
*      ParamIn   - (输入) 请求参数列表
*      TimeoutMS - (输入) 等待结果超时时间，单位: 毫秒，0表示不等待，负值表示一直等待
*      ParamOut  - (输出) 请求结果对象指针(CRB3Param *)，调用者使用完成后通过delete操作符释放
*
* 返回值:
*      CRB3_OK - 操作成功
*      CRB3_TIMEOUT - 等待超时
*      CRB3_FAILED  - 操作取消或CRL内部错误
*      
********************************************************/
int CRB3Client::Request(IN uint32_t CRId, IN CRB3Param * ParamIn, IN int TimeoutMS, OUT CRB3Param ** ParamOut)
{
	if(DioEnable)
	{
		return RequestDio(CRId, ParamIn, TimeoutMS, ParamOut);
	}
	
	uint32_t sequence;
	uint8_t packet_buf[CRB3_MAX_PACKET_SIZE];
	int buf_size = CRB3_MAX_PACKET_SIZE - 8;
	CRB3_ADDRESS addr;
	uint8_t * ptr;
	struct timespec ts;
	struct timespec cur_ts;
	struct timespec * ts_ptr;

	// calc timeout value
	ts_ptr = _InitTimeSpec(TimeoutMS, &ts);

	// add crl-entry & fill packet body
	if(CRB3_OK != CrlMgr.AddCRLSync(CRId, ParamIn, &sequence, packet_buf + 8, &buf_size))
	{
		LogMng->Printf(SEVERITY_EMERG, "request error: crl-table full");
		return CRB3_FAILED;
	}

	// fill packet header
	ptr = packet_buf;
	CRB3_SUBMIT_INT16_A(ptr, (int16_t)(8 + buf_size));
	CRB3_SUBMIT_INT16_A(ptr, CRB3_PKT_CR_REQ);
	CRB3_SUBMIT_INT32_A(ptr, CRB3_OK);

	// select connection & send packet
	do
	{
		CRB3_CONNECTION * cnode;

		cnode = _GetConnect(CRId, ParamIn);
		if(cnode)
		{
			int irc;
			
			if(CRB3_OK == _CRSend(&cnode->ServerAddr, cnode->Protocol, packet_buf, 8 + buf_size))
			{
				DelayStat ds;

				if(ReqQps)
				{
					ReqQps->Pulse(1);
				}
				if(cnode->Rate)
				{
					cnode->Rate->Pulse(1);
				}
				cnode->Delay->Begin(ds);
				irc = CrlMgr.WaitCRL(sequence, ts_ptr, ParamOut);
				cnode->Delay->End(ds);
				_PutConnect(cnode);
				return irc;
			}
			_PutConnect(cnode);
		}

		// sleep & retry if send failed
		if(ts_ptr)
		{
			usleep(1000);
			clock_gettime(CLOCK_REALTIME, &cur_ts);
		}
	}while(ts_ptr && (cur_ts.tv_sec < ts_ptr->tv_sec ||
			(cur_ts.tv_sec == ts_ptr->tv_sec && cur_ts.tv_nsec < ts_ptr->tv_nsec)));

	CrlMgr.DelCRL(sequence);
	return CRB3_TIMEOUT;
}

/********************************************************
* 函数名: CRB3Client::RequestAsync
*
* 功能描述: 发送请求，请求结果通过回调函数返回(API模式/异步接口)
*           如果以NULL作为CRB3Param参数回调，则表示超时。
*           结果回调和超时回调可能同时发生，但是结果回调和超时回调各自本身是序列化的。
*           CRB3Param参数必须由用户在回调函数内通过delete操作符删除。
*
* 参数:
*      CRId      - (输入) 请求ID
*      ParamIn   - (输入) 请求参数列表
*      CbFunc    - (输入) 回调函数指针
*      CbArg     - (输入) 回调函数参数
*      TimeoutMS - (输入) 等待结果超时时间，单位: 毫秒，0或负值表示一直等待
*
* 返回值:
*      CRB3_OK - 操作成功
*      CRB3_FAILED          - CRL内部错误
*      CRB3_NOT_SERVICE     - 未找到可用的服务端
*      SOERR_PEND           - 发送失败，Pend队列中的数据在等待(仅TCP)
*      SOERR_INVALID_CLIENT - 发送失败，无效的客户端地址(仅TCP)
*      SOERR_SOCKET_ERROR   - 发送失败，套接字错误，具体信息查看errno
*      
********************************************************/
int CRB3Client::RequestAsync(IN uint32_t CRId, IN CRB3Param * ParamIn, IN CRB3_CLIENT_CALLBACK CbFunc, IN void * CbArg, IN int TimeoutMS)
{
	uint32_t sequence;
	uint8_t packet_buf[CRB3_MAX_PACKET_SIZE];
	int buf_size = CRB3_MAX_PACKET_SIZE - 8;
	CRB3_ADDRESS addr;
	uint8_t * ptr;
	int protocol;
	int irc;

	// add crl-entry
	if(CRB3_OK != CrlMgr.AddCRLAsync(CRId, ParamIn, CbFunc, CbArg, TimeoutMS, &sequence, packet_buf + 8, &buf_size))
	{
		LogMng->Printf(SEVERITY_EMERG, "request-async error: crl-table full");
		return CRB3_FAILED;
	}

	// fill packet header
	ptr = packet_buf;
	CRB3_SUBMIT_INT16_A(ptr, (int16_t)(8 + buf_size));
	CRB3_SUBMIT_INT16_A(ptr, CRB3_PKT_CR_REQ);
	CRB3_SUBMIT_INT32_A(ptr, CRB3_OK);

	// select connection
	if(CRB3_OK != _SelectConnect(CRId, ParamIn, &addr, &protocol))
	{
		CrlMgr.DelCRL(sequence);
		return CRB3_NOT_SERVICE;
	}

	// try to send request
	if(CRB3_OK != (irc = _CRSend(&addr, protocol, packet_buf, 8 + buf_size)))
	{
		CrlMgr.DelCRL(sequence);
		return irc;
	}
	return CRB3_OK;
}

/********************************************************
* 函数名: CRB3Client::CRSend
*
* 功能描述: 发送请求(PKT模式)
*
* 参数:
*      CRId      - (输入) 请求ID
*      CRSeq     - (输入) 请求顺序号
*      ParamIn   - (输入) 请求参数列表
*      TimeoutMS - (输入) 等待发送超时时间，单位: 毫秒，0表示不等待，负值表示一直等待
*
* 返回值:
*      CRB3_OK - 发送成功
*      CRB3_TIMEOUT            - 发送等待超时或取消
*      CRB3_INVALID_PARAMETER  - 无效的请求参数
*      
********************************************************/
int CRB3Client::CRSend(IN uint32_t CRId, IN uint32_t CRSeq, IN CRB3Param * ParamIn, IN int TimeoutMS)
{
	uint8_t pkt_buf[CRB3_MAX_PACKET_SIZE];
	int tlv_len = ParamIn->GetTLVLength();
	int pkt_len = 8 + CRB3_PARAM_RESERVE_LEN + tlv_len;
	uint8_t * ptr;
	struct timespec ts;
	struct timespec cur_ts;
	struct timespec * ts_ptr;
	CRB3_ADDRESS addr;

	// check packet buffer size
	if(pkt_len > CRB3_MAX_PACKET_SIZE)
	{
		return CRB3_INVALID_PARAMETER;
	}

	// calc timeout value
	ts_ptr = _InitTimeSpec(TimeoutMS, &ts);

	// encode packet header
	ptr = pkt_buf;
	CRB3_SUBMIT_INT16_A(ptr, (int16_t)pkt_len);
	CRB3_SUBMIT_INT16_A(ptr, CRB3_PKT_CR_REQ);
	CRB3_SUBMIT_INT32_A(ptr, 0);

	// encode request header
	CRB3_SUBMIT_INT16_A(ptr, 1);							// ReqNum
	CRB3_SUBMIT_INT16_A(ptr, 0);							// ReqFlag
	CRB3_SUBMIT_INT32_A(ptr, CRSeq);						// CR-Seq
	CRB3_SUBMIT_INT32_A(ptr, CRId);							// CR-Id
	CRB3_SUBMIT_INT16_A(ptr, 1);							// CR-Flag: PKT-Mode
	CRB3_SUBMIT_INT16_A(ptr, (int16_t)ParamIn->ParamCount);	// PNum
	CRB3_SUBMIT_INT32_A(ptr, (int32_t)tlv_len);				// PSize
	ParamIn->OutputTLV((char *)ptr);

	// select connection & send packet
	do
	{
		int protocol;
		
		if(CRB3_OK == _SelectConnect(CRId, ParamIn, &addr, &protocol) &&
			CRB3_OK == _CRSend(&addr, protocol, pkt_buf, pkt_len))
		{
			return CRB3_OK;
		}

		// sleep & retry if send failed
		if(ts_ptr)
		{
			usleep(1000);
			clock_gettime(CLOCK_REALTIME, &cur_ts);
		}
	}while(ts_ptr && (cur_ts.tv_sec < ts_ptr->tv_sec ||
			(cur_ts.tv_sec == ts_ptr->tv_sec && cur_ts.tv_nsec < ts_ptr->tv_nsec)));

	return CRB3_TIMEOUT;
}

/********************************************************
* 函数名: CRB3Client::CRRecv
*
* 功能描述: 接收请求结果(PKT模式)
*
* 参数:
*      CRId      - (输出) 接收到的请求ID
*      CRSeq     - (输出) 接收到的请求顺序号
*      ParamOut  - (输出) 接收到的请求结果对象指针(CRB3Param *)
*      TimeoutMS - (输入) 接收超时时间，单位: 毫秒，0表示不等待，负值表示一直等待
*
* 返回值:
*      CRB3_OK - 操作成功
*      CRB3_TIMEOUT - 接收超时
*      CRB3_FAILED  - 操作取消
*      
********************************************************/
int CRB3Client::CRRecv(OUT uint32_t * CRId, OUT uint32_t * CRSeq, OUT CRB3Param ** ParamOut, IN int TimeoutMS)
{
	int irc = 0;
	int pop_len = 0;
	int fd = -1;
	int fd6 = -1;
	struct timespec ts;
	CRB3_RSP rsp_entry;

	// calc timeout value
	if(TimeoutMS > 0)
	{
		memset(&ts, 0, sizeof(ts));
		clock_gettime(CLOCK_REALTIME, &ts);
		ts.tv_nsec += (TimeoutMS % 1000) * 1000000;
		ts.tv_sec += TimeoutMS / 1000;
		if(ts.tv_nsec >= 1000000000)
		{
			ts.tv_nsec -= 1000000000;
			ts.tv_sec++;
		}
	}

	// wait resp queue
	pthread_mutex_lock(&RspMutex);
	while(((fd = DgramSocket) >= 0 || (fd6 = DgramSocket6) >= 0) && irc != ETIMEDOUT &&
			0 == (pop_len = quePop(&RspQueue, &rsp_entry, sizeof(CRB3_RSP))))
	{
		if(TimeoutMS < 0)
		{
			irc = pthread_cond_wait(&RspCond, &RspMutex);
		}
		else if(TimeoutMS > 0)
		{
			irc = pthread_cond_timedwait(&RspCond, &RspMutex, &ts);
		}
		else
		{
			break;
		}
	}
	pthread_mutex_unlock(&RspMutex);

	// set result
	if(pop_len == sizeof(CRB3_RSP))
	{
		if(CRId)
		{
			*CRId = rsp_entry.CRId;
		}
		if(CRSeq)
		{
			*CRSeq = rsp_entry.CRSeq;
		}
		if(ParamOut)
		{
			*ParamOut = rsp_entry.ParamOut;
		}
		else
		{
			delete rsp_entry.ParamOut;
		}
		return CRB3_OK;
	}
	if(fd < 0 && fd6 < 0)
	{
		return CRB3_FAILED;
	}
	return CRB3_TIMEOUT;
}

/********************************************************
* 函数名: CRB3Client::ShowConnect
*
* 功能描述: 显示服务端列表
*
* 参数:
*      OutputBuf - (输出) 输出缓冲区地址
*      BufLen    - (输入) 输出缓冲区大小，单位: 字节
*
* 返回值: 实际输出大小，单位: 字节
*      
********************************************************/
int CRB3Client::ShowConnect(OUT char * OutputBuf, IN int BufLen)
{
	int irc = 0;

	seprintf(OutputBuf, BufLen, irc,
			"SERVER_IP            PORT    PROTOCOL  STATUS     LAST_NOTIFY_RECEIVED  REQ-DELAY  REQ-QPS  PENDING  WEIGHT\n");
	seprintf(OutputBuf, BufLen, irc,
			"-----------------------------------------------------------------------------------------------------------\n");
	pthread_rwlock_rdlock(&ConnectLocker);
	if(irc < BufLen)
		irc += _ShowConnectList(&WorkingList, &OutputBuf[irc], BufLen - irc);
	if(irc < BufLen)
		irc += _ShowConnectList(&BackupList, &OutputBuf[irc], BufLen - irc);
	if(irc < BufLen)
		irc += _ShowConnectList(&ExceptionList, &OutputBuf[irc], BufLen - irc);
	pthread_rwlock_unlock(&ConnectLocker);
	return irc;
}

/********************************************************
* 函数名: CRB3Client::ShowQueue
*
* 功能描述: 显示队列情况
*
* 参数:
*      OutputBuf - (输出) 输出缓冲区地址
*      BufLen    - (输入) 输出缓冲区大小，单位: 字节
*
* 返回值: 实际输出大小，单位: 字节
*      
********************************************************/
int CRB3Client::ShowQueue(OUT char * OutputBuf, IN int BufLen)
{
	int irc = 0;
	int qsize;
	int qused;
	int qfree;

	seprintf(OutputBuf, BufLen, irc,
			"QUEUE_NAME    SIZE      USED     FREE\n");
	seprintf(OutputBuf, BufLen, irc,
			"-------------------------------------------------\n");

	pthread_mutex_lock(&RspMutex);
	qsize = RspQueue.total_size / sizeof(CRB3_RSP);
	qfree = RspQueue.free_size / sizeof(CRB3_RSP);
	qused = qsize - qfree;
	seprintf(OutputBuf, BufLen, irc,
			"%-13s %-9d %-9d %d(%d%%)\n", "PKT_RX", qsize, qused, qfree, qfree * 100 / qsize);
	pthread_mutex_unlock(&RspMutex);
	return irc;
}

/********************************************************
* 函数名: CRB3Client::_InitTimeSpec
*
* 功能描述: 计算超时时间点
*
* 参数:
*      TimeoutMS - (输入) 超时时间，单位: 毫秒，0表示不超时，负值表示永久等待
*      TimeSpec  - (输入) 超时时间点存储缓冲地址
*                  (输出) 计算后的超时时间点
*
* 返回值: 超时时间点存储缓冲地址，NULL表示超时时间为0
*      
********************************************************/
struct timespec * CRB3Client::_InitTimeSpec(IN int TimeoutMS, INOUT struct timespec * TimeSpec)
{
	if(TimeoutMS != 0)
	{
		if(TimeoutMS > 0)
		{
			clock_gettime(CLOCK_REALTIME, TimeSpec);
			TimeSpec->tv_nsec += (TimeoutMS % 1000) * 1000000;
			TimeSpec->tv_sec += TimeoutMS / 1000;
			if(TimeSpec->tv_nsec >= 1000000000)
			{
				TimeSpec->tv_nsec -= 1000000000;
				TimeSpec->tv_sec++;
			}
		}
		else
		{
			TimeSpec->tv_sec = 0x7fffffff;
			TimeSpec->tv_nsec = 0;
		}
		return TimeSpec;
	}
	return NULL;
}

/********************************************************
* 函数名: CRB3Client::_CRSend
*
* 功能描述: 发送请求报文
*
* 参数:
*      Addr      - (输入) 发送目的地址和端口
*      Protocol  - (输入) 使用的协议(SOCK_DGRAM or SOCK_STREAM)
*      PacketBuf - (输入) 报文数据地址
*      BufSize   - (输入) 报文数据大小，单位: 字节
*
* 返回值:
*      SOERR_OK   - 发送成功，数据已部分或全部发送
*      SOERR_PEND - 发送失败，Pend队列中的数据在等待(仅TCP)
*      SOERR_INVALID_CLIENT - 发送失败，无效的客户端地址(仅TCP)
*      SOERR_SOCKET_ERROR   - 发送失败，套接字错误，具体信息查看errno
********************************************************/
int CRB3Client::_CRSend(IN CRB3_ADDRESS * Addr, IN int Protocol, IN uint8_t * PacketBuf, IN int BufSize)
{
	if(Protocol == SOCK_DGRAM)
	{
		int irc = sendto(Addr->sa.sa_family == AF_INET ? DgramSocket : DgramSocket6, PacketBuf, BufSize, MSG_DONTWAIT, &Addr->sa,
						Addr->sa.sa_family == AF_INET ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6));
		return irc > 0 ? SOERR_OK : SOERR_SOCKET_ERROR;
	}
	else
	{
		return Send(Addr, (char *)PacketBuf, BufSize);
	}
}

/********************************************************
* 函数名: CRB3Client::_SQSend
*
* 功能描述: 发送状态查询报文
*
* 参数:
*      Addr - (输入) 发送目的地址和端口
*
* 返回值: 发送的字节数，0或负值表示有错误
*      
********************************************************/
int CRB3Client::_SQSend(IN CRB3_ADDRESS * Addr)
{
	char packet_buf[8];
	char * ptr = packet_buf;

	CRB3_SUBMIT_INT16(ptr, 8);
	CRB3_SUBMIT_INT16(ptr, CRB3_PKT_STATUS_REQ);
	CRB3_SUBMIT_INT32(ptr, 0);
	return sendto(Addr->sa.sa_family == AF_INET ? DgramSocket : DgramSocket6, packet_buf, 8, 0, &Addr->sa, CRB3_SOCKLEN(Addr));
}

/********************************************************
* 函数名: CRB3Client::_ARSend
*
* 功能描述: 发送Alive响应报文
*
* 参数:
*      Addr - (输入) 发送目的地址和端口
*
* 返回值: 发送的字节数，0或负值表示有错误
*      
********************************************************/
int CRB3Client::_ARSend(IN CRB3_ADDRESS * Addr)
{
	char packet_buf[8];
	char * ptr = packet_buf;

	CRB3_SUBMIT_INT16(ptr, 8);
	CRB3_SUBMIT_INT16(ptr, CRB3_PKT_ALIVE_RSP);
	CRB3_SUBMIT_INT32(ptr, 0);
	return Send(Addr, packet_buf, 8);
}

/********************************************************
* 函数名: CRB3Client::_Response
*
* 功能描述: 请求结果处理例程
*
* 参数:
*      CRSeq    - (输入) 待处理的请求顺序号
*      CRId     - (输入) 待处理的请求ID
*      CRFlag   - (输入) 待处理的请求标志
*      ParamOut - (输入) 待处理的请求结果参数
*
* 返回值:
*      CRB3_OK - 操作成功
*      CRB3_EMPTY_QUEUE - 操作失败: 接收队列已满(仅PKT模式)
*      CRB3_NOT_FIND    - 操作失败: 未找到对应的CRL条目(仅API模式)
*      CRB3_NOT_SERVICE - 操作失败: CRL服务未启动(仅API模式)
********************************************************/
int CRB3Client::_Response(IN uint32_t CRSeq, IN uint32_t CRId, IN uint16_t CRFlag, IN CRB3Param * ParamOut)
{
	int irc;

	if(CRFlag & 0x0001)	// PKT-Mode
	{
		CRB3_RSP rsp_entry;

		rsp_entry.CRId = CRId;
		rsp_entry.CRSeq = CRSeq;
		rsp_entry.ParamOut = ParamOut;
		pthread_mutex_lock(&RspMutex);
		if(0 == quePush(&RspQueue, &rsp_entry, sizeof(CRB3_RSP)))
		{
			irc = CRB3_EMPTY_QUEUE;
			PktRxDiscard++;
		}
		else
		{
			irc = CRB3_OK;
			PktRxSucceed++;
			pthread_cond_signal(&RspCond);
		}
		pthread_mutex_unlock(&RspMutex);
	}
	else	// API-Mode
	{
		irc = CrlMgr.FillCRL(CRSeq, ParamOut);
	}

	return irc;
}

/********************************************************
* 函数名: CRB3Client::_FindConnect
*
* 功能描述: 查找服务端节点
*
* 参数:
*      IpAddress - (输入) 待查找的服务端节点的IP地址
*      Port      - (输入) 待查找的服务端节点的服务端口
*      FromList  - (输出) 待查找的服务端节点所属链表
*
* 返回值: 服务端节点指针，NULL表示未找到
*      
********************************************************/
CRB3_CONNECTION * CRB3Client::_FindConnect(IN char * IpAddress, IN int Port, OUT LIST ** FromList)
{
	CRB3_ADDRESS addr;

	CRB3String2Address(IpAddress, Port, &addr);
	return _FindConnect2(addr, FromList);
}

/********************************************************
* 函数名: CRB3Client::_FindConnect2
*
* 功能描述: 查找服务端节点
*
* 参数:
*      Addr     - (输入) 待查找的服务端节点的IP地址和端口
*      FromList - (输出) 待查找的服务端节点所属链表
*
* 返回值: 服务端节点指针，NULL表示未找到
*      
********************************************************/
CRB3_CONNECTION * CRB3Client::_FindConnect2(IN CRB3_ADDRESS Addr, OUT LIST ** FromList)
{
	CRB3_CONNECTION * cnode;

	cnode = (CRB3_CONNECTION *)lstFirst(&WorkingList);
	while(cnode)
	{
		if(0 == CRB3AddressCompare(&cnode->ServerAddr, &Addr))
		{
			if(FromList)
			{
				*FromList = &WorkingList;
			}
			return cnode;
		}
		cnode = (CRB3_CONNECTION *)lstNext((NODE *)cnode);
	}
	cnode = (CRB3_CONNECTION *)lstFirst(&BackupList);
	while(cnode)
	{
		if(0 == CRB3AddressCompare(&cnode->ServerAddr, &Addr))
		{
			if(FromList)
			{
				*FromList = &BackupList;
			}
			return cnode;
		}
		cnode = (CRB3_CONNECTION *)lstNext((NODE *)cnode);
	}
	cnode = (CRB3_CONNECTION *)lstFirst(&ExceptionList);
	while(cnode)
	{
		if(0 == CRB3AddressCompare(&cnode->ServerAddr, &Addr))
		{
			if(FromList)
			{
				*FromList = &ExceptionList;
			}
			return cnode;
		}
		cnode = (CRB3_CONNECTION *)lstNext((NODE *)cnode);
	}
	return NULL;
}

/********************************************************
* 函数名: CRB3Client::_SelectConnect
*
* 功能描述: 选择服务端
*
* 参数:
*      CRId     - (输入) 请求ID
*      ParamIn  - (输入) 请求参数列表
*      Addr     - (输出) 选择到的服务端的IP地址和服务端口
*      Protocol - (输出) 数据通信协议
*
* 返回值:
*      CRB3_OK - 操作成功
*      CRB3_NOT_FIND - 未找到可用的服务端
*      
********************************************************/
int CRB3Client::_SelectConnect(IN uint32_t CRId, IN CRB3Param * ParamIn,
					OUT CRB3_ADDRESS * Addr, OUT int * Protocol)
{
	CRB3_CONNECTION * cnode;

	cnode = _GetConnect(CRId, ParamIn);
	if(cnode)
	{
		*Addr = cnode->ServerAddr;
		*Protocol = cnode->Protocol;
		_PutConnect(cnode);
		return CRB3_OK;
	}
	return CRB3_NOT_FIND;
}

CRB3_CONNECTION * CRB3Client::_GetConnect(IN uint32_t CRId, IN CRB3Param * ParamIn)
{
	CRB3_CONNECTION * cnode;

	pthread_rwlock_wrlock(&ConnectLocker);
	if(ServiceMode == 2)
	{
		cnode = _NCHAlgCalc(CRId, ParamIn);
		if(cnode)
		{
			CRB3_ATOMIC32_INC(cnode->Pending);
		}
	}
	else
	{
		cnode = (CRB3_CONNECTION *)lstFirst(&WorkingList);
		if(cnode && ServiceMode == 1)
		{
			// find node which meet the conditions
			while(cnode && CRB3_ATOMIC32_INC_AND_TEST(cnode->Pending) > cnode->PendMax)
			{
				CRB3_ATOMIC32_DEC(cnode->Pending);
				cnode = (CRB3_CONNECTION *)lstNext(&cnode->node);
			}

			if(cnode)
			{
				// rebalance it
				lstDelete(&WorkingList, (NODE *)cnode);
				lstAdd(&WorkingList, (NODE *)cnode);
			}
		}
	}
	if(cnode)
	{
		cnode->Reference++;
		cnode->RefList = &WorkingList;
	}
	pthread_rwlock_unlock(&ConnectLocker);
	return cnode;
}

void CRB3Client::_PutConnect(IN CRB3_CONNECTION * cnode)
{
	pthread_rwlock_wrlock(&ConnectLocker);
	CRB3_ATOMIC32_DEC(cnode->Pending);
	if(--cnode->Reference == 0 && cnode->ToDelete)
	{
		if(cnode->Rate)delete cnode->Rate;
		delete cnode->Delay;
		lstDelete(cnode->RefList, &cnode->node);
		free(cnode);
	}
	pthread_rwlock_unlock(&ConnectLocker);
}

/********************************************************
* 函数名: CRB3Client::_ShowConnectList
*
* 功能描述: 显示服务端链表的节点信息
*
* 参数:
*      List      - (输入) 服务端链表
*      OutputBuf - (输出) 输出缓冲区地址
*      BufLen    - (输入) 输出缓冲区大小，单位: 字节
*
* 返回值: 实际输出大小，单位: 字节
*      
********************************************************/
int CRB3Client::_ShowConnectList(IN LIST * List, OUT char * OutputBuf, IN int BufLen)
{
	CRB3_CONNECTION * cnode = (CRB3_CONNECTION *)lstFirst(List);
	int irc = 0;

	while(cnode)
	{
		int port;
		char ipstr[40];
		char status[12];
		char tbuf_notify[40];
		struct tm datetime;
		char protocol[8];
		char delay_str[16];
		char qps_str[16];

		CRB3Address2String(&cnode->ServerAddr, ipstr, 40, &port);
		switch(cnode->Status)
		{
		case CRB3_STATUS_WORKING:	strncpy(status, "master", 11);		break;
		case CRB3_STATUS_BACKUP:	strncpy(status, "slaver", 11);		break;
		case CRB3_STATUS_EXCEPTION:	strncpy(status, "exception", 11);	break;
		default:					strncpy(status, "unknow", 9);		break;
		}
		if(cnode->LastNotifyTime > 0)
		{
			localtime_r(&cnode->LastNotifyTime, &datetime);
			strftime(tbuf_notify, 40, "%Y-%m-%d %X", &datetime);
		}
		else
		{
			strncpy(tbuf_notify, "N/A", 39);
		}
		if(cnode->Protocol == SOCK_DGRAM)
		{
			strcpy(protocol, "UDP");
		}
		else
		{
			strcpy(protocol, "TCP");
		}
		if(cnode->Status == CRB3_STATUS_WORKING && cnode->Delay)
		{
			hrtime_t delay;
			
			delay = cnode->Delay->GetDelayAvg();
			snprintf(delay_str, sizeof(delay_str), "%u.%06us", delay.tv_sec, delay.tv_usec);
		}
		else
		{
			strcpy(delay_str, "N/A");
		}
		if(cnode->Status == CRB3_STATUS_WORKING && cnode->Rate)
		{
			StatNode stat;

			cnode->Rate->GetRateLast(1, &stat);
			snprintf(qps_str, sizeof(qps_str), "%u", stat.PulseCnt);
		}
		else
		{
			strcpy(qps_str, "N/A");
		}
		seprintf(OutputBuf, BufLen, irc, "%-20s %-7d %-9s %-10s %-21s %-11s %-7s %3d/%-3d  %f\n",
					ipstr, port, protocol, status, tbuf_notify, delay_str, qps_str, cnode->Pending, cnode->PendMax, cnode->PendWeight);
		cnode = (CRB3_CONNECTION *)lstNext((NODE *)cnode);
	}
	return irc;
}

/********************************************************
* 函数名: CRB3Client::_ReceiveDatagram
*
* 功能描述: 处理从数据报套接字中收到的报文
*
* 参数:
*      sockfd - (输入) 数据报套接字
*
* 返回值:
*      CRB3_OK - 处理成功
*      CRB3_NOT_CONNECTED     - 处理失败，套接字已关闭
*      CRB3_INVALID_PARAMETER - 处理失败，报文头部太短
*      
********************************************************/
int CRB3Client::_ReceiveDatagram(IN int sockfd)
{
	char packet[CRB3_MAX_PACKET_SIZE];
	socklen_t len;
	int packet_size;
	CRB3_ADDRESS from_addr;
	
	len = sizeof(from_addr);
	packet_size = recvfrom(sockfd, packet, CRB3_MAX_PACKET_SIZE, 0, &from_addr.sa, &len);
	if(packet_size <= 0)
	{
		if(packet_size < 0 && errno != EINTR)
		{
			LogMng->Printf(SEVERITY_EMERG, "recvfrom error: %s", strerror(errno));
		}
		return CRB3_NOT_CONNECTED;
	}
	else if(packet_size < 8)
	{
		LogMng->Printf(SEVERITY_ALERT, "receive error: invalid packet size(%d)", packet_size);
		return CRB3_INVALID_PARAMETER;
	}

	return _PacketProcess(&from_addr, packet, packet_size);
}

/********************************************************
* 函数名: CRB3Client::_ReceiveLoop
*
* 功能描述: 报文接收循环
*
* 参数: 无
*
* 返回值: CRB3_OK - 正常退出
*      
********************************************************/
int CRB3Client::_ReceiveLoop(void)
{
	struct epoll_event events[8];

	memset(events, 0, sizeof(struct epoll_event) * 8);
	while(DgramSocket >= 0 || DgramSocket6 >= 0)
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

		for(idx=0;idx<nfds;idx++)
		{
			// receive from datagram socket
			if(events[idx].data.fd == DgramSocket)
			{
				if(events[idx].events == EPOLLIN &&
					CRB3_OK != (irc = _ReceiveDatagram(DgramSocket)))
				{
				}
			}
			else if(events[idx].data.fd == DgramSocket6)
			{
				if(events[idx].events == EPOLLIN &&
					CRB3_OK != (irc = _ReceiveDatagram(DgramSocket6)))
				{
				}
			}

			// receive from stream socket
			else
			{
				OnEvent(&events[idx]);
			}
		}

		#if 0
		// output rate log
		if(RateLogFile[0])
		{
			static time_t prev_timestamp = 0;
			StatNode rx_qps;
			StatNode rx_bps;
			StatNode tx_qps;
			StatNode tx_bps;

			RxQps.GetRateLast(1, &rx_qps);
			RxBps.GetRateLast(1, &rx_bps);
			TxQps.GetRateLast(1, &tx_qps);
			TxBps.GetRateLast(1, &tx_bps);
			if(rx_qps.Timestamp > prev_timestamp &&
				rx_bps.Timestamp > prev_timestamp &&
				tx_qps.Timestamp > prev_timestamp &&
				tx_bps.Timestamp > prev_timestamp)
			{
				FILE * fp = fopen(RateLogFile, "a");
				if(fp)
				{
					char tbuf[64];
					struct tm datetime;
					
					localtime_r(&rx_qps.Timestamp, &datetime);
					strftime(tbuf, 64, "%Y-%m-%d %X", &datetime);
					fprintf(fp, "[%s] rx_qps: %u , rx_bps: %u , tx_qps: %u , tx_bps: %u\n",
						tbuf, rx_qps.PulseCnt, rx_bps.PulseCnt, tx_qps.PulseCnt, tx_bps.PulseCnt);
					fclose(fp);
				}

				prev_timestamp = rx_qps.Timestamp;
			}
		}
		#endif
	}
	return CRB3_OK;
}

/********************************************************
* 函数名: CRB3Client::_CheckLoop
*
* 功能描述: 服务端状态检查循环
*
* 参数: 无
*
* 返回值: CRB3_OK - 正常退出
*      
********************************************************/
int CRB3Client::_CheckLoop(void)
{
	int irc;
	int pop_len;
	bool is_exit = false;
	struct timespec ts;
	CRB3_STATUS_RSP status_resp;

	// set timeout value of initiation
	clock_gettime(CLOCK_REALTIME, &ts);

	// enter check loop
	while(1)
	{
		bool go_action = false;
		
		// wait notify, timeout, or exit singal
		irc = 0;
		pop_len = 0;
		pthread_mutex_lock(&CheckMutex);
		while((is_exit = CheckExit) == false && irc != ETIMEDOUT &&
				0 == (pop_len = quePop(&CheckQueue, &status_resp, sizeof(CRB3_STATUS_RSP))))
		{
			irc = pthread_cond_timedwait(&CheckCond, &CheckMutex, &ts);
		}
		pthread_mutex_unlock(&CheckMutex);

		// process exit signal
		if(is_exit == true)break;

		// process timeout
		if(irc == ETIMEDOUT)
		{
			CRB3_CONNECTION * cnode;
			double total = 0.0;
			int count = 0;

			// check working node & send status query
			pthread_rwlock_rdlock(&ConnectLocker);
			cnode = (CRB3_CONNECTION *)lstFirst(&WorkingList);
			while(cnode)
			{
				if(time(NULL) - cnode->LastNotifyTime >= NotifyTimeout)
				{
					cnode->Flag |= CRB3_FLAG_GO_EXCEPTION;
					go_action = true;
				}
				else
				{
					hrtime_t delay;
					
					delay = cnode->Delay->GetDelayAvg();
					if(delay.tv_sec > 0 || delay.tv_usec > 0)
					{
						count++;
						cnode->PendWeight = 1000000.0 / (delay.tv_sec * 1000000 + delay.tv_usec);
						total += cnode->PendWeight;
					}
				}
				_SQSend(&cnode->ServerAddr);
				if(cnode->Protocol == SOCK_STREAM)
				{
					_CheckStream(cnode);
				}
				cnode = (CRB3_CONNECTION *)lstNext((NODE *)cnode);
			}
			if(count > 0 && total > 0)
			{
				cnode = (CRB3_CONNECTION *)lstFirst(&WorkingList);
				while(cnode)
				{
					if(!(cnode->Flag & CRB3_FLAG_GO_EXCEPTION))
					{
						cnode->PendMax = (int)(MaxConcurrent * cnode->PendWeight / total) + 1;
					}
					cnode = (CRB3_CONNECTION *)lstNext((NODE *)cnode);
				}
			}

			// check backup node & send status query
			cnode = (CRB3_CONNECTION *)lstFirst(&BackupList);
			while(cnode)
			{
				if(time(NULL) - cnode->LastNotifyTime >= NotifyTimeout)
				{
					cnode->Flag |= CRB3_FLAG_GO_EXCEPTION;
					go_action = true;
				}
				_SQSend(&cnode->ServerAddr);
				#if 0
				if(cnode->Protocol == SOCK_STREAM)
				{
					_CheckStream(cnode);
				}
				#endif
				cnode = (CRB3_CONNECTION *)lstNext((NODE *)cnode);
			}

			// travel exception list & send status query
			cnode = (CRB3_CONNECTION *)lstFirst(&ExceptionList);
			while(cnode)
			{
				_SQSend(&cnode->ServerAddr);
				#if 0
				if(cnode->Protocol == SOCK_STREAM)
				{
					_CheckStream(cnode);
				}
				#endif
				cnode = (CRB3_CONNECTION *)lstNext((NODE *)cnode);
			}
			pthread_rwlock_unlock(&ConnectLocker);

			// set next timeout value
			clock_gettime(CLOCK_REALTIME, &ts);
			ts.tv_sec += NotifyGap;
		}

		// process notify
		if(pop_len == sizeof(CRB3_STATUS_RSP))
		{
			CRB3_CONNECTION * cnode;
			LIST * from_list;

			pthread_rwlock_rdlock(&ConnectLocker);
			cnode = _FindConnect2(status_resp.ServerAddr, &from_list);
			if(cnode)
			{
				cnode->LastNotifyTime = time(NULL);
				if(cnode->Status != status_resp.Status)
				{
					if(status_resp.Status == CRB3_STATUS_WORKING)
					{
						cnode->Flag |= CRB3_FLAG_GO_WORKING;
					}
					else if(status_resp.Status == CRB3_STATUS_BACKUP)
					{
						cnode->Flag |= CRB3_FLAG_GO_BACKUP;
					}
					else
					{
						cnode->Flag |= CRB3_FLAG_GO_EXCEPTION;
					}
					go_action = true;
				}
			}
			pthread_rwlock_unlock(&ConnectLocker);
		}

		// go action process if happened
		if(go_action)
		{
			pthread_rwlock_wrlock(&ConnectLocker);
			_GoAction(&WorkingList, "master");
			_GoAction(&BackupList, "slave");
			_GoAction(&ExceptionList, "exception");
			pthread_rwlock_unlock(&ConnectLocker);
		}

	}
	return CRB3_OK;
}

/********************************************************
* 函数名: CRB3Client::_GoAction
*
* 功能描述: 处理节点状态变化引起的节点移动
*
* 参数:
*      FromList - (输入) 待处理的链表
*
* 返回值: 无
********************************************************/
void CRB3Client::_GoAction(IN LIST * FromList, IN char * FromStatus)
{
	CRB3_CONNECTION * cnode;
	CRB3_CONNECTION * cnext;

	cnode = (CRB3_CONNECTION *)lstFirst(FromList);
	while(cnode)
	{
		char ipstr[40];
		int port;
		
		cnext = (CRB3_CONNECTION *)lstNext((NODE *)cnode);
		if(cnode->Flag & CRB3_FLAG_GO_WORKING)
		{
			lstDelete(FromList, (NODE *)cnode);
			#ifdef CRB3_TRUST_SERVER
			lstInsert(&WorkingList, NULL, (NODE *)cnode);
			#else
			lstAdd(&WorkingList, (NODE *)cnode);
			#endif
			cnode->Status = CRB3_STATUS_WORKING;

			CRB3Address2String(&cnode->ServerAddr, ipstr, 40, &port);
			LogMng->Printf(SEVERITY_ALERT, "server '%s(%d)' status change: %s -> master", ipstr, port, FromStatus);
		}
		else if(cnode->Flag & CRB3_FLAG_GO_BACKUP)
		{
			lstDelete(FromList, (NODE *)cnode);
			lstAdd(&BackupList, (NODE *)cnode);
			cnode->Status = CRB3_STATUS_BACKUP;

			CRB3Address2String(&cnode->ServerAddr, ipstr, 40, &port);
			LogMng->Printf(SEVERITY_ALERT, "server '%s(%d)' status change: %s -> slave", ipstr, port, FromStatus);
		}
		else if(cnode->Flag & CRB3_FLAG_GO_EXCEPTION)
		{
			lstDelete(FromList, (NODE *)cnode);
			lstAdd(&ExceptionList, (NODE *)cnode);
			cnode->Status = CRB3_STATUS_EXCEPTION;

			CRB3Address2String(&cnode->ServerAddr, ipstr, 40, &port);
			LogMng->Printf(SEVERITY_ALERT, "server '%s(%d)' status change: %s -> exception", ipstr, port, FromStatus);
		}
		cnode->Flag &= ~(CRB3_FLAG_GO_WORKING | CRB3_FLAG_GO_BACKUP | CRB3_FLAG_GO_EXCEPTION);
		cnode = cnext;
	}
}

/********************************************************
* 函数名: CRB3Client::_CheckStream
*
* 功能描述: 检查数据流是否存在，如果不存在则建立并尝试连接
*
* 参数:
*      cnode - (输入) 待检测的数据流
*
* 返回值: 无
********************************************************/
void CRB3Client::_CheckStream(IN CRB3_CONNECTION * cnode)
{
	bool connecting;
	int irc;
	int retry = 10;
	char ipstr[40];
	int port;

	CRB3Address2String(&cnode->ServerAddr, ipstr, 40, &port);
retry:
	irc = GetInfo(&cnode->ServerAddr, NULL, NULL, &connecting);
	if(irc != SOERR_OK)
	{
		LogMng->Printf(SEVERITY_DEBUG, "connect to server '%s(%d)'\n", ipstr, port);
		ConnectTo(&cnode->ServerAddr, NULL, false, NULL);
		goto retry;
	}
	if(connecting)
	{
		if(--retry > 0)
		{
			usleep(100000);
			goto retry;
		}
		LogMng->Printf(SEVERITY_DEBUG, "connect to server '%s(%d)' timeout\n", ipstr, port);
		Disconnect(&cnode->ServerAddr);
	}
}

/********************************************************
* 函数名: CRB3Client::_PacketProcess
*
* 功能描述: 处理报文(包括UDP和TCP)
*
* 参数:
*      from_addr   - (输入) 服务端地址
*      packet      - (输入) 报文缓冲地址
*      packet_size - (输入) 报文大小，单位: 字节
*
* 返回值: SOERR_OK表示成功
*      
********************************************************/
int CRB3Client::_PacketProcess(IN CRB3_ADDRESS * from_addr, IN char * packet, IN int packet_size)
{
	uint16_t hdr_len;
	uint16_t hdr_id;
	uint32_t hdr_status;
	char * ptr = packet;
	char * ptr_end = ptr + packet_size;

	CRB3_FETCH_INT16(ptr, hdr_len);
	CRB3_FETCH_INT16(ptr, hdr_id);
	CRB3_FETCH_INT32(ptr, hdr_status);
	if(hdr_id == CRB3_PKT_ALIVE_REQ)
	{
		_ARSend(from_addr);
	}
	else if(hdr_id == CRB3_PKT_STATUS_RSP)
	{
		uint32_t status;
		uint32_t weight;
		CRB3_STATUS_RSP notify;

		// process status notify
		CRB3_FETCH_INT32(ptr, status);
		CRB3_FETCH_INT32(ptr, weight);
		notify.ServerAddr = *from_addr;
		notify.Status = status;
		notify.Weight = weight;
		pthread_mutex_lock(&CheckMutex);
		quePush(&CheckQueue, &notify, sizeof(CRB3_STATUS_RSP));
		pthread_cond_signal(&CheckCond);
		pthread_mutex_unlock(&CheckMutex);
	}
	else if(hdr_id == CRB3_PKT_CR_RSP)
	{
		uint16_t resp_num;
		uint16_t resp_flag;
		uint16_t idx;

		if(hdr_status != CRB3_OK)
		{
			LogMng->Printf(SEVERITY_ALERT, "receive error: status code (%d)", hdr_status);
			return CRB3_OK;
		}

		CRB3_FETCH_INT16(ptr, resp_num);
		CRB3_FETCH_INT16(ptr, resp_flag);

		// process status notify if carried with link-status
		if(resp_flag & 0x0001)
		{
			uint32_t new_status;
			uint32_t new_weight;
			CRB3_STATUS_RSP notify;

			CRB3_FETCH_INT32(ptr, new_status);
			CRB3_FETCH_INT32(ptr, new_weight);
			notify.ServerAddr = *from_addr;
			notify.Status = new_status;
			notify.Weight = new_weight;
			pthread_mutex_lock(&CheckMutex);
			quePush(&CheckQueue, &notify, sizeof(CRB3_STATUS_RSP));
			pthread_cond_signal(&CheckCond);
			pthread_mutex_unlock(&CheckMutex);
		}

		// process response
		for(idx=0;idx<resp_num;idx++)
		{
			uint32_t cr_seq;
			uint32_t cr_id;
			uint16_t cr_flag;
			uint16_t param_num;
			uint32_t param_size;;
			CRB3Param * param;
			int irc;

			CRB3_FETCH_INT32(ptr, cr_seq);
			CRB3_FETCH_INT32(ptr, cr_id);
			CRB3_FETCH_INT16(ptr, cr_flag);
			CRB3_FETCH_INT16(ptr, param_num);
			CRB3_FETCH_INT32(ptr, param_size);
			if(ptr + param_size > ptr_end)
			{
				LogMng->Printf(SEVERITY_ALERT, "receive error: invalid parameter size(%u) for response sequence(%u)", param_size, cr_seq);
				break;
			}
			if(param_num > 0)
			{
				param = new CRB3Param((int)param_num, (char *)ptr, (int)param_size);
			}
			else
			{
				param = new CRB3Param(0);
			}
			if(CRB3_OK != (irc = _Response(cr_seq, cr_id, cr_flag, param)))
			{
				LogMng->Printf(SEVERITY_ALERT, "receive error: response failed with error code %d", irc);
				delete param;
			}
		}
	}
	else
	{
		LogMng->Printf(SEVERITY_ALERT, "receive error: unknow packet type(%d)", hdr_id);
	}

	return CRB3_OK;
}

int CRB3Client::_ProcessDioResponse(IN char * packet_buf, IN int packet_size, OUT CRB3Param ** ParamOut)
{
	uint16_t hdr_len;
	uint16_t hdr_id;
	uint32_t hdr_status;
	char * ptr = packet_buf;
	char * ptr_end = ptr + packet_size;

	CRB3_FETCH_INT16(ptr, hdr_len);
	CRB3_FETCH_INT16(ptr, hdr_id);
	CRB3_FETCH_INT32(ptr, hdr_status);
	if(hdr_id != CRB3_PKT_CR_RSP)
	{
		LogMng->Printf(SEVERITY_ALERT, "dio receive error: invalid response id (%d)", hdr_id);
		return CRB3_FAILED;
	}

	uint16_t resp_num;
	uint16_t resp_flag;
	uint16_t idx;

	if(hdr_status != CRB3_OK)
	{
		LogMng->Printf(SEVERITY_ALERT, "dio receive error: status code (%d)", hdr_status);
		return CRB3_FAILED;
	}

	CRB3_FETCH_INT16(ptr, resp_num);
	CRB3_FETCH_INT16(ptr, resp_flag);

	// process status notify if carried with link-status
	if(resp_num > 1)
	{
		LogMng->Printf(SEVERITY_ALERT, "dio receive error: invalid response number (%d)", resp_num);
		return CRB3_FAILED;
	}

	// process response
	uint32_t cr_seq;
	uint32_t cr_id;
	uint16_t cr_flag;
	uint16_t param_num;
	uint32_t param_size;;
	CRB3Param * param;
	int irc;

	CRB3_FETCH_INT32(ptr, cr_seq);
	CRB3_FETCH_INT32(ptr, cr_id);
	CRB3_FETCH_INT16(ptr, cr_flag);
	CRB3_FETCH_INT16(ptr, param_num);
	CRB3_FETCH_INT32(ptr, param_size);
	if(ptr + param_size > ptr_end)
	{
		LogMng->Printf(SEVERITY_ALERT, "dio receive error: invalid parameter size(%u) for response sequence(%u)", param_size, cr_seq);
		return CRB3_FAILED;
	}
	if(param_num > 0)
	{
		*ParamOut = new CRB3Param((int)param_num, (char *)ptr, (int)param_size);
	}
	else
	{
		*ParamOut = new CRB3Param(0);
	}
	return CRB3_OK;
}

int CRB3Client::_NCHInit(void)
{
	NCHMap.InitNodes = 0;
	NCHMap.MaxNodes = CRB3_MAX_HASH_NODES;
	memset(NCHMap.NCHTable, 0, sizeof(int) * CRB3_MAX_HASH_NODES);
	ihashInit(&NCHAlg, 4*1024, NULL);
	NCHAlgDef.type = ALG_TYPE_PMASK;
	NCHAlgDef.value = 0x0001;
	irbtInit(&NCHTree);
	return CRB3_OK;
}

int CRB3Client::_NCHLoad(IN int init_nodes)
{
	int idx;
	
	NCHMap.InitNodes = init_nodes;
	for(idx = 0; idx < NCHMap.MaxNodes; idx++)
	{
		NCHMap.NCHTable[idx] = genid(NCHMap.InitNodes);
	}
	return CRB3_OK;
}

int CRB3Client::_NCHGet(IN int srv_id)
{
	if(NCHMap.InitNodes > 0 && srv_id >= 0 && srv_id < NCHMap.MaxNodes)
	{
		return NCHMap.NCHTable[srv_id];
	}
	return(-1);
}

int CRB3Client::_NCHAlgConfig(IN const char * nch_alg)
{
	char buf[4096];
	char * token;
	char * nexttoken;
	
	// string format: default,<type>,<value>|<crid>,<type>,<value>|...
	//                <type> := {pmask|fixed}
	//                <value> := {<pmask-value>|<server-index>}
	//                <pmask-value> := integer of hex format like 'C1'
	memset(buf, 0, sizeof(buf));
	strncpy(buf, nch_alg, sizeof(buf) - 1);
	token = strtok_r(buf, "|", &nexttoken);
	while(token)
	{
		char * subtoken;
		char * str_crid = strtok_r(token, ",", &subtoken);
		char * str_type = strtok_r(NULL, ",", &subtoken);
		char * str_value = strtok_r(NULL, ",", &subtoken);

		do
		{
			CRB3_NCH_ALG_TYPE type;
			uint64_t value = 0;
			
			if(!str_value)
			{
				LogMng->Printf(SEVERITY_EMERG, "[CRB] syntax error of node_hash_alg configure");
				break;
			}
			
			if(0 == strcmp(str_type, "pmask"))
			{
				type = ALG_TYPE_PMASK;
			}
			else if(0 == strcmp(str_type, "fixed"))
			{
				type = ALG_TYPE_FIXED;
			}
			else
			{
				LogMng->Printf(SEVERITY_EMERG, "[CRB] invalid type '%s' of node_hash_alg configure", str_type);
				break;
			}
			
			sscanf(str_value, "%llx", &value);
			if(value == 0)
			{
				LogMng->Printf(SEVERITY_EMERG, "[CRB] invalid value '%s' of node_hash_alg configure", str_value);
				break;
			}
			
			if(0 == strcmp(str_crid, "default"))
			{
				NCHAlgDef.type = type;
				NCHAlgDef.value = value;
			}
			else
			{
				uint32_t crid;
				crid = (uint32_t)atoi(str_crid);
				if(type == ALG_TYPE_PMASK)
				{
					_NCHAlgSetPMask(crid, value);
				}
				else
				{
					_NCHAlgSetFixed(crid, value);
				}
			}
		}while(0);
		token = strtok_r(NULL, "|", &nexttoken);
	}
	return CRB3_OK;
}

int CRB3Client::_NCHAlgSetFixed(IN uint32_t CRId, IN int ServerId)
{
	CRB3_NCH_ALG_NODE * anode;

	anode = (CRB3_NCH_ALG_NODE *)ihashSearch(&NCHAlg, (unsigned long)CRId);
	if(anode)
	{
		anode->type = ALG_TYPE_FIXED;
		anode->value = _NCHGet(ServerId);
		anode->func = NULL;
	}
	else
	{
		anode = (CRB3_NCH_ALG_NODE *)malloc(sizeof(CRB3_NCH_ALG_NODE));
		anode->type = ALG_TYPE_FIXED;
		anode->value = _NCHGet(ServerId);
		anode->func = NULL;
		ihashInsert(&NCHAlg, &anode->node, (unsigned long)CRId);
	}
	return CRB3_OK;
}

int CRB3Client::_NCHAlgSetPMask(IN uint32_t CRId, IN uint64_t ParamMask)
{
	if(!ParamMask)
	{
		return CRB3_INVALID_PARAMETER;
	}
	
	CRB3_NCH_ALG_NODE * anode;
	anode = (CRB3_NCH_ALG_NODE *)ihashSearch(&NCHAlg, (unsigned long)CRId);
	if(anode)
	{
		anode->type = ALG_TYPE_PMASK;
		anode->value = ParamMask;
		anode->func = NULL;
	}
	else
	{
		anode = (CRB3_NCH_ALG_NODE *)malloc(sizeof(CRB3_NCH_ALG_NODE));
		anode->type = ALG_TYPE_PMASK;
		anode->value = ParamMask;
		anode->func = NULL;
		ihashInsert(&NCHAlg, &anode->node, (unsigned long)CRId);
	}
	return CRB3_OK;
}

int CRB3Client::_NCHAlgSetCustom(IN uint32_t CRId, IN NCH_ALG_HASH_FUNC Func, IN uint64_t Spare)
{
	CRB3_NCH_ALG_NODE * anode;
	anode = (CRB3_NCH_ALG_NODE *)ihashSearch(&NCHAlg, (unsigned long)CRId);
	if(anode)
	{
		anode->type = ALG_TYPE_CUSTOM;
		anode->value = Spare;
		anode->func = Func;
	}
	else
	{
		anode = (CRB3_NCH_ALG_NODE *)malloc(sizeof(CRB3_NCH_ALG_NODE));
		anode->type = ALG_TYPE_CUSTOM;
		anode->value = Spare;
		anode->func = Func;
		ihashInsert(&NCHAlg, &anode->node, (unsigned long)CRId);
	}
	return CRB3_OK;
}

CRB3_CONNECTION * CRB3Client::_NCHAlgCalc(IN uint32_t CRId, IN CRB3Param * Param)
{
	CRB3_NCH_ALG_NODE * anode;
	unsigned long hash;
	
	// calc hash value by 'Param'
	anode = (CRB3_NCH_ALG_NODE *)ihashSearch(&NCHAlg, (unsigned long)CRId);
	if(!anode)
	{
		anode = &NCHAlgDef;
	}
	if(anode->type == ALG_TYPE_CUSTOM)
	{
		hash = anode->func(Param, anode->value);
	}
	else if(anode->type == ALG_TYPE_PMASK)
	{
		uint64_t qword = anode->value;
		int idx;
		uint8_t * oct;

		hash = 0;
		while(qword && ((idx = __builtin_ctzll(qword)) < Param->ParamCount))
		{
			int off;
			int len = 256;
			uint8_t data[256];
			
			len = Param->GetRaw(idx, data, len);
			for(off=0;off<len;off++)
			{
				hash = hash * 131 + data[off];
			}
			qword &= ~(1 << idx);
		}
	}
	else	// ALG_TYPE_FIXED
	{
		hash = (unsigned long)anode->value;
	}
	hash = hash % NCHMap.MaxNodes;
	
	// search connection node from 'NCHTree'
	CRB3_CONNECTION * cnode;
	IRBT_NODE * inode;
	bool loop = false;
	inode = irbtPredecessor(&NCHTree, hash + 1);
	if(!inode)
	{
		loop = true;
		inode = irbtLast(&NCHTree);
	}
	while(inode &&
		(cnode = (CRB3_CONNECTION *)((unsigned long)inode - offsetof(CRB3_CONNECTION, node_nch))) &&
		cnode->Status != CRB3_STATUS_WORKING)
	{
		inode = irbtPrev(inode);
		if(!inode && !loop)
		{
			loop = true;
			inode = irbtLast(&NCHTree);
		}
	}
	return inode ? cnode : NULL;
}

int CRB3Client::_NCHFree(void)
{
	CRB3_NCH_ALG_NODE * anode = (CRB3_NCH_ALG_NODE *)ihashNext(&NCHAlg, NULL);
	CRB3_NCH_ALG_NODE * anext;
	
	while(anode)
	{
		anext = (CRB3_NCH_ALG_NODE *)ihashNext(&NCHAlg, &anode->node);
		free(anode);
		anode = anext;
	}
	ihashFree(&NCHAlg);
	return CRB3_OK;
}

/********************************************************
* 函数名: CRB3Client::PacketSink
*
* 功能描述: 处理报文接收
*
* 参数:
*      PacketBuf  - (输入) 待处理的报文数据缓冲
*      PacketSize - (输入) 待处理的报文数据长度，单位: 字节
*      FromAddr   - (输入) 报文来源地址
*      FromSocket - (输入) 报文来源套接字
*
* 返回值:
*      SOERR_OK - 处理成功，报文缓冲可以被释放
********************************************************/
int CRB3Client::PacketSink(IN uint8_t * PacketBuf, IN int PacketSize, IN SO_ADDRESS * FromAddr, IN int FromSocket, IN void * Spare)
{
	return _PacketProcess(FromAddr, (char *)PacketBuf, PacketSize);
}

/********************************************************
* 函数名: CRB3Client::ReceiveThread
*
* 功能描述: 报文接收线程
*
* 参数:
*      arg - (输入) CRB3客户端对象指针
*
* 返回值: CRB3_OK - 正常退出
*      
********************************************************/
void * CRB3Client::ReceiveThread(IN void * arg)
{
	CRB3Client * client_ref = (CRB3Client *)arg;

	return (void *)(long)client_ref->_ReceiveLoop();
}

/********************************************************
* 函数名: CRB3Client::CheckThread
*
* 功能描述: 服务端状态检查线程
*
* 参数:
*      arg - (输入) CRB3客户端对象指针
*
* 返回值: CRB3_OK - 正常退出
*      
********************************************************/
void * CRB3Client::CheckThread(IN void * arg)
{
	CRB3Client * client_ref = (CRB3Client *)arg;

	return (void *)(long)client_ref->_CheckLoop();
}

#ifdef NAMESPACE
}
#endif
