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
    SocketResult_CreateFailed = -1,
    SocketResult_CanotBind = -2,
    SocketResult_CanotListen = -3,
    SocketResult_CanotConnect = -4,
  };

#define _ChkWSACleanup(_status) _status = WSACleanup(); ASSERT(_status != WSANOTINITIALISED)

} // namespace clstd

#endif // _CLSTD_SOCKET_H_