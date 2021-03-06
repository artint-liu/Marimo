#ifndef _EXPRESSION_PARSER_H_
#define _EXPRESSION_PARSER_H_

#define ENABLE_SYNTAX_VERIFY // 语法检查开关, 只用来做语法解析临时调试用, 正式版不要关闭这个

#define VerifyIdentifierDefinition_Static   0x0001
#define VerifyIdentifierDefinition_Const    0x0002
#define VerifyIdentifierDefinition_Member   0x0004

namespace UVShader
{
  class CodeParser;
  class NameContext;
  struct COMMINTRTYPEDESC;
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
    KeywordFilter_return   = 0x0008,    // 有返回值的函数才开启这个
    KeywordFilter_typedef  = 0x0010,

    KeywordFilter_All = 0xffffffff,
    KeywordFilter_InFuntionArgument = 0,
    KeywordFilter_InFuntion = KeywordFilter_typedef,
    //KeywordMask_FlowsControl,
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

  struct TYPEDESC
  {
    typedef cllist<const TYPEDESC*> CPtrList;
    typedef clvector<TYPEDESC> Array;
    typedef clvector<const TYPEDESC*> CPtrArray;
    enum TypeCate
    {
      TypeCate_Empty,
      TypeCate_Void,
      TypeCate_FloatScaler,    // 浮点-数字类型
      TypeCate_IntegerScaler,  // 整数-数字类型
      TypeCate_MultiDim,  // 多维类型
      TypeCate_String,
      TypeCate_Sampler1D,
      TypeCate_Sampler2D,
      TypeCate_Sampler3D,
      TypeCate_SamplerCube,
      //TypeCate_Scaler, // TODO: 与TypeCate_FloatNumeric，TypeCate_IntegerNumeric重复
      TypeCate_Vector,
      TypeCate_Matrix,
      TypeCate_Struct,
      //TypeCate_Flag_Scaler   = 0x10000,
      //TypeCate_Flag_MultiDim = 0x20000,
      //TypeCate_Flag_Sampler  = 0x40000,
      //TypeCate_Flag_Struct   = 0x80000,
    };
    typedef clvector<size_t> DimList_T;

    TypeCate            cate;
    NameContext* const  pNameCtx;
    RefString           name; // 类型名
    COMMINTRTYPEDESC*   pDesc; // 结构体(内置) 的描述, 多维数组这个指向它的基础类型
    const SYNTAXNODE*   pMemberNode; // 结构体(用户定义) 的描述, 必须是"SYNTAXNODE::MODE_Block"
    DimList_T           sDimensions; // 维度列表 int var[a][b][c][d] 储存为{d，c，b，a}
    const TYPEDESC*     pElementType; // float[3][2]的 pElementType=float[2]，float3[2]的pElementType=float3，float3的pElementType=float，只有数学类型才有
    const TOKEN*        pLocation;

    CLogger* GetLogger() const;
    static GXBOOL MatchScaler(const TOKEN* ptkMember, GXLPCSTR scaler_set); // 保证.xxxx, .xyxy, .yxwz这种也是合理的成员
    GXLPCSTR Resolve(int& R, int& C) const;
    GXBOOL IsVector() const;
    GXBOOL IsMatrix() const;
    GXBOOL IsSameType(const TYPEDESC* pOtherTypeDesc) const;
    GXBOOL GetMemberTypeList(TYPEDESC::CPtrList& sMemberTypeList) const;
    size_t CountOf() const; // 获得向量，矩阵和数组包含的基础类型数量
    const TYPEDESC* ConfirmArrayCount(size_t nCount) const;
    VALUE::Rank GetRank() const;
  };

  struct TYPEINSTANCE
  {
    typedef clvector<TYPEINSTANCE>   Array;
    const TYPEDESC* pTypeDesc;
    const TOKEN*    pLocation; // 输出定位
  };

  struct FUNCDESC // 用户定义的函数描述
  {
    typedef cllist<const FUNCDESC*> CPtrList;
    // FIXME: 因为定义顺序关系, 返回值和形参应改储存TYPEDESC, 而不是名字, 这个要改暂时备忘
    RefString         ret_type;     // 返回的类型
    RefString         name;         // 类型名
    TYPEINSTANCE::Array  sFormalTypes; // 函数, 形参类型表
    clStringW& ToString(clStringW& str) const;
  };

  struct INTRINSIC_FUNC
  {
    enum RetType
    {
      RetType_Scaler0 = -1,
      //RetType_FromName = -2, // 返回类型是函数名
      //RetType_Vector4 = -3,  // 4维向量,一般代表颜色
      RetType_Bool = -3,
      //RetType_Float4 = -4,  // 4维向量,一般代表颜色
      RetType_Last = -5,
      RetType_Argument0 = 0, // 返回类型同第一个参数类型
      RetType_Argument1 = 1, // 返回类型同第二个参数类型
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

    GXLPCSTR  name;   // 函数名
    int       type;   // 0 表示与第一个参数相同, 1表示与第二个参数相同, 以此类推
                      // -1表示第一个参数的标量值, 例如第一个参数是int3, 则返回值是int
    size_t    count;  // 参数数量
    u16*      params;

    static int GetTypeTemplateTypeIndex(GXDWORD dwMasks);
    int GetTemplateID(size_t nParamIndex) const
    {
      ASSERT(nParamIndex < count);
      return ((params[nParamIndex] & 0xE000) >> 13);
    }
  };

  struct BUILDIN_FUNCTION_PROTOTYPE
  {
    GXLPCSTR  type;   // 返回值
    GXLPCSTR  name;   // 函数名(key)
    size_t    count;
    GXLPCSTR  formal_param; // 串联形参表，如"float\0float\0"
  };

  // 用来实现向量和矩阵的组合初始化
  // 如: float3(pos.xy, 1)
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
    ValueResult_Variable,    // 出现变量, 无法计算出常量
    ValueResult_Failed,      // 其它错误
    ValueResult_NotStructMember,
    ValueResult_NotNumeric,
    ValueResult_BadRank,
    ValueResult_CanNotInferType,
    ValueResult_2065 = 2065, // 未声明标识符
    ValueResult_3861 = 3861, // 找不到标识符
    ValueResult_5039 = 5039, // 函数参数数量不匹配
    ValueResult_SubscriptOutOfRange, // 下标超出范围
  };

  struct IDNFDESC
  {
    const static size_t shift = (sizeof(size_t) * 8 - 1);
    const static size_t Registering = (size_t)1 << shift; // 登记中
    const TYPEDESC*   pType;
    size_t            nOffset;// 与 Registering 不为 0 表示在登记
    SYNTAXNODE::GLOB  glob;   // 所有常量都应该定义这个值
    ValuePool         pool;   // 常量池

    GXBOOL IsRegistering() const
    {
      return (nOffset & Registering) != 0;
    }
  };

  enum InputModifier // 函数参数修饰
  {
    InputModifier_in      = 0x01,
    InputModifier_out     = 0x02,
    InputModifier_inout   = 0x03,
    InputModifier_uniform = 0x04, // Input only constant data
  };

  struct VALUE_CONTEXT
  {
    typedef clvector<VALUE_CONTEXT> Array;
    // [属性]
    const NameContext&  name_ctx;
    const NameContext*  pMemberCtx;
    GXBOOL              bNeedValue;
    CLogger*            pLogger;

    // [值]
    ValueResult     result;
    const TYPEDESC* pType;
    const VALUE*    pValue; // 不能修改这个内容
    size_t          count;
    ValuePool       pool;

    VALUE_CONTEXT(const VALUE_CONTEXT& vctx);
    VALUE_CONTEXT(const NameContext& _name_ctx);
    VALUE_CONTEXT(const NameContext& _name_ctx, CLogger* pLogger);
    VALUE_CONTEXT(const NameContext& _name_ctx, GXBOOL _bNeedValue);

    void SetProperty(const VALUE_CONTEXT& vctx); // 从vctx复制属性
    void UsePool();
    void ClearValue();
    const VALUE_CONTEXT& ClearValue(ValueResult r);
    void ClearValueOnly();
    void CopyValue(const VALUE_CONTEXT& vc);
    void SetType(VALUE::Rank rank);
    const TYPEDESC* CastUpward(const TYPEDESC* pTargetType);
    const TYPEDESC* MergeType(const TYPEDESC* pScalerType, const TYPEDESC* pVecMatType);
    GXBOOL IsNeedValue() const;
    void  GenerateMathComponentValue(const DOTOPERATOR_RESULT& sDotOperator);
    VALUE::Rank TypeRank() const;
  };

  // 检查VALUE_CONTEXT输入与输出值的有效性
  struct VALUE_CONTEXT_CHECKER
  {
    int nErrorCount;
    const VALUE_CONTEXT& vctx;
    VALUE_CONTEXT_CHECKER(const VALUE_CONTEXT& _vctx);
    ~VALUE_CONTEXT_CHECKER();
    void ClearErrorCount(); // 跳过错误输出检查
  };

#define CHECK_VALUE_CONTEXT VALUE_CONTEXT_CHECKER vcc(vctx)
#define CHECK_VALUE_CONTEXT_CLEARERRORCOUNT vcc.ClearErrorCount()

  class NameContext
  {
  public:
    struct VALUEDESC
    {
      const VALUE* pValue;
      size_t count;
    };

    typedef clmap<RefString, TYPEDESC>  TypeMap;
    typedef std::multimap<RefString, FUNCDESC>  FuncMap;
    typedef clmap<TokenPtr, IDNFDESC>  IdentifierMap;
    typedef clStringA::LPCSTR LPCSTR;
    typedef clmap<RefString, const NameContext*> StructContextMap;
    typedef ArithmeticExpression::GLOB GLOB;

    enum State
    {
      State_SelfAdaptionLength = 1,
      State_Ok = 0,
      State_HasError = -1,                  // 其它错误，已经在函数内部输出
      State_TypeNotFound = -2,              // 没有找到类型
      State_DuplicatedType = -3,            // 重复注册类型
      State_DuplicatedIdentifier = -4,      // 重复定义变量
      State_DefineAsType = -5,              // 变量已经被定义为类型
      State_DefineAsIdentifier = -6,        // 类型已经被定义为变量
      State_VariableIsNotIdentifier = -7,   // 期望的变量名不是一个标识符
      State_RequireConstantExpression = -8, // 需要常量表达式
      State_RequireSubscript = -9,          // 缺少下标: int a[2][] = {};
      State_CastTypeError = -10,            // 类型转换错误，含有的右值不能转换成变量类型
      //State_RequireInitList = -11,
    };

  protected:
    CodeParser* m_pCodeParser;
    clStringA   m_strName;    // 域名
    const NameContext* m_pParent;     // 默认记录的祖先
    const NameContext* m_pVariParent; // 变量记录的祖先，比如结构体成员NameContext.m_pVariParent应该为空

    TYPEDESC::Array m_aBasicType;
    TypeMap       m_TypeMap;  // typedef 会产生两个内容相同的TYPEDESC
    FuncMap       m_FuncMap;
    IdentifierMap m_IdentifierMap;
    State         m_eLastState;
    StructContextMap  m_StructContextMap; // 结构体成员的NameContext
    const TYPEDESC*   m_pReturnType;
    size_t      m_nCount;

    NameContext* GetRoot();
    const NameContext* GetRoot() const;

  private: // 禁止拷贝构造和复制构造
    NameContext(const NameContext& sNameCtx){}
    NameContext& operator=(const NameContext sNameCtx) { return *this; }

    State IntRegisterIdentifier(IDNFDESC** ppIdentifier, const TYPEDESC* pTypeDesc, const TOKEN* ptkIdentifier, GXDWORD dwMidifier, const GLOB* pValueExprGlob);
    State IntRegisterIdentifier(IDNFDESC** ppIdentifier, const RefString& rstrType, const TOKEN* ptkIdentifier, GXDWORD dwMidifier, const GLOB* pValueExprGlob);
  public:
    NameContext(GXLPCSTR szName);
    NameContext(GXLPCSTR szName, const NameContext* pParent, const NameContext* pVariParent = reinterpret_cast<NameContext*>(-1));
    ~NameContext();

    GXDWORD allow_keywords; // 过滤的关键字

    void SetParser(CodeParser* pCodeParser);
    void Cleanup();
    void Reset();

    void BuildIntrinsicType();
    CLogger* GetLogger() const;

    GXBOOL SetReturnType(const RefString& rstrTypeName);
    const TYPEDESC* GetReturnType() const;

    const TYPEDESC* GetType(const RefString& rstrType) const;
    const TYPEDESC* GetType(const TOKEN& token) const;
    const TYPEDESC* GetType(VALUE::Rank rank) const;
    const TYPEDESC* GetIntrinsicType(const RefString& rstrType) const;
    const TYPEDESC* ConfirmArrayCount(const TYPEDESC* pTypeDesc, size_t nCount); // 设置不确定长度数组类型的长度
    const TYPEDESC* GetIdentifier(const TOKEN* ptkName) const;
    const IDNFDESC* GetIdentifierDesc(const TOKEN* ptkName) const;
    const NameContext* GetStructContext(const RefString& rstrName) const;
    State  TypeDefine(const TOKEN* ptkOriName, const TOKEN* ptkNewName);
    GXBOOL RegisterStruct(const TOKEN* ptkName, const SYNTAXNODE* pMemberNode);
    GXBOOL RegisterStructContext(const RefString& rstrName, const NameContext* pContext);
    //GXBOOL RegisterFunction(const clStringA& strRetType, const clStringA& strName, const FUNCTION_ARGUMENT* pArguments, int argc);
    GXBOOL RegisterFunction(const RefString& rstrRetType, const RefString& rstrName, const TYPEINSTANCE::Array& type_array);
    GXBOOL IsTypedefedType(const TOKEN* ptkTypename, const TYPEDESC** ppTypeDesc = NULL) const;
    GXBOOL TranslateType(RefString& rstrTypename, const TOKEN* ptkTypename) const; // 转换typedef定义过的类型
    const TYPEDESC* RegisterIdentifier(const TOKEN& tkType, const GLOB* pIdentifierDeclGlob, GXDWORD dwModifier, const GLOB* pValueExprGlob = NULL); // TODO: 应该增加个第一参数是TYPEDESC的重载
    const TYPEDESC* RegisterIdentifier(const TOKEN& tkType, const TOKEN* ptkIdentifier, GXDWORD dwModifier, const GLOB* pValueExprGlob = NULL); // TODO: 应该增加个第一参数是TYPEDESC的重载
#ifdef ENABLE_SYNTAX_VERIFY
    const TYPEDESC* RegisterTypes(const TOKEN& tkBaseType, const TYPEDESC::DimList_T& sDimensions); // 根据多维列表依次注册类型，返回值是最高维度类型
    const TYPEDESC* RegisterMultidimIdentifier(const TOKEN& tkType, const SYNTAXNODE* pNode, GXDWORD dwModifier, const GLOB* pValueExprGlob);
#endif
    State  GetLastState() const;
    void GetMatchedFunctions(const TOKEN* pFuncName, size_t nFormalCount, cllist<const FUNCDESC*>& aMatchedFunc) const;
    
    //static GXBOOL TestIntrinsicType(TYPEDESC* pOut, const clStringA& strType);
#ifdef ENABLE_SYNTAX_VERIFY
    VALUE::State CalculateConstantValue(State& eLastState, VALUE& value_out, const GLOB* pGlob) const;
    VALUE::State Calculate(VALUE& value_out, const SYNTAXNODE* pNode) const;
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
  public:
    union ELEMENT
    {
      typedef cllist<ELEMENT> List;

      SYNTAXNODE::GLOB   glob;
      VALUE*             pValue; // 一定在pool里面
    };
    STATIC_ASSERT(sizeof(ELEMENT) == sizeof(SYNTAXNODE::GLOB)); // BreakComma要使用这个特性

    struct STACKDESC
    {
      ELEMENT::List sInitList;
      ELEMENT::List::iterator iter;
      const TOKEN* ptkOpcode; // 用于输出定位
    };

  private:
    CodeParser*             m_pCodeParser;
    NameContext&            m_rNameCtx;
    const SYNTAXNODE::GLOB* m_pInitListGlob;
    cllist<STACKDESC>       m_sStack;
    GXBOOL                  m_bNeedAlignDepth;
    GXBOOL                  m_bConstantValues; // 所有值全是常量
    VALUE*                  m_pValuePool;
    size_t                  m_nValueCount;
    cllist<clStringA>       m_DebugStrings; // 用来生成解析结构式
    cllist<COMMALIST>       m_RearrangedGlob;

    STACKDESC& Top();
    const STACKDESC& Top() const;
    GXBOOL Enter(const SYNTAXNODE::GLOB* pInitListGlob);

  public:
    enum
    {
      E_FAILED = -1
    };
//#define REFACTOR_122
//#ifndef REFACTOR_122
//    enum Result
//    {
//      Result_Failed = -1,
//      Result_Ok = 0,
//      Result_ExpandVecMat = 1,  // 展开函数形式向量或者矩阵（类似"float3(a,b,c)"形式）初始化
//      Result_NotAligned = 2,    // 函数形式向量或者矩阵（类似"float3(a,b,c)"形式）初始化与index没对齐
//      Result_VecMatConstruct = 3,
//    };
//#endif

    CInitList(CodeParser* pCodeParser, NameContext& rNameCtx, const SYNTAXNODE::GLOB* pInitListGlob);
    CLogger* GetLogger();
    void SetValuePool(VALUE* pValuePool, size_t count);
    const ELEMENT* Get();
    const TYPEDESC* ExpandValue(VALUE_CONTEXT& vctx, size_t base_index, size_t array_index);
    const TYPEDESC* CastToValuePool(VALUE_CONTEXT& vctx, const TYPEDESC* pRefTypeDesc, size_t base_index, size_t array_index);
    const TOKEN* GetLocation() const; // 获得代码位置相关的Glob, 用于错误输出定位
    const SYNTAXNODE::GLOB* Step();
    GXBOOL Step(size_t nDimDepth, size_t nListDepth);
    GXBOOL IsEnd() const;
    GXBOOL Empty() const;
    size_t Depth() const;
    GXBOOL IsConstantValues() const;
    GXBOOL NeedAlignDepth(size_t nDimDepth, size_t nListDepth) const;
    void ClearAlignDepthFlag();
    GXBOOL IsValue(const ELEMENT* pElement) const;
    VALUE& FillZeroByRank(size_t index, VALUE::Rank rank);

    size_t GetMaxCount(const TYPEDESC* pRefType) const; // 如果是自适应长度，用这个来获得最大可能的长度，否则返回类型的实际长度
    size_t BeginList();
    VALUE* ValuePoolEnd() const;
    size_t ValuePoolCount() const;
    SYNTAXNODE*  GetRearrangedList();
    NameContext& GetNameContext() const;

    void DbgListBegin(const RefString& rstrTypeName);
    void DbgListAdd(const ELEMENT* pElement);
    void DbgListAdd(const clStringA& str);
    void DbgListEnd();
    void DbgPushString();
    void DbgPopString();
    void DbgSetString(const clStringA& str);
    clStringA& DbgGetString();
  };

  struct PHONYTOKEN : TOKEN
  {
    typedef cllist<PHONYTOKEN>   List;
    typedef clvector<PHONYTOKEN> Array;

    const TOKEN* ptkOriginal; // 原始的token
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
    typedef clstack<int> MacroStack;        // 带形参宏所用的处理堆栈
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

      int formal_index; // 对应的形参索引
    };

    struct MACRO
    {
      typedef clmap<RefString, MACRO> Dict; // macro 支持重载，所以 key 的名字用 clStringA 来储存, Dict 要求具有排序性
      typedef clvector<int>           IntArray;
      //typedef clhash_set<clStringA>   Set;
      MACRO_TOKEN::Array aFormalParams; // 形参
      MACRO_TOKEN::List aTokens;        // 替换内容
      int               nid;            // 定义顺序，相当于id，防止无限展开
      size_t            nNumTokens;     // 宏名占用的token数量，最少为1
      //GXDWORD bTranslate     : 1; // 含有下面任意一个标记，需要转义
      //GXDWORD bHasLINE       : 1; // 有__LINE__宏, 这个宏是有变化的
      //GXDWORD bHasFILE       : 1; // 有__FILE__宏
      //GXDWORD bPoundSign     : 1; // #解析

      //MACRO();

      void set            (const Dict& dict, const TOKEN::Array& tokens, int begin_at);
      void clear          ();

      void ClearContainer (); // 这个用来清除iterator里的container，指向subparse容易出错
      //int  ExpandMacro    (const Dict& dict); // 展开宏
    };

    struct PARSER_CONTEXT
    {
      GXUINT            nRefCount;
      clstd::StringSetA Strings;
      MACRO::Dict       Macros;
      size_t            mid;    // id计数器
    };

    struct MACRO_EXPAND_CONTEXT
    {
      typedef clset<int> IDSet_T;
      const TOKEN* pLineNumRef;
      TOKEN::List stream;
      const MACRO* pMacro;

      clvector<TOKEN::List> ActualParam;
      IDSet_T OrderSet; // 展开宏的集合，防止无限展开递归
    };

    //////////////////////////////////////////////////////////////////////////

    enum AttachFlag
    {
      AttachFlag_Preprocess       = 0x00000001, // 在预处理中，这个状态不再认为#为预处理开始标记
      AttachFlag_RetainPreprocess = 0x00000002, // 保留预处理命令为激活状态
      AttachFlag_NotExpandMacro   = 0x00020000, // 不展开宏
      AttachFlag_NotExpandCond    = 0x00040000, // 不展开defined()里面的宏
    };

    enum MacroExpand
    {
      MacroExpand_Skip = 0,     // 不是宏，或者已经展开了一次，跳过
      MacroExpand_Ok,           // 执行了展开
      MacroExpand_Incomplete,   // 不完整，需要后续
      MacroExpand_Rematch,      // 之前的不完整，在这里重新匹配
    };

    enum StatementType
    {
      StatementType_Empty,
      StatementType_Definition,     // 变量定义
      StatementType_FunctionDecl,   // 函数声明
      StatementType_Function,       // 函数体
      StatementType_Typedef,        // Typedef
      StatementType_Struct,         // 结构体
      StatementType_Signatures,     // 用于shader输入输出标记的结构体
      //StatementType_Expression,     // 表达式
    };

    enum FunctionStorageClass // 函数修饰，可选
    {
      StorageClass_empty,
      StorageClass_inline,
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

    //////////////////////////////////////////////////////////////////////////
    struct STATEMENT_FUNC // 函数体/函数声明
    {
      FunctionStorageClass eStorageClass; // [opt]
      GXLPCSTR     szReturnType;  // [req]
      GXLPCSTR     szName;        // [req]
      GXLPCSTR     szSemantic;    // [opt]
      GLOB         arguments_glob;
      clsize              nNumOfArguments;  // [opt]
    };

    struct STATEMENT_STRU // 结构体定义
    {
      GXLPCSTR         szName;
      clsize           nNumOfMembers; // 不是必要的
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
        STATEMENT_DEFN defn; // 全局变量定义
      };
    };

    typedef clvector<STATEMENT> StatementArray;

    //////////////////////////////////////////////////////////////////////////

    struct RTPPCONTEXT // 运行时预处理解析上下文
    {
      T_LPCSTR ppend;       // 当前预处理的结尾
      T_LPCSTR stream_end;  // 字符流的结尾，GetStreamPtr() + GetStreamCount()
      iterator iter_next;   // #标志预处理结束，忽略注释后第一个token
    };


    enum RTState // 运行时状态
    {
      //State_InPreprocess = 0x00000001, // 解析预处理
      State_MoreValidation = 0x00000001,   // 更多语法检查，在语法树链检查中，如果遇到错误也会继续检查后面的链的内容
    };


    struct OPERAND // TODO: 这个是临时结构体，以后放在源文件中，而不是头文件里
    {
      VALUE  v;
      const TOKEN* pToken; // 如果VALUE解析了token, pToken应该为空

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
      const TKSCOPE*  pScope;   // 限定区域
      TKSCOPE::TYPE   begin;    // 开始
      GXBOOL          bBeginMate;// scope begin 取自m_aTokens[begin].scope
      TKSCOPE::TYPE   end;
      GXBOOL          bEndMate;
      GXWCHAR         chTermin; // 如果begin遇到这个终结符，返回一个begin==end的scope

      MAKESCOPE(){}
      MAKESCOPE(const TKSCOPE*_pScope, TKSCOPE::TYPE _begin, GXBOOL _bIndBegin, TKSCOPE::TYPE _end, GXBOOL _bIndEnd, GXWCHAR _chTermin)
        : pScope(_pScope), begin(_begin), bBeginMate(_bIndBegin), end(_end), bEndMate(_bIndEnd), chTermin(_chTermin) {}
      MAKESCOPE(const TKSCOPE& _scope, TKSCOPE::TYPE _begin, GXBOOL _bIndBegin, TKSCOPE::TYPE _end, GXBOOL _bIndEnd, GXWCHAR _chTermin)
        : pScope(&_scope), begin(_begin), bBeginMate(_bIndBegin), end(_end), bEndMate(_bIndEnd), chTermin(_chTermin) {}
    };

    struct PHONY_TOKEN
    {
      clStringA::LPCSTR szPhonyText;  // 替代字符串
      clsize            nPhonyLength;
      clStringA::LPCSTR ori_marker;   // 原始字符串地址
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
    GXBOOL  ParseFunctionArguments(NameContext& sNameCtx, STATEMENT* pStat, TKSCOPE* pArgScope, TYPEINSTANCE::Array& types_array, int& nTypeOnlyCount);
    GXBOOL  VerifyFunctionArgument(NameContext& sNameCtx, const GLOB* pGlob, const GLOB& rBaseGlob, TYPEINSTANCE::Array& types_array, int& nTypeOnlyCount);
    //GXBOOL  ParseFunctionArguments2(NameContext& sNameSet, STATEMENT* pStat, TKSCOPE* pArgScope, int& nTypeOnlyCount);

    GXBOOL  ParseStatementAs_Typedef(TKSCOPE* pScope);
    GXBOOL  ParseStatementAs_Struct(TKSCOPE* pScope);

    //GXBOOL  ParseStatementAs_Expression(STATEMENT* pStat, NameSet& sNameSet, TKSCOPE* pScope); // (算数表)达式

    GXBOOL  CalculateValue(OPERAND& sOut, const GLOB* pDesc);

    SYNTAXNODE* FlatDefinition(SYNTAXNODE* pThisChain);
    static GLOB* BreakDefinition(SYNTAXNODE::PtrList& sVarList, SYNTAXNODE* pNode); // 分散结构体成员
    static SYNTAXNODE::GlobList& BreakComma(SYNTAXNODE::GlobList& sExprList, const GLOB& sGlob); // 列出逗号并列式
    static SYNTAXNODE::GlobPtrList& BreakComma(SYNTAXNODE::GlobPtrList& sExprList, GLOB& sGlob); // 列出逗号并列式
    static SYNTAXNODE::GlobList& BreakChain(SYNTAXNODE::GlobList& sExprList, const GLOB& sGlob); // 列出链并列式，如果sGlob不是链，list为空
    static SYNTAXNODE::GlobList& BreakChain2(SYNTAXNODE::GlobList& sExprList, const GLOB& sGlob); // 列出链并列式，如果sGlob不是链，就直接填进去

    GXBOOL  ParseExpression(GLOB& glob, NameContext* pNameSet, const TKSCOPE& scope);
    GXBOOL  ParseToChain(GLOB& glob, NameContext* pNameSet, const TKSCOPE& scope);
    GXBOOL  ParseCodeBlock(GLOB& glob, NameContext* pNameSet, const TKSCOPE& scope);
    TKSCOPE::TYPE  TryParseSingle(NameContext* pNameSet, GLOB& glob, const TKSCOPE& scope); // 解析一个代码块, 一条关键字表达式或者一条表达式

    GXBOOL  TryKeywords(NameContext& sNameSet, const TKSCOPE& scope, GLOB* pDest, TKSCOPE::TYPE* parse_end);
    TKSCOPE::TYPE  ParseFlowIf(const NameContext& sParentSet, const TKSCOPE& scope, GLOB* pDesc, GXBOOL bElseIf);
    TKSCOPE::TYPE  MakeFlowForScope(const TKSCOPE& scope, TKSCOPE* pInit, TKSCOPE* pCond, TKSCOPE* pIter, TKSCOPE* pBlock, GLOB* pBlockNode);
    TKSCOPE::TYPE  ParseFlowFor(const NameContext& sParentSet, const TKSCOPE& scope, GLOB* pDesc);
    TKSCOPE::TYPE  ParseFlowWhile(const NameContext& sParentSet, const TKSCOPE& scope, GLOB* pDesc);
    TKSCOPE::TYPE  ParseFlowDoWhile(const NameContext& sParentSet, const TKSCOPE& scope, GLOB* pDesc);
    TKSCOPE::TYPE  ParseFlowSwitch(const NameContext& sParentCtx, const TKSCOPE& scope, GLOB* pDesc);
    TKSCOPE::TYPE  ParseFlowCase(const NameContext& sParentCtx, const TKSCOPE& scope, GLOB* pDesc);
    TKSCOPE::TYPE  ParseTypedef(NameContext& sNameSet, const TKSCOPE& scope, GLOB* pDesc);
    TKSCOPE::TYPE  ParseStructDefinition(NameContext& sNameSet, const TKSCOPE& scope, GLOB* pMembers, GLOB* pDefinitions, const TOKEN** ppName, int* pSignatures, int* pDefinition);

    //const TYPEDESC* GetMember(const NameSet& sNameSet, const SYNTAXNODE* pNode) const;
    //VALUE::State CalcuateConstantValue(VALUE& value_out, const NameContext& sNameSet, const SYNTAXNODE::GLOB* pGlob);

    GXBOOL  MakeScope(TKSCOPE* pOut, MAKESCOPE* pParam);
    GXBOOL  OnToken(TOKEN& token);
    void    OnMinusPlus(cllist<clsize>& UnaryPendingList, TOKEN& token, size_t c_size);
    void    GetNext(iterator& it);
    iterator  MakeupMacroFunc(TOKEN::List& stream, iterator& it, const iterator& end);
    void    ExpandMacroFunc(MACRO_EXPAND_CONTEXT& c);
    MacroExpand ExpandMacroContent(TOKEN::List& sTokenList, const TOKEN& line_num, MACRO_EXPAND_CONTEXT::IDSet_T* pOrderSet);
    MacroExpand TryMatchMacro(MACRO_EXPAND_CONTEXT& ctx_out, TOKEN::List::iterator* it_out, const TOKEN::List::iterator& it_begin, const TOKEN::List::iterator& it_end);
    GXBOOL  MergeStringToken(const TOKEN& token);

    auto AddMacro(const RefString& rstrMacroName, const MACRO& macro);
    void NormalizeMacro(const RefString& rstrMacroName);
    const MACRO* FindMacro(const TOKEN& token);

    T_LPCSTR DoPreprocess(const RTPPCONTEXT& ctx, T_LPCSTR begin, T_LPCSTR end);
    void     PP_Pragma(const TOKEN::Array& aTokens);
    void     PP_Define(const TOKEN::Array& aTokens);
    void     PP_Include(const TOKEN::Array& aTokens);
    void     PP_Undefine(const RTPPCONTEXT& ctx, const TOKEN::Array& aTokens);
    T_LPCSTR PP_IfDefine(const RTPPCONTEXT& ctx, GXBOOL bNot, const TOKEN::Array& aTokens); // bNot 表示 if not define
    T_LPCSTR PP_If(const RTPPCONTEXT& ctx, CodeParser* pParser);
    T_LPCSTR PP_SkipConditionalBlock(PPCondRank session, T_LPCSTR begin, T_LPCSTR end); // 从这条预处理的行尾开始，跳过这块预处理，begin应该是当前预处理的结尾
    T_LPCSTR PP_FindPreProcessIdentifier(T_LPCSTR begin, T_LPCSTR end);
    void     PP_UserError(T_LPCSTR position, const clStringW& strText);
    GXBOOL   ExpandInnerMacro(TOKEN& token, const TOKEN& line_num); // 主要是替换__FILE__ __LINE__


    static T_LPCSTR Macro_SkipComment(T_LPCSTR begin, T_LPCSTR end);  // 返回跳过注释的地址
    static T_LPCSTR Macro_SkipGapsAndNewLine(T_LPCSTR begin, T_LPCSTR end);  // 返回跳过制表符和空格后的字符串地址
    static T_LPCSTR Macro_SkipGaps(T_LPCSTR begin, T_LPCSTR end);  // 返回跳过制表符和空格后的字符串地址
    GXBOOL CompareString(T_LPCSTR str1, T_LPCSTR str2, size_t count);

    GXBOOL  ParseStatement(TKSCOPE* pScope);
    void    RelocalePointer();
    
    GXLPCSTR GetUniqueString(const TOKEN* pSym);
    GXLPCSTR GetUniqueString(T_LPCSTR szText);

    //void VarOutputErrorW(const TOKEN* pLocation, GXUINT code, va_list arglist) const;
    //void OutputErrorW(GXUINT code, ...);  // 从最后一个有效token寻找行号
    //void OutputErrorW(const GLOB& glob, GXUINT code, ...) const;
    //void OutputErrorW(const SYNTAXNODE* pNode, GXUINT code, ...) const;
    //void OutputErrorW(const TOKEN& token, GXUINT code, ...) const;
    //void OutputErrorW(const TOKEN* pToken, GXUINT code, ...) const;
    //void OutputErrorW(T_LPCSTR ptr, GXUINT code, ...);

    //void OutputMissingSemicolon(const TOKEN* ptkLocation); // 输出缺少分号的提示

    CodeParser* GetRootParser();
    clBuffer* OpenIncludeFile(const clStringW& strFilename);
    virtual T_LPCSTR GetOriginPtr(const TOKEN* pToken) const override;


#ifdef ENABLE_SYNTAX_VERIFY
    const TYPEDESC* InferUserFunctionType(FUNCDESC::CPtrList& aUserFunc, VALUE_CONTEXT& vctx, const TYPEDESC::CPtrList& sTypeList, const SYNTAXNODE* pFuncNode, int nStep); // 返回ERROR_TYPEDESC表示推导出现错误

    int CompareFunctionArguments(const NameContext &sNameSet, const TOKEN* ptkFuncName, const TYPEINSTANCE::Array& sFormalTypes, const TYPEDESC::CPtrList &sCallTypeList, GXBOOL bTolerance); // -1:出错，0：不匹配，1：匹配, bTolerance 更宽容的匹配

    static GXLPCSTR InferBuildinFunction(const RefString& rstrFunctionName, const TYPEDESC::CPtrList& sArgumentsTypeList, GXBOOL* pError);
    GXBOOL InferBuildinFunction_Wildcard(VALUE_CONTEXT& vctx, const RefString& rstrFunctionName, const SYNTAXNODE::GlobList& sExprList, const TYPEDESC::CPtrList& sArgumentsTypeList, const VALUE_CONTEXT::Array& vctx_params);
    GXBOOL InferBuildinFunction_WildcardTable(INTRINSIC_FUNC* pFunctionsTable, size_t nTableLen, VALUE_CONTEXT& vctx, const RefString& rstrFunctionName, const SYNTAXNODE::GlobList& sExprList, const TYPEDESC::CPtrList& sArgumentsTypeList);
    GXBOOL ExtendParamDimension(TYPEDESC::CPtrArray& aExtendArgumentTypes, const INTRINSIC_FUNC& test_func, const TYPEDESC::CPtrList& sArgumentsTypeList);
    const TYPEDESC* InferFunctionReturnedType(VALUE_CONTEXT& vctx, const SYNTAXNODE* pFuncNode);
    const TYPEDESC* InferConstructorsInStructType(const NameContext& sNameSet, const TYPEDESC::CPtrList& sArgumentsTypeList, const SYNTAXNODE* pFuncNode); // 扩展语法：结构体构造
    ValueResult TokenToValue(VALUE_CONTEXT& vctx, const TOKEN* pToken) const;
    const TYPEDESC* InferType(VALUE_CONTEXT& vctx, const GLOB& sGlob);
    const TYPEDESC* InferType(VALUE_CONTEXT& vctx, const TOKEN* pToken) const;
    const TYPEDESC* InferType(VALUE_CONTEXT& vctx, const SYNTAXNODE* pNode);
    const TYPEDESC* RearrangeInitList(size_t nTopIndex, const TYPEDESC* pRefType, CInitList& rInitList, size_t nDepth);
    const TYPEDESC* InferInitList_Struct(size_t nTopIndex, const TYPEDESC* pRefType, CInitList& rInitList, size_t nDepth);
    const TYPEDESC* InferInitList_LinearScalerArray(size_t nTopIndex, const TYPEDESC* pRefType, CInitList& rInitList, size_t nDepth);
    const TYPEDESC* InferInitList(ValuePool* pValuePool, NameContext& sNameSet, const TYPEDESC* pRefType, GLOB* pInitListGlob); // pInitListGlob.pNode->mode 必须是 MODE_InitList
    //const TYPEDESC* InferInitMemberList(const NameContext& sNameSet, const TYPEDESC* pLeftType, const GLOB* pInitListGlob); // pInitListGlob->pNode->mode 必须是 MODE_InitList, 或者pInitListGlob是token
    const TYPEDESC* InferMemberType(VALUE_CONTEXT& vctx, const SYNTAXNODE* pNode);
    const TYPEDESC* InferSubscriptType(VALUE_CONTEXT& vctx, const TYPEDESC* pDotOverrideType, const SYNTAXNODE* pNode);
    const TYPEDESC* InferSubscriptTypeB(VALUE_CONTEXT& vctx, const SYNTAXNODE* pNode);
    const TYPEDESC* InferTypeByOperator(const TOKEN* pOperator, const TYPEDESC* pFirst, const TYPEDESC* pSecond);
    const TYPEDESC* InferDifferentTypesOfCalculations(const TOKEN* pToken, const TYPEDESC* pFirst, const TYPEDESC* pSecond) const;
    const TYPEDESC* InferTypesOfMultiplication(const TYPEDESC* pFirst, const TYPEDESC* pSecond) const;
    //const TYPEDESC* InitList_SyncType(NameContext& sNameSet, const TYPEDESC* pRefType, const TYPEDESC* pListType, const GLOB* pElementGlob);
    const TYPEDESC* InitList_CastType(const TYPEDESC* pLeftType, const TYPEDESC* pListType, size_t nListCount, const GLOB* pLocation);
#endif

    const TYPEDESC* InferRightValueType(NameContext& sNameSet, const GLOB& right_glob, const TOKEN* ptkLocation);
    GXBOOL CompareScaler(const RefString& rstrTypeFrom, GXLPCSTR szTypeTo);
    GXBOOL TryTypeCasting(const TYPEDESC* pTypeTo, const TYPEDESC* pTypeFrom, const TOKEN* pLocation, GXBOOL bFormalParam = FALSE); // pLocation 用于错误输出定位
    GXBOOL MergeValueContext(VALUE_CONTEXT& vctx, const TOKEN* pOperator, VALUE_CONTEXT* pAB, const TOKEN* pLocation); // pLocation 用于错误输出定位
    GXBOOL TryTypeCasting(const NameContext& sNameSet, GXLPCSTR szTypeTo, const TYPEDESC* pTypeFrom, const TOKEN* pLocation); // pLocation 用于错误输出定位
    GXBOOL TryTypeCasting(const NameContext& sNameSet, GXDWORD dwArgMask, const TYPEDESC* pTypeFrom, const TOKEN* pLocation); // pLocation 用于错误输出定位
    GXBOOL TryReinterpretCasting(const TYPEDESC* pTypeTo, const TYPEDESC* pTypeFrom, const TOKEN* pLocation); // pLocation 用于错误输出定位

    //static GXLPCSTR ResolveType(const TYPEDESC* pTypeDesc, int& R, int& C);
    static GXBOOL IsComponent(const TYPEDESC* pRowVector, const TYPEDESC* pMatrixDesc, const TYPEDESC* pColumnVector);

#ifdef ENABLE_SYNTAX_VERIFY
    //const TYPEDESC2* Verify_Type(const TOKEN& tkType);
    //const TYPEDESC* Verify_Struct(const TOKEN& tkType, const NameContext* pNameSet);
    static const TOKEN* GetIdentifierNameWithoutSeamantic(const GLOB& glob); // 取去掉语意的变量名，如“vColor”
    const SYNTAXNODE::GLOB* GetIdentifierDeclWithoutSeamantic(const GLOB& glob); // 取去掉语意的变量声明，可能含有下标，如“vColor[2][3]”
    GXBOOL Verify_MacroFormalList(const MACRO_TOKEN::List& sFormalList);

    GXBOOL Verify_IdentifierTypedDefinition(NameContext& sNameSet, const TOKEN& tkType, const GLOB& second_glob, GXDWORD dwFlags = 0); // VerifyIdentifierDefinition_*
    GXBOOL Verify_IdentifierDefinition(NameContext& sNameSet, const SYNTAXNODE* pNode, GXDWORD dwFlags = 0); // VerifyIdentifierDefinition_*

    //GXBOOL Verify2_VariableInit(NameContext& sNameSet, const TYPEDESC* pType, const SYNTAXNODE& rNode);
    //GXBOOL Verify_FunctionBlock(const STATEMENT_EXPR& expr);
    GXBOOL Verify_Block(const SYNTAXNODE* pNode, const NameContext* pParentSet);
    GXBOOL Verify_Chain(const SYNTAXNODE* pNode, NameContext& sNameContext);
    GXBOOL Verify_Node(const SYNTAXNODE* pNode, NameContext& sNameContext, GXBOOL& result);
    GXBOOL Verify_StructMember(NameContext& sParentSet, const RefString& rstrStructName, const SYNTAXNODE& rNode);
    const TYPEDESC* Verify2_LeftValue(const NameContext& sNameSet, const GLOB& left_glob, const TOKEN& opcode); // opcode 主要是为了定位
    //GXBOOL Verify2_RightValue(const NameContext& sNameSet, const TYPEDESC* pType, SYNTAXNODE::MODE mode, const GLOB& right_glob);
#endif
    static GXBOOL DumpValueState(CLogger* pLogger, VALUE::State state, const TOKEN* pToken); // 警告返回FALSE，错误返回TRUE
    static void DumpStateError(CLogger* pLogger, NameContext::State state, const TOKEN& tkType, const TOKEN& tkVar);


    void SetRepalcedValue(const GLOB& glob, const VALUE& value);
    PHONYTOKEN* AddPhonyToken(TOKEN::Type type, const clStringA& str, const TOKEN* pToken);

    //VALUE::State CalculateValueAsConstantDefinition(VALUE& value_out, NameContext& sNameCtx, const GLOB& const_expr_glob);

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
    GXDWORD             m_dwState;          // 内部状态, 参考AttachFlag
    GXDWORD             m_dwParserState;    // 内部状态, 参考RTState
    CodeParser*         m_pParent;
    TypeSet             m_TypeSet;
    StatementArray      m_aStatements;
    StatementArray      m_aSubStatements;   // m_aStatements的次级储存，没有顺序关系
    int                 m_nPPRecursion;     // 条件预处理递归次数

    
    PARSER_CONTEXT*     m_pContext;
    PhonyTokenDict_T    m_PhonyTokenDict;   // 用户从替换的token中找到原始token信息
    PHONYTOKEN::Array   m_aPhonyTokens;     // 储存假的token


    FileDict            m_sIncludeFiles;    // 包含文件集合, 仅限于Root parser
    Include*            m_pInclude;
    CodeParser*         m_pSubParser;
    TOKEN::List         m_ExpandedStream;   // 宏展开流
    NameContext         m_GlobalCtx;
    ValueDict           m_ValueDict;        // 常量表达式可以直接替换成数字的

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