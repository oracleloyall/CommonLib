/*
 * scksvr.hpp
 *
 *  Created on: 2013-9-26
 *      Author: root
 */

#ifndef SCKSVR_HPP_
#define SCKSVR_HPP_

#include "global.hpp"
#include "hclasses.hpp"
#include "hhmii.hpp"
#include "hhmsi.hpp"
#include "hepoll.hpp"
#include "hdateutils.hpp"
#include "hpasutils.hpp"
#include "hepoll.hpp"
#include "hpcb.hpp"
#include "logout.hpp"
#ifdef H_GZIP
#include "hzlib.hpp"
#endif

const string MSCGI_VER = "1.2";
const string MSCGI_ENGINE = "Masanari Stinger";

const int H_COMMON = 5;
const int H_CAUTION = 4;
const int H_WARNING = 3;
const int H_EMERGENCY = 2;
const int H_DEADLY = 1;

const int DEF_BUFF_LEN = 4096;

enum THSessionStatus {ssWaitHead, ssWaitForm, ssProcessing, ssWaitOutput, ssWaitClose};
typedef THSessionStatus* PHSessionStatus;

//{* Server Options *}

typedef Byte TServerType;
const TServerType stUnknown = 0;
const TServerType stManage = 1;
const TServerType stAudit = 2;

typedef Byte TServerStat;
const TServerStat ssUnknown = 0;
const TServerStat ssDisconnected = 1;
const TServerStat ssConnecting = 2;
const TServerStat ssConnected = 3;
const TServerStat ssAuthed = 4;

typedef struct THServer
{
  string ip;
  int port;
  int sock;
  TServerType type;
  TServerStat stat;
  Cardinal date;
} *PHServer;

typedef struct THNetOption
{
  THServer Servers[16];
  THHMSI*  ServerIndex;
  Cardinal SessionLife;
  Cardinal MaxContext;
  Cardinal MaxConnection;
  Cardinal MaxBuffer;
  Cardinal MaxSession;
  Cardinal MaxCache;
  int	   BufferLen;
} *PHNetOption;

typedef struct THOtherOption
{
  string def_302;
  string whitelist;
  string blacklist;
  string auth_serv;
  string audit_serv;
  string interface;
  string MAC;

  int	hard_id;
  bool  do_auth;
  bool  should_auth;
  bool  do_audit;
  bool  should_audit;
  string dev_id;
  bool  UseGZIP;
  int   ap_interval;
  int   user_interval;
  int   audit_interval;
} *PHOtherOption;

typedef struct THLogOption
{
  bool UseLog;
  int LogLevel;
  int LogVolumeSize;
  int LogTotalSize;
  string file;
} *PHLogOption;

class THOption
{
public:
	THNetOption Net;
	THLogOption Log;
	THOtherOption Other;
};

// {* Sessions *}

class THSession
{
public:
  TStringList* Info;
  int Index;
  int DataLen;
  void* Data;
  bool Used;
  string Token;
  Cardinal LastOp;
  THSession();
  virtual ~THSession();
};

typedef THSession* PHSession;

class THSessionManager
{
public:
  int SessionCount;
  PHSession* Sessions;
  THHMSI* UsedSessionMap;
  THSP* FreeSessionStack;
  string NewToken();
  THSession* GetSession(const string Token);
  THSession* NewSession(const string Token = "");
  bool DelSession(const string Token);
  THSessionManager(int ASessionCount);
  ~THSessionManager();
};

//{* Connection contexts *}

enum THServerProtocol {spTCP = 0, spUDP};

typedef struct THRequest
{
  int ContentLength;
  Word RemotePort;
  Word ServerPort;
  THServerProtocol ServerProtocol;

  string RemoteAddress;

  // * packet parameters **
  int FormLength;
  int PacketLength;
  // * extended for binary form
  pByte Head;
  pByte Body;
  pByte Data; // temp. not alloc
  pByte Content;
  THCB* Buffer;
  // * extended OK
} *PHRequest;

typedef struct THResponse
{
  int ResponseCode;
  TMemoryStream ResponseStream;
  bool AbortGZIP;
  bool AutoAcknowledge;
} *PHResponse;

class THSockContext:public THContext
{
  public:
	 THSessionStatus SessionStatus;
	 PHRequest Request;
	 PHResponse Response;
    TStringList Page;
    Int64 ScriptTick;
    // add for ack
    Cardinal local_seq;
    Cardinal peer_seq;
    bool Logined;
    string MAC;
    string IP;
    // add end
    void Close();
    THSockContext(THEpollServer* FSocketEngine);
    virtual ~THSockContext();
};

//{* main server *}

class THSockServer:public THEpollServer
{
private:
  bool FPower;
  void FillHeadVars(PHRequest req, THContext* FClientContext);
protected:
  double FDate;
  double ActionDate;
  double ListDate;
  double UsrDate;
  double DevDate;
  void InitOption();
  bool LoadOption();
  void LogConfig();
  LogMin* MyLog;

  PHRequest GetRequest(THContext* FClientContext);
  bool SendBuffer(THContext* FClientContext, pByte src, Word srclen);
  virtual void ProcessRequest(THSockContext* FClientContext);
  virtual void BuildResponse(THSockContext* FClientContext);
  void PrintInfo(THSockContext* FClientContext);

public:
  // data
  int head_len;
  int body_max;
  int content_size;
  // end
  THOption Option;
  //Terminated: Boolean;
  THSessionManager* SessionManager;
  THSockServer();
  virtual ~THSockServer();
  void OnStarted();
  void OnEpollExt();
  int OnVerify(THPointer p, sockaddr_in* sa);
  void PowerOff();
  bool PowerOn();
  bool WriteRes(THSockContext* FClientContext, const string uri);
  bool SaveConfig(string s);
  void CloseContext(THContext* Context);
  void ClearSession();
  void Log(const string info, int level = 5);
  bool Power;
  //int ContextCount();
  int GetContextCount();
  THContext* GetContext(int Index);
  THContext* Contexts(int Index);
  // events for bind
  virtual void OnNew(THContext* FClientContext);
  virtual void OnClose(THContext* FClientContext);
  virtual bool OnCheckPacketHead(THContext* FClientContext);
  virtual bool OnCheckPacketBody(THContext* FClientContext);
  void RecvCompletedEvent(THContext* FClientContext);
  virtual void Patrol(THEpollServer* server, bool LowSpeedEvent);
  virtual void HandleUnix(const char*cmd,const char* body);
  THContext* CreateClientContext(THEpollServer* es);
  // events end
};

#endif /* MHTTP_HPP_ */
