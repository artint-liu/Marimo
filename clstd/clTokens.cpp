#include <stdarg.h>
#include "clstd.h"
#include "clString.h"
#include "clTokens.h"
#include "clUtility.h"

//using namespace clstd;

//TokensT<char *,*>::iterator {
//  preview  (#(#("[",$e.length,"]"),[$e.marker, s]))
//
//  children (
//    #(
//      #([size] : $e.length),
//      #array(
//        expr: $e.marker[$i],
//        size: $e.length
//      )
//    )
//  )
//}

namespace clstd
{

//////////////////////////////////////////////////////////////////////////
//
// 显式声明模板类
//
  //template class TokensT<clStringA>;
  //template class TokensT<clStringW>;

  namespace TokensUtility {
    template b32 IsHeadOfLine<clStringA>(const TokensT<clStringA>* pToken, clStringA::LPCSTR pChar);
    template b32 IsHeadOfLine<clStringW>(const TokensT<clStringW>* pToken, clStringW::LPCSTR pChar);
#if 0
    template clsize ExtendCStyleNumeric<ch>(const ch* str, clsize max_len);
    template clsize ExtendCStyleNumeric<wch>(const wch* str, clsize max_len);
#endif
  } // namespace TokensUtility

#define IS_GAP(_POINTER)            ((*_POINTER) == 0x20 || (*_POINTER) == '\t' || (*_POINTER) == '\r' || (*_POINTER) == '\n')
#define IS_ESCAPE(_POINTER)         ((*_POINTER) == '\\')
#define IS_LABEL(_POINTER)          ( (*_POINTER) == '_' || \
                                    ((*_POINTER) >= '0' && (*_POINTER) <= '9') || \
                                    ((*_POINTER) >= 'a' && (*_POINTER) <= 'z') || \
                                    ((*_POINTER) >= 'A' && (*_POINTER) <= 'Z') )

#define IS_QUOT(_POINTER)           ((*_POINTER) == '\'' || (*_POINTER) == '\"')
#define QUOT_GROUP(_POINTER)        ( CHAR_TYPE(_POINTER) & M_GROUP_MASK)
#define IS_OPEN_BRAKERS(_POINTER)   ((*_POINTER) == '(' || (*_POINTER) == '[' || (*_POINTER) == '{')
#define IS_CLOSE_BRAKERS(_POINTER)  ((*_POINTER) == ')' || (*_POINTER) == ']' || (*_POINTER) == '}')
#define TRIM_HEAD_GAPS(_POINTER)    while(IS_GAP(_POINTER) && (_POINTER < m_pEnd)) {  _POINTER++;  }


#define SET_SYMBOL(ch)    m_aCharSem[(int)ch] = M_SYMBOL

#define _TOEKN_TEMPL template<class _TStr>
#define _TOKEN_IMPL  TokensT<_TStr>

  _TOEKN_TEMPL _TOKEN_IMPL::TokensT(T_LPCSTR pStream /* = NULL */, clsize uCountOfChar /* = NULL */)
    : m_pBegin          (pStream)
    , m_pEnd            (pStream + uCountOfChar)
    , m_dwFlags         (0)
    , m_pCallBack       (NULL)
    , m_lParam          (0)
    , m_lParamTrigger   (0)
  {
  }

  _TOEKN_TEMPL
    b32 _TOKEN_IMPL::Attach(T_LPCSTR pStream, clsize uCountOfChar)
  {
    m_pBegin = pStream;
    m_pEnd   = pStream + uCountOfChar;

    m_itEnd.pContainer  = this;
    m_itEnd.marker      = &pStream[uCountOfChar];
    m_itEnd.length      = 0;
    return TRUE;
  }

  _TOEKN_TEMPL
    b32 _TOKEN_IMPL::IsHeadOfStream(T_LPCSTR pPointer) const
  {
    return pPointer == m_pBegin;
  }

  _TOEKN_TEMPL
    b32 _TOKEN_IMPL::IsEndOfStream(T_LPCSTR pPointer) const
  {
    return pPointer >= m_pEnd;
  }

  _TOEKN_TEMPL
    typename _TOKEN_IMPL::T_LPCSTR _TOKEN_IMPL::GetStreamPtr() const
  {
    return m_pBegin;
  }

  _TOEKN_TEMPL
    clsize _TOKEN_IMPL::GetStreamCount() const
  {
    return m_pEnd - m_pBegin;
  }

  _TOEKN_TEMPL
    b32 _TOKEN_IMPL::Get(T_LPCSTR pPointer, T_LPCSTR* ppOutPointer, clsize* pOutCount) const
  {
    if(pPointer < m_pBegin || IsEndOfStream(pPointer)) {
      return false;
    }

    if(ppOutPointer != NULL) {
      *ppOutPointer = pPointer;
    }

    // 开括号闭括号的话，直接返回
    if(IS_OPEN_BRAKERS(pPointer) || IS_CLOSE_BRAKERS(pPointer))
    {
      if(pOutCount != NULL) {
        *pOutCount = 1;
      }
      return true;
    }
    else if(IS_QUOT(pPointer))
    {
      const TChar c = *pPointer;
      T_LPCSTR pInQuot = pPointer + 1;

      while( ! IsEndOfStream(pInQuot))
      {
        if(IS_ESCAPE(pInQuot))
        {
          pInQuot += 2;
          continue;
        }
        else if(IS_QUOT(pInQuot) && (*pInQuot) == c)
        {
          if(pOutCount != NULL) {
            *pOutCount = pInQuot + 1 - pPointer;
          }
          return true;
        }
        pInQuot++;
      }
    }
    else if(IS_LABEL(pPointer))
    {
      T_LPCSTR pBegin = pPointer;
      pPointer++;
      while(( ! IsEndOfStream(pPointer)) && IS_LABEL(pPointer)) {
        pPointer++;
      }

      if(pOutCount != NULL) {
        *pOutCount = pPointer - pBegin;
      }
      return true;
    }
    else // IS_SYMBOL(pPointer))
    {
      //const clsize uBegin = m_uPointer;
      T_LPCSTR pBegin = pPointer;
      pPointer++;
      if(pOutCount != NULL) {
        *pOutCount = pPointer - pBegin;
      }
      return true;
    }
    return false;
  }

  _TOEKN_TEMPL
    u32 _TOKEN_IMPL::SetFlags(u32 dwFlags)
  {
    u32 dwOldFlags = m_dwFlags;
    m_dwFlags = dwFlags;
    return dwOldFlags;
  }

  _TOEKN_TEMPL
    typename _TOKEN_IMPL::IteratorProc 
    _TOKEN_IMPL::SetIteratorCallBack(typename _TOKEN_IMPL::IteratorProc pNew, u32_ptr lParam)
  {
    IteratorProc pOldCallBack = m_pCallBack;
    m_pCallBack = pNew;
    m_lParam  = lParam;
    return pOldCallBack;
  }

  _TOEKN_TEMPL
    u32 _TOKEN_IMPL::GetFlags() const
  {
    return m_dwFlags;
  }

  _TOEKN_TEMPL
    typename _TOKEN_IMPL::iterator _TOKEN_IMPL::nearest(clsize nOffset) const
  {
    iterator it(this);
    //m_uPointer = nOffset;
    T_LPCSTR pPointer = m_pBegin + nOffset;
    TRIM_HEAD_GAPS(pPointer);
    Get(pPointer, &it.marker, &it.length);
    return it;
  }

  _TOEKN_TEMPL
    typename _TOKEN_IMPL::iterator _TOKEN_IMPL::begin() const
  {
    iterator itBegin(this);
    return next(itBegin); // 这里这么写是为了保证begin也能触发特殊符号回调函数
  }

#if defined(__clang__) || defined(__GNUC__)
  _TOEKN_TEMPL 
    typename _TOKEN_IMPL::const_iterator& _TOKEN_IMPL::end() const
#else
  _TOEKN_TEMPL 
    typename const _TOKEN_IMPL::iterator& _TOKEN_IMPL::end() const
#endif
  {
    return m_itEnd;
  }

  _TOEKN_TEMPL 
    typename _TOKEN_IMPL::iterator& _TOKEN_IMPL::next(iterator& it) const
  {
    T_LPCSTR pPointer = it.marker + it.length;
    TRIM_HEAD_GAPS(pPointer);
    if( ! Get(pPointer, &it.marker, &it.length))
    {    
      it = end();
      return it;
    }

    const clsize remain = m_pEnd - pPointer;

    if(m_pCallBack != NULL) {
      m_pCallBack(it, remain, m_lParam);
    }
    return it;
  }

  _TOEKN_TEMPL
    typename _TOKEN_IMPL::iterator& _TOKEN_IMPL::iterator::operator++()
  {
    return pContainer->next(*this);
  }

  _TOEKN_TEMPL
    typename _TOKEN_IMPL::iterator& _TOKEN_IMPL::iterator::operator++(int)
  {
    return pContainer->next(*this);
  }

  _TOEKN_TEMPL
    clsize _TOKEN_IMPL::iterator::offset() const
  {
    return (u32)(marker - pContainer->m_pBegin);
  }

  _TOEKN_TEMPL
    _TStr _TOKEN_IMPL::iterator::ToRawString() const
  {
    _TStr strTemp;
    if(length == 0) {
      return strTemp;
    }
    return strTemp.Append(marker, length);
  }

  _TOEKN_TEMPL
    _TStr& _TOKEN_IMPL::iterator::ToRawString(_TStr& str) const
  {
    if(length == 0) {
      str.Clear();
      return str;
    }

    return str.Append(marker, length);
  }

  _TOEKN_TEMPL
    _TStr _TOKEN_IMPL::iterator::ToString() const
  {
    _TStr strTemp;
    if(length == 0) {
      return strTemp;
    }
    u32 i = (marker[0] != '\"' && marker[0] != '\'') ? 0 : 1;
    u32 n = 0;
    TChar* pTemp = strTemp.GetBuffer(length + 1);

    for(; i < length - 1; i++) {
      pTemp[n++] = marker[i];
    }

    if(marker[length - 1] != '\"' && marker[length - 1] != '\'') {
      pTemp[n++] = marker[i];
    }

    pTemp[n] = '\0';
    strTemp.ReleaseBuffer();

    return strTemp;
  }

  _TOEKN_TEMPL
    _TStr& _TOKEN_IMPL::iterator::ToString(_TStr& str) const
  {
    if(length == 0) {
      str.Clear();
      return str;
    }
    u32 i = (marker[0] != '\"' && marker[0] != '\'') ? 0 : 1;
    u32 n = 0;
    TChar* pTemp = str.GetBuffer(length + 1);

    for(; i < length - 1; i++) {
      pTemp[n++] = marker[i];
    }

    if(marker[length - 1] != '\"' && marker[length - 1] != '\'') {
      pTemp[n++] = marker[i];
    }

    pTemp[n] = '\0';
    str.ReleaseBuffer();

    return str;
  }

  _TOEKN_TEMPL
    b32 _TOKEN_IMPL::iterator::IsEqual(T_LPCSTR str, clsize count) const
  {
    return (count == length && ! clstd::strncmpT(marker, str, (int)length));
  }
  
  _TOEKN_TEMPL
    b32 _TOKEN_IMPL::iterator::BeginsWith(TChar ch) const
  {
    return (length >= 1) && (marker[0] == ch);
  }

  _TOEKN_TEMPL
    b32 _TOKEN_IMPL::iterator::EndsWith(TChar ch) const
  {
    return (length >= 1) && (marker[length - 1] == ch);
  }

  _TOEKN_TEMPL
    b32 _TOKEN_IMPL::iterator::BeginsWith(T_LPCSTR str) const
  {
    const size_t uCmpLength = clstd::strlenT(str);
    if(length < uCmpLength) {
      return FALSE;
    }
    return clstd::strncmpT(marker, str, uCmpLength) == 0;
  }

  _TOEKN_TEMPL
    b32 _TOKEN_IMPL::iterator::EndsWith(T_LPCSTR str) const
  {
    const size_t uCmpLength = clstd::strlenT(str);
    if(length < uCmpLength) {
      return FALSE;
    }
    return clstd::strncmpT(marker + length - uCmpLength, str, uCmpLength) == 0;
  }

  _TOEKN_TEMPL
    b32 _TOKEN_IMPL::iterator::BeginsWith(T_LPCSTR str, clsize count) const
  {
    return (length < count) ? FALSE
      : (clstd::strncmpT(marker, str, count) == 0);
  }

  _TOEKN_TEMPL
    b32 _TOKEN_IMPL::iterator::EndsWith(T_LPCSTR str, clsize count) const
  {
    return (length < count) ? FALSE 
      : (clstd::strncmpT(marker + length - count, str, count) == 0);
  }

  _TOEKN_TEMPL
    b32 _TOKEN_IMPL::iterator::operator==(const _TStr& str) const
  {
    return (str.GetLength() == length && ! clstd::strncmpT(marker, (T_LPCSTR)str, (int)length));
  }

  _TOEKN_TEMPL
    b32 _TOKEN_IMPL::iterator::operator==(T_LPCSTR pStr) const
  {
    const size_t nStrLength = clstd::strlenT(pStr);
    return (nStrLength == length && ! clstd::strncmpT(marker, pStr, (int)length));
  }

  _TOEKN_TEMPL
    b32 _TOKEN_IMPL::iterator::operator==(TChar ch) const
  {
    return (length == 1 && marker[0] == ch);
  }

  _TOEKN_TEMPL
    b32 _TOKEN_IMPL::iterator::operator!=(const _TStr& str) const
  {
    return (str.GetLength() != length || clstd::strncmpT(marker, (T_LPCSTR)str, (int)length));
  }

  _TOEKN_TEMPL
    b32 _TOKEN_IMPL::iterator::operator!=(T_LPCSTR pStr) const
  {
    return (strlenT(pStr) != length || clstd::strncmpT(marker, pStr, (int)length));
  }

  _TOEKN_TEMPL
    b32 _TOKEN_IMPL::iterator::operator!=(TChar ch) const
  {
    return (length != 1 || marker[0] != ch);
  }

  _TOEKN_TEMPL
    b32 _TOKEN_IMPL::iterator::operator==(const iterator& it) const
  {
    ASSERT(pContainer == it.pContainer);
    return (marker == it.marker && length == it.length);
  }

  _TOEKN_TEMPL
    b32 _TOKEN_IMPL::iterator::operator!=(const iterator& it) const
  {
    return (!operator==(it));
  }

  _TOEKN_TEMPL
    b32 _TOKEN_IMPL::iterator::operator>=(const iterator& it) const
  {
    ASSERT(pContainer == it.pContainer);
    return (marker > it.marker) || 
      (marker == it.marker && length == it.length);
  }

  _TOEKN_TEMPL
    b32 _TOKEN_IMPL::iterator::operator<=(const iterator& it) const
  {
    ASSERT(pContainer == it.pContainer);
    return (marker < it.marker) || 
      (marker == it.marker && length == it.length);
  }

  _TOEKN_TEMPL
    b32 _TOKEN_IMPL::iterator::operator>(const iterator& it) const
  {
    ASSERT(pContainer == it.pContainer);
    return (marker > it.marker);
  }

  _TOEKN_TEMPL
    b32 _TOKEN_IMPL::iterator::operator<(const iterator& it) const
  {
    ASSERT(pContainer == it.pContainer);
    return (marker < it.marker);
  }

  _TOEKN_TEMPL
    typename _TOKEN_IMPL::iterator& _TOKEN_IMPL::iterator::operator=(const iterator& it)
  {
    pContainer = it.pContainer;
    marker = it.marker;
    length = it.length;
    return *this;
  }

  _TOEKN_TEMPL
    typename _TOKEN_IMPL::iterator  _TOKEN_IMPL::iterator::operator+(const size_t n) const
  {  
    ASSERT(((int)n) > 0);
    iterator it = *this;
    for(size_t i = 0; i < n; i++)
      ++it;
    return it;
  }


#if defined(__clang__) || defined(__GNUC__)
  _TOEKN_TEMPL
    typename _TOKEN_IMPL::const_iterator _TOKEN_IMPL::find(const iterator& itBegin, int nCount, ...) const
#else
  _TOEKN_TEMPL
    typename const _TOKEN_IMPL::iterator _TOKEN_IMPL::find(const iterator& itBegin, int nCount, ...) const
#endif
  {
    iterator it = itBegin;
    va_list arglist;

    while(it != end())
    {
      va_start(arglist, nCount);
      for(int i = 0; i < nCount; i++)
      {
        if(it == va_arg(arglist, T_LPCSTR)) // *(T_LPCSTR*)arglist)
        {
          va_end(arglist);
          return it;
        }
        va_arg(arglist, char*);
      }
      ++it;
    }
    va_end(arglist);
    return m_itEnd;
  }

  _TOEKN_TEMPL
    b32 _TOKEN_IMPL::find_pair(const iterator& itCurrent, iterator& itOpen, iterator& itClose, T_LPCSTR chOpen, T_LPCSTR chClose) const
  {
    iterator it = itCurrent;
    itOpen = itCurrent;
    int nDepth = 0;
    while(it != end())
    {
      if(it == chOpen)
      {
        if(nDepth == 0)
          itOpen = it;
        nDepth++;
      }
      else if(it == chClose)
      {
        nDepth--;
        if(nDepth <= 0)
        {
          itClose = it;
          return(nDepth == 0);
        }
      }
      ++it;
    }
    return false;
  }

  template class TokensT<clStringA>;
  template class TokensT<clStringW>;

  //////////////////////////////////////////////////////////////////////////

  namespace TokensUtility
  {
    template<class _TStr>
    b32 IsHeadOfLine(const TokensT<_TStr>* pToken, typename _TStr::LPCSTR pChar)
    {
      auto* ptr = pToken->GetStreamPtr();
      auto* p = pChar - 1;
      while(p >= ptr) {
        if(*p == '\n') {
          return TRUE;
        }
        else if(*p == '/' && (p - 1) >= ptr && *(p - 1) == '*') // C代码特有
        {
          p -= 2;
          while(p >= ptr) {
            if(*p == '*' && (p - 1) >= ptr && *(p - 1) == '/') {
              p -= 2;
              break;
            }
            --p;
          }
          continue;
        }
        else if( ! IS_GAP(p)) {

          return FALSE;
        }
        --p;
      }
      return TRUE; // 到文档开头了
    }

#if 0
    template<typename _TCh>
    clsize ExtendCStyleNumeric(const _TCh* str, clsize max_len)
    {
      if(str == NULL || max_len == 0 ||
        _CL_NOT_(isdigit(str[0])) && str[0] != '.')
      {
        return 0;
      }
      b32 bDot = str[0] == '.';

      //bENotation;

      for(clsize i = 1; i < max_len; i++)
      {
      }

      return bDot ? 0 : 1;
    }
#endif // 0

  } // namespace TokenUtility

} // namespace clstd
