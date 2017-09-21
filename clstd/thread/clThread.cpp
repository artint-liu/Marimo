#include "clstd.h"
#include "clThread.h"
#include "clSignal.h"

#if defined(_CL_SYSTEM_WINDOWS)
# include <sys/timeb.h>
#else
# include <sys/time.h>
#endif // #if defined(_CL_SYSTEM_WINDOWS)

namespace clstd
{
#ifdef _WIN32
  namespace _win32
  {
    DWORD WINAPI ThreadStartRoutine(LPVOID lpThreadParameter)
    {
      Thread* pThread = static_cast<Thread*>(lpThreadParameter);
      return pThread->StartRoutine();
    }

    Thread::Thread()
      : m_handle(NULL)
      , m_ThreadId(0)
      , m_dwExitCode(0)
    {
    }

    Thread::~Thread()
    {
      if(this != NULL && m_handle != NULL && m_handle != INVALID_HANDLE_VALUE)
      {
        TerminateThread(m_handle, 0);
        CloseHandle(m_handle);
        m_handle = NULL;
        m_ThreadId = 0;
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

    Thread::Result Thread::Wait(u32 nMilliSec)
    {
      if(m_handle)
      {
        DWORD dwRet = WaitForSingleObject(m_handle, nMilliSec);
        if(dwRet == WAIT_TIMEOUT) {
          return Result_TimeOut;
        }

        BOOL bResult = GetExitCodeThread(m_handle, &m_dwExitCode);
        ASSERT(bResult);

        CloseHandle(m_handle);
        m_handle = NULL;
        m_ThreadId = 0;
      }
      return Result_Ok;
    }
    
    Thread::Result Thread::GetExitCode(u32* pExitCode) const
    {
      if(m_handle) {
        return Result_Running;
      }
      ASSERT(m_ThreadId == 0);
      if(pExitCode != NULL) {
        *pExitCode = m_dwExitCode;
      }
      return Result_Ok;
    }

    this_thread::id Thread::GetId() const
    {
      return this_thread::GetId();
    }

    //i32 Thread::StartRoutine()
    //{
    //  return 0;
    //}
  } // namespace _win32
#endif // #ifdef _WIN32

#if defined(_CPLUSPLUS_11_THREAD) || (__cplusplus >= 201103L) || (_MSC_VER >= 1900)
  namespace cxx11
  {

    void* ThreadStart(void* pParam)
    {
      Thread* pThread = reinterpret_cast<Thread*>(pParam);
      pThread->m_dwExitCode = static_cast<u32>(pThread->StartRoutine());
      pThread->m_pWaitExit->Set();

      return static_cast<void*>(0);
    }

    Thread::Thread()
      : m_pThread(NULL)
      , m_pWaitExit(NULL)
      , m_dwExitCode(0)
    {
    }

    Thread::~Thread()
    {
      SAFE_DELETE(m_pWaitExit);
    }

    i32 Thread::StartRoutine()
    {
      return 0;
    }

    b32 Thread::Start()
    {
      //new(&m_thread) std::thread(std::bind(&Thread::StartRoutine, this));
      if(m_pThread == NULL)
      {
        if(m_pWaitExit == NULL) {
          m_pWaitExit = new Signal;
          if(m_pWaitExit == NULL) {
            return FALSE;
          }
        }

        m_pThread = new std::thread(ThreadStart, this);
        if(m_pThread == NULL) {
          return FALSE;
        }

        return TRUE;
      }
      return FALSE;
    }

    Thread::Result Thread::Wait(u32 nMilliSec)
    {
      if(nMilliSec != -1 && m_pWaitExit->WaitTimeOut(nMilliSec) == Signal::eTimeOut) {
        return Result_TimeOut;
      }

      if (m_pThread->joinable()) {
        m_pThread->join();
      }

      delete m_pThread;
      m_pThread = NULL;
      return Result_Ok;
    }

    Thread::Result Thread::GetExitCode(u32* pExitCode) const
    {
      if(m_pThread) {
        return Result_Running;
      }
      
      if(pExitCode) {
        *pExitCode = m_dwExitCode;
      }

      return Result_Ok;
    }

    this_thread::id Thread::GetId() const
    {
      return this_thread::GetId();
    }

  } // namespace cxx11
#endif // #if defined(_CPLUSPLUS_11_THREAD) || (__cplusplus >= 201103L) || (_MSC_VER >= 1900)

#if defined(POSIX_THREAD)
  namespace _posix
  {

    void* ThreadStart(void* pParam)
    {
      Thread* pThread = static_cast<Thread*>(pParam);
      //pthread_cond_wait(&pThread->m_cond, &pThread->m_mtx);
      pThread->m_pSignal->Wait();      
      pThread->m_dwExitCode = static_cast<u32>(pThread->StartRoutine());
      pThread->m_pWaitExit->Set();
      
      SAFE_DELETE(pThread->m_pSignal);
      return static_cast<void*>(0);
    }

    Thread::Thread()
      : m_tidp(NULL)
      , m_pSignal(NULL)
      , m_pWaitExit(NULL)
      , m_dwExitCode(0)
    {
      //m_tidp.p = NULL;
      //m_tidp.x = 0;
    }

    Thread::~Thread()
    {
      if(m_tidp) {
        pthread_cancel(*m_tidp);
        delete m_tidp;
        m_tidp = NULL;
      }

      SAFE_DELETE(m_pWaitExit);
      SAFE_DELETE(m_pSignal);
    }

    b32 Thread::Start()
    {
      if( ! m_pSignal)
      {
        int err;
        //memset(&m_tidp, 0, sizeof(m_tidp));
        m_tidp = new pthread_t;

        m_pSignal = new Signal;
        if( ! m_pSignal) {
          return FALSE;
        }

        m_pWaitExit = new Signal;
        if( ! m_pWaitExit) {
          return FALSE;
        }

        err = pthread_create(m_tidp, NULL, ThreadStart, (void*)this);
        if(err != 0) {
          SAFE_DELETE(m_pSignal);
          return FALSE;
        }
      }
      
      if(m_pSignal) {
        m_pSignal->Set();
        return TRUE;
      }
      else {
        CLBREAK;
      }
      return FALSE;
    }

    i32 Thread::StartRoutine()
    {
      return 0;
    }

    Thread::Result Thread::Wait(u32 nMilliSec)
    {
      if(nMilliSec != -1 && m_pWaitExit->WaitTimeOut(nMilliSec) == Signal::eTimeOut) {
        return Result_TimeOut;
      }

      pthread_join(*m_tidp, NULL);

      delete m_tidp;
      m_tidp = NULL;

      return Result_Ok;
    }

    Thread::Result Thread::GetExitCode(u32* pExitCode) const
    {
      if(m_tidp) {
        return Result_Running;
      }
      if(pExitCode) {
        *pExitCode = m_dwExitCode;
      }
      return Result_Ok;
    }

    this_thread::id Thread::GetId() const
    {
      return this_thread::GetId();
    }

    void abs_time_after(timespec* t, u32 uMilliSec)
    {
#if defined(_CL_SYSTEM_WINDOWS)
      _timeb time1970;
      _ftime(&time1970);

      t->tv_sec = time1970.time + (uMilliSec / 1000);
      t->tv_nsec = (time1970.millitm + (uMilliSec % 1000)) * 1000000;
#else
      timeval now;
      gettimeofday(&now, NULL);
      int nsec = now.tv_usec * 1000 + (uMilliSec % 1000) * 1000000;
      t->tv_sec = now.tv_sec + nsec / 1000000000 + uMilliSec / 1000;
      t->tv_nsec = nsec % 1000000000;
#endif
    }

  } // namespace _posix
#endif // #ifdef POSIX_THREAD

} // namespace clstd