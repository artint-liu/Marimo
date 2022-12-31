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
  std::cout << "��\'R\'������ȡ����, ��\'W\'����д����. ���������˳�" << std::endl;
  char ch = clstd_cli::getch();
  std::cout << "ÿ������д��/��ȡһ��buffer����, ��������˳�" << std::endl;

  if (ch == 'r' || ch == 'R')
  {
    while (!clstd_cli::kbhit())
    {
      DWORD dwNum;
      sSharedBuffer.Read(reinterpret_cast<u8*>(&dwNum), sizeof(dwNum));
      std::cout << "��ȡbuffer����:" << (int)dwNum << std::endl;
      
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
      std::cout << "д��buffer����:" << (int)dwNum << std::endl;

      clstd::Sleep(2000);
    }
  }
}