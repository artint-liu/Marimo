#include "clstd.h"
#include "clSchedule.h"

namespace clstd
{
  TIMER* Schedule::Find(void* handle, u32 id, b32 bRemove)
  {
    HandleDict::iterator iterHandle = m_Handles.find(handle);
    if(iterHandle == m_Handles.end()) {
      return NULL;
    }

    IdDict::iterator iterId = iterHandle->second.find(id);
    if(iterId == iterHandle->second.end()) {
      return NULL;
    }

    TIMER* pTimer = iterId->second;

    // 删除对象，但不释放内存
    if(bRemove)
    {
      iterHandle->second.erase(iterId);

      if(iterHandle->second.empty()) {
        m_Handles.erase(iterHandle);
      }
    }

    return pTimer;
  }

  void Schedule::InsertInOrder(TIMER* t) // 按发生顺序插入到表里
  {
    ASSERT(t->pNext == NULL); // 检查已经不在顺序表里（一定程度上防止）

    CountDownOrder::iterator iter_order = m_Order.find(t->timing);
    if(iter_order == m_Order.end()) {
      m_Order.insert(clmake_pair(t->timing, TimerChain(t)));
    }
    else {
      iter_order->second.push_back(t);
      //t->pNext = iter_order->second;
      //iter_order->second = t;
    }
  }

  //TIMER* Schedule::RemoveFrontFromTimerChain(CountDownOrder::iterator& it)
  //{
  //  TIMER* pFront = it->second;

  //  return pFront;
  //}

  Schedule::Schedule()
    //: m_CurrTick(0)
  {
  }

  Schedule::~Schedule()
  {
    m_Order.clear();
    for(auto iter_handle = m_Handles.begin(); iter_handle != m_Handles.end(); ++iter_handle)
    {
      IdDict& id_dict = iter_handle->second;
      for(auto iter_id = id_dict.begin(); iter_id != id_dict.end(); ++iter_id)
      {
        delete (iter_id->second);
        iter_id->second = NULL;
      }
      id_dict.clear();
    }
    m_Handles.clear();
  }

  void Schedule::CreateTimer(void* handle, u32 id, TIMER::tick_t elapse, TimerProc proc)
  {
    if(elapse == 0) {
      return;
    }

    TIMER* t = Find(handle, id, FALSE);
    if(t) {
      t->elapse = elapse;
      t->proc = proc;

      CountDownOrder::iterator iter_order = m_Order.find(t->timing);
      ASSERT(iter_order != m_Order.end()); // 肯定有啊！

      //// 从队列中移除timer，移除后队列对应位置可能是个NULL
      //TIMER** ppCurr = &iter_order->second;
      //do {
      //  if(*ppCurr == t)
      //  {
      //    *ppCurr = t->pNext; // t->pNext 有可能是NULL
      //    t->pNext = NULL;
      //    break;
      //  }
      //  ppCurr = &((*ppCurr)->pNext);
      //} while(*ppCurr);
      
      iter_order->second.erase(t);
      //if(iter_order->first != 0 && iter_order->second.empty())
      //{
      //  // t马上要加入m_Order[0]，所以m_Order[0]即使为空也不删
      //  m_Order.erase(iter_order);
      //}

      t->pNext  = NULL;     
      t->timing = 0;

      // 肯定没有删除标记
      ASSERT(TEST_FLAG_NOT(t->flags, TIMER::TimerFlag_Useless));
    }
    else {
      t = new TIMER(handle, id, elapse, proc);
      HandleDict::iterator iterHandle = m_Handles.find(handle);
      if(iterHandle == m_Handles.end())
      {
        IdDict dict;
        iterHandle = m_Handles.insert(clmake_pair(handle, dict)).first;
      }
      iterHandle->second.insert(clmake_pair(id, t));
    }

    ASSERT(t->timing == 0);
    ASSERT(t->pNext == NULL);

    InsertInOrder(t);
  }

  void Schedule::DestroyTimer(void* handle, u32 id)
  {
    TIMER* t = Find(handle, id, TRUE);
    if(t) {
      SET_FLAG(t->flags, TIMER::TimerFlag_Useless);
    }
  }

  void Schedule::RemoveByHandle(void* handle)
  {
    HandleDict::iterator iter_handle_timers = m_Handles.find(handle);
    if(iter_handle_timers == m_Handles.end()) {
      return;
    }

    // 只做废弃标记，在下一个update循环里会删除这些TIMER
    IdDict& id_dict = iter_handle_timers->second;
    for(auto iter_id = id_dict.begin(); iter_id != id_dict.end(); ++iter_id)
    {
      // handle下的所有timer标记为弃用
      SET_FLAG(iter_id->second->flags, TIMER::TimerFlag_Useless);
    }

    m_Handles.erase(iter_handle_timers);
  }

  //TIMER::tick_t Schedule::GetNearestTick(TIMER::tick_t curr_tick) const
  //{
  //  //m_CurrTick = curr_tick;
  //  if(m_Order.empty()) {
  //    return (TIMER::tick_t)-1;
  //  }

  //  // 发生时间不会返回负值
  //  TIMER* t = m_Order.begin()->second.front();
  //  if(t->timing < curr_tick) {
  //    return 0;
  //  }
  //  else {
  //    return t->timing - curr_tick;
  //  }
  //}

  TIMER::tick_t Schedule::GetTimer(TIMER::tick_t curr_tick, TIMER* pOutTimer)
  {
    TIMER* t = NULL;
    
    do {
      if(m_Order.empty()) {
        return (TIMER::tick_t) - 1;
      }

      TimerChain& rChain = m_Order.begin()->second;
      
      if(rChain.empty()) {
        m_Order.erase(m_Order.begin());
      }
      else
      {
        t = rChain.front();
        if(TEST_FLAG(t->flags, TIMER::TimerFlag_Useless))
        {
          rChain.pop_front();
          delete t;
          t = NULL;
        }
      }
    } while(t == NULL);

    //t = m_Order.begin()->second.front();


    if(t->timing <= curr_tick) {
      pOutTimer->handle = t->handle;
      pOutTimer->id     = t->id;
      pOutTimer->proc   = t->proc;
      pOutTimer->elapse = t->elapse;

      //if(t->pNext == NULL)
      //{
      //  m_Order.erase(m_Order.begin());
      //}
      //else
      //{
      //  m_Order.begin()->second = t->pNext;
      //  t->pNext = NULL;
      //}
      m_Order.begin()->second.pop_front();

      if(t->timing == 0) {
        t->timing = curr_tick + t->elapse;
      } else {
        t->timing += t->elapse;
      }
      t->pNext = NULL;
      InsertInOrder(t);

      return 0;
    }
    else {
      return t->timing - curr_tick;
    }
  }

  //////////////////////////////////////////////////////////////////////////

  Schedule::TimerChain::TimerChain()
    : m_pFront(NULL)
    , m_pBack(NULL)
    , m_count(0)
  {
  }

  Schedule::TimerChain::TimerChain(TIMER* pTimer)
    : m_pFront(pTimer)
    , m_pBack(pTimer)
    , m_count(1)
  {
  }

  void Schedule::TimerChain::push_back(TIMER* pTimer)
  {
    if(m_count == 0)
    {
      ASSERT(m_pFront == NULL && m_pBack == NULL);
      m_pFront = m_pBack = pTimer;
    }
    else
    {
      m_pBack->pNext = pTimer;
      m_pBack = pTimer;
    }
    m_count++;
  }

  TIMER* Schedule::TimerChain::pop_front()
  {
    ASSERT(m_count > 1 || m_pFront == m_pBack);

    TIMER* pFront = m_pFront;
    if(m_count)
    {
      m_pFront = m_pFront->pNext;
      m_count--;

      if(m_count == 0)
      {
        ASSERT(m_pFront == NULL && pFront == m_pBack);
        m_pBack = NULL;
      }
    }
    return pFront;
  }

  TIMER* Schedule::TimerChain::front() const
  {
    return m_pFront;
  }

  size_t Schedule::TimerChain::size() const
  {
    return m_count;
  }

  b32 Schedule::TimerChain::empty() const
  {
    return (m_count == 0);
  }

  void Schedule::TimerChain::clear()
  {
    m_pFront = NULL;
    m_pBack = NULL;
    m_count = 0;
  }

  b32 Schedule::TimerChain::erase(const TIMER* pTimer)
  {
    if(pTimer == NULL || m_pFront == NULL) {
      return FALSE;
    }
    else if(m_pFront == pTimer)
    {
      pop_front();
      return TRUE;
    }

    TIMER* pPrev = m_pFront;
    do 
    {
      if(pPrev->pNext == pTimer) {
        pPrev->pNext = pTimer->pNext;
        m_count--;
        return TRUE;
      }
      pPrev = pPrev->pNext;
    } while (pPrev);
    return FALSE;
  }

  TIMER* Schedule::TimerChain::find(const TIMER* pTimer)
  {
    if(pTimer == NULL) {
      return NULL;
    }
    else if(m_pFront == pTimer) {
      return m_pFront;
    }

    TIMER* pPrev = m_pFront;

    while(pPrev)
    {
      if(pPrev->pNext == pTimer) {
        return pPrev;
      }
      pPrev = pPrev->pNext;
    }

    return pPrev;
  }

} // namespace clstd