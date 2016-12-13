#ifndef _CLSTD_SIGNAL_H_
#define _CLSTD_SIGNAL_H_

#if defined(_CPLUSPLUS_11_THREAD)
#include <condition_variable>
#include <mutex>
#endif

namespace clstd
{
#if defined(_CPLUSPLUS_11_THREAD)
  namespace c11
  {
    class Signal
    {
    private:
      std::condition_variable m_cond;
      std::mutex              m_mutex;

    public:
      static const i32 eTimeOut = static_cast<i32>(std::cv_status::timeout);
      Signal();
      virtual ~Signal();

      //i32 Reset       ();
      i32 Set();
      i32 Wait();
      i32 WaitTimeOut(u32 dwMilliSec);
    };
  } // namespace c11
#elif defined(POSIX_THREAD)
  namespace _posix
  {
    class Signal
    {
    private:
      pthread_cond_t    m_cond;
      pthread_mutex_t   m_mutex;

    public:
      static const i32 eTimeOut = ETIMEDOUT;
      Signal();
      virtual ~Signal();

      //i32 Reset       ();
      i32 Set();
      i32 Wait();
      i32 WaitTimeOut(u32 dwMilliSec);
    };
  }
#endif // #if defined(_CPLUSPLUS_11_THREAD)

#ifdef _WIN32
  namespace _win32
  {
    class Signal
    {
    private:
      HANDLE m_hEvent; // 默认初始化为阻塞状态，自动重置

    public:
      static const i32 eTimeOut = WAIT_TIMEOUT;
      Signal();
      virtual ~Signal();

      //i32 Reset       (); // linux下没找到类似实现
      i32 Set         ();
      i32 Wait        ();
      i32 WaitTimeOut (u32 dwMilliSec);
    };
  } // namespace _win32
#endif // #ifdef _WIN32


  //////////////////////////////////////////////////////////////////////////
#if defined(_CPLUSPLUS_11_THREAD)
  typedef c11::Signal Signal;
#elif defined(_WIN32) && !defined(POSIX_THREAD)
  typedef _win32::Signal Signal;
#else
  typedef _posix::Signal Signal;
#endif // #if defined(_Win32) && !defined(POSIX_THREAD)

} // namespace clstd

#endif // #ifndef _CLSTD_SIGNAL_H_
