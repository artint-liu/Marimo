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

  class ExpressionParser : public SmartStreamA
  {
  public:
    struct SYMBOL
    {
      const static int pair_bits = 22;
      const static int precedence_bits = 10;
      iterator  sym;
      int       pair : pair_bits;             // 括号匹配索引
      int       precedence : precedence_bits; // 符号优先级
    };

    typedef clvector<SYMBOL> SymbolArray;

    enum StatementType
    {
      StatementType_Empty,
      StatementType_FunctionDecl,   // 函数声明
      StatementType_Function,       // 函数体
      StatementType_Struct,         // 结构体
      StatementType_Signatures,     // 用于shader输入输出标记的结构体
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
    };

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
          GXLPCSTR szName;
          STRUCT_MEMBER*  pMembers;
          clsize          nNumOfMembers;
        }stru;
      };
    };

    typedef clvector<STATEMENT> StatementArray;

    struct RTSCOPE // 运行时的范围描述结构体
    {
      clsize begin;
      clsize end;
    };

    struct INTRINSIC_TYPE
    {
      GXLPCSTR name;
    };

  private:
    static u32 CALLBACK MultiByteOperatorProc(iterator& it, u32 nRemain, u32_ptr lParam);
    static u32 CALLBACK IteratorProc         (iterator& it, u32 nRemain, u32_ptr lParam);

    clsize  EstimateForSymbolsCount () const;  // 从Stream的字符数估计Symbol的数量

    GXBOOL  ParseStatementAs_Function(RTSCOPE* pScope);
    GXBOOL  ParseFunctionArguments(STATEMENT* pStat, RTSCOPE* pArgScope);

    GXBOOL  ParseStatementAs_Struct(RTSCOPE* pScope);
    GXBOOL  ParseStructMember(STATEMENT* pStat, RTSCOPE* pStruScope);

    GXBOOL  ParseExpression(RTSCOPE* pScope, int nMinPrecedence);


    GXBOOL  ParseStatement(RTSCOPE* pScope);
    void    RelocalePointer();
    GXBOOL  IsIntrinsicType(GXLPCSTR szType);

    GXLPCSTR GetUniqueString(const SYMBOL* pSym);
    GXBOOL   ParseType(GXOUT TYPE* pType);

  protected:
    SymbolArray         m_aSymbols;
    clstd::StringSetA   m_Strings;
    StatementArray      m_aStatements;
    ArgumentsArray      m_aArgumentsPack;   // 所有函数参数都存在这个表里
    MemberArray         m_aMembersPack;     // 结构体所有成员变量都存在这里
    SYMBOL              m_CurSymInfo;       // 遍历时符号的优先级信息
    int                 m_nMaxPrecedence;   // 优先级最大值
    int                 m_nDbgNumOfExpressionParse; // 调试模式用于记录解析表达式迭代次数的变量
    static INTRINSIC_TYPE s_aIntrinsicType[];
  public:
    ExpressionParser();
    b32                 Attach                  (const char* szExpression, clsize nSize);
    clsize              GenerateSymbols         ();
    const SymbolArray*  GetSymbolsArray         () const;
    GXBOOL              Parse                   ();

    void DbgDumpScope(clStringA& str, const RTSCOPE& scope);
    void DbgDumpScope(clStringA& str, clsize begin, clsize end);
    /*为了测试，临时改为公共函数*/GXBOOL  ParseStatementAs_Expression(RTSCOPE* pScope); // (算数表)达式

  };

} // namespace UVShader

#endif // #ifndef _EXPRESSION_PARSER_H_