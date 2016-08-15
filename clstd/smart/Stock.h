#ifndef _CLSTD_STOCK_H_
#define _CLSTD_STOCK_H_

// 重新设计的新接口 Stock（敏捷托盘），用来储存用户数据文件和数据格式
// 设计目的是为了替换旧的SmartProfile类
// 废弃SmartProfile主要原因是， SmartProfile中很多接口定义不清晰，有点混乱
// 结构解析仍然采用逐步解析方式，用到的才解析

// 这个接口的特点：
// 1.解析速度快，渐进式解析。
// 2.结构简单，使用方便。
// 3.访问内容时不需要分配额外内存，不调用new操作。

#ifndef _SMARTSTREAM_2_H_
#error Must be include "smartstream.h" first.
#endif // _SMARTSTREAM_2_H_


namespace clstd
{
  class Buffer;

  //////////////////////////////////////////////////////////////////////////

  template<class _TStr>
  class StockT
  {
  public:
    //typedef typename _Traits::_TCh _TCh;
    typedef SmartStreamT<_TStr> _SmartStreamT;
    typedef typename _SmartStreamT::iterator _MyIterator;
    typedef typename _TStr::LPCSTR T_LPCSTR;
    typedef typename _TStr::TChar  TChar;

  public:
    //struct SECTION;
    class Section;
    struct ATTRIBUTE
    {
      typedef void (*unspecified_bool_type)(ATTRIBUTE***);
      const Section* pSection;
      _MyIterator   itKey;   // 键
      _MyIterator   itValue; // 值

      ATTRIBUTE(){}
      ATTRIBUTE(Section* pCoSection) : pSection(pCoSection){}

      b32     empty       () const;

      b32     NextKey     ();
      _TStr   SectionName () const;
      _TStr   KeyName     () const;
      int     ToInt       () const;
      _TStr   ToString    () const;
      float   ToFloat     () const;
      b32     ToBoolean   () const;
      _TStr&  KeyName     (_TStr& str) const;
      _TStr&  ToString    (_TStr& str) const;
      //void    SetValue    (T_LPCSTR str);

                  operator unspecified_bool_type() const;
      b32         operator==(const ATTRIBUTE& attr) const;
      b32         operator!=(const ATTRIBUTE& attr) const;
      ATTRIBUTE&  operator++();

      //T_LPCSTR      operator=(T_LPCSTR str);
      //int           operator=(int val);
      //float         operator=(float val);
      //unsigned int  operator=(unsigned int val);
    };

    //////////////////////////////////////////////////////////////////////////

    class Section
    {
      friend class StockT;
      typedef void (*unspecified_bool_type)(Section***);
    protected:
      StockT*       pStock;
      Section*      pParent;  // 父section
      int           nModify;  // 修改次数，如果与Stock不一致，则说明这个section已经过期，不再可用
      int           nDepth;   // 所在深度，用于文本对齐. 根是0, 如果是<0，说明这个Section已经失效

    public:
      _MyIterator   itSectionName;
      _MyIterator   itBegin;  // Section开始的'{'位置
      _MyIterator   itEnd;    // Section结束的'}'位置

      //SECTION* m_desc;
    protected:
#ifdef _DEBUG
      b32 DbgCheck() const
      {
        return ( empty() ) || nDepth == 0 || ((itBegin.marker[0] == '{' || itBegin == itBegin.pContainer->begin()) &&
          (itEnd.marker[0] == '}' || itEnd == itBegin.pContainer->end()));
      }
#else
      b32 DbgCheck() const { return TRUE; }
#endif


    public:
      Section();
      //Section(SECTION* desc);
      Section(StockT* pCoStock);
      ~Section();

      //StockT*   GetStock            () const
      //{
      //  return pStock;
      //}
      b32       empty               () const;
      void      clear               ();

      _TStr     SectionName         () const;
      Section   Open                (T_LPCSTR szSubPath) const; // 等价于StockT::Open()
      Section   Create              (T_LPCSTR szSubPath); // 等价于StockT::Create()
      b32       NextSection         (T_LPCSTR szName = NULL);
      b32       Rename              (T_LPCSTR szNewName);
      b32       FirstKey            (ATTRIBUTE& param) const;

      b32       GetKey              (T_LPCSTR szKey, ATTRIBUTE& param) const;
      int       GetKeyAsString      (T_LPCSTR szKey, T_LPCSTR szDefault, TChar* szBuffer, int nCount) const;
      _TStr     GetKeyAsString      (T_LPCSTR szKey, const _TStr& strDefault) const;
      int       GetKeyAsInteger     (T_LPCSTR szKey, int nDefault) const;
      float     GetKeyAsFloat       (T_LPCSTR szKey, float fDefault) const;
      b32       GetKeyAsBoolean     (T_LPCSTR szKey, b32 bDefault) const;
      b32       SetKey              (T_LPCSTR szKey, T_LPCSTR val);
      b32       SetKey              (T_LPCSTR szKey, int val);
      b32       SetKey              (T_LPCSTR szKey, size_t val);
      b32       SetKey              (T_LPCSTR szKey, float val);
      b32       SetKey              (T_LPCSTR szKey, b32 bValue, T_LPCSTR szTrue, T_LPCSTR szFalse);
      b32       DeleteKey           (T_LPCSTR szKey);
      b32       InsertKey           (T_LPCSTR szKey, T_LPCSTR val); // SetKey 替换原来键值，InsertKey 可以插入新的键值

      void      operator++();
      ATTRIBUTE operator[](T_LPCSTR name) const;
                operator unspecified_bool_type() const;


      ATTRIBUTE begin ();
      ATTRIBUTE end   ();
    };

  protected:
    //typedef clvector<SECTION*>  SectionArray;

    _SmartStreamT m_SmartStream;
    Buffer        m_Buffer;
    int           m_nModify;
    //SectionArray  m_aHandles;

    static void ReverseByteOrder16(u16* ptr, clsize nCount);

  public:
    StockT();
    virtual ~StockT();

    b32 LoadA(const ch* lpProfile);
    b32 SaveA(const ch* lpProfile) const;
    b32 LoadW(const wch* lpProfile);
    b32 SaveW(const wch* lpProfile) const;

    b32 Attach(BufferBase* pBuffer); // 内部会复制一份

    b32 Close();

    //////////////////////////////////////////////////////////////////////////


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
    Section Create(T_LPCSTR szPath);
    Section Create(Section* desc, T_LPCSTR szSubPath); // 等价于SECTION::Create

    //************************************
    // Method:    Open 打开指定的Section
    // Qualifier:
    // Parameter: _TCh * szPath
    // 打开Section的路径名，如"sect1/sect0"或者"sect"
    // 注意：不再使用时需要用CloseSection关闭
    //************************************
    Section Open(T_LPCSTR szPath) const;
    Section Open(Section* desc, T_LPCSTR szSubPath) const; // 等价于SECTION::Open

    //************************************
    // Method:    DeleteSection 删除指定的Section
    // Returns:   b32 删除成功返回true，如果指定的Section不存在则返回false
    // Qualifier:
    // Parameter: _TCh * szPath
    // Section的路径名，如果是"sect"这种形式，则直接删除它及所包含的子Section
    // 如果是"sect3/sect2/sect1/sect0"，则删除的是"sect3/sect2/sect1"路径下的"sect0"
    //************************************
    b32 DeleteSection(T_LPCSTR szPath);
    // b32 DeleteSection(Section* desc); // TODO: 没实现

    T_LPCSTR GetText(clsize* length) const;

    // std
    //begin();
    //end();

  protected:
    //b32      Append            (Section* pSect, T_LPCSTR szText, clsize nCount);
    //b32      Insert            (Section* pSect, clsize nPos, T_LPCSTR szText, clsize nCount);
    b32      ToNativeCodec     (); // 根据BOM转为本地编码，StockA为ANSI，StockW为Unicode-16
    b32      Replace           (Section* pSect, clsize nPos, clsize nReplaced, T_LPCSTR szText, clsize nCount);
    b32      FindSingleSection (Section* pFindSect, T_LPCSTR szName, Section& pOutSect) const; // szName为NULL表示查找任何Section;
    b32      NewSection        (Section* pSection, T_LPCSTR szName, Section& pNewSect);
    clsize   InsertString      (Section* pSect, const _MyIterator& it, const _TStr& str);

    b32      Remove            (Section* pSect, const _MyIterator& itBegin, const _MyIterator& itEnd);
    //SECTION* AddSection        (SECTION* desc);
    //b32      RemoveSection     (SECTION* desc);
    b32      RelocateIterator  (_MyIterator& it, T_LPCSTR lpOldPtr, T_LPCSTR lpNewPtr, clsize uActPos, clsize sizeReplaced, clsize sizeInsert);
    b32      RelocateSection   (Section* pSection, T_LPCSTR lpOldPtr, T_LPCSTR lpNewPtr, clsize uActPos, clsize sizeReplaced, clsize sizeInsert);
    void     TrimFrontTab      (clsize& uOffset);
  };

  class StockA : public StockT<clStringA> {};
  class StockW : public StockT<clStringW> {};
} // namespace clstd



#endif // _CLSTD_STOCK_H_