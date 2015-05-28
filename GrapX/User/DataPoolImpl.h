#ifndef _MARIMO_DATAPOOL_IMPLEMENT_H_
#define _MARIMO_DATAPOOL_IMPLEMENT_H_

class SmartStockW;

namespace Marimo
{
  struct DataPoolBuildTime;
  class DataPoolVariableImpl;

  // 内部实现的函数表
  struct DataPoolVariable::VTBL
  {
    typedef DataPoolVariable Variable;
    typedef DataPoolVariableImpl VarImpl;


    GXUINT    (*GetSize     )(GXCONST VarImpl* pThis);  // 字节大小, 数组是数组大小, 动态数据大小可变, 结构是是结构体大小
    //GXUINT    (*GetOffset   )(GXCONST VarImpl* pThis);  // 偏移,全局变量是全局偏移, 结构体变量是结构体内偏移
    //GXLPCSTR  (*GetName     )(GXCONST VarImpl* pThis);  // 获得定义名, 变量名, 数组变量名或者结构体变量名
    //GXLPCSTR  (*GetTypeName )(GXCONST VarImpl* pThis);  // 类型, 变量为变量名, 数组为"Type[n]"形式, 动态数组为"Type[]"形式, 结构体为"struct Name"形式

    // 结构体专用
    Variable  (*GetMember   )(GXCONST VarImpl* pThis, GXLPCSTR szName);    // 获得成员

    // 数组或动态数组专用 
    Variable  (*GetIndex    )(GXCONST VarImpl* pThis, int nIndex);         // 获得特定索引的变量
    GXUINT    (*GetLength   )(GXCONST VarImpl* pThis);                     // 获得数组的成员个数, 注意与GetSize区别
    Variable  (*NewBack     )(        VarImpl* pThis, GXUINT nIncrease);   // 在动态数组上追加数据, 动态数组专用
    GXBOOL    (*Remove      )(        VarImpl* pThis, GXUINT nIndex, GXUINT nCount);      // 移出动态数组指定索引的数据, 动态数组专用

    // 变量专用
    //clStringW (*ToStringW   )(GXCONST VarImpl* pThis);                   // 变量按照其含义转值, 数组和结构体等同于GetTypeName()
    //clStringA (*ToStringA   )(GXCONST VarImpl* pThis);
    GXBOOL    (*ParseW      )(        VarImpl* pThis, GXLPCWSTR szString, GXUINT length); // 按照变量类型转值(unicode)
    GXBOOL    (*ParseA      )(        VarImpl* pThis, GXLPCSTR szString, GXUINT length);  // 按照变量类型转值
    u32       (*ToInteger   )(GXCONST VarImpl* pThis);
    u64       (*ToInt64     )(GXCONST VarImpl* pThis);
    float     (*ToFloat     )(GXCONST VarImpl* pThis);
    clStringW (*ToStringW   )(GXCONST VarImpl* pThis);
    clStringA (*ToStringA   )(GXCONST VarImpl* pThis);
    GXBOOL    (*SetAsInteger)(        VarImpl* pThis, u32 val);             // 如果变量不足32位会被截断
    GXBOOL    (*SetAsInt64  )(        VarImpl* pThis, u64 val);
    GXBOOL    (*SetAsFloat  )(        VarImpl* pThis, float val);
    GXBOOL    (*SetAsStringW)(        VarImpl* pThis, GXLPCWSTR szString);
    GXBOOL    (*SetAsStringA)(        VarImpl* pThis, GXLPCSTR szString);
    GXBOOL    (*Retain      )(        VarImpl* pThis, GUnknown* pUnknown);
    GXBOOL    (*Query       )(GXCONST VarImpl* pThis, GUnknown** ppUnknown);
    GXBOOL    (*GetData     )(GXCONST VarImpl* pThis, GXLPVOID lpData, GXUINT cbSize);
    GXBOOL    (*SetData     )(        VarImpl* pThis, GXLPCVOID lpData, GXUINT cbSize);
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
    struct TYPE_DESC;
    struct ENUM_DESC;

    typedef VARIABLE_DESC*        LPVD;
    typedef const VARIABLE_DESC*  LPCVD;
    typedef const ENUM_DESC*      LPCED;
    typedef const TYPE_DESC*      LPCTD;



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

      GXUINT  nBufHeaderOffset; // 数据缓冲头（包括global var）描述开始地址
      GXUINT  nDescOffset;      // 运行时描述信息表在文件中的偏移, 就是Buffer在文件中写入的位置
      GXUINT  nStringVarOffset; // 字符串变量在文件的偏移
      GXUINT  nBuffersOffset;   // Buffer数据在文件中的偏移

      GXUINT  nNumOfTypes;      // 类型描述数量
      GXUINT  nNumOfVar;        // 变量描述数量
      GXUINT  nNumOfMember;     // 成员变量描述数量
      GXUINT  nNumOfEnums;      // 枚举描述数量
      GXUINT  cbDescTabNames;   // 所有描述表中名字(字符串)占用的总尺寸

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
      RuntimeFlag_Fixed     = 0x00000001,   // 只要出现动态数组，object或者string，就为false
      RuntimeFlag_Readonly  = 0x00000002,   // 只读模式，这个牛逼在于，所有一切都在一块内存上，不用析构了
#ifdef ENABLE_DATAPOOL_WATCHER
      RuntimeFlag_AutoKnock = 0x00000004,
#endif // #ifdef ENABLE_DATAPOOL_WATCHER
    };


    struct TYPE_DESC : DATAPOOL_TYPE_DESC
    {
      inline GXUINT GetMemberIndex(DataPoolImpl::LPCVD aGlobalMemberTab) const  // 获得自己成员变量在全局成员变量表的位置
      {
        auto aMembers = GetMembers();
        return (GXUINT)(aMembers - aGlobalMemberTab);
      }

      inline GXUINT GetEnumIndex(DataPoolImpl::LPCED aGlobalEnumTab) const
      {
        auto const aEnums = GetEnumMembers();
        return (GXUINT)(aEnums - aGlobalEnumTab);
      }
    };

    struct VARIABLE_DESC : DATAPOOL_VARIABLE_DESC
    {
      typedef GXLPCVOID    VTBL;

      DataPoolArray** GetAsBufferPtr(GXBYTE* pBaseData) const
      {
        ASSERT(IsDynamicArray()); // 动态数组
        return (DataPoolArray**)(pBaseData + nOffset);
      }

      DataPoolArray* CreateAsBuffer(DataPoolImpl* pDataPool, clBufferBase* pParent, GXBYTE* pBaseData, int nInitCount) const
      {
        ASSERT(IsDynamicArray()); // 一定是动态数组
        ASSERT(nInitCount >= 0);

        DataPoolArray** ppBuffer = GetAsBufferPtr(pBaseData);  // 动态数组
        if(*ppBuffer == NULL && TEST_FLAG_NOT(pDataPool->m_dwRuntimeFlags, RuntimeFlag_Readonly))
        {
          // 这里ArrayBuffer只能使用指针形式
          *ppBuffer = new DataPoolArray(TypeSize() * 10);  // 十倍类型大小
          (*ppBuffer)->Resize(nInitCount * TypeSize(), TRUE);

#ifdef _DEBUG
          pDataPool->m_nDbgNumOfArray++;
#endif // #ifdef _DEBUG
        }
        return *ppBuffer;
      }



      DataPoolArray* GetAsBuffer(GXBYTE* pBaseData) const
      {
        return *GetAsBufferPtr(pBaseData);
      }

      GXLPVOID GetAsPtr(GXBYTE* pBaseData) const
      {
        return pBaseData + nOffset;
      }

      GUnknown** GetAsObject(GXBYTE* pBaseData) const
      {
        ASSERT(GetTypeCategory() == T_OBJECT); // object
        return (GUnknown**)(pBaseData + nOffset);
      }

      clStringW* GetAsStringW(GXBYTE* pBaseData) const
      {
        ASSERT(GetTypeCategory() == T_STRING); // Unicode 字符串
        return (clStringW*)(pBaseData + nOffset);
      }

      clStringA* GetAsStringA(GXBYTE* pBaseData) const
      {
        ASSERT(GetTypeCategory() == T_STRINGA); // ANSI 字符串
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
      VTBL* GetMethod() const;
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



#ifdef ENABLE_DATAPOOL_WATCHER
    typedef clvector<DataPoolWatcher*>  WatcherArray;
#endif // #ifdef ENABLE_DATAPOOL_WATCHER



  public:
    DataPoolImpl(GXLPCSTR szName);
    virtual ~DataPoolImpl();

    virtual GXBOOL SaveW( GXLPCWSTR szFilename );
    virtual GXBOOL Save( clFile& file );
    virtual GXBOOL Load( clFile& file, GXDWORD dwFlag );

    //virtual GXHRESULT ImportDataFromFileW (GXLPCWSTR szFilename);
    virtual LPCSTR    GetVariableName     (GXUINT nIndex) const; // 获得变量的名字
    virtual GXHRESULT GetLayout           (GXLPCSTR szStructName, DataLayoutArray* pLayout);

    virtual GXBOOL    IsFixedPool         () const; // 池中不含有字符串和动态数组
    virtual GXLPVOID  GetFixedDataPtr     (); // 必须是RawPool才返回指针
    virtual GXBOOL    QueryByName         (GXLPCSTR szName, DataPoolVariable* pVar);
    virtual GXBOOL    QueryByExpression   (GXLPCSTR szExpression, DataPoolVariable* pVar);
    virtual GXBOOL    FindFullName        (clStringA* str, DataPool::LPCVD pVarDesc, clBufferBase* pBuffer, GXUINT nOffset); // 查找变量全名

    virtual GXBOOL    IsAutoKnock         ();
    virtual GXBOOL    IsKnocking          (const DataPoolVariable* pVar);
    virtual GXBOOL    SetAutoKnock        (GXBOOL bAutoKnock);

    virtual GXBOOL    Impluse             (const DataPoolVariable& var, DataAction reason, GXUINT index, GXUINT count);
    virtual GXBOOL    Watch               (GXLPCSTR szExpression, ImpulseProc pImpulseCallback, GXLPARAM lParam);
    virtual GXBOOL    Watch               (GXLPCSTR szExpression, DataPoolWatcher* pWatcher);
    virtual GXBOOL    Watch               (GXLPCSTR szExpression, GXHWND hWnd);
    virtual GXBOOL    Watch               (DataPoolVariable* pVar, ImpulseProc pImpulseCallback, GXLPARAM lParam);
    virtual GXBOOL    Watch               (DataPoolVariable* pVar, DataPoolWatcher* pWatcher);
    virtual GXBOOL    Watch               (DataPoolVariable* pVar, GXHWND hWnd);
    virtual GXBOOL    Ignore              (GXLPCSTR szExpression, ImpulseProc pImpulseCallback);
    virtual GXBOOL    Ignore              (GXLPCSTR szExpression, DataPoolWatcher* pWatcher);
    virtual GXBOOL    Ignore              (GXLPCSTR szExpression, GXHWND hWnd);
    virtual GXBOOL    Ignore              (DataPoolVariable* pVar, ImpulseProc pImpulseCallback);
    virtual GXBOOL    Ignore              (DataPoolVariable* pVar, DataPoolWatcher* pWatcher);
    virtual GXBOOL    Ignore              (DataPoolVariable* pVar, GXHWND hWnd);

    virtual iterator        begin       ();
    virtual iterator        end         ();
    virtual named_iterator  named_begin ();
    virtual named_iterator  named_end   ();

  protected:
    //void    IntImportSections (IMPORT& import, Section sectParent, MOVariable* varParent);
    //void    IntImportKeys     (IMPORT& import, Section sect, MOVariable* var);
    GXBOOL  Initialize        (LPCTYPEDECL pTypeDecl, LPCVARDECL pVarDecl);
    GXBOOL  Cleanup           (GXLPVOID lpBuffer, const DATAPOOL_VARIABLE_DESC* pVarDesc, int nVarDescCount);
    GXBOOL  CleanupArray      (const VARIABLE_DESC* pVarDesc, GXLPVOID lpFirstElement, int nElementCount);
    GXVOID  InitializeValue   (GXUINT nBaseOffset, LPCVARDECL pVarDecl);
    LPCVD   IntGetVariable    (LPCVD pVdd, GXLPCSTR szName);
#ifdef ENABLE_DATAPOOL_WATCHER
    GXBOOL  IntIsImpulsing    (const DataPoolVariable* pVar) const;
#endif // #ifdef ENABLE_DATAPOOL_WATCHER
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
    void          CopyVariables       (VARIABLE_DESC* pDestVarDesc, GXLPCVOID pSrcVector, const clstd::STRINGSETDESC* pTable, GXINT_PTR lpBase);
    GXBOOL        IntCreateUnary      (clBufferBase* pBuffer, LPCVD pThisVdd, VARIABLE* pVar);
    GXBOOL        IntQuery            (GXINOUT VARIABLE* pVar, GXLPCSTR szVariableName, GXUINT nIndex);
    GXINT         IntQueryByExpression(GXLPCSTR szExpression, VARIABLE* pVar);
#ifdef ENABLE_DATAPOOL_WATCHER
    //int           FindWatcher         (DataPoolWatcher* pWatcher);
    //int           FindWatcherByName   (GXLPCSTR szClassName);
#endif // #ifdef ENABLE_DATAPOOL_WATCHER
    //LPCENUMDESC   IntGetEnum          (GXUINT nPackIndex) const;  // m_aEnumPck中的索引
    LPCVD         IntFindVariable     (LPCVD pVarDesc, int nCount, GXUINT nOffset);
    GXBOOL        IntWatch            (DataPoolVariable* pVar, ImpulseProc pImpulseCallback, GXLPARAM lParam);
    GXBOOL        IntIgnore           (DataPoolVariable* pVar, ImpulseProc pImpulseCallback, GXLPARAM lParam);

    GXSIZE_T      IntGetRTDescHeader    ();   // 获得运行时描述表大小
    GXSIZE_T      IntGetRTDescNames     ();   // 获得运行时描述表字符串表所占的大小
    static GXUINT IntChangePtrSize      (GXUINT nSizeofPtr, VARIABLE_DESC* pVarDesc, GXUINT nCount);
    static void   IntClearChangePtrFlag (TYPE_DESC* pTypeDesc, GXUINT nCount);
    void          DbgIntDump            ();

    //void          Generate              (GXLPVOID lpBuffer, LPCVD pVarDesc, int nVarCount);

    GXBOOL IntFindEnumFlagValue(LPCTD pTypeDesc, LPCSTR szName, EnumFlag* pOutEnumFlag) GXCONST;

    protected:
      clStringA           m_Name;             // 如果是具名对象的话，储存DataPool的名字

      // LocalizePtr根据这些参数重定位下面的指针
      clFixedBuffer       m_Buffer;
      GXUINT              m_nNumOfTypes;
      GXUINT              m_nNumOfVar;
      GXUINT              m_nNumOfMember;
      GXUINT              m_nNumOfEnums;
      // =====================

      // 这些可以被LocalizePtr方法重定位
      TYPE_DESC*          m_aTypes;
      SortedIndexType*    m_aGSIT;            // Grouped sorted index table, 详细见下
      VARIABLE_DESC*      m_aVariables;       // 所有变量描述表
      VARIABLE_DESC*      m_aMembers;         // 所有的结构体成员描述都存在这张表上
      ENUM_DESC*          m_aEnums;           // 所有枚举成员都在这个表上
      // =====================

      clRefBuffer         m_VarBuffer;        // 变量空间开始地址, 这个指向了m_Buffer

#ifdef _DEBUG
      GXUINT              m_nDbgNumOfArray;   // 动态数组的缓冲区
      GXUINT              m_nDbgNumOfString;  // 动态数组的缓冲区
#endif // #ifdef _DEBUG

#ifdef ENABLE_DATAPOOL_WATCHER
      //WatcherArray        m_aWatchers;
      typedef clset<GXLPCVOID> KnockingSet;
      KnockingSet         m_ImpulsingSet;    // 记录正在发送更改通知的Variable列表,防止多个相同指向的Variable反复递归.
      // COMMENT:
      // KnockingSet目前储存了Var指向变量的地址,静态变量不会有问题
      // 但是动态数组会有问题,在OnKnock时向动态数组增加元素可能
      // 会导致这个地址变化, 而最后在var->GetPtr无法找到并删除KnockingSet中的元素


      struct WATCH_FIXED // 固定变量监视器
      {
        ImpulseProc pCallback;
        GXLPARAM    lParam;

        bool operator<(const WATCH_FIXED& t) const;
      };
      typedef clset<WATCH_FIXED>  WatchFixedList;
      typedef clhash_map<GXLPVOID, WatchFixedList> WatchFixedDict;

      WatchFixedDict      m_FixedDict;
#endif // #ifdef ENABLE_DATAPOOL_WATCHER
      GXDWORD           m_dwRuntimeFlags;
  }; // class DataPoolImpl

  // 如果是数字类型，则转换返回TRUE，否则返回FALSE
  // 支持0x十六进制，0b二进制，0开头的八进制和十进制
  template<typename _TCh>
  GXBOOL IsNumericT(const _TCh* str, clsize len, GXINT* pInteger)
  {
    if(str[0] == '0') {
      if(str[1] == 'x' || str[1] == 'X')
      {
        if(clstd::IsNumericT(str + 2, 16, len - 2)) {
          *pInteger = GXATOI(str + 2, 16, len - 2);
          return TRUE;
        }
      }
      else if(str[1] == 'b' || str[1] == 'B')
      {
        if(clstd::IsNumericT(str + 2, 2, len - 2)) {
          *pInteger = GXATOI(str + 2, 2, len - 2);
          return TRUE;
        }
      }
      else
      {
        if(clstd::IsNumericT(str + 1, 8, len - 1)) {
          *pInteger = GXATOI(str + 1, 8, len - 1);
          return TRUE;
        }
      }
    }
    else if(clstd::IsNumericT(str, 10, len)) {
      *pInteger = GXATOI(str, 10, len);
      return TRUE;
    }
    return FALSE;
  }


} // namespace Marimo

#endif // _MARIMO_DATAPOOL_IMPLEMENT_H_