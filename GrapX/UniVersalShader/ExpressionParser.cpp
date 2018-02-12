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
  extern STRUCTDESC s_aIntrinsicStruct[];
  extern STRUCTDESC s_aBaseType[];


  DefaultInclude s_DefaultInclude;

  CodeParser::CodeParser(PARSER_CONTEXT* pContext, Include* pInclude)
    : m_pContext(pContext)
    , m_pSubParser(NULL)
    , m_dwState(0)
    , m_pParent(NULL)
    , m_nPPRecursion(0)
    , m_pInclude(pInclude ? pInclude : &s_DefaultInclude)
  {
    m_GlobalSet.allow_keywords = KeywordFilter_typedef;
    m_GlobalSet.SetParser(this);
    if(pContext) {
      pContext->nRefCount++;
    }

    SetIteratorCallBack(CodeParser::IteratorProc, 0);
    InitPacks();
  }

  CodeParser::~CodeParser()
  {
    if(m_pContext && _CL_NOT_(--m_pContext->nRefCount)) {
      delete m_pContext;
    }
    m_pContext = NULL;

    // 如果是子解析器，这里借用了父对象的信息定位，退出时要清空，避免析构时删除
    SAFE_DELETE(m_pSubParser);

    if(_CL_NOT_(m_bRefMsg)) {
      ErrorMessage::Destroy(m_pMsg);
    }
    m_pMsg = NULL;

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

  void CodeParser::Cleanup()
  {
    m_PhonyTokenDict.clear();
    m_GlobalSet.Cleanup();
    m_errorlist.clear();
    m_aTokens.clear();
    m_aStatements.clear();
    //m_Macros.clear();
    SAFE_DELETE(m_pSubParser);
    InitPacks();
  }

  b32 CodeParser::Attach(const char* szExpression, clsize nSize, GXDWORD dwFlags, GXLPCWSTR szFilename)
  {
    Cleanup();
    if( ! m_pContext) {
      m_pContext = new PARSER_CONTEXT;
      m_pContext->nRefCount = 1;
    }

    m_dwState = dwFlags;
    if(TEST_FLAG_NOT(dwFlags, AttachFlag_NotLoadMessage))
    {
      if( ! m_pMsg) {
        m_pMsg = ErrorMessage::Create();
        m_pMsg->LoadErrorMessage(L"uvsmsg.txt");
        m_pMsg->SetMessageSign('C');
        m_pMsg->PushFile(szFilename);
      }
      m_pMsg->GenerateCurLines(szExpression, nSize);
    }
    return CTokens::Attach(szExpression, nSize);
  }

  //////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////

  u32 CALLBACK CodeParser::IteratorProc( iterator& it, u32 remain, u32_ptr lParam )
  {
    GXBOOL bENotation = FALSE;

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
        pThis->OutputErrorW(UVS_EXPORT_TEXT2(2014, "预处理器命令必须作为第一个非空白空间启动", pThis));
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
        return 0 ;
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
          pThis->OutputErrorW(it.marker, UVS_EXPORT_TEXT2(1020, "意外的 #endif", pThis));
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
    else
    {
      ArithmeticExpression::IteratorProc(it, remain, lParam);
    }

    if(TEST_FLAG(m_aCharSem[it.marker[0]], M_CALLBACK)) {
      return MultiByteOperatorProc(it, remain, lParam);
    }

    ASSERT((int)remain >= 0);
    return 0;
  }

  clsize CodeParser::GenerateTokens(CodeParser* pParent)
  {
    CodeParser* pPrevParent = m_pParent;
    m_pParent = pParent;
    auto stream_end = end();
    ASSERT(m_aTokens.empty()); // 调用者负责清空

    if( ! m_pMsg && pParent) {
      m_bRefMsg = TRUE;
      m_pMsg = pParent->m_pMsg;
    }


    m_aTokens.reserve(EstimateForTokensCount());
    TOKEN token;
    //TOKEN l_token; // 用来在迭代器中储存符号优先级的信息
    //clstack<int> sMacroStack;
    //clStringA strMacro;
    
    // 只是清理
    token.ClearMarker();
    token.precedence = 0;
    token.unary      = 0;

    SetIteratorCallBack(IteratorProc, (u32_ptr)&token);

    PairStack sStack[countof(s_PairMark)];


    auto it = begin();
    token.Set(it);

    int EOE = m_aTokens.size(); // End Of Expression
    cllist<clsize> UnaryPendingList; // "++", "--" 未确定前置还是后缀的列表

    //m_Macros[MACRO_FILE]
    //m_Macros[MACRO_LINE]

    for(; it.pContainer == NULL || it != stream_end; GetNext(it, token))
    {
      // 上一个预处理结束后，后面的语句长度设置为0可以回到主循环步进
      // 这样做是为了避免如果下一条语句也是预处理指令，不会在处理回调中发生递归调用
      if(it.length == 0) {
        continue;
      }

      if( ! m_pParent && token == PREPROCESS_pound) {
        OutputErrorW(token, UVS_EXPORT_TEXT(2121, "“#”: 无效字符 : 可能是宏展开的结果")); // 无效的"#"号_可能是宏扩展
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
            if(l_back.precedence != 0 && l_back != ')' && l_back != ']' && (!l_back.unary)) {
              const auto& p = s_plus_minus[(int)(it.marker[0] - '+')];
              token.SetArithOperatorInfo(p);
            }
          }
          else if(it.length == 2 && (it.marker[1] == '-' || it.marker[1] == '+'))
          {
            // "++","--" 默认按照前缀操作符处理, 这里检查是否转换为后缀操作符
            if(l_back.IsIdentifier()) {
              const auto& p = s_UnaryLeftOperand[(int)(it.marker[0] - '+')];
              token.SetArithOperatorInfo(p);
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
      MarryBracket(sStack, token);
      
      if(OnToken(token)) {
        token.ClearMarker();
      }

      // 可能被宏展开后清除
      if(token.marker)
      {
        if(_CL_NOT_(MergeStringToken(token)))
        {
          m_aTokens.push_back(token);

          if(token == "false")
          {
            m_aTokens.back().type = TOKEN::TokenType_Integer;
            SetTokenPhonyString(m_aTokens.size() - 1, "0");
          }
          else if(token == "true")
          {
            m_aTokens.back().type = TOKEN::TokenType_Integer;
            SetTokenPhonyString(m_aTokens.size() - 1, "1");
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
      OutputErrorW(UVS_EXPORT_TEXT(1070, "“%s”文件中的 #if/#endif 对不匹配"), m_pMsg->GetFilenameW());
    }

    // 检查括号匹配
    for(int i = 0; i < countof(s_PairMark); ++i)
    {
      PairStack& s = sStack[i];
      if( ! s.empty()) {
        // ERROR: 闭括号不匹配
        OutputErrorW(m_aTokens[s.top()], UVS_EXPORT_TEXT(5001, "文件异常结尾, 缺少闭括号."));
        //ERROR_MSG__MISSING_CLOSEDBRACKET;
      }
    }

    // 对未确定的"++","--"重新确定
    for(auto it = UnaryPendingList.begin(); it != UnaryPendingList.end(); ++it)
    {
      clsize n = *it;
      if(n + 1 >= m_aTokens.size()) {
        // 后缀操作符, 但是语法有错误
        token.SetArithOperatorInfo(s_UnaryLeftOperand[(int)(m_aTokens[n].marker[0] - '+')]);
      }
      //else if(m_aTokens[n + 1].IsIdentifier()) {
      //  // 前缀操作符, 不做处理
      //}
    }

    SetIteratorCallBack(IteratorProc, NULL);

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
      if(last_one.pContainer && token.pContainer && last_one.marker < token.marker &&
        ! last_one.BeginsWith('\"') && ! last_one.EndsWith('\"') && ! token.BeginsWith('\"') && ! token.EndsWith('\"'))
      {
        ASSERT(last_one.marker < m_pEnd && token.marker < m_pEnd);
        last_one.length = (clsize)token.marker + token.length - (clsize)last_one.marker;
      }
      else {
        //PHONY_TOKEN pt;
        const iterator* aTonkens[] = {&last_one, &token};
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

  void CodeParser::SetTokenPhonyString(int index, const clStringA& str)
  {
    PHONY_TOKEN pt;
    auto emplace = m_PhonyTokenDict.emplace(clmake_pair(index, pt));
    TOKEN& token = m_aTokens[index];
    if(emplace.second)
    {
      ASSERT(_CL_NOT_(token.bPhony)); // 第一次添加肯定不是替代值
      emplace.first->second.ori_marker = token.marker;
    }
    else
    {
      ASSERT(token.bPhony); // 添加肯定已经标记为"替代品"
    }
    emplace.first->second.szPhonyText = GetUniqueString(str.CStr());
    emplace.first->second.nPhonyLength = str.GetLength();
    token.SetPhonyString(emplace.first->second.szPhonyText, emplace.first->second.nPhonyLength);
    //return emplace.first->second.str;
  }

  const CodeParser::MACRO* CodeParser::FindMacro(const TOKEN& token) // TODO: 正常的查找都要换做这个
  {
    clStringA strTokenName = token.ToString();

    auto& it_macro = m_pContext->Macros.find(strTokenName);
    if(it_macro != m_pContext->Macros.end()) {
      return &it_macro->second;
    }

    //if(m_pParent) {
    //  it_macro = m_pParent->m_Macros.find(strTokenName);
    //  return it_macro == m_pParent->m_Macros.end() ? NULL : &it_macro->second;
    //}

    return NULL;
  }

  void CodeParser::GetNext(iterator& it, TOKEN& token) // TODO: it 与 token 合并为 token
  {
    if(m_ExpandedStream.empty()) {
      token.ClearMarker();
      token.ClearArithOperatorInfo();
      ++it; // next(it);
      token.Set(it);
    }
    else {
      token = m_ExpandedStream.front();

      if(token.marker == NULL) {
        // m_ExpandedStream 最后一个token可能记录的是结尾
        ASSERT(m_ExpandedStream.size() == 1);
        it = end();
      } else {
        it = token;
      }
      token.semi_scope = -1;
      token.scope = -1;
      m_ExpandedStream.pop_front();
    }
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

    //clStringA strTokenName = token.ToString();
    const MACRO* pMacro = FindMacro(token);

    // 如果没有定义宏的形参和代换，则不处理这个宏
    if( ! pMacro || (pMacro->aFormalParams.empty() && pMacro->aTokens.empty())) {
      return FALSE;
    }

    iterator it = token;
    TOKEN::List stream;

    if(pMacro->aFormalParams.empty())
    {
      stream.insert(stream.end(), pMacro->aTokens.begin(), pMacro->aTokens.end());
      token.ClearMarker();
      ++it;
      token.Set(it);
    }
    else
    {
      int depth = 0;
      iterator it_end = end();
      TOKEN save_token = token;

      for(; it != it_end; ++it)
      {
        token.Set(it);
        stream.push_back(token);
        token.ClearMarker();
        token.ClearArithOperatorInfo();

        if(it == '(') {
          depth++;
        }
        else if(it == ')') {
          depth--;
          if(depth <= 0) {
            break;
          }
        }
      }

      ASSERT(it == it_end || it == ')');

      if(depth < 0)
      {
        if(TEST_FLAG(m_dwState, AttachFlag_NotExpandCond)) {
          // FIXME: 这里没有处理 #if defined(ADD(1,2)) 这种符合ADD(a,b)形参的形式, 可能会导致后面表达式计算出错！
          token = save_token;
          return FALSE;
        }
        else {
          // ERROR: 宏看起来有形参，但实际使用时缺少形参
          CLBREAK;
        }
      }

      if(it == ')') {
        ++it;
      }
      token.Set(it);
    } // if(ctx.pMacro->aFormalParams.empty())

    ASSERT(token.pContainer);
    TOKEN next_token = token;
    ExpandMacroStream(stream, token);
    m_ExpandedStream = stream;
    m_ExpandedStream.push_back(next_token);
    return TRUE;
  }

  GXBOOL CodeParser::TryMatchMacro(MACRO_EXPAND_CONTEXT& ctx_out, TOKEN::List::iterator* it_out, const TOKEN::List::iterator& it_begin, const TOKEN::List::iterator& it_end)
  {
    if(it_begin->type == TOKEN::TokenType_String || TEST_FLAG(m_dwState, AttachFlag_NotExpandMacro) || ! m_ExpandedStream.empty()) {
      return FALSE;
    }

    // 没有找到同名宏定义
    if(_CL_NOT_(ctx_out.pMacro = FindMacro(*it_begin))) {
      return FALSE;
    }

    TOKEN::List::iterator it = it_begin;

    if(ctx_out.pMacro->aFormalParams.empty())
    {
      ctx_out.ActualParam.clear();
      ++it;
    }
    else
    {
      auto it_prev = it;
      ++it;
      //if(*it != '(' || it == it_end) {
      if(it == it_end) {
        OutputErrorW(*it_prev, UVS_EXPORT_TEXT(2008, "“%s”:宏定义中的意外"), clStringW(it_prev->ToString()));
        *it_out = it;
        return TRUE;
      }
      else if(*it != '(') {
        // 宏定义有形参"r(a)", 但是代码中非函数形式"r=x"形式的调用不认为是宏
        return FALSE;
      }

      if(++it == it_end) {
        OutputErrorW(*it, UVS_EXPORT_TEXT(2008, "“%s”:宏定义中的意外"), clStringW(it->ToString()));
        *it_out = it;
        return TRUE;
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
      if(*it == ')') {
        ++it;
      }
    } // if(ctx.pMacro->aFormalParams.empty())

    ctx_out.stream.clear();
    ExpandMacro(ctx_out);
    *it_out = it;
    return TRUE;
  }

  void CodeParser::ExpandMacro(MACRO_EXPAND_CONTEXT& c)
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
          ExpandMacroStream(p, *c.pLineNumRef);
          c.stream.insert(c.stream.end(), p.begin(), p.end());
        }
      }
      else {
        c.stream.push_back(*it_element);
        bPound = FALSE;
      }
    }

    ExpandMacroStream(c.stream, *c.pLineNumRef); // 这个展开非形参部分的宏
  }

  void CodeParser::ExpandMacroStream(TOKEN::List& sTokenList, const TOKEN& line_num)
  {
    TOKEN::List::iterator itMacroEnd; // 如果有展开的宏，则将宏的结尾放置在这个里面
    TOKEN::List::iterator it_end = sTokenList.end();
    MACRO_EXPAND_CONTEXT ctx;
    ctx.pLineNumRef = &line_num;

    for(auto it = sTokenList.begin(); it != it_end;)
    {
      if(ExpandInnerMacro(*it, line_num)) {

      }
      else if(TryMatchMacro(ctx, &itMacroEnd, it, it_end)) {
        it = sTokenList.erase(it, itMacroEnd);
        TOKEN::List::iterator it_next = it;
        if (it_next != it_end) {        
          ++it_next;
        }
        sTokenList.insert(it, ctx.stream.begin(), ctx.stream.end());
        it = it_next;
      }
      else {
        ++it;
      }
    } // for
  }

  

  


  GXBOOL CodeParser::Parse()
  {
    __try
    {
      TKSCOPE scope(0, m_aTokens.size());
      while(ParseStatement(&scope));
      RelocalePointer();
      return m_errorlist.empty();
    }
    //catch(const std::exception& e)
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      // ERROR: 致命错误, 无法从错误中恢复
      OutputErrorW(UVS_EXPORT_TEXT(5002, "致命错误, 无法从错误中恢复"));
    }
    return FALSE;
  }

  GXBOOL CodeParser::ParseStatement(TKSCOPE* pScope)
  {
    return (pScope->begin < pScope->end) &&
      (
        ParseStatementAs_Struct(pScope) ||
        ParseStatementAs_Typedef(pScope) ||
        ParseStatementAs_Definition(pScope) ||
        ParseStatementAs_Function(pScope)
      );
  }

  GXBOOL CodeParser::ParseStatementAs_Definition(TKSCOPE* pScope)
  {
    if(pScope->begin == pScope->end) {
      return TRUE;
    }

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
    if(!ParseExpression(stat.sRoot, m_GlobalSet, scope))
    {
      ERROR_MSG__MISSING_SEMICOLON(IDX2ITER(scope.end));
      return FALSE;
    }

    pScope->begin = definition_end; // 步进到下一个statement

    if(stat.sRoot.IsNode() && stat.sRoot.pNode->mode != SYNTAXNODE::MODE_Definition)
    {
      const TOKEN tk = stat.sRoot.pNode->GetAnyTokenAPB();
      clStringW str;
      OutputErrorW(tk, UVS_EXPORT_TEXT(5023, "“%s” : 不是一个定义."), tk.ToString(str).CStr());
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
      Verify_VariableDefinition(m_GlobalSet, stat.sRoot.pNode);
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

    NameContext sNameSet_Func(m_GlobalSet);
    if(pScope->begin == pScope->end) {
      return TRUE;
    }

    TOKEN* p = &m_aTokens[pScope->begin];
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
        OutputErrorW(*p, UVS_EXPORT_TEXT(2055, "应输入形参表，而不是类型表"));
      }

      TKSCOPE func_statement_block; // (m_aTokens[p->scope].scope, p->scope);
      InitTokenScope(func_statement_block, *p, TRUE);

      stat.type = StatementType_Function;
      p = &m_aTokens[p->scope];
      ++p;

      if(func_statement_block.IsValid())
      {
        m_nDbgNumOfExpressionParse = 0;
        m_aDbgExpressionOperStack.clear();

        ASSERT(func_statement_block.begin < func_statement_block.end); // 似乎不应该有相等的情况, "{}" 区间这种是相差一个的
        sNameSet_Func.allow_keywords = KeywordFilter_InFuntion;
        if(func_statement_block.GetSize() == 1)
        {
          stat.sRoot.ptr = NULL;
        }
        else if(ParseCodeBlock(stat.sRoot, sNameSet_Func, func_statement_block))
        {
#ifdef ENABLE_SYNTAX_VERIFY
          if(Verify_Block(stat.sRoot.pNode, &sNameSet_Func) == FALSE)
          {
            return FALSE;
          }
#endif
        }
      }
    }
    else {
      // ERROR: 声明看起来是一个函数，但是既不是声明也不是实现。
      OutputErrorW(*p, UVS_EXPORT_TEXT(5011, "声明看起来是一个函数，但是既不是声明也不是实现"));
      return FALSE;
    }


    m_aStatements.push_back(stat);
    pScope->begin = p - &m_aTokens.front();
    return TRUE;
  }

  SYNTAXNODE* CodeParser::FlatDefinition(SYNTAXNODE* pThisChain)
  {
    SYNTAXNODE* pNode = pThisChain->Operand[0].pNode;

    // 外部保证满足如下关系
    ASSERT(pThisChain->mode == SYNTAXNODE::MODE_Chain);
    ASSERT(pNode->mode == SYNTAXNODE::MODE_Definition);

    SYNTAXNODE::PtrList sVarList;
    SYNTAXNODE::GLOB* pFirstVar = BreakDefinition(sVarList, pNode->Operand[1].pNode);
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

  CodeParser::TKSCOPE::TYPE CodeParser::ParseStructDefinition(NameContext& sNameSet, const TKSCOPE& scope, SYNTAXNODE::GLOB* pMembers, SYNTAXNODE::GLOB* pDefinitions, int* pSignatures, int* pDefinitionNum)
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
      OutputErrorW(*ptkName, UVS_EXPORT_TEXT(2332, "“%s”: 缺少标记名"), (p - 1)->ToString(str).CStr());
      ASSERT(str == _CLTEXT("struct"));
      return p - pTokensFront;
    }

    clStringA strName;
    p->ToString(strName);

    ++p;
    if(p >= pEnd) {
      OutputErrorW(*p, UVS_EXPORT_TEXT(2143, "语法错误 : 缺少“;”"));
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

      NameContext sMemberSet(sNameSet);

      if(_CL_NOT_(ParseExpression(*pMembers, sMemberSet, sMembersScope)))
      {
        ERROR_MSG__MISSING_SEMICOLON(IDX2ITER(sMembersScope.end));
        return sMembersScope.end;
      }

      p = pTokensFront + sMembersScope.end;
      if(p >= pEnd) {
        OutputErrorW(m_aTokens[scope.begin], UVS_EXPORT_TEXT(1004, "遇到意外的文件结束")); // 遇到意外的文件结束
        return scope.end;
      }
      else if(m_aTokens[scope.begin] != ";" && m_aTokens[scope.begin].IsIdentifier() == FALSE) {
        OutputErrorW(m_aTokens[scope.begin], UVS_EXPORT_TEXT(2145, "语法错误：标识符前面缺少“%s”"), ";"); // "语法错误：标识符前面缺少“%s”"
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
                OutputErrorW(t, UVS_EXPORT_TEXT(2062, "意外的类型“%s”"), clStringW(str));
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
          Verify_StructMember(sNameSet, *pMembers->pNode); // 检查失败仍然继续解析
#endif
        }

        //stat.type = nSignatures ? StatementType_Signatures : StatementType_Struct;
        //stat.stru.nNumOfMembers = nDefination;
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
        OutputErrorW(*ptkName, UVS_EXPORT_TEXT(2371, "“%s”: 重定义；不同的基类型"), ptkName->ToString(str).CStr());
        return sMembersScope.end;
      }


      p = pTokensFront + sMembersScope.end;
    } // else if(*p == '{')

    //////////////////////////////////////////////////////////////////////////

    if(p > &m_aTokens.back()) {
      OutputErrorW(*pTokensFront, UVS_EXPORT_TEXT(1004, "遇到意外的文件结束"));
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

      SYNTAXNODE::GLOB glob;

      if(_CL_NOT_(ParseExpression(glob, sNameSet, scope_var)))
      {
        ERROR_MSG__MISSING_SEMICOLON(IDX2ITER(scope_var.end));
        return FALSE;
      }

      // 封装一个定义序列
      SYNTAXNODE* pNewDef = m_NodePool.Alloc();
#ifdef ENABLE_SYNTAX_NODE_ID
      pNewDef->id = m_nNodeId++;
#endif
      pNewDef->magic = SYNTAXNODE::FLAG_OPERAND_MAGIC;
      pNewDef->mode = SYNTAXNODE::MODE_Definition;
      pNewDef->pOpcode = NULL;
      pNewDef->Operand[0].pTokn = ptkName;
      pNewDef->Operand[1].ptr = glob.ptr;

      glob.pNode = pNewDef;

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
      OutputErrorW(*p, UVS_EXPORT_TEXT(2143, "语法错误 : 缺少“;”(在“%s”的前面)"), p->ToString(str).CStr());
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
      else if(*p == "uniform") {
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
    pStat->func.pArguments = (FUNCTION_ARGUMENT*)m_aArgumentsPack.size();
    pStat->func.nNumOfArguments = aArgs.size();

    m_aArgumentsPack.reserve(m_aArgumentsPack.size() + aArgs.size());
    for(auto it = aArgs.begin(); it != aArgs.end(); ++it)
    {
      clStringA str;
      sNameSet.RegisterVariable(it->ptkType->ToString(str), it->ptkName);
      m_aArgumentsPack.push_back(*it);
    }
    //m_aArgumentsPack.insert(m_aArgumentsPack.end(), aArgs.begin(), aArgs.end());
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

  //GXBOOL CodeParser::ParseStatementAs_Expression(STATEMENT* pStat, NameSet& sNameSet, TKSCOPE* pScope)
  //{
  //  m_nDbgNumOfExpressionParse = 0;
  //  m_aDbgExpressionOperStack.clear();

  //  ASSERT(pScope->begin < pScope->end); // 似乎不应该有相等的情况, "{}" 区间这种是相差一个的
  //  if(pScope->begin + 1 == pScope->end) {
  //    return TRUE;
  //  }

  //  return ParseCodeBlock(pStat->sRoot, sNameSet, *pScope);
  //}

  void CodeParser::DbgDumpSyntaxTree(clStringArrayA* pArray, const SYNTAXNODE* pNode, int precedence, clStringA* pStr, int depth)
  {
    clStringA str[2];
    for(int i = 0; i < 2; i++)
    {
      if(pNode->Operand[i].pTokn) {
        const auto flag = pNode->Operand[i].GetType();
        if(flag == SYNTAXNODE::FLAG_OPERAND_IS_TOKEN) {
          str[i] = pNode->Operand[i].pTokn->ToString();
        }
        else if(flag == SYNTAXNODE::FLAG_OPERAND_IS_NODE) {
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

      if(pNode->mode == SYNTAXNODE::MODE_Chain) {
        strCommand.Append("(Chain)");
      }
      else if(pNode->mode == SYNTAXNODE::MODE_Block) {
        strCommand.Append("(Block)");
      }
      else if(pNode->mode == SYNTAXNODE::MODE_ArrayAssignment) {
        strCommand.Append("(Array)");
      }
      else if(pNode->mode == SYNTAXNODE::MODE_Definition) {
        strCommand.Append("(definition)");
      }
    }
    else
    {
      strCommand.Format("[%s] [%s] [%s]", pNode->pOpcode ? pNode->pOpcode->ToString() : "", str[0], str[1]);
    }

    if(pArray) {
      pArray->push_back(strCommand);
    }
    TRACE("%s\n", strCommand);

    clStringA strOut;
    switch(pNode->mode)
    {
    case SYNTAXNODE::MODE_Undefined: // 解析出错才会有这个
      break;

    case SYNTAXNODE::MODE_FunctionCall: // 函数调用
      strOut.Format("%s(%s)", str[0], str[1]);
      break;

    case SYNTAXNODE::MODE_ArrayAlloc:
      ASSERT(str[1].IsEmpty());
      strOut.Format("%s[]", str[0]);
      break;

    case SYNTAXNODE::MODE_ArrayIndex:
      strOut.Format("%s[%s]", str[0], str[1]);
      break;

    //case SYNTAXNODE::MODE_DefinitionConst:
    //  strOut.Format("const %s %s", str[0], str[1]);
    //  break;

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
    case SYNTAXNODE::MODE_ArrayAssignment:
      strOut.Format("%s={%s}", str[0], str[1]);
      break;

    case SYNTAXNODE::MODE_StructDef:
      strOut.Format("struct %s ", str[0]);
      strOut.Append(str[1]);
      break;

    case SYNTAXNODE::MODE_Flow_DoWhile:
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
  GXBOOL CodeParser::TryKeywords(NameContext& sNameSet, const TKSCOPE& scope, SYNTAXNODE::GLOB* pDesc, TKSCOPE::TYPE* parse_end)
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
      OutputErrorW(front, UVS_EXPORT_TEXT(2181, "没有匹配 if 的非法 else"));
    }
    else if(front == "for") {
      pend = ParseFlowFor(sNameSet, scope, pDesc);
      ASSERT(m_aTokens[pend - 1] == ';' || m_aTokens[pend - 1] == '}');
    }
    else if(front == "if") {
      pend = ParseFlowIf(sNameSet, scope, pDesc, FALSE);
      ASSERT(m_aTokens[pend - 1] == ';' || m_aTokens[pend - 1] == '}');
    }
    else if(front == "while") {
      pend = ParseFlowWhile(sNameSet, scope, pDesc);
      ASSERT(m_aTokens[pend - 1] == ';' || m_aTokens[pend - 1] == '}');
    }
    else if(front == "do") {
      pend = ParseFlowDoWhile(sNameSet, scope, pDesc);
      ASSERT(m_aTokens[pend - 1] == ';' || m_aTokens[pend - 1] == '}');
    }
    else if(front == "typedef") {
      pend = ParseTypedef(sNameSet, scope, pDesc);
      ASSERT(m_aTokens[pend - 1] == ';');
    }
    else if(front == "struct") {
      //pend = ParseStructDefine(scope, pDesc);
      SYNTAXNODE::GLOB sMembers = { 0 };
      SYNTAXNODE::GLOB sVariable = { 0 };
      int nSignatures = 0;
      int nDefinition = 0;
      pend = ParseStructDefinition(sNameSet, scope, &sMembers, &sVariable, &nSignatures, &nDefinition);

      if(sMembers.ptr)
      {
        SYNTAXNODE::GLOB sName;
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
        OutputErrorW(front, UVS_EXPORT_TEXT(2043, "非法 break"));
      }
      eMode = SYNTAXNODE::MODE_Flow_Break;
    }
    else if(front == "continue") {
      if(TEST_FLAG_NOT(sNameSet.allow_keywords, KeywordFilter_continue)) {
        OutputErrorW(front, UVS_EXPORT_TEXT(2044, "非法 continue"));
      }
      eMode = SYNTAXNODE::MODE_Flow_Continue;
    }
    else if(front == "case") {
      if(TEST_FLAG_NOT(sNameSet.allow_keywords, KeywordFilter_case)) {
        OutputErrorW(front, UVS_EXPORT_TEXT(2046, "非法 case"));
      }

      TKSCOPE::TYPE i = scope.begin + 1;
      for(; i < scope.end; i++)
      {
        if(m_aTokens[i] == ':') {
          break;
        }
      }

      if(i == scope.end) {
        OutputErrorW(front, UVS_EXPORT_TEXT(5025, "case 结尾缺少“:”"));
        pend = scope.begin + 1;
      }
      else {
        pend = i + 1;
      }
      eMode = SYNTAXNODE::MODE_Flow_Case;
    }
    else if(front == "discard") {
      eMode = SYNTAXNODE::MODE_Flow_Discard;
    }
    else if(front == "return")
    {
      eMode = SYNTAXNODE::MODE_Return;
    }
    else {
      bret = FALSE;
    }

    while(eMode == SYNTAXNODE::MODE_Flow_Break || eMode == SYNTAXNODE::MODE_Flow_Continue ||
      eMode == SYNTAXNODE::MODE_Flow_Discard || eMode == SYNTAXNODE::MODE_Return)
    {
      SYNTAXNODE::GLOB A = {0}, B = {0};

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
        pend = front.semi_scope + 1;
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

  GXBOOL CodeParser::ParseExpression(SYNTAXNODE::GLOB& glob, NameContext& sNameSet, const TKSCOPE& scope)
  {
    // 解析一条表达式语句或者一个语句块
    ASSERT(scope.begin == scope.end ||
      (m_aTokens[scope.begin].semi_scope == scope.end - 1 && m_aTokens[scope.end - 1] == ';') ||
      (m_aTokens[scope.begin].scope == scope.end - 1 && m_aTokens[scope.begin] == '{' && m_aTokens[scope.end - 1] == '}'));

    if(m_aTokens[scope.begin] == '{') {
      return ParseCodeBlock(glob, sNameSet, scope);
    }
    else if(ParseArithmeticExpression(1, TKSCOPE(scope.begin, scope.end - 1), &glob) == FALSE)
    {
      OutputErrorW(m_aTokens[scope.begin], UVS_EXPORT_TEXT(5013, "表达式解析失败"));
      return FALSE;
    }
    return TRUE;
  }

  //////////////////////////////////////////////////////////////////////////
  // 解析一个代码块
  // 避免每条语句解析都递归
  GXBOOL CodeParser::ParseCodeBlock(SYNTAXNODE::GLOB& glob, NameContext& sNameSet, const TKSCOPE& scope)
  {
    // 保证解析内容在数学区间: [begin, end)
    ASSERT(m_aTokens[scope.begin].scope == scope.end - 1);
    ASSERT(m_aTokens[scope.begin] == '{' && m_aTokens[scope.end - 1] == '}');

    TKSCOPE step_scope(scope.begin + 1, scope.end - 1); // begin会步进, end恒定
    MakeSyntaxNode(&glob, SYNTAXNODE::MODE_Block, NULL, NULL);

    return ParseToChain(glob.pNode->Operand[0], sNameSet, step_scope);
  }

  //////////////////////////////////////////////////////////////////////////
  // 解析一个范围内的所有表达式, 解析的表达式会连接成为chain
  GXBOOL CodeParser::ParseToChain(SYNTAXNODE::GLOB& glob, NameContext& sNameSet, const TKSCOPE& scope)
  {
    SYNTAXNODE::GLOB A;
    SYNTAXNODE::GLOB* pCurrNode = &glob;
    TKSCOPE step_scope = scope;

    while(step_scope.GetSize() > 0)
    {
      A.ptr = NULL;
      
      const TKSCOPE::TYPE pos = TryParseSingle(sNameSet, A, step_scope);
      if(pos == TKSCOPE::npos) {
        return FALSE;
      }
      else if(pos == step_scope.begin + 1) {
        ASSERT(m_aTokens[step_scope.begin] == ';'); // 步进一次只可能是遇到了单独的分号
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

  CodeParser::TKSCOPE::TYPE CodeParser::TryParseSingle(NameContext& sNameSet, SYNTAXNODE::GLOB& glob, const TKSCOPE& step_scope)
  {
    ASSERT(step_scope.GetSize() > 0); // 外部保证解析时不为空
    const auto& front = m_aTokens[step_scope.begin];
    TKSCOPE::TYPE parse_end = TKSCOPE::npos;

    if(step_scope.GetSize() == 1) {
      ASSERT(front == ';');
      return step_scope.end;
    }
    else if(front == '{')
    {
      TKSCOPE sub_block;
      InitTokenScope(sub_block, front, TRUE);

      if(sub_block.end > step_scope.end) {
        OutputErrorW(front, UVS_EXPORT_TEXT(2059, "括号不匹配, 缺少\"%s\"."), GetPairOfBracket(front.marker[0]));
        return TKSCOPE::npos;
      }

      if(ParseCodeBlock(glob, sNameSet, sub_block) == FALSE) {
        return TKSCOPE::npos;
      }
      return sub_block.end;
    }
    else if(TryKeywords(sNameSet, step_scope, &glob, &parse_end)) // FIXME: 这里不能用KeywordMask_All, 要从外部继承进来
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
      OutputErrorW(front, UVS_EXPORT_TEXT(5016, "表达式应该以\';\'结尾."));
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
      OutputErrorW(front, UVS_EXPORT_TEXT(5013, "表达式解析失败"));
      return TKSCOPE::npos;
    }
    return front.semi_scope + 1;
  }

  //////////////////////////////////////////////////////////////////////////

  CodeParser::TKSCOPE::TYPE CodeParser::ParseFlowIf(const NameContext& sParentSet, const TKSCOPE& scope, SYNTAXNODE::GLOB* pDesc, GXBOOL bElseIf)
  {
    // 与 ParseFlowWhile 相似
    NameContext sNameSet_FlowIf(sParentSet);
    SYNTAXNODE::GLOB A = {0}, B = {0};
    GXBOOL bret = TRUE;
    ASSERT(m_aTokens[scope.begin] == "if");

    TKSCOPE sConditional(scope.begin + 2, m_aTokens[scope.begin + 1].scope);
    TKSCOPE sStatement;

    if(sConditional.begin >= scope.end || sConditional.end == -1 || sConditional.end > scope.end)
    {
      // ERROR: if 语法错误
      OutputErrorW(m_aTokens[sConditional.begin], UVS_EXPORT_TEXT(5014, "if 语法错误"));
      return TKSCOPE::npos;
    }

    bret = bret && ParseArithmeticExpression(0, sConditional, &A);


    sStatement.begin = sConditional.end + 1;
    sStatement.end = scope.end;

    sStatement.begin = TryParseSingle(sNameSet_FlowIf, B, sStatement);
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
      }
      else
      {
        sStatement.begin = TryParseSingle(sNameSet_FlowIf, B, sStatement);
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

  CodeParser::TKSCOPE::TYPE CodeParser::ParseFlowWhile(const NameContext& sParentSet, const TKSCOPE& scope, SYNTAXNODE::GLOB* pDesc)
  {
    // 与 ParseFlowIf 相似
    SYNTAXNODE::GLOB A = {0}, B = {0};
    NameContext sNameSet_FlowWhile(sParentSet);
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

    sBlock.end = TryParseSingle(sNameSet_FlowWhile, B, TKSCOPE(sBlock.begin, scope.end));
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
  
  CodeParser::TKSCOPE::TYPE CodeParser::ParseFlowDoWhile(const NameContext& sParentSet, const TKSCOPE& scope, SYNTAXNODE::GLOB* pDesc)
  {
    ASSERT(m_aTokens[scope.begin] == "do");
    // do{...}while(...); 中while()不允许使用类型定义
    NameContext sNameSet_Do(sParentSet);


    if(scope.begin + 1 >= scope.end) {
      // ERROR: do 语法错误
      return TKSCOPE::npos;
    }

    TKSCOPE sConditional;
    TKSCOPE sBlock;
    SYNTAXNODE::GLOB A = {0}, B = {0};


    //if( ! MakeScope(&sBlock, &MAKESCOPE(scope, scope.begin + 2, FALSE, scope.begin + 1, TRUE, 0))) {
    //  return TKSCOPE::npos;
    //}
    InitTokenScope(sBlock, scope.begin + 1, FALSE);

    TKSCOPE::TYPE while_token = sBlock.end + 1;
    
    if(while_token >= scope.end && m_aTokens[while_token] != "while") {
      // ERROR: while 语法错误
      OutputErrorW(while_token, UVS_EXPORT_TEXT(5011, "do...while语法错误, 没有出现预期的\"while\"关键字."));
      return TKSCOPE::npos;
    }

    InitTokenScope(sConditional, while_token + 1, FALSE);

    GXBOOL bret = ParseToChain(B, sNameSet_Do, sBlock);
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

  CodeParser::TKSCOPE::TYPE CodeParser::ParseTypedef(NameContext& sNameSet, const TKSCOPE& scope, SYNTAXNODE::GLOB* pDest)
  {
    // typedef A B;
    SYNTAXNODE::GLOB A = {0}, B = {0};
    const TOKEN& front = m_aTokens[scope.begin];
    ASSERT(front == "typedef");

    if(front.semi_scope - scope.begin < 3 || scope.GetSize() < 3) {
      OutputErrorW(front, UVS_EXPORT_TEXT(5019, "“typedef”语法错误"));
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
      OutputErrorW(*A.pTokn, UVS_EXPORT_TEXT(5020, "“typedef”定义中使用的“%s”不是一个类型"), A.pTokn->ToString(strTypenameW).CStr());
      return TKSCOPE::npos;
    }
    case NameContext::State_DuplicatedType:
    {
      clStringA str;
      OutputErrorW(*A.pTokn, UVS_EXPORT_TEXT(5021, "“typedef”定义的类型“%s”已经存在"), B.pTokn->ToString(str).CStr());
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

  CodeParser::TKSCOPE::TYPE CodeParser::MakeFlowForScope(const TKSCOPE& scope, TKSCOPE* pInit, TKSCOPE* pCond, TKSCOPE* pIter, TKSCOPE* pBlock, SYNTAXNODE::GLOB* pBlockNode)
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

  CodeParser::TKSCOPE::TYPE CodeParser::ParseFlowFor(const NameContext& sParentSet, const TKSCOPE& scope, SYNTAXNODE::GLOB* pDesc)
  {
    NameContext sNameSet_FlowFor(sParentSet);
    TKSCOPE sInitializer, sConditional, sIterator;
    TKSCOPE sBlock;

    SYNTAXNODE::GLOB uInit = {0}, uCond = {0}, uIter = {0}, uBlock = {0}, D;
    
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
    //if(sBlock.end == RTSCOPE::npos)
    //{
    //  ASSERT(m_aTokens[sBlock.begin] == "for"); // MakeFlowForScope 函数保证
    //  sBlock.end = ParseFlowFor(RTSCOPE(sBlock.begin, scope.end), &uBlock);
    //}
    //else

    if(uBlock.ptr == NULL) {
      //ParseExpression(&uBlock, sBlock);
      if(TryParseSingle(sNameSet_FlowFor, uBlock, sBlock) == TKSCOPE::npos) {
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

    //DbgDumpScope("for_2", sConditional, sIterator);
    //DbgDumpScope("for_1", sInitializer, sBlock);

    bret = bret && MakeSyntaxNode(&D, SYNTAXNODE::MODE_Flow_ForRunning, &uCond, &uIter);
    bret = bret && MakeSyntaxNode(&D, SYNTAXNODE::MODE_Flow_ForInit, &uInit, &D);
    bret = bret && MakeSyntaxNode(pDesc, SYNTAXNODE::MODE_Flow_For, &D, &uBlock);
    
    return bret ? sBlock.end : TKSCOPE::npos;
  }

  const CodeParser::StatementArray& CodeParser::GetStatements() const
  {
    return m_aStatements;
  }

  void CodeParser::RelocaleStatements( StatementArray& aStatements )
  {
    for(auto it = aStatements.begin(); it != aStatements.end(); ++it)
    {
      switch(it->type)
      {
      case StatementType_FunctionDecl:
      case StatementType_Function:
        //it->func.pArguments = &m_aArgumentsPack[(GXINT_PTR)it->func.pArguments];
        IndexToPtr(it->func.pArguments, m_aArgumentsPack);
        //IndexToPtr(it->func.pExpression, m_aSubStatements);
        break;

      //case StatementType_Expression:
      //  break;

      //case StatementType_Definition:
      //  break;
      }
    }
  }


  //////////////////////////////////////////////////////////////////////////

  CodeParser* CodeParser::GetSubParser()
  {
    if( ! m_pSubParser) {
      m_pSubParser = new CodeParser(m_pContext, m_pInclude);
    }
    return m_pSubParser;
  }

  CodeParser::T_LPCSTR CodeParser::DoPreprocess(const RTPPCONTEXT& ctx, T_LPCSTR begin, T_LPCSTR end)
  {
    CodeParser* pParse = GetSubParser();
    GXDWORD dwFlags = AttachFlag_NotLoadMessage | AttachFlag_Preprocess;

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
    pParse->GenerateTokens(this);
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
        OutputErrorW(tokens.front(), 9999, __FILEW__, __LINE__);
      }

      clStringW strW = tokens[1].ToString();
      strW.TrimBoth('\"');
      m_pMsg->SetCurrentFilenameW(strW);
    }
    else if(tokens.front() == PREPROCESS_line) {
      clStringW str;
      if(tokens.size() != 2) {
        OutputErrorW(tokens.front(), UVS_EXPORT_TEXT(2005, "#line 应输入行号，却找到“%s”"), _CLTEXT("new line"));
        return end;
      }
      else if(tokens[1].type != TOKEN::TokenType_Integer)
      {
        OutputErrorW(tokens.front(), UVS_EXPORT_TEXT(2005, "#line 应输入行号，却找到“%s”"), tokens[1].ToString(str).CStr());
        return end;
      }

      GXINT nLine = tokens[1].ToString().ToInteger();
      if(nLine > 0)
      {
        m_pMsg->SetCurrentTopLine(0);
        GXINT nCurLine = m_pMsg->LineFromPtr(tokens[1].marker);
        m_pMsg->SetCurrentTopLine(nLine - nCurLine - 1);
      }
      else {
        // ERROR: 行号不正确
      }
    }
    else {
      OutputErrorW(tokens.front(), UVS_EXPORT_TEXT(1021, "无效的预处理器命令 \"%s\"."), clStringW(tokens.front().ToString()));
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
    //m_MacrosSet.insert(strMacroName);

    if(count == 1) {
      OutputErrorW(tokens.front(), UVS_EXPORT_TEXT(2007, "#define 缺少定义."));
      return;
    }
    
    clStringA strMacroName(tokens[1].marker, tokens[1].length);

    if(count == 2) // "#define MACRO" 形
    {
      m_pContext->Macros.insert(clmake_pair(strMacroName, l_m));
    }
    else if(count == 3) // "#define MACRO XXX" 形
    {
      // "#define MACROxxx" 异形, xxx可能是字符串, 符号等
      if(tokens[1].end() == tokens[2].begin()) {
        clStringW str;
        OutputErrorW(tokens[1], UVS_EXPORT_TEXT(2008, "“%s”: 宏定义中的意外"), tokens[2].ToString(str).CStr());
      }

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
          OutputErrorW(tokens[2], UVS_EXPORT_TEXT(2008, "“%s”:宏定义中的意外"), clStringW(tokens[2].ToString()));
          return;
        }

        const int scope_end = tokens[2].scope;
        if(scope_end > 3) // #define MACRO(...) ... 形解析
        {
          for(int i = 3; i < scope_end; i++)
          {
            if((_CL_NOT_(tokens[i].IsIdentifier()) && (i & 1)) ||
              (tokens[i] != ',' && (i & 1) == 0))
            {
              OutputErrorW(tokens[i], UVS_EXPORT_TEXT(2010, "“%s”:宏形参表中的意外"), clStringW(tokens[i].ToString()));
              return;
            }
            if(i & 1) {
              sFormalList.push_back(tokens[i]);
              sFormalList.back().formal_index = sFormalList.size() - 1;
              sFormalList.back().type = TOKEN::TokenType_FormalParams;
            }
          }

#ifdef ENABLE_SYNTAX_VERIFY
          if(_CL_NOT_(Verify_MacroFormalList(sFormalList))) {
            return;
          }
#endif
        }
        l_define = scope_end + 1;
      }
      else {} // #define MACRO ... 形解析

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
    clStringW strPath = m_pMsg->GetFilePathW();
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
        OutputErrorW(aTokens[1], UVS_EXPORT_TEXT(2012, "在\"<\"之后缺少名称."));
        return;
      }
      else if(i == aTokens.size()) {
        OutputErrorW(aTokens[1], UVS_EXPORT_TEXT(2013, "缺少\">\"."));
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
      OutputErrorW(aTokens[0], UVS_EXPORT_TEXT(2006, "\"#include\" 应输入文件名, 缺找到\"%s\""), aTokens[1].ToString(str).CStr());
      return;
    }

    clBuffer* pBuffer = OpenIncludeFile(strPath);
    if(pBuffer == NULL) {
      // ERROR: 无法打开文件
      OutputErrorW(aTokens[0], UVS_EXPORT_TEXT(5003, "无法打开包含文件: \"%s\""), strPath.CStr());
      return;
    }

    ASSERT(m_pMsg); // 应该从m_pParent设置过临时的m_pMsg
    CodeParser parser(m_pContext, m_pInclude);

    m_pMsg->PushFile(strPath);
    m_pMsg->GenerateCurLines((GXLPCSTR)pBuffer->GetPtr(), pBuffer->GetSize());

    parser.Attach((GXLPCSTR)pBuffer->GetPtr(), pBuffer->GetSize(), AttachFlag_NotLoadMessage, strPath);
    parser.GenerateTokens(this);

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

    m_pMsg->PopFile();
  }

  void CodeParser::PP_Undefine(const RTPPCONTEXT& ctx, const TOKEN::Array& aTokens)
  {
    ASSERT(aTokens.front() == "undef");
    if(aTokens.size() == 1) {
      OutputErrorW(aTokens.front(), UVS_EXPORT_TEXT(4006, "#undef 应输入标识符"));
      return;
    }
    else if(aTokens.size() > 2) {
      OutputErrorW(aTokens.front(), UVS_EXPORT_TEXT(4067, "预处理器指令后有意外标记 - 应输入换行符"));
      return;
    }

    //clStringA str = aTokens[1].ToString();
    clStringA strMacroName = aTokens[1].ToString();
    //m_MacrosSet.erase(strMacroName);
    auto it = m_pContext->Macros.find(strMacroName);
    //ASSERT(it != m_pContext->Macros.end()); // 集合里有的话化名表里也应该有

    if(it != m_pContext->Macros.end())
    {
    }
  }

  CodeParser::T_LPCSTR CodeParser::PP_IfDefine(const RTPPCONTEXT& ctx, GXBOOL bNot, const TOKEN::Array& tokens)
  {
    //const auto& tokens = *m_pSubParser->GetTokensArray();
    ASSERT( ! tokens.empty() && (tokens.front() == PREPROCESS_ifdef || tokens.front() == PREPROCESS_ifndef));
    //T_LPCSTR stream_end = GetStreamPtr() + GetStreamCount();

    if(tokens.size() == 1) {
      // ERROR: ifdef 缺少定义
      OutputErrorW(tokens.front(), UVS_EXPORT_TEXT(1016, "#ifdef 应输入标识符."));
      return ctx.stream_end;
    }
    else if(tokens.size() == 2) {
      const GXBOOL bFind = (m_pContext->Macros.find(tokens[1].ToString()) == m_pContext->Macros.end());
      if(( ! bNot && bFind) || (bNot && ! bFind))
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
      CLBREAK; // 没完成
    }
    return ctx.stream_end;
  }

  void CodeParser::PP_Pragma(const TOKEN::Array& aTokens)
  {
    ASSERT(aTokens.front() == PREPROCESS_pragma);
    if(aTokens[1] == PREPROCESS_message)
    {
      if(aTokens[2] != '(') {
        OutputErrorW(aTokens[2], UVS_EXPORT_TEXT(2059, "语法错误 :“%s”"), clStringW(aTokens[2].ToString()));
        return;
      }

      if(aTokens[3].type != TOKEN::TokenType_String) {
        OutputErrorW(aTokens[2], UVS_EXPORT_TEXT(2059, "语法错误 :“%s”"), clStringW(aTokens[2].ToString()));
        return;
      }

      ASSERT(aTokens[2].GetScope() != -1); // 如果能走到这里括号一定是配对的
      clStringW str = aTokens[3].ToString();
      str.TrimBoth('\"');
      str.Append("\r\n");
      m_pMsg->WriteMessageW(FALSE, str);
    }
    else {
      // 不能识别的pragma子指令
      OutputErrorW(aTokens[1], UVS_EXPORT_TEXT(1021, "无效的预处理器命令 “%s”"), clStringW(aTokens[1].ToString()));
    }
  }

  //////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////

  GXBOOL CodeParser::CalculateValue(OPERAND& sOut, const SYNTAXNODE::GLOB* pDesc)
  {
    OPERAND param[2];
    //SYNTAXNODE::GLOB desc[2];
    //desc[0].un = 
    const SYNTAXNODE* pNode = TryGetNode(pDesc);

    if(pNode) {
      param[0].clear();
      param[1].clear();
      SYNTAXNODE::GLOB l_desc;
      for(int i = 0; i < 2; i++)
      {
        if( ! pNode->Operand[i].ptr) {
          continue;
        }
        //l_desc.flag = pNode->GetOperandType(i);
        l_desc.ptr = pNode->Operand[i].ptr;
        CalculateValue(param[i], &l_desc);
      }
    }
    else {
      sOut.pToken = pDesc->pTokn;
      return TRUE;
    }

    if(pNode->mode == SYNTAXNODE::MODE_Opcode)
    {
      for(int i = 0; i < 2; i++) {
        if(param[i].v.rank == VALUE::Rank_Undefined && param[i].pToken) {
          param[i].v.set(*param[i].pToken);
        }
      }
      sOut.Calculate(*pNode->pOpcode, param);
    }
    else if(pNode->mode == SYNTAXNODE::MODE_FunctionCall)
    {
      if(*(pNode->Operand[0].pTokn) == PREPROCESS_defined)
      {
        ASSERT(pNode->Operand[0].GetType() == SYNTAXNODE::FLAG_OPERAND_IS_TOKEN);

        if(pNode->Operand[1].GetType() != SYNTAXNODE::FLAG_OPERAND_IS_TOKEN) {
          OutputErrorW(*pNode->Operand[0].pTokn, UVS_EXPORT_TEXT(2004, "应输入“defined(id)”"));
          return FALSE;
        }

        auto it = m_pContext->Macros.find(pNode->Operand[1].pTokn->ToString());

        sOut.v.rank = VALUE::Rank_Signed64;
        sOut.v.nValue64 = (it != m_pContext->Macros.end());
      }
      else {
        // ERROR: 类函数调用只能是defined
      }
    }

    return TRUE;
  }

  //////////////////////////////////////////////////////////////////////////

  CodeParser::T_LPCSTR CodeParser::PP_If(const RTPPCONTEXT& ctx, CodeParser* pParser)
  {
    SYNTAXNODE::GLOB sDesc = {0};
    TKSCOPE scope(1, pParser->m_aTokens.size());
    if( ! pParser->ParseArithmeticExpression(0, scope, &sDesc)) {
      // ERROR: 无法解析表达式
      OutputErrorW(pParser->m_aTokens.front(), UVS_EXPORT_TEXT(5004, "无法解析#if的条件表达式"));
      return ctx.iter_next.marker;
    }


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
      TRACE("#if %s\n", result.v.ToString());

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
        OutputErrorW(begin, UVS_EXPORT_TEXT(1018, "意外的 #elif"));
        return end;
      }
      else if(session == PPCondRank_else) {
        OutputErrorW(begin, UVS_EXPORT_TEXT(1019, "意外的 #else"));
        return end;
      }
    }

    UINT depth = 0;
    T_LPCSTR p = begin;
    for(; p < end; ++p)
    {
      if(*p != '\n') {
        continue;
      }

      if((p = Macro_SkipGaps(p, end)) >= end) {
        break;
      }

      if(*p != '#') {
        continue;
      }

      if((p = Macro_SkipGaps(p, end)) >= end) {
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
          OutputErrorW(p, UVS_EXPORT_TEXT(1018, "意外的 #elif"));
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

        pParse->Attach(p, (clsize)line_end - (clsize)p, AttachFlag_NotLoadMessage, NULL);
        pParse->GenerateTokens(this);
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
        OutputErrorW(p, UVS_EXPORT_TEXT(1021, "无效的预处理器命令 \"%s\"."), str);
      }
    }
    
    for(; *p != '\n' && p < end; p++);
    if(p == end)
    {
      OutputErrorW(begin, UVS_EXPORT_TEXT(1004, "意外的文件结束"));
      return end;
    }
    return (p + 1);    
  }

  void CodeParser::PP_UserError(T_LPCSTR position, const clStringW& strText)
  {
    OutputErrorW(position, UVS_EXPORT_TEXT(1189, "#error : %s"), strText);
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
      str.Append(m_pMsg->GetFilenameW());
      str.Append("\"");

      token.type = TOKEN::TokenType_String;
      token.Set(m_pContext->Strings, str);
      return TRUE;
    }
    else if(token == MACRO_LINE)
    {
      str.AppendInteger32(m_pMsg->LineFromPtr(line_num.marker));
      token.type = TOKEN::TokenType_Integer;
      token.Set(m_pContext->Strings, str);
      return TRUE;
    }
    return FALSE;
  }

  CodeParser::T_LPCSTR CodeParser::Macro_SkipGaps( T_LPCSTR p, T_LPCSTR end )
  {
    do {
      p++;
    } while ((*p == '\t' || *p == 0x20) && p < end);
    return p;
  }

  GXBOOL CodeParser::CompareString(T_LPCSTR str1, T_LPCSTR str2, size_t count)
  {
    GXBOOL bSame = GXSTRNCMP(str1, str2, count) == 0;
    const TChar& c = str1[count];
    ASSERT(c != '\0');
    return bSame && (&c == m_pEnd || c == '\t' || c == 0x20 || c == '\r' || c == '\n');
  }

  //void CodeParser::OutputErrorW( GXSIZE_T offset, GXUINT code, ... )
  //{
  //  va_list  arglist;
  //  va_start(arglist, code);
  //  m_pMsg->VarWriteErrorW(TRUE, offset, code, arglist);
  //  va_end(arglist);
  //}

  //////////////////////////////////////////////////////////////////////////
  void CodeParser::OutputErrorW(GXUINT code, ...)
  {
    const char* ptr = NULL;
    for (auto it = m_aTokens.rbegin(); it != m_aTokens.rend(); ++it) {
      if(it->pContainer) {
        ptr = it->marker;
      }
    }

    va_list arglist;
    va_start(arglist, code);
    m_pMsg->VarWriteErrorW(TRUE, ptr, code, arglist);
    va_end(arglist);
    return;
  }

  void CodeParser::OutputErrorW(const TOKEN& token, GXUINT code, ...)
  {
    va_list  arglist;
    va_start(arglist, code);
    if(token.bPhony) {
      auto it = m_PhonyTokenDict.find(&token - &m_aTokens.front());
      if(it == m_PhonyTokenDict.end()) {
        m_pMsg->VarWriteErrorW(TRUE, (GXSIZE_T)0, code, arglist);
      }
      else {
        m_pMsg->VarWriteErrorW(TRUE, it->second.ori_marker, code, arglist);
      }
    }
    else {
      m_pMsg->VarWriteErrorW(TRUE, token.marker, code, arglist);
    }
    va_end(arglist);
  }

  void CodeParser::OutputErrorW(T_LPCSTR ptr, GXUINT code, ...)
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

#ifdef ENABLE_SYNTAX_VERIFY

#if 0
  const TYPEDESC2* CodeParser::Verify_Type(const TOKEN& tkType)
  {
    const TYPEDESC2* pTypeDesc = GetType(tkType);
    return pTypeDesc;
  }

  const TYPEDESC2* CodeParser::Verify_Struct(const TOKEN& tkType, const NameSet* pNameSet)
  {
    clStringA str;
    tkType.ToString(str);
    //TYPEDESC sType = {0};
    /*
    for(auto it = m_aStatements.begin(); it != m_aStatements.end(); ++it)
    {
      if(it->type == StatementType_Struct)
      {
        if(str == it->stru.szName) {
          sType.cate = TYPEDESC::TypeCate_Struct;
          sType.maxC = 1;
          sType.maxR = 1;
          sType.name = it->stru.szName;
          return &(*m_TypeSet.insert(sType).first);
        }
      }
    }
    */

    if(pNameSet)
    {
      return pNameSet->GetType(str);
    }

    return NULL;
  }
#endif

  GXBOOL CodeParser::Verify_MacroFormalList(const MACRO_TOKEN::List& sFormalList)
  {
    clStringW str;
    if(sFormalList.size() <= 1)
    {
      if(sFormalList.front().IsIdentifier() == FALSE) {
        OutputErrorW(sFormalList.front(), UVS_EXPORT_TEXT(2010, "“%s”: 宏形参表中的意外"), sFormalList.front().ToString(str).CStr());
        return FALSE;
      }
      return TRUE;
    }

    for(auto it1 = sFormalList.begin(); it1 != sFormalList.end(); ++it1)
    {
      if(it1->IsIdentifier() == FALSE) {
        OutputErrorW(*it1, UVS_EXPORT_TEXT(2010, "“%s”: 宏形参表中的意外"), it1->ToString(str).CStr());
        return FALSE;
      }

      auto it2 = it1;
      for(++it2; it2 != sFormalList.end(); ++it2)
      {
        if(*it1 == *it2) {
          OutputErrorW(*it2, UVS_EXPORT_TEXT(2009, "宏形式“%s”重复使用"), it2->ToString(str).CStr());
          return FALSE;
        }
      }
    }
    return TRUE;
  }

  GXBOOL CodeParser::Verify_VariableDefinition(NameContext& sNameSet, const SYNTAXNODE* pNode)
  {
    ASSERT(pNode->mode == SYNTAXNODE::MODE_Definition); // 只检查定义

    if(pNode->Operand[0].IsToken())
    {
      if(*pNode->Operand[0].pTokn == "const") {
        return Verify_VariableDefinition(sNameSet, pNode->Operand[1].pNode);
      }
    }

    const TOKEN& tkVar = pNode->Operand[1].IsToken()
      ? *pNode->Operand[1].pTokn
      : pNode->Operand[1].pNode->GetAnyTokenAB();

    ASSERT(pNode->Operand[0].IsToken()); // 外面的拆解保证不会出现这个

    clStringA strType, strVar;
    pNode->Operand[0].pTokn->ToString(strType);
    tkVar.ToString(strVar);

    DbgDumpSyntaxTree(NULL, pNode, 0, NULL, 1);
   
#if 0
    // 检查类型定义
    const TYPEDESC* pType = NULL;
    if(_CL_NOT_(pNode->Operand[0].pTokn->IsIdentifier()) || (
      _CL_NOT_(pType = Verify_Type(*pNode->Operand[0].pTokn)) &&
      _CL_NOT_(pType = Verify_Struct(*pNode->Operand[0].pTokn, &sNameSet))) )
    {
      clStringW str;
      OutputErrorW(*pNode->Operand[0].pTokn,
        UVS_EXPORT_TEXT(5024, "错误的数据类型 : \"%s\""),
        pNode->Operand[0].pTokn->ToString(str).CStr());

      return FALSE;
    }
#endif

    // 检查变量名
    clStringW str;
    const TYPEDESC* pType = NULL;
    if(tkVar.IsIdentifier())
    {
      //clStringA strA;
      if((pType = sNameSet.RegisterVariable(strType, &tkVar)) == NULL)
      {
        switch(sNameSet.GetLastState())
        {
        case NameContext::State_TypeNotFound:
        {
          str = strType;
          OutputErrorW(tkVar, UVS_EXPORT_TEXT(5012, "“%s”: 类型未定义"), str.CStr());
          return FALSE;
        }
        case NameContext::State_DuplicatedVariable:
        {
          OutputErrorW(tkVar, UVS_EXPORT_TEXT(2371, "“%s”: 重定义"), tkVar.ToString(str).CStr());
          return FALSE;
        }
        case NameContext::State_DefineAsType:
        {
          OutputErrorW(tkVar, UVS_EXPORT_TEXT(5013, "“%s”: 变量已经被定义为类型"), tkVar.ToString(str).CStr());
          return FALSE;
        }
        default:
          CLBREAK; // 预期之外的状态
          break;
        }
      }
    }
    else
    {
      OutputErrorW(tkVar, UVS_EXPORT_TEXT(3000, "预期是一个变量名 : \"%s\""), tkVar.ToString(str).CStr());
      return FALSE;
    }

    return pNode->Operand[1].IsToken() ? TRUE
      : Verify2_VariableExpr(sNameSet, *pNode->Operand[0].pTokn, pType, *pNode->Operand[1].pNode);
  }

  GXBOOL CodeParser::Verify2_VariableExpr(NameContext& sNameSet, const TOKEN& tkType, const TYPEDESC* pType, const SYNTAXNODE& rNode)
  {
    GXBOOL result = TRUE;
    RecursiveNode<const SYNTAXNODE>(this, &rNode, [this, &result, &pType, &sNameSet]
    (const SYNTAXNODE* pNode, int depth) -> GXBOOL
    {
      if(pNode->CompareOpcode('=') || pNode->mode == SYNTAXNODE::MODE_ArrayAssignment) {
        if(pNode->Operand[1].IsToken())
        {
          const TOKEN& tkRightValue = *pNode->Operand[1].pTokn;
          if(//pType->cate == TYPEDESC::TypeCate_Struct ||
            (pType->cate == TYPEDESC::TypeCate_Numeric && tkRightValue.IsNumeric() == FALSE) ||
            (pType->cate == TYPEDESC::TypeCate_String && tkRightValue.type != TOKEN::TokenType_String) )
          {
            clStringW str;
            clStringW str2(pType->name);
            OutputErrorW(tkRightValue, UVS_EXPORT_TEXT(5025, "无法将\"%s\"转换为\"%s\"类型"),
              tkRightValue.ToString(str).CStr(), str2.CStr());
            return (result = FALSE);
          }
          else if(pType->cate == TYPEDESC::TypeCate_Struct)
          {
            const TYPEDESC* pVarTypeDesc = sNameSet.GetVariable(&tkRightValue);
            if(pType->cate != pVarTypeDesc->cate || pType->name != pVarTypeDesc->name)
            {
              clStringW str;
              clStringW str2(pType->name);
              OutputErrorW(tkRightValue, UVS_EXPORT_TEXT(5025, "无法将\"%s\"转换为\"%s\"类型"),
                tkRightValue.ToString(str).CStr(), str2.CStr());
            }
          }

          if(pType->cate == TYPEDESC::TypeCate_Numeric)
          {
            VALUE v;
            VALUE::State s = v.set(tkRightValue);
            if(TEST_FLAG(s, VALUE::State_ErrorMask)) {
              clStringW str;
              if(TEST_FLAG(s, VALUE::State_IllegalNumber))
              {
                OutputErrorW(tkRightValue, UVS_EXPORT_TEXT(2041, "非法的数字 : “%s”"), tkRightValue.ToString(str).CStr());
              }
              else
              {
                OutputErrorW(tkRightValue, UVS_EXPORT_TEXT(2021, "应输入数值, 而不是“%s”"), tkRightValue.ToString(str).CStr());
              }
              return (result = FALSE);
            }
          }
        }
        else if(pNode->Operand[1].IsNode())
        {
          const NODE_CALC* pnodeRightValue = static_cast<const NODE_CALC*>(pNode->Operand[1].pNode);
          if(pType->cate == TYPEDESC::TypeCate_Numeric)
          {
            VALUE v;
            VALUE::State s = pnodeRightValue->Calcuate(sNameSet, v);
            if(TEST_FLAG(s, VALUE::State_ErrorMask))
            {
              OutputErrorW(pnodeRightValue->GetAnyTokenAB(), UVS_EXPORT_TEXT(5026, "无法计算数学表达式"));
              return (result = FALSE);
            }
            CLNOP
          }
          else if(pType->cate == TYPEDESC::TypeCate_String)
          {
            // token解析会自动连接字符串, 所以不会出现两个token都是字符串的情况
            OutputErrorW(pnodeRightValue->GetAnyTokenAB(), UVS_EXPORT_TEXT(5027, "字符串表达式语法错误"));
            return (result = FALSE);
          }
          else if(pType->cate == TYPEDESC::TypeCate_Struct)
          {
            if(pNode->mode == SYNTAXNODE::MODE_ArrayAssignment)
            {
              OutputErrorW(pnodeRightValue->GetAnyTokenAB(), UVS_EXPORT_TEXT(5028, "不支持结构体赋值"));
              return (result = FALSE);
            }
          }
        }
        else {
          OutputErrorW(*pNode->pOpcode, UVS_EXPORT_TEXT(5022, "赋值错误, \"=\" 后应该有表达式"));
          return (result = FALSE);
        }
        return FALSE;
      }
      else if(pNode->mode == SYNTAXNODE::MODE_ArrayIndex)
      {
        //if(pNode->Operand[1].IsToken())
        //{
        //}
        //else if(pNode->Operand[1].IsNode())
        //{
        //  CLBREAK;
        //}
        //else {
        //  CLBREAK;
        //}
      }
      else {
        if(depth <= 1 && pNode->mode == SYNTAXNODE::MODE_Opcode && pNode->pOpcode == NULL) {
          CLNOP
        }
      }
      return TRUE;
    });

    return result;
  }

  GXBOOL CodeParser::Verify_Block(const SYNTAXNODE* pNode, const NameContext* pParentSet)
  {
    GXBOOL result = TRUE;
    NameContext sNameSet(pParentSet);
    RecursiveNode<const SYNTAXNODE>(this, pNode, [this, &result, &sNameSet]
    (const SYNTAXNODE* pNode, int depth) -> GXBOOL
    {
      if(pNode->mode == SYNTAXNODE::MODE_Flow_For)
      {
        NameContext sFlowForSet(&sNameSet);
        if(pNode->Operand[0].IsNode() && _CL_NOT_(Verify_Block(pNode->Operand[0].pNode, &sFlowForSet)))
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
        if(Verify_VariableDefinition(sNameSet, pNode) == FALSE) {
          result = FALSE;
        }
        return FALSE; // 不再递归
      }
      else if(pNode->mode == SYNTAXNODE::MODE_StructDef)
      {
        clStringA str;
        // 注册结构体类型
        //sNameSet.RegisterType(pNode->Operand[0].pTokn->ToString(str), TYPEDESC::TypeCate_Struct);
        return FALSE;
      }
      else if(pNode->mode == SYNTAXNODE::MODE_Opcode)
      {
        if(pNode->pOpcode)
        {
          if(*pNode->pOpcode == '=')
          {
            if(Verify2_LeftValue(sNameSet, pNode->Operand[0], *pNode->pOpcode) == FALSE) {
              result = FALSE;
            }
          }
        }
      }
      return TRUE;
    });

    return result;
  }

  GXBOOL CodeParser::Verify_StructMember(const NameContext& sParentSet, const SYNTAXNODE& rNode)
  {
    GXBOOL result = TRUE;
    NameContext sSet(sParentSet);
    RecursiveNode<const SYNTAXNODE>(this, &rNode, [this, &result, &sSet](const SYNTAXNODE* pNode, int depth) -> GXBOOL
    {
      if(pNode->mode == SYNTAXNODE::MODE_Definition)
      {
        const SYNTAXNODE::GLOB& Op = pNode->Operand[1];
        clStringA strType;
        pNode->Operand[0].pTokn->ToString(strType);

        if(Op.IsToken()) {
          clStringA str;
          if(sSet.RegisterVariable(strType, Op.pTokn/*->ToString(str)*/) == NULL)
          {
            clStringW str;
            switch(sSet.GetLastState())
            {
            case NameContext::State_DuplicatedVariable:
              OutputErrorW(*Op.pTokn, UVS_EXPORT_TEXT(2030, "“%s”: 结构成员重定义"), Op.pTokn->ToString(str));
              break;
            case NameContext::State_TypeNotFound:
              str = strType;
              OutputErrorW(*Op.pTokn, UVS_EXPORT_TEXT(2062, "“%s”: 意外的类型"), str.CStr());
              break;
            default:
              CLBREAK; // 没有处理的错误
              break;
            }

            result = FALSE;
          }
        }
        else if(Op.IsNode()) {
          auto& tk = Op.pNode->GetAnyTokenAB();
          clStringA str;
          if(sSet.RegisterVariable(strType, &tk/*.ToString(str)*/) == NULL)
          {
            clStringW str;
            switch(sSet.GetLastState())
            {
            case NameContext::State_DuplicatedVariable:
              OutputErrorW(tk, UVS_EXPORT_TEXT(2030, "“%s”: 结构成员重定义"), tk.ToString(str));
              break;
            case NameContext::State_TypeNotFound:
              str = strType;
              OutputErrorW(*Op.pTokn, UVS_EXPORT_TEXT(2062, "“%s”: 意外的类型"), str.CStr());
              break;
            default:
              CLBREAK; // 没有处理的错误
              break;
            }
            result = FALSE;
          }
        }
      }
      return TRUE;
    });
    return result;
  }

  GXBOOL CodeParser::Verify2_LeftValue(const NameContext& sNameSet, const SYNTAXNODE::GLOB& left_glob, const TOKEN& opcode)
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
    if(left_glob.IsToken())
    {
      if(sNameSet.GetVariable(left_glob.pTokn/*->ToString(strA)*/) == NULL)
      {
        //strW = strA;
        left_glob.pTokn->ToString(strW);
        OutputErrorW(*left_glob.pTokn, UVS_EXPORT_TEXT(2065, "“%s”: 未声明的标识符"), strW.CStr());
        return FALSE;
      }
      return TRUE;
    }
    else if(left_glob.IsNode())
    {
      const TYPEDESC* pDesc = sNameSet.GetMember(left_glob.pNode);
      if(pDesc == NULL)
      {
        OutputErrorW(left_glob.pNode->GetAnyTokenAB(), UVS_EXPORT_TEXT(5023, "不明确的成员变量"));
        return FALSE;
      }
    }    
    else {
      OutputErrorW(opcode, UVS_EXPORT_TEXT(5010, "“=”前缺少左值"));
      return FALSE;
    }



    return TRUE;
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

  NameContext::NameContext()
    : m_pCodeParser(NULL)
    , m_pParent(NULL)
    , m_eLastState(State_Ok)
    , allow_keywords(KeywordFilter_All)
  {   
  }

  NameContext::NameContext(const NameContext* pParent)
    : m_pCodeParser(pParent->m_pCodeParser)
    , m_pParent(pParent)
    , m_eLastState(State_Ok)
    , allow_keywords(KeywordFilter_All)
  {
  }

  NameContext::NameContext(const NameContext& sParent)
    : m_pCodeParser(sParent.m_pCodeParser)
    , m_pParent(&sParent)
    , m_eLastState(State_Ok)
    , allow_keywords(KeywordFilter_All)
  {
  }

  void NameContext::Cleanup()
  {
    m_TypeMap.clear();
    m_VariableMap.clear();
  }
  
  GXBOOL NameContext::TestIntrinsicType(TYPEDESC* pOut, const clStringA& strType)
  {
    ASSERT(pOut->pDesc == NULL && pOut->pMemberNode == NULL); // 强制外部初始化

    // 内置结构体
    for(int i = 0; s_aIntrinsicStruct[i].name; i++)
    {
      if(strType == s_aIntrinsicStruct[i].name) {
        pOut->cate = TYPEDESC::TypeCate_Struct;
        pOut->name = strType;
        pOut->pDesc = &s_aIntrinsicStruct[i];
        return TRUE;
        //it = m_TypeMap.insert(clmake_pair(strType, td)).first;
      }
    }

    // 内置基础类型
    for(int i = 0; s_aBaseType[i].name; i++)
    {
      if(strType == s_aBaseType[i].name)
      {
        pOut->cate = TYPEDESC::TypeCate_Numeric;
        pOut->name = strType;
        pOut->pDesc = &s_aBaseType[i];
        return TRUE;
      }
    }

    // 字符串类型
    if(strType == s_szString) {
      pOut->cate = TYPEDESC::TypeCate_String;
      pOut->name = strType;
      pOut->pDesc = NULL;
      return TRUE;
    }

    return FALSE;
  }

  NameContext* NameContext::GetRoot()
  {
    const NameContext* pSet = this;
    while(pSet->m_pParent)
    {
      pSet = pSet->m_pParent;
    }
    return reinterpret_cast<NameContext*>(reinterpret_cast<size_t>(pSet));
  }

  void NameContext::SetParser(CodeParser* pCodeParser)
  {
    m_pCodeParser = pCodeParser;
  }

  const TYPEDESC* NameContext::RegisterVariable(const clStringA& strType, const TOKEN* ptrVariable)
  {
    const TYPEDESC* pDesc = GetType(strType);
    if(pDesc == NULL)
    {
      // 如果注册vec4, 那么vec3和vec2都会被注册
      clStringA strTypeRec = strType;
      ch* pc = &strTypeRec.Back();
     
      do
      {
        TYPEDESC td = { TYPEDESC::TypeCate_Empty };

        // TODO: 如果是vec4类型, 则同时也能返回vec3和vec2
        if(TestIntrinsicType(&td, strTypeRec) == FALSE) {
          m_eLastState = State_TypeNotFound;
          return NULL;
        }

        // 内置类型由根节点持有
        pDesc = &(GetRoot()->m_TypeMap.insert(clmake_pair(strTypeRec, td)).first->second);
        *pc -= 1;
      } while(*pc == '2' || *pc == '3');
    }

    if(ptrVariable)
    {
      clStringA strVari;
      ptrVariable->ToString(strVari);
      if(GetType(strVari)) {
        m_eLastState = State_DefineAsType;
        return NULL;
      }

      auto result = m_VariableMap.insert(clmake_pair(ptrVariable, pDesc));
      if(result.second) {
        // 添加成功, 返回type描述
        m_eLastState = State_Ok;
        return result.first->second;
      }

      m_eLastState = State_DuplicatedVariable;
      return NULL;
    }
    return pDesc;
  }

  NameContext::State NameContext::GetLastState() const
  {
    return m_eLastState;
  }

  const TYPEDESC* NameContext::GetMember(const SYNTAXNODE* pNode) const
  {
    const TYPEDESC* pTypeDesc = NULL;
    if(pNode->Operand[0].IsToken())
    {
      pTypeDesc = GetVariable(pNode->Operand[0].pTokn);
    }
    else
    {
      ASSERT(pNode->mode == SYNTAXNODE::MODE_Opcode && pNode->CompareOpcode('.'));
      pTypeDesc = GetMember(pNode->Operand[0].pNode);
    }
    ASSERT(pNode->Operand[1].IsToken());

    clStringA strTypename;
    
    if(pTypeDesc->GetMemberTypename(strTypename, pNode->Operand[1].pTokn))
    {
      pTypeDesc = GetType(strTypename);
    }
    else
    {
      clStringW str1, str2;
      pTypeDesc = NULL;
      m_pCodeParser->OutputErrorW(*pNode->Operand[1].pTokn, UVS_EXPORT_TEXT2(2039, "“%s”: 不是“%s”的成员", m_pCodeParser),
        pNode->Operand[1].pTokn->ToString(str1).CStr(), pNode->Operand[0].pTokn->ToString(str1).CStr());
    }

    return pTypeDesc;
  }

  GXBOOL NameContext::RegisterStruct(const TOKEN* ptkName, const SYNTAXNODE* pMemberNode)
  {
    ASSERT(pMemberNode == NULL || pMemberNode->mode == SYNTAXNODE::MODE_Block);
    TYPEDESC td = {TYPEDESC::TypeCate_Empty};
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

  const TYPEDESC* NameContext::GetVariable(const TOKEN* ptkName) const
  {
    auto it = m_VariableMap.find(TokenPtr(ptkName));
    return it != m_VariableMap.end()
      ? it->second
      : (m_pParent ? m_pParent->GetVariable(ptkName) : NULL);
  }

  const TYPEDESC* NameContext::GetType(const clStringA& strType) const
  {
    auto it = m_TypeMap.find(strType);
    return it != m_TypeMap.end()
      ? &it->second
      : (m_pParent ? m_pParent->GetType(strType) : NULL);
  }

  NameContext::State NameContext::TypeDefine(const TOKEN* ptkOriName, const TOKEN* ptkNewName)
  {
    clStringA strOriName;
    TYPEDESC td = { TYPEDESC::TypeCate_Empty };
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

  VALUE::State NODE_CALC::Calcuate(const NameContext& sNameSet, VALUE& value_out) const
  {
    VALUE p[2];
    VALUE::State s = VALUE::State_OK;
    if(mode == MODE_FunctionCall)
    {
      value_out.SetOne();
      return VALUE::State_Call;
    }
    else if(mode == MODE_Opcode)
    {
      
      ASSERT(pOpcode != NULL);
      if(*pOpcode == '.')
      {
        //-----------------------------------------
        //<ID_0028>        [.] [surf] [ff3]
        //<ID_0029>      [.] [surf.ff3]<ID_0028> [fx]
        //<ID_0030>    [.] [surf.ff3.fx]<ID_0029> [color]
        //<ID_0031>  [.] [surf.ff3.fx.color]<ID_0030> [x]
        //surf.ff3.fx.color.x
        //-----------------------------------------

        clStringA strTypename;
        const TYPEDESC* pTypeDesc = sNameSet.GetMember(this);

        if(pTypeDesc->cate == TYPEDESC::TypeCate_Numeric)
        {
          value_out.SetOne();
          return VALUE::State_OK;
        }
        else {
          CLBREAK; // 不是数学类型
        }
      }
    }

    for(int i = 0; i < 2; i++)
    {
      if(Operand[i].IsNode()) {
        s = static_cast<const NODE_CALC*>(Operand[i].pNode)->Calcuate(sNameSet, p[i]);
      }
      else if(Operand[i].IsToken()) {
        if(Operand[i].pTokn->IsNumeric()) {
          s = p[i].set(*Operand[i].pTokn);
        }
        else if(Operand[i].pTokn->IsIdentifier()) {
          p[i].SetOne(); // 标识符用临时值1
          s = VALUE::State_Identifier;
        }
      }
      else {
        p[i].SetZero();
      }

      if(s < VALUE::State_OK) {
        return s;
      }
    }

    if(pOpcode == NULL) {
      return VALUE::State_BadOpcode;
    }

    s |= value_out.Calculate(*pOpcode, p[0], p[1]);
    return s;

  }

  //////////////////////////////////////////////////////////////////////////

  GXBOOL TYPEDESC::GetMemberTypename(clStringA& strTypename, const TOKEN* ptkMember) const
  {
    strTypename.Clear();

    if(pMemberNode)
    {
      ASSERT(pDesc == NULL);
      const TYPEDESC* pDesc = NULL;

      // 确定符合结构体描述
      ASSERT(pMemberNode->mode == SYNTAXNODE::MODE_Block &&
        pMemberNode->Operand[0].IsNode() &&
        pMemberNode->Operand[1].IsToken() && *pMemberNode->Operand[1].pTokn == ';');

      RecursiveNode<const SYNTAXNODE>(NULL, pMemberNode->Operand[0].pNode, [&pDesc, &strTypename, &ptkMember]
      (const SYNTAXNODE* pNode, int depth)-> GXBOOL
      {
        if(strTypename.IsNotEmpty()) // 找到类型后尽快结束迭代
        {
          return FALSE;
        }
        else if(pNode->mode == SYNTAXNODE::MODE_Definition)
        {
          if(pNode->Operand[1].IsToken()) {
            if(*pNode->Operand[1].pTokn == *ptkMember) {
              ASSERT(pNode->Operand[0].IsToken());
              pNode->Operand[0].pTokn->ToString(strTypename);
              return FALSE;
            }
          }
          else if(pNode->Operand[1].IsNode()) {
            if(pNode->Operand[1].pNode->GetAnyTokenAB() == *ptkMember) {
              ASSERT(pNode->Operand[0].IsToken());
              pNode->Operand[0].pTokn->ToString(strTypename);
              return FALSE;
            }
          }
          else {
            CLBREAK;
          }
        }
        return TRUE;
      });
      //return pDesc;
      return strTypename.IsNotEmpty();
    }
    else if(pDesc)
    {
      ASSERT(pMemberNode == NULL);
      for(size_t i = 0; i < pDesc->count; i++)
      {
        if(*ptkMember == pDesc->list[i].name) {
          strTypename = pDesc->list[i].type;
          return TRUE;
        }
      }

      ASSERT(clstd::strlenT(pDesc->set0) == clstd::strlenT(pDesc->set1)); // 保证这两个长度一致

      if(MatchScaler(ptkMember, pDesc->set0) || MatchScaler(ptkMember, pDesc->set1))
      {
        strTypename = pDesc->name;
        strTypename.Back() = '0' + ptkMember->length;
        return TRUE;
      }
    }
    return FALSE;
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

} // namespace UVShader
