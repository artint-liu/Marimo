#ifndef _CLSTD_STRING_ATTACH_H_
#define _CLSTD_STRING_ATTACH_H_

namespace clstd
{
  template<typename _TCh>
  class StringAttachX
  {
  public:
    typedef _TCh TChar;
    typedef const TChar* LPCSTR;
    //typedef _Traits MyTraits;

  private:
    _TCh*   m_pBuf;         // 字符串缓冲
    size_t  m_nCount;       // 字符串长度，不包含'\0'，字符数
    size_t  m_nCapacity;    // 缓冲区容量长度，包含'\0'，字符数
    void* const   m_pAttached;    // 外部附加缓冲区，如果外部附加缓冲区有效，并且字符串没有超出尺寸，则优先使用这个
    const size_t  m_cbAttached;   // 附加缓冲区尺寸，字节数

    // 外部附加缓冲区在类构造后不会改变，这样保证了类的生命周期小于等于外部缓冲区的周期

  private:
    void _Resize(size_t count);

  public:
    StringAttachX();
    StringAttachX(void* pAttachedBuffer, size_t cbAttached); // 缓冲区，字节长度
    StringAttachX(void* pAttachedBuffer, size_t cbAttached, size_t nLength); // 缓冲区，字节长度，初始化字符长度
    ~StringAttachX();

    bool operator==(LPCSTR szStr) const;

    const _TCh* CStr() const;
    size_t GetLength() const;
    b32 IsEmpty() const;
    b32 IsNotEmpty() const;
    b32 IsAttached() const;

    StringAttachX& Clear();
    StringAttachX& Reserve(size_t count);
    
    StringAttachX& VarFormat(const _TCh *pFmt, va_list arglist);
    StringAttachX& Format(const _TCh *pFmt, ...);
    StringAttachX& AppendFormat(const _TCh *pFmt, ...);

    StringAttachX& Append(_TCh c);
    StringAttachX& Append(_TCh c, size_t count);
    StringAttachX& Append(const _TCh* pStr);
    StringAttachX& Append(const _TCh* pStr, size_t count);
    StringAttachX& Append(const _TCh* pStr, _TCh cFill, long nWidth);

    int Compare(LPCSTR szStr) const;
    int Compare(LPCSTR szStr, size_t count) const;
  };

} // namespace clstd

typedef clstd::StringAttachX<wch> clStringAttachW;
typedef clstd::StringAttachX<ch> clStringAttachA;

namespace clstd
{
  namespace StringUtility
  {
    clStringAttachA& ConvertToUtf8(clStringAttachA& strUtf8, const clStringAttachW& strUnicode);
    clStringAttachA& ConvertToUtf8(clStringAttachA& strUtf8, const wch* szUnicode, size_t nUnicode);
    clStringAttachW& ConvertFromUtf8(clStringAttachW& strUnicode, const clStringAttachA& strUtf8);
    clStringAttachW& ConvertFromUtf8(clStringAttachW& strUnicode, const ch* szUtf8, size_t nUtf8);
  } // namespace StringUtility
} // namespace clstd


#endif // _CLSTD_STRING_ATTACH_H_