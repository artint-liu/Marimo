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
  (_CATE)->cate == TYPEDESC::TypeCate_FloatScaler || \
  (_CATE)->cate == TYPEDESC::TypeCate_IntegerScaler)

#define IS_VECMAT_CATE(_CATE) (\
  (_CATE)->cate == TYPEDESC::TypeCate_Vector || \
  (_CATE)->cate == TYPEDESC::TypeCate_Matrix)

#define IS_STRUCT_CATE(_CATE) (\
  (_CATE)->cate == TYPEDESC::TypeCate_Vector || \
  (_CATE)->cate == TYPEDESC::TypeCate_Matrix || \
  (_CATE)->cate == TYPEDESC::TypeCate_Struct)

#define IS_SAMPLER_CATE(_CATE) (\
  (_CATE)->cate == TYPEDESC::TypeCate_Sampler1D || \
  (_CATE)->cate == TYPEDESC::TypeCate_Sampler2D || \
  (_CATE)->cate == TYPEDESC::TypeCate_Sampler3D || \
  (_CATE)->cate == TYPEDESC::TypeCate_SamplerCube ) 

#define REDUCE_ERROR_MESSAGE

//#define VOID_TYPEDESC ((const TYPEDESC*)-1)
#define ERROR_TYPEDESC ((const TYPEDESC*)-1)
#define PARSER_NOTIMPLEMENT TRACE("%s(%d):没咋处理的地方\n", __FILE__, __LINE__)

#define DUMP_STATE_IF_FAILED \
if(state != VALUE::State_OK && DumpValueState(vctx.pLogger, state, pOperator)) { \
  vctx.result = ValueResult_Failed; return FALSE; }


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
  static GXLPCSTR s_szStructMember = "struct member";  // Name Context 结构体成员

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
  const size_t STR_BOOL_LENGTH = clstd::strlenT(STR_BOOL);
  const size_t STR_HALF_LENGTH = clstd::strlenT(STR_HALF);
  const size_t STR_FLOAT_LENGTH  = clstd::strlenT(STR_FLOAT);
  const size_t STR_DOUBLE_LENGTH = clstd::strlenT(STR_DOUBLE);

  const GXLPCSTR STR_RETURN = "return";

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
  extern COMMINTRTYPEDESC s_aIntrinsicType[];
  extern size_t s_aIntrinsicType_len;
  extern COMMINTRTYPEDESC s_aBaseType[];
  extern INTRINSIC_FUNC s_wildcard_functions[];
  extern size_t s_wildcard_functions_len;
  extern INTRINSIC_FUNC s_wildcard_unary_functions[];
  extern size_t s_wildcard_unary_functions_len;

  extern PERCOMPONENTMATH s_PreComponentMath[];

  extern BUILDIN_FUNCTION_PROTOTYPE s_functions_prototype[];
  extern size_t s_functions_prototype_len;


  const PERCOMPONENTMATH* FindPerComponentMathOperations(const RefString& rstrName)
  {
    for(int i = 0; s_PreComponentMath[i].name != NULL; i++) {
      if(rstrName == s_PreComponentMath[i].name) {
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
    , m_dwParserState(State_MoreValidation)
    , m_pParent(NULL)
    , m_nPPRecursion(0)
    , m_pInclude(&s_DefaultInclude)
    , m_GlobalCtx("$")
  {
    m_GlobalCtx.allow_keywords = KeywordFilter_typedef;
    m_GlobalCtx.SetParser(this);
    m_GlobalCtx.BuildIntrinsicType();
    InitPacks();
  }

  CodeParser::CodeParser(CodeParser* pParent, PARSER_CONTEXT* pContext, Include* pInclude)
    : m_pContext(pContext)
    , m_pSubParser(NULL)
    , m_dwState(0)
    , m_dwParserState(State_MoreValidation)
    , m_pParent(pParent)
    , m_nPPRecursion(0)
    , m_pInclude(pInclude ? pInclude : &s_DefaultInclude)
    , m_GlobalCtx("$")
  {
    //m_GlobalSet.allow_keywords = KeywordFilter_typedef; // TODO: 去掉？
    //m_GlobalSet.SetParser(this);
    //m_GlobalSet.BuildIntrinsicType(); // TODO: 去掉？
    if(pContext) {
      pContext->nRefCount++;
    }

    if(pContext && ! pContext->Macros.empty())
    {
      //int n = 0;
      for(auto it = pContext->Macros.begin(); it != pContext->Macros.end(); ++it/*, n++*/)
      {
        //ASSERT(it->second.nOrder == n); // 必须指定宏的order
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
    STATEMENT stat = {StatementType_Empty};
    m_aSubStatements.push_back(stat);
  }

  void CodeParser::Reset()
  {
    m_ValueDict.clear();
    m_PhonyTokenDict.clear();
    m_GlobalCtx.Reset();
    m_aTokens.clear();
    m_aStatements.clear();
    if(m_pLogger && m_pParent == NULL) {
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
      m_pContext->mid = 1;
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
      if(TEST_FLAG(((CodeParser*)it.pContainer)->m_dwState, AttachFlag_RetainPreprocess))
      {
        it.length = 0;
        return 0;
      }
      CodeParser* pThis = (CodeParser*)it.pContainer;

      // 测试是否已经在预处理中
      if(TEST_FLAG(pThis->m_dwState, AttachFlag_Preprocess)) {
        return 0;
      }
      SET_FLAG(pThis->m_dwState, AttachFlag_Preprocess);

      if( ! TOKENSUTILITY::IsHeadOfLine(it.pContainer, it.marker)) {
        //ERROR_MSG_C2014_预处理器命令必须作为第一个非空白空间启动;
        GetLogger()->OutputErrorW(it.marker, UVS_EXPORT_TEXT(2014, "预处理器命令必须作为第一个非空白空间启动"));
        return 0;
      }

      RTPPCONTEXT ctx;
      ctx.iter_next = it;
      ctx.iter_next.length = TOKENSUTILITY::ExtendToNewLine(ctx.iter_next.marker, remain);
      
      // ppend 与 iter_next.marker 之间可能存在空白，大量注释等信息，为了
      // 减少预处理解析的工作量，这里预先存好 ppend 再步进 iter_next
      ctx.ppend = ctx.iter_next.end();
      ctx.stream_end = pThis->GetStreamPtr() + pThis->GetStreamCount();
      ++ctx.iter_next;

      if(++it >= ctx.iter_next) { // 只有一个'#', 直接跳过
        RESET_FLAG(pThis->m_dwState, AttachFlag_Preprocess);
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
            RESET_FLAG(pThis->m_dwState, AttachFlag_Preprocess);
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
      if(it == '#') { // keep pound as PreProcessing command
        it.length = 0;
      }

      RESET_FLAG(pThis->m_dwState, AttachFlag_Preprocess);
    }
    else if(it.marker[0] == '\\')
    {
      if((remain > 0 && it.marker[1] == '\n') ||
        (remain > 1 && it.marker[1] == '\r' && it.marker[2] == '\n') ||
        (remain == 1)) {
        ++it;
      }
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
      else if(*it == '\\') {
        GetLogger()->OutputErrorW(*it, UVS_EXPORT_TEXT(2017, "非法的转义序列"));
      }

      const size_t c_size = (int)m_aTokens.size();

      if(TEST_FLAG_NOT(m_dwState, AttachFlag_Preprocess) && (it.marker[0] == '-' || it.marker[0] == '+'))
      {
        OnMinusPlus(UnaryPendingList, *it, c_size);
      }
      
      // 只是清理
      //l_token.ClearMarker();
      //l_token.ClearArithOperatorInfo();


      // 符号配对处理
      // ...={...}这种情况不更新EOE
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

  GXBOOL CodeParser::DumpValueState(CLogger* pLogger, VALUE::State state, const TOKEN* pToken)
  {
    // 警告返回FALSE，错误返回TRUE
    clStringW strW;
    switch(state)
    {
    case VALUE::State_UnknownOpcode:
      pLogger->OutputErrorW(*pToken, UVS_EXPORT_TEXT2(5033, "无效的操作符：“%s”", pLogger), pToken->ToString(strW).CStr());
      break;
    case VALUE::State_SyntaxError:
      pLogger->OutputErrorW(*pToken, UVS_EXPORT_TEXT2(5034, "语法错误：“%s”", pLogger), pToken->ToString(strW).CStr());
      break;
    case VALUE::State_Overflow:
      pLogger->OutputErrorW(*pToken, UVS_EXPORT_TEXT2(5035, "溢出：“%s”", pLogger), pToken->ToString(strW).CStr());
      break;
    case VALUE::State_IllegalChar:
      pLogger->OutputErrorW(*pToken, UVS_EXPORT_TEXT2(5036, "非法字符：“%s”", pLogger), pToken->ToString(strW).CStr());
      break;
    case VALUE::State_BadOpcode:
      pLogger->OutputErrorW(*pToken, UVS_EXPORT_TEXT2(5037, "错误的操作符：“%s”", pLogger), pToken->ToString(strW).CStr());
      break;
    case VALUE::State_IllegalNumber:
      pLogger->OutputErrorW(*pToken, UVS_EXPORT_TEXT2(5038, "非法的数字：“%s”", pLogger), pToken->ToString(strW).CStr());
      break;
    case VALUE::State_DivideByZeroF: // 浮点数
      // error C2124 : 被零除或对零求模
      pLogger->OutputErrorW(*pToken, UVS_EXPORT_TEXT2(_WARNING(2124), "被零除或对零求模", pLogger));
      return FALSE;
    case VALUE::State_DivideByZeroI: // 整数
      // error C2124 : 被零除或对零求模
      pLogger->OutputErrorW(*pToken, UVS_EXPORT_TEXT2(2124, "被零除或对零求模", pLogger));
      return FALSE;
    case VALUE::State_BadIdentifier: // 内部输出
      break;
    case VALUE::State_Call: // 向量/矩阵等常量的初始化
    case VALUE::State_Variable: // 右值中含有变量，不是一个在编译时就能计算出数值的表达式
      break;
    default:
      CLBREAK;
      //PARSER_BREAK(const_expr_glob.pNode->Operand[1]);
      break;
    }
    return TRUE; // “错误”返回
  }

  void CodeParser::DumpStateError(CLogger* pLogger, NameContext::State state, const TOKEN& tkType, const TOKEN& tkVar)
  {
    clStringW strW;
    switch(state)
    {
    case NameContext::State_Ok:
      CLBREAK;  // 外面保证不会进入这个分支
      break;

    case NameContext::State_TypeNotFound:
    {
      pLogger->OutputErrorW(tkType, UVS_EXPORT_TEXT2(5012, "“%s”: 类型未定义", pLogger), tkType.ToString(strW).CStr());
      break;
    }
    case NameContext::State_DuplicatedIdentifier:
    {
      pLogger->OutputErrorW(tkVar, UVS_EXPORT_TEXT2(2371, "“%s”: 重定义", pLogger), tkVar.ToString(strW).CStr());
      break;
    }
    //case NameContext::State_RequireInitList:
    //{
    //  pLogger->OutputErrorW(tkVar, UVS_EXPORT_TEXT2(5015, "“%s”: 数组定义需要初始化列表来设置", pLogger), tkVar.ToString(strW).CStr());
    //  break;
    //}
    case NameContext::State_DefineAsType:
    {
      pLogger->OutputErrorW(tkVar, UVS_EXPORT_TEXT2(5013, "“%s”: 变量已经被定义为类型", pLogger), tkVar.ToString(strW).CStr());
      break;
    }
    case NameContext::State_VariableIsNotIdentifier:
    {
      pLogger->OutputErrorW(tkVar, UVS_EXPORT_TEXT2(3000, "预期是一个变量名 : \"%s\"", pLogger), tkVar.ToString(strW).CStr());
      break;
    }
    case NameContext::State_HasError:
      break;
    case NameContext::State_CastTypeError:
    case NameContext::State_RequireConstantExpression:
      break;  // 内部已处理
    default:
      CLBREAK; // 预期之外的状态
      break;
    }
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

  PHONYTOKEN* CodeParser::AddPhonyToken(TOKEN::Type type, const clStringA& str, const TOKEN* pToken)
  {
    ASSERT(pToken >= &m_aTokens.front() && pToken <= &m_aTokens.back());
    m_aPhonyTokens.push_back(PHONYTOKEN());
    PHONYTOKEN* pPhonyToken = &m_aPhonyTokens.back();
    //*static_cast<TOKEN*>(pPhonyToken) = *pToken;
    pPhonyToken->marker     = GetUniqueString(str);
    pPhonyToken->length     = str.GetLength();
    pPhonyToken->scope      = -1;
    pPhonyToken->semi_scope = pToken->semi_scope;
    pPhonyToken->precedence = 0;
    pPhonyToken->unary      = 0;
    pPhonyToken->unary_mask = 0;
    
    pPhonyToken->type       = type;
    //pPhonyToken->bPhony     = 1;
    pPhonyToken->ptkOriginal= pToken;
    return pPhonyToken;
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

  auto CodeParser::AddMacro(const RefString& rstrMacroName, const MACRO& macro)
  {
    return m_pContext->Macros.insert(clmake_pair(rstrMacroName, macro));
  }

  void CodeParser::NormalizeMacro(const RefString& rstrMacroName)
  {
    auto& it_macro = m_pContext->Macros.find(rstrMacroName);
    if(it_macro == m_pContext->Macros.end()) {
      return;
    }

    MACRO& origin_macro = it_macro->second;

    // 对于“#define A B”并且B也是宏定义的形式进行查找，将A与最终的定义对应起来
    if(it_macro->second.aFormalParams.size() == 0 && it_macro->second.aTokens.size() == 1)
    {
      MACRO_EXPAND_CONTEXT::IDSet_T sIDSet; // 防止无限查找
      sIDSet.insert(it_macro->second.nid);
      while(true) {
        RefString rstrTokenName(it_macro->second.aTokens.front().marker, it_macro->second.aTokens.front().length);
        auto& it_replace = m_pContext->Macros.find(rstrTokenName);

        if(it_replace == m_pContext->Macros.end() ||
          sIDSet.find(it_replace->second.nid) != sIDSet.end()) {
          break;
        }
        it_macro = it_replace;
        if(_CL_NOT_(it_replace->second.aFormalParams.size() == 0 && it_replace->second.aTokens.size() == 1))
        {
          break;
        }
        sIDSet.insert(it_macro->second.nid);
      }
    }

    origin_macro.aFormalParams = it_macro->second.aFormalParams;
    origin_macro.aTokens       = it_macro->second.aTokens;
  }

  const CodeParser::MACRO* CodeParser::FindMacro(const TOKEN& token) // TODO: 正常的查找都要换做这个
  {
    RefString rstrTokenName(token.marker, token.length);

    auto& it_macro = m_pContext->Macros.find(rstrTokenName);
    if(it_macro == m_pContext->Macros.end()) {
      return NULL;
    }
    
    return &it_macro->second;
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
    int depth = 0;  // "()"
    int depth1 = 0; // "{}"
    TOKEN::T_LPCSTR begin_ptr = it.marker;
    
    for(auto iter_stream = stream.begin(); iter_stream != stream.end(); ++iter_stream)
    {
      if(*iter_stream == '(') {
        depth++;
      }
      else if(*iter_stream == ')') {
        depth--;
      }
      else if(*iter_stream == '{') {
        depth1++;
      }
      else if(*iter_stream == '}') {
        depth1--;
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
        if(depth <= 0 && depth1 == 0) {
          // 防止处理预处理命令
          const GXDWORD dwOldState = m_dwState;
          m_dwState |= AttachFlag_RetainPreprocess;
          ++it;
          m_dwState = dwOldState;
          break;
        }
      }
      else if(*it == '{') {
        depth1++;
      }
      else if(*it == '}') {
        depth1--;
      }
      else if(depth == 0 && depth1 == 0 && it.marker > begin_ptr) {
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
    //if( ! pMacro || (pMacro->aFormalParams.empty() && pMacro->aTokens.empty())) {
    if(pMacro == NULL) {
      return FALSE;
    }
    else if(pMacro->aFormalParams.empty() && pMacro->aTokens.empty())
    {
      if(TEST_FLAG(m_dwState, AttachFlag_Preprocess)) {
        return FALSE; // 不替换宏预处理表达式解析中的宏
      }
      // 宏替换成空白
      m_ExpandedStream.clear();
      return TRUE;
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

      const GXDWORD dwOldState = m_dwState;
      m_dwState |= AttachFlag_RetainPreprocess;
      ++it;
      m_dwState = dwOldState;
    }
    else
    {
      int depth = 0;
      TOKEN save_token = *it;

      it = MakeupMacroFunc(stream, it, end());

      if(depth < 0)
      {
        CLBREAK; // 不应该到这里
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
      if(result != MacroExpand_Ok) { // 对于不完整的宏调用，只能重新展开一次
        GetLogger()->OutputMissingSemicolon(&m_aTokens.back());
        return FALSE;
      }
    }
    m_ExpandedStream = stream;
    m_ExpandedStream.push_back(next_token);
    if(m_ExpandedStream.front().pContainer == NULL) {
      m_ExpandedStream.front().pContainer = reinterpret_cast<CodeParser*>(macro_offset | 1);
    }
    ASSERT(((size_t)m_ExpandedStream.back().pContainer & 1) == 0);
    return TRUE;
  }

  void CodeParser::OnMinusPlus(cllist<clsize>& UnaryPendingList, TOKEN& it, size_t c_size)
  {
    ASSERT(it.marker[0] == '-' || it.marker[0] == '+');

    if(c_size == 0) // 单独表达式解析时才可能出现符号在开头的情况
    {
      // "++" "--" 默认是右操作数
      if(it.length == 1)
      {
        it.SetArithOperatorInfo(s_plus_minus_sign[(int)(it.marker[0] - '+')]);
      }
      return;
    }

    ASSERT(it == '-' || it == '+' || it == "--" || it == "++" || it == "+=" || it == "-=");
    const auto& l_back = m_aTokens.back();

    // 如果是 -,+ 检查前一个符号是不是操作符或者括号，如果是就认为这个 -,+ 是正负号
    if(it.length == 1)
    {
      const ArithmeticExpression::MBO& p = s_plus_minus_sign[(int)(it.marker[0] - '+')];
      // 一元操作符，+/-就不转换为正负号
      // '}' 就不判断了 { something } - abc 这种格式应该是语法错误
      if(l_back == STR_RETURN || l_back == ';' ||
        //(l_back.precedence != 0 && l_back != ')' && l_back != ']' && (!l_back.unary)))
        (l_back.precedence != 0 && l_back != ')' && l_back != ']' && (l_back.unary_mask != UNARY_LEFT_OPERAND)))
      {
        it.SetArithOperatorInfo(p);
      }
    }
    else if(it.length == 2 && (it.marker[1] == '-' || it.marker[1] == '+'))
    {
      // "++","--" 默认按照前缀操作符处理, 这里检查是否转换为后缀操作符
      if((l_back.IsIdentifier() && l_back != STR_RETURN) || l_back == ']') {
        const auto& p = s_UnaryLeftOperand[(int)(it.marker[0] - '+')];
        it.SetArithOperatorInfo(p);
      }
      else {
        UnaryPendingList.push_back(c_size);
      }
    }
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
      ctx_out.OrderSet.find(ctx_out.pMacro->nid) != ctx_out.OrderSet.end())
    {
      return MacroExpand_Skip;
    }

    TOKEN::List::iterator it = it_begin;
    ctx_out.OrderSet.insert(ctx_out.pMacro->nid);

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

      int depth = 1;  // "()"
      int depth1 = 0; // "{}"

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
        else if(*it == '{') {
          depth1++;
        }
        else if(*it == '}') {
          depth1--;
        }
        else if(*it == ',' && depth == 1 && depth1 == 0)
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

  CodeParser::MacroExpand CodeParser::ExpandMacroContent(TOKEN::List& sTokenList, const TOKEN& line_num, MACRO_EXPAND_CONTEXT::IDSet_T* pOrderSet) // 展开宏内容里的宏
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
    if(GetLogger()->ErrorCount(FALSE) > 0) {
      return FALSE;
    }
#if 1
    TKSCOPE scope(0, m_aTokens.size());
    while(ParseStatement(&scope));
    RelocalePointer();
    return m_pLogger->ErrorCount(FALSE) == 0;
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
    else if(pScope->begin + 2 < pScope->end && m_aTokens[pScope->begin + 2] == '(') {
      // 函数特征：<type> <func>() 
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
    if(_CL_NOT_(ParseExpression(stat.sRoot, &m_GlobalCtx, scope)))
    {
      ASSERT(DbgErrorCount() > nErrorCount); // 确保内部输出了错误消息
      return FALSE;
    }

    // 函数声明
    if(stat.sRoot.IsToken()) {
      clStringW strW;
      GetLogger()->OutputErrorW(stat.sRoot.pTokn, UVS_EXPORT_TEXT(_WARNING(5059), "“%s” : 孤立的定义."), stat.sRoot.pTokn->ToString(strW).CStr());
      pScope->begin = definition_end; // 步进到下一个statement
      return TRUE;
    }
    else if(stat.sRoot.pNode->Operand[1].IsNode() && stat.sRoot.pNode->Operand[1].pNode->mode == SYNTAXNODE::MODE_FunctionCall)
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
        Verify_IdentifierDefinition(m_GlobalCtx, pChainNode->Operand[0].pNode,
          (stat.defn.modifier == UniformModifier_const) ? VerifyIdentifierDefinition_Const : 0);
        pChainNode = pChainNode->Operand[1].pNode;
      }
#endif
      m_aStatements.push_back(stat);
    }
    else
    {
#ifdef ENABLE_SYNTAX_VERIFY
      Verify_IdentifierDefinition(m_GlobalCtx, stat.sRoot.pNode,
        (stat.defn.modifier == UniformModifier_const) ? VerifyIdentifierDefinition_Const : 0);
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
    TYPEINSTANCE::Array types_array;

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
    NameContext sNameSet_Func(strFunctionName, &m_GlobalCtx);

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
      ParseFunctionArguments(sNameSet_Func, &stat, &ArgScope, types_array, nTypeOnlyCount);
    }
    p = &m_aTokens[p->scope];
    ASSERT(*p == ')');

    ++p;

    // #
    // # [: Semantic]
    // #
    if(*p == ":") {
      stat.func.szSemantic = m_pContext->Strings.add((++p)->ToString());
      p++;
    }

    if(*p == ";") { // 函数声明
      stat.type = StatementType_FunctionDecl;
    }
    else if(*p == "{") { // 函数体
      if(nTypeOnlyCount != 0) {
        // 声明了一个类型，没有变量名，没使用不算错
        GetLogger()->OutputErrorW(*p, UVS_EXPORT_TEXT(_WARNING(2055), "应输入形参表，而不是类型表")); // warning
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
            return FALSE;
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
    m_GlobalCtx.RegisterFunction(stat.func.szReturnType, stat.func.szName, types_array);
      //&m_aArgumentsPack[(size_t)stat.func.pArguments], stat.func.nNumOfArguments);

    pScope->begin = p - &m_aTokens.front();
    return TRUE;
  }

  GXBOOL CodeParser::ParseStatement_SyntaxError(TKSCOPE* pScope)
  {
    const TOKEN& token = m_aTokens[pScope->begin];
    clStringW strW;
    GetLogger()->OutputErrorW(token, UVS_EXPORT_TEXT(5032, "语法错误：不能识别的格式“%s”"), token.ToString(strW).CStr());
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
    if(sGlob.IsNode() && sGlob.pNode->CompareOpcode(',') && sGlob.pNode->mode != SYNTAXNODE::MODE_BracketList) {
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

  SYNTAXNODE::GlobList& CodeParser::BreakChain2(SYNTAXNODE::GlobList& sExprList, const GLOB& sGlob)
  {
    if(sGlob.IsNode() && sGlob.pNode->mode == SYNTAXNODE::MODE_Chain) {
      sExprList.push_back(sGlob.pNode->Operand[0]);

      if(sGlob.pNode->Operand[1].ptr) {
        return BreakChain(sExprList, sGlob.pNode->Operand[1]);
      }
    }
    else {
      sExprList.push_back(sGlob);
    }
    return sExprList;
  }

  CodeParser::TKSCOPE::TYPE CodeParser::ParseStructDefinition(NameContext& sNameSet, const TKSCOPE& scope, GLOB* pMembers, GLOB* pDefinitions, const TOKEN** ppName, int* pSignatures, int* pDefinitionNum)
  {
    // pMembers->ptr为空表示解析失败，空定义pMembers->ptr也是一个block

    const TOKEN* pTokensFront = &m_aTokens.front();
    const TOKEN* p = &m_aTokens[scope.begin];
    const TOKEN* pEnd = pTokensFront + scope.end;

    ASSERT(*p == "struct");

    // 名字
    const TOKEN* ptkName = ++p;
    if(*ptkName == '{') // 匿名结构体
    {
      clStringA str;
      str.Format("_st_noname_%u", p - &m_aTokens.front());

      PHONYTOKEN* pPhonyToken = AddPhonyToken(TOKEN::TokenType_Identifier, str, ptkName);
      ptkName = pPhonyToken;
    }
    else if(ptkName >= pEnd || _CL_NOT_(ptkName->IsIdentifier()))
    {
      clStringW str;
      GetLogger()->OutputErrorW(*ptkName, UVS_EXPORT_TEXT(2332, "“%s”: 缺少标记名"), (p - 1)->ToString(str).CStr());
      ASSERT(str == _CLTEXT("struct"));
      //return p - pTokensFront;
      return scope.end;
    }
    else {
      ++p;
    }

    *ppName = ptkName; // 返回结构体名，用来传递匿名结构体

    if(p >= pEnd) {
      //GetLogger()->OutputErrorW(*p, UVS_EXPORT_TEXT(2143, "语法错误 : 缺少“;”"));
      GetLogger()->OutputMissingSemicolon(p);
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
          Verify_StructMember(sNameSet, RefString(ptkName->marker, ptkName->length), *pMembers->pNode); // 检查失败仍然继续解析
#endif
        }
      }
      else // 空结构体
      {
        *pSignatures = 0;
        *pDefinitionNum = 0;
      }

      if(sNameSet.RegisterStruct(ptkName, pMembers->pNode) == FALSE &&
        sNameSet.GetIdentifier(ptkName))
      {
        clStringW str;
        GetLogger()->OutputErrorW(*ptkName, UVS_EXPORT_TEXT(2371, "“%s”: 重定义；不同的基类型"), ptkName->ToString(str).CStr());
        return sMembersScope.end;
      }


      p = pTokensFront + sMembersScope.end;
    } // else if(*p == '{')
    else
    {
      GetLogger()->OutputMissingSemicolon(p);
      return scope.end;
    }

    //////////////////////////////////////////////////////////////////////////

    if(p > &m_aTokens.back()) {
      GetLogger()->OutputErrorW(*pTokensFront, UVS_EXPORT_TEXT(1004, "遇到意外的文件结束"));
      return scope.end;
    }
    else if(*p == ';') {
      pMembers->pNode->Operand[1].pTokn = p; // Block后面的分号
      return p - pTokensFront + 1;
    }
    else if(p->IsIdentifier() && p->semi_scope >= 0)
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

      if(glob.pNode->Operand[1].IsNode() && // <type> <identifier A>,<identifier B>... 形式
        glob.pNode->Operand[1].pNode->CompareOpcode(','))
      {
        MakeSyntaxNode(&glob, SYNTAXNODE::MODE_Chain, &glob, NULL);
        FlatDefinition(glob.pNode);

#ifdef ENABLE_SYNTAX_VERIFY
        SYNTAXNODE* pChainNode = glob.pNode;
        while(pChainNode) {
          Verify_IdentifierDefinition(sNameSet, pChainNode->Operand[0].pNode);
          pChainNode = pChainNode->Operand[1].pNode;
        }
#endif
      }
      else  // <type> <identifier/identifier[n]>; 形式
      {
        if(_CL_NOT_(Verify_IdentifierDefinition(sNameSet, glob.pNode))) {
          return scope.end;
        }
      }

      pDefinitions->ptr = glob.ptr;
      return p - pTokensFront;
    }
    else// if(*p != ';') 
    {
      clStringW str;
      //GetLogger()->OutputErrorW(*p, UVS_EXPORT_TEXT(2143, "语法错误 : 缺少“;”(在“%s”的前面)"), p->ToString(str).CStr());
      GetLogger()->OutputMissingSemicolon(p);
      return scope.end;
    }

    //////////////////////////////////////////////////////////////////////////
    CLBREAK; // 其实不会到这里
    return p - pTokensFront;
  }

  GXBOOL CodeParser::ParseStatementAs_Struct(TKSCOPE* pScope)
  {
    const TOKEN* p = &m_aTokens[pScope->begin];
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
    
    pScope->begin = ParseStructDefinition(m_GlobalCtx, *pScope, &stat_stru.sRoot,
      &stat_vari.sRoot, &p, &nSignatures, &nDefinition);

    if(stat_stru.sRoot.ptr)
    {
      stat_stru.type = nSignatures ? StatementType_Signatures : StatementType_Struct;
      stat_stru.stru.szName = GetUniqueString(p);
      stat_stru.stru.nNumOfMembers = nDefinition;
      m_aStatements.push_back(stat_stru);

      if(stat_vari.sRoot.ptr) {
        m_aStatements.push_back(stat_vari);
      }
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
    if(TryKeywords(m_GlobalCtx, *pScope, &stat.sRoot, &end)) {
      m_aStatements.push_back(stat);
    }
    pScope->begin = end;
    return TRUE;
  }

  GXBOOL CodeParser::VerifyFunctionArgument(NameContext& sNameCtx, const GLOB* pGlob, const GLOB& rBaseGlob, TYPEINSTANCE::Array& types_array, int& nTypeOnlyCount)
  {
    const TOKEN* ptkType = NULL;
    const TOKEN* ptkArgName = NULL;
    clStringW strW;
    TYPEINSTANCE type_inst;

    if(pGlob->ptr == NULL)
    {
      // ERROR: func(float a,,float b); 形式
      GetLogger()->OutputErrorW(rBaseGlob, UVS_EXPORT_TEXT(5060, "语法错误 : 参数列表出现空项"));
      return FALSE;
    }
    else if(pGlob->IsToken()) {
      ptkType = pGlob->pTokn;
    }
    else // node
    {
      // Operand - A
      if(pGlob->pNode->Operand[0].IsNode())
      {
        GetLogger()->OutputErrorW(pGlob->pNode->Operand[0], UVS_EXPORT_TEXT(5062, "“%s”不正确的参数语法"), pGlob->pNode->GetAnyTokenAPB().ToString(strW).CStr());
        return FALSE;
      }
      else
      {
        ptkType = pGlob->pNode->Operand[0].pTokn;
      }

      // Operand - B
      const GLOB& second_glob = pGlob->pNode->Operand[1];
      if(second_glob.IsToken())
      {
        ptkArgName = second_glob.pTokn;
      }
      else if(second_glob.IsNode())
      {
        if(second_glob.pNode->mode == SYNTAXNODE::MODE_Subscript)
        {
          type_inst.pTypeDesc = sNameCtx.RegisterMultidimIdentifier(*ptkType, second_glob.pNode, 0, NULL);
        }
        else if(second_glob.pNode->CompareOpcode(':'))
        {
          const GLOB* pVarableDecl = GetIdentifierDeclWithoutSeamantic(second_glob);
          if(pVarableDecl == NULL) {
            return NULL;
          }
          type_inst.pTypeDesc = sNameCtx.RegisterIdentifier(*ptkType, pVarableDecl, 0);
        }
        else
        {
          const TOKEN& tk = second_glob.pNode->GetAnyTokenPAB();
          clStringW strW;
          GetLogger()->OutputErrorW(tk, UVS_EXPORT_TEXT(5078, "参数错误：参数中无法识别的符号“%s”"), tk.ToString(strW).CStr());
          return FALSE;
        }

        // 断言保证只处理上面的有效分支
        ASSERT((second_glob.pNode->mode == SYNTAXNODE::MODE_Subscript) || second_glob.pNode->CompareOpcode(':'));
        if(type_inst.pTypeDesc == NULL) {
          DumpStateError(GetLogger(), sNameCtx.GetLastState(), *ptkType, *ptkArgName);
          return FALSE;
        }
        type_inst.pLocation = ptkType;
        types_array.push_back(type_inst);
        return TRUE;
      }
      else {
        // 应该到不了这里
        GetLogger()->OutputMissingSemicolon(pGlob->GetFrontToken());
        return FALSE;
      }
    }

    ASSERT(ptkType);
    if(ptkArgName == NULL)
    {
      type_inst.pTypeDesc = sNameCtx.GetType(*ptkType);
      if(type_inst.pTypeDesc == NULL) {
        GetLogger()->OutputErrorW(ptkType, UVS_EXPORT_TEXT(5061, "“%s”不是一个类型"), ptkType->ToString(strW).CStr());
        return FALSE;
      }
      nTypeOnlyCount++;
    }
    else
    {
      type_inst.pTypeDesc = sNameCtx.RegisterIdentifier(*ptkType, ptkArgName, 0);
      if(type_inst.pTypeDesc == NULL) {
        DumpStateError(GetLogger(), sNameCtx.GetLastState(), *ptkType, *ptkArgName);
        return FALSE;
      }
    }
    
    type_inst.pLocation = ptkType;
    types_array.push_back(type_inst);
    return TRUE;
  }

  GXBOOL CodeParser::ParseFunctionArguments(NameContext& sNameCtx, STATEMENT* pStat, TKSCOPE* pArgScope, TYPEINSTANCE::Array& types_array, int& nTypeOnlyCount)
  {
    // 函数参数格式
    // [InputModifier] Type Name [: Semantic]
    // 函数可以包含多个参数，用逗号分隔

    nTypeOnlyCount = 0;
    SYNTAXNODE::GlobPtrList sArgList;
    if(pArgScope->IsValid()) {
      GXBOOL bret = ArithmeticExpression::BreakComma(1, *pArgScope, &pStat->func.arguments_glob, OPP(0));
      if(bret < 0) {
        ParseArithmeticExpression(1, *pArgScope, &pStat->func.arguments_glob);
      }
    }
    else {
      return TRUE;
    }

    if(pStat->func.arguments_glob.CompareAsToken("void")) {
      pStat->func.arguments_glob.ptr = NULL;
      pStat->func.nNumOfArguments = 0;
      return TRUE;
    }

    //clStringW strW;
    const TOKEN* ptkType = NULL;
    BreakComma(sArgList, pStat->func.arguments_glob);
    pStat->func.nNumOfArguments = sArgList.size();

    for(const GLOB* pGlob : sArgList)
    {
      u32 eModifier = 0;
      
      while(pGlob->IsNode())
      {
        if(*pGlob->pNode->Operand[0].pTokn == "in") {
          eModifier |= InputModifier_in;
        }
        else if(*pGlob->pNode->Operand[0].pTokn == "out") {
          eModifier |= InputModifier_out;
        }
        else if(*pGlob->pNode->Operand[0].pTokn == "inout") {
          eModifier |= InputModifier_inout;
        }
        else if(*pGlob->pNode->Operand[0].pTokn == "uniform" || *pGlob->pNode->Operand[0].pTokn == "const") {
          eModifier |= InputModifier_uniform;
        }
        else {
          break;
        }
        pGlob = &pGlob->pNode->Operand[1];
      }

      if(eModifier != 0 &&
        eModifier != InputModifier_in && eModifier != InputModifier_out &&
        eModifier != InputModifier_inout && eModifier != InputModifier_uniform &&
        eModifier != (InputModifier_in | InputModifier_uniform))
      {
        GetLogger()->OutputErrorW(m_aTokens[pArgScope->begin], UVS_EXPORT_TEXT(5050, "参数类型修饰错误"));
        return FALSE;
      }

      if(VerifyFunctionArgument(sNameCtx, pGlob, pStat->func.arguments_glob, types_array, nTypeOnlyCount) == FALSE) {
        return FALSE;
      }
    } // for


    return TRUE;
  }

  void CodeParser::RelocalePointer()
  {
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
        if(pNode->Operand[i].IsReplaced()) {
          str[i] = "$R"; // replaced value
        }
        else if(pNode->Operand[i].IsToken()) {
          str[i] = pNode->Operand[i].pTokn->ToString();
        }
        else if(pNode->Operand[i].IsNode()) {
          if(depth == 0) {
            DbgDumpSyntaxTree(pArray, pNode->Operand[i].pNode, pNode->pOpcode ? pNode->pOpcode->precedence : 0, &str[i], 0);
          }
          else if(i == 1 && depth > 128) {
            str[i] = "...";
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
      ASSERT(str[0] == STR_RETURN);
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

    case SYNTAXNODE::MODE_BracketList:
      strOut.Format("(%s,%s)", str[0], str[1]);
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
    else if(front == "switch") {
      pend = ParseFlowSwitch(sNameSet, scope, pDesc);
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
      const TOKEN* ptkStructName = NULL;
      pend = ParseStructDefinition(sNameSet, scope, &sMembers, &sVariable, &ptkStructName, &nSignatures, &nDefinition);

      if(sMembers.ptr)
      {
        GLOB sName;
        sName.pTokn = ptkStructName;

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
    else if(front == "case" || front == "default") {
      if(TEST_FLAG_NOT(sNameSet.allow_keywords, KeywordFilter_case)) {
        GetLogger()->OutputErrorW(front, UVS_EXPORT_TEXT(2046, "非法 case"));
      }
      else {
        pend = ParseFlowCase(sNameSet, scope, pDesc);
      }
    }
    else if(front == "discard") {
      eMode = SYNTAXNODE::MODE_Flow_Discard;
      pend = scope.begin + 2;
    }
    else if(front == STR_RETURN)
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
      if(B.ptr && eMode != SYNTAXNODE::MODE_Chain) {
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
          if(B.ptr && eMode != SYNTAXNODE::MODE_Chain) {
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
      if(B.ptr && eMode != SYNTAXNODE::MODE_Chain) {
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

    if(bret && B.CompareAsNode(SYNTAXNODE::MODE_Block) == FALSE) {
      // TODO: 忘了为啥要先转成MODE_Chain，如果这个多余其它的也去掉这个
      //if(eMode != SYNTAXNODE::MODE_Chain) {
      //  bret = bret && MakeSyntaxNode(&B, SYNTAXNODE::MODE_Chain, &B, NULL);
      //}
      bret = bret && MakeSyntaxNode(&B, SYNTAXNODE::MODE_Block, &B, NULL);
    }


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

  CodeParser::TKSCOPE::TYPE CodeParser::ParseFlowSwitch(const NameContext& sParentCtx, const TKSCOPE& scope, GLOB* pDesc)
  {
    ASSERT(m_aTokens[scope.begin] == "switch");
    // switch(x) {...} 6 个token
    if(scope.begin + 6 > scope.end) {
      GetLogger()->OutputErrorW(m_aTokens[scope.begin], UVS_EXPORT_TEXT(5070, "switch ... case 意外的结束"));
      return TKSCOPE::npos;
    }

    if(m_aTokens[scope.begin + 1].scope == TKSCOPE::npos || m_aTokens[scope.begin + 1] != '(')
    {
      GetLogger()->OutputErrorW(m_aTokens[scope.begin], UVS_EXPORT_TEXT(5071, "switch 语法错误：没有正确的条件表达式"));
      return TKSCOPE::npos;
    }

    const TOKEN& tkStateBlockBegin = m_aTokens[m_aTokens[scope.begin + 1].scope + 1];
    if(tkStateBlockBegin.scope == TKSCOPE::npos || tkStateBlockBegin != '{')
    {
      GetLogger()->OutputErrorW(m_aTokens[scope.begin], UVS_EXPORT_TEXT(5072, "switch 语法错误：条件表达式后需要代码块"));
      return TKSCOPE::npos;
    }

    TKSCOPE sConditional(scope.begin + 2, m_aTokens[scope.begin + 1].scope);
    TKSCOPE sStatementBlock(sConditional.end + 1, tkStateBlockBegin.scope + 1);
    GLOB A = { 0 }, B = { 0 };

    if(sConditional.end > scope.end || sStatementBlock.end > scope.end)
    {
      GetLogger()->OutputErrorW(m_aTokens[scope.begin], UVS_EXPORT_TEXT(5070, "switch ... case 意外的结束"));
      return TKSCOPE::npos;
    }

    NameContext sSwitchCtx("switch", &sParentCtx); // 只是转换const修饰，理论上switch上下文不会注册变量，函数和类型，它的子集可以。

    GXBOOL bret = ParseArithmeticExpression(0, sConditional, &A);
    bret = bret && ParseToChain(B, &sSwitchCtx, sStatementBlock);
    bret = bret && MakeSyntaxNode(pDesc, SYNTAXNODE::MODE_Flow_Switch, NULL, &A, &B);

    return bret ? sStatementBlock.end : scope.end;
  }

  CodeParser::TKSCOPE::TYPE CodeParser::ParseFlowCase(const NameContext& sParentCtx, const TKSCOPE& scope, GLOB* pDesc)
  {
    const GXBOOL bCaseLabel = m_aTokens[scope.begin] == "case";
    ASSERT(bCaseLabel || m_aTokens[scope.begin] == "default");
    const TOKEN& front = m_aTokens[scope.begin];

    TKSCOPE::TYPE i = scope.begin + 1;
    if(bCaseLabel)
    {
      // 避免“...?...:...”三元操作符被解释为“case ...:”
      while(i < scope.end && (m_aTokens[i] != ':' || m_aTokens[i].scope == 0)) {
        i++;
      }

      if(i == scope.end) {
        GetLogger()->OutputErrorW(front, UVS_EXPORT_TEXT(5025, "case 结尾缺少“:”"));
        return scope.end;
      }
    }
    else
    {
      if(m_aTokens[i] != ':') {
        GetLogger()->OutputErrorW(front, UVS_EXPORT_TEXT(5026, "switch 表达式中 default 格式不正确"));
        return scope.end;
      }
    }

    GLOB A = { 0 }, B = { 0 }, C = { 0 };
    TKSCOPE sCaseConditional(scope.begin + 1, i);

    GXBOOL bret = bCaseLabel ? ParseArithmeticExpression(0, sCaseConditional, &A) : TRUE;
    GLOB* pCurrGlob = &B;
    TKSCOPE step_scope = {i + 1, scope.end}; // 跳过"case"

    NameContext sCaseCtx(bCaseLabel ? "case" : "default", &sParentCtx);

    while(step_scope.GetSize() > 0 && m_aTokens[step_scope.begin] != "case" && m_aTokens[step_scope.begin] != "default")
    {
      C.ptr = NULL;

      const TKSCOPE::TYPE pos = TryParseSingle(&sCaseCtx, C, step_scope);
      if(pos == TKSCOPE::npos) {
        return scope.end;
      }
      else if(pos == step_scope.begin + 1) {
        if(m_aTokens[step_scope.begin] != ';') { // 步进一次只可能是遇到了单独的分号
          return scope.end;
        }
        step_scope.begin = pos;
        continue;
      }

      MakeSyntaxNode(pCurrGlob, SYNTAXNODE::MODE_Chain, &C, NULL);
#if 0 // case 标签下初始化变量
      if(C.IsNode() && C.pNode->mode == SYNTAXNODE::MODE_Definition)
      {
        clStringW strW;
        GetLogger()->OutputErrorW(m_aTokens[step_scope.begin], UVS_EXPORT_TEXT(2360, "“%s”的初始化操作由“case”标签跳过"), C.GetFrontToken()->ToString(strW).CStr());
        return scope.end;
      }
#endif

      step_scope.begin = pos;
      pCurrGlob = &pCurrGlob->pNode->Operand[1];
    }

    bret = bret && MakeSyntaxNode(pDesc, bCaseLabel ? SYNTAXNODE::MODE_Flow_Case : SYNTAXNODE::MODE_Flow_CaseDefault, NULL, &A, &B);
    return bret ? step_scope.begin : scope.end;
  }

  CodeParser::TKSCOPE::TYPE CodeParser::ParseTypedef(NameContext& sNameSet, const TKSCOPE& scope, GLOB* pDesc)
  {
    // typedef A B;
    GLOB A = {0}, B = {0};
    GLOB S = {0};
    const TOKEN& front = m_aTokens[scope.begin];
    ASSERT(front == "typedef");

    if((front.semi_scope >= 0 && front.semi_scope - scope.begin < 3) || scope.GetSize() < 3) {
      GetLogger()->OutputErrorW(front, UVS_EXPORT_TEXT(5019, "“typedef”语法错误"));
      return TKSCOPE::npos;
    }

    if(m_aTokens[scope.begin + 1] == "struct")
    {
      TKSCOPE type_scope(scope.begin + 1, scope.end);
      GLOB sMembers = { 0 }, sTypes = { 0 };
      int nSignatures = 0, nDefinition = 0;
      const TOKEN* ptkStructName = NULL;
      clsize pend = ParseStructDefinition(sNameSet, type_scope, &sMembers, &sTypes, &ptkStructName, &nSignatures, &nDefinition);

      if(sMembers.ptr)
      {
        GLOB sName;
        sName.pTokn = ptkStructName;
        MakeSyntaxNode(&S, SYNTAXNODE::MODE_StructDef, &sName, &sMembers);
      }

      ASSERT(nSignatures == 0);  // 没处理这个错误
      if(sTypes.ptr)
      {
        SYNTAXNODE::GlobList sChainList;
        BreakChain2(sChainList, sTypes);
        for(GLOB& glob : sChainList)
        {
          if(glob.IsNode() && glob.pNode->mode == SYNTAXNODE::MODE_Definition) {
            glob.pNode->mode = SYNTAXNODE::MODE_Typedef;
            if(glob.pNode->Operand[0].IsToken() && glob.pNode->Operand[1].IsToken()) {
              const NameContext::State state = sNameSet.TypeDefine(glob.pNode->Operand[0].pTokn, glob.pNode->Operand[1].pTokn);
              if(state == NameContext::State_HasError) {
                return TKSCOPE::npos;
              }
              continue;
            }
          }
          GetLogger()->OutputMissingSemicolon(glob.GetFrontToken());
          return TKSCOPE::npos;
        }
        MakeSyntaxNode(pDesc, SYNTAXNODE::MODE_Chain, &S, NULL);
        MakeSyntaxNode(&pDesc->pNode->Operand[1], SYNTAXNODE::MODE_Chain, &sTypes, NULL);
        
        return pend;
      }

      pDesc->ptr = S.ptr;
      GetLogger()->OutputErrorW(S, UVS_EXPORT_TEXT(_WARNING(4091), "没有声明变量"));
      return pend;
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

    if(A.IsNode()) {
      GetLogger()->OutputMissingSemicolonB(A.GetBackToken());
      return TKSCOPE::npos;
    }

    NameContext::State state = sNameSet.TypeDefine(A.pTokn, B.pTokn);
    if(state == NameContext::State_HasError) {
      return TKSCOPE::npos;
    }

    MakeSyntaxNode(pDesc, SYNTAXNODE::MODE_Typedef, NULL, &A, &B);
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
    else if(TryKeywords(m_GlobalCtx, TKSCOPE(pBlock->begin, scope.end), pBlockNode, &pBlock->end))// FIXME: 暂时使用全局NameSet: m_GlobalSet
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
      if(uBlock.ptr && eMode != SYNTAXNODE::MODE_Chain) {
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
    if(clstd::strcmpT(szFunc, "EnableParserBreak") == 0)
    {
      s_bParserBreak = clStringA(szArguments).ToBoolean();
    }
    else if(clstd::strcmpT(szFunc, "EnableParserAssert") == 0)
    {
      s_bParserAssert = clStringA(szArguments).ToBoolean();
    }
    else if(clstd::strcmpT(szFunc, "EnableDumpSyntaxTree") == 0)
    {
      s_bDumpSyntaxTree = clStringA(szArguments).ToBoolean();
    }
    ArithmeticExpression::Invoke(szFunc, szArguments);
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
        clStringW strW;
        GetLogger()->OutputErrorW(tokens.front(), UVS_EXPORT_TEXT(2005, "#line 应输入行号，却找到“%s”"), tokens.size() > 2 ? tokens[2].ToString(strW).CStr() : _CLTEXT("new line"));
        return ctx.iter_next.end(); // 这个才是正确的结束位置，end似乎不是
      }
      else if(tokens[1].type != TOKEN::TokenType_Integer)
      {
        GetLogger()->OutputErrorW(tokens.front(), UVS_EXPORT_TEXT(2005, "#line 应输入行号，却找到“%s”"), tokens[1].ToString(str).CStr());
        return end;
      }

      GXINT nLine = tokens[1].ToString().ToInteger();
      if(nLine > 0)
      {
        GetLogger()->SetLine(tokens[1].marker, nLine);
      }
      else {
        // ERROR: 行号不正确
      }
    }
    else {
      GetLogger()->OutputErrorW(tokens.front(), UVS_EXPORT_TEXT(_WARNING(1021), "无效的预处理器命令 \"%s\"."), clStringW(tokens.front().ToString()));
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
    l_m.nid = m_pContext->mid++;
    //m_MacrosSet.insert(strMacroName);

    if(count == 1) {
      GetLogger()->OutputErrorW(tokens.front(), UVS_EXPORT_TEXT(2007, "#define 缺少定义."));
      return;
    }
    
    RefString rstrMacroName(tokens[1].marker, tokens[1].length);

    if(count == 2) // "#define MACRO" 形
    {
      l_m.nNumTokens = 1;
      AddMacro(rstrMacroName, l_m);
    }
    else if(count == 3) // "#define MACRO XXX" 形
    {
      // "#define MACROxxx" 异形, xxx可能是字符串, 符号等
      if(tokens[1].end() == tokens[2].begin()) {
        clStringW str;
        GetLogger()->OutputErrorW(tokens[1], UVS_EXPORT_TEXT(2008, "“%s”: 宏定义中的意外"), tokens[2].ToString(str).CStr());
      }

      l_m.nNumTokens = 1;
      auto result = AddMacro(rstrMacroName, l_m);

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

      auto result = AddMacro(rstrMacroName, l_m);
      if( ! result.second) {
        result.first->second.clear();
      }

      // 设置宏代替的表达式，如果表达式中含有宏也会展开
      result.first->second.aFormalParams.insert(
        result.first->second.aFormalParams.begin(),
        sFormalList.begin(), sFormalList.end());

      result.first->second.set(m_pContext->Macros, tokens, l_define);
    }

    NormalizeMacro(rstrMacroName);
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

    RefString rstrMacroName(aTokens[1].marker, aTokens[1].length);
    auto it = m_pContext->Macros.find(rstrMacroName);

    if(it != m_pContext->Macros.end())
    {
      m_pContext->Macros.erase(it);
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
      RefString rstr(tokens[1].marker, tokens[1].length);
      const GXBOOL bNotDefined = (m_pContext->Macros.find(rstr) == m_pContext->Macros.end());
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
      GetLogger()->OutputErrorW(aTokens[1], UVS_EXPORT_TEXT(_WARNING(1021), "无效的预处理器命令 “%s”"), clStringW(aTokens[1].ToString()));
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

        RefString rstr(pNode->Operand[1].pTokn->marker, pNode->Operand[1].pTokn->length);
        auto it = m_pContext->Macros.find(rstr);

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
        RefString rstrMacro(sDesc.pTokn->marker, sDesc.pTokn->length);
        auto it = m_pContext->Macros.find(rstrMacro);
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
        // depth 不空，说明在预处理域内，直接忽略
        // depth 为空，测试表达式(TODO)
        //if( ! sRankStack.empty()) {
        if(depth || session == PPCondRank_elif) {
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

        pParse->Attach(p, (clsize)line_end - (clsize)p, AttachFlag_Preprocess, NULL);
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
        // depth 不空，说明在预处理域内，直接忽略
        // depth 为空，转到下行行首
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
        GetLogger()->OutputErrorW(p, UVS_EXPORT_TEXT(_WARNING(1021), "无效的预处理器命令 \"%s\"."), str.CStr());
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

  UVShader::ArithmeticExpression::T_LPCSTR CodeParser::Macro_SkipComment(T_LPCSTR p, T_LPCSTR end)
  {
    // 跳过块注释
    if(*p == '/' && p + 1 < end && p[1] == '*') {
      p += 2;
      while(_CL_NOT_(p[0] == '*' && p + 1 < end && p[1] == '/'))
      {
        if(++p >= end) return end;
      }
      p += 2;
    }
    return p;
  }

  CodeParser::T_LPCSTR CodeParser::Macro_SkipGapsAndNewLine(T_LPCSTR p, T_LPCSTR end)
  {
    p = Macro_SkipComment(p, end);
    while(p < end && (*p == '\t' || *p == 0x20 || *p == '\r' || *p == '\n'))
    {
      ++p;
      p = Macro_SkipComment(p, end);
    }
    return p;

  }

  CodeParser::T_LPCSTR CodeParser::Macro_SkipGaps(T_LPCSTR p, T_LPCSTR end)
  {
    do {
      p = Macro_SkipComment(p, end);
      p++;
    } while (p < end && (*p == '\t' || *p == 0x20));
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

  void CLogger::OutputMissingSemicolon(const TOKEN* ptkLocation)
  {
    clStringW strW;
    OutputErrorW(ptkLocation, UVS_EXPORT_TEXT2(2143, "语法错误 : 缺少“;”(在“%s”的前面)", this), ptkLocation->ToString(strW).CStr());
  }

  void CLogger::OutputMissingSemicolonB(const TOKEN* ptkLocation)
  {
    clStringW strW;
    OutputErrorW(ptkLocation, UVS_EXPORT_TEXT2(5081, "语法错误 : 缺少“;”(在“%s”的后面)", this), ptkLocation->ToString(strW).CStr());
  }

  void CLogger::OutputTypeCastFailed(const TOKEN* ptkLocation, GXLPCWSTR szOpcode, const TYPEDESC* pTypeTo, const TYPEDESC* pTypeFrom)
  {
    clStringW strTo, strFrom;
    pTypeTo->name.ToString(strTo);
    pTypeFrom->name.ToString(strFrom);
    //pOpcode->ToString(strOpcode);
    OutputErrorW(ptkLocation, UVS_EXPORT_TEXT2(2440, "“%s”: 无法从“%s”转换为“%s”", this),
      szOpcode, strFrom.CStr(), strTo.CStr());
  }

  void CLogger::OutputTypeCastFailed(const TOKEN* ptkLocation, const TOKEN* pOpcode, const TYPEDESC* pTypeTo, const TYPEDESC* pTypeFrom)
  {
    clStringW strOpcode;
    OutputTypeCastFailed(ptkLocation, pOpcode ? pOpcode->ToString(strOpcode) : _CLTEXT("="), pTypeTo, pTypeFrom);
  }

  void CLogger::VarOutputErrorW(const TOKEN* pLocation, GXUINT code, va_list arglist) const
  {
#ifdef REDUCE_ERROR_MESSAGE
    // waning code 大于 c_nErrorIdLimit，所以不受显示数量限制
    if(code < c_nErrorIdLimit &&
      (m_nDisplayedError >= c_nMaxErrorCount || m_nSessionError > c_nMaxSessionError))
    {
      return;
    }
#endif // REDUCE_ERROR_MESSAGE
    const GXUINT error_id = code & (~UVS_WARNING_MASK);
    const GXBOOL bError = TEST_FLAG_NOT(code, UVS_WARNING_MASK);

    if(pLocation)
    {
      if(pLocation->bPhony) {
        T_LPCSTR pOriginMarker = static_cast<const ArithmeticExpression*>(pLocation->pContainer)->GetOriginPtr(pLocation);
        if(pOriginMarker)
        {
          m_pMsg->VarWriteErrorW(bError, pOriginMarker, error_id, arglist);
          return;
        }
      }
      else {
        m_pMsg->VarWriteErrorW(bError, pLocation->marker, error_id, arglist);
        return;
      }
    }

    // pLocation 为空或者 bPhony 下没找到
    m_pMsg->VarWriteErrorW(bError, (GXSIZE_T)0, error_id, arglist);
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
    const GXUINT error_id = code & (~UVS_WARNING_MASK);
    m_pMsg->VarWriteErrorW(TEST_FLAG_NOT(code, UVS_WARNING_MASK), ptr, error_id, arglist);
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

  const TOKEN* CodeParser::GetIdentifierNameWithoutSeamantic(const GLOB& glob)
  {
    // 递归解析带语意的变量定义
    // <Identifier> [: seamantic1] [: seamantic2] ...
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
        return GetIdentifierNameWithoutSeamantic(glob.pNode->Operand[0]);
      }
    }

    return NULL;
  }

  // 获得去除语意的数组变量
  const SYNTAXNODE::GLOB* CodeParser::GetIdentifierDeclWithoutSeamantic(const GLOB& glob)
  {
    // 递归解析带语意的变量定义
    // <Identifier>["[m]"]["[n]"] [: seamantic1] [: seamantic2] ...
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
        return GetIdentifierDeclWithoutSeamantic(glob.pNode->Operand[0]);
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

  GXBOOL CodeParser::Verify_IdentifierDefinition(NameContext& sNameSet, const SYNTAXNODE* pNode, GXDWORD dwFlags)
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
          //TYPEDESC sDesc = { TYPEDESC::TypeCate_Empty, &sNameSet };
          pNode->Operand[1].pTokn->ToString(strType);

          if(sNameSet.GetType(*pNode->Operand[1].pTokn)/* || NameContext::TestIntrinsicType(&sDesc, strType)*/)
          {
            // ERROR: "const float;" 形式
            GetLogger()->OutputErrorW(*pNode->Operand[1].pTokn, UVS_EXPORT_TEXT(_WARNING(4091), "没有声明变量"));
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
            return Verify_IdentifierDefinition(sNameSet, pNode->Operand[1].pNode, dwFlags | VerifyIdentifierDefinition_Const);
          }
          else {
            // ERROR: "const i = 0;" 形式
            GetLogger()->OutputErrorW(pNode->GetAnyTokenBPA(), UVS_EXPORT_TEXT(4430, "缺少类型说明符"));
            return FALSE;
          }
        }
        else {
          // ERROR: "const;" 形式
          GetLogger()->OutputErrorW(*pNode->Operand[1].pTokn, UVS_EXPORT_TEXT(4430, "缺少类型说明符"));
          return FALSE;
        }
      }
      else  if(*pNode->Operand[0].pTokn == "static")
      {
        // 递归
        return Verify_IdentifierDefinition(sNameSet, pNode->Operand[1].pNode, dwFlags | VerifyIdentifierDefinition_Static);
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

    return Verify_IdentifierTypedDefinition(sNameSet, *pNode->Operand[0].pTokn, pNode->Operand[1], dwFlags);
  }

  GXBOOL CodeParser::Verify_IdentifierTypedDefinition(NameContext& sNameSet, const TOKEN& tkType, const GLOB& second_glob, GXDWORD dwFlags)
  {
    const TYPEDESC* pRightTypeDesc = NULL;

    const TOKEN* ptkVar = NULL;
    const TYPEDESC* pType = NULL;

    if(second_glob.IsToken())
    {
      ptkVar = second_glob.pTokn;
      pType = sNameSet.RegisterIdentifier(tkType, ptkVar, dwFlags);
    }
    else if(second_glob.IsNode())
    {
      if(second_glob.pNode->mode == SYNTAXNODE::MODE_Subscript) // 下标
      {
        pType = sNameSet.RegisterMultidimIdentifier(tkType, second_glob.pNode, 0, NULL);
      }
      else if(second_glob.pNode->mode == SYNTAXNODE::MODE_Subscript0) // 自适应下标
      {
        // error C2133: “a”: 未知的大小
        clStringW strW;
        ptkVar = GetIdentifierNameWithoutSeamantic(second_glob);
        GetLogger()->OutputErrorW(ptkVar, UVS_EXPORT_TEXT(2133, "“%s”: 未知的大小"), ptkVar->ToString(strW).CStr());
        return FALSE;
      }
      else if(second_glob.pNode->CompareOpcode(':')) // 语义
      {
        ptkVar = GetIdentifierNameWithoutSeamantic(second_glob);
        if(ptkVar)
        {
          pType = sNameSet.RegisterIdentifier(tkType, ptkVar, dwFlags);
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


        GLOB& right_glob = second_glob.pNode->Operand[1];
        if(right_glob.IsNode() == FALSE || right_glob.pNode->mode != SYNTAXNODE::MODE_InitList) {
          GetLogger()->OutputErrorW(tkType, UVS_EXPORT_TEXT(5048, "预期应该是初始化列表"));
          return FALSE;
        }
        else if(right_glob.pNode->Operand[0].ptr == NULL) {
          GetLogger()->OutputErrorW(tkType, UVS_EXPORT_TEXT(5049, "初始化列表不应该为空"));
          return FALSE;
        }

        const GLOB* pVarableDecl = GetIdentifierDeclWithoutSeamantic(second_glob.pNode->Operand[0]);

        if(pVarableDecl)
        {
          pType = sNameSet.RegisterIdentifier(tkType, pVarableDecl, dwFlags, &right_glob);
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
        const IDNFDESC* pVariDesc = sNameSet.GetIdentifierDesc(ptkVarName);
        ASSERT(pVariDesc != NULL && pVariDesc->glob.CompareAsNode(SYNTAXNODE::MODE_InitList));
        ASSERT(right_glob.CompareAsNode(SYNTAXNODE::MODE_InitList));
        right_glob.pNode = pVariDesc->glob.pNode;

        ASSERT(pType || sNameSet.GetLastState() != NameContext::State_Ok);
      }
      else if(second_glob.pNode->CompareOpcode('=')) // 赋值
      {
        const GLOB& right_glob = second_glob.pNode->Operand[1];
        const size_t nErrorCount = DbgErrorCount();
        const GLOB* pVarableDecl = GetIdentifierDeclWithoutSeamantic(second_glob.pNode->Operand[0]);

        ptkVar = GetIdentifierNameWithoutSeamantic(*pVarableDecl);
        
        if(pVarableDecl == NULL)
        {
          GetLogger()->OutputErrorW(second_glob.pNode->Operand[0], UVS_EXPORT_TEXT(5057, "定义标识符语法错误"));
          return NULL;
        }

        if(TEST_FLAG(dwFlags, VerifyIdentifierDefinition_Const)) //if(bConstIdentifier)
        {
          // TODO: 检查value与tkType类型是否匹配, 比如一个“string s = 23;”是非法的
          pRightTypeDesc = pType = sNameSet.RegisterIdentifier(tkType, pVarableDecl, VerifyIdentifierDefinition_Const, &right_glob);
          if(pType) {
            const IDNFDESC* pIdnfDesc = sNameSet.GetIdentifierDesc(ptkVar);
            if(pIdnfDesc && pIdnfDesc->pool.size() == 1) {
              SetRepalcedValue(right_glob, pIdnfDesc->pool.front());
            }
          }
        }
        else
        {
          pRightTypeDesc = InferRightValueType(sNameSet, right_glob, ptkVar);
          if(pRightTypeDesc == NULL || pVarableDecl == NULL) {
            // InferRightValueType2和GetIdentifierWithoutSeamantic内部应该输出错误
            ASSERT(DbgErrorCount() > nErrorCount);
            return NULL;
          }

          pType = sNameSet.RegisterIdentifier(tkType, pVarableDecl, dwFlags, &right_glob);
          // 后面比较pRightTypeDesc是否能转换为pType
        }

        // 右侧类型推导失败，注册变量名（RegisterIdentifier）后再退出，
        // 这样后面就不会报找不到变量的错误

        ASSERT(pType || sNameSet.GetLastState() != NameContext::State_Ok);
      }
      else if(second_glob.pNode->CompareOpcode(','))
      {
        const size_t nErrorCount = DbgErrorCount();
        //ASSERT(bMember == FALSE); // 成员变量定义在结构体解析时已经展开了，不应该存在“,”并列式
        ASSERT(TEST_FLAG(dwFlags, VerifyIdentifierDefinition_Member) == FALSE); // 成员变量定义在结构体解析时已经展开了，不应该存在“,”并列式
        if(Verify_IdentifierTypedDefinition(sNameSet, tkType, second_glob.pNode->Operand[0], dwFlags) == FALSE ||
          Verify_IdentifierTypedDefinition(sNameSet, tkType, second_glob.pNode->Operand[1], dwFlags) == FALSE)
        {
          ASSERT(DbgErrorCount() > nErrorCount);
          return FALSE;
        }
        return TRUE;
      }
      else
      {
        const TOKEN* ptkLocation = second_glob.pNode->GetAnyTokenPtrPAB();
        if(ptkLocation) {
          GetLogger()->OutputMissingSemicolon(ptkLocation);
          return FALSE;
        }
        // 意料之外的变量定义语法
        GetLogger()->OutputMissingSemicolon(second_glob.GetFrontToken());
        return FALSE;
      }
    }
    else
    {
      PARSER_BREAK(second_glob);
      // 定义但是没写名字，报错
      return FALSE;
    }

    // 检查变量名
    if(pType == NULL)
    {
      clStringW strW;
      ptkVar = ptkVar != NULL ? ptkVar : second_glob.GetFrontToken();

      DumpStateError(GetLogger(), sNameSet.GetLastState(), tkType, *ptkVar);
      return FALSE;
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
          GetLogger()->OutputTypeCastFailed(&token, second_glob.pNode->pOpcode, pType, pRightTypeDesc);
          return FALSE;
        }
      }
    }
    return TRUE;
  }

  GXBOOL CodeParser::Verify_Chain(const SYNTAXNODE* pNode, NameContext& sNameContext)
  {
    GXBOOL result = TRUE;

    if(pNode->mode == SYNTAXNODE::MODE_Chain) // 对chain进行特殊处理，防止堆栈溢出
    {
      do {
        // 验证第一个节点
        if(pNode->Operand[0].IsNode())
        {
          GXBOOL bCheckNode = Verify_Node(pNode->Operand[0].pNode, sNameContext, result);
          if(result == FALSE && TEST_FLAG_NOT(m_dwParserState, State_MoreValidation)) {
            return result;
          }
          else if(bCheckNode)
          {
            const SYNTAXNODE* pSubNode = pNode->Operand[0].pNode;

            RecursiveNode<const SYNTAXNODE>(this, pSubNode, [this, &result, &sNameContext]
            (const SYNTAXNODE* pNode, int depth) -> GXBOOL
            {
              return Verify_Node(pNode, sNameContext, result);
            });
            if(result == FALSE) {
              return result;
            }
          }
        }
        else if(pNode->Operand[0].IsToken())
        {
          if(sNameContext.GetType(*pNode->Operand[0].pTokn) != NULL) // 类型
          {
            clStringW strW;
            GetLogger()->OutputErrorW(pNode->Operand[0].pTokn, UVS_EXPORT_TEXT(_WARNING(5059), "“%s” : 孤立的定义."), pNode->Operand[0].pTokn->ToString(strW).CStr());
          }
          else if(InferRightValueType(sNameContext, pNode->Operand[0], pNode->Operand[0].pTokn) == NULL) {
            return FALSE;
          }
        }
        else {
          if(pNode->Operand[1].ptr) {
            GetLogger()->OutputMissingSemicolon(&pNode->GetAnyTokenAPB());
            return FALSE;
          }
        }

        // 转移第二个节点到当前pNode
        if(pNode->Operand[1].ptr && pNode->Operand[1].IsNode())
        {
          pNode = pNode->Operand[1].pNode;
        }
        else {
          ASSERT(pNode->Operand[1].ptr == NULL); // 好像没有其它情况了，这里限制一下看看
          break;
        }
      } while(pNode->mode == SYNTAXNODE::MODE_Chain);
    }
    else
    {
      RecursiveNode<const SYNTAXNODE>(this, pNode, [this, &result, &sNameContext]
      (const SYNTAXNODE* pNode, int depth) -> GXBOOL
      {
        return Verify_Node(pNode, sNameContext, result);
      });
    }
    return result;
  }

  GXBOOL CodeParser::Verify_Node(const SYNTAXNODE* pNode, NameContext& sNameContext, GXBOOL& result)
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
        if(InferRightValueType(sNameContext, pNode->Operand[0], pNode->Operand[0].pTokn) == NULL)
        {
          result = FALSE;
        }
      }
      return FALSE;
    }
    else if(pNode->mode == SYNTAXNODE::MODE_Chain)
    {
      result = Verify_Chain(pNode, sNameContext);
      return FALSE;
    }
    else if(
      pNode->mode == SYNTAXNODE::MODE_Flow_ForInit ||
      pNode->mode == SYNTAXNODE::MODE_Flow_ForRunning ||
      pNode->mode == SYNTAXNODE::MODE_Flow_While )
    {
      return TRUE;
    }
    else if(pNode->mode == SYNTAXNODE::MODE_Flow_DoWhile) // TODO: 把MODE_Flow_If合并进来测试一下
    {
      const TYPEDESC* pTypeDesc = InferRightValueType(sNameContext, pNode->Operand[0], pNode->Operand[0].GetFrontToken());
      if(result = (pTypeDesc != NULL)) {
        if(pNode->Operand[1].IsToken()) {
          VALUE_CONTEXT vctx(sNameContext);
          vctx.pLogger = GetLogger();
          pTypeDesc = InferType(vctx, pNode->Operand[1].pTokn);
        }
        else if(pNode->Operand[1].IsNode()) {
          ASSERT(pNode->Operand[1].CompareAsNode(SYNTAXNODE::MODE_Block)); // 节点解析时保证
          result = Verify_Block(pNode->Operand[1].pNode, &sNameContext);
        }
      }
      return FALSE;
    }
    else if(pNode->mode == SYNTAXNODE::MODE_Flow_If) // TODO: 应该和MODE_Flow_DoWhile合并？
    {
      const TYPEDESC* pTypeDesc = InferRightValueType(sNameContext, pNode->Operand[0], pNode->Operand[0].GetFrontToken());
      result = result && (pTypeDesc != NULL);
      ASSERT(pNode->Operand[1].ptr == NULL || pNode->Operand[1].IsNode() && pNode->Operand[1].pNode->mode == SYNTAXNODE::MODE_Block);
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
      if(Verify_IdentifierDefinition(sNameContext, pNode) == FALSE) {
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
          *pNode->pOpcode == "&=" || *pNode->pOpcode == "|=" || *pNode->pOpcode == "^=")
        {
          // 表达式中如果左侧出现错误就不再检查右侧，主要是防止重复信息太多
          // TODO: 需要验证左值
          const size_t nErrorCount = DbgErrorCount();
          const TYPEDESC* pRightTypeDesc = InferRightValueType(sNameContext, pNode->Operand[1], pNode->pOpcode);
          const TYPEDESC* pLeftTypeDesc = Verify2_LeftValue(sNameContext, pNode->Operand[0], *pNode->pOpcode);

          if(pRightTypeDesc == NULL || pLeftTypeDesc == NULL)
          {
            ASSERT(DbgErrorCount() > nErrorCount);
            return FALSE;
          }

          if(TryTypeCasting(pLeftTypeDesc, pRightTypeDesc, pNode->pOpcode) == FALSE)
          {
            if(*pNode->pOpcode == "*=" && pRightTypeDesc->IsMatrix() && pLeftTypeDesc->IsVector() &&
              IsComponent(NULL, pRightTypeDesc, pLeftTypeDesc))
            {
            }
            else
            {
              GetLogger()->OutputTypeCastFailed(&pNode->GetAnyTokenPAB(), pNode->pOpcode, pLeftTypeDesc, pRightTypeDesc);
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
              if(InferRightValueType(sNameContext, pNode->Operand[i], pNode->pOpcode) == NULL)
              {
                result = FALSE;
              }
              break;
            }
          }
        }
        else
        {
          VALUE_CONTEXT vctx(sNameContext);
          vctx.bNeedValue = FALSE;
          vctx.pLogger = GetLogger();
          InferType(vctx, pNode);
          if(vctx.result != ValueResult_OK && vctx.result != ValueResult_Variable) {
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
      ASSERT(pTypeTo);

      if(pTypeFrom == NULL) {
        result = FALSE;
      }
      else if(TryTypeCasting(pTypeTo, pTypeFrom, pNode->Operand[0].pTokn) == FALSE)
      {
        // error C2440: “return”: 无法从“TypeFrom”转换为“TypeTo”
        GetLogger()->OutputTypeCastFailed(pNode->Operand[0].pTokn, _CLTEXT("return"), pTypeTo, pTypeFrom);
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
    else if(pNode->mode == SYNTAXNODE::MODE_Flow_Switch)
    {
      return FALSE;
    }
    else if(pNode->mode == SYNTAXNODE::MODE_Subscript ||
      pNode->mode == SYNTAXNODE::MODE_BracketList)
    {
      VALUE_CONTEXT vctx(sNameContext, FALSE);
      vctx.pLogger = GetLogger();
      InferType(vctx, pNode);
      return FALSE;
    }
    else if(pNode->mode == SYNTAXNODE::MODE_Assignment)
    {
      clStringW strW;
      const TOKEN* ptkVar = GetIdentifierNameWithoutSeamantic(pNode->Operand[0]);
      if(ptkVar)
      {
        ptkVar->ToString(strW);
        GetLogger()->OutputErrorW(pNode, UVS_EXPORT_TEXT(5053, "“%s”无法使用初始化列表赋值"), strW.CStr());
      }
      else
      {
        GetLogger()->OutputMissingSemicolon(pNode->GetAnyTokenPtrPAB());
      }
      return FALSE;
    }
    else if(pNode->mode == SYNTAXNODE::MODE_Typedef)
    {
      return FALSE;
    }

    PARSER_BREAK(pNode); // 应该处理 pNode->mode
    return TRUE;
  }

  GXBOOL CodeParser::Verify_Block(const SYNTAXNODE* pNode, const NameContext* pParentSet)
  {
    ASSERT(pNode->mode == SYNTAXNODE::MODE_Block);
    NameContext sNameContext(s_szNCName_Block, pParentSet);
    return Verify_Chain(pNode, sNameContext);
  }

  GXBOOL CodeParser::Verify_StructMember(NameContext& sParentSet, const RefString& rstrStructName, const SYNTAXNODE& rNode)
  {
    GXBOOL result = TRUE;
    NameContext* pStructMemberSet = new NameContext(s_szStructMember, &sParentSet, NULL);
    RecursiveNode<const SYNTAXNODE>(this, &rNode, [this, &result, pStructMemberSet](const SYNTAXNODE* pNode, int depth) -> GXBOOL
    {
      if(pNode->mode == SYNTAXNODE::MODE_Definition)
      {
        const GLOB& second_glob = pNode->Operand[1];
        const TOKEN* ptkVar = NULL;
        const TYPEDESC* pType = NULL;
        clStringA strType;
        
        const TOKEN* ptkType = pNode->Operand[0].pTokn;
        //ptkType->ToString(strType);
        pType = pStructMemberSet->GetType(*ptkType);
        if(pType == NULL)
        {
          clStringW strW;
          ptkType->ToString(strW);
          GetLogger()->OutputErrorW(*ptkVar, UVS_EXPORT_TEXT(2062, "“%s”: 意外的类型"), strW.CStr());
          return FALSE;
        }

        if(second_glob.IsToken())
        {
          ptkVar = second_glob.pTokn;
          pType = pStructMemberSet->RegisterIdentifier(*ptkType, ptkVar, 0);
        }
        else if(second_glob.IsNode())
        {
          if(second_glob.pNode->mode == SYNTAXNODE::MODE_Subscript) // 下标
          {
            pType = pStructMemberSet->RegisterMultidimIdentifier(*ptkType, second_glob.pNode, 0, NULL);
          }
          else if(second_glob.pNode->mode == SYNTAXNODE::MODE_Subscript0) // 自适应下标
          {
            CLBREAK;
          }
          else if(second_glob.pNode->CompareOpcode(':')) // 语义
          {
            //ptkVar = &second_glob.pNode->GetAnyTokenAB();
            ptkVar = GetIdentifierNameWithoutSeamantic(second_glob);
            if(ptkVar)
            {
              pType = pStructMemberSet->RegisterIdentifier(*ptkType, ptkVar, 0);
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
          case NameContext::State_DuplicatedIdentifier:
            GetLogger()->OutputErrorW(*ptkVar, UVS_EXPORT_TEXT(2030, "“%s”: 结构成员重定义"), ptkVar->ToString(strW));
            break;
          case NameContext::State_TypeNotFound:
            CLBREAK; // 上面判断了，这里不应该有
          case NameContext::State_HasError:
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

    sParentSet.RegisterStructContext(rstrStructName, pStructMemberSet);
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
      pTypeDesc = sNameSet.GetIdentifier(left_glob.pTokn/*->ToString(strA)*/);
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
        pTypeDesc = InferSubscriptType(vctx, NULL, pLeftNode);
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

  const TYPEDESC* CodeParser::InferUserFunctionType(FUNCDESC::CPtrList& aUserFunc, VALUE_CONTEXT& vctx, const TYPEDESC::CPtrList& sTypeList, const SYNTAXNODE* pFuncNode, int nStep)
  {
    // 返回ERROR_TYPEDESC表示语法出现错误而失败
    // 返回NULL表示没找到匹配函数

    static GXBOOL bTolerance[2] = {FALSE, TRUE}; // 第一次严格匹配，防止找到重载函数，如果找不到第二遍放宽匹配条件
    //cllist<const FUNCDESC*> aUserFunc;
    if(nStep == 0) {
      vctx.name_ctx.GetMatchedFunctions(pFuncNode->Operand[0].pTokn, -1/*sTypeList.size()*/, aUserFunc);
    }

    for(auto iter_func = aUserFunc.begin(); iter_func != aUserFunc.end(); ++iter_func)
    {
      if((*iter_func)->sFormalTypes.size() != sTypeList.size()) {
        continue;
      }

      int result = CompareFunctionArguments(vctx.name_ctx, pFuncNode->Operand[0].pTokn, (*iter_func)->sFormalTypes, sTypeList, bTolerance[nStep]);
      if(result == -1) {
        vctx.ClearValue(ValueResult_CanNotInferType);
        return ERROR_TYPEDESC;
      }
      else if(result == 1) {
        vctx.ClearValueOnly();
        vctx.result = ValueResult_OK;
        vctx.pType = vctx.name_ctx.GetType((*iter_func)->ret_type);
        return vctx.pType;
      }
    }
    return NULL;
  }

  int CodeParser::CompareFunctionArguments(const NameContext &sNameSet, const TOKEN* ptkFuncName, const TYPEINSTANCE::Array& sFormalTypes, const TYPEDESC::CPtrList &sCallTypeList, GXBOOL bTolerance)
  {
    // -1:出错，0：不匹配，1：匹配
    // bTolerance = TRUE 更宽容的匹配，float可以映射为float2以上的类型
    int i = 0;
    for(auto iter_arg = sCallTypeList.begin(); iter_arg != sCallTypeList.end(); ++iter_arg, ++i)
    {
      const TYPEDESC* pArgumentTypeDesc = *iter_arg;
      const TYPEDESC* pFormalTypeDesc = sFormalTypes[i].pTypeDesc; // sNameSet.GetType(*sFormalTypes[i]);

      if(pFormalTypeDesc == NULL)
      {
        clStringW strW;
        GetLogger()->OutputErrorW(*sFormalTypes[i].pLocation,
          UVS_EXPORT_TEXT(2062, "意外的类型“%s”"), sFormalTypes[i].pLocation->ToString(strW).CStr());
        return -1;
      }

      // TODO: TryTypeCasting 最后这个参数只是大致定位,改为更准确的!
      if(pArgumentTypeDesc == NULL)
      {
        return -1; // 无法推导参数类型
      }
      else if(_CL_NOT_(TryTypeCasting(pFormalTypeDesc, pArgumentTypeDesc, sFormalTypes[i].pLocation, !(bTolerance)))) {
        return 0;
      }
    }
    return 1;
  }

  GXLPCSTR CodeParser::InferBuildinFunction(const RefString& rstrFunctionName, const TYPEDESC::CPtrList& sArgumentsTypeList, GXBOOL* pError)
  {
    pError = FALSE;
    RefString rstrKey = rstrFunctionName;

    const BUILDIN_FUNCTION_PROTOTYPE* pPrototypeEnd = s_functions_prototype + s_functions_prototype_len;
    const BUILDIN_FUNCTION_PROTOTYPE* pPrototype =
      clstd::BinarySearch((const BUILDIN_FUNCTION_PROTOTYPE*)s_functions_prototype, pPrototypeEnd, rstrKey,
      [](const BUILDIN_FUNCTION_PROTOTYPE* pPrototype, const RefString& rstrKey) -> int
    {
      return RefString(pPrototype->name).Compare(rstrKey);
    });

    if(pPrototype == NULL) {
      return NULL;
    }
    
    do {
      if(pPrototype->count == sArgumentsTypeList.size())
      {
        GXLPCSTR szParamTypeName = pPrototype->formal_param;
        auto it = sArgumentsTypeList.begin();
        size_t pi = 0;

        // 比较形参名，必须完全一致
        for(; pi < pPrototype->count; pi++, szParamTypeName += ((*it)->name.GetLength() + 1), ++it)
        {
          if((*it)->name != szParamTypeName) {
            break;
          }
        }

        if(pi == pPrototype->count) {
          return pPrototype->type;
        }
      }

      pPrototype++;
    } while (
      pPrototype < pPrototypeEnd &&
      clstd::strlenT(pPrototype->name) - 1 == rstrKey.GetLength() &&
      pPrototype->name[rstrKey.GetLength()] == '$'
      );
    return NULL;
  }

  GXBOOL CodeParser::InferBuildinFunction_Wildcard(VALUE_CONTEXT& vctx,
    const RefString& rstrFunctionName,
    const SYNTAXNODE::GlobList& sExprList, const TYPEDESC::CPtrList& sArgumentsTypeList, const VALUE_CONTEXT::Array& vctx_params)
  {
    //CHECK_VALUE_CONTEXT;
    ASSERT(sExprList.size() == sArgumentsTypeList.size());

    if(sExprList.size() == 1)
    {
      if(InferBuildinFunction_WildcardTable( // 一个参数函数原型
        s_wildcard_unary_functions, s_wildcard_unary_functions_len,
        vctx, rstrFunctionName, sExprList, sArgumentsTypeList) == FALSE) {
        return FALSE;
      }
    }
    else if(InferBuildinFunction_WildcardTable( // 多参数函数原型
      s_wildcard_functions, s_wildcard_functions_len,
      vctx, rstrFunctionName, sExprList, sArgumentsTypeList) == FALSE)
    {
      return FALSE;
    }

    if(vctx.bNeedValue)
    {
      // TODO: 修改一下，现在不是常量折叠也会进到这里来计算
      VALUE_CONTEXT::Array _vctx_params = vctx_params;
      if(rstrFunctionName == "max" || rstrFunctionName == "min" || rstrFunctionName == "pow")
      {
        size_t n = 0;
        const size_t count = vctx.pType->CountOf();
        for(VALUE_CONTEXT& vp : _vctx_params) {
          vp.CopyValue(vctx_params[n++]);
          if(vp.TypeRank() < vctx.TypeRank() || vp.count < count) {
            vp.CastUpward(vctx.pType);
          }
        }
        
        //VALUE value;
        vctx.result = ValueResult_OK;
        vctx.pool.clear();
        //const size_t count = vctx.pType->CountOf();

        if(rstrFunctionName == "max")
        {
          for(size_t i = 0; i < count; i++)
          {
            vctx.pool.push_back(
              _vctx_params[0].pValue[i].Compare(
                _vctx_params[1].pValue[i]) >= 0 ? _vctx_params[0].pValue[i] : _vctx_params[1].pValue[i]);
          }
          vctx.UsePool();
        }
        else if(rstrFunctionName == "min")
        {
          for(size_t i = 0; i < count; i++)
          {
            vctx.pool.push_back(
              _vctx_params[0].pValue[i].Compare(
                _vctx_params[1].pValue[i]) <= 0 ? _vctx_params[0].pValue[i] : _vctx_params[1].pValue[i]);
          }
          vctx.UsePool();
        }
        else if(rstrFunctionName == "pow")
        {
          VALUE value;
          for(size_t i = 0; i < count; i++)
          {
            double r = pow(_vctx_params[0].pValue[i].CastTo<double>(), _vctx_params[1].pValue[i].CastTo<double>());
            value.set(VALUE::Rank_Double, &r);
            vctx.pool.push_back(value);
            vctx.pool.back().CastValueByRank(vctx.TypeRank());
          }
          vctx.UsePool();
        }
        else
        {
          CLBREAK;
        }
      }
      //else {
      //  vctx.bNeedValue = FALSE;
      //  vctx.ClearValueOnly();
      //}
    }

    return TRUE;
  }

  GXBOOL CodeParser::InferBuildinFunction_WildcardTable(
    INTRINSIC_FUNC* pFunctionsTable, size_t nTableLen, 
    VALUE_CONTEXT& vctx,
    const RefString& rstrFunctionName,
    const SYNTAXNODE::GlobList& sExprList, const TYPEDESC::CPtrList& sArgumentsTypeList)
  {
    // 出错返回 FALSE，正常状态返回TRUE，根据vctx确定是否有结果
    // sExprList 用于消息定位，不要做其它用处

    ASSERT(vctx.pType == NULL); // 外部保证这个，至少应该清理一下

    const TYPEDESC* pRetType = NULL;

    // TODO: 改成二分查找
    for(int i = 0; pFunctionsTable[i].name != NULL; i++)
    {
      const INTRINSIC_FUNC& test_func = pFunctionsTable[i];
      if(rstrFunctionName == test_func.name)
      {
        if(sArgumentsTypeList.size() == test_func.count)
        {
          size_t n = 0;
          TYPEDESC::CPtrArray sExtendArgumentTypes;
          if(_CL_NOT_(ExtendParamDimension(sExtendArgumentTypes, test_func, sArgumentsTypeList))) // 扩展相同id的参数为相同维度
          {
            continue;
          }

          auto iter_type = sExtendArgumentTypes.begin();
          auto iter_expr = sExprList.begin();
          for(; n < test_func.count; n++, ++iter_type, ++iter_expr)
          {
            const TYPEDESC* pTypeDesc = *iter_type; // InferType(sNameSet, *it);
            if(pTypeDesc)
            {
              ASSERT(test_func.type > INTRINSIC_FUNC::RetType_Last);
              if(test_func.type == INTRINSIC_FUNC::RetType_Scaler0) {
                pRetType = pTypeDesc;
              }
              else if(//test_func.type == INTRINSIC_FUNC::RetType_FromName ||
                test_func.type == INTRINSIC_FUNC::RetType_Bool/* ||
                test_func.type == INTRINSIC_FUNC::RetType_Float4*/)
              {
              }
              else if(test_func.type == n) { // RetType_Argument0
                pRetType = pTypeDesc;
              }
              else if(test_func.type < 0 || n >= test_func.count) {
                CLBREAK;
              }

              if(TEST_FLAG(test_func.params[n], 8)) // out 修饰
              {
                // FIXME: 如果没有重载或者有重载并且形参数唯一匹配,才输出这条错误消息
                if(iter_expr->IsNode()) {
                  //error C2664: “UVShader::sincos”: 不能将参数 2 从“float”转换为“float &”
                  clStringW strFunc = test_func.name;
                  GetLogger()->OutputErrorW(iter_expr->pNode->GetAnyTokenAPB(),
                    UVS_EXPORT_TEXT(2664, "“%s”: 参数 %d 不能使用“out”修饰"),
                    strFunc.CStr(), n); // TODO: 没有testcase
                  return FALSE;
                }
              }

              if(TryTypeCasting(vctx.name_ctx, (GXDWORD)test_func.params[n], pTypeDesc, iter_expr->GetFrontToken())) {
                continue;
              }
              break;
            }
            else {
              return FALSE;
            }
          } // for

          if(n == test_func.count) {
            if(test_func.type == INTRINSIC_FUNC::RetType_Scaler0) {
              vctx.pType = vctx.name_ctx.GetType(pRetType->pDesc->component_type);
            }
            else if(test_func.type == INTRINSIC_FUNC::RetType_Bool) {
              vctx.pType = vctx.name_ctx.GetType(STR_BOOL);
            }
            //else if(test_func.type == INTRINSIC_FUNC::RetType_Float4) {
            //  vctx.pType = vctx.name_ctx.GetType(STR_FLOAT4);
            //}
            else if(test_func.type >= 0 && test_func.type < (int)sExtendArgumentTypes.size()) {
              auto iter_type = sExtendArgumentTypes.begin();
              int index = 0;
              while(index < (test_func.type - INTRINSIC_FUNC::RetType_Argument0)) {
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
            //return vctx.pType; //sNameSet.GetType(s_wildcard_functions[i].type);
          }
        }
      }
    }

    return TRUE;
  }

  GXBOOL CodeParser::ExtendParamDimension(TYPEDESC::CPtrArray& aExtendArgumentTypes, const INTRINSIC_FUNC& test_func, const TYPEDESC::CPtrList& sArgumentsTypeList)
  {
    // 数学参数默认1维开始，相同TempID的参数维度会进行扩展
    // 1维时，如果后面参数是n维（n>1）则替换维n维
    // n维（n>1）时，如果后面参数是m维（m>1）则不能继续匹配
    const TYPEDESC* aTypes[8] = { NULL }; // 索引是TempID
    if(sArgumentsTypeList.size() == 1) {
      aExtendArgumentTypes.push_back(sArgumentsTypeList.front());
      return TRUE;
    }

    aExtendArgumentTypes.reserve(sArgumentsTypeList.size());
    auto iter_type = sArgumentsTypeList.begin();
    size_t n = 0;

    // 按照TempID分组扩展数学参数维度
    for(; iter_type != sArgumentsTypeList.end(); ++iter_type, n++)
    {
      if(IS_SCALER_CATE(*iter_type) || IS_VECMAT_CATE(*iter_type))
      {
        const TYPEDESC*& pTypeInTab = aTypes[test_func.GetTemplateID(n)];
        if(pTypeInTab == NULL || pTypeInTab->CountOf() == 1)
        {
          pTypeInTab = *iter_type;
        }
        else {
          size_t pd = (*iter_type)->CountOf();
          if(pd != 1 && pd != pTypeInTab->CountOf()) {
            return FALSE;
          }
        }
        aExtendArgumentTypes.push_back(pTypeInTab);
      }
      else {
        aExtendArgumentTypes.push_back(*iter_type);
      }
    }

    ASSERT(aExtendArgumentTypes.size() == sArgumentsTypeList.size());

    // 生成新的参数类型表, 改写维度较小的参数
    for(n  = 0; n < aExtendArgumentTypes.size(); n++)
    {
      if(IS_SCALER_CATE(aExtendArgumentTypes[n]) || IS_VECMAT_CATE(aExtendArgumentTypes[n]))
      {
        const TYPEDESC* pTypeInTab = aTypes[test_func.GetTemplateID(n)];
        if(pTypeInTab->CountOf() > aExtendArgumentTypes[n]->CountOf())
        {
          aExtendArgumentTypes[n] = pTypeInTab;
        }
      }
    }

    return TRUE;
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
      vctx.result = ValueResult_Failed;
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

    VALUE_CONTEXT::Array vctx_params;
    vctx_params.reserve(sExprList.size());
    //auto iter_vctx_param = vctx_param.begin();

    GXBOOL bNeedValue = vctx.bNeedValue; // 参数中如果不是常量，则整个参数表都不储存常量
    for(auto it = sExprList.begin(); it != sExprList.end(); ++it)
    {
      //vctx_param.ClearValue();
      vctx_params.push_back(vctx);
      VALUE_CONTEXT& vctx_param = vctx_params.back();

      vctx_param.bNeedValue = bNeedValue;
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

        // 这个池只用于Pre-component math数学结构, 参数列表的所有常量是首尾相接放在一个池里的
        vctx.pool.insert(vctx.pool.end(), vctx_param.pValue, vctx_param.pValue + vctx_param.count);
      }
    }

    if(bNeedValue && _CL_NOT_(sExprList.empty())) {
      // 必须等pool稳定之后才能确定指针位置
      vctx.UsePool();
    }
    else {
      vctx.bNeedValue = FALSE;
      vctx.ClearValueOnly();
      for(VALUE_CONTEXT& vc : vctx_params) {
        vc.bNeedValue = FALSE;
        vc.ClearValueOnly();
      }
    }

    FUNCDESC::CPtrList sUserFuncs;

    // 用户定义函数 - 严格匹配
    pRetType = InferUserFunctionType(sUserFuncs, vctx, sArgumentsTypeList, pFuncNode, 0);
    if(pRetType == ERROR_TYPEDESC) {
      return NULL;
    }
    else if(pRetType) {
      return pRetType;
    }

    RefString rstrFunctionName;
    vctx.name_ctx.TranslateType(rstrFunctionName, pFuncNode->Operand[0].pTokn);

    // mul()比较特殊
    if(rstrFunctionName == s_szMultiplicationFunc && sArgumentsTypeList.size() == 2)
    {
      pRetType = InferTypesOfMultiplication(sArgumentsTypeList.front(), sArgumentsTypeList.back());
      if(pRetType) {
        vctx.ClearValueOnly();
        vctx.result = ValueResult_OK;
        vctx.pType = pRetType;
        return pRetType;
      }
    }

    // 内置函数原型查找
    GXBOOL bError = FALSE;
    GXLPCSTR szRetType = InferBuildinFunction(rstrFunctionName, sArgumentsTypeList, &bError);
    if(bError) {
      vctx.ClearValue();
      vctx.result = ValueResult_Failed;      
      return vctx.pType; // 出错
    }
    else if(szRetType)
    {
      // 找到返回值，同时标记为无法计算出常量
      vctx.ClearValueOnly();
      vctx.result = ValueResult_OK;
      vctx.pType = vctx.name_ctx.GetType(szRetType);
      ASSERT(vctx.pType);
      return vctx.pType;
    }

    // 通配符形式的内部函数列表
    if(InferBuildinFunction_Wildcard(vctx, rstrFunctionName, sExprList, sArgumentsTypeList, vctx_params) == FALSE) {
      return NULL;
    }
    else if(vctx.pType) {
      return vctx.pType;
    }

    // 用户定义函数 - 宽松匹配
    pRetType = InferUserFunctionType(sUserFuncs, vctx, sArgumentsTypeList, pFuncNode, 1);
    if(pRetType == ERROR_TYPEDESC) {
      return NULL;
    }
    else if(pRetType) {
      return pRetType;
    }



    // 确切参数类型的函数列表
    const PERCOMPONENTMATH* pPreCompMath = FindPerComponentMathOperations(rstrFunctionName);

    if(pPreCompMath)
    {
      int nScalerCount = 0;
      for(auto it = sArgumentsTypeList.begin(); it != sArgumentsTypeList.end(); ++it)
      {
        const TYPEDESC* pTypeDesc = *it; // InferType(sNameSet, *it);
        nScalerCount += pTypeDesc->CountOf();
      }

      vctx.result = ValueResult_OK;
      if(pPreCompMath->scaler_count <= nScalerCount || nScalerCount == 1)
      {
        vctx.pType = vctx.name_ctx.GetType(pPreCompMath->name);
        ASSERT(vctx.pType); // FindPerComponentMathOperations 列表里放了一个不存在的内置类型
        const VALUE::Rank rank = vctx.TypeRank();
        ASSERT(VALUE::IsNumericRank(rank));

        if(vctx.IsNeedValue() && pPreCompMath->scaler_count > nScalerCount)
        {
          ASSERT(vctx.count == 1);
          VALUE value = *vctx.pValue;
          value.CastValueByRank(rank);
          vctx.pool.assign(pPreCompMath->scaler_count, value);
          vctx.UsePool();
        }
        else if(vctx.pool.empty() == FALSE)
        {
          const VALUE::Rank rank = vctx.TypeRank();

          // 截断
          if(pPreCompMath->scaler_count < nScalerCount) {
            vctx.pool.erase(vctx.pool.begin() + pPreCompMath->scaler_count, vctx.pool.end());
            vctx.count = vctx.pool.size();
          }

          ASSERT(vctx.pool.size() == pPreCompMath->scaler_count);
          std::for_each(vctx.pool.begin(), vctx.pool.end(), [rank](VALUE& value) {
            value.CastValueByRank(rank);
          });
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

    // 扩展支持：结构体初始化类型
    pRetType = InferConstructorsInStructType(vctx.name_ctx, sArgumentsTypeList, pFuncNode);
    if(pRetType) {
      vctx.ClearValueOnly();
      vctx.result = ValueResult_OK;
      vctx.pType = pRetType;
      return pRetType;
    }

    // TODO: 没有找到名字的提示找不到标识符, 找到名字但是参数不匹配的提示没有找到重载
    //  error C3861: “func”: 找不到标识符
    clStringW strW;
    clStringW strInfoW;
    {
      strInfoW.Append(_CLTEXT("\r\n\t函数调用：\r\n"));
      strInfoW.Append(_CLTEXT("\t\t")).Append(rstrFunctionName.GetPtr(), rstrFunctionName.GetLength()).Append('(');
      for(const TYPEDESC* pt : sArgumentsTypeList)
      {
        strInfoW.Append(pt->name.GetPtr(), pt->name.GetLength()).Append(',');
      }
      strInfoW.TrimRight(',');
      strInfoW.Append(_CLTEXT(")\r\n"));
    }

    if(_CL_NOT_(sUserFuncs.empty()))
    {
      strInfoW.Append(_CLTEXT("\t无法在下列原型中找到匹配项：\r\n"));
      clStringW strFunc;
      for(const FUNCDESC* fd : sUserFuncs)
      {        
        strInfoW.Append(_CLTEXT("\t\t")).Append(fd->ToString(strFunc)).Append(_CLTEXT("\r\n"));
      }
    }

    GetLogger()->OutputErrorW(*pFuncNode->Operand[0].pTokn, UVS_EXPORT_TEXT(3861, "“%s”: 找不到标识符%s"),
      pFuncNode->Operand[0].pTokn->ToString(strW).CStr(), strInfoW.CStr());
    vctx.result = ValueResult_3861;


    return NULL;
  }

  const TYPEDESC* CodeParser::InferConstructorsInStructType(const NameContext& sNameSet, const TYPEDESC::CPtrList& sArgumentsTypeList, const SYNTAXNODE* pFuncNode)
  {
    if(pFuncNode->Operand[0].IsToken() == FALSE) {
      return NULL;
    }

    const TOKEN* ptkFuncName = pFuncNode->Operand[0].pTokn;
    const TYPEDESC* pTypeDesc = sNameSet.GetType(*ptkFuncName);
    if(pTypeDesc == NULL || pTypeDesc->cate != TYPEDESC::TypeCate_Struct) {
      return NULL;
    }

    TYPEDESC::CPtrList sMemberTypeList;
    pTypeDesc->GetMemberTypeList(sMemberTypeList);
    if(sMemberTypeList.size() != sArgumentsTypeList.size()) {
      return NULL; // 参数数量不匹配，跳过
    }

    auto iter_MemberType = sMemberTypeList.begin();
    auto iter_ArgumentsType = sArgumentsTypeList.begin();
    for(; iter_MemberType != sMemberTypeList.end(); ++iter_MemberType, ++iter_ArgumentsType)
    {
      
      if(TryTypeCasting(*iter_MemberType, *iter_ArgumentsType, ptkFuncName) == FALSE) {
        return NULL;
      }
    }

    return pTypeDesc;
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
    return NULL;
  }
  
  ValueResult CodeParser::TokenToValue(VALUE_CONTEXT& vctx, const TOKEN* pToken) const
  {
    ASSERT(pToken->type > TOKEN::TokenType_FirstNumeric && pToken->type < TOKEN::TokenType_LastNumeric); // 外部保证
    
    VALUE val;
    VALUE::State s = val.set(*pToken);      

    if(TEST_FLAG(s, VALUE::State_ErrorMask) && DumpValueState(vctx.pLogger, s, pToken)) {
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
    CHECK_VALUE_CONTEXT;
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

    // 针对 stru.member[n] 这种情况的特殊用法，member使用pMemberCtx域，n使用name_ctx域
    const IDNFDESC* pVariDesc = vctx.pMemberCtx
      ? vctx.pMemberCtx->GetIdentifierDesc(pToken)
      : vctx.name_ctx.GetIdentifierDesc(pToken);

    if(pVariDesc == NULL)
    {
      // C2065: “m”: 未声明的标识符
      clStringW strW;
      vctx.pLogger->OutputErrorW(*pToken, UVS_EXPORT_TEXT2(2065, "“%s”: 未声明的标识符", vctx.pLogger), pToken->ToString(strW).CStr());
      vctx.result = ValueResult_2065;
      return NULL;
    }
    
    vctx.pType = pVariDesc->pType;
    if(vctx.bNeedValue) {
      if(vctx.pValue == NULL) // 第一个进入的函数
      {
        //const ValuePool* pPool = vctx.name_ctx.GetValuePool(pToken);
        const IDNFDESC* pIdnfDesc = vctx.name_ctx.GetIdentifierDesc(pToken);
        if(pIdnfDesc && _CL_NOT_(pIdnfDesc->pool.empty()))
        {
          vctx.result = ValueResult_OK;
          vctx.pValue = &pIdnfDesc->pool.front();
          vctx.count = vctx.pType->CountOf();
          ASSERT(vctx.count == pIdnfDesc->pool.size());
        }
        else
        {
          vctx.result = ValueResult_Variable;
        }
      }
      else
      {
        //ASSERT(vctx.count <= pVariDesc->nOffset + vctx.pType->CountOf());
        ASSERT(pVariDesc->nOffset + vctx.pType->CountOf() <= vctx.count);
        vctx.result = ValueResult_OK;
        vctx.pValue += pVariDesc->nOffset;
        vctx.count = vctx.pType->CountOf();
      }
    }
    else
    {
      vctx.result = ValueResult_OK;
    }

    return pVariDesc->pType;
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
      return InferSubscriptType(vctx, NULL, pNode);
    }
    else if(pNode->mode == SYNTAXNODE::MODE_TypeCast)
    {
      if(pNode->Operand[0].IsToken())
      {
        const TYPEDESC* pCastTypeDesc = vctx.name_ctx.GetType(*pNode->Operand[0].pTokn);
        const TYPEDESC* pSource = InferType(vctx, pNode->Operand[1]);
        if(TryReinterpretCasting(pCastTypeDesc, pSource, pNode->Operand[0].pTokn))
        {
          vctx.pType = pCastTypeDesc;
          return vctx.pType;
        }
        vctx.ClearValue(ValueResult_Failed);
      }
      else {
        PARSER_BREAK(pNode->Operand[0]);
      }
      return NULL;
    }
    else if(pNode->mode == SYNTAXNODE::MODE_InitList)
    {
      GetLogger()->OutputErrorW(pNode->Operand[0], UVS_EXPORT_TEXT(5058, "不能使用初始化列表"));
      vctx.result = ValueResult_Failed;
      return NULL;
    }
    else if(pNode->pOpcode)
    {
      if(*pNode->pOpcode == '.')
      {
        const TYPEDESC* pTypeDesc = InferMemberType(vctx, pNode);
        return pTypeDesc;
      }
      else if(pNode->mode == SYNTAXNODE::MODE_Opcode)
      {
        ASSERT(pNode->pOpcode); // 上面分支判断了，这里防止以后重构遗失这个条件

        // 一元，右操作数 || 一元，左操作数 || 二元，左或右操作数
        if((pNode->pOpcode->unary_mask == 0x01 && pNode->Operand[1].ptr == NULL) ||
          (pNode->pOpcode->unary_mask == 0x02 && pNode->Operand[0].ptr == NULL) ||
          (pNode->pOpcode->unary == 0 && (pNode->Operand[0].ptr == NULL || pNode->Operand[1].ptr == NULL)))
        {
          clStringW strW;
          pNode->pOpcode->ToString(strW);
          GetLogger()->OutputErrorW(pNode->pOpcode, UVS_EXPORT_TEXT(5046, "“%s”缺少必要的操作数"), strW.CStr());
          vctx.result = ValueResult_Failed;
          return NULL;
        }
      }
    }

    //const TYPEDESC* pTypeDesc[2] = {NULL, NULL};
    VALUE_CONTEXT v[2] = {vctx.name_ctx, vctx.name_ctx};
    v[0].pLogger = v[1].pLogger = vctx.pLogger;

    if(pNode->CompareOpcode('?'))
    {
      ASSERT(pNode->Operand[1].CompareAsNode(':')); // 这个似乎外部保证过了，这里不再检查
      InferType(v[0], pNode->Operand[0]);
      if(v[0].result != ValueResult_OK && v[0].result != ValueResult_Variable) {
        vctx.result = v[0].result;
        return NULL;
      }
      else if(v[0].pValue == NULL) { // 只推导类型而不计算值
        v[1].bNeedValue = FALSE;
        InferType(v[1], pNode->Operand[1]);
        vctx.CopyValue(v[1]);
        return vctx.pType;
      }
      else if(v[0].count == 1) { // 标量bool
        InferType(v[1], _CL_NOT_(v[0].pValue->IsZero())
          ? pNode->Operand[1].pNode->Operand[0]
          : pNode->Operand[1].pNode->Operand[1]);
        vctx.CopyValue(v[1]);
        return vctx.pType;
      }
      else { // 向量，矩阵
        vctx.CopyValue(v[0]); // “?”的计算结果放入vctx，使用后面MergeValueContext来组合计算结果
        return InferType(vctx, pNode->Operand[1].pNode);
      }
    }
    else if(pNode->pOpcode && pNode->pOpcode->unary)
    {
      ASSERT(
        *pNode->pOpcode == '~' || *pNode->pOpcode == '!' ||
        *pNode->pOpcode == '-' || *pNode->pOpcode == '+' ||
        *pNode->pOpcode == "--" || *pNode->pOpcode == "++");

      ASSERT(pNode->pOpcode->unary_mask == 0x01 || pNode->pOpcode->unary_mask == 0x02);

      VALUE_CONTEXT& vr = pNode->pOpcode->unary_mask == 0x01 ? v[1] : v[0];

      if(InferType(vr, pNode->Operand[&vr - v]) == NULL) {
        ASSERT(vr.pType == NULL);
        vctx.ClearValue(vr.result);
        return NULL;
      }
    }
    else
    {
      for(int i = 0; i < 2; i++)
      {
        if(InferType(v[i], pNode->Operand[i]) == NULL) {
          ASSERT(v[i].pType == NULL);
          vctx.result = v[i].result;
          return NULL;
        }
        PARSER_ASSERT(pNode->Operand[i].ptr != NULL, pNode->GetAnyTokenAPB());
      }
    }


    vctx.pType = InferTypeByOperator(pNode->pOpcode, v[0].pType, v[1].pType);
    if(vctx.pType) {
      vctx.result = ValueResult_OK;
      return vctx.pType;
    }
    MergeValueContext(vctx, pNode->pOpcode, v, &pNode->GetAnyTokenPAB());
    return vctx.pType;
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
    size_t index = 0;
    size_t nListDepth = 0;
    size_t remain = sMemberTypeList.size();

    VALUE_CONTEXT vctx(rInitList.GetNameContext());
    vctx.pLogger = GetLogger();

    const TYPEDESC* pExpandedType = rInitList.ExpandValue(vctx, nTopIndex, index);
    if(pExpandedType == NULL) {
      return NULL;
    }
    else if(IS_STRUCT_CATE(pExpandedType) && vctx.count == 0)
    {
      // [Doc\初始化列表]不能推导值的构造函数必须位于结构体开头并且小于结构体长度
      // 相同类型，或者元素数量小于等于结构类型的数学类型
      if(pRefType->IsSameType(pExpandedType) ||
        (IS_VECMAT_CATE(pRefType) && IS_VECMAT_CATE(pExpandedType) && pRefType->CountOf() >= pExpandedType->CountOf()))
      {
        const CInitList::ELEMENT* pElement = rInitList.Get();
        rInitList.DbgListAdd(pElement);

        nListDepth = rInitList.BeginList();
        rInitList.Step(nDimDepth, nListDepth);

        if(pRefType->CountOf() == pExpandedType->CountOf())
        {
          goto FINAL_FUNC;
        }
      }
      else if(pExpandedType->CountOf() < pRefType->CountOf()) {
        // 可能是包含类型，由下面的RearrangeInitList来验证
        // nothing
      }
      else {
        GetLogger()->OutputTypeCastFailed(rInitList.GetLocation(), _CLTEXT("初始化列表"), pRefType, pExpandedType);
        return NULL;
      }
    }
    vctx.ClearValue(); // Debug

    nListDepth = rInitList.BeginList();


    for(auto it = sMemberTypeList.begin(); it != sMemberTypeList.end(); ++it, --remain)
    {
      if(IS_STRUCT_CATE(*it) || (*it)->cate == TYPEDESC::TypeCate_MultiDim)
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

      // 标量
      vctx.pType = *it;
      const TYPEDESC* pResultTypeDesc = rInitList.CastToValuePool(vctx, pRefType, nTopIndex, index);
      if(pResultTypeDesc == NULL) { // 失败
        return NULL;
      }
      else if(IS_STRUCT_CATE(pResultTypeDesc)) {
        if(vctx.pValue == NULL) { // 没有推导出具体值
          //index += pRefType->CountOf();

          // [Doc\初始化列表]不能推导值的构造函数必须位于结构体开头并且小于结构体长度
          //if(it == sMemberTypeList.begin() && pRefType->CountOf() <= sMemberTypeList.size()) {
          //  rInitList.Step(nDimDepth, nListDepth);
          //  break;
          //}
          //else {
          GetLogger()->OutputErrorW(rInitList.GetLocation(), UVS_EXPORT_TEXT(2078, "初始值设定项太多"));
          break;
          //}
        }
        else { // 推导出具体值时，会构建一个假的堆栈，便于逐个取出替代值
          nListDepth++;
          index++;

          if(pRefType->CountOf() > remain) {
            GetLogger()->OutputErrorW(rInitList.GetLocation(), UVS_EXPORT_TEXT(2078, "初始值设定项太多"));
            return NULL;
          }
        }
      }
      else if(IS_SCALER_CATE(pResultTypeDesc)) {
        index++;
      }
      else {
        CLBREAK; // 意外的类型
      }

      vctx.ClearValue();
      if(rInitList.Step(nDimDepth, nListDepth) == FALSE) {
        break;
      }
    }

FINAL_FUNC:
    if((rInitList.Depth() >= nDimDepth || rInitList.Depth() > nListDepth) && rInitList.IsEnd() == FALSE || index > pRefType->CountOf())
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

  const TYPEDESC* CodeParser::InferInitList_LinearScalerArray(size_t nTopIndex, const TYPEDESC* pRefType, CInitList& rInitList, size_t nDimDepth)
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
      //CInitList::Result result = rInitList.CastToValuePool(pRefType->pElementType, nTopIndex, index);
      //if(result != CInitList::Result_Ok) {
      //  GetLogger()->OutputErrorW(rInitList.GetLocation(), UVS_EXPORT_TEXT(2077, "设定项不能用于标量数组"));
      //  return NULL;
      //}
      VALUE_CONTEXT vctx(rInitList.GetNameContext());
      vctx.pLogger = GetLogger();
      vctx.pType = pRefType;
      const TYPEDESC* pResultTypeDesc = rInitList.CastToValuePool(vctx, pRefType->pElementType, nTopIndex, index);
      if(pResultTypeDesc == NULL || IS_SCALER_CATE(pResultTypeDesc) == FALSE) {
        GetLogger()->OutputErrorW(rInitList.GetLocation(), UVS_EXPORT_TEXT(2077, "设定项不能用于标量数组"));
        return NULL;
      }

      index++;
      if(rInitList.Step(nDimDepth, nListDepth) == FALSE) {
        break;
      }
    }


    if((rInitList.Depth() >= nDimDepth || rInitList.Depth() > nListDepth) && rInitList.IsEnd() == FALSE)
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
      str.Replace(1, len, pRefType->name.GetPtr(), pRefType->name.GetLength());
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
      //CInitList::Result result = rInitList.CastToValuePool(pRefType, nTopIndex, index);
      //if(result != CInitList::Result_Ok) {
      //  return NULL;
      //}
      VALUE_CONTEXT vctx(rInitList.GetNameContext());
      vctx.pLogger = GetLogger();
      vctx.pType = pRefType;

      const TYPEDESC* pResultTypeDesc = rInitList.CastToValuePool(vctx, pRefType, nTopIndex, index);
      if(IS_SCALER_CATE(pResultTypeDesc) == FALSE) {
        GetLogger()->OutputErrorW(rInitList.GetLocation(), UVS_EXPORT_TEXT(2077, "设定项不能用于标量数组"));
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
      return InferInitList_LinearScalerArray(nTopIndex, pRefType, rInitList, nDimDepth);
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
        rInitList.DbgGetString().Replace(1, len, pRefType->name.GetPtr(), pRefType->name.GetLength());
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
      if(il.IsConstantValues()) {
        pValuePool->assign(pValues, pValues + copy_count);
      }
      else {
        ASSERT(pValuePool->empty());
      }
    }
    SAFE_DELETE_ARRAY(pValues);

    clStringA strRearrange;
    if(nErrorCount == DbgErrorCount()) {
      pInitListGlob->pNode = il.GetRearrangedList(); // 用整理的初始化列表代替原来的列表
      DbgDumpSyntaxTree(NULL, pInitListGlob->pNode, 0, &strRearrange, 1);
    }
    else {
      strRearrange = "[ERROR]";
    }
    m_aDbgExpressionOperStack.push_back(strRearrange);
    m_aDbgExpressionOperStack.push_back(il.DbgGetString());
    return pTypeDesc;
  }

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
        InferSubscriptType(vctx, NULL, pChildNode);
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
        ASSERT(vctx.pType == NULL);
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
        if(pMemberContext || pTypeDesc->pDesc->lpDotOverride)
        {
          //ASSERT(vctx.bNeedValue == FALSE); // 断在这里就是没实现计算值的功能
          VALUE_CONTEXT vctx_subscript(vctx);
          vctx_subscript.pValue = vctx.pValue;
          vctx_subscript.count = vctx.count;
          vctx_subscript.pMemberCtx = pMemberContext;
          pTypeDesc = InferSubscriptType(vctx_subscript, pTypeDesc, pMemberNode);
          if(vctx_subscript.result != ValueResult_OK && vctx_subscript.result != ValueResult_Variable) {
            vctx.result = vctx_subscript.result;
            vctx.pType = NULL;
            return vctx.pType;
          }

          if(vctx_subscript.pool.empty()) {
            vctx.pValue = vctx_subscript.pValue;
          }
          else {
            ASSERT(vctx_subscript.pValue == &vctx_subscript.pool.front());
            vctx.pool = vctx_subscript.pool;
            vctx.pValue = &vctx.pool.front();
          }
          vctx.count = vctx_subscript.count;
          vctx.pType = vctx_subscript.pType;
          return vctx.pType;
        }
      }
#if 1
      // “.length()”求数组长度
      else if(pMemberNode->mode == SYNTAXNODE::MODE_FunctionCall)
      {
        if(pMemberNode->Operand[0].CompareAsToken(s_szLengthFunc) && pMemberNode->Operand[1].ptr == NULL)
        {
          ASSERT(pTypeDesc);
          if(pTypeDesc->sDimensions.empty()) {
            vctx.pLogger->OutputErrorW(pNode->Operand[0], UVS_EXPORT_TEXT(2109, "下标要求数组类型"));
            return vctx.ClearValue(ValueResult_Failed).pType;
          }
#ifdef _DEBUG
          const TOKEN* pMemberToken = pNode->Operand[0].GetBackToken();
          TRACE("length of(%s) = %d\n", pMemberToken->ToString().CStr(), pTypeDesc->sDimensions.back());
#endif
          vctx.pType = m_GlobalCtx.GetType(STR_INT);
          vctx.pool.assign(1, VALUE());
          vctx.pool.front().set(vctx.TypeRank(), &pTypeDesc->sDimensions.back());
          vctx.UsePool();
          return vctx.pType;
        }
      }
#endif
      else {
      }

      clStringW strW;
      const TOKEN* pToken = pNode->Operand[1].GetFrontToken();
      GetLogger()->OutputErrorW(*pToken, UVS_EXPORT_TEXT(5041, "结构体不支持的操作: “%s”"), pToken->ToString(strW).CStr());
      return vctx.ClearValue(ValueResult_Failed).pType;
    }
    else if(pNode->Operand[1].IsToken())
    {
      const NameContext* pMemberContext = vctx.name_ctx.GetStructContext(pTypeDesc->name);
      if(pMemberContext)
      {
        ASSERT(vctx.result == ValueResult_OK || vctx.result == ValueResult_Variable);
        const IDNFDESC* pVariDesc = pMemberContext->GetIdentifierDesc(pNode->Operand[1].pTokn);

        if(pVariDesc)
        {
          vctx.pType = pVariDesc->pType;
          if(vctx.IsNeedValue())
          {
            ASSERT(vctx.pValue && vctx.pType);
            vctx.pValue += pVariDesc->nOffset;
            ASSERT(vctx.count >= vctx.pType->CountOf());
            vctx.count = vctx.pType->CountOf();
          }
          return vctx.pType;
        }
        // 转到函数尾输出错误信息
      }
      else if(pTypeDesc->pDesc && pTypeDesc->pDesc->lpDotOverride)
      {
        DOTOPERATOR_RESULT sDotOperator;
        if(pTypeDesc->pDesc->lpDotOverride(pTypeDesc->pDesc, &sDotOperator, pNode->Operand[1].pTokn))
        {
          vctx.pType = m_GlobalCtx.GetType(sDotOperator.strType.CStr());
          vctx.GenerateMathComponentValue(sDotOperator);
          return vctx.pType;
        }
      }
    }

    clStringW str1, str2;
    GetLogger()->OutputErrorW(*pNode->Operand[1].pTokn, UVS_EXPORT_TEXT(ERR_IS_NOT_MEMBER, "“%s”: 不是“%s”的成员"),
      pNode->Operand[1].pTokn->ToString(str1).CStr(), pTypeDesc->name.ToString(str2).CStr());
    pTypeDesc = NULL;
    vctx.ClearValue(ValueResult_NotStructMember);

    return pTypeDesc;
  }

  const TYPEDESC* CodeParser::InferSubscriptType(VALUE_CONTEXT& vctx, const TYPEDESC* pDotOverrideType, const SYNTAXNODE* pNode)
  {
    CHECK_VALUE_CONTEXT;
    ASSERT(pNode->mode == SYNTAXNODE::MODE_Subscript);

    const TYPEDESC* pTypeDesc = NULL;
    if(pNode->Operand[0].ptr)
    {
      if(vctx.pMemberCtx == NULL && pDotOverrideType != NULL)
      {
        DOTOPERATOR_RESULT sDotOperator;
        if(_CL_NOT_(pDotOverrideType->pDesc->lpDotOverride(pDotOverrideType->pDesc, &sDotOperator, pNode->Operand[0].pTokn)))
        {
          vctx.ClearValue(ValueResult_NotStructMember);
          return NULL;
        }

        vctx.pType = pTypeDesc = m_GlobalCtx.GetType(sDotOperator.strType.CStr());
        vctx.GenerateMathComponentValue(sDotOperator);
      }
      else {
        pTypeDesc = InferType(vctx, pNode->Operand[0]);
      }

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
    CHECK_VALUE_CONTEXT;
    VALUE_CONTEXT vctx_subscript(vctx);
    const TYPEDESC* pSubscriptType = InferType(vctx_subscript, pNode->Operand[1]);

    if((vctx_subscript.result != ValueResult_OK && vctx_subscript.result != ValueResult_Variable) ||
      vctx_subscript.pType->cate != TYPEDESC::TypeCate_IntegerScaler)
    {
      GetLogger()->OutputErrorW(pNode->GetAnyTokenAB(), UVS_EXPORT_TEXT(2058, "常量表达式不是整型")); // TODO: 定位不准
      return vctx.ClearValue(ValueResult_Failed).pType;
    }

    if(vctx.pType->cate == TYPEDESC::TypeCate_MultiDim)
    {
      if(vctx.IsNeedValue() && vctx_subscript.pValue)
      {
        ASSERT(vctx_subscript.count == 1);
        ASSERT(_CL_NOT_(vctx_subscript.pValue->IsNegative())); // TODO: 输出错误提示不能为负值
        const size_t element_count = vctx.pType->pElementType->CountOf();
        PARSER_ASSERT(vctx_subscript.pValue->uValue64 * element_count < vctx.count, pNode->GetAnyTokenAPB());
        vctx.pValue += vctx_subscript.pValue->uValue64 * element_count;
        vctx.pType = vctx.pType->pElementType;
        ASSERT(vctx.count >= element_count)
        vctx.count = element_count;
      }
      else
      {
        vctx.ClearValueOnly();
        vctx.pType = vctx.pType->pElementType;
        vctx.result = ValueResult_Variable;
      }
      return vctx.pType;
    }
    else if(IS_VECMAT_CATE(vctx.pType) && vctx.pType->pDesc && vctx.pType->pDesc->lpSubscript)
    {
      vctx.pType = vctx.pType->pDesc->lpSubscript(vctx, vctx_subscript.pValue);
      if(vctx.result == ValueResult_SubscriptOutOfRange) {
        GetLogger()->OutputErrorW(pNode->Operand[1], UVS_EXPORT_TEXT(5056, "下标超出定义范围"));
      }
      else {
        ASSERT(vctx.pType);
      }
      return vctx.pType;
    }

    GetLogger()->OutputErrorW(*pNode->Operand[0].GetFrontToken(), UVS_EXPORT_TEXT(2109, "下标要求数组类型"));
    return vctx.ClearValue(ValueResult_Failed).pType;
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

  const TYPEDESC* CodeParser::InferDifferentTypesOfCalculations(const TOKEN* pToken, const TYPEDESC* pFirst, const TYPEDESC* pSecond) const
  {
    ASSERT(pToken); // 暂时不支持
    if(*pToken == '*')
    {
      return InferTypesOfMultiplication(pFirst, pSecond);
    }

    CLBREAK;
    return NULL;
  }

  const TYPEDESC* CodeParser::InferTypesOfMultiplication(const TYPEDESC* pFirst, const TYPEDESC* pSecond) const
  {
    // 推导乘法类型，支持矩阵与向量乘法
    // 注意，mul双标量乘法与dot等效，与“*”乘法不一致

    //Version   |Name   |Purpose  |Template Type  |Component Type   |Size
    //1
    //          |x      |in       |scalar         |float, int       |1
    //          |y      |in       |scalar         |same as input x  |1
    //          |ret    |out      |scalar         |same as input x  |1
    //2
    //          |x      |in       |scalar         |float, int       |1
    //          |y      |in       |vector         |float, int       |any
    //          |ret    |out      |vector         |float, int       |same dimension(s) as input y
    //3
    //          |x      |in       |scalar         |float, int       |1
    //          |y      |in       |matrix         |float, int       |any
    //          |ret    |out      |matrix         |same as input y  |same dimension(s) as input y
    //4
    //          |x      |in       |vector         |float, int       |any
    //          |y      |in       |scalar         |float, int       |1
    //          |ret    |out      |vector         |float, int       |same dimension(s) as input x
    //5
    //          |x      |in       |vector         |float, int       |any
    //          |y      |in       |vector         |float, int       |same dimension(s) as input x
    //          |ret    |out      |scalar         |float, int       |1
    //6
    //          |x      |in       |vector         |float, int       |any
    //          |y      |in       |matrix         |float, int       |rows = same dimension(s) as input x, columns = any
    //          |ret    |out      |vector         |float, int       |same dimension(s) as input y columns
    //7
    //          |x      |in       |matrix         |float, int       |any
    //          |y      |in       |scalar         |float, int       |1
    //          |ret    |out      |matrix         |float, int       |same dimension(s) as input x
    //8
    //          |x      |in       |matrix         |float, int       |any
    //          |y      |in       |vector         |float, int       |number of columns in input x
    //          |ret    |out      |vector         |float, int       |number of rows in input x
    //9
    //          |x      |in       |matrix         |float, int       |any
    //          |y      |in       |matrix         |float, int       |rows = number of columns in input x
    //          |ret    |out      |matrix         |float, int       |rows = number of rows in input x, columns = number of columns in input y
    //

    const TYPEDESC* pResultType = NULL;
    clStringA strTypeName;
    int R1, C1;
    int R2, C2;

    if(IS_SCALER_CATE(pFirst))
    {
      if(IS_SCALER_CATE(pSecond))
      {
        if(pFirst->name == pSecond->name) {
          return pFirst;
        }
      }
      else if(pSecond->IsVector() || pSecond->IsMatrix())
      {
        if(pFirst->name == pSecond->pElementType->name) {
          return pSecond;
        }
      }
    }
    else if(pFirst->IsVector())
    {
      if(IS_SCALER_CATE(pSecond))
      {
        if(pFirst->pElementType->name == pSecond->name) {
          return pFirst;
        }
      }
      else if(pSecond->IsVector())
      {
        if(pFirst->name == pSecond->name) {
          return pFirst;
        }
      }
      else if(pSecond->IsMatrix())
      {
        if(IsComponent(pFirst, pSecond, NULL)) {
          pSecond->Resolve(R2, C2);
          //strTypeName.Format("%s%d", pFirst->pElementType->name, C2);
          strTypeName.Append(pFirst->pElementType->name.GetPtr(), pFirst->pElementType->name.GetLength()).AppendInteger32(C2);
          pResultType = m_GlobalCtx.GetType(strTypeName.CStr());
          ASSERT(pResultType);
        }
      }
    }
    else if(pFirst->IsMatrix())
    {
      if(IS_SCALER_CATE(pSecond))
      {
        if(pFirst->pElementType->name == pSecond->name) {
          return pFirst;
        }
      }
      else if(pSecond->IsVector())
      {
        if(IsComponent(NULL, pFirst, pSecond))
        {
          pFirst->Resolve(R1, C1);
          strTypeName.Append(pFirst->pElementType->name.GetPtr(), pFirst->pElementType->name.GetLength()).AppendInteger32(R1);
          pResultType = m_GlobalCtx.GetType(strTypeName.CStr());
          ASSERT(pResultType);
        }
      }
      else if(pSecond->IsMatrix() && (pFirst->pElementType == pSecond->pElementType))
      {
        pFirst->Resolve(R1, C1);
        pSecond->Resolve(R2, C2);
        if(R2 == C1) {
          // R1 x C2
          strTypeName.Append(pFirst->pElementType->name.GetPtr(), pFirst->pElementType->name.GetLength());
          strTypeName.AppendFormat("%dx%d", R1, C2);
          pResultType = m_GlobalCtx.GetType(strTypeName.CStr());
          ASSERT(pResultType);
        }
      }
    }

    return pResultType;
  }

  const TYPEDESC* CodeParser::InferRightValueType(NameContext& sNameSet, const GLOB& right_glob, const TOKEN* ptkLocation)
  {
    if(right_glob.ptr == NULL)
    {
      GetLogger()->OutputMissingSemicolon(ptkLocation);
      return NULL;
    }

    // 不能使用初始化列表
    ASSERT(_CL_NOT_(right_glob.IsNode() && right_glob.pNode->mode == SYNTAXNODE::MODE_InitList));

    const size_t nErrorCount = DbgErrorCount();
    VALUE_CONTEXT vctx(sNameSet);
    vctx.pLogger = GetLogger();
    const TYPEDESC* pRightType = InferType(vctx, right_glob);
    if(pRightType == NULL) {
      ASSERT(DbgErrorCount() > nErrorCount); // InferType 内部应该输出错误
    }
    return pRightType;
  }
  
  GXBOOL CodeParser::CompareScaler(const RefString& rstrTypeFrom, GXLPCSTR szTypeTo)
  {
    return (
      ((rstrTypeFrom == STR_FLOAT) || (rstrTypeFrom == STR_HALF) || (rstrTypeFrom == STR_DOUBLE)) &&
      (clstd::strcmpT(szTypeTo, STR_FLOAT) || clstd::strcmpT(szTypeTo, STR_HALF) || clstd::strcmpT(szTypeTo, STR_DOUBLE)) ) ||
      ( ((rstrTypeFrom == STR_UINT) || (rstrTypeFrom == STR_INT)) && (clstd::strcmpT(szTypeTo, STR_UINT) || clstd::strcmpT(szTypeTo, STR_INT)) );
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

    // [整数限定操作符]
    // 检查非法操作数类型，比如对浮点数求模
    if(*pOperator == '%' || *pOperator == '&' || *pOperator == '|' || *pOperator == '~' ||
      *pOperator == '^' || *pOperator == '!' || *pOperator == "<<" || *pOperator == ">>")
    {
      //C2296 : “ << ” : 非法，左操作数包含“float”类型
      //C2297 : “ << ” : 非法，右操作数包含“float”类型
      clStringW strOperatorW;
      clStringW strTypeW;
      GXBOOL bret = TRUE;

      for(int i = 0; i < 2; i++)
      {
        if(pAB[i].pType && (pAB[i].pType->cate == TYPEDESC::TypeCate_FloatScaler ||
          (IS_VECMAT_CATE(pAB[i].pType) && pAB[i].pType->pElementType->cate == TYPEDESC::TypeCate_FloatScaler)))
        {
          // 注意：错误消息这么写是为了便于文本抽取
          GetLogger()->OutputErrorW(*pLocation,
            i == 0 ? UVS_EXPORT_TEXT(2296, "“%s” : 非法，左操作数包含“%s”类型")
                   : UVS_EXPORT_TEXT(2297, "“%s” : 非法，右操作数包含“%s”类型"),
            pOperator->ToString(strOperatorW).CStr(), pAB[i].pType->name.ToString(strTypeW).CStr());
          bret = FALSE;
        }
      }

      if(_CL_NOT_(bret)) {
        vctx.ClearValue(ValueResult_Failed);
        return bret;
      }
    }

    if(pOperator->unary) // TODO: 改成 else if???
    {
      if(pOperator->unary_mask == 0x01) {
        vctx.pType = pAB[1].pType;
      }
      else if(pOperator->unary_mask == 0x02) {
        vctx.pType = pAB[0].pType;
      }
    }
    else if(*pOperator == '>' || *pOperator == '<' || *pOperator == "!=" ||
      *pOperator == ">=" || *pOperator == "<=" || *pOperator == "==" )
    {
      // [Doc\条件表达式]向量比较返回类型是bool向量
      if((IS_SCALER_CATE(pAB[0].pType) && IS_SCALER_CATE(pAB[1].pType)) ||
        (IS_SAMPLER_CATE(pAB[0].pType) && IS_SAMPLER_CATE(pAB[1].pType))) {
        vctx.pType = vctx.name_ctx.GetType("bool");
      }
      else if(IS_VECMAT_CATE(pAB[0].pType) && IS_VECMAT_CATE(pAB[1].pType))
      {
        int R0, C0, R1, C1;
        pAB[0].pType->Resolve(R0, C0);
        pAB[1].pType->Resolve(R1, C1);

        if(R0 != R1 || C0 != C1) {
          vctx.pLogger->OutputTypeCastFailed(pLocation, pOperator, pAB[0].pType, pAB[1].pType);
          vctx.ClearValue(ValueResult_Failed);
          return FALSE;
        }

        clStringA strTypeName;
        if(C0 == 0) {
          strTypeName.Format("bool%d", R0);
        }
        else {
          strTypeName.Format("bool%dx%d", R0, C0);
        }

        vctx.pType = vctx.name_ctx.GetType(strTypeName.CStr());
        ASSERT(vctx.pType);
      }
      else if(pAB[0].pType->cate == TYPEDESC::TypeCate_Struct || pAB[1].pType->cate == TYPEDESC::TypeCate_Struct) {
        clStringW strType0, strType1;
        vctx.pLogger->OutputErrorW(pLocation, UVS_EXPORT_TEXT(5077, "无法将结构体直接用于比较：“%s”与“%s”"),
          pAB[0].pType->name.ToString(strType0).CStr(), pAB[1].pType->name.ToString(strType0).CStr());
        vctx.ClearValue(ValueResult_Failed);
        return FALSE;
      }
      else
      {
        vctx.pLogger->OutputTypeCastFailed(pLocation, pOperator, pAB[0].pType, pAB[1].pType);
        vctx.ClearValue(ValueResult_Failed);
        return FALSE;
      }
    }
    else if(IS_SCALER_CATE(pAB[0].pType) && IS_SCALER_CATE(pAB[1].pType))
    {
      vctx.pType = (pAB[0].TypeRank() >= pAB[1].TypeRank()) ? pAB[0].pType : pAB[1].pType;
      ASSERT(pAB[0].pType->CountOf() == pAB[1].pType->CountOf());
    }
    else if(IS_SCALER_CATE(pAB[0].pType) && IS_VECMAT_CATE(pAB[1].pType))
    {
      if(vctx.bNeedValue && pAB[0].IsNeedValue() && pAB[1].IsNeedValue()) {
        vctx.pType = pAB[0].CastUpward(pAB[1].pType);
        ASSERT(pAB[0].pType->CountOf() == pAB[1].pType->CountOf());
      }
      else {
        vctx.pType = vctx.MergeType(pAB[0].pType, pAB[1].pType);
      }
    }
    else if(IS_VECMAT_CATE(pAB[0].pType) && IS_SCALER_CATE(pAB[1].pType))
    {
      if(vctx.bNeedValue && pAB[0].IsNeedValue() && pAB[1].IsNeedValue()) {
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
      pAB[0].pType->IsVector() && pAB[1].pType->IsVector())
    {
      if(vctx.bNeedValue && pAB[0].IsNeedValue() && pAB[1].IsNeedValue())
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
    else if(pAB[0].pType->IsVector() && pAB[1].pType->IsMatrix() && IsComponent(pAB[0].pType, pAB[1].pType, NULL))
    {
      vctx.pType = InferTypesOfMultiplication(pAB[0].pType, pAB[1].pType);
      if(vctx.bNeedValue && pAB[0].IsNeedValue() && pAB[1].IsNeedValue())
      {
        VALUE value[4];
        int R2, C2;
        VALUE::State state;
        pAB[1].pType->Resolve(R2, C2);
        vctx.pool.reserve(4);

        //                      C2
        // | x y z w | * | m00 m01 m02 |
        //               | m10 m11 m12 | R2
        //               | m20 m21 m22 |
        //               | m30 m31 m32 |
        for(int c = 0; c < C2; c++)
        {
          for(int r = 0; r < R2; r++) {
            state = value[r].Calculate("*", 1, pAB[0].pValue[r], pAB[1].pValue[r * C2 + c]);
            DUMP_STATE_IF_FAILED;
          }
          for(int r = 0; r < R2 - 1; r++) {
            state = value[0].Calculate("+", 1, value[r], value[r + 1]);
            DUMP_STATE_IF_FAILED;
          }
          vctx.result = ValueResult_OK;
          vctx.pool.push_back(value[0]);
          vctx.pool.back().CastValueByRank(vctx.TypeRank());
        }
        return TRUE;
      }
    }
    else if(pAB[0].pType->IsMatrix() && pAB[1].pType->IsVector() && IsComponent(NULL, pAB[0].pType, pAB[1].pType))
    {
      vctx.pType = InferTypesOfMultiplication(pAB[0].pType, pAB[1].pType);
      if(vctx.bNeedValue && pAB[0].IsNeedValue() && pAB[1].IsNeedValue())
      {
        VALUE value[4];
        int R1, C1;
        VALUE::State state;
        pAB[0].pType->Resolve(R1, C1);
        vctx.pool.reserve(4);

        //           C1
        //    | m00 m01 m02 |   | x |
        // R1 | m10 m11 m12 | * | y |
        //    | m20 m21 m22 |   | z |
        //    | m30 m31 m32 |   | w |
        for(int r = 0; r < R1; r++)
        {
          for(int c = 0; c < C1; c++) {
            state = value[c].Calculate("*", 1, pAB[0].pValue[r * C1 + c], pAB[1].pValue[r]);
            DUMP_STATE_IF_FAILED;
          }
          for(int c = 0; c < C1 - 1; c++) {
            state = value[0].Calculate("+", 1, value[c], value[c + 1]);
            DUMP_STATE_IF_FAILED;
          }
          vctx.result = ValueResult_OK;
          vctx.pool.push_back(value[0]);
          vctx.pool.back().CastValueByRank(vctx.TypeRank());
          return TRUE;
        }
      }
    }
    else
    {
      // int3 vs float3, cast type
      // float3 vs float4, error!
      vctx.pLogger->OutputTypeCastFailed(pLocation, pOperator, pAB[0].pType, pAB[1].pType);
      vctx.ClearValue(ValueResult_Failed);
      return FALSE;
    }


    if(vctx.bNeedValue)
    {
       if( // 含有变量而无法计算常量时的处理
         (pAB[0].result == ValueResult_Variable || pAB[0].pValue == NULL) ||
         (pAB[1].result == ValueResult_Variable || pAB[1].pValue == NULL) )
       {
         vctx.ClearValueOnly();
         vctx.result = ValueResult_Variable;
         return TRUE;
       }
       else if(*pOperator == ':') // ...?...:...三元操作的冒号
       {
         // 这里不可能是类型推导
         ASSERT(vctx.bNeedValue);

         // 这里vctx也有输出值
         ASSERT(vctx.pValue != NULL && vctx.count > 0);

         // 这里用的肯定是池
         ASSERT(_CL_NOT_(vctx.pool.empty()) && vctx.pool.size() >= vctx.count);
         ValuePool p00l; // 故意这么写的，好玩
         p00l.reserve(vctx.count);
         for(size_t i = 0; i < vctx.count; i++)
         {
           if(_CL_NOT_(vctx.pValue[i].IsZero())) {
             p00l.push_back(pAB[0].pValue[i]);
           }
           else {
             p00l.push_back(pAB[1].pValue[i]);
           }
         }
         vctx.result = ValueResult_OK;
         vctx.pool.assign(p00l.begin(), p00l.end());
         vctx.UsePool();
         return TRUE;
       }

       VALUE value;
       vctx.result = ValueResult_OK;
       vctx.pool.clear();
       const size_t count = pAB[0].pType->CountOf();
       for(size_t i = 0; i < count; i++)
       {
         VALUE::State state = value.Calculate(*pOperator, pAB[0].pValue[i], pAB[1].pValue[i]);
         DUMP_STATE_IF_FAILED;
         vctx.pool.push_back(value);
         vctx.pool.back().CastValueByRank(vctx.TypeRank());
       }

       vctx.UsePool();
       return TRUE;
    }

    // 不计算值
    vctx.ClearValueOnly();
    vctx.result = ValueResult_OK;
    return TRUE;
  }

  GXBOOL CodeParser::TryTypeCasting(const TYPEDESC* pTypeTo, const TYPEDESC* pTypeFrom, const TOKEN* pLocation, GXBOOL bFormalParam)
  {
    ASSERT(pTypeTo != NULL && pTypeFrom != NULL);

    if(IS_SCALER_CATE(pTypeTo) && IS_SCALER_CATE(pTypeFrom))
    {
      if(pTypeTo->pDesc->rank < pTypeFrom->pDesc->rank &&
        (pTypeTo->pDesc->rank != VALUE::Rank_Double && pTypeTo->pDesc->rank != VALUE::Rank_Float) &&
        (pTypeFrom->pDesc->rank != VALUE::Rank_Double && pTypeFrom->pDesc->rank != VALUE::Rank_Float)
        ) {
        clStringW strFrom, strTo;
        pTypeFrom->name.ToString(strFrom);
        pTypeTo->name.ToString(strTo);
        GetLogger()->OutputErrorW(*pLocation, UVS_EXPORT_TEXT(_WARNING(4244), "从“%s”转换到“%s”，可能丢失数据"), strFrom.CStr(), strTo.CStr());
      }
      return TRUE;
    }
    else if((pTypeTo->IsVector() && pTypeFrom->IsVector()) || (pTypeTo->IsMatrix() && pTypeFrom->IsMatrix()))
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
    else if(IS_VECMAT_CATE(pTypeTo) && IS_SCALER_CATE(pTypeFrom) && bFormalParam == FALSE) // [Doc\函数\参数] 在函数调用时不会使用隐式类型扩展（如float扩展为float3）
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

  GXBOOL CodeParser::TryReinterpretCasting(const TYPEDESC* pTypeTo, const TYPEDESC* pTypeFrom, const TOKEN* ptkLocation)
  {
    if(TryTypeCasting(pTypeTo, pTypeFrom, ptkLocation)) {
      return TRUE;
    }

    GetLogger()->OutputTypeCastFailed(ptkLocation, _CLTEXT("类型强制转换"), pTypeTo, pTypeFrom);
    return FALSE;
  }

  GXBOOL CodeParser::IsComponent(const TYPEDESC* pRowVector, const TYPEDESC* pMatrixDesc, const TYPEDESC* pColumnVector)
  {
    ASSERT(pRowVector != NULL || pColumnVector != NULL);
    ASSERT(pRowVector == NULL || pColumnVector == NULL);
    ASSERT(pRowVector == NULL || pRowVector->IsVector());
    ASSERT(pMatrixDesc->IsMatrix());
    ASSERT(pColumnVector == NULL || pColumnVector->IsVector());
    GXLPCSTR szScaler = NULL;
    int R, C;
    int RV, CV;
    szScaler = pMatrixDesc->Resolve(R, C);

    if(pRowVector) {
      pRowVector->Resolve(RV, CV);
      return RV == R && (pRowVector->pElementType == pMatrixDesc->pElementType);
    }

    pColumnVector->Resolve(RV, CV);
    return RV == C && (pMatrixDesc->pElementType == pColumnVector->pElementType);
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
    m_pReturnType = NULL;
    m_TypeMap.clear();
    m_FuncMap.clear();
    m_IdentifierMap.clear();
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
    if(m_aBasicType.empty() == FALSE) {
      return;
    }
    TYPEDESC td = { TYPEDESC::TypeCate_Empty, this };
    
    m_aBasicType.reserve(s_aIntrinsicType_len);
    // 内置结构体
    for(int i = 0; s_aIntrinsicType[i].name; i++)
    {
      // 需要预排序
      td.cate = static_cast<TYPEDESC::TypeCate>(s_aIntrinsicType[i].cate);
      td.name = s_aIntrinsicType[i].name;
      td.pDesc = &s_aIntrinsicType[i];
      td.pElementType = NULL;

#if 1
      if(td.cate == TYPEDESC::TypeCate_Vector || td.cate == TYPEDESC::TypeCate_Matrix)
      {
        ASSERT(td.name.BeginsWith(td.pDesc->component_type)); // 确保分量与类型名一致
        td.pElementType = GetIntrinsicType(td.pDesc->component_type);
        ASSERT(td.pElementType && td.pElementType->name == td.pDesc->component_type);
      }
#endif
      m_aBasicType.push_back(td);
    }
  }

  CLogger* NameContext::GetLogger() const
  {
    return m_pCodeParser->GetLogger();
  }

  GXBOOL NameContext::SetReturnType(const RefString& rstrTypeName)
  {
    ASSERT(GetReturnType() == NULL);
    m_pReturnType = GetType(rstrTypeName);
    return (m_pReturnType != NULL);
  }

  const TYPEDESC* NameContext::GetReturnType() const
  {
    return m_pReturnType ? m_pReturnType : 
      (m_pParent ? m_pParent->GetReturnType() : NULL);
  }

#ifdef ENABLE_SYNTAX_VERIFY
  VALUE::State NameContext::CalculateConstantValue(State& eLastState, VALUE& value_out, const GLOB* pGlob) const
  {
    if(pGlob->IsNode())
    {
      return Calculate(value_out, pGlob->pNode);
    }
    else if(pGlob->IsToken())
    {
      if(pGlob->pTokn->IsIdentifier())
      {
        if(const IDNFDESC* pIdnf = GetIdentifierDesc(pGlob->pTokn))
        {
          if(_CL_NOT_(pIdnf->pool.empty()))
          {
            ASSERT(pIdnf->pool.size() == 1);
            value_out = pIdnf->pool.front();
            return VALUE::State_OK;
          }
        }
      }
      else if(pGlob->pTokn->IsNumeric())
      {
        return value_out.set(*pGlob->pTokn);
      }

      // C2057: 应输入常量表达式
      eLastState = State_RequireConstantExpression;
      return VALUE::State_SyntaxError;
    }
    CLBREAK;
    return VALUE::State_SyntaxError;
  }

  VALUE::State NameContext::Calculate(VALUE& value_out, const SYNTAXNODE* pNode) const
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
          VALUE_CONTEXT vctx(*this);
          vctx.pLogger = GetLogger();
          // 参考下面的[数组长度计算]
          if(m_pCodeParser->InferFunctionReturnedType(vctx, pNode))
          {
            if(vctx.pValue == NULL) {
              value_out.SetZero();
              return VALUE::State_Call;
            }
            // FIXEME: 检查vctx.pool.size() == 1
            value_out = *vctx.pValue;
            return VALUE::State_OK;
          }
          value_out.SetZero();
          return VALUE::State_SyntaxError;
        }

        if(pNode->Operand[1].IsNode())
        {
          if((s = Calculate(value_out, pNode->Operand[1].pNode)) != VALUE::State_OK) {
            return s;
          }

          s = value_out.CastValueByRank(target_rank);
        }
        else if(pNode->Operand[1].IsToken())
        {
          //GetVariableValue(value_out, pNode->Operand[1].pTokn);
          const IDNFDESC* pIdnfDesc = GetIdentifierDesc(pNode->Operand[1].pTokn);

          if(pIdnfDesc == NULL) {
            return VALUE::State_BadIdentifier;
          }
          else if(pIdnfDesc->pool.empty()) {
            return VALUE::State_Variable;
          }
          else {
            ASSERT(pIdnfDesc->pool.size() == 1);
            value_out = pIdnfDesc->pool.front();
          }

          s = value_out.CastValueByRank(target_rank);
        }
        else {
          // int a = int(); 形式
          value_out.rank = VALUE::Rank_Undefined;
          return VALUE::State_Variable;
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
        vctx.pLogger = GetLogger();
        const TYPEDESC* pTypeDesc = m_pCodeParser->InferMemberType(vctx, pNode);

        // [数组长度计算]应该改为：
        // if(pTypeDesc)
        // {
        //   if(IS_SCALER_CATE(pTypeDesc))
        //   {
        //     if(vctx.pValue)
        //     {
        //       // 下标常量
        //     }
        //     else
        //     {
        //       // 下标变量
        //     }
        //   }
        //   else
        //   {
        //     // 无法转换类型到标量
        //   }
        // }
        // else
        // {
        //   // 出错
        // }

        if(pTypeDesc && IS_SCALER_CATE(pTypeDesc))
        {
          if(vctx.pValue && vctx.count == 1)
          {
            value_out = *vctx.pValue;
            return VALUE::State_OK;
          }
          value_out.rank = VALUE::Rank_Undefined;
          return VALUE::State_Variable;
        }
        value_out.SetZero();
        return VALUE::State_SyntaxError;
      }
    }
    else if(pNode->mode == SYNTAXNODE::MODE_Subscript)
    {
      VALUE_CONTEXT vctx(*this);
      vctx.pLogger = GetLogger();

      m_pCodeParser->InferSubscriptType(vctx, NULL, pNode);
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
        s = Calculate(p[i], pNode->Operand[i].pNode);
      }
      else if(pNode->Operand[i].IsToken()) {
        if(pNode->Operand[i].pTokn->IsNumeric()) {
          s = p[i].set(*pNode->Operand[i].pTokn);
        }
        else if(pNode->Operand[i].pTokn->IsIdentifier()) {
          //const VALUE* pValue = GetVariableValue(pNode->Operand[i].pTokn);
          const IDNFDESC* pIdnfDesc = GetIdentifierDesc(pNode->Operand[i].pTokn);
          if(pIdnfDesc)
          {
            ASSERT(pIdnfDesc->pool.size() == 1);
            p[i] = pIdnfDesc->pool.front();
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

      if(s == VALUE::State_Call || s == VALUE::State_Variable ||
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

  NameContext::State NameContext::IntRegisterIdentifier(
    IDNFDESC** ppVariable, const RefString& rstrType,
    const TOKEN* ptkVariable, GXDWORD dwModifier, const GLOB* pValueExprGlob)
  {
    const TYPEDESC* pDesc = GetType(rstrType);
    if(pDesc == NULL)
    {
      clStringW strW;
      GetLogger()->OutputErrorW(ptkVariable, UVS_EXPORT_TEXT(5012, "“%s”: 类型未定义"), rstrType.ToString(strW).CStr());
      return State_TypeNotFound;
    }
    return IntRegisterIdentifier(ppVariable, pDesc, ptkVariable, dwModifier, pValueExprGlob);
  }
  
  NameContext::State NameContext::IntRegisterIdentifier(
    IDNFDESC** ppVariable, const TYPEDESC* pTypeDesc,
    const TOKEN* ptkVariable, GXDWORD dwModifier, const GLOB* pValueExprGlob)
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

    clpair<IdentifierMap::iterator, bool> insert_result;
    
    // 防止后面误用
    {
      IDNFDESC sIdnfDesc;
      sIdnfDesc.pType = pTypeDesc;
      sIdnfDesc.nOffset = m_nCount | IDNFDESC::Registering;
      insert_result = m_IdentifierMap.insert(clmake_pair(ptkVariable, sIdnfDesc));
    }

    IDNFDESC& rIdnfDesc = insert_result.first->second;
    if(insert_result.second) {
      // 添加成功, 返回type描述
      m_eLastState = State_Ok;
      m_nCount += rIdnfDesc.pType->CountOf();
    }
    else {
      return State_DuplicatedIdentifier;
    }

    if(pValueExprGlob)
    {
      rIdnfDesc.glob = *pValueExprGlob;

      // 如果是初始化列表，就进行推导
      // 这里会重新输出一个整理过的初始化列表到sVariDesc.glob中
      if(pValueExprGlob->CompareAsNode(SYNTAXNODE::MODE_InitList))
      {
        const size_t nErrorCount = m_pCodeParser->DbgErrorCount();
        const GXBOOL bSizeless = (rIdnfDesc.pType->CountOf() == 0);
        rIdnfDesc.pType = m_pCodeParser->InferInitList(&rIdnfDesc.pool, *this, pTypeDesc, &rIdnfDesc.glob);

        if(rIdnfDesc.pType == NULL) {
          ASSERT(rIdnfDesc.pool.empty());
          ASSERT(nErrorCount != m_pCodeParser->DbgErrorCount()); // 缺少无法转换的提示信息
          m_IdentifierMap.erase(insert_result.first);
          return State_CastTypeError;
        }
        else if(bSizeless) {
          m_nCount += rIdnfDesc.pType->CountOf();
        }

        // 变量不记录数值
        if(TEST_FLAG_NOT(dwModifier, VerifyIdentifierDefinition_Const))
        {
          rIdnfDesc.pool.clear();
        }
      }
      else
      {
        VALUE_CONTEXT vctx(*this);
        vctx.pLogger = GetLogger();
        const size_t nErrorCount = m_pCodeParser->DbgErrorCount();
        m_pCodeParser->InferType(vctx, *pValueExprGlob);
        if(vctx.result != ValueResult_OK && vctx.result != ValueResult_Variable) {
          ASSERT(nErrorCount < m_pCodeParser->DbgErrorCount());
          m_IdentifierMap.erase(insert_result.first);
          return State_HasError; // 计算表达式错误
        }
        else if(rIdnfDesc.pType->cate == TYPEDESC::TypeCate_MultiDim && rIdnfDesc.pType->sDimensions.back() == 0 &&
          vctx.pType->cate == TYPEDESC::TypeCate_MultiDim && rIdnfDesc.pType->sDimensions.size() == vctx.pType->sDimensions.size() &&
          m_pCodeParser->TryTypeCasting(rIdnfDesc.pType->pElementType, vctx.pType->pElementType, ptkVariable) )
        {
          rIdnfDesc.pType = rIdnfDesc.pType->ConfirmArrayCount(vctx.pType->sDimensions.back());
          m_nCount += rIdnfDesc.pType->CountOf();
        }
        else if(rIdnfDesc.pType->cate == TYPEDESC::TypeCate_MultiDim && vctx.pType->cate != TYPEDESC::TypeCate_MultiDim) {
          // int a[3] = 0; 形式
          GetLogger()->OutputErrorW(*pValueExprGlob, UVS_EXPORT_TEXT(5051, "应该使用初始化列表来初始化数组"));
          return State_HasError;
        }
        else if(m_pCodeParser->TryTypeCasting(rIdnfDesc.pType, vctx.pType, ptkVariable) == FALSE) {
          GetLogger()->OutputTypeCastFailed(ptkVariable, _CLTEXT("初始化"), rIdnfDesc.pType, vctx.pType);
          return State_CastTypeError;
        }

        if(vctx.pValue) {
          if(TEST_FLAG(dwModifier, VerifyIdentifierDefinition_Const))
          {
            rIdnfDesc.pool.assign(vctx.pValue, vctx.pValue + vctx.count);

            // 转换为对应级别的值
            if(IS_SCALER_CATE(pTypeDesc) || IS_VECMAT_CATE(pTypeDesc))
            {
              std::for_each(rIdnfDesc.pool.begin(), rIdnfDesc.pool.end(), [pTypeDesc](VALUE& value) {
                value.CastValueByRank(static_cast<VALUE::Rank>(pTypeDesc->pDesc->rank));
              });
            }
          }
        }
        else
        {
          ASSERT(vctx.pValue == NULL && vctx.count == 0);
          ASSERT(vctx.result == ValueResult_OK || vctx.result == ValueResult_Variable);
        }
      }
    }
    else {
      rIdnfDesc.glob.ptr = NULL;
    }

    // 尺寸一定与类型相符
    ASSERT(rIdnfDesc.pool.empty() || rIdnfDesc.pool.size() == rIdnfDesc.pType->CountOf()); // 常量池得长度与类型相符
    ASSERT(TEST_FLAG(dwModifier, VerifyIdentifierDefinition_Const) ||  rIdnfDesc.pool.empty()); // 常量确保有常量池
    ASSERT(rIdnfDesc.pType); // 类型不为空
    ASSERT(insert_result.first->second.IsRegistering()); // 登记中

    insert_result.first->second.nOffset &= (~IDNFDESC::Registering);

    *ppVariable = &insert_result.first->second;
    return State_Ok;
  }

  const TYPEDESC* NameContext::RegisterIdentifier(const TOKEN& tkType, const GLOB* pVariableDeclGlob, GXDWORD dwModifier, const GLOB* pValueExprGlob)
  {
    if(pVariableDeclGlob->IsToken()) {
      return RegisterIdentifier(tkType, pVariableDeclGlob->pTokn, dwModifier, pValueExprGlob);
    }
    else if(pVariableDeclGlob->IsNode()) {
      return RegisterMultidimIdentifier(tkType, pVariableDeclGlob->pNode, dwModifier, pValueExprGlob);
    }
    return NULL;
  }

  const TYPEDESC* NameContext::RegisterIdentifier(const TOKEN& tkType, const TOKEN* ptkVariable, GXDWORD dwModifier, const GLOB* pValueExprGlob)
  {
    // PS: 返回值似乎没什么用
    //const TYPEDESC* pTypeDesc = NULL;
    IDNFDESC* pVariDesc = NULL;
    RefString rstrType(tkType.marker, tkType.length);
    ASSERT(tkType.IsIdentifier());
    //tkType.ToString(strType);
    //ASSERT(strType.IsIdentifier());

    m_eLastState = IntRegisterIdentifier(&pVariDesc, rstrType, ptkVariable, dwModifier, pValueExprGlob);
    ASSERT(m_eLastState != State_Ok || pVariDesc->pType != NULL);
    return pVariDesc ? pVariDesc->pType : NULL;
  }

#ifdef ENABLE_SYNTAX_VERIFY

  const TYPEDESC* NameContext::RegisterTypes(const TOKEN& tkBaseType, const TYPEDESC::DimList_T& sDimensions)
  {
    // int x[2][3][4] 列表储存为 "{4,3,2}"
    // 函数会依次注册"int*4", "int*4*3", "int*4*3*2"这几个类型，最后返回"int*4*3*2"这个类型描述
    // 对于自适应长度类型数组"int x[][3][4]", 最后返回的类型为"int*4*3*0"
    // 注意多维数组类型并不一定注册在当前Context下，而是注册在基础类型所属的Context下

    RefString rstrBaseType(tkBaseType.marker, tkBaseType.length);

    const TYPEDESC* pBaseTypeDesc = GetType(rstrBaseType);
    if(pBaseTypeDesc == NULL) {
      clStringW strW;
      GetLogger()->OutputErrorW(tkBaseType, UVS_EXPORT_TEXT2(5012, "“%s”: 类型未定义", GetLogger()), tkBaseType.ToString(strW).CStr());
      return NULL;
    }

    ASSERT(pBaseTypeDesc->cate != TYPEDESC::TypeCate_MultiDim); // 肯定不是多维类型
    TypeMap& sCurrentTypeMap = pBaseTypeDesc->pNameCtx->m_TypeMap;
    clStringA strTypeName;
    pBaseTypeDesc->name.ToString(strTypeName); // name才是真实类型名

    TYPEDESC td = { TYPEDESC::TypeCate_MultiDim, pBaseTypeDesc->pNameCtx };
    //td.name = rstrBaseType;
    td.pElementType = pBaseTypeDesc;
    td.pDesc = pBaseTypeDesc->pDesc;
    for(auto it = sDimensions.begin(); it != sDimensions.end(); ++it)
    {
      strTypeName.Append('*').AppendInteger32(*it); // int x[2][3][4] 记为"int*4*3*2"
      td.name = m_pCodeParser->GetUniqueString(strTypeName);
      td.sDimensions.push_back(*it);

      ASSERT(*it != 0 || (&*it == &sDimensions.back()));

      auto result = sCurrentTypeMap.insert(clmake_pair(td.name, td));
      td.pElementType = &result.first->second;
    }
    return td.pElementType;
  }

  const TYPEDESC* NameContext::RegisterMultidimIdentifier(const TOKEN& tkType, const SYNTAXNODE* pNode, GXDWORD dwModifier, const GLOB* pValueExprGlob)
  {
    ASSERT(pNode->mode == SYNTAXNODE::MODE_Subscript || pNode->mode == SYNTAXNODE::MODE_Subscript0); // 外部保证

    TYPEDESC::DimList_T sDimensions;
    //const TYPEDESC* pTypeDesc = NULL;
    IDNFDESC* pVariDesc = NULL;


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
        if(m_strName == s_szStructMember) {
          state = m_pParent->CalculateConstantValue(m_eLastState, value, &subscript_glob);
        }
        else {
          state = CalculateConstantValue(m_eLastState, value, &subscript_glob);
        }
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
      else if(state == VALUE::State_SyntaxError || state == VALUE::State_Call)
      {
        clStringW strW;
        GetLogger()->OutputErrorW(subscript_glob,
          UVS_EXPORT_TEXT(2057, "应输入常量表达式：“%s”"),
          subscript_glob.GetFrontToken()->ToString(strW).CStr());
        m_eLastState = State_RequireConstantExpression;
        return NULL;
      }
      else if(state == VALUE::State_BadIdentifier)
      {
        ASSERT(m_pCodeParser->DbgErrorCount() > nErrorCount); // 确保内部输出了错误消息
        m_eLastState = State_VariableIsNotIdentifier;
        return NULL;
      }
      else if(state == VALUE::State_Variable)
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

        const TYPEDESC* pSizelessTypeDesc = RegisterTypes(tkType, sDimensions); // 可能缺少最高维尺寸的数组类型
        TRACE("var \"%s\":\n", pNode->GetAnyTokenAPB().ToString().CStr());
        if(pSizelessTypeDesc == NULL) {
          m_eLastState = State_HasError;
          return NULL;
        }

        m_eLastState = IntRegisterIdentifier(&pVariDesc, pSizelessTypeDesc, ptkVariable, dwModifier, pValueExprGlob);
        if(m_eLastState == State_Ok) {
          ASSERT(pVariDesc->pType);
          break;// 下面进行初始化列表的推导
        }
        else
        {
          CodeParser::DumpStateError(GetLogger(), m_eLastState, tkType, *ptkVariable);
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

    return pVariDesc->pType;
  }

#endif

  NameContext::State NameContext::GetLastState() const
  {
    return m_eLastState;
  }

  void NameContext::GetMatchedFunctions(const TOKEN* pFuncName, size_t nFormalCount, cllist<const FUNCDESC*>& aMatchedFunc) const
  {
    // nFormalCount == -1 表示不过滤参数数量
    const NameContext* pRoot = GetRoot(); // 没有局部函数, 所以直接从根查找

    RefString rstrFuncName(pFuncName->marker, pFuncName->length);
    //pFuncName->ToString(strFuncName);


    auto it = pRoot->m_FuncMap.find(rstrFuncName);
    while(it != pRoot->m_FuncMap.end() && it->first == rstrFuncName)
    {
      if(nFormalCount == (size_t)-1 || it->second.sFormalTypes.size() == nFormalCount) {
        aMatchedFunc.push_back(&it->second);
      }
      ++it;
    }
  }

  GXBOOL NameContext::RegisterStruct(const TOKEN* ptkName, const SYNTAXNODE* pMemberNode)
  {
    ASSERT(pMemberNode == NULL || pMemberNode->mode == SYNTAXNODE::MODE_Block);
    TYPEDESC td = {TYPEDESC::TypeCate_Empty, this};
    //clStringA strName;
    td.cate = TYPEDESC::TypeCate_Struct;
    td.name.Set(ptkName->marker, ptkName->length);//->ToString(strName);
    td.pMemberNode = pMemberNode;
    td.pLocation = ptkName;

    auto result = m_TypeMap.insert(clmake_pair(RefString(ptkName->marker, ptkName->length), td));
    //auto result = m_TypeMap.insert(clmake_pair(strName, td));

    // 结构体声明后可以定义
    if(result.first->second.pMemberNode == NULL) {
      result.first->second.pMemberNode = pMemberNode;
      return TRUE;
    }
    else if(pMemberNode == NULL) { // 定义后也可以声明
      return TRUE;
    }
    // TODO: 定义两次给出错误信息
    return result.second;
  }

  GXBOOL NameContext::RegisterStructContext(const RefString& rstrName, const NameContext* pContext)
  {
    auto result = m_StructContextMap.insert(clmake_pair(rstrName, pContext));
    ASSERT(result.second); // RegisterStruct保证不会重复添加结构体定义
    return result.second;
  }

  GXBOOL NameContext::RegisterFunction(const RefString& rstrRetType, const RefString& rstrName, const TYPEINSTANCE::Array& type_array)
    // const FUNCTION_ARGUMENT* pArguments, int argc)
  {
    ASSERT(m_pParent == NULL);
    FUNCDESC td;
    td.ret_type = rstrRetType;
    td.name = rstrName; // ptkName->ToString(strName);

    auto it = m_FuncMap.insert(clmake_pair(rstrName, td));
    it->second.sFormalTypes = type_array;
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
      return (pTypeDesc->name != RefString(ptkTypename->marker, ptkTypename->length)); // 字典名与原始名不一致, 说明是typedef的类型
    }
    return FALSE;
  }

  GXBOOL NameContext::TranslateType(RefString& rstrTypename, const TOKEN* ptkTypename) const
  {
    const TYPEDESC* pTypeFunc = NULL;
    if(IsTypedefedType(ptkTypename, &pTypeFunc))
    {
      rstrTypename = pTypeFunc->name;
      return TRUE;
    }
    rstrTypename.Set(ptkTypename->marker, ptkTypename->length);
    //ptkTypename->ToString(strTypename);
    return FALSE;
  }

  const TYPEDESC* NameContext::GetIdentifier(const TOKEN* ptkName) const
  {
    auto it = m_IdentifierMap.find(TokenPtr(ptkName));
    
    return (it != m_IdentifierMap.end() && _CL_NOT_(it->second.IsRegistering()))
      ? it->second.pType
      : (m_pVariParent
        ? m_pVariParent->GetIdentifier(ptkName)
        : ((ptkName->type == TOKEN::TokenType_String) ? GetType("string") : NULL));
  }

  const IDNFDESC* NameContext::GetIdentifierDesc(const TOKEN* ptkName) const
  {
    auto it = m_IdentifierMap.find(TokenPtr(ptkName));
    if(it == m_IdentifierMap.end() || it->second.IsRegistering()) {
      if(m_pVariParent) {
        return m_pVariParent->GetIdentifierDesc(ptkName);
      }
      return NULL;
    }
    return &it->second;
  }

  const NameContext* NameContext::GetStructContext(const RefString& rstrName) const
  {
    auto it = m_StructContextMap.find(rstrName);
    return (it != m_StructContextMap.end())
      ? it->second
      : (m_pParent
        ? m_pParent->GetStructContext(rstrName)
        : NULL);
  }

  const TYPEDESC* NameContext::GetType(const RefString& rstrType) const
  {
    const TYPEDESC* pBasicTypeDesc;
    auto it = m_TypeMap.find(rstrType);
    //if(it != m_TypeMap.end() || (m_BasicTypeMap.empty() == FALSE &&
    //  (it = m_BasicTypeMap.find(rstrType)) != m_BasicTypeMap.end()))
    if(it != m_TypeMap.end())
    {
      return &it->second;
    }
    else if(m_aBasicType.empty() == FALSE && (pBasicTypeDesc = GetIntrinsicType(rstrType)) != NULL)
    {
      return pBasicTypeDesc;
    }
    else if(m_pParent) {
      return m_pParent->GetType(rstrType);
    }
    return NULL;
  }

  const TYPEDESC* NameContext::GetType(const TOKEN& token) const
  {
    //clStringA str;
    RefString rstr(token.marker, token.length);
    return GetType(rstr);
  }

  const TYPEDESC* NameContext::GetType(VALUE::Rank rank) const
  {
    const NameContext* pRoot = GetRoot();
    //clStringA strTypeName;
    GXLPCSTR szTypeName;
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
      szTypeName = STR_UINT;
      break;
    case UVShader::VALUE::Rank_Signed:
      szTypeName = STR_INT;
      break;
    case UVShader::VALUE::Rank_Float:
      szTypeName = STR_FLOAT;
      break;
    case UVShader::VALUE::Rank_Double:
      szTypeName = STR_DOUBLE;
      break;
    default:
      CLBREAK;
      return NULL;
    }

    return pRoot->GetType(szTypeName);
  }

  const UVShader::TYPEDESC* NameContext::GetIntrinsicType(const RefString& rstrType) const
  {
    return clstd::BinarySearch(&m_aBasicType.front(), &m_aBasicType.back() + 1, rstrType,
      [](const TYPEDESC* pType, const RefString& rstr)->int{
        //RefString rstrA(pType->name.CStr(), pType->name.GetLength());
        return pType->name.Compare(rstr);
      });
  }

  const TYPEDESC* NameContext::ConfirmArrayCount(const TYPEDESC* pTypeDesc, size_t nCount)
  {
    ASSERT(pTypeDesc->sDimensions.back() == 0); // 外部保证
    ASSERT(pTypeDesc->name.EndsWith("*0"));
    ASSERT(pTypeDesc->sDimensions.size() == 1 || GetType(pTypeDesc->pElementType->name)); // 检查多维数组的上一级类型也存在
    clStringA name;
    pTypeDesc->name.ToString(name);
    name.TrimRight('0');
    name.AppendUInt32(static_cast<u32>(nCount));

    auto result = m_TypeMap.insert(
      clmake_pair(RefString(m_pCodeParser->GetUniqueString(name),
        name.GetLength()), *pTypeDesc));
    //auto result = m_TypeMap.insert(clmake_pair(name, *pTypeDesc));

    if(result.second) {
      result.first->second.name = result.first->first; // RefString
      result.first->second.sDimensions.back() = nCount;
    }
    return &result.first->second;
  }

  NameContext::State NameContext::TypeDefine(const TOKEN* ptkOriName, const TOKEN* ptkNewName)
  {
    //clStringA strOriName;
    clStringW strW;
    //TYPEDESC td = { TYPEDESC::TypeCate_Empty, this };
    const TYPEDESC* pDesc = GetType(*ptkOriName);
    if(pDesc == NULL) {
      //pDesc = &td; // 这是干嘛？没看懂
      GetLogger()->OutputErrorW(ptkOriName, UVS_EXPORT_TEXT(5020, "“typedef”定义中使用的“%s”不是一个类型"), ptkOriName->ToString(strW).CStr());
      return State_HasError;
    }

    RefString strNewName(ptkNewName->marker, ptkNewName->length);
    auto it = m_TypeMap.insert(clmake_pair(strNewName, *pDesc));
    if(it.second) {
      it.first->second.pLocation = ptkNewName;
      return State_Ok;
    }

    GetLogger()->OutputErrorW(ptkOriName, UVS_EXPORT_TEXT(5021, "“typedef”定义的类型“%s”已经存在"), ptkNewName->ToString(strW).CStr());
    return State_HasError;
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
      szRxC =(name.GetLength() > STR_HALF_LENGTH) ?  &name[STR_HALF_LENGTH] : NULL;
    }
    else if(name.BeginsWith(STR_FLOAT))
    {
      szScaler = STR_FLOAT;
      szRxC = (name.GetLength() > STR_FLOAT_LENGTH) ? &name[STR_FLOAT_LENGTH] : NULL;
    }
    else if(name.BeginsWith(STR_DOUBLE))
    {
      szScaler = STR_DOUBLE;
      szRxC = (name.GetLength() > STR_DOUBLE_LENGTH) ? &name[STR_DOUBLE_LENGTH] : NULL;
    }
    else if(name.BeginsWith(STR_INT))
    {
      szScaler = STR_INT;
      szRxC = (name.GetLength() > STR_INT_LENGTH) ? &name[STR_INT_LENGTH] : NULL;
    }
    else if(name.BeginsWith(STR_UINT))
    {
      szScaler = STR_UINT;
      szRxC = (name.GetLength() > STR_UINT_LENGTH) ? &name[STR_UINT_LENGTH] : NULL;
    }
    else if(name.BeginsWith(STR_BOOL))
    {
      szScaler = STR_BOOL;
      szRxC = (name.GetLength() > STR_BOOL_LENGTH) ? &name[STR_BOOL_LENGTH] : NULL;
    }
    else
    {
      PARSER_NOTIMPLEMENT;
      clStringA str;
      name.ToString(str);
      TRACE("%s\n", str.CStr());
    }

    R = C = 0;

    if(szScaler && szRxC != NULL)
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
    //ASSERT(cate != TypeCate_Struct || pMemberNode->mode == SYNTAXNODE::MODE_Block); // 防止以后修改
    if(cate == TypeCate_Struct && 
      (pMemberNode == NULL || pMemberNode->mode != SYNTAXNODE::MODE_Block))
    {
      clStringW strW;
      GetLogger()->OutputErrorW(pLocation, UVS_EXPORT_TEXT(5080, "结构体“%s”缺少定义"), name.ToString(strW).CStr());
      return FALSE;
    }

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
        const TOKEN* ptkMember = CodeParser::GetIdentifierNameWithoutSeamantic(it->pNode->Operand[1]);
        if(ptkMember == NULL) {
          GetLogger()->OutputMissingSemicolon(it->pNode->Operand[1].GetFrontToken());
          return FALSE;
        }

        const TYPEDESC* pMemberTypeDesc = pStructCtx->GetIdentifier(ptkMember);
        if(pMemberTypeDesc != NULL)
        {
          sMemberTypeList.push_back(pMemberTypeDesc);
        }
        else
        {
          clStringW strMemberName, strStructName;
          name.ToString(strStructName);
          ptkMember->ToString(strMemberName);
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

  VALUE::Rank TYPEDESC::GetRank() const
  {
    ASSERT(pDesc);
    return static_cast<VALUE::Rank>(pDesc->rank);
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
      CodeParser::BreakComma(reinterpret_cast<SYNTAXNODE::GlobList&>(top.sInitList), *pInitListGlob);
      top.iter = top.sInitList.begin();
      top.ptkOpcode = ptkOpcode;
      if(top.sInitList.empty() || _CL_NOT_(top.iter->glob.CompareAsNode(SYNTAXNODE::MODE_InitList))) {
        break;
      }
      else {
        ptkOpcode = top.iter->glob.pNode->pOpcode;
        pInitListGlob = &top.iter->glob.pNode->Operand[0];
      }
    } // while
    return TRUE;
  }

  CInitList::CInitList(CodeParser* pCodeParser, NameContext& rNameCtx, const SYNTAXNODE::GLOB* pInitListGlob)
    : m_pCodeParser(pCodeParser)
    , m_rNameCtx(rNameCtx)
    , m_pInitListGlob(pInitListGlob)
    , m_bNeedAlignDepth(FALSE)
    , m_bConstantValues(TRUE)
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

  const CInitList::ELEMENT* CInitList::Get()
  {
    if(m_sStack.empty()) {
      return NULL;
    }
    else if(IsEnd() == FALSE)
    {
      Enter(&(Top().iter->glob));
    }
    
    if(Top().sInitList.empty()) {
      return NULL;
    }
    return &(*Top().iter);
  }

  const TYPEDESC* CInitList::ExpandValue(VALUE_CONTEXT& vctx, size_t base_index, size_t array_index)
  {
    // base_index  是当前列表的基础索引, 也可以理解为 top index
    // array_index 是当前列表的元素索引
    const size_t index = base_index + array_index;
    const ELEMENT* pElement = Get();
    const SYNTAXNODE::GLOB* pGlob = pElement ? &pElement->glob : NULL;

    const TYPEDESC* pTypeDesc = NULL;

    if(pElement == NULL)
    {
      // 使用了“{}”定义，解释为0
      pTypeDesc = m_rNameCtx.GetType("int");
    }
    else if(IsValue(pElement)) {
      pTypeDesc = m_rNameCtx.GetType(pElement->pValue->rank);
    }
    else if(_CL_NOT_(IsValue(pElement)))
    {
      ASSERT(pGlob->IsToken() || _CL_NOT_(pGlob->CompareAsNode(SYNTAXNODE::MODE_InitList)));
      pTypeDesc = m_pCodeParser->InferType(vctx, *pGlob);

      if(pTypeDesc == NULL) {
        return NULL;
      }
      else if(IS_STRUCT_CATE(pTypeDesc))
      {
        if(vctx.pValue)
        {
          STACKDESC stage;

          const size_t value_count = pTypeDesc->CountOf();
          ELEMENT el;
          el.glob.ptr = NULL;
          for(size_t i = 0; i < value_count; i++)
          {
            //ASSERT(m_pValuePool[i + index].rank == VALUE::Rank_Unsigned && m_pValuePool[i + index].nValue64 == 0); // 测试没被写过
            m_pValuePool[i + index] = vctx.pValue[i];
            el.pValue = &m_pValuePool[i + index];
            stage.sInitList.push_back(el);
          }

          if(IS_VECMAT_CATE(pTypeDesc))
          {
            STACKDESC& top = Top();
            stage.iter = top.sInitList.insert(top.iter, stage.sInitList.begin(), stage.sInitList.end());
            top.sInitList.erase(top.iter);
            top.iter = stage.iter;
          }
          else
          {
            stage.ptkOpcode = pGlob->GetFrontToken(); // pGlob->pNode->Operand[0].GetFrontToken();
            m_sStack.push_back(stage);
            Top().iter = Top().sInitList.begin();
          }

          pElement = Get();
          pGlob = NULL; // &(pElement->glob);
        }
        else {
          ASSERT(IS_STRUCT_CATE(pTypeDesc) && vctx.count == 0);
          //DbgListAdd(pElement);
        }
      }
      else if(_CL_NOT_(IS_SCALER_CATE(pTypeDesc)))
      {
        GetLogger()->OutputErrorW(*pGlob, UVS_EXPORT_TEXT(5052, "初始值设定项不能用于初始化列表"));
        return NULL;
      }
    }

    //DbgListAdd(pElement);
    ASSERT(pTypeDesc);
    return pTypeDesc;
  }

  const TYPEDESC* CInitList::CastToValuePool(VALUE_CONTEXT& vctx, const TYPEDESC* pRefTypeDesc, size_t base_index, size_t array_index)
  {
    // pRefTypeDesc 参考类型描述，标量/标量数组中这个是标量类型，结构体/向量/矩阵中这个是结构体/向量/矩阵类型
    // base_index  是当前列表的基础索引, 也可以理解为 top index
    // array_index 是当前列表的元素索引
    const size_t index = base_index + array_index;
    const ELEMENT* pElement = Get();
    const SYNTAXNODE::GLOB* pGlob = pElement ? &pElement->glob : NULL;

    const TYPEDESC* pTypeDesc = NULL;
    //Result func_result = Result_Ok;

    if(pElement == NULL)
    {
      // 使用了“{}”定义，解释为0
      ASSERT(m_pValuePool[index].rank == VALUE::Rank_Unsigned && m_pValuePool[index].nValue64 == 0);
      pTypeDesc = m_rNameCtx.GetType("int");
    }
    else if(IsValue(pElement)) {
      ASSERT(pElement->pValue == m_pValuePool + index); // 已在池中
      ASSERT(IS_SCALER_CATE(vctx.pType));
      pElement->pValue->CastValueByRank(vctx.pType->GetRank());
      vctx.pValue = pElement->pValue;
      vctx.count = 1;
      pTypeDesc = m_rNameCtx.GetType(pElement->pValue->rank);
    }
    else
    {
      ASSERT(pGlob->IsToken() || _CL_NOT_(pGlob->CompareAsNode(SYNTAXNODE::MODE_InitList)));
      //VALUE_CONTEXT vctx(m_rNameCtx);
      //vctx.pLogger = GetLogger();
      //vctx.bNeedValue = FALSE;
      const TYPEDESC* pMemberType = vctx.pType;
      vctx.pType = NULL; // 外部用这个传递值，这里用之前清理一下
      pTypeDesc = m_pCodeParser->InferType(vctx, *pGlob);

      if(pTypeDesc == NULL) {
        return NULL;
      }
      else if(IS_STRUCT_CATE(pTypeDesc))
      {
        // 这里允许推导出非常量表达式
        if(vctx.pValue)
        {
          vctx.pLogger->OutputTypeCastFailed(GetLocation(), _CLTEXT("初始化列表"), pMemberType, pTypeDesc);
          vctx.ClearValue(ValueResult_Failed);
          return NULL;
        }
      }
      else if(IS_SCALER_CATE(pTypeDesc))
      {
        if(vctx.count == 0) {
          m_bConstantValues = FALSE;
        }
        else if(vctx.count == 1) {
          m_pValuePool[index] = *vctx.pValue;
          ASSERT(pMemberType->pDesc);
          m_pValuePool[index].CastValueByRank(pMemberType->GetRank());
        }
        else {
          CLBREAK;
        }
      }
      else
      {
        GetLogger()->OutputErrorW(*pGlob, UVS_EXPORT_TEXT(5052, "初始值设定项不能用于初始化列表"));
        return NULL;
      }
    }
    DbgListAdd(pElement);
    ASSERT(pTypeDesc);
    return pTypeDesc;
  }

  const TOKEN* CInitList::GetLocation() const
  {
    ASSERT(!m_sStack.empty());
    const STACKDESC& top = Top();

    if(top.sInitList.empty() || IsValue(&*(top.iter))) {
      return Top().ptkOpcode;
    }
    return Top().iter->glob.GetFrontToken();
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
    return &(top.iter->glob);
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
    return m_sStack.size();
  }

  GXBOOL CInitList::IsConstantValues() const
  {
    return m_bConstantValues;
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

  GXBOOL CInitList::IsValue(const ELEMENT* pElement) const
  {
    return (pElement->pValue >= m_pValuePool && pElement->pValue < (m_pValuePool + m_nValueCount));
  }

  VALUE& CInitList::FillZeroByRank(size_t index, VALUE::Rank rank)
  {
    ASSERT(index < m_nValueCount);
    ASSERT(m_pValuePool[index].IsZero() && m_pValuePool[index].rank == VALUE::Rank_Unsigned); // 覆盖测试
    return m_pValuePool[index].SetZero(rank);
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

  UVShader::NameContext& CInitList::GetNameContext() const
  {
    return m_rNameCtx;
  }

  void CInitList::DbgListBegin(const RefString& rstrTypeName)
  {
    m_DebugStrings.back() = "<";
    m_DebugStrings.back().Append(rstrTypeName.GetPtr(), rstrTypeName.GetLength()).Append(">{");

    COMMALIST cm;
    cm.Init(m_pCodeParser);
    m_RearrangedGlob.push_back(cm);
  }

  void CInitList::DbgListAdd(const ELEMENT* pElement)
  {
    clStringA strA;
    DbgListAdd(pElement ? (IsValue(pElement) 
      ? pElement->pValue->ToString(strA)
      : pElement->glob.ToString(strA)) : "0");

    COMMALIST& cm = m_RearrangedGlob.back();
    if(pElement == NULL)
    {
      VALUE v;
      SYNTAXNODE::GLOB temp_glob = { NULL };
      m_pCodeParser->SetRepalcedValue(temp_glob, v.SetZero());
      cm.PushBack(&temp_glob);
    }
    else if(IsValue(pElement))
    {
      SYNTAXNODE::GLOB temp_glob = { NULL };
      m_pCodeParser->SetRepalcedValue(temp_glob, *pElement->pValue);
      cm.PushBack(&temp_glob);
    }
    else
    {
      cm.PushBack(&pElement->glob);
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
    , pMemberCtx(NULL)
    , bNeedValue(TRUE)
    , pLogger(NULL)
    , result(ValueResult_Undefined)
    , pType(NULL)
    , pValue(NULL)
    , count(0)
  {
  }

  VALUE_CONTEXT::VALUE_CONTEXT(const NameContext& _name_ctx, CLogger* pLogger)
    : name_ctx(_name_ctx)
    , pMemberCtx(NULL)
    , bNeedValue(TRUE)
    , pLogger(pLogger)
    , result(ValueResult_Undefined)
    , pType(NULL)
    , pValue(NULL)
    , count(0)
  {
  }

  VALUE_CONTEXT::VALUE_CONTEXT(const NameContext& _name_ctx, GXBOOL _bNeedValue)
    : name_ctx(_name_ctx)
    , pMemberCtx(NULL)
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
    , pMemberCtx(NULL)
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

  const VALUE_CONTEXT& VALUE_CONTEXT::ClearValue(ValueResult r)
  {
    result = r;
    pType = NULL;
    pValue = NULL;
    count = 0;
    pool.clear();
    return *this;
  }

  void VALUE_CONTEXT::ClearValueOnly()
  {
    pValue = NULL;
    count = 0;
    pool.clear();
  }

  void VALUE_CONTEXT::CopyValue(const VALUE_CONTEXT& vc)
  {
    result = vc.result;
    pType  = vc.pType;
    pValue = vc.pValue;
    count  = vc.count;
    if(_CL_NOT_(vc.pool.empty()))
    {
      pool = vc.pool;
      UsePool();
    }
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
      (type_count <= targ_count && TypeRank() < pTargetType->pDesc->rank));

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
    else if(pType->IsVector() && pTargetType->IsVector() && type_count == targ_count)
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
        (*it).CastValueByRank(pTargetType->GetRank());
      }
      UsePool();
    }
    else if(IS_SCALER_CATE(pType) && IS_SCALER_CATE(pTargetType))
    {
      if(pool.empty()) {
        pool.push_back(*pValue);
        UsePool();
      }
      pool.front().CastValueByRank(pTargetType->GetRank());
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
    ASSERT(VALUE::IsNumericRank(pScalerType->GetRank()));
    ASSERT(VALUE::IsNumericRank(pVecMatType->GetRank()));
    const TYPEDESC* pTypeDesc = pVecMatType;
    if(pScalerType->pDesc->rank > pVecMatType->pDesc->rank)
    {
      clStringA str = pScalerType->pDesc->name;
      str.Append(pVecMatType->pDesc->name + clstd::strlenT(pVecMatType->pDesc->component_type));
      pTypeDesc = name_ctx.GetType(str.CStr());
      ASSERT(pTypeDesc);
    }    
    return pTypeDesc;
  }

  GXBOOL VALUE_CONTEXT::IsNeedValue() const
  {
    ASSERT((pValue == NULL && count == 0) || (pValue != NULL && count > 0));
    return bNeedValue && result == ValueResult_OK && pValue != NULL;
  }

  void VALUE_CONTEXT::GenerateMathComponentValue(const DOTOPERATOR_RESULT& sDotOperator)
  {
    if(pValue == NULL) {
      result = ValueResult_OK;
      return;
    }

    if(IS_SCALER_CATE(pType))
    {
      pValue = pValue + sDotOperator.components[0];
      count = 1;
      result = ValueResult_OK;
    }
    else if(IS_VECMAT_CATE(pType))
    {
      ValuePool temp_pool;
      for(int i = 0; sDotOperator.components[i] != -1; i++)
      {
        temp_pool.push_back(pValue[sDotOperator.components[i]]); // FIXME: 可能会有溢出
        temp_pool.back().CastValueByRank(TypeRank());
      }
      pool.assign(temp_pool.begin(), temp_pool.end());
      result = ValueResult_OK;
      UsePool();
    }
    else {
      CLBREAK;
    }
  }

  VALUE::Rank VALUE_CONTEXT::TypeRank() const
  {
    return (static_cast<VALUE::Rank>(pType->pDesc->rank));
  }

  //////////////////////////////////////////////////////////////////////////

  VALUE_CONTEXT_CHECKER::VALUE_CONTEXT_CHECKER(const VALUE_CONTEXT& _vctx)
    : vctx(_vctx)
    , nErrorCount(static_cast<int>(_vctx.pLogger->ErrorCount(TRUE)))
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
      ASSERT(static_cast<int>(vctx.pLogger->ErrorCount(TRUE)) > nErrorCount);
    }

    // 检查value有效性
    if(vctx.pValue && vctx.pType)
    {
      // 指针有效时肯定有值
      ASSERT(vctx.pValue != NULL && vctx.count > 0);

      // 类型与数量一致
      ASSERT(vctx.count == vctx.pType->CountOf());

      if(vctx.count &&
        (IS_SCALER_CATE(vctx.pType) || IS_VECMAT_CATE(vctx.pType)))
      {
        const VALUE::Rank rank = vctx.TypeRank();
        for(size_t i = 0; i < vctx.count; i++)
        {
          ASSERT(rank == vctx.pValue[i].rank || vctx.pValue[i].IsZero());
        }
      }

      for(size_t i = 0; i < vctx.count; i++)
      {
        ASSERT(vctx.pValue[i].rank == VALUE::Rank_String ||
          vctx.pValue[i].rank >= VALUE::Rank_First && vctx.pValue[i].rank <= VALUE::Rank_Last);
      }
    }

    if(vctx.pValue && vctx.pool.empty() == FALSE) {
      ASSERT(vctx.pValue >= &vctx.pool.front() && vctx.pValue <= &vctx.pool.back());
    }

    ASSERT((vctx.pValue != NULL && vctx.count > 0) ||
      (vctx.pValue == NULL && vctx.count == 0));
  }

  void VALUE_CONTEXT_CHECKER::ClearErrorCount()
  {
    --nErrorCount;
  }

  clStringW& FUNCDESC::ToString(clStringW& str) const
  {
    str.Clear();
    str.Append(ret_type.GetPtr(), ret_type.GetLength()).Append(' ').Append(name.GetPtr(), name.GetLength()).Append('(');
    for(const TYPEINSTANCE& ti : sFormalTypes)
    {
      str.Append(ti.pTypeDesc->name.GetPtr(), ti.pTypeDesc->name.GetLength()).Append(',');
    }
    str.TrimRight(',');
    str.Append(')');
    return str;
  }

} // namespace UVShader
