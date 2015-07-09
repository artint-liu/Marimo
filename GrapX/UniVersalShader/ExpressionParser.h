#ifndef _EXPRESSION_PARSER_H_
#define _EXPRESSION_PARSER_H_

namespace UVShader
{
  class ExpressionParser : public SmartStreamA
  {
  public:
    struct SYMBOL
    {
      iterator  sym;
      int       pair : 22;       // ����ƥ������
      int       precedence : 10; // �������ȼ�
    };

    typedef clvector<SYMBOL> SymbolArray;

    enum StatementType
    {
      StatementType_Empty,
      StatementType_FunctionDecl,   // ��������
      StatementType_Function,       // ������
      StatementType_Struct,         // �ṹ��
      StatementType_Signatures,     // ����shader���������ǵĽṹ��
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
    };

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
          GXLPCSTR szName;
          STRUCT_MEMBER*  pMembers;
          clsize          nNumOfMembers;
        }stru;
      };
    };

    typedef clvector<STATEMENT> StatementArray;

    struct RTSCOPE // ����ʱ�ķ�Χ�����ṹ��
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

    clsize  EstimateForSymbolsCount () const;  // ��Stream���ַ�������Symbol������

    GXBOOL  ParseStatementAs_Function(RTSCOPE* pScope);
    GXBOOL  ParseFunctionArguments(STATEMENT* pStat, RTSCOPE* pArgScope);

    GXBOOL  ParseStatementAs_Struct(RTSCOPE* pScope);
    GXBOOL  ParseStructMember(STATEMENT* pStat, RTSCOPE* pStruScope);

    GXBOOL  ParseStatement(RTSCOPE* pScope);
    void    RelocalePointer();
    GXBOOL  IsIntrinsicType(GXLPCSTR szType);

    GXLPCSTR GetUniqueString(const SYMBOL* pSym);
    GXBOOL   ParseType(GXOUT TYPE* pType);

  protected:
    SymbolArray         m_aSymbols;
    clstd::StringSetA   m_Strings;
    StatementArray      m_aStatements;
    ArgumentsArray      m_aArgumentsPack;   // ���к��������������������
    MemberArray         m_aMembersPack;     // �ṹ�����г�Ա��������������
    SYMBOL              m_CurSymInfo;       // ����ʱ���ŵ����ȼ���Ϣ

    static INTRINSIC_TYPE s_aIntrinsicType[];
  public:
    ExpressionParser();
    b32                 Attach                  (const char* szExpression, clsize nSize);
    clsize              GenerateSymbols         ();
    const SymbolArray*  GetSymbolsArray         () const;
    GXBOOL              Parse                   ();
  };

} // namespace UVShader

#endif // #ifndef _EXPRESSION_PARSER_H_