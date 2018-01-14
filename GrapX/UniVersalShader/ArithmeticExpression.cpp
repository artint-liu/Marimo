#include <grapx.h>
#include <clUtility.h>
#include <Smart/SmartStream.h>
#include <clTokens.h>
#include <clStringSet.h>
#include "ArithmeticExpression.h"
#include "UVShaderError.h"
//#include "ExpressionParser.h"

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

#define FOR_EACH_MBO(_N, _IDX) for(int _IDX = 0; s_Operator##_N[_IDX].szOperator != NULL; _IDX++)

namespace UVShader
{
#ifdef USE_CLSTD_TOKENS
  u32 ArithmeticExpression::m_aCharSem[128];
#endif // USE_CLSTD_TOKENS

  ArithmeticExpression::ArithmeticExpression()
    : m_pMsg(NULL)
    //, m_nMaxPrecedence(0)
    , m_nDbgNumOfExpressionParse(0)
    , m_NodePool(128)
    , m_bRefMsg(FALSE)
  {
#ifdef _DEBUG
    // 检查名字与其设定长度是一致的
    for(int i = 0; s_aIntrinsicType[i].name != NULL; ++i) {
      ASSERT(GXSTRLEN(s_aIntrinsicType[i].name) == s_aIntrinsicType[i].name_len);
    }
#endif // #ifdef _DEBUG

#ifdef USE_CLSTD_TOKENS
    u32* aCharSem = m_aCharSem; // TODO: 其实初始化一次就可以
#else
    u32 aCharSem[128];
    GetCharSemantic(aCharSem, 0, 128);
#endif
    int nMaxPrecedence = 0;

    FOR_EACH_MBO(1, i) {
      nMaxPrecedence = clMax(nMaxPrecedence, s_Operator1[i].precedence);
      aCharSem[s_Operator1[i].szOperator[0]] |= M_CALLBACK;
    }

    FOR_EACH_MBO(2, i) {
      nMaxPrecedence = clMax(nMaxPrecedence, s_Operator2[i].precedence);
      aCharSem[s_Operator2[i].szOperator[0]] |= M_CALLBACK;
    }

    FOR_EACH_MBO(3, i) {
      nMaxPrecedence = clMax(nMaxPrecedence, s_Operator3[i].precedence);
      aCharSem[s_Operator3[i].szOperator[0]] |= M_CALLBACK;
    }

    ASSERT(nMaxPrecedence <= (1 << (TOKEN::precedence_bits - 1))); // 检测位域表示范围没有超过优先级
    ASSERT(nMaxPrecedence != TOKEN::ID_BRACE); // 保证优先级最大值与括号的ID不冲突
    ASSERT(s_MaxPrecedence == nMaxPrecedence); // 如果两个不一致，修改 s_MaxPrecedence 的值

    SetFlags(GetFlags() | F_SYMBOLBREAK);

#ifdef USE_CLSTD_TOKENS
#else
    SetCharSemantic(aCharSem, 0, 128);
#endif // USE_CLSTD_TOKENS

    //SetIteratorCallBack(IteratorProc, 0);
    //SetTriggerCallBack(MultiByteOperatorProc, 0);
  }

  ArithmeticExpression::~ArithmeticExpression()
  {
    m_NodePool.Clear();
  }

  const ArithmeticExpression::TOKEN::Array* ArithmeticExpression::GetTokensArray() const
  {
    return &m_aTokens;
  }

  clsize ArithmeticExpression::EstimateForTokensCount() const
  {
    auto count = GetStreamCount();
    return (count << 1) + (count >> 1); // 按照 字节数：符号数=2.5：1来计算
  }

  const ArithmeticExpression::MBO* MatchOperator(const ArithmeticExpression::MBO* op, u32 op_len, ArithmeticExpression::iterator& it, u32 remain)
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

  u32 CALLBACK ArithmeticExpression::IteratorProc( iterator& it, u32 remain, u32_ptr lParam )
  {
    if(it.BeginsWith('\"') && it.EndsWith('\"')) {
      TOKEN& l_token = *(TOKEN*)lParam;
      l_token.type = TOKEN::TokenType_String;
      return 0;
    }

    GXBOOL bIsNumeric = SmartStreamUtility::ExtendToCStyleNumeric(it, remain);
    if(bIsNumeric) {
      TOKEN& l_token = *(TOKEN*)lParam;
      l_token.type = TOKEN::TokenType_Numeric;
    }
    ASSERT((int)remain >= 0);
    return 0;
  }

  u32 CALLBACK ArithmeticExpression::MultiByteOperatorProc( iterator& it, u32 remain, u32_ptr lParam )
  {
    ASSERT(lParam != NULL); // 需要 lParam 指向一个 TOKEN 结构体作为临时储存对象

    if(it.front() == '.' && it.length > 1) { // 跳过".5"这种格式的浮点数
      return 0;
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
    else
    {
      TOKEN& l_token = *(TOKEN*)lParam;
      //CodeParser* pParser = (CodeParser*)it.pContainer;
      ASSERT(l_token.marker == NULL); // 每次用完外面都要清理这个

      const MBO* pProp = NULL;
      // 从多字节到单字节符号匹配,其中有一个返回TRUE就不执行后面的匹配了
      if(
        (pProp = MatchOperator(s_Operator3, 3, it, remain)) ||
        (pProp = MatchOperator(s_Operator2, 2, it, remain)) ||
        (pProp = MatchOperator(s_Operator1, 1, it, remain)) )
      {
        l_token.Set(it);
        l_token.precedence = pProp->precedence;
        l_token.scope = -1;
        l_token.unary = pProp->unary;
        l_token.unary_mask = pProp->unary_mask;
        l_token.type = TOKEN::TokenType_Operator;
      }
    }
    return 0;
  }

  const ArithmeticExpression::SYNTAXNODE* ArithmeticExpression::TryGetNode(const SYNTAXNODE::DESC* pDesc) const
  {
    if(pDesc->un.IsNode()) {
      return pDesc->un.pNode;
    }
    return NULL;
  }

  ArithmeticExpression::SYNTAXNODE::MODE ArithmeticExpression::TryGetNodeMode(const SYNTAXNODE::DESC* pDesc) const
  {
    if(pDesc->un.IsNode()) {
      return pDesc->un.pNode->mode;
    }
    return SYNTAXNODE::MODE_Undefined;
  }

  GXBOOL ArithmeticExpression::MakeSyntaxNode(SYNTAXNODE::DESC* pDest, SYNTAXNODE::MODE mode, SYNTAXNODE::DESC* pOperandA, SYNTAXNODE::DESC* pOperandB)
  {
    return MakeSyntaxNode(pDest, mode, NULL, pOperandA, pOperandB);
  }

  GXBOOL ArithmeticExpression::MakeSyntaxNode(SYNTAXNODE::DESC* pDest, SYNTAXNODE::MODE mode, const TOKEN* pOpcode, SYNTAXNODE::DESC* pOperandA, SYNTAXNODE::DESC* pOperandB)
  {
    const SYNTAXNODE::DESC* pOperand[] = {pOperandA, pOperandB};
    SYNTAXNODE sNode; // = {SYNTAXNODE::FLAG_OPERAND_MAGIC, mode, pOpcode};
    sNode.magic   = SYNTAXNODE::FLAG_OPERAND_MAGIC;
    sNode.mode    = mode;
    sNode.pOpcode = pOpcode;

    for(int i = 0; i < 2; ++i)
    {
      if(pOperand[i] == NULL || pOperand[i]->un.ptr == NULL) {
        ASSERT(pOperand[i] == NULL || pOperand[i]->un.ptr == NULL);
        sNode.Operand[i].ptr = NULL;
      }
      else if(pOperand[i]->un.ptr != NULL) {
        sNode.Operand[i].ptr = pOperand[i]->un.ptr;
      }
      else {
        CLBREAK; // 空的或者不存在的 pOperand[i]->flag 类型        
      }
    }

    pDest->un.pNode = m_NodePool.PushBack(sNode);

#ifdef _DEBUG
    static size_t id = 1;
    pDest->un.pNode->id = id++;
#endif

    return TRUE;
  }

  GXBOOL ArithmeticExpression::MakeInstruction(int depth, const TOKEN* pOpcode, int nMinPrecedence, const TKSCOPE* pScope, SYNTAXNODE::DESC* pParent, int nMiddle)
  {
    ASSERT((int)pScope->begin <= nMiddle);
    ASSERT(nMiddle <= (int)pScope->end);

    TKSCOPE scopeA(pScope->begin, nMiddle);
    TKSCOPE scopeB(nMiddle + 1, pScope->end);
    SYNTAXNODE::DESC A = {0}, B = {0};
    GXBOOL bresult = TRUE;
    //SYNTAXNODE::MODE _mode = SYNTAXNODE::MODE_Opcode;

    if(*pOpcode == '?') // 三元操作符处理
    {
      const TOKEN& s = m_aTokens[nMiddle];
      //SYNTAXNODE sNodeB;
      //B.pNode = &sNodeB;
      bresult = ParseArithmeticExpression(depth + 1, scopeA, &A, nMinPrecedence);

      if(s.scope >= (int)pScope->begin && s.scope < (int)pScope->end) {
        ASSERT(m_aTokens[s.scope] == ':');
        bresult = bresult && MakeInstruction(depth + 1, &m_aTokens[s.scope], nMinPrecedence, &scopeB, &B, s.scope);
      }
      else {
        // ERROR: ?:三元操作符不完整
      }
      MakeSyntaxNode(pParent, SYNTAXNODE::MODE_Opcode, pOpcode, &A, &B);
    }
    else if(*pOpcode == '=' && m_aTokens[nMiddle + 1] == '{')
    {
      bresult =
        ParseArithmeticExpression(depth + 1, scopeA, &A, nMinPrecedence) &&
        ParseArithmeticExpression(depth + 1, scopeB, &B, nMinPrecedence);

      // Opcode 这里设置为空, 因为'='的优先级高于初始化列表中的',', 
      // 在生成表达式时会给初始化列表加上'('和')'使初始化列表出现语法错误.
      MakeSyntaxNode(pParent, SYNTAXNODE::MODE_ArrayAssignment, NULL, &A, &B);
    }
    else
    {
      bresult =
        ParseArithmeticExpression(depth + 1, scopeA, &A, nMinPrecedence) &&
        ParseArithmeticExpression(depth + 1, scopeB, &B, nMinPrecedence);

      MakeSyntaxNode(pParent, SYNTAXNODE::MODE_Opcode, pOpcode, &A, &B);
    }

    if(_CL_NOT_(bresult)) {
      return bresult;
    }

    DbgDumpScope(pOpcode->ToString(), scopeA, scopeB);

    if(pOpcode->unary) {
      if(A.un.pNode != NULL && B.un.pNode != NULL)
      {
        // ERROR: 一元操作符不能同时带有左右操作数
        return FALSE;
      }

      if(TEST_FLAG_NOT(pOpcode->unary_mask, UNARY_LEFT_OPERAND) && A.un.pNode != NULL)
      {
        // ERROR: 一元操作符不接受左值
        return FALSE;
      }

      if(TEST_FLAG_NOT(pOpcode->unary_mask, UNARY_RIGHT_OPERAND) && B.un.pNode != NULL)
      {
        // ERROR: 一元操作符不接受右值
        return FALSE;
      }
    }

    return bresult;
  }
  //////////////////////////////////////////////////////////////////////////

  GXBOOL ArithmeticExpression::ParseArithmeticExpression(int depth, clsize begin, clsize end, SYNTAXNODE::DESC* pDesc)
  {
    TKSCOPE scope(begin, end);
    return ParseArithmeticExpression(depth + 1, scope, pDesc);
  }

  GXBOOL ArithmeticExpression::ParseArithmeticExpression(int depth, const TKSCOPE& scope_in, SYNTAXNODE::DESC* pDesc)
  {
    TKSCOPE scope = scope_in;
    if(scope.end > scope.begin && m_aTokens[scope.end - 1] == ';') {
      scope.end--; // TODO: 确定这个是否为必须
    }
    return ParseArithmeticExpression(depth + 1, scope, pDesc, TOKEN::FIRST_OPCODE_PRECEDENCE);
  }

  GXBOOL ArithmeticExpression::ParseArithmeticExpression(int depth, const TKSCOPE& scope, SYNTAXNODE::DESC* pDesc, int nMinPrecedence)
  {
    int nCandidate = s_MaxPrecedence;
    GXINT_PTR i = (GXINT_PTR)scope.end - 1;
    GXINT_PTR nCandidatePos = i;
    SYNTAXNODE::DESC A, B;

    if(depth > 1000)
    {
      // ERROR: 表达式解析堆栈不足
      m_pMsg->WriteErrorW(TRUE, m_aTokens[scope.begin].offset(), E9999_未定义错误_vsd, __FILEW__, __LINE__);
      return FALSE;
    }

    const GXINT_PTR count = scope.end - scope.begin;

    if(count <= 1) {
      if(count == 1) {
        *pDesc = m_aTokens[scope.begin];
      }
      return TRUE;
    }

    const auto& front = m_aTokens[scope.begin];

    if(count == 2)
    {
      // 处理两种可能：(1)变量使用一元符号运算 (2)定义变量
      A = front;
      B = m_aTokens[scope.begin + 1];
      GXBOOL bret = TRUE;

      ASSERT(*B.un.pTokn != ';'); // 已经在外部避免了表达式内出现分号

      if(A.un.pTokn->precedence > 0)
      {
        bret = MakeSyntaxNode(pDesc, SYNTAXNODE::MODE_Opcode, A.un.pTokn, NULL, &B);
        DbgDumpScope(A.un.pTokn->ToString(), TKSCOPE(0,0), TKSCOPE(scope.begin + 1, scope.end));
      }
      else if(B.un.pTokn->precedence > 0)
      {
        bret = MakeSyntaxNode(pDesc, SYNTAXNODE::MODE_Opcode, B.un.pTokn, &A, NULL);
        DbgDumpScope(B.un.pTokn->ToString(), TKSCOPE(scope.begin, scope.begin + 1), TKSCOPE(0,0));
      }
      else {
        // 变量声明
        bret = MakeSyntaxNode(pDesc, SYNTAXNODE::MODE_Definition, &A, &B);
      }
      return bret;
    }
    else if(front.precedence == 0 && m_aTokens[scope.begin + 1].precedence == 0) // 变量声明
    {
      ASSERT(count > 2);
      SYNTAXNODE::MODE mode = SYNTAXNODE::MODE_Definition;
      TKSCOPE scope_expr(scope.begin + 1, scope.end);
      //if(front == "const") {
      //  if(count == 3) {
      //    // ERROR: 缺少常量赋值
      //    return FALSE;
      //  }
      //  else if(m_aTokens[scope.begin + 2].precedence != 0)
      //  {
      //    // m_aTokens[scope.begin + 1] 是类型 ERROR: 缺少适当的变量名
      //    // m_aTokens[scope.begin + 1] 不是类型 ERROR: 缺少类型名
      //    return FALSE;
      //  }

      //  mode = SYNTAXNODE::MODE_DefinitionConst;
      //  A = m_aTokens[scope.begin + 1];
      //  scope_expr.begin++;
      //}
      //else
      {
        A = front;
      }
      B.un.ptr = NULL;
      GXBOOL bret = ParseArithmeticExpression(depth + 1, scope_expr, &B);
      bret = bret && MakeSyntaxNode(pDesc, mode, &A, &B);
      return bret;
    }
    else if((front == '(' || front == '[' || front == '{') && front.scope == scope.end - 1)  // 括号内表达式
    {
      // 括号肯定是匹配的
      ASSERT(m_aTokens[scope.end - 1].scope == scope.begin);
      return ParseArithmeticExpression(depth + 1, TKSCOPE(scope.begin + 1, scope.end - 1), pDesc, TOKEN::FIRST_OPCODE_PRECEDENCE);
    }
    else if(m_aTokens[scope.begin + 1].scope == scope.end - 1)  // 整个表达式是函数调用
    {
      return ParseFunctionCall(scope, pDesc);
    }

    while(nMinPrecedence <= s_MaxPrecedence)
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
            return MakeInstruction(depth + 1, &s, nMinPrecedence, &scope, pDesc, i);
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
            return MakeInstruction(depth + 1, &s, nMinPrecedence, &scope, pDesc, i);
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

    if( ! ParseFunctionIndexCall(scope, pDesc))
    {
      GXINT_PTR len = (m_aTokens[scope.end - 1].marker - m_aTokens[scope.begin].marker) + m_aTokens[scope.end - 1].length;
      ASSERT(len >= 0);
      clStringA strMsg(m_aTokens[scope.begin].marker, len);
      TRACE("ERROR: 无法解析\"%s\"\n", strMsg);
      return FALSE;
    }
    return TRUE;
  }

  GXBOOL ArithmeticExpression::ParseFunctionIndexCall(const TKSCOPE& scope, SYNTAXNODE::DESC* pDesc)
  {
    // 从右到左解析这两种形式:
    // name(...)(...)(...)
    // name[...][...][...]
    // 括号域之间不能有其他符号, 括号域之内是数学表达式

    struct CONTEXT
    {
      SYNTAXNODE::MODE mode;
      SYNTAXNODE::DESC B;
    };

    typedef clstack<CONTEXT> SyntaxStack;
    SyntaxStack node_stack;
    SYNTAXNODE::DESC A;
    CONTEXT c;
    TOKEN* pBack = &m_aTokens[scope.end - 1];
    A = m_aTokens[scope.begin];
    ASSERT(A.un.pTokn->precedence == 0); // 第一个必须不是运算符号


    while(1) {
      if(pBack->scope == TKSCOPE::npos) {
        ERROR_MSG__MISSING_SEMICOLON(*A.un.pTokn);
        return FALSE;
      }
      c.mode = *pBack == ')' ? SYNTAXNODE::MODE_FunctionCall : SYNTAXNODE::MODE_ArrayIndex;
      c.B.un.ptr = NULL;

      if( ! ParseArithmeticExpression(0, TKSCOPE(pBack->scope + 1, pBack - &m_aTokens.front()), &c.B)) {
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
      if( ! MakeSyntaxNode(pDesc, c.mode, &A, &c.B)) {
        CLBREAK;
        return FALSE;
      }

      if( ! node_stack.empty()) {
        c = node_stack.top();
        node_stack.pop();
        A = *pDesc;
      }
      else {
        break;
      }
    }

    return TRUE;
  }

  GXBOOL ArithmeticExpression::CompareToken(int index, TOKEN::T_LPCSTR szName)
  {
    if(index < 0 || index >= (int)m_aTokens.size()) {
      return FALSE;
    }
    return m_aTokens[index] == szName;
  }

  GXBOOL ArithmeticExpression::ParseFunctionCall(const TKSCOPE& scope, SYNTAXNODE::DESC* pDesc)
  {
    // 括号肯定是匹配的
    ASSERT(m_aTokens[scope.end - 1].scope == scope.begin + 1);

    SYNTAXNODE::DESC A, B = {0};
    A = m_aTokens[scope.begin];

    // TODO: 检查m_aTokens[scope.begin]是函数名

    const TOKEN& bracket = m_aTokens[scope.begin + 1];
    ASSERT(bracket == '[' || bracket == '(');
    GXBOOL bret = ParseArithmeticExpression(0, TKSCOPE(scope.begin + 2, scope.end - 1), &B, TOKEN::FIRST_OPCODE_PRECEDENCE);

    SYNTAXNODE::MODE mode;
    if(bracket == '(') {
      mode = SYNTAXNODE::MODE_FunctionCall;
    }
    else if(bracket.scope == m_aTokens[bracket.scope].scope + 1) // "[]"
    {
      mode = SYNTAXNODE::MODE_ArrayAlloc;
    }
    else {
     mode = SYNTAXNODE::MODE_ArrayIndex;
    }

    MakeSyntaxNode(pDesc, mode, &A, &B);
    DbgDumpScope(bracket == '(' ? "F" : "I", TKSCOPE(scope.begin, scope.begin + 1),
      TKSCOPE(scope.begin + 2, scope.end - 1));

    return bret;
  }

  void ArithmeticExpression::InitTokenScope(TKSCOPE& scope, const TOKEN& token) const
  {
    // 使用token的括号作用域初始化一个作用域
    ASSERT(&token >= &m_aTokens.front() && &token < &m_aTokens.back()); // token必须是本类的序列中的
    ASSERT(token.scope != -1); // 必须有匹配

    scope.end = token.scope;
    scope.begin = m_aTokens[token.scope].scope;
  }

  GXBOOL ArithmeticExpression::MarryBracket(PairStack* sStack, TOKEN& token)
  {
    // 返回值表示是否更新表达式结尾(End of Expresion)

    if(token.type == TOKEN::TokenType_String) {
      return FALSE;
    }

    const int c_size = (int)m_aTokens.size();
    for(int i = 0; i < countof(s_PairMark); ++i)
    {
      PAIRMARK& c = s_PairMark[i];
      PairStack&    s = sStack[i];
      if(token == c.chOpen) {
        s.push(c_size);
      }
      else if(token == c.chClosed) {
        if(s.empty()) {
          if(c.bCloseAE) {
            ASSERT(token == ':'); // 目前只有这个
            token.SetArithOperatorInfo(s_semantic);
            break;
          }
          else {
            // ERROR: 括号不匹配
            //ERROR_MSG__MISSING_OPENBRACKET;
            m_pMsg->WriteErrorW(TRUE, token.offset(), E2059_SyntaxError_vs, _CLTEXT("("));
            break;
          }
        }
        token.scope = s.top();
        m_aTokens[token.scope].scope = c_size;
        s.pop();
      }
      else {
        continue;
      }

      // ?: 操作符precedence不为0，拥有自己的优先级；其他括号我们标记为括号
      if(token.precedence == 0) {
        token.precedence = TOKEN::ID_BRACE;
      }

      return c.bNewEOE;
        //EOE = c_size + 1;
    } // for
    return FALSE;
  }

  //////////////////////////////////////////////////////////////////////////

  void ArithmeticExpression::DbgDumpScope( clStringA& str, clsize begin, clsize end, GXBOOL bRaw )
  {
    if(end - begin > 1 && m_aTokens[end - 1] == ';') {
      --end;
    }

    if(bRaw)
    {
      if(begin < end) {
        str.Append(m_aTokens[begin].marker,
          (m_aTokens[end - 1].marker - m_aTokens[begin].marker) + m_aTokens[end - 1].length);
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

  void ArithmeticExpression::DbgDumpScope( clStringA& str, const TKSCOPE& scope )
  {
    DbgDumpScope(str, scope.begin, scope.end, FALSE);
  }

  void ArithmeticExpression::DbgDumpScope(GXLPCSTR opcode, const TKSCOPE& scopeA, const TKSCOPE& scopeB )
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

  const clStringArrayA& ArithmeticExpression::DbgGetExpressionStack() const
  {
    return m_aDbgExpressionOperStack;
  }

  //////////////////////////////////////////////////////////////////////////

  void ArithmeticExpression::VALUE::clear()
  {
    uValue64 = 0;
    rank = Rank_Undefined;
  }

  void ArithmeticExpression::VALUE::SetZero()
  {
    uValue64 = 0;
    rank = Rank_Signed;
  }

  void ArithmeticExpression::VALUE::SetOne()
  {
    uValue64 = 1;
    rank = Rank_Signed;
  }

  ArithmeticExpression::VALUE& ArithmeticExpression::VALUE::set(const VALUE& v)
  {
    *this = v;
    return *this;
  }

  ArithmeticExpression::VALUE::State ArithmeticExpression::VALUE::set( const TOKEN& token )
  {
    // 注意，有关上限的检查写的不严谨
    // X是任意内容，D是指数字
    // "-X" "+X" ".X" "Xf"
    // "De-D" "DeD"
    auto ptr     = token.marker;
    size_t count = token.length;
    GXDWORD dwFlags = 0;
    GXQWORD digi[3] = {0}; // [0]是整数部分，[1]是小数部分, [2]是指数
    size_t p = 0; // part
    size_t i = 0;
    GXBOOL bNegExp = FALSE;
    ASSERT(count != 0); // 不可能为空

    rank = Rank_BadValue;

    //if(ptr[count - 1] == 'f' || ptr[count - 1] == 'F') {
    //  SETBIT(dwFlags, Rank_float);
    //  count--;
    //}
    // 符号解析
    if(ptr[i] == '-') {
      SET_FLAG(dwFlags, Rank_Signed);
      i++;
    }
    else if(ptr[i] == '+') {
      i++;
    }
    
    while(ptr[i] == 0x20 || ptr[i] == '\t' || ptr[i] == '\r' || ptr[i] == '\n') { 
      if(++i >= count) { // 只有一个+/-符号
        return State_SyntaxError;
      }
    }

    //if(i >= count) { // 只有一个+/-符号，这个怎么破？
    //  return State_SyntaxError;
    //}

    for(; i < count; i++)
    {
      if(ptr[i] == '.')
      {
        SET_FLAG(dwFlags, Rank_float);
        p++;
        if(p >= 2) { // 不会出现两个‘.’, 这个由TOKEN中的浮点数分析保证
          return State_SyntaxError;
        }
      }
      else if(ptr[i] == 'e' || ptr[i] == 'E')
      {
        p = 2;
        i++;
        SET_FLAG(dwFlags, Rank_float);
        if(i >= count) {
          return State_SyntaxError;
        }

        auto c = ptr[i];
        if(c == '-') {
          bNegExp = TRUE;
        }
        else if(c != '+') {
          i--;
        }        
      }
      else if(ptr[i] >= '0' && ptr[i] <= '9') 
      {
        int n = ptr[i] - '0';
        if(digi[p] >= (ULLONG_MAX - (ULLONG_MAX % 10)) ||
          (digi[p] >= (ULLONG_MAX / 10) && n > (ULLONG_MAX % 10))) {
            return State_Overflow;
        }
        digi[p] = digi[p] * 10 + n;
      }
      else {
        return State_IllegalChar;
      }
    }

    // 例子
     //float x[] = {
     //  9e9, 9.9e9, 9e9f, 9.9e9f,
     //  9e-9, 9.9e-9, 9e-9f, 9.9e-9f };
//#define R 18446744073709551616
//#define R 1e3
//#if R + R > 0
//    int test = 0;
//#endif

    if(dwFlags == Rank_float)
    {
      fValue64 = (double)digi[1];
      while(fValue64 > 1.0) {
        fValue64 *= 0.1;
      }
      fValue64 += digi[0];

      if(digi[2]) {
        if(bNegExp) {
          for(size_t i = 0; i < digi[2]; i++) {
            fValue64 *= 0.1;
          }
        }
        else {
          for(size_t i = 0; i < digi[2]; i++) {
            fValue64 *= 10.0;
          }
        }
      }

      if(ptr[0] == '-') {
        fValue64 = -fValue64;
      }
      SET_FLAG(dwFlags, Rank_F_LongLong);
    }
    else
    {
      ASSERT(digi[1] == 0);
      ASSERT(digi[2] == 0);
      uValue64 = digi[0];

      if(TEST_FLAG(dwFlags, Rank_Signed)) {
        if(nValue64 > 0x8000000000000000) {
          return State_Overflow;
        }
        nValue64 = -nValue64;
      }

      SET_FLAG(dwFlags, Rank_F_LongLong);

      //if((TEST_FLAG(dwFlags, Rank_Signed) && uValue64 > INT_MAX) ||
      //  (TEST_FLAG_NOT(dwFlags, Rank_Signed) && uValue64 > UINT_MAX)) {
      //    SETBIT(dwFlags, Rank_F_LongLong);
      //}
    }

    rank = (Rank)dwFlags;
    return State_OK;
  }

  template<typename _Ty>
  typename _Ty ArithmeticExpression::VALUE::CalculateT(const TOKEN& opcode, _Ty& t1, _Ty& t2)
  {
    if(opcode.length == 1)
    {
      switch(opcode.marker[0])
      {
      case '+': return t1 + t2;
      case '-': return t1 - t2;
      case '*': return t1 * t2;
      case '/': return t1 / t2;
      case '<': return _Ty(t1 < t2);
      case '>': return _Ty(t1 > t2);
      case '!': return _Ty( ! t2);
      default:
        TRACE("Unsupport opcode(%c).\n", opcode);
        CLBREAK;
      }
    }
    else
    {
      if(opcode == "&&") {
        return t1 && t2;
      }
      else if(opcode == "||") {
        return t1 || t2;
      }
      else if(opcode == "==") {
        return t1 == t2;
      }
      else {
        TRACE("Unsupport opcode(%c).\n", opcode);
        CLBREAK;
      }
    }
    return (_Ty)0;
  }

  ArithmeticExpression::VALUE::State ArithmeticExpression::VALUE::Calculate(const TOKEN& token, const VALUE& param0, const VALUE& param1)
  {
    *this = param0;
    VALUE second = param1;
    Rank type = clMax(this->rank, second.rank);
    State state = SyncRank(type);
    state = (state == State_OK) ? second.SyncRank(type) : state;
    
    if(state != State_OK) {
      return state;
    }

    if(type == Rank_Signed64) {
      nValue64 = CalculateT(token, nValue64, second.nValue64);
    }
    else if(type == Rank_Unsigned64) {
      uValue64 = CalculateT(token, uValue64, second.uValue64);
    }
    else if(type == Rank_Double) {
      fValue64 = CalculateT(token, fValue64, second.fValue64);
    }
    else {
      CLBREAK;
      return State_SyntaxError;
    }
    rank = type;
    return State_OK;
  }

  clStringA ArithmeticExpression::VALUE::ToString() const
  {
    clStringA str;
    switch(rank)
    {
    case Rank_Signed64:
      str.AppendInteger64(nValue64);
      break;
    case Rank_Unsigned64:
      str.AppendUInt64(uValue64);
      break;
    case Rank_Double:
      str.AppendFloat((float)fValue64);
      break;
    default:
      CLBREAK;
    }
    return str;
  }

  ArithmeticExpression::VALUE::State ArithmeticExpression::VALUE::SyncRank(Rank _type)
  {
    if(rank == _type || rank == Rank_Undefined) { // 同级或者未定义（一元操作情况）
      return State_OK;
    }
    else if(rank == Rank_BadValue || rank > _type) {
      return State_SyntaxError;
    }

    if(_type == Rank_Double) {
      double d;
      if(rank == Rank_Signed64) {
        d = (double)nValue64;
      }
      else {
        ASSERT(Rank_Unsigned64);
        d = (double)uValue64;
      }
    }
    else {
      ASSERT(_type == Rank_Signed64 && rank == Rank_Unsigned64);
      if(uValue64 & 0x8000000000000000) {
        return State_Overflow;
      }
    }
    rank = _type;
    return State_OK;
  }

  //ArithmeticExpression::VALUE::State ArithmeticExpression::VALUE::SyncLevel(VALUE& t1, VALUE& t2)
  //{
  //  Type type = clMax(t1.type, t2.type);
  //  
  //}
  
  //////////////////////////////////////////////////////////////////////////
  void ArithmeticExpression::TOKEN::ClearMarker()
  {
#ifdef ENABLE_STRINGED_SYMBOL
    symbol.Clear();
#endif // #ifdef ENABLE_STRINGED_SYMBOL
    marker = 0;
    length = 0;
    type = TokenType_Undefine;
    bInStringSet = 0;
  }

  void ArithmeticExpression::TOKEN::Set(const iterator& _iter)
  {
    if(_iter.length != 0)
    {
#ifdef ENABLE_STRINGED_SYMBOL
      symbol = _iter.ToString();
#endif // #ifdef ENABLE_STRINGED_SYMBOL
      scope = -1;
      semi_scope = -1;
      //marker = _iter;
      pContainer = _iter.pContainer;
      marker     = _iter.marker;
      length     = _iter.length;

      bInStringSet = 0;
    }
  }

  void ArithmeticExpression::TOKEN::Set(clstd::StringSetA& sStrSet, const clStringA& str)
  {
    ASSERT(str.IsNotEmpty());
#ifdef ENABLE_STRINGED_SYMBOL
    symbol = str;
#endif // #ifdef ENABLE_STRINGED_SYMBOL
    pContainer = NULL;
    marker     = sStrSet.add(str);
    length     = str.GetLength();
    bInStringSet      = 1;
  }

  void ArithmeticExpression::TOKEN::ClearArithOperatorInfo()
  {
    unary      = 0;
    unary_mask = 0;
    precedence = 0;
  }

  clStringA ArithmeticExpression::TOKEN::ToString() const
  {
    if(type == TokenType_String && ! BeginsWith('\"')) {
      return clStringA("\"") + ToRawString() + '\"';
    }
    return ToRawString();
  }

  int ArithmeticExpression::TOKEN::GetScope() const
  {
    return scope >= 0 ? scope : semi_scope;
  }

  GXBOOL ArithmeticExpression::TOKEN::operator==(const TOKEN& t) const
  {
    // 不可能出现指向同一地址却长度不同的情况
    ASSERT((marker == t.marker && length == t.length) || 
      (marker != t.marker));

    return (marker == t.marker) || (length == t.length
      && GXSTRNCMP(marker, t.marker, length) == 0);
  }

  GXBOOL ArithmeticExpression::TOKEN::operator==(SmartStreamA::T_LPCSTR str) const
  {
    return (*static_cast<const iterator*>(this) == str);
  }

  GXBOOL ArithmeticExpression::TOKEN::operator==(SmartStreamA::TChar ch) const
  {
    return (*static_cast<const iterator*>(this) == ch);
  }

  GXBOOL ArithmeticExpression::TOKEN::operator!=(SmartStreamA::T_LPCSTR str) const
  {
    return (*static_cast<const iterator*>(this) != str);
  }

  GXBOOL ArithmeticExpression::TOKEN::operator!=(SmartStreamA::TChar ch) const
  {
    return (*static_cast<const iterator*>(this) != ch);
  }

  b32 ArithmeticExpression::TOKEN::operator<(const TOKEN& _token) const
  {
    return (b32)(*static_cast<const iterator*>(this) < _token);
  }

  b32 ArithmeticExpression::TOKEN::IsIdentifier() const
  {
    if(length < 1) {
      return FALSE;
    }

    if(_CL_NOT_(marker[0] == '_' ||
      (marker[0] >= 'A' && marker[0] <= 'Z') ||
      (marker[0] >= 'a' && marker[0] <= 'z') ))
    {
      return FALSE;
    }
    
    for(clsize i = 1; i < length; i++)
    {
      if(_CL_NOT_(marker[0] == '_' ||
        (marker[0] >= 'A' && marker[0] <= 'Z') ||
        (marker[0] >= 'a' && marker[0] <= 'z') ||
        (marker[0] >= '0' && marker[0] <= '9') ))
      {
        return FALSE;
      }
    }
    return TRUE;
  }

} // namespace UVShader