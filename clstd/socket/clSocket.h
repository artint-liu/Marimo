#ifndef _CLSTD_SOCKET_H_
#define _CLSTD_SOCKET_H_

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
    class TCPClient;
    class TCPListener;
    typedef std::function<void(TCPClient&)> TCPClientProc;

    int Startup();
    int Cleanup();
    b32 IsStartup();

#if defined(_CL_SYSTEM_WINDOWS)
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

    class TCPClient
    {
    protected:
      SOCKET  m_socket;

    public:
      TCPClient();
      TCPClient(SOCKET socket);
      virtual ~TCPClient();

      SocketResult Connect (CLLPCSTR addr_port);
      SocketResult Connect (CLLPCSTR addr, CLUSHORT port);
      SocketResult Connect (u32 addr, CLUSHORT port);

      b32 IsInvalidSocket() const;
      int WaitSocket  (long timeout_sec = 0);
      int CloseSocket ();

      i32 Send    (CLLPCVOID pData, CLINT nLen) const;
      i32 Recv    (CLLPCVOID pData, CLINT nLen, b32 bRecvSpecifySize) const;
      i32 Send    (const BufferBase& buf) const;
      //i32 Send    (const StockA& stock) const;
      //i32 Send    (const StockW& stock) const;
      i32 Send    (const Repository& repo); // 发送失败会主动关闭socket
    };

    namespace Internal {

      class TCPClientThread : public clstd::Thread, TCPClient
      {
        friend class TCPListener;
        
        TCPClientThread(TCPListener* pListener, SOCKET socket, TCPClientProc fn);
        virtual ~TCPClientThread();
        i32 Run() override;
        void SetSocket(SOCKET socket);
        SOCKET GetSocket() const;

        this_thread::id m_tid;
        TCPListener*    m_pListener;
        TCPClientProc   m_func;
        Signal*         m_pWaiting;
      };
    }

    class TCPListener
    {
    protected:
      typedef clist<SOCKET> SocketList;
      typedef clist<Internal::TCPClientThread*> ClientList;
      SOCKET      m_socket;
      SocketList  m_SocketList;

      // 异步模式
      this_thread::id   m_tid;
      ClientList        m_ClientList;
      ClientList        m_RecyclePool;
      volatile size_t   m_nIdelThread; // 异步模式下空闲的客户端线程，可能不准
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

#endif // _CL_SYSTEM_WINDOWS
  } // namespace net_sockets

} // namespace clstd

#endif // _CLSTD_SOCKET_H_