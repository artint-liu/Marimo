#ifndef _ARITHMETIC_EXPRESSION_H_
#define _ARITHMETIC_EXPRESSION_H_

#define IDX2ITER(_IDX)                           m_aTokens[_IDX]

#define UNARY_LEFT_OPERAND    2 // 10B， 注意是允许操作数在左侧
#define UNARY_RIGHT_OPERAND   1 // 01B
#define ENABLE_STRINGED_SYMBOL
#define OPP(_PRE) (ArithmeticExpression::TOKEN::FIRST_OPCODE_PRECEDENCE + _PRE)

#define USE_CLSTD_TOKENS

// 注意UVS_EXPORT_TEXT不能改名, 它在Sample中作为标记符号抽取ErrorMessage
#if defined(UVS_EXPORT_TEXT_IS_SIGN)
#define UVS_EXPORT_TEXT(_CODE_ID, _MESSAGE)  MarkCode(_CODE_ID, _MESSAGE)
#else
#define UVS_EXPORT_TEXT(_CODE_ID, _MESSAGE)  _CODE_ID
#endif

#define ENABLE_SYNTAX_NODE_ID

namespace Marimo
{
  template<typename _TChar>
  class DataPoolErrorMsg;
} // namespace Marimo

namespace UVShader
{
  //class ArithmeticExpression : public SmartStreamA
#ifdef USE_CLSTD_TOKENS
  typedef clstd::TokensT<clStringA> CTokens;
# define TOKENSUTILITY clstd::TokensUtility
#else
  typedef SmartStreamA CTokens;
# define TOKENSUTILITY SmartStreamUtility
#endif // USE_CLSTD_TOKENS

  template<class _Ty>
  class StablePool
  {
  public:
    typedef _Ty T;

    struct POOL
    {
      _Ty* pBegin;
      _Ty* pEnd;
    };

    typedef cllist<POOL> PoolList;

  protected:
    PoolList m_PoolList;
    int      m_nElementCount; // 增长数量，实际分配内存是数量*sizeof(_Ty)
    _Ty*     m_pNew;

  public:
    StablePool(int nCount)
      : m_nElementCount(nCount), m_pNew(NULL) {}

    ~StablePool()
    {
      Clear();
    }

    _Ty* Alloc()
    {
      ASSERT(m_pNew == NULL || (
        m_pNew >= m_PoolList.back().pBegin &&
        m_pNew < m_PoolList.back().pEnd));

      if(m_pNew == NULL || (++m_pNew) == m_PoolList.back().pEnd) {
        POOL pool = { NULL, NULL };
        m_pNew = new _Ty[m_nElementCount];
        pool.pBegin = m_pNew;
        pool.pEnd = pool.pBegin + m_nElementCount;
        m_PoolList.push_back(pool);
      }
      return m_pNew;
    }

    _Ty* PushBack(const _Ty& t) // 把t追加到最后， 并且返回一个稳定地址
    {
      _Ty* ptr = Alloc();
      *ptr = t;
      return ptr;
    }

    void Clear()
    {
      std::for_each(m_PoolList.begin(), m_PoolList.end(),
        [](POOL& pool)
      {
        delete[] pool.pBegin;
      });
      m_PoolList.clear();
      m_pNew = NULL;
    }

  };

  class ArithmeticExpression : public CTokens
  {
  public:
    typedef cllist<iterator> IterList;
    typedef CTokens::T_LPCSTR T_LPCSTR;
    typedef CTokens::TChar    TChar;


    //typedef clvector<TOKEN> TokenArray;
    //typedef cllist<TOKEN> TokenList;

    // 运行时记录符号和操作符等属性
    struct TOKEN : CTokens::iterator
    {
      typedef cllist<TOKEN>   List;
      typedef clvector<TOKEN> Array;
      typedef CTokens::T_LPCSTR T_LPCSTR;
      enum Type
      {
        TokenType_Undefine = 0,
        TokenType_String = 1,
        TokenType_FormalParams = 2,  // 宏的形参
        TokenType_FirstNumeric = 5,
        TokenType_Numeric = TokenType_FirstNumeric,
        TokenType_LastNumeric,
        TokenType_Name = 20,
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
      int       scope;                          // 括号匹配索引
      int       semi_scope : scope_bits;      // 分号作用域
      int       precedence : precedence_bits; // 符号优先级
      u32       unary : 1;               // 是否为一元操作符
      u32       unary_mask : 2;               // 一元操作符允许位：11B表示操作数可以在左边和右边，01B表示操作数只能在右边，10B表示操作数只能在左边

      Type      type : 8;               // 类型
      u32       bInStringSet : 1;               // 在String字典而不再流中
                                                // [precedence]
                                                // 1~14 表示运算符的优先级
                                                // 15 表示"{","}","(",")","[","]"这些之一，此时scope数据是相互对应的，例如
                                                // array[open].scope = closed && array[closed].scope = open

      void ClearMarker();
      void Set(const iterator& _iter);
      void Set(clstd::StringSetA& sStrSet, const clStringA& str); // 设置外来字符串

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

      GXBOOL operator==(const TOKEN& t) const;
      GXBOOL operator==(SmartStreamA::T_LPCSTR str) const;
      GXBOOL operator==(SmartStreamA::TChar ch) const;
      GXBOOL operator!=(SmartStreamA::T_LPCSTR str) const;
      GXBOOL operator!=(SmartStreamA::TChar ch) const;
      b32    operator<(const TOKEN& _token) const;

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

      //template<class _List, class _Iter>
      //static void Append(_List& tokens, int offset, const _Iter& begin, const _Iter& end)
      //{
      //  Append(tokens, offset, begin, end, [](int, const TOKEN&){});
      //}

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

#ifdef USE_CLSTD_TOKENS
    enum {
      M_CALLBACK = 1,
    };
#endif // USE_CLSTD_TOKENS

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
      GXUINT    bNewEOE : 1;  // 更新End Of Expression的位置
      GXUINT    bCloseAE : 1;  // AE = Another Explanation, 闭区间符号有另外解释，主要是"...?...:..."操作符
    };

    struct SYNTAXNODE
    {
      typedef clist<SYNTAXNODE*> PtrList;

      enum FLAGS
      {
        FLAG_OPERAND_MAGIC = 0xffff,
        FLAG_OPERAND_UNDEFINED = 0,
        FLAG_OPERAND_IS_NODE,
        FLAG_OPERAND_IS_TOKEN,
      };

      enum MODE
      {
        MODE_Undefined,
        MODE_Opcode,              // 操作符 + 操作数 模式: A (操作符) B
        MODE_ArrayAssignment,     // 数组赋值: A={B}; 其它形式赋值属于MODE_Opcode
        MODE_FunctionCall,        // 函数调用: A(B)
        MODE_TypeConversion,      // 类型转换: (A)B
        MODE_ArrayIndex,          // 索引调用: A[B], 方括号内有内容的都认为是数组调用
        MODE_ArrayAlloc,          // 自适应数组分配: A[], 注意"k[5]"这种形式认为是索引调用
        MODE_Definition,          // 变量定义: A B
        //MODE_DefinitionConst,     // 常量量定义: const A B
        MODE_StructDef,           // 结构定义:
        MODE_Flow_While,          // while(A) {B}
        MODE_Flow_If,             // if(A) {B}
        MODE_Flow_ElseIf,
        MODE_Flow_Else,
        MODE_Flow_For,            // for(A) {B} [[MODE_Flow_ForInit] [MODE_Flow_ForRunning]] [statement block]
        MODE_Flow_ForInit,        // for 的初始化部分 [MODE_Flow_ForInit] [MODE_Flow_ForRunning]
        MODE_Flow_ForRunning,     // for 的条件和步进部分
        //MODE_Flow_Switch,
        MODE_Flow_DoWhile,        // do{B}while(A)
        MODE_Flow_Break,
        MODE_Flow_Continue,
        MODE_Flow_Discard,
        MODE_Return,
        MODE_Block,         // {A}B, B只可能是';'
        MODE_Chain,         // 表达式链表,链表中的应该属于同一个作用域, [A:statement][B:next chain]，在chain结尾应该是[A:statement][B:NULL]这样
      };

      const static int s_NumOfOperand = 2;

      MODE    mode  : 16;
      GXDWORD magic : 16; // 0xffff
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
        FLAGS GetType() const;

        GLOB& operator=(const TOKEN& token) {
          pTokn = &token;
          return *this;
        }
      };

      //struct DESC {
      //  GLOB    un;
      //  DESC& operator=(const TOKEN& token) {
      //    un.pTokn = &token;
      //    return *this;
      //  }
      //};
      //typedef clist<GLOB> UNList;

      GLOB Operand[s_NumOfOperand];


      void Clear();

      static void RecursiveNode(ArithmeticExpression* pParser, SYNTAXNODE* pNode, std::function<GXBOOL(SYNTAXNODE*, int)> func, int depth = 0); // 广度优先递归

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

      b32 CompareOpcode(TChar ch) const;
      b32 CompareOpcode(T_LPCSTR str) const;

      const TOKEN& GetAnyTokenAB() const;   // 为错误处理找到一个定位的token 顺序:operand[0], operand[1]
      const TOKEN& GetAnyTokenAPB() const;  // 为错误处理找到一个定位的token 顺序:operand[0], opcode, operand[1]
      const TOKEN& GetAnyTokenPAB() const; // 为错误处理找到一个定位的token 顺序:opcode, operand[0], operand[1]
    }; // struct SYNTAXNODE

    typedef clstack<int>          PairStack;
    
    //struct SYNTAXNODEPOOL
    //{
    //  SYNTAXNODE* pBegin;
    //  SYNTAXNODE* pEnd;
    //};

    //typedef cllist<SYNTAXNODEPOOL> SyntaxNodePoolList;



    struct VALUE
    {
      enum State {
        State_OK = 0,
        State_SyntaxError = -1,
        State_Overflow = -2,
        State_IllegalChar = -3,
      };
      enum Rank {
        // 这些值由特殊用法不能轻易修改
        Rank_Unsigned   = 0, // 000B
        Rank_Signed     = 1, // 001B
        Rank_float      = 3, // 011B
        Rank_Unsigned64 = 4, // 100B
        Rank_Signed64   = 5, // 101B
        Rank_Double     = 7, // 111B

        Rank_F_LongLong = 4, // 100B 标记为64位类型
        Rank_Undefined   = -1, // 未定义
        Rank_BadValue   = -2, // 计算异常
        Rank_First = Rank_Unsigned, // 第一个
        Rank_Last  = Rank_Double,   // 最后一个
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

      VALUE(){}
      VALUE(const TOKEN& token) { set(token); }

      void clear();
      void SetZero();
      void SetOne();
      State set(const TOKEN& token);
      VALUE& set(const VALUE& v);
      State SyncRank(Rank _type);  // 调整为到 type 指定的级别
      State Calculate(const TOKEN& token, const VALUE& param0, const VALUE& param1);
      clStringA ToString() const;

      template<typename _Ty>
      typename _Ty CalculateT(const TOKEN& opcode, _Ty& t1, _Ty& t2);
    };

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
    static INTRINSIC_TYPE s_aIntrinsicType[];
    static MBO s_semantic;
    static MBO s_plus_minus[];
    static MBO s_Operator1[];
    static MBO s_Operator2[];
    static MBO s_Operator3[];
    static PAIRMARK s_PairMark[4];
    static const int s_MaxPrecedence = 14;
    //static const int s_nPairMark = sizeof(s_PairMark) / sizeof(PAIRMARK);
#ifdef ENABLE_SYNTAX_NODE_ID
    clsize              m_nNodeId;
#endif

    GXBOOL              m_bRefMsg; // 如果为TRUE，析构时不删除m_pMsg
    ErrorMessage*       m_pMsg;
    TOKEN::Array        m_aTokens;
    //int                 m_nMaxPrecedence;   // 优先级最大值

    // 语法节点的内存池
    //SyntaxNodePoolList  m_NodePoolList;
    //SYNTAXNODE*         m_pNewNode;           // 记录当前分配节点的指针
    StablePool<SYNTAXNODE> m_NodePool;

    int                 m_nDbgNumOfExpressionParse; // 调试模式用于记录解析表达式迭代次数的变量
    clStringArrayA      m_aDbgExpressionOperStack;

#ifdef USE_CLSTD_TOKENS
    static u32 m_aCharSem[128];
#endif // USE_CLSTD_TOKENS
  protected:
    static u32 CALLBACK IteratorProc         (iterator& it, u32 nRemain, u32_ptr lParam);
    static u32 CALLBACK MultiByteOperatorProc(iterator& it, u32 nRemain, u32_ptr lParam);

    void    InitTokenScope(TKSCOPE& scope, const TOKEN& token, b32 bHasBracket) const;
    void    InitTokenScope(TKSCOPE& scope, GXUINT index, b32 bHasBracket) const;
    GXBOOL  MarryBracket(PairStack* sStack, TOKEN& token);
    GXBOOL  MakeSyntaxNode(SYNTAXNODE::GLOB* pDest, SYNTAXNODE::MODE mode, const TOKEN* pOpcode, SYNTAXNODE::GLOB* pOperandA, SYNTAXNODE::GLOB* pOperandB);
    GXBOOL  MakeSyntaxNode(SYNTAXNODE::GLOB* pDest, SYNTAXNODE::MODE mode, SYNTAXNODE::GLOB* pOperandA, SYNTAXNODE::GLOB* pOperandB);
    GXBOOL  MakeInstruction(int depth, const TOKEN* pOpcode, int nMinPrecedence, const TKSCOPE* pScope, SYNTAXNODE::GLOB* pParent, int nMiddle); // nMiddle是把RTSCOPE分成两个RTSCOPE的那个索引

    GXBOOL  ParseFunctionCall(const TKSCOPE& scope, SYNTAXNODE::GLOB* pDesc);
    GXBOOL  ParseTypeCast(const TKSCOPE& scope, SYNTAXNODE::GLOB* pDesc);
    GXBOOL  ParseFunctionIndexCall(const TKSCOPE& scope, SYNTAXNODE::GLOB* pDesc);

    GXBOOL  CompareToken(int index, TOKEN::T_LPCSTR szName); // 带容错的

#if defined(UVS_EXPORT_TEXT_IS_SIGN)
    GXUINT  MarkCode(GXUINT code, GXLPCSTR szMessage);
#endif
  public:
    static TChar GetPairOfBracket(TChar ch); // 获得与输入配对的括号

  public:
    ArithmeticExpression();
    virtual ~ArithmeticExpression();

    //SYNTAXNODE::FLAGS TryGetNodeType(const SYNTAXNODE::UN* pUnion) const; // TODO: 修改所属类
    //SYNTAXNODE::MODE  TryGetNode    (const SYNTAXNODE::UN* pUnion) const; // TODO: 修改所属类
    const SYNTAXNODE* TryGetNode        (const SYNTAXNODE::GLOB* pDesc) const; // TODO: 修改所属类
    SYNTAXNODE::MODE  TryGetNodeMode    (const SYNTAXNODE::GLOB* pDesc) const; // TODO: 修改所属类

    clsize              EstimateForTokensCount  () const;   // 从Stream的字符数估计Token的数量
    //clsize              GenerateTokens          ();
    const TOKEN::Array* GetTokensArray          () const;

    GXBOOL  ParseArithmeticExpression(int depth, clsize begin, clsize end, SYNTAXNODE::GLOB* pDesc);
    GXBOOL  ParseArithmeticExpression(int depth, const TKSCOPE& scope, SYNTAXNODE::GLOB* pDesc);
    GXBOOL  ParseArithmeticExpression(int depth, const TKSCOPE& scope, SYNTAXNODE::GLOB* pDesc, int nMinPrecedence); // 递归函数

    void DbgDumpScope(clStringA& str, const TKSCOPE& scope);
    void DbgDumpScope(clStringA& str, clsize begin, clsize end, GXBOOL bRaw);
    void DbgDumpScope(GXLPCSTR opcode, const TKSCOPE& scopeA, const TKSCOPE& scopeB);
    const clStringArrayA& DbgGetExpressionStack() const;

  };


} // namespace UVShader

namespace stdext
{
  inline size_t hash_value(const UVShader::ArithmeticExpression::TOKEN& _token)
  {
    u32 _Val = 2166136261U;

    auto* pBegin = _token.marker;
    auto* pEnd   = _token.end();
    while (pBegin != pEnd) {
      _Val = 16777619U * _Val ^ (u32)*pBegin++;
    }
    return (_Val);
  }
}

// gcc stlport
namespace std
{
  template<> struct hash<UVShader::ArithmeticExpression::TOKEN>
  {
    size_t operator()(const UVShader::ArithmeticExpression::TOKEN& _token) const { return stdext::hash_value(_token); }
  };
}

#endif // _ARITHMETIC_EXPRESSION_H_