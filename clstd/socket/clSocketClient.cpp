#include "clstd.h"
#include "clString.h"
#include "thread/clThread.h"
#include "clSocket.h"
#include "clSocketClient.h"

//inet_addr()

#define SOCKET_ERROR_LOG(_STAT, _MSG) if (_STAT == SOCKET_ERROR) { CLOG_ERROR(_MSG); }
#define MAX_RECV_BUF 4096

#if defined(_WINDOWS)
#pragma comment(lib, "Ws2_32.lib")
namespace clstd
{

  TCPClient::TCPClient()
    : m_clientSocket (0)
  {
  }

  TCPClient::~TCPClient()
  {
  }

  SocketResult TCPClient::Connect(CLLPCSTR addr_port)
  {
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
    WSADATA		Data;
    SOCKADDR_IN SockAddr;

    //	
    //

    int status = WSAStartup(CLMAKEWORD(1, 1), &Data);
    if (status != 0) {
      CLOG_ERROR("ERROR: WSAStartup unsuccessful\r\n");
    }

    // zero the sockaddr_in structure
    memset(&SockAddr, 0, sizeof(SockAddr));

    // specify the port portion of the address
    SockAddr.sin_port = htons(port);
    // specify the address family as Internet
    SockAddr.sin_family = AF_INET;
    // specify that the address does not matter
    SockAddr.sin_addr.s_addr = addr;

    // create a socket  socket(通信发生的区域,套接字的类型,套接字使用的特定协议)
    m_clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_clientSocket == INVALID_SOCKET) {
      _ChkWSACleanup(status);
      CLOG_ERROR("ERROR: socket unsuccessful\n");
      return SocketResult_CreateFailed;
    }

    status = connect(m_clientSocket, (SOCKADDR*)&SockAddr, sizeof(SockAddr));
    if(status == SOCKET_ERROR) {
      _ChkWSACleanup(status);
      CLOG_ERROR("ERROR: can not connect the server.\n");
      return SocketResult_CanotConnect;
    }
    OnEvent(/*m_clientSocket, */SE_CONNECT);

    return SocketResult_Ok;
  }

  int TCPClient::Close(u32 nMilliSec)
  {
    int status = 0;
    if(m_clientSocket)
    {
      status = closesocket(m_clientSocket);
      //status = closesocket(m_serverSocket);
      m_clientSocket = 0;
      //m_serverSocket = 0;

      if(nMilliSec != 0) {
        Wait(nMilliSec);
      }

      _ChkWSACleanup(status);
    }
    return status;
  }

  i32 TCPClient::Send(CLLPCVOID pData, CLINT nLen)
  {
    return send(m_clientSocket, (const char*)pData, nLen, 0);
  }

  i32 TCPClient::Recv(CLLPCVOID pData, CLINT nLen, b32 bRecvSpecifySize)
  {
    if(bRecvSpecifySize == FALSE)
    {
      return recv(m_clientSocket, (char*)pData, nLen, 0);
    }
    else
    {
      u32 nSpecifySize = nLen;
      do {
        int result = recv(m_clientSocket, (char*)pData, nSpecifySize, 0);

        if(result == SOCKET_ERROR) {
          return result;
        }

        nSpecifySize -= result;
        pData = (char*)pData + result;
      } while (nSpecifySize > 0);
      return nLen;
    }
  }

  i32 TCPClient::Run()
  {
    int addrLen = sizeof(SOCKADDR_IN);
    CLOG("Got the connection...\r\n");
    MainLoop();
    return 0;
  }

  int TCPClient::MainLoop()
  {
    fd_set ReadSet;
    int result = 0;

    while(1)
    {
      FD_ZERO(&ReadSet);
      FD_SET(m_clientSocket, &ReadSet);
      //result = recv(m_clientSocket, recvBuf, MAX_RECV_BUF, 0);
      result = select(0, &ReadSet, 0, 0, 0);
      if(result == SOCKET_ERROR || result == 0)
      {
        //#ifndef _DEBUG
        //			g_Buffer.uEnd = 0;
        //			g_Buffer.uStart = 0;
        //#endif // _DEBUG
        TRACE("disconnected...\r\n");
        break;
      }
      OnEvent(/*m_clientSocket, */SE_READ);
    }
    //SAFE_DELETE(buffer);
    return result;
  }

  //void TCPClient::OnEvent(/*SOCKET sock, */SocketEvent eEvent)
  //{
  //}
} // namespace clstd
#endif // #if defined(_WINDOWS)