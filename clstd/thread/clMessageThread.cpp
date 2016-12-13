#include "clstd.h"
#include "clLocker.h"
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
#if _CPLUSPLUS_11_THREAD
    typedef std::thread::id id;
    std::thread::id GetCurrentId()
    {
      return std::this_thread::get_id();
    }
#elif (defined(_WINDOWS) || defined(_WIN32)) && !defined(POSIX_THREAD)
    typedef size_t id;
    id GetCurrentId()
    {
      return GetCurrentThreadId();
    }
#else
    typedef size_t id;
    id GetCurrentId()
    {
      return (id)pthread_self().p;
    }
#endif // #if (defined(_WINDOWS) || defined(_WIN32)) && !defined(POSIX_THREAD)
  } // namespace thread
} // namespace clstd
