#include <functional>
#include "clstd.h"
#include "clString.h"
#include "thread/clThread.h"
#include "thread/clSignal.h"
#include "clRepository.h"

#if defined(_CL_SYSTEM_LINUX)
# include <errno.h>
# include <arpa/inet.h>
# include <unistd.h>
# include <algorithm> // std::for_each
#endif // #if defined(_CL_SYSTEM_LINUX)

#include "clSocket.h"
#include "clSocketServer.h"

#define SOCKET_ERROR_LOG(_STAT, _MSG) if (_STAT == SOCKET_ERROR) { CLOG_ERROR(_MSG); }

#if defined(_CL_SYSTEM_WINDOWS)
typedef int socklen_t;
#elif defined(_CL_SYSTEM_LINUX)
struct WSADATA
{
  void* xxx;
};

int WSAStartup(WORD ver, WSADATA* pData)
{
  return 0;
}

int WSACleanup()
{
  return 0;
}

int WSAGetLastError()
{
  return 0;
}

int closesocket(SOCKET s)
{
  ::close(s);
}
#endif

namespace clstd
{
  namespace net_sockets
  {
//#if defined(_CL_SYSTEM_WINDOWS)
    u32 g_nWSAStartup = 0;
    int Startup()
    {
      WSADATA Data;
      int result = WSAStartup(CLMAKEWORD(1, 1), &Data);
      if(result == 0) {
        g_nWSAStartup++;
      }
      else {
        CLOG_ERROR("ERROR: WSAStartup unsuccessful\r\n");
      }
      return result;
    }
    
    int Cleanup()
    {
      ASSERT(g_nWSAStartup);
      if(g_nWSAStartup)
      {
        int result = WSACleanup();
        if(result == 0) {
          g_nWSAStartup--;
        }
        else {
          CLOG_ERROR("ERROR: WSACleanup unsuccessful\r\n");
        }
        return result;
      }
      return -1;
    }

    b32 IsStartup()
    {
      return g_nWSAStartup;
    }

    //////////////////////////////////////////////////////////////////////////

    TCPClient::TCPClient()
      : m_socket (INVALID_SOCKET)
    {
    }

    TCPClient::TCPClient(SOCKET socket)
      : m_socket(socket)
    {
    }

    TCPClient::~TCPClient()
    {
    }

    SocketResult TCPClient::Connect(CLLPCSTR addr_port)
    {
      ASSERT(IsStartup());

      clStringA strAddrPort = addr_port;
      clStringA strAddr;
      clStringA strPort;
      strAddrPort.DivideBy(':', strAddr, strPort);
      return Connect(inet_addr(strAddr), strPort.ToInteger());
    }

    SocketResult TCPClient::Connect(CLLPCSTR addr, CLUSHORT port)
    {
      return Connect(inet_addr(addr), port);
    }

    SocketResult TCPClient::Connect(u32 addr, CLUSHORT port)
    {
      //WSADATA		Data;
      SOCKADDR_IN SockAddr;

      ASSERT(IsStartup());

      if(m_socket != INVALID_SOCKET) {
        CloseSocket();
      }
      //	
      //
      int status = 0;
      //int status = WSAStartup(CLMAKEWORD(1, 1), &Data);
      //if (status != 0) {
      //}

      // zero the sockaddr_in structure
      memset(&SockAddr, 0, sizeof(SockAddr));

      // specify the port portion of the address
      SockAddr.sin_port = htons(port);
      // specify the address family as Internet
      SockAddr.sin_family = AF_INET;
      // specify that the address does not matter
      SockAddr.sin_addr.s_addr = addr;

      // create a socket  socket(通信发生的区域,套接字的类型,套接字使用的特定协议)
      m_socket = ::socket(AF_INET, SOCK_STREAM, 0);
      if (m_socket == INVALID_SOCKET) {
        //_ChkWSACleanup(status);
        CLOG_ERROR("ERROR: socket unsuccessful\n");
        m_socket = INVALID_SOCKET;
        return SocketResult_CreateFailed;
      }

      // 阻塞模式下，返回的是尝试连接的结果，服务器应答不在这里体现
      status = ::connect(m_socket, (SOCKADDR*)&SockAddr, sizeof(SockAddr));
      if(status == SOCKET_ERROR) {
        //_ChkWSACleanup(status);
        CLOG_ERROR("ERROR: can not connect the server.\n");
        m_socket = INVALID_SOCKET;
        return SocketResult_CanotConnect;
      }
      //OnEvent(/*m_clientSocket, */SE_CONNECT);

      return SocketResult_Ok;
    }

    b32 TCPClient::IsInvalidSocket() const
    {
      return m_socket == INVALID_SOCKET;
    }

    SocketResult TCPClient::WaitSocket(long timeout_sec)
    {
      fd_set read_set;
      FD_ZERO(&read_set);
      FD_SET(m_socket, &read_set);

      timeval tv = {timeout_sec, 0};

      // 返回结果是read_set里需要处理的socket数量，这里肯定是1
      // 如果是0代表超时, SOCKET_ERROR表示出错
      int result = ::select(0, &read_set, 0, 0, timeout_sec ? &tv : 0);
      
      if(result > 0)
      {
        ASSERT(result == 1); // read_set 只填了1个
      }
      else if(result == 0)
      {
        //CLOG("socket timeout(%d).", result);
        return SocketResult_TimeOut;
      }
      else if(result == SOCKET_ERROR)
      {
        if(m_socket == INVALID_SOCKET)
        {
          CLOG("user disconnected(%d) ...", result);
        }
        else
        {
          CLOG("socket error : disconnected(%d:%d) ...", result, WSAGetLastError());
          CloseSocket();
        }
        return SocketResult_Disconnected;
      }
      else {
        CLOG_ERROR("Unexpected socket result(%d).", result);
        CLBREAK; // 意外的结果
        return SocketResult_UnknownError;
      }

      return SocketResult_Ok;
    }

    int TCPClient::CloseSocket()
    {
      int status = 0;
      if(m_socket != INVALID_SOCKET)
      {
        const SOCKET local_socket = m_socket;
        m_socket = INVALID_SOCKET;
        status = ::closesocket(local_socket);
        CLOG("closesocket(%d)", status);

        //_ChkWSACleanup(status);
      }
      return status;
    }

    i32 TCPClient::Send(CLLPCVOID pData, CLINT nLen) const
    {
      return ::send(m_socket, (const char*)pData, nLen, 0);
    }

    i32 TCPClient::Send(const BufferBase& buf) const
    {
      return Send(buf.GetPtr(), (CLINT)buf.GetSize());
    }

    i32 TCPClient::Send(const Repository& repo)
    {
      Repository::RAWDATA raw;
      repo.GetRawData(&raw);
      int result = 0;
      int r = 0;
      do {
        r = ::send(m_socket, (const char*)raw.header, (int)raw.cbHeader, 0);
        if(r != raw.cbHeader) {
          CLOG_ERROR("can not send repository header(%d).", r);
          break;
        }
        result += r;

        r = ::send(m_socket, (const char*)raw.keys, (int)raw.cbKeys, 0);
        if(r != raw.cbKeys) {
          CLOG_ERROR("can not send repository keys(%d).", r);
          break;
        }
        result += r;

        r = ::send(m_socket, (const char*)raw.names, (int)raw.cbNames, 0);
        if(r != raw.cbNames) {
          CLOG_ERROR("can not send repository names(%d).", r);
          break;
        }
        result += r;

        r = ::send(m_socket, (const char*)raw.data, (int)raw.cbData, 0);
        if(r != raw.cbData) {
          CLOG_ERROR("can not send repository data(%d).", r);
          break;
        }
        return result + r;
      } while(0);

      CloseSocket();
      return r;
    }

    //i32 TCPClient::Send(const StockA& stock) const
    //{
    //}

    //i32 TCPClient::Send(const StockW& stock) const
    //{
    //}

    i32 TCPClient::Recv(CLLPCVOID pData, CLINT nLen, b32 bRecvSpecifySize) const
    {
      if(bRecvSpecifySize == FALSE)
      {
        return ::recv(m_socket, (char*)pData, nLen, 0);
      }
      else
      {
        u32 nSpecifySize = nLen;
        do {
          int result = ::recv(m_socket, (char*)pData, nSpecifySize, 0);

          // 返回0表示温和地断开了链接
          if(result == SOCKET_ERROR || result == 0) {
            return result;
          }

          nSpecifySize -= result;
          pData = (char*)pData + result;
        } while (nSpecifySize > 0);
        return nLen;
      }
    }

    //////////////////////////////////////////////////////////////////////////

    namespace Internal {
      TCPClientThread::TCPClientThread(TCPListener* pListener, SOCKET socket, TCPClientProc fn)
        : TCPClient(socket)
        , m_tid(0)
        , m_pListener(pListener)
        , m_func(fn)
        , m_pWaiting(NULL)
      {
      }
      
      i32 TCPClientThread::StartRoutine()
      {
        ASSERT(m_tid == 0 && m_pWaiting == NULL);
        m_pWaiting = new Signal;
        m_tid      = this_thread::GetId();

        while(true)
        {
          if(m_socket == INVALID_SOCKET) {
            break;
          }
          
          m_func(*this);          
          m_pListener->CloseClientSocket(this);
          m_pWaiting->Wait();
        }

        return 0;
      }

      void TCPClientThread::SetSocket(SOCKET socket)
      {
        ASSERT(m_socket == INVALID_SOCKET);
        m_socket = socket;
        m_pWaiting->Set();
      }

      SOCKET TCPClientThread::GetSocket() const
      {
        return m_socket;
      }

      TCPClientThread::~TCPClientThread()
      {
        SAFE_DELETE(m_pWaiting);
      }

    } // namespace Internal

    //////////////////////////////////////////////////////////////////////////

    TCPListener::TCPListener()
      : m_socket(INVALID_SOCKET)
      , m_tid(0)
      , m_nIdleThread(0)
    {
      if( ! IsStartup()) {
        CLOG_ERROR("must be call net_socket::Startup() first.");
      }
      ASSERT(IsStartup());
    }

    TCPListener::~TCPListener()
    {
      for(auto it = m_ClientList.begin(); it != m_ClientList.end(); ++it)
      {
        (*it)->m_pWaiting->Set();
        (*it)->Wait(-1);
        SAFE_DELETE(*it);
      }
      
      for(auto it = m_RecyclePool.begin(); it != m_RecyclePool.end(); ++it)
      {
        (*it)->m_pWaiting->Set();
        (*it)->Wait(-1);
        SAFE_DELETE(*it);
      }
      m_ClientList.clear();
    }

    clstd::SocketResult TCPListener::OpenPort(CLUSHORT port)
    {
      SOCKADDR_IN serverSockAddr = {};
      ASSERT(net_sockets::IsStartup());

      int result = 0;

      serverSockAddr.sin_port = htons(port);
      serverSockAddr.sin_family = AF_INET;
      serverSockAddr.sin_addr.s_addr = htonl(INADDR_ANY);

      // 通信发生的区域,套接字的类型,套接字使用的特定协议
      m_socket = socket(AF_INET, SOCK_STREAM, 0);
      if (m_socket == INVALID_SOCKET) {
        //_ChkWSACleanup(status);
        CLOG_ERROR("ERROR: socket unsuccessful\r\n");
        return SocketResult_CreateFailed;
      }

      // associate the socket with the address
      result = ::bind(m_socket, (LPSOCKADDR)&serverSockAddr, sizeof(serverSockAddr));
      if (result == SOCKET_ERROR) { 
        //_ChkWSACleanup(status);
        CLOG_ERROR("bind unsuccessful(%d:%d)", result, WSAGetLastError()); 
        return SocketResult_CanotBind;
      }

      //FD_ISSET
      return SocketResult_Ok;
    }

    SocketResult TCPListener::ListenSocket()
    {
      int result = 0;
      result = ::listen(m_socket, 1);
      if(result == SOCKET_ERROR)
      {
        CLOG_ERROR("listen unsuccessful(%d:%d)", result, WSAGetLastError());
        return SocketResult_CanotListen;
      }

      CLOG("Waiting for connection(%d)...", result);
      return SocketResult_Ok;
    }

    int TCPListener::WaitSocket()
    {
      fd_set ReadSet;
      //fd_set ExceptSet;
      int result = 0;

      FD_ZERO(&ReadSet);
      FD_SET(m_socket, &ReadSet);

      //FD_ZERO(&ExceptSet);
      //FD_SET(m_socket, &ExceptSet);

      ASSERT(m_SocketList.size() < FD_SETSIZE - 1); // ServerSocket 要占用一个
      for(auto it = m_SocketList.begin(); it != m_SocketList.end(); ++it)
      {
        ASSERT(*it != 0);
        FD_SET(*it, &ReadSet);
        //FD_SET(*it, &ExceptSet);
      }

      result = ::select(m_socket + 1, &ReadSet, 0, 0, 0);

      if(result == 0)
      {
        // Time Out
        //ASSERT(ExceptSet.fd_count == 0);
      }
      else if(result == SOCKET_ERROR)
      {
        if(FD_ISSET(m_socket, &ReadSet))
        {
          return result;
        }
      }
      else if(result != 0)
      {
        //ASSERT(ExceptSet.fd_count == 0);
        if(FD_ISSET(m_socket, &ReadSet))
        {
          SOCKADDR_IN clientSockAddr;
          socklen_t addrLen = sizeof(SOCKADDR_IN);

          // accept the connection request when one is received
          SOCKET client = ::accept(m_socket, (LPSOCKADDR)&clientSockAddr, &addrLen);
          if(client != INVALID_SOCKET) {
            if(m_SocketList.size() < FD_SETSIZE - 1) {
              CLOG("Got the connection(socket:%d)...", client);
              if(OnAccept(client, clientSockAddr)) {
                m_SocketList.push_back(client);
              }
              else {
                ::closesocket(client);
              }
            }
            else {
              ::closesocket(client);
            }
          }
        }

        for(auto it = m_SocketList.begin(); it != m_SocketList.end();)
        {
          if(FD_ISSET(*it, &ReadSet))
          {
            u32 dwPeek;
            result = recv(*it, (char*)&dwPeek, sizeof(u32), MSG_PEEK);

            if(result == 0 || result == SOCKET_ERROR) // 端口已经关闭
            {
              // 这种方式下
              // 如果客户端在Debug下出现断点并关闭，这里会无法收到close消息
              OnDisconnect(*it);

              result = ::closesocket(*it);
              CLOG("client disconnected(%d).", *it);

              it = m_SocketList.erase(it);
              continue;
            }
            else {
              OnRecv(*it);
            }
          }
          ++it;
        } // for
      }
      return result;
    }

    int TCPListener::WaitSocketAsync(long timeout_sec, TCPClientProc proc)
    {
      fd_set ReadSet;
      //fd_set ExceptSet;
      int result = 0;
      if( ! m_tid) {
        m_tid = this_thread::GetId();
      }
      else if(m_tid != this_thread::GetId()) {
        CLOG_ERROR("call %s in different thread.", __FUNCTION__);
        return -1;
      }

      FD_ZERO(&ReadSet);
      FD_SET(m_socket, &ReadSet);

      //FD_ZERO(&ExceptSet);
      //FD_SET(m_socket, &ExceptSet);

      if(m_nIdleThread)
      {
        for(auto it = m_ClientList.begin(); it != m_ClientList.end();)
        {
          auto const pClient = *it;
          if(pClient->m_socket == INVALID_SOCKET) {
            m_RecyclePool.push_back(pClient);
            m_ClientList.erase(it++);
          }
          else {
            ++it;
          }
        }
        m_nIdleThread = 0;
      }

      timeval tv = {timeout_sec, 0};
      result = ::select(m_socket + 1, &ReadSet, NULL, NULL, &tv);

      if(result == 0)
      {
        // Time Out
        //ASSERT(ExceptSet.fd_count == 0);
      }
      else if(result == SOCKET_ERROR)
      {
        std::for_each(m_ClientList.begin(), m_ClientList.end(),
          [](Internal::TCPClientThread* pClient)
        {
          pClient->CloseSocket();
        });

        if(FD_ISSET(m_socket, &ReadSet))
        {
          return result;
        }
      }
      else if(result != 0)
      {
        //ASSERT(ExceptSet.fd_count == 0);
        if(FD_ISSET(m_socket, &ReadSet))
        {
          SOCKADDR_IN clientSockAddr;
          socklen_t addrLen = sizeof(SOCKADDR_IN);

          // accept the connection request when one is received
          SOCKET client = ::accept(m_socket, (LPSOCKADDR)&clientSockAddr, &addrLen);
          if(client != INVALID_SOCKET) {
            CLOG("Got the connection(socket:%d)...", client);
            if(OnAccept(client, clientSockAddr)) {
              //m_SocketList.push_back(client);
              Internal::TCPClientThread* pClient = NULL;
              if(m_RecyclePool.empty())
              {
                pClient = new Internal::TCPClientThread(this, client, proc);
                pClient->Start();
              }
              else
              {
                pClient = m_RecyclePool.front();
                m_RecyclePool.pop_front();
                pClient->SetSocket(client);
              }
              m_ClientList.push_back(pClient);
            }
            else {
              CLOG_WARNING("user refused connection(from %s:%d)",
                inet_ntoa(clientSockAddr.sin_addr), ntohs(clientSockAddr.sin_port));
              ::closesocket(client);
            }
          }
          else {
            CLOG_ERROR("accept return INVALID_SOCKET");
          }
        }
      }
      return result;
    }

    int TCPListener::CloseSocket()
    {
      int result = 0;

      /*
      // 退出时清理客户端端口
      for(auto it = m_SocketList.begin(); it != m_SocketList.end(); ++it)
      {
        result = ::closesocket(*it);
        if(result == SOCKET_ERROR) {
          CLOG_ERROR("Error for closing socket(socket:%d, result:%d)...\r\n", *it, result);
        }
      }
      m_SocketList.clear();//*/

      if(m_socket)
      {
        CLOG("Close server socket...");
        const SOCKET local_socket = m_socket;
        m_socket = 0;
        result = ::closesocket(local_socket);
        
        if(result) {
          CLOG_ERROR("Error for closing server socket(%d)...", result);
        }
        //_ChkWSACleanup(status);
      }

      return result;
    }

    i32 TCPListener::Send(SOCKET sock, CLLPCVOID pData, u32 nLen)
    {
      i32 nSent = 0;
      do {
        const int result = send(sock, (const char*)pData + nSent, nLen - nSent, 0);
        if(result < 0) {
          CLOG_ERROR("%s send error(%d:%d)", __FUNCTION__, result, WSAGetLastError());
          return result;
        }
        nSent += result;
      } while(nSent < (i32)nLen);
      return nSent;
    }

    i32 TCPListener::Recv(SOCKET sock, CLLPVOID pData, u32 nLen)
    {
      return recv(sock, (char*)pData, nLen, 0);
    }

    i32 TCPListener::GetClientCount() const
    {
      const auto tid = this_thread::GetId();
      if(tid != m_tid)
      {
        CLOG_ERROR("call %s in unexpected thread(tid:%u).", __FUNCTION__, (u32)tid);
        return -1;
      }

      if( ! m_SocketList.empty()) {
        return (i32)m_SocketList.size() - 1;
      }
      return (i32)m_ClientList.size();
    }

    void TCPListener::CloseClientSocket(TCPClient* pClient)
    {
      auto pClientTh = static_cast<Internal::TCPClientThread*>(pClient);
      pClient->CloseSocket();
      auto tid = this_thread::GetId();
      if(tid == pClientTh->m_tid)
      {
        ++m_nIdleThread;
      }
      else if(tid == m_tid)
      {
        m_ClientList.erase(std::find(m_ClientList.begin(), m_ClientList.end(), pClientTh));
        m_RecyclePool.push_back(pClientTh);
      }
      else
      {
        CLOG_ERROR("call %s in unexpected thread(tid:%d).", __FUNCTION__, (u32)tid);
        CLBREAK;
      }
    }

    b32 TCPListener::OnAccept(SOCKET socket, const SOCKADDR_IN& addr_in)
    {
      return TRUE;
    }

    void TCPListener::OnDisconnect(SOCKET socket)
    {
    }

    void TCPListener::OnRecv(SOCKET socket)
    {
    }

//#endif // _CL_SYSTEM_WINDOWS
  } // namespace net_sockets
} // namespace clstd