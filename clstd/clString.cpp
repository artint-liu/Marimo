#include "clstd.h"
#include "clString.H"
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <ctype.h>

// 设置区域
// setlocale (LC_ALL,"");
//typedef clstd::Allocator clAllocator;

// string 头的表示结构,不是实际的结构
// gcc 默认的 wchar_t 是 4 bytes 的
static const struct STR_HEADER_EXP{
  void*         pAllocator;
  size_t        capacity;   // 最大容纳的字符数量(不含'\0')
  size_t        size;       // 字符串实际字符数量(不含'\0')
  ch            buf[4];
}s_EmptyStr = {NULL, 0, 0, '\0', '\0', '\0', '\0'};

const size_t  MAX_DIGITS = 80;

#define CLSTD_HEADER_SIZE           (MEMBER_OFFSET(STR_HEADER_EXP, buf))
#define CLSTR_PTR(_CLSTRBUF)        ((u8*)(_CLSTRBUF) - CLSTD_HEADER_SIZE)
#define CLSTR_LENGTH(_CLSTRBUF)     (*(size_t*)(CLSTR_PTR(_CLSTRBUF) + MEMBER_OFFSET(STR_HEADER_EXP, size)))
#define CLSTR_CAPACITY(_CLSTRBUF)   (*(size_t*)(CLSTR_PTR(_CLSTRBUF) + MEMBER_OFFSET(STR_HEADER_EXP, capacity)))
#define CLSTR_ALLOCATOR(_CLSTRBUF)  (*(_TAllocator**)(CLSTR_PTR(_CLSTRBUF) + MEMBER_OFFSET(STR_HEADER_EXP, pAllocator)))
#define CLSTR_EXTRA                 (CLSTD_HEADER_SIZE + sizeof(_TCh))  // 头尺寸 + 结尾符号
#define IS_EMPTY_MODEL_STR          ((u32_ptr)m_pBuf == (u32_ptr)s_EmptyStr.buf || CLSTR_ALLOCATOR(m_pBuf) == NULL)

#define _CLSTR_TEMPL template<typename _TCh, class _TAllocator, _TAllocator& alloc, class _Traits>
#define _CLSTR_IMPL StringX<_TCh, _TAllocator, alloc, _Traits>

//#ifdef _IOS
//clStringX<wch, g_Alloc_clStringW, clStringW_traits> g_clStringInstW;
//clStringX<ch, g_Alloc_clStringA, clstd::StringA_traits> g_clStringInstA;
//#endif // _IOS

template<typename _TCh>
_TCh* fcvt(double arg, int ndigits, int *decpt, int *sign);
template<typename _TCh>
_TCh* ecvt(double arg, int ndigits, int *decpt, int *sign);
template <typename _TCh>
void ReadlizeFloatString(_TCh* str, int nSignificance = 5); // nSignificance就是相当于一个容差


namespace clstd
{
  template<typename _TReal, typename _TCh>
  _TReal _xstrtofT(const _TCh *str);

  template<typename _TUInt, typename _TCh>
  _TUInt _xstrtouT(const _TCh *str);

  template<typename _TUInt, typename _TCh>
  _TUInt _xstrtouT(i32 radix, const _TCh *str, clsize len = -1);  // (2-36进制)字符串转无符号整数

  // string to integer
  template<typename _TInt, typename _TCh>
  _TInt _xstrtoiT(const _TCh *String);

  template<typename _TInt, typename _TCh>
  _TInt _xstrtoiT(i32 radix, const _TCh *str, clsize len = -1);  // (2-36进制)字符串转带符号整数

  // double(float) to string
  template<typename _TCh>
  int _ftoxstrT(double value, _TCh* ascii, int width, int prec1, ch format);

  // 有符号数字 => 字符串
  template<typename _TCh, typename _TNum, typename _TUNum>
  void _ltoxstrT(_TNum value, _TCh* pDest, size_t uSize, i32 radix, i32 upper);

  // 无符号数字 => 字符串
  template<typename _TCh, typename _TNum>
  void _ultoxstrT(_TNum value, _TCh* pDest, size_t uSize, i32 radix, i32 upper);

  // 数学分组：12345678 => 12,345,678

  // 有符号数字 => 字符串(带数学分组)
  template<typename _TCh, typename _TNum, typename _TUNum>
  void _ltogxstrT(_TNum value, _TCh* pDest, size_t uSize, i32 radix, i32 group, i32 upper);

  // 无符号数字 => 字符串(带数学分组)
  template<typename _TCh, typename _TNum>
  void _ultogxstrT(_TNum value, _TCh* pDest, size_t uSize, i32 radix, i32 group, i32 upper);
  
  //////////////////////////////////////////////////////////////////////////

  template float  _xstrtofT<float>(const wch* str);
  template float  _xstrtofT<float>(const  ch* str);
  template double _xstrtofT<double>(const wch* str);
  template double _xstrtofT<double>(const  ch* str);

  template u32    _xstrtouT<u32>(const wch* String);
  template u32    _xstrtouT<u32>(const  ch* String);
  template u32    _xstrtouT<u32>(i32 radix, const wch* str, clsize len);
  template u32    _xstrtouT<u32>(i32 radix, const  ch* str, clsize len);

  template u64    _xstrtouT<u64>(const wch* String);
  template u64    _xstrtouT<u64>(const  ch* String);
  template u64    _xstrtouT<u64>(i32 radix, const wch* str, clsize len);
  template u64    _xstrtouT<u64>(i32 radix, const  ch* str, clsize len);

  template i32    _xstrtoiT<i32>(const wch* str);
  template i32    _xstrtoiT<i32>(const  ch* str);
  template i32    _xstrtoiT<i32>(i32 radix, const wch* str, clsize len);
  template i32    _xstrtoiT<i32>(i32 radix, const  ch* str, clsize len);

  template i64    _xstrtoiT<i64>(const wch* str);
  template i64    _xstrtoiT<i64>(const  ch* str);
  template i64    _xstrtoiT<i64>(i32 radix, const wch* str, clsize len);
  template i64    _xstrtoiT<i64>(i32 radix, const  ch* str, clsize len);

  template int _ftoxstrT(double value, wch* ascii, int width, int prec1, ch format);
  template int _ftoxstrT(double value,  ch* ascii, int width, int prec1, ch format);

  template wch*   strcpyT   (wch* pDest, const wch* pSrc);
  template  ch*   strcpyT   ( ch* pDest, const  ch* pSrc);
  template wch*   strstrT   (wch* pStr, const wch* pSubStr);
  template  ch*   strstrT   ( ch* pStr, const  ch* pSubStr);
  template wch*   strcpynT  (wch* pDest, const wch* pSrc, clsize uCount);
  template  ch*   strcpynT  ( ch* pDest, const  ch* pSrc, clsize uCount);
  template int    strncmpiT ( const wch* str1, const wch* str2, clsize n);
  template int    strncmpiT ( const  ch* str1, const  ch* str2, clsize n);
  template int    strcmpiT  ( const wch* str1, const wch* str2);
  template int    strcmpiT  ( const  ch* str1, const  ch* str2);
  template int    strncmpT  ( const wch* str1, const wch* str2, clsize n);
  template int    strncmpT  ( const  ch* str1, const  ch* str2, clsize n);
  template int    strcmpT   ( const wch* str1, const wch* str2);
  template int    strcmpT   ( const  ch* str1, const  ch* str2);
  template wch*   strchrT   ( const wch* str, i32 ch);
  template  ch*   strchrT   ( const  ch* str, i32 ch);
  template clsize strlenT   ( const wch* str);
  template clsize strlenT   ( const  ch* str);

  template ch     tolowerT  (ch c);
  template wch    tolowerT  (wch c);

  template b32    IsNumericT(const wch* str, clsize len, i32 radix);
  template b32    IsNumericT(const  ch* str, clsize len, i32 radix);

  template u32    HashStringT(const wch* str);
  template u32    HashStringT(const  ch* str);
}

//const static CLALLOCPLOY aclAllocPloyW[] =
//{
//  {32, 1024},
//  {48, 512},
//  {64, 512},
//  {96, 256},
//  {128, 256},
//  {256, 256},
//  {512, 256},
//  {1024, 64},
//  {0,0},
//};
//
//const static CLALLOCPLOY aclAllocPloyA[] =
//{
//  {16, 512},
//  {32, 512},
//  {48, 512},
//  {64, 512},
//  {96, 256},
//  {128, 256},
//  {256, 256},
//  {512, 128},
//  {1024, 32},
//  {0,0},
//};

//extern clstd::Allocator g_Alloc_clStringW;//(aclAllocPloyW);
//extern clstd::Allocator g_Alloc_clStringA;//(aclAllocPloyA);
//extern clstd::StdAllocator g_StdAlloc;

//////////////////////////////////////////////////////////////////////////

clsize clstd::StringW_traits::StringLength(const wch* pStr)
{
  return wcslen(pStr);
}

clsize clstd::StringW_traits::XStringLength(const _XCh* pStrX)
{
  return strlen(pStrX);
}

wch* clstd::StringW_traits::CopyStringN(wch* pStrDest, const wch* pStrSrc, size_t uLength)
{
  return clstd::strcpynT(pStrDest, pStrSrc, uLength);
}

i32 clstd::StringW_traits::CompareString(const wch* pStr1, const wch* pStr2)
{
  return wcscmp(pStr1, pStr2);
}

i32 clstd::StringW_traits::CompareStringNoCase(const wch* pStr1, const wch* pStr2)
{
  return clstd::strcmpiT(pStr1, pStr2);
}

const wch* clstd::StringW_traits::StringSearchChar(const wch* pStr, wch cCh)
{
  return wcschr(pStr, cCh);
}

void clstd::StringW_traits::Unsigned32ToString(wch* pDestStr, size_t uMaxLength, u32 uNum, i32 nNumGroup)
{
  if(nNumGroup <= 0) {
    clstd::ultox(uNum, pDestStr, uMaxLength, 10);
  }
  else {
    clstd::_ultogxstrT(uNum, pDestStr, uMaxLength, 10, nNumGroup, 0);
  }
}

void clstd::StringW_traits::Integer32ToString(wch* pDestStr, size_t uMaxLength, i32 iNum, i32 nNumGroup)
{
  if(nNumGroup <= 0) {
    clstd::ltox(iNum, pDestStr, uMaxLength, 10);
  }
  else {
    clstd::_ltogxstrT<wch, i32, u32>(iNum, pDestStr, uMaxLength, 10, nNumGroup, 0);
  }
}

i32 clstd::StringW_traits::StringToInteger32(wch* pString)
{
  return clstd::_xstrtoiT<i32>(pString);
}

void clstd::StringW_traits::FloatToString(wch* pDestStr, size_t uMaxLength, float fNum, char mode)
{
  if(mode == 'F' || mode == 'E') {
    clstd::_ftoxstrT(fNum, pDestStr, (int)uMaxLength, 10, mode);
  }
  else {
    clstd::_ftoxstrT(fNum, pDestStr, (int)uMaxLength, 10, 'F');
    ReadlizeFloatString(pDestStr);
  }
}

void clstd::StringW_traits::Unsigned64ToString(wch* pDestStr, size_t uMaxLength, u64 uNum, i32 nNumGroup)
{
  if(nNumGroup <= 0) {
    clstd::ul64tox(uNum, pDestStr, uMaxLength, 10);
  }
  else {
    clstd::_ultogxstrT(uNum, pDestStr, uMaxLength, 10, nNumGroup, 0);
  }
}

void clstd::StringW_traits::Integer64ToString(wch* pDestStr, size_t uMaxLength, i64 iNum, i32 nNumGroup)
{
  if(nNumGroup <= 0) {
    clstd::l64tox(iNum, pDestStr, uMaxLength, 10);
  }
  else {
    clstd::_ltogxstrT<wch, i64, u64>(iNum, pDestStr, uMaxLength, 10, nNumGroup, 0);
  }
}

void clstd::StringW_traits::HexToLowerString(wch* pDestStr, size_t uMaxLength, u32 uValue)
{
  clstd::ultox(uValue, pDestStr, uMaxLength, 16);
}

void clstd::StringW_traits::HexToUpperString(wch* pDestStr, size_t uMaxLength, u32 uValue)
{
  clstd::ultox(uValue, pDestStr, uMaxLength, 16, 1);
}

void clstd::StringW_traits::BinaryToString(wch* pDestStr, size_t uMaxLength, u32 uValue)
{
  clstd::ultox(uValue, pDestStr, uMaxLength, 2);
}

void clstd::StringW_traits::OctalToString(wch* pDestStr, size_t uMaxLength, u32 uValue)
{
  clstd::ultox(uValue, pDestStr, uMaxLength, 8);
}

size_t clstd::StringW_traits::XStringToNative(wch* pNativeStr, size_t uLength, const _XCh* pStrX, size_t cchX)
{
#if defined(_WIN32) && !defined(_C_STANDARD)
   return (size_t)
     MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, 
     pStrX, (int)cchX, pNativeStr, (int)uLength);
#else
    setlocale(LC_ALL, "C");
    return (size_t)mbstowcs(pNativeStr, pStrX, uLength);
#endif // _WINDOWS
}
//////////////////////////////////////////////////////////////////////////
clsize clstd::StringA_traits::StringLength(const ch* pStr)
{
  return strlen(pStr);
}

clsize clstd::StringA_traits::XStringLength(const _XCh* pStrX)
{
  return wcslen(pStrX);
}

ch* clstd::StringA_traits::CopyStringN(ch* pStrDest, const ch* pStrSrc, size_t uLength)
{
  return clstd::strcpynT(pStrDest, pStrSrc, uLength);
}

i32 clstd::StringA_traits::CompareString(const ch* pStr1, const ch* pStr2)
{
  return strcmp(pStr1, pStr2);
}

i32 clstd::StringA_traits::CompareStringNoCase(const ch* pStr1, const ch* pStr2)
{
  return clstd::strcmpiT(pStr1, pStr2);
}

const ch* clstd::StringA_traits::StringSearchChar(const ch* pStr, ch cCh)
{
  return strchr(pStr, cCh);
}

void clstd::StringA_traits::Unsigned32ToString(ch* pDestStr, size_t uMaxLength, u32 uNum, i32 nNumGroup)
{
  if(nNumGroup <= 0) {
    clstd::ultox(uNum, pDestStr, uMaxLength, 10);
  }
  else {
    clstd::_ultogxstrT(uNum, pDestStr, uMaxLength, 10, nNumGroup, 0);
  }
}

void clstd::StringA_traits::Integer32ToString(ch* pDestStr, size_t uMaxLength, i32 iNum, i32 nNumGroup)
{
  if(nNumGroup <= 0) {
    clstd::ltox(iNum, pDestStr, uMaxLength, 10);
  }
  else {
    clstd::_ltogxstrT<ch, i32, u32>(iNum, pDestStr, uMaxLength, 10, nNumGroup, 0);
  }
}

i32 clstd::StringA_traits::StringToInteger32(ch* pString)
{
  return atoi(pString);
}

void clstd::StringA_traits::Unsigned64ToString(ch* pDestStr, size_t uMaxLength, u64 uNum, i32 nNumGroup)
{
  if(nNumGroup <= 0) {
    clstd::ul64tox(uNum, pDestStr, uMaxLength, 10);
  }
  else {
    clstd::_ultogxstrT(uNum, pDestStr, uMaxLength, 10, nNumGroup, 0);
  }
}

void clstd::StringA_traits::Integer64ToString(ch* pDestStr, size_t uMaxLength, i64 iNum, i32 nNumGroup)
{
  if(nNumGroup <= 0) {
    clstd::l64tox(iNum, pDestStr, uMaxLength, 10);
  }
  else {
    clstd::_ltogxstrT<ch, i64, u64>(iNum, pDestStr, uMaxLength, 10, nNumGroup, 0);
  }
}

void clstd::StringA_traits::FloatToString(ch* pDestStr, size_t uMaxLength, float fNum, char mode)
{
  if(mode == 'F' || mode == 'E') {
    clstd::_ftoxstrT(fNum, pDestStr, (int)uMaxLength, 10, mode);
  }
  else {
    clstd::_ftoxstrT(fNum, pDestStr, (int)uMaxLength, 10, 'F');
    ReadlizeFloatString(pDestStr);
  }
}

void clstd::StringA_traits::HexToLowerString(ch* pDestStr, size_t uMaxLength, u32 uValue)
{
  clstd::ultox(uValue, pDestStr, uMaxLength, 16);
}

void clstd::StringA_traits::HexToUpperString(ch* pDestStr, size_t uMaxLength, u32 uValue)
{
  clstd::ultox(uValue, pDestStr, uMaxLength, 16, 1);
}

void clstd::StringA_traits::BinaryToString(ch* pDestStr, size_t uMaxLength, u32 uValue)
{
  clstd::ultox(uValue, pDestStr, uMaxLength, 2);
}

void clstd::StringA_traits::OctalToString(ch* pDestStr, size_t uMaxLength, u32 uValue)
{
  clstd::ultox(uValue, pDestStr, uMaxLength, 8);
}

size_t clstd::StringA_traits::XStringToNative(ch* pNativeStr, size_t uLength, const _XCh* pStrX, size_t cchX)
{
#if defined(_WIN32) && !defined(_C_STANDARD)
  return (size_t)
    WideCharToMultiByte(CP_ACP, NULL, 
    pStrX, (int)cchX, pNativeStr, (int)uLength, NULL, NULL);
//#elif defined(_IOS)
//  return (size_t)SimpleUnicodeToASCII(pNativeStr, uLength, pStrX);
#else
  setlocale(LC_ALL, "");
  return (size_t)wcstombs(pNativeStr, pStrX, uLength);
#endif // _WINDOWS
}
//////////////////////////////////////////////////////////////////////////

namespace clstd
{
  _CLSTR_TEMPL
    _CLSTR_IMPL::StringX()
    : m_pBuf  ((_TCh*)s_EmptyStr.buf)
  {
  }

  _CLSTR_TEMPL
    _CLSTR_IMPL::StringX(const _TCh* pStr)
    : m_pBuf(NULL)
  {
    if(pStr == NULL) {
      m_pBuf = (_TCh*)s_EmptyStr.buf;
    }
    else {
      const clsize uStrLen = _Traits::StringLength(pStr);
      _AllocBuffer(&alloc, uStrLen);
      _Traits::CopyStringN(m_pBuf, pStr, uStrLen);
    }
  }

  _CLSTR_TEMPL
    _CLSTR_IMPL::StringX(const _TCh* pStr, size_t uCount)
    : m_pBuf(NULL)
  {
    _AllocBuffer(&alloc, uCount);
    _Traits::CopyStringN(m_pBuf, pStr, uCount);
  }

  _CLSTR_TEMPL
    _CLSTR_IMPL::StringX(const _XCh* pStrX)
    : m_pBuf(NULL)
  {
    if(pStrX == NULL) {
      m_pBuf = (_TCh*)s_EmptyStr.buf;
    }
    else {
      const size_t uInputLength = _Traits::XStringLength(pStrX);
      size_t uLength = _Traits::XStringToNative(NULL, 0, pStrX, uInputLength);
      _AllocBuffer(&alloc, uLength);

      _Traits::XStringToNative(m_pBuf, uLength, pStrX, uInputLength);
    }
  }

  _CLSTR_TEMPL
    _CLSTR_IMPL::StringX(const _XCh* pStrX, size_t uCount)
  {
    const size_t uInputLength = (uCount == (size_t)-1)
      ? _Traits::XStringLength(pStrX) : uCount;

    size_t uLength = _Traits::XStringToNative(NULL, 0, pStrX, uInputLength);
    _AllocBuffer(&alloc, uLength);

    _Traits::XStringToNative(
      m_pBuf, uLength, pStrX, uInputLength);
  }

  _CLSTR_TEMPL
    _CLSTR_IMPL::StringX(const _TCh cCh, size_t uCount)
    : m_pBuf(NULL)
  {
    _AllocBuffer(&alloc, uCount);

    for(size_t i = 0; i < uCount; i++)
      m_pBuf[i] = cCh;
  }
  //_CLSTR_TEMPL
  //_CLSTR_IMPL::StringX(StringX& clStr)
  //{
  //  allocLength(clStr.GetLength());
  //  _Traits::CopyStringN(m_pBuf, clStr.m_pBuf, clStr.GetLength());
  //}

  _CLSTR_TEMPL
    _CLSTR_IMPL::StringX(const StringX& clStr)
    : m_pBuf(NULL)
  {
    _AllocBuffer(&alloc, clStr.GetLength());
    _Traits::CopyStringN(m_pBuf, clStr.m_pBuf, clStr.GetLength());
  }

  _CLSTR_TEMPL
    _CLSTR_IMPL::StringX(const int nInteger)
    : m_pBuf(NULL)
  {
    _AllocBuffer(&alloc, MAX_DIGITS);

    _Traits::Integer32ToString(m_pBuf, MAX_DIGITS, nInteger, 0);
    _Reduce(_Traits::StringLength(m_pBuf));
  }


  _CLSTR_TEMPL
    _CLSTR_IMPL::StringX(const float fFloat, char mode)
    : m_pBuf(NULL)
  {
    _AllocBuffer(&alloc, MAX_DIGITS);

    _Traits::FloatToString(m_pBuf, MAX_DIGITS, fFloat, mode);
    _Reduce(_Traits::StringLength(m_pBuf));
  }

  _CLSTR_TEMPL
    _CLSTR_IMPL::StringX(const long lLong)
    : m_pBuf(NULL)
  {
    _AllocBuffer(&alloc, MAX_DIGITS);

    _Traits::Integer32ToString(m_pBuf, MAX_DIGITS, lLong, 0);
    _Reduce(_Traits::StringLength(m_pBuf));
  }

  _CLSTR_TEMPL
    _CLSTR_IMPL::StringX(const u32 uInteger)
    : m_pBuf(NULL)
  {
    _AllocBuffer(&alloc, MAX_DIGITS);

    _Traits::Unsigned32ToString(m_pBuf, MAX_DIGITS, uInteger, 0);
    _Reduce(_Traits::StringLength(m_pBuf));
  }

  //_CLSTR_TEMPL
  //  _CLSTR_IMPL::StringX(const unsigned long uLong)
  //  : m_pBuf(NULL)
  //{
  //  _Alloc(&alloc, MAX_DIGITS);

  //  _Traits::Unsigned32ToString(m_pBuf, MAX_DIGITS, uLong, 0);
  //  _Reduce(_Traits::StringLength(m_pBuf));
  //}

  _CLSTR_TEMPL
    _CLSTR_IMPL::StringX(const u64 val)
    : m_pBuf(NULL)
  {
    _AllocBuffer(&alloc, MAX_DIGITS);

//#if defined(_X86) || defined(_ARM)
//      _Traits::Unsigned32ToString(m_pBuf, MAX_DIGITS, val, 0);
//#elif defined(_X64) || defined(_ARM64)
      _Traits::Unsigned64ToString(m_pBuf, MAX_DIGITS, val, 0);
//#else
//# error 缺少CPU架构定义
//#endif

    _Reduce(_Traits::StringLength(m_pBuf));
  }

  _CLSTR_TEMPL
    _CLSTR_IMPL::~StringX()
  {
    _TAllocator* const pAlloc = CLSTR_ALLOCATOR(m_pBuf);

    // 如果容量是0,那么一定来自s_EmptyStr, 否则应该是正常的分配器
    const size_t uCapacity = CLSTR_CAPACITY(m_pBuf);
    ASSERT((uCapacity != 0 && ( ! IS_EMPTY_MODEL_STR)) ||
      (uCapacity == 0 /*&& IS_EMPTY_MODEL_STR */) && pAlloc == NULL);
    // 注释掉 "&& IS_EMPTY_MODEL_STR" 是因为不能以此判断, clstd多重连入时 s_EmptyStr 会有多个实例地址

    if(uCapacity != 0)
      pAlloc->Free(CLSTR_PTR(m_pBuf));
  }
  //_CLSTR_IMPL::StringX(const _TCh* pFmt, ...)
  //{
  //}

  _CLSTR_TEMPL
    _CLSTR_IMPL& _CLSTR_IMPL::operator=(const _TCh* pStr)
  {
    if(pStr == NULL) {
      Clear();
    }
    else {
      const clsize uStrLen = _Traits::StringLength(pStr);
      _ResizeLengthNoCopy(uStrLen);
      _Traits::CopyStringN(m_pBuf, pStr, uStrLen);
    }
    return *this;
  }

  _CLSTR_TEMPL
    _CLSTR_IMPL& _CLSTR_IMPL::operator=(const _XCh* pStrX)
  {
    if(pStrX == NULL) {
      Clear();
    }
    else {
      const size_t uInputLength = _Traits::XStringLength(pStrX);
      size_t uLength = _Traits::XStringToNative(NULL, 0, pStrX, uInputLength);
      _ResizeLengthNoCopy(uLength);
      _Traits::XStringToNative(m_pBuf, uLength, pStrX, uInputLength);
    }
    return *this;
  }

  _CLSTR_TEMPL
    _CLSTR_IMPL& _CLSTR_IMPL::operator=(const _TCh ch)
  {
    _ResizeLengthNoCopy(1);
    *m_pBuf = ch;
    return *this;
  }

  _CLSTR_TEMPL
    _CLSTR_IMPL& _CLSTR_IMPL::operator=(const StringX& clStr)
  {
    _ResizeLengthNoCopy(clStr.GetLength());
    _Traits::CopyStringN(m_pBuf, clStr.m_pBuf, clStr.GetLength());
    return *this;
  }

  _CLSTR_TEMPL
    _CLSTR_IMPL& _CLSTR_IMPL::operator=(const int nInteger)
  {
    _ResizeLengthNoCopy(MAX_DIGITS);
    _Traits::Integer32ToString(m_pBuf, MAX_DIGITS, nInteger, 0);
    _Reduce(_Traits::StringLength(m_pBuf));
    return *this;
  }

  _CLSTR_TEMPL
    _CLSTR_IMPL& _CLSTR_IMPL::operator=(const float fFloat)
  {
    _ResizeLengthNoCopy(MAX_DIGITS);
    _Traits::FloatToString(m_pBuf, MAX_DIGITS, fFloat, 'F');
    _Reduce(_Traits::StringLength(m_pBuf));
    return *this;
  }

  _CLSTR_TEMPL
    _CLSTR_IMPL& _CLSTR_IMPL::operator=(const long lLong)
  {
    _ResizeLengthNoCopy(MAX_DIGITS);
    _Traits::Integer32ToString(m_pBuf, MAX_DIGITS, lLong, 0);
    _Reduce(_Traits::StringLength(m_pBuf));
    return *this;
  }

  _CLSTR_TEMPL
    _CLSTR_IMPL& _CLSTR_IMPL::operator=(const unsigned int uInteger)
  {
    _ResizeLengthNoCopy(MAX_DIGITS);
    _Traits::Unsigned32ToString(m_pBuf, MAX_DIGITS, uInteger, 0);
    _Reduce(_Traits::StringLength(m_pBuf));
    return *this;
  }

  _CLSTR_TEMPL
    _CLSTR_IMPL& _CLSTR_IMPL::operator=(const unsigned long uLong)
  {
    _ResizeLengthNoCopy(MAX_DIGITS);
    _Traits::Unsigned32ToString(m_pBuf, MAX_DIGITS, uLong, 0);
    _Reduce(_Traits::StringLength(m_pBuf));
    return *this;
  }

  //_CLSTR_TEMPL
  //bool _CLSTR_IMPL::operator==(const _TCh* pStr) const
  //{
  //  return Compare(pStr) == 0;
  //}

  _CLSTR_TEMPL
    bool _CLSTR_IMPL::operator==(const StringX& clStr) const
  {
    return (CLSTR_LENGTH(m_pBuf) == CLSTR_LENGTH(clStr.m_pBuf) &&
      Compare(clStr.m_pBuf) == 0);
  }

  _CLSTR_TEMPL
    bool _CLSTR_IMPL::operator<(const StringX& clStr2) const
  {
    return (Compare(clStr2) < 0);
  }

  _CLSTR_TEMPL
    bool _CLSTR_IMPL::operator>(const StringX& clStr2) const
  {
    return (Compare(clStr2) > 0);
  }

  _CLSTR_TEMPL
    _CLSTR_IMPL _CLSTR_IMPL::operator+(const _TCh* pStr) const
  {
    _CLSTR_IMPL strTemp = *this;
    strTemp.Append(pStr);
    return strTemp;
  }

  _CLSTR_TEMPL
    _CLSTR_IMPL _CLSTR_IMPL::operator+(const _TCh ch) const
  {
    _CLSTR_IMPL strTemp = *this;
    strTemp.Append(ch);
    return strTemp;
  }

  _CLSTR_TEMPL
    _CLSTR_IMPL _CLSTR_IMPL::operator+(const _XCh* pStrX) const
  {
    _CLSTR_IMPL strTemp = *this;
    strTemp.Append(pStrX);
    return strTemp;
  }

  _CLSTR_TEMPL
    _CLSTR_IMPL _CLSTR_IMPL::operator+(const StringX& clStr2) const
  {
    _CLSTR_IMPL strTemp = *this;
    strTemp.Append(clStr2);
    return strTemp;
  }

  //_CLSTR_TEMPL
  //_CLSTR_IMPL _CLSTR_IMPL::operator+(int nInteger) const
  //{
  //  _CLSTR_IMPL strTemp = *this;
  //  strTemp+=nInteger;
  //  return strTemp;
  //}

  //_CLSTR_TEMPL
  //_CLSTR_IMPL _CLSTR_IMPL::operator+(float fFloat) const
  //{
  //  _CLSTR_IMPL strTemp = *this;
  //  strTemp+=fFloat;
  //  return strTemp;
  //}

  //_CLSTR_TEMPL
  //_CLSTR_IMPL _CLSTR_IMPL::operator+(long lLong) const
  //{
  //  _CLSTR_IMPL strTemp = *this;
  //  strTemp+=lLong;
  //  return strTemp;
  //}

  //_CLSTR_TEMPL
  //_CLSTR_IMPL _CLSTR_IMPL::operator+(unsigned int uInteger) const
  //{
  //  _CLSTR_IMPL strTemp = *this;
  //  strTemp+=uInteger;
  //  return strTemp;
  //}
  //
  //_CLSTR_TEMPL
  //_CLSTR_IMPL _CLSTR_IMPL::operator+(unsigned long uLong) const
  //{
  //  _CLSTR_IMPL strTemp = *this;
  //  strTemp+=uLong;
  //  return strTemp;
  //}

  _CLSTR_TEMPL
    _CLSTR_IMPL& _CLSTR_IMPL::operator+=(const _TCh* pStr)
  {
    const clsize uInputLen = _Traits::StringLength(pStr);
    const clsize uStrLen = CLSTR_LENGTH(m_pBuf);
    _ResizeLength(uStrLen + uInputLen);
    _Traits::CopyStringN(m_pBuf + uStrLen, pStr, uInputLen);
    return *this;
  }

  _CLSTR_TEMPL
    _CLSTR_IMPL& _CLSTR_IMPL::operator+=(const _XCh* pStrX)
  {
    Append(pStrX);
    return *this;
  }

  _CLSTR_TEMPL
    _CLSTR_IMPL& _CLSTR_IMPL::operator+=(const _TCh cCh)
  {
    _ResizeLength(CLSTR_LENGTH(m_pBuf) + 1);
    m_pBuf[CLSTR_LENGTH(m_pBuf) - 1] = cCh;
    return *this;
  }

  _CLSTR_TEMPL
    _CLSTR_IMPL& _CLSTR_IMPL::operator+=(const StringX& clStr)
  {
    const clsize uStrLen = CLSTR_LENGTH(m_pBuf);
    _ResizeLength(uStrLen + clStr.GetLength());
    _Traits::CopyStringN(m_pBuf + uStrLen, clStr.m_pBuf, clStr.GetLength());
    return *this;
  }

  //_CLSTR_TEMPL
  //_CLSTR_IMPL& _CLSTR_IMPL::operator+=(const int nInteger)
  //{
  //  const size_t uStrLength = CLSTR_LENGTH(m_pBuf);
  //  _ResizeLength(uStrLength + MAX_DIGITS);
  //  _Traits::Integer32ToString(m_pBuf + uStrLength, MAX_DIGITS, nInteger);
  //  _Reduce(_Traits::StringLength(m_pBuf));
  //  return *this;
  //}

  //_CLSTR_TEMPL
  //_CLSTR_IMPL& _CLSTR_IMPL::operator+=(const float fFloat)
  //{
  //  const size_t uStrLength = CLSTR_LENGTH(m_pBuf);
  //  _ResizeLength(uStrLength + MAX_DIGITS);
  //  _Traits::FloatToString(m_pBuf + uStrLength, MAX_DIGITS, fFloat);
  //  _Reduce(_Traits::StringLength(m_pBuf));
  //  return *this;
  //}

  //_CLSTR_TEMPL
  //_CLSTR_IMPL& _CLSTR_IMPL::operator+=(const long lLong)
  //{
  //  const size_t uStrLength = CLSTR_LENGTH(m_pBuf);
  //  _ResizeLength(uStrLength + MAX_DIGITS);
  //  _Traits::Integer32ToString(m_pBuf + uStrLength, MAX_DIGITS, lLong);
  //  _Reduce(_Traits::StringLength(m_pBuf));
  //  return *this;
  //}
  _CLSTR_TEMPL
    _CLSTR_IMPL& _CLSTR_IMPL::AppendFloat(float val, char mode)
  {
    const size_t uStrLength = CLSTR_LENGTH(m_pBuf);
    _ResizeLength(uStrLength + MAX_DIGITS);
    _Traits::FloatToString(m_pBuf + uStrLength, MAX_DIGITS, val, mode);
    _Reduce(_Traits::StringLength(m_pBuf));
    return *this;
  }

  _CLSTR_TEMPL
    _CLSTR_IMPL& _CLSTR_IMPL::AppendInteger32(s32 val, int nNumGroup)
  {
    const size_t uStrLength = CLSTR_LENGTH(m_pBuf);
    _ResizeLength(uStrLength + MAX_DIGITS);
    _Traits::Integer32ToString(m_pBuf + uStrLength, MAX_DIGITS, val, nNumGroup);
    _Reduce(_Traits::StringLength(m_pBuf));
    return *this;
  }

  _CLSTR_TEMPL
    _CLSTR_IMPL& _CLSTR_IMPL::AppendUInt32(u32 val, int nNumGroup)
  {
    const size_t uStrLength = CLSTR_LENGTH(m_pBuf);
    _ResizeLength(uStrLength + MAX_DIGITS);
    _Traits::Unsigned32ToString(m_pBuf + uStrLength, MAX_DIGITS, val, nNumGroup);
    _Reduce(_Traits::StringLength(m_pBuf));
    return *this;
  }

  _CLSTR_TEMPL
    _CLSTR_IMPL& _CLSTR_IMPL::AppendInteger64(s64 val, int nNumGroup)
  {
    const size_t uStrLength = CLSTR_LENGTH(m_pBuf);
    _ResizeLength(uStrLength + MAX_DIGITS);
    _Traits::Integer64ToString(m_pBuf + uStrLength, MAX_DIGITS, val, nNumGroup);
    _Reduce(_Traits::StringLength(m_pBuf));
    return *this;
  }

  _CLSTR_TEMPL
    _CLSTR_IMPL& _CLSTR_IMPL::AppendUInt64(u64 val, int nNumGroup)
  {
    const size_t uStrLength = CLSTR_LENGTH(m_pBuf);
    _ResizeLength(uStrLength + MAX_DIGITS);
    _Traits::Unsigned64ToString(m_pBuf + uStrLength, MAX_DIGITS, val, nNumGroup);
    _Reduce(_Traits::StringLength(m_pBuf));
    return *this;
  }

  //_CLSTR_TEMPL
  //_CLSTR_IMPL& _CLSTR_IMPL::operator+=(const unsigned int uInteger)
  //{
  //  const size_t uStrLength = CLSTR_LENGTH(m_pBuf);
  //  _ResizeLength(uStrLength + MAX_DIGITS);
  //  _Traits::Unsigned32ToString(m_pBuf + uStrLength, MAX_DIGITS, uInteger);
  //  _Reduce(_Traits::StringLength(m_pBuf));
  //  return *this;
  //}
  //
  //_CLSTR_TEMPL
  //_CLSTR_IMPL& _CLSTR_IMPL::operator+=(const unsigned long uLong)
  //{
  //  const size_t uStrLength = CLSTR_LENGTH(m_pBuf);
  //  _ResizeLength(uStrLength + MAX_DIGITS);
  //  _Traits::Unsigned32ToString(m_pBuf + uStrLength, MAX_DIGITS, uLong);
  //  _Reduce(_Traits::StringLength(m_pBuf));
  //  return *this;
  //}

  _CLSTR_TEMPL
    _TCh& _CLSTR_IMPL::operator[](int nIdx)
  {
    ASSERT(nIdx <= (int)CLSTR_LENGTH(m_pBuf));
    return m_pBuf[nIdx];
  }

  _CLSTR_TEMPL
    _TCh& _CLSTR_IMPL::operator[](clsize nIdx)
  {
    ASSERT(nIdx <= CLSTR_LENGTH(m_pBuf));
    return m_pBuf[nIdx];
  }

  //_CLSTR_TEMPL
  //  _TCh& _CLSTR_IMPL::operator[](clsize nIdx)
  //{
  //  ASSERT(nIdx <= (int)CLSTR_LENGTH(m_pBuf));
  //  return m_pBuf[nIdx];
  //}

  _CLSTR_TEMPL
    _TCh& _CLSTR_IMPL::Front() const
  {
    return m_pBuf[0];
  }

  _CLSTR_TEMPL
    _TCh& _CLSTR_IMPL::Back() const
  {
    return m_pBuf[CLSTR_LENGTH(m_pBuf) - 1];
  }

  _CLSTR_TEMPL
    _CLSTR_IMPL::operator const _TCh*() const
  {
    return m_pBuf;
  }

  _CLSTR_TEMPL
    size_t _CLSTR_IMPL::GetLength() const
  {
    ASSERT(CLSTR_LENGTH(m_pBuf) <= CLSTR_CAPACITY(m_pBuf));
    return CLSTR_LENGTH(m_pBuf);
  }

  _CLSTR_TEMPL
    size_t _CLSTR_IMPL::GetCapacity() const
  {
    return CLSTR_CAPACITY(m_pBuf);
  }

  _CLSTR_TEMPL
    _TCh* _CLSTR_IMPL::GetBuffer() const
  {
    return m_pBuf;
  }

  _CLSTR_TEMPL
    _TCh* _CLSTR_IMPL::GetBuffer(size_t nSize)
  {
    if(nSize > 0) {
      _ResizeLength(nSize);
    }
    return m_pBuf;
  }

  _CLSTR_TEMPL
    void _CLSTR_IMPL::Reserve(size_t nSize)
  {
    if(nSize > CLSTR_CAPACITY(m_pBuf)) {
      const size_t length = CLSTR_LENGTH(m_pBuf);
      _Grow(nSize);
      CLSTR_LENGTH(m_pBuf) = length;
      if(length == 0) {
        // 空字符串在扩容时没有复制操作，新空间可能没有正确的结尾
        m_pBuf[length] = '\0';
      }
    }
  }

  _CLSTR_TEMPL
    void _CLSTR_IMPL::ReleaseBuffer()
  {
    const size_t nBufferLength = _Traits::StringLength(m_pBuf);
    ASSERT(CLSTR_CAPACITY(m_pBuf) >= nBufferLength && // 不包含结尾'\0'
      CLSTR_LENGTH(m_pBuf) >= nBufferLength);
    CLSTR_LENGTH(m_pBuf) = nBufferLength;
  }

  _CLSTR_TEMPL
    size_t _CLSTR_IMPL::Format(const _TCh *pFmt, ...)
  {
    if( ! IS_EMPTY_MODEL_STR) {
      CLSTR_LENGTH(m_pBuf) = 0;
    }

    // 防止上面的条件判断失败，字符串长度又没有清零
    ASSERT(CLSTR_LENGTH(m_pBuf) == 0);

    va_list  arglist;
    va_start(arglist, pFmt);
    return VarFormat(pFmt, arglist);
  }

  _CLSTR_TEMPL
    int _CLSTR_IMPL::Compare(const _TCh* pStr) const
  {
    return _Traits::CompareString(m_pBuf, pStr);
  }

  _CLSTR_TEMPL
    int _CLSTR_IMPL::CompareNoCase(const _TCh* pStr) const
  {
    return _Traits::CompareStringNoCase(m_pBuf, pStr);
  }

  _CLSTR_TEMPL
    int _CLSTR_IMPL::Compare(const _TCh* pStr, size_t count) const
  {
    return clstd::strncmpT(m_pBuf, pStr, count);
  }

  _CLSTR_TEMPL
    int _CLSTR_IMPL::CompareNoCase(const _TCh* pStr, size_t count) const
  {
    return clstd::strncmpiT(m_pBuf, pStr, count);
  }

  _CLSTR_TEMPL
    b32 _CLSTR_IMPL::BeginsWith(const _TCh c) const
  {
    return CLSTR_LENGTH(m_pBuf) > 1 && m_pBuf[0] == c;
  }

  _CLSTR_TEMPL
    b32 _CLSTR_IMPL::BeginsWith(const _TCh* pStr) const
  {
    const size_t uStrLength = CLSTR_LENGTH(m_pBuf);
    const size_t uCmpLength = clstd::strlenT(pStr);
    if(uStrLength < uCmpLength) {
      return FALSE;
    }
    return clstd::strncmpT(m_pBuf, pStr, uCmpLength) == 0;
  }

  _CLSTR_TEMPL
    b32 _CLSTR_IMPL::BeginsWith(const _XCh* pStr) const
  {
    return EndsWith(StringX(pStr));
  }

  _CLSTR_TEMPL
    b32 _CLSTR_IMPL::EndsWith(const _TCh c) const
  {
    return CLSTR_LENGTH(m_pBuf) > 1 && m_pBuf[CLSTR_LENGTH(m_pBuf) - 1] == c;
  }

  _CLSTR_TEMPL
    b32 _CLSTR_IMPL::EndsWith(const _TCh* pStr) const
  {
    const size_t uStrLength = CLSTR_LENGTH(m_pBuf);
    const size_t uCmpLength = clstd::strlenT(pStr);
    if(uStrLength < uCmpLength) {
      return FALSE;
    }
    return clstd::strncmpT(m_pBuf + uStrLength - uCmpLength, pStr, uCmpLength) == 0;
  }

  _CLSTR_TEMPL
    b32 _CLSTR_IMPL::EndsWith(const _XCh* pStr) const
  {
    return EndsWith(StringX(pStr));
  }

  _CLSTR_TEMPL
    size_t _CLSTR_IMPL::Insert(size_t idx, _TCh cCh)
  {
    const size_t uStrLength = CLSTR_LENGTH(m_pBuf);
    _ResizeLength(uStrLength + 1);
    if(idx >= uStrLength)
    {
      m_pBuf[uStrLength] = cCh;
    }
    else
    {
      memcpy(m_pBuf + idx + 1, m_pBuf + idx, (uStrLength - idx + 1) * sizeof(_TCh));
      m_pBuf[idx] = cCh;
    }
    return uStrLength + 1;
  }

  _CLSTR_TEMPL
    size_t _CLSTR_IMPL::Insert(size_t idx, _TCh cCh, size_t count)
  {
    const size_t uStrLength = CLSTR_LENGTH(m_pBuf);
    _ResizeLength(uStrLength + count);
    if(idx >= uStrLength)
    {
      m_pBuf[uStrLength] = cCh;
    }
    else
    {
      memcpy(m_pBuf + idx + count, m_pBuf + idx, (uStrLength - idx + 1) * sizeof(_TCh));

      for(size_t i = 0; i < count; i++) {
        m_pBuf[i] = cCh;
      }
    }
    return uStrLength + count;
  }

  _CLSTR_TEMPL
    size_t _CLSTR_IMPL::Insert(size_t idx, const _TCh* pStr)
  {
    const size_t uStrLength = CLSTR_LENGTH(m_pBuf);
    const size_t uInputLength = _Traits::StringLength(pStr);
    _ResizeLength(uStrLength + uInputLength);
    if(idx >= uStrLength)
    {
      _Traits::CopyStringN(m_pBuf + uStrLength, pStr, uInputLength);
    }
    else
    {
      memcpy(m_pBuf + idx + uInputLength, m_pBuf + idx, 
        (uStrLength - idx + 1) * sizeof(_TCh));
      _Traits::CopyStringN(m_pBuf + idx, pStr, uInputLength);
    }
    return uStrLength + uInputLength;
  }

  _CLSTR_TEMPL
    size_t _CLSTR_IMPL::Remove(_TCh cCh)
  {
    const size_t uStrLength = CLSTR_LENGTH(m_pBuf);
    size_t i = 0, d = 0;
    for(; i < uStrLength; i++)
    {
      if(m_pBuf[i] != cCh)
      {
        m_pBuf[d] = m_pBuf[i];
        d++;
      }
    }
    if(i != d)
      _Reduce(d);
    return d;
  }

  _CLSTR_TEMPL
    size_t _CLSTR_IMPL::Remove(size_t idx, size_t uCount)
  {
    const size_t uStrLength = CLSTR_LENGTH(m_pBuf);
    if(uCount == (size_t)-1 || idx + uCount >= uStrLength)
    {
      _Reduce(idx);
      return idx;
    }
    _Traits::CopyStringN(m_pBuf + idx, m_pBuf + idx + uCount, uStrLength - idx - uCount);
    _Reduce(uStrLength - uCount);
    return uStrLength - uCount;
  }

  _CLSTR_TEMPL
    void _CLSTR_IMPL::TrimLeft(_TCh cTarget)
  {
    size_t i = 0;
    const size_t uStrLen = CLSTR_LENGTH(m_pBuf);
    for(;i < uStrLen; i++) {
      if(m_pBuf[i] != cTarget) {
        break;
      }
    }

    if(i != 0)
    {
      _Traits::CopyStringN(m_pBuf, m_pBuf + i, uStrLen - i);
      _Reduce(uStrLen - i);
    }
  }

  _CLSTR_TEMPL
    void _CLSTR_IMPL::TrimLeft(const _TCh* pTarget)
  {
    size_t i = 0;
    size_t n;
    const size_t uStrLen = CLSTR_LENGTH(m_pBuf);
    for(;i < uStrLen; i++)
    {
      for(n = 0;pTarget[n] != 0; n++)
        if(m_pBuf[i] == pTarget[n])
          break;
      if(m_pBuf[i] != pTarget[n])
        break;
    }
    if(i != 0)
    {
      _Traits::CopyStringN(m_pBuf, m_pBuf + i, uStrLen - i);
      _Reduce(uStrLen - i);
    }
  }

  _CLSTR_TEMPL
    void _CLSTR_IMPL::TrimRight(_TCh cTarget)
  {
    const clsize uStrLen = CLSTR_LENGTH(m_pBuf);
    clsize i = uStrLen - 1;
    for(;i != (clsize)-1; i--)
      if(m_pBuf[i] != cTarget)
        break;
    if(++i != uStrLen)
      _Reduce(i);
  }

  _CLSTR_TEMPL
    void _CLSTR_IMPL::TrimRight(const _TCh* pTarget)
  {
    const clsize uStrLen = CLSTR_LENGTH(m_pBuf);
    clsize i = uStrLen - 1;
    clsize n;
    for(;i != (clsize)-1; i--)
    {
      for(n = 0; pTarget[n] != 0; n++)
        if(m_pBuf[i] == pTarget[n])
          break;
      if(m_pBuf[i] != pTarget[n])
        break;
    }
    if(++i != uStrLen)
      _Reduce(i);
  }

  _CLSTR_TEMPL
  void _CLSTR_IMPL::TrimBoth(_TCh cTarget)
  {
    const size_t uStrLen = CLSTR_LENGTH(m_pBuf);
    size_t i0 = 0, i1 = uStrLen - 1;
    for(; i0 < uStrLen; i0++) {
      if(m_pBuf[i0] != cTarget) {
        break;
      }
    }

    for(; i1 > i0; i1--)
    {
      if(m_pBuf[i1] != cTarget) {
        break;
      }
    }

    if(i0 >= i1) {
      Clear();
      return;
    }

    if(i0 != 0 || i1 != uStrLen - 1)
    {
      i1 = i1 - i0 + 1; // 这里转换为长度
      _Traits::CopyStringN(m_pBuf, m_pBuf + i0, i1);
      _Reduce(i1);
    }
  }

  _CLSTR_TEMPL
  void _CLSTR_IMPL::Augment(const _TCh* szLeft, const _TCh* szRight)
  {
    const clsize left  = szLeft ? _Traits::StringLength(szLeft) : 0;
    const clsize right = szRight ?  _Traits::StringLength(szRight) : 0;
    const size_t uStrLen = CLSTR_LENGTH(m_pBuf);
    _ResizeLength(left + uStrLen + right);

    if(szLeft) {
      _Traits::CopyStringN(m_pBuf + left, m_pBuf, uStrLen);
      _Traits::CopyStringN(m_pBuf, szLeft, left);
    }

    if(szRight) {
      _Traits::CopyStringN(m_pBuf + left + uStrLen, szRight, right);
    }
  }


  _CLSTR_TEMPL
    b32 _CLSTR_IMPL::IsEmpty() const
  {
    return this == 0 || CLSTR_LENGTH(m_pBuf) == 0;
  }

  _CLSTR_TEMPL
    b32 _CLSTR_IMPL::IsNotEmpty() const
  {
    return this && CLSTR_LENGTH(m_pBuf) > 0;
  }

  _CLSTR_TEMPL
    b32 _CLSTR_IMPL::IsFloat() const // [NOT BEEN TESTED]
  {
    // 目前不支持1e2f这种科学计数形式
    const clsize uStrLen = CLSTR_LENGTH(m_pBuf);
    if(uStrLen == 0) { // null string
      return FALSE;
    }

    // bNumeric 用来防止".f"被认为是合法的
    b32 bNumeric = (*m_pBuf >= '0' && *m_pBuf <= '9');
    b32 bHasDot = FALSE;

    if(uStrLen == 1) {
      return bNumeric;
    }
    else if(( ! bNumeric) && 
      *m_pBuf != '+' && *m_pBuf != '-' && *m_pBuf != '.') {
        return FALSE;
    }

    for(clsize i = 1; i < uStrLen; i++)
    {
      const _TCh c = m_pBuf[i];
      if(c >= '0' && c <= '9') {
        bNumeric = TRUE;
        continue;
      }
      else if(c == '.' && ! bHasDot) {
        bHasDot = TRUE;
        continue;
      }
      else if((c == 'f' || c == 'F') && i == uStrLen - 1) {
        return bNumeric;
      }
      return FALSE;
    }
    return bNumeric;
  }

  _CLSTR_TEMPL
    b32 _CLSTR_IMPL::IsInteger() const // [NOT BEEN TESTED]
  {
    const clsize uStrLen = CLSTR_LENGTH(m_pBuf);

    if(uStrLen == 0) { // null string
      return FALSE;
    }
    else if(uStrLen == 1) {
      return (*m_pBuf >= '0' && *m_pBuf <= '9');
    }
    else if((*m_pBuf < '0' || *m_pBuf > '9') && *m_pBuf != '+' && *m_pBuf != '-') {
      return FALSE;
    }

    for(clsize i = 1; i < uStrLen; i++)
    {
      const _TCh c = m_pBuf[i];
      if(c < '0' || c > '9') {
        return FALSE;
      }
    }
    return TRUE;
  }

  _CLSTR_TEMPL
    b32 _CLSTR_IMPL::IsAlphanumeric() const // [NOT BEEN TESTED]
  {
    const clsize uStrLen = CLSTR_LENGTH(m_pBuf);
    if(uStrLen == 0) { // null string
      return FALSE;
    }

    for(clsize i = 0; i < uStrLen; i++)
    {
      const _TCh c = m_pBuf[i];
      if((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
        (c >= '0' && c <= '9') || c == '_') {
          continue;
      }
      return FALSE;
    }
    return TRUE;
  }

  _CLSTR_TEMPL
    void _CLSTR_IMPL::Clear()
  {
    _Reduce(0);
  }

  _CLSTR_TEMPL
    size_t _CLSTR_IMPL::Find(_TCh cFind, size_t uStart /* = 0 */) const
  {
    const _TCh* pFind = _Traits::StringSearchChar(m_pBuf + uStart, cFind);
    return pFind == NULL ? npos : pFind - m_pBuf;
  }

  _CLSTR_TEMPL
    size_t _CLSTR_IMPL::Find(const _TCh* pFind, size_t uStart /* = 0 */) const
  {
    const _TCh* pFindResult = clstd::strstrT<_TCh>(m_pBuf + uStart, pFind);
    return pFindResult == NULL ? npos : pFindResult - m_pBuf;
  }

  _CLSTR_TEMPL
    size_t _CLSTR_IMPL::FindAny(const _TCh* pCharList, size_t uStart) const
  {
    if(pCharList == NULL) {
      return npos;
    }

    size_t nLength = CLSTR_LENGTH(m_pBuf);
    const _TCh* pList = pCharList;

    for(size_t i = 0; i < nLength; i++)
    {
      while(*pList) {
        if(m_pBuf[i] == *pList) {
          return i;
        }
        pList++;
      }

      pList = pCharList;
    }
    return npos;
  }

  _CLSTR_TEMPL
    size_t _CLSTR_IMPL::FindAnyFromList(const _TCh* pCharList, size_t uStart /* = 0 */) const
  {
    if(pCharList == NULL) {
      return npos;
    }
    while(*pCharList) {
      size_t pos = Find(*pCharList, uStart);

      if(pos != npos) {
        return pos;
      }

      pCharList++;
    }
    return npos;
  }

  _CLSTR_TEMPL
    size_t _CLSTR_IMPL::ReverseFind(_TCh cFind) const
  {
    size_t uIdx = CLSTR_LENGTH(m_pBuf);
    while((int)--uIdx >= 0) {
      if(m_pBuf[uIdx] == cFind) {
        return uIdx;
      }
    }
    return npos;
  }

  _CLSTR_TEMPL
    size_t _CLSTR_IMPL::ReverseFind(_TCh cFind, int nStart, int nEnd) const
  {
    size_t uIdx = CLSTR_LENGTH(m_pBuf);

    if(nEnd > 0) {
      uIdx = (size_t)nEnd;
    }

    while((int)--uIdx >= nStart) {
      if(m_pBuf[uIdx] == cFind) {
        return uIdx;
    } }
    return npos;
  }

  _CLSTR_TEMPL
  size_t _CLSTR_IMPL::ReverseFindAny(const _TCh* pFindCharList) const
  {
    size_t uIdx = CLSTR_LENGTH(m_pBuf);
    while((int)--uIdx >= 0) {
      const _TCh* l = pFindCharList;
      while(*l) {
        if(m_pBuf[uIdx] == *l++) {
          return uIdx;
        }
      }
    }
    return npos;
  }

  _CLSTR_TEMPL
    _CLSTR_IMPL _CLSTR_IMPL::Left(size_t uCount) const
  {
    if(uCount > CLSTR_LENGTH(m_pBuf))
      uCount = CLSTR_LENGTH(m_pBuf);
    return _CLSTR_IMPL(m_pBuf, uCount);
  }

  _CLSTR_TEMPL
    _CLSTR_IMPL _CLSTR_IMPL::Right(size_t uCount) const
  {
    const size_t uStrLength = CLSTR_LENGTH(m_pBuf);
    if(uCount > uStrLength)
      uCount = uStrLength;
    return _CLSTR_IMPL(m_pBuf + uStrLength - uCount, uCount);
  }

  _CLSTR_TEMPL
    _CLSTR_IMPL _CLSTR_IMPL::SubString(size_t uStart, size_t uCount) const
  {
    const size_t uStrLength = CLSTR_LENGTH(m_pBuf);
    if(uStart + uCount > uStrLength || ((i32)uCount) < 0)
      uCount = uStrLength - uStart;
    if((i32)uCount < 0)
      return _CLSTR_IMPL();
    return _CLSTR_IMPL(m_pBuf + uStart, uCount);
  }

  _CLSTR_TEMPL
    size_t _CLSTR_IMPL::DivideBy(_TCh cCh, StringX& strFront, StringX& strBack) const
  {
    size_t pos = Find(cCh);
    if(pos != npos) {
      strFront = SubString(0, pos);
      strBack  = &m_pBuf[pos + 1];
    }
    else {
      strFront = *this;
      strBack.Clear();
    }
    return pos;
  }

  _CLSTR_TEMPL
    size_t _CLSTR_IMPL::ReverseDivideBy(_TCh cCh, StringX& strFront, StringX& strBack) const
  {
    size_t pos = ReverseFind(cCh);
    if(pos != npos) {
      strFront = SubString(0, pos);
      strBack  = &m_pBuf[pos + 1];
    }
    else {
      strFront = *this;
      strBack.Clear();
    }
    return pos;
  }

  _CLSTR_TEMPL
    i32 _CLSTR_IMPL::ToInteger(int nRadix) const
  {
    return clstd::xtoi(nRadix, m_pBuf);
  }

  _CLSTR_TEMPL
    double _CLSTR_IMPL::ToFloat() const
  {
    return clstd::_xstrtofT<double>(m_pBuf);
  }

  _CLSTR_TEMPL
    u32 _CLSTR_IMPL::GetHash() const
  {
    return HashStringT(&m_pBuf[0], CLSTR_LENGTH(m_pBuf));
    //u32 _Val = 2166136261U;

    //_TCh* pBegin = &m_pBuf[0];
    //_TCh* pEnd = &m_pBuf[CLSTR_LENGTH(m_pBuf)];
    //while (pBegin != pEnd)
    //  _Val = 16777619U * _Val ^ (u32)*pBegin++;
    //return (_Val);
  }

  _CLSTR_TEMPL
    u32 _CLSTR_IMPL::GetCRC32() const
  {
    extern u32 chksum_crc32 (unsigned char *block, unsigned int length);
    return chksum_crc32((u8*)&m_pBuf[0], (unsigned int)CLSTR_LENGTH(m_pBuf));
  }

  _CLSTR_TEMPL
    void _CLSTR_IMPL::Replace(size_t idx, size_t uCount, const _TCh* pStr)
  {
    const size_t uStrLength = CLSTR_LENGTH(m_pBuf);
    const size_t uInputLength = pStr == NULL ? 0 : _Traits::StringLength(pStr);
    if(idx > uStrLength)
      idx = uStrLength;

    if(idx + uCount > uStrLength || ((i32)uCount) < 0)
      uCount = uStrLength - idx;

    if(uCount < uInputLength) {
      _ResizeLength(uStrLength - uCount + uInputLength);
    }

    if(uStrLength - idx - uCount != 0 && uInputLength != uCount)
      memcpy(m_pBuf + idx + uInputLength, 
      m_pBuf + idx + uCount, (uStrLength - idx - uCount) * sizeof(_TCh));

    if(uCount > uInputLength)
      _Reduce(uStrLength - uCount + uInputLength);
    if(pStr != NULL)
      _Traits::CopyStringN(m_pBuf + idx, pStr, uInputLength);
  }

  _CLSTR_TEMPL
    size_t _CLSTR_IMPL::Replace(_TCh cFind, _TCh cReplaceWith, size_t uStart/* = 0*/)
  {
    const size_t uStrLength = CLSTR_LENGTH(m_pBuf);
    if(uStart >= uStrLength)
      return 0;
    size_t nCount = 0;
    for(size_t i = uStart; i < uStrLength; i++)
    {
      if(m_pBuf[i] == cFind)
      {
        m_pBuf[i] = cReplaceWith;
        nCount++;
      }
    }
    return nCount;
  }

  _CLSTR_TEMPL
    _CLSTR_IMPL& _CLSTR_IMPL::Append(const _TCh *pStr)
  {
    const clsize uInputLen = _Traits::StringLength(pStr);
    return Append(pStr, uInputLen);
  }

  _CLSTR_TEMPL
    _CLSTR_IMPL& _CLSTR_IMPL::Append(const _XCh *pStrX)
  {
    const size_t uInputLength = _Traits::XStringLength(pStrX);
    const size_t uStrLen = CLSTR_LENGTH(m_pBuf);

    size_t uLength = _Traits::XStringToNative(NULL, 0, pStrX, uInputLength);

    _ResizeLength(uLength + uStrLen);
    _Traits::XStringToNative(m_pBuf + uStrLen, uLength, pStrX, uInputLength);

    return *this;
  }

  _CLSTR_TEMPL
    _CLSTR_IMPL& _CLSTR_IMPL::Append(const _XCh* pStrX, size_t uCount)
  {
    const size_t uStrLen = CLSTR_LENGTH(m_pBuf);

    size_t uLength = _Traits::XStringToNative(NULL, 0, pStrX, uCount);

    _ResizeLength(uLength + uStrLen);
    _Traits::XStringToNative(m_pBuf + uStrLen, uLength, pStrX, uCount);

    return *this;
  }

  _CLSTR_TEMPL
    _CLSTR_IMPL& _CLSTR_IMPL::Append(const _TCh *pStr, size_t uCount)
  {
    const clsize uStrLen = CLSTR_LENGTH(m_pBuf);
    _ResizeLength(uStrLen + uCount);
    _Traits::CopyStringN(m_pBuf + uStrLen, pStr, uCount);
    return *this;
  }

  _CLSTR_TEMPL
    _CLSTR_IMPL& _CLSTR_IMPL::Append(const _TCh *pStr, _TCh c, long nWidth)
  {
    const clsize uInputLen = _Traits::StringLength(pStr);

    if(nWidth == 0) {
      return Append(pStr, uInputLen);
    }
    else if(nWidth > 0) {
      nWidth -= (long)uInputLen;
      if(nWidth > 0) {
        Append(c, nWidth);
      }
      Append(pStr);
    }
    else { // if(nWidth < 0)
      ASSERT(nWidth < 0);
      Append(pStr);
      nWidth += (long)uInputLen;
      if(nWidth < 0) {
        Append(c, -nWidth);
      }
    }
    return *this;
  }

  _CLSTR_TEMPL
    _CLSTR_IMPL& _CLSTR_IMPL::Append(_TCh cCh)
  {
    const clsize uStrLen = CLSTR_LENGTH(m_pBuf);
    _ResizeLength(uStrLen + 1);
    m_pBuf[uStrLen] = cCh;
    return *this;
  }

  _CLSTR_TEMPL
    _CLSTR_IMPL& _CLSTR_IMPL::Append(_TCh cCh, size_t uCount)
  {
    const size_t uPrevLength = CLSTR_LENGTH(m_pBuf);
    _ResizeLength(uPrevLength + uCount);
    for(size_t i = 0; i < uCount; i++) {
      m_pBuf[uPrevLength + i] = cCh;
    }
    return *this;
  }

  _CLSTR_TEMPL
    _CLSTR_IMPL& _CLSTR_IMPL::Append(const StringX& clStr)
  {
    const clsize uStrLen = CLSTR_LENGTH(m_pBuf);
    _ResizeLength(uStrLen + clStr.GetLength());
    _Traits::CopyStringN(m_pBuf + uStrLen, clStr.m_pBuf, clStr.GetLength());
    return *this;
  }

  _CLSTR_TEMPL
    _CLSTR_IMPL& _CLSTR_IMPL::AppendFormat(const _TCh *pFmt, ...)
  {
    va_list  arglist;
    va_start(arglist, pFmt);
    VarFormat(pFmt, arglist);
    return *this;
  }

  _CLSTR_TEMPL
    void _CLSTR_IMPL::MakeReverser()
  {
    size_t uLength = CLSTR_LENGTH(m_pBuf);

    if(uLength == 0)
      return;

    const size_t uLoop = uLength >> 1;
    uLength--;

    for(size_t i = 0; i < uLoop; i++)
    {
      m_pBuf[i]           = m_pBuf[i] ^ m_pBuf[uLength - i];
      m_pBuf[uLength - i] = m_pBuf[i] ^ m_pBuf[uLength - i];
      m_pBuf[i]           = m_pBuf[i] ^ m_pBuf[uLength - i];
    }
  }

  _CLSTR_TEMPL
    void _CLSTR_IMPL::MakeUpper()
  {
    const size_t uLength = CLSTR_LENGTH(m_pBuf);
    for(size_t i = 0; i < uLength; i++)
    {
      _TCh& c = m_pBuf[i];
      if(c >= 'a' && c <= 'z')
        c = c - 'a' + 'A';
    }
  }

  _CLSTR_TEMPL
    void _CLSTR_IMPL::MakeLower()
  {
    const size_t uLength = CLSTR_LENGTH(m_pBuf);
    for(size_t i = 0; i < uLength; i++)
    {
      _TCh& c = m_pBuf[i];
      if(c >= 'A' && c <= 'Z')
        c = c - 'A' + 'a';
    }
  }


  _CLSTR_TEMPL
    size_t _CLSTR_IMPL::VarFormat(const _TCh *pFmt, va_list arglist)
  {
    const _TCh* ptr = pFmt;
    _TCh        buffer[MAX_DIGITS];  // 用来作为数字转换的缓冲区,对于32位整数和浮点数,转换为字符串后长度都不大于16
    int         i;

    while (*ptr != '\0')
    {
      const _TCh* ptr2 = _Traits::StringSearchChar(ptr, '%');
      if(ptr2 == NULL)
      {
        Append(ptr);
        break;
      }
      else
      {
        int nWidth = 0;
        int nLong = 0;
        b32 bZeroPrefix = FALSE;
        Append(ptr, ptr2 - ptr);
        ptr = ptr2 + 1;
SEQUENCE:
        switch(*ptr)
        {
        case '\0':
          goto FUNC_RET;
        case '%':
          Append((_TCh)'%');
          break;
        case 'l':
          ptr++;
          if(nLong < 2) {
            nLong++;
            goto SEQUENCE;
          }
          break;
        case 's':
          Append((_TCh*)va_arg(arglist, _TCh*), bZeroPrefix && nWidth > 0 ? '0' : ' ', nWidth);
          break;
        case 'c':
          Append((_TCh)va_arg(arglist, int/*_TCh*/));
          break;

        case 'd':
          if(nLong == 2) {
            _Traits::Integer64ToString(buffer, MAX_DIGITS, va_arg(arglist, i64), 0);
          }
          else {
            _Traits::Integer32ToString(buffer, MAX_DIGITS, va_arg(arglist, int), 0);
          }
          Append(buffer, bZeroPrefix && nWidth > 0 ? '0' : ' ', nWidth);
          break;
        case 'u':
          if(nLong == 2) {
            _Traits::Unsigned64ToString(buffer, MAX_DIGITS, va_arg(arglist, u64), 0);
          }
          else {
            _Traits::Unsigned32ToString(buffer, MAX_DIGITS, va_arg(arglist, unsigned long), 0);
          }
          Append(buffer);
          break;
        case 'f':
          //_gcvt_s(buffer, 16, va_arg(arglist, double), 5);
          //swprintf_s(buffer, MAX_DIGITS, L"%f", va_arg(arglist, double));
          _Traits::FloatToString(buffer, MAX_DIGITS, (float)va_arg(arglist, double), 'F');
          Append(buffer);
          break;

        case 'o':
          _Traits::OctalToString(buffer, MAX_DIGITS, va_arg(arglist, unsigned long));
          Append(buffer);
          break;

        case 'b':
          _Traits::BinaryToString(buffer, MAX_DIGITS, va_arg(arglist, unsigned long));
          Append(buffer);
          break;

        case 'X':
          _Traits::HexToUpperString(buffer, MAX_DIGITS, va_arg(arglist, unsigned long));

          nWidth -= (int)_Traits::StringLength(buffer);
          if(nWidth > 0)
            Append('0', nWidth);

          Append(buffer);
          break;

        case 'x':
          _Traits::HexToLowerString(buffer, MAX_DIGITS, va_arg(arglist, unsigned long));

          nWidth -= (int)_Traits::StringLength(buffer);
          if(nWidth > 0)
            Append('0', nWidth);

          Append(buffer);
          break;

        case '*':
          nWidth = (int)va_arg(arglist, int);
          ptr++;
          goto SEQUENCE;

        case '0':
          bZeroPrefix = TRUE;
          ptr++;
          goto SEQUENCE;

        case '.':  // "%.3f"
          i = 0;
          while(1)
          {
            ptr++;
            if(*ptr >= '0' && *ptr <= '9')
              buffer[i++] = *ptr;
            else if(*ptr == '\0')
              goto FUNC_RET;
            else if(*ptr == 'f' || i >= sizeof(buffer))
            {
              buffer[i] = '\0';
              nWidth = _Traits::StringToInteger32(buffer);

              _Traits::FloatToString(buffer, MAX_DIGITS, (float)va_arg(arglist, double), 'F');
              const _TCh* pDot = _Traits::StringSearchChar(buffer, '.');
              if(pDot != NULL) {
                int nn = nWidth;
                while(*++pDot != '\0' && nn--)        ; // 没错，就是分号！
                *(_TCh*)pDot = '\0';
              }            
              Append(buffer);

              //size_t nPos = ReverseFind('.');
              //if(nPos != npos)
              //{
              //  nPos = GetLength() - nPos - 1;
              //  if(nPos < sizeof(buffer))
              //  {
              //    nLen -= (int)nPos;
              //    if(nLen > 0)
              //      Append('\0', nLen);
              //  }
              //}
              break;
            }
            else
              break;
          }
          break;
        default:
          if(*ptr >= '0' && *ptr <= '9')  // "%8d"
          {
            i = 0;
            while(1)
            {
              if(*ptr >= '0' && *ptr <= '9')
                buffer[i++] = *ptr;
              else if(*ptr == '\0')
                goto FUNC_RET;
              else if(i >= sizeof(buffer))
                break;
              else if(*ptr == 'd' || *ptr == 'X' || *ptr == 'x')
              {
                buffer[i] = '\0';
                nWidth = _Traits::StringToInteger32(buffer);
                goto SEQUENCE;
              }
              //else if(*ptr == 'd' || i >= sizeof(buffer))
              //{
              //  buffer[i] = '\0';
              //  int nLen = _Traits::StringToInteger32(buffer);

              //  //_itow_s(va_arg(arglist, int), buffer, MAX_DIGITS, 10);
              //  _Traits::Integer32ToString(buffer, MAX_DIGITS, va_arg(arglist, int));

              //  nLen -= (int)_Traits::StringLength(buffer);
              //  if(nLen > 0)
              //    Append('0', nLen);

              //  operator+=(buffer);
              //  break;
              //}
              else
                break;
              ptr++;
            }
          }
        }
      }
      ptr++;
    }

FUNC_RET:
    va_end(arglist);
    return CLSTR_LENGTH(m_pBuf);
  }

  //////////////////////////////////////////////////////////////////////////

  _CLSTR_TEMPL
    void _CLSTR_IMPL::_ResizeLength(size_t uLength)
  {
    if(uLength > CLSTR_CAPACITY(m_pBuf))
    {
      _Grow(uLength);
    }
    else if(CLSTR_LENGTH(m_pBuf) == uLength) {
      return;
    }

    CLSTR_LENGTH(m_pBuf) = uLength;
    m_pBuf[uLength] = '\0';
  }

  _CLSTR_TEMPL
    void _CLSTR_IMPL::_ResizeLengthNoCopy(size_t uLength)
  {
    _TAllocator* pAlloc = CLSTR_ALLOCATOR(m_pBuf);
    if(pAlloc == NULL) {
      pAlloc = &alloc;
    }
#ifdef _X86
    ASSERT((int)pAlloc >= 0);
#endif // #ifdef _X86

    if(uLength > CLSTR_CAPACITY(m_pBuf))
    {
      if(CLSTR_CAPACITY(m_pBuf) != 0)
        pAlloc->Free(CLSTR_PTR(m_pBuf));

      _AllocBuffer(pAlloc, uLength);
    }
    else if(uLength == 0) {
      if(CLSTR_CAPACITY(m_pBuf) != 0)
        pAlloc->Free(CLSTR_PTR(m_pBuf));

      m_pBuf = (_TCh*)&s_EmptyStr.buf;
    }
    else
    {
      CLSTR_LENGTH(m_pBuf) = uLength;
      m_pBuf[uLength] = 0;
    }
  }

  _CLSTR_TEMPL
    void _CLSTR_IMPL::_Reduce(size_t uLength)
  {
    ASSERT(uLength <= CLSTR_LENGTH(m_pBuf));
    if(CLSTR_LENGTH(m_pBuf) != uLength) {
      CLSTR_LENGTH(m_pBuf) = uLength;
      m_pBuf[uLength] = 0;
    }
  }

  _CLSTR_TEMPL
    void _CLSTR_IMPL::_Grow(size_t uNewCapacity)
  {
    // 注意：这里重新分配缓冲后没有设置 CLSTR_LENGTH(m_pBuf) 这个要在外面设置
    ASSERT(uNewCapacity > CLSTR_CAPACITY(m_pBuf));

    _TAllocator* pAlloc = CLSTR_ALLOCATOR(m_pBuf);
    if(pAlloc == NULL) {
      pAlloc = &alloc;
    }

#ifdef _X86
    ASSERT((int)pAlloc >= 0);
#endif // #ifdef _X86

    clsize uCapacity;
    // 最后加上 CLSTD_HEADER_SIZE, 得到 string buffer 的实际地址
    void* ptrNew = (u8*)pAlloc->Alloc(uNewCapacity * sizeof(_TCh) + CLSTR_EXTRA, &uCapacity) + CLSTD_HEADER_SIZE;

    const size_t uStrLen = CLSTR_LENGTH(m_pBuf);
    if(uStrLen > 0) {
      _Traits::CopyStringN((_TCh*)ptrNew, m_pBuf, uStrLen + 1);
    }

    CLSTR_CAPACITY(ptrNew) = (uCapacity - CLSTR_EXTRA) / sizeof(_TCh);
    if(CLSTR_CAPACITY(m_pBuf) != 0) {
      pAlloc->Free(CLSTR_PTR(m_pBuf));
    }
    m_pBuf = (_TCh*)ptrNew;
    CLSTR_ALLOCATOR(m_pBuf) = pAlloc;
  }

  _CLSTR_TEMPL
    void _CLSTR_IMPL::_AllocBuffer(_TAllocator* pAlloc, size_t uLength)
  {
    clsize uCapacity;
    if(uLength == 0) {
      m_pBuf = (_TCh*)s_EmptyStr.buf;
    }
    else {
      m_pBuf = (_TCh*)((u8*)pAlloc->Alloc(uLength * sizeof(_TCh) + CLSTR_EXTRA, &uCapacity) + CLSTD_HEADER_SIZE);

      CLSTR_LENGTH(m_pBuf)    = uLength;
      CLSTR_CAPACITY(m_pBuf)  = (uCapacity - CLSTR_EXTRA) / sizeof(_TCh);
      CLSTR_ALLOCATOR(m_pBuf) = pAlloc;
      m_pBuf[uLength] = 0;
    }
  }
} // namespace clstd

extern wch wine_casemap_lower[];

namespace clstd
{
  const static ch c_RadixChar[][37] = {
    {"0123456789abcdefghijklmnopqrstuvwxyz"},
    {"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"},};

  template<typename _TCh>
  _TCh* strcpynT(_TCh* pDest, const _TCh* pSrc, size_t uCount)
  {
    _TCh* start = pDest;

    while (uCount && (*pDest++ = *pSrc++))    /* copy string */
      uCount--;

    if (uCount)                              /* pad out with zeroes */
      while (--uCount)
        *pDest++ = L'\0';

    return(start);
  }
  
  template<typename _TCh>
  _TCh* strcpyT(_TCh* pDest, const _TCh* pSrc)
  {
    _TCh* start = pDest;

    while ((*pDest++ = *pSrc++));    /* copy string */
    //*pDest++ = L'\0';

    return(start);
  }

  template<typename _TCh>
  _TCh* strstrT(_TCh* pStr, const _TCh* pSubStr)
  {
    int n;
    if(*pSubStr)
    {
      while(*pStr)
      {
        for(n=0; pStr[n] == pSubStr[n]; n++)
        {
          if(pSubStr[n + 1] == '\0')
            return pStr;
        }
        pStr++;
      }
      return NULL;
    }
    else
      return pStr;
  }

  template<typename _TCh, typename _TNum, typename _TUNum>
  void _ltoxstrT(_TNum value, _TCh* pDest, size_t uSize, i32 radix, i32 upper)
  {
    if(radix == 10 && value < 0)
    {
      *pDest++ = '-';
      uSize--;
      value = -value;
    }
    _ultoxstrT((_TUNum)value, pDest, uSize, radix, upper);
  }

  template<typename _TCh, typename _TNum>
  void _ultoxstrT(_TNum value, _TCh* pDest, size_t uSize, i32 radix, i32 upper)
  {
    size_t l = 0;
    uSize--;

    while(l < uSize)
    {
      pDest[l++] = c_RadixChar[upper][value % radix];
      value /= radix;
      if(value == 0)
        break;
    }

    pDest[l] = 0;
    size_t m = l >> 1;
    l--;

    // 首尾交换
    for(size_t i = 0; i < m; i++)
    {
      pDest[i] = pDest[i] ^ pDest[l - i];
      pDest[l - i] = pDest[i] ^ pDest[l - i];
      pDest[i] = pDest[i] ^ pDest[l - i];
    }
  }

  //
  // 有符号数字 => 字符串(带数学分组)
  //
  template<typename _TCh, typename _TNum, typename _TUNum>
  void _ltogxstrT(_TNum value, _TCh* pDest, size_t uSize, i32 radix, i32 group, i32 upper)
  {
    if(radix == 10 && value < 0)
    {
      *pDest++ = '-';
      uSize--;
      value = -value;
    }
    _ultogxstrT((_TUNum)value, pDest, uSize, radix, group, upper);
  }

  // 无符号数字 => 字符串(带数学分组)
  template<typename _TCh, typename _TNum>
  void _ultogxstrT(_TNum value, _TCh* pDest, size_t uSize, i32 radix, i32 group, i32 upper)
  {
    size_t l = 0;
    i32 g = 0;
    uSize--;

    while(l < uSize)
    {
      if(g >= group) {
        g = 0;
        pDest[l++] = ',';
        if(l >= uSize) {
          break;
        }
      }

      pDest[l++] = c_RadixChar[upper][value % radix];
      g++;

      value /= radix;
      if(value == 0)
        break;
    }

    pDest[l] = 0;
    size_t m = l >> 1;
    l--;

    // 首尾交换
    for(size_t i = 0; i < m; i++)
    {
      pDest[i] = pDest[i] ^ pDest[l - i];
      pDest[l - i] = pDest[i] ^ pDest[l - i];
      pDest[i] = pDest[i] ^ pDest[l - i];
    }
  }

  template<typename _TInt, typename _TCh>
  _TInt _xstrtoiT(const _TCh *String)
  {
    _TInt Value = 0, Digit;
    b32 bNeg = false;
    _TCh c;
    if(*String == '-') {
      bNeg = true;
      String++;
    }

    while ((c = *String++) != '\0') {
      if (c >= '0' && c <= '9') {
        Digit = (_TInt) (c - '0');
      } else {
        break;
      }

      Value = (Value * 10) + Digit;
    }
    return bNeg ? -Value : Value;
  }

  template<typename _TInt, typename _TCh>
  _TInt _xstrtoiT(i32 radix, const _TCh *str, clsize len)
  {
    if(radix < 2 || radix > 36) {
      return 0;
    }

    clsize i = 0;
    _TInt Value = 0, Digit;
    _TCh c;

    b32 bNeg = false;
    if(*str == '-') {
      bNeg = true;
      i++;
    }

    while (i < len && (c = str[i++]) != '\0') {

      if (c >= '0' && c <= '9')
        Digit = (_TInt)(c - '0');
      else if(c >= 'a' && c <= 'z')
        Digit = (_TInt)(c - 'a') + 10;
      else if(c >= 'A' && c <= 'Z')
        Digit = (_TInt)(c - 'A') + 10;
      else
        break;
      if(Digit >= (_TInt)radix)
        break;

      Value = (Value * radix) + Digit;
    }
    return bNeg ? -Value : Value;
  }

  template<typename _TUInt, typename _TCh>
  _TUInt _xstrtouT(const _TCh *str)
  {
    _TUInt Value = 0, Digit;
    _TCh c;

    while ((c = *str++) != '\0') {

      if (c >= '0' && c <= '9')
        Digit = (_TUInt)(c - '0');
      else
        break;

      Value = (Value * 10) + Digit;
    }
    return Value;
  }

  template<typename _TUInt, typename _TCh>
  _TUInt _xstrtouT(i32 radix, const _TCh *str, clsize len)
  {
    if(radix < 2 || radix > 36)
      return 0;

    clsize i = 0;
    _TUInt Value = 0, Digit;
    _TCh c;

    while (i < len && (c = str[i++]) != '\0') {

      if (c >= '0' && c <= '9')
        Digit = (_TUInt)(c - '0');
      else if(c >= 'a' && c <= 'z')
        Digit = (_TUInt)(c - 'a') + 10;
      else if(c >= 'A' && c <= 'Z')
        Digit = (_TUInt)(c - 'A') + 10;
      else
        break;
      if(Digit >= (_TUInt)radix)
        break;

      Value = (Value * radix) + Digit;
    }
    return Value;
  }


  template<typename _TReal, typename _TCh>
  _TReal _xstrtofT(const _TCh *str)
  {
    _TReal a(0.0);
    int e = 0;
    int c;
    b32 bNeg = false;

    if(*str == '-') {
      bNeg = true;
      str++;
    }

    while ((c = *str++) != '\0' && (c >= '0' && c <= '9')) {
      a = a * _TReal(10.0) + (c - '0');
    }

    if (c == '.') {
      while ((c = *str++) != '\0' && (c >= '0' && c <= '9')) {
        a = a * _TReal(10.0) + (c - '0');
        e--;
      }
    }

    if (c == 'e' || c == 'E') {
      int sign = 1;
      int i = 0;
      c = *str++;

      if (c == '+') {
        c = *str++;
      } else if (c == '-') {
        c = *str++;
        sign = -1;
      }

      while (isdigit(c)) {
        i = i * 10 + (c - '0');
        c = *str++;
      }

      e += i * sign;
    }

    while (e > 0) {
      a *= _TReal(10.0);
      e--;
    }

    while (e < 0) {
      a *= _TReal(0.1);
      e++;
    }

    return bNeg ? -a : a;
  }

  // ftoa 代码主要来自 2.11BSD
  template<typename _TCh>
  int _ftoxstrT(double value, _TCh* ascii, int width, int prec1, ch format)
  {
     int             expon;
     int             sign;
     register int    avail;
     register _TCh   *a;
     register _TCh   *p;
     ch              mode;
     int             lowercase;
     int             prec;
     //_TCh            *fcvt(), *ecvt();

     static _TCh nan[] = {'#','N','A','N', 0};
     if(value != value) {
       ASSERT(*(CLDWORD*)&value == 0xe0000000); // #nan
       // 没有的再加
       for(int i = 0; i < width; i++)
       {
         ascii[i] = nan[i];
         if(ascii[i] == '\0') {
           return i;
         }
       }
     }

     prec = prec1;
     mode = format;
     lowercase = 'a' - 'A';
     if (mode >= 'a')
       mode -= 'a' - 'A';
     else
       lowercase = 0;
 
     if (mode != 'E')
     {
       /* try 'F' style output */
       p = fcvt<_TCh>(value, prec, &expon, &sign);
       avail = width;
       a = ascii;
 
       /* output sign */
       if (sign)
       {
         avail--;
         *a++ = '-';
       }
 
       /* output '0' before the decimal point */
       if (expon <= 0)
       {
         *a++ = '0';
         avail--;
       }
 
       /* compute space length left after dec pt and fraction */
       avail -= prec + 1;
       if (mode == 'G')
         avail -= 4;
 
       if (avail >= expon)
       {
 
         /* it fits.  output */
         while (expon > 0)
         {
           /* output left of dp */
           expon--;
           if (*p)
           {
             *a++ = *p++;
           }
           else
             *a++ = '0';
         }
 
         /* output fraction (right of dec pt) */
         avail = expon;
         goto frac_out;
       }
       /* won't fit; let's hope for G format */
     }
 
     if (mode != 'F')
     {
       /* try to do E style output */
       p = ecvt<_TCh>(value, prec + 1, &expon, &sign);
       avail = width - 5;
       a = ascii;
 
       /* output the sign */
       if (sign)
       {
         *a++ = '-';
         avail--;
       }
     }
 
     /* check for field too small */
     if (mode == 'F' || avail < prec)
     {
       /* sorry joker, you lose */
       a = ascii;
       for (avail = width; avail > 0; avail--)
         *a++ = '*';
       *a = 0;
       return (0);
     }
 
     /* it fits; output the number */
     mode = 'E';
 
     /* output the LHS single digit */
     *a++ = *p++;
     expon--;
 
     /* output the rhs */
     avail = 1;
 
     frac_out:
     *a++ = '.';
     while (prec > 0)
     {
       prec--;
       if (avail < 0)
       {
         avail++;
         *a++ = '0';
       }
       else
       {
         if (*p)
           *a++ = *p++;
         else
           *a++ = '0';
       }
     }
 
     /* output the exponent */
     if (mode == 'E')
     {
       *a++ = 'E' + lowercase;
       if (expon < 0)
       {
         *a++ = '-';
         expon = -expon;
       }
       else
         *a++ = '+';
       *a++ = (expon / 10) % 10 + '0';
       *a++ = expon % 10 + '0';
     }
 
     /* output spaces on the end in G format */
     if (mode == 'G')
     {
       *a++ = ' ';
       *a++ = ' ';
       *a++ = ' ';
       *a++ = ' ';
     }
 
     /* finally, we can return */
     *a = 0;
     avail = (int)(a - ascii);
     return (avail);
   }

  //////////////////////////////////////////////////////////////////////////

  void ltox(i32 value, wch* pDest, size_t uSize, i32 radix, i32 upper)
  {
    _ltoxstrT<wch, i32, u32>(value, pDest, uSize, radix, upper);
  }

  void ltox(i32 value, ch* pDest, size_t uSize, i32 radix, i32 upper)
  {
    _ltoxstrT<ch, i32, u32>(value, pDest, uSize, radix, upper);
  }

  void l64tox(i64 value, wch* pDest, size_t uSize, i32 radix, i32 upper)
  {
    _ltoxstrT<wch, i64, u64>(value, pDest, uSize, radix, upper);
  }

  void l64tox(i64 value, ch* pDest, size_t uSize, i32 radix, i32 upper)
  {
    _ltoxstrT<ch, i64, u64>(value, pDest, uSize, radix, upper);
  }

  //////////////////////////////////////////////////////////////////////////
  
  i32 xtoi(const wch* str)
  {
    return _xstrtoiT<i32>(str);
  }

  i32 xtoi(const ch* str)
  {
    return _xstrtoiT<i32>(str);
  }

  i32 xtoi(i32 radix, const wch* str, clsize len)
  {
    return _xstrtoiT<i32>(radix, str, len);
  }

  i32 xtoi(i32 radix, const ch* str, clsize len)
  {
    return _xstrtoiT<i32>(radix, str, len);
  }

  i64 xtoi64(i32 radix, const wch* str, clsize len)
  {
    return _xstrtoiT<i64>(radix, str, len);
  }

  i64 xtoi64(i32 radix, const ch* str, clsize len)
  {
    return _xstrtoiT<i64>(radix, str, len);
  }

  //////////////////////////////////////////////////////////////////////////

  u32 xtou(const wch* str)
  {
    return _xstrtouT<u32>(str);
  }

  u32 xtou(const ch* str)
  {
    return _xstrtouT<u32>(str);
  }

  u32 xtou(i32 radix, const wch* str, clsize len)
  {
    return _xstrtouT<u32>(radix, str, len);
  }
  
  u32 xtou(i32 radix, const ch* str, clsize len)
  {
    return _xstrtouT<u32>(radix, str, len);
  }

  u64 xtou64(i32 radix, const wch* str, clsize len)
  {
    return _xstrtouT<u64>(radix, str, len);
  }

  u64 xtou64(i32 radix, const ch* str, clsize len)
  {
    return _xstrtouT<u64>(radix, str, len);
  }

  //////////////////////////////////////////////////////////////////////////

  void ultox(u32 value, wch* pDest, size_t uSize, i32 radix, i32 upper)
  {
    _ultoxstrT<wch, u32>(value, pDest, uSize, radix, upper);
  }

  void ultox(u32 value, ch* pDest, size_t uSize, i32 radix, i32 upper)
  {
    _ultoxstrT<ch, u32>(value, pDest, uSize, radix, upper);
  }

  void ul64tox(u64 value, wch* pDest, size_t uSize, i32 radix, i32 upper)
  {
    _ultoxstrT<wch, u64>(value, pDest, uSize, radix, upper);
  }

  void ul64tox(u64 value, ch* pDest, size_t uSize, i32 radix, i32 upper)
  {
    _ultoxstrT<ch, u64>(value, pDest, uSize, radix, upper);
  }

  //////////////////////////////////////////////////////////////////////////

  float xtof(const ch* str)
  {
    return clstd::_xstrtofT<float>(str);
  }
 
  float xtof(const wch* str)
  {
    return clstd::_xstrtofT<float>(str);
  }

  double xtod(const ch* str)
  {
    return clstd::_xstrtofT<double>(str);
  }

  double xtod(const wch* str)
  {
    return clstd::_xstrtofT<double>(str);
  }

  //////////////////////////////////////////////////////////////////////////

  template<typename _TCh>
  _TCh tolowerT(_TCh ch)
  {
    return ch + wine_casemap_lower[wine_casemap_lower[ch >> 8] + (ch & 0xff)];
  }

  template<typename _TCh>
  int strncmpiT( const _TCh *str1, const _TCh *str2, clsize n )
  {
    int ret = 0;
    for ( ; n > 0; n--, str1++, str2++)
      if ((ret = tolowerT(*str1) - tolowerT(*str2)) || !*str1) break;
    return ret;
  }

  template<typename _TCh>
  int strcmpiT(const _TCh *str1, const _TCh *str2)
  {
    int ret = 0;
    for ( ;;str1++, str2++)
      if ((ret = tolowerT(*str1) - tolowerT(*str2)) || !*str1) break;
    return ret;
  }

  template<typename _TCh>
  int strncmpT( const _TCh *str1, const _TCh *str2, clsize n)
  {
    int ret = 0;
    for ( ; n > 0; n--, str1++, str2++)
      if ((ret = (*str1) - (*str2)) || !*str1) break;
    return ret;
  }

  template<typename _TCh>
  int strcmpT(const _TCh *str1, const _TCh *str2)
  {
    int ret = 0;
    for ( ;;str1++, str2++)
      if ((ret = (*str1) - (*str2)) || !*str1) break;
    return ret;
  }

  template<typename _TCh>
  _TCh* strchrT( const _TCh *str, const i32 ch)
  {
    while (*str && *str != (_TCh)ch) {
      str++;
    }

    if (*str == (_TCh)ch) {
      return((_TCh*)str);
    }
    return NULL;
  }

  // 从 visual studio 2010 引用的代码
  template<typename _TCh>
  clsize strlenT( const _TCh* str)
  {
    const _TCh *eos = str;
    while( *eos++ ) ;
    return( eos - str - 1 );
  }

  template<typename _TCh>
  b32 IsNumericT(const _TCh* str, clsize len, i32 radix) // 没测
  {
    if(radix < 2 && radix > 36) {
      return FALSE;
    }
    else if(radix <= 10) {
      for(clsize i = 0; i < len && str[i] != '\0'; ++i) {
        const _TCh& c = str[i];
        if(c < '0' || c >= ('0' + radix)) {
          return FALSE;
        }
      }
    }
    else {
      radix -= 10;
      for(clsize i = 0; i < len && str[i] != '\0'; ++i)
      {
        const _TCh& c = str[i];
        if((c < '0' || c > '9') &&
           (c < 'a' || c >= 'a' + radix) &&
           (c < 'A' || c >= 'A' + radix) )
        {
          return FALSE;
        }
      }
    }
    return TRUE;
  }

  template<typename _TCh>
  u32 HashStringT(const _TCh* str, clsize len)
  {
    clsize _Val = 2166136261U;

    const _TCh* pBegin = str;
    const _TCh* pEnd = str + len;
    while (pBegin != pEnd) {
      _Val = 16777619U * _Val ^ (u32)*pBegin++;
    }
    return (u32)_Val;
  }

  template<typename _TCh>
  u32 HashStringT(const _TCh* str)
  {
    return HashStringT(str, strlenT(str));
  }

} // namespace clstd

extern "C" b32 strcmpnA(const ch* lpString1, const ch* lpString2, int nCount)
{
  return clstd::strncmpT(lpString1, lpString2, nCount) == 0;
  //return (CompareStringA(LOCALE_USER_DEFAULT, NULL, lpString1, nCount, lpString2, -1) - 2) == 0;
}

extern "C" b32 strcmpnW(const wch* lpString1, const wch* lpString2, int nCount)
{
  return clstd::strncmpT(lpString1, lpString2, nCount) == 0;
  //return (CompareStringW(LOCALE_USER_DEFAULT, NULL, lpString1, nCount, lpString2, -1) - 2) == 0;
}

// 可读化浮点字符串
// 调整格式化后的字符串修改为更容易阅读的数值
// 如"1.500000"改为"1.5", 或者"1.499999"改为"1.5"
template <typename _TCh>
void ReadlizeFloatString(_TCh* str, int nSignificance) // nSignificance就是相当于一个容差
{
  //TRACEW(L"%s => ", str);
  _TCh* c = clstd::strchrT(str, '.');
  if(c == NULL) {
    return;
  }

  _TCh* l0 = c;
  _TCh* l9 = c;
  c++;

  while(*c != '\0')
  {
    if(*c != '0' && c - l0 < nSignificance) {
      l0 = c;
    }
    if(*c != '9' && c - l9 < nSignificance) {
      l9 = c;
    }
    c++;
  }

  if(*l0 == '.')
  {
    l0[2] = '\0';
    ASSERT(c >= &l0[2]);
  }
  else if(*l9 == '.')
  {
    if(l9[-1] >= '0' && l9[-1] <= '8')
    {
      l9[2] = '\0';
      ASSERT(c >= &l9[2]);
      l9[-1]++;
    }
    else {
      l9[nSignificance] = '\0';
    }
  }
  else if(*l0 != '.' && l0[1] == '0' && l0 < l9)
  {
    l0[1] = '\0';
  }
  else if(*l9 != '.' && l9[1] == '9' && l9 < l0)
  {
    l9[1] = '\0';
    ASSERT(*l9 >= '0' && *l9 <= '8');
    (*l9)++;
  }
}

int SimpleASCIItoUnicode(wch* pDestStr, int nCount, const ch* pSrcStr)
{
  if(pSrcStr == NULL)
    return 0;
  int n = 0;
  ch c;
  while(pSrcStr[n] && (nCount == 0 || n < nCount))
  {
    c = pSrcStr[n];
    if(c < 0 && (int)c >= 128)
      c = '?';
    if(pDestStr != NULL)
      pDestStr[n] = c;
    n++;
  }
  if(pDestStr != NULL)
    pDestStr[n] = '\0';
  return n;
}

int SimpleUnicodeToASCII(ch* pDestStr, int nCount, const wch* pSrcStr)
{
  if(pSrcStr == NULL)
    return 0;
  int n = 0;
  wch c;
  while(pSrcStr[n] && (nCount == 0 || n < nCount))
  {
    c = pSrcStr[n];
    if(c < 0 && (int)c >= 128)
      c = '?';
    if(pDestStr != NULL)
      pDestStr[n] = (ch)c;
    n++;
  }
  if(pDestStr != NULL)
    pDestStr[n] = '\0';
  return n;
}

//template class clstd::StringX<wch, clstd::Allocator, g_Alloc_clStringW, clstd::StringW_traits>;
//template class clstd::StringX<ch, clstd::Allocator, g_Alloc_clStringA, clstd::StringA_traits>;
template class clstd::StringX<wch, clstd::StdAllocator, g_StdAlloc, clstd::StringW_traits>;
template class clstd::StringX<ch, clstd::StdAllocator, g_StdAlloc, clstd::StringA_traits>;

//////////////////////////////////////////////////////////////////////////
clStringW AnsiStringToUnicodeString(const clStringA& str)
{
  clStringW strOutput((const ch*)str);
  return strOutput;
}

clStringA UnicodeStringToAnsiString(const clStringW& str)
{
  clStringA strOutput = (const wch*)str;
  return strOutput;
}

// 任何时候clString都应该是一个指针的大小,否则变参函数入栈的内容会有问题!!
STATIC_ASSERT(sizeof(clStringA) == sizeof(void*));
STATIC_ASSERT(sizeof(clStringW) == sizeof(void*));
