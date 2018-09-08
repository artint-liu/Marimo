#ifndef _EXPRESSION_PARSER_H_
#define _EXPRESSION_PARSER_H_

#define ENABLE_SYNTAX_VERIFY // 语法检查开关, 只用来做语法解析临时调试用, 正式版不要关闭这个

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
    KeywordFilter_return   = 0x0008,    // 有返回值的函数才开启这个
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
      TypeCate_Flag_Scaler   = 0x10000,
      TypeCate_Flag_MultiDim = 0x20000,
      TypeCate_Flag_Sampler  = 0x40000,
      TypeCate_Flag_Struct   = 0x80000,
    };
    typedef clvector<size_t> DimList_T;

    TypeCate            cate;
    NameContext* const  pNameCtx;
    clStringA           name; // 类型名
    COMMINTRTYPEDESC*   pDesc; // 结构体(内置) 的描述, 多维数组这个指向它的基础类型
    const SYNTAXNODE*   pMemberNode; // 结构体(用户定义) 的描述, 必须是"SYNTAXNODE::MODE_Block"
    DimList_T           sDimensions; // 维度列表 int var[a][b][c][d] 储存为{d，c，b，a}
    const TYPEDESC*     pElementType; // float[3][2]的 pElementType=float[2]，float3[2]的pElementType=float3，float3的pElementType=float，只有数学类型才有

    CLogger* GetLogger() const;
    static GXBOOL MatchScaler(const TOKEN* ptkMember, GXLPCSTR scaler_set); // 保证.xxxx, .xyxy, .yxwz这种也是合理的成员
    GXLPCSTR Resolve(int& R, int& C) const;
    GXBOOL IsVector() const;
    GXBOOL IsMatrix() const;
    GXBOOL IsSameType(const TYPEDESC* pOtherTypeDesc) const;
    GXBOOL GetMemberTypeList(TYPEDESC::CPtrList& sMemberTypeList) const;
    size_t CountOf() const; // 获得向量，矩阵和数组包含的基础类型数量
    const TYPEDESC* ConfirmArrayCount(size_t nCount) const;
  };

  struct FUNCDESC // 用户定义的函数描述
  {
    // FIXME: 因为定义顺序关系, 返回值和形参应改储存TYPEDESC, 而不是名字, 这个要改暂时备忘
    clStringA         ret_type;     // 返回的类型
    clStringA         name;         // 类型名
    TOKEN::PtrCArray  sFormalTypes; // 函数, 形参类型表
  };

  struct INTRINSIC_FUNC
  {
    enum RetType
    {
      RetType_Scaler0 = -1,
      //RetType_FromName = -2, // 返回类型是函数名
      //RetType_Vector4 = -3,  // 4维向量,一般代表颜色
      RetType_Bool = -3,
      RetType_Float4 = -4,  // 4维向量,一般代表颜色
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
    ValueResult_NotConst,
    ValueResult_NotStructMember,
    ValueResult_3861, // 找不到标识符
    ValueResult_5039, // 函数参数数量不匹配
  };

  struct VARIDESC
  {
    const TYPEDESC* pDesc;
    size_t nOffset;
    VALUE sConstValue;      // 规定 string 类型不能以 const 修饰，所以这里用VALUE
    SYNTAXNODE::GLOB  glob; // 所有常量都应该定义这个值，数字类型常量同时还应该定义sConstValue
  };

  enum InputModifier // 函数参数修饰
  {
    InputModifier_in,
    InputModifier_out,
    InputModifier_inout,
    InputModifier_uniform, // Input only constant data
  };

  struct FUNCTION_ARGUMENT // 函数参数
  {
    InputModifier eModifier;  // [opt]
    const TOKEN*  ptkType;    // [req]
    const TOKEN*  ptkName;    // [req]
    GXLPCSTR      szSemantic; // [opt]
  };

  struct VALUE_CONTEXT
  {
    // [属性]
    const NameContext&  name_ctx;
    GXBOOL              bNeedValue;
    CLogger*            pLogger;

    // [值]
    ValueResult     result;
    const TYPEDESC* pType;
    const VALUE*    pValue;
    size_t          count;
    ValuePool       pool;

    VALUE_CONTEXT(const VALUE_CONTEXT& vctx);
    VALUE_CONTEXT(const NameContext& _name_ctx);
    VALUE_CONTEXT(const NameContext& _name_ctx, GXBOOL _bNeedValue);

    void SetProperty(const VALUE_CONTEXT& vctx); // 从vctx复制属性
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
      State_HasError = -1,           // 其它错误，已经在函数内部输出
      State_TypeNotFound = -2,       // 没有找到类型
      State_DuplicatedType = -3,     // 重复注册类型
      State_DuplicatedVariable = -4, // 重复定义变量
      State_DefineAsType = -5,       // 变量已经被定义为类型
      State_DefineAsVariable = -6,   // 类型已经被定义为变量
      State_VariableIsNotIdentifier = -7, // 期望的变量名不是一个标识符
      State_RequireConstantExpression = -8, // 需要常量表达式
      State_RequireSubscript = -9,   // 缺少下标: int a[2][] = {};
      State_CastTypeError = -10,     // 类型转换错误，含有的右值不能转换成变量类型
    };

  protected:
    CodeParser* m_pCodeParser;
    clStringA m_strName;    // 域名
    const NameContext* m_pParent;     // 默认记录的祖先
    const NameContext* m_pVariParent; // 变量记录的祖先，比如结构体成员NameContext.m_pVariParent应该为空

    TypeMap     m_TypeMap;  // typedef 会产生两个内容相同的TYPEDESC
    FuncMap     m_FuncMap;
    VariableMap m_VariableMap;
    State       m_eLastState;
    StructContextMap  m_StructContextMap; // 结构体成员的NameContext
    ValuePoolMap      m_ValuePoolMap;
    const TYPEDESC*   m_pReturnType;
    size_t      m_nCount;

    NameContext* GetRoot();
    const NameContext* GetRoot() const;

  private: // 禁止拷贝构造和复制构造
    NameContext(const NameContext& sNameCtx){}
    NameContext& operator=(const NameContext sNameCtx) { return *this; }

    State IntRegisterVariable(const TYPEDESC** ppType, VARIDESC** ppVariable, const TYPEDESC* pTypeDesc, const TOKEN* ptkVariable, const VALUE* pConstValue, const GLOB* pValueExprGlob);
    State IntRegisterVariable(const TYPEDESC** ppType, VARIDESC** ppVariable, const clStringA& strType, const TOKEN* ptkVariable, const VALUE* pConstValue, const GLOB* pValueExprGlob);
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

    GXBOOL SetReturnType(GXLPCSTR szTypeName);
    const TYPEDESC* GetReturnType() const;

    const TYPEDESC* GetType(const clStringA& strType) const;
    const TYPEDESC* GetType(const TOKEN& token) const;
    const TYPEDESC* GetType(VALUE::Rank rank) const;
    const TYPEDESC* ConfirmArrayCount(const TYPEDESC* pTypeDesc, size_t nCount); // 设置不确定长度数组类型的长度
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
    const TYPEDESC* RegisterVariable(const clStringA& strType, const GLOB* pVariableDeclGlob, const VALUE* pConstValue = NULL, const GLOB* pValueExprGlob = NULL); // TODO: 应该增加个第一参数是TYPEDESC的重载
    const TYPEDESC* RegisterVariable(const clStringA& strType, const TOKEN* ptkVariable, const VALUE* pConstValue = NULL, const GLOB* pValueExprGlob = NULL); // TODO: 应该增加个第一参数是TYPEDESC的重载
    //void ChangeVariableType(const TYPEDESC* pTypeDesc, const SYNTAXNODE* pVariableDeclNode); // 只能改变之前没确定长度数组类型的变量
#ifdef ENABLE_SYNTAX_VERIFY
    const TYPEDESC* RegisterTypes(const clStringA& strBaseType, const TYPEDESC::DimList_T& sDimensions); // 根据多维列表依次注册类型，返回值是最高维度类型
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
      const TOKEN* ptkOpcode; // 用于输出定位
    };



    CodeParser*             m_pCodeParser;
    NameContext&            m_rNameCtx;
    const SYNTAXNODE::GLOB* m_pInitListGlob;
    cllist<STACKDESC>       m_sStack;
    GXBOOL                  m_bNeedAlignDepth;
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
    enum Result
    {
      Result_Failed = -1,
      Result_Ok = 0,
      Result_ExpandVecMat = 1,  // 展开函数形式向量或者矩阵（类似"float3(a,b,c)"形式）初始化
      Result_NotAligned = 2,    // 函数形式向量或者矩阵（类似"float3(a,b,c)"形式）初始化与index没对齐
    };

    CInitList(CodeParser* pCodeParser, NameContext& rNameCtx, const SYNTAXNODE::GLOB* pInitListGlob);
    CLogger* GetLogger();
    void SetValuePool(VALUE* pValuePool, size_t count);
    const SYNTAXNODE::GLOB* Get();
    Result CastToValuePool(const TYPEDESC* pRefTypeDesc, size_t base_index, size_t array_index);
    const TOKEN* GetLocation() const; // 获得代码位置相关的Glob, 用于错误输出定位
    const SYNTAXNODE::GLOB* Step();
    GXBOOL Step(size_t nDimDepth, size_t nListDepth);
    GXBOOL IsEnd() const;
    GXBOOL Empty() const;
    size_t Depth() const;
    GXBOOL NeedAlignDepth(size_t nDimDepth, size_t nListDepth) const;
    void ClearAlignDepthFlag();

    size_t GetMaxCount(const TYPEDESC* pRefType) const; // 如果是自适应长度，用这个来获得最大可能的长度，否则返回类型的实际长度
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
      typedef clmap<clStringA, MACRO> Dict; // macro 支持重载，所以 key 的名字用 clStringA 来储存, Dict 要求具有排序性
      typedef clvector<int>           IntArray;
      //typedef clhash_set<clStringA>   Set;
      MACRO_TOKEN::Array aFormalParams; // 形参
      MACRO_TOKEN::List aTokens;        // 替换内容
      int               nOrder;         // 定义顺序，相当于id，防止无限展开
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
    };

    struct MACRO_EXPAND_CONTEXT
    {
      typedef clset<int> OrderSet_T;
      const TOKEN* pLineNumRef;
      TOKEN::List stream;
      const MACRO* pMacro;

      clvector<TOKEN::List> ActualParam;
      OrderSet_T OrderSet; // 展开宏的集合，防止无限展开递归
    };

    //////////////////////////////////////////////////////////////////////////

    enum AttachFlag // 注意: 与RTState公用标记位
    {
      AttachFlag_Preprocess     = 0x00000001, // 在预处理中，这个状态不再认为#为预处理开始标记
      //AttachFlag_NotLoadMessage = 0x00010000, // TODO: 这个干掉？！
      AttachFlag_NotExpandMacro = 0x00020000, // 不展开宏
      AttachFlag_NotExpandCond  = 0x00040000, // 不展开defined()里面的宏
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
    struct STATEMENT_FUNC // 函数体/函数声明
    {
      FunctionStorageClass eStorageClass; // [opt]
      GXLPCSTR     szReturnType;  // [req]
      GXLPCSTR     szName;        // [req]
      GXLPCSTR     szSemantic;    // [opt]
      FUNCTION_ARGUMENT*  pArguments;       // [opt]
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


    enum RTState // 运行时状态, 与AttachFlag公用标记位
    {
      State_InPreprocess = 0x00000001, // 解析预处理
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
    GXBOOL  ParseFunctionArguments(NameContext& sNameSet, STATEMENT* pStat, TKSCOPE* pArgScope, int& nTypeOnlyCount);

    GXBOOL  ParseStatementAs_Typedef(TKSCOPE* pScope);
    GXBOOL  ParseStatementAs_Struct(TKSCOPE* pScope);

    //GXBOOL  ParseStatementAs_Expression(STATEMENT* pStat, NameSet& sNameSet, TKSCOPE* pScope); // (算数表)达式

    GXBOOL  CalculateValue(OPERAND& sOut, const GLOB* pDesc);

    SYNTAXNODE* FlatDefinition(SYNTAXNODE* pThisChain);
    static GLOB* BreakDefinition(SYNTAXNODE::PtrList& sVarList, SYNTAXNODE* pNode); // 分散结构体成员
    static SYNTAXNODE::GlobList& BreakComma(SYNTAXNODE::GlobList& sExprList, const GLOB& sGlob); // 列出逗号并列式
    static SYNTAXNODE::GlobPtrList& BreakComma(SYNTAXNODE::GlobPtrList& sExprList, GLOB& sGlob); // 列出逗号并列式
    static SYNTAXNODE::GlobList& BreakChain(SYNTAXNODE::GlobList& sExprList, const GLOB& sGlob); // 列出链并列式

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
    T_LPCSTR PP_IfDefine(const RTPPCONTEXT& ctx, GXBOOL bNot, const TOKEN::Array& aTokens); // bNot 表示 if not define
    T_LPCSTR PP_If(const RTPPCONTEXT& ctx, CodeParser* pParser);
    T_LPCSTR PP_SkipConditionalBlock(PPCondRank session, T_LPCSTR begin, T_LPCSTR end); // 从这条预处理的行尾开始，跳过这块预处理，begin应该是当前预处理的结尾
    T_LPCSTR PP_FindPreProcessIdentifier(T_LPCSTR begin, T_LPCSTR end);
    void     PP_UserError(T_LPCSTR position, const clStringW& strText);
    GXBOOL   ExpandInnerMacro(TOKEN& token, const TOKEN& line_num); // 主要是替换__FILE__ __LINE__


    static T_LPCSTR Macro_SkipGapsAndNewLine(T_LPCSTR begin, T_LPCSTR end);  // 返回跳过制表符和空格后的字符串地址
    static T_LPCSTR Macro_SkipGaps(T_LPCSTR begin, T_LPCSTR end);  // 返回跳过制表符和空格后的字符串地址
    GXBOOL CompareString(T_LPCSTR str1, T_LPCSTR str2, size_t count);

    GXBOOL  ParseStatement(TKSCOPE* pScope);
    void    RelocaleStatements(StatementArray& aStatements);
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
    const TYPEDESC* InferUserFunctionType(const NameContext& sNameSet, const TYPEDESC::CPtrList& sTypeList, const SYNTAXNODE* pFuncNode); // 返回ERROR_TYPEDESC表示推导出现错误
    const TYPEDESC* InferFunctionReturnedType(VALUE_CONTEXT& vctx, const SYNTAXNODE* pFuncNode);
    const TYPEDESC* InferType(VALUE_CONTEXT& vctx, const NameContext& sNameSet, const GLOB& sGlob);
    const TYPEDESC* InferType(VALUE_CONTEXT& vctx, const TOKEN* pToken) const;
    const TYPEDESC* InferType(VALUE_CONTEXT& vctx, const NameContext& sNameSet, const SYNTAXNODE* pNode);
    const TYPEDESC* RearrangeInitList(size_t nTopIndex, const TYPEDESC* pRefType, CInitList& rInitList, size_t nDepth);
    const TYPEDESC* InferInitList_Struct(size_t nTopIndex, const TYPEDESC* pRefType, CInitList& rInitList, size_t nDepth);
    const TYPEDESC* InferInitList_LinearArray(size_t nTopIndex, const TYPEDESC* pRefType, CInitList& rInitList, size_t nDepth);
    const TYPEDESC* InferInitList(ValuePool* pValuePool, NameContext& sNameSet, const TYPEDESC* pRefType, GLOB* pInitListGlob); // pInitListGlob.pNode->mode 必须是 MODE_InitList
    //const TYPEDESC* InferInitMemberList(const NameContext& sNameSet, const TYPEDESC* pLeftType, const GLOB* pInitListGlob); // pInitListGlob->pNode->mode 必须是 MODE_InitList, 或者pInitListGlob是token
    const TYPEDESC* InferMemberType(VALUE_CONTEXT& vctx, const SYNTAXNODE* pNode);
    const TYPEDESC* InferSubscriptType(VALUE_CONTEXT& vctx, const SYNTAXNODE* pNode);
    const TYPEDESC* InferSubscriptTypeB(VALUE_CONTEXT& vctx, const SYNTAXNODE* pNode);
    const TYPEDESC* InferTypeByOperator(const TOKEN* pOperator, const TYPEDESC* pFirst, const TYPEDESC* pSecond);
    static const TYPEDESC* InferDifferentTypesOfCalculations(const TOKEN* pToken, const TYPEDESC* pFirst, const TYPEDESC* pSecond);
    static const TYPEDESC* InferDifferentTypesOfMultiplication(const TYPEDESC* pFirst, const TYPEDESC* pSecond);
    const TYPEDESC* InitList_SyncType(NameContext& sNameSet, const TYPEDESC* pRefType, const TYPEDESC* pListType, const GLOB* pElementGlob);
    const TYPEDESC* InitList_CastType(const TYPEDESC* pLeftType, const TYPEDESC* pListType, size_t nListCount, const GLOB* pLocation);
#endif

    const TYPEDESC* InferRightValueType(NameContext& sNameSet, const TYPEDESC* pLeftTypeDesc, const GLOB* pVarGlob, const GLOB& right_glob, const TOKEN* pLocation); // pLocation 用于错误输出定位
    GXBOOL CompareScaler(GXLPCSTR szTypeFrom, GXLPCSTR szTypeTo);
    GXBOOL TryTypeCasting(const TYPEDESC* pTypeTo, const TYPEDESC* pTypeFrom, const TOKEN* pLocation); // pLocation 用于错误输出定位
    GXBOOL TryTypeCasting(const NameContext& sNameSet, GXLPCSTR szTypeTo, const TYPEDESC* pTypeFrom, const TOKEN* pLocation); // pLocation 用于错误输出定位
    GXBOOL TryTypeCasting(const NameContext& sNameSet, GXDWORD dwArgMask, const TYPEDESC* pTypeFrom, const TOKEN* pLocation); // pLocation 用于错误输出定位
    GXBOOL TryReinterpretCasting(const TYPEDESC* pTypeTo, const TYPEDESC* pTypeFrom, const TOKEN* pLocation); // pLocation 用于错误输出定位

    //static GXLPCSTR ResolveType(const TYPEDESC* pTypeDesc, int& R, int& C);
    static GXBOOL IsComponent(const TYPEDESC* pRowVector, const TYPEDESC* pMatrixDesc, const TYPEDESC* pColumnVector);

#ifdef ENABLE_SYNTAX_VERIFY
    //const TYPEDESC2* Verify_Type(const TOKEN& tkType);
    //const TYPEDESC* Verify_Struct(const TOKEN& tkType, const NameContext* pNameSet);
    static const TOKEN* GetVariableNameWithoutSeamantic(const GLOB& glob); // 取去掉语意的变量名，如“vColor”
    const SYNTAXNODE::GLOB* GetVariableDeclWithoutSeamantic(const GLOB& glob); // 取去掉语意的变量声明，可能含有下标，如“vColor[2][3]”
    GXBOOL Verify_MacroFormalList(const MACRO_TOKEN::List& sFormalList);

    GXBOOL Verify_VariableTypedDefinition(NameContext& sNameSet, const TOKEN& tkType, const GLOB& second_glob, GXBOOL bConstVariable = FALSE, GXBOOL bMember = FALSE);
    GXBOOL Verify_VariableDefinition(NameContext& sNameSet, const SYNTAXNODE* pNode, GXBOOL bConstVariable = FALSE, GXBOOL bMember = FALSE);
    //GXBOOL Verify2_VariableInit(NameContext& sNameSet, const TYPEDESC* pType, const SYNTAXNODE& rNode);
    //GXBOOL Verify_FunctionBlock(const STATEMENT_EXPR& expr);
    GXBOOL Verify_Chain(const SYNTAXNODE* pNode, NameContext& sNameContext);
    GXBOOL Verify_Block(const SYNTAXNODE* pNode, const NameContext* pParentSet);
    GXBOOL Verify_StructMember(NameContext& sParentSet, const clStringA& strStructName, const SYNTAXNODE& rNode);
    const TYPEDESC* Verify2_LeftValue(const NameContext& sNameSet, const GLOB& left_glob, const TOKEN& opcode); // opcode 主要是为了定位
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
    GXDWORD             m_dwState;          // 内部状态, 参考RTState
    CodeParser*         m_pParent;
    TypeSet             m_TypeSet;
    StatementArray      m_aStatements;
    StatementArray      m_aSubStatements;   // m_aStatements的次级储存，没有顺序关系
    int                 m_nPPRecursion;     // 条件预处理递归次数

    
    PARSER_CONTEXT*     m_pContext;
    PhonyTokenDict_T    m_PhonyTokenDict;   // 用户从替换的token中找到原始token信息

    ArgumentsArray      m_aArgumentsPack;   // 所有函数参数都存在这个表里

    FileDict            m_sIncludeFiles;    // 包含文件集合, 仅限于Root parser
    Include*            m_pInclude;
  //MACRO::Set          m_MacrosSet;        // 实名集合
    CodeParser*         m_pSubParser;
    //MACRO_EXPAND_CONTEXT::List m_sMacroStack;
    TOKEN::List         m_ExpandedStream;   // 宏展开流
    NameContext         m_GlobalSet;
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