#include "clstd.h"
#include "clLocker.h"
#include "thread/clSignal.h"
#include "clThread.h"
#include "clMessageThread.h"
#include "clString.h"
#include "clUtility.h"
//#include <sys/time.h>

#if defined(POSIX_THREAD) && defined(_CL_SYSTEM_LINUX)
# include <sys/syscall.h>
# include <sys/types.h>
# include <unistd.h>
#endif

namespace clstd
{

#if defined(_CPLUSPLUS_11_THREAD) || (__cplusplus >= 201103L) || (_MSC_VER >= 1900)
  namespace cxx11
  {
    namespace this_thread
    {
      id GetId()
      {
        return std::this_thread::get_id();
      }
    }
  }
#endif // #if defined(_CPLUSPLUS_11_THREAD) || (__cplusplus >= 201103L) || (_MSC_VER >= 1900)

#if defined(_CL_SYSTEM_WINDOWS)
  namespace _win32
  {
    namespace this_thread
    {
      id GetId()
      {
        return GetCurrentThreadId();
      }
    }
  }
#endif // #if defined(_CL_SYSTEM_WINDOWS)

#if defined(POSIX_THREAD)
  namespace _posix
  {
    namespace this_thread
    {
      id GetId()
      {
#if defined(_CL_SYSTEM_WINDOWS)
        return GetCurrentThreadId();
#else
        return ::syscall(SYS_gettid);
#endif
      }
    }
  }
#endif // #if defined(POSIX_THREAD)

} // namespace clstd
