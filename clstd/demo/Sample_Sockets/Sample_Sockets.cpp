// Sample_Sockets.cpp : 定义控制台应用程序的入口点。
//
#include <functional>
#include <conio.h>

#include <clstd.h>
#include <thread/clThread.h>
#include <socket/clSocket.h>
#include <clString.h>
#include <clUtility.h>

#pragma comment(lib, "Ws2_32.lib")

void StartAsServer();
void StartAsClient();
void PrintUsage();

int main(int argc, char* argv[])
{
  if(argc != 2) {
    PrintUsage();
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
        else if(result == SOCKET_ERROR) {
          break;
        }
        CLOG("%s(%d) %s: %d", __FILE__, __LINE__, __FUNCTION__, result);
      } while(1);
    });

    //CLOG("%s(%d) %s: %d", __FILE__, __LINE__, __FUNCTION__, result);
    if(_kbhit() && _getch() == VK_ESCAPE) {
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

  while(1) {
    int result = 0;
    
    result = client.WaitSocket(1);
    if(result == SOCKET_ERROR) {
      CLOG_ERROR("client: socket error(%d)", result);
      break;
    }

    //result = client.Recv(buffer, 4096, FALSE);
    //if(result == SOCKET_ERROR) {
    //  break;
    //}
    //else if(result == 0) {
    //  break;
    //}

    if(_kbhit()) {
      int c = _getch();
      if(c == VK_ESCAPE)
      {
        break;
      }

      str.Append(c);
      CLOG("Input:%s", str);
      if(str.GetLength() > 8) {
        client.Send(str, str.GetLength());
        str.Clear();
      }
    }
  }
}
