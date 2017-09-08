#include "clstd.h"
#include "clUtility.h"
#include "Samples_Timer.h"
#include "clSchedule.h"

using namespace clstd;

namespace clstd_sample
{

  // 基础测试
  void Timer_Basic()
  {
    Schedule s;
    u32 tick = (u32)GetTime64();
    u32 begin_tick = tick;
    const u32 stride = 100; // 模拟时间步进

    s.CreateTimer((void*)0x5001, 101, 1000, NULL);
    s.CreateTimer((void*)0x5001, 102, 387, NULL);
    s.CreateTimer((void*)0x5000, 101, 990, NULL);
    s.CreateTimer((void*)0x5000, 103, 1990, NULL);

    while(1)
    {
      TIMER tm;
      u32 nearest_tick = 0;
      const u32 rel_tick = tick - begin_tick;

      // 获得已经触发的timer时，可能包含多个已经触发的timer
      // 所以这里要多次GetTimer()，直到返回值不为0
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

      // 结束条件
      if(rel_tick >= 10000) {
        break;
      }
    }
  }

  // 用两个计时器不同步进测试触发顺序
  void Timer_Delta()
  {
    Schedule s[2];
    u32 begin_tick = (u32)GetTime64();
    u32 tick[2] = { begin_tick, begin_tick };

    // 低频stride发生时将timer加入集合，高频stride到达与低频同样的时间时检查集合里是否为0
    const u32 stride[2] = { 100, 2 }; // 模拟时间步进
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
      // 低频调度器中记录timer和发生次数
      do {
        nearest_tick = s[0].GetTimer(tick[0], &tm);

        if(nearest_tick == 0) {
          const u32 u = ((u32)tm.handle) * 10000 + tm.id; // 生成唯一索引
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

      // 高频调度器里依次去掉timer
      while(tick[0] != tick[1])
      {
        do {
          nearest_tick = s[1].GetTimer(tick[1], &tm);

          if(nearest_tick == 0) {
            const u32 u = ((u32)tm.handle) * 10000 + tm.id; // 生成唯一索引
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

      // 结束条件
      if(tick[1] - begin_tick >= 10001) {
        break;
      }
    }
    ASSERT(stTimerSet.empty());
  }

  // 测试入口
  void CL_CALLBACK Sample_Timer()
  {
    Timer_Basic();
    Timer_Delta();
  }

} // namespace clstd_sample
