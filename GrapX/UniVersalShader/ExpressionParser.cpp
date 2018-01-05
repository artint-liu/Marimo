#include <grapx.h>
#include <clUtility.h>
#include <Smart/SmartStream.h>
#include <clTokens.h>
#include <clStringSet.h>
#include "ArithmeticExpression.h"
#include "ExpressionParser.h"

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

  //////////////////////////////////////////////////////////////////////////

  DefaultInclude s_DefaultInclude;

  CodeParser::CodeParser(PARSER_CONTEXT* pContext, Include* pInclude)
    : m_pContext(pContext)
    , m_pSubParser(NULL)
    , m_dwState(0)
    , m_pParent(NULL)
    , m_pInclude(pInclude ? pInclude : &s_DefaultInclude)
  {
    if(pContext) {
      pContext->nRefCount++;
    }

    SetIteratorCallBack(CodeParser::IteratorProc, 0);
    InitPacks();
  }

  CodeParser::~CodeParser()
  {
    if( ! (--m_pContext->nRefCount)) {
      delete m_pContext;
    }
    m_pContext = NULL;

    SAFE_DELETE(m_pSubParser);
    ErrorMessage::Destroy(m_pMsg);

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
    ClearSyntaxNodePool();

    STRUCT_MEMBER member = {NULL};
    m_aMembersPack.clear();
    m_aMembersPack.push_back(member);

    FUNCTION_ARGUMENT argument = {InputModifier_in};
    m_aArgumentsPack.clear();
    m_aArgumentsPack.push_back(argument);

    STATEMENT stat = {StatementType_Empty};
    m_aSubStatements.push_back(stat);
  }

  void CodeParser::Cleanup()
  {
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
        m_pMsg->LoadErrorMessageW(L"uvsmsg.txt");
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
        ERROR_MSG_C2014_预处理器命令必须作为第一个非空白空间启动;
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
        it = ctx.iter_next;
      }
      else if(it == PREPROCESS_error)
      {
        clStringW str(it.marker + it.length, ctx.ppend - (it.marker + it.length));
        pThis->OutputErrorW(it.marker, E1189_用户定义错误_vs, str);
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

#ifdef USE_CLSTD_TOKENS
#else
    SetTriggerCallBack(MultiByteOperatorProc, (u32_ptr)&token);
#endif // USE_CLSTD_TOKENS

    SetIteratorCallBack(IteratorProc, (u32_ptr)&token);

    PairStack sStack[countof(s_PairMark)];


    auto it = begin();
    token.Set(it);

    int EOE = m_aTokens.size(); // End Of Expression

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
        OutputErrorW(E2121_无效的井号_可能是宏扩展);
      }

      const int c_size = (int)m_aTokens.size();

      // 如果是 -,+ 检查前一个符号是不是操作符或者括号，如果是就认为这个 -,+ 是正负号
      if((it == '-' || it == '+') && ! m_aTokens.empty())
      {        
        const auto& l_back = m_aTokens.back();

        // 一元操作符，+/-就不转换为正负号
        // '}' 就不判断了 { something } - abc 这种格式应该是语法错误
        if(l_back.precedence != 0 && l_back != ')' && l_back != ']' && ( ! l_back.unary)) {
          const auto& p = s_plus_minus[(int)(it.marker[0] - '+')];
          token.SetArithOperatorInfo(p);
        }
      }

      
      // 只是清理
      //l_token.ClearMarker();
      //l_token.ClearArithOperatorInfo();


      // 符号配对处理
      // ...={...}这种情况不更新EOE
      if(MarryBracket(sStack, token) && m_aTokens.back() != "=" &&
        _CL_NOT_(CompareToken(token.scope - 1, "=")))
      {
        EOE = c_size + 1;
      }
      
      if(OnToken(token)) {
        token.ClearMarker();
      }

      // 可能被宏展开后清除
      if(token.marker) {
        if( ! MergeStringToken(token)){
          m_aTokens.push_back(token);
        }
      }

      if(it == ';') {
        ASSERT(EOE < (int)m_aTokens.size());
        for(auto it = m_aTokens.begin() + EOE; it != m_aTokens.end(); ++it)
        {
          ASSERT(it->semi_scope == -1); // 如果非-1，则说明被覆盖
          it->semi_scope = c_size;
        }
        EOE = c_size + 1;
      }
    }

    for(int i = 0; i < countof(s_PairMark); ++i)
    {
      PairStack& s = sStack[i];
      if( ! s.empty()) {
        // ERROR: 不匹配
        ERROR_MSG__MISSING_CLOSEDBRACKET;
      }
    }

#ifdef USE_CLSTD_TOKENS
#else
    SetTriggerCallBack(MultiByteOperatorProc, NULL);
#endif // #ifdef USE_CLSTD_TOKENS
    SetIteratorCallBack(IteratorProc, NULL);

    // 如果是子解析器，这里借用了父对象的信息定位，退出时要清空，避免析构时删除
    if(pParent) {
      m_pMsg = NULL;
    }
    m_pParent = pPrevParent;
    return m_aTokens.size();
  }

  GXBOOL CodeParser::MergeStringToken(const TOKEN& token)
  {
    if(m_aTokens.empty()) {
      return FALSE;
    }

    auto& last_one = m_aTokens.back();

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
        clStringA str;
        const iterator* aTonkens[] = {&last_one, &token};
        
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
        //str.Append(last_one.marker, last_one.length);
        //str.Append(token.marker, token.length);
        last_one.Set(m_pContext->Strings, str);
      }
#ifdef ENABLE_STRINGED_SYMBOL
      last_one.symbol = last_one.ToString();
#endif // #ifdef ENABLE_STRINGED_SYMBOL
      return TRUE;
    }
    return FALSE;
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

    //auto itMacro = m_Macros.find(strTokenName);
    //if(itMacro == m_Macros.end()) {
    //  return FALSE;
    //}
    
    //MACRO* pMacro = &itMacro->second;
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
    //ctx.pLineNumRef = &token;
    //ExpandMacro(ctx);
    //m_ExpandedStream = ctx.stream;
    TOKEN next_token = token;
    ExpandMacroStream(stream, token);
    m_ExpandedStream = stream;
    m_ExpandedStream.push_back(next_token);
    //*/
    return TRUE;
  }

//#define AAA(a,b) a b
  GXBOOL CodeParser::TryMatchMacro(MACRO_EXPAND_CONTEXT& ctx_out, TOKEN::List::iterator* it_out, const TOKEN::List::iterator& it_begin, const TOKEN::List::iterator& it_end)
  {
    if(it_begin->type == TOKEN::TokenType_String || TEST_FLAG(m_dwState, AttachFlag_NotExpandMacro) || ! m_ExpandedStream.empty()) {
      return FALSE;
    }

    if( ! (ctx_out.pMacro = FindMacro(*it_begin))) {
      return FALSE;
    }

    //clStringA strTokenName = it_begin->ToString();

    //auto itMacro = m_Macros.find(strTokenName);
    //if(itMacro == m_Macros.end()) {
    //  return FALSE;
    //}

    TOKEN::List::iterator it = it_begin;
    //ctx_out.pMacro = &itMacro->second;
    ctx_out.stream.clear();
    ctx_out.ActualParam.clear();

    if(ctx_out.pMacro->aFormalParams.empty())
    {
      ++it;
    }
    else
    {
      ++it;
      if(*it != '(' || it == it_end) {
        OutputErrorW(*it, E2008_宏定义中的意外_vs, clStringW(it->ToString()));
        *it_out = it;
        return TRUE;
      }

      if(++it == it_end) {
        OutputErrorW(*it, E2008_宏定义中的意外_vs, clStringW(it->ToString()));
        *it_out = it;
        return TRUE;
      }

      int depth = 1;

      //
      // 准备宏实参表
      //
      TOKEN::List ll;
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
            c.stream.back().type = ArithmeticExpression::TOKEN::TokenType_String;
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
    RTSCOPE scope(0, m_aTokens.size());
    while(ParseStatement(&scope));
    RelocalePointer();
    return TRUE;
  }

  GXBOOL CodeParser::ParseStatement(RTSCOPE* pScope)
  {    
    return (pScope->begin < pScope->end) && (
      ParseStatementAs_Definition(pScope) ||
      ParseStatementAs_Function(pScope) || ParseStatementAs_Struct(pScope));
  }

  GXBOOL CodeParser::ParseStatementAs_Definition(RTSCOPE* pScope)
  {
    TOKEN* p = &m_aTokens[pScope->begin];

    if(p->semi_scope == pScope->begin) { // 独立的分号
      pScope->begin++;
      return TRUE;
    }
    else if((RTSCOPE::TYPE)p->semi_scope == RTSCOPE::npos || (RTSCOPE::TYPE)p->semi_scope > pScope->end) {
      return FALSE;
    }

    const TOKEN* pEnd = &m_aTokens.front() + p->semi_scope;

    STATEMENT stat = {StatementType_Definition};
    RTSCOPE::TYPE definition_end = p->semi_scope;

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

    if( ! ParseExpression(RTSCOPE(pScope->begin, definition_end), &stat.defn.sRoot))
    {
      ERROR_MSG__MISSING_SEMICOLON(IDX2ITER(definition_end));
      return FALSE;
    }
    //*
    //
    // 将类型定义中的逗号表达式展开为独立的类型定义
    // 如 float a, b, c; 改为
    // float a; float b; float c;
    //
    SYNTAXNODE* pNode = stat.defn.sRoot.un.pNode;
    cllist<SYNTAXNODE*> sDefinitionList;
    SYNTAXNODE::RecursiveNode(this, pNode, [this, &sDefinitionList](SYNTAXNODE* pNode) -> GXBOOL {
      if(_CL_NOT_(
        pNode->mode == ArithmeticExpression::SYNTAXNODE::MODE_Definition || 
        //pNode->mode == ArithmeticExpression::SYNTAXNODE::MODE_DefinitionConst ||
        (pNode->pOpcode && (*(pNode->pOpcode)) == ',')) )
      {
        return FALSE;
      }
      sDefinitionList.push_back(pNode);
      return TRUE;
    });

    if(sDefinitionList.size() == 1) {
      m_aStatements.push_back(stat);
    }
    else {
      ASSERT( ! sDefinitionList.empty());
      // 这里list应该是如下形式
      // (define) [type] [node1]
      // (node1) [node2] [var define node N]
      // (node2) [node3] [var define node (N-1)]
      // ...
      // (nodeN) [var define node 1] [var define node 2]

      auto& front = sDefinitionList.front();

      ASSERT(front->mode == SYNTAXNODE::MODE_Definition/* || front->mode == SYNTAXNODE::MODE_DefinitionConst*/);

      front->Operand[1].ptr = sDefinitionList.back()->Operand[0].ptr;
      //front->SetOperandType(1, sDefinitionList.back()->GetOperandType(0));
      stat.defn.sRoot.un.pNode = front;
      m_aStatements.push_back(stat);

      auto it = sDefinitionList.end();
      ASSERT(front->GetOperandType(0) == SYNTAXNODE::FLAG_OPERAND_IS_TOKEN);
      for(--it; it != sDefinitionList.begin(); --it)
      {
        SYNTAXNODE& SyntaxNode = **it;

        // 逗号并列式改为独立类型定义式
        SyntaxNode.mode = front->mode;
        SyntaxNode.pOpcode = NULL;
        SyntaxNode.Operand[0].ptr = front->Operand[0].ptr; // type
        //SyntaxNode.SetOperandType(0, SYNTAXNODE::FLAG_OPERAND_IS_TOKEN); // front->GetOperandType(0)

        // 加入列表
        stat.defn.sRoot.un.pNode = *it;
        m_aStatements.push_back(stat);
      }
    }//*/
    ASSERT(pEnd->semi_scope == definition_end && *pEnd == ';');
    pScope->begin = definition_end + 1;
    return TRUE;
  }

  GXBOOL CodeParser::ParseStatementAs_Function(RTSCOPE* pScope)
  {
    // 函数语法
    //[StorageClass] Return_Value Name ( [ArgumentList] ) [: Semantic]
    //{
    //  [StatementBlock]
    //};

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
    if(p[0].scope != p[1].scope + 1) // 有参数: 两个括号不相邻
    {
      RTSCOPE ArgScope(m_aTokens[p->scope].scope + 1, p->scope);
      ParseFunctionArguments(&stat, &ArgScope);
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
      RTSCOPE func_statement_block(m_aTokens[p->scope].scope, p->scope);

      stat.type = StatementType_Function;
      p = &m_aTokens[p->scope];
      ++p;

      if(func_statement_block.IsValid())
      {
        STATEMENT sub_stat = {StatementType_Expression};
        if(ParseStatementAs_Expression(&sub_stat, &func_statement_block/*, FALSE*/))
        {
          stat.func.pExpression = (STATEMENT*)m_aSubStatements.size();
          m_aSubStatements.push_back(sub_stat);
        }
      }
    }
    else {
      // ERROR: 声明看起来是一个函数，但是既不是声明也不是实现。
      return FALSE;
    }


    m_aStatements.push_back(stat);
    pScope->begin = p - &m_aTokens.front();
    return TRUE;
  }

  GXBOOL CodeParser::ParseStatementAs_Struct( RTSCOPE* pScope )
  {
    TOKEN* p = &m_aTokens[pScope->begin];
    const TOKEN* pEnd = &m_aTokens.front() + pScope->end;

    if(*p != "struct") {
      return FALSE;
    }

    STATEMENT stat = {StatementType_Empty};
    INC_BUT_NOT_END(p, pEnd);

    stat.stru.szName = GetUniqueString(p);
    INC_BUT_NOT_END(p, pEnd);

    if(*p != "{") {
      // ERROR: 错误的结构体定义
      return FALSE;
    }

    RTSCOPE StruScope(m_aTokens[p->scope].scope + 1, p->scope);
    // 保证分析函数的域
    ASSERT(m_aTokens[StruScope.begin - 1] == "{" && m_aTokens[StruScope.end] == "}"); 

    pScope->begin = StruScope.end + 1;
    if(m_aTokens[pScope->begin] != ";") {
      // ERROR: 缺少“；”
      return FALSE;
    }
    ++pScope->begin;

    // #
    // # 解析成员变量
    // #
    if(ParseStructMembers(&stat, &StruScope))
    {
      m_aStatements.push_back(stat);
      return TRUE;
    }
    return FALSE;
  }

  GXBOOL CodeParser::ParseFunctionArguments(STATEMENT* pStat, RTSCOPE* pArgScope)
  {
    // 函数参数格式
    // [InputModifier] Type Name [: Semantic]
    // 函数可以包含多个参数，用逗号分隔

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
      arg.szType = GetUniqueString(p);

      INC_BUT_NOT_END(p, pEnd); // ERROR: 函数参数声明不正确

      arg.szName = GetUniqueString(p);

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
    m_aArgumentsPack.insert(m_aArgumentsPack.end(), aArgs.begin(), aArgs.end());
    return TRUE;
  }

  void CodeParser::RelocalePointer()
  {
    RelocaleStatements(m_aStatements);
    RelocaleStatements(m_aSubStatements);
  }

  GXBOOL CodeParser::ParseStructMember(STATEMENT* pStat, STRUCT_MEMBER& member, TOKEN**pp, const TOKEN* pMemberEnd)
  {
    TOKEN*& p = *pp;

    member.szName = GetUniqueString(p);
    p++;
    //INC_BUT_NOT_END(p, pMemberEnd); // ERROR: 结构体成员声明不正确

    if(p == pMemberEnd || *p == ',') {
      if(pStat->type != StatementType_Empty && pStat->type != StatementType_Struct) {
        // ERROR: 结构体成员作用不一致。纯结构体和Shader标记结构体混合定义
        OutputErrorW(*p, E9999_未定义错误_vsd, __FILEW__, __LINE__);
        return FALSE;
      }
      pStat->type = StatementType_Struct;
      //++p;
    }
    else if(*p == ':') {
      if(pStat->type != StatementType_Empty && pStat->type != StatementType_Signatures) {
        // ERROR: 结构体成员作用不一致。纯结构体和Shader标记结构体混合定义
        OutputErrorW(*p, E9999_未定义错误_vsd, __FILEW__, __LINE__);
        return FALSE;
      }
      pStat->type = StatementType_Signatures;
      INC_BUT_NOT_END(p, pMemberEnd);
      member.szSignature = GetUniqueString(p);

      // TODO: 检查这个是Signature

      INC_AND_END(p, pMemberEnd);

      ASSERT(*p == ';' || *p == ','); // 如果发生这个断言错误，检查如何处理这个编译错误
    }
    ASSERT(p <= pMemberEnd);
    return TRUE;
  }

  GXBOOL CodeParser::ParseStructMembers( STATEMENT* pStat, RTSCOPE* pStruScope )
  {
    // 作为结构体成员
    // Type[RxC] MemberName; 
    // 作为Shader标记
    // Type[RxC] MemberName : ShaderFunction; 
    // 这个结构体成员必须一致，要么全是普通成员变量，要么全是Shader标记

    TOKEN* p = &m_aTokens[pStruScope->begin];
    const TOKEN* pEnd = &m_aTokens.front() + pStruScope->end;
    MemberArray aMembers;

    while(p < pEnd)
    {
      STRUCT_MEMBER member = {NULL};

      if((RTSCOPE::TYPE)p->semi_scope == RTSCOPE::npos || (RTSCOPE::TYPE)p->semi_scope >= pStruScope->end) {
        ERROR_MSG__MISSING_SEMICOLON(*p);
        return FALSE;
      }

      const TOKEN* pMemberEnd = &m_aTokens.front() + p->semi_scope;
      member.szType = GetUniqueString(p);
      if(ParseType(p) == NULL) {
        clStringW strType = member.szType;
        OutputErrorW(*p, E1050_缺少类型描述, strType.CStr());
      }

      INC_BUT_NOT_END(p, pMemberEnd); // ERROR: 结构体成员声明不正确

      if( ! ParseStructMember(pStat, member, &p, pMemberEnd)) {
        return FALSE;
      }

      aMembers.push_back(member);

      while(*p == ',' && p < pMemberEnd) {
        INC_BUT_NOT_END(p, pMemberEnd); // 语法错误
        if( ! ParseStructMember(pStat, member, &p, pEnd)) {
          return FALSE;
        }
        aMembers.push_back(member);
      }

      ASSERT(*p == ';'); // 上面打印了错误信息,这个可有可无
      p++;

      //else if(*p == ',') {
      //  INC_BUT_NOT_END(p, pEnd); // 语法错误
      //  if( ! ParseStructMember(pStat, member, &p, pEnd)) {
      //    return FALSE;
      //  }
      //  CLBREAK;
      //}
      //else {
      //  // ERROR: 缺少“；”
      //  ERROR_MSG__MISSING_SEMICOLON;
      //  return FALSE;
      //}

      //aMembers.push_back(member);
    }

    pStat->stru.nNumOfMembers = aMembers.size();
    if(pStat->stru.nNumOfMembers)
    {
      pStat->stru.pMembers = (STRUCT_MEMBER*)m_aMembersPack.size();
      m_aMembersPack.insert(m_aMembersPack.end(), aMembers.begin(), aMembers.end());
    }
    else
    {
      ASSERT(pStat->type == StatementType_Empty); // 没有成员时这个值没有初始化
      pStat->stru.pMembers = NULL;
      pStat->type = StatementType_Struct;
    }

    return TRUE;
  }

  GXLPCSTR CodeParser::GetUniqueString( const TOKEN* pSym )
  {
    return m_pContext->Strings.add(pSym->ToString());
  }

  const CodeParser::TYPE* CodeParser::ParseType(const TOKEN* pSym)
  {
    TYPE sType = {NULL, 1, 1};

    // 对于内置类型，要解析出 Type[RxC] 这种格式
    for(int i = 0; s_aIntrinsicType[i].name != NULL; ++i)
    {
      const INTRINSIC_TYPE& t = s_aIntrinsicType[i];
      if(pSym->BeginsWith(t.name, t.name_len)) {
        const auto* pElement = pSym->marker + t.name_len;
        const int   remain   = pSym->length - (int)t.name_len;
        sType.name = t.name;

        // [(1..4)[x(1..4)]]
        if(remain == 0) {
          ;
        }
        else if(remain == 1 && *pElement >= '1' && *pElement <= '4') { // TypeR 格式
          sType.R = *pElement - '0';
          ASSERT(sType.R >= 1 && sType.R <= 4);
        }
        else if(remain == 3 && *pElement >= '1' && *pElement <= '4' && // TypeRxC 格式
          pElement[1] == 'x' && pElement[2] >= '1' && pElement[2] <= '4')
        {
          sType.R = pElement[0] - '0';
          sType.C = pElement[2] - '0';
          ASSERT(sType.R >= 1 && sType.R <= 4);
          ASSERT(sType.C >= 1 && sType.C <= 4);
        }
        else {
          break;
        }

        return &(*m_TypeSet.insert(sType).first);
      }
    }
    
    // TODO: 查找用户定义类型
    // 1.typdef 定义
    // 1.struct 定义

    return NULL;
  }

  GXBOOL CodeParser::ParseStatementAs_Expression(STATEMENT* pStat, RTSCOPE* pScope/*, GXBOOL bDbgRelocale */)
  {
    m_nDbgNumOfExpressionParse = 0;
    m_aDbgExpressionOperStack.clear();

    ASSERT(pScope->begin < pScope->end); // 似乎不应该有相等的情况, "{}" 区间这种是相差一个的
    if(pScope->begin + 1 == pScope->end) {
      return TRUE;
    }

    STATEMENT& stat = *pStat;
    
    GXBOOL bret = ParseExpression(*pScope, &stat.expr.sRoot);
    //TRACE("m_nDbgNumOfExpressionParse=%d\n", m_nDbgNumOfExpressionParse);

//#ifdef _DEBUG
//    // 这个测试会重定位指针，所以仅作为一次性调用，之后m_aSyntaxNodePack不能再添加新的数据了
//    if( ! bret)
//    {
//      TRACE("编译错误\n");
//      m_aDbgExpressionOperStack.clear();
//    }
//    else if(bDbgRelocale && TryGetNodeType(&stat.expr.sRoot) == SYNTAXNODE::FLAG_OPERAND_IS_NODEIDX) {
//      IndexToPtr(stat.expr.sRoot.pNode, m_aSyntaxNodePack);
//      RelocaleSyntaxPtr(stat.expr.sRoot.pNode);
//      DbgDumpSyntaxTree(stat.expr.sRoot.pNode, 0);
//    }
//#endif // #ifdef _DEBUG
    //m_aStatements.push_back(stat);
    return bret;
  }

  void CodeParser::DbgDumpSyntaxTree(clStringArrayA* pArray, const SYNTAXNODE* pNode, int precedence, clStringA* pStr)
  {
    clStringA str[2];
    for(int i = 0; i < 2; i++)
    {
      if(pNode->Operand[i].pTokn) {
        //const auto flag = TryGetNodeType(&pNode->Operand[i]);
        const auto flag = pNode->GetOperandType(i);
        if(flag == SYNTAXNODE::FLAG_OPERAND_IS_TOKEN) {
          str[i] = pNode->Operand[i].pTokn->ToString();
        }
        else if(flag == SYNTAXNODE::FLAG_OPERAND_IS_NODE) {
          DbgDumpSyntaxTree(pArray, pNode->Operand[i].pNode, pNode->pOpcode ? pNode->pOpcode->precedence : 0, &str[i]);
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
    strCommand.Format("[%s] [%s] [%s]", pNode->pOpcode ? pNode->pOpcode->ToString() : "", str[0], str[1]);
    if(pArray) {
      pArray->push_back(strCommand);
    }
    TRACE("%s\n", strCommand);

    clStringA strOut;
    switch(pNode->mode)
    {
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

  //GXBOOL CodeParser::ParseExpression(SYNTAXNODE::UN* pUnion, clsize begin, clsize end)
  //{
  //  RTSCOPE scope(begin, end);
  //  return ParseExpression(&scope, pUnion);
  //}
 
  GXBOOL CodeParser::ParseRemainStatement(RTSCOPE::TYPE parse_end, const RTSCOPE& scope, SYNTAXNODE::DESC* pDesc)
  {
    GXBOOL bret = TRUE;
    if(parse_end == RTSCOPE::npos) {
      return FALSE;
    }
    ASSERT(parse_end <= scope.end);
    if(parse_end < scope.end)
    {
      SYNTAXNODE::DESC A, B = {0};
      A = *pDesc;

      bret = ParseExpression(RTSCOPE(parse_end + 1, scope.end), &B) &&
        MakeSyntaxNode(pDesc, SYNTAXNODE::MODE_Chain, &A, &B);
    }
    return bret ? parse_end != RTSCOPE::npos : FALSE;
  }

  //////////////////////////////////////////////////////////////////////////
  GXBOOL CodeParser::TryKeywords(const RTSCOPE& scope, SYNTAXNODE::DESC* pDesc, RTSCOPE::TYPE* parse_end)
  {
    // 如果是关键字，返回true，否则返回false
    // 解析成功parse_end返回表达式最后一个token的索引，parse_end是这个关键字表达式之内的！
    // 解析失败parse_end返回RTSCOPE::npos

    const auto& front = m_aTokens[scope.begin];
    auto& pend = *parse_end;
    GXBOOL bret = TRUE;
    SYNTAXNODE::MODE eMode = SYNTAXNODE::MODE_Undefined;

    ASSERT(pend == RTSCOPE::npos); // 强调调用者要初始化这个变量

    if(front == "else") {
      // ERROR: "else" 不能独立使用
    }
    else if(front == "for") {
      pend = ParseFlowFor(scope, pDesc);
    }
    else if(front == "if") {
      pend = ParseFlowIf(scope, pDesc, FALSE);
    }
    else if(front == "while") {
      pend = ParseFlowWhile(scope, pDesc);
    }
    else if(front == "do") {
      pend = ParseFlowDoWhile(scope, pDesc);
    }
    else if(front == "struct") {
      pend = ParseStructDefine(scope, pDesc);
    }
    else if(front == "break") {
      eMode = SYNTAXNODE::MODE_Flow_Break;
    }
    else if(front == "continue") {
      eMode = SYNTAXNODE::MODE_Flow_Continue;
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
      SYNTAXNODE::DESC A = {0}, B = {0};

      A = front;
      pend = scope.begin + 1;

      if(front.semi_scope == RTSCOPE::npos || (RTSCOPE::TYPE)front.semi_scope > scope.end) {
        // ERROR: 缺少;
        ERROR_MSG__MISSING_SEMICOLON(front);
        break;
      }

      if(eMode == SYNTAXNODE::MODE_Return) {
        bret = ParseArithmeticExpression(scope.begin + 1, front.semi_scope, &B);
        MakeSyntaxNode(pDesc, eMode, NULL, &A, &B);
        pend = front.semi_scope;
      }
      else {
        MakeSyntaxNode(pDesc, eMode, NULL, &A, NULL);
      }
      break;
    }

    ASSERT(( ! bret && pend == RTSCOPE::npos) || bret);
    ASSERT(pend == RTSCOPE::npos || (pend > scope.begin && pend <= scope.end));
    return bret;
  }

  //GXBOOL CodeParser::TryBlock(const RTSCOPE& scope, SYNTAXNODE::UN* pUnion, RTSCOPE::TYPE* parse_end)
  //{
  //  const auto& first = m_aTokens[scope.begin];
  //  auto& pend = *parse_end;
  //  GXBOOL bret = TRUE;

  //  ASSERT(pend == RTSCOPE::npos); // 强调调用者要初始化这个变量

  //  if(first == '{')
  //  {
  //    if(first.scope > scope.end) {
  //      // ERROR: 没有正确的'}'来匹配
  //    }
  //    else {
  //      ParseExpression(pUnion, scope.begin + 1, first.scope);
  //    }
  //  }
  //  else {
  //    ASSERT(first.semi_scope < scope.end);
  //  }

  //  return bret;
  //}

  GXBOOL CodeParser::ParseExpression(const RTSCOPE& scope, SYNTAXNODE::DESC* pDesc)
  {
    ASSERT(scope.end == m_aTokens.size() || m_aTokens[scope.end] == ';' || 
      m_aTokens[scope.end] == '}');


    const GXINT_PTR count = scope.end - scope.begin;
    SYNTAXNODE::UN A = {0}, B = {0};
    GXBOOL bret = TRUE;
    RTSCOPE::TYPE parse_end = RTSCOPE::npos;

    if(count <= 1) {
      if(count < 0) {
        CLBREAK;
        return FALSE;
      }
      else if(count == 1) {
        *pDesc = m_aTokens[scope.begin];
      }
      
      return TRUE;  // count == 0 / count == 1
    }

    const auto& front = m_aTokens[scope.begin];

    if(front == '{') // 代码块
    {
      if((clsize)front.scope > scope.end) {
        // ERROR: 没有正确的'}'来匹配
      }

      ASSERT((RTSCOPE::TYPE)front.scope <= scope.end);  // scopeB.begin 最多仅仅比scope.end大1

      //////////////////////////////////////////////////////////////////////////
      //
      // ParseRemainStatement 在处理 statement block 剩余部分时有微妙的不同：
      // parse_end 如果严格限定等于 scope.end，pDesc不做任何修改，这种用法通常在
      // if/else if/else/for/while关键字的代码块解析中使用
      //
      // parse_end 如果小于 scope.end, pDesc会被修改为MODE_Chain，需要注意：
      // * parse_end 如果仅仅比 scope.end 小 1，则意味着生成的MODE_Chain是chain的结尾。
      // * 这种用法在 statement block 中用来标记chain的结尾
      //
      //////////////////////////////////////////////////////////////////////////
      bret = ParseExpression(RTSCOPE(scope.begin + 1, front.scope), pDesc);
      bret = bret && MakeSyntaxNode(pDesc, SYNTAXNODE::MODE_Block, pDesc, NULL);
      return ParseRemainStatement(front.scope, scope, pDesc);
    }
    else if(TryKeywords(scope, pDesc, &parse_end))
    {
      if(parse_end == RTSCOPE::npos) {
        return FALSE; // 解析错误, 直接返回
      }
      // 解析剩余部分
      return ParseRemainStatement(parse_end, scope, pDesc);
    }
    else if(front.semi_scope == RTSCOPE::npos) {
      ERROR_MSG__MISSING_SEMICOLON(front);
      return FALSE;
    }
    else if((clsize)front.semi_scope < scope.end)
    {
      ASSERT(m_aTokens[front.semi_scope] == ';'); // 目前进入这个循环的只可能是遇到分号

      RTSCOPE scopeA(scope.begin, front.semi_scope);

      // 跳过只是一个分号的情况
      if(scopeA.begin == scopeA.end) {
        return ParseExpression(RTSCOPE(scopeA.end + 1, scope.end), pDesc);
      }
      else
      {
        //RTSCOPE scopeB(front.semi_scope + 1, scope.end);
        ASSERT((RTSCOPE::TYPE)front.semi_scope + 1 <= scope.end); // 感觉有可能begin>end，这里如果遇到就改成if(scopeB.begin >= scopeB.end)

        bret = ParseExpression(scopeA, pDesc);
        return ParseRemainStatement(front.semi_scope, scope, pDesc);
      }
      //bret = bret && ParseExpression(scopeB, &B);
      //bret = bret && MakeSyntaxNode(pDesc, SYNTAXNODE::MODE_Chain, &A, &B);
    }

    ParseArithmeticExpression(scope, pDesc);
    return TRUE;
  }

  //////////////////////////////////////////////////////////////////////////



  //////////////////////////////////////////////////////////////////////////


  CodeParser::RTSCOPE::TYPE CodeParser::ParseFlowIf(const RTSCOPE& scope, SYNTAXNODE::DESC* pDesc, GXBOOL bElseIf)
  {
    // 与 ParseFlowWhile 相似
    SYNTAXNODE::DESC A = {0}, B = {0};
    GXBOOL bret = TRUE;
    ASSERT(m_aTokens[scope.begin] == "if");

    RTSCOPE sConditional(scope.begin + 2, m_aTokens[scope.begin + 1].scope);
    RTSCOPE sBlock;

    if(sConditional.begin >= scope.end || sConditional.end == -1 || sConditional.end > scope.end)
    {
      // ERROR: if 语法错误
      return RTSCOPE::npos;
    }

    bret = bret && ParseArithmeticExpression(sConditional, &A);



    sBlock.begin = sConditional.end + 1;
    sBlock.end = RTSCOPE::npos;
    if(sBlock.begin >= scope.end) {
      // ERROR: if 语法错误
      return RTSCOPE::npos;
    }

    auto& block_begin = m_aTokens[sBlock.begin];
    //SYNTAXNODE::MODE eMode = SYNTAXNODE::MODE_Undefined;

    if(TryKeywords(RTSCOPE(sBlock.begin, scope.end), &B, &sBlock.end))
    {
      //eMode = SYNTAXNODE::MODE_Flow_If;
      bret = sBlock.end != RTSCOPE::npos;
    }
    else
    {
      if(block_begin == '{')
      {
        sBlock.end = block_begin.scope;
        //sBlock.begin ++;
      }
      else {
        sBlock.end = block_begin.semi_scope;
      }

      if(sBlock.end > scope.end)
      {
        // ERROR: if 语法错误
        return RTSCOPE::npos;
      }
      bret = bret && ParseExpression(sBlock, &B);
    }

    const SYNTAXNODE::MODE eMode = TryGetNodeMode(&B);
    if(eMode != SYNTAXNODE::MODE_Block) {
      if(eMode != SYNTAXNODE::MODE_Chain) {
        bret = bret && MakeSyntaxNode(&B, SYNTAXNODE::MODE_Chain, &B, NULL);
      }
      bret = bret && MakeSyntaxNode(&B, SYNTAXNODE::MODE_Block, &B, NULL);
    }

    bret = bret && MakeSyntaxNode(pDesc, SYNTAXNODE::MODE_Flow_If, &A, &B);

    auto result = sBlock.end;


    if(bret && (scope.end - sBlock.end) > 1 && m_aTokens[sBlock.end + 1] == "else")
    {
      auto nNextBegin = sBlock.end + 1;

      // 只处理 else if/else 两种情况
      A = *pDesc;
      ++nNextBegin;
      if(nNextBegin >= scope.end) {
        // ERROR: else 语法错误
        return RTSCOPE::npos;
      }

      SYNTAXNODE::MODE eNextMode = SYNTAXNODE::MODE_Flow_Else;

      if(m_aTokens[nNextBegin] == "if")
      {
        eNextMode = SYNTAXNODE::MODE_Flow_ElseIf;
        result = ParseFlowIf(RTSCOPE(nNextBegin, scope.end), &B, TRUE);
      }
      else
      {
        auto& else_begin = m_aTokens[nNextBegin];
        result = RTSCOPE::npos;
        if(TryKeywords(RTSCOPE(nNextBegin, scope.end), &B, &result))
        {
          ;
        }
        else
        {
          result = else_begin == '{' ? else_begin.scope : else_begin.semi_scope;

          if(result == RTSCOPE::npos || result > scope.end) {
            // ERROR: else 语法错误
            return RTSCOPE::npos;
          }

          bret = ParseExpression(RTSCOPE(nNextBegin, result), &B);
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

    //  if(eNextMode != SYNTAXNODE::MODE_Chain) {
    //    if(eNextMode == SYNTAXNODE::MODE_Flow_Else) {
    //      DbgDumpScope("else", RTSCOPE(0,0), RTSCOPE(nNextBegin, scope.end));
    //    }
    //    DbgDumpScope(bElseIf ? "elif" : "if", sConditional, sBlock);
    //  }
    }
    //else {
    //  DbgDumpScope("if", sConditional, sBlock);
    //}

    return bret ? result : RTSCOPE::npos;
  }

  CodeParser::RTSCOPE::TYPE CodeParser::ParseFlowWhile(const RTSCOPE& scope, SYNTAXNODE::DESC* pDesc)
  {
    // 与 ParseFlowIf 相似
    SYNTAXNODE::DESC A = {0}, B = {0};
    GXBOOL bret = TRUE;
    ASSERT(m_aTokens[scope.begin] == "while");


    RTSCOPE sConditional(scope.begin + 2, m_aTokens[scope.begin + 1].scope);
    RTSCOPE sBlock;

    if( ! MakeScope(&sConditional, &MAKESCOPE(scope, scope.begin + 2, FALSE, scope.begin + 1, TRUE, 0))) {
      return RTSCOPE::npos;
    }
    
    bret = bret && ParseArithmeticExpression(sConditional, &A);


    sBlock.begin = sConditional.end + 1;
    sBlock.end   = RTSCOPE::npos;
    if(sBlock.begin >= scope.end) {
      // ERROR: while 语法错误
      return RTSCOPE::npos;
    }

    auto& block_begin = m_aTokens[sBlock.begin];
    //SYNTAXNODE::MODE eMode = SYNTAXNODE::MODE_Undefined;
    if(TryKeywords(RTSCOPE(sBlock.begin, scope.end), &B, &sBlock.end))
    {
      bret = sBlock.end != RTSCOPE::npos;
    }
    else
    {
      if(block_begin == '{')
      {
        sBlock.end = block_begin.scope;
        //sBlock.begin++;
      }
      else {
        sBlock.end = block_begin.semi_scope;
      }

      if(sBlock.end > scope.end)
      {
        // ERROR: while 语法错误
        return RTSCOPE::npos;
      }
      bret = bret && ParseExpression(sBlock, &B);
    }

    const SYNTAXNODE::MODE eMode = TryGetNodeMode(&B);
    if(eMode != SYNTAXNODE::MODE_Block) {
      if(eMode != SYNTAXNODE::MODE_Chain) {
        bret = bret && MakeSyntaxNode(&B, SYNTAXNODE::MODE_Chain, &B, NULL);
      }
      bret = bret && MakeSyntaxNode(&B, SYNTAXNODE::MODE_Block, &B, NULL);
    }

    //if(TryGetNode(&B) != SYNTAXNODE::MODE_Block) {
    //  bret = bret && MakeSyntaxNode(&B, SYNTAXNODE::MODE_Block, &B, NULL);
    //}

    bret = bret && MakeSyntaxNode(pDesc, SYNTAXNODE::MODE_Flow_While, &A, &B);

    DbgDumpScope("while", sConditional, sBlock);
    return bret ? sBlock.end : RTSCOPE::npos;
  }
  
  CodeParser::RTSCOPE::TYPE CodeParser::ParseFlowDoWhile(const RTSCOPE& scope, SYNTAXNODE::DESC* pDesc)
  {
    ASSERT(m_aTokens[scope.begin] == "do");

    if(scope.begin + 1 >= scope.end) {
      // ERROR: do 语法错误
      return RTSCOPE::npos;
    }

    RTSCOPE sConditional;
    RTSCOPE sBlock;
    SYNTAXNODE::DESC A = {0}, B = {0};


    if( ! MakeScope(&sBlock, &MAKESCOPE(scope, scope.begin + 2, FALSE, scope.begin + 1, TRUE, 0))) {
      return RTSCOPE::npos;
    }

    RTSCOPE::TYPE while_token = sBlock.end + 1;
    
    if(while_token >= scope.end && m_aTokens[while_token] != "while") {
      // ERROR: while 语法错误
      return RTSCOPE::npos;
    }

    if( ! MakeScope(&sConditional, &MAKESCOPE(scope, while_token + 2, FALSE, while_token + 1, TRUE, 0))) {
      return RTSCOPE::npos;
    }

    // TODO： 验证域的开始是括号和花括号

    GXBOOL bret = ParseExpression(sBlock, &B);
    bret = bret && ParseArithmeticExpression(sConditional, &A);
    bret = bret && MakeSyntaxNode(pDesc, SYNTAXNODE::MODE_Flow_DoWhile, NULL, &A, &B);

    RTSCOPE::TYPE while_end = sConditional.end + 1;
    if(while_end >= scope.end || m_aTokens[while_end] != ';') {
      // ERROR: 缺少 ";"
      return while_end;
    }

    return bret ? while_end : RTSCOPE::npos;
  }

  CodeParser::RTSCOPE::TYPE CodeParser::ParseStructDefine(const RTSCOPE& scope, SYNTAXNODE::DESC* pDesc)
  {
    SYNTAXNODE::DESC T, B = {0};
    GXBOOL bret = TRUE;
    ASSERT(m_aTokens[scope.begin] == "struct");


    //RTSCOPE sName(scope.begin + 2, m_aTokens[scope.begin + 1].scope);
    RTSCOPE::TYPE index = scope.begin + 2;

    if(index >= scope.end) {
      // ERROR: struct 定义错误，缺少定义名
      return RTSCOPE::npos;
    }

    // 确定定义块的范围
    RTSCOPE sBlock(index, m_aTokens[index].scope);
    if(sBlock.end == RTSCOPE::npos || sBlock.end >= scope.end) {
      ERROR_MSG__MISSING_SEMICOLON(IDX2ITER(sBlock.begin));
      return RTSCOPE::npos;
    }

    clstack<SYNTAXNODE::DESC> NodeStack;

    while(++index < sBlock.end && bret)
    {
      auto& decl = m_aTokens[index];
      if(decl.semi_scope == RTSCOPE::npos || (RTSCOPE::TYPE)decl.semi_scope >= sBlock.end) {
        ERROR_MSG_缺少分号(decl);
        break;
      }
      else if(index == decl.semi_scope) {
        continue;
      }

      bret = bret && ParseArithmeticExpression(RTSCOPE(index, decl.semi_scope), &T);
      NodeStack.push(T);
      
      index = decl.semi_scope;
    }

    while( ! NodeStack.empty())
    {
      MakeSyntaxNode(&B, SYNTAXNODE::MODE_Chain, &NodeStack.top(), &B);
      NodeStack.pop();
    }


    index = sBlock.end + 1;
    if(index >= scope.end || m_aTokens[index] != ';') {
      ERROR_MSG_缺少分号(IDX2ITER(sBlock.end - 1));
    }
    else {
      T = m_aTokens[index];
      ASSERT(*T.un.pTokn == ';');
      bret = bret && MakeSyntaxNode(&B, SYNTAXNODE::MODE_Block, &B, &T);
    }

    T = m_aTokens[scope.begin + 1];
    bret = bret && MakeSyntaxNode(pDesc, SYNTAXNODE::MODE_StructDef, &T, &B);

    //DbgDumpScope("while", sConditional, sBlock);
    return bret ? sBlock.end + 1 : RTSCOPE::npos;
  }

  GXBOOL CodeParser::MakeScope(RTSCOPE* pOut, MAKESCOPE* pParam)
  {
    // 这个函数用来从用户指定的begin和end中获得表达式所用的scope
    // 如果输入的begin和end符合要求则直接设置为scope，否则将返回FALSE
    // 可以通过mate标志获得begin或end位置上的配偶范围
    const MAKESCOPE& p = *pParam;
    RTSCOPE::TYPE& begin = pOut->begin;
    RTSCOPE::TYPE& end   = pOut->end;

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

  CodeParser::RTSCOPE::TYPE CodeParser::MakeFlowForScope(const RTSCOPE& scope, RTSCOPE* pInit, RTSCOPE* pCond, RTSCOPE* pIter, RTSCOPE* pBlock, SYNTAXNODE::DESC* pBlockNode)
  {
    ASSERT(m_aTokens[scope.begin] == "for"); // 外部保证调用这个函数的正确性

    auto open_bracket = scope.begin + 1;
    if(open_bracket >= scope.end || m_aTokens[open_bracket] != '(' || 
      (pIter->end = m_aTokens[open_bracket].scope) > scope.end)
    {
      // ERROR: for 格式错误
      return RTSCOPE::npos;
    }

    //
    // initializer
    // 初始化部分
    //
    pInit->begin  = scope.begin + 2;
    pInit->end    = m_aTokens[scope.begin].semi_scope;
    if(pInit->begin >= scope.end || pInit->end == -1) {
      // ERROR: for 格式错误
      return RTSCOPE::npos;
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
      return RTSCOPE::npos;
    }

    //
    // iterator
    // 迭代部分
    //
    pIter->begin = pCond->end + 1;
    // 上面设置过 pIter->end
    if(pIter->begin >= scope.end || pIter->begin > pIter->end) {
      // ERROR: for 格式错误
      return RTSCOPE::npos;
    }

    //
    // block
    //
    //RTSCOPE sBlock;
    pBlock->begin = pIter->end + 1;
    pBlock->end   = RTSCOPE::npos;
    if(pBlock->begin >= scope.end) {
      // ERROR: for 缺少执行
      return RTSCOPE::npos;
    }

    auto& block_begin = m_aTokens[pBlock->begin];
    if(block_begin == '{')
    {
      pBlock->end = block_begin.scope;
      //pBlock->begin++;
    }
    else if(TryKeywords(RTSCOPE(pBlock->begin, scope.end), pBlockNode, &pBlock->end))
    {
      ; // 没想好该干啥，哇哈哈哈!
    }
    //else if(block_begin == "for")
    //{
    //  pBlock->end = RTSCOPE::npos;
    //  return scope.end;  // 这个地方有特殊处理，特殊返回
    //  //return ParseFlowFor(&RTSCOPE(pBlock->begin, pScope->end), pUnion);
    //}
    // "if"
    else
    {
      pBlock->end = block_begin.semi_scope;
    }

    ASSERT(pBlock->end != -1);

    if(pBlock->end > scope.end) {
      // ERROR: for 格式错误
      return RTSCOPE::npos;
    }

    //ParseExpression(pBlock, pUnion);
    return pBlock->end;
  }

  CodeParser::RTSCOPE::TYPE CodeParser::ParseFlowFor(const RTSCOPE& scope, SYNTAXNODE::DESC* pDesc)
  {
    RTSCOPE sInitializer, sConditional, sIterator;
    RTSCOPE sBlock;

    SYNTAXNODE::DESC uInit = {0}, uCond = {0}, uIter = {0}, uBlock = {0}, D;
    
    auto result = MakeFlowForScope(scope, &sInitializer, &sConditional, &sIterator, &sBlock, &uBlock);
    if(result == RTSCOPE::npos)
    {
      return result;
    }

    ASSERT(m_aTokens[sBlock.begin] == "for" || m_aTokens[sBlock.end] == ';' || m_aTokens[sBlock.end] == '}');

    ParseArithmeticExpression(sInitializer, &uInit, TOKEN::FIRST_OPCODE_PRECEDENCE);
    ParseArithmeticExpression(sConditional, &uCond, TOKEN::FIRST_OPCODE_PRECEDENCE);
    ParseArithmeticExpression(sIterator   , &uIter, TOKEN::FIRST_OPCODE_PRECEDENCE);
    //if(sBlock.end == RTSCOPE::npos)
    //{
    //  ASSERT(m_aTokens[sBlock.begin] == "for"); // MakeFlowForScope 函数保证
    //  sBlock.end = ParseFlowFor(RTSCOPE(sBlock.begin, scope.end), &uBlock);
    //}
    //else

    if( ! uBlock.un.ptr) {
      ParseExpression(sBlock, &uBlock);
      //MakeSyntaxNode(&uBlock, SYNTAXNODE::MODE_Chain, &uBlock, NULL);
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
    
    return bret ? sBlock.end : RTSCOPE::npos;
  }

  const CodeParser::StatementArray& CodeParser::GetStatments() const
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
        IndexToPtr(it->func.pExpression, m_aSubStatements);
        break;

      case StatementType_Struct:
      case StatementType_Signatures:
        //it->stru.pMembers = &m_aMembersPack[(GXINT_PTR)it->stru.pMembers];
        IndexToPtr(it->stru.pMembers, m_aMembersPack);
        break;

      case StatementType_Expression:
        break;

      case StatementType_Definition:
        break;
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
      return PP_IfDefine(ctx, FALSE, tokens);
    }
    else if(tokens.front() == PREPROCESS_ifndef) {
      return PP_IfDefine(ctx, TRUE, tokens);
    }
    else if(tokens.front() == PREPROCESS_if) {
      return PP_If(ctx, pParse);
    }
    else if(tokens.front() == PREPROCESS_pragma) {
      PP_Pragma(tokens);
    }
    else if(tokens.front() == PREPROCESS_file) {
      if(tokens.size() != 2) {
        // ERROR: #file 格式不正确
      }

      clStringW strW = tokens[1].ToString();
      strW.TrimBoth('\"');
      m_pMsg->SetCurrentFilenameW(strW);
    }
    else if(tokens.front() == PREPROCESS_line) {
      if(tokens.size() != 2) {
        // ERROR: #line 格式不正确
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
      OutputErrorW(tokens.front(), E1021_无效的预处理器命令_vs, clStringW(tokens.front().ToString()));
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
    clStringA strMacroName(tokens[1].marker, tokens[1].length);
    //m_MacrosSet.insert(strMacroName);

    if(count == 1) {
      OutputErrorW(tokens.front(), E2007_define缺少定义_vs);
    }
    else if(count == 2) // "#define MACRO" 形
    {
      m_pContext->Macros.insert(clmake_pair(strMacroName, l_m));
    }
    else if(count == 3) // "#define MACRO XXX" 形
    {
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
          OutputErrorW(tokens[2], E2008_宏定义中的意外_vs, clStringW(tokens[2].ToString()));
          return;
        }

        const int scope_end = tokens[2].scope;
        if(scope_end > 3) // #define MACRO(...) ... 形解析
        {
          for(int i = 3; i < scope_end; i++)
          {
            if((tokens[i] == ',' && (i & 1)) || (tokens[i] != ',' && (i & 1) == 0)) {
              OutputErrorW(tokens[i], E2010_宏形参表中的意外, clStringW(tokens[i].ToString()));
              return;
            }
            if(i & 1) {
              sFormalList.push_back(tokens[i++]);
              sFormalList.back().formal_index = sFormalList.size() - 1;
              sFormalList.back().type = TOKEN::TokenType_FormalParams;
            }
          }
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
    clStringW strHeader = aTokens[1].ToString();
    strHeader.TrimBoth('\"');

    clpathfile::CombinePath(strPath, strPath, strHeader);

    //CodeParser* pRoot = GetRootParser();

    clBuffer* pBuffer = OpenIncludeFile(strPath);
    if(pBuffer == NULL) {
      OutputErrorW(E9999_未定义错误_vsd, __FILEW__, __LINE__);
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
      OutputErrorW(aTokens.front(), E4006_undef应输入标识符);
      return;
    }
    else if(aTokens.size() > 2) {
      OutputErrorW(aTokens.front(), E4067_预处理器指令后有意外标记_应输入换行符);
      return;
    }

    //clStringA str = aTokens[1].ToString();
    clStringA strMacroName = aTokens[1].ToString();
    //m_MacrosSet.erase(strMacroName);
    auto it = m_pContext->Macros.find(strMacroName);
    ASSERT(it != m_pContext->Macros.end()); // 集合里有的话化名表里也应该有
    
    //strMacroName.Append("@A");
    //do {
    //  it = m_Macros.erase(it);
    //} while (it->first.BeginsWith(strMacroName));
  }

  CodeParser::T_LPCSTR CodeParser::PP_IfDefine(const RTPPCONTEXT& ctx, GXBOOL bNot, const TOKEN::Array& tokens)
  {
    //const auto& tokens = *m_pSubParser->GetTokensArray();
    ASSERT( ! tokens.empty() && (tokens.front() == PREPROCESS_ifdef || tokens.front() == PREPROCESS_ifndef));
    //T_LPCSTR stream_end = GetStreamPtr() + GetStreamCount();

    if(tokens.size() == 1) {
      // ERROR: ifdef 缺少定义
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
        OutputErrorW(aTokens[2], E2059_SyntaxError_vs, clStringW(aTokens[2].ToString()));
        return;
      }

      if(aTokens[3].type != TOKEN::TokenType_String) {
        OutputErrorW(aTokens[2], E2059_SyntaxError_vs, clStringW(aTokens[2].ToString()));
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
      OutputErrorW(aTokens[1], E1021_无效的预处理器命令_vs, clStringW(aTokens[1].ToString()));
    }
  }

  //////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////

  GXBOOL CodeParser::CalculateValue(OPERAND& sOut, const SYNTAXNODE::DESC* pDesc)
  {
    OPERAND param[2];
    //SYNTAXNODE::DESC desc[2];
    //desc[0].un = 
    const SYNTAXNODE* pNode = TryGetNode(pDesc);

    if(pNode) {
      param[0].clear();
      param[1].clear();
      SYNTAXNODE::DESC l_desc;
      for(int i = 0; i < 2; i++)
      {
        if( ! pNode->Operand[i].ptr) {
          continue;
        }
        //l_desc.flag = pNode->GetOperandType(i);
        l_desc.un.ptr = pNode->Operand[i].ptr;
        CalculateValue(param[i], &l_desc);
      }
    }
    else {
      sOut.pToken = pDesc->un.pTokn;
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
        ASSERT(pNode->GetOperandType(0) == SYNTAXNODE::FLAG_OPERAND_IS_TOKEN);

        if(pNode->GetOperandType(1) != SYNTAXNODE::FLAG_OPERAND_IS_TOKEN) {
          OutputErrorW(*pNode->Operand[0].pTokn, E2004_应输入_defined_id);
          return FALSE;
        }

        auto it = m_pContext->Macros.find(pNode->Operand[1].pTokn->ToString());

        sOut.v.rank = ArithmeticExpression::VALUE::Rank_Signed64;
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
    SYNTAXNODE::DESC sDesc;
    if( ! pParser->ParseArithmeticExpression(1, pParser->m_aTokens.size(), &sDesc)) {
      // ERROR: 无法解析表达式
      return ctx.iter_next.marker;
    }


    VALUE v;
    if(sDesc.IsToken()) {
      // TODO: 测试 sDesc.un.pSym 能解析为一个数字，如果定义不为数字需要报错
      v = *sDesc.un.pTokn;
    }
    else
    {      
      OPERAND result;
      CalculateValue(result, &sDesc);
      TRACE("#if %s\n", result.v.ToString());

      v = result.v;
    }
    
    if(v.nValue64 == 0) {
      return PP_SkipConditionalBlock(PPCondRank_if, ctx.ppend, ctx.stream_end);
    }
    return ctx.iter_next.marker;
  }

  CodeParser::T_LPCSTR CodeParser::PP_SkipConditionalBlock(PPCondRank session, T_LPCSTR begin, T_LPCSTR end)
  {
    // session 调用 Macro_SkipConditionalBlock 所处的状态，这个决定了跳过多少预处理指令
    //typedef clstack<PPCondRank> RankStack;  // 预处理命令嵌套堆栈
    //RankStack sRankStack;
    UINT depth = 0;
    //PPCondRank rank = PPCondRank_Empty;
    T_LPCSTR p = begin;
    for(; p < end; ++p)
    {
      if(*p != '\n') {
        continue;
      }

      if((p = Macro_SkipGaps(p, end)) >= end) {
        return end;
      }

      if(*p != '#') {
        continue;
      }

      if((p = Macro_SkipGaps(p, end)) >= end) {
        return end;
      }

      size_t str_elif_len = 4;

      // if 要放在 ifdef, ifndef 后面
      if(CompareString(p, PREPROCESS_ifdef, 5) || CompareString(p, PREPROCESS_ifndef, 6) || CompareString(p, PREPROCESS_if, 2))
      {
        //pp_stack.push(p);
        depth++;
        //sRankStack.push(PPCondRank_if);
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

        p += 5; // "endif" 长度
        break;
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
        OutputErrorW(p, E1021_无效的预处理器命令_vs, str);
      }
    }
    
    for(; *p != '\n' && p < end; p++);
    return p != end ? ++p : end;
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
      token.type = TOKEN::TokenType_Numeric;
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

  void CodeParser::OutputErrorW(GXUINT code, ...)
  {
    for (auto it = m_aTokens.rbegin(); it != m_aTokens.rend(); ++it) {
      if(it->pContainer) {
        va_list arglist;
        va_start(arglist, code);
        m_pMsg->VarWriteErrorW(TRUE, it->marker, code, arglist);
        va_end(arglist);
        return;
      }
    }
  }

  void CodeParser::OutputErrorW(const TOKEN& token, GXUINT code, ...)
  {
    va_list  arglist;
    va_start(arglist, code);
    m_pMsg->VarWriteErrorW(TRUE, token.marker, code, arglist);
    va_end(arglist);
  }

  void CodeParser::OutputErrorW(T_LPCSTR ptr, GXUINT code, ...)
  {
    va_list  arglist;
    va_start(arglist, code);
    m_pMsg->VarWriteErrorW(TRUE, ptr, code, arglist);
    va_end(arglist);
  }

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

  //////////////////////////////////////////////////////////////////////////

  bool CodeParser::TYPE::operator<( const TYPE& t ) const
  {
    const int r = GXSTRCMP(name, t.name);
    if(r < 0) {
      return TRUE;
    }
    else if(r > 0) {
      return FALSE;
    }
    return ((R << 3) | C) < ((t.R << 3) | t.C);
  }

  //////////////////////////////////////////////////////////////////////////

  void CodeParser::MACRO::clear()
  {
    aTokens.clear();
    aFormalParams.clear();
  }

  //CodeParser::MACRO::MACRO() : bTranslate(0), bHasLINE(0), bHasFILE(0), bPoundSign(0)
  //{}

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
          token.type = ArithmeticExpression::TOKEN::TokenType_FormalParams;
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

} // namespace UVShader
