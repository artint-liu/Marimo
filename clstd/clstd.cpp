#include "clstd.h"
#include "clString.h"

#if defined(_CL_SYSTEM_WINDOWS)
# include <conio.h>
# include <Windows.h>
# include <Shlwapi.h>
# include <vld.h>
# pragma comment(lib, "shlwapi.lib")
# pragma warning(disable : 4996)
#elif defined(_CL_SYSTEM_LINUX)
# include <stdio.h>
# include <termios.h>
# include <unistd.h>
# include <fcntl.h>
#endif

namespace clstd
{
//enum Output
}

#if defined(_CL_SYSTEM_WINDOWS)
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
      OutputDebugStringW(szString);
    }
    fputws(szString, stdout); // TODO: 根据消息类型区分stdout/stderr
  }
}

#else
# define SET_TEXT_COLOR(_CR)
# define RESTORE_TEXT_COLOR()
#define LOG_INFO_PREFIX         "\e[1;0m[INFO] "
#define LOG_ERROR_PREFIX        "\e[1;31m[ERROR] "
#define LOG_WARNING_PREFIX      "\e[1;33m[WARN] "

int _vsnwprintf(//_TCh*, size_t, const _TCh*, va_list
  wch* string, size_t count, const wch* format, va_list ap )
{
  return 0;
}

namespace clstd
{
  void OutputString(const ch* szString)
  {
    fputs(szString, stdout); // TODO: 根据消息类型区分stdout/stderr
  }

  void OutputString(const wch* szString)
  {
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

// s_strRootDir 要在 alloc 之前析构
//clstd::Allocator g_Alloc_clStringW("StringPoolW", aclAllocPloyW);
//clstd::Allocator g_Alloc_clStringA("StringPoolA", aclAllocPloyA);
clstd::StdAllocator g_StdAlloc;
clStringW s_strRootDir;

#define MAX_TRACE_BUFFER 4096
//#ifdef _DEBUG

//#if defined(_WINDOWS) || defined(_WIN32)
//#else
//void OutputDebugStringA( CLLPCSTR lpOutputString )
//{
//  puts(lpOutputString);
//}
//
//void OutputDebugStringW( CLLPCWSTR lpOutputString )
//{
//  puts("NOT implement OutputDebugStringW");
//}
//#endif

//class CLogAllocator
//{
//private:
//  s8 m_buffer[2048];
//  b32 m_bUsingLocal;
//
//public:
//  CLogAllocator()
//    : m_bUsingLocal(FALSE)
//  {}
//
//  void* Alloc(clsize nBytes, clsize* pCapacity)
//  {
//    if(nBytes <= sizeof(m_buffer) && ! m_bUsingLocal) {
//      *pCapacity = sizeof(m_buffer);
//      m_bUsingLocal = TRUE;
//      return &m_buffer;
//    }
//
//    *pCapacity = ALIGN_16(nBytes);
//    return new s8[*pCapacity];
//  }
//
//  void Free(void* ptr)
//  {
//    if(ptr == &m_buffer) {
//      ASSERT(m_bUsingLocal);
//      m_bUsingLocal = FALSE;
//    }
//    else {
//      delete ptr;
//    }
//  }
//};

template<typename _TCh,
  int vsnprintfT(_TCh*, size_t, const _TCh*, va_list)>
void _cl_vtraceT(const _TCh *fmt, va_list val)
{
  int nTimes = 1;
  int nWriteToBuf;
  _TCh buffer[MAX_TRACE_BUFFER];

  // 流程描述:
  // * 用原始缓冲尝试生成Trace信息
  // * 如果失败尝试分配使用两倍大小的缓冲生成
  // * 如果再失败尝试分配使用三倍大小的缓冲生成
  // ! 综上,超大字符串会比较慢
  do {
    const int nBufferSize = MAX_TRACE_BUFFER * nTimes;
    _TCh* pBuffer = nTimes == 1 ? buffer : new _TCh[nBufferSize];
    nWriteToBuf = vsnprintfT(pBuffer, nBufferSize, fmt, val);
    nTimes++;

    if(nWriteToBuf >= 0) {
      clstd::OutputString(buffer);
    }

    if(pBuffer != buffer && pBuffer != NULL) {
      delete[] pBuffer;
      pBuffer = NULL;
    }
  } while(nWriteToBuf < 0);

  ASSERT(nWriteToBuf < MAX_TRACE_BUFFER * nTimes - 1 && nWriteToBuf >= 0);
}

template<typename _TCh,
  int vsnprintfT(_TCh*, size_t, const _TCh*, va_list)>
  void _cl_vlogT(const _TCh* prefix, const _TCh* fmt, va_list val)
{
  size_t prefix_len = clstd::strlenT(prefix);
  size_t fmt_len = clstd::strlenT(fmt);
  //size_t len = prefix_len + fmt_len + (sizeof('\r') + sizeof('\n') + sizeof('\0'));

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

  _cl_vtraceT<_TCh, vsnprintfT>((const _TCh*)buffer.GetPtr(), val);
}

// 不能用clString,因为clString使用的分配池会调用TRACE
extern "C" void _cl_traceA(const char *fmt, ...)
{
  va_list val;
  va_start(val, fmt);
  _cl_vtraceT<char, vsnprintf>(fmt, val);
  va_end(val);
}

extern "C" void _cl_traceW(const wch *fmt, ...)
{
  va_list val;
  va_start(val, fmt);
  _cl_vtraceT<wch, _vsnwprintf>(fmt, val);
  va_end(val);
}

extern "C" void _cl_log_infoA(const char *fmt, ...)
{
  SET_TEXT_COLOR(0x07);

  va_list val;
  va_start(val, fmt);
  _cl_vlogT<char, vsnprintf>(LOG_INFO_PREFIX, fmt, val);
  va_end(val);

  RESTORE_TEXT_COLOR();
}

extern "C" void _cl_log_errorA(const char *fmt, ...)
{
  SET_TEXT_COLOR(0x0C);

  va_list val;
  va_start(val, fmt);
  _cl_vlogT<char, vsnprintf>(LOG_ERROR_PREFIX, fmt, val);
  va_end(val);

  RESTORE_TEXT_COLOR();
}

extern "C" void _cl_log_warningA(const char *fmt, ...)
{
  SET_TEXT_COLOR(0x0E);

  va_list val;
  va_start(val, fmt);
  _cl_vlogT<char, vsnprintf>(LOG_WARNING_PREFIX, fmt, val);
  va_end(val);

  RESTORE_TEXT_COLOR();
}

extern "C" void _cl_log_infoW(const wch *fmt, ...)
{
  SET_TEXT_COLOR(0x07);

  va_list val;
  va_start(val, fmt);
  _cl_vlogT<wch, _vsnwprintf>(LOG_INFO_PREFIX_W, fmt, val);
  va_end(val);

  RESTORE_TEXT_COLOR();
}

extern "C" void _cl_log_errorW(const wch *fmt, ...)
{
  SET_TEXT_COLOR(0x0C);

  va_list val;
  va_start(val, fmt);
  _cl_vlogT<wch, _vsnwprintf>(LOG_ERROR_PREFIX_W, fmt, val);
  va_end(val);

  RESTORE_TEXT_COLOR();
}

extern "C" void _cl_log_warningW(const wch *fmt, ...)
{
  SET_TEXT_COLOR(0x0E);

  va_list val;
  va_start(val, fmt);
  _cl_vlogT<wch, _vsnwprintf>(LOG_WARNING_PREFIX_W, fmt, val);
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
    _cl_vlogT<char, vsnprintf>(LOG_INFO_PREFIX, fmt, val);
    va_end(val);

    RESTORE_TEXT_COLOR();
  }

  void _cl_log_error(const char *fmt, ...)
  {
    SET_TEXT_COLOR(0x0C);

    va_list val;
    va_start(val, fmt);
    _cl_vlogT<char, vsnprintf>(LOG_ERROR_PREFIX, fmt, val);
    va_end(val);

    RESTORE_TEXT_COLOR();
  }

  void _cl_log_warning(const char *fmt, ...)
  {
    SET_TEXT_COLOR(0x0E);

    va_list val;
    va_start(val, fmt);
    _cl_vlogT<char, vsnprintf>(LOG_WARNING_PREFIX, fmt, val);
    va_end(val);

    RESTORE_TEXT_COLOR();
  }

  void _cl_log_info(const wch *fmt, ...)
  {
    va_list val;
    va_start(val, fmt);
    _cl_vlogT<wch, _vsnwprintf>(LOG_INFO_PREFIX_W, fmt, val);
    va_end(val);
  }

  void _cl_log_error(const wch *fmt, ...)
  {
    SET_TEXT_COLOR(0x0C);

    va_list val;
    va_start(val, fmt);
    _cl_vlogT<wch, _vsnwprintf>(LOG_ERROR_PREFIX_W, fmt, val);
    va_end(val);

    RESTORE_TEXT_COLOR();
  }

  void _cl_log_warning(const wch *fmt, ...)
  {
    SET_TEXT_COLOR(0x0E);

    va_list val;
    va_start(val, fmt);
    _cl_vlogT<wch, _vsnwprintf>(LOG_WARNING_PREFIX_W, fmt, val);
    va_end(val);
   
    RESTORE_TEXT_COLOR();
  }
}


/*/
extern "C" void _cl_traceA(char *fmt, ...)
{
  if(IsDebuggerPresent() == FALSE)
    return;

  char buffer[MAX_TRACE_BUFFER];
  va_list val;
  va_start(val, fmt);
  int nWriteToBuf = wvnsprintfA(buffer, MAX_TRACE_BUFFER, fmt, val);
  va_end(val);

  ASSERT(nWriteToBuf < MAX_TRACE_BUFFER - 1 && nWriteToBuf >= 0);

  OutputDebugStringA(buffer);
}

extern "C" void _cl_traceW(wch *fmt, ...)
{
  if(IsDebuggerPresent() == FALSE)
    return;

  WCHAR buffer[1024];
  va_list val;
  va_start(val, fmt);
  int nWriteToBuf = wvnsprintfW(buffer, MAX_TRACE_BUFFER, fmt, val);
  va_end(val);

  ASSERT(nWriteToBuf < MAX_TRACE_BUFFER - 1 && nWriteToBuf >= 0);
  OutputDebugStringW(buffer);
}
//*/
//#endif //defined(_WINDOWS) || defined(_WIN32)

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

//#endif


//extern "C" int cl_atoi(const char* szStr)
//{
//  return atoi(szStr);
//}
//
//extern "C" double cl_atof(const char* szStr)
//{
//  return atof(szStr);
//}

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
