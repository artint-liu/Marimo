#ifndef _CLSTD_TOKEN_H_
#define _CLSTD_TOKEN_H_

namespace clstd {

  template<class _TStr>
  class TokensT
  {
  public:
    //enum MARK
    //{
    //  M_ESCAPE      = 0x0800,  // 转义符
    //  M_SYMBOL      = 0x0000,  // 符号
    //  M_GAP         = 0x0001,  // 空白
    //  M_QUOT        = 0x0002,  // 引号
    //  M_OPN_BRAKETS = 0x0004,  // 开括号
    //  M_CLS_BRAKETS = 0x0008,  // 闭括号
    //  M_LABEL       = 0x0010,  // 标签
    //  M_TYPE_MASK   = 0x001F,  // 类型掩码
    //  M_CALLBACK    = 0x0020,  // 触发回调, 必须是 iterator 的首字节
    //  M_GROUP_MASK  = 0xF000,  // 组掩码, 只适用于引号括号
    //};

    enum FLAGS
    {
      F_SYMBOLBREAK = 0x0001,    // 切开 Symbol, 对于连续的 Symbol 将分成独立的字符分别返回.
    };

    typedef typename _TStr::LPCSTR T_LPCSTR;
    typedef typename _TStr::TChar  TChar;

    struct iterator
    {
    public:
      T_LPCSTR        marker;
      clsize          length;
      const TokensT*  pContainer;

      iterator()
        : marker    (NULL)
        , length    (0)
        , pContainer(NULL) {}

      iterator(const TokensT*  _pContainer) 
        : marker    (_pContainer->m_pBegin)
        , length    (0)
        , pContainer(_pContainer) {}

      iterator(const iterator& it)
        : marker    (it.marker)
        , length    (it.length)
        , pContainer(it.pContainer) {}

      iterator&  operator++();
      iterator&  operator++(int);
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
      _TStr   ToRawString () const;   // 返回原始字符串
      _TStr&  ToRawString (_TStr& str) const;   // 返回原始字符串
      _TStr   ToString    () const;   // 返回字符串，如果字符串带有单引号或者双引号，会被去掉。
      _TStr&  ToString    (_TStr& str) const;   // 返回字符串，如果字符串带有单引号或者双引号，会被去掉。
      b32     IsEqual     (T_LPCSTR str, clsize count) const;
      b32     BeginsWith  (TChar ch) const;
      b32     EndsWith    (TChar ch) const;
      b32     BeginsWith  (T_LPCSTR str) const;
      b32     EndsWith    (T_LPCSTR str) const;
      b32     BeginsWith  (T_LPCSTR str, clsize count) const;
      b32     EndsWith    (T_LPCSTR str, clsize count) const;

      //
      // 符合STL规范的接口
      //
      inline const TChar& front() const
      {
        return marker[0];
      }

      inline const TChar& back() const
      {
        return marker[length - 1];
      }

      inline T_LPCSTR begin() const
      {
        return marker;
      }

      inline T_LPCSTR end() const
      {
        return marker + length;
      }
    };
    typedef size_t (CALLBACK *IteratorProc)(iterator& it, clsize nRemain, u32_ptr lParam); // nRemain 是it.marker[0]到文件结尾的(剩余)长度. 目前不关心返回值
    typedef const iterator const_iterator; 

  protected:
    static const u32     c_nCharTabCount = 128;
    T_LPCSTR      m_pBegin;
    T_LPCSTR      m_pEnd;
    u32           m_dwFlags;
    iterator      m_itEnd;
    IteratorProc  m_pCallBack;        // 针对每一个 Iterator
    u32_ptr       m_lParam;
    u32_ptr       m_lParamTrigger;

  public:
    TokensT(T_LPCSTR pStream = NULL, clsize uCountOfChar = 0);

    b32      Attach            (T_LPCSTR pStream, clsize uCountOfChar);
    b32      IsEndOfStream     (T_LPCSTR pPointer) const;
    b32      IsHeadOfStream    (T_LPCSTR pPointer) const;
    T_LPCSTR GetStreamPtr      () const;
    clsize   GetStreamCount    () const; // 返回的是TChar的类型长度，不是字节数 GetStreamEnd() - GetStreamPtr()
    b32      Get               (T_LPCSTR pPointer, T_LPCSTR* ppOutPointer, clsize* pOutCount) const;    // 取指针当前处的内容
    u32      SetFlags          (u32 dwFlags);
    u32      GetFlags          () const;

    IteratorProc SetIteratorCallBack(IteratorProc pNew, u32_ptr lParam);

    iterator&       next      (iterator& it) const;
    iterator        nearest   (clsize nOffset) const; // 按照nOffset查找最近的iterator，iterator的偏移大于等于nOffset
    iterator        begin     () const;
    const_iterator& end       () const;
    const_iterator  find      (const iterator& itBegin, int nCount, ...) const; // 从 itBegin 开始查找, 如果找到参数列表的任意一个就马上返回
    b32             find_pair (const iterator& itCurrent, iterator& itOpen, iterator& itClose, T_LPCSTR chOpen, T_LPCSTR chClose) const;
  };

  //////////////////////////////////////////////////////////////////////////
  typedef TokensT<clStringA> TokensA;
  typedef TokensT<clStringW> TokensW;
  //////////////////////////////////////////////////////////////////////////

  namespace TokensUtility
  {
    template<class _TStr>
    b32 IsHeadOfLine(const TokensT<_TStr>* pToken, typename _TStr::LPCSTR pChar);

    namespace cxxstyle // C++风格，忽略注释块
    {
      template<class _TStr>
      b32 IsHeadOfLine(const TokensT<_TStr>* pToken, typename _TStr::LPCSTR pChar);

      template<class _TStr>
      clsize ExtendToNewLine(_TStr pTokenPtr, clsize remain);

    } // namespace cxxstyle

#if 0
    // 扫描一个字符串, 返回符合C语言的数字格式的长度
    // 字符串需要以数字或者'.'开头, 函数不会识别'+','-'
    template<typename _TCh>
    clsize ExtendCStyleNumeric(const _TCh* str, clsize max_len);
#endif // 0
  } // namespace TokensUtility

} // namespace clstd

#else
# pragma message(__FILE__": warning : Duplicate included this file.")
#endif // _CLSTD_TOKEN_H_