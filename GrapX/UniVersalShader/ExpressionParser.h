#ifndef _EXPRESSION_PARSER_H_
#define _EXPRESSION_PARSER_H_

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

  class CodeParser : public ArithmeticExpression
  {
  public:


    struct MACRO
    {
      typedef clhash_map<clStringA, MACRO> Dict; // TODO: key 改为 TOKEN

      TokenList aFormalParams;  // 形参
      TokenList aTokens;        // 替换内容

      void set(const Dict& dict, const TokenArray& tokens, int begin_at);
      void clear();

      void ClearContainer (); // 这个用来清除iterator里的container，指向subparse容易出错
      int  ExpandMacro    (const Dict& dict); // 展开宏
    };


    enum AttachFlag
    {
      AttachFlag_NotLoadMessage = 0x00000001,
    };

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

    struct RTPPCONTEXT // 运行时预处理解析上下文
    {
      T_LPCSTR ppend;       // 当前预处理的结尾
      T_LPCSTR stream_end;  // 字符流的结尾，GetStreamPtr() + GetStreamCount()
      iterator iter_next;   // #标志预处理结束，忽略注释后第一个token

    };

    enum RTState // 运行时状态
    {
      State_InPreprocess = 0x00000001, // 解析预处理
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
    static u32 CALLBACK IteratorProc         (iterator& it, u32 nRemain, u32_ptr lParam);

    void    InitPacks();
    void    Cleanup();

    GXBOOL  ParseStatementAs_Definition(RTSCOPE* pScope);
    GXBOOL  ParseStatementAs_Function(RTSCOPE* pScope);
    GXBOOL  ParseFunctionArguments(STATEMENT* pStat, RTSCOPE* pArgScope);

    GXBOOL  ParseStatementAs_Struct(RTSCOPE* pScope);
    GXBOOL  ParseStructMember(STATEMENT* pStat, STRUCT_MEMBER& member, TOKEN**p, const TOKEN* pEnd);
    GXBOOL  ParseStructMembers(STATEMENT* pStat, RTSCOPE* pStruScope);

    GXBOOL  ParseStatementAs_Expression(STATEMENT* pStat, RTSCOPE* pScope/*, GXBOOL bDbgRelocale*/); // (算数表)达式

    //GXBOOL  ParseArithmeticExpression(clsize begin, clsize end, SYNTAXNODE::UN* pUnion);
    //GXBOOL  ParseArithmeticExpression(const RTSCOPE& scope, SYNTAXNODE::UN* pUnion);
    //GXBOOL  ParseArithmeticExpression(const RTSCOPE& scope, SYNTAXNODE::UN* pUnion, int nMinPrecedence); // 递归函数

    GXBOOL  ParseRemainStatement(RTSCOPE::TYPE parse_end, const RTSCOPE& scope, SYNTAXNODE::UN* pUnion);
    GXBOOL  ParseExpression(const RTSCOPE& scope, SYNTAXNODE::UN* pUnion);
    //GXBOOL  ParseExpression(SYNTAXNODE::UN* pUnion, clsize begin, clsize end);
    //GXBOOL  ParseFunctionCall(const RTSCOPE& scope, SYNTAXNODE::UN* pUnion);
    GXBOOL  ParseFunctionIndexCall(const RTSCOPE& scope, SYNTAXNODE::UN* pUnion);
    GXBOOL  TryKeywords(const RTSCOPE& scope, SYNTAXNODE::UN* pUnion, RTSCOPE::TYPE* parse_end);
    GXBOOL  TryBlock(const RTSCOPE& scope, SYNTAXNODE::UN* pUnion, RTSCOPE::TYPE* parse_end); // 解析一个代码块，用{}限定的一组或者仅有一句表达式的代码
    RTSCOPE::TYPE  ParseFlowIf(const RTSCOPE& scope, SYNTAXNODE::UN* pUnion, GXBOOL bElseIf);
    RTSCOPE::TYPE  MakeFlowForScope(const RTSCOPE& scope, RTSCOPE* pInit, RTSCOPE* pCond, RTSCOPE* pIter, RTSCOPE* pBlock, SYNTAXNODE::UN* pBlockNode);
    RTSCOPE::TYPE  ParseFlowFor(const RTSCOPE& scope, SYNTAXNODE::UN* pUnion);
    RTSCOPE::TYPE  ParseFlowWhile(const RTSCOPE& scope, SYNTAXNODE::UN* pUnion);
    RTSCOPE::TYPE  ParseFlowDoWhile(const RTSCOPE& scope, SYNTAXNODE::UN* pUnion);
    RTSCOPE::TYPE  ParseStructDefine(const RTSCOPE& scope, SYNTAXNODE::UN* pUnion);
    GXBOOL  MakeScope(RTSCOPE* pOut, MAKESCOPE* pParam);
    //GXBOOL  FindScope(RTSCOPE* pOut, RTSCOPE::TYPE _begin, RTSCOPE::TYPE _end);

    T_LPCSTR ParseMacro(const RTPPCONTEXT& ctx, T_LPCSTR begin, T_LPCSTR end);
    void     Macro_Define(const TokenArray& aTokens);
    void     Macro_Undefine(const RTPPCONTEXT& ctx, const TokenArray& aTokens);
    T_LPCSTR Macro_IfDefine(const RTPPCONTEXT& ctx, GXBOOL bNot, const TokenArray& aTokens); // bNot 表示 if not define
    T_LPCSTR Macro_SkipConditionalBlock(T_LPCSTR begin, T_LPCSTR end); // 从这条预处理的行尾开始，跳过这块预处理，begin应该是当前预处理的结尾

    static T_LPCSTR Macro_SkipGaps( T_LPCSTR begin, T_LPCSTR end );  // 返回跳过制表符和空格后的字符串地址
    static GXBOOL CompareString(T_LPCSTR str1, T_LPCSTR str2, size_t count);

    GXBOOL  ParseStatement(RTSCOPE* pScope);
    void    RelocaleStatements(StatementArray& aStatements);
    void    RelocalePointer();
    void    RelocaleSyntaxPtr(SYNTAXNODE* pNode);
    GXBOOL  IsIntrinsicType(GXLPCSTR szType);

    GXLPCSTR GetUniqueString(const TOKEN* pSym);
    const TYPE* ParseType(const TOKEN* pSym);
    //clsize   FindSemicolon(clsize begin, clsize end) const;

    //void OutputErrorW(GXSIZE_T offset, GXUINT code, ...);
    void OutputErrorW(const TOKEN& token, GXUINT code, ...);
    void OutputErrorW(T_LPCSTR ptr, GXUINT code, ...);
    //void OutputErrorW(const TOKEN& token, GXUINT code, ...);

    //SYNTAXNODE::MODE TryGetNode(const SYNTAXNODE::UN* pUnion) const;

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
    GXDWORD             m_dwState;          // 内部状态, 参考RTState
    clstd::StringSetA   m_Strings;
    TypeSet             m_TypeSet;
    StatementArray      m_aStatements;
    StatementArray      m_aSubStatements;   // m_aStatements的次级储存，没有顺序关系
    
    MemberArray         m_aMembersPack;     // 结构体所有成员变量都存在这里
    ArgumentsArray      m_aArgumentsPack;   // 所有函数参数都存在这个表里

    MACRO::Dict         m_Macros;
    CodeParser*         m_pSubParser;

  public:
    CodeParser();
    virtual ~CodeParser();
    b32                 Attach                  (const char* szExpression, clsize nSize, GXDWORD dwFlags, GXLPCWSTR szFilename);
    clsize              GenerateTokens          ();
    GXBOOL              Parse                   ();

    const StatementArray& GetStatments          () const;

//    GXBOOL   IsToken(const SYNTAXNODE::UN* pUnion) const;
    //SYNTAXNODE::FLAGS TryGetNodeType(const SYNTAXNODE::UN* pUnion) const;

    void DbgDumpSyntaxTree(clStringArrayA* pArray, const SYNTAXNODE* pNode, int precedence, clStringA* pStr = NULL);


  };

} // namespace UVShader

#endif // #ifndef _EXPRESSION_PARSER_H_