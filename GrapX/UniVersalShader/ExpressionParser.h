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
      int       scope : scope_bits;           // ����ƥ������
      int       precedence : precedence_bits; // �������ȼ�
      // [precedence]
      // 1~14 ��ʾ����������ȼ�
      // 15 ��ʾ"{","}","(",")","[","]"��Щ֮һ����ʱscope�������໥��Ӧ�ģ�����
      // array[open].scope = closed && array[closed].scope = open
    };

    typedef clvector<SYMBOL> SymbolArray;

    enum StatementType
    {
      StatementType_Empty,
      StatementType_FunctionDecl,   // ��������
      StatementType_Function,       // ������
      StatementType_Struct,         // �ṹ��
      StatementType_Signatures,     // ����shader���������ǵĽṹ��
      StatementType_Expression,     // ���ʽ
    };

    enum StorageClass // �������Σ���ѡ
    {
      StorageClass_empty,
      StorageClass_inline,
    };

    enum InputModifier // ������������
    {
      InputModifier_in,
      InputModifier_out,
      InputModifier_inout,
      InputModifier_uniform,
    };

    struct TYPE
    {
      GXLPCSTR  name;
      int       R;    // Row, ���� array size
      int       C;    // Column

      bool operator<(const TYPE& t) const;
    };
    typedef clset<TYPE>   TypeSet;

    struct FUNCTION_ARGUMENT // ��������
    {
      InputModifier eModifier;
      GXLPCSTR      szType;
      GXLPCSTR      szName;
      GXLPCSTR      szSemantic;
    };
    typedef clvector<FUNCTION_ARGUMENT> ArgumentsArray;
    
    struct STRUCT_MEMBER // �ṹ���Ա/Shader���
    {
      GXLPCSTR szType;
      GXLPCSTR szName;
      GXLPCSTR szSignature;  // �ṹ���Աû������
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
        MODE_Normal,        // ������ + ������ ģʽ
        MODE_FunctionCall,  // ��������
        MODE_Definition,    // ��������
        MODE_Flow_While,
        MODE_Flow_If,
        MODE_Flow_ElseIf,
        MODE_Flow_Else,
        MODE_Flow_For,            // [[MODE_Flow_ForInit] [MODE_Flow_ForRunning]] [statement block]
        MODE_Flow_ForInit,        // for �ĳ�ʼ������ [MODE_Flow_ForInit] [MODE_Flow_ForRunning]
        MODE_Flow_ForRunning,     // for �������Ͳ�������
        //MODE_Flow_Switch,
        //MODE_Flow_Do,
        //MODE_Flow_Break,
        //MODE_Flow_Continue,
        //MODE_Flow_Discard,
        MODE_Return,
        MODE_Chain,         // ���ʽ����,�����е�Ӧ������ͬһ��������
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
        struct { // ������/��������
          StorageClass eStorageClass;
          GXLPCSTR     szReturnType;
          GXLPCSTR     szName;
          GXLPCSTR     szSemantic;
          FUNCTION_ARGUMENT*  pArguments;
          clsize              nNumOfArguments;
        }func;

        struct { // �ṹ�嶨��
          GXLPCSTR        szName;
          STRUCT_MEMBER*  pMembers;
          clsize          nNumOfMembers;
        }stru;

        struct { // ���ʽ����
          SYNTAXNODE::UN  sRoot;
        }expr;
      };
    };

    typedef clvector<STATEMENT> StatementArray;

    //////////////////////////////////////////////////////////////////////////

    struct RTSCOPE // ����ʱ�ķ�Χ�����ṹ��
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

    struct INTRINSIC_TYPE // ��������
    {
      GXLPCSTR name;
      clsize   name_len;
      int      R;         // �������ֵ
      int      C;         // �������ֵ
    };

    //////////////////////////////////////////////////////////////////////////

  private:
    static u32 CALLBACK MultiByteOperatorProc(iterator& it, u32 nRemain, u32_ptr lParam);
    static u32 CALLBACK IteratorProc         (iterator& it, u32 nRemain, u32_ptr lParam);

    void    InitPacks();
    void    Cleanup();
    clsize  EstimateForSymbolsCount () const;  // ��Stream���ַ�������Symbol������

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
    GXBOOL  MakeInstruction(const SYMBOL* pOpcode, int nMinPrecedence, RTSCOPE* pScope, SYNTAXNODE::UN* pParent, int nMiddle); // nMiddle�ǰ�RTSCOPE�ֳ�����RTSCOPE���Ǹ�����
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
    
    MemberArray         m_aMembersPack;     // �ṹ�����г�Ա��������������
    ArgumentsArray      m_aArgumentsPack;   // ���к��������������������
    SyntaxNodeArray     m_aSyntaxNodePack;

    SYMBOL              m_CurSymInfo;       // ����ʱ���ŵ����ȼ���Ϣ
    int                 m_nMaxPrecedence;   // ���ȼ����ֵ
    int                 m_nDbgNumOfExpressionParse; // ����ģʽ���ڼ�¼�������ʽ���������ı���
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

    /*Ϊ�˲��ԣ���ʱ��Ϊ��������*/GXBOOL  ParseStatementAs_Expression(RTSCOPE* pScope, GXBOOL bDbgRelocale); // (������)��ʽ

    clStringArrayA    m_aDbgExpressionOperStack;
  };

} // namespace UVShader

#endif // #ifndef _EXPRESSION_PARSER_H_