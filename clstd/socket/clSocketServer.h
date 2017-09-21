#ifndef _SOCKET_SERVER_H_
#define _SOCKET_SERVER_H_

#ifndef _CLSTD_THREAD_H_
# error Must be include "clThread.h" first.
#endif // #ifdef _CLSTD_THREAD_H_

#ifndef _CLSTD_SOCKET_H_
# error Must be include "clSocket.h" first.
#endif

namespace clstd
{
  enum SocketEvent;
#if defined(_CL_SYSTEM_WINDOWS)
  class TCPServer : public Thread
  {
  public:
    typedef clist<SOCKET> SocketList;
  protected:
    SOCKET		  m_ServerSocket;
    SocketList  m_ClientList;
    int MainLoop  ();
    i32 StartRoutine() override;

  public:
    	TCPServer();
    	virtual ~TCPServer();
    
    	SocketResult OpenPort(CLUSHORT port);

      // 关闭网络套接字
      // dwMilliSec: 等待线程超时
      // 0: 不等待
      // -1(0xffffffff): 等待直到退出
      // 其它: 等待超时时间
      int Close (u32 nMilliSec);
      i32 Send  (SOCKET sock, CLLPCVOID pData, u32 nLen);
      i32 Recv  (SOCKET sock, CLLPVOID pData, u32 nLen);

  public:
    virtual void OnEvent(SOCKET sock, SocketEvent eEvent) = 0;
  };

  class UDPSocket : public Thread
  {
  public:
    enum PropertyFlags
    {
      PM_Send      = 0x0001,
      PM_Recv      = 0x0002,
      PM_Broadcast = 0x0004,

      SendOnly     = PM_Send,
      RecvOnly     = PM_Recv,
      SendRecv     = PM_Send | PM_Recv,
      SendBC       = PM_Send | PM_Broadcast,
      SendRecvBC   = PM_Send | PM_Recv | PM_Broadcast,
    };

    typedef u32_ptr IPAddr;

    const static IPAddr BroadcastAddress = -1;

  protected:
    SOCKET        m_Socket;
    u32           m_dwFlags;

    int MainLoop();
    i32 StartRoutine() override;
  public:
    UDPSocket();
    virtual ~UDPSocket();

    SocketResult OpenPort(PropertyFlags dwFlags, u32 uRecvPort);

    // 关闭网络套接字
    // dwMilliSec: 等待线程超时
    // 0: 不等待
    // -1(0xffffffff): 等待直到退出
    // 其它: 等待超时时间
    int Close (u32 nMilliSec);
    i32 Send(CLLPCSTR szIPAddress, u32 wPort, CLLPCVOID pData, u32 nLen);
    i32 Send(IPAddr uIPAddress, u32 wPort, CLLPCVOID pData, u32 nLen);
    //i32 SendBroadCast(u32 wPort, CLLPCVOID pData, u32 nLen);
    i32 Recv(CLLPCVOID pData, u32 nLen, IPAddr* uIPAddress, u32* wPort);

    template<class _StringT>
    static size_t SockAddrToString(_StringT& str, u32_ptr uIPAddress)
    {
      str = inet_ntoa(*(in_addr*)&uIPAddress);
      return str.GetLength();
    }

  public:
    virtual void OnEvent(SOCKET sock, SocketEvent eEvent) = 0;
  };
#endif // #if defined(_CL_SYSTEM_WINDOWS)
} // namespace clstd

#endif // _SOCKET_SERVER_H_