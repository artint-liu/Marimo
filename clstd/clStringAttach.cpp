#include "clstd.h"
#include <stdarg.h>
#include "clStringAttach.h"

#define _SHOW_NOT_(x) (!(x))
#define _CLSTRATTACH_TEMPL template<typename _TCh>
#define _CLSTRATTACH_IMPL  StringAttachX<_TCh>

namespace clstd
{
  template<typename _TCh>
  _TCh* strcpynT(_TCh* pDest, const _TCh* pSrc, size_t uCount);

  template<typename _TCh>
  _TCh* strchrT(const _TCh *str, i32 ch);

  template<typename _TCh>
  clsize strlenT(const _TCh* str);

  template<typename _TCh> // 比较str1的前n个字符,如果遇到'\0'则提前结束
  int strncmpT(const _TCh *str1, const _TCh *str2, clsize n);

  template<typename _TCh>
  int strcmpT(const _TCh *str1, const _TCh *str2);

  extern i32 xtoi(const wch* str);
  extern i32 xtoi(const ch* str);
  extern int ftox(double value, wch* pDest, int uSize, int prec1, ch format);
  extern int ftox(double value, ch* pDest, int uSize, int prec1, ch format);
  extern void ltox(i32 value, wch* pDest, size_t uSize, i32 radix, i32 upper);
  extern void ltox(i32 value, ch* pDest, size_t uSize, i32 radix, i32 upper);
  extern void ultox(u32 value, wch* pDest, size_t uSize, i32 radix, i32 upper = 0);
  extern void ultox(u32 value, ch* pDest, size_t uSize, i32 radix, i32 upper = 0);
  extern void l64tox(i64 value, wch* pDest, size_t uSize, i32 radix, i32 upper);
  extern void l64tox(i64 value, ch* pDest, size_t uSize, i32 radix, i32 upper);
  extern void ul64tox(u64 value, wch* pDest, size_t uSize, i32 radix, i32 upper = 0);
  extern void ul64tox(u64 value, ch* pDest, size_t uSize, i32 radix, i32 upper = 0);
} // namespace clstd

#include "clStringFormatted.h"

namespace clstd
{
  _CLSTRATTACH_TEMPL
    _CLSTRATTACH_IMPL::StringAttachX() : m_pBuf(NULL)
    , m_nCount(0)
    , m_nCapacity(0)
    , m_pAttached(NULL)
    , m_cbAttached(0)
  {
  }

  _CLSTRATTACH_TEMPL
    _CLSTRATTACH_IMPL::StringAttachX(void* pAttachedBuffer, size_t cbAttached)
    : m_pBuf(static_cast<TChar*>(pAttachedBuffer))
    , m_nCount(0)
    , m_nCapacity(cbAttached / sizeof(TChar))
    , m_pAttached(pAttachedBuffer)
    , m_cbAttached(cbAttached)
  {
    // 这里不使用strlen来检测长度，保证m_nCount <= m_nCapacity
    size_t i = 0;
    while(m_pBuf[i] != '\0' && i + 1 < m_nCapacity) {
      i++;
    }

    m_nCount = i;
  }

  _CLSTRATTACH_TEMPL
    _CLSTRATTACH_IMPL::StringAttachX(void* pAttachedBuffer, size_t cbAttached, size_t nLength)
    : m_pBuf(static_cast<TChar*>(pAttachedBuffer))
    , m_nCount(nLength)
    , m_nCapacity(cbAttached / sizeof(TChar))
    , m_pAttached(pAttachedBuffer)
    , m_cbAttached(cbAttached)
  {
  }

  _CLSTRATTACH_TEMPL
    _CLSTRATTACH_IMPL::~StringAttachX()
  {
    ASSERT(m_nCount < m_nCapacity);
    if(_SHOW_NOT_(IsAttached())) {
      SAFE_DELETE_ARRAY(m_pBuf);
    }
  }

  _CLSTRATTACH_TEMPL
    bool _CLSTRATTACH_IMPL::operator==(LPCSTR szStr) const
  {
    return Compare(szStr) == 0;
  }

  _CLSTRATTACH_TEMPL
  const _TCh* _CLSTRATTACH_IMPL::CStr() const
  {
    return m_pBuf;
  }

  _CLSTRATTACH_TEMPL
  size_t _CLSTRATTACH_IMPL::GetLength() const
  {
    ASSERT(m_nCount < m_nCapacity);
    return m_nCount;
  }

  _CLSTRATTACH_TEMPL
    b32 _CLSTRATTACH_IMPL::IsEmpty() const
  {
    return (m_nCount == 0);
  }

  _CLSTRATTACH_TEMPL
    b32 _CLSTRATTACH_IMPL::IsNotEmpty() const
  {
    return _SHOW_NOT_(IsEmpty());
  }

  _CLSTRATTACH_TEMPL
  b32 _CLSTRATTACH_IMPL::IsAttached() const
  {
    return m_pBuf == m_pAttached;
  }

  _CLSTRATTACH_TEMPL
    _CLSTRATTACH_IMPL& _CLSTRATTACH_IMPL::Clear()
  {
   if(IsAttached()) {
     ASSERT(m_nCapacity == m_cbAttached / sizeof(_TCh));
     ASSERT(m_nCount < m_nCapacity);
   }
   else if(m_pAttached) {
     SAFE_DELETE_ARRAY(m_pBuf);
     m_pBuf = (_TCh*)m_pAttached;
     m_nCapacity = m_cbAttached / sizeof(_TCh);
   }

   m_nCount = 0;
   m_pBuf[0] = '\0';
   return *this;
  }

  _CLSTRATTACH_TEMPL
    _CLSTRATTACH_IMPL& _CLSTRATTACH_IMPL::Reserve(size_t count)
  {
    ASSERT(m_nCount < m_nCapacity);
    if(count < m_nCapacity) {
      return *this;
    }

    size_t nNewCapacity = ALIGN_16(count + 1);
    _TCh* pNewBuf = new _TCh[nNewCapacity];

    if(m_pBuf)
    {
      memcpy(pNewBuf, m_pBuf, (m_nCount + 1) * sizeof(_TCh));

      // 构造函数中如果引用attach缓冲内容，则结尾可能不是'\0'
      if(pNewBuf[m_nCount] != '\0') {
        pNewBuf[m_nCount] = '\0';
      }

      if(_SHOW_NOT_(IsAttached())) {
        delete[] m_pBuf;
      }
    }

    m_pBuf = pNewBuf;
    m_nCapacity = nNewCapacity;

    return *this;
  }

  _CLSTRATTACH_TEMPL
  void _CLSTRATTACH_IMPL::_Resize(size_t count)
  {
    ASSERT(count >= m_nCount); // 这个内部函数主要是扩充长度的，避免缩减功能的调用
    Reserve(count);
    ASSERT(m_nCount < m_nCapacity && count < m_nCapacity);
    m_pBuf[count] = '\0';
    m_nCount = count;
  }

  _CLSTRATTACH_TEMPL
    _CLSTRATTACH_IMPL& _CLSTRATTACH_IMPL::VarFormat(const _TCh *pFmt, va_list arglist)
  {
    class StringXF : public StringFormattedT<StringAttachX> {};
    STATIC_ASSERT(sizeof(StringXF) == sizeof(StringAttachX));
    reinterpret_cast<StringXF*>(this)->StringFormattedT<StringAttachX>::VarFormat(pFmt, arglist);
    return *this;
  }

  _CLSTRATTACH_TEMPL
    _CLSTRATTACH_IMPL& _CLSTRATTACH_IMPL::Format(const _TCh *pFmt, ...)
  {
    Clear();
    va_list  arglist;
    va_start(arglist, pFmt);
    return VarFormat(pFmt, arglist);
  }

  _CLSTRATTACH_TEMPL
    _CLSTRATTACH_IMPL& _CLSTRATTACH_IMPL::AppendFormat(const _TCh *pFmt, ...)
  {
    va_list arglist;
    va_start(arglist, pFmt);
    return VarFormat(pFmt, arglist);
  }

  _CLSTRATTACH_TEMPL
    _CLSTRATTACH_IMPL& _CLSTRATTACH_IMPL::Append(_TCh c)
  {
    const size_t len = GetLength();
    _Resize(m_nCount + 1);
    m_pBuf[len] = c;
    return *this;
  }

  _CLSTRATTACH_TEMPL
    _CLSTRATTACH_IMPL& _CLSTRATTACH_IMPL::Append(_TCh c, size_t count)
  {
    size_t len = GetLength();
    _Resize(m_nCount + count);

    while((i32)count > 0) {
      m_pBuf[len++] = c;
      count--;
    }
    return *this;
  }

  _CLSTRATTACH_TEMPL
    _CLSTRATTACH_IMPL& _CLSTRATTACH_IMPL::Append(const _TCh* pStr)
  {
    const clsize uInputLen = strlenT(pStr);
    return Append(pStr, uInputLen);
  }

  _CLSTRATTACH_TEMPL
    _CLSTRATTACH_IMPL& _CLSTRATTACH_IMPL::Append(const _TCh* pStr, size_t count)
  {
    if(count) {
      const clsize uStrLen = GetLength();
      _Resize(uStrLen + count);
      strcpynT(m_pBuf + uStrLen, pStr, count);
    }
    return *this;
  }

  _CLSTRATTACH_TEMPL
    _CLSTRATTACH_IMPL& _CLSTRATTACH_IMPL::Append(const _TCh* pStr, _TCh cFill, long nWidth)
  {
    const clsize uInputLen = clstd::strlenT(pStr);

    if(nWidth == 0) {
      return Append(pStr, uInputLen);
    }
    else if(nWidth > 0) {
      nWidth -= (long)uInputLen;
      if(nWidth > 0) {
        Append(cFill, nWidth);
      }
      Append(pStr);
    }
    else { // if(nWidth < 0)
      ASSERT(nWidth < 0);
      Append(pStr);
      nWidth += (long)uInputLen;
      if(nWidth < 0) {
        Append(cFill, -nWidth);
      }
    }
    return *this;
  }

  _CLSTRATTACH_TEMPL
    int _CLSTRATTACH_IMPL::Compare(LPCSTR szStr) const
  {
    return clstd::strcmpT(m_pBuf, szStr);
  }

  _CLSTRATTACH_TEMPL
    int _CLSTRATTACH_IMPL::Compare(LPCSTR szStr, size_t count) const
  {
    return clstd::strncmpT(m_pBuf, szStr, count);
  }

  template class clstd::StringAttachX<wch>;
  template class clstd::StringAttachX<ch>;

} // namespace clstd