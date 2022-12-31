#include <iostream>
#include "clstd.h"
#include "clUtility.h"
#include "clBuffer.h"
//#include "clLinkedList.h"
#include "Samples_SharedMemory.h"
//#include "clSchedule.h"

using namespace clstd;

namespace clstd_sample
{

} // namespace clstd_sample

int main()
{
  clstd::SharedBuffer sSharedBuffer("TestSharedMemory");
  std::cout << "按\'R\'启动读取进程, 按\'W\'启动写进程. 其它按键退出" << std::endl;
  char ch = clstd_cli::getch();
  std::cout << "每隔两秒写入/读取一次buffer内容, 按任意键退出" << std::endl;

  if (ch == 'r' || ch == 'R')
  {
    while (!clstd_cli::kbhit())
    {
      DWORD dwNum;
      sSharedBuffer.Read(reinterpret_cast<u8*>(&dwNum), sizeof(dwNum));
      std::cout << "读取buffer内容:" << (int)dwNum << std::endl;
      
      clstd::Sleep(2000);
    }
  }
  else if (ch == 'w' || ch == 'W')
  {
    clstd::Rand rnd;
    rnd.srand(clstd::GetTime());

    while (!clstd_cli::kbhit())
    {
      DWORD dwNum = rnd.rand();
      sSharedBuffer.Write(reinterpret_cast<u8*>(&dwNum), sizeof(dwNum));
      std::cout << "写入buffer内容:" << (int)dwNum << std::endl;

      clstd::Sleep(2000);
    }
  }
}