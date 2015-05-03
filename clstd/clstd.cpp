#include "clstd.h"
#include "clString.H"
#if defined(_WINDOWS) || defined(_WIN32)
#include <Windows.h>
#include <Shlwapi.h>
#  include <vld.h>
#pragma comment(lib, "shlwapi.lib")
#pragma warning(disable : 4996)
#endif // #if defined(_WINDOWS) || defined(_WIN32)

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

// s_strRootDir Ҫ�� alloc ֮ǰ����
clstd::Allocator g_Alloc_clStringW("StringPoolW", aclAllocPloyW);
clstd::Allocator g_Alloc_clStringA("StringPoolA", aclAllocPloyA);
clstd::StdAllocator g_StdAlloc;
clStringW s_strRootDir;

#define MAX_TRACE_BUFFER 4096
//#ifdef _DEBUG

#if defined(_WINDOWS) || defined(_WIN32)
template<typename _TCh,
  int vsnprintfT(_TCh*, size_t, const _TCh*, va_list),
  void __stdcall OutputDebugStringT(const _TCh*),
  int printfT(const _TCh*, ...)>
void _cl_vtraceT(const _TCh *fmt, va_list val)
{
  //if(IsDebuggerPresent() == FALSE)
  //  return;

  int nTimes = 1;
  int nWriteToBuf;
  _TCh buffer[MAX_TRACE_BUFFER];

  // ��������:
  // * ��ԭʼ���峢������Trace��Ϣ
  // * ���ʧ�ܳ��Է���ʹ��������С�Ļ�������
  // * �����ʧ�ܳ��Է���ʹ��������С�Ļ�������
  // ! ����,�����ַ�����Ƚ���
  do {
    const int nBufferSize = MAX_TRACE_BUFFER * nTimes;
    _TCh* pBuffer = nTimes == 1 ? buffer : new _TCh[nBufferSize];
    nWriteToBuf = vsnprintfT(pBuffer, nBufferSize, fmt, val);
    nTimes++;

    if(nWriteToBuf >= 0) {
      if(IsDebuggerPresent()) {
        OutputDebugStringT(pBuffer);
      }
      else {
        printfT(pBuffer);
      }
    }

    if(pBuffer != buffer && pBuffer != NULL) {
      delete[] pBuffer;
      pBuffer = NULL;
    }
  } while(nWriteToBuf < 0);

  ASSERT(nWriteToBuf < MAX_TRACE_BUFFER * nTimes - 1 && nWriteToBuf >= 0);
}

// ������clString,��ΪclStringʹ�õķ���ػ����TRACE
extern "C" void _cl_traceA(const char *fmt, ...)
{
  //if(IsDebuggerPresent() == FALSE)
  //  return;

  //va_list val;
  //int nTimes = 1;
  //int nWriteToBuf;
  //char buffer[MAX_TRACE_BUFFER];

  //// ��������:
  //// * ��ԭʼ���峢������Trace��Ϣ
  //// * ���ʧ�ܳ��Է���ʹ��������С�Ļ�������
  //// * �����ʧ�ܳ��Է���ʹ��������С�Ļ�������
  //// ! ����Խ��Խ��
  //do {
  //  const int nBufferSize = MAX_TRACE_BUFFER * nTimes;
  //  char* pBuffer = nTimes == 1 ? buffer : new char[nBufferSize];
  //  va_start(val, fmt);
  //  nWriteToBuf = vsnprintf(pBuffer, nBufferSize, fmt, val);
  //  va_end(val);
  //  nTimes++;

  //  if(nWriteToBuf >= 0) {
  //    OutputDebugStringA(pBuffer);
  //  }

  //  if(pBuffer != buffer && pBuffer != NULL) {
  //    delete[] pBuffer;
  //    pBuffer = NULL;
  //  }
  //} while(nWriteToBuf < 0);

  //ASSERT(nWriteToBuf < MAX_TRACE_BUFFER * nTimes - 1 && nWriteToBuf >= 0);
  va_list val;
  va_start(val, fmt);
  _cl_vtraceT<char, vsnprintf, OutputDebugStringA, printf>(fmt, val);
  va_end(val);
}

extern "C" void _cl_traceW(const wchar_t *fmt, ...)
{
  //if(IsDebuggerPresent() == FALSE)
  //  return;

  //WCHAR buffer[MAX_TRACE_BUFFER];
  //va_list val;
  //va_start(val, fmt);
  //int nWriteToBuf = wvnsprintfW(buffer, MAX_TRACE_BUFFER, fmt, val);
  //va_end(val);

  //ASSERT(nWriteToBuf < MAX_TRACE_BUFFER - 1 && nWriteToBuf >= 0);
  //OutputDebugStringW(buffer);
  va_list val;
  va_start(val, fmt);
  _cl_vtraceT<wchar_t, _vsnwprintf, OutputDebugStringW, wprintf>(fmt, val);
  va_end(val);
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

extern "C" void _cl_traceW(wchar_t *fmt, ...)
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
  PCHAR  pszCaption = "ϵͳAPI���ô���";
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
    ">%s(%d)\n%sʧ��:\n���Ĵ�����������:%d - %s",
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

extern "C" void _cl_assertW(const wchar_t *pszSrc, const wchar_t *pszSrcFile,int nLine)
{
  const wchar_t* pwszCaption = L"Assert failed";
  _cl_traceW(L"================== %s ==================\n>%s(%d): assert failed: \"%s\"\n\n\n",
    pwszCaption,pszSrcFile, nLine, pszSrc);
}

//#endif


extern "C" int cl_atoi(const char* szStr)
{
  return atoi(szStr);
}

extern "C" double cl_atof(const char* szStr)
{
  return atof(szStr);
}

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
    return atomic_inc(Addend);
  }

  CLLONG InterlockedDecrement(CLLONG volatile *Addend)
  {
    return atomic_dec(&Addend);
  }

  CLLONG InterlockedExchange(CLLONG volatile *Target, CLLONG Value)
  {
    STATIC_ASSERT(0);
  }

  CLLONG InterlockedExchangeAdd(CLLONG volatile *Addend, CLLONG Value)
  {
    STATIC_ASSERT(0);
  }

  CLLONG InterlockedCompareExchange (CLLONG volatile *Destination, CLLONG Exchange, CLLONG Comparand)
  {
    STATIC_ASSERT(0);
  }
#endif // #ifdef _WIN32
} // namespace clstd