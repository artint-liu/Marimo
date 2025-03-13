// Sample_Sockets.cpp : 定义控制台应用程序的入口点。
//
#include <functional>
//#include <conio.h>

#include <clstd.h>
#include <thread/clThread.h>
#include <socket/clSocket.h>
#include <socket/clSocketServer.h>
#include <socket/clSocketClient.h>
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

class MyServer : public clstd::TCPServer
{
    virtual void OnEvent(SOCKET sock, clstd::SocketEvent eEvent) override
    {
        byte data[1024];
        int result = 0;
        switch (eEvent)
        {
        case clstd::SE_CONNECT:
            CLOG("SE_CONNECT");
            break;
        case clstd::SE_ACCEPT:
            CLOG("SE_ACCEPT");
            break;
        case clstd::SE_READ:
            CLOG("SE_READ");
            result = Recv(sock, &data, sizeof(data));
            if (result < 0)
            {
                CloseSocket();
            }
            else
            {
                CLOG("接收(%d)", sock);
                clstd::DumpMemory((const char*)data, result);
            }
            break;
        case clstd::SE_WRITE:
            CLOG("SE_WRITE");
            break;
        case clstd::SE_CLOSE:
            CLOG("SE_CLOSE");
            break;
        }

    }
};

void StartAsServer()
{
    MyServer server;
    clstd::SocketResult result = server.OpenPort(8071);
    if (result == clstd::SocketResult_Ok)
    {
        CLOG("启动服务器");
        server.Start();
        server.Wait();
        CLOG("服务器关闭");
    }
}

//////////////////////////////////////////////////////////////////////////

class MyClient : public clstd::TCPClient
{
    virtual void OnEvent(clstd::SocketEvent eEvent) override
    {
        switch (eEvent)
        {
        case clstd::SE_CONNECT:
            CLOG("SE_CONNECT");
            break;
        case clstd::SE_ACCEPT:
            CLOG("SE_ACCEPT");
            break;
        case clstd::SE_READ:
            CLOG("SE_READ");
            break;
        case clstd::SE_WRITE:
            CLOG("SE_WRITE");
            break;
        case clstd::SE_CLOSE:
            CLOG("SE_CLOSE");
            break;
        }
    }
};

void StartAsClient()
{
    MyClient client;
    const char* szHello = "Hello world";
    client.Connect("127.0.0.1", 8071);
    while (!clstd_cli::kbhit())
    {
        Sleep(1000);
        CLOG("sending data");
        i32 result = client.Send(szHello, strlen(szHello));
        CLOG("send %d byte(s)", result);

        if (result < 0)
        {
            CLOG("断开链接，服务器可能关闭");
            client.Close(-1);
            break;
        }
    }
}
