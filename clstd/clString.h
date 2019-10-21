#ifndef _CLSTD_STRING_H_
#define _CLSTD_STRING_H_

#ifndef _CL_ALLOCATOR_H_
#error Must be include "clAllocator.h" first.
#endif // _CL_ALLOCATOR_H_

//创建本机对象的自定义视图
// https://docs.microsoft.com/zh-cn/visualstudio/debugger/create-custom-views-of-native-objects?view=vs-2015
// https://docs.microsoft.com/zh-cn/visualstudio/debugger/format-specifiers-in-cpp?view=vs-2015

// autoexp.dat 解析文件
//clstd::StringX<wchar_t,*>|clstd::StringX<unsigned short,*>{
//  preview  (  #("L", [$e.m_pBuf,su]))
//
//  children (
//    #(
//      #([size] : *(size_t*)((char*)$e.m_pBuf-4)),
//      #([capacity] : *(size_t*)((char*)$e.m_pBuf-8)),
//      #array(expr: $e.m_pBuf[$i], size:(*(size_t*)((char*)$e.m_pBuf-4)))
//    )
//  )
//}
//
//clstd::StringX<char,*>{
//  preview  ([$e.m_pBuf,s])
//
//  children (
//    #(
//      #([size] : *(size_t*)((char*)$e.m_pBuf-4)),
//      #([capacity] : *(size_t*)((char*)$e.m_pBuf-8)),
//      #array(expr: $e.m_pBuf[$i], size:(*(size_t*)((char*)$e.m_pBuf-4)))
//    )
//  )
//}

//#include <string>
//#include <xstring>

#include <stdarg.h>

namespace clstd
{
  template<typename _TCh>
  _TCh* strcpynT(_TCh* pDest, const _TCh* pSrc, size_t uCount);

  template<typename _TCh>
  _TCh* strcpyT(_TCh* pDest, const _TCh* pSrc);

  template<typename _TCh>
  _TCh* strstrT(_TCh* pStr, const _TCh* pSubStr);

  template<typename _TCh>
  _TCh tolowerT(_TCh ch);

  template<typename _TCh>  // 比较str1的前n个字符,如果遇到'\0'则提前结束
  int strncmpiT( const _TCh *str1, const _TCh *str2, clsize n );

  template<typename _TCh>
  int strcmpiT( const _TCh *str1, const _TCh *str2);

  template<typename _TCh> // 比较str1的前n个字符,如果遇到'\0'则提前结束
  int strncmpT( const _TCh *str1, const _TCh *str2, clsize n );

  template<typename _TCh>
  int strcmpT(const _TCh *str1, const _TCh *str2);

  template<typename _TCh>
  _TCh* strchrT(const _TCh *str, i32 ch);

  template<typename _TCh>
  clsize strlenT(const _TCh* str);

  template<typename _TCh>
  b32 IsNumericT(const _TCh* str, clsize len = -1, i32 radix = 10);  // 遇到\0或者达到len时退出

  template<typename _TCh>
  u32 HashStringT(const _TCh* str, clsize len);

  template<typename _TCh>
  u32 HashStringT(const _TCh* str);

  // string to integer
  i32 xtoi(const wch* str);
  i32 xtoi(const ch* str);
  i32 xtoi(i32 radix, const wch* str, clsize len = -1);
  i32 xtoi(i32 radix, const ch* str, clsize len = -1);
  i64 xtoi64(i32 radix, const wch* str, clsize len = -1);
  i64 xtoi64(i32 radix, const ch* str, clsize len = -1);

  // string to unsigned integer
  u32 xtou(const wch* str);
  u32 xtou(const ch* str);
  u32 xtou(i32 radix, const wch* str, clsize len = -1);
  u32 xtou(i32 radix, const ch* str, clsize len = -1);
  u64 xtou64(i32 radix, const wch* str, clsize len = -1);
  u64 xtou64(i32 radix, const ch* str, clsize len = -1);

  // integer to string
  void ltox(i32 value, wch* pDest, size_t uSize, i32 radix, i32 upper = 0);
  void ltox(i32 value, ch* pDest, size_t uSize, i32 radix, i32 upper = 0);
  void l64tox(i64 value, wch* pDest, size_t uSize, i32 radix, i32 upper = 0);
  void l64tox(i64 value, ch* pDest, size_t uSize, i32 radix, i32 upper = 0);

  // unsigned integer to string
  void ultox(u32 value, wch* pDest, size_t uSize, i32 radix, i32 upper = 0);
  void ultox(u32 value, ch* pDest, size_t uSize, i32 radix, i32 upper = 0);
  void ul64tox(u64 value, wch* pDest, size_t uSize, i32 radix, i32 upper = 0);
  void ul64tox(u64 value, ch* pDest, size_t uSize, i32 radix, i32 upper = 0);

  int ftox(double value, wch* pDest, int uSize, int prec1 = 10, ch format = 'F');
  int ftox(double value,  ch* pDest, int uSize, int prec1 = 10, ch format = 'F');

  float xtof(const ch* str);
  float xtof(const wch* str);
  double xtod(const ch* str);
  double xtod(const wch* str);
  float xtof(const ch* str, size_t len);
  float xtof(const wch* str, size_t len);
  double xtod(const ch* str, size_t len);
  double xtod(const wch* str, size_t len);

  int isdigit(i32 c);
}


//typedef std::string AString;
//typedef std::wstring WString;

//extern clstd::Allocator g_Alloc_clStringW;
//extern clstd::Allocator g_Alloc_clStringA;
extern clstd::StdAllocator g_StdAlloc;

namespace clstd
{
  template<typename _TCh, class _TAllocator, _TAllocator& alloc, class _Traits>
  class StringX
  {
  public:
    enum 
    {
#ifdef __clang__
      npos = -1,
#else
      npos = (size_t)-1,
#endif // #ifdef __clang__
    };
    typedef typename _Traits::_XCh _XCh;
    typedef _Traits MyTraits;
    typedef _TCh        TChar;
    typedef _XCh        XChar;
    typedef const _TCh* LPCSTR;
#if defined(_CL_ARCH_X86) || defined(_CL_ARCH_ARM)
    typedef size_t      U32;
    typedef u64         U64;
#elif defined(_CL_ARCH_X64) || defined(_CL_ARCH_ARM64)
    typedef u32         U32;
    typedef size_t      U64;
#endif

  private:
    _TCh*        m_pBuf;  // 只能有一个变量, 否则作为Format参数时会把多余的成员变量也压入堆栈, 出现问题.

  protected:
    void  _AllocBuffer        (_TAllocator* pAlloc, size_t uLength);
    void  _ResizeLength       (size_t uLength);
    void  _ResizeLengthNoCopy (size_t uLength);
    void  _Reduce             (size_t uLength);
    void  _Grow               (size_t uCapacity);

  public:
    StringX();
    StringX(const _TCh* pStr);
    StringX(const _TCh* pStr, size_t uCount);
    StringX(const _XCh* pStrX);
    StringX(const _XCh* pStrX, size_t uCount);
    StringX(const _TCh cCh, size_t uCount);
    StringX(const StringX& clStr);
    explicit StringX(const int nInteger);
    explicit StringX(const float fFloat, char mode = 'F');
    explicit StringX(const long lLong);
#if defined(_CL_ARCH_X86) || defined(_CL_ARCH_ARM)
    explicit StringX(const size_t val);
    explicit StringX(const U64 val);
#elif defined(_CL_ARCH_X64) || defined(_CL_ARCH_ARM64)
    explicit StringX(const U32 uInteger);
    explicit StringX(const size_t val);
#else
# error Missing cpu architecture
#endif
    ~StringX();

    StringX& operator=(const _TCh* pStr);
    StringX& operator=(const _XCh* pStrX);
    StringX& operator=(const _TCh ch);
    StringX& operator=(const StringX& clStr);
    StringX& operator=(const int nInteger);
    StringX& operator=(const float fFloat);
    StringX& operator=(const long lLong);
    StringX& operator=(const unsigned int uInteger);
    StringX& operator=(const unsigned long uLong);

    //bool operator==(const _TCh* pStr) const;
    bool operator==(const StringX& clStr2) const;
    bool operator<(const StringX& clStr2) const;
    bool operator>(const StringX& clStr2) const;

    inline friend bool operator==(const StringX& clStr, const _TCh* pStr)
    {
      return clStr.Compare(pStr) == 0;
    }
    inline friend bool operator!=(const StringX& clStr, const _TCh* pStr)
    {
      return clStr.Compare(pStr) != 0;
    }

    StringX operator+(const _TCh* pStr) const;
    StringX operator+(const _TCh ch) const;
    StringX operator+(const StringX& clStr2) const;
    StringX operator+(const _XCh* pStrX) const;

    // 下面这些有歧义 (str + n) 不确定是字符串后面追加数字还是字符串数组索引
    //StringX operator+(int nInteger) const;
    //StringX operator+(float fFloat) const;
    //StringX operator+(long lLong) const;
    //StringX operator+(unsigned int uInteger) const;
    //StringX operator+(unsigned long uLong) const;

    StringX& operator+=(const _TCh* pStr);
    StringX& operator+=(const _XCh* pStrX);
    StringX& operator+=(const _TCh cCh);
    StringX& operator+=(const StringX& clStr);
    //StringX& operator+=(const float fFloat);
    //StringX& operator+=(const int nInteger);  // 参数容易混淆_TCh,出现Bug
    //StringX& operator+=(const long lLong);  // 参数容易混淆为_TCh,出现Bug
    //StringX& operator+=(const unsigned int uInteger);
    //StringX& operator+=(const unsigned long uLong);

    _TCh&    operator[]    (int nIdx);
    _TCh&    operator[]    (clsize nIdx);
    //_TCh&    operator[]    (clsize nIdx);
    _TCh&       Front     ();
    _TCh&       Back      ();
    const _TCh& Front     () const;
    const _TCh& Back      () const;

    operator const _TCh*() const;
    const _TCh* CStr() const;

    size_t      GetLength       () const; // 获得字符串长度，不包括'\0'
    size_t      GetCapacity     () const; // 获得字符串缓冲区容量，一般没啥卵用

    _TCh*       GetBuffer       () const;       // 获得可直接操作的缓冲缓冲区指针，完成后需要调用ReleaseBuffer
    _TCh*       GetBuffer       (size_t nSize); // 分配一个指定长度的缓冲区，操作完成后调用ReleaseBuffer
    void        Reserve         (size_t nSize); // 缓冲区扩充到指定容量
    void        ReleaseBuffer   ();             // 重新规整缓冲区

    size_t      Format          (const _TCh *pFmt, ...);  // 按照指定格式生成字符串（原来的内容会被替代）

    size_t      Replace         (size_t idx, size_t uCount, const _TCh* pStr, size_t nStrLength);      // 从 idx 开始把 uCount 个字符替换为 pStr, 返回值是pStr替换后结尾的位置
    size_t      Replace         (size_t idx, size_t uCount, const _TCh* pStr);      // 从 idx 开始把 uCount 个字符替换为 pStr, 返回值是pStr替换后结尾的位置
    size_t      Replace         (_TCh cFind, _TCh cReplaceWith, size_t uStart = 0); // 查找某个字符，替换为另一个
    StringX&    Replace         (const _TCh* szFind, const _TCh* szReplace, size_t uStart = 0);

    StringX&    Append          (const _XCh* pStrX);                          // 追加不同编码的字符串
    StringX&    Append          (const _XCh* pStrX, size_t uCount);           // 还用解释么？
    StringX&    Append          (const _TCh* pStr);                           // 追加字符串
    StringX&    Append          (const _TCh* pStr, size_t uCount);            // 追加字符串，如果长度大于count会被截断
    StringX&    Append          (const _TCh* pStr, _TCh cFill, long nWidth);  // 追加字符串，如果长度小于width会以指定字符填充, width > 0 填充到头部，width < 0 填充尾部，width == 0按实际大小填充
    StringX&    Append          (_TCh cCh);                                   // 追加一个字符
    StringX&    Append          (_TCh cCh, size_t uCount);                    // 追加N个字符
    StringX&    Append          (const StringX& clStr);                       // 这个还能看不懂？
    StringX&    AppendFloat     (float val, char mode = 'F');                 // mode是转换模式，'F'标准浮点模式，'E'科学计数模式，'R'阅读增强模式
    StringX&    AppendInteger32 (s32 val, int nNumGroup = 0);                 // 后面几个自己猜
    StringX&    AppendUInt32    (u32 val, int nNumGroup = 0);
    StringX&    AppendInteger64 (s64 val, int nNumGroup = 0);
    StringX&    AppendUInt64    (u64 val, int nNumGroup = 0);
    StringX&    AppendFormat    (const _TCh *pFmt, ...);                      // 追加格式化字符串，这个保留原来的内容

    void        MakeReverser    (); // 卧槽，全反了
    void        MakeUpper       (); // 全部大写化
    void        MakeLower       (); // 全部小写化

    int         Compare         (const _TCh* pStr) const;                 // 比较字符串，返回0是全部相同
    int         CompareNoCase   (const _TCh* pStr) const;                 // 忽略大小写敏感的比较
    int         Compare         (const _TCh* pStr, size_t count) const;
    int         CompareNoCase   (const _TCh* pStr, size_t count) const;
    b32         BeginsWith      (const _TCh c) const;                     // 检查是否以 c 开头？
    b32         BeginsWith      (const _TCh* pStr) const;                 // 检查是否以 pStr 中的字符串开头？
    b32         BeginsWith      (const _XCh* pStr) const;                 // 同上，只是另一个字符集的字符
    b32         EndsWith        (const _TCh c) const;
    b32         EndsWith        (const _TCh* pStr) const;                 // 检查是否以 pStr 字符串结尾
    b32         EndsWith        (const _XCh* pStr) const;

    size_t      Insert          (size_t idx, _TCh cCh);                   // 在指定位置插入字符
    size_t      Insert          (size_t idx, _TCh cCh, size_t count);     // 在指定位置插入N个字符
    size_t      Insert          (size_t idx, const _TCh* pStr);           // 在指定位置插入字符串

    size_t      Remove          (_TCh cCh);                   // 移除字符串中指定字符
    size_t      Remove          (size_t idx, size_t uCount);  // 移除字符串中某一串字符，返回值是剩余长度
    void        TrimLeft        (_TCh cTarget);               // 如果字符串开头有连续的特定字符，则裁掉
    void        TrimLeft        (const _TCh* pTarget);        // 裁剪以pTarget列表中为开头的字符
    void        TrimRight       (_TCh cTarget);               // 裁剪结尾
    void        TrimRight       (const _TCh* pTarget);
    void        TrimBoth        (_TCh cTarget);               // 修剪两端的字符, 如果两端存在连续的cTarget将都会被修剪掉
    void        Augment         (const _TCh* szLeft, const _TCh* szRight);   // 扩充两端

    b32         IsEmpty         () const; // 判断为空（完全等价于" ! IsNotEmpty()"）
    b32         IsNotEmpty      () const; // 判断不为空，吼吼

    b32         IsFloat         () const; // 以'+'or'-'开头，'0'-'9',只出现一次的'.'，'F'或'f'结尾的形式，暂时不支持"1e2"这种科学计数形式
    b32         IsInteger       () const; // 判断是否是整数形式的字符串，以'+'or'-'开头，后面全是数字
    b32         IsAlphanumeric  () const; // 判断字符串是否只包含['a'-'z', 'A'-'Z', '0'-'9', '_']这些字符
    b32         IsIdentifier    () const; // 判断字符串是否为标识符: 以['a'-'z', 'A'-'Z', '_'], 包含['a'-'z', 'A'-'Z', '0'-'9', '_']的字符串
    void        Clear           ();       // 清除字符串内容

    size_t      Find            (_TCh cFind, size_t uStart = 0) const;
    size_t      Find            (const _TCh* pFind, size_t uStart = 0) const;
    size_t      FindAny         (const _TCh* pCharList, size_t uStart = 0) const; // 字符串字母顺序优先查找
    size_t      FindAnyFromList (const _TCh* pCharList, size_t uStart = 0) const; // 列表字母顺序优先查找
    size_t      ReverseFind     (_TCh cFind) const;
    size_t      ReverseFind     (_TCh cFind, int nStart, int nEnd) const;
    size_t      ReverseFindAny  (const _TCh* pFindCharList) const;                // 字符串顺序反向查找
    StringX     Left            (size_t uCount) const;                            // 将前N个字符生成一个新字符串
    StringX     Right           (size_t uCount) const;                            // 将后N个字符生成一个新字符串
    StringX     SubString       (size_t uStart, size_t uCount) const;             // 将指定的某段儿生成一个新字符串
    size_t      DivideBy        (_TCh cCh, StringX& strFront, StringX& strBack) const; // 查找特定字符并切为前后两段
    size_t      ReverseDivideBy (_TCh cCh, StringX& strFront, StringX& strBack) const; // 从结尾开始搜索特定字符并切为前后两段

    b32         ToBoolean       () const;
    i32         ToInteger       (int nRadix = 10) const;  // 尝试将字符串转换为有符号整数，参数描述这个字符串的进制
    u32         ToUInteger      (int nRadix = 10) const;  // 尝试将字符串转换为无符号整数，参数描述这个字符串的进制
    i64         ToInteger64     (int nRadix = 10) const;  // 尝试将字符串转换为64位有符号整数，参数描述这个字符串的进制
    u64         ToUInteger64    (int nRadix = 10) const;  // 尝试将字符串转换为64位无符号整数，参数描述这个字符串的进制
    double      ToFloat         () const;                 // 尝试转换为浮点数

    u32         GetHash         () const; // 计算字符串的hash，这个算法是独立且平台一致的
    u32         GetCRC32        () const; // 计算字符串的CRC32
    size_t      VarFormat       (const _TCh *pFmt, va_list arglist);  // 在原始内容后面，使用变参列表追加格式化字符串
  }; // class StringX

  struct StringW_traits
  {
    typedef ch _XCh;
    static clsize     StringLength        (const wch* pStr);
    static clsize     XStringLength       (const _XCh* pStrX);
    static wch*       CopyStringN         (wch* pStrDest, const wch* pStrSrc, size_t uCopyLength);
    static i32        CompareString       (const wch* pStr1, const wch* pStr2);
    static i32        CompareStringNoCase (const wch* pStr1, const wch* pStr2);
    static const wch* StringSearchChar    (const wch* pStr, wch cCh);
    static void       Unsigned32ToString  (wch* pDestStr, size_t uMaxLength, u32 uNum, i32 nNumGroup);
    static void       Integer32ToString   (wch* pDestStr, size_t uMaxLength, i32 iNum, i32 nNumGroup);
    static i32        StringToInteger32   (wch* pString);
    static void       Unsigned64ToString  (wch* pDestStr, size_t uMaxLength, u64 uNum, i32 nNumGroup);
    static void       Integer64ToString   (wch* pDestStr, size_t uMaxLength, i64 iNum, i32 nNumGroup);
    static int        FloatToString       (wch* pDestStr, size_t uMaxLength, size_t precision, float fNum, char mode);
    static void       HexToLowerString    (wch* pDestStr, size_t uMaxLength, u32 uValue);
    static void       HexToUpperString    (wch* pDestStr, size_t uMaxLength, u32 uValue);
    static void       BinaryToString      (wch* pDestStr, size_t uMaxLength, u32 uValue);
    static void       OctalToString       (wch* pDestStr, size_t uMaxLength, u32 uValue);
    static size_t     XStringToNative     (wch* pNativeStr, size_t uLength, const _XCh* pStrX, size_t cchX);
  };

  struct StringA_traits
  {
    typedef wch _XCh;
    static clsize     StringLength        (const ch* pStr);
    static clsize     XStringLength       (const _XCh* pStrX);
    static ch*        CopyStringN         (ch* pStrDest, const ch* pStrSrc, size_t uCopyLength);
    static const ch*  StringSearchChar    (const ch* pStr, ch cCh);
    static i32        CompareString       (const ch* pStr1, const ch* pStr2);
    static i32        CompareStringNoCase (const ch* pStr1, const ch* pStr2);
    static void       Unsigned32ToString  (ch* pDestStr, size_t uMaxLength, u32 uNum, i32 nNumGroup);
    static void       Integer32ToString   (ch* pDestStr, size_t uMaxLength, i32 iNum, i32 nNumGroup);
    static i32        StringToInteger32   (ch* pString);
    static void       Unsigned64ToString  (ch* pDestStr, size_t uMaxLength, u64 uNum, i32 nNumGroup);
    static void       Integer64ToString   (ch* pDestStr, size_t uMaxLength, i64 iNum, i32 nNumGroup);
    static int        FloatToString       (ch* pDestStr, size_t uMaxLength, size_t precision, float fNum, char mode);
    static void       HexToLowerString    (ch* pDestStr, size_t uMaxLength, u32 uValue);
    static void       HexToUpperString    (ch* pDestStr, size_t uMaxLength, u32 uValue);
    static void       BinaryToString      (ch* pDestStr, size_t uMaxLength, u32 uValue);
    static void       OctalToString       (ch* pDestStr, size_t uMaxLength, u32 uValue);
    static size_t     XStringToNative     (ch* pNativeStr, size_t uLength, const _XCh* pStrX, size_t cchX);
  };

  //inline size_t hash_value(const clstd::StringX<wch, clstd::Allocator, g_Alloc_clStringW, clstd::StringW_traits>& _Str)
  //{
  //  return _Str.GetHash();
  //}

  //inline size_t hash_value(const  clstd::StringX<ch, clstd::Allocator, g_Alloc_clStringA, clstd::StringA_traits>& _Str)
  //{
  //  return _Str.GetHash();
  //}

  inline size_t hash_value(const clstd::StringX<wch, clstd::StdAllocator, g_StdAlloc, clstd::StringW_traits>& _Str)
  {
    return _Str.GetHash();
  }

  inline size_t hash_value(const  clstd::StringX<ch, clstd::StdAllocator, g_StdAlloc, clstd::StringA_traits>& _Str)
  {
    return _Str.GetHash();
  }

  //////////////////////////////////////////////////////////////////////////

  namespace StringUtility
  {
    // 按照ch分割分解字符串, 不会跳过空白
    template<typename _TCh, class _Fn>
    size_t Resolve(const _TCh* str, size_t len, _TCh ch, _Fn fn) // fn(size_t index, const _TCh* str, size_t len)
    {
      if( ! len) { return 0; }

      const _TCh* str_begin = str;
      const _TCh* str_end   = str + len;
      size_t index = 0;

      while(str < str_end) {
        if(*str == ch) {
          fn(index++, str_begin, str - str_begin);
          str_begin = ++str;
          continue;
        }
        str++;
      }

      fn(index++, str_begin, str - str_begin);
      return index;
    }

    // 按照ch分割分解字符串
    template<typename _TStr, class _Fn>
    size_t Resolve(const _TStr& str, typename _TStr::TChar ch, _Fn fn) // // fn(size_t index, const _TCh* str, size_t len)
    {
      return Resolve((typename _TStr::LPCSTR)str, str.GetLength(), ch, fn);
    }

  } // namespace StringUtility

} // namespace clstd

int SimpleASCIItoUnicode(wch* pDestStr, int nCount, const ch* pSrcStr);
int SimpleUnicodeToASCII(ch* pDestStr, int nCount, const wch* pSrcStr);

typedef clstd::StringX<wch, clstd::StdAllocator, g_StdAlloc, clstd::StringW_traits> clStringW;
typedef clstd::StringX<ch, clstd::StdAllocator,  g_StdAlloc, clstd::StringA_traits> clStringA;
typedef clstd::StringX<wch, clstd::StdAllocator, g_StdAlloc, clstd::StringW_traits> clTextBufferW;
typedef clstd::StringX<ch, clstd::StdAllocator,  g_StdAlloc, clstd::StringA_traits> clTextBufferA;

//inline size_t hash_value(const clStringW& _Str)
//{
//  return _Str.GetHash();
//}
//
//inline size_t hash_value(const clStringA& _Str)
//{
//  return _Str.GetHash();
//}

// gcc stlport
namespace std
{
  template<> struct hash<clStringA>
  {
    size_t operator()(const clStringA& _Str) const { return _Str.GetHash(); }
  };

  template<> struct hash<clStringW>
  {
    size_t operator()(const clStringW& _Str) const { return _Str.GetHash(); }
  };
}

//#ifdef _UNICODE
//typedef clStringW clString;
//#else
//typedef clStringA clString;
//#endif // _UNICODE
typedef const clStringW clConstStringW;
typedef const clStringA clConstStringA;
//typedef const clString clConstString;

typedef clvector<clStringW>   clStringArrayW;
typedef clvector<clStringA>   clStringArrayA;
typedef cllist<clStringW>     clStringListW;
typedef cllist<clStringA>     clStringListA;
typedef clhash_set<clStringW> clStringHashSetW;
typedef clhash_set<clStringA> clStringHashSetA;
//typedef clset<clStringW>      clStringSetW;
//typedef clset<clStringA>      clStringSetA;

clStringW AnsiStringToUnicodeString(const clStringA& str);
clStringA UnicodeStringToAnsiString(const clStringW& str);

//#ifdef _UNICODE
//#define AS2TS(STR)  (const wch*)AnsiStringToUnicodeString(STR)
//#define WS2TS(STR)  clStringW(STR)
//#define TS2AS(STR)  (const ch*)UnicodeStringToAnsiString(STR)
//#define TS2WS(STR)  clStringW(STR)
//typedef clStringArrayW clStringArray;
//typedef clStringListW  clStringList;
//#else
//#define AS2TS(STR)  clStringA(STR)
//#define WS2TS(STR)  (const ch*)UnicodeStringToAnsiString(STR)
//#define TS2AS(STR)  clStringA(STR)
//#define TS2WS(STR)  (const wch*)AnsiStringToUnicodeString(STR)
//typedef clStringArrayA clStringArray;
//typedef clStringListA  clStringList;
//#endif

#define AS2WS(STR)  (const wch*)AnsiStringToUnicodeString(STR)
#define WS2AS(STR)  (const ch*)UnicodeStringToAnsiString(STR)

namespace clstd
{
  namespace StringUtility
  {
    // ConvertToUtf8，ConvertFromUtf8 转换不清除目标中的原始内容
    clStringA& ConvertToUtf8(clStringA& strUtf8, const clStringW& strUnicode);
    clStringA& ConvertToUtf8(clStringA& strUtf8, const wch* szUnicode, size_t nUnicode);
    clStringW& ConvertFromUtf8(clStringW& strUnicode, const clStringA& strUtf8);
    clStringW& ConvertFromUtf8(clStringW& strUnicode, const ch* szUtf8, size_t nUtf8);

    // strDestination 与 strSource 可以是相同对象
    clStringW& ExpandEnvironmentStringsFromSet(clStringW& strDestination, const clStringW& strSource, const clmap<clStringW, clStringW>& dict, clStringW::LPCSTR szTranslateBegin, clStringW::LPCSTR szTranslateEnd);
    clStringA& ExpandEnvironmentStringsFromSet(clStringA& strDestination, const clStringA& strSource, const clmap<clStringA, clStringA>& dict, clStringA::LPCSTR szTranslateBegin, clStringA::LPCSTR szTranslateEnd);
    clStringW& ExpandEnvironmentStringsFromSet(clStringW& strDestination, const clStringW& strSource, const clhash_map<clStringW, clStringW>& dict, clStringW::LPCSTR szTranslateBegin, clStringW::LPCSTR szTranslateEnd);
    clStringA& ExpandEnvironmentStringsFromSet(clStringA& strDestination, const clStringA& strSource, const clhash_map<clStringA, clStringA>& dict, clStringA::LPCSTR szTranslateBegin, clStringA::LPCSTR szTranslateEnd);

  } // namespace StringUtility
} // namespace clstd

#else
# pragma message(__FILE__": warning : Duplicate included this file.")
#endif // _CLSTD_STRING_H_