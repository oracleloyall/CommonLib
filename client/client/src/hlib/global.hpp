/*
 * global.hpp
 *
 *  Created on: 2013-9-22
 *      Author: root
 */

#ifndef GLOBAL_HPP_
#define GLOBAL_HPP_

// basic compile options
#define	key_id 1677722705
#define		H3C_TEST
#define		H3C_COMD_LINE
#ifdef	H3C_COMD_LINE
#define	USING_H3C_API
#endif
//#define H_OPENWR_MAC
//#define H3_					//调用类H3C，API接口
#define AP_NAME            	//需要使用ap_name相关命令
#define	ADD     				//后添加内容,用于匹配配置选项和需要配置wan、wlan口比对
#define		USER_INTERVAL		//开启终端用户汇报的心跳时间，单位秒，默认为30秒
//#define		ADD_TEMPLATENAME
//#define SET_LIMIT_W			//开启带宽设置功能
#define		POWER
#define		THIS_				//
#define  TEMPLATENAME			//开启templatename比对
#ifndef		TEMPLATENAME
#define		MAC
#endif

#define UNIX_IPC
#define H_SAFE_MODE

//#define H_DEBUG

#ifdef H_DEBUG

// debug summary
//#define H_PRINT_MEMORY
//#define H_PRINT_NETWORK
// debug summary end

// debug print
#define H_PRINT_LOG_DATE
#define H_PRINT_RECV_PACKET
//#define H_PRINT_RECV_PACKET_CE
#define H_PRINT_SEND_PACKET
#define H_PRINT_ACK
#define H_PRINT_UPGOING_JMS
// debug print end

#endif

//; will defined by build template,

#define H_GZIP
//#define H_ICONV

// platform options

#ifdef H_MIPS
#define H_EB
#undef H_ICONV
#endif

#ifdef __LP64__
#define H_64BIT
#else
#ifdef __amd64
#define H_64BIT
#else
#ifdef _WIN64
#define H_64BIT
#endif
#endif
#endif

#include <string>
#include <iostream>
#include <cstring>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <malloc.h>

#include <sys/time.h>
#include <sys/statfs.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <libgen.h>
#include <pthread.h>
#include <fcntl.h>
#include <list>
#include <math.h>
#include <dirent.h>
#include <stdint.h>

using namespace std;

typedef int Integer;
typedef int64_t Int64;
typedef int64_t* pInt64;
typedef uint64_t UInt64;
typedef uint64_t* pUInt64;
typedef uint Cardinal;
typedef Cardinal* pCardinal;
typedef unsigned short int Word;
typedef Word* pWord;
typedef unsigned char Byte;
typedef Byte* pByte;
typedef double Double;
typedef Double* pDouble;

#endif /* GLOBAL_HPP_ */
