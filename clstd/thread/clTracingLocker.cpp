#include "clstd.h"
#include "clLocker.h"
#include "clString.h"
#include "clUtility.h"
#include "clTracingLocker.h"

namespace clstd
{
  TracingLocker::TracingLocker()
    : m_szFile(NULL)
    , m_nLine (0)
  {    
  }

  TracingLocker::~TracingLocker()
  {
    this->Locker::~Locker();
  }

  void TracingLocker::Lock()
  {
    m_szFile = NULL;
    m_nLine  = 0;
    Locker::Lock();
    // 成功锁定后才计时, 不计算锁定等待的时间
    m_Trace.Begin();
  }

  void TracingLocker::Lock(const char* szFile, int nLine)
  {
    m_szFile = szFile;
    m_nLine  = nLine;
    Locker::Lock();
    // 成功锁定后才计时, 不计算锁定等待的时间
    m_Trace.Begin();
  }

  void TracingLocker::Unlock()
  {
    Locker::Unlock();
  }

  b32 TracingLocker::Unlock(double rCriticalTime)
  {
    m_Trace.End();
    double dwDeltaTime = m_Trace.GetDeltaTime(); // 在unlock之外取DeltaTime可能得不到正确结果
    Locker::Unlock();
    if(dwDeltaTime > rCriticalTime)
    {
      CLOG_WARNING(">%s(%d): Lock time:%f\r\n", m_szFile ? m_szFile : "", m_nLine, dwDeltaTime);
      return TRUE;
    }
    return FALSE;
  }

  b32 TracingLocker::TryLock()
  {
    return Locker::TryLock();
  }
} // namespace clstd