#ifndef _CLSTD_THREAD_QUEUE_H_
#define _CLSTD_THREAD_QUEUE_H_

namespace clstd
{
  template<class _TMsg, class _TQueue = clqueue<_TMsg>>
  class ThreadQueueT
  {
  protected:
    Locker  m_locker;
    Signal  m_signal;
    _TQueue m_queue;

  public:
    b32 Get(_TMsg* pMsg)
    {
      return GetTimeOut(pMsg, -1);
    }

    i32 GetTimeOut(_TMsg* pMsg, u32 dwMilliSec) // -1: time out; 0: normal message
    {
      while(1)
      {
        m_locker.Lock();
        if(m_queue.size() > 0)
        {
          *pMsg = *m_queue.begin();
          //const i32 ret = (i32)(pMsg->message != ((u32)-1));
          //if(ret) {
          m_aMessage.erase(m_aMessage.begin());
          //}
          m_locker.Unlock();
          return 0;
        }

        m_locker.Unlock();
        const i32 ret = m_signal.WaitTimeOut(m_hEvent, dwMilliSec);
        if(ret == __WAIT_TIMEOUT) {
          return -1;
        }
      }
      return 1;

    }

    b32 Wait()
    {

    }

    b32 Peek(_TMsg* pMsg, b32 bRemoveMsg)
    {

    }

    b32 Post(const _TMsg* pMsg)
    {
      m_locker.Lock();
      m_queue.push_back(pMsg);
      m_locker.Unlock();
      return TRUE;
    }
  };

  //class ThreadQueue
  //{

  //};
} // namespace clstd

#endif // #ifndef _CLSTD_THREAD_QUEUE_H_