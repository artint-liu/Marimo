#ifndef _MARIMO_DATAPOOL_IMPLEMENT_H_
#define _MARIMO_DATAPOOL_IMPLEMENT_H_

//class StockW;

// 不跨越16字节边界相关的宏
#define NOT_CROSS_16_BYTES_BOUNDARY static_cast<GXUINT>(-16)
#define IS_MARK_NX16B(_FLAGS) ((_FLAGS & DataPoolCreation_NotCross16BytesBoundary) == DataPoolCreation_NotCross16BytesBoundary)
#define IS_MEMBER_NX16B(_ENUM) (_ENUM == DataPoolPack::NotCross16Boundary || _ENUM == DataPoolPack::NotCross16BoundaryShort)
#define SELECT_INTERNAL_TYPE_TABLE(_FLAGS) IS_MARK_NX16B(_FLAGS)\
  ? DataPoolInternal::c_TypeDefine_NX16B : DataPoolInternal::c_TypeDefine

namespace Marimo
{
  struct DataPoolBuildTime;
  class DataPoolVariableImpl;

  namespace DataPoolInternal
  {
    //GXUINT CreationFlagsToAlignSize(GXDWORD dwFlags);
    DataPoolPack CreationFlagsToMemberPack(GXDWORD dwFlags);
    GXUINT NotCross16BytesBoundaryArraySize(GXUINT nTypeSize, GXUINT nElementCount);
    GXUINT GetMemberAlignMask(DataPoolPack eMemberPack);
  }

  struct DATAPOOL_HASHALGO
  {
    u16  eType : 2; // clstd::StaticStringsDict::HashType
    u16  nBucket : 14;
    s16  nPos;
    u16  nOffset;     // 自定位

    GXSIZE_T HashString(GXLPCSTR str) const;

    inline u8* HashToIndex(GXSIZE_T _hash) const
    {
      return (u8*)((GXINT_PTR)&nOffset + nOffset) + (_hash % nBucket) * 2;
    }
    //IntArray indices;
  };

#ifdef DEBUG_DECL_NAME
#else
  STATIC_ASSERT(sizeof(DATAPOOL_HASHALGO) == 6);
#endif // DEBUG_DECL_NAME


  // 内部实现的函数表
  struct DataPoolVariable::VTBL
  {
    typedef DataPoolVariable Variable;
    typedef DataPoolVariableImpl VarImpl;


    GXUINT    (*GetSize     )(const VarImpl* pThis);  // 字节大小, 数组是数组大小, 动态数据大小可变, 结构是是结构体大小
    //GXUINT    (*GetOffset   )(const VarImpl* pThis);  // 偏移,全局变量是全局偏移, 结构体变量是结构体内偏移
    //GXLPCSTR  (*GetName     )(const VarImpl* pThis);  // 获得定义名, 变量名, 数组变量名或者结构体变量名
    //GXLPCSTR  (*GetTypeName )(const VarImpl* pThis);  // 类型, 变量为变量名, 数组为"Type[n]"形式, 动态数组为"Type[]"形式, 结构体为"struct Name"形式

    // 结构体专用
    Variable  (*GetMember   )(const VarImpl* pThis, GXLPCSTR szName);    // 获得成员

    // 数组或动态数组专用 
    Variable  (*GetIndex    )(const VarImpl* pThis, GXSIZE_T nIndex);    // 获得特定索引的变量
    GXSIZE_T  (*GetLength   )(const VarImpl* pThis);                     // 获得数组的成员个数, 注意与GetSize区别
    Variable  (*NewBack     )(      VarImpl* pThis, GXUINT nIncrease);   // 在动态数组上追加数据, 动态数组专用
    GXBOOL    (*Remove      )(      VarImpl* pThis, GXSIZE_T nIndex, GXSIZE_T nCount);      // 移出动态数组指定索引的数据, 动态数组专用

    // 变量专用
    //clStringW (*ToStringW   )(const VarImpl* pThis);                   // 变量按照其含义转值, 数组和结构体等同于GetTypeName()
    //clStringA (*ToStringA   )(const VarImpl* pThis);
    GXBOOL    (*ParseW      )(      VarImpl* pThis, GXLPCWSTR szString, GXUINT length); // 按照变量类型转值(unicode)
    GXBOOL    (*ParseA      )(      VarImpl* pThis, GXLPCSTR szString, GXUINT length);  // 按照变量类型转值
    u32       (*ToInteger   )(const VarImpl* pThis);
    u64       (*ToInt64     )(const VarImpl* pThis);
    float     (*ToFloat     )(const VarImpl* pThis);
    clStringW (*ToStringW   )(const VarImpl* pThis);
    clStringA (*ToStringA   )(const VarImpl* pThis);
    GXBOOL    (*SetAsInteger)(      VarImpl* pThis, u32 val);             // 如果变量不足32位会被截断
    GXBOOL    (*SetAsInt64  )(      VarImpl* pThis, u64 val);
    GXBOOL    (*SetAsFloat  )(      VarImpl* pThis, float val);
    GXBOOL    (*SetAsStringW)(      VarImpl* pThis, GXLPCWSTR szString);
    GXBOOL    (*SetAsStringA)(      VarImpl* pThis, GXLPCSTR szString);
    GXBOOL    (*Retain      )(      VarImpl* pThis, GUnknown* pUnknown);
    GXBOOL    (*Query       )(const VarImpl* pThis, GUnknown** ppUnknown);
    GXBOOL    (*GetData     )(const VarImpl* pThis, GXLPVOID lpData, GXUINT cbSize);
    GXBOOL    (*SetData     )(      VarImpl* pThis, GXLPCVOID lpData, GXUINT cbSize);
  };

  class DataPoolArray : public clBuffer
  {
  protected:
      clBufferBase* m_pParent;
  public:
      DataPoolArray(clBufferBase* pParent, u32 nPageSize)
          : clBuffer(nPageSize)
          , m_pParent(pParent)
      {}

      DataPoolArray(u32 nSize, GXLPBYTE pPlacement) // placement new 专用
          : m_pParent((clBufferBase*)0x12345678)
      {      
          ASSERT((GXINT_PTR)this == (GXINT_PTR)pPlacement); // 纯验证用，没实际意义

          m_lpBuffer  = (pPlacement + sizeof(DataPoolArray));
          m_uSize     = nSize;
          m_nCapacity = nSize;
          m_nPageSize = 0;
      }

      void SetParent(clBufferBase* pParent) {
          ASSERT( ! m_pParent || m_pParent == (clBufferBase*)0x12345678);
          m_pParent = pParent;
      }

      clBufferBase* GetParent() const {
          return m_pParent;
      }
  };

  class DataPoolImpl : public DataPool
  {
    friend class DataPool;
    friend class DataPoolVariable;
    friend class DataPoolVariableImpl;

  protected:
    //typedef SmartStockW::Section Section;
    //typedef clstd::TextLinesW clTextLinesW;

    typedef DataPoolUtility::iterator                 iterator;
    typedef DataPoolUtility::named_iterator           named_iterator;

  public:
    struct VARIABLE_DESC;
    struct STRUCT_DESC;
    struct ENUM_DESC;

    typedef DATAPOOL_HASHALGO     HASHALGO;
    typedef DATAPOOL_TYPE_DESC    TYPE_DESC;
    typedef VARIABLE_DESC*        LPVD;
    typedef const VARIABLE_DESC*  LPCVD;
    typedef const ENUM_DESC*      LPCED;
    typedef const TYPE_DESC*      LPCTD;
    typedef const STRUCT_DESC*    LPCSD;


    //struct IMPORT
    //{
    //  typedef DataPoolErrorMsg<GXWCHAR> DataPoolErrorMsgW;
    //  SmartStockW       ss;
    //  DataPoolErrorMsgW ErrorMsg;
    //};

    struct VAR_COUNT // 导入数据时查找variable并标记使用的结构体
    {
      MOVariable var;
      GXUINT     nCount;
    };

#pragma pack(push, 1)
    struct FILE_HEADER // 文件头
    {
      GXDWORD dwFlags;          // 标志，64位指针
      GXDWORD dwHashMagic;      // "DataPool" hash值，用来校验hash算法的一致性

      GXUINT  nBufHeaderOffset; // 数据缓冲头（包括global var）描述开始地址
      GXUINT  nDescOffset;      // 运行时描述信息表在文件中的偏移, 就是Buffer在文件中写入的位置
      GXUINT  nStringVarOffset; // 字符串变量在文件的偏移
      GXUINT  nBuffersOffset;   // Buffer数据在文件中的偏移

      GXUINT  nNumOfTypes;      // 类型描述数量
      GXUINT  nNumOfStructs;    // 结构/枚举/标志描述数量
      GXUINT  nNumOfVar;        // 变量描述数量
      GXUINT  nNumOfMember;     // 成员变量描述数量
      GXUINT  nNumOfEnums;      // 枚举描述数量
      GXUINT  nNumOfNames;      // 所有描述表中名字(字符串)数量
      GXUINT  cbNames;          // 所有描述表中名字(字符串)占用的总尺寸
      GXUINT  cbHashBuckets;

      GXUINT  cbVariableSpace;  // 全局变量占用的空间
      GXUINT  nNumOfPtrVars;    // 全局变量中包含的指针数，用来在不同平台下调整cbVariableSpace的尺寸

      GXUINT  nNumOfStrings;    // 字符串变量数量
      GXUINT  cbStringSpace;    // 字符串变量占用的空间

      GXUINT  nNumOfArrayBufs;  // 动态数组的数量
    };

    struct FILE_BUFFERHEADER  // 文件Buffer信息头
    {
      GXUINT nBufferSize;
      GXUINT nNumOfRel;
      GXUINT nType; // 0的话是全局变量的buffer
    };
#pragma pack(pop)


    enum RuntimeFlag
    {
      RuntimeFlag_Fixed     = 0x10000000,   // 只要出现动态数组，object或者string，就为false
      RuntimeFlag_Readonly  = 0x20000000,   // 只读模式，这个牛逼在于，所有一切都在一块内存上，不用析构了
//#ifdef ENABLE_DATAPOOL_WATCHER
      RuntimeFlag_AutoKnock = 0x40000000,
//#endif // #ifdef ENABLE_DATAPOOL_WATCHER
    };

    //typedef DATAPOOL_TYPE_DESC TYPE_DESC;
    //struct TYPE_DESC : DATAPOOL_TYPE_DESC{};

    struct STRUCT_DESC : DATAPOOL_STRUCT_DESC
    {
      inline GXUINT GetMemberIndex(DataPoolImpl::LPCVD aGlobalMemberTab) const  // 获得自己成员变量在全局成员变量表的位置
      {
        auto aMembers = GetMembers();
        return (GXUINT)(aMembers - static_cast<const DATAPOOL_VARIABLE_DESC*>(aGlobalMemberTab));
      }

      inline GXUINT GetEnumIndex(DataPoolImpl::LPCED aGlobalEnumTab) const
      {
        auto const aEnums = GetEnumMembers();
        return (GXUINT)(aEnums - static_cast<const DATAPOOL_ENUM_DESC*>(aGlobalEnumTab));
      }

    };

    struct VARIABLE_DESC : DATAPOOL_VARIABLE_DESC
    {
      typedef GXLPCVOID    VTBL;

      DataPoolArray** GetAsBufferObjPtr(GXBYTE* pBaseData) const
      {
        ASSERT(IsDynamicArray()); // 动态数组
        return (DataPoolArray**)(pBaseData + nOffset);
      }

      DataPoolArray* GetAsBuffer(GXBYTE* pBaseData) const
      {
        return *GetAsBufferObjPtr(pBaseData);
      }

      GXLPVOID GetAsPtr(GXBYTE* pBaseData) const
      {
        return pBaseData + nOffset;
      }

      GUnknown** GetAsObject(GXBYTE* pBaseData) const
      {
        ASSERT(GetTypeCategory() == DataPoolTypeClass::Object); // object
        return (GUnknown**)(pBaseData + nOffset);
      }

      clStringW* GetAsStringW(GXBYTE* pBaseData) const
      {
        ASSERT(GetTypeCategory() == DataPoolTypeClass::String); // Unicode 字符串
        return (clStringW*)(pBaseData + nOffset);
      }

      clStringA* GetAsStringA(GXBYTE* pBaseData) const
      {
        ASSERT(GetTypeCategory() == DataPoolTypeClass::StringA); // ANSI 字符串
        return (clStringA*)(pBaseData + nOffset);
      }

      GXUINT GetUsageSize() const // 运行时的内存尺寸，动态数组在32/64位下不一致
      {
        if(IsDynamicArray()) {
          return sizeof(clBuffer*);
        }
        return GetSize();
      }

      GXUINT GetSize() const  // 稳定的变量描述尺寸，对照GetMemorySize()
      {
        ASSERT( ! IsDynamicArray()); // 不应该是动态数组
        return nCount * TypeSize();
      }

      VTBL* GetUnaryMethod() const;
      VTBL* GetMethod(GXDWORD dwFlags) const;
    };

    // 枚举成员
    struct ENUM_DESC : DATAPOOL_ENUM_DESC
    {
      inline DataPool::LPCSTR GetName() const
      {
#ifdef DEBUG_DECL_NAME
        // 自定位方法和这个成员变量地址相关，必须使用引用或者指针类型来传递这个结构体
        ASSERT((DataPool::LPCSTR)((GXINT_PTR)&nName + nName) == Name || Name == NULL);
#endif // #ifdef DEBUG_DECL_NAME
        return (DataPool::LPCSTR)((GXINT_PTR)&nName + nName);
      }
    };
    //typedef const ENUM_DESC* LPCENUMDESC;


    struct VARIABLE // 用于内部查询传递的结构体
    {
      typedef GXLPCVOID           VTBL;
      typedef const VARIABLE_DESC DPVDD;
      VTBL*         vtbl;
      DPVDD*        pVdd;
      clBufferBase* pBuffer;
      GXUINT        AbsOffset;

      void Set(VTBL* _vtbl, DPVDD* _pVdd, clBufferBase* _pBuffer, GXUINT _AbsOffset)
      {
        vtbl      = _vtbl;
        pVdd      = _pVdd;
        pBuffer   = _pBuffer;
        AbsOffset = _AbsOffset;
      }

      GXBOOL IsValid()
      {
        // 只读模式下，未使用的动态数组pBuffer有可能是NULL, 所以这里不检查pBuffer
        return vtbl && pVdd;
      }
    };

    typedef i32                 Enum;         // 数据池所使用的枚举类型的C++表示
    typedef u32                 Flag;         // 数据池所使用的标志类型的C++表示
    typedef u32                 EnumFlag;     // 枚举和标志类型的统一表示
    typedef clstd::FixedBuffer  clFixedBuffer;
    typedef clstd::RefBuffer    clRefBuffer;
    typedef DataPoolBuildTime   BUILDTIME;
    typedef GXUINT              SortedIndexType;



//#ifdef ENABLE_DATAPOOL_WATCHER
    typedef clvector<DataPoolWatcher*>  WatcherArray;
//#endif // #ifdef ENABLE_DATAPOOL_WATCHER
    struct SIZELIST
    {
      GXSIZE_T cbTypes;
      GXSIZE_T cbStructs;
      GXSIZE_T cbHashAlgo;
      GXSIZE_T cbGVSIT;
      GXSIZE_T cbVariables;
      GXSIZE_T cbMembers;
      GXSIZE_T cbEnums;
      GXSIZE_T cbNameTable;
    };


  public:
    DataPoolImpl(GXLPCSTR szName);
    virtual ~DataPoolImpl();

    virtual GXBOOL Save(GXLPCWSTR szFilename) override;
    virtual GXBOOL Save(clFile& file) override;
    virtual GXBOOL Load(clFile& file, DataPoolCreation dwFlags) override;

    virtual LPCSTR    GetVariableName     (GXUINT nIndex) const override; // 获得变量的名字

    virtual GXDWORD   GetFlags            () const override;
    virtual GXBOOL    IsFixedPool         () const override; // 池中不含有字符串和动态数组
#ifndef DISABLE_DATAPOOL_WATCHER
    virtual GXBOOL    IsAutoKnock         () override;
    virtual GXBOOL    IsKnocking          (const DataPoolVariable* pVar) override;
#endif // #ifndef DISABLE_DATAPOOL_WATCHER

    virtual GXLPVOID  GetFixedDataPtr     () override; // 必须是RawPool才返回指针
    virtual GXUINT    GetNameId           (LPCSTR szName) override;
    virtual GXBOOL    QueryByName         (GXLPCSTR szName, DataPoolVariable* pVar) override;
    virtual GXBOOL    QueryByExpression   (GXLPCSTR szExpression, DataPoolVariable* pVar) override;
    virtual GXBOOL    FindFullName        (clStringA* str, DataPool::LPCVD pVarDesc, clBufferBase* pBuffer, GXUINT nOffset) override; // 查找变量全名

#ifndef DISABLE_DATAPOOL_WATCHER
    virtual GXBOOL    SetAutoKnock        (GXBOOL bAutoKnock) override;

    virtual GXBOOL    Impulse             (const DataPoolVariable& var, DataAction reason, GXSIZE_T index, GXSIZE_T count) override;
    virtual GXBOOL    Watch               (GXLPCSTR szExpression, ImpulseProc pImpulseCallback, GXLPARAM lParam) override;
    virtual GXBOOL    Watch               (GXLPCSTR szExpression, DataPoolWatcher* pWatcher) override;
    virtual GXBOOL    Watch               (GXLPCSTR szExpression, GXHWND hWnd) override;
    virtual GXBOOL    Watch               (DataPoolVariable* pVar, ImpulseProc pImpulseCallback, GXLPARAM lParam) override;
    virtual GXBOOL    Watch               (DataPoolVariable* pVar, DataPoolWatcher* pWatcher) override;
    virtual GXBOOL    Watch               (DataPoolVariable* pVar, GXHWND hWnd) override;
    virtual GXBOOL    Ignore              (GXLPCSTR szExpression, ImpulseProc pImpulseCallback) override;
    virtual GXBOOL    Ignore              (GXLPCSTR szExpression, DataPoolWatcher* pWatcher) override;
    virtual GXBOOL    Ignore              (GXLPCSTR szExpression, GXHWND hWnd) override;
    virtual GXBOOL    Ignore              (DataPoolVariable* pVar, ImpulseProc pImpulseCallback) override;
    virtual GXBOOL    Ignore              (DataPoolVariable* pVar, DataPoolWatcher* pWatcher) override;
    virtual GXBOOL    Ignore              (DataPoolVariable* pVar, GXHWND hWnd) override;
#endif // #ifndef DISABLE_DATAPOOL_WATCHER

    virtual iterator        begin       () override;
    virtual iterator        end         () override;
    virtual named_iterator  named_begin () override;
    virtual named_iterator  named_end   () override;

  protected:
    //void    IntImportSections (IMPORT& import, Section sectParent, MOVariable* varParent);
    //void    IntImportKeys     (IMPORT& import, Section sect, MOVariable* var);
    GXBOOL  Initialize        (LPCTYPEDECL pTypeDecl, LPCVARDECL pVarDecl, DataPoolCreation dwFlags);
    GXBOOL  Cleanup           (GXLPVOID lpBuffer, const DATAPOOL_VARIABLE_DESC* pVarDesc, int nVarDescCount);
    GXBOOL  CleanupArray      (const VARIABLE_DESC* pVarDesc, GXLPVOID lpFirstElement, GXUINT nElementCount);
    GXVOID  InitializeValue   (GXUINT nBaseOffset, LPCVARDECL pVarDecl);
    LPCVD   IntGetVariable    (LPCVD pVdd, GXLPCSTR szName);
#ifndef DISABLE_DATAPOOL_WATCHER
    GXBOOL  IntIsImpulsing    (const DataPoolVariable* pVar) const;
#endif // #ifndef DISABLE_DATAPOOL_WATCHER
    void    LocalizeTables    (BUILDTIME& bt, GXSIZE_T cbVarSpace);
    clsize  LocalizePtr       ();
    template<class DescT>
    void    SortNames         (const DescT* pDescs, SortedIndexType* pDest, int nBeign, int nCount);

    template<class DescT>
    void    SelfLocalizable   (DescT* pDescs, int nCount, GXINT_PTR lpBase);

    template<class _TIter>
    _TIter& first_iterator    (_TIter& it);


    void    GenGSIT           ();

    const clBufferBase* IntGetEntryBuffer   () const; // 获得数据池最基础的buffer
    LPCTD         FindType            (GXLPCSTR szTypeName) const;
    //void          CopyVariables       (VARIABLE_DESC* pDestVarDesc, GXLPCVOID pSrcVector, const clstd::STRINGSETDESC* pTable, GXINT_PTR lpBase);
    GXBOOL        IntCreateUnary      (clBufferBase* pBuffer, LPCVD pThisVdd, VARIABLE* pVar);
    GXBOOL        IntQuery            (GXINOUT VARIABLE* pVar, GXLPCSTR szVariableName, GXUINT nIndex);
    GXINT         IntQueryByExpression(GXLPCSTR szExpression, VARIABLE* pVar);
//#ifdef ENABLE_DATAPOOL_WATCHER
    //int           FindWatcher         (DataPoolWatcher* pWatcher);
    //int           FindWatcherByName   (GXLPCSTR szClassName);
//#endif // #ifdef ENABLE_DATAPOOL_WATCHER
    //LPCENUMDESC   IntGetEnum          (GXUINT nPackIndex) const;  // m_aEnumPck中的索引
    LPCVD         IntFindVariable     (LPCVD pVarDesc, int nCount, GXUINT nOffset);
    GXBOOL        IntIgnore           (DataPoolVariable* pVar, ImpulseProc pImpulseCallback, GXLPARAM lParam);

    GXSIZE_T      IntGetRTDescHeader    (SIZELIST* pSizeList);   // 获得运行时描述表大小
    GXSIZE_T      IntGetRTDescNames     ();   // 获得运行时描述表字符串表所占的大小
    static GXUINT IntChangePtrSize      (GXUINT nSizeofPtr, VARIABLE_DESC* pVarDesc, GXUINT nCount);

    template<class _Ty>
    static void   IntClearChangePtrFlag (_Ty* pTypeDesc, GXUINT nCount);
    void          DbgIntDump            ();

    GXBOOL          IntFindEnumFlagValue  (LPCSD pTypeDesc, LPCSTR szName, EnumFlag* pOutEnumFlag) const;
    DataPoolArray*  IntCreateArrayBuffer  (clBufferBase* pParent, LPCVD pVarDesc, GXBYTE* pBaseData, int nInitCount);

    protected:
      clStringA           m_Name;             // 如果是具名对象的话，储存DataPool的名字

      // LocalizePtr根据这些参数重定位下面的指针
      clFixedBuffer       m_Buffer;
      GXUINT              m_nNumOfTypes;
      GXUINT              m_nNumOfStructs;
      GXUINT              m_nNumOfVar;
      GXUINT              m_nNumOfMember;
      GXUINT              m_nNumOfEnums;
      GXUINT              m_cbHashBuckets;      // Hash Buckets
      // =====================

      // 这些可以被LocalizePtr方法重定位
      TYPE_DESC*          m_aTypes;
      STRUCT_DESC*        m_aStructs;
      HASHALGO*           m_aHashAlgos;       // 1 + m_nNumOfStructs
      u8*                 m_aHashBuckets;     // var, struct1, struct2, ...
      SortedIndexType*    m_aGSIT;            // Grouped sorted index table, 详细见下
      VARIABLE_DESC*      m_aVariables;       // 所有变量描述表
      VARIABLE_DESC*      m_aMembers;         // 所有的结构体成员描述都存在这张表上
      ENUM_DESC*          m_aEnums;           // 所有枚举成员都在这个表上
      GXUINT*             m_pNamesTabBegin;   // variable/type name table 的开始
      GXUINT*             m_pNamesTabEnd;     // table 的结尾,也是Name ptr的开始
      // =====================

      clRefBuffer         m_VarBuffer;        // 变量空间开始地址, 这个指向了m_Buffer

//#ifdef _DEBUG
//      GXUINT              m_nDbgNumOfArray;   // 动态数组的缓冲区
//      GXUINT              m_nDbgNumOfString;  // 动态数组的缓冲区
//#endif // #ifdef _DEBUG

#ifndef DISABLE_DATAPOOL_WATCHER
      struct WATCH_FIXED;

      typedef clset<GXLPCVOID>                      ImpulsingSet;
      typedef clset<WATCH_FIXED>                    WatchFixedList;
      typedef clmap<GXLPVOID, WatchFixedList>       WatchFixedDict;
      typedef clmap<DataPoolArray*, WatchFixedDict> WatchableArray;


      ImpulsingSet         m_ImpulsingSet;    // 记录正在发送更改通知的Variable列表,防止多个相同指向的Variable反复递归.
      // COMMENT:
      // ImpulsingSet目前储存了Var指向变量的地址,静态变量不会有问题
      // 但是动态数组会有问题,在OnKnock时向动态数组增加元素可能
      // 会导致这个地址变化, 而最后在var->GetPtr无法找到并删除KnockingSet中的元素


      struct WATCH_FIXED // 固定变量监视器
      {
        ImpulseProc pCallback;
        GXLPARAM    lParam;
        bool operator<(const WATCH_FIXED& t) const; // set "<" 方法
      };


      WatchFixedDict      m_FixedDict;      // 在根上的监视变量
      WatchableArray      m_WatchableArray; // 可监视的数组,这个数组Buffer在根上的

  static  GXBOOL  IntAddToWatchDict       (WatchFixedDict& sDict, GXLPVOID key, ImpulseProc pImpulseCallback, GXLPARAM lParam);
  static  GXBOOL  IntRemoveFromWatchDict  (WatchFixedDict& sDict, GXLPVOID key, ImpulseProc pImpulseCallback, GXLPARAM lParam);
          GXBOOL  IntWatch                (DataPoolVariable* pVar, ImpulseProc pImpulseCallback, GXLPARAM lParam);
          void    IntImpulse              (WatchFixedDict& sDict, GXLPVOID key, DATAPOOL_IMPULSE* pImpulse);
          void    IntCleanupWatchObj      (WatchFixedDict& sWatchDict);
#endif // #ifndef DISABLE_DATAPOOL_WATCHER
//#endif // #ifdef ENABLE_DATAPOOL_WATCHER
      GXDWORD           m_dwRuntimeFlags; // RuntimeFlag 与 DataPoolCreation 供用，注意不要冲突
  }; // class DataPoolImpl

  // 如果是数字类型，则转换返回TRUE，否则返回FALSE
  // 支持0x十六进制，0b二进制，0开头的八进制和十进制
  template<typename _TCh>
  GXBOOL IsNumericT(const _TCh* str, clsize len, GXINT* pInteger)
  {
    if(str[0] == '0') {
      if(str[1] == 'x' || str[1] == 'X')
      {
        if(clstd::IsNumericT(str + 2, len - 2, 16)) {
          *pInteger = clstd::xtoi(16, str + 2, len - 2);
          return TRUE;
        }
      }
      else if(str[1] == 'b' || str[1] == 'B')
      {
        if(clstd::IsNumericT(str + 2, len - 2, 2)) {
          *pInteger = clstd::xtoi(2, str + 2, len - 2);
          return TRUE;
        }
      }
      else
      {
        if(clstd::IsNumericT(str + 1, len - 1), 8) {
          *pInteger = clstd::xtoi(8, str + 1, len - 1);
          return TRUE;
        }
      }
    }
    else if(clstd::IsNumericT(str, len, 10)) {
      *pInteger = clstd::xtoi(10, str, len);
      return TRUE;
    }
    return FALSE;
  }


} // namespace Marimo

#endif // _MARIMO_DATAPOOL_IMPLEMENT_H_