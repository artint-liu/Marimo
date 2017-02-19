#include "clstd.h"
#include "clString.h"
#include "thread/clThread.h"
#include "clSocket.h"
#include "clSocketServer.h"

#define SOCKET_ERROR_LOG(_STAT, _MSG) if (_STAT == SOCKET_ERROR) { CLOG_ERROR(_MSG); }

namespace clstd
{
  namespace net_sockets
  {
#if defined(_CL_SYSTEM_WINDOWS)
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
      : m_socket (0)
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
      strAddr.DivideBy(':', strAddr, strPort);
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
        m_socket = 0;
        return SocketResult_CreateFailed;
      }

      status = ::connect(m_socket, (SOCKADDR*)&SockAddr, sizeof(SockAddr));
      if(status == SOCKET_ERROR) {
        //_ChkWSACleanup(status);
        CLOG_ERROR("ERROR: can not connect the server.\n");
        m_socket = 0;
        return SocketResult_CanotConnect;
      }
      //OnEvent(/*m_clientSocket, */SE_CONNECT);

      return SocketResult_Ok;
    }

    int TCPClient::WaitSocket()
    {
      fd_set read_set;
      FD_ZERO(&read_set);
      FD_SET(m_socket, &read_set);
      int result = ::select(0, &read_set, 0, 0, 0);
      
      if(result == SOCKET_ERROR || result == 0) {
        CLOG("socket error : disconnected(%d) ...", result);
        CloseSocket();
        return result;
      }
      else if(m_socket == 0) 
      {
        CLOG("user disconnected(%d) ...", result);
        return SocketResult_Disconnected;
      }
      return result;
    }

    int TCPClient::CloseSocket()
    {
      int status = 0;
      if(m_socket)
      {
        const SOCKET local_socket = m_socket;
        m_socket = 0;
        status = ::closesocket(local_socket);
        CLOG("closesocket(%d)", status);

        //_ChkWSACleanup(status);
      }
      return status;
    }

    i32 TCPClient::Send(CLLPCVOID pData, CLINT nLen)
    {
      return send(m_socket, (const char*)pData, nLen, 0);
    }

    i32 TCPClient::Recv(CLLPCVOID pData, CLINT nLen, b32 bRecvSpecifySize)
    {
      if(bRecvSpecifySize == FALSE)
      {
        return recv(m_socket, (char*)pData, nLen, 0);
      }
      else
      {
        u32 nSpecifySize = nLen;
        do {
          int result = recv(m_socket, (char*)pData, nSpecifySize, 0);

          if(result == SOCKET_ERROR) {
            return result;
          }

          nSpecifySize -= result;
          pData = (char*)pData + result;
        } while (nSpecifySize > 0);
        return nLen;
      }
    }

    //////////////////////////////////////////////////////////////////////////

    TCPListener::TCPListener()
    {
      ASSERT(IsStartup());
    }

    TCPListener::~TCPListener()
    {
    }

    clstd::SocketResult TCPListener::OpenPort(CLUSHORT port)
    {
      SOCKADDR_IN serverSockAddr = {};
      ASSERT(net_sockets::IsStartup());

      int status = 0;

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
      status = bind(m_socket, (LPSOCKADDR)&serverSockAddr, sizeof(serverSockAddr));
      if (status == SOCKET_ERROR) { 
        //_ChkWSACleanup(status);
        CLOG_ERROR("ERROR: bind unsuccessful\r\n"); 
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
        CLOG_ERROR("ERROR: listen unsuccessful(%d)", result);
        return SocketResult_CanotListen;
      }

      CLOG("Waiting for connection(%d)...", result);
      return SocketResult_Ok;
    }

    int TCPListener::WaitSocket()
    {
      fd_set ReadSet;
      fd_set ExceptSet;
      int result = 0;

      FD_ZERO(&ReadSet);
      FD_SET(m_socket, &ReadSet);

      FD_ZERO(&ExceptSet);
      FD_SET(m_socket, &ExceptSet);

      ASSERT(m_SocketList.size() < FD_SETSIZE - 1); // ServerSocket 要占用一个
      for(auto it = m_SocketList.begin(); it != m_SocketList.end(); ++it)
      {
        ASSERT(*it != NULL);
        FD_SET(*it, &ReadSet);
        FD_SET(*it, &ExceptSet);
      }

      result = ::select(0, &ReadSet, 0, &ExceptSet, 0);

      if(result == 0)
      {
        // Time Out
        ASSERT(ExceptSet.fd_count == 0);
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
        ASSERT(ExceptSet.fd_count == 0);
        if(FD_ISSET(m_socket, &ReadSet))
        {
          SOCKADDR_IN clientSockAddr;
          int addrLen = sizeof(SOCKADDR_IN);

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

    int TCPListener::CloseSocket()
    {
      int result = 0;

      // 退出时清理客户端端口
      for(auto it = m_SocketList.begin(); it != m_SocketList.end(); ++it)
      {
        result = ::closesocket(*it);
        if(result == SOCKET_ERROR) {
          CLOG_ERROR("Error for closing socket(socket:%d, result:%d)...\r\n", *it, result);
        }
      }
      m_SocketList.clear();

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

    b32 TCPListener::OnAccept(SOCKET socket, const SOCKADDR_IN& addr_in)
    {
      return TRUE;
    }

    void TCPListener::OnDisconnect(SOCKET socket)
    {
    }

#endif // _CL_SYSTEM_WINDOWS
  } // namespace net_sockets
} // namespace clstd