#ifndef _EXPRESSION_PARSER_H_
#define _EXPRESSION_PARSER_H_

namespace Marimo
{
  template<typename _TChar>
  class DataPoolErrorMsg;
} // namespace Marimo

namespace UVShader
{
  //struct GRAMMAR
  //{
  //  enum Modified
  //  {
  //    Modified_optional,
  //    Modified_required,
  //  };

  //  Modified eModified;
  //  GXLPCSTR szMarker;

  //  GXLPVOID pDest;
  //  GXSIZE_T offset;

  //  GRAMMAR* pChild;
  //};
#define OPP(_PRE) (CodeParser::TOKEN::FIRST_OPCODE_PRECEDENCE + _PRE)
#define UNARY_LEFT_OPERAND    2 // 10B， 注意是允许操作数在左侧
#define UNARY_RIGHT_OPERAND   1 // 01B

#define ENABLE_STRINGED_SYMBOL
  class CodeParser : public SmartStreamA
  {
  public:
    struct TOKEN // 运行时记录符号和操作符等属性
    {
      const static int scope_bits = 22;
      const static int precedence_bits = 7;
      const static int FIRST_OPCODE_PRECEDENCE = 1;
      const static int ID_BRACE = 15;
#ifdef ENABLE_STRINGED_SYMBOL
      clStringA symbol;
#endif // #ifdef ENABLE_STRINGED_SYMBOL
      iterator  marker;
      int       scope;                        // 括号匹配索引
      int       semi_scope : scope_bits;      // 分号作用域
      int       precedence : precedence_bits; // 符号优先级
      u32       unary      : 1;               // 是否为一元操作符
      u32       unary_mask : 2;               // 一元操作符允许位：11B表示操作数可以在左边和右边，01B表示操作数只能在右边，10B表示操作数只能在左边
      // [precedence]
      // 1~14 表示运算符的优先级
      // 15 表示"{","}","(",")","[","]"这些之一，此时scope数据是相互对应的，例如
      // array[open].scope = closed && array[closed].scope = open

      void ClearMarker()
      {
#ifdef ENABLE_STRINGED_SYMBOL
        symbol.Clear();
#endif // #ifdef ENABLE_STRINGED_SYMBOL
        marker.marker = 0;
      }

      void Set(const iterator& _iter)
      {
#ifdef ENABLE_STRINGED_SYMBOL
        symbol = _iter.ToString();
#endif // #ifdef ENABLE_STRINGED_SYMBOL
        marker = _iter;
      }

      template<class _Ty>
      void SetArithOperatorInfo(const _Ty& t) // 设置数学符号信息
      {
        unary      = t.unary;
        unary_mask = t.unary_mask;
        precedence = t.precedence;
      }

      void ClearArithOperatorInfo()
      {
        unary      = 0;
        unary_mask = 0;
        precedence = 0;
      }

      clStringA ToString() const
      {
        return marker.ToString();
      }

      int GetScope() const
      {
        return scope >= 0 ? scope : semi_scope;
      }

      GXBOOL operator==(SmartStreamA::T_LPCSTR str) const
      {
        return (marker == str);
      }

      GXBOOL operator==(SmartStreamA::TChar ch) const
      {
        return (marker == ch);
      }

      GXBOOL operator!=(SmartStreamA::T_LPCSTR str) const
      {
        return (marker != str);
      }

      GXBOOL operator!=(SmartStreamA::TChar ch) const
      {
        return (marker != ch);
      }
    };
        
    struct MBO // 静态定义符号属性用的结构体
    {
      clsize nLen;
      char* szOperator;
      int precedence; // 优先级，越大越高

      u32 unary      : 1; // 参考 TOKEN 机构体说明
      u32 unary_mask : 2;
    };


    typedef clvector<TOKEN> TokenArray;

    enum StatementType
    {
      StatementType_Empty,
      StatementType_Definition,     // 变量定义
      StatementType_FunctionDecl,   // 函数声明
      StatementType_Function,       // 函数体
      StatementType_Struct,         // 结构体
      StatementType_Signatures,     // 用于shader输入输出标记的结构体
      StatementType_Expression,     // 表达式
    };

    enum StorageClass // 函数修饰，可选
    {
      StorageClass_empty,
      StorageClass_inline,
    };

    enum InputModifier // 函数参数修饰
    {
      InputModifier_in,
      InputModifier_out,
      InputModifier_inout,
      InputModifier_uniform,
    };

    enum UniformModifier
    {
      UniformModifier_empty,
      UniformModifier_const,
    };

    struct TYPE
    {
      GXLPCSTR  name;
      int       R;    // Row, 或者 array size
      int       C;    // Column

      bool operator<(const TYPE& t) const;
    };
    typedef clset<TYPE>   TypeSet;

    struct FUNCTION_ARGUMENT // 函数参数
    {
      InputModifier eModifier;  // [opt]
      GXLPCSTR      szType;     // [req]
      GXLPCSTR      szName;     // [req]
      GXLPCSTR      szSemantic; // [opt]
    };
    typedef clvector<FUNCTION_ARGUMENT> ArgumentsArray;
    
    struct STRUCT_MEMBER // 结构体成员/Shader标记
    {
      GXLPCSTR szType;       // [req]
      GXLPCSTR szName;       // [req]
      GXLPCSTR szSignature;  // [req], 结构体成员没有这项
    };
    typedef clvector<STRUCT_MEMBER> MemberArray;

    struct SYNTAXNODE
    {
      enum FLAGS
      {
        FLAG_OPERAND_SHIFT      = 8,
        FLAG_OPERAND_TYPEMASK   = 0x0000000F,
        FLAG_OPERAND_IS_NODEIDX = 0x00000001,
        FLAG_OPERAND_IS_NODE    = 0x00000002,
        FLAG_OPERAND_IS_TOKEN   = 0x00000004,
      };

      enum MODE
      {
        MODE_Undefined,
        MODE_Opcode,          // 操作符 + 操作数 模式
        MODE_FunctionCall,    // 函数调用
        MODE_ArrayIndex,      // 函数调用
        MODE_Definition,      // 变量定义
        MODE_DefinitionConst, // 常量量定义
        MODE_StructDef,       // 结构定义
        MODE_Flow_While,
        MODE_Flow_If,         // if(A) {B}
        MODE_Flow_ElseIf,
        MODE_Flow_Else,
        MODE_Flow_For,            // [[MODE_Flow_ForInit] [MODE_Flow_ForRunning]] [statement block]
        MODE_Flow_ForInit,        // for 的初始化部分 [MODE_Flow_ForInit] [MODE_Flow_ForRunning]
        MODE_Flow_ForRunning,     // for 的条件和步进部分
        //MODE_Flow_Switch,
        MODE_Flow_DoWhile,
        MODE_Flow_Break,
        MODE_Flow_Continue,
        MODE_Flow_Discard,
        MODE_Return,
        MODE_Block,
        MODE_Chain,         // 表达式链表,链表中的应该属于同一个作用域, [A:statement][B:next chain]，在chain结尾应该是[A:statement][B:NULL]这样
      };

      const static int s_NumOfOperand = 2;

      GXDWORD flags : 16;
      MODE    mode  : 16;
      const TOKEN* pOpcode;

      union UN {
        void*         ptr;    // 任意类型，在只是判断UN是否有效时用具体类型可能会产生误解，所以定义了通用类型
        SYNTAXNODE*   pNode;
        size_t        nNodeIndex;
        const TOKEN*  pSym;
      };

      UN Operand[s_NumOfOperand];

      inline FLAGS GetOperandType(const int index) const {
        const int shift = FLAG_OPERAND_SHIFT * index;
        return (FLAGS)((flags >> shift) & FLAG_OPERAND_TYPEMASK);
      }

      inline void SetOperandType(const int index, FLAGS flag) {
        const int shift = FLAG_OPERAND_SHIFT * index;
        flags = (flags & (~(FLAG_OPERAND_TYPEMASK << shift))) | (flag << shift);
      }

      inline GXBOOL OperandA_IsNodeIndex() const {
        const GXDWORD dwTypeMask = FLAG_OPERAND_TYPEMASK;
        return (flags & dwTypeMask) == FLAG_OPERAND_IS_NODEIDX;
      }

      inline GXBOOL OperandB_IsNodeIndex() const {
        const GXDWORD dwTypeMask = FLAG_OPERAND_TYPEMASK << FLAG_OPERAND_SHIFT;
        const GXDWORD dwNode = FLAG_OPERAND_IS_NODEIDX << FLAG_OPERAND_SHIFT;
        return (flags & dwTypeMask) == dwNode;
      }

      inline void Clear()
      {
        flags  = 0;
        mode   = MODE_Undefined;
        pOpcode = NULL;
        Operand[0].ptr = NULL;
        Operand[1].ptr = NULL;
      }

      template<class _Func>
      static void RecursiveNode(CodeParser* pParser, SYNTAXNODE* pNode, _Func func) // 广度优先递归
      {
        if(func(pNode)) {
          int i = 0;
          do {
            auto type = pNode->GetOperandType(i);
            switch(type)
            {
            case FLAG_OPERAND_IS_NODE:
              RecursiveNode(pParser, pNode->Operand[i].pNode, func);
              break;
            case FLAG_OPERAND_IS_NODEIDX:
              RecursiveNode(pParser, &pParser->
                m_aSyntaxNodePack[pNode->Operand[i].nNodeIndex], func);
              break;
            }
            i++;
          }while(i < 2);
        }
      }

      template<class _Func>
      static void RecursiveNode2(CodeParser* pParser, SYNTAXNODE* pNode, _Func func) // 深度优先递归
      {
        int i = 0;
        do {
          auto type = pNode->GetOperandType(i);
          switch(type)
          {
          case FLAG_OPERAND_IS_NODE:
            RecursiveNode(pParser, pNode->Operand[i].pNode, func);
            break;
          case FLAG_OPERAND_IS_NODEIDX:
            RecursiveNode(pParser, &pParser->
              m_aSyntaxNodePack[pNode->Operand[i].nNodeIndex], func);
            break;
          }
          i++;
        }while(i < 2);
        func(pNode);
      }

    };
    typedef clvector<SYNTAXNODE> SyntaxNodeArray;

    //////////////////////////////////////////////////////////////////////////
    struct STATEMENT;
    struct STATEMENT_EXPR;

    struct STATEMENT_FUNC // 函数体/函数声明
    {
      StorageClass eStorageClass; // [opt]
      GXLPCSTR     szReturnType;  // [req]
      GXLPCSTR     szName;        // [req]
      GXLPCSTR     szSemantic;    // [opt]
      FUNCTION_ARGUMENT*  pArguments;       // [opt]
      clsize              nNumOfArguments;  // [opt]
      STATEMENT* pExpression;
    };

    struct STATEMENT_STRU // 结构体定义
    {
      GXLPCSTR        szName;
      STRUCT_MEMBER*  pMembers;
      clsize          nNumOfMembers;
    };

    struct STATEMENT_EXPR // 表达式定义
    {
      SYNTAXNODE::UN  sRoot;
    };
    
    struct STATEMENT_DEFN
    {
      UniformModifier modifier;
      GXLPCSTR        szType;
      SYNTAXNODE::UN  sRoot;
    };


    struct STATEMENT
    {
      StatementType type;
      union
      {
        STATEMENT_FUNC func;
        STATEMENT_STRU stru;
        STATEMENT_EXPR expr;
        STATEMENT_DEFN defn; // 全局变量定义
      };
    };

    typedef clvector<STATEMENT> StatementArray;

    //////////////////////////////////////////////////////////////////////////

    struct RTSCOPE // 运行时的范围描述结构体
    {
      typedef clsize TYPE;
      const static TYPE npos = -1;
      TYPE begin;
      TYPE end;

      RTSCOPE(){}
      RTSCOPE(clsize _begin, clsize _end) : begin(_begin), end(_end) {}

      inline GXBOOL IsValid() const {
        return (begin != (clsize)-1) && begin < end;
      }
    };

    //struct CONSTRUCT_RTSCOPE : public RTSCOPE
    //{
    //  CONSTRUCT_RTSCOPE(clsize _begin, clsize _end) {
    //    begin = _begin;
    //    end = _end;
    //  }
    //  operator RTSCOPE*()
    //  {
    //    return this;
    //  }
    //};

    struct INTRINSIC_TYPE // 内置类型
    {
      GXLPCSTR name;
      clsize   name_len;
      int      R;         // 最大允许值
      int      C;         // 最大允许值
    };

    struct MAKESCOPE
    {
      const RTSCOPE*  pScope;   // 限定区域
      RTSCOPE::TYPE   begin;    // 开始
      GXBOOL          bBeginMate;// scope begin 取自m_aTokens[begin].scope
      RTSCOPE::TYPE   end;
      GXBOOL          bEndMate;
      GXWCHAR         chTermin; // 如果begin遇到这个终结符，返回一个begin==end的scope

      MAKESCOPE(){}
      MAKESCOPE(const RTSCOPE*_pScope, RTSCOPE::TYPE _begin, GXBOOL _bIndBegin, RTSCOPE::TYPE _end, GXBOOL _bIndEnd, GXWCHAR _chTermin)
        : pScope(_pScope), begin(_begin), bBeginMate(_bIndBegin), end(_end), bEndMate(_bIndEnd), chTermin(_chTermin) {}
      MAKESCOPE(const RTSCOPE& _scope, RTSCOPE::TYPE _begin, GXBOOL _bIndBegin, RTSCOPE::TYPE _end, GXBOOL _bIndEnd, GXWCHAR _chTermin)
        : pScope(&_scope), begin(_begin), bBeginMate(_bIndBegin), end(_end), bEndMate(_bIndEnd), chTermin(_chTermin) {}
    };
    //////////////////////////////////////////////////////////////////////////

  protected:
    static u32 CALLBACK MultiByteOperatorProc(iterator& it, u32 nRemain, u32_ptr lParam);
    static u32 CALLBACK IteratorProc         (iterator& it, u32 nRemain, u32_ptr lParam);

    void    InitPacks();
    void    Cleanup();
    clsize  EstimateForTokensCount () const;  // 从Stream的字符数估计Token的数量

    GXBOOL  ParseStatementAs_Definition(RTSCOPE* pScope);
    GXBOOL  ParseStatementAs_Function(RTSCOPE* pScope);
    GXBOOL  ParseFunctionArguments(STATEMENT* pStat, RTSCOPE* pArgScope);

    GXBOOL  ParseStatementAs_Struct(RTSCOPE* pScope);
    GXBOOL  ParseStructMember(STATEMENT* pStat, STRUCT_MEMBER& member, TOKEN**p, const TOKEN* pEnd);
    GXBOOL  ParseStructMembers(STATEMENT* pStat, RTSCOPE* pStruScope);

    GXBOOL  ParseStatementAs_Expression(STATEMENT* pStat, RTSCOPE* pScope/*, GXBOOL bDbgRelocale*/); // (算数表)达式

    GXBOOL  ParseArithmeticExpression(clsize begin, clsize end, SYNTAXNODE::UN* pUnion);
    GXBOOL  ParseArithmeticExpression(const RTSCOPE& scope, SYNTAXNODE::UN* pUnion);
    GXBOOL  ParseArithmeticExpression(const RTSCOPE& scope, SYNTAXNODE::UN* pUnion, int nMinPrecedence); // 递归函数

    GXBOOL  ParseRemainStatement(RTSCOPE::TYPE parse_end, const RTSCOPE& scope, SYNTAXNODE::UN* pUnion);
    GXBOOL  ParseExpression(const RTSCOPE& scope, SYNTAXNODE::UN* pUnion);
    //GXBOOL  ParseExpression(SYNTAXNODE::UN* pUnion, clsize begin, clsize end);
    GXBOOL  ParseFunctionCall(const RTSCOPE& scope, SYNTAXNODE::UN* pUnion);
    GXBOOL  ParseFunctionIndexCall(const RTSCOPE& scope, SYNTAXNODE::UN* pUnion);
    GXBOOL  TryKeywords(const RTSCOPE& scope, SYNTAXNODE::UN* pUnion, RTSCOPE::TYPE* parse_end);
    GXBOOL  TryBlock(const RTSCOPE& scope, SYNTAXNODE::UN* pUnion, RTSCOPE::TYPE* parse_end); // 解析一个代码块，用{}限定的一组或者仅有一句表达式的代码
    RTSCOPE::TYPE  ParseFlowIf(const RTSCOPE& scope, SYNTAXNODE::UN* pUnion, GXBOOL bElseIf);
    RTSCOPE::TYPE  MakeFlowForScope(const RTSCOPE& scope, RTSCOPE* pInit, RTSCOPE* pCond, RTSCOPE* pIter, RTSCOPE* pBlock, SYNTAXNODE::UN* pBlockNode);
    RTSCOPE::TYPE  ParseFlowFor(const RTSCOPE& scope, SYNTAXNODE::UN* pUnion);
    RTSCOPE::TYPE  ParseFlowWhile(const RTSCOPE& scope, SYNTAXNODE::UN* pUnion);
    RTSCOPE::TYPE  ParseFlowDoWhile(const RTSCOPE& scope, SYNTAXNODE::UN* pUnion);
    RTSCOPE::TYPE  ParseStructDefine(const RTSCOPE& scope, SYNTAXNODE::UN* pUnion);
    GXBOOL  MakeInstruction(const TOKEN* pOpcode, int nMinPrecedence, const RTSCOPE* pScope, SYNTAXNODE::UN* pParent, int nMiddle); // nMiddle是把RTSCOPE分成两个RTSCOPE的那个索引
    GXBOOL  MakeSyntaxNode(SYNTAXNODE::UN* pDest, SYNTAXNODE::MODE mode, const TOKEN* pOpcode, SYNTAXNODE::UN* pOperandA, SYNTAXNODE::UN* pOperandB);
    GXBOOL  MakeSyntaxNode(SYNTAXNODE::UN* pDest, SYNTAXNODE::MODE mode, SYNTAXNODE::UN* pOperandA, SYNTAXNODE::UN* pOperandB);
    GXBOOL  MakeScope(RTSCOPE* pOut, MAKESCOPE* pParam);
    //GXBOOL  FindScope(RTSCOPE* pOut, RTSCOPE::TYPE _begin, RTSCOPE::TYPE _end);

    GXBOOL  ParseStatement(RTSCOPE* pScope);
    void    RelocaleStatements(StatementArray& aStatements);
    void    RelocalePointer();
    void    RelocaleSyntaxPtr(SYNTAXNODE* pNode);
    GXBOOL  IsIntrinsicType(GXLPCSTR szType);

    GXLPCSTR GetUniqueString(const TOKEN* pSym);
    const TYPE* ParseType(const TOKEN* pSym);
    //clsize   FindSemicolon(clsize begin, clsize end) const;

    SYNTAXNODE::MODE TryGetNode(const SYNTAXNODE::UN* pUnion) const;

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


  protected:
    typedef Marimo::DataPoolErrorMsg<ch> ErrorMessage;
    ErrorMessage*       m_pMsg;
    TokenArray          m_aTokens;
    clstd::StringSetA   m_Strings;
    TypeSet             m_TypeSet;
    StatementArray      m_aStatements;
    StatementArray      m_aSubStatements;   // m_aStatements的次级储存，没有顺序关系
    
    MemberArray         m_aMembersPack;     // 结构体所有成员变量都存在这里
    ArgumentsArray      m_aArgumentsPack;   // 所有函数参数都存在这个表里
    SyntaxNodeArray     m_aSyntaxNodePack;  // 表达式语法节点

    TOKEN              m_CurSymInfo;       // 遍历时符号的优先级信息
    int                 m_nMaxPrecedence;   // 优先级最大值
    int                 m_nDbgNumOfExpressionParse; // 调试模式用于记录解析表达式迭代次数的变量
    static INTRINSIC_TYPE s_aIntrinsicType[];
  public:
    CodeParser();
    virtual ~CodeParser();
    b32                 Attach                  (const char* szExpression, clsize nSize);
    clsize              GenerateTokens          ();
    const TokenArray*   GetTokensArray          () const;
    GXBOOL              Parse                   ();

    const StatementArray& GetStatments          () const;

//    GXBOOL   IsToken(const SYNTAXNODE::UN* pUnion) const;
    SYNTAXNODE::FLAGS TryGetNodeType(const SYNTAXNODE::UN* pUnion) const;

    void DbgDumpScope(clStringA& str, const RTSCOPE& scope);
    void DbgDumpScope(clStringA& str, clsize begin, clsize end, GXBOOL bRaw);
    void DbgDumpScope(GXLPCSTR opcode, const RTSCOPE& scopeA, const RTSCOPE& scopeB);
    void DbgDumpSyntaxTree(clStringArrayA* pArray, const SYNTAXNODE* pNode, int precedence, clStringA* pStr = NULL);


    clStringArrayA    m_aDbgExpressionOperStack;
  };

} // namespace UVShader

#endif // #ifndef _EXPRESSION_PARSER_H_