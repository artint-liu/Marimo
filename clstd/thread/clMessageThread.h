#ifndef _CLSTD_THREAD_MESSAGE_H_
#define _CLSTD_THREAD_MESSAGE_H_

#ifndef _CLSTD_SIGNAL_H_
#error Must be include "clSignal.h" first.
#endif // _CLSTD_SIGNAL_H_


//#define POSIX_THREAD

#ifdef POSIX_THREAD
#include <pthread.h>
#endif // #ifdef POSIX_THREAD

#define _CLMSGTRD_TEMPL template<class _TMsg, class _TList>
#define _CLMSGTRD_IMPL  MsgThreadT<_TMsg, _TList>


struct CLMTCREATESTRUCT
{
  void* pThis;
  void* pUserParam;
};

typedef u32 (CL_CALLBACK *CLTHREADCALLBACK)(CLMTCREATESTRUCT*);

namespace clstd
{
  namespace thread
  {
    class Context
    {
    private:
      DWORD dwTlsIndex;
    public:
      Context();
      ~Context();
    };
  } // namespace thread

  //class MessageQueue
  //class MsgQueue
  //{
  //public:
  //  MsgQueue(u32 nMsgLength);
  //  ~MsgQueue();
  //};

  //////////////////////////////////////////////////////////////////////////
  struct THREADMSG
  {
    void*   handle;
    u32     message;
  };

  template<class _TMsg, class _TList = cllist<_TMsg>>
  class MsgThreadT : public Thread
  {
  protected:
    typedef clstd::Locker clLocker;
    typedef _TList QueueType;
//#if (defined(_WINDOWS) || defined(_WIN32)) && !defined(POSIX_THREAD)
//    HANDLE m_handle;
//    DWORD  m_dwID;
//    HANDLE m_hEvent;
//#else
//    pthread_t         m_tidp;
//    pthread_cond_t    m_cond;
//    pthread_mutex_t   m_mutex;
//#endif // (defined(_WINDOWS) || defined(_WIN32)) && !defined(POSIX_THREAD)
    Signal                m_signal;
    clLocker              m_locker;
    _TList                m_aMessage;
    //CLTHREADCALLBACK      m_pThreadStartup;
    //void*                 m_pUserPtr;
  protected:
    MsgThreadT();
    virtual ~MsgThreadT(){}

    //static u32 CL_CALLBACK ThreadEntryPoint(MsgThreadT* pThis);
  public:
    //static MsgThreadT*  CreateThread  (CLTHREADCALLBACK pCallBack, void* pParam);
    //static void         DestroyThread (MsgThreadT* pCLThread);

    // GetMessage 与 GetMessageTimeOut 遇到 -1 (Quit)消息后会阻断在这里, 不会将此消息从队列移除.
    // 如果希望强制移除Quit消息,应使用 PeekMessage 来进行处理
    b32       GetMessage        (_TMsg* pMsg);
    i32       GetMessageTimeOut (_TMsg* pMsg, u32 dwMSec); // -1: time out; 0: quit message, 1:normal message
    b32       WaitMessage       ();

    // 检查消息队列是否有消息, Quit消息会当作普通消息处理, 根据uRemoveMsg标志由用户来决定是否移除.
    b32       PeekMessage       (_TMsg* pMsg, void* handle, u32 dwMsgFilterMin, u32 dwMsgFilterMax, u32 uRemoveMsg);

    b32       PostMessage       (const _TMsg* pMsg);
    void      PostQuitMessage   (u32_ptr nExitCode);
    u32       WaitThreadQuit    (u32 nMilliSec);               // 至少之前调用了 PostQuitMessage, 这个内部会调用DestroyThread

    clLocker* GetLocker         ();

  };

  //_CLMSGTRD_TEMPL
  //  _CLMSGTRD_IMPL* _CLMSGTRD_IMPL::CreateThread(CLTHREADCALLBACK pCallBack, void* pParam)
  //{
  //  MsgThreadT* pThread = new MsgThreadT();
  //  pThread->m_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
  //  pThread->m_pThreadStartup = pCallBack;
  //  pThread->m_pUserPtr = pParam;
  //  pThread->m_handle =
  //    ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadEntryPoint, (LPVOID)pThread, NULL, &pThread->m_dwID);
  //  if(pThread->m_handle == INVALID_HANDLE_VALUE)
  //  {
  //    CloseHandle(pThread->m_hEvent);
  //    delete pThread;
  //    return NULL;
  //  }

  //  return pThread;
  //}

  _CLMSGTRD_TEMPL
    b32 _CLMSGTRD_IMPL::PostMessage(const _TMsg* pMsg)
  {
    //clstd::ScopeLocker lock(&m_locker);
    m_locker.Lock();
    m_aMessage.push_back(*pMsg);
    m_locker.Unlock();
    m_signal.Set();
    //SetEvent(m_hEvent);
    return TRUE;
  }

  //b32 _CLMSGTRD_IMPL::GetMessage(_TMsg* pMsg)
  _CLMSGTRD_TEMPL
    i32 _CLMSGTRD_IMPL::GetMessageTimeOut(_TMsg* pMsg, u32 nMSec)
  {
    //_TMsg msg;
    while(1)
    {
      m_locker.Lock();
      if(m_aMessage.size() > 0)
      {
        *pMsg = *m_aMessage.begin();
        const i32 ret = (i32)(pMsg->message != ((u32)-1));
        if(ret) {
          m_aMessage.erase(m_aMessage.begin());
        }
        m_locker.Unlock();
        return ret;
      }
      m_locker.Unlock();
      const DWORD ret = m_signal.WaitTimeOut(nMSec);
      if(ret == WAIT_TIMEOUT) {
        return -1;
      }
    }
    return 1;
  }

  _CLMSGTRD_TEMPL
    void _CLMSGTRD_IMPL::PostQuitMessage(u32_ptr nExitCode)
  {
    _TMsg msg;
    // memset 清空操作可能会破坏其成员的构造数据
    // memset(&msg, 0, sizeof(msg));
    msg.message = (u32)-1;
    msg.handle  = (void*)nExitCode;

    m_locker.Lock();
    m_aMessage.insert(m_aMessage.begin(), msg);
    //SetEvent(m_hEvent);
    m_locker.Unlock();
    m_signal.Set();
  }

  _CLMSGTRD_TEMPL
    u32 _CLMSGTRD_IMPL::WaitThreadQuit(u32 nMilliSec)
  {
    u32 dwExitCode = Thread::Wait(nMilliSec);
    //WaitForSingleObject(m_handle, -1);
    //dwExitCode = (u32)m_pUserPtr;
    //CloseHandle(m_hEvent);
    //CloseHandle(m_handle);
    //m_hEvent = NULL;
    //m_handle = NULL;
    //m_dwID = NULL;
    //delete this;
    return dwExitCode;
  }

  _CLMSGTRD_TEMPL
    b32 _CLMSGTRD_IMPL::WaitMessage()
  {
    //WaitForSingleObject(m_hEvent, -1);
    m_signal.WaitTimeOut(-1);
    return TRUE;
  }

//#if (defined(_WINDOWS) || defined(_WIN32)) && !defined(POSIX_THREAD)
//  _CLMSGTRD_TEMPL
//  _CLMSGTRD_IMPL* _CLMSGTRD_IMPL::CreateThread(CLTHREADCALLBACK pCallBack, void* pParam)
//  {
//    MsgThreadT* pThread = new MsgThreadT();
//    pThread->m_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
//    pThread->m_pThreadStartup = pCallBack;
//    pThread->m_pUserPtr = pParam;
//    pThread->m_handle =
//      ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadEntryPoint, (LPVOID)pThread, NULL, &pThread->m_dwID);
//    if(pThread->m_handle == INVALID_HANDLE_VALUE)
//    {
//      CloseHandle(pThread->m_hEvent);
//      delete pThread;
//      return NULL;
//    }
//
//    return pThread;
//  }
//
//  _CLMSGTRD_TEMPL
//  b32 _CLMSGTRD_IMPL::PostMessage(const _TMsg* pMsg)
//  {
//    clstd::ScopeLocker lock(&m_locker);
//    m_aMessage.push_back(*pMsg);
//    SetEvent(m_hEvent);
//    return TRUE;
//  }
//
//  //b32 _CLMSGTRD_IMPL::GetMessage(_TMsg* pMsg)
//  _CLMSGTRD_TEMPL
//  i32 _CLMSGTRD_IMPL::GetMessageTimeOut(_TMsg* pMsg, u32 nMSec)
//  {
//    //_TMsg msg;
//    while(1)
//    {
//      m_locker.Lock();
//      if(m_aMessage.size() > 0)
//      {
//        *pMsg = *m_aMessage.begin();
//        const i32 ret = (i32)(pMsg->message != ((u32)-1));
//        if(ret) {
//          m_aMessage.erase(m_aMessage.begin());
//        }
//        m_locker.Unlock();
//        return ret;
//      }
//      m_locker.Unlock();
//      const DWORD ret = WaitForSingleObject(m_hEvent, nMSec);
//      if(ret == WAIT_TIMEOUT) {
//        return -1;
//      }
//    }
//    return 1;
//  }
//
//  _CLMSGTRD_TEMPL
//  void _CLMSGTRD_IMPL::PostQuitMessage(u32_ptr nExitCode)
//  {
//    _TMsg msg;
//    // memset 清空操作可能会破坏其成员的构造数据
//    // memset(&msg, 0, sizeof(msg));
//    msg.message = (u32)-1;
//    msg.handle  = (void*)nExitCode;
//
//    m_locker.Lock();
//    m_aMessage.insert(m_aMessage.begin(), msg);
//    SetEvent(m_hEvent);
//    m_locker.Unlock();
//  }
//
//  _CLMSGTRD_TEMPL
//  u32 _CLMSGTRD_IMPL::WaitThreadQuit()
//  {
//    u32 dwExitCode;
//    WaitForSingleObject(m_handle, -1);
//    dwExitCode = (u32)m_pUserPtr;
//    CloseHandle(m_hEvent);
//    CloseHandle(m_handle);
//    m_hEvent = NULL;
//    m_handle = NULL;
//    m_dwID = NULL;
//    delete this;
//    return dwExitCode;
//  }
//
//  _CLMSGTRD_TEMPL
//  b32 _CLMSGTRD_IMPL::WaitMessage()
//  {
//    WaitForSingleObject(m_hEvent, -1);
//    return TRUE;
//  }
//
//#else
//
//  //
//  // 没测试过!!!
//  //
//
//  _CLMSGTRD_TEMPL
//  _CLMSGTRD_IMPL* _CLMSGTRD_IMPL::CreateThread(CLTHREADCALLBACK pCallBack, void* pParam)
//  //MsgThreadT* _CLMSGTRD_IMPL::CreateThread(CLTHREADCALLBACK pCallBack, void* pParam)
//  {
//    MsgThreadT* pThread = new MsgThreadT;
//    //pThread->m_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
//    pthread_cond_init(&pThread->m_cond, NULL);
//
//    pThread->m_pThreadStartup = pCallBack;
//    pThread->m_pUserPtr = pParam;
//    pthread_mutex_init(&pThread->m_mutex, NULL);
//    //pThread->m_handle =
//    //  ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadEntryPoint, (LPVOID)pThread, NULL, &pThread->m_dwID);
//
//    int err = pthread_create(&pThread->m_tidp, NULL, (void*(*)(void*))ThreadEntryPoint, (void*)pThread);
//    if(err != NULL)
//    {
//      delete pThread;
//      return NULL;
//    }
//    return pThread;
//  }
//
//  _CLMSGTRD_TEMPL
//  b32 _CLMSGTRD_IMPL::PostMessage(const _TMsg* pMsg)
//  {
//    clScopeLocker lock(&m_locker);
//    m_aMessage.push_back(*pMsg);
//    pthread_cond_signal(&m_cond);
//    return TRUE;
//  }
//
//  _CLMSGTRD_TEMPL
//  i32 _CLMSGTRD_IMPL::GetMessageTimeOut(_TMsg* pMsg, u32 nMSec)
//  {
//    while(1)
//    {
//      m_locker.Lock();
//      if(m_aMessage.size() > 0)
//      {
//        *pMsg = *m_aMessage.begin();
//        m_aMessage.erase(m_aMessage.begin());
//        m_locker.Unlock();
//        return pMsg->message != ((u32)-1);
//      }
//      m_locker.Unlock();
//      if(nMSec == (u32)-1) {
//        pthread_cond_wait(&m_cond, &m_mutex);
//      }
//      else {
//        timespec timeout;
//
//        // 这个算法不精确!
//        // TODO: windows版下这个tv_nsec时间貌似是无效的, 所以用了近似值来代替, 注意要实现其他平台的版本
//        timeout.tv_sec = time(0) + (nMSec + 999) / 1000;
//        timeout.tv_nsec = 0;
//
//        const int ret = pthread_cond_timedwait(&m_cond, &m_mutex, &timeout);
//        if(ret == ETIMEDOUT) {
//          return -1;
//        }
//      }
//    }
//    return TRUE;
//  }
//
//  _CLMSGTRD_TEMPL
//  b32 _CLMSGTRD_IMPL::WaitMessage()
//  {
//    pthread_cond_wait(&m_cond, &m_mutex);
//    return TRUE;
//  }
//
//  _CLMSGTRD_TEMPL
//  void _CLMSGTRD_IMPL::PostQuitMessage(u32_ptr nExitCode)
//  {
//    _TMsg msg;
//    // memset 清空操作可能会破坏其成员的构造数据
//    // memset(&msg, 0, sizeof(msg));
//    msg.message = (u32)-1;
//    msg.wParam = nExitCode;
//
//    m_locker.Lock();
//    m_aMessage.insert(m_aMessage.begin(), msg);
//    pthread_cond_signal(&m_cond);
//    m_locker.Unlock();
//
//    //return TRUE;
//  }
//
//  _CLMSGTRD_TEMPL
//  u32 _CLMSGTRD_IMPL::WaitThreadQuit()
//  {
//    u32 dwExitCode;
//    void* param;
//
//    pthread_join(m_tidp, &param);
//    dwExitCode = (u32)m_pUserPtr;
//    pthread_cond_destroy(&m_cond);
//    pthread_mutex_destroy(&m_mutex);
//
//    delete this;
//    return dwExitCode;
//  }
//#endif // (defined(_WINDOWS) || defined(_WIN32)) && !defined(POSIX_THREAD)

  _CLMSGTRD_TEMPL
  _CLMSGTRD_IMPL::MsgThreadT()
    //: m_pThreadStartup(NULL)
    //, m_pUserPtr      (NULL)
  {
  }

  //_CLMSGTRD_TEMPL
  //void _CLMSGTRD_IMPL::DestroyThread(MsgThreadT* pCLThread)
  //{
  //  delete pCLThread;
  //}

  _CLMSGTRD_TEMPL
  b32 _CLMSGTRD_IMPL::PeekMessage(_TMsg* pMsg, void* handle, u32 dwMsgFilterMin, u32 dwMsgFilterMax, u32 uRemoveMsg)
  {
    m_locker.Lock();
    size_t uMsgCount = m_aMessage.size();
    if(uMsgCount > 0)
    {
      for(_TList::iterator it = m_aMessage.begin();
        it != m_aMessage.end(); ++it)
      {
        if((handle == NULL || handle == it->handle) &&
          (it->message >= dwMsgFilterMin && (it->message <= dwMsgFilterMax || dwMsgFilterMax == 0)) )
        {
          if(pMsg != NULL)
            *pMsg = *it;
          if(uRemoveMsg == 0x0001/*PM_REMOVE*/)
            m_aMessage.erase(it);
          m_locker.Unlock();
          ASSERT(uMsgCount >= 1);
          return (b32)uMsgCount;
        }
      }
    }
    //ASSERT(m_aMessage.size() == 0);
    m_locker.Unlock();
    return FALSE;
  }


  //_CLMSGTRD_TEMPL
  //u32 _CLMSGTRD_IMPL::ThreadEntryPoint(MsgThreadT* pThis)
  //{
  //  CLMTCREATESTRUCT clst;
  //  clst.pThis = pThis;
  //  clst.pUserParam = pThis->m_pUserPtr;
  //  pThis->m_pUserPtr = (void*)pThis->m_pThreadStartup(&clst);
  //  return 0;
  //}

  _CLMSGTRD_TEMPL
  clstd::Locker* _CLMSGTRD_IMPL::GetLocker()
  {
    return &m_locker;
  }


  _CLMSGTRD_TEMPL
  b32 _CLMSGTRD_IMPL::GetMessage(_TMsg* pMsg)
  {
    const i32 val = GetMessageTimeOut(pMsg, -1);
    ASSERT(val == 0 || val == 1);
    return (b32)val;
  }

} // namespace clstd

//template<class _TMsg>
//class clMessageThreadT
//{
//#if (defined(_WINDOWS) || defined(_WIN32)) && !defined(POSIX_THREAD)
//  HANDLE m_handle;
//  DWORD  m_dwID;
//  HANDLE m_hEvent;
//#else
//  pthread_t         m_tidp;
//  pthread_cond_t    m_cond;
//  pthread_mutex_t   m_mutex;
//#endif // (defined(_WINDOWS) || defined(_WIN32)) && !defined(POSIX_THREAD)
//
//  //const u32             m_uElementSize;
//  clLocker              m_locker;
//  clvector<_TMsg>       m_aMessage;
//  CLTHREADCALLBACK      m_pThreadStartup;
//  void*                 m_pUserPtr;
//
//  clMessageThread();
//
//  static u32 CL_CALLBACK ThreadEntryPoint(clMessageThreadT* pThis);
//public:
//  static clMessageThread* CreateThread(CLTHREADCALLBACK pCallBack, void* pParam);
//  static void DestroyThread(clMessageThread* pCLThread);
//
//  b32     GetMessage        (_TMsg* pMsg);
//  i32     GetMessageTimeOut (_TMsg* pMsg, u32 dwMSec); // -1: time out; 0: quit message, 1:normal message
//  b32     WaitMessage       ();
//  b32     PeekMessage       (_TMsg* pMsg, void* handle, u32 dwMsgFilterMin, u32 dwMsgFilterMax, u32 uRemoveMsg);
//
//  //b32     PostMessage       (void* handle, u32 message, u32_ptr wParam, u32_ptr lParam); // time和pos信息都会被写成0
//  b32     PostMessage       (const _TMsg* pMsg);
//  void    PostQuitMessage   (u32_ptr nExitCode);
//  u32     WaitThreadQuit    ();                   // 至少之前调用了 PostQuitMessage, 这个内部会调用DestroyThread
//
//  clLocker* GetLocker       ();
//
//  static u32 GetCurrentId();
//  // PeekMessage
//  // SendMessageTimeOut
//};


#else
#pragma message(__FILE__": warning : Duplicate included this file.")
#endif // _CLSTD_THREAD_MESSAGE_H_