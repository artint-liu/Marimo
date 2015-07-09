#include <grapx.h>
#include <clUtility.h>
#include <Smart/SmartStream.h>
#include <clStringSet.h>
#include "ExpressionParser.h"

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
  ExpressionParser::INTRINSIC_TYPE s_aIntrinsicType[] = {
    {"bool"},
    {"int"},
    {"uint"},
    {"dword"},
    {"half"},
    {"float"},
    {"double"},
    {NULL},
  };

  ExpressionParser::ExpressionParser()
  {
    u32 aCharSem[128];
    GetCharSemantic(aCharSem, 0, 128);

    FOR_EACH_MBO(1, i) {
      aCharSem[s_Operator1[i].szOperator[0]] |= M_CALLBACK;
    }

    FOR_EACH_MBO(2, i) {
      aCharSem[s_Operator2[i].szOperator[0]] |= M_CALLBACK;
    }

    FOR_EACH_MBO(3, i) {
      aCharSem[s_Operator3[i].szOperator[0]] |= M_CALLBACK;
    }

    SetFlags(GetFlags() | F_SYMBOLBREAK);
    SetCharSemantic(aCharSem, 0, 128);
    SetIteratorCallBack(IteratorProc, 0);
    SetTriggerCallBack(MultiByteOperatorProc, 0);
  }

  b32 ExpressionParser::Attach( const char* szExpression, clsize nSize )
  {
    m_aSymbols.clear();
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
    PairStack stackBrackets;        // 圆括号
    PairStack stackSquareBrackets;  // 方括号
    PairStack stackBrace;           // 花括号
    SYMBOL sym;
    for(auto it = begin(); it != stream_end; ++it)
    {
      sym.sym = it;
      sym.pair = -1;

      ASSERT(m_CurSymInfo.sym.marker == NULL || it.marker == m_CurSymInfo.sym.marker); // 遍历时一定这个要保持一致
      sym.precedence = m_CurSymInfo.precedence;

      // 只是清理
      m_CurSymInfo.sym.marker = NULL;
      m_CurSymInfo.precedence = 0;

      if(it == "(") {
        stackBrackets.push((int)m_aSymbols.size());
      }
      else if(it == "[") {
        stackSquareBrackets.push((int)m_aSymbols.size());
      }
      else if(it == "{") {
        stackBrace.push((int)m_aSymbols.size());
      }
      else if(it == ")") {
        if(stackBrackets.empty()) {
          // ERROR: ")"不匹配
        }
        sym.pair = stackBrackets.top();
        stackBrackets.pop();
      }
      else if(it == "]") {
        if(stackSquareBrackets.empty()) {
          // ERROR: "]"不匹配
        }
        sym.pair = stackSquareBrackets.top();
        stackSquareBrackets.pop();
      }
      else if(it == "}") {
        if(stackBrace.empty()) {
          // ERROR: "}"不匹配
        }
        sym.pair = stackBrace.top();
        stackBrace.pop();
      }

      if(sym.pair >= 0) {
        m_aSymbols[sym.pair].pair = (int)m_aSymbols.size();
      }

      m_aSymbols.push_back(sym);
    }


    if( ! stackBrackets.empty()) {
      // ERROR: "("不匹配
    }

    if( ! stackSquareBrackets.empty()) {
      // ERROR: "["不匹配
    }

    if( ! stackBrace.empty()) {
      // ERROR: "{"不匹配
    }

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
        it->func.pArguments = &m_aArgumentsPack[(GXINT_PTR)it->func.pArguments];
        break;

      case StatementType_Struct:
      case StatementType_Signatures:
        it->stru.pMembers = &m_aMembersPack[(GXINT_PTR)it->stru.pMembers];
        break;
      }
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

  GXBOOL ExpressionParser::ParseType( GXOUT TYPE* pType )
  {
    static GXLPCSTR szBool   = "bool";
    static GXLPCSTR szInt    = "int";
    static GXLPCSTR szUint   = "uint";
    static GXLPCSTR szHalf   = "half";
    static GXLPCSTR szFloat  = "float";
    static GXLPCSTR szDouble = "double";
    
    // 对于内置类型，要解析出 Type[RxC] 这种格式
    return TRUE;
  }

} // namespace UVShader


