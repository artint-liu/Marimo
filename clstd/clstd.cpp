#include "clstd.h"
#include "clString.H"
#if defined(_WINDOWS) || defined(_WIN32)
#include <Windows.h>
#include <Shlwapi.h>
#  include <vld.h>
#pragma comment(lib, "shlwapi.lib")
#pragma warning(disable : 4996)
#endif // #if defined(_WINDOWS) || defined(_WIN32)

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

#if defined(_WINDOWS) || defined(_WIN32)
template<typename _TCh,
  int vsnprintfT(_TCh*, size_t, const _TCh*, va_list),
  void __stdcall OutputDebugStringT(const _TCh*),
  int fprintfT(FILE*, const _TCh*, ...)>
void _cl_vtraceT(const _TCh *fmt, va_list val)
{
  //if(IsDebuggerPresent() == FALSE)
  //  return;

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
      if(IsDebuggerPresent()) {
        OutputDebugStringT(pBuffer);
      }
      fprintfT(stdout, pBuffer); // TODO: 根据消息类型区分stdout/stderr
    }

    if(pBuffer != buffer && pBuffer != NULL) {
      delete[] pBuffer;
      pBuffer = NULL;
    }
  } while(nWriteToBuf < 0);

  ASSERT(nWriteToBuf < MAX_TRACE_BUFFER * nTimes - 1 && nWriteToBuf >= 0);
}

template<typename _TCh,
  int vsnprintfT(_TCh*, size_t, const _TCh*, va_list),
  void __stdcall OutputDebugStringT(const _TCh*),
  int fprintfT(FILE*, const _TCh*, ...)>
  void _cl_vlogT(const _TCh* prefix, const _TCh* fmt, va_list val)
{
  size_t prefix_len = clstd::strlenT(prefix);
  size_t fmt_len = clstd::strlenT(fmt);
  size_t len = prefix_len + fmt_len + (sizeof('\r') + sizeof('\n') + sizeof('\0'));

  clstd::LocalBuffer<MAX_TRACE_BUFFER> buffer;
  static _TCh s_szCRLF[] = {'\r', '\n', '\0'};

  buffer
    .Append(prefix, prefix_len * sizeof(_TCh))
    .Append(fmt, fmt_len * sizeof(_TCh))
    .Append(s_szCRLF, sizeof(s_szCRLF));

  _cl_vtraceT<_TCh, vsnprintfT, OutputDebugStringT, fprintfT>((const _TCh*)buffer.GetPtr(), val);
}

// 不能用clString,因为clString使用的分配池会调用TRACE
extern "C" void _cl_traceA(const char *fmt, ...)
{
  va_list val;
  va_start(val, fmt);
  _cl_vtraceT<char, vsnprintf, OutputDebugStringA, fprintf>(fmt, val);
  va_end(val);
}

extern "C" void _cl_traceW(const wch *fmt, ...)
{
  va_list val;
  va_start(val, fmt);
  _cl_vtraceT<wch, _vsnwprintf, OutputDebugStringW, fwprintf>(fmt, val);
  va_end(val);
}

extern "C" void _cl_log_infoA(const char *fmt, ...)
{
  va_list val;
  va_start(val, fmt);
  _cl_vlogT<char, vsnprintf, OutputDebugStringA, fprintf>("[INFO] ", fmt, val);
  va_end(val);
}

extern "C" void _cl_log_errorA(const char *fmt, ...)
{
  va_list val;
  va_start(val, fmt);
  _cl_vlogT<char, vsnprintf, OutputDebugStringA, fprintf>("[ERROR] ", fmt, val);
  va_end(val);
}

extern "C" void _cl_log_warningA(const char *fmt, ...)
{
  va_list val;
  va_start(val, fmt);
  _cl_vlogT<char, vsnprintf, OutputDebugStringA, fprintf>("[WARN] ", fmt, val);
  va_end(val);
}

extern "C" void _cl_log_infoW(const wch *fmt, ...)
{
  va_list val;
  va_start(val, fmt);
  _cl_vlogT<wch, _vsnwprintf, OutputDebugStringW, fwprintf>(L"[INFO] ", fmt, val);
  va_end(val);
}

extern "C" void _cl_log_errorW(const wch *fmt, ...)
{
  va_list val;
  va_start(val, fmt);
  _cl_vlogT<wch, _vsnwprintf, OutputDebugStringW, fwprintf>(L"[ERROR] ", fmt, val);
  va_end(val);
}

extern "C" void _cl_log_warningW(const wch *fmt, ...)
{
  va_list val;
  va_start(val, fmt);
  _cl_vlogT<wch, _vsnwprintf, OutputDebugStringW, fwprintf>(L"[WARN] ", fmt, val);
  va_end(val);
}

namespace clstd
{
  void _cl_log_info(const char *fmt, ...)
  {
    va_list val;
    va_start(val, fmt);
    _cl_vlogT<char, vsnprintf, OutputDebugStringA, fprintf>("[INFO] ", fmt, val);
    va_end(val);
  }

  void _cl_log_error(const char *fmt, ...)
  {
    va_list val;
    va_start(val, fmt);
    _cl_vlogT<char, vsnprintf, OutputDebugStringA, fprintf>("[ERROR] ", fmt, val);
    va_end(val);
  }

  void _cl_log_warning(const char *fmt, ...)
  {
    va_list val;
    va_start(val, fmt);
    _cl_vlogT<char, vsnprintf, OutputDebugStringA, fprintf>("[WARN] ", fmt, val);
    va_end(val);
  }

  void _cl_log_info(const wch *fmt, ...)
  {
    va_list val;
    va_start(val, fmt);
    _cl_vlogT<wch, _vsnwprintf, OutputDebugStringW, fwprintf>(L"[INFO] ", fmt, val);
    va_end(val);
  }

  void _cl_log_error(const wch *fmt, ...)
  {
    va_list val;
    va_start(val, fmt);
    _cl_vlogT<wch, _vsnwprintf, OutputDebugStringW, fwprintf>(L"[ERROR] ", fmt, val);
    va_end(val);
  }

  void _cl_log_warning(const wch *fmt, ...)
  {
    va_list val;
    va_start(val, fmt);
    _cl_vlogT<wch, _vsnwprintf, OutputDebugStringW, fwprintf>(L"[WARN] ", fmt, val);
    va_end(val);
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
#endif //defined(_WINDOWS) || defined(_WIN32)

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
