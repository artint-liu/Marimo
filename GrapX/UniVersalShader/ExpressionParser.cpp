#include <grapx.h>
#include <clUtility.h>
#include <Smart/SmartStream.h>
#include <clTokens.h>
#include <clStringSet.h>
#include <clStablePool.h>
#include "ArithmeticExpression.h"
#include "ExpressionParser.h"
#include "UVShaderError.h"

#include "clPathFile.h"
#include "clTextLines.h"
#include "../User/DataPoolErrorMsg.h"

#define PARSER_BREAK(_GLOB) DbgBreak(_GLOB)
#define PARSER_BREAK2(PARSER, _GLOB) PARSER->DbgBreak(_GLOB)
#define PARSER_ASSERT(_X, _GLOB) DbgAssert(_X, _GLOB)
#define IS_SCALER_CATE(_CATE) (\
  _CATE->cate == TYPEDESC::TypeCate_FloatScaler || \
  _CATE->cate == TYPEDESC::TypeCate_IntegerScaler)

#define IS_VECTOR_CATE(_CATE) (_CATE->cate == TYPEDESC::TypeCate_Vector)
#define IS_MATRIX_CATE(_CATE) (_CATE->cate == TYPEDESC::TypeCate_Matrix)

#define IS_VECMAT_CATE(_CATE) (\
  _CATE->cate == TYPEDESC::TypeCate_Vector || \
  _CATE->cate == TYPEDESC::TypeCate_Matrix)

#define IS_STRUCT_CATE(_CATE) (\
  _CATE->cate == TYPEDESC::TypeCate_Vector || \
  _CATE->cate == TYPEDESC::TypeCate_Matrix || \
  _CATE->cate == TYPEDESC::TypeCate_Struct)

#define IS_SAMPLER_CATE(_CATE) (\
  _CATE == TYPEDESC::TypeCate_Sampler1D || \
  _CATE == TYPEDESC::TypeCate_Sampler2D || \
  _CATE == TYPEDESC::TypeCate_Sampler3D || \
  _CATE == TYPEDESC::TypeCate_SamplerCube ) 

#define REDUCE_ERROR_MESSAGE

//#define VOID_TYPEDESC ((const TYPEDESC*)-1)
#define ERROR_TYPEDESC ((const TYPEDESC*)-1)
#define PARSER_NOTIMPLEMENT TRACE("%s(%d):没咋处理的地方\n", __FILE__, __LINE__)

// TODO:
// 1.float3(0) => float3(0,0,0)
// 2.返回值未完全初始化
// 3.code block 表达式中如果中间语句缺少分号不报错


//////////////////////////////////////////////////////////////////////////
//
// 函数定义
//
//例：float4 VertexShader_Tutorial_1(float4 inPos : POSITION ) : POSITION
//
//  A function declaration contains:
//
//  A return type 
//  A function name 
//  An argument list (optional) 
//  An output semantic (optional) 
//  An annotation (optional) 
//
//////////////////////////////////////////////////////////////////////////
//
// 用户自定义警告(https://support.microsoft.com/zh-cn/kb/155196)
//
//#define __STR2__(x) #x
//#define __STR1__(x) __STR2__(x)
//#define __LOC__ __FILE__ "("__STR1__(__LINE__)") : Warning Msg: "
//#pragma message(__LOC__"Need to do 3D collision testing")
//

#if defined(UVS_EXPORT_TEXT_IS_SIGN)
GXLPCSTR g_ExportErrorMessage1 = __FILE__;
#endif

// 步进符号指针，但是不期望遇到结尾指针
#define INC_BUT_NOT_END(_P, _END) \
  if(++_P >= _END) {  \
    return FALSE;     \
  }

#define INC_AND_END(_P, _END) \
  if(++_P != _END) {  \
    return FALSE;     \
  }

#define OUT_OF_SCOPE(s) (s == (clsize)-1)

//static clsize s_nMultiByteOperatorLen = 0; // 最大长度

#define PREPROCESS_define   "define"
#define PREPROCESS_defined  "defined"
#define PREPROCESS_if       "if"
#define PREPROCESS_ifndef   "ifndef"
#define PREPROCESS_ifdef    "ifdef"
#define PREPROCESS_elif     "elif"
#define PREPROCESS_else     "else"
#define PREPROCESS_endif    "endif"
#define PREPROCESS_undef    "undef"
#define PREPROCESS_include  "include"
#define PREPROCESS_line     "line"
#define PREPROCESS_file     "file"
#define PREPROCESS_pragma   "pragma"
#define PREPROCESS_message  "message"
#define PREPROCESS_error    "error"
#define PREPROCESS_pound    '#'
#define MACRO_FILE          "__FILE__"
#define MACRO_LINE          "__LINE__"

namespace UVShader
{
  TOKEN::T_LPCSTR s_szString = "string";
  TOKEN::T_LPCSTR s_szSampler1D = "sampler1D";
  TOKEN::T_LPCSTR s_szSampler2D = "sampler2D";
  TOKEN::T_LPCSTR s_szSampler3D = "sampler3D";
  TOKEN::T_LPCSTR s_szSamplerCube = "samplerCUBE";

  TOKEN::T_LPCSTR s_szLengthFunc = "length";
  TOKEN::T_LPCSTR s_szMultiplicationFunc = "mul";
  static GXLPCSTR s_szNCName_Block = "{";  // Name Context 代码块名

  extern GXLPCSTR STR_VOID;

  extern GXLPCSTR STR_INT;
  extern GXLPCSTR STR_UINT;
  extern GXLPCSTR STR_HALF;
  extern GXLPCSTR STR_BOOL;
  extern GXLPCSTR STR_FLOAT;
  extern GXLPCSTR STR_FLOAT2;
  extern GXLPCSTR STR_FLOAT3;
  extern GXLPCSTR STR_FLOAT4;

  extern GXLPCSTR STR_DOUBLE;

  extern GXLPCSTR STR_FLOAT2x2;
  extern GXLPCSTR STR_FLOAT2x3;
  extern GXLPCSTR STR_FLOAT2x4;
  extern GXLPCSTR STR_FLOAT3x2;
  extern GXLPCSTR STR_FLOAT3x3;
  extern GXLPCSTR STR_FLOAT3x4;
  extern GXLPCSTR STR_FLOAT4x2;
  extern GXLPCSTR STR_FLOAT4x3;
  extern GXLPCSTR STR_FLOAT4x4;
  
  extern GXLPCSTR STR_DOUBLE;
  extern GXLPCSTR STR_DOUBLE2;
  extern GXLPCSTR STR_DOUBLE3;
  extern GXLPCSTR STR_DOUBLE4;
  
  extern GXLPCSTR STR_DOUBLE2x2;
  extern GXLPCSTR STR_DOUBLE2x3;
  extern GXLPCSTR STR_DOUBLE2x4;
  extern GXLPCSTR STR_DOUBLE3x2;
  extern GXLPCSTR STR_DOUBLE3x3;
  extern GXLPCSTR STR_DOUBLE3x4;
  extern GXLPCSTR STR_DOUBLE4x2;
  extern GXLPCSTR STR_DOUBLE4x3;
  extern GXLPCSTR STR_DOUBLE4x4;

  const size_t STR_INT_LENGTH = clstd::strlenT(STR_INT);
  const size_t STR_UINT_LENGTH = clstd::strlenT(STR_UINT);
  const size_t STR_HALF_LENGTH = clstd::strlenT(STR_HALF);
  const size_t STR_FLOAT_LENGTH  = clstd::strlenT(STR_FLOAT);
  const size_t STR_DOUBLE_LENGTH = clstd::strlenT(STR_DOUBLE);

  static b32 s_bParserBreak = TRUE;
  static b32 s_bParserAssert = TRUE;
  static b32 s_bDumpSyntaxTree = TRUE;

  //////////////////////////////////////////////////////////////////////////
#if 0
  enum GrammarCategory // 类别
  {
    GrammarCategory_DontCare = 0, // 不太关心
    GrammarCategory_Known,        // 已知类别, 必须在name中指定
    GrammarCategory_Type,         // 变量类型
    GrammarCategory_Variable,     // 变量
    GrammarCategory_Child,
    GrammarCategory_End,
  };

  struct GRAMMAR_ELEMENT
  {
    GrammarCategory cate;     // 语法类别
    GXLPCSTR        name;     // 指定已知类别的名字
    GXBOOL          optional; // 可选类型，不一定存在
    GRAMMAR_ELEMENT*child;    // 子类别
  };

  GRAMMAR_ELEMENT stru_var[] = {
    {GrammarCategory_Variable, NULL, 0, NULL},
    {GrammarCategory_Known, ",", 0, NULL},
    {GrammarCategory_End,},
  };

  GRAMMAR_ELEMENT stru_member[] = {
    {GrammarCategory_Type, NULL, 0, NULL},
    {GrammarCategory_End,},
  };
#endif

#if 0
  const CTokens::T_LPCSTR c_StorageClass =
    "extern\0nointerpolation\0precise\0shared\0groupshared\0static\0uniform\0volatile\0";

  const CTokens::T_LPCSTR c_TypeModifier = "const\0row_major\0column_major\0";

  enum StreamCategory
  {
    StreamCategory_DontCare = 0,      // 不关心
    StreamCategory_Known,             // 已知的Token, 在name中指定
    StreamCategory_KnownList,         // 已知TokenList, 在name中指定, 用"\0"分割
    StreamCategory_OneToken,          // 任意一个Token
    StreamCategory_SomeTokens,        // 任意多个Token
    StreamCategory_OpenBracket,       // 开括号"(", "[", "{", 具体在name中指定
    StreamCategory_CloseBracket,      // 闭括号")", "]", "}"
    StreamCategory_End,               // 结束符号
  };

  struct EXPSTREAM
  {
    StreamCategory  cate;
    CTokens::T_LPCSTR name;
  };
 
  // [Storage_Class] [Type_Modifier] Type Name[Index] 
  const EXPSTREAM s_Steam_LeftDefVariable1[] = {
    {StreamCategory_KnownList, c_StorageClass},
    {StreamCategory_KnownList, c_TypeModifier},
    {StreamCategory_OneToken, NULL},
    {StreamCategory_OneToken, NULL},
    {StreamCategory_OpenBracket, "["},
    {StreamCategory_SomeTokens, NULL},
    {StreamCategory_CloseBracket, "]"},
    {StreamCategory_End, NULL},
  };

  // [Storage_Class] [Type_Modifier] Type Name
  const EXPSTREAM s_Steam_LeftDefVariable2[] = {
    {StreamCategory_KnownList, c_StorageClass},
    {StreamCategory_KnownList, c_TypeModifier},
    {StreamCategory_OneToken, NULL},
    {StreamCategory_OneToken, NULL},
    {StreamCategory_End, NULL},
  };
#endif

  //////////////////////////////////////////////////////////////////////////
  extern COMMINTRTYPEDESC s_aIntrinsicStruct[];
  extern COMMINTRTYPEDESC s_aBaseType[];
  extern INTRINSIC_FUNC s_functions[];

  const PERCOMPONENTMATH* FindPerComponentMathOperations(GXLPCSTR szName)
  {
    extern PERCOMPONENTMATH s_PreComponentMath[];
    for(int i = 0; s_PreComponentMath[i].name != NULL; i++) {
      if(clstd::strcmpT(szName, s_PreComponentMath[i].name) == 0) {
        return &s_PreComponentMath[i];
      }
    }
    return NULL;
  }



  DefaultInclude s_DefaultInclude;

  // 构造函数
  CodeParser::CodeParser()
    : m_pContext(NULL)
    , m_pSubParser(NULL)
    , m_dwState(0)
    , m_pParent(NULL)
    , m_nPPRecursion(0)
    , m_pInclude(&s_DefaultInclude)
    , m_GlobalSet("$")
  {
    m_GlobalSet.allow_keywords = KeywordFilter_typedef;
    m_GlobalSet.SetParser(this);
    m_GlobalSet.BuildIntrinsicType();
    InitPacks();
  }

  CodeParser::CodeParser(CodeParser* pParent, PARSER_CONTEXT* pContext, Include* pInclude)
    : m_pContext(pContext)
    , m_pSubParser(NULL)
    , m_dwState(0)
    , m_pParent(pParent)
    , m_nPPRecursion(0)
    , m_pInclude(pInclude ? pInclude : &s_DefaultInclude)
    , m_GlobalSet("$")
  {
    //m_GlobalSet.allow_keywords = KeywordFilter_typedef; // TODO: 去掉？
    //m_GlobalSet.SetParser(this);
    //m_GlobalSet.BuildIntrinsicType(); // TODO: 去掉？
    if(pContext) {
      pContext->nRefCount++;
    }

    if(pContext && ! pContext->Macros.empty())
    {
      int n = 0;
      for(auto it = pContext->Macros.begin(); it != pContext->Macros.end(); ++it, n++)
      {
        ASSERT(it->second.nOrder == n); // 必须指定宏的order
        ASSERT(it->second.nNumTokens > 0); // 肯定大于0啊
      }
    }

    //InitPacks(); // TODO: 去掉？

    // 子解析结构，用来分解预处理语句，所以这里不初始化变量类型相关的结构体
    m_pLogger = pParent->GetLogger();
    m_pLogger->AddRef();
  }



  CodeParser::~CodeParser()
  {
    if(m_pContext && _CL_NOT_(--m_pContext->nRefCount)) {
      delete m_pContext;
    }
    m_pContext = NULL;

    // 如果是子解析器，这里借用了父对象的信息定位，退出时要清空，避免析构时删除
    SAFE_DELETE(m_pSubParser);
    SAFE_RELEASE(m_pLogger);

    // 释放包含文件缓存
    ASSERT( ! m_pParent || (m_pParent && m_sIncludeFiles.empty())); // 只有 root parser 才有包含文件的缓存
    for(auto it = m_sIncludeFiles.begin();
      it != m_sIncludeFiles.end(); ++it) {
      m_pInclude->Close(it->second);
    }
  }

  void CodeParser::InitPacks()
  {
    //
    // 所有pack类型，都存一个空值，避免记录0索引与NULL指针混淆的问题
    //
    m_NodePool.Clear();

    FUNCTION_ARGUMENT argument = {InputModifier_in};
    m_aArgumentsPack.clear();
    m_aArgumentsPack.push_back(argument);

    STATEMENT stat = {StatementType_Empty};
    m_aSubStatements.push_back(stat);
  }

  void CodeParser::Reset()
  {
    m_ValueDict.clear();
    m_PhonyTokenDict.clear();
    m_GlobalSet.Reset();
    m_aTokens.clear();
    m_aStatements.clear();
    if(m_pLogger) {
      m_pLogger->Reset();
    }

    SAFE_DELETE(m_pSubParser);
    InitPacks();
  }

  b32 CodeParser::Attach(const char* szExpression, clsize nSize, GXDWORD dwFlags, GXLPCWSTR szFilename)
  {
    Reset();
    if( ! m_pContext) {
      m_pContext = new PARSER_CONTEXT;
      m_pContext->nRefCount = 1;
    }

    m_dwState = dwFlags;
    if(m_pLogger == NULL) {
      m_pLogger = new CLogger();
    }

    if(m_pParent == NULL) {
      m_pLogger->Initialize(szExpression, nSize, szFilename);
    }
    //if(TEST_FLAG_NOT(dwFlags, AttachFlag_NotLoadMessage))
    //{
    //  if( ! m_pMsg) {
    //    m_pMsg = ErrorMessage::Create();
    //    m_pMsg->LoadErrorMessage(L"uvsmsg.txt");
    //    m_pMsg->SetMessageSign('C');
    //    m_pMsg->PushFile(szFilename);
    //  }
    //  m_pMsg->GenerateCurLines(szExpression, nSize);
    //}
    return CTokens::Attach(szExpression, nSize);
  }

  //////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////

  u32 CodeParser::StepIterator( ArithmeticExpression::iterator& it)
  {
    GXBOOL bENotation = FALSE;
    size_t remain = m_pEnd - it.marker;

    if(it.marker[0] == '#' && TEST_FLAG_NOT(((CodeParser*)it.pContainer)->m_dwState, AttachFlag_Preprocess))
    {
      CodeParser* pThis = (CodeParser*)it.pContainer;

      // 测试是否已经在预处理中
      if(TEST_FLAG(pThis->m_dwState, State_InPreprocess)) {
        return 0;
      }
      SET_FLAG(pThis->m_dwState, State_InPreprocess);

      if( ! TOKENSUTILITY::IsHeadOfLine(it.pContainer, it.marker)) {
        //ERROR_MSG_C2014_预处理器命令必须作为第一个非空白空间启动;
        GetLogger()->OutputErrorW(it->marker, UVS_EXPORT_TEXT(2014, "预处理器命令必须作为第一个非空白空间启动"));
        return 0;
      }

      RTPPCONTEXT ctx;
      ctx.iter_next = it;
      SmartStreamUtility::ExtendToNewLine(ctx.iter_next, 1, remain, 0x0001);
      
      // ppend 与 iter_next.marker 之间可能存在空白，大量注释等信息，为了
      // 减少预处理解析的工作量，这里预先存好 ppend 再步进 iter_next
      ctx.ppend = ctx.iter_next.end();
      ctx.stream_end = pThis->GetStreamPtr() + pThis->GetStreamCount();
      ++ctx.iter_next;

      if(++it >= ctx.iter_next) { // 只有一个'#', 直接跳过
        RESET_FLAG(pThis->m_dwState, State_InPreprocess);
        return 0;
      }

      // define
      // if, ifndef, ifdef, elif, else, endif, undef
      // include
      // line, file
      // pragma
      // error

      if(it == PREPROCESS_else)
      {
        it.marker = pThis->PP_SkipConditionalBlock(PPCondRank_else, ctx.iter_next.marker, ctx.stream_end);
        it.length = 0;
      }
      else if(it == PREPROCESS_elif)
      {
        it.marker = pThis->PP_SkipConditionalBlock(PPCondRank_elif, ctx.iter_next.marker, ctx.stream_end);
        it.length = 0;
      }
      else if(it == PREPROCESS_endif)
      {
        if(pThis->m_nPPRecursion <= 0) {
          GetLogger()->OutputErrorW(it.marker, UVS_EXPORT_TEXT(1020, "意外的 #endif"));
          return 0;
        }

        pThis->m_nPPRecursion--;
        it = ctx.iter_next;
      }
      else if(it == PREPROCESS_error)
      {
        clStringW str(it.marker + it.length, ctx.ppend - (it.marker + it.length));
        pThis->PP_UserError(it.marker, str);
      }
      else
      {
        // DoPreprocess 处理中，先将命令分解到子tokens集合，再进行解析
        auto next_marker = pThis->DoPreprocess(ctx, it.marker, ctx.ppend);
        if(next_marker == ctx.iter_next.marker) {
          it = ctx.iter_next;

          // 防止进入MultiByteOperatorProc，因为这个函数要求it是无效的
          if(TEST_FLAG(m_aCharSem[it.marker[0]], M_CALLBACK)) {
            RESET_FLAG(pThis->m_dwState, State_InPreprocess);
            return 0;
          }
        }
        else //if(next_marker != ctx.stream_end)
        {
          it.marker = next_marker;
          it.length = 0;
        }
        ASSERT(it.marker >= ctx.iter_next.marker);
      }//*/


      // 如果在预处理中步进iterator则会将#当作一般符号处理
      // 这里判断如果下一个token仍然是#则置为0，这样会重新激活迭代器处理当前这个预处理
      if(it == '#') {
        it.length = 0;
      }

      RESET_FLAG(pThis->m_dwState, State_InPreprocess);
    }
    else if((it.marker[0] == '\\' && remain > 0 && it.marker[1] == '\n') ||
      (it.marker[0] == '\\' && remain > 1 && it.marker[1] == '\r' && it.marker[2] == '\n'))
    {
      ++it;
    }
    else
    {
      ArithmeticExpression::StepIterator(it);
    }

    if(TEST_FLAG(m_aCharSem[it.marker[0]], M_CALLBACK)) {
      return MultiByteOperatorProc(it, remain);
    }

    ASSERT((int)remain >= 0);
    return 0;
  }

  clsize CodeParser::GenerateTokens()
  {
    CodeParser* pPrevParent = m_pParent;
    //m_pParent = pParent;
    ArithmeticExpression::iterator stream_end = end();
    ASSERT(m_aTokens.empty()); // 调用者负责清空

    //if( ! m_pMsg && pParent) {
    //  m_bRefMsg = TRUE;
    //  m_pMsg = pParent->m_pMsg;
    //}
    //if(pParent)
    //{
    //  m_pLogger = pParent->GetLogger();
    //  m_pLogger->AddRef();
    //}


    m_aTokens.reserve(EstimateForTokensCount());
    PairStack sStack[countof(s_PairMark)];


    ArithmeticExpression::iterator it = begin();
    //token.Set(it);

    int EOE = m_aTokens.size(); // End Of Expression
    cllist<clsize> UnaryPendingList; // "++", "--" 未确定前置还是后缀的列表

    for(; /*it.pContainer == NULL || */((size_t)it.pContainer & 1) || it != stream_end; GetNext(it))
    {
      // 上一个预处理结束后，后面的语句长度设置为0可以回到主循环步进
      // 这样做是为了避免如果下一条语句也是预处理指令，不会在处理回调中发生递归调用
      if(it.length == 0) {
        continue;
      }

      if( ! m_pParent && *it == PREPROCESS_pound) {
        GetLogger()->OutputErrorW(*it, UVS_EXPORT_TEXT(2121, "“#”: 无效字符 : 可能是宏展开的结果")); // 无效的"#"号_可能是宏扩展
      }

      const int c_size = (int)m_aTokens.size();

      
      if(c_size > 0)
      {
        if(it.marker[0] == '-' || it.marker[0] == '+')
        {
          ASSERT(it == '-' || it == '+' || it == "--" || it == "++" || it == "+=" || it == "-=");
          const auto& l_back = m_aTokens.back();

          // 如果是 -,+ 检查前一个符号是不是操作符或者括号，如果是就认为这个 -,+ 是正负号
          if(it.length == 1)
          {
            // 一元操作符，+/-就不转换为正负号
            // '}' 就不判断了 { something } - abc 这种格式应该是语法错误
            if(l_back == "return" ||
              (l_back.precedence != 0 && l_back != ')' && l_back != ']' && (!l_back.unary))) {
              const auto& p = s_plus_minus[(int)(it.marker[0] - '+')];
              it->SetArithOperatorInfo(p);
            }
          }
          else if(it.length == 2 && (it.marker[1] == '-' || it.marker[1] == '+'))
          {
            // "++","--" 默认按照前缀操作符处理, 这里检查是否转换为后缀操作符
            if(l_back.IsIdentifier()) {
              const auto& p = s_UnaryLeftOperand[(int)(it.marker[0] - '+')];
              it->SetArithOperatorInfo(p);
            }
            else {
              UnaryPendingList.push_back(c_size);
            }
          }
        }
      }
      
      // 只是清理
      //l_token.ClearMarker();
      //l_token.ClearArithOperatorInfo();


      // 符号配对处理
      // ...={...}这种情况不更新EOE
      //if(MarryBracket(sStack, token) && m_aTokens.back() != "=" &&
      //  _CL_NOT_(CompareToken(token.scope - 1, "=")))
      MarryBracket(sStack, *it, TEST_FLAG(m_dwState, AttachFlag_Preprocess));
      
      if(OnToken(*it)) {
        it->ClearMarker();
      }

      // 可能被宏展开后清除
      if(it->marker)
      {
        if(_CL_NOT_(MergeStringToken(*it)))
        {
          m_aTokens.push_back(*it);

          if(*it == "false")
          {
            m_aTokens.back().type = TOKEN::TokenType_Integer;
            SetTokenPhonyString(m_aTokens.size() - 1, "0");
          }
          else if(*it == "true")
          {
            m_aTokens.back().type = TOKEN::TokenType_Integer;
            SetTokenPhonyString(m_aTokens.size() - 1, "1");
          }
          else if(*it == ';') {
            m_aTokens.back().type = TOKEN::TokenType_Semicolon;
          }
        }
      }

      if(it == ';') {
        ASSERT(EOE < (int)m_aTokens.size());
        int brace_depth = 0;

        // 分号作用域不能穿过"{", 但是可以穿过匹配并且内部不含分号的"{","}"
        for(TOKEN* p = &m_aTokens.back(); p >= &m_aTokens.front() + EOE; p--)
        {
          ASSERT(p->semi_scope == -1); // 如果非-1，则说明被覆盖
          if(*p == '}') {
            brace_depth++;
          }
          else if(*p == '{') {
            brace_depth--;
            if(brace_depth < 0) {
              break;
            }
          }
          p->semi_scope = c_size;
        }
        EOE = c_size + 1;
      }
    }

    if(m_nPPRecursion != 0) {
      // 从最后一个有效token寻找行号
      const char* ptr = NULL;
      for(auto it = m_aTokens.rbegin(); it != m_aTokens.rend(); ++it) {
        if(it->pContainer) {
          ptr = it->marker;
          break;
        }
      }
      GetLogger()->OutputErrorW(ptr, UVS_EXPORT_TEXT(1070, "“%s”文件中的 #if/#endif 对不匹配"), GetLogger()->GetFilenameW());
    }

    // 检查括号匹配
    if(TEST_FLAG_NOT(m_dwState, AttachFlag_Preprocess))
    {
      for(int i = 0; i < countof(s_PairMark); ++i)
      {
        PairStack& s = sStack[i];
        if(!s.empty()) {
          // ERROR: 闭括号不匹配
          GetLogger()->OutputErrorW(m_aTokens[s.top()], UVS_EXPORT_TEXT(5001, "文件异常结尾, 缺少闭括号."));
          //ERROR_MSG__MISSING_CLOSEDBRACKET;
        }
      }
    }

    // 对未确定的"++","--"重新确定
    for(auto it = UnaryPendingList.begin(); it != UnaryPendingList.end(); ++it)
    {
      clsize n = *it;
      if(n + 1 >= m_aTokens.size()) {
        // 后缀操作符, 但是语法有错误
        // 忘了这个是干啥用的了 先中断吧
        CLBREAK;
        // token.SetArithOperatorInfo(s_UnaryLeftOperand[(int)(m_aTokens[n].marker[0] - '+')]);
      }
      //else if(m_aTokens[n + 1].IsIdentifier()) {
      //  // 前缀操作符, 不做处理
      //}
    }


    //// 如果是子解析器，这里借用了父对象的信息定位，退出时要清空，避免析构时删除
    //if(pParent) {
    //  m_pMsg = NULL;
    //}
    m_pParent = pPrevParent;
    return m_aTokens.size();
  }

  GXBOOL CodeParser::MergeStringToken(const TOKEN& token)
  {
    if(m_aTokens.empty()) {
      return FALSE;
    }

    TOKEN& last_one = m_aTokens.back();

    if(token.type == TOKEN::TokenType_String && last_one.type == TOKEN::TokenType_String)
    {
      // 合并代码中原有字符串token，这样保持了原文的空白信息
      if((last_one.pContainer == token.pContainer) && last_one.marker < token.marker &&
        ! last_one.BeginsWith('\"') && ! last_one.EndsWith('\"') && ! token.BeginsWith('\"') && ! token.EndsWith('\"'))
      {
        ASSERT(last_one.marker < m_pEnd && token.marker < m_pEnd);
        last_one.length = (clsize)token.marker + token.length - (clsize)last_one.marker;
      }
      else {
        //PHONY_TOKEN pt;
        const TOKEN* aTonkens[] = {&last_one, &token};
        //auto& emplace = m_PhonyTokenDict.emplace(m_aTokens.size() - 1);
        //PHONY_TOKEN& pt = emplace.first->second;
        clStringA str;

        // TODO: 优化处理分号
        for(int i = 0; i < 2; i++)
        {
          if(aTonkens[i]->BeginsWith('\"')) {
            str.Append(aTonkens[i]->marker + 1, aTonkens[i]->length - 1);
          }
          else {
            str.Append(aTonkens[i]->marker, aTonkens[i]->length);
          }

          if(str.EndsWith('\"')) {
            str.Remove(str.GetLength() - 1, 1);
          }
        }

        SetTokenPhonyString(m_aTokens.size() - 1, str);
      }
#ifdef ENABLE_STRINGED_SYMBOL
      last_one.symbol = last_one.ToString();
#endif // #ifdef ENABLE_STRINGED_SYMBOL
      return TRUE;
    }
    return FALSE;
  }

  void CodeParser::SetRepalcedValue(const GLOB& glob, const VALUE& value)
  {
    if(glob.IsToken()) {
      if(glob.pTokn->type > TOKEN::TokenType_FirstNumeric && glob.pTokn->type < TOKEN::TokenType_LastNumeric) {
        return;
      }
      ASSERT(glob.pTokn->type != TOKEN::TokenType_String);
      ASSERT(glob.pTokn->bPhony == FALSE);
      reinterpret_cast<TOKEN*>(reinterpret_cast<size_t>(glob.pTokn))->bPhony = TRUE; // 强制修改
      m_ValueDict.insert(clmake_pair(glob.pTokn, value));
    }
    else if(glob.IsNode()) {
      ASSERT(glob.pNode->magic == SYNTAXNODE::FLAG_OPERAND_MAGIC);
      glob.pNode->magic = SYNTAXNODE::FLAG_OPERAND_MAGIC_REPLACED;
      m_ValueDict.insert(clmake_pair(glob.pNode, value));
    }
    else {
      // 对空的glob增加一个替换值
      SYNTAXNODE node = { SYNTAXNODE::MODE_Undefined };
      node.magic = SYNTAXNODE::FLAG_OPERAND_MAGIC_REPLACED;
      reinterpret_cast<GLOB*>(reinterpret_cast<size_t>(&glob))->pNode = m_NodePool.PushBack(node);
      m_ValueDict.insert(clmake_pair(glob.pNode, value));
    }
  }

  void CodeParser::GetRepalcedValue(VALUE& value, const GLOB& glob) const
  {
    if(glob.IsToken()) {
      ASSERT(glob.pTokn->HasReplacedValue());
      auto it = m_ValueDict.find(glob.pTokn);
      ASSERT(it != m_ValueDict.end()); // 标记了不可能查不到
      value = it->second;
    }
    else if(glob.IsNode()) {
      ASSERT(glob.pNode->magic == SYNTAXNODE::FLAG_OPERAND_MAGIC_REPLACED);
      auto it = m_ValueDict.find(glob.pNode);
      ASSERT(it != m_ValueDict.end()); // 标记了不可能查不到
      value = it->second;
    }
    else {
    }
  }

  // 计算常量定义中的值
  // 返回值：State_OK, State_Call, State_Identifier 不会输出消息，
  // 其它结果会在内部输出错误消息
  VALUE::State CodeParser::CalculateValueAsConstantDefinition(VALUE& value_out, NameContext& sNameCtx, const GLOB& const_expr_glob)
  {
    value_out.clear();
    VALUE::State state = sNameCtx.CalculateConstantValue(value_out, this, &const_expr_glob);
    if(state != VALUE::State_OK)
    {
      //const TOKEN* pToken = const_expr_glob.pNode->Operand[1].GetFrontToken();
      const TOKEN* pToken = const_expr_glob.GetFrontToken();
      switch(state)
      {
      case VALUE::State_UnknownOpcode:
        GetLogger()->OutputErrorW(*pToken, UVS_EXPORT_TEXT(5033, "无效的操作符"));
        break;
      case VALUE::State_SyntaxError:
        GetLogger()->OutputErrorW(*pToken, UVS_EXPORT_TEXT(5034, "语法错误"));
        break;
      case VALUE::State_Overflow:
        GetLogger()->OutputErrorW(*pToken, UVS_EXPORT_TEXT(5035, "溢出"));
        break;
      case VALUE::State_IllegalChar:
        GetLogger()->OutputErrorW(*pToken, UVS_EXPORT_TEXT(5036, "非法字符"));
        break;
      case VALUE::State_BadOpcode:
        GetLogger()->OutputErrorW(*pToken, UVS_EXPORT_TEXT(5037, "错误的操作符"));
        break;
      case VALUE::State_IllegalNumber:
        GetLogger()->OutputErrorW(*pToken, UVS_EXPORT_TEXT(5038, "非法的数字"));
        break;
      case VALUE::State_DivideByZero:
        // error C2124 : 被零除或对零求模
        GetLogger()->OutputErrorW(*pToken, UVS_EXPORT_TEXT(2124, "被零除或对零求模"));
        break;
      case VALUE::State_BadIdentifier: // 内部输出
        break;
      case VALUE::State_Call: // 向量/矩阵等常量的初始化
      case VALUE::State_Identifier: // 右值中含有变量，不是一个在编译时就能计算出数值的表达式
        break;
      default:
        PARSER_BREAK(const_expr_glob.pNode->Operand[1]);
        break;
      }
    }
    return state;
  }

  void CodeParser::DbgBreak(const GLOB& glob)
  {
    DbgBreak(glob.GetFrontToken());
  }

  void CodeParser::DbgBreak(const SYNTAXNODE* pNode)
  {
    DbgBreak(&pNode->GetAnyTokenAPB());
  }

  void CodeParser::DbgBreak(const TOKEN* pToken)
  {
    GetLogger()->OutputErrorW(pToken, UVS_EXPORT_TEXT(9999, "没实现的功能"));

    if(s_bParserBreak) {
      CLBREAK;
    }
  }

  void CodeParser::DbgAssert(b32 bConditional, const GLOB& glob)
  {
    DbgAssert(bConditional, *glob.GetFrontToken());
  }

  void CodeParser::DbgAssert(b32 bConditional, const TOKEN& token)
  {
    if(_CL_NOT_(bConditional))
    {
      GetLogger()->OutputErrorW(token, UVS_EXPORT_TEXT(9998, "断言异常"));

      if(s_bParserAssert) {
        CLBREAK;
      }
    }
  }

  void CodeParser::SetTokenPhonyString(int index, const clStringA& str)
  {
    PHONY_TOKEN pt;
    auto emplace = m_PhonyTokenDict.emplace(clmake_pair(index, pt));
    TOKEN& token = m_aTokens[index];
    if(emplace.second)
    {
      // 宏多次展开时可能遇到替代值
      //ASSERT(_CL_NOT_(token.bPhony)); // 第一次添加肯定不是替代值
      emplace.first->second.ori_marker = token.marker;
    }
    else
    {
      ASSERT(token.bPhony); // 添加肯定已经标记为"替代品"
    }
    emplace.first->second.szPhonyText = GetUniqueString(str.CStr());
    emplace.first->second.nPhonyLength = str.GetLength();
    token.SetPhonyString(emplace.first->second.szPhonyText, emplace.first->second.nPhonyLength);
  }

  const CodeParser::MACRO* CodeParser::FindMacro(const TOKEN& token) // TODO: 正常的查找都要换做这个
  {
    clStringA strTokenName = token.ToString();

    auto& it_macro = m_pContext->Macros.find(strTokenName);
    if(it_macro != m_pContext->Macros.end()) {
      return &it_macro->second;
    }

    return NULL;
  }

  void CodeParser::GetNext(ArithmeticExpression::iterator& it)
  {
    if(m_ExpandedStream.empty()) {
      ++it; // next(it);
    }
    else {
      *it = m_ExpandedStream.front();

      if(it->marker == NULL) {
        // m_ExpandedStream 最后一个token可能记录的是结尾
        ASSERT(m_ExpandedStream.size() == 1);
        it = end();
      } else {
        it = *it;
      }
      it->semi_scope = -1;
      it->scope = -1;
      m_ExpandedStream.pop_front();
      if( ! m_ExpandedStream.empty() && m_ExpandedStream.front().pContainer == NULL) {
        m_ExpandedStream.front().pContainer = it.pContainer;
      }
    }
  }

  CodeParser::iterator CodeParser::MakeupMacroFunc(TOKEN::List& stream, iterator& it, const iterator& end)
  {
    int depth = 0;
    TOKEN::T_LPCSTR begin_ptr = it.marker;
    
    for(auto iter_stream = stream.begin(); iter_stream != stream.end(); ++iter_stream)
    {
      if(*iter_stream == '(') {
        depth++;
      }
      else if(*iter_stream == ')') {
        depth--;
      }
    }

    for(; it != end; ++it)
    {
      stream.push_back(*it);

      if(it == '(') {
        depth++;
      }
      else if(it == ')') {
        depth--;
        if(depth <= 0) {
          ++it;
          break;
        }
      }
      else if(depth == 0 && it.marker > begin_ptr) {
        // “MARCRO_FUNC”后面不是“(”
        stream.pop_back();
        break;
      }
    }

    return it;
  }

  GXBOOL CodeParser::OnToken(TOKEN& token)
  {
    if(token.type == TOKEN::TokenType_String || TEST_FLAG(m_dwState, AttachFlag_NotExpandMacro) || ! m_ExpandedStream.empty()) {
      return FALSE;
    }

    if(ExpandInnerMacro(token, token))
    {
      return FALSE;
    }

    const MACRO* pMacro = FindMacro(token);

    // 如果没有定义宏的形参和代换，则不处理这个宏
    if( ! pMacro || (pMacro->aFormalParams.empty() && pMacro->aTokens.empty())) {
      return FALSE;
    }

    ASSERT(pMacro->nNumTokens > 0);

    iterator it = token;
    TOKEN::List stream;
    const size_t macro_offset = token.offset();

    if(pMacro->aFormalParams.empty() && pMacro->nNumTokens == 1)
    {
      // 这里不展开不含形参的宏，保证和带形参宏处理级别一致
      // 对于“#define Time Time+5”这种写法，如果这里展开，
      // 在次级再做展开就会产生“Time+5+5”这样错误的表达式
      stream.push_back(*it);
      ++it;
    }
    else
    {
      int depth = 0;
      TOKEN save_token = *it;

      it = MakeupMacroFunc(stream, it, end());

      if(depth < 0)
      {
        if(TEST_FLAG(m_dwState, AttachFlag_NotExpandCond)) {
          // FIXME: 这里没有处理 #if defined(ADD(1,2)) 这种符合ADD(a,b)形参的形式, 可能会导致后面表达式计算出错！
          token = save_token;
          return FALSE;
        }
        else {
          // ERROR: 宏看起来有形参，但实际使用时缺少形参
          PARSER_BREAK(&token);
        }
      }
    } // if(ctx.pMacro->aFormalParams.empty())

    ASSERT(token.pContainer);
    TOKEN next_token = *it;
    if(ExpandMacroContent(stream, token, NULL) == MacroExpand_Rematch)
    {
      it = MakeupMacroFunc(stream, it, end());
      next_token = *it;
      MacroExpand result = ExpandMacroContent(stream, token, NULL);
      ASSERT(result == MacroExpand_Ok); // 对于不完整的宏调用，只能重新展开一次
    }
    m_ExpandedStream = stream;
    m_ExpandedStream.push_back(next_token);
    if(m_ExpandedStream.front().pContainer == NULL) {
      m_ExpandedStream.front().pContainer = reinterpret_cast<CodeParser*>(macro_offset | 1);
    }
    ASSERT(((size_t)m_ExpandedStream.back().pContainer & 1) == 0);
    return TRUE;
  }

  CodeParser::MacroExpand CodeParser::TryMatchMacro(MACRO_EXPAND_CONTEXT& ctx_out, TOKEN::List::iterator* it_out, const TOKEN::List::iterator& it_begin, const TOKEN::List::iterator& it_end)
  {
    // 返回0表示没展开宏
    // 返回1表示展开宏，it_out是结尾
    // 返回-1表示当前宏调用不完整，需要上一级补充完整
    if(it_begin->type == TOKEN::TokenType_String || TEST_FLAG(m_dwState, AttachFlag_NotExpandMacro) || ! m_ExpandedStream.empty()) {
      return MacroExpand_Skip;
    }

    // 没有找到同名宏定义 || 不展开集合中存在的宏（防止无限展开）
    if(_CL_NOT_(ctx_out.pMacro = FindMacro(*it_begin)) || 
      ctx_out.OrderSet.find(ctx_out.pMacro->nOrder) != ctx_out.OrderSet.end())
    {
      return MacroExpand_Skip;
    }

    TOKEN::List::iterator it = it_begin;
    ctx_out.OrderSet.insert(ctx_out.pMacro->nOrder);

    if(ctx_out.pMacro->aFormalParams.empty() && ctx_out.pMacro->nNumTokens == 1)
    {
      ctx_out.ActualParam.clear();
      ++it;

      ctx_out.stream.clear();
      ctx_out.stream.insert(ctx_out.stream.end(), ctx_out.pMacro->aTokens.begin(), ctx_out.pMacro->aTokens.end());
    }
    else
    {
      auto it_prev = it;
      ++it;
      if(it == it_end || *it != '(') {
        // 宏定义有形参"r(a)", 但是代码中非函数形式"r=x"形式的调用不认为是宏
        return MacroExpand_Skip;
      }

      if(++it == it_end) {
        GetLogger()->OutputErrorW(*it, UVS_EXPORT_TEXT(2008, "“%s”:宏定义中的意外"), clStringW(it->ToString()));
        *it_out = it;
        return MacroExpand_Ok;
      }

      int depth = 1;

      //
      // 准备宏实参表
      //
      TOKEN::List ll;
      ctx_out.ActualParam.clear();
      ctx_out.ActualParam.reserve(ctx_out.pMacro->aFormalParams.size() * 2); // 预估
      ctx_out.ActualParam.push_back(ll);
      for(; it != it_end; ++it)
      {
        if(*it == '(') {
          depth++;
        }
        else if(*it == ')') {
          depth--;
          if( ! depth) {
            break;
          }
        }
        else if(*it == ',' && depth == 1)
        {
          ctx_out.ActualParam.push_back(ll);
          continue;
        }

        ctx_out.ActualParam.back().push_back(*it);
      }

      ASSERT(it == it_end || *it == ')');

      if(depth == 0)
      {
        ASSERT(*it == ')');
        ++it;
        if(ctx_out.ActualParam.size() < ctx_out.pMacro->aFormalParams.size()) {
          clStringW strW;
          it_begin->ToString(strW);
          GetLogger()->OutputErrorW(it_begin->marker, UVS_EXPORT_TEXT(4003, "“%s”宏的实参不足"), strW.CStr());
          return MacroExpand_Skip;
        }
        
        ctx_out.stream.clear();
        ExpandMacroFunc(ctx_out); // 展开含有形参的宏“M(a,b)”或者不完整的“M(a,”形式
      }
      else {
        ASSERT(it == it_end);
        *it_out = it;
        return MacroExpand_Incomplete;
      }
    } // if(ctx.pMacro->aFormalParams.empty())


    *it_out = it;

    // 重新对流中的token进行一次检查，展开不含形参的宏
    return ExpandMacroContent(ctx_out.stream, *ctx_out.pLineNumRef, &ctx_out.OrderSet);
  }

  void CodeParser::ExpandMacroFunc(MACRO_EXPAND_CONTEXT& c) // 展开宏调用的内容
  {
    GXBOOL bPound = FALSE;
    MACRO_EXPAND_CONTEXT ctx;
    ctx.pLineNumRef = c.pLineNumRef;

    for(auto it_element = c.pMacro->aTokens.begin(); it_element != c.pMacro->aTokens.end(); ++it_element)
    {
      if(*it_element == PREPROCESS_pound) {
        bPound = TRUE;
        continue;
      }

      if(it_element->type == TOKEN::TokenType_FormalParams) {
        TOKEN::List& p = c.ActualParam[it_element->formal_index];
        if(bPound) {
          std::for_each(p.begin(), p.end(), [&c](const TOKEN& tt) {
            c.stream.push_back(tt);
            c.stream.back().type = TOKEN::TokenType_String;
            c.stream.back().precedence = 0;
          });
        }
        else {
          // 参数在单独的列表中预先展开，再存入父列表
          ExpandMacroContent(p, *c.pLineNumRef, NULL);
          c.stream.insert(c.stream.end(), p.begin(), p.end());
        }
      }
      else {
        c.stream.push_back(*it_element);
        bPound = FALSE;
      }
    }
  }

  CodeParser::MacroExpand CodeParser::ExpandMacroContent(TOKEN::List& sTokenList, const TOKEN& line_num, MACRO_EXPAND_CONTEXT::OrderSet_T* pOrderSet) // 展开宏内容里的宏
  {
    TOKEN::List::iterator itMacroEnd; // 如果有展开的宏，则将宏的结尾放置在这个里面
    TOKEN::List::iterator it_end = sTokenList.end();
    MACRO_EXPAND_CONTEXT ctx;
    MacroExpand result = MacroExpand_Ok;
    ctx.pLineNumRef = &line_num;

    if(pOrderSet) {
      ctx.OrderSet = *pOrderSet;
    }

    for(auto it = sTokenList.begin(); it != it_end;)
    {
      if(ExpandInnerMacro(*it, line_num)) {
        ++it;
      }
      else if((result = TryMatchMacro(ctx, &itMacroEnd, it, it_end)) != MacroExpand_Skip) {
        if(result == MacroExpand_Incomplete) {
          ASSERT(itMacroEnd == it_end); // TryMatchMacro约定
          return MacroExpand_Rematch;
        }
        else if(result == MacroExpand_Ok)
        {
          // TODO: 精简一下这个代码
          it = sTokenList.erase(it, itMacroEnd);
          ASSERT(it == itMacroEnd); // 理论上应该是一个
          sTokenList.insert(it, ctx.stream.begin(), ctx.stream.end());
        }
        else if(result == MacroExpand_Rematch)
        {
          it = sTokenList.erase(it, itMacroEnd);

          if(it == sTokenList.begin()) {
            sTokenList.insert(it, ctx.stream.begin(), ctx.stream.end());
            it = sTokenList.begin();
          }
          else {
            auto it_prev = it;
            --it_prev;
            sTokenList.insert(it, ctx.stream.begin(), ctx.stream.end());
            it = ++it_prev;
          }
        }
        else {
          CLBREAK;
        }

        // 恢复当前层级的集合
        if(pOrderSet) {
          ctx.OrderSet = *pOrderSet;
        }
        else {
          ctx.OrderSet.clear();
        }
      }
      else {
        ++it;
      }
    } // for
    return MacroExpand_Ok;
  }

  //////////////////////////////////////////////////////////////////////////
  
  GXBOOL CodeParser::Parse()
  {
#if 1
    TKSCOPE scope(0, m_aTokens.size());
    while(ParseStatement(&scope));
    RelocalePointer();
    return m_pLogger->ErrorCount() == 0;
#else
    __try
    {
      TKSCOPE scope(0, m_aTokens.size());
      while(ParseStatement(&scope));
      RelocalePointer();
      //return m_errorlist.empty();
      return m_pLogger->ErrorCount() == 0;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // ERROR: 致命错误, 无法从错误中恢复
      GetLogger()->OutputErrorW(UVS_EXPORT_TEXT(5002, "致命错误, 无法从错误中恢复"));
    }
#endif
    return FALSE;
  }

  GXBOOL CodeParser::ParseStatement(TKSCOPE* pScope)
  {
    return (pScope->begin < pScope->end) &&
      (
        ParseStatementAs_Struct(pScope) ||
        ParseStatementAs_Typedef(pScope) ||
        ParseStatementAs_Definition(pScope) ||
        ParseStatementAs_Function(pScope) ||
        ParseStatement_SyntaxError(pScope)
      );
  }

  GXBOOL CodeParser::ParseStatementAs_Definition(TKSCOPE* pScope)
  {
    if(pScope->begin == pScope->end) {
      return TRUE;
    }
    TKSCOPE saved_scope = *pScope;

    TOKEN* p = &m_aTokens[pScope->begin];

    if(p->semi_scope == pScope->begin) { // 独立的分号
      pScope->begin++;
      return TRUE;
    }
    else if((TKSCOPE::TYPE)p->semi_scope == TKSCOPE::npos || (TKSCOPE::TYPE)p->semi_scope > pScope->end) {
      return FALSE;
    }

    const TOKEN* pEnd = &m_aTokens.front() + p->semi_scope;

    STATEMENT stat = {StatementType_Definition};
    TKSCOPE::TYPE definition_end = p->semi_scope + 1;

    // 储存限定
    if(*p == "extern") {
      stat.defn.storage_class = VariableStorageClass_extern;
    }
    else if(*p == "nointerpolation") {
      stat.defn.storage_class = VariableStorageClass_nointerpolation;
    }
    else if(*p == "precise") {
      stat.defn.storage_class = VariableStorageClass_precise;
    }
    else if(*p == "shared") {
      stat.defn.storage_class = VariableStorageClass_shared;
    }
    else if(*p == "groupshared") {
      stat.defn.storage_class = VariableStorageClass_groupshared;
    }
    else if(*p == "static") {
      stat.defn.storage_class = VariableStorageClass_static;
    }
    else if(*p == "uniform") {
      stat.defn.storage_class = VariableStorageClass_uniform;
    }
    else if(*p == "volatile") {
      stat.defn.storage_class = VariableStorageClass_volatile;
    }
    else {
      stat.defn.storage_class = VariableStorageClass_empty;
    }

    if(stat.defn.storage_class != VariableStorageClass_empty) {
      INC_BUT_NOT_END(p, pEnd);
      pScope->begin++;
    }

    // TODO: 是不是能直接简化用表达式解析，然后再填到stat中？
    // 修饰
    if(*p == "const") {
      stat.defn.modifier = UniformModifier_const;
    }
    else if(*p == "row_major") {
      stat.defn.modifier = UniformModifier_row_major;
    }
    else if(*p == "column_major") {
      stat.defn.modifier = UniformModifier_column_major;
    }
    else {
      stat.defn.modifier = UniformModifier_empty;
    }

    if(stat.defn.modifier != UniformModifier_empty)
    {
      INC_BUT_NOT_END(p, pEnd);
      pScope->begin++;
    }

    stat.defn.szType = GetUniqueString(p);

    TKSCOPE scope(pScope->begin, definition_end);
    const size_t nErrorCount = DbgErrorCount();
    if(_CL_NOT_(ParseExpression(stat.sRoot, &m_GlobalSet, scope)))
    {
      ASSERT(DbgErrorCount() > nErrorCount); // 确保内部输出了错误消息
      return FALSE;
    }

    // 函数声明
    if(stat.sRoot.pNode->Operand[1].IsNode() && stat.sRoot.pNode->Operand[1].pNode->mode == SYNTAXNODE::MODE_FunctionCall)
    {
      *pScope = saved_scope;
      return FALSE;
    }

    pScope->begin = definition_end; // 步进到下一个statement

    if(stat.sRoot.IsNode() && stat.sRoot.pNode->mode != SYNTAXNODE::MODE_Definition)
    {
      const TOKEN tk = stat.sRoot.pNode->GetAnyTokenAPB();
      clStringW str;
      GetLogger()->OutputErrorW(tk, UVS_EXPORT_TEXT(5023, "“%s” : 不是一个定义."), tk.ToString(str).CStr());
      return TRUE;
    }

    // abc; 这种会被解析为token
    if(stat.sRoot.IsNode() &&
      stat.sRoot.pNode->Operand[1].IsNode() &&
      stat.sRoot.pNode->Operand[1].pNode->CompareOpcode(','))
    {
      MakeSyntaxNode(&stat.sRoot, SYNTAXNODE::MODE_Chain, &stat.sRoot, NULL);
      FlatDefinition(stat.sRoot.pNode);
#ifdef ENABLE_SYNTAX_VERIFY
      SYNTAXNODE* pChainNode = stat.sRoot.pNode;
      while(pChainNode) {
        Verify_VariableDefinition(m_GlobalSet, pChainNode->Operand[0].pNode);
        pChainNode = pChainNode->Operand[1].pNode;
      }
#endif
      m_aStatements.push_back(stat);
    }
    else
    {
#ifdef ENABLE_SYNTAX_VERIFY
      Verify_VariableDefinition(m_GlobalSet, stat.sRoot.pNode,
        stat.defn.modifier == UniformModifier_const);
#endif
      m_aStatements.push_back(stat);
    }

    ASSERT(pEnd->semi_scope + 1 == definition_end && *pEnd == ';');
    return TRUE;
  }

  GXBOOL CodeParser::ParseStatementAs_Function(TKSCOPE* pScope)
  {
    // 函数语法
    //[StorageClass] Return_Value Name ( [ArgumentList] ) [: Semantic]
    //{
    //  [StatementBlock]
    //};

    if(pScope->begin == pScope->end) {
      return TRUE;
    }

    TOKEN* p = &m_aTokens[pScope->begin];
    TOKEN* ptkReturnedType = p;
    const TOKEN* pEnd = &m_aTokens.front() + pScope->end;

    STATEMENT stat = {StatementType_Empty};

    // ## [StorageClass]
    if(*p == "inline") {
      stat.func.eStorageClass = StorageClass_inline;
      INC_BUT_NOT_END(p, pEnd); // ERROR: inline 应该修饰函数
    }
    else {
      stat.func.eStorageClass = StorageClass_empty;
    }

    // 检测函数格式特征
    if(p[2] != "(" || p[2].scope < 0) {
      return FALSE;
    }

    // #
    // # Return_Value
    // #
    stat.func.szReturnType = GetUniqueString(p);
    INC_BUT_NOT_END(p, pEnd); // ERROR: 缺少“；”

    // #
    // # Name
    // #
    stat.func.szName = GetUniqueString(p);
    INC_BUT_NOT_END(p, pEnd); // ERROR: 缺少“；”

    clStringA strFunctionName;
    strFunctionName.Format("%s()", stat.func.szName);
    NameContext sNameSet_Func(strFunctionName, &m_GlobalSet);

    ASSERT(*p == '('); // 由前面的特征检查保证
    ASSERT(p->scope >= 0);  // 由 GenerateTokens 函数保证

    // #
    // # ( [ArgumentList] )
    // #
    int nTypeOnlyCount = 0;
    if(p[0].scope != p[1].scope + 1) // 有参数: 两个括号不相邻
    {
      TKSCOPE ArgScope; //(m_aTokens[p->scope].scope + 1, p->scope);
      InitTokenScope(ArgScope, *p, FALSE);
      sNameSet_Func.allow_keywords = KeywordFilter_InFuntionArgument;
      ParseFunctionArguments(sNameSet_Func, &stat, &ArgScope, nTypeOnlyCount);
    }
    p = &m_aTokens[p->scope];
    ASSERT(*p == ')');

    ++p;

    // #
    // # [: Semantic]
    // #
    if(*p == ":") {
      stat.func.szSemantic = m_pContext->Strings.add((p++)->ToString());
    }

    if(*p == ";") { // 函数声明
      stat.type = StatementType_FunctionDecl;
    }
    else if(*p == "{") { // 函数体
      if(nTypeOnlyCount != 0) {
        GetLogger()->OutputErrorW(*p, UVS_EXPORT_TEXT(2055, "应输入形参表，而不是类型表"));
      }

      TKSCOPE func_statement_block; // (m_aTokens[p->scope].scope, p->scope);
      InitTokenScope(func_statement_block, *p, TRUE);

      stat.type = StatementType_Function;
      p = &m_aTokens[p->scope];
      ++p;

      if(func_statement_block.IsValid())
      {
        m_aDbgExpressionOperStack.clear();

        GetLogger()->ResetSessionError();
        ASSERT(func_statement_block.begin < func_statement_block.end); // 似乎不应该有相等的情况, "{}" 区间这种是相差一个的
        sNameSet_Func.allow_keywords = KeywordFilter_InFuntion;
        if(func_statement_block.GetSize() == 1)
        {
          stat.sRoot.ptr = NULL;
        }
        else if(ParseCodeBlock(stat.sRoot, &sNameSet_Func, func_statement_block))
        {
#ifdef ENABLE_SYNTAX_VERIFY
          const size_t nErrorCount = DbgErrorCount();
          if(sNameSet_Func.SetReturnType(stat.func.szReturnType) == FALSE)
          {
            clStringW strW;
            GetLogger()->OutputErrorW(*ptkReturnedType, UVS_EXPORT_TEXT(5031, "函数返回值“%s”不是一个类型"), ptkReturnedType->ToString(strW).CStr());
          }          
          else if(Verify_Block(stat.sRoot.pNode, &sNameSet_Func) == FALSE)
          {
            ASSERT(DbgErrorCount() > nErrorCount); // 内部输出错误，这里校验
            //TRACE("函数内部语法错误\n");
          }
#endif
        }
      }
    }
    else {
      // ERROR: 声明看起来是一个函数，但是既不是声明也不是实现。
      GetLogger()->OutputErrorW(*p, UVS_EXPORT_TEXT(5011, "声明看起来是一个函数，但是既不是声明也不是实现"));
      return FALSE;
    }


    m_aStatements.push_back(stat);
    m_GlobalSet.RegisterFunction(stat.func.szReturnType, stat.func.szName, &m_aArgumentsPack[(size_t)stat.func.pArguments], stat.func.nNumOfArguments);

    pScope->begin = p - &m_aTokens.front();
    return TRUE;
  }

  GXBOOL CodeParser::ParseStatement_SyntaxError(TKSCOPE* pScope)
  {
    const TOKEN& token = m_aTokens[pScope->begin];
    clStringW strW;
    GetLogger()->OutputErrorW(token, UVS_EXPORT_TEXT(5032, "语法错误：不能识别得格式“%s”"), token.ToString(strW).CStr());
    return FALSE;
  }

#ifdef REFACTOR_COMMA
  SYNTAXNODE* CodeParser::FlatDefinition(SYNTAXNODE* pThisChain)
  {
    SYNTAXNODE* pNode = pThisChain->Operand[0].pNode;

    // 外部保证满足如下关系
    ASSERT(pThisChain->mode == SYNTAXNODE::MODE_Chain);
    ASSERT(pNode->mode == SYNTAXNODE::MODE_Definition);
    ASSERT(pNode->Operand[1].IsNode() && pNode->Operand[1].pNode->CompareOpcode(','));

    SYNTAXNODE::PtrList sVarList;
    GLOB* pFirstVar = BreakDefinition(sVarList, pNode->Operand[1].pNode);
    pNode->Operand[1].ptr = pFirstVar->ptr;

    for(auto it = sVarList.begin(); it != sVarList.end(); ++it)
    {
      SYNTAXNODE* n = *it;
      n->mode = pNode->mode; // SYNTAXNODE::MODE_Definition;
      n->pOpcode = NULL;

      if(n->Operand[1].IsNode() && n->Operand[1].pNode->CompareOpcode(',')) {
        // 把下一级的节点提上来
        n->Operand[1].ptr = n->Operand[1].pNode->Operand[0].ptr;
      }
      n->Operand[0].ptr = pNode->Operand[0].ptr;

      SYNTAXNODE* pChain = AllocNode(SYNTAXNODE::MODE_Chain, n, pThisChain->Operand[1].ptr);

      pThisChain->Operand[1].pNode = pChain;
      pThisChain = pChain;
    }
    return pThisChain;
  }
#else
  SYNTAXNODE* CodeParser::FlatDefinition(SYNTAXNODE* pThisChain)
  {
    SYNTAXNODE* pNode = pThisChain->Operand[0].pNode;

    // 外部保证满足如下关系
    ASSERT(pThisChain->mode == SYNTAXNODE::MODE_Chain);
    ASSERT(pNode->mode == SYNTAXNODE::MODE_Definition);

    SYNTAXNODE::PtrList sVarList;
    GLOB* pFirstVar = BreakDefinition(sVarList, pNode->Operand[1].pNode);
    pNode->Operand[1].ptr = pFirstVar->ptr;

    for(auto it = sVarList.begin(); it != sVarList.end(); ++it)
    {
      SYNTAXNODE* n = *it;
      n->mode = pNode->mode; // SYNTAXNODE::MODE_Definition;
      n->pOpcode = NULL;
      n->Operand[0].ptr = pNode->Operand[0].ptr;

      SYNTAXNODE* pChain = m_NodePool.Alloc();
#ifdef ENABLE_SYNTAX_NODE_ID
      pChain->id = m_nNodeId++;
#endif
      pChain->magic = SYNTAXNODE::FLAG_OPERAND_MAGIC;
      pChain->mode = SYNTAXNODE::MODE_Chain;
      pChain->pOpcode = NULL;
      pChain->Operand[0].ptr = n;
      pChain->Operand[1].ptr = pThisChain->Operand[1].ptr;

      pThisChain->Operand[1].pNode = pChain;
      pThisChain = pChain;
    }
    return pThisChain;
  }
#endif

#ifdef REFACTOR_COMMA
  SYNTAXNODE::GLOB* CodeParser::BreakDefinition(SYNTAXNODE::PtrList& sVarList, SYNTAXNODE* pNode)
  {
    if(pNode->Operand[1].IsNode() && pNode->Operand[1].pNode->CompareOpcode(','))
    {
      sVarList.push_back(pNode);
      BreakDefinition(sVarList, pNode->Operand[1].pNode);
    }
    else {
      sVarList.push_back(pNode);
    }
    return &pNode->Operand[0];
  }

  SYNTAXNODE::GlobList& CodeParser::BreakComma(SYNTAXNODE::GlobList& sExprList, const GLOB& sGlob)
  {
    // (,) [a] [b,c]
    if(sGlob.IsNode() && sGlob.pNode->CompareOpcode(',')) {
      sExprList.push_back(sGlob.pNode->Operand[0]);

      if(sGlob.pNode->Operand[1].IsNode()) {
        return BreakComma(sExprList, sGlob.pNode->Operand[1]);
      }
      else if(sGlob.pNode->Operand[1].IsToken()) {
        sExprList.push_back(sGlob.pNode->Operand[1]);
        return sExprList;
      }
    }

    if(sGlob.ptr) {
      sExprList.push_back(sGlob);
    }
    return sExprList;
  }

  SYNTAXNODE::GlobPtrList& CodeParser::BreakComma(SYNTAXNODE::GlobPtrList& sExprList, GLOB& sGlob)
  {
    // (,) [a] [b,c]
    if(sGlob.IsNode() && sGlob.pNode->CompareOpcode(',')) {
      sExprList.push_back(&sGlob.pNode->Operand[0]);

      if(sGlob.pNode->Operand[1].IsNode()) {
        return BreakComma(sExprList, sGlob.pNode->Operand[1]);
      }
      else if(sGlob.pNode->Operand[1].IsToken()) {
        sExprList.push_back(&sGlob.pNode->Operand[1]);
        return sExprList;
      }
    }

    if(sGlob.ptr) {
      sExprList.push_back(&sGlob);
    }
    return sExprList;
  }
#else
  SYNTAXNODE::GLOB* CodeParser::BreakDefinition(SYNTAXNODE::PtrList& sVarList, SYNTAXNODE* pNode)
  {
    if(pNode->Operand[0].IsNode() && pNode->Operand[0].pNode->CompareOpcode(','))
    {
      sVarList.push_front(pNode);
      return BreakDefinition(sVarList, pNode->Operand[0].pNode);
    }
    sVarList.push_front(pNode);
    return &pNode->Operand[0];
  }

  SYNTAXNODE::GlobList& CodeParser::BreakComma(SYNTAXNODE::GlobList& sExprList, const GLOB& sGlob)
  {
    // (,) [a,b] [c]
    if(sGlob.IsNode() && sGlob.pNode->CompareOpcode(',')) {
      sExprList.push_front(sGlob.pNode->Operand[1]);
      
      if(sGlob.pNode->Operand[0].IsNode()) {
        return BreakComma(sExprList, sGlob.pNode->Operand[0]);
      }
      else if(sGlob.pNode->Operand[0].IsToken()) {
        sExprList.push_front(sGlob.pNode->Operand[0]);
        return sExprList;
      }
    }

    if(sGlob.ptr) {
      sExprList.push_front(sGlob);
    }
    return sExprList;
  }
#endif

  SYNTAXNODE::GlobList& CodeParser::BreakChain(SYNTAXNODE::GlobList& sExprList, const GLOB& sGlob)
  {
    if(sGlob.IsNode() && sGlob.pNode->mode == SYNTAXNODE::MODE_Chain) {
      sExprList.push_back(sGlob.pNode->Operand[0]);

      if(sGlob.pNode->Operand[1].IsNode()) {
        return BreakChain(sExprList, sGlob.pNode->Operand[1]);
      }
      else if(sGlob.pNode->Operand[1].IsToken()) {
        sExprList.push_back(sGlob.pNode->Operand[1]);
        return sExprList;
      }
    }
    return sExprList;
  }

  CodeParser::TKSCOPE::TYPE CodeParser::ParseStructDefinition(NameContext& sNameSet, const TKSCOPE& scope, GLOB* pMembers, GLOB* pDefinitions, int* pSignatures, int* pDefinitionNum)
  {
    // pMembers->ptr为空表示解析失败，空定义pMembers->ptr也是一个block

    const TOKEN* pTokensFront = &m_aTokens.front();
    const TOKEN* p = &m_aTokens[scope.begin];
    const TOKEN* pEnd = pTokensFront + scope.end;

    ASSERT(*p == "struct");

    // 名字
    const TOKEN* ptkName = ++p;
    if(ptkName >= pEnd || _CL_NOT_(ptkName->IsIdentifier()))
    {
      clStringW str;
      GetLogger()->OutputErrorW(*ptkName, UVS_EXPORT_TEXT(2332, "“%s”: 缺少标记名"), (p - 1)->ToString(str).CStr());
      ASSERT(str == _CLTEXT("struct"));
      return p - pTokensFront;
    }

    clStringA strName;
    p->ToString(strName);

    ++p;
    if(p >= pEnd) {
      GetLogger()->OutputErrorW(*p, UVS_EXPORT_TEXT(2143, "语法错误 : 缺少“;”"));
      return scope.end;
    }
    else if(*p == ';') {
      // 结构体声明, 只是声明有这么个东西
      sNameSet.RegisterStruct(ptkName, NULL);
      return (p - pTokensFront + 1);
    }
    else if(*p == '{')
    {
      // 结构体定义

      TKSCOPE sMembersScope;
      InitTokenScope(sMembersScope, *p, TRUE);
      ASSERT(sMembersScope.end < scope.end); // 给出的作用域不对

      //NameContext sMemberSet(strName, &sNameSet);

      if(_CL_NOT_(ParseExpression(*pMembers, NULL, sMembersScope)))
      {
        ERROR_MSG__MISSING_SEMICOLON(IDX2ITER(sMembersScope.end));
        return sMembersScope.end;
      }

      p = pTokensFront + sMembersScope.end;
      if(p >= pEnd) {
        GetLogger()->OutputErrorW(m_aTokens[scope.begin], UVS_EXPORT_TEXT(1004, "遇到意外的文件结束")); // 遇到意外的文件结束
        return scope.end;
      }
      else if(m_aTokens[scope.begin] != ";" && m_aTokens[scope.begin].IsIdentifier() == FALSE) {
        GetLogger()->OutputErrorW(m_aTokens[scope.begin], UVS_EXPORT_TEXT(2145, "语法错误：标识符前面缺少“%s”"), ";"); // "语法错误：标识符前面缺少“%s”"
        return sMembersScope.end;
      }

      //////////////////////////////////////////////////////////////////////////
      //TRACE("---------------------------------------------\n");
      //DbgDumpSyntaxTree(NULL, pMembers->pNode, 0, NULL, 1);
      //TRACE("---------------------------------------------\n");

      if(pMembers->pNode->Operand[0].ptr)
      {
        // 这里面所做的就是将 type a,b,c;这种形式展开为
        // type a; type b; type c;
        if(pMembers->IsNode()) {
          SYNTAXNODE* pThisChain = NULL;
          int nSignatures = 0; // Signature Member数量
          int nDefination = 0; // 定义数量
          b32 result = TRUE;
          RecursiveNode<SYNTAXNODE>(this, pMembers->pNode->Operand[0].pNode,
            [this, &pThisChain, &nDefination, &nSignatures, &result]
          (SYNTAXNODE* pNode, int depth)->GXBOOL
          {
            if(depth == 0)
            {
              if(pNode->mode != SYNTAXNODE::MODE_Chain) {
                // ERROR!
                CLBREAK;
                return FALSE;
              }
              pThisChain = pNode;
            }
            else if(depth == 1)
            {
              if(pNode->mode != SYNTAXNODE::MODE_Definition) {
                // ERROR
                const TOKEN& t = pNode->GetAnyTokenAPB();
                clStringA str = t.ToString();
                GetLogger()->OutputErrorW(t, UVS_EXPORT_TEXT(2062, "意外的类型“%s”"), clStringW(str));
                result = FALSE;
                return FALSE;
              }

              nDefination++;

              if(pNode->Operand[0].IsToken() && pNode->Operand[1].IsNode()
                && pNode->Operand[1].pNode->CompareOpcode(','))
              {
                pThisChain = FlatDefinition(pThisChain);
              }
            }
            else if(depth == 2)
            {
              if(pNode->CompareOpcode(':')) {
                nSignatures++;
              }
            }

            return TRUE;
          });

          if(_CL_NOT_(result)) {
            return scope.end;
          }

          //if(nSignatures && nSignatures != nDefination) {
          //  // ERROR: 不是所有成员都定义为signatures
          //  CLBREAK;
          //}
          *pSignatures = nSignatures;
          *pDefinitionNum = nDefination;

#ifdef ENABLE_SYNTAX_VERIFY
          Verify_StructMember(sNameSet, strName, *pMembers->pNode); // 检查失败仍然继续解析
#endif
        }
      }
      else // 空结构体
      {
        //stat.type = StatementType_Struct;
        //stat.stru.nNumOfMembers = 0;
        *pSignatures = 0;
        *pDefinitionNum = 0;
      }

      if(sNameSet.RegisterStruct(ptkName, pMembers->pNode) == FALSE &&
        sNameSet.GetVariable(ptkName))
      {
        clStringW str;
        GetLogger()->OutputErrorW(*ptkName, UVS_EXPORT_TEXT(2371, "“%s”: 重定义；不同的基类型"), ptkName->ToString(str).CStr());
        return sMembersScope.end;
      }


      p = pTokensFront + sMembersScope.end;
    } // else if(*p == '{')

    //////////////////////////////////////////////////////////////////////////

    if(p > &m_aTokens.back()) {
      GetLogger()->OutputErrorW(*pTokensFront, UVS_EXPORT_TEXT(1004, "遇到意外的文件结束"));
      return scope.end;
    }
    else if(*p == ';') {
      pMembers->pNode->Operand[1].pTokn = p; // Block后面的分号
      return p - pTokensFront + 1;
    }
    else if(p->IsIdentifier())
    {
      const auto semi_end = p->semi_scope;
      TKSCOPE scope_var(p - pTokensFront, semi_end + 1);
      pMembers->pNode->Operand[1].pTokn = &m_aTokens[semi_end]; // Block后面的分号

      GLOB glob;

      if(_CL_NOT_(ParseExpression(glob, &sNameSet, scope_var)))
      {
        ERROR_MSG__MISSING_SEMICOLON(IDX2ITER(scope_var.end));
        return FALSE;
      }

      // 封装一个定义序列

      glob.pNode = AllocNode(SYNTAXNODE::MODE_Definition, NULL, glob.ptr);
      glob.pNode->Operand[0].pTokn = ptkName;

      p = pTokensFront + clMin(scope.end, scope_var.end);

      if(glob.pNode->Operand[1].IsNode() &&
        glob.pNode->Operand[1].pNode->CompareOpcode(','))
      {
        MakeSyntaxNode(&glob, SYNTAXNODE::MODE_Chain, &glob, NULL);
        FlatDefinition(glob.pNode);

#ifdef ENABLE_SYNTAX_VERIFY
        SYNTAXNODE* pChainNode = glob.pNode;
        while(pChainNode) {
          Verify_VariableDefinition(sNameSet, pChainNode->Operand[0].pNode);
          pChainNode = pChainNode->Operand[1].pNode;
        }
#endif
      }
      pDefinitions->ptr = glob.ptr;
      return p - pTokensFront;
    }
    else if(*p != ';') {
      clStringW str;
      GetLogger()->OutputErrorW(*p, UVS_EXPORT_TEXT(2143, "语法错误 : 缺少“;”(在“%s”的前面)"), p->ToString(str).CStr());
      return scope.end;
    }

    //////////////////////////////////////////////////////////////////////////
    CLBREAK; // 其实不会到这里
    return p - pTokensFront;
  }

  GXBOOL CodeParser::ParseStatementAs_Struct(TKSCOPE* pScope)
  {
    TOKEN* p = &m_aTokens[pScope->begin];
    if(*p != "struct") {
      return FALSE;
    }

    STATEMENT stat_stru = { StatementType_Empty };
    STATEMENT stat_vari = { StatementType_Definition };
    stat_vari.defn.modifier = UniformModifier_empty;
    stat_vari.defn.storage_class = VariableStorageClass_empty;
    stat_vari.defn.szType = stat_stru.stru.szName;
    int nSignatures = 0;
    int nDefinition = 0;
    
    pScope->begin = ParseStructDefinition(m_GlobalSet, *pScope, &stat_stru.sRoot,
      &stat_vari.sRoot, &nSignatures, &nDefinition);

    if(stat_stru.sRoot.ptr)
    {
      stat_stru.type = nSignatures ? StatementType_Signatures : StatementType_Struct;
      stat_stru.stru.szName = GetUniqueString(p + 1);
      stat_stru.stru.nNumOfMembers = nDefinition;
    }

    m_aStatements.push_back(stat_stru);

    if(stat_vari.sRoot.ptr) {
      m_aStatements.push_back(stat_vari);
    }

    return TRUE;
  }

  GXBOOL CodeParser::ParseStatementAs_Typedef(TKSCOPE* pScope)
  {
    if(pScope->begin == pScope->end) {
      return TRUE;
    }

    TOKEN* p = &m_aTokens[pScope->begin];
    if(*p != "typedef") {
      return FALSE;
    }

    STATEMENT stat = { StatementType_Typedef };

    TKSCOPE::TYPE end = TKSCOPE::npos;
    if(TryKeywords(m_GlobalSet, *pScope, &stat.sRoot, &end)) {
      m_aStatements.push_back(stat);
    }
    pScope->begin = end;
    return TRUE;
  }

  GXBOOL CodeParser::ParseFunctionArguments(NameContext& sNameSet, STATEMENT* pStat, TKSCOPE* pArgScope, int& nTypeOnlyCount)
  {
    // 函数参数格式
    // [InputModifier] Type Name [: Semantic]
    // 函数可以包含多个参数，用逗号分隔

    nTypeOnlyCount = 0; // 类型描述的数量
    TOKEN* p = &m_aTokens[pArgScope->begin];
    const TOKEN* pEnd = &m_aTokens.front() + pArgScope->end;
    ASSERT(*pEnd == ")"); // 函数参数列表的结尾必须是闭圆括号

    if(pArgScope->begin >= pArgScope->end) {
      // ERROR: 函数参数声明不正确
      return FALSE;
    }

    ArgumentsArray aArgs;

    while(1)
    {
      FUNCTION_ARGUMENT arg = {InputModifier_in};

      if(*p == "in") {
        arg.eModifier = InputModifier_in;
      }
      else if(*p == "out") {
        arg.eModifier = InputModifier_out;
      }
      else if(*p == "inout") {
        arg.eModifier = InputModifier_inout;
      }
      else if(*p == "uniform" || *p == "const") {
        arg.eModifier = InputModifier_uniform;
      }
      else {
        goto NOT_INC_P;
      }

      INC_BUT_NOT_END(p, pEnd); // ERROR: 函数参数声明不正确

NOT_INC_P:
      arg.ptkType = p;
      if(++p >= pEnd) {
        nTypeOnlyCount++;
        aArgs.push_back(arg);
        break;
      }
      
      if(*p == ',') {
        nTypeOnlyCount++;
        aArgs.push_back(arg);
        INC_BUT_NOT_END(p, pEnd); // ERROR: 函数参数声明不正确
        continue;
      }

      arg.ptkName = p;

      if(++p >= pEnd) {
        aArgs.push_back(arg);
        break;
      }

      if(*p == ",") {
        aArgs.push_back(arg);
        INC_BUT_NOT_END(p, pEnd); // ERROR: 函数参数声明不正确
        continue;
      }
      else if(*p == ":") {
        INC_BUT_NOT_END(p, pEnd); // ERROR: 函数参数声明不正确
        arg.szSemantic = GetUniqueString(p);
      }
      else {
        // ERROR: 函数参数声明不正确
        return FALSE;
      }
    }

    ASSERT( ! aArgs.empty());

    if(aArgs.size() == 1 && *(aArgs.front().ptkType) == "void")
    {
      // func(void) 形式
      pStat->func.pArguments = 0;
      pStat->func.nNumOfArguments = 0;
    }
    else
    {
      pStat->func.pArguments = (FUNCTION_ARGUMENT*)m_aArgumentsPack.size();
      pStat->func.nNumOfArguments = aArgs.size();

      //StringArray sFormalTypenames;
      m_aArgumentsPack.reserve(m_aArgumentsPack.size() + aArgs.size());
      //sFormalTypenames.reserve(aArgs.size());

      for(auto it = aArgs.begin(); it != aArgs.end(); ++it)
      {
        clStringA str;
        if(it->ptkName)
        {
          const TYPEDESC* pTypeDesc = sNameSet.RegisterVariable(it->ptkType->ToString(str), it->ptkName); // 注册在临时域内, 用来检查形参重名
        }
        m_aArgumentsPack.push_back(*it);
      }
    }

    return TRUE;
  }

  void CodeParser::RelocalePointer()
  {
    RelocaleStatements(m_aStatements);
    RelocaleStatements(m_aSubStatements);
  }

  GXLPCSTR CodeParser::GetUniqueString( const TOKEN* pSym )
  {
    return m_pContext->Strings.add(pSym->ToString());
  }

  GXLPCSTR CodeParser::GetUniqueString(T_LPCSTR szText)
  {
    return m_pContext->Strings.add(szText);
  }

  void CodeParser::DbgDumpSyntaxTree(clStringArrayA* pArray, const SYNTAXNODE* pNode, int precedence, clStringA* pStr, int depth)
  {
    if(s_bDumpSyntaxTree == FALSE) {
      return;
    }

    clStringA str[2];
    for(int i = 0; i < 2; i++)
    {
      if(pNode->Operand[i].ptr) {
        //const auto flag = pNode->Operand[i].GetType();
        if(pNode->Operand[i].IsToken()) {
          str[i] = pNode->Operand[i].pTokn->ToString();
        }
        else if(pNode->Operand[i].IsNode()) {
          if(depth == 0) {
            DbgDumpSyntaxTree(pArray, pNode->Operand[i].pNode, pNode->pOpcode ? pNode->pOpcode->precedence : 0, &str[i], 0);
          }
          else if(pNode->mode == SYNTAXNODE::MODE_Chain) {
            DbgDumpSyntaxTree(pArray, pNode->Operand[i].pNode, pNode->pOpcode ? pNode->pOpcode->precedence : 0, &str[i], depth);
          }
          else {
            DbgDumpSyntaxTree(pArray, pNode->Operand[i].pNode, pNode->pOpcode ? pNode->pOpcode->precedence : 0, &str[i], depth + 1);
          }
        }
        else {
          CLBREAK;
        }
      }
      else {
        str[i].Clear();
      }
    }

    clStringA strCommand;
    if(depth)
    {
#ifdef _DEBUG
      strCommand.AppendFormat("<ID_%04d>", pNode->id);
#endif
      strCommand.Append(' ', depth * 2);
#ifdef _DEBUG
      if(pNode->Operand[0].IsNode() && pNode->Operand[1].IsNode())
      {
        strCommand.AppendFormat("[%s] [%s]<ID_%04d> [%s]<ID_%04d>",
          pNode->pOpcode ? pNode->pOpcode->ToString() : "",
          str[0], pNode->Operand[0].pNode->id,
          str[1], pNode->Operand[1].pNode->id);
      }
      else if(pNode->Operand[0].IsNode())
      {
        strCommand.AppendFormat("[%s] [%s]<ID_%04d> [%s]",
          pNode->pOpcode ? pNode->pOpcode->ToString() : "",
          str[0], pNode->Operand[0].pNode->id,
          str[1]);
      }
      else if(pNode->Operand[1].IsNode())
      {
        strCommand.AppendFormat("[%s] [%s] [%s]<ID_%04d>",
          pNode->pOpcode ? pNode->pOpcode->ToString() : "",
          str[0], str[1], pNode->Operand[1].pNode->id);
      }
      else {
        strCommand.AppendFormat("[%s] [%s] [%s]", pNode->pOpcode ? pNode->pOpcode->ToString() : "", str[0], str[1]);
      }
#else
      strCommand.AppendFormat("[%s] [%s] [%s]", pNode->pOpcode ? pNode->pOpcode->ToString() : "", str[0], str[1]);
#endif

    }
    else
    {
      strCommand.Format("[%s] [%s] [%s]", pNode->pOpcode ? pNode->pOpcode->ToString() : "", str[0], str[1]);
    }

    int dots = 40 - (int)strCommand.GetLength();
    if(pNode->mode == SYNTAXNODE::MODE_Chain) {
      if(dots > 0) { strCommand.Append('.', dots); }
      strCommand.Append("(Chain)");
    }
    else if(pNode->mode == SYNTAXNODE::MODE_Block) {
      if(dots > 0) { strCommand.Append('.', dots); }
      strCommand.Append("(Block)");
    }
    else if(pNode->mode == SYNTAXNODE::MODE_Assignment) {
      if(dots > 0) { strCommand.Append('.', dots); }
      strCommand.Append("(Assignment)");
    }
    else if(pNode->mode == SYNTAXNODE::MODE_Definition) {
      if(dots > 0) { strCommand.Append('.', dots); }
      strCommand.Append("(definition)");
    }
    else if(pNode->mode == SYNTAXNODE::MODE_Subscript) {
      if(dots > 0) { strCommand.Append('.', dots); }
      strCommand.Append("(subscript)");
    }
    else if(pNode->mode == SYNTAXNODE::MODE_Subscript0) {
      if(dots > 0) { strCommand.Append('.', dots); }
      strCommand.Append("(subscript0)");
    }
    else if(pNode->mode == SYNTAXNODE::MODE_InitList) {
      if(dots > 0) { strCommand.Append('.', dots); }
      strCommand.Append("(init list)");
    }
    else if(pNode->mode == SYNTAXNODE::MODE_Bracket) {
      if(dots > 0) { strCommand.Append('.', dots); }
      strCommand.Append("(bracket)");
    }
    else if(pNode->mode == SYNTAXNODE::MODE_Opcode) {
      if(dots > 0) { strCommand.Append('.', dots); }
      strCommand.Append("(opcode)");
    }
    if(pArray) {
      pArray->push_back(strCommand);
    }
    TRACE("%s\n", strCommand);

    clStringA strOut;
    switch(pNode->mode)
    {
    case SYNTAXNODE::MODE_Undefined: // 设置为替换Node或者解析出错才会有这个
      break;

    case SYNTAXNODE::MODE_FunctionCall: // 函数调用
      strOut.Format("%s(%s)", str[0], str[1]);
      break;

    case SYNTAXNODE::MODE_Subscript0:
      ASSERT(str[1].IsEmpty());
      strOut.Format("%s[]", str[0]);
      break;

    case SYNTAXNODE::MODE_Subscript:
      strOut.Format("%s[%s]", str[0], str[1]);
      break;

    case SYNTAXNODE::MODE_TypeCast:
      strOut.Format("(%s)%s", str[0], str[1]);
      break;

    case SYNTAXNODE::MODE_Definition:
      strOut.Format("%s %s", str[0], str[1]);
      break;

    case SYNTAXNODE::MODE_Flow_If:
      strOut.Format("if(%s){%s;}", str[0], str[1]);
      break;

    case SYNTAXNODE::MODE_Flow_Else:
      strOut.Format("%s else {%s;}", str[0], str[1]);
      break;

    case SYNTAXNODE::MODE_Flow_ElseIf:
      strOut.Format("%s else %s", str[0], str[1]);
      break;

    case SYNTAXNODE::MODE_Flow_While:
      strOut.Format("%s(%s)", str[0], str[1]);
      break;

    case SYNTAXNODE::MODE_Flow_For:
      strOut.Format("for(%s){%s}", str[0], str[1]);
      break;

    case SYNTAXNODE::MODE_Flow_ForInit:
    case SYNTAXNODE::MODE_Flow_ForRunning:
      strOut.Format("%s;%s", str[0], str[1]);
      break;

    case SYNTAXNODE::MODE_Return:
      ASSERT(str[0] == "return");
      strOut.Format("return %s", str[1]);
      break;

    case SYNTAXNODE::MODE_Block:
      strOut.Format("{%s}", str[0]);
      break;

    case SYNTAXNODE::MODE_Chain:
      if(str[1].IsEmpty()) {
        strOut.Format("%s", str[0]);
      }
      else {
        strOut.Format("%s;%s", str[0], str[1]);
      }
      break;

    case SYNTAXNODE::MODE_Opcode:
      if(precedence > pNode->pOpcode->precedence) { // 低优先级先运算
        strOut.Format("(%s%s%s)", str[0], pNode->pOpcode->ToString(), str[1]);
      }
      else {
        strOut.Format("%s%s%s", str[0], pNode->pOpcode->ToString(), str[1]);
      }
      break;
    case SYNTAXNODE::MODE_Assignment:
      strOut.Format("%s=%s", str[0], str[1]);
      break;

    case SYNTAXNODE::MODE_StructDef:
      strOut.Format("struct %s ", str[0]);
      strOut.Append(str[1]);
      break;

    case SYNTAXNODE::MODE_InitList:
      ASSERT(pNode->Operand[1].ptr == NULL);
      strOut.Format("{%s}", str[0]);
      break;

    case SYNTAXNODE::MODE_Bracket:
      strOut.Format("%s%s", str[0], str[1]);
      break;

    case SYNTAXNODE::MODE_Flow_DoWhile:
      break;

    case SYNTAXNODE::MODE_CommaList:
      strOut.Format("%s,%s", str[0], str[1]);
      break;

    default:
      // 没处理的 pNode->mode 类型
      CLBREAK;
      break;
    }

    if(pStr) {
      *pStr = strOut;
    }
    else {
      TRACE("%s\n", strOut);
    }
  }

  //////////////////////////////////////////////////////////////////////////
  GXBOOL CodeParser::TryKeywords(NameContext& sNameSet, const TKSCOPE& scope, GLOB* pDesc, TKSCOPE::TYPE* parse_end)
  {
    // 如果是关键字，返回true，否则返回false
    // 解析成功parse_end返回表达式最后一个token的索引，parse_end是这个关键字表达式之内的！
    // 解析失败parse_end返回RTSCOPE::npos

    const auto& front = m_aTokens[scope.begin];
    auto& pend = *parse_end;
    GXBOOL bret = TRUE;
    SYNTAXNODE::MODE eMode = SYNTAXNODE::MODE_Undefined;

    ASSERT(pend == TKSCOPE::npos); // 强调调用者要初始化这个变量

    if(front == "else") {
      // ERROR: "else" 不能独立使用
      GetLogger()->OutputErrorW(front, UVS_EXPORT_TEXT(2181, "没有匹配 if 的非法 else"));
    }
    else if(front == "for") {
      pend = ParseFlowFor(sNameSet, scope, pDesc);
      ASSERT(pend == TKSCOPE::npos || m_aTokens[pend - 1] == ';' || m_aTokens[pend - 1] == '}');
    }
    else if(front == "if") {
      pend = ParseFlowIf(sNameSet, scope, pDesc, FALSE);
      ASSERT(pend == TKSCOPE::npos || m_aTokens[pend - 1] == ';' || m_aTokens[pend - 1] == '}');
    }
    else if(front == "while") {
      pend = ParseFlowWhile(sNameSet, scope, pDesc);
      ASSERT(pend == TKSCOPE::npos || m_aTokens[pend - 1] == ';' || m_aTokens[pend - 1] == '}');
    }
    else if(front == "do") {
      pend = ParseFlowDoWhile(sNameSet, scope, pDesc);
      ASSERT(pend == TKSCOPE::npos || m_aTokens[pend - 1] == ';' || m_aTokens[pend - 1] == '}');
    }
    else if(front == "typedef") {
      pend = ParseTypedef(sNameSet, scope, pDesc);
      ASSERT(pend == TKSCOPE::npos || m_aTokens[pend - 1] == ';');
    }
    else if(front == "struct") {
      //pend = ParseStructDefine(scope, pDesc);
      GLOB sMembers = { 0 };
      GLOB sVariable = { 0 };
      int nSignatures = 0;
      int nDefinition = 0;
      pend = ParseStructDefinition(sNameSet, scope, &sMembers, &sVariable, &nSignatures, &nDefinition);

      if(sMembers.ptr)
      {
        GLOB sName;
        sName = m_aTokens[scope.begin + 1];

        MakeSyntaxNode(pDesc, SYNTAXNODE::MODE_StructDef, &sName, &sMembers);
      }

      ASSERT(nSignatures == 0);  // 没处理这个错误
      if(sVariable.ptr != NULL)
      {
        MakeSyntaxNode(pDesc, SYNTAXNODE::MODE_Chain, pDesc, &sVariable);
      }

      //sNameSet.RegisterStruct()
    }
    else if(front == "break") {
      if(TEST_FLAG_NOT(sNameSet.allow_keywords, KeywordFilter_break)) {
        GetLogger()->OutputErrorW(front, UVS_EXPORT_TEXT(2043, "非法 break"));
      }
      eMode = SYNTAXNODE::MODE_Flow_Break;
      pend = scope.begin + 2;
    }
    else if(front == "continue") {
      if(TEST_FLAG_NOT(sNameSet.allow_keywords, KeywordFilter_continue)) {
        GetLogger()->OutputErrorW(front, UVS_EXPORT_TEXT(2044, "非法 continue"));
      }
      eMode = SYNTAXNODE::MODE_Flow_Continue;
      pend = scope.begin + 2;
    }
    else if(front == "case") {
      if(TEST_FLAG_NOT(sNameSet.allow_keywords, KeywordFilter_case)) {
        GetLogger()->OutputErrorW(front, UVS_EXPORT_TEXT(2046, "非法 case"));
      }

      TKSCOPE::TYPE i = scope.begin + 1;
      for(; i < scope.end; i++)
      {
        if(m_aTokens[i] == ':') {
          break;
        }
      }

      if(i == scope.end) {
        GetLogger()->OutputErrorW(front, UVS_EXPORT_TEXT(5025, "case 结尾缺少“:”"));
        pend = scope.begin + 1;
      }
      else {
        pend = i + 1;
      }
      eMode = SYNTAXNODE::MODE_Flow_Case;
    }
    else if(front == "discard") {
      eMode = SYNTAXNODE::MODE_Flow_Discard;
      pend = scope.begin + 2;
    }
    else if(front == "return")
    {
      eMode = SYNTAXNODE::MODE_Return;
      pend = scope.begin + 2;
    }
    else {
      bret = FALSE;
    }

    while(pend != TKSCOPE::npos && bret && (
      eMode == SYNTAXNODE::MODE_Flow_Break || eMode == SYNTAXNODE::MODE_Flow_Continue ||
      eMode == SYNTAXNODE::MODE_Flow_Discard || eMode == SYNTAXNODE::MODE_Return))
    {
      GLOB A = {0}, B = {0};

      A = front;
      pend = scope.begin + 2;

      if(front.semi_scope == TKSCOPE::npos || (TKSCOPE::TYPE)front.semi_scope > scope.end) {
        // ERROR: 缺少;
        ERROR_MSG__MISSING_SEMICOLON(front);
        break;
      }

      if(eMode == SYNTAXNODE::MODE_Return) {
        TKSCOPE ret_scope(scope.begin + 1, front.semi_scope);
        bret = ParseArithmeticExpression(0, ret_scope, &B);
        MakeSyntaxNode(pDesc, eMode, NULL, &A, &B);
        pend = bret ? front.semi_scope + 1 : TKSCOPE::npos;
      }
      else {
        MakeSyntaxNode(pDesc, eMode, NULL, &A, NULL);
      }
      break;
    }

    ASSERT(( ! bret && pend == TKSCOPE::npos) || bret);
    ASSERT(pend == TKSCOPE::npos || (pend > scope.begin && pend <= scope.end));
    return bret;
  }

  //////////////////////////////////////////////////////////////////////////

  GXBOOL CodeParser::ParseExpression(GLOB& glob, NameContext* pNameSet, const TKSCOPE& scope)
  {
    // 解析一条表达式语句或者一个语句块
    ASSERT(scope.begin == scope.end ||
      (m_aTokens[scope.begin].semi_scope == scope.end - 1 && m_aTokens[scope.end - 1] == ';') ||
      (m_aTokens[scope.begin].scope == scope.end - 1 && m_aTokens[scope.begin] == '{' && m_aTokens[scope.end - 1] == '}'));

    if(m_aTokens[scope.begin] == '{') {
      return ParseCodeBlock(glob, pNameSet, scope);
    }
    else if(ParseArithmeticExpression(1, TKSCOPE(scope.begin, scope.end - 1), &glob) == FALSE)
    {
      GetLogger()->OutputErrorW(m_aTokens[scope.begin], UVS_EXPORT_TEXT(5013, "表达式解析失败"));
      return FALSE;
    }
    return TRUE;
  }

  //////////////////////////////////////////////////////////////////////////
  // 解析一个代码块
  // 避免每条语句解析都递归
  GXBOOL CodeParser::ParseCodeBlock(GLOB& glob, NameContext* pNameSet, const TKSCOPE& scope)
  {
    // 保证解析内容在数学区间: [begin, end)
    ASSERT(m_aTokens[scope.begin].scope == scope.end - 1);
    ASSERT(m_aTokens[scope.begin] == '{' && m_aTokens[scope.end - 1] == '}');

    TKSCOPE step_scope(scope.begin + 1, scope.end - 1); // begin会步进, end恒定
    MakeSyntaxNode(&glob, SYNTAXNODE::MODE_Block, NULL, NULL);

    return ParseToChain(glob.pNode->Operand[0], pNameSet, step_scope);
  }

  //////////////////////////////////////////////////////////////////////////
  // 解析一个范围内的所有表达式, 解析的表达式会连接成为chain
  GXBOOL CodeParser::ParseToChain(GLOB& glob, NameContext* pNameSet, const TKSCOPE& scope)
  {
    GLOB A;
    GLOB* pCurrNode = &glob;
    TKSCOPE step_scope = scope;

    while(step_scope.GetSize() > 0)
    {
      A.ptr = NULL;
      
      const TKSCOPE::TYPE pos = TryParseSingle(pNameSet, A, step_scope);
      if(pos == TKSCOPE::npos) {
        return FALSE;
      }
      else if(pos == step_scope.begin + 1) {
        if(m_aTokens[step_scope.begin] != ';') { // 步进一次只可能是遇到了单独的分号
          return FALSE;
        }
        step_scope.begin = pos;
        continue;
      }

      step_scope.begin = pos;
      MakeSyntaxNode(pCurrNode, SYNTAXNODE::MODE_Chain, &A, NULL);

      // 变量声明展开
      if(A.IsNode() && A.pNode->mode == SYNTAXNODE::MODE_Definition &&
        A.pNode->Operand[1].IsNode() && A.pNode->Operand[1].pNode->CompareOpcode(','))
      {
        pCurrNode = &(FlatDefinition(pCurrNode->pNode)->Operand[1]);
        continue;
      }

      pCurrNode = &pCurrNode->pNode->Operand[1];
    }
    return TRUE;
  }

  //////////////////////////////////////////////////////////////////////////
  // 解析一个代码块, 一条关键字表达式或者一条表达式
  // 如果解析失败, 返回TKSCOPE::npos
  // 如果解析成功, 返回接下来解析的起始位置

  CodeParser::TKSCOPE::TYPE CodeParser::TryParseSingle(NameContext* pNameSet, GLOB& glob, const TKSCOPE& step_scope)
  {
    ASSERT(step_scope.GetSize() > 0); // 外部保证解析时不为空
    const auto& front = m_aTokens[step_scope.begin];
    TKSCOPE::TYPE parse_end = TKSCOPE::npos;

    if(step_scope.GetSize() == 1) {
      if(front != ';') {
        GetLogger()->OutputErrorW(front, UVS_EXPORT_TEXT(2143, "语法错误 : 缺少“;”"));
      }      
      return step_scope.end;
    }
    else if(front == '{')
    {
      TKSCOPE sub_block;
      InitTokenScope(sub_block, front, TRUE);

      if(sub_block.end > step_scope.end) {
        GetLogger()->OutputErrorW(front, UVS_EXPORT_TEXT(2059, "括号不匹配, 缺少\"%s\"."), GetPairOfBracket(front.marker[0]));
        return TKSCOPE::npos;
      }

      if(ParseCodeBlock(glob, pNameSet, sub_block) == FALSE) {
        return TKSCOPE::npos;
      }
      return sub_block.end;
    }
    else if(pNameSet && TryKeywords(*pNameSet, step_scope, &glob, &parse_end)) // FIXME: 这里不能用KeywordMask_All, 要从外部继承进来
    {
      if(parse_end == TKSCOPE::npos) {
        return TKSCOPE::npos; // 解析错误, 直接返回
      }
      return parse_end;
    }

    // 普通表达式
    TKSCOPE exp_scope;

    if(front.semi_scope == TKSCOPE::npos)
    {
      GetLogger()->OutputErrorW(front, UVS_EXPORT_TEXT(5016, "表达式应该以\';\'结尾."));
      return TKSCOPE::npos;
    }

    exp_scope.begin = step_scope.begin;
    exp_scope.end   = front.semi_scope;
    ASSERT(m_aTokens[step_scope.begin].semi_scope <= (int)step_scope.end);

    //step_scope.begin = front.semi_scope + 1;

    if(exp_scope.GetSize() == 0) // 跳过独立的分号
    {
      return front.semi_scope + 1;
    }

    if(ParseArithmeticExpression(1, exp_scope, &glob) == FALSE)
    {
      GetLogger()->OutputErrorW(front, UVS_EXPORT_TEXT(5013, "表达式解析失败"));
      return TKSCOPE::npos;
    }
    return front.semi_scope + 1;
  }

  //////////////////////////////////////////////////////////////////////////

  CodeParser::TKSCOPE::TYPE CodeParser::ParseFlowIf(const NameContext& sParentSet, const TKSCOPE& scope, GLOB* pDesc, GXBOOL bElseIf)
  {
    // 与 ParseFlowWhile 相似
    NameContext sNameSet_FlowIf("if", &sParentSet);
    GLOB A = {0}, B = {0};
    GXBOOL bret = TRUE;
    ASSERT(m_aTokens[scope.begin] == "if");

    TKSCOPE sConditional(scope.begin + 2, m_aTokens[scope.begin + 1].scope);
    TKSCOPE sStatement;

    if(sConditional.begin >= scope.end || sConditional.end == -1 || sConditional.end > scope.end)
    {
      // ERROR: if 语法错误
      GetLogger()->OutputErrorW(m_aTokens[sConditional.begin], UVS_EXPORT_TEXT(5014, "if 语法错误"));
      return TKSCOPE::npos;
    }

    bret = bret && ParseArithmeticExpression(0, sConditional, &A);
    if(bret == FALSE) {
      GetLogger()->OutputErrorW(m_aTokens[sConditional.begin], UVS_EXPORT_TEXT(5043, "语法错误：“if”条件表达式"));
      return TKSCOPE::npos;
    }


    sStatement.begin = sConditional.end + 1;
    sStatement.end = scope.end;

    sStatement.begin = TryParseSingle(&sNameSet_FlowIf, B, sStatement);
    if(sStatement.begin == TKSCOPE::npos) {
      return TKSCOPE::npos;
    }

    const SYNTAXNODE::MODE eMode = TryGetNodeMode(&B);
    if(eMode != SYNTAXNODE::MODE_Block) {
      if(eMode != SYNTAXNODE::MODE_Chain) {
        bret = bret && MakeSyntaxNode(&B, SYNTAXNODE::MODE_Chain, &B, NULL);
      }
      bret = bret && MakeSyntaxNode(&B, SYNTAXNODE::MODE_Block, &B, NULL);
    }

    bret = bret && MakeSyntaxNode(pDesc, SYNTAXNODE::MODE_Flow_If, &A, &B);

    if(bret && (scope.end - sStatement.begin) > 1 && m_aTokens[sStatement.begin] == "else")
    {
      // 只处理 else if/else 两种情况
      A = *pDesc;
      ++sStatement.begin;
      if(sStatement.begin >= scope.end) {
        // ERROR: else 语法错误
        CLBREAK;
        return TKSCOPE::npos;
      }

      SYNTAXNODE::MODE eNextMode = SYNTAXNODE::MODE_Flow_Else;

      if(m_aTokens[sStatement.begin] == "if")
      {
        eNextMode = SYNTAXNODE::MODE_Flow_ElseIf;
        sStatement.begin = ParseFlowIf(sNameSet_FlowIf, TKSCOPE(sStatement.begin, scope.end), &B, TRUE);
        if(sStatement.begin == TKSCOPE::npos) {
          return TKSCOPE::npos;
        }
      }
      else
      {
        sStatement.begin = TryParseSingle(&sNameSet_FlowIf, B, sStatement);
        if(sStatement.begin == TKSCOPE::npos)
        {
          return TKSCOPE::npos;
        }
      }

      // else if/else 处理
      if(eNextMode == SYNTAXNODE::MODE_Flow_Else)
      {
        const SYNTAXNODE::MODE eMode = TryGetNodeMode(&B);
        if(eMode != SYNTAXNODE::MODE_Block) {
          if(eMode != SYNTAXNODE::MODE_Chain) {
            bret = bret && MakeSyntaxNode(&B, SYNTAXNODE::MODE_Chain, &B, NULL);
          }
          bret = bret && MakeSyntaxNode(&B, SYNTAXNODE::MODE_Block, &B, NULL);
        }
      }
           

      bret = bret && MakeSyntaxNode(pDesc, eNextMode, &A, &B);
    }

    return bret ? sStatement.begin : TKSCOPE::npos;
  }

  CodeParser::TKSCOPE::TYPE CodeParser::ParseFlowWhile(const NameContext& sParentSet, const TKSCOPE& scope, GLOB* pDesc)
  {
    // 与 ParseFlowIf 相似
    GLOB A = {0}, B = {0};
    NameContext sNameSet_FlowWhile("while", &sParentSet);
    GXBOOL bret = TRUE;
    ASSERT(m_aTokens[scope.begin] == "while");


    TKSCOPE sConditional(scope.begin + 2, m_aTokens[scope.begin + 1].scope);
    TKSCOPE sBlock;

    if( ! MakeScope(&sConditional, &MAKESCOPE(scope, scope.begin + 2, FALSE, scope.begin + 1, TRUE, 0))) {
      return TKSCOPE::npos;
    }
    
    bret = bret && ParseArithmeticExpression(0, sConditional, &A);

    sBlock.begin = sConditional.end + 1;
    sBlock.end   = TKSCOPE::npos;
    if(sBlock.begin >= scope.end) {
      // ERROR: while 语法错误
      return TKSCOPE::npos;
    }

    auto& block_begin = m_aTokens[sBlock.begin];
    //SYNTAXNODE::MODE eMode = SYNTAXNODE::MODE_Undefined;

    sBlock.end = TryParseSingle(&sNameSet_FlowWhile, B, TKSCOPE(sBlock.begin, scope.end));
    if(sBlock.end == TKSCOPE::npos) {
      return TKSCOPE::npos;
    }

    const SYNTAXNODE::MODE eMode = TryGetNodeMode(&B);
    if(eMode != SYNTAXNODE::MODE_Block) {
      if(eMode != SYNTAXNODE::MODE_Chain) {
        bret = bret && MakeSyntaxNode(&B, SYNTAXNODE::MODE_Chain, &B, NULL);
      }
      bret = bret && MakeSyntaxNode(&B, SYNTAXNODE::MODE_Block, &B, NULL);
    }

    bret = bret && MakeSyntaxNode(pDesc, SYNTAXNODE::MODE_Flow_While, &A, &B);

    DbgDumpScope("while", sConditional, sBlock); // 可能会输出[{...]的情况, 因为end='}'不包含在输出中
    return bret ? sBlock.end : TKSCOPE::npos;
  }
  
  CodeParser::TKSCOPE::TYPE CodeParser::ParseFlowDoWhile(const NameContext& sParentSet, const TKSCOPE& scope, GLOB* pDesc)
  {
    ASSERT(m_aTokens[scope.begin] == "do");
    // do{...}while(...); 中while()不允许使用类型定义
    NameContext sNameSet_Do("do", &sParentSet);


    if(scope.begin + 1 >= scope.end) {
      // ERROR: do 语法错误
      GetLogger()->OutputErrorW(m_aTokens[scope.begin], UVS_EXPORT_TEXT(5044, "do ... while 意外的结束"));
      return TKSCOPE::npos;
    }

    TKSCOPE sConditional;
    TKSCOPE sBlock;
    GLOB A = {0}, B = {0};

    const TOKEN& tkScope = m_aTokens[scope.begin + 1];
    TKSCOPE::TYPE while_token;
    if(tkScope.scope != TKSCOPE::npos)
    {
      InitTokenScope(sBlock, tkScope, FALSE);
      while_token = sBlock.end + 1;
    }
    else if(tkScope.semi_scope != TKSCOPE::npos)
    {
      sBlock.begin = scope.begin + 1;
      sBlock.end = tkScope.semi_scope + 1;
      while_token = sBlock.end;
    }
    else {
      GetLogger()->OutputErrorW(tkScope, UVS_EXPORT_TEXT(5045, "do ... while 语法错误"));
      return TKSCOPE::npos;
    }

    
    if(while_token >= scope.end && m_aTokens[while_token] != "while") {
      // ERROR: while 语法错误
      GetLogger()->OutputErrorW(m_aTokens[while_token], UVS_EXPORT_TEXT(5011, "do...while 语法错误, 没有出现预期的\"while\"关键字."));
      return TKSCOPE::npos;
    }

    InitTokenScope(sConditional, while_token + 1, FALSE);

    GXBOOL bret = ParseToChain(B, &sNameSet_Do, sBlock);
    bret = bret && ParseArithmeticExpression(0, sConditional, &A);
    bret = bret && MakeSyntaxNode(pDesc, SYNTAXNODE::MODE_Flow_DoWhile, NULL, &A, &B);

    // TODO: 检查A中没有类型定义

    TKSCOPE::TYPE while_end = sConditional.end + 2;
    if(while_end >= scope.end || m_aTokens[while_end] != ';') {
      // ERROR: 缺少 ";"
      return while_end;
    }

    return bret ? while_end : TKSCOPE::npos;
  }

  CodeParser::TKSCOPE::TYPE CodeParser::ParseTypedef(NameContext& sNameSet, const TKSCOPE& scope, GLOB* pDest)
  {
    // typedef A B;
    GLOB A = {0}, B = {0};
    const TOKEN& front = m_aTokens[scope.begin];
    ASSERT(front == "typedef");

    if(front.semi_scope - scope.begin < 3 || scope.GetSize() < 3) {
      GetLogger()->OutputErrorW(front, UVS_EXPORT_TEXT(5019, "“typedef”语法错误"));
      return TKSCOPE::npos;
    }

    TKSCOPE type_scope(scope.begin + 1, front.semi_scope - 1);
    ASSERT(type_scope.GetSize() >= 1); // 上面保证

    if(type_scope.GetSize() == 1) {
      A = m_aTokens[type_scope.begin];
    }
    else {
      if(ParseArithmeticExpression(0, type_scope, &A) == FALSE) {
        return TKSCOPE::npos;
      }
    }
    B = m_aTokens[front.semi_scope - 1];

    ASSERT(A.IsToken()); // 目前仅处理这个情况
    clStringA strTypename;

    NameContext::State state = sNameSet.TypeDefine(A.pTokn, B.pTokn);
    switch(state)
    {
    case NameContext::State_Ok: break;
    case NameContext::State_TypeNotFound:
    {
      clStringA strTypenameW;
      GetLogger()->OutputErrorW(*A.pTokn, UVS_EXPORT_TEXT(5020, "“typedef”定义中使用的“%s”不是一个类型"), A.pTokn->ToString(strTypenameW).CStr());
      return TKSCOPE::npos;
    }
    case NameContext::State_DuplicatedType:
    {
      clStringA str;
      GetLogger()->OutputErrorW(*A.pTokn, UVS_EXPORT_TEXT(5021, "“typedef”定义的类型“%s”已经存在"), B.pTokn->ToString(str).CStr());
      return TKSCOPE::npos;
    }
    default:
      CLBREAK; // 没有处理的错误
      break;
    }


    MakeSyntaxNode(pDest, SYNTAXNODE::MODE_Typedef, NULL, &A, &B);
    return front.semi_scope + 1;
  }

  GXBOOL CodeParser::MakeScope(TKSCOPE* pOut, MAKESCOPE* pParam)
  {
    // 这个函数用来从用户指定的begin和end中获得表达式所用的scope
    // 如果输入的begin和end符合要求则直接设置为scope，否则将返回FALSE
    // 可以通过mate标志获得begin或end位置上的配偶范围
    const MAKESCOPE& p = *pParam;
    TKSCOPE::TYPE& begin = pOut->begin;
    TKSCOPE::TYPE& end   = pOut->end;

    ASSERT(p.pScope->begin < p.pScope->end);

    if(p.begin >= p.pScope->end) {
      // ERROR:
      return FALSE;
    }

    if(p.bBeginMate) {
      begin = m_aTokens[p.begin].GetScope();

      if(OUT_OF_SCOPE(begin) || begin >= p.pScope->end) {
        // ERROR:
        return FALSE;
      }
    }
    else {
      begin = p.begin;
    }

    if(p.chTermin != 0 && m_aTokens[begin] == (TChar)p.chTermin) {
      end = begin;
      return TRUE;
    }

    if(p.end >= p.pScope->end) {
      // ERROR:
      return FALSE;
    }

    if(p.bEndMate) {
      end = m_aTokens[p.end].GetScope();

      if(OUT_OF_SCOPE(end) || end >= p.pScope->end) {
        // ERROR:
        return FALSE;
      }
    }
    else {
      end = p.end;
    }

    return TRUE;
  }

  CodeParser::TKSCOPE::TYPE CodeParser::MakeFlowForScope(const TKSCOPE& scope, TKSCOPE* pInit, TKSCOPE* pCond, TKSCOPE* pIter, TKSCOPE* pBlock, GLOB* pBlockNode)
  {
    ASSERT(m_aTokens[scope.begin] == "for"); // 外部保证调用这个函数的正确性

    auto open_bracket = scope.begin + 1;
    if(open_bracket >= scope.end || m_aTokens[open_bracket] != '(' || 
      (pIter->end = m_aTokens[open_bracket].scope) > scope.end)
    {
      // ERROR: for 格式错误
      return TKSCOPE::npos;
    }

    //
    // initializer
    // 初始化部分
    //
    pInit->begin  = scope.begin + 2;
    pInit->end    = m_aTokens[scope.begin].semi_scope;
    if(pInit->begin >= scope.end || pInit->end == -1) {
      // ERROR: for 格式错误
      return TKSCOPE::npos;
    }
    ASSERT(pInit->begin <= pInit->end);

    //
    // conditional
    // 条件部分
    //
    pCond->begin  = pInit->end + 1;
    pCond->end    = m_aTokens[pCond->begin].semi_scope;

    if(pCond->begin >= scope.end) {
      // ERROR: for 格式错误
      return TKSCOPE::npos;
    }

    //
    // iterator
    // 迭代部分
    //
    pIter->begin = pCond->end + 1;
    // 上面设置过 pIter->end
    if(pIter->begin >= scope.end || pIter->begin > pIter->end) {
      // ERROR: for 格式错误
      return TKSCOPE::npos;
    }

    //
    // block
    //
    //RTSCOPE sBlock;
    pBlock->begin = pIter->end + 1;
    pBlock->end   = TKSCOPE::npos;
    if(pBlock->begin >= scope.end) {
      // ERROR: for 缺少执行
      return TKSCOPE::npos;
    }

    auto& block_begin = m_aTokens[pBlock->begin];
    if(block_begin == '{')
    {
      pBlock->end = block_begin.scope + 1;
    }
    else if(TryKeywords(m_GlobalSet, TKSCOPE(pBlock->begin, scope.end), pBlockNode, &pBlock->end))// FIXME: 暂时使用全局NameSet: m_GlobalSet
    {
      ; // 没想好该干啥，哇哈哈哈!
    }
    else
    {
      pBlock->end = block_begin.semi_scope + 1;
    }

    ASSERT(pBlock->end != -1);

    if(pBlock->end > scope.end) {
      // ERROR: for 格式错误
      return TKSCOPE::npos;
    }

    //ParseExpression(pBlock, pUnion);
    return pBlock->end;
  }

  CodeParser::TKSCOPE::TYPE CodeParser::ParseFlowFor(const NameContext& sParentSet, const TKSCOPE& scope, GLOB* pDesc)
  {
    NameContext sNameSet_FlowFor("for", &sParentSet);
    TKSCOPE sInitializer, sConditional, sIterator;
    TKSCOPE sBlock;

    GLOB uInit = {0}, uCond = {0}, uIter = {0}, uBlock = {0}, D;
    
    auto result = MakeFlowForScope(scope, &sInitializer, &sConditional, &sIterator, &sBlock, &uBlock);
    if(result == TKSCOPE::npos)
    {
      return result;
    }

    ASSERT(m_aTokens[sInitializer.end] == ';');
    ASSERT(m_aTokens[sConditional.end] == ';');
    ASSERT(m_aTokens[sIterator.end] == ')');
    ASSERT(m_aTokens[sBlock.begin] == "for" || m_aTokens[sBlock.end - 1] == ';' || m_aTokens[sBlock.end - 1] == '}');

    ParseArithmeticExpression(0, sInitializer, &uInit, TOKEN::FIRST_OPCODE_PRECEDENCE);
    ParseArithmeticExpression(0, sConditional, &uCond, TOKEN::FIRST_OPCODE_PRECEDENCE);
    ParseArithmeticExpression(0, sIterator   , &uIter, TOKEN::FIRST_OPCODE_PRECEDENCE);

    if(uBlock.ptr == NULL) {
      if(TryParseSingle(&sNameSet_FlowFor, uBlock, sBlock) == TKSCOPE::npos) {
        return TKSCOPE::npos;
      }
    }

    GXBOOL bret = TRUE;
    // 如果不是代码块，就转换为代码块
    const SYNTAXNODE::MODE eMode = TryGetNodeMode(&uBlock);
    if(eMode != SYNTAXNODE::MODE_Block) {
      if(eMode != SYNTAXNODE::MODE_Chain) {
        bret = MakeSyntaxNode(&uBlock, SYNTAXNODE::MODE_Chain, &uBlock, NULL);
      }
      bret = bret && MakeSyntaxNode(&uBlock, SYNTAXNODE::MODE_Block, &uBlock, NULL);
    }

    bret = bret && MakeSyntaxNode(&D, SYNTAXNODE::MODE_Flow_ForRunning, &uCond, &uIter);
    bret = bret && MakeSyntaxNode(&D, SYNTAXNODE::MODE_Flow_ForInit, &uInit, &D);
    bret = bret && MakeSyntaxNode(pDesc, SYNTAXNODE::MODE_Flow_For, &D, &uBlock);
    
    return bret ? sBlock.end : TKSCOPE::npos;
  }

  const CodeParser::StatementArray& CodeParser::GetStatements() const
  {
    return m_aStatements;
  }

  void CodeParser::Invoke(GXLPCSTR szFunc, GXLPCSTR szArguments)
  {
    if(clstd::strcmpT(szFunc, "EnableParserBreak"))
    {
      s_bParserBreak = clStringA(szArguments).ToBoolean();
    }
    else if(clstd::strcmpT(szFunc, "EnableParserAssert"))
    {
      s_bParserAssert = clStringA(szArguments).ToBoolean();
    }
    else if(clstd::strcmpT(szFunc, "EnableDumpSyntaxTree"))
    {
      s_bDumpSyntaxTree = clStringA(szArguments).ToBoolean();
    }
    ArithmeticExpression::Invoke(szFunc, szArguments);
  }

  void CodeParser::RelocaleStatements(StatementArray& aStatements)
  {
    for(auto it = aStatements.begin(); it != aStatements.end(); ++it)
    {
      switch(it->type)
      {
      case StatementType_FunctionDecl:
      case StatementType_Function:
        IndexToPtr(it->func.pArguments, m_aArgumentsPack);
        break;
      }
    }
  }


  //////////////////////////////////////////////////////////////////////////

  CodeParser* CodeParser::GetSubParser()
  {
    if( ! m_pSubParser) {
      m_pSubParser = new CodeParser(this, m_pContext, m_pInclude);
    }
    return m_pSubParser;
  }

  CodeParser::T_LPCSTR CodeParser::DoPreprocess(const RTPPCONTEXT& ctx, T_LPCSTR begin, T_LPCSTR end)
  {
    CodeParser* pParse = GetSubParser();
    GXDWORD dwFlags = AttachFlag_Preprocess;

    ASSERT(sizeof(PREPROCESS_define) - 1 == 6);

    if(CompareString(begin, PREPROCESS_define, sizeof(PREPROCESS_define) - 1) ||
      CompareString(begin, PREPROCESS_undef, sizeof(PREPROCESS_undef) - 1) ||
      CompareString(begin, PREPROCESS_ifdef, sizeof(PREPROCESS_ifdef) - 1) ||
      CompareString(begin, PREPROCESS_ifndef, sizeof(PREPROCESS_ifndef) - 1)) {
      dwFlags |= AttachFlag_NotExpandMacro;
    }
    else if(CompareString(begin, PREPROCESS_if, sizeof(PREPROCESS_if) - 1) ||
      CompareString(begin, PREPROCESS_elif, sizeof(PREPROCESS_elif) - 1)) {
      dwFlags |= AttachFlag_NotExpandCond;
    }

    pParse->Attach(begin, (clsize)end - (clsize)begin, dwFlags, NULL);
    pParse->GenerateTokens();
    const auto& tokens = *pParse->GetTokensArray();

#ifdef _DEBUG
    if(tokens.front() == PREPROCESS_define || tokens.front() == PREPROCESS_undef || 
      tokens.front() == PREPROCESS_ifdef || tokens.front() == PREPROCESS_ifndef) {
      ASSERT(TEST_FLAG(dwFlags, AttachFlag_NotExpandMacro));
    }
    else {
      ASSERT(TEST_FLAG_NOT(dwFlags, AttachFlag_NotExpandMacro));
    }
#endif // #ifdef _DEBUG

    if(tokens.front() == PREPROCESS_include) {
      PP_Include(tokens);
    }
    else if(tokens.front() == PREPROCESS_define) {
      PP_Define(tokens);
    }
    else if(tokens.front() == PREPROCESS_undef) {
      PP_Undefine(ctx, tokens);
    }
    else if(tokens.front() == PREPROCESS_ifdef) {
      m_nPPRecursion++;
      return PP_IfDefine(ctx, FALSE, tokens);
    }
    else if(tokens.front() == PREPROCESS_ifndef) {
      m_nPPRecursion++;
      return PP_IfDefine(ctx, TRUE, tokens);
    }
    else if(tokens.front() == PREPROCESS_if) {
      m_nPPRecursion++;
      return PP_If(ctx, pParse);
    }
    else if(tokens.front() == PREPROCESS_pragma) {
      PP_Pragma(tokens);
    }
    else if(tokens.front() == PREPROCESS_file) {
      if(tokens.size() != 2) {
        // ERROR: #file 格式不正确
        GetLogger()->OutputErrorW(tokens.front(), 9999, __FILEW__, __LINE__);
      }

      clStringW strW = tokens[1].ToString();
      strW.TrimBoth('\"');
      GetLogger()->SetCurrentFilenameW(strW);
    }
    else if(tokens.front() == PREPROCESS_line) {
      clStringW str;
      if(tokens.size() != 2) {
        GetLogger()->OutputErrorW(tokens.front(), UVS_EXPORT_TEXT(2005, "#line 应输入行号，却找到“%s”"), _CLTEXT("new line"));
        return end;
      }
      else if(tokens[1].type != TOKEN::TokenType_Integer)
      {
        GetLogger()->OutputErrorW(tokens.front(), UVS_EXPORT_TEXT(2005, "#line 应输入行号，却找到“%s”"), tokens[1].ToString(str).CStr());
        return end;
      }

      GXINT nLine = tokens[1].ToString().ToInteger();
      if(nLine > 0)
      {
        //GetLogger()->SetCurrentTopLine(0);
        //GXINT nCurLine = GetLogger()->LineFromPtr(tokens[1].marker);
        //GetLogger()->SetCurrentTopLine(nLine - nCurLine - 1);
        GetLogger()->SetLine(tokens[1].marker, nLine);
      }
      else {
        // ERROR: 行号不正确
      }
    }
    else {
      GetLogger()->OutputErrorW(tokens.front(), UVS_EXPORT_TEXT(1021, "无效的预处理器命令 \"%s\"."), clStringW(tokens.front().ToString()));
    }
    return ctx.iter_next.marker;
  }

  void CodeParser::PP_Define(const TOKEN::Array& tokens)
  {
    MACRO l_m;
    MACRO_TOKEN::List sFormalList;
    //const auto& tokens = *m_pSubParser->GetTokensArray();
    ASSERT( ! tokens.empty() && tokens.front() == PREPROCESS_define);
    const auto count = tokens.size();
    l_m.nNumTokens = 0;
    l_m.nOrder = m_pContext->Macros.size();
    //m_MacrosSet.insert(strMacroName);

    if(count == 1) {
      GetLogger()->OutputErrorW(tokens.front(), UVS_EXPORT_TEXT(2007, "#define 缺少定义."));
      return;
    }
    
    clStringA strMacroName(tokens[1].marker, tokens[1].length);

    if(count == 2) // "#define MACRO" 形
    {
      l_m.nNumTokens = 1;
      m_pContext->Macros.insert(clmake_pair(strMacroName, l_m));
    }
    else if(count == 3) // "#define MACRO XXX" 形
    {
      // "#define MACROxxx" 异形, xxx可能是字符串, 符号等
      if(tokens[1].end() == tokens[2].begin()) {
        clStringW str;
        GetLogger()->OutputErrorW(tokens[1], UVS_EXPORT_TEXT(2008, "“%s”: 宏定义中的意外"), tokens[2].ToString(str).CStr());
      }

      l_m.nNumTokens = 1;
      auto result = m_pContext->Macros.insert(clmake_pair(strMacroName, l_m));

      // 如果已经添加过，清除原有数据
      if( ! result.second) {
        result.first->second.clear();
      }
      result.first->second.set(m_pContext->Macros, tokens, 2);
    }
    else
    {
      int l_define = 2;

      // 宏定义名后不是开括号并且紧跟在宏定义名后则报错
      if(tokens[1].end() == tokens[2].marker)
      {
        if(tokens[2] != '(') {
          GetLogger()->OutputErrorW(tokens[2], UVS_EXPORT_TEXT(2008, "“%s”:宏定义中的意外"), clStringW(tokens[2].ToString()));
          return;
        }

        const int scope_end = tokens[2].scope;
        l_m.nNumTokens = 3;
        if(scope_end > 3) // #define MACRO(...) ... 形解析
        {
          int i = 3;
          for(; i < scope_end; i++)
          {
            if((_CL_NOT_(tokens[i].IsIdentifier()) && (i & 1)) ||
              (tokens[i] != ',' && (i & 1) == 0))
            {
              GetLogger()->OutputErrorW(tokens[i], UVS_EXPORT_TEXT(2010, "“%s”:宏形参表中的意外"), clStringW(tokens[i].ToString()));
              return;
            }
            if(i & 1) {
              sFormalList.push_back(tokens[i]);
              sFormalList.back().formal_index = sFormalList.size() - 1;
              sFormalList.back().type = TOKEN::TokenType_FormalParams;
            }
          }
          l_m.nNumTokens = (size_t)i;


#ifdef ENABLE_SYNTAX_VERIFY
          if(_CL_NOT_(Verify_MacroFormalList(sFormalList))) {
            return;
          }
#endif
        }
        l_define = scope_end + 1;
      }
      else { // #define MACRO ... 形解析
        l_m.nNumTokens = 1;
      }

      auto result = m_pContext->Macros.insert(clmake_pair(strMacroName, l_m));
      if( ! result.second) {
        result.first->second.clear();
      }

      // 设置宏代替的表达式，如果表达式中含有宏也会展开
      result.first->second.aFormalParams.insert(
        result.first->second.aFormalParams.begin(),
        sFormalList.begin(), sFormalList.end());

      result.first->second.set(m_pContext->Macros, tokens, l_define);
    }
  }

  void CodeParser::PP_Include(const TOKEN::Array& aTokens)
  {
    clStringW strPath = GetLogger()->GetFilePathW();
    clpathfile::RemoveFileSpec(strPath);

    if(aTokens[1] == '<')
    {
      size_t i = 2;
      for(; i < aTokens.size(); i++) {
        if(aTokens[i] == '>') {
          break;
        }
      }

      if(i == 2) {
        GetLogger()->OutputErrorW(aTokens[1], UVS_EXPORT_TEXT(2012, "在\"<\"之后缺少名称."));
        return;
      }
      else if(i == aTokens.size()) {
        GetLogger()->OutputErrorW(aTokens[1], UVS_EXPORT_TEXT(2013, "缺少\">\"."));
        return;
      }

      clStringW strHeader(aTokens[2].marker, (size_t)(aTokens[i - 1].end() - aTokens[2].begin()));
      clpathfile::CombinePath(strPath, strPath, strHeader);
    }
    else if(aTokens[1].type == TOKEN::TokenType_String)
    {
      clStringW strHeader = aTokens[1].ToString();
      strHeader.TrimBoth('\"');
      clpathfile::CombinePath(strPath, strPath, strHeader);
    }
    else {
      clStringW str;
      GetLogger()->OutputErrorW(aTokens[0], UVS_EXPORT_TEXT(2006, "\"#include\" 应输入文件名, 缺找到\"%s\""), aTokens[1].ToString(str).CStr());
      return;
    }

    clBuffer* pBuffer = OpenIncludeFile(strPath);
    if(pBuffer == NULL) {
      // ERROR: 无法打开文件
      GetLogger()->OutputErrorW(aTokens[0], UVS_EXPORT_TEXT(5003, "无法打开包含文件: \"%s\""), strPath.CStr());
      return;
    }

    ASSERT(GetLogger()); // 应该从m_pParent设置过临时的m_pMsg
    CodeParser parser(this, m_pContext, m_pInclude);

    //m_pMsg->PushFile(strPath);
    //m_pMsg->GenerateCurLines((GXLPCSTR)pBuffer->GetPtr(), pBuffer->GetSize());
    GetLogger()->PushFile(strPath, 0, (GXLPCSTR)pBuffer->GetPtr(), pBuffer->GetSize());

    parser.Attach((GXLPCSTR)pBuffer->GetPtr(), pBuffer->GetSize(), 0, strPath);
    parser.GenerateTokens();

    //if( ! m_pMsg->GetErrorLevel()) // 有错误

    m_aTokens.reserve(m_aTokens.capacity() + parser.m_aTokens.size());
    clsize offset = m_aTokens.size();
    for(auto it = parser.m_aTokens.begin(); it != parser.m_aTokens.end(); ++it)
    {
      m_aTokens.push_back(*it);
      auto& last_one = m_aTokens.back();
      last_one.scope += offset;
      last_one.semi_scope += offset;
    }

    GetLogger()->PopFile();
  }

  void CodeParser::PP_Undefine(const RTPPCONTEXT& ctx, const TOKEN::Array& aTokens)
  {
    ASSERT(aTokens.front() == "undef");
    if(aTokens.size() == 1) {
      GetLogger()->OutputErrorW(aTokens.front(), UVS_EXPORT_TEXT(4006, "#undef 应输入标识符"));
      return;
    }
    else if(aTokens.size() > 2) {
      GetLogger()->OutputErrorW(aTokens.front(), UVS_EXPORT_TEXT(4067, "预处理器指令后有意外标记 - 应输入换行符"));
      return;
    }

    clStringA strMacroName = aTokens[1].ToString();
    auto it = m_pContext->Macros.find(strMacroName);

    if(it != m_pContext->Macros.end())
    {
    }
  }

  CodeParser::T_LPCSTR CodeParser::PP_IfDefine(const RTPPCONTEXT& ctx, GXBOOL bNot, const TOKEN::Array& tokens)
  {
    ASSERT( ! tokens.empty() && (tokens.front() == PREPROCESS_ifdef || tokens.front() == PREPROCESS_ifndef));

    if(tokens.size() == 1) {
      // ERROR: ifdef 缺少定义
      GetLogger()->OutputErrorW(tokens.front(), UVS_EXPORT_TEXT(1016, "#ifdef 应输入标识符."));
      return ctx.stream_end;
    }
    else if(tokens.size() == 2) {
      const GXBOOL bNotDefined = (m_pContext->Macros.find(tokens[1].ToString()) == m_pContext->Macros.end());
      if(( ! bNot && bNotDefined) || (bNot && ! bNotDefined))
      {
        const T_LPCSTR pBlockEnd = 
          PP_SkipConditionalBlock(PPCondRank_if, ctx.ppend, ctx.stream_end);
        ASSERT(pBlockEnd >= ctx.iter_next.marker);
        return pBlockEnd;
      }
      else {
        return ctx.iter_next.marker;
      }
    }
    else {
      GetLogger()->OutputErrorW(tokens.front(), UVS_EXPORT_TEXT(4067, "预处理器指令后有意外标记 - 应输入换行符")); // FIXME: 应该是警告
    }
    return ctx.iter_next.marker;
  }

  void CodeParser::PP_Pragma(const TOKEN::Array& aTokens)
  {
    ASSERT(aTokens.front() == PREPROCESS_pragma);
    if(aTokens[1] == PREPROCESS_message)
    {
      if(aTokens[2] != '(') {
        GetLogger()->OutputErrorW(aTokens[2], UVS_EXPORT_TEXT(2059, "语法错误 :“%s”"), clStringW(aTokens[2].ToString()));
        return;
      }

      if(aTokens[3].type != TOKEN::TokenType_String) {
        GetLogger()->OutputErrorW(aTokens[2], UVS_EXPORT_TEXT(2059, "语法错误 :“%s”"), clStringW(aTokens[2].ToString()));
        return;
      }

      ASSERT(aTokens[2].GetScope() != -1); // 如果能走到这里括号一定是配对的
      clStringW str = aTokens[3].ToString();
      str.TrimBoth('\"');
      str.Append("\r\n");
      GetLogger()->WriteMessageW(str);
    }
    else {
      // 不能识别的pragma子指令
      GetLogger()->OutputErrorW(aTokens[1], UVS_EXPORT_TEXT(1021, "无效的预处理器命令 “%s”"), clStringW(aTokens[1].ToString()));
    }
  }

  //////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////

  GXBOOL CodeParser::CalculateValue(OPERAND& sOut, const GLOB* pDesc)
  {
    if(pDesc->IsToken())
    {
      sOut.pToken = pDesc->pTokn;
      return TRUE;
    }
    else if(pDesc->IsNode())
    {
      const SYNTAXNODE* pNode = pDesc->pNode;
      OPERAND param[2];
      param[0].clear();
      param[1].clear();
      GLOB l_desc;
      for(int i = 0; i < 2; i++)
      {
        if(!pNode->Operand[i].ptr) {
          continue;
        }
        l_desc.ptr = pNode->Operand[i].ptr;
        CalculateValue(param[i], &l_desc);
      }

      if(pNode->Operand[0].CompareAsToken(PREPROCESS_defined))
      {
        // "#if defined(COND)" 和 "#if defined COND" 都可以
        ASSERT(pNode->mode == SYNTAXNODE::MODE_FunctionCall ||
          pNode->mode == SYNTAXNODE::MODE_Definition);

        ASSERT(pNode->Operand[0].IsToken());

        if( ! pNode->Operand[1].IsToken()) {
          GetLogger()->OutputErrorW(*pNode->Operand[0].pTokn, UVS_EXPORT_TEXT(2004, "应输入“defined(id)”"));
          return FALSE;
        }

        auto it = m_pContext->Macros.find(pNode->Operand[1].pTokn->ToString());

        sOut.v.rank = VALUE::Rank_Signed64;
        sOut.v.nValue64 = (it != m_pContext->Macros.end());
      }
      else if(pNode->mode == SYNTAXNODE::MODE_Opcode)
      {
        for(int i = 0; i < 2; i++) {
          if(param[i].v.rank == VALUE::Rank_Undefined && param[i].pToken) {
            if(param[i].pToken->type == TOKEN::TokenType_Identifier)
            {
              const MACRO* pMacro = FindMacro(*param[i].pToken);
              if(pMacro) {
                PARSER_BREAK(param[i].pToken);
              }
              else {
                param[i].v.SetZero(); // [Doc\预编译\宏]没有定义的宏默认为0
              }
            }
            else if(param[i].pToken->type > TOKEN::TokenType_FirstNumeric &&
              param[i].pToken->type < TOKEN::TokenType_LastNumeric)
            {
              param[i].v.set(*param[i].pToken);
            }
            else {
              PARSER_BREAK(param[i].pToken);
            }
          }
        }
        sOut.Calculate(*pNode->pOpcode, param);
      }
      else
      {
        CLBREAK;
        // pNode->mode == SYNTAXNODE::MODE_FunctionCall
        // ERROR: 类函数调用只能是defined
      }
    }
    else {
      PARSER_BREAK(*pDesc);
    }

    return TRUE;
  }

  //////////////////////////////////////////////////////////////////////////

  CodeParser::T_LPCSTR CodeParser::PP_If(const RTPPCONTEXT& ctx, CodeParser* pParser)
  {
    GLOB sDesc = {0};
    TKSCOPE scope(1, pParser->m_aTokens.size());
    pParser->EnableHigherDefinition(TRUE);
    if( ! pParser->ParseArithmeticExpression(0, scope, &sDesc)) {
      // ERROR: 无法解析表达式
      GetLogger()->OutputErrorW(pParser->m_aTokens.front(), UVS_EXPORT_TEXT(5004, "无法解析#if的条件表达式"));
      pParser->EnableHigherDefinition(FALSE);
      return ctx.iter_next.marker;
    }
    pParser->EnableHigherDefinition(FALSE);

    VALUE v;
    if(sDesc.IsToken()) {
      v.SetZero();

      if(sDesc.pTokn->IsIdentifier())
      {
        clStringA strMacro = sDesc.pTokn->ToString();
        auto it = m_pContext->Macros.find(strMacro);
        if(it != m_pContext->Macros.end() && it->second.aFormalParams.empty()) {
          v = it->second.aTokens.front();
        }
      }
      else {
        v.set(*sDesc.pTokn);
      }
    }
    else if(sDesc.IsNode())
    {      
      OPERAND result;
      CalculateValue(result, &sDesc);
      //TRACE("#if %s\n", result.v.ToString());

      v = result.v;
    }
    else {
      // 定义了一个带形参的宏, 但实际只用了宏的名字
      ASSERT(pParser->m_aTokens.size() == 1);
      v.SetZero();
    }
    
    if(v.nValue64 == 0) {
      return PP_SkipConditionalBlock(PPCondRank_if, ctx.ppend, ctx.stream_end);
    }
    return ctx.iter_next.marker;
  }

  CodeParser::T_LPCSTR CodeParser::PP_SkipConditionalBlock(PPCondRank session, T_LPCSTR begin, T_LPCSTR end)
  {
    // session 调用 Macro_SkipConditionalBlock 所处的状态，这个决定了跳过多少预处理指令
    if(m_nPPRecursion == 0)
    {
      if(session == PPCondRank_elif) {
        GetLogger()->OutputErrorW(begin, UVS_EXPORT_TEXT(1018, "意外的 #elif"));
        return end;
      }
      else if(session == PPCondRank_else) {
        GetLogger()->OutputErrorW(begin, UVS_EXPORT_TEXT(1019, "意外的 #else"));
        return end;
      }
    }

    UINT depth = 0;
    T_LPCSTR p = begin;
    for(; p < end; ++p)
    {
      if((p = PP_FindPreProcessIdentifier(p, end)) == end) {
        break;
      }

      size_t str_elif_len = 4;

      // if 要放在 ifdef, ifndef 后面
      if(CompareString(p, PREPROCESS_ifdef, 5) || CompareString(p, PREPROCESS_ifndef, 6) || CompareString(p, PREPROCESS_if, 2))
      {
        depth++;
      }
      else if(CompareString(p, PREPROCESS_elif, str_elif_len))
      {
        // pp_stack 不空，说明在预处理域内，直接忽略
        // pp_stack 为空，测试表达式(TODO)
        //if( ! sRankStack.empty()) {
        if(depth) {
          continue;
        }

        if(session > PPCondRank_elif) {
          // ERROR: fatal error C1018: 意外的 #elif
          GetLogger()->OutputErrorW(p, UVS_EXPORT_TEXT(1018, "意外的 #elif"));
          CLBREAK;
        }
        session = PPCondRank_elif;

        CodeParser* pParse = GetSubParser();
        T_LPCSTR line_end = p;
        for(; *line_end != '\n' && line_end < end; line_end++);

        if(line_end == end) {
          // ERROR: 意外的文件结尾
          CLBREAK;
        }

        pParse->Attach(p, (clsize)line_end - (clsize)p, 0, NULL);
        pParse->GenerateTokens();
        const auto& tokens = *pParse->GetTokensArray();
        if(tokens.empty()) {
          // ERROR: “elif” 后面需要表达式
          CLBREAK;
        }

        RTPPCONTEXT ctx;
        ctx.ppend = line_end + 1;
        ctx.stream_end = end;
        ctx.iter_next.pContainer = pParse;
        ctx.iter_next.marker = ctx.ppend;
        ctx.iter_next.length = 0;

        return PP_If(ctx, pParse);
      }
      else if(CompareString(p, PREPROCESS_else, 4))
      {
        // pp_stack 不空，说明在预处理域内，直接忽略
        // pp_stack 为空，转到下行行首
        //if( ! pp_stack.empty()) {
        if( depth || session == PPCondRank_elif) {
          continue;
        }

        if(session >= PPCondRank_else) {
          // ERROR: fatal error C1019: 意外的 #else
          GetLogger()->OutputErrorW(p, UVS_EXPORT_TEXT(9999, "没实现的功能"));
          CLBREAK;
        }
        session = PPCondRank_else;

        p += 4; // "else" 长度
        break;
      }
      else if(CompareString(p, PREPROCESS_endif, 5))
      {
        // 测试了下，vs2010下 #endif 后面随便敲些什么都不会报错
        //if( ! pp_stack.empty()) {
        //  pp_stack.pop();
        if(depth) {
          depth--;
          continue;
        }

        --m_nPPRecursion;
        p += 5; // "endif" 长度
        p = Macro_SkipGapsAndNewLine(p, end);
        if(p == end) {
          return end;
        }
        break;
      }
      else if(CompareString(p, PREPROCESS_line, 4) || CompareString(p, PREPROCESS_file, 4))
      {
        p += 4; // "line"/"file" 长度
        continue;
      }
      else if(CompareString(p, PREPROCESS_undef, 5) || CompareString(p, PREPROCESS_error, 5))
      {
        p += 5; // "error"/"undef"  长度
        continue;
      }
      else if(CompareString(p, PREPROCESS_define, 6) || CompareString(p, PREPROCESS_pragma, 6))
      {
        p += 6; // "define"/"pragma" 长度
        continue;
      }
      else if(CompareString(p, PREPROCESS_include, 7) || CompareString(p, PREPROCESS_message, 7))
      {
        p += 7; // "message"/"include" 长度
        continue;
      }
      else {
        // ERROR: 无效的预处理命令
        T_LPCSTR pend = p;
        for(; pend < end; pend++) {
          if(*pend == 0x20 || *pend == '\t' || *pend == '\r' || *pend == '\n') {
            break;
          }
        }
        clStringW str(p, (size_t)pend - (size_t)p);
        GetLogger()->OutputErrorW(p, UVS_EXPORT_TEXT(1021, "无效的预处理器命令 \"%s\"."), str);
      }
    }
    
    //for(; *p != '\n' && p < end; p++);
    p = Macro_SkipGapsAndNewLine(p, end);
    if(p == end)
    {
      GetLogger()->OutputErrorW(begin, UVS_EXPORT_TEXT(1004, "意外的文件结束"));
      return end;
    }
    return p;
  }

  CodeParser::T_LPCSTR CodeParser::PP_FindPreProcessIdentifier(T_LPCSTR begin, T_LPCSTR end)
  {
    T_LPCSTR p = begin;

    for(; p < end;)
    {
      // 跳过空白找"#"
      if((p = Macro_SkipGapsAndNewLine(p, end)) >= end) {
        return end;
      }
      if(*p == '#') {
        if((p = Macro_SkipGaps(p, end)) >= end) {
          return end;
        }
        return p;
      }

      while(*p != '\n') {
        if(++p >= end) {
          return end;
        }
      }
      ++p;
    }

    return end;
  }

  void CodeParser::PP_UserError(T_LPCSTR position, const clStringW& strText)
  {
    GetLogger()->OutputErrorW(position, UVS_EXPORT_TEXT(1189, "#error : %s"), strText);
  }

  GXBOOL CodeParser::ExpandInnerMacro(TOKEN& token, const TOKEN& line_num)
  {
    ASSERT(line_num.pContainer); // 这个必须是流中的token
    if(token.type == TOKEN::TokenType_String) {
      return FALSE;
    }

    clStringA str;
    if(token == MACRO_FILE)
    {
      str = "\"";
      str.Append(GetLogger()->GetFilenameW());
      str.Append("\"");

      token.type = TOKEN::TokenType_String;
      token.Set(m_pContext->Strings, str);
      return TRUE;
    }
    else if(token == MACRO_LINE)
    {
      str.AppendInteger32(GetLogger()->GetLine(line_num));
      token.type = TOKEN::TokenType_Integer;
      token.Set(m_pContext->Strings, str);
      return TRUE;
    }
    return FALSE;
  }

  CodeParser::T_LPCSTR CodeParser::Macro_SkipGapsAndNewLine(T_LPCSTR p, T_LPCSTR end)
  {
    while((*p == '\t' || *p == 0x20 || *p == '\r' || *p == '\n') && p < end)
    {
      ++p;
    }
    return p;

  }

  CodeParser::T_LPCSTR CodeParser::Macro_SkipGaps(T_LPCSTR p, T_LPCSTR end)
  {
    do {
      p++;
    } while ((*p == '\t' || *p == 0x20) && p < end);
    return p;
  }

  GXBOOL CodeParser::CompareString(T_LPCSTR str1, T_LPCSTR str2, size_t count)
  {
    GXBOOL bSame = GXSTRNCMP(str1, str2, count) == 0;
    const TChar* c = str1 + count;
    ASSERT(c != '\0');
    return bSame && (c == m_pEnd || _CL_NOT_(
      (*c >= 'a' && *c <= 'z') ||
      (*c >= 'A' && *c <= 'Z') ||
      *c == '_' ));
  }

  //////////////////////////////////////////////////////////////////////////

  //void CLogger::OutputErrorW(GXUINT code, ...)
  //{
  //  const char* ptr = NULL;
  //  for (auto it = m_aTokens.rbegin(); it != m_aTokens.rend(); ++it) {
  //    if(it->pContainer) {
  //      ptr = it->marker;
  //    }
  //  }

  //  va_list arglist;
  //  va_start(arglist, code);
  //  m_pMsg->VarWriteErrorW(TRUE, ptr, code, arglist);
  //  va_end(arglist);
  //  return;
  //}

  void CLogger::OutputMissingSemicolon(const TOKEN* ptkLocation)
  {
    OutputErrorW(ptkLocation, UVS_EXPORT_TEXT2(2143, "语法错误 : 缺少“;”", this));
  }

  void CLogger::VarOutputErrorW(const TOKEN* pLocation, GXUINT code, va_list arglist) const
  {
#ifdef REDUCE_ERROR_MESSAGE
    if(code < c_nErrorIdLimit &&
      (m_nErrorCount >= c_nMaxErrorCount || m_nSessionError > c_nMaxSessionError))
    {
      return;
    }
#endif // REDUCE_ERROR_MESSAGE

    if(pLocation)
    {
      if(pLocation->bPhony) {
        T_LPCSTR pOriginMarker = static_cast<const ArithmeticExpression*>(pLocation->pContainer)->GetOriginPtr(pLocation);
        if(pOriginMarker)
        {
          m_pMsg->VarWriteErrorW(TRUE, pOriginMarker, code, arglist);
          return;
        }
        //auto it = m_PhonyTokenDict.find(pLocation - &m_aTokens.front());
        //if(it != m_PhonyTokenDict.end()) {
        //  m_pMsg->VarWriteErrorW(TRUE, it->second.ori_marker, code, arglist);
        //  return;
        //}
      }
      else {
        m_pMsg->VarWriteErrorW(TRUE, pLocation->marker, code, arglist);
        return;
      }
    }

    // pLocation 为空或者 bPhony 下没找到
    m_pMsg->VarWriteErrorW(TRUE, (GXSIZE_T)0, code, arglist);
  }

  void CLogger::OutputErrorW(const GLOB& glob, GXUINT code, ...) const
  {
    va_list  arglist;
    va_start(arglist, code);
    VarOutputErrorW(glob.GetFrontToken(), code, arglist);
    va_end(arglist);
  }

  void CLogger::OutputErrorW(const SYNTAXNODE* pNode, GXUINT code, ...) const
  {
    va_list  arglist;
    va_start(arglist, code);
    VarOutputErrorW(&pNode->GetAnyTokenAPB(), code, arglist);
    va_end(arglist);
  }

  void CLogger::OutputErrorW(const TOKEN& token, GXUINT code, ...) const
  {
    va_list  arglist;
    va_start(arglist, code);
    VarOutputErrorW(&token, code, arglist);
    va_end(arglist);
  }

  void CLogger::OutputErrorW(const TOKEN* pToken, GXUINT code, ...) const
  {
    va_list  arglist;
    va_start(arglist, code);
    VarOutputErrorW(pToken, code, arglist);
    va_end(arglist);
  }

  void CLogger::OutputErrorW(T_LPCSTR ptr, GXUINT code, ...)
  {
    va_list  arglist;
    va_start(arglist, code);
    m_pMsg->VarWriteErrorW(TRUE, ptr, code, arglist);
    va_end(arglist);
  }

  //////////////////////////////////////////////////////////////////////////

  CodeParser* CodeParser::GetRootParser()
  {
    CodeParser* pRoot = this;
    while(pRoot->m_pParent) {
      pRoot = pRoot->m_pParent;
    }
    return pRoot;
  }

  clBuffer* CodeParser::OpenIncludeFile(const clStringW& strFilename)
  {
    auto it = m_sIncludeFiles.find(strFilename);
    if(it != m_sIncludeFiles.end()) {
      return it->second;
    }

    clBuffer* pBuffer = NULL;
    GXHRESULT hr = m_pInclude->Open(Include::IncludeType_Local, strFilename, &pBuffer);
    if(GXSUCCEEDED(hr)) {
      m_sIncludeFiles[strFilename] = pBuffer;
      return pBuffer;
    }

    return NULL;
  }

  CodeParser::T_LPCSTR CodeParser::GetOriginPtr(const TOKEN* pToken) const
  {
    ASSERT(pToken->bPhony);
    auto it = m_PhonyTokenDict.find(pToken - &m_aTokens.front());
    return it != m_PhonyTokenDict.end() ? it->second.ori_marker : NULL;
  }

#ifdef ENABLE_SYNTAX_VERIFY

  const TOKEN* CodeParser::GetVariableNameWithoutSeamantic(const GLOB& glob)
  {
    // 递归解析带语意的变量定义
    // <variable> [: seamantic1] [: seamantic2] ...
    if(glob.IsToken())
    {
      return glob.pTokn;
    }
    else if(glob.IsNode())
    {
      if(glob.pNode->CompareOpcode(':') ||
        glob.pNode->mode == SYNTAXNODE::MODE_Subscript0 ||
        glob.pNode->mode == SYNTAXNODE::MODE_Subscript)
      {
        return GetVariableNameWithoutSeamantic(glob.pNode->Operand[0]);
      }
      else
      {
        return NULL;
      }
    }
    CLBREAK;
    return NULL;
  }

  // 获得去除语意的数组变量
  const SYNTAXNODE::GLOB* CodeParser::GetVariableDeclWithoutSeamantic(const GLOB& glob)
  {
    // 递归解析带语意的变量定义
    // <variable>["[m]"]["[n]"] [: seamantic1] [: seamantic2] ...
    if(glob.IsToken())
    {
      // 初始化列表不一定赋值给数组: float3 v = {1,1,1};
      return &glob;
    }
    else if(glob.IsNode())
    {
      if(glob.pNode->mode == SYNTAXNODE::MODE_Subscript0 ||
        glob.pNode->mode == SYNTAXNODE::MODE_Subscript)
      {
        return &glob;
      }
      else if(glob.pNode->CompareOpcode(':'))
      {
        return GetVariableDeclWithoutSeamantic(glob.pNode->Operand[0]);
      }
      else
      {
        PARSER_BREAK(glob);
      }
    }
    PARSER_NOTIMPLEMENT;
    CLBREAK;
    return NULL;
  }

  GXBOOL CodeParser::Verify_MacroFormalList(const MACRO_TOKEN::List& sFormalList)
  {
    clStringW str;
    if(sFormalList.size() <= 1)
    {
      if(sFormalList.front().IsIdentifier() == FALSE) {
        GetLogger()->OutputErrorW(sFormalList.front(), UVS_EXPORT_TEXT(2010, "“%s”: 宏形参表中的意外"), sFormalList.front().ToString(str).CStr());
        return FALSE;
      }
      return TRUE;
    }

    for(auto it1 = sFormalList.begin(); it1 != sFormalList.end(); ++it1)
    {
      if(it1->IsIdentifier() == FALSE) {
        GetLogger()->OutputErrorW(*it1, UVS_EXPORT_TEXT(2010, "“%s”: 宏形参表中的意外"), it1->ToString(str).CStr());
        return FALSE;
      }

      auto it2 = it1;
      for(++it2; it2 != sFormalList.end(); ++it2)
      {
        if(*it1 == *it2) {
          GetLogger()->OutputErrorW(*it2, UVS_EXPORT_TEXT(2009, "宏形式“%s”重复使用"), it2->ToString(str).CStr());
          return FALSE;
        }
      }
    }
    return TRUE;
  }

  GXBOOL CodeParser::Verify_VariableDefinition(NameContext& sNameSet, const SYNTAXNODE* pNode, GXBOOL bConstVariable, GXBOOL bMember)
  {
    ASSERT(pNode->mode == SYNTAXNODE::MODE_Definition); // 只检查定义

    if(pNode->Operand[0].IsToken())
    {
      if(*pNode->Operand[0].pTokn == "const")
      {
        // TODO: 合并一下
        if(pNode->Operand[1].IsToken())
        {
          clStringA strType;
          TYPEDESC sDesc = { TYPEDESC::TypeCate_Empty, &sNameSet };
          pNode->Operand[1].pTokn->ToString(strType);

          if(sNameSet.GetType(*pNode->Operand[1].pTokn) || NameContext::TestIntrinsicType(&sDesc, strType))
          {
            // ERROR: "const float;" 形式
            GetLogger()->OutputErrorW(*pNode->Operand[1].pTokn, UVS_EXPORT_TEXT(4091, "没有声明变量"));
            return FALSE;
          }
          else
          {
            // ERROR: "const i;" 形式
            GetLogger()->OutputErrorW(*pNode->Operand[1].pTokn, UVS_EXPORT_TEXT(4430, "缺少类型说明符"));
            return FALSE;
          }
        }
        else if(pNode->Operand[1].IsNode())
        {
          if(pNode->Operand[1].pNode->mode == SYNTAXNODE::MODE_Definition)
          {
            // 递归
            return Verify_VariableDefinition(sNameSet, pNode->Operand[1].pNode, TRUE);
          }
          else {
            // ERROR: "const i = 0;" 形式
            GetLogger()->OutputErrorW(*pNode->Operand[1].pTokn, UVS_EXPORT_TEXT(4430, "缺少类型说明符"));
            return FALSE;
          }
        }
        else {
          // ERROR: "const;" 形式
          GetLogger()->OutputErrorW(*pNode->Operand[1].pTokn, UVS_EXPORT_TEXT(4430, "缺少类型说明符"));
          return FALSE;
        }
      }
      else {
        const TYPEDESC* pType = sNameSet.GetType(*pNode->Operand[0].pTokn);
        if(pType == NULL) {
          clStringW strW;
          pNode->Operand[0].pTokn->ToString(strW);
          GetLogger()->OutputErrorW(pNode->Operand[0].pTokn, UVS_EXPORT_TEXT(2062, "“%s”: 意外的类型"), strW.CStr());
          return FALSE;
        }
        // ASSERT is type
      }
    }
    else {
      CLBREAK;
    }

    return Verify_VariableTypedDefinition(sNameSet, *pNode->Operand[0].pTokn, pNode->Operand[1], bConstVariable, bMember);
  }

  GXBOOL CodeParser::Verify_VariableTypedDefinition(NameContext& sNameSet, const TOKEN& tkType, const GLOB& second_glob, GXBOOL bConstVariable, GXBOOL bMember)
  {
    const TYPEDESC* pRightTypeDesc = NULL;

    //const GLOB& second_glob = pNode->Operand[1];
    const TOKEN* ptkVar = NULL;
    const TYPEDESC* pType = NULL;
    clStringA strType;

    tkType.ToString(strType);

    if(second_glob.IsToken())
    {
      ptkVar = second_glob.pTokn;
      pType = sNameSet.RegisterVariable(strType, ptkVar);
    }
    else if(second_glob.IsNode())
    {
      if(second_glob.pNode->mode == SYNTAXNODE::MODE_Subscript) // 下标
      {
        pType = sNameSet.RegisterMultidimVariable(strType, second_glob.pNode, NULL);
      }
      else if(second_glob.pNode->mode == SYNTAXNODE::MODE_Subscript0) // 自适应下标
      {
        CLBREAK;
      }
      else if(second_glob.pNode->CompareOpcode(':')) // 语义
      {
        //ptkVar = &second_glob.pNode->GetAnyTokenAB();
        ptkVar = GetVariableNameWithoutSeamantic(second_glob);
        if(ptkVar)
        {
          pType = sNameSet.RegisterVariable(strType, ptkVar);
          ASSERT(pType || sNameSet.GetLastState() != NameContext::State_Ok);
          return TRUE;
        }
        return FALSE;
      }
      else if(second_glob.pNode->mode == SYNTAXNODE::MODE_Assignment) // 赋值
      {
        // 初始化列表不一定赋值给数组: float3 v = {1,1,1};
        // [Doc\变量定义]使用初始化列表赋值的变量，左值先于右值检查
        // int p = 0;
        // void func_0() {
        //   float p = p + 1; // 右值的p是int类型的全局变量
        // }
        // void func_1() {
        //   float p[] = {p + 1}; // 错误，右值的p是float[]
        // }
        // [/Doc]


        //const GLOB& rInitList = second_glob.pNode->Operand[1];
        GLOB& right_glob = second_glob.pNode->Operand[1];
        if(right_glob.IsNode() == FALSE || right_glob.pNode->mode != SYNTAXNODE::MODE_InitList) {
          GetLogger()->OutputErrorW(tkType, UVS_EXPORT_TEXT(5048, "预期应该是初始化列表"));
          return FALSE;
        }
        else if(right_glob.pNode->Operand[0].ptr == NULL) {
          GetLogger()->OutputErrorW(tkType, UVS_EXPORT_TEXT(5049, "初始化列表不应该为空"));
          return FALSE;
        }

        const GLOB* pVarableDecl = GetVariableDeclWithoutSeamantic(second_glob.pNode->Operand[0]);

        if(pVarableDecl)
        {
          pType = sNameSet.RegisterVariable(strType, pVarableDecl, NULL, &right_glob);
        }

        if(pVarableDecl == NULL || pType == NULL)
        {
          return FALSE;
        }

        pRightTypeDesc = pType;
        const TOKEN* ptkVarName = NULL;
        // 处理自适应长度数组类型，忽略 "int a = {0};" 这种形式
        if(pVarableDecl->IsNode())
        {
          const GLOB* pAutoLenGlob = pVarableDecl;
          while(pAutoLenGlob->pNode->Operand[0].IsNode()) {
            pAutoLenGlob = &pAutoLenGlob->pNode->Operand[0];
          }
          
          ptkVarName = pAutoLenGlob->pNode->Operand[0].pTokn;
          if(pAutoLenGlob->pNode->Operand[1].ptr == NULL) // 自适应长度类型，这个必然为空“[]”
          {
            VALUE value;
            value.set(VALUE::Rank_Unsigned, &pRightTypeDesc->sDimensions.back());
            SetRepalcedValue(pAutoLenGlob->pNode->Operand[1], value);
          }
        }
        else
        {
          ptkVarName = pVarableDecl->pTokn;
        }

        // 用整理过的初始化列表替换原有的列表
        const VARIDESC* pVariDesc = sNameSet.GetVariableDesc(ptkVarName);
        ASSERT(pVariDesc != NULL && pVariDesc->glob.CompareAsNode(SYNTAXNODE::MODE_InitList));
        ASSERT(right_glob.CompareAsNode(SYNTAXNODE::MODE_InitList));
        right_glob.pNode = pVariDesc->glob.pNode;

        ASSERT(pType || sNameSet.GetLastState() != NameContext::State_Ok);
      }
      else if(second_glob.pNode->CompareOpcode('=')) // 赋值
      {
        VALUE value;
        VALUE::State state = VALUE::State_OK;
        const GLOB& right_glob = second_glob.pNode->Operand[1];

        if(bConstVariable)
        {
          state = CalculateValueAsConstantDefinition(value, sNameSet, right_glob);
          if(state != VALUE::State_OK && state != VALUE::State_Call && state != VALUE::State_Identifier) {
            return FALSE;
          }
        }

        const size_t nErrorCount = DbgErrorCount();

        pRightTypeDesc = InferRightValueType(sNameSet, NULL, NULL, right_glob, ptkVar);
        ptkVar = GetVariableNameWithoutSeamantic(second_glob.pNode->Operand[0]);

        if(pRightTypeDesc == NULL || ptkVar == NULL) {
          // InferRightValueType2和GetVariableWithoutSeamantic内部应该输出错误
          ASSERT(DbgErrorCount() > nErrorCount);
          return NULL;
        }

        if(bConstVariable)
        {
          // TODO: 检查value与strType类型是否匹配, 比如一个“string s = 23;”是非法的
          if(state == VALUE::State_OK) {
            pType = sNameSet.RegisterVariable(strType, ptkVar, &value, &right_glob);
            SetRepalcedValue(right_glob/*second_glob.pNode->Operand[1]*/, value);
          }
          else {
            pType = sNameSet.RegisterVariable(strType, ptkVar, NULL, &right_glob);
          }
        }
        else
        {
          pType = sNameSet.RegisterVariable(strType, ptkVar);
        }

        //if(pRightTypeDesc == NULL) {
        //  // 右侧类型推导失败，注册变量名（RegisterVariable）后再退出，
        //  // 这样后面就不会报找不到变量的错误
        //  return FALSE;
        //}

        ASSERT(pType || sNameSet.GetLastState() != NameContext::State_Ok);
      }
      else if(second_glob.pNode->CompareOpcode(','))
      {
        const size_t nErrorCount = DbgErrorCount();
        ASSERT(bMember == FALSE); // 成员变量定义在结构体解析时已经展开了，不应该存在“,”并列式
        if(Verify_VariableTypedDefinition(sNameSet, tkType, second_glob.pNode->Operand[0], bConstVariable, FALSE) == FALSE ||
          Verify_VariableTypedDefinition(sNameSet, tkType, second_glob.pNode->Operand[1], bConstVariable, FALSE) == FALSE)
        {
          ASSERT(DbgErrorCount() > nErrorCount);
          return FALSE;
        }
        return TRUE;
      }
      else
      {
        // 意料之外的变量定义语法
        PARSER_BREAK(second_glob);
        CLBREAK;
        return FALSE;
      }
    }
    else
    {
      PARSER_BREAK(second_glob);
      // 定义但是没写名字，报错
      return FALSE;
    }

    //ASSERT(pNode->Operand[0].IsToken()); // 外面的拆解保证不会出现这个

    //pNode->Operand[0].pTokn->ToString(strType);

    //// 检查变量名
    if(pType == NULL)
    {
      clStringW strW;
      switch(sNameSet.GetLastState())
      {
      case NameContext::State_TypeNotFound:
      {
        strW = strType;
        GetLogger()->OutputErrorW(tkType, UVS_EXPORT_TEXT(5012, "“%s”: 类型未定义"), strW.CStr());
        return FALSE;
      }
      case NameContext::State_DuplicatedVariable:
      {
        GetLogger()->OutputErrorW(*ptkVar, UVS_EXPORT_TEXT(2371, "“%s”: 重定义"), ptkVar->ToString(strW).CStr());
        return FALSE;
      }
      case NameContext::State_DefineAsType:
      {
        ptkVar = ptkVar != NULL ? ptkVar : second_glob.GetFrontToken();
        GetLogger()->OutputErrorW(*ptkVar, UVS_EXPORT_TEXT(5013, "“%s”: 变量已经被定义为类型"), ptkVar->ToString(strW).CStr());
        return FALSE;
      }
      case NameContext::State_VariableIsNotIdentifier:
      {
        ptkVar = ptkVar != NULL ? ptkVar : second_glob.GetFrontToken();
        GetLogger()->OutputErrorW(*ptkVar, UVS_EXPORT_TEXT(3000, "预期是一个变量名 : \"%s\""), ptkVar->ToString(strW).CStr());
        return FALSE;
      }
      case NameContext::State_RequireConstantExpression:
        break;  // 内部已处理
      default:
        CLBREAK; // 预期之外的状态
        break;
      }
    }

    if(second_glob.IsNode())
    {
      ASSERT(pRightTypeDesc != NULL ||
        second_glob.pNode->mode == SYNTAXNODE::MODE_Subscript); // 下标情况下可以没有pRightTypeDesc

      if(pRightTypeDesc)
      {
        const TOKEN& token = *second_glob.GetFrontToken();
        if(TryTypeCasting(pType, pRightTypeDesc, &token) == FALSE)
        {
          clStringW strFrom = pRightTypeDesc->name;
          clStringW strTo = pType->name;
          GetLogger()->OutputErrorW(token, UVS_EXPORT_TEXT(2440, "“=”: 无法从“%s”转换为“%s”"), strFrom.CStr(), strTo.CStr());
          return FALSE;
        }
      }
    }
    return TRUE;
  }

  GXBOOL CodeParser::Verify_Chain(const SYNTAXNODE* pNode, NameContext& sNameContext)
  {
    GXBOOL result = TRUE;
    RecursiveNode<const SYNTAXNODE>(this, pNode, [this, &result, &sNameContext]
    (const SYNTAXNODE* pNode, int depth) -> GXBOOL
    {
      // return FALSE 表示不再遍历后面的节点

      if(pNode->mode == SYNTAXNODE::MODE_Block)
      {
        // TODO: 测试 int a; if(a) {a} 能否编译通过(block中没有分号)
        ASSERT(pNode->Operand[1].ptr == NULL ||
          (pNode->Operand[1].IsToken() && *pNode->Operand[1].pTokn == ';'));

        if(pNode->Operand[0].IsNode())
        {
          NameContext sBlockNameContext(s_szNCName_Block, &sNameContext);
          result = Verify_Chain(pNode->Operand[0].pNode, sBlockNameContext);
        }
        else if(pNode->Operand[0].IsToken())
        {
          if(InferRightValueType(sNameContext, NULL, NULL, pNode->Operand[0], pNode->pOpcode) == NULL)
          {
            result = FALSE;
          }
        }
        return FALSE;
      }
      else if(pNode->mode == SYNTAXNODE::MODE_Chain ||
        pNode->mode == SYNTAXNODE::MODE_Flow_ForInit ||
        pNode->mode == SYNTAXNODE::MODE_Flow_ForRunning ||
        pNode->mode == SYNTAXNODE::MODE_Flow_While ||
        pNode->mode == SYNTAXNODE::MODE_Flow_DoWhile
        )
      {
        return TRUE;
      }
      else if(pNode->mode == SYNTAXNODE::MODE_Flow_If)
      {
        const TYPEDESC* pTypeDesc = InferRightValueType(sNameContext, NULL, NULL, pNode->Operand[0], NULL);
        result = result && (pTypeDesc != NULL);
        ASSERT(pNode->Operand[1].IsNode() && pNode->Operand[1].pNode->mode == SYNTAXNODE::MODE_Block);
        return TRUE;
      }
      else if(pNode->mode == SYNTAXNODE::MODE_Flow_ElseIf)
      {
        ASSERT(pNode->Operand[0].IsNode() && pNode->Operand[0].pNode->mode == SYNTAXNODE::MODE_Flow_If);
        return TRUE;
      }
      else if(pNode->mode == SYNTAXNODE::MODE_Flow_Else)
      {
        ASSERT(pNode->Operand[0].IsNode() && pNode->Operand[0].pNode->mode == SYNTAXNODE::MODE_Flow_If);
        ASSERT(pNode->Operand[1].IsNode() && pNode->Operand[1].pNode->mode == SYNTAXNODE::MODE_Block);
        return TRUE;
      }
      else if(pNode->mode == SYNTAXNODE::MODE_Flow_Continue)
      {
        ASSERT(pNode->Operand[0].IsToken() && *pNode->Operand[0].pTokn == "continue");
        ASSERT(pNode->Operand[1].ptr == NULL);
        return FALSE;
      }
      else if(pNode->mode == SYNTAXNODE::MODE_Flow_Break)
      {
        ASSERT(pNode->Operand[0].IsToken() && *pNode->Operand[0].pTokn == "break");
        ASSERT(pNode->Operand[1].ptr == NULL);
        return FALSE;
      }
      else if(pNode->mode == SYNTAXNODE::MODE_Flow_Discard)
      {
        ASSERT(pNode->Operand[0].IsToken() && *pNode->Operand[0].pTokn == "discard");
        ASSERT(pNode->Operand[1].ptr == NULL);
        return FALSE;
      }
      else if(pNode->mode == SYNTAXNODE::MODE_Flow_For)
      {
        NameContext sFlowForSet("for", &sNameContext);
        if(pNode->Operand[0].IsNode() && _CL_NOT_(Verify_Chain(pNode->Operand[0].pNode, sFlowForSet))) // FIXME: 这里不能用Verify_Block
        {
          result = FALSE;
        }

        if(pNode->Operand[1].IsNode() && _CL_NOT_(Verify_Block(pNode->Operand[1].pNode, &sFlowForSet)))
        {
          result = FALSE;
        }
        return FALSE;
      }
      else if(pNode->mode == SYNTAXNODE::MODE_Definition)
      {
        if(Verify_VariableDefinition(sNameContext, pNode) == FALSE) {
          result = FALSE;
        }
        return FALSE; // 不再递归
      }
      else if(pNode->mode == SYNTAXNODE::MODE_StructDef)
      {
        clStringA str;
        // TODO: 注册结构体类型
        //sNameSet.RegisterType(pNode->Operand[0].pTokn->ToString(str), TYPEDESC::TypeCate_Struct);
        return FALSE;
      }
      else if(pNode->mode == SYNTAXNODE::MODE_Opcode)
      {
        // TODO: 写的不完整
        if(pNode->pOpcode)
        {
          if(*pNode->pOpcode == '=' ||
            *pNode->pOpcode == "-=" || *pNode->pOpcode == "+=" ||
            *pNode->pOpcode == "*=" || *pNode->pOpcode == "/=" || *pNode->pOpcode == "%=" ||
            *pNode->pOpcode == "<<=" || *pNode->pOpcode == ">>=" ||
            *pNode->pOpcode == "&=" || *pNode->pOpcode == "|=" || *pNode->pOpcode == "^=" )
          {
            // 表达式中如果左侧出现错误就不再检查右侧，主要是防止重复信息太多
            // TODO: 需要验证左值
            const size_t nErrorCount = DbgErrorCount();
            const TYPEDESC* pRightTypeDesc = InferRightValueType(sNameContext, NULL, NULL, pNode->Operand[1], pNode->pOpcode);
            const TYPEDESC* pLeftTypeDesc = Verify2_LeftValue(sNameContext, pNode->Operand[0], *pNode->pOpcode);
            
            if(pRightTypeDesc == NULL || pLeftTypeDesc == NULL)
            {
              ASSERT(DbgErrorCount() > nErrorCount);
              return FALSE;
            }

            if(TryTypeCasting(pLeftTypeDesc, pRightTypeDesc, pNode->pOpcode) == FALSE)
            {
              if(*pNode->pOpcode == "*=" && IsComponent(NULL, pRightTypeDesc, pLeftTypeDesc))
              {
              }
              else
              {
                clStringW strFrom = pRightTypeDesc->name;
                clStringW strTo = pLeftTypeDesc->name;
                GetLogger()->OutputErrorW(pNode->GetAnyTokenPAB(), UVS_EXPORT_TEXT(2440, "“=”: 无法从“%s”转换为“%s”"), strFrom.CStr(), strTo.CStr());
                result = FALSE;
              }
            }
          }
          else if(pNode->pOpcode->unary)
          {
            for(int i = 0; i < 2; i++)
            {
              if(pNode->Operand[i].ptr)
              {
                if(InferRightValueType(sNameContext, NULL, NULL, pNode->Operand[i], pNode->pOpcode) == NULL)
                {
                  result = FALSE;
                }
                break;
              }
            }
          }
          else
          {
            if(InferRightValueType(sNameContext, NULL, NULL, pNode->Operand[1], pNode->pOpcode) == NULL)
            {
              result = FALSE;
            }
          }
        }
        else
        {
          CLBREAK; // SYNTAXNODE::MODE_Opcode 模式没有 Opcode
        }
        return FALSE;
      }
      else if(pNode->mode == SYNTAXNODE::MODE_Return)
      {
        // error C2059: 语法错误:“return”
        const TYPEDESC* pTypeTo = sNameContext.GetReturnType();
        if(pTypeTo->cate == TYPEDESC::TypeCate_Void)
        {
          // error C2562: <function name>:“void”函数返回值
          ASSERT(pNode->Operand[0].IsToken());
          if(pNode->Operand[1].ptr)
          {
            GetLogger()->OutputErrorW(*pNode->Operand[0].pTokn, UVS_EXPORT_TEXT(2562, "“void”函数返回值"));
            result = FALSE;
          }
          return FALSE;
        }

        VALUE_CONTEXT vctx(sNameContext);
        vctx.pLogger = GetLogger();
        const TYPEDESC* pTypeFrom = InferType(vctx, pNode->Operand[1]);
        //const TYPEDESC* pTypeTo = sNameSet.GetType(szReturnType);
        ASSERT(pTypeTo);

        if(pTypeFrom == NULL) {
          result = FALSE;
        }
        else if(TryTypeCasting(pTypeTo, pTypeFrom, pNode->Operand[0].pTokn) == FALSE)
        {
          // error C2440: “return”: 无法从“TypeFrom”转换为“TypeTo”
          clStringW strFrom = pTypeFrom->name;
          clStringW strTo = pTypeTo->name;
          GetLogger()->OutputErrorW(*pNode->Operand[0].pTokn, UVS_EXPORT_TEXT(2440, "“=”: 无法从“%s”转换为“%s”"), strFrom.CStr(), strTo.CStr());
          result = FALSE;
        }
        return FALSE;
      }
      else if(pNode->mode == SYNTAXNODE::MODE_FunctionCall)
      {
        VALUE_CONTEXT vctx(sNameContext, FALSE);
        vctx.pLogger = GetLogger();
        InferFunctionReturnedType(vctx, pNode);
        return FALSE; // 不再遍历后面的节点
      }

      PARSER_BREAK(pNode); // 应该处理 pNode->mode
      return TRUE;
    });

    return result;
  }

  GXBOOL CodeParser::Verify_Block(const SYNTAXNODE* pNode, const NameContext* pParentSet)
  {
    NameContext sNameContext(s_szNCName_Block, pParentSet);
    return Verify_Chain(pNode, sNameContext);
  }

  GXBOOL CodeParser::Verify_StructMember(NameContext& sParentSet, const clStringA& strStructName, const SYNTAXNODE& rNode)
  {
    GXBOOL result = TRUE;
    NameContext* pStructMemberSet = new NameContext(s_szNCName_Block, &sParentSet, NULL);
    RecursiveNode<const SYNTAXNODE>(this, &rNode, [this, &result, pStructMemberSet](const SYNTAXNODE* pNode, int depth) -> GXBOOL
    {
      if(pNode->mode == SYNTAXNODE::MODE_Definition)
      {
        const GLOB& second_glob = pNode->Operand[1];
        const TOKEN* ptkVar = NULL;
        const TYPEDESC* pType = NULL;
        clStringA strType;

        pNode->Operand[0].pTokn->ToString(strType);
        pType = pStructMemberSet->GetType(strType);
        if(pType == NULL)
        {
          clStringW strW = strType;
          GetLogger()->OutputErrorW(*ptkVar, UVS_EXPORT_TEXT(2062, "“%s”: 意外的类型"), strW.CStr());
          return FALSE;
        }

        if(second_glob.IsToken())
        {
          ptkVar = second_glob.pTokn;
          pType = pStructMemberSet->RegisterVariable(strType, ptkVar);
        }
        else if(second_glob.IsNode())
        {
          if(second_glob.pNode->mode == SYNTAXNODE::MODE_Subscript) // 下标
          {
            pType = pStructMemberSet->RegisterMultidimVariable(strType, second_glob.pNode, NULL);
          }
          else if(second_glob.pNode->mode == SYNTAXNODE::MODE_Subscript0) // 自适应下标
          {
            CLBREAK;
          }
          else if(second_glob.pNode->CompareOpcode(':')) // 语义
          {
            //ptkVar = &second_glob.pNode->GetAnyTokenAB();
            ptkVar = GetVariableNameWithoutSeamantic(second_glob);
            if(ptkVar)
            {
              pType = pStructMemberSet->RegisterVariable(strType, ptkVar);
              ASSERT(pType || pStructMemberSet->GetLastState() != NameContext::State_Ok);
              return TRUE;
            }
            return FALSE;
          }
          else {
            PARSER_BREAK(second_glob); // 没有处理的错误
            result = FALSE;
            return FALSE;
          }
        }
        else {
          PARSER_BREAK(second_glob); // 没有处理的错误
          result = FALSE;
          return FALSE;
        }

        if(pType == NULL)
        {
          clStringW strW;
          switch(pStructMemberSet->GetLastState())
          {
          case NameContext::State_DuplicatedVariable:
            GetLogger()->OutputErrorW(*ptkVar, UVS_EXPORT_TEXT(2030, "“%s”: 结构成员重定义"), ptkVar->ToString(strW));
            break;
          case NameContext::State_TypeNotFound:
            CLBREAK; // 上面判断了，这里不应该有
            break;
          default:
            PARSER_BREAK(second_glob); // 没有处理的错误
            break;
          }
          result = FALSE;
        }
        return FALSE;

        // TODO: 检查sMemberName是否有错误状态
      } // SYNTAXNODE::MODE_Definition
      return TRUE;
    });

    sParentSet.RegisterStructContext(strStructName, pStructMemberSet);
    return result;
  }

  const TYPEDESC* CodeParser::Verify2_LeftValue(const NameContext& sNameSet, const GLOB& left_glob, const TOKEN& opcode)
  {
    //Any of the following C expressions can be l-value expressions:
    //  An identifier of integral, floating, pointer, structure, or union type
    //  A subscript ([ ]) expression that does not evaluate to an array
    //  A member-selection expression (–> or .)
    //  A unary-indirection (*) expression that does not refer to an array
    //  An l-value expression in parentheses
    //  A const object (a nonmodifiable l-value)

    //clStringA strA;
    clStringW strW;
    const TYPEDESC* pTypeDesc = NULL;
    if(left_glob.IsToken())
    {
      pTypeDesc = sNameSet.GetVariable(left_glob.pTokn/*->ToString(strA)*/);
      if(pTypeDesc == NULL)
      {
        //strW = strA;
        left_glob.pTokn->ToString(strW);
        GetLogger()->OutputErrorW(*left_glob.pTokn, UVS_EXPORT_TEXT(2065, "“%s”: 未声明的标识符"), strW.CStr());
        return NULL;
      }
      return pTypeDesc;
    }
    else if(left_glob.IsNode())
    {
      const SYNTAXNODE* pLeftNode = left_glob.pNode;
      if(pLeftNode->mode == SYNTAXNODE::MODE_Opcode)
      {
        if(pLeftNode->CompareOpcode('.'))
        {
          VALUE_CONTEXT vctx(sNameSet, FALSE);
          vctx.pLogger = GetLogger();
          pTypeDesc = InferMemberType(vctx, left_glob.pNode);
          if(pTypeDesc == NULL)
          {
            GetLogger()->OutputErrorW(left_glob.pNode->GetAnyTokenAB(), UVS_EXPORT_TEXT(5022, "不明确的成员变量"));
            return NULL;
          }
          return pTypeDesc;
        }
        else
        {
          PARSER_BREAK(left_glob);
        }
      }
      else if(pLeftNode->mode == SYNTAXNODE::MODE_Subscript)
      {
        VALUE_CONTEXT vctx(sNameSet, FALSE);
        vctx.pLogger = GetLogger();
        pTypeDesc = InferSubscriptType(vctx, pLeftNode);
        return pTypeDesc;
      }
      else {
        // error C2106: “=”: 左操作数必须为左值
        GetLogger()->OutputErrorW(opcode, UVS_EXPORT_TEXT(2106, "“=”: 左操作数必须为左值"));
        return NULL;
      }
    }    
    GetLogger()->OutputErrorW(opcode, UVS_EXPORT_TEXT(5010, "“=”前缺少左值"));
    return NULL;
  }

  const TYPEDESC* CodeParser::InferUserFunctionType(const NameContext& sNameSet, const TYPEDESC::CPtrList& sTypeList, const SYNTAXNODE* pFuncNode)
  {
    // 返回ERROR_TYPEDESC表示语法出现错误而失败
    // 返回NULL表示没找到匹配函数

    cllist<const FUNCDESC*> aUserFunc;
    sNameSet.GetMatchedFunctions(pFuncNode->Operand[0].pTokn, sTypeList.size(), aUserFunc);
    for (auto iter_func = aUserFunc.begin(); iter_func != aUserFunc.end(); ++iter_func)
    {
      int i = 0;
      size_t nConfirm = 0;
      ASSERT((*iter_func)->sFormalTypes.size() == sTypeList.size());
      for (auto iter_arg = sTypeList.begin(); iter_arg != sTypeList.end(); ++iter_arg, ++i)
      {
        const TYPEDESC* pArgumentTypeDesc = *iter_arg;
        const TOKEN* ptkFormal = (*iter_func)->sFormalTypes[i];
        const TYPEDESC* pFormalTypeDesc = sNameSet.GetType(*ptkFormal);
        //ASSERT(pFormalTypeDesc != NULL);

        if(pFormalTypeDesc == NULL)
        {
          clStringW strW;
          GetLogger()->OutputErrorW(*ptkFormal, UVS_EXPORT_TEXT(2062, "意外的类型“%s”"), ptkFormal->ToString(strW).CStr());
          return ERROR_TYPEDESC;
        }

        // TODO: TryTypeCasting 最后这个参数只是大致定位,改为更准确的!
        if (pArgumentTypeDesc == NULL)
        {
          return ERROR_TYPEDESC; // 无法推导参数类型
        }
        else if (TryTypeCasting(pFormalTypeDesc, pArgumentTypeDesc, pFuncNode->Operand[0].pTokn)) {
          nConfirm++;
        }
        else {
          break;
        }
      }

      if(nConfirm == sTypeList.size()) {
        return sNameSet.GetType((*iter_func)->ret_type);
      }
    }
    return NULL;
  }

  const TYPEDESC* CodeParser::InferFunctionReturnedType(VALUE_CONTEXT& vctx, const SYNTAXNODE* pFuncNode)
  {
    CHECK_VALUE_CONTEXT;
    ASSERT(pFuncNode->mode == SYNTAXNODE::MODE_FunctionCall);
    if(pFuncNode->Operand[0].IsNode())
    {
      // “float[2](0, 1)” 这种形式
      clStringW strW;
      GetLogger()->OutputErrorW(pFuncNode->Operand[0], UVS_EXPORT_TEXT(5042, "语法错误: 无效的函数“%s”"),
        pFuncNode->GetAnyTokenAPB().ToString(strW).CStr());
      return NULL;
    }
    else if(pFuncNode->Operand[0].ptr == NULL)
    {
      PARSER_BREAK(pFuncNode); // 想不出啥情况会出现这个情况
    }

    const TYPEDESC* pRetType = NULL;

    // 拆解参数成列表
    TYPEDESC::CPtrList sArgumentsTypeList;
    SYNTAXNODE::GlobList sExprList;
    BreakComma(sExprList, pFuncNode->Operand[1]);
    VALUE_CONTEXT vctx_param(vctx);
    GXBOOL bNeedValue = vctx.bNeedValue; // 参数中如果不是常量，则整个参数表都不储存常量
    for(auto it = sExprList.begin(); it != sExprList.end(); ++it)
    {
      vctx_param.ClearValue();
      const TYPEDESC* pTypeDesc = InferType(vctx_param, *it);
      if(vctx_param.result != ValueResult_OK && vctx_param.result != ValueResult_Variable) {
        vctx.result = vctx_param.result;
        return NULL;
      }
      ASSERT(pTypeDesc);
      sArgumentsTypeList.push_back(pTypeDesc);
      
      if(vctx.bNeedValue && bNeedValue)
      {
        if(vctx_param.count == 0) {
          bNeedValue = FALSE;
        }
        vctx.pool.insert(vctx.pool.end(), vctx_param.pValue, vctx_param.pValue + vctx_param.count);
      }
    }

    if(bNeedValue) {
      // 必须等pool稳定之后才能确定指针位置
      vctx.UsePool();
      //vctx.pValue = &vctx.pool.front();
      //vctx.count  = vctx.pool.size();
    }
    else {
      vctx.pValue = NULL;
      vctx.count  = 0;
      vctx.pool.clear();
    }

    pRetType = InferUserFunctionType(vctx.name_ctx, sArgumentsTypeList, pFuncNode);
    if(pRetType == ERROR_TYPEDESC) {
      return NULL;
    }
    else if(pRetType) {
      vctx.result = ValueResult_OK;
      vctx.pType = pRetType;
      return pRetType;
    }

    clStringA strFunctionName;
    const TYPEDESC* pTypeFunc = NULL;
    if(vctx.name_ctx.IsTypedefedType(pFuncNode->Operand[0].pTokn, &pTypeFunc))
    {
      strFunctionName = pTypeFunc->name;
    }
    else {
      pFuncNode->Operand[0].pTokn->ToString(strFunctionName);
    }

    // mul()比较特殊
    if(strFunctionName == s_szMultiplicationFunc && sArgumentsTypeList.size() == 2)
    {
      pTypeFunc = InferDifferentTypesOfMultiplication(sArgumentsTypeList.front(), sArgumentsTypeList.back());
      if(pTypeFunc) {
        vctx.result = ValueResult_OK;
        vctx.pType = pTypeFunc;
        return pTypeFunc;
      }
    }

    // 通配符形式的内部函数列表
    for(int i = 0; s_functions[i].name != NULL; i++)
    {
      if(strFunctionName == s_functions[i].name)
      {
        if(sArgumentsTypeList.size() == s_functions[i].count)
        {
          size_t n = 0;
          auto it = sArgumentsTypeList.begin();
          auto it_expr = sExprList.begin();
          for(; n < s_functions[i].count; n++, ++it, ++it_expr)
          {
            const TYPEDESC* pTypeDesc = *it; // InferType(sNameSet, *it);
            if(pTypeDesc)
            {
              ASSERT(s_functions[i].type > INTRINSIC_FUNC::RetType_Last);
              if(s_functions[i].type == INTRINSIC_FUNC::RetType_Scaler0) {
                pRetType = pTypeDesc;
              }
              else if(//s_functions[i].type == INTRINSIC_FUNC::RetType_FromName ||
                s_functions[i].type == INTRINSIC_FUNC::RetType_Bool ||
                s_functions[i].type == INTRINSIC_FUNC::RetType_Float4)
              {
              }
              else if(s_functions[i].type == n) {
                pRetType = pTypeDesc;
              }
              else if(s_functions[i].type < 0 || n >= s_functions[i].count) {
                CLBREAK;
              }

              if(TEST_FLAG(s_functions[i].params[n], 8)) // out 修饰
              {
                // FIXME: 如果没有重载或者有重载并且形参数唯一匹配,才输出这条错误消息
                if(it_expr->IsNode()) {
                  //error C2664: “UVShader::sincos”: 不能将参数 2 从“float”转换为“float &”
                  clStringW strFunc = s_functions[i].name;
                  GetLogger()->OutputErrorW(it_expr->pNode->GetAnyTokenAPB(),
                    UVS_EXPORT_TEXT(2664, "“%s”: 参数 %d 不能使用“out”修饰"),
                    strFunc.CStr(), n); // TODO: 没有testcase
                  return NULL;
                }
              }

              // TODO: TryTypeCasting 最后这个参数只是大致定位,改为更准确的!
              if(TryTypeCasting(vctx.name_ctx, (GXDWORD)s_functions[i].params[n], pTypeDesc, pFuncNode->Operand[0].pTokn)) {
                continue;
              }
              break;
            }
            else {
              return NULL;
            }
          }

          if(n == s_functions[i].count) {
            if(s_functions[i].type == INTRINSIC_FUNC::RetType_Scaler0) {
              vctx.pType = vctx.name_ctx.GetType(pRetType->pDesc->component_type);
            }
            else if(s_functions[i].type == INTRINSIC_FUNC::RetType_Bool) {
              vctx.pType = vctx.name_ctx.GetType(STR_BOOL);
            }
            else if(s_functions[i].type == INTRINSIC_FUNC::RetType_Float4) {
              vctx.pType = vctx.name_ctx.GetType(STR_FLOAT4);
            }
            else if(s_functions[i].type >= 0 && s_functions[i].type < (int)sArgumentsTypeList.size()) {
              auto iter_type = sArgumentsTypeList.begin();
              int index = 0;
              while(index < s_functions[i].type) {
                ++iter_type;
                ++index;
              }
              vctx.pType = *iter_type;
            }
            else {
              CLBREAK;
            }

            vctx.ClearValueOnly();
            vctx.result = ValueResult_OK;
            //vctx.pType = pRetType;
            return vctx.pType; //sNameSet.GetType(s_functions[i].type);
          }
        }
      }
    }

    // 确切参数类型的函数列表
    const PERCOMPONENTMATH* pPreCompMath = FindPerComponentMathOperations(strFunctionName);

    //for(int i = 0; s_PreComponentMath[i].name != NULL; i++)
    //{
    if(pPreCompMath)
    {
      int nScalerCount = 0;
      for(auto it = sArgumentsTypeList.begin(); it != sArgumentsTypeList.end(); ++it)
      {
        const TYPEDESC* pTypeDesc = *it; // InferType(sNameSet, *it);
        int R, C;

        pTypeDesc->Resolve(R, C);
        if(C == 0) // pTypeDesc必须是标量或者向量
        {
          if(R == 0) {
            nScalerCount++;
          }
          else {
            nScalerCount += R;
          }
        }
        else {
          break;
        }
      }

      vctx.result = ValueResult_OK;
      if(pPreCompMath->scaler_count == nScalerCount || nScalerCount == 1)
      {
        vctx.pType = vctx.name_ctx.GetType(pPreCompMath->name);
        ASSERT(vctx.pType); // FindPerComponentMathOperations 列表里放了一个不存在的内置类型
        const VALUE::Rank rank = vctx.TypeRank();
        ASSERT(VALUE::IsNumericRank(rank));

        if(vctx.IsNeedValue() &&
          (pPreCompMath->scaler_count > nScalerCount || vctx.pValue->rank != rank))
        {
          ASSERT(vctx.count == 1 || vctx.pValue->rank != rank);
          VALUE value = *vctx.pValue;
          value.CastValueByRank(rank);
          vctx.pool.assign(pPreCompMath->scaler_count, value);
          vctx.UsePool();
        }

        return vctx.pType;
      }
      else
      {
        clStringW str(pPreCompMath->name);
        GetLogger()->OutputErrorW(*pFuncNode->Operand[0].pTokn,
          UVS_EXPORT_TEXT(5039, "“%s”: 参数数量不匹配，参数只提供了%d个标量"), str.CStr(), nScalerCount);
        vctx.result = ValueResult_5039;
        return NULL;
      }

      //break;
    }
    //}

    // TODO: 没有找到名字的提示找不到标识符, 找到名字但是参数不匹配的提示没有找到重载
    //  error C3861: “func”: 找不到标识符
    clStringW strW;
    GetLogger()->OutputErrorW(*pFuncNode->Operand[0].pTokn, UVS_EXPORT_TEXT(3861, "“%s”: 找不到标识符"),
      pFuncNode->Operand[0].pTokn->ToString(strW).CStr());
    vctx.result = ValueResult_3861;

    return NULL;
  }

  const TYPEDESC* CodeParser::InferType(VALUE_CONTEXT& vctx, const GLOB& sGlob)
  {
    ASSERT(vctx.pLogger);
    if(sGlob.IsNode()) {
      return InferType(vctx, sGlob.pNode);
    }
    else if(sGlob.IsToken()) {
      return InferType(vctx, sGlob.pTokn);
    }
    CLBREAK;
  }
  
  ValueResult CodeParser::TokenToValue(VALUE_CONTEXT& vctx, const TOKEN* pToken) const
  {
    ASSERT(pToken->type > TOKEN::TokenType_FirstNumeric && pToken->type < TOKEN::TokenType_LastNumeric); // 外部保证
    
    VALUE val;
    VALUE::State s = val.set(*pToken);      

    if(TEST_FLAG(s, VALUE::State_ErrorMask)) {
      clStringW str;
      if(TEST_FLAG(s, VALUE::State_IllegalNumber))
      {
        vctx.pLogger->OutputErrorW(*pToken, UVS_EXPORT_TEXT2(2041, "非法的数字 : “%s”", vctx.pLogger), pToken->ToString(str).CStr());
      }
      else
      {
        vctx.pLogger->OutputErrorW(*pToken, UVS_EXPORT_TEXT2(2021, "应输入数值, 而不是“%s”", vctx.pLogger), pToken->ToString(str).CStr());
      }
      vctx.result = ValueResult_NotNumeric;
      return vctx.result;
    }

    vctx.result = ValueResult_OK;
    vctx.pType = vctx.name_ctx.GetType(val.rank);
    vctx.pool.push_back(val);
    //vctx.pValue = &vctx.pool.front();
    //vctx.count = vctx.pType->CountOf();
    vctx.UsePool();
    ASSERT(vctx.count == 1); // vctx.pType->CountOf()应该为1

    return vctx.result;
  }

  const TYPEDESC* CodeParser::InferType(VALUE_CONTEXT& vctx, const TOKEN* pToken) const
  {
    // TODO: 直接提示找不到符号？
    if(pToken->type > TOKEN::TokenType_FirstNumeric && pToken->type < TOKEN::TokenType_LastNumeric)
    {
      TokenToValue(vctx, pToken);
      return vctx.pType;
    }

    if(pToken->type == TOKEN::TokenType_String)
    {
      VALUE value;
      value.set(VALUE::Rank_String, pToken->marker);
      vctx.pType = vctx.name_ctx.GetType(s_szString);
      vctx.pool.assign(1, value);
      //vctx.pValue = &vctx.pool.front();
      //vctx.count = vctx.pool.size();
      vctx.UsePool();
      vctx.result = ValueResult_OK;
      return vctx.pType;
    }

    const VARIDESC* pVariDesc = vctx.name_ctx.GetVariableDesc(pToken);
    if(pVariDesc == NULL)
    {
      // C2065: “m”: 未声明的标识符
      clStringW strW;
      vctx.pLogger->OutputErrorW(*pToken, UVS_EXPORT_TEXT2(2065, "“%s”: 未声明的标识符", vctx.pLogger), pToken->ToString(strW).CStr());
      vctx.result = ValueResult_2065;
      return NULL;
    }
    
    vctx.pType = pVariDesc->pDesc;
    if(vctx.bNeedValue) {
      if(vctx.pValue == NULL) // 第一个进入的函数
      {
        const ValuePool* pPool = vctx.name_ctx.GetValuePool(pToken);
        if(pPool)
        {
          vctx.result = ValueResult_OK;
          vctx.pValue = &pPool->front();
          vctx.count = vctx.pType->CountOf();
          ASSERT(vctx.count == pPool->size());
        }
        else
        {
          vctx.result = ValueResult_Variable;
        }
      }
      else
      {
        ASSERT(vctx.count <= pVariDesc->nOffset + vctx.pType->CountOf());
        vctx.result = ValueResult_OK;
        vctx.pValue += pVariDesc->nOffset;
        vctx.count = vctx.pType->CountOf();
      }
    }
    else
    {
      vctx.result = ValueResult_OK;
    }

    return pVariDesc->pDesc;
  }
  
  const TYPEDESC* CodeParser::InferType(VALUE_CONTEXT& vctx, const SYNTAXNODE* pNode)
  {
    ASSERT(pNode->mode != SYNTAXNODE::MODE_Block &&
      pNode->mode != SYNTAXNODE::MODE_Chain);
    CHECK_VALUE_CONTEXT;

    if(pNode->mode == SYNTAXNODE::MODE_FunctionCall)
    {
      return InferFunctionReturnedType(vctx, pNode);
    }
    else if(pNode->mode == SYNTAXNODE::MODE_Subscript)
    {
      return InferSubscriptType(vctx, pNode);
    }
    else if(pNode->mode == SYNTAXNODE::MODE_TypeCast)
    {
      if(pNode->Operand[0].IsToken())
      {
        const TYPEDESC* pCastTypeDesc = vctx.name_ctx.GetType(*pNode->Operand[0].pTokn);
        const TYPEDESC* pSource = InferType(vctx, pNode->Operand[1]);
        if(TryReinterpretCasting(pCastTypeDesc, pSource, pNode->Operand[0].pTokn))
        {
          return pCastTypeDesc;
        }
      }
      else {
        PARSER_BREAK(pNode->Operand[0]);
      }
      return NULL;
    }
    else if(pNode->mode == SYNTAXNODE::MODE_InitList)
    {
      CLBREAK;
    }
    else if(pNode->pOpcode)
    {
      if(pNode->pOpcode->unary)
      {
        ASSERT(
          *pNode->pOpcode == '~' ||
          *pNode->pOpcode == '!' ||
          *pNode->pOpcode == '-' ||
          *pNode->pOpcode == '+' ||
          *pNode->pOpcode == "--" ||
          *pNode->pOpcode == "++" );

        if(pNode->pOpcode->unary_mask == 0x01)
        {
          return InferType(vctx, pNode->Operand[1]);
        }
        else if(pNode->pOpcode->unary_mask == 0x02)
        {
          return InferType(vctx, pNode->Operand[0]);
        }
        else // 11B
        {
          CLBREAK
        }
      }
      else if(*pNode->pOpcode == '.')
      {
        const TYPEDESC* pTypeDesc = InferMemberType(vctx, pNode);
        return pTypeDesc;
      }
      else if(pNode->mode == SYNTAXNODE::MODE_Opcode)
      {
        ASSERT(pNode->pOpcode); // 上面分支判断了，这里防止以后重构遗失这个条件
        if(pNode->Operand[0].ptr == NULL || pNode->Operand[1].ptr == NULL) {
          clStringW strW;
          pNode->pOpcode->ToString(strW);
          GetLogger()->OutputErrorW(pNode->pOpcode, UVS_EXPORT_TEXT(5046, "“%s”缺少必要的操作数"), strW.CStr());
          return NULL;
        }
      }
    }

    const TYPEDESC* pTypeDesc[2] = {NULL, NULL};
    VALUE_CONTEXT v[2] = {vctx.name_ctx, vctx.name_ctx};
    for(int i = 0; i < 2; i++)
    {
      v[i].pLogger = vctx.pLogger;
      pTypeDesc[i] = InferType(v[i], pNode->Operand[i]);
      PARSER_ASSERT(pNode->Operand[i].ptr != NULL, pNode->GetAnyTokenAPB());
      ASSERT(v[i].pType == pTypeDesc[i]);
    }

    if(pTypeDesc[0] == NULL) {
      vctx.result = v[0].result;
      return NULL;
    }
    else if(pTypeDesc[1] == NULL) {
      vctx.result = v[1].result;
      return NULL;
    }
    else // if(pTypeDesc[0] != NULL && pTypeDesc[1] != NULL)
    {
      const TYPEDESC* pResultTypeDesc = InferTypeByOperator(pNode->pOpcode, pTypeDesc[0], pTypeDesc[1]);
      if(pResultTypeDesc) {
        return pResultTypeDesc;
      }
//#if 0
//      const GXBOOL bFirstScaler  = IS_SCALER_CATE(pTypeDesc[0]);
//      const GXBOOL bSecondScaler = IS_SCALER_CATE(pTypeDesc[1]);
//      if(bFirstScaler && bSecondScaler)
//      {
//        VALUE::Rank rank = (VALUE::Rank)clMax(pTypeDesc[0]->pDesc->rank, pTypeDesc[1]->pDesc->rank);
//        vctx.pType = vctx.name_ctx.GetType(rank);
//        if(vctx.pType) {
//          vctx.result = ValueResult_OK;
//        }
//        else {
//          vctx.result = ValueResult_BadRank;
//        }
//        ASSERT(vctx.pType->pDesc->rank >= VALUE::Rank_First && vctx.pType->pDesc->rank <= VALUE::Rank_Last);
//        return vctx.pType;
//      }
//      else if(bFirstScaler && IS_VECMAT_CATE(pTypeDesc[1]))
//      {
//        // TODO: 是否应考虑符号?
//        if(_CL_NOT_(TryTypeCasting(pTypeDesc[1], pTypeDesc[0], &pNode->GetAnyTokenPAB())))
//        {
//          clStringW strFromW(pTypeDesc[0]->name);
//          clStringW strToW(pTypeDesc[1]->name);
//          GetLogger()->OutputErrorW(pNode->GetAnyTokenPAB(), UVS_EXPORT_TEXT(2440, "“=”: 无法从“%s”转换为“%s”"), strFromW.CStr(), strToW.CStr());
//          return NULL;
//        }
//        return pTypeDesc[1];
//      }
//      else if(IS_VECMAT_CATE(pTypeDesc[0]) && bSecondScaler)
//      {
//        // TODO: 是否应考虑符号?
//        if(TryTypeCasting(pTypeDesc[0], pTypeDesc[1], &pNode->GetAnyTokenPAB())) {
//          return pTypeDesc[0];
//        }
//        CLBREAK; // 没处理
//        return NULL;
//      }
//      else if(IS_VECMAT_CATE(pTypeDesc[0]) && IS_VECMAT_CATE(pTypeDesc[1]))
//      {
//        if(pTypeDesc[0]->name == pTypeDesc[1]->name)
//        {
//          //ASSERT(pTypeDesc[0] == pTypeDesc[1]); // 地址应该一样
//          return pTypeDesc[0];
//        }
//        else {
//          return InferDifferentTypesOfCalculations(pNode->pOpcode, pTypeDesc[0], pTypeDesc[1]);
//        }
//      }
//      else {
//        return InferDifferentTypesOfCalculations(pNode->pOpcode, pTypeDesc[0], pTypeDesc[1]);
//      }
//#else
      MergeValueContext(vctx, pNode->pOpcode, v, &pNode->GetAnyTokenPAB());
      return vctx.pType;
//#endif

    }
    return NULL;
  }

  //////////////////////////////////////////////////////////////////////////
  
  const TYPEDESC* CodeParser::InferInitList_Struct(size_t nTopIndex, const TYPEDESC* pRefType, CInitList& rInitList, size_t nDimDepth)
  {
    ASSERT(nTopIndex + pRefType->CountOf() <= rInitList.ValuePoolCount());

    rInitList.DbgListBegin(pRefType->name);

    TYPEDESC::CPtrList sMemberTypeList;
    if(pRefType->GetMemberTypeList(sMemberTypeList) == FALSE) {
      return NULL;
    }

    const GLOB* pGlob = NULL;
    size_t nListDepth = rInitList.BeginList();
    size_t index = 0;

    for(auto it = sMemberTypeList.begin(); it != sMemberTypeList.end(); ++it)
    {
      if((*it)->cate == TYPEDESC::TypeCate_Struct || (*it)->cate == TYPEDESC::TypeCate_MultiDim)
      {
        rInitList.DbgPushString();
        if(RearrangeInitList(nTopIndex + index, *it, rInitList, nDimDepth + 1) == NULL) {
          return NULL;
        }
        index += (*it)->CountOf();
        rInitList.DbgPopString();

        if(rInitList.NeedAlignDepth(nDimDepth, nListDepth)) {
          break;
        }
        else if(rInitList.Empty()) {
          break;
        }
        continue;
      }

      CInitList::Result result = rInitList.CastToValuePool(pRefType, nTopIndex, index);
      switch(result)
      {
      case UVShader::CInitList::Result_Failed:
        return NULL;
        break;
      case UVShader::CInitList::Result_ExpandVecMat:
        nListDepth++;
        break;
      case UVShader::CInitList::Result_Ok:
      case UVShader::CInitList::Result_NotAligned:
        // 不处理
        break;
      default:
        CLBREAK; // 意外的返回值
        break;
      }

      index++;

      if(rInitList.Step(nDimDepth, nListDepth) == FALSE) {
        break;
      }
    }


    if((rInitList.Depth() > nDimDepth || rInitList.Depth() > nListDepth) && rInitList.IsEnd() == FALSE)
    {
      GetLogger()->OutputErrorW(rInitList.GetLocation(), UVS_EXPORT_TEXT(2078, "初始值设定项太多"));
      return NULL;
    }
    else if(rInitList.Depth() == nListDepth)
    {
      rInitList.ClearAlignDepthFlag();
    }

    rInitList.DbgListEnd();
    TRACE("%s\n", rInitList.DbgGetString().CStr());
    return pRefType;
  }

  const TYPEDESC* CodeParser::InferInitList_LinearArray(size_t nTopIndex, const TYPEDESC* pRefType, CInitList& rInitList, size_t nDimDepth)
  {
    ASSERT(nTopIndex + pRefType->CountOf() <= rInitList.ValuePoolCount());
    size_t index;
    const b32 bAdaptedLength = (_CL_NOT_(pRefType->sDimensions.empty()) && pRefType->sDimensions.back() == 0);
    rInitList.DbgListBegin(pRefType->name);
    const size_t nListDepth = rInitList.BeginList();

    const GLOB* pGlob = NULL;
    size_t array_count = bAdaptedLength ? (size_t)-1 : pRefType->sDimensions.back();
    for(index = 0; index < array_count;)
    {
      CInitList::Result result = rInitList.CastToValuePool(pRefType, nTopIndex, index);
      if(result != CInitList::Result_Ok) {
        return NULL;
      }

      index++;
      if(rInitList.Step(nDimDepth, nListDepth) == FALSE) {
        break;
      }
    }


    if((rInitList.Depth() > nDimDepth || rInitList.Depth() > nListDepth) && rInitList.IsEnd() == FALSE)
    {
      GetLogger()->OutputErrorW(rInitList.GetLocation(), UVS_EXPORT_TEXT(2078, "初始值设定项太多"));
      return NULL;
    }
    else if(rInitList.Depth() == nListDepth)
    {
      rInitList.ClearAlignDepthFlag();
    }

    rInitList.DbgListEnd();

    if(bAdaptedLength) {
      clStringA str = rInitList.DbgGetString();
      const size_t len = pRefType->name.GetLength();
      pRefType = pRefType->ConfirmArrayCount(index);
      str.Replace(1, len, pRefType->name);
      rInitList.DbgSetString(str);
    }
    TRACE("%s\n", rInitList.DbgGetString().CStr());
    return pRefType;
  }

  const TYPEDESC* CodeParser::RearrangeInitList(size_t nTopIndex, const TYPEDESC* pRefType, CInitList& rInitList, size_t nDimDepth)
  {
    ASSERT(IS_SCALER_CATE(pRefType) || IS_STRUCT_CATE(pRefType) || pRefType->cate == TYPEDESC::TypeCate_MultiDim);
    // 被这个功能折磨了快三周，没找文档，完全使靠vs2010 C++的黑盒测试出来的
    // 之前比较关注花括号的层级，后来发现“,”才比较重要，一段列表前面的逗号一定是这个列表所在层级的开始(后来发现也不完全是，靠)
    
    size_t index;

    if(IS_STRUCT_CATE(pRefType)) // 结构体
    {
      return InferInitList_Struct(nTopIndex, pRefType, rInitList, nDimDepth);
    }
    else if(IS_SCALER_CATE(pRefType))
    {
      //ASSERT(pValuePool + pRefType->CountOf() <= rInitList.ValuePoolEnd());
      ASSERT(nTopIndex + pRefType->CountOf() <= rInitList.ValuePoolCount());
      const GLOB* pGlob;
      index = 0;
      rInitList.DbgListBegin(pRefType->name);
      CInitList::Result result = rInitList.CastToValuePool(pRefType, nTopIndex, index);
      if(result != CInitList::Result_Ok) {
        return NULL;
      }
      rInitList.DbgListEnd();
      TRACE("%s\n", rInitList.DbgGetString());

      pGlob = rInitList.Step();
      if(pGlob != NULL || _CL_NOT_(rInitList.Empty())) {
        GetLogger()->OutputErrorW(rInitList.GetLocation(), UVS_EXPORT_TEXT(2078, "初始值设定项太多"));
        return NULL;
      }
    }
    else if(IS_SCALER_CATE(pRefType->pElementType)) // 标量数组
    {
      return InferInitList_LinearArray(nTopIndex, pRefType, rInitList, nDimDepth);
    }
    else if(pRefType->cate == TYPEDESC::TypeCate_MultiDim) // 数组
    {
      const b32 bAdaptedLength = (_CL_NOT_(pRefType->sDimensions.empty()) && pRefType->sDimensions.back() == 0);
      const size_t nListDepth = rInitList.BeginList();

      rInitList.DbgListBegin(pRefType->name);
      size_t array_count = bAdaptedLength ? (size_t)-1 : pRefType->sDimensions.back();
      size_t nCountOfElement = pRefType->pElementType->CountOf();
      for(index = 0; index < array_count; index++)
      {
        rInitList.DbgPushString();
        //ASSERT(pValuePool + pRefType->pElementType->CountOf() <= rInitList.ValuePoolEnd());
        ASSERT(nTopIndex + pRefType->pElementType->CountOf() <= rInitList.ValuePoolCount());
        if(RearrangeInitList(nTopIndex + index * nCountOfElement, pRefType->pElementType, rInitList, nDimDepth + 1) == NULL) {
          return NULL;
        }
        rInitList.DbgPopString();
        if(rInitList.NeedAlignDepth(nDimDepth, nListDepth)) {
          index++;
          break;
        }
      }
      rInitList.DbgListEnd();


      if(bAdaptedLength) {
        const size_t len = pRefType->name.GetLength();
        pRefType = pRefType->ConfirmArrayCount(index);
        rInitList.DbgGetString().Replace(1, len, pRefType->name);
      }

      TRACE("%s\n", rInitList.DbgGetString().CStr());

      if(nDimDepth == 1 && rInitList.Empty() == FALSE && rInitList.IsEnd() == FALSE)
      {
        GetLogger()->OutputErrorW(rInitList.GetLocation(), UVS_EXPORT_TEXT(2078, "初始值设定项太多"));
        return NULL;
      }
    }
    else
    {
      CLBREAK;
    }

    return pRefType;
  }

  const TYPEDESC* CodeParser::InferInitList(ValuePool* pValuePool, NameContext& sNameSet, const TYPEDESC* pRefType, GLOB* pInitListGlob)
  {
    // pInitListGlob 输入初始化列表，同时返回一初始化列表
    ASSERT(pInitListGlob->CompareAsNode(SYNTAXNODE::MODE_InitList)); // 外部保证
    ASSERT(pRefType != NULL);

    m_aDbgExpressionOperStack.clear();
    const size_t nErrorCount = DbgErrorCount();

    CInitList il(this, sNameSet, pInitListGlob);

    const size_t count = il.GetMaxCount(pRefType);
    VALUE* pValues = new VALUE[count];
    memset(pValues, 0, sizeof(VALUE) * count); // 暂时还不用调用构造函数
    il.SetValuePool(pValues, count);


    const TYPEDESC* pTypeDesc = RearrangeInitList(0, pRefType, il, 1);
    if(pTypeDesc)
    {
      size_t copy_count = count;
      if(pRefType->cate == TYPEDESC::TypeCate_MultiDim && pRefType->sDimensions.back() == 0)
      {
        copy_count = pTypeDesc->CountOf();
        ASSERT(copy_count != 0);
      }
      pValuePool->assign(pValues, pValues + copy_count);
    }
    SAFE_DELETE_ARRAY(pValues);

    clStringA strRearrange;
    if(nErrorCount == DbgErrorCount()) {
      pInitListGlob->pNode = il.GetRearrangedList();
      DbgDumpSyntaxTree(NULL, pInitListGlob->pNode, 0, &strRearrange);
    }
    else {
      strRearrange = "[ERROR]";
    }
    m_aDbgExpressionOperStack.push_back(strRearrange);
    m_aDbgExpressionOperStack.push_back(il.DbgGetString());
    return pTypeDesc;
  }
#if 0
  const TYPEDESC* CodeParser::InitList_SyncType(NameContext& sNameSet, const TYPEDESC* pRefType, const TYPEDESC* pListType, const GLOB* pElementGlob)
  {
    const TYPEDESC* pElementType = NULL;

    if(pElementGlob->IsNode() && pElementGlob->pNode->mode == SYNTAXNODE::MODE_InitList) {
      pElementType = InferInitList(sNameSet, pRefType, *pElementGlob);
    }
    else {
      pElementType = InferType(sNameSet, *pElementGlob);
    }

    if(pElementType == NULL) {
      return NULL; // 出现错误
    }
    else if(pListType == NULL || pElementType == pListType) {
      return pElementType;
    }
    else if(pElementType->name == STR_VOID) {
      return pListType;
    }
    else if(IS_SCALER_CATE(pElementType) && IS_SCALER_CATE(pListType)) {
      return pElementType->pDesc->rank >= pListType->pDesc->rank ? pElementType : pListType;
    }
    else if(IS_SCALER_CATE(pElementType) && (pListType->cate == TYPEDESC::TypeCate_MultiDim ||
      pListType->cate == TYPEDESC::TypeCate_Vector || pListType->cate == TYPEDESC::TypeCate_Matrix))
    {
      return pListType;
    }
    else if(IS_SCALER_CATE(pListType) && (pElementType->cate == TYPEDESC::TypeCate_MultiDim ||
      pElementType->cate == TYPEDESC::TypeCate_Vector || pElementType->cate == TYPEDESC::TypeCate_Matrix))
    {
      return pElementType;
    }

    // error C2440: “初始化”: 无法从“int”转换为“UVShader::TYPEDESC::TypeCate”
    clStringW strFrom = pElementType->name;
    clStringW strTo = pRefType->name;
    OutputErrorW(*pElementGlob, UVS_EXPORT_TEXT(2440, "“初始化”: 无法从“%s”转换为“%s”"), strFrom.CStr(), strTo.CStr());
    return NULL;
  }
#endif
  const TYPEDESC* CodeParser::InitList_CastType(const TYPEDESC* pLeftType, const TYPEDESC* pListType, size_t nListCount, const GLOB* pLocation)
  {
    if(pListType->IsSameType(pLeftType))
    {
      return pLeftType;
    }
    else if(IS_SCALER_CATE(pListType) && IS_SCALER_CATE(pLeftType))
    {
      if(pListType->pDesc->rank > pLeftType->pDesc->rank)
      {
        // TODO: 数据类型收窄警告？
      }
      return pListType->pDesc->rank >= pLeftType->pDesc->rank
        ? pListType : pLeftType;
    }
    else if(IS_SCALER_CATE(pListType) && (pLeftType->cate == TYPEDESC::TypeCate_Vector))
    {
      // "float" "2" -> "float3"
      // "float" "3" -> "float3"
      int R, C;
      pLeftType->Resolve(R, C);
      //const TYPEDESC* pCompType = sNameSet.GetType(pLeftType->pDesc->component_type);
      ASSERT(pLeftType->pElementType != NULL);
      if(pListType->pDesc->rank > pLeftType->pElementType->pDesc->rank)
      {
        // TODO: 数据类型收窄警告？
      }

      if(nListCount <= (size_t)R)
      {
        return pLeftType;
      }
      // Jump to "初始值设定项太多"
    }
    else if(IS_SCALER_CATE(pListType) && (pLeftType->cate == TYPEDESC::TypeCate_MultiDim))
    {
      // "float" "2" -> "float[3]"
      // "float" "3" -> "float[3]"
      ASSERT(pLeftType->pElementType != NULL);
      if(pListType->pDesc->rank > pLeftType->pElementType->pDesc->rank)
      {
        // TODO: 数据类型收窄警告？
      }

      if(nListCount <= pLeftType->CountOf())
      {
        return pLeftType;
      }
      // Jump to "初始值设定项太多"
    }
    else if(pListType->cate == TYPEDESC::TypeCate_MultiDim && pLeftType->cate == TYPEDESC::TypeCate_MultiDim)
    {
      // "float[4]" "2" -> "float[4][3]"
      // "float[4]" "3" -> "float[4][3]"

      ASSERT(pLeftType->pElementType != NULL);
      if(pListType->pDesc->rank > pLeftType->pElementType->pDesc->rank)
      {
        // TODO: 数据类型收窄警告？
      }

      if(nListCount <= pLeftType->CountOf())
      {
        return pLeftType;
      }

      // Jump to "初始值设定项太多"
    }
    else if(pListType->cate == TYPEDESC::TypeCate_Vector && pLeftType->cate == TYPEDESC::TypeCate_Vector)
    {
      CLBREAK; // 没实现
      if(pListType == pLeftType || pListType->name == pLeftType->name) {
        //return sNameSet.RegisterArrayType(pBaseType, sExprList.size());
      }
      CLNOP
    }
    else if(pLeftType->pElementType->IsSameType(pListType)) // pLeftType是pListType的数组类型
    {
      return pLeftType;
    }
    else
    {
      CLBREAK;
    }
    // error C2078: 初始值设定项太多
    GetLogger()->OutputErrorW(*pLocation, UVS_EXPORT_TEXT(2078, "初始值设定项太多"));
    return NULL;
  }

  const TYPEDESC* CodeParser::InferMemberType(VALUE_CONTEXT& vctx, const SYNTAXNODE* pNode)
  {
    CHECK_VALUE_CONTEXT;
    ASSERT(pNode->mode == SYNTAXNODE::MODE_Opcode && pNode->CompareOpcode('.')); // 外部保证
    // ab.cd.ef 解析为
    // [.] [[.] [ab] [cd]] [ef]

    const TYPEDESC* pTypeDesc = NULL;
    if(pNode->Operand[0].IsToken())
    {
      //pTypeDesc = vctx.name_ctx.GetVariable(pNode->Operand[0].pTokn);
      //if(pTypeDesc == NULL) {
      //  clStringW strToken;
      //  GetLogger()->OutputErrorW(*pNode->Operand[0].pTokn, UVS_EXPORT_TEXT(2065, "“%s”: 未声明的标识符"),
      //    pNode->Operand[0].pTokn->ToString(strToken).CStr());
      //  return NULL;
      //}
      //ASSERT(pTypeDesc);

      //if(vctx.bNeedValue)
      //{
      //  const ValuePool* pValueDesc = vctx.name_ctx.GetValuePool(pNode->Operand[0].pTokn);
      //  ASSERT(pValueDesc);

      //  if(vctx.pValue) {
      //    CLBREAK;
      //  }
      //  else {
      //    vctx.pType = pTypeDesc;
      //    vctx.pValue = &pValueDesc->front();
      //    vctx.count = pValueDesc->size();
      //  }
      //}
      InferType(vctx, pNode->Operand[0].pTokn);
      if(vctx.result != ValueResult_OK && vctx.result != ValueResult_Variable) {
        return NULL;
      }
      pTypeDesc = vctx.pType;
    }
    else if(pNode->Operand[0].IsNode())
    {
      SYNTAXNODE* pChildNode = pNode->Operand[0].pNode;
      if(pChildNode->mode == SYNTAXNODE::MODE_Opcode && pChildNode->CompareOpcode('.'))
      {
        pTypeDesc = InferMemberType(vctx, pChildNode);
        ASSERT(pTypeDesc == vctx.pType);
        //ASSERT(vctx.bNeedValue == FALSE); // 断在这里就是没实现计算值的功能
      }
      else if(pChildNode->mode == SYNTAXNODE::MODE_Subscript)
      {
        //// 结构体起始类型，相当于上面的[ab]
        //// 例："float2(a, b).x" => [.][float2(a, b)][x]
        //PARSER_ASSERT(pChildNode->CompareOpcode('.') == FALSE, pNode->Operand[0]); // 不应该出现使用'.'操作符且不是MODE_Opcode的情况
        //pTypeDesc = InferType(vctx, vctx.name_ctx, pChildNode);
        //ASSERT(vctx.bNeedValue == FALSE); // 断在这里就是没实现计算值的功能
        InferSubscriptType(vctx, pChildNode);
        pTypeDesc = vctx.pType;
      }
      else {
        InferType(vctx, pChildNode);
        pTypeDesc = vctx.pType;
      }

      if(vctx.result != ValueResult_OK && vctx.result != ValueResult_Variable) {
        return NULL;
      }
      else if(pTypeDesc == NULL) {
        ASSERT(vctx.pType);
        return NULL;
      }
    }
    else
    {
      CLBREAK;
    }


    if(pNode->Operand[1].IsNode())
    {
      const SYNTAXNODE* pMemberNode = pNode->Operand[1].pNode;
      if(pMemberNode->mode == SYNTAXNODE::MODE_Subscript) // 带下标的成员
      {
        const NameContext* pMemberContext = vctx.name_ctx.GetStructContext(pTypeDesc->name);
        if(pMemberContext)
        {
          //ASSERT(vctx.bNeedValue == FALSE); // 断在这里就是没实现计算值的功能
          VALUE_CONTEXT vctx_member(*pMemberContext, vctx.bNeedValue);
          vctx_member.pValue = vctx.pValue;
          vctx_member.count  = vctx.count;
          vctx_member.pLogger = vctx.pLogger;
          pTypeDesc = InferSubscriptType(vctx_member, pMemberNode);
          if(vctx_member.result != ValueResult_OK && vctx.result != ValueResult_Variable) {
            vctx.result = vctx_member.result;
            return NULL;
          }
          if(vctx_member.pool.empty()) {
            vctx.pValue = vctx_member.pValue;
          }
          else {
            ASSERT(vctx_member.pValue == &vctx_member.pool.front());
            vctx.pool = vctx_member.pool;
            vctx.pValue = &vctx.pool.front();
          }
          vctx.count = vctx_member.count;
          vctx.pType = vctx_member.pType;
          return pTypeDesc;
        }
      }
#if 1
      // 暂时不支持“length()”方法
      // “.length()”求数组长度
      else if(pMemberNode->mode == SYNTAXNODE::MODE_FunctionCall)
      {
        if(pMemberNode->Operand[0].CompareAsToken(s_szLengthFunc) && pMemberNode->Operand[1].ptr == NULL)
        {
#ifdef _DEBUG
          const TOKEN* pMemberToken = pNode->Operand[0].GetBackToken();
          TRACE("length of(%s) = %d\n", pMemberToken->ToString().CStr(), pTypeDesc->sDimensions.back());
#endif
          ASSERT(vctx.bNeedValue == FALSE); // 断在这里就是没实现计算值的功能
          return m_GlobalSet.GetType(STR_INT);
        }
      }
#endif
      else {
      }

      clStringW strW;
      const TOKEN* pToken = pNode->Operand[1].GetFrontToken();
      GetLogger()->OutputErrorW(*pToken, UVS_EXPORT_TEXT(5041, "结构体不支持的操作: “%s”"), pToken->ToString(strW).CStr());
      return NULL;
    }
    else if(pNode->Operand[1].IsToken())
    {
      const NameContext* pMemberContext = vctx.name_ctx.GetStructContext(pTypeDesc->name);
      if(pMemberContext)
      {
        ASSERT(vctx.result == ValueResult_OK || vctx.result == ValueResult_Variable);
        const VARIDESC* pVariDesc = pMemberContext->GetVariableDesc(pNode->Operand[1].pTokn);

        if(pVariDesc)
        {
          vctx.pType = pVariDesc->pDesc;
          if(vctx.IsNeedValue())
          {
            ASSERT(vctx.pValue && vctx.pType);
            vctx.pValue += pVariDesc->nOffset;
            ASSERT(vctx.count >= vctx.pType->CountOf());
            vctx.count = vctx.pType->CountOf();
          }
          return vctx.pType;
        }
        //else
        //{
        //  ASSERT(&vctx.name_ctx == pMemberContext);
        //  pTypeDesc = InferType(vctx, pNode->Operand[1].pTokn);
        //  return pTypeDesc;
        //}
        // 转到函数尾输出错误信息
      }
      else if(pTypeDesc->pDesc && pTypeDesc->pDesc->lpDotOverride)
      {
        DOTOPERATOR_RESULT sDotOperator;
        if(pTypeDesc->pDesc->lpDotOverride(pTypeDesc->pDesc, &sDotOperator, pNode->Operand[1].pTokn))
        {
          vctx.pType = m_GlobalSet.GetType(sDotOperator.strType);
          if(vctx.IsNeedValue())
          {
            if(IS_SCALER_CATE(vctx.pType))
            {
              vctx.pValue = vctx.pValue + sDotOperator.components[0];
              vctx.count = 1;
            }
            else if(IS_VECMAT_CATE(vctx.pType))
            {
              ValuePool temp_pool;
              for(int i = 0; sDotOperator.components[i] != -1; i++)
              {
                temp_pool.push_back(vctx.pValue[sDotOperator.components[i]]);
              }
              vctx.pool.assign(temp_pool.begin(), temp_pool.end());
              vctx.UsePool();
            }
            else {
              CLBREAK;
            }
          }
          return vctx.pType;
        }
      }
    }

    clStringW str1, str2;
    pTypeDesc = NULL;
    GetLogger()->OutputErrorW(*pNode->Operand[1].pTokn, UVS_EXPORT_TEXT(ERR_IS_NOT_MEMBER, "“%s”: 不是“%s”的成员"),
      pNode->Operand[1].pTokn->ToString(str1).CStr(), pNode->Operand[0].GetFrontToken()->ToString(str2).CStr());
    vctx.result = ValueResult_NotStructMember;
    vctx.pType = NULL;
    vctx.ClearValueOnly();

    return pTypeDesc;
  }

  const TYPEDESC* CodeParser::InferSubscriptType(VALUE_CONTEXT& vctx, const SYNTAXNODE* pNode)
  {
    ASSERT(pNode->mode == SYNTAXNODE::MODE_Subscript);

    const TYPEDESC* pTypeDesc = NULL;
    if(pNode->Operand[0].ptr)
    {
      pTypeDesc = InferType(vctx, pNode->Operand[0]);
      if(pTypeDesc == NULL || (vctx.result != ValueResult_OK && vctx.result != ValueResult_Variable)) {
        return NULL;
      }
    }
    else
    {
      CLBREAK;
    }

    return InferSubscriptTypeB(vctx, pNode);
  }

  const TYPEDESC* CodeParser::InferSubscriptTypeB(VALUE_CONTEXT& vctx, const SYNTAXNODE* pNode) // pNode是变量&下标节点
  {
    VALUE_CONTEXT vctx_subscript(vctx);
    const TYPEDESC* pSubscriptType = InferType(vctx_subscript, pNode->Operand[1]);
    if((vctx_subscript.result != ValueResult_OK && vctx_subscript.result != ValueResult_Variable) ||
      vctx_subscript.pType->cate != TYPEDESC::TypeCate_IntegerScaler)
    {
      GetLogger()->OutputErrorW(pNode->GetAnyTokenAB(), UVS_EXPORT_TEXT(2058, "常量表达式不是整型")); // TODO: 定位不准
      return NULL;
    }

    if(vctx.pType->cate == TYPEDESC::TypeCate_MultiDim)
    {
      if(vctx.IsNeedValue())
      {
        ASSERT(vctx_subscript.count == 1);
        ASSERT(vctx_subscript.pValue->nValue64 >= 0); // TODO: 输出错误提示不能为负值
        const size_t element_count = vctx.pType->pElementType->CountOf();
        ASSERT(vctx_subscript.pValue->uValue64 * element_count < vctx.count);
        vctx.pValue += vctx_subscript.pValue->uValue64 * element_count;
        vctx.pType = vctx.pType->pElementType;
        ASSERT(vctx.count >= element_count)
        vctx.count = element_count;
      }
      else
      {
        vctx.pType = vctx.pType->pElementType;
      }
      return vctx.pType;
    }
    else if(IS_STRUCT_CATE(vctx.pType) && vctx.pType->pDesc && vctx.pType->pDesc->lpSubscript)
    {
      vctx.pType = vctx.pType->pDesc->lpSubscript(vctx.pType->pDesc, vctx.name_ctx);
      ASSERT(vctx.pType);
      return vctx.pType;
    }

    GetLogger()->OutputErrorW(*pNode->Operand[0].pTokn, UVS_EXPORT_TEXT(2109, "下标要求数组类型"));
    return NULL;
  }

  const TYPEDESC* CodeParser::InferTypeByOperator(const TOKEN* pOperator, const TYPEDESC* pFirst, const TYPEDESC* pSecond)
  {
    if(pOperator)
    {
      if(*pOperator == ',') {
        // 例: a = 1, b = 2 返回后者类型
        return pSecond;
      }
    }
    return NULL;
  }

  const TYPEDESC* CodeParser::InferDifferentTypesOfCalculations(const TOKEN* pToken, const TYPEDESC* pFirst, const TYPEDESC* pSecond)
  {
    ASSERT(pToken); // 暂时不支持
    if(*pToken == '*')
    {
      return InferDifferentTypesOfMultiplication(pFirst, pSecond);
    }

    CLBREAK;
    return NULL;
  }

  const TYPEDESC* CodeParser::InferDifferentTypesOfMultiplication(const TYPEDESC* pFirst, const TYPEDESC* pSecond)
  {
    // 推导乘法类型，支持矩阵与向量乘法
    if(pFirst->name == pSecond->name) {
      return pFirst;
    }
    else if(pFirst->IsVector() && pSecond->IsMatrix() && IsComponent(pFirst, pSecond, NULL)) {
      return pFirst;
    }
    else if(pFirst->IsMatrix() && pSecond->IsVector() && IsComponent(NULL, pFirst, pSecond)) {
      return pSecond;
    }
    else {}
    return NULL;
  }

  const TYPEDESC* CodeParser::InferRightValueType(NameContext& sNameSet, const TYPEDESC* pLeftTypeDesc, const GLOB* pVarGlob, const GLOB& right_glob, const TOKEN* pLocation)
  {
    // 这个函数外部不输出 Error/Warning
    ASSERT(right_glob.IsNode() || right_glob.IsToken());

    //ASSERT( // 这两个参数要一致
    //  (ptkType == NULL && ppMinorType == NULL) ||
    //  (ptkType != NULL && ppMinorType != NULL));

    if(right_glob.IsNode() && right_glob.pNode->mode == SYNTAXNODE::MODE_InitList)
    {
      CLBREAK; // 不能使用初始化列表
    }

    const size_t nErrorCount = DbgErrorCount();
    VALUE_CONTEXT vctx(sNameSet);
    vctx.pLogger = GetLogger();
    const TYPEDESC* pRightType = InferType(vctx, right_glob);
    if(pRightType == NULL) {
      ASSERT(DbgErrorCount() > nErrorCount); // InferType 内部应该输出错误
    }
    return pRightType;
  }
  
  GXBOOL CodeParser::CompareScaler(GXLPCSTR szTypeFrom, GXLPCSTR szTypeTo)
  {
    return (
      (clstd::strcmpT(szTypeFrom, STR_FLOAT) || clstd::strcmpT(szTypeFrom, STR_HALF) || clstd::strcmpT(szTypeFrom, STR_DOUBLE)) &&
      (clstd::strcmpT(szTypeTo, STR_FLOAT) || clstd::strcmpT(szTypeTo, STR_HALF) || clstd::strcmpT(szTypeTo, STR_DOUBLE)) );
  }

  GXBOOL CodeParser::TryTypeCasting(const NameContext& sNameSet, GXLPCSTR szTypeTo, const TYPEDESC* pTypeFrom, const TOKEN* pLocation)
  {
    const TYPEDESC* pTypeTo = sNameSet.GetType(szTypeTo);
    return TryTypeCasting(pTypeTo, pTypeFrom, pLocation);
  }

  GXBOOL CodeParser::MergeValueContext(VALUE_CONTEXT& vctx, const TOKEN* pOperator, VALUE_CONTEXT* pAB, const TOKEN* pLocation)
  {
    ASSERT(pAB != NULL);
    CHECK_VALUE_CONTEXT;
    // pOperator 为空相当于“=”操作
    // pAB内容可能会改变


    if(IS_SCALER_CATE(pAB[0].pType) && IS_SCALER_CATE(pAB[1].pType))
    {
      if(pAB[0].TypeRank() >= pAB[1].TypeRank()) {
        vctx.pType = pAB[0].pType;
      }
      else {
        vctx.pType = pAB[1].pType;
      }
      ASSERT(pAB[0].pType->CountOf() == pAB[1].pType->CountOf());
    }
    else if(IS_SCALER_CATE(pAB[0].pType) && IS_VECMAT_CATE(pAB[1].pType))
    {
      if(vctx.IsNeedValue() && pAB[0].IsNeedValue() && pAB[1].IsNeedValue()) {
        vctx.pType = pAB[0].CastUpward(pAB[1].pType);
        ASSERT(pAB[0].pType->CountOf() == pAB[1].pType->CountOf());
      }
      else {
        vctx.pType = vctx.MergeType(pAB[0].pType, pAB[1].pType);
      }
    }
    else if(IS_VECMAT_CATE(pAB[0].pType) && IS_SCALER_CATE(pAB[1].pType))
    {
      if(vctx.IsNeedValue() && pAB[0].IsNeedValue() && pAB[1].IsNeedValue()) {
        vctx.pType = pAB[1].CastUpward(pAB[0].pType);
        ASSERT(pAB[0].pType->CountOf() == pAB[1].pType->CountOf());
      }
      else {
        vctx.pType = vctx.MergeType(pAB[1].pType, pAB[0].pType);
      }
    }
    else if(pAB[0].pType->IsSameType(pAB[1].pType))
    {
      vctx.pType = pAB[0].pType;
    }
    else if(pAB[0].pType->CountOf() == pAB[1].pType->CountOf() &&
      pAB[0].pType->cate == TYPEDESC::TypeCate_Vector && pAB[1].pType->cate == TYPEDESC::TypeCate_Vector)
    {
      if(vctx.IsNeedValue() && pAB[0].IsNeedValue() && pAB[1].IsNeedValue())
      {
        if(pAB[0].TypeRank() > pAB[1].TypeRank())
        {
          vctx.pType = pAB[1].CastUpward(pAB[0].pType);
        }
        else if(pAB[0].TypeRank() < pAB[1].TypeRank())
        {
          vctx.pType = pAB[0].CastUpward(pAB[1].pType);
        }
        else // if(pAB[0].TypeRank() == pAB[1].TypeRank())
        {
          vctx.pType = pAB[0].pType;
        }

        ASSERT(pAB[0].pType->CountOf() == pAB[1].pType->CountOf());
      }
      else
      {
        if(pAB[0].TypeRank() >= pAB[1].TypeRank()) {
          vctx.pType = pAB[0].pType;
        }
        else {
          vctx.pType = pAB[1].pType;
        }
      }
    }
    else
    {
      // int3 vs float3, cast type
      // float3 vs float4, error!
      CLBREAK;
    }


    if(vctx.bNeedValue)
    {
       if(
         (pAB[0].result == ValueResult_OK || pAB[0].result == ValueResult_Variable || pAB[0].pValue == NULL) &&
         (pAB[1].result == ValueResult_OK || pAB[1].result == ValueResult_Variable || pAB[1].pValue == NULL) )
       {
         vctx.ClearValueOnly();
         vctx.result = ValueResult_Variable;
         return TRUE;
       }

       VALUE value;
       vctx.result = ValueResult_OK;
       vctx.pool.clear();
       const size_t count = pAB[0].pType->CountOf();
       for(size_t i = 0; i < count; i++)
       {
         if(value.Calculate(*pOperator, pAB[0].pValue[i], pAB[1].pValue[i]) != VALUE::State_OK) {
           CLBREAK;
           vctx.result = ValueResult_Failed;
           return FALSE;
         }
         vctx.pool.push_back(value);
       }

       vctx.UsePool();
       return TRUE;
    }

    vctx.ClearValue();
    vctx.result = ValueResult_Failed;
    return FALSE;

//
//    if(IS_SCALER_CATE(pAB[0].pType) && IS_SCALER_CATE(pAB[1].pType))
//    {
//      if(vctx.bNeedValue)
//      {
//        if(value.Calculate(*pOperator, *(pAB[0].pValue), *(pAB[1].pValue))) {
//          vctx.result = ValueResult_OK;
//        }
//        else {
//          vctx.result = ValueResult_Failed;
//        }
//
//        vctx.pool.assign(1, value);
//        vctx.SetType(clMax(pAB[0].pValue->rank, pAB[1].pValue->rank));
//        vctx.UserPool();
//      }
//      else if(pAB[0].pType->pDesc->rank < pAB[1].pType->pDesc->rank &&
//        (pAB[0].pType->pDesc->rank != VALUE::Rank_Double && pAB[0].pType->pDesc->rank != VALUE::Rank_Float) &&
//        (pAB[1].pType->pDesc->rank != VALUE::Rank_Double && pAB[1].pType->pDesc->rank != VALUE::Rank_Float)
//        ) {
//        clStringW strFrom = pAB[1].pType->name;
//        clStringW strTo = pAB[0].pType->name;
//        GetLogger()->OutputErrorW(*pLocation, UVS_EXPORT_TEXT(4244, "从“%s”转换到“%s”，可能丢失数据"), strFrom.CStr(), strTo.CStr());
//      }
//      return TRUE;
//    }
//    else if(IS_SCALER_CATE(pAB[0].pType) && IS_VECMAT_CATE(pAB[1].pType))
//    {
//      if(vctx.bNeedValue && _CL_NOT_(pOperator == NULL || *pOperator == '='))
//      {
//        const size_t count = pAB[1].pType->CountOf();
//        vctx.result = ValueResult_OK;
//        vctx.pool.clear();
//        for(size_t i = 0; i < count; i++)
//        {
//          if(value.Calculate(*pOperator, *(pAB[0].pValue), pAB[1].pValue[i]) == FALSE) {
//            vctx.result = ValueResult_Failed;
//            return FALSE;
//          }
//          vctx.pool.push_back(value);
//        }
//
//        vctx.pType = pAB[0].pType;
//        vctx.UserPool();
//        return TRUE;
//      }
//      // TODO: 无法将pAB[1].pType类型转换为pAB[0].pType类型
//      CLBREAK;
//    }
//    else if(IS_VECMAT_CATE(pAB[0].pType) && IS_SCALER_CATE(pAB[1].pType))
//    {
//      if(vctx.bNeedValue && _CL_NOT_(pOperator == NULL || *pOperator == '='))
//      {
//        const size_t count = pAB[0].pType->CountOf();
//        vctx.result = ValueResult_OK;
//        vctx.pool.clear();
//        for(size_t i = 0; i < count; i++)
//        {
//          if(value.Calculate(*pOperator, pAB[0].pValue[i], *(pAB[1].pValue)) == FALSE) {
//            vctx.result = ValueResult_Failed;
//            return FALSE;
//          }
//          vctx.pool.push_back(value);
//        }
//
//        vctx.pType = pAB[0].pType;
//        vctx.UserPool();
//        return TRUE;
//      }
//      // TODO: 无法将pAB[1].pType类型转换为pAB[0].pType类型
//      CLBREAK;
//      //return (pAB[0].pType->pDesc && pAB[0].pType->pDesc->component_type &&
//      //  CompareScaler(pAB[1].pType->name, pAB[0].pType->pDesc->component_type));
//    }
//    else if(IS_VECMAT_CATE(pAB[0].pType) && IS_VECMAT_CATE(pAB[1].pType))
//    {
//      CLBREAK;
//#if 0
//      if(vctx.bNeedValue && _CL_NOT_(pOperator == NULL || *pOperator == '='))
//      {
//        const size_t count = pAB[0].pType->CountOf();
//        vctx.result = ValueResult_OK;
//        vctx.pool.clear();
//        for(int i = 0; i < count; i++)
//        {
//          if(value.Calculate(*pOperator, *(pAB[0].pValue), *(pAB[1].pValue)) == FALSE) {
//            vctx.result = ValueResult_Failed;
//            return FALSE;
//          }
//          vctx.pool.push_back(value);
//        }
//
//        vctx.pType = pAB[0].pType;
//        vctx.UserPool();
//        return TRUE;
//      }
//#endif
//      CLBREAK;
//      //if(pAB[0].pType->name == pAB[1].pType->name)
//      //{
//      //  ASSERT(pAB[0].pType->pDesc == pAB[1].pType->pDesc); // 应该同一个名的函数只注册过一次
//      //  return TRUE;
//      //}
//    }
//
//    CLBREAK;
//    return FALSE;
//    //return (pAB[0].pType->name == pAB[1].pType->name);
  }

  GXBOOL CodeParser::TryTypeCasting(const TYPEDESC* pTypeTo, const TYPEDESC* pTypeFrom, const TOKEN* pLocation)
  {
    ASSERT(pTypeTo != NULL && pTypeFrom != NULL);

    if(IS_SCALER_CATE(pTypeTo) && IS_SCALER_CATE(pTypeFrom))
    {
      if(pTypeTo->pDesc->rank < pTypeFrom->pDesc->rank &&
        (pTypeTo->pDesc->rank != VALUE::Rank_Double && pTypeTo->pDesc->rank != VALUE::Rank_Float) &&
        (pTypeFrom->pDesc->rank != VALUE::Rank_Double && pTypeFrom->pDesc->rank != VALUE::Rank_Float)
        ) {
        clStringW strFrom = pTypeFrom->name;
        clStringW strTo = pTypeTo->name;
        GetLogger()->OutputErrorW(*pLocation, UVS_EXPORT_TEXT(4244, "从“%s”转换到“%s”，可能丢失数据"), strFrom.CStr(), strTo.CStr());
      }
      return TRUE;
    }
    else if((IS_VECTOR_CATE(pTypeTo) && IS_VECTOR_CATE(pTypeFrom)) || (IS_MATRIX_CATE(pTypeTo) && IS_MATRIX_CATE(pTypeFrom)))
    {
      return pTypeTo->CountOf() == pTypeFrom->CountOf();
    }
    else if(IS_STRUCT_CATE(pTypeTo) && IS_STRUCT_CATE(pTypeFrom))
    {
      if(pTypeTo->name == pTypeFrom->name)
      {
        ASSERT(pTypeTo->pDesc == pTypeFrom->pDesc); // 应该同一个名的函数只注册过一次
        return TRUE;
      }
    }
    else if(IS_STRUCT_CATE(pTypeTo) && IS_SCALER_CATE(pTypeFrom))
    {
      return (pTypeTo->pDesc && pTypeTo->pDesc->component_type &&
        CompareScaler(pTypeFrom->name, pTypeTo->pDesc->component_type));
    }

    return (pTypeTo->name == pTypeFrom->name);
  }
  
  GXBOOL CodeParser::TryTypeCasting(const NameContext& sNameSet, GXDWORD dwArgMask, const TYPEDESC* pTypeFrom, const TOKEN* pLocation)
  {
    if(TEST_FLAG(dwArgMask, INTRINSIC_FUNC::ArgMask_Scaler))
    {
      if(TryTypeCasting(sNameSet, STR_FLOAT, pTypeFrom, pLocation)) {
        return TRUE;
      }
    }
    
    if(TEST_FLAG(dwArgMask, INTRINSIC_FUNC::ArgMask_Vector))
    {
      if(TryTypeCasting(sNameSet, STR_FLOAT2, pTypeFrom, pLocation) ||
        TryTypeCasting(sNameSet, STR_FLOAT3, pTypeFrom, pLocation) ||
        TryTypeCasting(sNameSet, STR_FLOAT4, pTypeFrom, pLocation))
      {
        return TRUE;
      }
    }
    
    if(TEST_FLAG(dwArgMask, INTRINSIC_FUNC::ArgMask_Matrix))
    {
      if(TryTypeCasting(sNameSet, STR_FLOAT2x2, pTypeFrom, pLocation) ||
        TryTypeCasting(sNameSet, STR_FLOAT2x3, pTypeFrom, pLocation) ||
        TryTypeCasting(sNameSet, STR_FLOAT2x4, pTypeFrom, pLocation) ||
        TryTypeCasting(sNameSet, STR_FLOAT3x2, pTypeFrom, pLocation) ||
        TryTypeCasting(sNameSet, STR_FLOAT3x3, pTypeFrom, pLocation) ||
        TryTypeCasting(sNameSet, STR_FLOAT3x4, pTypeFrom, pLocation) ||
        TryTypeCasting(sNameSet, STR_FLOAT4x2, pTypeFrom, pLocation) ||
        TryTypeCasting(sNameSet, STR_FLOAT4x3, pTypeFrom, pLocation) ||
        TryTypeCasting(sNameSet, STR_FLOAT4x4, pTypeFrom, pLocation))
      {
        return TRUE;
      }
    }
    
    if(
      (TEST_FLAG(dwArgMask, INTRINSIC_FUNC::ArgMask_Sampler1D) && pTypeFrom->cate == TYPEDESC::TypeCate_Sampler1D) ||
      (TEST_FLAG(dwArgMask, INTRINSIC_FUNC::ArgMask_Sampler2D) && pTypeFrom->cate == TYPEDESC::TypeCate_Sampler2D) ||
      (TEST_FLAG(dwArgMask, INTRINSIC_FUNC::ArgMask_Sampler3D) && pTypeFrom->cate == TYPEDESC::TypeCate_Sampler3D) ||
      (TEST_FLAG(dwArgMask, INTRINSIC_FUNC::ArgMask_SamplerCube) && pTypeFrom->cate == TYPEDESC::TypeCate_SamplerCube) )
    {
      return TRUE;
    }

    return FALSE;
  }

  GXBOOL CodeParser::TryReinterpretCasting(const TYPEDESC* pTypeTo, const TYPEDESC* pTypeFrom, const TOKEN* pLocation)
  {
    if(TryTypeCasting(pTypeTo, pTypeFrom, pLocation)) {
      return TRUE;
    }

    PARSER_NOTIMPLEMENT;
    return FALSE;
  }

  GXBOOL CodeParser::IsComponent(const TYPEDESC* pRowVector, const TYPEDESC* pMatrixDesc, const TYPEDESC* pColumnVector)
  {
    ASSERT(pRowVector != NULL || pColumnVector != NULL);
    ASSERT(pRowVector == NULL || pColumnVector == NULL);
    GXLPCSTR szScaler = NULL;
    int R, C;
    int RV, CV;
    szScaler = pMatrixDesc->Resolve(R, C);

    if(pRowVector) {
      if(_CL_NOT_(pRowVector->name.BeginsWith(szScaler))) {
        return FALSE;
      }
      pRowVector->Resolve(RV, CV);
      return RV == R;
    }

    if(_CL_NOT_(pColumnVector->name.BeginsWith(szScaler))) {
      return FALSE;
    }
    pColumnVector->Resolve(RV, CV);
    return RV == C;
  }

#endif // #ifdef ENABLE_SYNTAX_VERIFY

  //////////////////////////////////////////////////////////////////////////

  void CodeParser::MACRO::clear()
  {
    aTokens.clear();
    aFormalParams.clear();
  }

  void CodeParser::MACRO::set(const Dict& dict, const TOKEN::Array& tokens, int begin_at)
  {
    ASSERT(tokens.front() == PREPROCESS_define);
    aTokens.clear();
    ASSERT(TOKEN::DbgCheck(tokens, 0));
    TOKEN::Append(aTokens, begin_at, tokens.begin() + begin_at, tokens.end(), 
      [&](int addi, MACRO_TOKEN& token)
    {
        const auto it = std::find(aFormalParams.begin(), aFormalParams.end(), token);
        if(it != aFormalParams.end()) {
          token.formal_index = it->formal_index; 
          token.type = TOKEN::TokenType_FormalParams;
        }
    });
#ifdef _DEBUG
    TOKEN::Array aDbgToken;
    aDbgToken.insert(aDbgToken.begin(), aTokens.begin(), aTokens.end());
    ASSERT(TOKEN::DbgCheck(aDbgToken, 0));
#endif
    ClearContainer();

    //while(ExpandMacro(dict) > 0); // 反复调用直到返回0 
  }

  void CodeParser::MACRO::ClearContainer()
  {
    for(auto it = aTokens.begin(); it != aTokens.end(); ++it) {
      it->pContainer = NULL;
    }
    for(auto it = aFormalParams.begin(); it != aFormalParams.end(); ++it) {
      it->pContainer = NULL;
    }
  }

  GXHRESULT DefaultInclude::Open(IncludeType eIncludeType, GXLPCWSTR szFileName, clBuffer** ppBuffer)
  {
    clFile file;
    if( ! file.OpenExisting(szFileName)) {
      return GX_FAIL;
    }

    if(file.MapToBuffer(ppBuffer)) {
      return GX_OK; // 文件为空当作成功处理
    }

    return GX_FAIL;
  }

  GXHRESULT DefaultInclude::Close(clBuffer* pBuffer)
  {
    delete pBuffer;
    return GX_OK;
  }

  //////////////////////////////////////////////////////////////////////////

  NameContext::NameContext(GXLPCSTR szName)
    : m_pCodeParser(NULL)
    , m_strName(szName)
    , m_pParent(NULL)
    , m_pVariParent(NULL)
    , m_eLastState(State_Ok)
    , allow_keywords(KeywordFilter_All)
    , m_pReturnType(NULL)
    , m_nCount(0)
  {   
  }

  NameContext::NameContext(GXLPCSTR szName, const NameContext* pParent, const NameContext* pVariParent)
    : m_pCodeParser(pParent->m_pCodeParser)
    , m_strName(szName)
    , m_pParent(pParent)
    , m_pVariParent(pVariParent == reinterpret_cast<NameContext*>(-1) ? pParent : pVariParent)
    , m_eLastState(State_Ok)
    , allow_keywords(KeywordFilter_All)
    , m_pReturnType(NULL)
    , m_nCount(0)
  {
  }

  NameContext::~NameContext()
  {
    Cleanup();
  }

  void NameContext::Cleanup()
  {
    for(auto it = m_StructContextMap.begin(); it != m_StructContextMap.end(); ++it)
    {
      SAFE_DELETE(it->second);
    }

    m_StructContextMap.clear();
    m_ValuePoolMap.clear();
    m_pReturnType = NULL;
    m_TypeMap.clear();
    m_FuncMap.clear();
    m_VariableMap.clear();
    m_StructContextMap.clear();
  }

  void NameContext::Reset()
  {
    Cleanup();
    m_eLastState = State_Ok;
    m_pReturnType = NULL;
    BuildIntrinsicType(); // TODO: CodeParser::Attach每次都调用这个，优化一下
  }
  
  void NameContext::BuildIntrinsicType()
  {
    // 只有Root才能调用这个
    ASSERT(m_pParent == NULL);

    TYPEDESC td = { TYPEDESC::TypeCate_Empty, this };

    // 内置基础类型
    for(int i = 0; s_aBaseType[i].name; i++)
    {
      if(s_aBaseType[i].rank == VALUE::Rank_Float || s_aBaseType[i].rank == VALUE::Rank_Double)
      {
        td.cate = TYPEDESC::TypeCate_FloatScaler;
      }
      else
      {
        td.cate = TYPEDESC::TypeCate_IntegerScaler;
      }
      td.name = s_aBaseType[i].name;
      td.pDesc = &s_aBaseType[i];

      m_TypeMap.insert(clmake_pair(td.name, td));
    }

    // 内置结构体
    for(int i = 0; s_aIntrinsicStruct[i].name; i++)
    {
      //if(strType == s_aIntrinsicStruct[i].name) {
      td.cate = static_cast<TYPEDESC::TypeCate>(s_aIntrinsicStruct[i].cate);
      td.name = s_aIntrinsicStruct[i].name;
      td.pDesc = &s_aIntrinsicStruct[i];
      auto it = m_TypeMap.find(td.pDesc->component_type);
      ASSERT(it != m_TypeMap.end()); // 向量矩阵一定有元素类型
      td.pElementType = &it->second;
      m_TypeMap.insert(clmake_pair(td.name, td));
      //}
    }

    // void
    td.name = STR_VOID;
    td.cate = TYPEDESC::TypeCate_Void;
    td.pDesc = NULL;
    td.pMemberNode = NULL;
    td.sDimensions.clear();
    td.pElementType = NULL;
    m_TypeMap.insert(clmake_pair(td.name, td));

    // 字符串类型
    //if(strType == s_szString) {
    td.cate = TYPEDESC::TypeCate_String;
    td.name = s_szString;
    td.pDesc = NULL;
    m_TypeMap.insert(clmake_pair(td.name, td));

    td.cate = TYPEDESC::TypeCate_Sampler1D;
    td.name = s_szSampler1D;
    td.pDesc = NULL;
    m_TypeMap.insert(clmake_pair(td.name, td));

    td.cate = TYPEDESC::TypeCate_Sampler2D;
    td.name = s_szSampler2D;
    td.pDesc = NULL;
    m_TypeMap.insert(clmake_pair(td.name, td));

    td.cate = TYPEDESC::TypeCate_Sampler3D;
    td.name = s_szSampler3D;
    td.pDesc = NULL;
    m_TypeMap.insert(clmake_pair(td.name, td));

    td.cate = TYPEDESC::TypeCate_SamplerCube;
    td.name = s_szSamplerCube;
    td.pDesc = NULL;
    m_TypeMap.insert(clmake_pair(td.name, td));
    //}
  }

  CLogger* NameContext::GetLogger() const
  {
    return m_pCodeParser->GetLogger();
  }

  GXBOOL NameContext::SetReturnType(GXLPCSTR szTypeName)
  {
    ASSERT(GetReturnType() == NULL);
    m_pReturnType = GetType(szTypeName);
    return (m_pReturnType != NULL);
  }

  const TYPEDESC* NameContext::GetReturnType() const
  {
    return m_pReturnType ? m_pReturnType : 
      (m_pParent ? m_pParent->GetReturnType() : NULL);
  }

  GXBOOL NameContext::TestIntrinsicType(TYPEDESC* pOut, const clStringA& strType)
  {
    ASSERT(pOut->pDesc == NULL && pOut->pMemberNode == NULL); // 强制外部初始化
    CLBREAK;
    return FALSE;
  }

#ifdef ENABLE_SYNTAX_VERIFY
  VALUE::State NameContext::CalculateConstantValue(VALUE& value_out, CodeParser* pParser, const GLOB* pGlob)
  {
    if(pGlob->IsNode())
    {
      return Calculate(value_out, pParser, pGlob->pNode);
    }
    else if(pGlob->IsToken())
    {
      if(pGlob->pTokn->IsIdentifier())
      {
        if(const VALUE* pValue = GetVariableValue(pGlob->pTokn))
        {
          value_out = *pValue;
          return VALUE::State_OK;
        }
      }
      else if(pGlob->pTokn->IsNumeric())
      {
        return value_out.set(*pGlob->pTokn);
      }

      // C2057: 应输入常量表达式
      m_eLastState = State_RequireConstantExpression;
      return VALUE::State_SyntaxError;
    }
    CLBREAK;
  }

  VALUE::State NameContext::Calculate(VALUE& value_out, CodeParser* pMsgLogger, const SYNTAXNODE* pNode) const
  {
    VALUE p[2];
    VALUE::State s = VALUE::State_OK;
    if(pNode->mode == SYNTAXNODE::MODE_FunctionCall)
    {
      if(pNode->Operand[0].IsToken())
      {
        const TOKEN& func_name = *pNode->Operand[0].pTokn;
        VALUE::Rank target_rank = VALUE::Rank_Undefined;
        if(func_name == STR_HALF)
        {
          target_rank = VALUE::Rank_Float;
        }
        else if(func_name == STR_FLOAT)
        {
          target_rank = VALUE::Rank_Float;
        }
        else if(func_name == STR_INT)
        {
          target_rank = VALUE::Rank_Signed;
        }
        else if(func_name == STR_UINT)
        {
          target_rank = VALUE::Rank_Double;
        }
        else if(func_name == STR_DOUBLE)
        {
          target_rank = VALUE::Rank_Double;
        }
        else {
          value_out.SetZero();
          return VALUE::State_Call;
        }

        if(pNode->Operand[1].IsNode())
        {
          if((s = Calculate(value_out, pMsgLogger, pNode->Operand[1].pNode)) != VALUE::State_OK) {
            return s;
          }

          s = value_out.CastValueByRank(target_rank);
        }
        else if(pNode->Operand[1].IsToken())
        {
          GetVariableValue(value_out, pNode->Operand[1].pTokn);

          if(value_out.rank == VALUE::Rank_Undefined) {
            return VALUE::State_Identifier;
          }
          else if(value_out.rank == VALUE::Rank_BadValue) {
            return VALUE::State_BadIdentifier;
          }

          s = value_out.CastValueByRank(target_rank);
        }
        else {
          // int a = int(); 形式
          value_out.rank = VALUE::Rank_Undefined;
          return VALUE::State_Identifier;
        }

        //else {
        //  PARSER_BREAK2(pParser, pNode); // 不是数学类型
        //}

        // 显式类型转换下，数据截断和丢失数据不需要报错
        return (s == VALUE::State_OK || s == VALUE::State_LoseOfData || s == VALUE::State_Truncation)
          ? VALUE::State_OK : s;
      }
      value_out.SetZero();
      return VALUE::State_Call;
    }
    else if(pNode->mode == SYNTAXNODE::MODE_Opcode)
    {

      ASSERT(pNode->pOpcode != NULL);
      if(*pNode->pOpcode == '.')
      {
        //-----------------------------------------
        //<ID_0028>        [.] [surf] [ff3]
        //<ID_0029>      [.] [surf.ff3]<ID_0028> [fx]
        //<ID_0030>    [.] [surf.ff3.fx]<ID_0029> [color]
        //<ID_0031>  [.] [surf.ff3.fx.color]<ID_0030> [x]
        //surf.ff3.fx.color.x
        //-----------------------------------------

        clStringA strTypename;
        VALUE_CONTEXT vctx(*this);
        vctx.pLogger = pMsgLogger->GetLogger();
        const TYPEDESC* pTypeDesc = pMsgLogger->InferMemberType(vctx, pNode);

        if(pTypeDesc && IS_SCALER_CATE(pTypeDesc))
        {
          if(vctx.pValue && vctx.count == 1)
          {
            value_out = *vctx.pValue;
            return VALUE::State_OK;
          }
          value_out.rank = VALUE::Rank_Undefined;
          return VALUE::State_Identifier;
        }
        else if(vctx.result == ValueResult_Undefined) {
          PARSER_BREAK2(pMsgLogger, pNode); // 不是数学类型
        }
      }
      else {
      }
    }
    else if(pNode->mode == SYNTAXNODE::MODE_Subscript)
    {
      VALUE_CONTEXT vctx(*this);
      vctx.pLogger = GetLogger();

      m_pCodeParser->InferSubscriptType(vctx, pNode);
      if(vctx.result != ValueResult_OK || vctx.pType == NULL) {
        return VALUE::State_SyntaxError;
      }
      else if(vctx.count != 1) {
        return VALUE::State_BadIdentifier;
      }
      value_out = *vctx.pValue;
      return VALUE::State_OK;
    }

    for(int i = 0; i < 2; i++)
    {
      if(pNode->Operand[i].IsNode()) {
        s = Calculate(p[i], pMsgLogger, pNode->Operand[i].pNode);
      }
      else if(pNode->Operand[i].IsToken()) {
        if(pNode->Operand[i].pTokn->IsNumeric()) {
          s = p[i].set(*pNode->Operand[i].pTokn);
        }
        else if(pNode->Operand[i].pTokn->IsIdentifier()) {
          const VALUE* pValue = GetVariableValue(pNode->Operand[i].pTokn);
          if(pValue)
          {
            p[i] = *pValue;
          }
          else
          {
            clStringW strW;
            pNode->Operand[i].pTokn->ToString(strW);
            GetLogger()->OutputErrorW(pNode->Operand[i].pTokn, UVS_EXPORT_TEXT(3861, "“%s”: 找不到标识符"), strW.CStr());
            return VALUE::State_BadIdentifier;
          }
        }
        else {
          // error C2059: 语法错误:“%s”
          clStringW strW;
          GetLogger()->OutputErrorW(pNode->Operand[i].pTokn,
            UVS_EXPORT_TEXT(2059, "语法错误:“%s”"),
            pNode->Operand[i].pTokn->ToString(strW).CStr());
          return VALUE::State_SyntaxError;
        }
      }
      else {
        p[i].SetZero();
      }

      if(s == VALUE::State_Call || s == VALUE::State_Identifier ||
        (s & VALUE::State_ErrorMask))
      {
        return s;
      }
    }

    if(pNode->pOpcode == NULL) {
      return VALUE::State_BadOpcode;
    }

    s = VALUE::State(int(s) | int(value_out.Calculate(*(pNode->pOpcode), p[0], p[1])));
    return s;
  }

#endif

  NameContext* NameContext::GetRoot()
  {
    const NameContext* pSet = this;
    while(pSet->m_pParent)
    {
      pSet = pSet->m_pParent;
    }
    return reinterpret_cast<NameContext*>(reinterpret_cast<size_t>(pSet));
  }

  const NameContext* NameContext::GetRoot() const
  {
    const NameContext* pSet = this;
    while(pSet->m_pParent)
    {
      pSet = pSet->m_pParent;
    }
    return pSet;
  }

  void NameContext::SetParser(CodeParser* pCodeParser)
  {
    m_pCodeParser = pCodeParser;
  }

  NameContext::State NameContext::IntRegisterVariable(
    const TYPEDESC** ppType, VARIDESC** ppVariable, const clStringA& strType,
    const TOKEN* ptkVariable, const VALUE* pConstValue, const GLOB* pValueExprGlob)
  {
    const TYPEDESC* pDesc = GetType(strType);
    if(pDesc == NULL)
    {
      clStringW strW = strType;
      GetLogger()->OutputErrorW(ptkVariable, UVS_EXPORT_TEXT(5012, "“%s”: 类型未定义"), strW.CStr());
      return State_TypeNotFound;
    }
    return IntRegisterVariable(ppType, ppVariable, pDesc, ptkVariable, pConstValue, pValueExprGlob);
  }
  
  NameContext::State NameContext::IntRegisterVariable(
    const TYPEDESC** ppType, VARIDESC** ppVariable, const TYPEDESC* pTypeDesc,
    const TOKEN* ptkVariable, const VALUE* pConstValue, const GLOB* pValueExprGlob)
  {
    ASSERT(ptkVariable);

    if(ptkVariable->IsIdentifier() == FALSE)
    {
      return State_VariableIsNotIdentifier;
    }

    // 变量名可以同类型名，但是必须是结构体
    // （就是不能为关键字，没试过typedef的名字行不行）
    const TYPEDESC* pTestVarName = GetType(*ptkVariable);
    if(pTestVarName && pTestVarName->cate != TYPEDESC::TypeCate_Struct) {
      return State_DefineAsType;
    }

    VARIDESC sVariDesc;
    sVariDesc.pDesc = pTypeDesc;
    sVariDesc.nOffset = m_nCount;

    if(pValueExprGlob)
    {
      //VALUEDESC vd = { NULL };

      // 注册变量的ValuePool
      auto iter_find_result = m_ValuePoolMap.find(ptkVariable);
      if(iter_find_result == m_ValuePoolMap.end())
      {
        auto iter_insert_result = m_ValuePoolMap.insert(clmake_pair(ptkVariable, ValuePool()));
        if(iter_insert_result.second) // 这个判断多余吧？
        {
          ValuePool& vp = iter_insert_result.first->second;

          sVariDesc.glob = *pValueExprGlob;
          // 如果是初始化列表，就进行推导
          // 这里会重新输出一个整理过的初始化列表到sVariDesc.glob中
          if(pValueExprGlob->CompareAsNode(SYNTAXNODE::MODE_InitList))
          {
            const size_t nErrorCount = m_pCodeParser->DbgErrorCount();
            sVariDesc.pDesc = m_pCodeParser->InferInitList(&vp, *this, pTypeDesc, &sVariDesc.glob);
            if(sVariDesc.pDesc == NULL) {
              ASSERT(vp.empty());
              ASSERT(nErrorCount != m_pCodeParser->DbgErrorCount()); // 缺少无法转换的提示信息
              return State_CastTypeError;
            }
          }
          else if(pValueExprGlob->CompareAsNode(SYNTAXNODE::MODE_FunctionCall))
          {
            // TODO: 换成Infer函数
            clStringA strFuncName;
            ASSERT(pValueExprGlob->pNode->Operand[0].IsToken());

            const PERCOMPONENTMATH* pPreCompMath = FindPerComponentMathOperations
            (pValueExprGlob->pNode->Operand[0].pTokn->ToString(strFuncName));

            if(pPreCompMath)
            {
              //vd.pValue = new VALUE[pPreCompMath->scaler_count];
              //vd.count = pPreCompMath->scaler_count;
              VALUE value_zero;
              vp.assign(pPreCompMath->scaler_count, value_zero.SetZero());

              SYNTAXNODE::GlobList sInitList;
              int ii = 0;
              CodeParser::BreakComma(sInitList, pValueExprGlob->pNode->Operand[1]);
              for(auto it = sInitList.begin(); it != sInitList.end(); ++it, ii++)
              {
                VALUE::State state = CalculateConstantValue(vp[ii], m_pCodeParser, &*it);
                if(state != VALUE::State_OK) {
                  GetLogger()->OutputErrorW(*it, UVS_EXPORT_TEXT(5054, "无法计算表达式常量"));
                  return State_RequireConstantExpression;
                }
              }
            } // if(pPreCompMath)
          }
          else if(pValueExprGlob->IsToken() &&
            pValueExprGlob->pTokn->type > TOKEN::TokenType_FirstNumeric &&
            pValueExprGlob->pTokn->type < TOKEN::TokenType_LastNumeric)
          {
            VALUE_CONTEXT vctx(*this);
            vctx.pLogger = GetLogger();
            if(m_pCodeParser->TokenToValue(vctx, pValueExprGlob->pTokn) != ValueResult_OK) {
              return State_HasError;
            }
            vp = vctx.pool;
          }

          if(vp.size() == 1) {
            sVariDesc.sConstValue = vp.front();
          }

        } // iter_insert_result.second
      }
    }
    else {
      sVariDesc.glob.ptr = NULL;
    }

    //if(pConstValue) {
    //  sVariDesc.sConstValue = *pConstValue;
    //}
    //else {
    //  sVariDesc.sConstValue.rank = VALUE::Rank_Undefined;
    //  sVariDesc.sConstValue.nValue = 0;
    //}
    auto result = m_VariableMap.insert(clmake_pair(ptkVariable, sVariDesc));
    if(result.second) {
      // 添加成功, 返回type描述
      m_eLastState = State_Ok;
      *ppType = result.first->second.pDesc;
      *ppVariable = &result.first->second;
      m_nCount += sVariDesc.pDesc->CountOf();
      return State_Ok;
    }

    return State_DuplicatedVariable;
  }

  const TYPEDESC* NameContext::RegisterVariable(const clStringA& strType, const GLOB* pVariableDeclGlob, const VALUE* pConstValue/*想去掉*/, const GLOB* pValueExprGlob)
  {
    if(pVariableDeclGlob->IsToken()) {
      return RegisterVariable(strType, pVariableDeclGlob->pTokn, pConstValue, pValueExprGlob);
    }
    else if(pVariableDeclGlob->IsNode()) {
      return RegisterMultidimVariable(strType, pVariableDeclGlob->pNode, pValueExprGlob);
    }
    return NULL;
  }

  const TYPEDESC* NameContext::RegisterVariable(const clStringA& strType, const TOKEN* ptkVariable, const VALUE* pConstValue/*想去掉*/, const GLOB* pValueExprGlob)
  {
    ASSERT(strType.IsIdentifier());
    // PS: 返回值似乎没什么用
    const TYPEDESC* pTypeDesc = NULL;
    VARIDESC* pVariDesc = NULL;
    m_eLastState = IntRegisterVariable(&pTypeDesc, &pVariDesc, strType, ptkVariable, pConstValue, pValueExprGlob);
    return pTypeDesc;
  }

#ifdef ENABLE_SYNTAX_VERIFY

  const TYPEDESC* NameContext::RegisterTypes(const clStringA& strBaseType, const TYPEDESC::DimList_T& sDimensions)
  {
    // int x[2][3][4] 列表储存为 "{4,3,2}"
    // 函数会依次注册"int*4", "int*4*3", "int*4*3*2"这几个类型，最后返回"int*4*3*2"这个类型描述
    // 对于自适应长度类型数组"int x[][3][4]", 最后返回的类型为"int*4*3*0"

    const TYPEDESC* pBaseTypeDesc = GetType(strBaseType);

    TYPEDESC td = { TYPEDESC::TypeCate_MultiDim, this };
    td.name = strBaseType;
    td.pElementType = pBaseTypeDesc;
    td.pDesc = pBaseTypeDesc->pDesc;
    for(auto it = sDimensions.begin(); it != sDimensions.end(); ++it)
    {
      td.name.Append('*').AppendInteger32(*it); // int x[2][3][4] 记为"int*4*3*2"
      td.sDimensions.push_back(*it);

      ASSERT(*it != 0 || (&*it == &sDimensions.back()));

      auto result = m_TypeMap.insert(clmake_pair(td.name, td));
      td.pElementType = &result.first->second;
    }
    return td.pElementType;
  }

  const TYPEDESC* NameContext::RegisterMultidimVariable(const clStringA& strType, const SYNTAXNODE* pNode, const GLOB* pValueExprGlob)
  {
    ASSERT(pNode->mode == SYNTAXNODE::MODE_Subscript || pNode->mode == SYNTAXNODE::MODE_Subscript0); // 外部保证

    TYPEDESC::DimList_T sDimensions;
    const TYPEDESC* pTypeDesc = NULL;
    VARIDESC* pVariDesc = NULL;


    while(TRUE)
    {
      VALUE value;
      VALUE::State state = VALUE::State_OK;
      const GLOB& subscript_glob = pNode->Operand[1];
      const b32 bSelfAdaptionLength = (subscript_glob.ptr == NULL); // TODO: 最高维度才能用自适应长度
      const size_t nErrorCount = m_pCodeParser->DbgErrorCount();
      const GLOB* pFirstGlob = &pNode->Operand[0];
      if(bSelfAdaptionLength)
      {
        // 自适应长度的数组
        // 变量会先被注册为类似“int*2*0”类型，在之后的确定长度后修改为“int*2*3”
        if(pFirstGlob->IsToken())
        {
          m_eLastState = State_SelfAdaptionLength;
          value.SetZero();
        }
        else
        {
          // error C2087: “a”: 缺少下标: int a[2][] = {1,2,3,4};
          GetLogger()->OutputErrorW(*pFirstGlob, UVS_EXPORT_TEXT(2087, "缺少下标"));
          m_eLastState = State_RequireSubscript;
          return NULL;
        }
      }
      else {
        state = CalculateConstantValue(value, m_pCodeParser, &subscript_glob);
      }

      if(state == VALUE::State_OK)
      {
        //int a[-12]; // C2118 负下标
        //int b[1.3]; // C2058 常量表达式不是整型
        //int c[0];   // C2466 不能分配常量大小为 0 的数组
        if(value.rank == VALUE::Rank_Float || value.rank == VALUE::Rank_Double)
        {
          GetLogger()->OutputErrorW(*pFirstGlob->GetFrontToken(), UVS_EXPORT_TEXT(2058, "常量表达式不是整型"));
          m_eLastState = State_HasError;
          return NULL;
        }
        else if(value.nValue64 < 0) {
          GetLogger()->OutputErrorW(*pFirstGlob->GetFrontToken(), UVS_EXPORT_TEXT(2118, "负下标"));
          m_eLastState = State_HasError;
          return NULL;
        }
        else if(_CL_NOT_(bSelfAdaptionLength) && value.nValue64 == 0) {
          GetLogger()->OutputErrorW(*pFirstGlob->GetFrontToken(), UVS_EXPORT_TEXT(2466, "不能分配常量大小为 0 的数组"));
          m_eLastState = State_HasError;
          return NULL;
        }

        // 下标如果不是整形数字，则替换为计算后的常量
        if(subscript_glob.IsNode() || (subscript_glob.pTokn && subscript_glob.pTokn->type != TOKEN::TokenType_Integer))
        {
          m_pCodeParser->SetRepalcedValue(subscript_glob, value);
        }

        sDimensions.push_back((size_t)value.nValue64);
      }
      else if(state == VALUE::State_SyntaxError)
      {
        if(m_eLastState == State_RequireConstantExpression)
        {
          clStringW strW;
          GetLogger()->OutputErrorW(subscript_glob,
            UVS_EXPORT_TEXT(2057, "应输入常量表达式：“%s”"),
            subscript_glob.GetFrontToken()->ToString(strW).CStr());
        }
        else {
          PARSER_BREAK2(m_pCodeParser, subscript_glob);
        }
        return NULL;
      }
      else if(state == VALUE::State_BadIdentifier)
      {
        ASSERT(m_pCodeParser->DbgErrorCount() > nErrorCount); // 确保内部输出了错误消息
        m_eLastState = State_VariableIsNotIdentifier;
        return NULL;
      }
      else if(state == VALUE::State_Identifier)
      {
        GetLogger()->OutputErrorW(subscript_glob, 
          UVS_EXPORT_TEXT(5047, "不支持复杂表达式声明数组长度"));
        m_eLastState = State_RequireConstantExpression;
        return NULL;
      }
      else
      {
        PARSER_BREAK2(m_pCodeParser, subscript_glob);
        return NULL;
      }

      if(pFirstGlob->IsToken())
      {
        const TOKEN* ptkVariable = pFirstGlob->pTokn;
        ASSERT(sDimensions.empty() == FALSE);

        const TYPEDESC* pSizelessTypeDesc = RegisterTypes(strType, sDimensions); // 可能缺少最高维尺寸的数组类型
        TRACE("var \"%s\":\n", pNode->GetAnyTokenAPB().ToString().CStr());
        m_eLastState = IntRegisterVariable(&pTypeDesc, &pVariDesc, pSizelessTypeDesc, ptkVariable, NULL, pValueExprGlob);
        if(m_eLastState == State_Ok) {
          if(pSizelessTypeDesc != pTypeDesc) {
            CLNOP
          }
          pVariDesc->pDesc = pTypeDesc;
          break;// 下面进行初始化列表的推导
        }
        else
        {
          return NULL;
        }
      }
      else if(pFirstGlob->IsNode())
      {
        pNode = pFirstGlob->pNode;
      }
      else
      {
        CLBREAK;
        return NULL;
      }
    } // while

    if(pValueExprGlob == NULL)
    {
      return pTypeDesc;
    }
    else if(_CL_NOT_(pValueExprGlob->IsNode() && pValueExprGlob->pNode->mode == SYNTAXNODE::MODE_InitList))
    {
      // int a[3] = 0; 形式
      GetLogger()->OutputErrorW(*pValueExprGlob, UVS_EXPORT_TEXT(5051, "应该使用初始化列表来初始化数组"));
      return NULL;
    }

    return pTypeDesc;
  }

#endif

  NameContext::State NameContext::GetLastState() const
  {
    return m_eLastState;
  }

  void NameContext::GetMatchedFunctions(const TOKEN* pFuncName, size_t nFormalCount, cllist<const FUNCDESC*>& aMatchedFunc) const
  {
    const NameContext* pRoot = GetRoot(); // 没有局部函数, 所以直接从根查找

    clStringA strFuncName;
    pFuncName->ToString(strFuncName);

    auto it = pRoot->m_FuncMap.find(strFuncName);
    while(it != pRoot->m_FuncMap.end() && it->first == strFuncName)
    {
      if(it->second.sFormalTypes.size() == nFormalCount) {
        aMatchedFunc.push_back(&it->second);
      }
      ++it;
    }
  }

  GXBOOL NameContext::RegisterStruct(const TOKEN* ptkName, const SYNTAXNODE* pMemberNode)
  {
    ASSERT(pMemberNode == NULL || pMemberNode->mode == SYNTAXNODE::MODE_Block);
    TYPEDESC td = {TYPEDESC::TypeCate_Empty, this};
    clStringA strName;
    td.cate = TYPEDESC::TypeCate_Struct;
    td.name = ptkName->ToString(strName);
    td.pMemberNode = pMemberNode;

    auto result = m_TypeMap.insert(clmake_pair(strName, td));

    // 结构体声明可以重复
    if(result.first->second.pMemberNode == NULL) {
      result.first->second.pMemberNode = pMemberNode;
      return TRUE;
    }
    else if(pMemberNode == NULL) { // 定义后也可以声明
      return TRUE;
    }
    return result.second;
  }

  GXBOOL NameContext::RegisterStructContext(const clStringA& strName, const NameContext* pContext)
  {
    auto result = m_StructContextMap.insert(clmake_pair(strName, pContext));
    ASSERT(result.second); // RegisterStruct保证不会重复添加结构体定义
    return result.second;
  }

  GXBOOL NameContext::RegisterFunction(const clStringA& strRetType, const clStringA& strName, const FUNCTION_ARGUMENT* pArguments, int argc)
  {
    ASSERT(m_pParent == NULL);
    FUNCDESC td;
    td.ret_type = strRetType;
    td.name = strName; // ptkName->ToString(strName);

    auto it = m_FuncMap.insert(clmake_pair(strName, td));
    //clStringA str;
    for(int i = 0; i < argc; i++)
    {
      //str.Clear();
      it->second.sFormalTypes.push_back(pArguments[i].ptkType);
    }
    return TRUE;
  }

  GXBOOL NameContext::IsTypedefedType(const TOKEN* ptkTypename, const TYPEDESC** ppTypeDesc) const
  {
    const TYPEDESC* pTypeDesc = GetType(*ptkTypename);
    if(pTypeDesc)
    {
      if(ppTypeDesc) {
        *ppTypeDesc = pTypeDesc;
      }
      return ((*ptkTypename) != pTypeDesc->name); // 字典名与原始名不一致, 说明是typedef的类型
    }
    return FALSE;
  }

  const TYPEDESC* NameContext::GetVariable(const TOKEN* ptkName) const
  {
    auto it = m_VariableMap.find(TokenPtr(ptkName));
    
    return (it != m_VariableMap.end())
      ? it->second.pDesc
      : (m_pVariParent
        ? m_pVariParent->GetVariable(ptkName)
        : ((ptkName->type == TOKEN::TokenType_String) ? GetType("string") : NULL));
  }

  const ValuePool* NameContext::GetValuePool(const TOKEN* ptkName) const
  {
    auto it = m_ValuePoolMap.find(TokenPtr(ptkName));

    return (it != m_ValuePoolMap.end()) ? &it->second
      : (m_pParent ? m_pParent->GetValuePool(ptkName) :  NULL);
  }

  const VALUE* NameContext::GetVariableValue(const TOKEN* ptkName) const
  {
    auto it = m_VariableMap.find(TokenPtr(ptkName));
    return (it != m_VariableMap.end() && it->second.sConstValue.rank != VALUE::Rank_Undefined)
      ? &it->second.sConstValue : (m_pVariParent ? m_pVariParent->GetVariableValue(ptkName) : NULL);
  }

  VALUE& NameContext::GetVariableValue(VALUE& value, const TOKEN* ptkName) const
  {
    // 注意这个变量“未定义”不是Rank_Undefined
    // 找不到： value.rank = VALUE::Rank_BadValue;
    // 不是常量：value.rank = VALUE::Rank_Undefined;
    auto it = m_VariableMap.find(TokenPtr(ptkName));
    if(it == m_VariableMap.end()) {
      if(m_pVariParent) {
        return m_pVariParent->GetVariableValue(value, ptkName);
      }
      value.rank = VALUE::Rank_BadValue;
      return value;
    }
    value = it->second.sConstValue;
    return value;
  }

  const VARIDESC* NameContext::GetVariableDesc(const TOKEN* ptkName) const
  {
    auto it = m_VariableMap.find(TokenPtr(ptkName));
    if(it == m_VariableMap.end()) {
      if(m_pVariParent) {
        return m_pVariParent->GetVariableDesc(ptkName);
      }
      return NULL;
    }
    return &it->second;
  }

  const NameContext* NameContext::GetStructContext(const clStringA& strName) const
  {
    auto it = m_StructContextMap.find(strName);
    return (it != m_StructContextMap.end())
      ? it->second
      : (m_pParent
        ? m_pParent->GetStructContext(strName)
        : NULL);
  }

  const TYPEDESC* NameContext::GetType(const clStringA& strType) const
  {
    auto it = m_TypeMap.find(strType);
    return (it != m_TypeMap.end())
      ? &it->second
      : (m_pParent ? m_pParent->GetType(strType) : NULL);
  }

  const TYPEDESC* NameContext::GetType(const TOKEN& token) const
  {
    clStringA str;
    return GetType(token.ToString(str));
  }

  const TYPEDESC* NameContext::GetType(VALUE::Rank rank) const
  {
    const NameContext* pRoot = GetRoot();
    clStringA strTypeName;
    //{"float"},
    //{ "half" },
    //{ "int" },
    //{ "bool" },
    //{ "uint" },
    //{ "double" },
    
    // TODO: 和s_aBaseType中的字符串做统一, 保证一次修改
    switch(rank)
    {
    case UVShader::VALUE::Rank_Unsigned:
      strTypeName = STR_UINT;
      break;
    case UVShader::VALUE::Rank_Signed:
      strTypeName = STR_INT;
      break;
    case UVShader::VALUE::Rank_Float:
      strTypeName = STR_FLOAT;
      break;
    case UVShader::VALUE::Rank_Double:
      strTypeName = STR_DOUBLE;
      break;
    default:
      CLBREAK;
      return NULL;
    }

    return pRoot->GetType(strTypeName);
  }

  const TYPEDESC* NameContext::ConfirmArrayCount(const TYPEDESC* pTypeDesc, size_t nCount)
  {
    ASSERT(pTypeDesc->sDimensions.back() == 0); // 外部保证
    ASSERT(pTypeDesc->name.EndsWith("*0"));
    clStringA name = pTypeDesc->name;
    name.TrimRight('0');
    name.AppendUInt32(static_cast<u32>(nCount));
    auto result = m_TypeMap.insert(clmake_pair(name, *pTypeDesc));
    if(result.second) {
      result.first->second.name = name;
      result.first->second.sDimensions.back() = nCount;
    }
    return &result.first->second;
  }

  NameContext::State NameContext::TypeDefine(const TOKEN* ptkOriName, const TOKEN* ptkNewName)
  {
    clStringA strOriName;
    TYPEDESC td = { TYPEDESC::TypeCate_Empty, this };
    const TYPEDESC* pDesc = GetType(ptkOriName->ToString(strOriName));
    if(pDesc == NULL) {
      if(TestIntrinsicType(&td, strOriName) == FALSE) {
        return State_TypeNotFound;
      }
      pDesc = &td;
    }

    clStringA strNewName;
    return m_TypeMap.insert(clmake_pair(ptkNewName->ToString(strNewName), *pDesc)).second
      ? State_Ok : State_DuplicatedType;
  }

  //////////////////////////////////////////////////////////////////////////
  // 操作符号重载
  VALUE::State operator|(VALUE::State a, VALUE::State b)
  {
    return VALUE::State((u32)a | (u32)b);
  }

  void operator|=(VALUE::State& a, VALUE::State b)
  {
    a = VALUE::State((u32)a | (u32)b);
  }

  CLogger* TYPEDESC::GetLogger() const
  {
    return pNameCtx->GetLogger();
  }

  GXBOOL TYPEDESC::MatchScaler(const TOKEN* ptkMember, GXLPCSTR scaler_set)
  {
    size_t match_count = 0;
    for(size_t i = 0; i < ptkMember->length; i++)
    {
      for(size_t n = 0; scaler_set[n]; n++)
      {
        if(ptkMember->marker[i] == scaler_set[n]) {
          match_count++;
          break;
        }
      }
    }
    return match_count == ptkMember->length;
  }

  GXLPCSTR TYPEDESC::Resolve(int& R, int& C) const
  {
    // floatN 类型：R=N，C=0
    // floatNxM 类型：R=N，C=M
    if(cate != TypeCate_FloatScaler &&
      cate != TypeCate_IntegerScaler &&
      cate != TypeCate_Vector &&
      cate != TypeCate_Matrix)
    {
      R = C = 0;
      return NULL;
    }

    GXLPCSTR szRxC = NULL;
    GXLPCSTR szScaler = NULL;
    if(name.BeginsWith(STR_HALF))
    {
      szScaler = STR_HALF;
      szRxC = &name[STR_HALF_LENGTH];
    }
    else if(name.BeginsWith(STR_FLOAT))
    {
      szScaler = STR_FLOAT;
      szRxC = &name[STR_FLOAT_LENGTH];
    }
    else if(name.BeginsWith(STR_DOUBLE))
    {
      szScaler = STR_DOUBLE;
      szRxC = &name[STR_DOUBLE_LENGTH];
    }
    else if(name.BeginsWith(STR_INT))
    {
      szScaler = STR_INT;
      szRxC = &name[STR_INT_LENGTH];
    }
    else if(name.BeginsWith(STR_UINT))
    {
      szScaler = STR_UINT;
      szRxC = &name[STR_UINT_LENGTH];
    }
    else
    {
      PARSER_NOTIMPLEMENT;
      TRACE("%s\n", name.CStr());
    }

    R = C = 0;

    if(szScaler && szRxC[0] != '\0')
    {
      R = szRxC[0] - '0';
      ASSERT(R >= 1 && R <= 4);
      if(szRxC[1] != '\0')
      {
        ASSERT(szRxC[1] == 'x');
        C = szRxC[2] - '0';
        ASSERT(C >= 1 && C <= 4);
      }
    }
    return szScaler;
  }

  GXBOOL TYPEDESC::IsVector() const
  {
    return (cate == TypeCate_Vector);
  }

  GXBOOL TYPEDESC::IsMatrix() const
  {
    return (cate == TypeCate_Matrix);
  }

  GXBOOL TYPEDESC::IsSameType(const TYPEDESC* pOtherTypeDesc) const // 不区分typedef出来的类型
  {
    return ((this == pOtherTypeDesc) || (name == pOtherTypeDesc->name));
  }

  GXBOOL TYPEDESC::GetMemberTypeList(TYPEDESC::CPtrList& sMemberTypeList) const
  {
    ASSERT(IS_STRUCT_CATE(this)); // 外部保证
    ASSERT(cate != TypeCate_Struct || pMemberNode->mode == SYNTAXNODE::MODE_Block); // 防止以后修改

    sMemberTypeList.clear();

    if(cate == TypeCate_Vector || cate == TypeCate_Matrix)
    {
      int R, C;
      Resolve(R, C);
      C = (cate == TypeCate_Matrix) ? R * C : R;
      for(int i = 0; i < C; i++) {
        sMemberTypeList.push_back(pElementType);
      }
      return TRUE;
    }

    const NameContext* pStructCtx = pNameCtx->GetStructContext(name);
    if(pStructCtx == NULL) {
      return FALSE;
    }

    SYNTAXNODE::GlobList sMemberList;

    CodeParser::BreakChain(sMemberList, pMemberNode->Operand[0]);
    for(auto it = sMemberList.begin(); it != sMemberList.end(); ++it)
    {
      if(it->IsNode() && it->pNode->mode == SYNTAXNODE::MODE_Definition)
      {
        const TOKEN* ptkMember = CodeParser::GetVariableNameWithoutSeamantic(it->pNode->Operand[1]);
        if(ptkMember == NULL) {
          GetLogger()->OutputMissingSemicolon(it->pNode->Operand[1].GetFrontToken());
          return FALSE;
        }

        const TYPEDESC* pMemberTypeDesc = pStructCtx->GetVariable(ptkMember);
        if(pMemberTypeDesc != NULL)
        {
          sMemberTypeList.push_back(pMemberTypeDesc);
        }
        else
        {
          clStringW strMemberName, strStructName = name;
          GetLogger()->OutputErrorW(ptkMember, UVS_EXPORT_TEXT(ERR_IS_NOT_MEMBER, "“%s”: 不是“%s”的成员"),
            ptkMember->ToString(strMemberName).CStr(), strStructName.CStr());
          return FALSE;
        }
      }
      else {
        CLBREAK;
      }
    }
    return TRUE;
  }

  size_t TYPEDESC::CountOf() const
  {
    //size_t count = 1;
    if(cate == TypeCate_Vector || cate == TypeCate_Matrix)
    {
      int R, C;
      Resolve(R, C); // FIXME: 结构体不正确
      return (cate == TypeCate_Vector)
        ? static_cast<size_t>(R)
        : static_cast<size_t>(R * C);
    }
    else if(cate == TypeCate_MultiDim)
    {
      return pElementType->CountOf() * sDimensions.back();
    }
    else if(cate == TypeCate_Struct)
    {
      TYPEDESC::CPtrList sMemberTypeList;
      size_t count = 0;
      if(GetMemberTypeList(sMemberTypeList))
      {
        auto it = sMemberTypeList.begin();
        for(; it != sMemberTypeList.end(); ++it)
        {
          count += (*it)->CountOf();
        }
      }
      else
      {
        count = (size_t)-1;
      }
      return count;
    }
    else
    {
      return 1;
    }
  }

  const TYPEDESC* TYPEDESC::ConfirmArrayCount(size_t nCount) const
  {
    return pNameCtx->ConfirmArrayCount(this, nCount);
  }

  int INTRINSIC_FUNC::GetTypeTemplateTypeIndex(GXDWORD dwMasks)
  {
    return (dwMasks & ArgMask_TemplType) >> ArgMask_TemplShift;
  }

  //////////////////////////////////////////////////////////////////////////

  CInitList::STACKDESC& CInitList::Top()
  {
    return m_sStack.back();
  }

  const CInitList::STACKDESC& CInitList::Top() const
  {
    return m_sStack.back();
  }

  GXBOOL CInitList::Enter(const SYNTAXNODE::GLOB* pInitListGlob)
  {
    if(_CL_NOT_(pInitListGlob->CompareAsNode(SYNTAXNODE::MODE_InitList))) {
      return FALSE;
    }

    const TOKEN* ptkOpcode = pInitListGlob->pNode->pOpcode;
    pInitListGlob = &pInitListGlob->pNode->Operand[0];
    while(true)
    {
      m_sStack.push_back(STACKDESC());
      STACKDESC& top = Top();
      CodeParser::BreakComma(top.sInitList, *pInitListGlob);
      top.iter = top.sInitList.begin();
      top.ptkOpcode = ptkOpcode;
      if(top.sInitList.empty() || _CL_NOT_(top.iter->CompareAsNode(SYNTAXNODE::MODE_InitList))) {
        break;
      }
      else {
        ptkOpcode = top.iter->pNode->pOpcode;
        pInitListGlob = &top.iter->pNode->Operand[0];
      }
    } // while
    return TRUE;
  }

  CInitList::CInitList(CodeParser* pCodeParser, NameContext& rNameCtx, const SYNTAXNODE::GLOB* pInitListGlob)
    : m_pCodeParser(pCodeParser)
    , m_rNameCtx(rNameCtx)
    , m_pInitListGlob(pInitListGlob)
    , m_bNeedAlignDepth(FALSE)
    , m_pValuePool(NULL)
    , m_nValueCount(0)
  {
    ASSERT(pInitListGlob->CompareAsNode(SYNTAXNODE::MODE_InitList));
    Enter(pInitListGlob);
    m_DebugStrings.push_back("");

    COMMALIST cm = { NULL, NULL };
  }

  CLogger* CInitList::GetLogger()
  {
    return m_pCodeParser->GetLogger();
  }

  void CInitList::SetValuePool(VALUE* pValuePool, size_t count)
  {
    m_pValuePool = pValuePool;
    m_nValueCount = count;
  }

  const SYNTAXNODE::GLOB* CInitList::Get()
  {
    if(m_sStack.empty()) {
      return NULL;
    }
    else if(IsEnd() == FALSE)
    {
      Enter(&*Top().iter);
    }
    
    if(Top().sInitList.empty()) {
      return NULL;
    }
    return &*Top().iter;
  }

  CInitList::Result CInitList::CastToValuePool(const TYPEDESC* pRefTypeDesc, size_t base_index, size_t array_index)
  {
    // pRefTypeDesc 参考类型描述，标量/标量数组中这个是标量类型，结构体/向量/矩阵中这个是结构体/向量/矩阵类型
    // base_index  是当前列表的基础索引, 也可以理解为 top index
    // array_index 是当前列表的元素索引
    const size_t index = base_index + array_index;
    const SYNTAXNODE::GLOB* pGlob = Get();
    const TYPEDESC* pTypeDesc = NULL;
    Result func_result = Result_Ok;
    if(pGlob == reinterpret_cast<const SYNTAXNODE::GLOB*>(CInitList::E_FAILED)) {
      CLBREAK;
      return Result_Failed;
    }

    if(pGlob == NULL)
    {
      // 使用了“{}”定义，解释为0
      ASSERT(m_pValuePool[index].rank == VALUE::Rank_Unsigned && m_pValuePool[index].nValue64 == 0);
    }
    else
    {
      ASSERT(pGlob->IsToken() || _CL_NOT_(pGlob->CompareAsNode(SYNTAXNODE::MODE_InitList)));
      VALUE_CONTEXT vctx(m_rNameCtx);
      vctx.pLogger = GetLogger();
      pTypeDesc = m_pCodeParser->InferType(vctx, *pGlob);
      if(pTypeDesc == NULL) {
        return Result_Failed;
      }
      else if(IS_VECMAT_CATE(pTypeDesc) && pRefTypeDesc->IsSameType(pTypeDesc) &&
        pGlob->CompareAsNode(SYNTAXNODE::MODE_FunctionCall) && pGlob->pNode->Operand[1].ptr != NULL) // 要满足 float3(a,b,c) 格式        
      {
        //int i = 0;
        const size_t scaler_count = pTypeDesc->CountOf();

        const PERCOMPONENTMATH* pPreCompMath = FindPerComponentMathOperations(pTypeDesc->name);
        if(pPreCompMath)
        {
          m_sStack.push_back(STACKDESC());
          STACKDESC& top = Top();
          CodeParser::BreakComma(top.sInitList, pGlob->pNode->Operand[1]);
          top.iter = top.sInitList.begin();
          top.ptkOpcode = pGlob->pNode->Operand[0].GetFrontToken();
          // TODO: pGlob->pNode->Operand[1] 里面必须是标量，如果有向量就对不上数量了

          if(index % scaler_count != 0)
          {
            // 需要上面先展开，这样这里返回后才会产生"初始值设定项太多"的错误
            return Result_NotAligned;
          }
        }
        else //(s_PreComponentMath[i].name == NULL)
        {
          GetLogger()->OutputErrorW(*pGlob, UVS_EXPORT_TEXT(5052, "初始值设定项不能用于初始化列表"));
          return Result_Failed;
        }

        pGlob = Get();
        func_result = Result_ExpandVecMat;
      }
      else if(_CL_NOT_(IS_SCALER_CATE(pTypeDesc)))
      {
        GetLogger()->OutputErrorW(*pGlob, UVS_EXPORT_TEXT(5052, "初始值设定项不能用于初始化列表"));
        return Result_Failed;
      }

      ASSERT(m_pValuePool[index].rank == VALUE::Rank_Unsigned && m_pValuePool[index].nValue64 == 0);
      VALUE::State state = m_rNameCtx.CalculateConstantValue(m_pValuePool[index], m_pCodeParser, pGlob);
      if(state == VALUE::State_OK) {
        if(IS_SCALER_CATE(pTypeDesc)) {
          m_pValuePool[index].CastValueByRank(static_cast<VALUE::Rank>(pTypeDesc->pDesc->rank));
        }
      }
      else {
        GetLogger()->OutputErrorW(*pGlob, UVS_EXPORT_TEXT(5053, "无法计算初始化列表常量"));
        return Result_Failed;
      }
    }
    DbgListAdd(pGlob);
    return func_result;
  }

  const TOKEN* CInitList::GetLocation() const
  {
    ASSERT(!m_sStack.empty());
    if(Top().sInitList.empty()) {
      return Top().ptkOpcode;
    }
    return (*Top().iter).GetFrontToken();
  }

  const SYNTAXNODE::GLOB* CInitList::Step()
  {
    // 返回值是步进后的Glob
    STACKDESC& top = Top();
    if(!top.sInitList.empty()) {
      ++top.iter;
    }

    if(IsEnd()) {
      do {
        m_sStack.pop_back();
        if(m_sStack.empty()) {
          break;
        }
        ++Top().iter;
      } while(IsEnd());
      m_bNeedAlignDepth = TRUE;
      return NULL;
    }

    m_bNeedAlignDepth = FALSE;
    return &*top.iter;
  }

  GXBOOL CInitList::Step(size_t nDimDepth, size_t nListDepth)
  {
    if(Step() == NULL) {
      if(Depth() == nDimDepth || Depth() == nListDepth) {
        return TRUE;
      }
      return FALSE; // 结束
    }
    else if(Depth() > nDimDepth || Depth() > nListDepth) {
      return FALSE; // 深度过深并且没有结束：初始值设定项太多
    }
    return TRUE;
  }

  GXBOOL CInitList::IsEnd() const
  {
    const STACKDESC& top = Top();
    return (top.iter == top.sInitList.end());
  }

  GXBOOL CInitList::Empty() const
  {
    return m_sStack.empty();
  }

  size_t CInitList::Depth() const
  {
    //ASSERT(_CL_NOT_(m_sStack.empty()));
    return m_sStack.size();
  }

  GXBOOL CInitList::NeedAlignDepth(size_t nDimDepth, size_t nListDepth) const
  {
    const size_t depth = Depth();
    return m_bNeedAlignDepth && depth < nDimDepth && depth != nListDepth;
  }

  void CInitList::ClearAlignDepthFlag()
  {
    m_bNeedAlignDepth = FALSE;
  }

  size_t CInitList::GetMaxCount(const TYPEDESC* pRefType) const
  {
    size_t count = pRefType->CountOf();
    if(count == 0)
    {
      if(pRefType->cate == TYPEDESC::TypeCate_MultiDim) {
        count = (m_sStack.empty() ? 0 : m_sStack.front().sInitList.size()) * pRefType->pElementType->CountOf();
      }
      else {
        CLBREAK;
      }
    }
    return count;      
  }

  size_t CInitList::BeginList()
  {
    Get(); // 强制进入列表最深层
    return Depth();
  }

  VALUE* CInitList::ValuePoolEnd() const
  {
    return m_pValuePool + m_nValueCount;
  }

  size_t CInitList::ValuePoolCount() const
  {
    return m_nValueCount;
  }

  SYNTAXNODE* CInitList::GetRearrangedList()
  {
    ASSERT(m_RearrangedGlob.size() == 1);
    return m_RearrangedGlob.front().Get();
  }

  void CInitList::DbgListBegin(const clStringA& strTypeName)
  {
    m_DebugStrings.back() = "<";
    m_DebugStrings.back().Append(strTypeName).Append(">{");

    COMMALIST cm;
    cm.Init(m_pCodeParser);
    m_RearrangedGlob.push_back(cm);
  }

  void CInitList::DbgListAdd(const SYNTAXNODE::GLOB* pGlob)
  {
    clStringA strA;
    DbgListAdd(pGlob ? pGlob->ToString(strA) : "0");
    COMMALIST& cm = m_RearrangedGlob.back();
    if(pGlob == NULL)
    {
      VALUE v;
      SYNTAXNODE::GLOB temp_glob = { NULL };
      m_pCodeParser->SetRepalcedValue(temp_glob, v.SetZero());
      cm.PushBack(&temp_glob);
    }
    else
    {
      cm.PushBack(pGlob);
    }
  }

  void CInitList::DbgListAdd(const clStringA& str)
  {
    m_DebugStrings.back().Append(str).Append(',');
  }

  void CInitList::DbgListEnd()
  {
    m_DebugStrings.back().TrimRight(',');
    m_DebugStrings.back().Append("}");

    if(m_RearrangedGlob.size() > 1)
    {
      SYNTAXNODE::GLOB glob;
      glob.pNode = m_RearrangedGlob.back().Get();
      m_RearrangedGlob.pop_back();
      COMMALIST& cm = m_RearrangedGlob.back();
      
      cm.PushBack(&glob);
    }
  }

  void CInitList::DbgPushString()
  {
    ASSERT(!m_DebugStrings.empty());
    m_DebugStrings.push_back("");
  }

  void CInitList::DbgPopString()
  {
    clStringA str = m_DebugStrings.back();
    m_DebugStrings.pop_back();
    ASSERT(!m_DebugStrings.empty());
    DbgListAdd(str);
  }

  void CInitList::DbgSetString(const clStringA& str)
  {
    m_DebugStrings.back() = str;
  }

  clStringA& CInitList::DbgGetString()
  {
    return m_DebugStrings.back();
  }

  //////////////////////////////////////////////////////////////////////////

  void COMMALIST::Init(CodeParser* pParser)
  {
    pCodeParser = pParser;
    pBegin = pEnd = pCodeParser->AllocNode(SYNTAXNODE::MODE_InitList, NULL, NULL);
  }

  SYNTAXNODE* COMMALIST::Get()
  {
    return pBegin;
  }

  COMMALIST& COMMALIST::PushBack(const SYNTAXNODE::GLOB* pGlob)
  {
    if(pEnd->mode == SYNTAXNODE::MODE_InitList)
    {
      if(pEnd->Operand[0].ptr == NULL)
      {
        pEnd->Operand[0].ptr = pGlob->ptr;
      }
      else
      {
        pEnd->Operand[0].pNode = pCodeParser->AllocNode(SYNTAXNODE::MODE_CommaList, pEnd->Operand[0].ptr, pGlob->ptr);
        pEnd = pEnd->Operand[0].pNode;
      }
    }
    else
    {
      ASSERT(pEnd->mode == SYNTAXNODE::MODE_CommaList);
      ASSERT(pEnd->Operand[0].ptr != NULL && pEnd->Operand[1].ptr != NULL);

      pEnd->Operand[1].pNode = pCodeParser->AllocNode(SYNTAXNODE::MODE_CommaList, pEnd->Operand[1].ptr, pGlob->ptr);
      pEnd = pEnd->Operand[1].pNode;
    }
    return *this;
  }

  VALUE_CONTEXT::VALUE_CONTEXT(const NameContext& _name_ctx)
    : name_ctx(_name_ctx)
    , bNeedValue(TRUE)
    , pLogger(NULL)
    , result(ValueResult_Undefined)
    , pType(NULL)
    , pValue(NULL)
    , count(0)
  {
  }

  VALUE_CONTEXT::VALUE_CONTEXT(const NameContext& _name_ctx, GXBOOL _bNeedValue)
    : name_ctx(_name_ctx)
    , bNeedValue(_bNeedValue)
    , pLogger(NULL)
    , result(ValueResult_Undefined)
    , pType(NULL)
    , pValue(NULL)
    , count(0)
  {
  }

  VALUE_CONTEXT::VALUE_CONTEXT(const VALUE_CONTEXT& vctx)
    : name_ctx(vctx.name_ctx)
    , bNeedValue(vctx.bNeedValue)
    , pLogger(vctx.pLogger)
    , result(ValueResult_Undefined)
    , pType(NULL)
    , pValue(NULL)
    , count(0)
  {
  }

  void VALUE_CONTEXT::SetProperty(const VALUE_CONTEXT& vctx) // 从vctx复制属性
  {
    bNeedValue = vctx.bNeedValue;
    pLogger = vctx.pLogger;
  }

  void VALUE_CONTEXT::UsePool()
  {
    ASSERT(pool.empty() == FALSE);
    pValue = &pool.front();
    count = pool.size();
  }

  void VALUE_CONTEXT::ClearValue()
  {
    result = ValueResult_Undefined;
    pType  = NULL;
    pValue = NULL;
    count  = 0;
    pool.clear();
  }

  void VALUE_CONTEXT::ClearValueOnly()
  {
    pValue = NULL;
    count = 0;
    pool.clear();
  }

  void VALUE_CONTEXT::SetType(VALUE::Rank rank)
  {
    pType = name_ctx.GetType(rank);
    ASSERT(pType);
  }

  // 根据参数类型扩充元素个数，如果rank比较低也会提升
  const TYPEDESC* VALUE_CONTEXT::CastUpward(const TYPEDESC* pTargetType)
  {
    const size_t type_count = pType->CountOf();
    const size_t targ_count = pTargetType->CountOf();
    ASSERT(type_count < targ_count ||
      (type_count < targ_count && TypeRank() < pTargetType->pDesc->rank));

    if(IS_SCALER_CATE(pType) && IS_VECMAT_CATE(pTargetType))
    {
      VALUE value = *pValue;
      if(value.rank > pTargetType->pElementType->pDesc->rank)
      {
        pType = MergeType(pType, pTargetType);
      }
      else
      {
        value.CastValueByRank(static_cast<VALUE::Rank>(pTargetType->pDesc->rank));
        pType = pTargetType;
      }
      pool.assign(targ_count, value);
      UsePool();
    }
    else if(IS_VECTOR_CATE(pType) && IS_VECTOR_CATE(pTargetType) && type_count == targ_count)
    {
      VALUE value;
      if(pool.empty()) // 从常数表中摘出到临时池中
      {        
        pool.assign(pValue, pValue + count);
      }
      else if(pool.size() != count) // 缩紧临时池
      {
        ASSERT(pool.size() > count);
        ValuePool temp_pool;
        temp_pool.assign(pValue, pValue + count);
        pool.assign(temp_pool.begin(), temp_pool.end());
      }

      // rank 映射
      for(auto it = pool.begin(); it != pool.end(); ++it)
      {
        (*it).CastValueByRank(static_cast<VALUE::Rank>(pTargetType->pDesc->rank));
      }
      UsePool();
    }
    else
    {
      CLBREAK;
    }
    return pType;
  }

  // 根据给定类型获得更高阶向量矩阵类型
  const TYPEDESC* VALUE_CONTEXT::MergeType(const TYPEDESC* pScalerType, const TYPEDESC* pVecMatType)
  {
    ASSERT(IS_SCALER_CATE(pScalerType));
    ASSERT(IS_VECMAT_CATE(pVecMatType));
    ASSERT(VALUE::IsNumericRank(static_cast<VALUE::Rank>(pScalerType->pDesc->rank)));
    ASSERT(VALUE::IsNumericRank(static_cast<VALUE::Rank>(pVecMatType->pDesc->rank)));
    if(pScalerType->pDesc->rank > pVecMatType->pDesc->rank)
    {
      clStringA str = pScalerType->pDesc->name;
      str.Append(pVecMatType->pDesc->name + clstd::strlenT(pVecMatType->pDesc->component_type));
      pVecMatType = name_ctx.GetType(str);
      ASSERT(pVecMatType);
    }    
    return pVecMatType;
  }

  GXBOOL VALUE_CONTEXT::IsNeedValue() const
  {
    ASSERT((pValue == NULL && count == 0) || (pValue != NULL && count > 0));
    return bNeedValue && result == ValueResult_OK && pValue != NULL;
  }

  VALUE::Rank VALUE_CONTEXT::TypeRank() const
  {
    return (static_cast<VALUE::Rank>(pType->pDesc->rank));
  }

  //////////////////////////////////////////////////////////////////////////

  VALUE_CONTEXT_CHECKER::VALUE_CONTEXT_CHECKER(const VALUE_CONTEXT& _vctx)
    : vctx(_vctx)
    , nErrorCount(_vctx.pLogger->ErrorCount())
  {
    ASSERT(vctx.pLogger); // 必须指定Logger
  }

  VALUE_CONTEXT_CHECKER::~VALUE_CONTEXT_CHECKER()
  {
    // 必须返回写入处理结果
    ASSERT(vctx.result != ValueResult_Undefined);

    // 没有算出返回值时一定要输出错误信息
    if(vctx.pType == NULL || (vctx.result != ValueResult_OK && vctx.result != ValueResult_Variable))
    {
      ASSERT(vctx.pLogger->ErrorCount() > nErrorCount);
    }

    // 检查value有效性
    if(vctx.pValue && vctx.pType)
    {
      // 指针有效时肯定有值
      ASSERT(vctx.pValue != NULL && vctx.count > 0);

      // 类型与数量一致
      ASSERT(vctx.count == vctx.pType->CountOf());

      if(vctx.pool.empty() == FALSE &&
        (IS_SCALER_CATE(vctx.pType) || IS_VECMAT_CATE(vctx.pType)))
      {
        const VALUE::Rank rank = vctx.TypeRank();
        for(size_t i = 0; i < vctx.count; i++)
        {
          ASSERT(rank == vctx.pValue[i].rank);
        }
      }
    }

    if(vctx.pValue && vctx.pool.empty() == FALSE) {
      ASSERT(vctx.pValue >= &vctx.pool.front() && vctx.pValue <= &vctx.pool.back());
    }
  }

} // namespace UVShader
