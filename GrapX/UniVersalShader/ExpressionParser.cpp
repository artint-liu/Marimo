#include <grapx.h>
#include <clUtility.h>
#include <Smart/SmartStream.h>
#include <clStringSet.h>
#include "ExpressionParser.h"

#include "clTextLines.h"
#include "../User/DataPoolErrorMsg.h"

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

static clsize s_nMultiByteOperatorLen = 0; // 最大长度
struct MBO
{
  clsize nLen;
  char* szOperator;
  int precedence; // 优先级，越大越高
};

static MBO s_Operator1[] = {
  {1, ".",  14},
//{1, "+", 13}, // 正号
//{1, "−", 13}, // 负号
  {1, "!", 13},
  {1, "~", 13},
  {1, "&", 13},
  {1, "*", 12},
  {1, "/", 12},
  {1, "%", 12},
  {1, "+", 11}, // 加法
  {1, "-", 11}, // 减法
  {1, "<",  9},
  {1, ">",  9},
  {1, "&",  7},
  {1, "^",  6},
  {1, "|",  5},
  {1, "=",  2},
  {1, "?",  2}, // ?: 操作符
  {1, ":",  2}, // ?: 操作符
  {1, ",",  1},
  {NULL,},
};

static MBO s_Operator2[] = {
  {2, "--", 14},
  {2, "++", 14},
  {2, ">>", 10},
  {2, "<<", 10},
  {2, "<=", 9},
  {2, ">=", 9},
  {2, "==", 8},
  {2, "!=", 8},
  {2, "&&", 4},
  {2, "||", 3},
  {2, "+=", 2},
  {2, "-=", 2},
  {2, "*=", 2},
  {2, "/=", 2},
  {2, "%=", 2},
  {2, "&=", 2},
  {2, "^=", 2},
  {2, "|=", 2},
  {NULL,},

  // 不会用到的符号
  {2, "::",  0},
  {2, "->",  0},
  {2, ".*",  0},
  {2, "->*", 0}, 
};

static MBO s_Operator3[] = {
  {3, "<<=",2},
  {3, ">>=",2},
  {NULL,},
};

#define FOR_EACH_MBO(_N, _IDX) for(int _IDX = 0; s_Operator##_N[_IDX].szOperator != NULL; _IDX++)

inline b32 IS_NUM(char c)
{
  return c >= '0' && c <= '9';
}

namespace UVShader
{
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

  GXBOOL MatchOperator(const MBO* op, u32 op_len, ExpressionParser::iterator& it, u32 remain, int* precedence)
  {
    if(remain <= op_len) {
      return FALSE;
    }

    for(int i = 0; op[i].szOperator != NULL; ++i) {
      ASSERT(op_len == op[i].nLen);
      if(clstd::strncmpT(op[i].szOperator, it.marker, op[i].nLen) == 0)
      {
        *precedence = op[i].precedence;
        it.length = op[i].nLen;
        return TRUE;
      }
    }
    return FALSE;
  }

  u32 CALLBACK ExpressionParser::MultiByteOperatorProc( iterator& it, u32 nRemain, u32_ptr lParam )
  {
    if(it.front() == '.' && it.length > 1) { // 跳过".5"这种格式的浮点数
      return 0;
    }

    ExpressionParser* pParser = (ExpressionParser*)it.pContainer;
    ASSERT(pParser->m_CurSymInfo.sym.marker == NULL); // 每次用完外面都要清理这个

    int precedence = 0;
    // 从多字节到单字节符号匹配,其中有一个返回TRUE就不执行后面的匹配了
    if(
      MatchOperator(s_Operator3, 3, it, nRemain, &precedence) ||
      MatchOperator(s_Operator2, 2, it, nRemain, &precedence) ||
      MatchOperator(s_Operator1, 1, it, nRemain, &precedence) )
    {
      pParser->m_CurSymInfo.precedence = precedence;
      pParser->m_CurSymInfo.sym = it;
      pParser->m_CurSymInfo.pair = -1;
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

    struct PAIR_CONTEXT
    {
      GXLPCSTR szOpen;    // 开区间
      GXLPCSTR szClosed;  // 闭区间
      //PairStack* pStack;
      PairStack sStack;
    };

    PAIR_CONTEXT pair_context[] = {
      {"(", ")", },   // 圆括号
      {"[", "]", },   // 方括号
      {"{", "}", },   // 花括号
      {"?", ":", },
      {NULL, },
    };


    for(auto it = begin(); it != stream_end; ++it)
    {
      sym.sym = it;
      sym.pair = -1;

      ASSERT(m_CurSymInfo.sym.marker == NULL || it.marker == m_CurSymInfo.sym.marker); // 遍历时一定这个要保持一致
      sym.precedence = m_CurSymInfo.precedence;

      // 只是清理
      m_CurSymInfo.sym.marker = NULL;
      m_CurSymInfo.precedence = 0;

      // 符号配对处理
      for(int i = 0; pair_context[i].szOpen != NULL; ++i)
      {
        PAIR_CONTEXT& c = pair_context[i];
        if(it == c.szOpen) {
          c.sStack.push((int)m_aSymbols.size());
        }
        else if(it == c.szClosed) {
          if(c.sStack.empty()) {
            // ERROR: 不匹配            
          }
          sym.pair = c.sStack.top();
          m_aSymbols[sym.pair].pair = (int)m_aSymbols.size();
          c.sStack.pop();
        }
      }

      //if(it == "(") {
      //  stackBrackets.push((int)m_aSymbols.size());
      //}
      //else if(it == "[") {
      //  stackSquareBrackets.push((int)m_aSymbols.size());
      //}
      //else if(it == "{") {
      //  stackBrace.push((int)m_aSymbols.size());
      //}
      //else if(it == ")") {
      //  if(stackBrackets.empty()) {
      //    // ERROR: ")"不匹配
      //  }
      //  sym.pair = stackBrackets.top();
      //  stackBrackets.pop();
      //}
      //else if(it == "]") {
      //  if(stackSquareBrackets.empty()) {
      //    // ERROR: "]"不匹配
      //  }
      //  sym.pair = stackSquareBrackets.top();
      //  stackSquareBrackets.pop();
      //}
      //else if(it == "}") {
      //  if(stackBrace.empty()) {
      //    // ERROR: "}"不匹配
      //  }
      //  sym.pair = stackBrace.top();
      //  stackBrace.pop();
      //}

      //if(sym.pair >= 0) {
      //  m_aSymbols[sym.pair].pair = (int)m_aSymbols.size();
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
    //if( ! stackBrackets.empty()) {
    //  // ERROR: "("不匹配
    //}

    //if( ! stackSquareBrackets.empty()) {
    //  // ERROR: "["不匹配
    //}

    //if( ! stackBrace.empty()) {
    //  // ERROR: "{"不匹配
    //}

    return m_aSymbols.size();
  }

  const ExpressionParser::SymbolArray* ExpressionParser::GetSymbolsArray() const
  {
    return &m_aSymbols;
  }

  GXBOOL ExpressionParser::Parse()
  {
    RTSCOPE scope = {0, m_aSymbols.size()};
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
    if(p[2].sym != "(" || p[2].pair < 0) {
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
    ASSERT(p->pair >= 0);  // 由 GenerateSymbols 函数保证

    // #
    // # ( [ArgumentList] )
    // #
    if(p[0].pair != p[1].pair + 1) // 有参数: 两个括号不相邻
    {
      RTSCOPE ArgScope = {m_aSymbols[p->pair].pair + 1, p->pair};
      ParseFunctionArguments(&stat, &ArgScope);
    }
    p = &m_aSymbols[p->pair];
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
      stat.type = StatementType_Function;
      p = &m_aSymbols[p->pair];
      ++p;
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

    RTSCOPE StruScope = {m_aSymbols[p->pair].pair + 1, p->pair};
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
    for(auto it = m_aStatements.begin(); it != m_aStatements.end(); ++it)
    {
      switch(it->type)
      {
      case StatementType_FunctionDecl:
      case StatementType_Function:
        //it->func.pArguments = &m_aArgumentsPack[(GXINT_PTR)it->func.pArguments];
        IndexToPtr(it->func.pArguments, m_aArgumentsPack);
        break;

      case StatementType_Struct:
      case StatementType_Signatures:
        //it->stru.pMembers = &m_aMembersPack[(GXINT_PTR)it->stru.pMembers];
        IndexToPtr(it->stru.pMembers, m_aMembersPack);
        break;
      }
    }
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

  GXBOOL ExpressionParser::ParseStatementAs_Expression( RTSCOPE* pScope, GXBOOL bDbgRelocale )
  {
    m_nDbgNumOfExpressionParse = 0;
    m_aDbgExpressionOperStack.clear();

    if(pScope->end > 0 && m_aSymbols[pScope->end - 1].sym == ";") {
      --pScope->end;
    }

    STATEMENT stat = {StatementType_Expression};
    
    GXBOOL bret = ParseExpression(pScope, &stat.expr.sRoot, 1);
    TRACE("m_nDbgNumOfExpressionParse=%d\n", m_nDbgNumOfExpressionParse);

#ifdef _DEBUG
    // 这个测试会重定位指针，所以仅作为一次性调用，之后m_aSyntaxNodePack不能再添加新的数据了
    if(bDbgRelocale && ! IsSymbol(&stat.expr.sRoot)) {
      IndexToPtr(stat.expr.sRoot.pNode, m_aSyntaxNodePack);
      RelocaleSyntaxPtr(stat.expr.sRoot.pNode);
      DbgDumpSyntaxTree(stat.expr.sRoot.pNode, 0);
    }
#endif // #ifdef _DEBUG
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
    if(pNode->mode == SYNTAXNODE::MODE_FunctionCall) // 函数调用
    {
      strOut.Format("%s(%s)", str[0], str[1]);
    }
    else if(pNode->mode == SYNTAXNODE::MODE_Definition)
    {
      strOut.Format("%s %s", str[0], str[1]);
    }
    else if(pNode->mode == SYNTAXNODE::MODE_Flow_If || 
      pNode->mode == SYNTAXNODE::MODE_Flow_While ||
      pNode->mode == SYNTAXNODE::MODE_Flow_Switch)
    {
      strOut.Format("%s(%s)", str[0], str[1]);
    }
    else if(pNode->mode == SYNTAXNODE::MODE_Return)
    {
      ASSERT(str[0] == "return");
      strOut.Format("return %s", str[1]);
    }
    else if(precedence > pNode->pOpcode->precedence) { // 低优先级先运算
      strOut.Format("(%s%s%s)", str[0], pNode->pOpcode->sym.ToString(), str[1]);
    }
    else {
      strOut.Format("%s%s%s", str[0], pNode->pOpcode->sym.ToString(), str[1]);
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
    RTSCOPE scope = {begin, end};
    return ParseExpression(&scope, pUnion, nMinPrecedence);
  }

  GXBOOL ExpressionParser::ParseExpression( RTSCOPE* pScope, SYNTAXNODE::UN* pUnion, int nMinPrecedence )
  {
    ASSERT(pScope->begin <= pScope->end);

    const GXINT_PTR count = pScope->end - pScope->begin;
    SYNTAXNODE::UN A, B;
    GXBOOL bret = TRUE;


    if(count <= 1) {
      if(count == 1) {
        pUnion->pSym = &m_aSymbols[pScope->begin];
      }
      return TRUE;
    }
    else if(m_aSymbols[pScope->begin].sym == "return")
    {
      A.pSym = &m_aSymbols[pScope->begin];
      bret = ParseExpression(&B, 1, pScope->begin + 1, pScope->end);
      MakeSyntaxNode(pUnion, SYNTAXNODE::MODE_Return, NULL, &A, &B);
      return bret;
    }

    else if(count == 2)
    {
      // 处理两种可能：(1)变量使用一元符号运算 (2)定义变量

      A.pSym = &m_aSymbols[pScope->begin];
      B.pSym = &m_aSymbols[pScope->begin + 1];

      if(A.pSym->precedence > 0)
      {
        // TODO: 检查支持左侧运算
        MakeSyntaxNode(pUnion, SYNTAXNODE::MODE_Normal, A.pSym, NULL, &B);
      }
      else if(B.pSym->precedence > 0)
      {
        // TODO: 检查支持右侧运算
        MakeSyntaxNode(pUnion, SYNTAXNODE::MODE_Normal, A.pSym, &B, NULL);
      }
      else {
        // 变量声明
        //const TYPE* pType = ParseType(A.pSym);
        MakeSyntaxNode(pUnion, SYNTAXNODE::MODE_Definition, NULL, &A, &B);
      }
    }

    else if(m_aSymbols[pScope->begin].pair == pScope->end - 1)  // 括号内表达式
    {
      // 括号肯定是匹配的
      ASSERT(m_aSymbols[pScope->end - 1].pair == pScope->begin);
      return ParseExpression(pUnion, 1, pScope->begin + 1, pScope->end - 1);
    }

    else if(m_aSymbols[pScope->begin + 1].pair == pScope->end - 1)  // 函数调用
    {
      // 括号肯定是匹配的
      ASSERT(m_aSymbols[pScope->end - 1].pair == pScope->begin + 1);

      A.pSym = &m_aSymbols[pScope->begin];
      clStringA strFunction = A.pSym->sym.ToString();
      
      // TODO: 检查m_aSymbols[pScope->begin]是函数名

      RTSCOPE sArgumentScope = {pScope->begin + 2, pScope->end - 1};
      bret = ParseExpression(&sArgumentScope, &B, 1);

      SYNTAXNODE::MODE mode = SYNTAXNODE::MODE_FunctionCall;
      if(strFunction == "if")
      {
        mode = SYNTAXNODE::MODE_Flow_If;
      }
      else if(strFunction == "while")
      {
        mode = SYNTAXNODE::MODE_Flow_While;
      }
      else if(strFunction == "switch")
      {
        mode = SYNTAXNODE::MODE_Flow_Switch;
      }

      MakeSyntaxNode(pUnion, mode, NULL, &A, &B);

#ifdef _DEBUG
      clStringA strArgs;
      DbgDumpScope(strArgs, sArgumentScope);
      // <Make OperString>
      clStringA strIntruction;
      strIntruction.Format("[F] [%s] [%s]", strFunction, strArgs);
      TRACE("%s\n", strIntruction);
      m_aDbgExpressionOperStack.push_back(strIntruction);
      // </Make OperString>
#endif // #ifdef _DEBUG

      return bret;
    }
    
    int nCandidate = m_nMaxPrecedence;
    GXINT_PTR i = (GXINT_PTR)pScope->end - 1;
    GXINT_PTR nCandidatePos = i;

    while(nMinPrecedence <= m_nMaxPrecedence)
    {
      if(nMinPrecedence == 2)
      {
        for(i = (GXINT_PTR)pScope->begin; i < (GXINT_PTR)pScope->end; ++i)
        {
          m_nDbgNumOfExpressionParse++;

          const SYMBOL& s = m_aSymbols[i];

          if(s.precedence == 0) // 跳过非运算符, 也包括括号
          {
            if(s.pair >= 0) {
              ASSERT(s.pair < (int)pScope->end); // 闭括号肯定在表达式区间内
              i = s.pair;
            }
            continue;
          }
          else if(s.sym == ':') {
            continue;
          }

          // ?: 操作符标记：precedence 储存优先级，pair 储存?:的关系

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

          if(s.precedence == 0) // 跳过非运算符, 也包括括号
          {
            if(s.pair >= 0) {
              ASSERT(s.pair < (int)pScope->end); // 闭括号肯定在表达式区间内
              i = s.pair;
            }
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

  GXBOOL ExpressionParser::MakeInstruction(const SYMBOL* pOpcode, int nMinPrecedence, RTSCOPE* pScope, SYNTAXNODE::UN* pParent, int nMiddle)
  {
    RTSCOPE scopeA = {pScope->begin, nMiddle};
    RTSCOPE scopeB = {nMiddle + 1, pScope->end};
    SYNTAXNODE::UN A = {0}, B = {0};
    GXBOOL bresult = TRUE;

    if(pOpcode->sym == '?') {
      const SYMBOL& s = m_aSymbols[nMiddle];
      //SYNTAXNODE sNodeB;
      //B.pNode = &sNodeB;
      bresult = ParseExpression(&scopeA, &A, nMinPrecedence);

      if(s.pair >= (int)pScope->begin && s.pair < (int)pScope->end) {
        ASSERT(m_aSymbols[s.pair].sym == ':');
        bresult = bresult && MakeInstruction(&m_aSymbols[s.pair], nMinPrecedence, &scopeB, &B, s.pair);
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

#ifdef _DEBUG
    // <Trace>
    clStringA strA, strB;
    DbgDumpScope(strA, scopeA);
    DbgDumpScope(strB, scopeB);

    // <Make OperString>
    clStringA strIntruction;
    strIntruction.Format("[%s] [%s] [%s]", pOpcode->sym.ToString(), strA, strB);
    TRACE("%s\n", strIntruction);
    m_aDbgExpressionOperStack.push_back(strIntruction);
    // </Make OperString>
    // </Trace>
#endif // #ifdef _DEBUG

    return bresult;
  }

  void ExpressionParser::DbgDumpScope( clStringA& str, clsize begin, clsize end, GXBOOL bRaw )
  {
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

  GXBOOL ExpressionParser::MakeSyntaxNode(SYNTAXNODE::UN* pDest, SYNTAXNODE::MODE mode, const SYMBOL* pOpcode, SYNTAXNODE::UN* pOperandA, SYNTAXNODE::UN* pOperandB)
  {
    const SYNTAXNODE::UN* pOperand[] = {pOperandA, pOperandB};
    SYNTAXNODE sNode = {0, mode, pOpcode};

    for(int i = 0; i < 2; ++i)
    {
      const int nFlagShift = SYNTAXNODE::FLAG_OPERAND_SHIFT * i;
      if(IsSymbol(pOperand[i])) {
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
