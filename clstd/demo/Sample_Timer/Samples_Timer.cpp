#include "clstd.h"
#include "clUtility.h"
#include "Samples_Timer.h"
#include "clSchedule.h"

using namespace clstd;

namespace clstd_sample
{

  // ��������
  void Timer_Basic()
  {
    Schedule s;
    u32 tick = (u32)GetTime64();
    u32 begin_tick = tick;
    const u32 stride = 100; // ģ��ʱ�䲽��

    s.CreateTimer((void*)0x5001, 101, 1000, NULL);
    s.CreateTimer((void*)0x5001, 102, 387, NULL);
    s.CreateTimer((void*)0x5000, 101, 990, NULL);
    s.CreateTimer((void*)0x5000, 103, 1990, NULL);

    while(1)
    {
      TIMER tm;
      u32 nearest_tick = 0;
      const u32 rel_tick = tick - begin_tick;

      // ����Ѿ�������timerʱ�����ܰ�������Ѿ�������timer
      // ��������Ҫ���GetTimer()��ֱ������ֵ��Ϊ0
      do {
        nearest_tick = s.GetTimer(tick, &tm);

        if(nearest_tick) {
          CLOG("nearest_tick(%u):%u", rel_tick, nearest_tick);
        }
        else {
          CLOG("nearest_tick(%u):%u, handle:%p, id:%d", rel_tick, nearest_tick, tm.handle, tm.id);
          ASSERT((rel_tick % tm.elapse) < stride);
        }
      } while(nearest_tick == 0);

      tick += stride;

      // ��������
      if(rel_tick >= 10000) {
        break;
      }
    }
  }

  // ��������2 - ��ɾ��
  void Timer_Basic2()
  {
    Schedule s;
    u32 tick = (u32)GetTime64();
    u32 begin_tick = tick;
    const u32 stride = 100; // ģ��ʱ�䲽��
    int index = 0;

    struct TIMER_SAMP
    {
      void* handle;
      u32   id;
      u32   elapse;
    };

    TIMER_SAMP aTimers[] =
    {
      {(void*)0x2001, 102, 239},
      {(void*)0x2002, 101, 1284},
      {(void*)0x2001, 103, 3892},
      {(void*)0x2004, 102, 1000},
      {(void*)0x2005, 101, 1000},
      {(void*)0x2003, 103, 339},
      {(void*)0x2004, 102, 500},
      {(void*)0x2002, 101, 480},
      {(void*)0x2001, 103, 838},
    };

    s.CreateTimer((void*)0x5001, 101, 1000, NULL);
    s.CreateTimer((void*)0x5001, 102, 387, NULL);
    s.CreateTimer((void*)0x5000, 101, 990, NULL);
    s.CreateTimer((void*)0x5000, 103, 1990, NULL);

    while(1)
    {
      TIMER tm;
      u32 nearest_tick = 0;
      const u32 rel_tick = tick - begin_tick;

      // ����Ѿ�������timerʱ�����ܰ�������Ѿ�������timer
      // ��������Ҫ���GetTimer()��ֱ������ֵ��Ϊ0
      do {
        nearest_tick = s.GetTimer(tick, &tm);

        if(nearest_tick) {
          CLOG("nearest_tick(%u):%u", rel_tick, nearest_tick);
        }
        else {
          CLOG("nearest_tick(%u):%u, handle:%p, id:%d", rel_tick, nearest_tick, tm.handle, tm.id);
          if(index <= 29) {
            ASSERT((rel_tick % tm.elapse) < stride);
          }
        }
      } while(nearest_tick == 0);

      if(index == 29)
      {
        s.DestroyTimer((void*)0x5001, 101);
        s.RemoveByHandle((void*)0x502);
        //s.DestroyTimer((void*)0x5001, 102);
      }
      else if(index >= 50 && index < 50 + countof(aTimers))
      {
        int n = index - 50;
        CLOG("CreateTimer, handle:%p, id:%u, elapse:%u", aTimers[n].handle, aTimers[n].id, aTimers[n].elapse);
        s.CreateTimer(aTimers[n].handle, aTimers[n].id, aTimers[n].elapse, NULL);
      }
      else if(index >= 55 && index < 55 + countof(aTimers))
      {
        int n = index - 55;
        CLOG("DestroyTimer, handle:%p, id:%u", aTimers[n].handle, aTimers[n].id);
        //s.DestroyTimer(aTimers[n].handle, aTimers[n].id);
      }

      tick += stride;
      index++;

      // ��������
      if(rel_tick >= 10000) {
        break;
      }
    }
  }

  // ��������ʱ����ͬ�������Դ���˳��
  void Timer_Delta()
  {
    Schedule s[2];
    u32 begin_tick = (u32)GetTime64();
    u32 tick[2] = { begin_tick, begin_tick };

    // ��Ƶstride����ʱ��timer���뼯�ϣ���Ƶstride�������Ƶͬ����ʱ��ʱ��鼯�����Ƿ�Ϊ0
    const u32 stride[2] = { 100, 2 }; // ģ��ʱ�䲽��
    ASSERT((stride[0] % stride[1]) == 0);

    for(int i = 0; i < 2; i++)
    {
      s[i].CreateTimer((void*)0x5001, 101, 1000, NULL);
      s[i].CreateTimer((void*)0x5001, 102, 387, NULL);
      s[i].CreateTimer((void*)0x5000, 101, 990, NULL);
      s[i].CreateTimer((void*)0x5000, 103, 1990, NULL);
    }

    clmap<u32, int> stTimerSet;

    while(1)
    {
      TIMER tm;
      u32 nearest_tick = 0;

      CLOG("Schedule[0]:%u", tick[0] - begin_tick);
      // ��Ƶ�������м�¼timer�ͷ�������
      do {
        nearest_tick = s[0].GetTimer(tick[0], &tm);

        if(nearest_tick == 0) {
          const u32 u = ((u32)tm.handle) * 10000 + tm.id; // ����Ψһ����
          CLOG("timer:%u", u);

          auto it = stTimerSet.find(u);
          if(it == stTimerSet.end()) {
            stTimerSet.insert(clmake_pair(u, 1));
          }
          else {
            it->second++;
          }
        }
      } while(nearest_tick == 0);

      CLOG("count:%d", stTimerSet.size());

      // ��Ƶ������������ȥ��timer
      while(tick[0] != tick[1])
      {
        do {
          nearest_tick = s[1].GetTimer(tick[1], &tm);

          if(nearest_tick == 0) {
            const u32 u = ((u32)tm.handle) * 10000 + tm.id; // ����Ψһ����
            CLOG("timer:%u(%u)", u, tick[1] - begin_tick);
            auto it = stTimerSet.find(u);

            ASSERT(it != stTimerSet.end());
            if(it != stTimerSet.end() && --it->second == 0)
            {
              stTimerSet.erase(it);
            }
          }
        } while(nearest_tick == 0);

        tick[1] += stride[1];
      }

      tick[0] += stride[0];

      //ASSERT(stTimerSet.empty());

      // ��������
      if(tick[1] - begin_tick > 10000) {
        break;
      }
    }
    ASSERT(stTimerSet.empty());
  }

  // �������
  void CL_CALLBACK Sample_Timer()
  {
    Timer_Basic();
    Timer_Basic2();
    Timer_Delta();
  }

} // namespace clstd_sample
