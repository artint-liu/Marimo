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


#define CHAR_TYPE         ((TChar)m_pStream[m_uPointer] >= 0x80 ? M_LABEL : m_aCharSem[m_pStream[m_uPointer]])
#define IS_GAP            (m_pStream[m_uPointer] >= 0 && (CHAR_TYPE & M_GAP) != 0)
#define IS_ESCAPE         (m_pStream[m_uPointer] >= 0 && (CHAR_TYPE & M_ESCAPE) != 0)
#define IS_LABEL          (m_pStream[m_uPointer] < 0 || (CHAR_TYPE & M_LABEL) != 0)
#define IS_SYMBOL         (m_pStream[m_uPointer] >= 0 && (CHAR_TYPE & M_TYPE_MASK) == M_SYMBOL)
#define IS_QUOT           (m_pStream[m_uPointer] >= 0 && (CHAR_TYPE & M_QUOT) != 0)
#define QUOT_GROUP        (CHAR_TYPE & M_GROUP_MASK)
#define IS_OPEN_BRAKERS   ((CHAR_TYPE & M_OPN_BRAKETS) != 0)
#define IS_CLOSE_BRAKERS  ((CHAR_TYPE & M_CLS_BRAKETS) != 0)
#define TRIM_HEAD_GAPS    while(IS_GAP && IsEndOfStream() == false){  m_uPointer++;  }
#define TRIM_TAIL_GAPS    while(IS_GAP && IsHeadOfStream() == false){  m_uPointer--;  }

#define SET_SYMBOL(ch)    m_aCharSem[(int)ch] = M_SYMBOL

#define _SS_TEMPL template<class _TStr, class _Traits>
#define _SS_IMPL SmartStreamT<_TStr, _Traits>

_SS_TEMPL _SS_IMPL::SmartStreamT(T_LPCSTR pStream /* = NULL */, clsize uCountOfChar /* = NULL */)
: m_pStream         (pStream)
, m_uCountOfChar    (uCountOfChar)
, m_uPointer        (0)
, m_dwFlags         (NULL)
, m_pCallBack       (NULL)
, m_lParam          (NULL)
, m_pTriggerCallBack(NULL)
, m_lParamTrigger   (NULL)
{
  int i;
  for(i = 0; i < 128; i++)
    m_aCharSem[i] = 0;

  m_aCharSem[(int)0x20] = M_GAP;
  m_aCharSem[(int)'\t'] = M_GAP;
  m_aCharSem[(int)'\r'] = M_GAP;
  m_aCharSem[(int)'\n'] = M_GAP;

  for(i = (int)'0'; i <= (int)'9'; i++)
    m_aCharSem[i] = M_LABEL;
  for(i = (int)'a'; i <= (int)'z'; i++)
    m_aCharSem[i] = M_LABEL;
  for(i = (int)'A'; i <= (int)'Z'; i++)
    m_aCharSem[i] = M_LABEL;
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
  if(m_pStream != NULL)
    TRIM_HEAD_GAPS
}

_SS_TEMPL
b32 _SS_IMPL::Initialize(T_LPCSTR pStream, clsize uCountOfChar)
{
  m_pStream       = pStream;
  m_uCountOfChar  = uCountOfChar;
  m_uPointer      = 0;

  m_itEnd.pContainer  = this;
  m_itEnd.marker      = &pStream[uCountOfChar];
  m_itEnd.length      = 0;
  //m_itEnd.remain    = 0;
  TRIM_HEAD_GAPS
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
SemType _SS_IMPL::GetCharSemantic( TChar ch ) const
{
  return m_aCharSem[ch];
}

_SS_TEMPL
b32 _SS_IMPL::IsHeadOfStream() const
{
  return m_uPointer == 0;
}

_SS_TEMPL
b32 _SS_IMPL::IsEndOfStream() const
{
  return m_uPointer >= m_uCountOfChar;
}

_SS_TEMPL
void _SS_IMPL::Reset()
{
  m_uPointer = 0;
}

_SS_TEMPL
void _SS_IMPL::EndOfStream()
{
  m_uPointer = m_uCountOfChar - 1;
}

_SS_TEMPL
clsize _SS_IMPL::GetPointer() const
{
  return m_uPointer;
}

_SS_TEMPL
typename _SS_IMPL::T_LPCSTR _SS_IMPL::GetStreamPtr() const
{
  return m_pStream;
}

_SS_TEMPL
clsize _SS_IMPL::GetStreamCount() const
{
  return m_uCountOfChar;
}
//u32 _SS_IMPL::SetPointer(u32 uPointer)
//{
//  u32 uOldPointer = m_uPointer;
//  m_uPointer = uPointer;
//  return uOldPointer;
//}

_SS_TEMPL
b32 _SS_IMPL::Get(T_LPCSTR* ppStream, clsize* pCount)
{
  if(IsEndOfStream() == TRUE)
    return false;
  if(ppStream != NULL)
    *ppStream = &m_pStream[m_uPointer];
  if(IS_OPEN_BRAKERS || IS_CLOSE_BRAKERS)
  {
    if(pCount != NULL)
      *pCount = 1;
    return true;
  }
  else if(IS_QUOT)
  {
    const u32 grp = QUOT_GROUP;
    const clsize uBegin = m_uPointer;  // 不记录这个用指针相减也可以,但要除以CHAR的大小
    m_uPointer++;
    while (IsEndOfStream() == false)
    {
      if(IS_ESCAPE)
      {
        m_uPointer += 2;
        continue;
      }
      else if(IS_QUOT && QUOT_GROUP == grp)
      {
        if(pCount != NULL)
          *pCount = (m_uPointer + 1) - uBegin;
        m_uPointer = uBegin;  // 还原
        return true;
      }
      m_uPointer++;
    }
  }
  else if(IS_LABEL || IS_SYMBOL)
  {
    const clsize uBegin = m_uPointer;
    SemType t = (CHAR_TYPE & M_TYPE_MASK);
    m_uPointer++;
    if(t != M_SYMBOL || TEST_FLAG(m_dwFlags, F_SYMBOLBREAK) == FALSE)
    {
      while((CHAR_TYPE & M_TYPE_MASK) == t && IsEndOfStream() == false)
      {
        m_uPointer++;
      }
    }
    if(pCount != NULL)
      *pCount = m_uPointer - uBegin;
    m_uPointer = uBegin;  // 还原指针
    return true;
  }
  else
    ASSERT(0);
  return false;
}

//bool _SS_IMPL::GetPrevRef(T_LPCSTR pStream, u32* pCount)
//{
//
//}
//
//bool _SS_IMPL::GetNextRef(T_LPCSTR pStream, u32* pCount)
//{
//  
//}

_SS_TEMPL
b32 _SS_IMPL::ReadPrev(T_LPCSTR* ppStream, clsize* pCount)
{
  if(IsHeadOfStream())
    return false;
  
  m_uPointer--;
  
  if(IsHeadOfStream())
  {
    TRIM_HEAD_GAPS;
    if(pCount != NULL)
      *pCount = 1;
    if(ppStream != NULL)
      *ppStream = &m_pStream[m_uPointer];
    return false;
  }

  TRIM_TAIL_GAPS;
  const clsize rbegin = m_uPointer;
  //u32 prev = m_uPointer;;

  if(IS_OPEN_BRAKERS || IS_CLOSE_BRAKERS)
  {
    if(pCount != NULL)
      *pCount = 1;

    goto TRUE_RET;
  }
  else if(IS_QUOT)
  {
    const u32 grp = QUOT_GROUP;
    m_uPointer--;
    while (IsHeadOfStream() == false)
    {
      if(IS_QUOT && QUOT_GROUP == grp)
      {
        if(m_uPointer > 0 && (m_aCharSem[m_pStream[m_uPointer - 1]] & M_ESCAPE) != 0)
        {
          m_uPointer -= 2;
          continue;
        }
        break;
      }
      m_uPointer--;
    }
    if(pCount != NULL)
      *pCount = rbegin - m_uPointer + 1;
    goto TRUE_RET;
  }
  else if(IS_LABEL || IS_SYMBOL)
  {
    SemType t = (CHAR_TYPE & M_TYPE_MASK);
    m_uPointer--;
    if(t != M_SYMBOL || TEST_FLAG(m_dwFlags, F_SYMBOLBREAK) == FALSE)
    {
      while((CHAR_TYPE & M_TYPE_MASK) == t && IsHeadOfStream() == false)
      {
        m_uPointer--;
      }
    }
    if(IsHeadOfStream() == false || (CHAR_TYPE & M_TYPE_MASK) != t)
      m_uPointer++;
    if(pCount != NULL)
      *pCount = rbegin - (m_uPointer - 1);
    goto TRUE_RET;
  }
  else
    ASSERT(0);

TRUE_RET:
  if(ppStream != NULL)
    *ppStream = &m_pStream[m_uPointer];
  return true;
}

_SS_TEMPL
clsize _SS_IMPL::Read(T_LPCSTR* ppStream, clsize* pCount)
{
  clsize        uCount;
  const clsize  uOldPointer = m_uPointer;
  if(Get(ppStream, &uCount) == false)
    return -1;
  m_uPointer += uCount;
  if(pCount != NULL)
    *pCount = uCount;
  TRIM_HEAD_GAPS;
  return uOldPointer;
}

_SS_TEMPL
b32 _SS_IMPL::PushPointer()
{
  m_stackPointer.push_back(m_uPointer);
  return true;
}

_SS_TEMPL
b32 _SS_IMPL::PopPointer(b32 bDiscard)
{
  size_t size = m_stackPointer.size();
  if(size == 0)
  {
#ifdef _DEBUG
    ASSERT(0);
#endif // _DEBUG
    return false;
  }
  clvector<clsize>::iterator it = m_stackPointer.begin() + size - 1;
  if(bDiscard == FALSE)
    m_uPointer = *it;
  m_stackPointer.erase(it);
  return true;
}

_SS_TEMPL
b32 _SS_IMPL::ClearPointerStack()
{
  m_stackPointer.clear();
  return true;
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
typename _SS_IMPL::iterator _SS_IMPL::nearest(clsize nOffset)
{
  iterator it(this);
  m_uPointer = nOffset;
  TRIM_HEAD_GAPS;
  Get(&it.marker, &it.length);
  return it;
}

_SS_TEMPL
typename _SS_IMPL::iterator _SS_IMPL::begin()
{
  iterator itBegin(this);
  return next(itBegin); // 这里好像不对

  ////u32 uOldPointer = m_uPointer;
  //m_uPointer = 0;
  //TRIM_HEAD_GAPS
  //GetRef(&beginIt.marker, &beginIt.length);
  ////m_uPointer = uOldPointer;
  //return beginIt;
}

#if defined(_WINDOWS)
_SS_TEMPL 
typename const _SS_IMPL::iterator& _SS_IMPL::end() const
#elif defined(_IOS)
_SS_TEMPL 
  typename _SS_IMPL::const_iterator& _SS_IMPL::end() const
#endif // #if defined(_WINDOWS)
{
  return m_itEnd;
}

_SS_TEMPL 
typename _SS_IMPL::iterator& _SS_IMPL::next(iterator& it)
{
  m_uPointer = (u32)(it.marker - m_pStream) + it.length;
  TRIM_HEAD_GAPS;
  if(Get(&it.marker, &it.length) == false)
  {
    // 结尾
    it.marker = &m_pStream[m_uCountOfChar];
    it.length = 0;
    //it.remain = 0;
    return it;
  }
  const clsize remain = m_uCountOfChar - m_uPointer;
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
  return (u32)(marker - pContainer->m_pStream);
}

_SS_TEMPL
_TStr _SS_IMPL::iterator::ToString() const
{
  _TStr strTemp;
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
  //const size_t uStrLength = CLSTR_LENGTH(m_pBuf);
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