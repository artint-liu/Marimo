#ifndef _CLSTD_SOCKET_H_
#define _CLSTD_SOCKET_H_

namespace clstd
{
  enum SocketEvent
  {
    SE_CONNECT,
    SE_ACCEPT,
    SE_READ,
    SE_WRITE,
    SE_CLOSE,
  };

  enum SocketResult
  {
    SocketResult_Ok = 0,
    SocketResult_Error = -1,
    SocketResult_Disconnected = -2,

    SocketResult_CreateFailed = -3,
    SocketResult_CanotBind = -4,
    SocketResult_CanotListen = -5,
    SocketResult_CanotConnect = -6,
  };

#define _ChkWSACleanup(_status) _status = WSACleanup(); ASSERT(_status != WSANOTINITIALISED)

  namespace net_sockets
  {
    int Startup();
    int Cleanup();

    b32 IsStartup();

#if defined(_CL_SYSTEM_WINDOWS)
    class TCPClient
    {
    protected:
      SOCKET  m_socket;

    public:
      TCPClient();
      virtual ~TCPClient();

      SocketResult Connect (CLLPCSTR addr_port);
      SocketResult Connect (CLLPCSTR addr, CLUSHORT port);
      SocketResult Connect (u32 addr, CLUSHORT port);

      int WaitSocket  ();
      int CloseSocket ();

      i32 Send    (CLLPCVOID pData, CLINT nLen);
      i32 Recv    (CLLPCVOID pData, CLINT nLen, b32 bRecvSpecifySize);
    };

    class TCPListener
    {
    protected:
      typedef clist<SOCKET> SocketList;
      SOCKET      m_socket;
      SocketList  m_SocketList;

    public:
      TCPListener();
      virtual ~TCPListener();

      SocketResult OpenPort(CLUSHORT port);
      SocketResult ListenSocket();

      int WaitSocket  ();
      int CloseSocket ();

      i32 Send  (SOCKET sock, CLLPCVOID pData, u32 nLen);
      i32 Recv  (SOCKET sock, CLLPVOID pData, u32 nLen);

      virtual b32  OnAccept(SOCKET socket, const SOCKADDR_IN& addr_in); // 不处理直接返回TRUE，返回FALSE则断开链接
      virtual void OnDisconnect(SOCKET socket);
      virtual void OnRecv(SOCKET socket) = 0;
    };

#endif // _CL_SYSTEM_WINDOWS
  } // namespace net_sockets

} // namespace clstd

#endif // _CLSTD_SOCKET_H_