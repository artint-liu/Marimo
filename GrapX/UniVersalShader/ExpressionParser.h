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
#define OPP(_PRE) (ExpressionParser::SYMBOL::FIRST_OPCODE_PRECEDENCE + _PRE)

  class ExpressionParser : public SmartStreamA
  {
  public:
    struct SYMBOL
    {
      const static int scope_bits = 22;
      const static int precedence_bits = 10;
      const static int FIRST_OPCODE_PRECEDENCE = 1;
      const static int ID_BRACE = 15;
      iterator  sym;
      int       scope : scope_bits;           // 括号匹配索引
      int       precedence : precedence_bits; // 符号优先级
      // [precedence]
      // 1~14 表示运算符的优先级
      // 15 表示"{","}","(",")","[","]"这些之一，此时scope数据是相互对应的，例如
      // array[open].scope = closed && array[closed].scope = open
    };

    typedef clvector<SYMBOL> SymbolArray;

    enum StatementType
    {
      StatementType_Empty,
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
      InputModifier eModifier;
      GXLPCSTR      szType;
      GXLPCSTR      szName;
      GXLPCSTR      szSemantic;
    };
    typedef clvector<FUNCTION_ARGUMENT> ArgumentsArray;
    
    struct STRUCT_MEMBER // 结构体成员/Shader标记
    {
      GXLPCSTR szType;
      GXLPCSTR szName;
      GXLPCSTR szSignature;  // 结构体成员没有这项
    };
    typedef clvector<STRUCT_MEMBER> MemberArray;

    struct SYNTAXNODE
    {
      enum FLAGS
      {
        FLAG_OPERAND_SHIFT     = 8,
        FLAG_OPERAND_TYPEMASK  = 0x00000003,
        FLAG_OPERAND_IS_NODE   = 0x00000001,
        FLAG_OPERAND_IS_SYMBOL = 0x00000002,
      };

      enum MODE
      {
        MODE_Normal,        // 操作符 + 操作数 模式
        MODE_FunctionCall,  // 函数调用
        MODE_Definition,    // 变量定义
        MODE_Flow_While,
        MODE_Flow_If,
        MODE_Flow_ElseIf,
        MODE_Flow_Else,
        MODE_Flow_For,            // [[MODE_Flow_ForInit] [MODE_Flow_ForRunning]] [statement block]
        MODE_Flow_ForInit,        // for 的初始化部分 [MODE_Flow_ForInit] [MODE_Flow_ForRunning]
        MODE_Flow_ForRunning,     // for 的条件和步进部分
        //MODE_Flow_Switch,
        //MODE_Flow_Do,
        //MODE_Flow_Break,
        //MODE_Flow_Continue,
        //MODE_Flow_Discard,
        MODE_Return,
        MODE_Chain,         // 表达式链表,链表中的应该属于同一个作用域
      };

      const static int s_NumOfOperand = 2;

      GXDWORD flags : 16;
      MODE mode : 16;
      const SYMBOL* pOpcode;

      union UN {
        SYNTAXNODE* pNode;
        const SYMBOL* pSym;
      };

      UN Operand[s_NumOfOperand];

      inline FLAGS GetOperandType(int index) const {
        const int shift = FLAG_OPERAND_SHIFT * index;
        return (FLAGS)((flags >> shift) & FLAG_OPERAND_TYPEMASK);
      }

      inline GXBOOL OperandA_IsNode() const {
        const GXDWORD dwTypeMask = FLAG_OPERAND_TYPEMASK;
        return (flags & dwTypeMask) == FLAG_OPERAND_IS_NODE;
      }

      inline GXBOOL OperandB_IsNode() const {
        const GXDWORD dwTypeMask = FLAG_OPERAND_TYPEMASK << FLAG_OPERAND_SHIFT;
        const GXDWORD dwNode = FLAG_OPERAND_IS_NODE << FLAG_OPERAND_SHIFT;
        return (flags & dwTypeMask) == dwNode;
      }
    };
    typedef clvector<SYNTAXNODE> SyntaxNodeArray;

    //////////////////////////////////////////////////////////////////////////

    struct STATEMENT
    {
      StatementType type;
      union
      {
        struct { // 函数体/函数声明
          StorageClass eStorageClass;
          GXLPCSTR     szReturnType;
          GXLPCSTR     szName;
          GXLPCSTR     szSemantic;
          FUNCTION_ARGUMENT*  pArguments;
          clsize              nNumOfArguments;
        }func;

        struct { // 结构体定义
          GXLPCSTR        szName;
          STRUCT_MEMBER*  pMembers;
          clsize          nNumOfMembers;
        }stru;

        struct { // 表达式定义
          SYNTAXNODE::UN  sRoot;
        }expr;
      };
    };

    typedef clvector<STATEMENT> StatementArray;

    //////////////////////////////////////////////////////////////////////////

    struct RTSCOPE // 运行时的范围描述结构体
    {
      clsize begin;
      clsize end;

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

    //////////////////////////////////////////////////////////////////////////

  private:
    static u32 CALLBACK MultiByteOperatorProc(iterator& it, u32 nRemain, u32_ptr lParam);
    static u32 CALLBACK IteratorProc         (iterator& it, u32 nRemain, u32_ptr lParam);

    void    InitPacks();
    void    Cleanup();
    clsize  EstimateForSymbolsCount () const;  // 从Stream的字符数估计Symbol的数量

    GXBOOL  ParseStatementAs_Function(RTSCOPE* pScope);
    GXBOOL  ParseFunctionArguments(STATEMENT* pStat, RTSCOPE* pArgScope);

    GXBOOL  ParseStatementAs_Struct(RTSCOPE* pScope);
    GXBOOL  ParseStructMember(STATEMENT* pStat, RTSCOPE* pStruScope);

    GXBOOL  ParseExpression(RTSCOPE* pScope, SYNTAXNODE::UN* pUnion, int nMinPrecedence);
    GXBOOL  ParseExpression(SYNTAXNODE::UN* pUnion, int nMinPrecedence, clsize begin, clsize end);
    GXBOOL  ParseFunctionCall(RTSCOPE* pScope, SYNTAXNODE::UN* pUnion, int nMinPrecedence);
    GXBOOL  ParseFlowIf(RTSCOPE* pScope, SYNTAXNODE::UN* pUnion);
    GXBOOL  ParseFlowFor(RTSCOPE* pScope, SYNTAXNODE::UN* pUnion);
    GXBOOL  ParseFlowWhile(RTSCOPE* pScope, SYNTAXNODE::UN* pUnion);
    GXBOOL  MakeInstruction(const SYMBOL* pOpcode, int nMinPrecedence, RTSCOPE* pScope, SYNTAXNODE::UN* pParent, int nMiddle); // nMiddle是把RTSCOPE分成两个RTSCOPE的那个索引
    GXBOOL  MakeSyntaxNode(SYNTAXNODE::UN* pDest, SYNTAXNODE::MODE mode, const SYMBOL* pOpcode, SYNTAXNODE::UN* pOperandA, SYNTAXNODE::UN* pOperandB);


    GXBOOL  ParseStatement(RTSCOPE* pScope);
    void    RelocalePointer();
    void    RelocaleSyntaxPtr(SYNTAXNODE* pNode);
    GXBOOL  IsIntrinsicType(GXLPCSTR szType);

    GXLPCSTR GetUniqueString(const SYMBOL* pSym);
    const TYPE* ParseType(const SYMBOL* pSym);
    GXBOOL   IsSymbol(const SYNTAXNODE::UN* pUnion) const;
    clsize   FindSemicolon(clsize begin, clsize end) const;

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
    ErrorMessage*           m_pMsg;
    SymbolArray         m_aSymbols;
    clstd::StringSetA   m_Strings;
    TypeSet             m_TypeSet;
    StatementArray      m_aStatements;
    
    MemberArray         m_aMembersPack;     // 结构体所有成员变量都存在这里
    ArgumentsArray      m_aArgumentsPack;   // 所有函数参数都存在这个表里
    SyntaxNodeArray     m_aSyntaxNodePack;

    SYMBOL              m_CurSymInfo;       // 遍历时符号的优先级信息
    int                 m_nMaxPrecedence;   // 优先级最大值
    int                 m_nDbgNumOfExpressionParse; // 调试模式用于记录解析表达式迭代次数的变量
    static INTRINSIC_TYPE s_aIntrinsicType[];
  public:
    ExpressionParser();
    virtual ~ExpressionParser();
    b32                 Attach                  (const char* szExpression, clsize nSize);
    clsize              GenerateSymbols         ();
    const SymbolArray*  GetSymbolsArray         () const;
    GXBOOL              Parse                   ();

    void DbgDumpScope(clStringA& str, const RTSCOPE& scope);
    void DbgDumpScope(clStringA& str, clsize begin, clsize end, GXBOOL bRaw);
    void DbgDumpScope(GXLPCSTR opcode, const RTSCOPE& scopeA, const RTSCOPE& scopeB);
    void DbgDumpSyntaxTree(const SYNTAXNODE* pNode, int precedence, clStringA* pStr = NULL);

    /*为了测试，临时改为公共函数*/GXBOOL  ParseStatementAs_Expression(RTSCOPE* pScope, GXBOOL bDbgRelocale); // (算数表)达式

    clStringArrayA    m_aDbgExpressionOperStack;
  };

} // namespace UVShader

#endif // #ifndef _EXPRESSION_PARSER_H_