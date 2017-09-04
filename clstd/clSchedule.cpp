#include "clstd.h"
#include "clSchedule.h"

namespace clstd
{
  TIMER* Schedule::Find(void* handle, u32 id)
  {
    HandleDict::iterator iterHandle = m_Handles.find(handle);
    if(iterHandle == m_Handles.end()) {
      return NULL;
    }

    IdDict::iterator iterId = iterHandle->second.find(id);
    if(iterId == iterHandle->second.end()) {
      return NULL;
    }

    return iterId->second;
  }

  void Schedule::CreateTimer(void* handle, u32 id, u32 elapse, TimerProc proc)
  {
    TIMER* t = Find(handle, id);
    if(t) {
      t->elapse = elapse;
      t->proc = proc;

      CountDownOrder::iterator iter_order = m_Order.find(t->count_down);
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
      
      t->count_down = 0;
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

    ASSERT(t->count_down == 0);
    ASSERT(t->pNext == NULL);

    CountDownOrder::iterator iter_order = m_Order.find(0);
    if(iter_order == m_Order.end()) {
      m_Order.insert(clmake_pair(0, t));
    }
    else {
      t->pNext = iter_order->second;
      iter_order->second = t;
    }
  }

  void Schedule::DestroyTimer(void* handle, u32 id)
  {
    TIMER* t = Find(handle, id);
    if(t) {
      SET_FLAG(t->flags, TIMER::TimerFlag_Useless);
    }
  }

} // namespace clstd