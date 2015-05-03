#include "clstd.h"
#include "clLocker.h"
#include "clString.H"
#include "clUtility.H"
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
    Locker::~Locker();
  }

  void TracingLocker::Lock()
  {
    m_szFile = NULL;
    m_nLine  = 0;
    Locker::Lock();
    // �ɹ�������ż�ʱ, �����������ȴ���ʱ��
    m_Trace.Begin();
  }

  void TracingLocker::Lock(const char* szFile, int nLine)
  {
    m_szFile = szFile;
    m_nLine  = nLine;
    Locker::Lock();
    // �ɹ�������ż�ʱ, �����������ȴ���ʱ��
    m_Trace.Begin();
  }

  void TracingLocker::Unlock()
  {
    Locker::Unlock();
  }

  b32 TracingLocker::Unlock(double rCriticalTime)
  {
    m_Trace.End();
    double dwDeltaTime = m_Trace.GetDeltaTime(); // ��unlock֮��ȡDeltaTime���ܵò�����ȷ���
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