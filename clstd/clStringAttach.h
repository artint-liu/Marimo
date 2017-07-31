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
    _TCh*   m_pBuf;         // �ַ�������
    size_t  m_nCount;       // �ַ������ȣ�������'\0'���ַ���
    size_t  m_nCapacity;    // �������������ȣ�����'\0'���ַ���
    void* const   m_pAttached;    // �ⲿ���ӻ�����������ⲿ���ӻ�������Ч�������ַ���û�г����ߴ磬������ʹ�����
    const size_t  m_cbAttached;   // ���ӻ������ߴ磬�ֽ���

    // �ⲿ���ӻ��������๹��󲻻�ı䣬������֤�������������С�ڵ����ⲿ������������

  private:
    void _Resize(size_t count);

  public:
    StringAttachX();
    StringAttachX(void* pAttachedBuffer, size_t cbAttached); // ���������ֽڳ���
    StringAttachX(void* pAttachedBuffer, size_t cbAttached, size_t nLength); // ���������ֽڳ��ȣ���ʼ���ַ�����
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

#endif // _CLSTD_STRING_ATTACH_H_