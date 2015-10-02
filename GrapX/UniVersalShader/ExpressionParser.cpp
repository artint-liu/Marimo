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

    ASSERT(m_nMaxPrecedence <= (1 << (SYMBOL::precedence_bits - 1))); // 检测位域表示范围没有超过优先级
    ASSERT(m_nMaxPrecedence != SYMBOL::ID_BRACE); // 保证优先级最大值与括号的ID不冲突

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
    m_aSymbols.clear();
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
    ASSERT((int)remain >= 0);
    return 0;
  }

  clsize CodeParser::EstimateForSymbolsCount() const
  {
    auto count = GetStreamCount();
    return (count << 1) + (count >> 1); // 按照 字节数：符号数=2.5：1来计算
  }

  clsize CodeParser::GenerateSymbols()
  {
    auto stream_end = end();
    ASSERT(m_aSymbols.empty()); // 调用者负责清空

    m_aSymbols.reserve(EstimateForSymbolsCount());
    typedef clstack<int> PairStack;
    //PairStack stackBrackets;        // 圆括号
    //PairStack stackSquareBrackets;  // 方括号
    //PairStack stackBrace;           // 花括号
    SYMBOL sym;
    
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
      const int c_size = (int)m_aSymbols.size();
      sym.Set(it);
      sym.scope = -1;
      sym.semi_scope = -1;

      ASSERT(m_CurSymInfo.marker.marker == NULL || it.marker == m_CurSymInfo.marker.marker); // 遍历时一定这个要保持一致

      sym.precedence = m_CurSymInfo.precedence;
      sym.unary      = m_CurSymInfo.unary;
      sym.unary_mask = m_CurSymInfo.unary_mask;

      // 如果是 -,+ 检查前一个符号是不是操作符或者括号，如果是就认为这个 -,+ 是正负号
      if((it == '-' || it == '+') && ! m_aSymbols.empty())
      {        
        const auto& l_back = m_aSymbols.back();

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
          m_aSymbols[sym.scope].scope = c_size;
          c.sStack.pop();
        }
        else {
          continue;
        }

        // ?: 操作符会设置precedence的优先级，而我们只想设置括号的标记
        if(sym.precedence == 0) {
          sym.precedence = SYMBOL::ID_BRACE;
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
          m_aSymbols[bracket_stack.top() - 1] == "for" ) {
          EOE = bracket_stack.top() - 1;
          }
          m_aSymbols[EOE].semi_scope = c_size;

        }
        EOE = c_size + 1;
      }
      // 这个可能不需要，因为按照语法for应该是在";"分号或者"}"闭花括号后面
      //else if(it == "for") { // for(;;) 这个形式有点特殊
      //  EOE = c_size;
      //}

      m_aSymbols.push_back(sym);

      /*/
      m_aSymbols.push_back(sym);

      if(it == ';') {
        ASSERT(EOE < (int)m_aSymbols.size());
        for(auto it = m_aSymbols.begin() + EOE; it != m_aSymbols.end(); ++it)
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

    return m_aSymbols.size();
  }

  const CodeParser::SymbolArray* CodeParser::GetSymbolsArray() const
  {
    return &m_aSymbols;
  }

  GXBOOL CodeParser::Parse()
  {
    RTSCOPE scope(0, m_aSymbols.size());
    while(ParseStatement(&scope));
    RelocalePointer();
    return TRUE;
  }

  GXBOOL CodeParser::ParseStatement(RTSCOPE* pScope)
  {    
    return (pScope->begin < pScope->end) && (
      ParseStatementAs_Function(pScope) || ParseStatementAs_Struct(pScope));
  }

  GXBOOL CodeParser::ParseStatementAs_Function(RTSCOPE* pScope)
  {
    // 函数语法
    //[StorageClass] Return_Value Name ( [ArgumentList] ) [: Semantic]
    //{
    //  [StatementBlock]
    //};

    SYMBOL* p = &m_aSymbols[pScope->begin];
    const SYMBOL* pEnd = &m_aSymbols.front() + pScope->end;

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
    ASSERT(p->scope >= 0);  // 由 GenerateSymbols 函数保证

    // #
    // # ( [ArgumentList] )
    // #
    if(p[0].scope != p[1].scope + 1) // 有参数: 两个括号不相邻
    {
      RTSCOPE ArgScope(m_aSymbols[p->scope].scope + 1, p->scope);
      ParseFunctionArguments(&stat, &ArgScope);
    }
    p = &m_aSymbols[p->scope];
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
      RTSCOPE func_statement_block(m_aSymbols[p->scope].scope + 1, p->scope);

      stat.type = StatementType_Function;
      p = &m_aSymbols[p->scope];
      ++p;

      if(func_statement_block.IsValid())
      {
        STATEMENT sub_stat = {StatementType_Expression};
        if(ParseStatementAs_Expression(&sub_stat, &func_statement_block, FALSE))
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
    pScope->begin = p - &m_aSymbols.front();
    return TRUE;
  }

  GXBOOL CodeParser::ParseStatementAs_Struct( RTSCOPE* pScope )
  {
    SYMBOL* p = &m_aSymbols[pScope->begin];
    const SYMBOL* pEnd = &m_aSymbols.front() + pScope->end;

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

    RTSCOPE StruScope(m_aSymbols[p->scope].scope + 1, p->scope);
    // 保证分析函数的域
    ASSERT(m_aSymbols[StruScope.begin - 1] == "{" && m_aSymbols[StruScope.end] == "}"); 

    pScope->begin = StruScope.end + 1;
    if(m_aSymbols[pScope->begin] != ";") {
      // ERROR: 缺少“；”
      return FALSE;
    }
    ++pScope->begin;

    // #
    // # 解析成员变量
    // #
    ParseStructMember(&stat, &StruScope);

    m_aStatements.push_back(stat);
    return TRUE;
  }

  GXBOOL CodeParser::ParseFunctionArguments(STATEMENT* pStat, RTSCOPE* pArgScope)
  {
    // 函数参数格式
    // [InputModifier] Type Name [: Semantic]
    // 函数可以包含多个参数，用逗号分隔

    SYMBOL* p = &m_aSymbols[pArgScope->begin];
    const SYMBOL* pEnd = &m_aSymbols.front() + pArgScope->end;
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
    if(pNode->OperandA_IsNode() && pNode->Operand[0].pNode) {
      //pNode->Operand[0].pNode = &m_aSyntaxNodePack[(GXINT_PTR)pNode->Operand[0].pNode];
      IndexToPtr(pNode->Operand[0].pNode, m_aSyntaxNodePack);
      RelocaleSyntaxPtr((SYNTAXNODE*)pNode->Operand[0].pNode);
    }

    if(pNode->OperandB_IsNode() && pNode->Operand[1].pNode) {
      //pNode->Operand[1].pNode = &m_aSyntaxNodePack[(GXINT_PTR)pNode->Operand[1].pNode];
      IndexToPtr(pNode->Operand[1].pNode, m_aSyntaxNodePack);
      RelocaleSyntaxPtr((SYNTAXNODE*)pNode->Operand[1].pNode);
    }
  }

  GXBOOL CodeParser::ParseStructMember( STATEMENT* pStat, RTSCOPE* pStruScope )
  {
    // 作为结构体成员
    // Type[RxC] MemberName; 
    // 作为Shader标记
    // Type[RxC] MemberName : ShaderFunction; 
    // 这个结构体成员必须一致，要么全是普通成员变量，要么全是Shader标记

    SYMBOL* p = &m_aSymbols[pStruScope->begin];
    const SYMBOL* pEnd = &m_aSymbols.front() + pStruScope->end;
    MemberArray aMembers;

    while(p < pEnd)
    {
      STRUCT_MEMBER member = {NULL};
      member.szType = GetUniqueString(p);
      INC_BUT_NOT_END(p, pEnd); // ERROR: 结构体成员声明不正确

      member.szName = GetUniqueString(p);
      INC_BUT_NOT_END(p, pEnd); // ERROR: 结构体成员声明不正确

      if(*p == ";") {
        if(pStat->type == StatementType_Empty || pStat->type == StatementType_Struct) {
          pStat->type = StatementType_Struct;
          ++p;
        }
        else {
          // ERROR: 结构体成员作用不一致。纯结构体和Shader标记结构体混合定义
          return FALSE;
        }
      }
      else if(*p == ":") {
        if(pStat->type == StatementType_Empty || pStat->type == StatementType_Signatures) {
          pStat->type = StatementType_Signatures;
          INC_BUT_NOT_END(p, pEnd);
          member.szSignature = GetUniqueString(p);
          
          // TODO: 检查这个是Signature

          INC_BUT_NOT_END(p, pEnd);
          if(*p != ";") {
            // ERROR: 缺少“；”
            return FALSE;
          }
          ++p;
        }
        else {
          // ERROR: 结构体成员作用不一致。纯结构体和Shader标记结构体混合定义
          return FALSE;
        }
      }
      else {
        // ERROR: 缺少“；”
        return FALSE;
      }

      aMembers.push_back(member);
    }

    pStat->stru.pMembers = (STRUCT_MEMBER*)m_aMembersPack.size();
    pStat->stru.nNumOfMembers = aMembers.size();
    m_aMembersPack.insert(m_aMembersPack.begin(), aMembers.begin(), aMembers.end());
    return TRUE;
  }

  GXLPCSTR CodeParser::GetUniqueString( const SYMBOL* pSym )
  {
    return m_Strings.add(pSym->ToString());
  }

  const CodeParser::TYPE* CodeParser::ParseType(const SYMBOL* pSym)
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

  GXBOOL CodeParser::ParseStatementAs_Expression(STATEMENT* pStat, RTSCOPE* pScope, GXBOOL bDbgRelocale )
  {
    m_nDbgNumOfExpressionParse = 0;
    m_aDbgExpressionOperStack.clear();

    if(pScope->begin == pScope->end) {
      return TRUE;
    }

    STATEMENT& stat = *pStat;
    
    GXBOOL bret = ParseExpression(pScope, &stat.expr.sRoot);
    TRACE("m_nDbgNumOfExpressionParse=%d\n", m_nDbgNumOfExpressionParse);

#ifdef _DEBUG
    // 这个测试会重定位指针，所以仅作为一次性调用，之后m_aSyntaxNodePack不能再添加新的数据了
    if( ! bret)
    {
      TRACE("编译错误\n");
      m_aDbgExpressionOperStack.clear();
    }
    else if(bDbgRelocale && ! IsSymbol(&stat.expr.sRoot)) {
      IndexToPtr(stat.expr.sRoot.pNode, m_aSyntaxNodePack);
      RelocaleSyntaxPtr(stat.expr.sRoot.pNode);
      DbgDumpSyntaxTree(stat.expr.sRoot.pNode, 0);
    }
#endif // #ifdef _DEBUG
    //m_aStatements.push_back(stat);
    return bret;
  }

  void CodeParser::DbgDumpSyntaxTree(const SYNTAXNODE* pNode, int precedence, clStringA* pStr)
  {
    clStringA str[2];
    for(int i = 0; i < 2; i++)
    {
      if(pNode->Operand[i].pSym) {
        if(IsSymbol(&pNode->Operand[i])) {
          str[i] = pNode->Operand[i].pSym->ToString();
        }
        else {
          DbgDumpSyntaxTree(pNode->Operand[i].pNode, pNode->pOpcode ? pNode->pOpcode->precedence : 0, &str[i]);
        }
      }
      else {
        str[i].Clear();
      }
    }

    TRACE("[%s] [%s] [%s]\n",
      pNode->pOpcode ? pNode->pOpcode->ToString() : "",
      str[0], str[1]);

    clStringA strOut;
    switch(pNode->mode)
    {
    case SYNTAXNODE::MODE_FunctionCall: // 函数调用
      strOut.Format("%s(%s)", str[0], str[1]);
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

  GXBOOL CodeParser::ParseExpression(SYNTAXNODE::UN* pUnion, clsize begin, clsize end)
  {
    RTSCOPE scope(begin, end);
    return ParseExpression(&scope, pUnion);
  }

  GXBOOL CodeParser::ParseArithmeticExpression(clsize begin, clsize end, SYNTAXNODE::UN* pUnion)
  {
    RTSCOPE scope(begin, end);
    return ParseArithmeticExpression(&scope, pUnion);
  }

  GXBOOL CodeParser::ParseArithmeticExpression(RTSCOPE* pScope, SYNTAXNODE::UN* pUnion)
  {
    if(pScope->end > pScope->begin && m_aSymbols[pScope->end - 1] == ';') {
      pScope->end--;
    }
    return ParseArithmeticExpression(pScope, pUnion, SYMBOL::FIRST_OPCODE_PRECEDENCE);
  }

  GXBOOL CodeParser::ParseRemainStatement(RTSCOPE::TYPE parse_end, const RTSCOPE& scope, SYNTAXNODE::UN* pUnion)
  {
    GXBOOL bret = TRUE;
    if(parse_end != RTSCOPE::npos && (scope.end - parse_end) > 1)
    {
      SYNTAXNODE::UN A, B;
      A = *pUnion;

      bret = ParseExpression(&B, parse_end + 1, scope.end) &&
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

    const auto& first = m_aSymbols[scope.begin];
    auto& pend = *parse_end;
    GXBOOL bret = TRUE;

    ASSERT(pend == RTSCOPE::npos); // 强调调用者要初始化这个变量

    if(first == "else") {
      // ERROR: "else" 不能独立使用
    }
    else if(first == "for") {
      pend = ParseFlowFor(scope, pUnion);
    }
    else if(first == "if") {
      pend = ParseFlowIf(scope, pUnion, FALSE);
    }
    else if(first == "while") {
      pend = ParseFlowWhile(scope, pUnion);
    }
    else if(first == "do") {
      pend = ParseFlowDoWhile(scope, pUnion);
    }
    else {
      bret = FALSE;
    }

    ASSERT(( ! bret && pend == RTSCOPE::npos) || bret);
    ASSERT(pend == RTSCOPE::npos || (pend > scope.begin && pend <= scope.end));
    return bret;
  }

  GXBOOL CodeParser::ParseExpression( RTSCOPE* pScope, SYNTAXNODE::UN* pUnion)
  {
    ASSERT(pScope->end == m_aSymbols.size() || m_aSymbols[pScope->end] == ';' || 
      m_aSymbols[pScope->end] == '}');


    const GXINT_PTR count = pScope->end - pScope->begin;
    SYNTAXNODE::UN A = {0}, B = {0};
    GXBOOL bret = TRUE;
    RTSCOPE::TYPE parse_end = RTSCOPE::npos;

    if(count <= 1) {
      if(count < 0) {
        CLBREAK;
        return FALSE;
      }
      else if(count == 1) {
        pUnion->pSym = &m_aSymbols[pScope->begin];
      }
      
      return TRUE;  // count == 0 / count == 1
    }

    const auto& first = m_aSymbols[pScope->begin];

    if(first == '{') // 代码块
    {
      if((clsize)first.scope > pScope->end) {
        // ERROR: 没有正确的'}'来匹配
      }

      bret = ParseExpression(&A, pScope->begin + 1, first.scope);
      if((clsize)first.scope + 1 >= pScope->end) {
        *pUnion = A;
      }
      else {
        bret = bret && ParseExpression(&B, first.scope + 1, pScope->end);
        bret = bret && MakeSyntaxNode(pUnion, SYNTAXNODE::MODE_Chain, NULL, &A, &B);
      }
      return bret;
    }

    else if(TryKeywords(*pScope, pUnion, &parse_end))
    {
      if(parse_end == RTSCOPE::npos) {
        return FALSE; // 解析错误, 直接返回
      }
      // 解析剩余部分
      return ParseRemainStatement(parse_end, *pScope, pUnion);
    }


    //else if(first == "else")
    //{
    //  // ERROR: "else" 不能独立使用
    //  return FALSE;
    //}
    //else if(first == "for")
    //{
    //  auto parse_end = ParseFlowFor(pScope, pUnion);
    //  return ParseRemainStatement(parse_end, pScope, pUnion);
    //}
    //else if(first == "if")
    //{
    //  auto parse_end = ParseFlowIf(pScope, pUnion, FALSE);
    //  return ParseRemainStatement(parse_end, pScope, pUnion);
    //}
    //else if(first == "while")
    //{
    //  auto parse_end = ParseFlowWhile(pScope, pUnion);
    //  return ParseRemainStatement(parse_end, pScope, pUnion);
    //}
    //else if(first == "do")
    //{
    //  auto parse_end = ParseFlowDoWhile(pScope, pUnion);
    //  return ParseRemainStatement(parse_end, pScope, pUnion);
    //}

    else if(first == "return")
    {
      A.pSym = &first;
      bret = ParseArithmeticExpression(pScope->begin + 1, pScope->end, &B);
      MakeSyntaxNode(pUnion, SYNTAXNODE::MODE_Return, NULL, &A, &B);
      return bret;
    }
    else if(first == "break") {
      A.pSym = &first;
      bret = MakeSyntaxNode(pUnion, SYNTAXNODE::MODE_Flow_Break, NULL, &A, NULL);
      ASSERT(count == 1); // 未处理后面的情况
      return bret;
    }
    else if(first == "continue") {
      A.pSym = &first;
      bret = MakeSyntaxNode(pUnion, SYNTAXNODE::MODE_Flow_Continue, NULL, &A, NULL);
      ASSERT(count == 1); // 未处理后面的情况
      return bret;
    }
    else if(first == "discard") {
      A.pSym = &first;
      bret = MakeSyntaxNode(pUnion, SYNTAXNODE::MODE_Flow_Discard, NULL, &A, NULL);
      ASSERT(count == 1); // 未处理后面的情况
      return bret;
    }
    //else if(first.scope >= 0 && (clsize)first.scope < pScope->end && first.precedence != SYMBOL::ID_BRACE)
    else if(first.semi_scope >= 0 && (clsize)first.semi_scope < pScope->end)
    {
      ASSERT(m_aSymbols[first.semi_scope] == ';'); // 目前进入这个循环的只可能是遇到分号
      bret =
        ParseExpression(&A, pScope->begin, first.semi_scope) &&
        ParseExpression(&B, first.semi_scope + 1, pScope->end) &&
        MakeSyntaxNode(pUnion, SYNTAXNODE::MODE_Chain, NULL, &A, &B);

      return bret;
    }

    ParseArithmeticExpression(pScope, pUnion);
    return TRUE;
  }

  //////////////////////////////////////////////////////////////////////////

  GXBOOL CodeParser::ParseArithmeticExpression(RTSCOPE* pScope, SYNTAXNODE::UN* pUnion, int nMinPrecedence)
  {
    int nCandidate = m_nMaxPrecedence;
    GXINT_PTR i = (GXINT_PTR)pScope->end - 1;
    GXINT_PTR nCandidatePos = i;
    const GXINT_PTR count = pScope->end - pScope->begin;

    if(count <= 1) {
      if(count == 1) {
        pUnion->pSym = &m_aSymbols[pScope->begin];
      }
      return TRUE;
    }

    const auto& first = m_aSymbols[pScope->begin];

    if(count == 2)
    {
      // 处理两种可能：(1)变量使用一元符号运算 (2)定义变量
      SYNTAXNODE::UN A = {0}, B = {0};

      A.pSym = &first;
      B.pSym = &m_aSymbols[pScope->begin + 1];
      GXBOOL bret = TRUE;

      ASSERT(*B.pSym != ';'); // 已经在外部避免了表达式内出现分号

      /*if(B.pSym->sym == ';')
      {
        pUnion->pSym = A.pSym;
        return TRUE;
      }
      else */if(A.pSym->precedence > 0)
      {
        bret = MakeSyntaxNode(pUnion, SYNTAXNODE::MODE_Normal, A.pSym, NULL, &B);
        DbgDumpScope(A.pSym->ToString(), RTSCOPE(0,0), RTSCOPE(pScope->begin + 1, pScope->end));
      }
      else if(B.pSym->precedence > 0)
      {
        bret = MakeSyntaxNode(pUnion, SYNTAXNODE::MODE_Normal, B.pSym, &A, NULL);
        DbgDumpScope(B.pSym->ToString(), RTSCOPE(pScope->begin, pScope->begin + 1), RTSCOPE(0,0));
      }
      else {
        // 变量声明
        bret = MakeSyntaxNode(pUnion, SYNTAXNODE::MODE_Definition, NULL, &A, &B);
      }
      return bret;
    }
    else if((first == '(' || first == '[') && first.scope == pScope->end - 1)  // 括号内表达式
    {
      // 括号肯定是匹配的
      ASSERT(m_aSymbols[pScope->end - 1].scope == pScope->begin);
      return ParseArithmeticExpression(&RTSCOPE(pScope->begin + 1, pScope->end - 1), pUnion, SYMBOL::FIRST_OPCODE_PRECEDENCE);
    }
    else if(m_aSymbols[pScope->begin + 1].scope == pScope->end - 1)  // 整个表达式是函数调用
    {
      return ParseFunctionCall(pScope, pUnion);
    }

    while(nMinPrecedence <= m_nMaxPrecedence)
    {
      if(nMinPrecedence == OPP(1))
      {
        for(i = (GXINT_PTR)pScope->begin; i < (GXINT_PTR)pScope->end; ++i)
        {
          m_nDbgNumOfExpressionParse++;

          const SYMBOL& s = m_aSymbols[i];

          if(s.precedence == SYMBOL::ID_BRACE) // 跳过非运算符, 也包括括号
          {
            ASSERT(s.scope < (int)pScope->end); // 闭括号肯定在表达式区间内
            i = s.scope;
            continue;
          }
          else if(s.precedence == 0 || s == ':') {
            continue;
          }

          // ?: 操作符标记：precedence 储存优先级，scope 储存?:的关系

          if(s.precedence == nMinPrecedence) {
            MakeInstruction(&s, nMinPrecedence, pScope, pUnion, i);
            return TRUE;
          }
          else if(s.precedence < nCandidate) {
            nCandidate = s.precedence;
            // 这里优先级因为从LTR切换到RTL，所以不记录 nCandidatePos
          }
        } // for

        nCandidatePos = (GXINT_PTR)pScope->end - 1;
      }
      else
      {
        for(; i >= (GXINT_PTR)pScope->begin; --i)
        {
          m_nDbgNumOfExpressionParse++;
          const SYMBOL& s = m_aSymbols[i];

          // 优先级（2）是从右向左的，这个循环处理从左向右
          ASSERT(nMinPrecedence != 2);

          // 跳过非运算符, 也包括括号
          if(s.precedence == SYMBOL::ID_BRACE)
          {
            ASSERT(s.scope < (int)pScope->end); // 闭括号肯定在表达式区间内
            i = s.scope;
            continue;
          }
          else if(s.precedence == 0) {
            continue;
          }

          if(s.precedence == nMinPrecedence) {
            MakeInstruction(&s, nMinPrecedence, pScope, pUnion, i);
            return TRUE;
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
    return TRUE;
  }

  //////////////////////////////////////////////////////////////////////////

  GXBOOL CodeParser::ParseFunctionCall(RTSCOPE* pScope, SYNTAXNODE::UN* pUnion)
  {
    // 括号肯定是匹配的
    ASSERT(m_aSymbols[pScope->end - 1].scope == pScope->begin + 1);

    SYNTAXNODE::UN A, B;

    A.pSym = &m_aSymbols[pScope->begin];
    clStringA strFunction = A.pSym->ToString();

    // TODO: 检查m_aSymbols[pScope->begin]是函数名

    //GXBOOL bret = ParseExpression(&B, pScope->begin + 2, pScope->end - 1);
    GXBOOL bret = ParseArithmeticExpression(&RTSCOPE(pScope->begin + 2, pScope->end - 1), &B, SYMBOL::FIRST_OPCODE_PRECEDENCE);

    SYNTAXNODE::MODE mode = SYNTAXNODE::MODE_FunctionCall;

    MakeSyntaxNode(pUnion, mode, NULL, &A, &B);
    DbgDumpScope("F", RTSCOPE(pScope->begin, pScope->begin + 1),
      RTSCOPE(pScope->begin + 2, pScope->end - 1));

    return bret;
  }

  CodeParser::RTSCOPE::TYPE CodeParser::ParseFlowIf(const RTSCOPE& scope, SYNTAXNODE::UN* pUnion, GXBOOL bElseIf)
  {
    // 与 ParseFlowWhile 相似
    SYNTAXNODE::UN A = {0}, B = {0};
    GXBOOL bret = TRUE;
    ASSERT(m_aSymbols[scope.begin] == "if");

    RTSCOPE sConditional(scope.begin + 2, m_aSymbols[scope.begin + 1].scope);
    RTSCOPE sBlock;

    if(sConditional.begin >= scope.end || sConditional.end == -1 || sConditional.end > scope.end)
    {
      // ERROR: if 语法错误
      return RTSCOPE::npos;
    }

    bret = bret && ParseArithmeticExpression(&sConditional, &A);



    sBlock.begin = sConditional.end + 1;
    sBlock.end = RTSCOPE::npos;
    if(sBlock.begin >= scope.end) {
      // ERROR: if 语法错误
      return RTSCOPE::npos;
    }

    auto& block_begin = m_aSymbols[sBlock.begin];
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
        sBlock.begin++;
      }
      else {
        sBlock.end = block_begin.semi_scope;
      }

      if(sBlock.end > scope.end)
      {
        // ERROR: if 语法错误
        return RTSCOPE::npos;
      }
      bret = bret && ParseExpression(&sBlock, &B);
    }

    //if(block_begin == '{')
    //{
    //  sBlock.end = block_begin.scope;
    //  sBlock.begin++;
    //}
    //else if(TryKeywords(RTSCOPE(sBlock.begin, scope.end), &B, &sBlock.end))
    //{
    //  eMode = SYNTAXNODE::MODE_Flow_If;
    //  bret = sBlock.end != RTSCOPE::npos;
    //}
    //else {
    //  sBlock.end = block_begin.semi_scope;
    //}

    //if(eMode == SYNTAXNODE::MODE_Undefined && sBlock.end > scope.end)
    //{
    //  // ERROR: if 语法错误
    //  return RTSCOPE::npos;
    //}


    //if(eMode == SYNTAXNODE::MODE_Flow_If)
    //{
    //  sBlock.end = ParseFlowIf(RTSCOPE(sBlock.begin, scope.end), &B, FALSE);
    //  bret = sBlock.end != RTSCOPE::npos;
    //}
    //else
    //if(eMode != SYNTAXNODE::MODE_Flow_If)
    //{
    //  ASSERT(eMode == SYNTAXNODE::MODE_Undefined);
    //  bret = bret && ParseExpression(&sBlock, &B);
    //}
    bret = bret && MakeSyntaxNode(pUnion, SYNTAXNODE::MODE_Flow_If, NULL, &A, &B);
    //DbgDumpScope("if", sConditional, sBlock);


    auto result = sBlock.end;


    if(bret && (scope.end - sBlock.end) > 1 && m_aSymbols[sBlock.end + 1] == "else")
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

      if(m_aSymbols[nNextBegin] == "if")
      {
        eNextMode = SYNTAXNODE::MODE_Flow_ElseIf;
        result = ParseFlowIf(RTSCOPE(nNextBegin, scope.end), &B, TRUE);
      }
      else
      {
        auto& else_begin = m_aSymbols[nNextBegin];
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

          bret = ParseExpression(&B, nNextBegin, result);
        }
      }

      //if(eNextMode == SYNTAXNODE::MODE_Chain) {
      //  DbgDumpScope(bElseIf ? "elif" : "if", sConditional, sBlock);
      //}


      bret = bret && MakeSyntaxNode(pUnion, eNextMode, NULL, &A, &B);

      if(eNextMode != SYNTAXNODE::MODE_Chain) {
        if(eNextMode == SYNTAXNODE::MODE_Flow_Else) {
          DbgDumpScope("else", RTSCOPE(0,0), RTSCOPE(nNextBegin, scope.end));
        }
        DbgDumpScope(bElseIf ? "elif" : "if", sConditional, sBlock);
      }
    }
    else {
      DbgDumpScope("if", sConditional, sBlock);
    }

    return bret ? result : RTSCOPE::npos;
  }

  CodeParser::RTSCOPE::TYPE CodeParser::ParseFlowWhile(const RTSCOPE& scope, SYNTAXNODE::UN* pUnion)
  {
    // 与 ParseFlowIf 相似
    SYNTAXNODE::UN A = {0}, B = {0};
    GXBOOL bret = TRUE;
    ASSERT(m_aSymbols[scope.begin] == "while");


    RTSCOPE sConditional(scope.begin + 2, m_aSymbols[scope.begin + 1].scope);
    RTSCOPE sBlock;

    if( ! MakeScope(&sConditional, &MAKESCOPE(scope, scope.begin + 2, FALSE, scope.begin + 1, TRUE, 0))) {
      return RTSCOPE::npos;
    }
    
    bret = bret && ParseArithmeticExpression(&sConditional, &A);


    //if(sConditional.begin >= scope.end || sConditional.end == -1 || sConditional.end > scope.end)
    //{
    //  // ERROR: while 语法错误
    //  return RTSCOPE::npos;
    //}

    sBlock.begin = sConditional.end + 1;
    sBlock.end   = RTSCOPE::npos;
    if(sBlock.begin >= scope.end) {
      // ERROR: while 语法错误
      return RTSCOPE::npos;
    }

    auto& block_begin = m_aSymbols[sBlock.begin];
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
        sBlock.begin++;
      }
      else {
        sBlock.end = block_begin.semi_scope;
      }

      if(sBlock.end > scope.end)
      {
        // ERROR: while 语法错误
        return RTSCOPE::npos;
      }
      bret = bret && ParseExpression(&sBlock, &B);
    }

    //if(block_begin == '{')
    //{
    //  sBlock.end = block_begin.scope;
    //  sBlock.begin++;
    //}
    //else if(TryKeywords(RTSCOPE(sBlock.begin, scope.end), &B, &sBlock.end))
    //{
    //  eMode = SYNTAXNODE::MODE_Flow_While;
    //  bret = sBlock.end != RTSCOPE::npos;
    //}
    ////else if(block_begin == "while")
    ////{
    ////  sBlock.end = RTSCOPE::npos;
    ////  eMode = SYNTAXNODE::MODE_Flow_While;
    ////}
    //// "for"
    //// "if"
    //else {
    //  sBlock.end = block_begin.semi_scope;
    //}

    //if(eMode == SYNTAXNODE::MODE_Undefined && sBlock.end > scope.end)
    //{
    //  // ERROR: while 语法错误
    //  return RTSCOPE::npos;
    //}

    //if(eMode == SYNTAXNODE::MODE_Flow_While)
    //{
    //  sBlock.end = ParseFlowWhile(RTSCOPE(sBlock.begin, scope.end), &B);
    //  bret = sBlock.end != RTSCOPE::npos;
    //}
    //else
    //if(eMode != SYNTAXNODE::MODE_Flow_While)
    //{
    //  ASSERT(eMode == SYNTAXNODE::MODE_Undefined);
    //  bret = bret && ParseExpression(&sBlock, &B);
    //}
    bret = bret && MakeSyntaxNode(pUnion, SYNTAXNODE::MODE_Flow_While, NULL, &A, &B);

    DbgDumpScope("while", sConditional, sBlock);
    return bret ? sBlock.end : RTSCOPE::npos;
  }
  
  CodeParser::RTSCOPE::TYPE CodeParser::ParseFlowDoWhile(const RTSCOPE& scope, SYNTAXNODE::UN* pUnion)
  {
    ASSERT(m_aSymbols[scope.begin] == "do");

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
    
    if(while_token >= scope.end && m_aSymbols[while_token] != "while") {
      // ERROR: while 语法错误
      return RTSCOPE::npos;
    }

    if( ! MakeScope(&sConditional, &MAKESCOPE(scope, while_token + 2, FALSE, while_token + 1, TRUE, 0))) {
      return RTSCOPE::npos;
    }

    // TODO： 验证域的开始是括号和花括号

    GXBOOL bret = ParseExpression(&sBlock, &B);
    bret = bret && ParseArithmeticExpression(&sConditional, &A);
    bret = bret && MakeSyntaxNode(pUnion, SYNTAXNODE::MODE_Flow_DoWhile, NULL, &A, &B);

    RTSCOPE::TYPE while_end = sConditional.end + 1;
    if(while_end >= scope.end || m_aSymbols[while_end] != ';') {
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
      begin = m_aSymbols[p.begin].GetScope();

      if(OUT_OF_SCOPE(begin) || begin >= p.pScope->end) {
        // ERROR:
        return FALSE;
      }
    }
    else {
      begin = p.begin;
    }

    if(p.chTermin != 0 && m_aSymbols[begin] == (TChar)p.chTermin) {
      end = begin;
      return TRUE;
    }

    if(p.end >= p.pScope->end) {
      // ERROR:
      return FALSE;
    }

    if(p.bEndMate) {
      end = m_aSymbols[p.end].GetScope();

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
    ASSERT(m_aSymbols[scope.begin] == "for"); // 外部保证调用这个函数的正确性

    auto open_bracket = scope.begin + 1;
    if(open_bracket >= scope.end || m_aSymbols[open_bracket] != '(' || 
      (pIter->end = m_aSymbols[open_bracket].scope) > scope.end)
    {
      // ERROR: for 格式错误
      return RTSCOPE::npos;
    }

    //
    // initializer
    // 初始化部分
    //
    pInit->begin  = scope.begin + 2;
    pInit->end    = m_aSymbols[scope.begin].semi_scope;
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
    pCond->end    = m_aSymbols[pCond->begin].semi_scope;

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

    auto& block_begin = m_aSymbols[pBlock->begin];
    if(block_begin == '{')
    {
      pBlock->end = block_begin.scope;
      pBlock->begin++;
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

    ASSERT(m_aSymbols[sBlock.begin] == "for" || m_aSymbols[sBlock.end] == ';' || m_aSymbols[sBlock.end] == '}');

    ParseArithmeticExpression(&sInitializer, &uInit, SYMBOL::FIRST_OPCODE_PRECEDENCE);
    ParseArithmeticExpression(&sConditional, &uCond, SYMBOL::FIRST_OPCODE_PRECEDENCE);
    ParseArithmeticExpression(&sIterator   , &uIter, SYMBOL::FIRST_OPCODE_PRECEDENCE);
    //if(sBlock.end == RTSCOPE::npos)
    //{
    //  ASSERT(m_aSymbols[sBlock.begin] == "for"); // MakeFlowForScope 函数保证
    //  sBlock.end = ParseFlowFor(RTSCOPE(sBlock.begin, scope.end), &uBlock);
    //}
    //else

    if( ! uBlock.ptr)
    {
      ParseExpression(&sBlock, &uBlock);
    }

    DbgDumpScope("for_2", sConditional, sIterator);
    DbgDumpScope("for_1", sInitializer, sBlock);

    GXBOOL bret = MakeSyntaxNode(&D, SYNTAXNODE::MODE_Flow_ForRunning, NULL, &uCond, &uIter);
    bret = bret && MakeSyntaxNode(&D, SYNTAXNODE::MODE_Flow_ForInit, NULL, &uInit, &D);
    bret = bret && MakeSyntaxNode(pUnion, SYNTAXNODE::MODE_Flow_For, NULL, &D, &uBlock);
    
    return bret ? sBlock.end : RTSCOPE::npos;
  }

  GXBOOL CodeParser::MakeInstruction(const SYMBOL* pOpcode, int nMinPrecedence, RTSCOPE* pScope, SYNTAXNODE::UN* pParent, int nMiddle)
  {
    ASSERT((int)pScope->begin <= nMiddle);
    ASSERT(nMiddle <= (int)pScope->end);

    RTSCOPE scopeA(pScope->begin, nMiddle);
    RTSCOPE scopeB(nMiddle + 1, pScope->end);
    SYNTAXNODE::UN A = {0}, B = {0};
    GXBOOL bresult = TRUE;

    if(*pOpcode == '?') {
      const SYMBOL& s = m_aSymbols[nMiddle];
      //SYNTAXNODE sNodeB;
      //B.pNode = &sNodeB;
      bresult = ParseArithmeticExpression(&scopeA, &A, nMinPrecedence);

      if(s.scope >= (int)pScope->begin && s.scope < (int)pScope->end) {
        ASSERT(m_aSymbols[s.scope] == ':');
        bresult = bresult && MakeInstruction(&m_aSymbols[s.scope], nMinPrecedence, &scopeB, &B, s.scope);
      }
      else {
        // ERROR: ?:三元操作符不完整
      }
    }
    else {
      bresult = 
        ParseArithmeticExpression(&scopeA, &A, nMinPrecedence) &&
        ParseArithmeticExpression(&scopeB, &B, nMinPrecedence) ;
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
    if(end - begin > 1 && m_aSymbols[end - 1] == ';') {
      --end;
    }

    if(bRaw)
    {
      if(begin < end) {
        str.Append(m_aSymbols[begin].marker.marker,
          (m_aSymbols[end - 1].marker.marker - m_aSymbols[begin].marker.marker) + m_aSymbols[end - 1].marker.length);
      }
      else {
        str.Clear();
      }
    }
    else
    {
      for (clsize i = begin; i < end; ++i)
      {
        str.Append(m_aSymbols[i].ToString());
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

  GXBOOL CodeParser::MakeSyntaxNode(SYNTAXNODE::UN* pDest, SYNTAXNODE::MODE mode, const SYMBOL* pOpcode, SYNTAXNODE::UN* pOperandA, SYNTAXNODE::UN* pOperandB)
  {
    const SYNTAXNODE::UN* pOperand[] = {pOperandA, pOperandB};
    SYNTAXNODE sNode = {0, mode, pOpcode};

    for(int i = 0; i < 2; ++i)
    {
      const int nFlagShift = SYNTAXNODE::FLAG_OPERAND_SHIFT * i;
      if(pOperand[i] == NULL) {
        sNode.Operand[i].pSym = NULL;
      }
      else if(IsSymbol(pOperand[i])) {
        SET_FLAG(sNode.flags, SYNTAXNODE::FLAG_OPERAND_IS_SYMBOL << nFlagShift);
        sNode.Operand[i].pSym = pOperand[i]->pSym;
      }
      else {
        SET_FLAG(sNode.flags, SYNTAXNODE::FLAG_OPERAND_IS_NODE << nFlagShift);
        ASSERT((size_t)pOperand[i]->pNode < m_aSyntaxNodePack.size()); // 这时候还是索引，所以肯定小于序列的长度
        sNode.Operand[i].pNode = pOperand[i]->pNode;
      }
    }

    pDest->pNode = (SYNTAXNODE*)m_aSyntaxNodePack.size();
    m_aSyntaxNodePack.push_back(sNode);

    return TRUE;
  }

  GXBOOL CodeParser::IsSymbol(const SYNTAXNODE::UN* pUnion) const
  {
    const SYMBOL* pBegin = &m_aSymbols.front();
    const SYMBOL* pBack   = &m_aSymbols.back();

    return pUnion->pSym >= pBegin && pUnion->pSym <= pBack;
  }

  //clsize CodeParser::FindSemicolon( clsize begin, clsize end ) const
  //{
  //  for(; begin < end; ++begin)
  //  {
  //    if(m_aSymbols[begin] == ';') {
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
      }
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
