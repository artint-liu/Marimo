#ifndef _CLSTD_THREAD_H_
#define _CLSTD_THREAD_H_

#ifdef POSIX_THREAD
#include <pthread.h>
#endif // #ifdef POSIX_THREAD

namespace clstd
{
  namespace thread
  {
    u32 GetCurrentId();
  } // namespace thread
#ifdef _WIN32
  namespace _win32
  {
    class Thread
    {
    protected:
      HANDLE m_handle;
      DWORD  m_ThreadId;
    public:
      Thread();
      virtual ~Thread();
      virtual i32 Run();

      b32   Start ();
      u32   Wait  (u32 nMilliSec);
    };
  } // namespace _win32
#endif // #ifdef _WIN32

#ifdef POSIX_THREAD
  namespace _posix
  {
    class Thread
    {
      friend void* ThreadStart(void* pParam);
    protected:
      pthread_t       m_tidp;
      pthread_cond_t  m_cond;
    public:
      Thread();
      virtual ~Thread();
      virtual i32 Run();

      b32 Start();
      u32 Wait(u32 nMilliSec);
    };
  } // namespace _posix
#endif // #ifdef POSIX_THREAD

#if defined(_WIN32) && !defined(POSIX_THREAD)
  typedef _win32::Thread Thread;
#else
  typedef _posix::Thread Thread;
#endif // #if defined(_Win32) && !defined(POSIX_THREAD)

} // namespace clstd

#endif // #ifndef _CLSTD_THREAD_H_