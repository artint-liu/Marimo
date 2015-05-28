#ifndef _MARIMO_DATAPOOL_IMPLEMENT_H_
#define _MARIMO_DATAPOOL_IMPLEMENT_H_

class SmartStockW;

namespace Marimo
{
  struct DataPoolBuildTime;
  class DataPoolVariableImpl;

  // �ڲ�ʵ�ֵĺ�����
  struct DataPoolVariable::VTBL
  {
    typedef DataPoolVariable Variable;
    typedef DataPoolVariableImpl VarImpl;


    GXUINT    (*GetSize     )(GXCONST VarImpl* pThis);  // �ֽڴ�С, �����������С, ��̬���ݴ�С�ɱ�, �ṹ���ǽṹ���С
    //GXUINT    (*GetOffset   )(GXCONST VarImpl* pThis);  // ƫ��,ȫ�ֱ�����ȫ��ƫ��, �ṹ������ǽṹ����ƫ��
    //GXLPCSTR  (*GetName     )(GXCONST VarImpl* pThis);  // ��ö�����, ������, ������������߽ṹ�������
    //GXLPCSTR  (*GetTypeName )(GXCONST VarImpl* pThis);  // ����, ����Ϊ������, ����Ϊ"Type[n]"��ʽ, ��̬����Ϊ"Type[]"��ʽ, �ṹ��Ϊ"struct Name"��ʽ

    // �ṹ��ר��
    Variable  (*GetMember   )(GXCONST VarImpl* pThis, GXLPCSTR szName);    // ��ó�Ա

    // �����̬����ר�� 
    Variable  (*GetIndex    )(GXCONST VarImpl* pThis, int nIndex);         // ����ض������ı���
    GXUINT    (*GetLength   )(GXCONST VarImpl* pThis);                     // �������ĳ�Ա����, ע����GetSize����
    Variable  (*NewBack     )(        VarImpl* pThis, GXUINT nIncrease);   // �ڶ�̬������׷������, ��̬����ר��
    GXBOOL    (*Remove      )(        VarImpl* pThis, GXUINT nIndex, GXUINT nCount);      // �Ƴ���̬����ָ������������, ��̬����ר��

    // ����ר��
    //clStringW (*ToStringW   )(GXCONST VarImpl* pThis);                   // ���������京��תֵ, ����ͽṹ���ͬ��GetTypeName()
    //clStringA (*ToStringA   )(GXCONST VarImpl* pThis);
    GXBOOL    (*ParseW      )(        VarImpl* pThis, GXLPCWSTR szString, GXUINT length); // ���ձ�������תֵ(unicode)
    GXBOOL    (*ParseA      )(        VarImpl* pThis, GXLPCSTR szString, GXUINT length);  // ���ձ�������תֵ
    u32       (*ToInteger   )(GXCONST VarImpl* pThis);
    u64       (*ToInt64     )(GXCONST VarImpl* pThis);
    float     (*ToFloat     )(GXCONST VarImpl* pThis);
    clStringW (*ToStringW   )(GXCONST VarImpl* pThis);
    clStringA (*ToStringA   )(GXCONST VarImpl* pThis);
    GXBOOL    (*SetAsInteger)(        VarImpl* pThis, u32 val);             // �����������32λ�ᱻ�ض�
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

    struct VAR_COUNT // ��������ʱ����variable�����ʹ�õĽṹ��
    {
      MOVariable var;
      GXUINT     nCount;
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
      inline GXUINT GetMemberIndex(DataPoolImpl::LPCVD aGlobalMemberTab) const  // ����Լ���Ա������ȫ�ֳ�Ա�������λ��
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
        ASSERT(IsDynamicArray()); // ��̬����
        return (DataPoolArray**)(pBaseData + nOffset);
      }

      DataPoolArray* CreateAsBuffer(DataPoolImpl* pDataPool, clBufferBase* pParent, GXBYTE* pBaseData, int nInitCount) const
      {
        ASSERT(IsDynamicArray()); // һ���Ƕ�̬����
        ASSERT(nInitCount >= 0);

        DataPoolArray** ppBuffer = GetAsBufferPtr(pBaseData);  // ��̬����
        if(*ppBuffer == NULL && TEST_FLAG_NOT(pDataPool->m_dwRuntimeFlags, RuntimeFlag_Readonly))
        {
          // ����ArrayBufferֻ��ʹ��ָ����ʽ
          *ppBuffer = new DataPoolArray(TypeSize() * 10);  // ʮ�����ʹ�С
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
        // ֻ��ģʽ�£�δʹ�õĶ�̬����pBuffer�п�����NULL, �������ﲻ���pBuffer
        return vtbl && pVdd;
      }
    };

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



  public:
    DataPoolImpl(GXLPCSTR szName);
    virtual ~DataPoolImpl();

    virtual GXBOOL SaveW( GXLPCWSTR szFilename );
    virtual GXBOOL Save( clFile& file );
    virtual GXBOOL Load( clFile& file, GXDWORD dwFlag );

    //virtual GXHRESULT ImportDataFromFileW (GXLPCWSTR szFilename);
    virtual LPCSTR    GetVariableName     (GXUINT nIndex) const; // ��ñ���������
    virtual GXHRESULT GetLayout           (GXLPCSTR szStructName, DataLayoutArray* pLayout);

    virtual GXBOOL    IsFixedPool         () const; // ���в������ַ����Ͷ�̬����
    virtual GXLPVOID  GetFixedDataPtr     (); // ������RawPool�ŷ���ָ��
    virtual GXBOOL    QueryByName         (GXLPCSTR szName, DataPoolVariable* pVar);
    virtual GXBOOL    QueryByExpression   (GXLPCSTR szExpression, DataPoolVariable* pVar);
    virtual GXBOOL    FindFullName        (clStringA* str, DataPool::LPCVD pVarDesc, clBufferBase* pBuffer, GXUINT nOffset); // ���ұ���ȫ��

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

    const clBufferBase* IntGetEntryBuffer   () const; // ������ݳ��������buffer
    LPCTD         FindType            (GXLPCSTR szTypeName) const;
    void          CopyVariables       (VARIABLE_DESC* pDestVarDesc, GXLPCVOID pSrcVector, const clstd::STRINGSETDESC* pTable, GXINT_PTR lpBase);
    GXBOOL        IntCreateUnary      (clBufferBase* pBuffer, LPCVD pThisVdd, VARIABLE* pVar);
    GXBOOL        IntQuery            (GXINOUT VARIABLE* pVar, GXLPCSTR szVariableName, GXUINT nIndex);
    GXINT         IntQueryByExpression(GXLPCSTR szExpression, VARIABLE* pVar);
#ifdef ENABLE_DATAPOOL_WATCHER
    //int           FindWatcher         (DataPoolWatcher* pWatcher);
    //int           FindWatcherByName   (GXLPCSTR szClassName);
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
      //WatcherArray        m_aWatchers;
      typedef clset<GXLPCVOID> KnockingSet;
      KnockingSet         m_ImpulsingSet;    // ��¼���ڷ��͸���֪ͨ��Variable�б�,��ֹ�����ָͬ���Variable�����ݹ�.
      // COMMENT:
      // KnockingSetĿǰ������Varָ������ĵ�ַ,��̬��������������
      // ���Ƕ�̬�����������,��OnKnockʱ��̬��������Ԫ�ؿ���
      // �ᵼ�������ַ�仯, �������var->GetPtr�޷��ҵ���ɾ��KnockingSet�е�Ԫ��


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
  }; // class DataPoolImpl

  // ������������ͣ���ת������TRUE�����򷵻�FALSE
  // ֧��0xʮ�����ƣ�0b�����ƣ�0��ͷ�İ˽��ƺ�ʮ����
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