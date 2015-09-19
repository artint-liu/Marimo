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
  static ExpressionParser::MBO s_plus_minus[] = {
    {1, "+", OPP(12), TRUE, UNARY_RIGHT_OPERAND}, // 正号
    {},
    {1, "-", OPP(12), TRUE, UNARY_RIGHT_OPERAND}, // 负号
  };

  static ExpressionParser::MBO s_Operator1[] = {
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

  static ExpressionParser::MBO s_Operator2[] = {
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

  static ExpressionParser::MBO s_Operator3[] = {
    {3, "<<=", OPP(1)},
    {3, ">>=", OPP(1)},
    {NULL,},
  };
  //////////////////////////////////////////////////////////////////////////

  ExpressionParser::INTRINSIC_TYPE ExpressionParser::s_aIntrinsicType[] = {
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

  ExpressionParser::ExpressionParser()
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

  ExpressionParser::~ExpressionParser()
  {
    SAFE_DELETE(m_pMsg);
  }

  void ExpressionParser::InitPacks()
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
  }

  void ExpressionParser::Cleanup()
  {
    m_aSymbols.clear();
    m_aStatements.clear();

    InitPacks();
  }

  b32 ExpressionParser::Attach( const char* szExpression, clsize nSize )
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
  const ExpressionParser::MBO* MatchOperator(const ExpressionParser::MBO* op, u32 op_len, ExpressionParser::iterator& it, u32 remain)
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

  u32 CALLBACK ExpressionParser::MultiByteOperatorProc( iterator& it, u32 nRemain, u32_ptr lParam )
  {
    if(it.front() == '.' && it.length > 1) { // 跳过".5"这种格式的浮点数
      return 0;
    }

    ExpressionParser* pParser = (ExpressionParser*)it.pContainer;
    ASSERT(pParser->m_CurSymInfo.sym.marker == NULL); // 每次用完外面都要清理这个

    //int precedence = 0;
    const MBO* pProp = NULL;
    // 从多字节到单字节符号匹配,其中有一个返回TRUE就不执行后面的匹配了
    if(
      (pProp = MatchOperator(s_Operator3, 3, it, nRemain)) ||
      (pProp = MatchOperator(s_Operator2, 2, it, nRemain)) ||
      (pProp = MatchOperator(s_Operator1, 1, it, nRemain)) )
    {
      pParser->m_CurSymInfo.precedence = pProp->precedence;
      pParser->m_CurSymInfo.sym = it;
      pParser->m_CurSymInfo.scope = -1;
      pParser->m_CurSymInfo.unary = pProp->unary;
      pParser->m_CurSymInfo.unary_mask = pProp->unary_mask;
      return 0;
    }
    return 0;
  }

  u32 CALLBACK ExpressionParser::IteratorProc( iterator& it, u32 remain, u32_ptr lParam )
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

  clsize ExpressionParser::EstimateForSymbolsCount() const
  {
    auto count = GetStreamCount();
    return (count << 1) + (count >> 1); // 按照 字节数：符号数=2.5：1来计算
  }

  clsize ExpressionParser::GenerateSymbols()
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
    m_CurSymInfo.sym.marker = NULL;
    m_CurSymInfo.precedence = 0;
    m_CurSymInfo.unary      = 0;

    struct PAIR_CONTEXT
    {
      GXLPCSTR szOpen;    // 开区间
      GXLPCSTR szClosed;  // 闭区间
      GXBOOL   bNewEOE;   // 更新End Of Expression的位置
      PairStack sStack;
    };

    PAIR_CONTEXT pair_context[] = {
      {"(", ")", FALSE},   // 圆括号
      {"[", "]", FALSE},   // 方括号
      {"{", "}", TRUE },   // 花括号
      {"?", ":", FALSE},
      {NULL, },
    };

    int EOE = 0; // End Of Expression

    for(auto it = begin(); it != stream_end; ++it)
    {
      const int c_size = (int)m_aSymbols.size();
      sym.sym = it;
      sym.scope = -1;
      sym.semi_scope = -1;

      ASSERT(m_CurSymInfo.sym.marker == NULL || it.marker == m_CurSymInfo.sym.marker); // 遍历时一定这个要保持一致

      sym.precedence = m_CurSymInfo.precedence;
      sym.unary      = m_CurSymInfo.unary;
      sym.unary_mask = m_CurSymInfo.unary_mask;

      // 如果是 -,+ 检查前一个符号是不是操作符或者括号，如果是就认为这个 -,+ 是正负号
      if((it == '-' || it == '+') && ! m_aSymbols.empty())
      {        
        const auto& l_back = m_aSymbols.back();

        // 一元操作符，+/-就不转换为正负号
        // '}' 就不判断了 { something } - abc 这种格式应该是语法错误
        if(l_back.precedence != 0 && l_back.sym != ')' && l_back.sym != ']' && ( ! l_back.unary)) {
          const auto& p = s_plus_minus[(int)(it.marker[0] - '+')];
          sym.precedence = p.precedence;
          sym.unary      = p.unary;
          sym.unary_mask = p.unary_mask;
        }
      }

      // 只是清理
      m_CurSymInfo.sym.marker = NULL;
      m_CurSymInfo.precedence = 0;
      m_CurSymInfo.unary      = 0;

      // 符号配对处理
      for(int i = 0; pair_context[i].szOpen != NULL; ++i)
      {
        PAIR_CONTEXT& c = pair_context[i];
        if(it == c.szOpen) {
          c.sStack.push(c_size);
        }
        else if(it == c.szClosed) {
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

      if(it == ';') {
        //if(EOE < c_size && m_aSymbols[EOE].scope == -1) // ";"的域不能覆盖其他的域记录
        //{
        //  m_aSymbols[EOE].scope = c_size;
        //}

        if(EOE < c_size) {
          m_aSymbols[EOE].semi_scope = c_size;
        }
        EOE = c_size + 1;
      }
      // 这个可能不需要，因为按照语法for应该是在";"分号或者"}"闭花括号后面
      //else if(it == "for") { // for(;;) 这个形式有点特殊
      //  EOE = c_size;
      //}

      m_aSymbols.push_back(sym);
    }

    for(int i = 0; pair_context[i].szOpen != NULL; ++i)
    {
      PAIR_CONTEXT& c = pair_context[i];
      if( ! c.sStack.empty()) {
        // ERROR: 不匹配
      }
    }

    return m_aSymbols.size();
  }

  const ExpressionParser::SymbolArray* ExpressionParser::GetSymbolsArray() const
  {
    return &m_aSymbols;
  }

  GXBOOL ExpressionParser::Parse()
  {
    RTSCOPE scope(0, m_aSymbols.size());
    while(ParseStatement(&scope));
    RelocalePointer();
    return TRUE;
  }

  GXBOOL ExpressionParser::ParseStatement(RTSCOPE* pScope)
  {    
    return (pScope->begin < pScope->end) && (
      ParseStatementAs_Function(pScope) || ParseStatementAs_Struct(pScope));
  }

  GXBOOL ExpressionParser::ParseStatementAs_Function(RTSCOPE* pScope)
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
    if(p->sym == "inline") {
      stat.func.eStorageClass = StorageClass_inline;
      INC_BUT_NOT_END(p, pEnd); // ERROR: inline 应该修饰函数
    }
    else {
      stat.func.eStorageClass = StorageClass_empty;
    }

    // 检测函数格式特征
    if(p[2].sym != "(" || p[2].scope < 0) {
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

    ASSERT(p->sym == "("); // 由前面的特征检查保证
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
    ASSERT(p->sym == ")");

    ++p;

    // #
    // # [: Semantic]
    // #
    if(p->sym == ":") {
      stat.func.szSemantic = m_Strings.add((p++)->sym.ToString());
    }

    if(p->sym == ";") { // 函数声明
      stat.type = StatementType_FunctionDecl;
    }
    else if(p->sym == "{") { // 函数体
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

  GXBOOL ExpressionParser::ParseStatementAs_Struct( RTSCOPE* pScope )
  {
    SYMBOL* p = &m_aSymbols[pScope->begin];
    const SYMBOL* pEnd = &m_aSymbols.front() + pScope->end;

    if(p->sym != "struct") {
      return FALSE;
    }

    STATEMENT stat = {StatementType_Empty};
    INC_BUT_NOT_END(p, pEnd);

    stat.stru.szName = GetUniqueString(p);
    INC_BUT_NOT_END(p, pEnd);

    if(p->sym != "{") {
      // ERROR: 错误的结构体定义
      return FALSE;
    }

    RTSCOPE StruScope(m_aSymbols[p->scope].scope + 1, p->scope);
    // 保证分析函数的域
    ASSERT(m_aSymbols[StruScope.begin - 1].sym == "{" && m_aSymbols[StruScope.end].sym == "}"); 

    pScope->begin = StruScope.end + 1;
    if(m_aSymbols[pScope->begin].sym != ";") {
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

  GXBOOL ExpressionParser::ParseFunctionArguments(STATEMENT* pStat, RTSCOPE* pArgScope)
  {
    // 函数参数格式
    // [InputModifier] Type Name [: Semantic]
    // 函数可以包含多个参数，用逗号分隔

    SYMBOL* p = &m_aSymbols[pArgScope->begin];
    const SYMBOL* pEnd = &m_aSymbols.front() + pArgScope->end;
    ASSERT(pEnd->sym == ")"); // 函数参数列表的结尾必须是闭圆括号

    if(pArgScope->begin >= pArgScope->end) {
      // ERROR: 函数参数声明不正确
      return FALSE;
    }

    ArgumentsArray aArgs;

    while(1)
    {
      FUNCTION_ARGUMENT arg = {InputModifier_in};

      if(p->sym == "in") {
        arg.eModifier = InputModifier_in;
      }
      else if(p->sym == "out") {
        arg.eModifier = InputModifier_out;
      }
      else if(p->sym == "inout") {
        arg.eModifier = InputModifier_inout;
      }
      else if(p->sym == "uniform") {
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

      if(p->sym == ",") {
        aArgs.push_back(arg);
        INC_BUT_NOT_END(p, pEnd); // ERROR: 函数参数声明不正确
        continue;
      }
      else if(p->sym == ":") {
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

  void ExpressionParser::RelocalePointer()
  {
    RelocaleStatements(m_aStatements);
    RelocaleStatements(m_aSubStatements);
  }

  void ExpressionParser::RelocaleSyntaxPtr(SYNTAXNODE* pNode)
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

  GXBOOL ExpressionParser::ParseStructMember( STATEMENT* pStat, RTSCOPE* pStruScope )
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

      if(p->sym == ";") {
        if(pStat->type == StatementType_Empty || pStat->type == StatementType_Struct) {
          pStat->type = StatementType_Struct;
          ++p;
        }
        else {
          // ERROR: 结构体成员作用不一致。纯结构体和Shader标记结构体混合定义
          return FALSE;
        }
      }
      else if(p->sym == ":") {
        if(pStat->type == StatementType_Empty || pStat->type == StatementType_Signatures) {
          pStat->type = StatementType_Signatures;
          INC_BUT_NOT_END(p, pEnd);
          member.szSignature = GetUniqueString(p);
          
          // TODO: 检查这个是Signature

          INC_BUT_NOT_END(p, pEnd);
          if(p->sym != ";") {
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

  GXLPCSTR ExpressionParser::GetUniqueString( const SYMBOL* pSym )
  {
    return m_Strings.add(pSym->sym.ToString());
  }

  const ExpressionParser::TYPE* ExpressionParser::ParseType(const SYMBOL* pSym)
  {
    TYPE sType = {NULL, 1, 1};

    // 对于内置类型，要解析出 Type[RxC] 这种格式
    for(int i = 0; s_aIntrinsicType[i].name != NULL; ++i)
    {
      const INTRINSIC_TYPE& t = s_aIntrinsicType[i];
      if(pSym->sym.BeginsWith(t.name, t.name_len)) {
        const auto* pElement = pSym->sym.marker + t.name_len;
        const int   remain   = pSym->sym.length - (int)t.name_len;
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

  GXBOOL ExpressionParser::ParseStatementAs_Expression(STATEMENT* pStat, RTSCOPE* pScope, GXBOOL bDbgRelocale )
  {
    m_nDbgNumOfExpressionParse = 0;
    m_aDbgExpressionOperStack.clear();

    if(pScope->begin == pScope->end) {
      return TRUE;
    }

    STATEMENT& stat = *pStat;
    
    GXBOOL bret = ParseExpression(pScope, &stat.expr.sRoot, SYMBOL::FIRST_OPCODE_PRECEDENCE);
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

  void ExpressionParser::DbgDumpSyntaxTree(const SYNTAXNODE* pNode, int precedence, clStringA* pStr)
  {
    clStringA str[2];
    for(int i = 0; i < 2; i++)
    {
      if(pNode->Operand[i].pSym) {
        if(IsSymbol(&pNode->Operand[i])) {
          str[i] = pNode->Operand[i].pSym->sym.ToString();
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
      pNode->pOpcode ? pNode->pOpcode->sym.ToString() : "",
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
        strOut.Format("(%s%s%s)", str[0], pNode->pOpcode->sym.ToString(), str[1]);
      }
      else {
        strOut.Format("%s%s%s", str[0], pNode->pOpcode->sym.ToString(), str[1]);
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

  GXBOOL ExpressionParser::ParseExpression(SYNTAXNODE::UN* pUnion, int nMinPrecedence, clsize begin, clsize end)
  {
    RTSCOPE scope(begin, end);
    return ParseExpression(&scope, pUnion, nMinPrecedence);
  }

  GXBOOL ExpressionParser::ParseExpression( RTSCOPE* pScope, SYNTAXNODE::UN* pUnion, int nMinPrecedence )
  {
    ASSERT(pScope->begin <= pScope->end);

    const GXINT_PTR count = pScope->end - pScope->begin;
    SYNTAXNODE::UN A = {0}, B = {0};
    GXBOOL bret = TRUE;


    if(count <= 1) {
      if(count == 1) {
        pUnion->pSym = &m_aSymbols[pScope->begin];
      }
      return TRUE;
    }

    const auto& first = m_aSymbols[pScope->begin];

    if(first.sym == "return")
    {
      A.pSym = &first;
      bret = ParseExpression(&B, SYMBOL::FIRST_OPCODE_PRECEDENCE, pScope->begin + 1, pScope->end);
      MakeSyntaxNode(pUnion, SYNTAXNODE::MODE_Return, NULL, &A, &B);
      return bret;
    }

    else if(first.sym == "if")
    {
      return ParseFlowIf(pScope, pUnion);
    }
    else if(first.sym == "else")
    {
      // ERROR: "else" 不能独立使用
      return FALSE;
    }
    else if(first.sym == "for")
    {
      return ParseFlowFor(pScope, pUnion);
    }
    else if(first.sym == "while")
    {
      return ParseFlowWhile(pScope, pUnion);
    }

    else if(first.sym == "break") {
      A.pSym = &first;
      bret = MakeSyntaxNode(pUnion, SYNTAXNODE::MODE_Flow_Break, NULL, &A, NULL);
      ASSERT(count == 1); // 未处理后面的情况
      return bret;
    }
    else if(first.sym == "continue") {
      A.pSym = &first;
      bret = MakeSyntaxNode(pUnion, SYNTAXNODE::MODE_Flow_Continue, NULL, &A, NULL);
      ASSERT(count == 1); // 未处理后面的情况
      return bret;
    }
    else if(first.sym == "discard") {
      A.pSym = &first;
      bret = MakeSyntaxNode(pUnion, SYNTAXNODE::MODE_Flow_Discard, NULL, &A, NULL);
      ASSERT(count == 1); // 未处理后面的情况
      return bret;
    }
    //else if(first.scope >= 0 && (clsize)first.scope < pScope->end && first.precedence != SYMBOL::ID_BRACE)
    else if(first.semi_scope >= 0 && (clsize)first.semi_scope < pScope->end)
    {
      ASSERT(m_aSymbols[first.semi_scope].sym == ';'); // 目前进入这个循环的只可能是遇到分号
      bret =
        ParseExpression(&A, SYMBOL::FIRST_OPCODE_PRECEDENCE, pScope->begin, first.semi_scope) &&
        ParseExpression(&B, SYMBOL::FIRST_OPCODE_PRECEDENCE, first.semi_scope + 1, pScope->end) &&
        MakeSyntaxNode(pUnion, SYNTAXNODE::MODE_Chain, NULL, &A, &B);

      return bret;
    }

    else if(count == 2)
    {
      // 处理两种可能：(1)变量使用一元符号运算 (2)定义变量

      A.pSym = &first;
      B.pSym = &m_aSymbols[pScope->begin + 1];

      if(B.pSym->sym == ';')
      {
        pUnion->pSym = A.pSym;
        return TRUE;
      }
      else if(A.pSym->precedence > 0)
      {
        // TODO: 检查支持左侧运算
        bret = MakeSyntaxNode(pUnion, SYNTAXNODE::MODE_Normal, A.pSym, NULL, &B);
        DbgDumpScope(A.pSym->sym.ToString(), RTSCOPE(0,0), RTSCOPE(pScope->begin + 1, pScope->end));
      }
      else if(B.pSym->precedence > 0)
      {
        // TODO: 检查支持右侧运算
        bret = MakeSyntaxNode(pUnion, SYNTAXNODE::MODE_Normal, B.pSym, &A, NULL);
        DbgDumpScope(B.pSym->sym.ToString(), RTSCOPE(pScope->begin, pScope->begin + 1), RTSCOPE(0,0));
      }
      else {
        // 变量声明
        bret = MakeSyntaxNode(pUnion, SYNTAXNODE::MODE_Definition, NULL, &A, &B);
      }
      return bret;
    }

    else if(first.scope == pScope->end - 1)  // 括号内表达式
    {
      // 括号肯定是匹配的
      ASSERT(m_aSymbols[pScope->end - 1].scope == pScope->begin);
      return ParseExpression(pUnion, SYMBOL::FIRST_OPCODE_PRECEDENCE, pScope->begin + 1, pScope->end - 1);
    }

    else if(m_aSymbols[pScope->begin + 1].scope == pScope->end - 1)  // 函数调用
    {
      return ParseFunctionCall(pScope, pUnion, nMinPrecedence);
    }
    
    int nCandidate = m_nMaxPrecedence;
    GXINT_PTR i = (GXINT_PTR)pScope->end - 1;
    GXINT_PTR nCandidatePos = i;

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
          else if(s.precedence == 0 || s.sym == ':') {
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

  GXBOOL ExpressionParser::ParseFunctionCall( RTSCOPE* pScope, SYNTAXNODE::UN* pUnion, int nMinPrecedence )
  {
    // 括号肯定是匹配的
    ASSERT(m_aSymbols[pScope->end - 1].scope == pScope->begin + 1);

    SYNTAXNODE::UN A, B;

    A.pSym = &m_aSymbols[pScope->begin];
    clStringA strFunction = A.pSym->sym.ToString();

    // TODO: 检查m_aSymbols[pScope->begin]是函数名

    GXBOOL bret = ParseExpression(&B, SYMBOL::FIRST_OPCODE_PRECEDENCE, pScope->begin + 2, pScope->end - 1);

    SYNTAXNODE::MODE mode = SYNTAXNODE::MODE_FunctionCall;

    MakeSyntaxNode(pUnion, mode, NULL, &A, &B);
    DbgDumpScope("F", RTSCOPE(pScope->begin, pScope->begin + 1),
      RTSCOPE(pScope->begin + 2, pScope->end - 1));

    return bret;
  }

  GXBOOL ExpressionParser::ParseFlowIf( RTSCOPE* pScope, SYNTAXNODE::UN* pUnion)
  {
    // 与 ParseFlowWhile 相似
    SYNTAXNODE::UN A, B;
    GXBOOL bElseIf = FALSE; // 判断这个if是else if中的表达式

    RTSCOPE sConditional(pScope->begin + 1, m_aSymbols[pScope->begin + 1].scope);
    RTSCOPE sBlock;

    if(sConditional.end >= 0 && sConditional.end < pScope->end) {
      ++sConditional.begin;
    }
    else {
      // ERROR: if 语法错误
      return FALSE;
    }

    sBlock.begin = sConditional.end + 1;
    if(sBlock.begin >= pScope->end) {
      // ERROR: if 语法错误
      return FALSE;
    }

    sBlock.end = m_aSymbols[sBlock.begin].GetScope();
    if(sBlock.end == -1)
    {
      // 如果是"if(...) ...;"这种单语句形式，"if"位置的scope应该就是分号的位置
      sBlock.end = m_aSymbols[pScope->begin].GetScope();

      // "else if(...) ...;" 这种单语句形式，"else"位置的记录了分号的位置
      if(sBlock.end == -1 && pScope->begin > 0) {
        bElseIf = TRUE;
        sBlock.end = m_aSymbols[pScope->begin - 1].GetScope();
      }
    }
    else if(m_aSymbols[sBlock.begin].sym == '{') {
      ++sBlock.begin;
    }


    // 这么写是为了和sConditional检查流程看起来相似
    if( ! (sBlock.end >= 0 && sBlock.end < pScope->end))
    {
      // ERROR: if 语法错误
      return FALSE;
    }




    GXBOOL bret =
      ParseExpression(&sConditional, &A, SYMBOL::FIRST_OPCODE_PRECEDENCE) &&
      ParseExpression(&sBlock, &B, SYMBOL::FIRST_OPCODE_PRECEDENCE) &&
      MakeSyntaxNode(pUnion, SYNTAXNODE::MODE_Flow_If, NULL, &A, &B);

    SYNTAXNODE::MODE eNextMode = SYNTAXNODE::MODE_Chain;


    // 剩余标记当作与if同级的表达式链处理
    if(bret && (pScope->end - sBlock.end) > 1)
    {
      A = *pUnion;

      auto nNextBegin = sBlock.end + 1;
      if(m_aSymbols[nNextBegin].sym == "else") {
        ++nNextBegin;

        if(nNextBegin < m_aSymbols.size() && m_aSymbols[nNextBegin].sym == "if")
        {
          eNextMode = SYNTAXNODE::MODE_Flow_ElseIf;
        }
        else
        {
          eNextMode = SYNTAXNODE::MODE_Flow_Else;
        }
      }

      if(eNextMode == SYNTAXNODE::MODE_Chain) {
        DbgDumpScope(bElseIf ? "elif" : "if", sConditional, sBlock);
      }

      bret =
        ParseExpression(&B, SYMBOL::FIRST_OPCODE_PRECEDENCE, nNextBegin, pScope->end) &&
        MakeSyntaxNode(pUnion, eNextMode, NULL, &A, &B);

      if(eNextMode != SYNTAXNODE::MODE_Chain) {
        if(eNextMode == SYNTAXNODE::MODE_Flow_Else) {
          DbgDumpScope("else", RTSCOPE(0,0), RTSCOPE(nNextBegin, pScope->end));
        }
        DbgDumpScope(bElseIf ? "elif" : "if", sConditional, sBlock);
      }
    }
    else {
      DbgDumpScope(bElseIf ? "elif" : "if", sConditional, sBlock);
    }

    return bret;
  }

  GXBOOL ExpressionParser::ParseFlowWhile( RTSCOPE* pScope, SYNTAXNODE::UN* pUnion)
  {
    // 与 ParseFlowIf 相似
    SYNTAXNODE::UN A, B;

    RTSCOPE sConditional(pScope->begin + 1, m_aSymbols[pScope->begin + 1].scope);
    RTSCOPE sBlock;

    if(sConditional.end >= 0 && sConditional.end < pScope->end) {
      ++sConditional.begin;
    }
    else {
      // ERROR: while 语法错误
      return FALSE;
    }

    sBlock.begin = sConditional.end + 1;
    if(sBlock.begin >= pScope->end) {
      // ERROR: while 语法错误
      return FALSE;
    }

    sBlock.end = m_aSymbols[sBlock.begin].scope;
    if(sBlock.end == -1)
    {
      // 如果是"while(...) ...;"这种单语句形式，"while"位置的scope应该就是分号的位置
      sBlock.end = m_aSymbols[pScope->begin].scope;
    }
    else if(m_aSymbols[sBlock.begin].sym == '{') {
      ++sBlock.begin;
    }

    // 这么写是为了和sConditional检查流程看起来相似
    if( ! (sBlock.end >= 0 && sBlock.end < pScope->end))
    {
      // ERROR: while 语法错误
      return FALSE;
    }

    GXBOOL bret =
      ParseExpression(&sConditional, &A, SYMBOL::FIRST_OPCODE_PRECEDENCE) &&
      ParseExpression(&sBlock, &B, SYMBOL::FIRST_OPCODE_PRECEDENCE) &&
      MakeSyntaxNode(pUnion, SYNTAXNODE::MODE_Flow_While, NULL, &A, &B);

    DbgDumpScope("while", sConditional, sBlock);

    // 剩余标记当作与while同级的表达式链处理
    if(bret && (pScope->end - sBlock.end) > 1)
    {
      A = *pUnion;

      auto nNextBegin = sBlock.end + 1;
      SYNTAXNODE::MODE eNextMode = SYNTAXNODE::MODE_Chain;

      bret =
        ParseExpression(&B, SYMBOL::FIRST_OPCODE_PRECEDENCE, nNextBegin, pScope->end) &&
        MakeSyntaxNode(pUnion, eNextMode, NULL, &A, &B);
    }

    return bret;
  }

  GXBOOL ExpressionParser::MakeScope(MAKESCOPE* pParam)
  {
    const MAKESCOPE& p = *pParam;
    RTSCOPE::TYPE& begin = pParam->pOut->begin;
    RTSCOPE::TYPE& end   = pParam->pOut->end;

    ASSERT(p.pScope->begin < p.pScope->end);

    if(p.begin >= p.pScope->end) {
      // ERROR:
      return FALSE;
    }

    if(p.bIndBegin) {
      begin = m_aSymbols[p.begin].GetScope();

      if(OUT_OF_SCOPE(begin) || begin >= p.pScope->end) {
        // ERROR:
        return FALSE;
      }
    }
    else {
      begin = p.begin;
    }

    if(p.chTermin != 0 && m_aSymbols[begin].sym == (TChar)p.chTermin) {
      end = begin;
      return TRUE;
    }

    if(p.end >= p.pScope->end) {
      // ERROR:
      return FALSE;
    }

    if(p.bIndEnd) {
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


  GXBOOL ExpressionParser::ParseFlowFor(RTSCOPE* pScope, SYNTAXNODE::UN* pUnion)
  {
    RTSCOPE sInitializer, sConditional, sIterator, sBlock;

    //
    // 初始化部分
    //
    MAKESCOPE ms;
    ms.pOut       = &sInitializer;
    ms.pScope     = pScope;
    ms.begin      = pScope->begin + 2;
    ms.bIndBegin  = FALSE;
    ms.end        = pScope->begin;
    ms.bIndEnd    = TRUE;
    ms.chTermin   = ';';

    if( ! MakeScope(&ms)) {
      return FALSE;
    }

    //if(pScope->begin + 2 < pScope->end) {
    //  sInitializer = RTSCOPE(pScope->begin + 2, m_aSymbols[pScope->begin].scope);
    //  if(OUT_OF_SCOPE(sInitializer.end)) {
    //    // ERROR: 缺少";"
    //    return FALSE;
    //  }

    ASSERT(m_aSymbols[sInitializer.end].sym == ';'); // 应该断在此处
    //}
    //else {
    //  // ERROR: for 语法错误
    //  CLBREAK;
    //  return FALSE;
    //}

    //
    // 条件部分
    //
    ms.pOut       = &sConditional;
    ms.pScope     = pScope;
    ms.begin      = sInitializer.end + 1;
    ms.bIndBegin  = FALSE;
    ms.end        = sInitializer.end + 1;
    ms.bIndEnd    = TRUE;
    ms.chTermin   = ';';

    if( ! MakeScope(&ms)) {
      return TRUE;
    }

    //if(sInitializer.end + 1 < pScope->end)
    //{
    //  const auto nCondScope = sInitializer.end + 1;
    //  sConditional = RTSCOPE(nCondScope, m_aSymbols[nCondScope].sym == ';' 
    //    ? nCondScope : m_aSymbols[nCondScope].scope);

    //  if(OUT_OF_SCOPE(sConditional.end)) {
    //    // ERROR: 缺少";"
    //    return FALSE;
    //  }
    //  ASSERT(m_aSymbols[sConditional.end].sym == ';'); // 应该断在此处
    //}
    //else {
    //  // ERROR: for 语法错误
    //  CLBREAK;
    //  return FALSE;
    //}


    //
    // 迭代部分
    //
    ms = MAKESCOPE(&sIterator, pScope, sConditional.end + 1, FALSE, pScope->begin + 1, TRUE, ')');
    if( ! MakeScope(&ms)) {
      return FALSE;
    }

    if(m_aSymbols[pScope->begin + 1].sym != '(') {
      // ERROR: for 语法错误
      CLBREAK;
      return FALSE;
    }
    //const auto& sBeginOfFor = m_aSymbols[pScope->begin + 1];

    //if(sBeginOfFor.sym != '(') {
    //  // ERROR: for 语法错误
    //  CLBREAK;
    //  return FALSE;
    //}

    //if(sBeginOfFor.scope > (INT_PTR)pScope->end) {
    //  // ERROR: 缺少";"
    //  CLBREAK;
    //  return FALSE;
    //}

    //if(sConditional.end + 1 < pScope->end)
    //{
    //  sIterator = RTSCOPE(sConditional.end + 1, sBeginOfFor.scope);
    //}
    //else {
    //  // ERROR: for 语法错误
    //  CLBREAK;
    //  return FALSE;
    //}


    //
    // 循环语句
    //
    // 1. for(;;) {...}
    // 2. for(;;) ...;

    if(sIterator.end + 1 >= pScope->end) {
      // ERROR: for 缺少语句
      CLBREAK;
      return FALSE;
    }

    auto& sFirstBlock = m_aSymbols[sIterator.end + 1];
    if(sFirstBlock.sym == '{') {
      if(sIterator.end + 2 >= pScope->end) {
        // ERROR: for 缺少语句
        CLBREAK;
        return FALSE;
      }

      sBlock = RTSCOPE(sIterator.end + 2, sFirstBlock.GetScope());
    }
    else {
      // 防止是for(...;...;)这种形式
      if(sIterator.IsValid() && m_aSymbols[sIterator.begin].sym != ')') {
        sBlock = RTSCOPE(sIterator.end + 1, m_aSymbols[sIterator.begin].GetScope());
      }
      else {
        sBlock = RTSCOPE(sIterator.end + 1, FindSemicolon(sIterator.end + 1, pScope->end));
      }
    }

    if(OUT_OF_SCOPE(sBlock.end) || sBlock.end >= pScope->end) {
      // ERROR: for 缺少语句
      CLBREAK;
      return FALSE;
    }


    //SYNTAXNODE::UN A, B, C, D;
    SYNTAXNODE::UN uInit = {0}, uCond = {0}, uIter = {0}, uBlock = {0}, D;

    ParseExpression(&sInitializer, &uInit, SYMBOL::FIRST_OPCODE_PRECEDENCE);
    ParseExpression(&sConditional, &uCond, SYMBOL::FIRST_OPCODE_PRECEDENCE);
    ParseExpression(&sIterator   , &uIter, SYMBOL::FIRST_OPCODE_PRECEDENCE);
    ParseExpression(&sBlock      , &uBlock, SYMBOL::FIRST_OPCODE_PRECEDENCE);

    DbgDumpScope("for_2", sConditional, sIterator);
    DbgDumpScope("for_1", sInitializer, sBlock);

    GXBOOL bret = MakeSyntaxNode(&D, SYNTAXNODE::MODE_Flow_ForRunning, NULL, &uCond, &uIter);
    bret = bret && MakeSyntaxNode(&D, SYNTAXNODE::MODE_Flow_ForInit, NULL, &uInit, &D);
    bret = bret && MakeSyntaxNode(pUnion, SYNTAXNODE::MODE_Flow_For, NULL, &D, &uBlock);
    
    return bret;
  }

  GXBOOL ExpressionParser::MakeInstruction(const SYMBOL* pOpcode, int nMinPrecedence, RTSCOPE* pScope, SYNTAXNODE::UN* pParent, int nMiddle)
  {
    RTSCOPE scopeA(pScope->begin, nMiddle);
    RTSCOPE scopeB(nMiddle + 1, pScope->end);
    SYNTAXNODE::UN A = {0}, B = {0};
    GXBOOL bresult = TRUE;

    if(pOpcode->sym == '?') {
      const SYMBOL& s = m_aSymbols[nMiddle];
      //SYNTAXNODE sNodeB;
      //B.pNode = &sNodeB;
      bresult = ParseExpression(&scopeA, &A, nMinPrecedence);

      if(s.scope >= (int)pScope->begin && s.scope < (int)pScope->end) {
        ASSERT(m_aSymbols[s.scope].sym == ':');
        bresult = bresult && MakeInstruction(&m_aSymbols[s.scope], nMinPrecedence, &scopeB, &B, s.scope);
      }
      else {
        // ERROR: ?:三元操作符不完整
      }
    }
    else {
      bresult = 
        ParseExpression(&scopeA, &A, nMinPrecedence) &&
        ParseExpression(&scopeB, &B, nMinPrecedence) ;
    }

    MakeSyntaxNode(pParent, SYNTAXNODE::MODE_Normal, pOpcode, &A, &B);

    DbgDumpScope(pOpcode->sym.ToString(), scopeA, scopeB);

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

  void ExpressionParser::DbgDumpScope( clStringA& str, clsize begin, clsize end, GXBOOL bRaw )
  {
    if(end - begin > 1 && m_aSymbols[end - 1].sym == ';') {
      --end;
    }

    if(bRaw)
    {
      if(begin < end) {
        str.Append(m_aSymbols[begin].sym.marker,
          (m_aSymbols[end - 1].sym.marker - m_aSymbols[begin].sym.marker) + m_aSymbols[end - 1].sym.length);
      }
      else {
        str.Clear();
      }
    }
    else
    {
      for (clsize i = begin; i < end; ++i)
      {
        str.Append(m_aSymbols[i].sym.ToString());
      }
    }
  }

  void ExpressionParser::DbgDumpScope( clStringA& str, const RTSCOPE& scope )
  {
    DbgDumpScope(str, scope.begin, scope.end, FALSE);
  }

  void ExpressionParser::DbgDumpScope(GXLPCSTR opcode, const RTSCOPE& scopeA, const RTSCOPE& scopeB )
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

  GXBOOL ExpressionParser::MakeSyntaxNode(SYNTAXNODE::UN* pDest, SYNTAXNODE::MODE mode, const SYMBOL* pOpcode, SYNTAXNODE::UN* pOperandA, SYNTAXNODE::UN* pOperandB)
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
        sNode.Operand[i].pNode = pOperand[i]->pNode;
      }
    }

    pDest->pNode = (SYNTAXNODE*)m_aSyntaxNodePack.size();
    m_aSyntaxNodePack.push_back(sNode);

    return TRUE;
  }

  GXBOOL ExpressionParser::IsSymbol(const SYNTAXNODE::UN* pUnion) const
  {
    const SYMBOL* pBegin = &m_aSymbols.front();
    const SYMBOL* pBack   = &m_aSymbols.back();

    return pUnion->pSym >= pBegin && pUnion->pSym <= pBack;
  }

  clsize ExpressionParser::FindSemicolon( clsize begin, clsize end ) const
  {
    for(; begin < end; ++begin)
    {
      if(m_aSymbols[begin].sym == ';') {
        return begin;
      }
    }
    return -1;
  }

  const ExpressionParser::StatementArray& ExpressionParser::GetStatments() const
  {
    return m_aStatements;
  }

  void ExpressionParser::RelocaleStatements( StatementArray& aStatements )
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

  bool ExpressionParser::TYPE::operator<( const TYPE& t ) const
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
