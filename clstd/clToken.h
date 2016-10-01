#ifndef _CLSTD_TOKEN_H_
#define _CLSTD_TOKEN_H_

namespace clstd {

  template<class _TStr>
  class TokenT
  {
  public:
    //enum MARK
    //{
    //  M_ESCAPE      = 0x0800,  // ת���
    //  M_SYMBOL      = 0x0000,  // ����
    //  M_GAP         = 0x0001,  // �հ�
    //  M_QUOT        = 0x0002,  // ����
    //  M_OPN_BRAKETS = 0x0004,  // ������
    //  M_CLS_BRAKETS = 0x0008,  // ������
    //  M_LABEL       = 0x0010,  // ��ǩ
    //  M_TYPE_MASK   = 0x001F,  // ��������
    //  M_CALLBACK    = 0x0020,  // �����ص�, ������ iterator �����ֽ�
    //  M_GROUP_MASK  = 0xF000,  // ������, ֻ��������������
    //};

    enum FLAGS
    {
      F_SYMBOLBREAK = 0x0001,    // �п� Symbol, ���������� Symbol ���ֳɶ������ַ��ֱ𷵻�.
    };

    typedef typename _TStr::LPCSTR T_LPCSTR;
    typedef typename _TStr::TChar  TChar;
    //typedef u32                    SemType;

    struct iterator
    {
    public:
      const TokenT*   pContainer;
      T_LPCSTR        marker;
      clsize          length;

      iterator()
        : pContainer(NULL)
        , marker    (NULL)
        , length    (0){}

      iterator(const TokenT*  _pContainer) 
        : pContainer(_pContainer)
        , marker    (_pContainer->m_pBegin)
        , length    (0){}

      iterator(const iterator& it)
        : pContainer(it.pContainer)
        , marker    (it.marker)
        , length    (it.length){}

      iterator&  operator++();
      iterator&  operator++(int);
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
      _TStr   ToRawString () const;   // ����ԭʼ�ַ���
      _TStr   ToString    () const;   // �����ַ���������ַ������е����Ż���˫���ţ��ᱻȥ����
      b32     BeginsWith  (TChar ch) const;
      b32     EndsWith    (TChar ch) const;
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
    typedef size_t (CALLBACK *IteratorProc)(iterator& it, clsize nRemain, u32_ptr lParam); // nRemain ��it.marker[0]���ļ���β��(ʣ��)����. Ŀǰ�����ķ���ֵ
    typedef const iterator const_iterator; 

  protected:
    static const u32     c_nCharTabCount = 128;
    T_LPCSTR      m_pBegin;
    T_LPCSTR      m_pEnd;
    //SemType       m_aCharSem[c_nCharTabCount];
    u32           m_dwFlags;
    iterator      m_itEnd;
    IteratorProc  m_pCallBack;        // ���ÿһ�� Iterator
    u32_ptr       m_lParam;
    //IteratorProc  m_pTriggerCallBack; // ���д�����־�� Iterator ���ֽ�
    u32_ptr       m_lParamTrigger;

  public:
    TokenT(T_LPCSTR pStream = NULL, clsize uCountOfChar = 0);

    b32      Initialize        (T_LPCSTR pStream, clsize uCountOfChar);
    //void     GetCharSemantic   (SemType* pCharSem, clsize uStart, clsize uEnd) const;
    //void     SetCharSemantic   (const SemType* pCharSem, clsize uStart, clsize uEnd);
    //SemType  GetCharSemantic   (TChar ch) const;
    //SemType  SetCharSemantic   (TChar ch, SemType flagCharSem);
    b32      IsEndOfStream     (T_LPCSTR pPointer) const;
    b32      IsHeadOfStream    (T_LPCSTR pPointer) const;
    T_LPCSTR GetStreamPtr      () const;
    clsize   GetStreamCount    () const; // ���ص���TChar�����ͳ��ȣ������ֽ��� GetStreamEnd() - GetStreamPtr()
    b32      Get               (T_LPCSTR pPointer, T_LPCSTR* ppOutPointer, clsize* pOutCount) const;    // ȡָ�뵱ǰ��������
    u32      SetFlags          (u32 dwFlags);
    u32      GetFlags          () const;

    IteratorProc SetIteratorCallBack(IteratorProc pNew, u32_ptr lParam);
    //IteratorProc SetTriggerCallBack (IteratorProc pTrigger, u32_ptr lParam);

    iterator&       next      (iterator& it) const;
    iterator        nearest   (clsize nOffset) const; // ����nOffset���������iterator��iterator��ƫ�ƴ��ڵ���nOffset
    iterator        begin     () const;
    const_iterator& end       () const;
    const_iterator  find      (const iterator& itBegin, int nCount, ...) const; // �� itBegin ��ʼ����, ����ҵ������б������һ�������Ϸ���
    b32             find_pair (const iterator& itCurrent, iterator& itOpen, iterator& itClose, T_LPCSTR chOpen, T_LPCSTR chClose) const;
  };

} // namespace clstd

#else
# pragma message(__FILE__": warning : Duplicate included this file.")
#endif // _CLSTD_TOKEN_H_