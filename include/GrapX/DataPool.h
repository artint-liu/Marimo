#ifndef _MARIMO_DATA_POOL_H_
#define _MARIMO_DATA_POOL_H_

// ���뿪��
#define ENABLE_DATAPOOL_WATCHER     // DataPool ������
//#define DEBUG_DECL_NAME           // ʹ���ַ���ָ�봢���Զ�λ�����������������Խ����ܱ���ͼ���

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
    T_UNDEFINE = 0, // ��β�õ�
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
    T_ENUM,         // ö��
    T_FLAG,         // ��־��ö��
    T_STRUCT,
    T_STRUCTALWAYS, // ��ʹ�����û��ʹ��, Ҳ�ᱣ�����ĸ�ʽ
    T_MAX,
  };

  enum DataPoolLoad
  {
    DataPoolLoad_ReadOnly = 0x00000001,
  };

  // ��û����
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
    DATACT_Deleting,  // ���ڸı䣬��Ա��ûɾ
    DATACT_Deleted,   // ɾ�����֪ͨ
  };

//#define ENABLE_OLD_DATA_ACTION
#ifdef ENABLE_OLD_DATA_ACTION
  struct KNOCKACTION
  {
    const DataPoolVariable* pSponsor; // ������
    DataPool*   pDataPool;
    DataAction  Action;
    clStringW   Name;
    GXLPCVOID   ptr;
    GXINT       Index;  // ֻ�ж�̬�������Ч
  };
#endif // #ifdef ENABLE_OLD_DATA_ACTION

  struct DATAPOOL_IMPULSE
  {
    const DataPoolVariable* sponsor; // ������
    DataAction reason;  // ���ݸı�����
    GXUINT     index;   // ���������Ԫ�أ���ǿ�ʼ����
    GXUINT     count;   // ���������Ԫ�أ����Ԫ������
    GXLPARAM   param;   // �û�����
  };
  typedef const DATAPOOL_IMPULSE* LPCDATAIMPULSE;

  const GXDWORD VarDeclFlag_Const = 0x0001;

  //////////////////////////////////////////////////////////////////////////

  // DataPool ���������ṹ��
  struct VARIABLE_DECLARATION
  {
    GXLPCSTR  Type;       // ��������
    GXLPCSTR  Name;       // ������
    GXDWORD   Flags;      // ��־,"VarDeclFlag_*"
    GXINT     Count;      // ���ȶ�,0,1��ʾ1��Ԫ��
                          // ����1��ʾn������
                          // С��0��ʾ�䳤����,�䳤��������ݲ���ͳһ��ַ��
                          // ��̬�����ʼ��ʱ, 
                          //   ���Init��Ϊ��, ���ʼ�Ķ�̬�����СΪabs(Count);
                          //   ���InitΪ��, ���ʼ��̬�����СΪ0.
    GXLPVOID  Init;       // ��ʼֵ,�����Ϊ NULL, count �ľ���ֵ����Ԫ�ظ���,
                          // �������㹻���ȵ�����, �������string����
  };
  typedef const VARIABLE_DECLARATION* LPCVARDECL;

  // DataPool ö�������ṹ��
  struct ENUM_DECLARATION // ö�٣���־��ö�ٶ������
  {
    GXLPCSTR  Name;       // ������
    GXINT     Value;      // ֵ������int��
  };
  typedef const ENUM_DECLARATION* LPCENUMDECL;

  // ���������ṹ��
  struct TYPE_DECLARATION
  {
    TypeCategory          Cate;
    GXLPCSTR              Name;
    union {
      VARIABLE_DECLARATION* Struct; // �ṹ�� ��Ա
      ENUM_DECLARATION*     Enum;
    }as;
  };
  typedef const TYPE_DECLARATION* LPCTYPEDECL;

  //////////////////////////////////////////////////////////////////////////

  // �ڲ����������ṹ��, ͬʱҲ���ļ�����Ľṹ������
  // �ַ�����Ա����ʹ�����Զ�λ��������������ڽṹ���������ڴ��е�λ��
  // ����ʹ��ʱҪע����ڲ�ȡ��������Ҫʹ��ָ�룬���ܸ���ʵ����
  // ��ʹ�����÷���ʱ��release�����Ż�Ҳ���ܸı���ʱ�������ڴ�λ�ö�����Ҫע��

#pragma pack(push, 1)
  struct DATAPOOL_TYPE_DESC
  {
#ifdef DEBUG_DECL_NAME
    GXLPCSTR     Name;
#endif
    GXUINT       nName;   // �Զ�λ
    TypeCategory Cate;
    GXUINT       cbSize;
    GXUINT       Member;  // �Զ�λ
    //GXUINT       nMemberIndex; // �� m_aStructMember �� m_aEnumPck �е�����
    GXUINT       nMemberCount;
    // TODO: nMemberIndexҲ��Ϊ�Զ�λ��ʽ
  };

  struct DATAPOOL_VARIABLE_DESC
  {
    GXUINT      TypeDesc;           // �����Զ�λ��ָ��(TYPE_DESC*)����
#ifdef DEBUG_DECL_NAME
    GXLPCSTR    Name;
#endif // DEBUG_DECL_NAME
    GXUINT      nName;              // StringBaseƫ��
    GXUINT      nOffset;            // ȫ�ֱ����Ǿ���ƫ�ƣ���Ա�����ǽṹ����ƫ��
    GXUINT      nCount   : 30;      // �����С,һԪ����Ӧ����1,��̬�����ݶ�Ϊ0
    GXUINT      bDynamic : 1;       // Ӧ��ʹ��IsDynamicArray()�ӿ�
    GXUINT      bConst   : 1;
  };

  struct DATAPOOL_ENUM_DESC
  {
#ifdef DEBUG_DECL_NAME
    GXLPCSTR    Name;     // ����
#endif // DEBUG_DECL_NAME
    GXUINT      nName;
    GXINT       Value;    // ֵ
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

    DataPoolArray(GXLPBYTE pPlacement, u32 nSize) // placement new ר��
    {      
      ASSERT((GXINT_PTR)this == (GXINT_PTR)pPlacement); // ����֤�ã�ûʵ������

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
      RuntimeFlag_Fixed     = 0x00000001,   // ֻҪ���ֶ�̬���飬object����string����Ϊfalse
      RuntimeFlag_Readonly  = 0x00000002,   // ֻ��ģʽ�����ţ�����ڣ�����һ�ж���һ���ڴ��ϣ�����������
#ifdef ENABLE_DATAPOOL_WATCHER
      RuntimeFlag_AutoKnock = 0x00000004,
#endif // #ifdef ENABLE_DATAPOOL_WATCHER
    };


    struct TYPE_DESC : DATAPOOL_TYPE_DESC
    {
      inline DataPool::LPCSTR GetName() const
      {
#ifdef DEBUG_DECL_NAME
        // �Զ�λ�����������Ա������ַ��أ�����ʹ�����û���ָ����������������ṹ��
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

      inline GXUINT GetMemberIndex(LPCVD aGlobalMemberTab) const  // ����Լ���Ա������ȫ�ֳ�Ա�������λ��
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
        ASSERT(IsDynamicArray()); // ��̬����
        return (DataPoolArray**)(pBaseData + nOffset);
      }

      DataPoolArray* CreateAsBuffer(DataPool* pDataPool, GXBYTE* pBaseData, int nInitCount) const
      {
        ASSERT(IsDynamicArray()); // һ���Ƕ�̬����

        DataPoolArray** ppBuffer = GetAsBufferPtr(pBaseData);  // ��̬����
        if(*ppBuffer == NULL && TEST_FLAG_NOT(pDataPool->m_dwRuntimeFlags, RuntimeFlag_Readonly))
        {
          //ASSERT(nInitCount >= 1);
          // ����ArrayBufferֻ��ʹ��ָ����ʽ
          *ppBuffer = new DataPoolArray(TypeSize() * 10);  // ʮ�����ʹ�С
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
        // �Զ�λ�����������Ա������ַ��أ�����ʹ�����û���ָ����������������ṹ��
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
        ASSERT(GetTypeCategory() == T_STRING); // Unicode �ַ���
        return (clStringW*)(pBaseData + nOffset);
      }

      clStringA* GetAsStringA(GXBYTE* pBaseData) const
      {
        ASSERT(GetTypeCategory() == T_STRINGA); // ANSI �ַ���
        return (clStringA*)(pBaseData + nOffset);
      }

      GXUINT GetUsageSize() const // ����ʱ���ڴ�ߴ磬��̬������32/64λ�²�һ��
      {
        if(IsDynamicArray()) {
          return sizeof(clBuffer*);
        }
        return GetSize();
      }

      GXUINT GetSize() const  // �ȶ��ı��������ߴ磬����GetMemorySize()
      {
        ASSERT( ! IsDynamicArray()); // ��Ӧ���Ƕ�̬����
        return nCount * TypeSize();
      }

      VTBL* GetUnaryMethod() const;
      VTBL* GetMethod() const;
    };

    // ö�ٳ�Ա
    struct ENUM_DESC : DATAPOOL_ENUM_DESC
    {
      inline DataPool::LPCSTR GetName() const
      {
#ifdef DEBUG_DECL_NAME
        // �Զ�λ�����������Ա������ַ��أ�����ʹ�����û���ָ����������������ṹ��
        ASSERT((DataPool::LPCSTR)((GXINT_PTR)&nName + nName) == Name || Name == NULL);
#endif // #ifdef DEBUG_DECL_NAME
        return (DataPool::LPCSTR)((GXINT_PTR)&nName + nName);
      }
    };
    //typedef const ENUM_DESC* LPCENUMDESC;

    
    struct VARIABLE // �����ڲ���ѯ���ݵĽṹ��
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
    struct FILE_HEADER // �ļ�ͷ
    {
      GXDWORD dwFlags;          // ��־��64λָ��

      GXUINT  nBufHeaderOffset; // ���ݻ���ͷ������global var��������ʼ��ַ
      GXUINT  nDescOffset;      // ����ʱ������Ϣ�����ļ��е�ƫ��, ����Buffer���ļ���д���λ��
      GXUINT  nStringVarOffset; // �ַ����������ļ���ƫ��
      GXUINT  nBuffersOffset;   // Buffer�������ļ��е�ƫ��

      GXUINT  nNumOfTypes;      // ������������
      GXUINT  nNumOfVar;        // ������������
      GXUINT  nNumOfMember;     // ��Ա������������
      GXUINT  nNumOfEnums;      // ö����������
      GXUINT  cbDescTabNames;   // ����������������(�ַ���)ռ�õ��ܳߴ�

      GXUINT  cbVariableSpace;  // ȫ�ֱ���ռ�õĿռ�
      GXUINT  nNumOfPtrVars;    // ȫ�ֱ����а�����ָ�����������ڲ�ͬƽ̨�µ���cbVariableSpace�ĳߴ�

      GXUINT  nNumOfStrings;    // �ַ�����������
      GXUINT  cbStringSpace;    // �ַ�������ռ�õĿռ�

      GXUINT  nNumOfArrayBufs;  // ��̬���������
    };

    struct FILE_BUFFERHEADER  // �ļ�Buffer��Ϣͷ
    {
      GXUINT nBufferSize;
      GXUINT nNumOfRel;
      GXUINT nType; // 0�Ļ���ȫ�ֱ�����buffer
    };
#pragma pack(pop)

    typedef i32                 Enum;         // ���ݳ���ʹ�õ�ö�����͵�C++��ʾ
    typedef u32                 Flag;         // ���ݳ���ʹ�õı�־���͵�C++��ʾ
    typedef u32                 EnumFlag;     // ö�ٺͱ�־���͵�ͳһ��ʾ
    typedef clstd::FixedBuffer  clFixedBuffer;
    typedef clstd::RefBuffer    clRefBuffer;
    typedef DataPoolBuildTime   BUILDTIME;
    typedef GXUINT              SortedIndexType;

#ifdef ENABLE_DATAPOOL_WATCHER
    typedef clvector<DataPoolWatcher*>  WatcherArray;
#endif // #ifdef ENABLE_DATAPOOL_WATCHER



  protected:
    clStringA           m_Name;             // ����Ǿ�������Ļ�������DataPool������

    // LocalizePtr������Щ�����ض�λ�����ָ��
    clFixedBuffer       m_Buffer;
    GXUINT              m_nNumOfTypes;
    GXUINT              m_nNumOfVar;
    GXUINT              m_nNumOfMember;
    GXUINT              m_nNumOfEnums;
    // =====================

    // ��Щ���Ա�LocalizePtr�����ض�λ
    TYPE_DESC*          m_aTypes;
    SortedIndexType*    m_aGSIT;            // Grouped sorted index table, ��ϸ����
    VARIABLE_DESC*      m_aVariables;       // ���б���������
    VARIABLE_DESC*      m_aMembers;         // ���еĽṹ���Ա�������������ű���
    ENUM_DESC*          m_aEnums;           // ����ö�ٳ�Ա�����������
    // =====================

    clRefBuffer         m_VarBuffer;        // �����ռ俪ʼ��ַ, ���ָ����m_Buffer

#ifdef _DEBUG
    GXUINT              m_nDbgNumOfArray;   // ��̬����Ļ�����
    GXUINT              m_nDbgNumOfString;  // ��̬����Ļ�����
#endif // #ifdef _DEBUG

#ifdef ENABLE_DATAPOOL_WATCHER
    WatcherArray        m_aWatchers;
    typedef clset<GXLPCVOID> KnockingSet;
    KnockingSet         m_setKnocking;    // ��¼���ڷ��͸���֪ͨ��Variable�б�,��ֹ�����ָͬ���Variable�����ݹ�.

    struct WATCH_FIXED // �̶�����������
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
    // KnockingSetĿǰ������Varָ������ĵ�ַ,��̬��������������
    // ���Ƕ�̬�����������,��OnKnockʱ��̬��������Ԫ�ؿ���
    // �ᵼ�������ַ�仯, �������var->GetPtr�޷��ҵ���ɾ��KnockingSet�е�Ԫ��

    // GSIT:
    // #.�Ƕ�һ��variable / Enum��nName����������λ��, "һ��variable"��ָȫ�ֱ������ϻ��߽ṹ��Ա����
    // #.for each(i)
    //     VarDesc[m_aGSIT[i]].nName �ǰ��յ���˳�����ӵ�,������Ա��������ʹ�ö��ַ�����
    // #.m_aGSIT��m_aVariables+m_aMembers������˳���Ӧ


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

    const clBufferBase* IntGetEntryBuffer   () const; // ������ݳ��������buffer
    LPCTD         FindType            (GXLPCSTR szTypeName) const;
    void          CopyVariables       (VARIABLE_DESC* pDestVarDesc, GXLPCVOID pSrcVector, const clstd::STRINGSETDESC* pTable, GXINT_PTR lpBase);
    GXBOOL        IntCreateUnary      (clBufferBase* pBuffer, LPCVD pThisVdd, int nOffsetAdd, VARIABLE* pVar);
    GXBOOL        IntQuery            (clBufferBase* pBuffer, LPCVD pParentVdd, GXLPCSTR szVariable, int nOffsetAdd, VARIABLE* pVar);
    GXBOOL        IntQueryByExpression(GXLPCSTR szExpression, VARIABLE* pVar);
#ifdef ENABLE_DATAPOOL_WATCHER
    int           FindWatcher         (DataPoolWatcher* pWatcher);
    int           FindWatcherByName   (GXLPCSTR szClassName);
#endif // #ifdef ENABLE_DATAPOOL_WATCHER
    //LPCENUMDESC   IntGetEnum          (GXUINT nPackIndex) const;  // m_aEnumPck�е�����
    LPCVD         IntFindVariable     (LPCVD pVarDesc, int nCount, GXUINT nOffset);
    GXBOOL        IntWatch            (DataPoolVariable* pVar, ImpulseProc pImpulseCallback, GXLPARAM lParam);
    GXBOOL        IntIgnore           (DataPoolVariable* pVar, ImpulseProc pImpulseCallback, GXLPARAM lParam);

    GXSIZE_T      IntGetRTDescHeader    ();   // �������ʱ�������С
    GXSIZE_T      IntGetRTDescNames     ();   // �������ʱ�������ַ�������ռ�Ĵ�С
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

    virtual GXLPCSTR    GetVariableName     (GXUINT nIndex) const; // ��ñ���������
#ifdef DATAPOOLCOMPILER_PROJECT
#else
    virtual GXHRESULT   GetLayout           (GXLPCSTR szStructName, DataLayoutArray* pLayout);
#endif // #ifdef DATAPOOLCOMPILER_PROJECT
    virtual GXHRESULT   ImportDataFromFileW (GXLPCWSTR szFilename) = 0;

    virtual GXBOOL      IsFixedPool         (); // ���в������ַ����Ͷ�̬����
    virtual GXLPVOID    GetFixedDataPtr     (); // ������RawPool�ŷ���ָ��
    virtual GXBOOL      QueryByName         (GXLPCSTR szName, DataPoolVariable* pVar);
    virtual GXBOOL      QueryByExpression   (GXLPCSTR szExpression, DataPoolVariable* pVar);
    virtual GXBOOL      FindFullName        (clStringA* str, const VARIABLE_DESC* pVarDesc, clBufferBase* pBuffer, GXUINT nOffset); // ���ұ���ȫ��

#ifdef ENABLE_DATAPOOL_WATCHER
    virtual GXBOOL      IsAutoKnock         ();
    virtual GXBOOL      IsKnocking          (const DataPoolVariable* pVar);
    virtual GXBOOL      SetAutoKnock        (GXBOOL bAutoKnock);

#ifdef ENABLE_OLD_DATA_ACTION
    virtual GXHRESULT   CreateWatcher       (GXLPCSTR szClassName);
    virtual GXHRESULT   RemoveWatcherByName (GXLPCSTR szClassName);
    virtual GXHRESULT   AddWatcher          (DataPoolWatcher* pWatcher);
    virtual GXHRESULT   RemoveWatcher       (DataPoolWatcher* pWatcher);
    virtual GXHRESULT   RegisterIdentify    (GXLPCSTR szClassName, GXLPVOID pIdentify); // TODO: ������Ĳ���
    virtual GXHRESULT   UnregisterIdentify  (GXLPCSTR szClassName, GXLPVOID pIdentify); // TODO: ������Ĳ���
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

    // TODO: �������DynamicArray_Removeʹ��
    static  GXHRESULT   FindDataPool        (DataPool** ppDataPool, GXLPCSTR szName);
    static  GXHRESULT   FindVariable        (DataPool** ppDataPool, DataPoolVariable* pVar, GXLPCSTR szGlobalExpession);
    static  GXHRESULT   CreateDataPool      (DataPool** ppDataPool, GXLPCSTR szName/*= NULL*/, const TYPE_DECLARATION* pTypeDecl, const VARIABLE_DECLARATION* pVarDecl);
    static  GXHRESULT   CreateFromResolver  (DataPool** ppDataPool, GXLPCSTR szName/*= NULL*/, DataPoolCompiler* pResolver);
    static  GXHRESULT   CreateFromFileW     (DataPool** ppDataPool, GXLPCSTR szName/*= NULL*/, GXLPCWSTR szFilename, GXDWORD dwFlag);
    static  GXHRESULT   CompileFromMemory   (DataPool** ppDataPool, GXLPCSTR szName/*= NULL*/, DataPoolInclude* pInclude, GXLPCSTR szDefinitionCodes, GXSIZE_T nCodeLength = 0);
    static  GXHRESULT   CompileFromFileW    (DataPool** ppDataPool, GXLPCSTR szName/*= NULL*/, GXLPCWSTR szFilename, DataPoolInclude* pInclude = NULL);
    static  GXBOOL      IsIllegalName       (GXLPCSTR szName); // �������/���������Ƿ����Ҫ��

    // ע�ⲻ֧�� struct ���԰���, ֧�ֶ�̬������԰�������:
    // struct A
    //{
    //  A a[];    // ֧����������, ��ʾ���Ƕ�̬����,������ʽ��������֧��, ��������.
    //  A ab[10]; // ��֧����������, ���������޵ݹ�����ĳ���
    //};

    // ���ڶ�̬����,�κγ�Ա��ȡַ"&"�����������ǲ���ȫ��, �����ַ����Ϊ��̬�����Ա����ɾ���ı�

    // ��(DataPool)�д����ֵ:
    // �������ֺͽṹ��, ����ȫ������˳��ֲ�, ���ְ���������Сռ��Ӧ�ֽ�,�ṹ�尴�ո�����Ա����ռ�������Ŀռ�.
    // ���ڹ̶����ȵ�����(��������,�ṹ��������߽ṹ��������), ����չ��������˳��ֲ�,ռ�ÿռ����ͬ��.
    // �����ַ���(string)����, ��������˳��ֲ�,ռ��4�ֽ�, ָ���ַ�����ַ, �����ַ�ǿɱ��.
    // ���ڶ�̬��������, ����ȫ������˳��ֲ�,ռ��4�ֽ�,������(clBuffer*)��ָ��.
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
    // Returns:   �������ڱ�ʶ��ͬWatcher���ַ���
    // Qualifier:
    // Parameter: clStringA GetClassName 
    //************************************
    GXSTDINTERFACE(clStringA GetClassName       ());

    //************************************
    // Method:    RegisterPrivate
    // FullName:  RegisterPrivate
    // Access:    public 
    // Returns:   �ɹ�����GX_OK,���򷵻�GX_FAIL
    // Qualifier: ����ע��Watcher˽�����ݵĽӿ�, ����UI Watcher���HWND
    //            ע������ͬ��Watcher�ڲ�,������Ϊÿ��HWND����һ��Watcher,
    //            �����һ��ר�õ�WatcherҲ���Բ�ȥʵ������ӿڵĹ���.
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
  // DataPool ������ʹ�õ�ͷ�ļ��򿪷���
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
  // DataPool ������
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