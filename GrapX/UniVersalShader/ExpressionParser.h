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

  struct TYPEDESC
  {
    enum TypeCate
    {
      TypeCate_Numeric,
      TypeCate_String,
      TypeCate_Struct,
    };

    GXLPCSTR  name;
    int       maxR : 8;    // Row, ���� array size
    int       maxC : 8;    // Column
    TypeCate  cate : 8;

    bool operator<(const TYPEDESC& t) const;
  };

  class NameSet
  {
  public:
    typedef clset<clStringA> StrSet;
    typedef clmap<clStringA, TYPEDESC> TypeMap;
    typedef clStringA::LPCSTR LPCSTR;
  
  protected:
    const NameSet* m_pParent;
    TypeMap  m_TypeMap;
    StrSet   m_VariableSet;

  public:
    NameSet();
    NameSet(const NameSet* pParent);

    void   Cleanup();
    GXBOOL RegisterType(LPCSTR szName, TYPEDESC::TypeCate cate);
    GXBOOL RegisterVariable(LPCSTR szName);
    GXBOOL RegisterType(const clStringA& strName, TYPEDESC::TypeCate cate);
    GXBOOL RegisterVariable(const clStringA& strName);

    GXBOOL HasType(LPCSTR szName) const;
    GXBOOL HasVariable(LPCSTR szName) const;
    const TYPEDESC* GetType(LPCSTR szName) const;
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
    //  //TKSCOPE::Array aArgs; // ʵ��

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

    enum FunctionStorageClass // �������Σ���ѡ
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

    struct FUNCTION_ARGUMENT // ��������
    {
      InputModifier eModifier;  // [opt]
      GXLPCSTR      szType;     // [req]
      GXLPCSTR      szName;     // [req]
      GXLPCSTR      szSemantic; // [opt]
    };
    typedef clvector<FUNCTION_ARGUMENT> ArgumentsArray;
    
    //////////////////////////////////////////////////////////////////////////
    struct STATEMENT;
    struct STATEMENT_EXPR;

    struct STATEMENT_FUNC // ������/��������
    {
      FunctionStorageClass eStorageClass; // [opt]
      GXLPCSTR     szReturnType;  // [req]
      GXLPCSTR     szName;        // [req]
      GXLPCSTR     szSemantic;    // [opt]
      FUNCTION_ARGUMENT*  pArguments;       // [opt]
      clsize              nNumOfArguments;  // [opt]
      STATEMENT* pExpression;
    };

    struct STATEMENT_STRU // �ṹ�嶨��
    {
      GXLPCSTR         szName;
      clsize           nNumOfMembers; // ���Ǳ�Ҫ��
      SYNTAXNODE::GLOB sRoot;
    };

    struct STATEMENT_EXPR // ���ʽ����
    {
      SYNTAXNODE::GLOB sRoot;
    };
    
    struct STATEMENT_DEFN
    {
      VariableStorageClass  storage_class;
      UniformModifier       modifier;
      GXLPCSTR              szType;
      SYNTAXNODE::GLOB      sRoot;
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
      const TKSCOPE*  pScope;   // �޶�����
      TKSCOPE::TYPE   begin;    // ��ʼ
      GXBOOL          bBeginMate;// scope begin ȡ��m_aTokens[begin].scope
      TKSCOPE::TYPE   end;
      GXBOOL          bEndMate;
      GXWCHAR         chTermin; // ���begin��������ս��������һ��begin==end��scope

      MAKESCOPE(){}
      MAKESCOPE(const TKSCOPE*_pScope, TKSCOPE::TYPE _begin, GXBOOL _bIndBegin, TKSCOPE::TYPE _end, GXBOOL _bIndEnd, GXWCHAR _chTermin)
        : pScope(_pScope), begin(_begin), bBeginMate(_bIndBegin), end(_end), bEndMate(_bIndEnd), chTermin(_chTermin) {}
      MAKESCOPE(const TKSCOPE& _scope, TKSCOPE::TYPE _begin, GXBOOL _bIndBegin, TKSCOPE::TYPE _end, GXBOOL _bIndEnd, GXWCHAR _chTermin)
        : pScope(&_scope), begin(_begin), bBeginMate(_bIndBegin), end(_end), bEndMate(_bIndEnd), chTermin(_chTermin) {}
    };

    struct PHONY_TOKEN
    {
      clStringA         str;          // ����ַ���
      clStringA::LPCSTR ori_marker;   // ԭʼ�ַ�����ַ
    };

    typedef clmap<int, PHONY_TOKEN> PhonyTokenDict_T;
    //////////////////////////////////////////////////////////////////////////

  protected:
    static u32 CALLBACK IteratorProc         (iterator& it, u32 nRemain, u32_ptr lParam);

    void    InitPacks();
    void    Cleanup();

    GXBOOL  ParseStatementAs_Definition(TKSCOPE* pScope);
    GXBOOL  ParseStatementAs_Function(TKSCOPE* pScope);
    GXBOOL  ParseFunctionArguments(STATEMENT* pStat, TKSCOPE* pArgScope);

    GXBOOL  ParseStatementAs_Struct(TKSCOPE* pScope);

    GXBOOL  ParseStatementAs_Expression(STATEMENT* pStat, TKSCOPE* pScope); // (������)��ʽ

    GXBOOL  CalculateValue(OPERAND& sOut, const SYNTAXNODE::GLOB* pDesc);

    SYNTAXNODE::GLOB* BreakDefinition(SYNTAXNODE::PtrList& sVarList, SYNTAXNODE* pNode); // ��ɢ�ṹ���Ա
    GXBOOL BreakDefinition(STATEMENT& stat, NameSet& sNameSet, const TKSCOPE& scope);

    GXBOOL  ParseExpression(SYNTAXNODE::GLOB& glob, const TKSCOPE& scope);
    GXBOOL  ParseToChain(SYNTAXNODE::GLOB& glob, const TKSCOPE& scope);
    GXBOOL  ParseCodeBlock(SYNTAXNODE::GLOB& glob, const TKSCOPE& scope);
    TKSCOPE::TYPE  TryParseSingle(SYNTAXNODE::GLOB& glob, const TKSCOPE& scope); // ����һ�������, һ���ؼ��ֱ��ʽ����һ�����ʽ

    GXBOOL  TryKeywords(const TKSCOPE& scope, SYNTAXNODE::GLOB* pDesc, TKSCOPE::TYPE* parse_end);
    TKSCOPE::TYPE  ParseFlowIf(const TKSCOPE& scope, SYNTAXNODE::GLOB* pDesc, GXBOOL bElseIf);
    TKSCOPE::TYPE  MakeFlowForScope(const TKSCOPE& scope, TKSCOPE* pInit, TKSCOPE* pCond, TKSCOPE* pIter, TKSCOPE* pBlock, SYNTAXNODE::GLOB* pBlockNode);
    TKSCOPE::TYPE  ParseFlowFor(const TKSCOPE& scope, SYNTAXNODE::GLOB* pDesc);
    TKSCOPE::TYPE  ParseFlowWhile(const TKSCOPE& scope, SYNTAXNODE::GLOB* pDesc);
    TKSCOPE::TYPE  ParseFlowDoWhile(const TKSCOPE& scope, SYNTAXNODE::GLOB* pDesc);
    TKSCOPE::TYPE  ParseStructDefine(const TKSCOPE& scope, SYNTAXNODE::GLOB* pDesc);
    GXBOOL  MakeScope(TKSCOPE* pOut, MAKESCOPE* pParam);
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
    void     PP_UserError(T_LPCSTR position, const clStringW& strText);
    GXBOOL   ExpandInnerMacro(TOKEN& token, const TOKEN& line_num); // ��Ҫ���滻__FILE__ __LINE__


    static T_LPCSTR Macro_SkipGaps( T_LPCSTR begin, T_LPCSTR end );  // ���������Ʊ���Ϳո����ַ�����ַ
    GXBOOL CompareString(T_LPCSTR str1, T_LPCSTR str2, size_t count);

    GXBOOL  ParseStatement(TKSCOPE* pScope);
    void    RelocaleStatements(StatementArray& aStatements);
    void    RelocalePointer();
    
    GXLPCSTR GetUniqueString(const TOKEN* pSym);
    const TYPEDESC* ParseType(const TOKEN& token);

    void OutputErrorW(GXUINT code, ...);  // �����һ����ЧtokenѰ���к�
    void OutputErrorW(const TOKEN& token, GXUINT code, ...);
    void OutputErrorW(T_LPCSTR ptr, GXUINT code, ...);

    CodeParser* GetRootParser();
    clBuffer* OpenIncludeFile(const clStringW& strFilename);

    const TYPEDESC* Verify_Type(const TOKEN& tkType);
    const TYPEDESC* Verify_Struct(const TOKEN& tkType, const NameSet* pNameSet);
    GXBOOL Verify_MacroFormalList(const MACRO_TOKEN::List& sFormalList);
    GXBOOL Verify_VariableDefinition(NameSet& sNameSet, const SYNTAXNODE& rNode);
    GXBOOL Verify2_VariableExpr(const TOKEN& tkType, const TYPEDESC* pType, const SYNTAXNODE& rNode);
    //GXBOOL Verify_FunctionBlock(const STATEMENT_EXPR& expr);
    GXBOOL Verify_Block(const SYNTAXNODE* pNode, const NameSet* pParentSet);
    GXBOOL Verify_StructMember(const SYNTAXNODE& rNode);

    const clStringA& InsertStableTokenString(int index, const clStringA& str);

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
    int                 m_nPPRecursion;     // ����Ԥ����ݹ����
    
    PARSER_CONTEXT*     m_pContext;
    PhonyTokenDict_T    m_PhonyTokenDict;   // �û����滻��token���ҵ�ԭʼtoken��Ϣ

    ArgumentsArray      m_aArgumentsPack;   // ���к��������������������

    FileDict            m_sIncludeFiles;    // �����ļ�����, ������Root parser
    Include*            m_pInclude;
  //MACRO::Set          m_MacrosSet;        // ʵ������
    CodeParser*         m_pSubParser;
    //MACRO_EXPAND_CONTEXT::List m_sMacroStack;
    TOKEN::List         m_ExpandedStream;   // ��չ����
    NameSet             m_GlobalSet;

  public:
    CodeParser(PARSER_CONTEXT* pContext, Include* pInclude);
    virtual ~CodeParser();
    b32                 Attach                  (const char* szExpression, clsize nSize, GXDWORD dwFlags, GXLPCWSTR szFilename);
    clsize              GenerateTokens          (CodeParser* pParent = NULL);
    GXBOOL              Parse                   ();

    const StatementArray& GetStatments          () const;

//    GXBOOL   IsToken(const SYNTAXNODE::UN* pUnion) const;
    //SYNTAXNODE::FLAGS TryGetNodeType(const SYNTAXNODE::UN* pUnion) const;

    void DbgDumpSyntaxTree(clStringArrayA* pArray, const SYNTAXNODE* pNode, int precedence, clStringA* pStr = NULL, int depth = 0);


  };

  //////////////////////////////////////////////////////////////////////////

} // namespace UVShader

#endif // #ifndef _EXPRESSION_PARSER_H_