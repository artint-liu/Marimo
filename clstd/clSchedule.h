#ifndef _CLSTD_TIMER_SCHEDULE_H_
#define _CLSTD_TIMER_SCHEDULE_H_

namespace clstd
{
  typedef void (CL_CALLBACK *TimerProc)(void* handle, u32 id);

  struct TIMER
  {
    enum TimerFlag
    {
      TimerFlag_Useless = 0x0001,   // 销毁
    };
    typedef u32 tick_t;

    TIMER*    pNext;
    TimerProc proc;
    void*     handle;
    u32       id;
    u32       flags;
    tick_t    elapse;
    tick_t    timing;

    TIMER()
      : pNext(NULL)
      , proc(NULL)
      , handle(NULL)
      , id(0)
      , flags(0)
      , elapse(0)
      , timing(0)
    {}

    TIMER(void* _handle, u32 _id, tick_t _elapse, TimerProc _proc)
      : pNext(NULL)
      , proc(_proc)
      , handle(_handle)
      , id(_id)
      , flags(0)
      , elapse(_elapse)
      , timing(0)
    {}
  };

  class Schedule
  {
  protected:
    typedef clmap<u32, TIMER*>        IdDict;
    typedef clhash_map<void*, IdDict> HandleDict;
    //typedef LinkedList<TIMER>         TimerChain2;

    class TimerChain
    {
      TIMER* m_pFront;
      TIMER* m_pBack;
      size_t m_count;

    public:
      TimerChain();
      TimerChain(TIMER* pTimer);

      void    push_back(TIMER* pTimer);
      TIMER*  pop_front();
      TIMER*  front() const;
      size_t  size() const;
      b32     empty() const;
      void    clear();
      b32     erase(const TIMER* pTimer);
      TIMER*  find(const TIMER* pTimer); // 返回值是pTimer的前一个timer，如果返回pTimer表示pTimer是第一个timer
    };
    typedef clmap<u32, TimerChain>    CountDownOrder;

    HandleDict m_Handles;
    CountDownOrder m_Order;
    //TIMER::tick_t m_CurrTick;

  protected:
    TIMER* Find(void* handle, u32 id, b32 bRemove);
    void InsertInOrder(TIMER* t);
    //TIMER* RemoveFrontFromTimerChain(CountDownOrder::iterator& it);
  public:
    Schedule();
    virtual ~Schedule();

    void CreateTimer(void* handle, u32 id, TIMER::tick_t elapse, TimerProc proc);
    void DestroyTimer(void* handle, u32 id);
    void RemoveByHandle(void* handle);
    //TIMER::tick_t GetNearestTick(TIMER::tick_t curr_tick) const;
    TIMER::tick_t GetTimer(TIMER::tick_t curr_tick, TIMER* pOutTimer); // 只会改写handle, proc, id, elapse这四个成员变量
  };

  //////////////////////////////////////////////////////////////////////////
  // TIMER::tick_t GetTimer(TIMER::tick_t curr_tick, TIMER* pOutTimer);
  // 根据绝对时间curr_tick获得发生时间或者触发timer的信息
  // 备注：
  //    如果timer列表为空，返回(TIMER::tick_t)-1
  //    如果没有timer到达触发时间，pOutTimer保持不变，函数返回最近一个timer的距离curr_tick的毫秒数
  //    如果有timer到达触发时间，pOutTimer返回这个timer的信息，函数返回0
  //    pOutTimer中只有handle, proc, id, elapse这四个成员变量有效，其它成员变量不会改变

} // namespace clstd

#endif // _CLSTD_TIMER_SCHEDULE_H_