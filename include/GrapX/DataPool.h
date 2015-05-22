#ifndef _MARIMO_DATA_POOL_H_
#define _MARIMO_DATA_POOL_H_

// 编译开关
#define ENABLE_DATAPOOL_WATCHER     // DataPool 监视器
//#define DEBUG_DECL_NAME           // 使用字符串指针储存自定位副本，如果打开这个调试将不能保存和加载

#ifdef ENABLE_DATAPOOL_WATCHER
# define STR_DATAPOOL_WATCHER_UI       "DataPool/Watcher/UI"
# define ON_KNOCKVAR(_KNOCKACT, _VAR)  (_KNOCKACT->pSponsor != &_VAR && _KNOCKACT->Name == _VAR.GetName())
#endif // #ifdef ENABLE_DATAPOOL_WATCHER

namespace clstd
{
  struct STRINGSETDESC;
} // namespace clstd

//class SmartStockW;

namespace Marimo
{
  class DataPool;
  class DataPoolWatcher;
  class DataPoolVariable;
  class DataPoolCompiler;
  class DataPoolInclude;
  struct DataPoolBuildTime;

  namespace DataPoolUtility
  {
    struct iterator;
    struct element_iterator;
    struct element_reverse_iterator;
    struct named_iterator;
    struct named_element_iterator;
    struct named_element_reverse_iterator;
  } // namespace DataPoolUtility


  enum TypeCategory
  {
    T_UNDEFINE = 0, // 结尾用的
    T_BYTE,         // 1 byte
    T_WORD,         // 2 bytes
    T_DWORD,        // 4 bytes
    T_QWORD,        // 8 bytes
    T_SBYTE,        // signed 1 byte
    T_SWORD,        // signed 2 bytes
    T_SDWORD,       // signed 4 bytes
    T_SQWORD,       // signed 8 bytes
    T_FLOAT,        // float 32
    T_STRING,
    T_STRINGA,      // ANSI string
    T_OBJECT,       // GUnknown*
    T_ENUM,         // 枚举
    T_FLAG,         // 标志型枚举
    T_STRUCT,
    T_STRUCTALWAYS, // 即使定义后没有使用, 也会保留它的格式
    T_MAX,
  };

  enum DataPoolLoad
  {
    DataPoolLoad_ReadOnly = 0x00000001,
  };

  // 还没用上
  enum TypeCategoryFlag
  {
    TypeCategoryFlag_Byte         = 1 << T_BYTE,
    TypeCategoryFlag_Word         = 1 << T_WORD,
    TypeCategoryFlag_Dword        = 1 << T_DWORD,
    TypeCategoryFlag_Qword        = 1 << T_QWORD,
    TypeCategoryFlag_SByte        = 1 << T_SBYTE,
    TypeCategoryFlag_SWord        = 1 << T_SWORD,
    TypeCategoryFlag_SDWord       = 1 << T_SDWORD,
    TypeCategoryFlag_SQWord       = 1 << T_SQWORD,
    TypeCategoryFlag_Float        = 1 << T_FLOAT,
    TypeCategoryFlag_String       = 1 << T_STRING,
    TypeCategoryFlag_StringA      = 1 << T_STRINGA,
    TypeCategoryFlag_Object       = 1 << T_OBJECT,
    TypeCategoryFlag_Enum         = 1 << T_ENUM,
    TypeCategoryFlag_Flag         = 1 << T_FLAG,
    TypeCategoryFlag_Struct       = 1 << T_STRUCT,
    TypeCategoryFlag_StructAlways = 1 << T_STRUCTALWAYS,
  };

  enum DataAction
  {
    DATACT_Undefined,
    DATACT_Change,
    DATACT_Insert,
    DATACT_Deleting,  // 正在改变，成员还没删
    DATACT_Deleted,   // 删除后的通知
  };

//#define ENABLE_OLD_DATA_ACTION
#ifdef ENABLE_OLD_DATA_ACTION
  struct KNOCKACTION
  {
    const DataPoolVariable* pSponsor; // 发起者
    DataPool*   pDataPool;
    DataAction  Action;
    clStringW   Name;
    GXLPCVOID   ptr;
    GXINT       Index;  // 只有动态数组才有效
  };
#endif // #ifdef ENABLE_OLD_DATA_ACTION

  struct DATAPOOL_IMPULSE
  {
    const DataPoolVariable* sponsor; // 发起者
    DataAction reason;  // 数据改变类型
    GXUINT     index;   // 如果是数组元素，标记开始索引
    GXUINT     count;   // 如果是数组元素，标记元素数量
    GXLPARAM   param;   // 用户参数
  };
  typedef const DATAPOOL_IMPULSE* LPCDATAIMPULSE;

  const GXDWORD VarDeclFlag_Const = 0x0001;

  //////////////////////////////////////////////////////////////////////////

  // DataPool 变量声明结构体
  struct VARIABLE_DECLARATION
  {
    GXLPCSTR  Type;       // 类型名字
    GXLPCSTR  Name;       // 变量名
    GXDWORD   Flags;      // 标志,"VarDeclFlag_*"
    GXINT     Count;      // 长度度,0,1表示1个元素
                          // 大于1表示n个长度
                          // 小于0表示变长数组,变长数组的内容不在统一地址中
                          // 动态数组初始化时, 
                          //   如果Init不为空, 则初始的动态数组大小为abs(Count);
                          //   如果Init为空, 则初始动态数组大小为0.
    GXLPVOID  Init;       // 初始值,如果不为 NULL, count 的绝对值就是元素个数,
                          // 必须有足够长度的数据, 尤其对于string数据
  };
  typedef const VARIABLE_DECLARATION* LPCVARDECL;

  // DataPool 枚举声明结构体
  struct ENUM_DECLARATION // 枚举，标志型枚举都是这个
  {
    GXLPCSTR  Name;       // 变量名
    GXINT     Value;      // 值，都是int型
  };
  typedef const ENUM_DECLARATION* LPCENUMDECL;

  // 类型声明结构体
  struct TYPE_DECLARATION
  {
    TypeCategory          Cate;
    GXLPCSTR              Name;
    union {
      VARIABLE_DECLARATION* Struct; // 结构体 成员
      ENUM_DECLARATION*     Enum;
    }as;
  };
  typedef const TYPE_DECLARATION* LPCTYPEDECL;

  //////////////////////////////////////////////////////////////////////////

  // 内部储存描述结构体, 同时也是文件储存的结构体声明
  // 字符串成员变量使用了自定位方法，这个依赖于结构体自身在内存中的位置
  // 所以使用时要注意从内部取出的描述要使用指针，不能复制实体类
  // 当使用引用方法时，release版下优化也可能改变临时变量的内存位置而出错，要注意

#pragma pack(push, 1)
  struct DATAPOOL_TYPE_DESC
  {
#ifdef DEBUG_DECL_NAME
    GXLPCSTR     Name;
#endif
    GXUINT       nName;   // 自定位
    TypeCategory Cate;
    GXUINT       cbSize;
    GXUINT       Member;  // 自定位
    //GXUINT       nMemberIndex; // 在 m_aStructMember 或 m_aEnumPck 中的索引
    GXUINT       nMemberCount;
    // TODO: nMemberIndex也改为自定位方式
  };

  struct DATAPOOL_VARIABLE_DESC
  {
    GXUINT      TypeDesc;           // 减法自定位，指向(TYPE_DESC*)类型
#ifdef DEBUG_DECL_NAME
    GXLPCSTR    Name;
#endif // DEBUG_DECL_NAME
    GXUINT      nName;              // StringBase偏移
    GXUINT      nOffset;            // 全局变量是绝对偏移，成员变量是结构体内偏移
    GXUINT      nCount   : 30;      // 数组大小,一元变量应该是1,动态数组暂定为0
    GXUINT      bDynamic : 1;       // 应该使用IsDynamicArray()接口
    GXUINT      bConst   : 1;
  };

  struct DATAPOOL_ENUM_DESC
  {
#ifdef DEBUG_DECL_NAME
    GXLPCSTR    Name;     // 名字
#endif // DEBUG_DECL_NAME
    GXUINT      nName;
    GXINT       Value;    // 值
  };
#pragma pack(pop)

  STATIC_ASSERT(sizeof(TypeCategory) == 4);
#ifdef DEBUG_DECL_NAME
#else
  STATIC_ASSERT(sizeof(DATAPOOL_TYPE_DESC) == 20);
  STATIC_ASSERT(sizeof(DATAPOOL_VARIABLE_DESC) == 16);
  STATIC_ASSERT(sizeof(DATAPOOL_ENUM_DESC) == 8);
#endif // DEBUG_DECL_NAME

  //////////////////////////////////////////////////////////////////////////

  class DataPoolArray : public clBuffer
  {
  public:
    DataPoolArray(u32 nPageSize)
      : clBuffer(nPageSize)
    {}

    DataPoolArray(GXLPBYTE pPlacement, u32 nSize) // placement new 专用
    {      
      ASSERT((GXINT_PTR)this == (GXINT_PTR)pPlacement); // 纯验证用，没实际意义

      m_lpBuffer  = (pPlacement + sizeof(DataPoolArray));
      m_uSize     = nSize;
      m_nCapacity = nSize;
      m_nPageSize = 0;
    }
  };

  class GXDLL DataPool : public GUnknown
  {
    friend class DataPoolVariable;
    friend class DataPoolVariableImpl;
    //friend struct iterator;
    //friend struct element_iterator;
    friend struct DataPoolElementReserveIterator;

  public:
    struct VARIABLE_DESC;
    struct TYPE_DESC;
    struct ENUM_DESC;

    typedef GXLPCSTR              LPCSTR;
    typedef VARIABLE_DESC*        LPVD;
    typedef const VARIABLE_DESC*  LPCVD;
    typedef const ENUM_DESC*      LPCED;
    typedef const TYPE_DESC*      LPCTD;
    typedef DataPoolUtility::iterator                 iterator;
    typedef DataPoolUtility::named_iterator           named_iterator;

    typedef GXVOID (GXCALLBACK *ImpulseProc)(DATAPOOL_IMPULSE* pImpulse);

    //typedef DataPoolUtility::element_iterator         element_iterator;
    //typedef DataPoolUtility::element_reverse_iterator relement_iterator;

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
      inline DataPool::LPCSTR GetName() const
      {
#ifdef DEBUG_DECL_NAME
        // 自定位方法和这个成员变量地址相关，必须使用引用或者指针类型来传递这个结构体
        ASSERT((DataPool::LPCSTR)((GXINT_PTR)&nName + nName) == Name || Name == NULL);
#endif // #ifdef DEBUG_DECL_NAME
        return (DataPool::LPCSTR)((GXINT_PTR)&nName + nName);
      }

      inline DataPool::LPCVD GetMembers() const
      {
        return (DataPool::LPCVD)((GXINT_PTR)&Member + Member);
      }

      inline DataPool::LPCED GetEnumMembers() const
      {
        return (DataPool::LPCED)((GXINT_PTR)&Member + Member);
      }

      inline GXUINT GetMemberIndex(LPCVD aGlobalMemberTab) const  // 获得自己成员变量在全局成员变量表的位置
      {
        LPCVD const aMembers = GetMembers();
        return (GXUINT)(aMembers - aGlobalMemberTab);
      }

      inline GXUINT GetEnumIndex(LPCED aGlobalEnumTab) const
      {
        LPCED const aEnums = GetEnumMembers();
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

      DataPoolArray* CreateAsBuffer(DataPool* pDataPool, GXBYTE* pBaseData, int nInitCount) const
      {
        ASSERT(IsDynamicArray()); // 一定是动态数组

        DataPoolArray** ppBuffer = GetAsBufferPtr(pBaseData);  // 动态数组
        if(*ppBuffer == NULL && TEST_FLAG_NOT(pDataPool->m_dwRuntimeFlags, RuntimeFlag_Readonly))
        {
          //ASSERT(nInitCount >= 1);
          // 这里ArrayBuffer只能使用指针形式
          *ppBuffer = new DataPoolArray(TypeSize() * 10);  // 十倍类型大小
          (*ppBuffer)->Resize(nInitCount * TypeSize(), TRUE);
          //TRACE("%%%s[%08x]\n", Name, *ppBuffer);
#ifdef _DEBUG
          pDataPool->m_nDbgNumOfArray++;
#endif // #ifdef _DEBUG
        }
        return *ppBuffer;
      }

      inline LPCTD GetTypeDesc() const
      {
        const TYPE_DESC* pRetTypeDesc = (TYPE_DESC*)((GXINT_PTR)&TypeDesc - TypeDesc);
        return pRetTypeDesc;
      }

      inline GXUINT TypeSize() const
      {
        //return ((TYPE_DESC*)((GXLONG_PTR)pTypeDesc & (~3)))->cbSize;
        return GetTypeDesc()->cbSize;
      }

      inline TypeCategory GetTypeCategory() const
      {
        //return ((TYPE_DESC*)((GXLONG_PTR)pTypeDesc & (~3)))->Cate;
        return GetTypeDesc()->Cate;
      }

      inline DataPool::LPCSTR VariableName() const
      {
#ifdef DEBUG_DECL_NAME
        // 自定位方法和这个成员变量地址相关，必须使用引用或者指针类型来传递这个结构体
        ASSERT((DataPool::LPCSTR)((GXINT_PTR)&nName + nName) == Name || Name == NULL);
#endif // #ifdef DEBUG_DECL_NAME
        return (DataPool::LPCSTR)((GXINT_PTR)&nName + nName);
      }

      inline DataPool::LPCSTR TypeName() const
      {
        return GetTypeDesc()->GetName();
      }

      inline LPCVD MemberBeginPtr() const
      {
        //return ((TYPE_DESC*)((GXLONG_PTR)pTypeDesc & (~3)))->nMemberIndex;
        LPCTD pTypeDesc = GetTypeDesc();
        return pTypeDesc->GetMembers();
      }

      inline GXUINT MemberCount() const
      {
        //return ((TYPE_DESC*)((GXLONG_PTR)pTypeDesc & (~3)))->nMemberCount;
        LPCTD pTypeDesc = GetTypeDesc();
        return pTypeDesc->nMemberCount;
      }

      inline GXBOOL IsDynamicArray() const
      {
        //return pTypeDesc->Name[0] == '#';
        return bDynamic;
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
        return vtbl && pBuffer&& pVdd;
      }
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
    WatcherArray        m_aWatchers;
    typedef clset<GXLPCVOID> KnockingSet;
    KnockingSet         m_setKnocking;    // 记录正在发送更改通知的Variable列表,防止多个相同指向的Variable反复递归.

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


    // COMMENT:
    // KnockingSet目前储存了Var指向变量的地址,静态变量不会有问题
    // 但是动态数组会有问题,在OnKnock时向动态数组增加元素可能
    // 会导致这个地址变化, 而最后在var->GetPtr无法找到并删除KnockingSet中的元素

    // GSIT:
    // #.是对一组variable / Enum的nName排序后的索引位置, "一组variable"是指全局变量集合或者结构成员集合
    // #.for each(i)
    //     VarDesc[m_aGSIT[i]].nName 是按照递增顺序增加的,这样成员变量可以使用二分法查找
    // #.m_aGSIT与m_aVariables+m_aMembers按分组顺序对应


    DataPool(GXLPCSTR szName);
    virtual ~DataPool();
    GXBOOL  Initialize        (LPCTYPEDECL pTypeDecl, LPCVARDECL pVarDecl);
    GXBOOL  Cleanup           (GXLPVOID lpBuffer, LPCVD pVarDesc, int nVarDescCount);
    GXBOOL  CleanupArray      (LPCVD pVarDesc, GXLPVOID lpFirstElement, int nElementCount);
    GXVOID  InitializeValue   (GXUINT nBaseOffset, LPCVARDECL pVarDecl);
    LPCVD   IntGetVariable    (LPCVD pVdd, GXLPCSTR szName);
#ifdef ENABLE_DATAPOOL_WATCHER
    GXBOOL  IntIsKnocking     (const DataPoolVariable* pVar) const;
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
    GXBOOL        IntCreateUnary      (clBufferBase* pBuffer, LPCVD pThisVdd, int nOffsetAdd, VARIABLE* pVar);
    GXBOOL        IntQuery            (clBufferBase* pBuffer, LPCVD pParentVdd, GXLPCSTR szVariable, int nOffsetAdd, VARIABLE* pVar);
    GXBOOL        IntQueryByExpression(GXLPCSTR szExpression, VARIABLE* pVar);
#ifdef ENABLE_DATAPOOL_WATCHER
    int           FindWatcher         (DataPoolWatcher* pWatcher);
    int           FindWatcherByName   (GXLPCSTR szClassName);
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

  public:
#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    virtual GXHRESULT   AddRef              ();
    virtual GXHRESULT   Release             ();
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

    virtual GXBOOL      SaveW               (GXLPCWSTR szFilename);
    virtual GXBOOL      Save                (clFile& file);
    virtual GXBOOL      Load                (clFile& file, GXDWORD dwFlag);

    virtual GXLPCSTR    GetVariableName     (GXUINT nIndex) const; // 获得变量的名字
#ifdef DATAPOOLCOMPILER_PROJECT
#else
    virtual GXHRESULT   GetLayout           (GXLPCSTR szStructName, DataLayoutArray* pLayout);
#endif // #ifdef DATAPOOLCOMPILER_PROJECT
    virtual GXHRESULT   ImportDataFromFileW (GXLPCWSTR szFilename) = 0;

    virtual GXBOOL      IsFixedPool         (); // 池中不含有字符串和动态数组
    virtual GXLPVOID    GetFixedDataPtr     (); // 必须是RawPool才返回指针
    virtual GXBOOL      QueryByName         (GXLPCSTR szName, DataPoolVariable* pVar);
    virtual GXBOOL      QueryByExpression   (GXLPCSTR szExpression, DataPoolVariable* pVar);
    virtual GXBOOL      FindFullName        (clStringA* str, const VARIABLE_DESC* pVarDesc, clBufferBase* pBuffer, GXUINT nOffset); // 查找变量全名

#ifdef ENABLE_DATAPOOL_WATCHER
    virtual GXBOOL      IsAutoKnock         ();
    virtual GXBOOL      IsKnocking          (const DataPoolVariable* pVar);
    virtual GXBOOL      SetAutoKnock        (GXBOOL bAutoKnock);

#ifdef ENABLE_OLD_DATA_ACTION
    virtual GXHRESULT   CreateWatcher       (GXLPCSTR szClassName);
    virtual GXHRESULT   RemoveWatcherByName (GXLPCSTR szClassName);
    virtual GXHRESULT   AddWatcher          (DataPoolWatcher* pWatcher);
    virtual GXHRESULT   RemoveWatcher       (DataPoolWatcher* pWatcher);
    virtual GXHRESULT   RegisterIdentify    (GXLPCSTR szClassName, GXLPVOID pIdentify); // TODO: 名字起的不好
    virtual GXHRESULT   UnregisterIdentify  (GXLPCSTR szClassName, GXLPVOID pIdentify); // TODO: 名字起的不好
#endif // #ifdef ENABLE_OLD_DATA_ACTION

    virtual GXHRESULT   ImpluseByVariable   (DataAction eType, const DataPoolVariable& var, GXUINT nIndex, GXBOOL bForce = TRUE);

    virtual GXBOOL      Watch               (GXLPCSTR szExpression, ImpulseProc pImpulseCallback, GXLPARAM lParam);
    virtual GXBOOL      Watch               (GXLPCSTR szExpression, DataPoolWatcher* pWatcher);
    virtual GXBOOL      Watch               (GXLPCSTR szExpression, GXHWND hWnd);
    virtual GXBOOL      Watch               (DataPoolVariable* pVar, ImpulseProc pImpulseCallback, GXLPARAM lParam);
    virtual GXBOOL      Watch               (DataPoolVariable* pVar, DataPoolWatcher* pWatcher);
    virtual GXBOOL      Watch               (DataPoolVariable* pVar, GXHWND hWnd);

    virtual GXBOOL      Ignore              (GXLPCSTR szExpression, ImpulseProc pImpulseCallback);
    virtual GXBOOL      Ignore              (GXLPCSTR szExpression, DataPoolWatcher* pWatcher);
    virtual GXBOOL      Ignore              (GXLPCSTR szExpression, GXHWND hWnd);
    virtual GXBOOL      Ignore              (DataPoolVariable* pVar, ImpulseProc pImpulseCallback);
    virtual GXBOOL      Ignore              (DataPoolVariable* pVar, DataPoolWatcher* pWatcher);
    virtual GXBOOL      Ignore              (DataPoolVariable* pVar, GXHWND hWnd);
#endif // #ifdef ENABLE_DATAPOOL_WATCHER

    iterator        begin       ();
    iterator        end         ();
    named_iterator  named_begin ();
    named_iterator  named_end   ();

    // TODO: 这个还被DynamicArray_Remove使用
    static  GXHRESULT   FindDataPool        (DataPool** ppDataPool, GXLPCSTR szName);
    static  GXHRESULT   FindVariable        (DataPool** ppDataPool, DataPoolVariable* pVar, GXLPCSTR szGlobalExpession);
    static  GXHRESULT   CreateDataPool      (DataPool** ppDataPool, GXLPCSTR szName/*= NULL*/, const TYPE_DECLARATION* pTypeDecl, const VARIABLE_DECLARATION* pVarDecl);
    static  GXHRESULT   CreateFromResolver  (DataPool** ppDataPool, GXLPCSTR szName/*= NULL*/, DataPoolCompiler* pResolver);
    static  GXHRESULT   CreateFromFileW     (DataPool** ppDataPool, GXLPCSTR szName/*= NULL*/, GXLPCWSTR szFilename, GXDWORD dwFlag);
    static  GXHRESULT   CompileFromMemory   (DataPool** ppDataPool, GXLPCSTR szName/*= NULL*/, DataPoolInclude* pInclude, GXLPCSTR szDefinitionCodes, GXSIZE_T nCodeLength = 0);
    static  GXHRESULT   CompileFromFileW    (DataPool** ppDataPool, GXLPCSTR szName/*= NULL*/, GXLPCWSTR szFilename, DataPoolInclude* pInclude = NULL);
    static  GXBOOL      IsIllegalName       (GXLPCSTR szName); // 检查类型/变量命名是否符合要求

    // 注意不支持 struct 的自包含, 支持动态数组的自包含功能:
    // struct A
    //{
    //  A a[];    // 支持这种声明, 表示它是动态数组,其它形式的声明不支持, 例子如下.
    //  A ab[10]; // 不支持这种声明, 这样会无限递归数组的长度
    //};

    // 对于动态数组,任何成员的取址"&"操作都可能是不安全的, 这个地址会因为动态数组成员的增删而改变

    // 池(DataPool)中储存的值:
    // 对于数字和结构体, 按照全局声明顺序分布, 数字按照声明大小占相应字节,结构体按照各个成员声明占用连续的空间.
    // 对于固定长度的数组(数字数组,结构体数组或者结构体中数组), 按照展开的声明顺序分布,占用空间情况同上.
    // 对于字符串(string)类型, 按照声明顺序分布,占用4字节, 指向字符串地址, 这个地址是可变的.
    // 对于动态数组类型, 按照全局声明顺序分布,占用4字节,内容是(clBuffer*)的指针.
  };

#ifdef ENABLE_OLD_DATA_ACTION
#ifdef ENABLE_DATAPOOL_WATCHER
  class DataPoolWatcher : public GUnknown
  {
  public:
#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    GXSTDINTERFACE(GXHRESULT AddRef   ());
    GXSTDINTERFACE(GXHRESULT Release  ());
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    //************************************
    // Method:    GetClassName
    // FullName:  GetClassName
    // Access:    public 
    // Returns:   返回用于标识不同Watcher的字符串
    // Qualifier:
    // Parameter: clStringA GetClassName 
    //************************************
    GXSTDINTERFACE(clStringA GetClassName       ());

    //************************************
    // Method:    RegisterPrivate
    // FullName:  RegisterPrivate
    // Access:    public 
    // Returns:   成功返回GX_OK,否则返回GX_FAIL
    // Qualifier: 用来注册Watcher私有数据的接口, 比如UI Watcher会把HWND
    //            注册在相同的Watcher内部,而不是为每个HWND产生一个Watcher,
    //            如果是一个专用的Watcher也可以不去实现这个接口的功能.
    // Parameter: GXLPVOID pIndentify
    //************************************
    GXSTDINTERFACE(GXHRESULT RegisterPrivate    (GXLPVOID pIndentify));
    GXSTDINTERFACE(GXHRESULT UnregisterPrivate  (GXLPVOID pIndentify));
    GXSTDINTERFACE(GXHRESULT OnKnock            (KNOCKACTION* pKnock));
  };
#else
  class DataPoolWatcher : public GUnknown
  {
  };
#endif // #ifdef ENABLE_DATAPOOL_WATCHER
#else
  class DataPoolWatcher : public GUnknown
  {
  public:
#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    GXSTDINTERFACE(GXHRESULT AddRef   ());
    GXSTDINTERFACE(GXHRESULT Release  ());
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    GXSTDINTERFACE(GXVOID OnImpulse   (LPCDATAIMPULSE pImpulse));
  };
#endif // #ifdef ENABLE_OLD_DATA_ACTION

  //
  // DataPool 编译器使用的头文件打开方法
  //
  class DataPoolInclude
  {
  public:
    enum IncludeType {
      IncludeType_System,
      IncludeType_Local,
    };
    virtual GXHRESULT Open(IncludeType eIncludeType, GXLPCWSTR pFileName, GXLPVOID lpParentData, GXLPCVOID *ppData, GXUINT *pBytes) GXPURE;
    virtual GXHRESULT Close(GXLPCVOID pData) GXPURE;
  };

  //
  // DataPool 编译器
  //
  class DataPoolCompiler : public GUnknown
  {
  public:
    struct MANIFEST
    {
      const TYPE_DECLARATION*     pTypes;
      const VARIABLE_DECLARATION* pVariables;
      const clStringListW*        pImportFiles;
    };
  public:
    GXSTDINTERFACE(GXHRESULT AddRef  ());
    GXSTDINTERFACE(GXHRESULT Release ());

    GXSTDINTERFACE(GXHRESULT GetManifest(MANIFEST* pManifest) const);

  public:
    static GXHRESULT CreateFromMemory(DataPoolCompiler** ppResolver, DataPoolInclude* pInclude, GXLPCSTR szDefinitionCodes, GXSIZE_T nCodeLength);
  };

  //////////////////////////////////////////////////////////////////////////
  namespace DataPoolUtility
  {

  } // namespace DataPoolUtility

} // namespace Marimo

#endif // #ifndef _MARIMO_DATA_POOL_H_