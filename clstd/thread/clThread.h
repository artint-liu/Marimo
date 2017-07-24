#ifndef _CLSTD_THREAD_H_
#define _CLSTD_THREAD_H_

#ifdef POSIX_THREAD
#include <pthread.h>
#endif // #ifdef POSIX_THREAD

#if defined(_CPLUSPLUS_11_THREAD) || (__cplusplus >= 201103L) || (_MSC_VER >= 1900)
# if (__cplusplus < 201103L) && (_MSC_VER < 1900)
#   error 低版本C++编译器没有将thread纳入标准库
# endif // # if (__cplusplus < 201103L) || (_MSC_VER < 1900)
#include <thread>
namespace clstd
{
  namespace cxx11
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
  } // namespace cxx11
} // namespace clstd
#endif // #if defined(_CPLUSPLUS_11_THREAD) || (__cplusplus >= 201103L) || (_MSC_VER >= 1900)


namespace clstd
{
  namespace this_thread
  {
#if defined(_CPLUSPLUS_11_THREAD) || (__cplusplus >= 201103L) || (_MSC_VER >= 1900)
    namespace cxx11
    {
      typedef std::thread::id id;
      id GetId();
    }
#endif // #if defined(_CPLUSPLUS_11_THREAD) || (__cplusplus >= 201103L) || (_MSC_VER >= 1900)

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

#if defined(POSIX_THREAD)
  namespace _posix
  {
    class Signal;
    class Thread
    {
      friend void* ThreadStart(void* pParam);
    protected:
      pthread_t       m_tidp;
      Signal*         m_pSignal;
      //pthread_cond_t  m_cond;
      //pthread_mutex_t m_mtx;
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
  typedef cxx11::Thread Thread;
#elif defined(_WIN32) && !defined(POSIX_THREAD)
  typedef _win32::Thread Thread;
#else
  typedef _posix::Thread Thread;
#endif // #if defined(_Win32) && !defined(POSIX_THREAD)

} // namespace clstd

#endif // #ifndef _CLSTD_THREAD_H_