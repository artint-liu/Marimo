#ifndef _MARIMO_DATA_POOL_H_
#define _MARIMO_DATA_POOL_H_

// 编译开关
//#define ENABLE_DATAPOOL_WATCHER     // DataPool 监视器
//#define DEBUG_DECL_NAME           // 使用字符串指针储存自定位副本，如果打开这个调试将不能保存和加载
//#ifdef ENABLE_DATAPOOL_WATCHER
//# define STR_DATAPOOL_WATCHER_UI       "DataPool/Watcher/UI"
//# define ON_KNOCKVAR(_KNOCKACT, _VAR)  (_KNOCKACT->pSponsor != &_VAR && _KNOCKACT->Name == _VAR.GetName())
//#endif // #ifdef ENABLE_DATAPOOL_WATCHER

namespace clstd
{
  struct STRINGSETDESC;
} // namespace clstd


namespace Marimo
{
  class DataPool;
  class DataPoolWatcher;
  class DataPoolVariable;
  class DataPoolCompiler;
  class DataPoolInclude;
  //struct DataPoolBuildTime;

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
    T_CATE_MASK = 0x1f, // 64位调整指针时会标记 TYPE_CHANGED_FLAG，这里用安全掩码去除 TYPE_CHANGED_FLAG 标记
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
//#ifdef ENABLE_OLD_DATA_ACTION
//  struct KNOCKACTION
//  {
//    const DataPoolVariable* pSponsor; // 发起者
//    DataPool*   pDataPool;
//    DataAction  Action;
//    clStringW   Name;
//    GXLPCVOID   ptr;
//    GXINT       Index;  // 只有动态数组才有效
//  };
//#endif // #ifdef ENABLE_OLD_DATA_ACTION

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
  struct DATAPOOL_VARIABLE_DESC;
  struct DATAPOOL_ENUM_DESC;

#ifdef DEBUG_DECL_NAME
# define DATAPOOL_DECL_STRING_NAME GXLPCSTR Name
#else
# define DATAPOOL_DECL_STRING_NAME
#endif // #ifdef DEBUG_DECL_NAME

  struct DATAPOOL_TYPE_DESC
  {
    DATAPOOL_DECL_STRING_NAME;
    GXUINT       nName;   // 自定位
    TypeCategory Cate;
    GXUINT       cbSize;

    inline GXINT_PTR GetName() const // 返回值依赖于DataPool的Variable类型
    {
#ifdef DEBUG_DECL_NAME
      // 自定位方法和这个成员变量地址相关，必须使用引用或者指针类型来传递这个结构体
      ASSERT((GXLPCSTR)((GXINT_PTR)&nName + nName) == Name || Name == NULL);
#endif // #ifdef DEBUG_DECL_NAME
      return ((GXINT_PTR)&nName + nName);
    }
  };


  struct DATAPOOL_STRUCT_DESC : public DATAPOOL_TYPE_DESC
  {
    GXUINT       Member;  // 自定位
    GXUINT       nMemberCount;

    inline const DATAPOOL_VARIABLE_DESC* GetMembers() const
    {
      return (const DATAPOOL_VARIABLE_DESC*)((GXINT_PTR)&Member + Member);
    }

    inline const DATAPOOL_ENUM_DESC* GetEnumMembers() const
    {
      return (const DATAPOOL_ENUM_DESC*)((GXINT_PTR)&Member + Member);
    }
  };

  struct DATAPOOL_HASHALGO
  {
    u16  eType   : 2; // clstd::StaticStringsDict::HashType
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

  struct DATAPOOL_VARIABLE_DESC
  {
    GXUINT      TypeDesc;           // 减法自定位，指向(TYPE_DESC*)类型
    DATAPOOL_DECL_STRING_NAME;
    GXUINT      nName;              // StringBase偏移
    GXUINT      nOffset;            // 全局变量是绝对偏移，成员变量是结构体内偏移
    GXUINT      nCount   : 30;      // 数组大小,一元变量应该是1,动态数组暂定为0
    GXUINT      bDynamic : 1;       // 应该使用IsDynamicArray()接口
    GXUINT      bConst   : 1;

    inline GXINT_PTR VariableName() const
    {
#ifdef DEBUG_DECL_NAME
      // 自定位方法和这个成员变量地址相关，必须使用引用或者指针类型来传递这个结构体
      ASSERT((GXLPCSTR)((GXINT_PTR)&nName + nName) == Name || Name == NULL);
#endif // #ifdef DEBUG_DECL_NAME
      return ((GXINT_PTR)&nName + nName);
    }

    inline GXINT_PTR TypeName() const
    {
      return GetTypeDesc()->GetName();
    }

    inline const DATAPOOL_TYPE_DESC* GetTypeDesc() const
    {
      const auto pRetTypeDesc = (DATAPOOL_TYPE_DESC*)((GXINT_PTR)&TypeDesc - TypeDesc);
      return pRetTypeDesc;
    }

    inline const DATAPOOL_STRUCT_DESC* GetStructDesc() const
    {
      const DATAPOOL_TYPE_DESC* pTypeDesc = GetTypeDesc();
      ASSERT(
        (pTypeDesc->Cate & T_CATE_MASK) == T_STRUCT ||
        (pTypeDesc->Cate & T_CATE_MASK) == T_ENUM ||
        (pTypeDesc->Cate & T_CATE_MASK) == T_FLAG );
      return static_cast<const DATAPOOL_STRUCT_DESC*>(pTypeDesc);
    }

    inline GXUINT TypeSize() const
    {
      return GetTypeDesc()->cbSize;
    }

    inline TypeCategory GetTypeCategory() const
    {
      return GetTypeDesc()->Cate;
    }

    inline const DATAPOOL_VARIABLE_DESC* MemberBeginPtr() const
    {
      auto pTypeDesc = GetStructDesc();
      return pTypeDesc->GetMembers();
    }

    inline GXUINT MemberCount() const
    {
      auto pTypeDesc = GetStructDesc();
      return pTypeDesc->nMemberCount;
    }

    inline GXBOOL IsDynamicArray() const
    {
      return bDynamic;
    }


  };

  struct DATAPOOL_ENUM_DESC
  {
    DATAPOOL_DECL_STRING_NAME;
    GXUINT      nName;
    GXINT       Value;    // 值
  };
#pragma pack(pop)

  STATIC_ASSERT(sizeof(TypeCategory) == 4);
#ifdef DEBUG_DECL_NAME
#else
  STATIC_ASSERT(sizeof(DATAPOOL_HASHALGO) == 6);
  STATIC_ASSERT(sizeof(DATAPOOL_ENUM_DESC) == 8);
  STATIC_ASSERT(sizeof(DATAPOOL_TYPE_DESC) == 12);
  STATIC_ASSERT(sizeof(DATAPOOL_STRUCT_DESC) == 20);
  STATIC_ASSERT(sizeof(DATAPOOL_VARIABLE_DESC) == 16);
#endif // DEBUG_DECL_NAME

  //////////////////////////////////////////////////////////////////////////



  //////////////////////////////////////////////////////////////////////////

  class DataPoolWatcher : public GUnknown
  {
  public:
#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    GXSTDINTERFACE(GXHRESULT AddRef   ());
    GXSTDINTERFACE(GXHRESULT Release  ());
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    GXSTDINTERFACE(GXVOID OnImpulse   (LPCDATAIMPULSE pImpulse));
  };
//#endif // #ifdef ENABLE_OLD_DATA_ACTION

  //////////////////////////////////////////////////////////////////////////

  class GXDLL DataPool : public GUnknown
  {
    //friend class DataPoolVariable;
    //friend struct iterator;
    //friend struct element_iterator;
    friend struct DataPoolElementReserveIterator;

  public:
    //struct VARIABLE_DESC;
    //struct TYPE_DESC;
    //struct ENUM_DESC;

    typedef GXLPCSTR              LPCSTR;
    typedef const DATAPOOL_VARIABLE_DESC*  LPCVD;
    typedef DataPoolUtility::iterator                 iterator;
    typedef DataPoolUtility::named_iterator           named_iterator;

    typedef GXVOID (GXCALLBACK *ImpulseProc)(DATAPOOL_IMPULSE* pImpulse);

    typedef i32                 Enum;         // 数据池所使用的枚举类型的C++表示
    typedef u32                 Flag;         // 数据池所使用的标志类型的C++表示
    typedef u32                 EnumFlag;     // 枚举和标志类型的统一表示
    typedef clstd::FixedBuffer  clFixedBuffer;
    typedef clstd::RefBuffer    clRefBuffer;
    typedef GXUINT              SortedIndexType;

  protected:

    // COMMENT:
    // GSIT:
    // #.是对一组variable / Enum的nName排序后的索引位置, "一组variable"是指全局变量集合或者结构成员集合
    // #.for each(i)
    //     VarDesc[m_aGSIT[i]].nName 是按照递增顺序增加的,这样成员变量可以使用二分法查找
    // #.m_aGSIT与m_aVariables+m_aMembers按分组顺序对应

    virtual ~DataPool(){};

  public:
#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    virtual GXHRESULT   AddRef              ();
    virtual GXHRESULT   Release             ();
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

    GXSTDINTERFACE(GXBOOL      SaveW               (GXLPCWSTR szFilename));
    GXSTDINTERFACE(GXBOOL      Save                (clFile& file));
    GXSTDINTERFACE(GXBOOL      Load                (clFile& file, GXDWORD dwFlag));

    GXSTDINTERFACE(GXLPCSTR    GetVariableName     (GXUINT nIndex) const); // 获得变量的名字

    // 从stock格式文件或者内存数据导入/导出的接口
    GXSTDINTERFACE(GXHRESULT   ImportDataFromFile  (GXLPCWSTR szFilename));
    GXSTDINTERFACE(GXHRESULT   ImportDataFromFile  (GXLPCSTR szFilename));
    GXSTDINTERFACE(GXHRESULT   ImportDataFromMemory(clstd::Buffer* pBuffer, GXLPCWSTR szRefFilename = NULL));
    GXSTDINTERFACE(GXHRESULT   ExportDataToFile    (GXLPCWSTR szFilename, GXLPCSTR szCodec = NULL)); // "ansi", "unicode"不区分大小写，默认为unicode
    GXSTDINTERFACE(GXHRESULT   ExportDataToFile    (GXLPCSTR szFilename, GXLPCSTR szCodec = NULL));
    GXSTDINTERFACE(GXHRESULT   ExportDataToMemory  (clstd::Buffer* pBuffer, GXLPCSTR szCodec = NULL));

    GXSTDINTERFACE(GXBOOL      IsFixedPool         () const);           // 池中不含有字符串和动态数组
    GXSTDINTERFACE(GXLPVOID    GetFixedDataPtr     ());                 // 必须是RawPool才返回指针
    GXSTDINTERFACE(GXUINT      GetNameId           (LPCSTR szName));    // 返回Type, Variable, Enum等内部稳定字符串的id
    GXSTDINTERFACE(GXBOOL      QueryByName         (GXLPCSTR szName, DataPoolVariable* pVar));
    GXSTDINTERFACE(GXBOOL      QueryByExpression   (GXLPCSTR szExpression, DataPoolVariable* pVar));
    GXSTDINTERFACE(GXBOOL      FindFullName        (clStringA* str, DataPool::LPCVD pVarDesc, clBufferBase* pBuffer, GXUINT nOffset)); // 查找变量全名

#ifndef DISABLE_DATAPOOL_WATCHER
    GXSTDINTERFACE(GXBOOL      IsAutoKnock         ());
    GXSTDINTERFACE(GXBOOL      IsKnocking          (const DataPoolVariable* pVar));
    GXSTDINTERFACE(GXBOOL      SetAutoKnock        (GXBOOL bAutoKnock));

    GXSTDINTERFACE(GXBOOL      Impulse             (const DataPoolVariable& var, DataAction reason, GXSIZE_T index, GXSIZE_T count));
    GXSTDINTERFACE(GXBOOL      Watch               (GXLPCSTR szExpression, ImpulseProc pImpulseCallback, GXLPARAM lParam));
    GXSTDINTERFACE(GXBOOL      Watch               (GXLPCSTR szExpression, DataPoolWatcher* pWatcher));
    GXSTDINTERFACE(GXBOOL      Watch               (GXLPCSTR szExpression, GXHWND hWnd));
    GXSTDINTERFACE(GXBOOL      Watch               (DataPoolVariable* pVar, ImpulseProc pImpulseCallback, GXLPARAM lParam));
    GXSTDINTERFACE(GXBOOL      Watch               (DataPoolVariable* pVar, DataPoolWatcher* pWatcher));
    GXSTDINTERFACE(GXBOOL      Watch               (DataPoolVariable* pVar, GXHWND hWnd));
    GXSTDINTERFACE(GXBOOL      Ignore              (GXLPCSTR szExpression, ImpulseProc pImpulseCallback));
    GXSTDINTERFACE(GXBOOL      Ignore              (GXLPCSTR szExpression, DataPoolWatcher* pWatcher));
    GXSTDINTERFACE(GXBOOL      Ignore              (GXLPCSTR szExpression, GXHWND hWnd));
    GXSTDINTERFACE(GXBOOL      Ignore              (DataPoolVariable* pVar, ImpulseProc pImpulseCallback));
    GXSTDINTERFACE(GXBOOL      Ignore              (DataPoolVariable* pVar, DataPoolWatcher* pWatcher));
    GXSTDINTERFACE(GXBOOL      Ignore              (DataPoolVariable* pVar, GXHWND hWnd));
#endif // #ifndef DISABLE_DATAPOOL_WATCHER

    GXSTDINTERFACE(iterator        begin       ());
    GXSTDINTERFACE(iterator        end         ());
    GXSTDINTERFACE(named_iterator  named_begin ());
    GXSTDINTERFACE(named_iterator  named_end   ());

    //
    // 高级应用
    //
  private:
    // 这里定义了watch的lambda表达式用法所用到了类，内部使用
    template<class _Fn>
    class StaticLambdaWatcher : public DataPoolWatcher
    {
      _Fn m_fn;
      GXHRESULT AddRef()  { return 1; }
      GXHRESULT Release() { return GX_OK; }
      GXVOID OnImpulse(LPCDATAIMPULSE pImpulse) {
        m_fn(pImpulse);
      }
    public:
      StaticLambdaWatcher(_Fn fn) : m_fn(fn){}
    };

  public:
    // 重载了[]符号，用着比较方便，但是不如QueryByExpression性能好.
    DataPoolVariable operator[](GXLPCSTR szExpression);

    // 基于lambda表达式的watch方法，改名为WatchFor是为了防止与Watch系列函数发生不明确的重载函数调用问题
    // _TVarId既可以是GXLPCSTR，也可以是DataPoolVariable*
    // 返回的DataPoolWatcher不要做额外的处理，只用于注销Watcher而用
    template<typename _TVarId, class _Fn>
    DataPoolWatcher* WatchFor(_TVarId Id, _Fn fn)
    {
      static StaticLambdaWatcher<_Fn> s_Watcher(fn);
      if(Watch(Id, &s_Watcher)) {
        return &s_Watcher;
      }
      return NULL;
    }

    //
    // 创建类的各种方法
    //
  public:
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
    static GXHRESULT CreateFromMemory(DataPoolCompiler** ppResolver, GXLPCWSTR szSourceFilePath, DataPoolInclude* pInclude, GXLPCSTR szDefinitionCodes, GXSIZE_T nCodeLength);
  };

  //////////////////////////////////////////////////////////////////////////
  namespace DataPoolUtility
  {

  } // namespace DataPoolUtility

} // namespace Marimo

#endif // #ifndef _MARIMO_DATA_POOL_H_