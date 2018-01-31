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
    int       maxR : 8;    // Row, 或者 array size
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
    typedef clstack<int> MacroStack;        // 带形参宏所用的处理堆栈

    struct MACRO_TOKEN : public TOKEN
    {
      typedef cllist<MACRO_TOKEN> List;
      typedef clvector<MACRO_TOKEN> Array;

      MACRO_TOKEN(const TOKEN& t)
        : TOKEN(t)
        , formal_index(-1)
      {
      }

      int formal_index; // 对应的形参索引
    };

    struct MACRO
    {
      typedef clmap<clStringA, MACRO> Dict; // macro 支持重载，所以 key 的名字用 clStringA 来储存, Dict 要求具有排序性
      typedef clvector<int>           IntArray;
      //typedef clhash_set<clStringA>   Set;
      MACRO_TOKEN::Array aFormalParams;  // 形参
      MACRO_TOKEN::List aTokens;        // 替换内容
      //GXDWORD bTranslate     : 1; // 含有下面任意一个标记，需要转义
      //GXDWORD bHasLINE       : 1; // 有__LINE__宏, 这个宏是有变化的
      //GXDWORD bHasFILE       : 1; // 有__FILE__宏
      //GXDWORD bPoundSign     : 1; // #解析

      //MACRO();

      void set            (const Dict& dict, const TOKEN::Array& tokens, int begin_at);
      void clear          ();

      void ClearContainer (); // 这个用来清除iterator里的container，指向subparse容易出错
      //int  ExpandMacro    (const Dict& dict); // 展开宏
    };

    //struct MACRO_EXPAND_CONTEXT
    //{
    //  typedef clstack<MACRO_EXPAND_CONTEXT> Stack;
    //  typedef cllist<MACRO_EXPAND_CONTEXT>  List;
    //  CodeParser* pParser;
    //  MACRO*      pMacro;
    //  TOKEN::List current;

    //  //// ActualParam 实参
    //  //TOKEN::Array aTokens; // 实参列表
    //  //TKSCOPE::Array aArgs; // 实参

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

    enum AttachFlag // 注意: 与RTState公用标记位
    {
      AttachFlag_Preprocess     = 0x00000001, // 在预处理中，这个状态不再认为#为预处理开始标记
      AttachFlag_NotLoadMessage = 0x00010000, // TODO: 这个干掉？！
      AttachFlag_NotExpandMacro = 0x00020000, // 不展开宏
      AttachFlag_NotExpandCond  = 0x00040000, // 不展开defined()里面的宏
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

    enum FunctionStorageClass // 函数修饰，可选
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

    struct FUNCTION_ARGUMENT // 函数参数
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

    struct STATEMENT_FUNC // 函数体/函数声明
    {
      FunctionStorageClass eStorageClass; // [opt]
      GXLPCSTR     szReturnType;  // [req]
      GXLPCSTR     szName;        // [req]
      GXLPCSTR     szSemantic;    // [opt]
      FUNCTION_ARGUMENT*  pArguments;       // [opt]
      clsize              nNumOfArguments;  // [opt]
      STATEMENT* pExpression;
    };

    struct STATEMENT_STRU // 结构体定义
    {
      GXLPCSTR         szName;
      clsize           nNumOfMembers; // 不是必要的
      SYNTAXNODE::GLOB sRoot;
    };

    struct STATEMENT_EXPR // 表达式定义
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


    enum RTState // 运行时状态, 与AttachFlag公用标记位
    {
      State_InPreprocess = 0x00000001, // 解析预处理
    };


    struct OPERAND // TODO: 这个是临时结构体，以后放在源文件中，而不是头文件里
    {
      VALUE  v;
      const TOKEN* pToken; // 如果VALUE解析了token, pToken应该为空

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
      const TKSCOPE*  pScope;   // 限定区域
      TKSCOPE::TYPE   begin;    // 开始
      GXBOOL          bBeginMate;// scope begin 取自m_aTokens[begin].scope
      TKSCOPE::TYPE   end;
      GXBOOL          bEndMate;
      GXWCHAR         chTermin; // 如果begin遇到这个终结符，返回一个begin==end的scope

      MAKESCOPE(){}
      MAKESCOPE(const TKSCOPE*_pScope, TKSCOPE::TYPE _begin, GXBOOL _bIndBegin, TKSCOPE::TYPE _end, GXBOOL _bIndEnd, GXWCHAR _chTermin)
        : pScope(_pScope), begin(_begin), bBeginMate(_bIndBegin), end(_end), bEndMate(_bIndEnd), chTermin(_chTermin) {}
      MAKESCOPE(const TKSCOPE& _scope, TKSCOPE::TYPE _begin, GXBOOL _bIndBegin, TKSCOPE::TYPE _end, GXBOOL _bIndEnd, GXWCHAR _chTermin)
        : pScope(&_scope), begin(_begin), bBeginMate(_bIndBegin), end(_end), bEndMate(_bIndEnd), chTermin(_chTermin) {}
    };

    struct PHONY_TOKEN
    {
      clStringA         str;          // 替代字符串
      clStringA::LPCSTR ori_marker;   // 原始字符串地址
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

    GXBOOL  ParseStatementAs_Expression(STATEMENT* pStat, TKSCOPE* pScope); // (算数表)达式

    GXBOOL  CalculateValue(OPERAND& sOut, const SYNTAXNODE::GLOB* pDesc);

    SYNTAXNODE::GLOB* BreakDefinition(SYNTAXNODE::PtrList& sVarList, SYNTAXNODE* pNode); // 分散结构体成员
    GXBOOL BreakDefinition(STATEMENT& stat, NameSet& sNameSet, const TKSCOPE& scope);

    GXBOOL  ParseExpression(SYNTAXNODE::GLOB& glob, const TKSCOPE& scope);
    GXBOOL  ParseToChain(SYNTAXNODE::GLOB& glob, const TKSCOPE& scope);
    GXBOOL  ParseCodeBlock(SYNTAXNODE::GLOB& glob, const TKSCOPE& scope);
    TKSCOPE::TYPE  TryParseSingle(SYNTAXNODE::GLOB& glob, const TKSCOPE& scope); // 解析一个代码块, 一条关键字表达式或者一条表达式

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
    T_LPCSTR PP_IfDefine(const RTPPCONTEXT& ctx, GXBOOL bNot, const TOKEN::Array& aTokens); // bNot 表示 if not define
    T_LPCSTR PP_If(const RTPPCONTEXT& ctx, CodeParser* pParser);
    T_LPCSTR PP_SkipConditionalBlock(PPCondRank session, T_LPCSTR begin, T_LPCSTR end); // 从这条预处理的行尾开始，跳过这块预处理，begin应该是当前预处理的结尾
    void     PP_UserError(T_LPCSTR position, const clStringW& strText);
    GXBOOL   ExpandInnerMacro(TOKEN& token, const TOKEN& line_num); // 主要是替换__FILE__ __LINE__


    static T_LPCSTR Macro_SkipGaps( T_LPCSTR begin, T_LPCSTR end );  // 返回跳过制表符和空格后的字符串地址
    GXBOOL CompareString(T_LPCSTR str1, T_LPCSTR str2, size_t count);

    GXBOOL  ParseStatement(TKSCOPE* pScope);
    void    RelocaleStatements(StatementArray& aStatements);
    void    RelocalePointer();
    
    GXLPCSTR GetUniqueString(const TOKEN* pSym);
    const TYPEDESC* ParseType(const TOKEN& token);

    void OutputErrorW(GXUINT code, ...);  // 从最后一个有效token寻找行号
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
    GXDWORD             m_dwState;          // 内部状态, 参考RTState
    CodeParser*         m_pParent;
    TypeSet             m_TypeSet;
    StatementArray      m_aStatements;
    StatementArray      m_aSubStatements;   // m_aStatements的次级储存，没有顺序关系
    int                 m_nPPRecursion;     // 条件预处理递归次数
    
    PARSER_CONTEXT*     m_pContext;
    PhonyTokenDict_T    m_PhonyTokenDict;   // 用户从替换的token中找到原始token信息

    ArgumentsArray      m_aArgumentsPack;   // 所有函数参数都存在这个表里

    FileDict            m_sIncludeFiles;    // 包含文件集合, 仅限于Root parser
    Include*            m_pInclude;
  //MACRO::Set          m_MacrosSet;        // 实名集合
    CodeParser*         m_pSubParser;
    //MACRO_EXPAND_CONTEXT::List m_sMacroStack;
    TOKEN::List         m_ExpandedStream;   // 宏展开流
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