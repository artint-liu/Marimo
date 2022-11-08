#include "clstd.h"
#include "clString.h"

#if defined(_CL_SYSTEM_WINDOWS) || defined(_CL_SYSTEM_UWP)
# include <conio.h>
# include <Windows.h>
# include <Shlwapi.h>
//# include <vld.h>
# pragma comment(lib, "shlwapi.lib")
//# pragma warning(disable : 4996)
#elif defined(_CL_SYSTEM_LINUX)
# include <stdio.h>
# include <termios.h>
# include <unistd.h>
# include <fcntl.h>
#endif

#if defined(_CL_SYSTEM_ANDROID)
# include <unistd.h>
#endif

#include "clStringAttach.h"
#define MAX_TRACE_BUFFER 2048

namespace clstd
{
//enum Output
}

#if defined(_CL_SYSTEM_WINDOWS) || defined(_CL_SYSTEM_UWP)
#define SET_TEXT_COLOR(_CR)     CONSOLE_SCREEN_BUFFER_INFO bi;  \
                                HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE); \
                                GetConsoleScreenBufferInfo(hStdout, &bi); \
                                SetConsoleTextAttribute(hStdout, _CR);

#define RESTORE_TEXT_COLOR()    SetConsoleTextAttribute(hStdout, bi.wAttributes);
#define LOG_INFO_PREFIX         "[INFO] "
#define LOG_ERROR_PREFIX        "[ERROR] "
#define LOG_WARNING_PREFIX      "[WARN] "

namespace clstd
{
  void OutputString(const ch* szString)
  {
    if(IsDebuggerPresent()) {
      OutputDebugStringA(szString);
    }
    fputs(szString, stdout); // TODO: 根据消息类型区分stdout/stderr
  }

  void OutputString(const wch* szString)
  {
    if(IsDebuggerPresent()) {
      OutputDebugStringW(reinterpret_cast<LPCWSTR>(szString));
    }
    fputws(reinterpret_cast<LPCWSTR>(szString), stdout); // TODO: 根据消息类型区分stdout/stderr
  }
}

#else
# define SET_TEXT_COLOR(_CR)
# define RESTORE_TEXT_COLOR()
#define LOG_INFO_PREFIX         "\e[1;0m[INFO] "
#define LOG_ERROR_PREFIX        "\e[1;31m[ERROR] "
#define LOG_WARNING_PREFIX      "\e[1;33m[WARN] "

namespace clstd
{
  void DumpMemory(const void* ptr, size_t count);

  void OutputString(const wch* szString)
  {
    const size_t count = strlenT(szString);
    if(count)
    {
      char buffer[MAX_TRACE_BUFFER];
      clStringAttachA str(buffer, sizeof(buffer), 0);
      StringUtility::ConvertToUtf8(str, szString, count);
      fputs(str.CStr(), stdout);
    }
  }

  void OutputString(const ch* szString)
  {
    size_t count = 0;
    b32 bASCII = TRUE;
    const ch* p = szString;
    while(*p) {
      if(*p++ & 0x80) {
        bASCII = FALSE;
      }
    }
    count = p - szString;

    if(count)
    {
      if(bASCII) {
        fputs(szString, stdout);
      }
      else
      {
        LocalBuffer<MAX_TRACE_BUFFER> buf;

        const size_t req_count = StringW_traits::XStringToNative(NULL, 0, szString, count);
        buf.Resize((req_count + 1) * sizeof(wch), FALSE);
        StringW_traits::XStringToNative((wch*)buf.GetPtr(), req_count, szString, count);
        ((wch*)buf.GetPtr())[req_count] = '\0';

        OutputString((const wch*)buf.GetPtr());
      }
    }
  }
}

#endif

#define LOG_INFO_PREFIX_W       _CLTEXT2(LOG_INFO_PREFIX)
#define LOG_ERROR_PREFIX_W      _CLTEXT2(LOG_ERROR_PREFIX)
#define LOG_WARNING_PREFIX_W    _CLTEXT2(LOG_WARNING_PREFIX)

#if 0
const static clstd::ALLOCPLOY aclAllocPloyW[] =
{
  {32, 1024},
  {48, 512},
  {64, 512},
  {96, 256},
  {128, 256},
  {256, 256},
  {512, 256},
  {1024, 64},
  {0,0},
};

const static clstd::ALLOCPLOY aclAllocPloyA[] =
{
  {16, 512},
  {32, 512},
  {48, 512},
  {64, 512},
  {96, 256},
  {128, 256},
  {256, 256},
  {512, 128},
  {1024, 32},
  {0,0},
};
#endif

clstd::StdAllocator g_StdAlloc;
clStringW s_strRootDir;

//#ifdef _DEBUG

template<typename _TCh>
void _cl_vtraceT(const _TCh *fmt, va_list val)
{
  _TCh buffer[MAX_TRACE_BUFFER];    
  clstd::StringAttachX<_TCh> str_attach(buffer, sizeof(buffer), 0);

  str_attach.VarFormat(fmt, val);

  if(str_attach.IsNotEmpty()) {
    clstd::OutputString(str_attach.CStr());
  }
}

template<typename _TCh>
  void _cl_vlogT(const _TCh* prefix, const _TCh* fmt, va_list val)
{
  size_t prefix_len = clstd::strlenT(prefix);
  size_t fmt_len = clstd::strlenT(fmt);

  clstd::LocalBuffer<MAX_TRACE_BUFFER> buffer;
  static _TCh s_szCRLF[] = {'\r', '\n', '\0'};
#if defined(_CL_SYSTEM_WINDOWS)
  buffer
    .Append(prefix, prefix_len * sizeof(_TCh))
    .Append(fmt, fmt_len * sizeof(_TCh))
    .Append(s_szCRLF, sizeof(s_szCRLF));
#else
  static _TCh s_szNormalClr[] = { '\e','[','0','m' };

  buffer
    .Append(prefix, prefix_len * sizeof(_TCh))
    .Append(fmt, fmt_len * sizeof(_TCh))
    .Append(s_szNormalClr, sizeof(s_szNormalClr))
    .Append(s_szCRLF, sizeof(s_szCRLF));
#endif

  _cl_vtraceT<_TCh>((const _TCh*)buffer.GetPtr(), val);
}

// 不能用clString,因为clString使用的分配池会调用TRACE
extern "C" void _cl_traceA(const char *fmt, ...)
{
  va_list val;
  va_start(val, fmt);
  _cl_vtraceT<char>(fmt, val);
  va_end(val);
}

extern "C" void _cl_traceW(const wch *fmt, ...)
{
  va_list val;
  va_start(val, fmt);
  _cl_vtraceT<wch>(fmt, val);
  va_end(val);
}

extern "C" void _cl_log_infoA(const char *fmt, ...)
{
  SET_TEXT_COLOR(0x07);

  va_list val;
  va_start(val, fmt);
  _cl_vlogT<char>(LOG_INFO_PREFIX, fmt, val);
  va_end(val);

  RESTORE_TEXT_COLOR();
}

extern "C" void _cl_log_errorA(const char *fmt, ...)
{
  SET_TEXT_COLOR(0x0C);

  va_list val;
  va_start(val, fmt);
  _cl_vlogT<char>(LOG_ERROR_PREFIX, fmt, val);
  va_end(val);

  RESTORE_TEXT_COLOR();
}

extern "C" void _cl_log_warningA(const char *fmt, ...)
{
  SET_TEXT_COLOR(0x0E);

  va_list val;
  va_start(val, fmt);
  _cl_vlogT<char>(LOG_WARNING_PREFIX, fmt, val);
  va_end(val);

  RESTORE_TEXT_COLOR();
}

extern "C" void _cl_log_infoW(const wch *fmt, ...)
{
  SET_TEXT_COLOR(0x07);

  va_list val;
  va_start(val, fmt);
  _cl_vlogT<wch>(LOG_INFO_PREFIX_W, fmt, val);
  va_end(val);

  RESTORE_TEXT_COLOR();
}

extern "C" void _cl_log_errorW(const wch *fmt, ...)
{
  SET_TEXT_COLOR(0x0C);

  va_list val;
  va_start(val, fmt);
  _cl_vlogT<wch>(LOG_ERROR_PREFIX_W, fmt, val);
  va_end(val);

  RESTORE_TEXT_COLOR();
}

extern "C" void _cl_log_warningW(const wch *fmt, ...)
{
  SET_TEXT_COLOR(0x0E);

  va_list val;
  va_start(val, fmt);
  _cl_vlogT<wch>(LOG_WARNING_PREFIX_W, fmt, val);
  va_end(val);

  RESTORE_TEXT_COLOR();
}

namespace clstd
{
  void _cl_log_info(const char *fmt, ...)
  {
    SET_TEXT_COLOR(0x07);

    va_list val;
    va_start(val, fmt);
    _cl_vlogT<char>(LOG_INFO_PREFIX, fmt, val);
    va_end(val);

    RESTORE_TEXT_COLOR();
  }

  void _cl_log_error(const char *fmt, ...)
  {
    SET_TEXT_COLOR(0x0C);

    va_list val;
    va_start(val, fmt);
    _cl_vlogT<char>(LOG_ERROR_PREFIX, fmt, val);
    va_end(val);

    RESTORE_TEXT_COLOR();
  }

  void _cl_log_warning(const char *fmt, ...)
  {
    SET_TEXT_COLOR(0x0E);

    va_list val;
    va_start(val, fmt);
    _cl_vlogT<char>(LOG_WARNING_PREFIX, fmt, val);
    va_end(val);

    RESTORE_TEXT_COLOR();
  }

  void* MemCopy(void* pDest, const void* pSrc, size_t count)
  {
    if(pDest == pSrc || count == 0) {
      return pDest;
    }
    else if(pDest > pSrc && (size_t)pDest < (size_t)pSrc + count) {
      // 覆盖内存拷贝
      for(size_t i = count - 1; i != (size_t)-1; i--) {
        static_cast<u8*>(pDest)[i] = static_cast<const u8*>(pSrc)[i];
      }
    }
    else {
      for(size_t i = 0; i < count; i++) {
        static_cast<u8*>(pDest)[i] = static_cast<const u8*>(pSrc)[i];
      }
    }
    return pDest;
  }

#if defined(_CL_SYSTEM_WINDOWS)
  void Sleep(u32 dwMilliseconds)
  {
    ::Sleep(dwMilliseconds);
  }
#else
  void Sleep(u32 dwMilliseconds)
  {
    ::usleep(dwMilliseconds * 1000);
  }
#endif

  void _cl_log_info(const wch *fmt, ...)
  {
    va_list val;
    va_start(val, fmt);
    _cl_vlogT<wch>(LOG_INFO_PREFIX_W, fmt, val);
    va_end(val);
  }

  void _cl_log_error(const wch *fmt, ...)
  {
    SET_TEXT_COLOR(0x0C);

    va_list val;
    va_start(val, fmt);
    _cl_vlogT<wch>(LOG_ERROR_PREFIX_W, fmt, val);
    va_end(val);

    RESTORE_TEXT_COLOR();
  }

  void _cl_log_warning(const wch *fmt, ...)
  {
    SET_TEXT_COLOR(0x0E);

    va_list val;
    va_start(val, fmt);
    _cl_vlogT<wch>(LOG_WARNING_PREFIX_W, fmt, val);
    va_end(val);
   
    RESTORE_TEXT_COLOR();
  }
}

extern "C" void _cl_WinVerifyFailure(const char *pszSrc, const char *pszSrcFile, int nLine, unsigned long dwErrorNum)
{
#ifdef _WIN32
  PCHAR  pszCaption = "系统API调用错误";
  LPSTR  lpBuffer;
  FormatMessageA(
    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
    NULL,
    dwErrorNum,
    LANG_NEUTRAL,
    (LPSTR)&lpBuffer,
    0,
    NULL
    );

  CHAR  buffer[1024];
  wsprintfA(
    buffer,
    ">%s(%d)\n%s失败:\n最后的错误错误代码是:%d - %s",
    pszSrcFile,
    nLine,
    pszSrc,
    dwErrorNum,
    lpBuffer
    ),
    OutputDebugStringA("\n====================== ");
  OutputDebugStringA(pszCaption);
  OutputDebugStringA(" ======================\n");
  OutputDebugStringA(buffer);
  MessageBoxA(NULL, buffer, pszCaption, MB_OK);
  LocalFree(lpBuffer);
#else
#endif
}

extern "C" void _cl_assertW(const wch *pszSrc, const wch *pszSrcFile,int nLine)
{
  const wch* pwszCaption = _CLTEXT("Assert failed");
  _cl_traceW(_CLTEXT("================== %s ==================\n>%s(%d): assert failed: \"%s\"\n\n\n"),
    pwszCaption,pszSrcFile, nLine, pszSrc);
}

#if defined(_CL_SYSTEM_WINDOWS) || defined(_CL_SYSTEM_UWP) || defined(_CONSOLE)
# ifdef _CL_ARCH_X86
extern "C" void _cl_Break()
{
  __asm int 3
}
# endif
#endif

//#endif

namespace clstd
{
  u16 bitswap(u16 w)
  {
    return ((w & 0xff) << 8) | ((w >> 8) & 0xff);
  }

  u32 bitswap(u32 dw)
  {
    return ((dw & 0xff) << 24) | ((dw & 0xff00) << 8) | ((dw >> 8) & 0xff00) | ((dw >> 24) & 0xff);
  }

  u64 bitswap(u64 qw)
  {
    return ((qw & 0xff) << 56) | ((qw & 0xff00) << 40) | ((qw & 0xff0000) << 24) | ((qw & 0xff000000) << 8) |
      ((qw >> 8) & 0xff000000) | ((qw >> 24) & 0xff0000) | ((qw >> 40) & 0xff00) | ((qw >> 56) & 0xff);
  }

#ifdef _WIN32
  CLLONG InterlockedIncrement(CLLONG volatile *Addend)
  {
    return ::InterlockedIncrement(Addend);
  }

  CLLONG InterlockedDecrement(CLLONG volatile *Addend)
  {
    return ::InterlockedDecrement(Addend);
  }

  CLLONG InterlockedExchange(CLLONG volatile *Target, CLLONG Value)
  {
    return ::InterlockedExchange(Target, Value);
  }

  CLLONG InterlockedExchangeAdd(CLLONG volatile *Addend, CLLONG Value)
  {
    return ::InterlockedExchangeAdd(Addend, Value);
  }

  CLLONG InterlockedCompareExchange(CLLONG volatile *Destination, CLLONG Exchange, CLLONG Comperand)
  {
    return ::InterlockedCompareExchange(Destination, Exchange, Comperand);
  }
#else
  CLLONG InterlockedIncrement(CLLONG volatile *Addend)
  {
		ASSERT(0);
    //return atomic_inc(Addend);
		return ++*Addend;
  }

  CLLONG InterlockedDecrement(CLLONG volatile *Addend)
  {
		ASSERT(0);
    //return atomic_dec(&Addend);
		return --*Addend;
  }

  CLLONG InterlockedExchange(CLLONG volatile *Target, CLLONG Value)
  {
		ASSERT(0);
		//STATIC_ASSERT(0);
		CLLONG v = *Target;
		*Target = Value;
		return v;
  }

  CLLONG InterlockedExchangeAdd(CLLONG volatile *Addend, CLLONG Value)
  {
		ASSERT(0);
    //STATIC_ASSERT(0);
		return ((*Addend) += Value);
  }

  /*CLLONG InterlockedCompareExchange(CLLONG volatile *Destination, CLLONG Exchange, CLLONG Comparand)
  {
		ASSERT(0);
    //STATIC_ASSERT(0);
  }*/
#endif // #ifdef _WIN32
} // namespace clstd

namespace clstd_cli
{
#if defined(_CL_SYSTEM_WINDOWS)
  int kbhit()
  {
    return ::_kbhit();
  }

  char getch()
  {
    return ::_getch();
  }
#elif defined(_CL_SYSTEM_LINUX)
  int kbhit()
  {
    struct termios oldt, newt;
    int ch;
    int oldf;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);
    if(ch != EOF)
    {
      ungetc(ch, stdin);
      return 1;
    }
    return 0;
  }

  char getch()
  {
    struct termios oldt, newt;
    int ch;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
    //return ::getchar();
  }
#endif
} // namespace clstd_cli
