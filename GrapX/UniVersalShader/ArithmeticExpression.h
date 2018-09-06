#ifndef _ARITHMETIC_EXPRESSION_H_
#define _ARITHMETIC_EXPRESSION_H_

#define IDX2ITER(_IDX)                           m_aTokens[_IDX]

#define UNARY_LEFT_OPERAND    2 // 10B， 注意是允许操作数在左侧
#define UNARY_RIGHT_OPERAND   1 // 01B
#define ENABLE_STRINGED_SYMBOL
#define OPP(_PRE) (TOKEN::FIRST_OPCODE_PRECEDENCE + _PRE)

#define USE_CLSTD_TOKENS

// 注意UVS_EXPORT_TEXT不能改名, 它在Sample中作为标记符号抽取ErrorMessage
#if defined(UVS_EXPORT_TEXT_IS_SIGN)
#define UVS_EXPORT_TEXT(_CODE_ID, _MESSAGE)  GetLogger()->SetError(GetLogger()->MarkCode(_CODE_ID, _MESSAGE))
#define UVS_EXPORT_TEXT2(_CODE_ID, _MESSAGE, _THIS)  _THIS->SetError(_THIS->MarkCode(_CODE_ID, _MESSAGE))
#else
#define UVS_EXPORT_TEXT(_CODE_ID, _MESSAGE)  GetLogger()->SetError(_CODE_ID)
#define UVS_EXPORT_TEXT2(_CODE_ID, _MESSAGE, _THIS)  _THIS->SetError(_CODE_ID)
#endif

#define ENABLE_SYNTAX_NODE_ID
#define REFACTOR_COMMA
namespace Marimo
{
  template<typename _TChar>
  class DataPoolErrorMsg;
} // namespace Marimo

namespace UVShader
{
  //class ArithmeticExpression : public SmartStreamA
  typedef clstd::TokensT<clStringA> CTokens;
# define TOKENSUTILITY clstd::TokensUtility
  class ArithmeticExpression;
  class NameContext;
  const size_t MAX_COMPONENTS = 16; // 4x4矩阵

  struct MEMBERLIST
  {
    GXLPCSTR type;      // 类型
    GXLPCSTR name;      // 成员名
  };

  //struct CONSTANTVALUE
  //{
  //  static const int MAX_VALUE = 16; // 最大就是一个4x4矩阵
  //  const TYPEDESC* pTypeDesc;
  //  VALUE           values[MAX_VALUE];
  //  size_t          count;
  //};

  struct DOTOPERATOR_RESULT
  {
    clStringA strType;
    int       components[MAX_COMPONENTS];
  };

  struct TOKEN;
  struct TYPEDESC;
  struct COMMINTRTYPEDESC;

  typedef GXBOOL (GXCALLBACK *OPERATORPROC_TOKEN)(const COMMINTRTYPEDESC* pDesc, DOTOPERATOR_RESULT* pResult, const TOKEN* pToken);
  typedef const TYPEDESC* (GXCALLBACK *OPERATORPROC_NAMECTX)(const COMMINTRTYPEDESC* pDesc, const NameContext& sNameCtx);

  struct COMMINTRTYPEDESC
  {
    GXLPCSTR    name;   // 结构体名
    GXLPCSTR    component_type;
    i16         rank;   // VALUE::Rank
    i16         cate;   // TYPEDESC::TypeCate
    OPERATORPROC_TOKEN lpDotOverride; // "."重载
    OPERATORPROC_NAMECTX lpSubscript; // "[]"下标重载
  };

  // 运行时记录符号和操作符等属性
  struct TOKEN : CTokens::iterator
  {
    typedef cllist<TOKEN>   List;
    typedef clvector<const TOKEN*>   PtrCArray;
    typedef clvector<TOKEN> Array;
    typedef CTokens::T_LPCSTR T_LPCSTR;
    typedef CTokens::TChar TChar;
    typedef clStringA _TStr;

    enum Type
    {
      TokenType_Identifier = 0,
      TokenType_String = 1,
      TokenType_FormalParams = 2,  // 宏的形参

      TokenType_FirstNumeric = 5,
      TokenType_Integer, // 整数, 包括正数负数
      TokenType_Real,    // 浮点数, float, double
      TokenType_LastNumeric,

      //TokenType_Name = 20,
      TokenType_Bracket = 20, // 括号
      TokenType_Semicolon,    // 分号
      TokenType_Operator = 30,
    };

    const static int scope_bits = 22;
    const static int precedence_bits = 7;
    const static int FIRST_OPCODE_PRECEDENCE = 1;
    const static int ID_BRACE = 15;
#ifdef ENABLE_STRINGED_SYMBOL
    clStringA symbol;
#endif // #ifdef ENABLE_STRINGED_SYMBOL
    //iterator  marker;
    //T_LPCSTR  str_head;
    //size_t    length;
    int       scope;                        // 括号匹配索引
    int       semi_scope : scope_bits;      // 分号作用域
    int       precedence : precedence_bits; // 符号优先级
    u32       unary : 1;                    // 是否为一元操作符
    u32       unary_mask : 2;               // 一元操作符允许位：11B表示操作数可以在左边和右边，01B表示操作数只能在右边，10B表示操作数只能在左边

    Type      type : 8;                     // 类型
    u32       bPhony : 1;                   // 在String字典而不在流中

    // [precedence]
    // 1~14 表示运算符的优先级
    // 15 表示"{","}","(",")","[","]"这些之一，此时scope数据是相互对应的，例如
    // array[open].scope = closed && array[closed].scope = open

    void ClearMarker();
    //void Set(const iterator& _iter);
    void Set(clstd::StringSetA& sStrSet, const clStringA& str); // 设置外来字符串
    void SetPhonyString(T_LPCSTR szText, size_t len); // 设置外来字符串

    template<class _Ty>
    void SetArithOperatorInfo(const _Ty& t) // 设置数学符号信息
    {
      unary = t.unary;
      unary_mask = t.unary_mask;
      precedence = t.precedence;
    }

    void ClearArithOperatorInfo();
    clStringA ToString() const;
    clStringA& ToString(clStringA& str) const;
    clStringW& ToString(clStringW& str) const;
    int GetScope() const;

    b32 IsIdentifier() const; // 标识符, 字母大小写, 下划线开头, 本体是字母数字, 下划线的字符串
    b32 IsNumeric() const;
    b32 HasReplacedValue() const;

    //TOKEN& operator++();
    //b32  operator!=(const iterator& it) const;

    GXBOOL operator==(const TOKEN& t) const;
    GXBOOL operator==(SmartStreamA::T_LPCSTR str) const;
    GXBOOL operator==(SmartStreamA::TChar ch) const;
    GXBOOL operator!=(SmartStreamA::T_LPCSTR str) const;
    GXBOOL operator!=(SmartStreamA::TChar ch) const;
    b32    operator<(const TOKEN& _token) const;
    clsize offset() const;

  public:

    // offset是指begin在序列中的索引, 用来修正括号匹配. 这么写的原因是_Iter类型有可能不支持“+”操作
    template<class _List, class _Iter, class _Fn>
    static void Append(_List& tokens, int offset, const _Iter& begin, const _Iter& end, _Fn fn)
    {
      int addi = (int)tokens.size() - offset;
      for(_Iter it = begin; it != end; ++it)
      {
        tokens.push_back(*it);

        auto& back = tokens.back();

        fn(addi, back); // 这个可能会改变addi，所以必须在这里做

        if(back.scope != -1) {
          back.scope += addi;
        }

        if(back.semi_scope != -1) {
          back.semi_scope += addi;
        }
        // TODO: 作为 +/- 符号 precedence 也可能会由于插入后上下文变化导致含义不同
      }
    }

    // 调试模式检查tokens序列合法性, 主要是检查scope是否匹配
    static GXBOOL DbgCheck(const Array& tokens, int begin)
    {
      int i = begin;
      for(auto it = tokens.begin() + begin; it != tokens.end(); ++it, ++i)
      {
        // 检查括号是否匹配
        if(it->scope != -1 && tokens[it->scope].scope != i) {
          return FALSE;
        }

        // 检查分号是否标记正确
        if(it->semi_scope != -1 && tokens[it->semi_scope] != ";") {
          return FALSE;
        }
      }

      return TRUE;
    }
  }; // struct TOKEN

  class TokenPtr
  {
    const TOKEN* ptr;

  public:
    TokenPtr(const TOKEN* p) : ptr(p) {}

    const TOKEN* Get() const
    {
      return ptr;
    }

    bool operator<(const TokenPtr& tp2) const;
  };

  //////////////////////////////////////////////////////////////////////////
  struct VALUE
  {
    enum State // Flags
    {
      State_OK = 0,
      State_Identifier    = 0x00000001, // 标识符
      State_Call          = 0x00000002, // 函数调用
      State_UnknownOpcode = 0x00000004, // 无法识别的操作符 FIXME: 这是错误消息？
      State_Truncation    = 0x00000008, // 类型被截断
      State_LoseOfData    = 0x00000010, // 类型转换可能丢失是数据

      State_WarningMask   = 0x0000ffff, // 警告的掩码
      State_ErrorMask     = 0xffff0000, // 错误的掩码
      State_SyntaxError   = 0x80000000,
      State_Overflow      = 0x20000000,
      State_IllegalChar   = 0x40000000,
      State_BadOpcode     = 0x10000000, // 错误的操作符
      State_IllegalNumber = 0x08000000, // 非法数字, 入8进制下的"05678"
      State_BadIdentifier = 0x04000000, // 找不到标识符
      State_DivideByZero  = 0x02000000, // 除零
    };

    enum Rank {
      // 这些值由特殊用法不能轻易修改
      Rank_Unsigned = 0,   // 000B
      Rank_Signed = 1,     // 001B
      Rank_Float = 3,      // 011B
      Rank_Unsigned64 = 4, // 100B
      Rank_Signed64 = 5,   // 101B
      Rank_Double = 7,     // 111B

      Rank_F_LongLong = 4, // 100B 标记为64位类型
      Rank_Undefined = -1, // 未定义
      Rank_BadValue = -2, // 计算异常
      Rank_First = Rank_Unsigned, // 第一个
      Rank_Last = Rank_Double,   // 最后一个
    };
    union {
      GXUINT   uValue;
      GXINT    nValue;
      float    fValue;
      GXUINT64 uValue64;
      GXINT64  nValue64;
      double   fValue64;
    };
    Rank rank;

    VALUE() {}
    VALUE(const TOKEN& token) {
      set(token);
    }

    void clear();
    VALUE& SetZero();
    VALUE& SetOne();
    State set(TOKEN::T_LPCSTR ptr, size_t count, b32 bInteger);
    State set(const TOKEN& token);
    void set(Rank r, const void* pValue);
    VALUE& set(const VALUE& v);
    State UpgradeValueByRank(Rank _type);  // 调整为到 type 指定的级别, 对于浮点与整数类型会做类型转换
    State CastValueByRank(Rank _type); // 按照rank转换值，这个可以指定更低的rank
    State Calculate(const TOKEN& token, const VALUE& param0, const VALUE& param1);
    clStringA ToString() const;

    template<typename _Ty>
    State CalculateT(_Ty& output, const TOKEN& opcode, const _Ty& t1, const _Ty& t2);
  };

  typedef clvector<VALUE> ValuePool;

  //////////////////////////////////////////////////////////////////////////
  struct SYNTAXNODE
  {
    typedef clist<SYNTAXNODE*> PtrList;

    enum FLAGS
    {
      FLAG_OPERAND_MAGIC = 0xffff,
      FLAG_OPERAND_MAGIC_REPLACED = 0xfffe,
      FLAG_OPERAND_UNDEFINED = 0,
      //FLAG_OPERAND_IS_NODE,
      //FLAG_OPERAND_IS_TOKEN,
    };

    enum MODE
    {
      MODE_Undefined,
      MODE_Opcode,              // 操作符 + 操作数 模式: A (操作符) B
      MODE_CommaList,           // 逗号列表, 增加这个是因为整理初始化列表时找逗号token做pOpcode比较麻烦
      MODE_Assignment,          // 初始化列表赋值: A=B; B应该是MODE_InitList, 其它形式赋值属于MODE_Opcode
      MODE_InitList,            // 数据初始化列表, {A}; B永远为空, Opcode 应该为‘{’或者为NULL
      MODE_FunctionCall,        // 函数调用: A(B)
      MODE_TypeCast,            // 类型转换: (A)B
      MODE_Typedef,             // typedef A B;
      MODE_Subscript,           // 下标: A[B], 方括号内有内容的都认为是下标
      MODE_Subscript0,          // 自适应下标: A[], B永远为空 // TODO: Dump时希望精简分支，不想处理这个分支，准备去掉这个或者修改机制
      MODE_Definition,          // 变量定义: A B
      MODE_Bracket,             // 空括号"()", "[]", 如果不是独立出现, 通常会被解释为函数和下标

      MODE_StructDef,           // 结构定义:
      MODE_Flow_While,          // while(A) {B}
      MODE_Flow_If,             // if(A) {B}
      MODE_Flow_ElseIf,         // A else B, A肯定是MODE_Flow_If
      MODE_Flow_Else,           // A else {B;}, A肯定是MODE_Flow_If
      MODE_Flow_For,            // for(A) {B} [[MODE_Flow_ForInit] [MODE_Flow_ForRunning]] [statement block]
      MODE_Flow_ForInit,        // for 的初始化部分 [MODE_Flow_ForInit] [MODE_Flow_ForRunning]
      MODE_Flow_ForRunning,     // for 的条件和步进部分

      MODE_Flow_DoWhile,        // do{B}while(A)
      MODE_Flow_Break,
      MODE_Flow_Continue,
      MODE_Flow_Case,           // 只检查,不支持
      MODE_Flow_Discard,
      MODE_Return,              // A="return" B
      MODE_Block,               // {A}B, B只可能是';'
      MODE_Chain,               // 表达式链表,链表中的应该属于同一个作用域, [A:statement][B:next chain]，在chain结尾应该是[A:statement][B:NULL]这样
    };

    const static int s_NumOfOperand = 2;

    MODE    mode : 16;
    GXDWORD magic : 16; // 0xffff, 0xfffe
    const TOKEN* pOpcode;
#ifdef ENABLE_SYNTAX_NODE_ID
    size_t  id;
#endif

    union GLOB {
      void*         ptr;    // 任意类型，在只是判断UN是否有效时用具体类型可能会产生误解，所以定义了通用类型
      SYNTAXNODE*   pNode;
      const TOKEN*  pTokn;

      GXBOOL IsToken() const;
      GXBOOL IsNode() const;
      GXBOOL IsReplacedNode() const;

      //FLAGS GetType() const;

      GXBOOL CompareAsToken(TOKEN::T_LPCSTR str) const; // 以token方式比较，如果不是token，则返回FALSE
      GXBOOL CompareAsToken(TOKEN::TChar c) const;
      GXBOOL CompareAsNode(MODE _mode) const;

      GLOB& operator=(const TOKEN& token) {
        pTokn = &token;
        return *this;
      }

      const TOKEN* GetFrontToken() const;
      const TOKEN* GetBackToken() const;
      clStringA& ToString(clStringA& str) const;
    };
    typedef clist<GLOB> GlobList;
    typedef clist<GLOB*> GlobPtrList;

    GLOB Operand[s_NumOfOperand];

    void Clear();

    template<class _Func, class _Ty>
    static GXBOOL RecursiveNode2(ArithmeticExpression* pParser, const SYNTAXNODE* pNode, _Ty& rOut, _Func func) // 深度优先递归
    {
      _Ty param[2];
      for(int i = 0; i < 2; i++)
      {
        const FLAGS f = pNode->GetType(i);
        GXBOOL bret = TRUE;
        if(f == FLAG_OPERAND_IS_NODE) {
          bret = RecursiveNode2(pParser, pNode->Operand[i].pNode, param[i], func);
        }
        else if(f == FLAG_OPERAND_IS_TOKEN) {
          bret = param[i].OnToken(pNode, i);
        }

        if(_CL_NOT_(bret)) {
          return bret;
        }
      }

      return func(rOut, pNode, param);
    }

    b32 CompareOpcode(CTokens::TChar ch) const;
    b32 CompareOpcode(CTokens::T_LPCSTR str) const;
    clStringA& ToString(clStringA& str) const;
    //clStringW& ToString(clStringW& str) const;

    //VALUE::State Calcuate(const NameSet& sNameSet, VALUE& value_out, std::function<GXBOOL(const SYNTAXNODE*)> func) const;
    //void GetSourceScope(TKSCOPE& scope) const; // 获得源代码的区间，如果SYNTAXNODE中的节点被替换过
    const TOKEN& GetAnyTokenAB() const;   // 为错误处理找到一个定位的token 顺序:operand[0], operand[1]
    const TOKEN& GetAnyTokenAB2() const;   // 为错误处理找到一个定位的token 顺序:operand[0], operand[1]
    const TOKEN& GetAnyTokenAPB() const;  // 为错误处理找到一个定位的token 顺序:operand[0], opcode, operand[1]
    const TOKEN& GetAnyTokenBPA() const;
    const TOKEN& GetAnyTokenPAB() const; // 为错误处理找到一个定位的token 顺序:opcode, operand[0], operand[1]
  }; // struct SYNTAXNODE

  //////////////////////////////////////////////////////////////////////////

  class CLogger
  {
    typedef SYNTAXNODE::GLOB  GLOB;
    typedef Marimo::DataPoolErrorMsg<ch> ErrorMessage;
    typedef CTokens::T_LPCSTR T_LPCSTR;
    static const int c_nMaxErrorCount = 100;
    static const int c_nMaxSessionError = 4;
    static const int c_nErrorIdLimit = 9000; // 低于这个id的消息受显示数量限制，大于等于这个id的不受限制

    GXUINT              m_uRefCount;
    ErrorMessage*       m_pMsg;

    clset<int>          m_errorlist; // 错误列表, 如果不为空表示解析失败
    size_t              m_nErrorCount;
    int                 m_nSessionError;    // 区间错误数量，如果在区间内大于一定值则不会再输出错误

  public:
    CLogger();
    virtual ~CLogger();

    GXUINT AddRef();
    GXUINT Release();

    void Initialize(const char* szExpression, clsize nSize, GXLPCWSTR szFilename);

    void Reset();
    void ResetSessionError(); // 重置区间错误计数

    void      SetCurrentFilenameW(GXLPCWSTR szFilename);
    GXLPCWSTR GetFilenameW(GXUINT idFile = 0) const; // 获得文件名，这个可以被SetCurrentFilenameW重新设置
    GXLPCWSTR GetFilePathW(GXUINT idFile = 0) const;  // 获得文件路径，这个是PushFile初始设置的文件路径，应该对应一个有效的物理路径
    void      SetLine(T_LPCSTR ptr, GXINT nLine);
    void      PushFile(GXLPCWSTR szFilename, GXINT nTopLine, T_LPCSTR szCodes, size_t length);
    void      PopFile();
    GXINT     GetLine(const TOKEN& token);


    void VarOutputErrorW(const TOKEN* pLocation, GXUINT code, va_list arglist) const;
    //void OutputErrorW(GXUINT code, ...);  // 从最后一个有效token寻找行号
    void OutputErrorW(const GLOB& glob, GXUINT code, ...) const;
    void OutputErrorW(const SYNTAXNODE* pNode, GXUINT code, ...) const;
    void OutputErrorW(const TOKEN& token, GXUINT code, ...) const;
    void OutputErrorW(const TOKEN* pToken, GXUINT code, ...) const;
    void OutputErrorW(T_LPCSTR ptr, GXUINT code, ...);
    void WriteMessageW(GXLPCWSTR szMessage);

    void OutputMissingSemicolon(const TOKEN* ptkLocation); // 输出缺少分号的提示

    GXUINT MarkCode(GXUINT code, GXLPCSTR szMessage);
    int    SetError(int err);
    GXBOOL HasError(int errcode) const;
    size_t ErrorCount() const;
  };

  //////////////////////////////////////////////////////////////////////////

  class ArithmeticExpression : public CTokens
  {
    //friend struct TOKEN;
  public:
    typedef cllist<iterator> IterList;
    typedef CTokens::T_LPCSTR T_LPCSTR;
    typedef CTokens::TChar    TChar;
    typedef SYNTAXNODE::GLOB  GLOB;
    //static const int c_nMaxSessionError = 4;


    //typedef clvector<TOKEN> TokenArray;
    //typedef cllist<TOKEN> TokenList;

#ifdef USE_CLSTD_TOKENS
    enum {
      M_CALLBACK = 1,
    };
#endif // USE_CLSTD_TOKENS

    struct iterator : public CTokens::iterator
    {
      // CTokens::iterator用于在文本流中步进，tk用于处理信息，比如宏替换或者合并字符串
      // 所以CTokens::iterator.marker与tk.marker不一定完全相等
      //typedef clStringA _TStr;
      TOKEN tk;

      iterator();
      iterator(const TOKEN& token);

      iterator& operator++();
      TOKEN* operator->();
      TOKEN& operator*();

      iterator& operator=(const TOKEN& token);
      //b32  operator==(const _TStr& str) const;
      //b32  operator==(T_LPCSTR pStr) const;
      //b32  operator==(TChar ch) const;            // 如果iterator是多字节将返回FALSE
      //b32  operator!=(const _TStr& str) const;
      //b32  operator!=(T_LPCSTR pStr) const;
      //b32  operator!=(TChar ch) const;
      //b32  operator==(const iterator& it) const;
      //b32  operator!=(const iterator& it) const;
      //b32  operator>=(const iterator& it) const;
      //b32  operator<=(const iterator& it) const;


    //  //TOKEN::T_LPCSTR begin() const;
    //  //TOKEN::T_LPCSTR end() const;
    };


    struct MBO // 静态定义符号属性用的结构体
    {
      clsize nLen;
      char* szOperator;
      int precedence; // 优先级，越大越高

      u32 unary : 1; // 参考 TOKEN 机构体说明
      u32 unary_mask : 2;
    };

    struct PAIRMARK
    {
      GXCHAR    chOpen;         // 开区间
      GXCHAR    chClosed;       // 闭区间
      //GXUINT    bNewEOE : 1;  // 更新End Of Expression的位置
      GXUINT    bCloseAE : 1;  // AE = Another Explanation, 闭区间符号有另外解释，主要是"...?...:..."操作符
    };

    //////////////////////////////////////////////////////////////////////////



    //////////////////////////////////////////////////////////////////////////


    typedef clstack<int>          PairStack;
    


    struct TKSCOPE // 运行时的Token范围描述结构体
    {
      typedef cllist<TKSCOPE> List;
      typedef clvector<TKSCOPE> Array;
      typedef clsize TYPE;
      const static TYPE npos = -1;
      TYPE begin;
      TYPE end; // 解析数学规则: [begin, end)

      TKSCOPE(){}
      TKSCOPE(clsize _begin, clsize _end) : begin(_begin), end(_end) {}

      inline GXBOOL IsValid() const {
        return (begin != (clsize)-1) && begin < end;
      }

      inline TYPE GetSize() const
      {
        ASSERT(begin <= end);
        return (end - begin);
      }
    };

    struct INTRINSIC_TYPE // 内置类型
    {
      GXLPCSTR name;
      clsize   name_len;
      int      R;         // 最大允许值
      int      C;         // 最大允许值
    };

    typedef Marimo::DataPoolErrorMsg<ch> ErrorMessage;
  protected:
    //static INTRINSIC_TYPE s_aIntrinsicType[];
    static MBO s_semantic;
    static MBO s_plus_minus[];
    static MBO s_Operator1[];
    static MBO s_Operator2[];
    static MBO s_UnaryLeftOperand[];
    static MBO s_Operator3[];
    static PAIRMARK s_PairMark[4];
    static const int s_MaxPrecedence = OPP(13);
    //static const int s_nPairMark = sizeof(s_PairMark) / sizeof(PAIRMARK);
#ifdef ENABLE_SYNTAX_NODE_ID
    clsize              m_nNodeId;
#endif
    GXBOOL              m_bHigherDefiniton; // “Identifier Identifier” 形式的定义转为更高优先级，默认比较低
    //GXBOOL              m_bRefMsg; // 如果为TRUE，析构时不删除m_pMsg
    //ErrorMessage*       m_pMsg;
    TOKEN::Array        m_aTokens;
    
    CLogger*             m_pLogger;   
    //clset<int>          m_errorlist; // 错误列表, 如果不为空表示解析失败
    //size_t              m_nErrorCount;
    //int                 m_nSessionError;    // 区间错误数量，如果在区间内大于一定值则不会再输出错误

    // 语法节点的内存池
    //SyntaxNodePoolList  m_NodePoolList;
    //SYNTAXNODE*         m_pNewNode;           // 记录当前分配节点的指针
    clstd::StablePool<SYNTAXNODE> m_NodePool;

    clStringArrayA      m_aDbgExpressionOperStack;

#ifdef USE_CLSTD_TOKENS
    static u32 m_aCharSem[128];
#endif // USE_CLSTD_TOKENS
  protected:
    static b32 TryExtendNumeric         (iterator& it, clsize remain);
    virtual u32     StepIterator         (ArithmeticExpression::iterator& it);
    u32     MultiByteOperatorProc(ArithmeticExpression::iterator& it, u32 nRemain);

    ArithmeticExpression::iterator begin();
    ArithmeticExpression::iterator end();

    void    InitTokenScope(TKSCOPE& scope, const TOKEN& token, b32 bHasBracket) const;
    void    InitTokenScope(TKSCOPE& scope, GXUINT index, b32 bHasBracket) const;
    GXBOOL  MarryBracket(PairStack* sStack, TOKEN& token, GXBOOL bSilent);
    GXBOOL  IsArrayList(const TOKEN& token);
    GXBOOL  MakeSyntaxNode(GLOB* pDest, SYNTAXNODE::MODE mode, const TOKEN* pOpcode, GLOB* pOperandA, GLOB* pOperandB);
    GXBOOL  MakeSyntaxNode(GLOB* pDest, SYNTAXNODE::MODE mode, GLOB* pOperandA, GLOB* pOperandB);
    GXBOOL  MakeInstruction(int depth, const TOKEN* pOpcode, int nMinPrecedence, const TKSCOPE* pScope, GLOB* pParent, int nMiddle); // nMiddle是把RTSCOPE分成两个RTSCOPE的那个索引
    GXBOOL  IsLikeTypeCast(const TKSCOPE& scope, TKSCOPE::TYPE i);

    GXBOOL  ParseFunctionCall(const TKSCOPE& scope, GLOB* pDesc);
    GXBOOL  ParseTypeCast(const TKSCOPE& scope, GLOB* pDesc);
    GXBOOL  ParseFunctionSubscriptCall(const TKSCOPE& scope, GLOB* pDesc);

    GXBOOL  CompareToken(int index, TOKEN::T_LPCSTR szName); // 带容错的
    TKSCOPE::TYPE  GetLowestPrecedence(const TKSCOPE& scope, int nMinPrecedence);
    TKSCOPE::TYPE FindComma(const TKSCOPE& scope);
    void    EnableHigherDefinition(GXBOOL bHigher);
    SYNTAXNODE* AllocNode(SYNTAXNODE::MODE m, void* pOperand0, void* pOperand1);

  public:
    static TChar GetPairOfBracket(TChar ch); // 获得与输入配对的括号

  public:
    ArithmeticExpression();
    virtual ~ArithmeticExpression();

    const SYNTAXNODE* TryGetNode        (const GLOB* pDesc) const; // TODO: 修改所属类
    SYNTAXNODE::MODE  TryGetNodeMode    (const GLOB* pDesc) const; // TODO: 修改所属类

    clsize              EstimateForTokensCount  () const;   // 从Stream的字符数估计Token的数量
    //clsize              GenerateTokens          ();
    const TOKEN::Array* GetTokensArray          () const;

    GXBOOL  ParseArithmeticExpression(int depth, const TKSCOPE& scope, GLOB* pDesc);
    GXBOOL  ParseArithmeticExpression(int depth, const TKSCOPE& scope, GLOB* pDesc, int nMinPrecedence); // 递归函数

    int BreakComma(int depth, const TKSCOPE& scope, GLOB* pDesc, int nMinPrecedence); // 返回值：-1，没处理；0，失败；1，成功
    CLogger* GetLogger();
    const CLogger* GetLogger() const;

    GXBOOL DbgHasError(int errcode) const;
    size_t DbgErrorCount() const;
    void DbgDumpScope(clStringA& str, const TKSCOPE& scope);
    void DbgDumpScope(clStringA& str, clsize begin, clsize end, GXBOOL bRaw);
    void DbgDumpScope(GXLPCSTR opcode, const TKSCOPE& scopeA, const TKSCOPE& scopeB);
    void Invoke(GXLPCSTR szFunc, GXLPCSTR szArguments);
    clStringArrayA& DbgGetExpressionStack();
    virtual T_LPCSTR GetOriginPtr(const TOKEN* pToken) const; // 获得token原始地址指针
  };

  template<class SYNTAXNODE_T>
  void RecursiveNode(ArithmeticExpression* pParser, SYNTAXNODE_T* pNode, std::function<GXBOOL(SYNTAXNODE_T*, int)> func, int depth = 0); // 广度优先递归

} // namespace UVShader

namespace stdext
{
  inline size_t hash_value(const UVShader::TOKEN& _token)
  {
    u32 _Val = 2166136261U;

    auto* pBegin = _token.marker;
    auto* pEnd   = _token.end();
    while (pBegin != pEnd) {
      _Val = 16777619U * _Val ^ (u32)*pBegin++;
    }
    return (_Val);
  }

  inline size_t hash_value(const UVShader::TOKEN* _token)
  {
    return hash_value(*_token);
  }
}

// gcc stlport
namespace std
{
  template<> struct hash<UVShader::TOKEN>
  {
    size_t operator()(const UVShader::TOKEN& _token) const { return stdext::hash_value(_token); }
  };

  template<> struct hash<UVShader::TOKEN*>
  {
    size_t operator()(const UVShader::TOKEN* _token) const {
      return stdext::hash_value(_token);
    }
  };
}

#endif // _ARITHMETIC_EXPRESSION_H_