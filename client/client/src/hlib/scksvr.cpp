/*
 * scksvr.cpp
 *
 *  Created on: 2013-9-26
 *      Author: root
 */

#ifndef SCKSVR_CPP_
#define SCKSVR_CPP_

#include "scksvr.hpp"
#include "logout.hpp"
#include "global.hpp"
#include "../proj/wdctl.hpp"

typedef string* pstring;

int H_BUFFER_LEN = DEF_BUFF_LEN;
extern int LogLevel;
extern bool UseLog;
// initializations

namespace scksvr
{
	class TUnitController
	{
		public:
		TUnitController();
		~TUnitController();
	};

	TUnitController::TUnitController()
	{
		// Initialization
	}

	TUnitController::~TUnitController()
	{
		// Finalization();
	}

	TUnitController ThisUnit;
}

//{* TSWSockSvr *}

void THSockContext::Close()
{
	((THSockServer*)this->Server)->CloseContext(this);
}

THSockContext::THSockContext(THEpollServer* FSocketEngine):THContext(FSocketEngine)
{
  SessionStatus = ssWaitHead;
  //Inherited Create(FSocketEngine);
  ScriptTick = 0;
  Request = new THRequest();
  Request->PacketLength = 0;
  Request->FormLength = 0;
  Request->Buffer = new THCB(H_BUFFER_LEN);
  Request->Head = (pByte)malloc(((THSockServer*)Server)->head_len);
  Request->Body = (pByte)malloc(((THSockServer*)Server)->body_max);
  Request->Content = (pByte)malloc(((THSockServer*)Server)->content_size);
  Response = new THResponse();
  Response->ResponseCode = 200;
  Response->AbortGZIP = false;
  Page.Clear();
}

THSockContext::~THSockContext()
{
  delete Request->Head;
  delete Request->Body;
  delete Request->Buffer;
  delete Request->Content;
  Request->Buffer = NULL;
  delete Request;
  delete Response;
}

//{* THSockServer *}

THContext* THSockServer::CreateClientContext(THEpollServer* es)
{
  THSockContext* c = new THSockContext(es);
  return (THContext*)c;
}

THContext* THSockServer::GetContext(int Index)
{
  if (ContextMgr == NULL) return NULL;
  THContext* c = ContextMgr->Contexts[Index];
  return ((c != NULL) && (c->Connected)) ? c : NULL;
}

int THSockServer::GetContextCount()
{
  return ContextMgr->Size; // must loop from 1 - count
}

void THSockServer::InitOption()
{
  Log("^THSockServer::InitOption()..", H_WARNING);
  Option.Net.ServerIndex = new THHMSI(16);
  Option.Net.MaxConnection = 16;
  Option.Net.MaxContext = 32;
  Option.Net.MaxBuffer = 256;
  Option.Net.MaxSession = 32;
  Option.Net.MaxCache = 128;
  Option.Net.SessionLife = 300000;
  Option.Log.UseLog = true;
  Option.Log.LogLevel = 5;
  Option.Log.LogVolumeSize = 40960;
  Option.Log.LogTotalSize = 16384;
  Option.Other.audit_serv = "122.224.64.245:3001";
  Option.Other.auth_serv = "122.224.64.245:3000";
  Option.Other.whitelist = ".alipay.com;.qq.com;.weibo.com;.sina.com.cn;.baidu.com";
  Option.Other.def_302 = "http://122.224.64.245/api10/login?prot_type=1&";

  Option.Other.hard_id = 100;
  Option.Other.do_auth = false;
  Option.Other.should_auth = true;
  Option.Other.do_audit = false;
  Option.Other.should_audit = false;
  Option.Other.dev_id = "";
  Option.Other.ap_interval = 30;
  Option.Other.user_interval = 30;
  Option.Other.audit_interval = 30;
  #ifdef H_GZIP
  Option.Other.UseGZIP = true;
  #else
  Option.Other.UseGZIP = false;
  #endif
  Option.Other.interface = "br-lan";
  char *cp = get_iface_mac(Option.Other.interface.c_str());
  if(cp)
  {
	  Option.Other.MAC = string(cp);
	  free(cp);
  }else
	  Option.Other.MAC = "FEDCBA987663";
  LoadOption();
  Log("$THSockServer::InitOption()..", H_WARNING);
}

bool THSockServer::SaveConfig(string s)
{
    // do not exists, regenarate it
    TStringList t;
    try {
     t.Add("; eWiFi default config file.");
     t.Add("; Just remove it if you want a default config,");
     t.Add(";  stinger will regenarate this configuration.");
     t.Add("; Written by Masanari, hibiki at Jun, 3rd 2013 Hangzhou");
     t.Add("; All rights reserved.");
     t.Add("");
     t.Add("; All sections and key names are case sensitive !!");
     t.Add("");
     t.Add("[Log]");
     if (Option.Log.UseLog)
      t.Add("UseLog=True");
     else
      t.Add("UseLog=False");
     t.Add("LogLevel=" + IntToStr(Option.Log.LogLevel));
     t.Add("LogVolumeSize=" + IntToStr(Option.Log.LogVolumeSize));
     t.Add("LogTotalSize=" + IntToStr(Option.Log.LogTotalSize));
     t.Add("");
     t.Add("[Net]");
     t.Add("SessionLife=" + IntToStr(Option.Net.SessionLife));
     t.Add("; max concurrent incoming connection quantity");
     t.Add("MaxContext=" + IntToStr(Option.Net.MaxContext));
     t.Add("MaxConnection=" + IntToStr(Option.Net.MaxConnection));
     t.Add("; max buffer for input and output, must twice larger than connection");
     t.Add("MaxBuffer=" + IntToStr(Option.Net.MaxBuffer));
     t.Add("MaxSession=" + IntToStr(Option.Net.MaxSession));
     t.Add("; max concurrent output connection route");
     t.Add("MaxCache=" + IntToStr(Option.Net.MaxCache));
     t.Add("");
     t.Add("[Other]");
     t.Add("; DO NOT MODIFY THIS, FOR VERSION CHECKING AND COPYRIGHT");
     t.Add(";Root=" + ThisPath());
     if (Option.Other.UseGZIP)
      t.Add("UseGZIP=True");
     else
      t.Add("UseGZIP=False");
     t.Add("Default302=" + Option.Other.def_302);
     t.Add("AuditServ=" + Option.Other.audit_serv);
     t.Add("AuthServ=" + Option.Other.auth_serv);
     t.Add("WhiteList=" + Option.Other.whitelist);
     t.Add("Interface=" + Option.Other.interface);
     t.Add("Mac=" + Option.Other.MAC);

     t.Add("Hard_id=" + IntToStr(Option.Other.hard_id));
     t.Add("DevID=" + Option.Other.dev_id);
     t.Add("Auth=Y");
     t.Add("Audit=N");
     t.Add("APInterval=" + IntToStr(Option.Other.ap_interval));
     t.Add("UserInterval=" + IntToStr(Option.Other.user_interval));
     t.Add("AuditInterval=" + IntToStr(Option.Other.audit_interval));
     t.Add("");
     t.Add("; DO NOT MODIFY this section, unless you read all the source and really know");
     t.Add(";  what you are doing");
     t.Add("[System]");
     t.Add("TransferBlockSize=" + IntToStr(DEF_BUF_LEN));
     t.Add("MaxRLimitRequired=" + IntToStr(DEF_POLL_LEN));
     t.Add("EpollBatchQuantity=" + IntToStr(DEF_EVENT_LEN));
     t.SaveToFile(s);
     return true;
   } // save end
   catch(...)
   { // err
     cout << "Regenarate configuration [ " << s << " ] error: " << endl;
     cout << " continue start with default configuration parameters." << endl;
     return false;
   } // end
} // end func

bool THSockServer::LoadOption()
{
	bool ISaveFile = false;
  Log("^THSockServer::LoadOption()..", H_CAUTION);
  TStringList m;
  int i;
  // 1. load file options
  string s = ChangeFileExt(AppName(), ".ini");
  if (FileExists(s))
  {
	  #ifdef H_DEBUG
	  cout << "Loading options.." << endl;
	  #endif
	  if (!m.LoadFromFile(s))
	  {
		  cout << "Read configuration [ " << s << " ] error: " << strerror(errno) << endl;
		  cout << "continue start with default configuration parameters." << endl;
		  return false;
	  }
	  // 1.1 read options
	  s = m.ReadString("Log", "UseLog", "");
     #ifdef H_DEBUG
	  Option.Log.UseLog = true;
	  #else
	  Option.Log.UseLog = !s.compare("True");
	  #endif
	  s = m.ReadString("Log", "LogLevel", "");
	  if (TryStrToInt(s, i)) Option.Log.LogLevel = i;
	  LogLevel = Option.Log.LogLevel;
	  s = m.ReadString("Log", "LogVolumeSize", "");
	  if (TryStrToInt(s, i)) Option.Log.LogVolumeSize = i;
	  s = m.ReadString("Log", "LogTotalSize", "");
	  if (TryStrToInt(s, i)) Option.Log.LogTotalSize = i;

	  s = m.ReadString("Net", "SessionLife", "");
	  if (TryStrToInt(s, i)) Option.Net.SessionLife = i;
	  s = m.ReadString("Net", "MaxContext", "");
	  if (TryStrToInt(s, i)) Option.Net.MaxContext = i;
	  s = m.ReadString("Net", "MaxConnection", "");
	  if (TryStrToInt(s, i)) Option.Net.MaxConnection = i;
	  s = m.ReadString("Net", "MaxBuffer", "");
	  if (TryStrToInt(s, i)) Option.Net.MaxBuffer = i;
	  s = m.ReadString("Net", "MaxSession", "");
	  if (TryStrToInt(s, i)) Option.Net.MaxSession = i;
	  s = m.ReadString("Net", "MaxCache", "");
	  if (TryStrToInt(s, i)) Option.Net.MaxCache = i;

	  Option.Other.def_302 = m.ReadString("Other", "Default302", Option.Other.def_302);
	  Option.Other.auth_serv = m.ReadString("Other", "AuthServ", Option.Other.auth_serv);
	  Option.Other.audit_serv = m.ReadString("Other", "AuditServ", Option.Other.audit_serv);
	  Option.Other.whitelist = m.ReadString("Other", "WhiteList", Option.Other.whitelist);
	  Option.Other.interface = m.ReadString("Other","Interface",Option.Other.interface);
	  Option.Other.MAC = m.ReadString("Other","Mac",Option.Other.MAC);
	  char *cp = get_iface_mac(Option.Other.interface.c_str());
	  if(cp)
	  {
		  if(string(cp) != Option.Other.MAC)
		  {
			  Option.Other.MAC = string(cp);
			  ISaveFile = true;
		  }
		  free(cp);
	  }

	  s = m.ReadString("Other","Hard_id","");
	  if(TryStrToInt(s,i))Option.Other.hard_id = i;
	  Option.Other.dev_id = m.ReadString("Other", "DevID", Option.Other.dev_id);
	  Option.Other.should_auth = m.ReadString("Other", "Auth", "Y") != "N";
	  Option.Other.should_audit = m.ReadString("Other", "Audit", "N") == "Y";
	  s = m.ReadString("Other", "APInterval", "");
	  if (TryStrToInt(s, i)) Option.Other.ap_interval = i;
	  s = m.ReadString("Other", "UserInterval", "");
	  if (TryStrToInt(s, i)) Option.Other.user_interval = i;
	  s = m.ReadString("Other", "AuditInterval", "");
	  if (TryStrToInt(s, i)) Option.Other.audit_interval = i;
	  // parse serv
	  int cnt = 0;
	  for(int k = 0; k < 16; k++) Option.Net.Servers[k].stat = ssUnknown;
	  TStringList* tmp = new TStringList();
	  if (Option.Other.should_auth)
	  { // check
		  Split(Option.Other.auth_serv, ";", *tmp);
		  for(int k = 0; k < tmp->Count(); k++)
		  {
			  string svr = tmp->Lines(k);
			  Trim(svr);
			  if (svr.length() > 0)
			  {
				  int j = PosChar(':', (pByte)svr.c_str(), svr.length());
				  if (j > 0)
				  {
					  string p = RightStr(svr, svr.length() - j);
					  if (TryStrToInt(p, Option.Net.Servers[cnt].port))
					  {
						  Option.Net.Servers[cnt].ip = LeftStr(svr, j - 1);
						  Option.Net.Servers[cnt].type = stManage;
						  Option.Net.Servers[cnt].stat = ssDisconnected;
						  Option.Net.Servers[cnt].date = GetTickCount() - 6000;
						  Option.Net.ServerIndex->AddS(svr, cnt);
						  if (cnt++ >= 16) break;
					  }
				  }
			  }
		  }
	  } // check end
	  if (Option.Other.should_audit)
	  { // check
		  tmp->Clear();
		  Split(Option.Other.audit_serv, ";", *tmp);
		  for(int k = 0; k < tmp->Count(); k++)
		   {
				  string svr = tmp->Lines(k);
				  Trim(svr);
				  if ((svr.length() > 0) && (cnt < 16))
				  {
					  int j = PosChar(':', (pByte)svr.c_str(), svr.length());
					  if (j > 0)
					  {
						  string p = RightStr(svr, svr.length() - j);
						  if (TryStrToInt(p, Option.Net.Servers[cnt].port))
						  {
							  Option.Net.Servers[cnt].ip = LeftStr(svr, j - 1);
							  Option.Net.Servers[cnt].type = stAudit;
							  Option.Net.Servers[cnt].stat = ssDisconnected;
							  Option.Net.ServerIndex->AddS(svr, cnt);
							  if (cnt++ >= 16) break;
						  }
					  }
				  }
		   }
	  } // check end
	  delete tmp;
	  // parse end;

	  s = m.ReadString("Other", "UseGZIP", "");
	  Option.Other.UseGZIP = (s == "True");

	  // 1.05 set lowlevel param

	  s = m.ReadString("System", "TransferBlockSize", "");
	  if (TryStrToInt(s, i)) H_BUF_LEN = i;
	  H_BUFFER_LEN = H_BUF_LEN;
	  s = m.ReadString("System", "MaxRLimitRequired", "");
	  if (TryStrToInt(s, i)) H_POLL_LEN = i;
	  s = m.ReadString("System", "EpollBatchQuantity", "");
	  if (TryStrToInt(s, i)) H_EVENT_LEN = i;

	  if(ISaveFile)
		  SaveConfig(ChangeFileExt(AppName(), ".ini"));

     return true;
  } // 1.1 end
  else
	  return SaveConfig(s);

  // 2. load parameter ??
  // FIX IT
} // end

void THSockServer::OnEpollExt()
{ // here, extend client mode control
  int sock; struct sockaddr_in server;
  int sc = Option.Net.ServerIndex->Count();
  Cardinal n = GetTickCount();
  for (int i = 0; i < sc; i++)
  { // check if
	  if ((Option.Net.Servers[i].stat == ssDisconnected) && ((n - Option.Net.Servers[i].date) > 5000))
	  { // need reconnect
		  Log("     Try connect to " + Option.Net.Servers[i].ip + ":" + IntToStr(Option.Net.Servers[i].port) + " ...");

		  sock = socket(AF_INET, SOCK_STREAM, 0);
		  if (sock == -1)
		  { // fail
			  Log("[oo] Failed to init the socket..", H_WARNING);
			  continue;
		  } // end

	  	  server.sin_addr.s_addr = inet_addr(Option.Net.Servers[i].ip.c_str());
	  	  server.sin_family = AF_INET;
	  	  server.sin_port = htons(Option.Net.Servers[i].port);
	  	  //int timeo = 3000;
	  	  //socklen_t len = sizeof(timeo);
	  	  //setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeo, len);
	  	  fcntl(sock, F_SETFL, O_NONBLOCK);

	  	  if ((connect(sock, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR) && (errno != EINPROGRESS))
	  	  { // connect failed
	  		  Log("[oo] Failed to connect the server..", H_WARNING);
	  		  close(sock);
	  		  continue;
	  	  } // connect end

	  	  epoll_event ev;
	  	  ev.events = EPOLLOUT;
	  	  ev.data.ptr = (void*)i;
	  	  if (epoll_ctl(kdpfd, EPOLL_CTL_ADD, sock, &ev) == SOCKET_ERROR)
	  	  { // reg epoll failed
	  		  Log("[xx] Failed to add to the epoll..", H_WARNING);
	  		  close(sock);
	  		  continue;
	  	  } // end

	  	  Option.Net.Servers[i].sock = sock;
	  	  Option.Net.Servers[i].stat = ssConnecting;
	  	  Log("[->] Added to epoll, stand by..");
	  } // end check of Disconnected sockets
  } // end
} // ext end;

int THSockServer::OnVerify(THPointer p, sockaddr_in* sa)
{
	Int64 sc = Option.Net.ServerIndex->Count();
	if (p >= sc) return -1;
	int i = (int)p;
	sa->sin_addr.s_addr = inet_addr(Option.Net.Servers[i].ip.c_str());
	sa->sin_family = AF_INET;
	sa->sin_port = htons(Option.Net.Servers[i].port);
	int fd = Option.Net.Servers[i].sock;
	epoll_ctl(kdpfd, EPOLL_CTL_DEL, fd, NULL);
	int err; int errlen = sizeof(err);
	if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, (socklen_t*)&errlen) == -1)
	{// error
		Option.Net.Servers[i].stat = ssDisconnected;
		Option.Net.Servers[i].date = GetTickCount();
		close(fd);
		Log("[xx] Cannot connect to peer. getsockopt failed.");
		return -1;
	}// end
	if (err != 0)
	{// error
		Option.Net.Servers[i].stat = ssDisconnected;
		Option.Net.Servers[i].date = GetTickCount();
		close(fd);
		Log("[xx] Connection failed: " + IntToStr(err));
		return -1;
	}// end
	Option.Net.Servers[i].stat = ssConnected;
	Option.Net.Servers[i].date = GetTickCount();
	return fd;
}

void THSockServer::LogConfig()
{
    // 3. output for check
  Log("$$ System starts now with such configurations: ", H_CAUTION);
  Log("", H_CAUTION);
  Log("LOG config >>", H_CAUTION);
  Log("  UseLog\t\t\t" + BoolToStr(Option.Log.UseLog, true), H_CAUTION);
//  UseLog =  BoolToStr(Option.Log.UseLog, true);
  Log("  LogLevel\t\t" + IntToStr(Option.Log.LogLevel), H_CAUTION);
  Log("  LogVolumeSize\t" + IntToStr(Option.Log.LogVolumeSize), H_CAUTION);
  Log("  LogTotalSize\t\t" + IntToStr(Option.Log.LogTotalSize), H_CAUTION);
  Log("", H_CAUTION);
  Log("Net config >>", H_CAUTION);
  Log("  SessionLife\t\t" + IntToStr(Option.Net.SessionLife), H_CAUTION);
  Log("  MaxContext\t\t" + IntToStr(Option.Net.MaxContext), H_CAUTION);
  Log("  MaxConnection\t" + IntToStr(Option.Net.MaxConnection), H_CAUTION);
  Log("  MaxBuffer\t\t" + IntToStr(Option.Net.MaxBuffer), H_CAUTION);
  Log("  MaxSession\t\t" + IntToStr(Option.Net.MaxSession), H_CAUTION);
  Log("  MaxCache\t\t" + IntToStr(Option.Net.MaxCache), H_CAUTION);
  Log("", H_CAUTION);
  Log("Other config >>", H_CAUTION);
  Log("  Default302\t\t" + Option.Other.def_302, H_CAUTION);
  Log("  WhiteList\t\t" + Option.Other.whitelist, H_CAUTION);
  Log("  AuthServer\t\t" + Option.Other.auth_serv, H_CAUTION);
  Log("  AuditServer\t\t" + Option.Other.audit_serv, H_CAUTION);
  Log("  UseGZIP\t\t" + BoolToStr(Option.Other.UseGZIP, true), H_CAUTION);
  Log("  DoAuth\t\t" + BoolToStr(Option.Other.should_auth), H_CAUTION);
  Log("  DoAudit\t\t" + BoolToStr(Option.Other.should_audit), H_CAUTION);
  Log("  Soft_id\t\t" + IntToStr(Option.Other.hard_id), H_CAUTION);
  Log("  Interface\t\t" + Option.Other.interface,H_CAUTION);
  Log("  MAC\t\t\t" + Option.Other.MAC,H_CAUTION);
  Log("", H_CAUTION);
  Log("System config >>", H_CAUTION);
  Log("  TransferBlockSize\t" + IntToStr(H_BUFFER_LEN), H_CAUTION);
  Log("  MaxRLimitRequired\t" + IntToStr(H_POLL_LEN), H_CAUTION);
  Log("  EpollBatchQuantity\t" + IntToStr(H_EVENT_LEN), H_CAUTION);
  Log("", H_CAUTION);
}

void THSockServer::Log(const string info, int level)
{
	#ifdef H_DEBUG
	#ifdef H_PRINT_LOG_DATE
	printf((FormatDateTime("hh:nn:ss", Now()) + " ").c_str());
	#endif
	printf(info.c_str());
	printf("\n");
	#endif
#ifdef	LOG_FILE_CONF
	if(Option.Log.UseLog)
		if(MyLog)
			MyLog->Log(info,level);
#endif
}

bool THSockServer::PowerOn()
{
  Log("THSockServer::PowerOn()..", H_DEADLY);
  if (Power) return false;
  #ifndef H_DEBUG
  if (Option.Log.UseLog)
  #endif
  {
    LogConfig();
  }
  if (!SessionManager)
	 SessionManager = new THSessionManager((int)Option.Net.MaxSession);
  Log("Start daemon...");
  if (StartServer(-1, Option.Net.MaxConnection, Option.Net.MaxBuffer, Option.Net.MaxCache))
  {
    // 0. init logger
    FPower = true;
    Log("Server start OK.", H_DEADLY);
    return true;
  }
  else
  {
    Log("Server start failed: " + LastError, H_DEADLY);
    FPower = false;
  }
  return false;
}

void THSockServer::OnStarted()
{
  FPower = true;
  Log("< GREAT > Server started OK at [ " + ThisPath() + " ].", H_DEADLY);
}

void THSockServer::PowerOff()
{
  if (FPower)
  {
    StopServer();
    FPower = false;
  }
}

void THSockServer::Patrol(THEpollServer* server, bool LowSpeedEvent)
{
  double d = Now();
  if ((d - FDate) > H_SEC)
  {
    FDate = d;
    Log(server->RunDiagnosis().c_str());
  }
}

void THSockServer::HandleUnix(const char*cmd,const char* body)
{
	Log(" CMD [ " + string(cmd) + " ] BODY :" + string(body));
}

void THSockServer::OnNew(THContext* FClientContext)
{
  Log("(++) New connection [ " + IntToStr(FClientContext->Socket) + " ] from [ " + FClientContext->PeerIP
  	  + " : " + IntToStr(FClientContext->PeerPort) + " ]..", H_CAUTION);
  THSockContext* cc = (THSockContext*)FClientContext;
  cc->local_seq = 0;
  cc->peer_seq = 0;
  cc->Response->AutoAcknowledge = false;
  cc->Response->AbortGZIP = false;
  cc->Connected = true;
  cc->Logined = false;
  cc->MAC = "";
  cc->IP = "";
}

void THSockServer::OnClose(THContext* FClientContext)
{
  Log("(--) Disconnected [ " + IntToStr(FClientContext->Socket) + " ] from [ " + FClientContext->PeerIP
  	  + " : " + IntToStr(FClientContext->PeerPort) + " ]..", H_CAUTION);
  // clear context for reuse
  THSockContext* cc = (THSockContext*)FClientContext;
  cc->SessionStatus = ssWaitHead;
  cc->Response->ResponseCode = 200;
  cc->Response->AbortGZIP = false;
  cc->Response->ResponseStream.Clear();
  cc->Connected = false;
  cc->Logined = false;
  if ((cc->Tag > -1) && (cc->Tag < Option.Net.ServerIndex->Count()))
  { // need reconnect
	  Option.Net.Servers[cc->Tag].stat = ssDisconnected;
	  Option.Net.Servers[cc->Tag].date = GetTickCount();
	  Log("[**] Mark channel " + IntToStr(cc->Tag) + " as disconnected, waiting batch reconnection.");
  } // simple end
}

bool THSockServer::SendBuffer(THContext* FClientContext, pByte src, Word srclen)
{
  return FClientContext->SendData(src, srclen);
}

PHRequest THSockServer::GetRequest(THContext* FClientContext)
{
  return ((THSockContext*)FClientContext)->Request;
}

void THSockServer::CloseContext(THContext* Context)
{
  THSockContext* cc = (THSockContext*)Context;
  cc->SessionStatus = ssWaitClose;
  cc->CloseSocket();
}

bool THSockServer::OnCheckPacketHead(THContext* FClientContext)
{
	PHRequest req = GetRequest(FClientContext);
    req->FormLength = 0;
	req->PacketLength = 0;
	return false;
}

bool THSockServer::OnCheckPacketBody(THContext* FClientContext)
{
	PHRequest req = GetRequest(FClientContext);
    // check CRC here, and expand gzip and so on
	req->ContentLength = 0;
	return false;
}

void THSockServer::RecvCompletedEvent(THContext* FClientContext)
{
	logout("coming into RecvCompleteEvent\n");
  bool checksign;
  string s, hname;

  // 1. add request stream;
  int j = FClientContext->RecvBuf->Size;
  PHRequest req = GetRequest(FClientContext);
  req->Buffer->WriteCirBuffer(FClientContext->RecvBuf->Buffer, j);

  #ifdef H_PRINT_RECV_PACKET_CE
  printf("<**> received %d bytes\n", j);
  #endif

  // 2. process head
  THSockContext* cc = (THSockContext*)FClientContext;
  PHSessionStatus ss = &cc->SessionStatus;

  // 2.5 record packet
  /*
  TStringList x;
  for (int k = 0; k < j; k++)
  {
	  x.Add(IntToStr(FClientContext->RecvBuf->Buffer[k]));
  }
  x.SaveToFile(ThisPath() + cc->PeerIP + "-" + FormatDateTime("yyyymmddhhnnss", Now()) + ".txt");
*/
  // 3. check status
  logout("coming into encode\n");
  do {
	  j = req->Buffer->m_nValidCount;
	  checksign = false;
	  switch (*ss)
	  {
	  	  case ssWaitHead:
	  	  {
	  		  if (j >= head_len)
	  		  {
	  			 checksign = true;
	  		     req->Buffer->GetBufferData(req->Head, head_len, 0);
	  		     if (OnCheckPacketHead(FClientContext))
	  		    	 *ss = ssWaitForm;
	  		     else
	  		    	 req->Buffer->HeadSafePosInc(1);
	  		  }
	  		  break;
	  	  }
	  	  case ssWaitForm:
	  	  {
	  		  if (j >= (req->PacketLength))
	  		  {
	  			  checksign = true;
	  			  if (req->FormLength < 1)
	  				  req->ContentLength = 0;
	  			  else
	  			  {
	  				  req->Buffer->GetBufferData(req->Body, req->FormLength, head_len);
	  				  req->ContentLength = req->FormLength;
	  			  }
	  			  if (OnCheckPacketBody(FClientContext))
	  			  {
	  				  req->Buffer->HeadSafePosInc(req->PacketLength);
	  				  *ss = ssProcessing;
	  				  ProcessRequest((THSockContext*)FClientContext);
	  				  // must manually call BuildResponse
	  			  }
	  			  else
	  				  req->Buffer->HeadSafePosInc(1);
	  			  *ss = ssWaitHead;
	  		  }
	  		  break;
	  	  }
	  	  default:
	  	  {

	  	  }
	  }
  } while (checksign);
}

void THSockServer::ProcessRequest(THSockContext* FClientContext)
{
  Log("(**) Sock Request [ " + IntToStr(FClientContext->Socket) + " ] from [ " + FClientContext->PeerIP
		  + " : " + IntToStr(FClientContext->PeerPort) + " ]: ", H_CAUTION);
  // dispatch here
  do {

   // wrote out static resource first

   // cannot be recognized
   FClientContext->Response->ResponseCode = 404;
  } while (false);
  BuildResponse(FClientContext);
}

void THSockServer::BuildResponse(THSockContext* FClientContext)
{
    FClientContext->SendData(FClientContext->Response->ResponseStream.Memory, FClientContext->Response->ResponseStream.Size);
    //FClientContext->SessionStatus = ssWaitOutput;
    FClientContext->SessionStatus = ssWaitHead;
}

// events

void OnNew_(THCustomEpollServer* server, THCustomContext* FClientContext)
{
	((THSockServer*)server)->OnNew((THContext*)FClientContext);
}

void OnClose_(THCustomEpollServer* server, THCustomContext* FClientContext)
{
	((THSockServer*)server)->OnClose((THContext*)FClientContext);
}

void RecvCompletedEvent_(THCustomEpollServer* server, THCustomContext* FClientContext)
{
    ((THSockServer*)server)->RecvCompletedEvent((THContext*)FClientContext);
}

THCustomContext* CreateClientContext_(THCustomEpollServer* es)
{
	return (THCustomContext*)((THSockServer*)es)->CreateClientContext((THSockServer*)es);
}

void Patrol_(THCustomEpollServer* server, bool LowSpeedEvent)
{
	((THSockServer*)server)->Patrol((THSockServer*)server, LowSpeedEvent);
}

void HeadleUnix_(THCustomEpollServer* server,const char* cmd,const char* body)
{
	((THSockServer*)server)->HandleUnix(cmd,body);
}

// events end

THSockServer::THSockServer():THEpollServer()
{
  MyLog = NULL;
  InitOption();
  //Inherited;
  head_len = 10;
  body_max = H_BUFFER_LEN;
  content_size = H_BUFFER_LEN << 1;

  Power = false;
 // InitOption();
  SessionManager = NULL;
  OnConnected = OnNew_;
  OnDisconnected = OnClose_;
  OnDataReceived = RecvCompletedEvent_;
  logout("OnDataReceived=%x,RecvCompletedEvent_=%x\n",OnDataReceived,RecvCompletedEvent_);
  OnCreateContext = CreateClientContext_;
  OnUnixHeadle = HeadleUnix_;
  OnPatrol = Patrol_;

  MyLog = new LogMin(Option.Log.LogTotalSize,Option.Log.file,Option.Log.LogLevel);
}

THSockServer::~THSockServer()
{
  if (FPower) PowerOff();
  if (SessionManager)
  {
	  delete SessionManager;
	  SessionManager = NULL;
  }
#ifdef	LOG_FILE_CONF
  	  delete MyLog;
#endif
}

void THSockServer::ClearSession()
{
  if (!SessionManager) return;
  int j = Option.Net.MaxSession - 1;
  Cardinal tt = Option.Net.SessionLife;
  Cardinal newtime = GetTickCount();
  try
  {
    for(int i = j; i > -1; i--)
      if (SessionManager->Sessions[i]->Used)
        if ((newtime - SessionManager->Sessions[i]->LastOp) > tt)
          SessionManager->DelSession(SessionManager->Sessions[i]->Token);
  }
  catch(...)
  {

  }
}

//{* THSessionManager *}

THSession* THSessionManager::GetSession(const string Token)
{
  int i;
  PHElement p = UsedSessionMap->FindS(Token);
  if (p) i = p->Value; else i = -1;
  if ((i > -1) && (i < SessionCount)) return Sessions[i]; return NULL;
}

THSession* THSessionManager::NewSession(const string Token)
{
  int i;
  string s = Token;
  if (s == "") s = NewToken();
  PHElement c = UsedSessionMap->FindS(Token);
  if (c) i = c->Value; else i = -1;
  if (i == -1)
    if (FreeSessionStack->Count() > 0)
    {
      PHSession p = *((PHSession*)(FreeSessionStack->Pop()));
      p->Token = Token;
      p->Used = true;
      p->Info->Clear();
      p->Data = NULL;
      p->DataLen = 0;
      p->LastOp = GetTickCount();
      UsedSessionMap->AddS(Token, p->Index);
      return p;
    }
    else
    {

    }
  else
    if (i < SessionCount)
      return Sessions[i];
  return NULL;
}

bool THSessionManager::DelSession(const string Token)
{
  int i;
  PHElement c = UsedSessionMap->FindS(Token);
  if (c) i = c->Value; else i = -1;
  if (i > -1)
  {
	 THSession* p = Sessions[i];
     p->Token = "";
     p->Info->Clear();
     if (p->DataLen > 0) free(p->Data);
     p->DataLen = 0;
     p->Data = NULL;
     p->Used = false;
     p->LastOp = 0;
     FreeSessionStack->Push(p);
     return true;
  }
  return false;
}

THSessionManager::THSessionManager(int ASessionCount)
{
  SessionCount = ASessionCount;
  Sessions = new PHSession[SessionCount];
  for (int i = 0; i < SessionCount; i++)
  {
    Sessions[i] = new THSession();
    Sessions[i]->Index = i;
  }
  UsedSessionMap = new THHMSI(ASessionCount);
  FreeSessionStack = new THSP(ASessionCount * 2);
  for (int i = 0; i < SessionCount; i++)
  {
	  FreeSessionStack->Push(Sessions[i]);
  }
}

THSessionManager::~THSessionManager()
{
  for(int i = 0; i < SessionCount; i++) delete Sessions[i];
  delete [] Sessions;
  delete FreeSessionStack;
  UsedSessionMap->Clear();
  delete UsedSessionMap;
}

string THSessionManager::NewToken()
{
  string s = RandomString();
  while (UsedSessionMap->FindS(s)) s = RandomString();
  return s;
}

//{* TMSCGISession *}

THSession::THSession()
{
  Info = new TStringList();
  Data = NULL;
  Used = false;
  Token = "";
  Index = -1;
  DataLen = 0;
}

THSession::~THSession()
{
  if (Info)
  {
	  Info->Clear();
	  delete Info;
	  Info = NULL;
  }
  if (Data)
  {
	  DataLen = 0;
	  free(Data);
	  Data = NULL;
  }
}

#endif /* SCKSVR_CPP_ */
