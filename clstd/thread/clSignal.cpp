#include "clstd.h"
#include "Thread/clSignal.H"

namespace clstd
{

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

#ifdef POSIX_THREAD
  namespace _posix
  {
    //
    // 没测试过!!
    //
    Signal::Signal()
      : m_cond(NULL)
      , m_mutex(NULL)
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
      return pthread_cond_signal(&m_cond);
    }

    i32 Signal::Wait()
    {
      return pthread_cond_wait(&m_cond, &m_mutex);
    }

    i32 Signal::WaitTimeOut(u32 dwMilliSec)
    {
      timespec timeout;

      // 这个算法不精确!
      // TODO: windows版下这个tv_nsec时间貌似是无效的, 所以用了近似值来代替, 注意要实现其他平台的版本
      timeout.tv_sec = time(0) + (dwMilliSec + 999) / 1000;
      timeout.tv_nsec = 0;

      const int ret = pthread_cond_timedwait(&m_cond, &m_mutex, &timeout);
      return ret;
    }

    //i32 Signal::Reset()
    //{
    //  STATIC_ASSERT(0);
    //}

  }
#endif // #ifdef POSIX_THREAD
} // namespace clstd
