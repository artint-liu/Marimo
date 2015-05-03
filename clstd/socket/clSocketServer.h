#ifndef _SOCKET_SERVER_H_
#define _SOCKET_SERVER_H_

#ifndef _CLSTD_THREAD_H_
#error Must be include "clThread.h" first.
#endif // #ifdef _CLSTD_THREAD_H_

namespace clstd
{
  enum SocketEvent;

  class TCPServer : public Thread
  {
  public:
    typedef clist<SOCKET> SocketList;
  protected:
    SOCKET		  m_ServerSocket;
    SocketList  m_ClientList;
    int MainLoop  ();
    i32 Run       ();

  public:
    	TCPServer();
    	virtual ~TCPServer();
    
    	SocketResult OpenPort(CLUSHORT port);

      // �ر������׽���
      // dwMilliSec: �ȴ��̳߳�ʱ
      // 0: ���ȴ�
      // -1(0xffffffff): �ȴ�ֱ���˳�
      // ����: �ȴ���ʱʱ��
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
  protected:
    SOCKET        m_Socket;
    u32           m_dwFlags;

    int MainLoop();
    i32 Run();
  public:
    UDPSocket();
    virtual ~UDPSocket();

    SocketResult OpenPort(u32 dwFlags, u32 uRecvPort);

    // �ر������׽���
    // dwMilliSec: �ȴ��̳߳�ʱ
    // 0: ���ȴ�
    // -1(0xffffffff): �ȴ�ֱ���˳�
    // ����: �ȴ���ʱʱ��
    int Close (u32 nMilliSec);
    i32 Send(CLLPCSTR szIPAddress, u32 wPort, CLLPCVOID pData, u32 nLen);
    i32 Send(u32_ptr uIPAddress, u32 wPort, CLLPCVOID pData, u32 nLen);
    i32 Recv(CLLPCVOID pData, u32 nLen, u32_ptr* uIPAddress, u32* wPort);

    template<class _StringT>
    static size_t SockAddrToString(_StringT& str, u32_ptr uIPAddress)
    {
      str = inet_ntoa(*(in_addr*)&uIPAddress);
      return str.GetLength();
    }

  public:
    virtual void OnEvent(SOCKET sock, SocketEvent eEvent) = 0;
  };

} // namespace clstd

#endif // _SOCKET_SERVER_H_