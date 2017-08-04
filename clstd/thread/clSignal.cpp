#include "clstd.h"
#include "thread/clSignal.h"

namespace clstd
{

#if defined(_CPLUSPLUS_11_THREAD) || (__cplusplus >= 201103L) || (_MSC_VER >= 1900)
  namespace cxx11
  {
    const i32 Signal::eTimeOut;

    Signal::Signal()
    {
    }

    Signal::~Signal()
    {
    }

    i32 Signal::Set()
    {
      m_cond.notify_all();
      return 1;
    }

    i32 Signal::Wait()
    {
      std::unique_lock<std::mutex> _lock(m_mutex);
      m_cond.wait(_lock);
      return 0;
    }

    i32 Signal::WaitTimeOut(u32 dwMilliSec)
    {
      std::unique_lock<std::mutex> _lock(m_mutex);
      return static_cast<i32>(m_cond.wait_for(_lock, std::chrono::milliseconds(dwMilliSec)));
    }
  } // namespace cxx11
#endif // #if defined(_CPLUSPLUS_11_THREAD) || (__cplusplus >= 201103L) || (_MSC_VER >= 1900)

#if defined(POSIX_THREAD)
  namespace _posix
  {
    void abs_time_after(timespec* t, u32 uMilliSec);

    Signal::Signal()
      : m_bSignaled(FALSE)
    {
      pthread_cond_init(&m_cond, NULL);
      pthread_mutex_init(&m_mutex, NULL);
    }

    Signal::~Signal()
    {
      pthread_cond_destroy(&m_cond);
      pthread_mutex_destroy(&m_mutex);
    }

    i32 Signal::Set()
    {
      pthread_mutex_lock(&m_mutex);
      m_bSignaled = TRUE;
      pthread_mutex_unlock(&m_mutex);
      return pthread_cond_signal(&m_cond);
    }

    i32 Signal::Wait()
    {
      i32 ret = 0;
      pthread_mutex_lock(&m_mutex);
      while( ! m_bSignaled)
      {
        ret = pthread_cond_wait(&m_cond, &m_mutex);
      }
      m_bSignaled = FALSE;
      pthread_mutex_unlock(&m_mutex);
      return ret;
    }

    i32 Signal::WaitTimeOut(u32 dwMilliSec)
    {
      if(dwMilliSec == TimeOut_Infinite)
      {
        return Wait();
      }

      timespec timeout;

//#if defined(_CL_SYSTEM_WINDOWS)
//      _timeb time1970;
//      _ftime(&time1970);
//
//      timeout.tv_sec = time1970.time + (dwMilliSec / 1000);
//      timeout.tv_nsec = (time1970.millitm + (dwMilliSec % 1000)) * 1000000;
//#else
//      timeval now;
//      gettimeofday(&now, NULL);
//      int nsec = now.tv_usec * 1000 + (dwMilliSec % 1000) * 1000000;
//      timeout.tv_nsec = nsec % 1000000000;
//      timeout.tv_sec = now.tv_sec + nsec / 1000000000 + dwMilliSec / 1000;
//#endif
      abs_time_after(&timeout, dwMilliSec);
      i32 ret = 0;
      pthread_mutex_lock(&m_mutex);
      if( ! m_bSignaled)
      {
        ret = pthread_cond_timedwait(&m_cond, &m_mutex, &timeout);
      }
      m_bSignaled = FALSE;
      pthread_mutex_unlock(&m_mutex);

      //const int ret = pthread_cond_timedwait(&m_cond, &m_mutex, &timeout);
      return ret;
  }

    //i32 Signal::Reset()
    //{
    //  STATIC_ASSERT(0);
    //}

}
#endif

#ifdef _WIN32
  namespace _win32
  {
    Signal::Signal()
      : m_hEvent(NULL)
    {
       m_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    }

    Signal::~Signal()
    {
      CloseHandle(m_hEvent);
    }

    i32 Signal::Set()
    {
      return SetEvent(m_hEvent);
    }

    i32 Signal::Wait()
    {
      return WaitForSingleObject(m_hEvent, -1);
    }

    i32 Signal::WaitTimeOut(u32 dwMilliSec)
    {
      return WaitForSingleObject(m_hEvent, dwMilliSec);
    }

    //i32 Signal::Reset()
    //{
    //  return (i32)ResetEvent(m_hEvent);
    //}
  } // namespace _win32
#endif // #ifdef _WIN32
} // namespace clstd
