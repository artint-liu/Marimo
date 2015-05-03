#ifndef _CLSTD_TRACING_LOCKER_H_
#define _CLSTD_TRACING_LOCKER_H_

namespace clstd
{
  class TracingLocker : public Locker
  {
  private:
    int         m_nLine;
    CLLPCSTR    m_szFile;
    TimeTrace   m_Trace;
  public:
    TracingLocker();
    ~TracingLocker();

    void  Lock();
    void  Lock(CLLPCSTR szFile, int nLine);
    void  Unlock();
    b32   Unlock(double rCriticalTime);
    b32   TryLock();
  };

  //////////////////////////////////////////////////////////////////////////

  class ScopedTracingLocker
  {
  private:
    TracingLocker* const m_locker;
    double m_rCriticalTime;
  public:
    ScopedTracingLocker(TracingLocker* locker, CLLPCSTR szFile, int nLine, double rCriticalTime)
      : m_locker(locker)
      , m_rCriticalTime(rCriticalTime)
    { m_locker->Lock(szFile, nLine); }
    ScopedTracingLocker(TracingLocker& locker, CLLPCSTR szFile, int nLine, double rCriticalTime) 
      : m_locker(&locker)
      , m_rCriticalTime(rCriticalTime)
    { m_locker->Lock(szFile, nLine); }
    ~ScopedTracingLocker() { m_locker->Unlock(m_rCriticalTime); }
  };

  class ScopedSafeTracingLocker
  {
  private:
    TracingLocker* const m_locker;
    double m_rCriticalTime;
  public:
    ScopedSafeTracingLocker(TracingLocker* locker, CLLPCSTR szFile, int nLine, double rCriticalTime) 
      : m_locker(locker)
      , m_rCriticalTime(rCriticalTime)
    { if(locker) { locker->Lock(szFile, nLine); }}
    ~ScopedSafeTracingLocker() { if(m_locker) { m_locker->Unlock(m_rCriticalTime); }}
  };

  // TODO: 应该增加一个对加锁解锁进行时间跟踪的Locker或者参数

} // namespace clstd

#define BEGIN_SCOPED_TRACING_LOCKER(_LCK, _CT)      { clstd::ScopedTracingLocker lock##__LINE__(_LCK, __FILE__, __LINE__, _CT);
#define END_SCOPED_TRACING_LOCKER                   }

#define BEGIN_SCOPED_SAFE_TRACING_LOCKER(_LCK, _CT) { clstd::ScopedSafeTracingLocker lock##__LINE__(_LCK, __FILE__, __LINE__, _CT);
#define END_SCOPED_SAFE_TRACING_LOCKER              }

#endif // #ifndef #ifndef _CLSTD_TRACING_LOCKER_H_