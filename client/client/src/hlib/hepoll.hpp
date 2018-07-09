/*
 * hepoll.hpp
 *
 *  Created on: 2013-9-25
 *      Author: root
 */

#ifndef HEPOLL_HPP_
#define HEPOLL_HPP_

#include "global.hpp"
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <sys/resource.h>
#include "hcache.hpp"
#include "hpasutils.hpp"
#include "hdateutils.hpp"

const int DEF_BUF_LEN = 4096;
const int DEF_POLL_LEN = 2000;
const int DEF_EVENT_LEN = 32;

const int SOCKET_ERROR = -1;
//const int EAGAIN = 11;

extern int H_BUF_LEN;
extern int H_POLL_LEN;
extern int H_EVENT_LEN;

class THCustomContext
{
public:
	virtual ~THCustomContext() {};
};

class THCustomEpollServer
{
public:
	virtual ~THCustomEpollServer() {};
};

typedef THCustomContext* (*TCreateContextEvent)(THCustomEpollServer* es);
typedef void (*TCommonContextEvent)(THCustomEpollServer* server, THCustomContext* context);
typedef void (*TCommonServerEvent)(THCustomEpollServer* server, bool LowSpeedEvent);
typedef void (*TCustomEpollExt)(THCustomEpollServer* server);
typedef void (*TCommonServeUdp)(THCustomEpollServer* server,const char*cmd,const char*body);

class THBuffer
{
  public:
	THBuffer* Next;
	int UID;
	int Socket;
	THCustomContext* Context;
	pByte Buffer;
	pByte Position;
    int Size;
    int FullLength;
    bool PostClose;
    bool AutoRelease;
    void Init();
    void AddData(pByte buf, int len);
    THBuffer(int Len = 0);
    ~THBuffer();
};

class THBufferMgr
{
 public:
  int Size;
  int Count;
  THBuffer* Current;
  THBuffer* GetBuffer();
  THBuffer* GetBufferEx(int len);
  void PutBuffer(THBuffer* buf);
  void PutBufferEx(THBuffer* buf_head, THBuffer* buf_tail, int len);
  THBufferMgr(int ASize);
  ~THBufferMgr();
};

class THContext:public THCustomContext
{
public:
	THContext* Next;
	int UID;
	int Tag;
	int Index;
	int Socket;
	string PeerIP;
	unsigned int PeerIP2;
    Word PeerPort;
    bool Using;
    bool PostClose; // close later
    bool Connected;
    bool Killed;
    bool Firing;
    double ConnectDate;
    Int64 ConnectTick;
    double LastDate;
    THCustomEpollServer* Server;
    THBuffer* RecvBuf;
    THBuffer* SendBuf;
    void KillSocket();
    void CloseSocket();
    THBuffer* AllocBuffer();
    bool SendBuffer(THBuffer* buf);
    bool SendData(pByte buf, int len);
    bool SendDataAndPostClose(pByte buf, int len);
    void SetSocket(int ASocket, sockaddr_in* Addr);
    THContext(THCustomEpollServer* AServer);
    virtual ~THContext();
};

typedef THContext* PHContext;

class THContextMgr
{
public:
  int Size;
  int Count;
  THContext* Current;
  THCustomEpollServer* Server;
  THContext** Contexts;
  int ActiveContextCount();
  THContext* GetContext();
  void PutContext(THContext* context);
  THContextMgr(THCustomEpollServer* AServer, int ASize);
  ~THContextMgr();
};

class THEpollServer:public THCustomEpollServer
{
public:
  int kdpfd;
  int listener;
  int UNIXfd;
  bool Active;
  string LastError;
  THBufferMgr* BufferMgr;
  THContextMgr* ContextMgr;
  THQCache* cache;
  double StartDate;
  Int64 IncomingData;
  Int64 OutgoingData;
  int IncomingConn;
  int Disconnected;
  Int64 ReceivedBlock;
  Int64 SentBlock;
  epoll_event* Events;
  TCommonContextEvent OnConnected;
  TCommonContextEvent OnDisconnected;
  TCommonContextEvent OnDataReceived;
  TCreateContextEvent OnCreateContext;
  TCommonServerEvent OnPatrol;
  TCommonServeUdp OnUnixHeadle;
  bool Terminated;
  virtual void OnStarted();
  virtual void OnEpollExt();
  virtual int OnVerify(THPointer p, sockaddr_in* sa);
  void SimpleServerExecute(bool active = false);
  int WriteSocket(PHCacheBlock block);
  string RunDiagnosis();
  bool SendBuffer(THBuffer* buf);
  void ReadUNIXIPC(int fd,THCustomEpollServer* server);
  bool StartServer(int Port, int Connection, int Buffer, int SendCache);
  void StopServer();
  THEpollServer();
  virtual ~THEpollServer();
};

#endif /* HEPOLL_HPP_ */
