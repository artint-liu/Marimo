#ifndef _CLSTD_SOCKET_H_
#define _CLSTD_SOCKET_H_

#if defined(_CL_SYSTEM_LINUX)
# include <sys/socket.h>
# include <netinet/in.h>

typedef int SOCKET;
typedef sockaddr_in SOCKADDR_IN;
typedef sockaddr SOCKADDR;
typedef sockaddr* LPSOCKADDR;
# define INVALID_SOCKET -1
# define SOCKET_ERROR -1
#endif

namespace clstd
{
  class Signal;
  class StockA;
  class StockW;
  class Repository;
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
    SocketResult_Disconnected = -1,
    SocketResult_Error = -2,
    SocketResult_UnknownError = -3,   // 没有在封装中处理的错误

    SocketResult_CreateFailed = -4,
    SocketResult_CanotBind = -5,
    SocketResult_CanotListen = -6,
    SocketResult_CanotConnect = -7,

    SocketResult_BadSocket = -8,

    SocketResult_TimeOut = 100,
  };

  enum SocketOption
  {
    SocketOption_SendTimeOut,     // 毫秒
    SocketOption_ReceiveTimeOut,
    SocketOption_SendBuffer,      // 字节
    SocketOption_ReceiveBuffer,
  };

//#define _ChkWSACleanup(_status) _status = WSACleanup(); ASSERT(_status != WSANOTINITIALISED)

  namespace net_sockets
  {
    class TCPClient;
    class TCPListener;
    typedef std::function<void(TCPClient&)> TCPClientProc;

    int Startup();
    int Cleanup();
    b32 IsStartup();

    class SocketInterface
    {
    protected:
      SOCKET  m_socket;

      SocketInterface(SOCKET socket);

    public:
      i32 GetOption(SocketOption eOpt);
      i32 SetOption(SocketOption eOpt, int nValue);
    };

//#if defined(_CL_SYSTEM_WINDOWS)
    //class TCPSocket
    //{
    //protected:
    //  SOCKET  m_socket;

    //public:
    //  int CloseSocket ();
    //  i32 Send    (CLLPCVOID pData, CLINT nLen);
    //  i32 Recv    (CLLPCVOID pData, CLINT nLen);
    //  i32 Send    (const BufferBase& buf);
    //  //i32 Recv    (CLLPCVOID pData, CLINT nLen);
    //};

    class TCPClient : public SocketInterface
    {
    public:
      TCPClient();
      TCPClient(SOCKET socket);
      virtual ~TCPClient();

      SocketResult Connect (CLLPCSTR addr_port);
      SocketResult Connect (CLLPCSTR addr, CLUSHORT port);
      SocketResult Connect (u32 addr, CLUSHORT port);

      b32 IsInvalidSocket() const;
      SocketResult WaitSocket  (long timeout_sec = 0);
      int CloseSocket ();

      i32 Send    (CLLPCVOID pData, CLINT nLen) const;
      i32 Recv    (CLLPCVOID pData, CLINT nLen, b32 bRecvSpecifySize) const;  // 返回收到了字节数，返回SOCKET_ERROR表示端口错误，0表示断开连接
      i32 Send    (const BufferBase& buf) const;
      //i32 Send    (const StockA& stock) const;
      //i32 Send    (const StockW& stock) const;
      i32 Send    (const Repository& repo); // 发送失败会主动关闭socket
    };

    namespace Internal {

      class TCPClientThread : public clstd::Thread, TCPClient
      {
        friend class clstd::net_sockets::TCPListener;
        
        TCPClientThread(TCPListener* pListener, SOCKET socket, TCPClientProc fn);
        virtual ~TCPClientThread();
        i32 StartRoutine() override;
        void SetSocket(SOCKET socket);
        SOCKET GetSocket() const;

        this_thread::id m_tid;
        TCPListener*    m_pListener;
        TCPClientProc   m_func;
        Signal*         m_pWaiting;
      };
    } // namespace Internal

    class TCPListener : public SocketInterface
    {
    protected:
      typedef clist<SOCKET> SocketList;
      typedef clist<Internal::TCPClientThread*> ClientList;
      SocketList  m_SocketList;

      // 异步模式
      this_thread::id   m_tid;
      ClientList        m_ClientList;
      ClientList        m_RecyclePool;
      volatile size_t   m_nIdleThread; // 异步模式下空闲的客户端线程，可能不准
      //Signal      m_IdleSig;

    public:
      TCPListener();
      virtual ~TCPListener();

      SocketResult OpenPort(CLUSHORT port);
      SocketResult ListenSocket();

      int WaitSocket      ();
      int WaitSocketAsync (long timeout_sec, TCPClientProc proc);
      int CloseSocket     ();

      i32 Send  (SOCKET sock, CLLPCVOID pData, u32 nLen);
      i32 Recv  (SOCKET sock, CLLPVOID pData, u32 nLen);

      i32 GetClientCount() const; // 只能TCPListener线程调用
      void CloseClientSocket(TCPClient* pClient);

      virtual b32  OnAccept(SOCKET socket, const SOCKADDR_IN& addr_in); // 不处理直接返回TRUE，返回FALSE则断开链接
      virtual void OnDisconnect(SOCKET socket);
      virtual void OnRecv(SOCKET socket);
    };

//#endif // _CL_SYSTEM_WINDOWS
  } // namespace net_sockets

} // namespace clstd

#endif // _CLSTD_SOCKET_H_