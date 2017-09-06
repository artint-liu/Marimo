#include "clstd.h"
#include "clSchedule.h"

namespace clstd
{
  TIMER* Schedule::Find(void* handle, u32 id) const
  {
    HandleDict::const_iterator iterHandle = m_Handles.find(handle);
    if(iterHandle == m_Handles.end()) {
      return NULL;
    }

    IdDict::const_iterator iterId = iterHandle->second.find(id);
    if(iterId == iterHandle->second.end()) {
      return NULL;
    }

    return iterId->second;
  }

  void Schedule::InsertInOrder(TIMER* t) // 按发生顺序插入到表里
  {
    ASSERT(t->pNext == NULL); // 检查已经不在顺序表里（一定程度上防止）

    CountDownOrder::iterator iter_order = m_Order.find(t->timing);
    if(iter_order == m_Order.end()) {
      m_Order.insert(clmake_pair(t->timing, t));
    }
    else {
      t->pNext = iter_order->second;
      iter_order->second = t;
    }
  }

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
    TIMER* t = Find(handle, id);
    if(t) {
      t->elapse = elapse;
      t->proc = proc;

      CountDownOrder::iterator iter_order = m_Order.find(t->timing);
      ASSERT(iter_order != m_Order.end()); // 肯定有啊！

      // 从队列中移除timer，移除后队列对应位置可能是个NULL
      TIMER** ppCurr = &iter_order->second;
      do {
        if(*ppCurr == t)
        {
          *ppCurr = t->pNext; // t->pNext 有可能是NULL
          t->pNext = NULL;
          break;
        }
        ppCurr = &((*ppCurr)->pNext);
      } while(*ppCurr);
      
      t->timing = 0;

      // 去掉删除标记
      if(TEST_FLAG(t->flags, TIMER::TimerFlag_Useless)) {
        RESET_FLAG(t->flags, TIMER::TimerFlag_Useless);
      }
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
    TIMER* t = Find(handle, id);
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

  TIMER::tick_t Schedule::GetNearestTick(TIMER::tick_t curr_tick) const
  {
    //m_CurrTick = curr_tick;
    if(m_Order.empty()) {
      return (TIMER::tick_t)-1;
    }

    // 发生时间不会返回负值
    TIMER* t = m_Order.begin()->second;
    if(t->timing < curr_tick) {
      return 0;
    }
    else {
      return t->timing - curr_tick;
    }
  }

} // namespace clstd