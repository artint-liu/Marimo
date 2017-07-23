#define __SLEEP_0 Sleep

class CMessage
{
  static CMessage* s_pInstance;
  
  clstd::this_thread::id  m_id;     // ��Ϣ�����߳�id
  clstd::Signal           m_signal; // ��Ϣ�ź�

  clstd::Locker m_RemoteLocker;     // Զ����Ϣ������֤ͬʱֻ��һ��Զ����Ϣ�ڴ�����
  clstd::Signal m_RemoteSignal;     // Զ����Ϣ��ɵ��ź�
  volatile CLDWORD m_RemoteCode;
  volatile CLDWORD m_RemoteResult;

  clstd::Locker m_QueueLocker;
  cllist<CLDWORD> m_Queue;          // Post����Ϣ����

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

  // �����õ��㷨�����̺߳����߳�ʹ����ͬ�㷨����
  // Ȼ��Ƚ����̵߳ļ������Ƿ��������̸߳���
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

  static CLDWORD SendMessage(CLDWORD code) // �κ��̶߳����Ե���
  {
    return s_pInstance->SendMsg(code);
  }

  CLDWORD SendMsg(CLDWORD code)
  {
    ASSERT(code != 0);

    if(clstd::this_thread::GetId() == m_id) // ���ص�����ֱ�ӵ��ú���
    {
      return MyDemoProc(code);
    }
    else
    {
      clstd::ScopedLocker __locker(m_RemoteLocker);

      __SLEEP_0(5);       CLDWORD result = 0;
      __SLEEP_0(5);       m_RemoteResult = 0;

                          // ע������û��������Ϣ�����̸߳���m_RemoteCode��Ч��������
                          // ����m_RemoteCodeҪ�����й���׼�����Ժ�������
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

  static b32 GetMessage(CLDWORD* pCode) // ���̵߳���
  {
    ASSERT(clstd::this_thread::GetId() == s_pInstance->m_id);
    return s_pInstance->GetMsg(pCode);
  }

  b32 GetMsg(CLDWORD* pCode) // ���̵߳���
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

  virtual i32 Run() override
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
