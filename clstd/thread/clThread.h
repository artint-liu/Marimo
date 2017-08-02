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
      virtual i32 StartRoutine();

      b32   Start();
      u32   Wait(u32 nMilliSec);
    };

    namespace this_thread
    {
      typedef std::thread::id id;
      id GetId();
    }

  } // namespace cxx11
} // namespace clstd
#endif // #if defined(_CPLUSPLUS_11_THREAD) || (__cplusplus >= 201103L) || (_MSC_VER >= 1900)


namespace clstd
{
#ifdef _CL_SYSTEM_WINDOWS
  namespace _win32
  {
    class Thread
    {
    protected:
      HANDLE m_handle;
      DWORD  m_ThreadId;
      DWORD  m_dwExitCode;

    public:
      enum Result
      {
        Result_Ok = 0,
        Result_Running = 1,
        Result_TimeOut = 258L,
      };

    public:
      Thread();
      virtual ~Thread();
      virtual i32 StartRoutine();

      b32     Start ();
      Result  Wait  (u32 nMilliSec);
      Result  GetExitCode(u32* pExitCode) const;
    };

    namespace this_thread
    {
      typedef size_t id;
      id GetId();
    }

  } // namespace _win32
#endif // #ifdef _CL_SYSTEM_WINDOWS

#if defined(POSIX_THREAD)
  namespace _posix
  {
    class Signal;
    class Thread
    {
      friend void* ThreadStart(void* pParam);
    protected:
      pthread_t*      m_tidp;
      Signal*         m_pSignal;
      Signal*         m_pWaitExit;
      u32             m_dwExitCode;
      //pthread_cond_t  m_cond;
      //pthread_mutex_t m_mtx;

    public:
      enum Result
      {
        Result_Ok = 0,
        Result_Running = 1,
        Result_TimeOut = 258L,
      };

    public:
      Thread();
      virtual ~Thread();
      virtual i32 StartRoutine();

      b32 Start();
      Result Wait(u32 nMilliSec);
      Result GetExitCode(u32* pExitCode) const;
    };

    namespace this_thread
    {
      typedef size_t id;
      id GetId();
    }

    void abs_time_after(timespec* t, u32 uMilliSec);
  } // namespace _posix
#endif // #ifdef POSIX_THREAD

#if defined(_CPLUSPLUS_11_THREAD)
  typedef cxx11::Thread Thread;
  namespace this_thread
  {
    typedef cxx11::this_thread::id id;
    inline id GetId()
    {
      return cxx11::this_thread::GetId();
    }
  }
#elif defined(_WIN32) && !defined(POSIX_THREAD)
  typedef _win32::Thread Thread;
  namespace this_thread
  {
    typedef _win32::this_thread::id id;
    inline id GetId()
    {
      return _win32::this_thread::GetId();
    }
  }
#else
  typedef _posix::Thread Thread;
  namespace this_thread
  {
    typedef _posix::this_thread::id id;
    inline id GetId()
    {
      return _posix::this_thread::GetId();
    }
  }
#endif // #if defined(_Win32) && !defined(POSIX_THREAD)
 
} // namespace clstd

#endif // #ifndef _CLSTD_THREAD_H_