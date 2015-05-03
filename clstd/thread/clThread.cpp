#include "clstd.h"
#include "clThread.h"
namespace clstd
{
#ifdef _WIN32
  namespace _win32
  {
    DWORD WINAPI ThreadStartRoutine(LPVOID lpThreadParameter)
    {
      Thread* pThread = static_cast<Thread*>(lpThreadParameter);
      return pThread->Run();
    }

    Thread::Thread()
      : m_handle(NULL)
      , m_ThreadId(NULL)
    {
    }

    Thread::~Thread()
    {
      if(this != NULL && m_handle != NULL && m_handle != INVALID_HANDLE_VALUE)
      {
        TerminateThread(m_handle, 0);
        CloseHandle(m_handle);
        m_handle = NULL;
        m_ThreadId = NULL;
      }
    }

    b32 Thread::Start()
    {
      if(m_handle == NULL) {
        // 不放到构造函数中启动是因为某些情况下可能需要用户选择是否另开线程
        // 比如UDP通讯,如果没有接受模式则不开启额外线程.
        m_handle = CreateThread(NULL, NULL, ThreadStartRoutine, this, CREATE_SUSPENDED, &m_ThreadId);
      }
      else if(m_handle == INVALID_HANDLE_VALUE) {
        return FALSE;
      }
      return ResumeThread(m_handle) != 0xffffffff;
    }

    u32 Thread::Wait(u32 nMilliSec)
    {
      DWORD dwThreadExitCode = 0;
      if(m_handle)
      {
        DWORD dwRet = WaitForSingleObject(m_handle, nMilliSec);
        GetExitCodeThread(m_handle, &dwThreadExitCode);
        CloseHandle(m_handle);
        m_handle = NULL;
        m_ThreadId = NULL;
      }
      return dwThreadExitCode;
    }
    
    i32 Thread::Run()
    {
      return 0;
    }
  } // namespace _win32
#endif // #ifdef _WIN32

#ifdef POSIX_THREAD
  namespace _posix
  {
    //
    // 没测试过!!!
    //

    void* ThreadStart(void* pParam)
    {
      Thread* pThread = static_cast<Thread*>(pParam);
      pthread_cond_wait(&pThread->m_cond, NULL);
      return (void*)pThread->Run();
    }

    Thread::Thread()
      : m_cond(NULL)
    {      
    }

    Thread::~Thread()
    {
      if(m_cond) {
        pthread_cond_destroy(&m_cond);
      }
      if(m_tidp.p) {
        pthread_cancel(m_tidp);
      }
    }

    b32 Thread::Start()
    {
      if(m_cond == NULL && m_tidp.p == NULL)
      {
        int err;
        memset(&m_tidp, 0, sizeof(m_tidp));

        err = pthread_cond_init(&m_cond, NULL);
        if(err != 0) {
          return FALSE;
        }

        err = pthread_create(&m_tidp, NULL, ThreadStart, (void*)this);
        if(err != 0) {
          pthread_cond_destroy(&m_cond);
          m_cond = NULL;
          return FALSE;
        }
      }
      
      if(m_cond && m_tidp.p) {
        return pthread_cond_signal(&m_cond) == 0;
      }
      else {
        // m_cond 与 m_tidp.p 应该同时产生和释放, 不应该出现这种状态
        CLBREAK;
      }
      return FALSE;
    }

    i32 Thread::Run()
    {
      return 0;
    }
  } // namespace _posix
#endif // #ifdef POSIX_THREAD

} // namespace clstd