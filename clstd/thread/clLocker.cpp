#include "clstd.h"
#include "clLocker.h"

namespace clstd
{
#if defined(_CPLUSPLUS_11_THREAD)
  namespace cxx11
  {
    Locker::Locker()
    {
    }
    Locker::~Locker()
    {
    }

    void Locker::Lock()
    {
      m_mutex.lock();
    }
    void Locker::Unlock()
    {
      m_mutex.unlock();
    }
    b32 Locker::TryLock()
    {
      m_mutex.try_lock();
    }
  } // namespace _posix
#endif // #if defined(_CPLUSPLUS_11_THREAD)

#ifdef POSIX_THREAD
  namespace _posix
  {
    Locker::Locker()
    {
      pthread_mutexattr_init(&m_mutexattr);
      //PTHREAD_MUTEX_RECURSIVE_NP
      pthread_mutexattr_settype(&m_mutexattr, PTHREAD_MUTEX_RECURSIVE);
      pthread_mutex_init(&m_mutex, &m_mutexattr);
    }

    Locker::~Locker()
    {
      pthread_mutex_destroy(&m_mutex);
      pthread_mutexattr_destroy(&m_mutexattr);
    }

    void Locker::Lock()
    {
      pthread_mutex_lock(&m_mutex);
    }

    void Locker::Unlock()
    {
      pthread_mutex_unlock(&m_mutex);
    }

    b32 Locker::TryLock()
    {
      return pthread_mutex_trylock(&m_mutex) == 0;
    }
  
  } // namespace _posix
#endif // #ifdef POSIX_THREAD

#if defined(_CL_SYSTEM_WINDOWS)
  namespace _win32
  {

    Locker::Locker()
    {
      InitializeCriticalSection(&m_CriticalSection);
    }

    Locker::~Locker()
    {
      DeleteCriticalSection(&m_CriticalSection);
    }

    void Locker::Lock()
    {
      EnterCriticalSection(&m_CriticalSection);
    }

    void Locker::Unlock()
    {
      LeaveCriticalSection(&m_CriticalSection);
    }

    b32 Locker::TryLock()
    {
      return TryEnterCriticalSection(&m_CriticalSection);
    }

  } // namespace _win32
#endif // #if defined(_CL_SYSTEM_WINDOWS)

} // namespace clstd