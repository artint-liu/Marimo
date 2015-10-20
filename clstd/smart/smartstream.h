#ifndef _SMARTSTREAM_2_H_
#define _SMARTSTREAM_2_H_

// 划分规则:
// 1.<引号>之内的字符都算作整体, 遇到<转义符>时根据后续字符内容转义.
// 2.<空格>用来断开<符号>和<标签>,不包括(1)

// {　open brace, open curly　左花括号
// }　close brace, close curly　右花括号
// (　open parenthesis, open paren　左圆括号
// )　close parenthesis, close paren　右圆括号
// () brakets/ parentheses　括号
// [　open bracket 左方括号
// ]　close bracket 右方括号
// [] square brackets　方括号
// \　backslash, sometimes escape　反斜线转义符，有时表示转义符或续行符
// "　single quotation marks 单引号
// ""　double quotation marks 双引号
// , Comma 逗号

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
    M_ESCAPE      = 0x0800,  // 转义符
    M_SYMBOL      = 0x0000,  // 符号
    M_GAP         = 0x0001,  // 空白
    M_QUOT        = 0x0002,  // 引号
    M_OPN_BRAKETS = 0x0004,  // 开括号
    M_CLS_BRAKETS = 0x0008,  // 闭括号
    M_LABEL       = 0x0010,  // 标签
    M_TYPE_MASK   = 0x001F,  // 类型掩码
    M_CALLBACK    = 0x0020,  // 触发回调, 必须是 iterator 的首字节
    M_GROUP_MASK  = 0xF000,  // 组掩码, 只适用于引号括号
  };

  enum FLAGS
  {
    F_SYMBOLBREAK = 0x0001,    // 切开 Symbol, 对于连续的 Symbol 将分成独立的字符分别返回.
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
    b32  operator==(TChar ch) const;            // 如果iterator是多字节将返回FALSE
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
    iterator  operator+(const size_t n) const;  // 只能使用增量

    clsize  offset      () const;   // 返回值 TChar 的类型偏移， 不是字节偏移
    _TStr   ToString    () const;
    b32     BeginsWith  (T_LPCSTR str) const;
    b32     EndsWith    (T_LPCSTR str) const;
    b32     BeginsWith  (T_LPCSTR str, clsize count) const;
    b32     EndsWith    (T_LPCSTR str, clsize count) const;

    //
    // 符合STL规范的接口
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
  typedef u32 (CALLBACK *IteratorProc)(iterator& it, clsize nRemain, u32_ptr lParam); // nRemain 是it.marker[0]到文件结尾的(剩余)长度. 目前不关心返回值
  typedef const iterator const_iterator; 

protected:
  static const u32     c_nCharTabCount = 128;
  T_LPCSTR      m_pBegin;
  T_LPCSTR      m_pEnd;
  SemType       m_aCharSem[c_nCharTabCount];
  u32           m_dwFlags;
  iterator      m_itEnd;
  IteratorProc  m_pCallBack;        // 针对每一个 Iterator
  u32_ptr       m_lParam;
  IteratorProc  m_pTriggerCallBack; // 带有触发标志的 Iterator 首字节
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
  clsize   GetStreamCount    () const; // 返回的是TChar的类型长度，不是字节数 GetStreamEnd() - GetStreamPtr()
  b32      Get               (T_LPCSTR pPointer, T_LPCSTR* ppOutPointer, clsize* pOutCount) const;    // 取指针当前处的内容
  u32      SetFlags          (u32 dwFlags);
  u32      GetFlags          () const;

  IteratorProc SetIteratorCallBack(IteratorProc pNew, u32_ptr lParam);
  IteratorProc SetTriggerCallBack (IteratorProc pTrigger, u32_ptr lParam);

  iterator&       next      (iterator& it) const;
  iterator        nearest   (clsize nOffset) const; // 按照nOffset查找最近的iterator，iterator的偏移大于等于nOffset
  iterator        begin     () const;
  const_iterator& end       () const;
  const_iterator  find      (const iterator& itBegin, int nCount, ...) const; // 从 itBegin 开始查找, 如果找到参数列表的任意一个就马上返回
  b32             find_pair (const iterator& itCurrent, iterator& itOpen, iterator& itClose, T_LPCSTR chOpen, T_LPCSTR chClose) const;
};

typedef SmartStreamT<clStringA, SmartStream_TraitsA> SmartStreamA;
typedef SmartStreamT<clStringW, SmartStream_TraitsW> SmartStreamW;

namespace SmartStreamUtility
{
  //template<class _SSIt> //SmartStreamT::iterator
  //int ParseArray(_SSIt itBegin, _SSIt itEnd, CLLPCSTR Separators, CLLPCSTR Pairs, clStringArrayA& aStrings);

  // 查找配对标记开始和结束位置，同时会自动处理嵌套的配对标记。
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

  // FindPair 重载，支持字符方式表示
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

  // 把一段标记流按照指定的分隔符切分并放到字符串数组中
  // 分隔符列表是以单字节表示其含义, 没有前后字符上下文和顺序的关联, 分隔符列表至少要指定一个分隔符
  // 配对符号表可以为NULL, 如果不为空必须含有偶数个字符, 每个两个字符互相为一对.
  // bTight 表示配对符号中的内容是否以紧凑方式保存， 紧凑方式去掉了空白符号,如果没有正确的分隔符可能会造成错误的合并。
  // bTight == FALSE 时配对符号中的内容将保持字符串的原始内容
  template<class _Iter>
  int ParseArray(_Iter itBegin, _Iter itEnd, CLLPCSTR Separators, CLLPCSTR Pairs, b32 bTight, clStringArrayA& aStrings)
  {
    //ASSERT(bTight = TRUE);  // 暂时不支持其他

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

        // 找到了分割符号
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
                // 暂时不支持保留原始内容的方式
                CLBREAK;
              }
              ASSERT(str.IsNotEmpty());
              aStrings.push_back(str);
              str.Clear();
              break;
            }
          }
          // 找到了配对符号
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

  // 扩展it到换行标记，扩展后包含换行标记
  // offset 是相对于 it.marker 的偏移
  // remain 是 it.marker 到流结尾的剩余长度（字符数）
  // flags 是标志位，目前只有 0x0001 表示如果以"\"字符结尾并换行，则忽略此处的换行符继续扫描下一行
  // 返回true表示找到一个换行符号，false表示到了文件结尾，此时it被扩展到文件结尾
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

  // 判断pChar所在位置是不是在一行的开头，以下情况返回TRUE
  // 1.pChar之前的字符是'\n'
  // 2.pChar在文档开头
  // pChar之前的空白会被忽略，空白字符一般是0x20空格符号或者'\t'制表符号，
  // 也可以用户自己标记空白字符
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
    return TRUE; // 到文档开头了
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

  // [取出一些iterator放到list中]
  // 返回值：如果找到szEnd或者到达nCount数量，返回true
  //        如果到结尾的话返回false
  //        如果循环结束的优先级是szEnd>itEnd>nCount
  // 参数：nCount如果为0则表示忽略这个参数
  // 注意：it会被改变！
  template<typename _TCh, class _Iter, class _ListT>
  b32 Get(_Iter& it, const _Iter& itEnd, const _TCh* szEnd, clsize nCount, _ListT& list) // [没测试过]
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

  // [取出一些iterator放到list中]
  // 返回值：1.如果找到szEnd或者到达nCount数量，返回true
  //        2.如果到结尾的话返回false，此时数组中不含itEnd
  //        3.如果用户的lambda表达式中返回false，则终止循环并返回true，数组中不包含这个iterator
  // 参数：nCount如果为0则表示忽略这个参数
  // 注意：it会被改变！
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

  // 翻译引语
  // 如果iterator没有被引号引用，则直接转换为字符串返回
  // 否则会将引号去掉，并将引号内的内转义符翻译成其它字符
  // 注意：iterator必须是头尾使用引号的才能被翻译
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

      // 用来处理有可能是不正确的结尾
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
    // 极端情况下所有字符都变成"转义符+字符"形式，再加上首尾引号
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