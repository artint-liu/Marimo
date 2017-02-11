#ifndef _CLSTD_THREAD_H_
#define _CLSTD_THREAD_H_

#ifdef POSIX_THREAD
#include <pthread.h>
#endif // #ifdef POSIX_THREAD

#ifdef _CPLUSPLUS_11_THREAD
# if __cplusplus < 201103L
#   error 低版本C++编译器没有将thread纳入标准库
# endif // # if __cplusplus >= 201103L
#include <thread>
namespace clstd
{
  namespace c11
  {
    class Thread
    {
    protected:
      //HANDLE m_handle;
      //DWORD  m_ThreadId;
      //std::condition_variable m_cond;
      std::thread m_thread;
      //std::mutex m_mutex;
    public:
      Thread();
      virtual ~Thread();
      virtual i32 Run();

      b32   Start();
      u32   Wait(u32 nMilliSec);
    };
  } // namespace c11
} // namespace clstd
#endif // #ifdef _CPP11_THREAD


namespace clstd
{
  namespace this_thread
  {
#if _CPLUSPLUS_11_THREAD
    typedef std::this_thread::id id;
#elif defined(POSIX_THREAD)
    typedef size_t id;
#elif (defined(_WINDOWS) || defined(_WIN32))
    typedef size_t id;
#endif // #if (defined(_WINDOWS) || defined(_WIN32)) && !defined(POSIX_THREAD)

    id GetId();
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

#if defined(POSIX_THREAD) && 0
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

#if defined(_CPLUSPLUS_11_THREAD)
  typedef c11::Thread Thread;
#elif defined(_WIN32) && !defined(POSIX_THREAD)
  typedef _win32::Thread Thread;
#else
  typedef _posix::Thread Thread;
#endif // #if defined(_Win32) && !defined(POSIX_THREAD)

} // namespace clstd

#endif // #ifndef _CLSTD_THREAD_H_