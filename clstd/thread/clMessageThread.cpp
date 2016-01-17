#include "clstd.h"
#include "clLocker.h"
#undef ETIMEDOUT
#include "Thread/clSignal.H"
#include "clThread.h"
#include "clMessageThread.h"
#include "clString.H"
#include "clUtility.H"
//#include <sys/time.h>

#ifdef _CPLUSPLUS_11_THREAD
# include <thread>
#endif // #ifdef _CPP11_THREAD

namespace clstd
{
  namespace thread
  {
    size_t GetCurrentId()
    {
#if _CPLUSPLUS_11_THREAD
      return (size_t)std::this_thread::get_id();
#elif (defined(_WINDOWS) || defined(_WIN32)) && !defined(POSIX_THREAD)
      return GetCurrentThreadId();
#else
      return (u32)pthread_self().p;
#endif // #if (defined(_WINDOWS) || defined(_WIN32)) && !defined(POSIX_THREAD)
    }
  } // namespace thread
} // namespace clstd
