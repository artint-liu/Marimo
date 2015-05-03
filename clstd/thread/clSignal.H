#ifndef _CLSTD_SIGNAL_H_
#define _CLSTD_SIGNAL_H_

namespace clstd
{

#ifdef _WIN32
  namespace _win32
  {
    class Signal
    {
    private:
      HANDLE m_hEvent; // 默认初始化为阻塞状态，自动重置

    public:
      Signal();
      virtual ~Signal();

      //i32 Reset       (); // linux下没找到类似实现
      i32 Set         ();
      i32 Wait        ();
      i32 WaitTimeOut (u32 dwMilliSec);
    };
  } // namespace _win32
#endif // #ifdef _WIN32

#ifdef POSIX_THREAD
  namespace _posix
  {
    class Signal
    {
    private:
      pthread_cond_t    m_cond;
      pthread_mutex_t   m_mutex;

    public:
      Signal();
      virtual ~Signal();

      //i32 Reset       ();
      i32 Set         ();
      i32 Wait        ();
      i32 WaitTimeOut (u32 dwMilliSec);
    };
  }
#endif // #ifdef POSIX_THREAD

  //////////////////////////////////////////////////////////////////////////

#if defined(_WIN32) && !defined(POSIX_THREAD)
  typedef _win32::Signal Signal;
#else
  typedef _posix::Signal Signal;
#endif // #if defined(_Win32) && !defined(POSIX_THREAD)

} // namespace clstd

#endif // #ifndef _CLSTD_SIGNAL_H_
