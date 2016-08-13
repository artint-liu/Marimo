#ifndef _CLSTD_LOCKER_H_
#define _CLSTD_LOCKER_H_

#if (defined(_WINDOWS) || defined(_WIN32)) && !defined(POSIX_THREAD)
//#include <windows.h>
#else
# include <pthread.h>
#endif

namespace clstd
{
  class Locker
  {
  private:
#ifdef _WIN32
    CRITICAL_SECTION m_CriticalSection;
#else
    pthread_mutex_t m_mutex;
    pthread_mutexattr_t m_mutexattr;
#endif // _WINDOWS
  public:
    Locker();
    ~Locker();

    void Lock();
    void Unlock();
    b32 TryLock();
  };

  //////////////////////////////////////////////////////////////////////////

  class ScopedLocker
  {
  private:
    Locker* const m_locker;
  public:
    ScopedLocker(Locker* locker) 
      : m_locker(locker){ m_locker->Lock(); }
    ScopedLocker(Locker& locker) 
      : m_locker(&locker){ m_locker->Lock(); }
    ~ScopedLocker() { m_locker->Unlock(); }
  };

  class ScopedSafeLocker
  {
  private:
    Locker* const m_locker;
  public:
    ScopedSafeLocker(Locker* locker) 
      : m_locker(locker){ if(locker) { locker->Lock(); }}
    ~ScopedSafeLocker() { if(m_locker) { m_locker->Unlock(); }}
  };

  // TODO: 应该增加一个对加锁解锁进行时间跟踪的Locker或者参数

} // namespace clstd

#define BEGIN_SCOPED_LOCKER(_LCK)      { clstd::ScopedLocker lock##__LINE__(_LCK);
#define END_SCOPED_LOCKER              }

#define BEGIN_SCOPED_SAFE_LOCKER(_LCK) { clstd::ScopedSafeLocker lock##__LINE__(_LCK);
#define END_SCOPED_SAFE_LOCKER         }

#endif // _CLSTD_LOCKER_H_