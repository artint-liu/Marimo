#ifndef _EXPRESSION_PARSER_H_
#define _EXPRESSION_PARSER_H_

#define ENABLE_SYNTAX_VERIFY // �﷨��鿪��, ֻ�������﷨������ʱ������, ��ʽ�治Ҫ�ر����

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

  class CodeParser;

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

  //struct TYPEDESC
  //{
  //  enum TypeCate
  //  {
  //    TypeCate_Numeric,
  //    TypeCate_String,
  //    TypeCate_Struct,
  //  };

  //  GXLPCSTR  name;
  //  int       maxR : 8;    // Row, ���� array size
  //  int       maxC : 8;    // Column
  //  TypeCate  cate : 8;

  //  bool operator<(const TYPEDESC& t) const;
  //};

  enum KeywordFilter
  {
    KeywordFilter_break    = 0x0001,
    KeywordFilter_continue = 0x0002,
    KeywordFilter_case     = 0x0004,
    KeywordFilter_return   = 0x0008,    // �з���ֵ�ĺ����ſ������
    KeywordFilter_typedef  = 0x0010,

    KeywordFilter_All = 0xffffffff,
    KeywordFilter_InFuntionArgument = 0,
    KeywordFilter_InFuntion = KeywordFilter_typedef,
    //KeywordMask_FlowsControl,
  };

  struct TYPEDESC
  {
    enum TypeCate
    {
      TypeCate_Empty,
      TypeCate_Numeric,
      TypeCate_String,
      TypeCate_Struct,
    };

    TypeCate          cate;
    clStringA         name; // ������
    STRUCTDESC*       pDesc;
    const SYNTAXNODE* pMemberNode;

    GXBOOL GetMemberTypename(clStringA& strTypename, const TOKEN* ptkMember) const;
  };

  class NameContext
  {
  public:
    typedef clmap<clStringA, TYPEDESC>  TypeMap;
    typedef clmap<TokenPtr, const TYPEDESC*>  VariableMap;
    typedef clStringA::LPCSTR LPCSTR;

    enum State
    {
      State_Ok = 0,
      State_TypeNotFound = -1,     // û���ҵ�����
      State_DuplicatedType = -2,  // �ظ�ע������
      State_DuplicatedVariable = -3,
      State_DefineAsType = -4,    // �����Ѿ�������Ϊ����
      State_DefineAsVariable = -5,// �����Ѿ�������Ϊ����
    };
  
  protected:
    CodeParser* m_pCodeParser;
    const NameContext* m_pParent;

    TypeMap     m_TypeMap;
    VariableMap m_VariableMap;
    State       m_eLastState;

    GXBOOL TestIntrinsicType(TYPEDESC* pOut, const clStringA& strType);
    NameContext* GetRoot();

  public:
    NameContext();
    NameContext(const NameContext* pParent);
    NameContext(const NameContext& sParent);

    GXDWORD allow_keywords; // ���˵Ĺؼ���

    void SetParser(CodeParser* pCodeParser);
    void Cleanup();

    const TYPEDESC* GetType(const clStringA& strType) const;
    const TYPEDESC* GetVariable(const TOKEN* ptkName) const;
    State  TypeDefine(const TOKEN* ptkOriName, const TOKEN* ptkNewName);
    GXBOOL RegisterStruct(const TOKEN* ptkName, const SYNTAXNODE* pMemberNode);
    const TYPEDESC* RegisterVariable(const clStringA& strType, const TOKEN* ptrVariable);
    State  GetLastState() const;
    const TYPEDESC* GetMember(const SYNTAXNODE* pNode) const;
  };

  struct NODE_CALC : public SYNTAXNODE
  {
    //const TYPEDESC* GetMember(const NameSet& sNameSet) const;
    VALUE::State Calcuate(const NameContext& sNameSet, VALUE& value_out) const;
  };

  class CodeParser : public ArithmeticExpression
  {
    friend class NameContext;
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
      StatementType_Typedef,        // Typedef
      StatementType_Struct,         // �ṹ��
      StatementType_Signatures,     // ����shader���������ǵĽṹ��
      //StatementType_Expression,     // ���ʽ
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
      //GXLPCSTR      szType;     // [req]
      //GXLPCSTR      szName;     // [req]
      const TOKEN*  ptkType;    // [req]
      const TOKEN*  ptkName;    // [req]
      GXLPCSTR      szSemantic; // [opt]
    };
    typedef clvector<FUNCTION_ARGUMENT> ArgumentsArray;
    
    //////////////////////////////////////////////////////////////////////////
    struct STATEMENT_FUNC // ������/��������
    {
      FunctionStorageClass eStorageClass; // [opt]
      GXLPCSTR     szReturnType;  // [req]
      GXLPCSTR     szName;        // [req]
      GXLPCSTR     szSemantic;    // [opt]
      FUNCTION_ARGUMENT*  pArguments;       // [opt]
      clsize              nNumOfArguments;  // [opt]
    };

    struct STATEMENT_STRU // �ṹ�嶨��
    {
      GXLPCSTR         szName;
      clsize           nNumOfMembers; // ���Ǳ�Ҫ��
    };

    struct STATEMENT_DEFN
    {
      VariableStorageClass  storage_class;
      UniformModifier       modifier;
      GXLPCSTR              szType;
    };


    struct STATEMENT
    {
      StatementType type;
      SYNTAXNODE::GLOB sRoot;
      union
      {
        STATEMENT_FUNC func;
        STATEMENT_STRU stru;
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
      //clStringA         str;          // ����ַ���
      clStringA::LPCSTR szPhonyText;
      clsize            nPhonyLength;
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
    GXBOOL  ParseFunctionArguments(NameContext& sNameSet, STATEMENT* pStat, TKSCOPE* pArgScope);

    GXBOOL  ParseStatementAs_Typedef(TKSCOPE* pScope);
    GXBOOL  ParseStatementAs_Struct(TKSCOPE* pScope);

    //GXBOOL  ParseStatementAs_Expression(STATEMENT* pStat, NameSet& sNameSet, TKSCOPE* pScope); // (������)��ʽ

    GXBOOL  CalculateValue(OPERAND& sOut, const SYNTAXNODE::GLOB* pDesc);

    SYNTAXNODE* FlatDefinition(SYNTAXNODE* pThisChain);
    SYNTAXNODE::GLOB* BreakDefinition(SYNTAXNODE::PtrList& sVarList, SYNTAXNODE* pNode); // ��ɢ�ṹ���Ա

    GXBOOL  ParseExpression(SYNTAXNODE::GLOB& glob, NameContext& sNameSet, const TKSCOPE& scope);
    GXBOOL  ParseToChain(SYNTAXNODE::GLOB& glob, NameContext& sNameSet, const TKSCOPE& scope);
    GXBOOL  ParseCodeBlock(SYNTAXNODE::GLOB& glob, NameContext& sNameSet, const TKSCOPE& scope);
    TKSCOPE::TYPE  TryParseSingle(NameContext& sNameSet, SYNTAXNODE::GLOB& glob, const TKSCOPE& scope); // ����һ�������, һ���ؼ��ֱ��ʽ����һ�����ʽ

    GXBOOL  TryKeywords(NameContext& sNameSet, const TKSCOPE& scope, SYNTAXNODE::GLOB* pDest, TKSCOPE::TYPE* parse_end);
    TKSCOPE::TYPE  ParseFlowIf(const NameContext& sParentSet, const TKSCOPE& scope, SYNTAXNODE::GLOB* pDesc, GXBOOL bElseIf);
    TKSCOPE::TYPE  MakeFlowForScope(const TKSCOPE& scope, TKSCOPE* pInit, TKSCOPE* pCond, TKSCOPE* pIter, TKSCOPE* pBlock, SYNTAXNODE::GLOB* pBlockNode);
    TKSCOPE::TYPE  ParseFlowFor(const NameContext& sParentSet, const TKSCOPE& scope, SYNTAXNODE::GLOB* pDesc);
    TKSCOPE::TYPE  ParseFlowWhile(const NameContext& sParentSet, const TKSCOPE& scope, SYNTAXNODE::GLOB* pDesc);
    TKSCOPE::TYPE  ParseFlowDoWhile(const NameContext& sParentSet, const TKSCOPE& scope, SYNTAXNODE::GLOB* pDesc);
    TKSCOPE::TYPE  ParseTypedef(NameContext& sNameSet, const TKSCOPE& scope, SYNTAXNODE::GLOB* pDesc);
    TKSCOPE::TYPE  ParseStructDefinition(NameContext& sNameSet, const TKSCOPE& scope, SYNTAXNODE::GLOB* pMembers, SYNTAXNODE::GLOB* pDefinitions, int* pSignatures, int* pDefinition);

    //const TYPEDESC* GetMember(const NameSet& sNameSet, const SYNTAXNODE* pNode) const;
    //VALUE::State Calcuate(VALUE& value_out, const NameSet& sNameSet, const SYNTAXNODE* pNode) const;

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
    GXLPCSTR GetUniqueString(T_LPCSTR szText);
    const TYPEDESC* ParseType(const TOKEN& token);

    void OutputErrorW(GXUINT code, ...);  // �����һ����ЧtokenѰ���к�
    void OutputErrorW(const TOKEN& token, GXUINT code, ...);
    void OutputErrorW(T_LPCSTR ptr, GXUINT code, ...);

    CodeParser* GetRootParser();
    clBuffer* OpenIncludeFile(const clStringW& strFilename);

#ifdef ENABLE_SYNTAX_VERIFY
    //const TYPEDESC2* Verify_Type(const TOKEN& tkType);
    const TYPEDESC* Verify_Struct(const TOKEN& tkType, const NameContext* pNameSet);
    GXBOOL Verify_MacroFormalList(const MACRO_TOKEN::List& sFormalList);
    GXBOOL Verify_VariableDefinition(NameContext& sNameSet, const SYNTAXNODE& rNode);
    GXBOOL Verify2_VariableExpr(NameContext& sNameSet, const TOKEN& tkType, const TYPEDESC* pType, const SYNTAXNODE& rNode);
    //GXBOOL Verify_FunctionBlock(const STATEMENT_EXPR& expr);
    GXBOOL Verify_Block(const SYNTAXNODE* pNode, const NameContext* pParentSet);
    GXBOOL Verify_StructMember(const NameContext& sParentSet, const SYNTAXNODE& rNode);
    GXBOOL Verify2_LeftValue(const NameContext& sNameSet, const SYNTAXNODE::GLOB& left_glob, const TOKEN& opcode); // opcode ��Ҫ��Ϊ�˶�λ
#endif

    void SetTokenPhonyString(int index, const clStringA& str);

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
    NameContext         m_GlobalSet;

  public:
    CodeParser(PARSER_CONTEXT* pContext, Include* pInclude);
    virtual ~CodeParser();
    b32                 Attach                  (const char* szExpression, clsize nSize, GXDWORD dwFlags, GXLPCWSTR szFilename);
    clsize              GenerateTokens          (CodeParser* pParent = NULL);
    GXBOOL              Parse                   ();

    const StatementArray& GetStatements          () const;

//    GXBOOL   IsToken(const SYNTAXNODE::UN* pUnion) const;
    //SYNTAXNODE::FLAGS TryGetNodeType(const SYNTAXNODE::UN* pUnion) const;

    static void DbgDumpSyntaxTree(clStringArrayA* pArray, const SYNTAXNODE* pNode, int precedence, clStringA* pStr = NULL, int depth = 0);


  };

  //////////////////////////////////////////////////////////////////////////

} // namespace UVShader

#endif // #ifndef _EXPRESSION_PARSER_H_