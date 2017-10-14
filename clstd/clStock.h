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

//#ifndef _SMARTSTREAM_2_H_
//#error Must be include "smartstream.h" first.
//#endif // _SMARTSTREAM_2_H_

#ifndef _CLSTD_TOKEN_H_
#error Must be include "clTokens.h" first.
#endif // _CLSTD_TOKEN_H_


namespace clstd
{
  class MemBuffer;

  //////////////////////////////////////////////////////////////////////////
  // TODO: 增加 StockReaderT 类，作为只读方式加载
  // TODO: StockT 改善写入时 buffer 内存增长策略

  template<class _TStr>
  class StockT
  {
  public:
    //typedef typename _Traits::_TCh _TCh;
    typedef TokensT<_TStr> _TTokens;
    typedef typename _TTokens::iterator _MyIterator;
    typedef typename _TStr::LPCSTR T_LPCSTR;
    typedef typename _TStr::TChar  TChar;

    // Section::Query
    enum QueryType {
      QueryType_Default = 0,    // 如果查询失败，则返回一个无效的Section
      QueryType_FindAlways = 1, // 如果查询失败，会新建一个Section
    };


  public:
    //struct SECTION;
    class Section;
    struct ATTRIBUTE
    {
      typedef void (*unspecified_bool_type)(ATTRIBUTE***);
      const Section* pSection;
      _MyIterator   key;   // 键
      _MyIterator   value; // 值

      ATTRIBUTE(){}
      ATTRIBUTE(Section* pCoSection) : pSection(pCoSection){}
      ~ATTRIBUTE();

      b32     IsEmpty     () const;
      b32     NextKey     ();
      _TStr   SectionName () const;
      _TStr   KeyName     () const;
      int     ToInt       (int nDefault = 0) const;
      u32     ToUInt      (u32 nDefault = 0) const;
      _TStr   ToString    (T_LPCSTR szDefault = NULL) const;
      float   ToFloat     (float fDefault = 0.0f) const;
      b32     ToBoolean   (b32 bDefault = false) const; // 如果不存在则返回默认值，如果是“1”,“OK”,“yes”,"true"（不区分大小写）返回TRUE，否则返回false
      _TStr&  KeyName     (_TStr& str) const;
      _TStr&  ToString    (_TStr& str, T_LPCSTR szDefault = NULL) const;

      template<class _TFn>
      size_t ToArray(_TFn fn, TChar ch = TChar(',')) const // void fn(size_t index, const TChar* begin, size_t length)
      {
        _TStr strValue;
        return StringUtility::Resolve(ToString(strValue), ch, fn);
      }
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
      size_t        nModify;  // 修改次数，如果与Stock不一致，则说明这个section已经过期，不再可用
      int           nDepth;   // 所在深度，用于文本对齐. 根是0, 如果是<0，说明这个Section已经失效

    public:
      _MyIterator   name;        // Section Name
      _MyIterator   iter_begin;  // Section开始的'{'位置
      _MyIterator   iter_end;    // Section结束的'}'位置

      //SECTION* m_desc;
    protected:
#ifdef _DEBUG
      b32 DbgCheck() const
      {
        return ( empty() ) || nDepth == 0 || ((iter_begin.marker[0] == '{' || iter_begin == iter_begin.pContainer->begin()) &&
          (iter_end.marker[0] == '}' || iter_end == iter_begin.pContainer->end()));
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
      b32       empty               () const; // TODO: IsEmpty()
      void      clear               ();

      StockT*   GetStock            () const;
      _TStr     SectionName         () const;
      Section   Open                (T_LPCSTR szSubPath) const; // 等价于StockT::OpenSection()
      Section   Create              (T_LPCSTR szSubPath); // 等价于StockT::CreateSection()
      Section   Query               (T_LPCSTR szSubSection, T_LPCSTR szMainKey, T_LPCSTR szMatchValue, QueryType eType = QueryType_Default);
      void      Delete              ();
      b32       NextSection         (T_LPCSTR szName = NULL); // 如果失败，表示没有后续Section，不会改变当前section内容，这与operator++行为不同
      b32       Rename              (T_LPCSTR szNewName);
      b32       FirstKey            (ATTRIBUTE& param) const;
      ATTRIBUTE FirstKey            () const;

      b32       GetKey              (T_LPCSTR szKey, ATTRIBUTE& param) const;
      int       GetKeyAsString      (T_LPCSTR szKey, T_LPCSTR szDefault, TChar* szBuffer, int nCount) const;
      _TStr     GetKeyAsString      (T_LPCSTR szKey, const _TStr& strDefault) const;
      int       GetKeyAsInteger     (T_LPCSTR szKey, int nDefault) const;
      float     GetKeyAsFloat       (T_LPCSTR szKey, float fDefault) const;
      b32       GetKeyAsBoolean     (T_LPCSTR szKey, b32 bDefault) const;
      b32       SetKey              (T_LPCSTR szKey, T_LPCSTR val);
      b32       SetKey              (T_LPCSTR szKey, int val);
      b32       SetKey              (T_LPCSTR szKey, u32 val);
      b32       SetKey              (T_LPCSTR szKey, u64 val);
      b32       SetKey              (T_LPCSTR szKey, float val);
      b32       SetKey              (T_LPCSTR szKey, b32 bValue, T_LPCSTR szTrue, T_LPCSTR szFalse);

      // 比较并设置键值，如果值已经存在并且与原来值一直，则不更改数据，目的是为了避免修改Modification值
      b32       ComparedSetKey      (T_LPCSTR szKey, T_LPCSTR val);
      b32       ComparedSetKey      (T_LPCSTR szKey, int val);
      b32       ComparedSetKey      (T_LPCSTR szKey, u32 val);
      b32       ComparedSetKey      (T_LPCSTR szKey, u64 val);
      b32       ComparedSetKey      (T_LPCSTR szKey, float val, char mode = 'F'); // F/E/R 模式，参考clString
      b32       ComparedSetKey      (T_LPCSTR szKey, b32 bValue, T_LPCSTR szTrue, T_LPCSTR szFalse);

      b32       DeleteKey           (T_LPCSTR szKey);
      b32       InsertKey           (T_LPCSTR szKey, T_LPCSTR val); // SetKey 替换原来键值，InsertKey 可以插入新的键值

      Section&  operator=(const Section& sect);
      void      operator++();
      ATTRIBUTE operator[](T_LPCSTR name) const;
                operator unspecified_bool_type() const;

    };

  protected:
    //typedef clvector<SECTION*>  SectionArray;

    _TTokens  m_SmartStream;
    MemBuffer m_Buffer;
    size_t    m_nModify;
    //SectionArray  m_aHandles;

    static void ReverseByteOrder16(u16* ptr, clsize nCount);

  public:
    StockT();
    virtual ~StockT();

    b32 LoadFromFile(const ch* lpProfile);
    b32 LoadFromFile(const wch* lpProfile);
    b32 SaveToFile(const ch* lpProfile) const;
    b32 SaveToFile(const wch* lpProfile) const;

    b32 Set(BufferBase* pBuffer);         // 内部会复制一份
    b32 Set(T_LPCSTR str, clsize nCount); // 内部会复制一份

    size_t GetModification() const;       // 获得修改标记，如果两次获得值不一致，说明期间做出了修改。注意这个不能用值含义判断是否修改，而要比较两次获得值。

    b32 Close();

    //////////////////////////////////////////////////////////////////////////


    //************************************
    // Method:    CreateSection 创建section
    // Qualifier:
    // Parameter: _TCh * szPath
    // szPath支持"sect3/sect2/sect1/sect0"和"sect"两种格式
    // 如果带有路径，sect3，sect2和sect1路径先执行查找功能，如果找不到则会创建
    // 最后的sect0会直接创建
    // 如果不带路径，则直接在根上创建sect
    // 注意：不再使用时需要用CloseSection关闭
    //************************************
    Section CreateSection(T_LPCSTR szPath);
    Section CreateSection(Section* desc, T_LPCSTR szSubPath); // 等价于SECTION::Create

    //************************************
    // Method:    OpenSection 打开指定的Section
    // Qualifier:
    // Parameter: _TCh * szPath
    // 打开Section的路径名，如"sect1/sect0"或者"sect"
    // 注意：不再使用时需要用CloseSection关闭
    //************************************
    Section OpenSection(T_LPCSTR szPath) const;
    Section OpenSection(Section* desc, T_LPCSTR szSubPath) const; // 等价于SECTION::Open

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

    const MemBuffer& GetBuffer() const;

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

#else
# pragma message(__FILE__": warning : Duplicate included this file.")
#endif // _CLSTD_STOCK_H_