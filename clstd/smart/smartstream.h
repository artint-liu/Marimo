#ifndef _SMARTSTREAM_2_H_
#define _SMARTSTREAM_2_H_

// ���ֹ���:
// 1.<����>֮�ڵ��ַ�����������, ����<ת���>ʱ���ݺ����ַ�����ת��.
// 2.<�ո�>�����Ͽ�<����>��<��ǩ>,������(1)

// {��open brace, open curly��������
// }��close brace, close curly���һ�����
// (��open parenthesis, open paren����Բ����
// )��close parenthesis, close paren����Բ����
// () brakets/ parentheses������
// [��open bracket ������
// ]��close bracket �ҷ�����
// [] square brackets��������
// \��backslash, sometimes escape����б��ת�������ʱ��ʾת��������з�
// "��single quotation marks ������
// ""��double quotation marks ˫����
// , Comma ����

//typedef unsigned __int32 u32;
//typedef char CH;
//typedef wchar_t WCH;
//
//#define CU32  const unsigned __int32
//#define CU16  const unsigned __int16
//#define CCH    const char
//#define SemType  u32

//#define CU32  const u32
//#define CU16  const u16
#define CCH    const ch

#pragma warning(disable: 4661)
#ifdef _X64
#pragma pack(push)
#pragma pack(8)
#endif // #ifdef _X64

//#include <vector>
//using namespace std;

extern "C" b32 strcmpnA(const ch* lpString1, const ch* lpString2, int nCount);
extern "C" b32 strcmpnW(const wch* lpString1, const wch* lpString2, int nCount);

struct SmartStream_TraitsW
{
  static b32  _StrCmpN(const wch*, const wch*, int);
};

struct SmartStream_TraitsA
{
  static b32  _StrCmpN(const ch*, const ch*, int);
};

template<
  class    _TStr, 
  class    _Traits>
class SmartStreamT
{
public:
  enum MARK
  {
    M_ESCAPE      = 0x0800,  // ת���
    M_SYMBOL      = 0x0000,  // ����
    M_GAP         = 0x0001,  // �հ�
    M_QUOT        = 0x0002,  // ����
    M_OPN_BRAKETS = 0x0004,  // ������
    M_CLS_BRAKETS = 0x0008,  // ������
    M_LABEL       = 0x0010,  // ��ǩ
    M_TYPE_MASK   = 0x001F,  // ��������
    M_CALLBACK    = 0x0020,  // �����ص�, ������ iterator �����ֽ�
    M_GROUP_MASK  = 0xF000,  // ������, ֻ��������������
  };

  enum FLAGS
  {
    F_SYMBOLBREAK = 0x0001,    // �п� Symbol, ���������� Symbol ���ֳɶ������ַ��ֱ𷵻�.
  };

  typedef typename _TStr::LPCSTR T_LPCSTR;
  typedef typename _TStr::TChar  TChar;
  typedef u32                    SemType;

  struct iterator
  {
  public:
    const
    SmartStreamT*   pContainer;
    T_LPCSTR        marker;
    clsize          length;

    iterator()
      : pContainer(NULL)
      , marker    (NULL)
      , length    (0){}

    iterator(const SmartStreamT*  _pContainer) 
      : pContainer(_pContainer)
      , marker    (_pContainer->m_pBegin)
      , length    (0){}
    
    iterator(const iterator& it)
      : pContainer(it.pContainer)
      , marker    (it.marker)
      , length    (it.length){}

    iterator&  operator++();
    b32  operator==(const _TStr& str) const;
    b32  operator==(T_LPCSTR pStr) const;
    b32  operator==(TChar ch) const;            // ���iterator�Ƕ��ֽڽ�����FALSE
    b32  operator!=(const _TStr& str) const;
    b32  operator!=(T_LPCSTR pStr) const;
    b32  operator!=(TChar ch) const;
    b32  operator==(const iterator& it) const;
    b32  operator!=(const iterator& it) const;
    b32  operator>=(const iterator& it) const;
    b32  operator<=(const iterator& it) const;
    b32  operator>(const iterator& it) const;
    b32  operator<(const iterator& it) const;
    iterator&  operator=(const iterator& it);
    iterator  operator+(const size_t n) const;  // ֻ��ʹ������

    clsize  offset      () const;   // ����ֵ TChar ������ƫ�ƣ� �����ֽ�ƫ��
    _TStr   ToString    () const;
    b32     BeginsWith  (T_LPCSTR str) const;
    b32     EndsWith    (T_LPCSTR str) const;
    b32     BeginsWith  (T_LPCSTR str, clsize count) const;
    b32     EndsWith    (T_LPCSTR str, clsize count) const;

    //
    // ����STL�淶�Ľӿ�
    //
    inline const TChar& front() const {
      return marker[0];
    }

    inline const TChar& back() const {
      return marker[length - 1];
    }

    inline T_LPCSTR begin() const {
      return marker;
    }

    inline T_LPCSTR end() const {
      return marker + length;
    }


  };
  typedef u32 (CALLBACK *IteratorProc)(iterator& it, clsize nRemain, u32_ptr lParam); // nRemain ��it.marker[0]���ļ���β��(ʣ��)����. Ŀǰ�����ķ���ֵ
  typedef const iterator const_iterator; 

protected:
  static const u32     c_nCharTabCount = 128;
  T_LPCSTR      m_pBegin;
  T_LPCSTR      m_pEnd;
  SemType       m_aCharSem[c_nCharTabCount];
  u32           m_dwFlags;
  iterator      m_itEnd;
  IteratorProc  m_pCallBack;        // ���ÿһ�� Iterator
  u32_ptr       m_lParam;
  IteratorProc  m_pTriggerCallBack; // ���д�����־�� Iterator ���ֽ�
  u32_ptr       m_lParamTrigger;

public:
  SmartStreamT               (T_LPCSTR pStream = NULL, clsize uCountOfChar = NULL);
  b32      Initialize        (T_LPCSTR pStream, clsize uCountOfChar);
  void     GetCharSemantic   (SemType* pCharSem, clsize uStart, clsize uEnd) const;
  void     SetCharSemantic   (const SemType* pCharSem, clsize uStart, clsize uEnd);
  SemType  GetCharSemantic   (TChar ch) const;
  SemType  SetCharSemantic   (TChar ch, SemType flagCharSem);
  b32      IsEndOfStream     (T_LPCSTR pPointer) const;
  b32      IsHeadOfStream    (T_LPCSTR pPointer) const;
  T_LPCSTR GetStreamPtr      () const;
  clsize   GetStreamCount    () const; // ���ص���TChar�����ͳ��ȣ������ֽ��� GetStreamEnd() - GetStreamPtr()
  b32      Get               (T_LPCSTR pPointer, T_LPCSTR* ppOutPointer, clsize* pOutCount) const;    // ȡָ�뵱ǰ��������
  u32      SetFlags          (u32 dwFlags);
  u32      GetFlags          () const;

  IteratorProc SetIteratorCallBack(IteratorProc pNew, u32_ptr lParam);
  IteratorProc SetTriggerCallBack (IteratorProc pTrigger, u32_ptr lParam);

  iterator&       next      (iterator& it) const;
  iterator        nearest   (clsize nOffset) const; // ����nOffset���������iterator��iterator��ƫ�ƴ��ڵ���nOffset
  iterator        begin     () const;
  const_iterator& end       () const;
  const_iterator  find      (const iterator& itBegin, int nCount, ...) const; // �� itBegin ��ʼ����, ����ҵ������б������һ�������Ϸ���
  b32             find_pair (const iterator& itCurrent, iterator& itOpen, iterator& itClose, T_LPCSTR chOpen, T_LPCSTR chClose) const;
};

typedef SmartStreamT<clStringA, SmartStream_TraitsA> SmartStreamA;
typedef SmartStreamT<clStringW, SmartStream_TraitsW> SmartStreamW;

namespace SmartStreamUtility
{
  //template<class _SSIt> //SmartStreamT::iterator
  //int ParseArray(_SSIt itBegin, _SSIt itEnd, CLLPCSTR Separators, CLLPCSTR Pairs, clStringArrayA& aStrings);

  // ������Ա�ǿ�ʼ�ͽ���λ�ã�ͬʱ���Զ�����Ƕ�׵���Ա�ǡ�
  template<typename TChar, class _Iter>
  b32 FindPair(const _Iter& itCurrent, _Iter& itOpen, _Iter& itClose, const TChar* chOpen, const TChar* chClose)
  {
    _Iter it = itCurrent;
    itOpen = itCurrent;
    int nDepth = 0;
    while(it != it.pContainer->end())
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

  // FindPair ���أ�֧���ַ���ʽ��ʾ
  template<class _Iter>
  b32 FindPair(const _Iter& itCurrent, _Iter& itOpen, _Iter& itClose, ch chOpen, ch chClose)
  {
    _Iter it = itCurrent;
    itOpen = itCurrent;
    int nDepth = 0;
    while(it != it.pContainer->end())
    {
      if(it.length == 1)
      {
        if(it.marker[0] == chOpen)
        {
          if(nDepth == 0)
            itOpen = it;
          nDepth++;
        }
        else if(it.marker[0] == chClose)
        {
          nDepth--;
          if(nDepth <= 0)
          {
            itClose = it;
            return(nDepth == 0);
          }
        }
      }
      ++it;
    }
    return false;
  }

  // ��һ�α��������ָ���ķָ����зֲ��ŵ��ַ���������
  // �ָ����б����Ե��ֽڱ�ʾ�京��, û��ǰ���ַ������ĺ�˳��Ĺ���, �ָ����б�����Ҫָ��һ���ָ���
  // ��Է��ű����ΪNULL, �����Ϊ�ձ��뺬��ż�����ַ�, ÿ�������ַ�����Ϊһ��.
  // bTight ��ʾ��Է����е������Ƿ��Խ��շ�ʽ���棬 ���շ�ʽȥ���˿հ׷���,���û����ȷ�ķָ������ܻ���ɴ���ĺϲ���
  // bTight == FALSE ʱ��Է����е����ݽ������ַ�����ԭʼ����
  template<class _Iter>
  int ParseArray(_Iter itBegin, _Iter itEnd, CLLPCSTR Separators, CLLPCSTR Pairs, b32 bTight, clStringArrayA& aStrings)
  {
    //ASSERT(bTight = TRUE);  // ��ʱ��֧������

    //i32 nPairsLen = 0;
    if(Separators == NULL) {
      return -1;
    }
    if(Pairs != NULL) {
      const int nPairsLen = clstd::strlenT(Pairs);
      if((nPairsLen & 1) == 1) {
        return -2;
      }
    }

    clStringA str;
    for(_Iter it = itBegin; it != itEnd; ++it)
    {
      if(it.length == 1)
      {
        int i = 0;
        for(; Separators[i] != '\0'; ++i) {
          if(it.marker[0] == Separators[i]) {
            if(str.IsNotEmpty())
            {
              aStrings.push_back(str);
              str.Clear();
            }
            break;
          }
        } // for

        // �ҵ��˷ָ����
        if(Separators[i] != '\0') { continue; }

        if(Pairs != NULL)
        {
          for(i = 0; Pairs[i] != '\0'; i += 2)
          {
            if(it.marker[0] == Pairs[i]) {
              _Iter itOpen, itClose;
              SmartStreamUtility::FindPair(it, itOpen, itClose, Pairs[i], Pairs[i + 1]);

              if(bTight)
              {
                for(it = itOpen; it != itClose; ++it) {
                  str.Append(it.ToString());
                }
                str.Append(it.ToString());
              }
              else
              {
                // ��ʱ��֧�ֱ���ԭʼ���ݵķ�ʽ
                CLBREAK;
              }
              ASSERT(str.IsNotEmpty());
              aStrings.push_back(str);
              str.Clear();
              break;
            }
          }
          // �ҵ�����Է���
          if(Pairs[i] != '\0') { continue; }
        } // if(Pairs != NULL)
      } // for
      str.Append(it.ToString());
    }
    if(str.IsNotEmpty()) {
      aStrings.push_back(str);
    }
    return (int)aStrings.size();
  }

  // ��չit�����б�ǣ���չ��������б��
  // offset ������� it.marker ��ƫ��
  // remain �� it.marker ������β��ʣ�೤�ȣ��ַ�����
  // flags �Ǳ�־λ��Ŀǰֻ�� 0x0001 ��ʾ�����"\"�ַ���β�����У�����Դ˴��Ļ��з�����ɨ����һ��
  // ����true��ʾ�ҵ�һ�����з��ţ�false��ʾ�����ļ���β����ʱit����չ���ļ���β
  template<class _Iter>
  b32 ExtendToNewLine(_Iter &it, clsize offset, clsize remain, u32 flags = 0)
  {
    while(offset < remain) {
      if(it.marker[offset] == '\n') {
        if(TEST_FLAG(flags, 0x0001) && (
          (offset > 0 && it.marker[offset - 1] == '\\') ||
          (offset > 1 && it.marker[offset - 1] == '\r' && it.marker[offset - 2] == '\\')))
        {
          ++offset;
          continue;
        }
        it.length = offset + 1;
        return TRUE;
      }
      ++offset;
    }
    it.length = remain;
    return FALSE;
  }

  // �ж�pChar����λ���ǲ�����һ�еĿ�ͷ�������������TRUE
  // 1.pChar֮ǰ���ַ���'\n'
  // 2.pChar���ĵ���ͷ
  // pChar֮ǰ�Ŀհ׻ᱻ���ԣ��հ��ַ�һ����0x20�ո���Ż���'\t'�Ʊ���ţ�
  // Ҳ�����û��Լ���ǿհ��ַ�
  template<class SmartStreamT, typename _Ty>
  b32 IsHeadOfLine(SmartStreamT* pStream, _Ty* pChar)
  {
    auto* ptr = pStream->GetStreamPtr();
    auto* p = pChar - 1;
    while(p >= ptr) {
      if(*p == '\n') {
        return TRUE;
      }
      else if(TEST_FLAG_NOT(pStream->GetCharSemantic(*p), SmartStreamT::M_GAP)) {
        return FALSE;
      }
      --p;
    }
    return TRUE; // ���ĵ���ͷ��
  }


  template<class _Iter>
  b32 ExtendToCStyleBlockComment(_Iter& it, clsize offset, clsize remain)
  {
    --remain;
    auto c0 = it.marker[offset];
    while(offset < remain)
    {
      auto c1 = it.marker[offset + 1];
      if(c0 == '*' && c1 == '/') {
        it.length = offset + 2;
        return TRUE;
      }
      c0 = c1;
      ++offset;
    }
    return FALSE;
  }

  // [ȡ��һЩiterator�ŵ�list��]
  // ����ֵ������ҵ�szEnd���ߵ���nCount����������true
  //        �������β�Ļ�����false
  //        ���ѭ�����������ȼ���szEnd>itEnd>nCount
  // ������nCount���Ϊ0���ʾ�����������
  // ע�⣺it�ᱻ�ı䣡
  template<typename _TCh, class _Iter, class _ListT>
  b32 Get(_Iter& it, const _Iter& itEnd, const _TCh* szEnd, clsize nCount, _ListT& list) // [û���Թ�]
  { 
    do {
      if(it == szEnd) {
        list.push_back(it);
        return TRUE;
      }
      else if(it == itEnd) {
        return FALSE;
      }
      list.push_back(it);
      ++it;
    } while (list.size() != nCount);
    return TRUE;
  }

  // [ȡ��һЩiterator�ŵ�list��]
  // ����ֵ��1.����ҵ�szEnd���ߵ���nCount����������true
  //        2.�������β�Ļ�����false����ʱ�����в���itEnd
  //        3.����û���lambda���ʽ�з���false������ֹѭ��������true�������в��������iterator
  // ������nCount���Ϊ0���ʾ�����������
  // ע�⣺it�ᱻ�ı䣡
  template<class _Iter, class _ListT, class _Fn>
  b32 Get(_Iter& it, const _Iter& itEnd, clsize nCount, _ListT& list, _Fn fn)
  { 
    do {
      if(it == itEnd) {
        return FALSE;
      }
      else if( ! fn(it)) {
        return TRUE;
      }
      list.push_back(it);
      ++it;
    } while (list.size() != nCount);
    return TRUE;
  }

  // ��������
  // ���iteratorû�б��������ã���ֱ��ת��Ϊ�ַ�������
  // ����Ὣ����ȥ�������������ڵ���ת�������������ַ�
  // ע�⣺iterator������ͷβʹ�����ŵĲ��ܱ�����
  template<class _Iter, class _TString>
  _TString& TranslateQuotation(const _Iter& it, _TString& str)
  {
    _TString::TChar* pTemp = str.GetBuffer(it.length + 1);
    u32 i = 0, n = 0;
    if(it.marker[0] == '\"' || it.marker[0] == '\'')
    {
      ++i;

      for(; i < it.length - 1; i++) {
        auto c = it.marker[i];
        if(c == '\\') {
          ++i;
          c = it.marker[i];
          switch(c)
          {
          case 'r': c = '\r'; break;
          case 'n': c = '\n'; break;
          case 't': c = '\t'; break;
          case'\\': c = '\\'; break;
          }
        }
        pTemp[n++] = c;
      }

      // ���������п����ǲ���ȷ�Ľ�β
      if(it.marker[it.length - 1] != '\"' && it.marker[it.length - 1] != '\'') {
        pTemp[n++] = it.marker[i];
      }
    }
    else
    {
      for(; i < it.length; i++) {
        pTemp[n++] = it.marker[i];
      }
    }

    pTemp[n] = '\0';
    str.ReleaseBuffer();

    return str;
  }

  template<class _TString>
  _TString& MakeQuotation(_TString& str, typename _TString::LPCSTR szText)
  {
    // ��������������ַ������"ת���+�ַ�"��ʽ���ټ�����β����
    clsize length = clstd::strlenT<_TString::TChar>(szText);
    _TString::TChar* pTemp = str.GetBuffer(length * 2 + 2 + 1);
    size_t n = 1;
    pTemp[0] = '\"';

    for(size_t i = 0; szText[i] != '\0'; i++)
    {
      auto c = szText[i];
      switch(c)
      {
      case '\r':  pTemp[n++] = '\\';  c = 'r';  break;
      case '\n':  pTemp[n++] = '\\';  c = 'n';  break;
      case '\t':  pTemp[n++] = '\\';  c = 't';  break;
      case '\"':  pTemp[n++] = '\\';  c = '\"'; break;
      case '\'':  pTemp[n++] = '\\';  c = '\''; break;
      case '\\':  pTemp[n++] = '\\';  c = '\\'; break;
      }
      pTemp[n++] = c;
    }
    pTemp[n++] = '\"';
    pTemp[n] = '\0';

    str.ReleaseBuffer();
    return str;
  }
} // namespace SmartStreamUtility

#ifdef _X64
#pragma pack(pop)
#endif // #ifdef _X64

#endif  // _SMARTSTREAM_2_H_