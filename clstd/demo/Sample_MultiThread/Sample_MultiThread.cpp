// Sample_MultiThread.cpp : 定义控制台应用程序的入口点。
//

#include "clstd.h"
#include "thread/clThread.h"
#include "thread/clSignal.h"
#include "clUtility.h"

#if defined(_CL_SYSTEM_LINUX)
#include <unistd.h>
#endif

#include "Sample_MultiThread.h"

void TestMessage()
{
  CLOG(__FUNCTION__);

  const int COUNT_THREAD = 51;
  SampleClient* pArray[COUNT_THREAD] = {};
  CMessage::Initialize();

  for(int i = 0; i < countof(pArray); i++)
  {
    pArray[i] = new SampleClient(i);
    pArray[i]->Start();
  }

  int exit_count = 0;
  while(exit_count != COUNT_THREAD)
  {
    CLDWORD code;
    if(!CMessage::GetMessage(&code))
    {
      exit_count++;
      CLOG("main: exit count(%d)", exit_count);
      continue;
    }
    CLDWORD result = CMessage::MyDemoProc(code);
    CLOG("main: fn(%08x)=%08x.", code, result);
    CLNOP
  }

  for(int i = 0; i < countof(pArray); i++)
  {
    pArray[i]->Wait(-1);
    CLOG("main: thread %d has been exit.", i);
    SAFE_DELETE(pArray[i]);
  }

  CLOG("main: all thread exit.");
  CMessage::Finalize();
}

void TestWait()
{
  CLOG(__FUNCTION__);
  const int sample_count = 10;
  SampleWait* t[sample_count];
  clstd::Signal s;

  for(int i = 0; i < sample_count; i++)
  {
    t[i] = new SampleWait(&s);
    t[i]->Start();
  }

  // 这句放在这里希望上面能够测试多线程中文log输出
  CLOG("测试多线程使用同一个信号等待时，激活信号的逻辑，设计上希望只有一个线程醒来");

  for(int i = 0; i < sample_count; i++)
  {
    clstd_cli::getch();
    CLOG("main: notify");
    s.Set();
  }

  for(int i = 0; i < sample_count; i++)
  {
    SAFE_DELETE(t[i]);
  }
}

void TestWaitTimeout()
{
  CLOG(__FUNCTION__);
  SampleTimeout t;
  t.Start();
  while(t.Wait(2000) == clstd::Thread::Result_TimeOut) {
    CLOG_WARNING("main: has been waited 2000ms");
  }
  CLOG("main: has been exited");
  u32 code = 0;
  clstd::Thread::Result result = t.GetExitCode(&code);
  ASSERT(result == clstd::Thread::Result_Ok);
  ASSERT(code == 8282);
}

namespace sample
{
  void pause()
  {
    CLOG("press any key to continue ...");
    clstd_cli::getch();
  }
} // namespace sample

int main()
{
  TestWait();
  sample::pause();

  TestWaitTimeout();
  sample::pause();

  TestMessage();
  return 0;
}

