#include "clstd.h"
#include "clLocker.h"
#undef ETIMEDOUT
#include "Thread/clSignal.H"
#include "clMessageThread.h"
#include "clString.H"
#include "clUtility.H"
//#include <sys/time.h>

namespace clstd
{
  namespace thread
  {
    u32 GetCurrentId()
    {
  #if (defined(_WINDOWS) || defined(_WIN32)) && !defined(POSIX_THREAD)
      return GetCurrentThreadId();
  #else
      return (u32)pthread_self().p;
  #endif // #if (defined(_WINDOWS) || defined(_WIN32)) && !defined(POSIX_THREAD)
    }
  } // namespace thread
} // namespace clstd
