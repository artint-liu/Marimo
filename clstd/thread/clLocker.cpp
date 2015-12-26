#include "clstd.h"
#include "clLocker.h"

namespace clstd
{
#ifdef _WIN32
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
#else  // _WINDOWS

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

#endif // _WINDOWS
} // namespace clstd