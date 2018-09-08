#ifndef _EXPRESSION_PARSER_H_
#define _EXPRESSION_PARSER_H_

#define ENABLE_SYNTAX_VERIFY // �﷨��鿪��, ֻ�������﷨������ʱ������, ��ʽ�治Ҫ�ر����

namespace UVShader
{
  class CodeParser;
  typedef clvector<clStringA> StringArray;

  class Include
  {
  public:
    enum IncludeType {
      IncludeType_System,
      IncludeType_Local,
    };

    GXSTDINTERFACE(GXHRESULT Open(IncludeType eIncludeType, GXLPCWSTR szFileName, clBuffer** ppBuffer));
    GXSTDINTERFACE(GXHRESULT Close(clBuffer* pBuffer));
  };


  class DefaultInclude : public Include
  {
  public:
    GXHRESULT Open(IncludeType eIncludeType, GXLPCWSTR szFileName, clBuffer** ppBuffer) override;
    GXHRESULT Close(clBuffer* pBuffer) override;
  };

  enum KeywordFilter
  {
    KeywordFilter_break    = 0x0001,
    KeywordFilter_continue = 0x0002,
    KeywordFilter_case     = 0x0004,
    KeywordFilter_return   = 0x0008,    // �з���ֵ�ĺ����ſ������
    KeywordFilter_typedef  = 0x0010,

    KeywordFilter_All = 0xffffffff,
    KeywordFilter_InFuntionArgument = 0,
    KeywordFilter_InFuntion = KeywordFilter_typedef,
    //KeywordMask_FlowsControl,
  };

  struct TYPEDESC
  {
    typedef cllist<const TYPEDESC*> CPtrList;
    enum TypeCate
    {
      TypeCate_Empty,
      TypeCate_Void,
      TypeCate_FloatScaler,    // ����-��������
      TypeCate_IntegerScaler,  // ����-��������
      TypeCate_MultiDim,  // ��ά����
      TypeCate_String,
      TypeCate_Sampler1D,
      TypeCate_Sampler2D,
      TypeCate_Sampler3D,
      TypeCate_SamplerCube,
      //TypeCate_Scaler, // TODO: ��TypeCate_FloatNumeric��TypeCate_IntegerNumeric�ظ�
      TypeCate_Vector,
      TypeCate_Matrix,
      TypeCate_Struct,
      TypeCate_Flag_Scaler   = 0x10000,
      TypeCate_Flag_MultiDim = 0x20000,
      TypeCate_Flag_Sampler  = 0x40000,
      TypeCate_Flag_Struct   = 0x80000,
    };
    typedef clvector<size_t> DimList_T;

    TypeCate            cate;
    NameContext* const  pNameCtx;
    clStringA           name; // ������
    COMMINTRTYPEDESC*   pDesc; // �ṹ��(����) ������, ��ά�������ָ�����Ļ�������
    const SYNTAXNODE*   pMemberNode; // �ṹ��(�û�����) ������, ������"SYNTAXNODE::MODE_Block"
    DimList_T           sDimensions; // ά���б� int var[a][b][c][d] ����Ϊ{d��c��b��a}
    const TYPEDESC*     pElementType; // float[3][2]�� pElementType=float[2]��float3[2]��pElementType=float3��float3��pElementType=float��ֻ����ѧ���Ͳ���

    CLogger* GetLogger() const;
    static GXBOOL MatchScaler(const TOKEN* ptkMember, GXLPCSTR scaler_set); // ��֤.xxxx, .xyxy, .yxwz����Ҳ�Ǻ���ĳ�Ա
    GXLPCSTR Resolve(int& R, int& C) const;
    GXBOOL IsVector() const;
    GXBOOL IsMatrix() const;
    GXBOOL IsSameType(const TYPEDESC* pOtherTypeDesc) const;
    GXBOOL GetMemberTypeList(TYPEDESC::CPtrList& sMemberTypeList) const;
    size_t CountOf() const; // ����������������������Ļ�����������
    const TYPEDESC* ConfirmArrayCount(size_t nCount) const;
  };

  struct FUNCDESC // �û�����ĺ�������
  {
    // FIXME: ��Ϊ����˳���ϵ, ����ֵ���β�Ӧ�Ĵ���TYPEDESC, ����������, ���Ҫ����ʱ����
    clStringA         ret_type;     // ���ص�����
    clStringA         name;         // ������
    TOKEN::PtrCArray  sFormalTypes; // ����, �β����ͱ�
  };

  struct INTRINSIC_FUNC
  {
    enum RetType
    {
      RetType_Scaler0 = -1,
      //RetType_FromName = -2, // ���������Ǻ�����
      //RetType_Vector4 = -3,  // 4ά����,һ�������ɫ
      RetType_Bool = -3,
      RetType_Float4 = -4,  // 4ά����,һ�������ɫ
      RetType_Last = -5,
      RetType_Argument0 = 0, // ��������ͬ��һ����������
      RetType_Argument1 = 1, // ��������ͬ�ڶ�����������
    };

    enum ArgMask
    {
      ArgMask_Scaler      = 0x01,
      ArgMask_Vector      = 0x02,
      ArgMask_Matrix      = 0x04,
      ArgMask_Sampler1D   = 0x08,
      ArgMask_Sampler2D   = 0x10,
      ArgMask_Sampler3D   = 0x20,
      ArgMask_SamplerCube = 0x40,
      ArgMask_Out         = 0x80,
      ArgMask_TemplType   = 0xe000,
      ArgMask_TemplShift  = 13,
    };

    GXLPCSTR  name;   // ������
    int       type;   // 0 ��ʾ���һ��������ͬ, 1��ʾ��ڶ���������ͬ, �Դ�����
                      // -1��ʾ��һ�������ı���ֵ, �����һ��������int3, �򷵻�ֵ��int
    size_t    count;  // ��������
    u16*      params;

    static int GetTypeTemplateTypeIndex(GXDWORD dwMasks);
  };

  // ����ʵ�������;������ϳ�ʼ��
  // ��: float3(pos.xy, 1)
  //     float4(pos.xy, 1, 1)
  //     float4(0, pos.xy, 1)
  struct PERCOMPONENTMATH // Per-Component Math Operations
  {
    GXLPCSTR name;
    int scaler_count;
  };

  enum ValueResult
  {
    ValueResult_Undefined = -1,
    ValueResult_OK = 0,
    ValueResult_NotConst,
    ValueResult_NotStructMember,
    ValueResult_3861, // �Ҳ�����ʶ��
    ValueResult_5039, // ��������������ƥ��
  };

  struct VARIDESC
  {
    const TYPEDESC* pDesc;
    size_t nOffset;
    VALUE sConstValue;      // �涨 string ���Ͳ����� const ���Σ�����������VALUE
    SYNTAXNODE::GLOB  glob; // ���г�����Ӧ�ö������ֵ���������ͳ���ͬʱ��Ӧ�ö���sConstValue
  };

  enum InputModifier // ������������
  {
    InputModifier_in,
    InputModifier_out,
    InputModifier_inout,
    InputModifier_uniform, // Input only constant data
  };

  struct FUNCTION_ARGUMENT // ��������
  {
    InputModifier eModifier;  // [opt]
    const TOKEN*  ptkType;    // [req]
    const TOKEN*  ptkName;    // [req]
    GXLPCSTR      szSemantic; // [opt]
  };

  struct VALUE_CONTEXT
  {
    // [����]
    const NameContext&  name_ctx;
    GXBOOL              bNeedValue;
    CLogger*            pLogger;

    // [ֵ]
    ValueResult     result;
    const TYPEDESC* pType;
    const VALUE*    pValue;
    size_t          count;
    ValuePool       pool;

    VALUE_CONTEXT(const VALUE_CONTEXT& vctx);
    VALUE_CONTEXT(const NameContext& _name_ctx);
    VALUE_CONTEXT(const NameContext& _name_ctx, GXBOOL _bNeedValue);

    void SetProperty(const VALUE_CONTEXT& vctx); // ��vctx��������
    void ClearValue();
  };

  class NameContext
  {
  public:
    struct VALUEDESC
    {
      const VALUE* pValue;
      size_t count;
    };

    typedef clmap<clStringA, TYPEDESC>  TypeMap;
    typedef std::multimap<clStringA, FUNCDESC>  FuncMap;
    typedef clmap<TokenPtr, VARIDESC>  VariableMap;
    typedef clStringA::LPCSTR LPCSTR;
    typedef clmap<clStringA, const NameContext*> StructContextMap;
    typedef ArithmeticExpression::GLOB GLOB;
    typedef clmap<TokenPtr, ValuePool> ValuePoolMap;

    enum State
    {
      State_SelfAdaptionLength = 1,
      State_Ok = 0,
      State_HasError = -1,           // ���������Ѿ��ں����ڲ����
      State_TypeNotFound = -2,       // û���ҵ�����
      State_DuplicatedType = -3,     // �ظ�ע������
      State_DuplicatedVariable = -4, // �ظ��������
      State_DefineAsType = -5,       // �����Ѿ�������Ϊ����
      State_DefineAsVariable = -6,   // �����Ѿ�������Ϊ����
      State_VariableIsNotIdentifier = -7, // �����ı���������һ����ʶ��
      State_RequireConstantExpression = -8, // ��Ҫ�������ʽ
      State_RequireSubscript = -9,   // ȱ���±�: int a[2][] = {};
      State_CastTypeError = -10,     // ����ת�����󣬺��е���ֵ����ת���ɱ�������
    };

  protected:
    CodeParser* m_pCodeParser;
    clStringA m_strName;    // ����
    const NameContext* m_pParent;     // Ĭ�ϼ�¼������
    const NameContext* m_pVariParent; // ������¼�����ȣ�����ṹ���ԱNameContext.m_pVariParentӦ��Ϊ��

    TypeMap     m_TypeMap;  // typedef ���������������ͬ��TYPEDESC
    FuncMap     m_FuncMap;
    VariableMap m_VariableMap;
    State       m_eLastState;
    StructContextMap  m_StructContextMap; // �ṹ���Ա��NameContext
    ValuePoolMap      m_ValuePoolMap;
    const TYPEDESC*   m_pReturnType;
    size_t      m_nCount;

    NameContext* GetRoot();
    const NameContext* GetRoot() const;

  private: // ��ֹ��������͸��ƹ���
    NameContext(const NameContext& sNameCtx){}
    NameContext& operator=(const NameContext sNameCtx) { return *this; }

    State IntRegisterVariable(const TYPEDESC** ppType, VARIDESC** ppVariable, const TYPEDESC* pTypeDesc, const TOKEN* ptkVariable, const VALUE* pConstValue, const GLOB* pValueExprGlob);
    State IntRegisterVariable(const TYPEDESC** ppType, VARIDESC** ppVariable, const clStringA& strType, const TOKEN* ptkVariable, const VALUE* pConstValue, const GLOB* pValueExprGlob);
  public:
    NameContext(GXLPCSTR szName);
    NameContext(GXLPCSTR szName, const NameContext* pParent, const NameContext* pVariParent = reinterpret_cast<NameContext*>(-1));
    ~NameContext();

    GXDWORD allow_keywords; // ���˵Ĺؼ���

    void SetParser(CodeParser* pCodeParser);
    void Cleanup();
    void Reset();

    void BuildIntrinsicType();
    CLogger* GetLogger() const;

    GXBOOL SetReturnType(GXLPCSTR szTypeName);
    const TYPEDESC* GetReturnType() const;

    const TYPEDESC* GetType(const clStringA& strType) const;
    const TYPEDESC* GetType(const TOKEN& token) const;
    const TYPEDESC* GetType(VALUE::Rank rank) const;
    const TYPEDESC* ConfirmArrayCount(const TYPEDESC* pTypeDesc, size_t nCount); // ���ò�ȷ�������������͵ĳ���
    const TYPEDESC* GetVariable(const TOKEN* ptkName) const;
    const ValuePool* GetValuePool(const TOKEN* ptkName) const;
    const VALUE*    GetVariableValue(const TOKEN* ptkName) const;
    const VARIDESC* GetVariableDesc(const TOKEN* ptkName) const;
    VALUE& GetVariableValue(VALUE& value, const TOKEN* ptkName) const;
    const NameContext* GetStructContext(const clStringA& strName) const;
    State  TypeDefine(const TOKEN* ptkOriName, const TOKEN* ptkNewName);
    //const TYPEDESC* RegisterArrayType(const TYPEDESC* pTypeDesc, size_t nDimension);
    GXBOOL RegisterStruct(const TOKEN* ptkName, const SYNTAXNODE* pMemberNode);
    GXBOOL RegisterStructContext(const clStringA& strName, const NameContext* pContext);
    GXBOOL RegisterFunction(const clStringA& strRetType, const clStringA& strName, const FUNCTION_ARGUMENT* pArguments, int argc);
    GXBOOL IsTypedefedType(const TOKEN* ptkTypename, const TYPEDESC** ppTypeDesc = NULL) const;
    const TYPEDESC* RegisterVariable(const clStringA& strType, const GLOB* pVariableDeclGlob, const VALUE* pConstValue = NULL, const GLOB* pValueExprGlob = NULL); // TODO: Ӧ�����Ӹ���һ������TYPEDESC������
    const TYPEDESC* RegisterVariable(const clStringA& strType, const TOKEN* ptkVariable, const VALUE* pConstValue = NULL, const GLOB* pValueExprGlob = NULL); // TODO: Ӧ�����Ӹ���һ������TYPEDESC������
    //void ChangeVariableType(const TYPEDESC* pTypeDesc, const SYNTAXNODE* pVariableDeclNode); // ֻ�ܸı�֮ǰûȷ�������������͵ı���
#ifdef ENABLE_SYNTAX_VERIFY
    const TYPEDESC* RegisterTypes(const clStringA& strBaseType, const TYPEDESC::DimList_T& sDimensions); // ���ݶ�ά�б�����ע�����ͣ�����ֵ�����ά������
    const TYPEDESC* RegisterMultidimVariable(const clStringA& strType, const SYNTAXNODE* pNode, const GLOB* pValueExprGlob);
#endif
    State  GetLastState() const;
    //const TYPEDESC* GetMember(const SYNTAXNODE* pNode) const;
    void GetMatchedFunctions(const TOKEN* pFuncName, size_t nFormalCount, cllist<const FUNCDESC*>& aMatchedFunc) const;
    
    static GXBOOL TestIntrinsicType(TYPEDESC* pOut, const clStringA& strType);
#ifdef ENABLE_SYNTAX_VERIFY
    VALUE::State CalculateConstantValue(VALUE& value_out, CodeParser* pParser, const GLOB* pGlob);
    VALUE::State Calculate(VALUE& value_out, CodeParser* pMsgLogger, const SYNTAXNODE* pNode) const;
#endif
  };

  struct COMMALIST
  {
    CodeParser* pCodeParser;
    SYNTAXNODE* pBegin;
    SYNTAXNODE* pEnd;

    void Init(CodeParser* pParser);
    SYNTAXNODE* Get();
    COMMALIST&  PushBack(const SYNTAXNODE::GLOB* pGlob);
  };

  class CInitList
  {
    struct STACKDESC
    {
      SYNTAXNODE::GlobList sInitList;
      SYNTAXNODE::GlobList::iterator iter;
      const TOKEN* ptkOpcode; // ���������λ
    };



    CodeParser*             m_pCodeParser;
    NameContext&            m_rNameCtx;
    const SYNTAXNODE::GLOB* m_pInitListGlob;
    cllist<STACKDESC>       m_sStack;
    GXBOOL                  m_bNeedAlignDepth;
    VALUE*                  m_pValuePool;
    size_t                  m_nValueCount;
    cllist<clStringA>       m_DebugStrings; // �������ɽ����ṹʽ
    cllist<COMMALIST>       m_RearrangedGlob;

    STACKDESC& Top();
    const STACKDESC& Top() const;
    GXBOOL Enter(const SYNTAXNODE::GLOB* pInitListGlob);

  public:
    enum
    {
      E_FAILED = -1
    };
    enum Result
    {
      Result_Failed = -1,
      Result_Ok = 0,
      Result_ExpandVecMat = 1,  // չ��������ʽ�������߾�������"float3(a,b,c)"��ʽ����ʼ��
      Result_NotAligned = 2,    // ������ʽ�������߾�������"float3(a,b,c)"��ʽ����ʼ����indexû����
    };

    CInitList(CodeParser* pCodeParser, NameContext& rNameCtx, const SYNTAXNODE::GLOB* pInitListGlob);
    CLogger* GetLogger();
    void SetValuePool(VALUE* pValuePool, size_t count);
    const SYNTAXNODE::GLOB* Get();
    Result CastToValuePool(const TYPEDESC* pRefTypeDesc, size_t base_index, size_t array_index);
    const TOKEN* GetLocation() const; // ��ô���λ����ص�Glob, ���ڴ��������λ
    const SYNTAXNODE::GLOB* Step();
    GXBOOL Step(size_t nDimDepth, size_t nListDepth);
    GXBOOL IsEnd() const;
    GXBOOL Empty() const;
    size_t Depth() const;
    GXBOOL NeedAlignDepth(size_t nDimDepth, size_t nListDepth) const;
    void ClearAlignDepthFlag();

    size_t GetMaxCount(const TYPEDESC* pRefType) const; // ���������Ӧ���ȣ����������������ܵĳ��ȣ����򷵻����͵�ʵ�ʳ���
    size_t BeginList();
    VALUE* ValuePoolEnd() const;
    size_t ValuePoolCount() const;
    SYNTAXNODE*  GetRearrangedList();

    void DbgListBegin(const clStringA& strTypeName);
    void DbgListAdd(const SYNTAXNODE::GLOB* pGlob);
    void DbgListAdd(const clStringA& str);
    void DbgListEnd();
    void DbgPushString();
    void DbgPopString();
    void DbgSetString(const clStringA& str);
    clStringA& DbgGetString();
  };

  //////////////////////////////////////////////////////////////////////////

  class CodeParser : public ArithmeticExpression
  {
    friend class NameContext;
    friend struct TYPEDESC;
    friend struct NODE_CALC;
    friend class CInitList;
    friend struct COMMALIST;
  public:
    typedef clstack<int> MacroStack;        // ���βκ����õĴ����ջ
    typedef ArithmeticExpression::iterator iterator;
    typedef NameContext::VALUEDESC VALUEDESC;

    struct MACRO_TOKEN : public TOKEN
    {
      typedef cllist<MACRO_TOKEN> List;
      typedef clvector<MACRO_TOKEN> Array;

      MACRO_TOKEN(const TOKEN& t)
        : TOKEN(t)
        , formal_index(-1)
      {
      }

      int formal_index; // ��Ӧ���β�����
    };

    struct MACRO
    {
      typedef clmap<clStringA, MACRO> Dict; // macro ֧�����أ����� key �������� clStringA ������, Dict Ҫ�����������
      typedef clvector<int>           IntArray;
      //typedef clhash_set<clStringA>   Set;
      MACRO_TOKEN::Array aFormalParams; // �β�
      MACRO_TOKEN::List aTokens;        // �滻����
      int               nOrder;         // ����˳���൱��id����ֹ����չ��
      size_t            nNumTokens;     // ����ռ�õ�token����������Ϊ1
      //GXDWORD bTranslate     : 1; // ������������һ����ǣ���Ҫת��
      //GXDWORD bHasLINE       : 1; // ��__LINE__��, ��������б仯��
      //GXDWORD bHasFILE       : 1; // ��__FILE__��
      //GXDWORD bPoundSign     : 1; // #����

      //MACRO();

      void set            (const Dict& dict, const TOKEN::Array& tokens, int begin_at);
      void clear          ();

      void ClearContainer (); // ����������iterator���container��ָ��subparse���׳���
      //int  ExpandMacro    (const Dict& dict); // չ����
    };

    struct PARSER_CONTEXT
    {
      GXUINT            nRefCount;
      clstd::StringSetA Strings;
      MACRO::Dict       Macros;
    };

    struct MACRO_EXPAND_CONTEXT
    {
      typedef clset<int> OrderSet_T;
      const TOKEN* pLineNumRef;
      TOKEN::List stream;
      const MACRO* pMacro;

      clvector<TOKEN::List> ActualParam;
      OrderSet_T OrderSet; // չ����ļ��ϣ���ֹ����չ���ݹ�
    };

    //////////////////////////////////////////////////////////////////////////

    enum AttachFlag // ע��: ��RTState���ñ��λ
    {
      AttachFlag_Preprocess     = 0x00000001, // ��Ԥ�����У����״̬������Ϊ#ΪԤ����ʼ���
      //AttachFlag_NotLoadMessage = 0x00010000, // TODO: ����ɵ�����
      AttachFlag_NotExpandMacro = 0x00020000, // ��չ����
      AttachFlag_NotExpandCond  = 0x00040000, // ��չ��defined()����ĺ�
    };

    enum MacroExpand
    {
      MacroExpand_Skip = 0,     // ���Ǻ꣬�����Ѿ�չ����һ�Σ�����
      MacroExpand_Ok,           // ִ����չ��
      MacroExpand_Incomplete,   // ����������Ҫ����
      MacroExpand_Rematch,      // ֮ǰ�Ĳ�����������������ƥ��
    };

    enum StatementType
    {
      StatementType_Empty,
      StatementType_Definition,     // ��������
      StatementType_FunctionDecl,   // ��������
      StatementType_Function,       // ������
      StatementType_Typedef,        // Typedef
      StatementType_Struct,         // �ṹ��
      StatementType_Signatures,     // ����shader���������ǵĽṹ��
      //StatementType_Expression,     // ���ʽ
    };

    enum FunctionStorageClass // �������Σ���ѡ
    {
      StorageClass_empty,
      StorageClass_inline,
    };

    enum VariableStorageClass
    {
      VariableStorageClass_empty,
      VariableStorageClass_extern,
      VariableStorageClass_nointerpolation,
      VariableStorageClass_precise,
      VariableStorageClass_shared,
      VariableStorageClass_groupshared,
      VariableStorageClass_static,
      VariableStorageClass_uniform,
      VariableStorageClass_volatile,
    };
    
    enum UniformModifier
    {
      UniformModifier_empty,
      UniformModifier_const,
      UniformModifier_row_major,
      UniformModifier_column_major,
    };

    enum PPCondRank
    {
      PPCondRank_Empty = 0,
      PPCondRank_if    = 1,
      PPCondRank_elif  = 2,
      PPCondRank_else  = 3,
      PPCondRank_endif = 4,
    };

    typedef clset<TYPEDESC>   TypeSet;

    typedef clvector<FUNCTION_ARGUMENT> ArgumentsArray;
    
    //////////////////////////////////////////////////////////////////////////
    struct STATEMENT_FUNC // ������/��������
    {
      FunctionStorageClass eStorageClass; // [opt]
      GXLPCSTR     szReturnType;  // [req]
      GXLPCSTR     szName;        // [req]
      GXLPCSTR     szSemantic;    // [opt]
      FUNCTION_ARGUMENT*  pArguments;       // [opt]
      clsize              nNumOfArguments;  // [opt]
    };

    struct STATEMENT_STRU // �ṹ�嶨��
    {
      GXLPCSTR         szName;
      clsize           nNumOfMembers; // ���Ǳ�Ҫ��
    };

    struct STATEMENT_DEFN
    {
      VariableStorageClass  storage_class;
      UniformModifier       modifier;
      GXLPCSTR              szType;
    };


    struct STATEMENT
    {
      StatementType type;
      GLOB sRoot;
      union
      {
        STATEMENT_FUNC func;
        STATEMENT_STRU stru;
        STATEMENT_DEFN defn; // ȫ�ֱ�������
      };
    };

    typedef clvector<STATEMENT> StatementArray;

    //////////////////////////////////////////////////////////////////////////

    struct RTPPCONTEXT // ����ʱԤ�������������
    {
      T_LPCSTR ppend;       // ��ǰԤ����Ľ�β
      T_LPCSTR stream_end;  // �ַ����Ľ�β��GetStreamPtr() + GetStreamCount()
      iterator iter_next;   // #��־Ԥ�������������ע�ͺ��һ��token
    };


    enum RTState // ����ʱ״̬, ��AttachFlag���ñ��λ
    {
      State_InPreprocess = 0x00000001, // ����Ԥ����
    };


    struct OPERAND // TODO: �������ʱ�ṹ�壬�Ժ����Դ�ļ��У�������ͷ�ļ���
    {
      VALUE  v;
      const TOKEN* pToken; // ���VALUE������token, pTokenӦ��Ϊ��

      GXBOOL OnToken(const SYNTAXNODE* pNode, int i)
      {
        clear();
        VALUE::State s = VALUE::State_OK;

        switch(pNode->mode)
        {
        case SYNTAXNODE::MODE_Opcode:
          s = v.set(*pNode->Operand[i].pTokn);

        case SYNTAXNODE::MODE_FunctionCall:
          if(i == 0) {
            pToken = pNode->Operand[i].pTokn;
          }
          else {
            s = v.set(*pNode->Operand[i].pTokn);
          }
          break;

        default:
          CLBREAK;
          return FALSE;
        }
        return s == VALUE::State_OK;
      }

      VALUE::State Calculate(const TOKEN& token, const OPERAND* param)
      {
        return v.Calculate(token, param[0].v, param[1].v);
      }

      void clear()
      {
        v.clear();
        pToken = NULL;
      }
    };

    struct MAKESCOPE
    {
      const TKSCOPE*  pScope;   // �޶�����
      TKSCOPE::TYPE   begin;    // ��ʼ
      GXBOOL          bBeginMate;// scope begin ȡ��m_aTokens[begin].scope
      TKSCOPE::TYPE   end;
      GXBOOL          bEndMate;
      GXWCHAR         chTermin; // ���begin��������ս��������һ��begin==end��scope

      MAKESCOPE(){}
      MAKESCOPE(const TKSCOPE*_pScope, TKSCOPE::TYPE _begin, GXBOOL _bIndBegin, TKSCOPE::TYPE _end, GXBOOL _bIndEnd, GXWCHAR _chTermin)
        : pScope(_pScope), begin(_begin), bBeginMate(_bIndBegin), end(_end), bEndMate(_bIndEnd), chTermin(_chTermin) {}
      MAKESCOPE(const TKSCOPE& _scope, TKSCOPE::TYPE _begin, GXBOOL _bIndBegin, TKSCOPE::TYPE _end, GXBOOL _bIndEnd, GXWCHAR _chTermin)
        : pScope(&_scope), begin(_begin), bBeginMate(_bIndBegin), end(_end), bEndMate(_bIndEnd), chTermin(_chTermin) {}
    };

    struct PHONY_TOKEN
    {
      clStringA::LPCSTR szPhonyText;  // ����ַ���
      clsize            nPhonyLength;
      clStringA::LPCSTR ori_marker;   // ԭʼ�ַ�����ַ
    };

    typedef clmap<int, PHONY_TOKEN> PhonyTokenDict_T;
    //////////////////////////////////////////////////////////////////////////

  protected:
    u32     StepIterator     (iterator& it) override;

    void    InitPacks();
    void    Reset();

    GXBOOL  ParseStatementAs_Definition(TKSCOPE* pScope);
    GXBOOL  ParseStatementAs_Function(TKSCOPE* pScope);
    GXBOOL  ParseStatement_SyntaxError(TKSCOPE* pScope);
    GXBOOL  ParseFunctionArguments(NameContext& sNameSet, STATEMENT* pStat, TKSCOPE* pArgScope, int& nTypeOnlyCount);

    GXBOOL  ParseStatementAs_Typedef(TKSCOPE* pScope);
    GXBOOL  ParseStatementAs_Struct(TKSCOPE* pScope);

    //GXBOOL  ParseStatementAs_Expression(STATEMENT* pStat, NameSet& sNameSet, TKSCOPE* pScope); // (������)��ʽ

    GXBOOL  CalculateValue(OPERAND& sOut, const GLOB* pDesc);

    SYNTAXNODE* FlatDefinition(SYNTAXNODE* pThisChain);
    static GLOB* BreakDefinition(SYNTAXNODE::PtrList& sVarList, SYNTAXNODE* pNode); // ��ɢ�ṹ���Ա
    static SYNTAXNODE::GlobList& BreakComma(SYNTAXNODE::GlobList& sExprList, const GLOB& sGlob); // �г����Ų���ʽ
    static SYNTAXNODE::GlobPtrList& BreakComma(SYNTAXNODE::GlobPtrList& sExprList, GLOB& sGlob); // �г����Ų���ʽ
    static SYNTAXNODE::GlobList& BreakChain(SYNTAXNODE::GlobList& sExprList, const GLOB& sGlob); // �г�������ʽ

    GXBOOL  ParseExpression(GLOB& glob, NameContext* pNameSet, const TKSCOPE& scope);
    GXBOOL  ParseToChain(GLOB& glob, NameContext* pNameSet, const TKSCOPE& scope);
    GXBOOL  ParseCodeBlock(GLOB& glob, NameContext* pNameSet, const TKSCOPE& scope);
    TKSCOPE::TYPE  TryParseSingle(NameContext* pNameSet, GLOB& glob, const TKSCOPE& scope); // ����һ�������, һ���ؼ��ֱ��ʽ����һ�����ʽ

    GXBOOL  TryKeywords(NameContext& sNameSet, const TKSCOPE& scope, GLOB* pDest, TKSCOPE::TYPE* parse_end);
    TKSCOPE::TYPE  ParseFlowIf(const NameContext& sParentSet, const TKSCOPE& scope, GLOB* pDesc, GXBOOL bElseIf);
    TKSCOPE::TYPE  MakeFlowForScope(const TKSCOPE& scope, TKSCOPE* pInit, TKSCOPE* pCond, TKSCOPE* pIter, TKSCOPE* pBlock, GLOB* pBlockNode);
    TKSCOPE::TYPE  ParseFlowFor(const NameContext& sParentSet, const TKSCOPE& scope, GLOB* pDesc);
    TKSCOPE::TYPE  ParseFlowWhile(const NameContext& sParentSet, const TKSCOPE& scope, GLOB* pDesc);
    TKSCOPE::TYPE  ParseFlowDoWhile(const NameContext& sParentSet, const TKSCOPE& scope, GLOB* pDesc);
    TKSCOPE::TYPE  ParseTypedef(NameContext& sNameSet, const TKSCOPE& scope, GLOB* pDesc);
    TKSCOPE::TYPE  ParseStructDefinition(NameContext& sNameSet, const TKSCOPE& scope, GLOB* pMembers, GLOB* pDefinitions, int* pSignatures, int* pDefinition);

    //const TYPEDESC* GetMember(const NameSet& sNameSet, const SYNTAXNODE* pNode) const;
    //VALUE::State CalcuateConstantValue(VALUE& value_out, const NameContext& sNameSet, const SYNTAXNODE::GLOB* pGlob);

    GXBOOL  MakeScope(TKSCOPE* pOut, MAKESCOPE* pParam);
    GXBOOL  OnToken(TOKEN& token);
    void    GetNext(iterator& it);
    iterator  MakeupMacroFunc(TOKEN::List& stream, iterator& it, const iterator& end);
    void    ExpandMacroFunc(MACRO_EXPAND_CONTEXT& c);
    MacroExpand ExpandMacroContent(TOKEN::List& sTokenList, const TOKEN& line_num, MACRO_EXPAND_CONTEXT::OrderSet_T* pOrderSet);
    MacroExpand TryMatchMacro(MACRO_EXPAND_CONTEXT& ctx_out, TOKEN::List::iterator* it_out, const TOKEN::List::iterator& it_begin, const TOKEN::List::iterator& it_end);
    GXBOOL  MergeStringToken(const TOKEN& token);
    const MACRO* FindMacro(const TOKEN& token);

    T_LPCSTR DoPreprocess(const RTPPCONTEXT& ctx, T_LPCSTR begin, T_LPCSTR end);
    void     PP_Pragma(const TOKEN::Array& aTokens);
    void     PP_Define(const TOKEN::Array& aTokens);
    void     PP_Include(const TOKEN::Array& aTokens);
    void     PP_Undefine(const RTPPCONTEXT& ctx, const TOKEN::Array& aTokens);
    T_LPCSTR PP_IfDefine(const RTPPCONTEXT& ctx, GXBOOL bNot, const TOKEN::Array& aTokens); // bNot ��ʾ if not define
    T_LPCSTR PP_If(const RTPPCONTEXT& ctx, CodeParser* pParser);
    T_LPCSTR PP_SkipConditionalBlock(PPCondRank session, T_LPCSTR begin, T_LPCSTR end); // ������Ԥ�������β��ʼ���������Ԥ����beginӦ���ǵ�ǰԤ����Ľ�β
    T_LPCSTR PP_FindPreProcessIdentifier(T_LPCSTR begin, T_LPCSTR end);
    void     PP_UserError(T_LPCSTR position, const clStringW& strText);
    GXBOOL   ExpandInnerMacro(TOKEN& token, const TOKEN& line_num); // ��Ҫ���滻__FILE__ __LINE__


    static T_LPCSTR Macro_SkipGapsAndNewLine(T_LPCSTR begin, T_LPCSTR end);  // ���������Ʊ���Ϳո����ַ�����ַ
    static T_LPCSTR Macro_SkipGaps(T_LPCSTR begin, T_LPCSTR end);  // ���������Ʊ���Ϳո����ַ�����ַ
    GXBOOL CompareString(T_LPCSTR str1, T_LPCSTR str2, size_t count);

    GXBOOL  ParseStatement(TKSCOPE* pScope);
    void    RelocaleStatements(StatementArray& aStatements);
    void    RelocalePointer();
    
    GXLPCSTR GetUniqueString(const TOKEN* pSym);
    GXLPCSTR GetUniqueString(T_LPCSTR szText);

    //void VarOutputErrorW(const TOKEN* pLocation, GXUINT code, va_list arglist) const;
    //void OutputErrorW(GXUINT code, ...);  // �����һ����ЧtokenѰ���к�
    //void OutputErrorW(const GLOB& glob, GXUINT code, ...) const;
    //void OutputErrorW(const SYNTAXNODE* pNode, GXUINT code, ...) const;
    //void OutputErrorW(const TOKEN& token, GXUINT code, ...) const;
    //void OutputErrorW(const TOKEN* pToken, GXUINT code, ...) const;
    //void OutputErrorW(T_LPCSTR ptr, GXUINT code, ...);

    //void OutputMissingSemicolon(const TOKEN* ptkLocation); // ���ȱ�ٷֺŵ���ʾ

    CodeParser* GetRootParser();
    clBuffer* OpenIncludeFile(const clStringW& strFilename);
    virtual T_LPCSTR GetOriginPtr(const TOKEN* pToken) const override;


#ifdef ENABLE_SYNTAX_VERIFY
    const TYPEDESC* InferUserFunctionType(const NameContext& sNameSet, const TYPEDESC::CPtrList& sTypeList, const SYNTAXNODE* pFuncNode); // ����ERROR_TYPEDESC��ʾ�Ƶ����ִ���
    const TYPEDESC* InferFunctionReturnedType(VALUE_CONTEXT& vctx, const SYNTAXNODE* pFuncNode);
    const TYPEDESC* InferType(VALUE_CONTEXT& vctx, const NameContext& sNameSet, const GLOB& sGlob);
    const TYPEDESC* InferType(VALUE_CONTEXT& vctx, const TOKEN* pToken) const;
    const TYPEDESC* InferType(VALUE_CONTEXT& vctx, const NameContext& sNameSet, const SYNTAXNODE* pNode);
    const TYPEDESC* RearrangeInitList(size_t nTopIndex, const TYPEDESC* pRefType, CInitList& rInitList, size_t nDepth);
    const TYPEDESC* InferInitList_Struct(size_t nTopIndex, const TYPEDESC* pRefType, CInitList& rInitList, size_t nDepth);
    const TYPEDESC* InferInitList_LinearArray(size_t nTopIndex, const TYPEDESC* pRefType, CInitList& rInitList, size_t nDepth);
    const TYPEDESC* InferInitList(ValuePool* pValuePool, NameContext& sNameSet, const TYPEDESC* pRefType, GLOB* pInitListGlob); // pInitListGlob.pNode->mode ������ MODE_InitList
    //const TYPEDESC* InferInitMemberList(const NameContext& sNameSet, const TYPEDESC* pLeftType, const GLOB* pInitListGlob); // pInitListGlob->pNode->mode ������ MODE_InitList, ����pInitListGlob��token
    const TYPEDESC* InferMemberType(VALUE_CONTEXT& vctx, const SYNTAXNODE* pNode);
    const TYPEDESC* InferSubscriptType(VALUE_CONTEXT& vctx, const SYNTAXNODE* pNode);
    const TYPEDESC* InferSubscriptTypeB(VALUE_CONTEXT& vctx, const SYNTAXNODE* pNode);
    const TYPEDESC* InferTypeByOperator(const TOKEN* pOperator, const TYPEDESC* pFirst, const TYPEDESC* pSecond);
    static const TYPEDESC* InferDifferentTypesOfCalculations(const TOKEN* pToken, const TYPEDESC* pFirst, const TYPEDESC* pSecond);
    static const TYPEDESC* InferDifferentTypesOfMultiplication(const TYPEDESC* pFirst, const TYPEDESC* pSecond);
    const TYPEDESC* InitList_SyncType(NameContext& sNameSet, const TYPEDESC* pRefType, const TYPEDESC* pListType, const GLOB* pElementGlob);
    const TYPEDESC* InitList_CastType(const TYPEDESC* pLeftType, const TYPEDESC* pListType, size_t nListCount, const GLOB* pLocation);
#endif

    const TYPEDESC* InferRightValueType(NameContext& sNameSet, const TYPEDESC* pLeftTypeDesc, const GLOB* pVarGlob, const GLOB& right_glob, const TOKEN* pLocation); // pLocation ���ڴ��������λ
    GXBOOL CompareScaler(GXLPCSTR szTypeFrom, GXLPCSTR szTypeTo);
    GXBOOL TryTypeCasting(const TYPEDESC* pTypeTo, const TYPEDESC* pTypeFrom, const TOKEN* pLocation); // pLocation ���ڴ��������λ
    GXBOOL TryTypeCasting(const NameContext& sNameSet, GXLPCSTR szTypeTo, const TYPEDESC* pTypeFrom, const TOKEN* pLocation); // pLocation ���ڴ��������λ
    GXBOOL TryTypeCasting(const NameContext& sNameSet, GXDWORD dwArgMask, const TYPEDESC* pTypeFrom, const TOKEN* pLocation); // pLocation ���ڴ��������λ
    GXBOOL TryReinterpretCasting(const TYPEDESC* pTypeTo, const TYPEDESC* pTypeFrom, const TOKEN* pLocation); // pLocation ���ڴ��������λ

    //static GXLPCSTR ResolveType(const TYPEDESC* pTypeDesc, int& R, int& C);
    static GXBOOL IsComponent(const TYPEDESC* pRowVector, const TYPEDESC* pMatrixDesc, const TYPEDESC* pColumnVector);

#ifdef ENABLE_SYNTAX_VERIFY
    //const TYPEDESC2* Verify_Type(const TOKEN& tkType);
    //const TYPEDESC* Verify_Struct(const TOKEN& tkType, const NameContext* pNameSet);
    static const TOKEN* GetVariableNameWithoutSeamantic(const GLOB& glob); // ȡȥ������ı��������硰vColor��
    const SYNTAXNODE::GLOB* GetVariableDeclWithoutSeamantic(const GLOB& glob); // ȡȥ������ı������������ܺ����±꣬�硰vColor[2][3]��
    GXBOOL Verify_MacroFormalList(const MACRO_TOKEN::List& sFormalList);

    GXBOOL Verify_VariableTypedDefinition(NameContext& sNameSet, const TOKEN& tkType, const GLOB& second_glob, GXBOOL bConstVariable = FALSE, GXBOOL bMember = FALSE);
    GXBOOL Verify_VariableDefinition(NameContext& sNameSet, const SYNTAXNODE* pNode, GXBOOL bConstVariable = FALSE, GXBOOL bMember = FALSE);
    //GXBOOL Verify2_VariableInit(NameContext& sNameSet, const TYPEDESC* pType, const SYNTAXNODE& rNode);
    //GXBOOL Verify_FunctionBlock(const STATEMENT_EXPR& expr);
    GXBOOL Verify_Chain(const SYNTAXNODE* pNode, NameContext& sNameContext);
    GXBOOL Verify_Block(const SYNTAXNODE* pNode, const NameContext* pParentSet);
    GXBOOL Verify_StructMember(NameContext& sParentSet, const clStringA& strStructName, const SYNTAXNODE& rNode);
    const TYPEDESC* Verify2_LeftValue(const NameContext& sNameSet, const GLOB& left_glob, const TOKEN& opcode); // opcode ��Ҫ��Ϊ�˶�λ
    //GXBOOL Verify2_RightValue(const NameContext& sNameSet, const TYPEDESC* pType, SYNTAXNODE::MODE mode, const GLOB& right_glob);
#endif

    void SetRepalcedValue(const GLOB& glob, const VALUE& value);

    VALUE::State CalculateValueAsConstantDefinition(VALUE& value_out, NameContext& sNameCtx, const GLOB& const_expr_glob);

    void DbgBreak(const GLOB& glob);
    void DbgBreak(const SYNTAXNODE* pNode);
    void DbgBreak(const TOKEN* pToken);
    void DbgAssert(b32 bConditional, const GLOB& glob);
    void DbgAssert(b32 bConditional, const TOKEN& token);

    void SetTokenPhonyString(int index, const clStringA& str);

    template<class _Ty>
    _Ty* IndexToPtr(clvector<_Ty>& array, _Ty* ptr_index)
    {
      return &array[(GXINT_PTR)ptr_index];
    }

    template<class _Ty>
    void IndexToPtr(_Ty*& ptr_index, clvector<_Ty>& array)
    {
      ptr_index = IndexToPtr(array, ptr_index);
    }
    CodeParser* GetSubParser();

  protected:
    typedef clmap<clStringW, clBuffer*> FileDict;
    typedef clmap<const void*, VALUE> ValueDict;
    GXDWORD             m_dwState;          // �ڲ�״̬, �ο�RTState
    CodeParser*         m_pParent;
    TypeSet             m_TypeSet;
    StatementArray      m_aStatements;
    StatementArray      m_aSubStatements;   // m_aStatements�Ĵμ����棬û��˳���ϵ
    int                 m_nPPRecursion;     // ����Ԥ����ݹ����

    
    PARSER_CONTEXT*     m_pContext;
    PhonyTokenDict_T    m_PhonyTokenDict;   // �û����滻��token���ҵ�ԭʼtoken��Ϣ

    ArgumentsArray      m_aArgumentsPack;   // ���к��������������������

    FileDict            m_sIncludeFiles;    // �����ļ�����, ������Root parser
    Include*            m_pInclude;
  //MACRO::Set          m_MacrosSet;        // ʵ������
    CodeParser*         m_pSubParser;
    //MACRO_EXPAND_CONTEXT::List m_sMacroStack;
    TOKEN::List         m_ExpandedStream;   // ��չ����
    NameContext         m_GlobalSet;
    ValueDict           m_ValueDict;        // �������ʽ����ֱ���滻�����ֵ�

  public:
    CodeParser();
    CodeParser(CodeParser* pParent, PARSER_CONTEXT* pContext, Include* pInclude);
    virtual ~CodeParser();
    b32                 Attach                  (const char* szExpression, clsize nSize, GXDWORD dwFlags, GXLPCWSTR szFilename);
    clsize              GenerateTokens          ();
    GXBOOL              Parse                   ();

    const StatementArray& GetStatements         () const;
    void                Invoke                  (GXLPCSTR szFunc, GXLPCSTR szArguments);
    void                GetRepalcedValue        (VALUE& value, const GLOB& glob) const;

    static void DbgDumpSyntaxTree(clStringArrayA* pArray, const SYNTAXNODE* pNode, int precedence, clStringA* pStr = NULL, int depth = 0);


  };

  //////////////////////////////////////////////////////////////////////////

} // namespace UVShader

#endif // #ifndef _EXPRESSION_PARSER_H_