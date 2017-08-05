#define __SLEEP_0(x) clstd::Sleep(x)

class CMessage
{
  static CMessage* s_pInstance;
  
  clstd::this_thread::id  m_id;     // 消息处理线程id
  clstd::Signal           m_signal; // 消息信号

  clstd::Locker m_RemoteLocker;     // 远程消息锁，保证同时只有一个远程消息在处理中
  clstd::Signal m_RemoteSignal;     // 远程消息完成的信号
  volatile CLDWORD m_RemoteCode;
  volatile CLDWORD m_RemoteResult;

  clstd::Locker m_QueueLocker;
  cllist<CLDWORD> m_Queue;          // Post的消息队列

  CMessage()
    : m_RemoteCode(0)
    , m_RemoteResult(0)
  {}

public:
  static void Initialize()
  {
    s_pInstance = new CMessage;
    s_pInstance->m_id = clstd::this_thread::GetId();
  }

  static void Finalize()
  {
    ASSERT(clstd::this_thread::GetId() == s_pInstance->m_id);
    SAFE_DELETE(s_pInstance);
  }

  // 测试用的算法，主线程和子线程使用相同算法计算
  // 然后比较主线程的计算结果是否被其它子线程干扰
  static CLDWORD MyDemoProc(CLDWORD code)
  {
    ASSERT(clstd::this_thread::GetId() == s_pInstance->m_id);
    return MyDemoProcAny(code);
  }

  static CLDWORD MyDemoProcAny(CLDWORD code)
  {
    CLDWORD result = clstd::GenerateCRC32((CLBYTE*)&code, sizeof(code));
    CLOG("crc32(%08x)=%08x", code, result);
    return result;
  }

  static CLDWORD SendMessage(CLDWORD code) // 任何线程都可以调用
  {
    return s_pInstance->SendMsg(code);
  }

  CLDWORD SendMsg(CLDWORD code)
  {
    ASSERT(code != 0);

    if(clstd::this_thread::GetId() == m_id) // 本地调用是直接调用函数
    {
      return MyDemoProc(code);
    }
    else
    {
      clstd::ScopedLocker __locker(m_RemoteLocker);

      __SLEEP_0(5);       CLDWORD result = 0;
      __SLEEP_0(5);       m_RemoteResult = 0;

                          // 注意这里没有锁，消息处理线程根据m_RemoteCode有效性来处理
                          // 所以m_RemoteCode要在所有工作准备好以后再设置
      __SLEEP_0(5);       m_RemoteCode = code;
      __SLEEP_0(5);       m_signal.Set();
      __SLEEP_0(5);       m_RemoteSignal.Wait();        ASSERT(m_RemoteResult != 0);
      __SLEEP_0(5);       result = m_RemoteResult;      ASSERT(result != 0);
      __SLEEP_0(5);       return result;
    }
  }

  static b32 PostMessage(CLDWORD code)
  {
    return s_pInstance->PostMsg(code);
  }

  b32 PostMsg(CLDWORD code)
  {
    m_QueueLocker.Lock();
    m_Queue.push_back(code);
    m_QueueLocker.Unlock();
    m_signal.Set();
    return TRUE;
  }

  static b32 GetMessage(CLDWORD* pCode) // 主线程调用
  {
    ASSERT(clstd::this_thread::GetId() == s_pInstance->m_id);
    return s_pInstance->GetMsg(pCode);
  }

  b32 GetMsg(CLDWORD* pCode) // 主线程调用
  {
    ASSERT(clstd::this_thread::GetId() == s_pInstance->m_id);
    while(1)
    {
      if(m_RemoteCode) {
        ResponseRemoteCall();
      }

      {
        clstd::ScopedLocker __lock(m_QueueLocker);
        if( ! m_Queue.empty()) {
          *pCode = m_Queue.front();
          m_Queue.pop_front();
          break;
        }
      }

      if(m_RemoteCode) {
        ResponseRemoteCall();
        continue;
      }

      m_signal.WaitTimeOut(clstd::Signal::TimeOut_Infinite);
    }
    return (*pCode != (CLDWORD)-1);
  }

  void ResponseRemoteCall()
  {
    __SLEEP_0(5);    m_RemoteResult = MyDemoProc(m_RemoteCode);   ASSERT(m_RemoteResult != 0);
    __SLEEP_0(5);    m_RemoteCode = 0;
    __SLEEP_0(5);    m_RemoteSignal.Set();
  }
};

CMessage* CMessage::s_pInstance;


class SampleClient : public clstd::Thread
{
  int m_index;
public:
  SampleClient(int index)
    : m_index(index)
  {
  }

  virtual i32 StartRoutine() override
  {
    clstd::Rand r;
    CLOG("thread[%d]: start.", m_index);
    r.srand((m_index << 5) | (u32)clstd::GetTime64());
    for(int i = 0; i < 100; i++)
    {
      CLDWORD code = r.rand();
      while(code == 0) {
        code = r.rand();
      }

      code |= (m_index << 24);

      if((r.rand() % 100) < 70)
      {
        CLDWORD result = CMessage::SendMessage(code);
        CLDWORD result_ref = CMessage::MyDemoProcAny(code);
        ASSERT(result_ref == result);
        CLOG("thread[%d]: fn(%08x)=%08x.", m_index, code, result);
      }
      else {
        CMessage::PostMessage(code);
        CLOG("thread[%d]: post(%08x).", m_index, code);
        continue;
      }
      //Sleep(30 + r.rand() % 70);
    }

    CMessage::PostMessage(CLDWORD(-1));
    CLOG("thread[%d]: exit.", m_index);
    return 0;
  }

};

// 测试clstd::Signal等待
class SampleWait : public clstd::Thread
{
  clstd::Signal* m_pSignal;
public:
  SampleWait(clstd::Signal* pSignal)
    : m_pSignal(pSignal) {}

  virtual i32 StartRoutine() override
  {
    CLOG("thread(%u): 就绪", GetId());
    m_pSignal->Wait();
    CLOG("thread(%u): 收到通知!", GetId());
    CLOG("thread(%u): exit...", GetId());
    return 8282;
  }
};


// 测试clstd::Signal等待超时
class SampleTimeout : public clstd::Thread
{
  clstd::Signal m_Signal;
public:
  virtual i32 StartRoutine() override
  {
    CLOG("thread(%u): 等待1100毫秒x5次", clstd::this_thread::GetId());
    for(int i = 0; i < 5; i++)
    {
      u64 time_begin = clstd::GetTime64();
      m_Signal.WaitTimeOut(1100);
      u64 time_end = clstd::GetTime64();
      i64 delta = (i64)(time_end - time_begin);
      CLOG("thread: wait time:%lld", delta);
    }

    CLOG("thread: 等待5秒退出");
    __SLEEP_0(5000);
    CLOG("thread: exit...");
    return 8282;
  }
};

