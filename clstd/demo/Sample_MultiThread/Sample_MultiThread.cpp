﻿// Sample_MultiThread.cpp : 定义控制台应用程序的入口点。
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
  SampleTimeout t;
  t.Start();
  t.Wait(-1);
}

int main()
{
  TestWait();
  TestMessage();
  return 0;
}

