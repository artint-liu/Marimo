#include <stdarg.h>
#include "../clstd.h"
#include "../clString.H"
#include "SmartStream.H"
#include "../clUtility.H"

//using namespace clstd;

//SmartStreamT<char *,*>::iterator {
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

//////////////////////////////////////////////////////////////////////////
//
// 显式声明模板类
//
template class SmartStreamT<clStringA, SmartStream_TraitsA>;
template class SmartStreamT<clStringW, SmartStream_TraitsW>;

b32 SmartStream_TraitsW::_StrCmpN(const wch* pStr1, const wch* pStr2, int nCount)
{
  return wcsncmp(pStr1, pStr2, nCount) == 0;
}
b32 SmartStream_TraitsA::_StrCmpN(const ch* pStr1, const ch* pStr2, int nCount)
{
  return strncmp(pStr1, pStr2, nCount) == 0;
}


//#define CHAR_TYPE         ((TChar)m_pStream[m_uPointer] >= 0x80 ? M_LABEL : m_aCharSem[m_pStream[m_uPointer]])
//#define IS_GAP            (m_pStream[m_uPointer] >= 0 && (CHAR_TYPE & M_GAP) != 0)
//#define IS_ESCAPE         (m_pStream[m_uPointer] >= 0 && (CHAR_TYPE & M_ESCAPE) != 0)
//#define IS_LABEL          (m_pStream[m_uPointer] < 0 || (CHAR_TYPE & M_LABEL) != 0)
//#define IS_SYMBOL         (m_pStream[m_uPointer] >= 0 && (CHAR_TYPE & M_TYPE_MASK) == M_SYMBOL)
//#define IS_QUOT           (m_pStream[m_uPointer] >= 0 && (CHAR_TYPE & M_QUOT) != 0)
//#define QUOT_GROUP        (CHAR_TYPE & M_GROUP_MASK)
//#define IS_OPEN_BRAKERS   ((CHAR_TYPE & M_OPN_BRAKETS) != 0)
//#define IS_CLOSE_BRAKERS  ((CHAR_TYPE & M_CLS_BRAKETS) != 0)
//#define TRIM_HEAD_GAPS              while(IS_GAP && IsEndOfStream() == false){  m_uPointer++;  }
//#define TRIM_TAIL_GAPS    while(IS_GAP && IsHeadOfStream() == false){  m_uPointer--;  }

#define CHAR_TYPE(_POINTER)         ((TChar)(*_POINTER) >= 0x80 ? M_LABEL : m_aCharSem[*_POINTER])
#define IS_GAP(_POINTER)            ((*_POINTER) >= 0 && (CHAR_TYPE(_POINTER) & M_GAP) != 0)
#define IS_ESCAPE(_POINTER)         ((*_POINTER) >= 0 && (CHAR_TYPE(_POINTER) & M_ESCAPE) != 0)
#define IS_LABEL(_POINTER)          ((*_POINTER) <  0 || (CHAR_TYPE(_POINTER) & M_LABEL) != 0)
#define IS_SYMBOL(_POINTER)         ((*_POINTER) >= 0 && (CHAR_TYPE(_POINTER) & M_TYPE_MASK) == M_SYMBOL)
#define IS_QUOT(_POINTER)           ((*_POINTER) >= 0 && (CHAR_TYPE(_POINTER) & M_QUOT) != 0)
#define QUOT_GROUP(_POINTER)        ( CHAR_TYPE(_POINTER) & M_GROUP_MASK)
#define IS_OPEN_BRAKERS(_POINTER)   ((CHAR_TYPE(_POINTER) & M_OPN_BRAKETS) != 0)
#define IS_CLOSE_BRAKERS(_POINTER)  ((CHAR_TYPE(_POINTER) & M_CLS_BRAKETS) != 0)
#define TRIM_HEAD_GAPS(_POINTER)    while(IS_GAP(_POINTER) && ( ! IsEndOfStream(_POINTER))){  _POINTER++;  }
//#define TRIM_TAIL_GAPS    while(IS_GAP && IsHeadOfStream() == false){  m_uPointer--;  }


#define SET_SYMBOL(ch)    m_aCharSem[(int)ch] = M_SYMBOL

#define _SS_TEMPL template<class _TStr, class _Traits>
#define _SS_IMPL SmartStreamT<_TStr, _Traits>

_SS_TEMPL _SS_IMPL::SmartStreamT(T_LPCSTR pStream /* = NULL */, clsize uCountOfChar /* = NULL */)
: m_pBegin          (pStream)
, m_pEnd            (pStream + uCountOfChar)
, m_dwFlags         (0)
, m_pCallBack       (NULL)
, m_lParam          (0)
, m_pTriggerCallBack(NULL)
, m_lParamTrigger   (0)
{
  int i;
  for(i = 0; i < 128; i++)
    m_aCharSem[i] = 0;

  m_aCharSem[(int)0x20] = M_GAP;
  m_aCharSem[(int)'\t'] = M_GAP;
  m_aCharSem[(int)'\r'] = M_GAP;
  m_aCharSem[(int)'\n'] = M_GAP;

  for(i = (int)'0'; i <= (int)'9'; i++) {
    m_aCharSem[i] = M_LABEL;
  }
  for(i = (int)'a'; i <= (int)'z'; i++) {
    m_aCharSem[i] = M_LABEL;
  }
  for(i = (int)'A'; i <= (int)'Z'; i++) {
    m_aCharSem[i] = M_LABEL;
  }
  m_aCharSem[(int)'_'] = M_LABEL;

  m_aCharSem[(int)'\''] = M_QUOT | 0x0000;
  m_aCharSem[(int)'\"'] = M_QUOT | 0x1000;

  m_aCharSem[(int)'\\'] = M_ESCAPE;

  m_aCharSem[(int)'('] = M_OPN_BRAKETS | 0x0000;
  m_aCharSem[(int)')'] = M_CLS_BRAKETS | 0x0000;
  m_aCharSem[(int)'['] = M_OPN_BRAKETS | 0x1000;
  m_aCharSem[(int)']'] = M_CLS_BRAKETS | 0x1000;
  m_aCharSem[(int)'{'] = M_OPN_BRAKETS | 0x2000;
  m_aCharSem[(int)'}'] = M_CLS_BRAKETS | 0x2000;
  //if(m_pStream != NULL)
  //  TRIM_HEAD_GAPS
}

_SS_TEMPL
b32 _SS_IMPL::Initialize(T_LPCSTR pStream, clsize uCountOfChar)
{
  m_pBegin = pStream;
  m_pEnd   = pStream + uCountOfChar;

  m_itEnd.pContainer  = this;
  m_itEnd.marker      = &pStream[uCountOfChar];
  m_itEnd.length      = 0;
  return TRUE;
}

_SS_TEMPL
void _SS_IMPL::GetCharSemantic(SemType* pCharSem, clsize uStart, clsize uEnd) const
{
  if(uStart >= c_nCharTabCount || uEnd <= uStart) {
    return;
  }
  clsize uLoopEnd = clMin(uEnd, c_nCharTabCount);

  for(clsize i = uStart; i < uLoopEnd; i++)
    pCharSem[i - uStart] = m_aCharSem[i];
}

_SS_TEMPL
void _SS_IMPL::SetCharSemantic(const SemType* pCharSem, clsize uStart, clsize uEnd)
{
  if(uStart >= c_nCharTabCount || uEnd <= uStart) {
    return;
  }

  clsize uLoopEnd = clMin(uEnd, c_nCharTabCount);
  for(clsize i = uStart; i < uLoopEnd; i++)
    m_aCharSem[i] = pCharSem[i - uStart];
}

_SS_TEMPL
typename _SS_IMPL::SemType _SS_IMPL::GetCharSemantic( TChar ch ) const
{
  if(ch >= c_nCharTabCount) {
    return 0;
  }
  return m_aCharSem[ch];
}

_SS_TEMPL
typename _SS_IMPL::SemType _SS_IMPL::SetCharSemantic(TChar ch, SemType flagCharSem)
{
  if(ch >= c_nCharTabCount) {
    return 0;
  }
  SemType ePrev = m_aCharSem[ch];
  m_aCharSem[ch] = flagCharSem;
  return ePrev;
}

_SS_TEMPL
b32 _SS_IMPL::IsHeadOfStream(T_LPCSTR pPointer) const
{
  return pPointer == m_pBegin;
}

_SS_TEMPL
b32 _SS_IMPL::IsEndOfStream(T_LPCSTR pPointer) const
{
  return pPointer >= m_pEnd;
}

_SS_TEMPL
typename _SS_IMPL::T_LPCSTR _SS_IMPL::GetStreamPtr() const
{
  return m_pBegin;
}

_SS_TEMPL
clsize _SS_IMPL::GetStreamCount() const
{
  return m_pEnd - m_pBegin;
}

_SS_TEMPL
b32 _SS_IMPL::Get(T_LPCSTR pPointer, T_LPCSTR* ppOutPointer, clsize* pOutCount) const
{
  if(pPointer < m_pBegin || IsEndOfStream(pPointer)) {
    return false;
  }

  if(ppOutPointer != NULL) {
    *ppOutPointer = pPointer;
      //&m_pFirst[m_uPointer];
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
    const u32 grp = QUOT_GROUP(pPointer);
    //const clsize uBegin = m_uPointer;  // 不记录这个用指针相减也可以,但要除以CHAR的大小
    T_LPCSTR pInQuot = pPointer + 1;
    //m_uPointer++;
    while( ! IsEndOfStream(pInQuot))
    {
      if(IS_ESCAPE(pInQuot))
      {
        pInQuot += 2;
        continue;
      }
      else if(IS_QUOT(pInQuot) && QUOT_GROUP(pInQuot) == grp)
      {
        if(pOutCount != NULL) {
          *pOutCount = pInQuot + 1 - pPointer;
            //(m_uPointer + 1) - uBegin;
        }
        //m_uPointer = uBegin;  // 还原
        return true;
      }
      //m_uPointer++;
      pInQuot++;
    }
  }
  else if(IS_LABEL(pPointer) || IS_SYMBOL(pPointer))
  {
    //const clsize uBegin = m_uPointer;
    T_LPCSTR pBegin = pPointer;
    const SemType t = (CHAR_TYPE(pPointer) & M_TYPE_MASK);
    pPointer++;
    if(t != M_SYMBOL || TEST_FLAG_NOT(m_dwFlags, F_SYMBOLBREAK))
    {
      while((CHAR_TYPE(pPointer) & M_TYPE_MASK) == t && ( ! IsEndOfStream(pPointer)))
      {
        pPointer++;
      }
    }
    if(pOutCount != NULL) {
      *pOutCount = pPointer - pBegin;
    }
    //m_uPointer = uBegin;  // 还原指针
    return true;
  }
  else
    ASSERT(0);
  return false;
}

_SS_TEMPL
u32 _SS_IMPL::SetFlags(u32 dwFlags)
{
  u32 dwOldFlags = m_dwFlags;
  m_dwFlags = dwFlags;
  return dwOldFlags;
}

_SS_TEMPL
typename _SS_IMPL::IteratorProc 
_SS_IMPL::SetIteratorCallBack(typename _SS_IMPL::IteratorProc pNew, u32_ptr lParam)
{
  IteratorProc pOldCallBack = m_pCallBack;
  m_pCallBack = pNew;
  m_lParam  = lParam;
  return pOldCallBack;
}

_SS_TEMPL
typename _SS_IMPL::IteratorProc 
_SS_IMPL::SetTriggerCallBack(typename _SS_IMPL::IteratorProc pTrigger, u32_ptr lParam)
{
  IteratorProc pOldCallBack = m_pTriggerCallBack;
  m_pTriggerCallBack = pTrigger;
  m_lParamTrigger  = lParam;
  return pOldCallBack;
}

_SS_TEMPL
u32 _SS_IMPL::GetFlags() const
{
  return m_dwFlags;
}

_SS_TEMPL
typename _SS_IMPL::iterator _SS_IMPL::nearest(clsize nOffset) const
{
  iterator it(this);
  //m_uPointer = nOffset;
  T_LPCSTR pPointer = m_pBegin + nOffset;
  TRIM_HEAD_GAPS(pPointer);
  Get(pPointer, &it.marker, &it.length);
  return it;
}

_SS_TEMPL
typename _SS_IMPL::iterator _SS_IMPL::begin() const
{
  iterator itBegin(this);
  return next(itBegin); // 这里这么写是为了保证begin也能触发特殊符号回调函数
}

#if defined(_WINDOWS)
_SS_TEMPL 
typename const _SS_IMPL::iterator& _SS_IMPL::end() const
#elif defined(_IOS) || defined(_ANDROID)
_SS_TEMPL 
  typename _SS_IMPL::const_iterator& _SS_IMPL::end() const
#endif // #if defined(_WINDOWS)
{
  return m_itEnd;
}

_SS_TEMPL 
typename _SS_IMPL::iterator& _SS_IMPL::next(iterator& it) const
{
  //m_uPointer = (u32)(it.marker - m_pFirst) + it.length;
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

  if(m_pTriggerCallBack != NULL && TEST_FLAG(m_aCharSem[it.marker[0]], M_CALLBACK)) {
    m_pTriggerCallBack(it, remain, m_lParamTrigger);
  }
  return it;
}

_SS_TEMPL
typename _SS_IMPL::iterator& _SS_IMPL::iterator::operator++()
{
  return pContainer->next(*this);
}

_SS_TEMPL
clsize _SS_IMPL::iterator::offset() const
{
  return (u32)(marker - pContainer->m_pBegin);
}

_SS_TEMPL
_TStr _SS_IMPL::iterator::ToString() const
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

_SS_TEMPL
b32 _SS_IMPL::iterator::BeginsWith(T_LPCSTR str) const
{
  const size_t uCmpLength = clstd::strlenT(str);
  if(length < uCmpLength) {
    return FALSE;
  }
  return clstd::strncmpT(marker, str, uCmpLength) == 0;
}

_SS_TEMPL
b32 _SS_IMPL::iterator::EndsWith(T_LPCSTR str) const
{
  //const size_t uStrLength = CLSTR_LENGTH(m_pBuf);
  const size_t uCmpLength = clstd::strlenT(str);
  if(length < uCmpLength) {
    return FALSE;
  }
  return clstd::strncmpT(marker + length - uCmpLength, str, uCmpLength) == 0;
}

_SS_TEMPL
b32 _SS_IMPL::iterator::BeginsWith(T_LPCSTR str, clsize count) const
{
  return (length < count) ? FALSE
    : (clstd::strncmpT(marker, str, count) == 0);
}

_SS_TEMPL
b32 _SS_IMPL::iterator::EndsWith(T_LPCSTR str, clsize count) const
{
  return (length < count) ? FALSE 
    : (clstd::strncmpT(marker + length - count, str, count) == 0);
}

_SS_TEMPL
b32 _SS_IMPL::iterator::operator==(const _TStr& str) const
{
  return (str.GetLength() == length && 
    _Traits::_StrCmpN(marker, str, (int)length));
}

_SS_TEMPL
b32 _SS_IMPL::iterator::operator==(T_LPCSTR pStr) const
{
  return (_TStr(pStr).GetLength() == length &&
    _Traits::_StrCmpN(marker, pStr, (int)length));
}

_SS_TEMPL
b32 _SS_IMPL::iterator::operator==(TChar ch) const
{
  return (length == 1 && marker[0] == ch);
}

_SS_TEMPL
b32 _SS_IMPL::iterator::operator!=(const _TStr& str) const
{
  return (str.GetLength() != length ||
    ! _Traits::_StrCmpN(marker, str, (int)length));
}

_SS_TEMPL
b32 _SS_IMPL::iterator::operator!=(T_LPCSTR pStr) const
{
  return (_TStr(pStr).GetLength() != length ||
    ! _Traits::_StrCmpN(marker, pStr, (int)length));
}

_SS_TEMPL
b32 _SS_IMPL::iterator::operator!=(TChar ch) const
{
  return (length != 1 || marker[0] != ch);
}

_SS_TEMPL
b32 _SS_IMPL::iterator::operator==(const iterator& it) const
{
  ASSERT(pContainer == it.pContainer);
  return (marker == it.marker && length == it.length);
}

_SS_TEMPL
b32 _SS_IMPL::iterator::operator!=(const iterator& it) const
{
  return (!operator==(it));
}

_SS_TEMPL
  b32 _SS_IMPL::iterator::operator>=(const iterator& it) const
{
  ASSERT(pContainer == it.pContainer);
  return (marker > it.marker) || 
    (marker == it.marker && length == it.length);
}

_SS_TEMPL
  b32 _SS_IMPL::iterator::operator<=(const iterator& it) const
{
  ASSERT(pContainer == it.pContainer);
  return (marker < it.marker) || 
    (marker == it.marker && length == it.length);
}

_SS_TEMPL
  b32 _SS_IMPL::iterator::operator>(const iterator& it) const
{
  ASSERT(pContainer == it.pContainer);
  return (marker > it.marker);
}

_SS_TEMPL
  b32 _SS_IMPL::iterator::operator<(const iterator& it) const
{
  ASSERT(pContainer == it.pContainer);
  return (marker < it.marker);
}

_SS_TEMPL
typename _SS_IMPL::iterator& _SS_IMPL::iterator::operator=(const iterator& it)
{
  pContainer = it.pContainer;
  marker = it.marker;
  length = it.length;
  return *this;
}

_SS_TEMPL
typename _SS_IMPL::iterator  _SS_IMPL::iterator::operator+(const size_t n) const
{  
  ASSERT(((int)n) > 0);
  iterator it = *this;
  for(size_t i = 0; i < n; i++)
    ++it;
  return it;
}

#if defined(_WINDOWS)
_SS_TEMPL
typename const _SS_IMPL::iterator _SS_IMPL::find(const iterator& itBegin, int nCount, ...) const
#elif defined(_IOS)
typename _SS_IMPL::const_iterator _SS_IMPL::find(const iterator& itBegin, int nCount, ...) const
#elif defined(_ANDROID)
_SS_TEMPL
typename _SS_IMPL::const_iterator _SS_IMPL::find(const iterator& itBegin, int nCount, ...) const
#else
#error new platform
#endif // #if defined(_WINDOWS)
{
  iterator it = itBegin;
  va_list arglist;

  while(it != end())
  {
    va_start(arglist, nCount);
    for(int i = 0; i < nCount; i++)
    {
      if(it == *(T_LPCSTR*)arglist)
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

_SS_TEMPL
b32 _SS_IMPL::find_pair(const iterator& itCurrent, iterator& itOpen, iterator& itClose, T_LPCSTR chOpen, T_LPCSTR chClose) const
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

namespace SmartStreamUtility
{
  //b32 FindPair(const SmartStreamA::iterator& itCurrent, SmartStreamA::iterator& itOpen, SmartStreamA::iterator& itClose, ch chOpen, ch chClose)
  //int ParseArray(SmartStreamA::iterator itBegin, SmartStreamA::iterator itEnd, CLLPCSTR Separators, CLLPCSTR Pairs, clStringArrayA& aStrings)
  //{
  //  return 0;
  //}
} // namespace SmartStreamUtility