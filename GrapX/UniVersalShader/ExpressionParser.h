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


  class CodeParser : public ArithmeticExpression
  {
  public:
    typedef clstack<int> MacroStack;        // ���βκ����õĴ����ջ

    struct MACRO_TOKEN : public TOKEN
    {
      typedef cllist<MACRO_TOKEN> List;
      typedef clvector<MACRO_TOKEN> Array;

      MACRO_TOKEN(const TOKEN& t)
        : TOKEN(t)
        , formal_index(-1)
      {
      }

      int formal_index; // ��Ӧ���β�����
    };

    struct MACRO
    {
      typedef clmap<clStringA, MACRO> Dict; // macro ֧�����أ����� key �������� clStringA ������, Dict Ҫ�����������
      typedef clvector<int>           IntArray;
      //typedef clhash_set<clStringA>   Set;
      MACRO_TOKEN::Array aFormalParams;  // �β�
      MACRO_TOKEN::List aTokens;        // �滻����
      //GXDWORD bTranslate     : 1; // ������������һ����ǣ���Ҫת��
      //GXDWORD bHasLINE       : 1; // ��__LINE__��, ��������б仯��
      //GXDWORD bHasFILE       : 1; // ��__FILE__��
      //GXDWORD bPoundSign     : 1; // #����

      //MACRO();

      void set            (const Dict& dict, const TOKEN::Array& tokens, int begin_at);
      void clear          ();

      void ClearContainer (); // ����������iterator���container��ָ��subparse���׳���
      //int  ExpandMacro    (const Dict& dict); // չ����
    };

    //struct MACRO_EXPAND_CONTEXT
    //{
    //  typedef clstack<MACRO_EXPAND_CONTEXT> Stack;
    //  typedef cllist<MACRO_EXPAND_CONTEXT>  List;
    //  CodeParser* pParser;
    //  MACRO*      pMacro;
    //  TOKEN::List current;

    //  //// ActualParam ʵ��
    //  //TOKEN::Array aTokens; // ʵ���б�
    //  //RTSCOPE::Array aArgs; // ʵ��

    //  ArithmeticExpression::iterator itSave;

    //  template<class _TIter>
    //  void Append(const _TIter& _begin, const _TIter& _end);
    //};
    //

    struct PARSER_CONTEXT
    {
      GXUINT            nRefCount;
      clstd::StringSetA Strings;
      MACRO::Dict       Macros;
    };

    struct MACRO_EXPAND_CONTEXT
    {
      const TOKEN* pLineNumRef;
      TOKEN::List stream;
      const MACRO* pMacro;

      clvector<TOKEN::List> ActualParam;
    };

    //////////////////////////////////////////////////////////////////////////

    enum AttachFlag // ע��: ��RTState���ñ��λ
    {
      AttachFlag_Preprocess     = 0x00000001, // ��Ԥ�����У����״̬������Ϊ#ΪԤ����ʼ���
      AttachFlag_NotLoadMessage = 0x00010000, // TODO: ����ɵ�����
      AttachFlag_NotExpandMacro = 0x00020000, // ��չ����
      AttachFlag_NotExpandCond  = 0x00040000, // ��չ��defined()����ĺ�
    };

    enum StatementType
    {
      StatementType_Empty,
      StatementType_Definition,     // ��������
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

    enum UniformModifier
    {
      UniformModifier_empty,
      UniformModifier_const,
    };

    enum PPCondRank
    {
      PPCondRank_Empty = 0,
      PPCondRank_if    = 1,
      PPCondRank_elif  = 2,
      PPCondRank_else  = 3,
      PPCondRank_endif = 4,
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
      InputModifier eModifier;  // [opt]
      GXLPCSTR      szType;     // [req]
      GXLPCSTR      szName;     // [req]
      GXLPCSTR      szSemantic; // [opt]
    };
    typedef clvector<FUNCTION_ARGUMENT> ArgumentsArray;
    
    struct STRUCT_MEMBER // �ṹ���Ա/Shader���
    {
      GXLPCSTR szType;       // [req]
      GXLPCSTR szName;       // [req]
      GXLPCSTR szSignature;  // [req], �ṹ���Աû������
    };
    typedef clvector<STRUCT_MEMBER> MemberArray;


    //////////////////////////////////////////////////////////////////////////
    struct STATEMENT;
    struct STATEMENT_EXPR;

    struct STATEMENT_FUNC // ������/��������
    {
      StorageClass eStorageClass; // [opt]
      GXLPCSTR     szReturnType;  // [req]
      GXLPCSTR     szName;        // [req]
      GXLPCSTR     szSemantic;    // [opt]
      FUNCTION_ARGUMENT*  pArguments;       // [opt]
      clsize              nNumOfArguments;  // [opt]
      STATEMENT* pExpression;
    };

    struct STATEMENT_STRU // �ṹ�嶨��
    {
      GXLPCSTR        szName;
      STRUCT_MEMBER*  pMembers;
      clsize          nNumOfMembers;
    };

    struct STATEMENT_EXPR // ���ʽ����
    {
      SYNTAXNODE::DESC sRoot;
    };
    
    struct STATEMENT_DEFN
    {
      UniformModifier modifier;
      GXLPCSTR        szType;
      SYNTAXNODE::DESC  sRoot;
    };


    struct STATEMENT
    {
      StatementType type;
      union
      {
        STATEMENT_FUNC func;
        STATEMENT_STRU stru;
        STATEMENT_EXPR expr;
        STATEMENT_DEFN defn; // ȫ�ֱ�������
      };
    };

    typedef clvector<STATEMENT> StatementArray;

    //////////////////////////////////////////////////////////////////////////

    struct RTPPCONTEXT // ����ʱԤ�������������
    {
      T_LPCSTR ppend;       // ��ǰԤ����Ľ�β
      T_LPCSTR stream_end;  // �ַ����Ľ�β��GetStreamPtr() + GetStreamCount()
      iterator iter_next;   // #��־Ԥ�������������ע�ͺ��һ��token

    };

    enum RTState // ����ʱ״̬, ��AttachFlag���ñ��λ
    {
      State_InPreprocess = 0x00000001, // ����Ԥ����
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
    struct OPERAND // TODO: �������ʱ�ṹ�壬�Ժ����Դ�ļ��У�������ͷ�ļ���
    {
      VALUE  v;
      const TOKEN* pToken; // ���VALUE������token, pTokenӦ��Ϊ��

      GXBOOL OnToken(const SYNTAXNODE* pNode, int i)
      {
        clear();
        VALUE::State s = VALUE::State_OK;

        switch(pNode->mode)
        {
        case ArithmeticExpression::SYNTAXNODE::MODE_Opcode:
          s = v.set(*pNode->Operand[i].pSym);

        case ArithmeticExpression::SYNTAXNODE::MODE_FunctionCall:
          if(i == 0) {
            pToken = pNode->Operand[i].pSym;
          }
          else {
            s = v.set(*pNode->Operand[i].pSym);
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
      const RTSCOPE*  pScope;   // �޶�����
      RTSCOPE::TYPE   begin;    // ��ʼ
      GXBOOL          bBeginMate;// scope begin ȡ��m_aTokens[begin].scope
      RTSCOPE::TYPE   end;
      GXBOOL          bEndMate;
      GXWCHAR         chTermin; // ���begin��������ս��������һ��begin==end��scope

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

    GXBOOL  ParseStatementAs_Expression(STATEMENT* pStat, RTSCOPE* pScope/*, GXBOOL bDbgRelocale*/); // (������)��ʽ

    GXBOOL  CalculateValue(OPERAND& sOut, const SYNTAXNODE::DESC* pDesc);


    //GXBOOL  ParseArithmeticExpression(clsize begin, clsize end, SYNTAXNODE::UN* pUnion);
    //GXBOOL  ParseArithmeticExpression(const RTSCOPE& scope, SYNTAXNODE::UN* pUnion);
    //GXBOOL  ParseArithmeticExpression(const RTSCOPE& scope, SYNTAXNODE::UN* pUnion, int nMinPrecedence); // �ݹ麯��

    GXBOOL  ParseRemainStatement(RTSCOPE::TYPE parse_end, const RTSCOPE& scope, SYNTAXNODE::DESC* pDesc);
    GXBOOL  ParseExpression(const RTSCOPE& scope, SYNTAXNODE::DESC* pDesc);
    //GXBOOL  ParseExpression(SYNTAXNODE::UN* pUnion, clsize begin, clsize end);
    //GXBOOL  ParseFunctionCall(const RTSCOPE& scope, SYNTAXNODE::UN* pUnion);
    GXBOOL  ParseFunctionIndexCall(const RTSCOPE& scope, SYNTAXNODE::UN* pUnion);
    GXBOOL  TryKeywords(const RTSCOPE& scope, SYNTAXNODE::DESC* pDesc, RTSCOPE::TYPE* parse_end);
    GXBOOL  TryBlock(const RTSCOPE& scope, SYNTAXNODE::UN* pUnion, RTSCOPE::TYPE* parse_end); // ����һ������飬��{}�޶���һ����߽���һ����ʽ�Ĵ���
    RTSCOPE::TYPE  ParseFlowIf(const RTSCOPE& scope, SYNTAXNODE::DESC* pDesc, GXBOOL bElseIf);
    RTSCOPE::TYPE  MakeFlowForScope(const RTSCOPE& scope, RTSCOPE* pInit, RTSCOPE* pCond, RTSCOPE* pIter, RTSCOPE* pBlock, SYNTAXNODE::DESC* pBlockNode);
    RTSCOPE::TYPE  ParseFlowFor(const RTSCOPE& scope, SYNTAXNODE::DESC* pDesc);
    RTSCOPE::TYPE  ParseFlowWhile(const RTSCOPE& scope, SYNTAXNODE::DESC* pDesc);
    RTSCOPE::TYPE  ParseFlowDoWhile(const RTSCOPE& scope, SYNTAXNODE::DESC* pDesc);
    RTSCOPE::TYPE  ParseStructDefine(const RTSCOPE& scope, SYNTAXNODE::DESC* pDesc);
    GXBOOL  MakeScope(RTSCOPE* pOut, MAKESCOPE* pParam);
    //GXBOOL  FindScope(RTSCOPE* pOut, RTSCOPE::TYPE _begin, RTSCOPE::TYPE _end);
    //void    OnNextToken(iterator& it, TOKEN& token, GXBOOL bReplace = TRUE);
    GXBOOL  OnToken(TOKEN& token);
    void    GetNext(iterator& it, TOKEN& token);
    void    ExpandMacro(MACRO_EXPAND_CONTEXT& c);
    void    ExpandMacroStream(TOKEN::List& sTokenList, const TOKEN& line_num);
    GXBOOL  TryMatchMacro(MACRO_EXPAND_CONTEXT& ctx_out, TOKEN::List::iterator* it_out, const TOKEN::List::iterator& it_begin, const TOKEN::List::iterator& it_end);
    GXBOOL  MergeStringToken(const TOKEN& token);
    const MACRO* FindMacro(const TOKEN& token);

    T_LPCSTR DoPreprocess(const RTPPCONTEXT& ctx, T_LPCSTR begin, T_LPCSTR end);
    void     PP_Pragma(const TOKEN::Array& aTokens);
    void     PP_Define(const TOKEN::Array& aTokens);
    void     PP_Include(const TOKEN::Array& aTokens);
    void     PP_Undefine(const RTPPCONTEXT& ctx, const TOKEN::Array& aTokens);
    T_LPCSTR PP_IfDefine(const RTPPCONTEXT& ctx, GXBOOL bNot, const TOKEN::Array& aTokens); // bNot ��ʾ if not define
    T_LPCSTR PP_If(const RTPPCONTEXT& ctx, CodeParser* pParser);
    T_LPCSTR PP_SkipConditionalBlock(PPCondRank session, T_LPCSTR begin, T_LPCSTR end); // ������Ԥ�������β��ʼ���������Ԥ����beginӦ���ǵ�ǰԤ����Ľ�β
    GXBOOL   ExpandInnerMacro(TOKEN& token, const TOKEN& line_num); // ��Ҫ���滻__FILE__ __LINE__

    //GXBOOL   Macro_ExpandMacroInvoke(int nMacro, TOKEN& token);

    template<class _List, class _Iter>
    void     AppendWithExpandProprocess(_List& tokens, int offset, int nSrcLine, const _Iter& begin, const _Iter& end);

    //static void StringTokenToString(clStringW& strOut, const TOKEN::Array& aTokens, int nBegin);
    static T_LPCSTR Macro_SkipGaps( T_LPCSTR begin, T_LPCSTR end );  // ���������Ʊ���Ϳո����ַ�����ַ
    GXBOOL CompareString(T_LPCSTR str1, T_LPCSTR str2, size_t count);

    GXBOOL  ParseStatement(RTSCOPE* pScope);
    void    RelocaleStatements(StatementArray& aStatements);
    void    RelocalePointer();
    void    RelocaleSyntaxPtr(SYNTAXNODE* pNode);
    GXBOOL  IsIntrinsicType(GXLPCSTR szType);

    GXLPCSTR GetUniqueString(const TOKEN* pSym);
    const TYPE* ParseType(const TOKEN* pSym);
    //clsize   FindSemicolon(clsize begin, clsize end) const;

    //void OutputErrorW(GXSIZE_T offset, GXUINT code, ...);
    void OutputErrorW(GXUINT code, ...);  // �����һ����ЧtokenѰ���к�
    void OutputErrorW(const TOKEN& token, GXUINT code, ...);
    void OutputErrorW(T_LPCSTR ptr, GXUINT code, ...);

    CodeParser* GetRootParser();
    clBuffer* OpenIncludeFile(const clStringW& strFilename);
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
    typedef clmap<clStringW, clBuffer*> FileDict;
    GXDWORD             m_dwState;          // �ڲ�״̬, �ο�RTState
    CodeParser*         m_pParent;
    TypeSet             m_TypeSet;
    StatementArray      m_aStatements;
    StatementArray      m_aSubStatements;   // m_aStatements�Ĵμ����棬û��˳���ϵ
    
    PARSER_CONTEXT*     m_pContext;

    MemberArray         m_aMembersPack;     // �ṹ�����г�Ա��������������
    ArgumentsArray      m_aArgumentsPack;   // ���к��������������������

    FileDict            m_sIncludeFiles;    // �����ļ�����, ������Root parser
    Include*            m_pInclude;
  //MACRO::Set          m_MacrosSet;        // ʵ������
    CodeParser*         m_pSubParser;
    //MACRO_EXPAND_CONTEXT::List m_sMacroStack;
    TOKEN::List         m_ExpandedStream;   // ��չ����
  public:
    CodeParser(PARSER_CONTEXT* pContext, Include* pInclude);
    virtual ~CodeParser();
    b32                 Attach                  (const char* szExpression, clsize nSize, GXDWORD dwFlags, GXLPCWSTR szFilename);
    clsize              GenerateTokens          (CodeParser* pParent = NULL);
    GXBOOL              Parse                   ();

    const StatementArray& GetStatments          () const;

//    GXBOOL   IsToken(const SYNTAXNODE::UN* pUnion) const;
    //SYNTAXNODE::FLAGS TryGetNodeType(const SYNTAXNODE::UN* pUnion) const;

    void DbgDumpSyntaxTree(clStringArrayA* pArray, const SYNTAXNODE* pNode, int precedence, clStringA* pStr = NULL);


  };

  //////////////////////////////////////////////////////////////////////////

} // namespace UVShader

#endif // #ifndef _EXPRESSION_PARSER_H_