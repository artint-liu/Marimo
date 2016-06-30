#ifndef _CLSTD_SMART_STOCK_H_
#define _CLSTD_SMART_STOCK_H_

// 重新设计的新接口 SmartStock（敏捷托盘），用来储存用户数据文件和数据格式
// 设计目的是为了替换旧的SmartProfile类
// 废弃SmartProfile主要原因是， SmartProfile中很多接口定义不清晰，有点混乱
// 结构解析仍然采用逐步解析方式，用到的才解析

#ifndef _SMARTSTREAM_2_H_
#error Must be include "smartstream.h" first.
#endif // _SMARTSTREAM_2_H_


namespace clstd
{
  class Buffer;

  //struct SmartStock_TraitsW
  //{
  //  //static clsize _StrLen(const wch*);
  //  static b32    _CheckBoolean(const wch*);
  //  typedef wch _TCh;
  //  typedef SmartStream_TraitsW SmartStream_Traits;
  //};

  //struct SmartStock_TraitsA
  //{
  //  //static clsize _StrLen(const ch*);
  //  static b32    _CheckBoolean(const ch*);
  //  typedef ch _TCh;
  //  typedef SmartStream_TraitsA SmartStream_Traits;
  //};

  //template <typename _TCh> b32 _CheckBoolean(const _TCh*);

  //////////////////////////////////////////////////////////////////////////

  template<class _TStr>
  class SmartStockT
  {
  public:
    //typedef typename _Traits::_TCh _TCh;
    typedef SmartStreamT<_TStr> _SmartStreamT;
    typedef typename _SmartStreamT::iterator _MyIterator;
    typedef typename _TStr::LPCSTR T_LPCSTR;
    typedef typename _TStr::TChar  TChar;

  public:
    struct SECTION;
    struct ATTRIBUTE
    {
      const SECTION* pSection;
      _MyIterator   itKey;   // 键
      _MyIterator   itValue; // 值

      ATTRIBUTE(){}
      ATTRIBUTE(SECTION* pCoSection) : pSection(pCoSection){}

      b32     NextKey     ();
      _TStr   SectionName () const;
      _TStr   KeyName     () const;
      int     ToInt       () const;
      _TStr   ToString    () const;
      float   ToFloat     () const;
      b32     ToBoolean   () const;
      _TStr&  KeyName     (_TStr& str) const;
      _TStr&  ToString    (_TStr& str) const;

      b32         operator==(const ATTRIBUTE& attr) const;
      b32         operator!=(const ATTRIBUTE& attr) const;
      ATTRIBUTE&  operator++();
    };

    //////////////////////////////////////////////////////////////////////////

    struct SECTION
    {
      SmartStockT*  pStock;
      int           nDepth;   // 所在深度，用于文本对齐. 根是0, 如果是<0，说明这个Section已经失效
      _MyIterator   itSectionName;
      _MyIterator   itBegin;  // Section开始的'{'位置
      _MyIterator   itEnd;    // Section结束的'}'位置

#ifdef _DEBUG
      b32 DbgCheck() const
      {
        return ( ! IsValid() ) || nDepth == 0 || ((itBegin.marker[0] == '{' || itBegin == itBegin.pContainer->begin()) &&
          (itEnd.marker[0] == '}' || itEnd == itBegin.pContainer->end()));
      }
#else
      b32 DbgCheck() const { return TRUE; }
#endif
      SECTION(){}
      SECTION(SmartStockT* pCoStock)
        : pStock  (pCoStock)
        , nDepth  (0)
        , itBegin(pCoStock->m_SmartStream.begin())
        , itEnd(pCoStock->m_SmartStream.end())
      {}

      b32       IsValid             () const;
      _TStr     SectionName         () const;
      SECTION*  Open                (T_LPCSTR szSubPath); // 等价于SmartStockT::Open()
      b32       NextSection         (T_LPCSTR szName);
      b32       Rename              (T_LPCSTR szNewName);
      b32       FirstKey            (ATTRIBUTE& param) const;

      b32       GetKey              (T_LPCSTR szKey, ATTRIBUTE& param) const;
      int       GetKeyAsString      (T_LPCSTR szKey, T_LPCSTR szDefault, TChar* szBuffer, int nCount) const;
      _TStr     GetKeyAsString      (T_LPCSTR szKey, const _TStr& strDefault) const;
      int       GetKeyAsInteger     (T_LPCSTR szKey, int nDefault) const;
      float     GetKeyAsFloat       (T_LPCSTR szKey, float fDefault) const;
      b32       GetKeyAsBoolean     (T_LPCSTR szKey, b32 bDefault) const;
      b32       SetKey              (T_LPCSTR szKey, T_LPCSTR szValue);
      b32       DeleteKey           (T_LPCSTR szKey);

      // std 适应接口
      ATTRIBUTE begin ();
      ATTRIBUTE end   ();
    };


    //
    // 用于自动处理SECTION_DESC*的类
    //
    class Section
    {
    protected:
      SECTION* m_desc;

    public:
      Section(SECTION* desc);
      ~Section();
      b32 IsValid() const;
      void Close();
      SECTION* operator=(SECTION* desc);
      SECTION* operator->() const;
      SECTION* operator&() const;
      SECTION& operator*() const;
      ATTRIBUTE operator[](T_LPCSTR name) const;
    };

  protected:
    typedef clvector<SECTION*>  SectionArray;

    _SmartStreamT m_SmartStream;
    Buffer*       m_pBuffer;
    SectionArray  m_aHandles;

    static void ReverseByteOrder16(u16* ptr, clsize nCount);

  public:
    SmartStockT();
    virtual ~SmartStockT();

    b32 LoadA(const ch* lpProfile);
    b32 SaveA(const ch* lpProfile) const;
    b32 LoadW(const wch* lpProfile);
    b32 SaveW(const wch* lpProfile) const;

    b32 Close();

    //////////////////////////////////////////////////////////////////////////

    b32 CloseSection(SECTION* desc);

    //************************************
    // Method:    Create 创建section
    // Qualifier:
    // Parameter: _TCh * szPath
    // szPath支持"sect3/sect2/sect1/sect0"和"sect"两种格式
    // 如果带有路径，sect3，sect2和sect1路径先执行查找功能，如果找不到则会创建
    // 最后的sect0会直接创建
    // 如果不带路径，则直接在根上创建sect
    // 注意：不再使用时需要用CloseSection关闭
    //************************************
    SECTION* Create(T_LPCSTR szPath);
    SECTION* Create(SECTION* desc, T_LPCSTR szSubPath);

    //************************************
    // Method:    Open 打开指定的Section
    // Qualifier:
    // Parameter: _TCh * szPath
    // 打开Section的路径名，如"sect1/sect0"或者"sect"
    // 注意：不再使用时需要用CloseSection关闭
    //************************************
    SECTION* Open(T_LPCSTR szPath);
    SECTION* Open(SECTION* desc, T_LPCSTR szSubPath); // 等价于SECTION::Open

    //************************************
    // Method:    DeleteSection 删除指定的Section
    // Returns:   b32 删除成功返回true，如果指定的Section不存在则返回false
    // Qualifier:
    // Parameter: _TCh * szPath
    // Section的路径名，如果是"sect"这种形式，则直接删除它及所包含的子Section
    // 如果是"sect3/sect2/sect1/sect0"，则删除的是"sect3/sect2/sect1"路径下的"sect0"
    //************************************
    b32 DeleteSection(T_LPCSTR szPath);

    T_LPCSTR GetText(clsize* length) const;

    // std
    //begin();
    //end();

  protected:
    b32      Append            (T_LPCSTR szText, clsize nCount);
    b32      Insert            (clsize nPos, T_LPCSTR szText, clsize nCount);
    b32      Replace           (clsize nPos, clsize nReplaced, T_LPCSTR szText, clsize nCount);
    b32      FindSigleSection  (const SECTION* pFindSect, T_LPCSTR szName, SECTION* pOutSect); // szName为NULL表示查找任何Section;
    b32      NewSection        (const SECTION* pSection, T_LPCSTR szName, SECTION* pNewSect);
    clsize   InsertString      (const _MyIterator& it, const _TStr& str);

    b32      Remove            (const _MyIterator& itBegin, const _MyIterator& itEnd);
    SECTION* AddSection        (SECTION* desc);
    b32      RemoveSection     (SECTION* desc);
    b32      RelocateIterator  (_MyIterator& it, T_LPCSTR lpOldPtr, T_LPCSTR lpNewPtr, clsize uActPos, clsize sizeReplaced, clsize sizeInsert);
    b32      RelocateSection   (T_LPCSTR lpOldPtr, T_LPCSTR lpNewPtr, clsize uActPos, clsize sizeReplaced, clsize sizeInsert);
    void     TrimFrontTab      (clsize& uOffset);
  };

  //template<class _STR>
  //_STR FromProfileString(const _STR& str);
  //
  //template<class _STR>
  //_STR ToProfileString(const _STR& str);

  class SmartStockA : public SmartStockT<clStringA> {};
  class SmartStockW : public SmartStockT<clStringW> {};
} // namespace clstd



#endif // _CLSTD_SMART_STOCK_H_