#include "clstd.h"
#include "clLocker.h"
#include "Thread/clSignal.h"
#include "clThread.h"
#include "clMessageThread.h"
#include "clString.h"
#include "clUtility.h"
//#include <sys/time.h>

#if defined(_CPLUSPLUS_11_THREAD) || (__cplusplus >= 201103L) || (_MSC_VER >= 1900)
# include <thread>

namespace clstd
{
  namespace this_thread
  {
    namespace cxx11
    {

      id GetId()
      {
        return std::this_thread::get_id();
      }

    } // namespace cxx11
  } // namespace this_thread
} // namespace clstd
#endif // #if defined(_CPLUSPLUS_11_THREAD) || (__cplusplus >= 201103L) || (_MSC_VER >= 1900)

namespace clstd
{
  namespace this_thread
  {

#if _CPLUSPLUS_11_THREAD
    id GetId()
    {
      return this_thread::cxx11::GetId();
    }
#elif (defined(_WINDOWS) || defined(_WIN32)) && !defined(POSIX_THREAD)
    id GetId()
    {
      return GetCurrentThreadId();
    }
#else
    id GetId()
    {
      return (id)pthread_self().p;
    }
#endif // #if (defined(_WINDOWS) || defined(_WIN32)) && !defined(POSIX_THREAD)
  } // namespace thread
} // namespace clstd
