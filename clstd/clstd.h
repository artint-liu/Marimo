#ifndef _CL_STD_CODE_
#define _CL_STD_CODE_

//#define POSIX_THREAD

// 预定义宏
// https://technet.microsoft.com/zh-cn/subscriptions/b0084kay(v=vs.80).aspx#_predir_table_1..3
// https://msdn.microsoft.com/en-us/library/b0084kay.aspx
// https://msdn.microsoft.com/zh-cn/library/79yewefw.aspx
// https://blogs.msdn.microsoft.com/c/2015/12/08/vs-2015-update-1-clang/
// https://gcc.gnu.org/onlinedocs/cpp/Predefined-Macros.html
// clang:clang -dM -E -x c /dev/null
// https://sourceforge.net/p/predef/wiki/Architectures/

// 避免使用"PLATFORM"做为库定义宏，这个单词可能指构架平台，操作系统平台或者支付平台，容易引起歧异

#if __cplusplus < 201103L && !defined(__UNREAL__)
# error 需要C++11或以上级别的编译器支持
#endif

// 处理器平台
#if defined(_M_IX86) || defined(__i386__)
# define _CL_ARCH_X86
#elif defined(_M_X64) || defined(__amd64__)
# define _CL_ARCH_X64
#elif defined(__arm__)
# define _CL_ARCH_ARM
#elif defined(__aarch64__)
# define _CL_ARCH_ARM64
#else
# error 这是一个意料之外的CPU构架
#endif

#if !(defined(_CL_SYSTEM_WINDOWS) || defined(_CL_SYSTEM_UWP) || defined(_CL_SYSTEM_IOS) || defined(_CL_SYSTEM_IOS_SIM) || defined(_CL_SYSTEM_ANDROID))
// 操作系统平台
# if defined(_WIN32) || defined(WIN32)
#   define _CL_SYSTEM_WINDOWS
# endif
#endif

#if defined(_CL_SYSTEM_WINDOWS)   // 桌面windows
#elif defined(_CL_SYSTEM_UWP)     // Universal windows platform
#elif defined(_CL_SYSTEM_IOS)
#elif defined(_CL_SYSTEM_IOS_SIM) // ios 模拟器
#elif defined(_CL_SYSTEM_ANDROID)
#endif


//#if ! defined(_WINDOWS) && ! defined(_IOS) && ! defined(_ANDROID)
//# define _WINDOWS
//#endif

#if defined(_CL_SYSTEM_WINDOWS) || defined(_CL_SYSTEM_UWP)
# if defined(__UNREAL__)
#   include "Windows/AllowWindowsPlatformTypes.h"
#   include <windows.h>
#   include "Windows/HideWindowsPlatformTypes.h"
# else
#   include <windows.h>
# endif
//# if !defined(_WIN32) && !defined(_WIN64)
//# define _WIN32
//# endif // #if !defined(_CL_ARCH_X86) && !defined(_CL_ARCH_X64)

#elif defined(_CL_SYSTEM_IOS)
# include <assert.h>
#elif defined(_CL_SYSTEM_ANDROID)
# include <wchar.h>
# define _CPLUSPLUS_11_THREAD
//# include <corecrt_io.h> // _findfile
#elif defined(_CL_SYSTEM_LINUX)
# include <stdint.h>
# include <stddef.h>
#elif defined(_CL_SYSTEM_MACOS)
# include <stdlib.h>
#else
# error 未知平台或者新增加的平台
#endif

#include "cltypes.h"
#include <memory.h>

#define CL_PI         ((float)3.141592654f)
#define CL_HALF_PI    (CL_PI * 0.5f)
#define CL_2PI        (CL_PI * 2.0f)
#define CL_INVERSEPI  ((float)  0.318309886f)
#define CL_INVERSE2PI ((float)1.0f / CL_2PI)
#define CL_RAD2DEG(r) (r * 180.0f / CL_PI)
#define CL_DEG2RAD(r) (r * CL_PI / 180.0f)

#define CLMAKEFOURCC(ch0, ch1, ch2, ch3)                      \
  ((CLDWORD)(CLBYTE)(ch0) | ((CLDWORD)(CLBYTE)(ch1) << 8) |   \
  ((CLDWORD)(CLBYTE)(ch2) << 16) | ((CLDWORD)(CLBYTE)(ch3) << 24 ))
#define CLMAKEWORD(a, b)      ((CLWORD)(((CLBYTE)(((CLDWORD_PTR)(a)) & 0xff)) | ((CLWORD)((CLBYTE)(((CLDWORD_PTR)(b)) & 0xff))) << 8))
#define CLMAKELONG(a, b)      ((CLLONG)(((CLWORD)(((CLDWORD_PTR)(a)) & 0xffff)) | ((CLDWORD)((CLWORD)(((CLDWORD_PTR)(b)) & 0xffff))) << 16))
#define _CL_NOT_(x)           (!(x)) // 这样醒目！

template<class _Ty>
inline void InlSetZeroT(_Ty& t) {
  memset(&t, 0, sizeof(t));
}

#define WIDEN(x) _CLTEXT(x)
#define __WFILE__ WIDEN(__FILE__)

#ifndef SAFE_DELETE
# define SAFE_DELETE(x)        if((x) != NULL) {delete (x); (x) = 0;}
#endif // SAFE_DELETE

#ifndef SAFE_DELETE_ARRAY
# define SAFE_DELETE_ARRAY(x)  if((x) != NULL) {delete[](x); (x) = 0;}
#endif // SAFE_DELETE_ARRAY

#ifndef SAFE_RELEASE
# define SAFE_RELEASE(pobj)    if((pobj) != NULL)  {(pobj)->Release(); (pobj) = NULL; }
#endif // SAFE_RELEASE

#ifndef SAFE_ADDREF
# define SAFE_ADDREF(pobj)    if((pobj) != NULL)  { (pobj)->AddRef(); }
#endif // SAFE_ADDREF

// Visual Studio 附加参数：/Zc:__cplusplus，则 __cplusplus == _MSVC_LANG
#if __cplusplus >= 201402L || defined(__UNREAL__)
# define CLENUM_CLASS(_TYPE, _ENUM)  enum class _ENUM : _TYPE
# define CLTRIVIAL_DEFAULT = default
#else
# define CLENUM_CLASS(_TYPE, _ENUM)  enum _ENUM
# define CLTRIVIAL_DEFAULT {}
#endif


#ifndef countof
template<typename _Ty, size_t _count>
char (&_cl_CountOfHelper(_Ty(&_array)[_count]))[_count];
# define countof(_arr) (sizeof(_cl_CountOfHelper(_arr)))
#endif

// 弃用标记
#if defined(__GNUC__) && ((__GNUC__ >= 4) || ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 1)))
# define CLDEPRECATED_ATTRIBUTE __attribute__((deprecated))
# define CLPRINTFLIKE(fmtarg, firstvararg) __attribute__((__format__(__printf__, fmtarg, firstvararg)))
#elif _MSC_VER >= 1400 //vs 2005 or higher
# define CLDEPRECATED_ATTRIBUTE __declspec(deprecated) 
# define CLPRINTFLIKE(fmtarg, firstvararg)
#else
# define CLDEPRECATED_ATTRIBUTE
# define CLPRINTFLIKE(fmtarg, firstvararg)
#endif 


#define SETBIT(_DW, _IDX)       ((_DW) |= (1 << (_IDX)))
#define RESETBIT(_DW, _IDX)     ((_DW) &= (~(1 << (_IDX))))
#define TESTBIT(_DW, _IDX)      ((_DW) & (1 << (_IDX)))

#define TEST_FLAG(_F, _M)       (((_F) & (_M)) != 0)
#define TEST_FLAG_NOT(_F, _M)   (((_F) & (_M)) == 0)
#define TEST_FLAGS_ANY(_F, _M)  (((_F) & (_M)) != 0)
#define TEST_FLAGS_ALL(_F, _M)  (((_F) & (_M)) == (_M))
#define SET_FLAG(_F, _M)        ((_F) |= (_M))
#define RESET_FLAG(_F, _M)      ((_F) = ((_F) & (~(_M))))
#define SWITCH_FLAG(_F, _M)     ((_F) = ((_F) ^ (_M)))
#define UPDATE_FLAG(_F, _M, _B) ((_F) = (_B) ? ((_F) | (_M)) : ((_F) & (~(_M))))

#define ALIGN_2(x)              (((x) + 1) & (~1))
#define ALIGN_4(x)              (((x) + 3) & (~3))
#define ALIGN_8(x)              (((x) + 7) & (~7))
#define ALIGN_16(x)             (((x) + 15) & (~15))
#define ALIGN_32(x)             (((x) + 31) & (~31))

#define DECL_ALIGN_1            __declspec(align(1))
#define DECL_ALIGN_2            __declspec(align(2))
#define DECL_ALIGN_4            __declspec(align(4))
#define DECL_ALIGN_8            __declspec(align(8))
#define DECL_ALIGN_16           __declspec(align(16))
#define DECL_ALIGN_32           __declspec(align(32))

#define MEMBER_OFFSET(_CLS, _MEMBER)  ((size_t)(&(((_CLS*)0)->_MEMBER)))
#define CLARRAYSIZE(_ARRAY)           (sizeof(_ARRAY)/sizeof(_ARRAY[0]))

//////////////////////////////////////////////////////////////////////////
//
// logs
//

// 数据格式化后原始消息
extern "C" void _cl_traceA(const char *fmt, ...) CLPRINTFLIKE(1, 2);
extern "C" void _cl_traceW(const wch *fmt, ...);

// 输出日志格式消息，一般是"[ERROR]错误信息\r\n"
extern "C" void _cl_log_infoA   (const char *fmt, ...) CLPRINTFLIKE(1, 2);
extern "C" void _cl_log_errorA  (const char *fmt, ...) CLPRINTFLIKE(1, 2);
extern "C" void _cl_log_warningA(const char *fmt, ...) CLPRINTFLIKE(1, 2);

extern "C" void _cl_log_infoW   (const wch *fmt, ...);
extern "C" void _cl_log_errorW  (const wch *fmt, ...);
extern "C" void _cl_log_warningW(const wch *fmt, ...);

namespace clstd
{
  void _cl_log_info   (const char *fmt, ...) CLPRINTFLIKE(1, 2);
  void _cl_log_error  (const char *fmt, ...) CLPRINTFLIKE(1, 2);
  void _cl_log_warning(const char *fmt, ...) CLPRINTFLIKE(1, 2);

  void _cl_log_info   (const wch *fmt, ...);
  void _cl_log_error  (const wch *fmt, ...);
  void _cl_log_warning(const wch *fmt, ...);

  void* MemCopy(void* pDest, const void* pSrc, size_t count); // 兼容性内存拷贝，支持覆盖内存拷贝
  void Sleep(u32 dwMilliseconds);
}

#define CLOG_WARNING  clstd::_cl_log_warning
#define CLOG_ERROR    clstd::_cl_log_error
#define CLOG          clstd::_cl_log_info
#define CLOG_WARNINGW _cl_log_warningW
#define CLOG_ERRORW   _cl_log_errorW
#define CLOGW         _cl_log_infoW
#define CLOG_WARNINGA _cl_log_warningA
#define CLOG_ERRORA   _cl_log_errorA
#define CLOGA         _cl_log_infoA



#ifdef _DEBUG
# if defined(_CL_SYSTEM_WINDOWS) || defined(_CL_SYSTEM_UWP) || defined(_CONSOLE)
#   ifdef __clang__
#     include <assert.h>
#     define TRACEW        wprintf
#     define TRACEA        printf
#     define TRACE         TRACEA
#     define CLBREAK       abort()
#     define CLUNIQUEBREAK abort()      // 只中断一次的断点
#     define CLNOP         ;
#     define VERIFY(v)     (v)
#     define ASSERT(x)     assert(x)
#     define STATIC_ASSERT(x)    static_assert(x, #x);
//#     define V(x)              if(FAILED(x)) { CLBREAK; }
#     define V_RETURN(x)       if(FAILED(x)) { return GX_FAIL; }
#   elif defined(_MSC_VER)


#     define TRACEW        _cl_traceW
#     define TRACEA        _cl_traceA
#     define TRACE         TRACEA

#     ifdef _CL_ARCH_X86
extern "C" void _cl_WinVerifyFailure(const char *pszSrc, const char *pszSrcFile, int nLine, unsigned long dwErrorNum);
extern "C" void _cl_assertW(const wch *pszSrc, const wch *pszSrcFile, int nLine);
extern "C" void _cl_Break();


#       if _MSC_FULL_VER >= 191627026
#         define CLBREAK       {_cl_Break();}
#         define CLABORT       {abort();}
#         define CLUNIQUEBREAK {_cl_Break();}      // 只中断一次的断点
#         define CLNOP         {do{}while(0);}
#       else
#         define CLBREAK       {__asm int 3}
#         define CLABORT       {__asm int 3}
// TODO: 稍后实现CLUNIQUEBREAK
//  1.线程自己持有
//  2.第一次中断，以后跳过并输出log
#         define CLUNIQUEBREAK {__asm int 3}      // 只中断一次的断点
#         define CLNOP         {__asm nop}
#       endif

#       define VERIFY(v)      if(!(v))  _cl_WinVerifyFailure(#v, __FILE__,__LINE__, GetLastError())
#       define ASSERT(x)      if(!(x)) {_cl_assertW(_CLTEXT(#x), __WFILE__, __LINE__); CLBREAK; } // TODO: 不要在这里面加入程序功能逻辑代码，Release版下会被忽略
#       define STATIC_ASSERT(x)    static_assert(x, #x);
//#       define V(x)              if(FAILED(x)) { CLBREAK; }
#       define V_RETURN(x)       if(FAILED(x)) { return GX_FAIL; }
#     elif defined(_CL_ARCH_X64)
//extern "C" void _cl_traceA(const char *fmt, ...);
//extern "C" void _cl_traceW(const wch *fmt, ...);
extern "C" void _cl_WinVerifyFailure(const char *pszSrc, const char *pszSrcFile, int nLine, unsigned long dwErrorNum);
extern "C" void _cl_assertW(const wch *pszSrc, const wch *pszSrcFile, int nLine);
extern "C" void _cl_Break();
extern "C" void _cl_NoOperation();
//#       define TRACEW        _cl_traceW
//#       define TRACEA        _cl_traceA
//#       define TRACE         TRACEA
#       define CLBREAK       { _cl_Break(); }
#       define CLABORT       { _cl_Break(); }
#       define CLUNIQUEBREAK CLBREAK
#       define CLNOP         { _cl_NoOperation(); }
#       define VERIFY(v)      if(!(v))  _cl_WinVerifyFailure(#v, __FILE__,__LINE__, GetLastError())
#       define ASSERT(x)      if(!(x)) {_cl_assertW(_CLTEXT(#x), __WFILE__, __LINE__); _cl_Break();} // TODO: 不要在这里面加入程序功能逻辑代码，Release版下会被忽略
#       define STATIC_ASSERT(x)    static_assert(x, #x);
//#       define V(x)              if(FAILED(x)) { _cl_Break(); }
#       define V_RETURN(x)       if(FAILED(x)) {return GX_FAIL;}
#     endif // #ifdef _CL_ARCH_X86
#   endif  // #   ifdef __clang__

# elif defined(_CL_SYSTEM_IOS) || defined(_CL_SYSTEM_ANDROID) || defined(_CL_SYSTEM_LINUX)
#	include <assert.h>
void _cl_traceA(const char *fmt, ...);
void _cl_traceW(const wch *fmt, ...);

#   define TRACEW           _cl_traceW
#   define TRACEA           _cl_traceA
#   define TRACE				      TRACEA
#   define CLBREAK          ASSERT(0)
#   define CLABORT          abort()
#   define CLNOP
#   define VERIFY(v)        (v)
#   define ASSERT(x)        assert(x)
#   define STATIC_ASSERT(x) static_assert(x, #x)
//#   define V(x)             (x)
#   define V_RETURN(x)      (x)
# else
#	error 新平台
# endif // #if defined(_WINDOWS) || defined(_CONSOLE)
#else  // _DEBUG
#  if defined(_CL_SYSTEM_WINDOWS)
extern "C" void _cl_traceA(const char *fmt, ...);
extern "C" void _cl_traceW(const wch *fmt, ...);
extern "C" void _cl_WinVerifyFailure(const char *pszSrc, const char *pszSrcFile, int nLine, unsigned long dwErrorNum);
extern "C" void _cl_assertW(const wch *pszSrc, const wch *pszSrcFile, int nLine);
#  elif defined(_CL_SYSTEM_IOS)
void _cl_traceA(const char *fmt, ...);
void _cl_traceW(const wch *fmt, ...);
#  elif defined(_CL_SYSTEM_ANDROID)
extern "C" void _cl_traceA(const char *fmt, ...);
extern "C" void _cl_traceW(const wch *fmt, ...);
#  endif // #if defined(_WINDOWS) || defined(_WIN32)

#  define TRACEW        _cl_traceW
#  define TRACEA        _cl_traceA
#  define TRACE         TRACEA
#  define CLBREAK       {;}
#  define CLUNIQUEBREAK CLBREAK
#  define CLNOP
#  define VERIFY(v)  (v)
#  define ASSERT(x)
#  define STATIC_ASSERT(x)
//#  define V(x)      (x)
#  define V_RETURN(x)  (x)
#endif  // _DEBUG

#define NOT_IMPLEMENT_FUNC_MAKER  TRACE("%s(%d): Not implement:%s()\n", __FILE__, __LINE__, __FUNCTION__)

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef INVALID_HANDLE_VALUE
#define INVALID_HANDLE_VALUE ((HANDLE)(u32_ptr)-1)
#endif // INVALID_HANDLE_VALUE

#ifndef CALLBACK
# if defined(_CL_SYSTEM_WINDOWS)
#   define CALLBACK __stdcall
# else
#   define CALLBACK
# endif
#endif

#if (defined(_CL_SYSTEM_WINDOWS) || defined(_CL_SYSTEM_UWP)) && !defined(POSIX_THREAD)
#define CL_CALLBACK  __stdcall
#else
# include <pthread.h>
# define CL_CALLBACK
#endif


#ifndef MAX_PATH
# define MAX_PATH          260
#endif

#if defined(_MSC_VER)
# define CLREGISTER register
#else
# define CLREGISTER
#endif

#ifndef _T
#ifdef _UNICODE
#define _T(x)      L ## x
#else
#define _T(x)      x
#endif
#endif

#define clrand    rand
#define clrand_s  rand_s

#define clmemcpy  memcpy
#define clmemmove memmove

//
// Byte Order Mark
//
#define BOM_UNICODE             0xFEFF
#define BOM_UNICODE_BIGENDIAN   0xFFFE
#define BOM_UTF8                0xBFBBEF

//extern "C" b32 strcmpnA(const ch* lpString1, const ch* lpString2, int nCount);
//extern "C" b32 strcmpnW(const wch* lpString1, const wch* lpString2, int nCount);
//extern "C" int cl_atoi(const char* szStr);
//extern "C" double cl_atof(const char* szStr);

namespace clstd
{
  struct CLSTDOBJCTSTAT     // clstd object statistics
  {
    CLLONG nNumLockers;     // 锁的数量
    CLLONG nNumAllocators;  // 分配器的数量
    CLLONG nNumThreads;     // 线程数量, 这个是基于clstd方法创建的线程
  };

  // 字节序交换
  u16 bitswap(u16 w);
  u32 bitswap(u32 dw);
  u64 bitswap(u64 qw);

  CLLONG InterlockedIncrement       (CLLONG volatile *Addend);
  CLLONG InterlockedDecrement       (CLLONG volatile *Addend);
  CLLONG InterlockedExchange        (CLLONG volatile *Target, CLLONG Value);
  CLLONG InterlockedExchangeAdd     (CLLONG volatile *Addend, CLLONG Value);
  CLLONG InterlockedCompareExchange (CLLONG volatile *Destination, CLLONG Exchange, CLLONG Comperand);

  template<class _Ty>
  static void __unspecified_bool_type(_Ty***)
  {
  }

  class NonCopyable
  {
  protected:
    NonCopyable() {}
  private:
    NonCopyable(const NonCopyable& ) {}
    NonCopyable& operator=(const NonCopyable& ) { return *this; }
  };
} // namespace clstd

namespace clstd_cli
{
  int kbhit();
  char getch();
}


// 常用的头文件
#include "thread/clLocker.h"
#include "clAllocator.h"
#include "clFile.h"
#include "clBuffer.h"

#include "clMathVector.h"
#include "floatx.h"
#include "clMatrix.h"
#include "clQuaternion.h"
#include "3D/Geometry.h"
#include "3D/GeoOp.h"
#include "clGeometry2D.h"

#else
#pragma message(__FILE__": warning : Duplicate included this file.")
#endif // _CL_STD_CODE_
