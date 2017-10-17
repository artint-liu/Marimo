// Sample_Sockets.cpp : 定义控制台应用程序的入口点。
//
#include <functional>
//#include <conio.h>

#include <clstd.h>
#include <thread/clThread.h>
#include <socket/clSocket.h>
#include <clString.h>
#include <clUtility.h>

#pragma comment(lib, "Ws2_32.lib")

#ifndef VK_ESCAPE
# define VK_ESCAPE 0x1B
#endif

void StartAsServer();
void StartAsClient();
void PrintUsage();

int main(int argc, char* argv[])
{
  if(argc != 2) {
    PrintUsage();
    return 0;
  }

  clstd::net_sockets::Startup();

  if(clstd::strcmpT(argv[1], "-s") == 0) {
    StartAsServer();
  }
  else if(clstd::strcmpT(argv[1], "-c") == 0) {
    StartAsClient();
  }
  else {
    PrintUsage();
  }

  clstd::net_sockets::Cleanup();
  return 0;
}

//////////////////////////////////////////////////////////////////////////

void PrintUsage()
{
  printf("usage:\n <.exe> <-s/-c>\n");
}

//////////////////////////////////////////////////////////////////////////

void StartAsServer()
{
  using namespace clstd;
  net_sockets::TCPListener listener;

  listener.OpenPort(34001);
  i32 nSendTimeOut = listener.GetOption(SocketOption_SendTimeOut);
  i32 nRecvTimeOut = listener.GetOption(SocketOption_ReceiveTimeOut);
  i32 nSendBuffer = listener.GetOption(SocketOption_SendBuffer);
  i32 nRecvBuffer = listener.GetOption(SocketOption_ReceiveBuffer);
  CLOG("Send timeout:%d, receive timeout:%d, send buffer:%d, receive buffer:%d",
    nSendTimeOut, nRecvTimeOut, nSendBuffer, nRecvBuffer);

  listener.ListenSocket();
  
  while(1)
  {
    int result = 0;
    result = listener.WaitSocketAsync(2, [](net_sockets::TCPClient& client)
    {
      do {
        CLBYTE buffer[4096];
        int result = 0;
        result = client.Recv(buffer, 4096, FALSE);
        if(result > 0) {
          clstd::DumpMemory(buffer, result);
        }
        else if(result == 0 || result == SOCKET_ERROR) {
          break;
        }
        CLOG("%s(%d) %s: %d", __FILE__, __LINE__, __FUNCTION__, result);
      } while(1);
    });

    //CLOG("%s(%d) %s: %d", __FILE__, __LINE__, __FUNCTION__, result);
    if(clstd_cli::kbhit() && clstd_cli::getch() == VK_ESCAPE) {
      break;
    }
  }
}

//////////////////////////////////////////////////////////////////////////

void StartAsClient()
{
  using namespace clstd;
  net_sockets::TCPClient client;
  //CLBYTE buffer[4096];
  clStringA str;
  
  CLOG("client: connecting");
  client.Connect("127.0.0.1:34001");
  CLOG("client: connected OK");

  i32 nSendTimeOut = client.GetOption(SocketOption_SendTimeOut);
  i32 nRecvTimeOut = client.GetOption(SocketOption_ReceiveTimeOut);
  i32 nSendBuffer  = client.GetOption(SocketOption_SendBuffer);
  i32 nRecvBuffer  = client.GetOption(SocketOption_ReceiveBuffer);
  CLOG("Send timeout:%d, receive timeout:%d, send buffer:%d, receive buffer:%d",
    nSendTimeOut, nRecvTimeOut, nSendBuffer, nRecvBuffer);

  CLOG("输入字符后按回车发送，ESC退出");

  while(1) {
    int result = 0;
    
    result = client.WaitSocket(1);
    if(result == SOCKET_ERROR) {
      CLOG_ERROR("client: socket error(%d)", result);
      break;
    }

    while(clstd_cli::kbhit()) {
      int c = clstd_cli::getch();
      if(c == VK_ESCAPE)
      {
        return;
      }

      if(c == '\r' || c == '\n'){
        if(str.IsNotEmpty())
        {
          result = client.Send(str, str.GetLength());
          CLOG("sent(result=%d):%s", result, str.CStr());
          str.Clear();
        }
      }
      else
      {
        str.Append(c);
        CLOG("Input:%s", str.CStr());
      }
    }
  }
}
