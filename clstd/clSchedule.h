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
    TIMER* pNext;
    void* handle;
    TimerProc proc;
    u32   flags;
    u32   id;
    u32   elapse;
    u32   count_down;

    TIMER(void* _handle, u32 _id, u32 _elapse, TimerProc _proc)
      : pNext(NULL)
      , handle(_handle)
      , proc(_proc)
      , flags(0)
      , id(_id)
      , elapse(_elapse)
      , count_down(0)
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
  protected:
    TIMER* Find(void* handle, u32 id);

  public:
    void CreateTimer(void* handle, u32 id, u32 elapse, TimerProc proc);
    void DestroyTimer(void* handle, u32 id);
  };
} // namespace clstd

#endif // _CLSTD_TIMER_SCHEDULE_H_