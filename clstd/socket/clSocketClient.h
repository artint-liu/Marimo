#ifndef _SOCKET_CLIENT_H_
#define _SOCKET_CLIENT_H_

#ifndef _CLSTD_THREAD_H_
#error Must be include "clThread.h" first.
#endif // #ifdef _CLSTD_THREAD_H_

namespace clstd
{
  enum SocketEvent;
#if defined(_CL_SYSTEM_WINDOWS)
  class TCPClient : public Thread
  {
  protected:
    SOCKET		m_clientSocket;
    int MainLoop();
  public:
    TCPClient();
    virtual ~TCPClient();

    SocketResult Connect (CLLPCSTR addr_port);
    SocketResult Connect (CLLPCSTR addr, CLUSHORT port);
    SocketResult Connect (u32 addr, CLUSHORT port);

    int Close   (u32 nMilliSec);
    i32 Send    (CLLPCVOID pData, CLINT nLen);
    i32 Recv    (CLLPCVOID pData, CLINT nLen, b32 bRecvSpecifySize);

    i32 StartRoutine() override;

  public:
    virtual void OnEvent(/*SOCKET sock, */SocketEvent eEvent) = 0;
  };
#endif // #if defined(_CL_SYSTEM_WINDOWS)
} // namespace clstd

#endif // _SOCKET_CLIENT_H_