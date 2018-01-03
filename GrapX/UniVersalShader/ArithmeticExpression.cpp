#include <grapx.h>
#include <clUtility.h>
#include <Smart/SmartStream.h>
#include <clTokens.h>
#include <clStringSet.h>
#include "ArithmeticExpression.h"
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
#ifdef ENABLE_NODE_INDEX
#else
    , m_pNewNode(NULL)
#endif
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
#ifdef ENABLE_NODE_INDEX
#else
    ClearSyntaxNodePool();
#endif
  }

  //inline b32 IS_NUM(char c)
  //{
  //  return c >= '0' && c <= '9';
  //}
  /*
  clsize ArithmeticExpression::GenerateTokens()
  {
    auto stream_end = end();
    ASSERT(m_aTokens.empty()); // 调用者负责清空

    m_aTokens.reserve(EstimateForTokensCount());
    typedef clstack<int> PairStack;
    //PairStack stackBrackets;        // 圆括号
    //PairStack stackSquareBrackets;  // 方括号
    //PairStack stackBrace;           // 花括号
    TOKEN token;

    // 只是清理
    m_CurSymInfo.ClearMarker();
    m_CurSymInfo.precedence = 0;
    m_CurSymInfo.unary      = 0;

    struct PAIR_CONTEXT
    {
      GXCHAR    chOpen;         // 开区间
      GXCHAR    chClosed;       // 闭区间
      GXUINT    bNewEOE   : 1;  // 更新End Of Expression的位置
      GXUINT    bCloseAE  : 1;  // AE = Another Explanation, 闭区间符号有另外解释，主要是"...?...:..."操作符
      PairStack sStack;
    };

    // 这里面有 PairStack 对象，所以不能声明为静态的
    PAIR_CONTEXT pair_context[] = {
      {'(', ')', FALSE, FALSE},   // 圆括号
      {'[', ']', FALSE, FALSE},   // 方括号
      {'{', '}', TRUE , FALSE},   // 花括号
      {'?', ':', FALSE, TRUE},    // 冒号不匹配会被解释为语意
      {NULL, },
    };

    int EOE = 0; // End Of Expression

    for(auto it = begin(); it != stream_end; ++it)
    {
      // 上一个预处理结束后，后面的语句长度设置为0可以回到主循环步进
      // 这样做是为了避免如果下一条语句也是预处理指令，不会在处理回调中发生递归调用
      if(it.length == 0) {
        continue;
      }


      const int c_size = (int)m_aTokens.size();
      token.Set(it);
      token.scope = -1;
      token.semi_scope = -1;

      ASSERT(m_CurSymInfo.marker == NULL ||
        m_CurSymInfo.marker == it.marker); // 遍历时一定这个要保持一致

      token.SetArithOperatorInfo(m_CurSymInfo);

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
      m_CurSymInfo.ClearMarker();
      m_CurSymInfo.ClearArithOperatorInfo();


      // 符号配对处理
      for(int i = 0; pair_context[i].chOpen != NULL; ++i)
      {
        PAIR_CONTEXT& c = pair_context[i];
        if(it == c.chOpen) {
          c.sStack.push(c_size);
        }
        else if(it == c.chClosed) {
          if(c.sStack.empty()) {
            if(c.bCloseAE) {
              ASSERT(token == ':'); // 目前只有这个
              token.SetArithOperatorInfo(s_semantic);
              break;
            }
            else {
              // ERROR: 括号不匹配
              ERROR_MSG__MISSING_OPENBRACKET;
            }
          }
          token.scope = c.sStack.top();
          m_aTokens[token.scope].scope = c_size;
          c.sStack.pop();
        }
        else {
          continue;
        }

        // ?: 操作符precedence不为0，拥有自己的优先级；其他括号我们标记为括号
        if(token.precedence == 0) {
          token.precedence = TOKEN::ID_BRACE;
        }

        if(c.bNewEOE) {
          EOE = c_size + 1;
        }
        break;
      } // for

      if(token.precedence == 0)
      {
        auto iter_token = m_Macros.find(token.ToString());
        if(iter_token != m_Macros.end())
        {
          token.Set(iter_token->second.value.marker);
        }
      }

      m_aTokens.push_back(token);

      if(it == ';') {
        ASSERT(EOE < (int)m_aTokens.size());
        for(auto it = m_aTokens.begin() + EOE; it != m_aTokens.end(); ++it)
        {
          it->semi_scope = c_size;
        }
        EOE = c_size + 1;
      }
    }

    for(int i = 0; pair_context[i].chOpen != NULL; ++i)
    {
      PAIR_CONTEXT& c = pair_context[i];
      if( ! c.sStack.empty()) {
        // ERROR: 不匹配
        ERROR_MSG__MISSING_CLOSEDBRACKET;
      }
    }

    return m_aTokens.size();
  }
  //*/

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

  //ArithmeticExpression::SYNTAXNODE::FLAGS ArithmeticExpression::TryGetNodeType( const SYNTAXNODE::DESC* pDesc ) const
  //{
  //  if(pDesc->un.ptr >= &m_aTokens.front() && pDesc->un.ptr <= &m_aTokens.back()) {
  //    return SYNTAXNODE::FLAG_OPERAND_IS_TOKEN;
  //  }
  //  else if(pDesc->ptr >= &m_aSyntaxNodePack.front() && pDesc->ptr <= &m_aSyntaxNodePack.back()) {
  //    return SYNTAXNODE::FLAG_OPERAND_IS_NODE;
  //  }
  //  else {
  //    return SYNTAXNODE::FLAG_OPERAND_IS_NODEIDX;
  //  }
  //}

  //ArithmeticExpression::SYNTAXNODE::MODE ArithmeticExpression::TryGetNode( const SYNTAXNODE::DESC* pDesc ) const
  //{
  //  if(pDesc->ptr >= &m_aTokens.front() && pDesc->ptr <= &m_aTokens.back()) {
  //    return SYNTAXNODE::MODE_Undefined;
  //  }
  //  else if(pDesc->ptr >= &m_aSyntaxNodePack.front() && pDesc->ptr <= &m_aSyntaxNodePack.back()) {
  //    return pDesc->pNode->mode;
  //  }
  //  else {
  //    return m_aSyntaxNodePack[pDesc->un.nNodeIndex].mode;
  //  }
  //}

  const ArithmeticExpression::SYNTAXNODE* ArithmeticExpression::TryGetNode(const SYNTAXNODE::DESC* pDesc) const
  {
    switch(pDesc->flag)
    {
    case SYNTAXNODE::FLAG_OPERAND_IS_TOKEN:
      return NULL;
    case SYNTAXNODE::FLAG_OPERAND_IS_NODE:
      return pDesc->un.pNode;
#ifdef ENABLE_NODE_INDEX
    case SYNTAXNODE::FLAG_OPERAND_IS_NODEIDX:
      return &m_aSyntaxNodePack[pDesc->un.nNodeIndex];
#endif
    }
    CLBREAK; // 不可能到这里
    return NULL;
  }

  ArithmeticExpression::SYNTAXNODE::MODE ArithmeticExpression::TryGetNodeMode(const SYNTAXNODE::DESC* pDesc) const
  {
    switch(pDesc->flag)
    {
    case SYNTAXNODE::FLAG_OPERAND_IS_TOKEN:
      return SYNTAXNODE::MODE_Undefined;
    case SYNTAXNODE::FLAG_OPERAND_IS_NODE:
      return pDesc->un.pNode->mode;
#ifdef ENABLE_NODE_INDEX
    case SYNTAXNODE::FLAG_OPERAND_IS_NODEIDX:
      return m_aSyntaxNodePack[pDesc->un.nNodeIndex].mode;
#endif
    case SYNTAXNODE::FLAG_OPERAND_UNDEFINED:
      return SYNTAXNODE::MODE_Undefined;
    default:
      CLBREAK;
    }
  }

  GXBOOL ArithmeticExpression::MakeSyntaxNode(SYNTAXNODE::DESC* pDest, SYNTAXNODE::MODE mode, SYNTAXNODE::DESC* pOperandA, SYNTAXNODE::DESC* pOperandB)
  {
    return MakeSyntaxNode(pDest, mode, NULL, pOperandA, pOperandB);
  }

  GXBOOL ArithmeticExpression::MakeSyntaxNode(SYNTAXNODE::DESC* pDest, SYNTAXNODE::MODE mode, const TOKEN* pOpcode, SYNTAXNODE::DESC* pOperandA, SYNTAXNODE::DESC* pOperandB)
  {
    const SYNTAXNODE::DESC* pOperand[] = {pOperandA, pOperandB};
    SYNTAXNODE sNode = {0, mode, pOpcode};

    for(int i = 0; i < 2; ++i)
    {
      const int nFlagShift = SYNTAXNODE::FLAG_OPERAND_SHIFT * i;
      if(pOperand[i] == NULL || pOperand[i]->un.ptr == NULL) {
        ASSERT(pOperand[i] == NULL || pOperand[i]->flag == SYNTAXNODE::FLAG_OPERAND_UNDEFINED);
        sNode.Operand[i].ptr = NULL;
      }
      else if(pOperand[i]->flag == SYNTAXNODE::FLAG_OPERAND_IS_TOKEN || pOperand[i]->flag == SYNTAXNODE::FLAG_OPERAND_IS_NODE) {
        sNode.SetOperandType(i, pOperand[i]->flag);
        sNode.Operand[i].ptr = pOperand[i]->un.ptr;
      }
#ifdef ENABLE_NODE_INDEX
      else if(pOperand[i]->flag == SYNTAXNODE::FLAG_OPERAND_IS_NODEIDX) {
        sNode.SetOperandType(i, SYNTAXNODE::FLAG_OPERAND_IS_NODEIDX);
        ASSERT(pOperand[i]->un.nNodeIndex < m_aSyntaxNodePack.size()); // 这时候还是索引，所以肯定小于序列的长度
        sNode.Operand[i].nNodeIndex = pOperand[i]->un.nNodeIndex;
      }
#endif
      else {
        CLBREAK; // 空的或者不存在的 pOperand[i]->flag 类型        
      }
    }

#ifdef ENABLE_NODE_INDEX
    pDest->flag = SYNTAXNODE::FLAG_OPERAND_IS_NODEIDX;
    pDest->un.nNodeIndex = m_aSyntaxNodePack.size();
    m_aSyntaxNodePack.push_back(sNode);
#else
    pDest->flag = SYNTAXNODE::FLAG_OPERAND_IS_NODE;
    pDest->un.pNode = AllocSyntaxNode();
    *pDest->un.pNode = sNode;
#endif

    return TRUE;
  }

  GXBOOL ArithmeticExpression::MakeInstruction(const TOKEN* pOpcode, int nMinPrecedence, const RTSCOPE* pScope, SYNTAXNODE::DESC* pParent, int nMiddle)
  {
    ASSERT((int)pScope->begin <= nMiddle);
    ASSERT(nMiddle <= (int)pScope->end);

    RTSCOPE scopeA(pScope->begin, nMiddle);
    RTSCOPE scopeB(nMiddle + 1, pScope->end);
    SYNTAXNODE::DESC A = {0}, B = {0};
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

    MakeSyntaxNode(pParent, SYNTAXNODE::MODE_Opcode, pOpcode, &A, &B);

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

  GXBOOL ArithmeticExpression::ParseArithmeticExpression(clsize begin, clsize end, SYNTAXNODE::DESC* pDesc)
  {
    RTSCOPE scope(begin, end);
    return ParseArithmeticExpression(scope, pDesc);
  }

  GXBOOL ArithmeticExpression::ParseArithmeticExpression(const RTSCOPE& scope_in, SYNTAXNODE::DESC* pDesc)
  {
    RTSCOPE scope = scope_in;
    if(scope.end > scope.begin && m_aTokens[scope.end - 1] == ';') {
      scope.end--;
    }
    return ParseArithmeticExpression(scope, pDesc, TOKEN::FIRST_OPCODE_PRECEDENCE);
  }

  GXBOOL ArithmeticExpression::ParseArithmeticExpression(const RTSCOPE& scope, SYNTAXNODE::DESC* pDesc, int nMinPrecedence)
  {
    int nCandidate = s_MaxPrecedence;
    GXINT_PTR i = (GXINT_PTR)scope.end - 1;
    GXINT_PTR nCandidatePos = i;
    SYNTAXNODE::DESC A, B;

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
        DbgDumpScope(A.un.pTokn->ToString(), RTSCOPE(0,0), RTSCOPE(scope.begin + 1, scope.end));
      }
      else if(B.un.pTokn->precedence > 0)
      {
        bret = MakeSyntaxNode(pDesc, SYNTAXNODE::MODE_Opcode, B.un.pTokn, &A, NULL);
        DbgDumpScope(B.un.pTokn->ToString(), RTSCOPE(scope.begin, scope.begin + 1), RTSCOPE(0,0));
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
      RTSCOPE scope_expr(scope.begin + 1, scope.end);
      if(front == "const") {
        if(count == 3) {
          // ERROR: 缺少常量赋值
          return FALSE;
        }
        else if(m_aTokens[scope.begin + 2].precedence != 0)
        {
          // m_aTokens[scope.begin + 1] 是类型 ERROR: 缺少适当的变量名
          // m_aTokens[scope.begin + 1] 不是类型 ERROR: 缺少类型名
          return FALSE;
        }

        mode = SYNTAXNODE::MODE_DefinitionConst;
        A = m_aTokens[scope.begin + 1];
        scope_expr.begin++;
      }
      else {
        A = front;
      }
      B.un.ptr = NULL;
      GXBOOL bret = ParseArithmeticExpression(scope_expr, &B);
      bret = bret && MakeSyntaxNode(pDesc, mode, &A, &B);
      return bret;
    }
    else if((front == '(' || front == '[' || front == '{') && front.scope == scope.end - 1)  // 括号内表达式
    {
      // 括号肯定是匹配的
      ASSERT(m_aTokens[scope.end - 1].scope == scope.begin);
      return ParseArithmeticExpression(RTSCOPE(scope.begin + 1, scope.end - 1), pDesc, TOKEN::FIRST_OPCODE_PRECEDENCE);
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
            return MakeInstruction(&s, nMinPrecedence, &scope, pDesc, i);
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
            return MakeInstruction(&s, nMinPrecedence, &scope, pDesc, i);
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

  GXBOOL ArithmeticExpression::ParseFunctionIndexCall(const RTSCOPE& scope, SYNTAXNODE::DESC* pDesc)
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
      if(pBack->scope == RTSCOPE::npos) {
        ERROR_MSG__MISSING_SEMICOLON(*A.un.pTokn);
        return FALSE;
      }
      c.mode = *pBack == ')' ? SYNTAXNODE::MODE_FunctionCall : SYNTAXNODE::MODE_ArrayIndex;
      c.B.un.ptr = NULL;

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

#ifdef ENABLE_NODE_INDEX
#else
  ArithmeticExpression::SYNTAXNODE* ArithmeticExpression::AllocSyntaxNode()
  {
    const size_t c_ElementCount = 128;
    
    ASSERT(m_pNewNode == NULL || (
      m_pNewNode >= m_NodePoolList.back().pBegin &&
      m_pNewNode < m_NodePoolList.back().pEnd));

    if(m_pNewNode == NULL || (++m_pNewNode) == m_NodePoolList.back().pEnd) {
      SYNTAXNODEPOOL pool = {NULL, NULL};
      m_pNewNode = new SYNTAXNODE[c_ElementCount];
      pool.pBegin = m_pNewNode;
      pool.pEnd = pool.pBegin + c_ElementCount;
      m_NodePoolList.push_back(pool);
    }
    return m_pNewNode;
  }

  void ArithmeticExpression::ClearSyntaxNodePool()
  {
    std::for_each(m_NodePoolList.begin(), m_NodePoolList.end(),
      [](SYNTAXNODEPOOL& pool)
    {
      delete[] pool.pBegin;
    });
    m_NodePoolList.clear();
    m_pNewNode = NULL;
  }
#endif
  GXBOOL ArithmeticExpression::ParseFunctionCall(const RTSCOPE& scope, SYNTAXNODE::DESC* pDesc)
  {
    // 括号肯定是匹配的
    ASSERT(m_aTokens[scope.end - 1].scope == scope.begin + 1);

    SYNTAXNODE::DESC A, B = {0};
    A = m_aTokens[scope.begin];

    // TODO: 检查m_aTokens[scope.begin]是函数名

    auto& bracket = m_aTokens[scope.begin + 1];
    ASSERT(bracket == '[' || bracket == '(');
    GXBOOL bret = ParseArithmeticExpression(RTSCOPE(scope.begin + 2, scope.end - 1), &B, TOKEN::FIRST_OPCODE_PRECEDENCE);

    SYNTAXNODE::MODE mode = bracket == '(' ? SYNTAXNODE::MODE_FunctionCall : SYNTAXNODE::MODE_ArrayIndex;

    MakeSyntaxNode(pDesc, mode, &A, &B);
    DbgDumpScope(bracket == '(' ? "F" : "I", RTSCOPE(scope.begin, scope.begin + 1),
      RTSCOPE(scope.begin + 2, scope.end - 1));

    return bret;
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
            ERROR_MSG__MISSING_OPENBRACKET;
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

  void ArithmeticExpression::DbgDumpScope( clStringA& str, const RTSCOPE& scope )
  {
    DbgDumpScope(str, scope.begin, scope.end, FALSE);
  }

  void ArithmeticExpression::DbgDumpScope(GXLPCSTR opcode, const RTSCOPE& scopeA, const RTSCOPE& scopeB )
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

} // namespace UVShader