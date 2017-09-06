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
    void*     handle;
    TimerProc proc;
    u32       flags;
    u32       id;
    tick_t    elapse;
    tick_t    timing;

    TIMER(void* _handle, u32 _id, tick_t _elapse, TimerProc _proc)
      : pNext(NULL)
      , handle(_handle)
      , proc(_proc)
      , flags(0)
      , id(_id)
      , elapse(_elapse)
      , timing(0)
    {}

    //TIMER()
    //  : handle(NULL)
    //  , id(0)
    //  , elapse(0)
    //  , proc(NULL)
    //{}
  };

  class Schedule
  {
  protected:
    typedef clmap<u32, TIMER*>        IdDict;
    typedef clhash_map<void*, IdDict> HandleDict;
    typedef clmap<u32, TIMER*>        CountDownOrder;
  
    HandleDict m_Handles;
    CountDownOrder m_Order;
    //TIMER::tick_t m_CurrTick;

  protected:
    TIMER* Find(void* handle, u32 id) const;
    void InsertInOrder(TIMER* t);
  public:
    Schedule();
    virtual ~Schedule();

    void CreateTimer(void* handle, u32 id, TIMER::tick_t elapse, TimerProc proc);
    void DestroyTimer(void* handle, u32 id);
    void RemoveByHandle(void* handle);
    TIMER::tick_t GetNearestTick(TIMER::tick_t curr_tick) const;
  };
} // namespace clstd

#endif // _CLSTD_TIMER_SCHEDULE_H_