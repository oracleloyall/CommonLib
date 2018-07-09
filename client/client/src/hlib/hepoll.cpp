/*
 * hepoll.cpp
 *
 *  Created on: 2013-9-25
 *      Author: root
 */

#ifndef HEPOLL_CPP_
#define HEPOLL_CPP_

#include "hepoll.hpp"
#include "logout.hpp"
#include "../proj/wdctl.hpp"
#include <sys/socket.h>

#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>

int H_BUF_LEN = DEF_BUF_LEN;
int H_POLL_LEN = DEF_POLL_LEN;
int H_EVENT_LEN = DEF_EVENT_LEN;

//{* THBuffer *}

THBuffer::THBuffer(int Len)
{
  AutoRelease = true;
  Buffer = NULL;
  FullLength = (Len > 0) ? Len : H_BUF_LEN;
  Buffer = (pByte)malloc(FullLength);
  Init();
}

void THBuffer::Init()
{
  Position = Buffer;
  Size = 0;
}

void THBuffer::AddData(pByte buf, int len)
{
  Move(buf, Position, len);
  Inc(Size, len);
}

THBuffer::~THBuffer()
{
  if (Buffer) free(Buffer);
}

//{* THBufferMgr *}

THBufferMgr::THBufferMgr(int ASize)
{
  Size = ASize;
  Count = 0;
  Current = NULL;
  for(int i = 0; i < Size; i++)
  {
	THBuffer* b = new THBuffer(H_BUF_LEN);
    b->Next = Current;
    Current = b;
    Inc(Count);
  }
}

THBuffer* THBufferMgr::GetBuffer()
{
   if (!Current) return NULL;
   Dec(Count);
   THBuffer* ret = Current;
   Current = Current->Next;
   return ret;
}

THBuffer* THBufferMgr::GetBufferEx(int len)
{
   if (Count < len) return NULL;
   Dec(Count, len);
   THBuffer* b = Current;
   for (int i = 1; i < len; i++) b = b->Next;
   THBuffer* ret = Current;
   Current = b->Next;
   b->Next = NULL;
   return ret;
}

void THBufferMgr::PutBuffer(THBuffer* buf)
{
   buf->Next = Current;
   Current = buf;
   Inc(Count);
}

void THBufferMgr::PutBufferEx(THBuffer* buf_head, THBuffer* buf_tail, int len)
{
   buf_tail->Next = Current;
   Current = buf_head;
   Inc(Count, len);
}

THBufferMgr::~THBufferMgr()
{
  THBuffer* b;
  while (Current)
  {
    b = Current->Next;
    delete Current;
    Current = b;
  }
}

//{* THContext *}

THContext::THContext(THCustomEpollServer* AServer)
{
  Server = AServer;
  RecvBuf = ((THEpollServer*)Server)->BufferMgr->GetBuffer();
  RecvBuf->Init();
  RecvBuf->AutoRelease = false;
  SendBuf = NULL;
  // other
  Next = NULL;
  ConnectTick = 0;
  Firing = false;
  ConnectDate = 0;
  PostClose = false;
  Socket = 0;
  Killed = false;
}

void THContext::SetSocket(int ASocket, sockaddr_in* Addr)
{
  ConnectTick = GetMicroSecondsDraft();
  //ConnectTick = GetMicroSeconds();
  UID = random();
  Tag = -1;
  Socket = ASocket;
  PeerIP = string(inet_ntoa(Addr->sin_addr));
  PeerPort = ntohs(Addr->sin_port);
  PostClose = false;
  Connected = true;
  Killed = false;
  Firing = false;
  ConnectDate = Now();
  LastDate = ConnectDate;
}

void THContext::KillSocket()
{
   if (Killed) return;
   Inc(((THEpollServer*)Server)->Disconnected);
   Killed = true;
   PostClose = false;
   close(Socket);
   epoll_ctl(((THEpollServer*)Server)->kdpfd, EPOLL_CTL_DEL, Socket, NULL);
   try
   {
	 TCommonContextEvent cce = ((THEpollServer*)Server)->OnDisconnected;
	 if (cce) (*cce)(Server, (THCustomContext*)this);
   }
   catch(...)
   {

   }
   Connected = false;
   ((THEpollServer*)Server)->ContextMgr->PutContext(this);
	// 褰撹窡璁よ瘉骞冲彴鏂紑閾炬帴鏃�,搴旇娓呴櫎iptables瑙勫垯
   wdctl_down();
}

void THContext::CloseSocket() // for internal use
{
  if (Killed || PostClose) return;
  PostClose = true;
  shutdown(Socket, SHUT_WR);
  KillSocket();
}

THBuffer* THContext::AllocBuffer()
{
  return ((THEpollServer*)Server)->BufferMgr->GetBuffer();
}

bool THContext::SendBuffer(THBuffer* buf)
{
  return ((THEpollServer*)Server)->SendBuffer(buf);
}

bool THContext::SendData(pByte buf, int len)
{
  int j = (len - 1) / H_BUF_LEN + 1;
  THBuffer* b = ((THEpollServer*)Server)->BufferMgr->GetBufferEx(j);
	if (!b) {
		logout(" THContext::SendData: Malloc error\n");
		return false;
	}
  THBuffer* c = b;
  for (int i = 1; i < j; i++)
  {
    c->Init();
    c->Socket = Socket;
    c->Context = this;
    c->UID = UID;
    c->PostClose = false;
    c->AddData(buf, H_BUF_LEN);
    buf += H_BUF_LEN;
    c = c->Next;
  }
  c->Init();
  c->Socket = Socket;
  c->Context = this;
  c->UID = UID;
  c->PostClose = false;
  c->AddData(buf, (len - 1) % H_BUF_LEN + 1);
  if (SendBuffer(b)) return true;
  ((THEpollServer*)Server)->BufferMgr->PutBufferEx(b, c, j);
  return false;
}

bool THContext::SendDataAndPostClose(pByte buf, int len)
{
	int j = (len - 1) / H_BUF_LEN + 1;
	THBuffer* b = ((THEpollServer*)Server)->BufferMgr->GetBufferEx(j);
	if (!b) return false;
	THBuffer* c = b;
	for (int i = 1; i < j; i++)
	{
	    c->Init();
	    c->Socket = Socket;
	    c->Context = this;
	    c->UID = UID;
	    c->PostClose = false;
	    c->AddData(buf, H_BUF_LEN);
	    buf += H_BUF_LEN;
	    c = c->Next;
	}
	c->Init();
	c->Socket = Socket;
	c->Context = this;
	c->UID = UID;
	c->PostClose = true;
	c->AddData(buf, (len - 1) % H_BUF_LEN + 1);
	if (SendBuffer(b)) return true;
	((THEpollServer*)Server)->BufferMgr->PutBufferEx(b, c, j);
	return false;
}

THContext::~THContext()
{
	((THEpollServer*)Server)->BufferMgr->PutBuffer(RecvBuf);
}

//{* THContextMgr *}

THContextMgr::THContextMgr(THCustomEpollServer* AServer, int ASize)
{
  Size = ASize;
  Count = 0;
  Current = NULL;
  Server = AServer;
  Contexts = new PHContext[ASize];
  memset(Contexts, 0, sizeof(PHContext) * ASize);
  for (int i = 0; i < Size; i++)
  {
   try
   {
    THContext* b = (THContext*)(*(((THEpollServer*)Server)->OnCreateContext))(Server);
    Contexts[Count] = b;
    b->Index = Count;
    b->Next = Current;
    b->Using = false;
    Current = b;
    Inc(Count);
   }
   catch(...)
   {

   }
  }
}

THContext* THContextMgr::GetContext()
{
   if (!Current) return NULL;
   Dec(Count);
   THContext* ret = Current;
   Current->Using = true;
   Current = Current->Next;
   return ret;
}

void THContextMgr::PutContext(THContext* context)
{
   context->Next = Current;
   Current = context;
   Inc(Count);
   Current->Using = false;
}

int THContextMgr::ActiveContextCount()
{
   return Size - Count;
}

THContextMgr::~THContextMgr()
{
  for (int i = 0; i < Size; i++)
   if (Contexts[i])
	delete Contexts[i];
  delete [] Contexts;
}

// THContextMgr end

int cbWriteSocket(void* sender, PHCacheBlock block)
{
	return ((THEpollServer*)sender)->WriteSocket(block);
}
void THEpollServer::ReadUNIXIPC(int fd,THCustomEpollServer* server)
{
	char readbuf[1024] = "";
	struct sockaddr_un sa_un;
	socklen_t len = 0;
	bzero(&sa_un,sizeof(struct sockaddr_un));
	int size = recvfrom(fd,(void*)readbuf,1024 - 1,0,(struct sockaddr*)&sa_un,&len);
	if(size < 0)
		return;
	char* cmd = readbuf;
	char *cp = strstr(cmd," ");
	logout("Recv %s from UNIX\n",readbuf);
	OnUnixHeadle(server,cmd,cp);
}

void THEpollServer::SimpleServerExecute(bool active)
{ // vars
  int nfds, curfds, n, new_fd, ret, loops;
  sockaddr their_addr;
  epoll_event ev;
  THContext* context;
  THContextMgr* cm;
  // begin
  OnStarted();
  loops = 0;
  curfds = 32;
  cm = ContextMgr;
  socklen_t len = sizeof(sockaddr);
  Events = (epoll_event*)malloc(H_EVENT_LEN * sizeof(epoll_event));
  while (!Terminated)
  { // main process
						  //OnEpollExt scksvr.cpp check if need reconnect
    if (active && ((loops % 128) == 0)) OnEpollExt(); // add connection at 1st
    loops++;
    nfds = epoll_wait(kdpfd, Events, curfds, 5);
    if (nfds == -1) break; // epoll is broken
    for (n = 0; n < nfds; n++)
    { // check outgoing connection
    	if(UNIXfd == Events[n].data.fd)
    	{
    		ReadUNIXIPC(UNIXfd,this);
    		continue;
    	}
    	if (active)
    	{ // is client mode
    		THPointer m = (THPointer)(Events[n].data.ptr);
				if ((m < 0) || (m > 65535))
					goto ReadData;
				// 涓嶅彲鑳芥槸寰呯‘璁ock
    		struct sockaddr_in tmp_addr;
         int s = OnVerify(m, &tmp_addr);
         if (s == -1)
        	{ // check
        	  close(s);
					continue; // 纭娌℃湁閾炬帴涓�, 鐩稿叧鎾ら攢鎿嶄綔鍦∣nVerify閲岄潰灏卞仛濂�
        	} // end
         new_fd = s;
         // success, continue
         context = cm->GetContext();
         if (!context)
         { // context full, close
           close(new_fd);
           continue;
         } // set and notice
         context->SetSocket(new_fd, &tmp_addr);
         context->Tag = m;
         goto ContinueAdd;
    	} // end client mode
    	// check incoming connection
      if (Events[n].data.fd == listener)
      { // accept
        new_fd = accept(listener, &their_addr, &len);
        if (new_fd < 0)
          continue;
        else
        { // success, continue
          context = cm->GetContext();
          if (!context)
          { // context full, close
            close(new_fd);
            continue;
          } // set and notice
          context->SetSocket(new_fd, (sockaddr_in*)(&their_addr));
          fcntl(new_fd, F_SETFL, O_NONBLOCK);
          ContinueAdd:
          IncomingConn++;
          try
          {
						//发送 连接请求 回调
						// call event
        	   (*OnConnected)(this, context);
          } // for safe
          catch(...)
          { // do sth
          } // nth
          ev.events = EPOLLIN;   //ev.events := EPOLLIN or EPOLLET;
          ev.data.ptr = context;
          if (epoll_ctl(kdpfd, EPOLL_CTL_ADD, new_fd, &ev) == SOCKET_ERROR)
            context->KillSocket();
        } // end check of available socket
      } // end check of incoming
      else
      { // exists, let's read
    	  ReadData:
        context = (THContext*)(Events[n].data.ptr);
        ret = recv(context->Socket, context->RecvBuf->Buffer, H_BUF_LEN, 0);
        if (ret > 0)
        { // data arrived
            IncomingData += ret;
            ReceivedBlock++;
            context->LastDate = Now();
            context->RecvBuf->Size = ret;
			   #ifdef H_PRINT_RECV_PACKET
            printf("{>>} received %d bytes:\n", ret);
 /*           for(int z = 0; z < ret; z++)
            { // print
            	printf("%.2x ", context->RecvBuf->Buffer[z]);
            	if ((z % 16) == 15) printf("\n");
            } // print end
            printf("\n");*/
			   #endif
            try
            { // call event
            	logout("OnDateReceive=%x\n",OnDataReceived);
            	(*OnDataReceived)(this, context);
            } // end event
            catch(...)
            { // for safe
            } // nth
        } // check other error
        else if ((ret == SOCKET_ERROR) && (errno == EAGAIN))  // no data
        { // check special situation, no error but wait

        } // end
        else
        { // no special, disconnected
            context->KillSocket();
        } // end
      } // a single fd is OK
    } // end while, all fds are OK

    // 2. write
    do {
    	n = cache->Loop(this, &cbWriteSocket);
    } while (n);
    // 3. patrol
    try { // call event
    	(*OnPatrol)(this, !(loops & 128));
    } // end
    catch(...)
    { // for safe

    } // nth now
  } // endless while, FIXIT: should check signal here!!
  Terminated = true;
  free(Events);
  close(kdpfd);
  close(listener);
} // end function

int THEpollServer::WriteSocket(PHCacheBlock block)
{
  int ret; THBuffer* x;
  //Begin
  if (!block->data) return 0;
  THBuffer* buf = (THBuffer*)block->data;
  // precheck for too fast switching contexts
  if ((!buf->Context) || (!((THContext*)(buf->Context))->Connected) || (buf->UID != ((THContext*)(buf->Context))->UID))
  {
    ret = 1;
    x = buf;
    while (x->Next)
    {
      x = x->Next;
      Inc(ret);
    }
    BufferMgr->PutBufferEx(buf, x, ret);
    cache->Release(block);
    return 0;
  }
  // send data
  THContext* context = (THContext*)(buf->Context);
  ret = send(buf->Socket, buf->Position, buf->Size, MSG_NOSIGNAL + MSG_DONTWAIT);
  if (ret > 0) // success
  {
    context->LastDate = Now();

    #ifdef H_PRINT_SEND_PACKET
#if 0
    printf("{<<} send %d bytes:\n", buf->Size);
    for(int z = 0; z < buf->Size; z++)
     {
     	printf("%.2x ", buf->Position[z]);
     	if ((z % 16) == 15) printf("\n");
     }
     printf("\n");
#endif
	#endif

    OutgoingData += ret;
    if (ret == buf->Size) // whole
    {
      SentBlock++;
      if (buf->PostClose) context->CloseSocket();
      x = buf->Next;
      BufferMgr->PutBuffer(buf);
      if (!x)  // single
       cache->Release(block);
      else
       block->data = x;
      if (buf->PostClose)
      {
        buf->PostClose = false;
        context->KillSocket();
      }
      return 1;
    }
    else // part of
    {
      buf->Position += ret;
      buf->Size -= ret;
      return 0;
    }
  }
  else if ((ret == SOCKET_ERROR) && (errno == EAGAIN))
  {
	  return 0;
  }
  else
  {
	  context->KillSocket();
	  return 0;
  }
}

//{* THEpollServer *}

THEpollServer::THEpollServer()
{
	Active = false;
}

THEpollServer::~THEpollServer()
{
	if (Active) StopServer();
}

bool THEpollServer::SendBuffer(THBuffer* buf)
{
	return cache->Add(buf);
}

#ifdef  UNIX_IPC
static int create_unix_socket(const char *sock_name)
{
    struct sockaddr_un sa_un;
    int sock;

    memset(&sa_un, 0, sizeof(sa_un));
    if (strlen(sock_name) > (sizeof(sa_un.sun_path) - 1))
    {
        logout("WDCTL socket name too long\n");
        return -1;
    }

    sock = socket(PF_UNIX, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        logout( "Could not get unix socket: %s", strerror(errno));
        return -1;
    }
    if (fcntl(sock, F_SETFL, O_NONBLOCK) == SOCKET_ERROR)
    {
    	logout("fcntl UNIXIPC error\n");
    	close(sock);
    	return -1;
    }
    /* If it exists, delete... Not the cleanest way to deal. */
    unlink(sock_name);

    strcpy(sa_un.sun_path, sock_name);
    sa_un.sun_family = AF_UNIX;
    logout( "Binding socket (%s) (%d)\n", sa_un.sun_path, strlen(sock_name));
    /* Which to use, AF_UNIX, PF_UNIX, AF_LOCAL, PF_LOCAL? */
    if (bind(sock, (struct sockaddr *)&sa_un, sizeof(struct sockaddr_un)))
    {
        logout("Could not bind unix socket: %s\n", strerror(errno));
        close(sock);
        return -1;
    }
    return sock;
}
#endif

bool THEpollServer::StartServer(int Port, int Connection, int Buffer, int SendCache)
{
  rlimit rt;
  int opt;
  sockaddr_in my_addr;
  epoll_event ev;
  //Begin

  // 1. check env
  if (Active) return false;

  // 2. build res-managers
  try
  {
	  cache = new THQCache(SendCache);
  }
  catch(...)
  {
	  LastError = "Error occured when create cache manager. ";
      return false;
  }
  try
  {
	  BufferMgr = new THBufferMgr(Buffer);
  }
  catch(...)
  {
      LastError = "Error occured when create buffer manager.";
      delete cache;
      return false;
  }
  try
  {
	  ContextMgr = new THContextMgr(this, Connection);
  }
  catch(...)
  {
	  LastError = "Error occured when create context manager.";
      delete cache;
      delete BufferMgr;
      return false;
  }

  // 3. prepare socket
  rt.rlim_max = H_POLL_LEN;
  rt.rlim_cur = H_POLL_LEN;
  if (setrlimit(RLIMIT_NOFILE, &rt) == -1)
  {
    LastError = "Error occured when set rlimit.";
    delete cache;
    delete ContextMgr;
    delete BufferMgr;
    return false;
  }

  if (Port == -1)
  {
	 kdpfd = epoll_create(H_POLL_LEN);
	 goto Ready;
  }

  listener = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
  if (listener == SOCKET_ERROR)
  {
    LastError = "Error occured when create main listening socket.";
    delete cache;
    delete ContextMgr;
    delete BufferMgr;
    return false;
  }

  if (fcntl(listener, F_SETFL, O_NONBLOCK) == SOCKET_ERROR)
  {
    LastError = "Error occured when set asychronized socket. ";
    delete cache;
    delete ContextMgr;
    delete BufferMgr;
    close(listener);
    return false;
  }

  opt = 1;
  if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == SOCKET_ERROR)
  {
    LastError = "Error occured when set reuse address.";
    delete cache;
    delete ContextMgr;
    delete BufferMgr;
    close(listener);
    return false;
  }
  memset(&my_addr, 0, sizeof(my_addr));
  my_addr.sin_family = AF_INET;
  my_addr.sin_port = htons(Port);
  if (bind(listener, (const sockaddr*)(&my_addr), sizeof(my_addr)) == SOCKET_ERROR)
  {
    LastError = string("Error occured when bind address: ") + strerror(errno);
    delete cache;
    delete ContextMgr;
    delete BufferMgr;
    close(listener);
    return false;
  }

  if (listen(listener, 128) == SOCKET_ERROR)
  {
	    delete cache;
	    delete ContextMgr;
	    delete BufferMgr;
	    close(listener);
	    return false;
  }

  // 3.5 prepare threads

  // 4 create EPOLL
  kdpfd = epoll_create(H_POLL_LEN);
  ev.events = EPOLLIN;
  //ev.events := EPOLLIN or EPOLLET;
  ev.data.fd = listener;
  if (epoll_ctl(kdpfd, EPOLL_CTL_ADD, listener, &ev) == SOCKET_ERROR)
  {
    LastError = string("Error occured when create epoll: ") + strerror(errno);
    delete cache;
    delete ContextMgr;
    delete BufferMgr;
    close(kdpfd);
    close(listener);
    return false;
  }

  // 5 create
  Ready:
#ifdef  UNIX_IPC
  if((UNIXfd = create_unix_socket("/var/client.sock")) > 0)
  {
	  ev.data.fd = UNIXfd;
	  ev.events = EPOLLIN;
	  if (epoll_ctl(kdpfd, EPOLL_CTL_ADD, UNIXfd, &ev) == SOCKET_ERROR)
	    {
	      LastError = string("Error occured when create epoll: ") + strerror(errno);
	      delete cache;
	      delete ContextMgr;
	      delete BufferMgr;
	      close(kdpfd);
	      close(listener);
	      return false;
	    }
  }
#endif
  StartDate = Now();
  IncomingData = 0;
  OutgoingData = 0;
  IncomingConn = 0;
  Disconnected = 0;
  ReceivedBlock = 0;
  SentBlock = 0;

  Active = true;
  Terminated = false;
  SimpleServerExecute(Port == -1);
  Active = false;
  return true;
}

void THEpollServer::OnStarted()
{
  // do nothing
}

void THEpollServer::OnEpollExt()
{
  // do nothing
}

int THEpollServer::OnVerify(THPointer p, sockaddr_in* sa)
{
	return -1;
}

void THEpollServer::StopServer()
{
  if (!Active) return;
  delete ContextMgr;
  delete BufferMgr;
  delete cache;
  Active = false;
}

string THEpollServer::RunDiagnosis()
{
  stringstream ss;
  ss << "*** Masanari self-diagnosis *******************************\n";
  ss << "BASIC >>\n";
  ss << " Start Date: " << FormatDateTime("yyyy-mm-dd hh:nn:ss", StartDate) << "\n";
  ss << " Total Time: " << FormatTinyTime((Now() - StartDate) / ((double)1 / 24 / 60 / 60)) << "\n";
  ss << "RESOURCE >>\n";
  ss << " Buffer  ( free / total ): " << BufferMgr->Count << " / " << BufferMgr->Size << "\n";
  ss << " Context ( free / total ): " << ContextMgr->Count << " / " << ContextMgr->Size << "\n";
  ss << " Cache   ( free / total ): " << cache->fc << " / " << cache->size << "\n";
  ss << "PERFORMANCE >>\n";
  ss << " Connection ( disconnect / total ): " << Disconnected << " / " << IncomingConn << "\n";
  ss << " Data Block    ( sent / received ): " << SentBlock << " / " << ReceivedBlock << "\n";
  ss << " Network IO statitics ( IN / OUT ): " << IncomingData << " / " << OutgoingData << "\n";
  ss << "***********************************************************\n";
  return ss.str();
}

#endif /* HEPOLL_HPP_ */
