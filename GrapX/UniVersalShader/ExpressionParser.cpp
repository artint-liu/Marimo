#include <grapx.h>
#include <clUtility.h>
#include <Smart/SmartStream.h>
#include <clStringSet.h>
#include "ExpressionParser.h"

#include "clTextLines.h"
#include "../User/DataPoolErrorMsg.h"

// TODO:
// 1.float3(0) => float3(0,0,0)
// 2.返回值未完全初始化

//////////////////////////////////////////////////////////////////////////
//
// 运算符定义
//
//  Precedence  Operator          Description                                               Associativity 
//
//
//  2           ++   --           Suffix/postfix increment and decrement                    Left-to-right
//              ()                Function call
//              []                Array subscripting
//              .                 Element selection by reference
//
//  3           ++   --           Prefix increment and decrement                            Right-to-left 
//              +   −             Unary plus and minus
//              !   ~             Logical NOT and bitwise NOT
//              (type)            Type cast
//              &                 Address-of
//  5           *   /   %         Multiplication, division, and remainder                   Left-to-right 
//  6           +   −             Addition and subtraction                                  Left-to-right 
//  7           <<   >>           Bitwise left shift and right shift                        Left-to-right 
//  8           <   <=            For relational operators < and ≤ respectively             Left-to-right 
//              >   >=            For relational operators > and ≥ respectively             Left-to-right 
//  9           ==   !=           For relational = and ≠ respectively                       Left-to-right 
//  10          &                 Bitwise AND                                               Left-to-right 
//  11          ^                 Bitwise XOR (exclusive or)                                Left-to-right 
//  12          |                 Bitwise OR (inclusive or)                                 Left-to-right 
//  13          &&                Logical AND                                               Left-to-right 
//  14          ||                Logical OR                                                Left-to-right 
//  15          ?:                Ternary conditional                                       Right-to-left 
//
//              =                 Direct assignment (provided by default for C++ classes)
//              +=   −=           Assignment by sum and difference
//              *=   /=   %=      Assignment by product, quotient, and remainder
//              <<=   >>=         Assignment by bitwise left shift and right shift
//              &=   ^=   |=      Assignment by bitwise AND, XOR, and OR
//
//  17          ,                 Comma                                                     Left-to-right 
//
// UVS 中不用的操作符号
//  1           ::                Scope resolution                                          Left-to-right
//  2           −>                Element selection through pointer
//  3           sizeof            Size-of
//  3           *                 Indirection (dereference)
//              new, new[]        Dynamic memory allocation
//              delete, delete[]  Dynamic memory deallocation
//
//  4           .*   ->*          Pointer to member                                         Left-to-right 
//  16          throw             Throw operator (for exceptions)                           Right-to-left 

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

// 步进符号指针，但是不期望遇到结尾指针
#define INC_BUT_NOT_END(_P, _END) \
  if(++_P >= _END) {  \
    return FALSE;     \
  }

#define OUT_OF_SCOPE(s) (s == (clsize)-1)

static clsize s_nMultiByteOperatorLen = 0; // 最大长度

#define ERROR_MSG__MISSING_SEMICOLON  CLBREAK


#define FOR_EACH_MBO(_N, _IDX) for(int _IDX = 0; s_Operator##_N[_IDX].szOperator != NULL; _IDX++)

inline b32 IS_NUM(char c)
{
  return c >= '0' && c <= '9';
}

namespace UVShader
{
  //static const int c_plus_minus_precedence = 12; // +, - 作为符号时的优先级
  
  // 这个按照ASCII顺序分布, "+",",","-" 分别是43，44，45
  static CodeParser::MBO s_plus_minus[] = {
    {1, "+", OPP(12), TRUE, UNARY_RIGHT_OPERAND}, // 正号
    {},
    {1, "-", OPP(12), TRUE, UNARY_RIGHT_OPERAND}, // 负号
  };

  static CodeParser::MBO s_Operator1[] = {
    {1, ".", OPP(13), FALSE},
    //{1, "+", OPP(12), TRUE, UNARY_RIGHT_OPERAND}, // 正号
    //{1, "−", OPP(12), TRUE, UNARY_RIGHT_OPERAND}, // 负号
    {1, "!", OPP(12), TRUE, UNARY_RIGHT_OPERAND},
    {1, "~", OPP(12), TRUE, UNARY_RIGHT_OPERAND},
    //{1, "&", OPP(12), TRUE, UNARY_RIGHT_OPERAND},
    {1, "*", OPP(11)},
    {1, "/", OPP(11)},
    {1, "%", OPP(11)},
    {1, "+", OPP(10)}, // 加法
    {1, "-", OPP(10)}, // 减法
    {1, "<", OPP( 8)},
    {1, ">", OPP( 8)},
    {1, "&", OPP( 6)},
    {1, "^", OPP( 5), TRUE, UNARY_RIGHT_OPERAND},
    {1, "|", OPP( 4)},
    {1, "=", OPP( 1)},
    {1, "?", OPP( 1)}, // ?: 操作符
    {1, ":", OPP( 1)}, // ?: 操作符
    {1, ",", OPP( 0)},
    {NULL,},
  };

  static CodeParser::MBO s_Operator2[] = {
    {2, "--", OPP(13), TRUE, UNARY_RIGHT_OPERAND | UNARY_LEFT_OPERAND},
    {2, "++", OPP(13), TRUE, UNARY_RIGHT_OPERAND | UNARY_LEFT_OPERAND},
    {2, ">>", OPP( 9)},
    {2, "<<", OPP( 9)},
    {2, "<=", OPP( 8)},
    {2, ">=", OPP( 8)},
    {2, "==", OPP( 7)},
    {2, "!=", OPP( 7)},
    {2, "&&", OPP( 3)},
    {2, "||", OPP( 2)},
    {2, "+=", OPP( 1)},
    {2, "-=", OPP( 1)},
    {2, "*=", OPP( 1)},
    {2, "/=", OPP( 1)},
    {2, "%=", OPP( 1)},
    {2, "&=", OPP( 1)},
    {2, "^=", OPP( 1)},
    {2, "|=", OPP( 1)},
    {NULL,},

    // 不会用到的符号
    {2, "::",  OPP(-1)},
    {2, "->",  OPP(-1)},
    {2, ".*",  OPP(-1)},
    {2, "->*", OPP(-1)}, 
  };

  static CodeParser::MBO s_Operator3[] = {
    {3, "<<=", OPP(1)},
    {3, ">>=", OPP(1)},
    {NULL,},
  };
  //////////////////////////////////////////////////////////////////////////

  CodeParser::INTRINSIC_TYPE CodeParser::s_aIntrinsicType[] = {
    {"int",   3, 4, 4},
    {"vec",   3, 4, 0},
    {"bool",  4, 4, 0},
    {"half",  4, 4, 4},
    {"uint",  4, 4, 4},
    {"dword", 5, 4, 4},
    {"float", 5, 4, 4},
    {"double",6, 4, 4},
    {NULL},
  };

  CodeParser::CodeParser()
    : m_nMaxPrecedence(0)
    , m_nDbgNumOfExpressionParse(0)
    , m_pMsg(NULL)
  {
#ifdef _DEBUG

    // 检查名字与其设定长度是一致的
    for(int i = 0; s_aIntrinsicType[i].name != NULL; ++i) {
      ASSERT(GXSTRLEN(s_aIntrinsicType[i].name) == s_aIntrinsicType[i].name_len);
    }

#endif // #ifdef _DEBUG

    u32 aCharSem[128];
    GetCharSemantic(aCharSem, 0, 128);

    FOR_EACH_MBO(1, i) {
      m_nMaxPrecedence = clMax(m_nMaxPrecedence, s_Operator1[i].precedence);
      aCharSem[s_Operator1[i].szOperator[0]] |= M_CALLBACK;
    }

    FOR_EACH_MBO(2, i) {
      m_nMaxPrecedence = clMax(m_nMaxPrecedence, s_Operator2[i].precedence);
      aCharSem[s_Operator2[i].szOperator[0]] |= M_CALLBACK;
    }

    FOR_EACH_MBO(3, i) {
      m_nMaxPrecedence = clMax(m_nMaxPrecedence, s_Operator3[i].precedence);
      aCharSem[s_Operator3[i].szOperator[0]] |= M_CALLBACK;
    }

    ASSERT(m_nMaxPrecedence <= (1 << (TOKEN::precedence_bits - 1))); // 检测位域表示范围没有超过优先级
    ASSERT(m_nMaxPrecedence != TOKEN::ID_BRACE); // 保证优先级最大值与括号的ID不冲突

    SetFlags(GetFlags() | F_SYMBOLBREAK);
    SetCharSemantic(aCharSem, 0, 128);
    SetIteratorCallBack(IteratorProc, 0);
    SetTriggerCallBack(MultiByteOperatorProc, 0);

    InitPacks();
  }

  CodeParser::~CodeParser()
  {
    SAFE_DELETE(m_pMsg);
  }

  void CodeParser::InitPacks()
  {
    //
    // 所有pack类型，都存一个空值，避免记录0索引与NULL指针混淆的问题
    //
    SYNTAXNODE node = {NULL};
    m_aSyntaxNodePack.clear();
    m_aSyntaxNodePack.push_back(node);

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

    InitPacks();
  }

  b32 CodeParser::Attach( const char* szExpression, clsize nSize )
  {
    Cleanup();
    if( ! m_pMsg) {
      m_pMsg = new ErrorMessage();
      m_pMsg->LoadErrorMessageW(L"uvsmsg.txt");
      m_pMsg->WriteErrorW(FALSE, 0, 0);
    }
    return SmartStreamA::Initialize(szExpression, nSize);
  }

  //////////////////////////////////////////////////////////////////////////
  const CodeParser::MBO* MatchOperator(const CodeParser::MBO* op, u32 op_len, CodeParser::iterator& it, u32 remain)
  {
    if(remain <= op_len) {
      return NULL;
    }

    for(int i = 0; op[i].szOperator != NULL; ++i) {
      ASSERT(op_len == op[i].nLen);
      if(clstd::strncmpT(op[i].szOperator, it.marker, op[i].nLen) == 0)
      {
        it.length = op[i].nLen;
        return &op[i];
      }
    }
    return NULL;
  }
  //////////////////////////////////////////////////////////////////////////

  u32 CALLBACK CodeParser::MultiByteOperatorProc( iterator& it, u32 nRemain, u32_ptr lParam )
  {
    if(it.front() == '.' && it.length > 1) { // 跳过".5"这种格式的浮点数
      return 0;
    }

    CodeParser* pParser = (CodeParser*)it.pContainer;
    ASSERT(pParser->m_CurSymInfo.marker.marker == NULL); // 每次用完外面都要清理这个

    //int precedence = 0;
    const MBO* pProp = NULL;
    // 从多字节到单字节符号匹配,其中有一个返回TRUE就不执行后面的匹配了
    if(
      (pProp = MatchOperator(s_Operator3, 3, it, nRemain)) ||
      (pProp = MatchOperator(s_Operator2, 2, it, nRemain)) ||
      (pProp = MatchOperator(s_Operator1, 1, it, nRemain)) )
    {
      pParser->m_CurSymInfo.Set(it);
      pParser->m_CurSymInfo.precedence = pProp->precedence;
      pParser->m_CurSymInfo.scope = -1;
      pParser->m_CurSymInfo.unary = pProp->unary;
      pParser->m_CurSymInfo.unary_mask = pProp->unary_mask;
      return 0;
    }
    return 0;
  }

  u32 CALLBACK CodeParser::IteratorProc( iterator& it, u32 remain, u32_ptr lParam )
  {
    GXBOOL bENotation = FALSE;

    //
    // 进入数字判断模式
    //
    if((it.front() == '.' && IS_NUM(it.marker[it.length])) ||             // 第一个是'.'
      (IS_NUM(it.front()) && (it.back() == 'e' || it.back() == 'E')) ||   // 第一个是数字，最后以'e'结尾
      (IS_NUM(it.back()) && it.marker[it.length] == '.'))                 // 最后是数字，下一个是'.'
    {
      it.length++;
      while(--remain)
      {
        if(IS_NUM(it.marker[it.length])) {
          it.length++;
        }
        else if( ! bENotation && // 没有切换到科学计数法时遇到‘e’标记
          (it.marker[it.length] == 'e' || it.marker[it.length] == 'E'))
        {
          bENotation = TRUE;
          it.length++;

          // 科学计数法，+/- 符号判断
          if((--remain) != 0 && (*(it.end()) == '-' || *(it.end()) == '+')) {
            it.length++;
          }
        }
        else {
          break;
        }
      }
      if(it.marker[it.length] == 'f' || it.marker[it.length] == 'F' ||
        it.marker[it.length] == 'h' || it.marker[it.length] == 'H') {
        it.length++;
      }
    }
    else if(it.marker[0] == '/' && (remain > 0 && it.marker[1] == '/')) // 处理单行注释“//...”
    {
      SmartStreamUtility::ExtendToNewLine(it, 2, remain);
      ++it;
    }
    else if(it.marker[0] == '/' && (remain > 0 && it.marker[1] == '*')) // 处理块注释“/*...*/”
    {
      SmartStreamUtility::ExtendToCStyleBlockComment(it, 2, remain);
      ++it;
    }
    else if(it.marker[0] == '#' && SmartStreamUtility::IsHeadOfLine(it.pContainer, it.marker))
    {
      SmartStreamUtility::ExtendToNewLine(it, 1, remain);
      ++it; // 暂时跳过
    }
    ASSERT((int)remain >= 0);
    return 0;
  }

  clsize CodeParser::EstimateForTokensCount() const
  {
    auto count = GetStreamCount();
    return (count << 1) + (count >> 1); // 按照 字节数：符号数=2.5：1来计算
  }

  clsize CodeParser::GenerateTokens()
  {
    auto stream_end = end();
    ASSERT(m_aTokens.empty()); // 调用者负责清空

    m_aTokens.reserve(EstimateForTokensCount());
    typedef clstack<int> PairStack;
    //PairStack stackBrackets;        // 圆括号
    //PairStack stackSquareBrackets;  // 方括号
    //PairStack stackBrace;           // 花括号
    TOKEN sym;
    
    // 只是清理
    m_CurSymInfo.ClearMarker();
    m_CurSymInfo.precedence = 0;
    m_CurSymInfo.unary      = 0;

    struct PAIR_CONTEXT
    {
      GXCHAR    chOpen;    // 开区间
      GXCHAR    chClosed;  // 闭区间
      GXBOOL    bNewEOE;   // 更新End Of Expression的位置
      PairStack sStack;
    };

    static PAIR_CONTEXT pair_context[] = {
      {'(', ')', FALSE},   // 圆括号
      {'[', ']', FALSE},   // 方括号
      {'{', '}', TRUE },   // 花括号
      {'?', ':', FALSE},
      {NULL, },
    };

    int EOE = 0; // End Of Expression

    for(auto it = begin(); it != stream_end; ++it)
    {
      const int c_size = (int)m_aTokens.size();
      sym.Set(it);
      sym.scope = -1;
      sym.semi_scope = -1;

      ASSERT(m_CurSymInfo.marker.marker == NULL || it.marker == m_CurSymInfo.marker.marker); // 遍历时一定这个要保持一致

      sym.precedence = m_CurSymInfo.precedence;
      sym.unary      = m_CurSymInfo.unary;
      sym.unary_mask = m_CurSymInfo.unary_mask;

      // 如果是 -,+ 检查前一个符号是不是操作符或者括号，如果是就认为这个 -,+ 是正负号
      if((it == '-' || it == '+') && ! m_aTokens.empty())
      {        
        const auto& l_back = m_aTokens.back();

        // 一元操作符，+/-就不转换为正负号
        // '}' 就不判断了 { something } - abc 这种格式应该是语法错误
        if(l_back.precedence != 0 && l_back != ')' && l_back != ']' && ( ! l_back.unary)) {
          const auto& p = s_plus_minus[(int)(it.marker[0] - '+')];
          sym.precedence = p.precedence;
          sym.unary      = p.unary;
          sym.unary_mask = p.unary_mask;
        }
      }

      // 只是清理
      m_CurSymInfo.ClearMarker();
      m_CurSymInfo.precedence = 0;
      m_CurSymInfo.unary      = 0;

      // 符号配对处理
      for(int i = 0; pair_context[i].chOpen != NULL; ++i)
      {
        PAIR_CONTEXT& c = pair_context[i];
        if(it == c.chOpen) {
          c.sStack.push(c_size);
        }
        else if(it == c.chClosed) {
          if(c.sStack.empty()) {
            // ERROR: 不匹配            
          }
          sym.scope = c.sStack.top();
          m_aTokens[sym.scope].scope = c_size;
          c.sStack.pop();
        }
        else {
          continue;
        }

        // ?: 操作符会设置precedence的优先级，而我们只想设置括号的标记
        if(sym.precedence == 0) {
          sym.precedence = TOKEN::ID_BRACE;
        }

        if(c.bNewEOE) {
          EOE = c_size + 1;
        }
      }
      
      /*
      if(it == ';') {
        if(EOE < c_size) {
          const auto& bracket_stack = pair_context[0].sStack;

          // 这个是要保证for中的分号定位在“for”上，而不是它前面的符号里
          if( ! bracket_stack.empty() && bracket_stack.top() > (EOE + 1) &&
          m_aTokens[bracket_stack.top() - 1] == "for" ) {
          EOE = bracket_stack.top() - 1;
          }
          m_aTokens[EOE].semi_scope = c_size;

        }
        EOE = c_size + 1;
      }
      // 这个可能不需要，因为按照语法for应该是在";"分号或者"}"闭花括号后面
      //else if(it == "for") { // for(;;) 这个形式有点特殊
      //  EOE = c_size;
      //}

      m_aTokens.push_back(sym);

      /*/
      m_aTokens.push_back(sym);

      if(it == ';') {
        ASSERT(EOE < (int)m_aTokens.size());
        for(auto it = m_aTokens.begin() + EOE; it != m_aTokens.end(); ++it)
        {
          it->semi_scope = c_size;
        }
        EOE = c_size + 1;
      }

      //*/
    }

    for(int i = 0; pair_context[i].chOpen != NULL; ++i)
    {
      PAIR_CONTEXT& c = pair_context[i];
      if( ! c.sStack.empty()) {
        // ERROR: 不匹配
      }
    }

    return m_aTokens.size();
  }

  const CodeParser::TokenArray* CodeParser::GetTokensArray() const
  {
    return &m_aTokens;
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

    if((RTSCOPE::TYPE)p->semi_scope == RTSCOPE::npos || (RTSCOPE::TYPE)p->semi_scope > pScope->end) {
      return FALSE;
    }
    const TOKEN* pEnd = &m_aTokens.front() + p->semi_scope;

    STATEMENT stat = {StatementType_Definition};
    RTSCOPE::TYPE definition_end = p->semi_scope;

    // 修饰
    if(*p == "const") {
      stat.defn.modifier = UniformModifier_const;
      INC_BUT_NOT_END(p, pEnd);
    }
    else {
      stat.defn.modifier = UniformModifier_empty;
    }

    stat.defn.szType = GetUniqueString(p);
    if(p + 1 < pEnd) {
      stat.defn.szName = GetUniqueString(p + 1);
    }

    if( ! ParseExpression(RTSCOPE(pScope->begin, definition_end), &stat.defn.sRoot))
    {
      ERROR_MSG__MISSING_SEMICOLON;
      return FALSE;
    }

    //// 类型
    //stat.defn.szType = GetUniqueString(p);
    //INC_BUT_NOT_END(p, pEnd);

    //while(1) {
    //  // 名称
    //  stat.defn.szName = GetUniqueString(p);
    //  INC_BUT_NOT_END(p, pEnd);

    //  // 数组定义----目前跳过了
    //  while(*p == '[') {
    //    if((RTSCOPE::TYPE)p->scope >= definition_end) {
    //      ERROR_MSG__MISSING_SEMICOLON;
    //    }
    //    p = &m_aTokens[p->scope];
    //    INC_BUT_NOT_END(p, pEnd);
    //  }

    //  if(*p == '=')
    //  {
    //    INC_BUT_NOT_END(p, pEnd);
    //    ParseExpression(RTSCOPE(p - &m_aTokens.front(), definition_end), &stat.defn.sRoot);
    //  }
    //  else if(*p == ';') {
    //    break;
    //  }
    //  else if(*p == ',') {
    //    m_aStatements.push_back(stat);
    //    INC_BUT_NOT_END(p, pEnd);
    //  }
    //  else {
    //    ERROR_MSG__MISSING_SEMICOLON;
    //    pScope->begin = definition_end + 1;
    //    return FALSE;
    //  }
    //}
    //SYNTAXNODE::RecursiveNode(stat.defn.sRoot.pNode, [](SYNTAXNODE* pNode) -> GXBOOL {
    //  if(pNode->pOpcode && (*(pNode->pOpcode)) == ',') {

    //  }
    //});

    m_aStatements.push_back(stat);
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
      stat.func.szSemantic = m_Strings.add((p++)->ToString());
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
    ParseStructMembers(&stat, &StruScope);

    m_aStatements.push_back(stat);
    return TRUE;
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

  void CodeParser::RelocaleSyntaxPtr(SYNTAXNODE* pNode)
  {
    if(pNode->OperandA_IsNodeIndex() && pNode->Operand[0].pNode) {
      IndexToPtr(pNode->Operand[0].pNode, m_aSyntaxNodePack);
      RelocaleSyntaxPtr((SYNTAXNODE*)pNode->Operand[0].pNode);
    }

    if(pNode->OperandB_IsNodeIndex() && pNode->Operand[1].pNode) {
      IndexToPtr(pNode->Operand[1].pNode, m_aSyntaxNodePack);
      RelocaleSyntaxPtr((SYNTAXNODE*)pNode->Operand[1].pNode);
    }
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
        return FALSE;
      }
      pStat->type = StatementType_Struct;
      //++p;
    }
    else if(*p == ':') {
      if(pStat->type != StatementType_Empty && pStat->type != StatementType_Struct) {
        // ERROR: 结构体成员作用不一致。纯结构体和Shader标记结构体混合定义
        return FALSE;
      }
      pStat->type = StatementType_Signatures;
      INC_BUT_NOT_END(p, pMemberEnd);
      member.szSignature = GetUniqueString(p);

      // TODO: 检查这个是Signature

      INC_BUT_NOT_END(p, pMemberEnd);

      ASSERT(*p == ';' || *p == ','); // 如果发生这个断言错误，检查如何处理这个编译错误
      //if(*p != ";") {
      //  ERROR_MSG__MISSING_SEMICOLON; // ERROR: 缺少“；”
      //  return FALSE;
      //}
      //++p;
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
        ERROR_MSG__MISSING_SEMICOLON;
        return FALSE;
      }

      const TOKEN* pMemberEnd = &m_aTokens.front() + p->semi_scope;
      member.szType = GetUniqueString(p);
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

      ASSERT(*p == ';');
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

    pStat->stru.pMembers = (STRUCT_MEMBER*)m_aMembersPack.size();
    pStat->stru.nNumOfMembers = aMembers.size();
    m_aMembersPack.insert(m_aMembersPack.end(), aMembers.begin(), aMembers.end());
    return TRUE;
  }

  GXLPCSTR CodeParser::GetUniqueString( const TOKEN* pSym )
  {
    return m_Strings.add(pSym->ToString());
  }

  const CodeParser::TYPE* CodeParser::ParseType(const TOKEN* pSym)
  {
    TYPE sType = {NULL, 1, 1};

    // 对于内置类型，要解析出 Type[RxC] 这种格式
    for(int i = 0; s_aIntrinsicType[i].name != NULL; ++i)
    {
      const INTRINSIC_TYPE& t = s_aIntrinsicType[i];
      if(pSym->marker.BeginsWith(t.name, t.name_len)) {
        const auto* pElement = pSym->marker.marker + t.name_len;
        const int   remain   = pSym->marker.length - (int)t.name_len;
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

    if(pScope->begin == pScope->end) {
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
      if(pNode->Operand[i].pSym) {
        if(TryGetNodeType(&pNode->Operand[i]) == SYNTAXNODE::FLAG_OPERAND_IS_TOKEN) {
          str[i] = pNode->Operand[i].pSym->ToString();
        }
        else {
          DbgDumpSyntaxTree(pArray, pNode->Operand[i].pNode, pNode->pOpcode ? pNode->pOpcode->precedence : 0, &str[i]);
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

    case SYNTAXNODE::MODE_ArrayIndex:
      strOut.Format("%s[%s]", str[0], str[1]);
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

    case SYNTAXNODE::MODE_Normal:
      if(precedence > pNode->pOpcode->precedence) { // 低优先级先运算
        strOut.Format("(%s%s%s)", str[0], pNode->pOpcode->ToString(), str[1]);
      }
      else {
        strOut.Format("%s%s%s", str[0], pNode->pOpcode->ToString(), str[1]);
      }
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
 
  GXBOOL CodeParser::ParseArithmeticExpression(clsize begin, clsize end, SYNTAXNODE::UN* pUnion)
  {
    RTSCOPE scope(begin, end);
    return ParseArithmeticExpression(scope, pUnion);
  }

  GXBOOL CodeParser::ParseArithmeticExpression(const RTSCOPE& scope_in, SYNTAXNODE::UN* pUnion)
  {
    RTSCOPE scope = scope_in;
    if(scope.end > scope.begin && m_aTokens[scope.end - 1] == ';') {
      scope.end--;
    }
    return ParseArithmeticExpression(scope, pUnion, TOKEN::FIRST_OPCODE_PRECEDENCE);
  }

  GXBOOL CodeParser::ParseRemainStatement(RTSCOPE::TYPE parse_end, const RTSCOPE& scope, SYNTAXNODE::UN* pUnion)
  {
    GXBOOL bret = TRUE;
    if(parse_end == RTSCOPE::npos) {
      return FALSE;
    }
    ASSERT(parse_end <= scope.end);
    if(parse_end < scope.end)
    {
      SYNTAXNODE::UN A, B = {0};
      A = *pUnion;

      bret = ParseExpression(RTSCOPE(parse_end + 1, scope.end), &B) &&
        MakeSyntaxNode(pUnion, SYNTAXNODE::MODE_Chain, NULL, &A, &B);
    }
    return bret ? parse_end != RTSCOPE::npos : FALSE;
  }

  //////////////////////////////////////////////////////////////////////////
  GXBOOL CodeParser::TryKeywords(const RTSCOPE& scope, SYNTAXNODE::UN* pUnion, RTSCOPE::TYPE* parse_end)
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
      pend = ParseFlowFor(scope, pUnion);
    }
    else if(front == "if") {
      pend = ParseFlowIf(scope, pUnion, FALSE);
    }
    else if(front == "while") {
      pend = ParseFlowWhile(scope, pUnion);
    }
    else if(front == "do") {
      pend = ParseFlowDoWhile(scope, pUnion);
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
      SYNTAXNODE::UN A = {0}, B = {0};

      A.pSym = &front;
      pend = scope.begin + 1;

      if(front.semi_scope == RTSCOPE::npos || (RTSCOPE::TYPE)front.semi_scope > scope.end) {
        // ERROR: 缺少;
        ERROR_MSG__MISSING_SEMICOLON;
        break;
      }

      if(eMode == SYNTAXNODE::MODE_Return) {
        bret = ParseArithmeticExpression(scope.begin + 1, front.semi_scope, &B);
        MakeSyntaxNode(pUnion, eMode, NULL, &A, &B);
        pend = front.semi_scope;
      }
      else {
        MakeSyntaxNode(pUnion, eMode, NULL, &A, NULL);
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

  GXBOOL CodeParser::ParseExpression(const RTSCOPE& scope, SYNTAXNODE::UN* pUnion)
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
        pUnion->pSym = &m_aTokens[scope.begin];
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
      // parse_end 如果严格限定等于 scope.end，pUnion不做任何修改，这种用法通常在
      // if/else if/else/for/while关键字的代码块解析中使用
      //
      // parse_end 如果小于 scope.end, pUnion会被修改为MODE_Chain，需要注意：
      // * parse_end 如果仅仅比 scope.end 小 1，则意味着生成的MODE_Chain是chain的结尾。
      // * 这种用法在 statement block 中用来标记chain的结尾
      //
      //////////////////////////////////////////////////////////////////////////
      bret = ParseExpression(RTSCOPE(scope.begin + 1, front.scope), pUnion);
      bret = bret && MakeSyntaxNode(pUnion, SYNTAXNODE::MODE_Block, pUnion, NULL);
      return ParseRemainStatement(front.scope, scope, pUnion);
    }
    else if(TryKeywords(scope, pUnion, &parse_end))
    {
      if(parse_end == RTSCOPE::npos) {
        return FALSE; // 解析错误, 直接返回
      }
      // 解析剩余部分
      return ParseRemainStatement(parse_end, scope, pUnion);
    }
    else if(front.semi_scope == RTSCOPE::npos) {
      ERROR_MSG__MISSING_SEMICOLON;
      return FALSE;
    }
    else if((clsize)front.semi_scope < scope.end)
    {
      ASSERT(m_aTokens[front.semi_scope] == ';'); // 目前进入这个循环的只可能是遇到分号

      RTSCOPE scopeA(scope.begin, front.semi_scope);

      // 跳过只是一个分号的情况
      if(scopeA.begin == scopeA.end) {
        return ParseExpression(RTSCOPE(scopeA.end + 1, scope.end), pUnion);
      }
      else
      {
        //RTSCOPE scopeB(front.semi_scope + 1, scope.end);
        ASSERT((RTSCOPE::TYPE)front.semi_scope + 1 <= scope.end); // 感觉有可能begin>end，这里如果遇到就改成if(scopeB.begin >= scopeB.end)

        bret = ParseExpression(scopeA, pUnion);
        return ParseRemainStatement(front.semi_scope, scope, pUnion);
      }
      //bret = bret && ParseExpression(scopeB, &B);
      //bret = bret && MakeSyntaxNode(pUnion, SYNTAXNODE::MODE_Chain, &A, &B);
    }

    ParseArithmeticExpression(scope, pUnion);
    return TRUE;
  }

  //////////////////////////////////////////////////////////////////////////

  GXBOOL CodeParser::ParseArithmeticExpression(const RTSCOPE& scope, SYNTAXNODE::UN* pUnion, int nMinPrecedence)
  {
    int nCandidate = m_nMaxPrecedence;
    GXINT_PTR i = (GXINT_PTR)scope.end - 1;
    GXINT_PTR nCandidatePos = i;
    SYNTAXNODE::UN A, B;

    const GXINT_PTR count = scope.end - scope.begin;

    if(count <= 1) {
      if(count == 1) {
        pUnion->pSym = &m_aTokens[scope.begin];
      }
      return TRUE;
    }

    const auto& front = m_aTokens[scope.begin];

    if(count == 2)
    {
      // 处理两种可能：(1)变量使用一元符号运算 (2)定义变量
      A.pSym = &front;
      B.pSym = &m_aTokens[scope.begin + 1];
      GXBOOL bret = TRUE;

      ASSERT(*B.pSym != ';'); // 已经在外部避免了表达式内出现分号

      if(A.pSym->precedence > 0)
      {
        bret = MakeSyntaxNode(pUnion, SYNTAXNODE::MODE_Normal, A.pSym, NULL, &B);
        DbgDumpScope(A.pSym->ToString(), RTSCOPE(0,0), RTSCOPE(scope.begin + 1, scope.end));
      }
      else if(B.pSym->precedence > 0)
      {
        bret = MakeSyntaxNode(pUnion, SYNTAXNODE::MODE_Normal, B.pSym, &A, NULL);
        DbgDumpScope(B.pSym->ToString(), RTSCOPE(scope.begin, scope.begin + 1), RTSCOPE(0,0));
      }
      else {
        // 变量声明
        bret = MakeSyntaxNode(pUnion, SYNTAXNODE::MODE_Definition, &A, &B);
      }
      return bret;
    }
    else if(front.precedence == 0 && m_aTokens[scope.begin + 1].precedence == 0) // 变量声明
    {
      ASSERT(count > 2);
      A.pSym = &front;
      B.ptr = NULL;
      GXBOOL bret = ParseArithmeticExpression(RTSCOPE(scope.begin + 1, scope.end), &B);
      bret = bret && MakeSyntaxNode(pUnion, SYNTAXNODE::MODE_Definition, &A, &B);
      return bret;
    }
    else if((front == '(' || front == '[') && front.scope == scope.end - 1)  // 括号内表达式
    {
      // 括号肯定是匹配的
      ASSERT(m_aTokens[scope.end - 1].scope == scope.begin);
      return ParseArithmeticExpression(RTSCOPE(scope.begin + 1, scope.end - 1), pUnion, TOKEN::FIRST_OPCODE_PRECEDENCE);
    }
    else if(m_aTokens[scope.begin + 1].scope == scope.end - 1)  // 整个表达式是函数调用
    {
      return ParseFunctionCall(scope, pUnion);
    }

    while(nMinPrecedence <= m_nMaxPrecedence)
    {
      if(nMinPrecedence == OPP(1))
      {
        for(i = (GXINT_PTR)scope.begin; i < (GXINT_PTR)scope.end; ++i)
        {
          m_nDbgNumOfExpressionParse++;

          const TOKEN& s = m_aTokens[i];

          if(s.precedence == TOKEN::ID_BRACE) // 跳过非运算符, 也包括括号
          {
            ASSERT(s.scope < (int)scope.end); // 闭括号肯定在表达式区间内
            i = s.scope;
            continue;
          }
          else if(s.precedence == 0 || s == ':') { // 跳过非运算符, 这里包括三元运算符的次级运算符
            continue;
          }

          // ?: 操作符标记：precedence 储存优先级，scope 储存?:的关系

          if(s.precedence == nMinPrecedence) {
            return MakeInstruction(&s, nMinPrecedence, &scope, pUnion, i);
          }
          else if(s.precedence < nCandidate) {
            nCandidate = s.precedence;
            // 这里优先级因为从LTR切换到RTL，所以不记录 nCandidatePos
          }
        } // for

        nCandidatePos = (GXINT_PTR)scope.end - 1;
      }
      else
      {
        for(; i >= (GXINT_PTR)scope.begin; --i)
        {
          m_nDbgNumOfExpressionParse++;
          const TOKEN& s = m_aTokens[i];

          // 优先级（2）是从右向左的，这个循环处理从左向右
          ASSERT(nMinPrecedence != 2);

          // 跳过非运算符, 也包括括号
          if(s.precedence == TOKEN::ID_BRACE)
          {
            ASSERT(s.scope < (int)scope.end); // 闭括号肯定在表达式区间内
            i = s.scope;
            continue;
          }
          else if(s.precedence == 0) { // 跳过非运算符
            continue;
          }

          if(s.precedence == nMinPrecedence) {
            return MakeInstruction(&s, nMinPrecedence, &scope, pUnion, i);
          }
          else if(s.precedence < nCandidate) {
            nCandidate = s.precedence;
            nCandidatePos = i;
          }
        } // for
      }

      if(nMinPrecedence >= nCandidate) {
        break;
      }

      nMinPrecedence = nCandidate;
      i = nCandidatePos;
    }

    if( ! ParseFunctionIndexCall(scope, pUnion))
    {
      clStringA strMsg(m_aTokens[scope.begin].marker.marker, (m_aTokens[scope.end - 1].marker.marker - m_aTokens[scope.begin].marker.marker) + m_aTokens[scope.end - 1].marker.length);
      TRACE("ERROR: 无法解析\"%s\"\n", strMsg);
      return FALSE;
    }
    return TRUE;
  }

  //////////////////////////////////////////////////////////////////////////

  GXBOOL CodeParser::ParseFunctionCall(const RTSCOPE& scope, SYNTAXNODE::UN* pUnion)
  {
    // 括号肯定是匹配的
    ASSERT(m_aTokens[scope.end - 1].scope == scope.begin + 1);

    SYNTAXNODE::UN A, B = {0};
    A.pSym = &m_aTokens[scope.begin];

    // TODO: 检查m_aTokens[scope.begin]是函数名

    auto& bracket = m_aTokens[scope.begin + 1];
    ASSERT(bracket == '[' || bracket == '(');
    GXBOOL bret = ParseArithmeticExpression(RTSCOPE(scope.begin + 2, scope.end - 1), &B, TOKEN::FIRST_OPCODE_PRECEDENCE);

    SYNTAXNODE::MODE mode = bracket == '(' ? SYNTAXNODE::MODE_FunctionCall : SYNTAXNODE::MODE_ArrayIndex;

    MakeSyntaxNode(pUnion, mode, NULL, &A, &B);
    DbgDumpScope(bracket == '(' ? "F" : "I", RTSCOPE(scope.begin, scope.begin + 1),
      RTSCOPE(scope.begin + 2, scope.end - 1));

    return bret;
  }

  GXBOOL CodeParser::ParseFunctionIndexCall(const RTSCOPE& scope, SYNTAXNODE::UN* pUnion)
  {
    // 从右到左解析这两种形式:
    // name(...)(...)(...)
    // name[...][...][...]
    // 括号域之间不能有其他符号, 括号域之内是数学表达式

    struct CONTEXT
    {
      SYNTAXNODE::MODE mode;
      SYNTAXNODE::UN   B;
    };

    typedef clstack<CONTEXT> SyntaxStack;
    SyntaxStack node_stack;
    SYNTAXNODE::UN A;
    CONTEXT c;
    TOKEN* pBack = &m_aTokens[scope.end - 1];
    A.pSym = &m_aTokens[scope.begin];
    ASSERT(A.pSym->precedence == 0); // 第一个必须不是运算符号


    while(1) {
      if(pBack->scope == RTSCOPE::npos) {
        ERROR_MSG__MISSING_SEMICOLON;
        return FALSE;
      }
      c.mode = *pBack == ')' ? SYNTAXNODE::MODE_FunctionCall : SYNTAXNODE::MODE_ArrayIndex;
      c.B.ptr = NULL;

      if( ! ParseArithmeticExpression(RTSCOPE(pBack->scope + 1, pBack - &m_aTokens.front()), &c.B)) {
        return FALSE;
      }

      if(scope.begin + 1 == pBack->scope) {
        break;
      }
      else {
        node_stack.push(c);
        pBack = &m_aTokens[pBack->scope - 1];
      }
    }

    while(1) {
      if( ! MakeSyntaxNode(pUnion, c.mode, &A, &c.B)) {
        CLBREAK;
        return FALSE;
      }

      if( ! node_stack.empty()) {
        c = node_stack.top();
        node_stack.pop();
        A = *pUnion;
      }
      else {
        break;
      }
    }

    return TRUE;
  }

  CodeParser::RTSCOPE::TYPE CodeParser::ParseFlowIf(const RTSCOPE& scope, SYNTAXNODE::UN* pUnion, GXBOOL bElseIf)
  {
    // 与 ParseFlowWhile 相似
    SYNTAXNODE::UN A = {0}, B = {0};
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

    const SYNTAXNODE::MODE eMode = TryGetNode(&B);
    if(eMode != SYNTAXNODE::MODE_Block) {
      if(eMode != SYNTAXNODE::MODE_Chain) {
        bret = bret && MakeSyntaxNode(&B, SYNTAXNODE::MODE_Chain, &B, NULL);
      }
      bret = bret && MakeSyntaxNode(&B, SYNTAXNODE::MODE_Block, &B, NULL);
    }

    bret = bret && MakeSyntaxNode(pUnion, SYNTAXNODE::MODE_Flow_If, &A, &B);

    auto result = sBlock.end;


    if(bret && (scope.end - sBlock.end) > 1 && m_aTokens[sBlock.end + 1] == "else")
    {
      auto nNextBegin = sBlock.end + 1;

      // 只处理 else if/else 两种情况
      A = *pUnion;
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
        const SYNTAXNODE::MODE eMode = TryGetNode(&B);
        if(eMode != SYNTAXNODE::MODE_Block) {
          if(eMode != SYNTAXNODE::MODE_Chain) {
            bret = bret && MakeSyntaxNode(&B, SYNTAXNODE::MODE_Chain, &B, NULL);
          }
          bret = bret && MakeSyntaxNode(&B, SYNTAXNODE::MODE_Block, &B, NULL);
        }
      }
           

      bret = bret && MakeSyntaxNode(pUnion, eNextMode, &A, &B);

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

  CodeParser::RTSCOPE::TYPE CodeParser::ParseFlowWhile(const RTSCOPE& scope, SYNTAXNODE::UN* pUnion)
  {
    // 与 ParseFlowIf 相似
    SYNTAXNODE::UN A = {0}, B = {0};
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

    const SYNTAXNODE::MODE eMode = TryGetNode(&B);
    if(eMode != SYNTAXNODE::MODE_Block) {
      if(eMode != SYNTAXNODE::MODE_Chain) {
        bret = bret && MakeSyntaxNode(&B, SYNTAXNODE::MODE_Chain, &B, NULL);
      }
      bret = bret && MakeSyntaxNode(&B, SYNTAXNODE::MODE_Block, &B, NULL);
    }

    //if(TryGetNode(&B) != SYNTAXNODE::MODE_Block) {
    //  bret = bret && MakeSyntaxNode(&B, SYNTAXNODE::MODE_Block, &B, NULL);
    //}

    bret = bret && MakeSyntaxNode(pUnion, SYNTAXNODE::MODE_Flow_While, NULL, &A, &B);

    DbgDumpScope("while", sConditional, sBlock);
    return bret ? sBlock.end : RTSCOPE::npos;
  }
  
  CodeParser::RTSCOPE::TYPE CodeParser::ParseFlowDoWhile(const RTSCOPE& scope, SYNTAXNODE::UN* pUnion)
  {
    ASSERT(m_aTokens[scope.begin] == "do");

    if(scope.begin + 1 >= scope.end) {
      // ERROR: do 语法错误
      return RTSCOPE::npos;
    }

    RTSCOPE sConditional;
    RTSCOPE sBlock;
    SYNTAXNODE::UN A = {0}, B = {0};


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
    bret = bret && MakeSyntaxNode(pUnion, SYNTAXNODE::MODE_Flow_DoWhile, NULL, &A, &B);

    RTSCOPE::TYPE while_end = sConditional.end + 1;
    if(while_end >= scope.end || m_aTokens[while_end] != ';') {
      // ERROR: 缺少 ";"
      return while_end;
    }

    return bret ? while_end : RTSCOPE::npos;
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

  CodeParser::RTSCOPE::TYPE CodeParser::MakeFlowForScope(const RTSCOPE& scope, RTSCOPE* pInit, RTSCOPE* pCond, RTSCOPE* pIter, RTSCOPE* pBlock, SYNTAXNODE::UN* pBlockNode)
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

  CodeParser::RTSCOPE::TYPE CodeParser::ParseFlowFor(const RTSCOPE& scope, SYNTAXNODE::UN* pUnion)
  {
    RTSCOPE sInitializer, sConditional, sIterator;
    RTSCOPE sBlock;

    SYNTAXNODE::UN uInit = {0}, uCond = {0}, uIter = {0}, uBlock = {0}, D;
    
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

    if( ! uBlock.ptr) {
      ParseExpression(sBlock, &uBlock);
      //MakeSyntaxNode(&uBlock, SYNTAXNODE::MODE_Chain, &uBlock, NULL);
    }

    GXBOOL bret = TRUE;
    // 如果不是代码块，就转换为代码块
    const SYNTAXNODE::MODE eMode = TryGetNode(&uBlock);
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
    bret = bret && MakeSyntaxNode(pUnion, SYNTAXNODE::MODE_Flow_For, &D, &uBlock);
    
    return bret ? sBlock.end : RTSCOPE::npos;
  }

  GXBOOL CodeParser::MakeInstruction(const TOKEN* pOpcode, int nMinPrecedence, const RTSCOPE* pScope, SYNTAXNODE::UN* pParent, int nMiddle)
  {
    ASSERT((int)pScope->begin <= nMiddle);
    ASSERT(nMiddle <= (int)pScope->end);

    RTSCOPE scopeA(pScope->begin, nMiddle);
    RTSCOPE scopeB(nMiddle + 1, pScope->end);
    SYNTAXNODE::UN A = {0}, B = {0};
    GXBOOL bresult = TRUE;

    if(*pOpcode == '?') {
      const TOKEN& s = m_aTokens[nMiddle];
      //SYNTAXNODE sNodeB;
      //B.pNode = &sNodeB;
      bresult = ParseArithmeticExpression(scopeA, &A, nMinPrecedence);

      if(s.scope >= (int)pScope->begin && s.scope < (int)pScope->end) {
        ASSERT(m_aTokens[s.scope] == ':');
        bresult = bresult && MakeInstruction(&m_aTokens[s.scope], nMinPrecedence, &scopeB, &B, s.scope);
      }
      else {
        // ERROR: ?:三元操作符不完整
      }
    }
    else {
      bresult = 
        ParseArithmeticExpression(scopeA, &A, nMinPrecedence) &&
        ParseArithmeticExpression(scopeB, &B, nMinPrecedence) ;
    }

    MakeSyntaxNode(pParent, SYNTAXNODE::MODE_Normal, pOpcode, &A, &B);

    DbgDumpScope(pOpcode->ToString(), scopeA, scopeB);

    if(pOpcode->unary) {
      if(A.pNode != NULL && B.pNode != NULL)
      {
        // ERROR: 一元操作符不能同时带有左右操作数
        return FALSE;
      }

      if(TEST_FLAG_NOT(pOpcode->unary_mask, UNARY_LEFT_OPERAND) && A.pNode != NULL)
      {
        // ERROR: 一元操作符不接受左值
        return FALSE;
      }

      if(TEST_FLAG_NOT(pOpcode->unary_mask, UNARY_RIGHT_OPERAND) && B.pNode != NULL)
      {
        // ERROR: 一元操作符不接受右值
        return FALSE;
      }
    }

    return bresult;
  }

  void CodeParser::DbgDumpScope( clStringA& str, clsize begin, clsize end, GXBOOL bRaw )
  {
    if(end - begin > 1 && m_aTokens[end - 1] == ';') {
      --end;
    }

    if(bRaw)
    {
      if(begin < end) {
        str.Append(m_aTokens[begin].marker.marker,
          (m_aTokens[end - 1].marker.marker - m_aTokens[begin].marker.marker) + m_aTokens[end - 1].marker.length);
      }
      else {
        str.Clear();
      }
    }
    else
    {
      for (clsize i = begin; i < end; ++i)
      {
        str.Append(m_aTokens[i].ToString());
      }
    }
  }

  void CodeParser::DbgDumpScope( clStringA& str, const RTSCOPE& scope )
  {
    DbgDumpScope(str, scope.begin, scope.end, FALSE);
  }

  void CodeParser::DbgDumpScope(GXLPCSTR opcode, const RTSCOPE& scopeA, const RTSCOPE& scopeB )
  {
    clStringA strA, strB;
    DbgDumpScope(strA, scopeA);
    DbgDumpScope(strB, scopeB);

    // <Make OperString>
    clStringA strIntruction;
    strIntruction.Format("[%s] [%s] [%s]", opcode, strA, strB);
    TRACE("%s\n", strIntruction);
    m_aDbgExpressionOperStack.push_back(strIntruction);
    // </Make OperString>
  }

  GXBOOL CodeParser::MakeSyntaxNode(SYNTAXNODE::UN* pDest, SYNTAXNODE::MODE mode, SYNTAXNODE::UN* pOperandA, SYNTAXNODE::UN* pOperandB)
  {
    return MakeSyntaxNode(pDest, mode, NULL, pOperandA, pOperandB);
  }

  GXBOOL CodeParser::MakeSyntaxNode(SYNTAXNODE::UN* pDest, SYNTAXNODE::MODE mode, const TOKEN* pOpcode, SYNTAXNODE::UN* pOperandA, SYNTAXNODE::UN* pOperandB)
  {
    const SYNTAXNODE::UN* pOperand[] = {pOperandA, pOperandB};
    SYNTAXNODE sNode = {0, mode, pOpcode};

    for(int i = 0; i < 2; ++i)
    {
      const int nFlagShift = SYNTAXNODE::FLAG_OPERAND_SHIFT * i;
      if(pOperand[i] == NULL) {
        sNode.Operand[i].pSym = NULL;
      }
      else if(TryGetNodeType(pOperand[i]) == SYNTAXNODE::FLAG_OPERAND_IS_TOKEN) {
        SET_FLAG(sNode.flags, SYNTAXNODE::FLAG_OPERAND_IS_TOKEN << nFlagShift);
        sNode.Operand[i].pSym = pOperand[i]->pSym;
      }
      else {
        SET_FLAG(sNode.flags, SYNTAXNODE::FLAG_OPERAND_IS_NODEIDX << nFlagShift);
        ASSERT((size_t)pOperand[i]->pNode < m_aSyntaxNodePack.size()); // 这时候还是索引，所以肯定小于序列的长度
        sNode.Operand[i].pNode = pOperand[i]->pNode;
      }
    }

    pDest->pNode = (SYNTAXNODE*)m_aSyntaxNodePack.size();
    m_aSyntaxNodePack.push_back(sNode);

    return TRUE;
  }

  //GXBOOL CodeParser::IsToken(const SYNTAXNODE::UN* pUnion) const
  //{
  //  const TOKEN* pBegin = &m_aTokens.front();
  //  const TOKEN* pBack   = &m_aTokens.back();

  //  return pUnion->pSym >= pBegin && pUnion->pSym <= pBack;
  //}

  //clsize CodeParser::FindSemicolon( clsize begin, clsize end ) const
  //{
  //  for(; begin < end; ++begin)
  //  {
  //    if(m_aTokens[begin] == ';') {
  //      return begin;
  //    }
  //  }
  //  return -1;
  //}

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
        IndexToPtr(it->expr.sRoot.pNode, m_aSyntaxNodePack);
        RelocaleSyntaxPtr(it->expr.sRoot.pNode);
        break;

      case StatementType_Definition:
        if(TryGetNodeType(&it->defn.sRoot) == SYNTAXNODE::FLAG_OPERAND_IS_NODEIDX)
        {
          IndexToPtr(it->defn.sRoot.pNode, m_aSyntaxNodePack);
          RelocaleSyntaxPtr(it->defn.sRoot.pNode);
        }
        break;
      }
    }
  }

  CodeParser::SYNTAXNODE::FLAGS CodeParser::TryGetNodeType( const SYNTAXNODE::UN* pUnion ) const
  {
    if(pUnion->ptr >= &m_aTokens.front() && pUnion->ptr <= &m_aTokens.back()) {
      return SYNTAXNODE::FLAG_OPERAND_IS_TOKEN;
    }
    else if(pUnion->ptr >= &m_aSyntaxNodePack.front() && pUnion->ptr <= &m_aSyntaxNodePack.back()) {
      return SYNTAXNODE::FLAG_OPERAND_IS_NODE;
    }
    else {
      return SYNTAXNODE::FLAG_OPERAND_IS_NODEIDX;
    }
  }

  CodeParser::SYNTAXNODE::MODE CodeParser::TryGetNode( const SYNTAXNODE::UN* pUnion ) const
  {
    if(pUnion->ptr >= &m_aTokens.front() && pUnion->ptr <= &m_aTokens.back()) {
      return SYNTAXNODE::MODE_Undefined;
    }
    else if(pUnion->ptr >= &m_aSyntaxNodePack.front() && pUnion->ptr <= &m_aSyntaxNodePack.back()) {
      return pUnion->pNode->mode;
    }
    else {
      return m_aSyntaxNodePack[(int)pUnion->pNode].mode;
    }
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

} // namespace UVShader
